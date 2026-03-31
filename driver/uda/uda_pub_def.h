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
 * Create: 2023-2-20
 */

#ifndef UDA_PUB_DEF_H
#define UDA_PUB_DEF_H

#include <linux/sched.h>

#ifndef EMU_ST
#include "drv_log.h"
#else
#include "ut_log.h"
#endif

#define module_uda "uda"

#define uda_err(fmt, ...) do { \
    drv_err(module_uda, "<%s:%d:%d:%d> " fmt, \
        current->comm, current->tgid, current->pid, smp_processor_id(), ##__VA_ARGS__); \
} while (0)
#define uda_warn(fmt, ...) do { \
    drv_warn(module_uda, "<%s:%d:%d:%d> " fmt, \
        current->comm, current->tgid, current->pid, smp_processor_id(), ##__VA_ARGS__); \
} while (0)
#define uda_info(fmt, ...) do { \
    drv_info(module_uda, "<%s:%d:%d:%d> " fmt, \
        current->comm, current->tgid, current->pid, smp_processor_id(), ##__VA_ARGS__); \
} while (0)
#define uda_debug(fmt, ...) do { \
    drv_pr_debug(module_uda, "<%s:%d:%d:%d> " fmt, \
        current->comm, current->tgid, current->pid, smp_processor_id(), ##__VA_ARGS__); \
} while (0)

#ifndef __GFP_ACCOUNT
#  ifdef __GFP_KMEMCG
#    define __GFP_ACCOUNT __GFP_KMEMCG /* for linux version 3.10 */
#  endif
#  ifdef __GFP_NOACCOUNT
#    define __GFP_ACCOUNT 0 /* for linux version 4.1 */
#  endif
#endif

#ifdef DRV_HOST
#define UDA_PHY_DEV_NAME "davinci"
#define UDA_MIA_DEV_NAME "vdavinci"
#define UDA_LOCAL_FLAG 0
#define UDA_UDEV_MAX_NUM 1124
#define UDA_MIA_UDEV_OFFSET 100
#else
#define UDA_PHY_DEV_NAME "davinci"
#define UDA_MIA_DEV_NAME "davinci"
#define UDA_LOCAL_FLAG 1
#define UDA_UDEV_MAX_NUM 64
#define UDA_MIA_UDEV_OFFSET 32
#endif
#define UDA_SUB_DEV_MAX_NUM 16
#define UDA_MAX_PHY_DEV_NUM UDA_MIA_UDEV_OFFSET

#endif

