#ifndef __VDP_COMM_DEBUG_H__
#define __VDP_COMM_DEBUG_H__
#include <linux/printk.h>

/* lowercase helpers used by vdp internal code */
#define vdp_alert(fmt, ...) pr_alert(fmt, ##__VA_ARGS__)
#define vdp_err(fmt, ...) pr_err(fmt, ##__VA_ARGS__)
#define vdp_warn(fmt, ...) pr_warn(fmt, ##__VA_ARGS__)
#define vdp_info(fmt, ...) pr_info(fmt, ##__VA_ARGS__)

/*
 * VDP_*_TRACE — media trace functions originally exported by the closed-source
 * mediabase sys module.  Confirmed via IDA Pro that drv_hdmi.ko and
 * ascend_vdp_hifb.ko reference them only through logging macros; the actual
 * code paths are optimised away for ascend310Brc.  Map to printk so the
 * symbols resolve at compile time.
 */
#define VDP_EMERG_TRACE(fmt, ...) pr_emerg(fmt, ##__VA_ARGS__)
#define VDP_ERR_TRACE(fmt, ...)   pr_err(fmt, ##__VA_ARGS__)
#define VDP_WARN_TRACE(fmt, ...)  pr_warn(fmt, ##__VA_ARGS__)
#define VDP_INFO_TRACE(fmt, ...)  pr_info(fmt, ##__VA_ARGS__)
#define VDP_DEBUG_TRACE(fmt, ...) pr_debug(fmt, ##__VA_ARGS__)

#endif
