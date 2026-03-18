/*
* Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
* Create: 2022-7-15
*/
#include <linux/types.h>
#include <linux/kref.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>

#include "trs_pub_def.h"
#include "trs_id_conv.h"

#define TRS_ID_CONV_MAX_PHY_ID  0xFFFFU

static DEFINE_SPINLOCK(id_conv_lock);

struct id_conv_node {
    u32 phy_id;
    bool init;
};

struct id_conv_head {
    u32 ts_inst;
    int type;
    u32 max_id;

    struct spinlock lock;
    struct kref ref;
    struct id_conv_node *node;
};

static struct id_conv_head *g_id_conv_head[TRS_TS_INST_MAX_NUM][TRS_ID_TYPE_MAX];
#ifndef EMU_ST
static int id_conv_param_check(struct trs_id_inst *inst, int type)
{
    if (trs_id_inst_check(inst) != 0) {
        return -EINVAL;
    }

    if ((type < TRS_STREAM_ID) || (type >= TRS_ID_TYPE_MAX)) {
        return -EINVAL;
    }

    return 0;
}

static struct id_conv_head *id_conv_head_create(u32 ts_inst, int type, u32 max_id)
{
    struct id_conv_head *conv_head = kzalloc(sizeof(struct id_conv_head), GFP_KERNEL | __GFP_ACCOUNT);
    struct id_conv_node *conv_node = NULL;
    u32 i;

    if (conv_head == NULL) {
        return NULL;
    }
    conv_node = kzalloc(sizeof(struct id_conv_node) * max_id, GFP_KERNEL | __GFP_ACCOUNT);
    if (conv_node == NULL) {
        kfree(conv_head);
        return NULL;
    }
    for (i = 0; i < max_id; i++) {
        conv_node[i].init = false;
        conv_node[i].phy_id = TRS_ID_CONV_MAX_PHY_ID;
    }
    conv_head->ts_inst = ts_inst;
    conv_head->type = type;
    conv_head->node = conv_node;
    conv_head->max_id = max_id;
    spin_lock_init(&conv_head->lock);
    kref_init(&conv_head->ref);

    return conv_head;
}

void id_conv_head_destroy(struct id_conv_head *conv)
{
    if (conv != NULL) {
        if (conv->node != NULL) {
            kfree(conv->node);
            conv->node = NULL;
        }
        kfree(conv);
    }
}

static void id_conv_head_release(struct kref *kref)
{
    struct id_conv_head *conv_head = container_of(kref, struct id_conv_head, ref);

    id_conv_head_destroy(conv_head);
}

static int id_conv_head_add(struct id_conv_head *conv_head)
{
    spin_lock_bh(&id_conv_lock);
    if (g_id_conv_head[conv_head->ts_inst][conv_head->type] != NULL) {
        spin_unlock_bh(&id_conv_lock);
        return -ENODEV;
    }
    g_id_conv_head[conv_head->ts_inst][conv_head->type] = conv_head;
    spin_unlock_bh(&id_conv_lock);
    return 0;
}

static void id_conv_head_del(u32 ts_inst, int type)
{
    struct id_conv_head *conv_head = NULL;

    spin_lock_bh(&id_conv_lock);
    conv_head = g_id_conv_head[ts_inst][type];
    g_id_conv_head[ts_inst][type] = NULL;
    spin_unlock_bh(&id_conv_lock);

    if (conv_head != NULL) {
        kref_put(&conv_head->ref, id_conv_head_release);
    }
}

static struct id_conv_head *id_conv_head_get(u32 ts_inst, int type)
{
    struct id_conv_head *conv_head = NULL;

    spin_lock_bh(&id_conv_lock);
    conv_head = g_id_conv_head[ts_inst][type];
    if (conv_head != NULL) {
        if (kref_get_unless_zero(&conv_head->ref) == 0) {
            spin_unlock_bh(&id_conv_lock);
            return NULL;
        }
    }
    spin_unlock_bh(&id_conv_lock);
    return conv_head;
}

static void id_conv_head_put(struct id_conv_head *conv_head)
{
    kref_put(&conv_head->ref, id_conv_head_release);
}

int id_conv_init(struct trs_id_inst *inst, int type, u32 max_id)
{
    struct id_conv_head *conv_head = NULL;
    u32 ts_inst;
    int ret;

    ret = id_conv_param_check(inst, type);
    if (ret != 0) {
        return ret;
    }

    ts_inst = trs_id_inst_to_ts_inst(inst);
    conv_head = id_conv_head_create(ts_inst, type, max_id);
    if (conv_head == NULL) {
        return -ENODEV;
    }

    ret = id_conv_head_add(conv_head);
    if (ret != 0) {
        id_conv_head_destroy(conv_head);
    }
    return ret;
}

void id_conv_exit(struct trs_id_inst *inst, int type)
{
    u32 ts_inst;
    int ret;

    ret = id_conv_param_check(inst, type);
    if (ret != 0) {
        return;
    }
    ts_inst = trs_id_inst_to_ts_inst(inst);
    id_conv_head_del(ts_inst, type);
}

static int _id_conv_add(struct id_conv_head *conv_head, u32 id, u32 phy_id)
{
    if (id >= conv_head->max_id) {
        return -EFAULT;
    }
    if (conv_head->node[id].init == true) {
        if (conv_head->node[id].phy_id == phy_id) {
            return 0;
        }
        return -ENODEV;
    }
    conv_head->node[id].init = true;
    conv_head->node[id].phy_id = phy_id;
    return 0;
}

int id_conv_add(struct trs_id_inst *inst, int type, u32 id, u32 phy_id)
{
    struct id_conv_head *conv_head = NULL;
    u32 ts_inst;
    int ret;

    ret = id_conv_param_check(inst, type);
    if (ret != 0) {
        return ret;
    }

    ts_inst = trs_id_inst_to_ts_inst(inst);
    conv_head = id_conv_head_get(ts_inst, type);
    if (conv_head == NULL) {
        return -ENODEV;
    }

    spin_lock_bh(&conv_head->lock);
    ret = _id_conv_add(conv_head, id, phy_id);
    spin_unlock_bh(&conv_head->lock);

    id_conv_head_put(conv_head);
    return ret;
}

static void _id_conv_del(struct id_conv_head *conv_head, u32 id)
{
    if (id < conv_head->max_id) {
        if (conv_head->node[id].init == true) {
            conv_head->node[id].init = false;
            conv_head->node[id].phy_id = TRS_ID_CONV_MAX_PHY_ID;
        }
    }
}

void id_conv_del(struct trs_id_inst *inst, int type, u32 id)
{
    struct id_conv_head *conv_head = NULL;
    u32 ts_inst;
    int ret;

    ret = id_conv_param_check(inst, type);
    if (ret != 0) {
        return;
    }

    ts_inst = trs_id_inst_to_ts_inst(inst);
    conv_head = id_conv_head_get(ts_inst, type);
    if (conv_head == NULL) {
        return;
    }

    spin_lock_bh(&conv_head->lock);
    _id_conv_del(conv_head, id);
    spin_unlock_bh(&conv_head->lock);

    id_conv_head_put(conv_head);
}

static int _id_conv_v2p(struct id_conv_head *conv_head, u32 id, u32 *phy_id)
{
    if ((id >= conv_head->max_id) || (conv_head->node[id].init != true)) {
        return -EACCES;
    }
    *phy_id = conv_head->node[id].phy_id;
    return 0;
}

int id_conv_v2p(struct trs_id_inst *inst, int type, u32 id, u32 *phy_id)
{
    struct id_conv_head *conv_head = NULL;
    u32 ts_inst;
    int ret;

    ret = id_conv_param_check(inst, type);
    if (ret != 0) {
        return ret;
    }

    if (phy_id == NULL) {
        return -EINVAL;
    }

    ts_inst = trs_id_inst_to_ts_inst(inst);
    conv_head = id_conv_head_get(ts_inst, type);
    if (conv_head == NULL) {
        return -ENODEV;
    }
    spin_lock_bh(&conv_head->lock);
    ret = _id_conv_v2p(conv_head, id, phy_id);
    spin_unlock_bh(&conv_head->lock);

    id_conv_head_put(conv_head);
    return ret;
}
#else
void trs_id_conv_stub_test(void)
{
}
#endif

