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

#ifndef TSDRV_KERNEL_UT
#include <linux/crypto.h>
#include <linux/fs.h>
#include <linux/scatterlist.h>
#include <linux/securec.h>
#ifndef AOS_LLVM_BUILD
#include <linux/securectype.h>
#endif
#include <linux/vmalloc.h>

#include "tsdrv_firmware_reader.h"
#include "devdrv_interface.h"
#include "tsdrv_log.h"
#include "tz_api.h"
#include "user_cfg_interface.h"
#include "tsdrv_file_load.h"

STATIC int devdrv_tsfw_tee_verify(u32 dev_id, const u8 *file_content, u32 file_size)
{
    int ret;
    const u8 *code_base = NULL;
    u32 head_offset = 0;
    int sign = PKCS_SIGN_TYPE_ON;

    ret = devdrv_config_get_pss_cfg_stub(0, &sign);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to invoke the devdrv_config_get_pss_cfg. (dev_id=%u; ret=%d)", dev_id, ret);
        return -EINVAL;
    }

    ret = verify_init_session((int)dev_id);
    if (ret != 0) {
        TSDRV_PRINT_ERR("dev_id(%u) verify init session failed, ret(%d).\n", dev_id, ret);
        return -EINVAL;
    }

    code_base = file_content + DEVDRV_SEC_HEAD_SIZE;
    if (sec_update_verification((int)dev_id, file_content, head_offset,
        code_base, file_size, (u32)sign) != 0) {
        TSDRV_PRINT_ERR("dev(%u) sec_update_verification fail.\n", dev_id);
        ret = -EINVAL;
    }

    verify_finalize_session((int)dev_id);
    return ret;
}

int devdrv_tsfw_bin_check(struct devdrv_info *dev_info, const char *firmware_path)
{
    int ret;
    size_t fsize;

    ret = tsdrv_firmware_read(firmware_path, (void **)&dev_info->fw_src_addr, &fsize);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Read firmware failed. (path=%s)\n", firmware_path);
        return ret;
    }

    ret = devdrv_tsfw_tee_verify(dev_info->dev_id, dev_info->fw_src_addr, (u32)fsize);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Firmware verify failed. (devid=%u; ret=%d)\n", dev_info->dev_id, ret);
        vfree(dev_info->fw_src_addr);
        dev_info->fw_src_addr = NULL;
        dev_info->fw_verify = 0;
        return -TS_FW_VERIFY_FAILED;
    }

    dev_info->sec_head_size = DEVDRV_SEC_HEAD_SIZE;
    dev_info->pos = 0;
    dev_info->fw_len = (u32)fsize - DEVDRV_SEC_HEAD_SIZE;
    dev_info->fw_verify = 1;

    return ret;
}
#endif

