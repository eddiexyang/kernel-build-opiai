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

#include <linux/of_irq.h>
#include <linux/hisi-spmi.h>

#ifdef CONFIG_HISI_SPMI
/**
 * of_spmi_register_devices() - Register devices in spmi Device Tree
 * @ctrl: the spmi_controller which devices should be registered.
 *
 * This routine scans the SPMI Device Tree, allocating resources
 * and creating spmi_devices according to the SPMI bus Device
 * Tree hierarchy. Details of this hierarchy can be found in
 * Documentation/devicetree/bindings/spmi. This routine is
 * normally called from the probe routine of the driver registering
 * as a spmi_controller.
 */
int of_spmi_register_devices(struct spmi_controller *ctrl);
int spmi_get_irq(struct spmi_device *dev, struct spmi_resource *node, unsigned int res_num);
int spmi_get_irq_byname(struct spmi_device *dev, struct spmi_resource *node, const char *name);
struct resource *spmi_get_resource_byname(struct spmi_device *dev, struct spmi_resource *node,
				unsigned int type, const char *name);
struct resource *spmi_get_resource(struct spmi_device *dev, struct spmi_resource *node,
				unsigned int type, unsigned int res_num);

#else
static int of_spmi_register_devices(struct spmi_controller *ctrl)
{
	return -ENXIO;
}
#endif /* end of CONFIG_OF_SPMI */
