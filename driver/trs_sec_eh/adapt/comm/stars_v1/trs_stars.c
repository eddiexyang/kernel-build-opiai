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
* Create: 2022-7-15
*/
#include <linux/spinlock.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/kref.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/jiffies.h>
#include <linux/rwlock_types.h>

#include "trs_stars_reg_def.h"
#include "trs_pub_def.h"
#include "trs_stars.h"
struct trs_stars {
    struct trs_id_inst inst;

    void __iomem *base;
    phys_addr_t paddr;
    size_t size;

    u32 stride; // stars topic sched stride size
    int type;
    u32 cq_num;
    u32 cq_grp_num;

    trs_stars_set_cq_l1_mask_t set_cq_l1_mask;
    trs_stars_get_valid_cq_list_t get_valid_cq_list;
    struct kref ref;
};

static struct trs_stars *g_trs_stars[TRS_TS_INST_MAX_NUM][TRS_STARS_MAX];
static DEFINE_RWLOCK(trs_stars_lock);
static struct trs_stars *trs_stars_create(struct trs_id_inst *inst, int type, struct trs_stars_attr *attr)
{
    struct trs_stars *stars = kzalloc(sizeof(struct trs_stars), GFP_KERNEL);

    if (stars == NULL) {
        return NULL;
    }
    stars->base = ioremap(attr->paddr, attr->size);
    if (stars->base == NULL) {
        kfree(stars);
        return NULL;
    }
    stars->inst = *inst;
    stars->paddr = attr->paddr;
    stars->size = attr->size;
    stars->stride = attr->stride;
    stars->type = type;
    stars->cq_num = attr->cq_num;
    stars->cq_grp_num = attr->cq_grp_num;
    stars->set_cq_l1_mask = attr->set_cq_l1_mask;
    stars->get_valid_cq_list = attr->get_valid_cq_list;

    kref_init(&stars->ref);
    return stars;
}

static void trs_stars_destroy(struct trs_stars *stars)
{
    if (stars->base != NULL) {
        iounmap(stars->base);
        stars->base = NULL;
    }
    kfree(stars);
}

int trs_stars_test_bit(u32 nr, u32 val)
{
    return val & (1U << nr);
}

static int trs_stars_add(struct trs_stars *stars)
{
    u32 ts_inst = trs_id_inst_to_ts_inst(&stars->inst);
    unsigned long flags;

    write_lock_irqsave(&trs_stars_lock, flags);
    if (g_trs_stars[ts_inst][stars->type] != NULL) {
        write_unlock_irqrestore(&trs_stars_lock, flags);
        return -ENODEV;
    }
    g_trs_stars[ts_inst][stars->type] = stars;
    write_unlock_irqrestore(&trs_stars_lock, flags);
    return 0;
}

static void trs_stars_release(struct kref *kref)
{
    struct trs_stars *stars = container_of(kref, struct trs_stars, ref);

    trs_info("Trs stars uninit. (devid=%u; tsid=%u; type=%d)\n", stars->inst.devid, stars->inst.tsid, stars->type);
    trs_stars_destroy(stars);
}

static void trs_stars_del(struct trs_id_inst *inst, int type)
{
    u32 ts_inst = trs_id_inst_to_ts_inst(inst);
    struct trs_stars *stars = NULL;
    unsigned long flags;

    write_lock_irqsave(&trs_stars_lock, flags);
    stars = g_trs_stars[ts_inst][type];
    g_trs_stars[ts_inst][type] = NULL;
    write_unlock_irqrestore(&trs_stars_lock, flags);

    if (stars != NULL) {
        kref_put(&stars->ref, trs_stars_release);
    }
}

static struct trs_stars *trs_stars_get(struct trs_id_inst *inst, int type)
{
    u32 ts_inst = trs_id_inst_to_ts_inst(inst);
    struct trs_stars *stars = NULL;
    unsigned long flags;

    read_lock_irqsave(&trs_stars_lock, flags);
    stars = g_trs_stars[ts_inst][type];
    if (stars != NULL) {
        kref_get(&stars->ref);
    }
    read_unlock_irqrestore(&trs_stars_lock, flags);
    return stars;
}

static void trs_stars_put(struct trs_stars *stars)
{
    if (stars != NULL) {
        kref_put(&stars->ref, trs_stars_release);
    }
}

static int trs_stars_range_check(struct trs_stars *stars, unsigned long offset, size_t size)
{
    if (unlikely((offset > (unsigned long)stars->size) || (offset + (unsigned long)size) > stars->size)) {
        return -ENOMEM;
    }
    return 0;
}

int trs_stars_get_sq_tail(struct trs_id_inst *inst, u32 sqid, u32 *tail)
{
    struct trs_stars *stars = NULL;
    void __iomem *vaddr = NULL;
    unsigned long offset;
    int ret;

    stars = trs_stars_get(inst, TRS_STARS_SCHED);
    if (stars == NULL) {
        return -ENODEV;
    }

    offset = (unsigned long)sqid * stars->stride + TRS_STARS_SCHED_SQ_TAIL_OFFSET;
    ret = trs_stars_range_check(stars, offset, sizeof(u32));
    if (ret == 0) {
        vaddr = stars->base + offset;
        *tail = readl(vaddr);
    }
    trs_stars_put(stars);
    return ret;
}

int trs_stars_set_sq_tail(struct trs_id_inst *inst, u32 sqid, u32 tail)
{
    struct trs_stars *stars = NULL;
    void __iomem *vaddr = NULL;
    unsigned long offset;
    int ret;

    stars = trs_stars_get(inst, TRS_STARS_SCHED);
    if (stars == NULL) {
        return -ENODEV;
    }

    offset = (unsigned long)sqid * stars->stride + TRS_STARS_SCHED_SQ_TAIL_OFFSET;
    ret = trs_stars_range_check(stars, offset, sizeof(u32));
    if (ret == 0) {
        vaddr = stars->base + offset;
        writel(tail, vaddr);
    }
    trs_stars_put(stars);
    return ret;
}

int trs_stars_get_cq_head(struct trs_id_inst *inst, u32 cqid, u32 *head)
{
    struct trs_stars *stars = NULL;
    void __iomem *vaddr = NULL;
    unsigned long offset;
    int ret;

    stars = trs_stars_get(inst, TRS_STARS_SCHED);
    if (stars == NULL) {
        return -ENODEV;
    }

    offset = (unsigned long)cqid * stars->stride + TRS_STARS_SCHED_CQ_HEAD_OFFSET;
    ret = trs_stars_range_check(stars, offset, sizeof(u32));
    if (ret == 0) {
        vaddr = stars->base + offset;
        *head = readl(vaddr);
    }
    trs_stars_put(stars);
    return ret;
}

int trs_stars_set_cq_head(struct trs_id_inst *inst, u32 cqid, u32 head)
{
    struct trs_stars *stars = NULL;
    void __iomem *vaddr = NULL;
    unsigned long offset;
    int ret;

    stars = trs_stars_get(inst, TRS_STARS_SCHED);
    if (stars == NULL) {
        return -ENODEV;
    }

    offset = (unsigned long)cqid * stars->stride + TRS_STARS_SCHED_CQ_HEAD_OFFSET;
    ret = trs_stars_range_check(stars, offset, sizeof(u32));
    if (ret == 0) {
        vaddr = stars->base + offset;
        writel(head, vaddr);
    }
    trs_stars_put(stars);
    return ret;
}

int trs_stars_get_cq_tail(struct trs_id_inst *inst, u32 cqid, u32 *tail)
{
    struct trs_stars *stars = NULL;
    void __iomem *vaddr = NULL;
    unsigned long offset;
    int ret;

    stars = trs_stars_get(inst, TRS_STARS_SCHED);
    if (stars == NULL) {
        return -ENODEV;
    }

    offset = (unsigned long)cqid * stars->stride + TRS_STARS_SCHED_CQ_TAIL_OFFSET;
    ret = trs_stars_range_check(stars, offset, sizeof(u32));
    if (ret == 0) {
        vaddr = stars->base + offset;
        *tail = readl(vaddr);
    }
    trs_stars_put(stars);
    return ret;
}

static bool _trs_stars_sq_is_enabled(struct trs_stars *stars, u32 sqid)
{
    void __iomem *vaddr = NULL;
    unsigned long offset;
    u32 val;
    int ret;

    offset = (unsigned long)sqid * stars->stride + TRS_STARS_SCHED_SQ_STATUS_OFFSET;
    ret = trs_stars_range_check(stars, offset, sizeof(u32));
    if (ret == 0) {
        vaddr = stars->base + offset;
        val = readl(vaddr);
        if ((val & 0x1) == 1) {
            return true;
        }
    }
    return false;
}

static int _trs_stars_set_sq_status(struct trs_stars *stars, u32 sqid, int val)
{
    void __iomem *vaddr = NULL;
    unsigned long offset;
    int ret;

    offset = (unsigned long)sqid * stars->stride + TRS_STARS_SCHED_SQ_STATUS_OFFSET;
    ret = trs_stars_range_check(stars, offset, sizeof(u32));
    if (ret == 0) {
        vaddr = stars->base + offset;
        writel((val & 0x1U), vaddr);
    }
    return ret;
}

static int _trs_stars_get_sq_status(struct trs_stars *stars, u32 sqid, u32 *val)
{
    void __iomem *vaddr = NULL;
    unsigned long offset;
    int ret;

    offset = (unsigned long)sqid * stars->stride + TRS_STARS_SCHED_SQ_STATUS_OFFSET;
    ret = trs_stars_range_check(stars, offset, sizeof(u32));
    if (ret == 0) {
        vaddr = stars->base + offset;
        *val = readl(vaddr);
    }
    return ret;
}

static int _trs_stars_set_sq_head(struct trs_stars *stars, u32 sqid, u32 head)
{
    void __iomem *vaddr = NULL;
    unsigned long offset;
    int ret;

    offset = (unsigned long)sqid * stars->stride + TRS_STARS_SCHED_SQ_HEAD_OFFSET;
    ret = trs_stars_range_check(stars, offset, sizeof(u32));
    if (ret == 0) {
        vaddr = stars->base + offset;
        writel(head, vaddr);
    }
    return ret;
}

static int _trs_stars_get_sq_head(struct trs_stars *stars, u32 sqid, u32 *head)
{
    void __iomem *vaddr = NULL;
    unsigned long offset;
    int ret;

    offset = (unsigned long)sqid * stars->stride + TRS_STARS_SCHED_SQ_HEAD_OFFSET;
    ret = trs_stars_range_check(stars, offset, sizeof(u32));
    if (ret == 0) {
        vaddr = stars->base + offset;
        *head = readl(vaddr);
    }
    return ret;
}

static void trs_stars_cqint_pack(struct trs_stars_cqint *cqint, struct trs_stars *stars, u32 group)
{
    cqint->base = stars->base;
    cqint->size = stars->size;
    cqint->group = group;
    cqint->cq_num = stars->cq_num;
    cqint->cq_grp_num = stars->cq_grp_num;
}

int trs_stars_set_cq_l1_mask(struct trs_id_inst *inst, u32 val, u32 group)
{
    struct trs_stars *stars = NULL;
    struct trs_stars_cqint cqint;

    stars = trs_stars_get(inst, TRS_STARS_CQINT);
    if (stars == NULL) {
        return -ENODEV;
    }

    trs_stars_cqint_pack(&cqint, stars, group);
    if (stars->set_cq_l1_mask != NULL) {
        stars->set_cq_l1_mask(&cqint, val);
    }
    trs_stars_put(stars);
    return 0;
}

int trs_stars_get_valid_cq_list(struct trs_id_inst *inst, u32 group, u32 cqid[], u32 num, u32 *valid_num)
{
    struct trs_stars *stars = NULL;
    struct trs_stars_cqint cqint;
    int ret = -EEXIST;

    stars = trs_stars_get(inst, TRS_STARS_CQINT);
    if (stars == NULL) {
        return -ENODEV;
    }
    trs_stars_cqint_pack(&cqint, stars, group);
    if (stars->get_valid_cq_list != NULL) {
        ret = stars->get_valid_cq_list(&cqint, cqid, num, valid_num);
    }
    trs_stars_put(stars);

    return ret;
}

int trs_stars_get_cq_affinity_group(struct trs_id_inst *inst, u32 cq_id, u32 *group)
{
    struct trs_stars *stars = trs_stars_get(inst, TRS_STARS_CQINT);
    if (stars == NULL) {
        return -ENODEV;
    }

    *group = cq_id / (stars->cq_num / stars->cq_grp_num);
    trs_stars_put(stars);

    return 0;
}

/*
 * Sq is disabled, then enable it and return succee.
 * Sq is alreay in enable state, return -EAGAIN.
 */
int trs_stars_sq_enable(struct trs_id_inst *inst, u32 sqid)
{
    struct trs_stars *stars = NULL;
    int ret = -EAGAIN;

    stars = trs_stars_get(inst, TRS_STARS_SCHED);
    if (stars == NULL) {
        return -ENODEV;
    }
    if (!_trs_stars_sq_is_enabled(stars, sqid)) {
        /* Disabled sq status, re-enable it */
        ret = _trs_stars_set_sq_status(stars, sqid, 1);
    }
    trs_stars_put(stars);

    return ret;
}

int trs_stars_get_sq_head_paddr(struct trs_id_inst *inst, u32 sqid, u64 *paddr)
{
    struct trs_stars *stars = NULL;
    unsigned long offset;
    int ret;

    stars = trs_stars_get(inst, TRS_STARS_SCHED);
    if (stars == NULL) {
        return -ENODEV;
    }

    offset = (unsigned long)sqid * stars->stride + TRS_STARS_SCHED_SQ_HEAD_OFFSET;
    ret = trs_stars_range_check(stars, offset, sizeof(u32));
    if (ret == 0) {
        *paddr = (u64)(stars->paddr + offset);
    }

    trs_stars_put(stars);

    return ret;
}

int trs_stars_get_sq_tail_paddr(struct trs_id_inst *inst, u32 sqid, u64 *paddr)
{
    struct trs_stars *stars = NULL;
    unsigned long offset;
    int ret;

    stars = trs_stars_get(inst, TRS_STARS_SCHED);
    if (stars == NULL) {
        trs_err("Stars uninit. (devid=%u; tsid=%u; sqid=%u)\n", inst->devid, inst->tsid, sqid);
        return -ENODEV;
    }

    offset = (unsigned long)sqid * stars->stride + TRS_STARS_SCHED_SQ_TAIL_OFFSET;
    ret = trs_stars_range_check(stars, offset, sizeof(u32));
    if (ret == 0) {
        *paddr = (u64)(stars->paddr + offset);
    }

    trs_stars_put(stars);

    return ret;
}

int trs_stars_get_sq_head(struct trs_id_inst *inst, u32 sqid, u32 *head)
{
    struct trs_stars *stars = NULL;
    int ret;

    stars = trs_stars_get(inst, TRS_STARS_SCHED);
    if (stars == NULL) {
        return -ENODEV;
    }
    ret = _trs_stars_get_sq_head(stars, sqid, head);
    trs_stars_put(stars);

    return ret;
}

int trs_stars_set_sq_head(struct trs_id_inst *inst, u32 sqid, u32 head)
{
    struct trs_stars *stars = NULL;
    int ret;

    stars = trs_stars_get(inst, TRS_STARS_SCHED);
    if (stars == NULL) {
        return -ENODEV;
    }
    if (_trs_stars_sq_is_enabled(stars, sqid)) {
        trs_stars_put(stars);
        trs_err("Sq is enabled. do not set sq head. (devid=%u; tsid=%u; sqid=%u)\n", inst->devid, inst->tsid, sqid);
        return -EBUSY;
    }
    ret = _trs_stars_set_sq_head(stars, sqid, head);
    trs_stars_put(stars);
    return ret;
}

int trs_stars_set_sq_status(struct trs_id_inst *inst, u32 sqid, u32 val)
{
    struct trs_stars *stars = NULL;
    int ret;

    stars = trs_stars_get(inst, TRS_STARS_SCHED);
    if (stars == NULL) {
        return -ENODEV;
    }

    if (val == 0) {
        if (!_trs_stars_sq_is_enabled(stars, sqid)) {
            ret = 0;
            goto out;
        }
        ret = _trs_stars_set_sq_status(stars, sqid, 0);
    } else {
        if (_trs_stars_sq_is_enabled(stars, sqid)) {
            ret = 0;
            goto out;
        }
        ret = _trs_stars_set_sq_status(stars, sqid, 1);
    }
out:
    trs_stars_put(stars);
    return ret;
}

int trs_stars_get_sq_status(struct trs_id_inst *inst, u32 sqid, u32 *val)
{
    struct trs_stars *stars = NULL;
    int ret;

    stars = trs_stars_get(inst, TRS_STARS_SCHED);
    if (stars == NULL) {
        return -ENODEV;
    }
    ret = _trs_stars_get_sq_status(stars, sqid, val);
    trs_stars_put(stars);
    return ret;
}

int trs_stars_get_rtsq_paddr(struct trs_id_inst *inst, u32 sqid, phys_addr_t *paddr, size_t *size)
{
    struct trs_stars *stars = NULL;
    unsigned long offset;
    int ret;

    ret = trs_id_inst_check(inst);
    if (ret != 0) {
        return ret;
    }

    stars = trs_stars_get(inst, TRS_STARS_SCHED);
    if (stars == NULL) {
        return -ENODEV;
    }
    offset = (unsigned long)sqid * stars->stride;
    ret = trs_stars_range_check(stars, offset, (size_t)stars->stride);
    if (ret == 0) {
        if (paddr != NULL) {
            *paddr = stars->paddr + offset;
        }
        if (size != NULL) {
            *size = stars->stride;
        }
    }
    trs_stars_put(stars);
    return ret;
}
EXPORT_SYMBOL(trs_stars_get_rtsq_paddr);

int trs_stars_init(struct trs_id_inst *inst, int type, struct trs_stars_attr *attr)
{
    struct trs_stars *stars = NULL;
    int ret;

    stars = trs_stars_create(inst, type, attr);
    if (stars == NULL) {
        return -ENOMEM;
    }

    ret = trs_stars_add(stars);
    if (ret != 0) {
        trs_stars_destroy(stars);
    }
    trs_debug("Trs stars init. (devid=%u; tsid=%u; type=%d; ret=%d)\n", inst->devid, inst->tsid, type, ret);
    return ret;
}

void trs_stars_uninit(struct trs_id_inst *inst, int type)
{
    trs_stars_del(inst, type);
}

