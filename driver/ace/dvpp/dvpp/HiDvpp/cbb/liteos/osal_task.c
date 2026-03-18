/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2016-2020.
 * Description: osal task source file.
 * Author: Hisilicon multimedia software group
 * Create: 2016-11-11
 */

#include "hi_osal.h"
#include "los_task.h"
#include "securec.h"

#define OSAL_STACK_SIZE 0x4000  // 16k
#define TASK_NAME_LEN  32

typedef struct {
    osal_task_t thread;
    hi_char name[TASK_NAME_LEN];
} task_wrapper_t;

_Static_assert(LOSCFG_BASE_CORE_TSK_DEFAULT_PRIO > (osal_priority_normal - osal_priority_min),
    "priority config error");

static UINT32 osal_priority_trans(hi_u32 priority)
{
    return (UINT32)LOSCFG_BASE_CORE_TSK_DEFAULT_PRIO +
        ((priority <= osal_priority_max && priority >= osal_priority_min) ?
        (priority - osal_priority_normal) : 0);
}

osal_task_t *osal_kthread_create_ex(threadfn_t thread, void *data, const hi_char *name, hi_u32 priority)
{
    unsigned int taskid;
    unsigned int ret;
    TSK_INIT_PARAM_S task_param;
    task_wrapper_t *p;
    unsigned int retry_cnt = 0;

    p = (task_wrapper_t *)osal_kmalloc(sizeof(task_wrapper_t), 0);
    if (p == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "kmalloc error!\n");
        return NULL;
    }
    (void)memset_s(p, sizeof(task_wrapper_t), 0, sizeof(task_wrapper_t));

    if (name != NULL &&
        strncpy_s(p->name, TASK_NAME_LEN, name, TASK_NAME_LEN - 1) != 0) {
        HI_TRACE_OSAL(HI_DBG_ERR, "strncpy_s bugs\n");
        osal_kfree(p);
        return NULL;
    }

    (void)memset_s(&task_param, sizeof(TSK_INIT_PARAM_S), 0,
                   sizeof(TSK_INIT_PARAM_S));
    task_param.pfnTaskEntry = (TSK_ENTRY_FUNC)thread;
    task_param.usTaskPrio = osal_priority_trans(priority);
    task_param.auwArgs[0] = (AARCHPTR)data;
    task_param.uwStackSize = OSAL_STACK_SIZE;
    task_param.pcName = p->name;
    task_param.uwResved = LOS_TASK_STATUS_DETACHED;
    // 对于liteos而言，因频繁创建和销毁线程，会导致系统不能及时回收线程资源，
    // 进而将线程资源耗尽，即调用LOS_TaskCreate失败。
    // 因此若出现此种情况，则调用LOS_TaskResRecycle接口强制系统回收资源，
    // 重新调用一次LOS_TaskCreate创建线程
    while (retry_cnt++ < 2U) { // 尝试2次创建
        ret = LOS_TaskCreate(&taskid, &task_param);
        if (ret == LOS_OK) {
            break;
        }
        LOS_TaskResRecycle();
    }

    if (ret != LOS_OK) {
        HI_TRACE_OSAL(HI_DBG_ERR, "kthread create error ret=%0x!\n", ret);
        osal_kfree(p);
        p = NULL;
        return NULL;
    }

    ((osal_task_t *)p)->task_struct = (void *)(uintptr_t)taskid;

    return (osal_task_t *)p;
}

osal_task_t *osal_kthread_create(threadfn_t thread, void *data, const hi_char *name)
{
    return osal_kthread_create_ex(thread, data, name, osal_priority_normal);
}

void osal_kthread_destory(osal_task_t *task, hi_u32 stop_flag)
{
    if (task == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }

    if ((stop_flag != 0) && (task->task_struct != NULL)) {
        (void)LOS_TaskDelete((unsigned int)(uintptr_t)(task->task_struct));
    }

    task->task_struct = NULL;
    osal_kfree(task);
}
