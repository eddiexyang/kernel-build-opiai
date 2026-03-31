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
*/
#ifndef LPM_DEVMNG_VRD_UPGRADE_H
#define LPM_DEVMNG_VRD_UPGRADE_H

#include <linux/types.h>
#include "lpm_devmng_common.h"

#define VRD_FIRMWARE_FILE_PATH  "vrd.bin"

// vrd type
enum {
	VRD_MP2975,
	VRD_XDPE132,    // not support yet
	VRD_XDPE122,
	VRD_BUTT
};

// upgrade flag
#define UPGRADE_VALID          BIT(0)   // ugrade valid
#define UPGRADE_AOTU_RESET     BIT(1)   // autoreset after upgrade success

#define VRD_FIRMWARE_SIZE_MAX  0x4000    // 16K share memory max
struct lpm_vrd_upgrade_info {
	uint8_t slave_addr;
	uint8_t vrd_type;
	uint16_t version;
	uint32_t flag;
	uint32_t firmware_size;
	uint8_t *firmware_buf;   // 4bytes for each config data:  [0]=page, [1]=offset, [2]=data_high [3]=data_low
};

struct lpm_vrd_info_cfg_in {
	uint32_t dev_id;
};

#ifdef LPM_VRD_UPGRADE

int32_t lpm_vrd_upgrade_request(uint32_t dev_id, const struct lpm_vrd_upgrade_info *vrd_upgrade_info);
int32_t lpm_vrd_upgrade_get_status_info(char *in, uint32_t in_len, char *out, uint32_t out_len);
int32_t lpm_vrd_upgrade_probe(uint64_t *param, uint32_t param_num);
int32_t lpm_vrd_upgrade_remove(uint64_t *param, uint32_t param_num);

#else

static inline int32_t lpm_vrd_upgrade_probe(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	return 0;
}
static inline int32_t lpm_vrd_upgrade_remove(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	return 0;
}

#endif

#endif