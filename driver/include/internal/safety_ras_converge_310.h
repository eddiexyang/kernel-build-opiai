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
 * Create: 2022-01-20
 */
#ifndef SAFETY_RAS_CONVERGE_310_H
#define SAFETY_RAS_CONVERGE_310_H

#include "dms_define.h"
#include "safety_ras_init_310.h"

#define MEM_ERR_STATUS 0x1U

struct ddr_event_data {
	uint32_t excep_id;            // bbox exception id
	uint8_t timestamp_sec[8];     // 8 bytes for second
	uint8_t timestamp_usec[8];    // 8 bytes for usecond
	uint16_t event_id;            // fault event id
} __attribute__((packed));

// detail err code, inner use
enum {
	MEMORY_EXP_OB = 0x37U,           // 地址膨胀后越界中断
	MEMORY_SBRAM_1ERR = 0x36U,       // SBRAM 1bit错误中断
	MEMORY_SBRAM_DERR = 0x35U,       // SBRAM 2bit错误中断
	MEMORY_RBUF_SERR = 0x34U,        // IECC_RBUF 1bit错误中断
	MEMORY_RBUF_DERR = 0x33U,        // IECC_RBUF 2bit错误中断
	MEMORY_WBUF_SERR = 0x32U,        // IECC_WBUF 1bit错误中断
	MEMORY_WBUF_DERR = 0x31U,        // IECC_WBUF 2bit错误中断
	MEMORY_IECC_SERR = 0x30U,        // IECC 1bit错误中断
	MEMORY_IECC_DERR = 0x2FU,        // IECC 2bit错误中断
	MEMORY_RD_TIME_OUT = 0x29U,      // DDRPHY 读数据超时中断
	MEMORY_AREF_ALARM = 0x28U,       // DDR AREF命令错误中断
	MEMORY_SBRAM_MERR = 0x27U,       // SBRAM多比特错误中断
	MEMORY_RPRAM_MERR = 0x26U,       // RPRAM多比特错误中断
	MEMORY_SREF_ERR = 0x25U,         // 自刷新接口有命令访问中断
	MEMORY_WSRAM_MERR = 0x24U,       // WSRAM多比特错误中断
	MEMORY_PHYRAM_MERR = 0x22U,      // phyram多bit中断
	MEMORY_UCERAM_MERR = 0x23U,      // uceram多bit中断
	MEMORY_WSRAM_SERR = 0x2EU,       // WSRAM单比特错误中断
	MEMORY_SBRAM_SERR = 0x2DU,       // SBRAM单比特错误中断
	MEMORY_RPRAM_SERR = 0x2CU,       // RPRAM单比特错误中断
	MEMORY_PHYRAM_SERR = 0x2BU,      // phyram单bit中断状态
	MEMORY_UCERAM_SERR = 0x2AU,      // uceram单bit中断状态
	MEMORY_HIGH_TEMPERATURE = 0x21U, // 高温中断状态
	MEMORY_LOW_TEMPERATURE = 0x20U,  // 低温中断状态
};
#endif /* SAFETY_RAS_CONVERGE_310_H */