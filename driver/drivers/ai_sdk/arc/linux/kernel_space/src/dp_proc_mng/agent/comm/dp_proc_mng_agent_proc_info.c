/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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
 * Create: 2022-08-13
 */
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/hashtable.h>
#include <linux/mutex.h>
#include <linux/securec.h>
#include <linux/kthread.h>

#include "dp_proc_mng_cmd.h"
#include "dp_proc_mng_log.h"
#include "davinci_api.h"
#include "dp_proc_mng_pid_map.h"
#include "dp_proc_mng_proc_info.h"

struct dp_proc_mng_bind_cgroup_node {
    struct list_head list;
    struct semaphore sync_sm;
    BIND_CGROUP_TYPE bind_type;
    pid_t pid;
    int result;
    pid_t tgid;
};

#define DP_PROC_MNG_AICPU_CGROUP_FILE     "/sys/fs/cgroup/cpuset/AICPU/tasks"
#define DP_PROC_MNG_DATACPU_CGROUP_FILE   "/sys/fs/cgroup/cpuset/DataCPU/tasks"
#define DP_PROC_MNG_MEM_CGROUP_FILE       "/sys/fs/cgroup/memory/usermemory/tasks"

#define DP_PROC_MNG_PID_LEN 16
#define CGROUP_MEM_TYPE      1
#define CGROUP_CPU_TYPE      0

/*
 * dp_proc_mng module init
 */
STATIC struct dp_proc_mng_info *dp_proc_mng = NULL;

struct dp_proc_mng_info *dp_proc_get_manager_info(void)
{
    return dp_proc_mng;
}

int dp_proc_mng_info_init(void)
{
    dp_proc_mng = kzalloc(sizeof(struct dp_proc_mng_info), GFP_KERNEL);
    if (dp_proc_mng == NULL) {
        dp_proc_mng_drv_err("kzalloc return NULL, failed to alloc mem for manager struct.\n");
        return -ENOMEM;
    }

    spin_lock_init(&dp_proc_mng->proc_hash_table_lock);
    INIT_LIST_HEAD(&dp_proc_mng->hostpid_list_header);
    hash_init(dp_proc_mng->proc_hash_table);
    mutex_init(&dp_proc_mng->dp_proc_mng_sign_list_lock);

    return 0;
}

STATIC void dp_proc_mng_free_hashtable(struct dp_proc_mng_info *d_info)
{
    struct dp_proc_mng_sign *d_sign = NULL;
    struct hlist_node *local_sign = NULL;
    u32 bkt;

    if (d_info == NULL) {
        dp_proc_mng_drv_err("dp_proc_mng_info is NULL\n");
        return;
    }

    hash_for_each_safe(d_info->proc_hash_table, bkt, local_sign, d_sign, link) {
        hash_del(&d_sign->link);
        kfree(d_sign);
        d_sign = NULL;
    }

    return;
}

void dp_proc_mng_info_unint(void)
{
    dp_proc_mng_free_hashtable(dp_proc_mng);

    kfree(dp_proc_mng);
    dp_proc_mng = NULL;
}

int dp_proc_mng_davinci_module_init(const struct file_operations *ops)
{
    int ret;

    ret = drv_davinci_register_sub_module(DAVINCI_DP_PROC_MNG_SUB_MODULE_NAME, ops);
    if (ret != 0) {
        dp_proc_mng_drv_err("Register sub module failed. (ret=%d)\n", ret);
        return -ENODEV;
    }
    return 0;
}

void dp_proc_mng_davinci_module_uninit(void)
{
    int ret;

    ret = drv_ascend_unregister_sub_module(DAVINCI_DP_PROC_MNG_SUB_MODULE_NAME);
    if (ret != 0) {
        dp_proc_mng_drv_err("Unregister sub module failed. (ret=%d)\n", ret);
        return;
    }
    return;
}

STATIC int dp_proc_mng_ictl_bind_host_pid(struct file *file, struct dp_proc_mng_ioctl_arg *arg)
{
    struct dp_proc_mng_bind_host_pid *para_info = &arg->data.bind_host_pid_para;
    int node_id = numa_node_id();
    int ret;

    para_info->sign[PROCESS_SIGN_LENGTH - 1] = '\0';

    if ((para_info->len != PROCESS_SIGN_LENGTH) || (para_info->mode >= AICPUFW_MAX_PLAT) || (para_info->cp_type < 0) ||
        (para_info->cp_type >= PROCESS_CPTYPE_MAX) || (para_info->chip_id >= DP_PROC_MNG_MAX_NODE_NUM) ||
        (para_info->vfid >= VFID_NUM_MAX)) {
        dp_proc_mng_drv_err("invalid para length(%u) cp_type(%d) chip_id(%u) hostpid(%d) vfid(%u).\n", para_info->len,
            para_info->cp_type, para_info->chip_id, para_info->host_pid, para_info->vfid);
        return -EINVAL;
    }

    if ((para_info->mode == AICPUFW_ONLINE_PLAT) && (para_info->vfid == 0)) {
        ret = dp_proc_mng_check_process_sign(para_info->host_pid, para_info->sign, para_info->len);
        if (ret != 0) {
            dp_proc_mng_drv_err("d2h check sign failed, ret(%d).\n", ret);
            return -EINVAL;
        }
    }

    ret = dp_proc_mng_bind_host_pid(para_info);
    if (ret != 0) {
        dp_proc_mng_drv_err("bind_hostpid error. dev_id:%u, ret:%d, host_pid:%d, cp_type:%d, devpid:%d\n", node_id, ret,
            para_info->host_pid, para_info->cp_type, current->tgid);
        return ret;
    }

    return 0;
}

STATIC int dp_proc_mng_ictl_query_host_pid(struct file *file, struct dp_proc_mng_ioctl_arg *arg)
{
    struct dp_proc_mng_query_pid *para_info = &arg->data.query_pid_para;
    struct dp_proc_mng_info *d_info = dp_proc_get_manager_info();
    struct dp_proc_mng_sign *d_sign = NULL;
    u32 bkt = 0;
    int i, j;

    if (para_info->pid <= 0) {
        dp_proc_mng_drv_err("Invalid para pid(%d).\n", para_info->pid);
        return -EINVAL;
    }

    spin_lock_bh(&d_info->proc_hash_table_lock);

    hash_for_each(d_info->proc_hash_table, bkt, d_sign, link) {
        for (i = 0; i < DP_PROC_MNG_MAX_NODE_NUM; i++) {
            for (j = 0; j < VFID_NUM_MAX; j++) {
                if (d_sign->devpid[i][j][PROCESS_CP1] == para_info->pid)
                    para_info->cp_type = PROCESS_CP1;
#ifndef EMU_ST
                else if (d_sign->devpid[i][j][PROCESS_CP2] == para_info->pid)
                    para_info->cp_type = PROCESS_CP2;
                else if (d_sign->devpid[i][j][PROCESS_QS] == para_info->pid)
                    para_info->cp_type = PROCESS_QS;
                else if (d_sign->devpid[i][j][PROCESS_DEV_ONLY] == para_info->pid)
                    para_info->cp_type = PROCESS_DEV_ONLY;
#endif
                else
                    continue;
                arg->head.devid = i;
                arg->head.vfid = j;
                para_info->host_pid = d_sign->hostpid;
                spin_unlock_bh(&d_info->proc_hash_table_lock);
                return 0;
            }
        }
    }

    spin_unlock_bh(&d_info->proc_hash_table_lock);

    if (para_info->cp_type != PROCESS_CPTYPE_MAX) {
        dp_proc_mng_drv_err("cannot find cp_pid %d in hashtable.\n", para_info->pid);
    }

    return -EINVAL;
}

#define DP_PROC_MNG_WAIT_MAX_TIME 10000    /* 10 s */
STATIC int dp_proc_mng_ictl_bind_cgroup(struct file *file, struct dp_proc_mng_ioctl_arg *arg)
{
    struct dp_proc_mng_bind_cgroup_para *para = &arg->data.bind_cgroup_para;
    struct dp_proc_mng_info *d_info = dp_proc_get_manager_info();
    struct dp_proc_mng_bind_cgroup_node *node = NULL;
    int ret;

    if (para->bind_type >= BIND_CGROUP_MAX_TYPE) {
        dp_proc_mng_drv_warn("Operation not permitted. (bind_type=%u)\n", para->bind_type);
        return -EPERM;
    }

    node = kmalloc(sizeof(struct dp_proc_mng_bind_cgroup_node), GFP_KERNEL | __GFP_ACCOUNT);
    if (node == NULL) {
        dp_proc_mng_drv_err("Kmalloc failed.\n");
        return -ENOMEM;
    }
    node->pid = current->pid;
    node->tgid = current->tgid;
    node->result = -1;
    node->bind_type = para->bind_type;
    sema_init(&node->sync_sm, 0);

    mutex_lock(&d_info->bind_cgroup_list_lock);
    list_add_tail(&node->list, &d_info->bind_cgroup_list);
    mutex_unlock(&d_info->bind_cgroup_list_lock);

    (void)queue_work(d_info->bind_cgroup_wq, &d_info->bind_cgroup_work);

    ret = down_timeout(&node->sync_sm, msecs_to_jiffies(DP_PROC_MNG_WAIT_MAX_TIME));
    if (ret != 0) {
        dp_proc_mng_drv_warn("Add cgroup timeout. (ret=%d)\n", ret);
    }

    mutex_lock(&d_info->bind_cgroup_list_lock);
    list_del(&node->list);
    mutex_unlock(&d_info->bind_cgroup_list_lock);

    ret = node->result;
    kfree(node);

    return ret;
}


int (* const dp_proc_mng_ioctl_handlers[DP_PROC_MNG_CMD_MAX_CMD])(struct file *file,
    struct dp_proc_mng_ioctl_arg *arg) = {
        [_IOC_NR(DP_PROC_MNG_PROCESS_STATUS_QUERY)] = NULL,
        [_IOC_NR(DP_PROC_MNG_BIND_PID_ID)] = dp_proc_mng_ictl_bind_host_pid,
        [_IOC_NR(DP_PROC_MNG_UNBIND_PID_ID)] = NULL,
        [_IOC_NR(DP_PROC_MNG_QUERY_PID_ID)] = dp_proc_mng_ictl_query_host_pid,
        [_IOC_NR(DP_PROC_MNG_BIND_CGROUP)] = dp_proc_mng_ictl_bind_cgroup,
};

STATIC char *dp_proc_mng_get_cgroup_filename(struct dp_proc_mng_bind_cgroup_node *node, int cgroup_bind_type)
{
    if (cgroup_bind_type == CGROUP_CPU_TYPE) {
    switch (node->bind_type) {
        case BIND_AICPU_CGROUP:
            return DP_PROC_MNG_AICPU_CGROUP_FILE;
        case BIND_DATACPU_CGROUP:
            return DP_PROC_MNG_DATACPU_CGROUP_FILE;
        default:
            return NULL;
        }
    } else if (cgroup_bind_type == CGROUP_MEM_TYPE) {
        return DP_PROC_MNG_MEM_CGROUP_FILE;
    } else {
        return NULL;
    }
}

STATIC pid_t dp_proc_mng_get_cgroup_pid(struct dp_proc_mng_bind_cgroup_node *node, int cgroup_bind_type)
{
    return cgroup_bind_type == CGROUP_CPU_TYPE ? node->pid : node->tgid;
}

STATIC int _dp_proc_mng_bind_cgroup(struct dp_proc_mng_bind_cgroup_node *node, int cgroup_bind_type)
{
    char data[DP_PROC_MNG_PID_LEN] = {0};
    struct file *filp = NULL;
    char *filename = NULL;
    ssize_t result;
    loff_t offset;
    pid_t tmp_data;
    int ret;

    if (node->result == 0) {
        return 0;
    }

    filename = dp_proc_mng_get_cgroup_filename(node, cgroup_bind_type);
    if (filename == NULL) {
        dp_proc_mng_drv_err("Get cgroup filename failed. (pid=%d; tgid=%d; bind_type=%u; cgroup_bind_type=%d)\n",
            node->pid, node->tgid, node->bind_type, cgroup_bind_type);
        return -1;
    }

    tmp_data = dp_proc_mng_get_cgroup_pid(node, cgroup_bind_type);

    ret = snprintf_s(data, DP_PROC_MNG_PID_LEN, DP_PROC_MNG_PID_LEN - 1, "%d", tmp_data);
    if (ret < 0) {
        return ret;
    }

    filp = filp_open(filename, O_RDWR, 0);
    if (IS_ERR_OR_NULL(filp)) {
        dp_proc_mng_drv_err("Open cgroup file failed. (err=%ld)\n", PTR_ERR(filp));
        return -1;
    }

    offset = vfs_llseek(filp, 0, SEEK_END);
    if (offset < 0) {
        filp_close(filp, NULL);
        dp_proc_mng_drv_err("Vfs_llseek cgroup file failed. (offset=%lld)\n", offset);
        return -1;
    }

    result = kernel_write(filp, (const void *)data, DP_PROC_MNG_PID_LEN, &offset);
    if (result != DP_PROC_MNG_PID_LEN) {
        filp_close(filp, NULL);
        dp_proc_mng_drv_err("write cgroup file failed. (pid=%d; result=%ld)\n", node->pid, result);
        return -1;
    }

    filp_close(filp, NULL);
    return 0;
}

STATIC int dp_proc_mng_bind_cgroup(struct dp_proc_mng_bind_cgroup_node *node)
{
    return _dp_proc_mng_bind_cgroup(node, CGROUP_CPU_TYPE);
}

STATIC void dp_proc_mng_bind_cgroup_work(struct work_struct *work)
{
    struct dp_proc_mng_info *d_info = container_of(work, struct dp_proc_mng_info, bind_cgroup_work);
    struct dp_proc_mng_bind_cgroup_node *node = NULL;

    mutex_lock(&d_info->bind_cgroup_list_lock);
    list_for_each_entry(node, &d_info->bind_cgroup_list, list) {
        node->result = dp_proc_mng_bind_cgroup(node);
        up(&node->sync_sm);
    }
    mutex_unlock(&d_info->bind_cgroup_list_lock);
}

int dp_proc_mng_create_work(void)
{
    struct dp_proc_mng_info *d_info = dp_proc_get_manager_info();
    struct workqueue_struct *wq = NULL;

    INIT_LIST_HEAD(&d_info->bind_cgroup_list);
    mutex_init(&d_info->bind_cgroup_list_lock);
    INIT_WORK(&d_info->bind_cgroup_work, dp_proc_mng_bind_cgroup_work);

    wq = create_singlethread_workqueue("dp_proc_mng_bind_cgroup");
    if (wq == NULL) {
        mutex_destroy(&d_info->bind_cgroup_list_lock);
        dp_proc_mng_drv_err("Create add cgroup work failed.\n");
        return -EINVAL;
    }
    d_info->bind_cgroup_wq = wq;

    return 0;
}

void dp_proc_mng_destroy_work(void)
{
    struct dp_proc_mng_info *d_info = dp_proc_get_manager_info();

    if (d_info->bind_cgroup_wq != NULL) {
        destroy_workqueue(d_info->bind_cgroup_wq);
        d_info->bind_cgroup_wq = NULL;
    }
    mutex_destroy(&d_info->bind_cgroup_list_lock);
}
