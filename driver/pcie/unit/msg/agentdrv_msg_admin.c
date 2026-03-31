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
#include "agentdrv_msg_admin.h"
#include "agentdrv_msg.h"
#include "agentdrv_msg_p2p.h"
#include "agentdrv_unit.h"
#include "agentdrv_ctrl.h"
#include "agentdrv_pci.h"
#include "devdrv_dma.h"

int agentdrv_notify_host_dma_err_irq(struct agentdrv_msg_dev *msg_dev, void *data);
int devdrv_dma_chan_remote_op_func(struct agentdrv_msg_dev *msg_dev, void *data);
int agentdrv_hccs_host_dma_addr_map(struct agentdrv_msg_dev *msg_dev, void *data);
int agentdrv_hccs_host_dma_addr_unmap(struct agentdrv_msg_dev *msg_dev, void *data);

static int (*agentdrv_oper_func[])(struct agentdrv_msg_dev *msg_dev, void *data) = {
    agentdrv_msg_alloc_msg_queue,
    agentdrv_msg_free_msg_queue,
    agentdrv_notify_host_dma_err_irq,
    agentdrv_get_general_interrupt_db_info,
    agentdrv_notify_dev_online,
    agentdrv_cfg_p2p_msg_chan,
    agentdrv_cfg_p2p_tx_atu,
    agentdrv_get_rx_atu,
    devdrv_dma_chan_remote_op_func,
    agentdrv_hccs_host_dma_addr_map,
    agentdrv_hccs_host_dma_addr_unmap,
    agentdrv_sriov_event_notify,
    agentdrv_get_ep_suspend_status};

int agentdrv_notify_host_dma_err_irq(struct agentdrv_msg_dev *msg_dev, void *data)
{
    struct devdrv_notify_dma_err_irq_cmd *cmd_data = (struct devdrv_notify_dma_err_irq_cmd *)data;
    struct devdrv_admin_msg_reply *reply = (struct devdrv_admin_msg_reply *)data;

    agentdrv_set_dma_host_err_irq(msg_dev->dev_id, cmd_data->dma_chan_id, cmd_data->err_irq);

    /* no response message */
    reply->len = sizeof(struct devdrv_admin_msg_reply);

    return 0;
}

void devdrv_get_admin_sq_base(int chip_id, int func_id, u64 *sq_base)
{
    struct agentdrv_devctrl *agent_dev = agentdrv_get_dev(chip_id);
    int connect_protocol;

    if (agent_dev == NULL) {
        devdrv_err("agent_dev is null.\n");
        *sq_base = 0;
        return;
    }

    connect_protocol = agent_dev->shr_para[func_id]->connect_protocol;
    *sq_base = agent_dev->shr_para[func_id]->admin_chan_sq_base;
}

int agentdrv_init_admin_queue_remote_sq(struct agentdrv_msg_chan *chan)
{
    if (chan->sq.base_h == 0) {
        devdrv_get_admin_sq_base(chan->msg_dev->dev_id, chan->msg_dev->func_id, (u64 *)&chan->sq.base_h);
        devdrv_info("Admin chan base_h init. (chip=%d; func_id=%d)\n", chan->msg_dev->dev_id, chan->msg_dev->func_id);
    }

    return 0;
}

int agentdrv_admin_wakeup_sq_db_work(struct agentdrv_msg_chan *msg_chan)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    u32 func_id;

    agent_dev = agentdrv_get_dev(msg_chan->msg_dev->dev_id);
    if (agent_dev == NULL) {
        devdrv_err("Function agentdrv_get_dev failed, agent_dev is null.\n");
        return -EINVAL;
    }

    func_id = msg_chan->msg_dev->func_id;
    agent_dev->shr_para[func_id]->admin_msg_status = DEVDRV_MSG_CMD_IRQ_BEGIN;

    /* start work queue */
    (void)queue_work(msg_chan->msg_dev->work_queue, &msg_chan->rx_work);

    return 0;
}

void agentdrv_admin_queue_cmd_reply(struct agentdrv_msg_chan *chan, u32 status, u32 dev_id,
                                    enum devdrv_dma_data_type data_type)
{
    struct devdrv_admin_msg_command *msg_head = NULL;
    struct devdrv_admin_msg_reply *msg_reply = NULL;
    int ret;

    msg_head = (struct devdrv_admin_msg_command *)chan->sq.base_d;
    msg_reply = (struct devdrv_admin_msg_reply *)msg_head->data;

    msg_head->status = status;

    /* If there is response information, move to the host side first */
    if ((msg_reply->len > DEVDRV_ADMIN_MSG_DATA_LEN) && (status == DEVDRV_MSG_CMD_FINISH_SUCCESS)) {
        devdrv_err("Reply len is invalid. (dev_id=%u; len=%d)\n", dev_id, msg_reply->len);

        msg_head->status = DEVDRV_MSG_CMD_FINISH_FAILED;
    }
    if ((msg_reply->len > sizeof(struct devdrv_admin_msg_reply)) &&
        (msg_head->status == DEVDRV_MSG_CMD_FINISH_SUCCESS)) {
        ret = devdrv_dma_sync_copy(dev_id, data_type, chan->sq.dma_base_d + DEVDRV_ADMIN_MSG_HEAD_LEN,
                                   chan->sq.base_h + DEVDRV_ADMIN_MSG_HEAD_LEN, msg_reply->len,
                                   DEVDRV_DMA_DEVICE_TO_HOST);
        if (ret != 0) {
            devdrv_err("Admin chan send msg reply failed. (dev_id=%u; ret=%u)\n", dev_id, ret);

            msg_head->status = DEVDRV_MSG_CMD_FINISH_FAILED;
        }
    }

    /* respond to host side to execute result status */
    ret = devdrv_respond_msg_to_remote_side(dev_id, chan->sq.dma_base_d, chan->sq.base_h,
        (void *)msg_head, (u32)DEVDRV_ADMIN_MSG_HEAD_LEN);
    if (ret != 0) {
        devdrv_err("Admin chan send msg status failed. (dev_id=%u; ret=%u)\n", dev_id, ret);
    }
}

void agentdrv_admin_queue_cmd_proc(struct work_struct *p_work)
{
    struct agentdrv_msg_chan *chan = container_of(p_work, struct agentdrv_msg_chan, rx_work);
    u32 dev_id;
    int ret;
    struct devdrv_admin_msg_command *msg_head = NULL;
    enum devdrv_dma_data_type data_type = DEVDRV_DMA_DATA_PCIE_MSG;
    struct devdrv_dma_dev *dma_dev = NULL;

    (void)agentdrv_init_admin_queue_remote_sq(chan);

    dev_id = agentdrv_get_agentid_by_msg_dev(chan->msg_dev);
    devdrv_debug("Admin chan recv a command. (dev_id=%u)\n", dev_id);

    dma_dev = devdrv_get_dma_dev(dev_id);
    if (dma_dev == NULL) {
        devdrv_err("Call devdrv_get_dma_dev failed, dma_dev is NULL. (dev_id=%u)\n", dev_id);
        return;
    }

    /* copy sq desc cmd from host */
    ret = devdrv_dma_sync_copy(dev_id, data_type, chan->sq.base_h, chan->sq.dma_base_d, DEVDRV_ADMIN_MSG_QUEUE_BD_SIZE,
                               DEVDRV_DMA_HOST_TO_DEVICE);
    if (ret != 0) {
        devdrv_err("Admin chan fetch command failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
        agentdrv_admin_queue_cmd_reply(chan, DEVDRV_MSG_CMD_FINISH_FAILED, dev_id, data_type);
        return;
    }

    msg_head = (struct devdrv_admin_msg_command *)chan->sq.base_d;

    if (msg_head->status != DEVDRV_MSG_CMD_BEGIN) {
        devdrv_err("Admin chan command status error. (dev_id=%u; opcode=%u; status=%u)\n",
            dev_id, msg_head->opcode, msg_head->status);

        agentdrv_admin_queue_cmd_reply(chan, DEVDRV_MSG_CMD_FINISH_FAILED, dev_id, data_type);
        return;
    }

    devdrv_debug("Admin chan recv a command opcode. (dev_id=%u; opcode=%u)\n", dev_id, msg_head->opcode);

    if (msg_head->opcode >= DEVDRV_ADMIN_MSG_MAX) {
        devdrv_err("Admin chan cmd opcode error. (dev_id=%u; opcode=%u)\n", dev_id, msg_head->opcode);

        agentdrv_admin_queue_cmd_reply(chan, DEVDRV_MSG_CMD_FINISH_FAILED, dev_id, data_type);

        return;
    } else {
        ret = agentdrv_oper_func[msg_head->opcode](chan->msg_dev, (void *)msg_head->data);
        if (ret != 0) {
            devdrv_err("Admin chan command opcode executor failed. (dev_id=%u; opcode=%u; ret=%d)\n",
                       dev_id, msg_head->opcode, ret);
            agentdrv_admin_queue_cmd_reply(chan, DEVDRV_MSG_CMD_FINISH_FAILED, dev_id, data_type);
            return;
        }
    }

    agentdrv_admin_queue_cmd_reply(chan, DEVDRV_MSG_CMD_FINISH_SUCCESS, dev_id, data_type);
    /* DEVDRV_GET_EP_SUSPEND_STATUS opcode only is used in mdc */
    /* make sure ep release dma resource after handshake msg reply finished */
    if (msg_head->opcode == DEVDRV_GET_EP_SUSPEND_STATUS) {
        agentdrv_wake_up_ep_suspend();
    }
}

int agentdrv_msg_dev_admin_chan_init(struct agentdrv_msg_dev *msg_dev)
{
    struct agentdrv_msg_chan *chan = NULL;
    int ret;

    chan = &msg_dev->io_chan[AGENTDRV_ADMIN_CHAN_ID];
    chan->status = AGENTDRV_ENABLE;

    /* init admin queue local buf info */
    ret = agentdrv_msg_alloc_local_sq(chan, DEVDRV_ADMIN_MSG_QUEUE_DEPTH, DEVDRV_ADMIN_MSG_QUEUE_BD_SIZE);
    if (ret != 0) {
        devdrv_err("Admin chan alloc sq failed. (dev_id=%d; func_id=%d; chan_id=%u; ret=%d)\n",
                   msg_dev->dev_id, msg_dev->func_id, chan->chan_id, ret);
        return ret;
    }

    /* init admin_queue db_fun */
    chan->sq.db_fun = agentdrv_admin_wakeup_sq_db_work;

    INIT_WORK(&chan->rx_work, agentdrv_admin_queue_cmd_proc);
    chan->rx_work_flag = 1;

    return ret;
}
