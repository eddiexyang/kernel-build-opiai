/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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

#include <linux/workqueue.h>
#include <linux/types.h>
#ifndef TSDRV_KERNEL_UT
#ifndef AOS_LLVM_BUILD
#include <linux/irq.h>
#else
#include <linux/module.h>
#endif
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/kallsyms.h>
#include <linux/version.h>
#include "kernel_version_adapt.h"
#include "tsdrv_log.h"
#include "kernel_version_adapt.h"
#include "tsdrv_cpumask.h"
/**
 *  The alloc_workqueue_attrs and free_workqueue_attrs functions is not
 *  exported in the kernel and needs to be searched through the symbol table.
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)) || (defined(AOS_LLVM_BUILD))
typedef struct workqueue_attrs *(alloc_workqueue_attrs_func)(gfp_t gfp_mask);
typedef void (free_workqueue_attrs_func)(struct workqueue_attrs *attrs);
#endif


/**
 * alloc workqueue attrs
 *
 * The alloc_workqueue_attrs function cannot be directly invoked because it is not exported.
 * need to search the kernel symbol table for this function.
 */
STATIC struct workqueue_attrs *tsdrv_alloc_workqueue_attrs(gfp_t gfp_mask)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)) && (!defined AOS_LLVM_BUILD)
    struct workqueue_attrs *attris;

    attris = kzalloc(sizeof(*attris), GFP_KERNEL);
    if (attris == NULL)
        goto FAIL;
    if (!alloc_cpumask_var(&attris->cpumask, GFP_KERNEL))
        goto FAIL;

    cpumask_copy(attris->cpumask, cpu_possible_mask);
    return attris;
FAIL:
    free_workqueue_attrs(attris);
    return NULL;
#else
    static alloc_workqueue_attrs_func *alloc_workqueue_attrs_pt = NULL;

    if (!alloc_workqueue_attrs_pt) {
        alloc_workqueue_attrs_pt = (alloc_workqueue_attrs_func *)
            (uintptr_t)__kallsyms_lookup_name("alloc_workqueue_attrs");
        if (IS_ERR_OR_NULL(alloc_workqueue_attrs_pt)) {
#ifndef TSDRV_UT
            TSDRV_PRINT_ERR("fail to find symbol alloc workqueue attrs\n");
            return NULL;
#endif
        }
    }
    return alloc_workqueue_attrs_pt(gfp_mask);
#endif
}

/**
 * free workqueue attrs
 *
 * The free_workqueue_attrs function cannot be directly invoked because it is not exported.
 * need to search the kernel symbol table for this function.
 */
STATIC void tsdrv_free_workqueue_attrs(struct workqueue_attrs *attrs)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
    static free_workqueue_attrs_func *free_workqueue_attrs_pt = NULL;
#endif

    if (attrs == NULL) {
        return;
    }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
    if (attrs) {
        free_cpumask_var(attrs->cpumask);
        kfree(attrs);
    }
#else
    if (!free_workqueue_attrs_pt) {
        free_workqueue_attrs_pt = (free_workqueue_attrs_func *)
            (uintptr_t)__kallsyms_lookup_name("free_workqueue_attrs");
        if (IS_ERR_OR_NULL(free_workqueue_attrs_pt)) {
#ifndef TSDRV_UT
            TSDRV_PRINT_ERR("fail to find symbol free workqueue attrs\n");
            return;
#endif
        }
    }
    free_workqueue_attrs_pt(attrs);
#endif
}

/**
 * Setting the Affinity of the Workqueue Thread
 * @wq: unbound type workqueue
 * @flag: WQ_HIGHPRI or 0
 *
 * Bind the work queue thread to the control CPU.
 */
int tsdrv_set_workqueue_affinity(struct workqueue_struct *wq, u32 flag)
{
    struct workqueue_attrs *attrs = NULL;
    static struct cpumask g_tsdrv_wq_cpumask;
    int ret;

    if (wq == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("alloc workqueue attrs failed\n");
        return -EINVAL;
#endif
    }

    tsdrv_get_ctrlcpu_mask(&g_tsdrv_wq_cpumask);
    if (cpumask_empty(&g_tsdrv_wq_cpumask)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("alloc workqueue attrs failed\n");
        return -EINVAL;
#endif
    }

    attrs = tsdrv_alloc_workqueue_attrs(GFP_KERNEL);
    if (attrs == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("alloc workqueue attrs failed\n");
        return -ENOMEM;
#endif
    }
#ifndef AOS_LLVM_BUILD
    attrs->nice = (flag & WQ_HIGHPRI) ? MIN_NICE : 0;
#else
    attrs->nice = 0;
#endif
    attrs->no_numa = true;
    cpumask_copy(attrs->cpumask, &g_tsdrv_wq_cpumask);

    ret = apply_workqueue_attrs(wq, attrs);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("apply workqueue attrs failed %d\n", ret);
#endif
    } else {
#ifndef TSDRV_UT
        TSDRV_PRINT_INFO("apply workqueue attrs success\n");
#endif
    }
    tsdrv_free_workqueue_attrs(attrs);
    return ret;
}
#else
int tsdrv_set_workqueue_affinity(struct workqueue_struct *wq, u32 flag)
{
    return 0;
}
#endif
