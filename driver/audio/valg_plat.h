/*
* Copyright (c) Hisilicon Technologies Co., Ltd. 2011-2019. All rights reserved.
* Description:
* Author: Hisilicon multimedia software group
* Create: 2011/06/28
*/

#ifndef __VALG_PLAT_H__
#define __VALG_PLAT_H__

#include "hi_osal.h"

#include "hi_type.h"
#include "hi_errno.h"
#include "hi_debug.h"
#include "hi_board.h"

#include "hi_common.h"
#include "hi_comm_video.h"
#include "mm_ext.h"
#include "vb_ext.h"
#include "dev_ext.h"
#include "proc_ext.h"

#if defined(__KERNEL__)  || defined(__LITEOS__)
#else
#include <sys/time.h>
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define VALG_TRACE(level, fmt, ...)                                                                \
    do {                                                                                                    \
        HI_TRACE(level, HI_ID_VALG, "[Valg]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } while (0)
#define VALG_ERR_LEVEL_NOTE 0


typedef osal_spinlock_t valg_lock;
typedef unsigned long VALG_LOCK_FLAG;


__inline static hi_s32 valg_create_lock(valg_lock *lock, hi_bool locked)
{
    HI_UNUSED(locked);
    return osal_spin_lock_init(lock);
}

__inline static hi_void valg_lock_irqsave(valg_lock *lock, VALG_LOCK_FLAG *flag)
{
    if ((lock == NULL) || (flag == NULL)) {
        VALG_TRACE(HI_DBG_ERR, " lock:%lu or flag:%lu is NULL\n", (hi_uintptr_t)lock, (hi_uintptr_t)flag);
        return;
    }

    osal_spin_lock_irqsave(lock, flag);
}

__inline static hi_void valg_unlock(valg_lock *lock, VALG_LOCK_FLAG *flag)
{
    if ((lock == NULL) || (flag == NULL)) {
        VALG_TRACE(HI_DBG_ERR, " lock:%lu or flag:%lu is NULL\n", (hi_uintptr_t)lock, (hi_uintptr_t)flag);
        return;
    }

    osal_spin_unlock_irqrestore(lock, flag);
}

__inline static hi_void valg_delete_lock(valg_lock *lock)
{
    osal_spin_lock_destroy(lock);
    return;
}
#define forceinline         __inline__ __attribute__((always_inline))

typedef osal_timer_t valg_timer;
typedef hi_void (*PTR_TIMER_CALLBACK)(unsigned long data);

__inline static hi_s32 valg_timer_init(valg_timer *ph_timer, PTR_TIMER_CALLBACK ptr_timer, hi_ulong ul_data)
{
    hi_s32 ret;

    ret = osal_timer_init(ph_timer);
    if (ret) {
        return ret;
    }

    ph_timer->data = ul_data;
    ph_timer->function = ptr_timer;
    return HI_SUCCESS;
}

__inline static hi_s32 valg_timer_start_once(valg_timer *ph_timer, hi_ulong ul_interval)
{
    return osal_set_timer(ph_timer, ul_interval);
}

__inline static hi_void valg_timer_start_once_on_cpu(valg_timer *ph_timer, hi_ulong interval)
{
    osal_set_timer_on_cpu(ph_timer, interval);
}

__inline static hi_void valg_timer_delete(valg_timer *ph_timer)
{
    osal_del_timer(ph_timer);
    osal_timer_destroy(ph_timer);

    return;
}

#ifndef DVPP_UTST
__inline static hi_s32 valg_thread_init(osal_task_t **thread, hi_void *task_function, hi_void *data,
    char *task_name)
{
    *thread = osal_kthread_create(task_function, data, task_name);
    if (*thread == NULL) {
        VALG_TRACE(HI_DBG_ERR, "create thread fail!!!\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

__inline static hi_s32 valg_thread_init_and_bind(osal_task_t **thread, hi_void *task_function, hi_void *data,
    char *task_name, hi_u32 cpu)
{
    *thread = osal_kthread_create_and_bind(task_function, data, task_name, cpu);
    if (*thread == NULL) {
        VALG_TRACE(HI_DBG_ERR, "create thread fail!!!\n");
        return HI_FAILURE;
    }

    VALG_TRACE(HI_DBG_INFO, "bind thread success, cpu_id = %u!\n", cpu);
    return HI_SUCCESS;
}

__inline static hi_s32 valg_thread_init_and_bind_cpu_bit_map(osal_task_t **thread, threadfn_t task_function,
    hi_void *data, const char *task_name, const void *mask)
{
    *thread = osal_kthread_create_and_bind_cpu_bit_map(task_function, data, task_name, mask);
    if (*thread == NULL) {
        VALG_TRACE(HI_DBG_ERR, "create thread fail!!!\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}
#endif

__inline static hi_void valg_thread_bind_cpu_bitmap(void *task_struct, const void *mask)
{
    osal_kthread_bind_cpu_bitmap(task_struct, mask);
}

__inline static hi_void valg_thread_delete(osal_task_t *task, hi_bool stop_flag)
{
    osal_kthread_destroy(task, stop_flag);
}

#if 1
typedef hi_s32 VALG_POOL_HANDLE;
typedef vb_blk_handle VALG_BLOCK_HANDLE;
struct media_mem_file;

static __inline hi_s32 valg_vb_init(const hi_char *cp_mmz_name, VALG_POOL_HANDLE *ph_pool, hi_u32 blk_num,
                                    hi_u64 blk_size, const hi_char *buf_name, hi_vb_remap_mode mode,
                                    struct media_mem_file *mfile, hi_u32 device_id)
{
    if (ckfn_vb_create_pool() == HI_FALSE) {
        VALG_TRACE(HI_DBG_ERR, "call_vb_create_pool function is NULL!\n");
        return HI_FAILURE;
    }
    return call_vb_create_pool((hi_u32 *)ph_pool, blk_num, blk_size, cp_mmz_name, buf_name, mode, mfile, device_id);
}
__inline static hi_s32 valg_vb_free(VALG_POOL_HANDLE h_pool)
{
    if (ckfn_vb_destroy_pool() == HI_FALSE) {
        VALG_TRACE(HI_DBG_ERR, "call_vb_destroy_pool function is NULL!\n");
        return HI_FAILURE;
    }

    return call_vb_destroy_pool((hi_u32)h_pool);
}
__inline static VALG_BLOCK_HANDLE valg_vb_get_blk(VALG_POOL_HANDLE h_pool, hi_u32 uid)
{
    if (ckfn_vb_get_blk_by_pool_id() == HI_FALSE) {
        VALG_TRACE(HI_DBG_ERR, "call_vb_get_blk_by_pool_id function is NULL!\n");
        return HI_FAILURE;
    }

    return call_vb_get_blk_by_pool_id((hi_u32)h_pool, uid);
}
__inline static hi_s32 valg_vb_user_add(VALG_BLOCK_HANDLE h_block, hi_u32 uid)
{
    if (ckfn_vb_user_add() == HI_FALSE) {
        VALG_TRACE(HI_DBG_ERR, "call_vb_user_add function is NULL!\n");
        return HI_FAILURE;
    }
    if (ckfn_vb_handle_to_pool_id() == HI_FALSE) {
        VALG_TRACE(HI_DBG_ERR, "call_vb_handle_to_pool_id function is NULL!\n");
        return HI_FAILURE;
    }
    if (ckfn_vb_handle_to_phys() == HI_FALSE) {
        VALG_TRACE(HI_DBG_ERR, "call_vb_handle_to_phys function is NULL!\n");
        return HI_FAILURE;
    }

    return call_vb_user_add(call_vb_handle_to_pool_id(h_block), call_vb_handle_to_phys(h_block), uid);
}
__inline static hi_s32 valg_vb_user_sub(VALG_BLOCK_HANDLE h_block, hi_u32 uid)
{
    if (ckfn_vb_user_sub() == HI_FALSE) {
        VALG_TRACE(HI_DBG_ERR, "call_vb_user_sub function is NULL!\n");
        return HI_FAILURE;
    }
    if (ckfn_vb_handle_to_pool_id() == HI_FALSE) {
        VALG_TRACE(HI_DBG_ERR, "call_vb_handle_to_pool_id function is NULL!\n");
        return HI_FAILURE;
    }
    if (ckfn_vb_handle_to_phys() == HI_FALSE) {
        VALG_TRACE(HI_DBG_ERR, "call_vb_handle_to_phys function is NULL!\n");
        return HI_FAILURE;
    }

    return call_vb_user_sub(call_vb_handle_to_pool_id(h_block), call_vb_handle_to_phys(h_block), uid);
}
__inline static hi_s32 valg_vb_user_add_by_phy(hi_u32 pool_id, hi_u64 phy_addr, hi_u32 uid)
{
    if (ckfn_vb_user_add() == HI_FALSE) {
        VALG_TRACE(HI_DBG_ERR, "call_vb_user_add function is NULL!\n");
        return HI_FAILURE;
    }

    return call_vb_user_add(pool_id, phy_addr, uid);
}

__inline static hi_s32 valg_vb_user_sub_by_phy(hi_u32 pool_id, hi_u64 phy_addr, hi_u32 uid)
{
    if (ckfn_vb_user_sub() == HI_FALSE) {
        VALG_TRACE(HI_DBG_ERR, "ckfn_vb_user_sub function is NULL!\n");
        return HI_FAILURE;
    }

    return call_vb_user_sub(pool_id, phy_addr, uid);
}

__inline static hi_s32 valg_vb_get_user_cnt(VALG_BLOCK_HANDLE h_block)
{
    if (ckfn_vb_inquire_user_cnt() == HI_FALSE) {
        VALG_TRACE(HI_DBG_ERR, "call_vb_inquire_user_cnt function is NULL!\n");
        return HI_FAILURE;
    }

    return call_vb_inquire_user_cnt(h_block);
}
__inline static hi_u64 valg_vb_handle_to_phys(VALG_BLOCK_HANDLE h_block)
{
    if (ckfn_vb_handle_to_phys() == HI_FALSE) {
        VALG_TRACE(HI_DBG_ERR, "call_vb_handle_to_phys function is NULL!\n");
        return (hi_u64)HI_FAILURE;
    }

    return call_vb_handle_to_phys(h_block);
}
__inline static hi_void *valg_vb_handle_to_kern(VALG_BLOCK_HANDLE h_block)
{
    if (ckfn_vb_handle_to_kern() == HI_FALSE) {
        VALG_TRACE(HI_DBG_ERR, "call_vb_handle_to_kern function is NULL!\n");
        return HI_NULL;
    }

    return ((hi_void *)(uintptr_t)call_vb_handle_to_kern(h_block));
}
__inline static hi_u32 valg_vb_handle_to_pool_id(VALG_BLOCK_HANDLE h_block)
{
    if (ckfn_vb_handle_to_pool_id() == HI_FALSE) {
        VALG_TRACE(HI_DBG_ERR, "call_vb_handle_to_pool_id function is NULL!\n");
        return (hi_u32)HI_FAILURE;
    }

    return call_vb_handle_to_pool_id(h_block);
}

#endif

typedef osal_irq_handler_t PTR_INT_CALLBACK;

__inline static hi_s32 valg_irq_init(hi_s32 irq, PTR_INT_CALLBACK ptr_call_back, hi_void *data)
{
    return osal_request_irq(irq, ptr_call_back, NULL, "vedu", data);
}

__inline static hi_s32 valg_irq_delete(hi_s32 irq, void *dev_id)
{
    osal_free_irq(irq, dev_id);
    return HI_SUCCESS;
}

__inline static hi_u64 get_sys_time_by_sec(void)
{
#if defined(__KERNEL__)  || defined(__LITEOS__)
    osal_timeval_t time;
    osal_gettimeofday(&time);
#else
    struct timeval time;
    gettimeofday(&time, NULL);
#endif
    return (hi_u64)time.tv_sec;
}

__inline static hi_u64 get_sys_time_by_usec(void)
{
#if defined(__KERNEL__)  || defined(__LITEOS__)
    osal_timeval_t time;
    osal_gettimeofday(&time);
#else
    struct timeval time;
    gettimeofday(&time, NULL);
#endif
    return (hi_u64)((time.tv_sec * 1000000LLU) + time.tv_usec);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
