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
 * Create: 2022-08-13
 */

#define pr_fmt(fmt) "XSMEM_VMA: <%s:%d> " fmt, __func__, __LINE__

#include <linux/types.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/rbtree_augmented.h>
#include <linux/version.h>

#include "xsmem_framework.h"
#include "xsmem_algo_vma.h"

#define XSMEM_VMA_ALIGN PAGE_SIZE

struct vma_ctrl {
    unsigned long       total_size;
    unsigned long       free_size;
    unsigned long       alloc_block_cnt;

    struct rb_root      free_block_root;
    struct mutex mutex;
};

struct vma_block {
    struct rb_node      block_rb_node;

    unsigned long       start;
    unsigned long       size;
    unsigned long       subtree_max_size;
};

static inline struct vma_block *to_vma_block(struct rb_node *node)
{
    return rb_entry(node, struct vma_block, block_rb_node);
}

static unsigned long subtree_max_size(struct rb_node *node)
{
    return node ? to_vma_block(node)->subtree_max_size : 0;
}

static unsigned long compute_subtree_max_size(struct vma_block *block)
{
    return max3(block->size, subtree_max_size(block->block_rb_node.rb_left),
        subtree_max_size(block->block_rb_node.rb_right));
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)
static int update_subtree_max_size(struct vma_block *block, bool compare_flag)
{
    unsigned long subtree_max_size;

    subtree_max_size = compute_subtree_max_size(block);
    if ((compare_flag == true) && (block->subtree_max_size == subtree_max_size)) {
        return -EPERM;
    }
    block->subtree_max_size = subtree_max_size;

    return 0;
}
RB_DECLARE_CALLBACKS(static, vma_block_subtree_max_size_cb, struct vma_block, block_rb_node,
                     subtree_max_size, update_subtree_max_size);
#else
RB_DECLARE_CALLBACKS(static, vma_block_subtree_max_size_cb, struct vma_block, block_rb_node,
                     unsigned long, subtree_max_size, compute_subtree_max_size);
#endif

static void block_update_subtree_max_size(struct vma_block *block)
{
    vma_block_subtree_max_size_cb_propagate(&block->block_rb_node, NULL);
}

void *vma_inst_create(unsigned long pool_size)
{
    struct vma_ctrl *ctrl = NULL;
    struct vma_block *block = NULL;

    if ((IS_ALIGNED(pool_size, XSMEM_VMA_ALIGN) == 0) || (pool_size == 0)) {
        pr_err("Input xsm_pool size invalid. (pool_size=%lx)\n", pool_size);
        return NULL;
    }

    ctrl = kmalloc(sizeof(*ctrl), GFP_KERNEL | __GFP_ACCOUNT);
    if (unlikely(ctrl == NULL)) {
        return NULL;
    }

    block = kmalloc(sizeof(*block), GFP_KERNEL | __GFP_ACCOUNT);
    if (unlikely(block == NULL)) {
        kfree(ctrl);
        return NULL;
    }

    ctrl->total_size = pool_size;
    ctrl->free_size = pool_size;
    ctrl->alloc_block_cnt = 0;
    ctrl->free_block_root.rb_node = NULL;
    mutex_init(&ctrl->mutex);

    block->start = 0;
    block->size = pool_size;
    block->subtree_max_size = pool_size;
    rb_link_node(&block->block_rb_node, NULL, &ctrl->free_block_root.rb_node);
    rb_insert_color(&block->block_rb_node, &ctrl->free_block_root);
    return (void *)ctrl;
}

static int vma_algo_pool_init(struct xsm_pool *xp, struct xsm_reg_arg *arg)
{
    void *ctrl = NULL;

    ctrl = vma_inst_create(arg->pool_size);
    if (ctrl == NULL) {
        return -EINVAL;
    }

    xp->private = ctrl;
    return 0;
}

#ifdef DEBUG_XSMEM_VMA
static void block_dump(struct vma_block *block)
{
    pr_info("block: start: %#lx, size: %#lx, subtree_max_size: %#lx\n",
        block->start, block->size, block->subtree_max_size);
}

static void pool_dump(struct vma_ctrl *ctrl)
{
    struct vma_block *block, *tmp;

    rbtree_postorder_for_each_entry_safe(block, tmp, &ctrl->free_block_root, block_rb_node)
        block_dump(block);
}
#else
static void pool_dump(struct vma_ctrl *ctrl) {}
#endif

int vma_inst_destroy(void *vma_ctrl)
{
    struct vma_ctrl *ctrl = (struct vma_ctrl *)vma_ctrl;
    struct vma_block *block = to_vma_block(ctrl->free_block_root.rb_node);

    mutex_destroy(&ctrl->mutex);
    WARN(ctrl->total_size != ctrl->free_size, "not all block freed\n");
    WARN(ctrl->total_size != block->size, "not all block freed\n");
    WARN(ctrl->total_size != block->subtree_max_size, "not all block freed\n");

    pool_dump(ctrl);

    kfree(block);
    kfree(ctrl);
    return 0;
}

static int vma_algo_pool_free(struct xsm_pool *xp)
{
    vma_inst_destroy(xp->private);
    xp->private = NULL;
    return 0;
}

static struct vma_block *find_first_proper_block(struct rb_root *root, unsigned long size)
{
    struct rb_node *node = root->rb_node;

    while (node) {
        struct vma_block *block = to_vma_block(node);

        if (subtree_max_size(block->block_rb_node.rb_left) >= size) {
            node = block->block_rb_node.rb_left;
            continue;
        }

        if (block->size >= size) {
            return block;
        }

        if (subtree_max_size(block->block_rb_node.rb_right) >= size) {
            node = block->block_rb_node.rb_right;
        } else {
            return NULL;
        }
    }

    return NULL;
}

static int vma_alloc_block(struct rb_root *root, unsigned long size, unsigned long *start)
{
    struct vma_block *block = NULL;

    block = find_first_proper_block(root, size);
    if (block == NULL) {
        return -ENOSPC;
    }

    *start = block->start;
    if (block->size == size) {
        rb_erase_augmented(&block->block_rb_node, root, &vma_block_subtree_max_size_cb);
        kfree(block);
    } else {
        block->start += size;
        block->size -= size;
        block_update_subtree_max_size(block);
    }

    return 0;
}

int vma_algo_alloc(void *vma_ctrl, unsigned long alloc_size,
    unsigned long *addr, unsigned long *real_size)
{
    struct vma_ctrl *ctrl = (struct vma_ctrl *)vma_ctrl;
    unsigned long real_size_tmp;
    unsigned long offset;
    int ret;

    if (alloc_size == 0) {
        pr_err("alloc size %lx invalid\n", alloc_size);
        return -EINVAL;
    }

    real_size_tmp = ALIGN(alloc_size, XSMEM_VMA_ALIGN);
    if (real_size_tmp < alloc_size) {
        return -EOVERFLOW;
    }

    if (real_size_tmp > ctrl->free_size) {
        return -ENOSPC;
    }

    mutex_lock(&ctrl->mutex);
    ret = vma_alloc_block(&ctrl->free_block_root, real_size_tmp, &offset);
    if (ret == 0) {
        ctrl->free_size -= real_size_tmp;
        ctrl->alloc_block_cnt++;
        *real_size = real_size_tmp;
        *addr = offset;
    }
    mutex_unlock(&ctrl->mutex);
    return ret;
}

static int vma_algo_block_alloc(struct xsm_pool *xp, struct xsm_block *blk)
{
    return vma_algo_alloc(xp->private, blk->alloc_size, &blk->offset, &blk->real_size);
}

static struct vma_block *prev_block(struct rb_node *parent, struct rb_node **link)
{
    if (parent == NULL) {
        return NULL;
    }

    if (link == &parent->rb_right) {
        return to_vma_block(parent);
    } else {
        /* the prev node should always exist */
        return to_vma_block(rb_prev(parent));
    }
}

static struct vma_block *next_block(struct rb_node *parent, struct rb_node **link)
{
    if (parent == NULL) {
        return NULL;
    }

    if (link == &parent->rb_left) {
        return to_vma_block(parent);
    } else {
        /* the next node should always exist */
        return to_vma_block(rb_next(parent));
    }
}

/* two intervals overlapped ? [l1, h1), [l2, h2) */
#define is_overlap(l1, h1, l2, h2) (((l1) < (h2)) && ((l2) < (h1)))

static int vma_free_block(struct rb_root *root, unsigned long start, unsigned long size)
{
    bool merged = false;
    struct vma_block *block = NULL, *prev = NULL;
    struct rb_node *parent = NULL;
    struct rb_node **link = &root->rb_node;

    while (*link) {
        parent = *link;
        block = to_vma_block(*link);
        if (start < block->start) {
            link = &parent->rb_left;
        } else if (start > block->start) {
            link = &parent->rb_right;
        } else {
            pr_err("double free a block\n");
            return -EFAULT;
        }
    }

    prev = prev_block(parent, link);
    if (prev) {
        if (prev->start + prev->size == start) {
            prev->size += size;
            block_update_subtree_max_size(prev);
            merged = true;
        } else if (is_overlap(start, start + size, prev->start, prev->start + prev->size)) {
            pr_err("prev-node overlap detected\n");
            return -EINVAL;
        }
    }

    block = next_block(parent, link);
    if (block) {
        if (start + size == block->start) {
            if (merged) {
                rb_erase_augmented(&block->block_rb_node, root, &vma_block_subtree_max_size_cb);
                prev->size += block->size;
                kfree(block);
                block = prev;
            } else {
                block->start -= size;
                block->size += size;
            }

            block_update_subtree_max_size(block);
            merged = true;
        } else if (is_overlap(start, start + size, block->start, block->start + block->size)) {
            pr_err("next-node overlap detected\n");
            return -EINVAL;
        }
    }

    if (!merged) {
        block = kmalloc(sizeof(*block), GFP_KERNEL | __GFP_ACCOUNT);
        if (unlikely(block == NULL)) {
            pr_err("alloc new free block memroy failed\n");
            return -ENOMEM;
        }

        block->start = start;
        block->size = size;
        block->subtree_max_size = size;

        rb_link_node(&block->block_rb_node, parent, link);
        rb_insert_augmented(&block->block_rb_node, root, &vma_block_subtree_max_size_cb);
    }

    return 0;
}

int vma_algo_free(void *vma_ctrl, unsigned long addr, unsigned long real_size)
{
    struct vma_ctrl *ctrl = (struct vma_ctrl *)vma_ctrl;
    int ret;

    mutex_lock(&ctrl->mutex);
    ret = vma_free_block(&ctrl->free_block_root, addr, real_size);
    ctrl->alloc_block_cnt--;
    ctrl->free_size += real_size;
    mutex_unlock(&ctrl->mutex);
    return ret;
}

static int vma_algo_block_free(struct xsm_pool *xp, struct xsm_block *blk)
{
    return vma_algo_free(xp->private, blk->offset, blk->real_size);
}

void vma_algo_show(void *vma_ctrl, struct seq_file *seq)
{
    struct vma_ctrl *ctrl = (struct vma_ctrl *)vma_ctrl;
    struct vma_block *block = NULL;
    struct rb_node *node = NULL;
    int vma_free_block_cnt = 0;

    seq_printf(seq, "    total_size=%lu, free_size=%lu, alloc_block_cnt=%ld\n",
            ctrl->total_size, ctrl->free_size, ctrl->alloc_block_cnt);

    mutex_lock(&ctrl->mutex);
    node = rb_first(&ctrl->free_block_root);
    while (node != NULL) {
        vma_free_block_cnt++;
        block = to_vma_block(node);
        seq_printf(seq, "        free_block_%d:size=%lu\n", vma_free_block_cnt, block->size);
        node = rb_next(node);
    }
    mutex_unlock(&ctrl->mutex);
}

static void vma_algo_pool_show(struct xsm_pool *xp, struct seq_file *seq)
{
    vma_algo_show(xp->private, seq);
}

static struct xsm_pool_algo vma_algo = {
    .num = XSMEM_ALGO_VMA,
    .name = "vma_algo",
    .xsm_pool_init = vma_algo_pool_init,
    .xsm_pool_free = vma_algo_pool_free,
    .xsm_pool_show = vma_algo_pool_show,
    .xsm_block_alloc = vma_algo_block_alloc,
    .xsm_block_free = vma_algo_block_free,
};

struct xsm_pool_algo *xsm_get_vma_algo(void)
{
    return &vma_algo;
}