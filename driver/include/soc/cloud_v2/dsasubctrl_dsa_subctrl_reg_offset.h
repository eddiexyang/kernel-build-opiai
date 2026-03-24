/*
 * Copyright     :  Copyright (C) 2021, Huawei Technologies Co. Ltd.
 * File name     :  dsa_subctrl_reg_offset.h
 * Project line  :  ICT
 * Department    :  ICT Processor Chipset Development Dep
 * Author        :  xxx
 * Version       :  1.0
 * Date          :  
 * Description   :  The description of AI
 * Others        :  Generated automatically by nManager V5.1 
 * History       :  xxx 2021/09/14 20:47:06 Create file
 */

#ifndef __DSA_SUBCTRL_REG_OFFSET_H__
#define __DSA_SUBCTRL_REG_OFFSET_H__

/* DSA_SUBCTRL Base address of Module's Register */
#define DSA_SUBCTRL_BASE                       (0x0)

/******************************************************************************/
/*                      SOC DSA_SUBCTRL Registers' Definitions                            */
/******************************************************************************/

#define DSA_SUBCTRL_SC_CLK_PI_MON_SEL_REG            (DSA_SUBCTRL_BASE + 0x0)    /* PI MONITOR工作时钟频率选择寄存器 */
#define DSA_SUBCTRL_SC_SMMU_ICG_EN_REG               (DSA_SUBCTRL_BASE + 0x300)  /* smmu时钟使能寄存器 */
#define DSA_SUBCTRL_SC_SMMU_ICG_DIS_REG              (DSA_SUBCTRL_BASE + 0x304)  /* smmu时钟禁止寄存器 */
#define DSA_SUBCTRL_SC_DSA_ICG_EN_REG                (DSA_SUBCTRL_BASE + 0x308)  /* DSA时钟使能寄存器 */
#define DSA_SUBCTRL_SC_DSA_ICG_DIS_REG               (DSA_SUBCTRL_BASE + 0x30C)  /* DSA时钟禁止寄存器 */
#define DSA_SUBCTRL_SC_FTE_ICG_EN_REG                (DSA_SUBCTRL_BASE + 0x310)  /* FTE时钟使能寄存器 */
#define DSA_SUBCTRL_SC_FTE_ICG_DIS_REG               (DSA_SUBCTRL_BASE + 0x314)  /* FTE时钟禁止寄存器 */
#define DSA_SUBCTRL_SC_SYS_SRAM_ICG_EN_REG           (DSA_SUBCTRL_BASE + 0x318)  /* SYS_SRAM时钟使能寄存器 */
#define DSA_SUBCTRL_SC_SYS_SRAM_ICG_DIS_REG          (DSA_SUBCTRL_BASE + 0x31C)  /* SYS_SRAM时钟禁止寄存器 */
#define DSA_SUBCTRL_SC_PEH_ICG_EN_REG                (DSA_SUBCTRL_BASE + 0x320)  /* PEH时钟使能寄存器 */
#define DSA_SUBCTRL_SC_PEH_ICG_DIS_REG               (DSA_SUBCTRL_BASE + 0x324)  /* PEH时钟禁止寄存器 */
#define DSA_SUBCTRL_SC_PM_CLK_EN_REG                 (DSA_SUBCTRL_BASE + 0x328)  /* PM时钟使能寄存器 */
#define DSA_SUBCTRL_SC_PM_CLK_DIS_REG                (DSA_SUBCTRL_BASE + 0x32C)  /* PM时钟禁止寄存器 */
#define DSA_SUBCTRL_SC_PI_MONITOR_ICG_EN_REG         (DSA_SUBCTRL_BASE + 0x330)  /* PI MONITOR工作时钟使能寄存器 */
#define DSA_SUBCTRL_SC_PI_MONITOR_ICG_DIS_REG        (DSA_SUBCTRL_BASE + 0x334)  /* PI MONITOR工作时钟禁止寄存器 */
#define DSA_SUBCTRL_SC_PI_MONITOR_APB_ICG_EN_REG     (DSA_SUBCTRL_BASE + 0x338)  /* PI MONITOR APB接口时钟使能寄存器 */
#define DSA_SUBCTRL_SC_PI_MONITOR_APB_ICG_DIS_REG    (DSA_SUBCTRL_BASE + 0x33C)  /* PI MONITOR APB接口时钟禁止寄存器 */
#define DSA_SUBCTRL_SC_ECO_ICG_EN_REG                (DSA_SUBCTRL_BASE + 0x9F0)  /* ECO预留时钟使能寄存器。 */
#define DSA_SUBCTRL_SC_ECO_ICG_DIS_REG               (DSA_SUBCTRL_BASE + 0x9F4)  /* ECO预留时钟禁止寄存器。 */
#define DSA_SUBCTRL_SC_PM_RESET_REQ_REG              (DSA_SUBCTRL_BASE + 0xA10)  /* PM软复位请求控制寄存器 */
#define DSA_SUBCTRL_SC_PM_RESET_DREQ_REG             (DSA_SUBCTRL_BASE + 0xA14)  /* PM软复位去请求控制寄存器 */
#define DSA_SUBCTRL_SC_DSA_RESET_REQ_REG             (DSA_SUBCTRL_BASE + 0xA20)  /* DSA软复位请求控制寄存器 */
#define DSA_SUBCTRL_SC_DSA_RESET_DREQ_REG            (DSA_SUBCTRL_BASE + 0xA24)  /* DSA软复位去请求控制寄存器 */
#define DSA_SUBCTRL_SC_FTE_RESET_REQ_REG             (DSA_SUBCTRL_BASE + 0xA28)  /* FTE软复位请求控制寄存器 */
#define DSA_SUBCTRL_SC_FTE_RESET_DREQ_REG            (DSA_SUBCTRL_BASE + 0xA2C)  /* FTE软复位去请求控制寄存器 */
#define DSA_SUBCTRL_SC_SYS_SRAM_RESET_REQ_REG        (DSA_SUBCTRL_BASE + 0xA30)  /* SYS_SRAM软复位请求控制寄存器 */
#define DSA_SUBCTRL_SC_SYS_SRAM_RESET_DREQ_REG       (DSA_SUBCTRL_BASE + 0xA34)  /* SYS_SRAM软复位去请求控制寄存器 */
#define DSA_SUBCTRL_SC_PEH_RESET_REQ_REG             (DSA_SUBCTRL_BASE + 0xA38)  /* PEH软复位请求控制寄存器 */
#define DSA_SUBCTRL_SC_PEH_RESET_DREQ_REG            (DSA_SUBCTRL_BASE + 0xA3C)  /* PEH软复位去请求控制寄存器 */
#define DSA_SUBCTRL_SC_PIPE_RESET_REQ_REG            (DSA_SUBCTRL_BASE + 0xA40)  /* PIPE软复位请求控制寄存器 */
#define DSA_SUBCTRL_SC_PIPE_RESET_DREQ_REG           (DSA_SUBCTRL_BASE + 0xA44)  /* PIPE软复位去请求控制寄存器 */
#define DSA_SUBCTRL_SC_READ_RESET_REQ_REG            (DSA_SUBCTRL_BASE + 0xA48)  /* READ软复位请求控制寄存器 */
#define DSA_SUBCTRL_SC_READ_RESET_DREQ_REG           (DSA_SUBCTRL_BASE + 0xA4C)  /* READ软复位去请求控制寄存器 */
#define DSA_SUBCTRL_SC_PI_MONITOR_RESET_REQ_REG      (DSA_SUBCTRL_BASE + 0xA50)  /* PI Monitor软复位请求控制寄存器 */
#define DSA_SUBCTRL_SC_PI_MONITOR_RESET_DREQ_REG     (DSA_SUBCTRL_BASE + 0xA54)  /* PI Monitor软复位去请求控制寄存器 */
#define DSA_SUBCTRL_SC_PI_MONITOR_APB_RESET_REQ_REG  (DSA_SUBCTRL_BASE + 0xA58)  /* PI Monitor APB接口软复位请求控制寄存器 */
#define DSA_SUBCTRL_SC_PI_MONITOR_APB_RESET_DREQ_REG (DSA_SUBCTRL_BASE + 0xA5C)  /* PI Monitor APB接口软复位去请求控制寄存器 */
#define DSA_SUBCTRL_SC_ECO_RESET_REQ_REG             (DSA_SUBCTRL_BASE + 0xFF0)  /* ECO预留软复位请求寄存器。 */
#define DSA_SUBCTRL_SC_ECO_RESET_DREQ_REG            (DSA_SUBCTRL_BASE + 0xFF4)  /* ECO预留软复位去请求寄存器。 */
#define DSA_SUBCTRL_SC_DISPATCH0_ERRRSP_REG          (DSA_SUBCTRL_BASE + 0x2060) /* dispatch的ERR响应类型控制寄存器 */
#define DSA_SUBCTRL_SC_HPM_EN_REG                    (DSA_SUBCTRL_BASE + 0x2500) /* HPM使能寄存器 */
#define DSA_SUBCTRL_SC_HPM_DIV_REG                   (DSA_SUBCTRL_BASE + 0x2504) /* HPM DIV寄存器 */
#define DSA_SUBCTRL_SC_MEM_CTRL_SMMU_REG             (DSA_SUBCTRL_BASE + 0x3000) /* SMMU MEM控制寄存器 */
#define DSA_SUBCTRL_SC_MEM_CTRL_PI_MONITOR_REG       (DSA_SUBCTRL_BASE + 0x3004) /* PI MONITOR MEM控制寄存器 */
#define DSA_SUBCTRL_SC_MEM_CTRL_SP_SYS_SRAM_REG      (DSA_SUBCTRL_BASE + 0x3008) /* SYS SRAM MEM控制寄存器 */
#define DSA_SUBCTRL_SC_SMMU_ICG_ST_REG               (DSA_SUBCTRL_BASE + 0x5300) /* smmu时钟状态寄存器 */
#define DSA_SUBCTRL_SC_DSA_ICG_ST_REG                (DSA_SUBCTRL_BASE + 0x5308) /* DSA时钟状态寄存器 */
#define DSA_SUBCTRL_SC_FTE_ICG_ST_REG                (DSA_SUBCTRL_BASE + 0x5310) /* FTE时钟状态寄存器 */
#define DSA_SUBCTRL_SC_SYS_SRAM_ICG_ST_REG           (DSA_SUBCTRL_BASE + 0x5318) /* SYS_SRAM时钟状态寄存器 */
#define DSA_SUBCTRL_SC_PEH_ICG_ST_REG                (DSA_SUBCTRL_BASE + 0x5320) /* PEH时钟状态寄存器 */
#define DSA_SUBCTRL_SC_PM_CLK_ST_REG                 (DSA_SUBCTRL_BASE + 0x5328) /* PM时钟状态寄存器 */
#define DSA_SUBCTRL_SC_PI_MONITOR_ICG_ST_REG         (DSA_SUBCTRL_BASE + 0x5330) /* PI MONITOR工作时钟状态寄存器 */
#define DSA_SUBCTRL_SC_PI_MONITOR_APB_ICG_ST_REG     (DSA_SUBCTRL_BASE + 0x5338) /* PI MONITOR APB接口时钟状态寄存器 */
#define DSA_SUBCTRL_SC_ECO_ICG_ST_REG                (DSA_SUBCTRL_BASE + 0x59F0) /* ECO预留时钟状态寄存器 */
#define DSA_SUBCTRL_SC_PM_RESET_ST_REG               (DSA_SUBCTRL_BASE + 0x5A10) /* PM的当前软复位状态 */
#define DSA_SUBCTRL_SC_DSA_RESET_ST_REG              (DSA_SUBCTRL_BASE + 0x5A20) /* DSA复位状态寄存器 */
#define DSA_SUBCTRL_SC_FTE_RESET_ST_REG              (DSA_SUBCTRL_BASE + 0x5A28) /* FTE复位状态寄存器 */
#define DSA_SUBCTRL_SC_SYS_SRAM_RESET_ST_REG         (DSA_SUBCTRL_BASE + 0x5A30) /* SYS_SRAM复位状态寄存器 */
#define DSA_SUBCTRL_SC_PEH_RESET_ST_REG              (DSA_SUBCTRL_BASE + 0x5A38) /* PEH复位状态寄存器 */
#define DSA_SUBCTRL_SC_PIPE_RESET_ST_REG             (DSA_SUBCTRL_BASE + 0x5A40) /* PIPE复位状态寄存器 */
#define DSA_SUBCTRL_SC_READ_RESET_ST_REG             (DSA_SUBCTRL_BASE + 0x5A48) /* READ复位状态寄存器 */
#define DSA_SUBCTRL_SC_PI_MONITOR_RESET_ST_REG       (DSA_SUBCTRL_BASE + 0x5A50) /* PI_MONITOR复位状态寄存器 */
#define DSA_SUBCTRL_SC_PI_MONITOR_APB_RESET_ST_REG   (DSA_SUBCTRL_BASE + 0x5A58) /* PI MONITOR APB接口复位状态寄存器 */
#define DSA_SUBCTRL_SC_ECO_RESET_ST_REG              (DSA_SUBCTRL_BASE + 0x5FF0) /* ECO预留的复位状态寄存器。 */
#define DSA_SUBCTRL_HPM_PC_0_ORG_REG                 (DSA_SUBCTRL_BASE + 0x6500) /* hpm 0v状态 */
#define DSA_SUBCTRL_HPM_PC_1_ORG_REG                 (DSA_SUBCTRL_BASE + 0x6504) /* hpm 0v状态 */
#define DSA_SUBCTRL_HPM_PC_VALID_REG                 (DSA_SUBCTRL_BASE + 0x6508) /* hpm 有效状态 */
#define DSA_SUBCTRL_SC_PM_EN_REG                     (DSA_SUBCTRL_BASE + 0x6600) /* PM EN */
#define DSA_SUBCTRL_SC_PM_SYS_CFG_L_REG              (DSA_SUBCTRL_BASE + 0x6604) /* PM_SYS_CFG_L */
#define DSA_SUBCTRL_SC_PM_SYS_CFG_H_REG              (DSA_SUBCTRL_BASE + 0x6608) /* PM_SYS_CFG_H */
#define DSA_SUBCTRL_SC_PM_DATA_RPT_REG               (DSA_SUBCTRL_BASE + 0x660C) /* PM_DATA_RPT */
#define DSA_SUBCTRL_CFG_VERSION_REG                  (DSA_SUBCTRL_BASE + 0xE0A0) /* VERSION寄存器 */
#define DSA_SUBCTRL_SC_SYSCTRL_LOCK_REG              (DSA_SUBCTRL_BASE + 0xF100) /* 系统控制器锁定寄存器。（注意：1，此寄存器空间必须是安全访问才能成功） */
#define DSA_SUBCTRL_SC_SYSCTRL_UNLOCK_REG            (DSA_SUBCTRL_BASE + 0xF110) /* 系统控制器解锁寄存器。（注意：1，此寄存器空间必须是安全访问才能成功） */
#define DSA_SUBCTRL_SC_SMMU_SEC_LOCK_REG             (DSA_SUBCTRL_BASE + 0xF408) /* SMMU SEC LOCK寄存器 */
#define DSA_SUBCTRL_ECO_RSV0_REG                     (DSA_SUBCTRL_BASE + 0xFF0C) /* ECO 寄存器0 */
#define DSA_SUBCTRL_ECO_RSV1_REG                     (DSA_SUBCTRL_BASE + 0xFF10) /* ECO 寄存器1 */
#define DSA_SUBCTRL_ECO_RSV2_REG                     (DSA_SUBCTRL_BASE + 0xFF14) /* ECO 寄存器2 */
#define DSA_SUBCTRL_ECO_RSV3_REG                     (DSA_SUBCTRL_BASE + 0xFF18) /* ECO 寄存器3 */
#define DSA_SUBCTRL_FPGA_VER_REG                     (DSA_SUBCTRL_BASE + 0xFFFC) /* FPGA版本寄存器。 */

#endif // __DSA_SUBCTRL_REG_OFFSET_H__
