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
* Create: 2023-3-10
*/

#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/notifier.h>
#include <linux/profile.h>
#include <linux/list.h>
#include <linux/uidgid.h>
#include <linux/pid.h>
#include <linux/capability.h>
#include <linux/sched.h>
#include <linux/nsproxy.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
#include <linux/sched/signal.h>
#include <linux/sched/task.h>
#endif
#include <linux/cred.h>
#include <linux/delay.h>
#include <linux/cpumask.h>
#include <linux/device_cgroup.h>

#include "davinci_api.h"

#include "uda_notifier.h"
#include "uda_dev.h"

#define UDA_NS_NUM 128
#define UDA_HOST_ROOT_TGID 1

#define FREE_NS_NODE_TRY_COUNT 20
#define FREE_NS_NODE_INTERVAL_MS 50

struct uda_dev_shared_check {
    u32 udevid;
    u32 share_occupied;
};

static u32 uda_detected_dev_num = 0;
static dev_t uda_dev;
static struct class *uda_class;

static struct work_struct host_ns_init_work;
static struct mnt_namespace *host_ns = NULL;

static struct mutex access_ns_mutex;
struct list_head access_ns_head;

static u32 uda_ns_id[UDA_NS_NUM];

static kuid_t g_phy_dev_uid = GLOBAL_ROOT_UID;
static kgid_t g_phy_dev_gid = GLOBAL_ROOT_GID;
static umode_t g_phy_dev_mode = DEV_MODE_PERMISSION;

static u8 g_dev_shared[UDA_MAX_PHY_DEV_NUM];

void uda_init_udev_ns_id(u32 udevid, struct uda_access *access, struct mnt_namespace *ns, u32 ns_id);
int uda_occupy_dev_by_ns(u32 udevid, struct uda_access *access, struct mnt_namespace *ns);
void uda_unoccupy_dev_by_ns(u32 udevid, struct uda_access *access, struct mnt_namespace *ns);
bool uda_is_ns_in_udev_shared_list(struct mnt_namespace *ns, struct uda_access *access);
bool uda_tgid_exited(u32 tgid, TASK_TIME_TYPE start_time);
void uda_ns_node_destroy_work(struct work_struct *p_work);
u32 uda_get_ns_pid_count(struct mnt_namespace *ns);
static void uda_destroy_ns_node(struct uda_ns_node *ns_node, bool from_work);

static u32 uda_alloc_ns_id(void)
{
    u32 i;

    for (i = 0; i < UDA_NS_NUM; i++) {
        if (uda_ns_id[i] == 0) {
            uda_ns_id[i] = 1;
            return i;
        }
    }

    return UDA_NS_NUM;
}

static void uda_free_ns_id(u32 ns_id)
{
    uda_ns_id[ns_id] = 0;
}

static struct mnt_namespace *uda_get_task_ns(struct task_struct *task)
{
    return (task->nsproxy != NULL) ? task->nsproxy->mnt_ns : NULL;
}

static struct mnt_namespace *uda_get_current_ns(void)
{
    return uda_get_task_ns(current);
}

static struct mnt_namespace *uda_get_host_ns(void)
{
    return host_ns;
}

void uda_host_ns_init_work(struct work_struct *work)
{
    host_ns = uda_get_current_ns(); /* in some os(centos), init_task mnt_ns may be null, so use kernel work ns */
    uda_info("Ns info. (ns=%pK; init.task ns=%pK)\n", host_ns, init_task.nsproxy->mnt_ns);
}

static bool uda_is_admin_task(struct task_struct *task)
{
    const struct cred *cred = NULL;
    bool is_admin;

    rcu_read_lock();
    cred = __task_cred(task); //lint !e1058 !e64 !e666
    is_admin = uid_eq(cred->uid, GLOBAL_ROOT_UID) &&
        cap_issubset(CAP_FULL_SET, cred->cap_effective);
    rcu_read_unlock();

    return is_admin;
}

bool uda_cur_is_host(void)
{
    return (uda_get_current_ns() == uda_get_host_ns());
}

bool uda_cur_is_admin(void)
{
    return (uda_cur_is_host() || (uda_is_admin_task(current)));
}

static bool uda_devcgroup_permission_allow(struct uda_dev_inst *dev_inst)
{
    /* devcgroup_check_permission is implemented in the kernel of 5.5, so use kernel open replace it,
       filp_open also check file user, device multi-user configuration can not use */
#ifdef DRV_HOST
    struct file *filp = filp_open(dev_inst->access.name, O_WRONLY, 0);
    if (IS_ERR_OR_NULL(filp)) {
        return false;
    }

    filp_close(filp, NULL);
    return true;
#else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 5, 0)
    dev_t devno = dev_inst->access.devno;
    return (devcgroup_check_permission(DEVCG_DEV_CHAR, MAJOR(devno), MINOR(devno), DEVCG_ACC_WRITE) == 0);
#else
    return true;
#endif
#endif
}

static void uda_init_ns_node_dev(struct uda_ns_node *ns_node, u32 root_tgid, TASK_TIME_TYPE tgid_time)
{
    u32 udevid;
    u32 max_num = uda_cur_is_admin() ? UDA_MAX_PHY_DEV_NUM : UDA_UDEV_MAX_NUM;

    max_num = (uda_cur_is_host() && (root_tgid != UDA_HOST_ROOT_TGID)) ? UDA_UDEV_MAX_NUM : max_num;

    ns_node->root_tgid = root_tgid;
    ns_node->tgid_time = tgid_time;
    ns_node->dev_num = 0;
    for (udevid = 0; udevid < max_num; udevid++) {
        struct uda_dev_inst *dev_inst = NULL;

        if (!uda_can_access_udevid(udevid)) {
            continue;
        }

        dev_inst = uda_dev_inst_get(udevid);
        if (dev_inst == NULL) {
            uda_err("Should get dev inst. (udevid=%u)", udevid);
            continue;
        }

        if (uda_cur_is_host()) {
            if (!uda_devcgroup_permission_allow(dev_inst)) {
                uda_dev_inst_put(dev_inst);
                continue;
            }
        }

        if (ns_node->dev_num >= UDA_DEV_MAX_NUM) {
            uda_warn("Dev num in ns is overflow. (ns_id=%u; ns=%pK; dev_num=%u)",
                ns_node->ns_id, ns_node->ns, ns_node->dev_num);
            uda_dev_inst_put(dev_inst);
            break;
        }

        ns_node->devid_to_udevid[ns_node->dev_num] = udevid;
        ns_node->dev_num++;
        uda_init_udev_ns_id(udevid, &dev_inst->access, ns_node->ns, ns_node->ns_id);
        (void)uda_notifier_call(udevid, &dev_inst->type, UDA_OCCUPY);
        uda_dev_inst_put(dev_inst);
    }
}

static void uda_uninit_ns_node_dev(struct uda_ns_node *ns_node)
{
    u32 devid;

    if (ns_node->ns_id >= UDA_NS_NUM) { /* admin not set namespace */
        return;
    }

    for (devid = 0; devid < ns_node->dev_num; devid++) {
        u32 udevid = ns_node->devid_to_udevid[devid];
        struct uda_dev_inst *dev_inst = uda_dev_inst_get(udevid);
        if (dev_inst != NULL) {
            (void)uda_notifier_call(udevid, &dev_inst->type, UDA_UNOCCUPY);
            uda_unoccupy_dev_by_ns(udevid, &dev_inst->access, ns_node->ns);
            uda_dev_inst_put(dev_inst);
        }
    }
}

void uda_for_each_ns_node_safe(void *priv, void (*func)(struct uda_ns_node *ns_node, void *priv))
{
    struct uda_ns_node *ns_node = NULL, *tmp_node = NULL;

    mutex_lock(&access_ns_mutex);
    list_for_each_entry_safe(ns_node, tmp_node, &access_ns_head, node) {
        func(ns_node, priv);
    }
    mutex_unlock(&access_ns_mutex);
}

static struct uda_ns_node *uda_find_ns_node(struct mnt_namespace *ns, u32 root_tgid)
{
    struct uda_ns_node *ns_node = NULL, *first_ns_node = NULL;

    list_for_each_entry(ns_node, &access_ns_head, node) {
        if (ns_node->ns == ns) {
            /* Processes added to the dev cgroup create independent ns nodes. Therefore, exact matching is preferred */
            if (ns_node->root_tgid == root_tgid) {
                return ns_node;
            }

            if (first_ns_node == NULL) {
                first_ns_node = ns_node;
            }
        }
    }

    if (first_ns_node == NULL) {
        return NULL;
    }

    return ((ns == uda_get_host_ns()) && (first_ns_node->root_tgid != UDA_HOST_ROOT_TGID)) ? NULL : first_ns_node;
}

static bool uda_is_admin_ns_node(struct uda_ns_node *ns_node)
{
    if (ns_node->ns_id < UDA_NS_NUM) { /* normal container ns node */
        return false;
    }

    if (ns_node->ns != uda_get_host_ns()) { /* admin container ns node */
        return true;
    }

    if (ns_node->root_tgid == UDA_HOST_ROOT_TGID) { /* host ns node */
        return true;
    }

    return false; /* host devgroup proc ns node */
}

int uda_set_dev_ns_identify(u32 udevid, u64 identify)
{
    struct uda_dev_inst *dev_inst = NULL;
    struct mnt_namespace *ns = NULL;
    struct uda_ns_node *ns_node = NULL;
    int ret = -EINVAL;

    if (!uda_can_access_udevid(udevid)) {
        uda_err("Can not access udev. (udevid=%u)\n", udevid);
        return -EFAULT;
    }

    dev_inst = uda_dev_inst_get(udevid);
    if (dev_inst == NULL) {
        uda_err("Invalid udev. (udevid=%u)\n", udevid);
        return -EINVAL;
    }

    ns = (dev_inst->access.ns != NULL) ? dev_inst->access.ns : uda_get_current_ns();
    ns_node = uda_find_ns_node(ns, current->tgid);
    if (ns_node != NULL) {
        ns_node->identify = identify;
        ret = 0;
    }
    uda_dev_inst_put(dev_inst);

    return ret;
}
EXPORT_SYMBOL(uda_set_dev_ns_identify);

int uda_get_dev_ns_identify(u32 udevid, u64 *identify)
{
    struct uda_dev_inst *dev_inst = NULL;
    int ret = -EINVAL;

    if (identify == NULL) {
        uda_err("Null ptr.\n");
        return -EINVAL;
    }

    dev_inst = uda_dev_inst_get(udevid);
    if (dev_inst == NULL) {
        uda_err("Invalid udev. (udevid=%u)\n", udevid);
        return -EINVAL;
    }

    if (dev_inst->access.ns != NULL) {
        struct uda_ns_node *ns_node = NULL;
        mutex_lock(&access_ns_mutex);
        ns_node = uda_find_ns_node(dev_inst->access.ns, current->tgid);
        if (ns_node != NULL) {
            *identify = ns_node->identify;
            ret = 0;
        }
        mutex_unlock(&access_ns_mutex);
    }

    uda_dev_inst_put(dev_inst);

    return ret;
}
EXPORT_SYMBOL(uda_get_dev_ns_identify);

int uda_get_cur_ns_id(u32 *ns_id)
{
    struct uda_ns_node *ns_node = NULL;
    int ret = -EINVAL;

    if (ns_id == NULL) {
        uda_err("Null ptr.\n");
        return -EINVAL;
    }

    mutex_lock(&access_ns_mutex);
    ns_node = uda_find_ns_node(uda_get_current_ns(), current->tgid);
    if (ns_node != NULL) {
        *ns_id = ns_node->ns_id;
        ret = 0;
    }
    mutex_unlock(&access_ns_mutex);

    return ret;
}
EXPORT_SYMBOL(uda_get_cur_ns_id);

static bool uda_dev_is_occupied(struct uda_access *access)
{
    struct uda_ns_node *ns_node = NULL;
    unsigned int i;

    if (access->ns == NULL) {
        return false;
    }
    ns_node = uda_find_ns_node(access->ns, current->tgid);
    if (ns_node == NULL) {
        uda_info("not found access ns ns_node.(name=%s)\n", access->name);
        return false;
    }
    if (!uda_tgid_exited(ns_node->root_tgid, ns_node->tgid_time)) {
        return true;
    }
    /* pre container none root tgid would exit after root tgid, wait then all exited */
    for (i = 0; i < FREE_NS_NODE_TRY_COUNT; i++) {
        if (uda_get_ns_pid_count(ns_node->ns) == 0) {
            break;
        }
        msleep(FREE_NS_NODE_INTERVAL_MS);
    }
    uda_info("old ns_node not used, free and reoccupy.(ns_id=%u; tgid=%u; i=%u)\n",
        ns_node->ns_id, ns_node->root_tgid, i);
    mutex_unlock(&access->mutex);
    uda_uninit_ns_node_dev(ns_node);
    mutex_lock(&access->mutex);
    uda_destroy_ns_node(ns_node, false);
    return false;
}

static struct uda_ns_node *uda_create_ns_node(struct mnt_namespace *ns)
{
    static u32 ns_id = UDA_NS_NUM;
    struct uda_ns_node *ns_node = kzalloc(sizeof(struct uda_ns_node), GFP_KERNEL | __GFP_ACCOUNT);
    if (ns_node == NULL) {
        uda_err("Out of memory.\n");
        return NULL;
    }

    ns_node->ns = ns;
    if (uda_cur_is_admin()) {
        ns_node->ns_id = ns_id++;
        if (ns_node->ns_id < UDA_NS_NUM) {
            ns_id = UDA_NS_NUM;
            ns_node->ns_id = UDA_NS_NUM;
        }
    } else {
        ns_node->ns_id = uda_alloc_ns_id();
        if (ns_node->ns_id == UDA_NS_NUM) {
            uda_err("Alloc ns id fail.\n");
            kfree(ns_node);
            return NULL;
        }
    }
    INIT_DELAYED_WORK(&ns_node->wait_destroy_work, uda_ns_node_destroy_work);
    list_add_tail(&ns_node->node, &access_ns_head);
    return ns_node;
}

static void uda_destroy_ns_node(struct uda_ns_node *ns_node, bool from_work)
{
    if (ns_node->ns_id < UDA_NS_NUM) {
        uda_free_ns_id(ns_node->ns_id);
    }
    if (!from_work) {
        (void)cancel_delayed_work_sync(&ns_node->wait_destroy_work);
    }
    list_del(&ns_node->node);
    kfree(ns_node);
}

int uda_ns_node_devid_to_udevid(u32 devid, u32 *udevid)
{
    struct uda_ns_node *ns_node = NULL;

    mutex_lock(&access_ns_mutex);
    ns_node = uda_find_ns_node(uda_get_current_ns(), current->tgid);
    if (ns_node == NULL) {
        mutex_unlock(&access_ns_mutex);
        return -EAGAIN;
    }

    if (devid >= ns_node->dev_num) {
        mutex_unlock(&access_ns_mutex);
        return -ENODEV;
    }

    *udevid = ns_node->devid_to_udevid[devid];
    mutex_unlock(&access_ns_mutex);
    return 0;
}
EXPORT_SYMBOL(uda_ns_node_devid_to_udevid);

static struct uda_dev_inst *uda_dev_inst_get_by_devid(u32 devid)
{
    u32 udevid = UDA_INVALID_UDEVID;

    if (devid < UDA_DEV_MAX_NUM) {
        int ret;

        ret = uda_ns_node_devid_to_udevid(devid, &udevid);
        if (ret == -EAGAIN) {
            /* Some commands are earlier than ns node create */
            (void)uda_setup_ns_node(UDA_MAX_PHY_DEV_NUM);
            if (uda_cur_is_admin()) {
                ret = uda_ns_node_devid_to_udevid(devid, &udevid);
            }
        }

        if (ret != 0) {
            return NULL;
        }
    } else if (devid >= UDA_MIA_UDEV_OFFSET) {
        if (uda_cur_is_admin()) {
            /* logic devid equal to udevid when devid is bigger than UDA_DEV_MAX_NUM(mia dev) */
            udevid = devid;
        }
    }

    return uda_dev_inst_get(udevid);
}

int uda_devid_to_udevid(u32 devid, u32 *udevid)
{
    struct uda_dev_inst *dev_inst = NULL;

    if (udevid == NULL) {
        return -EINVAL;
    }

    dev_inst = uda_dev_inst_get_by_devid(devid);
    if (dev_inst == NULL) {
        return -EINVAL;
    }

    *udevid = dev_inst->udevid;
    uda_dev_inst_put(dev_inst);
    return 0;
}
EXPORT_SYMBOL(uda_devid_to_udevid);

/* for compatible, obp return phy_id, milan return udevid */
int uda_devid_to_phy_devid(u32 devid, u32 *phy_devid, u32 *vfid)
{
    u32 udevid;
    int ret = uda_devid_to_udevid(devid, &udevid);
    if (ret != 0) {
        uda_err("Trans failed. (devid=%u)\n", devid);
        return ret;
    }

    if ((phy_devid == NULL) || (vfid == NULL)) {
        return -EINVAL;
    }

    if (uda_is_support_udev_mng()) {
        *phy_devid = udevid;
        *vfid = 0;
    } else {
        if (uda_is_phy_dev(udevid)) {
            *phy_devid = udevid;
            *vfid = 0;
        } else {
            struct uda_mia_dev_para mia_para;
            ret = uda_udevid_to_mia_devid(udevid, &mia_para);
            if (ret != 0) {
                uda_err("Query mia dev failed. (devid=%u; udevid=%u)\n", devid, udevid);
                return ret;
            }

            *phy_devid = mia_para.phy_devid;
            *vfid = mia_para.sub_devid + 1; /* vfid start from 1 */
        }
    }

    return 0;
}
EXPORT_SYMBOL(uda_devid_to_phy_devid);

int uda_udevid_to_devid(u32 udevid, u32 *devid)
{
    struct uda_dev_inst *dev_inst = NULL;

    /* only surport host query mia dev, in other scenarios, the query is performed in user mode. */
    if ((devid == NULL) || uda_is_phy_dev(udevid)) {
        uda_err("Invalid udevid or null ptr. (udevid=%u)\n", udevid);
        return -EINVAL;
    }

    dev_inst = uda_dev_inst_get(udevid);
    if (dev_inst == NULL) {
        uda_err("Invalid udevid. (udevid=%u)\n", udevid);
        return -EINVAL;
    }

    *devid = udevid; /* logic devid equal to udevid when devid is bigger than UDA_DEV_MAX_NUM(mia dev) */
    uda_dev_inst_put(dev_inst);
    return 0;
}

bool uda_task_can_access_udevid(struct task_struct *task, u32 udevid)
{
    bool ret = false;
    struct mnt_namespace *ns = uda_get_task_ns(task);
    struct uda_dev_inst *dev_inst = uda_dev_inst_get(udevid);
    if (dev_inst != NULL) {
        struct uda_access *access = &dev_inst->access;
        mutex_lock(&access->mutex);
        if ((ns == uda_get_host_ns()) || uda_is_admin_task(task)
            || (ns == dev_inst->access.ns) || uda_is_ns_in_udev_shared_list(ns, access)) {
            ret = true;
        }
        mutex_unlock(&access->mutex);
        uda_dev_inst_put(dev_inst);
    }

    return ret;
}

bool uda_proc_can_access_udevid(int tgid, u32 udevid)
{
    struct task_struct *task = NULL;
    struct pid *pid = NULL;
    bool ret;

    pid = find_get_pid(tgid);
    if (pid == NULL) {
        uda_err("Find pid failed. (tgid=%d)\n", tgid);
        return false;
    }

    task = get_pid_task(pid, PIDTYPE_PID);
    if (task == NULL) {
        put_pid(pid);
        uda_err("Get task failed. (tgid=%d)\n", tgid);
        return false;
    }

    ret = uda_task_can_access_udevid(task, udevid);
    put_task_struct(task);
    put_pid(pid);

    return ret;
}
EXPORT_SYMBOL(uda_proc_can_access_udevid);

bool uda_can_access_udevid(u32 udevid)
{
    return uda_task_can_access_udevid(current, udevid);
}
EXPORT_SYMBOL(uda_can_access_udevid);

/* if udevid belong to task namespace or it's split davinci belong to, both return true(can access) */
bool uda_task_can_access_udevid_inherit(struct task_struct *task, u32 udevid)
{
    struct uda_ns_node *ns_node = NULL;
    u32 devid;
    u32 tmp_udevid;
    struct uda_mia_dev_para mia_para = {0};
    bool can_access = false;

    if (task == NULL) {
        return false;
    }
    mutex_lock(&access_ns_mutex);
    ns_node = uda_find_ns_node(uda_get_task_ns(task), task->tgid);
    if (ns_node != NULL) {
        for (devid = 0; devid < ns_node->dev_num; devid++) {
            tmp_udevid = ns_node->devid_to_udevid[devid];
            if (tmp_udevid == udevid) {
                can_access = true;
                break;
            }
            if (uda_is_phy_dev(tmp_udevid)) {
                continue;
            }
            if ((uda_udevid_to_mia_devid(tmp_udevid, &mia_para) == 0) && (mia_para.phy_devid == udevid)) {
                can_access = true;
                break;
            }
        }
    }
    mutex_unlock(&access_ns_mutex);
    return can_access;
}
EXPORT_SYMBOL(uda_task_can_access_udevid_inherit);

static u32 uda_get_ns_node_devid_from_udevid(struct uda_ns_node *ns_node, u32 udevid)
{
    u32 devid;

    for (devid = 0; devid < ns_node->dev_num; devid++) {
        if (udevid == ns_node->devid_to_udevid[devid]) {
            return devid;
        }
    }

    return UDA_DEV_MAX_NUM;
}

static void uda_dev_restore_ns(struct uda_ns_node *ns_node, void *priv)
{
    struct uda_access *access = (struct uda_access *)priv;
    u32 udevid = MINOR(access->devno);

    /* only check normal container, if namespace have been restored, not try again */
    if ((ns_node->ns_id >= UDA_NS_NUM) || (access->ns != NULL)) {
        return;
    }

    if (uda_get_ns_node_devid_from_udevid(ns_node, udevid) < UDA_DEV_MAX_NUM) {
        mutex_lock(&access->mutex);
        (void)uda_occupy_dev_by_ns(udevid, access, ns_node->ns);
        mutex_unlock(&access->mutex);
        uda_init_udev_ns_id(udevid, access, ns_node->ns, ns_node->ns_id);
        uda_info("Restore namespace. (udevid=%u; ns_id=%u)\n", udevid, ns_node->ns_id);
    }
}

static void uda_try_to_restore_ns(struct uda_access *access)
{
    uda_for_each_ns_node_safe((void *)access, uda_dev_restore_ns);
}

static void uda_add_udev_to_admin_ns_node(struct uda_ns_node *ns_node, void *priv)
{
    struct uda_access *access = (struct uda_access *)priv;
    u32 udevid = MINOR(access->devno);

    /* only check admin ns node */
    if (!uda_is_admin_ns_node(ns_node)) {
        return;
    }

    /* hotreset not del udevid from namespace */
    if (uda_get_ns_node_devid_from_udevid(ns_node, udevid) < UDA_DEV_MAX_NUM) {
        return;
    }

    ns_node->devid_to_udevid[ns_node->dev_num++] = udevid;
    uda_info("Add to namespace. (udevid=%u; ns_id=%u)\n", udevid, ns_node->ns_id);
}

static void uda_try_to_add_udev_to_ns_node(struct uda_access *access)
{
    uda_for_each_ns_node_safe((void *)access, uda_add_udev_to_admin_ns_node);
}

static void uda_remove_udev_from_admin_ns_node(struct uda_ns_node *ns_node, void *priv)
{
    struct uda_access *access = (struct uda_access *)priv;
    u32 udevid = MINOR(access->devno);
    u32 devid;

    /* only check admin ns node */
    if (!uda_is_admin_ns_node(ns_node)) {
        return;
    }

    devid = uda_get_ns_node_devid_from_udevid(ns_node, udevid);
    if (devid < UDA_DEV_MAX_NUM) {
        uda_info("Remove from namespace. (udevid=%u; ns_id=%u)\n", udevid, ns_node->ns_id);
        ns_node->dev_num--;
    }

    for (; devid < ns_node->dev_num; devid++) {
        ns_node->devid_to_udevid[devid] = ns_node->devid_to_udevid[devid + 1];
    }
}

static void uda_try_to_remove_udev_from_ns_node(struct uda_access *access)
{
    uda_for_each_ns_node_safe((void *)access, uda_remove_udev_from_admin_ns_node);
}

static struct uda_access_share_node *uda_find_udev_shared_node(struct mnt_namespace *ns, struct uda_access *access)
{
    struct uda_access_share_node *share_node = NULL;

    list_for_each_entry(share_node, &access->share_head, node) {
        if (share_node->ns == ns) {
            return share_node;
        }
    }

    return NULL;
}

bool uda_is_ns_in_udev_shared_list(struct mnt_namespace *ns, struct uda_access *access)
{
    return (uda_find_udev_shared_node(ns, access) != NULL);
}

void uda_init_udev_ns_id(u32 udevid, struct uda_access *access, struct mnt_namespace *ns, u32 ns_id)
{
    mutex_lock(&access->mutex);
    if (uda_is_dev_shared(udevid)) {
        struct uda_access_share_node *share_node = uda_find_udev_shared_node(ns, access);
        if (share_node != NULL) {
            share_node->ns_id = ns_id;
        }
    } else {
        access->ns_id = ns_id;
    }
    mutex_unlock(&access->mutex);
}

static int uda_flush_invalid_share_occupy(struct uda_access *access)
{
    unsigned int node_num = 0;
    struct list_head *pos;
    struct uda_access_share_node *share_node = NULL, *share_node_n = NULL;

    list_for_each(pos, &access->share_head) {
        node_num++;
    }
    if (node_num <= UDA_NS_NUM) {
        return 0;
    }
    uda_info("share node num exceed.(node_num=%u)\n", node_num);

    list_for_each_entry_safe(share_node, share_node_n, &access->share_head, node) {
        if (share_node->ns_id == UDA_NS_NUM) {
            uda_info("find free first invalid share node.\n");
            list_del(&share_node->node);
            vfree(share_node);
            return 0;
        }
    }
    uda_err("not found invalid share node when node num exceed.\n");
    return -ESRCH;
}

static int uda_occupy_shared_dev(u32 udevid, struct uda_access *access, struct mnt_namespace *ns)
{
    struct uda_access_share_node *share_node = NULL;
    int ret;

    if (uda_is_ns_in_udev_shared_list(ns, access)) {
        return 0;
    }
    ret = uda_flush_invalid_share_occupy(access);
    if (ret != 0) {
        return ret;
    }

    share_node = vmalloc(sizeof(struct uda_access_share_node));
    if (share_node == NULL) {
        uda_err("Alloc share node failed. (udevid=%u)\n", udevid);
        return -ENOMEM;
    }

    share_node->ns = ns;
    share_node->ns_id = UDA_NS_NUM;
    list_add_tail(&share_node->node, &access->share_head);
    return 0;
}

static void uda_unoccupy_shared_dev(struct uda_access *access, struct mnt_namespace *ns)
{
    struct uda_access_share_node *share_node = uda_find_udev_shared_node(ns, access);
    if (share_node != NULL) {
        list_del(&share_node->node);
        vfree(share_node);
    }
}

int uda_occupy_dev_by_ns(u32 udevid, struct uda_access *access, struct mnt_namespace *ns)
{
    if (uda_is_dev_shared(udevid)) {
        return uda_occupy_shared_dev(udevid, access, ns);
    }

    if (access->ns == NULL) {
        uda_info("Occupy udevid. (udevid=%u; ns_id=%u)\n", udevid, access->ns_id);
        access->ns = ns; /* set udevid used by this namespace */
        return 0;
    }

    /* Repeatedly occupied by processes in this namespace */
    if (access->ns == ns) {
        return 0;
    }

    /* The occupied namespace is not alive. Preempt the device. */
    if (!uda_dev_is_occupied(access)) {
        uda_info("Preempt udevid. (udevid=%u; old_ns=%pK; new_ns=%pK)\n", udevid, access->ns, ns);
        access->ns = ns;
        return 0;
    }

    uda_err("Conflict open udevid. (udevid=%u; ns_id=%u)\n", udevid, access->ns_id);
    return -EBUSY;
}

void uda_unoccupy_dev_by_ns(u32 udevid, struct uda_access *access, struct mnt_namespace *ns)
{
    mutex_lock(&access->mutex);
    if (uda_is_dev_shared(udevid)) {
        uda_unoccupy_shared_dev(access, ns);
    } else {
        access->ns = NULL;
        access->ns_id = 0;
    }
    mutex_unlock(&access->mutex);
}

static int uda_occupy_dev(struct uda_dev_inst *dev_inst)
{
    struct uda_access *access = &dev_inst->access;

    if (access->dev == NULL) {
        uda_err("Dev has been remove. (udevid=%u)\n", dev_inst->udevid);
        return -ENODEV;
    }

    return uda_occupy_dev_by_ns(dev_inst->udevid, access, uda_get_current_ns());
}

static void uda_refresh_dev_ownership(struct inode *inode)
{
    g_phy_dev_uid = inode->i_uid;
    g_phy_dev_gid = inode->i_gid;
    g_phy_dev_mode = inode->i_mode;
}

static int uda_access_open(struct inode *inode, struct file *file)
{
    u32 udevid = iminor(inode);
    int ret = 0;
    struct uda_dev_inst *dev_inst = uda_dev_inst_get(udevid);
    if (dev_inst == NULL) {
        uda_err("Invalid udevid. (udevid=%u)\n", udevid);
        return -ENODEV;
    }

    uda_debug("Cdev open. (udevid=%u; inst_id=%u)\n", udevid, dev_inst->inst_id);

    if (uda_is_phy_dev(udevid)) {
        uda_refresh_dev_ownership(inode);
    }

    if (!uda_cur_is_admin()) {
        struct uda_access *access = &dev_inst->access;
        mutex_lock(&access_ns_mutex);
        mutex_lock(&access->mutex);
        ret = uda_occupy_dev(dev_inst);
        mutex_unlock(&access->mutex);
        mutex_unlock(&access_ns_mutex);
    }

    uda_dev_inst_put(dev_inst);

    return ret;
}

/* when all process in namespace exit, clear udevid ns, then the udevid can be used by another namespace */
static int uda_access_release(struct inode *inode, struct file *file)
{
    u32 udevid = iminor(inode);
    uda_debug("Cdev release. (udevid=%u)\n", udevid);
    return 0;
}

static const struct file_operations uda_access_fops = {
    .owner = THIS_MODULE,
    .open = uda_access_open,
    .release = uda_access_release,
};

static char *uda_devnode(const struct device *dev, umode_t *mode, kuid_t *uid, kgid_t *gid)
{
    if (dev != NULL) {
        u32 udevid = MINOR(dev->devt);

        if (mode != NULL) {
            if (!uda_is_phy_dev(udevid)) {
                *mode = DEV_MODE_PERMISSION;
            } else {
                *mode = g_phy_dev_mode;
            }
        }

        if ((uid != NULL) && (gid != NULL)) {
            *uid = g_phy_dev_uid;
            *gid = g_phy_dev_gid;
        }
    }

    return NULL;
}

const struct device_type uda_type = {
    .devnode = uda_devnode,
};

static void uda_device_release(struct device *dev)
{
    uda_debug("Free dev mem. (udevid=%u)\n", MINOR(dev->devt));
    kfree(dev);
}

static void uda_cdev_set_parent(struct cdev *cdev, struct kobject *kobj)
{
    cdev->kobj.parent = kobj;
}

/* old kernel has no cdev_device_add. cdev_device_add has a bug: if device_add failed, cdev will be double free */
static int uda_cdev_device_add(struct cdev *cdev, struct device *dev)
{
    int ret;

    uda_cdev_set_parent(cdev, &dev->kobj);

    ret = cdev_add(cdev, dev->devt, 1);
    if (ret != 0) {
        uda_err("Cdev add failed. (ret=%d)\n", ret);
        return ret;
    }

    return device_add(dev); /* cdev_add and cdev_alloc are Rollback by cdev_del */
}

static void uda_cdev_device_del(struct cdev *cdev, struct device *dev)
{
    device_del(dev);
    cdev_del(cdev);
}

static int uda_create_cdev(u32 udevid, struct uda_access *access)
{
    const char *name = uda_is_phy_dev(udevid) ? UDA_PHY_DEV_NAME : UDA_MIA_DEV_NAME;
    struct device *dev = NULL;
    int ret;

    (void)sprintf_s(access->name, UDA_DEV_NAME_LEN, "/dev/%s%u", name, udevid);
    access->devno = MKDEV(MAJOR(uda_dev), udevid);
    access->ns = NULL;
    access->ns_id = 0;
    INIT_LIST_HEAD(&access->share_head);
    mutex_init(&access->mutex);
    uda_try_to_restore_ns(access); /* container hotreset device, should restore it`s namespace */

    /* init cdev */
    access->cdev = cdev_alloc();
    if (access->cdev == NULL) {
        uda_err("Alloc cdev failed. (udevid=%u)\n", udevid);
        return -ENOMEM;
    }
    access->cdev->owner = THIS_MODULE;
    access->cdev->ops = &uda_access_fops;

    /* init dev */
    dev = kzalloc(sizeof(*dev), GFP_KERNEL);
    if (dev == NULL) {
        cdev_del(access->cdev);
        uda_err("Alloc dev failed. (udevid=%u)\n", udevid);
        return -ENOMEM;
    }
    device_initialize(dev);
    dev->devt = access->devno;
    dev->class = uda_class;
    dev->type = &uda_type;
    dev->release = uda_device_release;
    (void)kobject_set_name(&dev->kobj, "%s%u", name, udevid);
    access->dev = dev;

    /* attach device to cdev */
    ret = uda_cdev_device_add(access->cdev, access->dev);
    if (ret != 0) {
        cdev_del(access->cdev);
        kfree(access->dev);
        uda_err("Cdev add device failed. (udevid=%u; ret=%d)\n", udevid, ret);
        return ret;
    }

    if (uda_is_phy_dev(udevid)) {
        uda_try_to_add_udev_to_ns_node(access);
    }

    return 0;
}

static void uda_remove_cdev(u32 udevid, struct uda_access *access)
{
    uda_cdev_device_del(access->cdev, access->dev);
    put_device(access->dev);
}
static void uda_remove_share_list(struct uda_access *access)
{
    struct uda_access_share_node *share_node = NULL, *share_node_n = NULL;

    list_for_each_entry_safe(share_node, share_node_n, &access->share_head, node) {
        list_del(&share_node->node);
        vfree(share_node);
    }
}

int uda_access_add_dev(u32 udevid, struct uda_access *access)
{
    return uda_create_cdev(udevid, access);
}

/* hotrest device is force remove mode */
int uda_access_remove_dev(u32 udevid, struct uda_access *access, bool is_force)
{
    mutex_lock(&access->mutex);
    uda_remove_cdev(udevid, access);
    uda_remove_share_list(access);
    mutex_unlock(&access->mutex);

    if (!is_force) {
        uda_try_to_remove_udev_from_ns_node(access);
    }

    return 0;
}

static void uda_recycle_all_cdev(void)
{
    u32 udevid;

    for (udevid = 0; udevid < UDA_UDEV_MAX_NUM; udevid++) {
        struct uda_dev_inst *dev_inst = uda_dev_inst_get(udevid);
        if (dev_inst != NULL) {
            (void)uda_access_remove_dev(udevid, &dev_inst->access, true);
            uda_dev_inst_put(dev_inst);
        }
    }
}

static int uda_cdev_class_init(void)
{
#ifdef CFG_FEATURE_DEVID_TRANS
    int ret;

    ret = alloc_chrdev_region(&uda_dev, 0, UDA_UDEV_MAX_NUM, "devdrv-cdev");
    if (ret) {
        uda_err("Alloc chrdev region failed. (ret=%d)\n", ret);
        return ret;
    }

    uda_class = class_create("devdrv-class");
    if (IS_ERR(uda_class)) {
        unregister_chrdev_region(uda_dev, UDA_UDEV_MAX_NUM);
        uda_err("Create class failed.\n");
        return -EFAULT;
    }
#endif
    return 0;
}

static void uda_cdev_class_uninit(void)
{
#ifdef CFG_FEATURE_DEVID_TRANS
    class_destroy(uda_class);
    unregister_chrdev_region(uda_dev, UDA_UDEV_MAX_NUM);
#endif
}

u32 uda_get_cur_ns_dev_num(void)
{
    struct uda_ns_node *ns_node = NULL;
    u32 dev_num = 0;

    mutex_lock(&access_ns_mutex);
    ns_node = uda_find_ns_node(uda_get_current_ns(), current->tgid);
    if (ns_node != NULL) {
        dev_num = ns_node->dev_num;
    }
    mutex_unlock(&access_ns_mutex);

    return dev_num;
}
EXPORT_SYMBOL(uda_get_cur_ns_dev_num);

int uda_get_cur_ns_udevids(u32 udevids[], u32 num)
{
    struct uda_ns_node *ns_node = NULL;
    u32 devid;

    mutex_lock(&access_ns_mutex);
    ns_node = uda_find_ns_node(uda_get_current_ns(), current->tgid);
    if (ns_node != NULL) {
        for (devid = 0; (devid < ns_node->dev_num) && (devid < num); devid++) {
            udevids[devid] = ns_node->devid_to_udevid[devid];
        }
    }
    mutex_unlock(&access_ns_mutex);

    return 0;
}
EXPORT_SYMBOL(uda_get_cur_ns_udevids);

int uda_set_detected_phy_dev_num(u32 dev_num)
{
    if ((dev_num == 0) || (dev_num >= UDA_UDEV_MAX_NUM)) {
        uda_err("Invalid dev num. (dev_num=%u)\n", dev_num);
        return -EINVAL;
    }

    /* In a hybrid system, different probes set their detected devices. */
    uda_detected_dev_num = dev_num;
    uda_info("Set detected dev num. (total_dev_num=%u; dev_num=%u)\n", uda_detected_dev_num, dev_num);

    return 0;
}
EXPORT_SYMBOL(uda_set_detected_phy_dev_num);

u32 uda_get_detected_phy_dev_num(void)
{
    return uda_detected_dev_num;
}
EXPORT_SYMBOL(uda_get_detected_phy_dev_num);

bool uda_is_dev_shared(u32 udevid)
{
    if (udevid < sizeof(g_dev_shared)) {
        return (bool)g_dev_shared[udevid];
    }
    return false;
}

static void uda_dev_is_share_occupied(struct uda_ns_node *ns_node, void *priv)
{
    struct uda_dev_shared_check *check = (struct uda_dev_shared_check *)priv;

    if (ns_node->ns_id >= UDA_NS_NUM) {
        return;
    }

    if (uda_get_ns_node_devid_from_udevid(ns_node, check->udevid) < UDA_DEV_MAX_NUM) {
        check->share_occupied = true;
    }
}

int uda_set_dev_share(u32 udevid, u8 share_flag)
{
    struct uda_dev_inst *dev_inst = NULL;
    struct uda_dev_shared_check check = {udevid, false};

    if (share_flag != true && share_flag != false) {
        uda_err("invalid param.(share_flag=%u).\n", share_flag);
        return -EINVAL;
    }
    if (udevid >= sizeof(g_dev_shared)) {
        uda_err("invalid param.(udevid=%u).\n", udevid);
        return -EINVAL;
    }

    dev_inst = uda_dev_inst_get(udevid);
    if (dev_inst == NULL) {
        uda_err("Invalid udev. (udevid=%u)\n", udevid);
        return -EINVAL;
    }
    uda_dev_inst_put(dev_inst);
    if (uda_is_dev_shared(udevid) == share_flag) {
        return 0;
    }
    uda_for_each_ns_node_safe(&check, uda_dev_is_share_occupied);
    if (check.share_occupied) {
        uda_err("udevid is occupied in use, can not set share flag. (udevid=%u; share=%u)\n", udevid, share_flag);
        return -EBUSY;
    }
    uda_info("set dev share flag. (udevid=%u; share_flag=%u)\n", udevid, share_flag);
    g_dev_shared[udevid] = share_flag;
    return 0;
}
EXPORT_SYMBOL(uda_set_dev_share);

int uda_get_dev_share(u32 udevid, u8 *share_flag)
{
    struct uda_dev_inst *dev_inst = NULL;

    if (share_flag == NULL) {
        uda_err("Null ptr. (udevid=%u)\n", udevid);
        return -EINVAL;
    }

    dev_inst = uda_dev_inst_get(udevid);
    if (dev_inst == NULL) {
        uda_err("Invalid udev. (udevid=%u)\n", udevid);
        return -EINVAL;
    }
    *share_flag = uda_is_dev_shared(udevid);
    uda_dev_inst_put(dev_inst);
    return 0;
}
EXPORT_SYMBOL(uda_get_dev_share);

static void uda_wait_all_phy_startup(void)
{
    int wait_cnt = 0;

    /* wait all dev to be added */
    while (uda_get_dev_num() < uda_detected_dev_num) {
        ssleep(1);
        wait_cnt++;
        if (wait_cnt > 120) { /* max wait 120 seconds */
            uda_err("Wait timeout. (dev_num=%u; uda_detected_dev_num=%u)\n", uda_get_dev_num(), uda_detected_dev_num);
            break;
        }
    }
}

static int _uda_setup_ns_node(struct mnt_namespace *ns, u32 root_tgid, TASK_TIME_TYPE tgid_time, u32 dev_num)
{
    struct uda_ns_node *ns_node = NULL;

    ns_node = uda_find_ns_node(ns, root_tgid);
    if ((ns_node != NULL) && (ns_node->root_tgid == root_tgid)) {
        return 0;
    }

    ns_node = uda_create_ns_node(ns);
    if (ns_node == NULL) {
        return -ENOMEM;
    }

    uda_init_ns_node_dev(ns_node, root_tgid, tgid_time);
    if (!uda_cur_is_admin()) {
        if (dev_num > ns_node->dev_num) {
            uda_err("Dev num not match. (ns=%pK; dev_num=%u; node_dev_num=%u)\n", ns, dev_num, ns_node->dev_num);
            uda_uninit_ns_node_dev(ns_node);
            uda_destroy_ns_node(ns_node, false);
            return -EBUSY;
        } else if (dev_num < ns_node->dev_num) {
            uda_warn("More device setup. (ns=%pK; dev_num=%u; node_dev_num=%u)\n", ns, dev_num, ns_node->dev_num);
        }
    } else if (ns_node->dev_num == 0) {
        /* When there is no device in the physical machine, return error */
        uda_warn("No device setup.\n");
#ifdef EMU_ST
        uda_uninit_ns_node_dev(ns_node);
        uda_destroy_ns_node(ns_node, false);
#endif
        return -ENODEV;
    }

    uda_info("Setup success. (ns_id=%u; ns=%pK; root_tgid=%u; dev_num=%u; node_dev_num=%u)\n",
        ns_node->ns_id, ns, ns_node->root_tgid, dev_num, ns_node->dev_num);

    return 0;
}

int uda_find_ns_root_pid(struct mnt_namespace *ns, u32 *tgid, TASK_TIME_TYPE *tgid_time)
{
#ifndef EMU_ST
    struct task_struct *task = NULL;
    int ret = -EFAULT;

    rcu_read_lock();
    for_each_process(task) {
        if ((task != NULL) && (task->nsproxy != NULL) && (task->nsproxy->mnt_ns == ns)) {
            *tgid = task->tgid;
            *tgid_time = task->start_time;
            ret = 0;
            break;
        }
    }
    rcu_read_unlock();

    return ret;
#else
    *tgid = current->tgid;
    return 0;
#endif
}

u32 uda_get_ns_pid_count(struct mnt_namespace *ns)
{
    struct task_struct *task = NULL;
    u32 count = 0;

#ifndef EMU_ST
    rcu_read_lock();
    for_each_process(task) {
        if ((task != NULL) && (task->nsproxy != NULL) && (task->nsproxy->mnt_ns == ns)) {
            count++;
        }
    }
    rcu_read_unlock();
#endif

    return count;
}

bool uda_cur_task_is_added_to_dev_cgroup(void)
{
    u32 udevid;

    for (udevid = 0; udevid < UDA_UDEV_MAX_NUM; udevid++) {
        struct uda_dev_inst *dev_inst = uda_dev_inst_get(udevid);
        if (dev_inst != NULL) {
            /* If a file cannot be accessed, the process is added to the dev cgroup. If all davinci devices are
               added to the cgroup, the processing is the same as that the process is not added to the dev cgroup. */
            if (!uda_devcgroup_permission_allow(dev_inst)) {
                uda_dev_inst_put(dev_inst);
                return true;
            }
            uda_dev_inst_put(dev_inst);
        }
    }

    return false;
}

int uda_setup_ns_node(u32 dev_num)
{
    u32 root_tgid = UDA_HOST_ROOT_TGID;
    TASK_TIME_TYPE tgid_time;
    int ret;
    static int first_flag = 1;
    struct mnt_namespace *ns = uda_get_current_ns();
    if (ns == NULL) {
        uda_err("Name space is null.\n");
        return -EFAULT;
    }

    if (first_flag == 1) {
        uda_wait_all_phy_startup();
        first_flag = 0;
    }

    (void)memset_s(&tgid_time, sizeof(tgid_time), 0, sizeof(tgid_time));
    if (uda_cur_is_host()) {
        if (uda_cur_task_is_added_to_dev_cgroup()) {
            /* Create an independent ns node for the process is added to the dev cgroup. */
            root_tgid = current->tgid;
            tgid_time = current->start_time;
        }
    } else {
        ret = uda_find_ns_root_pid(ns, &root_tgid, &tgid_time);
        if (ret != 0) {
            uda_err("Find first tgid failed.\n");
            return ret;
        }
    }

    mutex_lock(&access_ns_mutex);
    ret = _uda_setup_ns_node(ns, root_tgid, tgid_time, dev_num);
    mutex_unlock(&access_ns_mutex);

    return ret;
}

bool uda_tgid_exited(u32 tgid, TASK_TIME_TYPE start_time)
{
    struct task_struct *tsk = NULL;
    struct pid *pid;

#ifndef EMU_ST
    rcu_read_lock();
    pid = get_pid(find_pid_ns(tgid, &init_pid_ns));
    rcu_read_unlock();
    if (pid == NULL) {
        return true;
    }
    tsk = get_pid_task(pid, PIDTYPE_PID);
    put_pid(pid);
    if (tsk == NULL) {
        return true;
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)
    if (start_time == tsk->start_time) {
        put_task_struct(tsk);
        return false;
    }
#else
    if (timespec_equal(&start_time, &tsk->start_time)) {
        put_task_struct(tsk);
        return false;
    }
#endif
    put_task_struct(tsk);
    return true;
#else
    return false;
#endif
}

void uda_ns_node_destroy_work(struct work_struct *p_work)
{
    struct uda_ns_node *ns_node = container_of(p_work, struct uda_ns_node, wait_destroy_work.work);
    u32 pid_count;

    /* ns_pid_count may not be 0 by mnt_ns reuse in new container, so force destroy when timeout */
    if (ns_node->destroy_try_count < FREE_NS_NODE_TRY_COUNT) {
        ns_node->destroy_try_count++;
        if (!uda_tgid_exited(ns_node->root_tgid, ns_node->tgid_time)) {
            uda_info("root_tgid exist.(ns_id=%u; tgid=%u)\n", ns_node->ns_id, ns_node->root_tgid);
#ifndef EMU_ST
            schedule_delayed_work(&ns_node->wait_destroy_work, msecs_to_jiffies(FREE_NS_NODE_INTERVAL_MS));
#endif
            return;
        }
        pid_count = uda_get_ns_pid_count(ns_node->ns);
        if (pid_count != 0) {
            uda_info("ns pid exist.(ns_id=%u; tgid=%u; pid_count=%u)\n", ns_node->ns_id, ns_node->root_tgid, pid_count);
#ifndef EMU_ST
            schedule_delayed_work(&ns_node->wait_destroy_work, msecs_to_jiffies(FREE_NS_NODE_INTERVAL_MS));
#endif
            return;
        }
    }
    /* use try lock avoid deadlock, when occupy, destroy invalid ns_node would cancel this work sync */
    if (!mutex_trylock(&access_ns_mutex)) {
        uda_info("not get mutex lock, wait next time.(ns_id=%u)\n", ns_node->ns_id);
#ifndef EMU_ST
        schedule_delayed_work(&ns_node->wait_destroy_work, msecs_to_jiffies(FREE_NS_NODE_INTERVAL_MS));
#endif
        return;
    }
    uda_uninit_ns_node_dev(ns_node);
    uda_info("work done.(ns_id=%u; tgid=%u; try=%u)\n", ns_node->ns_id, ns_node->root_tgid, ns_node->destroy_try_count);
#ifndef EMU_ST
    uda_destroy_ns_node(ns_node, true);
#endif
    mutex_unlock(&access_ns_mutex);
}

static void uda_cleanup_ns_node(struct mnt_namespace *ns, struct task_struct *task)
{
    struct uda_ns_node *ns_node = NULL;

    mutex_lock(&access_ns_mutex);
    ns_node = uda_find_ns_node(ns, task->tgid);
    /* first proc exit in namespace */
    if ((ns_node != NULL) && (ns_node->root_tgid == task->tgid)) {
        uda_info("Wait for destroy ns. (ns_id=%u; root_tgid=%u; task_name=%s; task_tgid=%u; task_pid=%u)\n",
            ns_node->ns_id, ns_node->root_tgid, task->comm, task->tgid, task->pid);
#ifndef EMU_ST
        schedule_delayed_work(&ns_node->wait_destroy_work, msecs_to_jiffies(FREE_NS_NODE_INTERVAL_MS));
#else
        uda_uninit_ns_node_dev(ns_node);
        uda_destroy_ns_node(ns_node, false);
#endif
    }
    mutex_unlock(&access_ns_mutex);
}

static void uda_recycle_ns_node_single(struct uda_ns_node *ns_node, void *priv)
{
    uda_destroy_ns_node(ns_node, false);
}

void uda_recycle_ns_node(void)
{
    uda_for_each_ns_node_safe(NULL, uda_recycle_ns_node_single);
}

static int uda_task_exit_notify(struct notifier_block *self, unsigned long val, void *data)
{
    struct task_struct *task = (struct task_struct *)data;
    if ((task != NULL) && (task->mm != NULL) && (task->tgid != 0) && (task->tgid == task->pid)) {
        /* Only the main thread of the user process is check. */
        uda_cleanup_ns_node(uda_get_task_ns(task), task);
    }

    return 0;
}

static struct notifier_block uda_task_exit_nb = {
    .notifier_call = uda_task_exit_notify,
};

static void uda_task_exit_reg(void)
{
    /*
     * The legacy profile task-exit hook is gone on modern kernels.
     * UDA can still function without it, so keep registration optional.
     */
    return;
}

static void uda_task_exit_unreg(void)
{
    return;
}

void uda_init_phy_dev_num(void)
{
    u32 node_num = (u32)cpu_to_node((int)num_online_cpus() - 1) + 1;
    (void)uda_set_detected_phy_dev_num(node_num);
}

int uda_access_init(void)
{
    int ret;

    mutex_init(&access_ns_mutex);
    INIT_LIST_HEAD(&access_ns_head);

    ret = uda_cdev_class_init();
    if (ret != 0) {
        return ret;
    }

    uda_task_exit_reg();
    INIT_WORK(&host_ns_init_work, uda_host_ns_init_work);
    schedule_work(&host_ns_init_work);

#ifndef DRV_HOST
    uda_init_phy_dev_num();
#endif

    return 0;
}

void uda_access_uninit(void)
{
    (void)cancel_work_sync(&host_ns_init_work);
    uda_task_exit_unreg();
    uda_recycle_ns_node();
    uda_recycle_all_cdev();
    uda_cdev_class_uninit();
}

#ifdef CFG_FEATURE_DEVID_TRANS
/* old interface stub */
int devdrv_get_devnum(u32 *dev_num)
{
    if (dev_num == NULL) {
        return -EINVAL;
    }

    *dev_num = uda_get_detected_phy_dev_num();
    return 0;
}
EXPORT_SYMBOL(devdrv_get_devnum);


int devdrv_get_vdevnum(u32 *dev_num)
{
    if (dev_num == NULL) {
        return -EINVAL;
    }

    *dev_num = uda_get_mia_dev_num();
    return 0;
}
EXPORT_SYMBOL(devdrv_get_vdevnum);

u32 devdrv_manager_get_devnum(void)
{
    return uda_get_cur_ns_dev_num();
}
EXPORT_SYMBOL(devdrv_manager_get_devnum);

int devdrv_get_devids(u32 *devices, u32 device_num)
{
    return uda_get_cur_ns_udevids(devices, device_num);
}
EXPORT_SYMBOL(devdrv_get_devids);

int devdrv_manager_container_logical_id_to_physical_id(u32 logical_dev_id, u32 *physical_dev_id, u32 *vfid)
{
    return uda_devid_to_phy_devid(logical_dev_id, physical_dev_id, vfid);
}
EXPORT_SYMBOL_GPL(devdrv_manager_container_logical_id_to_physical_id);

#endif

