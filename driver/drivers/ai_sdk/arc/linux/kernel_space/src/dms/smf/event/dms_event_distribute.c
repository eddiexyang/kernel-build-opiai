/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 and
* only version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* Description:
* Author: huawei
* Create: 2019-10-15
*/


#include <linux/errno.h>
#include <linux/slab.h>

#include "dms_define.h"
#include "dms_event.h"
#include "dms_event_distribute.h"
#include "dms_event_dfx.h"

#define DMS_INVALID_PID (-1)

#define DMS_EXCEPTION_KFIFO_CELL (sizeof(DMS_EVENT_NODE_STRU))
#define DMS_EXCEPTION_KFIFO_SIZE (DMS_EXCEPTION_KFIFO_CELL * DMS_MAX_EVENT_NUM)

STATIC DMS_EVENT_PROCESS_STRU g_dms_event_process[DMS_EVENT_EXCEPTION_PROCESS_MAX];
static DMS_EVENT_DISTRIBUTE_HANDLE_T g_dms_event_handle_list[DMS_EVENT_DISTRIBUTE_FUNC_MAX];
static struct mutex g_dms_event_handle_mutex;
static struct mutex g_dms_event_process_mutex;

static void dms_event_release_one_process(u32 process_index);

void dms_event_distribute_stru_init(void)
{
    u32 i;

    mutex_init(&g_dms_event_handle_mutex);
    mutex_init(&g_dms_event_process_mutex);

    mutex_lock(&g_dms_event_process_mutex);
    for (i = 0; i < DMS_EVENT_EXCEPTION_PROCESS_MAX; i++) {
        init_waitqueue_head(&g_dms_event_process[i].event_wait);
        mutex_init(&g_dms_event_process[i].process_mutex);
        mutex_lock(&g_dms_event_process[i].process_mutex);
        g_dms_event_process[i].exception_num = 0;
        g_dms_event_process[i].process_pid = DMS_INVALID_PID;
        g_dms_event_process[i].process_tid = DMS_INVALID_PID;
        g_dms_event_process[i].process_status = DMS_EVENT_PROCESS_STATUS_IDLE;
        mutex_unlock(&g_dms_event_process[i].process_mutex);
    }
    mutex_unlock(&g_dms_event_process_mutex);

    mutex_lock(&g_dms_event_handle_mutex);
    for (i = 0; i < DMS_EVENT_DISTRIBUTE_FUNC_MAX; i++) {
        g_dms_event_handle_list[i] = NULL;
    }
    mutex_unlock(&g_dms_event_handle_mutex);
}

void dms_event_distribute_stru_exit(void)
{
    u32 i;

    mutex_lock(&g_dms_event_process_mutex);
    for (i = 0; i < DMS_EVENT_EXCEPTION_PROCESS_MAX; i++) {
        dms_event_release_one_process(i);
        mutex_destroy(&g_dms_event_process[i].process_mutex);
    }
    mutex_unlock(&g_dms_event_process_mutex);

    mutex_lock(&g_dms_event_handle_mutex);
    for (i = 0; i < DMS_EVENT_DISTRIBUTE_FUNC_MAX; i++) {
        g_dms_event_handle_list[i] = NULL;
    }
    mutex_unlock(&g_dms_event_handle_mutex);

    mutex_destroy(&g_dms_event_handle_mutex);
    mutex_destroy(&g_dms_event_process_mutex);
}

STATIC int dms_event_add_to_process_list(u32 process_index, DMS_EVENT_NODE_STRU *exception_node)
{
    DMS_EVENT_PROCESS_STRU *event_proc = &g_dms_event_process[process_index];
    DMS_EVENT_NODE_STRU exception_buf = {{0}};

    mutex_lock(&event_proc->process_mutex);
    if (event_proc->process_status != DMS_EVENT_PROCESS_STATUS_WORK) {
        mutex_unlock(&event_proc->process_mutex);
        return DRV_ERROR_NONE;
    }
    while (kfifo_avail(&event_proc->event_fifo) < DMS_EXCEPTION_KFIFO_CELL) {
        if (!kfifo_out(&event_proc->event_fifo, &exception_buf, DMS_EXCEPTION_KFIFO_CELL)) {
            dms_warn("kfifo_out failed. (process_index=%u)\n", process_index);
        }
        dms_event("Dms event is covered. (dev_id=%u; event_id=0x%X; event_serial_num=%d; notify_serial_num=%d; "
                  "pid=%d; tid=%d)\n",
                  exception_buf.event.deviceid, exception_buf.event.event_id, exception_buf.event.event_serial_num,
                  exception_buf.event.notify_serial_num, event_proc->process_pid, event_proc->process_tid);
    }

    kfifo_in(&event_proc->event_fifo, exception_node, DMS_EXCEPTION_KFIFO_CELL);
    event_proc->exception_num = kfifo_len(&event_proc->event_fifo) / DMS_EXCEPTION_KFIFO_CELL;
    mutex_unlock(&event_proc->process_mutex);

    wake_up_interruptible(&event_proc->event_wait);
    return DRV_ERROR_NONE;
}

int dms_event_distribute_to_process(DMS_EVENT_NODE_STRU *exception_node)
{
    int fail_num = 0;
    u32 i;

    for (i = 0; i < DMS_EVENT_EXCEPTION_PROCESS_MAX; i++) {
        if (g_dms_event_process[i].process_status != DMS_EVENT_PROCESS_STATUS_WORK) {
            continue;
        }
        if (dms_event_add_to_process_list(i, exception_node)) {
            fail_num++;
            dms_err("Add event code to process failed. "
                    "(dev_id=%u; event_code=0x%x; process_index=%u)\n",
                    exception_node->event.deviceid, exception_node->event.event_code, i);
        }
    }

    if (fail_num) {
        dms_err("Distribute event code to process list failed. "
                "(dev_id=%u; event_code=0x%x; fail_num=%d)\n",
                exception_node->event.deviceid, exception_node->event.event_code, fail_num);
    }
    return fail_num;
}

static u32 dms_event_get_process_index(pid_t process_pid, pid_t process_tid)
{
    u32 i;
#ifndef AOS_LLVM_BUILD
    struct pid *original_pid = NULL;
#endif

    mutex_lock(&g_dms_event_process_mutex);
    for (i = 0; i < DMS_EVENT_EXCEPTION_PROCESS_MAX; i++) {
        if (g_dms_event_process[i].process_pid != process_pid) {
            continue;
        }
#ifndef AOS_LLVM_BUILD
        rcu_read_lock();
        original_pid = find_vpid(g_dms_event_process[i].process_tid);
        rcu_read_unlock();
        if (original_pid != NULL && g_dms_event_process[i].process_tid != process_tid) {
#else
        if (g_dms_event_process[i].process_tid != process_tid) {
#endif
            dms_err("Has start one thread before. (pid=%d; tid=%d)\n", process_pid, process_tid);
            mutex_unlock(&g_dms_event_process_mutex);
            return DMS_EVENT_EXCEPTION_PROCESS_MAX;
        }
        mutex_unlock(&g_dms_event_process_mutex);
        return i;
    }

    for (i = 0; i < DMS_EVENT_EXCEPTION_PROCESS_MAX; i++) {
        mutex_lock(&g_dms_event_process[i].process_mutex);
        if (g_dms_event_process[i].process_status == DMS_EVENT_PROCESS_STATUS_IDLE) {
            if (kfifo_alloc(&g_dms_event_process[i].event_fifo, DMS_EXCEPTION_KFIFO_SIZE, GFP_KERNEL | __GFP_ACCOUNT)) {
                mutex_unlock(&g_dms_event_process[i].process_mutex);
                mutex_unlock(&g_dms_event_process_mutex);
                dms_err("kfifo_alloc failed. (process_pid=%d)\n", process_pid);
                return DMS_EVENT_EXCEPTION_PROCESS_MAX;
            }
            kfifo_reset(&g_dms_event_process[i].event_fifo);
            g_dms_event_process[i].exception_num = 0;
            g_dms_event_process[i].process_pid = process_pid;
            g_dms_event_process[i].process_tid = process_tid;
            g_dms_event_process[i].process_status = DMS_EVENT_PROCESS_STATUS_WORK;
            mutex_unlock(&g_dms_event_process[i].process_mutex);
            mutex_unlock(&g_dms_event_process_mutex);
            dms_debug("Get a process success. (tgid=%d; pid=%d; index=%u)\n", process_pid, process_tid, i);
            return i;
        }
        mutex_unlock(&g_dms_event_process[i].process_mutex);
    }
    mutex_unlock(&g_dms_event_process_mutex);

    dms_err("Process list is full. (process_pid=%d)\n", process_pid);
    return DMS_EVENT_EXCEPTION_PROCESS_MAX;
}

static int dms_event_get_exception_by_process_index(u32 process_index,
    struct dms_event_para *fault_event)
{
    DMS_EVENT_PROCESS_STRU *event_proc = &g_dms_event_process[process_index];
    DMS_EVENT_NODE_STRU exception_node = {{0}};
    int ret = DRV_ERROR_NONE;

    mutex_lock(&event_proc->process_mutex);
    if (kfifo_len(&event_proc->event_fifo) < DMS_EXCEPTION_KFIFO_CELL) {
        dms_warn("The process index haven't exception. (process_index=%u)\n", process_index);
        ret = -ETIMEDOUT;
        goto out;
    }

    if (!kfifo_out(&event_proc->event_fifo, &exception_node, DMS_EXCEPTION_KFIFO_CELL)) {
        dms_err("kfifo_out failed. (process_index=%u)\n", process_index);
        ret = DRV_ERROR_INNER_ERR;
        goto out;
    }

    if (memcpy_s(fault_event, sizeof(struct dms_event_para),
                 &exception_node.event, sizeof(struct dms_event_para))) {
        dms_err("Call memcpy_s failed. (process_index=%u)\n", process_index);
        ret = DRV_ERROR_INNER_ERR;
        goto out;
    }

out:
    event_proc->exception_num = kfifo_len(&event_proc->event_fifo) / DMS_EXCEPTION_KFIFO_CELL;
    mutex_unlock(&event_proc->process_mutex);
    return ret;
}

STATIC int dms_event_clear_exception_by_process_index(u32 process_index,
    u32 devid)
{
    DMS_EVENT_PROCESS_STRU *event_proc = &g_dms_event_process[process_index];
    DMS_EVENT_NODE_STRU exception_node = {{0}};
    struct kfifo fifo_buf;

    mutex_lock(&event_proc->process_mutex);
    if (event_proc->process_status != DMS_EVENT_PROCESS_STATUS_WORK) {
        mutex_unlock(&event_proc->process_mutex);
        return DRV_ERROR_NONE;
    }

    if (kfifo_is_empty(&event_proc->event_fifo)) {
        mutex_unlock(&event_proc->process_mutex);
        return DRV_ERROR_NONE;
    }

    if (kfifo_alloc(&fifo_buf, DMS_EXCEPTION_KFIFO_SIZE, GFP_KERNEL)) {
        mutex_unlock(&event_proc->process_mutex);
        dms_err("kfifo_alloc failed. (process_index=%u)\n", process_index);
        return DRV_ERROR_OUT_OF_MEMORY;
    }
    kfifo_reset(&fifo_buf);

    while (kfifo_len(&event_proc->event_fifo) >= DMS_EXCEPTION_KFIFO_CELL) {
        if (!kfifo_out(&event_proc->event_fifo, &exception_node, DMS_EXCEPTION_KFIFO_CELL)) {
            dms_warn("kfifo_out from event_proc failed. (process_index=%u)\n", process_index);
            continue;
        }
        if (exception_node.event.deviceid != (unsigned short)devid) {
            kfifo_in(&fifo_buf, &exception_node, DMS_EXCEPTION_KFIFO_CELL);
        }
    }
    kfifo_reset(&event_proc->event_fifo);

    while (kfifo_len(&fifo_buf) >= DMS_EXCEPTION_KFIFO_CELL) {
        if (!kfifo_out(&fifo_buf, &exception_node, DMS_EXCEPTION_KFIFO_CELL)) {
            dms_warn("kfifo_out from fifo_buf failed. (process_index=%u)\n", process_index);
            continue;
        }
        kfifo_in(&event_proc->event_fifo, &exception_node, DMS_EXCEPTION_KFIFO_CELL);
    }
    event_proc->exception_num = kfifo_len(&event_proc->event_fifo) / DMS_EXCEPTION_KFIFO_CELL;
    mutex_unlock(&event_proc->process_mutex);

    kfifo_free(&fifo_buf);
    return DRV_ERROR_NONE;
}

int dms_event_get_exception(struct dms_event_para *fault_event, int timeout)
{
    u32 process_index;
    int ret;

    if (fault_event == NULL) {
        dms_err("Invalid parameter, fault_event is null.\n");
        return DRV_ERROR_PARA_ERROR;
    }

    process_index = dms_event_get_process_index(current->tgid, current->pid);
    if (process_index >= DMS_EVENT_EXCEPTION_PROCESS_MAX) {
        dms_err("The index of process is invalid. (process_index=%u)\n", process_index);
        return DRV_ERROR_RESOURCE_OCCUPIED;
    }

    if (timeout == DMS_EVENT_NO_TIMEOUT_FLAG) {
        do {
            ret = wait_event_interruptible_timeout(g_dms_event_process[process_index].event_wait,
                                                   (g_dms_event_process[process_index].exception_num > 0),
                                                   msecs_to_jiffies(DMS_EVENT_WAIT_TIME));
        } while (ret == 0);
    } else if ((timeout >= 0) && (timeout <= DMS_EVENT_WAIT_TIME_MAX)) {
        ret = wait_event_interruptible_timeout(g_dms_event_process[process_index].event_wait,
                                               (g_dms_event_process[process_index].exception_num > 0),
                                               msecs_to_jiffies((unsigned int)timeout));
    } else {
        dms_err("Invalid parameter. (timeout=%d)\n", timeout);
        return DRV_ERROR_PARA_ERROR;
    }

    if (ret == -ERESTARTSYS) {
        return -ERESTARTSYS;
    } else if (ret == 0) {
        return -ETIMEDOUT;
    }

    ret = dms_event_get_exception_by_process_index(process_index, fault_event);
    if (ret == -ETIMEDOUT) {
        return ret;
    } else if (ret) {
        dms_err("Get exception by process index failed. (process_index=%u; ret=%d)\n", process_index, ret);
        return ret;
    }

    return DRV_ERROR_NONE;
}

int dms_event_clear_exception(u32 devid)
{
    u32 process_index;

    for (process_index = 0; process_index < DMS_EVENT_EXCEPTION_PROCESS_MAX; process_index++) {
        dms_event_clear_exception_by_process_index(process_index, devid);
    }

    return DRV_ERROR_NONE;
}

static void dms_event_release_one_process(u32 process_index)
{
    mutex_lock(&g_dms_event_process[process_index].process_mutex);
    kfifo_free(&g_dms_event_process[process_index].event_fifo);
    g_dms_event_process[process_index].exception_num = 0;
    g_dms_event_process[process_index].process_pid = DMS_INVALID_PID;
    g_dms_event_process[process_index].process_tid = DMS_INVALID_PID;
    g_dms_event_process[process_index].process_status = DMS_EVENT_PROCESS_STATUS_IDLE;
    mutex_unlock(&g_dms_event_process[process_index].process_mutex);
}

void dms_event_release_proc(void)
{
    u32 process_index;

    mutex_lock(&g_dms_event_process_mutex);
    for (process_index = 0; process_index < DMS_EVENT_EXCEPTION_PROCESS_MAX; process_index++) {
        if (g_dms_event_process[process_index].process_pid == current->tgid) {
            dms_event_release_one_process(process_index);
            mutex_unlock(&g_dms_event_process_mutex);
            dms_debug("Release one process. (tgid=%d; pid=%d; index=%u)\n",
                      current->tgid, current->pid, process_index);
            return;
        }
    }
    mutex_unlock(&g_dms_event_process_mutex);
}
#ifndef DMS_SHARED_NO_EXPORT
EXPORT_SYMBOL(dms_event_release_proc);
#endif

int dms_event_subscribe_register(DMS_EVENT_DISTRIBUTE_HANDLE_T handle_func,
    DMS_DISTRIBUTE_PRIORITY priority)
{
    u32 i;

    if ((priority >= DMS_DISTRIBUTE_PRIORITY_MAX) ||
        (handle_func == NULL)) {
        dms_err("Invaild pararmeter. (priority=%u; handle_func=%s).\n",
                priority, (handle_func == NULL) ? "NULL" : "OK");
        return DRV_ERROR_PARA_ERROR;
    }

    mutex_lock(&g_dms_event_handle_mutex);
    for (i = DISTRIBUTE_FUNC_HEAD_INDEX(priority); i < DISTRIBUTE_FUNC_TAIL_INDEX(priority); i++) {
        if (g_dms_event_handle_list[i] == NULL) {
            g_dms_event_handle_list[i] = handle_func;
            mutex_unlock(&g_dms_event_handle_mutex);
            return DRV_ERROR_NONE;
        }
    }
    mutex_unlock(&g_dms_event_handle_mutex);

    dms_err("The distribute handle list is full. (priority=%u).\n", priority);
    return DRV_ERROR_NO_RESOURCES;
}
#ifndef DMS_SHARED_NO_EXPORT
EXPORT_SYMBOL(dms_event_subscribe_register);
#endif

void dms_event_subscribe_unregister(DMS_EVENT_DISTRIBUTE_HANDLE_T handle_func)
{
    u32 i;

    mutex_lock(&g_dms_event_handle_mutex);
    for (i = 0; i < DMS_EVENT_DISTRIBUTE_FUNC_MAX; i++) {
        if (g_dms_event_handle_list[i] == handle_func) {
            g_dms_event_handle_list[i] = NULL;
            break;
        }
    }
    mutex_unlock(&g_dms_event_handle_mutex);
}
#ifndef DMS_SHARED_NO_EXPORT
EXPORT_SYMBOL(dms_event_subscribe_unregister);
#endif

void dms_event_subscribe_unregister_all(void)
{
    u32 i;

    mutex_lock(&g_dms_event_handle_mutex);
    for (i = 0; i < DMS_EVENT_DISTRIBUTE_FUNC_MAX; i++) {
        g_dms_event_handle_list[i] = NULL;
    }
    mutex_unlock(&g_dms_event_handle_mutex);
}

int dms_event_distribute_handle(DMS_EVENT_NODE_STRU *exception_node, DMS_DISTRIBUTE_PRIORITY priority)
{
    int ret;
    u32 i;

    if ((priority >= DMS_DISTRIBUTE_PRIORITY_MAX) ||
        (exception_node == NULL)) {
        dms_err("Invalid pararmeter. (priority=%u; exception=%s).\n",
                priority, (exception_node == NULL) ? "NULL" : "OK");
        return DRV_ERROR_PARA_ERROR;
    }

    mutex_lock(&g_dms_event_handle_mutex);
    for (i = DISTRIBUTE_FUNC_HEAD_INDEX(priority); i < DMS_EVENT_DISTRIBUTE_FUNC_MAX; i++) {
        if (g_dms_event_handle_list[i] != NULL) {
            ret = g_dms_event_handle_list[i](exception_node);
            if (ret) {
                dms_err("Distribute event failed. (handle_index=%u; ret=%d)\n", i, ret);
                mutex_unlock(&g_dms_event_handle_mutex);
                return ret;
            }
        }
    }
    mutex_unlock(&g_dms_event_handle_mutex);

    return DRV_ERROR_NONE;
}
#ifndef DMS_SHARED_NO_EXPORT
EXPORT_SYMBOL(dms_event_distribute_handle);
#endif

ssize_t dms_event_print_subscribe_handle(char *str)
{
    int len, avl_len = EVENT_DFX_BUF_SIZE_MAX;
    char *refill_buf = str;
    u32 i;

    len = snprintf_s(str, avl_len, avl_len - 1, "Print subscribe handle function begin:\n");
    EVENT_DFX_CHECK_DO_SOMETHING(len < 0, return 0);
    str += len;
    avl_len -= len;

    mutex_lock(&g_dms_event_handle_mutex);

    for (i = 0; i < DMS_EVENT_DISTRIBUTE_FUNC_MAX; i++) {
        if (g_dms_event_handle_list[i] != NULL) {
            len = snprintf_s(str, avl_len, avl_len - 1, "event handle[%u]=ok\n", i);
            EVENT_DFX_CHECK_DO_SOMETHING(len < 0, goto out);
            str += len;
            avl_len -= len;
        }
    }

    len = snprintf_s(str, avl_len, avl_len - 1, "Print subscribe handle function end.\n");
    EVENT_DFX_CHECK_DO_SOMETHING(len < 0, goto out);
    str += len;

    mutex_unlock(&g_dms_event_handle_mutex);
    return str - refill_buf;
out:
    mutex_unlock(&g_dms_event_handle_mutex);
    dms_warn("snprintf_s failed.\n");
    return 0;
}

ssize_t dms_event_print_subscribe_process(char *str)
{
    int len, avl_len = EVENT_DFX_BUF_SIZE_MAX;
    char *refill_buf = str;
    u32 i;

    len = snprintf_s(str, avl_len, avl_len - 1, "Print subscribe process begin:\n");
    EVENT_DFX_CHECK_DO_SOMETHING(len < 0, return 0);
    str += len;
    avl_len -= len;

    mutex_lock(&g_dms_event_process_mutex);

    for (i = 0; i < DMS_EVENT_EXCEPTION_PROCESS_MAX; i++) {
        if (g_dms_event_process[i].process_status != DMS_EVENT_PROCESS_STATUS_WORK) {
            continue;
        }
        mutex_lock(&g_dms_event_process[i].process_mutex);
        len = snprintf_s(str, avl_len, avl_len - 1, "process[%u]: tgid=%d; pid=%d; event_num=%u\n",
                         i, g_dms_event_process[i].process_tid, g_dms_event_process[i].process_pid,
                         g_dms_event_process[i].exception_num);
        mutex_unlock(&g_dms_event_process[i].process_mutex);
        EVENT_DFX_CHECK_DO_SOMETHING(len < 0, goto out);
        str += len;
        avl_len -= len;
    }

    len = snprintf_s(str, avl_len, avl_len - 1, "Print subscribe process end.\n");
    EVENT_DFX_CHECK_DO_SOMETHING(len < 0, goto out);
    str += len;

    mutex_unlock(&g_dms_event_process_mutex);
    return str - refill_buf;
out:
    mutex_unlock(&g_dms_event_process_mutex);
    dms_warn("snprintf_s failed.\n");
    return 0;
}
