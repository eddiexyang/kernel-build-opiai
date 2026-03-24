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
 * Brief: for security upgrade, need check package's integrity by tee.
 * after checked successfully, call read/write interface supplied by
 * Tee to write to flash storage.
 */

#ifndef DEV_UPGRADE_SEC_H
#define DEV_UPGRADE_SEC_H

#include <linux/types.h>
#include "dev_upgrade_core.h"
#include "hiss/hsm_info.h"
#include "dev_upgrade_public.h"

#if (defined CFG_SOC_PLATFORM_MDC_V51)
#include "dev_upgrade_mdccore.h"
#ifdef LINUX_KERNEL_BUILD
#include "pkicms_api.h"
int pkicms_get_first_sign_addr(second_sign_t *image_input, char *read_buff, struct file *pfile, int *code_offset,
    unsigned int *first_len);
#endif
int dev_upgrade_init_img_buff(struct local_image_verify *local_img);
void dev_upgrade_release_img_buff(struct local_image_verify *local_img);
#endif

#define SUPPORT_TEE_SECURE_UPGRADE  1
#define DEV_UPGRADE_TEE_MAX_RETRY   3
#define DEV_UPGRADE_TEE_RETRY_WAIT  10

#define SEC_LARGE_IMG_VERIFY_END  1

typedef struct _part_xml_info_struct {
    loff_t offset;
    uint32_t lu;
    loff_t len;
} part_xml_info_struct;
/**
 * dev_upgrade_sec_upgrade_proc: update upgrade progress
 *
 */
int dev_upgrade_sec_upgrade_proc(dev_upgrade_core_ctrl *upgrade_ctrl);

int dev_upgrade_get_single_image_version(unsigned int dev_id, unsigned int cur_flag, unsigned int component_type,
    unsigned char *version_str, unsigned int max_len);
int get_ufs_capacity(u64 *capacity);
int devdrv_get_device_ids(u32 dev_id, u32 *board_id, u32 *slot_id);
#endif
