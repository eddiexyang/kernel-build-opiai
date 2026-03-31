#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#ifdef AOS_LLVM_BUILD
#include <linux/export.h>
#include <linux/err.h>
#include <linux/sched.h>
#endif // #ifdef AOS_LLVM_BUILD
#include "hi_type.h"
#include "securec.h"
#include "hi_osal.h"
osal_task_t *osal_kthread_create(threadfn_t thread, void *data, const hi_char *name)
{
    struct task_struct *k = NULL;
    osal_task_t *p = (osal_task_t *)osal_kmalloc(sizeof(osal_task_t), osal_gfp_kernel);
    if (p == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "kmalloc error!\n");
        return NULL;
    }
    (void)memset_s(p, sizeof(osal_task_t), 0, sizeof(osal_task_t));

    k = kthread_run(thread, data, name);
    if (IS_ERR(k)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "kthread create error!\n");
        osal_kfree(p);
        return NULL;
    }
    p->task_struct = k;
    return p;
}
EXPORT_SYMBOL(osal_kthread_create);

void osal_kthread_destroy(osal_task_t *task, hi_u32 stop_flag)
{
    if (task == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }
    /* note: When you call the Kthread_stop function, the thread function cannot be finished, otherwise it will oops. */
    if (stop_flag != 0) {
        (void)kthread_stop((struct task_struct *)(task->task_struct));
    }
    task->task_struct = NULL;
    osal_kfree(task);
}
EXPORT_SYMBOL(osal_kthread_destroy);

void osal_kthread_destory(osal_task_t *task, hi_u32 stop_flag)
__attribute__((alias("osal_kthread_destroy")));
EXPORT_SYMBOL(osal_kthread_destory);


osal_task_t *osal_kthread_create_and_bind(threadfn_t thread, void *data, const hi_char *name, hi_u32 cpu)
{
    struct task_struct *k = NULL;
    hi_s32 ret;
    osal_task_t *p = (osal_task_t *)osal_kmalloc(sizeof(osal_task_t), osal_gfp_kernel);
    if (p == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "kmalloc error!\n");
        return NULL;
    }
    ret = memset_s(p, sizeof(osal_task_t), 0, sizeof(osal_task_t));
    if (ret != EOK) {
        HI_TRACE_OSAL(HI_DBG_ERR, "memset ktread point fail!\n");
        osal_kfree(p);
        p = NULL;
        return NULL;
    }

    k = kthread_create(thread, data, name);
    if (IS_ERR(k)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "kthread create error!\n");
        osal_kfree(p);
        p = NULL;
        return NULL;
    }
    p->task_struct = k;
    kthread_bind(k, cpu);
    (void)wake_up_process(k);

    return p;
}
EXPORT_SYMBOL(osal_kthread_create_and_bind);

osal_task_t *osal_kthread_create_and_bind_cpu_bit_map(
    threadfn_t thread, void *data, const hi_char *name, const void *mask)
{
    osal_task_t *p = NULL;

    p = osal_kthread_create(thread, data, name);
    if (p == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "kthread create error!\n");
        return NULL;
    }
    (void)set_cpus_allowed_ptr(p->task_struct, (const struct cpumask*)mask);

    return p;
}
EXPORT_SYMBOL(osal_kthread_create_and_bind_cpu_bit_map);

void osal_kthread_bind_cpu_bitmap(void *task_struct, const void *mask)
{
    (void)set_cpus_allowed_ptr(task_struct, (const struct cpumask*)mask);
}
EXPORT_SYMBOL(osal_kthread_bind_cpu_bitmap);