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

#include <crypto/hash.h>
#include <linux/securec.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/vmalloc.h>
#include <linux/mpi.h>
#include <linux/bitops.h>
#include <linux/sched.h>

#include "hsm_info.h"
#include "hsm_fw_update.h"
#include "soc_verify.h"
#include "pkicms_common.h"
#include "drv_pkicms.h"
#ifdef CFG_SOC_PLATFORM_CLOUD_V2
#include "devdrv_manager_comm.h"
#include "kernel_version_adapt.h"
#include "ascend_kernel_hal.h"
#endif

#define RETRY_TIME 25

typedef struct _img_id_tag_map {
    int img_id;
    const char *tag;
} img_id_tag_map_t;

const img_id_tag_map_t g_img_id_tag_map_cms[] = {
    {ITEE_IMG_ID, "tee"},
    {DTB_IMG_ID, "dtimage"},
    {ZIMAGE_ID, "uimage"},
    {FS_IMG_ID, "initrd"},
    {SD_PEK_DTB_IMG_ID, "sdpekdtimg"},
    {SD_IMG_ID, "sdimg"},
    {PEK_IMG_ID, "pekimg"},
    {DP_IMG_ID, "dpimg"},
    {ROOTFS_IMG_ID, "sysdrv"},
    {APP_IMG_ID, "appimg"},
    {DTB_DP_PEK_IMG_ID, "dtdppekimg"},
    {DTB_SD_PEK_IMG_ID, "dtsdpekimg"},
    {DP_PEK_IMG_ID, "dppekimg"},
    {SD_PEK_IMG_ID, "sdpekimg"},
    {DP_CORE_IMG_ID, "dpcoreimg"},
};

const img_id_tag_map_t g_img_id_tag_map_soc[] = {
    {SOC_VERIFY_IMG_TSCH_FW, "tschfw"},
    {SOC_VERIFY_IMG_AICPU_KERNELS, "aicpu_kernels"},
    {SOC_VERIFY_IMG_FFTS_PLUS_FW, "fftsplusfw"},
};

/* *
 * open mdc package in read only mode.
 */
struct file *open_mdc_package(const char *full_name)
{
    struct file *fp = filp_open(full_name, O_RDONLY | O_LARGEFILE, 0);
    if (IS_ERR(fp)) {
        pkicms_err("%s: open file %s failed.\n", __func__, full_name);
        return NULL;
    }
    pkicms_debug("%s: finish to open file: %s.\n", __func__, full_name);
    return fp;
}

/* *
 * read package file.
 */
int read_mdc_package_content(struct file *fp, char *buff, u32 len, int *read_len)
{
    ssize_t ret;
    loff_t pos = fp->f_pos;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
    ret = kernel_read(fp, buff, len, &pos);
#else
    mm_segment_t old_fs;
    old_fs = get_fs();
    set_fs(KERNEL_DS);
    ret = vfs_read(fp, buff, len, &pos);
    set_fs(old_fs);
#endif

    if (ret < 0) {
        return READ_ERROR;
    }

    *read_len = ret;
    if (ret < len) {
        ret = READ_END;
    } else {
        ret = READ_CONTINUE;
    }

    pkicms_debug("%s: len=%d, pos=%lld.\n", __func__, *read_len, fp->f_pos);
    fp->f_pos = pos;
    return ret;
}

/* *
 * close package file point.
 */
void close_mdc_package(struct file *fp)
{
    if (filp_close(fp, NULL)) {
        pkicms_err("%s: close file error.\n", __func__);
    }
}

uintptr_t open_local_file(const char *filename)
{
    struct file *pfile = open_mdc_package(filename);
    if (pfile == NULL) {
        pkicms_err("error occured while open file %s.\n", filename);
        return (uintptr_t)NULL;
    }
    return (uintptr_t)pfile;
}

int read_local_file(uintptr_t fp, char *buff, u32 len, int *read_len)
{
    struct file *pfile = (struct file *)fp;
    return read_mdc_package_content(pfile, (char *)buff, len, read_len);
}

loff_t lseek_local_file(uintptr_t fp, loff_t offset, int whence)
{
    struct file *pfile = (struct file *)fp;
    loff_t file_size;

    if (whence == SEEK_CUR) {
        pkicms_err("not support SEEK_CUR.\n");
        return -EOPNOTSUPP;
    }

    if (pfile->f_inode == NULL) {
        pkicms_err("f_inode is null.\n");
        return -EINVAL;
    }

    file_size = (loff_t)pfile->f_inode->i_size;
    if ((offset > file_size) || ((offset < 0) && (offset + file_size < 0))) {
        pkicms_err("offset = %lld, is larger than file size.\n", offset);
        return -EINVAL;
    }

    return vfs_llseek(pfile, offset, whence);
}

void release_local_file(uintptr_t fp)
{
    close_mdc_package((struct file *)fp);
}

loff_t cur_offset_local_file(uintptr_t fp)
{
    struct file *pfile = (struct file *)fp;
    if (pfile == NULL) {
        return 0;
    }

    return pfile->f_pos;
}

int set_local_file_operations(image_fop_t *op)
{
    if (op == NULL) {
        pkicms_err("op can't be null.\n");
        return -EINVAL;
    }

    op->open = open_local_file;
    op->read = read_local_file;
    op->lseek = lseek_local_file;
    op->release = release_local_file;
    op->cur_offset = cur_offset_local_file;
    return 0;
}
EXPORT_SYMBOL(set_local_file_operations);

/*
 * calculate the hash code use sha256
 */
static void free_shash_sha256(shash_sha256_t *sha256)
{
    if (sha256->shash != NULL) {
        crypto_free_shash(sha256->shash);
        sha256->shash = NULL;
    }

    if (sha256->desc != NULL) {
        sha256->desc->tfm = NULL;
        kfree(sha256->desc);
        sha256->desc = NULL;
    }
}

int pkicms_sha256_init(shash_sha256_t *sha256)
{
    int ret;
    char *shash_desc = NULL;

    if (sha256 == NULL) {
        pkicms_err("input param invalid.\n");
        return -EINVAL;
    }

    sha256->desc = NULL;
    sha256->shash = crypto_alloc_shash("sha256", 0, 0);
    if (sha256->shash == NULL) {
        pkicms_err("crypto_alloc_shash failed.\n");
        return PKICMS_ERR_KERNEL;
    }
    pkicms_debug("crypto_shash_descsize(sha256) = %d.\n", crypto_shash_descsize(sha256->shash));

    shash_desc = (char *)kmalloc(sizeof(struct shash_desc) + crypto_shash_descsize(sha256->shash),
        GFP_KERNEL | __GFP_ACCOUNT);
    if (shash_desc == NULL) {
        pkicms_err("malloc shash desc failed. desc size=%d.\n", crypto_shash_descsize(sha256->shash));
        ret = PKICMS_ERR_MEMORY;
        goto OUT;
    }

    sha256->desc = (struct shash_desc *)shash_desc;
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
    sha256->desc->flags = 0;
#endif
    sha256->desc->tfm = sha256->shash;

    ret = crypto_shash_init(sha256->desc);
    if (ret < 0) {
        pkicms_err("crypto_shash_init failed. ret: %d.\n", ret);
        goto OUT;
    }
    return 0;

OUT:
    free_shash_sha256(sha256);
    return ret;
}

int pkicms_sha256_update(shash_sha256_t *sha256, const void *data, u32 len)
{
    int ret;
    if (sha256 == NULL || sha256->desc == NULL || data == NULL) {
        pkicms_err("input param invalid.\n");
        return -EINVAL;
    }

    ret = crypto_shash_update(sha256->desc, data, len);
    if (ret < 0) {
        pkicms_err("crypto_shash_update failed. ret: %d.\n", ret);
        goto OUT;
    }
    return 0;

OUT:
    free_shash_sha256(sha256);
    return ret;
}

int pkicms_sha256_final(shash_sha256_t *sha256, void *sha256_code, u32 *hash_len)
{
    int ret;
    if (sha256 == NULL || sha256->desc == NULL || sha256_code == NULL || hash_len == NULL) {
        pkicms_err("input param invalid.\n");
        return -EINVAL;
    }

    ret = crypto_shash_final(sha256->desc, sha256_code);
    if (ret < 0) {
        pkicms_err("crypto_shash_final failed. ret: %d.\n", ret);
        goto OUT;
    }

    *hash_len = crypto_shash_digestsize(sha256->desc->tfm);
    ret = 0;

OUT:
    free_shash_sha256(sha256);
    return ret;
}

int pkicms_sha256_string(const void *data, u32 len, u8 *hash_code, u32 *hash_len)
{
    int ret;
    shash_sha256_t sha256;

    ret = pkicms_sha256_init(&sha256);
    if (ret) {
        pkicms_err("img hash init failed! res = 0x%x\n", ret);
        return ret;
    }

    ret = pkicms_sha256_update(&sha256, data, len);
    if (ret != 0) {
        (void)pkicms_sha256_final(&sha256, hash_code, hash_len);
        pkicms_err("img hash update failed! res = 0x%x\n", ret);
        return ret;
    }

    ret = pkicms_sha256_final(&sha256, hash_code, hash_len);
    if (ret != 0) {
        pkicms_err("img hash final failed! res = 0x%x\n", ret);
        return ret;
    }

    return 0;
}
EXPORT_SYMBOL(pkicms_sha256_string);

STATIC int pkicms_hash_update_file_fragment(
    shash_sha256_t *sha256, const char *file_fragment, u32 len, loff_t offset, int flag)
{
    pkicms_debug("file fragment: offset=%llu, flag=%d\n", offset, flag);
    if (pkicms_sha256_update(sha256, file_fragment, len)) {
        pkicms_err("%s : close file error.\n", __func__);
        return -EINVAL;
    }

    return 0;
}

STATIC int pkicms_calc_hashcode(
    shash_sha256_t *sha256, const image_file_t *img_pkg, image_fop_t *op, uintptr_t pfile, char *read_buff)
{
    int ret;
    int read_flag = READ_CONTINUE;
    int read_len;
    loff_t offset;
    int retry_time = 0;

    offset = op->cur_offset(pfile);
    while (read_flag == READ_CONTINUE) {
        ret = EIO;
        read_len = SEC_IMG_VERIFY_MAX_SIZE;
        if ((unsigned long)offset + read_len > (unsigned long)img_pkg->content_offset + img_pkg->content_length) {
            read_len = img_pkg->content_offset + img_pkg->content_length - offset;
        }

        if ((read_len > SEC_IMG_VERIFY_MAX_SIZE) || (read_len < 0)) {
            pkicms_err("file_name(%s) read_len(%d) error, offset=%lld, content_offset=%lld, content_length=%lu.\n",
                img_pkg->file_name,
                read_len,
                offset,
                img_pkg->content_offset,
                img_pkg->content_length);
            goto out;
        }

        (void)memset_s(read_buff, SEC_IMG_VERIFY_MAX_SIZE, 0, SEC_IMG_VERIFY_MAX_SIZE);
        read_flag = op->read(pfile, read_buff, read_len, &read_len);
        if (read_flag == READ_ERROR) {
            pkicms_err("read the file failed. read_pos = %lld.\n", op->cur_offset(pfile));
            goto out;
        }

        pkicms_debug("%s: offset=%lld, read_len=%d\n", __func__, offset, read_len);
        if (read_len == 0) {
            pkicms_info("read to the end of the file!\n");
            goto out;
        }

        ret = pkicms_hash_update_file_fragment(sha256, read_buff, (u32)read_len, offset, read_flag);
        if (ret) {
            pkicms_err("%s : pkicms_hash_update_file_fragment. ret=%d.\n", __func__, ret);
            goto out;
        }

        if (retry_time >= RETRY_TIME) {
            retry_time = 0;
            yield();
        }
        retry_time++;

        if (read_flag == READ_END) {
            goto out;
        }

        offset = op->cur_offset(pfile);
        if ((unsigned long)offset >= (unsigned long)img_pkg->content_offset + img_pkg->content_length) {
            goto out;
        }
    }

out:

    return ret;
}

STATIC int pkicms_calc_file_hashcode(shash_sha256_t *sha256, const image_file_t *img_pkg, image_fop_t *op)
{
    int ret = -EIO;
    char *read_buff = NULL;
    uintptr_t pfile;
    loff_t offset;

    if (op == NULL) {
        pkicms_err("input parameter is invalid. op is NULL.\n");
        return -EINVAL;
    }

    read_buff = (char *)vmalloc(SEC_IMG_VERIFY_MAX_SIZE);
    if (read_buff == NULL) {
        pkicms_err("security upgrade large file failed. alloc memory failed.\n");
        return -ENOMEM;
    }

    pfile = op->open(img_pkg->file_name);
    if (pfile == (uintptr_t)NULL) {
        pkicms_err("%s : error occured while open file.\n", __func__);
        goto free_buff;
    }

    if (img_pkg->content_offset > 0) {
        offset = op->lseek(pfile, img_pkg->content_offset, SEEK_SET);
        if (offset < 0) {
            pkicms_err("seek file from begin error. \n");
            goto close_file;
        }
    }

    ret = pkicms_calc_hashcode(sha256, img_pkg, op, pfile, read_buff);
    if (ret != 0) {
        pkicms_err("pkicms_calc_hashcode failed. file_name(%s), ret(%d).\n", img_pkg->file_name, ret);
        goto close_file;
    }

close_file:
    pkicms_debug("%s : closing file.\n", __func__);
    op->release(pfile);
    pfile = (uintptr_t)NULL;
free_buff:
    pkicms_debug("%s : freeing read_buffer.\n", __func__);
    vfree(read_buff);
    read_buff = NULL;

    return ret;
}

int pkicms_calc_digest(
    const char *filename, loff_t data_offset, unsigned long data_len, u8 *hashcode, u32 hash_len, image_fop_t *op)
{
    int ret;
    image_file_t mdc_pkg = {0};
    shash_sha256_t sha256;

    if (filename != NULL) {
        if (strcpy_s(mdc_pkg.file_name, sizeof(mdc_pkg.file_name), filename) != EOK) {
            pkicms_err("set file name filed.\n");
            return -ENOMEM;
        }
    }

    mdc_pkg.content_offset = data_offset;
    mdc_pkg.content_length = data_len;

    ret = pkicms_sha256_init(&sha256);
    if (ret) {
        pkicms_err("hash sha256 init failed.\n");
        return ret;
    }

    ret = pkicms_calc_file_hashcode(&sha256, &mdc_pkg, op);
    if (ret) {
        (void)pkicms_sha256_final(&sha256, hashcode, &hash_len);
        pkicms_err("hash large file failed.\n");
        return ret;
    }

    ret = pkicms_sha256_final(&sha256, hashcode, &hash_len);
    if (ret) {
        pkicms_err("calc file sha256 failed. result=%d. len=%u.\n", ret, hash_len);
    }

    return ret;
}
EXPORT_SYMBOL(pkicms_calc_digest);

#ifdef CFG_SOC_PLATFORM_CLOUD_V2
typedef int (hal_kernel_get_hardware_info_func)(unsigned int phy_id, devdrv_hardware_info_t *hardware_info);
#endif

/* offset: efuse reg addr offset, reference: EFUSE0_CTRL_BASE */
int pkicms_read_efuse_reg(u32 dev_id, u64 efuse_offset, u32 *out_value)
{
    void __iomem *efuse_vir_addr = NULL;
    u32 reg_data;
#ifdef CFG_SOC_PLATFORM_CLOUD_V2
    devdrv_hardware_info_t hardware_info = {0};
    static hal_kernel_get_hardware_info_func *pkicms_get_hardware_info = NULL;
    int ret;

    if (pkicms_get_hardware_info == NULL) {
        pkicms_get_hardware_info =
            (hal_kernel_get_hardware_info_func *)(uintptr_t)__kallsyms_lookup_name("hal_kernel_get_hardware_info");
        if (IS_ERR_OR_NULL(pkicms_get_hardware_info)) {
            pkicms_err("fail to find symbol hal_kernel_get_hardware_info.\n");
            return -EINVAL;
        }
    }

    ret = pkicms_get_hardware_info(dev_id, &hardware_info);
    if (ret != 0) {
        pkicms_err("Failed to invoke hal_kernel_get_hardware_info. (devid=%u)\n", dev_id);
        return ret;
    }

    efuse_vir_addr = (void __iomem *)ioremap_wc(efuse_offset + hardware_info.phy_addr_offset, EFUSE_REG_READ_LEN);
#else
    efuse_vir_addr = (void __iomem *)ioremap_wc(efuse_offset + dev_id * SMP_REG_BASE_OFFSET, EFUSE_REG_READ_LEN);
#endif
    if (efuse_vir_addr == NULL) {
        pkicms_err("dev(%u) efuse_vir_addr memory req fail\n", dev_id);
        return -EINVAL;
    }
    reg_data = readl_relaxed(efuse_vir_addr);
    iounmap(efuse_vir_addr);
    efuse_vir_addr = NULL;

    /* efuse no burned */
    if (reg_data == 0) {
        pkicms_warn("dev(%u) efuse may not loaded\n", dev_id);
    }

    *out_value = reg_data;
    return 0;
}

#ifdef CFG_SOC_PLATFORM_MDC_LITE_ESL
// esl don't support tee
int sec_read_nsforbid(unsigned int dev_id, unsigned int *nsforbid_val)
{
    pkicms_err("sec_read_nsforbid error, esl don't support tee\n");
    return 0;
}
#endif

int pkicms_get_sec_check_enable_flag(int *flag)
{
    s32 ret;
    u32 out_value = 0;
#ifndef CFG_FEATURE_GET_ENABLE_FLAG
#ifdef CFG_SOC_PLATFORM_MDC_V51
    ret = pkicms_get_chip_info();
    if (ret == CHIP_TYPE_BS9SX1A) {
        ret = sec_read_nsforbid(0, &out_value);
    } else {
        ret = pkicms_read_efuse_reg(0, EFUSE_NS_FORBID_OFFSET, &out_value);
    }
#else
    // check NS_FORBIDDEN
    ret = pkicms_read_efuse_reg(0, EFUSE_NS_FORBID_OFFSET, &out_value);
#endif
#else
    ret = sec_read_nsforbid(0, &out_value);
#endif
    if (ret != 0) {
        pkicms_err("ef read hash check fail, ns forbid err, ret = %d\n", ret);
        return PKICMS_ERR_PARAM;
    }

    /* 安全启动使能标志读取数据为0， 表示不需要实际去校验签名 */
    if (out_value == 0) {
        *flag = PKICMS_SEC_CHECK_DISABLE;
    } else {
        *flag = PKICMS_SEC_CHECK_ENABLE;
    }

    return ret;
}
EXPORT_SYMBOL(pkicms_get_sec_check_enable_flag);

int pkicms_check_img_tag(int verify_type, int img_id, const img_ver_header *img_ver)
{
    int ret;
    int i;
    int num;
    int tag_len;
    img_id_tag_map_t *img_id_tag_map = NULL;

    if (verify_type == VERIFY_TYPE_SOC) {
        img_id_tag_map = (img_id_tag_map_t *)&g_img_id_tag_map_soc[0];
        num = sizeof(g_img_id_tag_map_soc) / sizeof(img_id_tag_map_t);
    } else if (verify_type == VERIFY_TYPE_CMS) {
        img_id_tag_map = (img_id_tag_map_t *)&g_img_id_tag_map_cms[0];
        num = sizeof(g_img_id_tag_map_cms) / sizeof(img_id_tag_map_t);
    } else {
        pkicms_warn("verify_type[%d] not valid.\n", verify_type);
        return -EINVAL;
    }

    for (i = 0; i < num; i++) {
        if (img_id_tag_map[i].img_id == img_id) {
            tag_len = strlen(img_id_tag_map[i].tag) + 1;
            ret = memcmp(img_id_tag_map[i].tag, (u8 *)img_ver->tag, tag_len);
            if (ret != 0) {
                pkicms_warn("image tag and id[%d] not match.\n", img_id);
                return -ENOENT;
            }

            return 0;
        }
    }

    if (verify_type == VERIFY_TYPE_CMS) {
        pkicms_info("image id[%d] not found, do not check tag.\n", img_id);
        return 0;
    }

    pkicms_warn("image id[%d] not found.\n", img_id);
    return -EINVAL;
}

int pkicms_ver_check(int verify_type, int img_id, const img_ver_header *img_ver, u32 hw_cnt)
{
    int ret;
    int efuse_bit_pos;
    unsigned long efuse_nvcnt = (unsigned long)hw_cnt;

#ifndef CFG_FEATURE_SIGN_VERSION_1
    if (img_ver->magic_num != IMG_VER_MAGIC) {
        pkicms_err("check fail, ver magic err, (magic = 0x%08x)\n", img_ver->magic_num);
        return -EINVAL;
    }
#endif

    /* check image tag */
    ret = pkicms_check_img_tag(verify_type, img_id, img_ver);
    if (ret != 0) {
        pkicms_err("pkicms_check_img_tag fail, ret=%d.\n", ret);
        return ret;
    }

    /* check image nvcnt */
    if (img_ver->nv_cnt > IMG_NVCNT_VER_MAX) {
        pkicms_err("nv_cnt(%d) fail\n", img_ver->nv_cnt);
        return -EINVAL;
    }

    /*
     * Find last bit set in efuse nvcnt(LSB 1,MSB 32)
     * and then calculate the bit mask of insecure versions
     * e.g: efuse_nvcnt is 0x010 -> last bit set is bit5(starts from 1)
     *      the bit mask of insecure versions: 0x1F
     */
    if (efuse_nvcnt == 0) {
        /* There is no bit set, any version(0-31) is OK */
        return 0;
    }

    efuse_bit_pos = find_last_bit((const unsigned long *)&efuse_nvcnt, BIT_POS_MAX);
    if ((int)img_ver->nv_cnt <= efuse_bit_pos) {
        pkicms_err("nv_cnt(%d) <= efuse_bit_pos(%d)\n", img_ver->nv_cnt, efuse_bit_pos);
        return -EINVAL;
    }

    return 0;
}

#ifdef CFG_SOC_PLATFORM_MDC_LITE_ESL
// esl don't support tee
int soc_get_nvcnt(unsigned int dev_id, unsigned int *buff, unsigned int size)
{
    pkicms_err("soc_get_nvcnt error, esl don't support tee\n");
    return 0;
}
#endif

int pkicms_nvcnt_read(u32 dev_id, u32 *hw_l2_nv_cnt)
{
    s32 ret;
    u32 out_value = 0;

    // read l2NVCNT
    ret = soc_get_nvcnt(dev_id, &out_value, sizeof(out_value));
    if (ret) {
        pkicms_info("soc_get_nvcnt is abnormal. (dev=%u; ret=%d)\n", dev_id, ret);
        return PKICMS_ERR_PARAM;
    }

    *hw_l2_nv_cnt = out_value;

    return 0;
}
int pkicms_write_file_buf(const char *path, u8 *buf, u32 len)
{
    return 0; /* kernel space not use now, implement when need */
}

