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

#ifndef URD_DEFINE_H
#define URD_DEFINE_H

#include <linux/types.h>
#include <linux/version.h>

#include "drv_log.h"

#ifdef STATIC_SKIP
#  define STATIC
#else
#  define STATIC    static
#endif

#ifndef __GFP_ACCOUNT
#  ifdef __GFP_KMEMCG
#    define __GFP_ACCOUNT __GFP_KMEMCG /* for linux version 3.10 */
#  endif
#  ifdef __GFP_NOACCOUNT
#    define __GFP_ACCOUNT 0 /* for linux version 4.1 */
#  endif
#endif

/*
 * kernel log
 */
#define MODULE_URD "urd_module"

#ifndef dms_err

#ifdef UT_VCAST  /* for ut test */
#  define dms_err(fmt, ...) drv_err(MODULE_URD, fmt, ##__VA_ARGS__)
#  define dms_warn(fmt, ...) drv_warn(MODULE_URD, fmt, ##__VA_ARGS__)
#  define dms_info(fmt, ...) drv_info(MODULE_URD, fmt, ##__VA_ARGS__)
#  define dms_event(fmt, ...) drv_event(MODULE_URD, fmt, ##__VA_ARGS__)
#  define dms_debug(fmt, ...) drv_pr_debug(MODULE_URD, fmt, ##__VA_ARGS__)
#else
#  define dms_err(fmt, ...) drv_err(MODULE_URD, \
      "<%s:%d:%d> " fmt, current->comm, current->tgid, current->pid, ##__VA_ARGS__)
#  define dms_warn(fmt, ...) drv_warn(MODULE_URD, \
      "<%s:%d:%d> " fmt, current->comm, current->tgid, current->pid, ##__VA_ARGS__)
#  define dms_info(fmt, ...) drv_info(MODULE_URD, \
      "<%s:%d:%d> " fmt, current->comm, current->tgid, current->pid, ##__VA_ARGS__)
#  define dms_event(fmt, ...) drv_event(MODULE_URD, \
      "<%s:%d:%d> " fmt, current->comm, current->tgid, current->pid, ##__VA_ARGS__)
#  define dms_debug(fmt, ...) drv_pr_debug(MODULE_URD, \
    "<%s:%d:%d> " fmt, current->comm, current->tgid, current->pid, ##__VA_ARGS__)
#endif
#endif

#ifdef CFG_SOC_PLATFORM_MDC_V51
#  define DMS_MAX_INPUT_LEN  4096
#  define DMS_MAX_OUTPUT_LEN  6144
#else
#  define DMS_MAX_INPUT_LEN  4096
#  define DMS_MAX_OUTPUT_LEN  4096
#endif

#endif  /* __URD_DEFINE_H__ */

