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
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/securec.h>

#include "agentdrv_msg_non_trans.h"
#include "devdrv_util.h"
#include "devdrv_dma.h"
#include "agentdrv_common_msg.h"

/* non-trans chan */
struct agentdrv_non_trans_msg_client_ctrl g_agentdrv_non_trans_msg_client_ctrls[agentdrv_msg_client_max];

/* non-trans msg client */
struct agentdrv_non_trans_msg_client *agentdrv_find_non_trans_msg_client_by_type(u32 type)
{
    if (type >= agentdrv_msg_client_max) {
        devdrv_err("Find client type is error. (type=%u)\n", type);
        return NULL;
    }

    if (g_agentdrv_non_trans_msg_client_ctrls[type].status == AGENTDRV_DISABLE) {
        devdrv_err("Find client type is not registered. (type=%u)\n", type);
        return NULL;
    }

    return &g_agentdrv_non_trans_msg_client_ctrls[type].non_trans_msg_client;
}

int agentdrv_register_non_trans_msg_client(const struct agentdrv_non_trans_msg_client *msg_client)
{
    if (msg_client == NULL) {
        devdrv_err("Input parameter is invalid.\n");
        return -EINVAL;
    }

    if ((u32)msg_client->type >= agentdrv_msg_client_max) {
        devdrv_err("msg_client type is error. (type=%d)\n", msg_client->type);
        return -EINVAL;
    }

    if (g_agentdrv_non_trans_msg_client_ctrls[msg_client->type].status == AGENTDRV_ENABLE) {
        devdrv_err("msg_client type is already registered. (type=%d)\n", msg_client->type);
        return -EINVAL;
    }
    if (msg_client->non_trans_msg_process == NULL) {
        devdrv_err("msg_client type non_trans_msg_process is NULL. (type=%d)\n", msg_client->type);
        return -EINVAL;
    }

    g_agentdrv_non_trans_msg_client_ctrls[msg_client->type].non_trans_msg_client = *msg_client;
    g_agentdrv_non_trans_msg_client_ctrls[msg_client->type].status = AGENTDRV_ENABLE;

    return 0;
}
EXPORT_SYMBOL(agentdrv_register_non_trans_msg_client);

int agentdrv_unregister_non_trans_msg_client(const struct agentdrv_non_trans_msg_client *msg_client)
{
    if (msg_client == NULL) {
        devdrv_err("Input parameter is invalid.\n");
        return -EINVAL;
    }

    if ((u32)msg_client->type >= agentdrv_msg_client_max) {
        devdrv_err("msg_client type is error. (type=%d)\n", msg_client->type);
        return -EINVAL;
    }

    g_agentdrv_non_trans_msg_client_ctrls[msg_client->type].status = AGENTDRV_DISABLE;

    return 0;
}
EXPORT_SYMBOL(agentdrv_unregister_non_trans_msg_client);

int agentdrv_non_trans_wakeup_sq_db_work(struct agentdrv_msg_chan *msg_chan)
{
    struct devdrv_non_trans_msg_desc *bd_desc = NULL;

    bd_desc = (struct devdrv_non_trans_msg_desc *)msg_chan->sq.base_reserve_d;
    bd_desc->status = DEVDRV_MSG_CMD_IRQ_BEGIN;
    /* clean dirty cache */
    agentdrv_flush_cache((u64)(uintptr_t)bd_desc, sizeof(struct devdrv_non_trans_msg_desc));

    msg_chan->stamp = (u32)jiffies;
    /* start work queue */
    (void)queue_work(msg_chan->msg_dev->work_queue, &msg_chan->rx_work);

    return 0;
}

void agentdrv_non_trans_rx_msg_record_resq_time(struct agentdrv_msg_chan *chan)
{
    u32 resq_time;

    resq_time = jiffies_to_msecs(jiffies - chan->stamp);
    if (resq_time > AGENTDRV_WORK_RESQ_TIME) {
        chan->chan_stat.rx_work_delay_cnt++;
        if (resq_time > chan->chan_stat.rx_work_max_time) {
            devdrv_info("Get resq_time. (msg_type=%u; chan_id=%u; resq_time=%u; cpu=%d)\n",
                        chan->msg_type, chan->chan_id, resq_time, smp_processor_id());
        }
    }
    if (resq_time > chan->chan_stat.rx_work_max_time) {
        chan->chan_stat.rx_work_max_time = resq_time;
    }
}

void agentdrv_non_trans_rx_msg_callback(struct agentdrv_msg_chan *chan,
    struct devdrv_non_trans_msg_desc *bd_desc, u32 dev_id)
{
    int ret;

    ret = chan->rx_msg_process((void *)chan, bd_desc->data, bd_desc->in_data_len, bd_desc->out_data_len,
                               &bd_desc->real_out_len);
    if ((ret == 0) && (bd_desc->real_out_len <= bd_desc->out_data_len)) {
        bd_desc->status = DEVDRV_MSG_CMD_FINISH_SUCCESS;
        chan->chan_stat.rx_success_cnt++;
    } else if (ret == -EINVAL) {
        chan->chan_stat.rx_para_err++;
        devdrv_warn("Get rx_msg_process. (dev_id=%u; msg_type=%d; ret=%d; out_buf=%d; out_len=%d)\n",
                    dev_id, (int)chan->msg_type, ret, bd_desc->out_data_len, bd_desc->real_out_len);
        bd_desc->status = DEVDRV_MSG_CMD_INVALID_PARA;
    } else if (ret == -EUNATCH) {
        bd_desc->status = DEVDRV_MSG_CMD_NULL_PROCESS_CB;
    } else {
        devdrv_err("Get rx_msg_process. (dev_id=%u; msg_type=%d; ret=%d; out_buf=%d; out_len=%d)\n",
                   dev_id, (int)chan->msg_type, ret, bd_desc->out_data_len, bd_desc->real_out_len);
        bd_desc->status = DEVDRV_MSG_CMD_FINISH_FAILED;
    }
}

void agentdrv_non_trans_rx_msg_task(struct work_struct *p_work)
{
    struct agentdrv_msg_chan *chan = container_of(p_work, struct agentdrv_msg_chan, rx_work);
    struct devdrv_non_trans_msg_desc *bd_desc = NULL;
    u32 dev_id = agentdrv_get_agentid_by_msg_dev(chan->msg_dev);
    enum devdrv_dma_data_type data_type = DEVDRV_DMA_DATA_PCIE_MSG;
    struct devdrv_dma_node dma_node = {0};
    int ret;
    u32 retry_times = 0;

    agentdrv_non_trans_rx_msg_record_resq_time(chan);
    /* the device side directly reads the sq descriptor written to the shared memory on the host side */
    bd_desc = (struct devdrv_non_trans_msg_desc *)chan->sq.base_reserve_d;
    if (bd_desc->in_data_len >= (chan->sq.desc_size - DEVDRV_NON_TRANS_MSG_HEAD_LEN)) {
        devdrv_err("in_data_len overflow. (dev_id=%u)\n", dev_id);
        return;
    }

    /* copy the shared memory message to the local sq, for shared memory messages, host write, device read */
    if (memcpy_s((void *)chan->sq.base_d, chan->sq.desc_size, (void *)bd_desc,
                 bd_desc->in_data_len + DEVDRV_NON_TRANS_MSG_HEAD_LEN) != 0) {
        devdrv_err("memcpy_s failed. (dev_id=%u)\n", dev_id);
        bd_desc = (struct devdrv_non_trans_msg_desc *)chan->sq.base_d;
        bd_desc->status = DEVDRV_MSG_CMD_FINISH_FAILED;
    }
    bd_desc = (struct devdrv_non_trans_msg_desc *)chan->sq.base_d;
    if (bd_desc->seq_num == chan->seq_num) {
        devdrv_warn("Get seq_num. (devid=%u; msy_type=%d; seq_num=%lld)\n",
                    dev_id, (int)chan->msg_type, chan->seq_num);
    }
    chan->seq_num = bd_desc->seq_num;
    if ((bd_desc->status == DEVDRV_MSG_CMD_IRQ_BEGIN) && (chan->rx_msg_process != NULL)) {
        /* callback */
        chan->chan_stat.rx_total_cnt++;
        agentdrv_non_trans_rx_msg_callback(chan, bd_desc, dev_id);
    } else {
        devdrv_err("Get desc_status. (dev_id=%u; chan_id=%d; desc_status=%d)\n",
                   dev_id, chan->chan_id, bd_desc->status);
        bd_desc->status = DEVDRV_MSG_CMD_FINISH_FAILED;
    }

    /* move result data to the host side first */
    if ((bd_desc->status == DEVDRV_MSG_CMD_FINISH_SUCCESS) && (bd_desc->real_out_len > 0)) {
        dma_node.src_addr = chan->sq.dma_base_d + DEVDRV_NON_TRANS_MSG_HEAD_LEN;
        dma_node.dst_addr = chan->sq.base_h + DEVDRV_NON_TRANS_MSG_HEAD_LEN;
        dma_node.size = bd_desc->real_out_len;
        dma_node.direction = DEVDRV_DMA_DEVICE_TO_HOST;
        ret = devdrv_dma_copy(chan->msg_dev->dma_dev, data_type, DEVDRV_INVALID_INSTANCE, &dma_node, 1,
                              DEVDRV_DMA_WAIT_QUREY, DEVDRV_DMA_SYNC, NULL);
        if (ret != 0) {
            devdrv_err("non_trans channel send msg reply failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
            bd_desc->status = DEVDRV_MSG_CMD_FINISH_FAILED;
        }
    }
    /* respond to host side to execute result status */
retry:
    ret = devdrv_respond_msg_to_remote_side(dev_id, chan->sq.dma_base_d, chan->sq.base_h,
        (void *)bd_desc, (u32)DEVDRV_NON_TRANS_MSG_HEAD_LEN);
    if (ret == (-ENOSPC)) {
        /* solve the bug of insufficient bd descriptor, Timeout time 3 seconds */
        if (retry_times >= AGENTDRV_DMA_COPY_RETRY_TIME) {
            devdrv_err("non_trans failed. (retry_times=%d; dev_id=%u; data_type=%d)",
                AGENTDRV_DMA_COPY_RETRY_TIME, dev_id, data_type);
            return;
        }
        retry_times++;
        data_type = retry_times % 2 ? DEVDRV_DMA_DATA_COMMON : DEVDRV_DMA_DATA_PCIE_MSG;
        usleep_range(1000, 1100);
        goto retry;
    }
    if (ret != 0) {
        devdrv_err("non_trans channel send msg status failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
    }
}

int agentdrv_sync_non_trans_status_handle(struct agentdrv_msg_chan *msg_chan, enum agentdrv_common_msg_type msg_type,
    u32 status, struct agentdrv_non_trans_msg_send_data_para *data_para)
{
    struct devdrv_non_trans_msg_desc *bd_desc = NULL;
    u32 dev_id = agentdrv_get_agentid_by_msg_dev(msg_chan->msg_dev);
    struct agentdrv_msg_chan_stat *chan_stat = &(msg_chan->chan_stat);
    u32 *real_out_len = data_para->real_out_len;
    int ret = 0;

    bd_desc = (struct devdrv_non_trans_msg_desc *)msg_chan->cq.base_reserve_d;
    if (status == DEVDRV_MSG_CMD_FINISH_SUCCESS) {
        *real_out_len = bd_desc->real_out_len;
        if (*real_out_len > data_para->out_data_len) {
            chan_stat->tx_reply_len_check_err++;
            devdrv_err("real_out_len is invalid.(dev_id=%u; msg_type=%d; common_type=%d; real_out_len=%d; "
                "out_data_len=%d)\n", dev_id, msg_chan->msg_type, msg_type, *real_out_len, data_para->out_data_len);
            return -EINVAL;
        }
        if (*real_out_len > 0) {
            ret = memcpy_s(data_para->data, data_para->out_data_len, (void *)bd_desc->data, *real_out_len);
            if (ret) {
                devdrv_err("memcpy_s failed. (dev_id=%d)\n", dev_id);
                ret = -EINVAL;
            }
        }
        chan_stat->tx_success_cnt++;
    } else if (status == DEVDRV_MSG_CMD_BEGIN) {
        ret = -ENOSYS;
        chan_stat->tx_timeout_err++;
        devdrv_err("Message send finish, no resp. (dev_id=%d; msg_type=%d; common_type=%d; status=%d; ret=%d)\n",
                   dev_id, msg_chan->msg_type, msg_type, status, ret);
    } else if (status == DEVDRV_MSG_CMD_FINISH_FAILED) {
        ret = -ETIMEDOUT;
        chan_stat->tx_process_err++;
        devdrv_err("Message send finish, process failed. (dev_id=%d; msg_type=%d; common_type=%d; status=%d; ret=%d)\n",
                   dev_id, msg_chan->msg_type, msg_type, status, ret);
    } else if (status == DEVDRV_MSG_CMD_NULL_PROCESS_CB) {
        ret = -EUNATCH;
        chan_stat->tx_no_callback++;
        devdrv_warn("Message send finish, no process cb. (dev_id=%d; msg_type=%d; common_type=%d; status=%d; ret=%d)\n",
                    dev_id, msg_chan->msg_type, msg_type, status, ret);
    } else {
        ret = -EINVAL;
        chan_stat->tx_invalid_para_err++;
        devdrv_warn("Message send finish. (dev_id=%d; msg_type=%d; common_type=%d; status=%d; ret=%d)\n", dev_id,
                    msg_chan->msg_type, msg_type, status, ret);
    }
    return ret;
}

static bool agentdrv_check_msg_dev_valid(const struct agentdrv_msg_chan *msg_chan)
{
    if (msg_chan->msg_dev->dev_status == DEVDRV_DEV_OFFLINE) {
        return false;
    }
    return true;
}

int agentdrv_set_msg_dev_status(u32 dev_id, int status)
{
    struct agentdrv_msg_dev *msg_dev = agentdrv_get_msg_dev(dev_id);
    if (msg_dev == NULL) {
        devdrv_err("Get msg dev is NULL. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }
    msg_dev->dev_status = (u32)status;
    return 0;
}
EXPORT_SYMBOL(agentdrv_set_msg_dev_status);

STATIC int agentdrv_sync_non_trans_msg_chan_send(struct agentdrv_msg_chan *msg_chan,
    enum agentdrv_common_msg_type msg_type, struct agentdrv_non_trans_msg_send_data_para *data_para)
{
    struct devdrv_non_trans_msg_desc *bd_desc = NULL;
    u32 dev_id = agentdrv_get_agentid_by_msg_dev(msg_chan->msg_dev);
    struct devdrv_dma_node dma_node[AGENTDRV_DMA_NODE_NUM] = {{0}};
    enum devdrv_dma_data_type data_type = DEVDRV_DMA_DATA_PCIE_MSG;
    int timeout = DEVDRV_MSG_D2H_TIMEOUT;
    u32 status = 0;
    int ret;
    int retry_times = 0;
    struct agentdrv_msg_chan_stat *chan_stat = &(msg_chan->chan_stat);
    u64 seq_num = chan_stat->tx_total_cnt;

    /* dma copy messages to the cq buf on the host side */
    dma_node[0].src_addr = msg_chan->cq.dma_base_d + DEVDRV_NON_TRANS_MSG_HEAD_LEN;
    dma_node[0].dst_addr = msg_chan->cq.base_h + DEVDRV_NON_TRANS_MSG_HEAD_LEN;
    dma_node[0].direction = DEVDRV_DMA_DEVICE_TO_HOST;
    dma_node[0].size = data_para->in_data_len;
    dma_node[1].src_addr = msg_chan->cq.dma_base_d;
    dma_node[1].dst_addr = msg_chan->cq.base_h;
    dma_node[1].direction = DEVDRV_DMA_DEVICE_TO_HOST;
    dma_node[1].size = DEVDRV_NON_TRANS_MSG_HEAD_LEN;

msg_retry:
    bd_desc = (struct devdrv_non_trans_msg_desc *)msg_chan->cq.base_reserve_d;
    bd_desc->status = DEVDRV_MSG_CMD_BEGIN;
    wmb();

    ret = devdrv_dma_copy(msg_chan->msg_dev->dma_dev, data_type, DEVDRV_INVALID_INSTANCE, dma_node,
                          AGENTDRV_DMA_NODE_NUM, DEVDRV_DMA_WAIT_QUREY, DEVDRV_DMA_SYNC, NULL);
    if (ret) {
        chan_stat->tx_dma_copy_err++;
        devdrv_err("DMA copy failed. (dev_id=%u; msg_type=%d; common_type=%d; in_data_len=%d; ret=%d)\n", dev_id,
            msg_chan->msg_type, msg_type, data_para->in_data_len, ret);
        return ret;
    }

    /* inform host */
    agentdrv_iocmd_raise_int_to_h(msg_chan->msg_dev, msg_chan->irq_rx_msg_notify);

    /* wait for the host side to finish processing and place the result in the shared memory cq buf */
    while (timeout > 0) {
        status = bd_desc->status;
        if (status != DEVDRV_MSG_CMD_BEGIN) {
            break;
        }
        rmb();
        usleep_range(DEVDRV_MSG_WAIT_MIN_TIME, DEVDRV_MSG_WAIT_MAX_TIME);

        if (!agentdrv_check_msg_dev_valid(msg_chan)) {
            devdrv_err("Check msg chan is invalid. (dev_id=%u; msg_type=%d; common_type=%d)\n",
                dev_id, msg_chan->msg_type, (u32)msg_type);
            return -EINVAL;
        }
        timeout -= DEVDRV_MSG_WAIT_MIN_TIME;
    }
    mb();
    if ((status != DEVDRV_MSG_CMD_BEGIN) && (bd_desc->seq_num != seq_num)) {
        devdrv_warn("Reply num invalid. (dev_id=%u; num=%lld; reply_num=%lld; status=%d)\n",
                    dev_id, seq_num, bd_desc->seq_num, status);

        if (retry_times > DEVDRV_MSG_RETRY_MAX_CYCLE) {
            devdrv_warn("Trans msg send timeout. (dev_id=%u)\n", dev_id);
            return -ETIMEDOUT;
        }
        retry_times++;

        goto msg_retry;
    }

    return agentdrv_sync_non_trans_status_handle(msg_chan, msg_type, status, data_para);
}

/* send to host cq tail */
int agentdrv_sync_non_trans_msg_send(struct agentdrv_msg_chan *msg_chan, void *data, u32 in_data_len, u32 out_data_len,
                                     u32 *real_out_len, enum agentdrv_common_msg_type msg_type)
{
    struct devdrv_non_trans_msg_desc *bd_desc = NULL;
    u32 dev_id = agentdrv_get_agentid_by_msg_dev(msg_chan->msg_dev);
    struct agentdrv_non_trans_msg_send_data_para data_para;
    int ret;
    u32 max_data_len = msg_chan->cq.desc_size - DEVDRV_NON_TRANS_MSG_HEAD_LEN;
    struct agentdrv_msg_chan_stat *chan_stat = &(msg_chan->chan_stat);
    u64 seq_num;

    mutex_lock(&msg_chan->mutex);
    chan_stat->tx_total_cnt++;
    seq_num = chan_stat->tx_total_cnt;
    if ((in_data_len > max_data_len) || (out_data_len > max_data_len)) {
        chan_stat->tx_len_check_err++;
        mutex_unlock(&msg_chan->mutex);
        devdrv_err("in_data_len or out_data_len is bigger than max_data_len. "
                   "(dev_id=%u; msg_type=%d; common_type=%u; in_data_len=%u; out_data_len=%u; max_data_len=%u)\n",
                   dev_id, (u32)msg_chan->msg_type, (u32)msg_type, in_data_len, out_data_len, max_data_len);
        return -EINVAL;
    }

    if (msg_chan->status == AGENTDRV_DISABLE) {
        devdrv_warn("Msg chan status is disable. (dev_id=%u; msg_type=%d; common_type=%u)",
            dev_id, (u32)msg_chan->msg_type, (u32)msg_type);
        mutex_unlock(&msg_chan->mutex);
        return -EINVAL;
    }
    devdrv_debug("Get in out data_len. (dev_id=%u; msg_type=%d; comm_type=%u; in_date_len=%u; out_data_len=%u)\n",
                 dev_id, (u32)msg_chan->msg_type, (u32)msg_type, in_data_len, out_data_len);

    data_para.data = data;
    data_para.in_data_len = in_data_len;
    data_para.out_data_len = out_data_len;
    data_para.real_out_len = real_out_len;
    /* put messages to send to cq buf */
    bd_desc = (struct devdrv_non_trans_msg_desc *)msg_chan->cq.base_d;
    bd_desc->in_data_len = in_data_len;
    bd_desc->out_data_len = out_data_len;
    bd_desc->real_out_len = 0;
    bd_desc->msg_type = msg_type;
    bd_desc->seq_num = seq_num;
    bd_desc->status = DEVDRV_MSG_CMD_BEGIN;

    if ((in_data_len <= 0) || (memcpy_s((void *)bd_desc->data, max_data_len, data, in_data_len) != EOK)) {
        chan_stat->tx_len_check_err++;
        mutex_unlock(&msg_chan->mutex);
        devdrv_err("in_date_len less than zero or memcpy_s failed. (dev_id=%u; msg_type=%u; in_date_len=%u)\n",
                   dev_id, (u32)msg_type, in_data_len);
        return -EINVAL;
    }

    ret = agentdrv_sync_non_trans_msg_chan_send(msg_chan, msg_type, &data_para);

    mutex_unlock(&msg_chan->mutex);

    return ret;
}
