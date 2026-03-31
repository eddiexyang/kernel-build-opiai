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

#ifndef DAVINCI_INTF_UT
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/list.h>
#include <linux/atomic.h>

#include "securec.h"
#include "davinci_interface.h"
#include "davinci_api.h"
#include "davinci_intf_init.h"
#include "davinci_intf_common.h"
#include "davinci_intf_process.h"
#ifdef CFG_HOST_ENV
#include "tsdrv_common.h"
#endif


/* create file node info */
struct davinci_intf_file_stru *create_file_proc_entry(
    struct davinci_intf_process_stru *proc,
    struct file *file)
{
    struct davinci_intf_file_stru *file_node = NULL;
    static unsigned int seq = 0;

    file_node = (struct davinci_intf_file_stru *)kzalloc(sizeof(struct davinci_intf_file_stru),
        GFP_KERNEL | __GFP_ACCOUNT);
    if (file_node == NULL) {
        log_intf_err("kzalloc failed. (size=%lu)\n", sizeof(struct davinci_intf_file_stru));
        return NULL;
    }
    file_node->file_op = file;
    file_node->owner_pid = current->tgid;
    file_node->seq = seq++;
    file_node->open_time = jiffies_to_msecs(jiffies);
    list_add_tail(&file_node->list, &proc->file_list);
    return file_node;
}

struct davinci_intf_file_stru *get_file_proc_entry(
    struct davinci_intf_process_stru *proc,
    struct file *file)
{
    struct davinci_intf_file_stru *file_node = NULL;
    struct list_head *file_list = NULL;
    struct davinci_intf_file_stru *next = NULL;

    file_list = &proc->file_list;
    list_for_each_entry_safe(file_node, next, file_list, list)
    {
        if (file_node->file_op == file) {
            return file_node;
        }
    }
    return NULL;
}

unsigned int get_file_module_cnt(struct davinci_intf_process_stru *proc, const char *module_name)
{
    struct davinci_intf_file_stru *file_node = NULL;
    struct list_head *file_list = NULL;
    struct davinci_intf_file_stru *next = NULL;
    unsigned int cnt = 0;

    file_list = &proc->file_list;
    list_for_each_entry_safe(file_node, next, file_list, list)
    {
        if (strcmp(file_node->module_name, module_name) == 0) {
            cnt++;
        }
    }
    return cnt;
}

void destory_file_proc_entry(
    struct davinci_intf_process_stru *proc,
    struct file *file)
{
    struct davinci_intf_file_stru *file_node = NULL;
    struct list_head *file_list = NULL;
    struct davinci_intf_file_stru *next = NULL;

    file_list = &proc->file_list;

    list_for_each_entry_safe(file_node, next, file_list, list)
    {
        if (file_node->file_op == file) {
            list_del(&file_node->list);
            kfree(file_node);
            file_node = NULL;
            return;
        }
    }
    return;
}

int check_module_file_close_in_process(
    struct davinci_intf_process_stru *proc,
    const char *module_name)
{
    struct davinci_intf_file_stru *file_node = NULL;
    struct list_head *file_list = NULL;
    struct davinci_intf_file_stru *next = NULL;

    file_list = &proc->file_list;

    list_for_each_entry_safe(file_node, next, file_list, list)
    {
        if (strcmp(file_node->module_name, module_name) == 0) {
            return FALSE;
        }
    }
    return TRUE;
}

void destory_file_proc_list(struct davinci_intf_process_stru *proc)
{
    struct davinci_intf_file_stru *file_node = NULL;
    struct list_head *file_list = NULL;
    struct davinci_intf_file_stru *next = NULL;
    file_list = &proc->file_list;

    list_for_each_entry_safe(file_node, next, file_list, list)
    {
        list_del(&file_node->list);
        kfree(file_node);
        file_node = NULL;
    }
    return;
}

struct davinci_intf_process_stru *create_process_entry(
    struct davinci_intf_stru *cb,
    pid_t  proc_pid)
{
    struct davinci_intf_process_stru *proc_node = NULL;
    struct list_head *process_list = NULL;
    process_list = &cb->process_list;

    proc_node = (struct davinci_intf_process_stru *)kzalloc(
        sizeof(struct davinci_intf_process_stru),
        GFP_KERNEL | __GFP_ACCOUNT);
    if (proc_node == NULL) {
        log_intf_err("kzalloc failed. (size=%lu)\n", sizeof(struct davinci_intf_process_stru));
        return NULL;
    }
    proc_node->owner_pid = proc_pid;
    proc_node->owner_cb = (void *)cb;

    mutex_init(&proc_node->res_lock);
    INIT_LIST_HEAD(&proc_node->file_list);
    list_add_tail(&proc_node->list, &cb->process_list);
    atomic_set(&proc_node->work_count, 0);
    return proc_node;
}

struct davinci_intf_process_stru *get_process_entry(
    struct davinci_intf_stru *cb,
    pid_t proc_pid)
{
    struct davinci_intf_process_stru *proc_node = NULL;
    struct list_head *process_list = NULL;
    struct davinci_intf_process_stru *next = NULL;

    process_list = &cb->process_list;

    list_for_each_entry_safe(proc_node, next, process_list, list)
    {
        if (proc_node->owner_pid == proc_pid) {
            return proc_node;
        }
    }
    return NULL;
}

void destroy_process_entry(struct davinci_intf_stru *cb)
{
    struct davinci_intf_process_stru *proc_node = NULL;
    struct list_head *process_list = NULL;
    struct davinci_intf_process_stru *next = NULL;
    pid_t  proc_pid = current->tgid;

    process_list = &cb->process_list;

    list_for_each_entry_safe(proc_node, next, process_list, list)
    {
        if (proc_node->owner_pid == proc_pid) {
            list_del(&proc_node->list);
            destory_file_proc_list(proc_node);
            kfree(proc_node);
            proc_node = NULL;
            return;
        }
    }
    return;
}

void free_process_entry(struct davinci_intf_process_stru *proc)
{
    if (proc == NULL) {
        return;
    }
    mutex_destroy(&proc->res_lock);
    return;
}

void destroy_process_list(struct davinci_intf_stru *cb)
{
    struct davinci_intf_process_stru *proc_node = NULL;
    struct list_head *process_list = NULL;
    struct davinci_intf_process_stru *next = NULL;

    process_list = &cb->process_list;

    list_for_each_entry_safe(proc_node, next, process_list, list)
    {
        list_del(&proc_node->list);
        destory_file_proc_list(proc_node);
        kfree(proc_node);
        proc_node = NULL;
    }
    return;
}

int check_module_file_close(
    struct davinci_intf_stru *cb,
    const char *module_name)
{
    struct davinci_intf_process_stru *proc_node = NULL;
    struct list_head *process_list = NULL;
    struct davinci_intf_process_stru *next = NULL;

    process_list = &cb->process_list;

    list_for_each_entry_safe(proc_node, next, process_list, list)
    {
        if (check_module_file_close_in_process(proc_node, module_name) == FALSE) {
            return FALSE;
        }
    }
    return TRUE;
}

int add_file_to_list(struct davinci_intf_stru *cb, struct file *file)
{
    struct davinci_intf_process_stru *proc = NULL;
    struct davinci_intf_file_stru *file_op = NULL;
    pid_t  proc_pid = current->tgid;

    proc = get_process_entry(cb, proc_pid);
    if (proc == NULL) {
        proc = create_process_entry(cb, proc_pid);
        if (proc == NULL) {
            return -ENOMEM;
        }
    }

    file_op = get_file_proc_entry(proc, file);
    if (file_op != NULL) {
        log_intf_err("File already init. (pid=%d; file=%llx)\n",
            proc_pid,
            (unsigned long long)(uintptr_t)file);
        return -EBADF;
    }

    file_op = create_file_proc_entry(proc, file);
    if (file_op == NULL) {
        return -ENOMEM;
    }
    return 0;
}

int add_module_to_list(struct davinci_intf_stru *cb,
    pid_t  owner_pid,
    struct file *file,
    const char *module_name)
{
    struct davinci_intf_process_stru *proc = NULL;
    struct davinci_intf_file_stru *file_op = NULL;
    pid_t  proc_pid = owner_pid;
    int ret;

    proc = get_process_entry(cb, proc_pid);
    if (proc == NULL) {
        return -ESRCH;
    }

    file_op = get_file_proc_entry(proc, file);
    if (file_op == NULL) {
        return -EBADF;
    }

    ret = strcpy_s(file_op->module_name, DAVINIC_MODULE_NAME_MAX, module_name);
    if (ret != 0) {
        log_intf_err("strcpy_s error. (module_name=\"%s\"; ret=%d)\n",
            module_name,
            ret);
        return ret;
    }
    return 0;
}

#else
void remove_file_from_list(void)
{
}
#endif
