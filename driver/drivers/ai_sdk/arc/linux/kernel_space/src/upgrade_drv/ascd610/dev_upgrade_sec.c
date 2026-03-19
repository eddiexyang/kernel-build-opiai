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


/*
 * for security upgrade, need check package's integrity by tee.
 * after checked successfully, call read/write interface supplied by
 * Tee to write to flash storage.
 */

#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/ctype.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <asm/uaccess.h>
#include <linux/securec.h>
#include <linux/errno.h>
#include <linux/gfp.h>

#include "dev_upgrade_public.h"
#include "dev_upgrade_sec.h"
#include "dev_upgrade_core.h"
#include "dev_upgrade_adapt.h"
#include "user_cfg_interface.h"

#if (defined CFG_SOC_PLATFORM_MDC_V51)
#include "dev_upgrade_ufs.h"
#include "dev_upgrade_crl.h"
#include "drv_pkicms.h"
upgrade_ufs_component_info *dev_upgrade_ufs_get_local_component(unsigned int component_type);
#endif

#ifdef STATIC_SKIP
#  define STATIC
#else
#  define STATIC static
#endif

/**
 * use to get security upgrade version
 */
#define MDC_PKG_UFS_CONFIG "/etc/mdc_config.xml"
#define MDC_PKG_SSD_CONFIG "/etc/mdc_ssd_config.xml"
#define MDC_PKG_EMMC_CONFIG "/etc/mdc_emmc_config.xml"
#define MDC_PKG_CONFIG_SIZE     10240
#define ESBC_IMG_HEADER_OFFSET  0x2000
#define ROOTFS_IMG_HEADER_LEN   0x200000
#define ESBC_IMG_VERSION_LEN    16
#define ESBC_IMG_VERSION_START offsetof(img_ver_header, ver)
#define ESBC_IMG_VERSION_FRAME_LEN 8
#define STORAGE_OFFSET_MAX_LEN  16
#define ESBC_IMG_HEADER_SIZE    0x1000
#define MDC_CONFIG_PATH_SIZE    1024

#define GB_SIZE_OFFSET          21
#define STORAGE_CAPACITY_64GB   64
#define STORAGE_CAPACITY_128GB  128
#define STORAGE_CAPACITY_256GB  256

#define ROOTFS_BLOCK_READ_SIZE      0x500
#define ROOTFS_BLOCK_COUNT_OFFSET   0x404
#define ROOTFS_BLOCK_SIZE_OFFSET    0x418
#define HASH_TREE_SIZE              0x1400000          // 20M
#define BLOCK_SIZE_1K               1024
#define BLOCK_SIZE_2K               2048
#define BLOCK_SIZE_4K               4096
#define BLOCK_SIZE_0                0
#define BLOCK_SIZE_1                1
#define BLOCK_SIZE_2                2
#define INVALID_VALUE               -1
#define INVALID_VERSION             0

/**
 * larege_file_block_handle: large file block handler call back
 * @component: upgrade component data, include component name,
 *             file path, file size an so on
 * @read_buff: file block read from large file
 * @read_len:  bock size
 * @offset:    offset of block from the large file
 * @flag:      READ_CONTINUE/READ_END
 *
 * Description: For a large file, read a data block whose size does not
 * exceed the value of SEC_IMG_VERIFY_MAX_SIZE each time, and use
 * the data of the data block to call back this function for processing.
 * The current application scenario is as follows:
 * 1. Call the TEE to verify the upgrade component package;
 * 2. write data to Logical units of the UFS.
 */
#if (defined CFG_SOC_PLATFORM_MDC_V51)
typedef int(*larege_file_block_handle)(
    const struct upgrade_component *component,
    const mdc_package_t *mdc_pkg,
    const char *read_buff, u32 read_len, loff_t offset, int flag);
#endif

/**
 * After device upgraded. release resource
 */
void dev_upgrade_core_resource_release(dev_upgrade_core_ctrl *upgrade_ctrl);

/**
 * set upgrade progress
 */
int dev_upgrade_schedule_update(int dev_id, unsigned int component_type,
    unsigned int total_size, unsigned int finish_size);


/**
 * is_ufs_image: if file is store in ufs.
 * @component: upgrade component pointer
 *
 * Flash image content is saved in file_content filed.
 * If component is stored in UFS, the file_content is NULL,
 * and the image file name saved in sub_pkt filed.
 */
STATIC inline bool is_ufs_image(const struct upgrade_component *component)
{
    return component->file_content == NULL;
}

/**
 * dev_upgrade_verify_sec_large_file: verify large file in security upgrade
 * @component: upgrade component information, see @upgrade_component struct.
 *
 * If file is too large, need split file to multi block to verify.
 */
#if (defined CFG_SOC_PLATFORM_MDC_V51)
STATIC int dev_upgrade_handle_large_file(const struct upgrade_component *component,
    const mdc_package_t *mdc_pkg, larege_file_block_handle block_function_callback)
{
    int img_id;
    int ret = 0;
    char *read_buff = NULL;
    int read_flag = READ_CONTINUE;
    int read_len = 0;
    loff_t offset = 0;
    struct local_image_verify *local_img;

    local_img = dev_upgrade_get_local_imgs();
    read_buff = (char *)vmalloc(SEC_IMG_VERIFY_MAX_SIZE);
    if (!read_buff) {
        dev_upgrade_err("security upgrade large file failed. alloc memory failed.\n");
        return -ENOMEM;
    }
    dev_upgrade_get_img_id(mdc_pkg->file_name, &img_id);
    if (img_id >= IMG_ID_MAX_NUM) {
        dev_upgrade_err("error img_id: %d\n", img_id);
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_INDEX);
        goto _free_buff;
    }
    while (read_flag == READ_CONTINUE) {
        (void)memset_s(read_buff, SEC_IMG_VERIFY_MAX_SIZE, 0, SEC_IMG_VERIFY_MAX_SIZE);
        read_flag = dev_upgrade_read_img((uintptr_t)&local_img[img_id], read_buff, SEC_IMG_VERIFY_MAX_SIZE, &read_len);
        if (read_flag == READ_ERROR) {
            dev_upgrade_err("img_id: %d read file failed. read pos=%lld.\n", img_id, local_img[img_id].img_loff);
            ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_READ);
            goto _free_buff;
        }

        /* if the last read len is zero. return success. */
        if (read_len == 0) {
            dev_upgrade_info("read to the end of the file. filename=%s.", mdc_pkg->file_name);
            goto _free_buff;
        }

        ret = block_function_callback(component, mdc_pkg, (const char *)read_buff, (u32)read_len, offset, read_flag);
        if (ret) {
            dev_upgrade_err("%s : verify large file failed. ret=%d\n", __func__, ret);
            goto _free_buff;
        }
        if (read_flag == READ_END) {
            goto _free_buff;
        }

        /**
         * Each time the file is read successfully, the POS in the
         * pfile is automatically added to the current read position.
         */
        offset = local_img[img_id].img_loff;
    }

_free_buff:
    dev_upgrade_debug("%s : freeing read_buffer...\n", __func__);
    DEV_UPGRADE_VFREE(read_buff);
    return ret;
}

int dev_upgrade_write_ufs_file_block(
    const struct upgrade_component *component,
    const mdc_package_t *mdc_pkg,
    const char *read_buff, u32 read_len, loff_t offset, int flag)
{
    /**
     * A file may be stored in a non-start position of an LU.
     * Therefore, the storage offset of a block is the offset of the entire file
     * plus the offset of the current block relative to the file.
     */
    loff_t block_offset;
    block_offset = mdc_pkg->storage.start + offset;
    return dev_upgrade_disk_write(QUERY_FROM_BOOTSTRAP, mdc_pkg->storage.lun, block_offset, read_buff, read_len);
}
#ifdef LINUX_KERNEL_BUILD
int dev_upgrade_del_second_sign(int image_id, const struct upgrade_component *component,
    mdc_package_t *mdc_pkg, larege_file_block_handle block_function_callback)
{
    int ret, read_flag = READ_CONTINUE, code_offset = 0;
    char *read_buff = NULL, *first_addr = NULL;
    struct file *pfile = NULL;
    unsigned int read_len;
    second_sign_t image_input = {0};

    dev_upgrade_info("start to del second sign, file_name=%10s.\n", mdc_pkg->file_name);

    read_buff = (char *)vmalloc(SEC_IMG_VERIFY_MAX_SIZE);
    if (!read_buff) {
        dev_upgrade_err("security upgrade large file failed. alloc memory failed.\n");
        return -ENOMEM;
    }

    pfile = open_mdc_package(mdc_pkg->file_name);
    if (IS_ERR_OR_NULL(pfile)) {
        dev_upgrade_err("fail to open_mdc_package,file:%s.\n", mdc_pkg->file_name);
        ret = -ENODEV;
        goto _free_buff;
    }

    (void)memset_s(read_buff, SEC_IMG_VERIFY_MAX_SIZE, 0, SEC_IMG_VERIFY_MAX_SIZE);

    read_flag = read_mdc_package_content(pfile, read_buff, SEC_IMG_VERIFY_MAX_SIZE, &read_len);
    if ((read_flag == READ_ERROR) || (read_len == 0)) {
        dev_upgrade_err("read file failed, read_flag=%d, read_len=%d.\n", read_flag, read_len);
        ret = -ENODEV;
        goto _close_file;
    }

    first_addr = read_buff;
    dev_upgrade_info("old len: %d.\n", read_len);

    image_input.image_id = image_id, image_input.file_name = mdc_pkg->file_name;

    ret = pkicms_get_first_sign_addr(&image_input, read_buff, pfile, &code_offset, &read_len);
    if (ret || code_offset == 0) {
        dev_upgrade_info("%s: ret=%d, sign_flag=%d.", __func__, ret, code_offset);
        ret = -ENODEV;
        goto _close_file;
    }

    first_addr = read_buff + code_offset;

    dev_upgrade_info("new len: %d.\n", read_len);

    // TEE only need 1 time write
    ret = block_function_callback(component, mdc_pkg, (const char *)first_addr, read_len, 0, READ_END);
    if (ret) {
        dev_upgrade_err("%s : block_function_callback failed. ret=%d\n", __func__, ret);
        goto _close_file;
    }

_close_file:
    close_mdc_package(pfile);
    pfile = NULL;
_free_buff:
    DEV_UPGRADE_VFREE(read_buff);
    return ret;
}
#endif
#endif

int dev_upgrade_storage_ufs(struct upgrade_component *component)
{
#if (defined CFG_SOC_PLATFORM_MDC_V51)
    unsigned int i;
    int ret, image_id;
    mdc_package_t *mdc_pkg = NULL;
    for (i = 0; i < component->sub_pkt.sub_file_count; ++i) {
        mdc_pkg = component->sub_pkt.sub_file_info[i];
        if (IS_ERR_OR_NULL(mdc_pkg)) {
            dev_upgrade_err("sub file is null, index=%d\n", i);
            return -EINVAL;
        }

        dev_upgrade_info("file name=%s, lun=%u, offset=%llu, len=%llu\n", mdc_pkg->file_name,
            mdc_pkg->storage.lun, mdc_pkg->storage.start, mdc_pkg->storage.len);
#ifdef LINUX_KERNEL_BUILD
        dev_upgrade_get_img_id(mdc_pkg->file_name, &image_id);
        if (image_id == ITEE_IMG_ID) {
            ret = dev_upgrade_del_second_sign(image_id, component, mdc_pkg, dev_upgrade_write_ufs_file_block);
            if (!ret) { // if fail, continue old process
                return ret;
            }
        }
#endif
        ret = dev_upgrade_handle_large_file(
            component, mdc_pkg, dev_upgrade_write_ufs_file_block);
        if (ret) {
            dev_upgrade_err("verify mdc package failed. file index=%d\n", i);
            return ret;
        }
    }
    return 0;
#else
    dev_upgrade_err("%s: only mdc device upgrade support UFS.\n", __func__);
    return -EINVAL;
#endif
}

#if defined(CFG_SOC_PLATFORM_MDC_LITE_ESL) || defined(CFG_SOC_PLATFORM_MINIV3)
// esl don't support tee
int sec_img_update(unsigned int dev_id, unsigned int idx)
{
    dev_upgrade_err("sec_img_update error, esl don't support tee");
    return 0;
}
#endif

/**
 * dev_upgrade_sec_img_upgrade: notify the tee of security upgrade completion
 * @upgrade_ctrl: upgrade components information, see @dev_upgrade_core_ctrl struct.
 * schedule_callback: callback function after upgrade finished per component.
 *
 * The kernel driver calls the TEE interfaces in sequence to instruct the TEE to
 * write firmware 0/1/... to the flash memory. The purpose of this design is to
 * update the firmware upgrade progress each time of the component upgraded.
 */
STATIC int dev_upgrade_sec_img_upgrade(dev_upgrade_core_ctrl *upgrade_ctrl)
{
    int ret = 0;
    unsigned int idx = 0;
    struct upgrade_component *component = NULL;

    upgrade_ctrl->finish_size = 0;
    for (idx = 0; idx < upgrade_ctrl->total_component; idx++) {
        component = upgrade_ctrl->component_list[idx];

        if (is_ufs_image(component)) {
            ret = dev_upgrade_storage_ufs(component);
        } else {
            dev_upgrade_info("update by tee, image index=[%u], component type= %u\n", idx, component->component_type);
            ret = sec_img_update(upgrade_ctrl->dev_id, idx);
        }

        if (ret != 0) {
            dev_upgrade_err("update err, ret = %d, index = %u\n", ret, idx);
            return -ERANGE;
        }

        /* set upgrade progress. */
        ret = dev_upgrade_schedule_update(component->dev_id,
            component->component_type, component->file_size, component->file_size);
        if (ret) {
            dev_upgrade_warn("update schedule err, ret = %d, index = %u\n", ret, idx);
        }
#if (defined CFG_SOC_PLATFORM_MINIV2) || (defined CFG_SOC_PLATFORM_MINIV3)
        dev_upgrade_localcfg_proc(upgrade_ctrl, component);
#endif
        upgrade_ctrl->finish_size += (component->file_size * component->total_areas);
    }
    return idx;
}

int dev_upgrade_verify_component(struct upgrade_component *component)
{
#if (defined CFG_SOC_PLATFORM_MDC_V51)
    unsigned int i;
    int ret;
    mdc_package_t *mdc_pkg = NULL;

    /* verify all image */
    for (i = 0; i < component->sub_pkt.sub_file_count; ++i) {
        mdc_pkg = component->sub_pkt.sub_file_info[i];
        if (IS_ERR_OR_NULL(mdc_pkg)) {
            dev_upgrade_err("sub file is null, index=%d\n", i);
            return -EINVAL;
        }

        ret = dev_upgrade_verify_cms_img_process(component->component_type, mdc_pkg->file_name);
        if (ret != 0) {
            dev_upgrade_err("dev_upgrade_verify_cms_img_process failed. file index=%d, ret=%d\n", i, ret);
            return ret;
        }
    }

    /* verify image crl */
    for (i = 0; i < component->sub_pkt.sub_file_count; ++i) {
        mdc_pkg = component->sub_pkt.sub_file_info[i];

        /* check crl info, use itrustee.image */
        if (strcmp(mdc_pkg->name, ITRUSTEE_IMG_FILE) == 0) {
            ret = dev_upgrade_verify_img_crl_process(mdc_pkg->file_name);
            if (ret != 0) {
                dev_upgrade_err("dev_upgrade_verify_img_crl_process fail, ret=%d.\n", ret);
                return ret;
            }

            break;
        }
    }

    return 0;
#else
    dev_upgrade_err("%s: only mdc device upgrade support UFS.\n", __func__);
    return -EINVAL;
#endif
}

#if defined(CFG_SOC_PLATFORM_MDC_LITE_ESL) || defined(CFG_SOC_PLATFORM_MINIV3)
// esl don't support tee
int sec_img_verify(unsigned int dev_id, unsigned int img_num, img_verify_info_s *img_info, unsigned int pss_cfg)
{
    dev_upgrade_err("sec_img_verify error, esl don't support tee");
    return 0;
}
#endif
/**
 * dev_upgrade_verify_sec: verify the upgrade component by tee interface
 * @upgrade_ctrl: upgrade components information, see @dev_upgrade_core_ctrl struct.
 *
 * In firmware upgrade, The kernel driver sends all firmware data to the TEE for
 * security verification at a time. The kernel driver obtains the TEE verification
 * result. If the verification fails, the kernel driver stops the upgrade and
 * returns an upgrade failure message.
 */
STATIC int dev_upgrade_verify_sec(dev_upgrade_core_ctrl *upgrade_ctrl)
{
    int ret = -EINVAL;
    u32 idx = 0;
    int sign;
    img_verify_info_s *img_info = NULL;
    struct upgrade_component *curr_component = NULL;
    int  flash_img_num = 0;

    if ((upgrade_ctrl->total_component == 0) || (upgrade_ctrl->total_component > DSMI_COMPONENT_TYPE_MAX)) {
        dev_upgrade_err("upgrade verification error, upgrade component count is invalid.\n");
        return -EINVAL;
    }

    if (devdrv_config_get_pss_cfg(0, &sign)) {
        dev_upgrade_err("Failed to invoke the devdrv_config_get_pss_cfg. (dev_id=%d)", upgrade_ctrl->dev_id);
        return -EINVAL;
    }

    img_info = (img_verify_info_s *)kzalloc(
        upgrade_ctrl->total_component * sizeof(img_verify_info_s), GFP_KERNEL | __GFP_ACCOUNT);
    if (IS_ERR_OR_NULL(img_info)) {
        dev_upgrade_err("upgrade verification error, no memory is insufficient.\n");
        ret = -ENOMEM;
        goto out;
    }

    for (idx = 0; idx < upgrade_ctrl->total_component; idx++) {
        curr_component = upgrade_ctrl->component_list[idx];
        if (curr_component == NULL) {
            dev_upgrade_err("component index%u is null\n", idx);
            ret = -EINVAL;
            goto out;
        }

        if (is_ufs_image(curr_component)) {
            ret = dev_upgrade_verify_component(curr_component);
            if (ret) {
                dev_upgrade_err("verify component error, ret=%d.\n", ret);
                goto out;
            }
        } else {
            img_info[flash_img_num].image_buf = curr_component->file_content;
            img_info[flash_img_num].img_len = curr_component->file_size;
            img_info[flash_img_num].img_id = curr_component->component_type;
            ++flash_img_num;
        }
    }

    dev_upgrade_warn("before sec_img_verify flash_img_num %d, sign %d\n", flash_img_num, sign);
    if (flash_img_num > 0) {
        ret = sec_img_verify(upgrade_ctrl->dev_id, flash_img_num, img_info, sign);
        if (ret) {
            dev_upgrade_err("upgrade verification error, ret=%d.\n", ret);
        }
    }
    dev_upgrade_warn("after sec_img_verify flash_img_num %d, sign %d\n", flash_img_num, sign);
out:
    if (img_info != NULL) {
        DEV_UPGRADE_KFREE(img_info);
    }

    return ret;
}

#if defined(CFG_SOC_PLATFORM_MDC_LITE_ESL) || defined(CFG_SOC_PLATFORM_MINIV3)
// esl don't support tee
int sec_update_finish(unsigned int dev_id)
{
    dev_upgrade_err("sec_update_finish error, esl don't support tee");
    return 0;
}
#endif
/**
 * dev_upgrade_finish_sec: notify the tee of security upgrade completion
 * @upgrade_ctrl: upgrade components information, see @dev_upgrade_core_ctrl struct.
 *
 * After all firmware is upgraded, the kernel driver notifies the TEE of
 * the upgrade completion, and the TEE writes the upgrade information to the
 * flash memory (image upgrade flag area). Describes how to start and
 * synchronize firmware. In addition, the TEE releases all resources and memory.
 */
STATIC void dev_upgrade_finish_sec(dev_upgrade_core_ctrl *upgrade_ctrl)
{
    int i = 0;
    for (i = 0; i < DEV_UPGRADE_TEE_MAX_RETRY; ++i) {
        if (sec_update_finish(upgrade_ctrl->dev_id)) {
            dev_upgrade_warn("send notification of upgrade completion error. devid is %d\n", upgrade_ctrl->dev_id);
            msleep(DEV_UPGRADE_TEE_RETRY_WAIT);
            continue;
        }
        dev_upgrade_info("notify upgrade completion success. devid us %d\n", upgrade_ctrl->dev_id);
        break;
    }
}

STATIC int dev_upgrade_pre_check_handler(dev_upgrade_core_ctrl *upgrade_ctrl)
{
#if (defined CFG_SOC_PLATFORM_MDC_V51)
    int ret;
    unsigned int idx;
    struct upgrade_component *component = NULL;

    ret = dev_upgrade_mdc_check_update_proc_para(upgrade_ctrl);
    if (ret != 0) {
        dev_upgrade_err("dev_upgrade_mdc_check_update_proc_para fail, ret=%d.\n", ret);
        return ret;
    }

    for (idx = 0; idx < upgrade_ctrl->total_component; idx++) {
        component = upgrade_ctrl->component_list[idx];
        if (is_ufs_image(component)) {
            ret = dev_upgrade_component_para_chk(upgrade_ctrl->dev_id, (DSMI_COMPONENT_TYPE)component->component_type,
                &(component->sub_pkt), DISK_OP_TYPE_UPDATE);
            if (ret != 0) {
                dev_upgrade_err("update para chk fail, ret = %d, index = %u\n", ret, idx);
                return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
            }
        }
    }
#endif

    return OK;
}

#if (defined CFG_SOC_PLATFORM_MDC_V51)

STATIC int dev_upgrade_local_imgs_init_common(dev_upgrade_core_ctrl *upgrade_ctrl,
    struct local_image_verify *local_imgs)
{
    int i, j;
    int img_id;
    mdc_package_t *mdc_pkg = NULL;
    struct upgrade_component *curr_component = NULL;

    for (i = 0; i < upgrade_ctrl->total_component; i++) {
        curr_component = upgrade_ctrl->component_list[i];
        if (curr_component == NULL) {
            dev_upgrade_err("component index%i is null\n", i);
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
        }

        if (!is_ufs_image(curr_component)) {
            continue;
        }

        for (j = 0; j < curr_component->sub_pkt.sub_file_count; j++) {
            mdc_pkg = curr_component->sub_pkt.sub_file_info[j];
            if (IS_ERR_OR_NULL(mdc_pkg)) {
                dev_upgrade_err("sub file is null, index=%d\n", j);
                return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
            }
            dev_upgrade_get_img_id(mdc_pkg->file_name, &img_id);
            if (img_id >= IMG_ID_MAX_NUM) {
                dev_upgrade_err("error img_id: %d\n", img_id);
                return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_INDEX);
            }
            local_imgs[img_id].img_id = img_id;
            local_imgs[img_id].file_name = mdc_pkg->file_name;
            local_imgs[img_id].buff_size = mdc_pkg->storage.len;
        }
    }
    return 0;
}

/* kernel_read不能一次读取2G，使用1G分批读取 */
#define FILE_ONCE_READ (1 * 1024 * 1024 * 1024)
#define MAX_IMG_LEN    ((unsigned long long)8 * 1024 * 1024 * 1024)

STATIC int dev_upgrade_read_img_file(struct local_image_verify *local_img)
{
    ssize_t ret;
    loff_t pos = 0;
    loff_t read_buff_size;
    loff_t read_buff_remain;
    read_buff_remain = local_img->fp->f_inode->i_size;
    local_img->img_len = 0;
    local_img->img_loff = 0;
    while (read_buff_remain > 0) {
        pos = local_img->img_len;
        read_buff_size = read_buff_remain < FILE_ONCE_READ ? read_buff_remain : FILE_ONCE_READ;
        ret = kernel_read(local_img->fp, local_img->img_buff + pos, read_buff_size, &pos);
        if (ret < 0) {
            dev_upgrade_err("read file %s failed, ret = %ld\n", local_img->file_name, ret);
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_READ);
        }
        local_img->img_len += ret;
        read_buff_remain -= read_buff_size;
    }
    return 0;
}

int dev_upgrade_init_img_buff(struct local_image_verify *local_img)
{
    int ret;
    if (local_img->file_name == NULL) {
        return 0;
    }
    if (local_img->img_buff != NULL) {
        dev_upgrade_info("img_buff is already init\n");
        return 0;
    }
    if (local_img->buff_size > MAX_IMG_LEN) {
        dev_upgrade_err("buff_size(%llu) too large\n", local_img->buff_size);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }
    local_img->img_buff = vzalloc(local_img->buff_size);
    if (local_img->img_buff == NULL) {
        dev_upgrade_err("vzalloc fail, img_id = %d\n", local_img->img_id);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_MALLOC);
    }
    local_img->fp = filp_open(local_img->file_name, O_RDONLY | O_LARGEFILE, 0);
    if (IS_ERR_OR_NULL(local_img->fp)) {
        dev_upgrade_err("open file %s failed.", local_img->file_name);
        DEV_UPGRADE_VFREE(local_img->img_buff);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_OPEN);
    }
    if (local_img->fp->f_inode->i_size > local_img->buff_size) {
        dev_upgrade_err("file %s is too large(%lld)\n", local_img->file_name, local_img->fp->f_inode->i_size);
        DEV_UPGRADE_VFREE(local_img->img_buff);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_OPEN);
    }
    ret = dev_upgrade_read_img_file(local_img);
    if (ret != 0) {
        dev_upgrade_err("dev_upgrade_read_img_file failed, ret = %d\n", ret);
        filp_close(local_img->fp, NULL);
        DEV_UPGRADE_VFREE(local_img->img_buff);
        return ret;
    }

    return 0;
}

STATIC int dev_upgrade_local_imgs_init(dev_upgrade_core_ctrl *upgrade_ctrl)
{
    int i;
    int ret;
    struct local_image_verify *local_imgs;

    local_imgs = dev_upgrade_get_local_imgs();
    (void)memset_s(local_imgs, sizeof(struct local_image_verify) * IMG_ID_MAX_NUM, 0,
        sizeof(struct local_image_verify) * IMG_ID_MAX_NUM);

    ret = dev_upgrade_local_imgs_init_common(upgrade_ctrl, local_imgs);
    if (ret != 0) {
        dev_upgrade_err("local imgs init base info failed, ret = %d\n", ret);
        return ret;
    }

    for (i = 0; i < IMG_ID_MAX_NUM; i++) {
        ret = dev_upgrade_init_img_buff(&local_imgs[i]);
        if (ret != 0) {
            dev_upgrade_err("local imgs init image buff failed, ret = %d\n", ret);
            return ret;
        }
    }

    return 0;
}

void dev_upgrade_release_img_buff(struct local_image_verify *local_img)
{
    if (!(IS_ERR_OR_NULL(local_img->fp))) {
        if (filp_close(local_img->fp, NULL) != 0) {
            dev_upgrade_err("close file error\n");
        } else {
            local_img->fp = NULL;
        }
    }
    DEV_UPGRADE_VFREE(local_img->img_buff);
}

STATIC void dev_upgrade_local_imgs_release(void)
{
    int i;
    struct local_image_verify *local_img = NULL;
    local_img = dev_upgrade_get_local_imgs();
    for (i = 0; i < IMG_ID_MAX_NUM; i++) {
        dev_upgrade_release_img_buff(&local_img[i]);
    }
}
#endif

STATIC int dev_upgrade_roll_back_check_all(dev_upgrade_core_ctrl *upgrade_ctrl)
{
#if (defined CFG_FEATURE_ROLL_BACK)
    int ret;
    unsigned int index;
    struct upgrade_component *component = NULL;
    /* before writing flash, validate all files */
    for (index = 0; index < upgrade_ctrl->total_component; index++) {
        component = upgrade_ctrl->component_list[index];
        if (component == NULL) {
            dev_upgrade_err("Component index%u is null\n", index);
            return ERROR;
        }

        ret = dev_upgrade_roll_back_check(component->dev_id, component->file_content, component->file_size);
        if (ret != 0) {
            dev_upgrade_err("Roll back check failed. (ret=%d, dev=%u, type=%u)\n", ret, component->dev_id,
                component->component_type);
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_VERSION_NOT_MATCH);
        }
    }
#endif

   return OK;
}

/**
 * dev_upgrade_sec_upgrade_proc: security upgrade by calling tee API
 * @upgrade_ctrl: upgrade components information, see @dev_upgrade_core_ctrl struct.
 * Return: total count of the component upgraded, or little than 0 if an error occurred.
 *
 * Security firmware upgrade solution. The firmware in the flash memory is
 * verified and written into the flash memory by the Tee, preventing the
 * firmware from being tampered with due to the intrusion of the device kernel driver.
 */
int dev_upgrade_sec_upgrade_proc(dev_upgrade_core_ctrl *upgrade_ctrl)
{
    int ret;
    int finish = 0;

    if (IS_ERR_OR_NULL(upgrade_ctrl)) {
        dev_upgrade_err("upgrade verification error, upgrade ctrl param is NULL.\n");
        return -EINVAL;
    }

    ret = dev_upgrade_pre_check_handler(upgrade_ctrl);
    if (ret != 0) {
        dev_upgrade_err("dev_upgrade_pre_check_handler failed. ret=%d.\n", ret);
        return ret;
    }
#if (defined CFG_SOC_PLATFORM_MDC_V51)
    ret = dev_upgrade_local_imgs_init(upgrade_ctrl);
    if (ret != 0) {
        dev_upgrade_err("dev_upgrade_local_imgs_init failed. ret=%d.\n", ret);
        goto out;
    }
#endif

    /**
     * In DC and MDC scenarios, the TEE interface needs to be invoked
     * to verify the integrity of the upgrade component package.
     */
    ret = dev_upgrade_verify_sec(upgrade_ctrl);
    if (ret) {
        dev_upgrade_err("verify upgrade images failed. ret=%d.\n", ret);
        goto out;
    }
    /* To rollback check after verify */
    ret = dev_upgrade_roll_back_check_all(upgrade_ctrl);
    if (ret != 0) {
        dev_upgrade_err("Roll back check all failed. ret=%d.\n", ret);
        goto out;
    }
    /**
     * Firmware upgrade. The TEE obtains firmware data from sec_img_verify.
     */
    finish = dev_upgrade_sec_img_upgrade(upgrade_ctrl);
    if (finish < 0) {
        dev_upgrade_err("upgrade images in secure failed.\n");
        goto out;
    }
out:
    dev_upgrade_finish_sec(upgrade_ctrl);
#if (defined CFG_SOC_PLATFORM_MDC_V51)
    dev_upgrade_local_imgs_release();
#endif
    if (finish == upgrade_ctrl->total_component) {
        dev_upgrade_info("upgrade %u files succ\n", upgrade_ctrl->total_component);

        /**
         * in MDC, after upgrade, need to write a flag to BIOS.
         */
#if (defined CFG_SOC_PLATFORM_MDC_V51)
        ret = dev_upgrade_mdc_flag_update(upgrade_ctrl);
        if (ret != 0) {
            dev_upgrade_err("dev_upgrade_mdc_flag_update fail, ret=%d.\n", ret);
            return ret;
        }
#endif
        upgrade_ctrl->schedule = UPGRADE_SCHEDULE_PERCENTAGE_100;
        ret = OK;
    } else {
        dev_upgrade_err("upgrade fail, %u/%u files finish!\n", finish, upgrade_ctrl->total_component);
        ret = (ret == dev_errno_make(DEV_MID_UPGRADE, ERRNO_VERSION_NOT_MATCH)) ?
            dev_errno_make(DEV_MID_UPGRADE, ERRNO_VERSION_NOT_MATCH) : ERROR;
    }

    dev_upgrade_core_resource_release(upgrade_ctrl);

    return ret;
}

#if (defined CFG_SOC_PLATFORM_MDC_V51)

STATIC int load_mdc_pkg_config(char *out_buf, size_t *out_size, size_t total_size, const char *filename)
{
    struct file *fp = NULL;
    loff_t offset = 0;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)
    mm_segment_t old_fs;
    int ret;
#endif

    fp = filp_open(filename, O_RDONLY, S_IRUSR);
    if (IS_ERR_OR_NULL(fp)) {
        dev_upgrade_err("unable to open file: %s, errno = %ld.\n", filename, PTR_ERR(fp));
        return -ENODEV;
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
    *out_size = kernel_read(fp, out_buf, total_size, &offset);
#else
    old_fs = get_fs();
    set_fs(KERNEL_DS);
    ret = vfs_read(fp, out_buf, total_size, &offset);
    set_fs(old_fs);
#endif

    filp_close(fp, NULL);
    fp = NULL;
    return 0;
}

#define STORAGE_NOTATION  1024
#define DECIMAL_SCALE     10
static loff_t str_to_loff_t(const char *str)
{
    unsigned int i;
    loff_t result = 0;
    unsigned int len = strlen(str);

    for (i = 0; i < len; i++) {
        if (isdigit(str[i])) {
            result = result * DECIMAL_SCALE + (str[i] - '0');
        } else if (str[i] == 'G' || str[i] == 'g') {
            result = result * STORAGE_NOTATION * STORAGE_NOTATION * STORAGE_NOTATION;
            break;
        } else if (str[i] == 'M' || str[i] == 'm') {
            result = result * STORAGE_NOTATION * STORAGE_NOTATION;
            break;
        } else if ((str[i] == 'K' || str[i] == 'k')) {
            result = result * STORAGE_NOTATION;
            break;
        } else {
            break;
        }
    }
    return result;
}

int get_component_attr(const char *component_str, const char *tag, char *val, int len)
{
    const char *component_store_start = NULL;
    const char *component_store_end = NULL;

    component_store_start = strstr(component_str, tag);
    if (component_store_start == NULL) {
        dev_upgrade_err("get component %s from mdc config failed.\n", tag);
        return -EFAULT;
    }

    component_store_start += strlen(tag);
    component_store_end = strstr(component_store_start, "\"");
    if (component_store_end == NULL) {
        dev_upgrade_err("get component tag %s from mdc config failed.\n", tag);
        return -EFAULT;
    }

    (void)memset_s(val, len, 0, len);
    if (memcpy_s(val, len, component_store_start, component_store_end - component_store_start) != EOK) {
        dev_upgrade_err("memcpy_s error:%s:%d\n", __FILE__, __LINE__);
        return -EFAULT;
    }

    return 0;
}

STATIC int parse_image_storage_info(const char *src_buf, const char *component_name,
    uint32_t cur_flag, part_xml_info_struct *xmlInfo)
{
    int ret;
    char storage_offset[STORAGE_OFFSET_MAX_LEN];
    char storage_start_k[STORAGE_OFFSET_MAX_LEN];
    char storage_lu[STORAGE_OFFSET_MAX_LEN];
    char storage_lu_k[STORAGE_OFFSET_MAX_LEN];
    char storage_len[STORAGE_OFFSET_MAX_LEN];
    char storage_len_k[STORAGE_OFFSET_MAX_LEN];
    const char *component_type_ptr = NULL;
    component_type_ptr = strstr(src_buf, component_name);
    if (component_type_ptr == NULL) {
        if (strcmp(component_name, "Recovery") == 0) {
            dev_upgrade_warn("get component %s from mdc config failed.\n", component_name);
        } else {
            dev_upgrade_err("get component %s from mdc config failed.\n", component_name);
        }
        return -ENOSR;
    }

    (void)memset_s(storage_start_k, STORAGE_OFFSET_MAX_LEN, 0, STORAGE_OFFSET_MAX_LEN);
    (void)memset_s(storage_lu_k, STORAGE_OFFSET_MAX_LEN, 0, STORAGE_OFFSET_MAX_LEN);
    (void)memset_s(storage_len_k, STORAGE_OFFSET_MAX_LEN, 0, STORAGE_OFFSET_MAX_LEN);
    if (cur_flag == UFS_MASTER_AREA) {
        if (strcpy_s(storage_start_k, STORAGE_OFFSET_MAX_LEN, " start=\"") != EOK) {
            dev_upgrade_err("memcpy_s error:start\n");
            return -EFAULT;
        }

        if (strcpy_s(storage_lu_k, STORAGE_OFFSET_MAX_LEN, " LUN=\"") != EOK) {
            dev_upgrade_err("memcpy_s error:LUN\n");
            return -EFAULT;
        }
    } else {
        if (strcpy_s(storage_start_k, STORAGE_OFFSET_MAX_LEN, " backup_start=\"") != EOK) {
            dev_upgrade_err("memcpy_s error:backup_start\n");
            return -EFAULT;
        }

        if (strcpy_s(storage_lu_k, STORAGE_OFFSET_MAX_LEN, " backupLU=\"") != EOK) {
            dev_upgrade_err("memcpy_s error:backupLU\n");
            return -EFAULT;
        }
    }
    if (strcpy_s(storage_len_k, STORAGE_OFFSET_MAX_LEN, " len=\"") != EOK) {
            dev_upgrade_err("memcpy_s error:len\n");
            return -EFAULT;
    }

    ret = get_component_attr(component_type_ptr, storage_start_k, storage_offset, STORAGE_OFFSET_MAX_LEN);
    if (ret) {
        dev_upgrade_err("get component %s storage start failed, ret = %d.\n", component_name, ret);
        return -EFAULT;
    }
    xmlInfo->offset = str_to_loff_t(storage_offset);

    ret = get_component_attr(component_type_ptr, storage_lu_k, storage_lu, STORAGE_OFFSET_MAX_LEN);
    if (ret) {
        dev_upgrade_err("get component %s LU failed, ret = %d.\n", component_name, ret);
        return -EFAULT;
    }
    ret = kstrtou32(storage_lu, DECIMAL_SCALE, &(xmlInfo->lu));
    if (ret) {
        dev_upgrade_err("get component %s LU failed, lu=%s ret = %d.\n", component_name, storage_lu, ret);
    }

    ret = get_component_attr(component_type_ptr, storage_len_k, storage_len, STORAGE_OFFSET_MAX_LEN);
    if (ret) {
        dev_upgrade_err("get component %s storage len failed, ret = %d.\n", component_name, ret);
        return -EFAULT;
    }
    xmlInfo->len = str_to_loff_t(storage_len);

    return ret;
}

static bool dev_upgrade_file_exist(const char *file_path)
{
    struct file *fp = NULL;

    fp = filp_open(file_path, O_RDONLY, 0);
    if (IS_ERR_OR_NULL((void const *)fp)) {
        dev_upgrade_warn("file %s not exist.\n", file_path);
        return false;
    }

    filp_close(fp, NULL);
    fp = NULL;
    return true;
}

int get_nvme_ssd_capacity(u64 *capacity)
{
    long long capacity_ret = dev_upgrade_get_lun_size(DISK_OP_TYPE_SYNC, 0);
    if (capacity_ret < 0) {
        dev_upgrade_err("get ssd capacity failed. ret=%lld\n", capacity_ret);
        return -ENODATA;
    }

    *capacity = (u64)(capacity_ret / UPGRADE_SYS_FILE_BLOCK_SIZE);
    return 0;
}

#if defined(CFG_SOC_PLATFORM_MDC_LITE_ESL) || defined(CFG_SOC_PLATFORM_MINIV3)
// esl don't support tee
int get_ufs_capacity(u64 *capacity)
{
    dev_upgrade_err("get_ufs_capacity error, esl don't support tee");
    return 0;
}
#endif
int get_install_config_path(unsigned int mieda_type, char *config_path, unsigned int len)
{
    unsigned int device_id = 0;
    u64 storage_capacity = 0;
    u64 capacity_in_gb = 0;
    u32 board_id = 0;
    u32 slot_id = 0;
    int ret;

    ret = devdrv_get_device_ids(device_id, &board_id, &slot_id);
    if (ret) {
        dev_upgrade_err("device manager info failed. ret=%d\n", ret);
        return -ENOSR;
    }

    if (mieda_type == UPGRADE_SSD_MEDIA) {
        ret = get_nvme_ssd_capacity(&storage_capacity);
    } else {
#ifdef CFG_SOC_PLATFORM_MDC_V11
        ret = 0;
#else
        ret = get_ufs_capacity(&storage_capacity);
#endif
    }

    if (ret) {
        dev_upgrade_err("devid %d dsmi_get_slot_id call error ret = %d!\n", device_id, ret);
        return -EINVAL;
    }

    /* The returned value is in the unit of 512 bytes, result size is converted to GB */
    capacity_in_gb = (storage_capacity >> GB_SIZE_OFFSET);
    if (capacity_in_gb > (STORAGE_CAPACITY_128GB + 1)) {
        capacity_in_gb = STORAGE_CAPACITY_256GB;
    } else if (capacity_in_gb > (STORAGE_CAPACITY_64GB + 1)) {
        capacity_in_gb = STORAGE_CAPACITY_128GB;
    } else {
        capacity_in_gb = STORAGE_CAPACITY_64GB;
    }

    if (mieda_type == UPGRADE_UFS_MEDIA) {
        ret = sprintf_s(config_path, len, "/etc/mdc_config_cap%llu.xml", capacity_in_gb);
    } else {
        ret = sprintf_s(config_path, len, "/etc/mdc_ssd_config_cap%llu.xml", capacity_in_gb);
    }
    if (ret < 0) {
        dev_upgrade_err("memory operation error.ret = %d!\n", ret);
        return -ENOMEM;
    }

    if (dev_upgrade_file_exist(config_path)) {
        dev_upgrade_info("mdc config file is %s\n", config_path);
        return 0;
    }

    if (mieda_type == UPGRADE_UFS_MEDIA) {
        ret = strncpy_s(config_path, len, MDC_PKG_UFS_CONFIG, sizeof(MDC_PKG_UFS_CONFIG));
    } else {
        ret = strncpy_s(config_path, len, MDC_PKG_SSD_CONFIG, sizeof(MDC_PKG_SSD_CONFIG));
    }
#ifdef CFG_SOC_PLATFORM_MDC_V11
    ret = strncpy_s(config_path, len, MDC_PKG_EMMC_CONFIG, sizeof(MDC_PKG_EMMC_CONFIG));
#endif
    if (ret != 0) {
        dev_upgrade_err("memcpy failed\n");
        return -ENOMEM;
    }
    return 0;
}

/**
 * get_img_storage_devname: get image storage device name
 * @img_idx: image index, only for images stored in UFS.
 * @cur_flag: the master/slave flag of running system.
 * Return: the storage name which storing the image, and NULL for not found.
 *
 * Different images are stored in different logical partitions.
 * This function obtains the name of the logical partition where
 * the image is stored based on the image sequence number.
 */
int get_img_storage_info(uint32_t img_idx, uint32_t cur_flag, part_xml_info_struct *xmlInfo)
{
    int ret;
    char *src_buf = NULL;
    size_t out_size = 0;
    unsigned int stat = UPGRADE_UFS_MEDIA;
    char xml_file_name[MDC_CONFIG_PATH_SIZE] = { 0 };

#define MAX_COMPONENT_COUNT 8

    const char *component_names[] = { "RAWData", "RO_SysDrv", "RO_ADSApp",
        "RO_ComIsolator", "RO_Cluster", "RO_Customized", "SysBaseConfig", "Recovery" };
    if (img_idx >= MAX_COMPONENT_COUNT) {
        dev_upgrade_err("image index is invalid. index= %u\n", img_idx);
        return -EINVAL;
    }

    src_buf = kzalloc(MDC_PKG_CONFIG_SIZE + 1, GFP_KERNEL | __GFP_ACCOUNT);
    if (src_buf == NULL) {
        dev_upgrade_err("kzalloc src_buf failed, size = %d\n", MDC_PKG_CONFIG_SIZE);
        return -ENOMEM;
    }

    ret = dev_upgrade_get_media(QUERY_FROM_BOOTSTRAP, &stat);
    if (ret != 0) {
        dev_upgrade_err("dev_upgrade_get_media failed, ret = %d.\n", ret);
        goto _out;
    }

    ret = get_install_config_path(stat, xml_file_name, MDC_CONFIG_PATH_SIZE);
    if (ret != 0) {
        dev_upgrade_err("get mdc config file failed, ret = %d.\n", ret);
        goto _out;
    }

    /* read ini file content into src_buf */
    ret = load_mdc_pkg_config(src_buf, &out_size, MDC_PKG_CONFIG_SIZE, (const char *)xml_file_name);
    if (ret != 0) {
        dev_upgrade_err("load mdc config failed, ret = %d.\n", ret);
        goto _out;
    }

    ret = parse_image_storage_info(src_buf, component_names[img_idx], cur_flag, xmlInfo);
    if (ret) {
        if (strcmp(component_names[img_idx], "Recovery") == 0) {
            dev_upgrade_warn("parse storage offset and LU failed. ret = %d.\n", ret);
        } else {
            dev_upgrade_err("parse storage offset and LU failed. ret = %d.\n", ret);
        }
    }

_out:
    DEV_UPGRADE_KFREE(src_buf);
    return ret;
}

/**
 * get_rootfs_data_size: get root filesystem data size from storage
 * @img_id: image id to be queried.
 * @lun: physical partition ID obtained from the XML file.
 * @img_offset: image offset in the XML partition.
 * @pos: secondary header offset.
 * Return: On success, zero is returned.  On error, an error no is returned.
 *
 * Read the field from rootfs which contains the block count(0x404) and
 * the block size(0x418), to calculate size of rootfs data size.
 */
STATIC loff_t get_rootfs_data_size(u32 img_id, const u8 lun, const u64 img_offset)
{
    int ret;
    u8 rootfs_buff[ROOTFS_BLOCK_READ_SIZE];
    u32 block_count, block_size_level;

    ret = dev_upgrade_disk_read(QUERY_FROM_CMDLINE, lun, img_offset, rootfs_buff, ROOTFS_BLOCK_READ_SIZE);
    if (ret != 0) {
        return -ENXIO;
    }

    block_count = *(u32 *)(uintptr_t)(rootfs_buff + ROOTFS_BLOCK_COUNT_OFFSET);
    block_size_level = *(u32 *)(uintptr_t)(rootfs_buff + ROOTFS_BLOCK_SIZE_OFFSET);

    if (block_size_level == BLOCK_SIZE_0) {
        return (loff_t)block_count * (loff_t)BLOCK_SIZE_1K;
    } else if (block_size_level == BLOCK_SIZE_1) {
        return (loff_t)block_count * (loff_t)BLOCK_SIZE_2K;
    } else if (block_size_level == BLOCK_SIZE_2) {
        return (loff_t)block_count * (loff_t)BLOCK_SIZE_4K;
    } else {
        return -ENODATA;
    }
}

/**
 * get_rootfs_secondary_header_offset: get root filesystem data size from storage from storage
 * @img_id: image id to be queried.
 * @lun: physical partition ID obtained from the XML file.
 * @img_offset: image offset in the XML partition.
 * @pos: secondary header offset.
 * Return: On success, zero is returned.  On error, an error no is returned.
 *
 * Get the secondary header offset by judging the magic word of the secondary header
 * if magic num is 0x3a3aaa33, secondary header offset is rootfs_data_size(variable length pack mode)
 * else if secondary header offset is rootfs_data_size + HASH_TREE_SIZE(fixed length pack mode).
 */
STATIC int get_rootfs_secondary_header_offset(u32 img_id, const u8 lun, const u64 img_offset, loff_t *pos)
{
    int ret;
    img_ver_header secondary_header;
    loff_t rootfs_data_size;

    if (img_id != DSMI_COMPONENT_TYPE_SYSDRV) {
        dev_upgrade_err("component type is error.(component_type=%d; expect=%d)\n", img_id, DSMI_COMPONENT_TYPE_SYSDRV);
        return -EINVAL;
    }

    rootfs_data_size = get_rootfs_data_size(img_id, lun, img_offset);
    if (rootfs_data_size < 0) {
        *pos = INVALID_VALUE;
        dev_upgrade_warn("rootfs data size is invalid. (ret=%lld)\n", rootfs_data_size);
        return 0;
    }

    ret = dev_upgrade_disk_read(
        QUERY_FROM_CMDLINE, lun, img_offset + rootfs_data_size, (char *)&secondary_header, sizeof(img_ver_header));
    if (ret != 0) {
        dev_upgrade_err("read rootfs secondary_header failed. (ret=%d; lun=%u; offset=%llu)\n", ret, lun, img_offset);
        return -ENXIO;
    }
#ifdef CFG_FEATURE_SIGN_VERSION_1
    *pos = rootfs_data_size + HASH_TREE_SIZE;
#else
    if (secondary_header.magic_num == IMG_VER_MAGIC) {
        *pos = rootfs_data_size;
    } else {
        *pos = rootfs_data_size + HASH_TREE_SIZE;
    }
#endif
    return OK;
}

/**
 * read_version_from_image: read image version data from storage
 * @img_id: image id to be queried
 * @cur_flag: current boot partition, 0 is master, 1 is backup.
 * @version_buff: version buff read from image.
 * @len: version_buff's max size
 * Return: On success, zero is returned.  On error, an error no is returned.
 *
 * Read the string that contains the version information based on
 * the security header data structure of the image file and save
 * the character string to version_buff.
 */
int read_version_from_image(uint32_t img_id, unsigned int cur_flag, u8 *version_buff, uint32_t len)
{
    loff_t pos = ESBC_IMG_HEADER_OFFSET;
    int ret;
    part_xml_info_struct xmlInfo = {0};

    ret = get_img_storage_info(img_id - DSMI_COMPONENT_TYPE_RAWDATA, cur_flag, &xmlInfo);
    if (ret) {
        if (img_id == DSMI_COMPONENT_TYPE_RECOVERY) {
            dev_upgrade_warn("The recovery partition is not found. The recovery version cannot be queried.\n");
            return 0;
        }
        dev_upgrade_err("cannot find component storage device name. component type=%u\n", img_id);
        return -ENXIO;
    }

    if (img_id == DSMI_COMPONENT_TYPE_SYSDRV) {
        ret = get_rootfs_secondary_header_offset(img_id, xmlInfo.lu, xmlInfo.offset, &pos);
        if (pos == INVALID_VALUE) {
            (void)memset_s(version_buff, len, INVALID_VERSION, len);
            dev_upgrade_warn("rootfs img is invalid, version is unreliable.(cur_flag=%u)\n", cur_flag);
            return 0;
        } else if (ret < 0) {
            dev_upgrade_err("get rootfs secondary header offset failed. (ret=%d)\n", ret);
            return -ENXIO;
        }
    }

    ret = dev_upgrade_disk_read(QUERY_FROM_CMDLINE, xmlInfo.lu, xmlInfo.offset + pos, version_buff, len);
    dev_upgrade_info("imgid=%d, disk read lu=%u, offset=%lld, pos=%lld, len=%u\n",
                     img_id, xmlInfo.lu, xmlInfo.offset, pos, len);
    if (ret) {
        dev_upgrade_err("read version data from lu %u failed. ret=%d\n", xmlInfo.lu, ret);
        return -ENXIO;
    }

    return OK;
}

/**
 * dev_upgrade_image_format_version: format the image version.
 * @version_buf: version data read from storage device
 *               by function read_version_from_image
 * @version: version buff formatted.
 * @len: version's max size
 */
void dev_upgrade_image_format_version(const u8 *version_buf, unsigned char *version, int len)
{
    int ret;

    dev_upgrade_info("version_len=%d\n", len);
    ret = memcpy_s(version, len, version_buf + ESBC_IMG_VERSION_START, ESBC_IMG_VERSION_LEN);
    if (ret != 0) {
        dev_upgrade_err("copy version data error. ret=%d\n", ret);
    }
}

/**
 * read_register_program_version_from_image: read image version data from storage
 * @img_id: image id to be queried
 * @cur_flag: current boot partition, 0 is master, 1 is backup.
 * @version_buff: version buff read from image.
 * @len: version_buff's max size
 * Return: On success, zero is returned.  On error, an error no is returned.
 *
 * Read the string that contains the version information based on
 * the security header data structure of the image file and save
 * the character string to version_buff.
 */
int read_register_program_version_from_image(unsigned int img_id, unsigned int cur_flag, u8 *version_buff, uint32_t len)
{
    loff_t pos = ESBC_IMG_HEADER_OFFSET;
    int ret;
    unsigned long long lun_offset;
    uint32_t lun_id = 0;
    PROG_ALL_PKG_INFO *all_pkg_info = NULL;
    PROG_PKG_INFO *pkg_info = NULL;
    part_xml_info_struct xmlInfo = {0};

    all_pkg_info = dev_upgrade_get_register_package_info();
    pkg_info = (cur_flag == UFS_MASTER_AREA) ? &all_pkg_info->pkg_info_a[img_id] : &all_pkg_info->pkg_info_b[img_id];

    if (pkg_info->flag != DRV_ENABLE) {
        dev_upgrade_warn("flag is invalid. img_id=%u, cur=%u, pag_info_flag=%u.\n", img_id, cur_flag, pkg_info->flag);
        return OK;
    }

    lun_offset = pkg_info->lun_offset;
    lun_id = pkg_info->lun_id;

    if (img_id == DSMI_COMPONENT_TYPE_SYSDRV) {
        ret = get_img_storage_info(img_id - DSMI_COMPONENT_TYPE_RAWDATA, cur_flag, &xmlInfo);
        if (ret) {
            dev_upgrade_err("cannot find component storage device name. component type=%u\n", img_id);
            return -ENXIO;
        }
        ret = get_rootfs_secondary_header_offset(img_id, xmlInfo.lu, xmlInfo.offset, &pos);
        if (pos == INVALID_VALUE) {
            (void)memset_s(version_buff, len, INVALID_VERSION, len);
            dev_upgrade_warn("rootfs img is invalid, version is unreliable.(cur_flag=%u)\n", cur_flag);
            return 0;
        } else if (ret < 0) {
            dev_upgrade_err("get rootfs secondary header offset failed. (ret=%d)\n", ret);
            return -ENXIO;
        }
    }

    ret = dev_upgrade_disk_read(QUERY_FROM_CMDLINE, lun_id, lun_offset + pos, version_buff, len);
    if (ret) {
        dev_upgrade_err("read version data from lun_id %u failed. ret=%d\n", lun_id, ret);
        return -ENXIO;
    }

    return OK;
}

/**
 * dev_upgrade_get_single_image_version: get component's version stored in UFS
 * @dev_id: device id
 * @component_type: component type
 * @version_str:  the version which read from UFS storage.
 * @len: version's max size
 * Return: On success, zero is returned.  On error, an error no is returned.
 *
 * After a secure upgrade, system images are stored in the UFS logical
 * partition. This function can be used to query the version number of
 * the running system image from the UFS storage and convert the version
 * number into a readable format.
 */
int dev_upgrade_get_single_image_version(unsigned int dev_id, unsigned int cur_flag, unsigned int component_type,
    unsigned char *version_str, unsigned int max_len)
{
    int  ret;
    u8 *version_buff = NULL;
    PROG_ALL_PKG_INFO *all_pkg_info = NULL;
    version_buff = (u8 *)kzalloc(ESBC_IMG_HEADER_SIZE, GFP_KERNEL | __GFP_ACCOUNT);
    if (version_buff == NULL) {
        dev_upgrade_err("alloc memory failed.\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_INIT);
    }

    dev_upgrade_info("read image version from device %u. component type=%u\n", dev_id, component_type);

    all_pkg_info = dev_upgrade_get_register_package_info();
    if (all_pkg_info != NULL && all_pkg_info->flag == DRV_ENABLE) {
        ret = read_register_program_version_from_image(component_type, cur_flag, version_buff, ESBC_IMG_HEADER_SIZE);
    } else {
        ret = read_version_from_image(component_type, cur_flag, version_buff, ESBC_IMG_HEADER_SIZE);
    }

    if (ret != OK) {
        dev_upgrade_err("component(%u) read image version fail, ret: %d.\n", component_type, ret);
        DEV_UPGRADE_KFREE(version_buff);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_CONTENT);
    }

    (void)memset_s(version_str, max_len, 0, max_len);
    dev_upgrade_image_format_version(version_buff, version_str, max_len);

    DEV_UPGRADE_KFREE(version_buff);
    return OK;
}
#endif
