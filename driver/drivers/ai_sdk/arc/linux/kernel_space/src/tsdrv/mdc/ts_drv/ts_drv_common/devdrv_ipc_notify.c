/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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
#ifdef CFG_FEATURE_IPC_NOTIFY
#include <linux/delay.h>
#ifndef AOS_LLVM_BUILD
#include <linux/irq.h>
#endif
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#ifndef AOS_LLVM_BUILD
#include <linux/idr.h>
#endif
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/vmalloc.h>
#include <linux/version.h>

#include "devdrv_ipc_notify.h"
#include "devdrv_manager_common.h"
#include "devdrv_manager.h"
#include "devdrv_nsm.h"
#include "devdrv_manager_container.h"
#include "tsdrv_device.h"
#include "devdrv_id.h"

#if defined(CFG_SOC_PLATFORM_MINI) && (!defined(CFG_SOC_PLATFORM_MINIV2)) && (!defined(CFG_SOC_PLATFORM_MINIV3))
#include "devdrv_manager_container_noplugin.h"
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 15, 0)
#ifndef AOS_LLVM_BUILD
#include <linux/sched/signal.h>
#endif
#endif

STATIC u32 devdrv_get_ts_num(void)
{
    u32 tsid;

#ifndef CFG_SOC_PLATFORM_MINIV2
    tsid = DEVDRV_MAX_TS_NUM;
#else
    tsid = 1;
#endif
    return tsid;
}

STATIC int devdrv_manager_ipc_notify_param_check(struct devdrv_notify_ioctl_info *notify_ioctl_info,
    struct ipc_notify_info *ipc_notify_info)
{
    u32 maxid;
    u32 tsnum;

    if ((notify_ioctl_info->dev_id >= TSDRV_MAX_DAVINCI_NUM) || (notify_ioctl_info->tsid >= DEVDRV_MAX_TS_NUM)
        || (ipc_notify_info == NULL) || (notify_ioctl_info->id_type != NOTIFY_ID_TYPE)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("invalid parameter, dev_id = %u, ts_id = %u, idtype = %u, (ipc_notify_info == NULL)(%d).\n",
            notify_ioctl_info->dev_id, notify_ioctl_info->tsid, notify_ioctl_info->id_type, (ipc_notify_info == NULL));
        return -ENODEV;
#endif
    }

    tsnum = devdrv_get_ts_num();
    if (notify_ioctl_info->tsid >= tsnum) {
#ifndef TSDRV_UT
        devdrv_drv_err("invalid ts id, dev_id = %u, ts_id = %u.\n",
            notify_ioctl_info->dev_id, notify_ioctl_info->tsid);
        return -ENODEV;
#endif
    }

    maxid = (notify_ioctl_info->id_type == IPC_EVENT_TYPE) ? DEVDRV_MAX_IPC_EVENT_ID : DEVDRV_MAX_NOTIFY_ID;

    if (notify_ioctl_info->notify_id >= maxid) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("invalid notify id %d.\n", notify_ioctl_info->notify_id);
        return -ENODEV;
#endif
    }

    return 0;
}

STATIC int devdrv_manager_ipc_notify_id_check(struct devdrv_notify_ioctl_info *notify_ioctl_info,
                                              struct ipc_notify_node *notify_node)
{
    if ((notify_node == NULL) || (notify_node->dev_id >= TSDRV_MAX_DAVINCI_NUM) ||
        (notify_node->notify_id >= DEVDRV_MAX_NOTIFY_ID)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("notify id range check failed, dev_id(%u), notify_id(%u)\n",
            (notify_node == NULL) ? 0 : notify_node->dev_id,
            (notify_node == NULL) ? 0 : notify_node->notify_id);
        return -ENODEV;
#endif
    }

    if ((notify_ioctl_info->dev_id != notify_node->dev_id) ||
        (notify_ioctl_info->notify_id != notify_node->notify_id)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("notify id check failed,  "
                        "notify_ioctl_info.dev_id = %u, "
                        "notify_node->dev_id = %u, "
                        "notify_ioctl_info.notify_id = %u, "
                        "notify_node->notify_id = %u\n",
                        notify_ioctl_info->dev_id, notify_node->dev_id, notify_ioctl_info->notify_id,
                        notify_node->notify_id);
        return -EFAULT;
#endif
    }

    return 0;
}

STATIC struct tsdrv_id_info *devdrv_manager_check_notify_info(struct tsdrv_ts_resource *ts_res,
    u32 notify_id, u32 type)
{
    struct tsdrv_id_info *notify_info = NULL;

    if (type == IPC_EVENT_TYPE) {
        notify_info = (struct tsdrv_id_info *)(ts_res->id_res[TSDRV_IPC_EVENT_ID].id_addr +
                                                       (long)sizeof(struct tsdrv_id_info) * notify_id);
    } else {
        notify_info = (struct tsdrv_id_info *)(ts_res->id_res[TSDRV_NOTIFY_ID].id_addr +
                                                       (long)sizeof(struct tsdrv_id_info) * notify_id);
    }

    spin_lock(&notify_info->spinlock);
    if (notify_info->ctx == NULL) {
#ifndef TSDRV_UT
        spin_unlock(&notify_info->spinlock);
        TSDRV_PRINT_ERR("notify id is not allocated yet, notify_id(%u)\n", notify_id);
        return NULL;
#endif
    }
    if (notify_info->id != (int)notify_id) {
#ifndef TSDRV_UT
        spin_unlock(&notify_info->spinlock);
        TSDRV_PRINT_ERR("invalid notify id(%d), notify_id = %u\n", notify_info->id, notify_id);
        return NULL;
#endif
    }
    spin_unlock(&notify_info->spinlock);

    return notify_info;
}

static inline struct tsdrv_id_info *devdrv_manager_get_notify_info(struct tsdrv_ts_resource *ts_res,
    u32 notify_id, u32 type)
{
    struct tsdrv_id_info *notify_info = NULL;

    if (type == IPC_EVENT_TYPE) {
        notify_info = (struct tsdrv_id_info *)(ts_res->id_res[TSDRV_IPC_EVENT_ID].id_addr +
                                                       (long)sizeof(struct tsdrv_id_info) * notify_id);
    } else {
        notify_info = (struct tsdrv_id_info *)(ts_res->id_res[TSDRV_NOTIFY_ID].id_addr +
                                                       (long)sizeof(struct tsdrv_id_info) * notify_id);
    }

    return notify_info;
}
#ifndef AOS_LLVM_BUILD
static void devdrv_manager_notify_node_proc(u32 dev_id, struct ipc_notify_node *notify_node,
    struct tsdrv_id_info *id_info, struct devdrv_notify_ioctl_info *ioctl_info, struct ipc_notify_info *ipc_info)
{
    u64 current_time = ktime_to_ns(ktime_get());
    int i;

    mutex_init(&notify_node->mutex);
    /* set ipc notify node ref to 1 when it created */
    atomic_set(&notify_node->open_info[0].ref, DEVDRV_IPC_REF_INIT);

    atomic_inc(&id_info->ref);

    mutex_lock(&ipc_info->info_mutex);
    mutex_lock(&notify_node->mutex);
    notify_node->delete_flag = 0;
    notify_node->dev_id = dev_id;
    notify_node->tsid = ioctl_info->tsid;
    notify_node->notify_id = ioctl_info->notify_id;
    notify_node->type = ioctl_info->id_type;
    notify_node->open_info[0].pid = (pid_t)current->tgid;
    notify_node->open_info[0].set_time = current_time;

    for (i = 0; i < DEVDRV_PID_MAX_NUM; i++) {
        notify_node->open_info[i].node = notify_node;
    }
    list_add(&notify_node->create_list_node, &ipc_info->create_list_head);
    mutex_unlock(&notify_node->mutex);

    ipc_info->create_fd_num++;
    TSDRV_PRINT_DEBUG("create ipc notify, phyid(%u), notify_id(%u), create_fd_num(%u).\n",
        dev_id, ioctl_info->notify_id, ipc_info->create_fd_num);
    mutex_unlock(&ipc_info->info_mutex);

    return;
}
#endif
int devdrv_manager_ipc_notify_create(void *c, unsigned long arg,  void *n)
{
#ifndef AOS_LLVM_BUILD
    struct devdrv_notify_ioctl_info *ioctl_info  = (struct devdrv_notify_ioctl_info *)n;
    struct devdrv_manager_context *context = (struct devdrv_manager_context *)c;
    struct ipc_notify_info *ipc_notify_info = context->ipc_notify_info;
    struct ipc_notify_node *notify_node = NULL;
    struct tsdrv_id_info *notify_info = NULL;
    struct tsdrv_device *ts_dev = NULL;
    u32 dev_id = 0;
    u32 vfid = 0;
    int ret;

    if (devdrv_manager_ipc_notify_param_check(ioctl_info, ipc_notify_info)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("invalid parameter. devid(%u)\n", ioctl_info->dev_id);
        return -ENODEV;
#endif
    }

    ret = devdrv_manager_container_logical_id_to_physical_id(ioctl_info->dev_id, &dev_id, &vfid);
    if ((ret != 0) || (vfid != TSDRV_PM_FID)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("logical id convert to physical id failed, ret(%d), logid(%u), phyid(%u), vfid(%u).\n", ret,
            ioctl_info->dev_id, dev_id, vfid);
        return -EFAULT;
#endif
    }

    ts_dev = tsdrv_get_dev(dev_id);
    if (ts_dev == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("dev_info(%pK) or cce_ctrl is NULL. phyid(%u)\n", ts_dev, dev_id);
        return -EINVAL;
#endif
    }
    if ((tsdrv_dev_ref_inc_return(dev_id, TSDRV_PM_FID) > TSDRV_MAX_DEV_REF) ||
        (tsdrv_get_dev_status(dev_id, TSDRV_PM_FID) != TSDRV_DEV_ACTIVE)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("device%u will be unregistered, cannot ioctl again.\n", dev_id);
        tsdrv_dev_ref_dec(dev_id, TSDRV_PM_FID);
        return -EEXIST;
#endif
    }

    notify_info = devdrv_manager_check_notify_info(&(ts_dev->dev_res[TSDRV_PM_FID].ts_resource[ioctl_info->tsid]),
        ioctl_info->notify_id, ioctl_info->id_type);
    if (notify_info == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("check notify id(%u) failed, phyid(%u)\n", ioctl_info->notify_id, dev_id);
        tsdrv_dev_ref_dec(dev_id, TSDRV_PM_FID);
        return -ENODEV;
#endif
    }
    /* Set ref to 1 when calling devdrv_alloc_notify_id , each notify_id corresponds to an ipc_name */
    if (atomic_read(&notify_info->ref) != 1) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("ipc_notify had been created. notify_id(%u), phyid(%u)\n", ioctl_info->notify_id, dev_id);
        tsdrv_dev_ref_dec(dev_id, TSDRV_PM_FID);
        return -EINVAL;
#endif
    }

    if (devdrv_ipc_create(ioctl_info->name, (void *)&notify_node, DEVDRV_IPC_NOTIFY)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("create ipc notify failed.\n");
        tsdrv_dev_ref_dec(dev_id, TSDRV_PM_FID);
        return -EINVAL;
#endif
    }

    ret = copy_to_user_safe((void *)((uintptr_t)arg), (void *)ioctl_info, sizeof(struct devdrv_notify_ioctl_info));
    if (ret != 0) {
#ifndef TSDRV_UT
        (void)devdrv_ipc_del(ioctl_info->name, DEVDRV_IPC_NAME_SIZE, DEVDRV_IPC_NOTIFY);
        tsdrv_dev_ref_dec(dev_id, TSDRV_PM_FID);
        TSDRV_PRINT_ERR("copy to user failed, ret(%d).\n", ret);
        return -EFAULT;
#endif
    }

    devdrv_manager_notify_node_proc(dev_id, notify_node, notify_info, ioctl_info, ipc_notify_info);
    tsdrv_dev_ref_dec(dev_id, TSDRV_PM_FID);

#endif
    return 0;
}

STATIC int devdrv_manager_match_pid(struct ipc_notify_node *notify_node, pid_t pid)
{
    int i;

    for (i = 0; i < DEVDRV_PID_MAX_NUM; i++) {
        if (notify_node->open_info[i].pid == pid) {
            return i;
        }
    }
    return DEVDRV_PID_MAX_NUM;
}

#define DEVDRV_NSEC_PER_SEC 1000000000L
STATIC u64 devdrv_manager_get_context_start_time(struct devdrv_manager_context *context)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0)
    return context->start_time;
#else
    return ((u64)(context->start_time.tv_sec * DEVDRV_NSEC_PER_SEC) + context->start_time.tv_nsec);
#endif
}

STATIC int devdrv_manager_check_pid(struct ipc_notify_node *notify_node, u64 start_time, pid_t pid)
{
    int index;

    index = devdrv_manager_match_pid(notify_node, pid);
    if ((index >= 0) && (index < DEVDRV_PID_MAX_NUM) &&
        (start_time <= notify_node->open_info[index].set_time)) {
        return index;
    }

    return DEVDRV_PID_MAX_NUM;
}

STATIC void devdrv_manager_ipc_notify_pid_show(struct ipc_notify_node *notify_node)
{
    int i;

    for (i = 0; i < DEVDRV_PID_MAX_NUM; i++) {
        TSDRV_PRINT_DEBUG("pid[%d] = %d start_time(%llu).devid(%u) notify(%u).\n", i, notify_node->open_info[i].pid,
            notify_node->open_info[i].set_time, notify_node->dev_id, notify_node->notify_id);
    }
}

/* open func passed to the user the physical id */
int devdrv_manager_ipc_notify_open(void *c, unsigned long arg, void *n)
{
    struct devdrv_notify_ioctl_info *ioctl_info  = (struct devdrv_notify_ioctl_info *)n;
    struct devdrv_manager_context *context = (struct devdrv_manager_context *)c;
    struct ipc_notify_info *ipc_notify_info = context->ipc_notify_info;
    struct ipc_notify_node *notify_node = NULL;
    struct tsdrv_id_info *notify_info = NULL;
    struct tsdrv_device *ts_dev = NULL;
    int pid_index;
    int ret;

    if (devdrv_ipc_find(ioctl_info->name, (void *)&notify_node, DEVDRV_IPC_NOTIFY)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("find ipc notify failed.\n");
        return -EINVAL;
#endif
    }

    pid_index = devdrv_manager_check_pid(notify_node, devdrv_manager_get_context_start_time(context), current->tgid);
    if (pid_index == DEVDRV_PID_MAX_NUM || pid_index == 0) {
        devdrv_manager_ipc_notify_pid_show(notify_node);
        TSDRV_PRINT_ERR("current tgid has no permission, pid_index(%d), start_time(%llu), notifyid(%d), devid(%u)\n",
            pid_index, devdrv_manager_get_context_start_time(context), notify_node->notify_id, notify_node->dev_id);
        return -EINVAL;
    }

    mutex_lock(&ipc_notify_info->info_mutex);
    if (atomic_inc_return(&notify_node->open_info[pid_index].ref) >= NOTIFY_MAX_OPEM_TIMES) {
#ifndef TSDRV_UT
        atomic_dec(&notify_node->open_info[pid_index].ref);
        mutex_unlock(&ipc_notify_info->info_mutex);
        TSDRV_PRINT_ERR("open notify times out of range.\n");
        return -EINVAL;
#endif
    }

    ts_dev = tsdrv_get_dev(notify_node->dev_id);
    if (ts_dev == NULL) {
#ifndef TSDRV_UT
        atomic_dec(&notify_node->open_info[pid_index].ref);
        mutex_unlock(&ipc_notify_info->info_mutex);
        TSDRV_PRINT_ERR("dev_info(%pK) or cce_ctrl is NULL. dev_id(%u)\n", ts_dev, notify_node->dev_id);
        return -EINVAL;
#endif
    }

    if ((tsdrv_dev_ref_inc_return(notify_node->dev_id, TSDRV_PM_FID) > TSDRV_MAX_DEV_REF) ||
        (tsdrv_get_dev_status(notify_node->dev_id, TSDRV_PM_FID) != TSDRV_DEV_ACTIVE)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("device%u will be unregistered, cannot ioctl again.\n", notify_node->dev_id);
        atomic_dec(&notify_node->open_info[pid_index].ref);
        tsdrv_dev_ref_dec(notify_node->dev_id, TSDRV_PM_FID);
        mutex_unlock(&ipc_notify_info->info_mutex);
        return -EEXIST;
#endif
    }

    notify_info = devdrv_manager_check_notify_info(&(ts_dev->dev_res[TSDRV_PM_FID].ts_resource[notify_node->tsid]),
        notify_node->notify_id, notify_node->type);
    if (notify_info == NULL) {
#ifndef TSDRV_UT
        atomic_dec(&notify_node->open_info[pid_index].ref);
        tsdrv_dev_ref_dec(notify_node->dev_id, TSDRV_PM_FID);
        mutex_unlock(&ipc_notify_info->info_mutex);
        TSDRV_PRINT_ERR("check notify id(%u) failed, dev_id(%u)\n", notify_node->notify_id, notify_node->dev_id);
        return -ENODEV;
#endif
    }

    ioctl_info->dev_id = notify_node->dev_id;
    ioctl_info->tsid = notify_node->tsid;
    ioctl_info->notify_id = notify_node->notify_id;
    ret = copy_to_user_safe((void *)(uintptr_t)arg, (void *)ioctl_info, sizeof(struct devdrv_notify_ioctl_info));
    if (ret == 0) {
        if (atomic_read(&notify_node->open_info[pid_index].ref) == 1) {
            atomic_inc(&notify_info->ref);
            ipc_notify_info->open_fd_num++;
            mutex_lock(&notify_node->mutex);
            list_add(&notify_node->open_info[pid_index].open_list_node, &ipc_notify_info->open_list_head);
            mutex_unlock(&notify_node->mutex);
        }
    } else {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("copy to user failed. dev_id(%u)\n", ioctl_info->dev_id);
        atomic_dec(&notify_node->open_info[pid_index].ref);
#endif
    }
    tsdrv_dev_ref_dec(notify_node->dev_id, TSDRV_PM_FID);
    mutex_unlock(&ipc_notify_info->info_mutex);

    return ret;
}

STATIC int devdrv_manager_ipc_notify_update_open_info(struct ipc_notify_node *node, pid_t pid,
    u64 current_time)
{
    u32 i;

    for (i = 0; i < DEVDRV_PID_MAX_NUM; i++) {
        if (node->open_info[i].pid == pid) {
            node->open_info[i].set_time = current_time;
            TSDRV_PRINT_DEBUG("set pid succ. dev_id(%u) notify(%u) set_time(%llu)\n",
                node->dev_id, node->notify_id, node->open_info[i].set_time);
            return 0;
        }
        if (node->open_info[i].pid == 0) {
            node->open_info[i].pid = pid;
            node->open_info[i].set_time = current_time;
            TSDRV_PRINT_DEBUG("set pid succ. dev_id(%u) notify(%u) set_time(%llu)\n",
                node->dev_id, node->notify_id, node->open_info[i].set_time);
            return 0;
        }
    }

    return -EINVAL;
}

int devdrv_manager_ipc_notify_set_pid(void *c, void *n)
{
    struct devdrv_notify_ioctl_info *notify_ioctl_info  = (struct devdrv_notify_ioctl_info *)n;
    struct ipc_notify_node *notify_node = NULL;
    u64 current_time = ktime_to_ns(ktime_get());
    unsigned int i;
    int ret;

    if (devdrv_ipc_find(notify_ioctl_info->name, (void *)&notify_node, DEVDRV_IPC_NOTIFY)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("find ipc notify failed.\n");
        return -EINVAL;
#endif
    }

    if (notify_node->open_info[0].pid != (pid_t)current->tgid) {
        TSDRV_PRINT_ERR("Invalid process, curr_tgid[%d], creat_tgid[%d].\n",
            current->tgid, notify_node->open_info[0].pid);
        return -EPERM;
    }

    for (i = 0; i < DEVDRV_PID_MAX_NUM; i++) {
        if (notify_ioctl_info->pid[i] == 0) {
            break;
        }
        ret = devdrv_manager_ipc_notify_update_open_info(notify_node, notify_ioctl_info->pid[i], current_time);
        if (ret != 0) {
#ifndef TSDRV_UT
            TSDRV_PRINT_ERR("set pid has reached the maximum value, the pid following index:%d fail to be set.\n", i);
            return -EINVAL;
#endif
        }
    }

    return 0;
}

STATIC struct ipc_notify_node *devdrv_manager_find_notify_node(struct ipc_notify_info *ipc_notify_info,
                                                               struct devdrv_notify_ioctl_info *notify_ioctl_info)
{
    struct ipc_notify_node *notify_node = NULL;

    if (devdrv_manager_ipc_notify_param_check(notify_ioctl_info, ipc_notify_info)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("invalid parameter\n");
        return NULL;
#endif
    }

    if (devdrv_ipc_find(notify_ioctl_info->name, (void *)&notify_node, DEVDRV_IPC_NOTIFY)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("find ipc notify failed.\n");
        return NULL;
#endif
    }

    if (devdrv_manager_ipc_notify_id_check(notify_ioctl_info, notify_node)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("notify id check failed. dev_id(%u)\n", notify_ioctl_info->dev_id);
        return NULL;
#endif
    }

    return notify_node;
}

static bool devdrv_manager_notify_should_del(struct ipc_notify_node *notify_node)
{
    int i;

    for (i = 0; i < DEVDRV_PID_MAX_NUM; i++) {
        if (atomic_read(&notify_node->open_info[i].ref) != 0) {
            return false;
        }
    }
    if (notify_node->delete_flag == 1) {
#ifndef TSDRV_UT
        TSDRV_PRINT_INFO("Already start to deleting\n");
        return false;
#endif
    }
    notify_node->delete_flag = 1;
    return true;
}

STATIC void devdrv_manager_ipc_notify_free(struct ipc_notify_node *notify_node)
{
    struct tsdrv_ts_resource *ts_resource = NULL;
    enum tsdrv_id_type id_type = TSDRV_NOTIFY_ID;
    struct tsdrv_id_info *notify_info = NULL;
    struct tsdrv_device *ts_dev = NULL;
    u32 notify_id;

    ts_dev = tsdrv_get_dev(notify_node->dev_id);
    if (ts_dev == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("dev_info(%pK) or cce_ctrl is NULL. dev_id(%u)\n", ts_dev, notify_node->dev_id);
        return;
#endif
    }
    if ((tsdrv_dev_ref_inc_return(notify_node->dev_id, TSDRV_PM_FID) > TSDRV_MAX_DEV_REF) ||
        (tsdrv_get_dev_status(notify_node->dev_id, TSDRV_PM_FID) != TSDRV_DEV_ACTIVE)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("device%u will be unregistered, cannot ioctl again.\n", notify_node->dev_id);
        tsdrv_dev_ref_dec(notify_node->dev_id, TSDRV_PM_FID);
        return;
#endif
    }

    notify_id = notify_node->notify_id;
    ts_resource = &(ts_dev->dev_res[TSDRV_PM_FID].ts_resource[notify_node->tsid]);
    notify_info = devdrv_manager_get_notify_info(ts_resource, notify_id, notify_node->type);
    if (notify_info == NULL) {
#ifndef TSDRV_UT
        tsdrv_dev_ref_dec(notify_node->dev_id, TSDRV_PM_FID);
        TSDRV_PRINT_INFO("unable to check notify id, notify_id(%u), dev_id(%u)"
                         "do not care it in recycle process\n", notify_id, notify_node->dev_id);
        return;
#endif
    }

    if (NOTIFY_ID_TYPE == IPC_EVENT_TYPE) {
        notify_id |= IPC_EVENT_MASK;
        id_type = TSDRV_IPC_EVENT_ID;
    }

    mutex_lock(&ts_resource->id_res[id_type].id_mutex_t);
    if (atomic_dec_and_test(&notify_info->ref) && (notify_info->valid == 1)) {
#ifndef TSDRV_UT
        if (devdrv_notify_ts_msg(notify_node->dev_id, notify_node->tsid, notify_node->type, notify_id, notify_info)) {
            TSDRV_PRINT_ERR("send ts notify reset msg failed. dev_id(%u)\n", notify_node->dev_id);
        }

        list_add(&notify_info->list, &ts_resource->id_res[id_type].id_available_list);
        notify_info->valid = 0;
        ts_resource->id_res[id_type].id_available_num++;
#endif
    }
    mutex_unlock(&ts_resource->id_res[id_type].id_mutex_t);
    tsdrv_dev_ref_dec(notify_node->dev_id, TSDRV_PM_FID);

    return;
}

int devdrv_manager_ipc_notify_destroy(void *c, void *n)
{
#ifndef AOS_LLVM_BUILD
    struct devdrv_manager_context *context = (struct devdrv_manager_context *)c;
    struct devdrv_notify_ioctl_info *notify_ioctl_info  = (struct devdrv_notify_ioctl_info *)n;
    struct ipc_notify_info *ipc_notify_info = context->ipc_notify_info;
    struct ipc_notify_node *notify_node = NULL;
    bool notify_node_del;
    u32 logic_id, vfid;
    int ret;

    logic_id = notify_ioctl_info->dev_id;

    ret = devdrv_manager_container_logical_id_to_physical_id(logic_id, &notify_ioctl_info->dev_id, &vfid);
    if (ret != 0) {
        TSDRV_PRINT_ERR("logical id convert to physical id failed, ret(%d), logic_id(%u), phyid(%u).\n",
            ret, logic_id, notify_ioctl_info->dev_id);
        return -EFAULT;
    }

    mutex_lock(&ipc_notify_info->info_mutex);
    notify_node = devdrv_manager_find_notify_node(ipc_notify_info, notify_ioctl_info);
    if (notify_node == NULL) {
        mutex_unlock(&ipc_notify_info->info_mutex);
        TSDRV_PRINT_ERR("find notify node failed\n");
        return -EFAULT;
    }

    if (notify_node->open_info[0].pid != current->tgid) {
#ifndef TSDRV_UT
        mutex_unlock(&ipc_notify_info->info_mutex);
        TSDRV_PRINT_ERR("current tgid has no permission to destroy the notify node.\n");
        return -EFAULT;
#endif
    }

    if ((ipc_notify_info->create_fd_num == 0) || (atomic_read(&notify_node->open_info[0].ref) == 0)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("destroy notify node fail.(%u %d)\n", ipc_notify_info->create_fd_num,
            atomic_read(&notify_node->open_info[0].ref));
        mutex_unlock(&ipc_notify_info->info_mutex);
        return -EFAULT;
#endif
    }

    ipc_notify_info->create_fd_num--;
    mutex_lock(&notify_node->mutex);
    list_del(&notify_node->create_list_node);
    atomic_dec(&notify_node->open_info[0].ref);
    notify_node_del = devdrv_manager_notify_should_del(notify_node);
    devdrv_manager_ipc_notify_free(notify_node);
    mutex_unlock(&notify_node->mutex);

    if (notify_node_del == true) {
        ret = devdrv_ipc_del(notify_node->name, DEVDRV_IPC_NAME_SIZE, DEVDRV_IPC_NOTIFY);
        if (ret != 0) {
            TSDRV_PRINT_ERR("ipc del failed, dev_id(%u)\n", notify_node->dev_id);
        }
    }
    mutex_unlock(&ipc_notify_info->info_mutex);
#endif
    return 0;
}

/* the user passed the physical id */
int devdrv_manager_ipc_notify_close(void *c, void *n)
{
    struct devdrv_manager_context *context = (struct devdrv_manager_context *)c;
    struct devdrv_notify_ioctl_info *notify_ioctl_info  = (struct devdrv_notify_ioctl_info *)n;
    struct ipc_notify_info *ipc_notify_info = context->ipc_notify_info;
    struct ipc_notify_node *notify_node = NULL;
    bool notify_node_del;
    int pid_index, ret;

    mutex_lock(&ipc_notify_info->info_mutex);
    notify_node = devdrv_manager_find_notify_node(ipc_notify_info, notify_ioctl_info);
    if (notify_node == NULL) {
        mutex_unlock(&ipc_notify_info->info_mutex);
        TSDRV_PRINT_ERR("find notify node failed\n");
        return -EFAULT;
    }

    pid_index = devdrv_manager_check_pid(notify_node, devdrv_manager_get_context_start_time(context), current->tgid);
    if (pid_index == DEVDRV_PID_MAX_NUM || pid_index == 0) {
#ifndef TSDRV_UT
        devdrv_manager_ipc_notify_pid_show(notify_node);
        mutex_unlock(&ipc_notify_info->info_mutex);
        TSDRV_PRINT_ERR("current tgid has no permission, pid_index(%d), start_time(%llu), notifyid(%d), devid(%u)\n",
            pid_index, devdrv_manager_get_context_start_time(context), notify_node->notify_id, notify_node->dev_id);
        return -EFAULT;
#endif
    }

    if (atomic_read(&notify_node->open_info[pid_index].ref) > (int)DEVDRV_IPC_REF_INIT) {
#ifndef TSDRV_UT
        TSDRV_PRINT_DEBUG("multi close notify: index(%d),notify_id(%u),ref(%d).\n",
            pid_index, notify_node->notify_id, atomic_read(&notify_node->open_info[pid_index].ref));

        atomic_dec(&notify_node->open_info[pid_index].ref);
        mutex_unlock(&ipc_notify_info->info_mutex);
        return 0;
#endif
    }

    if ((ipc_notify_info->open_fd_num == 0) || (atomic_read(&notify_node->open_info[pid_index].ref) == 0)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("close notify fail.(%u %d)\n", ipc_notify_info->open_fd_num,
            atomic_read(&notify_node->open_info[pid_index].ref));
        mutex_unlock(&ipc_notify_info->info_mutex);
        return -EFAULT;
#endif
    }
    TSDRV_PRINT_DEBUG("valid close notify: index(%d),notify_id(%u),ref(%d).\n",
        pid_index, notify_node->notify_id, atomic_read(&notify_node->open_info[pid_index].ref));

    mutex_lock(&notify_node->mutex);
    list_del(&notify_node->open_info[pid_index].open_list_node);

    ipc_notify_info->open_fd_num--;
    atomic_dec(&notify_node->open_info[pid_index].ref);
    notify_node_del = devdrv_manager_notify_should_del(notify_node);
    devdrv_manager_ipc_notify_free(notify_node);
    mutex_unlock(&notify_node->mutex);
    if (notify_node_del == true) {
#ifndef TSDRV_UT
        ret = devdrv_ipc_del(notify_node->name, DEVDRV_IPC_NAME_SIZE, DEVDRV_IPC_NOTIFY);
        if (ret != 0) {
            TSDRV_PRINT_ERR("ipc del failed, dev_id(%u)\n", notify_node->dev_id);
        }
#endif
    }
    mutex_unlock(&ipc_notify_info->info_mutex);

    return 0;
}

static inline void devdrv_manager_ipc_notify_leak_print(struct devdrv_manager_context *context)
{
    struct ipc_notify_info *ipc_notify_info = context->ipc_notify_info;

    TSDRV_PRINT_WARN("Process Name: %s  "
                     "PID: %d  "
                     "TGID: %d\n",
                     context->task->comm, context->pid, context->tgid);

    TSDRV_PRINT_WARN("ipc_notify_info->create_fd_num = %d, "
                     "ipc_notify_info->open_fd_num = %d\n",
                     ipc_notify_info->create_fd_num, ipc_notify_info->open_fd_num);
}

STATIC void devdrv_manager_ipc_notify_recycle_opened(struct devdrv_manager_context *context,
                                                     struct ipc_notify_info *ipc_notify_info)
{
#ifndef TSDRV_UT
    struct ipc_open_info *open_info = NULL;
    struct ipc_notify_node *notify_node = NULL;
    struct list_head *pos = NULL, *n = NULL;

    mutex_lock(&ipc_notify_info->info_mutex);
    if (!list_empty_careful(&ipc_notify_info->open_list_head)) {
        list_for_each_safe(pos, n, &ipc_notify_info->open_list_head) {
            bool node_del;
            int pid_index;

            open_info = list_entry(pos, struct ipc_open_info, open_list_node);
            ipc_notify_info->open_fd_num--;
            notify_node = open_info->node;
            mutex_lock(&notify_node->mutex);
            devdrv_manager_ipc_notify_free(notify_node);

            pid_index = devdrv_manager_match_pid(notify_node, context->tgid);
            if (pid_index == DEVDRV_PID_MAX_NUM) {
                mutex_unlock(&notify_node->mutex);
                TSDRV_PRINT_ERR("can't find valid pid(%d).\n", context->tgid);
                continue;
            }
            atomic_set(&notify_node->open_info[pid_index].ref, 0);
            node_del = devdrv_manager_notify_should_del(notify_node);
            mutex_unlock(&notify_node->mutex);
            if (node_del && (devdrv_ipc_del(notify_node->name, DEVDRV_IPC_NAME_SIZE, DEVDRV_IPC_NOTIFY) != 0)) {
                TSDRV_PRINT_ERR("ipc del failed, dev_id(%u)\n", notify_node->dev_id);
            }
        }
    }
    TSDRV_PRINT_INFO("finish recycle opened ipc node, open_fd_num(%u)\n", ipc_notify_info->open_fd_num);
    mutex_unlock(&ipc_notify_info->info_mutex);
#endif
}

STATIC void devdrv_manager_ipc_notify_recycle_created(struct devdrv_manager_context *context,
                                                      struct ipc_notify_info *ipc_notify_info)
{
#ifndef TSDRV_UT
    struct ipc_notify_node *notify_node = NULL;
    struct list_head *pos = NULL, *n = NULL;
    int pid_index;

    TSDRV_PRINT_INFO("start recycle created ipc node, create_fd_num(%u)\n", ipc_notify_info->create_fd_num);

    mutex_lock(&ipc_notify_info->info_mutex);
    if (!list_empty_careful(&ipc_notify_info->create_list_head)) {
        list_for_each_safe(pos, n, &ipc_notify_info->create_list_head) {
            notify_node = list_entry(pos, struct ipc_notify_node, create_list_node);
            ipc_notify_info->create_fd_num--;
            pid_index = devdrv_manager_match_pid(notify_node, context->tgid);
            if (pid_index != DEVDRV_PID_MAX_NUM) {
                bool node_del;

                mutex_lock(&notify_node->mutex);
                atomic_dec(&notify_node->open_info[pid_index].ref);
                devdrv_manager_ipc_notify_free(notify_node);
                node_del = devdrv_manager_notify_should_del(notify_node);
                mutex_unlock(&notify_node->mutex);
                if (node_del && (devdrv_ipc_del(notify_node->name, DEVDRV_IPC_NAME_SIZE, DEVDRV_IPC_NOTIFY) != 0)) {
                    TSDRV_PRINT_ERR("ipc del failed, dev_id(%u)\n", notify_node->dev_id);
                }
            }
        }
    }
    TSDRV_PRINT_INFO("finish recycle created ipc node, create_fd_num(%u)\n", ipc_notify_info->create_fd_num);
    mutex_unlock(&ipc_notify_info->info_mutex);
#endif
}

STATIC void devdrv_manager_ipc_notify_recycle_work(struct devdrv_manager_context *context,
                                                   struct ipc_notify_info *ipc_notify_info)
{
#ifndef TSDRV_UT
    devdrv_manager_ipc_notify_recycle_opened(context, ipc_notify_info);
    devdrv_manager_ipc_notify_recycle_created(context, ipc_notify_info);
#endif
}

void devdrv_manager_ipc_notify_release_recycle(void *ctx)
{
#ifndef TSDRV_UT
    struct devdrv_manager_context *context = (struct devdrv_manager_context *)ctx;
    struct ipc_notify_info *ipc_notify_info = NULL;

    if ((context == NULL) || (context->ipc_notify_info == NULL)) {
        TSDRV_PRINT_ERR("context(%pK) or context->ipc_notify_info is NULL\n", context);
        return;
    }
    ipc_notify_info = context->ipc_notify_info;

    if ((ipc_notify_info->create_fd_num > 0) || (ipc_notify_info->open_fd_num > 0)) {
        devdrv_manager_ipc_notify_leak_print(context);
        devdrv_manager_ipc_notify_recycle_work(context, ipc_notify_info);
    }
#endif
}
#endif

