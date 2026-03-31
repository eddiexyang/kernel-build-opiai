/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
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
 * Create: 2020-4-1
 */

#ifndef PNGD_DRV_H
#define PNGD_DRV_H

#include "hi_errno.h"
#include "hi_debug.h"

#define PNGD_EMERG_TRACE(fmt, ...)                                                                            \
do {                                                                                                    \
    HI_EMERG_TRACE(HI_ID_PNGD, "[PNGD]:%s [Line]:%d " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
} while (0)

#define PNGD_ALERT_TRACE(fmt, ...)                                                                            \
do {                                                                                                    \
    HI_ALERT_TRACE(HI_ID_PNGD, "[PNGD]:%s [Line]:%d " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
} while (0)

#define PNGD_CRIT_TRACE(fmt, ...)                                                                            \
do {                                                                                                    \
    HI_CRIT_TRACE(HI_ID_PNGD, "[PNGD]:%s [Line]:%d " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
} while (0)

#define PNGD_ERR_TRACE(fmt, ...)                                                                            \
do {                                                                                                    \
    HI_ERR_TRACE(HI_ID_PNGD, "[PNGD]:%s [Line]:%d " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
} while (0)

#define PNGD_WARN_TRACE(fmt, ...)                                                                            \
do {                                                                                                    \
    HI_WARN_TRACE(HI_ID_PNGD, "[PNGD]:%s [Line]:%d " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
} while (0)

#define PNGD_NOTICE_TRACE(fmt, ...)                                                                            \
do {                                                                                                    \
    HI_NOTICE_TRACE(HI_ID_PNGD, "[PNGD]:%s [Line]:%d " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
} while (0)

#define PNGD_INFO_TRACE(fmt, ...)                                                                            \
do {                                                                                                    \
    HI_INFO_TRACE(HI_ID_PNGD, "[PNGD]:%s [Line]:%d " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
} while (0)

#define PNGD_DEBUG_TRACE(fmt, ...)                                                                           \
do {                                                                                                    \
    HI_DEBUG_TRACE(HI_ID_PNGD, "[PNGD]:%s [Line]:%d " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
} while (0)

#endif // end PNGD_DRV_H