#include "hi_osal.h"
#include "osal_list.h"

/*
 * mutex要求持锁和释放锁必须是同一个task，本实现暂不满足这条限制，后续会替换为VOS提供的mutex原生实现
 */

/*
 * 互斥锁初始化要求：
 *   1、调用本接口前需要初始化mutex->mutex指针，指向的内存需要满足：
 *      a、第一个u64字节位置存储event mask
 *      b、第二个u64字节位置存储resource id
 */
hi_s32 osal_mutex_init(osal_mutex_t *mutex)
{
    hi_s32 ret;
    osal_semaphore_t *sem = NULL;

    if ((mutex == NULL) || (mutex->mutex == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }
    sem = osal_kmalloc(sizeof(osal_semaphore_t), osal_gfp_kernel);
    if (sem == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "kmalloc error!\n");
        return -1;
    }
    sem->sem = mutex->mutex;
    ret = osal_sema_init(sem, 1);
    if (ret != 0) {
        HI_TRACE_OSAL(HI_DBG_ERR, "sem init fail!\n");
        osal_kfree(sem);
        return -1;
    }
    mutex->mutex = sem;
    return 0;
}

hi_s32 osal_mutex_lock(const osal_mutex_t *mutex)
{
    osal_semaphore_t *sem = NULL;
    hi_s32 ret;

    if ((mutex == NULL) || (mutex->mutex == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }
    sem = (osal_semaphore_t *)(mutex->mutex);

    ret = osal_down(sem);
    if (ret != 0) {
        HI_TRACE_OSAL(HI_DBG_ERR, "sem down fail!\n");
        return -1;
    }
    return 0;
}

hi_s32 osal_mutex_lock_interruptible(const osal_mutex_t *mutex)
{
    return osal_mutex_lock(mutex);
}

// 无调用者，暂不实现
hi_s32 osal_mutex_trylock(const osal_mutex_t *mutex)
{
    return -1;
}

void osal_mutex_unlock(const osal_mutex_t *mutex)
{
    osal_semaphore_t *sem = NULL;

    if ((mutex == NULL) || (mutex->mutex == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }
    sem = (osal_semaphore_t *)(mutex->mutex);
    osal_up(sem);
}

void osal_mutex_destroy(osal_mutex_t *mutex)
{
    osal_semaphore_t *sem = NULL;

    if ((mutex == NULL) || (mutex->mutex == NULL)) {
        return;
    }
    sem = (osal_semaphore_t *)(mutex->mutex);

    if (sem != NULL) {
        osal_sema_destroy(sem);
        osal_kfree(sem);
        mutex->mutex = NULL;
    }
}

void osal_mutex_destory(osal_mutex_t *mutex)
{
    osal_mutex_destroy(mutex);
}
