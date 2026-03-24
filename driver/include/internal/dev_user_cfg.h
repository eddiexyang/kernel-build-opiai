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

#ifndef _DEV_USER_CFG_H
#define _DEV_USER_CFG_H

#include "drv_log.h"

/* error code */
#define UC_OK 0
#define UC_ERR_PARA (-EINVAL)
#define UC_ERR_SHA256_CHECK (-ENODATA)
#define UC_ERR_MEM_ALLOC (-ENOMEM)
#define UC_ERR_MEM_CPY (-ENOMEM)
#define UC_ERR_MEM_INIT (-ENOMEM)
#define UC_ERR_MEM_CMP (-EDOM)
#define UC_ERR_NO_AUTHORITY (-EACCES)
#define UC_ERR_ITEM_NOT_SET (-ENOENT)
#define UC_ERR_HEAD_NOT_FOUND UC_ERR_ITEM_NOT_SET
#define UC_ERR_FLASH_INVALID UC_ERR_ITEM_NOT_SET
#define UC_ERR_HEAD_NUM (-ENOSPC)
#define UC_ERR_FLASH_WR_TIMEOUT (-ETIMEDOUT)
#define UC_ERR_FLASH_FULL (-ENOSPC)
#define UC_ERR_GET_ITEM_FAIL (-ENXIO)
#define UC_ERR_FLASH_OP_FAIL (-ENODEV)

/* log definition */
#define USER_CFG_LOG_PREFIX "user_cfg_drv"

#define DEV_USER_CFG_ERR(fmt, ...)                                                                     \
    do {                                                                                              \
        drv_err(USER_CFG_LOG_PREFIX, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__); \
    } while (0);
#define DEV_USER_CFG_WARN(fmt, ...)                                                                     \
    do {                                                                                               \
        drv_warn(USER_CFG_LOG_PREFIX, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__); \
    } while (0);
#define DEV_USER_CFG_INFO(fmt, ...)                                                                     \
    do {                                                                                               \
        drv_info(USER_CFG_LOG_PREFIX, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__); \
    } while (0);
#define DEV_USER_CFG_DEBUG(fmt, ...)                                                                     \
    do {                                                                                                \
        drv_debug(USER_CFG_LOG_PREFIX, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__); \
    } while (0);
#define DEV_USER_CFG_EVENT(fmt, ...)                                                                     \
    do {                                                                                                \
        drv_event(USER_CFG_LOG_PREFIX, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__); \
    } while (0);

#ifndef DRV_CHECK_STR
#define DRV_CHECK_STR(a)                                                    \
    {                                                                       \
        DEV_USER_CFG_WARN("[%s %d] Drv_check:%s\n", __func__, __LINE__, #a); \
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

#define DEV_USER_CFG_VFREE(p) \
    do {                     \
        vfree(p);            \
        p = NULL;            \
    } while (0)

#define DEV_USER_CFG_KFREE(p) \
    do {                     \
        kfree(p);            \
        p = NULL;            \
    } while (0)

#ifdef STATIC_SKIP
#define STATIC
#else
#define STATIC static
#endif

#define SC_PAD_INFO_PAGE_SIZE 0x1000
#define SC_PAD_INFO_OFFSET 0x8C
#define BOOT_CFG_MASK 0x20


#endif /* _DEV_USER_CFG_H */

