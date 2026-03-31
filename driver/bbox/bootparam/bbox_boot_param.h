/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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

#ifndef BBOX_BOOT_PARAM_H
#define BBOX_BOOT_PARAM_H
#include "bbox_platform.h"

#define BBOX_DEVICE_MAX_NUM         4U
#define BBOX_REBOOT_REASON_LEN      25U
#define BBOX_REBOOT_REASON_STR_LEN  (BBOX_DEVICE_MAX_NUM * BBOX_REBOOT_REASON_LEN)
#define BBOX_SUB_REASON_LEN         8U
#define BBOX_SUB_REASON_STR_LEN     (BBOX_DEVICE_MAX_NUM * BBOX_SUB_REASON_LEN)
#define BBOX_DEVMEM_ADDR_LEN        32U
#define BBOX_DEVMEM_SIZE_LEN        32U

#ifdef BBOX_SOC_PLATFORM_MDC
#define AOS_CORE_DISABLE            0U
#define AOS_CORE_ENABLE             1U
#define CMDLINE_AOSENABLE_LEN       12U
#define BBOX_AOSMODE_SIZE_LEN       32U
#define CMDLINE_AOSENABLE           "kernel_utils"
#endif

#define BBOX_CLOCK_VIRTTIME          0U
#define BBOX_CLOCK_REALTIME          1U
#define BBOX_DPCLK_REAL              100U
#define BBOX_DPCLK_DEFAULT           0U
#define CMDLINE_DPCLK_LEN            3U
#define BBOX_DPCLK_SIZE              16U

#define CMDLINE_FILE "/proc/cmdline"


struct bbox_boot_param_mgr {
    const char *command_line;
    u8 reboot_etype[DEVICE_MAX_NUM];
    u8 sub_etype[DEVICE_MAX_NUM];
    u64 devmem_addr;
    u32 devmem_len;
    u32 device_num;
    u32 dpclk;
#ifdef BBOX_SOC_PLATFORM_MDC
    u32 aos_flag;
#endif
};

#define BBOX_BOOT_PARAM_SIZE      0x2800
#define BBOX_BOOT_PARAM_HEAD_SIZE 0x80
#define BBOX_BOOT_PARAM_DATA_SIZE 0x2780
#define BBOX_BOOT_PARAM_MAGIC     0xa1c3
#define BBOX_BOOT_PARAM_MAGIC_END 0xd2b4
#define BBOX_BOOT_PARAM_VERSION   0x1

#define REGS_REBOOT_REASOPN 1
#define REGS_PMU            2

#define BBOX_BOOT_DATA_AREA_NUM 8

struct bbox_boot_data_head {
    u8 type;
    u8 reserve[3];                  // reserve 3 byte
    u16 offset;
    u16 size;
};

struct bbox_boot_param {
    u32 magic;                                                  // BBOX_BOOT_PARAM_MAGIC
    u16 version;                                                // BBOX_BOOT_PARAM_VERSION
    u8 reboot_reason;
    u8 sub_reason;
    u64 devmem_addr;
    u64 devmem_len;
    u32 reserve[9];                                             // reserve 9 byte
    u32 magic_end;                                              // BBOX_BOOT_PARAM_MAGIC_END
    struct bbox_boot_data_head head[BBOX_BOOT_DATA_AREA_NUM];   // data head
    u8 data[0];                                                 // size is BBOX_BOOT_PARAM_DATA_SIZE
};

u32 bbox_get_device_num(void);
u8 bbox_get_reboot_type(u32 devid);
u8 bbox_get_sub_etype(u32 devid);
u64 bbox_get_devmem_addr(void);
u32 bbox_get_devmem_len(void);
#ifdef BBOX_SOC_PLATFORM_MDC
u32 bbox_get_aosmode_flag(void);
#endif

bool bbox_use_real_dpclk(void);
s32 bbox_boot_param_init(void);
#endif

