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

#ifndef ASCEND_CTL_SYSFS_H
#define ASCEND_CTL_SYSFS_H

#include <linux/platform_device.h>
#include <linux/semaphore.h>

#define SMP_DEV_MAX 4
#define BOARD_ID_LEN 4
#define SYSFS_SHOW_SIZE_MAX 64
#define USER_CFG_GET_CPU_CFG_FUNC_NAME "dev_user_cfg_get_cpu_cfg"

/* board id */
#define BOARD_ID_DC_2P_150 150
#define BOARD_ID_DC_2P_151 151
#define BOARD_ID_DC_2P_153 153
#define BOARD_ID_DC_2P_160 160
#define BOARD_ID_DC_2P_EVB_A 900
#define BOARD_ID_DC_2P_EVB_B 901
#define BOARD_ID_HELPER_2P_PG 475

#define SMP_DEV_NUM_1P 1
#define SMP_DEV_NUM_2P 2
#define SMP_DEV_NUM_4P 4

typedef struct _cpu_cfg {
    unsigned int ctrl_cpu_num;
    unsigned int data_cpu_num;
    unsigned int aicpu_num;
} cpu_cfg_t;

typedef struct _ascend_ctl_sysfs {
    struct device *dev;
    struct kobject *kobj;
    unsigned int smp_dev_num;
    cpu_cfg_t dts_cpu_cfg;
    cpu_cfg_t flash_cpu_cfg;
    struct semaphore flash_cpu_cfg_sema;
    bool ready;
} ascend_ctl_sysfs_t;

typedef int (*get_cpu_cfg_handle_t)(unsigned int dev_id, cpu_cfg_t *data, unsigned int size);

void ascend_ctl_set_platform_device(struct platform_device *pdev);
void ascend_ctl_get_dts_cpu_cfg(struct platform_device *pdev);
void ascend_ctl_check_dts_cpu_cfg_valid(void);
int ascend_ctl_sysfs_init(void);
void ascend_ctl_sysfs_exit(void);

#endif
