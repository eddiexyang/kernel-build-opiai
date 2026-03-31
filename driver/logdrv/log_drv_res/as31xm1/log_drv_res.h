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
 * Create: 2023-6-20
 */
#ifndef LOG_DRV_RES_H
#define LOG_DRV_RES_H


#define LOG_CHIP_OFFSET 0
#define LOG_DEVICE_OFFSET 0
#define LOG_DIE_HIGH_OFFSET 0

#define LOG_TS_BUFFER_LEN (1024 * 1024)
#define TS_RESERVED_MEM_ADDR (0x9D80000)

#define LOG_BIOS_BUFFER_LEN (2 * 1024 * 1024)
#define BIOS_RESERVED_MEM_ADDR (0xC000000)

#define LOG_HSM_BUFFER_LEN (512 * 1024)
#define HSM_RESERVED_MEM_ADDR (0x1A80000)

#define LOG_LPM3_BUFFER_LEN (128 * 1024)
#define LPM3_RESERVED_MEM_ADDR (0x1B00000)

#endif /* LOG_DRV_RES_H */