/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * Description:
 * Author: huawei
 * Create: 2019-10-15
 */

#include <securec.h>
#include <linux/io.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/semaphore.h>

#include "hsm_info.h"
#include "drv_pkicms.h"
#include "soc_verify.h"
#include "user_cfg_interface.h"
#ifdef CFG_SOC_PLATFORM_MDC_V51
#include "kthread_affinity.h"
#endif

#define SHA256_BYTE_LEN 128
#define INVALID_NVCNT 0xFFFFFFFF
#define NVCNT_WAIT_TIMEOUT 5000

#define SOC_CHIP_INFO_REG_BASE 0x8000F000UL
#define SOC_CHIP_INFO_MAP_SIZE 0x1000
#define SOC_CHIP_INFO_REG_OFFSET 0xFF8
#define CHIP_TYPE_REG_VALUE_MASK 0xF

STATIC u32 g_nvcnt_value[DEVICE_NUM_MAX] = {0};
STATIC struct semaphore g_nvcnt_sema;

STATIC s32 hash_cal_soc(const u8 *in, u32 in_size, u8 *hash, u32 hash_size)
{
    s32 ret;
    u8 hash_str[SHA256_BYTE_LEN] = {0};
    u32 hash_str_len = 0;
    ret = pkicms_sha256_string(in, in_size, hash_str, &hash_str_len);
    if (ret != 0) {
        pkicms_err("Soc verify hash cal soc fail, pki cms err: %d\n", ret);
        return ret;
    }

    if (hash_str_len > hash_size) {
        pkicms_err("Soc verify hash cal soc fail, size err: str_len: %u, max: %u\n", hash_str_len, hash_size);
        return SOC_VERIFY_FAILED;
    }

    ret = memcpy_s(hash, hash_size, hash_str, hash_str_len);
    if (ret != 0) {
        pkicms_err("Soc verify hash cal soc fail, cpy err: %d\n", ret);
    }
    return ret;
}

/**
 * sig: signature data
 * src: src data
 * r_n pub key n data
 * verify_result : 1: success，0: failed
 */
STATIC s32 rsa_verify_soc(const u8 *sig, const u32 sig_len, const u8 *src, u32 src_len, const u8 *r_n, u32 r_len,
    CVB_INT *verify_result, u32 sign_alg)
{
    s32 ret;
    u32 salt_len;
    CMSCBB_ERROR_CODE cms_ret;
    CMSCBB_CRYPTO_VRF_CTX vrf_ctx = CVB_NULL;
    CmscbbBigInt e = {3, {0x01, 0x00, 0x01}};
    CmscbbBigInt n = {0};
#if CMSCBB_SUPPORT_RSAPSS
    CmscbbKeyAndAlgInfo verifyInfo = {0};
#endif
    /* use public key to verify signer info */
    cms_ret = CmscbbCryptoVerifyCreateCtx(&vrf_ctx);
    if (cms_ret != CVB_SUCCESS) {
        pkicms_err("Soc verify err, create ctx fail, err: %u\n", cms_ret);
        return SOC_VERIFY_FAILED;
    }

    do {
        n.uiLength = r_len;
        ret = memcpy_s(n.aVal, CMSCBB_MAX_INT_DIGITS, r_n, r_len);
        if (ret != 0) {
            pkicms_err("Soc verify err, Failed to copy key val, err: %d\n", ret);
            break;
        }
        ret = SOC_VERIFY_FAILED;
#if CMSCBB_SUPPORT_RSAPSS
        verifyInfo.e = &e;
        verifyInfo.n = &n;
        verifyInfo.hashAlg = CMSCBB_HASH_SHA256;
        cms_ret = CmscbbCryptoVerifyInit(vrf_ctx, &verifyInfo);
#else
        cms_ret = CmscbbCryptoVerifyInit(vrf_ctx, &e, &n, CMSCBB_HASH_SHA256);
#endif
        if (cms_ret != CVB_SUCCESS) {
            pkicms_err("Soc verify fail, Failed to verify context err: %u\n", cms_ret);
            break;
        }
        cms_ret = CmscbbCryptoVerifyUpdate(vrf_ctx, src, src_len);
        if (cms_ret != CVB_SUCCESS) {
            pkicms_err("Soc verify fail, verify update err: %u\n", cms_ret);
            break;
        }

        if (((sign_alg >> SIGN_RSA_ALG_SHIFT) & SIGN_RSA_ALG_MASK) == SIGN_RSA_PKCS_MODE) {
            cms_ret = CmscbbCryptoVerifyFinal(vrf_ctx, sig, sig_len, verify_result);
        } else {
            /* 新增pss验证接口 */
            salt_len = ((sign_alg >> PASS_SLEN_SHIFT) & PASS_SLEN_MASK);
            cms_ret = CmscbbCryptoPssVerifyFinal(vrf_ctx, sig, sig_len, verify_result, salt_len);
        }

        if (cms_ret != CVB_SUCCESS) {
            pkicms_err("Soc verify final err, sig_len: %u, src_len: %u, r_len: %u\n", sig_len, src_len, r_len);
            print_hex_dump(KERN_INFO, "rsa verify final err, soc sig value:",
                0, 32, 1, sig, 64, false); /* 32 1 64 */
            print_hex_dump(KERN_INFO, "rsa verify final err, soc src value:",
                0, 32, 1, src, 64, false); /* 32 1 64 */
            print_hex_dump(KERN_INFO, "rsa verify final err, soc r_n value:",
                0, 32, 1, r_n, 64, false); /* 32 1 64 */
            break;
        }
        ret = SOC_VERIFY_SUCCESS;
    } while (0);
    CmscbbCryptoVerifyDestroyCtx(vrf_ctx);
    return ret;
}

STATIC s32 sub_key_cert_auth(const esbc_header_t *image_head, const sub_key_cert_t *sub_key_cert, u32 sign_alg)
{
    u8 pub_e[RSA_E_BYTES] = {0x01, 0x00, 0x01};
    const u8 *image_pub_e = image_head->root_pub_key_e + image_head->root_pub_k_len - RSA_E_BYTES;
    u32 cert_info_len;
    u8 *sub_key_sign = NULL;
    s32 result = 0;
    s32 ret;

    if (sub_key_cert->sub_key_sign_len != ESBC_ROOTKEY_LEN) {
        pkicms_err("sub key cert auth fail, sig n size err, now: %u, expect: %u\n",
            sub_key_cert->sub_key_sign_len,
            ESBC_ROOTKEY_LEN);
        return SOC_VERIFY_FAILED;
    }

#if (defined CFG_FEATURE_RSA_2048) && (defined CFG_FEATURE_RSA_4096)
    if ((sub_key_cert->sub_key_len != RSA_SUBKEY_2048_BYTES) && (sub_key_cert->sub_key_len != RSA_SUBKEY_4096_BYTES)) {
#elif (defined CFG_FEATURE_RSA_4096)
    if (sub_key_cert->sub_key_len != RSA_SUBKEY_4096_BYTES) {
#elif (defined CFG_FEATURE_RSA_2048)
    if (sub_key_cert->sub_key_len != RSA_SUBKEY_2048_BYTES) {
#endif
        pkicms_err("Image sec auth header fail, key len error. (sub_key_len=%u)\n", sub_key_cert->sub_key_len);
        return SOC_VERIFY_FAILED;
    }

    ret = memcmp(image_pub_e, pub_e, RSA_E_BYTES);
    if (ret != 0) {
        pkicms_err("sub key cert auth fail, now get image e 0x[%02x][%02x][%02x] is invalid\n",
            image_pub_e[0],
            image_pub_e[1],
            image_pub_e[2]); /* 0,1,2 print pub_e */
        return SOC_VERIFY_FAILED;
    }

    sub_key_sign = (sub_key_cert->sub_key_len == RSA_SUBKEY_2048_BYTES)
                       ? (u8 *)sub_key_cert->sub_key.subkey_2048.subkey_sign
                       : (u8 *)sub_key_cert->sub_key.subkey_4096.subkey_sign;

    cert_info_len = (sub_key_cert->sub_key_len == RSA_SUBKEY_2048_BYTES)
                        ? (sizeof(sub_key_cert_t) - ESBC_ROOTKEY_LEN - RSA_SUBKEY_4096_BYTES)
                        : (sizeof(sub_key_cert_t) - ESBC_ROOTKEY_LEN);

    ret = rsa_verify_soc((const u8 *)sub_key_sign,
        ESBC_ROOTKEY_LEN,
        (const u8 *)sub_key_cert,
        cert_info_len,
        image_head->root_pub_key,
        image_head->root_pub_k_len,
        &result,
        sign_alg);
    if ((ret != 0) || (result != 1)) {
        pkicms_err("sub key cert auth fail, rsa_verify_err: %d, result: %d\n", ret, result);
        return SOC_VERIFY_FAILED;
    }

    return SOC_VERIFY_SUCCESS;
}

STATIC s32 image_secure_auth_header(
    sub_key_cert_t *sub_key_cert, esbc_header_t *image_head, const efuse_info_t *ef, u32 sign)
{
    s32 ret;
    u32 sub_key_id;
    u8 pub_e[RSA_E_BYTES] = {0x01, 0x00, 0x01};
    u8 *sub_key_e = NULL;
    u8 *sub_key_pub_e = NULL;
    u32 rsa_sign_alg;

    rsa_sign_alg = ((sub_key_cert->sub_key_sign_alg >> SIGN_RSA_ALG_SHIFT) & SIGN_RSA_ALG_MASK);
#if (defined CFG_FEATURE_PSS_SIGN) && (defined CFG_FEATURE_PKCS_SIGN)
    if (((rsa_sign_alg != SIGN_RSA_PKCS_MODE) && (rsa_sign_alg != SIGN_RSA_PSS_MODE)) ||
        ((sign == SIGN_RSA_PSS_MODE) && (rsa_sign_alg != SIGN_RSA_PSS_MODE))) {
#elif (defined CFG_FEATURE_PKCS_SIGN)
    if ((rsa_sign_alg != SIGN_RSA_PKCS_MODE) || (sign != SIGN_RSA_PKCS_MODE)) {
#elif (defined CFG_FEATURE_PSS_SIGN)
    if ((rsa_sign_alg != SIGN_RSA_PSS_MODE) || (sign != SIGN_RSA_PSS_MODE)) {
#endif
        pkicms_err("The sign and sign_alg do not match. (sign_alg=%u; sign=%u)\n", rsa_sign_alg, sign);
        return SOC_VERIFY_FAILED;
    }

    /* 5. root pub key unsigned sub cert */
    ret = sub_key_cert_auth(image_head, sub_key_cert, sub_key_cert->sub_key_sign_alg);
    if (ret != 0) {
        pkicms_err("image sec auth header fail, sub key cert auth err: %d\n", ret);
        return SOC_VERIFY_FAILED;
    }

    sub_key_e = (sub_key_cert->sub_key_len == RSA_SUBKEY_2048_BYTES) ? sub_key_cert->sub_key.subkey_2048.subkey_e
                                                                     : sub_key_cert->sub_key.subkey_4096.subkey_e;

    sub_key_pub_e = sub_key_e + sub_key_cert->sub_key_len - RSA_E_BYTES;
    ret = memcmp(sub_key_pub_e, pub_e, RSA_E_BYTES);
    if (ret != 0) {
        pkicms_err("image sec auth header fail, now get sub e 0x[%02x][%02x][%02x] is invalid\n",
            sub_key_pub_e[0],
            sub_key_pub_e[1],
            sub_key_pub_e[2]); /* 0,1,2 print pub_e */
        return SOC_VERIFY_FAILED;
    }

    /* 6. check sub key category */
    if (ef->hw_category != sub_key_cert->sub_key_category) {
        pkicms_err("image sec auth header fail, image_category: %u, ef: %u\n",
            sub_key_cert->sub_key_category,
            ef->hw_category);
        return SOC_VERIFY_FAILED;
    }

    /* sub key valid check */
    sub_key_id = (sub_key_cert->sub_key_id) & SUBKEY_ID_NUM_MASK;
    if (((1 << sub_key_id) & ef->hw_sub_key_id) != 0) {
        pkicms_err("Sub yek ID check fail\n");
        return SOC_VERIFY_FAILED;
    }

    return SOC_VERIFY_SUCCESS;
}

STATIC s32 head_offset_check(const esbc_header_t *image_head)
{
    if (image_head->code_offset != GENERAL_IMAGE_OFFSET) {
        pkicms_err("Img head offset is invalid. (offset=0x%x)\n", image_head->code_offset);
        return SOC_VERIFY_FAILED;
    }

    if (image_head->subkey_cert_offset != SUBKEY_CERT_OFFSET) {
        pkicms_err("Subkey cert offset is invalid. (offset=0x%x)\n", image_head->subkey_cert_offset);
        return SOC_VERIFY_FAILED;
    }

    if (image_head->sign_offset != CODE_SIGN_OFFSET) {
        pkicms_err("Code sign offset is invalid. (offset=0x%x)\n", image_head->sign_offset);
        return SOC_VERIFY_FAILED;
    }

    return SOC_VERIFY_SUCCESS;
}

STATIC s32 head_info_check(const esbc_header_t *image_head, const efuse_info_t *ef, u32 sign)
{
    s32 ret;
    u32 rsa_sign_alg;

    /* 1.offset phase in header check */
    ret = head_offset_check(image_head);
    if (ret != 0) {
        pkicms_err("head info check fail, offset check err: %d\n", ret);
        return SOC_VERIFY_FAILED;
    }

    /* 2.Determine the L2 signature algorithm and length. */
    /* If the result is 0, the L2 signature hash algorithm is SHA256 */
    if (((image_head->sign_alg) & 0xFFFF) != 0) {
        pkicms_err("Head info check fail, hash not support. (sign_alg=0x%x)\n", image_head->sign_alg);
        return SOC_VERIFY_FAILED;
    }

    rsa_sign_alg = ((image_head->sign_alg >> SIGN_RSA_ALG_SHIFT) & SIGN_RSA_ALG_MASK);
#if (defined CFG_FEATURE_PSS_SIGN) && (defined CFG_FEATURE_PKCS_SIGN)
    if (((rsa_sign_alg != SIGN_RSA_PKCS_MODE) && (rsa_sign_alg != SIGN_RSA_PSS_MODE)) ||
        ((sign == SIGN_RSA_PSS_MODE) && (rsa_sign_alg != SIGN_RSA_PSS_MODE))) {
#elif (defined CFG_FEATURE_PKCS_SIGN)
    if ((rsa_sign_alg != SIGN_RSA_PKCS_MODE) || (sign != SIGN_RSA_PKCS_MODE)) {
#elif (defined CFG_FEATURE_PSS_SIGN)
    if ((rsa_sign_alg != SIGN_RSA_PSS_MODE) || (sign != SIGN_RSA_PSS_MODE)) {
#endif
        pkicms_err("The sign and sign_alg do not match. (sign_alg=%u; sign=%u)\n", rsa_sign_alg, sign);
        return SOC_VERIFY_FAILED;
    }

    if (image_head->root_pub_k_len != ESBC_ROOTKEY_LEN) {
        pkicms_err("head info check fail, rot_pub_len: %u, expect: %u\n", image_head->root_pub_k_len, ESBC_ROOTKEY_LEN);
        return SOC_VERIFY_FAILED;
    }

    /* 3.Verify the root public key. */
    if (ef->hw_hash_check != CHECK_SUCCESS) {
        pkicms_err("head info check fail, ef hash check err\n");
        return SOC_VERIFY_FAILED;
    }

    return SOC_VERIFY_SUCCESS;
}

STATIC s32 image_secure_auth(esbc_header_t *image_head, const efuse_info_t *ef, u32 sign)
{
    s32 ret;
    u8 *sub_key_n = NULL;
    u8 signature[RSA_SUBKEY_4096_BYTES] = {0};
    u8 *sign_content = signature;
    s32 verify_result = 0;
    sub_key_cert_t *sub_key_cert = (sub_key_cert_t *)((uintptr_t)image_head + (image_head->subkey_cert_offset));
    u8 *sig = (u8 *)((uintptr_t)image_head + (image_head->sign_offset));
    u8 *code_sec = (u8 *)((uintptr_t)image_head + (image_head->code_offset));

    ret = head_info_check(image_head, ef, sign);
    if (ret != 0) {
        pkicms_err("image sec auth fail, head check err: %d\n", ret);
        return SOC_VERIFY_FAILED;
    }

    ret = image_secure_auth_header(sub_key_cert, image_head, ef, sign);
    if (ret != 0) {
        pkicms_err("image sec auth fail, auth header err: %d\n", ret);
        return SOC_VERIFY_FAILED;
    }

    sub_key_n = (sub_key_cert->sub_key_len == RSA_SUBKEY_2048_BYTES) ? sub_key_cert->sub_key.subkey_2048.subkey_n
                                                                     : sub_key_cert->sub_key.subkey_4096.subkey_n;

    /* 8.Signature authentication of code segments */
    ret = memcpy_s((void *)signature, RSA_SUBKEY_4096_BYTES, sig, sub_key_cert->sub_key_len);
    if (ret != 0) {
        pkicms_err("image sec auth fail, cpy err: %d\n", ret);
        return SOC_VERIFY_FAILED;
    }

    ret = rsa_verify_soc(sign_content,
        sub_key_cert->sub_key_len,
        code_sec,
        image_head->code_len,
        sub_key_n,
        sub_key_cert->sub_key_len,
        &verify_result,
        image_head->sign_alg);
    if ((ret != 0) || (verify_result != 1)) {
        pkicms_err("image sec auth fail, verify err: %d, result: %d\n", ret, verify_result);
        return SOC_VERIFY_FAILED;
    }

    return SOC_VERIFY_SUCCESS;
}

STATIC s32 pre_package_check(const esbc_header_t *image_head, const u8 *image_position, u32 size)
{
    s32 ret;
    u8 digest[ESBC_HASH_SIZE] = {0};

    /* Two kind of verify should be done even though the secure boot is disable */
    /* 1. Check pre-code in header at first */
    if (image_head->preamble != BOOT_PRE_PARAM) {
        pkicms_err("pre package check fail, preamble: 0x%08x err\n", image_head->preamble);
        return SOC_VERIFY_FAILED;
    }

    if (image_head->head_magic != BOOT_MAGIC_CODE) {
        pkicms_err("pre package check fail, magic: 0x%08x err\n", image_head->head_magic);
        return SOC_VERIFY_FAILED;
    }

    /* 2. Cal code Hash and cmp it to hash record in header */
    if ((image_head->code_len == 0) || (image_head->code_len > MAX_IMAGE_SIZE) ||
        (image_head->code_len != (size - GENERAL_IMAGE_OFFSET))) {
        pkicms_err("pre package check fail, code len: %u, size: %u, err\n", image_head->code_len, size);
        return SOC_VERIFY_FAILED;
    }
    ret = hash_cal_soc(image_position, image_head->code_len, digest, ESBC_HASH_SIZE);
    if (ret != 0) {
        pkicms_err("pre package check fail, hash cal err: %d\n", ret);
        return SOC_VERIFY_FAILED;
    }
    ret = memcmp(digest, image_head->src_hash, ESBC_HASH_SIZE);
    if (ret != 0) {
        pkicms_err("pre package check fail, hash check err: %d\n", ret);
        return SOC_VERIFY_FAILED;
    }
    return SOC_VERIFY_SUCCESS;
}

STATIC s32 copy_pub_key(u8 *pub_key, u32 pub_key_len, const esbc_header_t *image_head)
{
    s32 ret;
    u32 key_len = pub_key_len >> 1;

    ret = memcpy_s(pub_key, pub_key_len, image_head->root_pub_key, key_len);
    if (ret != 0) {
        pkicms_err("cpy pub key len: %u, err: %d\n", key_len, ret);
        return SOC_VERIFY_FAILED;
    }

    ret = memcpy_s(pub_key + key_len, key_len, image_head->root_pub_key_e, key_len);
    if (ret != 0) {
        pkicms_err("cpy pub key-e len: %u, err: %d\n", key_len, ret);
        return SOC_VERIFY_FAILED;
    }
    return SOC_VERIFY_SUCCESS;
}

STATIC u32 efuse_hash_check(u32 dev_id, u8 *pub_key, u32 pub_key_len, efuse_info_t *ef)
{
    u32 i;
    s32 ret;
    u8 tmp;
    u32 reverse_len = EFUSE_HASH_LEN >> 1;
    u64 offset = EFUSE_ROTPK1_OFFSET;
    u32 efuse_hash[EFUSE_HASH_WORDS] = {0};
    u8 hash_array[EFUSE_HASH_LEN] = {0};

    /* get efuse hash info */
    for (i = 0; i < EFUSE_HASH_WORDS; i++) {
        ret = pkicms_read_efuse_reg(dev_id, offset + i * sizeof(u32), efuse_hash + i);
        if (ret != 0) {
            pkicms_err("Ef hash check fail, read err. (dev=%u; ret=%d)\n", dev_id, ret);
            return SOC_VERIFY_FAILED;
        }
    }
    // check if ROTPK is all zero
    ret = memcmp(hash_array, efuse_hash, EFUSE_HASH_LEN);
    if (ret == 0) {
        pkicms_err("Ef read hash check fail, rotpk all zero. (dev=%u)\n", dev_id);
        return SOC_VERIFY_FAILED;
    }

    // Reverse the rotpk data
    for (i = 0; i < reverse_len; i++) {
        tmp = *((u8 *)efuse_hash + i);
        *((u8 *)efuse_hash + i) = *((u8 *)efuse_hash + EFUSE_HASH_LEN - i - 1);
        *((u8 *)efuse_hash + EFUSE_HASH_LEN - i - 1) = tmp;
    }

    ret = hash_cal_soc(pub_key, pub_key_len, hash_array, EFUSE_HASH_LEN);
    if (ret) {
        pkicms_err("Ef hash check fail, hash cal err. (dev=%u; ret=%d)\n", dev_id, ret);
        return SOC_VERIFY_FAILED;
    }

    ret = memcmp((u8 *)efuse_hash, hash_array, EFUSE_HASH_LEN);
    if (ret != 0) {
        ef->hw_hash_check = CHECK_FAIL;
        pkicms_info("Ef read hash check abnormal, hash array not match. (dev=%u)\n", dev_id);
    } else {
        ef->hw_hash_check = CHECK_SUCCESS;
        pkicms_info("Ef read hash check succ. (dev=%u)\n", dev_id);
    }

    return ret;
}

STATIC s32 efuse_read_and_hash_check(u32 dev_id, u8 *pub_key, u32 pub_key_len, efuse_info_t *ef)
{
    s32 ret;
    u32 out_value = 0;
#ifndef CFG_ENV_ESL
#ifdef CFG_SOC_PLATFORM_CLOUD_V2
    u32 i;
    u64 ns_forbid = 0;
    u32 *efuse_value = NULL;

    efuse_value = (u32 *)&ns_forbid;
    for (i = 0; i < (sizeof(u64) / sizeof(u32)); i++) {
        ret = pkicms_read_efuse_reg(dev_id, EFUSE_NS_FORBID_OFFSET + i * sizeof(u32), efuse_value + i);
        if (ret != 0) {
            pkicms_err("Ef read hash check fail, ns forbid err. (dev_id=%u; ret=%d)\n", dev_id, ret);
            return SOC_VERIFY_FAILED;
        }
    }
    out_value = (u32)(ns_forbid >> NS_FORBID_BIT_OFFSET);
#else

    ret = pkicms_read_efuse_reg(dev_id, EFUSE_NS_FORBID_OFFSET, &out_value);
    if (ret != 0) {
        pkicms_err("Ef read hash check fail, ns forbid err. (dev=%u; ret=%d)\n", dev_id, ret);
        return SOC_VERIFY_FAILED;
    }
#endif
#endif
    /* The read data of the secure boot enable flag is 0, indicating that the signature does not need to be verified. */
    if (out_value == 0) {
        ef->hw_hash_check = CHECK_BLANK;
        pkicms_info("Ef read hash check succ, ns forbid blank. (dev=%u)\n", dev_id);
        return SOC_VERIFY_SUCCESS;
    }

    // check ROTPK
    ret = efuse_hash_check(dev_id, pub_key, pub_key_len, ef);
    if (ret) {
        pkicms_err("Ef read hash check fail, hash check err. (dev=%u; ret=%d)\n", dev_id, ret);
        return SOC_VERIFY_FAILED;
    }

    // read SUBKEY CATEGORY 1
    ret = pkicms_read_efuse_reg(dev_id, EFUSE_HW_CATEGORY_OFFSET, &out_value);
    if (ret) {
        pkicms_err("Ef read hash check fail, ef category err. (dev=%u; ret=%d)\n", dev_id, ret);
        return SOC_VERIFY_FAILED;
    }

    ef->hw_category = out_value;

    // read SUBKEY ID MASK 1
    ret = pkicms_read_efuse_reg(dev_id, EFUSE_SUBKEYID_MASK, &out_value);
    if (ret) {
        pkicms_err("Ef read hash check fail, ef sub key m1 err. (dev=%u; ret=%d)\n", dev_id, ret);
        return SOC_VERIFY_FAILED;
    }

    ef->hw_sub_key_id = out_value;
    if (g_nvcnt_value[dev_id] != INVALID_NVCNT) {
        ef->hw_l2_nv_cnt = g_nvcnt_value[dev_id];
    } else {
        pkicms_err("Nvcnt value is invalid. (dev_id=%u)\n", dev_id);
        return SOC_VERIFY_FAILED;
    }

    return SOC_VERIFY_SUCCESS;
}

STATIC s32 get_efuse_info(u32 dev_id, esbc_header_t *image_head, efuse_info_t *ef)
{
    s32 ret;
    u8 pub_key[ESBC_ROOTKEY_LEN << 1] = {0};

    // Bonding the public key of ROT1 and ROT2
    // pub_key: 0: root_pub_key, ESBC_ROOTKEY_LEN: root_pub_key_e
    ret = copy_pub_key(pub_key, ESBC_ROOTKEY_LEN << 1, image_head);
    if (ret != 0) {
        pkicms_err("dev(%u) get ef info fail, cpy pub key err: %d\n", dev_id, ret);
        return SOC_VERIFY_FAILED;
    }

    // Check if the hash of the public key matches and read the efuse flags
    ret = efuse_read_and_hash_check(dev_id, pub_key, ESBC_ROOTKEY_LEN << 1, ef);
    if (ret != 0) {
        pkicms_err("dev(%u) get ef info fail, hash check err: %d\n", dev_id, ret);
        return SOC_VERIFY_FAILED;
    }
    return SOC_VERIFY_SUCCESS;
}

s32 pkicms_get_chip_info(void)
{
    void __iomem *reg_addr = NULL;
    int reg_value;
    int chip_type;
#ifdef CFG_SOC_PLATFORM_MDC_LITE_ESL
    // esl don't support tee
    pkicms_err("sec_read_nsforbid error, esl don't support tee\n");
    chip_type = CHIP_TYPE_BS9SX1A;
    return chip_type;
#endif

    reg_addr = ioremap(SOC_CHIP_INFO_REG_BASE, SOC_CHIP_INFO_MAP_SIZE);
    if (reg_addr == NULL) {
        pkicms_err("devm_ioremap failed.\n");
        return -1;
    }

    reg_value = readl(reg_addr + SOC_CHIP_INFO_REG_OFFSET);
    iounmap(reg_addr);
    chip_type = reg_value & CHIP_TYPE_REG_VALUE_MASK;
    pkicms_info("get chip_type success, chip_type = %d.\n", chip_type);
    return chip_type;
}

s32 soc_verify(u32 dev_id, s32 img_id, u8 *image_head_base, u32 size)
{
    s32 ret;
    int sign = PKCS_SIGN_TYPE_ON;
    efuse_info_t ef = {0};
    u8 *image_position;
    esbc_header_t *image_head;

#ifdef CFG_FEATURE_NOT_SUPPORT_SOC_VERIFY
    pkicms_info("1910B not support soc verify\n");
    return SOC_VERIFY_FAILED;
#endif

    if ((dev_id >= DEVICE_NUM_MAX) || (image_head_base == NULL) || (size < GENERAL_IMAGE_OFFSET) ||
        (size > MAX_IMAGE_SIZE)) {
        pkicms_err(
            "Parameter check error. (dev_id=%d; image_head_base is null %d; len=%u)\n",
            dev_id, image_head_base == NULL, size);
        return SOC_VERIFY_FAILED;
    }

    image_position = image_head_base + GENERAL_IMAGE_OFFSET;
    image_head = (esbc_header_t *)(uintptr_t)image_head_base;

    ret = pre_package_check(image_head, image_position, size);
    if (ret != 0) {
        pkicms_err("Soc verify fail, pre pak check error. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return SOC_VERIFY_FAILED;
    }

#ifdef CFG_SOC_PLATFORM_MDC_V51
    ret = pkicms_get_chip_info();
    if (ret == CHIP_TYPE_BS9SX1A) {
        pkicms_err("failed! not support the chip type\n");
        return SOC_VERIFY_FAILED;
    }
#endif
    ret = get_efuse_info(dev_id, image_head, &ef);
    if (ret != 0) {
        pkicms_err("Soc verify fail, get ef info error. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return SOC_VERIFY_FAILED;
    }

    /* If the EFUSE is not burnt, this branch is used. */
    if (ef.hw_hash_check == CHECK_BLANK) {
        pkicms_info("Soc verify check ok, ef compare position is blank, verify pass. (dev_id=%u)\n", dev_id);
        return SOC_VERIFY_SUCCESS;
    }
#ifdef CFG_FEATURE_PSS_SIGN
    ret = devdrv_config_get_pss_cfg(0, &sign);
    if (ret) {
        pkicms_err("Failed to invoke the devdrv_config_get_pss_cfg. (dev_id=%d; ret=%d)\n", dev_id, ret);
        return SOC_VERIFY_FAILED;
    }
#endif

    ret = image_secure_auth(image_head, &ef, sign);
    if (ret != 0) {
        pkicms_err("Soc verify fail, image sec auth error. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return SOC_VERIFY_FAILED;
    }
    ret = pkicms_ver_check(VERIFY_TYPE_SOC, img_id, (const img_ver_header *)image_position, ef.hw_l2_nv_cnt);
    if (ret != 0) {
        pkicms_err("Soc verify fail, image check error. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return SOC_VERIFY_FAILED;
    }

    pkicms_info("Soc verify success. (dev_id=%u)\n", dev_id);
    return SOC_VERIFY_SUCCESS;
}
EXPORT_SYMBOL(soc_verify);

#ifdef CFG_SOC_PLATFORM_MINIV2
STATIC s32 soc_verify_get_boot_cfg(u32 *boot_cfg)
{
    void __iomem *regs = NULL;

    regs = ioremap((unsigned long)SC_PAD_INFO_BASE, SC_PAD_INFO_PAGE_SIZE);
    if (regs == NULL) {
        pkicms_err("Remap page for boot cfg register failed.\n");
        return SOC_VERIFY_FAILED;
    }

    *boot_cfg = readl((void __iomem *)(uintptr_t)((long)(uintptr_t)regs + SC_PAD_INFO_OFFSET));
    (void)iounmap(regs);
    regs = NULL;

    *boot_cfg &= BOOT_CFG_MASK;

    return SOC_VERIFY_SUCCESS;
}
#endif

STATIC s32 soc_verify_get_nvcnt_thread(void *arg)
{
    s32 ret, i;
    u32 device_num;

#ifdef CFG_SOC_PLATFORM_MINIV2
    u32 boot_cfg = 0;
    soc_verify_get_boot_cfg(&boot_cfg);
    if (boot_cfg != 0) {
        device_num = DEVICE_NUM_MAX_2P;
    } else {
        device_num = DEVICE_NUM_MAX_1P;
    }
#else
    device_num = DEVICE_NUM_MAX;
#endif

    pkicms_info("Get nvcnt thread in\n");

    for (i = 0; i < device_num; i++) {
        ret = soc_get_nvcnt(i, &g_nvcnt_value[i], sizeof(u32));
        if (ret != 0) {
            pkicms_warn("Do not get valid nvcnt. (dev_id=%u; ret=%d)\n", i, ret);
            g_nvcnt_value[i] = INVALID_NVCNT;
        }

        pkicms_info("Get nvcnt success. (dev_id=%u; nvcnt=0x%x)\n", i, g_nvcnt_value[i]);
    }

    up(&g_nvcnt_sema);
    return SOC_VERIFY_SUCCESS;
}

s32 soc_verify_init(void)
{
    s32 ret;
    struct task_struct *read_nvcnt_task = NULL;

    sema_init(&g_nvcnt_sema, 0);
    read_nvcnt_task = kthread_create(soc_verify_get_nvcnt_thread, NULL, "soc_verify_kthread");
    if (IS_ERR(read_nvcnt_task) || (read_nvcnt_task == NULL)) {
        pkicms_err("Create thread for get nvcnt failed.\n");
        return SOC_VERIFY_FAILED;
    } else {
#ifdef CFG_SOC_PLATFORM_MDC_V51
        kthread_bind_to_ctrl_cpu(read_nvcnt_task);
#endif
        (void)wake_up_process(read_nvcnt_task);
        ret = down_timeout(&g_nvcnt_sema, msecs_to_jiffies(NVCNT_WAIT_TIMEOUT));
        if (ret != 0) {
            pkicms_err("Get nvcnt timeout\n");
            return ret;
        }
    }

    pkicms_info("Soc verify init success\n");
    return SOC_VERIFY_SUCCESS;
}
