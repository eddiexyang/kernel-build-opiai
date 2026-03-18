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

#include "agentdrv_msg_interface.h"
#include "devdrv_util.h"
#include "agentdrv_msg.h"
#include "agentdrv_unit.h"

int agentdrv_get_remote_rx_msg_notify_irq(void *msg_chan)
{
    struct agentdrv_msg_chan *chan = (struct agentdrv_msg_chan *)msg_chan;
    int offset = agentdrv_get_msix_offset(chan);

    return (offset < 0 ? offset : (chan->irq_rx_msg_notify + offset));
}
EXPORT_SYMBOL(agentdrv_get_remote_rx_msg_notify_irq);

int agentdrv_get_remote_tx_finish_notify_irq(void *msg_chan)
{
    struct agentdrv_msg_chan *chan = (struct agentdrv_msg_chan *)msg_chan;
    int offset = agentdrv_get_msix_offset(chan);

    return (offset < 0 ? offset : (chan->irq_tx_finish_notify + offset));
}
EXPORT_SYMBOL(agentdrv_get_remote_tx_finish_notify_irq);

/* devid */
int agentdrv_get_msg_chan_devid(void *msg_chan)
{
    int dev_id = 0;
    struct agentdrv_msg_chan *chan = (struct agentdrv_msg_chan *)msg_chan;
    if ((chan == NULL) || (chan->msg_dev == NULL)) {
        devdrv_err("Input parameter is invalid.\n");
        return -EINVAL;
    }

    devdrv_chipfunc2dev(&dev_id, chan->msg_dev->dev_id, chan->msg_dev->func_id);
    return dev_id;
}
EXPORT_SYMBOL(agentdrv_get_msg_chan_devid);

/* priv */
int agentdrv_set_msg_chan_priv(void *msg_chan, void *priv)
{
    struct agentdrv_msg_chan *chan = (struct agentdrv_msg_chan *)msg_chan;
    if (chan == NULL) {
        devdrv_err("Input parameter is invalid.\n");
        return -EINVAL;
    }
    chan->priv = priv;
    return 0;
}
EXPORT_SYMBOL(agentdrv_set_msg_chan_priv);

void *agentdrv_get_msg_chan_priv(void *msg_chan)
{
    struct agentdrv_msg_chan *chan = (struct agentdrv_msg_chan *)msg_chan;
    if (chan == NULL) {
        devdrv_err("Input parameter is invalid.\n");
        return NULL;
    }
    return chan->priv;
}
EXPORT_SYMBOL(agentdrv_get_msg_chan_priv);

/* local sq */
void agentdrv_set_msg_chan_local_sq_head(void *msg_chan, u32 head)
{
    struct agentdrv_msg_chan *chan = (struct agentdrv_msg_chan *)msg_chan;
    if (chan == NULL) {
        devdrv_err_spinlock("Input parameter is invalid.\n");
        return;
    }
    chan->sq.head = head;
}
EXPORT_SYMBOL(agentdrv_set_msg_chan_local_sq_head);

void *agentdrv_get_msg_chan_local_sq(void *msg_chan, u32 ptr)
{
    struct agentdrv_msg_chan *chan = (struct agentdrv_msg_chan *)msg_chan;
    if (chan == NULL) {
        devdrv_err("Input parameter is invalid.\n");
        return NULL;
    }
    return (void *)((char *)chan->sq.base_d + (u64)ptr * chan->sq.desc_size);
}

void *agentdrv_get_msg_chan_local_sq_tail(void *msg_chan, u32 *tail)
{
    struct agentdrv_msg_chan *chan = (struct agentdrv_msg_chan *)msg_chan;
    if ((chan == NULL) || (tail == NULL)) {
        devdrv_err_spinlock("Input parameter is invalid.\n");
        return NULL;
    }
    *tail = chan->sq.tail;
    return (void *)((char *)chan->sq.base_d + (u64)chan->sq.tail * chan->sq.desc_size);
}
EXPORT_SYMBOL(agentdrv_get_msg_chan_local_sq_tail);

dma_addr_t agentdrv_get_msg_chan_local_sq_tail_dma_addr(void *msg_chan)
{
    struct agentdrv_msg_chan *chan = (struct agentdrv_msg_chan *)msg_chan;
    if (chan == NULL) {
        devdrv_err_spinlock("Input parameter is invalid.\n");
        return 0;
    }
    return chan->sq.dma_base_d + (dma_addr_t)chan->sq.tail * (dma_addr_t)chan->sq.desc_size;
}
EXPORT_SYMBOL(agentdrv_get_msg_chan_local_sq_tail_dma_addr);

void agentdrv_move_msg_chan_local_sq_tail(void *msg_chan)
{
    struct agentdrv_msg_chan *chan = (struct agentdrv_msg_chan *)msg_chan;
    if (chan == NULL) {
        devdrv_err_spinlock("Input parameter is invalid.\n");
        return;
    }
    if (chan->sq.depth != 0) {
        chan->sq.tail = (chan->sq.tail + 1) % chan->sq.depth;
    }
}
EXPORT_SYMBOL(agentdrv_move_msg_chan_local_sq_tail);

bool agentdrv_msg_chan_local_sq_full_check(void *msg_chan)
{
    struct agentdrv_msg_chan *chan = (struct agentdrv_msg_chan *)msg_chan;
    if (chan == NULL) {
        devdrv_err_spinlock("Input parameter is invalid.\n");
        return true;
    }
    if ((chan->sq.depth != 0) && (((chan->sq.tail + 1) % chan->sq.depth) == chan->sq.head)) {
        return true;
    } else {
        return false;
    }
}
EXPORT_SYMBOL(agentdrv_msg_chan_local_sq_full_check);

/* host sq */
dma_addr_t agentdrv_get_msg_chan_host_sq_tail_dma_addr(void *msg_chan)
{
    struct agentdrv_msg_chan *chan = (struct agentdrv_msg_chan *)msg_chan;
    if (chan == NULL) {
        devdrv_err_spinlock("Input parameter is invalid.\n");
        return 0;
    }
    return chan->sq.base_h + (dma_addr_t)chan->sq.tail * (dma_addr_t)chan->sq.desc_size;
}
EXPORT_SYMBOL(agentdrv_get_msg_chan_host_sq_tail_dma_addr);

/* local cq */
void *agentdrv_get_msg_chan_local_cq(void *msg_chan, u32 ptr)
{
    struct agentdrv_msg_chan *chan = (struct agentdrv_msg_chan *)msg_chan;
    if (chan == NULL) {
        devdrv_err("Input parameter is invalid.\n");
        return NULL;
    }
    return (void *)((char *)chan->cq.base_d + (u64)ptr * chan->cq.desc_size);
}

void *agentdrv_get_msg_chan_local_cq_tail(void *msg_chan)
{
    struct agentdrv_msg_chan *chan = (struct agentdrv_msg_chan *)msg_chan;
    if (chan == NULL) {
        devdrv_err("Input parameter is invalid.\n");
        return NULL;
    }
    return (void *)((char *)chan->cq.base_d + (u64)chan->cq.tail * chan->cq.desc_size);
}
EXPORT_SYMBOL(agentdrv_get_msg_chan_local_cq_tail);

dma_addr_t agentdrv_get_msg_chan_local_cq_tail_dma_addr(void *msg_chan)
{
    struct agentdrv_msg_chan *chan = (struct agentdrv_msg_chan *)msg_chan;
    if (chan == NULL) {
        devdrv_err("Input parameter is invalid.\n");
        return 0;
    }
    return chan->cq.dma_base_d + (dma_addr_t)chan->cq.tail * (dma_addr_t)chan->cq.desc_size;
}
EXPORT_SYMBOL(agentdrv_get_msg_chan_local_cq_tail_dma_addr);

void agentdrv_move_msg_chan_local_cq_tail(void *msg_chan)
{
    struct agentdrv_msg_chan *chan = (struct agentdrv_msg_chan *)msg_chan;
    if (chan == NULL) {
        devdrv_err("Input parameter is invalid.\n");
        return;
    }
    if (chan->cq.depth != 0) {
        chan->cq.tail = (chan->cq.tail + 1) % chan->cq.depth;
    }
}
EXPORT_SYMBOL(agentdrv_move_msg_chan_local_cq_tail);

/* host cq */
dma_addr_t agentdrv_get_msg_chan_host_cq_tail_dma_addr(void *msg_chan)
{
    struct agentdrv_msg_chan *chan = (struct agentdrv_msg_chan *)msg_chan;
    if (chan == NULL) {
        devdrv_err("Input parameter is invalid.\n");
        return 0;
    }
    return chan->cq.base_h + (dma_addr_t)chan->cq.tail * (dma_addr_t)chan->cq.desc_size;
}
EXPORT_SYMBOL(agentdrv_get_msg_chan_host_cq_tail_dma_addr);

/* reserve sq */
void *agentdrv_get_msg_chan_reserve_sq_head(void *msg_chan, u32 *head)
{
    struct agentdrv_msg_chan *chan = (struct agentdrv_msg_chan *)msg_chan;
    if ((chan == NULL) || (head == NULL)) {
        devdrv_err("Input parameter is invalid.\n");
        return NULL;
    }
    *head = chan->sq.head_res;
    return (void *)((char *)chan->sq.base_reserve_d + (u64)chan->sq.head_res * chan->sq.desc_size);
}
EXPORT_SYMBOL(agentdrv_get_msg_chan_reserve_sq_head);

void agentdrv_move_msg_chan_reserve_sq_head(void *msg_chan)
{
    struct agentdrv_msg_chan *chan = (struct agentdrv_msg_chan *)msg_chan;
    if (chan == NULL) {
        devdrv_err("Input parameter is invalid.\n");
        return;
    }
    if (chan->sq.depth != 0) {
        chan->sq.head_res = (chan->sq.head_res + 1) % chan->sq.depth;
    }
}
EXPORT_SYMBOL(agentdrv_move_msg_chan_reserve_sq_head);

dma_addr_t agentdrv_get_msg_chan_reserve_sq_head_dma_addr(void *msg_chan)
{
    struct agentdrv_msg_chan *chan = (struct agentdrv_msg_chan *)msg_chan;
    if (chan == NULL) {
        devdrv_err("Input parameter is invalid.\n");
        return 0;
    }
    return chan->sq.dma_reserve_d + (dma_addr_t)chan->sq.head_res * (dma_addr_t)chan->sq.desc_size;
}

/* reserve cq */
void *agentdrv_get_msg_chan_reserve_cq_head(void *msg_chan)
{
    struct agentdrv_msg_chan *chan = (struct agentdrv_msg_chan *)msg_chan;
    if (chan == NULL) {
        devdrv_err_spinlock("Input parameter is invalid.\n");
        return NULL;
    }
    return (void *)((char *)chan->cq.base_reserve_d + (u64)chan->cq.head_res * chan->cq.desc_size);
}
EXPORT_SYMBOL(agentdrv_get_msg_chan_reserve_cq_head);

void agentdrv_move_msg_chan_reserve_cq_head(void *msg_chan)
{
    struct agentdrv_msg_chan *chan = (struct agentdrv_msg_chan *)msg_chan;
    if (chan == NULL) {
        devdrv_err("Input parameter is invalid.\n");
        return;
    }
    if (chan->cq.depth != 0) {
        chan->cq.head_res = (chan->cq.head_res + 1) % chan->cq.depth;
    }
}
EXPORT_SYMBOL(agentdrv_move_msg_chan_reserve_cq_head);
