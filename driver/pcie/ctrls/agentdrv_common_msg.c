/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
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
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/pci.h>

#include "devdrv_dma.h"
#include "agentdrv_msg.h"
#include "agentdrv_msg_non_trans.h"
#include "devdrv_msg_def.h"
#include "devdrv_interface.h"
#include "devdrv_util.h"
#include "agentdrv_ctrl.h"
#include "resource_comm_drv.h"
#include "agentdrv_common_msg.h"

struct agentdrv_common_msg g_pcie_common_msg;

void agentdrv_init_common_msg(void)
{
    int i, j;
    struct agentdrv_common_msg_ctx *msg_ctx = NULL;

    for (i = 0; i < MAX_AGENTDEV_CNT; i++) {
        g_pcie_common_msg.msg_chan[i] = NULL;
    }

    for (i = 0; i < AGENTDRV_COMMON_MSG_TYPE_MAX; i++) {
        msg_ctx = &g_pcie_common_msg.msg_ctx[i];
        msg_ctx->rx_func = NULL;
        msg_ctx->init_notify = NULL;
        for (j = 0; j < MAX_AGENTDEV_CNT; j++) {
            msg_ctx->notify_flag[j] = DEVDRV_INVALID;
        }
    }
}

STATIC void agentdrv_sriov_comm_msg_notify(u32 dev_id, struct agentdrv_msg_chan *chan)
{
    struct agentdrv_common_msg_ctx *msg_ctx = NULL;
    u32 func_totl = agentdrv_res_get_func_total();
    u32 func_id = (u32)chan->msg_dev->func_id;
    u32 i;

    if ((devdrv_is_sriov_enabled(func_totl) == false) || (func_id == 0)) {
        return;
    }

    for (i = 0; i < (u32)AGENTDRV_COMMON_MSG_TYPE_MAX; i++) {
        msg_ctx = &g_pcie_common_msg.msg_ctx[i];
        if (msg_ctx->init_notify != NULL) {
            msg_ctx->init_notify(dev_id);
            msg_ctx->notify_flag[dev_id] = DEVDRV_VALID;
        }
    }
}

int agentdrv_init_common_msg_chan(void *msg_chan)
{
    struct agentdrv_msg_chan *chan = (struct agentdrv_msg_chan *)msg_chan;
    int dev_id;

    if (msg_chan == NULL) {
        devdrv_err("Input parameter is error.\n");
        return -EINVAL;
    }

    dev_id = agentdrv_get_msg_chan_devid(msg_chan);
    if ((dev_id >= 0) && (dev_id < MAX_AGENTDEV_CNT)) {
        g_pcie_common_msg.msg_chan[dev_id] = chan;
        agentdrv_sriov_comm_msg_notify((u32)dev_id, msg_chan);
    }

    return 0;
}

void agentdrv_uninit_common_msg_chan(void *msg_chan)
{
    struct agentdrv_msg_chan *chan = (struct agentdrv_msg_chan *)msg_chan;
    int dev_id;

    if (chan == NULL) {
        devdrv_err("Input parameter is error.\n");
        return;
    }

    dev_id = agentdrv_get_msg_chan_devid(msg_chan);
    if ((dev_id >= 0) && (dev_id < MAX_AGENTDEV_CNT)) {
        g_pcie_common_msg.msg_chan[dev_id] = NULL;
    }
}

int agentdrv_rx_msg_common_msg_process(void *msg_chan, void *data, u32 in_data_len, u32 out_data_len, u32 *real_out_len)
{
    struct devdrv_non_trans_msg_desc *msg_desc = NULL;
    struct agentdrv_common_msg_stat *com_stat = NULL;
    struct agentdrv_msg_chan *chan = (struct agentdrv_msg_chan *)msg_chan;
    u32 agent_id;
    int ret;
    u32 cost_time;

    if (chan == NULL) {
        devdrv_err("Input parameter is error.\n");
        return -EINVAL;
    }
    if (data == NULL) {
        devdrv_err("Input parameter is error.\n");
        return -EINVAL;
    }
    if (real_out_len == NULL) {
        devdrv_err("Input parameter is error.\n");
        return -EINVAL;
    }

    agent_id = agentdrv_get_agentid_by_msg_dev(chan->msg_dev);
    msg_desc = container_of(data, struct devdrv_non_trans_msg_desc, data);
    if (msg_desc->msg_type >= (u32)AGENTDRV_COMMON_MSG_TYPE_MAX) {
        devdrv_err("msg_type is invalid. (dev_id=%u; msg_type=%u)\n", agent_id, msg_desc->msg_type);
        return -EINVAL;
    }

    if (g_pcie_common_msg.msg_ctx[msg_desc->msg_type].rx_func == NULL) {
        return -EUNATCH;
    }

    com_stat = &(g_pcie_common_msg.com_msg_stat[agent_id][msg_desc->msg_type]);
    cost_time = jiffies_to_msecs(jiffies - chan->stamp);
    if (cost_time > com_stat->rx_work_max_time) {
        com_stat->rx_work_max_time = cost_time;
    }
    if (cost_time > AGENTDEV_COMMON_WORK_QUEUE_RESQ_TIME) {
        com_stat->rx_work_delay_cnt++;
    }

    com_stat->rx_total_cnt++;

    ret = g_pcie_common_msg.msg_ctx[msg_desc->msg_type].rx_func(agent_id, data, in_data_len, out_data_len,
                                                                real_out_len);
    if (ret == 0) {
        com_stat->rx_success_cnt++;
    }

    return ret;
}

struct agentdrv_msg_chan *agentdrv_get_common_msg_chan_by_id(u32 dev_id)
{
    struct agentdrv_msg_chan *msg_chan = NULL;

    if (dev_id < MAX_AGENTDEV_CNT) {
        msg_chan = g_pcie_common_msg.msg_chan[dev_id];
        if (msg_chan != NULL) {
            if (msg_chan->status == AGENTDRV_ENABLE) {
                return msg_chan;
            }
        }
    }

    return NULL;
}

struct agentdrv_common_msg_stat *agentdrv_get_common_msg_stat_by_type(u32 dev_id, u32 msg_type)
{
    return &(g_pcie_common_msg.com_msg_stat[dev_id][msg_type]);
}

int agentdrv_common_msg_send(u32 devid, void *data, u32 in_data_len, u32 out_data_len, u32 *real_out_len,
    enum agentdrv_common_msg_type msg_type)
{
    struct agentdrv_msg_chan *msg_chan = NULL;
    struct agentdrv_common_msg_stat *com_stat = NULL;
    int msg_type_tmp;
    int ret;

    if (data == NULL) {
        devdrv_err("Input parameter data is error.(devid=%u)\n", devid);
        return -EINVAL;
    }
    if (real_out_len == NULL) {
        devdrv_err("Input parameter real_out_len is error. (devid=%u)\n", devid);
        return -EINVAL;
    }

    msg_type_tmp = msg_type;
    msg_chan = agentdrv_get_common_msg_chan_by_id(devid);
    if (msg_chan == NULL) {
        devdrv_warn("Msg_chan is null. (devid=%u)\n", devid);
        return -EINVAL;
    }
    if ((msg_type_tmp < AGENTDRV_COMMON_MSG_PCIVNIC) || (msg_type_tmp >= AGENTDRV_COMMON_MSG_TYPE_MAX)) {
        devdrv_err("msg_type is invalid. (msg_type=%d)\n", msg_type_tmp);
        return -EINVAL;
    }

    com_stat = &(g_pcie_common_msg.com_msg_stat[devid][msg_type]);
    com_stat->tx_total_cnt++;
    ret = agentdrv_sync_non_trans_msg_send(msg_chan, data, in_data_len, out_data_len, real_out_len, msg_type);
    if (ret == 0) {
        com_stat->tx_success_cnt++;
    } else if (ret == -EINVAL) {
        com_stat->tx_einval_err++;
    } else if (ret == -ENODEV) {
        com_stat->tx_enodev_err++;
    } else if (ret == -ENOSYS) {
        com_stat->tx_enosys_err++;
    } else if (ret == -ETIMEDOUT) {
        com_stat->tx_etimedout_err++;
    } else {
        com_stat->tx_default_err++;
    }

    return ret;
}
EXPORT_SYMBOL(agentdrv_common_msg_send);

STATIC int agentdrv_common_msg_notify(struct agentdrv_common_msg_ctx *msg_ctx)
{
    u32 func_totl = agentdrv_res_get_func_total();
    int ready_notify_num = 0;
    int not_notify_num = 0;
    int dev_num = 0;
    int i;

    if (devdrv_is_sriov_enabled(func_totl) == true) {
        /* when enable sriov, only notify pf' msg in this flow */
        dev_num = devdrv_get_dev_num();
    } else {
        dev_num = devdrv_get_slot_num();
    }

    for (i = 0; i < dev_num; i++) {
        if (msg_ctx->notify_flag[i] == DEVDRV_VALID) {
            ready_notify_num++;
            continue;
        }

        if (agentdrv_get_common_msg_chan_by_id(i) == NULL) {
            not_notify_num++;
            continue;
        }

        ready_notify_num++;
        msg_ctx->init_notify((u32)i);
        msg_ctx->notify_flag[i] = DEVDRV_VALID;
    }

    if (ready_notify_num == dev_num) {
        return 0;
    }

    return not_notify_num;
}

void agentdrv_common_msg_notify_work(struct work_struct *p_work)
{
    struct agentdrv_common_msg_ctx *msg_ctx = container_of(p_work, struct agentdrv_common_msg_ctx, notify_work);
    int timeout = DEVDRV_COMMON_MSG_NOTIFY_TIMEOUT; /* 300s */

    devdrv_info("Common message notify init.\n");

    while (timeout > 0) {
        if (agentdrv_common_msg_notify(msg_ctx) > 0) {
            msleep(AGENTDRV_MSLEEP_10);
        } else {
            break;
        }
        timeout--;
    }
}

int agentdrv_register_common_msg_client(struct agentdrv_common_msg_client *msg_client)
{
    struct agentdrv_common_msg_ctx *msg_ctx = NULL;

    if (msg_client == NULL) {
        devdrv_err("Input parameter is error.\n");
        return -EINVAL;
    }

    if (msg_client->type >= AGENTDRV_COMMON_MSG_TYPE_MAX) {
        devdrv_err("msg_client type is invalid. (type=%u)\n", msg_client->type);
        return -EINVAL;
    }

    msg_ctx = &g_pcie_common_msg.msg_ctx[msg_client->type];
    msg_ctx->rx_func = msg_client->common_msg_recv;
    msg_ctx->init_notify = msg_client->init_notify;
    msg_ctx->work_flag = 0;
    INIT_WORK(&msg_ctx->notify_work, agentdrv_common_msg_notify_work);
    if (msg_client->init_notify != NULL) {
        if (agentdrv_common_msg_notify(msg_ctx) > 0) {
            devdrv_info("msg_client type not notify, common msg chan null. (type=%u)\n", (u32)msg_client->type);
            msg_ctx->work_flag = 1;
            (void)schedule_work(&msg_ctx->notify_work);
        }
    } else {
        devdrv_info("Common msg init_notify not register. (type=%u)\n", msg_client->type);
    }

    return 0;
}
EXPORT_SYMBOL(agentdrv_register_common_msg_client);

int agentdrv_unregister_common_msg_client(const struct agentdrv_common_msg_client *msg_client)
{
    struct agentdrv_common_msg_ctx *msg_ctx = NULL;
    int i;

    if (msg_client == NULL) {
        devdrv_err("Input parameter is error.\n");
        return -EINVAL;
    }

    if (msg_client->type >= AGENTDRV_COMMON_MSG_TYPE_MAX) {
        devdrv_err("msg_client type is invalid. (type=%u)\n", msg_client->type);
        return -EINVAL;
    }

    msg_ctx = &g_pcie_common_msg.msg_ctx[msg_client->type];
    msg_ctx->rx_func = NULL;
    msg_ctx->init_notify = NULL;
    if (msg_ctx->work_flag == 1) {
        (void)cancel_work_sync(&msg_ctx->notify_work);
    }

    for (i = 0; i < MAX_AGENTDEV_CNT; i++) {
        msg_ctx->notify_flag[i] = DEVDRV_INVALID;
    }

    return 0;
}
EXPORT_SYMBOL(agentdrv_unregister_common_msg_client);
