/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/vmalloc.h>
#include <linux/securec.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 20, 0) && !defined(AOS_LLVM_BUILD))
#include <linux/namei.h>
#endif

#ifdef AOS_LLVM_BUILD
#include <linux/firmware.h>
#endif

#include "tsdrv_log.h"
#include "tsdrv_firmware_load.h"
#include "tsdrv_file_load.h"
#include "tsdrv_firmware_reader.h"
#ifndef AOS_LLVM_BUILD
size_t get_file_size(const char *path)
{
    size_t filesize = 0xffff;
    int error;
    struct kstat statbuff;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 20, 0)
    struct path kernel_path;

    statbuff.size = 0;
    error = kern_path(path, LOOKUP_FOLLOW, &kernel_path);
    if (error >= 0) {
        error = vfs_getattr(&kernel_path, &statbuff, STATX_BASIC_STATS, AT_NO_AUTOMOUNT);
        if (error >= 0) {
            filesize = (size_t)statbuff.size;
        }
    }
#else
    mm_segment_t old_fs;

    old_fs = get_fs();
    /*lint -emacro(501,KERNEL_DS)*/
    set_fs((mm_segment_t)KERNEL_DS); /*lint !e501*/

    statbuff.size = 0;
    error = vfs_stat(path, &statbuff);
    if (error < 0) {
#ifndef TSDRV_UT
        set_fs(old_fs);
        return filesize;
#endif
    } else {
        filesize = (size_t)statbuff.size;
    }
    set_fs(old_fs);
#endif

    return filesize;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 16, 0)
bool copy_firmware(struct file *fp, size_t fsize, loff_t pos, void *firmware_dst_addr)
{
    ssize_t ret;

    ret = kernel_read(fp, firmware_dst_addr, fsize, &pos);
    if (ret < 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("error in %s ,ret:%ld\n ", __func__, ret);
        return false;
#endif
    }
    return true;
}
#else

bool copy_firmware(struct file *fp, size_t fsize, loff_t pos, void *firmware_dst_addr)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
    ssize_t ret;
    ret = vfs_read(fp, firmware_dst_addr, fsize, &pos);
#else
    mm_segment_t old_fs;
    ssize_t ret;

    old_fs = get_fs();
    /*lint -emacro(501,KERNEL_DS)*/
    set_fs((mm_segment_t)KERNEL_DS); /*lint !e501*/
    ret = vfs_read(fp, firmware_dst_addr, fsize, &pos);

    set_fs(old_fs);
#endif
    if (ret < 0) {
        TSDRV_PRINT_ERR("error in %s ,ret:%ld\n ", __func__, ret);
        return false;
    }
    return true;
}
#endif

STATIC int tsdrv_file_read(const char *path, void **fw_src_addr, size_t file_size)
{
    int ret;
    loff_t pos = 0;
    void *src_addr = NULL;
    struct file *fp = NULL;

    src_addr = vzalloc(file_size);
    if (src_addr == NULL) {
        TSDRV_PRINT_ERR("Alloc memory failed. (size=%lu)\n", file_size);
        return -ENOMEM;
    }

    fp = filp_open(path, O_RDONLY, 0);
    if (IS_ERR(fp)) {
        TSDRV_PRINT_ERR("Open firmware file. (path=%s)\n", path);
        ret = -EINVAL;
        goto open_fail;
    }

    if (copy_firmware(fp, file_size, pos, src_addr) == false) {
        TSDRV_PRINT_ERR("Copy firmware file. (path=%s)\n", path);
        ret = -EINVAL;
        goto copy_fail;
    }

    *fw_src_addr = src_addr;
    (void)filp_close(fp, NULL);
    return 0;

copy_fail:
    (void)filp_close(fp, NULL);
open_fail:
    vfree(src_addr);
    return ret;
}

int tsdrv_firmware_read(const char *path, void **fw_src_addr, size_t *file_size)
{
    size_t fsize;
    int ret;

    fsize = get_file_size(path);
    if (fsize <= DEVDRV_SEC_HEAD_SIZE) {
        TSDRV_PRINT_ERR("tsfw size is invalid, insufficient head size. (file_size=%lu; head_size=%d)\n",
            fsize, DEVDRV_SEC_HEAD_SIZE);
        return -EINVAL;
    }

    if (fsize > DEVDRV_TS_MEMORY_SIZE) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("tsfw size is invalid, larger than max size. (file_size=%lu; max_size=%d)\n",
            fsize, DEVDRV_TS_MEMORY_SIZE);
        return -EINVAL;
#endif
    }

    ret = tsdrv_file_read(path, fw_src_addr, fsize);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Failed to tsdrv_file_read.\n");
        return ret;
#endif
    }

    *file_size = (size_t)fsize;
    return 0;
}
#else
int tsdrv_get_firmware(const char *path, void *dst_addr, size_t dest_max)
{
    const struct firmware *fw = NULL;
    int ret;

    ret = request_firmware(&fw, path, NULL);
    if (ret || (fw == NULL)) {
        TSDRV_PRINT_ERR("Fail to load firmware. (path=%s; ret=%d)\n", path, ret);
        return ret;
    }

    if (fw->size > dest_max) {
        devdrv_drv_err("File size is invalid. (path=%s; ret=%d)\n", path, fw->size);
        release_firmware(fw);
        return -ENOMEM;
    }

    ret = memcpy_s(dst_addr, dest_max, fw->data, fw->size);
    if (ret != EOK) {
        TSDRV_PRINT_ERR("memcpy_s failed, ret = %d\n", ret);
        release_firmware(fw);
        return -EINVAL;
    }
    release_firmware(fw);
    return 0;
}

int tsdrv_firmware_read(const char *path, void **fw_src_addr, size_t *file_size)
{
    const struct firmware *fw = NULL;
    int ret;
    void *src_addr = NULL;

    ret = request_firmware(&fw, path, NULL);
    if (ret || (fw == NULL)) {
        TSDRV_PRINT_ERR("Fail to load firmware. (path=%s; ret=%d)\n", path, ret);
        return ret;
    }

    if (fw->size <= DEVDRV_SEC_HEAD_SIZE || fw->size > DEVDRV_TS_MEMORY_SIZE) {
        TSDRV_PRINT_ERR("File size is invalid. (path=%s; size=%lu)\n", path, fw->size);
        release_firmware(fw);
        return -EINVAL;
    }

    src_addr = vzalloc(fw->size);
    if (src_addr == NULL) {
        TSDRV_PRINT_ERR("Alloc memory failed. (size=%lu)\n", fw->size);
        release_firmware(fw);
        return -ENOMEM;
    }

    ret = memcpy_s(src_addr, fw->size, fw->data, fw->size);
    if (ret != EOK) {
        TSDRV_PRINT_ERR("memcpy_s failed, ret = %d\n", ret);
        vfree(src_addr);
        src_addr = NULL;
        release_firmware(fw);
        return -EINVAL;
    }
    *file_size = (size_t)fw->size;
    *fw_src_addr = src_addr;
    release_firmware(fw);
    return 0;
}
#endif

int tsdrv_firmware_write(struct devdrv_info *dev_info)
{
    void *src_addr = (void *)(uintptr_t)(dev_info->fw_src_addr +
        dev_info->sec_head_size + dev_info->pos);
    void *dst_addr = (void *)((uintptr_t)dev_info->fw_info.ts_boot_addr_virt);
    int ret;

    ret = memcpy_s(dst_addr, DEVDRV_TS_MEMORY_SIZE, src_addr, dev_info->fw_len);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Copy firmware to dst addr failed. (ret=%d)\n", ret);
        goto end;
#endif
    }

#ifndef CFG_SOC_PLATFORM_MINIV2
    isb();
    tsdrv_flush_cache(dev_info, (u64)(uintptr_t)dst_addr, DEVDRV_TS_MEMORY_SIZE);
#endif
end:
    devdrv_drv_info("tsdrv_firmware_write ok. (devid=%u)\n", dev_info->dev_id);
    vfree(dev_info->fw_src_addr);
    dev_info->fw_src_addr = NULL;
    return ret;
}

#ifdef CFG_SOC_PLATFORM_CLOUD_V2
int tsdrv_ffts_read(const char *path, void **ffts_src_addr, size_t *file_size)
{
    size_t fsize;
    int ret;

    fsize = get_file_size(path);
    if (fsize <= DEVDRV_SEC_HEAD_SIZE) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("File size is invalid, insufficient head size. (file_size=%lu; head_size=%d)\n",
            fsize, DEVDRV_SEC_HEAD_SIZE);
        return -EINVAL;
#endif
    }

    if (fsize > DIE0_FFTS_RESERVED_ADDR) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("ffts size is invalid, larger than max size. (file_size=%lu; max_size=%d)\n",
            fsize, DIE0_FFTS_RESERVED_ADDR);
        return -EINVAL;
#endif
    }

    ret = tsdrv_file_read(path, ffts_src_addr, fsize);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to tsdrv_file_read.\n");
        return ret;
    }

    *file_size = (size_t)fsize;
    return 0;
}

int tsdrv_ffts_write(struct devdrv_info *dev_info, void *src, u32 src_size, void *dst, u32 dst_size)
{
#ifndef TSDRV_UT
    int ret;

    ret = memcpy_s(dst, dst_size, src + DEVDRV_SEC_HEAD_SIZE, src_size - DEVDRV_SEC_HEAD_SIZE);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Copy ffts file to dst addr failed. (ret=%d)\n", ret);
    }

    isb();
    tsdrv_flush_cache(dev_info, (u64)(uintptr_t)dst, dst_size);
    return ret;
#else
    return 0;
#endif
}
#endif
