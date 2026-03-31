/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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
* Create: 2019-10-15
*/
#ifndef TSDRV_UT
#ifdef CFG_FEATURE_CDQM

#include <linux/types.h>
#include <asm/io.h>

#include "cdq_manager.h"
#include "tsdrv_cdqm_module.h"
#include "tsdrv_log.h"
#define CDQM_F2NF_NS_NUM 2


STATIC inline void cdqm_writel(const void __iomem *io_base, u32 offset, u32 val)
{
    TSDRV_PRINT_INFO("write reg(0x%pK) + offset (0x%x) value(0x%x).\n", (void *)(uintptr_t)io_base, offset, val);
    writel(val, (volatile void*)io_base + offset);
}

STATIC inline void cdqm_readl(const void __iomem *io_base, u32 offset, u32 *val)
{
    *val = readl(io_base + offset);
    TSDRV_PRINT_INFO("read reg(0x%pK) + offset (0x%x) value(0x%x).\n", (void *)(uintptr_t)io_base, offset, *val);
}

void tsdrv_cdqm_set_int_irq(const void __iomem *io_base, u32 irq, u8 ns_num)
{
    cdqm_writel(io_base, STARS_INT_HTIC_CDQ_INT_ID(ns_num), irq);
}

void tsdrv_cdqm_set_int_f2nf_clr(const void __iomem *io_base, u32 cdq_s)
{
    cdqm_writel(io_base, STARS_CDQM_CDQ_INT_CLR2(cdq_s), 1);
}

void tsdrv_cdqm_get_cdq_rdy_num(const void __iomem *io_base, u32 qid, u32 *rdy_num)
{
    cdqm_readl(io_base, STARS_CDQM_CDQ_BATCH_RDY_NUM(CDQ_S(qid), CDQ_PS(qid)), rdy_num);
}

void tsdrv_cdqm_get_cdq_rdy_idx(const void __iomem *io_base, u32 qid, u32 batch_num, u32 *rdy_idx)
{
    cdqm_readl(io_base, STARS_CDQM_CDQ_BATCH_RDY_IDX(CDQ_S(qid), CDQ_PS(qid)), rdy_idx);

    /* hardware will always increase */
    if (batch_num > 0) {
        *rdy_idx = *rdy_idx % batch_num;
    }
}

void tsdrv_cdqm_write_topic_id(const void __iomem *io_base, u32 topic_id)   ///same
{
    cdqm_writel(io_base, STARS_CDQM_CDQ_TOPICID, (topic_id << CDQM_CDQ_TOPIC_ID_BIT));
}

void tsdrv_cdqm_set_batch_rdy_notify(const void __iomem *io_base, u32 qid)
{
    cdqm_writel(io_base, STARS_CDQM_CDQ_BATCH_RDY_NOTIFY(CDQ_S(qid), CDQ_PS(qid)), 1);
}

void tsdrv_cdqm_read_f2nf_int_sts2_l1(const void __iomem *io_base, u32 cdq_s, u32 *status)
{
    u32 sts;
    cdqm_readl(io_base, STARS_CDQM_CDQ_INT_STS2_L1(cdq_s), &sts);
    *status = sts & (1 << CDQM_CDQ_F2NF_INT_STS2_L1_BIT);
}

void tsdrv_cdqm_read_f2nf_int_sts2_l2(const void __iomem *io_base, u32 cdq_s, u32 *status)
{
    cdqm_readl(io_base, STARS_CDQM_CDQ_INT_STS2_L2(cdq_s), status);
    *status &= 0xff;
}

void tsdrv_cdqm_f2nf_int_clr2(const void __iomem *io_base, u32 cdq_s, u32 val)
{
    cdqm_writel(io_base, STARS_CDQM_CDQ_INT_CLR2_L2(cdq_s), val);
}

void tsdrv_cdqm_set_cdq_batch_get(const void __iomem *io_base, u32 qid, u32 en)
{
    cdqm_writel(io_base, STARS_CDQM_CDQ_BATCH_GET(CDQ_S(qid), CDQ_PS(qid)), en);
}

void tsdrv_cdqm_set_batch_get_notify(const void __iomem *io_base, u32 qid, u32 set)
{
    cdqm_writel(io_base, STARS_CDQM_CDQ_BATCH_GET_NOTIFY(CDQ_S(qid), CDQ_PS(qid)), set);
}

void tsdrv_cdqm_set_cdq_pid(const void __iomem *io_base, u32 qid, u32 pid)
{
    cdqm_writel(io_base, STARS_CDQM_CDQ_PID(CDQ_S(qid), CDQ_PS(qid)), pid);
}

void tsdrv_cdqm_cdq_f2nf_unmask_host(const void __iomem *io_base, u32 qid)
{
    u32 mask;

    cdqm_readl(io_base, STARS_CDQM_CDQ_INT_MASK2_L2(CDQ_S(qid)), &mask);
    cdqm_writel(io_base, STARS_CDQM_CDQ_INT_MASK2_L2(CDQ_S(qid)), mask & (~(1 << CDQ_PS(qid))));
}

void tsdrv_cdqm_cdq_f2nf_unmask_ctrl(const void __iomem *io_base, u32 qid)
{
    u32 cdq_g = qid / GRP_CDQ_NUM;
    u32 grp_qid = qid % GRP_CDQ_NUM;
    u32 mask;

    cdqm_readl(io_base, STARS_CDQM_CDQ_F2NF_INT_MASK1(cdq_g), &mask);
    cdqm_writel(io_base, STARS_CDQM_CDQ_F2NF_INT_MASK1(cdq_g), mask & (~(1 << grp_qid)));
}
#endif /* CFG_FEATURE_CDQM */

#else /* TSDRV_UT */

void ut_host_cdq_manager(void)
{
}

#endif /* TSDRV_UT */
