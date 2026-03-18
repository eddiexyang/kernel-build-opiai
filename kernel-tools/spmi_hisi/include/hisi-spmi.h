/* Copyright (c) Huawei Technologies Co., Ltd. 2018-2021. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _LINUX_HISI_SPMI_H
#define _LINUX_HISI_SPMI_H

#include <linux/device.h>
#include <linux/types.h>
#include <linux/mod_devicetable.h>

/* The Maximum of slave identifier */
#define SPMI_MAX_SLAVE_ID		16
#define SPMI_CMD_EXT_READ_MAX_READ_LEN  (16)
#define SPMI_CMD_EXT_READ_MAX_READ_REG_LEN  (0XFF)
#define SPMI_CMD_EXT_READL_MAX_READ_LEN  (8)
#define SPMI_CMD_EXT_WRITEL_MAX_READL_LEN  (8)
#define SPMI_CMD_EXT_WRITE_MAX_READ_LEN  (16)
#define SPMI_CMD_EXT_WRITE_MAX_READ_REG_LEN  (0XFF)

/* The SPMI Commands code */
enum spmi_commands {
	SPMI_CMD_EXT_WRITE = 0x00,
	SPMI_CMD_EXT_READ = 0x01,
	SPMI_CMD_EXT_WRITEL = 0x02,
	SPMI_CMD_EXT_READL = 0x03,
	SPMI_CMD_WRITE = 0x04,
	SPMI_CMD_READ = 0x05,
	SPMI_CMD_MAX,
};

struct spmi_device;

/**
 * struct spmi_controller: About interface to the SPMI master controller.
 * @nr: board-specific number identifier for this controller/bus.
 * @read_cmd: Send the register read command sequence on the SPMI bus.
 * @write_cmd: Send the register write command sequence on the SPMI bus.
 */
struct spmi_controller {
	struct device		dev;
	unsigned int		nr;
	struct completion	dev_released;
	int		(*read_cmd)(struct spmi_controller *, u8 opcode,
				u8 sid, u16 addr, u8 bc, u8 *buf);
	int		(*write_cmd)(struct spmi_controller *, u8 opcode,
				u8 sid, u16 addr, u8 bc, u8 *buf);
};
#define to_spmi_controller(d) container_of(d, struct spmi_controller, dev)

/**
 * struct spmi_driver: About manage SPMI generic/slave device driver function.
 * @probe: Binds the driver to a SPMI device.
 * @remove: Unbinds the driver from the SPMI device.
 * @shutdown: Shutdown the callback used during powerdown/halt.
 * @suspend: standard suspend the callback used during system suspend.
 * @resume: standard resume the callback used during system resume.
 * @driver: SPMI device drivers should initialize name and owner
 *	    field of this structure
 * @id_table: list of SPMI devices supported by this driver
 */
struct spmi_driver {
	int				(*probe)(struct spmi_device *dev);
	int				(*remove)(struct spmi_device *dev);
	void				(*shutdown)(struct spmi_device *dev);
	int				(*suspend)(struct spmi_device *dev, pm_message_t pmesg);
	int				(*resume)(struct spmi_device *dev);

	struct device_driver		driver;
	const struct spmi_device_id	*id_table;
};
#define to_spmi_driver(d) container_of(d, struct spmi_driver, driver)

/**
 * struct spmi_resource: The spmi_resource device_node.
 * @num_resources: The number of resources for this device node.
 * @resources: The array of resources for this device_node.
 * @of_node: device_node of the resource in question.
 * @label: The name used to reference the device from the driver.
 *
 * Note that we explicitly add a 'label' pointer here since per the
 * ePAPR 2.2.2, the device_node->name should be generic and not reflect
 * precise programming model. Thus label enables a platform specific
 * name to be assigned with the 'label' binding to allow for
 * unique query names.
 */
struct spmi_resource {
	struct resource		*resource;
	u32			num_resources;
	struct device_node	*of_node;
	const char		*label;
};

/**
 * About client/device handle (struct spmi_device):
 * ------------------------------------------
 *  This is the client/device handle returned when a
 *  SPMI device is registered with a controller.
 *  Pointer to this structure is used by client-driver as a handle.
 *  @dev: Driver model representation of the device.
 *  @name: The name of driver to use with this device.
 *  @ctrl: SPMI controller managing the bus hosting this device.
 *  @sid: The slave Identifier.
 */
struct spmi_device {
	struct device		dev;
	const char		*name;
	struct spmi_controller	*ctrl;
	struct spmi_resource	res;
	struct spmi_resource	*dev_node;
	u32			num_dev_node;
	u8			sid;
};
#define to_spmi_device(d) container_of(d, struct spmi_device, dev)


/**
 * struct spmi_boardinfo: Declare board information for SPMI device bringup.
 * @name: The name of driver to use with this device.
 * @slave_id: The slave identifier.
 * @of_node: The pointer to the OpenFirmware device node.
 * @dev_node: array of SPMI resources when used with spmi-dev-container.
 * @platform_data: About spmi_device platform_data.
 */
struct spmi_boardinfo {
	char			name[SPMI_NAME_SIZE];
	uint8_t			slave_id;
	struct device_node	*of_node;
	struct spmi_resource	res;
	struct spmi_resource	*dev_node;
	u32			num_dev_node;
	const void		*platform_data;
};

/**
 * spmi_driver_register(): Client driver registration in SPMI framework.
 * @drv: client driver to be associated with client-device.
 *
 * This API will register the client driver with the SPMI framework.
 * It's call from the driver's module_init function.
 */
extern int spmi_driver_register(struct spmi_driver *drv);

/**
 * spmi_driver_unregister(): reverse effect of spmi_driver_register.
 * @sdrv: the driver to unregister.
 * Context: can sleep
 */
static inline void spmi_driver_unregister(struct spmi_driver *sdrv)
{
	if (sdrv)
		driver_unregister(&sdrv->driver);
}

/**
 * spmi_del_controller: Controller tear-down.
 * Controller added with the above API is teared down using this API.
 */
extern int spmi_del_controller(struct spmi_controller *ctrl);

/**
 * spmi_add_controller: Controller bring-up.
 * @ctrl: controller to be registered.
 *
 * A controller is registered with the framework using this API. ctrl->nr
 * is the desired number with which SPMI framework registers the controller.
 * Function will return -EBUSY if the number is in use.
 */
extern int spmi_add_controller(struct spmi_controller *ctrl);


/**
 * spmi_busnum_to_ctrl: Map bus number to controller
 * @busnum: the bus number
 *
 * Returns controller device representing this bus number
 */
extern struct spmi_controller *spmi_busnum_to_ctrl(u32 bus_num);

/**
 * spmi_add_device: Add spmi_device allocated with spmi_alloc_device().
 * @spmi_dev: spmi_device to be added (registered).
 */
extern int spmi_add_device(struct spmi_device *spmi_dev);

/**
 * spmi_alloc_device: Allocate a new SPMI devices.
 * @ctrl: controller to which this device is to be added to.
 * Context: can to sleep
 *
 * Allows a driver to allocate and initialize a SPMI device
 * without registering it immediately.  This allows a driver to
 * directly fill the spmi_device structure before calling spmi_add_device().
 *
 * Caller is responsible to call spmi_add_device() on the returned spmi_device.
 * If the caller needs to discard the spmi_device without adding it,
 * then spmi_dev_put() should be called.
 */
extern struct spmi_device *spmi_alloc_device(struct spmi_controller *ctrl);

/**
 * spmi_new_device: Instantiates a new SPMI device.
 * @ctrl: controller to which this device is to be added to.
 * @info: About the board information for this device.
 *
 * Returns NULL or the new device.
 */
extern struct spmi_device *spmi_new_device(struct spmi_controller *ctrl,
				struct spmi_boardinfo const *info);

/* spmi_remove_device: Remove the effect of spmi_add_device() */
extern void spmi_remove_device(struct spmi_device *spmi_dev);

static inline void spmi_set_ctrldata(struct spmi_controller *ctrl, void *data)
{
	dev_set_drvdata(&ctrl->dev, data);
}

static inline void *spmi_get_ctrldata(const struct spmi_controller *ctrl)
{
	return dev_get_drvdata(&ctrl->dev);
}

static inline void *spmi_get_devicedata(const struct spmi_device *dev)
{
	return dev_get_drvdata(&dev->dev);
}


static inline void spmi_dev_put(struct spmi_device *spmidev)
{
	if (spmidev)
		put_device(&spmidev->dev);
}

static inline void spmi_set_devicedata(struct spmi_device *dev, void *data)
{
	dev_set_drvdata(&dev->dev, data);
}

/**
 * spmi_register_read() : read register
 * @dev: spmi device.
 * @sid: slave identifier.
 * @addr: slave register address (5-bit address).
 * @buf: buffer to be populated with data from the slave.
 *
 * Reads one byte data from a Slave device register.
 */
extern int spmi_register_read(struct spmi_controller *ctrl,
				u8 sid, u8 ad, u8 *buf);

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
extern int spmi_ext_register_read(struct spmi_controller *ctrl,
				u8 sid, u16 ad, u8 *buf, int len);

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
extern int spmi_ext_register_readl(struct spmi_controller *ctrl,
				u8 sid, u16 ad, u8 *buf, int len);

/**
 * spmi_register_write() - write register
 * @dev: spmi device.
 * @sid: slave identifier.
 * @addr: slave register address (5-bit address).
 * @buf: buffer containing the data to be transferred to the Slave.
 *
 * Writes one byte data to a Slave device register.
 */
extern int spmi_register_write(struct spmi_controller *ctrl,
				u8 sid, u8 ad, u8 *buf);

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
extern int spmi_ext_register_write(struct spmi_controller *ctrl,
				u8 sid, u16 ad, u8 *buf, int len);

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
extern int spmi_ext_register_writel(struct spmi_controller *ctrl,
				u8 sid, u16 ad, u8 *buf, int len);

#endif
