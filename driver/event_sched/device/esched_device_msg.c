/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
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

#include "devdrv_interface.h"
#include "securec.h"
#include "esched.h"
#include "esched_fops.h"
#include "esched_device_msg.h"

struct sched_msg_ops dev_msg_ops[SCHED_MAX_CHIP_NUM];

STATIC struct sched_msg_ops *esched_device_get_msg_ops(u32 dev_id)
{
    return &dev_msg_ops[dev_id];
}

STATIC int esched_device_msg_send(u32 dev_id, struct esched_ctrl_msg *msg, u32 msg_len)
{
#ifndef CFG_NOT_SURPPORT_PCIE_HOST_DEVICE_COMM
    int ret;
    u32 out_len;
    ret = agentdrv_common_msg_send(dev_id, (void *)msg, msg_len, msg_len, &out_len, AGENTDRV_COMMON_MSG_ESCHED);
    if ((ret != 0) || (msg->error_code != 0)) {
        sched_err("Failed to invoke the agentdrv_common_msg_send. "
                  "(dev_id=%u; error_code=%d; ret=%d)\n", dev_id, msg->error_code, ret);
        return ret != 0 ? ret : msg->error_code;
    }
#endif
    return 0;
}

STATIC int sched_remote_pid_check(int dev_pid, int host_pid)
{
    enum devdrv_process_type cp_type;
    u32 dev_id, vfid, master_pid0, master_pid1;
    int ret;

    ret = devdrv_query_process_host_pid(dev_pid, &dev_id, &vfid, &master_pid0, &cp_type);
    if (ret != 0) {
#if !defined(EVENT_SCHED_UT) && !defined(EMU_ST)
        sched_err("Query pid failed by cp. (dev_pid=%d)\n", dev_pid);
        return DRV_ERROR_NO_PROCESS;
#endif
    }

    if (host_pid == master_pid0) {
#if !defined(EVENT_SCHED_UT) && !defined(EMU_ST)
        return 0;
#endif
    }

    ret = devdrv_query_master_pid_by_host_slave(host_pid, &master_pid1);
    if (ret != 0) {
#if !defined(EVENT_SCHED_UT) && !defined(EMU_ST)
        sched_err("Query pid failed by host slave. (host_pid=%d)\n", host_pid);
        return DRV_ERROR_NO_PROCESS;
#endif
    }

    if (master_pid0 != master_pid1) {
#if !defined(EVENT_SCHED_UT) && !defined(EMU_ST)
        sched_err("Proc not match. (dev_pid=%u; host_pid=%d; dev2master_pid=%u; host2master_pid=%d)\n",
            dev_pid, host_pid, master_pid0, master_pid1);
        return DRV_ERROR_NO_PROCESS;
#endif
    }

    return 0;
}

STATIC int sched_publish_event_proxy(u32 devid, struct esched_ctrl_msg *msg)
{
    struct esched_remote_submit_msg *submit_msg = &msg->submit_msg;
    struct sched_published_event_func event_func;
    u32 chip_id = devid;
    int32_t ret;

    event_func.event_ack_func = NULL;
    event_func.event_finish_func = NULL;

    if (submit_msg->event_info.msg_len != 0) {
        submit_msg->event_info.msg = submit_msg->msg;
    } else {
        submit_msg->event_info.msg = NULL;
    }
    submit_msg->event_info.publish_timestamp = sched_get_cur_timestamp();

    ret = sched_publish_event_para_check(&submit_msg->event_info);
    if (ret != 0) {
        return ret;
    }

    ret = sched_remote_pid_check(submit_msg->event_info.pid, submit_msg->pid);
    if (ret != 0) {
        return ret;
    }

    return sched_publish_event(chip_id, SCHED_PUBLISH_FORM_KERNEL, &submit_msg->event_info, &event_func);
}

STATIC int sched_query_gid_proxy(u32 devid, struct esched_ctrl_msg *msg)
{
    struct esched_remote_query_gid_msg *query_msg = &msg->query_gid_msg;

    return sched_query_local_task_gid(devid, query_msg->pid, query_msg->grp_name, &query_msg->gid);
}

int sched_query_remote_task_gid_msg_send(u32 chip_id, int pid, const char *grp_name, u32 *gid)
{
    struct esched_ctrl_msg msg;
    int ret;

    msg.type = ESCHED_MSG_TYPE_REMOTE_QUERY_GID;
    msg.error_code = 0;
    msg.query_gid_msg.pid = pid;
    ret = strcpy_s(msg.query_gid_msg.grp_name, EVENT_MAX_GRP_NAME_LEN, grp_name);
    if (ret != 0) {
        sched_err("Failed to invoke strcpy_s. (chip_id=%u; ret=%d)\n", chip_id, ret);
        return DRV_ERROR_INNER_ERR;
    }

    ret = esched_device_msg_send(chip_id, (void *)&msg, sizeof(msg));
    if (ret == 0) {
        *gid = msg.query_gid_msg.gid;
    }

    return ret;
}

int sched_publish_event_to_remote(u32 chip_id, u32 event_src,
    const struct sched_published_event_info *event_info, struct sched_published_event_func *event_func)
{
    struct esched_ctrl_msg msg;
    u32 dev_id = chip_id;
    int ret;

    if ((event_func->event_ack_func != NULL) || (event_func->event_finish_func != NULL)) {
        sched_err("The callback function is not supported for remote submission.\n");
        return DRV_ERROR_PARA_ERROR;
    }

    msg.type = ESCHED_MSG_TYPE_REMOTE_SUBMIT;
    msg.error_code = 0;
    msg.submit_msg.event_info = *event_info;
    msg.submit_msg.pid = current->tgid;

    ret = sched_remote_pid_check(current->tgid, event_info->pid);
    if (ret != 0) {
        return ret;
    }

    if (event_info->msg_len > 0) {
        if (event_src == SCHED_PUBLISH_FORM_KERNEL) {
            ret = memcpy_s(msg.submit_msg.msg, SCHED_MAX_EVENT_MSG_LEN, event_info->msg, event_info->msg_len);
        } else {
            ret = copy_from_user_safe(msg.submit_msg.msg, (void *)event_info->msg, event_info->msg_len);
        }

        if (ret != 0) {
            sched_err("Failed to invoke the copy the variable msg. "
                      "(pid=%d; gid=%u; event_id=%u)\n", event_info->pid, event_info->gid, event_info->event_id);
            return ret;
        }
    }

    return esched_device_msg_send(dev_id, (void *)&msg, sizeof(msg));
}

STATIC int esched_ctrl_msg_recv(u32 devid, void *data, u32 in_data_len, u32 out_data_len, u32 *real_out_len)
{
    struct esched_ctrl_msg *msg = (struct esched_ctrl_msg *)data;
    struct sched_msg_ops *msg_ops = NULL;
#ifdef CFG_FEATURE_HARDWARE_SCHED
    struct sched_hard_res *res;
#endif
    int ret = 0;

    if ((devid >= SCHED_MAX_CHIP_NUM) || (data == NULL) ||
        (real_out_len == NULL) || (in_data_len != sizeof(struct esched_ctrl_msg))) {
        sched_err("The variable devid, data or real_out_len is invalid. "
                  "(devid=%u; in_data_len=%u; out_data_len=%u)\n", devid, in_data_len, out_data_len);
        return -EINVAL;
    }

    *real_out_len = sizeof(*msg);
    msg_ops = esched_device_get_msg_ops(devid);

#ifdef CFG_FEATURE_HARDWARE_SCHED
    res = esched_get_hard_res(devid);
    if ((res == NULL) || (res->init_flag != SCHED_VALID)) {
        sched_debug("Hard res not init yet. (dev_id=%u)\n", devid);
        return -EINVAL;
    }
#endif

    switch (msg->type) {
        case ESCHED_MSG_TYPE_ADD_HOST_PID:
            if (msg_ops->add_host_pid != NULL) {
                ret = msg_ops->add_host_pid(devid, &msg->host_pid_msg);
            }
            break;
        case ESCHED_MSG_TYPE_DEL_HOST_PID:
            if (msg_ops->del_host_pid != NULL) {
                ret = msg_ops->del_host_pid(devid, &msg->host_pid_msg);
            }
            break;
        case ESCHED_MSG_TYPE_ADD_POOL:
            if (msg_ops->add_host_pool != NULL) {
                ret = msg_ops->add_host_pool(devid, &msg->pool_msg);
            }
            break;
        case ESCHED_MSG_TYPE_GET_CPU_MBID:
            if (msg_ops->get_host_cpu_mbid != NULL) {
                ret = msg_ops->get_host_cpu_mbid(devid, &msg->mbid_msg);
            }
            break;
        case ESCHED_MSG_TYPE_ADD_MB:
            if (msg_ops->add_host_mb != NULL) {
                ret = msg_ops->add_host_mb(devid, &msg->mb_msg);
            }
            break;
        case ESCHED_MSG_TYPE_CONF_INTR:
            if (msg_ops->conf_host_intr != NULL) {
                ret = msg_ops->conf_host_intr(devid, &msg->intr_msg);
            }
            break;
        case ESCHED_MSG_TYPE_REMOTE_SUBMIT:
            ret = sched_publish_event_proxy(devid, msg);
            break;
        case ESCHED_MSG_TYPE_REMOTE_QUERY_GID:
            ret = sched_query_gid_proxy(devid, msg);
            break;
        default:
            ret = -EINVAL;
            break;
    }

    if ((ret != 0) && !((msg->type == ESCHED_MSG_TYPE_REMOTE_QUERY_GID) && (ret == DRV_ERROR_UNINIT))) {
        sched_err("Failed to invoke the deviched_ctrl_msg_recv. (devid=%u; type=%d; ret=%d)\n",
            devid, (int)msg->type, ret);
    }

    msg->error_code = ret;
    return 0;
}

STATIC u32 esched_drv_get_numa_node_num(void)
{
    return ((u32)cpu_to_node(num_online_cpus() - 1) + 1);
}

static void esched_init_sia_msg_ops(void)
{
    u32 index;
    u32 dev_num = esched_drv_get_numa_node_num();
    int len = sizeof(struct sched_msg_ops) * SCHED_MAX_CHIP_NUM;

    (void)memset_s(dev_msg_ops, len, 0, len);

    for (index = 0; index < dev_num; ++index) {
        esched_setup_sia_msg_ops(index);
    }
}

struct agentdrv_common_msg_client esched_device_msg_client = {
    .type = AGENTDRV_COMMON_MSG_ESCHED,
    .common_msg_recv = esched_ctrl_msg_recv,
};

int esched_client_init(void)
{
#ifndef CFG_NOT_SURPPORT_PCIE_HOST_DEVICE_COMM
    int ret;
#endif
    esched_init_sia_msg_ops();
#ifndef CFG_NOT_SURPPORT_PCIE_HOST_DEVICE_COMM
    ret = agentdrv_register_common_msg_client(&esched_device_msg_client);
    if (ret != 0) {
        sched_err("Failed to register client. (type=%d)\n", (int)esched_device_msg_client.type);
        return ret;
    }
#endif
    return 0;
}

void esched_client_uninit(void)
{
#ifndef CFG_NOT_SURPPORT_PCIE_HOST_DEVICE_COMM
    (void)agentdrv_unregister_common_msg_client(&esched_device_msg_client);
#endif
}

void esched_drv_register_msg_ops(u32 dev_id, struct sched_msg_ops *ops)
{
    struct sched_msg_ops *msg_ops = esched_device_get_msg_ops(dev_id);

    if (msg_ops->valid == 1) {
        return;
    }

    msg_ops->add_host_pid = ops->add_host_pid;
    msg_ops->del_host_pid = ops->del_host_pid;
    msg_ops->add_host_pool = ops->add_host_pool;
    msg_ops->get_host_cpu_mbid = ops->get_host_cpu_mbid;
    msg_ops->add_host_mb = ops->add_host_mb;
    msg_ops->conf_host_intr = ops->conf_host_intr;
    msg_ops->valid = 1;
}

