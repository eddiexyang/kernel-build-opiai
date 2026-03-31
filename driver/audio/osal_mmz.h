/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 * Create: 2012-1-1
 */

#ifndef _OSAL_MMZ_H
#define _OSAL_MMZ_H

#include "hi_osal.h"

#define OT_MMZ_NAME_LEN           32
#define OT_MMB_NAME_LEN           32

#define MAX_MMZ_NUM 6

#define SZ_1K           0x00000400

#ifndef PAGE_SIZE
#define PAGE_SIZE       4 * SZ_1K
#endif

struct ot_media_memory_zone {
    char name[OT_MMZ_NAME_LEN];

    unsigned long gfp;

    unsigned long phys_start;
    unsigned long nbytes;

    struct osal_list_head list;
    union {
        struct device *cma_dev;
        unsigned char *bitmap;
    };
    struct osal_list_head mmb_list;

    unsigned int alloc_type;
    unsigned long block_align;

    void (*destructor)(const void *);
};
typedef struct ot_media_memory_zone ot_mmz_t;

// TODO
#define OT_MMZ_FMT_S              "PHYS(0x%08lX, 0x%08lX), GFP=%lu, nBYTES=%luKB,    NAME=\"%s\""
#define ot_mmz_fmt_arg(p) (p)->phys_start, (p)->phys_start + (p)->nbytes - 1, (p)->gfp, (p)->nbytes / SZ_1K, (p)->name

struct ot_media_memory_block {
#ifndef MMZ_V2_SUPPORT
    unsigned int id;
#endif
    char name[OT_MMB_NAME_LEN];
    struct ot_media_memory_zone *zone;
    struct osal_list_head list;

    unsigned long phys_addr;
    void *kvirt;
    unsigned long length;

    unsigned long flags;

    unsigned int order;

    int phy_ref;
    int map_ref;
};
typedef struct ot_media_memory_block ot_mmb_t;

#define OT_MMB_FMT_S       "flags=0x%08lX, length=%luKB, name=\"%s\""
#define ot_mmb_fmt_arg(p)  (p)->flags, (p)->length / SZ_1K, (p)->name

#define MMZ_DBG_LEVEL              0x0
#define MMZ_INFO_LEVEL             0x1
#define MMZ_ERR_LEVEL              0x2
#define MMZ_WARNING_LEVEL              0x2

#define mmz_trace(level, s, params...)                                             \
    do {                                                                           \
    } while (0)

#define MMZ_GRAIN                  PAGE_SIZE

#define mmz_align2(x, g) (((g) == 0) ? (0) : ((((x) + (g) - 1) / (g)) * (g)))
#define mmz_grain_align(x) mmz_align2(x, MMZ_GRAIN)


typedef struct {
    unsigned long gfp;
    unsigned long phys_start;
    unsigned long nbytes;
    unsigned long block_align;
    unsigned int alloc_type;
} ot_mmz_create_attr;

/*
 * APIs
 */
ot_mmz_t *ot_mmz_create_v2(const char *name, const ot_mmz_create_attr *mmz_create_attr, hi_u32 idx);

int ot_mmz_destroy(const ot_mmz_t *zone);

int ot_mmz_register(ot_mmz_t *zone);
int ot_mmz_unregister(ot_mmz_t *zone);
ot_mmz_t *ot_mmz_find(unsigned long gfp, const char *mmz_name);

ot_mmb_t *ot_mmb_alloc(const char *name, unsigned long size, unsigned long align,
    unsigned long gfp, const char *mmz_name);
int ot_mmb_free(ot_mmb_t *mmb);
ot_mmb_t *ot_mmb_getby_phys(unsigned long addr);
void mmz_exit_check(void);

#endif
