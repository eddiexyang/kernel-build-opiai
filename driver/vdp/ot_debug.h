/* Stub ot_debug.h */
#ifndef __OT_DEBUG_H__
#define __OT_DEBUG_H__
#include "ot_type.h"
#include <linux/printk.h>
#define OT_ASSERT(expr) do {} while(0)
#define ot_emerg_trace(mod, fmt, ...) pr_emerg(fmt, ##__VA_ARGS__)
#define ot_alert_trace(mod, fmt, ...) pr_alert(fmt, ##__VA_ARGS__)
#define ot_crit_trace(mod, fmt, ...) pr_crit(fmt, ##__VA_ARGS__)
#define ot_err_trace(mod, fmt, ...) pr_err(fmt, ##__VA_ARGS__)
#define ot_warn_trace(mod, fmt, ...) pr_warn(fmt, ##__VA_ARGS__)
#define ot_notice_trace(mod, fmt, ...) pr_notice(fmt, ##__VA_ARGS__)
#define ot_info_trace(mod, fmt, ...) pr_info(fmt, ##__VA_ARGS__)
#define ot_debug_trace(mod, fmt, ...) pr_debug(fmt, ##__VA_ARGS__)
#endif
