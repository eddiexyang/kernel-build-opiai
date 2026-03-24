/*
 * Copyright     :  Copyright (C) 2021, Huawei Technologies Co. Ltd.
 * File name     :  hipciec_ap_iob_tx_reg_reg_offset.h
 * Department    :  CAD Development Department
 * Author        :  xxx
 * Version       :  1.0
 * Description   :  PCIE Controller 5.0  Version 200
 * Others        :  Generated automatically by nManager V5.1
 * History       :  xxx 2021/10/25 15:27:37 Create file
 */

#ifndef __HIPCIEC50_AP_IOB_TX_REG_REG_OFFSET_H__
#define __HIPCIEC50_AP_IOB_TX_REG_REG_OFFSET_H__

/* HIPCIEC50_AP_IOB_TX_REG Base address of Module's Register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE                       (0x0)

/******************************************************************************/
/*                      HiPCIECTRL50V200 HIPCIEC50_AP_IOB_TX_REG Registers' Definitions                            */
/******************************************************************************/

#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TXATU_CONTROL_0_0_REG                (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x0)    /* IOB TX address translation unit control register0.Common information. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TXATU_CONTROL_2_0_REG                (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x8)    /* IOB TX address translation unit control information register2.Extend the IOB_TXATU_REGION_SIZE. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TXATU_REGION_SIZE_0_REG              (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0xC)    /* IOB TX address translation unit region size low 32bit. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TXATU_BASE_ADDR_L_0_REG              (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x10)   /* IOB TX address translation unit base address low 32bit.the address is align to 4KByte. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TXATU_BASE_ADDR_H_0_REG              (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x14)   /* IOB TX address translation unit base address high 32bit. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TXATU_TAR_ADDR_L_0_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x18)   /* IOB TX address translation unit target address low 32bit.the address is align to 4KByte. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TXATU_TAR_ADDR_H_0_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1C)   /* IOB TX address translation unit target address high 32bit. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_ECAM_CONTROL0_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x600)  /* IOB TX ECAM control register0 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_ECAM_CONTROL1_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x604)  /* IOB TX ECAM control register1 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_ECAM_BASE_ADDR_L_REG            (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x608)  /* IOB TX ECAM base address low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_ECAM_BASE_ADDR_H_REG            (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x60C)  /* IOB TX ECAM base address high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_CXL_BASE_BUS_0_REG              (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x610)  /* IOB TX BASE BUS for CXL Device RCiEP CfgSpace */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_CXL_BASE_BUS_1_REG              (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x630)  /* IOB TX BASE BUS for CXL Device RCiEP CfgSpace */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_CXL_RCRB_BASE_L_0_REG           (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x614)  /* IOB TX CXL RCRB BASE ADDR low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_CXL_RCRB_BASE_L_1_REG           (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x634)  /* IOB TX CXL RCRB BASE ADDR low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_CXL_RCRB_BASE_H_0_REG           (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x618)  /* IOB TX CXL RCRB BASE ADDR high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_CXL_RCRB_BASE_H_1_REG           (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x638)  /* IOB TX CXL RCRB BASE ADDR high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_EP_REMAP_CTRL_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x680)  /* IOB TX EP Address remap control register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_MSK0_REG                    (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x800)  /* IOB_TX interrupt mask register0 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_MSK1_REG                    (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x804)  /* IOB_TX interrupt mask register1 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_MSK2_REG                    (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x808)  /* IOB_TX interrupt mask register2 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_MSK3_REG                    (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x80C)  /* IOB_TX interrupt mask register3 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_MSK4_REG                    (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x810)  /* IOB_TX interrupt mask register4 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_MSK5_REG                    (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x814)  /* IOB_TX interrupt mask register5 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_STATUS0_REG                 (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x820)  /* IOB_TX interrupt status register0 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_STATUS1_REG                 (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x824)  /* IOB_TX interrupt status register1 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_STATUS2_REG                 (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x828)  /* IOB_TX interrupt status register2 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_STATUS3_REG                 (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x82C)  /* IOB_TX interrupt status register3 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_STATUS4_REG                 (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x830)  /* IOB_TX interrupt status register4 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_STATUS5_REG                 (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x834)  /* IOB_TX interrupt status register5 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_RO0_REG                     (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x840)  /* IOB_TX interrupt read only status register0 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_RO1_REG                     (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x844)  /* IOB_TX interrupt read only status register1 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_RO2_REG                     (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x848)  /* IOB_TX interrupt read only status register2 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_RO3_REG                     (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x84C)  /* IOB_TX interrupt read only status register3 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_RO4_REG                     (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x850)  /* IOB_TX interrupt read only status register4 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_RO5_REG                     (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x854)  /* IOB_TX interrupt read only status register5 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_SET0_REG                    (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x860)  /* IOB_TX interrupt set register0 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_SET1_REG                    (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x864)  /* IOB_TX interrupt set register1 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_SET2_REG                    (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x868)  /* IOB_TX interrupt set register2 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_SET3_REG                    (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x86C)  /* IOB_TX interrupt set register3 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_SET4_REG                    (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x870)  /* IOB_TX interrupt set register4 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_SET5_REG                    (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x874)  /* IOB_TX interrupt set register5 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_SEVERITY0_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x880)  /* IOB TX interrupt severity register0 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_SEVERITY1_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x884)  /* IOB TX interrupt severity register1 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_SEVERITY2_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x888)  /* IOB TX interrupt severity register2 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_SEVERITY3_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x88C)  /* IOB TX interrupt severity register3 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_SEVERITY4_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x890)  /* IOB TX interrupt severity register4 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_SEVERITY5_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x894)  /* IOB TX interrupt severity register5 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_RO_CE0_REG                  (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x8A0)  /* IOB_TX  correctable error  interrupt read only status register0 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_RO_CE1_REG                  (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x8A4)  /* IOB_TX  correctable error interrupt read only status register1 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_RO_CE2_REG                  (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x8A8)  /* IOB_TX  correctable error interrupt read only status register2 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_RO_CE3_REG                  (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x8AC)  /* IOB_TX  correctable error interrupt read only status register3 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_RO_CE4_REG                  (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x8B0)  /* IOB_TX  correctable error interrupt read only status register4 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_RO_CE5_REG                  (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x8B4)  /* IOB_TX  correctable error interrupt read only status register5 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_RO_NFE0_REG                 (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x8C0)  /* IOB_TX  nonfatal error  interrupt read only status register0 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_RO_NFE1_REG                 (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x8C4)  /* IOB_TX  nonfatal error interrupt read only status register1 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_RO_NFE2_REG                 (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x8C8)  /* IOB_TX  nonfatal error interrupt read only status register2 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_RO_NFE3_REG                 (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x8CC)  /* IOB_TX  nonfatal error interrupt read only status register3 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_RO_NFE4_REG                 (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x8D0)  /* IOB_TX  nonfatal error interrupt read only status register4 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_RO_NFE5_REG                 (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x8D4)  /* IOB_TX  nonfatal error interrupt read only status register5 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_RO_FE0_REG                  (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x8E0)  /* IOB_TX  fatal error  interrupt read only status register0 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_RO_FE1_REG                  (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x8E4)  /* IOB_TX  fatal error interrupt read only status register1 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_RO_FE2_REG                  (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x8E8)  /* IOB_TX  fatal error interrupt read only status register2 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_RO_FE3_REG                  (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x8EC)  /* IOB_TX  fatal error interrupt read only status register3 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_INT_RO_FE4_REG                  (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x8F0)  /* IOB_TX  fatal error interrupt read only status register4 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_ICG_EN_REG                      (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x900)  /* IOB_TX submodule AXI ICG enable register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_DAT_BE_ECC_ERR_INJECT_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x9E0)  /* IOB TX data be buffer ecc injection register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_DAT_BE_ECC_ERR_ADDR_REG         (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x9E4)  /* IOB TX data be buffer ecc error address register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_DAT_BE_ECC_ERR_CNT_REG          (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x9E8)  /* IOB TX data be buffer ecc error counter register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_HED_BUF_ECC_ERR_INJECT_REG      (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x9F0)  /* IOB TX head buffer ecc injection register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_HED_BUF_ECC_ERR_ADDR_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x9F4)  /* IOB TX head buffer ecc error address register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_HED_BUF_ECC_ERR_CNT_REG         (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x9F8)  /* IOB TX head buffer ecc error counter register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_DAT_BUF_ECC_ERR_INJECT_REG      (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0xA00)  /* IOB TX data buffer ecc injection register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_DAT_BUF_ECC_ERR_ADDR_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0xA04)  /* IOB TX data buffer ecc error address register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_DAT_BUF_ECC_ERR_CNT_REG         (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0xA08)  /* IOB TX data buffer ecc error counter register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_P2P_BUF_ECC_ERR_INJECT_REG  (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0xA10)  /* IOB TX TCS P2P head sram ecc injection register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_P2P_BUF_ECC_ERR_ADDR0_REG   (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0xA14)  /* IOB TX TCS P2P head sram ecc error address register0 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_P2P_BUF_ECC_ERR_ADDR1_REG   (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0xA18)  /* IOB TX TCS P2P head sram ecc error address register1 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_P2P_BUF_ECC_ERR_CNT_REG     (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0xA1C)  /* IOB TX TCS P2P head sram ecc error counter register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_NC_BUF_ECC_ERR_INJECT_REG   (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0xA20)  /* IOB TX TCS NC directory sram ecc injection register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_NC_BUF_ECC_ERR_CNT_REG      (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0xA24)  /* IOB TX TCS NC directory sram ecc error counter register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_NC_BUF_ECC_ERR_ADDR0_REG    (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0xA28)  /* IOB TX TCS NC directory sram ecc error address register0 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_NC_BUF_ECC_ERR_ADDR1_0_REG  (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0xA2C)  /* IOB TX TCS NC directory sram ecc error counter register1 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_NC_BUF_ECC_ERR_ADDR1_1_REG  (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0xA30)  /* IOB TX TCS NC directory sram ecc error counter register1 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_NC_BUF_ECC_ERR_ADDR1_2_REG  (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0xA34)  /* IOB TX TCS NC directory sram ecc error counter register1 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_NC_BUF_ECC_ERR_ADDR1_3_REG  (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0xA38)  /* IOB TX TCS NC directory sram ecc error counter register1 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_NC_BUF_ECC_ERR_ADDR1_4_REG  (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0xA3C)  /* IOB TX TCS NC directory sram ecc error counter register1 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_NC_BUF_ECC_ERR_ADDR1_5_REG  (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0xA40)  /* IOB TX TCS NC directory sram ecc error counter register1 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_NC_BUF_ECC_ERR_ADDR1_6_REG  (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0xA44)  /* IOB TX TCS NC directory sram ecc error counter register1 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_NC_BUF_ECC_ERR_ADDR1_7_REG  (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0xA48)  /* IOB TX TCS NC directory sram ecc error counter register1 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_NC_BUF_ECC_ERR_ADDR1_8_REG  (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0xA4C)  /* IOB TX TCS NC directory sram ecc error counter register1 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_NC_BUF_ECC_ERR_ADDR1_9_REG  (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0xA50)  /* IOB TX TCS NC directory sram ecc error counter register1 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_NC_BUF_ECC_ERR_ADDR1_10_REG (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0xA54)  /* IOB TX TCS NC directory sram ecc error counter register1 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_NC_BUF_ECC_ERR_ADDR1_11_REG (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0xA58)  /* IOB TX TCS NC directory sram ecc error counter register1 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_NC_BUF_ECC_ERR_ADDR1_12_REG (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0xA5C)  /* IOB TX TCS NC directory sram ecc error counter register1 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_NC_BUF_ECC_ERR_ADDR1_13_REG (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0xA60)  /* IOB TX TCS NC directory sram ecc error counter register1 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_NC_BUF_ECC_ERR_ADDR1_14_REG (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0xA64)  /* IOB TX TCS NC directory sram ecc error counter register1 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_NC_BUF_ECC_ERR_ADDR1_15_REG (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0xA68)  /* IOB TX TCS NC directory sram ecc error counter register1 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_REQ_BUF_ECC_ERR_INJECT_REG  (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0xA70)  /* IOB TX TCS req queue sram ecc injection register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_REQ_BUF_ECC_ERR_ADDR_REG    (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0xA74)  /* IOB TX TCS req queue sram ecc error address register0 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_REQ_BUF_ECC_ERR_CNT_REG     (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0xA78)  /* IOB TX TCS req queue sram ecc error counter register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_AXIS_CONTROL0_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0xC00)  /* iob_tx_axis_control_0 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_AXIS_CONTROL1_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0xC04)  /* iob_tx_axis_control_1 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_AXIS_CONTROL2_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0xC08)  /* iob_tx_axis_control_2 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_AXIS_CREDIT_CTRL_REG            (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0xC10)  /* axis credit cnt control */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_AXIS_CREDIT_TH_REG              (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0xC14)  /* axis credit cnt width */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_AXIS_RPLC_DATA_REG              (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0xC20)  /* axis repair rdata for unusual cpl */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_DBUF_P_PRI_RGN_ADDR_0_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1000) /* The start and end address of P private space for port region n(n=0~3) in data buffer. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_DBUF_P_PRI_RGN_ADDR_1_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1010) /* The start and end address of P private space for port region n(n=0~3) in data buffer. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_DBUF_P_PRI_RGN_ADDR_2_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1020) /* The start and end address of P private space for port region n(n=0~3) in data buffer. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_DBUF_P_PRI_RGN_ADDR_3_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1030) /* The start and end address of P private space for port region n(n=0~3) in data buffer. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_DBUF_NP_PRI_RGN_ADDR_0_REG      (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1004) /* The start and end address of NP private space for port region n(n=0~3) in data buffer. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_DBUF_NP_PRI_RGN_ADDR_1_REG      (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1014) /* The start and end address of NP private space for port region n(n=0~3) in data buffer. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_DBUF_NP_PRI_RGN_ADDR_2_REG      (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1024) /* The start and end address of NP private space for port region n(n=0~3) in data buffer. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_DBUF_NP_PRI_RGN_ADDR_3_REG      (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1034) /* The start and end address of NP private space for port region n(n=0~3) in data buffer. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_DBUF_CPL_PRI_RGN_ADDR_0_REG     (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1008) /* The start and end address of CPL private space for port region n(n=0~3) in data buffer. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_DBUF_CPL_PRI_RGN_ADDR_1_REG     (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1018) /* The start and end address of CPL private space for port region n(n=0~3) in data buffer. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_DBUF_CPL_PRI_RGN_ADDR_2_REG     (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1028) /* The start and end address of CPL private space for port region n(n=0~3) in data buffer. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_DBUF_CPL_PRI_RGN_ADDR_3_REG     (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1038) /* The start and end address of CPL private space for port region n(n=0~3) in data buffer. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_DBUF_ALL_SHR_RGN_ADDR_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1080) /* The start address and end address of all shared space in data buffer. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_DBUF_P_SHR_RGN_ADDR_REG         (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1084) /* The start and end address of P shared space in data buffer. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_DBUF_NP_SHR_RGN_ADDR_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1088) /* The start and end address of NP shared space in data buffer. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_DBUF_CPL_SHR_RGN_ADDR_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x108C) /* The start and end address of CPL shared space in data buffer. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_DBUF_SRC_PRI_RGN_ADDR_0_REG     (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x10A0) /* The start and end address of source private region n(n=0~15) in data buffer. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_DBUF_SRC_PRI_RGN_ADDR_1_REG     (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x10A4) /* The start and end address of source private region n(n=0~15) in data buffer. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_DBUF_SRC_PRI_RGN_ADDR_2_REG     (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x10A8) /* The start and end address of source private region n(n=0~15) in data buffer. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_HBUF_P_PRI_RGN_ADDR_0_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1100) /* The start and end address of P private space for port region n(n=0~3) in header buffer. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_HBUF_P_PRI_RGN_ADDR_1_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1110) /* The start and end address of P private space for port region n(n=0~3) in header buffer. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_HBUF_P_PRI_RGN_ADDR_2_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1120) /* The start and end address of P private space for port region n(n=0~3) in header buffer. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_HBUF_P_PRI_RGN_ADDR_3_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1130) /* The start and end address of P private space for port region n(n=0~3) in header buffer. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_HBUF_NP_PRI_RGN_ADDR_0_REG      (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1104) /* The start and end address of NP private space for port region n(n=0~3) in header buffer. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_HBUF_NP_PRI_RGN_ADDR_1_REG      (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1114) /* The start and end address of NP private space for port region n(n=0~3) in header buffer. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_HBUF_NP_PRI_RGN_ADDR_2_REG      (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1124) /* The start and end address of NP private space for port region n(n=0~3) in header buffer. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_HBUF_NP_PRI_RGN_ADDR_3_REG      (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1134) /* The start and end address of NP private space for port region n(n=0~3) in header buffer. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_HBUF_CPL_PRI_RGN_ADDR_0_REG     (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1108) /* The start and end address of CPL private space for port region n(n=0~3) in header buffer. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_HBUF_CPL_PRI_RGN_ADDR_1_REG     (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1118) /* The start and end address of CPL private space for port region n(n=0~3) in header buffer. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_HBUF_CPL_PRI_RGN_ADDR_2_REG     (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1128) /* The start and end address of CPL private space for port region n(n=0~3) in header buffer. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_HBUF_CPL_PRI_RGN_ADDR_3_REG     (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1138) /* The start and end address of CPL private space for port region n(n=0~3) in header buffer. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_HBUF_ALL_SHR_RGN_ADDR_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1180) /* The start address and end address of all shared space in header buffer. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_HBUF_P_SHR_RGN_ADDR_REG         (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1184) /* The start and end address of P shared space in header buffer. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_HBUF_NP_SHR_RGN_ADDR_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1188) /* The start and end address of NP shared space in header buffer. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_HBUF_CPL_SHR_RGN_ADDR_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x118C) /* The start and end address of CPL shared space in header buffer. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_HBUF_SRC_PRI_RGN_ADDR_0_REG     (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x11A0) /* The start and end address of source private region n(n=0~15) in header buffer. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_HBUF_SRC_PRI_RGN_ADDR_1_REG     (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x11A4) /* The start and end address of source private region n(n=0~15) in header buffer. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_HBUF_SRC_PRI_RGN_ADDR_2_REG     (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x11A8) /* The start and end address of source private region n(n=0~15) in header buffer. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_ALC_BUF_REQ_WRR_WGT_REG         (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1200) /* allocation buffer requests wrr weight register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_REQ_QUE_RETRY_TH_REG            (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1204) /* the water level of request queue. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_REQ_QUE_TIMEOUT_CTRL_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1208) /* request queue timeout control register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_SRC_PRI_RGN_MAP_REG             (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x120C) /* source private region mapping register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_BUF_RGN_CFG_REG                 (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1210) /* buffer region config register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_REQ_TOUT_NOTIFY_MSK_REG         (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1280) /* IOB TX request timeout notify the Physical Layer mask register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_CRS_RETRY_CTRL_REG              (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x12F0) /* IOB TX CRS retry control register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_REQ_TIMER_CFG_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1300) /* TQM Request timeout timer threshold */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_REQ_TIMER_UNIT_REG              (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1340) /* TQM Request timeout timer unit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_REMOTE_TLP_STREAMOUT_WGT_0_REG  (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1350) /* TQM remote TLP streaming out  wrr weight register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_REMOTE_TLP_STREAMOUT_WGT_1_REG  (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1358) /* TQM remote TLP streaming out  wrr weight register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_LOCAL_TLP_STREAMOUT_WGT_REG     (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1390) /* TQM local TLP streaming out wrr weight register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_DUAL_CHN_CTRL_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1394) /* TQM dual channals control register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TRACE_CDT_CLR_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x13FC) /* TQM TLP tracing credit clear register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TLP_CDT_CLR_0_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1400) /* TQM TLP credit clear register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TLP_CDT_CLR_1_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1408) /* TQM TLP credit clear register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_CFG0_CDT_CLR_REG                (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1440) /* TQM Cfg0 credit clear register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_CDT_ACTIVE_MSK_REG              (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1444) /* TQM  credit mask register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_BA_TIMER_CFG_REG            (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1800) /* TCS Buffer Allocate timeout release timer threshold */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_BA_COUNTER_CFG_REG          (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1804) /* TCS Buffer Allocate pre allocate times threshold */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_BA_PRE_SIZE_REG             (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1808) /* TCS pre allocate buf control register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_BA_CTRL_REG                 (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x180C) /* TCS avaliable allocated buffer threshold register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_REQ_HALT_TIMER_REG          (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1810) /* TCS request halt timer register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_SEQ_BA_CTRL_REG             (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1814) /* TCS sequence flow avaliable allocated buffer threshold register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_SEQ_CTRL_REG                (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1818) /* TCS sequence flow control register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_SEQ_SKT_MAP_REG             (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x181C) /* TCS sequence flow socket bitmap register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_LINK_CTRL_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1820) /* TCS CHI interface link interaction control register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_NORMAL_BA_CTRL_REG          (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1824) /* TCS normal request avaliable allocated buffer threshold register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_DEC_ERR_INFO_L_REG          (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1840) /* TCS decode error information register low 32bit register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_DEC_ERR_INFO_H_REG          (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1844) /* TCS decode error information register high 32bit register.. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_ETH_PORT_NUM_REG            (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1850) /* TCS Through Port Number register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_BT_TH_SRCID_REG             (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1858) /* TCS BT Through srcdi regsiter. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_ORDER_CTRL_REG              (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1860) /* TCS ordering control regsiter */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_P2P_CTRL_REG                (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1868) /* TCS P2P control regsiter */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_P2P_BA_CTRL0_REG            (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1870) /* TCS P2P avaliable allocated buffer threshold register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_P2P_BA_CTRL1_REG            (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1874) /* TCS P2P avaliable allocated buffer threshold register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_RESP_CTRL_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1880) /* CHI bus return Compdata or Comp Flit RespErr field control register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_RESP_VID_REG                (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1884) /* CfgRd read vendor ID behavior control register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_RDATA_CTRL_REG              (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1888) /* CHI Bus return CompData data payload control register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_RPLC_DATA_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x188C) /* CHI Bus return CompData replaced data payload. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_POISON_FORWARD_REG          (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1890) /* CHI Bus return CompData poison bit control register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_POISON_CTRL_0_REG           (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x18A0) /* CHI Bus receive Poisoned DataFlit behavior control register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_POISON_CTRL_1_REG           (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x18A8) /* CHI Bus receive Poisoned DataFlit behavior control register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_MISC_CTRL0_REG              (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1900) /* TCS misc ctrl0 register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_MISC_CTRL1_REG              (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1904) /* TCS misc ctrl1 register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_BASE_L_0_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A00) /* IOB_TX NC directory base address low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_BASE_L_1_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A10) /* IOB_TX NC directory base address low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_BASE_L_2_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A20) /* IOB_TX NC directory base address low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_BASE_L_3_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A30) /* IOB_TX NC directory base address low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_BASE_L_4_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A40) /* IOB_TX NC directory base address low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_BASE_L_5_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A50) /* IOB_TX NC directory base address low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_BASE_L_6_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A60) /* IOB_TX NC directory base address low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_BASE_L_7_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A70) /* IOB_TX NC directory base address low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_BASE_L_8_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A80) /* IOB_TX NC directory base address low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_BASE_L_9_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A90) /* IOB_TX NC directory base address low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_BASE_L_10_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1AA0) /* IOB_TX NC directory base address low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_BASE_L_11_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1AB0) /* IOB_TX NC directory base address low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_BASE_L_12_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1AC0) /* IOB_TX NC directory base address low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_BASE_L_13_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1AD0) /* IOB_TX NC directory base address low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_BASE_L_14_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1AE0) /* IOB_TX NC directory base address low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_BASE_L_15_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1AF0) /* IOB_TX NC directory base address low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_BASE_H_0_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A04) /* IOB_TX NC directory base address high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_BASE_H_1_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A14) /* IOB_TX NC directory base address high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_BASE_H_2_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A24) /* IOB_TX NC directory base address high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_BASE_H_3_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A34) /* IOB_TX NC directory base address high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_BASE_H_4_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A44) /* IOB_TX NC directory base address high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_BASE_H_5_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A54) /* IOB_TX NC directory base address high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_BASE_H_6_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A64) /* IOB_TX NC directory base address high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_BASE_H_7_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A74) /* IOB_TX NC directory base address high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_BASE_H_8_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A84) /* IOB_TX NC directory base address high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_BASE_H_9_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A94) /* IOB_TX NC directory base address high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_BASE_H_10_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1AA4) /* IOB_TX NC directory base address high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_BASE_H_11_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1AB4) /* IOB_TX NC directory base address high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_BASE_H_12_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1AC4) /* IOB_TX NC directory base address high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_BASE_H_13_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1AD4) /* IOB_TX NC directory base address high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_BASE_H_14_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1AE4) /* IOB_TX NC directory base address high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_BASE_H_15_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1AF4) /* IOB_TX NC directory base address high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_MASK_L_0_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A08) /* IOB_TX NC directory address mask low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_MASK_L_1_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A18) /* IOB_TX NC directory address mask low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_MASK_L_2_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A28) /* IOB_TX NC directory address mask low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_MASK_L_3_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A38) /* IOB_TX NC directory address mask low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_MASK_L_4_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A48) /* IOB_TX NC directory address mask low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_MASK_L_5_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A58) /* IOB_TX NC directory address mask low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_MASK_L_6_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A68) /* IOB_TX NC directory address mask low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_MASK_L_7_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A78) /* IOB_TX NC directory address mask low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_MASK_L_8_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A88) /* IOB_TX NC directory address mask low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_MASK_L_9_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A98) /* IOB_TX NC directory address mask low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_MASK_L_10_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1AA8) /* IOB_TX NC directory address mask low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_MASK_L_11_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1AB8) /* IOB_TX NC directory address mask low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_MASK_L_12_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1AC8) /* IOB_TX NC directory address mask low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_MASK_L_13_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1AD8) /* IOB_TX NC directory address mask low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_MASK_L_14_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1AE8) /* IOB_TX NC directory address mask low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_MASK_L_15_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1AF8) /* IOB_TX NC directory address mask low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_MASK_H_0_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A0C) /* IOB_TX NC directory address mask high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_MASK_H_1_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A1C) /* IOB_TX NC directory address mask high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_MASK_H_2_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A2C) /* IOB_TX NC directory address mask high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_MASK_H_3_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A3C) /* IOB_TX NC directory address mask high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_MASK_H_4_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A4C) /* IOB_TX NC directory address mask high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_MASK_H_5_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A5C) /* IOB_TX NC directory address mask high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_MASK_H_6_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A6C) /* IOB_TX NC directory address mask high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_MASK_H_7_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A7C) /* IOB_TX NC directory address mask high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_MASK_H_8_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A8C) /* IOB_TX NC directory address mask high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_MASK_H_9_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1A9C) /* IOB_TX NC directory address mask high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_MASK_H_10_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1AAC) /* IOB_TX NC directory address mask high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_MASK_H_11_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1ABC) /* IOB_TX NC directory address mask high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_MASK_H_12_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1ACC) /* IOB_TX NC directory address mask high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_MASK_H_13_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1ADC) /* IOB_TX NC directory address mask high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_MASK_H_14_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1AEC) /* IOB_TX NC directory address mask high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_ADDR_MASK_H_15_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1AFC) /* IOB_TX NC directory address mask high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_PRDNTY_ADDR_L_0_REG             (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1C00) /* IOB_TX producer notify address low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_PRDNTY_ADDR_L_1_REG             (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1C10) /* IOB_TX producer notify address low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_PRDNTY_ADDR_L_2_REG             (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1C20) /* IOB_TX producer notify address low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_PRDNTY_ADDR_L_3_REG             (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1C30) /* IOB_TX producer notify address low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_PRDNTY_ADDR_L_4_REG             (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1C40) /* IOB_TX producer notify address low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_PRDNTY_ADDR_L_5_REG             (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1C50) /* IOB_TX producer notify address low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_PRDNTY_ADDR_L_6_REG             (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1C60) /* IOB_TX producer notify address low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_PRDNTY_ADDR_L_7_REG             (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1C70) /* IOB_TX producer notify address low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_PRDNTY_ADDR_L_8_REG             (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1C80) /* IOB_TX producer notify address low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_PRDNTY_ADDR_L_9_REG             (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1C90) /* IOB_TX producer notify address low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_PRDNTY_ADDR_L_10_REG            (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1CA0) /* IOB_TX producer notify address low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_PRDNTY_ADDR_L_11_REG            (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1CB0) /* IOB_TX producer notify address low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_PRDNTY_ADDR_L_12_REG            (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1CC0) /* IOB_TX producer notify address low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_PRDNTY_ADDR_L_13_REG            (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1CD0) /* IOB_TX producer notify address low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_PRDNTY_ADDR_L_14_REG            (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1CE0) /* IOB_TX producer notify address low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_PRDNTY_ADDR_L_15_REG            (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1CF0) /* IOB_TX producer notify address low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_PRDNTY_ADDR_H_0_REG             (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1C04) /* IOB_TX producer notify address high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_PRDNTY_ADDR_H_1_REG             (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1C14) /* IOB_TX producer notify address high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_PRDNTY_ADDR_H_2_REG             (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1C24) /* IOB_TX producer notify address high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_PRDNTY_ADDR_H_3_REG             (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1C34) /* IOB_TX producer notify address high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_PRDNTY_ADDR_H_4_REG             (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1C44) /* IOB_TX producer notify address high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_PRDNTY_ADDR_H_5_REG             (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1C54) /* IOB_TX producer notify address high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_PRDNTY_ADDR_H_6_REG             (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1C64) /* IOB_TX producer notify address high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_PRDNTY_ADDR_H_7_REG             (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1C74) /* IOB_TX producer notify address high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_PRDNTY_ADDR_H_8_REG             (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1C84) /* IOB_TX producer notify address high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_PRDNTY_ADDR_H_9_REG             (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1C94) /* IOB_TX producer notify address high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_PRDNTY_ADDR_H_10_REG            (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1CA4) /* IOB_TX producer notify address high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_PRDNTY_ADDR_H_11_REG            (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1CB4) /* IOB_TX producer notify address high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_PRDNTY_ADDR_H_12_REG            (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1CC4) /* IOB_TX producer notify address high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_PRDNTY_ADDR_H_13_REG            (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1CD4) /* IOB_TX producer notify address high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_PRDNTY_ADDR_H_14_REG            (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1CE4) /* IOB_TX producer notify address high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_PRDNTY_ADDR_H_15_REG            (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1CF4) /* IOB_TX producer notify address high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_UNIT_REG                 (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1E00) /* IOB_TX NC directory address unit configure register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_NC_DIR_SKT_MAP_REG              (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1E04) /* IOB_TX NC directory socket bitmap register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_GLB_SNP_ADDR_EN_REG             (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1E20) /* IOB_TX global snoop address enable register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_GLB_SNP_ADDR_BASE_L_REG         (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1E24) /* IOB_TX global snoop base address low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_GLB_SNP_ADDR_BASE_H_REG         (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1E28) /* IOB_TX global snoop base address high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_GLB_SNP_ADDR_LIMIT_L_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1E2C) /* IOB_TX global snoop address limit low 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_GLB_SNP_ADDR_LIMIT_H_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1E30) /* IOB_TX global snoop address limit high 32bit */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_PAID_INTLV_REG                  (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1E34) /* IOB_TX PAID decode select register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_PAID_ORDER_REG                  (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1E38) /* IOB_TX PAID order field enable register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_PAID_MODE_REG                   (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1E3C) /* IOB_TX PAID decode mode register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_GLB_SNP_EN_REG                  (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1E40) /* IOB_TX global snoop enable register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_GLB_SNP_DONE_REG                (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1E44) /* IOB_TX global snoop done register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_IOB_TX_TCS_NC_CTRL_REG                 (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x1E50) /* TCS E state control register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_IDLE_REG                    (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2000) /* AP_IOB_TX idle status */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_AXIS_WR_CNT_0_REG           (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2400) /* DFX iob_tx_axis wr counter registers */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_AXIS_WR_CNT_1_REG           (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2404) /* DFX iob_tx_axis wr counter registers */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_AXIS_WR_CNT_2_REG           (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2408) /* DFX iob_tx_axis wr counter registers */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_AXIS_WR_CNT_3_REG           (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x240C) /* DFX iob_tx_axis wr counter registers */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_AXIS_WR_CNT_4_REG           (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2410) /* DFX iob_tx_axis wr counter registers */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_AXIS_WR_CNT_5_REG           (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2414) /* DFX iob_tx_axis wr counter registers */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_AXIS_RD_CNT_0_REG           (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2440) /* DFX iob_tx_axis rd counter registers */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_AXIS_RD_CNT_1_REG           (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2444) /* DFX iob_tx_axis rd counter registers */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_AXIS_STATUS0_REG            (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2480) /* DFX iob_tx_axis debug info register0 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_AXIS_STATUS1_REG            (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2484) /* DFX iob_tx_axis debug info register1 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_AXIS_STATUS2_REG            (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2488) /* DFX iob_tx_axis debug info register2 */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_AXIS_ABNORMAL_CNT_0_REG     (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2500) /* DFX iob_tx_axis abnormal counter registers */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_AXIS_ABNORMAL_CNT_1_REG     (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2504) /* DFX iob_tx_axis abnormal counter registers */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_AXIS_CTRL_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2580) /* DFX iob_tx_axis control registers */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_P_CNT_0_REG                 (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2800) /* The statistic DFX of Post TLP */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_P_CNT_1_REG                 (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2804) /* The statistic DFX of Post TLP */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_P_CNT_2_REG                 (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2808) /* The statistic DFX of Post TLP */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_P_CNT_3_REG                 (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x280C) /* The statistic DFX of Post TLP */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_P_CNT_4_REG                 (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2810) /* The statistic DFX of Post TLP */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_NP_CNT_0_REG                (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2880) /* The statistic DFX of NonPost TLP */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_NP_CNT_1_REG                (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2884) /* The statistic DFX of NonPost TLP */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_NP_CNT_2_REG                (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2888) /* The statistic DFX of NonPost TLP */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_NP_CNT_3_REG                (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x288C) /* The statistic DFX of NonPost TLP */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_NP_CNT_4_REG                (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2890) /* The statistic DFX of NonPost TLP */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_NP_CNT_5_REG                (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2894) /* The statistic DFX of NonPost TLP */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_NP_CNT_6_REG                (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2898) /* The statistic DFX of NonPost TLP */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_NP_CNT_7_REG                (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x289C) /* The statistic DFX of NonPost TLP */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_NP_CNT_8_REG                (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x28A0) /* The statistic DFX of NonPost TLP */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_NP_CNT_9_REG                (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x28A4) /* The statistic DFX of NonPost TLP */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_NP_CNT_10_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x28A8) /* The statistic DFX of NonPost TLP */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_NP_CNT_11_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x28AC) /* The statistic DFX of NonPost TLP */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_CPL_CNT_0_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2900) /* The statistic DFX of Completion TLP */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_CPL_CNT_1_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2904) /* The statistic DFX of Completion TLP */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_CPL_CNT_2_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2908) /* The statistic DFX of Completion TLP */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_REQ_CNT_REG                 (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2980) /* The statistic DFX of Post and NonPost in TypeA interface data channel */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_BAR_CNT_0_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x29A0) /* The statistic DFX of source allocation buffer request and release buffer request. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_BAR_CNT_1_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x29A4) /* The statistic DFX of source allocation buffer request and release buffer request. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_BAR_CNT_2_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x29A8) /* The statistic DFX of source allocation buffer request and release buffer request. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_BAR_CNT_3_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x29AC) /* The statistic DFX of source allocation buffer request and release buffer request. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_BAR_CNT_4_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x29B0) /* The statistic DFX of source allocation buffer request and release buffer request. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_BAR_CNT_5_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x29B4) /* The statistic DFX of source allocation buffer request and release buffer request. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_BAR_CNT_6_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x29B8) /* The statistic DFX of source allocation buffer request and release buffer request. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_BAR_CNT_7_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x29BC) /* The statistic DFX of source allocation buffer request and release buffer request. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_BAR_CNT_8_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x29C0) /* The statistic DFX of source allocation buffer request and release buffer request. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_ARR_CNT_0_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x29D0) /* The statistic DFX of source allocation buffer response and retried. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_ARR_CNT_1_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x29D4) /* The statistic DFX of source allocation buffer response and retried. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_ARR_CNT_2_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x29D8) /* The statistic DFX of source allocation buffer response and retried. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_ARR_CNT_3_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x29DC) /* The statistic DFX of source allocation buffer response and retried. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_ARR_CNT_4_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x29E0) /* The statistic DFX of source allocation buffer response and retried. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_ARR_CNT_5_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x29E4) /* The statistic DFX of source allocation buffer response and retried. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_ARR_CNT_6_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x29E8) /* The statistic DFX of source allocation buffer response and retried. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_ARR_CNT_7_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x29EC) /* The statistic DFX of source allocation buffer response and retried. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_ARR_CNT_8_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x29F0) /* The statistic DFX of source allocation buffer response and retried. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_STATUS0_REG                 (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2A00) /* The status0 of TQM */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_STATUS1_REG                 (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2A04) /* The status1 of TQM */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_STATUS2_REG                 (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2A08) /* The status2 of TQM */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_STATUS3_REG                 (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2A0C) /* The status3 of TQM */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_STATUS4_REG                 (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2A10) /* The status4 of TQM */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_STATUS5_REG                 (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2A14) /* The status5 of TQM */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_FIFO_STATUS_REG             (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2A80) /* The FIFO status of TQM */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_BAS_STATUS0_REG             (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2B00) /* The status 0 of BAS. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_BAS_STATUS1_REG             (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2B04) /* The status 1 of BAS. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_BAS_STATUS2_REG             (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2B08) /* The status 2 of BAS. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_ABNORMAL_CNT_0_REG          (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2C00) /* The statistic of abnormal event. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_ABNORMAL_CNT_1_REG          (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2C04) /* The statistic of abnormal event. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_ABNORMAL_CNT_2_REG          (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2C08) /* The statistic of abnormal event. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_ABNORMAL_CNT_3_REG          (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2C0C) /* The statistic of abnormal event. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_ERROR0_REG                  (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2C80) /* The error indication of the TQM. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_ERROR1_REG                  (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2C84) /* The error indication of the TQM. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_CTRL0_REG                   (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2E00) /* The DFX control0 register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_CTRL1_REG                   (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2E04) /* The DFX control1 register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_BAS_CTRL_REG                (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x2E10) /* The DFX control register of BAS. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_TCS_NORMAL_CNT_0_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x3000) /* The TCS DFX normal counter register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_TCS_NORMAL_CNT_1_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x3004) /* The TCS DFX normal counter register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_TCS_NORMAL_CNT_2_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x3008) /* The TCS DFX normal counter register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_TCS_NORMAL_CNT_3_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x300C) /* The TCS DFX normal counter register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_TCS_NORMAL_CNT_4_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x3010) /* The TCS DFX normal counter register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_TCS_NORMAL_CNT_5_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x3014) /* The TCS DFX normal counter register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_TCS_NORMAL_CNT_6_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x3018) /* The TCS DFX normal counter register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_TCS_NORMAL_CNT_7_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x301C) /* The TCS DFX normal counter register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_TCS_NORMAL_CNT_8_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x3020) /* The TCS DFX normal counter register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_TCS_NORMAL_CNT_9_REG        (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x3024) /* The TCS DFX normal counter register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_TCS_NORMAL_CNT_10_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x3028) /* The TCS DFX normal counter register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_TCS_NORMAL_CNT_11_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x302C) /* The TCS DFX normal counter register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_TCS_NORMAL_CNT_12_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x3030) /* The TCS DFX normal counter register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_TCS_NORMAL_CNT_13_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x3034) /* The TCS DFX normal counter register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_TCS_NORMAL_CNT_14_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x3038) /* The TCS DFX normal counter register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_TCS_NORMAL_CNT_15_REG       (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x303C) /* The TCS DFX normal counter register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_TCS_P2P_CNT_0_REG           (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x3080) /* The TCS DFX P2P counter register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_TCS_P2P_CNT_1_REG           (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x3084) /* The TCS DFX P2P counter register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_TCS_P2P_CNT_2_REG           (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x3088) /* The TCS DFX P2P counter register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_TCS_P2P_CNT_3_REG           (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x308C) /* The TCS DFX P2P counter register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_TCS_P2P_CNT_4_REG           (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x3090) /* The TCS DFX P2P counter register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_TCS_P2P_CNT_5_REG           (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x3094) /* The TCS DFX P2P counter register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_TCS_P2P_CNT_6_REG           (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x3098) /* The TCS DFX P2P counter register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_TCS_P2P_CNT_7_REG           (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x309C) /* The TCS DFX P2P counter register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_TCS_ARNORAML_CNT_0_REG      (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x3100) /* The TCS DFX abnormal counter register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_TCS_ARNORAML_CNT_1_REG      (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x3104) /* The TCS DFX abnormal counter register. */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_TCS_ERROR_REG               (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x3180) /* The TCS DFX error status */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_TCS_STATUS0_REG             (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x3200) /* The TCS DFX status0 register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_TCS_STATUS1_REG             (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x3204) /* The TCS DFX status1 register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_TCS_STATUS2_REG             (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x3208) /* The TCS DFX status2 register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_TCS_STATUS3_REG             (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x320C) /* The TCS DFX status3 register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_TCS_STATUS4_REG             (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x3210) /* The TCS DFX status4 register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_TCS_STATUS5_REG             (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x3214) /* The TCS DFX status5 register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_TCS_STATUS6_REG             (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x3218) /* The TCS DFX status6 register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_TCS_STATUS7_REG             (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x321C) /* The TCS DFX status7 register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_TCS_IDLE_REG                (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x3240) /* The TCS Idle register */
#define HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_DFX_IOB_TX_TCS_CTRL_REG                (HiPCIECTRL40V200_HIPCIEC_AP_IOB_TX_REG_BASE + 0x3300) /* The TCS DFX control */

#endif // __HIPCIEC50_AP_IOB_TX_REG_REG_OFFSET_H__
