/*
 * vdp_comm_pid_check.h - VDP common PID check stub
 *
 * This header is from the closed-source mediabase framework.
 * Confirmed via IDA Pro that drv_hdmi_intf.c includes it but
 * does not reference any symbols from it - all PID check logic
 * is implemented within drv_hdmi_intf.c itself (hdmi_check_is_same_pid).
 */
#ifndef VDP_COMM_PID_CHECK_H
#define VDP_COMM_PID_CHECK_H

#include "ot_type.h"
#include "hi_osal.h"

/*
 * pid_spin_lock_s - PID protection spinlock type
 * Confirmed via IDA: g_pid_lock is used with osal_spin_lock_init/irqsave/irqrestore
 * => it is an osal_spinlock_t
 */
typedef osal_spinlock_t pid_spin_lock_s;

/*
 * Inline wrappers mapping pid_spin_lock_* to osal_spin_lock_*.
 * The original symbols came from the closed-source mediabase layer;
 * confirmed via IDA that these are simple spinlock operations.
 */
static inline int pid_spin_lock_init(pid_spin_lock_s *lock)
{
	return osal_spin_lock_init(lock);
}

static inline void pid_spin_lock_destory(pid_spin_lock_s *lock)
{
	osal_spin_lock_destroy(lock);
}

/*
 * Call sites use spin_lock_irqsave(lock, flags) style (flags by value),
 * so these must be macros that take the address of flags.
 */
#define pid_spin_lock_irqsave(lock, flags) \
	osal_spin_lock_irqsave((lock), &(flags))

#define pid_spin_unlock_irqrestore(lock, flags) \
	osal_spin_unlock_irqrestore((lock), &(flags))

#endif /* VDP_COMM_PID_CHECK_H */
