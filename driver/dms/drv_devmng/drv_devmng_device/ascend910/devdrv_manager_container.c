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

#ifndef DEVDRV_MANAGER_HOST_UT_TEST
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/nsproxy.h>
#include <linux/sched.h>
#include <linux/rbtree.h>
#include <linux/uaccess.h>
#include <linux/cred.h>
#include <linux/namei.h>
#include <linux/pid_namespace.h>
#include <linux/cpuset.h>
#endif
#include "devdrv_manager.h"
#include "devdrv_manager_common.h"
#include "devdrv_manager_container.h"
#include "devdrv_pcie.h"
#include "dbl/uda.h"
#ifdef CFG_FEATURE_DEVICE_SHARE
#include "devdrv_manager_dev_share.h"
#endif

#ifndef DEVDRV_MANAGER_HOST_UT_TEST
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
#include <linux/sched/signal.h>
#include <linux/sched/task.h>
#include <linux/cgroup.h>
#endif

#ifdef CFG_FEATURE_OLD_DEVID_TRANS

#define DEVMNG_CONTAINER_DEVICE_CONFLICT 1
#define DEVMNG_CONTAINER_DEVICE_NORMAL 0

int devdrv_manager_container_is_in_container(void);

int devdrv_is_in_container(void)
{
    return devdrv_manager_container_is_in_container();
}

struct devdrv_container_table {
    struct list_head item_list_head;
    spinlock_t spinlock;
};

struct devdrv_container_main_pid {
    pid_t tgid;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)
    u64 start_time;
    u64 real_start_time;
#else
    struct timespec start_time;
    struct timespec real_start_time;
#endif
};

struct devdrv_container_item {
    struct list_head item_list_node;
    struct devdrv_container_main_pid main_pid;

    u32 id;
    u32 conflict_flag;
    u32 dev_num;
    u32 pf_num;
    u32 vf_num;
    u32 dev_id[DEVDRV_MAX_DAVINCI_NUM];
    u32 pf_devid[DEVDRV_PF_DEV_MAX_NUM];
    u32 vf_devid[MAX_VDEV_NUM];
    u64 dev_list_map[DEVDRV_CONTIANER_NUM_OF_LONG];
    u64 container_id;

    struct mnt_namespace *mnt_ns;
    struct pid_namespace *pid_ns;
    spinlock_t spinlock;
};

enum docker_status_type {
    DOCKER_STATUS_IDLE = 0,
    DOCKER_STATUS_USED
};

STATIC int docker_status[MAX_DOCKER_NUM] = {DOCKER_STATUS_IDLE};
#else
#include "dbl/uda.h"
#endif // CFG_FEATURE_OLD_DEVID_TRANS

struct mnt_namespace *devdrv_manager_get_host_mnt_ns(void)
{
    return init_task.nsproxy->mnt_ns;
}

int devdrv_devpid_container_convert(int *ipid)
{
#ifndef UT_VCAST
    struct pid *kpid = NULL;

    if (ipid == NULL) {
        devdrv_drv_err("Parameter invaild.\n");
        return -EINVAL;
    }

    if (devdrv_manager_container_is_in_container() != true) {
        /* Physical machines do not need to be converted. */
        return 0;
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0)
    kpid = find_pid_ns(*ipid, init_task.nsproxy->pid_ns_for_children);
#else
    kpid = find_pid_ns(*ipid, init_task.nsproxy->pid_ns);
#endif
    if (kpid != NULL) {
        *ipid = kpid->numbers[kpid->level].nr;
        return 0;
    } else {
        return -ESRCH;
    }
#endif
}

#if defined(CFG_HOST_ENV) || defined(CFG_FEATURE_BIND_TGID)
int devdrv_get_tgid_by_pid(int pid, int *tgid)
{
    struct task_struct *tsk = NULL;
    struct pid_namespace *pid_ns = NULL;
    int ret;

    if (tgid == NULL) {
        devdrv_drv_err("The tgid is NULL.\n");
        return -EINVAL;
    }

    /* To get current process's pid_ns. */
    pid_ns = task_active_pid_ns(current);
    if (pid_ns == NULL) {
        devdrv_drv_err("Failed to find current process's pid_ns. (pid=%d)\n", pid);
        return -EINVAL;
    }

    /*
    * The value of tsk is NULL, which means the PID and current process are not in the same pid_ns.
    * Even if we find it, it doesn't mean it really is, because each container may has its own pid_ns.
    */
    tsk = get_pid_task(find_pid_ns(pid, pid_ns), PIDTYPE_PID);
    if (tsk == NULL) {
        devdrv_drv_err("Failed to find task struct in current process's pid_ns. (pid=%d)\n", pid);
        return -EINVAL;
    }

    /* If not in container, it return tgid directly. */
    ret = devdrv_manager_container_is_in_container();
    if (ret < 0) {
        devdrv_drv_err("Failed to invoke devdrv_is_in_container. (ret=%d)\n", ret);
        goto OUT;
    } else if (ret != true) {
        *tgid = tsk->tgid;
        ret = 0;
        goto OUT;
    }

    /* If in container, the PID mnt_ns will be equal to the current mnt_ns. */
    if ((tsk->nsproxy != NULL) && (tsk->nsproxy->mnt_ns == current->nsproxy->mnt_ns)) {
        *tgid = tsk->tgid;
        ret = 0;
        goto OUT;
    }

    ret = -EINVAL;
    devdrv_drv_err("The PID does not exist in the container. (pid=%d)\n", pid);
OUT:
    put_task_struct(tsk);
    return ret;
}
#endif

STATIC bool devdrv_manager_container_is_admin_task(struct task_struct *tsk)
{
#ifdef CAP_FOR_EACH_U32
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0)
    kernel_cap_t privileged = (kernel_cap_t){{ ~0, (CAP_TO_MASK(CAP_AUDIT_READ + 1) -1)}};
#else
    kernel_cap_t privileged = CAP_FULL_SET;
#endif
#else
    kernel_cap_t privileged = CAP_FULL_SET;
#endif
    const struct cred *cred = NULL;
    kernel_cap_t effective;
    unsigned int i, user_id;

    rcu_read_lock();
    cred = __task_cred(tsk); //lint !e1058 !e64 !e666
    effective = cred->cap_effective;
    user_id = cred->uid.val;
    rcu_read_unlock();

 #ifdef CAP_FOR_EACH_U32
    CAP_FOR_EACH_U32(i)
    {
        if ((effective.cap[i] & privileged.cap[i]) != privileged.cap[i]) {
            return false;
        }
    }
 #else
    if (!cap_issubset(privileged, effective)) {
        return false;
    }
 #endif
    if (user_id == 0) {
        return true;
    } else {
        return false;
    }
}

bool devdrv_manager_container_is_admin(void)
{
    return devdrv_manager_container_is_admin_task(current);
}
#endif // DEVDRV_MANAGER_HOST_UT_TEST
/**
 * Check the process is running in the physical machine or in container
 */
bool devdrv_manager_container_is_host_system(struct mnt_namespace *mnt_ns)
{
    if (mnt_ns == devdrv_manager_get_host_mnt_ns()) {
        return true;
    }

    return false;
}
#ifndef DEVDRV_MANAGER_HOST_UT_TEST
#ifdef CFG_FEATURE_OLD_DEVID_TRANS
STATIC void devdrv_manager_container_print_list(struct devdrv_container_table *dev_container_table)
{
    struct devdrv_container_item *item = NULL;
    struct list_head *pos = NULL, *n = NULL;

    devdrv_drv_info_spinlock("=============device container table==============\n");
    devdrv_drv_info_spinlock("host_mnt_ns 0x%pK,host_pid_ns 0x%pK\n",
        (void *)(uintptr_t)devdrv_manager_get_host_mnt_ns(), (void *)(uintptr_t)&init_pid_ns);

    if (list_empty_careful(&dev_container_table->item_list_head)) {
        return;
    }

    list_for_each_safe(pos, n, &dev_container_table->item_list_head) {
        item = list_entry(pos, struct devdrv_container_item, item_list_node);
        devdrv_drv_info_spinlock("item id(%u) dev num (%u) dev map (0x%llx) mnt_ns(0x%pK)"
            " pid_ns(0x%pK) main_proc(%d) conflict_flag(%u)\n", item->id,
            item->dev_num, item->dev_list_map[0], (void *)(uintptr_t)item->mnt_ns,
            (void *)(uintptr_t)item->pid_ns, item->main_pid.tgid, item->conflict_flag);
    }
}

int devdrv_manager_container_get_devids_list(struct devdrv_manager_hccl_devinfo *hccl_devinfo)
{
    struct devdrv_container_table *dev_container_table = NULL;
    struct devdrv_container_item *item = NULL;
    struct list_head *pos = NULL, *n = NULL;
    u32 array_num = 0;
    u32 i;

    if (dev_manager_info == NULL) {
        devdrv_drv_err("dev_manager_info = 0x%pK\n", (void *)(uintptr_t)dev_manager_info);
        return -EINVAL;
    }

    dev_container_table = dev_manager_info->container_table;
    if (dev_container_table == NULL) {
        devdrv_drv_err("container table is NULL\n");
        return -ENOMEM;
    }

    spin_lock(&dev_container_table->spinlock);
    if (!list_empty_careful(&dev_container_table->item_list_head)) {
        list_for_each_safe(pos, n, &dev_container_table->item_list_head)
        {
            if (array_num != 0) {
                item = list_entry(pos, struct devdrv_container_item, item_list_node);
                hccl_devinfo->num_dev += item->dev_num;
                for (i = 0; i < item->dev_num; i++) {
                    hccl_devinfo->devids[i] = item->dev_id[i];
                }
            }
            array_num++;
        }
    }
    spin_unlock(&dev_container_table->spinlock);

    return 0;
}
#endif // CFG_FEATURE_OLD_DEVID_TRANS

#define DEVMNG_MAX_TEXT_LENGTH 34
#define DEVMNG_UNKNOWN_CONTAINER_ID 0xFFFFFFFFFFFF
#define HEXADECIMAL 16

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
STATIC int new_cpuset_read(char *buf, size_t count)
{
    struct cgroup_subsys_state *css = NULL;
    int retval;

    css = task_get_css(current, cpuset_cgrp_id);
    if (css == NULL) {
        devdrv_drv_err("task get css failed.\n");
        return -EINVAL;
    }

    retval = cgroup_path_ns(css->cgroup, buf,
        count, current->nsproxy->cgroup_ns);
    css_put(css);
    if (retval <= 0) {
        devdrv_drv_err("read buf failed. (retval=%d)\n", retval);
        return -EINVAL;
    }

    return count;
}
#endif

STATIC void devdrv_manager_get_container_id(unsigned long long *container_id)
{
    char cpuset_file_text[DEVMNG_MAX_TEXT_LENGTH] = {0};
    struct file *fp = NULL;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)
    loff_t pos = 0;
#endif
    int ret;
    char *res = NULL;
    char container_id_string[DEVMNG_MAX_TEXT_LENGTH] = {0};
    size_t container_id_len = 12;

    if (container_id == NULL) {
        devdrv_drv_err("input container id is NULL\n");
        return;
    }

    /* set default container id, which means we can't get real container id. */
    *container_id = DEVMNG_UNKNOWN_CONTAINER_ID;

    /**
     * /proc/[pid]/cpuset
     * inside docker the context of this file is
     * "/docker/<container_id>" or "/system.slice/docker-<container_id>"
     */
    fp = filp_open("/proc/self/cpuset", O_RDONLY, 0);
    if (IS_ERR(fp)) {
        devdrv_drv_err("open cpuset file failed, err = %ld.\n", PTR_ERR(fp));
        return;
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
    ret = new_cpuset_read(cpuset_file_text, DEVMNG_MAX_TEXT_LENGTH);
#else
    ret = kernel_read(fp, pos, cpuset_file_text, DEVMNG_MAX_TEXT_LENGTH);
#endif
    filp_close(fp, NULL);
    if ((ret <= 0) || (ret > DEVMNG_MAX_TEXT_LENGTH)) {
        devdrv_drv_err("get cpuset file context failed, ret = %d.\n", ret);
        return;
    }

    cpuset_file_text[DEVMNG_MAX_TEXT_LENGTH - 1] = '\0';
    res = strstr(cpuset_file_text, "docker");
    if (res == NULL) {
        devdrv_drv_err("cannot find docker in /proc/self/cpuset.\n");
        return;
    }

    ret = strncpy_s(container_id_string, DEVMNG_MAX_TEXT_LENGTH, res + strlen("docker") + 1, container_id_len);
    if (ret != 0) {
        devdrv_drv_err("strncpy_s container id failed. (ret=%d)\n", ret);
        return;
    }

    ret = kstrtoull(container_id_string, HEXADECIMAL, container_id);
    if (ret < 0) {
        devdrv_drv_err("kstrtoul failed. (ret=%d)\n", ret);
        return;
    }

    return;
}

STATIC int devdrv_manager_container_check_current(void)
{
    /* current->nsproxy is NULL when the release function is called */
    if (current == NULL || current->nsproxy == NULL || current->nsproxy->mnt_ns == NULL) {
        devdrv_drv_warn("(current == NULL) is %d, (current->nsproxy == NULL) is %d\n",
            (current == NULL), ((current == NULL) ? (-EINVAL) : (current->nsproxy == NULL)));
        return -EINVAL;
    }

    return 0;
}

#ifdef CFG_FEATURE_OLD_DEVID_TRANS
int devdrv_manager_container_get_docker_id(u32 *docker_id)
{
    struct devdrv_container_table *dev_container_table = NULL;
    struct mnt_namespace *current_mnt_ns = NULL;
    struct devdrv_container_item *item = NULL;
    struct list_head *pos = NULL;
    struct list_head *n = NULL;

    if (docker_id == NULL) {
        devdrv_drv_err("docker_id is NULL\n");
        return -EINVAL;
    }

    *docker_id = MAX_DOCKER_NUM;

    if (dev_manager_info == NULL) {
        devdrv_drv_err("dev_manager_info is NULL\n");
        return -EINVAL;
    }

    dev_container_table = dev_manager_info->container_table;
    if (dev_container_table == NULL) {
        devdrv_drv_err("container table is NULL\n");
        return -EINVAL;
    }

    if (devdrv_manager_container_is_in_container()) {
        current_mnt_ns = current->nsproxy->mnt_ns;
    } else {
        /*
         * Operation not permitted: the env is not docker.
         * there is no need to print to prevent too many prints, just need to return a clear error code.
         */
        return -EPERM;
    }

    spin_lock(&dev_container_table->spinlock);
    if (!list_empty_careful(&dev_container_table->item_list_head)) {
        list_for_each_safe(pos, n, &dev_container_table->item_list_head) {
            item = list_entry(pos, struct devdrv_container_item, item_list_node);
            if (item->mnt_ns == current_mnt_ns) {
                *docker_id = item->id;
                break;
            }
        }
    }
    spin_unlock(&dev_container_table->spinlock);

    if (*docker_id >= MAX_DOCKER_NUM) {
        devdrv_drv_err("can't get docker_id or the docker_id is invalid: docker_id = %u\n", *docker_id);
        return -EINVAL;
    }

    return 0;
}
#else
int devdrv_manager_container_get_docker_id(u32 *docker_id)
{
    int ret;

    if (devdrv_manager_container_is_in_container() == false) {
        return -EPERM;
    }

    ret = uda_get_cur_ns_id(docker_id);
    if (ret != 0) {
        devdrv_drv_err("Query ns id failed\n");
        return ret;
    }

    return 0;
}
#endif
EXPORT_SYMBOL(devdrv_manager_container_get_docker_id);

#ifdef CFG_FEATURE_OLD_DEVID_TRANS
STATIC inline struct pid_namespace *devdrv_manager_container_get_pid_ns(struct task_struct *tsk)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0)
    return tsk->nsproxy->pid_ns_for_children;
#else
    return tsk->nsproxy->pid_ns;
#endif
}

STATIC struct task_struct *devdrv_manager_container_get_main_proc_by_first_pid(
    struct pid_namespace *pid_ns)
{
    struct task_struct *tsk = get_pid_task(find_pid_ns(1, pid_ns), PIDTYPE_PID);

    /* 1st pid equal to main pid */
    if (tsk == NULL) {
        return NULL;
    }

    if (tsk->nsproxy == NULL) {
        put_task_struct(tsk);
        return NULL;
    }

    if (tsk->nsproxy->mnt_ns != current->nsproxy->mnt_ns) {
        put_task_struct(tsk);
        return NULL;
    }

    return tsk;
}

STATIC struct task_struct *devdrv_manager_container_get_main_proc_by_task_list(void)
{
    struct task_struct *tsk = NULL;
    struct task_struct *p = NULL;

    rcu_read_lock();
    for_each_process(p) {
        if ((p != NULL) && (p->nsproxy != NULL) &&
            (p->nsproxy->mnt_ns == current->nsproxy->mnt_ns)) {
            tsk = p;
            get_task_struct(tsk);
            break;
        }
    }
    rcu_read_unlock();

    return tsk;
}

STATIC int devdrv_manager_container_find_main_proc(pid_t *nr, pid_t *vnr)
{
    struct task_struct *main_tsk = NULL;
    struct pid_namespace *pid_ns = NULL;

    pid_ns = task_active_pid_ns(current);
    if (pid_ns == NULL) {
        devdrv_drv_err_spinlock("find current pid ns failed.\n");
        return -EINVAL;
    }

    main_tsk = devdrv_manager_container_get_main_proc_by_first_pid(pid_ns);
    if (main_tsk == NULL) {
        main_tsk = devdrv_manager_container_get_main_proc_by_task_list();
        if (main_tsk == NULL) {
            devdrv_drv_err_spinlock("find main pid failed, current mnt_ns(0x%pK),"
                " pid_ns(0x%pK), current pid_ns(0x%pK).\n",
                (void *)(uintptr_t)current->nsproxy->mnt_ns, (void *)(uintptr_t)pid_ns,
                (void *)(uintptr_t)devdrv_manager_container_get_pid_ns(current));
            return -EINVAL;
        }
    }

    *nr = task_tgid_nr(main_tsk);
    *vnr = task_pid_vnr(main_tsk);
    put_task_struct(main_tsk);

    return 0;
}

STATIC struct devdrv_container_item *devdrv_manager_container_item_init(
    struct devdrv_container_table *dev_container_table, struct devdrv_manager_container_para *para)
{
    struct devdrv_container_item *item = NULL;
    struct task_struct *task = NULL;
    pid_t main_proc, vnr;
    u32 i;

    if (devdrv_manager_container_find_main_proc(&main_proc, &vnr)) {
        devdrv_drv_err_spinlock("find_main_proc failed.\n");
        return NULL;
    }

    task = get_pid_task(find_pid_ns(main_proc, &init_pid_ns), PIDTYPE_PID);
    if (task == NULL) {
        devdrv_drv_err_spinlock("get task is NULL, main_proc(%d), vnr(%d)"
            "init_pid_ns(0x%pK) mnt_ns(0x%pK) pid_ns(0x%pK)\n",
            main_proc, vnr, (void *)(uintptr_t)&init_pid_ns,
            (void *)(uintptr_t)para->mnt_ns, (void *)(uintptr_t)para->pid_ns);
        return NULL;
    }

    item = kzalloc(sizeof(struct devdrv_container_item), GFP_ATOMIC | __GFP_ACCOUNT);
    if (item == NULL) {
        put_task_struct(task);
        devdrv_drv_err_spinlock("item kzalloc failed\n");
        return NULL;
    }

    for (i = 0; i < DEVDRV_CONTIANER_NUM_OF_LONG; i++) {
        item->dev_list_map[i] = 0;
    }
    item->dev_num = 0;
    item->conflict_flag = DEVMNG_CONTAINER_DEVICE_NORMAL;
    item->mnt_ns = para->mnt_ns;
    item->pid_ns = para->pid_ns;
    item->main_pid.tgid = main_proc;
    item->main_pid.start_time = task->start_time;
    item->main_pid.real_start_time = get_start_time(task);
    item->container_id = para->container_id;

    put_task_struct(task);

    spin_lock_init(&item->spinlock);
    list_add_tail(&item->item_list_node, &dev_container_table->item_list_head);

    return item;
}

STATIC int devdrv_manager_container_init_docker_id(struct devdrv_container_item *item)
{
    u32 i;

    for (i = 0; i < MAX_DOCKER_NUM; i++) {
        if (docker_status[i] == DOCKER_STATUS_IDLE) {
            item->id = i;
            docker_status[i] = DOCKER_STATUS_USED;
            break;
        }
    }

    if (i >= MAX_DOCKER_NUM) {
        devdrv_drv_err_spinlock("no idle docker, i = %d\n", i);
        return -EBUSY;
    }
    return 0;
}

STATIC void devdrv_manager_container_uninit_docker_id(struct devdrv_container_item *item)
{
    docker_status[item->id] = DOCKER_STATUS_IDLE;
    item->id = MAX_DOCKER_NUM;
}

STATIC void devdrv_manager_container_item_free(struct devdrv_container_item *item)
{
    u32 i;
    devdrv_manager_container_uninit_docker_id(item);

    for (i = 0; i < item->dev_num; i++) {
        if (dev_mnt_vdevice_add_inform(item->dev_id[i], VDEV_UNBIND, NULL, 0)) {
            devdrv_drv_err_spinlock("Inform vdevice unbind failed. (dev_id = %u)\n", item->dev_id[i]);
        }
    }

    list_del(&item->item_list_node);
    kfree(item);
    item = NULL;
}

STATIC bool devdrv_manager_container_check_main_proc_timestamp(struct devdrv_container_item *item)
{
    struct task_struct *tsk = NULL;

    tsk = get_pid_task(find_pid_ns(item->main_pid.tgid, &init_pid_ns), PIDTYPE_PID);
    if (tsk == NULL) {
        return true;
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)
    if (item->main_pid.start_time == tsk->start_time) {
        put_task_struct(tsk);
        return false;
    }
#else
    if (timespec_equal(&item->main_pid.start_time, &tsk->start_time)) {
        put_task_struct(tsk);
        return false;
    }
#endif
    put_task_struct(tsk);

    return true;
}
/* delete invaild item */
STATIC bool devdrv_manager_container_check_and_free_item(struct devdrv_container_item *item)
{
    if (item->mnt_ns == init_task.nsproxy->mnt_ns) {
        /* Host mnt_ns not need check and remover here. */
        return false;
    }
    if (devdrv_manager_container_check_main_proc_timestamp(item)) {
        devdrv_manager_container_item_free(item);
        return true;
    }
    return false;
}

STATIC void devdrv_manager_container_check_and_free_table(struct devdrv_container_table *dev_container_table)
{
    struct devdrv_container_item *item = NULL;
    struct list_head *pos = NULL, *n = NULL;
    if (!list_empty_careful(&dev_container_table->item_list_head)) {
        list_for_each_safe(pos, n, &dev_container_table->item_list_head) {
            item = list_entry(pos, struct devdrv_container_item, item_list_node);
            (void)devdrv_manager_container_check_and_free_item(item);
        }
    }
    return;
}

int devdrv_manager_container_check_refresh_table(void)
{
    struct devdrv_container_table *dev_container_table = NULL;
    struct devdrv_manager_info *manager_info = NULL;
    manager_info = devdrv_get_manager_info();
    if (manager_info == NULL) {
        devdrv_drv_err("manager_info is NULL.\n");
        return -EINVAL;
    }
    dev_container_table = manager_info->container_table;
    if (dev_container_table == NULL) {
        devdrv_drv_err("container table is NULL\n");
        return -ENOMEM;
    }
    spin_lock(&dev_container_table->spinlock);
    devdrv_manager_container_check_and_free_table(dev_container_table);
    spin_unlock(&dev_container_table->spinlock);
    dev_mnt_vdevice_inform();
    return 0;
};

STATIC struct devdrv_container_item *devdrv_manager_container_find_item(
    struct devdrv_container_table *dev_container_table, struct mnt_namespace *mnt_ns)
{
    struct devdrv_container_item *item = NULL;
    struct list_head *pos = NULL, *n = NULL;

    if (!list_empty_careful(&dev_container_table->item_list_head)) {
        list_for_each_safe(pos, n, &dev_container_table->item_list_head) {
            item = list_entry(pos, struct devdrv_container_item, item_list_node);
            if (item->mnt_ns == mnt_ns) {
                return item;
            }
        }
    }

    return NULL;
}

int devdrv_manager_set_container_id(struct devdrv_container_table *dev_container_table)
{
    struct mnt_namespace *current_mnt_ns = NULL;
    struct devdrv_container_item *item = NULL;
    u64 container_id = 0;
    int has_not_init;

    if (dev_container_table == NULL) {
        devdrv_drv_err("container table is NULL\n");
        return -EINVAL;
    }

    if (devdrv_manager_container_is_in_container()) {
        current_mnt_ns = current->nsproxy->mnt_ns;
    } else {
        /*
         * Operation not permitted: the env is not docker.
         * there is no need to print to prevent too many prints, just need to return a clear error code.
         */
        return -EPERM;
    }

    spin_lock(&dev_container_table->spinlock);
    item = devdrv_manager_container_find_item(dev_container_table, current_mnt_ns);
    if (item == NULL) {
        spin_unlock(&dev_container_table->spinlock);
        devdrv_drv_err("cannot find item in container table.\n");
        return -EINVAL;
    }
    has_not_init = (item->container_id == 0);
    spin_unlock(&dev_container_table->spinlock);

    if (has_not_init) {
        devdrv_manager_get_container_id(&container_id);
        spin_lock(&dev_container_table->spinlock);
        item = devdrv_manager_container_find_item(dev_container_table, current_mnt_ns);
        if (item != NULL) {
            item->container_id = container_id;
        }

        spin_unlock(&dev_container_table->spinlock);
    }

    return 0;
}

STATIC void devdrv_manager_container_set_item_conflict(
    struct devdrv_container_table *dev_container_table, struct mnt_namespace *mnt_ns)
{
    struct devdrv_container_item *item = NULL;
    struct list_head *pos = NULL, *n = NULL;
    u32 i;

    if (list_empty_careful(&dev_container_table->item_list_head)) {
        return;
    }

    list_for_each_safe(pos, n, &dev_container_table->item_list_head) {
        item = list_entry(pos, struct devdrv_container_item, item_list_node);
        if (item->mnt_ns == mnt_ns) {
            item->conflict_flag = DEVMNG_CONTAINER_DEVICE_CONFLICT;
            item->dev_num = 0;
            for (i = 0; i <  DEVDRV_MAX_DAVINCI_NUM; i++) {
                item->dev_id[i] = DEVDRV_MAX_DAVINCI_NUM;
            }
            for (i = 0; i < DEVDRV_CONTIANER_NUM_OF_LONG; i++) {
                item->dev_list_map[i] = 0x0;
            }
            return;
        }
    }
}
/* 设置当前item的conflict标志 */
STATIC bool devdrv_manager_container_devid_check(struct devdrv_container_table *dev_container_table,
    struct devdrv_manager_container_para *para)
{
    struct devdrv_container_item *item = NULL;
    struct list_head *pos = NULL;
    struct list_head *n = NULL;
    u32 i, j;

    if (para->mnt_ns == devdrv_manager_get_host_mnt_ns()) {
        return true;
    }

    if (list_empty_careful(&dev_container_table->item_list_head)) {
        return false;
    }

    list_for_each_safe(pos, n, &dev_container_table->item_list_head) {
        item = list_entry(pos, struct devdrv_container_item, item_list_node);
        if ((item->mnt_ns == devdrv_manager_get_host_mnt_ns()) ||
            (item->mnt_ns == para->mnt_ns)) {
            continue;
        }
        /* 如果冲突，则不需要根据此item设置当前item的冲突标志 */
        if (item->conflict_flag == DEVMNG_CONTAINER_DEVICE_CONFLICT) {
            continue;
        }
        for (i = 0; i < item->dev_num; i++) {
            for (j = 0; j < para->dev_num; j++) {
                if (item->dev_id[i] == para->dev_id[j]) {
#ifdef CFG_FEATURE_DEVICE_SHARE
                    if (get_device_share_flag(item->dev_id[i]) != DEVICE_UNSHARE) {
                        continue;
                    }
#endif
                    devdrv_manager_container_set_item_conflict(dev_container_table, para->mnt_ns);
                    devdrv_drv_err_spinlock("device[%u] = %u is in used, "
                            "item_mntns(0x%pK) current_mntns(0x%pK)\n",
                            i, item->dev_id[i], (void *)(uintptr_t)item->mnt_ns,
                            (void *)(uintptr_t)current->nsproxy->mnt_ns);
                    return false;
                }
            }
        }
    }

    return true;
}

STATIC int devdrv_manager_container_table_add_new_item(struct devdrv_container_table *dev_container_table,
    struct devdrv_manager_container_para *para)
{
    struct devdrv_container_item *item = NULL;
    int ret;

    item = devdrv_manager_container_item_init(dev_container_table, para);
    if (item == NULL) {
        devdrv_drv_err_spinlock("init item failed\n");
        return -ENOMEM;
    }
    ret = devdrv_manager_container_init_docker_id(item);
    if (ret) {
        devdrv_manager_container_item_free(item);
        devdrv_drv_err_spinlock("init docker id failed, ret = %d\n", ret);
        return ret;
    }

    return 0;
}

STATIC inline int devdrv_manager_container_is_item_conflict(struct devdrv_container_item *item)
{
    if (item->conflict_flag != DEVMNG_CONTAINER_DEVICE_NORMAL) {
        devdrv_drv_err_spinlock("invaild item id(%u) dev num (%u) dev map (0x%llx) mnt_ns(0x%pK)"
            " pid_ns(0x%pK) main_proc(%d) conflict_flag(%u)\n", item->id,
            item->dev_num, item->dev_list_map[0], (void *)(uintptr_t)item->mnt_ns,
            (void *)(uintptr_t)item->pid_ns, item->main_pid.tgid, item->conflict_flag);
        return -EBUSY;
    }
    return 0;
}

STATIC void devdrv_manager_container_refresh_devid_list(struct devdrv_container_item *item)
{
    u32 idx, offset, i;
    u32 dev_num = 0;
    u32 pf_num = 0;
    u32 vf_num = 0;

    for (i = 0; i < DEVDRV_MAX_DAVINCI_NUM; i++) {
        item->dev_id[i] = VDAVINCI_MAX_VDEV_ID + 1;
        if (i < DEVDRV_PF_DEV_MAX_NUM) {
            item->pf_devid[i] = VDAVINCI_MAX_VDEV_ID + 1;
        }
        if (i < MAX_VDEV_NUM) {
            item->vf_devid[i] = VDAVINCI_MAX_VDEV_ID + 1;
        }
    }

    for (idx = 0, offset = 0; offset < BIT_NUM_U64; offset++) {
        if ((item->dev_list_map[idx] & (0x1ULL << offset)) &&
            (dev_num < DEVDRV_MAX_DAVINCI_NUM)) {
            item->pf_devid[pf_num] = offset;
            pf_num++;
            item->dev_id[dev_num] = offset;
            dev_num++;
        }
    }
    for (idx = 1; idx < DEVDRV_CONTIANER_NUM_OF_LONG; idx++) {
        for (offset = 0; offset < BIT_NUM_U64; offset++) {
            if ((item->dev_list_map[idx] & (0x1ULL << offset)) &&
                (dev_num < DEVDRV_MAX_DAVINCI_NUM)) {
                item->vf_devid[vf_num] = BIT_NUM_U64 * idx + offset;
                vf_num++;
                item->dev_id[dev_num] = BIT_NUM_U64 * idx + offset;
                dev_num++;
            }
        }
    }
    item->dev_num = dev_num;
    item->pf_num = pf_num;
    item->vf_num = vf_num;
}

STATIC int devdrv_manager_container_table_add_exist_item(struct devdrv_container_item *item,
    struct devdrv_manager_container_para *para)
{
    u32 idx, offset, i;

    if (para->dev_num > DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err_spinlock("invaild dev_num = %u\n", para->dev_num);
        return -EINVAL;
    }

    for (i = 0; i < para->dev_num; i++) {
        if (para->dev_id[i] > VDAVINCI_MAX_VDEV_ID) {
            devdrv_drv_err_spinlock("invaild deviceid, dev_id[%u] = %u\n", i, para->dev_id[i]);
            return -EINVAL;
        }
        idx = para->dev_id[i] / BIT_NUM_U64;
        offset = para->dev_id[i] % BIT_NUM_U64;

        item->dev_list_map[idx] |= (0x01ULL << offset);
        devdrv_manager_container_refresh_devid_list(item);
        if (dev_mnt_vdevice_add_inform(para->dev_id[i], VDEV_BIND,
            para->mnt_ns, item->container_id)) {
            devdrv_drv_err_spinlock("Inform vdevice bind failed. (dev_id = %u)\n", para->dev_id[i]);
            return -EINVAL;
        }
    }
    return 0;
}

STATIC int devdrv_manager_container_table_add_devlist_to_item(
    struct devdrv_container_table *dev_container_table, struct devdrv_manager_container_para *para)
{
    struct devdrv_container_item *item = NULL;

    item = devdrv_manager_container_find_item(dev_container_table, para->mnt_ns);
    if (item == NULL) {
        devdrv_drv_err_spinlock("find item failed, para->mnt_ns = 0x%pK\n", (void *)(uintptr_t)para->mnt_ns);
        return -EINVAL;
    }

    return devdrv_manager_container_table_add_exist_item(item, para);
}

/* 恢复当前item的冲突位 */
STATIC void devdrv_manager_container_recover_conflict(
    struct devdrv_container_table *dev_container_table, struct devdrv_manager_container_para *para)
{
    struct devdrv_container_item *item = NULL;
    struct devdrv_container_item *cur_item = NULL;
    struct list_head *pos = NULL;
    struct list_head *n = NULL;
    u32 i;

    cur_item = devdrv_manager_container_find_item(dev_container_table, para->mnt_ns);
    if (cur_item->conflict_flag != DEVMNG_CONTAINER_DEVICE_CONFLICT) {
        return;
    }
    list_for_each_safe(pos, n, &dev_container_table->item_list_head) {
        item = list_entry(pos, struct devdrv_container_item, item_list_node);
        if ((item->mnt_ns == devdrv_manager_get_host_mnt_ns()) ||
            (item->conflict_flag == DEVMNG_CONTAINER_DEVICE_CONFLICT)) {
            continue;
        }
        /* 通过list_map判断, 当前item中的device是否被占用 */
        for (i = 0; i < DEVDRV_CONTIANER_NUM_OF_LONG; i++) {
            if ((cur_item->dev_list_map[i] & item->dev_list_map[i]) != 0) {
                return;
            }
        }
    }
    cur_item->conflict_flag = DEVMNG_CONTAINER_DEVICE_NORMAL;
    devdrv_drv_err_spinlock("Devices in docker is not conflict. (docker_id=%d)\n", cur_item->id);
    return;
}
/* 1.删除无效容器 2.找到当前容器对应的item，找不到则添加item节点 3.解当前item的冲突标志 */
STATIC int devdrv_manager_container_table_overlap_item(
    struct devdrv_container_table *dev_container_table, struct devdrv_manager_container_para *para)
{
    struct devdrv_container_item *item = NULL;

    devdrv_manager_container_check_and_free_table(dev_container_table);
    item = devdrv_manager_container_find_item(dev_container_table, para->mnt_ns);
    if (item == NULL) {
        return devdrv_manager_container_table_add_new_item(dev_container_table, para);
    }

    devdrv_manager_container_recover_conflict(dev_container_table, para);

    return 0;
}

int devdrv_manager_container_table_devlist_add_ns(u32 *devlist, u32 dev_num,
    struct mnt_namespace *mnt_ns)
{
    struct devdrv_container_table *dev_container_table = NULL;
    struct devdrv_manager_info *manager_info = NULL;
    struct devdrv_manager_container_para *para = NULL;
    int ret, ret_add_devlist;
    u32 i;

    /* privileged container not need to create item */
    if (devdrv_manager_container_is_admin() &&
        mnt_ns != devdrv_manager_get_host_mnt_ns()) {
        return 0;
    }

    manager_info = devdrv_get_manager_info();
    if (manager_info == NULL || dev_num > DEVDRV_MAX_DAVINCI_NUM || devlist == NULL) {
        devdrv_drv_err("manager_info = %pK, devlist = %pK, dev_num = %u\n",
                       manager_info, devlist, dev_num);
        return -EINVAL;
    }

    para = (struct devdrv_manager_container_para *)kzalloc(sizeof(struct devdrv_manager_container_para),
        GFP_KERNEL | __GFP_ACCOUNT);
    if (para == NULL) {
        devdrv_drv_err("Alloc memory for container parameter failed.\n");
        return -ENOMEM;
    }

    for (i = 0; i < dev_num; i++) {
        para->dev_id[i] = devlist[i];
    }

    for (i = dev_num; i < DEVDRV_MAX_DAVINCI_NUM; i++) {
        para->dev_id[i] = 0;
    }

    para->container_id = 0;
    para->dev_num = dev_num;
    para->mnt_ns = mnt_ns;
    para->start_time = current->start_time;
    para->real_start_time = get_start_time(current);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0)
    para->pid_ns = current->nsproxy->pid_ns_for_children;
#else
    para->pid_ns = current->nsproxy->pid_ns;
#endif
    dev_container_table = manager_info->container_table;
    if (dev_container_table == NULL) {
        devdrv_drv_err("container table is NULL\n");
        ret = -ENOMEM;
        goto FREE_EXIT;
    }
    spin_lock(&dev_container_table->spinlock);
    ret = devdrv_manager_container_table_overlap_item(dev_container_table, para);
    if (ret) {
        devdrv_manager_container_print_list(dev_container_table);
        spin_unlock(&dev_container_table->spinlock);
        devdrv_drv_err("add devlist to item failed, ret = %d\n", ret);
        ret = -ENODEV;
        goto FREE_EXIT;
    }
    if (!devdrv_manager_container_devid_check(dev_container_table, para)) {
        devdrv_drv_err_spinlock("device is in used\n");
        ret = -EBUSY;
    }
    ret_add_devlist = devdrv_manager_container_table_add_devlist_to_item(dev_container_table, para);
    if (ret_add_devlist) {
        devdrv_manager_container_print_list(dev_container_table);
        spin_unlock(&dev_container_table->spinlock);
        devdrv_drv_err("add devlist to item failed, ret = %d\n", ret);
        ret = -ENODEV;
        goto FREE_EXIT;
    }
    spin_unlock(&dev_container_table->spinlock);
    dev_mnt_vdevice_inform();

FREE_EXIT:
    kfree(para);
    para = NULL;
    return ret;
}
EXPORT_SYMBOL(devdrv_manager_container_table_devlist_add_ns);

STATIC int devdrv_manager_container_table_del_exist_item(struct devdrv_container_item *item,
    struct devdrv_manager_container_para *para)
{
    u32 idx, offset;
    u32 i;

    for (i = 0; i < para->dev_num; i++) {
        if (para->dev_id[i] > VDAVINCI_MAX_VDEV_ID) {
            devdrv_drv_err_spinlock("invaild deviceid, dev_id[%u] = %u\n", i, para->dev_id[i]);
            return -EINVAL;
        }
        idx = para->dev_id[i] / BIT_NUM_U64;
        offset = para->dev_id[i] % BIT_NUM_U64;
        if ((item->dev_list_map[idx] & (0x01ULL << offset)) == 0) {
            devdrv_drv_err_spinlock("exist failed, dev_id = %u\n", para->dev_id[i]);
            return -ENODEV;
        }
        item->dev_list_map[idx] &= ~(0x01ULL << offset);
    }
    devdrv_manager_container_refresh_devid_list(item);

    return 0;
}

STATIC int devdrv_manager_container_table_del_devlist_to_item(
    struct devdrv_container_table *dev_container_table, struct devdrv_manager_container_para *para)
{
    struct devdrv_container_item *item = NULL;

    item = devdrv_manager_container_find_item(dev_container_table, para->mnt_ns);
    if (item == NULL) {
        return -EINVAL;
    }

    return devdrv_manager_container_table_del_exist_item(item, para);
}

int devdrv_manager_container_table_devlist_del_ns(u32 *physical_devlist, u32 physical_dev_num,
    struct mnt_namespace *mnt_ns)
{
    struct devdrv_container_table *dev_container_table = NULL;
    struct devdrv_manager_container_para *para = NULL;
    struct devdrv_manager_info *manager_info = NULL;
    int ret;
    u32 i;

    manager_info = devdrv_get_manager_info();
    if (manager_info == NULL || physical_dev_num > DEVDRV_MAX_DAVINCI_NUM || physical_devlist == NULL) {
        devdrv_drv_err("manager_info = %pK, physical_devlist = %pK, physical_dev_num = %u\n",
                       manager_info, physical_devlist, physical_dev_num);
        return -EINVAL;
    }

    para = (struct devdrv_manager_container_para *)kzalloc(sizeof(struct devdrv_manager_container_para),
        GFP_KERNEL | __GFP_ACCOUNT);
    if (para == NULL) {
        devdrv_drv_err("Alloc memory for container parameter failed.\n");
        return -ENOMEM;
    }

    for (i = 0; i < physical_dev_num; i++) {
        para->dev_id[i] = physical_devlist[i];
    }
    para->dev_num = physical_dev_num;
    para->mnt_ns = mnt_ns;
    para->start_time = current->start_time;
    para->real_start_time = get_start_time(current);

    dev_container_table = manager_info->container_table;
    if (dev_container_table == NULL) {
        devdrv_drv_err("container table is NULL\n");
        kfree(para);
        para = NULL;
        return -ENOMEM;
    }
    spin_lock(&dev_container_table->spinlock);
    ret = devdrv_manager_container_table_del_devlist_to_item(dev_container_table, para);
    if (ret) {
        spin_unlock(&dev_container_table->spinlock);
        kfree(para);
        para = NULL;
        devdrv_drv_err("add devlist to item failed, ret = %d\n", ret);
        return -ENODEV;
    }
    spin_unlock(&dev_container_table->spinlock);
    kfree(para);
    para = NULL;
    return 0;
}
#endif // CFG_FEATURE_OLD_DEVID_TRANS

#ifdef CFG_FEATURE_OLD_DEVID_TRANS
int devdrv_manager_container_table_init(struct devdrv_manager_info *manager_info)
{
    struct devdrv_container_table *dev_container_table = NULL;
    struct devdrv_manager_container_para *para = NULL;
    struct devdrv_container_item *item = NULL;

    if (manager_info == NULL) {
        devdrv_drv_err("manager info or tsk is null\n");
        return -ENOMEM;
    }

    dev_container_table = kzalloc(sizeof(struct devdrv_container_table), GFP_KERNEL | __GFP_ACCOUNT);
    if (dev_container_table == NULL) {
        devdrv_drv_err("container_table kzalloc failed\n");
        return -ENOMEM;
    }

    para = (struct devdrv_manager_container_para *)kzalloc(sizeof(struct devdrv_manager_container_para),
        GFP_KERNEL | __GFP_ACCOUNT);
    if (para == NULL) {
        devdrv_drv_err("Alloc memory for container parameter failed.\n");
        kfree(dev_container_table);
        dev_container_table = NULL;
        return -ENOMEM;
    }

    INIT_LIST_HEAD(&dev_container_table->item_list_head);
    spin_lock_init(&dev_container_table->spinlock);

    para->mnt_ns = devdrv_manager_get_host_mnt_ns();
    para->pid_ns = &init_pid_ns;
    para->container_id = 0;
    devdrv_drv_info("container table init: host_mnt_ns(0x%pK) host_pid_ns(0x%pK)\n",
                    (void *)(uintptr_t)para->mnt_ns, (void *)(uintptr_t)&init_pid_ns);

    /* create host item */
    spin_lock(&dev_container_table->spinlock);
    item = devdrv_manager_container_item_init(dev_container_table, para);
    if (item == NULL) {
        spin_unlock(&dev_container_table->spinlock);
        devdrv_drv_err("create host item failed\n");
        kfree(dev_container_table);
        dev_container_table = NULL;
        kfree(para);
        para = NULL;
        return -ENODEV;
    }
    spin_unlock(&dev_container_table->spinlock);
    kfree(para);
    para = NULL;
    manager_info->container_table = dev_container_table;
    return 0;
}

void devdrv_manager_container_table_exit(struct devdrv_manager_info *manager_info)
{
    struct devdrv_container_table *dev_container_table = NULL;
    struct list_head *pos_item = NULL, *n_item = NULL;
    struct devdrv_container_item *item = NULL;

    if (manager_info == NULL || manager_info->container_table == NULL) {
        devdrv_drv_err("invalid parameter, dev_manager_info = %pK\n", manager_info);
        return;
    }
    dev_container_table = manager_info->container_table;

    spin_lock(&dev_container_table->spinlock);
    if (!list_empty_careful(&dev_container_table->item_list_head)) {
        list_for_each_safe(pos_item, n_item, &dev_container_table->item_list_head)
        {
            item = list_entry(pos_item, struct devdrv_container_item, item_list_node);
            devdrv_manager_container_item_free(item);
        }
    }
    spin_unlock(&dev_container_table->spinlock);

    kfree(manager_info->container_table);
    manager_info->container_table = NULL;
}
#else
#define DMS_MNG_NOTIFIER "dms_mng"
static int dms_mng_notifier_func(u32 udevid, enum uda_notified_action action)
{
    int ret = 0;

    if (action != UDA_UNOCCUPY && devdrv_manager_container_is_in_container() == false) {
        return 0;
    }

    if (action == UDA_OCCUPY) {
        u64 container_id;
        devdrv_manager_get_container_id(&container_id);
        ret = uda_set_dev_ns_identify(udevid, container_id);
        if (!uda_is_phy_dev(udevid)) {
            (void)dev_mnt_vdevice_add_inform(udevid, VDEV_BIND, current->nsproxy->mnt_ns, container_id);
            dev_mnt_vdevice_inform();
        }
    } else if (action == UDA_UNOCCUPY) {
        if (!uda_is_phy_dev(udevid)) {
            (void)dev_mnt_vdevice_add_inform(udevid, VDEV_UNBIND, NULL, 0);
            dev_mnt_vdevice_inform();
        }
    }

    devdrv_drv_info("notifier action. (udevid=%u; action=%d; ret=%d)\n", udevid, action, ret);

    return ret;
}

static int dms_mng_init_uda_notifier(void)
{
    struct uda_dev_type type;
    int ret;

    uda_davinci_local_real_entity_type_pack(&type); /* for device */
    ret = uda_real_virtual_notifier_register(DMS_MNG_NOTIFIER, &type, UDA_PRI1, dms_mng_notifier_func);
    if (ret) {
        devdrv_drv_err("uda_real_virtual_notifier_register failed\n");
        return ret;
    }

    uda_davinci_near_real_entity_type_pack(&type); /* for host */
    ret = uda_real_virtual_notifier_register(DMS_MNG_NOTIFIER, &type, UDA_PRI1, dms_mng_notifier_func);
    if (ret) {
        devdrv_drv_err("uda_real_virtual_notifier_register failed\n");
        return ret;
    }

    return 0;
}

static void dms_mng_uninit_uda_notifier(void)
{
    struct uda_dev_type type;

    uda_davinci_local_real_entity_type_pack(&type);
    (void)uda_real_virtual_notifier_unregister(DMS_MNG_NOTIFIER, &type);
    uda_davinci_near_real_entity_type_pack(&type);
    (void)uda_real_virtual_notifier_unregister(DMS_MNG_NOTIFIER, &type);
}

int devdrv_manager_container_table_init(struct devdrv_manager_info *manager_info)
{
    return dms_mng_init_uda_notifier();
}

void devdrv_manager_container_table_exit(struct devdrv_manager_info *manager_info)
{
    dms_mng_uninit_uda_notifier();
}
#endif // CFG_FEATURE_OLD_DEVID_TRANS

STATIC int devdrv_manager_container_get_bare_pid(struct devdrv_container_para *cmd)
{
    pid_t pid;
    int ret;

    if (cmd->para.out == NULL) {
        devdrv_drv_err("invalid parameter\n");
        return -EINVAL;
    }
    pid = current->pid;
    ret = copy_to_user_safe(cmd->para.out, &pid, sizeof(pid_t));
    if (ret) {
        devdrv_drv_err("pid = %d, copy to user failed: %d.\n", pid, ret);
        return -ENOMEM;
    }

    return 0;
}

STATIC int devdrv_manager_container_get_bare_tgid(struct devdrv_container_para *cmd)
{
    pid_t tgid;
    int ret;

    if (cmd->para.out == NULL) {
        devdrv_drv_err("invalid parameter\n");
        return -EINVAL;
    }
    tgid = current->tgid;
    ret = copy_to_user_safe(cmd->para.out, &tgid, sizeof(pid_t));
    if (ret) {
        devdrv_drv_err("tgid = %d, copy to user failed: %d.\n", tgid, ret);
        return -ENOMEM;
    }

    return 0;
}

#ifdef CFG_FEATURE_OLD_DEVID_TRANS
int devdrv_manager_container_table_overlap(struct devdrv_manager_context *dev_manager_context, u32 *docker_id)
{
    struct devdrv_container_table *dev_container_table = NULL;
    struct devdrv_manager_container_para *para = NULL;
    int ret;

    if (devdrv_manager_container_is_host_system(dev_manager_context->mnt_ns) || devdrv_manager_container_is_admin()) {
        *docker_id = MAX_DOCKER_NUM;
        return 0;
    }

    dev_container_table = dev_manager_info->container_table;
    if (dev_container_table == NULL) {
        devdrv_drv_err("container table is NULL\n");
        return -ENOMEM;
    }

    para = (struct devdrv_manager_container_para *)kzalloc(sizeof(struct devdrv_manager_container_para),
        GFP_KERNEL | __GFP_ACCOUNT);
    if (para == NULL) {
        devdrv_drv_err("Alloc memory for container parameter failed.\n");
        return -ENOMEM;
    }

    para->mnt_ns = dev_manager_context->mnt_ns;
    para->pid_ns = dev_manager_context->pid_ns;
    para->start_time = dev_manager_context->start_time;
    para->real_start_time = dev_manager_context->real_start_time;
    para->container_id = 0;

    spin_lock(&dev_container_table->spinlock);
    ret = devdrv_manager_container_table_overlap_item(dev_container_table, para);
    if (ret) {
        devdrv_manager_container_print_list(dev_container_table);
        spin_unlock(&dev_container_table->spinlock);
        kfree(para);
        para = NULL;
        devdrv_drv_err("add devlist to item failed, ret = %d\n", ret);
        return -ENODEV;
    }
    spin_unlock(&dev_container_table->spinlock);
    kfree(para);
    para = NULL;
    dev_mnt_vdevice_inform();

    ret = devdrv_manager_container_get_docker_id(docker_id);
    if (ret) {
        devdrv_drv_err("container get docker_id failed, ret(%d).\n", ret);
        return -EINVAL;
    }
    ret = devdrv_manager_set_container_id(dev_container_table);
    if (ret) {
        return -EINVAL;
    }

    return 0;
}

#if ((defined CFG_HOST_ENV) || (defined CFG_FEATURE_DEVICE_CONTAINER))
STATIC u32 devdrv_manager_container_get_cdev_id_by_logical_id(
    struct devdrv_container_table *dev_container_table,
    struct devdrv_manager_container_para *para, u32 logical_dev_id)
{
    u32 dev_id = DEVDRV_MAX_DAVINCI_NUM;
    struct mnt_namespace *mnt_ns = para->mnt_ns;
    struct devdrv_container_item *item = NULL;
    u32 idx, offset;
    u32 dev_index = 0;

    spin_lock(&dev_container_table->spinlock);
    item = devdrv_manager_container_find_item(dev_container_table, mnt_ns);
    if (item == NULL) {
        spin_unlock(&dev_container_table->spinlock);
        devdrv_drv_err("find item failed, mnt_ns = 0x%pK\n", (void *)(uintptr_t)mnt_ns);
        return VDAVINCI_MAX_VDEV_ID + 1;
    }

    if (logical_dev_id >= VDAVINCI_VDEV_OFFSET) {
        if (item->dev_list_map[logical_dev_id / BITS_PER_LONG_LONG] &
            (0x1ULL << (logical_dev_id % BITS_PER_LONG_LONG))) {
            dev_id = logical_dev_id;
        }
        spin_unlock(&dev_container_table->spinlock);
        return dev_id;
    }

    for (idx = 0; idx < DEVDRV_CONTIANER_NUM_OF_LONG; idx++) {
        for (offset = 0; offset < BIT_NUM_U64; offset++) {
            if ((item->dev_list_map[idx] & (0x1ULL << offset)) != 0) {
                if (dev_index == logical_dev_id) {
                    dev_id = idx * BIT_NUM_U64 + offset;
                    spin_unlock(&dev_container_table->spinlock);
                    return dev_id;
                }
                dev_index++;
            }
        }
    }
    spin_unlock(&dev_container_table->spinlock);

    return dev_id;
}

STATIC int devdrv_manager_container_logical_id_check(u32 logical_dev_id)
{
    u32 num_dev;

    if ((logical_dev_id >= DEVDRV_MAX_DAVINCI_NUM) || (dev_manager_info == NULL) ||
        (dev_manager_info->container_table == NULL)) {
        devdrv_drv_err("Invalid parameter. (logical_dev_id=%u; dev_manager_info=%u)\n",
            logical_dev_id, ((void *)(uintptr_t)dev_manager_info == NULL));
        return -EINVAL;
    }

    if (devdrv_manager_container_task_struct_check(current)) {
        devdrv_drv_err("invalid current pointer.\n");
        return -EINVAL;
    }

    if (logical_dev_id >= VDAVINCI_VDEV_OFFSET) {
        if ((current->nsproxy->mnt_ns != devdrv_manager_get_host_mnt_ns()) && (!devdrv_manager_container_is_admin())) {
            devdrv_drv_err("The logical ID is incorrect. (logical_dev_id=%u)\n", logical_dev_id);
            return -ENODEV;
        }
    } else {
        num_dev = devdrv_manager_container_get_devnum(current->nsproxy->mnt_ns, DEVDRV_PF_TYPE);
        if (logical_dev_id >= num_dev) {
            devdrv_drv_err("logic dev id is bigger than total. (logical_id=%u; num=%u)\n", logical_dev_id, num_dev);
            return -ENODEV;
        }
    }

    return 0;
}
#endif

STATIC int devdrv_manager_container_get_devlist_host(u32 *devlist, u32 devlist_len, u32 *devnum, u32 dev_type)
{
    struct devdrv_info *dev_info = NULL;
    u32 dev_num_tmp;
    u32 j = 0;
    u32 i;

    dev_num_tmp = devlist_len > DEVDRV_MAX_DAVINCI_NUM ? DEVDRV_MAX_DAVINCI_NUM : devlist_len;
    if (dev_num_tmp == 0) {
        devdrv_drv_err("wrong devlist length, devlist_len = %u\n", devlist_len);
        return -ENODEV;
    }

    i = ((dev_type == DEVDRV_VF_TYPE) ? VDAVINCI_VDEV_OFFSET : 0);
    for (; i < dev_num_tmp; i++) {
        if ((dev_type == DEVDRV_PF_TYPE) && (i >= DEVDRV_PF_DEV_MAX_NUM)) {
            break;
        }
        dev_info = devdrv_manager_get_devdrv_info(i);
        if (dev_info == NULL) {
            continue;
        }
        devlist[j++] = dev_info->dev_id;
    }
    *devnum = j;
    return 0;
}

STATIC int devdrv_manager_container_parameter_check(u32 *devlist, u32 devlist_len, u32 *devnum,
    struct mnt_namespace *mnt_ns)
{
    if (devlist == NULL || devlist_len == 0 || devlist_len > DEVDRV_MAX_DAVINCI_NUM ||
        devnum == NULL || dev_manager_info == NULL || dev_manager_info->container_table == NULL) {
        devdrv_drv_err("invalid parameter, dev_manager_info = %pK, "
                       "devlist = %pK, devlist_len = %u, devnum = %pK, mnt_ns = %pK\n",
                       dev_manager_info, devlist, devlist_len, devnum, mnt_ns);
        return -EINVAL;
    }

    return 0;
}

int devdrv_manager_container_get_devids(u32 *devlist, u32 devlist_len, u32 *devnum,
    struct mnt_namespace *mnt_ns, u32 dev_type)
{
    struct devdrv_container_table *dev_container_table = NULL;
    struct devdrv_container_item *item = NULL;
    u32 devnum_tmp;
    u32 i;

    if (devdrv_manager_container_parameter_check(devlist, devlist_len, devnum, mnt_ns) != 0) {
        return -EINVAL;
    }

    if (devdrv_manager_container_is_admin()) {
        mnt_ns = devdrv_manager_get_host_mnt_ns();
    }
    if (devdrv_manager_get_host_mnt_ns() == NULL) {
        return devdrv_manager_container_get_devlist_host(devlist, devlist_len, devnum, dev_type);
    }

    devnum_tmp = devdrv_manager_container_get_devnum(mnt_ns, dev_type);
    devnum_tmp = (devlist_len > devnum_tmp ? devnum_tmp : devlist_len);
    dev_container_table = dev_manager_info->container_table;
    spin_lock(&dev_container_table->spinlock);
    item = devdrv_manager_container_find_item(dev_container_table, mnt_ns);
    if (item == NULL) {
        spin_unlock(&dev_container_table->spinlock);
        devdrv_drv_err("item find failed, mnt_ns = 0x%pK\n", (void *)(uintptr_t)mnt_ns);
        return -ENODEV;
    }
    for (i = 0; i < devnum_tmp; i++) {
        if ((mnt_ns == devdrv_manager_get_host_mnt_ns()) && (dev_type == DEVDRV_VF_TYPE)) {
            devlist[i] = item->vf_devid[i];
        } else if ((mnt_ns == devdrv_manager_get_host_mnt_ns()) && (dev_type == DEVDRV_ALL_TYPE)) {
            devlist[i] = ((i < item->pf_num) ? i : item->dev_id[i]);
        } else {
            devlist[i] = i;
        }
    }
    spin_unlock(&dev_container_table->spinlock);

    *devnum = devnum_tmp;
    return 0;
}

STATIC int devdrv_manager_container_get_devlist_ns(struct devdrv_container_alloc_para *para,
    struct mnt_namespace *mnt_ns)
{
    struct devdrv_container_table *dev_container_table = dev_manager_info->container_table;
    struct devdrv_container_item *item = NULL;
    struct uda_mia_dev_para mia_para = {0};
    u32 vfid = 0;
    int ret;
    u32 i;

    if (dev_container_table == NULL) {
        devdrv_drv_err("container table is NULL.\n");
        return -ENOMEM;
    }
    if (devdrv_manager_container_is_admin()) {
        mnt_ns = devdrv_manager_get_host_mnt_ns();
    }

    para->num = 0;
    spin_lock(&dev_container_table->spinlock);
    item = devdrv_manager_container_find_item(dev_container_table, mnt_ns);
    if (item == NULL) {
        spin_unlock(&dev_container_table->spinlock);
        devdrv_drv_err("find item failed, mnt_ns = 0x%pK\n", (void *)(uintptr_t)mnt_ns);
        return -EINVAL;
    }
    if (devdrv_manager_container_is_item_conflict(item) != 0) {
        devdrv_manager_container_print_list(dev_container_table);
        spin_unlock(&dev_container_table->spinlock);
        devdrv_drv_err("some devices are inused by other docker, "
                       "mnt_ns = 0x%pK\n", (void *)(uintptr_t)mnt_ns);
        return -EBUSY;
    }
    if (item->dev_num > DEVDRV_MAX_DAVINCI_NUM) {
        spin_unlock(&dev_container_table->spinlock);
        devdrv_drv_err("too many devices, dev_num(%u), devlist_size(%d)\n",
                       item->dev_num, DEVDRV_MAX_DAVINCI_NUM);
        return -EINVAL;
    }
    for (i = 0; i < item->dev_num; i++) {
        para->vdev_id[i] = item->dev_id[i];
        ret = devmng_get_vdavinci_info(item->dev_id[i], &para->npu_id[i], &vfid);
        if (ret) {
            spin_unlock(&dev_container_table->spinlock);
            devdrv_drv_err("get phy id by vdevid failed, dev_id(%u), ret = %d\n", para->vdev_id[i], ret);
            return -EINVAL;
        }

        if (uda_is_phy_dev(para->npu_id[i])) {
            para->phy_dev_id[i] = para->npu_id[i];
            para->phy_dev_num++;
        } else {
            uda_udevid_to_mia_devid(para->npu_id[i], &mia_para);
            para->phy_dev_id[i] = mia_para.phy_devid;
        }
    }
    para->num = item->dev_num;
    spin_unlock(&dev_container_table->spinlock);

    return 0;
}

int devdrv_manager_container_get_davinci_devlist(struct devdrv_container_para *cmd)
{
    struct devdrv_container_alloc_para *para = NULL;
    int ret;

    if (cmd == NULL || cmd->para.out == NULL) {
        devdrv_drv_err("Invaild cmd. (cmd_is_null=%d).\n", (cmd == NULL));
        return -EINVAL;
    }

    para = (struct devdrv_container_alloc_para *)kzalloc(sizeof(struct devdrv_container_alloc_para),
        GFP_KERNEL | __GFP_ACCOUNT);
    if (para == NULL) {
        devdrv_drv_err("Alloc memory for container parameter failed.\n");
        return -ENOMEM;
    }

    (void)devdrv_manager_get_devnum();
    ret = devdrv_manager_container_get_devlist_ns(para, current->nsproxy->mnt_ns);
    if (ret == -EBUSY) {
        devdrv_drv_err("Get davinci devlist failed, device is busy. (ret=%d)\n", ret);
        kfree(para);
        para = NULL;
        return -EBUSY;
    }
    if (ret) {
        devdrv_drv_err("Get davinci devlist failed. (ret=%d)\n", ret);
        kfree(para);
        para = NULL;
        return -EINVAL;
    }

    if (devdrv_manager_container_is_in_container()) {
        para->is_in_container = true;
    } else {
        para->is_in_container = false;
    }

    ret = copy_to_user_safe(cmd->para.out, para, sizeof(struct devdrv_container_alloc_para));
    if (ret) {
        devdrv_drv_err("Copy_to_user failed. (ret=%d)\n", ret);
        kfree(para);
        para = NULL;
        return -ENOMEM;
    }

    kfree(para);
    para = NULL;
    return 0;
}

u32 devdrv_manager_container_get_devnum_bare(u32 dev_type)
{
    u32 dev_num;
    unsigned long flags;

    spin_lock_irqsave(&dev_manager_info->spinlock, flags);
    if (dev_type == DEVDRV_ALL_TYPE) {
        dev_num = dev_manager_info->num_dev;
    } else {
        dev_num = ((dev_type == DEVDRV_PF_TYPE) ? dev_manager_info->pf_num : dev_manager_info->vf_num);
    }
    spin_unlock_irqrestore(&dev_manager_info->spinlock, flags);

    return dev_num;
}

u32 devdrv_manager_container_get_devnum(struct mnt_namespace *mnt_ns, u32 dev_type)
{
    u32 dev_num;
#if ((defined CFG_HOST_ENV) || (defined CFG_FEATURE_DEVICE_CONTAINER))
    struct devdrv_container_table *dev_container_table = NULL;
    struct devdrv_container_item *item = NULL;

    if (mnt_ns == NULL || dev_manager_info == NULL || dev_manager_info->container_table == NULL) {
        devdrv_drv_err("invalid parameter, mnt_ns = %pK, dev_manager_info = %pK\n", mnt_ns, dev_manager_info);
        return DEVDRV_MAX_DAVINCI_NUM + 1;
    }

    if (devdrv_manager_container_is_admin()) {
        mnt_ns = devdrv_manager_get_host_mnt_ns();
    }

    /* arm centos environment, host_mnt_ns is NULL */
    if (devdrv_manager_get_host_mnt_ns() == NULL) {
        return devdrv_manager_container_get_devnum_bare(dev_type);
    }

    dev_container_table = dev_manager_info->container_table;
    spin_lock(&dev_container_table->spinlock);
    item = devdrv_manager_container_find_item(dev_container_table, mnt_ns);
    if (item == NULL) {
        spin_unlock(&dev_container_table->spinlock);
        devdrv_drv_err("item find failed, if you can search <get devnum succ>on the previous print. "
            "it's ok. mnt_ns = 0x%pK.\n", (void *)(uintptr_t)mnt_ns);
        return DEVDRV_MAX_DAVINCI_NUM + 1;
    }
    if ((mnt_ns == devdrv_manager_get_host_mnt_ns()) && (dev_type != DEVDRV_ALL_TYPE)) {
    #ifdef CFG_FEATURE_VFIO_SOC
        dev_num = devdrv_manager_container_get_devnum_bare(dev_type);
    #else
        dev_num = (dev_type == DEVDRV_PF_TYPE ? item->pf_num : item->vf_num);
    #endif
    } else if ((mnt_ns != devdrv_manager_get_host_mnt_ns()) && (dev_type == DEVDRV_VF_TYPE)) {
        dev_num = 0;
    } else {
        dev_num = item->dev_num;
    }
    spin_unlock(&dev_container_table->spinlock);
#else
    dev_num = devdrv_manager_container_get_devnum_bare(dev_type);
#endif

    return dev_num;
}

int devdrv_manager_container_task_struct_check(struct task_struct *tsk)
{
    if (tsk == NULL) {
        devdrv_drv_err("tsk is NULL.\n");
        return -EINVAL;
    }
    if (tsk->nsproxy == NULL) {
        devdrv_drv_err("tsk->nsproxy is NULL.\n");
        return -ENODEV;
    }
    if (tsk->nsproxy->mnt_ns == NULL) {
        devdrv_drv_err("tsk->nsproxy->mnt_ns is NULL.\n");
        return -EFAULT;
    }
    return 0;
}

int devdrv_manager_container_logical_id_to_cdev_id(u32 logical_dev_id, u32 *cdev_id)
{
#if ((defined CFG_HOST_ENV) || (defined CFG_FEATURE_DEVICE_CONTAINER))
    struct devdrv_container_table *dev_container_table = NULL;
    struct devdrv_manager_container_para *para = NULL;
    int ret;

    if (cdev_id == NULL) {
        devdrv_drv_err("*cdev_id is NULL.\n");
        return -EINVAL;
    }

    ret = devdrv_manager_container_logical_id_check(logical_dev_id);
    if (ret != 0) {
        devdrv_drv_err("Logical device id error. (logical_dev_id=%u; ret=%d)\n", logical_dev_id, ret);
        return ret;
    }

    para = (struct devdrv_manager_container_para *)kzalloc(sizeof(struct devdrv_manager_container_para),
        GFP_KERNEL | __GFP_ACCOUNT);
    if (para == NULL) {
        devdrv_drv_err("Alloc memory for container parameter failed.\n");
        return -ENOMEM;
    }

    dev_container_table = dev_manager_info->container_table;
    para->mnt_ns = current->nsproxy->mnt_ns;
    if (devdrv_manager_container_is_admin()) {
        para->mnt_ns = devdrv_manager_get_host_mnt_ns();
    }
    if (devdrv_manager_get_host_mnt_ns() == NULL) {
        *cdev_id = logical_dev_id;
        kfree(para);
        para = NULL;
        return 0;
    }
    *cdev_id = devdrv_manager_container_get_cdev_id_by_logical_id(dev_container_table, para, logical_dev_id);
    if (*cdev_id > VDAVINCI_MAX_VDEV_ID) {
        kfree(para);
        para = NULL;
        devdrv_drv_err("get davinci dev id failed, logical_id = %u, cdev_id = %u\n",
                       logical_dev_id, *cdev_id);
        return -ENODEV;
    }

    kfree(para);
    para = NULL;
#else
    u32 num_dev;

    if ((cdev_id == NULL) || (logical_dev_id >= DEVDRV_MAX_DAVINCI_NUM)) {
        devdrv_drv_err("Invalid logical id or cdev_id is NULL. (logical_dev_id=%u)\n", logical_dev_id);
        return -EINVAL;
    }

    num_dev = devdrv_manager_container_get_devnum_bare(DEVDRV_PF_TYPE);
    if (logical_dev_id >= DEVDRV_PF_DEV_MAX_NUM) {
        if (devdrv_manager_get_devdrv_info(logical_dev_id) == NULL) {
            devdrv_drv_err("Invalid logical device id. (logical_dev_id=%u)\n", logical_dev_id);
            return -EINVAL;
        }
    } else if (logical_dev_id >= num_dev) {
#ifndef CFG_SOC_PLATFORM_MDC_V11
        devdrv_drv_err("Logic dev id is bigger than device number. (logical_dev_id=%u; dev_num=%u)\n",
            logical_dev_id, num_dev);
        return -ENODEV;
#else
        /* in 1910B dc, there is 1 more remote device, but only 1 local physical devcie can be managered */
        if (logical_dev_id >= num_dev + 1) {
            devdrv_drv_err("Logic dev id is bigger than device number. (logical_dev_id=%u; dev_num=%u)\n",
                logical_dev_id, num_dev);
            return -ENODEV;
        }
#endif
    }

    *cdev_id = logical_dev_id;
#endif
    return 0;
}

int devdrv_manager_container_logical_id_to_physical_id(u32 logical_dev_id, u32 *physical_dev_id, u32 *vfid)
{
    u32 cdev_id;
    int ret;

    ret = devdrv_manager_container_logical_id_to_cdev_id(logical_dev_id, &cdev_id);
    if (ret) {
        devdrv_drv_err("get davinci id by logical id failed, logical_id(%u) ret = %d.\n", logical_dev_id, ret);
        return ret;
    }
    ret = devmng_get_vdavinci_info(cdev_id, physical_dev_id, vfid);
    if (ret) {
        devdrv_drv_err("get physical id by logical id failed, logical_id(%u) ret = %d.\n", logical_dev_id, ret);
        return ret;
    }

    if (*physical_dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("physical_dev_id(%u) is invalid.\n", *physical_dev_id);
        return -ENODEV;
    }

    return 0;
}
EXPORT_SYMBOL_GPL(devdrv_manager_container_logical_id_to_physical_id);
#endif // CFG_FEATURE_OLD_DEVID_TRANS

int devdrv_manager_container_is_in_container(void)
{
    struct mnt_namespace *current_mnt = NULL;
    struct mnt_namespace *host_mnt = NULL;
    int is_in;

    if (devdrv_manager_container_check_current()) {
        return false;
    }

    current_mnt = current->nsproxy->mnt_ns;
    host_mnt = devdrv_manager_get_host_mnt_ns();
    if (host_mnt == NULL) {
        is_in = false;
    } else if ((!devdrv_manager_container_is_admin()) && (current_mnt != host_mnt)) {
        is_in = true;
    } else {
        is_in = false;
    }

    return is_in;
}
EXPORT_SYMBOL_GPL(devdrv_manager_container_is_in_container);

int devdrv_manager_container_is_in_admin_container(void)
{
    if (devdrv_manager_container_check_current()) {
        return false;
    }

    if (devdrv_manager_container_is_host_system(current->nsproxy->mnt_ns)) {
        return false;
    }

    if (devdrv_manager_container_is_admin()) {
        return true;
    }

    return false;
}
EXPORT_SYMBOL(devdrv_manager_container_is_in_admin_container);

#ifdef CFG_FEATURE_OLD_DEVID_TRANS
int devdrv_manager_container_check_devid_in_container(u32 devid, pid_t hostpid)
{
    struct mnt_namespace *mnt_ns = NULL;
    struct task_struct *tsk = NULL;
    struct pid *pgrp = NULL;
    int ret;

    pgrp = find_get_pid(hostpid);
    if (pgrp == NULL) {
        devdrv_drv_err("pgrp is NULL\n");
        return -EINVAL;
    }

    tsk = get_pid_task(pgrp, PIDTYPE_PID);
    ret = devdrv_manager_container_task_struct_check(tsk);
    if (ret) {
        devdrv_drv_err("task_struct_check failed, ret(%d), devid(%u), hostpid(%d)\n",
                       ret, devid, hostpid);
        if (tsk != NULL) {
            put_task_struct(tsk);
        }
        put_pid(pgrp);
        return -EINVAL;
    }
    mnt_ns = tsk->nsproxy->mnt_ns;

    if ((mnt_ns == devdrv_manager_get_host_mnt_ns()) ||
        devdrv_manager_container_is_admin_task(tsk)) {
        put_task_struct(tsk);
        put_pid(pgrp);
        return 0;
    }

    ret = devdrv_manager_container_check_devid_in_container_ns(devid, tsk);
    if (ret) {
        devdrv_drv_err("device phyid %u is not belong to current docker,"
                       " ret(%d), hostpid(%d)\n", devid, ret, hostpid);
    }
    put_task_struct(tsk);
    put_pid(pgrp);

    return ret;
}
EXPORT_SYMBOL(devdrv_manager_container_check_devid_in_container);

int devdrv_manager_container_check_devid_in_container_ns(u32 devid, struct task_struct *tsk)
{
    struct devdrv_container_table *dev_container_table = NULL;
    struct mnt_namespace *mnt_ns = tsk->nsproxy->mnt_ns;
    struct devdrv_container_item *item = NULL;
    u32 idx, offset;
    u32 end_index, i;

    dev_container_table = dev_manager_info->container_table;
    if (devdrv_manager_container_is_admin_task(tsk)) {
        mnt_ns = devdrv_manager_get_host_mnt_ns();
    }

    spin_lock(&dev_container_table->spinlock);
    item = devdrv_manager_container_find_item(dev_container_table, mnt_ns);
    if (item == NULL) {
        spin_unlock(&dev_container_table->spinlock);
        devdrv_drv_err("item is null:mnt_ns(0x%pK), devid(%u)\n",
            (void *)(uintptr_t)mnt_ns, devid);
        return -EINVAL;
    }

    idx = devid / BIT_NUM_U64;
    offset = devid % BIT_NUM_U64;
    if ((idx < DEVDRV_CONTIANER_NUM_OF_LONG) &&
        (item->dev_list_map[idx] & (0x1ULL << offset))) {
        spin_unlock(&dev_container_table->spinlock);
        return 0;
    }

    end_index = (devid + 1) * VDAVINCI_MAX_DEVICE_COUNT + VDAVINCI_VDEV_OFFSET;
    for (i = (devid * VDAVINCI_MAX_DEVICE_COUNT) + VDAVINCI_VDEV_OFFSET; i < end_index; i++) {
        idx = i / BIT_NUM_U64;
        offset = i % BIT_NUM_U64;
        if ((idx < DEVDRV_CONTIANER_NUM_OF_LONG) &&
            (item->dev_list_map[idx] & (0x1ULL << offset))) {
            spin_unlock(&dev_container_table->spinlock);
            return 0;
        }
    }
    spin_unlock(&dev_container_table->spinlock);
    devdrv_drv_err("devid check in container failed:mnt_ns(0x%pK),devid(%u)\n",
        (void *)(uintptr_t)mnt_ns, devid);
    return -EINVAL;
}
EXPORT_SYMBOL(devdrv_manager_container_check_devid_in_container_ns);
#else
int devdrv_manager_container_check_devid_in_container(u32 devid, pid_t hostpid)
{
    return uda_proc_can_access_udevid(hostpid, devid) ? 0 : -EINVAL;
}
EXPORT_SYMBOL(devdrv_manager_container_check_devid_in_container);
int devdrv_manager_container_check_devid_in_container_ns(u32 devid, struct task_struct *task)
{
    bool ret = (task == current) ? uda_can_access_udevid(devid) : uda_proc_can_access_udevid(task->tgid, devid);
    return ret ? 0 : -EINVAL;
}
EXPORT_SYMBOL(devdrv_manager_container_check_devid_in_container_ns);
#endif // CFG_FEATURE_OLD_DEVID_TRANS

STATIC int (*CONST devdrv_manager_container_process_handler[DEVDRV_CONTAINER_MAX_CMD])(
    struct devdrv_container_para *cmd) = {
#ifdef CFG_FEATURE_OLD_DEVID_TRANS
        [DEVDRV_CONTAINER_GET_DAVINCI_DEVLIST] = devdrv_manager_container_get_davinci_devlist,
#endif
        [DEVDRV_CONTAINER_GET_BARE_PID] = devdrv_manager_container_get_bare_pid,
        [DEVDRV_CONTAINER_GET_BARE_TGID] = devdrv_manager_container_get_bare_tgid,
    };

int devdrv_manager_container_process(struct file *filep, unsigned long arg)
{
    struct devdrv_container_para container_cmd;
    int ret;

    if (!arg || (filep == NULL) || (filep->private_data == NULL)) {
        devdrv_drv_err("arg = %lu, filep = %pK\n", arg, filep);
        return -EINVAL;
    }

    ret = copy_from_user_safe(&container_cmd, (void *)((uintptr_t)arg), sizeof(struct devdrv_container_para));
    if (ret) {
        devdrv_drv_err("copy_from_user return error: %d.\n", ret);
        return ret;
    }

    if (container_cmd.para.cmd >= DEVDRV_CONTAINER_MAX_CMD) {
        devdrv_drv_err("invalid input container process cmd: %u.\n", container_cmd.para.cmd);
        return -EINVAL;
    }

    if (devdrv_manager_container_process_handler[container_cmd.para.cmd] == NULL) {
        devdrv_drv_err("not supported cmd: %u.\n", container_cmd.para.cmd);
        return -EINVAL;
    }

    ret = devdrv_manager_container_process_handler[container_cmd.para.cmd](&container_cmd);

    return ret;
}
#endif // DEVDRV_MANAGER_HOST_UT_TEST
