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

#include <linux/hugetlb.h>
#include <linux/io.h>
#include <linux/mm.h>

#include "devdrv_interface.h"
#include "soc_res.h"
#include "trs_device_comm.h"
#include "trs_device_tsfw_shr_mem.h"

#ifdef CFG_MEMORY_OPTIMIZE
#define MINI_V3_SHARE_MEM_BLOCK_NUM 8
#else
#define MINI_V3_SHARE_MEM_BLOCK_NUM 15
#endif
#define CLOUD_V2_SHARE_MEM_BLOCK_NUM 16
#define MAX_SHARE_MEM_BLOCK_NUM CLOUD_V2_SHARE_MEM_BLOCK_NUM
#define TASK_CACHE_MAGIC 0xABCD
struct tsfw_shr_mem {
    u32 magic;
    u32 count;
    u64 addr[MAX_SHARE_MEM_BLOCK_NUM];
};

static void trs_get_shr_mem_block_num(u32 devid, u32 *task_num)
{
    u32 chip_type = uda_get_chip_type(devid);

    switch (chip_type) {
        case HISI_MINI_V3:
            *task_num = MINI_V3_SHARE_MEM_BLOCK_NUM;
            break;
        case HISI_CLOUD_V2:
            *task_num = CLOUD_V2_SHARE_MEM_BLOCK_NUM;
            break;
        default:
            trs_info("Chip_type. (chip_type=%d)\n", chip_type);
            *task_num = MINI_V3_SHARE_MEM_BLOCK_NUM;
    }
    return;
}

void trs_tsfw_shr_mem_config(struct trs_id_inst *inst)
{
    struct tsfw_shr_mem *shr_mem = NULL;
    struct soc_rsv_mem_info rsv_mem;
    struct res_inst_info res_inst;
    struct page *hpage = NULL;
    u32 block_num, i, j;
    int ret;

    soc_resmng_inst_pack(&res_inst, inst->devid, TS_SUBSYS, 0);
    ret = soc_resmng_get_rsv_mem(&res_inst, "tsfw_shr_mem", &rsv_mem);
    if (ret != 0) {
        return;
    }

    shr_mem = ioremap(rsv_mem.rsv_mem, rsv_mem.rsv_mem_size);
    if (shr_mem == NULL) {
        trs_err("Remap failed.\n");
        return;
    }

    trs_get_shr_mem_block_num(inst->devid, &block_num);

    memset_io(shr_mem, 0, sizeof(*shr_mem));
    for (i = 0; i < block_num; i++) {
#ifndef EMU_ST /* if delete, the emu_st will mem leak */
        hpage = hugetlb_alloc_hugepage(trs_get_ts_nid(inst->devid), HUGETLB_ALLOC_BUDDY);
        if (hpage == NULL) {
            trs_err("Alloc hugepage failed. (i=%u)", i);
            for (j = 0; j < i; j++) {
                put_page(phys_to_page(shr_mem->addr[j]));
            }
            break;
        }
#endif
        shr_mem->addr[i] = page_to_phys(hpage);
    }

    shr_mem->magic = TASK_CACHE_MAGIC;
    shr_mem->count = i;
    iounmap(shr_mem);
    shr_mem = NULL;

    trs_info("Init tsfw shr mem success.\n");
}
