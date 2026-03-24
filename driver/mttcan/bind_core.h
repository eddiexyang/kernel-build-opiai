/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
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
 * Create: 2019-10-15
 */

#ifndef IRQ_BIND_CORE_H
#define IRQ_BIND_CORE_H

#include <linux/printk.h>
#include <linux/cpumask.h>
#include "drv_log.h"

#define bind_core_err(fmt, ...)    drv_err("bind_core", fmt, ##__VA_ARGS__)
#define bind_core_info(fmt, ...)    drv_info("bind_core", fmt, ##__VA_ARGS__)

#ifdef COMMON_FUNC_PREFIX
#define COMBINE_STRING(prefix, func) prefix##_##func
#define ADD_PREFIX(prefix, func) COMBINE_STRING(prefix, func)
#define get_irq_cpumask ADD_PREFIX(COMMON_FUNC_PREFIX, get_irq_cpumask)
#define find_irq_cpumask ADD_PREFIX(COMMON_FUNC_PREFIX, find_irq_cpumask)
#define bind_irq_to_core ADD_PREFIX(COMMON_FUNC_PREFIX, bind_irq_to_core)
#endif

int get_irq_cpumask(struct cpumask *irq_cpumask,
                    const char *irq_node_name,
                    const char *irq_cpu_name,
                    unsigned int online_cpu_num);

struct cpumask *find_irq_cpumask(unsigned int irq);

/**
 * @irq:        Interrupt to set affinity
 * @irq_node_name:  The node name that defined to bind irq to core
 * @irq_cpu_name:   The property name that contains cpus
 *
 * Bind irq to one or more cores. Returns 0 if bound irq to cores on success,
 * -1 if bound it failed.
 * To use this headfile, establish a soft link in module's file,
 * and add it in the module's Makefile.
 */
int bind_irq_to_core(unsigned int irq, const char *irq_node_name, const char *irq_cpu_name);

#endif
