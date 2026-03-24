/*
 * Copyright     :  Copyright (C) 2021, Huawei Technologies Co. Ltd.
 * File name     :  gicr_lowid_regs_reg_offset.h
 * Project line  :  Platform And Key Technologies Development
 * Department    :  CAD Development Department
 * Author        :  xxx
 * Version       :  1
 * Date          :  2013/3/10
 * Description   :  The description of xxx project
 * Others        :  Generated automatically by nManager V5.1 
 * History       :  xxx 2021/10/25 08:35:56 Create file
 */

#ifndef __GICR_LOWID_REGS_REG_OFFSET_H__
#define __GICR_LOWID_REGS_REG_OFFSET_H__

/* GICR_LOWID_REGS Base address of Module's Register */
#define SOC_GICR_LOWID_REGS_BASE                       (0x60000000)

/******************************************************************************/
/*                      SOC GICR_LOWID_REGS Registers' Definitions                            */
/******************************************************************************/

#define SOC_GICR_LOWID_REGS_GICR_IGROUPR0_REG                    (SOC_GICR_LOWID_REGS_BASE + 0x80)  /* 标识对应中断ID的范围为{0，31}分组至Group0或Group1 */
#define SOC_GICR_LOWID_REGS_GICR_ISENABLER0_REG                  (SOC_GICR_LOWID_REGS_BASE + 0x100) /* 中断ID{0，31}中单个中断的使能寄存器，决定是否发给Cpu Interface。 */
#define SOC_GICR_LOWID_REGS_GICR_ICENABLER0_REG                  (SOC_GICR_LOWID_REGS_BASE + 0x180) /* 中断ID{0，31}单个中断的清除使能的寄存器，确定是否发给CPU Interface。 */
#define SOC_GICR_LOWID_REGS_GICR_ISPENDR0_REG                    (SOC_GICR_LOWID_REGS_BASE + 0x200) /* lowid中断中单个中断设置pending状态的控制信号寄存器 */
#define SOC_GICR_LOWID_REGS_GICR_ICPENDR0_REG                    (SOC_GICR_LOWID_REGS_BASE + 0x280) /* lowid中断中单个中断清除pending状态的控制信号寄存器。 */
#define SOC_GICR_LOWID_REGS_GICR_ISACTIVER0_REG                  (SOC_GICR_LOWID_REGS_BASE + 0x300) /* lowid中断中单个中断设置active状态的控制信号寄存器。 */
#define SOC_GICR_LOWID_REGS_GICR_ICACTIVER0_REG                  (SOC_GICR_LOWID_REGS_BASE + 0x380) /* lowid中断中单个中断清除active状态的控制信号寄存器，这些寄存器在需要保存和回复GIC的状态时使用。 */
#define SOC_GICR_LOWID_REGS_GICR_IPRIORITY_LOWSGI_NS_0_REG       (SOC_GICR_LOWID_REGS_BASE + 0x400) /* 每8bit对应一个中断的优先级。x{0,3} */
#define SOC_GICR_LOWID_REGS_GICR_IPRIORITY_LOWSGI_NS_1_REG       (SOC_GICR_LOWID_REGS_BASE + 0x404) /* 每8bit对应一个中断的优先级。x{0,3} */
#define SOC_GICR_LOWID_REGS_GICR_IPRIORITY_LOWSGI_NS_2_REG       (SOC_GICR_LOWID_REGS_BASE + 0x408) /* 每8bit对应一个中断的优先级。x{0,3} */
#define SOC_GICR_LOWID_REGS_GICR_IPRIORITY_LOWSGI_NS_3_REG       (SOC_GICR_LOWID_REGS_BASE + 0x40C) /* 每8bit对应一个中断的优先级。x{0,3} */
#define SOC_GICR_LOWID_REGS_GICR_IPRIORITY_LOWSGI_S_0_REG        (SOC_GICR_LOWID_REGS_BASE + 0x400) /* 每8bit对应一个中断的优先级。x{0,3} */
#define SOC_GICR_LOWID_REGS_GICR_IPRIORITY_LOWSGI_S_1_REG        (SOC_GICR_LOWID_REGS_BASE + 0x404) /* 每8bit对应一个中断的优先级。x{0,3} */
#define SOC_GICR_LOWID_REGS_GICR_IPRIORITY_LOWSGI_S_2_REG        (SOC_GICR_LOWID_REGS_BASE + 0x408) /* 每8bit对应一个中断的优先级。x{0,3} */
#define SOC_GICR_LOWID_REGS_GICR_IPRIORITY_LOWSGI_S_3_REG        (SOC_GICR_LOWID_REGS_BASE + 0x40C) /* 每8bit对应一个中断的优先级。x{0,3} */
#define SOC_GICR_LOWID_REGS_GICR_IPRIORITY_LOWPPI2023_NS_REG     (SOC_GICR_LOWID_REGS_BASE + 0x414) /* 每8bit对应一个中断的优先级。 */
#define SOC_GICR_LOWID_REGS_GICR_IPRIORITY_LOWPPI2023_S_REG      (SOC_GICR_LOWID_REGS_BASE + 0x414) /* 每8bit对应一个中断的优先级。 */
#define SOC_GICR_LOWID_REGS_GICR_IPRIORITY_LOWPPI23TO31_NS_0_REG (SOC_GICR_LOWID_REGS_BASE + 0x418) /* 每8bit对应一个中断的优先级。y{0,1} */
#define SOC_GICR_LOWID_REGS_GICR_IPRIORITY_LOWPPI23TO31_NS_1_REG (SOC_GICR_LOWID_REGS_BASE + 0x41C) /* 每8bit对应一个中断的优先级。y{0,1} */
#define SOC_GICR_LOWID_REGS_GICR_IPRIORITY_LOWPPI23TO31_S_0_REG  (SOC_GICR_LOWID_REGS_BASE + 0x418) /* 每8bit对应一个中断的优先级。y{0,1} */
#define SOC_GICR_LOWID_REGS_GICR_IPRIORITY_LOWPPI23TO31_S_1_REG  (SOC_GICR_LOWID_REGS_BASE + 0x41C) /* 每8bit对应一个中断的优先级。y{0,1} */
#define SOC_GICR_LOWID_REGS_GICR_ICFGR_LOW_SGI_REG               (SOC_GICR_LOWID_REGS_BASE + 0xC00) /* 该寄存器每2bit表示一个SGI中断是电平敏感还是边沿触发 */
#define SOC_GICR_LOWID_REGS_GICR_ICFGR_LOW_PPI_REG               (SOC_GICR_LOWID_REGS_BASE + 0xC04) /* 该寄存器每2bit表示一个PPI中断是电平敏感还是边沿触发 */
#define SOC_GICR_LOWID_REGS_GICR_NSACR_REG                       (SOC_GICR_LOWID_REGS_BASE + 0xE00) /* 控制是否允许非安全软件产生安全SGI中断寄存器 */
#define SOC_GICR_LOWID_REGS_GICR_IGRPMODR0_REG                   (SOC_GICR_LOWID_REGS_BASE + 0xD00) /* 标识对应中断ID的范围为{0，31}分组的修饰 */

#endif // __GICR_LOWID_REGS_REG_OFFSET_H__
