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
* Create: 2022-8-15
*/
#include <linux/mutex.h>
#include <linux/completion.h>
#include <linux/string.h>
#include <linux/io.h>
#include <linux/kref.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/ktime.h>
#include <linux/list.h>
#include <linux/delay.h>

#include "trs_pub_def.h"
#include "trs_mailbox_def.h"
#include "trs_chan_update.h"
#include "trs_timestamp.h"
#include "trs_mbox.h"

static DEFINE_MUTEX(trs_mbox_mutex);

struct trs_mbox_chan {
    struct trs_id_inst inst;

    void __iomem *base;
    size_t size;
    u64 tx_time_us;
    u64 txdone_time_us;
    u64 wakeup_time_us;

    struct completion tx_complete;
    struct mutex mutex;

    struct kref ref;

    void *priv;
    struct trs_mbox_ops ops;

    u64 tx_cnt;
    u64 irq_cnt;
};

static struct trs_mbox_chan *g_mbox_chan[TRS_DEV_MAX_NUM];

static struct trs_mbox_chan *trs_mbox_chan_create(struct trs_id_inst *inst, struct trs_mbox_chan_attr *attr)
{
    struct trs_mbox_chan *chan = kzalloc(sizeof(struct trs_mbox_chan), GFP_KERNEL);

    if (chan == NULL) {
        trs_err("Alloc mbox chan fail\n");
        return NULL;
    }
    chan->base = ioremap(attr->base, attr->size);
    if (chan->base == NULL) {
        kfree(chan);
        trs_err("Ioremap chan base fail. (size=0x%lx)\n", attr->size);
        return NULL;
    }
    memset_io(chan->base, 0, chan->size);
    chan->size = attr->size;
    chan->priv = attr->priv;
    chan->ops = attr->ops;
    chan->inst = *inst;
    chan->tx_cnt = 0;
    chan->irq_cnt = 0;
    kref_init(&chan->ref);

    init_completion(&chan->tx_complete);
    mutex_init(&chan->mutex);

    return chan;
}

static void trs_mbox_chan_destroy(struct trs_mbox_chan *chan)
{
    if (chan != NULL) {
        if (chan->base != NULL) {
            iounmap(chan->base);
            chan->base = NULL;
        }
        mutex_destroy(&chan->mutex);
        kfree(chan);
    }
}

static void trs_mbox_chan_release(struct kref *kref)
{
    struct trs_mbox_chan *chan = container_of(kref, struct trs_mbox_chan, ref);

    if (chan->ops.mbox_release != NULL) {
        chan->ops.mbox_release(chan->priv);
    }
    trs_mbox_chan_destroy(chan);
}

static int trs_mbox_chan_add(struct trs_mbox_chan *chan)
{
    u32 ts_inst = trs_id_inst_to_ts_inst(&chan->inst);

    mutex_lock(&trs_mbox_mutex);
    if (g_mbox_chan[ts_inst] != NULL) {
        mutex_unlock(&trs_mbox_mutex);
        return -ENODEV;
    }
    g_mbox_chan[ts_inst] = chan;
    mutex_unlock(&trs_mbox_mutex);

    return 0;
}

static void trs_mbox_chan_del(struct trs_id_inst *inst)
{
    u32 ts_inst = trs_id_inst_to_ts_inst(inst);
    struct trs_mbox_chan *chan = NULL;

    mutex_lock(&trs_mbox_mutex);
    chan = g_mbox_chan[ts_inst];
    g_mbox_chan[ts_inst] = NULL;
    mutex_unlock(&trs_mbox_mutex);

    if (chan != NULL) {
        chan->ops.free_irq(chan->priv);
        kref_put(&chan->ref, trs_mbox_chan_release);
    }
}

static struct trs_mbox_chan *trs_mbox_chan_get(struct trs_id_inst *inst)
{
    u32 ts_inst = trs_id_inst_to_ts_inst(inst);
    struct trs_mbox_chan *chan = NULL;

    mutex_lock(&trs_mbox_mutex);
    chan = g_mbox_chan[ts_inst];
    if (chan != NULL) {
        kref_get(&chan->ref);
    }
    mutex_unlock(&trs_mbox_mutex);

    return chan;
}

static void trs_mbox_chan_put(struct trs_mbox_chan *chan)
{
    kref_put(&chan->ref, trs_mbox_chan_release);
}

void *trs_mbox_chan_init(struct trs_id_inst *inst, struct trs_mbox_chan_attr *attr)
{
    struct trs_mbox_chan *chan = NULL;
    int ret;

    chan = trs_mbox_chan_create(inst, attr);
    if (chan == NULL) {
        return NULL;
    }

    ret = trs_mbox_chan_add(chan);
    if (ret != 0) {
        trs_mbox_chan_destroy(chan);
        return NULL;
    }

    return chan;
}

void trs_mbox_chan_uninit(struct trs_id_inst *inst)
{
    trs_mbox_chan_del(inst);
}

static void trs_mbox_chan_write(struct trs_mbox_chan *chan, void *data, size_t size)
{
    memcpy_toio(chan->base, data, size);
    wmb();
}

static void trs_mbox_chan_get_result(struct trs_mbox_chan *chan, void *data)
{
    struct trs_mb_header *mbox_msg_header = (struct trs_mb_header *)chan->base;
    struct trs_mb_header *header = (struct trs_mb_header *)data;
    header->result = mbox_msg_header->result;
    rmb();
    if ((header->result != 0) && (header->cmd_type != TRS_MBOX_RECYCLE_CHECK)) {
        trs_warn("Pay attention to result. (result=%u; tx_time_us=%llu; txdone_time_us=%llu; wakeup_time_us=%llu)\n",
            header->result, chan->tx_time_us, chan->txdone_time_us, chan->wakeup_time_us);
    }
}

static u16 trs_mbox_chan_get_valid(struct trs_mbox_chan *chan)
{
    struct trs_mb_header *mbox_msg_header = (struct trs_mb_header *)chan->base;
    return mbox_msg_header->valid;
}

static int trs_mbox_chan_wait(struct trs_mbox_chan *chan, int timeout)
{
    if (timeout <= 0) {
        wait_for_completion(&chan->tx_complete);
    } else {
        int ret = wait_for_completion_timeout(&chan->tx_complete, msecs_to_jiffies(timeout));
        if (ret == 0) {
            return -ETIMEDOUT;
        }
    }
    return 0;
}

void trs_mbox_chan_txdone(void *mbox_chan)
{
    struct trs_mbox_chan *chan = (struct trs_mbox_chan *)mbox_chan;

    if (chan != NULL) {
        chan->irq_cnt++;
        chan->txdone_time_us = trs_get_us_timestamp();
        complete(&chan->tx_complete);
    }
}

static int trs_mbox_chan_trigger_irq(struct trs_mbox_chan *chan)
{
    int ret = -ENODEV;

    if (chan->ops.trigger_irq != NULL) {
        ret = chan->ops.trigger_irq(chan->priv);
    }
    return ret;
}

static int trs_mbox_chan_send(struct trs_mbox_chan *chan, void *data, size_t size, int timeout)
{
    int ret;

    if (size > chan->size) {
        trs_err("Send size invalid. (size=0x%lx; chan->size=0x%lx)\n", size, chan->size);
        return -EFAULT;
    }
    reinit_completion(&chan->tx_complete);
    trs_mbox_chan_write(chan, data, size);
    ret = trs_mbox_chan_trigger_irq(chan);
    if (ret != 0) {
        trs_err("Trs mbox trigger irq fail. (ret=%d)\n", ret);
        return ret;
    }
    chan->tx_time_us = trs_get_us_timestamp();
    ret = trs_mbox_chan_wait(chan, timeout);
    chan->wakeup_time_us = trs_get_us_timestamp();
    if (ret != 0) {
        trs_err("Mbox wait fail. (ret=%d; valid=0x%x; tx_time_us=%llu; txdone_time_us=%llu; wakeup_time_us=%llu)\n",
            ret, trs_mbox_chan_get_valid(chan), chan->tx_time_us, chan->txdone_time_us, chan->wakeup_time_us);
        return ret;
    }
    chan->tx_cnt++;
    trs_mbox_chan_get_result(chan, data);

    return 0;
}

int trs_mbox_send(struct trs_id_inst *inst, u32 chan_id, void *data, size_t size, int timeout)
{
    struct trs_mbox_chan *chan = NULL;
    int ret;

    if ((trs_id_inst_check(inst) != 0) || (data == NULL)) {
        return -EINVAL;
    }

    ret = trs_mb_update(inst, (int)current->tgid, data, (u32)size);
    if (ret != 0) {
        return ret;
    }

    ret = -ENXIO;
    chan = trs_mbox_chan_get(inst);
    if (chan != NULL) {
        mutex_lock(&chan->mutex);
        ret = trs_mbox_chan_send(chan, data, size, timeout);
        mutex_unlock(&chan->mutex);
        trs_mbox_chan_put(chan);
    }
    return ret;
}
EXPORT_SYMBOL(trs_mbox_send);

int trs_mbox_get_chan_num(struct trs_id_inst *inst)
{
    /* Only one channel number for each tscpu */
    return 1;
}
EXPORT_SYMBOL(trs_mbox_get_chan_num);

