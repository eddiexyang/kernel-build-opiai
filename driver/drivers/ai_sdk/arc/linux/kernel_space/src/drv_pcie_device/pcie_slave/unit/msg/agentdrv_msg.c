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

#ifdef CONFIG_GENERIC_BUG
#undef CONFIG_GENERIC_BUG
#endif
#ifdef CONFIG_BUG
#undef CONFIG_BUG
#endif
#include <linux/delay.h>
#include <linux/securec.h>
#include <linux/kallsyms.h>

#include "agentdrv_msg.h"
#include "agentdrv_msg_admin.h"
#include "agentdrv_unit.h"
#include "agentdrv_msg_p2p.h"
#include "agentdrv_msg_trans.h"
#include "agentdrv_msg_non_trans.h"
#include "devdrv_interface.h"
#include "devdrv_util.h"
#include "devdrv_dma.h"
#include "agentdrv_common_msg.h"
#include "agentdrv_multi_chip.h"
#include "resource_drv.h"
#include "nvme_drv.h"
#include "kernel_version_adapt.h"

#ifdef CFG_SOC_PLATFORM_MDC_V51
#include "workqueue_affinity.h"
#include "drv_cpu_mask.h"
#include "drv_cpu_type.h"
#endif

u32 agentdrv_get_agentid_by_msg_dev(const struct agentdrv_msg_dev *msg_dev)
{
    int slot_id = 0;
    devdrv_chipfunc2dev(&slot_id, msg_dev->dev_id, msg_dev->func_id);
    return (u32)slot_id;
}

u64 agentdrv_get_dev_rsv_mem_base(int chip_id, int func_id)
{
    struct agent_res_sdi_addr sdi_addr = {0};

    agentdrv_res_sdi_addr(chip_id, func_id, &sdi_addr);

    return sdi_addr.rsv_mem_base;
}

/* to be delete */
int agentdrv_get_msg_chan_queue_depth(void *msg_chan)
{
    struct agentdrv_msg_chan *chan = (struct agentdrv_msg_chan *)msg_chan;
    if (chan == NULL) {
        devdrv_err("Input parameter is null.\n");
        return -EINVAL;
    }
    return chan->sq.depth;
}

void agentdrv_iocmd_raise_int_to_h(struct agentdrv_msg_dev *dev, int irq)
{
    struct agentdrv_devctrl *agent_dev = (struct agentdrv_devctrl *)dev->priv_pci_unit;
    struct agentdrv_pfvf_info pfvf_info = { 0 };

    devdrv_soc_func2pfvf(dev->dev_id, dev->func_id, 1, &pfvf_info);

    irq += agent_dev->shr_para[dev->func_id]->msix_offset;
    devdrv_raise_int_to_h(dev->raise_int_base, pfvf_info.pf_num, pfvf_info.vf_num, irq);
}

/* interrupt */
void agentdrv_set_dma_host_err_irq(int dev_id, u32 dma_chan_id, int err_irq)
{
    struct agentdrv_devctrl *agent_dev = agentdrv_get_dev(dev_id);
    if (agent_dev == NULL) {
        devdrv_err("Function agentdrv_get_dev failed, agent_dev is null.\n");
        return;
    }

    if (dma_chan_id >= DEVDRV_DMA_CHAN_NUM) {
        devdrv_err("DMA channel ID is invalid. (dma_chan_id=%d)\n", dma_chan_id);
    } else {
        agent_dev->dma_host_err_irq[dma_chan_id] = (u32)err_irq;
    }
}

/* err interrupt reported to local, device side without notification */
int devdrv_notify_dma_err_irq(void *drvdata, u32 dma_chan_id, int err_irq)
{
    return 0;
}

void agentdrv_msg_chan_sq_doorbell_proc(struct agentdrv_msg_chan *chan)
{
    if (chan->status == AGENTDRV_DISABLE) {
        devdrv_warn("Channel status is disable.\n");
        return;
    }

    if (chan->sq.db_fun != NULL) {
        chan->sq.db_fun(chan);
    }
}

void agentdrv_msg_chan_cq_doorbell_proc(struct agentdrv_msg_chan *chan)
{
    if (chan->status == AGENTDRV_DISABLE) {
        devdrv_warn("Channel status is disable.\n");
        return;
    }

    if (chan->cq.db_fun != NULL) {
        chan->cq.db_fun(chan);
    }
}

struct agentdrv_msg_dev *agentdrv_alloc_msg_dev(u32 func_id)
{
    struct agentdrv_msg_dev *dev = NULL;
    u32 size, msg_chan_num;

    agentdrv_get_dma_max_msg_chan(func_id, &msg_chan_num);

    size = sizeof(struct agentdrv_msg_dev) + sizeof(struct agentdrv_msg_chan) * msg_chan_num;
    dev = (struct agentdrv_msg_dev *)kzalloc(size, GFP_KERNEL);
    if (dev == NULL) {
        devdrv_err("Call alloc failed, dev is null.\n");
        return NULL;
    }
    dev->io_chan_cnt = msg_chan_num;

    return dev;
}

void agentdrv_free_msg_dev(const struct agentdrv_msg_dev *msg_dev)
{
    kfree(msg_dev);
    msg_dev = NULL;
}

int agentdrv_msg_alloc_local_sq(struct agentdrv_msg_chan *chan, u32 depth, u32 bd_size)
{
    size_t alloc_size = (size_t)depth * bd_size;

    chan->sq.base_d = devdrv_dma_zalloc_coherent(chan->msg_dev->dev, alloc_size, &chan->sq.dma_base_d,
        GFP_KERNEL | __GFP_DMA);

    if (chan->sq.base_d == NULL) {
        devdrv_err("Call msg_alloc_local_sq failed. (dev_id=%d; func_id=%d; chan_id=%u)\n",
                   chan->msg_dev->dev_id,
                   chan->msg_dev->func_id,
                   chan->chan_id);
        return -ENOMEM;
    }

    chan->sq.depth = depth;
    chan->sq.desc_size = bd_size;
    chan->sq.head = 0;
    chan->sq.tail = 0;

    return 0;
}

STATIC int agentdrv_msg_alloc_local_cq(struct agentdrv_msg_chan *chan, u32 depth, u32 bd_size)
{
    size_t alloc_size = (size_t)depth * bd_size;

    chan->cq.base_d = devdrv_dma_zalloc_coherent(chan->msg_dev->dev, alloc_size, &chan->cq.dma_base_d,
        GFP_KERNEL | __GFP_DMA);

    if (chan->cq.base_d == NULL) {
        devdrv_err("Call msg_alloc_local_cq failed. (dev_id=%d; func_id=%d; chan_id=%u)\n",
                   chan->msg_dev->dev_id,
                   chan->msg_dev->func_id,
                   chan->chan_id);
        return -ENOMEM;
    }

    chan->cq.depth = depth;
    chan->cq.desc_size = bd_size;
    chan->cq.head = 0;
    chan->cq.tail = 0;

    return 0;
}

STATIC int agentdrv_msg_alloc_reserve_sq(struct agentdrv_msg_chan *msg_chan, u32 depth, u32 bd_size, u32 reserve_offset)
{
    struct agentdrv_msg_dev *msg_dev = msg_chan->msg_dev;
    u32 queue_size;
    u32 offset;
    dma_addr_t dma_addr_d;
    struct page *page = NULL;

    offset = reserve_offset;
    msg_chan->sq.base_reserve_d = msg_dev->reserve_mem_base + offset;
    queue_size = depth * bd_size;

    if (memset_s(msg_chan->sq.base_reserve_d, queue_size, 0, queue_size) != 0) {
        devdrv_err("Call memset_s failed.\n");
        return -ENOMEM;
    }

    /*lint -e648 */
    page = phys_to_page(agentdrv_get_dev_rsv_mem_base(msg_chan->msg_dev->dev_id, msg_chan->msg_dev->func_id) + offset);
    /*lint +e648 */
    dma_addr_d = dma_map_page(msg_dev->dev, page, 0, queue_size, DMA_BIDIRECTIONAL);
    if (dma_mapping_error(msg_dev->dev, dma_addr_d)) {
        devdrv_err("DMA mapping error.\n");
        return -ENOMEM;
    }
    msg_chan->sq.dma_reserve_d = dma_addr_d;
    msg_chan->sq.head_res = 0;
    msg_chan->sq.tail_res = 0;

    return 0;
}

STATIC int agentdrv_msg_alloc_reserve_cq(struct agentdrv_msg_chan *msg_chan, u32 depth, u32 bd_size, u32 reserve_offset)
{
    struct agentdrv_msg_dev *msg_dev = msg_chan->msg_dev;
    u32 queue_size;
    u32 offset;
    dma_addr_t dma_addr_d;
    struct page *page = NULL;

    offset = reserve_offset;
    msg_chan->cq.base_reserve_d = msg_dev->reserve_mem_base + offset;
    queue_size = depth * bd_size;

    if (memset_s(msg_chan->cq.base_reserve_d, queue_size, 0, queue_size) != 0) {
        devdrv_err("Call memset_s failed.\n");
        return -ENOMEM;
    }

    /*lint -e648 */
    page = phys_to_page(agentdrv_get_dev_rsv_mem_base(msg_chan->msg_dev->dev_id, msg_chan->msg_dev->func_id) + offset);
    /*lint +e648 */
    dma_addr_d = dma_map_page(msg_dev->dev, page, 0, queue_size, DMA_BIDIRECTIONAL);
    if (dma_mapping_error(msg_dev->dev, dma_addr_d)) {
        devdrv_err("DMA mapping error.\n");
        return -ENOMEM;
    }
    msg_chan->cq.dma_reserve_d = dma_addr_d;
    msg_chan->cq.head_res = 0;
    msg_chan->cq.tail_res = 0;

    return 0;
}

STATIC int agentdrv_msg_free_reserve_sq(struct agentdrv_msg_chan *msg_chan)
{
    struct agentdrv_msg_dev *msg_dev = NULL;
    u64 queue_size;

    msg_dev = msg_chan->msg_dev;
    if (msg_chan->sq.dma_reserve_d) {
        queue_size = (u64)(msg_chan->sq.depth) * msg_chan->sq.desc_size;
        dma_unmap_page(msg_dev->dev, msg_chan->sq.dma_reserve_d, queue_size, DMA_BIDIRECTIONAL);
        msg_chan->sq.dma_reserve_d = 0;
    }

    return 0;
}

STATIC int agentdrv_msg_free_reserve_cq(struct agentdrv_msg_chan *msg_chan)
{
    struct agentdrv_msg_dev *msg_dev = NULL;
    u64 queue_size;

    msg_dev = msg_chan->msg_dev;
    if (msg_chan->cq.dma_reserve_d) {
        queue_size = (u64)(msg_chan->cq.depth) * msg_chan->cq.desc_size;
        dma_unmap_page(msg_dev->dev, msg_chan->cq.dma_reserve_d, queue_size, DMA_BIDIRECTIONAL);
        msg_chan->cq.dma_reserve_d = 0;
    }

    return 0;
}

STATIC int agentdrv_msg_free_local_sq(struct agentdrv_msg_chan *msg_chan)
{
    u64 free_size;

    if (msg_chan->sq.base_d != NULL) {
        free_size = (u64)(msg_chan->sq.desc_size) * msg_chan->sq.depth;
        devdrv_dma_free_coherent(msg_chan->msg_dev->dev, free_size, msg_chan->sq.base_d, msg_chan->sq.dma_base_d);
        msg_chan->sq.base_d = 0;
    }

    return 0;
}

STATIC int agentdrv_msg_free_local_cq(struct agentdrv_msg_chan *msg_chan)
{
    u64 free_size;

    if (msg_chan->cq.base_d != NULL) {
        free_size = (u64)(msg_chan->cq.desc_size) * msg_chan->cq.depth;
        devdrv_dma_free_coherent(msg_chan->msg_dev->dev, free_size, msg_chan->cq.base_d, msg_chan->cq.dma_base_d);
        msg_chan->cq.base_d = 0;
    }

    return 0;
}

int agentdrv_msg_alloc_s_queue(struct agentdrv_msg_chan *chan, u32 depth, u32 bd_size, u32 reserve_offset)
{
    int ret;

    ret = agentdrv_msg_alloc_local_sq(chan, depth, bd_size);
    if (ret != 0) {
        devdrv_err("Call agentdrv_msg_alloc_local_sq failed. (ret=%d)\n", ret);
        return ret;
    }
    ret = agentdrv_msg_alloc_reserve_sq(chan, depth, bd_size, reserve_offset);

    return ret;
}

int agentdrv_msg_alloc_c_queue(struct agentdrv_msg_chan *chan, u32 depth, u32 bd_size, u32 reserve_offset)
{
    int ret;

    ret = agentdrv_msg_alloc_local_cq(chan, depth, bd_size);
    if (ret != 0) {
        devdrv_err("Call agentdrv_msg_alloc_local_cq failed. (ret=%d)\n", ret);
        return ret;
    }
    ret = agentdrv_msg_alloc_reserve_cq(chan, depth, bd_size, reserve_offset);

    return ret;
}

int agentdrv_msg_free_s_queue(struct agentdrv_msg_chan *chan)
{
    u32 ret;

    ret = (u32)agentdrv_msg_free_local_sq(chan);
    ret = ret | ((u32)agentdrv_msg_free_reserve_sq(chan));
    return (int)ret;
}

int agentdrv_msg_free_c_queue(struct agentdrv_msg_chan *chan)
{
    u32 ret;

    ret = (u32)agentdrv_msg_free_local_cq(chan);
    ret = ret | ((u32)agentdrv_msg_free_reserve_cq(chan));
    return (int)ret;
}

STATIC int agentdrv_msg_free_msg_queue_res(struct agentdrv_msg_chan *msg_chan)
{
    mutex_lock(&msg_chan->mutex);
    msg_chan->status = AGENTDRV_DISABLE;
    if (msg_chan->rx_work_flag != 0) {
        (void)cancel_work_sync(&msg_chan->rx_work);
        msg_chan->rx_work_flag = 0;
    }

    msg_chan->sq.db_fun = NULL;
    msg_chan->cq.db_fun = NULL;

    (void)agentdrv_msg_free_s_queue(msg_chan);
    (void)agentdrv_msg_free_c_queue(msg_chan);
    mutex_unlock(&msg_chan->mutex);

    return 0;
}

STATIC int agentdrv_msg_alloc_sqcq(
    struct agentdrv_msg_chan *msg_chan, const struct devdrv_create_queue_command *cmd_data)
{
    int ret = 0;

    if (cmd_data->sq_depth && cmd_data->sq_desc_size) {
        ret = agentdrv_msg_alloc_s_queue(msg_chan, cmd_data->sq_depth,
                                         cmd_data->sq_desc_size, cmd_data->sq_slave_mem_offset);
        if (ret != 0) {
            devdrv_err("SQ alloc failed. (dev_id=%d; func_id=%d; chan_id=%u)\n",
                       msg_chan->msg_dev->dev_id,
                       msg_chan->msg_dev->func_id,
                       msg_chan->chan_id);
            (void)agentdrv_msg_free_msg_queue_res(msg_chan);
            return ret;
        }
    }

    if (cmd_data->cq_depth && cmd_data->cq_desc_size) {
        ret = agentdrv_msg_alloc_c_queue(msg_chan, cmd_data->cq_depth,
                                         cmd_data->cq_desc_size, cmd_data->cq_slave_mem_offset);
        if (ret != 0) {
            devdrv_err("CQ alloc failed. (dev_id=%d; func_id=%d; chan_id=%u)\n",
                       msg_chan->msg_dev->dev_id,
                       msg_chan->msg_dev->func_id,
                       msg_chan->chan_id);
            (void)agentdrv_msg_free_msg_queue_res(msg_chan);
            return ret;
        }
    }

    return 0;
}

irqreturn_t devdrv_remote_dma_err_interrupt(int irq, void *data)
{
    struct agentdrv_devctrl *agent_dev_t = (struct agentdrv_devctrl *)data;

    u32 ch_id = (irq - agent_dev_t->pdev->irq) % DEVDRV_DMA_CHAN_NUM;

    agentdrv_iocmd_raise_int_to_h(agent_dev_t->p_agentdrv_msg_dev[0], agent_dev_t->dma_host_err_irq[ch_id]);
    return IRQ_HANDLED;
}

irqreturn_t devdrv_all_dma_err_interrupt(int irq, void *data)
{
    struct agentdrv_devctrl *agent_dev = (struct agentdrv_devctrl *)data;
    struct devdrv_dma_channel *dma_chan = NULL;
    u32 func_id = U32_MAX;
    u32 chan_id, side;
    int ret;

    ret = devdrv_get_dma_err_chan(agent_dev->dma_chan_base, &chan_id, &side);
    if (ret != 0) {
        return IRQ_HANDLED;
    }

    dma_chan = agentdrv_get_dma_chan_by_id(agent_dev, chan_id, &func_id);
    if (side == DEVDRV_DMA_LOCAL_SIDE) {
        if (dma_chan == NULL) {
            devdrv_err("No match local dma channel. (agent_id=%u; chan_id=%u)\n", agent_dev->agent_id, chan_id);
            return IRQ_HANDLED;
        }
        (void)schedule_work(&dma_chan->err_work);
    } else if (side == DEVDRV_DMA_REMOTE_SIDE) {
        if (func_id == U32_MAX) {
            devdrv_err("No match remote dma channel. (agent_id=%u; chan_id=%u)\n", agent_dev->agent_id, chan_id);
            return IRQ_HANDLED;
        }
        agentdrv_check_dma_urca_status(agent_dev, chan_id);  /* only check channels those sq/cq are remote side */
        agentdrv_iocmd_raise_int_to_h(agent_dev->p_agentdrv_msg_dev[func_id], agent_dev->dma_host_err_irq[chan_id]);
    } else {
        return IRQ_HANDLED;
    }

    return IRQ_HANDLED;
}

int devdrv_register_irq_func(void *drvdata, int vector_index, irqreturn_t (*callback_func)(int, void *), void *para,
                             const char *name)
{
    int vector = vector_index;
    void *data = drvdata;
    if (data != NULL) {
        (void)data;
    }
    return request_irq(vector, callback_func, 0, name, para);
}

int devdrv_unregister_irq_func(void *drvdata, int vector_index, void *para)
{
    int vector = vector_index;
    void *data = drvdata;
    if (data != NULL) {
        (void)data;
    }

    (void)free_irq(vector, para);
    return 0;
}

int devdrv_respond_msg_to_remote_side(u32 dev_id, u64 src, u64 dst, const void *data, u32 size)
{
    enum devdrv_dma_data_type data_type = DEVDRV_DMA_DATA_PCIE_MSG;
    struct devdrv_dma_dev *dma_dev = NULL;

    (void)src;
    dma_dev = devdrv_get_dma_dev(dev_id);
    if (dma_dev == NULL) {
        devdrv_err("Call devdrv_get_dma_dev failed, dma_dev is NULL. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }
    return devdrv_dma_copy_sml_pkt(dma_dev, data_type, dst, data, size);
}

int agentdrv_sync_msg_send(void *msg_chan, void *data, u32 in_data_len, u32 out_data_len, u32 *real_out_len)
{
    int ret;
    struct agentdrv_msg_chan *chan = (struct agentdrv_msg_chan *)msg_chan;

    if ((chan == NULL) || (chan->msg_dev == NULL) || (chan->msg_dev->dev == NULL) || (chan->cq.base_d == NULL) ||
        (chan->cq.base_reserve_d == NULL)) {
        devdrv_err("Input parameter is null.\n");
        return -EINVAL;
    }

    if (data == NULL) {
        devdrv_err("Input parameter data is null. (dev_id=%d; func_id=%d; chan_id=%u)\n",
            chan->msg_dev->dev_id, chan->msg_dev->func_id, chan->chan_id);
        return -EINVAL;
    }

    if (real_out_len == NULL) {
        devdrv_err("Input parameter real_out_len is null. (dev_id=%d; func_id=%d; chan_id=%u)\n",
            chan->msg_dev->dev_id, chan->msg_dev->func_id, chan->chan_id);
        return -EINVAL;
    }
    ret = agentdrv_sync_non_trans_msg_send(chan, data, in_data_len, out_data_len, real_out_len,
                                           AGENTDRV_COMMON_MSG_TYPE_MAX);
    if (ret != 0) {
        devdrv_err("Function agentdrv_sync_non_trans_msg_send failed. (dev_id=%d; func_id=%d; chan_id=%u; ret=%d)\n",
            chan->msg_dev->dev_id, chan->msg_dev->func_id, chan->chan_id, ret);
    }

    return ret;
}
EXPORT_SYMBOL(agentdrv_sync_msg_send);

STATIC void agentdrv_msg_alloc_debug_info(int dev_id, const struct devdrv_create_queue_command *cmd_data)
{
    devdrv_info("Get msg_alloc information. (dev_id=%d; msg_type=%d; queue_type=%d; queue_id=%d;"
                "sq_size=%d; cq_size=%d; sq_depth=%d; cq_depth=%d; tx_irq=%d; rx_irq=%d)\n",
                dev_id, cmd_data->msg_type, cmd_data->queue_type, cmd_data->queue_id,
                cmd_data->sq_desc_size, cmd_data->cq_desc_size, cmd_data->sq_depth,
                cmd_data->cq_depth, cmd_data->irq_tx_finish_notify, cmd_data->irq_rx_msg_notify);
}

STATIC int agentdrv_trans_msg_chan_init(struct agentdrv_msg_chan *msg_chan, u32 msg_type)
{
    struct agentdrv_trans_msg_client *trans_client = NULL;
    int ret;

    /* call client init function */
    trans_client = agentdrv_find_client_by_type(msg_type);
    if (trans_client == NULL) {
        devdrv_err("Search client by type failed. (dev_id=%d)\n", msg_chan->msg_dev->dev_id);
        return -EINVAL;
    }

    if (trans_client->init_trans_msg_chan != NULL) {
        ret = trans_client->init_trans_msg_chan(msg_chan);
        if (ret != 0) {
            devdrv_err("Call init_trans_msg_chan failed. (dev_id=%d; ret=%d)\n", msg_chan->msg_dev->dev_id, ret);
            return ret;
        }
    }

    msg_chan->rx_trans_msg_notify = trans_client->rx_trans_msg_notify;
    msg_chan->sq.db_fun = agentdrv_trans_rx_msg_notify;
    msg_chan->tx_trans_finish_notify = trans_client->tx_trans_finish_notify;
    msg_chan->cq.db_fun = agentdrv_trans_tx_msg_finish;
    msg_chan->rx_work_flag = 0;

    return 0;
}

STATIC int agentdrv_non_trans_msg_chan_init(struct agentdrv_msg_chan *msg_chan, u32 msg_type)
{
    struct agentdrv_non_trans_msg_client *non_trans_client = NULL;
    int ret;

    /* call non_trans client init function */
    non_trans_client = agentdrv_find_non_trans_msg_client_by_type(msg_type);
    if (non_trans_client == NULL) {
        devdrv_err("Find non trans message client failed. (dev_id=%d)\n", msg_chan->msg_dev->dev_id);
        return -EINVAL;
    }

    if (non_trans_client->init_non_trans_msg_chan != NULL) {
        ret = non_trans_client->init_non_trans_msg_chan((void *)msg_chan);
        if (ret != 0) {
            devdrv_err("Init non trans message chan failed. (dev_id=%d; ret=%d)\n", msg_chan->msg_dev->dev_id, ret);
            return ret;
        }
    }

    msg_chan->sq.db_fun = agentdrv_non_trans_wakeup_sq_db_work;
    msg_chan->rx_msg_process = non_trans_client->non_trans_msg_process;
    INIT_WORK(&msg_chan->rx_work, agentdrv_non_trans_rx_msg_task);
    msg_chan->cq.db_fun = NULL;
    msg_chan->rx_work_flag = 1;

    return 0;
}

STATIC int agentdrv_msg_chan_init(struct agentdrv_msg_chan *msg_chan, struct devdrv_create_queue_command *cmd_data)
{
    int ret;

    /* init base queue info */
    msg_chan->msg_type = (enum agentdrv_msg_client_type)cmd_data->msg_type;
    msg_chan->queue_type = (enum msg_queue_type)cmd_data->queue_type;
    msg_chan->chan_id = cmd_data->queue_id;
    msg_chan->sq.base_h = cmd_data->sq_dma_base_host;
    msg_chan->cq.base_h = cmd_data->cq_dma_base_host;

    msg_chan->irq_rx_msg_notify = cmd_data->irq_rx_msg_notify;
    msg_chan->irq_tx_finish_notify = cmd_data->irq_tx_finish_notify;

    /* alloc sq cq memory space */
    ret = agentdrv_msg_alloc_sqcq(msg_chan, cmd_data);
    if (ret != 0) {
        devdrv_err("Function agentdrv_msg_alloc_sqcq failed. (dev_id=%d; ret=%d)\n", msg_chan->msg_dev->dev_id, ret);
        return ret;
    }

    /* if sq enable, register interrupt for sq doorbell */
    if (cmd_data->queue_type == TRANSPARENT_MSG_QUEUE) {
        ret = agentdrv_trans_msg_chan_init(msg_chan, cmd_data->msg_type);
        if (ret != 0) {
            devdrv_err("Init trans message channel failed. (dev_id=%d; ret=%d)\n", msg_chan->msg_dev->dev_id, ret);
            (void)agentdrv_msg_free_msg_queue_res(msg_chan);
            return ret;
        }
    } else {
        ret = agentdrv_non_trans_msg_chan_init(msg_chan, cmd_data->msg_type);
        if (ret != 0) {
            devdrv_err("Init non trans msg_chan failed. (dev_id=%d; ret=%d)\n", msg_chan->msg_dev->dev_id, ret);
            (void)agentdrv_msg_free_msg_queue_res(msg_chan);
            return ret;
        }
    }

    msg_chan->status = AGENTDRV_ENABLE;

    return 0;
}

int agentdrv_msg_alloc_msg_queue(struct agentdrv_msg_dev *msg_dev, void *data)
{
    struct devdrv_create_queue_command *cmd_data = (struct devdrv_create_queue_command *)data;
    struct devdrv_admin_msg_reply *reply = (struct devdrv_admin_msg_reply *)data;
    struct agentdrv_msg_chan *msg_chan = NULL;
    int ret;
    int dev_id = msg_dev->dev_id;

    if (cmd_data->queue_id >= msg_dev->io_chan_cnt) {
        devdrv_err("queue_id is invalid. (dev_id=%d; queue_id=%d)\n", dev_id, cmd_data->queue_id);
        agentdrv_msg_alloc_debug_info(dev_id, cmd_data);
        return -EINVAL;
    }
    msg_chan = &msg_dev->io_chan[cmd_data->queue_id];
    if (msg_chan->status == AGENTDRV_ENABLE) {
        devdrv_err("Status is invalid. (dev_id=%d; msg_chan=%d)\n", dev_id, cmd_data->queue_id);
        agentdrv_msg_alloc_debug_info(dev_id, cmd_data);
        return -EINVAL;
    }

    ret = agentdrv_msg_chan_init(msg_chan, cmd_data);
    if (ret != 0) {
        devdrv_err("Function agentdrv_msg_chan_init failed. (dev_id=%d; ret=%d)\n", dev_id, ret);
        agentdrv_msg_alloc_debug_info(dev_id, cmd_data);
        return ret;
    }

    /* no response message */
    reply->len = sizeof(struct devdrv_admin_msg_reply);

    return 0;
}

int agentdrv_msg_free_msg_queue(struct agentdrv_msg_dev *msg_dev, void *data)
{
    struct devdrv_free_queue_cmd *cmd_data = (struct devdrv_free_queue_cmd *)data;
    struct devdrv_admin_msg_reply *reply = (struct devdrv_admin_msg_reply *)data;
    struct agentdrv_non_trans_msg_client *non_trans_client = NULL;
    struct agentdrv_trans_msg_client *trans_client = NULL;
    struct agentdrv_msg_chan *msg_chan = NULL;
    u32 queue_id;

    queue_id = cmd_data->queue_id;
    if (queue_id >= msg_dev->io_chan_cnt) {
        devdrv_err("Varibale queue_id is invalid. (queue_id=%u)\n", queue_id);
        return -EINVAL;
    }
    msg_chan = &msg_dev->io_chan[queue_id];

    if (msg_chan->status == AGENTDRV_DISABLE) {
        devdrv_info("Msg chan has allready been free. (dev_id=%d; func_id=%d; chan_id=%u; queue_id=%u)\n",
            msg_dev->dev_id, msg_dev->func_id, msg_chan->chan_id, queue_id);
        return 0;
    }

    if (msg_chan->queue_type == TRANSPARENT_MSG_QUEUE) {
        trans_client = agentdrv_find_client_by_type(msg_chan->msg_type);
        if (trans_client == NULL) {
            devdrv_err("Search client by type failed. (dev_id=%d; func_id=%d; chan_id=%u)\n",
                       msg_dev->dev_id, msg_dev->func_id, msg_chan->chan_id);
            return -EINVAL;
        }

        if (trans_client->uninit_trans_msg_chan != NULL) {
            (void)trans_client->uninit_trans_msg_chan(msg_chan);
        }
    } else {
        non_trans_client = agentdrv_find_non_trans_msg_client_by_type(msg_chan->msg_type);
        if (non_trans_client == NULL) {
            devdrv_err("Search client by type failed. (dev_id=%d; func_id=%d; chan_id=%u)\n",
                       msg_dev->dev_id, msg_dev->func_id, msg_chan->chan_id);
            return -EINVAL;
        }

        if (non_trans_client->uninit_non_trans_msg_chan != NULL) {
            non_trans_client->uninit_non_trans_msg_chan((void *)msg_chan);
        }
    }

    (void)agentdrv_msg_free_msg_queue_res(msg_chan);

    /* no response message */
    reply->len = sizeof(struct devdrv_admin_msg_reply);

    return 0;
}

STATIC void agentdrv_sriov_set_dma_idle(struct agentdrv_devctrl *agent_dev)
{
    agent_dev->dma_idle_bitmap |= (HOST_VF_DMA_MASK | DEV_VF_DMA_MASK);
}

STATIC void agentdrv_sriov_notify_enable(struct agentdrv_devctrl *agent_dev)
{
    u32 i;
    u32 func_idx = 0;
    struct agentdrv_cpu_data drv_cpu_info = {0};

    devdrv_sriov_pf_dma_traffic(agent_dev->p_agentdrv_dma_dev[0]);
    for (i = 1; i < agent_dev->p_agentdrv_dma_dev[0]->local_chan_num; i++) {
        devdrv_set_dma_chan_status(&agent_dev->p_agentdrv_dma_dev[0]->dma_chan[i], DEVDRV_DMA_CHAN_DISABLED);
    }

    for (func_idx = 0; func_idx < agent_dev->func_totl_num; func_idx++) {
        agentdrv_dma_unbind_irq(agent_dev, func_idx);
    }

    devdrv_dma_exit(agent_dev->p_agentdrv_dma_dev[0], DEVDRV_SRIOV_ENABLE);
    agentdrv_sriov_set_dma_idle(agent_dev);
    agentdrv_get_cpu_data_info(agent_dev, 0, &drv_cpu_info);
    agentdrv_dma_bind_irq(agent_dev, 0, &drv_cpu_info);

    for (func_idx = 1; func_idx < agent_dev->func_totl_num; func_idx++) {
        agent_dev->shr_para[func_idx]->dev_num = (int)agent_dev->func_totl_num;
        agent_dev->valid[func_idx] = AGENTDRV_ENABLE;
    }
}

STATIC void agentdrv_sriov_notify_disable(struct agentdrv_devctrl *agent_dev)
{
    u32 func_idx = 0;
    struct agentdrv_cpu_data drv_cpu_info;

    for (func_idx = 0; func_idx < agent_dev->func_totl_num; func_idx++) {
        agentdrv_dma_unbind_irq(agent_dev, func_idx);
    }

    for (func_idx = 1; func_idx < agent_dev->func_totl_num; func_idx++) {
        agent_dev->p_agentdrv_msg_dev[func_idx]->io_chan[AGENTDRV_ADMIN_CHAN_ID].sq.base_h = 0;
        agent_dev->valid[func_idx] = AGENTDRV_DISABLE;
    }

    (void)devdrv_sriov_dma_init_pf_chan(agent_dev->p_agentdrv_dma_dev[0]);
    agentdrv_get_cpu_data_info(agent_dev, 0, &drv_cpu_info);
    agentdrv_dma_bind_irq(agent_dev, 0, &drv_cpu_info);

    devdrv_res_dma_traffic(agent_dev->p_agentdrv_dma_dev[0]);
}

int agentdrv_sriov_event_notify(struct agentdrv_msg_dev *msg_dev, void *data)
{
    struct devdrv_sriov_event_notify_cmd *cmd_data = (struct devdrv_sriov_event_notify_cmd *)data;
    struct devdrv_admin_msg_reply *reply = (struct devdrv_admin_msg_reply *)data;
    struct agentdrv_devctrl *agent_dev = NULL;

    agent_dev = agentdrv_get_dev(msg_dev->dev_id);
    if ((agent_dev == NULL) || (agent_dev->shr_para[msg_dev->func_id] == NULL)) {
        devdrv_err("Input parameter is error.\n");
        return -EINVAL;
    }

    if (agent_dev->shr_para[msg_dev->func_id]->host_dev_id != cmd_data->devid) {
        devdrv_err("Input parameter is invalid.\n");
        return -EINVAL;
    }

    if (cmd_data->status == DEVDRV_SRIOV_ENABLE) {
        agentdrv_sriov_notify_enable(agent_dev);
    } else {
        agentdrv_sriov_notify_disable(agent_dev);
    }

    /* no response message */
    reply->len = (u32)sizeof(struct devdrv_admin_msg_reply);

    return 0;
}

#ifdef CFG_SOC_PLATFORM_MDC_V51
void agentdrv_set_msg_workqueue_affinity(struct workqueue_struct *wq)
{
    struct cpumask wq_cpumask;
    int ret;

#ifdef CFG_FEATURE_SUPPORT_CPUDOMAIN
    ret = drv_get_ctrlcpu_mask(&wq_cpumask);
#else
    ret = drv_get_ctrlcpu_mask_from_cpuset(&wq_cpumask);
#endif
    if (ret != 0) {
        devdrv_err("get ctrl cpu mask failed.(ret=%d)\n", ret);
        return;
    }

    if (set_workqueue_affinity(wq, 0, &wq_cpumask) != 0) {
        devdrv_err("bind workqueue dms_timer_common failed\n");
    }
}
#endif

int agentdrv_msg_dev_init(struct agentdrv_msg_dev *msg_dev)
{
    char *db_base = NULL;
    int ret;
    u32 i;

    if (msg_dev == NULL) {
        devdrv_err("Input parameter is error.\n");
        return -EINVAL;
    }
#ifdef CFG_SOC_PLATFORM_MDC_V51
    msg_dev->work_queue = alloc_workqueue("pcie_msg_workqueue", WQ_UNBOUND, 1);
#else
    msg_dev->work_queue = create_workqueue("pcie_msg_workqueue");
#endif
    if (msg_dev->work_queue == NULL) {
        devdrv_err("Create admin_chan_work_queue failed. (dev_id=%d)\n", msg_dev->dev_id);
        return -EINVAL;
    }

#ifdef CFG_SOC_PLATFORM_MDC_V51
    agentdrv_set_msg_workqueue_affinity(msg_dev->work_queue);
#endif

    db_base = (char *)msg_dev->doorbell_base + AGENTDRV_MSG_CHAN_USED_DB_START;

    for (i = 0; i < msg_dev->io_chan_cnt; i++) {
        msg_dev->io_chan[i].msg_dev = msg_dev;
        msg_dev->io_chan[i].sq.db_base = (void *)(db_base + (u64)i * AGENTDRV_DB_STRDE);
        msg_dev->io_chan[i].cq.db_base = (void *)(db_base + ((u64)i * AGENTDRV_DB_STRDE + AGENTDRV_SQ_CQ_QUEUE_OFFSET));
        mutex_init(&msg_dev->io_chan[i].mutex);
    }

    msg_dev->dev_status = DEVDRV_DEV_ONLINE;
    ret = agentdrv_msg_dev_admin_chan_init(msg_dev);
    if (ret != 0) {
        destroy_workqueue(msg_dev->work_queue);
        devdrv_err("Call admin_chan_init failed. (dev_id=%d; ret=%d)\n", msg_dev->dev_id, ret);
        return ret;
    }

#ifdef CFG_FEATURE_P2P
    agentdrv_p2p_msg_init(msg_dev);
#endif
    return 0;
}

void agentdrv_msg_dev_exit(struct agentdrv_msg_dev *msg_dev)
{
    struct agentdrv_msg_chan *chan = NULL;
    u32 i;

#ifdef CFG_FEATURE_P2P
    agentdrv_p2p_msg_exit(msg_dev);
#endif

    destroy_workqueue(msg_dev->work_queue);

    for (i = 0; i < msg_dev->io_chan_cnt; i++) {
        chan = &msg_dev->io_chan[i];
        if (chan->status == AGENTDRV_ENABLE)
            (void)agentdrv_msg_free_msg_queue_res(chan);
    }
}
