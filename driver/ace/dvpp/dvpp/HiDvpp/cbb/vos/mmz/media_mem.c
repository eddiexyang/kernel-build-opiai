/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2023. All rights reserved.
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
 * Create: 2020-4-20
 */

#include "securec.h"
#include "osal_mmz.h"
#include "mmz_ext.h"

static ot_mmz_t g_mmz_cb[MAX_MMZ_NUM] = {0};

OSAL_LIST_HEAD(g_mmz_list);

// 当前先自己写一个，等VOS支持后信号量切换VOS接口
#define DEFINE_SEMAPHORE(a) osal_semaphore_t a
static DEFINE_SEMAPHORE(g_mmz_lock);

hi_s32 osal_down(const osal_semaphore_t *sem)
{
    return 0;
}

void osal_up(const osal_semaphore_t *sem)
{
    return;
}

static int anony = 0;
unsigned int g_mmb_number = 0; /* for mmb id */

static int ot_mmz_get_anony(void)
{
    return anony;
}

static struct osal_list_head *ot_mmz_get_mmz_list(void)
{
    return &g_mmz_list;
}

#define begin_list_for_each_mmz(p, gfp, mmz_name)        \
    osal_list_for_each_entry(p, ot_mmz_get_mmz_list(), list)   \
    {                                                    \
        if ((gfp) == 0 ? 0 : (p)->gfp != (gfp)) {        \
            continue;                                       \
        }                                                   \
        if (((mmz_name) == NULL) || (*(mmz_name) == '\0')) { \
            if (ot_mmz_get_anony() == 1) {                \
                if (strcmp("anonymous", (p)->name)) {    \
                    continue;                            \
                }                                        \
            } else {                                     \
                break;                                   \
            }                                            \
        } else {                                         \
            if (strcmp(mmz_name, (p)->name)) {           \
                continue;                                \
            }                                            \
        }                                                \
        mmz_trace(1, OT_MMZ_FMT_S, ot_mmz_fmt_arg(p));
#define end_list_for_each_mmz() }

ot_mmz_t *ot_mmz_create_v2(const char *name, const ot_mmz_create_attr *mmz_create_attr, hi_u32 idx)
{
    ot_mmz_t *p = NULL;

    if ((name == NULL) || (mmz_create_attr == NULL)) {
        mmz_trace(MMZ_ERR_LEVEL, "%s: 'name' or mmz_attr can not be zero!", __FUNCTION__);
        return NULL;
    }

    if (idx >= MAX_MMZ_NUM) {
        mmz_trace(MMZ_ERR_LEVEL, "%s: idx should be smaller than %d!", __FUNCTION__, MAX_MMZ_NUM);
        return NULL;
    }

    p = &g_mmz_cb[idx];

    (void)memset_s(p, sizeof(ot_mmz_t), 0, sizeof(ot_mmz_t));
    if (strncpy_s(p->name, OT_MMZ_NAME_LEN, name, OT_MMZ_NAME_LEN - 1) != EOK) {
        mmz_trace(MMZ_ERR_LEVEL, "%s - strncpy_s failed!\n", __FUNCTION__);
        return NULL;
    }
    p->gfp         = mmz_create_attr->gfp;
    p->phys_start  = mmz_create_attr->phys_start;
    p->nbytes      = mmz_create_attr->nbytes;
    p->alloc_type  = mmz_create_attr->alloc_type;
    p->block_align = mmz_create_attr->block_align;

    OSAL_INIT_LIST_HEAD(&p->list);
    OSAL_INIT_LIST_HEAD(&p->mmb_list);

    return p;
}

int ot_mmz_destroy(const ot_mmz_t *zone)
{
    if (zone == NULL) {
        mmz_trace(MMZ_ERR_LEVEL, "%s - zone is null!\n", __FUNCTION__);
        return -1;
    }

    if (zone->destructor) {
        zone->destructor(zone);
    }

    return 0;
}

static int _check_mmz(const ot_mmz_t *zone)
{
    ot_mmz_t *p = NULL;

    unsigned long new_start = zone->phys_start;
    unsigned long new_end = zone->phys_start + zone->nbytes;

    if (zone->nbytes == 0) {
        mmz_trace(MMZ_ERR_LEVEL, "%s - zone->nbytes is 0!\n", __FUNCTION__);
        return -1;
    }

    osal_list_for_each_entry(p, &g_mmz_list, list) {
        unsigned long start, end;
        start = p->phys_start;
        end = p->phys_start + p->nbytes;

        if (new_start >= end) {
            continue;
        } else if ((new_start < start) && (new_end <= start)) {
            continue;
        } else {
        }

        mmz_trace(MMZ_ERR_LEVEL, "ERROR: Conflict MMZ:\n");
        mmz_trace(MMZ_ERR_LEVEL, "MMZ new:   " OT_MMZ_FMT_S "\n", ot_mmz_fmt_arg(zone));
        mmz_trace(MMZ_ERR_LEVEL, "MMZ exist: " OT_MMZ_FMT_S "\n", ot_mmz_fmt_arg(p));
        mmz_trace(MMZ_ERR_LEVEL, "Add new MMZ failed!\n");
        return -1;
    }

    return 0;
}

int ot_mmz_register(ot_mmz_t *zone)
{
    int ret;

    if (zone == NULL) {
        mmz_trace(MMZ_ERR_LEVEL, "%s - zone is null!\n", __FUNCTION__);
        return -1;
    }

    mmz_trace(MMZ_INFO_LEVEL, OT_MMZ_FMT_S, ot_mmz_fmt_arg(zone));

    osal_down(&g_mmz_lock);

    ret = _check_mmz(zone);
    if (ret) {
        osal_up(&g_mmz_lock);
        return ret;
    }

    OSAL_INIT_LIST_HEAD(&zone->mmb_list);

    osal_list_add(&zone->list, &g_mmz_list);

    osal_up(&g_mmz_lock);

    return 0;
}

int ot_mmz_unregister(ot_mmz_t *zone)
{
    int losts = 0;
    ot_mmb_t *p = NULL;

    if (zone == NULL) {
        mmz_trace(MMZ_ERR_LEVEL, "%s - zone is null!\n", __FUNCTION__);
        return -1;
    }

    osal_down(&g_mmz_lock);
    osal_list_for_each_entry(p, &zone->mmb_list, list) {
        mmz_trace(MMZ_WARNING_LEVEL,  "          MB Lost: " OT_MMB_FMT_S "\n", ot_mmb_fmt_arg(p));
        losts++;
    }

    if (losts != 0) {
        mmz_trace(MMZ_ERR_LEVEL, "%d mmbs not free, mmz<%s> can not be deregistered!\n", losts, zone->name);
        osal_up(&g_mmz_lock);
        return -1;
    }

    osal_list_del(&zone->list);
    osal_up(&g_mmz_lock);

    return 0;
}

static int calculate_fixed_region(unsigned long end, unsigned long start,
    unsigned long *fixed_start, unsigned long *fixed_len, unsigned long len)
{
    unsigned long blank_len;
    int ret = 0;

    blank_len = end - start;
    if ((blank_len < *fixed_len) && (blank_len >= len)) {
        *fixed_len = blank_len;
        *fixed_start = start;
        mmz_trace(MMZ_INFO_LEVEL, "%d: fixed_region: len=%luKB\n", __LINE__, *fixed_len / SZ_1K);
        ret = 1;
    }
    return ret;
}


static unsigned long _find_fixed_region(unsigned long *region_len, const ot_mmz_t *mmz,
    unsigned long size, unsigned long align)
{
#ifndef DVPP_VCAST_UTST
    unsigned long start, len;
    unsigned long fixed_start = 0;
    unsigned long fixed_len = -1;
    ot_mmb_t *p = NULL;

    align = mmz_grain_align(align);
    start = mmz_align2(mmz->phys_start, align);
    len = mmz_grain_align(size);

    osal_list_for_each_entry(p, &mmz->mmb_list, list) {
        ot_mmb_t *next = NULL;
        mmz_trace(MMZ_INFO_LEVEL, "p->length = %luKB \t", p->length / SZ_1K);
        next = osal_list_entry(p->list.next, __typeof__(*p), list);

        /* if p is the first entry or not */
        if (osal_list_first_entry(&mmz->mmb_list, __typeof__(*p), list) == p) {
            (void)calculate_fixed_region(p->phys_addr, start, &fixed_start, &fixed_len, len);
        }

        start = mmz_align2((p->phys_addr + p->length), align);
        // OSAL_BUG_ON((start < mmz->phys_start) || (start > (mmz->phys_start + mmz->nbytes)));

        /* if we have to alloc after the last node */
        if (osal_list_is_last(&p->list, &mmz->mmb_list)) {
            if (calculate_fixed_region(mmz->phys_start + mmz->nbytes, start, &fixed_start, &fixed_len, len) == 1) {
                break;
            } else {
                if (fixed_len != -1) {
                    goto out;
                }

                fixed_start = 0;
                mmz_trace(MMZ_INFO_LEVEL, "%d: fixed_region: len=%luKB\n", __LINE__, fixed_len / SZ_1K);
                goto out;
            }
        }

        /* blank is too little */
        if ((start + len) > next->phys_addr) {
            mmz_trace(MMZ_INFO_LEVEL, "start + len(%lu) > next->phy_addr\n", len);
            continue;
        }

        (void)calculate_fixed_region(next->phys_addr, start, &fixed_start, &fixed_len, len);
    }

    if ((mmz_grain_align(start + len) <= (mmz->phys_start + mmz->nbytes)) &&
        (start >= mmz->phys_start) && (start < (mmz->phys_start + mmz->nbytes))) {
        fixed_start = start;
    } else {
        fixed_start = 0;
    }
    mmz_trace(MMZ_INFO_LEVEL, "%d: fixed_region: len=%luKB\n", __LINE__, len / SZ_1K);

out:
    *region_len = len;
    return fixed_start;
#else
    return 0;
#endif
}

static int _do_mmb_alloc(ot_mmb_t *mmb)
{
    ot_mmb_t *p = NULL;

    /* add mmb sorted */
    osal_list_for_each_entry(p, &mmb->zone->mmb_list, list) {
        if (mmb->phys_addr < p->phys_addr) {
            break;
        }

        if (mmb->phys_addr == p->phys_addr) {
            mmz_trace(MMZ_ERR_LEVEL, "ERROR: media-mem allocator bad in %s! (%s, %d)",
                mmb->zone->name, __FUNCTION__, __LINE__);

            return -1;
        }
    }
    osal_list_add(&mmb->list, p->list.prev);

    mmz_trace(MMZ_INFO_LEVEL, OT_MMB_FMT_S, ot_mmb_fmt_arg(mmb));

    return 0;
}

static ot_mmb_t *mmb_alloc(const char *name, unsigned long size, unsigned long align,
    unsigned long gfp, const char *mmz_name, const ot_mmz_t *_user_mmz)
{
    ot_mmz_t *mmz = NULL;
    ot_mmb_t *mmb = NULL;

    unsigned long start, region_len;

    unsigned long fixed_start = 0;
    unsigned long fixed_len = ~1;
    ot_mmz_t *fixed_mmz = NULL;
    errno_t err_value;

    if ((size == 0) || (size > 0x40000000UL)) {
        mmz_trace(MMZ_ERR_LEVEL, "size is %lu, it should be in (0, %lu]", size, 0x40000000UL);
        return NULL;
    }

    if (align == 0) {
        align = MMZ_GRAIN;
    }

    size = mmz_grain_align(size);

    mmz_trace(MMZ_INFO_LEVEL, "size=%luKB, align=%lu", size / SZ_1K, align);

    begin_list_for_each_mmz(mmz, gfp, mmz_name)
        if ((_user_mmz != NULL) && (_user_mmz != mmz)) {
            continue;
        }

        start = _find_fixed_region(&region_len, mmz, size, align);
        if ((fixed_len > region_len) && (start != 0)) {
            fixed_len = region_len;
            fixed_start = start;
            fixed_mmz = mmz;
        }
    end_list_for_each_mmz()

    if (fixed_mmz == NULL) {
        return NULL;
    }

    // mmb控制块指针指向申请的内存头
    mmb = (ot_mmb_t *)fixed_start;

    (void)memset_s(mmb, sizeof(ot_mmb_t), 0, sizeof(ot_mmb_t));
    mmb->zone = fixed_mmz;
    mmb->phys_addr = fixed_start;
    mmb->length = size;
    mmb->id = ++g_mmb_number;

    if (name != NULL) {
        err_value = strncpy_s(mmb->name, OT_MMZ_NAME_LEN, name, OT_MMZ_NAME_LEN - 1);
    } else {
        err_value = strncpy_s(mmb->name, OT_MMZ_NAME_LEN, "<null>", OT_MMB_NAME_LEN - 1);
    }

    if ((err_value != EOK) || _do_mmb_alloc(mmb)) {
        mmb = NULL;
    }

    return mmb;
}

ot_mmb_t *ot_mmb_alloc(const char *name, unsigned long size, unsigned long align,
    unsigned long gfp, const char *mmz_name)
{
    ot_mmb_t *mmb = NULL;

    osal_down(&g_mmz_lock);
    mmb = mmb_alloc(name, size, align, gfp, mmz_name, NULL);
    osal_up(&g_mmz_lock);

    return mmb;
}

static int _mmb_free(ot_mmb_t *mmb)
{
    osal_list_del(&mmb->list);

    return 0;
}

int ot_mmb_free(ot_mmb_t *mmb)
{
    if (mmb == NULL) {
        mmz_trace(MMZ_ERR_LEVEL, "mmb is null");
        return -1;
    }

    mmz_trace(MMZ_INFO_LEVEL, OT_MMB_FMT_S, ot_mmb_fmt_arg(mmb));
    osal_down(&g_mmz_lock);

    _mmb_free(mmb);
    osal_up(&g_mmz_lock);
    return 0;
}

#define mach_mmb(p, val, member)                                                   \
    do {                                                                                 \
        ot_mmz_t *__mach_mmb_zone__ = NULL;                                             \
        (p) = NULL;                                                                      \
        osal_list_for_each_entry(__mach_mmb_zone__, &g_mmz_list, list) {                 \
            ot_mmb_t *__mach_mmb__ = NULL;                                              \
            osal_list_for_each_entry(__mach_mmb__, &__mach_mmb_zone__->mmb_list, list) { \
                if (__mach_mmb__->member == (val)) {                                     \
                    (p) = __mach_mmb__;                                                  \
                    break;                                                               \
                }                                                                        \
            }                                                                            \
            if ((p) != NULL) {                                                           \
                break;                                                                   \
            }                                                                            \
        }                                                                                \
    } while (0)

ot_mmb_t *ot_mmb_getby_phys(unsigned long addr)
{
    ot_mmb_t *p = NULL;
    osal_down(&g_mmz_lock);
    mach_mmb(p, addr, phys_addr);
    osal_up(&g_mmz_lock);
    return p;
}

ot_mmz_t *ot_mmz_find(unsigned long gfp, const char *mmz_name)
{
    ot_mmz_t *p = NULL;

    osal_down(&g_mmz_lock);
    begin_list_for_each_mmz(p, gfp, mmz_name)
        osal_up(&g_mmz_lock);
        return p;
    end_list_for_each_mmz()
    osal_up(&g_mmz_lock);

    return NULL;
}

void mmz_exit_check(void)
{
    ot_mmz_t *pmmz = NULL;
    struct osal_list_head *p = NULL;
    struct osal_list_head *n = NULL;

    osal_list_for_each_safe(p, n, &g_mmz_list) {
        pmmz = osal_list_entry(p, ot_mmz_t, list);
        mmz_trace(MMZ_WARNING_LEVEL, "MMZ force removed: " OT_MMZ_FMT_S "\n", ot_mmz_fmt_arg(pmmz));
        ot_mmz_unregister(pmmz);
        ot_mmz_destroy(pmmz);
    }
}