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
#include <linux/spinlock.h>
#include <linux/hashtable.h>
#include <linux/slab.h>
#include <linux/securec.h>

#include "dp_proc_mng_channel.h"
#include "ascend_hal_define.h"
#include "dp_proc_mng_proc_info.h"
#include "dp_proc_mng_agent_msg_client.h"
#include "dp_proc_mng_pid_map.h"

int dp_proc_mng_check_process_sign(pid_t hostpid, const char *sign, u32 len)
{
    struct dp_proc_mng_chan_check_process_sign dev_manager_msg_info = {{{0}}};
    u32 dev_id;
    int ret;

    dev_manager_msg_info.head.msg_id = DP_PROC_MNG_CHAN_CHECK_PROCESS_SIGN_D2H;
    dev_manager_msg_info.head.process_id.hostpid = hostpid;

    ret = strcpy_s(dev_manager_msg_info.sign, PROCESS_SIGN_LENGTH, sign);
    if (ret != 0) {
        dp_proc_mng_drv_err("strcpy_s failed, ret(%d).\n", ret);
        return -EINVAL;
    }

    dev_id = cpu_to_node((int)smp_processor_id()); /*lint !e666  !e453*/
    dev_manager_msg_info.head.process_id.devid = dev_id;
    ret = dp_proc_mng_agent_common_msg_send(dev_id, &dev_manager_msg_info, sizeof(dev_manager_msg_info),
        sizeof(dev_manager_msg_info));
    if (ret != 0) {
        dp_proc_mng_drv_err("common msg send failed, ret(%d)\n", ret);
        return -ENODEV;
    }

    if (dev_manager_msg_info.head.result != 0) {
        dp_proc_mng_drv_err("check process_sign failed, result(%u)\n",
            dev_manager_msg_info.head.result);
        return -ENODEV;
    }

    return 0;
}

struct dp_proc_mng_sign *dp_proc_mng_find_process_sign(struct dp_proc_mng_info *d_info,
    pid_t hostpid)
{
    struct dp_proc_mng_sign *proc_sign = NULL;
    u32 key;

    key = (u32)hostpid & DP_PROC_MNG_HASH_TABLE_MASK;
    /*lint -e666 */
    hash_for_each_possible(d_info->proc_hash_table, proc_sign, link, key) {
        if (proc_sign->hostpid == hostpid) {
            return proc_sign;
        }
    }
    return NULL;
}

STATIC struct dp_proc_mng_sign *dp_proc_mng_create_process_sign(struct dp_proc_mng_info *d_info,
    struct dp_proc_mng_bind_host_pid *para_info)
{
    struct dp_proc_mng_sign *d_sign = NULL;
    int ret;
    int i;
    int j;

    d_sign = kzalloc(sizeof(struct dp_proc_mng_sign), GFP_ATOMIC | GFP_ACCOUNT);
    if (d_sign == NULL) {
        return NULL;
    }
    d_sign->hostpid = para_info->host_pid;
    d_sign->cp_count = 0;
    for (i = 0; i < DP_PROC_MNG_MAX_NODE_NUM; i++) {
        for (j = 0; j < VFID_NUM_MAX; j++) {
            d_sign->devpid[i][j][PROCESS_CP1] = DP_PROC_MNG_PID_INVALID;
            d_sign->devpid[i][j][PROCESS_CP2] = DP_PROC_MNG_PID_INVALID;
            d_sign->devpid[i][j][PROCESS_DEV_ONLY] = DP_PROC_MNG_PID_INVALID;
            d_sign->devpid[i][j][PROCESS_QS] = DP_PROC_MNG_PID_INVALID;
        }
    }
    ret = strcpy_s(d_sign->sign, PROCESS_SIGN_LENGTH, para_info->sign);
    if (ret != 0) {
        kfree(d_sign);
        d_sign = NULL;
        return NULL;
    }
    return d_sign;
}

STATIC int dp_proc_mng_check_and_bind_hostpid(struct dp_proc_mng_sign *d_sign,
    struct dp_proc_mng_bind_host_pid *para_info, pid_t *hostpid)
{
    if (d_sign->devpid[para_info->chip_id][para_info->vfid][para_info->cp_type] == DP_PROC_MNG_PID_INVALID ||
        d_sign->devpid[para_info->chip_id][para_info->vfid][para_info->cp_type] == DP_PROC_MNG_PID_START_ONCE) {
        d_sign->devpid[para_info->chip_id][para_info->vfid][para_info->cp_type] = current->tgid;
        d_sign->cp_count += (para_info->cp_type == PROCESS_CP1 ? 1 : 0);
        d_sign->cp_count += (para_info->cp_type == PROCESS_DEV_ONLY ? 1 : 0);
        d_sign->cp_count += (para_info->cp_type == PROCESS_QS ? 1 : 0);
        d_sign->host_process_status = 0;
        return 0;
    }

    *hostpid = d_sign->devpid[para_info->chip_id][para_info->vfid][para_info->cp_type];
    d_sign->host_process_status = 0;
    return -EINVAL;
}

int dp_proc_mng_bind_host_pid(struct dp_proc_mng_bind_host_pid *para_info)
{
    struct dp_proc_mng_info *d_info = dp_proc_get_manager_info();
    struct dp_proc_mng_sign *d_sign = NULL;
    u32 key;
    int ret;
    pid_t ret_hostpid = 0;

    key = (u32)para_info->host_pid & DP_PROC_MNG_HASH_TABLE_MASK;
    spin_lock_bh(&d_info->proc_hash_table_lock);
    d_sign = dp_proc_mng_find_process_sign(d_info, para_info->host_pid);
#ifndef EMU_ST
    if (d_sign != NULL) {
        ret = dp_proc_mng_check_and_bind_hostpid(d_sign, para_info, &ret_hostpid);
        spin_unlock_bh(&d_info->proc_hash_table_lock);
        if (ret != 0) {
            dp_proc_mng_drv_err(
                "hostpid(%d) is already bound devpid[%d](%d), current devpid(%d), chip_id(%u) vfid(%u).\n",
                para_info->host_pid, para_info->cp_type, ret_hostpid, current->tgid, para_info->chip_id,
                para_info->vfid);
            return -EINVAL;
        }
        goto bind_succ;
    }
#endif

    d_sign = dp_proc_mng_create_process_sign(d_info, para_info);
#ifndef EMU_ST
    if (d_sign == NULL) {
        spin_unlock_bh(&d_info->proc_hash_table_lock);
        dp_proc_mng_drv_err("kzalloc sign failed or invalid sign, hostpid(%d).\n", para_info->host_pid);
        return -EINVAL;
    }
#endif
    ret = dp_proc_mng_check_and_bind_hostpid(d_sign, para_info, &ret_hostpid);
    if (ret != 0) {
        spin_unlock_bh(&d_info->proc_hash_table_lock);
        dp_proc_mng_drv_err("hostpid(%d) is already bound devpid[%d](%d), current devpid(%d), chip_id(%u) vfid(%u).\n",
            para_info->host_pid, para_info->cp_type, ret_hostpid, current->tgid, para_info->chip_id, para_info->vfid);
        kfree(d_sign);
        d_sign = NULL;
        return -EINVAL;
    }
    hash_add(d_info->proc_hash_table, &d_sign->link, key); //lint !e666
    spin_unlock_bh(&d_info->proc_hash_table_lock);

bind_succ:
    dp_proc_mng_drv_info("hostpid(%d) bind succ: devpid[%d](%d) para chip_id(%u) para vfid(%u)\n",
        para_info->host_pid, para_info->cp_type, current->tgid, para_info->chip_id, para_info->vfid);
    return 0;
}
