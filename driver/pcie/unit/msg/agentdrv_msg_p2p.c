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
#include <linux/securec.h>

#include "agentdrv_msg_p2p.h"
#include "agentdrv_unit.h"
#include "devdrv_atu.h"
#include "devdrv_util.h"
#include "devdrv_dma.h"
#include "devdrv_interface.h"
#include "resource_comm_drv.h"

STATIC p2p_msg_recv g_p2p_msg_recv_proc[AGENTDRV_P2P_MSG_TYPE_MAX] = {
    NULL,
};

devdrv_notify_func g_online_notify_func = NULL;

int agentdrv_register_dev_online_proc_func(devdrv_notify_func func)
{
    g_online_notify_func = func;
    return 0;
}
EXPORT_SYMBOL(agentdrv_register_dev_online_proc_func);

int agentdrv_cfg_p2p_msg_chan(struct agentdrv_msg_dev *msg_dev, void *data)
{
#ifdef CFG_FEATURE_P2P
    struct devdrv_p2p_msg_chan_cfg_cmd *cmd_data = (struct devdrv_p2p_msg_chan_cfg_cmd *)data;
    struct devdrv_admin_msg_reply *reply = (struct devdrv_admin_msg_reply *)data;
    struct agentdrv_devctrl *agent_dev = agentdrv_get_dev(msg_dev->dev_id);
    struct devdrv_shr_para *shr_para = NULL;
    u32 dst_devid = cmd_data->devid;

    if ((dst_devid >= DEVDRV_P2P_SUPPORT_MAX_DEVICE) || (agent_dev == NULL)) {
        devdrv_err("dev_id or dst_devid error. (dev_id=%d; dst_devid=%u)\n", msg_dev->dev_id, dst_devid);
        return -EINVAL;
    }

    shr_para = agent_dev->shr_para[msg_dev->func_id];

    agentdrv_p2p_msg_chan_init(msg_dev, dst_devid, shr_para->p2p_msg_base_addr[dst_devid],
                               shr_para->p2p_db_base_addr[dst_devid], cmd_data->op);

    /* no response message */
    reply->len = sizeof(struct devdrv_admin_msg_reply);

    return 0;
#else
    return -EINVAL;
#endif
}

#ifdef CFG_FEATURE_P2P
STATIC int agentdrv_surport_h2d_config(u32 dev_id)
{
    u32 host_flag = 0;
    int ret;

    ret = agentdrv_get_host_phy_mach_flag(dev_id, &host_flag);
    if (ret != 0) {
        devdrv_err("Get host phy failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return -EINVAL;
    }

    if (host_flag != DEVDRV_HOST_PHY_MACH_FLAG) {
        devdrv_err("Device is virtual, and not surport h2d config. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }
    return 0;
}
#endif

int agentdrv_cfg_p2p_tx_atu(struct agentdrv_msg_dev *msg_dev, void *data)
{
#ifdef CFG_FEATURE_P2P
    struct devdrv_tx_atu_cfg_cmd *cmd_data = (struct devdrv_tx_atu_cfg_cmd *)data;
    struct devdrv_admin_msg_reply *reply = (struct devdrv_admin_msg_reply *)data;
    struct agentdrv_devctrl *agent_dev = agentdrv_get_dev(msg_dev->dev_id);
    struct devdrv_shr_para *shr_para = NULL;
    struct devdrv_cfg_tx_atu_para tx_para;
    u32 dst_devid = cmd_data->devid;
    int ret;
    u32 status;

    if ((((cmd_data->atu_type == ATU_TYPE_TX_MEM) || (cmd_data->atu_type == ATU_TYPE_TX_IO) ||
        (cmd_data->atu_type == ATU_TYPE_RX_MEM)) && (dst_devid >= DEVDRV_P2P_SUPPORT_MAX_DEVICE)) ||
        (agent_dev == NULL)) {
        devdrv_err("atu_type or dst_devid error. (atu_type=%d; dst_devid=%d)\n", cmd_data->atu_type, dst_devid);
        devdrv_tx_atu_print_cfg_info();
        return -EINVAL;
    }

    if ((cmd_data->atu_type == ATU_TYPE_TX_HOST) && (agentdrv_surport_h2d_config(agent_dev->agent_id) != 0)) {
        devdrv_err("virtual not surport h2d tx atu config. (dst_devid=%u)\n", dst_devid);
        devdrv_tx_atu_print_cfg_info();
        return -EINVAL;
    }

    shr_para = agent_dev->shr_para[msg_dev->func_id];
    tx_para.local_devid = agent_dev->agent_id;
    tx_para.host_devid = shr_para->host_dev_id;
    tx_para.dst_host_devid = dst_devid;
    tx_para.atu_type = cmd_data->atu_type;

    status = DEVDRV_DEV_OFFLINE;
    if (cmd_data->op == DEVDRV_OP_ADD) {
        status = DEVDRV_DEV_ONLINE;
        ret = devdrv_set_tx_atu(agent_dev->apb_base, &tx_para, cmd_data->target_addr, cmd_data->target_size, shr_para);
        if (ret != 0) {
            devdrv_err("Set atu failed. (dev_id=%d; host_devid=%d; atu_type=%d; dst_devid=%u)\n",
                agent_dev->agent_id, shr_para->host_dev_id, cmd_data->atu_type, dst_devid);
            devdrv_tx_atu_print_cfg_info();
            return ret;
        }

        (void)devdrv_get_dev_tx_atu(agent_dev->apb_base, agent_dev->tx_atu, DEVDRV_TX_ATU_NUM, &tx_para,
            cmd_data->phy_addr);
    }
    if ((g_online_notify_func != NULL) && (cmd_data->atu_type != ATU_TYPE_TX_HOST)) {
        g_online_notify_func(agent_dev->agent_id, DEVDRV_DEV_ONLINE_NOTIFY, dst_devid, status);
    }
    if (cmd_data->op == DEVDRV_OP_DEL) {
        (void)devdrv_del_dev_tx_atu(agent_dev->apb_base, agent_dev->tx_atu, DEVDRV_TX_ATU_NUM, &tx_para,
            cmd_data->target_addr);
    }

    /* no response message */
    reply->len = sizeof(struct devdrv_admin_msg_reply);

    return 0;
#else
    return -EINVAL;
#endif
}

STATIC struct devdrv_iob_atu *agentdrv_get_atu(struct agentdrv_devctrl *agent_dev,
    u32 host_devid, u32 dst_host_devid, u32 atu_type, u64 addr)
{
    struct devdrv_iob_atu *atu = NULL;
    u32 atu_id;

    if (atu_type != ATU_TYPE_TX_HOST) {
        atu_id = devdrv_get_atu_id(agent_dev->agent_id, host_devid, dst_host_devid, atu_type);
    } else {
        atu_id = devdrv_get_h2d_atu_id(agent_dev->agent_id, (dma_addr_t)addr);
    }
    if (atu_id >= DEVDRV_TX_ATU_NUM) {
        devdrv_warn("No more valid atu id. (dev_id=%u; dst_devid=%u; atu_type=%u)\n",
            host_devid, dst_host_devid, atu_type);
        return NULL;
    }
    atu = &agent_dev->tx_atu[atu_id];
    if (atu->valid == ATU_INVALID) {
        devdrv_warn("Get atu invalid. (dev_id=%u; dst_devid=%u; atu_type=%u)\n",
            host_devid, dst_host_devid, atu_type);
        return NULL;
    }
    return atu;
}

STATIC int agentdrv_get_p2p_addr_para_check(u32 local_devid, u32 dst_host_devid, enum devdrv_p2p_addr_type type,
    const phys_addr_t *phy_addr, u32 *size)
{
    if (local_devid >= MAX_AGENTDEV_CNT) {
        devdrv_err("Device ID overflow. (dev_id=%u)\n", local_devid);
        return -EINVAL;
    }
    if (phy_addr == NULL) {
        devdrv_err("Phyical addr is invalid. (dev_id=%u)\n", local_devid);
        return -EINVAL;
    }
    if (size == NULL) {
        devdrv_err("Input parameter is error. (dev_id=%u)", local_devid);
        return -EINVAL;
    }
    if (type >= DEVDRV_P2P_ADDR_TYPE_MAX) {
        devdrv_err("Input parameter is error. (dev_id=%u; type=%u)\n", local_devid, type);
        return -EINVAL;
    }
    if ((type != DEVDRV_P2P_HOST_MEM) && (dst_host_devid >= DEVDRV_P2P_SUPPORT_MAX_DEVICE)) {
        devdrv_err("Input parameter is error. (dst_host_devid=%u)\n", dst_host_devid);
        return -EINVAL;
    }
    return 0;
}

STATIC int agentdrv_get_atu_type(enum devdrv_p2p_addr_type type, u32 host_dev_id, u32 dst_host_devid, u32 *atu_type)
{
    if (type == DEVDRV_P2P_HOST_MEM) {
        *atu_type = ATU_TYPE_TX_HOST;
    } else {
        *atu_type = ATU_TYPE_TX_IO;
        if (host_dev_id == dst_host_devid) {
            devdrv_err("Type not surport self. (dst_devid=%u; type=%u)\n", dst_host_devid, type);
            return -EINVAL;
        }
    }
    return 0;
}

/* local_devid:device devid, devid: host devid */
int agentdrv_get_p2p_addr(u32 local_devid, u32 dst_host_devid, enum devdrv_p2p_addr_type type,
    phys_addr_t *phy_addr, u32 *size)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    struct devdrv_iob_atu *atu = NULL;
    u32 host_dev_id, chip_id, func_id, atu_type;

    if (agentdrv_get_p2p_addr_para_check(local_devid, dst_host_devid, type, phy_addr, size)) {
        return -EINVAL;
    }
    devdrv_dev2chipfunc(local_devid, &chip_id, &func_id);
    agent_dev = agentdrv_get_dev((int)chip_id);
    if ((agent_dev == NULL) || (agentdrv_is_dev_valid(agent_dev, func_id) == false)) {
        devdrv_err("agent_dev is invalid. (local_devid=%u)\n", local_devid);
        return -EINVAL;
    }
    host_dev_id = agent_dev->shr_para[func_id]->host_dev_id;

    if (agentdrv_get_atu_type(type, host_dev_id, dst_host_devid, &atu_type)) {
        return -EINVAL;
    }

    atu = agentdrv_get_atu(agent_dev, host_dev_id, dst_host_devid, atu_type, 0);
    if (atu == NULL) {
        return -EINVAL;
    }

    switch (type) {
        case DEVDRV_P2P_IO_TS_DB:
            *phy_addr = (phys_addr_t)(atu->base_addr + DEVDRV_IO_TS_DB_OFFSET);
            *size = DEVDRV_IO_TS_DB_SIZE;
            break;
        case DEVDRV_P2P_IO_TS_SRAM:
            *phy_addr = (phys_addr_t)(atu->base_addr + DEVDRV_IO_TS_SRAM_OFFSET);
            *size = DEVDRV_IO_TS_SRAM_SIZE;
            break;
        case DEVDRV_P2P_IO_HWTS:
            *phy_addr = (phys_addr_t)(atu->base_addr + DEVDRV_IO_HWTS_OFFSET);
            *size = DEVDRV_IO_HWTS_SIZE;
            break;
        default:
            devdrv_err("p2p address type not surport. (dst_host_devid=%u; type=%d)\n", dst_host_devid, (int)type);
            return -EINVAL;
    }

    return 0;
}
EXPORT_SYMBOL(agentdrv_get_p2p_addr);

int agentdrv_devmem_txatu_target_to_base(u32 local_devid, u32 dst_host_devid, phys_addr_t target_addr,
    phys_addr_t *base_addr)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    struct devdrv_iob_atu *atu = NULL;
    u32 chip_id = 0;
    u32 func_id = 0;
    u32 host_devid;

    if (local_devid >= MAX_AGENTDEV_CNT) {
        devdrv_err("local_devid is error. (local_devid=%u)\n", local_devid);
        return -EINVAL;
    }

    if (dst_host_devid >= DEVDRV_P2P_SUPPORT_MAX_DEVICE) {
        devdrv_err("dst_host_devid is error. (local_devid=%u; dst_host_devid=%u)\n", local_devid, dst_host_devid);
        return -EINVAL;
    }

    if (base_addr == NULL) {
        devdrv_err("Input parameter is null. (local_devid=%u)\n", local_devid);
        return -EINVAL;
    }

    devdrv_dev2chipfunc(local_devid, &chip_id, &func_id);
    agent_dev = agentdrv_get_dev((int)chip_id);
    if ((agent_dev == NULL) || (agentdrv_is_dev_valid(agent_dev, func_id) == false)) {
        devdrv_err("agent_dev is invalid. (local_devid=%u)\n", local_devid);
        return -EINVAL;
    }
    host_devid = (u32)agent_dev->shr_para[func_id]->host_dev_id;
    atu = agentdrv_get_atu(agent_dev, host_devid, dst_host_devid, ATU_TYPE_TX_MEM, target_addr);
    if (atu == NULL) {
        return -EINVAL;
    }

    if (((u64)target_addr < atu->phy_addr) || ((u64)target_addr >= atu->phy_addr + atu->size)) {
        devdrv_err("Target address is not in range. (local_devid=%u; dst_host_devid=%u; range_size=0x%llx)\n",
                   local_devid, dst_host_devid, atu->size);
        return -EINVAL;
    }

    *base_addr = (phys_addr_t)(atu->base_addr + ((u64)target_addr - atu->phy_addr));

    return 0;
}
EXPORT_SYMBOL(agentdrv_devmem_txatu_target_to_base);

int agentdrv_devmem_txatu_host_target_to_base(u32 local_devid, phys_addr_t target_addr, phys_addr_t *base_addr)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    struct devdrv_iob_atu *atu = NULL;
    u32 chip_id = 0;
    u32 func_id = 0;
    u32 host_dev_id;
    u32 dst_host_devid = 0;

    if (local_devid >= MAX_AGENTDEV_CNT) {
        devdrv_err("local_devid is error. (local_devid=%u)\n", local_devid);
        return -EINVAL;
    }

    if (base_addr == NULL) {
        devdrv_err("Input parameter is null. (local_devid=%u)\n", local_devid);
        return -EINVAL;
    }

    devdrv_dev2chipfunc(local_devid, &chip_id, &func_id);
    agent_dev = agentdrv_get_dev((int)chip_id);
    if ((agent_dev == NULL) || (agentdrv_is_dev_valid(agent_dev, func_id) == false)) {
        devdrv_err("agent_dev is invalid. (local_devid=%u)\n", local_devid);
        return -EINVAL;
    }

    host_dev_id = (u32)agent_dev->shr_para[func_id]->host_dev_id;
    atu = agentdrv_get_atu(agent_dev, host_dev_id, dst_host_devid, ATU_TYPE_TX_HOST, target_addr);
    if (atu == NULL) {
        devdrv_err("Target address is not in range. (local_devid=%u; host_dev_id=%u)\n",
                   local_devid, host_dev_id);
        return -EINVAL;
    }

    if (((u64)target_addr < atu->phy_addr) || ((u64)target_addr >= atu->phy_addr + atu->size)) {
        devdrv_err("Target address is not in range. (local_devid=%u; host_dev_id=%u; range_size=0x%llx)\n",
                   local_devid, host_dev_id, atu->size);
        return -EINVAL;
    }

    *base_addr = (phys_addr_t)(atu->base_addr + ((u64)target_addr - atu->phy_addr));

    return 0;
}
EXPORT_SYMBOL(agentdrv_devmem_txatu_host_target_to_base);

STATIC int agentdrv_p2p_check_para(u32 local_devid, u32 host_devid, const struct agentdrv_msg_dev *msg_dev,
    const struct agentdrv_p2p_msg_send_data_para *data_para, u32 max_data_len)
{
    if ((data_para->data_len > max_data_len) || (data_para->in_len > max_data_len)) {
        devdrv_err("buf_len or in_len overflow. (local_devid=%u; max_data_len=%u; buf_len=%d; in_len=%d)\n",
            local_devid, max_data_len, data_para->data_len, data_para->in_len);
        return -EINVAL;
    }
    if ((msg_dev == NULL) || (data_para->data == NULL) || (data_para->out_len == NULL)) {
        devdrv_err("Input parameter is null. (local_devid=%u)\n", local_devid);
        return -EINVAL;
    }
    if (host_devid >= DEVDRV_P2P_SUPPORT_MAX_DEVICE) {
        devdrv_err("host_devid is error. (local_devid=%u; host_devid=%u)\n", local_devid, host_devid);
        return -EINVAL;
    }
    return 0;
}

STATIC int devdrv_p2p_ring_remote_doorbell(struct agentdrv_p2p_msg_chan *chan, u32 db_val)
{
    enum devdrv_dma_data_type data_type = DEVDRV_DMA_DATA_PCIE_MSG;
    struct devdrv_dma_dev *dma_dev = NULL;

    dma_dev = devdrv_get_dma_dev(chan->devid);
    if (dma_dev == NULL) {
        devdrv_err("Call devdrv_get_dma_dev failed, dma_dev is NULL. (dev_id=%u)\n", chan->devid);
        return -EINVAL;
    }
    return devdrv_dma_copy_sml_pkt(dma_dev, data_type, chan->remote_db_addr, (void *)&db_val, sizeof(u32));
}

int agentdrv_p2p_msg_chan_send(u32 local_devid, u32 devid, struct agentdrv_p2p_msg_chan *chan,
    enum agentdrv_p2p_msg_type msg_type, struct agentdrv_p2p_msg_send_data_para *data_para)
{
    enum devdrv_dma_data_type data_type = DEVDRV_DMA_DATA_PCIE_MSG;
    u32 max_data_len = DEVDRV_P2P_MSG_SIZE - sizeof(struct agentdrv_p2p_msg);
    int timeout = DEVDRV_MSG_TIMEOUT;
    int ret;
    int status = 0;
    u32 in_len = data_para->in_len;

    chan->send_msg->msg_type = (int)msg_type;
    chan->send_msg->buf_len = data_para->data_len;
    chan->send_msg->in_len = in_len;
    chan->send_msg->out_len = 0;
    chan->send_msg->status = DEVDRV_MSG_CMD_BEGIN;

    if ((in_len <= 0) || (memcpy_s((void *)chan->send_msg->data, max_data_len, data_para->data, in_len) != 0)) {
        devdrv_err("in_len is invalid or memcpy_s failed. (dev_id=%u; in_len=%u)\n", local_devid, in_len);
        return -EINVAL;
    }

    dma_sync_single_for_device(chan->dev, (dma_addr_t)chan->send_msg_dma_addr,
        in_len + sizeof(struct agentdrv_p2p_msg), DMA_TO_DEVICE);

    /* dma copy messages to the other device */
    ret = devdrv_dma_sync_copy(local_devid, data_type, (dma_addr_t)chan->send_msg_dma_addr,
                               (dma_addr_t)chan->remote_recv_msg_dma_addr, in_len + sizeof(struct agentdrv_p2p_msg),
                               DEVDRV_DMA_DEVICE_TO_HOST);
    if (ret) {
        devdrv_err("p2p msg send faild. (dev_id=%u; dst_dev_id=%u)\n", local_devid, devid);
        return ret;
    }

    /* soc doorbell must write 1 to raise the interrupt */
    if (agentdrv_get_soc_doorbell_capability() == true) {
        chan->send_num = 0x1;
    }
    /* inform the other device */
    ret = devdrv_p2p_ring_remote_doorbell(chan, chan->send_num);
    if (ret != 0) {
        devdrv_err("p2p db send faild. (local_devid=%u; dst_devid=%u)\n", local_devid, devid);
        return ret;
    }

    /* wait result */
    while (timeout > 0) {
        status = chan->send_msg->status;
        if (status != DEVDRV_MSG_CMD_BEGIN) {
            break;
        }

        rmb();

        usleep_range(DEVDRV_MSG_WAIT_MIN_TIME, DEVDRV_MSG_WAIT_MAX_TIME);
        timeout -= DEVDRV_MSG_WAIT_MIN_TIME;
    }
    mb();
    if (status != DEVDRV_MSG_CMD_FINISH_SUCCESS) {
        devdrv_err("Get finish status remain time. (dev_id=%u; dst_devid=%u; status=%d; time=%d)\n",
                   local_devid, devid, status, timeout);
        return -ENOSYS;
    }

    *(data_para->out_len) = chan->send_msg->out_len;
    if (*(data_para->out_len) > 0) {
        ret = memcpy_s(data_para->data, data_para->data_len, (void *)chan->send_msg->data, *(data_para->out_len));
        if (ret) {
            devdrv_err("memcpy_s failed. (ret=%d)\n", ret);
        }
    }
    chan->send_num++;

    return ret;
}

/* local_devid:device devid, devid: host devid */
int agentdrv_p2p_msg_send(u32 local_devid, u32 devid, enum agentdrv_p2p_msg_type msg_type, void *data, u32 data_len,
                          u32 in_len, u32 *out_len)
{
    struct agentdrv_p2p_msg_chan *chan = NULL;
    struct agentdrv_msg_dev *msg_dev = agentdrv_get_msg_dev(local_devid);
    struct agentdrv_p2p_msg_send_data_para data_para;
    u32 max_data_len = DEVDRV_P2P_MSG_SIZE - sizeof(struct agentdrv_p2p_msg);
    int ret;

    data_para.data = data;
    data_para.data_len = data_len;
    data_para.in_len = in_len;
    data_para.out_len = out_len;
    ret = agentdrv_p2p_check_para(local_devid, devid, msg_dev, &data_para, max_data_len);
    if (ret) {
        devdrv_err("Function agentdrv_p2p_msg_send parameters check failed.\n");
        return ret;
    }

    devdrv_debug("Parameters check ok. (local_devid=0x%x; host_devid=0x%x; msg_type=%x; date_len=%x; in_len=%x)\n",
                 local_devid, devid, msg_type, data_len, in_len);

    chan = &msg_dev->p2p_chan[devid];
    if (chan->valid == AGENTDRV_DISABLE) {
        devdrv_err("p2p dst_device not exist. (local_devid=%u; dst_devid=%u)", local_devid, devid);
        return -EINVAL;
    }
    mutex_lock(&chan->lock);

    ret = agentdrv_p2p_msg_chan_send(local_devid, devid, chan, msg_type, &data_para);
    if (ret) {
        devdrv_err("p2p dst_device msg_chan send failed. (dev_id=%u; dst_devid=%u; ret=%d; data_len=%u; out_len=%u)\n",
                   local_devid, devid, ret, data_len, *out_len);
    }

    mutex_unlock(&chan->lock);

    return ret;
}
EXPORT_SYMBOL(agentdrv_p2p_msg_send);

void agentdrv_p2p_msg_proc_task(struct work_struct *p_work)
{
    struct agentdrv_p2p_msg_chan *chan = container_of(p_work, struct agentdrv_p2p_msg_chan, msg_proc_task);
    struct agentdrv_p2p_msg *recv_msg = chan->recv_msg;
    enum devdrv_dma_data_type data_type = DEVDRV_DMA_DATA_PCIE_MSG;
    int ret;
    u32 msg_type;

    dma_sync_single_for_cpu(chan->dev, (dma_addr_t)chan->recv_msg_dma_addr, DEVDRV_P2P_MSG_SIZE, DMA_FROM_DEVICE);

    if (recv_msg->status != DEVDRV_MSG_CMD_BEGIN) {
        devdrv_err("p2p message channel recv status error. (dev_id=%u; msg_chan=%u; status=%d)\n",
                   chan->devid, chan->chan_id, recv_msg->status);
        return;
    }

    msg_type = recv_msg->msg_type;
    if (msg_type >= (u32)AGENTDRV_P2P_MSG_TYPE_MAX) {
        devdrv_err("p2p message channel recv message type error. (dev_id=%u; msg_chan=%u; msg type=%u)\n",
                   chan->devid, chan->chan_id, msg_type);
        return;
    }
    if (g_p2p_msg_recv_proc[msg_type] != NULL) {
        ret = g_p2p_msg_recv_proc[msg_type](chan->devid, (void *)recv_msg->data, recv_msg->buf_len, recv_msg->in_len,
                                            &recv_msg->out_len);
        if ((ret != 0) || (recv_msg->out_len > recv_msg->buf_len)) {
            devdrv_err("out_len is invalid. (dev_id=%u; msg_chan=%u; msg_type=%u; out_len=%u; buf_len=%u; in_len=%u)\n",
                       chan->devid, chan->chan_id, msg_type, recv_msg->out_len, recv_msg->buf_len, recv_msg->in_len);
            ret = -1;
        }
    } else {
        devdrv_err("p2p message channel recv message type not register. (dev_id=%u; msg_chan=%u; msg_type=%u)\n",
                   chan->devid, chan->chan_id, msg_type);
        ret = -1;
    }

    if (ret != 0) {
        recv_msg->status = DEVDRV_MSG_CMD_FINISH_FAILED;
    } else {
        ret = devdrv_dma_sync_copy(chan->devid, data_type,
                                   (dma_addr_t)(chan->recv_msg_dma_addr + DEVDRV_P2P_MSG_HEAD_LEN),
                                   (dma_addr_t)(chan->remote_send_msg_dma_addr + DEVDRV_P2P_MSG_HEAD_LEN),
                                   recv_msg->out_len, DEVDRV_DMA_DEVICE_TO_HOST);
        if (ret) {
            devdrv_err("p2p message channel recv reply failed. (dev_id=%u; msg_chan=%u; msg_type=%u; ret=%d)\n",
                       chan->devid, chan->chan_id, msg_type, ret);

            recv_msg->status = DEVDRV_MSG_CMD_FINISH_FAILED;
        } else {
            recv_msg->status = DEVDRV_MSG_CMD_FINISH_SUCCESS;
        }
    }

    /* respond to host side to execute result status */
    ret = devdrv_respond_msg_to_remote_side(chan->devid, chan->recv_msg_dma_addr, chan->remote_send_msg_dma_addr,
        (void *)recv_msg, (u32)DEVDRV_P2P_MSG_HEAD_LEN);
    if (ret != 0) {
        devdrv_err("p2p message channel recv message reply status fail. (dev_id=%u; msg_chan=%u; "
                   "msg_type=%u; ret=%d)\n", chan->devid, chan->chan_id, msg_type, ret);
    }
}

void agentdrv_p2p_msg_chan_doorbell_proc(struct agentdrv_p2p_msg_chan *chan)
{
    if (chan->valid == AGENTDRV_DISABLE) {
        devdrv_warn("Channel vaild is disable.\n");
        return;
    }

    (void)schedule_work(&chan->msg_proc_task);
}

int agentdrv_register_p2p_msg_proc_func(enum agentdrv_p2p_msg_type msg_type, p2p_msg_recv func)
{
    if ((u32)msg_type >= AGENTDRV_P2P_MSG_TYPE_MAX) {
        devdrv_err("message type error. (msg_type=%d)\n", (int)msg_type);
        return -EINVAL;
    }

    g_p2p_msg_recv_proc[msg_type] = func;
    return 0;
}
EXPORT_SYMBOL(agentdrv_register_p2p_msg_proc_func);

void agentdrv_p2p_msg_chan_init(struct agentdrv_msg_dev *msg_dev, int dst_devid, u64 msg_base, u64 db_base, u32 op)
{
    struct agentdrv_p2p_msg_chan *chan = &msg_dev->p2p_chan[dst_devid];
    u32 offset;
    int host_dev_id;
    struct agentdrv_devctrl *agent_dev = agentdrv_get_dev(msg_dev->dev_id);
    u32 p2p_db_start, p2p_db_end;

    if (op == DEVDRV_OP_DEL) {
        chan->remote_send_msg_dma_addr = 0;
        chan->remote_recv_msg_dma_addr = 0;
        chan->remote_db_addr = 0;
        chan->valid = AGENTDRV_DISABLE;
        devdrv_info("p2p msg chan delete.\n");
        return;
    }

    if ((msg_base == 0) || (db_base == 0)) {
        devdrv_err("Input parameter invalid.\n");
        return;
    }

    if (agent_dev == NULL) {
        devdrv_err("Function agentdrv_get_dev failed, agent_dev is null.\n");
        return;
    }

    host_dev_id = agent_dev->shr_para[msg_dev->func_id]->host_dev_id;

    offset = (u32)(DEVDRV_P2P_MSG_ADDR_OFFSET + DEVDRV_P2P_SEND_MSG_ADDR_OFFSET + host_dev_id * DEVDRV_P2P_MSG_SIZE);
    chan->remote_send_msg_dma_addr = msg_base + offset;

    offset = (u32)(DEVDRV_P2P_MSG_ADDR_OFFSET + DEVDRV_P2P_RECV_MSG_ADDR_OFFSET + host_dev_id * DEVDRV_P2P_MSG_SIZE);
    chan->remote_recv_msg_dma_addr = msg_base + offset;

    agentdrv_get_p2p_msg_db_range((u32)msg_dev->func_id, &p2p_db_start, &p2p_db_end);
    offset = (u32)((p2p_db_start + (u32)host_dev_id) * AGENTDRV_DB_STRDE);
    chan->remote_db_addr = db_base + offset;

    chan->valid = AGENTDRV_ENABLE;
    devdrv_info("P2P message channel init success. (host_dev_id=%d; dst_dev_id=%d)\n", host_dev_id, dst_devid);
}

void agentdrv_p2p_msg_init(struct agentdrv_msg_dev *msg_dev)
{
    u32 i;
    struct agentdrv_p2p_msg_chan *chan = NULL;
    u32 offset;
    u64 send_phy_addr, recv_phy_addr;
    struct page *page = NULL;

    for (i = 0; i < DEVDRV_P2P_SUPPORT_MAX_DEVICE; i++) {
        chan = &msg_dev->p2p_chan[i];
        chan->chan_id = i;
        chan->devid = msg_dev->dev_id;
        chan->send_num = AGENTDRV_MSG_SEND_NUM;
        chan->dev = msg_dev->dev;

        offset = (u32)(DEVDRV_P2P_MSG_ADDR_OFFSET + DEVDRV_P2P_SEND_MSG_ADDR_OFFSET + i * DEVDRV_P2P_MSG_SIZE);
        chan->send_msg = (struct agentdrv_p2p_msg *)(msg_dev->reserve_mem_base + offset);
        /*lint -e648 */
        send_phy_addr = agentdrv_get_dev_rsv_mem_base(msg_dev->dev_id, msg_dev->func_id) + offset;
        page = phys_to_page(send_phy_addr);
        /*lint +e648 */
        chan->send_msg_dma_addr = (u64)dma_map_page(msg_dev->dev, page, send_phy_addr % PAGE_SIZE, DEVDRV_P2P_MSG_SIZE,
                                                    DMA_BIDIRECTIONAL);

        offset = (u32)(DEVDRV_P2P_MSG_ADDR_OFFSET + DEVDRV_P2P_RECV_MSG_ADDR_OFFSET + i * DEVDRV_P2P_MSG_SIZE);
        chan->recv_msg = (struct agentdrv_p2p_msg *)(msg_dev->reserve_mem_base + offset);
        /*lint -e648 */
        recv_phy_addr = agentdrv_get_dev_rsv_mem_base(msg_dev->dev_id, msg_dev->func_id) + offset;
        page = phys_to_page(recv_phy_addr);
        /*lint +e648 */
        chan->recv_msg_dma_addr = (u64)dma_map_page(msg_dev->dev, page, recv_phy_addr % PAGE_SIZE, DEVDRV_P2P_MSG_SIZE,
                                                    DMA_BIDIRECTIONAL);
        mutex_init(&chan->lock);
        INIT_WORK(&chan->msg_proc_task, agentdrv_p2p_msg_proc_task);
    }
}

void agentdrv_p2p_msg_exit(struct agentdrv_msg_dev *msg_dev)
{
    int i;
    struct agentdrv_p2p_msg_chan *chan = NULL;

    for (i = 0; i < DEVDRV_P2P_SUPPORT_MAX_DEVICE; i++) {
        chan = &msg_dev->p2p_chan[i];

        (void)cancel_work_sync(&chan->msg_proc_task);

        dma_unmap_page(msg_dev->dev, (dma_addr_t)chan->send_msg_dma_addr, DEVDRV_P2P_MSG_SIZE, DMA_BIDIRECTIONAL);

        dma_unmap_page(msg_dev->dev, (dma_addr_t)chan->recv_msg_dma_addr, DEVDRV_P2P_MSG_SIZE, DMA_BIDIRECTIONAL);
    }
}
