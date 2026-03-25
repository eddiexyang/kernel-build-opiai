#ifndef _OPIAI_COMPAT_VIRT_WALL_TIME_H_
#define _OPIAI_COMPAT_VIRT_WALL_TIME_H_

#include <linux/timekeeping.h>
#include <linux/time_namespace.h>

/* Historical out-of-tree users expected "virtual" CLOCK_MONOTONIC in the
 * current time namespace. The current kernel spells this as ktime_get_ts64()
 * followed by timens_add_monotonic(), exactly like posix clock_gettime(). */
static inline void ktime_get_virtual_ts64(struct timespec64 *ts)
{
	ktime_get_ts64(ts);
	timens_add_monotonic(ts);
}

#endif
