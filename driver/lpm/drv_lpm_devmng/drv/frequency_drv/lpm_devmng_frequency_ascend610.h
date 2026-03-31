/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
#ifndef LPM_DEVMNG_FREQUENCY_ASCEND610_H
#define LPM_DEVMNG_FREQUENCY_ASCEND610_H

#include <linux/types.h>
#include "lpm_devmng_common.h"
#include "lpm_devmng_ipc.h"
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
#include "lpm_devmng_debugfs.h"
#endif

#ifdef LPM_FREQUENCY

struct lpm_frequency_result_info {
	// 0: success, other: fail
	uint32_t result;
	// if query LPM_AICORE0_ID or LPM_AICORE1_ID:
	//     data[0x0]: aicore current frequency (LPM_AICORE0_ID)
	//     data[0x1]: aicore rated frequency (LPM_AICORE1_ID)
	// if query other type(not aicore):
	//     data[0x0]: frequency of the core_id
	//     data[0x1]: not used
	uint32_t data[0x2];
};

struct lpm_frequency_query_ipc_cfg {
	enum lpm_devmng_core_id core_id;
	enum lpm_devmng_ipc_send_type send_type;
	uint32_t data_pos; // position of query result data
};

struct lpm_frequency_query_ipc_list {
	uint32_t support_num;
	struct lpm_frequency_query_ipc_cfg *ipc_cfg;
};

struct lpm_frequency_drv_priv {
	// supported frequency types for queries from ipc
	struct lpm_frequency_query_ipc_list query_list;
};

#endif

#endif
