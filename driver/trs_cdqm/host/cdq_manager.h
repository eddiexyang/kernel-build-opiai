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

#ifndef CDQ_MANAGER_H
#define CDQ_MANAGER_H

/* interrupt reg */
#define STARS_INT_REG_BASE_ADDR 0x00008000
#define STARS_INT_HTIC_CDQ_INT_ID(ns_num) (0x4 * (ns_num) + 0x09f0)
/* BitFields in STARS_INT_HTIC_INT_VF_MAP(ns_num) */
#define STARS_HTIC_INT_VF_NUM_BIT  11
#define STARS_HTIC_INT_VF_NUM_SIZE 8
#define STARS_HTIC_INT_INT_ID_BIT  0
#define STARS_HTIC_INT_INT_ID_SIZE 11

#define STARS_INT_HTIC_CDQM_F2NF_ID_NS (STARS_INT_HTIC_INT_ID_NS(2))
#define STARS_INT_REG_SIZE (32 * 1024)

/* CDQM_REG */
#define STARS_CDQM_BASE_ADDR 0x1000000
#define STARS_CDQM_SIZE (16 * 1024 * 1024)

/* offeset addr */
/* universal Reg offset */
#define STARS_CDQM_CDQ_TOPICID     0xB00
#define STARS_CDQM_CDQ_KERNEL_TYPE 0xB04
#define STARS_CDQM_ID              0xB08
#define STARS_CDQM_START_CDQ_ID    0xB0C
#define STARS_CDQM_PTR_VA_CFG      0xB10

/* BitFields in CDQM_CDQ_TOPICID */
#define CDQM_CDQ_TOPIC_ID_BIT  12
#define CDQM_CDQ_TOPIC_ID_SIZE 12

#define STARS_CDQM_CDQ_BATCH_GET_NOTIFY(cdq_s, cdq_ps) ((cdq_s) * 0x10000 + (cdq_ps) * 0x1000 + 0x10000C)
/* BitFields in CDQM_CDQ_GET_NOTIFY */
#define CDQM_CDQ_GET_NOTIFY_STATUS_BIT  1
#define CDQM_CDQ_GET_NOTIFY_STATUS_SIZE 1
#define CDQM_CDQ_GET_NOTIFY_SET_BIT     0
#define CDQM_CDQ_GET_NOTIFY_SET_SIZE    1

#define STARS_CDQM_CDQ_BATCH_RDY_NOTIFY(cdq_s, cdq_ps) ((cdq_s) * 0x10000 + (cdq_ps) * 0x1000 + 0x100010)
/* BitFields in CDQM_CDQ_BATCH_RDY_NOTIFY */
#define CDQM_CDQ_BATCH_RDY_NOTIFY_STATUS_BIT 1
#define CDQM_CDQ_GET_BATCH_RDY_STATUS_SIZE   1
#define CDQM_CDQ_GET_BATCH_RDY_SET_BIT       0
#define CDQM_CDQ_GET_BATCH_RDY_SET_SIZE      1

#define STARS_CDQM_CDQ_SIZE(cdq_s, cdq_ps) ((cdq_s) * 0x10000 + (cdq_ps) * 0x1000 + 0x100014)
/* BitField in STARS_CDQM_CDQ_SIZE */
#define CDQM_BATCH_SIZE_BIT    0
#define CDQM_BATCH_SIZE_SIZE   16
#define CDQM_RTCDQ_LENGTH_BIT  16
#define CDQM_RTCDQ_LENGTH_SIZE 16

#define STARS_CDQM_CDQ_BATCH_RDY_NUM(cdq_s, cdq_ps) ((cdq_s) * 0x10000 + (cdq_ps) * 0x1000 + 0x100018)
#define STARS_CDQM_CDQ_BATCH_RDY_IDX(cdq_s, cdq_ps) ((cdq_s) * 0x10000 + (cdq_ps) * 0x1000 + 0x10001C)
#define STARS_CDQM_CDQ_BATCH_GET(cdq_s, cdq_ps)     ((cdq_s) * 0x10000 + (cdq_ps) * 0x1000 + 0x100020)
#define STARS_CDQM_CDQ_PID(cdq_s, cdq_ps)           ((cdq_s) * 0x10000 + (cdq_ps) * 0x1000 + 0x100024)
#define STARS_CDQM_CDQ_TOPIC_INFO(cdq_s, cdq_ps)    ((cdq_s) * 0x10000 + (cdq_ps) * 0x1000 + 0x100028)
/* BitFeils in CDQM_CDQ_TOPIC_INFO(cdq_id) */
#define CDQM_CDQ_QOS_BIT     12
#define CDQM_CDQ_QOS_SIZE    4
#define CDQM_CDQ_GRP_ID_BIT  6
#define CDQM_CDQ_GRP_ID_SIZE 6
#define CDQM_CDQ_VFID_BIT    0
#define CDQM_CDQ_VFID_SIZE   6

#define STARS_CDQM_CDQ_INT_CLR2(cdq_s)      ((cdq_s) * 0x10000 + 0x108014)

#define STARS_CDQM_CDQ_INT_STS2_L1(cdq_s)   ((cdq_s) * 0x10000 + 0x108020)
/* BitField in STARS_CDQM_CDQ_INT_STS2_L1 */
#define CDQM_CDQ_F2NF_INT_STS2_L1_BIT  0
#define CDQM_CDQ_F2NF_INT_STS2_L1_SIZE 1

#define STARS_CDQM_CDQ_INT_STS2_L2(cdq_s)   ((cdq_s) * 0x10000 + 0x108084)
/* BitField in STARS_CDQM_CDQ_INT_STS2_L2 */
#define CDQM_CDQ_F2NF_INT_STS2_L2_BIT  0
#define CDQM_CDQ_F2NF_INT_STS2_L2_SIZE 8
#define STARS_CDQM_CDQ_INT_CLR2_L2(cdq_s)   ((cdq_s) * 0x10000 + 0x10808C)
/* BitField in STARS_CDQM_CDQ_INT_CLR2_L2 */
#define CDQM_CDQ_F2NF_INT_CLR2_L2_BIT  0
#define CDQM_CDQ_F2NF_INT_CLR2_L2_SIZE 8

#define CDQM_CDQ_REG_MASK(bit_start, bit_end) ((1 << (bit_end)) - (1 << (bit_start)))
#define CDQM_CLEAR_REG_NUM 0xFFFFFFFF

#define CDQM_CDQ_BIT(name) (1 << CDQM_CDQ_##name##_BIT)
#define CDQM_CDQ_MASK(name) ((1 << CDQM_CDQ_##name##_SIZE - 1) << CDQM_CDQ_##name##_BIT)


void tsdrv_cdqm_set_int_f2nf_clr(const void __iomem *io_base, u32 cdq_s);
void tsdrv_cdqm_read_f2nf_int_sts2_l2(const void __iomem *io_base, u32 cdq_s, u32 *status);
void tsdrv_cdqm_f2nf_int_clr2(const void __iomem *io_base, u32 cdq_s, u32 val);

void tsdrv_cdqm_set_int_irq(const void __iomem *io_base, u32 irq, u8 ns_num);

void tsdrv_cdqm_set_cdq_batch_get(const void __iomem *io_base, u32 qid, u32 en);
void tsdrv_cdqm_get_cdq_rdy_num(const void __iomem *io_base, u32 qid, u32 *rdy_num);
void tsdrv_cdqm_get_cdq_rdy_idx(const void __iomem *io_base, u32 qid, u32 batch_num, u32 *rdy_idx);
void tsdrv_cdqm_set_batch_get_notify(const void __iomem *io_base, u32 qid, u32 set);
void tsdrv_cdqm_write_topic_id(const void __iomem *io_base, u32 topic_id);
void tsdrv_cdqm_set_batch_rdy_notify(const void __iomem *io_base, u32 qid);
void tsdrv_cdqm_f2nf_int_mask_l1(const void __iomem *io_base);
void tsdrv_cdqm_f2nf_int_unmask_l1(const void __iomem *io_base);

#endif
