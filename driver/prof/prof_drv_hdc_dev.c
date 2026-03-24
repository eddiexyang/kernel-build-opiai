/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
 *
 * This program is free software; you could redistribute it and/or modify
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
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/kallsyms.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include "prof_drv_dev.h"
#include "prof_drv_hdc_dev.h"
#include "prof_hdc_msg_def.h"
#include "hdcdrv_interface.h"

#define PROF_POLL_WAIT_MAX 1000

volatile int prof_hdc_epoll_task_run_status[PROF_DEVICE_NUM_VALUE];
struct task_struct *prof_hdc_epoll_task[PROF_DEVICE_NUM_VALUE];
struct prof_hdc_epoll prof_hdc_epolls[PROF_DEVICE_NUM_VALUE];
struct prof_hdc_server prof_hdc_servers[PROF_DEVICE_NUM_VALUE];
struct prof_device_hdc_session *prof_hdc_sessions[PROF_DEVICE_NUM_VALUE] = {NULL};
struct prof_hdc_recycle_list g_prof_recycle_list;

STATIC void prof_hdc_session_close(struct prof_hdc_session *hdc_session);

STATIC inline struct prof_channel_hdc_session *prof_get_channel_hdc_session(u32 device_id, u32 vfid,
    u32 channel_id)
{
    return &prof_hdc_sessions[device_id]->prof_vf_hdc_session_info[vfid].prof_channel_hdc_session_info[channel_id];
}

STATIC inline struct prof_hdc_session *prof_get_channel_hdc_session_from_ctx(u32 device_id, u32 vfid,
    u32 channel_id, struct prof_hdc_session *hdc_session)
{
    struct prof_channel_hdc_session *channel_hdc_info = NULL;
    int i;

    channel_hdc_info = prof_get_channel_hdc_session(device_id, vfid, channel_id);
    mutex_lock(&channel_hdc_info->mutex);
    for (i = 0; i < PROF_SUB_CHANNEL_NUM_MAX; i++) {
        if (channel_hdc_info->prof_hdc_session_info[i] == hdc_session) {
            mutex_unlock(&channel_hdc_info->mutex);
            return channel_hdc_info->prof_hdc_session_info[i];
        }
    }
    mutex_unlock(&channel_hdc_info->mutex);

    return NULL;
}

STATIC void prof_add_channel_hdc_session(u32 device_id, u32 vfid, u32 channel_id,
    struct prof_hdc_session *hdc_session)
{
    struct prof_channel_hdc_session *channel_hdc_info = NULL;
    int i;

    channel_hdc_info = prof_get_channel_hdc_session(device_id, vfid, channel_id);
    mutex_lock(&channel_hdc_info->mutex);
    for (i = 0; i < PROF_SUB_CHANNEL_NUM_MAX; i++) {
        if (channel_hdc_info->prof_hdc_session_info[i] == NULL) {
            channel_hdc_info->prof_hdc_session_info[i] = hdc_session;
            mutex_unlock(&channel_hdc_info->mutex);
            return;
        }
    }
    mutex_unlock(&channel_hdc_info->mutex);
}

STATIC void prof_del_channel_hdc_session(u32 device_id, u32 vfid, u32 channel_id,
    struct prof_hdc_session *hdc_session)
{
    struct prof_channel_hdc_session *channel_hdc_info = NULL;
    int i;

    channel_hdc_info = prof_get_channel_hdc_session(device_id, vfid, channel_id);
    mutex_lock(&channel_hdc_info->mutex);
    for (i = 0; i < PROF_SUB_CHANNEL_NUM_MAX; i++) {
        if (channel_hdc_info->prof_hdc_session_info[i] == hdc_session) {
            channel_hdc_info->prof_hdc_session_info[i] = NULL;
            mutex_unlock(&channel_hdc_info->mutex);
            return;
        }
    }
    mutex_unlock(&channel_hdc_info->mutex);
}

void prof_free_hdc_sessions(u32 device_id)
{
    struct prof_channel_hdc_session *channel_hdc_info = NULL;
    int j, k;

    for (j = 0; j < PROF_VFID_NUM_MAX; j++) {
        for (k = 0; k < PROF_CHANNEL_NUM; k++) {
            channel_hdc_info = prof_get_channel_hdc_session(device_id, j, k);
            mutex_destroy(&channel_hdc_info->mutex);
        }
    }

    vfree((unsigned char *)prof_hdc_sessions[device_id]);
    prof_hdc_sessions[device_id] = NULL;
}

STATIC int prof_alloc_hdc_sessions(u32 device_id)
{
    struct prof_channel_hdc_session *channel_hdc_info = NULL;
    int j, k;

    prof_hdc_sessions[device_id] = (struct prof_device_hdc_session *)vzalloc(sizeof(struct prof_device_hdc_session));
    if (prof_hdc_sessions[device_id] == NULL) {
        prof_err("Failed to invoke function [vzalloc] to apply for memory.\n");
        return PROF_ERROR;
    }

    for (j = 0; j < PROF_VFID_NUM_MAX; j++) {
        for (k = 0; k < PROF_CHANNEL_NUM; k++) {
            channel_hdc_info = prof_get_channel_hdc_session(device_id, j, k);
            mutex_init(&channel_hdc_info->mutex);
        }
    }
    return PROF_OK;
}

STATIC int prof_check_read_ret_val(struct prof_ioctl_para prof_para, int buf_len)
{
    if (prof_para.ret_val == 0) {
        return PROF_NOT_READABLE_DATA;
    } else if (prof_para.ret_val > (buf_len - (int)sizeof(struct prof_hdc_msg))) {
        prof_err("Failed to make the profile to read data. (ret_val=%d; device_id=%u; channel_id=%u)\n",
            prof_para.ret_val, prof_para.device_id, prof_para.channel_id);
        return PROF_ERROR;
    }

    return PROF_OK;
}

#define PROF_HDC_SESSION_CHECK_COUNT 100
STATIC int prof_hdc_session_info_check(struct prof_hdc_session *hdc_session,
    u32 device_id, u32 vfid, u32 channel_id)
{
    struct prof_hdc_session *channel_hdc_session = NULL;
    int count = 0;

    channel_hdc_session = prof_get_channel_hdc_session_from_ctx(device_id, vfid, channel_id, hdc_session);
    while ((channel_hdc_session == NULL) && (count < PROF_HDC_SESSION_CHECK_COUNT)) {
        channel_hdc_session = prof_get_channel_hdc_session_from_ctx(device_id, vfid, channel_id, hdc_session);
        prof_warn("Profile read data. (device_id=%u; vfid=%u; channel_id=%u; check_count=%d)\n",
            device_id, vfid, channel_id, count);
        msleep(1);
        count++;
    }

    if (channel_hdc_session == hdc_session) {
        return PROF_OK;
    }

    prof_err("Failed to check information of HDC session."
        " (device_id=%u; vfid=%u; channel_id=%u; check_count=%d)\n",
        device_id, vfid, channel_id, count);
    return PROF_ERROR;
}

STATIC int prof_hdc_get_vfid(struct prof_hdc_session *hdc_session, int *vfid)
{
    int ret;

    ret = hdcdrv_kernel_get_session_vfid(hdc_session->session_fd, vfid);
    if (ret != 0) {
        prof_err("Failed to invoke function [hdcdrv_kernel_get_session_vfid]. (device_id=%d; session_fd=%d)\n",
            hdc_session->device_id, hdc_session->session_fd);
        return PROF_ERROR;
    }

    if ((*vfid < 0) || (*vfid >= PROF_VFID_NUM_MAX)) {
        prof_err("Parameter [vfid] was invalid. (device_id=%d; session_fd=%d; *vfid=%d)\n",
            hdc_session->device_id, hdc_session->session_fd, *vfid);
        return PROF_ERROR;
    }

    prof_debug("Profile read data. (devid=%d; session_fd=%d; *vfid=%d)\n",
        hdc_session->device_id, hdc_session->session_fd, *vfid);

    return PROF_OK;
}

static inline void *prof_poll_alloc_buff(int buf_len)
{
    void *buf = NULL;
    if ((buf_len > HDC_SEGMENT_LEN) || (buf_len <= 0)) {
        prof_err("Parameter [buf_len] was invalid. (buf_len=%d)\n", buf_len);
        return NULL;
    }
    buf = vmalloc(buf_len);
    if (buf == NULL) {
        prof_err("Failed to apply for the memory with the size of [buf_len]. (buf_len=%d)\n", buf_len);
    }
    return buf;
}

static inline bool prof_poll_continue_by_retval(int ret_val)
{
    if (ret_val == PROF_STOPPED_ALREADY) {
        msleep(1);
        return true;
    }
    if (ret_val != 1) {
        prof_err("Profile read an invalid value returned from [prof_poll]. (ret_val=%d)\n", ret_val);
        return true;
    }
    return false;
}

STATIC int prof_poll_thread(void *arg)
{
    struct prof_hdc_session *hdc_session = (struct prof_hdc_session *)arg;
    int buf_len = hdcdrv_get_segment();
    u32 vfid = hdc_session->vfid;
    struct prof_ioctl_para prof_para;
    struct prof_hdc_msg *head = NULL;
    struct prof_poll_box poll;
    int remote_status = 0;
    void *buf = NULL;
    int len, ret;

    buf = prof_poll_alloc_buff(buf_len);
    if (buf == NULL) {
        return PROF_ERROR;
    }

    head = (struct prof_hdc_msg *)buf;
    head->msg_type = PROF_HDC_DATA;
    head->ret_val = 0;

    while (hdc_session->poll_task_stop_flag == 0) {
        prof_para.out_buf = &poll;
        prof_para.poll_number = 1;
        prof_para.timeout = 1;
        prof_para.vfid = vfid;
        prof_para.use_mode = PROF_MODE_KERNEL;

        ret = prof_poll(&hdc_session->ctx, &prof_para);
        if (ret != 0) {
            prof_err("Profile read an invalid value returned from [prof_poll]. (ret=%d)\n", ret);
            continue;
        }

        if (prof_para.ret_val == 0) {
            if (hdc_session->ctx.poll.status != 0) {
                prof_event("Profile read data. (status=%u; poll_task_stop_flag=%d)\n",
                    hdc_session->ctx.poll.status, hdc_session->poll_task_stop_flag);
                break;
            }
            continue;
        }

        if (prof_poll_continue_by_retval(prof_para.ret_val) == true) {
            continue;
        }

        hdc_session->poll_task_run_status = (int)PROF_POLL_READABLE;

    read_again:
        prof_para.device_id = poll.device_id;
        prof_para.channel_id = poll.channel_id;
        prof_para.out_buf = buf + sizeof(struct prof_hdc_msg);
        prof_para.buf_len = buf_len - sizeof(struct prof_hdc_msg);
        prof_para.use_mode = PROF_MODE_KERNEL;
        ret = prof_drv_read(&hdc_session->ctx, &prof_para);
        if (ret != 0) {
            prof_err("Failed to drive the profile to read data. (ret=%d)\n", ret);
            goto poll_idle;
        }

        if (prof_para.ret_val < 0) {
            prof_warn("Profile [ret_val] was invalid. (ret_val=%d)\n", prof_para.ret_val);
            goto poll_idle;
        }

        /* if hdc remote has closed, continue */
        if (remote_status != 0) {
            goto poll_idle;
        }

        if (prof_check_read_ret_val(prof_para, buf_len) != PROF_OK) {
            goto poll_idle;
        }

        ret = prof_hdc_session_info_check(hdc_session, poll.device_id, vfid, poll.channel_id);
        if (ret != PROF_OK) {
            prof_err("Failed to check information of HDC session."
                " (device_id=%u; vfid=%u; channel_id=%u; thread_device=%d)\n",
                poll.device_id, vfid, poll.channel_id, hdc_session->device_id);
            goto poll_idle;
        }

        head->data_len = prof_para.ret_val;
        head->channel_id = poll.channel_id;

        len = prof_para.ret_val + sizeof(struct prof_hdc_msg);
        ret = hdcdrv_kernel_send_timeout(hdc_session->session_fd, hdc_session->session_id, buf, len,
            3000); /* 3000 ms */
        if (ret == HDCDRV_TX_REMOTE_CLOSE) {
            prof_err("The remote HDC driver had been closed. The timeout message was sent by HDC driver."
                " Profile had read the data. (session_fd=%d; len=%d; ret=%d)\n", hdc_session->session_fd, len, ret);
            remote_status = 1; /* remote has closed */
        } else if (ret != 0) {
            prof_err("The timeout message was sent by HDC driver. (session_fd=%d; len=%d; ret=%d)\n",
                hdc_session->session_fd, len, ret);
            msleep(100);
            goto poll_idle;
        }

        /* read_buf is full, read again */
        if (prof_para.buf_len == 0) {
            goto read_again;
        }

    poll_idle:
        wmb();
        hdc_session->poll_task_run_status = (int)PROF_POLL_IDLE;
    }

    vfree(buf);
    prof_info("Profile kernel polling thread was over."
        " (device_id=%d; session_fd=%d; vfid=%u; buf_len=%d)\n",
        hdc_session->device_id, hdc_session->session_fd, vfid, buf_len);

    /* hdc_session could only be used before this statement */
    hdc_session->poll_task_run_status = (int)PROF_POLL_DISABLE;

    return 0;
}

struct prof_hdc_session *prof_search_hdc_session(int session_fd)
{
    struct list_head *pos = NULL;
    struct list_head *n = NULL;
    struct prof_hdc_session *hdc_session = NULL;
    int i;

    for (i = 0; i < PROF_DEVICE_NUM_VALUE; i++) {
        if (prof_hdc_servers[i].valid == 0) {
            continue;
        }

        mutex_lock(&prof_hdc_servers[i].mutex);
        if (!list_empty_careful(&prof_hdc_servers[i].session_list)) {
            list_for_each_safe(pos, n, &prof_hdc_servers[i].session_list)
            {
                hdc_session = list_entry(pos, struct prof_hdc_session, list);
                if (hdc_session->session_fd == session_fd) {
                    mutex_unlock(&prof_hdc_servers[i].mutex);
                    return hdc_session;
                }
            }
        }
        mutex_unlock(&prof_hdc_servers[i].mutex);
    }

    return NULL;
}

STATIC int prof_ctrl_msg_get_channel_list(struct prof_hdc_session *hdc_session, struct prof_hdc_msg *head,
                                          int len)
{
    struct prof_ioctl_para prof_para = {0};

    if (len != (sizeof(struct prof_hdc_msg) + head->data_len)) {
        prof_err("Parameter [len] was invalid. (device_id=%d; session_fd=%d; len=%d)\n",
            hdc_session->device_id, hdc_session->session_fd, len);
        return PROF_ERROR;
    }

    prof_para.device_id = hdc_session->device_id;
    prof_para.vfid = hdc_session->vfid;
    prof_para.out_buf = head->data;
    prof_para.buf_len = head->data_len;
    prof_para.use_mode = PROF_MODE_KERNEL;

    return prof_get_channel_list(&prof_para);
}

STATIC int prof_ctrl_msg_start(struct prof_hdc_session *hdc_session, struct prof_hdc_msg *head, int len)
{
    struct prof_hdc_start_para *start_para = NULL;
    struct prof_ioctl_para prof_para = {0};
    enum prof_channel_type ch_type;
    int sec_ret;
    int ret;

    if (len != (sizeof(struct prof_hdc_msg) + sizeof(struct prof_hdc_start_para))) {
        prof_err("Parameter [len] was invalid. (device_id=%d; session_fd=%d; len=%d)\n",
            hdc_session->device_id, hdc_session->session_fd, len);
        return PROF_ERROR;
    }

    start_para = (struct prof_hdc_start_para *)head->data;

    prof_para.device_id = hdc_session->device_id;
    prof_para.vfid = hdc_session->vfid;
    prof_para.channel_id = head->channel_id;
    ch_type = prof_get_channel_type(head->channel_id);
    prof_para.buf_len = start_para->buf_len;
    prof_para.sample_period = start_para->sample_period;
    prof_para.user_data_size = start_para->user_data_size;

    ret = prof_check_channel_with_vfid(prof_para.device_id, prof_para.channel_id, prof_para.vfid);
    if (ret != PROF_OK) {
        prof_err("Failed to check the channel with vfid. (ret=%d; devid=%u; vfid=%u; channid=%u)\n",
            ret, prof_para.device_id, prof_para.vfid, prof_para.channel_id);
        return PROF_ERROR;
    }

    if (start_para->user_data_size > PROF_USER_DATA_LEN) {
        prof_err("The user data read by profile was oversize."
            " (device_id=%d; session_fd=%d; vfid=%d; user_data_size=%u)\n",
            hdc_session->device_id, hdc_session->session_fd, hdc_session->vfid, start_para->user_data_size);
        return PROF_ERROR;
    }

    if (start_para->user_data_size != 0) {
        sec_ret = memcpy_s(prof_para.user_data, PROF_USER_DATA_LEN, start_para->user_data, start_para->user_data_size);
        if (sec_ret != EOK) {
            prof_err("Failed to invoke function [memcpy_s]. (device_id=%d; session_fd=%d; user_data_size=%u)\n",
                hdc_session->device_id, hdc_session->session_fd, start_para->user_data_size);
            return PROF_ERROR;
        }
    }

    if (ch_type == PROF_TS_TYPE) {
        ret = prof_tscpu_start(&hdc_session->ctx, &prof_para);
    } else if (ch_type == PROF_PERIPHERAL_TYPE) {
        ret = prof_agent_start(&hdc_session->ctx, &prof_para);
    } else {
        prof_err("The channel type was not PROF_TS_TYPE."
            " (device_id=%d; session_fd=%d; ch_type=%d)\n",
            hdc_session->device_id, hdc_session->session_fd, ch_type);
        ret = PROF_ERROR;
    }

    if (ret == PROF_OK) {
        prof_debug("The device operation was success. (ret_val=%d)\n", prof_para.ret_val);
        ret = prof_para.ret_val;
        if (ret == PROF_OK) {
            prof_add_channel_hdc_session(hdc_session->device_id, hdc_session->vfid, prof_para.channel_id, hdc_session);
        }
    }

    return prof_para.ret_val;
}

STATIC int prof_ctrl_msg_stop(struct prof_hdc_session *hdc_session, struct prof_hdc_msg *head, int len)
{
    struct prof_ioctl_para prof_para = {0};
    int wait_num = 0;
    int ret;

    if (len != sizeof(struct prof_hdc_msg) + sizeof(prof_dev_dfx_info_t)) {
        prof_err("Parameter [len] was invalid. (device_id=%d; session_fd=%d; len=%d; correct_value=%lu)\n",
            hdc_session->device_id, hdc_session->session_fd, len,
            sizeof(struct prof_hdc_msg) + sizeof(prof_dev_dfx_info_t));
        return PROF_ERROR;
    }

    prof_para.device_id = hdc_session->device_id;
    prof_para.vfid = hdc_session->vfid;
    prof_para.channel_id = head->channel_id;
    prof_para.use_mode = PROF_MODE_KERNEL;
    prof_para.buf_len = head->data_len;
    prof_para.out_buf = head->data;

    ret = prof_drv_stop(&hdc_session->ctx, &prof_para);
    if (ret == PROF_OK) {
        while ((hdc_session->poll_task_run_status == (int)PROF_POLL_READABLE) && (wait_num < PROF_POLL_WAIT_MAX)) {
            msleep(1);
            wait_num++;
        }
        prof_info("Profile read the task state. (poll_task_run_status=%d; wait_num=%d; ret_val=%d)\n",
            hdc_session->poll_task_run_status, wait_num, prof_para.ret_val);

        prof_del_channel_hdc_session(prof_para.device_id, prof_para.vfid, prof_para.channel_id, hdc_session);
    }

    return prof_para.ret_val;
}

STATIC int prof_ctrl_msg_flush(struct prof_hdc_session *hdc_session, struct prof_hdc_msg *head, int len)
{
    struct prof_ioctl_para prof_para = {0};
    int wait_num = 0;
    int ret;

    if (len != (sizeof(struct prof_hdc_msg) + sizeof(u32))) {
        prof_err("Parameter [len] was invalid. (device_id=%d; session_fd=%d; len=%d)\n",
            hdc_session->device_id, hdc_session->session_fd, len);
        return PROF_ERROR;
    }

    prof_para.device_id = hdc_session->device_id;
    prof_para.vfid = hdc_session->vfid;
    prof_para.channel_id = head->channel_id;
    prof_para.buf_len = head->data_len;
    prof_para.out_buf = (u32 *)(head->data);
    prof_para.use_mode = PROF_MODE_KERNEL;

    ret = prof_data_len_flush(&hdc_session->ctx, &prof_para);
    if (ret != PROF_OK) {
        prof_err("Failed to flush the data length. (device_id=%d; vfid=%d; channel_id=%u; ret=%d)\n",
            hdc_session->device_id, hdc_session->vfid, head->channel_id, ret);
        return ret;
    }

    while ((hdc_session->poll_task_run_status == (int)PROF_POLL_READABLE) && (wait_num < PROF_POLL_WAIT_MAX)) {
        msleep(1);
        wait_num++;
    }

    prof_info("Profile read the channel data.  (device_id=%d; channel_id=%u; "
        " poll_task_run_status=%d; wait_num=%d; ret_val=%d; data_len=%u)\n", hdc_session->device_id,
        head->channel_id, hdc_session->poll_task_run_status, wait_num, ret, head->data_len);

    return ret;
}

STATIC void prof_ctrl_msg_proc(struct prof_hdc_session *hdc_session, unsigned char *buf, int len)
{
    struct prof_hdc_msg *head = (struct prof_hdc_msg *)buf;
    u32 retry_time = 0;
    int ret;

    if (len < (int)sizeof(struct prof_hdc_msg)) {
        prof_err("Parameter [len] was invalid. (device_id=%d; session_fd=%d; len=%d)\n",
            hdc_session->device_id, hdc_session->session_fd, len);
        return;
    }

    prof_info("Profile read the message type. (msg_type=%d)\n", head->msg_type);
    switch (head->msg_type) {
        case PROF_HDC_CMD_GET_CHANNEL:
            head->ret_val = prof_ctrl_msg_get_channel_list(hdc_session, head, len);
            break;
        case PROF_HDC_CMD_START:
            head->ret_val = prof_ctrl_msg_start(hdc_session, head, len);
            break;
        case PROF_HDC_CMD_STOP:
            head->ret_val = prof_ctrl_msg_stop(hdc_session, head, len);
            break;
        case PROF_HDC_CLOSE_SESSION:
            prof_hdc_session_close(hdc_session);
            return;
        case PROF_HDC_DATA_FLUSH:
            head->ret_val = prof_ctrl_msg_flush(hdc_session, head, len);
            break;
        default:
            head->ret_val = PROF_ERROR;
            prof_err("Failed to read the message type. (device_id=%d; session_fd=%d; msg_type=%d)\n",
                hdc_session->device_id, hdc_session->session_fd, head->msg_type);
            break;
    }

    do {
        ret = hdcdrv_kernel_send_timeout(hdc_session->session_fd,
            hdc_session->session_id, buf, len, 1000); /* The timeout is 1000 millisecond */
        if (ret == PROF_OK) {
            break;
        }
        if ((ret != HDCDRV_TX_TIMEOUT) || (retry_time >= 10)) { /* The max retry times is 10 */
            prof_err("Failed to make HDC driver send message. (device_id=%d; session_fd=%d; retry_time=%u; ret=%d)\n",
                hdc_session->device_id, hdc_session->session_fd, retry_time, ret);
            return;
        }
        retry_time++;
    } while (1);

    prof_info("Device send response to host successfully. (retry_time=%u; ret_val=%d)\n", retry_time, head->ret_val);
}

STATIC void prof_hdc_session_recv(struct prof_hdc_session *hdc_session)
{
    unsigned char *buf = NULL;
    int out_len, ret, len;

    ret = hdcdrv_kernel_recv_peek(hdc_session->session_fd, hdc_session->session_id, &len);
    if (ret != PROF_OK) {
        prof_err("Failed to invoke function [hdcdrv_kernel_recv_peek]. (device_id=%d; session_fd=%d; ret=%d)\n",
            hdc_session->device_id, hdc_session->session_fd, ret);
        return;
    }

    if ((len <= 0) || (len > HDC_SEGMENT_LEN) || (len > hdcdrv_get_segment())) {
        prof_err("The length read by profile was invalid. (device_id=%d; session_fd=%d; len=%d)\n",
            hdc_session->device_id, hdc_session->session_fd, len);
        return;
    }

    buf = (unsigned char *)vmalloc(len);
    if (buf == NULL) {
        prof_err("Failed to apply for memory with the size of [len, GFP_KERNEL]. (device_id=%d; session_fd=%d)\n",
            hdc_session->device_id, hdc_session->session_fd);
        return;
    }

    ret = hdcdrv_kernel_recv(hdc_session->session_fd, hdc_session->session_id, buf, len, &out_len);
    if (ret != PROF_OK) {
        prof_err("Failed to invoke function [hdcdrv_kernel_recv]. (device_id=%d; session_fd=%d; ret=%d)\n",
            hdc_session->device_id, hdc_session->session_fd, ret);
        goto out;
    }

    if (out_len > len) {
        prof_err("The [out_len] value was oversize. (device_id=%d; session_fd=%d; out_len=%d; len=%d)\n",
            hdc_session->device_id, hdc_session->session_fd, out_len, len);
        goto out;
    }

    prof_ctrl_msg_proc(hdc_session, buf, out_len);

out:
    vfree(buf);
    return;
}

STATIC void prof_wakeup_poll_thread(struct prof_hdc_session *hdc_session)
{
    hdc_session->ctx.poll.status = 1;
    wake_up(&hdc_session->ctx.poll.poll_wq);
}

STATIC void prof_hdc_session_free(u32 device_id, struct prof_hdc_session *hdc_session)
{
    int ret;

    ret = hdcdrv_kernel_epoll_ctl(prof_hdc_epolls[device_id].epfd, prof_hdc_epolls[device_id].magic_num,
        HDC_EPOLL_CTL_DEL, HDC_EPOLL_DATA_IN, hdc_session->session_fd, hdc_session->session_id,
        HDCDRV_EPOLL_CTL_PARA_NUM);
    if (ret != PROF_OK) {
        prof_err("Failed to invoke function [hdcdrv_kernel_epoll_ctl]. (device_id=%d; session_fd=%d; ret=%d)\n",
            hdc_session->device_id, hdc_session->session_fd, ret);
    }

    ret = hdcdrv_kernel_close(hdc_session->session_fd, hdc_session->session_id);
    if (ret != PROF_OK) {
        prof_err("Failed to invoke function [hdcdrv_kernel_close]. (device_id=%d; session_fd=%d; ret=%d)\n",
            hdc_session->device_id, hdc_session->session_fd, ret);
    }

    prof_info("Function [hdcdrv_kernel_close] invoked was success. (device_id=%d; session_fd=%d)\n",
        hdc_session->device_id, hdc_session->session_fd);
}

STATIC void prof_hdc_session_close(struct prof_hdc_session *hdc_session)
{
    struct prof_ioctl_para prof_para = {0};
    u32 device_id = hdc_session->device_id;
    u32 vfid = hdc_session->vfid;
    int wait_num = 0;
    int i;

    prof_para.device_id = device_id;
    prof_para.vfid = vfid;
    for (i = 0; i < PROF_CHANNEL_NUM; i++) {
        if (prof_get_channel_hdc_session_from_ctx(device_id, vfid, i, hdc_session) != hdc_session) {
            continue;
        }

        prof_para.channel_id = i;
        prof_para.use_mode = PROF_MODE_KERNEL;
        (void)prof_drv_stop(&hdc_session->ctx, &prof_para);
        prof_hdc_session_channel_free(device_id, vfid, i, &hdc_session->ctx);
        prof_del_channel_hdc_session(device_id, vfid, i, hdc_session);
    }

    if (!IS_ERR(hdc_session->poll_task)) {
        hdc_session->poll_task_stop_flag = 1;
        prof_wakeup_poll_thread(hdc_session);
        while ((hdc_session->poll_task_run_status != (int)PROF_POLL_DISABLE) &&
            (wait_num < PROF_HDC_CLOSE_WAIT_MAX_TIME)) {
                msleep(1);
                wait_num++;
        }
    }

    mutex_lock(&prof_hdc_servers[device_id].mutex);
    list_del(&hdc_session->list);
    mutex_unlock(&prof_hdc_servers[device_id].mutex);
    prof_hdc_session_free(device_id, hdc_session);
    if (hdc_session->poll_task_run_status != (int)PROF_POLL_DISABLE) {
        prof_warn("Failed to stop the poll thread. "
            " (device_id=%u; session_fd=%d; poll_task_run_status=%d; wait_num=%d)\n",
            device_id, hdc_session->session_fd, hdc_session->poll_task_run_status, wait_num);
        mutex_lock(&g_prof_recycle_list.mutex);
        list_add_tail(&hdc_session->list, &g_prof_recycle_list.session_list);
        mutex_unlock(&g_prof_recycle_list.mutex);
        return;
    }

    prof_ctx_uninit(&hdc_session->ctx);
    kfree(hdc_session);
}

STATIC void prof_hdc_session_proc(int session_fd, u32 event)
{
    struct prof_hdc_session *hdc_session = prof_search_hdc_session(session_fd);

    if (hdc_session == NULL) {
        prof_err("Failed to search session_fd. (session_fd=%d)\n", session_fd);
        return;
    }

    if ((event & HDC_EPOLL_DATA_IN) != 0) {
        prof_hdc_session_recv(hdc_session);
    }

    if ((event & HDCDRV_EPOLL_SESSION_CLOSE) != 0) {
        prof_hdc_session_close(hdc_session);
    }
}

STATIC int prof_hdc_ctx_init(struct prof_proc_ctx *ctx, int session_fd)
{
    int ret;

    ret = hdcdrv_kernel_get_session_peer_create_pid(session_fd, (int *)&ctx->collect_target_pid);
    if (ret != PROF_OK) {
        prof_err("Failed to get remote collect pid. (ret=%d)\n", ret);
        return ret;
    }

    ret = prof_ctx_init(ctx);
    if (ret != PROF_OK) {
        prof_err("Failed to initialize profile CTX. (ret=%d)\n", ret);
        return ret;
    }

    return ret;
}

STATIC void prof_hdc_session_accept(int device_id, int service_type)
{
    struct prof_hdc_session *hdc_session = NULL;
    int vfid = 0;
    int ret;

    if ((device_id < 0) || (device_id >= PROF_DEVICE_NUM_VALUE)) {
        prof_err("Parameter [device_id] was invalid. (device_id=%d)\n", device_id);
        return;
    }

    hdc_session = kzalloc(sizeof(struct prof_hdc_session), GFP_KERNEL | __GFP_ACCOUNT);
    if (hdc_session == NULL) {
        prof_err("Failed to invoke function [kzalloc] to apply for memory. (device_id=%d)\n", device_id);
        return;
    }

    ret = hdcdrv_kernel_accept(device_id, service_type,
        &hdc_session->session_fd, hdc_session->session_id);
    if (ret != PROF_OK) {
        prof_err("Failed to invoke function [hdcdrv_kernel_accept]. (device_id=%d; ret=%d)\n", device_id, ret);
        goto free_res;
    }
    prof_info("HDC driver kernel operation was success. (device_id=%d; session_fd=%d; service_type=%d)\n",
        device_id, hdc_session->session_fd, service_type);

    hdc_session->device_id = device_id;
    ret = prof_hdc_get_vfid(hdc_session, &vfid);
    if (ret != PROF_OK) {
        prof_err("Failed to make HDC get vfid. (device_id=%d; session_fd=%d; ret=%d)\n",
            hdc_session->device_id, hdc_session->session_fd, ret);
        goto session_close;
    }
    hdc_session->vfid = vfid;

    ret = hdcdrv_kernel_epoll_ctl(prof_hdc_epolls[device_id].epfd,
        prof_hdc_epolls[device_id].magic_num, HDC_EPOLL_CTL_ADD, HDC_EPOLL_DATA_IN | HDCDRV_EPOLL_SESSION_CLOSE,
        hdc_session->session_fd, hdc_session->session_id, HDCDRV_EPOLL_CTL_PARA_NUM);
    if (ret != PROF_OK) {
        prof_err("Failed to invoke function [hdcdrv_kernel_epoll_ctl]. (device_id=%d; ret=%d)\n", device_id, ret);
        goto session_close;
    }

    ret = prof_hdc_ctx_init(&hdc_session->ctx, hdc_session->session_fd);
    if (ret != PROF_OK) {
        prof_err("Failed to initialize profile CTX. (device_id=%d)\n", device_id);
        goto session_ctl_remove;
    }

    mutex_lock(&prof_hdc_servers[device_id].mutex);
    list_add_tail(&hdc_session->list, &prof_hdc_servers[device_id].session_list);
    mutex_unlock(&prof_hdc_servers[device_id].mutex);

    hdc_session->poll_task_stop_flag = 0;
    hdc_session->poll_task_run_status = (int)PROF_POLL_ENABLE;
    hdc_session->poll_task = kthread_create(prof_poll_thread, hdc_session, "prof_%d_%u",
        device_id, hdc_session->ctx.collect_target_pid);
    if (IS_ERR(hdc_session->poll_task)) {
        prof_err("Failed to start profile poll tasks.\n");
        prof_hdc_session_close(hdc_session);
        return;
    }
    (void)prof_init_affinity_cpuid_thread(device_id, hdc_session->poll_task);

    (void)wake_up_process(hdc_session->poll_task);
    return;

session_ctl_remove:
    (void)hdcdrv_kernel_epoll_ctl(prof_hdc_epolls[device_id].epfd, prof_hdc_epolls[device_id].magic_num,
        HDC_EPOLL_CTL_DEL, HDC_EPOLL_DATA_IN, hdc_session->session_fd, hdc_session->session_id,
        HDCDRV_EPOLL_CTL_PARA_NUM);
session_close:
    (void)hdcdrv_kernel_close(hdc_session->session_fd, hdc_session->session_id);

free_res:
    kfree(hdc_session);
}

#define PROF_HDC_SERVER_CREAT_WAIT_CNT_MAX 120
STATIC int prof_hdc_epoll_server_init(u32 device_id)
{
    int ret;

    if (prof_hdc_servers[device_id].valid == 1) {
        return PROF_OK;
    }

    prof_hdc_servers[device_id].device_id = device_id;
    prof_hdc_servers[device_id].server_type = HDC_SERVICE_TYPE_PROF;
    ret = hdcdrv_kernel_server_create(device_id, prof_hdc_servers[device_id].server_type);
    if (ret != PROF_OK) {
        return -EAGAIN;
    }

    ret = hdcdrv_kernel_epoll_alloc_fd(PROF_HDC_EVENT_NUM_MAX, &prof_hdc_epolls[device_id].epfd,
        &prof_hdc_epolls[device_id].magic_num);
    if (ret != PROF_OK) {
        prof_err("Failed to invoke function [hdcdrv_kernel_epoll_alloc_fd]. (ret=%d)\n", ret);
        goto free_server;
    }

    ret = hdcdrv_kernel_epoll_ctl(prof_hdc_epolls[device_id].epfd, prof_hdc_epolls[device_id].magic_num,
        HDCDRV_EPOLL_OP_ADD, HDC_EPOLL_CONN_IN, device_id,
        (char *)&prof_hdc_servers[device_id].server_type,
        HDCDRV_EPOLL_CTL_PARA_NUM);
    if (ret != PROF_OK) {
        prof_err("Failed to invoke function [hdcdrv_kernel_epoll_ctl]. (device_id=%u; ret=%d)\n",
            prof_hdc_servers[device_id].device_id, ret);
        goto free_epoll;
    }

    INIT_LIST_HEAD(&prof_hdc_servers[device_id].session_list);
    mutex_init(&prof_hdc_servers[device_id].mutex);
    prof_hdc_servers[device_id].valid = 1;

    return 0;
free_epoll:
    hdcdrv_kernel_epoll_free_fd(prof_hdc_epolls[device_id].epfd, prof_hdc_epolls[device_id].magic_num);
free_server:
    hdcdrv_kernel_server_destroy(prof_hdc_servers[device_id].device_id,
        prof_hdc_servers[device_id].server_type);
    return ret;
}

STATIC void prof_hdc_epoll_server_uninit(u32 device_id)
{
    struct prof_hdc_session *hdc_session = NULL;
    struct list_head *pos = NULL;
    struct list_head *n = NULL;
    int ret;

    if (prof_hdc_servers[device_id].valid == 0) {
        return;
    }

    prof_hdc_servers[device_id].valid = 0;
    mutex_lock(&prof_hdc_servers[device_id].mutex);
    if (!list_empty_careful(&prof_hdc_servers[device_id].session_list)) {
        list_for_each_safe(pos, n, &prof_hdc_servers[device_id].session_list) {
            hdc_session = list_entry(pos, struct prof_hdc_session, list);
            mutex_unlock(&prof_hdc_servers[device_id].mutex);
            prof_hdc_session_close(hdc_session);
            mutex_lock(&prof_hdc_servers[device_id].mutex);
        }
    }
    mutex_unlock(&prof_hdc_servers[device_id].mutex);

    ret = hdcdrv_kernel_epoll_ctl(prof_hdc_epolls[device_id].epfd, prof_hdc_epolls[device_id].magic_num,
        HDCDRV_EPOLL_OP_DEL, HDC_EPOLL_CONN_IN, prof_hdc_servers[device_id].device_id,
        (char *)&prof_hdc_servers[device_id].server_type, HDCDRV_EPOLL_CTL_PARA_NUM);
    if (ret != PROF_OK) {
        prof_err("Failed to invoke function [hdcdrv_kernel_epoll_ctl]. (device_id=%u; ret=%d)\n",
            prof_hdc_servers[device_id].device_id, ret);
    }

    ret = hdcdrv_kernel_server_destroy(prof_hdc_servers[device_id].device_id,
        prof_hdc_servers[device_id].server_type);
    if (ret != PROF_OK) {
        prof_err("Failed to invoke function [hdcdrv_kernel_server_destroy]. (device_id=%u; ret=%d)\n",
            prof_hdc_servers[device_id].device_id, ret);
    }
    mutex_destroy(&prof_hdc_servers[device_id].mutex);

    ret = hdcdrv_kernel_epoll_free_fd(prof_hdc_epolls[device_id].epfd, prof_hdc_epolls[device_id].magic_num);
    if (ret != PROF_OK) {
        prof_err("Failed to invoke function [hdcdrv_kernel_epoll_free_fd]. (epfd=%d; ret=%d)\n",
            prof_hdc_epolls[device_id].epfd, ret);
    }
}

STATIC int prof_hdc_epoll_thread(void *arg)
{
    int timeout = 3000;
    int ret, event_num;
    int i;
    int para1[PROF_HDC_EVENT_NUM];
    int para2[PROF_HDC_EVENT_NUM];
    u32 events[PROF_HDC_EVENT_NUM];
    u32 device_id = *(u32 *)arg;
    u32 retry_count = 0;

    if (device_id >= PROF_DEVICE_NUM_VALUE) {
        prof_err("Invalid device id. (dev_id=%u)\n", device_id);
        return -EINVAL;
    }

    prof_hdc_epoll_task_run_status[device_id] = 1;

    while (!kthread_should_stop()) {
        ret = prof_hdc_epoll_server_init(device_id);
        if ((ret == -EAGAIN) && (retry_count < PROF_HDC_SERVER_CREAT_WAIT_CNT_MAX)) {
            prof_warn("Failed to init, need retry (device_id=%u; retry_count=%u)\n", device_id, retry_count);
            retry_count++;
            msleep(1000); /* sleep 1000 milli second */
            continue;
        }

        if (ret != PROF_OK) {
            prof_hdc_epoll_task_run_status[device_id] = 0;
            prof_err("Failed to initialize the HDC epoll or server. (ret=%d; device_id=%u)\n", ret, device_id);
            return ret;
        }

        event_num = PROF_HDC_EVENT_NUM;
        ret = hdcdrv_kernel_epoll_wait(prof_hdc_epolls[device_id].epfd, prof_hdc_epolls[device_id].magic_num,
            timeout, &event_num, events, PROF_HDC_EVENT_NUM, para1,
            PROF_HDC_EVENT_NUM, para2, PROF_HDC_EVENT_NUM);
        if ((ret != PROF_OK) || (event_num > PROF_HDC_EVENT_NUM) || (event_num < 0)) {
            prof_err("Failed to invoke function [hdcdrv_kernel_epoll_wait] or the [event_num] was invalid."
                " (ret=%d; event_num=%d)\n", ret, event_num);
            continue;
        }
        if (event_num == 0) {
            continue;
        }

        for (i = 0; i < event_num; i++) {
            if (events[i] == HDC_EPOLL_CONN_IN) {
                prof_hdc_session_accept(para1[i], para2[i]);
            } else {
                prof_hdc_session_proc(para1[i], events[i]);
            }
        }
    }

    prof_hdc_epoll_server_uninit(device_id);
    prof_info("Profile HDC epoll thread was over. (device_id=%u)\n", device_id);
    prof_hdc_epoll_task_run_status[device_id] = 0;
    return 0;
}

STATIC void prof_recycle_guard_work(struct work_struct *p_work)
{
    struct list_head *pos = NULL;
    struct list_head *n = NULL;
    struct prof_hdc_session *hdc_session = NULL;
    mutex_lock(&g_prof_recycle_list.mutex);
    if (!list_empty_careful(&g_prof_recycle_list.session_list)) {
        list_for_each_safe(pos, n, &g_prof_recycle_list.session_list) {
            hdc_session = list_entry(pos, struct prof_hdc_session, list);
            if (hdc_session->poll_task_run_status == (int)PROF_POLL_DISABLE) {
                prof_info("Profile recycled guard work. (device_id=%d; session_fd=%d)\n",
                    hdc_session->device_id, hdc_session->session_fd);
                list_del(&hdc_session->list);
                prof_ctx_uninit(&hdc_session->ctx);
                kfree(hdc_session);
            }
        }
    }
    mutex_unlock(&g_prof_recycle_list.mutex);

    (void)schedule_delayed_work_on(0, &g_prof_recycle_list.guard_work,
        msecs_to_jiffies(PROF_HDC_RECYCLE_GUARD_INTERVAL));
}

int prof_hdc_init_each_device(u32 device_id)
{
    int epoll_wait_num = 0;
    int ret;

    ret = prof_alloc_hdc_sessions(device_id);
    if (ret != PROF_OK) {
        prof_err("Failed to allocate profile HDC sessions. (ret=%d)\n", ret);
        return ret;
    }

    prof_hdc_epoll_task[device_id] = kthread_create(prof_hdc_epoll_thread, &device_id,
        "prof_epoll_%u", device_id);
    if (IS_ERR(prof_hdc_epoll_task[device_id])) {
        prof_free_hdc_sessions(device_id);
        prof_err("Failed to start HDC epoll tasks.\n");
        return PROF_ERROR;
    }
    (void)prof_init_affinity_cpuid_thread(device_id, prof_hdc_epoll_task[device_id]);

    (void)wake_up_process(prof_hdc_epoll_task[device_id]);

    while ((prof_hdc_epoll_task_run_status[device_id] != 1) &&
        (epoll_wait_num < PROF_AGENT_UNINIT_EPOLL_MAX_WAIT_CNT)) {
        msleep(1000); /* sleep 1000 milli second */
        epoll_wait_num++;
    }

    if (prof_hdc_epoll_task_run_status[device_id] != 1) {
        prof_err("Wait HDC epoll tasks timeout. (device_id=%u)\n", device_id);
        (void)kthread_stop(prof_hdc_epoll_task[device_id]);
        prof_hdc_epoll_task[device_id] = NULL;
        prof_free_hdc_sessions(device_id);
        return PROF_ERROR;
    }

    prof_event("Profile hdc each device initialized successfully. (device_id=%u)\n", device_id);
    return 0;
}

void prof_hdc_uninit_each_device(u32 device_id)
{
    if ((!IS_ERR(prof_hdc_epoll_task[device_id])) &&
        (prof_hdc_epoll_task_run_status[device_id] == 1)) {
        (void)kthread_stop(prof_hdc_epoll_task[device_id]);
    }

    prof_hdc_epoll_task[device_id] = NULL;
    prof_free_hdc_sessions(device_id);

    prof_event("Profile hdc each device uninitialized successfully. (device_id=%u)\n", device_id);
}

STATIC void prof_hdc_guard_work_init(void)
{
    mutex_init(&g_prof_recycle_list.mutex);
    INIT_LIST_HEAD(&g_prof_recycle_list.session_list);
    INIT_DELAYED_WORK(&g_prof_recycle_list.guard_work, prof_recycle_guard_work);
    (void)schedule_delayed_work_on(0, &g_prof_recycle_list.guard_work,
        msecs_to_jiffies(PROF_HDC_RECYCLE_GUARD_INTERVAL));
}

STATIC void prof_hdc_guard_work_uninit(void)
{
    int work_wait_num = 0;

    while ((!list_empty(&g_prof_recycle_list.session_list)) &&
        (work_wait_num < PROF_AGENT_UNINIT_WORK_MAX_WAIT_CNT)) {
        msleep(PROF_AGENT_UNINIT_WORK_PER_WAIT_TIME);
        work_wait_num++;
    }
    (void)cancel_delayed_work_sync(&g_prof_recycle_list.guard_work);
}

int prof_hdc_agent_init(void)
{
    prof_hdc_guard_work_init();
    prof_event("Profile host agent initialized successfully.\n");
    return 0;
}

void prof_hdc_agent_uninit(void)
{
    prof_hdc_guard_work_uninit();
    prof_event("Profile host agent uninitialized successfully.\n");
}
#else
int prof_drv_hdc_dev_ut_test(void)
{
    return 0;
}
module_init(prof_drv_hdc_dev_ut_test);
#endif
