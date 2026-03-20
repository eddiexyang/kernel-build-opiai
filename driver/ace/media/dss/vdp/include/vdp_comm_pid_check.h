#ifndef VDP_COMM_PID_CHECK_H
#define VDP_COMM_PID_CHECK_H

#include "ot_osal.h"
#include "ot_type.h"

typedef osal_spinlock_t pid_spin_lock_s;

#define pid_spin_lock_init(lock)               osal_spin_lock_init(lock)
#define pid_spin_lock_destroy(lock)            osal_spin_lock_destroy(lock)
#define pid_spin_lock_destory(lock)            osal_spin_lock_destroy(lock)
#define pid_spin_lock_irqsave(lock, flag)      osal_spin_lock_irqsave(lock, &(flag))
#define pid_spin_unlock_irqrestore(lock, flag) osal_spin_unlock_irqrestore(lock, &(flag))

#endif
