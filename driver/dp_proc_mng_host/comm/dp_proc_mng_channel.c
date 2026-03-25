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
#include "devdrv_manager_comm.h"
#include "dp_proc_mng_channel.h"

static int dp_proc_mng_chan_check_process_sign_d2h_process(void *msg, u32 *ack_len)
{
    struct dp_proc_mng_chan_check_process_sign *msg_info = NULL;
    struct dp_proc_mng_chan_msg_head *msg_head = NULL;
    struct dp_proc_mng_info *d_info = NULL;
    struct dp_proc_mng_sign *d_sign = NULL;
    struct list_head *pos = NULL;
    struct list_head *n = NULL;
    int ret = 0;
    u32 dev_id;

    msg_info = (struct dp_proc_mng_chan_check_process_sign *)msg;
    msg_head = &msg_info->head;

    dev_id = msg_info->head.process_id.devid;
    if ((dev_id >= DP_PROC_MNG_MAX_DEVICE_NUM)) {
        dp_proc_mng_drv_err("Invalid device id. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    d_info = dp_proc_get_manager_info();
    if (d_info == NULL) {
        dp_proc_mng_drv_err("d_info is null. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    mutex_lock(&d_info->dp_proc_mng_sign_list_lock);
    if (list_empty_careful(&d_info->hostpid_list_header) != 0) {
        dp_proc_mng_drv_err("Hostpid sign list is empty. (dev_id=%u; hostpid=%d)\n", dev_id,
            msg_head->process_id.hostpid);
        ret = -ESRCH;
        goto out;
    }
    list_for_each_safe(pos, n, &d_info->hostpid_list_header) {
        d_sign = list_entry(pos, struct dp_proc_mng_sign, list);
        if (d_sign->hostpid == msg_head->process_id.hostpid) {
            ret = devdrv_manager_container_check_devid_in_container(dev_id, d_sign->hostpid);
            if (ret != 0) {
                dp_proc_mng_drv_err("Device id and hostpid mismatch in container. (dev_id=%u; hostpid=%d; ret=%d)\n",
                    dev_id, d_sign->hostpid, ret);
                break;
            }

            dp_proc_mng_drv_info("Process sign check success. (dev_id=%u; hostpid=%d)\n", dev_id, d_sign->hostpid);
            goto out;
        }
    }

    dp_proc_mng_drv_err("Host pid is not in process sign list. (dev_id=%u; hostpid=%d)\n", dev_id,
        msg_head->process_id.hostpid);

out:
    mutex_unlock(&d_info->dp_proc_mng_sign_list_lock);
    *ack_len = sizeof(*msg_info);
    msg_info->head.result = (short)ret;
    return ret;
}

struct dp_proc_mng_chan_handlers_st dp_proc_mng_master_msg_processes[DP_PROC_MNG_CHAN_D2H_MAX_ID] = {
    [DP_PROC_MNG_CHAN_CHECK_PROCESS_SIGN_D2H] =
        {
            dp_proc_mng_chan_check_process_sign_d2h_process,
            sizeof(struct dp_proc_mng_chan_check_process_sign),
            0
        },
};

int dp_proc_mng_chan_msg_dispatch(void *msg, u32 in_data_len, u32 out_data_len, u32 *ack_len,
    const struct dp_proc_mng_chan_handlers_st *msg_process)
{
    struct dp_proc_mng_chan_msg_head *head_msg = (struct dp_proc_mng_chan_msg_head *)msg;
    u32 head_len = sizeof(struct dp_proc_mng_chan_msg_head);
    u32 msg_id;
    int ret;

    head_msg->result = 0;
    *ack_len = 0;
    msg_id = head_msg->msg_id;
    if ((msg_id >= DP_PROC_MNG_CHAN_D2H_MAX_ID) || (msg_process[msg_id].chan_msg_processes == NULL)) {
        dp_proc_mng_drv_err("Invalid message_id or none process func. (msg_id=%u)\n", msg_id);
        ret = -ENOMSG;
        goto save_msg_ret;
    }

#ifdef CFG_FEATURE_VFIO
    // later add vfio scene
#endif

    if (head_msg->process_id.vfid >= DP_PROC_MNG_MAX_VF_NUM) {
        dp_proc_mng_drv_err("Message_id has invalid. (msg_id=%u; vfid=%d)\n", msg_id, head_msg->process_id.vfid);
        ret = -EINVAL;
        goto save_msg_ret;
    }
    ret = msg_process[msg_id].chan_msg_processes(msg, ack_len);
save_msg_ret:
    if (ret != 0) {
        head_msg->result = (short)ret;
        *ack_len = (*ack_len > head_len) ? *ack_len : head_len;
        dp_proc_mng_drv_err("Host msg recv process failed. (devid=%u; ret=%d)\n", head_msg->process_id.devid, ret);
    }

    return 0;
}
