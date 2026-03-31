// ******************************************************************************
// Copyright     :  Copyright (C) 2021, Hisilicon Technologies Co. Ltd.
// File name     :  stars_vpc_ns_reg_reg_offset.h
// Project line  :  Platform And Key Technologies Development
// Department    :  CAD Development Department
// Author        :  xxx
// Version       :  1
// Date          :  2020/04/01
// Description   :  The description of xxx project
// Others        :  Generated automatically by nManager V4.2
// History       :  xxx 2021/10/23 09:27:09 Create file
// ******************************************************************************

#ifndef __STARS_VPC_NS_REG_REG_OFFSET_H__
#define __STARS_VPC_NS_REG_REG_OFFSET_H__

/* STARS_VPC_NS_REG Base address of Module's Register */
#define SOC_STARS_VPC_NS_REG_BASE                       (0x2c00000)

/******************************************************************************/
/*                      SOC STARS_VPC_NS_REG Registers' Definitions                            */
/******************************************************************************/

#define SOC_STARS_VPC_NS_REG_STARS_PRIORITY_CTRL0_REG           (SOC_STARS_VPC_NS_REG_BASE + 0x0)
#define SOC_STARS_VPC_NS_REG_STARS_PRIORITY_CTRL1_REG           (SOC_STARS_VPC_NS_REG_BASE + 0x4)
#define SOC_STARS_VPC_NS_REG_STARS_PRIORITY_CTRL2_REG           (SOC_STARS_VPC_NS_REG_BASE + 0x8)
#define SOC_STARS_VPC_NS_REG_STARS_PRIORITY_CTRL3_REG           (SOC_STARS_VPC_NS_REG_BASE + 0xC)
#define SOC_STARS_VPC_NS_REG_STARS_FRIENDLY_CTRL_REG            (SOC_STARS_VPC_NS_REG_BASE + 0x20)
#define SOC_STARS_VPC_NS_REG_STARS_VPC_FSM_SEL_REG              (SOC_STARS_VPC_NS_REG_BASE + 0x810)
#define SOC_STARS_VPC_NS_REG_STARS_VPC_FSM_STATE_REG            (SOC_STARS_VPC_NS_REG_BASE + 0x814)
#define SOC_STARS_VPC_NS_REG_STARS_VPC_REDUNDANT_RSP_REG        (SOC_STARS_VPC_NS_REG_BASE + 0x820)
#define SOC_STARS_VPC_NS_REG_STARS_FREE_VPC_CORE_BITMAP_REG     (SOC_STARS_VPC_NS_REG_BASE + 0x824)
#define SOC_STARS_VPC_NS_REG_STARS_VPC_DFX_CNT_ENABLE_REG       (SOC_STARS_VPC_NS_REG_BASE + 0x830)  /* VPC调度器任务总数统计使能寄存器 */
#define SOC_STARS_VPC_NS_REG_STARS_VPC_DFX_TASK_VLD_CNT_REG     (SOC_STARS_VPC_NS_REG_BASE + 0x834)  /* VPC调度器下发任务总数 */
#define SOC_STARS_VPC_NS_REG_STARS_VPC_DFX_TASK_RSP_CNT_REG     (SOC_STARS_VPC_NS_REG_BASE + 0x838)  /* VPC调度器收到加速器完成响应总数 */
#define SOC_STARS_VPC_NS_REG_STARS_VPC_POOL_ENABLE_CTRL_NS_REG  (SOC_STARS_VPC_NS_REG_BASE + 0x840)
#define SOC_STARS_VPC_NS_REG_STARS_VPC_POOL_DISABLE_CTRL_NS_REG (SOC_STARS_VPC_NS_REG_BASE + 0x880)
#define SOC_STARS_VPC_NS_REG_STARS_VPC_POOL_STATUS0_NS_REG      (SOC_STARS_VPC_NS_REG_BASE + 0x8C0)
#define SOC_STARS_VPC_NS_REG_STARS_VPC_BASE_ADDR_LOW_0_REG      (SOC_STARS_VPC_NS_REG_BASE + 0x900)
#define SOC_STARS_VPC_NS_REG_STARS_VPC_BASE_ADDR_LOW_1_REG      (SOC_STARS_VPC_NS_REG_BASE + 0x910)
#define SOC_STARS_VPC_NS_REG_STARS_VPC_BASE_ADDR_HIGH_0_REG     (SOC_STARS_VPC_NS_REG_BASE + 0x904)
#define SOC_STARS_VPC_NS_REG_STARS_VPC_BASE_ADDR_HIGH_1_REG     (SOC_STARS_VPC_NS_REG_BASE + 0x914)
#define SOC_STARS_VPC_NS_REG_STARS_VPC_ADDR_IS_VIRTUAL_REG      (SOC_STARS_VPC_NS_REG_BASE + 0x908)
#define SOC_STARS_VPC_NS_REG_STARS_VPC_ADDR_SEC_LOCK_REG        (SOC_STARS_VPC_NS_REG_BASE + 0x90C)
#define SOC_STARS_VPC_NS_REG_STARS_VPC_CMDLST_INIT0_0_REG       (SOC_STARS_VPC_NS_REG_BASE + 0x1000)
#define SOC_STARS_VPC_NS_REG_STARS_VPC_CMDLST_INIT0_1_REG       (SOC_STARS_VPC_NS_REG_BASE + 0x1010)
#define SOC_STARS_VPC_NS_REG_STARS_VPC_CMDLST_INIT1_0_REG       (SOC_STARS_VPC_NS_REG_BASE + 0x1004)
#define SOC_STARS_VPC_NS_REG_STARS_VPC_CMDLST_INIT1_1_REG       (SOC_STARS_VPC_NS_REG_BASE + 0x1014)
#define SOC_STARS_VPC_NS_REG_STARS_VPC_CMDLST_KICK_0_REG        (SOC_STARS_VPC_NS_REG_BASE + 0x1104)
#define SOC_STARS_VPC_NS_REG_STARS_VPC_CMDLST_KICK_1_REG        (SOC_STARS_VPC_NS_REG_BASE + 0x1114)

#endif // __STARS_VPC_NS_REG_REG_OFFSET_H__
