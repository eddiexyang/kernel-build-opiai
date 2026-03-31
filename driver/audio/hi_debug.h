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

#ifndef HI_DEBUG_H__
#define HI_DEBUG_H__

#if !defined(__KERNEL__) && !defined(__LITEOS__)
#define __USER__
#endif

#include "hi_type.h"
#include "hi_common.h"

#ifdef __KERNEL__
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/sched.h>
#include "hi_common_adapt.h"
#elif defined(__LITEOS__)
#include "los_printf.h"
#else
#include <stdio.h>
#include <assert.h>
#include "slog.h"
#endif // __KERNEL__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define HI_DVPP_DBG_EMERG      0   /* system is unusable                   */
#define HI_DVPP_DBG_ALERT      1   /* action must be taken immediately     */
#define HI_DVPP_DBG_CRIT       2   /* critical conditions                  */
#define HI_DVPP_DBG_ERR        3   /* error conditions                     */
#define HI_DVPP_DBG_WARN       4   /* warning conditions                   */
#define HI_DVPP_DBG_NOTICE     5   /* normal but significant condition     */
#define HI_DVPP_DBG_INFO       6   /* informational                        */
#define HI_DVPP_DBG_DEBUG      7   /* debug-level messages                 */

#define CONFIG_HI_LOG_TRACE_LEVEL 8
#define CONFIG_HI_LOG_TRACE_SUPPORT

#define VERSION_BASE (618)

#define VERSION_CRL (0)

#define VERSION_CRQ (0)

#define VERSION_DW  (0)

#define VERSION_HW  (0)

#define VERSION_HWX (0)

#define VERSION_JLR (1)

#define VERSION_JJX (0)

#define VERSION_LX  (1)

#define VERSION_LJC (0)

#define VERSION_LJZ (0)

#define VERSION_LXC (0)

#define VERSION_LYH (0)

#define VERSION_LYT (0)

#define VERSION_LJH (0)

#define VERSION_MHQ (0)

#define VERSION_QXT (0)

#define VERSION_SL  (0)

#define VERSION_SQ  (0)

#define VERSION_TH  (0)

#define VERSION_WJ  (0)

#define VERSION_WCG (0)

#define VERSION_WW  (1)

#define VERSION_WYW (1)

#define VERSION_WL  (0)

#define VERSION_WLF (9)

#define VERSION_XCY (0)

#define VERSION_XH  (0)

#define VERSION_XSQ (0)

#define VERSION_ZHJ (0)

#define VERSION_ZYF (0)

#define VERSION_ZYX (0)

#define VERSION_ZZY (0)

#define VERSION_INFO (VERSION_BASE + \
                      VERSION_CRL + VERSION_CRQ + VERSION_DW  + VERSION_HW  + VERSION_HWX + VERSION_JLR + \
                      VERSION_JJX + VERSION_LX  + VERSION_LJC + VERSION_LJZ + VERSION_LXC + VERSION_LYH + \
                      VERSION_LYT + VERSION_LJH + VERSION_MHQ + VERSION_QXT + VERSION_SL  + VERSION_SQ  + \
                      VERSION_TH  + VERSION_WJ  + VERSION_WCG + VERSION_WW  + VERSION_WYW + VERSION_WL  + \
                      VERSION_WLF + VERSION_XCY + VERSION_XH  + VERSION_XSQ + VERSION_ZHJ + VERSION_ZYF + \
                      VERSION_ZYX + VERSION_ZZY)

#define VDEC_CTRL_LOG 0
#define VDEC_DECODE_CTRL_LOG 1

#ifdef __USER__
#define HI_PRINT      (hi_void)printf

/* #ifdef HI_DEBUG */
#ifdef CONFIG_HI_LOG_TRACE_SUPPORT

#ifdef BUILD_DEBUG
    #define HI_ASSERT(expr)                         \
        do {                                        \
            if (!(expr)) {                          \
                (void)printf("\nASSERT at:\n"       \
                    "  >Function : %s\n"            \
                    "  >Line No. : %d\n"            \
                    "  >Condition: %s\n",           \
                    __FUNCTION__, __LINE__, #expr); \
                assert(0);                          \
            }                                       \
        } while (0)
#else
    #define HI_ASSERT(expr)
#endif // #ifdef BUILD_DEBUG

    /* Using samples:
    ** HI_TRACE(HI_DBG_DEBUG, HI_ID_CMPI, "Test %d, %s\n", 12, "Test")
    * */
#if defined DVPP_UTST
    #define HI_TRACE(level, enModId, fmt, ...)  \
        do {                                    \
            printf("[HiDvpp][A%d] " fmt, VERSION_INFO, ##__VA_ARGS__);      \
        } while (0)
    #define HI_PRINTK_CTRL_LOG(enModId, idx, fmt, ...)                    \
        do { \
            printf("[HiDvpp][A%d] " fmt, VERSION_INFO, ##__VA_ARGS__);      \
        } while (0)
    #define HI_FLUSH_LOG()
#elif defined DVPP_VCAST_UTST // #ifdef DVPP_UTST
    #define HI_TRACE(level, enModId, fmt, ...)  \
        do {                                    \
            hi_s32 software_version = VERSION_INFO;  \
            dlog_debug(DVPP, "[HiDvpp][A%d] " fmt, software_version, ##__VA_ARGS__);      \
        } while (0)
    #define HI_FLUSH_LOG()
#else // #ifdef DVPP_UTST
    /* MDC形态libslog鉴于IAM对于小包读写性能不佳,为了提升日志流量和性能,引入了缓存,并做定时(1秒)定量(1000条)flush机制.
    ** 对于ERR日志,调用DlogFlush接口,确保相关进程在程序退出前,将缓存中剩余的日志Flush到slogd进程
    **/
#ifdef BUILD_MODULE_ISP
    #define HI_DLOG_HI_DBG_NOTICE(enModId, fmt, ...) \
        do { \
            hi_s32 software_version = VERSION_INFO;  \
            dlog_event(ISP, "[HiISP][A%d] " fmt, software_version, ##__VA_ARGS__);      \
            DlogFlush();  \
        } while (0)

    #define HI_DLOG_HI_DBG_ERR(enModId, fmt, ...) \
        do { \
            hi_s32 software_version = VERSION_INFO;  \
            dlog_error(ISP, "[HiISP][A%d] " fmt, software_version, ##__VA_ARGS__);      \
            DlogFlush();  \
        } while (0)

    #define HI_DLOG_HI_DBG_WARN(enModId, fmt, ...) \
        do { \
            hi_s32 software_version = VERSION_INFO;  \
            dlog_warn(ISP, "[HiISP][A%d] " fmt, software_version, ##__VA_ARGS__);      \
        } while (0)

    #define HI_DLOG_HI_DBG_INFO(enModId, fmt, ...) \
        do { \
            hi_s32 software_version = VERSION_INFO;  \
            dlog_info(ISP, "[HiISP][A%d] " fmt, software_version, ##__VA_ARGS__);      \
        } while (0)

    #define HI_DLOG_HI_DBG_DEBUG(enModId, fmt, ...) \
        do { \
            hi_s32 software_version = VERSION_INFO;  \
            dlog_debug(ISP, "[HiISP][A%d] " fmt, software_version, ##__VA_ARGS__);      \
        } while (0)

    #define HI_DLOG_HI_DBG_EVENT(enModId, fmt, ...) \
        do { \
            hi_s32 software_version = VERSION_INFO;  \
            dlog_event(ISP, "[HiISP][A%d] " fmt, software_version, ##__VA_ARGS__);      \
        } while (0)
#elif defined BUILD_MODULE_AUDIO
    #define HI_DLOG_HI_DBG_NOTICE(enModId, fmt, ...) \
        do { \
            hi_s32 software_version = VERSION_INFO;  \
            dlog_warn(AUDIO, "[HiAudio][A%d] " fmt, software_version, ##__VA_ARGS__);      \
            DlogFlush();  \
        } while (0)

    #define HI_DLOG_HI_DBG_ERR(enModId, fmt, ...) \
        do { \
            hi_s32 software_version = VERSION_INFO;  \
            dlog_error(AUDIO, "[HiAudio][A%d] " fmt, software_version, ##__VA_ARGS__);      \
            DlogFlush();  \
        } while (0)

    #define HI_DLOG_HI_DBG_WARN(enModId, fmt, ...) \
        do { \
            hi_s32 software_version = VERSION_INFO;  \
            dlog_warn(AUDIO, "[HiAudio][A%d] " fmt, software_version, ##__VA_ARGS__);      \
        } while (0)

    #define HI_DLOG_HI_DBG_INFO(enModId, fmt, ...) \
        do { \
            hi_s32 software_version = VERSION_INFO;  \
            dlog_info(AUDIO, "[HiAudio][A%d] " fmt, software_version, ##__VA_ARGS__);      \
        } while (0)

    #define HI_DLOG_HI_DBG_DEBUG(enModId, fmt, ...) \
        do { \
            hi_s32 software_version = VERSION_INFO;  \
            dlog_debug(AUDIO, "[HiAudio][A%d] " fmt, software_version, ##__VA_ARGS__);      \
        } while (0)

    #define HI_DLOG_HI_DBG_EVENT(enModId, fmt, ...) \
        do { \
            hi_s32 software_version = VERSION_INFO;  \
            dlog_event(AUDIO, "[HiAudio][A%d] " fmt, software_version, ##__VA_ARGS__);      \
        } while (0)
#else
    #define HI_DLOG_HI_DBG_NOTICE(enModId, fmt, ...) \
        do { \
            hi_s32 software_version = VERSION_INFO;  \
            dlog_warn(DVPP, "[HiDvpp][A%d] " fmt, software_version, ##__VA_ARGS__);      \
            DlogFlush();  \
        } while (0)

    #define HI_DLOG_HI_DBG_ERR(enModId, fmt, ...) \
        do { \
            hi_s32 software_version = VERSION_INFO;  \
            dlog_error(DVPP, "[HiDvpp][A%d] " fmt, software_version, ##__VA_ARGS__);      \
            DlogFlush();  \
        } while (0)

    #define HI_DLOG_HI_DBG_WARN(enModId, fmt, ...) \
        do { \
            hi_s32 software_version = VERSION_INFO;  \
            dlog_warn(DVPP, "[HiDvpp][A%d] " fmt, software_version, ##__VA_ARGS__);      \
        } while (0)

    #define HI_DLOG_HI_DBG_INFO(enModId, fmt, ...) \
        do { \
            hi_s32 software_version = VERSION_INFO;  \
            dlog_info(DVPP, "[HiDvpp][A%d] " fmt, software_version, ##__VA_ARGS__);      \
        } while (0)

    #define HI_DLOG_HI_DBG_DEBUG(enModId, fmt, ...) \
        do { \
            hi_s32 software_version = VERSION_INFO;  \
            dlog_debug(DVPP, "[HiDvpp][A%d] " fmt, software_version, ##__VA_ARGS__);      \
        } while (0)

    #define HI_DLOG_HI_DBG_EVENT(enModId, fmt, ...) \
        do { \
            hi_s32 software_version = VERSION_INFO;  \
            dlog_event(DVPP, "[HiDvpp][A%d] " fmt, software_version, ##__VA_ARGS__);      \
        } while (0)
#endif // end of BUILD_MODULE_ISP

#define HI_TRACE(level, enModId, fmt, ...) HI_DLOG_##level((enModId), fmt, ##__VA_ARGS__)

#define HI_FLUSH_LOG() \
    do { \
        DlogFlush(); \
    } while (0)
#endif // end of CONFIG_HI_LOG_TRACE_SUPPORT

#else
    #define HI_ASSERT(expr)
    #define HI_TRACE(level, enModId, fmt...)
    #define HI_FLUSH_LOG()
#endif // end of DVPP_UTST

#elif defined(__KERNEL__)
#define HI_PRINT      osal_printk
/* hi_chk_log_level implemented in drv_base.ko, not effective for drv_osal.ko */
#ifndef BUILD_DRV_OSAL
extern hi_s32 hi_chk_log_level(hi_s32 level, hi_mod_id mod_id);
extern hi_s32 hi_chk_debug_log_level(hi_u32 mod_id, hi_u32 idx);
#else
#define hi_chk_log_level(l, m)  HI_TRUE
#endif

/* #ifdef HI_DEBUG */
#ifdef CONFIG_HI_LOG_TRACE_SUPPORT
    /* Using samples:   HI_ASSERT(x>y) */
#ifdef BUILD_DEBUG
    #define HI_ASSERT(expr)               \
    do {                                   \
        if (!(expr)) {                    \
            osal_panic("\nASSERT at:\n"   \
                  "  >Function : %s\n"    \
                  "  >Line No. : %d\n"    \
                  "  >Condition: %s\n",   \
                  __FUNCTION__, __LINE__, #expr); \
        } \
    } while (0)
#else
    #define HI_ASSERT(expr)
#endif // #ifdef BUILD_DEBUG

    /* Using samples:
    ** HI_TRACE(HI_DBG_DEBUG, HI_ID_CMPI, "Test %d, %s\n", 12, "Test")
    **/
#ifdef DVPP_VCAST_UTST
    #define HI_TRACE(level, enModId, fmt, ...)  \
    do {                                        \
        (void)printk(KERN_DEBUG "[HiDvpp][A%d] " fmt, VERSION_INFO, ##__VA_ARGS__);         \
    } while (0)
#else // #ifdef DVPP_VCAST_UTST

#ifndef BUILD_DRV_OSAL
#ifdef BUILD_MODULE_ISP
    #define HI_PRINTK_HI_DBG_EMERG(enModId, fmt, ...) \
        do { \
            if (hi_chk_log_level(HI_DVPP_DBG_EMERG, (enModId)) == 1) { \
                (void)printk(KERN_EMERG "[HiISP][A%d] [%d %d]" fmt, \
                    VERSION_INFO, current->tgid, current->pid, ##__VA_ARGS__); \
            } \
        } while (0)

    #define HI_PRINTK_HI_DBG_ALERT(enModId, fmt, ...) \
        do { \
            if (hi_chk_log_level(HI_DVPP_DBG_ALERT, (enModId)) == 1) { \
                (void)printk(KERN_ALERT "[HiISP][A%d] [%d %d]" fmt, \
                    VERSION_INFO, current->tgid, current->pid, ##__VA_ARGS__); \
            } \
        } while (0)

    #define HI_PRINTK_HI_DBG_CRIT(enModId, fmt, ...) \
        do { \
            if (hi_chk_log_level(HI_DVPP_DBG_CRIT, (enModId)) == 1) { \
                (void)printk(KERN_CRIT "[HiISP][A%d] [%d %d]" fmt, \
                    VERSION_INFO, current->tgid, current->pid, ##__VA_ARGS__); \
            } \
        } while (0)

    #define HI_PRINTK_HI_DBG_ERR(enModId, fmt, ...) \
        do { \
            if (hi_chk_log_level(HI_DVPP_DBG_ERR, (enModId)) == 1) { \
                (void)printk(KERN_ERR "[HiISP][A%d] [%d %d]" fmt, \
                    VERSION_INFO, current->tgid, current->pid, ##__VA_ARGS__); \
            } \
        } while (0)

    #define HI_PRINTK_HI_DBG_WARN(enModId, fmt, ...) \
        do { \
            if (hi_chk_log_level(HI_DVPP_DBG_WARN, (enModId)) == 1) { \
                (void)printk(KERN_WARNING "[HiISP][A%d] [%d %d]" fmt, \
                    VERSION_INFO, current->tgid, current->pid, ##__VA_ARGS__); \
            } \
        } while (0)

    #define HI_PRINTK_HI_DBG_NOTICE(enModId, fmt, ...) \
        do { \
            (void)printk(KERN_NOTICE "[HiISP][A%d] [%d %d]" fmt, \
                    VERSION_INFO, current->tgid, current->pid, ##__VA_ARGS__); \
        } while (0)

    #define HI_PRINTK_HI_DBG_INFO(enModId, fmt, ...) \
        do { \
            if (hi_chk_log_level(HI_DVPP_DBG_INFO, (enModId)) == 1) { \
                (void)printk(KERN_INFO "[HiISP][A%d] [%d %d]" fmt, \
                    VERSION_INFO, current->tgid, current->pid, ##__VA_ARGS__); \
            } \
        } while (0)

    #define HI_PRINTK_HI_DBG_DEBUG(enModId, fmt, ...) \
        do { \
            if (hi_chk_log_level(HI_DVPP_DBG_DEBUG, (enModId)) == 1) { \
                (void)printk(KERN_DEBUG "[HiISP][A%d] [%d %d]" fmt, \
                    VERSION_INFO, current->tgid, current->pid, ##__VA_ARGS__); \
            } \
        } while (0)

    #define HI_PRINTK_CTRL_LOG(enModId, idx, fmt, ...)                    \
        do { \
            if (hi_chk_debug_log_level(enModId, idx) == 1) { \
                (void)printk(KERN_CRIT "[HiISP][A%d] [%d %d]" fmt, \
                    VERSION_INFO, current->tgid, current->pid, ##__VA_ARGS__); \
            } \
        } while (0)
#elif defined BUILD_MODULE_AUDIO
    #define HI_PRINTK_HI_DBG_EMERG(enModId, fmt, ...) \
        do { \
            if (hi_chk_log_level(HI_DVPP_DBG_EMERG, (enModId)) == 1) { \
                (void)printk(KERN_EMERG "[HiAudio][A%d] " fmt, VERSION_INFO, ##__VA_ARGS__); \
            } \
        } while (0)

    #define HI_PRINTK_HI_DBG_ALERT(enModId, fmt, ...) \
        do { \
            if (hi_chk_log_level(HI_DVPP_DBG_ALERT, (enModId)) == 1) { \
                (void)printk(KERN_ALERT "[HiAudio][A%d] " fmt, VERSION_INFO, ##__VA_ARGS__); \
            } \
        } while (0)

    #define HI_PRINTK_HI_DBG_CRIT(enModId, fmt, ...) \
        do { \
            if (hi_chk_log_level(HI_DVPP_DBG_CRIT, (enModId)) == 1) { \
                (void)printk(KERN_CRIT "[HiAudio][A%d] " fmt, VERSION_INFO, ##__VA_ARGS__); \
            } \
        } while (0)

    #define HI_PRINTK_HI_DBG_ERR(enModId, fmt, ...) \
        do { \
            if (hi_chk_log_level(HI_DVPP_DBG_ERR, (enModId)) == 1) { \
                (void)printk(KERN_ERR "[HiAudio][A%d] " fmt, VERSION_INFO, ##__VA_ARGS__); \
            } \
        } while (0)

    #define HI_PRINTK_HI_DBG_WARN(enModId, fmt, ...) \
        do { \
            if (hi_chk_log_level(HI_DVPP_DBG_WARN, (enModId)) == 1) { \
                (void)printk(KERN_WARNING "[HiAudio][A%d] " fmt, VERSION_INFO, ##__VA_ARGS__); \
            } \
        } while (0)

    #define HI_PRINTK_HI_DBG_NOTICE(enModId, fmt, ...) \
        do { \
            if (hi_chk_log_level(HI_DVPP_DBG_NOTICE, (enModId)) == 1) { \
                (void)printk(KERN_NOTICE "[HiAudio][A%d] " fmt, VERSION_INFO, ##__VA_ARGS__); \
            } \
        } while (0)

    #define HI_PRINTK_HI_DBG_INFO(enModId, fmt, ...) \
        do { \
            if (hi_chk_log_level(HI_DVPP_DBG_INFO, (enModId)) == 1) { \
                (void)printk(KERN_INFO "[HiAudio][A%d] " fmt, VERSION_INFO, ##__VA_ARGS__); \
            } \
        } while (0)

    #define HI_PRINTK_HI_DBG_DEBUG(enModId, fmt, ...) \
        do { \
            if (hi_chk_log_level(HI_DVPP_DBG_DEBUG, (enModId)) == 1) { \
                (void)printk(KERN_DEBUG "[HiAudio][A%d] " fmt, VERSION_INFO, ##__VA_ARGS__); \
            } \
        } while (0)

    #define HI_PRINTK_CTRL_LOG(enModId, idx, fmt, ...)                    \
        do { \
            if (hi_chk_debug_log_level(enModId, idx) == 1) { \
                (void)printk(KERN_CRIT "[HiAudio][A%d] " fmt, VERSION_INFO, ##__VA_ARGS__); \
            } \
        } while (0)
#else
    #define HI_PRINTK_HI_DBG_EMERG(enModId, fmt, ...) \
        do { \
            if (hi_chk_log_level(HI_DVPP_DBG_EMERG, (enModId)) == 1) { \
                (void)printk(KERN_EMERG "[HiDvpp][A%d] " fmt, VERSION_INFO, ##__VA_ARGS__); \
            } \
        } while (0)

    #define HI_PRINTK_HI_DBG_ALERT(enModId, fmt, ...) \
        do { \
            if (hi_chk_log_level(HI_DVPP_DBG_ALERT, (enModId)) == 1) { \
                (void)printk(KERN_ALERT "[HiDvpp][A%d] " fmt, VERSION_INFO, ##__VA_ARGS__); \
            } \
        } while (0)

    #define HI_PRINTK_HI_DBG_CRIT(enModId, fmt, ...) \
        do { \
            if (hi_chk_log_level(HI_DVPP_DBG_CRIT, (enModId)) == 1) { \
                (void)printk(KERN_CRIT "[HiDvpp][A%d] " fmt, VERSION_INFO, ##__VA_ARGS__); \
            } \
        } while (0)

    #define HI_PRINTK_HI_DBG_ERR(enModId, fmt, ...) \
        do { \
            if (hi_chk_log_level(HI_DVPP_DBG_ERR, (enModId)) == 1) { \
                (void)printk(KERN_ERR "[HiDvpp][A%d] " fmt, VERSION_INFO, ##__VA_ARGS__); \
            } \
        } while (0)

    #define HI_PRINTK_HI_DBG_WARN(enModId, fmt, ...) \
        do { \
            if (hi_chk_log_level(HI_DVPP_DBG_WARN, (enModId)) == 1) { \
                (void)printk(KERN_WARNING "[HiDvpp][A%d] " fmt, VERSION_INFO, ##__VA_ARGS__); \
            } \
        } while (0)

    #define HI_PRINTK_HI_DBG_NOTICE(enModId, fmt, ...) \
        do { \
            if (hi_chk_log_level(HI_DVPP_DBG_NOTICE, (enModId)) == 1) { \
                (void)printk(KERN_NOTICE "[HiDvpp][A%d] " fmt, VERSION_INFO, ##__VA_ARGS__); \
            } \
        } while (0)

    #define HI_PRINTK_HI_DBG_INFO(enModId, fmt, ...) \
        do { \
            if (hi_chk_log_level(HI_DVPP_DBG_INFO, (enModId)) == 1) { \
                (void)printk(KERN_INFO "[HiDvpp][A%d] " fmt, VERSION_INFO, ##__VA_ARGS__); \
            } \
        } while (0)

    #define HI_PRINTK_HI_DBG_DEBUG(enModId, fmt, ...) \
        do { \
            if (hi_chk_log_level(HI_DVPP_DBG_DEBUG, (enModId)) == 1) { \
                (void)printk(KERN_DEBUG "[HiDvpp][A%d] " fmt, VERSION_INFO, ##__VA_ARGS__); \
            } \
        } while (0)

    #define HI_PRINTK_CTRL_LOG(enModId, idx, fmt, ...)                    \
        do { \
            if (hi_chk_debug_log_level(enModId, idx) == 1) { \
                (void)printk(KERN_CRIT "[HiDvpp][A%d] " fmt, VERSION_INFO, ##__VA_ARGS__); \
            } \
        } while (0)
#endif // end of BUILD_MODULE_ISP
#else
    #define HI_PRINTK_HI_DBG_EMERG(enModId, fmt, ...) \
    do { \
        (void)printk(KERN_EMERG "[HiDvpp][A%d] " fmt, VERSION_INFO, ##__VA_ARGS__); \
    } while (0)

    #define HI_PRINTK_HI_DBG_ALERT(enModId, fmt, ...) \
    do { \
        (void)printk(KERN_ALERT "[HiDvpp][A%d] " fmt, VERSION_INFO, ##__VA_ARGS__); \
    } while (0)

    #define HI_PRINTK_HI_DBG_CRIT(enModId, fmt, ...) \
    do { \
        (void)printk(KERN_CRIT "[HiDvpp][A%d] " fmt, VERSION_INFO, ##__VA_ARGS__); \
    } while (0)

    #define HI_PRINTK_HI_DBG_ERR(enModId, fmt, ...) \
    do { \
        (void)printk(KERN_ERR "[HiDvpp][A%d] " fmt, VERSION_INFO, ##__VA_ARGS__); \
    } while (0)

    #define HI_PRINTK_HI_DBG_WARN(enModId, fmt, ...) \
    do { \
        (void)printk(KERN_WARNING "[HiDvpp][A%d] " fmt, VERSION_INFO, ##__VA_ARGS__); \
    } while (0)

    #define HI_PRINTK_HI_DBG_NOTICE(enModId, fmt, ...) \
    do { \
        (void)printk(KERN_NOTICE "[HiDvpp][A%d] " fmt, VERSION_INFO, ##__VA_ARGS__); \
    } while (0)

    #define HI_PRINTK_HI_DBG_INFO(enModId, fmt, ...) \
    do { \
        (void)printk(KERN_INFO "[HiDvpp][A%d] " fmt, VERSION_INFO, ##__VA_ARGS__); \
    } while (0)

    #define HI_PRINTK_HI_DBG_DEBUG(enModId, fmt, ...) \
    do { \
        (void)printk(KERN_DEBUG "[HiDvpp][A%d] " fmt, VERSION_INFO, ##__VA_ARGS__); \
    } while (0)
#endif // #ifndef BUILD_DRV_OSAL

    #define HI_TRACE(level, enModId, fmt, ...)  HI_PRINTK_##level(enModId, fmt, ##__VA_ARGS__)

#endif // #ifdef DVPP_VCAST_UTST
#else
    #define HI_ASSERT(expr)
    #define HI_TRACE(level, enModId, fmt...) osal_printk(fmt)
#endif // end of CONFIG_HI_LOG_TRACE_SUPPORT

#else
// liteos
extern int32_t logger_print(int32_t level, const char *fmt, ...);
extern int32_t logger_check_level(int32_t level, int32_t mod);
extern int32_t logger_check_specific(uint32_t flag);

#define HI_PRINTK_HI_DBG_EMERG(enModId, fmt, ...) \
do { \
    if (logger_check_level(HI_DVPP_DBG_EMERG, (int32_t)(enModId)) == 1) { \
        (void)logger_print(HI_DVPP_DBG_EMERG, "[HiDvpp][A%d] " fmt, VERSION_INFO, ##__VA_ARGS__); \
    } \
} while (0)

#define HI_PRINTK_HI_DBG_ALERT(enModId, fmt, ...) \
do { \
    if (logger_check_level(HI_DVPP_DBG_ALERT, (int32_t)(enModId)) == 1) { \
        (void)logger_print(HI_DVPP_DBG_ALERT,  "[HiDvpp][A%d] " fmt, VERSION_INFO, ##__VA_ARGS__); \
    } \
} while (0)

#define HI_PRINTK_HI_DBG_CRIT(enModId, fmt, ...) \
do { \
    if (logger_check_level(HI_DVPP_DBG_CRIT, (int32_t)(enModId)) == 1) { \
        (void)logger_print(HI_DVPP_DBG_CRIT, "[HiDvpp][A%d] " fmt, VERSION_INFO, ##__VA_ARGS__); \
    } \
} while (0)

#define HI_PRINTK_HI_DBG_ERR(enModId, fmt, ...) \
do { \
    if (logger_check_level(HI_DVPP_DBG_ERR, (int32_t)(enModId)) == 1) { \
        (void)logger_print(HI_DVPP_DBG_ERR, "[HiDvpp][A%d] " fmt, VERSION_INFO, ##__VA_ARGS__); \
    } \
} while (0)

#define HI_PRINTK_HI_DBG_WARN(enModId, fmt, ...) \
do { \
    if (logger_check_level(HI_DVPP_DBG_WARN, (int32_t)(enModId)) == 1) { \
        (void)logger_print(HI_DVPP_DBG_WARN, "[HiDvpp][A%d] " fmt, VERSION_INFO, ##__VA_ARGS__); \
    } \
} while (0)

#define HI_PRINTK_HI_DBG_NOTICE(enModId, fmt, ...) \
do { \
    if (logger_check_level(HI_DVPP_DBG_NOTICE, (int32_t)(enModId)) == 1) { \
        (void)logger_print(HI_DVPP_DBG_NOTICE, "[HiDvpp][A%d] " fmt, VERSION_INFO, ##__VA_ARGS__); \
    } \
} while (0)

#define HI_PRINTK_HI_DBG_INFO(enModId, fmt, ...) \
do { \
    if (logger_check_level(HI_DVPP_DBG_INFO, (int32_t)(enModId)) == 1) { \
        (void)logger_print(HI_DVPP_DBG_INFO, "[HiDvpp][A%d] " fmt, VERSION_INFO, ##__VA_ARGS__); \
    } \
} while (0)

#define HI_PRINTK_HI_DBG_DEBUG(enModId, fmt, ...) \
do { \
    if (logger_check_level(HI_DVPP_DBG_DEBUG, (int32_t)(enModId)) == 1) { \
        (void)logger_print(HI_DVPP_DBG_DEBUG, "[HiDvpp][A%d] " fmt, VERSION_INFO, ##__VA_ARGS__); \
    } \
} while (0)

#define HI_PRINTK_CTRL_LOG(enModId, idx, fmt, ...) \
    do { \
        if (logger_check_specific(1U << ((int32_t)(enModId) + idx)) == 1) { \
            logger_print(HI_DVPP_DBG_CRIT, "[HiDvpp][A%d] " fmt, VERSION_INFO, ##__VA_ARGS__); \
        } \
    } while (0)

#define HI_TRACE(level, enModId, fmt, ...)  HI_PRINTK_##level(enModId, fmt, ##__VA_ARGS__)

#define HI_ASSERT(expr)


#endif // end of __USER__

#if (CONFIG_HI_LOG_TRACE_LEVEL >= HI_DVPP_DBG_EMERG)

#define HI_EMERG_TRACE(mod, fmt...) \
do { \
    HI_TRACE(HI_DBG_EMERG, mod, fmt); \
} while (0)

#else
#define HI_EMERG_TRACE(mod, fmt...)
#endif

#if (CONFIG_HI_LOG_TRACE_LEVEL >= HI_DVPP_DBG_ALERT)

#define HI_ALERT_TRACE(mod, fmt...) \
do { \
    HI_TRACE(HI_DBG_ALERT, mod, fmt); \
} while (0)

#else
#define HI_ALERT_TRACE(mod, fmt...)
#endif

#if (CONFIG_HI_LOG_TRACE_LEVEL >= HI_DVPP_DBG_CRIT)

#define HI_CRIT_TRACE(mod, fmt...) \
do { \
    HI_TRACE(HI_DBG_CRIT, mod, fmt); \
} while (0)

#else
#define HI_CRIT_TRACE(mod, fmt...)
#endif

#if (CONFIG_HI_LOG_TRACE_LEVEL >= HI_DVPP_DBG_ERR)

#define HI_ERR_TRACE(mod, fmt...) \
do { \
    HI_TRACE(HI_DBG_ERR, mod, fmt); \
} while (0)

#else
#define HI_ERR_TRACE(mod, fmt...)
#endif

#if (CONFIG_HI_LOG_TRACE_LEVEL >= HI_DVPP_DBG_WARN)

#define HI_WARN_TRACE(mod, fmt...) \
do { \
    HI_TRACE(HI_DBG_WARN, mod, fmt); \
} while (0)

#else
#define HI_WARN_TRACE(mod, fmt...)
#endif

#if (CONFIG_HI_LOG_TRACE_LEVEL >= HI_DVPP_DBG_NOTICE)

#define HI_NOTICE_TRACE(mod, fmt...) \
do { \
    HI_TRACE(HI_DBG_NOTICE, mod, fmt); \
} while (0) \

#else
#define HI_NOTICE_TRACE(mod, fmt...)
#endif

#if (CONFIG_HI_LOG_TRACE_LEVEL >= HI_DVPP_DBG_INFO)

#define HI_INFO_TRACE(mod, fmt...) \
do { \
    HI_TRACE(HI_DBG_INFO, mod, fmt); \
} while (0)

#else
#define HI_INFO_TRACE(mod, fmt...)
#endif

#if (CONFIG_HI_LOG_TRACE_LEVEL >= HI_DVPP_DBG_DEBUG)

#define HI_DEBUG_TRACE(mod, fmt...) \
do { \
    HI_TRACE(HI_DBG_DEBUG, mod, fmt); \
} while (0)

#else
#define HI_DEBUG_TRACE(mod, fmt...)
#endif


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif // __cplusplus

#endif // HI_DEBUG_H__
