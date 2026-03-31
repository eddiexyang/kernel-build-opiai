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
#include "agentdrv_msg_trans.h"
#include "devdrv_util.h"

/* trans chan */
struct agentdrv_trans_msg_client_ctrl g_agentdrv_client_ctrls[agentdrv_msg_client_max];

/* trans msg client */
struct agentdrv_trans_msg_client *agentdrv_find_client_by_type(u32 type)
{
    if (type >= agentdrv_msg_client_max) {
        devdrv_err("Find client type is error. (type=%u)\n", type);
        return NULL;
    }

    if (g_agentdrv_client_ctrls[type].status == AGENTDRV_DISABLE) {
        devdrv_err("Find client type is not registered. (type=%u)\n", type);
        return NULL;
    }

    return &g_agentdrv_client_ctrls[type].trans_msg_client;
}

int agentdrv_register_trans_msg_client(const struct agentdrv_trans_msg_client *msg_client)
{
    if (msg_client == NULL) {
        devdrv_err("Input parameter is invalid.\n");
        return -EINVAL;
    }

    if ((u32)msg_client->type >= agentdrv_msg_client_max) {
        devdrv_err("msg_client type is error. (type=%d)\n", msg_client->type);
        return -EINVAL;
    }

    if (g_agentdrv_client_ctrls[msg_client->type].status == AGENTDRV_ENABLE) {
        devdrv_err("msg_client type is already registered. (type=%d)\n", msg_client->type);
        return -EINVAL;
    }
    if (msg_client->rx_trans_msg_notify == NULL) {
        devdrv_err("msg_client type rx_trans_msg_notify is NULL. (type=%d)\n", msg_client->type);
        return -EINVAL;
    }

    g_agentdrv_client_ctrls[msg_client->type].trans_msg_client = *msg_client;
    g_agentdrv_client_ctrls[msg_client->type].status = AGENTDRV_ENABLE;

    return 0;
}
EXPORT_SYMBOL(agentdrv_register_trans_msg_client);

int agentdrv_unregister_trans_msg_client(const struct agentdrv_trans_msg_client *msg_client)
{
    if (msg_client == NULL) {
        devdrv_err("msg_client is null.\n");
        return -EINVAL;
    }

    if ((u32)msg_client->type >= agentdrv_msg_client_max) {
        devdrv_err("msg_client type is error. (type=%d)\n", msg_client->type);
        return -EINVAL;
    }

    g_agentdrv_client_ctrls[msg_client->type].status = AGENTDRV_DISABLE;

    return 0;
}
EXPORT_SYMBOL(agentdrv_unregister_trans_msg_client);

int agentdrv_trans_rx_msg_notify(struct agentdrv_msg_chan *chan)
{
    if (chan->rx_trans_msg_notify != NULL) {
        chan->rx_trans_msg_notify((void *)chan);
    }

    return 0;
}

int agentdrv_trans_tx_msg_finish(struct agentdrv_msg_chan *chan)
{
    if (chan->tx_trans_finish_notify != NULL) {
        chan->tx_trans_finish_notify((void *)chan);
    }

    return 0;
}
