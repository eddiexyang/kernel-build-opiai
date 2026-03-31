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
#ifndef TSDRV_FILE_LOAD_H
#define TSDRV_FILE_LOAD_H

#ifndef TSDRV_UT
#define DEVDRV_SEC_HEAD_SIZE 0x2100
#else
#define DEVDRV_SEC_HEAD_SIZE 0
#endif

#define DIE0_FFTS_RESERVED_ADDR  0x1B424000
#define DIE1_FFTS_RESERVED_ADDR  0x101B424000
#define FFTS_RESERVED_SIZE (8 * 1024 * 1024)
#define FFTS_PERIPHCTRL70 0x01B4  // cpu2
#define FFTS_PERIPHCTRL68 0x01AC  // cpu1
#define FFTS_PERIPHCTRL12 0x00CC
#define FFTS_SC_TESTREG10 0x0C08
#define FFTS_DFX_TESTREG6 0xBF8
#define FFTS_SC_TESTREG_INIT 0
#define FFTS_SC_TESTREG_RDY 0x06
#define FFTS_ADDR_HIGH_BIT 32

#endif
