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
* Description: fault inject node management
* Author: Huawei Technologies Co.Ltd
* Create: 2022-11-22
*/

#ifndef DRVMEM_FAULT_INJECT_NODE_H
#define DRVMEM_FAULT_INJECT_NODE_H

#include <linux/types.h>
#include <linux/mutex.h>

#define MAX_FAULT_NODE_NAME_LEN 64U
struct fault_node_fault {
	uint32_t type;
	uint8_t name[MAX_FAULT_NODE_NAME_LEN];
	int32_t (*inject)(uint64_t base);
};

struct fault_node {
	uint32_t type;
	uint8_t name[MAX_FAULT_NODE_NAME_LEN];
	uint32_t fault_num;
	struct fault_node_fault *fault;
};

struct fault_node_list_node {
	struct hlist_node list;
	struct fault_node node;
};

struct fault_node_list_head {
	struct hlist_head head;
	struct mutex lock;
};

struct fault_node_table_info {
	struct fault_node *node;
	uint32_t num;
};

void drvmem_fault_node_init(void);
void drvmem_fault_node_uninit(void);
void drvmem_fault_node_show_list(void);
int32_t drvmem_fault_node_register(const struct fault_node *node);
int32_t drvmem_fault_node_unregister(const struct fault_node *node);
int32_t drvmem_fault_node_run_proc(uint64_t base, uint32_t node_type, uint32_t fault_type);
#endif
