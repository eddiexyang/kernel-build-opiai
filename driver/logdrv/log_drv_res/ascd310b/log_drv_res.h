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
#define LOG_DEVICE_OFFSET 0
#define LOG_DIE_HIGH_OFFSET 0

#define LOG_TS_BUFFER_LEN (1024 * 1024)

#define LOG_BIOS_BUFFER_LEN (4 * 1024 *1024)
#define BIOS_RESERVED_MEM_ADDR (0x22B40000)

#define LOG_HSM_BUFFER_LEN (1024 *1024)
#define HSM_RESERVED_MEM_ADDR (0x3000000)

#define LOG_LPM3_BUFFER_LEN (128 * 1024)
#define LPM3_RESERVED_MEM_ADDR (0x3100000)

#endif /* LOG_DRV_RES_H */