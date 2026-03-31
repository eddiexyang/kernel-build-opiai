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

#ifndef DEV_UPGRADE_DEF_H
#define DEV_UPGRADE_DEF_H

#include "drv_log.h"
#include "dev_upgrade_public.h"

#define UPGRADE_LOG_PREFIX "upgrade"

#define dev_upgrade_err(fmt, ...)                                                                 \
    do {                                                                                          \
        drv_err(UPGRADE_LOG_PREFIX, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__); \
    } while (0);
#define dev_upgrade_warn(fmt, ...)                                                                 \
    do {                                                                                           \
        drv_warn(UPGRADE_LOG_PREFIX, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__); \
    } while (0);
#define dev_upgrade_info(fmt, ...)                                                                 \
    do {                                                                                           \
        drv_info(UPGRADE_LOG_PREFIX, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__); \
    } while (0);
#define dev_upgrade_debug(fmt, ...)                                                                 \
    do {                                                                                            \
        drv_debug(UPGRADE_LOG_PREFIX, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__); \
    } while (0);
#define dev_upgrade_event(fmt, ...)                                                                     \
    do {                                                                                                \
        drv_event(UPGRADE_LOG_PREFIX, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__); \
    } while (0);

#ifndef DRV_CHECK_STR
#define DRV_CHECK_STR(a)                                                    \
    {                                                                       \
        dev_upgrade_warn("[%s %d] Drv_check:%s\n", __func__, __LINE__, #a); \
    }
#endif

#ifndef DRV_CHECK_CHK
#define DRV_CHECK_CHK(a)     \
    {                        \
        if (!(a)) {          \
            DRV_CHECK_STR(a) \
        }                    \
    }
#endif

#ifndef DRV_CHECK_RET
#define DRV_CHECK_RET(a)     \
    {                        \
        if (!(a)) {          \
            DRV_CHECK_STR(a) \
            return;          \
        }                    \
    }
#endif

#ifndef DRV_CHECK_RETV
#define DRV_CHECK_RETV(a, v) \
    {                        \
        if (!(a)) {          \
            DRV_CHECK_STR(a) \
            return (v);      \
        }                    \
    }
#endif

#endif /* _DEV_UPGRADE_DEF_H_ */
