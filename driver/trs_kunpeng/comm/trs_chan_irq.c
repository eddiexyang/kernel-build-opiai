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
* Create: 2022-10-15
*/
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/radix-tree.h>
#include <linux/interrupt.h>
#include <linux/vmalloc.h>

#include "soc_res.h"
#include "trs_chan_irq.h"

static RADIX_TREE(irq_tree, GFP_KERNEL);
DEFINE_MUTEX(irq_mutex);

#define MAX_PROC_CQ_NUM 1024

struct trs_chan_irq {
    struct trs_chan_irq_attr attr;
    int irq_index;
    u32 irq_type;
    u32 irq;
    int ref;
    struct trs_id_inst inst;
    spinlock_t lock;
    struct list_head head;
    u32 cqid_list[MAX_PROC_CQ_NUM];
    struct tasklet_struct task;
};

struct trs_chan_irq_node {
    struct list_head node;
    void (*handler)(int irq_type, int irq_index, void *para, u32 cqid[], u32 cq_num);
    void *para;
};

int trs_chan_get_irq_by_index(struct trs_id_inst *inst, int irq_type, int irq_index, u32 *irq)
{
    struct res_inst_info res_inst;
    int ret;

    soc_resmng_inst_pack(&res_inst, inst->devid, TS_SUBSYS, inst->tsid);

    ret = soc_resmng_get_irq_by_index(&res_inst, irq_type, irq_index, irq);
    if (ret != 0) {
        trs_err("Get failed. (devid=%u; tsid=%u; irq_type=%u; id=%u)\n", inst->devid, inst->tsid, irq_type, irq_index);
        return ret;
    }

    return 0;
}

int trs_chan_get_irq(struct trs_id_inst *inst, u32 irq_type, u32 irq[], u32 irq_num, u32 *valid_irq_num)
{
    struct res_inst_info res_inst;
    int ret;
    u32 i;

    soc_resmng_inst_pack(&res_inst, inst->devid, TS_SUBSYS, inst->tsid);
    ret = soc_resmng_get_irq_num(&res_inst, irq_type, valid_irq_num);
    if (ret != 0) {
        return ret;
    }

    *valid_irq_num = (*valid_irq_num > irq_num) ? irq_num : *valid_irq_num;
    for (i = 0; i < *valid_irq_num; i++) {
        ret = trs_chan_get_irq_by_index(inst, irq_type, i, &irq[i]);
        if (ret != 0) {
            return ret;
        }
    }

    return 0;
}
EXPORT_SYMBOL(trs_chan_get_irq);

static void trs_chan_irq_tasklet(unsigned long data)
{
    struct trs_chan_irq *chan_irq = (struct trs_chan_irq *)data;
    struct trs_chan_irq_node *irq_node = NULL;
    u32 cq_num = 0;
    u32 *cqid_list = NULL;

    if (chan_irq->attr.get_valid_cq != NULL) {
        int ret = chan_irq->attr.get_valid_cq(&chan_irq->inst, chan_irq->attr.group,
            chan_irq->cqid_list, MAX_PROC_CQ_NUM, &cq_num);
        if (ret == 0) {
            cqid_list = chan_irq->cqid_list;
        }
    }

    spin_lock_bh(&chan_irq->lock);
    list_for_each_entry(irq_node, &chan_irq->head, node) {
        irq_node->handler((int)chan_irq->irq_type, chan_irq->irq_index, irq_node->para, cqid_list, cq_num);
    }
    spin_unlock_bh(&chan_irq->lock);

    if (chan_irq->attr.intr_mask_config != NULL) {
        chan_irq->attr.intr_mask_config(&chan_irq->inst, chan_irq->attr.group, chan_irq->irq, 0);
    }
}

static irqreturn_t trs_adapt_chan_irq_proc(int irq, void *para)
{
    struct trs_chan_irq *chan_irq = (struct trs_chan_irq *)para;

    if (chan_irq->attr.intr_mask_config != NULL) {
        chan_irq->attr.intr_mask_config(&chan_irq->inst, chan_irq->attr.group, irq, 1);
    }
    tasklet_schedule(&chan_irq->task);
    return IRQ_HANDLED;
}

static struct trs_chan_irq *trs_create_chan_irq(struct trs_id_inst *inst, u32 irq, struct trs_chan_irq_attr *attr)
{
    struct trs_chan_irq *chan_irq = vzalloc(sizeof(struct trs_chan_irq));

    if (chan_irq != NULL) {
        int ret;
        chan_irq->attr = *attr;
        chan_irq->inst = *inst;
        spin_lock_init(&chan_irq->lock);
        INIT_LIST_HEAD(&chan_irq->head);
        tasklet_init(&chan_irq->task, trs_chan_irq_tasklet, (uintptr_t)chan_irq);

        ret = request_irq(irq, trs_adapt_chan_irq_proc, 0, attr->name, chan_irq);
        if (ret != 0) {
            vfree(chan_irq);
            trs_err("Request irq failed. (irq=%u)\n", irq);
            return NULL;
        }
#ifdef CFG_FEATURE_IRQ_BIND
        (void)irq_set_affinity_hint(irq, get_cpu_mask(0));
#endif
        trs_debug("Request irq success. (irq=%u)\n", irq);
    }

    return chan_irq;
}

void trs_destroy_chan_irq(struct trs_chan_irq *chan_irq)
{
    trs_info("Free irq success. (irq=%u)\n", chan_irq->irq);
    tasklet_kill(&chan_irq->task);
    (void)irq_set_affinity_hint(chan_irq->irq, NULL);
    (void)free_irq(chan_irq->irq, chan_irq);
    vfree(chan_irq);
}

int trs_chan_request_irq(struct trs_id_inst *inst, int irq_type, int irq_index, struct trs_chan_irq_attr *attr)
{
    struct trs_chan_irq_node *irq_node = NULL;
    struct trs_chan_irq *chan_irq = NULL;
    u32 irq;
    int ret;

    ret = trs_chan_get_irq_by_index(inst, irq_type, irq_index, &irq);
    if (ret != 0) {
        return ret;
    }

    irq_node = kzalloc(sizeof(struct trs_chan_irq_node), GFP_KERNEL);
    if (irq_node == NULL) {
        trs_err("Malloc failed. (size=%lx)\n", sizeof(struct trs_chan_irq_node));
        return -ENOMEM;
    }
    irq_node->handler = attr->handler;
    irq_node->para = attr->para;

    mutex_lock(&irq_mutex);
    chan_irq = radix_tree_lookup(&irq_tree, irq);
    if (chan_irq == NULL) {
        chan_irq = trs_create_chan_irq(inst, irq, attr);
        if (chan_irq == NULL) {
            mutex_unlock(&irq_mutex);
            kfree(irq_node);
            return -ENOMEM;
        }
        radix_tree_insert(&irq_tree, irq, chan_irq);

        chan_irq->irq_type = irq_type;
        chan_irq->irq_index = irq_index;
        chan_irq->irq = irq;
    }
    chan_irq->ref++;

    spin_lock_bh(&chan_irq->lock);
    list_add(&irq_node->node, &chan_irq->head);
    spin_unlock_bh(&chan_irq->lock);

    mutex_unlock(&irq_mutex);

    return 0;
}

int trs_chan_free_irq(struct trs_id_inst *inst, int irq_type, int irq_index, void *para)
{
    struct trs_chan_irq_node *irq_node = NULL;
    struct trs_chan_irq *chan_irq = NULL;
    struct trs_chan_irq_node *tmp = NULL;
    u32 irq;
    int ret;

    ret = trs_chan_get_irq_by_index(inst, irq_type, irq_index, &irq);
    if (ret != 0) {
        return ret;
    }

    mutex_lock(&irq_mutex);
    chan_irq = radix_tree_lookup(&irq_tree, irq);
    if (chan_irq == NULL) {
        mutex_unlock(&irq_mutex);
        trs_err("Find irq fail. (irq=%d)\n", irq);
        return -ENODEV;
    }

    ret = -EINVAL;
    spin_lock_bh(&chan_irq->lock);
    list_for_each_entry_safe(irq_node, tmp, &chan_irq->head, node) {
        if (irq_node->para == para) {
            list_del(&irq_node->node);
            kfree(irq_node);
            chan_irq->ref--;
            ret = 0;
            break;
        }
    }
    spin_unlock_bh(&chan_irq->lock);

    if (chan_irq->ref <= 0) {
        radix_tree_delete(&irq_tree, irq);
        trs_destroy_chan_irq(chan_irq);
    }
    mutex_unlock(&irq_mutex);

    return ret;
}
EXPORT_SYMBOL(trs_chan_free_irq);
