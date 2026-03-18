/*
* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
*/

#include "hi_osal.h"
#include "Os.h"

/* vos 中的task中的处理函数无法传参，此处借用参数data 传入TaskType */
/* vos中没有线程的概念，处理函数是以静态的task实现的，因此thread、name 入参都未使用 */
osal_task_t *osal_kthread_create(threadfn_t thread, void *data, const hi_char *name)
{
    struct task_struct *k = NULL;
    TaskType task_type = *((TaskType*)data);
    StatusType ret = ActivateTask(task_type);
    if (ret != E_OK) {
        HI_TRACE_OSAL(HI_DBG_ERR, "ActivateTask error, error code %#x!\n", ret);
        return NULL;
    }
    osal_task_t *p = (osal_task_t *)osal_kmalloc(sizeof(osal_task_t), osal_gfp_kernel);
    if (p == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "kmalloc error: allocate osal_task failed when creating task\n");
        return NULL;
    }

    p->task_struct = (void *)osal_kmalloc(sizeof(TaskType), osal_gfp_kernel);
    if (p->task_struct == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "kmalloc error: allocate osal_task failed when creating task\n");
        osal_kfree(p);
        return NULL;
    }

    *((TaskType *)p->task_struct) = task_type;

    return p;
}

/* vos 中的task 任务无法从外部终止，因此停止线程仅仅释放task 内存，task停止由各个任务自己停止 */
void osal_kthread_destroy(osal_task_t *task, hi_u32 stop_flag)
{
    if (task == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }

    if (task->task_struct == NULL) {
        osal_kfree(task);
        HI_TRACE_OSAL(HI_DBG_ERR, "task struct is null!\n");
        return;
    }

    osal_kfree(task->task_struct);
    task->task_struct = NULL;
    osal_kfree(task);
}

/* vos 中实现为空 */
osal_task_t *osal_kthread_create_and_bind(threadfn_t thread, void *data, const hi_char *name, hi_u32 cpu)
{
    return NULL;
}

/* vos 中实现为空 */
osal_task_t *osal_kthread_create_and_bind_cpu_bit_map(
    threadfn_t thread, void *data, const hi_char *name, const void *mask)
{
    return NULL;
}

/* vos 中实现为空 */
void osal_kthread_bind_cpu_bitmap(void *task_struct, const void *mask)
{
}
