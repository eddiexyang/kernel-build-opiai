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

#ifndef FW_UPGRADE_MINI_KERNEL_UT
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/errno.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include <linux/crypto.h>
#include <linux/fs.h>
#include <linux/scatterlist.h>
#include <linux/version.h>
#include <linux/securec.h>
#include <stdbool.h>
#include <linux/stat.h>
#include <linux/fcntl.h>
#include <linux/gfp.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 15, 0)
#include <crypto/hash.h>
#endif
#include "dev_upgrade_adapt.h"
#include "drv_whitelist.h"

#if (!defined SUPPORT_TEE_SECURE_UPGRADE)
#include "hisfc300_def.h"
#include "tee/tz_api.h"
#endif

#if (defined CFG_SOC_PLATFORM_CLOUD)
#include "user_cfg_management.h"
#include "dev_upgrade_cloud.h"
#elif (defined CFG_SOC_PLATFORM_MDC_V51)
#include "dev_upgrade_mdc.h"
#include "dev_upgrade_sec.h"
#include "dev_upgrade_mdccore.h"
#elif (defined CFG_SOC_PLATFORM_MINIV2)
#include "dev_upgrade_sec.h"
#include "dev_upgrade_mini.h"
#else
#include "dev_upgrade_mini.h"
#endif

#define MTD_FLASH_PARTITION "/proc/mtd"
#define FILE_ONE_LINE_MAX 256
#define DEV_PATH_NAME "/dev/"

dev_component_list_ctrl g_component_list_ctrl[DEVICE_NUM_MAX] = {{0}};

#if (!defined SUPPORT_TEE_SECURE_UPGRADE)
STATIC int dev_flash_erase_mtd_blk(const char *part_name, unsigned int len, unsigned int offset)
{
    int ret;
    unsigned int length;

    if (part_name == NULL) {
        dev_upgrade_err("para is invalid. \n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    length = (len + FLASH_BLOCK_SIZE - 1) / FLASH_BLOCK_SIZE * FLASH_BLOCK_SIZE;
    ret = hisi_sfc_ctl_flash_erase((unsigned char *)part_name, offset, (size_t)length);
    if (ret != 0) {
        dev_upgrade_err("hisi_sfc_ctl_flash_erase fail, ret: %d\n", ret);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_ERASE);
    }

    return 0;
}

STATIC int dev_flash_read_mtd_blk(const char *part_name, unsigned char *buf, unsigned int len, unsigned int offset)
{
    int ret;
    size_t ret_len = 0;

    if ((part_name == NULL) || (buf == NULL)) {
        dev_upgrade_err("para is invalid, part_name: 0x%x\n", (int)(uintptr_t)part_name);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    ret = hisi_sfc_ctl_flash_read((unsigned char *)part_name, offset, (size_t)len, (size_t *)&ret_len, buf);
    if (ret != 0) {
        dev_upgrade_err("hisi_sfc_ctl_flash_read fail, part_name: %s, offset: %u, len: %u, ret: %d\n",
            part_name, offset, len, ret);
        return ret;
    }

    if (len != (unsigned int)ret_len) {
        dev_upgrade_err("read data err, len(%u) != ret_len(%lu)\n", len, ret_len);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_READ);
    }

    return 0;
}

STATIC int dev_flash_write_mtd_blk(const char *part_name, unsigned char *buf, unsigned int len, unsigned int offset)
{
    int ret;
    size_t ret_len = 0;

    if ((part_name == NULL) || (buf == NULL)) {
        dev_upgrade_err("para is invalid. \n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    ret = dev_flash_erase_mtd_blk(part_name, len, offset);
    if (ret != 0) {
        dev_upgrade_err("dev_flash_erase_mtd_blk fail, part_name: %s, offset: %u, len: %u, ret: %d\n",
            part_name, offset, len, ret);
        return ret;
    }

    ret = hisi_sfc_ctl_flash_write((unsigned char *)part_name, offset, (size_t)len, (size_t *)&ret_len, buf);
    if (ret != 0) {
        dev_upgrade_err("hisi_sfc_ctl_flash_write fail, part_name: %s, offset: %u, len: %u, ret: %d\n",
            part_name, offset, len, ret);
        return ret;
    }

    if (len != (unsigned int)ret_len) {
        dev_upgrade_err("write data err, len(%u) != ret_len(%lu)\n", len, ret_len);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_WRITE);
    }

    return 0;
}

STATIC void dev_upgrade_mem_read(unsigned char *out_buff, const unsigned char *in_buff, unsigned int len)
{
    unsigned int i = 0;

    DRV_CHECK_RET(out_buff != NULL);
    DRV_CHECK_RET(in_buff != NULL);

    for (i = 0; i < len; i++) {
        out_buff[i] = in_buff[i];
    }

    return;
}

STATIC void dev_upgrade_print_buff(unsigned char *buff, unsigned int size)
{
    unsigned int loop = 0;

    DRV_CHECK_RET(buff != NULL);
    for (loop = 0; loop < size; loop++) {
        if (loop % LINE_FEED_PRINT == 0) {
            dev_upgrade_info("\n");
        }
        dev_upgrade_info("%02x ", buff[loop]);
    }
    dev_upgrade_info("\n\n");
}
#endif

STATIC component_area_info *dev_upgrade_get_component_static_table(int dev_id)
{
    component_area_info *component_table = NULL;
    switch (dev_id) {
        case INDEX_0:
            component_table = &g_component_area_info_chip0[0];
            break;
#ifdef CFG_SOC_PLATFORM_CLOUD
        case INDEX_1:
            component_table = &g_component_area_info_chip1[0];
            break;
        case INDEX_2:
            component_table = &g_component_area_info_chip2[0];
            break;
        case INDEX_3:
            component_table = &g_component_area_info_chip3[0];
            break;
#endif
        default:
            component_table = NULL;
            dev_upgrade_err("dev(%d) invalid\n", dev_id);
            break;
    }

    return component_table;
}

STATIC dev_component_list_ctrl *dev_upgrade_get_component_list_ctrl(int dev_id)
{
    if (dev_id >= DEVICE_NUM_MAX || dev_id < 0) {
        dev_upgrade_err("dev(%d) invalid\n", dev_id);
        return NULL;
    }

    return &g_component_list_ctrl[dev_id];
}

STATIC component_area_info *dev_component_area_info_search(const upg_comm_para *comm_para)
{
    unsigned int i = 0;
    dev_component_list_ctrl *comp_list_ctrl = NULL;

    comp_list_ctrl = dev_upgrade_get_component_list_ctrl(comm_para->dev_id);
    if (comp_list_ctrl == NULL) {
        dev_upgrade_err("get component list ctrl fail, dev(%d).\n", comm_para->dev_id);
        return NULL;
    }

    for (i = 0; i < comp_list_ctrl->count; i++) {
        if (comm_para->area == comp_list_ctrl->components[i].area &&
            comm_para->type == comp_list_ctrl->components[i].component_type) {
            return &comp_list_ctrl->components[i];
        }
    }

    return NULL;
}

#if (!defined SUPPORT_TEE_SECURE_UPGRADE)
#if (!defined CFG_SOC_PLATFORM_CLOUD) &&  (!defined CFG_SOC_PLATFORM_MINIV2)
STATIC int get_nve_desc(const char *part_name, NVE_PARTION_HEADER *desc_nve_partion, unsigned int len)
{
    unsigned int i = 0;
    int ret = 0;

    DRV_CHECK_RETV((part_name != NULL), ERRNO_NULL_POINT);
    DRV_CHECK_RETV((desc_nve_partion != NULL), ERRNO_NULL_POINT);
    DRV_CHECK_RETV((len <= NVE_AREA_CONT), ERRNO_INVALID_LENGTH);

    for (i = 0; i < len; i++) {
        // i 是该desc_nve_partion 所属于的分区号
        desc_nve_partion[i].belonged_are = i;

        ret = dev_flash_read_mtd_blk(part_name, desc_nve_partion[i].name, NVE_NAME_LEN,
            i * NVE_AREA_LEN + NVE_NAME_OFFSET);
        if (ret != 0) {
            dev_upgrade_err("read fash error, partion= %s, offset = 0x%x, len = %d, ret = 0x%x\n",
                            part_name, i * NVE_AREA_LEN + NVE_NAME_OFFSET, NVE_NAME_LEN, ret);
            return ret;
        }

        ret = dev_flash_read_mtd_blk(part_name, (unsigned char *)&desc_nve_partion[i].nve_age, sizeof(unsigned int),
            i * NVE_AREA_LEN + NVE_AGE_OFFSET);
        if (ret != 0) {
            dev_upgrade_err("read fash error, partion= %s, offset = 0x%x, len = %u, ret = 0x%x\n",
                            part_name, i * NVE_AREA_LEN + NVE_AGE_OFFSET, (unsigned int)sizeof(unsigned int), ret);
            return ret;
        }

        ret = dev_flash_read_mtd_blk(part_name, desc_nve_partion[i].nve_version, NVE_VER_LEN,
            i * NVE_AREA_LEN + NVE_VER_OFFSET);
        if (ret != 0) {
            dev_upgrade_err("read fash error, partion= %s, offset = 0x%x, len = %u, ret = 0x%x\n",
                            part_name, i * NVE_AREA_LEN + NVE_VER_OFFSET, (unsigned int)sizeof(unsigned int), ret);
            return ret;
        }
    }

    return 0;
}

// dev_upgrade_nve_update 获取有效文件大小
int check_nve_continer(unsigned char *buff, unsigned int buff_len)
{
    unsigned int valid_item;
    unsigned int *p_valid_item = NULL;
    unsigned int *p_nve_last_size = NULL;
    unsigned int nve_write_size;
    unsigned int *p_nve_last_bin_size = NULL;
    int ret;

    if ((buff_len < NVE_HEAD_LENGTH) || (buff == NULL)) {
        dev_upgrade_err("buffer null \n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    p_valid_item = (unsigned int *)&buff[NVE_VALID_ITEMS_START_ADDRESS];
    valid_item = *p_valid_item;
    if (valid_item == 0) {
        dev_upgrade_warn("valid nvbin num is not valid:%d\n", valid_item);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    if (valid_item > NVE_BNI_NUM_MAX ||
        (unsigned int)(NVE_AREA_LEN - sizeof(unsigned int)) < (unsigned int)(NVE_HEAD_LENGTH +
        (valid_item - 1) * NVE_BIN_ENTRY_LEN + NVE_BIN_ENTRY_OFFSET) ||
        (unsigned int)(NVE_AREA_LEN - sizeof(unsigned int)) < (unsigned int)(NVE_HEAD_LENGTH +
        (valid_item - 1) * NVE_BIN_ENTRY_LEN + NVE_BIN_SIZE_OFFSET)) {
        dev_upgrade_warn("nvbin num is invalid 0x%x\n", valid_item);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    p_nve_last_size = (unsigned int *)&buff[(unsigned int)(NVE_HEAD_LENGTH +
                      (valid_item - 1) * NVE_BIN_ENTRY_LEN + NVE_BIN_ENTRY_OFFSET)];
    p_nve_last_bin_size = (unsigned int *)&buff[(unsigned int)(NVE_HEAD_LENGTH +
                          (valid_item - 1) * NVE_BIN_ENTRY_LEN + NVE_BIN_SIZE_OFFSET)];
    nve_write_size = *p_nve_last_size + *p_nve_last_bin_size;
    dev_upgrade_debug("nve_write_size = %u\n", nve_write_size);
    if (nve_write_size > NVE_AREA_LEN) {
        dev_upgrade_warn("nvbin size is invalid 0x%x,max size is 0x%x\n", nve_write_size, NVE_AREA_LEN);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    ret = dev_upgrade_nve_sha256_check(buff, valid_item, nve_write_size);
    if (ret != 0) {
        dev_upgrade_warn("check nve continer fail,ret  = %d\n", ret);
        return ret;
    }
    return 0;
}

STATIC int check_nve_partion_validity(const char *part_name, NVE_PARTION_HEADER *desc_nve_partion)
{
    int ret;
    unsigned char *nve_continer = NULL;
    int belonged_are;

    DRV_CHECK_RETV((part_name != NULL), ERRNO_NULL_POINT);
    nve_continer = vzalloc(NVE_AREA_LEN);
    belonged_are = desc_nve_partion->belonged_are;

    if (nve_continer == NULL) {
        dev_upgrade_err(" malloc fail\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_MALLOC);
    }

    // 从分区1中获取 nve的内容
    ret = dev_flash_read_mtd_blk(part_name, nve_continer, NVE_AREA_LEN, belonged_are * NVE_AREA_LEN);
    if (ret != 0) {
        dev_upgrade_err("read fash error, partion= %s, offset = 0x%x, len = %u, ret = 0x%x\n",
                        part_name, belonged_are * NVE_AREA_LEN, NVE_AREA_LEN, ret);
        DEV_UPGRADE_VFREE(nve_continer);
        return ret;
    }

    ret = check_nve_continer(nve_continer, NVE_AREA_LEN);
    if (ret != 0) {
        DEV_UPGRADE_VFREE(nve_continer);
        dev_upgrade_warn("check nve continer fail, ret: %d\n", ret);
        return ret;
    }
    DEV_UPGRADE_VFREE(nve_continer);
    return 0;
}

STATIC int get_max_age_partion(const char *part_name, NVE_PARTION_HEADER *desc_nve_partion,
                               unsigned int len, unsigned int *max_age_partion)
{
    unsigned int i;
    unsigned int max_age;
    unsigned int invalid_num = 0;
    int ret;

    DRV_CHECK_RETV((desc_nve_partion != NULL), ERRNO_NULL_POINT);
    DRV_CHECK_RETV((max_age_partion != NULL), ERRNO_NULL_POINT);
    *max_age_partion = 0;
    max_age = desc_nve_partion[0].nve_age;
    DRV_CHECK_RETV((len <= NVE_AREA_CONT), ERRNO_INVALID_PARAMS);
    for (i = 0; i < len; i++) {
        ret = check_nve_partion_validity(part_name, &desc_nve_partion[i]);
        if (memcmp(desc_nve_partion[i].name, NVE_HEADER_NAME, NVE_NAME_LEN - 1) != 0 || ret != 0) {
            dev_upgrade_info(" the partion 0x%x have not valid nve,partion_name= %s, continer_check_ret = %d\n",
                             i, desc_nve_partion[i].name, ret);
            invalid_num++;
            continue;
        }

        dev_upgrade_info(" the partion 0x%x, age =0x%x, version = %x%x%x%x\n",
                         i, desc_nve_partion[i].nve_age,
                         desc_nve_partion[i].nve_version[INDEX_0],
                         desc_nve_partion[i].nve_version[INDEX_1],
                         desc_nve_partion[i].nve_version[INDEX_2],
                         desc_nve_partion[i].nve_version[INDEX_3]);

        if (desc_nve_partion[i].nve_age > max_age) {
            max_age = desc_nve_partion[i].nve_age;
            *max_age_partion = desc_nve_partion[i].belonged_are;
        }
    }

    if (invalid_num == NVE_AREA_CONT) {
        dev_upgrade_err("have not valid nve partion\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NOT_HAVE_VALID_PARTION);
    }

    dev_upgrade_info(" the partion 0x%x is the current nve , age =0x%x\n",
                     *max_age_partion, desc_nve_partion[*max_age_partion].nve_age);
    return 0;
}


STATIC int nve_find_current_partion(const char *part_name, unsigned int *current_partion)
{
    NVE_PARTION_HEADER desc_nve_partion[NVE_AREA_CONT] = {0};
    int ret;

    DRV_CHECK_RETV((part_name != NULL), ERRNO_NULL_POINT);
    DRV_CHECK_RETV((current_partion != NULL), ERRNO_NULL_POINT);

    ret = memset_s(desc_nve_partion, NVE_AREA_CONT * sizeof(NVE_PARTION_HEADER), 0,
        NVE_AREA_CONT * sizeof(NVE_PARTION_HEADER));
    if (ret != 0) {
        dev_upgrade_err("nve_find_current_partion memset_s fail: %d\n", ret);
        return ret;
    }

    ret = get_nve_desc(part_name, desc_nve_partion, NVE_AREA_CONT);
    if (ret != 0) {
        dev_upgrade_err("get nve des in partion fail, ret = 0x%x\n", ret);
        return ret;
    }
    // 如果本次升级了，分区0 会有数据，直接在分区0读版本号，因为age 为0，不能去和其他分区比较
    if (memcmp(desc_nve_partion[INDEX_0].name, NVE_HEADER_NAME, NVE_NAME_LEN - 1) == 0) {
        *current_partion = 0;
        dev_upgrade_info(" in partion 0x%x have nve\n", *current_partion);
        return 0;
    }

    if (memcmp(desc_nve_partion[INDEX_0].name, NVE_HEADER_NAME, NVE_NAME_LEN - 1) != 0 &&
        memcmp(desc_nve_partion[INDEX_1].name, NVE_HEADER_NAME, NVE_NAME_LEN - 1) != 0 &&
        memcmp(desc_nve_partion[INDEX_2].name, NVE_HEADER_NAME, NVE_NAME_LEN - 1) != 0 &&
        memcmp(desc_nve_partion[INDEX_3].name, NVE_HEADER_NAME, NVE_NAME_LEN - 1) != 0) {
        *current_partion = 0;
        dev_upgrade_err("never upgrade in this device,flash is empty, version is empty\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NOT_HAVE_VALID_PARTION);
    }
    ret = get_max_age_partion(part_name, desc_nve_partion, NVE_AREA_CONT, current_partion);
    if (ret != 0) {
        dev_upgrade_err("have not valid parton ,return %d\n", ret);
        return ret;
    }
    return 0;
}
#endif
#endif
int dev_upgrade_get_fs_file_size(const char *file_name, unsigned int *file_size)
{
    struct file *filp = NULL;

    /* the file may not exist, so print warnning */
    filp = filp_open(file_name, O_RDONLY, 0);
    if (IS_ERR_OR_NULL(filp)) {
        dev_upgrade_warn("unable to open file: %s (%ld)\n", file_name, PTR_ERR(filp));
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_OPEN);
    }

    /* get file total length */
    if (filp->f_inode == NULL) {
        filp_close(filp, NULL);
        filp = NULL;
        dev_upgrade_err("file inode is NULL.\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    *file_size = filp->f_inode->i_size;
    filp_close(filp, NULL);
    filp = NULL;

    return 0;
}

int dev_upgrade_read_fs_file(const char *file_name, loff_t offset, char *buf, unsigned int size)
{
    loff_t offset_tmp = offset;
    ssize_t result;
    struct file *filp = NULL;

    filp = filp_open(file_name, O_RDONLY, 0);
    if (IS_ERR_OR_NULL(filp)) {
        dev_upgrade_warn("unable to open file: %s (%ld), default return ok\n", file_name, PTR_ERR(filp));
        return 0;
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 13, 0)
    result = kernel_read(filp, buf, size, &offset_tmp);
#else
    result = kernel_read(filp, offset_tmp, buf, (unsigned long)size);
#endif
    if (result != size) {
        dev_upgrade_err("kernel read file error \n");
        filp_close(filp, NULL);
        filp = NULL;
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_READ);
    }

    filp_close(filp, NULL);
    filp = NULL;
    return 0;
}

int dev_upgrade_get_fs_file_size_fp(struct file *filp, unsigned int *file_size)
{
      /* get file total length */
    if (filp->f_inode == NULL) {
        dev_upgrade_err("file inode is NULL.\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }
    *file_size = filp->f_inode->i_size;
    return 0;
}

int dev_upgrade_read_fs_file_fp(struct file *filp, loff_t offset, char *buf, unsigned int size)
{
    loff_t offset_tmp = offset;
    ssize_t result;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 13, 0)
    result = kernel_read(filp, buf, size, &offset_tmp);
#else
    result = kernel_read(filp, offset_tmp, buf, (unsigned long)size);
#endif
    if (result != size) {
        dev_upgrade_err("kernel read file error \n");

        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_READ);
    }
    return 0;
}

int dev_upgrade_write_fs_file(const char *file_name, const char *buf, unsigned int size, int mode)
{
    int ret;
    struct file *filp = NULL;
    loff_t offset_tmp = 0;

    if (buf == NULL || file_name == NULL) {
        dev_upgrade_err("null pointer, buf: %pK\n", buf);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    if (size == 0) {
        dev_upgrade_err("size 0 is invalid\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_LENGTH);
    }

    filp = filp_open(file_name, O_RDWR | O_TRUNC | O_CREAT, mode);
    if (IS_ERR_OR_NULL(filp)) {
        dev_upgrade_err("unable to open file: %s (%ld)\n", file_name, PTR_ERR(filp));
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_OPEN);
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 13, 0)
    ret = kernel_write(filp, (char *)buf, size, &offset_tmp);
#else
    ret = kernel_write(filp, (char *)buf, size, offset_tmp);
#endif
    if (ret != size) {
        dev_upgrade_err("file(%s) write failed(%d), size: %u\n", file_name, ret, size);
        filp_close(filp, NULL);
        filp = NULL;
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_WRITE);
    }

    filp_close(filp, NULL);
    filp = NULL;

    return 0;
}

int dev_upgrade_file_content_check(const char *file_name, const char *buf, unsigned int size, unsigned int *check_res)
{
    int ret;
    unsigned int file_size = 0;
    char *file_buf = NULL;
    struct file *filp = NULL;

    if (buf == NULL || file_name == NULL || check_res == NULL) {
        dev_upgrade_err("null pointer, file_name: %pK, buf: %pK\n", file_name, buf);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    if (size == 0) {
        dev_upgrade_err("size 0 is invalid\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_LENGTH);
    }

    /* the file may not exist, so print warnning */
    filp = filp_open(file_name,  O_RDONLY | O_LARGEFILE, 0);
    if (IS_ERR_OR_NULL(filp)) {
        dev_upgrade_warn("unable to open file: %s (%ld)\n", file_name, PTR_ERR(filp));
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_OPEN);
    }

    *check_res = CHECK_UNKNOWN;
    ret = dev_upgrade_get_fs_file_size_fp(filp, &file_size);
    if ((ret != 0) || (size != file_size)) {
        *check_res = CHECK_HASH_DIFF;
        dev_upgrade_info("size is diff, new_size: %u, old_size: %u, ret: %d\n", size, file_size, ret);
        filp_close(filp, NULL);
        filp = NULL;
        return 0;
    }

    file_buf = (char *)(void *)vzalloc(file_size);
    if (file_buf == NULL) {
        filp_close(filp, NULL);
        filp = NULL;
        dev_upgrade_err("vzalloc fail\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_MALLOC);
    }

    ret = dev_upgrade_read_fs_file_fp(filp, 0, file_buf, file_size);
    if (ret != 0) {
        filp_close(filp, NULL);
        filp = NULL;
        dev_upgrade_err("get file size fail, ret: %d\n", ret);
        DEV_UPGRADE_VFREE(file_buf);
        return ret;
    }

    if (memcmp(buf, file_buf, (size_t)file_size)) {
        *check_res = CHECK_HASH_DIFF;
        filp_close(filp, NULL);
        filp = NULL;
        dev_upgrade_info("file content is different\n");
        DEV_UPGRADE_VFREE(file_buf);
        return 0;
    }
    filp_close(filp, NULL);
    filp = NULL;
    DEV_UPGRADE_VFREE(file_buf);

    *check_res = CHECK_ALL_THE_SAME;
    return 0;
}

#if (!defined SUPPORT_TEE_SECURE_UPGRADE)
STATIC int dev_upgrade_get_version_offset(component_area_info *area_info, unsigned int type, unsigned int *offset)
{
#if (defined CFG_SOC_PLATFORM_CLOUD) || (defined CFG_SOC_PLATFORM_MINIV2)
    (void)area_info;
    *offset = COMM_VERSION_OFFSET;
    return 0;
#else
    int ret;
    unsigned int current_partion = 0;

    if (type == DSMI_COMPONENT_TYPE_NVE) {
        ret = nve_find_current_partion(area_info->part_name, &current_partion);
        if (ret != 0) {
            dev_upgrade_err("nve get current partion fail 0x%x\n", ret);
            return ret;
        }

        DRV_CHECK_RETV((current_partion < NVE_AREA_CONT), dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_INDEX));
        *offset = NVE_VERSION_OFFSET + current_partion * NVE_AREA_LEN;
    } else if (type == DSMI_COMPONENT_TYPE_BOOTROM) {
        *offset = BOOTROM_HEAD_OFFSET + COMM_VERSION_OFFSET;
    } else {
        *offset = COMM_VERSION_OFFSET;
    }

    return 0;
#endif
}

int dev_upgrade_component_version_get(upg_comm_para *comm_para, unsigned int is_memory,
    unsigned char *o_buf, unsigned int o_len)
{
    int ret;
    unsigned int ver_offset = 0;
    component_area_info *comp_area_info = NULL;

    if (comm_para == NULL || o_buf == NULL) {
        dev_upgrade_err("null pointer, comm_para: %pK\n", comm_para);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    if (is_memory && comm_para->file_content == NULL) {
        dev_upgrade_err("dev(%d) file_content null pointer\n", comm_para->dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    if (o_len < COMM_VERSION_LENGTH) {
        dev_upgrade_err("dev(%d) o_len(%u) invalid\n", comm_para->dev_id, o_len);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_LENGTH);
    }

    comp_area_info = dev_component_area_info_search(comm_para);
    if (comp_area_info == NULL) {
        dev_upgrade_err("get comp_area_info struct fail, dev(%d), component(%u), area: %u\n",
            comm_para->dev_id, comm_para->type, comm_para->area);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    ret = dev_upgrade_get_version_offset(comp_area_info, comm_para->type, &ver_offset);
    if (ret != 0) {
        dev_upgrade_err("dev_upgrade_get_version_offset fail, ret: 0x%x\n", ret);
        return ret;
    }

    if (!is_memory) {
        if (comp_area_info->store_in == STORE_IN_FILESYSTEM) {
            ret = dev_upgrade_read_fs_file(comp_area_info->part_name, ver_offset, (char *)o_buf, COMM_VERSION_LENGTH);
        } else {
            ret = dev_flash_read_mtd_blk(comp_area_info->part_name, o_buf,
                COMM_VERSION_LENGTH, (comp_area_info->offset + ver_offset));
        }
        if (ret != 0) {
            dev_upgrade_err("dev(%d) read flash or fs err, ret = %x\n", comm_para->dev_id, ret);
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_READ);
        }
    } else {
        if ((ver_offset > comm_para->file_size) || (ver_offset + COMM_VERSION_LENGTH) > comm_para->file_size) {
            dev_upgrade_err("dev(%d) length invalid, file_size(%u), offset(%u)\n",
                comm_para->dev_id, comm_para->file_size, ver_offset);
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_LENGTH);
        }
        dev_upgrade_mem_read(o_buf, (comm_para->file_content + ver_offset), COMM_VERSION_LENGTH);
    }
    return 0;
}

STATIC bool dev_upgrade_is_cms_support(unsigned long long magic, unsigned int component_type)
{
    if ((magic == CMS_MAGIC_FLAG_VALUE) && ((component_type != DSMI_COMPONENT_TYPE_XLOADER) &&
        (component_type != DSMI_COMPONENT_TYPE_UEFI) && (component_type != DSMI_COMPONENT_TYPE_BOOTROM))) {
        return true;
    }

    return false;
}

STATIC int dev_upgrade_get_size_from_flash(component_area_info *area_info, unsigned int *length)
{
    int ret = 0;
    unsigned long long magic = 0;
    unsigned int code_size = 0;
    unsigned int fw_total_size = 0;

    if (area_info->component_type == DSMI_COMPONENT_TYPE_BOOTROM) {
        magic = 0;
        code_size = BOOTROM_HEAD_OFFSET;
    } else {
        ret = dev_flash_read_mtd_blk(area_info->part_name, (unsigned char *)&code_size,
            IMAGE_CODE_LEN, (area_info->offset + IMAGE_CODE_LEN_OFFSET));
        if (ret != 0) {
            dev_upgrade_err("read flash fail.\n");
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_READ);
        }

        ret = dev_flash_read_mtd_blk(area_info->part_name, (unsigned char *)&magic,
            CMS_MAGIC_FLAG_SIZE, (area_info->offset + CMS_MAGIC_FLAG_OFFSET));
        if (ret != 0) {
            dev_upgrade_err("read flash magic fail.\n");
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_READ);
        }
    }

    /* Non-CMS verification: img_total_len = SEC_HEAD_SIZE + code_size
     * CMS verification: img_total_len stored in head offset 0x498
     */
    if (dev_upgrade_is_cms_support(magic, area_info->component_type) == true) {
        ret = dev_flash_read_mtd_blk(area_info->part_name, (unsigned char *)&fw_total_size,
            FILE_TOTAL_LEN_SIZE, (area_info->offset + FILE_TOTAL_LEN_OFFSET));
        if (ret != 0) {
            dev_upgrade_err("read fw total size fail.\n");
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_READ);
        }
        *length = fw_total_size;
    } else {
        if (code_size >= area_info->size) {
            dev_upgrade_err("invalid length, component(%u), length: %x, max_size: %x\n",
                area_info->component_type, *length, area_info->size);
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_LENGTH);
        }
        *length = code_size + SEC_HEAD_SIZE;
    }

    if (*length > area_info->size) {
        dev_upgrade_err("invalid length, component(%u), length: %x, max_size: %x\n",
            area_info->component_type, *length, area_info->size);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_LENGTH);
    }

    return ret;
}

int dev_upgrade_get_image_size(upg_comm_para *comm_para, unsigned int *length)
{
    int ret;
    component_area_info *comp_area_info = NULL;

    if (comm_para == NULL || length == NULL) {
        dev_upgrade_err("null pointer, comm_para: %pK.\n", comm_para);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    comp_area_info = dev_component_area_info_search(comm_para);
    if (comp_area_info == NULL) {
        dev_upgrade_err("dev(%d) get comp_area_info struct fail, type(%u) area(%u)\n",
            comm_para->dev_id, comm_para->type, comm_para->area);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    ret = dev_upgrade_get_size_from_flash(comp_area_info, length);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) get size from flash fail.\n", comm_para->dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_READ);
    }

    return ret;
}

int dev_upgrade_read_flash_image(upg_comm_para *comm_para, unsigned char *buff, unsigned int length)
{
    int ret;
    component_area_info *comp_area_info = NULL;

    if (comm_para == NULL || buff == NULL) {
        dev_upgrade_err("null pointer, comm_para: %pK.\n", comm_para);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    comp_area_info = dev_component_area_info_search(comm_para);
    if (comp_area_info == NULL) {
        dev_upgrade_err("dev(%d) get comp_area_info struct fail, type(%u) area(%u)\n",
            comm_para->dev_id, comm_para->type, comm_para->area);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    if (length > comp_area_info->size) {
        dev_upgrade_err("length(%u) invalid.\n", length);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_LENGTH);
    }
    ret = dev_flash_read_mtd_blk(comp_area_info->part_name, buff, length, comp_area_info->offset);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) read flash fail.\n", comm_para->dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_READ);
    }

    dev_upgrade_info("dev(%d) component_type%u length: %u.\n", comm_para->dev_id, comm_para->type, length);

    return ret;
}

int dev_upgrade_save_to_flash(upg_comm_para *comm_para, dev_upgrade_scheule_handle update_progress)
{
    int ret = -1;
    unsigned int index;
    unsigned int offset, block_nums, remain;
    unsigned int block_size = FLASH_BLOCK_SIZE;
    component_area_info *comp_area_info = NULL;
    unsigned char *wr_buff = NULL;

    if (comm_para == NULL || comm_para->file_content == NULL || update_progress == NULL) {
        dev_upgrade_err("null pointer, comm_para: %pK, update_progress: %pK\n",
            comm_para, update_progress);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    wr_buff = comm_para->file_content;
    comp_area_info = dev_component_area_info_search(comm_para);
    if (comp_area_info == NULL) {
        dev_upgrade_err("dev(%d) get comp_area_info struct fail, type(%u) area(%u)\n",
            comm_para->dev_id, comm_para->type, comm_para->area);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    block_nums = comm_para->file_size / block_size;
    remain = comm_para->file_size % block_size;

    offset = comp_area_info->offset;
    for (index = 0; index < block_nums; index++) {
        ret = dev_flash_write_mtd_blk(comp_area_info->part_name, wr_buff, block_size, offset);
        if (ret != 0) {
            dev_upgrade_err("dev(%d) write flash fail.\n", comm_para->dev_id);
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_WRITE);
        }

        offset += block_size;
        wr_buff += block_size;
        update_progress(comm_para->dev_id, comm_para->type, comm_para->file_size, ((index + 1) * block_size));

        if (!(index % FLASH_BLKS_ONCE)) {
            msleep(1);
        }
    }

    if (remain) {
        ret = dev_flash_write_mtd_blk(comp_area_info->part_name, wr_buff, remain, offset);
        if (ret != 0) {
            dev_upgrade_err("dev(%d) write flash fail.\n", comm_para->dev_id);
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_WRITE);
        }
    }

    update_progress(comm_para->dev_id, comm_para->type, comm_para->file_size, comm_para->file_size);

    dev_upgrade_event("dev(%d) type %u write flash success, size = %u, block_nums = %u\n",
        comm_para->dev_id, comm_para->type, comp_area_info->size, (block_nums + remain));

    return ret;
}
#endif

STATIC void dev_upgrade_table_dump(dev_component_list_ctrl *comp_list_ctrl)
{
    unsigned int i;

    dev_upgrade_debug("\n-----------------------------------------------------------\n"
                      "dev_id: %u, count: %u, bitmap: %x\n",
                      comp_list_ctrl->dev_id, comp_list_ctrl->count, comp_list_ctrl->bitmap);

    for (i = 0; i < comp_list_ctrl->count; i++) {
        dev_upgrade_debug("\ncomponent_type : %u\n"
                          "area           : %u\n"
                          "part_name      : %s\n"
                          "dev_part       : %s\n"
                          "size           : %x\n",
                          comp_list_ctrl->components[i].component_type,
                          comp_list_ctrl->components[i].area,
                          comp_list_ctrl->components[i].part_name,
                          comp_list_ctrl->components[i].dev_part,
                          comp_list_ctrl->components[i].size);
    }

    dev_upgrade_debug("\n-----------------------------------------------------------\n");
}

int dev_upgrade_get_component_bitmap(int dev_id, unsigned int *bitmap)
{
    dev_component_list_ctrl *comp_list_ctrl = NULL;

    if (bitmap == NULL) {
        dev_upgrade_err("dev(%d) bitmap is null.\n", dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    comp_list_ctrl = dev_upgrade_get_component_list_ctrl(dev_id);
    if (comp_list_ctrl == NULL) {
        dev_upgrade_err("get component list ctrl fail, dev(%d).\n", dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_INDEX);
    }

    *bitmap = comp_list_ctrl->bitmap;

    dev_upgrade_debug("dev_id: %d bitmap: %x.\n", dev_id, *bitmap);

    return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 15, 0)
STATIC void calculate_sha256(unsigned char *buf, unsigned int len, unsigned char *check)
{
    int ret;
    struct crypto_shash *sha256 = NULL;

    sha256 = crypto_alloc_shash("sha256", 0, 0);
    if (IS_ERR_OR_NULL(sha256)) {
        dev_upgrade_err("crypto_alloc_shash failed.\n");
        return;
    }

    do {
        SHASH_DESC_ON_STACK(shash, sha256);
        shash->tfm = sha256;

        ret = crypto_shash_init(shash);
        if (ret < 0) {
            dev_upgrade_err("crypto_shash_init failed. ret: %d\n", ret);
            goto out;
        }
        ret = crypto_shash_update(shash, buf, len);
        if (ret < 0) {
            dev_upgrade_err("crypto_shash_update failed. ret: %d\n", ret);
            goto out;
        }
        ret = crypto_shash_final(shash, check);
        if (ret < 0) {
            dev_upgrade_err("crypto_shash_final failed. ret: %d\n", ret);
        }
    } while (0);

out:
    crypto_free_shash(sha256);
    sha256 = NULL;
}
#else
STATIC void calculate_sha256(unsigned char *buf, unsigned int len, unsigned char *check)
{
    struct scatterlist sg;
    struct hash_desc desc;
    int ret;

    ret = memset_s(check, NVE_CRC_LENGTH, 0, NVE_CRC_LENGTH);
    if (ret != 0) {
        dev_upgrade_err("memset_s failed.\n");
        return;
    }

    desc.flags = 0;
    desc.tfm = crypto_alloc_hash("sha256", 0, 0);

    if (IS_ERR_OR_NULL(desc.tfm)) {
        dev_upgrade_err("crypto_alloc_hash failed.\n");
        return;
    }

    sg_init_one(&sg, buf, len);

    ret = crypto_hash_init(&desc);
    if (ret != 0) {
        dev_upgrade_err("crypto_hash_init failed.\n");
        goto out;
    }
    ret = crypto_hash_update(&desc, &sg, len);
    if (ret != 0) {
        dev_upgrade_err("crypto_hash_update failed.\n");
        goto out;
    }
    ret = crypto_hash_final(&desc, check);
    if (ret != 0) {
        dev_upgrade_err("crypto_hash_update failed.\n");
    }
out:
    crypto_free_hash(desc.tfm);
    desc.tfm = NULL;
}
#endif

int dev_upgrade_nve_sha256_check(unsigned char *buff_addr, unsigned int valid_items, unsigned int len)
{
    int ret;
    unsigned char *crc_data_buf = NULL;
    unsigned int nve_bin_offset;
    unsigned int *p_nve_bin_offset = NULL;
    unsigned int *p_nve_crc_support = NULL;
    unsigned int i_end;
    unsigned int i_start;
    unsigned int i_count = 0;
    unsigned char *ram_buf = NULL;
    unsigned char buff_sha256[NVE_CRC_LENGTH] = {0};

    DRV_CHECK_RETV((buff_addr != NULL), ERRNO_NULL_POINT);
    DRV_CHECK_RETV((len <= NVE_AREA_LEN), ERRNO_INVALID_LENGTH);

    ram_buf = (unsigned char *)buff_addr;
    dev_upgrade_info("len = %u\n", len);
    if (len < NVE_CRC_SUPPORT || len <= NVE_CRC_LENGTH || valid_items < 1) {
        dev_upgrade_err("dev_upgrade_nve_sha256_check buff length is invalid! length = %u, valid_items = %u\n", len,
            valid_items);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }
    if (len < NVE_HEAD_LENGTH + ((long)valid_items - 1) * NVE_BIN_ENTRY_LEN + NVE_BIN_SIZE_OFFSET) {
        dev_upgrade_err("dev_upgrade_nve_sha256_check buff length is invalid! length = %d\n", len);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    p_nve_bin_offset =
        (unsigned int *)&ram_buf[NVE_HEAD_LENGTH + ((long)valid_items - 1) * NVE_BIN_ENTRY_LEN + NVE_BIN_ENTRY_OFFSET];
    nve_bin_offset = *p_nve_bin_offset + (unsigned int)(NVE_BIN_FILE_LEN - 1);
    if (nve_bin_offset > len) {
        dev_upgrade_err("dev_upgrade_nve_sha256_check nve_bin_offset is invalid! length = %d\n", len);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    p_nve_crc_support = (unsigned int *)&(ram_buf[NVE_CRC_SUPPORT]);
    if (*p_nve_crc_support != 0x1) {
        dev_upgrade_debug("crc not support!\n");
        return 0;
    }

    crc_data_buf = (unsigned char *)vmalloc(len);
    if (crc_data_buf == NULL) {
        dev_upgrade_err("malloc call fail\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_MALLOC);
    }

    ret = memset_s((void *)crc_data_buf, len, 0, len);
    if (ret != 0) {
        dev_upgrade_err("memset_s after malloc fail!\n");
        DEV_UPGRADE_VFREE(crc_data_buf);
        return ret;
    }

    i_end = (unsigned int)(NVE_SHA256_OFFSET);
    i_start = 0;
    for (i_count = i_start; i_count < i_end; i_count++) {
        crc_data_buf[i_count] = ram_buf[(unsigned int)(i_count)];
    }

    i_end = (unsigned int)(NVE_RESERVE_HEAD_OFFSET);
    i_start = (unsigned int)(NVE_SHA256_OFFSET);
    for (i_count = i_start; i_count < i_end; i_count++) {
        crc_data_buf[i_count] = 0;
    }

    i_start = (unsigned int)(NVE_RESERVE_HEAD_OFFSET);
    i_end = len;
    for (i_count = i_start; i_count < i_end; i_count++) {
        crc_data_buf[i_count] = ram_buf[(unsigned int)(i_count)];
    }

    calculate_sha256(crc_data_buf, len, buff_sha256);
    if (memcmp((unsigned char *)&(ram_buf[NVE_SHA256_OFFSET]), buff_sha256, (int)NVE_CRC_LENGTH)) {
        dev_upgrade_err("NVE SHA256 memcmp call fail\n");
        dev_upgrade_print_buff(&(ram_buf[NVE_SHA256_OFFSET]), (unsigned int)NVE_CRC_LENGTH);
        dev_upgrade_print_buff(&(buff_sha256[0]), (unsigned int)NVE_CRC_LENGTH);
        DEV_UPGRADE_VFREE(crc_data_buf);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_HASH);
    }

    DEV_UPGRADE_VFREE(crc_data_buf);

    return 0;
}

int dev_nve_check_back(const char *part_name, unsigned int len, unsigned int valid_item)
{
    unsigned char *buff = NULL;
    int ret;

    if (len > 0 && len <= NVE_AREA_LEN) {
        buff = (unsigned char *)vmalloc(len);
    } else {
        dev_upgrade_err("dev_nve_check_back malloc size cannot be 0!\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    if (buff == NULL) {
        dev_upgrade_err("dev_nve_check_back malloc fail.\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_MALLOC);
    }

    ret = memset_s((void *)buff, len, 0, len);
    if (ret != 0) {
        dev_upgrade_err("dev_nve_check_back memset_s fail: %d\n", ret);
        DEV_UPGRADE_VFREE(buff);
        return ret;
    }
    ret = dev_flash_read_mtd_blk(part_name, buff, len, (unsigned int)0);
    if (ret != 0) {
        dev_upgrade_err("dev_nve_check_back dev_flash_read_mtd_blk fail.\n");
        DEV_UPGRADE_VFREE(buff);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_READ);
    }

    if (len < NVE_CRC_SUPPORT + 1) {
        dev_upgrade_err("read len should not be less than NVE_CRC_SUPPORT!\n");
        DEV_UPGRADE_VFREE(buff);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    if (dev_upgrade_nve_sha256_check(buff, valid_item, len)) {
        dev_upgrade_err("dev_nve_check_back nve_crc_check fail!\n");
        DEV_UPGRADE_VFREE(buff);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_CRC);
    }

    DEV_UPGRADE_VFREE(buff);
    dev_upgrade_debug("\ndev_nve_check_back success!\n");

    return 0;
}

int dev_upgrade_nve_update(upg_comm_para *comm_para, unsigned char *buff)
{
    unsigned int valid_item;
    unsigned int *p_valid_item = NULL;
    component_area_info *comp_area_info = NULL;
    unsigned int *p_nve_last_size = NULL;
    unsigned int nve_write_size;
    unsigned int *p_nve_last_bin_size = NULL;
    int ret;

    DRV_CHECK_RETV((buff != NULL), ERRNO_NULL_POINT);

    comp_area_info = dev_component_area_info_search(comm_para);
    if (comp_area_info == NULL) {
        dev_upgrade_err("get comp_area_info struct fail.\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    p_valid_item = (unsigned int *)&buff[NVE_VALID_ITEMS_START_ADDRESS];
    valid_item = *p_valid_item;
    if (valid_item == 0) {
        dev_upgrade_err("valid nvbin num is not valid:%d or comp_to_addr->dev_part is NULL!\n", valid_item);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    p_nve_last_size = (unsigned int *)&buff[(unsigned int)(NVE_HEAD_LENGTH + (valid_item - 1) * NVE_BIN_ENTRY_LEN +
        NVE_BIN_ENTRY_OFFSET)];
    p_nve_last_bin_size = (unsigned int *)&buff[(unsigned int)(NVE_HEAD_LENGTH + (valid_item - 1) * NVE_BIN_ENTRY_LEN +
        NVE_BIN_SIZE_OFFSET)];
    nve_write_size = *p_nve_last_size + *p_nve_last_bin_size;
    dev_upgrade_info("nve_write_size = %u\n", nve_write_size);

    // 2.nve update
    if (dev_flash_write_mtd_blk(comp_area_info->part_name, buff, nve_write_size,
        (unsigned int)NVE_WRITE_DATA_ADDRESS)) {
        dev_upgrade_err("nve_update dev_flash_write_mtd_blk fail!\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_WRITE);
    }
    // 3.nve check
    ret = dev_nve_check_back(comp_area_info->part_name, nve_write_size, valid_item);
    if (ret) {
        dev_upgrade_err("nve_update dev_nve_check_back fail!\n");
        return ret;
    }

    return 0;
}


int dev_upgrade_component_space_check(int dev_id, unsigned int component_type, unsigned int size)
{
    component_area_info *comp_area_info = NULL;
    upg_comm_para comm_para = { 0 };

    comm_para.dev_id = dev_id;
    comm_para.type = component_type;
    comm_para.area = 0;

    comp_area_info = dev_component_area_info_search(&comm_para);
    if (comp_area_info == NULL) {
        dev_upgrade_err("get comp_area_info struct fail.\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    if (comp_area_info->size == FLASH_SIZE_CHECK_IGNORE) {
        dev_upgrade_info("component%u skips flash size check.\n", component_type);
    } else if (comp_area_info->size < size) {
        dev_upgrade_err("component%u space not enough, flash size: %x, act size: %x\n", component_type,
            comp_area_info->size, size);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_LENGTH);
    }

    return 0;
}

STATIC int dev_upgrade_op_cmd_proc(boot_area_info *bios_info, unsigned int op_flag, unsigned int *op_area)
{
    int ret = 0;
    void __iomem *map_base = NULL;
    unsigned int value, idx;

    map_base = ioremap(bios_info->base_addr, SIZE_OF_64K);
    if (map_base == NULL) {
        dev_upgrade_err("mmap fail.\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_SOURCE_NO);
    }

    if (op_flag == DEV_GET_CURR_BOOT_AREA) {
        /*
          mini--sysctrl reg(0x1100c3448 byte1)
          0x1--area0 boot
          0x2--area1 boot
        */
        value = *(volatile unsigned int *)(map_base + bios_info->boot_area_ofs);
        idx = ((value >> bios_info->boot_area_shift) & bios_info->boot_area_mask);
        if ((idx != BOOT_FROM_AREA0) && (idx != BOOT_FROM_AREA1)) {
            dev_upgrade_err("boot area info err: idx=%u\n", idx);
            ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_NONSUPPORT_ITEM);
            goto out;
        }
        *op_area = (idx == BOOT_FROM_AREA0) ? FLASH_AREA_0 : FLASH_AREA_1;
    } else if (op_flag == DEV_CLEAR_BOOT_COUNT) {
        /* upgrade or success, need clear boot count */
        value = *(volatile unsigned int *)(map_base + bios_info->boot_cnt_ofs);
        value &= ~(bios_info->boot_cnt_mask);
        *(volatile unsigned int *)(map_base + bios_info->boot_cnt_ofs) = value;
    } else {
        dev_upgrade_err("dev_upgrade_component_boot_area_op fail, op_flag(%x)err.\n", op_flag);
    }

out:
    iounmap(map_base);
    map_base = NULL;
    return ret;
}

STATIC void dev_upgrade_init_boot_info(int dev_id, boot_area_info *bios_info, unsigned int component_type)
{
    unsigned int type_to_offset[DSMI_COMPONENT_TYPE_MAX] = {
        0xffffffff, SC_BAK_DATA14, 0xffffffff, ISRAM_UEFI_OFFSET, 0xffffffff, 0xffffffff,
        0xffffffff, 0xffffffff, ISRAM_IMU_OFFSET, ISRAM_IMP_OFFSET, ISRAM_ACPI_OFFSET, 0xffffffff
    };
    bool rc_flag = devdrv_is_pci_rc_mode();

    bios_info->base_addr = SYSCTRL_REG_BASE;
    if (component_type == DSMI_COMPONENT_TYPE_UEFI && rc_flag) {
        bios_info->boot_area_ofs = SC_BAK_DATA14;
    } else {
        bios_info->boot_area_ofs = type_to_offset[component_type];
    }
    bios_info->boot_area_shift = 8;
    bios_info->boot_area_mask = 0xff;
    bios_info->boot_cnt_ofs = SC_SOFT_POR_RSV3;
    bios_info->boot_cnt_mask = 0xffffffff;

    return ;
}

int dev_upgrade_component_boot_area_op(int dev_id, unsigned int component_type,
                                       unsigned int op_flag, unsigned int *op_area)
{
    int ret;
    boot_area_info bios_info = { 0 };
    /**
     * There is currently no active / standby area.
     * By default, the primary partition is used directly.
     */
#if (defined CFG_SOC_PLATFORM_MINIV2)
    if (op_area != NULL) {
        *op_area = FLASH_AREA_0;
    }
    return 0;
#endif

    if (component_type >= DSMI_COMPONENT_TYPE_MAX) {
        dev_upgrade_err("dev(%d) component_type: %u is invalid\n", dev_id, component_type);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_INDEX);
    }

    if (op_flag == DEV_GET_CURR_BOOT_AREA || op_flag == DEV_GET_BOOT_FAIL_COUNT) {
        if (op_area == NULL) {
            dev_upgrade_err("dev(%d) op_area is null\n", dev_id);
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
        }
    }

    dev_upgrade_init_boot_info(dev_id, &bios_info, component_type);

    ret = dev_upgrade_op_cmd_proc(&bios_info, op_flag, op_area);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) dev_upgrade_op_cmd_proc fail\n", dev_id);
        return ret;
    }

    return 0;
}

STATIC int dev_upgrade_flash_copy(const component_area_info *src_area_info, component_area_info *dest_area_info)
{
    int ret;
    unsigned char *buff = NULL;

    buff = (unsigned char *)vmalloc(dest_area_info->size);
    if (buff == NULL) {
        dev_upgrade_err("vmalloc fail.\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_MALLOC);
    }

    ret = memset_s((void *)buff, dest_area_info->size, 0, dest_area_info->size);
    if (ret != 0) {
        dev_upgrade_err("memset_s fail: %d\n", ret);
        DEV_UPGRADE_VFREE(buff);
        return ret;
    }

    ret = dev_flash_read_mtd_blk(src_area_info->part_name, buff, dest_area_info->size, 0);
    if (ret != 0) {
        DEV_UPGRADE_VFREE(buff);
        dev_upgrade_err("read flash fail.\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_READ);
    }

    ret = dev_flash_write_mtd_blk(dest_area_info->part_name, buff, dest_area_info->size, 0);
    if (ret != 0) {
        DEV_UPGRADE_VFREE(buff);
        dev_upgrade_err("write flash fail.\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_WRITE);
    }

    DEV_UPGRADE_VFREE(buff);

    return 0;
}

int dev_upgrade_component_image_copy(int dev_id, unsigned int component_type,
    unsigned int dest_area, unsigned int src_area)
{
    int ret;
    component_area_info *src_area_info = NULL;
    component_area_info *dest_area_info = NULL;
    upg_comm_para comm_para = { 0 };

    comm_para.dev_id = dev_id;
    comm_para.type = component_type;
    comm_para.area = src_area;
    src_area_info = dev_component_area_info_search(&comm_para);
    if (src_area_info == NULL) {
        dev_upgrade_err("dev(%d) get comp_area_info struct fail, type(%u) area(%u)\n",
            comm_para.dev_id, comm_para.type, comm_para.area);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    comm_para.area = dest_area;
    dest_area_info = dev_component_area_info_search(&comm_para);
    if (dest_area_info == NULL) {
        dev_upgrade_err("dev(%d) get comp_area_info struct fail, type(%u) area(%u)\n",
            comm_para.dev_id, comm_para.type, comm_para.area);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    if (src_area_info->size == 0 || src_area_info->size > UPGRADE_FILE_SIZE_MAX) {
        dev_upgrade_err("dev(%d) dest_area_info->size: %u is invalid.\n", dev_id, dest_area_info->size);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_LENGTH);
    }

    ret = dev_upgrade_flash_copy(src_area_info, dest_area_info);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) dev_upgrade_flash_copy fail.\n", dev_id);
        return ret;
    }

    dev_upgrade_event("dev(%d) component(%u) image copy from %u to %u succ, size = %u\n",
        dev_id, component_type, src_area, dest_area, dest_area_info->size);
    return ret;
}

STATIC bool dev_upgrade_part_name_is_found(struct hisfc_mtd_info_st *mtd_part, int part_num, const char *part_name)
{
    int i;

    for (i = 0; i < part_num; i++) {
        if (strcmp((char *)mtd_part[i].mtdName, part_name) == 0) {
            return true;
        }
    }
    return false;
}

int dev_upgrade_flash_part_match(int dev_id,
    component_area_info *comp_area_info, dev_component_list_ctrl *comp_list_ctrl)
{
    int ret;
    unsigned int find_cnt = 0;
    unsigned int read_count;
    unsigned int total_count = 0;
    struct hisfc_mtd_info_st *mtd_part = NULL;

    ret = hisi_sfc_ctl_flash_read_mtd_info(0, &total_count, NULL);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) hisi_sfc_ctl_flash_read_mtd_info fail, ret: %d\n", dev_id, ret);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_READ);
    }

    if ((total_count == 0) || total_count > HISFC_PART_NUM_MAX) {
        dev_upgrade_err("dev(%d) total_count: %u invalid\n", dev_id, total_count);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_INDEX);
    }

    mtd_part = (struct hisfc_mtd_info_st *)kzalloc((total_count) * sizeof(struct hisfc_mtd_info_st),
        GFP_KERNEL | __GFP_ACCOUNT);
    if (mtd_part == NULL) {
        dev_upgrade_err("dev(%d) kzalloc failed!\n", dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_MALLOC);
    }

    read_count = total_count;
    ret = hisi_sfc_ctl_flash_read_mtd_info(read_count, &total_count, mtd_part);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) hisi_sfc_ctl_flash_read_mtd_info fail, ret: %d\n", dev_id, ret);
        DEV_UPGRADE_KFREE(mtd_part);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_READ);
    }

    /* compo_area_info matches the name of the list acquired by the flash, if match, adds to comp_list_ctrl */
    while (comp_area_info->component_type != DSMI_COMPONENT_TYPE_MAX) {
        if (dev_upgrade_part_name_is_found(mtd_part, total_count, comp_area_info->part_name) == true) {
            if (find_cnt >= COMPONENT_LIST_MAX) {
                dev_upgrade_err("dev(%d) find_cnt is invalid\n", dev_id);
                DEV_UPGRADE_KFREE(mtd_part);
                return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_INDEX);
            }
            comp_list_ctrl->components[find_cnt] = *comp_area_info;
            comp_list_ctrl->bitmap |= (1U << comp_area_info->component_type);
            find_cnt++;
        }

        comp_area_info++;
    }

    dev_upgrade_info("dev(%d) total_count: %u, find_cnt = %u\n", dev_id, total_count, find_cnt);

    comp_list_ctrl->dev_id = dev_id;
    comp_list_ctrl->count = find_cnt;
    DEV_UPGRADE_KFREE(mtd_part);

    return 0;
}

STATIC int dev_upgrade_fs_part_match(int dev_id, component_area_info *comp_area_info,
    dev_component_list_ctrl *comp_list_ctrl)
{
    unsigned int count = comp_list_ctrl->count;

    /* compo_area_info matches the name of the list acquired by the flash, if match, adds to comp_list_ctrl */
    while (comp_area_info->component_type != DSMI_COMPONENT_TYPE_MAX) {
        if (comp_area_info->store_in == STORE_IN_FILESYSTEM) {
            if (count >= COMPONENT_LIST_MAX) {
                dev_upgrade_err("dev(%d) find_cnt is invalid\n", dev_id);
                return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_INDEX);
            }
            comp_list_ctrl->components[count] = *comp_area_info;
            comp_list_ctrl->bitmap |= (1U << comp_area_info->component_type);
            count++;
        }

        comp_area_info++;
    }

    comp_list_ctrl->count = count;

    return 0;
}

int dev_upgrade_component_info_match(int dev_id, unsigned int *match_nums)
{
    int ret;
    component_area_info *comp_area_info = NULL;
    dev_component_list_ctrl *comp_list_ctrl = NULL;

    if (match_nums == NULL) {
        dev_upgrade_err("dev(%d) scan_nums is null\n", dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    comp_area_info = dev_upgrade_get_component_static_table(dev_id);
    if (comp_area_info == NULL) {
        dev_upgrade_err("get component table fail, dev(%d).\n", dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_INDEX);
    }

    comp_list_ctrl = dev_upgrade_get_component_list_ctrl(dev_id);
    if (comp_list_ctrl == NULL) {
        dev_upgrade_err("get component list ctrl fail, dev(%d).\n", dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_INDEX);
    }

    ret = memset_s((void *)comp_list_ctrl, sizeof(dev_component_list_ctrl),
                   0, sizeof(dev_component_list_ctrl));
    if (ret != 0) {
        dev_upgrade_err("dev(%d) memset_s fail: %d\n", dev_id, ret);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_INIT);
    }

    ret = dev_upgrade_flash_part_match(dev_id, comp_area_info, comp_list_ctrl);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) component list config fail, ret: 0x%x\n", dev_id, ret);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_INIT);
    }

    if (comp_list_ctrl->count != 0) {
        ret = dev_upgrade_fs_part_match(dev_id, comp_area_info, comp_list_ctrl);
        if (ret != 0) {
            dev_upgrade_err("dev(%d) component list config fail, ret: 0x%x\n", dev_id, ret);
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_INIT);
        }
    }

    *match_nums = comp_list_ctrl->count;
    dev_upgrade_table_dump(comp_list_ctrl);

    return 0;
}
#if (!defined SUPPORT_TEE_SECURE_UPGRADE)
STATIC int dev_upgrade_xloader_file_type_check(const unsigned char *file_content, unsigned int file_size)
{
    unsigned char *code_base = NULL;
    DRV_CHECK_RETV((file_size > (IMAGE_CODE_OFFSET + XLOADER_FILE_TYPE_CHECK_OFFSET + XLOADER_FILE_TYPE_CHECK_LEN)),
        ERRNO_INVALID_LENGTH);
    code_base = (unsigned char *)(uintptr_t)(((uintptr_t)file_content) + IMAGE_CODE_OFFSET +
                                             XLOADER_FILE_TYPE_CHECK_OFFSET);
    if (memcmp((void *)code_base, (void *)XLOADER_FILE_TYPE_CHECK_FLAG, XLOADER_FILE_TYPE_CHECK_LEN) == 0) {
        return 0;
    }

    return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FILE_TYPE);
}

STATIC int dev_upgrade_uefi_file_type_check(const unsigned char *file_content, unsigned int file_size)
{
    unsigned char *code_base = NULL;
    upgrade_efi_guid_t check_guid2 = UEFI_FILE_TYPE_CHECK_FLAG_EUID2;
    upgrade_efi_guid_t check_guid3 = UEFI_FILE_TYPE_CHECK_FLAG_EUID3;
    DRV_CHECK_RETV((file_size > (IMAGE_CODE_OFFSET + UEFI_FILE_TYPE_CHECK_OFFSET_SIGNATURE +
        UEFI_FILE_TYPE_CHECK_SIGNATURE_LEN)), ERRNO_INVALID_LENGTH);
    code_base = (unsigned char *)(uintptr_t)(((uintptr_t)file_content) + IMAGE_CODE_OFFSET +
        UEFI_FILE_TYPE_CHECK_OFFSET_EUID);
    /* check guid */
    if ((memcmp((void *)code_base, (void *)&check_guid2, UEFI_FILE_TYPE_CHECK_EUID_LEN) != 0) &&
        (memcmp((void *)code_base, (void *)&check_guid3, UEFI_FILE_TYPE_CHECK_EUID_LEN) != 0)) {
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FILE_TYPE);
    }

    code_base = (unsigned char *)(uintptr_t)(((uintptr_t)file_content)
                                             + IMAGE_CODE_OFFSET + UEFI_FILE_TYPE_CHECK_OFFSET_SIGNATURE);
    /* check Signature */
    if (memcmp((void *)code_base, (void *)UEFI_FILE_TYPE_CHECK_FLAG_SIGNATURE,
               UEFI_FILE_TYPE_CHECK_SIGNATURE_LEN) != 0) {
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FILE_TYPE);
    }

    return 0;
}

STATIC int dev_upgrade_file_type_check(unsigned int component_type, const unsigned char *file_content,
                                       unsigned int file_size)
{
    int ret = 0;
    switch (component_type) {
        case DSMI_COMPONENT_TYPE_XLOADER:
            ret = dev_upgrade_xloader_file_type_check(file_content, file_size);
            break;
        case DSMI_COMPONENT_TYPE_UEFI:
            ret = dev_upgrade_uefi_file_type_check(file_content, file_size);
            break;
        default:
            break;
    }
    if (ret != 0) {
        dev_upgrade_err("file type:%d check failed:%d.\n", component_type, ret);
    }
    return ret;
}

STATIC int dev_upgrade_non_cms_bootrom_verification(int dev_id, unsigned int type,
    unsigned char *file_content, unsigned int file_size)
{
    int ret;
    unsigned int head_offset;
    unsigned char *code_base = NULL;
    unsigned int img_len = *(unsigned int *)(file_content + IMAGE_CODE_LEN_OFFSET);

    ret = verify_init_session(dev_id);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) verify_init_session fail, ret: %d\n", dev_id, ret);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_HASH_CHECK);
    }

    if (file_size < (BOOTROM_HEAD_OFFSET + img_len)) {
        verify_finalize_session(dev_id);
        dev_upgrade_err("dev(%d) file_size(%u) invalid.\n", dev_id, file_size);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_LENGTH);
    }

    head_offset = BOOTROM_HEAD_OFFSET;
    code_base = file_content;
    if (sec_update_verification(dev_id, (const unsigned char *)file_content, head_offset,
                                (const unsigned char *)code_base, file_size, PKCS_SIGN_TYPE_ON) != 0) {
        verify_finalize_session(dev_id);
        dev_upgrade_err("dev(%d) sec_update_verification fail.\n", dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_HASH_CHECK);
    }

    verify_finalize_session(dev_id);
    return 0;
}


STATIC int dev_upgrade_non_cms_verification(int dev_id, unsigned int type,
    unsigned char *file_content, unsigned int file_size)
{
    int ret;
    unsigned int head_offset;
    unsigned char *code_base = NULL;

    ret = dev_upgrade_file_type_check(type, file_content, file_size);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) dev_upgrade_file_type_check fail, ret: %d\n", dev_id, ret);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FILE_TYPE);
    }

    ret = verify_init_session(dev_id);
    if (ret != 0) {
        dev_upgrade_err("dev(%d) verify_init_session fail, ret: %d\n", dev_id, ret);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_HASH_CHECK);
    }

    head_offset = 0x00;
    code_base = (unsigned char *)(uintptr_t)(((uintptr_t)file_content) + IMAGE_CODE_OFFSET);
    if (sec_update_verification(dev_id, (const unsigned char *)file_content, head_offset,
                                (const unsigned char *)code_base, file_size, PKCS_SIGN_TYPE_ON) != 0) {
        verify_finalize_session(dev_id);
        dev_upgrade_err("dev(%d) sec_update_verification fail.\n", dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_HASH_CHECK);
    }

    verify_finalize_session(dev_id);
    return 0;
}

int dev_upgrade_sec_verification(int dev_id, unsigned int type, unsigned char *file_content, unsigned int file_size)
{
    int ret;
    unsigned long long magic;

    if (file_content == NULL) {
        dev_upgrade_err("file_content is NULL\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }
    if ((type >= DSMI_COMPONENT_TYPE_MAX) || (dev_id >= DEVICE_NUM_MAX)) {
        dev_upgrade_err("dev(%u) type(%u) para invalid\n", dev_id, type);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NONSUPPORT_ITEM);
    }
    if (file_size < SEC_HEAD_SIZE) {
        dev_upgrade_err("file_size(%u) invalid\n", file_size);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NONSUPPORT_ITEM);
    }

    magic = *(unsigned long long *)(file_content + CMS_MAGIC_FLAG_OFFSET);

    if (dev_upgrade_is_cms_support(magic, type) == true) {
        /* This is for compatibility with earlier versions. */
        return 0;
    } else {
        if (type == DSMI_COMPONENT_TYPE_BOOTROM) {
            ret = dev_upgrade_non_cms_bootrom_verification(dev_id, type, file_content, file_size);
        } else {
            ret = dev_upgrade_non_cms_verification(dev_id, type, file_content, file_size);
        }
        if (ret != 0) {
            dev_upgrade_err("dev(%d) non cms verification fail, ret: %d\n", dev_id, ret);
            return ret;
        }
    }

    return 0;
}

void dev_upgrade_clear_localcfg(int dev_id)
{
    int ret;

    ret = dev_flash_erase_mtd_blk(PART_NAME_USER_CONFIG, DEV_FLASH_DOUBLE_AREAS * PART_NAME_USER_CONFIG_SIZE,
                                  PART_NAME_USER_CONFIG_OFFSET);
    if (ret != 0) {
        dev_upgrade_err("dev_flash_erase_mtd_blk err, Reset user config fail, ret: %d\n", ret);
        return;
    }

    dev_upgrade_event("Reset user config done\n");

    return;
}
#endif

int dev_upgrade_verify_image(int dev_id, const char *file_path, int mode)
{
    int ret;
    char *file_data = NULL;
    unsigned int file_size = 0;
    struct file *filp = NULL;

    /* the file may not exist, so print warnning */
    filp = filp_open(file_path, O_RDWR | O_LARGEFILE, 0);
    if (IS_ERR_OR_NULL(filp)) {
        dev_upgrade_warn("unable to open file: %s (%ld)\n", file_path, PTR_ERR(filp));
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_OPEN);
    }
    ret = dev_upgrade_get_fs_file_size_fp(filp, &file_size);
    if (ret != 0) {
        dev_upgrade_err("dev_id(%d) dev_upgrade_get_fs_file_size_fp fail, ret=%d.\n", dev_id, ret);
        filp_close(filp, NULL);
        filp = NULL;
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_READ);
    }
    if ((file_size > AICPU_SIZE_MAX) || (file_size < SEC_HEAD_SIZE + NVCNT_INFO_SIZE)) {
        filp_close(filp, NULL);
        filp = NULL;
        dev_upgrade_err("dev_id(%d) file size[%d] is out of range.\n", dev_id, file_size);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    file_data = (char *)vzalloc(file_size);
    if (file_data == NULL) {
        filp_close(filp, NULL);
        filp = NULL;
        dev_upgrade_err("dev_id(%d) vzalloc is null.\n", dev_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    ret = dev_upgrade_read_fs_file_fp(filp, 0, file_data, file_size);
    if (ret != 0) {
        filp_close(filp, NULL);
        filp = NULL;
        dev_upgrade_err("dev_id(%d) dev_upgrade_read_fs_file_fp fail, ret: %d\n", dev_id, ret);
        DEV_UPGRADE_VFREE(file_data);
        return ret;
    }

    ret = dev_upgrade_sec_verification(dev_id, DSMI_COMPONENT_TYPE_AICPU, file_data, file_size);
    if (ret != 0) {
        filp_close(filp, NULL);
        filp = NULL;
        dev_upgrade_err("dev(%d) dev_upgrade_sec_verification fail, ret: %d\n", dev_id, ret);
        DEV_UPGRADE_VFREE(file_data);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_HASH_CHECK);
    }

    filp_close(filp, NULL);
    filp = NULL;
    if (mode & COVER_FILE_WITHOUT_HEAD) {
        dev_upgrade_info("dev(%d) cover file without head.\n", dev_id);
        ret = dev_upgrade_write_fs_file(file_path, (char *)(file_data + SEC_HEAD_SIZE),
            (file_size - SEC_HEAD_SIZE - NVCNT_INFO_SIZE),  S_IRUSR | S_IWUSR | S_IRGRP);
        if (ret != 0) {
            dev_upgrade_err("cover file without head fail, ret: %d\n", ret);
            DEV_UPGRADE_VFREE(file_data);
            return ret;
        }
    }

    DEV_UPGRADE_VFREE(file_data);
    return ret;
}
STATIC int upgrade_read_efuse_reg(u32 dev_id, u64 efuse_offset, u32 *out_value)
{
    void __iomem *efuse_vir_addr = NULL;
    u32 reg_data;

    efuse_vir_addr = (void __iomem *)ioremap_wc(efuse_offset + dev_id, EFUSE_REG_READ_LEN);
    if (efuse_vir_addr == NULL) {
        dev_upgrade_err("io remap fail.(devid=%u)", dev_id);
        return -EINVAL;
    }
    reg_data = readl_relaxed(efuse_vir_addr);
    iounmap(efuse_vir_addr);
    efuse_vir_addr = NULL;

    *out_value = reg_data;
    return 0;
}
STATIC int dev_upgrade_get_root_hash(int dev_id, u32 *efuse_hash)
{
    u32 i;
    int ret;
    u8 tmp;
    u32 reverse_len = EFUSE_HASH_LEN >> 1;
    u64 offset = HW_HASH_OFFSET;

    for (i = 0; i < EFUSE_HASH_WORDS; i++) {
        ret = upgrade_read_efuse_reg(dev_id, offset + i * sizeof(u32), efuse_hash + i);
        if (ret != 0) {
            dev_upgrade_err("Ef hash read fail. (dev=%u; ret=%d)\n", dev_id, ret);
            return -1;
        }
    }
    for (i = 0; i < reverse_len; i++) {
        tmp = *((u8 *)efuse_hash + i);
        *((u8 *)efuse_hash + i) = *((u8 *)efuse_hash + EFUSE_HASH_LEN - i - 1);
        *((u8 *)efuse_hash + EFUSE_HASH_LEN - i - 1) = tmp;
    }
    return 0;
}

int dev_upgrade_get_check_info(int dev_id, IMG_CHECK_INFO *info)
{
    int ret;
    uint8_t zero_hash[EFUSE_HASH_LEN] = { 0 };

    info->nv_cnt = 0;
    info->pss_only = 0;
#ifdef CFG_FEATURE_PSS_SIGN
    ret = devdrv_config_get_pss_cfg(0, &info->pss_only);
    if (ret) {
        dev_upgrade_err("get pss config fail. (dev=%u; ret=%d)\n", dev_id, ret);
        return -1;
    }
#endif

    if (dev_upgrade_get_root_hash(dev_id, (u32 *)info->root_pk_hash)) {
        return -1;
    }
    if (memcmp(info->root_pk_hash, zero_hash, EFUSE_HASH_LEN) == 0) {
        info->need_check = 0;
        return 0;
    }
    info->need_check = 1;

    ret = upgrade_read_efuse_reg(dev_id, HW_CATEGORY_OFFSET, &info->subkey_category);
    if (ret) {
        dev_upgrade_err("efuse read subkey category fail. (dev=%u; ret=%d)\n", dev_id, ret);
        return -1;
    }
    info->subkey_id = 0;
    return 0;
}


#endif
