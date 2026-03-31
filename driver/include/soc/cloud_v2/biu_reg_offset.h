/*
 * Copyright     :  Copyright (C) 2022, Huawei Technologies Co. Ltd.
 * File name     :  biu_reg_offset.h
 * Project line  :  
 * Department    :  
 * Author        :  x198079
 * Version       :  V110
 * Date          :  Tester
 * Description   :  项目描述信息
 * Others        :  Generated automatically by nManager V5.1 
 * History       :  x198079 2022/03/04 09:31:54 Create file
 */

#ifndef __BIU_REG_OFFSET_H__
#define __BIU_REG_OFFSET_H__

/* BIU Base address of Module's Register */
#define BIU_BASE                       (0x0)

/******************************************************************************/
/*                      HiDMSS BIU Registers' Definitions                            */
/******************************************************************************/

#define BIU_BIU_CTRL0_REG           (BIU_BASE + 0x500) /* BIU控制寄存器0 */
#define BIU_BIU_CTRL1_REG           (BIU_BASE + 0x508) /* BIU控制寄存器1 */
#define BIU_BIU_CTRL2_REG           (BIU_BASE + 0x510) /* BIU控制寄存器2 */
#define BIU_BIU_STATUS2_REG         (BIU_BASE + 0x528) /* BIU状态寄存器2 */
#define BIU_BIU_STATUS3_REG         (BIU_BASE + 0x530) /* BIU状态寄存器3 */
#define BIU_BIU_STATUS4_REG         (BIU_BASE + 0x538) /* BIU状态寄存器4 */
#define BIU_BIU_STATUS5_REG         (BIU_BASE + 0x540) /* BIU状态寄存器5 */
#define BIU_SMMU_STREAMID_REG       (BIU_BASE + 0x548) /* THE STREAMID_FOR BIU */
#define BIU_L2_REMAP_PADDR_BASE_REG (BIU_BASE + 0x550) /* L2 PHY_ADDR_BASE */
#define BIU_BIU_L2_SIZE_REG         (BIU_BASE + 0x558) /* L2 SIZE */
#define BIU_BIU_L2_PAGE_SIZE_REG    (BIU_BASE + 0x560) /* L2_PAGE_SIZE */
#define BIU_BIU_CTRL6_REG           (BIU_BASE + 0x568) /* BIU控制寄存器6 */
#define BIU_BIU_STATUS8_REG         (BIU_BASE + 0x570) /* BIU状态寄存器8 */
#define BIU_BIU_STREAM_NS_REG       (BIU_BASE + 0x578) /* 'BIU stream NS bit */
#define BIU_AIC_ERROR_REG           (BIU_BASE + 0x700) 
#define BIU_AIC_ERROR_MASK_REG      (BIU_BASE + 0x708) /* error mask signal */
#define BIU_BIU_ERR_INFO_REG        (BIU_BASE + 0x710) /* L2 remap out of boundry */
#define BIU_L2_REMAP_CFG0_REG       (BIU_BASE + 0xB8)  /* L2 REMAPPING CONFIG REGISTER0 */
#define BIU_L2_REMAP_CFG1_REG       (BIU_BASE + 0xC0)  /* L2 REMAPPING CONFIG REGISTER1 */
#define BIU_L2_REMAP_CFG2_REG       (BIU_BASE + 0xC8)  /* L2 REMAPPING CONFIG REGISTER2 */
#define BIU_L2_REMAP_CFG3_REG       (BIU_BASE + 0xD0)  /* L2 REMAPPING CONFIG REGISTER3 */
#define BIU_L2_REMAP_CFG4_REG       (BIU_BASE + 0xD8)  /* L2 REMAPPING CONFIG REGISTER4 */
#define BIU_L2_REMAP_CFG5_REG       (BIU_BASE + 0xE0)  /* L2 REMAPPING CONFIG REGISTER5 */
#define BIU_L2_REMAP_CFG6_REG       (BIU_BASE + 0xE8)  /* L2 REMAPPING CONFIG REGISTER6 */
#define BIU_L2_REMAP_CFG7_REG       (BIU_BASE + 0xF0)  /* L2 REMAPPING CONFIG REGISTER7 */
#define BIU_L2_REMAP_VADDR_BASE_REG (BIU_BASE + 0xF8)  /* L2 VADDR_BASE */
#define BIU_SMMU_SUBSTREAMID_REG    (BIU_BASE + 0x90)  /* THE STREAMID_FOR BIU */
#define BIU_TASK_CFG_REG            (BIU_BASE + 0x98)  /* BIU read or write cmd axprot */
#define BIU_BIU_CTRL7_REG           (BIU_BASE + 0x580) /* BIU CTRl7 */

#endif // __BIU_REG_OFFSET_H__
