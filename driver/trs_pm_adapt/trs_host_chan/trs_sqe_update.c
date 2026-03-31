/*
* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
* Create: 2023-8-7
*/
#include <linux/rbtree.h>
#include <linux/vmalloc.h>
#include <linux/kallsyms.h>

#include "devdrv_interface.h"

#include "kernel_version_adapt.h"
#include "trs_pub_def.h"
#include "trs_chan_update.h"
#include "trs_chan.h"

#include "trs_sqe_update.h"

struct trs_rb_info {
    struct rb_root root;
    u32 rb_cnt;
    spinlock_t spinlock;
};

struct trs_rb_info trs_dma_desc_rb[TRS_DEV_MAX_NUM];

struct trs_dma_desc_node {
    struct rb_node desc_node;
    u64 rb_handle;

    /* rb key */
    u32 sqid;
    u32 sqeid;

    struct devdrv_dma_prepare *dma_prepare;
};

typedef u64 (*rb_handle_func)(struct rb_node *node);

static int trs_rb_insert(struct rb_root *root, struct rb_node *node, rb_handle_func get_handle)
{
    struct rb_node **cur_node = &root->rb_node;
    struct rb_node *parent = NULL;
    u64 handle = get_handle(node);

    /* Figure out where to put new node */
    while (*cur_node) {
        u64 tmp_handle = get_handle(*cur_node);

        parent = *cur_node;
        if (handle < tmp_handle) {
            cur_node = &((*cur_node)->rb_left);
        } else if (handle > tmp_handle) {
            cur_node = &((*cur_node)->rb_right);
        } else {
            trs_err("Insert same priv.\n");
            return -EINVAL;
        }
    }

    /* Add new node and rebalance tree. */
    rb_link_node(node, parent, cur_node);
    rb_insert_color(node, root);
    return 0;
}

static struct rb_node *trs_rb_search(struct rb_root *root, u64 handle, rb_handle_func get_handle)
{
    struct rb_node *node = NULL;

    node = root->rb_node;
    while (node != NULL) {
        u64 tmp_handle = get_handle(node);
        if (handle < tmp_handle) {
            node = node->rb_left;
        } else if (handle > tmp_handle) {
            node = node->rb_right;
        } else {
            return node;
        }
    }

    return NULL;
}

static int trs_rb_erase(struct rb_root *root, struct rb_node *node)
{
    int ret = -ENODEV;

    if (RB_EMPTY_NODE(node) == false) {
        rb_erase(node, root);
        RB_CLEAR_NODE(node);
        ret = 0;
    }
    return ret;
}

static u64 rb_handle_of_desc_node(struct rb_node *node)
{
    struct trs_dma_desc_node *tmp = rb_entry(node, struct trs_dma_desc_node, desc_node);
    return tmp->rb_handle;
}

static struct trs_dma_desc_node *trs_dma_desc_node_find(u32 devid, u64 rb_handle)
{
    struct trs_rb_info *rb_info = &trs_dma_desc_rb[devid];
    struct trs_dma_desc_node *dma_desc_node = NULL;
    struct rb_node *node = NULL;

    spin_lock_bh(&rb_info->spinlock);
    node = trs_rb_search(&rb_info->root, rb_handle, rb_handle_of_desc_node);
    spin_unlock_bh(&rb_info->spinlock);
    if (node != NULL) {
        dma_desc_node = rb_entry(node, struct trs_dma_desc_node, desc_node);
    }

    return dma_desc_node;
}

static int trs_dma_desc_node_create(u32 devid, u32 sqid, u32 sqeid, struct devdrv_dma_prepare *dma_prepare)
{
    struct trs_rb_info *rb_info = &trs_dma_desc_rb[devid];
    struct trs_dma_desc_node *node = NULL;
    int ret;

    node = vmalloc(sizeof(struct trs_dma_desc_node));
    if (node == NULL) {
        trs_err("Vzalloc failed.\n");
        return -ENOMEM;
    }

    node->sqid = sqid;
    node->sqeid = sqeid;
    node->dma_prepare = dma_prepare;
    node->rb_handle = (((u64)sqid << 32) | (u64)sqeid); /* low 32 bits is sqeid */

    RB_CLEAR_NODE(&node->desc_node);

    spin_lock_bh(&rb_info->spinlock);
    ret = trs_rb_insert(&rb_info->root, &node->desc_node, rb_handle_of_desc_node);
    rb_info->rb_cnt++;
    spin_unlock_bh(&rb_info->spinlock);
    if (ret != 0) {
        vfree(node);
    }
    return ret;
}

static int trs_dma_node_pack(u32 devid, u32 tsid, struct trs_dma_desc_addr_info *addr_info,
    struct devdrv_dma_node *dma_node)
{
    struct trs_id_inst inst = {.devid = devid, .tsid = tsid};
    struct trs_chan_sq_info sq_info;
    int ret;

    ret = trs_get_res_info(&inst, TRS_HW_SQ, addr_info->sqid, (void *)&sq_info);
    if (ret != 0) {
        trs_err("Get res addr failed. (devid=%u; tsid=%u; ret=%d)\n", devid, tsid, ret);
        return ret;
    }

    if (addr_info->sqeid >= sq_info.sq_para.sq_depth) {
        trs_err("Addr info invalid. (devid=%u; tsid=%u; sqeid=%u; depth=%u)\n",
            devid, tsid, addr_info->sqeid, sq_info.sq_para.sq_depth);
        return -EINVAL;
    }

    dma_node->src_addr = addr_info->src_va;

    dma_node->loc_passid = addr_info->passid;

    dma_node->dst_addr = sq_info.sq_phy_addr + addr_info->sqeid * sq_info.sq_para.sqe_size + addr_info->offset;

    dma_node->direction = DEVDRV_DMA_DEVICE_TO_HOST;
    dma_node->size = addr_info->size;

    return 0;
}

#define TRS_SQE_UPDATE_LIMITED (2048 * 512)
static bool trs_is_rb_node_limited(u32 devid)
{
    struct trs_rb_info *rb_info = &trs_dma_desc_rb[devid];

    spin_lock_bh(&rb_info->spinlock);
    if (rb_info->rb_cnt >= TRS_SQE_UPDATE_LIMITED) {
        spin_unlock_bh(&rb_info->spinlock);
        trs_warn("Dma_desc is limited. (devid=%u; cnt=%u)\n", devid, rb_info->rb_cnt);
        return true;
    }
    spin_unlock_bh(&rb_info->spinlock);

    return false;
}

/* sqid sqeid --key */
int hal_kernel_sqe_update_desc_create(u32 devid, u32 tsid, struct trs_dma_desc_addr_info *addr_info,
    struct trs_dma_desc *dma_desc)
{
    struct devdrv_dma_prepare *dma_prepare = NULL;
    struct trs_dma_desc_node *dma_desc_node = NULL;
    struct devdrv_dma_node dma_node;
    u64 rb_handle = (((u64)addr_info->sqid << 32) | (u64)addr_info->sqeid);
    u32 dma_node_num = 1;
    int ret;

    dma_desc_node = trs_dma_desc_node_find(devid, rb_handle);
    if (dma_desc_node != NULL) {
        dma_desc->sq_addr = (void *)(uintptr_t)dma_desc_node->dma_prepare->sq_dma_addr;
        dma_desc->sq_tail = dma_node_num;
        return 0;
    }

    if (trs_is_rb_node_limited(devid)) {
        return -EMFILE;
    }

    ret = trs_dma_node_pack(devid, tsid, addr_info, &dma_node);
    if (ret != 0) {
        return ret;
    }

    dma_prepare = devdrv_dma_link_prepare(devid, DEVDRV_DMA_DATA_TRAFFIC, &dma_node, dma_node_num,
        DEVDRV_DMA_DESC_FILL_FINISH);
    if (dma_prepare == NULL) {
        trs_err("Dma_link_prepare alloc failed. (devid=%u)\n", devid);
        return -ENOMEM;
    }

    dma_desc->sq_addr = (void *)(uintptr_t)dma_prepare->sq_dma_addr;
    dma_desc->sq_tail = dma_node_num;

    ret = trs_dma_desc_node_create(devid, addr_info->sqid, addr_info->sqeid, dma_prepare);
    if (ret != 0) {
        (void)devdrv_dma_link_free(dma_prepare);
    }

    trs_debug("Dma desc create. (devid=%u; sqid=%u; sqeid=%u; ssid=%u; offset=%u; size=%u)\n",
        devid, addr_info->sqid, addr_info->sqeid, addr_info->passid, addr_info->offset, addr_info->size);

    return ret;
}
EXPORT_SYMBOL(hal_kernel_sqe_update_desc_create);

static struct trs_dma_desc_node *trs_dma_desc_node_del_one_by_sqid(u32 devid, u32 sqid)
{
    struct trs_rb_info *rb_info = &trs_dma_desc_rb[devid];
    struct trs_dma_desc_node *pos = NULL;
    struct trs_dma_desc_node *tmp = NULL;

    spin_lock_bh(&rb_info->spinlock);
    rbtree_postorder_for_each_entry_safe(pos, tmp, &rb_info->root, desc_node) {
        if (pos->sqid == sqid) {
            (void)trs_rb_erase(&rb_info->root, &pos->desc_node);
            rb_info->rb_cnt--;
            spin_unlock_bh(&rb_info->spinlock);
            return pos;
        }
    }
    spin_unlock_bh(&rb_info->spinlock);

    return NULL;
}

void hal_kernel_sqe_update_desc_destroy(u32 devid, u32 tsid, u32 sqid)
{
    struct trs_dma_desc_node *node = NULL;
    u32 num = 0;

    while (1) {
        node = trs_dma_desc_node_del_one_by_sqid(devid, sqid);
        if (node == NULL) {
            break;
        }

        trs_debug("Dma desc destroy. (devid=%u; sqid=%u; sqeid=%u)\n", devid, node->sqid, node->sqeid);
        (void)devdrv_dma_link_free(node->dma_prepare);
        vfree(node);

        cond_resched();
        num++;
    }

    trs_debug("Destroy dma_desc info. (num=%u)\n", num);
}
EXPORT_SYMBOL(hal_kernel_sqe_update_desc_destroy);

void trs_sqe_update_init(u32 devid)
{
    struct trs_rb_info *rb_info = &trs_dma_desc_rb[devid];

    rb_info->root = RB_ROOT;
    rb_info->rb_cnt = 0;
    spin_lock_init(&rb_info->spinlock);
}

