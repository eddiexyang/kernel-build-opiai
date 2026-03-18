/*
* Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
* Create: 2022-7-9
*/
#include <linux/io.h>

#include "trs_pub_def.h"
#include "cdq_manager.h"
#include "cdqm_core.h"

#define CDQM_F2NF_NS_NUM 2

static inline void cdqm_writel(const void __iomem *io_base, u32 offset, u32 val)
{
    trs_info("write reg(0x%pK) + offset (0x%x) value(0x%x).\n", (void *)(uintptr_t)io_base, offset, val);
    writel(val, (volatile void*)io_base + offset);
}

static inline void cdqm_readl(const void __iomem *io_base, u32 offset, u32 *val)
{
    *val = readl(io_base + offset);
    trs_info("read reg(0x%pK) + offset (0x%x) value(0x%x).\n", (void *)(uintptr_t)io_base, offset, *val);
}

void tsdrv_cdqm_set_int_irq(const void __iomem *io_base, u32 irq, u8 ns_num)
{
}

void tsdrv_cdqm_set_int_f2nf_clr(const void __iomem *io_base, u32 cdq_s)
{
    cdqm_writel(io_base, STARS_CDQM_CDQ_INT_CLR2(cdq_s), 1);
}

void tsdrv_cdqm_get_cdq_rdy_num(const void __iomem *io_base, u32 qid, u32 *rdy_num)
{
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
}

void tsdrv_cdqm_set_batch_rdy_notify(const void __iomem *io_base, u32 qid)
{
}

void tsdrv_cdqm_read_f2nf_int_sts2_l2(const void __iomem *io_base, u32 cdq_s, u32 *status)
{
    cdqm_readl(io_base, STARS_CDQM_CDQ_INT_STS2_L2(cdq_s), status);
    *status &= 0xff;
#ifdef EMU_ST
    *status = 0x1;
#endif
}

void tsdrv_cdqm_f2nf_int_clr2(const void __iomem *io_base, u32 cdq_s, u32 val)
{
    cdqm_writel(io_base, STARS_CDQM_CDQ_INT_CLR2_L2(cdq_s), val);
}

void tsdrv_cdqm_set_cdq_batch_get(const void __iomem *io_base, u32 qid, u32 en)
{
}

void tsdrv_cdqm_set_batch_get_notify(const void __iomem *io_base, u32 qid, u32 set)
{
    cdqm_writel(io_base, STARS_CDQM_CDQ_BATCH_GET_NOTIFY(CDQ_S(qid), CDQ_PS(qid)), set);
}

void tsdrv_cdqm_f2nf_int_mask_l1(const void __iomem *io_base)
{
}
void tsdrv_cdqm_f2nf_int_unmask_l1(const void __iomem *io_base)
{
}

