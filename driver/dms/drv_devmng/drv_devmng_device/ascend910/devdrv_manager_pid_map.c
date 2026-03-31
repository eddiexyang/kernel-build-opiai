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

#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
#include <linux/sched/task.h>
#endif
#ifdef CFG_HOST_ENV
    #include <linux/pid_namespace.h>
#endif

#include "securec.h"
#ifndef DEVDRV_MANAGER_HOST_UT_TEST
    #ifndef CFG_HOST_ENV
    #include "drv_whitelist.h"
    #endif
#endif
#include "dbl/uda.h"
#include "devdrv_pcie.h"
#include "devdrv_manager.h"
#include "ascend_hal_error.h"
#include "devdrv_manager_container.h"
#include "devdrv_manager_pid_map.h"
#include "devdrv_manager_container.h"
#include "devdrv_manager_common.h"
#include "devdrv_interface.h"
#include "davinci_api.h"

extern struct devdrv_manager_info *dev_manager_info;

#if (!defined (DEVMNG_UT)) && (!defined (DEVDRV_MANAGER_HOST_UT_TEST))
struct devdrv_process_sign *devdrv_find_process_sign(struct devdrv_manager_info *d_info,
    pid_t hostpid)
{
    struct devdrv_process_sign *proc_sign = NULL;
    u32 key;

    key = (u32)hostpid & DEVDRV_PROC_HASH_TABLE_MASK;
    /*lint -e666 */
    hash_for_each_possible(d_info->proc_hash_table, proc_sign, link, key) {
        if (proc_sign->hostpid == hostpid) {
            return proc_sign;
        }
    }
    return NULL;
}

STATIC struct devdrv_process_sign *devdrv_create_process_sign(struct devdrv_manager_info *d_info,
    struct devdrv_ioctl_para_bind_host_pid para_info)
{
    struct devdrv_process_sign *d_sign = NULL;
    int ret;
    enum devdrv_process_type k;
    int i, j;

    d_sign = ka_vmalloc(sizeof(struct devdrv_process_sign), GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT, PAGE_KERNEL);
    if (d_sign == NULL) {
        return NULL;
    }
    d_sign->hostpid = para_info.host_pid;
    d_sign->cp_count = 0;
    d_sign->sync_proc_cnt = 0;
    for (i = 0; i < PID_MAP_DEVNUM; i++) {
        for (j = 0; j < VFID_NUM_MAX; j++) {
            for (k = DEVDRV_PROCESS_CP1; k < DEVDRV_PROCESS_CPTYPE_MAX; k++) {
                d_sign->devpid[i][j][k] = DEVMNG_PID_INVALID;
            }
        }
    }

    for (i = 0; i < DEVMNG_USER_PROC_MAX; i++) {
        d_sign->user_proc_device[i].valid = 0;
        d_sign->user_proc_host[i].valid = 0;
    }

    ret = strcpy_s(d_sign->sign, PROCESS_SIGN_LENGTH, para_info.sign);
    if (ret) {
        vfree(d_sign);
        d_sign = NULL;
        return NULL;
    }

    devdrv_drv_debug("Create master pid ctx. (master_pid=%d)\n", para_info.host_pid);

    return d_sign;
}

static int devdrv_get_cur_run_side(void)
{
#ifdef CFG_HOST_ENV
    return HOST_SIDE;
#else
    return DEVICE_SIDE;
#endif
}

static struct devdrv_process_user_info *devdrv_get_user_proc(struct devdrv_process_sign *d_sign, int slave_side)
{
    if (slave_side == DEVICE_SIDE) {
        return d_sign->user_proc_device;
    } else {
        return d_sign->user_proc_host;
    }
}

static bool devdrv_is_update_slave_proc_num(int slave_side)
{
    return (devdrv_get_cur_run_side() == slave_side);
}

static int devdrv_bind_user_proc(struct devdrv_process_sign *d_sign,
    struct devdrv_ioctl_para_bind_host_pid para_info, pid_t devpid, int slave_side)
{
    struct devdrv_process_user_info *user_proc = devdrv_get_user_proc(d_sign, slave_side);
    int i;

    /* should check proc */

    for (i = 0; i < DEVMNG_USER_PROC_MAX; i++) {
        if (user_proc[i].valid == 0) {
            break;
        }
    }

    if (i >= DEVMNG_USER_PROC_MAX) {
        devdrv_drv_err("Overflow. (pid=%d)\n", devpid);
        return -EINVAL;
    }

    user_proc[i].devid = para_info.chip_id;
    user_proc[i].vfid = para_info.vfid;
    user_proc[i].pid = devpid;
    user_proc[i].valid = 1;

    if (devdrv_is_update_slave_proc_num(slave_side)) {
        d_sign->cp_count++;
    } else {
        d_sign->sync_proc_cnt++;
    }

    return 0;
}

static int devdrv_unbind_user_proc(struct devdrv_process_sign *d_sign, u32 devid,
    u32 vfid, pid_t devpid, int slave_side)
{
    struct devdrv_process_user_info *user_proc = devdrv_get_user_proc(d_sign, slave_side);
    int i;

    for (i = 0; i < DEVMNG_USER_PROC_MAX; i++) {
        if ((user_proc[i].valid == 1) && (user_proc[i].pid == devpid)
            && (user_proc[i].devid == devid) && (user_proc[i].vfid == vfid)) {
            break;
        }
    }

    if (i >= DEVMNG_USER_PROC_MAX) {
        devdrv_drv_err("Not find. (pid=%d)\n", devpid);
        return -EINVAL;
    }

    user_proc[i].valid = 0;

    if (devdrv_is_update_slave_proc_num(slave_side)) {
        d_sign->cp_count--;
    } else {
        d_sign->sync_proc_cnt--;
    }

    return 0;
}

/* side: host 1 device 0 */
STATIC struct devdrv_process_user_info *devdrv_query_user_proc(struct devdrv_process_sign *d_sign,
    int slave_side, pid_t devpid)
{
    struct devdrv_process_user_info *user_proc = devdrv_get_user_proc(d_sign, slave_side);
    int i;

    for (i = 0; i < DEVMNG_USER_PROC_MAX; i++) {
        if ((user_proc[i].valid == 1) && (user_proc[i].pid == devpid)) {
            return &user_proc[i];
        }
    }

    return NULL;
}

/*
 * Notice:Cannot print because it is called in Spinlock
 */
STATIC int devdrv_check_and_bind_hostpid(struct devdrv_process_sign *d_sign,
    struct devdrv_ioctl_para_bind_host_pid para_info, pid_t devpid, int slave_side, pid_t *bound_devpid)
{
    if (para_info.cp_type == DEVDRV_PROCESS_USER) {
        *bound_devpid = 0;
        return devdrv_bind_user_proc(d_sign, para_info, devpid, slave_side);
    }

    if (d_sign->devpid[para_info.chip_id][para_info.vfid][para_info.cp_type] == DEVMNG_PID_INVALID ||
        d_sign->devpid[para_info.chip_id][para_info.vfid][para_info.cp_type] == DEVMNG_PID_START_ONCE) {
        if ((para_info.cp_type == DEVDRV_PROCESS_CP2) &&
            (d_sign->devpid[para_info.chip_id][para_info.vfid][DEVDRV_PROCESS_CP1] == DEVMNG_PID_INVALID)) {
            return -EPERM;
        }
        d_sign->devpid[para_info.chip_id][para_info.vfid][para_info.cp_type] = devpid;

        if (devdrv_is_update_slave_proc_num(slave_side)) {
            d_sign->cp_count += (para_info.cp_type == DEVDRV_PROCESS_CP1 ? 1 : 0);
            d_sign->cp_count += (para_info.cp_type == DEVDRV_PROCESS_DEV_ONLY ? 1 : 0);
            d_sign->cp_count += (para_info.cp_type == DEVDRV_PROCESS_QS ? 1 : 0);
            d_sign->cp_count += (para_info.cp_type == DEVDRV_PROCESS_HCCP ? 1 : 0);
        } else {
            d_sign->sync_proc_cnt += (para_info.cp_type == DEVDRV_PROCESS_CP1 ? 1 : 0);
            d_sign->sync_proc_cnt += (para_info.cp_type == DEVDRV_PROCESS_DEV_ONLY ? 1 : 0);
        }
        d_sign->host_process_status = 0;
        return 0;
    }

    *bound_devpid = d_sign->devpid[para_info.chip_id][para_info.vfid][para_info.cp_type];
    d_sign->host_process_status = 0;

    if (d_sign->devpid[para_info.chip_id][para_info.vfid][para_info.cp_type] == devpid) {
        return -EBUSY;
    }

    return -EINVAL;
}

STATIC int devdrv_verify_sign(char *sign, int mode, pid_t host_pid, pid_t *dev_pid)
{
    int ret;
    pid_t pid_tmp = -1;
#ifndef CFG_HOST_ENV
#ifndef CFG_BUILD_DEBUG
    const char *process_name[WHITE_LIST_PROCESS_NUM_FOR_BIND_PID] = {PROCESS_NAME_TSD};
#endif
#endif

    /* sign is used as dev_pid. */
    ret = memcpy_s(&pid_tmp, sizeof(pid_t), sign, sizeof(pid_t));
    if (ret != 0 || pid_tmp == -1) {
        devdrv_drv_err("Failed to get the dev_pid. (ret=%d; pid=%d)\n", ret, pid_tmp);
        return -EINVAL;
    }

    /* Compatible with early HIVA scenarios and RC mode */
    if ((mode == AICPUFW_OFFLINE_PLAT) && (host_pid == current->tgid)) {
        *dev_pid = current->tgid;
        return 0;
    } else {
#ifndef CFG_HOST_ENV
#ifndef CFG_BUILD_DEBUG
        ret = whitelist_process_handler(process_name, WHITE_LIST_PROCESS_NUM_FOR_BIND_PID);
        if (ret != 0) {
            devdrv_drv_err("whitelist_process_handler error or Invalid parameter."
                " (ret=%d; dev_pid=%d)\n", ret, pid_tmp);
            return ret;
        }
#endif
#else
        if (devdrv_get_tgid_by_pid(pid_tmp, dev_pid) != 0) {
            devdrv_drv_err("Failed to get tgid by pid. (pid=%d)\n", pid_tmp);
            return -EINVAL;
        }
        return 0;
#endif
        *dev_pid = pid_tmp;
    }

    return 0;
}

#ifndef CFG_FEATURE_RC_MODE
STATIC int devdrv_pid_map_sync_result_check(int ret, struct devdrv_manager_msg_info *dev_manager_msg_info, int out_len)
{
    if (ret != 0) {
        devdrv_drv_warn("send msg to remote fail ret = %d.\n", ret);
        return ret;
    }

    if (out_len != (sizeof(struct devdrv_pid_map_sync) + sizeof(struct devdrv_manager_msg_head))) {
        devdrv_drv_warn("receive response len %d is not equal = %ld.\n", out_len,
            (sizeof(struct devdrv_ioctl_para_query_pid) + sizeof(struct devdrv_manager_msg_head)));
        return -EINVAL;
    }

    if (dev_manager_msg_info->header.result != 0) {
        devdrv_drv_warn("Failed. (ret=%d).\n", dev_manager_msg_info->header.result);
        return dev_manager_msg_info->header.result;
    }

    return 0;
}
#endif

#ifdef CFG_HOST_ENV
STATIC int devdrv_pid_map_sync_to_one_device(struct devdrv_ioctl_para_bind_host_pid *para_info, pid_t pid, int op)
{
    int ret;
    int out_len = 0;
    u32 remote_udevid;
    struct devdrv_info *dev_info = NULL;
    struct devdrv_manager_msg_info dev_manager_msg_info = {{0}, {0}};
    struct devdrv_pid_map_sync *sync = (struct devdrv_pid_map_sync *)dev_manager_msg_info.payload;

    if (para_info->cp_type != DEVDRV_PROCESS_USER) {
        return 0;
    }

    dev_manager_msg_info.header.msg_id = DEVDRV_MANAGER_CHAN_PID_MAP_SYNC;
    dev_manager_msg_info.header.valid = DEVDRV_MANAGER_MSG_VALID;
    /* give a random value for checking result later */
    dev_manager_msg_info.header.result = (u16)DEVDRV_MANAGER_MSG_INVALID_RESULT;
    /* inform corresponding devid to device side */

    ret = uda_dev_get_remote_udevid(para_info->chip_id, &remote_udevid);
    if (ret != 0) {
        devdrv_drv_err("get remote udevid failed. (udevid=%u)\n", para_info->chip_id);
        return ret;
    }

    sync->cp_type = para_info->cp_type;
    sync->host_pid = para_info->host_pid;
    sync->chip_id = remote_udevid;
    sync->vfid = para_info->vfid;
    sync->pid = pid;
    sync->op = op;

    dev_manager_msg_info.header.dev_id = para_info->chip_id;
    dev_info = devdrv_manager_get_devdrv_info(para_info->chip_id);
    if (dev_info == NULL) {
        devdrv_drv_err("dev_info is NULL. (dev_id=%u)\n", para_info->chip_id);
        return -EINVAL;
    }

    ret = devdrv_manager_send_msg(dev_info, &dev_manager_msg_info, &out_len);
    if (devdrv_pid_map_sync_result_check(ret, &dev_manager_msg_info, out_len)) {
        return -EINVAL;
    }

    return 0;
}

STATIC int devdrv_pid_map_sync_to_peer(struct devdrv_ioctl_para_bind_host_pid *para_info, pid_t pid, int op)
{
    u32 *udevids = NULL;
    u32 dev_num, devid, i;
    int ret;

    if (para_info->chip_id < DEVDRV_MAX_DAVINCI_NUM) {
        return devdrv_pid_map_sync_to_one_device(para_info, pid, op);
    }

    dev_num = uda_get_cur_ns_dev_num();
    udevids = vmalloc(sizeof(u32) * dev_num);
    if (udevids == NULL) {
        devdrv_drv_err("Alloc udevids failed. (pid=%d; dev_num=%u)\n", pid, dev_num);
        return -ENOMEM;
    }

    ret = uda_get_cur_ns_udevids(udevids, dev_num);
    if (ret != 0) {
        vfree(udevids);
        devdrv_drv_err("Get udevids failed. (pid=%d; dev_num=%u)\n", pid, dev_num);
        return ret;
    }

    devid = para_info->chip_id;

    for (i = 0; i < dev_num; i++) {
        para_info->chip_id = udevids[i];
        ret = devdrv_pid_map_sync_to_one_device(para_info, pid, op);
        if (ret != 0) {
            devdrv_drv_err("Sync failed. (pid=%d; udevid=%u)\n", pid, udevids[i]);
            vfree(udevids);
            para_info->chip_id = devid;
            return ret;
        }
    }

    vfree(udevids);

    para_info->chip_id = devid;

    return 0;
}
#else
STATIC int devdrv_pid_map_sync_to_peer(struct devdrv_ioctl_para_bind_host_pid *para_info, pid_t pid, int op)
{
#ifdef CFG_FEATURE_RC_MODE
    return 0;
#else
    int ret;
    int out_len = 0;
    u32 remote_udevid;
    struct devdrv_manager_msg_info dev_manager_msg_info = {{0}, {0}};
    struct devdrv_pid_map_sync *sync = (struct devdrv_pid_map_sync *)dev_manager_msg_info.payload;

    if ((para_info->cp_type != DEVDRV_PROCESS_CP1) || (para_info->vfid != 0) ||
        (para_info->mode != AICPUFW_ONLINE_PLAT)) {
        return 0;
    }

    dev_manager_msg_info.header.msg_id = DEVDRV_MANAGER_CHAN_PID_MAP_SYNC;
    dev_manager_msg_info.header.valid = DEVDRV_MANAGER_MSG_VALID;
    /* give a random value for checking result later */
    dev_manager_msg_info.header.result = (u16)DEVDRV_MANAGER_MSG_INVALID_RESULT;
    /* inform corresponding devid to device side */

    ret = uda_dev_get_remote_udevid(para_info->chip_id, &remote_udevid);
    if (ret != 0) {
        devdrv_drv_err("get remote udevid failed. (udevid=%u)\n", para_info->chip_id);
        return ret;
    }

    sync->cp_type = DEVDRV_PROCESS_DEV_ONLY; /* host has cp, store device cp in dev only */
    sync->host_pid = para_info->host_pid;
    sync->chip_id = remote_udevid;
    sync->vfid = para_info->vfid;
    sync->pid = pid;
    sync->op = op;

    devdrv_drv_info("Sync pid map. (op=%d; host_pid=%d; pid=%d; cp_type=%u; devid=%u)\n",
        sync->op, sync->host_pid, sync->pid, sync->cp_type, sync->chip_id);
    dev_manager_msg_info.header.dev_id = para_info->chip_id;
    ret = agentdrv_common_msg_send(para_info->chip_id, &dev_manager_msg_info, sizeof(struct devdrv_manager_msg_info),
        sizeof(struct devdrv_manager_msg_info), (u32 *)&out_len, AGENTDRV_COMMON_MSG_DEVDRV_MANAGER);

    return devdrv_pid_map_sync_result_check(ret, &dev_manager_msg_info, out_len);
#endif
}
#endif

#ifdef CFG_HOST_ENV
bool devdrv_is_master_pid(pid_t master_pid)
{
    struct devdrv_manager_info *d_info = devdrv_get_manager_info();
    struct devdrv_process_sign *d_sign = NULL;

    mutex_lock(&d_info->devdrv_sign_list_lock);
    list_for_each_entry(d_sign, &d_info->hostpid_list_header, list) {
        if (d_sign->hostpid == master_pid) {
            mutex_unlock(&d_info->devdrv_sign_list_lock);
            return true;
        }
    }
    mutex_unlock(&d_info->devdrv_sign_list_lock);
    return false;
}
#endif
#define MAX_BIND_WAIT_TIMES  300  /* max wait time 3s */
#define MAX_BIND_WAIT_ONCE   10

int devdrv_wait_pid_init(pid_t proc_pid, int mode)
{
    u32 times = 0;
    struct pid *pid_proc = NULL;
    if (mode == AICPUFW_OFFLINE_PLAT) {
        return 0;
    }
    do {
        times++;
        /* check process alive */
        pid_proc = find_get_pid(proc_pid);
        if (pid_proc == NULL) {
            return -ESRCH;
        }
        put_pid(pid_proc);

        /* check process init status */
        if (ascend_intf_is_pid_init(proc_pid, DAVINCI_INTF_MODULE_DEVMNG)) {
            return 0;
        }
        (void)msleep(MAX_BIND_WAIT_ONCE);
    } while (times < MAX_BIND_WAIT_TIMES);

    return -ETIME;
}


#ifdef CFG_HOST_ENV
static int check_parent_child_relationship(pid_t master_pid, char *sign)
{
    int ret;
    pid_t slave_pid = -1;
    pid_t cur_tgid = current->tgid;
    struct task_struct *tsk = NULL;
    struct pid_namespace *pid_ns = NULL;

    ret = memcpy_s(&slave_pid, sizeof(pid_t), sign, sizeof(pid_t));
    if (ret != 0 || slave_pid == -1) {
        devdrv_drv_err("Failed to get the dev_pid. (ret=%d; pid=%d)\n", ret, slave_pid);
        return -EINVAL;
    }

    pid_ns = task_active_pid_ns(current);
    if (pid_ns == NULL) {
        devdrv_drv_err("Failed to find current process's pid_ns. (pid=%d)\n", slave_pid);
        return -EINVAL;
    }

    tsk = get_pid_task(find_pid_ns(slave_pid, pid_ns), PIDTYPE_PID);
    if (tsk == NULL) {
        devdrv_drv_err("Failed to find task struct in current process's pid_ns. (pid=%d)\n", slave_pid);
        return -EINVAL;
    }

    /* A process can bind itself, but can not help other process bind themseves. */
    if (master_pid == tsk->tgid && master_pid == cur_tgid) {
        put_task_struct(tsk);
        return 0;
    }

    /*
    Currently, it is not allowed to help other process to bind slave pid.
    If it is allowed to help master process to bind current process's subprocess,
    it is necessary to check whether current process has bind master pid already.
    */
    if (master_pid != cur_tgid) {
        devdrv_drv_warn("master pid is not current tgid. (master_pid=%d; current tgid=%d)\n", master_pid, cur_tgid);
        put_task_struct(tsk);
        return -EOPNOTSUPP;
    }

    /* the slave process must be a child of the current process */
    if (tsk->real_parent->tgid != cur_tgid) {
        devdrv_drv_err("slave pid's parent is not master pid. (master_pid=%d; slave_pid=%d, parent_pid=%d)\n",
            master_pid, slave_pid, tsk->real_parent->tgid);
        put_task_struct(tsk);
        return -EINVAL;
    }

    put_task_struct(tsk);
    return 0;
}
#endif

int devdrv_bind_hostpid(struct devdrv_ioctl_para_bind_host_pid para_info)
{
    struct devdrv_manager_info *d_info = devdrv_get_manager_info();
    struct devdrv_process_sign *d_sign = NULL;
    struct devdrv_process_sign *d_sign_create = NULL;
    u32 key;
    int ret, ret_sync;
    pid_t bound_devpid = -1;
    pid_t dev_pid = -1;

#if (defined CFG_FEATURE_BIND_TGID) || (defined CFG_HOST_ENV)
    pid_t tgid = 0;

    if (devdrv_get_tgid_by_pid(para_info.host_pid, &tgid) != 0) {
        devdrv_drv_err("Failed to get tgid by pid. (pid=%d)\n", para_info.host_pid);
        return -EINVAL;
    }

    para_info.host_pid = tgid;
#endif

#ifdef CFG_HOST_ENV
    if (!devdrv_is_master_pid(para_info.host_pid)) {
        devdrv_drv_err("Master proc not create. (host_pid=%d)\n", para_info.host_pid);
        return -EINVAL;
    }

    ret = check_parent_child_relationship(para_info.host_pid, para_info.sign);
    if (ret != 0) {
        devdrv_drv_err("check parent child relationship failed. (ret=%d)\n", ret);
        return ret;
    }
#endif

    ret = devdrv_verify_sign(para_info.sign, para_info.mode, para_info.host_pid, &dev_pid);
    if (ret != 0) {
        devdrv_drv_err("Failed to get the dev_pid. (ret=%d; dev_pid=%d)\n", ret, dev_pid);
        return -EINVAL;
    }
#ifndef CFG_HOST_ENV
    ret = devdrv_wait_pid_init(dev_pid, para_info.mode);
    if (ret != 0) {
        devdrv_drv_err("wait process init failed. (ret=%d; dev_pid=%d)\n", ret, dev_pid);
        return -EINVAL;
    }
#endif

    key = (u32)para_info.host_pid & DEVDRV_PROC_HASH_TABLE_MASK;
    d_sign_create = devdrv_create_process_sign(d_info, para_info);
    if (d_sign_create == NULL) {
        devdrv_drv_err("Create sign failed or invalid sign. (chip_id=%u; hostpid=%d)\n",
            para_info.chip_id, para_info.host_pid);
        return -EINVAL;
    }

    ret_sync = devdrv_pid_map_sync_to_peer(&para_info, dev_pid, 1);
    if (ret_sync != 0) {
        devdrv_drv_warn("Sync to peer unsuccessfully."
            "(host_pid=%d; dev_pid=%d; cp_type=%d; dev_id=%u; vfid=%u; mode=%d)\n",
            para_info.host_pid, dev_pid, para_info.cp_type, para_info.chip_id, para_info.vfid, para_info.mode);
    }

    spin_lock_bh(&d_info->proc_hash_table_lock);
    d_sign = devdrv_find_process_sign(d_info, para_info.host_pid);
    if (d_sign != NULL) {
        ret = devdrv_check_and_bind_hostpid(d_sign, para_info, dev_pid, devdrv_get_cur_run_side(), &bound_devpid);
        spin_unlock_bh(&d_info->proc_hash_table_lock);
        devdrv_drv_debug("Destroy master pid ctx when exist. (master_pid=%d)\n", para_info.host_pid);
        vfree(d_sign_create);
        d_sign_create = NULL;
        if (ret == -EBUSY) {
            devdrv_drv_warn("Hostpid has already bound current devpid. "
                "(hostpid=%d; devpid=%d; cp_type=%d; chip_id=%u; vfid=%u)\n",
                para_info.host_pid, dev_pid, para_info.cp_type, para_info.chip_id, para_info.vfid);
        } else if (ret != 0) {
            devdrv_drv_err("Bind failed or already bound another process. "
                "(ret=%d; host_pid=%d; cp_type=%d; bound_devpid=%d; dev_pid=%d; dev_id=%u; vfid=%u).\n",
                ret, para_info.host_pid, para_info.cp_type, bound_devpid, dev_pid, para_info.chip_id, para_info.vfid);
            goto BIND_FAILED;
        }
        goto bind_succ;
    }

    ret = devdrv_check_and_bind_hostpid(d_sign_create, para_info, dev_pid, devdrv_get_cur_run_side(), &bound_devpid);
    if (ret != 0) {
        spin_unlock_bh(&d_info->proc_hash_table_lock);
        devdrv_drv_err("Bind failed or already bound another process. "
            "(ret=%d; host_pid=%d; cp_type=%d; bound_devpid=%d; dev_pid=%d; dev_id=%u; vfid=%u).\n",
            ret, para_info.host_pid, para_info.cp_type, bound_devpid, dev_pid, para_info.chip_id, para_info.vfid);
        vfree(d_sign_create);
        d_sign_create = NULL;
        goto BIND_FAILED;
    }
    hash_add(d_info->proc_hash_table, &d_sign_create->link, key); //lint !e666
    spin_unlock_bh(&d_info->proc_hash_table_lock);

bind_succ:
    if (ret_sync != 0) {
        ret = devdrv_pid_map_sync_to_peer(&para_info, dev_pid, 1);
        if (ret != 0) {
            devdrv_drv_warn("Sync to peer unsuccessfully."
                "(host_pid=%d; dev_pid=%d; cp_type=%d; dev_id=%u; vfid=%u; mode=%d)\n",
                para_info.host_pid, dev_pid, para_info.cp_type, para_info.chip_id, para_info.vfid, para_info.mode);
        }
    }

    devdrv_drv_info("Bind pid success. (host_pid=%d; dev_pid=%d; cp_type=%d; dev_id=%u; vfid=%u; mode=%d)\n",
        para_info.host_pid, dev_pid, para_info.cp_type, para_info.chip_id, para_info.vfid, para_info.mode);
    return 0;

BIND_FAILED:
    if (ret_sync == 0) {
        ret_sync = devdrv_pid_map_sync_to_peer(&para_info, dev_pid, 0);
        if (ret_sync != 0) {
            devdrv_drv_warn("Sync to peer to unbind unsuccessfully."
                "(host_pid=%d; dev_pid=%d; cp_type=%d; dev_id=%u; vfid=%u; mode=%d)\n",
                para_info.host_pid, dev_pid, para_info.cp_type, para_info.chip_id, para_info.vfid, para_info.mode);
        }
    }
    return ret;
}

static int devdrv_release_single_devpid(pid_t host_pid, pid_t dev_pid,
    unsigned int dev_id, unsigned int vf_id, enum devdrv_process_type cp_type, int slave_side)
{
    struct devdrv_manager_info *d_info = devdrv_get_manager_info();
    struct devdrv_process_sign *d_sign = NULL;
    struct devdrv_process_sign *d_sign_check = NULL;
    int ret, count;

    if (d_info == NULL) {
        devdrv_drv_err("Global variable dev_manager_info is NULL.\n");
        return -EINVAL;
    }

    spin_lock_bh(&d_info->proc_hash_table_lock);
    d_sign = devdrv_find_process_sign(d_info, host_pid);
    if (d_sign == NULL) {
        spin_unlock_bh(&d_info->proc_hash_table_lock);
        devdrv_drv_warn("Can't find host_pid in table, please check interface parameters. "
                       "(host_pid=%d; dev_pid=%d; dev_id=%u; vf_id=%u; cp_type=%d)\n",
                       host_pid, dev_pid, dev_id, vf_id, cp_type);
        return -EINVAL;
    }

    if (cp_type == DEVDRV_PROCESS_USER) {
        ret = devdrv_unbind_user_proc(d_sign, dev_id, vf_id, dev_pid, slave_side);
    } else {
        if ((d_sign->devpid[dev_id][vf_id][cp_type] == dev_pid)) {
            if (cp_type == DEVDRV_PROCESS_CP1) {
                d_sign->devpid[dev_id][vf_id][DEVDRV_PROCESS_CP1] = DEVMNG_PID_INVALID;
                d_sign->devpid[dev_id][vf_id][DEVDRV_PROCESS_CP2] = DEVMNG_PID_INVALID;
                d_sign->cp_count -= (devdrv_is_update_slave_proc_num(slave_side)) ? 1 : 0;
                d_sign->sync_proc_cnt -= (!devdrv_is_update_slave_proc_num(slave_side)) ? 1 : 0;
            } else if ((cp_type == DEVDRV_PROCESS_CP2) &&
                        (d_sign->devpid[dev_id][vf_id][DEVDRV_PROCESS_CP1] != DEVMNG_PID_INVALID)) {
                d_sign->devpid[dev_id][vf_id][DEVDRV_PROCESS_CP2] = DEVMNG_PID_START_ONCE;
            } else if (cp_type == DEVDRV_PROCESS_CP2) {
                d_sign->devpid[dev_id][vf_id][DEVDRV_PROCESS_CP2] = DEVMNG_PID_INVALID;
            } else {
                d_sign->devpid[dev_id][vf_id][cp_type] = DEVMNG_PID_INVALID;
                d_sign->cp_count -= (devdrv_is_update_slave_proc_num(slave_side)) ? 1 : 0;
                d_sign->sync_proc_cnt -= (!devdrv_is_update_slave_proc_num(slave_side)) ? 1 : 0;
            }
            ret = 0;
        } else {
            ret = -EINVAL;
        }
    }
    if (ret != 0) {
        spin_unlock_bh(&d_info->proc_hash_table_lock);
        devdrv_drv_err("Can't find dev_pid in table, please check interface parameters. "
                       "(host_pid=%d; dev_pid=%d; dev_id=%u; vf_id=%u; cp_type=%d)\n",
                       host_pid, dev_pid, dev_id, vf_id, cp_type);
        return ret;
    }

    count = d_sign->cp_count + d_sign->sync_proc_cnt;
    if (count == 0) {
        hash_del(&d_sign->link);
    }

    d_sign_check = devdrv_find_process_sign(d_info, host_pid);
    if (d_sign_check == NULL || (d_sign_check->devpid[dev_id][vf_id][cp_type] == DEVMNG_PID_INVALID)) {
        spin_unlock_bh(&d_info->proc_hash_table_lock);
        devdrv_drv_info("Release device process. (host_pid=%d; dev_pid=%d; dev_id=%u; vf_id=%u; cp_type=%d)\n",
                        host_pid, dev_pid, dev_id, vf_id, cp_type);
        ret = 0;
        goto release_exit;
    } else {
        spin_unlock_bh(&d_info->proc_hash_table_lock);
        devdrv_drv_info("Find process sign is abnormal. (dev_pid=%d)\n", d_sign_check->devpid[dev_id][vf_id][cp_type]);
        ret = -EINVAL;
        goto release_exit;
    }

release_exit:
    if (count == 0) {
        devdrv_drv_info("Destroy master pid ctx when proc count is 0. (master_pid=%d)\n", d_sign->hostpid);
        vfree(d_sign);
        d_sign = NULL;
    }
    return ret;
}

#ifdef CFG_FEATURE_HOST_UNBIND
int devdrv_unbind_hostpid(struct devdrv_ioctl_para_bind_host_pid para_info)
{
    int ret;
    u32 dev_id;
    u32 phys_id = DEVDRV_MAX_DAVINCI_NUM, vf_id = 0;
    pid_t host_pid = -1;
    pid_t dev_pid = -1;

    if (para_info.chip_id < PID_MAP_DEVNUM) {
        ret = devdrv_manager_trans_and_check_id(para_info.chip_id, &phys_id, &vf_id, ALSO_DOES_SUPPORT_VF);
        if (ret != 0) {
            devdrv_drv_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n",
                para_info.chip_id, ret);
            return ret;
        }
    }

    dev_id = phys_id;
    vf_id = para_info.vfid;
#if (defined CFG_FEATURE_BIND_TGID) || (defined CFG_HOST_ENV)
    if (devdrv_get_tgid_by_pid(para_info.host_pid, &host_pid) != 0) {
        devdrv_drv_err("Failed to get tgid by pid. (pid=%d)\n", para_info.host_pid);
        return -EINVAL;
    }
#else
    host_pid = para_info.host_pid;
#endif

    ret = devdrv_verify_sign(para_info.sign, para_info.mode, host_pid, &dev_pid);
    if (ret != 0) {
        devdrv_drv_err("Failed to get the dev_pid. (ret=%d; dev_pid=%d)\n", ret, dev_pid);
        return -EINVAL;
    }

    ret = devdrv_release_single_devpid(host_pid, dev_pid, dev_id, vf_id, para_info.cp_type, devdrv_get_cur_run_side());
    if (ret == 0) {
        int sync_ret = devdrv_pid_map_sync_to_peer(&para_info, dev_pid, 0);
        if (sync_ret != 0) {
            devdrv_drv_warn("Sync failed. (host_pid=%d; dev_pid=%d; cp_type=%d; dev_id=%u; vfid=%u; mode=%d)\n",
                para_info.host_pid, dev_pid, para_info.cp_type, para_info.chip_id, para_info.vfid, para_info.mode);
        }
    }

    return ret;
}
#endif

static int devdrv_pid_map_sync_add(struct devdrv_pid_map_sync *sync)
{
    struct devdrv_manager_info *d_info = devdrv_get_manager_info();
    struct devdrv_ioctl_para_bind_host_pid para_info = {0};
    struct devdrv_process_sign *d_sign = NULL, *d_sign_tmp = NULL;
    int ret = 0, create_flag = 0;
    int sync_side = (devdrv_get_cur_run_side() == DEVICE_SIDE) ? HOST_SIDE : DEVICE_SIDE;
    u32 bound_devpid;

    para_info.host_pid = sync->host_pid;
    para_info.chip_id = sync->chip_id;
    para_info.vfid = sync->vfid;
    para_info.cp_type = sync->cp_type;

    /* The mutex lock for creation and destruction is missing, devdrv_create_process_sign use vmalloc,
       can not call in spin_lock_bh, so alloc before here */
    d_sign_tmp = devdrv_create_process_sign(d_info, para_info);
    if (d_sign_tmp == NULL) {
        return -ENOMEM;
    }

    spin_lock_bh(&d_info->proc_hash_table_lock);
    d_sign = devdrv_find_process_sign(d_info, sync->host_pid);
    if (d_sign == NULL) {
        create_flag = 1;
        d_sign = d_sign_tmp;
    }

    ret = devdrv_check_and_bind_hostpid(d_sign, para_info, sync->pid, sync_side, &bound_devpid);
    if ((ret == 0) && (create_flag == 1)) {
        hash_add(d_info->proc_hash_table, &d_sign->link, sync->host_pid & DEVDRV_PROC_HASH_TABLE_MASK); //lint !e666
    }
    spin_unlock_bh(&d_info->proc_hash_table_lock);

    if ((d_sign_tmp != NULL) && (create_flag == 0)) {
        devdrv_drv_debug("Destroy master pid ctx when sync. (master_pid=%d)\n", d_sign_tmp->hostpid);
        vfree(d_sign_tmp);
    }

    devdrv_drv_info("Sync finish. (op=%d; host_pid=%d; pid=%d; cp_type=%u; devid=%u; ret=%d)\n",
        sync->op, sync->host_pid, sync->pid, sync->cp_type, sync->chip_id, ret);

    return ret;
}

static int devdrv_pid_map_sync_del(struct devdrv_pid_map_sync *sync)
{
    int sync_side = (devdrv_get_cur_run_side() == DEVICE_SIDE) ? HOST_SIDE : DEVICE_SIDE;
    int ret;

    ret = devdrv_release_single_devpid(sync->host_pid, sync->pid, sync->chip_id, sync->vfid,
        sync->cp_type, sync_side);
    devdrv_drv_info("Sync finish. (op=%d; host_pid=%d; pid=%d; cp_type=%u; devid=%u; ret=%d)\n",
        sync->op, sync->host_pid, sync->pid, sync->cp_type, sync->chip_id, ret);

    return ret;
}

#ifdef CFG_HOST_ENV
int devdrv_pid_map_sync_proc(void *msg, u32 *ack_len)
#else
int devdrv_pid_map_sync_proc(u32 devid, void *msg, u32 in_len, u32 *ack_len)
#endif
{
    struct devdrv_manager_msg_info *dev_manager_msg_info = NULL;
    struct devdrv_pid_map_sync *sync = NULL;

    dev_manager_msg_info = (struct devdrv_manager_msg_info *)msg;

    if (dev_manager_msg_info->header.valid != DEVDRV_MANAGER_MSG_VALID) {
        devdrv_drv_err("invalid message\n");
        return -EINVAL;
    }

#ifndef CFG_HOST_ENV
    if (in_len < sizeof(struct devdrv_manager_msg_info)) {
        devdrv_drv_err("Invalid message from host. (dev_id=%u; valid=%u; in_len=%u)\n",
                       devid, dev_manager_msg_info->header.valid, in_len);
        return -EINVAL;
    }
#endif
    sync = (struct devdrv_pid_map_sync *)dev_manager_msg_info->payload;

    if ((sync->chip_id >= PID_MAP_DEVNUM) || (sync->vfid >= VFID_NUM_MAX) ||
        (sync->cp_type >= DEVDRV_PROCESS_CPTYPE_MAX)) {
        devdrv_drv_err("Invalid devid. (host_pid=%d; pid=%d; cp_type=%u; devid=%u; vfid=%u)\n",
            sync->host_pid, sync->pid, sync->cp_type, sync->chip_id, sync->vfid);
        return -EINVAL;
    }

    if (sync->op == 1) {
        dev_manager_msg_info->header.result = devdrv_pid_map_sync_add(sync);
    } else {
        dev_manager_msg_info->header.result = devdrv_pid_map_sync_del(sync);
    }
    dev_manager_msg_info->header.valid = DEVDRV_MANAGER_MSG_VALID;
    *ack_len = sizeof(struct devdrv_pid_map_sync) + sizeof(struct devdrv_manager_msg_head);

    return 0;
}

static int check_hostpid_devpid_bind_status(struct devdrv_process_sign *d_sign, unsigned int dev_id, int dev_pid,
                                            struct dev_pid_info *dev_pid_info_list, u32 in_size, u32 *out_size)
{
    u32 vfid;
    enum devdrv_process_type type;

    for (vfid = 0; vfid < VFID_NUM_MAX; vfid++) {
        for (type = 0; type < DEVDRV_PROCESS_CPTYPE_MAX; type++) {
            if (d_sign->devpid[dev_id][vfid][type] != dev_pid) {
                continue;
            }
            if (in_size == 0) {
                return 0;
            }
            dev_pid_info_list[*out_size].dev_id = dev_id;
            dev_pid_info_list[*out_size].vfid = vfid;
            dev_pid_info_list[*out_size].cp_type = type;
            *out_size += 1;
            if (*out_size >= in_size) {
                return -ENOMEM;
            }
        }
    }

    if (*out_size == 0) {
        return -ESRCH;
    }
    return 0;
}

/*
* 1. This API is used to check the bind status between the two inputs, host_pid and dev_pid.
* 2. Also, if the host_pid and dev_pid were bound,
* this API will output the dev_pid info in the structure array, "dev_pid_info",
* and output the array length by "out_size".
* 3. If Users don't care the dev_pid info, the input para "in_size" can set to 0,
* and the "dev_pid_info" and "out_size" set to NULL.
* 4. The host_pid and dev_pid must be tgid.
*/
bool devdrv_process_is_bind(pid_t host_pid, pid_t dev_pid, struct dev_pid_info* dev_pid_info_list,
                            u32 in_size, u32* out_size)
{
    struct devdrv_manager_info *d_info = devdrv_get_manager_info();
    struct devdrv_process_sign *d_sign = NULL;
    u32 key;
    int ret;
    u32 i;
    u32 size = 0;

    if (in_size != 0 && (dev_pid_info_list == NULL || out_size == NULL)) {
        devdrv_drv_err("Invalid para. (in_size=%d, dev_pid_info_list is NULL=%d, out_size is NULL=%d)\n",
            in_size, dev_pid_info_list == NULL, out_size == NULL);
        return false;
    }

    key = (u32)host_pid & DEVDRV_PROC_HASH_TABLE_MASK;
    spin_lock_bh(&d_info->proc_hash_table_lock);
    hash_for_each_possible(d_info->proc_hash_table, d_sign, link, key) {
        if (host_pid != d_sign->hostpid) {
            continue;
        }
        for (i = 0; i < PID_MAP_DEVNUM; i++) {
            ret = check_hostpid_devpid_bind_status(d_sign, i, dev_pid, dev_pid_info_list, in_size, &size);
            if (ret == 0 && in_size == 0) {
                spin_unlock_bh(&d_info->proc_hash_table_lock);
                return true;
            }

            if (ret == -ENOMEM) {
                spin_unlock_bh(&d_info->proc_hash_table_lock);
                *out_size = size;
                devdrv_drv_debug("The dev_pid_info_list is already full. (list size=%u\n", in_size);
                return true;
            }
        }
        break;
    }

    if (size == 0) {
        spin_unlock_bh(&d_info->proc_hash_table_lock);
        return false;
    }
    *out_size = size;
    spin_unlock_bh(&d_info->proc_hash_table_lock);
    return true;
}
EXPORT_SYMBOL(devdrv_process_is_bind);

static int find_sign_and_get_hostpid(struct devdrv_process_sign *d_sign, pid_t dev_pid, unsigned int dev_id,
                                     struct host_pid_info* host_pid_list, unsigned int in_size, unsigned int *list_size)
{
    u32 vfid;
    enum devdrv_process_type type;
    unsigned int num = *list_size;

    for (vfid = 0; vfid < VFID_NUM_MAX; vfid++) {
        for (type = DEVDRV_PROCESS_CP1; type < DEVDRV_PROCESS_CPTYPE_MAX; type++)  {
            if (d_sign->devpid[dev_id][vfid][type] != dev_pid) {
                continue;
            }
            if (num >= in_size) {
                return -EINVAL;
            }
            host_pid_list[num].host_pid = d_sign->hostpid;
            host_pid_list[num].dev_id = dev_id;
            host_pid_list[num].vfid = vfid;
            host_pid_list[num].cp_type = type;
            num++;
        }
        *list_size = num;
    }

    return 0;
}

/* This is a kernel API provided for other components to call on the device side */
int devdrv_query_host_pid_list(pid_t dev_pid, struct host_pid_info* host_pid_list, unsigned int in_size,
                               unsigned int* out_size)
{
    struct devdrv_manager_info *d_info = devdrv_get_manager_info();
    struct devdrv_process_sign *d_sign = NULL;
    u32 bkt;
    u32 dev_id;
    u32 list_size = 0;
    int ret;

    if (host_pid_list == NULL || out_size == NULL) {
        devdrv_drv_err("Invalid para. (host_pid_list is NULL=%d, len is NULL=%d)\n",
            host_pid_list == NULL, out_size == NULL);
        return -EINVAL;
    }

    spin_lock_bh(&d_info->proc_hash_table_lock);

    hash_for_each(d_info->proc_hash_table, bkt, d_sign, link) {
        for (dev_id = 0; dev_id < PID_MAP_DEVNUM; dev_id++) {
            ret = find_sign_and_get_hostpid(d_sign, dev_pid, dev_id, host_pid_list, in_size, &list_size);
            if (ret != 0) {
                spin_unlock_bh(&d_info->proc_hash_table_lock);
                devdrv_drv_err("The size of the host_pid_list is too small. (size=%u)\n", in_size);
                return ret;
            }
        }
    }

    spin_unlock_bh(&d_info->proc_hash_table_lock);

    *out_size = list_size;
    if (list_size == 0) {
        devdrv_drv_warn("The device pid didn't bind any host pid. (device pid=%d)\n", dev_pid);
        return -ESRCH;
    }

    return 0;
}
EXPORT_SYMBOL(devdrv_query_host_pid_list);

int devdrv_query_process_by_host_pid_kernel(unsigned int host_pid,
    unsigned int chip_id, enum devdrv_process_type cp_type, unsigned int vfid, int *pid)
{
    struct devdrv_manager_info *d_info = devdrv_get_manager_info();
    struct devdrv_process_sign *d_sign = NULL;
    u32 key;

    if ((d_info == NULL) || (pid == NULL) || (cp_type < 0) ||
        (cp_type >= DEVDRV_PROCESS_CPTYPE_MAX) || (vfid >= VFID_NUM_MAX)) {
        devdrv_drv_err("host_pid %u null ptr or invalid cp_type(%u) vfid(%u).\n", host_pid, cp_type, vfid);
        return -EINVAL;
    }

    if ((chip_id >= PID_MAP_DEVNUM) || (cp_type == DEVDRV_PROCESS_USER)) {
        devdrv_drv_debug("Not surpport. (chip id=%u, cp_type=%u)\n", chip_id, cp_type);
        return -EINVAL;
    }

    key = (u32)host_pid & DEVDRV_PROC_HASH_TABLE_MASK;
    spin_lock_bh(&d_info->proc_hash_table_lock);
    /*lint -e666 */
    hash_for_each_possible(d_info->proc_hash_table, d_sign, link, key) {
        if (d_sign->hostpid == host_pid) {
            *pid = d_sign->devpid[chip_id][vfid][cp_type];
            if ((cp_type == DEVDRV_PROCESS_CP2) && (*pid == DEVMNG_PID_INVALID)) {
                *pid = d_sign->devpid[chip_id][vfid][DEVDRV_PROCESS_CP1];
            }
            spin_unlock_bh(&d_info->proc_hash_table_lock);
            if (*pid == DEVMNG_PID_INVALID) {
                devdrv_drv_debug("The hostpid didn't bind this type device pid."
                    "(hostpid=%u, device id=%u, vfid=%u, cptype=%u)\n", host_pid, chip_id, vfid, cp_type);
                return DRV_ERROR_NO_PROCESS;
            }
            return 0;
        }
    }
    spin_unlock_bh(&d_info->proc_hash_table_lock);
    devdrv_drv_debug("Cannot find the host pid in hashtable, (host_pid=%u, cp_type=%u, chip_id=%u, vfid=%u)\n",
        host_pid, cp_type, chip_id, vfid);

    return DRV_ERROR_NO_PROCESS;
}
EXPORT_SYMBOL(devdrv_query_process_by_host_pid_kernel);

int devdrv_query_process_by_host_pid_user(unsigned int host_pid,
    unsigned int chip_id, enum devdrv_process_type cp_type, unsigned int vfid, int *pid)
{
    int ret;

#ifdef CFG_FEATURE_BIND_TGID
    pid_t tgid = 0;
    if (devdrv_get_tgid_by_pid(host_pid, &tgid) != 0) {
        devdrv_drv_err("Failed to get tgid by pid. (pid=%d)\n", host_pid);
        return -EINVAL;
    }

    host_pid = tgid;
#endif

    ret = devdrv_query_process_by_host_pid_kernel(host_pid, chip_id, cp_type, vfid, pid);
    if (ret == 0) {
#if defined(CFG_HOST_ENV) || defined(CFG_FEATURE_DEVICE_CONTAINER)
        if (devdrv_devpid_container_convert(pid) != 0) {
            devdrv_drv_err("Failed to convert pid. (pid=%d)\n", *pid);
            return -EINVAL;
        }
#endif
        return 0;
    }

    if (ret == DRV_ERROR_NO_PROCESS) {
        return ret;
    } else {
        return -EINVAL;
    }
}

int devdrv_query_process_by_host_pid(unsigned int host_pid,
    unsigned int chip_id, enum devdrv_process_type cp_type, unsigned int vfid, int *pid)
{
    return devdrv_query_process_by_host_pid_kernel(host_pid, chip_id, cp_type, vfid, pid);
}
EXPORT_SYMBOL(devdrv_query_process_by_host_pid);

int devdrv_query_process_host_pid(int pid, unsigned int *chip_id, unsigned int *vfid, unsigned int *host_pid,
    enum devdrv_process_type *cp_type)
{
    struct devdrv_manager_info *d_info = devdrv_get_manager_info();
    struct devdrv_process_sign *d_sign = NULL;
    u32 bkt = 0;
    int i, j;

    if ((chip_id == NULL) || (vfid == NULL) || (host_pid == NULL) || (cp_type == NULL)) {
        devdrv_drv_err("null ptr.\n");
        return -EINVAL;
    }

    spin_lock_bh(&d_info->proc_hash_table_lock);

    hash_for_each(d_info->proc_hash_table, bkt, d_sign, link) {
        struct devdrv_process_user_info *user_proc = NULL;
        for (i = 0; i < PID_MAP_DEVNUM; i++) {
            for (j = 0; j < VFID_NUM_MAX; j++) {
                if (d_sign->devpid[i][j][DEVDRV_PROCESS_CP1] == pid) {
                    *cp_type = DEVDRV_PROCESS_CP1;
                } else if (d_sign->devpid[i][j][DEVDRV_PROCESS_CP2] == pid) {
                    *cp_type = DEVDRV_PROCESS_CP2;
                } else if (d_sign->devpid[i][j][DEVDRV_PROCESS_QS] == pid) {
                    *cp_type = DEVDRV_PROCESS_QS;
                } else if (d_sign->devpid[i][j][DEVDRV_PROCESS_DEV_ONLY] == pid) {
                    *cp_type = DEVDRV_PROCESS_DEV_ONLY;
                } else if (d_sign->devpid[i][j][DEVDRV_PROCESS_HCCP] == pid) {
                    *cp_type = DEVDRV_PROCESS_HCCP;
                } else {
                    continue;
                }
                *chip_id = i;
                *vfid = j;
                *host_pid = d_sign->hostpid;
                spin_unlock_bh(&d_info->proc_hash_table_lock);
                return 0;
            }
        }

        user_proc = devdrv_query_user_proc(d_sign, devdrv_get_cur_run_side(), pid);
        if (user_proc != NULL) {
            *chip_id = user_proc->devid;
            *vfid = user_proc->vfid;
            *host_pid = d_sign->hostpid;
            *cp_type = DEVDRV_PROCESS_USER;
            spin_unlock_bh(&d_info->proc_hash_table_lock);
            return 0;
        }
    }

    spin_unlock_bh(&d_info->proc_hash_table_lock);

    return -EINVAL;
}
EXPORT_SYMBOL(devdrv_query_process_host_pid);

#ifdef CFG_HOST_ENV
/* only surport cp proc */
int devdrv_query_master_pid_by_device_slave(u32 udevid, int slave_pid, u32 *master_pid)
{
    struct devdrv_manager_info *d_info = devdrv_get_manager_info();
    struct devdrv_process_sign *d_sign = NULL;
    u32 bkt = 0, vfid = 0;

    if ((master_pid == NULL) || (udevid >= PID_MAP_DEVNUM)) {
        return -EINVAL;
    }

    spin_lock_bh(&d_info->proc_hash_table_lock);

    hash_for_each(d_info->proc_hash_table, bkt, d_sign, link) {
        /* host has cp, store device cp in dev_only */
        if (d_sign->devpid[udevid][vfid][DEVDRV_PROCESS_DEV_ONLY] == slave_pid) {
            *master_pid = d_sign->hostpid;
            spin_unlock_bh(&d_info->proc_hash_table_lock);
            return 0;
        }
    }

    spin_unlock_bh(&d_info->proc_hash_table_lock);

    return -EINVAL;
}

EXPORT_SYMBOL(devdrv_query_master_pid_by_device_slave);
#else
/* only surport user proc */
int devdrv_query_master_pid_by_host_slave(int slave_pid, u32 *master_pid)
{
    struct devdrv_manager_info *d_info = devdrv_get_manager_info();
    struct devdrv_process_sign *d_sign = NULL;
    u32 bkt = 0;

    if (master_pid == NULL) {
        return -EINVAL;
    }

    spin_lock_bh(&d_info->proc_hash_table_lock);
    hash_for_each(d_info->proc_hash_table, bkt, d_sign, link) {
        struct devdrv_process_user_info *user_proc = devdrv_get_user_proc(d_sign, HOST_SIDE);
        int i;

        for (i = 0; i < DEVMNG_USER_PROC_MAX; i++) {
            if ((user_proc[i].valid == 1) && (user_proc[i].pid == slave_pid)) {
                *master_pid = d_sign->hostpid;
                spin_unlock_bh(&d_info->proc_hash_table_lock);
                return 0;
            }
        }
    }

    spin_unlock_bh(&d_info->proc_hash_table_lock);

    return -EINVAL;
}
EXPORT_SYMBOL(devdrv_query_master_pid_by_host_slave);
#endif

STATIC unsigned int devdrv_query_process_host_pids_detail_by_pid(struct devdrv_process_sign *d_sign, int pid,
    unsigned int node, unsigned int vfid, devdrv_host_pids_info_t *host_pids_info)
{
    unsigned int i;
    unsigned int query_count = 0;

    for (i = 0; i < DEVDRV_PROCESS_CPTYPE_MAX; i++) {
        if (d_sign->devpid[node][vfid][i] == pid) {
            host_pids_info->cp_type[query_count] = i;
            host_pids_info->chip_id[query_count] = node;
            host_pids_info->vfid[query_count] = vfid;
            host_pids_info->host_pids[query_count] = d_sign->hostpid;
            query_count++;
        }
    }

    return query_count;
}

int devdrv_query_process_host_pids_by_pid(int pid, devdrv_host_pids_info_t *host_pids_info)
{
    struct devdrv_manager_info *d_info = devdrv_get_manager_info();
    struct devdrv_process_sign *d_sign = NULL;
    struct devdrv_process_user_info *user_proc = NULL;
    unsigned int query_count = 0;
    unsigned int i, j;
    u32 bkt = 0;

    if (host_pids_info == NULL) {
        devdrv_drv_err("Input host pid info is null.\n");
        return -EINVAL;
    }

    host_pids_info->vaild_num = 0;
    spin_lock_bh(&d_info->proc_hash_table_lock);
    hash_for_each(d_info->proc_hash_table, bkt, d_sign, link) {
        for (i = 0; i < PID_MAP_DEVNUM; i++) {
            for (j = 0; j < VFID_NUM_MAX; j++) {
                query_count = devdrv_query_process_host_pids_detail_by_pid(d_sign, pid, i, j, host_pids_info);
                if (query_count != 0) {
                    host_pids_info->vaild_num = query_count;
                    spin_unlock_bh(&d_info->proc_hash_table_lock);
                    return 0;
                }
            }
        }

        user_proc = devdrv_query_user_proc(d_sign, devdrv_get_cur_run_side(), pid);
        if (user_proc != NULL) {
            host_pids_info->cp_type[query_count] = DEVDRV_PROCESS_USER;
            host_pids_info->chip_id[query_count] = user_proc->devid;
            host_pids_info->vfid[query_count] = user_proc->vfid;
            host_pids_info->host_pids[query_count] = d_sign->hostpid;
            query_count++;
            host_pids_info->vaild_num = query_count;
            spin_unlock_bh(&d_info->proc_hash_table_lock);
            return 0;
        }
    }
    spin_unlock_bh(&d_info->proc_hash_table_lock);

    if (query_count == 0) {
        devdrv_drv_warn("Can't find cp_pid in hashtable. (pid=%d)\n", pid);
    }

    return 0;
}
EXPORT_SYMBOL(devdrv_query_process_host_pids_by_pid);

int devdrv_check_hostpid(pid_t hostpid, unsigned int chip_id, unsigned int vfid)
{
    struct devdrv_manager_info *d_info = NULL;
    struct devdrv_process_sign *d_sign = NULL;
    u32 key, i;

    if (vfid >= VFID_NUM_MAX || chip_id >= PID_MAP_DEVNUM) {
        devdrv_drv_err("vfid(%u) or chip_id(%u) is invalid\n", vfid, chip_id);
        return -EINVAL;
    }

    d_info = devdrv_get_manager_info();
    if (d_info == NULL) {
        devdrv_drv_err("d_info is null, hostpid:%d.\n", hostpid);
        return -EINVAL;
    }

    key = (u32)hostpid & DEVDRV_PROC_HASH_TABLE_MASK;
    spin_lock_bh(&d_info->proc_hash_table_lock);
    /*lint -e666 */
    hash_for_each_possible(d_info->proc_hash_table, d_sign, link, key) {
        if (d_sign->hostpid != hostpid) {
            continue;
        }
        for (i = DEVDRV_PROCESS_CP1; i < DEVDRV_PROCESS_CPTYPE_MAX; i++) {
            if (d_sign->devpid[chip_id][vfid][i] == current->tgid) {
                spin_unlock_bh(&d_info->proc_hash_table_lock);
                return 0;
            }
        }
    }
    spin_unlock_bh(&d_info->proc_hash_table_lock);
    devdrv_drv_err("hostpid(%d) or chip_id(%u) or vfid(%u) is error.\n", hostpid, chip_id, vfid);
    return -EINVAL;
}
EXPORT_SYMBOL(devdrv_check_hostpid);

int devdrv_check_sign(pid_t hostpid, const char *sign, u32 len)
{
    struct devdrv_manager_info *d_info = devdrv_get_manager_info();
    struct devdrv_process_sign *d_sign = NULL;
    u32 key;

    if ((sign == NULL) || (len != PROCESS_SIGN_LENGTH) || (d_info == NULL)) {
        devdrv_drv_err("sign is NULL(%d) or invalid length(%u) or d_info is NULL(%d).\n",
            (sign == NULL), len, (d_info == NULL));
        return -EINVAL;
    }

    key = (u32)hostpid & DEVDRV_PROC_HASH_TABLE_MASK;
    spin_lock_bh(&d_info->proc_hash_table_lock);
    /*lint -e666 */
    hash_for_each_possible(d_info->proc_hash_table, d_sign, link, key) {
        if (d_sign->hostpid == hostpid) {
            /* The sign is not used, not need check. */
            spin_unlock_bh(&d_info->proc_hash_table_lock);
            return 0;
        }
    }
    spin_unlock_bh(&d_info->proc_hash_table_lock);
    return -EINVAL;
}
EXPORT_SYMBOL(devdrv_check_sign);

int devdrv_get_dev_process(pid_t devpid)
{
    struct devdrv_manager_info *d_info = devdrv_get_manager_info();
    struct devdrv_process_sign *d_sign = NULL;
    u32 bkt = 0;
    int i, j;

    if (d_info == NULL) {
        return 0;
    }

    spin_lock_bh(&d_info->proc_hash_table_lock);
    hash_for_each(d_info->proc_hash_table, bkt, d_sign, link) {
        for (i = 0; i < PID_MAP_DEVNUM; i++) {
            for (j = 0; j < VFID_NUM_MAX; j++) {
                if ((d_sign->devpid[i][j][DEVDRV_PROCESS_CP1] != devpid) &&
                    (d_sign->devpid[i][j][DEVDRV_PROCESS_CP2] != devpid) &&
                    (d_sign->devpid[i][j][DEVDRV_PROCESS_QS] != devpid) &&
                    (d_sign->devpid[i][j][DEVDRV_PROCESS_DEV_ONLY] != devpid) &&
                    (d_sign->devpid[i][j][DEVDRV_PROCESS_HCCP] != devpid)) {
                    continue;
                }

                if (d_sign->host_process_status != 0) {
                    spin_unlock_bh(&d_info->proc_hash_table_lock);
                    return -EOWNERDEAD;
                }
                d_sign->in_use_count++;
                spin_unlock_bh(&d_info->proc_hash_table_lock);
                return 0;
            }
        }
    }
    spin_unlock_bh(&d_info->proc_hash_table_lock);

    return -ESRCH;
}
EXPORT_SYMBOL(devdrv_get_dev_process);

void devdrv_put_dev_process(pid_t devpid)
{
    struct devdrv_manager_info *d_info = devdrv_get_manager_info();
    struct devdrv_process_sign *d_sign = NULL;
    u32 bkt = 0;
    int i, j;

    if (d_info == NULL) {
        return;
    }

    spin_lock_bh(&d_info->proc_hash_table_lock);
    hash_for_each(d_info->proc_hash_table, bkt, d_sign, link) {
        for (i = 0; i < PID_MAP_DEVNUM; i++) {
            for (j = 0; j < VFID_NUM_MAX; j++) {
                if ((d_sign->devpid[i][j][DEVDRV_PROCESS_CP1] != devpid) &&
                    (d_sign->devpid[i][j][DEVDRV_PROCESS_CP2] != devpid) &&
                    (d_sign->devpid[i][j][DEVDRV_PROCESS_QS] != devpid) &&
                    (d_sign->devpid[i][j][DEVDRV_PROCESS_DEV_ONLY] != devpid) &&
                    (d_sign->devpid[i][j][DEVDRV_PROCESS_HCCP] != devpid)) {
                    continue;
                }
                d_sign->in_use_count =
                    (d_sign->in_use_count == 0) ? 0 : d_sign->in_use_count - 1;
                spin_unlock_bh(&d_info->proc_hash_table_lock);
                return;
            }
        }
    }
    spin_unlock_bh(&d_info->proc_hash_table_lock);

    return;
}
EXPORT_SYMBOL(devdrv_put_dev_process);

int devdrv_notice_process_exit(u32 host_pid)
{
    struct devdrv_manager_info *d_info = devdrv_get_manager_info();
    struct devdrv_process_sign *d_sign = NULL;
    u32 key = (u32)host_pid & DEVDRV_PROC_HASH_TABLE_MASK;

    if (d_info == NULL) {
        return 0;
    }

    spin_lock_bh(&d_info->proc_hash_table_lock);
    hash_for_each_possible(d_info->proc_hash_table, d_sign, link, key) {
        if (d_sign->hostpid == host_pid) {
            int ret = d_sign->in_use_count == 0 ? 0 : -EBUSY;
            d_sign->host_process_status = EOWNERDEAD;
            spin_unlock_bh(&d_info->proc_hash_table_lock);
            return ret;
        }
    }
    spin_unlock_bh(&d_info->proc_hash_table_lock);

    return 0;
}

int devdrv_fop_query_host_pid(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_ioctl_para_query_pid para_info = {0};
    int node_id = numa_node_id();
    unsigned int phy_id = 0;
    unsigned int vfid = 0;
    int ret;
    pid_t tgid = 0;

    ret = devdrv_manager_container_logical_id_to_physical_id(0, &phy_id, &vfid);
    if (ret != 0) {
        devdrv_drv_err("Transfer logical id to physical id failed. (ret=%d)\n", ret);
        return ret;
    }

    if (!devdrv_manager_is_pf_device(phy_id) || (vfid > 0)) {
        devdrv_drv_debug("Not support split mode. (phy_id=%u; vfid=%u)\n", phy_id, vfid);
        return -EPERM;
    }

    if (copy_from_user_safe(&para_info, (void *)(uintptr_t)arg, sizeof(struct devdrv_ioctl_para_query_pid))) {
        devdrv_drv_err("copy_from_user error. dev_id:%d\n", node_id);
        return -EINVAL;
    }

    if (para_info.pid <= 0) {
        devdrv_drv_err("invalid para pid(%d).\n", para_info.pid);
        return -EINVAL;
    }

#if defined(CFG_HOST_ENV) || defined(CFG_FEATURE_BIND_TGID)
    if (devdrv_get_tgid_by_pid(para_info.pid, &tgid) != 0) {
        devdrv_drv_err("Failed to get tgid by pid. (pid=%d)\n", para_info.pid);
        return -EINVAL;
    }
#else
    tgid = para_info.pid;
#endif

    ret = devdrv_query_process_host_pid(tgid, &para_info.chip_id, &para_info.vfid, &para_info.host_pid,
        &para_info.cp_type);
    if (ret) {
        return -ESRCH;
    }
    devdrv_drv_debug("Query host_pid information. "
        "(node_id=%d; dev_id=%u, dev_pid=%d, tgid=%d, vfid=%u, hostpid=%d)",
        node_id, para_info.chip_id, tgid, current->tgid, para_info.vfid, para_info.host_pid);
#if defined(CFG_HOST_ENV) || defined(CFG_FEATURE_DEVICE_CONTAINER)
    if (devdrv_devpid_container_convert(&para_info.host_pid) != 0) {
        devdrv_drv_err("Failed to convert devpid. (pid=%d)\n", para_info.host_pid);
        return -EINVAL;
    }
#endif

    ret = copy_to_user_safe((void *)(uintptr_t)arg, &para_info, sizeof(struct devdrv_ioctl_para_query_pid));
    if (ret != 0) {
        devdrv_drv_err("copy to user failed, ret(%d)\n", ret);
        return -EINVAL;
    }

    return 0;
}

#ifdef UT_VCAST
STATIC void devdrv_manager_set_sign_print(struct devdrv_process_sign *d_sign, u32 cp_type,
    u32 chip_id, u32 vfid, struct devdrv_process_sign *sign_print)
#else
STATIC inline void devdrv_manager_set_sign_print(struct devdrv_process_sign *d_sign, u32 cp_type,
    u32 chip_id, u32 vfid, struct devdrv_process_sign *sign_print)
#endif
{
    sign_print->hostpid = d_sign->hostpid;
    sign_print->devpid[chip_id][vfid][cp_type] = d_sign->devpid[chip_id][vfid][cp_type];
}

void devdrv_release_sync_to_peer(pid_t master_pid, pid_t slave_pid,
    unsigned int dev_id, unsigned int vf_id, enum devdrv_process_type cp_type)
{
    struct devdrv_ioctl_para_bind_host_pid para_info;
    para_info.chip_id = dev_id;
    para_info.vfid = vf_id;
    para_info.cp_type = cp_type;
    para_info.host_pid = master_pid;
    para_info.mode = AICPUFW_ONLINE_PLAT;

    devdrv_drv_info("Release sync. (master_pid=%d; slave_pid=%d; dev_id=%u; vf_id=%u; cp_type=%d)\n",
        master_pid, slave_pid, dev_id, vf_id, cp_type);

    (void)devdrv_pid_map_sync_to_peer(&para_info, slave_pid, 0);
}

void devdrv_release_try_to_sync_to_peer(pid_t slave_pid)
{
    unsigned int chip_id, vfid, master_pid;
    enum devdrv_process_type cp_type;

    int ret = devdrv_query_process_host_pid(slave_pid, &chip_id, &vfid, &master_pid, &cp_type);
    if (ret != 0) {
        return;
    }

    if (slave_pid == master_pid) { /* thread mode bind self */
        return;
    }

#ifdef CFG_HOST_ENV
    if (cp_type == DEVDRV_PROCESS_USER) {
        devdrv_release_sync_to_peer(master_pid, slave_pid, chip_id, vfid, cp_type);
    }
#else
    if (cp_type == DEVDRV_PROCESS_CP1) {
        devdrv_release_sync_to_peer(master_pid, slave_pid, chip_id, vfid, cp_type);
    }
#endif
}

STATIC void devdrv_manager_release_devpid(struct devdrv_process_sign *d_sign, pid_t devpid,
    struct devdrv_process_sign *sign_print, u32 *out_chip_id, u32 *out_vfid, struct list_head *free_list)
{
    struct devdrv_process_user_info *user_proc = devdrv_query_user_proc(d_sign, devdrv_get_cur_run_side(), devpid);
    u32 chip_id, vfid;

    if (user_proc != NULL) {
        d_sign->cp_count--;
        *out_chip_id = user_proc->devid;
        *out_vfid = user_proc->vfid;
        user_proc->valid = 0;
    }

    for (chip_id = 0; chip_id < PID_MAP_DEVNUM; chip_id++) {
        for (vfid = 0; vfid < VFID_NUM_MAX; vfid++) {
            if (d_sign->devpid[chip_id][vfid][DEVDRV_PROCESS_CP1] == devpid) {
                devdrv_manager_set_sign_print(d_sign, DEVDRV_PROCESS_CP1, chip_id, vfid, sign_print);
                devdrv_manager_set_sign_print(d_sign, DEVDRV_PROCESS_CP2, chip_id, vfid, sign_print);
                d_sign->devpid[chip_id][vfid][DEVDRV_PROCESS_CP1] = DEVMNG_PID_INVALID;
                d_sign->devpid[chip_id][vfid][DEVDRV_PROCESS_CP2] = DEVMNG_PID_INVALID;
                d_sign->cp_count--;
                *out_chip_id = chip_id;
                *out_vfid = vfid;
            } else if ((d_sign->devpid[chip_id][vfid][DEVDRV_PROCESS_CP2] == devpid) &&
                (d_sign->devpid[chip_id][vfid][DEVDRV_PROCESS_CP1] != DEVMNG_PID_INVALID)) {
                devdrv_manager_set_sign_print(d_sign, DEVDRV_PROCESS_CP2, chip_id, vfid, sign_print);
                d_sign->devpid[chip_id][vfid][DEVDRV_PROCESS_CP2] = DEVMNG_PID_START_ONCE;
                *out_chip_id = chip_id;
                *out_vfid = vfid;
            } else if (d_sign->devpid[chip_id][vfid][DEVDRV_PROCESS_CP2] == devpid) {
                devdrv_manager_set_sign_print(d_sign, DEVDRV_PROCESS_CP2, chip_id, vfid, sign_print);
                d_sign->devpid[chip_id][vfid][DEVDRV_PROCESS_CP2] = DEVMNG_PID_INVALID;
                *out_chip_id = chip_id;
                *out_vfid = vfid;
            }
            if (d_sign->devpid[chip_id][vfid][DEVDRV_PROCESS_DEV_ONLY] == devpid) {
                devdrv_manager_set_sign_print(d_sign, DEVDRV_PROCESS_DEV_ONLY, chip_id, vfid, sign_print);
                d_sign->devpid[chip_id][vfid][DEVDRV_PROCESS_DEV_ONLY] = DEVMNG_PID_INVALID;
                d_sign->cp_count--;
                *out_chip_id = chip_id;
                *out_vfid = vfid;
            }
            if (d_sign->devpid[chip_id][vfid][DEVDRV_PROCESS_QS] == devpid) {
                devdrv_manager_set_sign_print(d_sign, DEVDRV_PROCESS_QS, chip_id, vfid, sign_print);
                d_sign->devpid[chip_id][vfid][DEVDRV_PROCESS_QS] = DEVMNG_PID_INVALID;
                d_sign->cp_count--;
                *out_chip_id = chip_id;
                *out_vfid = vfid;
            }
            if (d_sign->devpid[chip_id][vfid][DEVDRV_PROCESS_HCCP] == devpid) {
                devdrv_manager_set_sign_print(d_sign, DEVDRV_PROCESS_HCCP, chip_id, vfid, sign_print);
                d_sign->devpid[chip_id][vfid][DEVDRV_PROCESS_HCCP] = DEVMNG_PID_INVALID;
                d_sign->cp_count--;
                *out_chip_id = chip_id;
                *out_vfid = vfid;
            }
        }
    }

    if ((d_sign->cp_count == 0) && (d_sign->sync_proc_cnt == 0)) {
        hash_del(&d_sign->link);
        list_add(&d_sign->list, free_list);
    }
}

void devdrv_manager_process_sign_release(pid_t devpid)
{
    struct devdrv_manager_info *d_info = devdrv_get_manager_info();
    struct devdrv_process_sign *d_sign_hostpid = NULL, *d_sign_devpid = NULL;
    struct hlist_node *local_sign = NULL;
    struct devdrv_process_sign *free_sign = NULL, *free_sign_tmp = NULL;
    struct devdrv_process_sign *sign_print = NULL;
    u32 bkt;
    u32 chip_id = 0;
    u32 vfid = 0;
    struct list_head *pos = NULL;
    struct list_head *n = NULL;
    struct list_head free_list_head;
    INIT_LIST_HEAD(&free_list_head);

    if (d_info == NULL) {
        devdrv_drv_err("dev_manager_info is NULL, devpid %d.\n", devpid);
        return;
    }

    /* for host side */
    mutex_lock(&d_info->devdrv_sign_list_lock);
    if (!list_empty_careful(&d_info->hostpid_list_header)) {
        list_for_each_safe(pos, n, &d_info->hostpid_list_header) {
            d_sign_hostpid = list_entry(pos, struct devdrv_process_sign, list);
            if (d_sign_hostpid->hostpid == devpid) {
                devdrv_drv_info("Delete hostpid sign list node. (hostpid=%d)\n", d_sign_hostpid->hostpid);
                list_del(&d_sign_hostpid->list);
                d_info->devdrv_sign_count[d_sign_hostpid->docker_id]--;
                vfree(d_sign_hostpid);
                d_sign_hostpid = NULL;
                break;
            }
        }
    }
    mutex_unlock(&d_info->devdrv_sign_list_lock);

    devdrv_release_try_to_sync_to_peer(devpid);

    sign_print = ka_vmalloc(sizeof(struct devdrv_process_sign), GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT, PAGE_KERNEL);
    if (sign_print == NULL) {
        devdrv_drv_err("vzalloc failed.\n");
        return;
    }

    spin_lock_bh(&d_info->proc_hash_table_lock);
    hash_for_each_safe(d_info->proc_hash_table, bkt, local_sign, d_sign_devpid, link) {
        /* release devpid if match */
        devdrv_manager_release_devpid(d_sign_devpid, devpid, sign_print, &chip_id, &vfid, &free_list_head);
    }
    spin_unlock_bh(&d_info->proc_hash_table_lock);

    list_for_each_entry_safe(free_sign, free_sign_tmp, &free_list_head, list) {
        list_del(&free_sign->list);
        devdrv_drv_info("Destroy master pid ctx when proc exit. (hostpid=%d)", free_sign->hostpid);
        vfree(free_sign);
        free_sign = NULL;
    }

    /* print information about hashnode when release devpid */
    if (sign_print->hostpid != 0) {
        devdrv_drv_info("release hostpid(%d), devpid(%d), cp1_pid(%d), cp2_pid(%d), "
                        "dev_only_pid(%d), qs_pid(%d), hccp_pid(%d), chip_id(%u), vfid(%u).\n",
                        sign_print->hostpid, devpid,
                        sign_print->devpid[chip_id][vfid][DEVDRV_PROCESS_CP1],
                        sign_print->devpid[chip_id][vfid][DEVDRV_PROCESS_CP2],
                        sign_print->devpid[chip_id][vfid][DEVDRV_PROCESS_DEV_ONLY],
                        sign_print->devpid[chip_id][vfid][DEVDRV_PROCESS_QS],
                        sign_print->devpid[chip_id][vfid][DEVDRV_PROCESS_HCCP],
                        chip_id, vfid);
    }

    vfree(sign_print);
    sign_print = NULL;
    return;
}

void devdrv_manager_free_hashtable(void)
{
    struct devdrv_process_sign *d_sign = NULL;
    struct hlist_node *local_sign = NULL;
    u32 bkt;

    if (dev_manager_info == NULL) {
        devdrv_drv_err("dev_manager_info is NULL\n");
        return;
    }

    hash_for_each_safe(dev_manager_info->proc_hash_table, bkt, local_sign, d_sign, link) {
        hash_del(&d_sign->link);
        vfree(d_sign);
        d_sign = NULL;
    }
    return;
}
#else
int devdrv_manager_pid_map(void)
{
    return 0;
}

int devdrv_query_process_by_host_pid(unsigned int host_pid,
    unsigned int chip_id, enum devdrv_process_type cp_type, unsigned int vfid, int *pid)
{
    return 0;
}
EXPORT_SYMBOL(devdrv_query_process_by_host_pid);
#endif
