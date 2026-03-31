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

#include <drvmem_fault_inject_node.h>
#include <drvmem_fault_inject_define.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/securec.h>
#include <memory_log.h>

struct fault_node_list_head g_fault_node_list = {0};

/*
* @brief global fault node list init
* @param [in] void
* @return void
*/
void drvmem_fault_node_init(void)
{
	INIT_HLIST_HEAD(&g_fault_node_list.head);
	mutex_init(&g_fault_node_list.lock);
	memory_drv_info("init fault node list successfully\n");
}

/*
* @brief global fault node list deinit
* @param [in] void
* @return void
*/
void drvmem_fault_node_uninit(void)
{
	struct hlist_node *tmp = NULL;
	struct fault_node_list_node *n = NULL;

	/* delete all list nodes */
	mutex_lock(&g_fault_node_list.lock);
	hlist_for_each_entry_safe(n, tmp, &g_fault_node_list.head, list) {
		hlist_del_init(&n->list);
		kfree(n);
		n = NULL;
	}
	mutex_unlock(&g_fault_node_list.lock);
	/* free lock */
	mutex_destroy(&g_fault_node_list.lock);
	memory_drv_info("uninit fault node list successfully\n");
}

/*
* @brief show fault node list information
* @param [in] void
* @return void
*/
void drvmem_fault_node_show_list(void)
{
	uint32_t i;
	struct hlist_node *tmp = NULL;
	struct fault_node_list_node *n = NULL;

	mutex_lock(&g_fault_node_list.lock);
	/* iterate all list nodes */
	hlist_for_each_entry_safe(n, tmp, &g_fault_node_list.head, list) {
		memory_drv_info("node: type = %u, name = %s, fault num = %u\n", n->node.type, n->node.name, n->node.fault_num);
		for (i = 0; i < n->node.fault_num; i++) {
			memory_drv_info("	fault: type = %u, name = %s\n", n->node.fault[i].type, n->node.fault[i].name);
		}
	}
	mutex_unlock(&g_fault_node_list.lock);
}

/*
* @brief check if the node info is valid
* @param [in] node: pointer to fault node
* @return true: valid, false: invalid
*/
STATIC bool drvmem_fault_node_is_valid(const struct fault_node *node)
{
	/* check type */
	if (node->type > FAULT_NODE_TYPE_MATA) {
		memory_drv_err("invalid node type, type = %u\n", node->type);
		return false;
	}
	/* check name len */
	if (strlen(node->name) >= MAX_FAULT_NODE_NAME_LEN) {
		memory_drv_err("invalid node name, name = %s\n", node->name);
		return false;
	}
	/* check fault */
	if (node->fault == NULL) {
		memory_drv_err("null pointer of fault node table\n");
		return false;
	}

	return true;
}

/*
* @brief allocate a new fault node
* @param [in] node: pointer to fault node
* @return pointer to fault node list node
*/
STATIC struct fault_node_list_node *drvmem_fault_node_new_node(const struct fault_node *node)
{
	int32_t ret;
	struct fault_node_list_node *n = NULL;
	/* memory alloc node list */
	n = (struct fault_node_list_node *)kzalloc(sizeof(struct fault_node_list_node), GFP_KERNEL | __GFP_ACCOUNT);
	if (n == NULL) {
		memory_drv_err("allocate memory for fault node failed, size = %lu\n", sizeof(struct fault_node_list_node));
		goto err_alloc_node;
	}

	/* copy fault node info to fault list node */
	ret = memcpy_s(&n->node, sizeof(struct fault_node), node, sizeof(struct fault_node));
	if (ret != 0) {
		memory_drv_err("copy fault node failed, size = %lu, ret = %d\n", sizeof(struct fault_node), ret);
		goto err_copy_mem;
	}

	return n;

err_copy_mem:
	/* copy error free memory */
	kfree(n);
	n = NULL;
err_alloc_node:
	return NULL;
}

/*
* @brief find fault node in fault node list by node type
* @param [in] type: fault node type
* @return pointer to fault node list node
*/
STATIC struct fault_node_list_node *drvmem_fault_node_find_node(uint32_t type)
{
	struct hlist_node *tmp = NULL;
	struct fault_node_list_node *n = NULL;
	/* find fault node in fault node list by node type */
	hlist_for_each_entry_safe(n, tmp, &g_fault_node_list.head, list) {
		if (n->node.type == type)
			return n;
	}
	return NULL;
}

/*
* @brief add new fault node to fault node list
* @param [in] node: pointer to fault node list node
* @return 0: success, else: fail
*/
STATIC int32_t drvmem_fault_node_add_node(const struct fault_node *node)
{
	int32_t ret;
	struct fault_node_list_node *n = NULL;

	mutex_lock(&g_fault_node_list.lock);
	n = drvmem_fault_node_find_node(node->type);
	if (n == NULL) {	/* node not found, create one and add to list */
		n = drvmem_fault_node_new_node(node);
		if (n == NULL) {
			mutex_unlock(&g_fault_node_list.lock);
			/* create new fault node failed */
			memory_drv_err("create new fault node failed, type = %d, name = %s\n", node->type, node->name);
			return -ENOMEM;
		}
		hlist_add_head(&n->list, &g_fault_node_list.head);
		mutex_unlock(&g_fault_node_list.lock);
		return 0;
	}

	/* find node, update node info */
	memory_drv_info("fault node exists, update node info, type = %u, name = %s\n", node->type, node->name);
	ret = memcpy_s(&n->node, sizeof(struct fault_node), node, sizeof(struct fault_node));
	if (ret != 0) {
		/* memcpy copy error */
		mutex_unlock(&g_fault_node_list.lock);
		memory_drv_err("copy fault node failed, size = %lu, ret = %d\n", sizeof(struct fault_node), ret);
		return ret;
	}
	mutex_unlock(&g_fault_node_list.lock);

	return 0;
}

/*
* @brief delete fault node from fault node list
* @param [in] type: fault node type
* @return 0: success, else: fail
*/
STATIC int32_t drvmem_fault_node_delete_node(uint32_t type)
{
	struct fault_node_list_node *n = NULL;

	mutex_lock(&g_fault_node_list.lock);
	n = drvmem_fault_node_find_node(type);
	if (n == NULL) {
		/* node not found, return error */
		mutex_unlock(&g_fault_node_list.lock);
		memory_drv_err("fault node does not exist, type = %u\n", type);
		return -ENOENT;
	}
	/* free resources */
	hlist_del_init(&n->list);
	kfree(n);
	n = NULL;
	mutex_unlock(&g_fault_node_list.lock);

	return 0;
}

/*
* @brief register fault node to fault node list
* @param [in] node: pointer to fault node
* @return 0: success, else: fail
*/
int32_t drvmem_fault_node_register(const struct fault_node *node)
{
	int32_t ret;
	/* args check */
	if (!drvmem_fault_node_is_valid(node)) {
		memory_drv_err("invalid fault node parameters, type = %d, name = %s\n", node->type, node->name);
		return -EINVAL ;
	}
	/* add node */
	ret = drvmem_fault_node_add_node(node);
	if (ret != 0) {
		memory_drv_err("add fault node to list failed, type = %d, name = %s, ret = %d\n", node->type, node->name, ret);
		return ret;
	}
	memory_drv_info("register fault node successfully, type = %d, name = %s\n", node->type, node->name);

	return 0;
}

/*
* @brief unregister fault node from fault node list
* @param [in] node: pointer to fault node
* @return 0: success, else: fail
*/
int32_t drvmem_fault_node_unregister(const struct fault_node *node)
{
	int32_t ret;
	/* delete node */
	ret = drvmem_fault_node_delete_node(node->type);
	if (ret != 0) {
		memory_drv_err("delete fault node from list failed, type = %d, name = %s\n", node->type, node->name);
		return ret;
	}
	memory_drv_info("unregister fault node successfully, type = %d, name = %s\n", node->type, node->name);

	return 0;
}

/*
* @brief run fault inject handler of every fault type in each node
* @param [in] base: base addr
* @param [in] node_type: node type
* @param [in] fault_type: fault type
* @return 0: success, else: fail
*/
int32_t drvmem_fault_node_run_proc(uint64_t base, uint32_t node_type, uint32_t fault_type)
{
	int32_t ret;
	struct fault_node_list_node *n = NULL;

	mutex_lock(&g_fault_node_list.lock);
	/* get node */
	n = drvmem_fault_node_find_node(node_type);
	if (n == NULL) {
		/* fault node does not exist return error */
		mutex_unlock(&g_fault_node_list.lock);
		memory_drv_err("fault node does not exist, node type = %u\n", node_type);
		return -ENOENT;
	}
	/* run fault inject handler */
	ret = n->node.fault[fault_type].inject(base);
	if (ret != 0) {
		/* run fault inject failed return error */
		mutex_unlock(&g_fault_node_list.lock);
		memory_drv_err("run fault inject failed, node type = %u, node name = %s, fault type = %u, fault name = %s\n",
			n->node.type, n->node.name, n->node.fault[fault_type].type, n->node.fault[fault_type].name);
		return ret;
	}
	mutex_unlock(&g_fault_node_list.lock);
	memory_drv_info("run fault inject succ, node type = %u, node name = %s, fault type = %u, fault name = %s\n",
		n->node.type, n->node.name, n->node.fault[fault_type].type, n->node.fault[fault_type].name);

	return 0;
}
