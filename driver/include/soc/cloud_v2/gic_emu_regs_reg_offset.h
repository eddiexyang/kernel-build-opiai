// ****************************************************************************** 
// Copyright     :  Copyright (C) 2021, Hisilicon Technologies Co. Ltd.
// File name     :  gic_emu_regs_reg_offset.h
// Project line  :  Platform And Key Technologies Development
// Department    :  CAD Development Department
// Author        :  xxx
// Version       :  1
// Date          :  2013/3/10
// Description   :  The description of xxx project
// Others        :  Generated automatically by nManager V5.1 
// History       :  xxx 2021/06/02 10:30:30 Create file
// ******************************************************************************

#ifndef __GIC_EMU_REGS_REG_OFFSET_H__
#define __GIC_EMU_REGS_REG_OFFSET_H__

/* GIC_EMU_REGS Base address of Module's Register */
#define SOC_GIC_EMU_REGS_BASE                       (0x0)

/******************************************************************************/
/*                      SOC GIC_EMU_REGS Registers' Definitions                            */
/******************************************************************************/

#define SOC_GIC_EMU_REGS_GIC_EMU_REGS_CNT_CLR_CE_REG   (SOC_GIC_EMU_REGS_BASE + 0xA000) /* ͳ�ƿ��ƼĴ�����CNT_CYC���ͼĴ�����������ź� */
#define SOC_GIC_EMU_REGS_EMU_CLR_PRI_EQ_RDCPT_0_REG    (SOC_GIC_EMU_REGS_BASE + 0xA004) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_CLR_PRI_EQ_RDCPT_1_REG    (SOC_GIC_EMU_REGS_BASE + 0xA104) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_CLR_NOVLD_INCPT_0_REG     (SOC_GIC_EMU_REGS_BASE + 0xA008) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_CLR_NOVLD_INCPT_1_REG     (SOC_GIC_EMU_REGS_BASE + 0xA108) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_SET_NOVLD_INCPT_0_REG     (SOC_GIC_EMU_REGS_BASE + 0xA00C) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_SET_NOVLD_INCPT_1_REG     (SOC_GIC_EMU_REGS_BASE + 0xA10C) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_SET_PRI_GT_RDCPT_0_REG    (SOC_GIC_EMU_REGS_BASE + 0xA010) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_SET_PRI_GT_RDCPT_1_REG    (SOC_GIC_EMU_REGS_BASE + 0xA110) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_INV_VLD_INCPT_0_REG       (SOC_GIC_EMU_REGS_BASE + 0xA014) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_INV_VLD_INCPT_1_REG       (SOC_GIC_EMU_REGS_BASE + 0xA114) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_CLR_1W_0_REG              (SOC_GIC_EMU_REGS_BASE + 0xA018) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_CLR_1W_1_REG              (SOC_GIC_EMU_REGS_BASE + 0xA118) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_SET_ADD_ENABLE_0_REG      (SOC_GIC_EMU_REGS_BASE + 0xA01C) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_SET_ADD_ENABLE_1_REG      (SOC_GIC_EMU_REGS_BASE + 0xA11C) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_CMD_LPI_MOVRTY_DONE_0_REG (SOC_GIC_EMU_REGS_BASE + 0xA020) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_CMD_LPI_MOVRTY_DONE_1_REG (SOC_GIC_EMU_REGS_BASE + 0xA120) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_CMD_LPI_MOV_DONE_0_REG    (SOC_GIC_EMU_REGS_BASE + 0xA024) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_CMD_LPI_MOV_DONE_1_REG    (SOC_GIC_EMU_REGS_BASE + 0xA124) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_LPI_CACHE_RPL_0_REG       (SOC_GIC_EMU_REGS_BASE + 0xA028) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_LPI_CACHE_RPL_1_REG       (SOC_GIC_EMU_REGS_BASE + 0xA128) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_LPI_RTY_FLAG_0_REG        (SOC_GIC_EMU_REGS_BASE + 0xA02C) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_LPI_RTY_FLAG_1_REG        (SOC_GIC_EMU_REGS_BASE + 0xA12C) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_MOV_ID_INCACHE_0_REG      (SOC_GIC_EMU_REGS_BASE + 0xA030) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_MOV_ID_INCACHE_1_REG      (SOC_GIC_EMU_REGS_BASE + 0xA130) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_INV_ID_INCACHE_0_REG      (SOC_GIC_EMU_REGS_BASE + 0xA034) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_INV_ID_INCACHE_1_REG      (SOC_GIC_EMU_REGS_BASE + 0xA134) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_CLR_ID_INCACHE_0_REG      (SOC_GIC_EMU_REGS_BASE + 0xA038) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_CLR_ID_INCACHE_1_REG      (SOC_GIC_EMU_REGS_BASE + 0xA138) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_SET_ID_INCACHE_0_REG      (SOC_GIC_EMU_REGS_BASE + 0xA03C) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_SET_ID_INCACHE_1_REG      (SOC_GIC_EMU_REGS_BASE + 0xA13C) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_SET_CF_INCACHE_0_REG      (SOC_GIC_EMU_REGS_BASE + 0xA040) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_SET_CF_INCACHE_1_REG      (SOC_GIC_EMU_REGS_BASE + 0xA140) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_MOVALL_0_REG              (SOC_GIC_EMU_REGS_BASE + 0xA044) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_MOVALL_1_REG              (SOC_GIC_EMU_REGS_BASE + 0xA144) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_INVALL_0_REG              (SOC_GIC_EMU_REGS_BASE + 0xA048) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_INVALL_1_REG              (SOC_GIC_EMU_REGS_BASE + 0xA148) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_MOV_0_REG                 (SOC_GIC_EMU_REGS_BASE + 0xA04C) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_MOV_1_REG                 (SOC_GIC_EMU_REGS_BASE + 0xA14C) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_INV_0_REG                 (SOC_GIC_EMU_REGS_BASE + 0xA050) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_INV_1_REG                 (SOC_GIC_EMU_REGS_BASE + 0xA150) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_CLR_0_REG                 (SOC_GIC_EMU_REGS_BASE + 0xA054) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_CLR_1_REG                 (SOC_GIC_EMU_REGS_BASE + 0xA154) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_SET_0_REG                 (SOC_GIC_EMU_REGS_BASE + 0xA058) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_SET_1_REG                 (SOC_GIC_EMU_REGS_BASE + 0xA158) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_RTY_MOV_BERR_0_REG        (SOC_GIC_EMU_REGS_BASE + 0xA05C) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_RTY_MOV_BERR_1_REG        (SOC_GIC_EMU_REGS_BASE + 0xA15C) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_BUFF_FULL_0_REG           (SOC_GIC_EMU_REGS_BASE + 0xA060) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_BUFF_FULL_1_REG           (SOC_GIC_EMU_REGS_BASE + 0xA160) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_RTY_MOV_BOK_0_REG         (SOC_GIC_EMU_REGS_BASE + 0xA064) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_RTY_MOV_BOK_1_REG         (SOC_GIC_EMU_REGS_BASE + 0xA164) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_GICR_INVALL_ACK_REG       (SOC_GIC_EMU_REGS_BASE + 0xA068) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_GICR_VCMDB_REQ_REG        (SOC_GIC_EMU_REGS_BASE + 0xA06C) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_VCMD_DOORBELL_GNT_REG     (SOC_GIC_EMU_REGS_BASE + 0xA070) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_VCMD_DOORBELL_PDING_REG   (SOC_GIC_EMU_REGS_BASE + 0xA074) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_GICR_CMDB_REQ_REG         (SOC_GIC_EMU_REGS_BASE + 0xA07C) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_CMDID_ALL_1W_0_REG        (SOC_GIC_EMU_REGS_BASE + 0xA080) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_CMDID_ALL_1W_1_REG        (SOC_GIC_EMU_REGS_BASE + 0xA180) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_CMDB_FIFO_PFULL_0_REG     (SOC_GIC_EMU_REGS_BASE + 0xA084) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_CMDB_FIFO_PFULL_1_REG     (SOC_GIC_EMU_REGS_BASE + 0xA184) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMY_SYNC_CARE_CNT_FULL_0_REG  (SOC_GIC_EMU_REGS_BASE + 0xA088) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMY_SYNC_CARE_CNT_FULL_1_REG  (SOC_GIC_EMU_REGS_BASE + 0xA188) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_MOV_CNT_FULL_0_REG        (SOC_GIC_EMU_REGS_BASE + 0xA08C) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_MOV_CNT_FULL_1_REG        (SOC_GIC_EMU_REGS_BASE + 0xA18C) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_TWO_CMDB_FIFO_PFULL_0_REG (SOC_GIC_EMU_REGS_BASE + 0xA090) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_TWO_CMDB_FIFO_PFULL_1_REG (SOC_GIC_EMU_REGS_BASE + 0xA190) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_SET_ADD_SAMETIME_0_REG    (SOC_GIC_EMU_REGS_BASE + 0xA094) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_SET_ADD_SAMETIME_1_REG    (SOC_GIC_EMU_REGS_BASE + 0xA194) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_STREAM_IRQ_REPLACE_REG    (SOC_GIC_EMU_REGS_BASE + 0xA200) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_STREAM_PFULL_REG          (SOC_GIC_EMU_REGS_BASE + 0xA204) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_STREAM_TOUT_REG           (SOC_GIC_EMU_REGS_BASE + 0xA208) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_MOVBUF_PRRSS_REG          (SOC_GIC_EMU_REGS_BASE + 0xA20C) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_CMDFIFO_PRRSS_REG         (SOC_GIC_EMU_REGS_BASE + 0xA210) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_1_CPUREQ_PRRSS_REG        (SOC_GIC_EMU_REGS_BASE + 0xA214) /* ͳ�ƼĴ��� */
#define SOC_GIC_EMU_REGS_EMU_ALL_CPUREQ_PRRSS_REG      (SOC_GIC_EMU_REGS_BASE + 0xA218) /* ͳ�ƼĴ��� */

#endif // __GIC_EMU_REGS_REG_OFFSET_H__
