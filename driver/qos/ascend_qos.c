// SPDX-License-Identifier: GPL-2.0
#include <linux/export.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/string.h>

#include "ascend_kernel_hal.h"

struct qos_dsmi_handle {
	void *slot[8];
};

struct qos_group_ops {
	void *slot[6];
};

struct qos_soc_adapter {
	void *slot[9];
};

struct qos_node_entry {
	struct list_head list;
	struct qos_master_node node;
};

static DEFINE_MUTEX(g_qos_node_lock);
static LIST_HEAD(g_qos_nodes);
static struct qos_dsmi_handle g_dsmi_handle;
static struct qos_group_ops g_group_ops;
static struct qos_soc_adapter g_soc_adapter;
static bool g_pcie_handler_got;
static bool g_pcie_node_registered;
static struct qos_master_node g_pcie_node;

static int (*qos_soc_strategy)(unsigned int dev_id, unsigned int online, unsigned int master)
	= NULL;

void *get_cfg_handle(void);
void *get_group_ops(void);
void *get_soc_adapter(void);
void *qos_register_dsmi_handle(const void *handle);
void qos_unregister_dsmi_handle(void);
void *qos_register_group_handle(const void *ops);
void qos_unregister_group_handle(void);
void *qos_register_soc_adapter(const void *adapter);
void qos_unregister_soc_adapter(void);
int qos_node_register(const struct qos_master_node *master);
int qos_node_unregister(const struct qos_master_node *master);
int qos_init_default_config(unsigned int dev_id, unsigned int reason);
void qos_deinit_default_config(void);
int qos_online_offline_strategy_chg(unsigned int dev_id, unsigned int master, unsigned int online);
int qos_init_node_list(void);
void qos_deinit_node_list(void);
int qos_init_group_mng(void);
void qos_deinit_group_mng(void);
int init_module_DMS_QOS_NODE(void);
void exit_module_DMS_QOS_NODE(void);
int qos_feature_node_init(void);
void qos_feature_node_uninit(void);

static struct qos_node_entry *qos_find_entry(enum qos_master_type master)
{
	struct qos_node_entry *entry;

	list_for_each_entry(entry, &g_qos_nodes, list) {
		if (entry->node.cfg.type == master)
			return entry;
	}

	return NULL;
}

static bool qos_node_is_valid(const struct qos_master_node *master)
{
	if (master == NULL)
		return false;

	if (master->cfg.type >= MASTER_INVALID)
		return false;

	if (master->name[0] == '\0')
		return false;

	return true;
}

void *get_cfg_handle(void)
{
	return &g_dsmi_handle;
}

void *get_group_ops(void)
{
	return &g_group_ops;
}

void *get_soc_adapter(void)
{
	return &g_soc_adapter;
}

void *qos_register_dsmi_handle(const void *handle)
{
	if (handle == NULL)
		return NULL;

	memcpy(&g_dsmi_handle, handle, sizeof(g_dsmi_handle));
	return &g_dsmi_handle;
}
EXPORT_SYMBOL(qos_register_dsmi_handle);

void qos_unregister_dsmi_handle(void)
{
	memset(&g_dsmi_handle, 0, sizeof(g_dsmi_handle));
}
EXPORT_SYMBOL(qos_unregister_dsmi_handle);

void *qos_register_group_handle(const void *ops)
{
	if (ops == NULL)
		return NULL;

	memcpy(&g_group_ops, ops, sizeof(g_group_ops));
	return &g_group_ops;
}
EXPORT_SYMBOL(qos_register_group_handle);

void qos_unregister_group_handle(void)
{
	memset(&g_group_ops, 0, sizeof(g_group_ops));
}
EXPORT_SYMBOL(qos_unregister_group_handle);

void *qos_register_soc_adapter(const void *adapter)
{
	if (adapter == NULL)
		return NULL;

	memcpy(&g_soc_adapter, adapter, sizeof(g_soc_adapter));
	qos_soc_strategy = g_soc_adapter.slot[8];
	return &g_soc_adapter;
}
EXPORT_SYMBOL(qos_register_soc_adapter);

void qos_unregister_soc_adapter(void)
{
	memset(&g_soc_adapter, 0, sizeof(g_soc_adapter));
	qos_soc_strategy = NULL;
}
EXPORT_SYMBOL(qos_unregister_soc_adapter);

int qos_node_register(const struct qos_master_node *master)
{
	struct qos_node_entry *entry;
	struct qos_node_entry *found;

	if (!qos_node_is_valid(master))
		return -EINVAL;

	entry = kzalloc(sizeof(*entry), GFP_KERNEL);
	if (entry == NULL)
		return -ENOMEM;

	memcpy(&entry->node, master, sizeof(entry->node));

	mutex_lock(&g_qos_node_lock);
	found = qos_find_entry(master->cfg.type);
	if (found != NULL) {
		memcpy(&found->node, master, sizeof(found->node));
		mutex_unlock(&g_qos_node_lock);
		kfree(entry);
		return 0;
	}

	list_add_tail(&entry->list, &g_qos_nodes);
	mutex_unlock(&g_qos_node_lock);
	return 0;
}
EXPORT_SYMBOL(qos_node_register);

int qos_node_unregister(const struct qos_master_node *master)
{
	struct qos_node_entry *entry;

	if (master == NULL)
		return -EINVAL;

	mutex_lock(&g_qos_node_lock);
	entry = qos_find_entry(master->cfg.type);
	if (entry != NULL) {
		list_del(&entry->list);
		mutex_unlock(&g_qos_node_lock);
		kfree(entry);
		return 0;
	}
	mutex_unlock(&g_qos_node_lock);
	return 0;
}
EXPORT_SYMBOL(qos_node_unregister);

int hal_kernel_qos_node_register(const struct qos_master_node *master)
{
	return qos_node_register(master);
}
EXPORT_SYMBOL(hal_kernel_qos_node_register);

int hal_kernel_qos_node_unregister(const struct qos_master_node *master)
{
	return qos_node_unregister(master);
}
EXPORT_SYMBOL(hal_kernel_qos_node_unregister);

int qos_init_default_config(unsigned int dev_id, unsigned int reason)
{
	return 0;
}
EXPORT_SYMBOL(qos_init_default_config);

void qos_deinit_default_config(void)
{
}
EXPORT_SYMBOL(qos_deinit_default_config);

int qos_online_offline_strategy_chg(unsigned int dev_id, unsigned int master, unsigned int online)
{
	int ret = 0;

	if (qos_soc_strategy != NULL)
		ret = qos_soc_strategy(dev_id, online, master);

	if (ret != 0)
		(void)qos_init_default_config(dev_id, (unsigned int)ret);

	return 0;
}

int hal_kernel_qos_notify_module_online(int dev_id, enum qos_master_type master)
{
	return qos_online_offline_strategy_chg((unsigned int)dev_id, (unsigned int)master, 1);
}
EXPORT_SYMBOL(hal_kernel_qos_notify_module_online);

int hal_kernel_qos_notify_module_offline(int dev_id, enum qos_master_type master)
{
	return qos_online_offline_strategy_chg((unsigned int)dev_id, (unsigned int)master, 0);
}
EXPORT_SYMBOL(hal_kernel_qos_notify_module_offline);

int qos_init_node_list(void)
{
	void (*get_qos_func)(struct qos_master_node *master);
	int ret;

	memset(&g_pcie_node, 0, sizeof(g_pcie_node));
	strscpy(g_pcie_node.name, "pcie", sizeof(g_pcie_node.name));
	g_pcie_node.cfg.type = MASTER_PCIE;

	get_qos_func = __symbol_get("hal_kernel_agentdrv_get_qos_func");
	if (get_qos_func == NULL)
		return 0;

	g_pcie_handler_got = true;
	get_qos_func(&g_pcie_node);
	ret = qos_node_register(&g_pcie_node);
	if (ret != 0) {
		__symbol_put("hal_kernel_agentdrv_get_qos_func");
		g_pcie_handler_got = false;
		return ret;
	}

	g_pcie_node_registered = true;
	return 0;
}

void qos_deinit_node_list(void)
{
	if (g_pcie_node_registered) {
		(void)qos_node_unregister(&g_pcie_node);
		g_pcie_node_registered = false;
	}

	if (g_pcie_handler_got) {
		__symbol_put("hal_kernel_agentdrv_get_qos_func");
		g_pcie_handler_got = false;
	}
}

int qos_init_group_mng(void)
{
	return 0;
}

void qos_deinit_group_mng(void)
{
}

int init_module_DMS_QOS_NODE(void)
{
	int ret;

	ret = qos_init_group_mng();
	if (ret != 0)
		return ret;

	ret = qos_init_node_list();
	if (ret != 0) {
		qos_deinit_group_mng();
		return ret;
	}

	return 0;
}

void exit_module_DMS_QOS_NODE(void)
{
	struct qos_node_entry *entry;
	struct qos_node_entry *tmp;

	qos_deinit_node_list();
	qos_deinit_default_config();
	qos_deinit_group_mng();
	qos_unregister_soc_adapter();
	qos_unregister_group_handle();
	qos_unregister_dsmi_handle();

	mutex_lock(&g_qos_node_lock);
	list_for_each_entry_safe(entry, tmp, &g_qos_nodes, list) {
		list_del(&entry->list);
		kfree(entry);
	}
	mutex_unlock(&g_qos_node_lock);
}

int qos_feature_node_init(void)
{
	return init_module_DMS_QOS_NODE();
}

void qos_feature_node_uninit(void)
{
	exit_module_DMS_QOS_NODE();
}

static int __init ascend_qos_init(void)
{
	return init_module_DMS_QOS_NODE();
}

static void __exit ascend_qos_exit(void)
{
	exit_module_DMS_QOS_NODE();
}

module_init(ascend_qos_init);
module_exit(ascend_qos_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Recovered Ascend QoS provider");
MODULE_AUTHOR("OpenAI");
