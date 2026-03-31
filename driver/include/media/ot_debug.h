/*
 * ot_debug.h - OT debug definitions (mediabase compatibility layer)
 *
 * This is a thin compatibility wrapper derived from IDA Pro reverse engineering
 * of ascend_vdp_hifb.ko. Debug levels and trace macros map to the underlying
 * HiSilicon debug infrastructure.
 */

#ifndef OT_DEBUG_H
#define OT_DEBUG_H

#include "hi_debug.h"
#include "ot_type.h"
#include "ot_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/* Debug levels */
#define OT_DBG_EMERG    0   /* system is unusable */
#define OT_DBG_ALERT    1   /* action must be taken immediately */
#define OT_DBG_CRIT     2   /* critical conditions */
#define OT_DBG_ERR      3   /* error conditions */
#define OT_DBG_WARN     4   /* warning conditions */
#define OT_DBG_NOTICE   5   /* normal but significant condition */
#define OT_DBG_INFO     6   /* informational */
#define OT_DBG_DEBUG    7   /* debug-level messages */

/* HI_DBG_* legacy aliases (used by drv/adp/ compress code) */
#ifndef HI_DBG_ERR
#define HI_DBG_ERR    OT_DBG_ERR
#define HI_DBG_WARN   OT_DBG_WARN
#define HI_DBG_INFO   OT_DBG_INFO
#define HI_DBG_DEBUG  OT_DBG_DEBUG
#endif

/* OT_TRACE(level, mod, fmt...) - generic trace used by HAL compress code */
#ifndef OT_TRACE
#define OT_TRACE(level, mod, fmt, ...) printk(KERN_DEBUG "[OT][%s] " fmt, __FUNCTION__, ##__VA_ARGS__)
#endif

/*
 * OT_DEBUG_TRACE - kernel-mode trace macro
 *
 * Wraps hi_chk_log_level + printk for conditional logging.
 * Usage: OT_DEBUG_TRACE(OT_ID_FB, "msg %d\n", val);
 */
#ifdef __KERNEL__
#define OT_DEBUG_TRACE(mod, fmt, ...)                                       \
    do {                                                                    \
        if (hi_chk_log_level(OT_DBG_ERR, (mod)) == 1) {                    \
            (void)printk(KERN_ERR "[OT][%s] " fmt,                         \
                __FUNCTION__, ##__VA_ARGS__);                               \
        }                                                                   \
    } while (0)

#define OT_ERR_TRACE(mod, fmt, ...)                                         \
    do {                                                                    \
        if (hi_chk_log_level(OT_DBG_ERR, (mod)) == 1) {                    \
            (void)printk(KERN_ERR "[OT][%s] " fmt,                         \
                __FUNCTION__, ##__VA_ARGS__);                               \
        }                                                                   \
    } while (0)
#else
#define OT_DEBUG_TRACE(mod, fmt, ...)   ((void)0)
#define OT_ERR_TRACE(mod, fmt, ...)     ((void)0)
#endif

/* Complete set of per-level trace macros used by audio/vdp modules.
 * Maps to printk at the appropriate kernel log level. */
#ifndef OT_EMERG_TRACE
#define OT_EMERG_TRACE(mod, fmt, ...)   printk(KERN_EMERG  "[OT][%s] " fmt, __FUNCTION__, ##__VA_ARGS__)
#endif
#ifndef OT_ALERT_TRACE
#define OT_ALERT_TRACE(mod, fmt, ...)   printk(KERN_ALERT  "[OT][%s] " fmt, __FUNCTION__, ##__VA_ARGS__)
#endif
#ifndef OT_CRIT_TRACE
#define OT_CRIT_TRACE(mod, fmt, ...)    printk(KERN_CRIT   "[OT][%s] " fmt, __FUNCTION__, ##__VA_ARGS__)
#endif
#ifndef OT_WARN_TRACE
#define OT_WARN_TRACE(mod, fmt, ...)    printk(KERN_WARNING "[OT][%s] " fmt, __FUNCTION__, ##__VA_ARGS__)
#endif
#ifndef OT_NOTICE_TRACE
#define OT_NOTICE_TRACE(mod, fmt, ...)  printk(KERN_NOTICE  "[OT][%s] " fmt, __FUNCTION__, ##__VA_ARGS__)
#endif
#ifndef OT_INFO_TRACE
#define OT_INFO_TRACE(mod, fmt, ...)    printk(KERN_INFO    "[OT][%s] " fmt, __FUNCTION__, ##__VA_ARGS__)
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* OT_DEBUG_H */
