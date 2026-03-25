/* Copyright (c) Huawei Technologies Co., Ltd. 2018-2021. All rights reserved.
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

#ifndef DRV_UT
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/idr.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/hisi-spmi.h>
#include <linux/pm_runtime.h>
#define CREATE_TRACE_POINTS
#include "include/spmi.h"
#include <linux/of_hisi_spmi.h>
#include "hisi-spmi-dbgfs.h"


static DEFINE_MUTEX(board_lock);
static LIST_HEAD(board_list);
static DEFINE_IDR(ctrl_idr);
static struct device_type spmi_ctrl_type;
static struct device_type spmi_dev_type;

struct spmii_boardinfo {
	struct list_head	list;
	struct spmi_boardinfo	board_info;
};

/* Forward declarations */
static int spmi_register_controller(struct spmi_controller *ctrl);
struct bus_type spmi_bus_type;

/**
 * spmi_busnum_to_ctrl: Map bus number to controller
 * @busnum: number of bus
 * Returns controller representing this bus number
 */
struct spmi_controller *spmi_busnum_to_ctrl(u32 bus_num)
{
	struct spmi_controller *ctrl = NULL;

	mutex_lock(&board_lock);
	ctrl = idr_find(&ctrl_idr, bus_num);
	mutex_unlock(&board_lock);

	return ctrl;
}
EXPORT_SYMBOL_GPL(spmi_busnum_to_ctrl);

/**
 * spmi_add_controller: Controller bring up.
 * @ctrl: the controller to be registered.
 * A controller is registered with framework using this API. ctrl->nr is the
 * desired number with which SPMI framework registers controller.
 * Function will return -EBUSY if number is in use.
 */
int spmi_add_controller(struct spmi_controller *ctrl)
{
	int	ret_id;

	if (!ctrl)
		return -EINVAL;

	pr_debug("adding controller for bus %d (0x%pK)\n", ctrl->nr, ctrl);

	mutex_lock(&board_lock);
	ret_id = idr_alloc(&ctrl_idr, ctrl, ctrl->nr, ctrl->nr + 1, GFP_KERNEL);
	mutex_unlock(&board_lock);

	if (ret_id < 0)
		return ret_id;

	ctrl->nr = ret_id;
	return spmi_register_controller(ctrl);
}
EXPORT_SYMBOL_GPL(spmi_add_controller);

/* Remove a device associated with one controller */
static int spmi_ctrl_remove_device(struct device *dev, void *data)
{
	struct spmi_controller *ctrl = data;
	struct spmi_device *spmidev = to_spmi_device(dev);

	if (spmidev->ctrl == ctrl && dev->type == &spmi_dev_type)
		spmi_remove_device(spmidev);

	return 0;
}

/**
 * spmi_del_controller: Controller tear down.
 * @ctrl: the controller to be removed.
 *
 * Controller added with the above API is torn down using the API.
 */
int spmi_del_controller(struct spmi_controller *spmi_ctrl)
{
	struct spmi_controller *found = NULL;

	if (!spmi_ctrl)
		return -EINVAL;

	/* Check that the spmi_ctrl has been added */
	mutex_lock(&board_lock);
	found = idr_find(&ctrl_idr, spmi_ctrl->nr);
	mutex_unlock(&board_lock);
	if (found != spmi_ctrl)
		return -EINVAL;

	/* Remove all clients associated with this controller */
	mutex_lock(&board_lock);
	bus_for_each_dev(&spmi_bus_type, NULL, spmi_ctrl, spmi_ctrl_remove_device);
	mutex_unlock(&board_lock);

#if defined(CONFIG_HISI_SPMI_DEBUG_FS) || defined(CONFIG_HISI_SPMI_DEBUG_FS_MODULE)
	(void)spmi_dfs_del_controller(spmi_ctrl);
#endif
	mutex_lock(&board_lock);
	idr_remove(&ctrl_idr, spmi_ctrl->nr);
	mutex_unlock(&board_lock);

	init_completion(&spmi_ctrl->dev_released);
	device_unregister(&spmi_ctrl->dev);
	wait_for_completion(&spmi_ctrl->dev_released);

	return 0;
}
EXPORT_SYMBOL_GPL(spmi_del_controller);

#define spmi_ctrl_attr_gr NULL

static void spmi_ctrl_release(struct device *dev)
{
	struct spmi_controller *spmi_ctrl = to_spmi_controller(dev);

	complete(&spmi_ctrl->dev_released);
}

static struct device_type spmi_ctrl_type = {
	.groups		= spmi_ctrl_attr_gr,
	.release	= spmi_ctrl_release,
};

#define spmi_device_uevent NULL
#define spmi_device_attr_gr NULL

static void spmi_dev_release(struct device *dev)
{
	struct spmi_device *spmi_dev_temp = to_spmi_device(dev);
	kfree(spmi_dev_temp);
}

static struct device_type spmi_dev_type = {
	.uevent		= spmi_device_uevent,
	.groups		= spmi_device_attr_gr,
	.release	= spmi_dev_release,
};

/**
 * spmi_alloc_device: Allocate a new SPMI devices.
 * @spmi_ctrl: controller to which this device is to be
 * added to.
 * Context: can sleep
 *
 * Caller is responsible to call spmi_add_device() on the
 * returned spmi_device.  If the caller needs to discard the
 * spmi_device without adding it, then spmi_dev_put() should be called.
 *
 * Allows a driver to allocate and initialize a SPMI device
 * without registering it immediately.  This allows a driver
 * to directly fill the spmi_device structure before calling
 * spmi_add_device().
 */
struct spmi_device *spmi_alloc_device(struct spmi_controller *spmi_ctrl)
{
	struct spmi_device *spmi_dev_temp = NULL;

	if (!spmi_ctrl || !spmi_busnum_to_ctrl(spmi_ctrl->nr)) {
		pr_err("Missing the spmi controller\n");
		return NULL;
	}

	spmi_dev_temp = kzalloc(sizeof(*spmi_dev_temp), GFP_KERNEL);
	if (!spmi_dev_temp) {
		dev_err(&spmi_ctrl->dev, "unable to allocate spmi_dev\n");
		return NULL;
	}

	spmi_dev_temp->dev.parent = spmi_ctrl->dev.parent;
	spmi_dev_temp->dev.type = &spmi_dev_type;
	spmi_dev_temp->dev.bus = &spmi_bus_type;
	spmi_dev_temp->ctrl = spmi_ctrl;
	device_initialize(&spmi_dev_temp->dev);

	return spmi_dev_temp;
}
EXPORT_SYMBOL_GPL(spmi_alloc_device);

/* Validate spmi device structure */
static struct device *get_valid_device(struct spmi_device *spmi_dev)
{
	struct device *dev = NULL;

	if (!spmi_dev)
		return NULL;

	dev = &spmi_dev->dev;
	if (dev->bus != &spmi_bus_type || dev->type != &spmi_dev_type)
		return NULL;

	if (!spmi_dev->ctrl || !spmi_busnum_to_ctrl(spmi_dev->ctrl->nr))
		return NULL;

	return dev;
}

/**
 * spmi_add_device: Add one new device without registering board info.
 * @spmi_dev: the SPMI device to be added (registered).
 *
 * Called when device doesn't have an explicit client-driver to be probed,
 * or the client-driver is a module installed dynamically.
 */
int spmi_add_device(struct spmi_device *spmi_dev)
{
	int rc;
	struct device *valid_dev = get_valid_device(spmi_dev);

	if (!valid_dev) {
		pr_err("invalid SPMI device\n");
		return -EINVAL;
	}

	/* Set the device name */
	dev_set_name(valid_dev, "%s-%s", spmi_dev->name, valid_dev->of_node->name);

	/* Device may be bound to an active driver when this returns */
	rc = device_add(valid_dev);
	if (rc < 0)
		dev_err(valid_dev, "Can't add %s, status %d\n", dev_name(valid_dev), rc);
	else
		dev_info(valid_dev, "device %s registered\n", dev_name(valid_dev));
	return rc;
}
EXPORT_SYMBOL_GPL(spmi_add_device);

/**
 * spmi_new_device: Instantiates one new spmi device.
 * @spmi_ctrl: controller to which this device is to be added to.
 * @info: board information of this device.
 *
 * Returns the new device when success or return NULL when fail.
 */
struct spmi_device *spmi_new_device(struct spmi_controller *spmi_ctrl,
				struct spmi_boardinfo const *info)
{
	struct spmi_device *spmi_dev_temp = NULL;
	int rc;

	if (!spmi_ctrl || !info)
		return NULL;

	spmi_dev_temp = spmi_alloc_device(spmi_ctrl);
	if (!spmi_dev_temp)
		return NULL;

	spmi_dev_temp->dev.platform_data = (void *)info->platform_data;
	spmi_dev_temp->num_dev_node = info->num_dev_node;
	spmi_dev_temp->dev.of_node = info->of_node;
	spmi_dev_temp->dev_node = info->dev_node;
	spmi_dev_temp->sid  = info->slave_id;
	spmi_dev_temp->name = info->name;
	spmi_dev_temp->res = info->res;

	rc = spmi_add_device(spmi_dev_temp);
	if (rc < 0) {
		spmi_dev_put(spmi_dev_temp);
		return NULL;
	}
	return spmi_dev_temp;
}
EXPORT_SYMBOL_GPL(spmi_new_device);

/**
 * spmi_remove_device: Remove the effect
 * of spmi_add_device().
 */
void spmi_remove_device(struct spmi_device *spmidev)
{
	device_unregister(&spmidev->dev);
}
EXPORT_SYMBOL_GPL(spmi_remove_device);

static inline int spmi_read_cmd(struct spmi_controller *ctrl,
				u8 opcode, u8 sid, u16 addr, u8 bc, u8 *buf)
{
	int ret;
	if (!ctrl || !ctrl->read_cmd || ctrl->dev.type != &spmi_ctrl_type)
		return -EINVAL;

	trace_spmi_read_begin(opcode, sid, addr);
	ret = ctrl->read_cmd(ctrl, opcode, sid, addr, bc, buf);
	trace_spmi_read_end(opcode, sid, addr, ret, bc, buf);
	return ret;
}

static inline int spmi_write_cmd(struct spmi_controller *ctrl,
				u8 opcode, u8 sid, u16 addr, u8 bc, u8 *buf)
{
	int ret;
	if (!ctrl || !ctrl->write_cmd || ctrl->dev.type != &spmi_ctrl_type)
		return -EINVAL;

	trace_spmi_write_begin(opcode, sid, addr, bc, buf);
	ret =  ctrl->write_cmd(ctrl, opcode, sid, addr, bc, buf);
	trace_spmi_write_end(opcode, sid, addr, ret);
	return ret;
}

/*
 * Register read/write : use 5 bit address, and up to 1 byte of data
 * Extended register read/write : use 8 bit address, and up to 16 bytes of data
 * Extended register read/write long : use 16 bit address, and up to 8 bytes of data
 */

/**
 * spmi_register_read() : read register
 * @dev: spmi device.
 * @sid: slave identifier.
 * @addr: slave register address (5-bit address).
 * @buf: buffer to be populated with data from the slave.
 *
 * Reads one byte data from a Slave device register.
 */
int spmi_register_read(struct spmi_controller *spmi_ctrl, u8 sid, u8 addr, u8 *buf)
{
	/* 4-bit Slave Identifier, 5-bit register address */
	if (addr > 0x1F || sid > SPMI_MAX_SLAVE_ID)
		return -EINVAL;

	return spmi_read_cmd(spmi_ctrl, SPMI_CMD_READ, sid, addr, 0, buf);
}
EXPORT_SYMBOL_GPL(spmi_register_read);

/**
 * spmi_ext_register_read() - read extended register
 * @dev: spmi device.
 * @sid: slave identifier.
 * @addr: slave register address (8-bit address).
 * @len: the request number of bytes to read (up to 16 bytes).
 * @buf: buffer to be populated with data from the slave.
 *
 * Reads up to 16 bytes of data from the extended register
 * space on a Slave device.
 */
int spmi_ext_register_read(struct spmi_controller *spmi_ctrl,
				u8 sid, u16 addr, u8 *buf, int len)
{
	/* 4-bit Slave Identifier, 8-bit register address, up to 16 bytes */
	if (sid > SPMI_MAX_SLAVE_ID || \
		len <= 0 || \
		len > SPMI_CMD_EXT_READ_MAX_READ_LEN || \
		SPMI_CMD_EXT_READ_MAX_READ_REG_LEN < addr) {
		dev_err(&spmi_ctrl->dev, "slave_addr=%d(max:%d),len=%d(max:%d),reg_addr= %d(max:%d)\n", \
						sid, SPMI_MAX_SLAVE_ID,len, SPMI_CMD_EXT_READ_MAX_READ_LEN, \
						addr, SPMI_CMD_EXT_READ_MAX_READ_REG_LEN);
		return -EINVAL;
	}

	return spmi_read_cmd(spmi_ctrl, SPMI_CMD_EXT_READ, sid, addr, len, buf);
}
EXPORT_SYMBOL_GPL(spmi_ext_register_read);

/**
 * spmi_ext_register_readl() : extended register read long
 * @dev: spmi device.
 * @sid: slave identifier.
 * @addr: slave register address (16-bit address).
 * @len: the request number of bytes to read (up to 8 bytes).
 * @buf: buffer to be populated with data from the slave.
 *
 * Reads up to 8 bytes of data from the extended register
 * space on a Slave device using 16 bit address.
 */
int spmi_ext_register_readl(struct spmi_controller *spmi_ctrl,
				u8 sid, u16 addr, u8 *buf, int len)
{
	/* 4-bit Slave Identifier, 16-bit register address, up to 8 bytes */
	if (sid > SPMI_MAX_SLAVE_ID || len <= 0 || len > SPMI_CMD_EXT_READL_MAX_READ_LEN) {
		dev_err(&spmi_ctrl->dev, "slave_addr=%d(max:%d),len=%d(max:%d)\n", \
						sid, SPMI_MAX_SLAVE_ID, len, SPMI_CMD_EXT_READL_MAX_READ_LEN);
		return -EINVAL;
	}

	return spmi_read_cmd(spmi_ctrl, SPMI_CMD_EXT_READL, sid, addr, len, buf);
}
EXPORT_SYMBOL_GPL(spmi_ext_register_readl);

/**
 * spmi_register_write() - write register
 * @dev: spmi device.
 * @sid: slave identifier.
 * @addr: slave register address (5-bit address).
 * @buf: buffer containing the data to be transferred to the Slave.
 *
 * Writes one byte data to a Slave device register.
 */
int spmi_register_write(struct spmi_controller *spmi_ctrl, u8 sid, u8 addr, u8 *buf)
{
	u8 op = SPMI_CMD_WRITE;

	/* 4-bit Slave Identifier, 5-bit register address */
	if (addr > 0x1F || sid > SPMI_MAX_SLAVE_ID)
		return -EINVAL;

	return spmi_write_cmd(spmi_ctrl, op, sid, addr, 1, buf);
}
EXPORT_SYMBOL_GPL(spmi_register_write);

/**
 * spmi_ext_register_write() : write extended register
 * @dev: spmi device.
 * @sid: slave identifier.
 * @addr: slave register address (8-bit address).
 * @buf: buffer containing the data to be transferred to the Slave.
 * @len: the request number of bytes to read (up to 16 bytes).
 *
 * Writes up to 16 bytes of data to the extended
 * register space of a Slave device.
 */
int spmi_ext_register_write(struct spmi_controller *spmi_ctrl, u8 sid,
				u16 addr, u8 *buf, int len)
{
	u8 op = SPMI_CMD_EXT_WRITE;

	/* 4-bit Slave Identifier, 8-bit register address, up to 16 bytes */
	if (sid > SPMI_MAX_SLAVE_ID || \
	len <= 0 || \
	len > SPMI_CMD_EXT_WRITE_MAX_READ_LEN || \
	SPMI_CMD_EXT_WRITE_MAX_READ_REG_LEN < addr) {
		dev_err(&spmi_ctrl->dev, "slave_addr=%d(max:%d),len=%d(max:%d),reg_addr= %d(max:%d)\n", \
						sid, SPMI_MAX_SLAVE_ID, len, SPMI_CMD_EXT_WRITE_MAX_READ_LEN, \
						addr, SPMI_CMD_EXT_WRITE_MAX_READ_REG_LEN);
		return -EINVAL;
	}

	return spmi_write_cmd(spmi_ctrl, op, sid, addr, len, buf);
}
EXPORT_SYMBOL_GPL(spmi_ext_register_write);

/**
 * spmi_ext_register_writel() : extended register write long
 * @dev: spmi device.
 * @sid: slave identifier.
 * @addr: slave register address (16-bit address).
 * @buf: buffer containing the data to be transferred to the Slave.
 * @len: the request number of bytes to read (up to 8 byte).
 *
 * Writes up to 8 bytes data to the extended register
 * space of a Slave device using 16-bit address.
 */
int spmi_ext_register_writel(struct spmi_controller *spmi_ctrl, u8 sid,
				u16 addr, u8 *buf, int len)
{
	u8 op = SPMI_CMD_EXT_WRITEL;

	/* 4-bit Slave Identifier, 16-bit register address, up to 16 bytes */
	if (sid > SPMI_MAX_SLAVE_ID || len <= 0 || len > SPMI_CMD_EXT_WRITEL_MAX_READL_LEN) {
		dev_err(&spmi_ctrl->dev, "slave_addr=%d(max:%d),len=%d(max:%d)\n", \
						sid, SPMI_MAX_SLAVE_ID, len, SPMI_CMD_EXT_WRITEL_MAX_READL_LEN);
		return -EINVAL;
	}

	return spmi_write_cmd(spmi_ctrl, op, sid, addr, len, buf);
}
EXPORT_SYMBOL_GPL(spmi_ext_register_writel);

static const struct spmi_device_id *spmi_match(const struct spmi_device_id *sd_id,
				const struct spmi_device *spmi_dev)
{
	while (sd_id->name[0]) {
		if (strncmp(spmi_dev->name, sd_id->name, SPMI_NAME_SIZE) == 0)
			return sd_id;
		sd_id++;
	}
	return NULL;
}

static int spmi_device_match(struct device *dev, const struct device_driver *dev_drv)
{
	struct spmi_driver *sdrv = to_spmi_driver(dev_drv);
	struct spmi_device *spmi_dev_temp = NULL;

	if (dev->type == &spmi_dev_type)
		spmi_dev_temp = to_spmi_device(dev);
	else
		return 0;

	/* Match an OF style. */
	if (of_driver_match_device(dev, dev_drv))
		return 1;

	if (sdrv->id_table)
		return spmi_match(sdrv->id_table, spmi_dev_temp) != NULL;

	if (dev_drv->name)
		return strncmp(spmi_dev_temp->name, dev_drv->name, SPMI_NAME_SIZE) == 0;
	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int spmi_legacy_suspend(struct device *dev)
{
	struct spmi_driver *spmi_drv = NULL;
	struct spmi_device *spmi_dev_temp = NULL;

	if (dev->type == &spmi_dev_type)
		spmi_dev_temp = to_spmi_device(dev);

	if (!spmi_dev_temp || !dev->driver)
		return 0;

	spmi_drv = to_spmi_driver(dev->driver);
	if (!spmi_drv->suspend)
		return 0;

	return spmi_drv->suspend(spmi_dev_temp, PMSG_SUSPEND);
}

static int spmi_legacy_resume(struct device *dev)
{
	struct spmi_driver *spmi_drv = NULL;
	struct spmi_device *spmi_dev_temp = NULL;

	if (dev->type == &spmi_dev_type)
		spmi_dev_temp = to_spmi_device(dev);

	if (!spmi_dev_temp || !dev->driver)
		return 0;

	spmi_drv = to_spmi_driver(dev->driver);
	if (!spmi_drv->resume)
		return 0;

	return spmi_drv->resume(spmi_dev_temp);
}

static int spmi_pm_resume(struct device *dev)
{
	const struct dev_pm_ops *pm = dev->driver ? dev->driver->pm : NULL;

	if (pm)
		return pm_generic_resume(dev);
	else
		return spmi_legacy_resume(dev);
}

static int spmi_pm_suspend(struct device *dev)
{
	const struct dev_pm_ops *pm = dev->driver ? dev->driver->pm : NULL;

	if (pm)
		return pm_generic_suspend(dev);
	else
		return spmi_legacy_suspend(dev);
}

#else
#define spmi_pm_resume		NULL
#define spmi_pm_suspend		NULL
#endif

static SIMPLE_DEV_PM_OPS(spmi_dev_pm_ops, spmi_pm_suspend, spmi_pm_resume);

struct device spmi_dev = {
	.init_name = "spmi",
};

struct bus_type spmi_bus_type = {
	.name		= "spmi",
	.match		= spmi_device_match,
	.pm		= &spmi_dev_pm_ops,
};
EXPORT_SYMBOL_GPL(spmi_bus_type);

static void spmi_drv_shutdown(struct device *dev)
{
	const struct spmi_driver *sdrv = to_spmi_driver(dev->driver);

	sdrv->shutdown(to_spmi_device(dev));
}

static int spmi_drv_remove(struct device *dev)
{
	const struct spmi_driver *sdrv = to_spmi_driver(dev->driver);

	return sdrv->remove(to_spmi_device(dev));
}

static int spmi_drv_probe(struct device *dev)
{
	const struct spmi_driver *sdrv = to_spmi_driver(dev->driver);

	return sdrv->probe(to_spmi_device(dev));
}

/**
 * spmi_driver_register(): Client driver registration in SPMI framework.
 * @drv: client driver to be associated with client-device.
 *
 * This API will register the client driver with the SPMI framework.
 * It's call from the driver's module_init function.
 */
int spmi_driver_register(struct spmi_driver *spmi_drv)
{
	spmi_drv->driver.bus = &spmi_bus_type;

	if (spmi_drv->probe)
		spmi_drv->driver.probe = spmi_drv_probe;

	if (spmi_drv->remove)
		spmi_drv->driver.remove = spmi_drv_remove;

	if (spmi_drv->shutdown)
		spmi_drv->driver.shutdown = spmi_drv_shutdown;

	return driver_register(&spmi_drv->driver);
}
EXPORT_SYMBOL_GPL(spmi_driver_register);

static int spmi_register_controller(struct spmi_controller *spmi_ctrl)
{
	int ret = 0;

	dev_set_name(&spmi_ctrl->dev, "spmi-%d", spmi_ctrl->nr);
	spmi_ctrl->dev.bus = &spmi_bus_type;
	spmi_ctrl->dev.type = &spmi_ctrl_type;
	ret = device_register(&spmi_ctrl->dev);
	if (ret)
		goto exit;

	dev_dbg(&spmi_ctrl->dev, "Bus spmi-%d registered: dev:0x%pK\n",
					spmi_ctrl->nr, &spmi_ctrl->dev);

#if defined(CONFIG_HISI_SPMI_DEBUG_FS) || defined(CONFIG_HISI_SPMI_DEBUG_FS_MODULE)
	ret = spmi_dfs_add_controller(spmi_ctrl);
	if (ret) {
		dev_err(&spmi_ctrl->dev, "Bus spmi-%d registered: dev:0x%pK add debug fs controller failed!\n",
						spmi_ctrl->nr, &spmi_ctrl->dev);
		goto exit;
	}
#endif
	return ret;

exit:
	mutex_lock(&board_lock);
	idr_remove(&ctrl_idr, spmi_ctrl->nr);
	mutex_unlock(&board_lock);
	return ret;
}

static int __init spmi_init(void)
{
	int ret;

	ret = bus_register(&spmi_bus_type);
	if (!ret)
		ret = device_register(&spmi_dev);

	if (ret)
		bus_unregister(&spmi_bus_type);

	return ret;
}

static void __exit spmi_exit(void)
{
	device_unregister(&spmi_dev);
	bus_unregister(&spmi_bus_type);
}

module_exit(spmi_exit);
postcore_initcall(spmi_init);

MODULE_LICENSE("GPL v2");
MODULE_VERSION("1.0");
MODULE_DESCRIPTION("SPMI module");
MODULE_ALIAS("platform:spmi");
#else
int spmi_init(void)
{
	return 0;
}
#endif
