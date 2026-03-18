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
#ifndef DRV_UT
#include <linux/irq.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/module.h>

#include <linux/hisi-spmi.h>
#include <linux/of_hisi_spmi.h>

struct of_spmi_res_info {
	struct device_node *node;
	uint32_t num_reg;
	uint32_t num_irq;
};

struct of_spmi_dev_info {
	struct spmi_controller *ctrl;
	struct spmi_boardinfo b_info;
};

#define SPMI_MAX_RES_NAME 256

/**
 * spmi_get_resource - get a resource for each device
 * @dev: device of spmi
 * @node: resource of device node
 * @type: type of resource
 * @res_num: index of resource
 *
 * If 'node' is specified as NULL, then the API treats this as a special case
 * to assume the first devnode. For configurations that do not use
 * spmi-dev-container, there is only one node to begin with,
 * so NULL should be passed in this case.
 *
 * Returns
 *  NULL when process failed.
 */
struct resource *spmi_get_resource(struct spmi_device *dev,
				struct spmi_resource *node, unsigned int type,
				unsigned int res_num)
{
	unsigned int num;

	/* if input node is not specified, default it to the first node */
	if (!node)
		node = &dev->res;

	for (num = 0; num < node->num_resources; num++) {
		struct resource *r = &node->resource[num];

		if (res_num-- == 0 && type == resource_type(r))
			return r;
	}
	return NULL;
}
EXPORT_SYMBOL_GPL(spmi_get_resource);

/**
 * spmi_get_resource_byname - get a resource for each device and given a name
 * @dev: spmi device
 * @node: resource of device node
 * @type: type of resource
 * @name: resource name
 */
struct resource *spmi_get_resource_byname(struct spmi_device *dev,
				struct spmi_resource *node,
				unsigned int type, const char *name)
{
	unsigned int num;

	/* if input node is not specified, default to the first node */
	if (!node)
		node = &dev->res;

	for (num = 0; num < node->num_resources; num++) {
		struct resource *r = &node->resource[num];

		if (!strncmp(r->name, name, SPMI_MAX_RES_NAME) && r->name &&
				type == resource_type(r))
			return r;
	}
	return NULL;
}
EXPORT_SYMBOL_GPL(spmi_get_resource_byname);

/**
 * spmi_get_irq - get an IRQ for each device
 * @dev: device of spmi
 * @node: resource of device node
 * @res_num: index of IRQ number
 *
 * Returns
 *  -ENXIO when process failed.
 */
int spmi_get_irq(struct spmi_device *dev, struct spmi_resource *node,
				unsigned int res_num)
{
	/* if input node is not specified, default to the first node */
	if (!node)
		node = &dev->res;

	return of_irq_get(node->of_node, res_num);
}
EXPORT_SYMBOL_GPL(spmi_get_irq);

/**
 * spmi_get_irq_byname - get an IRQ for each device and given a name
 * @dev: spmi device
 * @node: resource of device node
 * @name: resource name
 *
 * Returns -ENXIO when process failed.
 */
int spmi_get_irq_byname(struct spmi_device *dev,
				struct spmi_resource *node, const char *name)
{
	/* if input node is not specified, default to the first node */
	if (!node)
		node = &dev->res;

	return of_irq_get_byname(node->of_node, name);
}
EXPORT_SYMBOL_GPL(spmi_get_irq_byname);

/*
 * Initialize the r_info structure for the safe usage
 */
static inline void of_spmi_init_resource(struct of_spmi_res_info *r_info,
				struct device_node *node)
{
	r_info->num_irq = 0;
	r_info->num_reg = 0;
	r_info->node = node;
}

/*
 * To calculate the number of resources for allocating
 *
 * The caller is responsible to initialize the of_spmi_res_info structure.
 */
static void of_spmi_sum_resources(struct of_spmi_res_info *r_info,
				bool has_reg)
{
	struct of_phandle_args oirq;
	uint32_t flags;
	uint64_t size;
	int num = 0;

	while (of_irq_parse_one(r_info->node, num, &oirq) == 0)
		num++;

	r_info->num_irq += num;

	if (!has_reg)
		return;

	/*
	 * We can't use of_address_to_resource here since it
	 * includes address translation; and address translation assumes
	 * that no parent buses have a size-cell of 0. But SPMI does
	 * have a size-cell of 0.
	 */
	num = 0;
	while (of_get_address(r_info->node, num, &size, &flags) != NULL)
		num++;

	r_info->num_reg += num;
}

/*
 * Allocate enough memory to handle the resources associated with
 * the primary node.
 */
static int of_spmi_allocate_node_resources(struct of_spmi_dev_info *d_info,
				struct of_spmi_res_info *r_info)
{
	uint32_t num_reg = r_info->num_reg, num_irq = r_info->num_irq;
	struct resource *res = NULL;

	if (num_reg || num_irq) {
		res = kzalloc(sizeof(*res) * (num_reg + num_irq), GFP_KERNEL);
		if (!res)
			return -ENOMEM;
	}
	d_info->b_info.res.num_resources = num_irq + num_reg;
	d_info->b_info.res.resource = res;

	return 0;
}

/*
 * free the node resources - used for primary node
 */
static void of_spmi_free_node_resources(struct of_spmi_dev_info *d_info)
{
	kfree(d_info->b_info.res.resource);
}

static void of_spmi_populate_resources(struct of_spmi_dev_info *d_info,
				struct of_spmi_res_info *r_info,
				struct resource *res)
{
	uint32_t num_reg = r_info->num_reg, num_irq = r_info->num_irq;
	unsigned int i;
	const  __be32 *addrp = NULL;
	uint64_t size;
	uint32_t flags;

	if ((num_reg || num_irq) && (res != NULL)) {
		for (i = 0; i < num_reg; i++, res++) {
			/* Address length is always 16 bits */
			addrp = of_get_address(r_info->node, i, &size, &flags);
			BUG_ON(!addrp);
			res->start = be32_to_cpup(addrp);
			res->end = res->start + size - 1;
			res->flags = flags;
			of_property_read_string_index(r_info->node, "reg-names", i,
							&res->name);
		}
	}
}

/*
 * Gather the primary node resources and populate.
 */
static void of_spmi_populate_node_resources(struct of_spmi_dev_info *d_info,
				struct of_spmi_res_info *r_info)
{
	struct resource *spmi_res;

	spmi_res = d_info->b_info.res.resource;
	d_info->b_info.res.of_node = r_info->node;
	of_property_read_string(r_info->node, "label",
					&d_info->b_info.res.label);
	of_spmi_populate_resources(d_info, r_info, spmi_res);
}
/*
 * Allocate dev_node array for a spmi_device - used for spmi-dev-container
 */
static inline int of_spmi_alloc_devnode_store(struct of_spmi_dev_info *dev_info,
				uint32_t num_dev_node)
{
	dev_info->b_info.num_dev_node = num_dev_node;
	dev_info->b_info.dev_node = kzalloc(sizeof(struct spmi_resource) *
					num_dev_node, GFP_KERNEL);
	if (!dev_info->b_info.dev_node)
		return -ENOMEM;

	return 0;
}

/*
 * Allocate enough memory to handle the resources associated
 * with the spmi-dev-container nodes.
 */
static int of_spmi_allocate_devnode_resources(struct of_spmi_dev_info *d_info,
				struct of_spmi_res_info *r_info,
				uint32_t idx)
{
	uint32_t num_reg = r_info->num_reg, num_irq = r_info->num_irq;
	struct resource *res = NULL;

	if (num_reg || num_irq) {
		res = kzalloc(sizeof(*res) * (num_reg + num_irq), GFP_KERNEL);
		if (!res)
			return -ENOMEM;
	}
	d_info->b_info.dev_node[idx].num_resources = num_irq + num_reg;
	d_info->b_info.dev_node[idx].resource = res;

	return 0;
}

/*
 * create one single spmi_device
 */
static int of_spmi_create_device(struct of_spmi_dev_info *d_info,
				struct device_node *node)
{
	struct spmi_boardinfo *board_info = &d_info->b_info;
	struct spmi_controller *ctrl = d_info->ctrl;
	void *result = NULL;
	int rc;

	rc = of_modalias_node(node, board_info->name, sizeof(board_info->name));
	if (rc < 0) {
		dev_err(&ctrl->dev, "of spmi modalias failed on %s\n",
						node->full_name);
		return rc;
	}

	board_info->of_node = of_node_get(node);
	result = spmi_new_device(ctrl, board_info);
	if (result == NULL) {
		dev_err(&ctrl->dev, "of spmi: registering Failed %s\n",
						node->full_name);
		of_node_put(node);
		return -ENODEV;
	}
	return 0;
}
/*
 * free devnode resources : used with spmi_dev_container
 */
static void of_spmi_free_devnode_resources(struct of_spmi_dev_info *d_info)
{
	unsigned int node;

	for (node = 0; node < d_info->b_info.num_dev_node; node++)
		kfree(d_info->b_info.dev_node[node].resource);

	kfree(d_info->b_info.dev_node);
}
/*
 * Gather node devnode resources and populate : used with spmi_dev_container.
 */
static void of_spmi_populate_devnode_resources(struct of_spmi_dev_info *dev_info,
				struct of_spmi_res_info *r_info,
				int index)

{
	struct resource *spmi_res;

	spmi_res = dev_info->b_info.dev_node[index].resource;
	dev_info->b_info.dev_node[index].of_node = r_info->node;
	of_property_read_string(r_info->node, "label",
					&dev_info->b_info.dev_node[index].label);
	of_spmi_populate_resources(dev_info, r_info, spmi_res);
}

/*
 * Walks all children of a node containing the spmi_dev_container binding.
 * This special type of spmi_device can include resources from
 * more than one device node.
 */
static void of_spmi_walk_dev_container(struct of_spmi_dev_info *dev_info,
				struct device_node *container)
{
	struct spmi_controller *spmi_ctrl = dev_info->ctrl;
	struct of_spmi_res_info res_info = {};
	struct device_node *dev_node = NULL;
	int rc, i, num_dev_node = 0;

	if (!of_device_is_available(container))
		return;

	/*
	 * Count the total number of device_nodes so we know
	 * how much device_store to allocate.
	 */
	for_each_child_of_node(container, dev_node) {
		if (!of_device_is_available(dev_node))
			continue;
		num_dev_node++;
	}

	rc = of_spmi_alloc_devnode_store(dev_info, num_dev_node);
	if (rc) {
		dev_err(&spmi_ctrl->dev, "%s: unable to allocate devnode resources\n",
						__func__);
		return;
	}

	i = 0;
	for_each_child_of_node(container, dev_node) {
		if (!of_device_is_available(dev_node))
			continue;
		of_spmi_init_resource(&res_info, dev_node);
		of_spmi_sum_resources(&res_info, true);
		rc = of_spmi_allocate_devnode_resources(dev_info, &res_info, i);
		if (rc) {
			dev_err(&spmi_ctrl->dev, "%s: unable to allocate"
					" resources\n", __func__);
			of_spmi_free_devnode_resources(dev_info);
			return;
		}
		of_spmi_populate_devnode_resources(dev_info, &res_info, i);
		i++;
	}

	of_spmi_init_resource(&res_info, container);
	of_spmi_sum_resources(&res_info, true);

	rc = of_spmi_allocate_node_resources(dev_info, &res_info);
	if (rc) {
		dev_err(&spmi_ctrl->dev, "%s: unable to allocate resources\n",
								  __func__);
		of_spmi_free_node_resources(dev_info);
	}

	of_spmi_populate_node_resources(dev_info, &res_info);

	rc = of_spmi_create_device(dev_info, container);
	if (rc) {
		dev_err(&spmi_ctrl->dev, "%s: unable to create device for"
				" node %s\n", __func__, container->full_name);
		of_spmi_free_devnode_resources(dev_info);
		return;
	}
}

/*
 * Walks all children of a node containing the spmi_slave_container binding.
 * This indicates that all spmi_devices created from this point
 * all share the same slave_id.
 */
static void of_spmi_walk_slave_container(struct of_spmi_dev_info *dev_info,
				struct device_node *container)
{
	struct spmi_controller *ctrl = dev_info->ctrl;
	struct device_node *dev_node = NULL;
	int rc;

	for_each_child_of_node(container, dev_node) {
		struct of_spmi_res_info r_info;

		if (!of_device_is_available(dev_node))
			continue;

		/**
		 * Check to see if this dev_node contains children which
		 * should be all created as the same spmi_device.
		 */
		if (of_get_property(dev_node, "spmi-dev-container", NULL)) {
			of_spmi_walk_dev_container(dev_info, dev_node);
			continue;
		}

		of_spmi_init_resource(&r_info, dev_node);
		of_spmi_sum_resources(&r_info, true);

		rc = of_spmi_allocate_node_resources(dev_info, &r_info);
		if (rc) {
			dev_err(&ctrl->dev, "%s: unable to allocate"
						" resources\n", __func__);
			goto slave_err;
		}

		of_spmi_populate_node_resources(dev_info, &r_info);

		rc = of_spmi_create_device(dev_info, dev_node);
		if (rc) {
			dev_err(&ctrl->dev, "%s: unable to create device for"
				     " node %s\n", __func__, dev_node->full_name);
			goto slave_err;
		}
	}
	return;

slave_err:
	of_spmi_free_node_resources(dev_info);
}

int of_spmi_register_devices(struct spmi_controller *ctrl)
{
	struct device_node *dev_node = ctrl->dev.of_node;

	/* Only register child device if the ctrl have a node pointer set */
	if (!dev_node)
		return -ENODEV;

	for_each_child_of_node(ctrl->dev.of_node, dev_node) {
		struct of_spmi_dev_info dev_info = {};
		int rc, have_dev_container = 0;

		/* Get properties from the dev tree */
		rc = of_property_read_u8(dev_node, "slave_id", &dev_info.b_info.slave_id);
		if (rc) {
			dev_err(&ctrl->dev, "slave_id\n");
			continue;
		}

		dev_info.ctrl = ctrl;

		if (of_get_property(dev_node, "spmi-dev-container", NULL))
			have_dev_container = 1;
		if (of_get_property(dev_node, "spmi-slave-container", NULL)) {
			of_spmi_walk_dev_container(&dev_info, dev_node);
			if (!have_dev_container)
				of_spmi_walk_slave_container(&dev_info, dev_node);
		} else {
			struct of_spmi_res_info r_info;

			/**
			 * A dev container at the second level without a
			 * slave container is considered an error.
			 */
			if (have_dev_container) {
				dev_err(&ctrl->dev, "%s: structural error,"
				     " node %s has spmi_dev_container without"
				     " specifying spmi_slave_container\n",
								__func__, dev_node->full_name);
				continue;
			}

		if (!of_device_is_available(dev_node))
			continue;

		of_spmi_init_resource(&r_info, dev_node);
		of_spmi_sum_resources(&r_info, false);
		rc = of_spmi_allocate_node_resources(&dev_info, &r_info);
		if (rc) {
			dev_err(&ctrl->dev, "%s: unable to alloc"
					" resources.\n", __func__);
			of_spmi_free_node_resources(&dev_info);
			continue;
		}

		of_spmi_populate_node_resources(&dev_info, &r_info);

		rc = of_spmi_create_device(&dev_info, dev_node);
		if (rc) {
			dev_err(&ctrl->dev, "%s: unable to create "
					"device.\n", __func__);
				of_spmi_free_node_resources(&dev_info);
				continue;
			}
		}
	}

	return 0;
}
EXPORT_SYMBOL(of_spmi_register_devices);

MODULE_LICENSE("GPL v2");
#else
int of_spmi_register_devices(void)
{
	return 0;
}
#endif
