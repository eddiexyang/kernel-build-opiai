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

#include "cmscbb.h"

#include "linux/securec.h"
#include <linux/string.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/mpi.h>

#ifdef STATIC_SKIP
#define STATIC
#else
#define STATIC static
#endif

static CMSCBB_ERROR_CODE MemRegionCheck(const OEM_MEM_HEADER *mem_head)
{
    if (mem_head->Type != ITEE_CMS_TYPE_MAGIC) {
        return CMSCBB_ERR_CONTEXT_INVALID_PARAM;
    }

    if (mem_head->StartAddr != ITEE_CMS_SADDR_MAGIC) {
        return CMSCBB_ERR_CONTEXT_INVALID_PARAM;
    }

    if (mem_head->Used != OEM_MEM_USED_MAGIC) {
        pkicms_err("cmscbb free used magic wrong!\n");
        return CMSCBB_ERR_CONTEXT_INVALID_PARAM;
    }

    if (mem_head->Magic != OEM_MEM_MANAGE_MAGIC) {
        pkicms_err("cmscbb free, magic wrong!\n");
        return CMSCBB_ERR_CONTEXT_INVALID_PARAM;
    }

    return CVB_SUCCESS;
}

/*******************************************************************
 * Prototype    : CmscbbFree
 * Return Value : void
 *******************************************************************/
CVB_VOID CmscbbFree(CVB_VOID *ptr)
{
    OEM_MEM_HEADER *mem_head = NULL;

    if (ptr == NULL) {
        pkicms_err("cmscbb free, params null!\n");
        return;
    }

    mem_head = (OEM_MEM_HEADER *)((CVB_UINT64)(uintptr_t)ptr - sizeof(OEM_MEM_HEADER));
    if (MemRegionCheck(mem_head) != CVB_SUCCESS) {
        pkicms_warn("cmscbb free, head info is not valid\n");
        return;
    }

    kfree(mem_head);
}

/*******************************************************************
 * Prototype    : CmscbbMalloc
 * Return Value : CMSCBB_ERROR_CODE
 *******************************************************************/
CMSCBB_ERROR_CODE CmscbbMalloc(CVB_VOID **pp_byte, CVB_SIZE_T size)
{
    CVB_SIZE_T malloc_size = size;
    OEM_MEM_HEADER *mem_head = NULL;
    CVB_VOID *ptr = NULL;

    if (size == 0 || (pp_byte == NULL)) {
        pkicms_err("cmscbb malloc, input param wrong!\n");
        return CMSCBB_ERR_CONTEXT_INVALID_PARAM;
    }
    /* add mem header info for mallocing memory */
    malloc_size += sizeof(OEM_MEM_HEADER);
    ptr = kmalloc(malloc_size, __GFP_ACCOUNT);
    if (ptr == NULL) {
        pkicms_err("cmscbb malloc failed!\n");
        return CMSCBB_ERR_SYS_MEM_ALLOC;
    }

    if (memset_s((void *)ptr, malloc_size, 0x0, malloc_size) != 0) {
        pkicms_err("malloc block, memset failed!\n");
        CmscbbFree(ptr);
        *pp_byte = NULL;
        return CMSCBB_ERR_SYS_MEM_SET;
    }

    mem_head = (OEM_MEM_HEADER *)ptr;
    mem_head->Magic = OEM_MEM_MANAGE_MAGIC;
    mem_head->Length = size;
    mem_head->StartAddr = ITEE_CMS_SADDR_MAGIC;
    mem_head->Used = OEM_MEM_USED_MAGIC;
    mem_head->Type = ITEE_CMS_TYPE_MAGIC;

    *pp_byte = ptr + sizeof(OEM_MEM_HEADER);
    return CVB_SUCCESS;
}

/*******************************************************
 * Prototype    : CmscbbMdCreateCtx
 * Description  : create message digest context
 *******************************************************/
CMSCBB_ERROR_CODE CmscbbMdCreateCtx(CMSCBB_CRYPTO_MD_CTX *md_ctx)
{
    CMSCBB_ERROR_CODE ret;
    TA_CRYPTO_MD *pmd = NULL;

    if (md_ctx == NULL) {
        return CMSCBB_ERR_CONTEXT_INVALID_PARAM;
    }

    ret = CmscbbMalloc((void **)&pmd, sizeof(TA_CRYPTO_MD));
    if (ret != CVB_SUCCESS) {
        pkicms_err("cmscbb_createCtx, malloc failed!\n");
        return CMSCBB_ERR_SYS_MEM_ALLOC;
    }

    if (memset_s(pmd, sizeof(TA_CRYPTO_MD), 0, sizeof(TA_CRYPTO_MD)) != 0) {
        pkicms_err("cmscbb_createCtx, memset failed!\n");
        CmscbbFree(pmd);
        return CMSCBB_ERR_SYS_MEM_SET;
    }

    *md_ctx = (CMSCBB_CRYPTO_MD_CTX)pmd;
    return CVB_SUCCESS;
}

/****************************************************
 * Prototype    : CmscbbMdInit
 * Description  : init message digest context
 *****************************************************/
CMSCBB_ERROR_CODE CmscbbMdInit(CMSCBB_CRYPTO_MD_CTX md_ctx, CVB_UINT32 hash_id)
{
    TA_CRYPTO_MD *pmd = (TA_CRYPTO_MD *)md_ctx;

    if (pmd == NULL) {
        pkicms_err("cmscbb mdinit, md invalid!\n");
        return CMSCBB_ERR_CONTEXT_INVALID_PARAM;
    }

    pmd->hash_algo = hash_id;
    switch (pmd->hash_algo) {
        case CMSCBB_HASH_SHA256:
            if (pkicms_sha256_init(&pmd->op)) {
                return CMSCBB_ERR_PKI_CRYPTO_DIGEST_INIT;
            }
            return CVB_SUCCESS;

        case CMSCBB_HASH_SHA384:
        /* fall-through */
        case CMSCBB_HASH_SHA512:
        /* fall-through */
        default:
            pkicms_err("cmscbb mdinit, algo = 0x%x invalid!\n", (int)pmd->hash_algo);
            return CMSCBB_ERR_UNDEFINED;
    }
}

/******************************************************
 * Prototype    : CmscbbMdUpdate
 * Description  : calculate message digest segmently
 *******************************************************/
CMSCBB_ERROR_CODE CmscbbMdUpdate(CMSCBB_CRYPTO_MD_CTX md_ctx, const CVB_BYTE *data, CVB_UINT32 len)
{
    TA_CRYPTO_MD *pmd = (TA_CRYPTO_MD *)md_ctx;

    if (pmd == NULL) {
        pkicms_err("cmscbb mdupdate, md is null!\n");
        return CMSCBB_ERR_CONTEXT_INVALID_PARAM;
    }

    if ((data == NULL && len != 0) || (data != NULL && len == 0)) {
        pkicms_err("cmscbb mdupdate, data or len invalid!\n");
        return CMSCBB_ERR_CONTEXT_INVALID_PARAM;
    }

    switch (pmd->hash_algo) {
        case CMSCBB_HASH_SHA256:
            if (pkicms_sha256_update(&pmd->op, data, len)) {
                return CMSCBB_ERR_PKI_CRYPTO_DIGEST_UPDATE;
            }
            break;
        case CMSCBB_HASH_SHA384:
        /* fall-through */
        case CMSCBB_HASH_SHA512:
        /* fall-through */
        default:
            pkicms_err("cmscbb mdupdate, algo = 0x%x invalid!\n", (int)pmd->hash_algo);
            return CMSCBB_ERR_UNDEFINED;
    }

    return CVB_SUCCESS;
}

/************************************************
 * Prototype    : CmscbbMdFinal
 * Description  : get the final message digest
 *************************************************/
CMSCBB_ERROR_CODE CmscbbMdFinal(
    CMSCBB_CRYPTO_MD_CTX md_ctx, CVB_BYTE *digest, CVB_UINT32 *len, const CVB_UINT32 *digest_max_len)
{
    TA_CRYPTO_MD *pmd = (TA_CRYPTO_MD *)md_ctx;
    u32 res;

    if (pmd == NULL) {
        pkicms_err("cmscbb_mdFin, md_ctx is null!\n");
        return CMSCBB_ERR_CONTEXT_INVALID_PARAM;
    }

    if ((digest == NULL) || (len == NULL) || (digest_max_len == NULL)) {
        pkicms_err("cmscbb_mdFin, digest is null!\n");
        return CMSCBB_ERR_CONTEXT_INVALID_PARAM;
    }

    switch (pmd->hash_algo) {
        case CMSCBB_HASH_SHA256:
            *len = DIGEST_SIZE_SHA256;
            break;
        case CMSCBB_HASH_SHA384:
        /* fall-through */
        case CMSCBB_HASH_SHA512:
        /* fall-through */
        default:
            pkicms_err("cmscbb_mdFin, inv alg!\n");
            return CMSCBB_ERR_UNDEFINED;
    }

    res = pkicms_sha256_final(&pmd->op, digest, len);
    if (res != 0) {
        pkicms_err("TEE_DigestDoFinal failed, error code is 0x%x.\n", res);
        return CMSCBB_ERR_PKI_CRYPTO_DIGEST_FINAL;
    }

    return CVB_SUCCESS;
}

/*****************************************
 * Prototype    : CmscbbMdDestoryCtx
 * Description  : destroy context
 ******************************************/
CVB_VOID CmscbbMdDestoryCtx(CMSCBB_CRYPTO_MD_CTX md_ctx)
{
    TA_CRYPTO_MD *pmd = (TA_CRYPTO_MD *)md_ctx;

    if (pmd == NULL) {
        pkicms_err("cmscbb mdDestrCtx, param is null!\n");
        return;
    }

    CmscbbFree(pmd);
}

/************************************************
 * Prototype    : CmscbbCryptoVerifyCreateCtx
 * Description  : create RSA verification context
 **************************************************/
CMSCBB_ERROR_CODE CmscbbCryptoVerifyCreateCtx(CMSCBB_CRYPTO_VRF_CTX *ctx)
{
    CMSCBB_ERROR_CODE ret;
    TA_CRYPTO_VRF *pvrf = NULL;
    CMSCBB_CRYPTO_MD_CTX md_ctx = NULL;

    if (ctx == NULL) {
        pkicms_err("ccvcc param wrong!\n");
        return CMSCBB_ERR_CONTEXT_INVALID_PARAM;
    }

    /* malloc RSA verification memory */
    ret = CmscbbMalloc((void **)&pvrf, sizeof(TA_CRYPTO_VRF));
    if (pvrf == NULL) {
        pkicms_err("ccvcc createCtx failed!\n");
        return ret;
    }

    if (memset_s((void *)pvrf, sizeof(TA_CRYPTO_VRF), 0, sizeof(TA_CRYPTO_VRF)) != 0) {
        pkicms_err("ccvcc mem_s failed!\n");
        CmscbbFree(pvrf);
        return CMSCBB_ERR_SYS_MEM_SET;
    }

    /* create context */
    ret = CmscbbMdCreateCtx(&md_ctx);
    if (ret != CVB_SUCCESS) {
        CmscbbFree(pvrf);
        return ret;
    }

    pvrf->md_ctx = (TA_CRYPTO_MD *)md_ctx;
    *ctx = (CMSCBB_CRYPTO_VRF_CTX)pvrf;

    return CVB_SUCCESS;
}

/*******************************************************************************
 * Prototype    : CmscbbCryptoVerifyInit
 * Description  : init RSA verification
 *******************************************************************************/
#if CMSCBB_SUPPORT_RSAPSS
CMSCBB_ERROR_CODE CmscbbCryptoVerifyInit(CMSCBB_CRYPTO_VRF_CTX vrf_ctx, CmscbbKeyAndAlgInfo *info)
{
    TA_CRYPTO_VRF *rsa_ctx = (TA_CRYPTO_VRF *)vrf_ctx;

    if (rsa_ctx == CVB_NULL || info == CVB_NULL) {
        return CMSCBB_ERR_CONTEXT_INVALID_PARAM;
    }

    if (info->e == CVB_NULL || info->n == CVB_NULL) {
        return CMSCBB_ERR_CONTEXT_INVALID_PARAM;
    }

    if (info->e->uiLength == 0 || info->n->uiLength == 0) {
        return CMSCBB_ERR_CONTEXT_INVALID_PARAM;
    }

    if (CVB_SUCCESS != CmscbbMdInit((CMSCBB_CRYPTO_MD_CTX)rsa_ctx->md_ctx, info->hashAlg)) {
        return CMSCBB_ERR_SYS_MEM_ALLOC;
    }

    rsa_ctx->ekey = mpi_read_raw_data(info->e->aVal, info->e->uiLength);
    if (rsa_ctx->ekey == NULL) {
        return CMSCBB_ERR_CONTEXT_INVALID_PARAM;
    }

    rsa_ctx->nkey = mpi_read_raw_data(info->n->aVal, info->n->uiLength);
    if (rsa_ctx->nkey == NULL) {
        mpi_free(rsa_ctx->ekey);
        rsa_ctx->ekey = NULL;
        return CMSCBB_ERR_CONTEXT_INVALID_STRUCT;
    }

    return CVB_SUCCESS;
}
#else
CMSCBB_ERROR_CODE CmscbbCryptoVerifyInit(
    CMSCBB_CRYPTO_VRF_CTX vrf_ctx, const CmscbbBigInt *e, const CmscbbBigInt *n, CVB_UINT32 cmscbb_hashid)
{
    TA_CRYPTO_VRF *rsa_ctx = (TA_CRYPTO_VRF *)vrf_ctx;

    if (rsa_ctx == CVB_NULL || e == CVB_NULL || n == CVB_NULL) {
        return CMSCBB_ERR_CONTEXT_INVALID_PARAM;
    }

    if (CVB_SUCCESS != CmscbbMdInit((CMSCBB_CRYPTO_MD_CTX)rsa_ctx->md_ctx, cmscbb_hashid)) {
        return CMSCBB_ERR_SYS_MEM_ALLOC;
    }

    rsa_ctx->ekey = mpi_read_raw_data(e->aVal, e->uiLength);
    if (rsa_ctx->ekey == NULL) {
        return CMSCBB_ERR_CONTEXT_INVALID_PARAM;
    }

    rsa_ctx->nkey = mpi_read_raw_data(n->aVal, n->uiLength);
    if (rsa_ctx->nkey == NULL) {
        mpi_free(rsa_ctx->ekey);
        rsa_ctx->ekey = NULL;
        return CMSCBB_ERR_CONTEXT_INVALID_STRUCT;
    }

    return CVB_SUCCESS;
}
#endif

/*******************************************************************************
 * Prototype    : CmscbbCryptoVerifyUpdate
 * Return Value : CMSCBB_ERROR_CODE
 *******************************************************************************/
CMSCBB_ERROR_CODE CmscbbCryptoVerifyUpdate(CMSCBB_CRYPTO_VRF_CTX vrf_ctx, const CVB_BYTE *data, CVB_UINT32 len)
{
    CMSCBB_ERROR_CODE ret;
    TA_CRYPTO_VRF *pvrf = (TA_CRYPTO_VRF *)vrf_ctx;

    if ((pvrf == NULL) || (pvrf->md_ctx == NULL)) {
        pkicms_err("cmscbbMdUpdate, param null!\n");
        return CMSCBB_ERR_CONTEXT_INVALID_PARAM;
    }

    if (((data == NULL) && (len != 0)) || ((len == 0) && (data != NULL))) {
        pkicms_err("cmscbbMdUpdate,param wrong!\n");
        CmscbbFree(pvrf->md_ctx);
        return CMSCBB_ERR_CONTEXT_INVALID_PARAM;
    }

    ret = CmscbbMdUpdate(pvrf->md_ctx, data, len);
    if (ret != CVB_SUCCESS) {
        pkicms_err("cmscbbMdUpdate failed!\n");
        CmscbbFree(pvrf->md_ctx);
        return ret;
    }

    return CVB_SUCCESS;
}

static const CVB_BYTE sha256_fill_header[] = {
    0x30, 0x31, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00, 0x04, 0x20};
static const int sha256_header_len = sizeof(sha256_fill_header);

static const CVB_BYTE sha512_fill_header[] = {
    0x30, 0x51, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x03, 0x05, 0x00, 0x04, 0x40};
static const int sha512_header_len = sizeof(sha512_fill_header);

static CMSCBB_ERROR_CODE rsa_verify_v15_sha256_check(CVB_INT nDec, CVB_UINT32 nHashLen, CVB_BYTE *buff,
                                                     CVB_BYTE *pbHash)
{
    if (nDec - sha256_header_len != DIGEST_SIZE_SHA256) {
        return CMSCBB_ERR_PKI_CRYPTO_DIGEST_FINAL;
    }

    if (nHashLen != DIGEST_SIZE_SHA256) {
        return CMSCBB_ERR_PKI_CRYPTO_DIGEST_FINAL;
    }

    /* Check the padding header. */
    if (memcmp(buff, sha256_fill_header, sha256_header_len) != 0) {
        return CMSCBB_ERR_PKI_CRYPTO_DIGEST_FINAL;
    }

    /* Check the hash value. */
    if (memcmp(buff + sha256_header_len, pbHash, DIGEST_SIZE_SHA256) != 0) {
        return CMSCBB_ERR_PKI_CRYPTO_DIGEST_FINAL;
    }
    return CVB_SUCCESS;
}

static CMSCBB_ERROR_CODE rsa_verify_v15_sha512_check(CVB_INT nDec, CVB_UINT32 nHashLen, CVB_BYTE *buff,
                                                     CVB_BYTE *pbHash)
{
    if (nDec - sha512_header_len != DIGEST_SIZE_SHA512) {
        return CMSCBB_ERR_PKI_CRYPTO_DIGEST_FINAL;
    }

    if (nHashLen != DIGEST_SIZE_SHA512) {
        return CMSCBB_ERR_PKI_CRYPTO_DIGEST_FINAL;
    }

    if (memcmp(buff, sha512_fill_header, sha512_header_len) != 0) {
        return CMSCBB_ERR_PKI_CRYPTO_DIGEST_FINAL;
    }

    if (memcmp(buff + sha512_header_len, pbHash, DIGEST_SIZE_SHA512) != 0) {
        return CMSCBB_ERR_PKI_CRYPTO_DIGEST_FINAL;
    }
    return CVB_SUCCESS;
}

static CMSCBB_ERROR_CODE rsa_verify_v15(CVB_BYTE *szDec, CVB_UINT32 len, CVB_UINT32 hash_algo, CVB_BYTE *pbHash,
                                        CVB_UINT32 nHashLen)
{
    CMSCBB_ERROR_CODE ret;
    CVB_BYTE *p = szDec;
    CVB_INT nDec;

    if (*p++ != 0x01) {
        pkicms_err("flags error!\n");
        return CMSCBB_ERR_PKI_CRYPTO_DIGEST_FINAL;
    }

    while (*p == 0xFF) {
        if (p >= szDec + len) {
            return CMSCBB_ERR_PKI_CRYPTO_DIGEST_FINAL;
        }
        ++p;
    }

    /* Check whether the number of 0xFFs is greater than 10 */
    if (p - szDec <= 10) {
        return CMSCBB_ERR_PKI_CRYPTO_DIGEST_FINAL;
    }

    if (*p++ != 0x00) {
        return CMSCBB_ERR_PKI_CRYPTO_DIGEST_FINAL;
    }
    nDec = szDec + len - p; /* Obtain the actual data length. */

    switch (hash_algo) {
        case CMSCBB_HASH_SHA256: {
            /* Check the hash length. */
            ret = rsa_verify_v15_sha256_check(nDec, nHashLen, p, pbHash);
            if (ret != CVB_SUCCESS) {
                return ret;
            }
            break;
        }
        case CMSCBB_HASH_SHA512: {
            ret = rsa_verify_v15_sha512_check(nDec, nHashLen, p, pbHash);
            if (ret != CVB_SUCCESS) {
                return ret;
            }
            break;
        }
        default:
            return CMSCBB_ERR_PKI_CRYPTO_DIGEST_FINAL;
    }
    return CVB_SUCCESS;
}

CMSCBB_ERROR_CODE CmscbbVerifyRsa(
    TA_CRYPTO_VRF *pvrf, const CVB_BYTE *signature, CVB_UINT32 signature_len, u8 *msg_hash, u32 hash_len)
{
    CMSCBB_ERROR_CODE ret;
    int mpi_ret;
    MPI sig, dec;
    unsigned char *buffer = NULL;
    unsigned int nbytes = 0;
    int sign = 0;

    sig = mpi_read_raw_data(signature, signature_len);
    if (sig == NULL) {
        return CMSCBB_ERR_PKI_CMS_VERIFY_FAILED;
    }

    dec = mpi_alloc(0);
    if (dec == NULL) {
        mpi_free(sig);
        return CMSCBB_ERR_PKI_CMS_VERIFY_FAILED;
    }

    mpi_ret = mpi_powm(dec, sig, pvrf->ekey, pvrf->nkey);
    if (mpi_ret < 0) {
        mpi_free(sig);
        mpi_free(dec);
        return CMSCBB_ERR_PKI_CMS_VERIFY_FAILED;
    }

    buffer = mpi_get_buffer(dec, &nbytes, &sign);
    mpi_free(sig);
    mpi_free(dec);

    if (buffer == NULL) {
        pkicms_err("mpi get buffer failed.\n");
        return CMSCBB_ERR_PKI_CMS_SIG_ALGO;
    }

    ret = rsa_verify_v15(buffer, nbytes, pvrf->md_ctx->hash_algo, msg_hash, hash_len);
    kfree(buffer);

    if (ret != CVB_SUCCESS) {
        pkicms_err("TEE Asymmetric Verify Digest failed, error code is 0x%x\n", ret);
        return CMSCBB_ERR_PKI_CMS_VERIFY_FAILED;
    }

    return CVB_SUCCESS;
}

/************************************************
 * Prototype    : CmscbbCryptoVerifyFinal
 * Description  : verify RSA
 ************************************************/
CMSCBB_ERROR_CODE CmscbbCryptoPkcsVerifyFinal(
    CMSCBB_CRYPTO_VRF_CTX vrf_ctx, const CVB_BYTE *signature, CVB_UINT32 len, CVB_INT *r_result)
{
    CMSCBB_ERROR_CODE ret;
    TA_CRYPTO_VRF *pvrf = (TA_CRYPTO_VRF *)vrf_ctx;
    u8 msg_hash[CMSCBB_MAX_DIGEST_SIZE] = {0};
    u32 hash_len = 0;

    if (r_result == NULL) {
        if ((pvrf != NULL) && (pvrf->md_ctx != NULL)) {
            pkicms_err("ccvf, r_result null!\n");
            CmscbbFree(pvrf->md_ctx);
        }
        return CMSCBB_ERR_CONTEXT_INVALID_PARAM;
    }

    if ((pvrf == NULL) || (pvrf->md_ctx == NULL)) {
        pkicms_err("pvrf or pvrf->md_ctx is null!\n");
        *r_result = 0;
        return CMSCBB_ERR_CONTEXT_INVALID_PARAM;
    }

    /* get message original digest */
    ret = CmscbbMdFinal(
        (CMSCBB_CRYPTO_MD_CTX)pvrf->md_ctx, (CVB_BYTE *)msg_hash, (CVB_UINT32 *)&hash_len, (CVB_UINT32 *)&hash_len);
    if (ret != CVB_SUCCESS) {
        *r_result = 0;
        pkicms_err("cmscbb_mdfinal failed!\n");
        return ret;
    }

    ret = CmscbbVerifyRsa(pvrf, signature, len, msg_hash, hash_len);
    *r_result = (ret == CVB_SUCCESS) ? 1 : 0;
    return ret;
}

STATIC void crypto_mem_xor(u8 *scr, u32 scr_len, u8 *dst, u32 dst_len)
{
    u32 i;

    if (dst_len < scr_len) {
        pkicms_err("Details: dst_len < scr_len. (scr_len=%d; dst_len=%d)\n", scr_len, dst_len);
        return;
    }

    for (i = 0; i < scr_len; i++) {
        dst[i] ^= scr[i];
    }
}

STATIC void word2byter(u32 *W, u8 *B, u32 len)
{
    u32 i;

    for (i = 0; i < len; i++) {
        B[W2B_SIZE * i + W2B_OFF0] = (u8)((W[i] >> SHIFT24) & BYTE_MASK);
        B[W2B_SIZE * i + W2B_OFF1] = (u8)((W[i] >> SHIFT16) & BYTE_MASK);
        B[W2B_SIZE * i + W2B_OFF2] = (u8)((W[i] >> SHIFT8) & BYTE_MASK);
        B[W2B_SIZE * i + W2B_OFF3] = (u8)((W[i] >> SHIFT0) & BYTE_MASK);
    }
}

STATIC u32 rsa_pss_check_db(u8 *em_data, u32 hash_len, u32 salt_len, u32 block_len)
{
    u32 i;
    if (em_data[block_len - hash_len - RSA_PSS_TAIL_LEN - salt_len - RSA_PSS_TAIL_LEN] != PSS_PADDING1) {
        return CMSCBB_ERR_PKI_CMS_VERIFY_FAILED;
    }

    for (i = 0; i < block_len - hash_len - RSA_PSS_TAIL_LEN - salt_len - RSA_PSS_TAIL_LEN; i++) {
        if (em_data[i] != 0) {
            return CMSCBB_ERR_PKI_CMS_VERIFY_FAILED;
        }
    }

    return CVB_SUCCESS;
}

STATIC CMSCBB_ERROR_CODE rsa_pss_gen_maskeddb(u8 *em_data, u32 maskeddb_len, CVB_UINT32 hash_algo, CVB_UINT32 hash_len)
{
    u32 ret;
    u32 cnt = 0;
    u8 cnt_buf[sizeof(cnt)] = {0};
    u8 hash_result[SHA512_BLEN] = {0};
    u8 hash_m[DBMASK_DATA_LEN] = {0};
    u32 offset = maskeddb_len;
    u32 dbhash_len;

    if (hash_algo != CMSCBB_HASH_SHA256) {
        return CMSCBB_ERR_PKI_CMS_VERIFY_FAILED;
    }

    while (offset > hash_len) {
        word2byter(&cnt, cnt_buf, 1);
        memset_s(hash_m, sizeof(hash_m), 0, sizeof(hash_m));
        memcpy_s(hash_m, sizeof(hash_m), em_data + maskeddb_len, hash_len);
        memcpy_s(hash_m + hash_len, sizeof(hash_m) - hash_len, cnt_buf, RSA_PSS_CNT_LEN);
        ret = pkicms_sha256_string(hash_m, hash_len + RSA_PSS_CNT_LEN, hash_result, &dbhash_len);
        if (ret != CVB_SUCCESS) {
            return CMSCBB_ERR_PKI_CMS_VERIFY_FAILED;
        }

        cnt++;
        crypto_mem_xor(hash_result, hash_len, em_data + (maskeddb_len - offset), RSA_PSS_4096_BYTES);
        offset = offset - hash_len;
    }

    word2byter(&cnt, cnt_buf, 1);

    memset_s(hash_m, sizeof(hash_m), 0, sizeof(hash_m));
    memcpy_s(hash_m, sizeof(hash_m), em_data + maskeddb_len, hash_len);
    memcpy_s(hash_m + hash_len, sizeof(hash_m) - hash_len, cnt_buf, RSA_PSS_CNT_LEN);
    ret = pkicms_sha256_string(hash_m, hash_len + RSA_PSS_CNT_LEN, hash_result, &dbhash_len);
    if (ret != CVB_SUCCESS) {
        return CMSCBB_ERR_PKI_CMS_VERIFY_FAILED;
    }
    crypto_mem_xor(hash_result, offset, em_data + (maskeddb_len - offset), RSA_PSS_4096_BYTES);
    em_data[0] &= RSA_PSS_HEAD_MASK;

    return CVB_SUCCESS;
}

STATIC CMSCBB_ERROR_CODE rsa_verify_pss(
    CVB_BYTE *szDec, CVB_UINT32 len, CVB_UINT32 hash_algo, CVB_BYTE *pbHash, CVB_UINT32 nHashLen, u32 salt_len)
{
    CMSCBB_ERROR_CODE ret;
    u8 em_data[RSA_PSS_4096_BYTES] = {0};
    u32 maskeddb_len = len - nHashLen - RSA_PSS_TAIL_LEN;
    u8 hash_m[DBMASK_DATA_LEN] = {0};
    u8 hash_result[SHA512_BLEN] = {0};
    u32 salt_offset = len - RSA_PSS_TAIL_LEN - nHashLen - salt_len;
    u32 mhash_offset = len - RSA_PSS_TAIL_LEN - nHashLen;
    u32 hash_len = 0;

    pkicms_info("rsa_verify_pss (len = %u, nHashLen = %u, salt_len = %u)\n", len, nHashLen, salt_len);
    if ((nHashLen > DIGEST_SIZE_SHA512) || (salt_len >= PASS_SLEN_MASK)) {
        pkicms_err("nHashLen or salt_len out of range error! (nHashLen = %u, salt_len = %u) \n", nHashLen, salt_len);
        return CMSCBB_ERR_PKI_CRYPTO_DIGEST_FINAL;
    }

    if ((len < RSA_PSS_TAIL_LEN + nHashLen + salt_len) ||
        (len > RSA_PSS_4096_BYTES - DIGEST_SIZE_SHA256 + RSA_PSS_TAIL_LEN + nHashLen)) {
        pkicms_err("salt_offset or mhash_offset out of range error! \n");
        return CMSCBB_ERR_PKI_CRYPTO_DIGEST_FINAL;
    }

    if (memcpy_s((void *)em_data, RSA_PSS_4096_BYTES, (const void *)szDec, len) != 0) {
        return CMSCBB_ERR_PKI_CRYPTO_DIGEST_FINAL;
    }

    if (em_data[len - RSA_PSS_TAIL_LEN] != RSA_PSS_TAIL_V) {
        pkicms_err("rsa pss verify flag error! \n");
        return CMSCBB_ERR_PKI_CRYPTO_DIGEST_FINAL;
    }

    ret = rsa_pss_gen_maskeddb(em_data, maskeddb_len, hash_algo, nHashLen);
    if (ret != CVB_SUCCESS) {
        pkicms_err("rsa pss gen maskedDb error! \n");
        return CMSCBB_ERR_PKI_CRYPTO_DIGEST_FINAL;
    }

    ret = rsa_pss_check_db(em_data, nHashLen, salt_len, len);
    if (ret != CVB_SUCCESS) {
        pkicms_err("rsa pss check db error! \n");
        return CMSCBB_ERR_PKI_CRYPTO_DIGEST_FINAL;
    }

    memset_s(hash_m, sizeof(hash_m), 0, sizeof(hash_m));
    memcpy_s(hash_m + RSA_PSS_P1_LEN, sizeof(hash_m) - RSA_PSS_P1_LEN, pbHash, nHashLen);
    memcpy_s(hash_m + RSA_PSS_P1_LEN + nHashLen,
        sizeof(hash_m) - RSA_PSS_P1_LEN - nHashLen,
        em_data + salt_offset,
        salt_len);
    ret = pkicms_sha256_string(hash_m, RSA_PSS_P1_LEN + nHashLen + salt_len, hash_result, &hash_len);
    if (ret != CVB_SUCCESS) {
        return CMSCBB_ERR_PKI_CRYPTO_DIGEST_FINAL;
    }

    if (memcmp(hash_result, em_data + mhash_offset, DIGEST_SIZE_SHA256) != 0) {
        return CMSCBB_ERR_PKI_CRYPTO_DIGEST_FINAL;
    }

    return CVB_SUCCESS;
}

/*****************
 * This function is copied from the "mpi_get_buffer" function in Linux mpi lib,
 * but the operation is removed when the value of A is 0.
 */
#define PKICMS_BYTE_TO_BIT 8
void *pkicms_rsa_get_buffer(MPI a, unsigned int *nbytes, int *sign)
{
    uint8_t *p = NULL;
    uint8_t *buffer = NULL;
    mpi_limb_t alimb;
    int i, j;
    unsigned int n;

    if (sign != NULL) {
        *sign = a->sign;
    }

    *nbytes = n = a->nlimbs * BYTES_PER_MPI_LIMB;
    if (n == 0) {
        n++;    /* avoid zero length allocation */
    }

    p = buffer = kmalloc(n, GFP_KERNEL | __GFP_ACCOUNT);
    if (p == NULL) {
        return NULL;
    }

    for (i = a->nlimbs - 1; i >= 0; i--) {
        alimb = a->d[i];
        for (j = BYTES_PER_MPI_LIMB - 1; j >= 0; j--) {
            *p++ = alimb >> (j * PKICMS_BYTE_TO_BIT);
        }
    }
    return buffer;
}

/************************************************
 * Prototype    : CmscbbCryptoPssVerifyFinal
 * Description  : verify RSA pss mode
 ************************************************/
CMSCBB_ERROR_CODE CmscbbCryptoPssVerifyFinal(
    CMSCBB_CRYPTO_VRF_CTX vrf_ctx, const CVB_BYTE *signature, CVB_UINT32 len, CVB_INT *r_result, u32 salt_len)
{
    CMSCBB_ERROR_CODE ret;
    int mpi_ret;
    TA_CRYPTO_VRF *pvrf = (TA_CRYPTO_VRF *)vrf_ctx;
    MPI sig, dec;
    u8 msg_hash[CMSCBB_MAX_DIGEST_SIZE] = {0};
    u32 hash_len = 0;
    unsigned char *buffer = NULL;
    unsigned int nbytes = 0;
    int sign = 0;

    if (r_result == NULL) {
        if ((pvrf != NULL) && (pvrf->md_ctx != NULL)) {
            pkicms_err("ccvf, r_result null!\n");
            CmscbbFree(pvrf->md_ctx);
        }
        return CMSCBB_ERR_CONTEXT_INVALID_PARAM;
    }

    if ((pvrf == NULL) || (pvrf->md_ctx == NULL)) {
        pkicms_err("pvrf or pvrf->md_ctx is null!\n");
        *r_result = 0;
        return CMSCBB_ERR_CONTEXT_INVALID_PARAM;
    }

    /* get message original digest */
    ret = CmscbbMdFinal(
        (CMSCBB_CRYPTO_MD_CTX)pvrf->md_ctx, (CVB_BYTE *)msg_hash, (CVB_UINT32 *)&hash_len, (CVB_UINT32 *)&hash_len);
    if (ret != CVB_SUCCESS) {
        *r_result = 0;
        pkicms_err("cmscbb_mdfinal failed!\n");
        return ret;
    }

    sig = mpi_read_raw_data(signature, len);
    if (sig == NULL) {
        *r_result = 0;
        return CMSCBB_ERR_PKI_CMS_VERIFY_FAILED;
    }

    dec = mpi_alloc(0);
    if (dec == NULL) {
        mpi_free(sig);
        *r_result = 0;
        return CMSCBB_ERR_PKI_CMS_VERIFY_FAILED;
    }

    mpi_ret = mpi_powm(dec, sig, pvrf->ekey, pvrf->nkey);
    if (mpi_ret < 0) {
        mpi_free(sig);
        mpi_free(dec);
        *r_result = 0;
        return CMSCBB_ERR_PKI_CMS_VERIFY_FAILED;
    }

    buffer = pkicms_rsa_get_buffer(dec, &nbytes, &sign);
    mpi_free(sig);
    mpi_free(dec);

    if (buffer == NULL) {
        pkicms_err("mpi get buffer failed.\n");
        *r_result = 0;
        return CMSCBB_ERR_PKI_CMS_SIG_ALGO;
    }

    ret = rsa_verify_pss(buffer, nbytes, pvrf->md_ctx->hash_algo, msg_hash, hash_len, salt_len);
    kfree(buffer);

    if (ret != CVB_SUCCESS) {
        pkicms_err("TEE Asymmetric Verify Digest failed, error code is 0x%x\n", ret);
        *r_result = 0;
        return CMSCBB_ERR_PKI_CMS_VERIFY_FAILED;
    }

    *r_result = 1;
    return CVB_SUCCESS;
}

CMSCBB_ERROR_CODE CmscbbCryptoVerifyFinal(
    CMSCBB_CRYPTO_VRF_CTX vrf_ctx, const CVB_BYTE *signature, CVB_UINT32 len, CVB_INT *r_result)
{
#if (defined CFG_SOC_PLATFORM_MINIV2) || (defined CFG_SOC_PLATFORM_CLOUD_V2)
    return CmscbbCryptoPkcsVerifyFinal(vrf_ctx, signature, len, r_result);
#else
    pkicms_sign_alg sign_alg = pkicms_get_rsa_sign_alg_type();
    if (sign_alg == PKICMS_RSA_PKCS1) {
        return CmscbbCryptoPkcsVerifyFinal(vrf_ctx, signature, len, r_result);
    } else {
        u32 salt_len = pkicms_get_rsa_pss_salt_len();
        return CmscbbCryptoPssVerifyFinal(vrf_ctx, signature, len, r_result, salt_len);
    }
#endif
}

/***********************************************
 * Prototype    : CmscbbCryptoVerifyDestroyCtx
 * Description  : clear resource
 ************************************************/
CVB_VOID CmscbbCryptoVerifyDestroyCtx(CMSCBB_CRYPTO_VRF_CTX vrf_ctx)
{
    TA_CRYPTO_VRF *pvrf = (TA_CRYPTO_VRF *)vrf_ctx;

    if (pvrf == NULL) {
        pkicms_err("ccvdc null!\n");
        return;
    }

    CmscbbMdDestoryCtx(pvrf->md_ctx);
    mpi_free(pvrf->ekey);
    pvrf->ekey = NULL;
    mpi_free(pvrf->nkey);
    pvrf->nkey = NULL;
    CmscbbFree(pvrf);
}

CVB_UINT32 CmscbbStrlen(const CVB_CHAR *s)
{
    if (s == NULL) {
        pkicms_err("input str is null!\n");
        return 0;
    }

    return (CVB_UINT32)strlen((const char *)s);
}

/*******************************************************************************
 * Prototype    : CmscbbStrNCmp
 * Description  : string compare function
 * Return Value : CVB_INT, 0: s2 equal2 to s1, others: s2 differs to s1
 *******************************************************************************/
CVB_INT CmscbbStrNCmp(const CVB_CHAR *s1, const CVB_CHAR *s2, CVB_SIZE_T n)
{
    size_t len1;
    size_t len2;

    if (s1 == NULL || s2 == NULL) {
        pkicms_err("input str1/str2 is null!\n");
        return -1;
    }

    if (s1 == s2) {
        pkicms_debug("input str1/str2 buff is same!\n");
        return 0;
    }

    len1 = strlen((const char *)s1);
    len2 = strlen((const char *)s2);
    if (len1 != len2) {
        return -1;
    }

    return strncmp(s1, s2, n);
}

CVB_INT CmscbbMemCmp(const CVB_VOID *s1, const CVB_VOID *s2, CVB_SIZE_T n)
{
    if (s1 == NULL || s2 == NULL) {
        pkicms_err("input str1/str2 buff is null!\n");
        return -1;
    }

    return (CVB_INT)memcmp(s1, s2, n);
}

const CVB_CHAR *CmscbbStrStr(const CVB_CHAR *s1, const CVB_CHAR *s2)
{
    const char *p = s1;
    size_t len;

    if (s1 == NULL || s2 == NULL) {
        pkicms_err("input str1/str2 buff is null!\n");
        return NULL;
    }

    len = strlen(s2);
    for (; (p = strchr(p, *s2)) != 0; p++) {
        if (strncmp(p, s2, len) == 0) {
            return (const char *)p;
        }
    }

    return NULL;
}

CVB_CHAR *CmscbbStrChr(const CVB_CHAR *s, CVB_CHAR c)
{
    if (s == NULL) {
        pkicms_err("input str buff is null!\n");
        return NULL;
    }

    return (CVB_CHAR *)strchr(s, c);
}

CVB_INT CmscbbStrCmp(const CVB_CHAR *s1, const CVB_CHAR *s2)
{
    if (s1 == NULL || s2 == NULL) {
        pkicms_err("input str1/str2 is null!\n");
        return -1;
    }

    if (s1 == s2) {
        pkicms_debug("input str1/str2 buff is same!\n");
        return 0;
    }

    return strcmp(s1, s2);
}

CVB_VOID CmscbbLogPrint(CMSCBB_LOG_TYPE log_level, const CVB_CHAR *filename, CVB_INT line, const CVB_CHAR *function,
    CMSCBB_ERROR_CODE rc, const CVB_CHAR *log)
{
    switch (log_level) {
        case CMSCBB_LOG_TYPE_ERROR:
            printk(KERN_ERR "[%s][%s:%d]: errcode=%u, message:%s\n", "cmscbb", function, line, rc, log);
            break;
        case CMSCBB_LOG_TYPE_WARNING:
            printk(KERN_ERR "[%s][%s:%d]: errcode=%u, message:%s\n", "cmscbb", function, line, rc, log);
            break;
        case CMSCBB_LOG_TYPE_INFO:
            printk(KERN_ERR "[%s][%s:%d]: errcode=%u, message:%s\n", "cmscbb", function, line, rc, log);
            break;
        case CMSCBB_LOG_TYPE_DEBUG:
            printk(KERN_ERR "[%s][%s:%d]: errcode=%u, message:%s\n", "cmscbb", function, line, rc, log);
            break;
        default:
            printk(KERN_ERR "[%s][%s:%d]: errcode=%u, message:%s\n", "cmscbb", function, line, rc, log);
            break;
    }
}

#if CMSCBB_SUPPORT_FILE
/*
 * Prototype    : CmscbbFileOpen
 * Description  : Implement File Open
 * Params
 *   [IN] path: File name
 *   [IN] mode: Kind of access that's enabled
 * Return Value : CVB_FILE_HANDLE
 */
CVB_FILE_HANDLE CmscbbFileOpen(const CVB_CHAR *path, const CVB_CHAR *mode)
{
    (void)path;
    (void)mode;
    return CVB_NULL;
}

/*
 * Prototype    : CmscbbFileRead
 * Description  : Implementing file Reads
 * Params
 *   [IN] ptr: Storage location for data
 *   [IN] size: Item size in bytes
 *   [IN] fp: Pointer to FILE structure
 * Return Value : CVB_UINT32
 */
CVB_SIZE_T CmscbbFileRead(CVB_VOID *ptr, CVB_SIZE_T size, CVB_FILE_HANDLE fp)
{
    (void)ptr;
    (void)size;
    (void)fp;

    return 0;
}
/*
 * Prototype    : CmscbbFileClose
 * Description  : Implement file shutdown
 * Params
 *   [IN] fp: Pointer to FILE structure
 * Return Value : int
 */
CMSCBB_ERROR_CODE CmscbbFileClose(CVB_FILE_HANDLE fp)
{
    (void)fp;

    return 0;
}
/*
 * Prototype    : CmscbbFileGetSize
 * Description  : Achieve get file length
 * Params
 *   [IN] fp: Pointer to FILE structure
 * Return Value : CVB_INT64
 */
CVB_UINT64 CmscbbFileGetSize(CVB_FILE_HANDLE fp)
{
    (void)fp;

    return 0;
}
#endif /* CMSCBB_SUPPORT_FILE */
