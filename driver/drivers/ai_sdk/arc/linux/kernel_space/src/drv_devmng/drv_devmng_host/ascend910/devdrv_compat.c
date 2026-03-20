#include <linux/device.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>

#include "devdrv_interface.h"
#include "devdrv_manager.h"
#include "devdrv_manager_container.h"

static devdrv_dev_state_notify g_dev_state_notify;

int agentdrv_get_msg_chan_devid(void *msg_chan);
void *agentdrv_get_msg_chan_priv(void *msg_chan);
int agentdrv_get_host_phy_mach_flag(u32 dev_id, u32 *host_flag);

int devdrv_get_msg_chan_devid(void *msg_chan)
{
	return agentdrv_get_msg_chan_devid(msg_chan);
}
EXPORT_SYMBOL(devdrv_get_msg_chan_devid);

void *devdrv_get_msg_chan_priv(void *msg_chan)
{
	return agentdrv_get_msg_chan_priv(msg_chan);
}
EXPORT_SYMBOL(devdrv_get_msg_chan_priv);

int devdrv_get_host_phy_mach_flag(u32 dev_id, u32 *host_flag)
{
	return agentdrv_get_host_phy_mach_flag(dev_id, host_flag);
}
EXPORT_SYMBOL(devdrv_get_host_phy_mach_flag);

int devdrv_get_device_index(u32 host_dev_id)
{
	u32 physical_dev_id = host_dev_id;
	u32 vfid = 0;

	if (!devdrv_manager_container_logical_id_to_physical_id(host_dev_id,
			&physical_dev_id, &vfid))
		return (int)physical_dev_id;

	return (int)host_dev_id;
}
EXPORT_SYMBOL(devdrv_get_device_index);

int devdrv_get_host_type(void)
{
	struct devdrv_manager_info *manager_info = devdrv_get_manager_info();

	if (manager_info == NULL)
		return HOST_TYPE_NORMAL;

	return (int)manager_info->host_type;
}
EXPORT_SYMBOL(devdrv_get_host_type);

struct device *devdrv_get_pci_dev_by_devid(u32 devid)
{
	(void)devid;
	return NULL;
}
EXPORT_SYMBOL(devdrv_get_pci_dev_by_devid);

int devdrv_get_bbox_reservd_mem(unsigned int devid, unsigned long long *dma_addr,
	struct page **dma_pages, unsigned int *size)
{
	if (dma_addr == NULL || dma_pages == NULL || size == NULL)
		return -EINVAL;

	*dma_addr = 0;
	*dma_pages = NULL;
	*size = 0;
	return 0;
}
EXPORT_SYMBOL(devdrv_get_bbox_reservd_mem);

void devdrv_unregister_black_callback(struct devdrv_black_callback *black_callback)
{
	(void)black_callback;
}
EXPORT_SYMBOL(devdrv_unregister_black_callback);

int devdrv_hot_reset_device(u32 dev_id)
{
	(void)dev_id;
	return -EOPNOTSUPP;
}
EXPORT_SYMBOL(devdrv_hot_reset_device);

int devdrv_get_device_boot_status(u32 devid, u32 *boot_status)
{
	(void)devid;
	if (boot_status == NULL)
		return -EINVAL;

	*boot_status = DSMI_BOOT_STATUS_UNINIT;
	return 0;
}
EXPORT_SYMBOL(devdrv_get_device_boot_status);

int devdrv_pcie_reinit(u32 dev_id)
{
	(void)dev_id;
	return -EOPNOTSUPP;
}
EXPORT_SYMBOL(devdrv_pcie_reinit);

bool devdrv_is_p2p_enabled(u32 dev_id, u32 peer_dev_id)
{
	(void)dev_id;
	(void)peer_dev_id;
	return false;
}
EXPORT_SYMBOL(devdrv_is_p2p_enabled);

int devdrv_enable_p2p(int pid, u32 dev_id, u32 peer_dev_id)
{
	(void)pid;
	(void)dev_id;
	(void)peer_dev_id;
	return -EOPNOTSUPP;
}
EXPORT_SYMBOL(devdrv_enable_p2p);

int devdrv_disable_p2p(int pid, u32 dev_id, u32 peer_dev_id)
{
	(void)pid;
	(void)dev_id;
	(void)peer_dev_id;
	return -EOPNOTSUPP;
}
EXPORT_SYMBOL(devdrv_disable_p2p);

int devdrv_get_p2p_access_status(u32 devid, u32 peer_devid, int *status)
{
	(void)devid;
	(void)peer_devid;
	if (status == NULL)
		return -EINVAL;

	*status = 0;
	return 0;
}
EXPORT_SYMBOL(devdrv_get_p2p_access_status);

int devdrv_set_module_init_finish(int dev_id, int module)
{
	(void)dev_id;
	(void)module;
	return 0;
}
EXPORT_SYMBOL(devdrv_set_module_init_finish);

int devdrv_get_addr_info(u32 devid, enum devdrv_addr_type type, u32 index,
	u64 *addr, size_t *size)
{
	(void)devid;
	(void)type;
	(void)index;

	if (addr == NULL || size == NULL)
		return -EINVAL;

	*addr = 0;
	*size = 0;
	return -EOPNOTSUPP;
}
EXPORT_SYMBOL(devdrv_get_addr_info);

int devdrv_get_master_devid_in_the_same_os(u32 dev_id, u32 *master_dev_id)
{
	if (master_dev_id == NULL)
		return -EINVAL;

	*master_dev_id = dev_id;
	return 0;
}
EXPORT_SYMBOL(devdrv_get_master_devid_in_the_same_os);

int devdrv_pcie_read_proc(u32 dev_id, enum devdrv_addr_type type, u32 offset,
	unsigned char *value, u32 len)
{
	(void)dev_id;
	(void)type;
	(void)offset;

	if (value == NULL)
		return -EINVAL;

	memset(value, 0, len);
	return -EOPNOTSUPP;
}
EXPORT_SYMBOL(devdrv_pcie_read_proc);

int devdrv_get_pcie_id_info(u32 devid, struct devdrv_pcie_id_info *pcie_id_info)
{
	(void)devid;
	if (pcie_id_info == NULL)
		return -EINVAL;

	memset(pcie_id_info, 0, sizeof(*pcie_id_info));
	return -EOPNOTSUPP;
}
EXPORT_SYMBOL(devdrv_get_pcie_id_info);

struct devdrv_dma_prepare *devdrv_dma_link_prepare(u32 devid,
	enum devdrv_dma_data_type type, struct devdrv_dma_node *dma_node,
	u32 node_cnt)
{
	struct devdrv_dma_prepare *dma_prepare;

	(void)devid;
	(void)type;

	if (dma_node == NULL || node_cnt == 0)
		return NULL;

	dma_prepare = kzalloc(sizeof(*dma_prepare), GFP_KERNEL);
	if (dma_prepare == NULL)
		return NULL;

	dma_prepare->devid = devid;
	dma_prepare->sq_size = node_cnt;
	dma_prepare->sq_dma_addr = (dma_addr_t)dma_node[0].dst_addr;
	return dma_prepare;
}
EXPORT_SYMBOL(devdrv_dma_link_prepare);

int devdrv_dma_link_free(struct devdrv_dma_prepare *dma_prepare)
{
	kfree(dma_prepare);
	return 0;
}
EXPORT_SYMBOL(devdrv_dma_link_free);

int devdrv_pcie_prereset(u32 dev_id)
{
	(void)dev_id;
	return -EOPNOTSUPP;
}
EXPORT_SYMBOL(devdrv_pcie_prereset);

void devdrv_flush_p2p(int pid)
{
	(void)pid;
}
EXPORT_SYMBOL(devdrv_flush_p2p);

int devdrv_register_black_callback(struct devdrv_black_callback *black_callback)
{
	(void)black_callback;
	return 0;
}
EXPORT_SYMBOL(devdrv_register_black_callback);

void drvdrv_dev_state_notifier_register(devdrv_dev_state_notify state_callback)
{
	g_dev_state_notify = state_callback;
}
EXPORT_SYMBOL(drvdrv_dev_state_notifier_register);

void devdrv_dev_state_notifier_unregister(void)
{
	g_dev_state_notify = NULL;
}
EXPORT_SYMBOL(devdrv_dev_state_notifier_unregister);
