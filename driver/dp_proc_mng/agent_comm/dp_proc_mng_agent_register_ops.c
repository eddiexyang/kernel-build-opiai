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
#include <linux/fs.h>
#include <linux/slab.h>

#include "devdrv_interface.h"
#include "davinci_api.h"
#include "devdrv_interface.h"
#include "dp_proc_mng_cmd.h"
#include "dp_proc_mng_proc_info.h"
#include "dp_proc_mng_agent_msg_client.h"
#include "dp_proc_mng_pid_map.h"
#include "dp_proc_mng_register_ops.h"

STATIC inline void dp_proc_mng_set_sign_print(struct dp_proc_mng_sign *d_sign,
    u32 chip_id, u32 vfid, struct dp_proc_mng_sign *sign_print)
{
    int i;

    for (i = PROCESS_CP1; i < PROCESS_CPTYPE_MAX; i++) {
        sign_print->devpid[chip_id][vfid][i] =
            d_sign->devpid[chip_id][vfid][i];
    }
    sign_print->hostpid = d_sign->hostpid;
}

STATIC void dp_proc_mng_release_devpid(struct dp_proc_mng_sign *d_sign, pid_t devpid,
    struct dp_proc_mng_sign *sign_print, u32 *out_chip_id, u32 *out_vfid)
{
#ifndef EMU_ST
    u32 chip_id;
    u32 vfid;

    for (chip_id = 0; chip_id < DP_PROC_MNG_MAX_NODE_NUM; chip_id++) {
        for (vfid = 0; vfid < VFID_NUM_MAX; vfid++) {
            if ((d_sign->devpid[chip_id][vfid][PROCESS_CP1] == DP_PROC_MNG_PID_INVALID) &&
                (d_sign->devpid[chip_id][vfid][PROCESS_CP2] == DP_PROC_MNG_PID_INVALID) &&
                (d_sign->devpid[chip_id][vfid][PROCESS_DEV_ONLY] == DP_PROC_MNG_PID_INVALID) &&
                (d_sign->devpid[chip_id][vfid][PROCESS_QS] == DP_PROC_MNG_PID_INVALID)) {
                continue;
            }
            if (d_sign->devpid[chip_id][vfid][PROCESS_CP1] == devpid) {
                dp_proc_mng_set_sign_print(d_sign, chip_id, vfid, sign_print);
                d_sign->devpid[chip_id][vfid][PROCESS_CP1] = DP_PROC_MNG_PID_INVALID;
                d_sign->devpid[chip_id][vfid][PROCESS_CP2] = DP_PROC_MNG_PID_INVALID;
                d_sign->cp_count--;
                *out_chip_id = chip_id;
                *out_vfid = vfid;
            } else if ((d_sign->devpid[chip_id][vfid][PROCESS_CP2] == devpid) &&
                (d_sign->devpid[chip_id][vfid][PROCESS_CP1] != DP_PROC_MNG_PID_INVALID)) {
                dp_proc_mng_set_sign_print(d_sign, chip_id, vfid, sign_print);
                d_sign->devpid[chip_id][vfid][PROCESS_CP2] = DP_PROC_MNG_PID_START_ONCE;
                *out_chip_id = chip_id;
                *out_vfid = vfid;
            } else if (d_sign->devpid[chip_id][vfid][PROCESS_CP2] == devpid) {
                dp_proc_mng_set_sign_print(d_sign, chip_id, vfid, sign_print);
                d_sign->devpid[chip_id][vfid][PROCESS_CP2] = DP_PROC_MNG_PID_INVALID;
                *out_chip_id = chip_id;
                *out_vfid = vfid;
            } else if (d_sign->devpid[chip_id][vfid][PROCESS_DEV_ONLY] == devpid) {
                dp_proc_mng_set_sign_print(d_sign, chip_id, vfid, sign_print);
                d_sign->devpid[chip_id][vfid][PROCESS_DEV_ONLY] = DP_PROC_MNG_PID_INVALID;
                d_sign->cp_count--;
                *out_chip_id = chip_id;
                *out_vfid = vfid;
            } else if (d_sign->devpid[chip_id][vfid][PROCESS_QS] == devpid) {
                dp_proc_mng_set_sign_print(d_sign, chip_id, vfid, sign_print);
                d_sign->devpid[chip_id][vfid][PROCESS_QS] = DP_PROC_MNG_PID_INVALID;
                d_sign->cp_count--;
                *out_chip_id = chip_id;
                *out_vfid = vfid;
            }
        }
    }

    if (d_sign->cp_count == 0) {
        hash_del(&d_sign->link);
        kfree(d_sign);
        d_sign = NULL;
    }
#endif
}

void dp_process_sign_release(pid_t devpid)
{
    struct dp_proc_mng_info *d_info = dp_proc_get_manager_info();
    struct dp_proc_mng_sign *d_sign_hostpid = NULL, *d_sign_devpid = NULL;
    struct hlist_node *local_sign = NULL;
    struct dp_proc_mng_sign *sign_print = NULL;
    u32 bkt;
    u32 chip_id = 0;
    u32 vfid = 0;
    struct list_head *pos = NULL;
    struct list_head *n = NULL;

    if (d_info == NULL) {
        dp_proc_mng_drv_err("dev_manager_info is NULL, devpid %d.\n", devpid);
        return;
    }

#ifndef EMU_ST
    mutex_lock(&d_info->dp_proc_mng_sign_list_lock);
    if (!list_empty_careful(&d_info->hostpid_list_header)) {
        list_for_each_safe(pos, n, &d_info->hostpid_list_header) {
            d_sign_hostpid = list_entry(pos, struct dp_proc_mng_sign, list);
            if (d_sign_hostpid->hostpid == devpid) {
                dp_proc_mng_drv_info("Delete hostpid sign list node. (hostpid=%d)\n", d_sign_hostpid->hostpid);
                list_del(&d_sign_hostpid->list);
                d_info->dp_proc_mng_sign_count[d_sign_hostpid->docker_id]--;
                kfree(d_sign_hostpid);
                d_sign_hostpid = NULL;
                break;
            }
        }
    }
    mutex_unlock(&d_info->dp_proc_mng_sign_list_lock);
#endif

    sign_print = kzalloc(sizeof(struct dp_proc_mng_sign), GFP_ATOMIC | GFP_ACCOUNT);
    if (sign_print == NULL) {
        dp_proc_mng_drv_err("kzalloc failed.\n");
        return;
    }

    spin_lock_bh(&d_info->proc_hash_table_lock);
    hash_for_each_safe(d_info->proc_hash_table, bkt, local_sign, d_sign_devpid, link) {
        /* release devpid if match */
        dp_proc_mng_release_devpid(d_sign_devpid, devpid, sign_print, &chip_id, &vfid);
    }
    spin_unlock_bh(&d_info->proc_hash_table_lock);
    /* print information about hashnode when release devpid */
    if (sign_print->hostpid != 0) {
        dp_proc_mng_drv_info("release hostpid(%d), devpid(%d), cp1_pid(%d), cp2_pid(%d), "
            "dev_only_pid(%d), qs_pid(%d), chip_id(%u), vfid(%u).\n",
            sign_print->hostpid, devpid, sign_print->devpid[chip_id][vfid][PROCESS_CP1],
            sign_print->devpid[chip_id][vfid][PROCESS_CP2], sign_print->devpid[chip_id][vfid][PROCESS_DEV_ONLY],
            sign_print->devpid[chip_id][vfid][PROCESS_QS], chip_id, vfid);
    }

    kfree(sign_print);
    sign_print = NULL;
    return;
}

STATIC int dp_proc_mng_release_prepare(struct file *file_op, unsigned long mode)
{
    dp_process_sign_release(current->tgid);
    return 0;
}

const struct notifier_operations dp_agent_notifier_ops = {
    .notifier_call = dp_proc_mng_release_prepare,
};

int dp_proc_mng_register_hisi_oom_notifier(void)
{
    return 0;
}

void dp_proc_mng_unregister_hisi_oom_notifier(void)
{
    return;
}

int dp_proc_mng_register_ops_init(void)
{
    int ret;

    ret = drv_ascend_register_notify(DAVINCI_DP_PROC_MNG_SUB_MODULE_NAME, &dp_agent_notifier_ops);
    if (ret != 0) {
        dp_proc_mng_drv_err("Register agent notigy fail,(ret=%d).\n", ret);
        return ret;
    }

    ret = dp_proc_mng_register_hisi_oom_notifier();
    if (ret != 0) {
        dp_proc_mng_drv_err("Agent oom_handle register failed. (ret=%d).\n", ret);
        return ret;
    }

    ret = dp_proc_mng_agent_msg_chan_init_instance();

    return 0;
}

void dp_proc_mng_unregister_ops_init(void)
{
    dp_proc_mng_agent_msg_chan_uninit_instance();
    dp_proc_mng_unregister_hisi_oom_notifier();
}

