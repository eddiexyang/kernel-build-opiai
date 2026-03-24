/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
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
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/version.h>
#ifdef CFG_FEATURE_EXTERNAL_CDEV
#include "davinci_api.h"
#endif
#include "dbl/feature_loader.h"
#include "esched.h"
#include "esched_sysfs.h"
#include "esched_fops.h"


#ifdef AOS_LLVM_BUILD
#include "aos_dlib.h"
#endif

struct sched_char_dev char_dev;

#ifdef ESCHED_HOST
#define PCI_VENDOR_ID_HUAWEI 0x19e5
#define DEVDRV_DIVERSITY_PCIE_VENDOR_ID 0xFFFF
const struct pci_device_id g_esched_driver_tbl[] = {{ PCI_VDEVICE(HUAWEI, 0xd100), 0 },
                                                    { PCI_VDEVICE(HUAWEI, 0xd105), 0 },
                                                    { PCI_VDEVICE(HUAWEI, 0xa126), 0 },
                                                    { PCI_VDEVICE(HUAWEI, 0xd801), 0 },
                                                    { PCI_VDEVICE(HUAWEI, 0xd500), 0 },
                                                    { PCI_VDEVICE(HUAWEI, 0xd501), 0 },
                                                    { PCI_VDEVICE(HUAWEI, 0xd802), 0 },
                                                    { PCI_VDEVICE(HUAWEI, 0xd803), 0 },
                                                    { PCI_VDEVICE(HUAWEI, 0xd804), 0 },
                                                    { PCI_VDEVICE(HUAWEI, 0xd805), 0 },
                                                    { DEVDRV_DIVERSITY_PCIE_VENDOR_ID, 0xd500,
                                                      PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
                                                    {}};
MODULE_DEVICE_TABLE(pci, g_esched_driver_tbl);
#endif

int32_t copy_from_user_safe(void *to, const void __user *from, unsigned long n)
{
    if ((from == NULL) || (n == 0)) {
        sched_err("The variable from is NULL or n is zero.\n");
        return DRV_ERROR_INNER_ERR;
    }

    if (copy_from_user(to, (void *)from, n) != 0) {
        sched_err("Failed to invoke the copy_from_user. (size=%lu)\n", n);
        return DRV_ERROR_INNER_ERR;
    }

    return 0;
}

int32_t copy_to_user_safe(void __user *to, const void *from, unsigned long n)
{
    if ((to == NULL) || (n == 0)) {
        sched_err("The variable to is NULL or n is zero.\n");
        return DRV_ERROR_INNER_ERR;
    }

    if (copy_to_user(to, (void *)from, n) != 0) {
        sched_err("Failed to invoke the copy_from_user. (size=%lu)\n", n);
        return DRV_ERROR_INNER_ERR;
    }

    return 0;
}

u32 sched_ioctl_devid(u32 open_devid, u32 cmd_devid)
{
#ifdef CFG_FEATURE_EXTERNAL_CDEV
    return open_devid;
#else
    return cmd_devid;
#endif
}

STATIC int32_t sched_fop_set_sched_cpu(u32 devid, unsigned long arg)
{
    struct sched_ioctl_para_cpu_info para;

    if (copy_from_user_safe(&para, (void *)(uintptr_t)arg, sizeof(struct sched_ioctl_para_cpu_info)) != 0) {
        return DRV_ERROR_COPY_USER_FAIL;
    }

#ifdef ESCHED_HOST
    return DRV_ERROR_INVALID_DEVICE;
#else
    return sched_set_sched_cpu(sched_ioctl_devid(devid, para.dev_id), &para.cpu_mask);
#endif
}

STATIC int32_t sched_fop_proc_add_grp(u32 devid, unsigned long arg)
{
    struct sched_ioctl_para_add_grp para;

    if (copy_from_user_safe(&para, (void *)(uintptr_t)arg, sizeof(struct sched_ioctl_para_add_grp)) != 0) {
        return DRV_ERROR_COPY_USER_FAIL;
    }

    return sched_proc_add_grp(sched_ioctl_devid(devid, para.dev_id), para.gid, para.grp_name,
        para.sched_mode, para.thread_num);
}

#ifndef AOS_LLVM_BUILD
STATIC int32_t sched_get_tgid_by_vpid(int32_t vpid, int32_t *tgid)
{
    struct task_struct *task = NULL;

    rcu_read_lock();
    task = get_pid_task(find_vpid(vpid), PIDTYPE_PID);
    rcu_read_unlock();
    if (task == NULL) {
#if !defined (CFG_SOC_PLATFORM_MDC_V51) && !defined (CFG_SOC_PLATFORM_MDC_V11)
        sched_err("Failed to get pid task. (vpid=%d)\n", vpid);
#endif
        return DRV_ERROR_NO_PROCESS;
    }

    *tgid = task->pid;
    put_task_struct(task);

    return DRV_ERROR_NONE;
}
#endif

static int sched_query_task_gid(u32 devid, ESCHED_QUERY_TYPE type, struct esched_input_info *input,
    struct esched_output_info *output)
{
    struct esched_query_gid_output output_info;
    struct esched_query_gid_input input_info;
    int ret;

    if ((input->inLen != sizeof(struct esched_query_gid_input)) ||
        (output->outLen != sizeof(struct esched_query_gid_output))) {
        sched_err("Invalid para. (inLen=%u; outLen=%u)\n", input->inLen, output->outLen);
        return DRV_ERROR_PARA_ERROR;
    }

    if (copy_from_user_safe(&input_info, (void *)(uintptr_t)(input->inBuff), input->inLen) != 0) {
        return DRV_ERROR_COPY_USER_FAIL;
    }

    if (type == QUERY_TYPE_LOCAL_GRP_ID) {
#ifndef AOS_LLVM_BUILD
        ret = sched_get_tgid_by_vpid(input_info.pid, &input_info.pid);
        if (ret != 0) {
            return ret;
        }
#endif
        ret = sched_query_local_task_gid(devid, input_info.pid, input_info.grp_name, &output_info.grp_id);
    } else {
#ifdef CFG_FEATURE_REMOTE_SUBMIT
        ret = sched_query_remote_task_gid(devid, input_info.pid, input_info.grp_name, &output_info.grp_id);
#else
        ret = DRV_ERROR_NOT_SUPPORT;
#endif
    }

    if (ret != 0) {
        return ret;
    }

    if (copy_to_user_safe((void *)output->outBuff, &output_info, output->outLen) != 0) {
        return DRV_ERROR_COPY_USER_FAIL;
    }

    return 0;
}

STATIC int32_t sched_fop_query_info(u32 devid, unsigned long arg)
{
    struct sched_ioctl_para_query_info para;
    int ret;

    if (copy_from_user_safe(&para, (void *)(uintptr_t)arg, sizeof(para)) != 0) {
        return DRV_ERROR_COPY_USER_FAIL;
    }

    if ((para.input.inBuff == NULL) || (para.output.outBuff == NULL)) {
        sched_err("Invalid para. (inBuff=%u; outBuff=%u)\n", para.input.inBuff == NULL ? 1 : 0,
            para.output.outBuff == NULL ? 1 : 0);
        return DRV_ERROR_PARA_ERROR;
    }

    switch (para.type) {
        case QUERY_TYPE_LOCAL_GRP_ID:
        case QUERY_TYPE_REMOTE_GRP_ID:
            ret = sched_query_task_gid(sched_ioctl_devid(devid, para.dev_id),
                para.type, &para.input, &para.output);
            break;
        default:
            sched_err("Query info type is not valid. (type=%u)\n", para.type);
            return DRV_ERROR_NOT_SUPPORT;
    }

    return ret;
}

STATIC int32_t sched_fop_set_event_priority(u32 devid, unsigned long arg)
{
    struct sched_ioctl_para_set_event_pri para;

    if (copy_from_user_safe(&para, (void *)(uintptr_t)arg, sizeof(struct sched_ioctl_para_set_event_pri)) != 0) {
        return DRV_ERROR_COPY_USER_FAIL;
    }

    return sched_set_event_priority(sched_ioctl_devid(devid, para.dev_id), current->tgid, para.event_id, para.pri);
}

STATIC int32_t sched_fop_set_process_priority(u32 devid, unsigned long arg)
{
    struct sched_ioctl_para_set_proc_pri para;

    if (copy_from_user_safe(&para, (void *)(uintptr_t)arg, sizeof(struct sched_ioctl_para_set_proc_pri)) != 0) {
        return DRV_ERROR_COPY_USER_FAIL;
    }

    return sched_set_process_priority(sched_ioctl_devid(devid, para.dev_id), current->tgid, para.pri);
}

STATIC int32_t sched_fop_thread_subscribe_event(u32 devid, unsigned long arg)
{
    struct sched_ioctl_para_subscribe para;

    if (copy_from_user_safe(&para, (void *)(uintptr_t)arg, sizeof(struct sched_ioctl_para_subscribe)) != 0) {
        return DRV_ERROR_COPY_USER_FAIL;
    }

    return sched_thread_subscribe_event(sched_ioctl_devid(devid, para.dev_id), current->tgid, para.gid,
        para.tid, para.event_bitmap);
}

static int32_t sched_fop_grp_set_max_event_num(u32 devid, unsigned long arg)
{
    struct sched_ioctl_para_set_event_max_num para;
    int ret;

    ret = copy_from_user_safe(&para, (void *)(uintptr_t)arg, sizeof(struct sched_ioctl_para_set_event_max_num));
    if (ret != 0) {
        return DRV_ERROR_COPY_USER_FAIL;
    }

    return sched_grp_set_max_event_num(sched_ioctl_devid(devid, para.dev_id), current->tgid, para.gid,
        para.event_id, para.max_num);
}

STATIC int32_t sched_fop_get_exact_event(u32 devid, unsigned long arg)
{
    struct sched_ioctl_para_get_event para_info;
    struct sched_get_event_input *para = &para_info.input;
    int32_t ret;

    if (copy_from_user_safe(para, (void *)(uintptr_t)arg, sizeof(struct sched_get_event_input)) != 0) {
        return DRV_ERROR_COPY_USER_FAIL;
    }

    if ((para->msg == NULL) || (para->msg_len > SCHED_MAX_EVENT_MSG_LEN)) {
        sched_err("Invalid para. (dev_id=%u; msg_len=%u)\n", para->dev_id, para->msg_len);
        return DRV_ERROR_PARA_ERROR;
    }

    para_info.event.msg = para->msg;
    para_info.event.msg_len = para->msg_len;

    ret = sched_get_exact_event(sched_ioctl_devid(devid, para->dev_id), current->tgid, para->grp_id,
        para->thread_id, para->event_id, &para_info.event);
    if (unlikely(ret != 0)) {
        return ret;
    }

    if (copy_to_user_safe((void *)((uintptr_t)arg + sizeof(struct sched_get_event_input)),
        &para_info.event, sizeof(struct sched_subscribed_event)) != 0) {
        return DRV_ERROR_COPY_USER_FAIL;
    }

    return 0;
}

STATIC int32_t sched_fop_ack_event(u32 devid, unsigned long arg)
{
    struct sched_ioctl_para_ack para_info;
    char msg[SCHED_MAX_EVENT_MSG_LEN];

    if (copy_from_user_safe(&para_info, (void *)(uintptr_t)arg, sizeof(struct sched_ioctl_para_ack)) != 0) {
        return DRV_ERROR_COPY_USER_FAIL;
    }

    if (((para_info.msg == NULL) && (para_info.msg_len != 0)) ||
        ((para_info.msg != NULL) && ((para_info.msg_len == 0) || (para_info.msg_len > SCHED_MAX_EVENT_MSG_LEN)))) {
        sched_err("The member msg_len in para_info is out of range. (msg_len=%u; max=%d)\n",
                  para_info.msg_len, SCHED_MAX_EVENT_MSG_LEN);
        return DRV_ERROR_PARA_ERROR;
    }

    if ((para_info.msg != NULL) &&
        (copy_from_user_safe(msg, (void *)(uintptr_t)para_info.msg, para_info.msg_len) != 0)) {
        sched_err("Failed to invoke the copy_from_user_safe. (dev_id=%u; event_id=%u; msg_len=%u)\n",
            para_info.dev_id, para_info.event_id, para_info.msg_len);
        return DRV_ERROR_COPY_USER_FAIL;
    }

    return sched_ack_event(sched_ioctl_devid(devid, para_info.dev_id), current->tgid, para_info.event_id,
        para_info.subevent_id, msg, para_info.msg_len);
}

STATIC int32_t sched_fop_wait_event(u32 devid, unsigned long arg)
{
    struct sched_ioctl_para_wait para_info;
    struct sched_wait_input *para = &para_info.input;
    int32_t ret;

    if (copy_from_user_safe(para, (void *)(uintptr_t)arg, sizeof(struct sched_wait_input)) != 0) {
        return DRV_ERROR_COPY_USER_FAIL;
    }

    if (((para->msg == NULL) || (para->msg_len != SCHED_MAX_EVENT_MSG_LEN)) &&
        (para->timeout != SCHED_THREAD_SWAPOUT)) {
        sched_err("The msg is NULL or msg_len is invalid. (dev_id=%u; msg_len=%u; timeout=%d)\n",
            para->dev_id, para->msg_len, para->timeout);
        return DRV_ERROR_PARA_ERROR;
    }

    para_info.event.msg = para->msg;
    para_info.event.msg_len = para->msg_len;

    ret = sched_wait_event(sched_ioctl_devid(devid, para->dev_id), current->tgid, para->grp_id,
        para->thread_id, para->timeout, &para_info.event);
    if (unlikely(ret != 0)) {
        return ret;
    }

    if (copy_to_user_safe((void *)((uintptr_t)arg + sizeof(struct sched_wait_input)),
        &para_info.event, sizeof(struct sched_subscribed_event)) != 0) {
        return DRV_ERROR_COPY_USER_FAIL;
    }

    return 0;
}

STATIC int32_t sched_fop_get_event_trace(u32 devid, unsigned long arg)
{
    struct sched_ioctl_para_get_event_trace para;
    int32_t ret;

    if (copy_from_user_safe(&para, (void *)(uintptr_t)arg, sizeof(struct sched_ioctl_para_get_event_trace)) != 0) {
        return DRV_ERROR_COPY_USER_FAIL;
    }

    if (para.buff == NULL) {
        sched_err("Invalid para. (dev_id=%u)\n", para.dev_id);
        return DRV_ERROR_PARA_ERROR;
    }

    ret = sched_get_event_trace(sched_ioctl_devid(devid, para.dev_id), para.buff, para.buff_len, &para.data_len);
    if (unlikely(ret != 0)) {
        return ret;
    }

    if (copy_to_user_safe((void *)(uintptr_t)arg, &para, sizeof(struct sched_ioctl_para_get_event_trace)) != 0) {
        return DRV_ERROR_COPY_USER_FAIL;
    }

    return 0;
}

#ifdef CFG_FEATURE_TRACE_RECOED
STATIC int32_t sched_fop_trigger_sched_trace_record(u32 devid, unsigned long arg)
{
    struct sched_ioctl_para_trigger_sched_trace_record para;

    if (copy_from_user_safe(&para, (void *)(uintptr_t)arg,
        sizeof(struct sched_ioctl_para_trigger_sched_trace_record)) != 0) {
        return DRV_ERROR_COPY_USER_FAIL;
    }

    para.record_reason[SCHED_STR_MAX_LEN - 1] = '\0';
    para.key[SCHED_STR_MAX_LEN - 1] = '\0';

    return sched_trigger_sched_trace_record(sched_ioctl_devid(devid, para.dev_id), para.record_reason, para.key);
}
#endif

int32_t sched_publish_event_para_check(struct sched_published_event_info *event_info)
{
    if (event_info->gid >= SCHED_MAX_GRP_NUM) {
        sched_err("The variable gid is out of range. "
                  "(gid=%u; max=%d)\n", event_info->gid, SCHED_MAX_GRP_NUM);
        return DRV_ERROR_PARA_ERROR;
    }

    if (event_info->event_id >= SCHED_MAX_EVENT_TYPE_NUM) {
        sched_err("The variable event_id is out of range. (event_id=%u; max=%d)\n",
                  event_info->event_id, SCHED_MAX_EVENT_TYPE_NUM);
        return DRV_ERROR_PARA_ERROR;
    }

    if (((event_info->msg == NULL) && (event_info->msg_len != 0)) ||
        ((event_info->msg != NULL) && ((event_info->msg_len == 0) || (event_info->msg_len > EVENT_MAX_MSG_LEN)))) {
        sched_err("The variable msg_len is out of range. (msg_len=%u; max=%d)\n",
                  event_info->msg_len, SCHED_MAX_EVENT_MSG_LEN);
        return DRV_ERROR_PARA_ERROR;
    }

    return 0;
}

#if defined (ESCHED_HOST) && !defined (AOS_LLVM_BUILD) && !defined (EVENT_SCHED_UT) && !defined (EMU_ST)
STATIC int32_t sched_proc_mnt_ns_check(u32 chip_id, int pid)
{
    struct mnt_namespace *mnt_ns = NULL;

    mnt_ns = sched_get_proc_mnt_ns(chip_id, pid);
    if (mnt_ns != current->nsproxy->mnt_ns) {
        sched_err("Not in the same container. (chip_id=%u, pid=%d)", chip_id, pid);
        return DRV_ERROR_NOT_SUPPORT;
    }

    return 0;
}
#endif

sched_event_pre_proc sched_event_pre_proc_handle[SCHED_MAX_EVENT_TYPE_NUM] = {NULL};

#ifndef EMU_ST
int sched_register_event_pre_proc_handle(unsigned int event_id, sched_event_pre_proc handle)
{
    if (event_id >= SCHED_MAX_EVENT_TYPE_NUM) {
        sched_err("Invalid para. (event_id=%u; max=%u)\n", event_id, SCHED_MAX_EVENT_TYPE_NUM);
        return DRV_ERROR_PARA_ERROR;
    }

    if (sched_event_pre_proc_handle[event_id] != NULL) {
        sched_err("Handle has already been registered. (event_id=%u)\n", event_id);
        return DRV_ERROR_PARA_ERROR;
    }

    sched_event_pre_proc_handle[event_id] = handle;
    return 0;
}
EXPORT_SYMBOL(sched_register_event_pre_proc_handle);

void sched_unregister_event_pre_proc_handle(unsigned int event_id)
{
    if (event_id >= SCHED_MAX_EVENT_TYPE_NUM) {
        sched_err("Invalid para. (event_id=%u; max=%u)\n", event_id, SCHED_MAX_EVENT_TYPE_NUM);
        return;
    }

    sched_event_pre_proc_handle[event_id] = NULL;
    return;
}
EXPORT_SYMBOL(sched_unregister_event_pre_proc_handle);
#endif

int sched_submit_event_pre_proc(unsigned int dev_id, struct sched_published_event_info *event_info,
    struct sched_published_event_func *event_func)
{
    if (sched_event_pre_proc_handle[event_info->event_id] != NULL) {
#ifndef EMU_ST
        return sched_event_pre_proc_handle[event_info->event_id](dev_id, event_info, event_func);
#endif
    }
    return 0;
}

STATIC int32_t sched_submit_single_event(struct sched_numa_node *node, u32 event_src,
    struct sched_published_event_info *event_info, struct sched_published_event_func *event_func)
{
    return node->ops.sumbit_event(node->node_id, event_src, event_info, event_func);
}

STATIC int32_t sched_submit_multi_events(struct sched_numa_node *node, u32 event_src,
    struct sched_published_event_info *event_info, struct sched_published_event_func *event_func, unsigned long arg)
{
#ifndef AOS_LLVM_BUILD
    struct sched_ioctl_para_submit *usr_arg = (struct sched_ioctl_para_submit __user *)arg;
    u32 stamp = (u32)jiffies;
    unsigned int succ_num;
    int ret;

    for (succ_num = 0; succ_num < event_info->event_num; succ_num++) {
        ret = node->ops.sumbit_event(node->node_id, event_src, event_info, event_func);
        if (ret != 0) {
            break;
        }
        esched_try_cond_resched_by_time(&stamp, ESCHED_WAKEUP_TIMEINTERVAL);
    }

    if (event_info->event_num == succ_num) {
        return 0;
    } else if (succ_num != 0) {
        put_user(succ_num, &usr_arg->event_info.event_num);
        return 0;
    } else {
        return ret;
    }
#else
    sched_info("Not support for MDC yet.\n");
    return DRV_ERROR_NOT_SUPPORT;
#endif
}

STATIC int32_t sched_fop_submit_event(u32 devid, unsigned long arg)
{
    struct sched_numa_node *node = NULL;
    struct sched_ioctl_para_submit para;
    struct sched_published_event_info *event_info = NULL;
    struct sched_published_event_func event_func;
    int32_t ret;
    u32 chip_id;

    if (copy_from_user_safe(&para, (void *)(uintptr_t)arg, sizeof(struct sched_ioctl_para_submit)) != 0) {
        return DRV_ERROR_COPY_USER_FAIL;
    }

    chip_id = sched_ioctl_devid(devid, para.dev_id);

    node = sched_get_numa_node(chip_id);
    if (node == NULL) {
        sched_err("Invalid device. (devid=%u)\n", chip_id);
        return DRV_ERROR_INVALID_DEVICE;
    }

    event_info = &para.event_info;
    ret = sched_publish_event_para_check(event_info);
    if (ret != 0) {
        return ret;
    }

#ifndef AOS_LLVM_BUILD
    if (esched_dst_engine_is_local(event_info->dst_engine)) {
        ret = sched_get_tgid_by_vpid(event_info->pid, &event_info->pid);
        if (ret != 0) {
            return ret;
        }
    }
#endif

#if defined ESCHED_HOST && !defined (AOS_LLVM_BUILD) && !defined (EVENT_SCHED_UT) && !defined (EMU_ST)
    if ((event_info->dst_engine == CCPU_HOST) || (event_info->dst_engine == ACPU_HOST)) {
        ret = sched_proc_mnt_ns_check(chip_id, event_info->pid);
        if (ret != 0) {
            return ret;
        }
    }
#endif

    event_func.event_ack_func = NULL;
    event_func.event_finish_func = NULL;

    ret = sched_submit_event_pre_proc(chip_id, event_info, &event_func);
    if (ret != 0) {
        return ret;
    }

    if (event_info->event_num == 1) {
        return sched_submit_single_event(node, SCHED_PUBLISH_FORM_USER, event_info, &event_func);
    } else {
        return sched_submit_multi_events(node, SCHED_PUBLISH_FORM_USER, event_info, &event_func, arg);
    }
}

int32_t sched_submit_event(uint32_t chip_id, struct sched_published_event *event)
{
    struct sched_published_event_info *event_info = NULL;
    struct sched_numa_node *node = NULL;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
    struct timespec64 submit_event_time;
#else
    struct timeval submit_event_time;
#endif
    int32_t ret;

    if (event == NULL) {
        sched_err("The event is NULL. (chip_id=%u)\n", chip_id);
        return DRV_ERROR_PARA_ERROR;
    }

    event->event_info.publish_timestamp = sched_get_cur_timestamp();
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
    ktime_get_real_ts64(&submit_event_time);
    event->event_info.publish_timestamp_of_day =
        ((u64)submit_event_time.tv_sec * NSEC_PER_SEC) + (u64)submit_event_time.tv_nsec;
#else
    do_gettimeofday(&submit_event_time);
    event->event_info.publish_timestamp_of_day =
        ((u64)submit_event_time.tv_sec * USEC_PER_SEC) + (u64)submit_event_time.tv_usec;
#endif

    event_info = &event->event_info;
    event_info->tid = SCHED_INVALID_TID;
    event_info->dst_devid = SCHED_INVALID_DEVID;
    ret = sched_publish_event_para_check(event_info);
    if (ret != 0) {
        return ret;
    }

    node = esched_dev_get(chip_id);
    if (node == NULL) {
        sched_err("Invalid device. (chip_id=%u)\n", chip_id);
        return DRV_ERROR_INVALID_DEVICE;
    }

    ret = node->ops.sumbit_event(chip_id, SCHED_PUBLISH_FORM_KERNEL, event_info, &event->event_func);
    esched_dev_put(node);
    return ret;
}
EXPORT_SYMBOL(sched_submit_event);

int32_t sched_submit_event_to_thread(uint32_t chip_id, struct sched_published_event *event)
{
    struct sched_published_event_info *event_info = NULL;
    struct sched_numa_node *node = NULL;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
    struct timespec64 submit_event_time;
#else
    struct timeval submit_event_time;
#endif
    int32_t ret;

    if (event == NULL) {
        sched_err("The event is NULL. (chip_id=%u)\n", chip_id);
        return DRV_ERROR_PARA_ERROR;
    }

    if (event->event_info.tid == SCHED_INVALID_TID) {
        sched_err("User specific tid not support. (tid=%u)\n", SCHED_INVALID_TID);
        return DRV_ERROR_PARA_ERROR;
    }

    event->event_info.publish_timestamp = sched_get_cur_timestamp();
    event->event_info.dst_devid = SCHED_INVALID_DEVID;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
    ktime_get_real_ts64(&submit_event_time);
    event->event_info.publish_timestamp_of_day =
        ((u64)submit_event_time.tv_sec * NSEC_PER_SEC) + (u64)submit_event_time.tv_nsec;
#else
    do_gettimeofday(&submit_event_time);
    event->event_info.publish_timestamp_of_day =
        ((u64)submit_event_time.tv_sec * USEC_PER_SEC) + (u64)submit_event_time.tv_usec;
#endif

    event_info = &event->event_info;
    ret = sched_publish_event_para_check(event_info);
    if (ret != 0) {
        return ret;
    }

    node = esched_dev_get(chip_id);
    if (node == NULL) {
        sched_err("Invalid device. (chip_id=%u)\n", chip_id);
        return DRV_ERROR_INVALID_DEVICE;
    }

    ret = node->ops.sumbit_event(chip_id, SCHED_PUBLISH_FORM_KERNEL, event_info, &event->event_func);
    esched_dev_put(node);
    return ret;
}
EXPORT_SYMBOL(sched_submit_event_to_thread);

STATIC int32_t sched_fop_attach_proc_to_chip(u32 devid, unsigned long arg)
{
    struct sched_ioctl_para_attach para;

    if (copy_from_user_safe(&para, (void *)(uintptr_t)arg, sizeof(struct sched_ioctl_para_attach)) != 0) {
        return DRV_ERROR_COPY_USER_FAIL;
    }

    return sched_add_process(sched_ioctl_devid(devid, para.dev_id), current->tgid);
}

STATIC int32_t sched_fop_dettach_proc_from_chip(u32 devid, unsigned long arg)
{
    struct sched_ioctl_para_detach para;

    if (copy_from_user_safe(&para, (void *)(uintptr_t)arg, sizeof(struct sched_ioctl_para_detach)) != 0) {
        return DRV_ERROR_COPY_USER_FAIL;
    }

    return sched_del_process(sched_ioctl_devid(devid, para.dev_id), current->tgid);
}

STATIC int32_t (* sched_ioctl_handler[SCHED_CMD_MAX_NR])(u32 devid, unsigned long arg) = {
    [_IOC_NR(SCHED_SET_SCHED_CPU_ID)] = sched_fop_set_sched_cpu,
    [_IOC_NR(SCHED_PROC_ADD_GRP_ID)] = sched_fop_proc_add_grp,
    [_IOC_NR(SCHED_SET_EVENT_PRIORITY_ID)] = sched_fop_set_event_priority,
    [_IOC_NR(SCHED_SET_PROCESS_PRIORITY_ID)] = sched_fop_set_process_priority,
    [_IOC_NR(SCHED_THREAD_SUBSCRIBE_EVENT_ID)] = sched_fop_thread_subscribe_event,
    [_IOC_NR(SCHED_GET_EXACT_EVENT_ID)] = sched_fop_get_exact_event,
    [_IOC_NR(SCHED_ACK_EVENT_ID)] = sched_fop_ack_event,
    [_IOC_NR(SCHED_WAIT_EVENT_ID)] = sched_fop_wait_event,
    [_IOC_NR(SCHED_SUBMIT_EVENT_ID)] = sched_fop_submit_event,
    [_IOC_NR(SCHED_ATTACH_PROCESS_TO_CHIP_ID)] = sched_fop_attach_proc_to_chip,
    [_IOC_NR(SCHED_DETTACH_PROCESS_FROM_CHIP_ID)] = sched_fop_dettach_proc_from_chip,
    [_IOC_NR(SCHED_GRP_SET_EVENT_MAX_NUM)] = sched_fop_grp_set_max_event_num,
    [_IOC_NR(SCHED_QUERY_INFO)] = sched_fop_query_info,
    [_IOC_NR(SCHED_GET_NODE_EVENT_TRACE)] = sched_fop_get_event_trace,
#ifdef CFG_FEATURE_TRACE_RECOED
    [_IOC_NR(SCHED_TRIGGER_SCHED_TRACE_RECORD_VALUE)] = sched_fop_trigger_sched_trace_record,
#endif
};

void esched_register_ioctl_cmd_func(int nr, int32_t (*fn)(u32 devid, unsigned long arg))
{
    sched_ioctl_handler[nr] = fn;
}

STATIC int32_t sched_fop_open(struct inode *inode, struct file *filep)
{
#ifdef CFG_FEATURE_EXTERNAL_CDEV
    struct sched_numa_node *node = NULL;
    u32 devid = drv_davinci_get_device_id(filep);
    int ret = devdrv_manager_container_check_devid_in_container_ns(devid, current);
    if (ret != 0) {
        sched_err("Failed to check device in container. (devid=%u)\n", devid);
        return DRV_ERROR_INVALID_DEVICE;
    }

    node = esched_dev_get(devid);
    if (node == NULL) {
        sched_err("Invalid device. (devid=%u)\n", devid);
        return DRV_ERROR_INVALID_DEVICE;
    }

    esched_dev_put(node);
#endif

    filep->private_data = (void *)(uintptr_t)(current->tgid);

    return 0;
}

STATIC int32_t sched_fop_release(struct inode *inode, struct file *filep)
{
    int pid = (int)(uintptr_t)filep->private_data;

#ifdef CFG_FEATURE_EXTERNAL_CDEV
    u32 devid = drv_davinci_get_device_id(filep);
    struct sched_numa_node *node = esched_dev_get(devid);
    if (node == NULL) {
        return 0;
    }

    (void)sched_del_process(devid, pid);

    esched_dev_put(node);
#else
    u32 i;
    for (i = 0; i < SCHED_MAX_CHIP_NUM; i++) {
        (void)sched_del_process(i, pid);
    }
#endif

    module_feature_auto_uninit_task(pid);

    return 0;
}

STATIC long _sched_fop_ioctl(uint32_t devid, uint32_t cmd, unsigned long arg)
{
    if (_IOC_NR(cmd) >= SCHED_CMD_MAX_NR) {
        sched_err("The command is invalid, which is out of range. (cmd=%u)\n", _IOC_NR(cmd));
        return DRV_ERROR_INNER_ERR;
    }

    if (sched_ioctl_handler[_IOC_NR(cmd)] == NULL) {
        sched_err("The command is invalid. (cmd=%u)\n", _IOC_NR(cmd));
        return DRV_ERROR_INNER_ERR;
    }

    return sched_ioctl_handler[_IOC_NR(cmd)](devid, arg);
}

STATIC long sched_fop_ioctl(struct file *filep, uint32_t cmd, unsigned long arg)
{
#ifdef CFG_FEATURE_EXTERNAL_CDEV
    struct sched_numa_node *node = NULL;
#endif
    uint32_t devid = SCHED_MAX_CHIP_NUM;
    int pid = (int)(uintptr_t)filep->private_data;
    long ret;

    if (pid != current->tgid) { /* forked-process use father process fd, is not support */
        return DRV_ERROR_FILE_OPS;
    }

#ifdef CFG_FEATURE_EXTERNAL_CDEV
    devid = drv_davinci_get_device_id(filep);
    node = esched_dev_get(devid); /* Ensure that subsequent access to device objects is secure. */
    if (node == NULL) {
        /* After the VM goes offline, the user process(eg:tsd) may still be running. */
        return DRV_ERROR_NO_DEVICE;
    }
#endif

    ret = _sched_fop_ioctl(devid, cmd, arg);
#ifdef CFG_FEATURE_EXTERNAL_CDEV
    esched_dev_put(node);
#endif

    return ret;
}

STATIC const struct file_operations event_sched_fops = {
    .owner = THIS_MODULE,
    .open = sched_fop_open,
    .release = sched_fop_release,
#ifdef AOS_LLVM_BUILD
    .ioctl = sched_fop_ioctl,
#else
    .unlocked_ioctl = sched_fop_ioctl,
#endif
};

#if (!defined CFG_FEATURE_EXTERNAL_CDEV) && (!defined AOS_LLVM_BUILD)
STATIC char *sched_devnode(struct device *dev, umode_t *mode)
{
    return NULL;
}

STATIC int32_t sched_register_driver(void)
{
    int32_t ret;

    ret = alloc_chrdev_region(&char_dev.devno, 0, MINOR_DEV_COUNT, SCHED_CHAR_DEV_NAME);
    if (ret < 0) {
        sched_err("Failed to invoke the alloc_chrdev_region. (ret=%d)\n", ret);
        return DRV_ERROR_INNER_ERR;
    }

    cdev_init(&char_dev.cdev, &event_sched_fops);
    char_dev.cdev.owner = THIS_MODULE;

    if (cdev_add(&char_dev.cdev, char_dev.devno, 1) != 0) {
        sched_err("Failed to invoke the cdev_add. (devno=%u)\n", char_dev.devno);
        goto unregister_chrdev_region;
    }

    char_dev.dev_class = class_create(THIS_MODULE, SCHED_CHAR_DEV_NAME);
    if (IS_ERR(char_dev.dev_class)) {
        sched_err("Failed to invoke the class_create.\n");
        goto cdev_del;
    }

    char_dev.dev_class->devnode = sched_devnode;

    char_dev.device = device_create(char_dev.dev_class, NULL, char_dev.devno, NULL, SCHED_CHAR_DEV_NAME);
    if (IS_ERR(char_dev.device)) {
        sched_err("Failed to invoke the device_create.\n");
        goto class_destroy;
    }
    sched_debug("Called sched_register_cdev successfully.\n");

    return 0;

class_destroy:
    class_destroy(char_dev.dev_class);
cdev_del:
    cdev_del(&char_dev.cdev);
unregister_chrdev_region:
    unregister_chrdev_region(char_dev.devno, 1);
    return DRV_ERROR_INNER_ERR;
}

STATIC void sched_unregister_driver(void)
{
    device_destroy(char_dev.dev_class, char_dev.devno);
    class_destroy(char_dev.dev_class);
    cdev_del(&char_dev.cdev);
    unregister_chrdev_region(char_dev.devno, 1);
}
#endif

STATIC int32_t sched_register_cdev(void)
{
#ifdef AOS_LLVM_BUILD /* mdc */
    return register_driver(SCHED_CHAR_DEV_DRIVER_NAME, &event_sched_fops, DEVNODE_PERMISSION, NULL);
#elif defined(CFG_FEATURE_EXTERNAL_CDEV) /* dc */
    return drv_davinci_register_sub_module(DAVINCI_ESCHED_SUB_MODULE_NAME, &event_sched_fops);
#else /* helper host (Independent working mode) */
    return sched_register_driver();
#endif
}

STATIC void sched_unregister_cdev(void)
{
#ifdef AOS_LLVM_BUILD
    unregister_driver(SCHED_CHAR_DEV_DRIVER_NAME);
#elif defined(CFG_FEATURE_EXTERNAL_CDEV)
    (void)drv_ascend_unregister_sub_module(DAVINCI_ESCHED_SUB_MODULE_NAME);
#else
    sched_unregister_driver();
#endif
}

STATIC void sched_debugfs_init(void)
{
#ifdef AOS_LLVM_BUILD
    sched_procfs_init();
#elif defined(CFG_FEATURE_EXTERNAL_CDEV)
    struct device *dev = NULL;

    dev = davinci_intf_get_owner_device();
    if (dev == NULL) {
        sched_err("Failed to invoke devdrv_device_intf_get_owner_device.\n");
        return;
    }

    sched_sysfs_init(dev);
#else
    sched_sysfs_init(char_dev.device);
#endif
}

STATIC void sched_debugfs_uninit(void)
{
#ifdef AOS_LLVM_BUILD
    sched_procfs_uninit();
#elif defined(CFG_FEATURE_EXTERNAL_CDEV)
    struct device *dev = NULL;

    dev = davinci_intf_get_owner_device();
    if (dev == NULL) {
        sched_err("Failed to invoke devdrv_device_intf_get_owner_device.\n");
        return;
    }

    sched_sysfs_uninit(dev);
#else
    sched_sysfs_uninit(char_dev.device);
#endif
}

STATIC int32_t __init sched_init_module(void)
{
    int32_t ret;

    /* register cdev */
    ret = sched_register_cdev();
    if (ret != 0) {
        sched_err("Failed to invoke the sched_register_cdev. (ret=%d)\n", ret);
        return ret;
    }

    ret = esched_init();
    if (ret != 0) {
        sched_unregister_cdev();
        sched_err("Failed to invoke the esched_init. (ret=%d)\n", ret);
        return ret;
    }

    ret = module_feature_auto_init();
    if (ret != 0) {
        esched_uninit();
        sched_unregister_cdev();
        sched_err("Failed to invoke the esched_table_init. (ret=%d)\n", ret);
        return ret;
    }

    sched_debugfs_init();
    sched_debug("Called sched_init_module successfully.\n");
    return 0;
}

STATIC void __exit sched_exit_module(void)
{
    sched_debugfs_uninit();
    module_feature_auto_uninit();
    esched_uninit();
    sched_unregister_cdev();
    sched_debug("Called sched_exit_module successfully.\n");
}

module_init(sched_init_module);
module_exit(sched_exit_module);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("SCHEDULER DRIVER");
#if defined(CFG_FEATURE_HARDWARE_MIA) && defined(ESCHED_HOST)
MODULE_SOFTDEP("pre: ascend_trs_sec_eh_adapt");
#endif
