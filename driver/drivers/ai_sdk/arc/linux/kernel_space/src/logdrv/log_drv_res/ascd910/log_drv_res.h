/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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
 * Create: 2022-08-13
 */
#ifndef LOG_DRV_RES_H
#define LOG_DRV_RES_H

#define LOG_CHIP_OFFSET 0
#define LOG_DEVICE_OFFSET 0x0200000000000
#define LOG_DIE_HIGH_OFFSET 0

#define LOG_BUFF_SIZE_TS (1024 * 1024)
#define LOG_BUFF_SIZE_IMU_START (1024 * 1024)
#define LOG_BUFF_SIZE_UEFI_START (3 * 1024 * 1024)
#define LOG_BUFF_SIZE_IMU (4 * 1024 * 1024)
#define LOG_BUFF_SIZE_IMP (1024 * 1024)

#define LOG_CHANNEL_IMU_START_ADDR 0x5f800000
#define LOG_CHANNEL_UEFI_START_ADDR 0x5f900000
#define LOG_CHANNEL_IMU_RUN_ADDR 0x5fc00000
#define LOG_CHANNEL_IMP_RUN_ADDR 0x72d00000

#endif /* LOG_DRV_RES_H */

