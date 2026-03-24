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
#define LOG_DEVICE_OFFSET 0x8000000000
#define LOG_DIE_HIGH_OFFSET 0

#define LOG_TS_BUFFER_LEN (1024 * 1024)
#define TS1_DUMP_RESERVED_MEM_ADDR 0x28F00000
#define TSCPU1_RESERVED_MEM_ADDR_RUN (TS1_DUMP_RESERVED_MEM_ADDR + LOG_TS_BUFFER_LEN)
#define TSCPU1_RESERVED_MEM_ADDR_START (TSCPU1_RESERVED_MEM_ADDR_RUN + LOG_TS_BUFFER_LEN)
#define TS0_DUMP_RESERVED_MEM_ADDR (TSCPU1_RESERVED_MEM_ADDR_START + LOG_TS_BUFFER_LEN)
#define TSCPU0_RESERVED_MEM_ADDR_RUN (TS0_DUMP_RESERVED_MEM_ADDR + LOG_TS_BUFFER_LEN)

#define TS_MDC_CHANNEL_NUM 2

#define LOG_LP_BUFFER_LEN (128 * 1024)
#define LP_RESERVED_MEM_ADDR 0xA00000

#define LOG_ISP_BUFFER_LEN (1024 * 1024)
#define ISP_CHANNEL_NUM 4
#define ISP0_RESERVED_MEM_ADDR 0x2B701000
#define ISP1_RESERVED_MEM_ADDR (ISP0_RESERVED_MEM_ADDR + LOG_ISP_BUFFER_LEN)
#define ISP2_RESERVED_MEM_ADDR (ISP1_RESERVED_MEM_ADDR + LOG_ISP_BUFFER_LEN)
#define ISP3_RESERVED_MEM_ADDR (ISP2_RESERVED_MEM_ADDR + LOG_ISP_BUFFER_LEN)

#define LOG_SIS_BUFFER_LEN ((100 * 1024) + 128)
#define SIS_RESERVED_MEM_ADDR 0xc6f16400

#define LOG_HSM_BUFFER_LEN (1024 * 1024)
#define HSM_RESERVED_MEM_ADDR 0x3E00000

#define LOG_BIOS_BUFFER_LEN (1024 * 1024)
#define BIOS_RESERVED_MEM_ADDR 0xB300000

#define LOG_BIOS_ATF_BUFFER_LEN (1024 * 1024)
#define BIOS_ATF_RESERVED_MEM_ADDR 0xB400000

#define LOG_SIS_BIST_BUFFER_LEN (100 * 1024 + 128)
#define SIS_BIST_RESERVED_MEM_ADDR 0x2E587F80

#endif /* LOG_DRV_RES_H */
