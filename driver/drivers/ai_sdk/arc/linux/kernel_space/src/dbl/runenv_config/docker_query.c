/*
* Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
* Create: 2022-08-08
*/
#include <linux/sched.h>
#include <linux/version.h>
#include <linux/capability.h>
#include <linux/cred.h>
#include <linux/nsproxy.h>
#include <linux/types.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
#include <linux/sched/task.h>
#endif

#include "dbl/runenv_config.h"

#include "runenv_config_module.h"
#include "docker_query.h"

extern struct task_struct init_task;

static bool is_admin_task(struct task_struct *tsk)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0)
    kernel_cap_t privileged = (kernel_cap_t){{ ~0, (CAP_TO_MASK(CAP_AUDIT_READ + 1) -1)}};
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

    CAP_FOR_EACH_U32(i)
    {
        if ((effective.cap[i] & privileged.cap[i]) != privileged.cap[i]) {
            return false;
        }
    }
    if (user_id == 0) {
        return true;
    } else {
        return false;
    }
}

static int task_struct_check(struct task_struct *tsk)
{
    if (tsk == NULL) {
        recfg_err("The tsk is NULL\n");
        return -EINVAL;
    }

    if (tsk->nsproxy == NULL) {
        recfg_err("The tsk->nsproxy is NULL\n");
        return -ENODEV;
    }
    if (tsk->nsproxy->mnt_ns == NULL) {
        recfg_err("The tsk->nsproxy->mnt_ns is NULL\n");
        return -EFAULT;
    }
    return 0;
}

static struct mnt_namespace *get_current_mnt_ns(void)
{
    int ret;

    ret = task_struct_check(current);
    if (ret) {
        recfg_err("Current is invalid. (ret=%d)\n", ret);
        return NULL;
    }

    return current->nsproxy->mnt_ns;
}

static struct mnt_namespace *get_host_mnt_ns(void)
{
    return init_task.nsproxy->mnt_ns;
}

static bool check_in_host_mach(void)
{
    struct mnt_namespace *current_ns = NULL;
    struct mnt_namespace *host_ns = NULL;

    current_ns = get_current_mnt_ns();
    if (current_ns == NULL) {
        return false;
    }

    host_ns = get_host_mnt_ns();
    if (host_ns == NULL) {
        return false;
    }

    if (current_ns == host_ns) {
        return true;
    }

    return false;
}

bool run_in_normal_docker(void)
{
    if ((is_admin_task(current) == false) && (check_in_host_mach() == false)) {
        recfg_debug("In normal docker.\n");
        return true;
    }

    return false;
}
EXPORT_SYMBOL(run_in_normal_docker);

bool run_in_admin_docker(void)
{
    if ((is_admin_task(current) == true) && (check_in_host_mach() == false)) {
        recfg_debug("In admin docker.\n");
        return true;
    }

    return false;
}

