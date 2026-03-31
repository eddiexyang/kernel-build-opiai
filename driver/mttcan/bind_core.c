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

#include <linux/of.h>
#include <linux/interrupt.h>
#include <linux/securec.h>
#include <linux/atomic.h>
#include "bind_core.h"

#ifdef SUPPORT_FLEXIBLE_BIND_CORE
#ifdef CTRL_CPU_INFO_USER_CFG
#include "user_cfg_interface.h"
#else
#include <linux/aos/cpu_domain_info.h>
#define AOSCORE_CTRLCPU_ARR_SIZE 32
#endif
#endif

#ifdef STATIC_SKIP
    #define STATIC
#else
    #define STATIC static
#endif

#define MAX_IRQ_CPUMASK_NUM 100
#define AOSCORE_MAX_ONLINE_CPU_NUM 16

struct irq_cpu_mask {
    unsigned int irq;
    struct cpumask irq_cpumask;
};

static struct irq_cpu_mask g_irq_cpu_mask_map[MAX_IRQ_CPUMASK_NUM] = {0};
static atomic_t g_irq_cpu_mask_idx = {0};

#ifdef SUPPORT_FLEXIBLE_BIND_CORE
#ifdef CTRL_CPU_INFO_USER_CFG
STATIC int get_flexible_irq_cpu(unsigned int *irq_cpu, const unsigned int online_cpu_num)
{
    dev_cpu_nums_cfg_t cpu_cfg = {0};
    unsigned int dev_id = 0; /* only support 1P device, dev_id = 0 */
    unsigned int ctrlcpu_num;
    int ret;

    ret = dev_user_cfg_get_cpu_number(dev_id, &cpu_cfg);
    if (ret != 0) {
        bind_core_err("Get devinfo failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return -EINVAL;
    }

    ctrlcpu_num = cpu_cfg.ccpu_num;
    if (ctrlcpu_num == 0) {
        bind_core_err("Invalid ctrlcpu number. (dev_id=%u; ctrlcpu_num=%u)\n", dev_id, ctrlcpu_num);
        return -EINVAL;
    }

    bind_core_info("Get irq cpu. (src_irq_cpu=%u; ctrlcpu_num=%u)\n", *irq_cpu, ctrlcpu_num);
    *irq_cpu = *irq_cpu % ctrlcpu_num;

    return 0;
}
#else /* else of CTRL_CPU_INFO_USER_CFG */
STATIC void dump_domain_cpu_info(struct cpu_domain_info domain_info)
{
    bind_core_err("domain_cpu_info:ctrl=%u,ctrlmap=%lu.\n"
        "domain_cpu_info:ai=%u,aimap=%lu.\n"
        "domain_cpu_info:data=%u,datamap=%lu.\n",
        domain_info.ctrlcpu_num, domain_info.ctrlcpu_bitmap,
        domain_info.aicpu_num, domain_info.aicpu_bitmap,
        domain_info.datacpu_num, domain_info.datacpu_bitmap);
}

STATIC int irq_check_domain_info_num(struct cpu_domain_info domain_info)
{
    /* ctrlcpu_num the minimum value is 1. */
    if (domain_info.ctrlcpu_num == 0U) {
        bind_core_err("invalid, The value of ctrlcpu_num should be greater than 0, "
            "but the actual value is %u.\n", domain_info.ctrlcpu_num);
        return -EINVAL;
    }
    return 0;
}

STATIC int get_flexible_irq_cpu(unsigned int *irq_cpu, const unsigned int online_cpu_num)
{
    int err;
    unsigned int i;
    unsigned int ctrlcpu_bitmap_num = 0;
    unsigned int ctrlcpu_arr[AOSCORE_CTRLCPU_ARR_SIZE] = {0};
    struct cpu_domain_info domain_info;

    err = get_cpudomain_info(&domain_info);
    if (err != 0) {
        bind_core_err("get_cpudomain_info failed,err = %d.\n", err);
        return -EINVAL;
    }

    err = irq_check_domain_info_num(domain_info);
    if (err != 0) {
        bind_core_err("irq check domain info failed\n");
        dump_domain_cpu_info(domain_info);
        return -EINVAL;
    }

    for (i = 0; i < AOSCORE_CTRLCPU_ARR_SIZE; i++) {
        if (((domain_info.ctrlcpu_bitmap >> i) & 0x1U) != 0) {
            ctrlcpu_arr[ctrlcpu_bitmap_num] = i;
            ctrlcpu_bitmap_num++;
        }
    }

    if (ctrlcpu_bitmap_num != domain_info.ctrlcpu_num) {
        for (i = 0; i < ctrlcpu_bitmap_num; i++) {
            bind_core_err("crtrlcpu_arr[%u] = %u.\n", i, ctrlcpu_arr[i]);
        }
        bind_core_err("ctrlcpu_bitmap_num-%u is not equal ctrlcpu_num-%u, failed\n",
            ctrlcpu_bitmap_num, domain_info.ctrlcpu_num);
        dump_domain_cpu_info(domain_info);
        return -EINVAL;
    }

    bind_core_info("srcirq = %u. online_cpu_num=%u, ctrlcpu_bitmap=%lu. ctrlcpu_num=%u.\n",
        *irq_cpu, online_cpu_num, domain_info.ctrlcpu_bitmap, domain_info.ctrlcpu_num);
    *irq_cpu = ctrlcpu_arr[*irq_cpu % ctrlcpu_bitmap_num];
#ifdef RUN_IN_AOS
    int aos_first_cpu_num = get_boot_cpu_phyid();
    if ((aos_first_cpu_num < 0) || (aos_first_cpu_num > (s32)online_cpu_num)) {
        bind_core_err("get_boot_cpu_phyid failed aos_first_cpu_num = %d\n", aos_first_cpu_num);
        return -EINVAL;
    }
    *irq_cpu += (unsigned int)aos_first_cpu_num;
#endif /* end of RUN_IN_AOS */

    return 0;
}
#endif /* end of CTRL_CPU_INFO_USER_CFG */
#endif /* end of SUPPORT_FLEXIBLE_BIND_CORE */

struct cpumask *find_irq_cpumask(unsigned int irq)
{
    int i;
    int irq_idx_now;
    struct cpumask *irq_cpumask = NULL;
    for (i = 0; (i < MAX_IRQ_CPUMASK_NUM) && (i < atomic_read(&g_irq_cpu_mask_idx)); i++) {
        if (g_irq_cpu_mask_map[i].irq == irq) {
            return &g_irq_cpu_mask_map[i].irq_cpumask;
        }
    }
    irq_idx_now = atomic_fetch_inc(&g_irq_cpu_mask_idx);
    if (irq_idx_now >= MAX_IRQ_CPUMASK_NUM) {
        atomic_dec(&g_irq_cpu_mask_idx);
        return NULL;
    }

    g_irq_cpu_mask_map[irq_idx_now].irq = irq;
    irq_cpumask = &g_irq_cpu_mask_map[irq_idx_now].irq_cpumask;
    return irq_cpumask;
}

int get_irq_cpumask(struct cpumask *irq_cpumask, const char *irq_node_name, const char *irq_cpu_name,
    unsigned int online_cpu_num)
{
    int i;
    int err;
    unsigned int irq_cpu;
    int property_cpu_num;
    int bind_core_num = 0;
    struct device_node *irq_cpu_node = NULL;
    struct device_node *irq_affinity_hint_node = NULL;

    irq_affinity_hint_node = of_find_node_by_name(NULL, "irq_affinity_hint");
    if (irq_affinity_hint_node == NULL) {
        bind_core_err("get irq_affinity_hint_node failed.\n");
        return -EINVAL;
    }

    irq_cpu_node = of_find_node_by_name(irq_affinity_hint_node, irq_node_name);
    if (irq_cpu_node == NULL) {
        bind_core_err("get irq_cpu_node \"%s\" failed.\n", irq_node_name);
        return -EINVAL;
    }

    property_cpu_num = of_property_count_u32_elems(irq_cpu_node, irq_cpu_name);
    if (property_cpu_num < 0) {
        bind_core_err("property \"%s\" has no valid irq_cpu, errcode=%d\n", irq_cpu_name, property_cpu_num);
        return property_cpu_num;
    }

    for (i = 0; i < property_cpu_num; i++) {
        err = of_property_read_u32_index(irq_cpu_node, irq_cpu_name, i, &irq_cpu);
        if (err < 0) {
            bind_core_err("get irq_cpu failed, errcode=%d\n", err);
            return err;
        }
#ifdef SUPPORT_FLEXIBLE_BIND_CORE
        err = get_flexible_irq_cpu(&irq_cpu, online_cpu_num);
        if (err != 0) {
            bind_core_err("get_irq_cpu_aos failed online_cpu_num = %u.\n", online_cpu_num);
            continue;
        }
#else
        if (irq_cpu >= online_cpu_num) {
            bind_core_err("cpu %d is out of online_cpu_num, bind irq to it failed.\n", irq_cpu);
            continue;
        }
#endif
        cpumask_set_cpu(irq_cpu, irq_cpumask);
        bind_core_num++;
    }

    if (bind_core_num <= 0) {
        bind_core_err("bind_core_num = %d, no cpu need bound.\n", bind_core_num);
        return -ERANGE;
    }
    return 0;
}

/**
 * Bind irq to one or more cores.
 */
int bind_irq_to_core(unsigned int irq, const char *irq_node_name, const char *irq_cpu_name)
{
    int err;
    struct cpumask *irq_cpumask;
    unsigned int online_cpu_num = 0;

    if (irq_node_name == NULL) {
        bind_core_err("irq_node_name is null.\n");
        return -ENODEV;
    }
    if (irq_cpu_name == NULL) {
        bind_core_err("irq_cpu_name is null.\n");
        return -EINVAL;
    }
#ifdef RUN_IN_AOS
    online_cpu_num = AOSCORE_MAX_ONLINE_CPU_NUM;
#else
    online_cpu_num = num_online_cpus();
#endif

    irq_cpumask = find_irq_cpumask(irq);
    if (irq_cpumask == NULL) {
        bind_core_err("no space to store cpumask.\n");
        return -EINVAL;
    }
    cpumask_clear(irq_cpumask);

    err = get_irq_cpumask(irq_cpumask, irq_node_name, irq_cpu_name, online_cpu_num);
    if (err < 0) {
        bind_core_err("irq %u get cpumask failed.\n", irq);
        return err;
    }

    err = irq_set_affinity_hint(irq, irq_cpumask);
    if (err < 0) {
        bind_core_err("irq %u set affinity_hint failed. errcode=%d\n", irq, err);
        return err;
    }

    bind_core_info("node %s\'s core binding result of irq %u is: %*pbl\n",
        irq_node_name, irq, online_cpu_num, irq_cpumask);
    return 0;
}

