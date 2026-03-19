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

#ifndef PROF_UNIT_TEST

#include <linux/types.h>
#include <linux/delay.h>
#include <linux/kallsyms.h>
#include <linux/kthread.h>
#include <linux/module.h>
#ifdef AOS_LLVM_BUILD
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/atomic.h>
#include <linux/hrtimer.h>
#include <asm-generic/getorder.h>
#endif
#include "prof_drv_dev.h"
#include "prof_peripheral.h"
#include "kernel_version_adapt.h"
bool prof_agent_sample_valid(u32 channel_id)
{
    prof_sample_handle sample = NULL;
    char *cb_handler = NULL;
    cb_handler = prof_get_peri_cb_regs_handler(channel_id, (u32)PROF_PERI_SAMPLE);
    if (cb_handler != NULL) {
        sample = (prof_sample_handle)__symbol_get(cb_handler);
    }
    return (sample == NULL) ? false : true;
}


int prof_peripheral_callback_register(u32 channel_id, struct prof_sub_channel_info *sub_channel_info)
{
    char *cb_handler = NULL;

    if (sub_channel_info == NULL) {
        prof_err("Parameter [sub_channel_info] was invalid. (sub_channel_info=NULL)\n");
        return PROF_ERROR;
    }

    cb_handler = prof_get_peri_cb_regs_handler(channel_id, (u32)PROF_PERI_SAMPLE);
    if (cb_handler != NULL) {
        sub_channel_info->peri_channel.prof_sample_handle =
            (prof_sample_handle)__symbol_get(cb_handler);
        if (sub_channel_info->peri_channel.prof_sample_handle == NULL) {
            prof_err("Get symbol failed. (name=%s)\n", cb_handler);
        }
    }

    cb_handler = prof_get_peri_cb_regs_handler(channel_id, (u32)PROF_PERI_SAMPLE_USERDATA);
    if (cb_handler != NULL) {
        sub_channel_info->peri_channel.prof_sample_userdata_handle =
            (prof_sample_userdata_handle)__symbol_get(cb_handler);
    }

    cb_handler = prof_get_peri_cb_regs_handler(channel_id, (u32)PROF_PERI_SAMPLE_STOP);
    if (cb_handler != NULL) {
        sub_channel_info->peri_channel.prof_sample_stop_handle =
            (prof_sample_stop_handle)__symbol_get(cb_handler);
    }

    if (sub_channel_info->peri_channel.prof_sample_handle != NULL) {
        return PROF_OK;
    } else {
        prof_err("The channel was not supported. The function invoked was invalid. (channel_id=%u)\n",
            sub_channel_info->channel_id);
        return PROF_ERROR;
    }
}

int prof_agent_read(struct prof_proc_ctx *proc_ctx, struct prof_ioctl_para *para)
{
    prof_peripheral_buff_head_t *buff_head = NULL;
    struct prof_sub_channel_info *sub_channel_info = NULL;
    u32 channel_id = para->channel_id;
    u32 device_id = para->device_id;
    u32 vfid = para->vfid;
    u32 now_buf_len = 0;
    unsigned char *local_buf = NULL;
    u32 cnt = 0;
    u32 buf_size;
    u32 left_len;
    void *out_buf = NULL;
    int ret = PROF_ERROR;

    sub_channel_info = prof_get_sub_channel_info_from_ctx(proc_ctx, device_id, vfid, channel_id);
    if (sub_channel_info == NULL) {
        prof_err("The channel was disabled. (device_id=%u; vfid=%u; channel_id=%u)\n",
            device_id, vfid, channel_id);
        para->ret_val = PROF_STOPPED_ALREADY;
        return PROF_OK;
    }

    mutex_lock(&sub_channel_info->state_mutex);
    if (sub_channel_info->channel_state == (u32)PROF_CHANNEL_DISABLE ||
        sub_channel_info->channel_state == (u32)PROF_CHANNEL_IDLE) {
        mutex_unlock(&sub_channel_info->state_mutex);
        prof_warn("The channel with [PROF_PERIPHERAL_TYPE] type had been stopped. (device_id=%u; channel_id=%u)\n",
            device_id, channel_id);
        para->ret_val = PROF_STOPPED_ALREADY;
        return PROF_OK;
    }

    spin_lock_bh(&sub_channel_info->spinlock);
    if (sub_channel_info->proc_ctx != proc_ctx) {
        spin_unlock_bh(&sub_channel_info->spinlock);
        mutex_unlock(&sub_channel_info->state_mutex);
        prof_err("Failed to read other profile channel. (device_id=%u; channel_id=%u)\n",
            device_id, channel_id);
        return PROF_ERROR;
    }
    spin_unlock_bh(&sub_channel_info->spinlock);

    out_buf = para->out_buf;
    para->ret_val = 0;

    buff_head = &sub_channel_info->peri_channel.head;
    /* no new data to read */
    if (buff_head->head == buff_head->tail) {
        mutex_unlock(&sub_channel_info->state_mutex);
        return PROF_OK;
    }

    left_len = para->buf_len;
    while (buff_head->tail != buff_head->head) {
        now_buf_len = buff_head->data_len[buff_head->tail];

        if (now_buf_len == 0) {
            goto idle;
        }

        local_buf = sub_channel_info->vir_addr + ((u64)buff_head->one_buff_len * buff_head->tail);
        if (now_buf_len > left_len) {
            break;
        }
        ret = prof_send_data(out_buf, local_buf, now_buf_len, para);
        if (ret != PROF_OK) {
            prof_err("Failed to make profile send data. (ret=%d; mode=%u; now_buf_len=%u)\n",
                ret, para->use_mode, now_buf_len);
            break;
        }

        cnt++;
        left_len -= now_buf_len;
        out_buf += now_buf_len;
        buff_head->data_len[buff_head->tail] = 0;
idle:
        buff_head->tail++;
        if (buff_head->tail == PROF_PERIPHERAL_CACHE_NUM) {
            buff_head->tail = 0;
        }
    }

    buf_size = para->buf_len - left_len;

    if ((buff_head->tail == buff_head->head) &&
        (sub_channel_info->channel_state == (u32)PROF_CHANNEL_STOPPING)) {
        sub_channel_info->channel_state = (u32)PROF_CHANNEL_ENABLE;
        wake_up(&sub_channel_info->channel_wq);
    }

    sub_channel_info->prof_dfx.prof_read_count += cnt;
    sub_channel_info->prof_dfx.prof_out_total_size += buf_size;
    mutex_unlock(&sub_channel_info->state_mutex);
    para->ret_val = buf_size;

    return ret;
}

STATIC void prof_sample_stop(struct prof_sub_channel_info *sub_channel_info)
{
    struct prof_peri_para para = {0};
    int ret;

    if ((sub_channel_info != NULL) && (sub_channel_info->peri_channel.prof_sample_stop_handle != NULL)) {
        para.device_id = sub_channel_info->device_id;
        para.vfid = sub_channel_info->vfid;
        ret = sub_channel_info->peri_channel.prof_sample_stop_handle(para);
        if (ret != PROF_OK) {
            prof_err("Failed to stop profile sampling. (device_id=%u; ret=%d)\n", sub_channel_info->device_id, ret);
        }
    }
    return;
}

STATIC int prof_sample_start(struct prof_sub_channel_info *sub_channel_info)
{
    struct prof_peri_para para = {0};
    int ret;

    if (sub_channel_info->peri_channel.prof_sample_userdata_handle != NULL) {
        para.device_id = sub_channel_info->device_id;
        para.vfid = sub_channel_info->vfid;
        para.user_data = sub_channel_info->peri_channel.user_data;
        para.user_data_len = sub_channel_info->peri_channel.user_data_size;
        ret = sub_channel_info->peri_channel.prof_sample_userdata_handle(para);
        if (ret != PROF_OK) {
            prof_err("Failed to sample the profile channel data. (device_id=%u; vfid=%u; channel_id=%u;"
                " ret=%d; user_data_size=%u)\n",
                sub_channel_info->device_id, sub_channel_info->vfid, sub_channel_info->channel_id,
                ret, sub_channel_info->peri_channel.user_data_size);
            return ret;
        }
    }

    return PROF_OK;
}

STATIC int prof_sample(struct prof_sub_channel_info *sub_channel_info, int sample_flag)
{
    prof_peripheral_buff_head_t *buff_head = &sub_channel_info->peri_channel.head;
    struct prof_peri_para para = {0};

    if (sub_channel_info->peri_channel.prof_sample_handle != NULL) {
        para.device_id = sub_channel_info->device_id;
        para.vfid = sub_channel_info->vfid;
        para.buff = sub_channel_info->vir_addr + ((u64)buff_head->one_buff_len * buff_head->head);
        para.buff_len = buff_head->one_buff_len;
        para.sample_flag = sample_flag;
        para.target_pid = sub_channel_info->proc_ctx->collect_target_pid;

        return sub_channel_info->peri_channel.prof_sample_handle(para);
    }

    return 0;
}

static inline void prof_agent_update_full_cnt(prof_peripheral_buff_head_t *buff_head, int *full_cnt)
{
    int buff_cnt = *full_cnt;
    if (buff_head->tail == buff_head->head) {
        if (buff_cnt < PROF_PRINT_MAX) {
            prof_err("Profile sample lost all buffers and it needed to be read. (head=%u; tail=%u)\n",
                buff_head->head, buff_head->tail);
        }
        buff_cnt++;
    } else {
        buff_cnt = 0;
    }
    *full_cnt = buff_cnt;
}

STATIC bool prof_agent_read_invalid(int ret)
{
#ifdef CFG_SOC_PLATFORM_MDC_V51
    return ((ret == 0) || (ret == -EOPNOTSUPP)) ? true : false;
#else
    return (ret == 0);
#endif
}

STATIC int prof_agent_sample(void *arg)
{
    struct prof_sub_channel_info *sub_channel_info = (struct prof_sub_channel_info *)arg;
    prof_peripheral_buff_head_t *buff_head = NULL;
    int sample_flag = SAMPLE_WITH_HEADER, len = 0;
    int ret;
    u32 device_id = sub_channel_info->device_id, channel_id = sub_channel_info->channel_id;
    u32 vfid = sub_channel_info->vfid;
    static int err_print_cnt = 0;
    static int err_ring_buff_full_cnt = 0;

    ret = prof_sample_start(sub_channel_info);
    if (ret != PROF_OK) {
        sub_channel_info->peri_channel.sample_thread_flag = (int)PROF_SAMPLE_THREAD_EXIT;
        prof_err("Profile enable peri_drv failed!\n");
        return ret;
    }

    while (sub_channel_info->peri_channel.sample_thread_flag == (int)PROF_SAMPLE_THREAD_ENABLE) {
        len = 0;
        ret = down_interruptible(&sub_channel_info->peri_channel.sample_hrtimer.sync_timer_sema);
        if (ret == -EINTR) {
            prof_warn("Profile agent interrupted. (device_id=%u; vfid=%u; channle_id=%u, sub_ch_id=%u; ret=%d)\n",
                device_id, vfid, channel_id, sub_channel_info->sub_channel_id, ret);
            continue;
        }

        if (sub_channel_info->peri_channel.sample_thread_flag == (int)PROF_SAMPLE_THREAD_DISABLE) {
            prof_info("Profile sampling thread was over. Sampling was stopped."
                "(device_id=%u; vfid=%u; channle_id=%u, sub_ch_id=%u)\n",
                device_id, vfid, channel_id, sub_channel_info->sub_channel_id);
            break;
        }

        mutex_lock(&sub_channel_info->state_mutex);
        if (sub_channel_info->vir_addr == NULL) {
            mutex_unlock(&sub_channel_info->state_mutex);
            prof_err("Parameter [vir_addr] was [NULL]. (device_id=%u; vfid=%u; channle_id=%u, sub_ch_id=%u)\n",
                device_id, vfid, channel_id, sub_channel_info->sub_channel_id);
            break;
        }

        len = prof_sample(sub_channel_info, sample_flag);
        /* if read no data, continue */
        if (prof_agent_read_invalid(len)) {
            sample_flag = SAMPLE_ONLY_DATA;
            mutex_unlock(&sub_channel_info->state_mutex);
            continue;
        }

        buff_head = &sub_channel_info->peri_channel.head;
        if ((len < 0) || ((u32)len > buff_head->one_buff_len)) {
            if (err_print_cnt < PROF_PRINT_MAX) {
                prof_err("Failed to read profile sample length."
                    " (device_id=%u; vfid=%u; channel_id=%u, sub_ch_id=%u; len=%d; max=%u)\n",
                    device_id, vfid, channel_id, sub_channel_info->sub_channel_id, len, buff_head->one_buff_len);
                err_print_cnt++;
            }
            mutex_unlock(&sub_channel_info->state_mutex);
            continue;
        }
        err_print_cnt = 0;
        /* store data to ring_buffer head */
        buff_head->data_len[buff_head->head] = len;
        buff_head->head++;
        if (buff_head->head == PROF_PERIPHERAL_CACHE_NUM) {
            buff_head->head = 0;
        }
        /* if ring buffer is full, loss all ring buffer data */
        prof_agent_update_full_cnt(buff_head, &err_ring_buff_full_cnt);

        sub_channel_info->prof_dfx.prof_in_total_size += len;

        ret = prof_wake_up_poll_wq(sub_channel_info);
        if (ret != PROF_OK) {
            mutex_unlock(&sub_channel_info->state_mutex);
            prof_err("Failed to wake up the profile poll at the waiting queue. (ret=%d)\n", ret);
            break;
        }

        sample_flag = SAMPLE_ONLY_DATA;
        mutex_unlock(&sub_channel_info->state_mutex);
    }

    /* this func ptr is alternative implemented by peripheral module to by sampled. */
    prof_sample_stop(sub_channel_info);
    sub_channel_info->peri_channel.sample_thread_flag = (int)PROF_SAMPLE_THREAD_EXIT;
    prof_info("Profile sampling thread was over. (device_id=%u, vfid=%u; channle_id=%u, sub_ch_id=%u)\n",
        device_id, vfid, channel_id, sub_channel_info->sub_channel_id);

    return PROF_OK;
}

STATIC enum hrtimer_restart prof_agent_hrtimer_callback(struct hrtimer *timer)
{
    struct prof_sample_hrtimer *sample_hrtimer = NULL;

    sample_hrtimer = container_of(timer, struct prof_sample_hrtimer, timer);
    up(&sample_hrtimer->sync_timer_sema);
#ifndef AOS_LLVM_BUILD
    (void)hrtimer_forward(timer, hrtimer_cb_get_time(timer), sample_hrtimer->kt);
#else
    (void)hrtimer_forward_now(timer, sample_hrtimer->kt);
#endif
    return HRTIMER_RESTART;
}

STATIC void prof_agent_start_hrtimer(struct prof_sub_channel_info *sub_channel_info)
{
    sema_init(&sub_channel_info->peri_channel.sample_hrtimer.sync_timer_sema, 1);
    hrtimer_setup(&sub_channel_info->peri_channel.sample_hrtimer.timer,
        prof_agent_hrtimer_callback, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    sub_channel_info->peri_channel.sample_hrtimer.kt =
        ktime_set(0, sub_channel_info->peri_channel.sample_period * PROF_TIMES_MS_TO_NS);
    hrtimer_start(&sub_channel_info->peri_channel.sample_hrtimer.timer,
        sub_channel_info->peri_channel.sample_hrtimer.kt, HRTIMER_MODE_REL);
}

STATIC int prof_check_sample_thread_status(struct prof_sub_channel_info *sub_channel_info)
{
    int count = 0;

    while ((sub_channel_info->peri_channel.sample_thread_flag != (int)PROF_SAMPLE_THREAD_EXIT)
        && (count < PROF_CHECK_COUNT_MAX)) {
        msleep(1);
        count++;
    }

    if (sub_channel_info->peri_channel.sample_thread_flag != (int)PROF_SAMPLE_THREAD_EXIT) {
        prof_warn("Waiting for ending the sampling thread was timeout. (count=%d (ms))!\n", count);
        return PROF_TIMEOUT;
    }

    return PROF_OK;
}

STATIC int prof_agent_process(struct prof_ioctl_para *para, struct prof_sub_channel_info *sub_channel_info)
{
    int ret;

    if ((para == NULL) || (sub_channel_info == NULL)) {
        prof_err("Parameter [para] or [sub_channel_info] was NULL."
            " (para=%pK, sub_channel_info=%pK)\n", para, sub_channel_info);
        return PROF_ERROR;
    }

    ret = prof_check_sample_thread_status(sub_channel_info);
    if (ret != PROF_OK) {
        prof_err("Failed to check the state of sampling thread. (ret=%d)\n", ret);
        return PROF_ERROR;
    }

    sub_channel_info->peri_channel.sample_thread_flag = (int)PROF_SAMPLE_THREAD_ENABLE;
    sub_channel_info->peri_channel.sample_thread =
        kthread_create(prof_agent_sample, sub_channel_info, PROF_SAMPLE_PROC_NAME);
    if (IS_ERR(sub_channel_info->peri_channel.sample_thread)) {
        sub_channel_info->peri_channel.sample_thread_flag = (int)PROF_SAMPLE_THREAD_EXIT;
        prof_err("Failed to create the thread for peripheral channel to sample. (device_id=%u)\n",
            para->device_id);
        return PROF_ERROR;
    }

    (void)prof_init_affinity_cpuid_thread(para->device_id, sub_channel_info->peri_channel.sample_thread);

    sub_channel_info->channel_state = (u32)PROF_CHANNEL_ENABLE;
    prof_agent_start_hrtimer(sub_channel_info);
    (void)wake_up_process(sub_channel_info->peri_channel.sample_thread);
    para->ret_val = PROF_OK;
    prof_info("Profile started to sample. (device_id=%u; channel_id=%u)\n",
        para->device_id, sub_channel_info->channel_id);
    return PROF_OK;
}

STATIC int prof_check_agent_start_para_valid(struct prof_ioctl_para *para)
{
    int ret;

    ret = prof_check_device_state(para->device_id);
    if (ret != PROF_OK) {
        prof_err("Failed to check the state of device. (device_id=%u; ret=%d)\n", para->device_id, ret);
        return PROF_ERROR;
    }

    if ((para->sample_period < PROF_PERIOD_MIN) ||
        (para->sample_period > PROF_PERIOD_MAX)) {
        prof_err("Failed to sample at the period. (device_id=%u; sample_period=%ums)\n",
            para->device_id, para->sample_period);
        return PROF_ERROR;
    }

    if (para->user_data_size > PROF_USER_DATA_LEN) {
        prof_err("Parameter [user_data_size] was invalid. Failed to sample."
            " (device_id=%u; user_data_size=%u)\n", para->device_id, para->user_data_size);
        return PROF_ERROR;
    }

    return PROF_OK;
}

STATIC int prof_init_peri_sample_userdata(struct prof_sub_channel_info *sub_channel_info,
    const char *user_data, u32 user_data_size)
{
    /* this func ptr is alternative implemented by peripheral module to by sampled. */
    if (sub_channel_info->peri_channel.prof_sample_userdata_handle != NULL) {
        sub_channel_info->peri_channel.user_data_size = user_data_size;
        if (memcpy_s(sub_channel_info->peri_channel.user_data, PROF_USER_DATA_LEN, user_data, user_data_size) != EOK) {
            prof_err("Failed to invoke function [memcpy_s]. (user_data_size=%u)\n", user_data_size);
            return PROF_ERROR;
        }
    }

    return PROF_OK;
}

STATIC void prof_uninit_peri_sample_userdata(struct prof_sub_channel_info *sub_channel_info)
{
    if (sub_channel_info->peri_channel.prof_sample_userdata_handle != NULL) {
        if (memset_s(sub_channel_info->peri_channel.user_data, PROF_USER_DATA_LEN, 0, PROF_USER_DATA_LEN) != EOK) {
            prof_err("Failed to invoke function [memset_s].\n");
        }
    }
}

STATIC unsigned char *prof_peripheral_chan_alloc_mem(u32 device_id, u32 size)
{
#ifdef CFG_SOC_PLATFORM_MDC_V51
    return (unsigned char *)kzalloc_node(size, PROF__GFP_THISNODE, device_id);
#else
    prof_debug("Alloc peri sub channel memory. (device_id=%u; size=%u)", device_id, size);
    return (unsigned char *)prof_alloc_node_memory(device_id, size,
        PROF__GFP_THISNODE | __GFP_ZERO | GFP_HIGHUSER_MOVABLE, CTRL_NODE_MEM_TYPE);
#endif
}

void prof_peripheral_chan_free_mem(unsigned char *va, u32 size)
{
#ifdef CFG_SOC_PLATFORM_MDC_V51
    kfree(va);
#else
    prof_debug("Free peri sub channel memory. (size=%u)", size);
    free_pages((unsigned long)(uintptr_t)va, (u32)get_order(size));
#endif
}

STATIC int prof_init_sub_channel_info(struct prof_ioctl_para *para, struct prof_sub_channel_info *sub_channel_info)
{
    prof_peripheral_buff_head_t *buff_head = NULL;
    u32 device_id = para->device_id;
    u32 channel_id = para->channel_id;
    u32 buffer_cache_len;
    int ret;

    prof_init_dfx_info(sub_channel_info);

    sub_channel_info->peri_channel.prof_sample_handle = NULL;
    sub_channel_info->peri_channel.prof_sample_userdata_handle = NULL;
    sub_channel_info->peri_channel.prof_sample_stop_handle = NULL;

    ret = prof_peripheral_callback_register(channel_id, sub_channel_info);
    if (ret != PROF_OK) {
        prof_err("Failed to make profile peripheral use the callback register."
            " (device_id=%u; ret=%d; channel_id=%u)\n", device_id, ret, channel_id);
        return ret;
    }

    ret = prof_init_peri_sample_userdata(sub_channel_info, para->user_data, para->user_data_size);
    if (ret != PROF_OK) {
        return ret;
    }

    buffer_cache_len = PROF_PERIPHERAL_BUFFER_LEN * PROF_PERIPHERAL_CACHE_NUM;
    sub_channel_info->vir_addr = prof_peripheral_chan_alloc_mem(device_id, buffer_cache_len);
    if (sub_channel_info->vir_addr == NULL) {
        prof_err("Failed to invoke function [kzalloc] to apply for memory."
            " (device_id=%u; channel_id=%u; buffer_cache_len=%u)\n", device_id, channel_id, buffer_cache_len);
        return PROF_ERROR;
    }

    sub_channel_info->buf_len = buffer_cache_len;
    sub_channel_info->channel_id = channel_id;
    sub_channel_info->device_id = device_id;
    sub_channel_info->peri_channel.sample_period = para->sample_period;

    buff_head = &sub_channel_info->peri_channel.head;
    buff_head->buff_num = PROF_PERIPHERAL_CACHE_NUM;
    buff_head->one_buff_len = PROF_PERIPHERAL_BUFFER_LEN;
    buff_head->head = 0;
    buff_head->tail = 0;

    return PROF_OK;
}

int prof_agent_start(struct prof_proc_ctx *proc_ctx, struct prof_ioctl_para *para)
{
    struct prof_sub_channel_info *sub_channel_info = NULL;
    u32 device_id = para->device_id;
    u32 vfid = para->vfid;
    u32 channel_id = para->channel_id;
    int poll_fd_num;
    int ret;

    para->ret_val = PROF_ERROR;

    ret = prof_check_agent_start_para_valid(para);
    if (ret != PROF_OK) {
        return PROF_ERROR;
    }

    sub_channel_info = prof_alloc_sub_channel_resource(para, proc_ctx);
    if (sub_channel_info == NULL) {
        prof_err("Failed to allocate the subordinate channel. (device_id=%u; vfid=%u; channel_id=%u)\n",
            device_id, vfid, channel_id);
        return para->ret_val;
    }

    mutex_lock(&sub_channel_info->state_mutex);

    ret = prof_init_sub_channel_info(para, sub_channel_info);
    if (ret != PROF_OK) {
        prof_err("Failed to initialize the subordinate channels. (device_id=%u; chan_id=%u; ret=%d)\n",
            device_id, channel_id, ret);
        goto error;
    }

    ret = prof_agent_process(para, sub_channel_info);
    if (ret != 0) {
        prof_peripheral_chan_free_mem(sub_channel_info->vir_addr, sub_channel_info->buf_len);
        sub_channel_info->vir_addr = NULL;
        prof_err("Failed to operate the profile agent process. (device_id=%u; channel_id=%u; ret=%d)\n",
            device_id, channel_id, ret);
        goto error;
    }

    poll_fd_num = atomic_inc_return(&proc_ctx->poll.fd_num);
    mutex_unlock(&sub_channel_info->state_mutex);

    prof_info("Profile agent started successfully. (device_id=%u; channel_id=%u; poll_fd_num=%d)\n",
        device_id, channel_id, poll_fd_num);
    return PROF_OK;
error:
    prof_uninit_peri_sample_userdata(sub_channel_info);
    mutex_unlock(&sub_channel_info->state_mutex);
    prof_free_sub_channel_resource(sub_channel_info);

    return PROF_ERROR;
}

u32 prof_agent_get_data_len(struct prof_sub_channel_info *sub_channel_info)
{
    prof_peripheral_buff_head_t *buff_head = &sub_channel_info->peri_channel.head;
    u32 total_data_len = 0;
    u32 i;

    if (buff_head->head == buff_head->tail) {
        return total_data_len;
    }

    for (i = 0; i < buff_head->buff_num; i++) {
        total_data_len += buff_head->data_len[i];
    }

    return total_data_len;
}

int prof_agent_stop(struct prof_proc_ctx *proc_ctx, struct prof_ioctl_para *para)
{
    struct prof_sub_channel_info *sub_channel_info = NULL;
    u32 device_id = para->device_id;
    u32 vfid = para->vfid;
    u32 channel_id = para->channel_id;
    int ret;

    ret = prof_check_device_state(device_id);
    if (ret != PROF_OK) {
        prof_err("Failed to check the state of device. (device_id=%u; ret=%d)\n", device_id, ret);
        return PROF_ERROR;
    }

    if (prof_get_channel_type(channel_id) != PROF_PERIPHERAL_TYPE) {
        prof_err("Parameter [channel_id] was invalid."
            " (device_id=%u; channel_id=%u)\n", device_id, channel_id);
        return PROF_ERROR;
    }

    sub_channel_info = prof_get_sub_channel_info_from_ctx(proc_ctx, device_id, vfid, channel_id);
    if (sub_channel_info == NULL) {
        prof_warn("Profile agent was disabled. (device_id=%u; vfid=%u; channel_id=%u)\n",
            device_id, vfid, channel_id);
        para->ret_val = PROF_STOPPED_ALREADY;
        return PROF_OK;
    }
    para->ret_val = PROF_ERROR;

    mutex_lock(&sub_channel_info->state_mutex);
    ret = prof_stop_channel_status_check(sub_channel_info, proc_ctx);
    if (ret == PROF_STOPPED_ALREADY) {
        mutex_unlock(&sub_channel_info->state_mutex);
        para->ret_val = PROF_STOPPED_ALREADY;
        return PROF_OK;
    } else if (ret != PROF_OK) {
        mutex_unlock(&sub_channel_info->state_mutex);
        para->ret_val = ret;
        return PROF_ERROR;
    }

    (void)hrtimer_cancel(&sub_channel_info->peri_channel.sample_hrtimer.timer);
    if (sub_channel_info->peri_channel.sample_thread_flag == (int)PROF_SAMPLE_THREAD_ENABLE) {
        sub_channel_info->peri_channel.sample_thread_flag = (int)PROF_SAMPLE_THREAD_DISABLE;
        up(&sub_channel_info->peri_channel.sample_hrtimer.sync_timer_sema);
        sub_channel_info->channel_state = (u32)PROF_CHANNEL_STOPPING;
        mutex_unlock(&sub_channel_info->state_mutex);

        (void)prof_check_sample_thread_status(sub_channel_info);

        mutex_lock(&sub_channel_info->state_mutex);
        ret = prof_stop_wait_buff_read_over(sub_channel_info, PROF_PERIPHERAL_TYPE, proc_ctx);
        if (ret != PROF_OK) {
            mutex_unlock(&sub_channel_info->state_mutex);
            para->ret_val = PROF_STOPPED_ALREADY;
            return PROF_OK;
        }
    }

    /* except for input parameter check errors, other error return branches alse need to call it to uninit */
    prof_channel_info_free(sub_channel_info, PROF_PERIPHERAL_TYPE);

    prof_show_dfx_info(sub_channel_info);
    prof_dev_dfx_load(&sub_channel_info->prof_dfx, para);
    mutex_unlock(&sub_channel_info->state_mutex);
    prof_free_sub_channel_resource(sub_channel_info);

    prof_info("Profile agent channel stopped successfully. (channel_id=%u)\n", para->channel_id);

    para->ret_val = PROF_OK;
    return PROF_OK;
}

STATIC void prof_agent_sub_channel_stop(u32 device_id, u32 vfid,
    u32 channel_id, u32 sub_channel_id)
{
    struct prof_sub_channel_info *sub_channel_info = NULL;

    if (prof_get_channel_type(channel_id) != PROF_PERIPHERAL_TYPE) {
        prof_debug("The channel type was not [PROF_PERIPHERAL_TYPE]. No operation."
            " (device_id=%u; vfid=%u; channel_id=%u)\n", device_id, vfid, channel_id);
        return;
    }

    sub_channel_info = prof_get_sub_channel_info_from_index(device_id, vfid, channel_id, sub_channel_id);
    if (sub_channel_info == NULL) {
        return;
    }

    mutex_lock(&sub_channel_info->state_mutex);
    if (sub_channel_info->channel_state == (u32)PROF_CHANNEL_DISABLE ||
        sub_channel_info->channel_state == (u32)PROF_CHANNEL_IDLE) {
        mutex_unlock(&sub_channel_info->state_mutex);
        return;
    }

    (void)hrtimer_cancel(&sub_channel_info->peri_channel.sample_hrtimer.timer);

    if (sub_channel_info->peri_channel.sample_thread != NULL) {
        sub_channel_info->peri_channel.sample_thread_flag = (int)PROF_SAMPLE_THREAD_DISABLE;
        sub_channel_info->peri_channel.sample_thread = NULL;
        up(&sub_channel_info->peri_channel.sample_hrtimer.sync_timer_sema);
        mutex_unlock(&sub_channel_info->state_mutex);
        (void)prof_check_sample_thread_status(sub_channel_info);
        mutex_lock(&sub_channel_info->state_mutex);
    }

    if (sub_channel_info->vir_addr != NULL) {
        prof_peripheral_chan_free_mem(sub_channel_info->vir_addr, sub_channel_info->buf_len);
        sub_channel_info->vir_addr = NULL;
    }
    sub_channel_info->channel_state = (u32)PROF_CHANNEL_DISABLE;
    sub_channel_info->poll_flag = (int)POLL_INVALID;
    mutex_unlock(&sub_channel_info->state_mutex);
    prof_free_sub_channel_resource(sub_channel_info);
}

STATIC void prof_agent_channel_stop(u32 device_id, u32 vfid, u32 channel_id)
{
    int i;

    for (i = 0; i < PROF_SUB_CHANNEL_NUM_MAX; i++) {
        prof_agent_sub_channel_stop(device_id, vfid, channel_id, i);
    }
}

void prof_dev_agent_all_stop(u32 device_id)
{
    u32 j, k;

    for (j = 0; j < PROF_VFID_NUM_MAX; j++) {
        for (k = CHANNEL_HBM; k < CHANNEL_IDS_MAX; k++) {
            prof_agent_channel_stop(device_id, j, k);
        }
    }
}
#else
int prof_peripheral_ut_test(void)
{
    return 0;
}
module_init(prof_peripheral_ut_test);
#endif
