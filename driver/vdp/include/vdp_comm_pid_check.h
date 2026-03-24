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

#endif /* VDP_COMM_PID_CHECK_H */
