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
* Description: fault inject
* Author: Huawei Technologies Co.Ltd
* Create: 2022-11-22
*/

#ifndef DRVMEM_FAULT_INJECT_H
#define DRVMEM_FAULT_INJECT_H

#include <linux/types.h>

#define FM_FAULT_INJECT_RESERVED_LEN 32U

struct fault_inject_info {
	uint32_t node_type;
	uint32_t node_id;
	uint32_t sub_node_type;
	uint32_t sub_node_id;
	uint32_t fault_type;
	uint32_t sub_fault_type;
	uint32_t times;
	uint8_t reserved[FM_FAULT_INJECT_RESERVED_LEN];
};

int32_t drvmem_fault_inject_init(void);
int32_t drvmem_fault_inject_uninit(void);

int32_t drvmem_fault_inject_handler(void *feature, char *in, unsigned int in_len, char *out, unsigned int out_len);

#endif
