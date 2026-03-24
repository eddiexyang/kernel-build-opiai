#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/module.h>
#ifdef AOS_LLVM_BUILD
#include <linux/export.h>
#endif // #ifdef AOS_LLVM_BUILD
#include "hi_osal.h"

OSAL_LIST_HEAD(wq_list);
struct wq_node {
    struct osal_work_struct *osal_work;
    struct work_struct *work;
    struct osal_list_head node;
};

static struct osal_work_struct *osal_find_work(struct work_struct *work)
{
    struct osal_list_head *this = NULL;
    if (osal_list_empty(&wq_list)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "find work failed! wq_list is empty!\n");
        return NULL;
    }
    osal_list_for_each(this, &wq_list) {
        struct wq_node *ws = osal_list_entry(this, struct wq_node, node);
        if (ws->work == work) {
            return ws->osal_work;
        }
    }
    HI_TRACE_OSAL(HI_DBG_ERR, "find work failed!\n");
    return NULL;
}

static int osal_del_work(struct work_struct *work)
{
    struct osal_list_head *this = NULL;
    struct osal_list_head *next = NULL;
    if (osal_list_empty(&wq_list)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "find work failed! wq_list is empty!\n");
        return -1;
    }
    osal_list_for_each_safe(this, next, &wq_list) {
        struct wq_node *ws = osal_list_entry(this, struct wq_node, node);
        if (ws->work == work) {
            osal_list_del(this);
            osal_kfree(ws);
            return 0;
        }
    }
    HI_TRACE_OSAL(HI_DBG_ERR, "del work failed!\n");
    return -1;
}

static void osal_work_func(struct work_struct *work)
{
    struct osal_work_struct *ow = osal_find_work(work);
    if ((ow != NULL) && (ow->func != NULL)) {
        ow->func(ow);
    }
}

int osal_init_work(struct osal_work_struct *work, osal_work_func_t func)
{
    struct work_struct *w = NULL;
    struct wq_node *w_node = NULL;
    w = osal_kmalloc(sizeof(struct work_struct), osal_gfp_atomic);
    if (w == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "osal_init_work kmalloc failed!\n");
        return -1;
    }

    w_node = osal_kmalloc(sizeof(struct wq_node), osal_gfp_atomic);
    if (w_node == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "osal_init_work kmalloc failed!\n");
        osal_kfree(w);
        return -1;
    }
    INIT_WORK(w, osal_work_func);
    work->work = w;
    work->func = func;
    w_node->osal_work = work;
    w_node->work = w;
    osal_list_add(&(w_node->node), &wq_list);
    return 0;
}
EXPORT_SYMBOL(osal_init_work);

int osal_schedule_work(struct osal_work_struct *work)
{
    if ((work != NULL) && (work->work != NULL)) {
#ifndef AOS_LLVM_BUILD
        return (int)schedule_work(work->work);
#else
        return (int)schedule_work_on(0, work->work);
#endif  // #ifndef AOS_LLVM_BUILD
    } else {
        return (int)false;
    }
    return 0;
}
EXPORT_SYMBOL(osal_schedule_work);

void osal_destroy_work(struct osal_work_struct *work)
{
    if ((work != NULL) && (work->work != NULL)) {
        osal_del_work(work->work);
        osal_kfree((struct work_struct *)work->work);
        work->work = NULL;
    }
}
EXPORT_SYMBOL(osal_destroy_work);

