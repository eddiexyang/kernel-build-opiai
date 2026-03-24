/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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


#ifndef __DEVDRV_PM_H
#define __DEVDRV_PM_H

#include <linux/list.h>
#include <linux/time.h>
#include <linux/hrtimer.h>

#include "devdrv_common.h"
#include "tsdrv_status.h"

#define DEVDRV_WAKELOCK_TIMEOUT_SECOND 10 /* second */

struct devdrv_pm {
    int (*suspend)(u32 devid, u32 status);
    int (*resume)(u32 devid);
    int run_stage; /* 0-platform call, 1-sleep call */
    struct list_head list;
};

struct devdrv_msg_pm {
    int (*suspend)(void);
    int (*resume)(void);
    struct list_head list;
};

int devdrv_manager_msg_suspend(void);
int devdrv_manager_msg_resume(void);
void *devdrv_manager_register_msg_pm(int (*suspend)(void), int (*resume)(void));
void devdrv_manager_unregister_msg_pm(void *msg_pm_p);

int devdrv_manager_suspend(struct devdrv_info *info);
int devdrv_manager_resume(struct devdrv_info *info);
int devdrv_manager_get_hw_info(struct devdrv_info *dev_info);
struct devdrv_pm *devdrv_manager_register_pm(int run_stage, int (*suspend)(u32 devid, u32 status), int (*resume)(u32 devid));
void devdrv_manager_unregister_pm(struct devdrv_pm *pm);

void devdrv_ts_exception_task(unsigned long data);
void tsdrv_heart_beat_ai_down(u32 devid, u32 tsid, const void *data);
irqreturn_t devdrv_nfe_handler(int irq, void *data);

void devdrv_m3_notice_aicore_freq_state(int cmd);
void devdrv_inform_lpm3_change_aicore_freq(int gpio_value, int flag);
int devdrv_inform_lpm3_upper_ddr_freq(u32 devid);
int devdrv_inform_lpm3_lower_ddr_freq(u32 devid);
void tsdrv_heart_beat_ai_down(u32 devid, u32 tsid, const void *data);

/*
 * black box
 */
#define MNTN_LPM3_HEART_BEAT_ERROR_CODE 0xA619FFFFU
#define MNTN_IMU_HEART_BEAT_ERROR_CODE 0xA62FFFFFU

typedef struct ts_physical_addr_t {
    u64 addr;
    u32 len;
} ts_physical_addr;

enum MNTN_BUFFER_TYPE {
    TS_MNTN_BUFFER = 0x0,     // ts buffer, static reserved, size: 512k
    TS_MNTN_START_LOG_BUFFER, // ts start log buffer, dynamic mallocced, size: 50k
    MNTN_BUFFER_TYPE_MAX,
};

typedef struct bbox_config_t {
    u8 enable_bbox; // (0x1 enable for liteos vmcore);
    ts_physical_addr ts_paddr[MNTN_BUFFER_TYPE_MAX];
} bbox_tsconfig;


bbox_tsconfig *bbox_get_tsconfig(u32 dev_id);

/*
 * os heart beat
 */
#define OS_HEART_BEAT_MAX (0xFFFFFFFFFFFFFFFFULL - 1)
#define OS_HEART_BEAT_INTERVAL 5 /* 5 second */

int devdrv_get_boardid(void);
bool devdrv_is_pci_rc_mode(void);

struct os_heart_beat {
    struct hrtimer timer;
    u64 count;
};
void devdrv_os_heart_beat_exit(void);
int agentdrv_set_heartbeat_count(u32 devid, u64 count);
void devdrv_refresh_error_code_init(void);
void devdrv_refresh_error_code_exit(void);


#endif
