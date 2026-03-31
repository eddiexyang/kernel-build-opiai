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
* Create: 2022-03-28
*/

#ifndef __VIRTMNGDEV_TS_H__
#define __VIRTMNGDEV_TS_H__

#include <linux/mutex.h>
#include "icm_interface.h"
#include "virtmng_public_def.h"

#ifdef CFG_SOC_PLATFORM_CLOUD_V2
#define VMNGD_ICM_MBX_CHAN  HISI_RPROC_TX_TS_MBX2
#else
#define VMNGD_ICM_MBX_CHAN HISI_RPROC_MAX       // invalid mbx chan, not used
#endif

/* VF sram addr: 0x680200000 + vf_id * 0x800 + 0x80 , vfid = [1, 12] */
#define STARS_RESOURCE_CFG_SRAM_BASE_ADDR 0x680200000
#define STARS_RESOURCE_CFG_SRAM_MAP_SIZE PAGE_SIZE * 7 /* each vf 2KB,  max 12 + 1 vf, totally 26KB */
#define STARS_AIC_PG_OFFSET 2
#define STARS_AIv_PG_OFFSET 4

#pragma pack(push)
#pragma pack (1)

typedef struct vmng_ts_ack {
    u32 result;
    u32 reserved[4];
} vmng_ts_ack_t;

typedef struct vmng_ts_msg {
    u8 vfid;
    u8 reserved_0[3];
    u32 reserved_1[4];
} vmng_ts_msg_t;
#pragma pack(pop)

int vmngd_send_msg_to_ts_create(u32 dev_id, u32 vfid, vmng_vf_cfg_t *cfg);
int vmngd_send_msg_to_ts_destory(u32 dev_id, u32 vfid, vmng_vf_cfg_t *cfg);
int vmngd_send_msg_to_ts_reset(u32 dev_id, u32 vfid);

#endif
