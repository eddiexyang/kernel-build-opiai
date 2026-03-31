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
#include <linux/kref.h>
#include <linux/io.h>
#include <linux/delay.h>

#include "soc_res.h"
#include "id_pool.h"
#include "trs_pub_def.h"
#include "trs_chip_def.h"
#include "trs_chan_ops_stars.h"

static inline u32 _trs_chan_ops_stars_cqint_get_mid_status(void __iomem *base, u32 mid_index)
{
    return readl(base + TRS_STARS_CQINT_MID_STATUS_OFFSET(mid_index));
}

void _trs_chan_ops_stars_cqint_set_l1_status(void __iomem *base, u32 l1_bit)
{
    u32 val = readl(base + TRS_STARS_CQINT_L1_STATUS_OFFSET);

    writel((val | (1 << l1_bit)), base + TRS_STARS_CQINT_L1_STATUS_OFFSET);
}

void _trs_chan_ops_stars_cqint_set_mid_status(void __iomem *base, u32 mid_index, u32 mid_bit)
{
    u32 val = readl(base + TRS_STARS_CQINT_MID_STATUS_OFFSET(mid_index));
    writel((val | (1 << mid_bit)), base + TRS_STARS_CQINT_MID_STATUS_OFFSET(mid_index));
}

static inline u32 _trs_chan_ops_stars_cqint_get_l2_status(void __iomem *base, u32 l2_index)
{
    return readl(base + TRS_STARS_CQINT_L2_STATUS_OFFSET(l2_index));
}

void _trs_chan_ops_stars_cqint_set_l2_status(void __iomem *base, u32 l2_index, u32 l2_bit)
{
    u32 val = readl(base + TRS_STARS_CQINT_L2_STATUS_OFFSET(l2_index));
    writel(val | (1 << l2_bit), base + TRS_STARS_CQINT_L2_STATUS_OFFSET(l2_index));
}

static void _trs_chan_ops_stars_cqint_set_l2_ctrl(void __iomem *base, u32 l2_index, u32 val)
{
    writel(val, base + TRS_STARS_CQINT_L2_CTRL_OFFSET(l2_index));
}

static void trs_chan_ops_stars_get_valid_cqs(u32 l2_index, u32 l2_status, u32 cqid[], u32 *cq_index)
{
    u32 l2_bit;

    for (l2_bit = 0; l2_bit < TRS_STARS_CQINT_L2_STATUS_WIDTH; l2_bit++) {
        if (trs_stars_test_bit(l2_bit, l2_status) == 0) {
            continue;
        }

        cqid[*cq_index] = l2_index * TRS_STARS_CQINT_L2_STATUS_WIDTH + l2_bit;
        trs_debug("Trs cqint. (status=%u; l2_bit=%u; cq%u)\n", l2_status, l2_bit, cqid[*cq_index]);
        (*cq_index)++;
    }
}

static int trs_chan_ops_stars_ops_get_valid_cq_list(struct trs_stars_cqint *cqint, u32 cqid[], u32 num, u32 *valid_num)
{
    u32 i, mid_index, mid_status, mid_width, l2_index, l2_status, cq_index;

    cq_index = 0;

#ifdef CFG_MEMORY_OPTIMIZE
    mid_width = TRS_STARS_CQINT_MID_STATUS_WIDTH;
#else
    mid_width = cqint->cq_num / (TRS_STARS_CQINT_L2_STATUS_WIDTH * TRS_STARS_CQINT_MID_REG_NUM);
#endif

    for (mid_index = 0; mid_index < TRS_STARS_CQINT_MID_REG_NUM; mid_index++) {
        mid_status = _trs_chan_ops_stars_cqint_get_mid_status(cqint->base, mid_index);
        if (mid_status == 0) {
            continue;
        }

        for (i = 0; i < mid_width; i++) {
            if (trs_stars_test_bit(i, mid_status) == 0) {
                continue;
            }
            l2_index = (mid_index * mid_width + i);
            l2_status = _trs_chan_ops_stars_cqint_get_l2_status(cqint->base, l2_index);
            _trs_chan_ops_stars_cqint_set_l2_ctrl(cqint->base, l2_index, l2_status);
            trs_chan_ops_stars_get_valid_cqs(l2_index, l2_status, cqid, &cq_index);
        }
    }

    *valid_num = cq_index;

    return 0;
}

static void trs_chan_ops_stars_ops_set_cq_l1_mask(struct trs_stars_cqint *cqint, int val)
{
    writel((val & 0x1), cqint->base + TRS_STARS_CQINT_L1_MASK_OFFSET);
}

static void trs_chan_ops_stars_sched_attr_pack(struct trs_stars_attr *attr, phys_addr_t paddr, size_t size)
{
    attr->paddr = paddr;
    attr->size = size;
    attr->set_cq_l1_mask = NULL;
    attr->get_valid_cq_list = NULL;
    attr->stride = TRS_STARS_SCHED_STRIDE;
}

static int trs_chan_ops_stars_cqint_attr_pack(struct trs_id_inst *inst, struct trs_stars_attr *attr,
    phys_addr_t paddr, size_t size)
{
    struct id_pool_inst pool_inst;
    u32 rsv_hw_cq_num;
    int ret;

    attr->paddr = paddr;
    attr->size = size;
    attr->cq_grp_num = 1;
    id_pool_inst_pack(&pool_inst, inst->devid, inst->tsid);
    attr->set_cq_l1_mask = trs_chan_ops_stars_ops_set_cq_l1_mask;
    attr->get_valid_cq_list = trs_chan_ops_stars_ops_get_valid_cq_list;
    ret = id_pool_get_total_num(&pool_inst, TRS_HW_CQ_ID, &attr->cq_num);
    if (ret != 0) {
        trs_err("Get hw cq toltal num failed. (devid=%u; tsid=%u; ret=%d)\n", inst->devid, inst->tsid, ret);
        return ret;
    }
    ret = id_pool_get_total_num(&pool_inst, TRS_RSV_HW_CQ_ID, &rsv_hw_cq_num);
    if (ret == 0) { /* Probably no RSV HW CQ */
        attr->cq_num += rsv_hw_cq_num;
    }

    return 0;
}

int trs_chan_ops_stars_init(struct trs_id_inst *inst)
{
    struct res_inst_info res_inst;
    struct soc_reg_base_info io_base;
    struct trs_stars_attr stars_attr;
    int ret;

    soc_resmng_inst_pack(&res_inst, inst->devid, TS_SUBSYS, inst->tsid);
    ret = soc_resmng_get_reg_base(&res_inst, "TS_STARS_RTSQ_SCHED_REG", &io_base);
    if (ret == 0) {
        trs_chan_ops_stars_sched_attr_pack(&stars_attr, io_base.io_base, io_base.io_base_size);
        ret = trs_stars_init(inst, TRS_STARS_SCHED, &stars_attr);
        if (ret != 0) {
            return ret;
        }
    }

    ret = soc_resmng_get_reg_base(&res_inst, "TS_STARS_CQINT_REG", &io_base);
    if (ret == 0) {
        ret = trs_chan_ops_stars_cqint_attr_pack(inst, &stars_attr, io_base.io_base, io_base.io_base_size);
        if (ret != 0) {
            trs_err("Cqint attr pack failed. (devid=%u; tsid=%u; ret=%d)\n", inst->devid, inst->tsid, ret);
            trs_stars_uninit(inst, TRS_STARS_SCHED);
            return ret;
        }
        trs_info("Cq int. (devid=%u; tsid=%u; cq_grp_num=%u; cq_num=%u)\n",
            inst->devid, inst->tsid, stars_attr.cq_grp_num, stars_attr.cq_num);

        ret = trs_stars_init(inst, TRS_STARS_CQINT, &stars_attr);
        if (ret != 0) {
            trs_stars_uninit(inst, TRS_STARS_SCHED);
            return ret;
        }
    }

    return 0;
}

void trs_chan_ops_stars_uninit(struct trs_id_inst *inst)
{
    trs_stars_uninit(inst, TRS_STARS_CQINT);
    trs_stars_uninit(inst, TRS_STARS_SCHED);
}

