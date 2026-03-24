#ifndef __VDP_COMM_DEBUG_H__
#define __VDP_COMM_DEBUG_H__
#include <linux/printk.h>
#define vdp_alert(fmt, ...) pr_alert(fmt, ##__VA_ARGS__)
#define vdp_err(fmt, ...) pr_err(fmt, ##__VA_ARGS__)
#define vdp_warn(fmt, ...) pr_warn(fmt, ##__VA_ARGS__)
#define vdp_info(fmt, ...) pr_info(fmt, ##__VA_ARGS__)
#endif
