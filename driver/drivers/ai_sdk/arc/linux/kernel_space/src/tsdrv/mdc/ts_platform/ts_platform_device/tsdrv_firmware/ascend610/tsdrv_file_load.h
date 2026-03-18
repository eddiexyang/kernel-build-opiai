/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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
#ifdef CFG_SOC_MDC_V51_LITE
#define DEVDRV_SEC_HEAD_SIZE 0
#else
#define DEVDRV_SEC_HEAD_SIZE 0x2100
#endif
#else
#define DEVDRV_SEC_HEAD_SIZE 0x4
#endif

#endif
