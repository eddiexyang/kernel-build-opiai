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
#include <linux/types.h>
#include <linux/io.h>

#include "soc_res.h"

#include "trs_rsv_mem.h"
#include "trs_chan_ops_rsv_mem.h"

static const char *g_rsv_mem_name[] = {
    [RSV_MEM_HW_SQCQ] = "TS_SQCQ_MEM",
    [RSV_MEM_MAINT_SQCQ] = "TS_MAINT_SQCQ_MEM"
};

static const char *trs_chan_get_rsv_mem_name(int type)
{
    if (type >= RSV_MEM_MAX) {
        return "Unknown";
    }

    return g_rsv_mem_name[type];
}

static int trs_chan_ops_get_rsv_mem(struct trs_id_inst *inst, int type, phys_addr_t *paddr, size_t *size)
{
    struct res_inst_info res_inst;
    struct soc_rsv_mem_info rsv_mem;
    int ret;

    soc_resmng_inst_pack(&res_inst, inst->devid, TS_SUBSYS, inst->tsid);
    ret = soc_resmng_get_rsv_mem(&res_inst, trs_chan_get_rsv_mem_name(type), &rsv_mem);
    if (ret != 0) {
        trs_info("Rsv_mem_type(%s) is not set. (devid=%u; tsid=%u)\n",
            trs_chan_get_rsv_mem_name(type), inst->devid, inst->tsid);
        return ret;
    }
    *paddr = rsv_mem.rsv_mem;
    *size = rsv_mem.rsv_mem_size;

    trs_info("Trs rsv mem init. (devid=%u; tsid=%u; type=%d; size=0x%lx)\n", inst->devid, inst->tsid, type, *size);
    return 0;
}

static int _trs_chan_ops_rsv_mem_init(struct trs_id_inst *inst, int type)
{
    struct trs_rsv_mem_attr attr;
    void __iomem *vaddr = NULL;
    phys_addr_t paddr;
    size_t size;
    int ret;

    ret = trs_chan_ops_get_rsv_mem(inst, type, &paddr, &size);
    if (ret != 0) {
        return 0;
    }

    vaddr = ioremap_wc(paddr, size);
    if (vaddr == NULL) {
        trs_err("Iomem remap fail. (devid=%u; tsid=%u; type=%d)\n", inst->devid, inst->tsid, type);
        return -ENOMEM;
    }
    attr.paddr = paddr;
    attr.total_size = size;
    attr.vaddr = vaddr;
    attr.flag = 0;
    ret = trs_rsv_mem_init(inst, type, &attr);
    if (ret != 0) {
        iounmap(vaddr);
        trs_err("Trs rsv mem init fail. (devid=%u; tsid=%u; type=%d)\n", inst->devid, inst->tsid, type);
    }
    return ret;
}

static void _trs_chan_ops_rsv_mem_uninit(struct trs_id_inst *inst, int type)
{
    trs_rsv_mem_uninit(inst, type);
}

int trs_chan_ops_rsv_mem_init(struct trs_id_inst *inst)
{
    int ret;

    ret = _trs_chan_ops_rsv_mem_init(inst, RSV_MEM_HW_SQCQ);
    if (ret != 0) {
        return ret;
    }

    ret = _trs_chan_ops_rsv_mem_init(inst, RSV_MEM_MAINT_SQCQ);
    if (ret != 0) {
        _trs_chan_ops_rsv_mem_uninit(inst, RSV_MEM_HW_SQCQ);
    }
    return ret;
}

void trs_chan_ops_rsv_mem_uninit(struct trs_id_inst *inst)
{
    _trs_chan_ops_rsv_mem_uninit(inst, RSV_MEM_HW_SQCQ);
    _trs_chan_ops_rsv_mem_uninit(inst, RSV_MEM_MAINT_SQCQ);
}

