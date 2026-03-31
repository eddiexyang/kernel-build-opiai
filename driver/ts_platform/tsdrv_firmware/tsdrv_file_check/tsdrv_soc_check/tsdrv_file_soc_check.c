/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
 * Create: 2022-08-13
 */
#ifndef AOS_LLVM_BUILD
#include <linux/crypto.h>
#include <linux/scatterlist.h>
#include <linux/securectype.h>
#include <linux/sched/task.h>
#else
#include <linux/module.h>
#endif
#include <linux/fs.h>
#include <linux/securec.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#include <linux/vmalloc.h>
#include <linux/kallsyms.h>

#include "tsdrv_firmware_reader.h"
#include "tsdrv_common.h"
#include "tsdrv_log.h"
#include "drv_pkicms.h"
#include "tsdrv_file_load.h"

#define FILE_CHECK_TIMEOUT 1000

enum devdrv_tsfw_bin_check_status {
    TSFW_BIN_CHECK_SUCCESS = 1,
    TSFW_BIN_CHECK_FAILED = 2,
    TSFW_BIN_CHECK_MAX_STATUS
};

struct tsdrv_file_check_para {
    u32 dev_id;
    int file_type;
    u8 *file_src_addr;
    u32 file_size;
};

static int file_check_flag[MAX_CHIP_NUM][SOC_VERIFY_MAX];

/**
 * for BS9SX1A and ASCEND610 binary normalization
 */
typedef s32 (soc_verify_func)(u32 dev_id, s32 img_id, u8 *image_head_base, u32 size);

/**
 * find soc_verify in kernel symbols
 *
 * The drv_pkcms.ko file is not installed on BS9SX1A, and the kernel
 * does not have the soc_verify function. The binary consistency
 * between ASCEND610 and BS9SX1A is modified. Therefore, the function cannot
 * be directly invoked and the function is dynamically searched.
 */
soc_verify_func *devdrv_get_soc_verify_symbol(void)
{
    soc_verify_func *func = NULL;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0)) && (!defined(AOS_LLVM_BUILD))
    func = __symbol_get("soc_verify");
#else
    func = (soc_verify_func *)(uintptr_t)kallsyms_lookup_name("soc_verify");
#endif
    return func;
}

void devdrv_put_soc_verify_symbol(void)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0)) && (!defined(AOS_LLVM_BUILD))
    __symbol_put("soc_verify");
#endif
}

STATIC int devdrv_tsfw_soc_verify_thread(void *data)
{
    struct tsdrv_file_check_para *para = (struct tsdrv_file_check_para *)data;
    u32 dev_id = para->dev_id;
    int file_type = para->file_type;
    int ret;

#ifdef CFG_SOC_PLATFORM_MINIV3
        ret = 0;
#else
    soc_verify_func *tsfw_soc_verify = NULL;
    tsfw_soc_verify = devdrv_get_soc_verify_symbol();
    if (tsfw_soc_verify == NULL) {
        TSDRV_PRINT_ERR("soc_verify not find\n");
#ifndef TSDRV_UT
        file_check_flag[dev_id][file_type] = (int)TSFW_BIN_CHECK_FAILED;
#endif
        return -EFAULT;
    }

    ret = tsfw_soc_verify(dev_id, para->file_type, para->file_src_addr, para->file_size);
    if (ret != 0) {
        devdrv_put_soc_verify_symbol();
        TSDRV_PRINT_ERR("soc_verify failed, ret(%d).\n", ret);
        file_check_flag[dev_id][file_type] = (int)TSFW_BIN_CHECK_FAILED;
        return ret;
    }
    devdrv_put_soc_verify_symbol();
#endif

    file_check_flag[dev_id][file_type] = (int)TSFW_BIN_CHECK_SUCCESS;
    return ret;
}

int tsdrv_file_soc_verify(u32 dev_id, int file_type, void *file_src_addr, u32 file_size)
{
#define SLEEP_MS 10
    struct task_struct *check_thread = NULL;
    struct tsdrv_file_check_para para = {0};
    u32 times = 0;

    para.dev_id = dev_id;
    para.file_type = file_type;
    para.file_src_addr = file_src_addr;
    para.file_size = file_size;

    file_check_flag[dev_id][file_type] = 0;
    check_thread = kthread_create(devdrv_tsfw_soc_verify_thread,
        (void *)&para, "file_check_thread%u_%d", dev_id, file_type);
    if (IS_ERR(check_thread)) {
        TSDRV_PRINT_ERR("create file check thread fail, devid(%u) file_type(%d).\n", dev_id, file_type);
        return -EPERM;
    }
#ifndef AOS_LLVM_BUILD
    get_task_struct(check_thread);
#endif
    (void)wake_up_process(check_thread);

    while (1) {
        if (file_check_flag[dev_id][file_type] == (int)TSFW_BIN_CHECK_SUCCESS) {
            TSDRV_PRINT_INFO("file check success. devid(%u) file_type(%d).\n", dev_id, file_type);
            (void)kthread_stop(check_thread);
#ifndef AOS_LLVM_BUILD
            put_task_struct(check_thread);
#endif
            return 0;
        }
        if (file_check_flag[dev_id][file_type] == (int)TSFW_BIN_CHECK_FAILED) {
            TSDRV_PRINT_ERR("file check failed. devid(%u) file_type(%d).\n", dev_id, file_type);
            break;
        }

        if (times > FILE_CHECK_TIMEOUT) {
            TSDRV_PRINT_ERR("file check timeout. times(%u)(ms) devid(%u) file_type(%d)\n",
                times, dev_id, file_type);
            break;
        }

        msleep(SLEEP_MS);
        times++;
    }
    (void)kthread_stop(check_thread);
#ifndef AOS_LLVM_BUILD
    put_task_struct(check_thread);
#endif
    return -1;
}

int tsdrv_ffts_check(u32 dev_id, void *file_src_addr, u32 file_size)
{
#ifndef TSDRV_UT
    int ret;

    ret = tsdrv_file_soc_verify(dev_id, SOC_VERIFY_IMG_FFTS_PLUS_FW, file_src_addr, file_size);
    if (ret != 0) {
        TSDRV_PRINT_ERR("tsdrv_file_soc_verify failed, ret(%d).\n", ret);
        return ret;
    }

    return ret;
#else
    return 0;
#endif
}

/**
 * Whether to verify TS firmware signature
 *
 * According to MDC requirements, the TS firmware MDC does not
 * add signatures. The signature verification function is removed
 * but the signature header field is retained.
 */
STATIC bool devdrv_is_need_verify_tsfw(void)
{
#ifdef CFG_SOC_PLATFORM_MDC_V51
    return false;
#else
    return true;
#endif
}

int devdrv_tsfw_bin_check(struct devdrv_info *dev_info, const char *firmware_path)
{
    size_t fsize;
    int ret;

    ret = tsdrv_firmware_read(firmware_path, (void **)&dev_info->fw_src_addr, &fsize);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Read firmware failed. (path=%s)\n", firmware_path);
        return ret;
    }

    TSDRV_PRINT_INFO("Show firmware info. (devid=%u; size=%lu)\n",
        dev_info->dev_id, fsize);

    if (devdrv_is_need_verify_tsfw()) {
        dev_info->fw_len = (u32)fsize;
        ret = tsdrv_file_soc_verify(dev_info->dev_id, SOC_VERIFY_IMG_TSCH_FW, dev_info->fw_src_addr, (u32)fsize);
        if (ret != 0) {
            TSDRV_PRINT_ERR("Firmware verify failed. (devid=%u; ret=%d)\n", dev_info->dev_id, ret);
            vfree(dev_info->fw_src_addr);
            dev_info->fw_src_addr = NULL;
            dev_info->fw_verify = 0;
            return -TS_FW_VERIFY_FAILED;
        }
    }

    /* A signature is added to the TS firmware. Therefore, the signature
     * header needs to be removed from the data sent to the TS.
     * Set the size of the signature header by using tee_sec_head_size.
     * After the signature verification is complete, the signature header
     * is skipped based on the offset when the signature is loaded to the memory. */
    dev_info->sec_head_size = DEVDRV_SEC_HEAD_SIZE;
    dev_info->pos = 0;
    dev_info->fw_len = (u32)fsize - DEVDRV_SEC_HEAD_SIZE;
    dev_info->fw_verify = 1;

    TSDRV_PRINT_DEBUG("firmware check finished. (head size=%u, fw_len=%u)\n",
        dev_info->sec_head_size, dev_info->fw_len);
    return ret;
}

