/*
 * Copyright     :  Copyright (C) 2021, Huawei Technologies Co. Ltd.
 * File name     :  gic_emu_regs_reg_offset.h
 * Project line  :  Platform And Key Technologies Development
 * Department    :  CAD Development Department
 * Author        :  xxx
 * Version       :  1
 * Date          :  2013/3/10
 * Description   :  The description of xxx project
 * Others        :  Generated automatically by nManager V5.1 
 * History       :  xxx 2021/10/25 08:35:56 Create file
 */

#ifndef __GIC_EMU_REGS_REG_OFFSET_H__
#define __GIC_EMU_REGS_REG_OFFSET_H__

/* GIC_EMU_REGS Base address of Module's Register */
#define SOC_GIC_EMU_REGS_BASE                       (0x0)

/******************************************************************************/
/*                      SOC GIC_EMU_REGS Registers' Definitions                            */
/******************************************************************************/

#define SOC_GIC_EMU_REGS_GIC_EMU_REGS_CNT_CLR_CE_REG   (SOC_GIC_EMU_REGS_BASE + 0xA000) /* 统计控制寄存器与CNT_CYC类型寄存器读清控制信号 */
#define SOC_GIC_EMU_REGS_EMU_CLR_PRI_EQ_RDCPT_0_REG    (SOC_GIC_EMU_REGS_BASE + 0xA004) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_CLR_PRI_EQ_RDCPT_1_REG    (SOC_GIC_EMU_REGS_BASE + 0xA104) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_CLR_NOVLD_INCPT_0_REG     (SOC_GIC_EMU_REGS_BASE + 0xA008) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_CLR_NOVLD_INCPT_1_REG     (SOC_GIC_EMU_REGS_BASE + 0xA108) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_SET_NOVLD_INCPT_0_REG     (SOC_GIC_EMU_REGS_BASE + 0xA00C) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_SET_NOVLD_INCPT_1_REG     (SOC_GIC_EMU_REGS_BASE + 0xA10C) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_SET_PRI_GT_RDCPT_0_REG    (SOC_GIC_EMU_REGS_BASE + 0xA010) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_SET_PRI_GT_RDCPT_1_REG    (SOC_GIC_EMU_REGS_BASE + 0xA110) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_INV_VLD_INCPT_0_REG       (SOC_GIC_EMU_REGS_BASE + 0xA014) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_INV_VLD_INCPT_1_REG       (SOC_GIC_EMU_REGS_BASE + 0xA114) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_CLR_1W_0_REG              (SOC_GIC_EMU_REGS_BASE + 0xA018) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_CLR_1W_1_REG              (SOC_GIC_EMU_REGS_BASE + 0xA118) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_SET_ADD_ENABLE_0_REG      (SOC_GIC_EMU_REGS_BASE + 0xA01C) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_SET_ADD_ENABLE_1_REG      (SOC_GIC_EMU_REGS_BASE + 0xA11C) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_CMD_LPI_MOVRTY_DONE_0_REG (SOC_GIC_EMU_REGS_BASE + 0xA020) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_CMD_LPI_MOVRTY_DONE_1_REG (SOC_GIC_EMU_REGS_BASE + 0xA120) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_CMD_LPI_MOV_DONE_0_REG    (SOC_GIC_EMU_REGS_BASE + 0xA024) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_CMD_LPI_MOV_DONE_1_REG    (SOC_GIC_EMU_REGS_BASE + 0xA124) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_LPI_CACHE_RPL_0_REG       (SOC_GIC_EMU_REGS_BASE + 0xA028) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_LPI_CACHE_RPL_1_REG       (SOC_GIC_EMU_REGS_BASE + 0xA128) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_LPI_RTY_FLAG_0_REG        (SOC_GIC_EMU_REGS_BASE + 0xA02C) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_LPI_RTY_FLAG_1_REG        (SOC_GIC_EMU_REGS_BASE + 0xA12C) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_MOV_ID_INCACHE_0_REG      (SOC_GIC_EMU_REGS_BASE + 0xA030) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_MOV_ID_INCACHE_1_REG      (SOC_GIC_EMU_REGS_BASE + 0xA130) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_INV_ID_INCACHE_0_REG      (SOC_GIC_EMU_REGS_BASE + 0xA034) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_INV_ID_INCACHE_1_REG      (SOC_GIC_EMU_REGS_BASE + 0xA134) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_CLR_ID_INCACHE_0_REG      (SOC_GIC_EMU_REGS_BASE + 0xA038) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_CLR_ID_INCACHE_1_REG      (SOC_GIC_EMU_REGS_BASE + 0xA138) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_SET_ID_INCACHE_0_REG      (SOC_GIC_EMU_REGS_BASE + 0xA03C) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_SET_ID_INCACHE_1_REG      (SOC_GIC_EMU_REGS_BASE + 0xA13C) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_SET_CF_INCACHE_0_REG      (SOC_GIC_EMU_REGS_BASE + 0xA040) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_SET_CF_INCACHE_1_REG      (SOC_GIC_EMU_REGS_BASE + 0xA140) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_MOVALL_0_REG              (SOC_GIC_EMU_REGS_BASE + 0xA044) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_MOVALL_1_REG              (SOC_GIC_EMU_REGS_BASE + 0xA144) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_INVALL_0_REG              (SOC_GIC_EMU_REGS_BASE + 0xA048) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_INVALL_1_REG              (SOC_GIC_EMU_REGS_BASE + 0xA148) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_MOV_0_REG                 (SOC_GIC_EMU_REGS_BASE + 0xA04C) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_MOV_1_REG                 (SOC_GIC_EMU_REGS_BASE + 0xA14C) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_INV_0_REG                 (SOC_GIC_EMU_REGS_BASE + 0xA050) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_INV_1_REG                 (SOC_GIC_EMU_REGS_BASE + 0xA150) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_CLR_0_REG                 (SOC_GIC_EMU_REGS_BASE + 0xA054) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_CLR_1_REG                 (SOC_GIC_EMU_REGS_BASE + 0xA154) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_SET_0_REG                 (SOC_GIC_EMU_REGS_BASE + 0xA058) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_SET_1_REG                 (SOC_GIC_EMU_REGS_BASE + 0xA158) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_RTY_MOV_BERR_0_REG        (SOC_GIC_EMU_REGS_BASE + 0xA05C) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_RTY_MOV_BERR_1_REG        (SOC_GIC_EMU_REGS_BASE + 0xA15C) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_BUFF_FULL_0_REG           (SOC_GIC_EMU_REGS_BASE + 0xA060) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_BUFF_FULL_1_REG           (SOC_GIC_EMU_REGS_BASE + 0xA160) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_RTY_MOV_BOK_0_REG         (SOC_GIC_EMU_REGS_BASE + 0xA064) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_RTY_MOV_BOK_1_REG         (SOC_GIC_EMU_REGS_BASE + 0xA164) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_GICR_INVALL_ACK_REG       (SOC_GIC_EMU_REGS_BASE + 0xA068) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_GICR_VCMDB_REQ_REG        (SOC_GIC_EMU_REGS_BASE + 0xA06C) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_VCMD_DOORBELL_GNT_REG     (SOC_GIC_EMU_REGS_BASE + 0xA070) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_VCMD_DOORBELL_PDING_REG   (SOC_GIC_EMU_REGS_BASE + 0xA074) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_GICR_CMDB_REQ_REG         (SOC_GIC_EMU_REGS_BASE + 0xA07C) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_CMDID_ALL_1W_0_REG        (SOC_GIC_EMU_REGS_BASE + 0xA080) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_CMDID_ALL_1W_1_REG        (SOC_GIC_EMU_REGS_BASE + 0xA180) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_CMDB_FIFO_PFULL_0_REG     (SOC_GIC_EMU_REGS_BASE + 0xA084) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_CMDB_FIFO_PFULL_1_REG     (SOC_GIC_EMU_REGS_BASE + 0xA184) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMY_SYNC_CARE_CNT_FULL_0_REG  (SOC_GIC_EMU_REGS_BASE + 0xA088) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMY_SYNC_CARE_CNT_FULL_1_REG  (SOC_GIC_EMU_REGS_BASE + 0xA188) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_MOV_CNT_FULL_0_REG        (SOC_GIC_EMU_REGS_BASE + 0xA08C) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_MOV_CNT_FULL_1_REG        (SOC_GIC_EMU_REGS_BASE + 0xA18C) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_TWO_CMDB_FIFO_PFULL_0_REG (SOC_GIC_EMU_REGS_BASE + 0xA090) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_TWO_CMDB_FIFO_PFULL_1_REG (SOC_GIC_EMU_REGS_BASE + 0xA190) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_SET_ADD_SAMETIME_0_REG    (SOC_GIC_EMU_REGS_BASE + 0xA094) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_SET_ADD_SAMETIME_1_REG    (SOC_GIC_EMU_REGS_BASE + 0xA194) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_STREAM_IRQ_REPLACE_REG    (SOC_GIC_EMU_REGS_BASE + 0xA200) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_STREAM_PFULL_REG          (SOC_GIC_EMU_REGS_BASE + 0xA204) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_STREAM_TOUT_REG           (SOC_GIC_EMU_REGS_BASE + 0xA208) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_MOVBUF_PRRSS_REG          (SOC_GIC_EMU_REGS_BASE + 0xA20C) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_CMDFIFO_PRRSS_REG         (SOC_GIC_EMU_REGS_BASE + 0xA210) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_1_CPUREQ_PRRSS_REG        (SOC_GIC_EMU_REGS_BASE + 0xA214) /* 统计寄存器 */
#define SOC_GIC_EMU_REGS_EMU_ALL_CPUREQ_PRRSS_REG      (SOC_GIC_EMU_REGS_BASE + 0xA218) /* 统计寄存器 */

#endif // __GIC_EMU_REGS_REG_OFFSET_H__
