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
* Create: 2023-1-10
*/
#include <linux/types.h>
#include <linux/io.h>

#include "securec.h"
#include "soc_res.h"
#include "soc_adapt.h"

#include "trs_pm_adapt.h"
#include "trs_msg.h"
#include "trs_rsv_mem.h"
#include "trs_host_msg.h"
#include "trs_chan_near_ops_rsv_mem.h"

static int trs_chan_ops_set_sqcq_rsv_mem_dev_addr(struct trs_id_inst *inst, phys_addr_t paddr, size_t size)
{
    struct soc_rsv_mem_info rsv_mem = {.rsv_mem = paddr, .rsv_mem_size = size};
    struct res_inst_info res_inst;
    int ret;

    soc_resmng_inst_pack(&res_inst, inst->devid, TS_SUBSYS, inst->tsid);
    ret = soc_resmng_set_rsv_mem(&res_inst, "DEV_TS_SQCQ_MEM", &rsv_mem);
    if (ret != 0) {
        trs_info("Set rsv mem failed. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return ret;
    }

    return 0;
}


static int trs_chan_ops_get_rsv_mem(struct trs_id_inst *inst, const char *name, phys_addr_t *paddr, size_t *size)
{
    struct soc_rsv_mem_info rsv_mem;
    struct res_inst_info res_inst;
    int ret;

    soc_resmng_inst_pack(&res_inst, inst->devid, TS_SUBSYS, inst->tsid);
    ret = soc_resmng_get_rsv_mem(&res_inst, name, &rsv_mem);
    if (ret != 0) {
        trs_info("Rsv mem is not set. (devid=%u; tsid=%u; name=%s)\n", inst->devid, inst->tsid, name);
        return ret;
    }
    *paddr = rsv_mem.rsv_mem;
    *size = rsv_mem.rsv_mem_size;

    return 0;
}

static int _trs_chan_ops_rsv_mem_init(struct trs_id_inst *inst, int type)
{
    struct trs_rsv_mem_attr attr;
    void __iomem *vaddr = NULL;
    phys_addr_t paddr;
    size_t size;
    int ret;

    ret = trs_chan_ops_get_rsv_mem(inst, "TS_SQCQ_MEM", &paddr, &size);
    if (ret != 0) {
        return 0;
    }

    vaddr = ioremap(paddr, size);
    if (vaddr == NULL) {
        trs_err("Iomem remap fail. (devid=%u; tsid=%u; type=%d)\n", inst->devid, inst->tsid, type);
        return -ENOMEM;
    }
    attr.paddr = paddr;
    attr.total_size = size;
    attr.vaddr = vaddr;
    attr.flag = TRS_RSV_MEM_FLAG_DEVICE;
    ret = trs_rsv_mem_init(inst, type, &attr);
    if (ret != 0) {
        iounmap(vaddr);
        trs_err("Trs rsv mem init fail. (devid=%u; tsid=%u; type=%d; ret=%d)\n", inst->devid, inst->tsid, type, ret);
    }
    return ret;
}

static void _trs_chan_ops_rsv_mem_uninit(struct trs_id_inst *inst, int type)
{
    trs_rsv_mem_uninit(inst, type);
}

int trs_chan_near_sqcq_rsv_mem_h2d(struct trs_id_inst *inst, u64 host_addr, u64 *dev_addr)
{
    phys_addr_t host_base, dev_base;
    size_t size;
    int ret;

    ret = trs_chan_ops_get_rsv_mem(inst, "TS_SQCQ_MEM", &host_base, &size);
    ret |= trs_chan_ops_get_rsv_mem(inst, "DEV_TS_SQCQ_MEM", &dev_base, &size);
    if (ret != 0) {
        return ret;
    }

    *dev_addr = dev_base + (host_addr - host_base);
    return 0;
}

static int trs_chan_ops_dev_sqcq_rsv_mem_addr_init(struct trs_id_inst *inst)
{
    struct trs_msg_data msg;
    struct trs_msg_get_phy_addr *info = (struct trs_msg_get_phy_addr *)msg.payload;
    const char *name = "TS_SQCQ_MEM";
    int ret;

    if (trs_soc_get_sq_mem_side(inst) != TRS_CHAN_DEV_MEM) {
        return 0;
    }
    msg.header.tsid = inst->tsid;
    msg.header.cmdtype = TRS_MSG_GET_PHY_ADDR;
    msg.header.valid = TRS_MSG_SEND_MAGIC;
    msg.header.result = 0;

    (void)strcpy_s(info->name, SYNC_MAX_NAME_LEN, name);

    ret = trs_host_msg_send(inst->devid, &msg, sizeof(struct trs_msg_data));
    if (ret != 0) {
        trs_err("Msg send fail. (devid=%u; tsid=%u; ret=%d; result=%d)\n",
            inst->devid, inst->tsid, ret, msg.header.result);
        return -ENODEV;
    }

    return trs_chan_ops_set_sqcq_rsv_mem_dev_addr(inst, (phys_addr_t)info->addr, (size_t)info->size);
}

int trs_chan_near_ops_rsv_mem_init(struct trs_id_inst *inst)
{
    int ret = trs_chan_ops_dev_sqcq_rsv_mem_addr_init(inst);
    ret |= _trs_chan_ops_rsv_mem_init(inst, RSV_MEM_HW_SQCQ);
    return ret;
}

void trs_chan_near_ops_rsv_mem_uninit(struct trs_id_inst *inst)
{
    _trs_chan_ops_rsv_mem_uninit(inst, RSV_MEM_HW_SQCQ);
}

