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
#include "agentdrv_sysfs.h"
#include "agentdrv_unit.h"
#include "agentdrv_common_msg.h"
#include "devdrv_interface.h"

struct agentdrv_common_msg_client g_agentdrv_sysfs_comm_msg_client = {
    .type = AGENTDRV_COMMON_MSG_SYSFS,
    .common_msg_recv = agentdrv_sysfs_msg_recv,
};

void agentdrv_sysfs_get_link_info(struct agentdrv_devctrl *agent_dev, struct agentdrv_sysfs_msg *msg)
{
    void __iomem *virtual_addr = NULL;
    u32 reg_val;

    virtual_addr = agent_dev->apb_base + AGENTDRV_SYSFS_LINK_STATUS_REG_OFFSET;
    reg_val = readl(virtual_addr);

    msg->link_info.link_speed = (reg_val >> 8) & 0x0f;
    msg->link_info.link_width = reg_val & 0x1f;
    msg->link_info.link_status = (reg_val >> 24) & 0x3f;
}

void agentdrv_sysfs_get_rx_para_lane(void __iomem *apb_base, u64 rx_reg_in0, u64 rx_reg_in1,
                                     struct agentdrv_sysfs_rx_lane_para *rx_para)
{
    void __iomem *virtual_addr = NULL;
    u32 reg_val;

    virtual_addr = apb_base + rx_reg_in0;
    reg_val = readl(virtual_addr);
    rx_para->att = reg_val & 0x07;
    rx_para->gain = (reg_val >> 3) & 0x0f;
    rx_para->boost = (reg_val >> 11) & 0x1f;

    virtual_addr = apb_base + rx_reg_in1;
    reg_val = readl(virtual_addr);
    rx_para->tap1 = (reg_val >> 7) & 0xff;
    rx_para->tap2 = reg_val & 0x7f;
    rx_para->valid = 1;
}

void agentdrv_sysfs_get_rx_para(struct agentdrv_devctrl *agent_dev, struct agentdrv_sysfs_msg *msg)
{
    agentdrv_sysfs_get_rx_para_lane(agent_dev->apb_base, AGENTDRV_SYSFS_RX_IN_0_LANE0, AGENTDRV_SYSFS_RX_IN_1_LANE0,
                                    &msg->rx_para.lane_rx_para[0]);

    agentdrv_sysfs_get_rx_para_lane(agent_dev->apb_base, AGENTDRV_SYSFS_RX_IN_0_LANE1, AGENTDRV_SYSFS_RX_IN_1_LANE1,
                                    &msg->rx_para.lane_rx_para[1]);

    agentdrv_sysfs_get_rx_para_lane(agent_dev->apb_base, AGENTDRV_SYSFS_RX_IN_0_LANE2, AGENTDRV_SYSFS_RX_IN_1_LANE2,
                                    &msg->rx_para.lane_rx_para[2]);

    agentdrv_sysfs_get_rx_para_lane(agent_dev->apb_base, AGENTDRV_SYSFS_RX_IN_0_LANE3, AGENTDRV_SYSFS_RX_IN_1_LANE3,
                                    &msg->rx_para.lane_rx_para[3]);
    msg->rx_para.lane_count = AGENTDRV_SYSFS_RX_LANE_MAX;
}

void agentdrv_sysfs_get_tx_para_lane(void __iomem *apb_base, u64 tx_reg_in1, u64 tx_reg_in2,
                                     struct agentdrv_sysfs_tx_lane_para *tx_para)
{
    void __iomem *virtual_addr = NULL;
    u32 reg_val;

    virtual_addr = apb_base + tx_reg_in1;
    reg_val = readl(virtual_addr);
    tx_para->main = (reg_val >> 6) & 0x3f;

    virtual_addr = apb_base + tx_reg_in2;
    reg_val = readl(virtual_addr);
    tx_para->pre = reg_val & 0x3f;
    tx_para->post = (reg_val >> 6) & 0x3f;
    tx_para->valid = 1;
}

void agentdrv_sysfs_get_tx_para(struct agentdrv_devctrl *agent_dev, struct agentdrv_sysfs_msg *msg)
{
    agentdrv_sysfs_get_tx_para_lane(agent_dev->apb_base, AGENTDRV_SYSFS_TX_IN_1_LANE0, AGENTDRV_SYSFS_TX_IN_2_LANE0,
                                    &msg->tx_para.lane_tx_para[0]);

    agentdrv_sysfs_get_tx_para_lane(agent_dev->apb_base, AGENTDRV_SYSFS_TX_IN_1_LANE1, AGENTDRV_SYSFS_TX_IN_2_LANE1,
                                    &msg->tx_para.lane_tx_para[1]);

    agentdrv_sysfs_get_tx_para_lane(agent_dev->apb_base, AGENTDRV_SYSFS_TX_IN_1_LANE2, AGENTDRV_SYSFS_TX_IN_2_LANE2,
                                    &msg->tx_para.lane_tx_para[2]);

    agentdrv_sysfs_get_tx_para_lane(agent_dev->apb_base, AGENTDRV_SYSFS_TX_IN_1_LANE3, AGENTDRV_SYSFS_TX_IN_2_LANE3,
                                    &msg->tx_para.lane_tx_para[3]);

    msg->tx_para.lane_count = AGENTDRV_SYSFS_TX_LANE_MAX;
}

void agentdrv_sysfs_get_aer_count(struct agentdrv_devctrl *agent_dev, struct agentdrv_sysfs_msg *msg)
{
    void __iomem *virtual_addr = NULL;
    u32 reg_val;

    virtual_addr = agent_dev->apb_base + AGENTDRV_SYSFS_AER_CNT_INFO_REG;
    reg_val = readl(virtual_addr);
    msg->data[0] = reg_val & 0xffff;
}

void agentdrv_sysfs_clear_aer_count(struct agentdrv_devctrl *agent_dev, struct agentdrv_sysfs_msg *msg)
{
    void __iomem *virtual_addr = NULL;
    u32 reg_val = 0xffff;

    virtual_addr = agent_dev->apb_base + AGENTDRV_SYSFS_AER_CNT_INFO_REG;
    writel(reg_val, virtual_addr);
}


void agentdrv_sysfs_get_common_msg(u32 devid, struct agentdrv_sysfs_msg *msg)
{
    int i;
    struct agentdrv_common_msg_stat *msg_stat = NULL;

    for (i = 0; i < AGENTDRV_COMMON_MSG_TYPE_MAX; i++) {
        msg_stat = agentdrv_get_common_msg_stat_by_type(devid, i);
        msg->com_msg_stat[i].tx_total_cnt = msg_stat->tx_total_cnt;
        msg->com_msg_stat[i].tx_success_cnt = msg_stat->tx_success_cnt;
        msg->com_msg_stat[i].tx_einval_err = msg_stat->tx_einval_err;
        msg->com_msg_stat[i].tx_enodev_err = msg_stat->tx_enodev_err;
        msg->com_msg_stat[i].tx_enosys_err = msg_stat->tx_enosys_err;
        msg->com_msg_stat[i].tx_etimedout_err = msg_stat->tx_etimedout_err;
        msg->com_msg_stat[i].tx_default_err = msg_stat->tx_default_err;
        msg->com_msg_stat[i].rx_total_cnt = msg_stat->rx_total_cnt;
        msg->com_msg_stat[i].rx_success_cnt = msg_stat->rx_success_cnt;
        msg->com_msg_stat[i].rx_para_err = msg_stat->rx_para_err;
        msg->com_msg_stat[i].rx_work_max_time = msg_stat->rx_work_max_time;
        msg->com_msg_stat[i].rx_work_delay_cnt = msg_stat->rx_work_delay_cnt;
    }
}

void agentdrv_sysfs_get_non_trans_msg(struct agentdrv_devctrl *agent_dev, struct agentdrv_sysfs_msg *msg)
{
    struct agentdrv_msg_chan *chan = NULL;
    struct agentdrv_msg_dev *msg_dev = NULL;
    u32 i;
    u32 j = 0;

    msg_dev = agent_dev->p_agentdrv_msg_dev[0];

    for (i = 0; i < msg_dev->io_chan_cnt; i++) {
        chan = &(msg_dev->io_chan[i]);
        if ((chan->status == AGENTDRV_ENABLE) && (chan->queue_type == NON_TRANSPARENT_MSG_QUEUE)) {
            msg->chan_stat[j].msg_type = chan->msg_type;
            msg->chan_stat[j].tx_total_cnt = chan->chan_stat.tx_total_cnt;
            msg->chan_stat[j].tx_success_cnt = chan->chan_stat.tx_success_cnt;
            msg->chan_stat[j].tx_no_callback = chan->chan_stat.tx_no_callback;
            msg->chan_stat[j].tx_len_check_err = chan->chan_stat.tx_len_check_err;
            msg->chan_stat[j].tx_reply_len_check_err = chan->chan_stat.tx_reply_len_check_err;
            msg->chan_stat[j].tx_dma_copy_err = chan->chan_stat.tx_dma_copy_err;
            msg->chan_stat[j].tx_timeout_err = chan->chan_stat.tx_timeout_err;
            msg->chan_stat[j].tx_process_err = chan->chan_stat.tx_process_err;
            msg->chan_stat[j].tx_invalid_para_err = chan->chan_stat.tx_invalid_para_err;
            msg->chan_stat[j].rx_total_cnt = chan->chan_stat.rx_total_cnt;
            msg->chan_stat[j].rx_success_cnt = chan->chan_stat.rx_success_cnt;
            msg->chan_stat[j].rx_para_err = chan->chan_stat.rx_para_err;
            msg->chan_stat[j].rx_work_max_time = chan->chan_stat.rx_work_max_time;
            msg->chan_stat[j].rx_work_delay_cnt = chan->chan_stat.rx_work_delay_cnt;
            // process still ran in chan->rx_msg_process, rx_success_cnt didn't plus one yet!
            if (chan->msg_type == agentdrv_msg_client_common) {
                msg->chan_stat[j].rx_success_cnt++;
            }
            j++;
        }
    }
}

void agentdrv_sysfs_get_sync_dma_info(struct agentdrv_devctrl *agent_dev, u32 devid,
    struct agentdrv_sysfs_msg *msg)
{
    struct devdrv_dma_dev *dma_dev = NULL;
    int i;
    u32 chip_id;
    u32 func_id;

    devdrv_dev2chipfunc(devid, &chip_id, &func_id);
    dma_dev = agent_dev->p_agentdrv_dma_dev[func_id];

    for (i = 0; i < AGENTDRV_SYSFS_DMA_CHAN_NUM; i++) {
        msg->sync_dma_stat[i].sync_submit_cnt = dma_dev->dma_chan[i].status.sync_submit_cnt;
        msg->sync_dma_stat[i].async_submit_cnt = dma_dev->dma_chan[i].status.async_submit_cnt;
        msg->sync_dma_stat[i].sml_submit_cnt = dma_dev->dma_chan[i].status.sml_submit_cnt;
        msg->sync_dma_stat[i].trigger_remot_int_cnt = dma_dev->dma_chan[i].status.trigger_remot_int_cnt;
        msg->sync_dma_stat[i].trigger_local_128 = dma_dev->dma_chan[i].status.trigger_local_128;
        msg->sync_dma_stat[i].done_int_cnt = dma_dev->dma_chan[i].status.done_int_cnt;
        msg->sync_dma_stat[i].re_schedule_cnt = dma_dev->dma_chan[i].status.re_schedule_cnt;
        msg->sync_dma_stat[i].err_int_cnt = dma_dev->dma_chan[i].status.err_int_cnt;
        msg->sync_dma_stat[i].done_tasklet_in_cnt = dma_dev->dma_chan[i].status.done_tasklet_in_cnt;
        msg->sync_dma_stat[i].err_work_cnt = dma_dev->dma_chan[i].status.err_work_cnt;
        msg->sync_dma_stat[i].sync_sem_up_cnt = dma_dev->dma_chan[i].status.sync_sem_up_cnt;
        msg->sync_dma_stat[i].async_proc_cnt = dma_dev->dma_chan[i].status.async_proc_cnt;
        msg->sync_dma_stat[i].max_task_op_time = dma_dev->dma_chan[i].status.max_task_op_time;
        msg->sync_dma_stat[i].sq_idle_bd_cnt = dma_dev->dma_chan[i].status.sq_idle_bd_cnt;
    }
}

void agentdrv_sysfs_msg_recv_proc(struct agentdrv_devctrl *agent_dev, struct agentdrv_sysfs_msg *msg,
    u32 devid, u32 *real_out_len)
{
    u32 data_len = sizeof(struct agentdrv_sysfs_msg);

    switch (msg->type) {
        case AGENTDRV_SYSFS_LINK_INFO:
            agentdrv_sysfs_get_link_info(agent_dev, msg);
            *real_out_len = data_len;
            break;

        case AGENTDRV_SYSFS_RX_PARA:
            agentdrv_sysfs_get_rx_para(agent_dev, msg);
            *real_out_len = data_len;
            break;
        case AGENTDRV_SYSFS_TX_PARA:
            agentdrv_sysfs_get_tx_para(agent_dev, msg);
            *real_out_len = data_len;
            break;
        case AGENTDRV_SYSFS_AER_COUNT:
            agentdrv_sysfs_get_aer_count(agent_dev, msg);
            *real_out_len = data_len;
            break;
        case AGENTDRV_SYSFS_AER_CLEAR:
            agentdrv_sysfs_clear_aer_count(agent_dev, msg);
            *real_out_len = 0;
            break;
        case AGENTDRV_SYSFS_COMMON_MSG:
            agentdrv_sysfs_get_common_msg(devid, msg);
            *real_out_len = data_len;
            break;
        case AGENTDRV_SYSFS_NON_TRANS_MSG:
            agentdrv_sysfs_get_non_trans_msg(agent_dev, msg);
            *real_out_len = data_len;
            break;
        case AGENTDRV_SYSFS_SYNC_DMA_INFO:
            agentdrv_sysfs_get_sync_dma_info(agent_dev, devid, msg);
            *real_out_len = data_len;
            break;
        default:
            *real_out_len = 0;
            break;
    }
}

int agentdrv_sysfs_msg_recv(u32 devid, void *data, u32 in_data_len, u32 out_data_len, u32 *real_out_len)
{
    struct agentdrv_sysfs_msg *msg = NULL;
    u32 data_len;
    struct agentdrv_devctrl *agent_dev = NULL;
    u32 chip_id = 0;
    u32 func_id = 0;

    if (devid >= MAX_AGENTDEV_CNT) {
        devdrv_err("Device ID is invalid.(dev_id=%u)\n", devid);
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

    data_len = sizeof(struct agentdrv_sysfs_msg);
    // The in_data_len should not be less than msg->type.
    if ((out_data_len < data_len) || (in_data_len < sizeof(u32))) {
        devdrv_err("Output length invalid. (out_data_len=%u, in_data_len=%u, data_len=%u)\n",
            out_data_len, in_data_len, data_len);
        return -EINVAL;
    }

    devdrv_dev2chipfunc(devid, &chip_id, &func_id);
    agent_dev = agentdrv_get_dev((int)chip_id);
    if ((agent_dev == NULL) || (agentdrv_is_dev_valid(agent_dev, func_id) == false)) {
        devdrv_err("Device ID is invalid.(dev_id=%u)\n", devid);
        return -EINVAL;
    }

    msg = (struct agentdrv_sysfs_msg *)data;
    agentdrv_sysfs_msg_recv_proc(agent_dev, msg, devid, real_out_len);

    return 0;
}

void agentdrv_sysfs_comm_client_register(void)
{
    int ret;

    ret = agentdrv_register_common_msg_client(&g_agentdrv_sysfs_comm_msg_client);
    if (ret) {
        devdrv_err("Register agentdrv_sysfs_comm_msg_client failed.\n");
    } else {
        devdrv_debug("Register agentdrv_sysfs_comm_msg_client success.\n");
    }
}
