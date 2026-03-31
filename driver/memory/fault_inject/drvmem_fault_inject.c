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

#include <drvmem_fault_inject.h>
#include <drvmem_fault_inject_node.h>
#include <drvmem_fault_inject_define.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/securec.h>
#include <memory_log.h>
#include <dms_cmd_def.h>
#include <devdrv_manager_comm.h>
#include <drvmem_base_info.h>
#include <memory_adapter.h>

/*
* @brief fault inject init
* @param [in] void
* @return 0: success, else: fail
*/
int32_t drvmem_fault_inject_init(void)
{
	uint32_t i;
	struct fault_node_table_info table = {0};

	drvmem_fault_get_fault_node_table(&table);
	memory_drv_info("get fault node table successfully, num = %u\n", table.num);
	/* init node list */
	drvmem_fault_node_init();

	for (i = 0; i < table.num; i++) {
		/* regitster each node */
		int32_t ret = drvmem_fault_node_register(&table.node[i]);
		if (ret != 0) {
			memory_drv_err("register fault node failed, type = %d, name = %s\n",
				table.node[i].type, table.node[i].name);
			return -1;
		}
	}
	/* show node */
	drvmem_fault_node_show_list();
	memory_drv_info("init memory fault inject module successfully\n");

	return 0;
}

/*
* @brief fault inject uninit
* @param [in] void
* @return 0: success, else: fail
*/
int32_t drvmem_fault_inject_uninit(void)
{
	drvmem_fault_node_uninit();
	memory_drv_info("uninit memory fault inject module successfully\n");

	return 0;
}

/*
* @brief run fault inject handler of every fault type in each node
* @param [in] dev_id: device id
* @param [in] node_type: node type
* @param [in] fault_type: fault type
* @return 0: success, else: fail
*/
STATIC int32_t drvmem_fault_inject_proc(uint32_t dev_id, uint32_t node_type, uint32_t fault_type)
{
	int32_t ret;
	uint64_t base = 0;

	ret = memory_get_adapter()->get_base_addr(dev_id, &base);
	if (ret != 0) {
		memory_drv_err("get hbm base addr failed, dev id = %u\n", dev_id);
		return ret;
	}
	/* run fault inject handler */
	drvmem_fault_node_run_proc(base, node_type, fault_type);

	return 0;
}

/*
* @brief check fault inject handler param
* @param [in] in: input data
* @param [in] in_len: input data size
* @return 0: success, else: fail
*/
STATIC int32_t drvmem_fault_handler_check_param(char *in, unsigned int in_len)
{
	if (in == NULL) {
		memory_drv_err("null pointer of input data\n");
		return -EINVAL;
	}
	/* check len */
	if (in_len != sizeof(struct dms_fault_inject_in)) {
		memory_drv_err("invalid input data size, size = %u which should be %lu\n",
			in_len, sizeof(struct dms_fault_inject_in));
		return -EINVAL;
	}

	return 0;
}

/*
* @brief check fault inject handler input data
* @param [in] data: input data
* @return 0: success, else: fail
*/
STATIC int32_t drvmem_fault_check_input_data(struct dms_fault_inject_in *data)
{
	/* check buff */
	if (data->buff == NULL) {
		memory_drv_err("null pointer of input buffer\n");
		return -EINVAL;
	}
	/* check size */
	if (data->buff_size != sizeof(struct fault_inject_info)) {
		memory_drv_err("invalid input buffer size, size = %u which should be %lu\n",
			data->buff_size, sizeof(struct fault_inject_info));
		return -EINVAL;
	}

	return 0;
}

/* @brief fault inject handler
* @param [in] feature: feature list
* @param [in] in: input data
* @param [in] in_len: input data size
* @param [in] out: output data
* @param [in] out_len: output data size
* @return 0: success, else: fail
*/
int32_t drvmem_fault_inject_handler(void *feature, char *in, unsigned int in_len, char *out, unsigned int out_len)
{
	int32_t ret;
	struct dms_fault_inject_in *data = NULL;
	struct fault_inject_info info = {0};
	(void)feature;
	(void)out;
	(void)out_len;

	/* check params */
	ret = drvmem_fault_handler_check_param(in, in_len);
	if (ret != 0) {
		memory_drv_err("invalid param\n");
		return ret;
	}

	data = (struct dms_fault_inject_in *)in;
	ret = drvmem_fault_check_input_data(data);
	if (ret != 0) {
		memory_drv_err("invalid input data\n");
		return ret;
	}

	/* get data from userspace */
	ret = copy_from_user((void *)&info, data->buff, data->buff_size);
	if (ret != 0) {
		memory_drv_err("copy fault inject info from user space failed, size = %u, ret = %d\n", data->buff_size, ret);
		return ret;
	}
	/* run fault inject handler */
	ret = drvmem_fault_inject_proc(data->dev_id, info.node_type, info.fault_type);
	if (ret != 0) {
		memory_drv_err("run inject proc failed, dev id = %u, node type = %u, fault type = %u\n",
			data->dev_id, info.node_type, info.fault_type);
		return ret;
	}

	return 0;
}