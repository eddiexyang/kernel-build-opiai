/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
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
 * Create: 2019-10-15
 */

#ifndef DEVDRV_PLATFORM_H
#define DEVDRV_PLATFORM_H

#ifndef AOS_LLVM_BUILD
#include <linux/irqchip/arm-gic-v3.h>
#endif
#include "devdrv_common.h"
#include "devdrv_parse_pdata.h"
#ifdef SNAPSHOT_ENABLE
#include "drv_snapshot.h"
#endif
#ifdef AOS_LLVM_BUILD
#define acpi_disabled 1
#endif
#define DEVDRV_PLAT_TYPE_FPGA        0x0
#define DEVDRV_PLAT_TYPE_EMU         0x1
#define DEVDRV_PLAT_TYPE_ESL         0x2
#define DEVDRV_PLAT_TYPE_ASIC        0x3
#define DEVDRV_PLAT_TYPE_INVALID     0xFF
#define DEVDRV_PLAT_VERSION_MASK     0x00000FFF
#define DEVDRV_PLAT_AI_CORE_NUM_MASK 0x0000F000
#define DEVDRV_PLAT_MASK             0x000F0000
#define DEVDRV_PLAT_OFFSET           16
#define DEVDRV_PLAT_AI_CORE_NUM_2    0x00002000
#define DEVDRV_PLAT_AI_CORE_NUM_1    0x00000000
#define DEVDRV_PLAT_AI_CORE_NUM_3    0x00006000

typedef enum {
    TS_PLATFORM_SUSPEND_START = 0,
    TS_PLATFORM_SUSPEND_DEVMNG,
    TS_PLATFORM_SUSPEND_PM,
    TS_PLATFORM_SUSPEND_PM_START,
    TS_PLATFORM_SUSPEND_PM_NOT_READY,
    TS_PLATFORM_SUSPEND_RESET_TSCPU,
    TS_PLATFORM_SUSPEND_EXPECT = 50
} TS_PLATFORM_SNAPSHOT_SUSPEND;

typedef enum {
    TS_PLATFORM_RESUME_START = 0,
    TS_PLATFORM_RESUME_DEVMNG,
    TS_PLATFORM_RESUME_PM,
    TS_PLATFORM_RESUME_DERESET_TSCPU,
    TS_PLATFORM_RESUME_PM_NOT_READY = 20,
    TS_PLATFORM_RESUME_EXPECT = 50
} TS_PLATFORM_SNAPSHOT_RESUME;

#ifdef SNAPSHOT_ENABLE
    #define TSDRV_BOOTDOT(curr_state) drv_snapshot_bootdot_set(TS_PLATFORM_MODULE_ID, curr_state)
    #define TSDRV_BOOTDOT_INIT(init_state, expect_state) \
        drv_snapshot_bootdot_init(TS_PLATFORM_MODULE_ID, init_state, expect_state)
    #define TSDRV_BOOTDOT_TS_INTERVAL 10
    #define TSDRV_BOOTDOT_BY_TSID(curr_state, tsid) TSDRV_BOOTDOT((curr_state + TSDRV_BOOTDOT_TS_INTERVAL * (tsid)))
#else
    #define TSDRV_BOOTDOT(curr_state)
    #define TSDRV_BOOTDOT_INIT(init_state, expect_state)
    #define TSDRV_BOOTDOT_BY_TSID(curr_state, tsid)
#endif

u64 devdrv_get_addr_base(enum devdrv_dts_addr_index idx, u32 chip_id);
int tsdrv_get_ffts_mcu_irq_id(u32 dev_id, u32 *hwirq);
int devdrv_get_tscpu_irq_info(u32 dev_id, u32 *start_irq, u32 *irq_num);
int devdrv_set_irq_affinity(unsigned int irq, const struct cpumask *cpumask);
struct devdrv_info *tsdrv_get_devinfo(u32 devid);

#endif /* __DEVDRV_PLATFORM_H */
