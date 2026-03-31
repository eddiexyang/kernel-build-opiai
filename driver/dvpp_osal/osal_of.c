/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
 * Description:
 * Author: huawei
 * Create: 2023-4-1
 */
#include "hi_osal.h"

#include <linux/of.h>
#include <linux/of_irq.h>
#ifdef AOS_LLVM_BUILD
#include <linux/export.h>
#endif

int osal_of_device_is_compatible(const void *device, const char *compat)
{
    return of_device_is_compatible((const struct device_node*)device, compat);
}
EXPORT_SYMBOL(osal_of_device_is_compatible);

void *osal_of_find_compatible_node(void *from, const char *type, const char *compatible)
{
    return of_find_compatible_node((struct device_node*)from, type, compatible);
}
EXPORT_SYMBOL(osal_of_find_compatible_node);

void *osal_of_find_node_by_name(void *from, const char *name)
{
    return of_find_node_by_name((struct device_node*)from, name);
}
EXPORT_SYMBOL(osal_of_find_node_by_name);

hi_void osal_of_node_put(void *node)
{
    of_node_put((struct device_node*)node);
}
EXPORT_SYMBOL(osal_of_node_put);

void *osal_of_get_next_child(const void *node, void *prev)
{
    return of_get_next_child((const struct device_node*)node, (struct device_node *)prev);
}
EXPORT_SYMBOL(osal_of_get_next_child);

void *osal_of_irq_find_parent(void *child)
{
    return of_irq_find_parent((struct device_node*)child);
}
EXPORT_SYMBOL(osal_of_irq_find_parent);
