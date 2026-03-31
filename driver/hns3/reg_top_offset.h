/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: reg_top_offset
 * Author: huawei
 * Create: 2021-12-28
 */

#ifndef __REG_TOP_OFFSET_H__
#define __REG_TOP_OFFSET_H__

/* TOP Base address of Module's Register */
#define TOP_BASE                       (0x0)

/******************************************************************************/
/*                      xxx TOP Registers' Definitions                        */
/******************************************************************************/

#define TOP_DEVICE_ID_REG           (TOP_BASE + 0x0)   /* DEVICE_ID的设备ID。 */
#define TOP_VENDER_ID_REG           (TOP_BASE + 0x4)   /* 厂商ID。 */
#define TOP_NIC_IDLE_REG            (TOP_BASE + 0x410) /* 模块内部IDLE状态指示寄存器。 */
#define TOP_MST_OOO_WR_BP_REG       (TOP_BASE + 0x440) /* OOO写延时反压水线配置 */
#define TOP_MST_OOO_RD_BP_REG       (TOP_BASE + 0x444) /* OOO读延时反压水线配置 */
#define TOP_MST0_OOO_LATENCY_REG    (TOP_BASE + 0x448) /* 访问Master OOO 总线时延寄存器 */
#define TOP_BP_CPU_TIMEOUT_REG      (TOP_BASE + 0x500) /* 反压CPU的超时控制 */
#define TOP_BP_CPU_STATE_REG        (TOP_BASE + 0x504) /* 反压CPU的状态 */
#define TOP_CFG_ETH_JAM_QOS_REG     (TOP_BASE + 0x508) /* 反压CPU的qos配置 */
#define TOP_AXI_SLV_POISON_MASK_REG (TOP_BASE + 0x510) /* AXI Slave poison标志mask配置寄存器 */

#endif // __REG_TOP_OFFSET_H__
