/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
 * Create: 2023-08-31
 */
#ifndef SAFETY_RAS_REG_CHECK_310_H
#define SAFETY_RAS_REG_CHECK_310_H

#include <linux/module.h>
#include <asm/barrier.h>
#include "dms_sensor.h"
#include "dms_sensor_type.h"
#include "drv_log.h"
#include "dms_define.h"
#include "safety_ras_report_310.h"

#define DDR_REG_READ_CHECK_FAIL_REASON       0xFFU
#define DDR_REG_READ_CHECK_EVENT_TYPE        0x6U

#define REG_READ_CHECK_FAIL_FLAG             0x87654321U
#define REG_READ_CHECK_SHAREMEM_BASE_ADDR    0x402004DACU    // 共享内存起始地址
#define IO_REMAP_SIZE                        0x10U         // 内存地址映射大小
#define DDR_REG_REAG_CHECK_RESULT_OFFSET     0x0U

int32_t init_memory_pm_notifier(void);
int32_t deinit_memory_pm_notifier(void);
int32_t init_check_ddr_reg_read_result(void);
int32_t deinit_check_ddr_reg_read_result(void);
extern uint8_t memory_get_queue_index(uint8_t senseor_type, uint8_t chn);

extern struct drv_memory_devices *g_memory_devs_ptr;

#endif /* SAFETY_RAS_REG_CHECK_310_H */