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

#define LOG_CHIP_OFFSET     0x80000000000
#define LOG_DEVICE_OFFSET   0x10000000000
#define LOG_DIE_HIGH_OFFSET 0x1000000000

#define LOG_BUFF_SIZE_TS (1024 * 1024)

#define LOG_HSM_BUFFER_LEN (1024 * 1024)
#define HSM_RESERVED_MEM_ADDR (0x600000)

#define LOG_IMP_BUFFER_LEN (1 * 1024 * 1024)
#define IMP_RESERVED_MEM_ADDR (0x31E00000)

#define LOG_LPM3_BUFFER_LEN (128 * 1024)
#define LPM3_RESERVED_MEM_ADDR (0x36500000)

#endif /* LOG_DRV_RES_H */

