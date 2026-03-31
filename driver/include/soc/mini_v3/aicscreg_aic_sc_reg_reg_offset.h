/*
 * Copyright     :  Copyright (C) 2022, Huawei Technologies Co. Ltd.
 * File name     :  aic_sc_reg_reg_offset.h
 * Project line  :  Platform And Key Technologies Development
 * Department    :  CAD Development Department
 * Author        :  xxx
 * Version       :  1
 * Date          :  2013/3/10
 * Description   :  The description of xxx project
 * Others        :  Generated automatically by nManager V5.1 
 * History       :  xxx 2022/08/19 14:34:37 Create file
 */

#ifndef __AIC_SC_REG_REG_OFFSET_H__
#define __AIC_SC_REG_REG_OFFSET_H__

/* AIC_SC_REG Base address of Module's Register */
#define AIC_SC_REG_BASE                       (0x0)

/******************************************************************************/
/*                      xxx AIC_SC_REG Registers' Definitions                            */
/******************************************************************************/

#define AIC_SC_REG_RUN_STALL_REG                        (AIC_SC_REG_BASE + 0x0)    /* CCU, IFU */
#define AIC_SC_REG_DBG_CTRL_0_REG                       (AIC_SC_REG_BASE + 0x8)    /* DBG, CCU */
#define AIC_SC_REG_DBG_CTRL_1_REG                       (AIC_SC_REG_BASE + 0xC)    /* DBG, CCU */
#define AIC_SC_REG_RST_AND_INIT_CTRL_STATUS_REG         (AIC_SC_REG_BASE + 0x10)   /* all */
#define AIC_SC_REG_FAST_PATH_CTRL_REG                   (AIC_SC_REG_BASE + 0x18)   /* SC */
#define AIC_SC_REG_AI_CORE_INT_REG                      (AIC_SC_REG_BASE + 0x20)   /* SC, CCU */
#define AIC_SC_REG_AI_CORE_INT_MASK_REG                 (AIC_SC_REG_BASE + 0x28)   /* SC */
#define AIC_SC_REG_RST_CNT_REG                          (AIC_SC_REG_BASE + 0x30)   /* SC */
#define AIC_SC_REG_ECC_EN_REG                           (AIC_SC_REG_BASE + 0x38)   /* MTE, CUBE, VEC */
#define AIC_SC_REG_AXI_CLAMP_CTRL_REG                   (AIC_SC_REG_BASE + 0x40)   
#define AIC_SC_REG_AXI_CLAMP_STATE_0_REG                (AIC_SC_REG_BASE + 0x48)   
#define AIC_SC_REG_AXI_CLAMP_STATE_1_REG                (AIC_SC_REG_BASE + 0x4C)   
#define AIC_SC_REG_CLK_GATE_MASK_0_REG                  (AIC_SC_REG_BASE + 0x50)   
#define AIC_SC_REG_CLK_GATE_MASK_1_REG                  (AIC_SC_REG_BASE + 0x54)   
#define AIC_SC_REG_CLK_DELAY_CNT_REG                    (AIC_SC_REG_BASE + 0x58)   
#define AIC_SC_REG_CLK_DELAY_SC_REG                     (AIC_SC_REG_BASE + 0x5C)   
#define AIC_SC_REG_CLK_GATE_MASK_2_REG                  (AIC_SC_REG_BASE + 0x60)   
#define AIC_SC_REG_AXI_CLAMP_STATE_2_REG                (AIC_SC_REG_BASE + 0x68)   
#define AIC_SC_REG_AXI_CLAMP_STATE_3_REG                (AIC_SC_REG_BASE + 0x6C)   
#define AIC_SC_REG_LOCK_BYPASS_0_REG                    (AIC_SC_REG_BASE + 0x70)   /* SC */
#define AIC_SC_REG_LOCK_BYPASS_1_REG                    (AIC_SC_REG_BASE + 0x74)   /* SC */
#define AIC_SC_REG_SYSCTRL_LOCK_REG                     (AIC_SC_REG_BASE + 0x78)   /* SC */
#define AIC_SC_REG_PC_START_0_REG                       (AIC_SC_REG_BASE + 0x80)   /* CCU, IFU */
#define AIC_SC_REG_PC_START_1_REG                       (AIC_SC_REG_BASE + 0x84)   /* CCU, IFU */
#define AIC_SC_REG_PARA_BASE_0_REG                      (AIC_SC_REG_BASE + 0x88)   /* CCU (SPR) */
#define AIC_SC_REG_PARA_BASE_1_REG                      (AIC_SC_REG_BASE + 0x8C)   /* CCU (SPR) */
#define AIC_SC_REG_SMMU_SUBSTREAMID_REG                 (AIC_SC_REG_BASE + 0x90)   /* BIU, CCU */
#define AIC_SC_REG_SMMU_SUBSTREAMID_EN_REG              (AIC_SC_REG_BASE + 0x94)   /* BIU, CCU */
#define AIC_SC_REG_TASK_CFG_0_REG                       (AIC_SC_REG_BASE + 0x98)   /* SPR */
#define AIC_SC_REG_TASK_CFG_1_REG                       (AIC_SC_REG_BASE + 0x9C)   /* SPR */
#define AIC_SC_REG_DATA_MAIN_BASE_0_REG                 (AIC_SC_REG_BASE + 0xA0)   /* CCU (SPR) */
#define AIC_SC_REG_DATA_MAIN_BASE_1_PCIE_AXUSER_REG     (AIC_SC_REG_BASE + 0xA4)   /* CCU (SPR) */
#define AIC_SC_REG_DATA_UB_BASE_0_THREAD_ID_DIM_REG     (AIC_SC_REG_BASE + 0xA8)   /* CCU (SPR) */
#define AIC_SC_REG_DATA_UB_BASE_1_SUBBLOCK_ID_REG       (AIC_SC_REG_BASE + 0xAC)   /* CCU (SPR) */
#define AIC_SC_REG_DATA_SIZE_SAT_MODE_REG               (AIC_SC_REG_BASE + 0xB0)   /* CCU (SPR) */
#define AIC_SC_REG_AIC_PART_ID_AXQOS_REG                (AIC_SC_REG_BASE + 0xB4)   
#define AIC_SC_REG_L2_REMAP_CFG0_0_REG                  (AIC_SC_REG_BASE + 0xB8)   /* BIU */
#define AIC_SC_REG_L2_REMAP_CFG0_1_REG                  (AIC_SC_REG_BASE + 0xBC)   /* BIU */
#define AIC_SC_REG_L2_REMAP_CFG1_0_REG                  (AIC_SC_REG_BASE + 0xC0)   /* BIU */
#define AIC_SC_REG_L2_REMAP_CFG1_1_REG                  (AIC_SC_REG_BASE + 0xC4)   /* BIU */
#define AIC_SC_REG_L2_REMAP_CFG2_0_REG                  (AIC_SC_REG_BASE + 0xC8)   /* BIU */
#define AIC_SC_REG_L2_REMAP_CFG2_1_REG                  (AIC_SC_REG_BASE + 0xCC)   /* BIU */
#define AIC_SC_REG_L2_REMAP_CFG3_0_REG                  (AIC_SC_REG_BASE + 0xD0)   /* BIU */
#define AIC_SC_REG_L2_REMAP_CFG3_1_REG                  (AIC_SC_REG_BASE + 0xD4)   /* BIU */
#define AIC_SC_REG_L2_REMAP_CFG4_0_REG                  (AIC_SC_REG_BASE + 0xD8)   /* BIU */
#define AIC_SC_REG_L2_REMAP_CFG4_1_REG                  (AIC_SC_REG_BASE + 0xDC)   /* BIU */
#define AIC_SC_REG_L2_REMAP_CFG5_0_REG                  (AIC_SC_REG_BASE + 0xE0)   /* BIU */
#define AIC_SC_REG_L2_REMAP_CFG5_1_REG                  (AIC_SC_REG_BASE + 0xE4)   /* BIU */
#define AIC_SC_REG_L2_REMAP_CFG6_0_REG                  (AIC_SC_REG_BASE + 0xE8)   /* BIU */
#define AIC_SC_REG_L2_REMAP_CFG6_1_REG                  (AIC_SC_REG_BASE + 0xEC)   /* BIU */
#define AIC_SC_REG_L2_REMAP_CFG7_0_REG                  (AIC_SC_REG_BASE + 0xF0)   /* BIU */
#define AIC_SC_REG_L2_REMAP_CFG7_1_REG                  (AIC_SC_REG_BASE + 0xF4)   /* BIU */
#define AIC_SC_REG_AIC_STACK_PHY_BASE_0_REG             (AIC_SC_REG_BASE + 0xF8)   
#define AIC_SC_REG_AIC_STACK_PHY_BASE_1_REG             (AIC_SC_REG_BASE + 0xFC)   
#define AIC_SC_REG_DBG_RW_CTRL_REG                      (AIC_SC_REG_BASE + 0x100)  /* DBG */
#define AIC_SC_REG_DBG_ADDR_0_REG                       (AIC_SC_REG_BASE + 0x108)  /* DBG */
#define AIC_SC_REG_DBG_ADDR_1_REG                       (AIC_SC_REG_BASE + 0x10C)  /* DBG */
#define AIC_SC_REG_DBG_DATA0_0_REG                      (AIC_SC_REG_BASE + 0x110)  /* DBG */
#define AIC_SC_REG_DBG_DATA0_1_REG                      (AIC_SC_REG_BASE + 0x114)  /* DBG */
#define AIC_SC_REG_DBG_DATA1_0_REG                      (AIC_SC_REG_BASE + 0x118)  /* DBG */
#define AIC_SC_REG_DBG_DATA1_1_REG                      (AIC_SC_REG_BASE + 0x11C)  /* DBG */
#define AIC_SC_REG_DBG_DATA2_0_REG                      (AIC_SC_REG_BASE + 0x120)  /* DBG */
#define AIC_SC_REG_DBG_DATA2_1_REG                      (AIC_SC_REG_BASE + 0x124)  /* DBG */
#define AIC_SC_REG_DBG_DATA3_0_REG                      (AIC_SC_REG_BASE + 0x128)  /* DBG */
#define AIC_SC_REG_DBG_DATA3_1_REG                      (AIC_SC_REG_BASE + 0x12C)  /* DBG */
#define AIC_SC_REG_DFX_DATA_0_REG                       (AIC_SC_REG_BASE + 0x130)  /* DBG */
#define AIC_SC_REG_DFX_DATA_1_REG                       (AIC_SC_REG_BASE + 0x134)  /* DBG */
#define AIC_SC_REG_DBG_STATUS_REG                       (AIC_SC_REG_BASE + 0x138)  /* DBG */
#define AIC_SC_REG_RESERVED_REG00_0_REG                 (AIC_SC_REG_BASE + 0x140)  /* 保留寄存器 */
#define AIC_SC_REG_RESERVED_REG00_1_REG                 (AIC_SC_REG_BASE + 0x144)  /* 保留寄存器 */
#define AIC_SC_REG_RESERVED_REG01_0_REG                 (AIC_SC_REG_BASE + 0x148)  /* 保留寄存器 */
#define AIC_SC_REG_RESERVED_REG01_1_REG                 (AIC_SC_REG_BASE + 0x14C)  /* 保留寄存器 */
#define AIC_SC_REG_DJTAG_CLK_BYPASS_REG                 (AIC_SC_REG_BASE + 0x150)  /* DBG */
#define AIC_SC_REG_HW_BKPT0_TWO_LEVLE_SMID_REG          (AIC_SC_REG_BASE + 0x160)  /* CCU */
#define AIC_SC_REG_HW_BKPT1_TWO_LEVLE_SMID_REG          (AIC_SC_REG_BASE + 0x168)  /* CCU */
#define AIC_SC_REG_HW_BKPT2_TWO_LEVLE_SMID_REG          (AIC_SC_REG_BASE + 0x170)  /* CCU */
#define AIC_SC_REG_HW_BKPT3_TWO_LEVLE_SMID_REG          (AIC_SC_REG_BASE + 0x178)  /* CCU */
#define AIC_SC_REG_HW_BKPT4_TWO_LEVLE_SMID_REG          (AIC_SC_REG_BASE + 0x180)  /* CCU */
#define AIC_SC_REG_HW_BKPT5_TWO_LEVLE_SMID_REG          (AIC_SC_REG_BASE + 0x188)  /* CCU */
#define AIC_SC_REG_HW_BKPT6_TWO_LEVLE_SMID_REG          (AIC_SC_REG_BASE + 0x190)  /* CCU */
#define AIC_SC_REG_HW_BKPT7_TWO_LEVLE_SMID_REG          (AIC_SC_REG_BASE + 0x198)  /* CCU */
#define AIC_SC_REG_HW_BKPT8_TWO_LEVLE_SMID_REG          (AIC_SC_REG_BASE + 0x1A0)  /* CCU */
#define AIC_SC_REG_HW_BKPT9_TWO_LEVLE_SMID_REG          (AIC_SC_REG_BASE + 0x1A8)  /* CCU */
#define AIC_SC_REG_HW_BKPT10_TWO_LEVLE_SMID_REG         (AIC_SC_REG_BASE + 0x1B0)  /* CCU */
#define AIC_SC_REG_HW_BKPT11_TWO_LEVLE_SMID_REG         (AIC_SC_REG_BASE + 0x1B8)  /* CCU */
#define AIC_SC_REG_HW_BKPT12_TWO_LEVLE_SMID_REG         (AIC_SC_REG_BASE + 0x1C0)  /* CCU */
#define AIC_SC_REG_HW_BKPT13_TWO_LEVLE_SMID_REG         (AIC_SC_REG_BASE + 0x1C8)  /* CCU */
#define AIC_SC_REG_HW_BKPT14_TWO_LEVLE_SMID_REG         (AIC_SC_REG_BASE + 0x1D0)  /* CCU */
#define AIC_SC_REG_HW_BKPT15_TWO_LEVLE_SMID_REG         (AIC_SC_REG_BASE + 0x1D8)  /* CCU */
#define AIC_SC_REG_PMU_CTRL_0_REG                       (AIC_SC_REG_BASE + 0x200)  /* PMU */
#define AIC_SC_REG_PMU_CTRL_1_REG                       (AIC_SC_REG_BASE + 0x204)  /* PMU */
#define AIC_SC_REG_PMU_MIN_OV_CNT_0_REG                 (AIC_SC_REG_BASE + 0x208)  /* PMU */
#define AIC_SC_REG_PMU_MIN_OV_CNT_1_REG                 (AIC_SC_REG_BASE + 0x20C)  /* PMU */
#define AIC_SC_REG_PMU_CNT0_REG                         (AIC_SC_REG_BASE + 0x210)  /* PMU */
#define AIC_SC_REG_PMU_CNT1_REG                         (AIC_SC_REG_BASE + 0x218)  /* PMU */
#define AIC_SC_REG_PMU_CNT2_REG                         (AIC_SC_REG_BASE + 0x220)  /* PMU */
#define AIC_SC_REG_PMU_CNT3_REG                         (AIC_SC_REG_BASE + 0x228)  /* PMU */
#define AIC_SC_REG_PMU_CNT4_REG                         (AIC_SC_REG_BASE + 0x230)  /* PMU */
#define AIC_SC_REG_PMU_CNT5_REG                         (AIC_SC_REG_BASE + 0x238)  /* PMU */
#define AIC_SC_REG_PMU_CNT6_REG                         (AIC_SC_REG_BASE + 0x240)  /* PMU */
#define AIC_SC_REG_PMU_CNT7_REG                         (AIC_SC_REG_BASE + 0x248)  /* PMU */
#define AIC_SC_REG_PMU_TASK_CYC_CNT_0_REG               (AIC_SC_REG_BASE + 0x250)  /* PMU */
#define AIC_SC_REG_PMU_TASK_CYC_CNT_1_REG               (AIC_SC_REG_BASE + 0x254)  /* PMU */
#define AIC_SC_REG_RESERVED_0_REG                       (AIC_SC_REG_BASE + 0x258)  
#define AIC_SC_REG_RESERVED_1_REG                       (AIC_SC_REG_BASE + 0x25C)  
#define AIC_SC_REG_PMU_CNT0_IDX_RESERVED_REG            (AIC_SC_REG_BASE + 0x260)  /* PMU */
#define AIC_SC_REG_PMU_CNT1_IDX_RESERVED_REG            (AIC_SC_REG_BASE + 0x268)  /* PMU */
#define AIC_SC_REG_PMU_CNT2_IDX_RESERVED_REG            (AIC_SC_REG_BASE + 0x270)  /* PMU */
#define AIC_SC_REG_PMU_CNT3_IDX_RESERVED_REG            (AIC_SC_REG_BASE + 0x278)  /* PMU */
#define AIC_SC_REG_PMU_CNT4_IDX_RESERVED_REG            (AIC_SC_REG_BASE + 0x280)  /* PMU */
#define AIC_SC_REG_PMU_CNT5_IDX_RESERVED_REG            (AIC_SC_REG_BASE + 0x288)  /* PMU */
#define AIC_SC_REG_PMU_CNT6_IDX_RESERVED_REG            (AIC_SC_REG_BASE + 0x290)  /* PMU */
#define AIC_SC_REG_PMU_CNT7_IDX_RESERVED_REG            (AIC_SC_REG_BASE + 0x298)  /* PMU */
#define AIC_SC_REG_PMU_START_CNT_CYC_0_REG              (AIC_SC_REG_BASE + 0x2A0)  /* PMU */
#define AIC_SC_REG_PMU_START_CNT_CYC_1_REG              (AIC_SC_REG_BASE + 0x2A4)  /* PMU */
#define AIC_SC_REG_PMU_STOP_CNT_CYC_0_REG               (AIC_SC_REG_BASE + 0x2A8)  /* PMU */
#define AIC_SC_REG_PMU_STOP_CNT_CYC_1_REG               (AIC_SC_REG_BASE + 0x2AC)  /* PMU */
#define AIC_SC_REG_AIC_TASK_CYC_CNT_0_REG               (AIC_SC_REG_BASE + 0x2B0)  /* PMU */
#define AIC_SC_REG_AIC_TASK_CYC_CNT_1_REG               (AIC_SC_REG_BASE + 0x2B4)  /* PMU */
#define AIC_SC_REG_IFU_CTRL_0_REG                       (AIC_SC_REG_BASE + 0x300)  /* IFU */
#define AIC_SC_REG_IFU_CTRL_1_REG                       (AIC_SC_REG_BASE + 0x304)  /* IFU */
#define AIC_SC_REG_CACHE_INV_CTRL_REG                   (AIC_SC_REG_BASE + 0x308)  /* IFU */
#define AIC_SC_REG_IC_INV_VA_0_REG                      (AIC_SC_REG_BASE + 0x310)  /* IFU */
#define AIC_SC_REG_IC_INV_VA_1_REG                      (AIC_SC_REG_BASE + 0x314)  /* IFU */
#define AIC_SC_REG_IC_INV_STATUS_REG                    (AIC_SC_REG_BASE + 0x318)  /* IFU */
#define AIC_SC_REG_HW_BKPT_EN_REG                       (AIC_SC_REG_BASE + 0x400)  /* CCU */
#define AIC_SC_REG_HW_BKPT0_PC_0_REG                    (AIC_SC_REG_BASE + 0x408)  /* CCU */
#define AIC_SC_REG_HW_BKPT0_PC_1_REG                    (AIC_SC_REG_BASE + 0x40C)  /* CCU */
#define AIC_SC_REG_HW_BKPT1_PC_0_REG                    (AIC_SC_REG_BASE + 0x410)  /* CCU */
#define AIC_SC_REG_HW_BKPT1_PC_1_REG                    (AIC_SC_REG_BASE + 0x414)  /* CCU */
#define AIC_SC_REG_HW_BKPT2_PC_0_REG                    (AIC_SC_REG_BASE + 0x418)  /* CCU */
#define AIC_SC_REG_HW_BKPT2_PC_1_REG                    (AIC_SC_REG_BASE + 0x41C)  /* CCU */
#define AIC_SC_REG_HW_BKPT3_PC_0_REG                    (AIC_SC_REG_BASE + 0x420)  /* CCU */
#define AIC_SC_REG_HW_BKPT3_PC_1_REG                    (AIC_SC_REG_BASE + 0x424)  /* CCU */
#define AIC_SC_REG_HW_BKPT4_PC_0_REG                    (AIC_SC_REG_BASE + 0x428)  /* CCU */
#define AIC_SC_REG_HW_BKPT4_PC_1_REG                    (AIC_SC_REG_BASE + 0x42C)  /* CCU */
#define AIC_SC_REG_HW_BKPT5_PC_0_REG                    (AIC_SC_REG_BASE + 0x430)  /* CCU */
#define AIC_SC_REG_HW_BKPT5_PC_1_REG                    (AIC_SC_REG_BASE + 0x434)  /* CCU */
#define AIC_SC_REG_HW_BKPT6_PC_0_REG                    (AIC_SC_REG_BASE + 0x438)  /* CCU */
#define AIC_SC_REG_HW_BKPT6_PC_1_REG                    (AIC_SC_REG_BASE + 0x43C)  /* CCU */
#define AIC_SC_REG_HW_BKPT7_PC_0_REG                    (AIC_SC_REG_BASE + 0x440)  /* CCU */
#define AIC_SC_REG_HW_BKPT7_PC_1_REG                    (AIC_SC_REG_BASE + 0x444)  /* CCU */
#define AIC_SC_REG_HW_BKPT8_PC_0_REG                    (AIC_SC_REG_BASE + 0x448)  /* CCU */
#define AIC_SC_REG_HW_BKPT8_PC_1_REG                    (AIC_SC_REG_BASE + 0x44C)  /* CCU */
#define AIC_SC_REG_HW_BKPT9_PC_0_REG                    (AIC_SC_REG_BASE + 0x450)  /* CCU */
#define AIC_SC_REG_HW_BKPT9_PC_1_REG                    (AIC_SC_REG_BASE + 0x454)  /* CCU */
#define AIC_SC_REG_HW_BKPT10_PC_0_REG                   (AIC_SC_REG_BASE + 0x458)  /* CCU */
#define AIC_SC_REG_HW_BKPT10_PC_1_REG                   (AIC_SC_REG_BASE + 0x45C)  /* CCU */
#define AIC_SC_REG_HW_BKPT11_PC_0_REG                   (AIC_SC_REG_BASE + 0x460)  /* CCU */
#define AIC_SC_REG_HW_BKPT11_PC_1_REG                   (AIC_SC_REG_BASE + 0x464)  /* CCU */
#define AIC_SC_REG_HW_BKPT12_PC_0_REG                   (AIC_SC_REG_BASE + 0x468)  /* CCU */
#define AIC_SC_REG_HW_BKPT12_PC_1_REG                   (AIC_SC_REG_BASE + 0x46C)  /* CCU */
#define AIC_SC_REG_HW_BKPT13_PC_0_REG                   (AIC_SC_REG_BASE + 0x470)  /* CCU */
#define AIC_SC_REG_HW_BKPT13_PC_1_REG                   (AIC_SC_REG_BASE + 0x474)  /* CCU */
#define AIC_SC_REG_HW_BKPT14_PC_0_REG                   (AIC_SC_REG_BASE + 0x478)  /* CCU */
#define AIC_SC_REG_HW_BKPT14_PC_1_REG                   (AIC_SC_REG_BASE + 0x47C)  /* CCU */
#define AIC_SC_REG_HW_BKPT15_PC_0_REG                   (AIC_SC_REG_BASE + 0x480)  /* CCU */
#define AIC_SC_REG_HW_BKPT15_PC_1_REG                   (AIC_SC_REG_BASE + 0x484)  /* CCU */
#define AIC_SC_REG_CCU_IQ_TIMEOUT_REG                   (AIC_SC_REG_BASE + 0x488)  /* CCU */
#define AIC_SC_REG_CCU_CTRL_0_REG                       (AIC_SC_REG_BASE + 0x490)  /* CCU */
#define AIC_SC_REG_CCU_CTRL_1_REG                       (AIC_SC_REG_BASE + 0x494)  /* CCU */
#define AIC_SC_REG_SU_DELAY_CNT_REG                     (AIC_SC_REG_BASE + 0x498)  /* SU */
#define AIC_SC_REG_SU_STB_REG                           (AIC_SC_REG_BASE + 0x4A0)  /* SU */
#define AIC_SC_REG_RO_SPR_STATUS_0_REG                  (AIC_SC_REG_BASE + 0x4B0)  /* CCU */
#define AIC_SC_REG_RO_SPR_STATUS_1_REG                  (AIC_SC_REG_BASE + 0x4B4)  /* CCU */
#define AIC_SC_REG_RESERVED_SPR_STATUS_0_REG            (AIC_SC_REG_BASE + 0x4C0)  /* CCU */
#define AIC_SC_REG_RESERVED_SPR_STATUS_1_REG            (AIC_SC_REG_BASE + 0x4C4)  /* CCU */
#define AIC_SC_REG_COND_SPR_0_REG                       (AIC_SC_REG_BASE + 0x4C8)  /* CCU */
#define AIC_SC_REG_COND_SPR_1_REG                       (AIC_SC_REG_BASE + 0x4CC)  /* CCU */
#define AIC_SC_REG_CCU_REDIRECT_IFU_STATUS_REG          (AIC_SC_REG_BASE + 0x4D0)  /* CCU */
#define AIC_SC_REG_CROSS_CORE_FLAG_REG                  (AIC_SC_REG_BASE + 0x4F0)  
#define AIC_SC_REG_BIU_CTRL0_0_REG                      (AIC_SC_REG_BASE + 0x500)  /* BIU控制寄存器0 */
#define AIC_SC_REG_BIU_CTRL0_1_REG                      (AIC_SC_REG_BASE + 0x504)  /* BIU控制寄存器0 */
#define AIC_SC_REG_BIU_CTRL1_0_REG                      (AIC_SC_REG_BASE + 0x508)  /* BIU控制寄存器1 */
#define AIC_SC_REG_BIU_CTRL1_1_REG                      (AIC_SC_REG_BASE + 0x50C)  /* BIU控制寄存器1 */
#define AIC_SC_REG_BIU_CTRL2_0_REG                      (AIC_SC_REG_BASE + 0x510)  /* BIU控制寄存器2 */
#define AIC_SC_REG_BIU_CTRL2_1_REG                      (AIC_SC_REG_BASE + 0x514)  /* BIU控制寄存器2 */
#define AIC_SC_REG_BIU_STATUS0_0_REG                    (AIC_SC_REG_BASE + 0x518)  /* BIU状态寄存器0 */
#define AIC_SC_REG_BIU_STATUS0_1_REG                    (AIC_SC_REG_BASE + 0x51C)  /* BIU状态寄存器0 */
#define AIC_SC_REG_BIU_STATUS1_0_REG                    (AIC_SC_REG_BASE + 0x520)  /* BIU状态寄存器1 */
#define AIC_SC_REG_BIU_STATUS1_1_REG                    (AIC_SC_REG_BASE + 0x524)  /* BIU状态寄存器1 */
#define AIC_SC_REG_BIU_STATUS2_0_REG                    (AIC_SC_REG_BASE + 0x528)  /* BIU状态寄存器2 */
#define AIC_SC_REG_BIU_STATUS2_1_REG                    (AIC_SC_REG_BASE + 0x52C)  /* BIU状态寄存器2 */
#define AIC_SC_REG_BIU_STATUS3_0_REG                    (AIC_SC_REG_BASE + 0x530)  /* BIU状态寄存器3 */
#define AIC_SC_REG_BIU_STATUS3_1_REG                    (AIC_SC_REG_BASE + 0x534)  /* BIU状态寄存器3 */
#define AIC_SC_REG_BIU_STATUS4_0_REG                    (AIC_SC_REG_BASE + 0x538)  /* BIU状态寄存器4 */
#define AIC_SC_REG_BIU_STATUS4_1_REG                    (AIC_SC_REG_BASE + 0x53C)  /* BIU状态寄存器4 */
#define AIC_SC_REG_BIU_STATUS5_0_REG                    (AIC_SC_REG_BASE + 0x540)  /* BIU状态寄存器5 */
#define AIC_SC_REG_BIU_STATUS5_1_REG                    (AIC_SC_REG_BASE + 0x544)  /* BIU状态寄存器5 */
#define AIC_SC_REG_BIU_SMMU_STREAMID_REG                (AIC_SC_REG_BASE + 0x548)  /* THE STREAMID_FOR BIU */
#define AIC_SC_REG_BIU_L2_REMAP_PADDR_BASE_0_REG        (AIC_SC_REG_BASE + 0x550)  /* L2 PHY_ADDR_BASE */
#define AIC_SC_REG_BIU_L2_REMAP_PADDR_BASE_1_REG        (AIC_SC_REG_BASE + 0x554)  /* L2 PHY_ADDR_BASE */
#define AIC_SC_REG_BIU_L2_SIZE_0_REG                    (AIC_SC_REG_BASE + 0x558)  /* L2 SIZE */
#define AIC_SC_REG_BIU_L2_SIZE_1_REG                    (AIC_SC_REG_BASE + 0x55C)  /* L2 SIZE */
#define AIC_SC_REG_BIU_L2_PAGE_SIZE_REG                 (AIC_SC_REG_BASE + 0x560)  /* L2_PAGE_SIZE */
#define AIC_SC_REG_BIU_CTRL6_0_REG                      (AIC_SC_REG_BASE + 0x568)  /* BIU控制寄存器6 */
#define AIC_SC_REG_BIU_CTRL6_1_REG                      (AIC_SC_REG_BASE + 0x56C)  /* BIU控制寄存器6 */
#define AIC_SC_REG_BIU_STATUS8_0_REG                    (AIC_SC_REG_BASE + 0x570)  /* BIU状态寄存器8 */
#define AIC_SC_REG_BIU_STATUS8_1_REG                    (AIC_SC_REG_BASE + 0x574)  /* BIU状态寄存器8 */
#define AIC_SC_REG_BIU_STREAM_NS_REG                    (AIC_SC_REG_BASE + 0x578)  /* BIU stream NS bit */
#define AIC_SC_REG_BIU_CTRL7_REG                        (AIC_SC_REG_BASE + 0x580)  /* BIU控制寄存器7 */
#define AIC_SC_REG_BIU_CTRL8_0_REG                      (AIC_SC_REG_BASE + 0x588)  /* BIU控制寄存器8 */
#define AIC_SC_REG_BIU_CTRL8_1_REG                      (AIC_SC_REG_BASE + 0x58C)  /* BIU控制寄存器8 */
#define AIC_SC_REG_BIU_SMMU_CFG_REG                     (AIC_SC_REG_BASE + 0x590)  /* THE CONFIG_FOR BIU */
#define AIC_SC_REG_SMMU_SEC_STREAMID_REG                (AIC_SC_REG_BASE + 0x598)  /* the secure streamid for BIU */
#define AIC_SC_REG_AIC_ASIL_REG                         (AIC_SC_REG_BASE + 0x5A0)  /* ASIL code */
#define AIC_SC_REG_AIC_BIU_PARTID_CFG_REG               (AIC_SC_REG_BASE + 0x5A8)  /* PARTID for BIU */
#define AIC_SC_REG_AIC_BIU_PER_MON_AXPORT_REG           (AIC_SC_REG_BASE + 0x5B0)  
#define AIC_SC_REG_BIU_STATUS9_0_REG                    (AIC_SC_REG_BASE + 0x5C0)  /* BIU状态寄存器9 */
#define AIC_SC_REG_BIU_STATUS9_1_REG                    (AIC_SC_REG_BASE + 0x5C4)  /* BIU状态寄存器9 */
#define AIC_SC_REG_AXI_CLAMP_STATE_4_REG                (AIC_SC_REG_BASE + 0x5D0)  
#define AIC_SC_REG_AXI_CLAMP_STATE_5_REG                (AIC_SC_REG_BASE + 0x5D4)  
#define AIC_SC_REG_AXI_CLAMP_STATE_6_REG                (AIC_SC_REG_BASE + 0x5D8)  
#define AIC_SC_REG_AXI_CLAMP_STATE_7_REG                (AIC_SC_REG_BASE + 0x5DC)  
#define AIC_SC_REG_BIU_TH_DIRECT_REG                    (AIC_SC_REG_BASE + 0x5E0)  
#define AIC_SC_REG_CORE_ID_REG                          (AIC_SC_REG_BASE + 0x600)  /* TOPLEVEL */
#define AIC_SC_REG_AIC_VER_0_REG                        (AIC_SC_REG_BASE + 0x608)  /* TOPLEVEL */
#define AIC_SC_REG_AIC_VER_1_REG                        (AIC_SC_REG_BASE + 0x60C)  /* TOPLEVEL */
#define AIC_SC_REG_SMMU_VER_0_REG                       (AIC_SC_REG_BASE + 0x610)  /* TOPLEVEL */
#define AIC_SC_REG_SMMU_VER_1_REG                       (AIC_SC_REG_BASE + 0x614)  /* TOPLEVEL */
#define AIC_SC_REG_DISPATCH_VER_0_REG                   (AIC_SC_REG_BASE + 0x618)  /* TOPLEVEL */
#define AIC_SC_REG_DISPATCH_VER_1_REG                   (AIC_SC_REG_BASE + 0x61C)  /* TOPLEVEL */
#define AIC_SC_REG_AA_VER_0_REG                         (AIC_SC_REG_BASE + 0x620)  /* TOPLEVEL */
#define AIC_SC_REG_AA_VER_1_REG                         (AIC_SC_REG_BASE + 0x624)  /* TOPLEVEL */
#define AIC_SC_REG_CRG_VER_0_REG                        (AIC_SC_REG_BASE + 0x628)  /* TOPLEVEL */
#define AIC_SC_REG_CRG_VER_1_REG                        (AIC_SC_REG_BASE + 0x62C)  /* TOPLEVEL */
#define AIC_SC_REG_POWER_MODE_CTRL_0_REG                (AIC_SC_REG_BASE + 0x630)  /* TOPLEVEL */
#define AIC_SC_REG_POWER_MODE_CTRL_1_REG                (AIC_SC_REG_BASE + 0x634)  /* TOPLEVEL */
#define AIC_SC_REG_MEM_TMOD_CTRL_0_REG                  (AIC_SC_REG_BASE + 0x638)  /* TOPLEVEL */
#define AIC_SC_REG_MEM_TMOD_CTRL_1_REG                  (AIC_SC_REG_BASE + 0x63C)  /* TOPLEVEL */
#define AIC_SC_REG_AIC_ERROR_0_REG                      (AIC_SC_REG_BASE + 0x700)  /* ALL */
#define AIC_SC_REG_AIC_ERROR_1_REG                      (AIC_SC_REG_BASE + 0x704)  /* ALL */
#define AIC_SC_REG_AIC_ERROR_MASK_0_REG                 (AIC_SC_REG_BASE + 0x708)  /* ALL */
#define AIC_SC_REG_AIC_ERROR_MASK_1_REG                 (AIC_SC_REG_BASE + 0x70C)  /* ALL */
#define AIC_SC_REG_BIU_ERR_INFO_0_REG                   (AIC_SC_REG_BASE + 0x710)  /* BIU */
#define AIC_SC_REG_BIU_ERR_INFO_1_REG                   (AIC_SC_REG_BASE + 0x714)  /* BIU */
#define AIC_SC_REG_CCU_ERR_INFO_0_REG                   (AIC_SC_REG_BASE + 0x718)  /* CCU */
#define AIC_SC_REG_CCU_ERR_INFO_1_REG                   (AIC_SC_REG_BASE + 0x71C)  /* CCU */
#define AIC_SC_REG_CUBE_ERR_INFO_REG                    (AIC_SC_REG_BASE + 0x720)  /* CUBE */
#define AIC_SC_REG_BIU_ERR_INFO_2_REG                   (AIC_SC_REG_BASE + 0x724)  
#define AIC_SC_REG_IFU_ERR_INFO_0_REG                   (AIC_SC_REG_BASE + 0x728)  /* IFU */
#define AIC_SC_REG_IFU_ERR_INFO_1_REG                   (AIC_SC_REG_BASE + 0x72C)  /* IFU */
#define AIC_SC_REG_MTE_ERR_INFO_0_REG                   (AIC_SC_REG_BASE + 0x730)  /* MTE */
#define AIC_SC_REG_MTE_ERR_INFO_1_REG                   (AIC_SC_REG_BASE + 0x734)  /* MTE */
#define AIC_SC_REG_VEC_ERR_INFO_0_REG                   (AIC_SC_REG_BASE + 0x738)  /* VEC */
#define AIC_SC_REG_VEC_ERR_INFO_1_REG                   (AIC_SC_REG_BASE + 0x73C)  /* VEC */
#define AIC_SC_REG_MTE_SU_ECC_1BIT_ERR_0_REG            (AIC_SC_REG_BASE + 0x740)  /* MTE */
#define AIC_SC_REG_MTE_SU_ECC_1BIT_ERR_1_REG            (AIC_SC_REG_BASE + 0x744)  /* MTE */
#define AIC_SC_REG_VEC_ECC_1BIT_ERR_0_REG               (AIC_SC_REG_BASE + 0x748)  /* VEC, CUBE */
#define AIC_SC_REG_CUBE_ECC_1BIT_ERR_0_REG              (AIC_SC_REG_BASE + 0x74C)  /* VEC, CUBE */
#define AIC_SC_REG_AIC_ERROR_FORCE_0_REG                (AIC_SC_REG_BASE + 0x750)  /* ALL */
#define AIC_SC_REG_AIC_ERROR_FORCE_1_REG                (AIC_SC_REG_BASE + 0x754)  /* ALL */
#define AIC_SC_REG_AIC_ERROR_FORCE_2_REG                (AIC_SC_REG_BASE + 0x758)  /* ALL */
#define AIC_SC_REG_AIC_ERROR_2_REG                      (AIC_SC_REG_BASE + 0x760)  /* ALL */
#define AIC_SC_REG_AIC_ERROR_3_REG                      (AIC_SC_REG_BASE + 0x764)  /* ALL */
#define AIC_SC_REG_AIC_ERROR_MASK_2_REG                 (AIC_SC_REG_BASE + 0x768)  /* ALL */
#define AIC_SC_REG_AIC_ERROR_MASK_3_REG                 (AIC_SC_REG_BASE + 0x76C)  /* ALL */
#define AIC_SC_REG_CCU_BUS_ERR_INFO_0_REG               (AIC_SC_REG_BASE + 0x770)  /* CCU */
#define AIC_SC_REG_CCU_BUS_ERR_INFO_1_REG               (AIC_SC_REG_BASE + 0x774)  /* CCU */
#define AIC_SC_REG_SC_BUS_ERR_INFO_0_REG                (AIC_SC_REG_BASE + 0x778)  /* SC */
#define AIC_SC_REG_SC_BUS_ERR_INFO_1_REG                (AIC_SC_REG_BASE + 0x77C)  /* SC */
#define AIC_SC_REG_AIC_ERROR_4_REG                      (AIC_SC_REG_BASE + 0x780)  /* ALL */
#define AIC_SC_REG_AIC_ERROR_MASK_4_REG                 (AIC_SC_REG_BASE + 0x788)  /* ALL */
#define AIC_SC_REG_FIXP_ERR_INFO_0_REG                  (AIC_SC_REG_BASE + 0x78C)  /* FIXP */
#define AIC_SC_REG_AIC_ERROR_5_REG                      (AIC_SC_REG_BASE + 0x790)  /* VEC */
#define AIC_SC_REG_AIC_ERROR_MASK_5_REG                 (AIC_SC_REG_BASE + 0x794)  /* VEC */
#define AIC_SC_REG_AIC_ERROR_6_REG                      (AIC_SC_REG_BASE + 0x798)  
#define AIC_SC_REG_AIC_ERROR_MASK_6_REG                 (AIC_SC_REG_BASE + 0x79C)  
#define AIC_SC_REG_AIC_RB_VEC_ERR_INFO_0_REG            (AIC_SC_REG_BASE + 0x7A0)  /* VEC */
#define AIC_SC_REG_AIC_RB_VEC_ERR_INFO_1_REG            (AIC_SC_REG_BASE + 0x7A4)  /* VEC */
#define AIC_SC_REG_AIC_RB_VEC_ERR_INFO_2_REG            (AIC_SC_REG_BASE + 0x7A8)  /* VEC */
#define AIC_SC_REG_AIC_RB_VEC_ERR_INFO_3_REG            (AIC_SC_REG_BASE + 0x7AC)  /* VEC */
#define AIC_SC_REG_AIC_RB_VEC_ERR_INFO_4_REG            (AIC_SC_REG_BASE + 0x7B0)  /* VEC */
#define AIC_SC_REG_AIC_RB_VEC_ERR_INFO_5_REG            (AIC_SC_REG_BASE + 0x7B4)  /* VEC */
#define AIC_SC_REG_AIC_RB_VEC_ERR_INFO_6_REG            (AIC_SC_REG_BASE + 0x7B8)  /* VEC */
#define AIC_SC_REG_AIC_RB_VEC_ERR_INFO_7_REG            (AIC_SC_REG_BASE + 0x7BC)  /* VEC */
#define AIC_SC_REG_SU_MTE_ECC_1BIT_ERR_REG              (AIC_SC_REG_BASE + 0x7C0)  /* SU */
#define AIC_SC_REG_SU_DC_ECC_1BIT_ERR_INFO_REG          (AIC_SC_REG_BASE + 0x7C4)  /* SU */
#define AIC_SC_REG_FIXP_ERR_INFO_1_REG                  (AIC_SC_REG_BASE + 0x7C8)  /* FIXP */
#define AIC_SC_REG_MTE_1BIT_ECC_ADDR_REG                (AIC_SC_REG_BASE + 0x7D0)  
#define AIC_SC_REG_CUBE_ECC_1BIT_ERR_2_REG              (AIC_SC_REG_BASE + 0x7D4)  
#define AIC_SC_REG_AIC_WARN_STATUS_MASK_1_REG           (AIC_SC_REG_BASE + 0x7E0)  /* AIC */
#define AIC_SC_REG_AIC_WARN_STATUS_MASK_2_REG           (AIC_SC_REG_BASE + 0x7E4)  /* AIC */
#define AIC_SC_REG_AIC_WARN_STATUS_1_REG                (AIC_SC_REG_BASE + 0x7F0)  /* AIC */
#define AIC_SC_REG_AIC_WARN_STATUS_2_REG                (AIC_SC_REG_BASE + 0x7F4)  /* AIC */
#define AIC_SC_REG_SPR_STATUS_0_REG                     (AIC_SC_REG_BASE + 0x7F8)  /* CCU */
#define AIC_SC_REG_SPR_STATUS_1_REG                     (AIC_SC_REG_BASE + 0x7FC)  /* CCU */
#define AIC_SC_REG_PCT_CTRL_0_REG                       (AIC_SC_REG_BASE + 0x800)  /* PCT */
#define AIC_SC_REG_PCT_CTRL_1_REG                       (AIC_SC_REG_BASE + 0x804)  /* PCT */
#define AIC_SC_REG_PCT_STATUS_REG                       (AIC_SC_REG_BASE + 0x808)  /* PCT */
#define AIC_SC_REG_PCT_NUM_ENTRIES_REG                  (AIC_SC_REG_BASE + 0x810)  /* PCT */
#define AIC_SC_REG_PCT_START_CNT_CYC_0_REG              (AIC_SC_REG_BASE + 0x820)  /* PCT */
#define AIC_SC_REG_PCT_START_CNT_CYC_1_REG              (AIC_SC_REG_BASE + 0x824)  /* PCT */
#define AIC_SC_REG_PCT_STOP_CNT_CYC_0_REG               (AIC_SC_REG_BASE + 0x828)  /* PCT */
#define AIC_SC_REG_PCT_STOP_CNT_CYC_1_REG               (AIC_SC_REG_BASE + 0x82C)  /* PCT */
#define AIC_SC_REG_PCT_OV_TIMESTAMP_0_REG               (AIC_SC_REG_BASE + 0x830)  /* PCT */
#define AIC_SC_REG_PCT_OV_TIMESTAMP_1_REG               (AIC_SC_REG_BASE + 0x834)  /* PCT */
#define AIC_SC_REG_VEC_CTRL_0_REG                       (AIC_SC_REG_BASE + 0x900)  
#define AIC_SC_REG_VEC_DUMMY_0_REG                      (AIC_SC_REG_BASE + 0x904)  
#define AIC_SC_REG_VEC_CTRL_1_REG                       (AIC_SC_REG_BASE + 0x908)  
#define AIC_SC_REG_VEC_CTRL_2_REG                       (AIC_SC_REG_BASE + 0x90C)  
#define AIC_SC_REG_VEC_RC_CTRL_REG                      (AIC_SC_REG_BASE + 0x910)  
#define AIC_SC_REG_VEC_COOL_CFG_REGS_REG                (AIC_SC_REG_BASE + 0x920)  
#define AIC_SC_REG_VEC_DBG_SID_RESERVED_REG             (AIC_SC_REG_BASE + 0x930)  
#define AIC_SC_REG_VEC_DBG_SSID_RESERVED_REG            (AIC_SC_REG_BASE + 0x934)  
#define AIC_SC_REG_VEC_DBG_HW_BKPT_EN_REG               (AIC_SC_REG_BASE + 0x938)  
#define AIC_SC_REG_VEC_DBG_BKPT_ADDR_0_L_REG            (AIC_SC_REG_BASE + 0x940)  
#define AIC_SC_REG_VEC_DBG_BKPT_ADDR_0_H_REG            (AIC_SC_REG_BASE + 0x944)  
#define AIC_SC_REG_VEC_DBG_BKPT_ADDR_1_L_REG            (AIC_SC_REG_BASE + 0x948)  
#define AIC_SC_REG_VEC_DBG_BKPT_ADDR_1_H_REG            (AIC_SC_REG_BASE + 0x94C)  
#define AIC_SC_REG_VEC_DBG_BKPT_ADDR_2_L_REG            (AIC_SC_REG_BASE + 0x950)  
#define AIC_SC_REG_VEC_DBG_BKPT_ADDR_2_H_REG            (AIC_SC_REG_BASE + 0x954)  
#define AIC_SC_REG_VEC_DBG_BKPT_ADDR_3_L_REG            (AIC_SC_REG_BASE + 0x958)  
#define AIC_SC_REG_VEC_DBG_BKPT_ADDR_3_H_REG            (AIC_SC_REG_BASE + 0x95C)  
#define AIC_SC_REG_VEC_DBG_BKPT_SID_CFG_REG             (AIC_SC_REG_BASE + 0x960)  
#define AIC_SC_REG_VEC_DBG_BKPT_SSID_CFG_REG            (AIC_SC_REG_BASE + 0x964)  
#define AIC_SC_REG_VEC_SISS_MODE_EN_CFG_REG             (AIC_SC_REG_BASE + 0x968)  
#define AIC_SC_REG_VEC_DBG_STATUS_REG                   (AIC_SC_REG_BASE + 0x96C)  
#define AIC_SC_REG_VEC_DBG_CTRL_CFG_REG                 (AIC_SC_REG_BASE + 0x970)  
#define AIC_SC_REG_VEC_DBG_TWO_LEVEL_EN_CFG_RSV_REG     (AIC_SC_REG_BASE + 0x974)  
#define AIC_SC_REG_VEC_IRDROP_CFG_CTRL_REG              (AIC_SC_REG_BASE + 0x9FC)  
#define AIC_SC_REG_VEC_RESERVED_REG00_0_REG             (AIC_SC_REG_BASE + 0xA00)  /* 保留寄存器 */
#define AIC_SC_REG_VEC_RESERVED_REG00_1_REG             (AIC_SC_REG_BASE + 0xA04)  /* 保留寄存器 */
#define AIC_SC_REG_VEC_RESERVED_REG01_0_REG             (AIC_SC_REG_BASE + 0xA08)  /* 保留寄存器 */
#define AIC_SC_REG_VEC_RESERVED_REG01_1_REG             (AIC_SC_REG_BASE + 0xA0C)  /* 保留寄存器 */
#define AIC_SC_REG_CUBE_RESERVED_REG00_0_REG            (AIC_SC_REG_BASE + 0xA10)  /* 保留寄存器 */
#define AIC_SC_REG_CUBE_RESERVED_REG00_1_REG            (AIC_SC_REG_BASE + 0xA14)  /* 保留寄存器 */
#define AIC_SC_REG_CUBE_RESERVED_REG01_0_REG            (AIC_SC_REG_BASE + 0xA18)  /* 保留寄存器 */
#define AIC_SC_REG_CUBE_RESERVED_REG01_1_REG            (AIC_SC_REG_BASE + 0xA1C)  /* 保留寄存器 */
#define AIC_SC_REG_SC_RESERVED_REG00_0_REG              (AIC_SC_REG_BASE + 0xA20)  /* 保留寄存器 */
#define AIC_SC_REG_SC_RESERVED_REG00_1_REG              (AIC_SC_REG_BASE + 0xA24)  /* 保留寄存器 */
#define AIC_SC_REG_SC_RESERVED_REG01_0_REG              (AIC_SC_REG_BASE + 0xA28)  /* 保留寄存器 */
#define AIC_SC_REG_SC_RESERVED_REG01_1_REG              (AIC_SC_REG_BASE + 0xA2C)  /* 保留寄存器 */
#define AIC_SC_REG_SC_RESERVED_RO_REG00_0_REG           (AIC_SC_REG_BASE + 0xA30)  /* 保留寄存器 */
#define AIC_SC_REG_SC_RESERVED_RO_REG00_1_REG           (AIC_SC_REG_BASE + 0xA34)  /* 保留寄存器 */
#define AIC_SC_REG_SC_RESERVED_RO_REG01_0_REG           (AIC_SC_REG_BASE + 0xA38)  /* 保留寄存器 */
#define AIC_SC_REG_SC_RESERVED_RO_REG01_1_REG           (AIC_SC_REG_BASE + 0xA3C)  /* 保留寄存器 */
#define AIC_SC_REG_SC_RESERVED_REG02_0_REG              (AIC_SC_REG_BASE + 0xA40)  /* 保留寄存器 */
#define AIC_SC_REG_SC_RESERVED_REG02_1_REG              (AIC_SC_REG_BASE + 0xA44)  /* 保留寄存器 */
#define AIC_SC_REG_SC_RESERVED_REG03_0_REG              (AIC_SC_REG_BASE + 0xA48)  /* 保留寄存器 */
#define AIC_SC_REG_SC_RESERVED_REG03_1_REG              (AIC_SC_REG_BASE + 0xA4C)  /* 保留寄存器 */
#define AIC_SC_REG_VEC_RESERVED_RO_REG00_0_REG          (AIC_SC_REG_BASE + 0xA50)  /* 保留寄存器 */
#define AIC_SC_REG_VEC_RESERVED_RO_REG00_1_REG          (AIC_SC_REG_BASE + 0xA54)  /* 保留寄存器 */
#define AIC_SC_REG_VEC_RESERVED_RO_REG01_0_REG          (AIC_SC_REG_BASE + 0xA58)  /* 保留寄存器 */
#define AIC_SC_REG_VEC_RESERVED_RO_REG01_1_REG          (AIC_SC_REG_BASE + 0xA5C)  /* 保留寄存器 */
#define AIC_SC_REG_CUBE_RESERVED_RO_REG00_0_REG         (AIC_SC_REG_BASE + 0xA60)  /* 保留寄存器 */
#define AIC_SC_REG_CUBE_RESERVED_RO_REG00_1_REG         (AIC_SC_REG_BASE + 0xA64)  /* 保留寄存器 */
#define AIC_SC_REG_CUBE_RESERVED_RO_REG01_0_REG         (AIC_SC_REG_BASE + 0xA68)  /* 保留寄存器 */
#define AIC_SC_REG_CUBE_RESERVED_RO_REG01_1_REG         (AIC_SC_REG_BASE + 0xA6C)  /* 保留寄存器 */
#define AIC_SC_REG_MTE_CTRL_0_REG                       (AIC_SC_REG_BASE + 0xB00)  
#define AIC_SC_REG_MTE_CTRL_1_REG                       (AIC_SC_REG_BASE + 0xB04)  
#define AIC_SC_REG_MTE_WARMUP_0_REG                     (AIC_SC_REG_BASE + 0xB08)  
#define AIC_SC_REG_MTE_WARMUP_1_REG                     (AIC_SC_REG_BASE + 0xB0C)  
#define AIC_SC_REG_MTE_AIPP_RSV_1_REG                   (AIC_SC_REG_BASE + 0xB10)  
#define AIC_SC_REG_MTE_WAIPP_RSV_REG                    (AIC_SC_REG_BASE + 0xB14)  
#define AIC_SC_REG_MTE_HEBCE_RSV_REG                    (AIC_SC_REG_BASE + 0xB18)  
#define AIC_SC_REG_MTE_HEBCD_RSV_REG                    (AIC_SC_REG_BASE + 0xB1C)  
#define AIC_SC_REG_MTE_FIXP_RSV_REG                     (AIC_SC_REG_BASE + 0xB20)  
#define AIC_SC_REG_MTE_FIXP_WRMUP_0_REG                 (AIC_SC_REG_BASE + 0xB24)  
#define AIC_SC_REG_MTE_FIXP_WRMUP_1_REG                 (AIC_SC_REG_BASE + 0xB28)  
#define AIC_SC_REG_FIXP_CTRL_0_REG                      (AIC_SC_REG_BASE + 0xB2C)  
#define AIC_SC_REG_MTE_CTRL_2_REG                       (AIC_SC_REG_BASE + 0xB40)  
#define AIC_SC_REG_MTE_PCIE_TYPE_BST_LN_REG             (AIC_SC_REG_BASE + 0xB50)  
#define AIC_SC_REG_MTE_PCIE_WIN_SZ_REG                  (AIC_SC_REG_BASE + 0xB54)  
#define AIC_SC_REG_MTE_PCIE_WIN_BS_REG                  (AIC_SC_REG_BASE + 0xB58)  
#define AIC_SC_REG_MTE_PCIE_ENABLE_REG                  (AIC_SC_REG_BASE + 0xB5C)  
#define AIC_SC_REG_CUBE_CTRL_0_REG                      (AIC_SC_REG_BASE + 0xC00)  
#define AIC_SC_REG_CUBE_CTRL_1_REG                      (AIC_SC_REG_BASE + 0xC04)  
#define AIC_SC_REG_CUBE_DUMMY_CTRL_1_REG                (AIC_SC_REG_BASE + 0xC08)  
#define AIC_SC_REG_CUBE_DUMMY_CTRL_2_REG                (AIC_SC_REG_BASE + 0xC0C)  
#define AIC_SC_REG_BIU8_STATUS2_0_REG                   (AIC_SC_REG_BASE + 0xD28)  /* BIU状态寄存器2 */
#define AIC_SC_REG_BIU8_STATUS2_1_REG                   (AIC_SC_REG_BASE + 0xD2C)  /* BIU状态寄存器2 */
#define AIC_SC_REG_BIU8_STATUS3_0_REG                   (AIC_SC_REG_BASE + 0xD30)  /* BIU状态寄存器3 */
#define AIC_SC_REG_BIU8_STATUS3_1_REG                   (AIC_SC_REG_BASE + 0xD34)  /* BIU状态寄存器3 */
#define AIC_SC_REG_BIU8_STATUS4_0_REG                   (AIC_SC_REG_BASE + 0xD38)  /* BIU状态寄存器4 */
#define AIC_SC_REG_BIU8_STATUS4_1_REG                   (AIC_SC_REG_BASE + 0xD3C)  /* BIU状态寄存器4 */
#define AIC_SC_REG_BIU8_SMMU_STREAMID_REG               (AIC_SC_REG_BASE + 0xD48)  /* THE STREAMID_FOR BIU */
#define AIC_SC_REG_BIU8_L2_REMAP_PADDR_BASE_0_REG       (AIC_SC_REG_BASE + 0xD50)  /* L2 PHY_ADDR_BASE */
#define AIC_SC_REG_BIU8_L2_REMAP_PADDR_BASE_1_REG       (AIC_SC_REG_BASE + 0xD54)  /* L2 PHY_ADDR_BASE */
#define AIC_SC_REG_BIU8_L2_PAGE_SIZE_REG                (AIC_SC_REG_BASE + 0xD58)  /* L2_PAGE_SIZE */
#define AIC_SC_REG_BIU8_L2_SIZE_0_REG                   (AIC_SC_REG_BASE + 0xD60)  /* L2 SIZE */
#define AIC_SC_REG_BIU8_L2_SIZE_1_REG                   (AIC_SC_REG_BASE + 0xD64)  /* L2 SIZE */
#define AIC_SC_REG_BIU8_CTRL3_0_REG                     (AIC_SC_REG_BASE + 0xD80)  /* BIU控制寄存器3 */
#define AIC_SC_REG_BIU8_CTRL3_1_REG                     (AIC_SC_REG_BASE + 0xD84)  /* BIU控制寄存器3 */
#define AIC_SC_REG_BIU8_CTRL4_0_REG                     (AIC_SC_REG_BASE + 0xD88)  /* BIU控制寄存器4 */
#define AIC_SC_REG_BIU8_CTRL4_1_REG                     (AIC_SC_REG_BASE + 0xD8C)  /* BIU控制寄存器4 */
#define AIC_SC_REG_BIU8_CTRL5_0_REG                     (AIC_SC_REG_BASE + 0xD90)  /* BIU控制寄存器5 */
#define AIC_SC_REG_BIU8_CTRL5_1_REG                     (AIC_SC_REG_BASE + 0xD94)  /* BIU控制寄存器5 */
#define AIC_SC_REG_BIU8_STATUS6_0_REG                   (AIC_SC_REG_BASE + 0xD98)  /* BIU状态寄存器6 */
#define AIC_SC_REG_BIU8_STATUS6_1_REG                   (AIC_SC_REG_BASE + 0xD9C)  /* BIU状态寄存器6 */
#define AIC_SC_REG_BIU8_STATUS7_0_REG                   (AIC_SC_REG_BASE + 0xDA0)  /* BIU状态寄存器7 */
#define AIC_SC_REG_BIU8_STATUS7_1_REG                   (AIC_SC_REG_BASE + 0xDA4)  /* BIU状态寄存器7 */
#define AIC_SC_REG_CCU_TIME_COMPARE_COUNTER_CTRL_0_REG  (AIC_SC_REG_BASE + 0xE00)  /* WATCH_INT_COMPARE_COUNTER控制寄存器 */
#define AIC_SC_REG_CCU_TIME_COMPARE_COUNTER_CTRL_1_REG  (AIC_SC_REG_BASE + 0xE04)  /* WATCH_INT_COMPARE_COUNTER控制寄存器 */
#define AIC_SC_REG_CCU_TIME_COMPARE_COUNTER_INT_REG     (AIC_SC_REG_BASE + 0xE08)  /* WATCH_INT_COMPARE_COUNTER中断状态寄存器 */
#define AIC_SC_REG_AIC_SECURE_EN_REG                    (AIC_SC_REG_BASE + 0xF00)  
#define AIC_SC_REG_AIC_ERR_RESP_EN_REG                  (AIC_SC_REG_BASE + 0xF08)  
#define AIC_SC_REG_AIC_MEM_INIT_EN_REG                  (AIC_SC_REG_BASE + 0xF10)  
#define AIC_SC_REG_AIC_SRCID_LPID_MASK_REG              (AIC_SC_REG_BASE + 0xF18)  
#define AIC_SC_REG_AIC_RAM_ALWAYS_NOT_CLEAR_REG         (AIC_SC_REG_BASE + 0xF20)  
#define AIC_SC_REG_AIC_RAM_ALWAYS_CLEAR_REG             (AIC_SC_REG_BASE + 0xF28)  
#define AIC_SC_REG_AIC_SAFETY_INT_MASK_REG              (AIC_SC_REG_BASE + 0xF30)  
#define AIC_SC_REG_AIC_SAFETY_ERR_MASK_REG              (AIC_SC_REG_BASE + 0xF38)  
#define AIC_SC_REG_AIC_SAFETY_INT_REG                   (AIC_SC_REG_BASE + 0xF40)  
#define AIC_SC_REG_AIC_SAFETY_REG_PTY_CTL_REG           (AIC_SC_REG_BASE + 0xF48)  
#define AIC_SC_REG_AIC_SAFETY_1BIT_ECC_ERR_NUM_REG      (AIC_SC_REG_BASE + 0xF4C)  
#define AIC_SC_REG_AIC_PENDING_TASK_STATUS_REG          (AIC_SC_REG_BASE + 0xF50)  
#define AIC_SC_REG_AIC_EXCEPTION_CTRL_0_REG             (AIC_SC_REG_BASE + 0xFF0)  
#define AIC_SC_REG_AIC_EXCEPTION_RST_STATUS_REG         (AIC_SC_REG_BASE + 0xFF4)  
#define AIC_SC_REG_AIC_EXCEPTION_CTRL_1_REG             (AIC_SC_REG_BASE + 0xFF8)  
#define AIC_SC_REG_DFX_MEM_ACCESS0_CTRL_0_REG           (AIC_SC_REG_BASE + 0x1000) 
#define AIC_SC_REG_DFX_MEM_ACCESS0_CTRL_1_REG           (AIC_SC_REG_BASE + 0x1004) 
#define AIC_SC_REG_DFX_MEM_ACCESS0_STATUS_REG           (AIC_SC_REG_BASE + 0x1008) 
#define AIC_SC_REG_DFX_MEM_ACCESS0_WDATA0_0_REG         (AIC_SC_REG_BASE + 0x1010) 
#define AIC_SC_REG_DFX_MEM_ACCESS0_WDATA0_1_REG         (AIC_SC_REG_BASE + 0x1014) 
#define AIC_SC_REG_DFX_MEM_ACCESS0_WDATA1_0_REG         (AIC_SC_REG_BASE + 0x1018) 
#define AIC_SC_REG_DFX_MEM_ACCESS0_WDATA1_1_REG         (AIC_SC_REG_BASE + 0x101C) 
#define AIC_SC_REG_DFX_MEM_ACCESS0_WDATA2_0_REG         (AIC_SC_REG_BASE + 0x1020) 
#define AIC_SC_REG_DFX_MEM_ACCESS0_WDATA2_1_REG         (AIC_SC_REG_BASE + 0x1024) 
#define AIC_SC_REG_DFX_MEM_ACCESS0_WDATA3_0_REG         (AIC_SC_REG_BASE + 0x1028) 
#define AIC_SC_REG_DFX_MEM_ACCESS0_WDATA3_1_REG         (AIC_SC_REG_BASE + 0x102C) 
#define AIC_SC_REG_DFX_MEM_ACCESS0_WDATA4_REG           (AIC_SC_REG_BASE + 0x1030) 
#define AIC_SC_REG_DFX_MEM_ACCESS0_RDATA0_0_REG         (AIC_SC_REG_BASE + 0x1038) 
#define AIC_SC_REG_DFX_MEM_ACCESS0_RDATA0_1_REG         (AIC_SC_REG_BASE + 0x103C) 
#define AIC_SC_REG_DFX_MEM_ACCESS0_RDATA1_0_REG         (AIC_SC_REG_BASE + 0x1040) 
#define AIC_SC_REG_DFX_MEM_ACCESS0_RDATA1_1_REG         (AIC_SC_REG_BASE + 0x1044) 
#define AIC_SC_REG_DFX_MEM_ACCESS0_RDATA2_0_REG         (AIC_SC_REG_BASE + 0x1048) 
#define AIC_SC_REG_DFX_MEM_ACCESS0_RDATA2_1_REG         (AIC_SC_REG_BASE + 0x104C) 
#define AIC_SC_REG_DFX_MEM_ACCESS0_RDATA3_0_REG         (AIC_SC_REG_BASE + 0x1050) 
#define AIC_SC_REG_DFX_MEM_ACCESS0_RDATA3_1_REG         (AIC_SC_REG_BASE + 0x1054) 
#define AIC_SC_REG_DFX_MEM_ACCESS0_RDATA4_REG           (AIC_SC_REG_BASE + 0x1058) 
#define AIC_SC_REG_DFX_MEM_ACCESS1_CTRL_0_REG           (AIC_SC_REG_BASE + 0x1060) 
#define AIC_SC_REG_DFX_MEM_ACCESS1_CTRL_1_REG           (AIC_SC_REG_BASE + 0x1064) 
#define AIC_SC_REG_DFX_MEM_ACCESS1_STATUS_REG           (AIC_SC_REG_BASE + 0x1068) 
#define AIC_SC_REG_DFX_MEM_ACCESS1_WDATA0_0_REG         (AIC_SC_REG_BASE + 0x1070) 
#define AIC_SC_REG_DFX_MEM_ACCESS1_WDATA0_1_REG         (AIC_SC_REG_BASE + 0x1074) 
#define AIC_SC_REG_DFX_MEM_ACCESS1_WDATA1_0_REG         (AIC_SC_REG_BASE + 0x1078) 
#define AIC_SC_REG_DFX_MEM_ACCESS1_WDATA1_1_REG         (AIC_SC_REG_BASE + 0x107C) 
#define AIC_SC_REG_DFX_MEM_ACCESS1_WDATA2_0_REG         (AIC_SC_REG_BASE + 0x1080) 
#define AIC_SC_REG_DFX_MEM_ACCESS1_WDATA2_1_REG         (AIC_SC_REG_BASE + 0x1084) 
#define AIC_SC_REG_DFX_MEM_ACCESS1_WDATA3_0_REG         (AIC_SC_REG_BASE + 0x1088) 
#define AIC_SC_REG_DFX_MEM_ACCESS1_WDATA3_1_REG         (AIC_SC_REG_BASE + 0x108C) 
#define AIC_SC_REG_DFX_MEM_ACCESS1_WDATA4_REG           (AIC_SC_REG_BASE + 0x1090) 
#define AIC_SC_REG_DFX_MEM_ACCESS1_RDATA0_0_REG         (AIC_SC_REG_BASE + 0x1098) 
#define AIC_SC_REG_DFX_MEM_ACCESS1_RDATA0_1_REG         (AIC_SC_REG_BASE + 0x109C) 
#define AIC_SC_REG_DFX_MEM_ACCESS1_RDATA1_0_REG         (AIC_SC_REG_BASE + 0x10A0) 
#define AIC_SC_REG_DFX_MEM_ACCESS1_RDATA1_1_REG         (AIC_SC_REG_BASE + 0x10A4) 
#define AIC_SC_REG_DFX_MEM_ACCESS1_RDATA2_0_REG         (AIC_SC_REG_BASE + 0x10A8) 
#define AIC_SC_REG_DFX_MEM_ACCESS1_RDATA2_1_REG         (AIC_SC_REG_BASE + 0x10AC) 
#define AIC_SC_REG_DFX_MEM_ACCESS1_RDATA3_0_REG         (AIC_SC_REG_BASE + 0x10B0) 
#define AIC_SC_REG_DFX_MEM_ACCESS1_RDATA3_1_REG         (AIC_SC_REG_BASE + 0x10B4) 
#define AIC_SC_REG_DFX_MEM_ACCESS1_RDATA4_REG           (AIC_SC_REG_BASE + 0x10B8) 
#define AIC_SC_REG_DFX_MEM_ECC_INJECT_REG_REG           (AIC_SC_REG_BASE + 0x10C0) 
#define AIC_SC_REG_AIC_CONTEXT_SWITCH_CTRL_0_REG        (AIC_SC_REG_BASE + 0x1100) 
#define AIC_SC_REG_AIC_CONTEXT_SWITCH_CTRL_1_REG        (AIC_SC_REG_BASE + 0x1104) 
#define AIC_SC_REG_AIC_CONTEXT_SWITCH_CFG_ST_REG        (AIC_SC_REG_BASE + 0x1108) 
#define AIC_SC_REG_AIC_SYS_VA_BASE_0_REG                (AIC_SC_REG_BASE + 0x1110) 
#define AIC_SC_REG_AIC_SYS_VA_BASE_1_REG                (AIC_SC_REG_BASE + 0x1114) 
#define AIC_SC_REG_L2_VADDR_BASE_0_REG                  (AIC_SC_REG_BASE + 0x1118) /* BIU, CCU(SPR) */
#define AIC_SC_REG_L2_VADDR_BASE_1_REG                  (AIC_SC_REG_BASE + 0x111C) /* BIU, CCU(SPR) */
#define AIC_SC_REG_AIC_MPU_CHICKEN_BIT_REG              (AIC_SC_REG_BASE + 0x1120) 
#define AIC_SC_REG_AIC_UB_BASE_OFFSET_REG               (AIC_SC_REG_BASE + 0x1128) 
#define AIC_SC_REG_AIC_VEC_CSW_REG                      (AIC_SC_REG_BASE + 0x1130) 
#define AIC_SC_REG_AIC_VEC_PERIOD_REG                   (AIC_SC_REG_BASE + 0x1138) 
#define AIC_SC_REG_AIC_VEC_THR_LOW_REG                  (AIC_SC_REG_BASE + 0x1140) 
#define AIC_SC_REG_AIC_VEC_THR_HIGH_REG                 (AIC_SC_REG_BASE + 0x1148) 
#define AIC_SC_REG_AIC_VEC_FIFO_PKN_REG                 (AIC_SC_REG_BASE + 0x1150) 
#define AIC_SC_REG_AIV_RESTORE_PC_0_REG                 (AIC_SC_REG_BASE + 0x1158) 
#define AIC_SC_REG_AIV_RESTORE_PC_1_REG                 (AIC_SC_REG_BASE + 0x115C) 
#define AIC_SC_REG_AIV_RESTORE_BUFFER_ADDR_0_REG        (AIC_SC_REG_BASE + 0x1160) 
#define AIC_SC_REG_AIV_RESTORE_BUFFER_ADDR_1_REG        (AIC_SC_REG_BASE + 0x1164) 
#define AIC_SC_REG_UB_USE_TOTAL_SIZE_REG                (AIC_SC_REG_BASE + 0x11F0) 
#define AIC_SC_REG_PMU_CNT0_IDX_REG                     (AIC_SC_REG_BASE + 0x1280) /* PMU */
#define AIC_SC_REG_PMU_CNT1_IDX_REG                     (AIC_SC_REG_BASE + 0x1284) /* PMU */
#define AIC_SC_REG_PMU_CNT2_IDX_REG                     (AIC_SC_REG_BASE + 0x1288) /* PMU */
#define AIC_SC_REG_PMU_CNT3_IDX_REG                     (AIC_SC_REG_BASE + 0x128C) /* PMU */
#define AIC_SC_REG_PMU_CNT4_IDX_REG                     (AIC_SC_REG_BASE + 0x1290) /* PMU */
#define AIC_SC_REG_PMU_CNT5_IDX_REG                     (AIC_SC_REG_BASE + 0x1294) /* PMU */
#define AIC_SC_REG_PMU_CNT6_IDX_REG                     (AIC_SC_REG_BASE + 0x1298) /* PMU */
#define AIC_SC_REG_PMU_CNT7_IDX_REG                     (AIC_SC_REG_BASE + 0x129C) /* PMU */
#define AIC_SC_REG_AIC_SC_CHICKEN_BIT_CTRL_REG          (AIC_SC_REG_BASE + 0x2000) 
#define AIC_SC_REG_AIC_TASK_SCHEDULE_MODE_REG           (AIC_SC_REG_BASE + 0x2004) 
#define AIC_SC_REG_TASK_RUNING_PC_START_0_REG           (AIC_SC_REG_BASE + 0x2080) 
#define AIC_SC_REG_TASK_RUNING_PC_START_1_REG           (AIC_SC_REG_BASE + 0x2084) 
#define AIC_SC_REG_TASK_RUNING_PARA_BASE_0_REG          (AIC_SC_REG_BASE + 0x2088) /* CCU (SPR) */
#define AIC_SC_REG_TASK_RUNING_PARA_BASE_1_REG          (AIC_SC_REG_BASE + 0x208C) /* CCU (SPR) */
#define AIC_SC_REG_TASK_RUNING_SMMU_SUBSTREAMID_REG     (AIC_SC_REG_BASE + 0x2090) 
#define AIC_SC_REG_TASK_RUNING_SMMU_SUBSTREAMID_EN_REG  (AIC_SC_REG_BASE + 0x2094) 
#define AIC_SC_REG_TASK_RUNING_TASK_CFG_0_REG           (AIC_SC_REG_BASE + 0x2098) 
#define AIC_SC_REG_TASK_RUNING_TASK_CFG_1_REG           (AIC_SC_REG_BASE + 0x209C) 
#define AIC_SC_REG_TASK_RUNING_DATA_MAIN_BASE_0_REG     (AIC_SC_REG_BASE + 0x20A0) 
#define AIC_SC_REG_TASK_RUNING_DATA_MAIN_BASE_1_REG     (AIC_SC_REG_BASE + 0x20A4) 
#define AIC_SC_REG_TASK_RUNING_DATA_UB_BASE_0_REG       (AIC_SC_REG_BASE + 0x20A8) 
#define AIC_SC_REG_TASK_RUNING_DATA_UB_BASE_1_REG       (AIC_SC_REG_BASE + 0x20AC) 
#define AIC_SC_REG_TASK_RUNING_DATA_SIZE_SAT_MODE_REG   (AIC_SC_REG_BASE + 0x20B0) 
#define AIC_SC_REG_TASK_RUNING_AIC_PART_ID_AXQOS_REG    (AIC_SC_REG_BASE + 0x20B4) 
#define AIC_SC_REG_TASK_RUNING_L2_REMAP_CFG0_0_REG      (AIC_SC_REG_BASE + 0x20B8) 
#define AIC_SC_REG_TASK_RUNING_L2_REMAP_CFG0_1_REG      (AIC_SC_REG_BASE + 0x20BC) 
#define AIC_SC_REG_TASK_RUNING_L2_REMAP_CFG1_0_REG      (AIC_SC_REG_BASE + 0x20C0) 
#define AIC_SC_REG_TASK_RUNING_L2_REMAP_CFG1_1_REG      (AIC_SC_REG_BASE + 0x20C4) 
#define AIC_SC_REG_TASK_RUNING_L2_REMAP_CFG2_0_REG      (AIC_SC_REG_BASE + 0x20C8) 
#define AIC_SC_REG_TASK_RUNING_L2_REMAP_CFG2_1_REG      (AIC_SC_REG_BASE + 0x20CC) 
#define AIC_SC_REG_TASK_RUNING_L2_REMAP_CFG3_0_REG      (AIC_SC_REG_BASE + 0x20D0) 
#define AIC_SC_REG_TASK_RUNING_L2_REMAP_CFG3_1_REG      (AIC_SC_REG_BASE + 0x20D4) 
#define AIC_SC_REG_TASK_RUNING_L2_REMAP_CFG4_0_REG      (AIC_SC_REG_BASE + 0x20D8) 
#define AIC_SC_REG_TASK_RUNING_L2_REMAP_CFG4_1_REG      (AIC_SC_REG_BASE + 0x20DC) 
#define AIC_SC_REG_TASK_RUNING_L2_REMAP_CFG5_0_REG      (AIC_SC_REG_BASE + 0x20E0) 
#define AIC_SC_REG_TASK_RUNING_L2_REMAP_CFG5_1_REG      (AIC_SC_REG_BASE + 0x20E4) 
#define AIC_SC_REG_TASK_RUNING_L2_REMAP_CFG6_0_REG      (AIC_SC_REG_BASE + 0x20E8) 
#define AIC_SC_REG_TASK_RUNING_L2_REMAP_CFG6_1_REG      (AIC_SC_REG_BASE + 0x20EC) 
#define AIC_SC_REG_TASK_RUNING_L2_REMAP_CFG7_0_REG      (AIC_SC_REG_BASE + 0x20F0) 
#define AIC_SC_REG_TASK_RUNING_L2_REMAP_CFG7_1_REG      (AIC_SC_REG_BASE + 0x20F4) 
#define AIC_SC_REG_TASK_RUNING_AIC_STACK_PHY_BASE_0_REG (AIC_SC_REG_BASE + 0x20F8) 
#define AIC_SC_REG_TASK_RUNING_AIC_STACK_PHY_BASE_1_REG (AIC_SC_REG_BASE + 0x20FC) 
#define AIC_SC_REG_PMU_FIFO_CNT_0_0_REG                 (AIC_SC_REG_BASE + 0x2200) 
#define AIC_SC_REG_PMU_FIFO_CNT_0_1_REG                 (AIC_SC_REG_BASE + 0x2204) 
#define AIC_SC_REG_PMU_FIFO_CNT_0_2_REG                 (AIC_SC_REG_BASE + 0x2208) 
#define AIC_SC_REG_PMU_FIFO_CNT_0_3_REG                 (AIC_SC_REG_BASE + 0x220C) 
#define AIC_SC_REG_PMU_FIFO_CNT_0_4_REG                 (AIC_SC_REG_BASE + 0x2210) 
#define AIC_SC_REG_PMU_FIFO_CNT_0_5_REG                 (AIC_SC_REG_BASE + 0x2214) 
#define AIC_SC_REG_PMU_FIFO_CNT_0_6_REG                 (AIC_SC_REG_BASE + 0x2218) 
#define AIC_SC_REG_PMU_FIFO_CNT_0_7_REG                 (AIC_SC_REG_BASE + 0x221C) 
#define AIC_SC_REG_PMU_FIFO_MON_OV_0_0_REG              (AIC_SC_REG_BASE + 0x2220) 
#define AIC_SC_REG_PMU_FIFO_MON_OV_0_1_REG              (AIC_SC_REG_BASE + 0x2224) 
#define AIC_SC_REG_PMU_FIFO_TASK_CYC_0_0_REG            (AIC_SC_REG_BASE + 0x2228) 
#define AIC_SC_REG_PMU_FIFO_TASK_CYC_0_1_REG            (AIC_SC_REG_BASE + 0x222C) 
#define AIC_SC_REG_PMU_FIFO_OVERFLOW_0_REG              (AIC_SC_REG_BASE + 0x2230) 
#define AIC_SC_REG_PMU_FIFO_CNT_1_0_REG                 (AIC_SC_REG_BASE + 0x2240) 
#define AIC_SC_REG_PMU_FIFO_CNT_1_1_REG                 (AIC_SC_REG_BASE + 0x2244) 
#define AIC_SC_REG_PMU_FIFO_CNT_1_2_REG                 (AIC_SC_REG_BASE + 0x2248) 
#define AIC_SC_REG_PMU_FIFO_CNT_1_3_REG                 (AIC_SC_REG_BASE + 0x224C) 
#define AIC_SC_REG_PMU_FIFO_CNT_1_4_REG                 (AIC_SC_REG_BASE + 0x2250) 
#define AIC_SC_REG_PMU_FIFO_CNT_1_5_REG                 (AIC_SC_REG_BASE + 0x2254) 
#define AIC_SC_REG_PMU_FIFO_CNT_1_6_REG                 (AIC_SC_REG_BASE + 0x2258) 
#define AIC_SC_REG_PMU_FIFO_CNT_1_7_REG                 (AIC_SC_REG_BASE + 0x225C) 
#define AIC_SC_REG_PMU_FIFO_MON_OV_1_0_REG              (AIC_SC_REG_BASE + 0x2260) 
#define AIC_SC_REG_PMU_FIFO_MON_OV_1_1_REG              (AIC_SC_REG_BASE + 0x2264) 
#define AIC_SC_REG_PMU_FIFO_TASK_CYC_1_0_REG            (AIC_SC_REG_BASE + 0x2268) 
#define AIC_SC_REG_PMU_FIFO_TASK_CYC_1_1_REG            (AIC_SC_REG_BASE + 0x226C) 
#define AIC_SC_REG_PMU_FIFO_OVERFLOW_1_REG              (AIC_SC_REG_BASE + 0x2270) 
#define AIC_SC_REG_PMU_FIFO_INFO_REG                    (AIC_SC_REG_BASE + 0x2274) 
#define AIC_SC_REG_AIC_SC_DFX_STATUS_REG                (AIC_SC_REG_BASE + 0x2280) 
#define AIC_SC_REG_AIC_1BIT_ECC_MULTI_HIT_CNT_REG_REG   (AIC_SC_REG_BASE + 0x2290) 

#endif // __AIC_SC_REG_REG_OFFSET_H__
