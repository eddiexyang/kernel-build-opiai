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

#include <linux/types.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/jiffies.h>
#include <linux/pm.h>
#include <linux/slab.h>

#include "dms_pm_chain.h"
#include "devdrv_manager.h"

struct dms_pm_chain {
    struct list_head pm_list_header; /* for power manager */
    spinlock_t pm_list_lock;         /* for power manager */
};

static struct dms_pm_chain *dms_get_pm_chain(void)
{
    static struct dms_pm_chain pm_chain;
    return &pm_chain;
}


#ifndef CFG_FEATURE_HEART_BEAT
/*
 *
 * devdrv_manager_register_pm
 * run_stage: 0 - 1
 */
struct devdrv_pm *devdrv_manager_register_pm(int run_stage,
    int (*suspend)(u32 devid, u32 status), int (*resume)(u32 devid))
{
    struct dms_pm_chain *pm_chain = NULL;
    struct devdrv_pm *pm = NULL;
    unsigned long flags;

    if ((suspend == NULL) || (resume == NULL)) {
        devdrv_drv_err("callback func is null.\n");
        return NULL;
    }

    pm = kzalloc(sizeof(struct devdrv_pm), GFP_KERNEL | __GFP_ACCOUNT);
    if (pm == NULL) {
        devdrv_drv_err("kmalloc failed.\n");
        return NULL;
    }

    pm->suspend = suspend;
    pm->resume = resume;
    pm->run_stage = run_stage;

    pm_chain = dms_get_pm_chain();
    if (pm_chain == NULL) {
        devdrv_drv_err("pm_chain is null.\n");
        kfree(pm);
        pm = NULL;
        return NULL;
    }

    spin_lock_irqsave(&pm_chain->pm_list_lock, flags);
    list_add(&pm->list, &pm_chain->pm_list_header);
    spin_unlock_irqrestore(&pm_chain->pm_list_lock, flags);

    return pm;
}

EXPORT_SYMBOL(devdrv_manager_register_pm);
/*
 *
 * devdrv_manager_unregister_pm
 *
 */
void devdrv_manager_unregister_pm(struct devdrv_pm *pm)
{
    struct dms_pm_chain *pm_chain = NULL;
    unsigned long flags;

    if (pm == NULL) {
        devdrv_drv_err("invalid input argument.\n");
        return;
    }
    if ((pm->suspend == NULL) || (pm->resume == NULL)) {
        devdrv_drv_err("callback func is null.\n");
        return;
    }

    pm_chain = dms_get_pm_chain();
    if (pm_chain == NULL) {
        devdrv_drv_err("pm_chain is null.\n");
        return;
    }

    spin_lock_irqsave(&pm_chain->pm_list_lock, flags);
    list_del(&pm->list);
    spin_unlock_irqrestore(&pm_chain->pm_list_lock, flags);

    kfree(pm);
    pm = NULL;
}
EXPORT_SYMBOL(devdrv_manager_unregister_pm);
/*
 *
 * devdrv_manager_suspend_proc
 * suspend func
 */
int devdrv_manager_suspend_proc(int devid, int run_stage)
{
    int ret;
    unsigned long flags;
    struct list_head *pos = NULL, *n = NULL;
    struct list_head *stop = NULL;
    struct devdrv_pm *pm = NULL;
    struct dms_pm_chain *pm_chain = NULL;

    devdrv_drv_debug("call devdrv_manager_suspend_proc.\n");

    pm_chain = dms_get_pm_chain();
    if (pm_chain == NULL) {
        devdrv_drv_err("pm_chain is null.\n");
        return -EINVAL;
    }

    spin_lock_irqsave(&pm_chain->pm_list_lock, flags);
    list_for_each_safe(pos, n, &pm_chain->pm_list_header) {
        stop = pos;
        pm = list_entry(pos, struct devdrv_pm, list);
        if ((pm->suspend != NULL) && (pm->run_stage == run_stage)) {
            ret = pm->suspend(devid, TS_SUSPEND);
            if (ret) {
                spin_unlock_irqrestore(&pm_chain->pm_list_lock, flags);
                devdrv_drv_err("one suspend func failed ret = %d.\n", ret);
                return -EINVAL;
            }
        }
    }
    spin_unlock_irqrestore(&pm_chain->pm_list_lock, flags);

    devdrv_drv_info("devdrv_manager_suspend_proc succ.\n");
    return 0;
}
EXPORT_SYMBOL(devdrv_manager_suspend_proc);
/*
 *
 * devdrv_manager_resume_proc
 * resume func
 */
int devdrv_manager_resume_proc(struct devdrv_info *info)
{
    struct dms_pm_chain *pm_chain = NULL;
    struct list_head *pos = NULL, *n = NULL;
    struct devdrv_pm *pm = NULL;
    unsigned long flags;

    devdrv_drv_debug("call devdrv_manager_resume.\n");

    pm_chain = dms_get_pm_chain();
    if (pm_chain == NULL) {
        devdrv_drv_err("pm_chain is null.\n");
        return -EINVAL;
    }

    /* resume all registered resume callback func */
    spin_lock_irqsave(&pm_chain->pm_list_lock, flags);
    list_for_each_safe(pos, n, &pm_chain->pm_list_header) {
        pm = list_entry(pos, struct devdrv_pm, list);
        if (pm->resume != NULL) {
            (void)pm->resume(info->dev_id);
        }
    }
    spin_unlock_irqrestore(&pm_chain->pm_list_lock, flags);

    devdrv_drv_info("resume succ.\n");

    return 0;
}
EXPORT_SYMBOL(devdrv_manager_resume_proc);
#endif
/*
 *
 * dms_pm_chain_init
 * init func
 */
int dms_pm_chain_init(void)
{
    struct dms_pm_chain *pm_chain = NULL;

    devdrv_drv_debug("dms pm chain init.\n");

    pm_chain = dms_get_pm_chain();
    spin_lock_init(&pm_chain->pm_list_lock);
    INIT_LIST_HEAD(&pm_chain->pm_list_header);

    return 0;
}
