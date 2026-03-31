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

#ifndef DAVINCI_INTF_UT

#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/bitops.h>
#include <linux/suspend.h>
#include <linux/notifier.h>
#include <linux/version.h>
#include <linux/list.h>
#include <linux/ioctl.h>
#include <linux/module.h>
#include <linux/atomic.h>
#ifndef AOS_LLVM_BUILD
#include <linux/poll.h>
#endif
#include <linux/pci.h>
#include <linux/kthread.h>

#include "securec.h"
#include "davinci_interface.h"
#include "davinci_api.h"
#include "davinci_intf_init.h"
#include "davinci_intf_common.h"
#include "davinci_intf_process.h"
#ifdef CFG_HOST_ENV
#include "tsdrv_common.h"
#else
#include "drv_cpu_type.h"
#include "drv_cpu_mask.h"
#endif

#ifndef AOS_LLVM_BUILD
/* Device struct */
static struct class *g_davinci_class = NULL;
static dev_t g_davinci_intf_dev;
#endif
static struct davinci_intf_stru g_davinci_intf_cb = {{0}};
#ifdef CFG_FEATURE_MANAGE_DOCKER
static dev_t g_davinci_intf_docker_dev;
static struct cdev g_cdev_docker;
#endif

/* Check group id */
unsigned int g_authorized_group_id[DAVINIC_CONFIRM_USER_NUM] = {DAVINIC_NOT_CONFIRM_USER_ID,
    DAVINIC_NOT_CONFIRM_USER_ID, DAVINIC_NOT_CONFIRM_USER_ID};
#ifndef AOS_LLVM_BUILD
static int  g_authorized_group_id_num = 3;
module_param_array(g_authorized_group_id, int, &g_authorized_group_id_num, 0644);
MODULE_PARM_DESC(g_authorized_group_id, "Group ID of system user \"HwHiAiUser\", \"HwDmUser\", \"HwBaseUser\"");
#endif
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");

#define PCI_VENDOR_ID_HUAWEI 0x19e5
#define DEVDRV_DIVERSITY_PCIE_VENDOR_ID 0xFFFF

#ifdef AOS_LLVM_BUILD
#ifndef PCI_ANY_ID
#define PCI_ANY_ID (~0)
#endif
struct pci_device_id {
    __u32 vendor, device;       /* Vendor and device ID or PCI_ANY_ID */
    __u32 subvendor, subdevice; /* Subsystem ID's or PCI_ANY_ID */
    __u32 class, class_mask;    /* (class,subclass,prog-if) triplet */
    unsigned long driver_data; /* Data private to the driver */
};

#define PCI_VDEVICE(vend, dev) \
    .vendor = PCI_VENDOR_ID_##vend, .device = (dev), \
    .subvendor = PCI_ANY_ID, .subdevice = PCI_ANY_ID, 0, 0
#endif
static const struct pci_device_id g_davinci_intf_tbl[] = {
    { PCI_VDEVICE(HUAWEI, 0xd100),           0 },
    { PCI_VDEVICE(HUAWEI, 0xd105),           0 },
    { PCI_VDEVICE(HUAWEI, PCI_DEVICE_CLOUD), 0 },
    { PCI_VDEVICE(HUAWEI, 0xd801),           0 },
    { PCI_VDEVICE(HUAWEI, 0xd500),           0 },
    { PCI_VDEVICE(HUAWEI, 0xd501),           0 },
    { PCI_VDEVICE(HUAWEI, 0xd802),           0 },
    { PCI_VDEVICE(HUAWEI, 0xd803),           0 },
    { PCI_VDEVICE(HUAWEI, 0xd804),           0 },
    { PCI_VDEVICE(HUAWEI, 0xd805),           0 },
    { DEVDRV_DIVERSITY_PCIE_VENDOR_ID, 0xd500, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
    {}
};
MODULE_DEVICE_TABLE(pci, g_davinci_intf_tbl);

STATIC int drv_ascend_intf_ioctl_open_cmd(
    struct file *filep,
    unsigned int cmd,
    unsigned long arg);
STATIC int drv_ascend_intf_ioctl_close_cmd(
    struct file *filep,
    unsigned int cmd,
    unsigned long arg);
int drv_ascend_intf_ioctl_check_module_no_use(
    struct file *filep,
    unsigned int cmd,
    unsigned long arg);
extern void svm_process_exit_clean_slots(int pid);

STATIC int (*const g_davinci_ioctl_handlers[DAVINCI_INTF_IOCTL_CMD_MAX_NR])(struct file *filep, unsigned int cmd,
    unsigned long arg) = {
        [_IOC_NR(DAVINCI_INTF_IOCTL_OPEN)] = drv_ascend_intf_ioctl_open_cmd,
        [_IOC_NR(DAVINCI_INTF_IOCTL_CLOSE)] = drv_ascend_intf_ioctl_close_cmd,
        [_IOC_NR(DAVINCI_INTF_IOCTL_GET_MODULE_STATUS)] = drv_ascend_intf_ioctl_check_module_no_use,
};

STATIC void drv_davinci_svm_process_exit_clean(int pid)
{
#ifdef CFG_SUPPORT_OS_SVM_CLEAN
    svm_process_exit_clean_slots(pid);
#endif
    return;
}

STATIC int check_proc_open_module_count(pid_t owner_pid, const char *module_name, unsigned int max)
{
    struct davinci_intf_process_stru *proc = NULL;
    unsigned int cnt;

    if (max == 0) { /* no limit */
        return 0;
    }
    proc = get_process_entry(&g_davinci_intf_cb, owner_pid);
    if (proc == NULL) {
        log_intf_err("not found proc. (module_name=\"%s\"; pid=%d)\n", module_name, owner_pid);
        return -ESRCH;
    }

    cnt = get_file_module_cnt(proc, module_name);
    if (cnt >= max) {
        log_intf_err("process open module cnt exceeded. (module_name=\"%s\"; pid=%d; cnt=%u; max=%u)\n",
            module_name, owner_pid, cnt, max);
        return -EBADF;
    }
    return 0;
}

STATIC int drv_ascend_set_file_ops(const char *module_name,
    struct davinci_intf_private_stru *prvi)
{
    int ret = -1;
    struct davinci_intf_sub_module_stru *node_info = NULL;
    struct davinci_intf_sub_module_stru *node_info_next = NULL;
    struct file_operations *ops = NULL;

    if (module_name == NULL || prvi == NULL) {
        log_intf_err("Input parameter is null.\n");
        return -EINVAL;
    }
    ops = &prvi->fops;

    /* Module list empty */
    if (list_empty(&g_davinci_intf_cb.module_list) != 0) {
        log_intf_err("Module list is empty. (module_name=\"%s\")\n", module_name);
        return -EINVAL;
    }

    list_for_each_entry_safe(node_info, node_info_next, &g_davinci_intf_cb.module_list, list) {
        /* Find the module and save the ops */
        if (strncmp(node_info->module_name, module_name, DAVINIC_MODULE_NAME_MAX) == 0) {
            ret = check_proc_open_module_count(prvi->owner_pid, module_name, node_info->open_module_max);
            if (ret != 0) {
                return -EINVAL;
            }
            ret = memcpy_s(ops,
                sizeof(struct file_operations),
                &node_info->ops,
                sizeof(struct file_operations));
            if (ret != 0) {
                log_intf_err("memcpy_s error. (module_name=\"%s\"; ret=%d)\n",
                    module_name,
                    ret);
                return -EINVAL;
            }
            prvi->notifier = node_info->notifier;
            prvi->free_type = node_info->free_type;
            return 0;
        }
    }
    log_intf_err("Module not init. (module_name=\"%s\")\n", module_name);
    return  -EINVAL;
}

STATIC void drv_davinci_unset_file_ops(struct davinci_intf_private_stru *file_private)
{
    file_private->fops.open = NULL;
    file_private->fops.release = NULL;
    file_private->fops.unlocked_ioctl = NULL;
#ifndef AOS_LLVM_BUILD
    file_private->fops.mmap = NULL;
#endif
}

#ifndef AOS_LLVM_BUILD
/* To create /dev/davinci_manager_docker. */
#ifdef CFG_FEATURE_MANAGE_DOCKER
STATIC signed int drv_ascend_intf_docker_setup_cdev(const struct file_operations *fops)
{
    struct device *dev = NULL;
    int ret;

    if (fops == NULL) {
        log_intf_err("Invalid parameter. (fops_is_null=%d)\n", (fops == NULL));
        return -EINVAL;
    }

    ret = alloc_chrdev_region(&g_davinci_intf_docker_dev, 0, 1, "hisi_davinci_intf");
    if (ret != 0) {
        log_intf_err("Failed to invoke alloc_chrdev_region for davinci_manager_docker. (ret=%d)\n", ret);
        return -EINVAL;
    }

    g_cdev_docker.owner = THIS_MODULE;
    cdev_init(&g_cdev_docker, fops);
    ret = cdev_add(&g_cdev_docker, g_davinci_intf_docker_dev, 1);
    if (ret < 0) {
        log_intf_err("Failed to invoke cdev_add for davinci_manager_docker. (ret=%d)\n", ret);
        goto unregister_region;
    }

    dev = device_create(g_davinci_class, NULL, g_davinci_intf_docker_dev, NULL, "davinci_manager_docker");
    if (IS_ERR(dev)) {
        ret = PTR_ERR(dev);
        log_intf_err("Failed to invoke device_create for davinci_manager_docker. (ret=%d)\n", ret);
        goto delete_cdev;
    }

    return 0;

delete_cdev:
    cdev_del(&g_cdev_docker);
unregister_region:
    unregister_chrdev_region(g_davinci_intf_docker_dev, 1);

    return ret;
}

STATIC void drv_ascend_intf_docker_cleanup(void)
{
    cdev_del(&g_cdev_docker);
    device_destroy(g_davinci_class, g_davinci_intf_docker_dev);
    unregister_chrdev_region(g_davinci_intf_docker_dev, 1);
    return;
}
#endif

/* Create daivnci interface /dev/davinci_manager */
STATIC signed int drv_ascend_intf_setup_cdev(struct davinci_intf_stru *cb, const struct file_operations *fops)
{
    signed int rc;
    struct device *dev = NULL;

    if ((cb == NULL) || (fops == NULL)) {
        log_intf_err("Input parameter is null.\n");
        return -EINVAL;
    }

    g_davinci_class = class_create(THIS_MODULE, "devdrv_manager");
    if (IS_ERR(g_davinci_class)) {
        rc = PTR_ERR(g_davinci_class);
        g_davinci_class = NULL;
        log_intf_err("class_create failed. (rc=%d)\n", rc);

        return rc;
    }

    rc = alloc_chrdev_region(&g_davinci_intf_dev, 0, 1, "hisi_davinci_intf");
    if (rc) {
        log_intf_err("alloc_chrdev_region failed. (rc=%d)\n", rc);
        goto cls_destroy;
    }

    dev = device_create(g_davinci_class, NULL, g_davinci_intf_dev, NULL, DAVINCI_INTF_DEV_NAME);
    if (IS_ERR(dev)) {
        rc = PTR_ERR(dev);
        log_intf_err("device_create failed. (rc=%d)\n", rc);
        goto unregister_region;
    }
    cb->device = dev;

    cdev_init(&cb->cdev, fops);
    cb->cdev.owner = THIS_MODULE;
    cb->cdev.ops = fops;
    rc = cdev_add(&cb->cdev, g_davinci_intf_dev, 1);
    if (rc < 0) {
        log_intf_err("cdev_add failed. (rc=%d)\n", rc);
        goto dev_destroy;
    }

#ifdef CFG_FEATURE_MANAGE_DOCKER
    rc = drv_ascend_intf_docker_setup_cdev(fops);
    if (rc != 0) {
        log_intf_err("Failed to invoke drv_ascend_intf_docker_setup_cdev. (rc=%d)\n", rc);
        goto clean_manager;
    }
#endif

    return 0;

#ifdef CFG_FEATURE_MANAGE_DOCKER
clean_manager:
    cdev_del(&cb->cdev);
#endif
dev_destroy:
    device_destroy(g_davinci_class, g_davinci_intf_dev);
unregister_region:
    unregister_chrdev_region(g_davinci_intf_dev, 1);
cls_destroy:
    class_destroy(g_davinci_class);
    g_davinci_class = NULL;

    return rc;
}

STATIC signed int drv_ascend_intf_cleanup_cdev(struct davinci_intf_stru *cb)
{
    if (g_davinci_class == NULL) {
        log_intf_err("Input parameter is null.\n");
        return -EFAULT;
    }

#ifdef CFG_FEATURE_MANAGE_DOCKER
    drv_ascend_intf_docker_cleanup();
#endif
    cdev_del(&cb->cdev);
    device_destroy(g_davinci_class, g_davinci_intf_dev);
    unregister_chrdev_region(g_davinci_intf_dev, 1);
    class_destroy(g_davinci_class);
    g_davinci_class = NULL;

    return 0;
}
#endif
void  drv_davinci_free_list_node(
    struct davinci_intf_free_list_stru *free_list)
{
    struct davinci_intf_free_file_stru *file_next = NULL;
    struct davinci_intf_free_file_stru *file_pos = NULL;

    if (list_empty(&free_list->list)) {
        return;
    }
    list_for_each_entry_safe(file_pos, file_next, &free_list->list, list) {
        list_del(&file_pos->list);
        kfree(file_pos);
        file_pos = NULL;
    }
    return;
}

STATIC long drv_ascend_wait_work_finish(struct davinci_intf_private_stru *file_private)
{
    int wait_count = 0;
    /* Check file not in used */
    while ((atomic_read(&file_private->work_count) != 0) &&
        (wait_count < DAVINIC_CONFIRM_MAX_TIME)) {
        wait_count++;
        if (wait_count % DAVINIC_CONFIRM_WARN_MASK) {
            log_intf_warn("Waitting ioctl finish. (module_name=\"%s\"; work_count=%d)\n",
                file_private->module_name,
                atomic_read(&file_private->work_count));
        };
        msleep(DAVINIC_CONFIRM_EACH_TIME);
    }
    if (wait_count >= DAVINIC_CONFIRM_MAX_TIME) {
        log_intf_err("Wait ioctl finish timeout. (module_name=\"%s\"; work_count=%d)\n",
            file_private->module_name,
            atomic_read(&file_private->work_count));
        return -EBUSY;
    }

    return 0;
}
STATIC long drv_davinci_inc_work_count(struct davinci_intf_private_stru *file_private_data)
{
    /* If release_status has been set,no more work are allowed. */
    if (file_private_data->release_status == TRUE) {
        return -EBUSY;
    }
    atomic_inc(&file_private_data->work_count);
    return 0;
}

STATIC void drv_davinci_dec_work_count(struct davinci_intf_private_stru *file_private_data)
{
    atomic_dec(&file_private_data->work_count);
    return;
}

/* Check whether the process exists.
If the process does not exist, release related resources. */
STATIC void drv_davinci_intf_check_process(void)
{
    struct davinci_intf_process_stru *proc_node = NULL;
    struct list_head *process_list = NULL;
    struct list_head *pos = NULL;
    struct list_head *next = NULL;

    process_list = &g_davinci_intf_cb.process_list;
    if (list_empty_careful(process_list)) {
        return;
    }

    list_for_each_safe(pos, next, process_list)
    {
        proc_node = (struct davinci_intf_process_stru *)list_entry(
            pos,
            struct davinci_intf_process_stru,
            list);
        if (list_empty_careful(&proc_node->file_list) &&
            (atomic_read(&proc_node->work_count) <= 0)) {
            list_del(pos);
            free_process_entry(proc_node);
            kfree(proc_node);
            proc_node = NULL;
        }
    }
    return;
}

/* Release sub module */
STATIC long drv_ascend_intf_sub_call_release(struct davinci_intf_free_file_stru *file_node)
{
    struct davinci_intf_private_stru *file_private = NULL;
    int ret = 0;
    if (file_node == NULL) {
        log_intf_err("Invalid file_node.\n");
        return 0;
    }
    file_private = file_node->file_private;
    if (file_private == NULL) {
        log_intf_err("Invalid file_private_data. (module_name=\"%s\"; owner_pid=%d;)\n",
            file_node->module_name, file_node->owner_pid);
        return 0;
    }
    if (file_private->fops.release != NULL) {
        if (drv_ascend_wait_work_finish(file_private)) {
            log_intf_err("Dev is busy, cannot close. (module_name=\"%s\"; owner_pid=%d; work_count=%d)\n",
                file_private->module_name, file_private->owner_pid,
                atomic_read(&file_private->work_count));
            return -EBUSY;
        }
        /* Call file release */
        ret = file_private->fops.release(0, &file_private->priv_filep);
        if (ret != 0) {
            log_intf_err("Release failed. (module_name=\"%s\"; owner_pid=%d; ret=%d)\n",
                file_private->module_name, file_private->owner_pid,
                ret);
        }
    }

    if (file_private->fops.owner != NULL) {
        if (module_refcount(file_private->fops.owner) > 0) {
            module_put(file_private->fops.owner);
        }
    }
    if (file_private->notifier.owner != NULL) {
        if (module_refcount(file_private->notifier.owner) > 0) {
            module_put(file_private->notifier.owner);
        }
    }

    log_intf_debug("Release module success. (module_name=\"%s\"; pid=%d)\n",
        file_node->module_name, file_node->owner_pid);
    mutex_lock(&file_private->fmutex);
    drv_davinci_unset_file_ops(file_private);
    mutex_unlock(&file_private->fmutex);
    mutex_destroy(&file_private->fmutex);
    kfree(file_private);
    file_private = NULL;
    return ret;
}

int drv_davinci_release_task(void *arg)
{
    int all_flag = FALSE;
    int owner_pid = 0;

    struct davinci_intf_free_list_stru *free_list = (struct davinci_intf_free_list_stru *)arg;

    if (free_list) {
        all_flag = free_list->all_flag;
        owner_pid = free_list->owner_pid;
    }

    /* run release work, the "free_list" check int the function "drv_ascend_release_work" */
    drv_ascend_release_work(free_list);

    /* need call svm process exit clean */
    if (all_flag == TRUE) {
        drv_davinci_svm_process_exit_clean(owner_pid);
    }

    /* need call do_exit, noone will call kthread_stop */
    do_exit(0);
    return 0;
}

/* set release task cpumask,ensure that run in the CTRLCPU */
#ifdef CFG_BIND_CPU_ENABLE
static inline void drv_bind_task_by_mask(struct task_struct *release_task,
    cpumask_var_t cpu_mask)
{
#ifndef CFG_HOST_ENV
    if (IS_ERR(release_task) || (release_task == NULL)) {
        return;
    }
    if (cpumask_available(cpu_mask)) {
        /* set cpumask */
        set_cpus_allowed_ptr(release_task, cpu_mask);
    }
#endif
    return;
}
#endif

void drv_davinci_bind_ctrl_cpu(void *release_task)
{
#ifdef CFG_BIND_CPU_ENABLE
    drv_bind_task_by_mask((struct task_struct *)release_task, g_davinci_intf_cb.cpumask);
    return;
#endif
}
EXPORT_SYMBOL(drv_davinci_bind_ctrl_cpu);

int drv_davinci_release_run(void *arg)
{
    long ret;
    struct davinci_intf_free_file_stru *file_node = arg;
    ret = drv_ascend_intf_sub_call_release(file_node);
    atomic_dec(&file_node->owner_list->current_count);
    kfree(file_node);
    do_exit(0);
    return ret;
}

static inline void drv_ascend_wait_finish(struct davinci_intf_free_list_stru *free_list)
{
    unsigned int count = 0;
    while ((atomic_read(&free_list->current_count) > 0) &&
        (count < DAVINIC_FREE_WAIT_MAX_TIME)) {
        msleep(DAVINIC_FREE_WAIT_EACH_TIME);
        count++;
    }
    if (count >= DAVINIC_FREE_WAIT_MAX_TIME) {
        log_intf_err("Waiting for the previous task to complete timed out. (owner_pid=%d; count=%u)\n",
            free_list->owner_pid, count);
    }
}

static void drv_davinci_wait_free_finish(struct davinci_intf_free_file_stru *file_node)
{
    if ((file_node->owner_list->current_free_index == file_node->free_index) &&
        (file_node->free_type == DAVINIC_FREE_IN_PARALLEL)) {
        return;
    }

    drv_ascend_wait_finish(file_node->owner_list);
    return;
}

STATIC long drv_ascend_intf_release_file(struct davinci_intf_free_file_stru *file_node)
{
    long ret;
    struct task_struct *release_task = NULL;
    /* wait last free work finish */
    drv_davinci_wait_free_finish(file_node);
    file_node->owner_list->current_free_index = file_node->free_index;
    atomic_inc(&file_node->owner_list->current_count);
    if (file_node->free_type == DAVINIC_FREE_IN_ORDER) {
        ret = drv_ascend_intf_sub_call_release(file_node);
        atomic_dec(&file_node->owner_list->current_count);
        kfree(file_node);
        return ret;
    } else {
        /* create async free task */
        release_task = kthread_create(drv_davinci_release_run, (void*)file_node, "davinci_sub_recycle");
        if (IS_ERR(release_task) || (release_task == NULL)) {
            log_intf_warn("Kthread_create not up to expectations. (pid=%d, num=%ld)\n", file_node->owner_pid, PTR_ERR(release_task));
            ret = drv_ascend_intf_sub_call_release(file_node);
            atomic_dec(&file_node->owner_list->current_count);
            kfree(file_node);
            return ret;
        }
        drv_davinci_bind_ctrl_cpu(release_task);
        (void)wake_up_process(release_task);
    }
    return 0;
}

void drv_ascend_release_work(struct davinci_intf_free_list_stru *free_list)
{
    struct davinci_intf_free_file_stru *file_next = NULL;
    struct davinci_intf_free_file_stru *file_pos = NULL;
    struct list_head *file_list = NULL;
    int ret;
    if (free_list == NULL) {
        log_intf_err("Input parameter error,proc is NULL.\n");
        return;
    }

    /* If file list is empty, completion of the process */
    if (list_empty(&free_list->list)) {
        goto out;
    }

    atomic_set(&free_list->current_count, 0);

    file_list = &free_list->list;
    /* for each free_node */
    list_for_each_entry_safe(file_pos, file_next, file_list, list)
    {
        list_del(&file_pos->list);
        /* release the sub module's file */
        ret = drv_ascend_intf_release_file(file_pos);
        if (ret != 0) {
            log_intf_err("Release failed. (ret=%d)\n", ret);
            continue;
        }
    }
    /* wait all free finish */
    drv_ascend_wait_finish(free_list);
out:
    /* dec the work_count */
    if (free_list->owner_proc != NULL) {
        atomic_dec(&free_list->owner_proc->work_count);
    }
    kfree(free_list);
    free_list = NULL;
    return;
}

STATIC struct davinci_intf_sub_module_stru *drv_davinci_get_module(const char *module_name)
{
    struct davinci_intf_sub_module_stru *node_info = NULL;
    struct davinci_intf_sub_module_stru *node_info_next = NULL;

    if (list_empty(&g_davinci_intf_cb.module_list)) {
        return NULL;
    }

    list_for_each_entry_safe(node_info, node_info_next, &g_davinci_intf_cb.module_list, list) {
        if (strcmp(node_info->module_name, module_name) == 0) {
            return node_info;
        }
    }
    return  NULL;
}

STATIC int check_notifier(struct davinci_intf_private_stru *file_private, struct notifier_operations *noti)
{
    struct davinci_intf_sub_module_stru *module_stru = NULL;

    module_stru = drv_davinci_get_module(file_private->module_name);
    if (module_stru == NULL) {
        log_intf_err("not found module struct. (module_name=\"%s\";pid=%d)\n",
            file_private->module_name, file_private->owner_pid);
        return -ESRCH;
    }

    *noti = module_stru->notifier;
    if (noti->owner == file_private->notifier.owner && noti->notifier_call == file_private->notifier.notifier_call) {
        return 0;
    }

    log_intf_err("check notifier fail. (module_name=\"%s\"; pid=%d; diff owner=%d; fun=%d)\n",
        file_private->module_name, file_private->owner_pid, noti->owner != file_private->notifier.owner,
        noti->notifier_call != file_private->notifier.notifier_call);

    return -EBADF;
}

STATIC void drv_ascend_intf_call_notifier(
    struct davinci_intf_private_stru *file_private,
    unsigned long mode)
{
    int ret;
    struct notifier_operations verified_notifier = {0};

    if (file_private == NULL || file_private->notifier.notifier_call == NULL) {
        return;
    }

    ret = check_notifier(file_private, &verified_notifier);
    if (ret != 0) {
        return;
    }

    /* Call notifier_call */
    ret = verified_notifier.notifier_call(&file_private->priv_filep, mode);
    if (ret != 0) {
        log_intf_err("Notify sub module failed. (module_name=\"%s\"; mode=0x%lx; ret=%d)\n",
            file_private->module_name, mode, ret);
        return;
    }
}

STATIC void drv_davinci_notify_release_work(struct davinci_intf_free_list_stru *free_list)
{
    struct davinci_intf_free_file_stru *file_next = NULL;
    struct davinci_intf_free_file_stru *file_pos = NULL;
    struct list_head *file_list = NULL;
    file_list = &free_list->list;
    list_for_each_entry_safe(file_pos, file_next, file_list, list) {
        drv_ascend_intf_call_notifier(file_pos->file_private, NOTIFY_MODE_RELEASE_PREPARE);
    }
}

#if (!defined(CFG_HOST_ENV)) && (!defined(AOS_LLVM_BUILD))

int drv_ascend_get_ctrlcpu_mask(cpumask_t *ctrl_cpumask)
{
#ifdef CFG_FEATURE_SUPPORT_CPUDOMAIN
    return drv_get_ctrlcpu_mask(ctrl_cpumask);
#else
    return drv_get_ctrlcpu_mask_from_cpuset(ctrl_cpumask);
#endif
}

static void init_ctrl_cpumask(struct davinci_intf_stru *cb)
{
    int ret;
    cpumask_t ctrl_cpumask;

    cpumask_clear(&ctrl_cpumask);
    ret = drv_ascend_get_ctrlcpu_mask(&ctrl_cpumask);
    if (ret != 0) {
        log_intf_info("The cpumask configuration is not found. The CPU affinity will not be set.\n");
        return;
    }

    /* alloc cpumask var */
    if (!zalloc_cpumask_var(&cb->cpumask, GFP_KERNEL)) {
        log_intf_info("cpumask variable not be alloced, The CPU affinity will not be set.\n");
        return;
    }
    cpumask_copy(cb->cpumask, &ctrl_cpumask);
    return;
}
#else
static void init_ctrl_cpumask(struct davinci_intf_stru *cb)
{
    log_intf_debug("no need to init cpu mask.\n");
}
#endif

STATIC void drv_ascend_run_release_work(struct davinci_intf_free_list_stru *free_list)
{
    struct task_struct *release_task = NULL;

    /* when the file list is null,means already released */
    if (list_empty(&free_list->list)) {
        atomic_dec(&free_list->owner_proc->work_count);
        kfree(free_list);
        free_list = NULL;
        return;
    }

    /* Notify sub mode preare release first */
    drv_davinci_notify_release_work(free_list);

    /* init the queue_work,
     Normally the free_list free in the "drv_ascend_release_work" */
    release_task = kthread_create(drv_davinci_release_task, (void*)free_list, "davinci_recycle");
    if (IS_ERR(release_task) || (release_task == NULL)) {
        log_intf_warn("Kthread_create not up to expectations. (pid=%d, num=%ld)\n", free_list->owner_pid, PTR_ERR(release_task));
        drv_ascend_release_work(free_list);
        return;
    }

    drv_davinci_bind_ctrl_cpu(release_task);

    /* Start the work task */
    (void)wake_up_process(release_task);
    return;
}

/* Release resources when module exit */
STATIC int drv_ascend_intf_release_process(void)
{
    struct davinci_intf_process_stru *proc_node = NULL;
    struct list_head *process_list = NULL;
    struct list_head *pos = NULL;
    struct list_head *next = NULL;
    process_list = &g_davinci_intf_cb.process_list;
    if (list_empty(process_list)) {
        return 0;
    }

    list_for_each_safe(pos, next, process_list)
    {
        proc_node = (struct davinci_intf_process_stru *)list_entry(
            pos,
            struct davinci_intf_process_stru,
            list);
        list_del(pos);
        destory_file_proc_list(proc_node);
        kfree(proc_node);
    }
    return 0;
}

STATIC int drv_ascend_add_release_list_node(
    struct davinci_intf_free_list_stru *free_list,
    struct file *file, unsigned int free_index)
{
    struct davinci_intf_private_stru *file_private_data = NULL;
    struct davinci_intf_free_file_stru *node = NULL;
    int ret;
    file_private_data = file->private_data;
    if (file_private_data == NULL) {
        log_intf_err("Invalid file_private_data.\n");
        drv_davinci_free_list_node(free_list);
        return -EINVAL;
    }
    /* already in release state */
    if (file_private_data->release_status == TRUE) {
        return 0;
    }
    node = (struct davinci_intf_free_file_stru *)kzalloc(
        sizeof(struct davinci_intf_free_file_stru),
        GFP_ATOMIC | __GFP_ACCOUNT);
    if (node == NULL) {
        log_intf_err("kzalloc failed. (size=%lu)\n", sizeof(struct davinci_intf_free_file_stru));
        drv_davinci_free_list_node(free_list);
        return -ENOMEM;
    }
    INIT_LIST_HEAD(&node->list);
    node->owner_pid = file_private_data->owner_pid;
    node->file_private = file_private_data;
    node->free_type = file_private_data->free_type;
    node->owner_list = free_list;
    node->free_index = free_index;

    ret = strcpy_s(node->module_name, DAVINIC_MODULE_NAME_MAX,
        file_private_data->module_name);
    if (ret != 0) {
        log_intf_err("strcpy_s failed. (module_name=\"%s\"; ret=%d)\n",
            file_private_data->module_name,
            ret);
        kfree(node);
        node = NULL;
        drv_davinci_free_list_node(free_list);
        return -ENOSYS;
    }
    file_private_data->release_status = TRUE;
    /* private_data free in the function "drv_ascend_intf_sub_call_release" */
    file->private_data = NULL;
    list_add_tail(&node->list, &free_list->list);
    return 0;
}

STATIC int drv_ascend_add_release_list_all(
    struct davinci_intf_process_stru *proc,
    struct davinci_intf_free_list_stru *free_list,
    struct file *file)
{
    struct davinci_intf_sub_module_stru *node_info_next = NULL;
    struct davinci_intf_sub_module_stru *node_info = NULL;
    struct davinci_intf_file_stru *file_next = NULL;
    struct davinci_intf_file_stru *file_pos = NULL;
    struct list_head *module_list = NULL;
    struct list_head *file_list = NULL;
    unsigned int free_index = 0;
    int ret = 0;

    /* already free or no file opened */
    if (list_empty(&proc->file_list)) {
        return 0;
    }
    module_list = &g_davinci_intf_cb.module_list;
    list_for_each_entry_safe(node_info, node_info_next, module_list, list)
    {
        free_index++;
        mutex_lock(&proc->res_lock);
        file_list = &proc->file_list;
        list_for_each_entry_safe(file_pos, file_next, file_list, list)
        {
            if ((strcmp(node_info->module_name, file_pos->module_name) != 0) &&
                (strcmp(DAVINIC_UNINIT_FILE, file_pos->module_name) != 0)) {
                continue;
            }
            /* Delete file list first */
            list_del(&file_pos->list);
            ret = drv_ascend_add_release_list_node(free_list, file_pos->file_op, free_index);
            if (ret != 0) {
                log_intf_info("Unable to add release list. (ret=%d; pid=%d; module_name=\"%s\")\n",
                    ret, file_pos->owner_pid, file_pos->module_name);
                mutex_unlock(&proc->res_lock);
                return ret;
            }
            log_intf_info("Add release list success. (pid=%d; module_name=\"%s\"; "
                "seq=%u; time=%u; cur=%u; trig=%d; mm=%d)\n",
                file_pos->owner_pid, file_pos->module_name, file_pos->seq,
                file_pos->open_time, jiffies_to_msecs(jiffies), file_pos->file_op == file, current->mm != NULL);
            kfree(file_pos);
            file_pos = NULL;
        }
        mutex_unlock(&proc->res_lock);
    }
    return ret;
}

STATIC struct davinci_intf_free_list_stru *drv_ascend_make_release_list(
    struct davinci_intf_process_stru *proc,
    struct file *file,
    int all_flag)
{
    struct davinci_intf_free_list_stru *release_list = NULL;
    int ret;

    release_list = (struct davinci_intf_free_list_stru *)kzalloc(
        sizeof(struct davinci_intf_free_list_stru),
        GFP_ATOMIC | __GFP_ACCOUNT);
    if (release_list == NULL) {
        log_intf_err("kzalloc failed. (size=%lu)\n", sizeof(struct davinci_intf_free_list_stru));
        return NULL;
    }
    INIT_LIST_HEAD(&release_list->list);
    release_list->owner_pid = proc->owner_pid;
    release_list->owner_proc = proc;
    release_list->all_flag = all_flag;
    atomic_set(&release_list->current_count, 0);

    /* free single file description */
    if (all_flag == FALSE) {
        mutex_lock(&proc->res_lock);
        ret = drv_ascend_add_release_list_node(release_list, file, 0);
        if (ret != 0) {
            kfree(release_list);
            release_list = NULL;
            mutex_unlock(&proc->res_lock);
            return NULL;
        }
        /* need remove file description from process filelist */
        destory_file_proc_entry(proc, file);
        mutex_unlock(&proc->res_lock);
    } else {
        ret = drv_ascend_add_release_list_all(proc, release_list, file);
        if (ret != 0) {
            kfree(release_list);
            release_list = NULL;
            return NULL;
        }
    }
    return release_list;
}

STATIC int drv_ascend_intf_open(struct inode *inode, struct file *file)
{
    struct davinci_intf_private_stru *file_private_data = NULL;
    struct davinci_intf_stru *cb = &g_davinci_intf_cb;
    signed int ret = -EBUSY;
    ASSERT_RET((inode != NULL), (-EFAULT));
    ASSERT_RET((file != NULL), (-EFAULT));

    if (!try_module_get(THIS_MODULE)) {
        return ret;
    }

    file_private_data = (struct davinci_intf_private_stru *)kzalloc(
        sizeof(struct davinci_intf_private_stru), GFP_KERNEL | __GFP_ACCOUNT);
    if (file_private_data == NULL) {
        log_intf_err("kzalloc failed. (size=%lu)\n", sizeof(struct davinci_intf_private_stru));
        module_put(THIS_MODULE);
        return -ENOMEM;
    }
    mutex_init(&file_private_data->fmutex);
    file_private_data->device_cb = cb;
    file_private_data->device_id = DAVINIC_INTF_INVAILD_DEVICE_ID;
    file_private_data->owner_pid = current->tgid;
    file_private_data->close_flag = DAVINIC_NOT_INIT_BY_OPENCMD;
    file_private_data->release_status = FALSE;
    atomic_set(&file_private_data->work_count, 0);
    memcpy(&file_private_data->priv_filep, file, sizeof(struct file));
    ret = strcpy_s(file_private_data->module_name, DAVINIC_MODULE_NAME_MAX,
        DAVINIC_UNINIT_FILE);
    if (ret != 0) {
        log_intf_err("strcpy_s failed. (module_name=\"%s\"; ret=%d)\n",
            file_private_data->module_name,
            ret);
        ret = -ENOSYS;
        goto out_err;
    }
    mutex_lock(&cb->dmutex);
    /* Check process list */
    drv_davinci_intf_check_process();
    ret = add_file_to_list(cb, file);
    mutex_unlock(&cb->dmutex);
    if (ret != 0) {
        log_intf_err("Add file to list failed. (ret=%d)\n", ret);
        goto out_err;
    }
    file->private_data = file_private_data;
    atomic_inc(&cb->count);

    return ret;
out_err:
    module_put(THIS_MODULE);
    file->private_data = NULL;
    kfree(file_private_data);
    file_private_data = NULL;
    return ret;
}

STATIC int drv_ascend_intf_release(struct inode *inode, struct file *file)
{
    struct davinci_intf_private_stru *file_private_data = NULL;
    struct davinci_intf_free_list_stru *free_list = NULL;
    struct davinci_intf_process_stru *proc = NULL;
    struct davinci_intf_stru *cb = &g_davinci_intf_cb;
    int all_flag;
    ASSERT_RET((inode != NULL), (-EFAULT));
    ASSERT_RET((file != NULL), (-EFAULT));

    module_put(THIS_MODULE);
    mutex_lock(&cb->dmutex);
    /* Check process list */
    drv_davinci_intf_check_process();

    file_private_data = file->private_data;
    if ((file_private_data == NULL) || (file_private_data->device_cb == NULL)) {
        mutex_unlock(&cb->dmutex);
        return -EBADFD;
    }

    atomic_dec(&cb->count);
    proc = get_process_entry(cb, file_private_data->owner_pid);
    if (proc == NULL) {
        /* if proc is null, means already free by other fd */
        mutex_unlock(&cb->dmutex);
        return 0;
    }

    /* close_flag was FALSE, means free all file */
    all_flag = (file_private_data->close_flag == FALSE) ? TRUE : FALSE;
    free_list = drv_ascend_make_release_list(
        proc, file, all_flag);
    if (free_list == NULL) {
        mutex_unlock(&cb->dmutex);
        log_intf_err("Make free list failed. (module_name=\"%s\"; pid=%d)\n",
            file_private_data->module_name, file_private_data->owner_pid);
        return -EBADFD;
    }
    /* inc process work count */
    atomic_inc(&proc->work_count);
    mutex_unlock(&cb->dmutex);
    drv_ascend_run_release_work(free_list);
    return 0;
}

STATIC int drv_ascend_intf_init_file_private(
    struct davinci_intf_private_stru *file_private,
    const unsigned char *module_name,
    int device_id)
{
    int ret;

    mutex_lock(&file_private->fmutex);
    if (file_private->close_flag  != DAVINIC_NOT_INIT_BY_OPENCMD) {
        mutex_unlock(&file_private->fmutex);
        log_intf_err("Already call open command. (module_name=\"%s\"; new_name=\"%s\")\n",
            file_private->module_name,
            module_name);
        return -ENOSYS;
    }

    ret = drv_ascend_set_file_ops(module_name, file_private);
    if (ret != 0) {
        mutex_unlock(&file_private->fmutex);
        log_intf_err("set file ops failed. (module_name=\"%s\"; ret=%d)\n",
            module_name,
            ret);
        return ret;
    }
    ret = strcpy_s(file_private->module_name, DAVINIC_MODULE_NAME_MAX, module_name);
    if (ret != 0) {
        mutex_unlock(&file_private->fmutex);
        log_intf_err("strcpy_s failed. (module_name=\"%s\"; ret=%d)\n",
            module_name,
            ret);
        return -ENOSYS;
    }
    file_private->device_id = device_id;
    mutex_unlock(&file_private->fmutex);
    return 0;
}

STATIC int drv_ascend_intf_open_private(struct davinci_intf_private_stru *file_private)
{
    int ret;

    if (file_private->fops.owner != NULL) {
        if (!try_module_get(file_private->fops.owner)) {
            log_intf_err("module is busy. (module_name=\"%s\"; owner_pid=%d)\n",
                 file_private->module_name, file_private->owner_pid);
            return -EBUSY;
        }
    }

    if (file_private->notifier.owner != NULL) {
        if (!try_module_get(file_private->notifier.owner)) {
            log_intf_err("notify module is busy. (module_name=\"%s\"; owner_pid=%d)\n",
                 file_private->module_name, file_private->owner_pid);
            return -EBUSY;
        }
    }

    if (file_private->fops.open != NULL) {
        ret = file_private->fops.open(file_private->priv_filep.f_inode, &file_private->priv_filep);
        if (ret != 0) {
            log_intf_err("open fail. (module_name=\"%s\"; ret=%d)\n",
                file_private->module_name, ret);
        }
        return ret;
    }

    return -EINVAL;
}

int drv_ascend_intf_ioctl_open_cmd(
    struct file *filep,
    unsigned int cmd,
    unsigned long arg)
{
    int ret;
    struct davinci_intf_open_arg module_para = {{0}};
    struct davinci_intf_private_stru *file_private = NULL;
    size_t name_len;
    mutex_lock(&g_davinci_intf_cb.dmutex);
    file_private = filep->private_data;
    if (file_private == NULL) {
        log_intf_err("Invalid file_private_data.\n");
        mutex_unlock(&g_davinci_intf_cb.dmutex);
        return -EBADFD;
    }

    if ((void *)arg == NULL) {
        log_intf_err("Invalid arg from user.\n");
        mutex_unlock(&g_davinci_intf_cb.dmutex);
        return -EINVAL;
    }

    if (copy_from_user(&module_para, (void *)(uintptr_t)arg, sizeof(struct davinci_intf_open_arg))) {
        log_intf_err("copy_from_user failed. (size=%lu)\n", sizeof(struct davinci_intf_open_arg));
        mutex_unlock(&g_davinci_intf_cb.dmutex);
        return -EFAULT;
    }
    module_para.module_name[DAVINIC_MODULE_NAME_MAX - 1] = '\0';
    name_len = strnlen(module_para.module_name, DAVINIC_MODULE_NAME_MAX);
    if ((name_len == 0) || (name_len >= DAVINIC_MODULE_NAME_MAX)) {
        log_intf_err("Length out of range. (name_len=%lu)\n", name_len);
        mutex_unlock(&g_davinci_intf_cb.dmutex);
        return -EINVAL;
    }

    ret = drv_ascend_intf_init_file_private(file_private,
        module_para.module_name,
        module_para.device_id);
    if (ret != 0) {
        log_intf_err("Init file_private failed. (module_name=\"%s\"; ret=%d)\n",
            module_para.module_name, ret);
        mutex_unlock(&g_davinci_intf_cb.dmutex);
        return ret;
    }

    ret = drv_ascend_intf_open_private(file_private);
    if (ret != 0) {
        log_intf_err("Call open function failed. (module_name=\"%s\"; ret=%d)\n",
            file_private->module_name, ret);
        mutex_unlock(&g_davinci_intf_cb.dmutex);
        return ret;
    }

    ret = add_module_to_list(&g_davinci_intf_cb, file_private->owner_pid,
        filep, (char *)module_para.module_name);
    if (ret != 0) {
        log_intf_err("add_module_to_list failed. (module_name=\"%s\"; ret=%d)\n",
            module_para.module_name, ret);
        mutex_unlock(&g_davinci_intf_cb.dmutex);
        return ret;
    }

    mutex_lock(&file_private->fmutex);
    file_private->close_flag = FALSE;
    mutex_unlock(&file_private->fmutex);
    mutex_unlock(&g_davinci_intf_cb.dmutex);

    return 0;
}

int drv_ascend_intf_ioctl_close_cmd(struct file *filep,
    unsigned int cmd,
    unsigned long arg)
{
    struct davinci_intf_private_stru *file_private = NULL;
    mutex_lock(&g_davinci_intf_cb.dmutex);
    file_private = filep->private_data;
    if (file_private == NULL) {
        log_intf_err("Invalid file_private_data.\n");
        mutex_unlock(&g_davinci_intf_cb.dmutex);
        return -EBADFD;
    }

    /* child thread call will close file of father thread  */
    if (file_private->owner_pid != current->tgid) {
        mutex_unlock(&g_davinci_intf_cb.dmutex);
        return 0;
    }
    mutex_lock(&file_private->fmutex);
    file_private->close_flag = TRUE;
    mutex_unlock(&file_private->fmutex);
    mutex_unlock(&g_davinci_intf_cb.dmutex);
    return 0;
}

int drv_ascend_intf_ioctl_check_module_no_use(
    struct file *filep,
    unsigned int cmd,
    unsigned long arg)
{
    struct davinci_intf_check_no_use_arg module_para = {{0}};
    struct davinci_intf_private_stru *file_private = NULL;
    size_t name_len;
    mutex_lock(&g_davinci_intf_cb.dmutex);
    file_private = filep->private_data;
    if (file_private == NULL) {
        log_intf_err("Invalid file_private_data.\n");
        mutex_unlock(&g_davinci_intf_cb.dmutex);
        return -EBADFD;
    }

    if ((void *)arg == NULL) {
        log_intf_err("Invalid arg from user.\n");
        mutex_unlock(&g_davinci_intf_cb.dmutex);
        return -EINVAL;
    }

    if (copy_from_user(&module_para, (void *)(uintptr_t)arg, sizeof(struct davinci_intf_check_no_use_arg))) {
        log_intf_err("copy_from_user failed. (size=%lu)\n", sizeof(struct davinci_intf_check_no_use_arg));
        mutex_unlock(&g_davinci_intf_cb.dmutex);
        return -EFAULT;
    }
    name_len = strnlen(module_para.module_name, DAVINIC_MODULE_NAME_MAX);
    if ((name_len == 0) || (name_len >= DAVINIC_MODULE_NAME_MAX)) {
        log_intf_err("Length out of range. (name_len=%lu)\n", name_len);
        mutex_unlock(&g_davinci_intf_cb.dmutex);
        return -EINVAL;
    }

    module_para.status = check_module_file_close(&g_davinci_intf_cb, module_para.module_name);
    mutex_unlock(&g_davinci_intf_cb.dmutex);
    if (copy_to_user((void *)(uintptr_t)arg, &module_para, sizeof(struct davinci_intf_check_no_use_arg))) {
        log_intf_err("copy_to_user failed. (size=%lu)\n", sizeof(struct davinci_intf_check_no_use_arg));
        return -EFAULT;
    }
    return 0;
}

STATIC long drv_ascend_intf_ioctl_local(struct file *filep,
    unsigned int cmd,
    unsigned long arg)
{
    if (_IOC_NR(cmd) >= DAVINCI_INTF_IOCTL_CMD_MAX_NR) {
        log_intf_err("invalid cmd,out of range. (cmd=%u)\n", _IOC_NR(cmd));
        return -EINVAL;
    }

    if (g_davinci_ioctl_handlers[_IOC_NR(cmd)] == NULL) {
        log_intf_err("invalid cmd,function not defined. (cmd=%u)\n", _IOC_NR(cmd));
        return -EINVAL;
    }

    return g_davinci_ioctl_handlers[_IOC_NR(cmd)](filep, cmd, arg);
}

STATIC long drv_ascend_intf_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct davinci_intf_private_stru *file_private = NULL;
    long ret;
    ASSERT_RET((filep != NULL), (-EFAULT));

    file_private = filep->private_data;
    if (file_private == NULL) {
        log_intf_err("Invalid file_private_data.\n");
        return -EBADFD;
    }

    switch (_IOC_TYPE(cmd)) {
        case DAVINCI_INTF_IOC_MAGIC:
            return drv_ascend_intf_ioctl_local(filep, cmd, arg);
        default:
            if (file_private->fops.unlocked_ioctl == NULL) {
                log_intf_err("File not init. (cmd=%u)\n", _IOC_NR(cmd));
                return -ENODEV;
            }
            ret = drv_davinci_inc_work_count(file_private);
            if (ret != 0) {
                log_intf_err("Call is not allowed in released state. (cmd=%u; ret=%ld)\n",
                    _IOC_NR(cmd),
                    ret);
                return -ENODEV;
            }
            ret = file_private->fops.unlocked_ioctl(&file_private->priv_filep, cmd, arg);
            drv_davinci_dec_work_count(file_private);
            return ret;
    }
}

#ifndef AOS_LLVM_BUILD
STATIC int drv_ascend_intf_mmap(struct file *filep, struct vm_area_struct *vma)
{
    struct davinci_intf_private_stru *file_private = NULL;
    long ret;
    ASSERT_RET((filep != NULL), (-EFAULT));
    ASSERT_RET((vma != NULL), (-EFAULT));

    file_private = filep->private_data;
    if ((file_private == NULL) || (file_private->fops.mmap == NULL)) {
        log_intf_err("Invalid file_private_data.\n");
        return -EBADFD;
    }

    ret = drv_davinci_inc_work_count(file_private);
    if (ret != 0) {
        log_intf_err("Call is not allowed in released state. (ret=%ld)\n",
            ret);
        return -ENODEV;
    }
    ret = file_private->fops.mmap(&file_private->priv_filep, vma);
    drv_davinci_dec_work_count(file_private);

    return ret;
}

STATIC unsigned int drv_ascend_intf_poll(
    struct file *filep,
    struct poll_table_struct *wait)
{
    struct davinci_intf_private_stru *file_private = NULL;
    long ret;
    ASSERT_RET((filep != NULL), POLLERR);
    ASSERT_RET((wait != NULL), POLLERR);

    file_private = filep->private_data;
    if ((file_private == NULL) || (file_private->fops.poll == NULL)) {
        log_intf_err("Invalid file_private_data.\n");
        return POLLERR;
    }

    ret = drv_davinci_inc_work_count(file_private);
    if (ret != 0) {
        log_intf_err("Call is not allowed in released state. (ret=%ld)\n",
            ret);
        return POLLERR;
    }
    ret = file_private->fops.poll(&file_private->priv_filep, wait);
    drv_davinci_dec_work_count(file_private);
    return ret;
}
#endif

static const struct file_operations g_davinci_intf_fops = {
    .owner = THIS_MODULE,
    .open = drv_ascend_intf_open,
    .release = drv_ascend_intf_release,
#ifdef AOS_LLVM_BUILD
    .ioctl = drv_ascend_intf_ioctl,
#else
    .unlocked_ioctl = drv_ascend_intf_ioctl,
    .mmap = drv_ascend_intf_mmap,
    .poll = drv_ascend_intf_poll,
#endif
};

STATIC int drv_davinci_check_module_init(const char *module_name)
{
    struct davinci_intf_sub_module_stru *node_info = NULL;
    struct davinci_intf_sub_module_stru *node_info_next = NULL;

    if (list_empty(&g_davinci_intf_cb.module_list) != 0) {
        return FALSE;
    }

    list_for_each_entry_safe(node_info, node_info_next, &g_davinci_intf_cb.module_list, list) {
        if (strcmp(node_info->module_name, module_name) == 0) {
            return TRUE;
        }
    }
    return  FALSE;
}

STATIC struct davinci_intf_sub_module_stru *alloc_module_node(
    const char *module_name,
    const struct file_operations *ops)
{
    struct davinci_intf_sub_module_stru *node = NULL;
    int ret;

    node = (struct davinci_intf_sub_module_stru *)kzalloc(
        sizeof(struct davinci_intf_sub_module_stru),
        GFP_KERNEL | __GFP_ACCOUNT);
    if (node == NULL) {
        log_intf_err("kzalloc failed. (module_name=\"%s\"; size=%lu)\n",
            module_name,
            sizeof(struct davinci_intf_sub_module_stru));
        return NULL;
    }
    ret = strcpy_s(node->module_name, DAVINIC_MODULE_NAME_MAX, module_name);
    if (ret != 0) {
        log_intf_err("strcpy_s error. (module_name=\"%s\"; ret=%d)\n",
            module_name,
            ret);
        goto out_err;
    }
    ret = memcpy_s(&node->ops,
        sizeof(struct file_operations),
        ops,
        sizeof(struct file_operations));
    if (ret != 0) {
        log_intf_err("memcpy_s error. (module_name=\"%s\"; ret=%d)\n",
            module_name,
            ret);
        goto out_err;
    }
    node->valid = VALID;
    return node;
out_err:
    kfree(node);
    node = NULL;
    return NULL;
}
STATIC int drv_ascend_register_module(
    const char *module_name,
    const struct file_operations *ops, unsigned int free_type, unsigned int open_module_max)
{
    struct davinci_intf_sub_module_stru *node = NULL;

    if (module_name == NULL || ops == NULL
        || ((free_type != DAVINIC_FREE_IN_PARALLEL) && (free_type != DAVINIC_FREE_IN_ORDER))) {
        log_intf_err("Input parameter is null.\n");
        return -EINVAL;
    }

    if (strnlen(module_name, DAVINIC_MODULE_NAME_MAX) >= DAVINIC_MODULE_NAME_MAX) {
        log_intf_err("Length out of range. (length=%lu)\n",
            strnlen(module_name, DAVINIC_MODULE_NAME_MAX));
        return -EINVAL;
    }

    mutex_lock(&g_davinci_intf_cb.dmutex);
    if (drv_davinci_check_module_init(module_name) == TRUE) {
        mutex_unlock(&g_davinci_intf_cb.dmutex);
        log_intf_err("Already init. (module_name=\"%s\")\n", module_name);
        return -EINVAL;
    }
    node = alloc_module_node(module_name, ops);
    if (node == NULL) {
        mutex_unlock(&g_davinci_intf_cb.dmutex);
        log_intf_err("alloc_module_node failed. (module_name=\"%s\")\n",
            module_name);
        return -ENOMEM;
    }
    node->free_type = free_type;
    node->open_module_max = open_module_max;
    if (strcmp(node->module_name, DAVINCI_INTF_MODULE_TSDRV) == 0) {
        list_add(&node->list, &g_davinci_intf_cb.module_list);
    } else {
        list_add_tail(&node->list, &g_davinci_intf_cb.module_list);
    }
    mutex_unlock(&g_davinci_intf_cb.dmutex);
    return 0;
}
int drv_davinci_register_sub_module_cnt(
    const char *module_name,
    const struct file_operations *ops,
    unsigned int open_module_max)
{
    return drv_ascend_register_module(module_name, ops, DAVINIC_FREE_IN_ORDER, open_module_max);
}
EXPORT_SYMBOL(drv_davinci_register_sub_module_cnt);

int drv_davinci_register_sub_module(
    const char *module_name,
    const struct file_operations *ops)
{
    return drv_ascend_register_module(module_name, ops, DAVINIC_FREE_IN_ORDER, 0);
}
EXPORT_SYMBOL(drv_davinci_register_sub_module);

int drv_davinci_register_sub_parallel_module(
    const char *module_name,
    const struct file_operations *ops)
{
    return drv_ascend_register_module(module_name, ops, DAVINIC_FREE_IN_PARALLEL, 0);
}
EXPORT_SYMBOL(drv_davinci_register_sub_parallel_module);

int drv_ascend_unregister_sub_module(const char *module_name)
{
    struct davinci_intf_sub_module_stru *node_info = NULL;
    struct davinci_intf_sub_module_stru *node_info_next = NULL;
    if (module_name == NULL) {
        log_intf_err("Input parameter is null.\n");
        return -EINVAL;
    }

    mutex_lock(&g_davinci_intf_cb.dmutex);
    if (list_empty(&g_davinci_intf_cb.module_list) != 0) {
        mutex_unlock(&g_davinci_intf_cb.dmutex);
        log_intf_warn("There was no one registered. (module_name=\"%s\")\n", module_name);
        return 0;
    }

    /* Check file all close */
    if (check_module_file_close(&g_davinci_intf_cb, module_name) == FALSE) {
        mutex_unlock(&g_davinci_intf_cb.dmutex);
        log_intf_warn("Some file not close. (module_name=\"%s\")\n", module_name);
        return -EINVAL;
    }
    /* Release module */
    list_for_each_entry_safe(node_info, node_info_next, &g_davinci_intf_cb.module_list, list) {
        if (strncmp(node_info->module_name, module_name, DAVINIC_MODULE_NAME_MAX) == 0) {
            list_del(&node_info->list);
            mutex_unlock(&g_davinci_intf_cb.dmutex);
            kfree(node_info);
            node_info = NULL;
            return 0;
        }
    }
    mutex_unlock(&g_davinci_intf_cb.dmutex);
    return  0;
}
EXPORT_SYMBOL(drv_ascend_unregister_sub_module);

int drv_ascend_register_notify(
    const char *module_name,
    const struct notifier_operations *notifier)
{
    struct davinci_intf_sub_module_stru *node_info = NULL;
    struct davinci_intf_sub_module_stru *node_info_next = NULL;
    int ret;

    if (module_name == NULL) {
        log_intf_err("Input parameter is null.\n");
        return -EINVAL;
    }

    mutex_lock(&g_davinci_intf_cb.dmutex);
    /* replace module fops */
    list_for_each_entry_safe(node_info, node_info_next, &g_davinci_intf_cb.module_list, list) {
        if (strncmp(node_info->module_name, module_name, DAVINIC_MODULE_NAME_MAX) == 0) {
            if (node_info->notifier.notifier_call != NULL) {
                log_intf_warn("duplicate register notify. (module_name=\"%s\")\n", module_name);
            }
            ret = memcpy_s(&node_info->notifier, sizeof(struct notifier_operations),
                notifier, sizeof(struct notifier_operations));
            mutex_unlock(&g_davinci_intf_cb.dmutex);
            if (ret != 0) {
                log_intf_err("memcpy_s error. (module_name=\"%s\"; ret=%d)\n", module_name, ret);
                return -EINVAL;
            }
            return 0;
        }
    }
    mutex_unlock(&g_davinci_intf_cb.dmutex);
    log_intf_err("cannot find %s module node, replace failed.\n", module_name);
    return -EINVAL;
}
EXPORT_SYMBOL(drv_ascend_register_notify);

int drv_ascend_unregister_notify(const char *module_name)
{
    struct davinci_intf_sub_module_stru *node_info = NULL;
    struct davinci_intf_sub_module_stru *node_info_next = NULL;

    if (module_name == NULL) {
        log_intf_err("Input parameter is null.\n");
        return -EINVAL;
    }

    mutex_lock(&g_davinci_intf_cb.dmutex);
    /* replace module fops */
    list_for_each_entry_safe(node_info, node_info_next, &g_davinci_intf_cb.module_list, list) {
        if (strncmp(node_info->module_name, module_name, DAVINIC_MODULE_NAME_MAX) == 0) {
            if (node_info->notifier.notifier_call == NULL) {
                log_intf_warn("duplicate unregister notify. (module_name=\"%s\")\n", module_name);
            }
            (void)memset_s(&node_info->notifier, sizeof(struct notifier_operations),
                0, sizeof(struct notifier_operations));
            mutex_unlock(&g_davinci_intf_cb.dmutex);
            return 0;
        }
    }
    mutex_unlock(&g_davinci_intf_cb.dmutex);
    log_intf_err("cannot find module node, unregister failed. (module_name=\"%s\")\n", module_name);
    return -EINVAL;
}
EXPORT_SYMBOL(drv_ascend_unregister_notify);

int drv_ascend_replace_sub_module_fops(const char *module_name,
    const struct file_operations *ops)
{
    struct davinci_intf_sub_module_stru *node_info = NULL;
    struct davinci_intf_sub_module_stru *node_info_next = NULL;
    int ret;

    if (module_name == NULL) {
        log_intf_err("Input parameter is null.\n");
        return -EINVAL;
    }

    mutex_lock(&g_davinci_intf_cb.dmutex);
    /* replace module fops */
    list_for_each_entry_safe(node_info, node_info_next, &g_davinci_intf_cb.module_list, list) {
        if (strncmp(node_info->module_name, module_name, DAVINIC_MODULE_NAME_MAX) == 0) {
            ret = memcpy_s(&node_info->ops, sizeof(struct file_operations), ops,
                sizeof(struct file_operations));
            mutex_unlock(&g_davinci_intf_cb.dmutex);
            if (ret != 0) {
                log_intf_err("memcpy_s error. (module_name=\"%s\"; ret=%d)\n", module_name, ret);
                return -EINVAL;
            }
            return 0;
        }
    }
    mutex_unlock(&g_davinci_intf_cb.dmutex);
    log_intf_err("cannot find %s module node, replace failed.\n", module_name);
    return -EINVAL;
}
EXPORT_SYMBOL(drv_ascend_replace_sub_module_fops);

u32 drv_davinci_get_device_id(const struct file *filep)
{
    struct davinci_intf_private_stru *file_private = NULL;

    file_private = container_of(filep, struct davinci_intf_private_stru, priv_filep);
    return file_private->device_id;
}
EXPORT_SYMBOL(drv_davinci_get_device_id);

int ascend_intf_report_process_status(pid_t pid, unsigned int status)
{
    struct davinci_intf_process_stru *proc = NULL;

    mutex_lock(&g_davinci_intf_cb.dmutex);
    proc = get_process_entry(&g_davinci_intf_cb, pid);
    if (proc == NULL) {
        log_intf_err("Process not init,proc is NULL. (pid=%d)\n", pid);
        mutex_unlock(&g_davinci_intf_cb.dmutex);
        return -ESRCH;
    }
    /* Clear status */
    if (status & DAVINCI_INTF_PROCESS_CLEAR_STATUS) {
        proc->status &= ~status;
    } else {
        proc->status |= status;
    }
    mutex_unlock(&g_davinci_intf_cb.dmutex);

    return 0;
}
EXPORT_SYMBOL(ascend_intf_report_process_status);

STATIC int ascend_intf_get_process_status(pid_t pid, unsigned int *status)
{
    struct davinci_intf_process_stru *proc = NULL;

    mutex_lock(&g_davinci_intf_cb.dmutex);
    proc = get_process_entry(&g_davinci_intf_cb, pid);
    if (proc == NULL) {
        log_intf_err("Process not init,proc is NULL. (pid=%d)\n", pid);
        mutex_unlock(&g_davinci_intf_cb.dmutex);
        return -ESRCH;
    }
    *status = proc->status;
    mutex_unlock(&g_davinci_intf_cb.dmutex);

    return 0;
}

bool davinci_intf_confirm_user(void)
{
#ifdef AOS_LLVM_BUILD
    return true;
#else
    unsigned int i = 0, valid_flag = false;
    const struct cred *cred = current_cred();

    for (i = 0; i < DAVINIC_CONFIRM_USER_NUM; i++) {
        /* group id not init,means don't need confirm */
        if (g_authorized_group_id[i] == DAVINIC_NOT_CONFIRM_USER_ID) {
            return true;
        }
        /* if in the group, it will return 1, otherwise return 0. */
        if (in_group_p(*(kgid_t *)&g_authorized_group_id[i])) {
            valid_flag = true;
        }
    }
    if (valid_flag == true || cred->uid.val == DAVINIC_ROOT_USER_ID) {
        return true;
    }
    return false;
#endif
}
EXPORT_SYMBOL(davinci_intf_confirm_user);

u32 davinci_intf_get_manage_group(void)
{
    return g_authorized_group_id[1]; /* 0:HwHiAiUser; 1:HwDmUser; 2:HwBaseUser. */
}
EXPORT_SYMBOL(davinci_intf_get_manage_group);

struct device *davinci_intf_get_owner_device(void)
{
    return g_davinci_intf_cb.device;
}
EXPORT_SYMBOL(davinci_intf_get_owner_device);

int ascend_intf_report_device_status(unsigned int device_id, unsigned int status)
{
    if (device_id >= MAX_DEVICE_COUNT) {
        log_intf_err("Input parameter is error. (device_id=%u)\n", device_id);
        return -EINVAL;
    }
    /* Clear status */
    if (status & DAVINCI_INTF_DEVICE_CLEAR_STATUS) {
        g_davinci_intf_cb.device_status[device_id] &= ~status;
    } else {
        g_davinci_intf_cb.device_status[device_id] |= status;
    }
    return 0;
}
EXPORT_SYMBOL(ascend_intf_report_device_status);

int ascend_intf_get_status(struct ascend_intf_get_status_para para, unsigned int *status)
{
    if (((para.type == DAVINCI_STATUS_TYPE_DEVICE) && (para.para.device_id >= MAX_DEVICE_COUNT)) || (status == NULL)) {
        log_intf_err("Input parameter is error. (device_id=%u)\n", para.para.device_id);
        return -EINVAL;
    }

    switch (para.type) {
        case DAVINCI_STATUS_TYPE_PROCESS:
            return  ascend_intf_get_process_status(para.para.process_id, status);
        case DAVINCI_STATUS_TYPE_DEVICE:
            *status = g_davinci_intf_cb.device_status[para.para.device_id];
            return 0;
        default:
            log_intf_err("Input parameter is error. (type=%u)\n", para.type);
            return -EINVAL;
    }
}
EXPORT_SYMBOL(ascend_intf_get_status);

int ascend_intf_is_pid_init(pid_t process_id, const char *module_name)
{
    struct davinci_intf_process_stru *proc = NULL;
    mutex_lock(&g_davinci_intf_cb.dmutex);
    /* check process_pid open status */
    proc = get_process_entry(&g_davinci_intf_cb, process_id);
    if (proc == NULL) {
        mutex_unlock(&g_davinci_intf_cb.dmutex);
        return false;
    }
    /* if module_name not null, check module open status */
    if ((module_name != NULL) && (check_module_file_close_in_process(proc, module_name) == TRUE)) {
        mutex_unlock(&g_davinci_intf_cb.dmutex);
        return false;
    }
    mutex_unlock(&g_davinci_intf_cb.dmutex);
    return true;
}

EXPORT_SYMBOL(ascend_intf_is_pid_init);

int drv_davinci_intf_cb_init(struct davinci_intf_stru *cb)
{
    mutex_init(&cb->dmutex);
    atomic_set(&cb->count, 0);
    cb->device = NULL;
    INIT_LIST_HEAD(&cb->module_list);
    INIT_LIST_HEAD(&cb->process_list);
    init_ctrl_cpumask(cb);
    return 0;
}

int drv_davinci_intf_cb_destory(struct davinci_intf_stru *cb)
{
    mutex_lock(&cb->dmutex);
#ifndef CFG_HOST_ENV
#ifndef AOS_LLVM_BUILD
    if (cpumask_available(cb->cpumask)) {
        free_cpumask_var(cb->cpumask);
    }
#endif
#endif
    atomic_set(&cb->count, 0);
    cb->device = NULL;
    destroy_process_list(cb);
    mutex_unlock(&cb->dmutex);
    mutex_destroy(&cb->dmutex);
    return 0;
}

STATIC int __init drv_ascend_intf_init(void)
{
    int ret;
    ret = drv_davinci_intf_cb_init(&g_davinci_intf_cb);
    if (ret != 0) {
        return ret;
    }
#ifdef AOS_LLVM_BUILD
    ret = register_driver(DAVINCI_INTF_FULL_DEV_NAME, &g_davinci_intf_fops, DAVINIC_NONE_ROOT_ACCESS, NULL);
#else
    ret = drv_ascend_intf_setup_cdev(&g_davinci_intf_cb, &g_davinci_intf_fops);
#endif
    if (ret != 0) {
        log_intf_err("drv_ascend_intf_setup_cdev failed. (ret=%d)\n", ret);
        goto cleanup1;
    }
    return 0;
cleanup1:
    drv_davinci_intf_cb_destory(&g_davinci_intf_cb);
    return ret;
}

STATIC void __exit drv_davinci_intf_exit(void)
{
#ifdef AOS_LLVM_BUILD
    unregister_driver(DAVINCI_INTF_FULL_DEV_NAME);
#else
    drv_ascend_intf_cleanup_cdev(&g_davinci_intf_cb);
#endif
    drv_ascend_intf_release_process();
    atomic_set(&g_davinci_intf_cb.count, 0);
#ifndef CFG_HOST_ENV
#ifndef AOS_LLVM_BUILD
    if (cpumask_available(g_davinci_intf_cb.cpumask)) {
        free_cpumask_var(g_davinci_intf_cb.cpumask);
    }
#endif
#endif
    g_davinci_intf_cb.device = NULL;
    mutex_destroy(&g_davinci_intf_cb.dmutex);
}

module_init(drv_ascend_intf_init);
module_exit(drv_davinci_intf_exit);

MODULE_LICENSE("GPL");

#else
int drv_davinci_unset_file_ops(void)
{
    return 0;
}

#endif
