/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
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
 * Create: 2020-4-1
 */

#include "hi_osal.h"
#include <asm/current.h>

#include "hi_type.h"
#include "hi_math.h"
#include "hi_comm_video.h"
#include "pngd.h"
#include "hi_errno.h"
#include "hi_version.h"
#include "sys_drv.h"
#include "pngd_buf.h"
#include "vb_ext.h"
#include "hi_debug.h"
#include "mod_ext.h"
#include "sys_ext.h"
#include "pngd_ext.h"
#include "hi_buffer.h"
#include "valg_plat.h"
#include "hi_comm_video_adapt.h"
#include "mpi_pngd_adapt.h"
#include "securec.h"
#include "ascend_kernel_hal.h"
#include "media_mem_file.h"
#include "HiDvppPngdInternal.h"
#include "event_sched_inner.h"
#include "HiDvppSubmitEvent.h"
#include "dvpp_mem_check.h"
#include "vmngd.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif // end of #ifdef __cplusplus

#define SEC_TO_NSEC        1000000000ull
#define MAX_TIME_INTERVAL   2000000000ull

pngd_proc *g_pngd_proc = HI_NULL;
pngd_context *g_pngd_chn = HI_NULL;
osal_spinlock_t g_pngd_state_spin_lock;

hi_u32 PngdMaxChnNum = INTERNAL_PNGD_MAX_CHN_NUM;
const uint32_t MSEC_TO_USEC = 1000;
pngd_chn_vf_manager g_pngd_chn_vf_manager;

hi_s32 pngd_check_module_func(hi_payload_type type)
{
    switch (type) {
        case PT_PNG: {
            if ((cmpi_get_module_func_by_id(HI_ID_PNGD)) == NULL) {
                PNGD_ERR_TRACE("the drv_pngd.ko module(%d) not ready!\n", HI_ID_PNGD);
                return HI_ERR_PNGD_SYS_NOT_READY;
            }
            break;
        }
        default: {
            PNGD_ERR_TRACE("the payload type(%d) not support!\n", type);
            return HI_ERR_PNGD_NOT_SUPPORT;
        }
    }
    return HI_SUCCESS;
}

hi_s32 pngd_wait_get_image_callback(const hi_void *param)
{
    pngd_context *pngd_chn = HI_NULL;
    pngd_wait_image *wait_image = (pngd_wait_image *)param;

    pngd_chn = &g_pngd_chn[wait_image->chn_id];

    return ((pngd_chn->chn_state != PNGD_CREATED) ||
            (pngd_buf_busy_count(&pngd_chn->frame_info_buf) != 0) ||
            (pngd_chn->unique_id != wait_image->unique_id) ||
            (g_pngd_state != PNGD_STATE_STARTED));
}

hi_s32 pngd_wait_send_stream_callback(const hi_void *param)
{
    hi_s32 ret = 0;
    pngd_context *pngd_chn = HI_NULL;
    pngd_wait_sendstream *wait_info = (pngd_wait_sendstream *)param;

    pngd_chn = &g_pngd_chn[wait_info->chn_id];

    if (osal_down(&pngd_chn->pngd_sem)) {
        return HI_FALSE;
    }

    if (g_pngd_state != PNGD_STATE_STARTED) {
        osal_up(&pngd_chn->pngd_sem);
        return HI_TRUE;
    }

    if ((pngd_chn->chn_state != PNGD_CREATED) ||
        (pngd_chn->unique_id != wait_info->unique_id)) {
        osal_up(&pngd_chn->pngd_sem);
        return HI_TRUE;
    }

    ret = pngd_buf_is_free_empty(&pngd_chn->frame_info_buf);
    if (ret == HI_FALSE) {
        osal_up(&pngd_chn->pngd_sem);
        return HI_TRUE;
    }

    osal_up(&pngd_chn->pngd_sem);
    return HI_FALSE;
}

hi_void pngd_ctx_init(hi_s32 chn_id, pngd_context *pngd_chn)
{
    hi_s32 ret = 0;
    hi_s32 device_id = trans_pngd_chan_id_to_device_id(chn_id);
    hi_s32 device_chan_id = trans_pngd_chan_id_to_device_chan_id(chn_id);

    ret = memset_s(&g_pngd_proc[chn_id], sizeof(pngd_proc), 0, sizeof(pngd_proc));
    if (ret != 0) {
        PNGD_WARN_TRACE("pid %d usr chn %d device %d chn %d memset_s fail! ret = %d\n",
            current->tgid, pngd_chn->user_chn_id, device_id, device_chan_id, ret);
    }

    ret = memset_s(&pngd_chn->smmu, sizeof(smmu_resource), 0, sizeof(smmu_resource));
    if (ret != 0) {
        PNGD_WARN_TRACE("pid %d usr chn %d device %d chn %d memset_s fail! ret = %d\n",
            current->tgid, pngd_chn->user_chn_id, device_id, device_chan_id, ret);
    }

    pngd_buf_init(&pngd_chn->frame_info_buf, sizeof(pngd_image_info));

    pngd_chn->pid = current->tgid;
    pngd_chn->total_frames_in_buf = 0;
    pngd_chn->amount_frames = 0;
    // acl相关变量初始化
    pngd_chn->user_data = NULL;
    pngd_chn->is_acl = HI_FALSE;
    pngd_chn->aclmpi_mode = HI_FALSE;
    pngd_chn->is_stream_buf_full = HI_FALSE;

    pngd_chn->mpi_timer.data = 0;
    pngd_chn->mpi_timer.timer = HI_NULL;
    pngd_chn->mpi_timer.function = HI_NULL;
    pngd_chn->mpi_timer_pendding = HI_FALSE;

    return;
}

void pngd_send_done_event(hi_s32 chn_id, hi_u32 event)
{
    hi_s32 ret;
    pngd_context *pngd_chn = HI_NULL;
    struct sched_published_event pngd_event;
    struct DvppEventMsg event_msg;
    hi_s32 device_id = trans_pngd_chan_id_to_device_id(chn_id);
    hi_s32 device_chan_id = trans_pngd_chan_id_to_device_chan_id(chn_id);

    pngd_chn = &g_pngd_chn[chn_id];
    ret = memset_s(&pngd_event, sizeof(pngd_event), 0, sizeof(pngd_event));
    CHECK_DO_SOMETHING(ret != 0, PNGD_ERR_TRACE("memset_s failed! ret=%d\n", ret));

    pngd_event.event_info.pid = pngd_chn->pid; // 调用方的标识，进程Id
    pngd_event.event_info.gid = 0; // group id，当前为0
    // 填写dvpp事件
    pngd_event.event_info.event_id = pngd_chn->aclmpi_mode ? EVENT_DVPP_MPI_MSG : EVENT_DVPP_MSG;

    // 填写DVPP子事件
    event_msg.userData = pngd_chn->user_data;
    event_msg.chn_id = pngd_chn->user_chn_id; // 此处事件通知的通道id需要转换成用户态的通道id
    pngd_event.event_info.subevent_id = event;
    pngd_event.event_info.msg_len = sizeof(struct DvppEventMsg);
    pngd_event.event_info.msg = (char*)(&event_msg);
    PNGD_INFO_TRACE("[PNGD_ACL] device %d chn %d, user_chn_id %d, pid %d, Start sched_submit_event,"
        "event = %#x, user_data = %lu, \n", device_id, device_chan_id, pngd_chn->user_chn_id, pngd_chn->pid,
        (hi_u32)event, (hi_uintptr_t)pngd_chn->user_data);

     // 发送事件的接口
    ret = sched_submit_event(dvpp_calc_get_drv_required_dev_id(pngd_chn->ext_phy_dev_id, (hi_u32)device_id),
                             &pngd_event);
    if (ret != 0) {
        PNGD_ERR_TRACE("[PNGD_ACL] sched_submit_event fail. ret = %d", ret);
    }
}

hi_s32 pngd_set_acl(hi_s32 chn_id, pngd_acl_info *acl_info)
{
    pngd_context *pngd_chn = HI_NULL;
    if (acl_info == NULL) {
        PNGD_ERR_TRACE("acl_info is null ptr \n");
        return HI_ERR_PNGD_NULL_PTR;
    }

    pngd_chn = &g_pngd_chn[chn_id];

    if (osal_down(&pngd_chn->pngd_sem)) {
        return -ERESTARTSYS;
    }

    pngd_chn->is_acl = HI_TRUE;
    pngd_chn->is_stream_buf_full = HI_FALSE;
    pngd_chn->pid = current->tgid;
    pngd_chn->user_data = acl_info->user_data;
    pngd_chn->aclmpi_mode = acl_info->is_himpi;

    // 不在中断内,直接发事件,不设队列

    osal_up(&pngd_chn->pngd_sem);
    return HI_SUCCESS;
}

void pngd_create_modify_status(hi_s32 device_id, hi_s32 chn_id)
{
    unsigned long stream_flag = 0;
    unsigned long state_flag = 0;

    osal_spin_lock_irqsave(&(g_pngd_state_spin_lock), &(state_flag));
    if (((pngd_extern_func *)cmpi_get_module_func_by_id(HI_ID_PNGD))->pfn_pngd_add_chn != NULL) {
        ((pngd_extern_func *)cmpi_get_module_func_by_id(HI_ID_PNGD))->pfn_pngd_add_chn();
    }
    osal_spin_unlock_irqrestore(&(g_pngd_state_spin_lock), &(state_flag));

    osal_spin_lock_irqsave(&(g_pngd_chn[chn_id].stream_spin_lock), &(stream_flag));
    // 此变量只在创建通道和复位通道的时候才++，当重新创建通道或复位通道后，
    // 现在的通道已经不是之前的那个通道了，
    // unique_id++是为了防止之前那个通道里残余的数据在新通道继续使用
    g_pngd_chn[chn_id].unique_id++;
    g_pngd_chn[chn_id].chn_state = PNGD_CREATED;
    osal_spin_unlock_irqrestore(&(g_pngd_chn[chn_id].stream_spin_lock), &(stream_flag));
}

// 根据current->tgid查询vf id
hi_s32 pngd_query_and_set_vf_id(hi_s32 chn_id, hi_s32 device_id, hi_u32 *vf_id,
    pngd_context *pngd_chn, hi_bool is_host_mode)
{
    hi_s32 ret = HI_SUCCESS;
    hi_s32 device_chan_id = trans_pngd_chan_id_to_device_chan_id(chn_id);

    ret = dvpp_calc_get_vf_id(pngd_chn->logic_dev_id, &pngd_chn->ext_phy_dev_id, vf_id, is_host_mode);
    if (ret != HI_SUCCESS) {
        PNGD_ERR_TRACE("pid %u usr chn %d device %d chn %d dvpp_calc_get_vf_id failed\n",
            pngd_chn->pid, pngd_chn->user_chn_id, device_id, device_chan_id);
        return HI_ERR_PNGD_SYS_ERROR;
    }

    if (*vf_id >= MAX_VF_ID_NUM) {
        PNGD_ERR_TRACE("pid %u usr chn %d device %d chn %d vf_id %u >= %d is error\n",
            current->tgid, pngd_chn->user_chn_id, device_id, device_chan_id, *vf_id, MAX_VF_ID_NUM);
        return HI_ERR_PNGD_SYS_ERROR;
    }

    PNGD_INFO_TRACE("pid %u usr chn %d device %d chn %d vf_id = %u.\n",
        current->tgid, pngd_chn->user_chn_id, device_id, device_chan_id, *vf_id);

    osal_spin_lock(&g_pngd_chn_vf_manager.pngd_vf_lock[device_id][*vf_id]);
    // 判断该VF下的通道数是否已满
    if (g_pngd_chn_vf_manager.created_chn_num[device_id][*vf_id] >=
        g_pngd_chn_vf_manager.max_chn_num[device_id][*vf_id]) {
        osal_spin_unlock(&g_pngd_chn_vf_manager.pngd_vf_lock[device_id][*vf_id]);
        PNGD_ERR_TRACE("pid %u usr chn %d device %d chn %d vf_id %u has created chn num(%u) is full\n",
            current->tgid, pngd_chn->user_chn_id, device_id, device_chan_id, *vf_id,
            g_pngd_chn_vf_manager.created_chn_num[device_id][*vf_id]);
        return HI_ERR_PNGD_NOT_PERM;
    }
    g_pngd_chn_vf_manager.created_chn_num[device_id][*vf_id]++;
    osal_spin_unlock(&g_pngd_chn_vf_manager.pngd_vf_lock[device_id][*vf_id]);

    return HI_SUCCESS;
}
hi_s32 pngd_create(hi_s32 chn_id, pngd_create_info *create_info)
{
    hi_s32 i, ret;
    hi_u32 vf_id = 0;
    pngd_context *pngd_chn = &g_pngd_chn[chn_id];
    hi_s32 device_id = trans_pngd_chan_id_to_device_id(chn_id);
    hi_s32 device_chan_id = trans_pngd_chan_id_to_device_chan_id(chn_id);

    if (osal_down(&pngd_chn->pngd_sem)) {
        PNGD_ERR_TRACE("pid %u usr chn %d device %d chn %d semaphore down fail!\n",
            current->tgid, create_info->user_chan_id, device_id, device_chan_id);
        return -ERESTARTSYS;
    }

    // 检查此通道是否处于已销毁状态,只有在销毁的状态下才允许创建通道
    if (pngd_chn->chn_state != PNGD_DESTROYED) {
        osal_up(&pngd_chn->pngd_sem);
        return HI_ERR_PNGD_EXIST;
    }
    pngd_chn->user_chn_id = create_info->user_chan_id;
    pngd_chn->logic_dev_id = create_info->logic_dev_id;
    ret = pngd_query_and_set_vf_id(chn_id, device_id, &vf_id, pngd_chn, create_info->is_acl);
    if (ret != HI_SUCCESS) {
        osal_up(&pngd_chn->pngd_sem);
        return ret;
    }

    ret = media_mem_file_init(&pngd_chn->mfile, current->tgid, chn_id, HI_ID_PNGD);
    if (ret != HI_SUCCESS) {
        osal_spin_lock(&g_pngd_chn_vf_manager.pngd_vf_lock[device_id][vf_id]);
        g_pngd_chn_vf_manager.created_chn_num[device_id][vf_id]--;
        osal_spin_unlock(&g_pngd_chn_vf_manager.pngd_vf_lock[device_id][vf_id]);

        osal_up(&pngd_chn->pngd_sem);
        PNGD_ERR_TRACE("pid %u usr chn %d device %d chn %d init memfile fail\n",
            current->tgid, pngd_chn->user_chn_id, device_id, device_chan_id);
        return HI_ERR_PNGD_NO_MEM;
    }
    pngd_chn->attr = create_info->attr;

    pngd_ctx_init(chn_id, pngd_chn);
    for (i = 0; i < MAX_PNGD_FRAME_NUM; i++) {
        if (pngd_buf_alloc_free_node(&pngd_chn->frame_info_buf) == NULL) {
            pngd_buf_release(&pngd_chn->frame_info_buf);
            media_mem_file_uninit(&pngd_chn->mfile);

            osal_spin_lock(&g_pngd_chn_vf_manager.pngd_vf_lock[device_id][vf_id]);
            g_pngd_chn_vf_manager.created_chn_num[device_id][vf_id]--;
            osal_spin_unlock(&g_pngd_chn_vf_manager.pngd_vf_lock[device_id][vf_id]);

            osal_up(&pngd_chn->pngd_sem);
            PNGD_ERR_TRACE("pid %u usr chn %d device %d chn %d alloc_free_node err!\n",
                pngd_chn->pid, pngd_chn->user_chn_id, device_id, device_chan_id);
            return HI_ERR_PNGD_NO_MEM;
        }
    }

    pngd_create_modify_status(device_id, chn_id);
    pngd_chn->vf_id = vf_id;

    PNGD_INFO_TRACE("pid %u device %d chn %d user_chn_id %d, create end, kernel_mem:%lu",
        pngd_chn->pid, device_id, device_chan_id, pngd_chn->user_chn_id, pngd_chn->mfile.total_mem_kernel);

    osal_up(&pngd_chn->pngd_sem);
    return HI_SUCCESS;
}

void pngd_sync_decode_count(void)
{
    static hi_u64 send_stream_cur_time = 0;
    static hi_u64 send_stream_pre_time = 0;

    // 计算发送码流的bit率，以及发送的帧率
    if (ckfn_sys_get_sched_clock()) {
        send_stream_cur_time = call_sys_get_sched_clock();
        if ((send_stream_cur_time - send_stream_pre_time) >= SEC_TO_NSEC) {
            hi_u32 index;
            for (index = 0; index < PngdMaxChnNum; index++) {
                // 码率
                g_pngd_proc[index].real_stream_rate = g_pngd_chn[index].amount_stream;
                g_pngd_chn[index].amount_stream = 0;
                // 帧率
                g_pngd_proc[index].real_send_frame_rate = g_pngd_chn[index].amount_send_frames;
                g_pngd_chn[index].amount_send_frames = 0;
            }

            if ((send_stream_cur_time - send_stream_pre_time) > MAX_TIME_INTERVAL) {
                send_stream_pre_time = send_stream_cur_time;
            } else {
                send_stream_pre_time = send_stream_pre_time + SEC_TO_NSEC;
            }
        }
    } else {
        PNGD_ERR_TRACE("check function sys_get_sched_clock fail\n");
    }
}

hi_s32 pngd_send_stream_process(hi_s32 chn_id, hi_img_stream *stream, hi_pic_info png_pic_info, hi_u32 pid)
{
    unsigned long spin_flag = 0;
    hi_s32 device_id = trans_pngd_chan_id_to_device_id(chn_id);
    hi_s32 device_chan_id = trans_pngd_chan_id_to_device_chan_id(chn_id);
    pngd_context *pngd_chn = HI_NULL;
    pngd_image_info *img_info = HI_NULL;

    pngd_chn = &g_pngd_chn[chn_id];
    pngd_chn->amount_stream += stream->len;
    pngd_chn->amount_send_frames++;
    pngd_sync_decode_count();

    osal_spin_lock_irqsave(&(g_pngd_chn[chn_id].stream_spin_lock), &(spin_flag));

    img_info = (pngd_image_info *)pngd_buf_get_free(&pngd_chn->frame_info_buf);
    if (img_info == HI_NULL) {
        osal_spin_unlock_irqrestore(&(g_pngd_chn[chn_id].stream_spin_lock), &(spin_flag));
        PNGD_WARN_TRACE("pid %u usr chn %d device %d chn %d get free node fail!\n",
            pngd_chn->pid, pngd_chn->user_chn_id, device_id, device_chan_id);
        return HI_ERR_PNGD_BUF_FULL;
    }

    img_info->png_pic_info = png_pic_info;
    img_info->stream = (*stream);

    pngd_buf_put_busy(&pngd_chn->frame_info_buf, img_info);
    g_pngd_proc[chn_id].mpi_send_strm_times++;
    osal_spin_unlock_irqrestore(&(g_pngd_chn[chn_id].stream_spin_lock), &(spin_flag));

    return HI_SUCCESS;
}

hi_s32 pngd_check_send_stream(hi_s32 chn_id, hi_img_stream *stream, hi_pic_info png_pic_info, hi_s32 milli_sec)
{
    pngd_context *pngd_chn = HI_NULL;
    pngd_chn = &g_pngd_chn[chn_id];
    hi_s32 device_id = trans_pngd_chan_id_to_device_id(chn_id);
    hi_s32 device_chan_id = trans_pngd_chan_id_to_device_chan_id(chn_id);

    if (pngd_chn->chn_state != PNGD_CREATED) {
        PNGD_ERR_TRACE("pid %u usr chn %d device %d chn %d not create!\n",
            pngd_chn->pid, pngd_chn->user_chn_id, device_id, device_chan_id);
        return HI_ERR_PNGD_UNEXIST;
    }

    if (milli_sec < -1) {
        PNGD_ERR_TRACE("pid %u usr chn %d device %d chn %d milli_sec %d is illegal!\n",
            pngd_chn->pid, pngd_chn->user_chn_id, device_id, device_chan_id, milli_sec);
        return HI_ERR_PNGD_ILLEGAL_PARAM;
    }

    // 检查用户态指针指向的空间是否可用
    if ((stream->addr == HI_NULL) || (stream->len == 0) ||
        (osal_access_ok(OSAL_VERIFY_READ, stream->addr, stream->len) == 0)) {
        PNGD_ERR_TRACE("pid %u usr chn %d device %d chn %d buff can't access! len:0x%u\n",
            pngd_chn->pid, pngd_chn->user_chn_id, device_id, device_chan_id, stream->len);
        return HI_ERR_PNGD_ILLEGAL_PARAM;
    }

    if (png_pic_info.picture_buffer_size == 0) {
        PNGD_ERR_TRACE("pid %u usr chn %d device %d chn %d picture_buffer_size can not be 0\n",
            pngd_chn->pid, pngd_chn->user_chn_id, device_id, device_chan_id);
        return HI_ERR_PNGD_ILLEGAL_PARAM;
    }

    if (png_pic_info.picture_address == HI_NULL) {
        PNGD_ERR_TRACE("pid %u usr chn %d device %d chn %d picture_address is null\n",
            pngd_chn->pid, pngd_chn->user_chn_id, device_id, device_chan_id);
        return HI_ERR_PNGD_NULL_PTR;
    }

    pngd_chn->stream_milli_sec = milli_sec;

    return HI_SUCCESS;
}

static hi_s32 pngd_check_mem(hi_s32 chn_id, hi_img_stream *stream, hi_pic_info png_pic_info)
{
    hi_s32 ret = HI_SUCCESS;
    pngd_context *pngd_chn = &g_pngd_chn[chn_id];
    hi_u32 device_id = (hi_u32)trans_pngd_chan_id_to_device_id(chn_id);
    hi_u32 device_chan_id = (hi_u32)trans_pngd_chan_id_to_device_chan_id(chn_id);

    ret = dvpp_check_mem(device_id, (hi_u64)stream->addr, (hi_u64)stream->len);
    if (ret != HI_SUCCESS) {
        PNGD_ERR_TRACE("pid %u usr chn %d device %u chn %u stream input address check failed!\n",
            pngd_chn->pid, pngd_chn->user_chn_id, device_id, device_chan_id);
        return HI_ERR_PNGD_BAD_ADDR;
    }
    ret = dvpp_check_mem(device_id, (hi_u64)png_pic_info.picture_address, (hi_u64)png_pic_info.picture_buffer_size);
    if (ret != HI_SUCCESS) {
        PNGD_ERR_TRACE("pid %u usr chn %d device %u chn %u output address check failed!\n",
            pngd_chn->pid, pngd_chn->user_chn_id, device_id, device_chan_id);
        return HI_ERR_PNGD_BAD_ADDR;
    }
    return HI_SUCCESS;
}

hi_s32 pngd_wait_event_interruptible(hi_s32 chn_id, hi_img_stream *stream, hi_s32 unid)
{
    pngd_context *pngd_chn = HI_NULL;
    pngd_wait_sendstream wait_stream_info;
    hi_s32 device_id = trans_pngd_chan_id_to_device_id(chn_id);
    hi_s32 device_chan_id = trans_pngd_chan_id_to_device_chan_id(chn_id);
    pngd_chn = &g_pngd_chn[chn_id];

    wait_stream_info.chn_id = chn_id;
    wait_stream_info.unique_id = unid;
    wait_stream_info.stream_len = stream->len;
    if (osal_wait_event_interruptible(&pngd_chn->wait_stream,
        pngd_wait_send_stream_callback, (hi_void *)&wait_stream_info)) {
        /* wait_event_interruptible可能被ctrl+c打断 */
        PNGD_ERR_TRACE("pid %u usr chn %d device %d chn %d is stop with interruptible!\n",
            pngd_chn->pid, pngd_chn->user_chn_id, device_id, device_chan_id);
        return -ERESTARTSYS;
    }

    return HI_SUCCESS;
}

hi_s32 pngd_wait_event_timeout(hi_s32 chn_id, hi_img_stream *stream,
    hi_s32 unid, hi_s32 milli_sec)
{
    hi_s32 ret = 0;
    pngd_context *pngd_chn = HI_NULL;
    pngd_wait_sendstream wait_stream_info;
    hi_s32 device_id = trans_pngd_chan_id_to_device_id(chn_id);
    hi_s32 device_chan_id = trans_pngd_chan_id_to_device_chan_id(chn_id);

    pngd_chn = &g_pngd_chn[chn_id];

    wait_stream_info.chn_id = chn_id;
    wait_stream_info.unique_id = unid;
    wait_stream_info.stream_len = stream->len;
    ret = osal_wait_event_timeout_interruptible(&pngd_chn->wait_stream, pngd_wait_send_stream_callback,
        (hi_void *)&wait_stream_info, milli_sec);
    if (ret < 0) {
        /* wait_event_interruptible可能被ctrl+c打断 */
        PNGD_ERR_TRACE("pid %u usr chn %d device %d chn %d is stop with interruptible!\n",
            pngd_chn->pid, pngd_chn->user_chn_id, device_id, device_chan_id);
        return -ERESTARTSYS;
    } else if (ret == 0) {
        /* 超时了，返回失败 */
        PNGD_WARN_TRACE("pid %u usr chn %d device %d chn %d timeout!\n",
            pngd_chn->pid, pngd_chn->user_chn_id, device_id, device_chan_id);
        return HI_ERR_PNGD_BUF_FULL;
    }

    return HI_SUCCESS;
}

hi_s32 pngd_stream_wait_check(hi_s32 chn_id, pngd_context *pngd_chn,
    hi_img_stream *stream, hi_s32 unid)
{
    hi_u32 free_len   = 0;
    hi_s32 device_id = trans_pngd_chan_id_to_device_id(chn_id);
    hi_s32 device_chan_id = trans_pngd_chan_id_to_device_chan_id(chn_id);

    // 通道已经被销毁或被reset
    if ((pngd_chn->chn_state != PNGD_CREATED) || (unid != pngd_chn->unique_id) ||
        (g_pngd_state != PNGD_STATE_STARTED)) {
        PNGD_INFO_TRACE("pid %u usr chn %d device %d chn %d is destroyed\n",
            pngd_chn->pid, pngd_chn->user_chn_id, device_id, device_chan_id);
        return HI_ERR_PNGD_UNEXIST;
    }

    // 前面已经释放掉信号量了，有可能有其他的线程也在发送码流，导致码流BUF空间不足
    free_len = pngd_buf_free_count(&pngd_chn->frame_info_buf);
    if (free_len == 0) {
        PNGD_WARN_TRACE("pid %u usr chn %d device %d chn %d wake osal_up error datalen: %d usrlen:%d\n",
            pngd_chn->pid, pngd_chn->user_chn_id, device_id, device_chan_id, free_len, stream->len);
        return HI_ERR_PNGD_BUF_FULL;
    }

    return HI_SUCCESS;
}

hi_s32 pngd_jam_process_in_send_stream(hi_s32 chn_id, hi_s32 unid, hi_img_stream *stream, hi_s32 milli_sec)
{
    hi_s32 ret = 0;
    hi_s32 device_id = trans_pngd_chan_id_to_device_id(chn_id);
    hi_s32 device_chan_id = trans_pngd_chan_id_to_device_chan_id(chn_id);
    pngd_context *pngd_chn = &g_pngd_chn[chn_id];

    if (milli_sec == 0) {
        // 非阻塞方式，立刻返回
        PNGD_WARN_TRACE("pid %u usr chn %d device %d chn %d file mode is no block and buff full!\n",
            pngd_chn->pid, pngd_chn->user_chn_id, device_id, device_chan_id);
        return HI_ERR_PNGD_BUF_FULL;
    } else if (milli_sec == -1) {
        // 阻塞方式，进行死等
        ret = pngd_wait_event_interruptible(chn_id, stream, unid);
        if (ret != HI_SUCCESS) {
            return ret;
        }
    } else {
        // timeout方式
        ret = pngd_wait_event_timeout(chn_id, stream, unid, milli_sec);
        if (ret != HI_SUCCESS) {
            return ret;
        }
    }

    return HI_SUCCESS;
}
hi_s32 pngd_send_stream(hi_s32 chn_id, hi_img_stream *stream, hi_pic_info png_pic_info,
                        hi_u32 pid, hi_s32 milli_sec)
{
    pngd_context *pngd_chn = &g_pngd_chn[chn_id];
    hi_s32 ret  = 0;
    hi_s32 unid = 0;
    hi_u32 free_len   = 0;
    hi_s32 device_id = trans_pngd_chan_id_to_device_id(chn_id);
    hi_s32 device_chan_id = trans_pngd_chan_id_to_device_chan_id(chn_id);

    if (osal_down(&pngd_chn->pngd_sem)) {
        return -ERESTARTSYS;
    }

    ret = pngd_check_send_stream(chn_id, stream, png_pic_info, milli_sec);
    if (ret != HI_SUCCESS) {
        osal_up(&pngd_chn->pngd_sem);
        PNGD_ERR_TRACE("pid %u usr chn %d device %d chn %d check send stream param failed!\n",
            pngd_chn->pid, pngd_chn->user_chn_id, device_id, device_chan_id);
        return ret;
    }
    ret = pngd_check_mem(chn_id, stream, png_pic_info);
    if (ret != HI_SUCCESS) {
        osal_up(&pngd_chn->pngd_sem);
        return ret;
    }

    if (pngd_chn->aclmpi_mode) {
        pngd_mpi_stop_timer(pngd_chn);
    }

    // 获取可用的码流BUF空间
    free_len = pngd_buf_free_count(&pngd_chn->frame_info_buf);
    // 获取通道的unique_id，防止在阻塞或超时方式发送码流的过程中出现reset通道或销毁通道的情况
    unid = pngd_chn->unique_id;

    if (free_len == 0) {
        if (pngd_chn->aclmpi_mode) {
            ret = pngd_mpi_handle_input_full(pngd_chn, milli_sec);
            osal_up(&pngd_chn->pngd_sem);
            return ret;
        }
        osal_up(&pngd_chn->pngd_sem); // 释放信号量，要不然用户无法进行该通道的其他操作

        ret = pngd_jam_process_in_send_stream(chn_id, unid, stream, milli_sec);
        if (ret != HI_SUCCESS) {
            return ret;
        }
        if (osal_down(&pngd_chn->pngd_sem)) {
            return -ERESTARTSYS;
        }

        ret = pngd_stream_wait_check(chn_id, pngd_chn, stream, unid);
        if (ret != HI_SUCCESS) {
            osal_up(&pngd_chn->pngd_sem);
            return ret;
        }
    }

    ret = pngd_send_stream_process(chn_id, stream, png_pic_info, pid);

    // 需要唤醒可能在-1等待的GetImage
    osal_up(&pngd_chn->pngd_sem);
    osal_wakeup(&pngd_chn->wait_pic);

    if (pngd_chn->is_acl == HI_TRUE) {
        // 发送完毕直接通知acl来取,走GetImgData流程
        pngd_send_done_event(chn_id, PNGD_COMPLETE_SUCCESS);
        g_pngd_proc[pngd_chn->chn_id].acl_submit_complete_succ_times++;
    }

    return ret;
}

hi_s32 pngd_get_image_process(hi_s32 chn_id, hi_pic_info *png_pic_info,
                              hi_img_stream *stream, hi_bool *send_not_full_event)
{
    pngd_context *pngd_chn = NULL;
    pngd_image_info *img_info = NULL;
    hi_s32 device_chan_id = trans_pngd_chan_id_to_device_chan_id(chn_id);

    pngd_chn = &g_pngd_chn[chn_id];

    // step1: 获取busy链表上的节点
    img_info = pngd_buf_get_busy(&pngd_chn->frame_info_buf);
    if (img_info == NULL) {
        // 外面在调用这个函数的时候已经确定busy_count一定不为0，
        //   则一定可以从busy链表上获取到节点, 如果获取不到则说明程序有bug
        return HI_ERR_PNGD_BUF_EMPTY;
    }

    // step2: 拷贝节点信息
    (*png_pic_info) = img_info->png_pic_info;
    (*stream) = img_info->stream;
    g_pngd_proc[chn_id].mpi_get_pics++;

    // step3: 释放img_info到free队列
    pngd_buf_put_free(&pngd_chn->frame_info_buf, img_info);

    // step4: 给Host发送满到非满事件
    if ((pngd_chn->aclmpi_mode) && (pngd_chn->is_stream_buf_full == HI_TRUE)) {
        pngd_chn->is_stream_buf_full = HI_FALSE;
        (*send_not_full_event) = HI_TRUE;
    }

    return HI_SUCCESS;
}

// pngd_get_image阻塞等待分支处理函数
hi_s32 pngd_block_process_in_get_image(hi_s32 chn_id, hi_u32 uni_id,
                                       hi_pic_info *png_pic_info, hi_img_stream *stream)
{
    hi_s32 ret = HI_SUCCESS;
    hi_bool send_not_full_event = HI_FALSE;
    unsigned long spin_flag = 0;
    hi_s32 device_id = trans_pngd_chan_id_to_device_id(chn_id);
    hi_s32 device_chan_id = trans_pngd_chan_id_to_device_chan_id(chn_id);
    pngd_wait_image wait_image;

    while (1) {
        wait_image.chn_id = chn_id;
        wait_image.unique_id = uni_id;
        if (osal_wait_event_interruptible(&g_pngd_chn[chn_id].wait_pic,
            pngd_wait_get_image_callback, (hi_void *)&wait_image)) {
            return -ERESTARTSYS;
        }

        osal_spin_lock_irqsave(&(g_pngd_chn[chn_id].stream_spin_lock), &(spin_flag));

        // 检查此通道是否已被销毁或reset或重新创建
        if ((g_pngd_chn[chn_id].chn_state != PNGD_CREATED) || (uni_id != g_pngd_chn[chn_id].unique_id) ||
            (g_pngd_state != PNGD_STATE_STARTED)) {
            osal_spin_unlock_irqrestore(&(g_pngd_chn[chn_id].stream_spin_lock), &(spin_flag));
            PNGD_ERR_TRACE("pid %u usr chn %d device %d chn %d get frame error! it's been destroyed or reset!\n",
                g_pngd_chn[chn_id].pid, g_pngd_chn[chn_id].user_chn_id, device_id, device_chan_id);
            return HI_ERR_PNGD_UNEXIST;
        }

        // 检查是否能获取图像，如果不能则继续等待，否则跳出循环
        if (g_pngd_chn[chn_id].frame_info_buf.busy_count == 0) {
            osal_spin_unlock_irqrestore(&(g_pngd_chn[chn_id].stream_spin_lock), &(spin_flag));
            continue;
        }

        ret = pngd_get_image_process(chn_id, png_pic_info, stream, &send_not_full_event);
        osal_spin_unlock_irqrestore(&(g_pngd_chn[chn_id].stream_spin_lock), &(spin_flag));
        if (send_not_full_event == HI_TRUE) {
            g_pngd_proc[chn_id].acl_submit_not_full_times++;
            g_pngd_proc[chn_id].acl_is_full = 0;
            pngd_send_done_event(chn_id, PNGD_FULL_TO_NOT_FULL);
        }

        return ret;
    }

    return HI_SUCCESS;
}

// pngd_get_image超时等待分支处理函数
hi_s32 pngd_timeout_process_in_get_image(hi_s32 chn_id, hi_u32 uni_id,
                                         hi_pic_info *png_pic_info, hi_img_stream *stream, hi_s32 milli_sec)
{
    hi_s32 ret = HI_SUCCESS;
    hi_u64 start_time = 0;
    unsigned long spin_flag = 0;
    hi_bool send_not_full_event = HI_FALSE;
    hi_s32 device_id = trans_pngd_chan_id_to_device_id(chn_id);
    hi_s32 device_chan_id = trans_pngd_chan_id_to_device_chan_id(chn_id);
    pngd_wait_image wait_image;

    // 超时
    wait_image.chn_id = chn_id;
    wait_image.unique_id = uni_id;
    start_time = get_sys_time_by_usec();
    ret = osal_wait_event_timeout_interruptible(&g_pngd_chn[chn_id].wait_pic,
                                                pngd_wait_get_image_callback, (hi_void *)&wait_image,
                                                milli_sec);
    if (ret < 0) {  // wait_event出错
        return -ERESTARTSYS;
    } else if (ret == 0) {  // 等待超时
        return HI_ERR_PNGD_BUF_EMPTY;
    } else {  // wait条件满足
        osal_spin_lock_irqsave(&(g_pngd_chn[chn_id].stream_spin_lock), &(spin_flag));

        // 检查此通道是否已被销毁或reset或重新创建
        if ((g_pngd_chn[chn_id].chn_state != PNGD_CREATED) || (uni_id != g_pngd_chn[chn_id].unique_id) ||
            (PNGD_STATE_STARTED != g_pngd_state)) {
            osal_spin_unlock_irqrestore(&(g_pngd_chn[chn_id].stream_spin_lock), &(spin_flag));
            PNGD_ERR_TRACE("pid %u usr chn %d device %d chn %d get frame error! it's been destroyed or reset!\n",
                g_pngd_chn[chn_id].pid, g_pngd_chn[chn_id].user_chn_id, device_id, device_chan_id);
            return HI_ERR_PNGD_UNEXIST;
        }

        // 检查是否能获取图像，如果不能，则返回无图像
        if (g_pngd_chn[chn_id].frame_info_buf.busy_count == 0) {
            osal_spin_unlock_irqrestore(&(g_pngd_chn[chn_id].stream_spin_lock), &(spin_flag));
            return HI_ERR_PNGD_BUF_EMPTY;
        }

        ret = pngd_get_image_process(chn_id, png_pic_info, stream, &send_not_full_event);
        osal_spin_unlock_irqrestore(&(g_pngd_chn[chn_id].stream_spin_lock), &(spin_flag));
        if (send_not_full_event == HI_TRUE) {
            g_pngd_proc[chn_id].acl_is_full = 0;
            g_pngd_proc[chn_id].acl_submit_not_full_times++;
            pngd_send_done_event(chn_id, PNGD_FULL_TO_NOT_FULL);
        }

        return ret;
    }

    return HI_SUCCESS;
}

hi_s32 pngd_get_image(hi_s32 chn_id, hi_pic_info *png_pic_info, hi_img_stream *stream, hi_s32 milli_sec)
{
    hi_bool send_not_full_event = HI_FALSE;
    pngd_context *pngd_chn = &g_pngd_chn[chn_id];
    hi_s32 ret = HI_SUCCESS;
    unsigned long spin_flag = 0;
    hi_u32 uni_id = 0;

    if (osal_down(&pngd_chn->pngd_sem)) {
        return -ERESTARTSYS;
    }

    if (g_pngd_chn[chn_id].chn_state != PNGD_CREATED) {
        osal_up(&pngd_chn->pngd_sem);
        return HI_ERR_PNGD_UNEXIST;
    }

    uni_id = pngd_chn->unique_id;
    if (pngd_chn->is_acl == HI_TRUE) {
        g_pngd_proc[chn_id].acl_get_frame_times++;
    }

    osal_spin_lock_irqsave(&(g_pngd_chn[chn_id].stream_spin_lock), &(spin_flag));

    // step1: 如果busy链表上有图像，则获取图像
    if (pngd_chn->frame_info_buf.busy_count != 0) {
        ret = pngd_get_image_process(chn_id, png_pic_info, stream, &send_not_full_event);
        osal_spin_unlock_irqrestore(&(g_pngd_chn[chn_id].stream_spin_lock), &(spin_flag));
        osal_up(&pngd_chn->pngd_sem);

        if (send_not_full_event == HI_TRUE) {
            g_pngd_proc[chn_id].acl_is_full = 0;
            g_pngd_proc[chn_id].acl_submit_not_full_times++;
            pngd_send_done_event(chn_id, PNGD_FULL_TO_NOT_FULL);
        }
        return ret;
    }

    // 释放锁和信号量,进入超时或阻塞流程
    osal_spin_unlock_irqrestore(&(g_pngd_chn[chn_id].stream_spin_lock), &(spin_flag));
    osal_up(&pngd_chn->pngd_sem);

    if (milli_sec == 0) { // 非阻塞，获取不到图像，直接返回
        return HI_ERR_PNGD_BUF_EMPTY;
    }

    // 阻塞
    if (milli_sec < 0) {
        ret = pngd_block_process_in_get_image(chn_id, uni_id, png_pic_info, stream);
        if (ret != HI_SUCCESS) {
            return ret;
        }
    } else {
    // 超时
        ret = pngd_timeout_process_in_get_image(chn_id, uni_id, png_pic_info, stream, milli_sec);
        if (ret != HI_SUCCESS) {
            return ret;
        }
    }

    osal_wakeup(&pngd_chn->wait_stream);

    return ret;
}

hi_void pngd_destroy_vb(hi_s32 device_id, hi_s32 device_chan_id, pngd_context *pngd_chn)
{
    unsigned long state_flag = 0;
    unsigned long stream_flag = 0;

    pngd_buf_release(&pngd_chn->frame_info_buf);

    osal_spin_lock_irqsave(&(pngd_chn->stream_spin_lock), &(stream_flag));
    pngd_chn->chn_state = PNGD_DESTROYED;
    osal_spin_unlock_irqrestore(&(pngd_chn->stream_spin_lock), &(stream_flag));

    osal_spin_lock_irqsave(&(g_pngd_state_spin_lock), &(state_flag));
    if (((pngd_extern_func *)cmpi_get_module_func_by_id(HI_ID_PNGD))->pfn_pngd_del_chn != NULL) {
        ((pngd_extern_func *)cmpi_get_module_func_by_id(HI_ID_PNGD))->pfn_pngd_del_chn();
    }
    osal_spin_unlock_irqrestore(&(g_pngd_state_spin_lock), &(state_flag));

    PNGD_NOTICE_TRACE("pid %u usr chn %d device %d chn %d kernel_mem %llu\n",
        pngd_chn->pid, pngd_chn->user_chn_id, device_id, device_chan_id, pngd_chn->mfile.total_mem_kernel);
    media_mem_file_uninit(&pngd_chn->mfile);
}

hi_s32 pngd_destroy(hi_s32 chn_id)
{
    pngd_context *pngd_chn = HI_NULL;
    unsigned long stream_flag = 0;
    hi_s32 device_id = trans_pngd_chan_id_to_device_id(chn_id);
    hi_s32 device_chan_id = trans_pngd_chan_id_to_device_chan_id(chn_id);

    if (chn_id < 0) {
        PNGD_ERR_TRACE("pngd chnl:%d has not been created \n", chn_id);
        return HI_ERR_PNGD_UNEXIST;
    }
    if (chn_id >= (g_device_num * PNGD_MAX_CHN_NUM_PER_DEVICE)) {
        PNGD_ERR_TRACE("device id %d is out of range[0, %d) \n",
            chn_id / PNGD_MAX_CHN_NUM_PER_DEVICE, g_device_num);
        return HI_ERR_PNGD_INVALID_CHN_ID;
    }
    pngd_chn = &g_pngd_chn[chn_id];
    // step1: 检查通道可以被销毁的必要条件
    if (osal_down(&pngd_chn->pngd_sem)) {
        PNGD_ERR_TRACE("pid %u usr chn %d device %d chn %d semaphore down fail!\n",
            pngd_chn->pid, pngd_chn->user_chn_id, device_id, device_chan_id);
        return -ERESTARTSYS;
    }

    if (pngd_chn->aclmpi_mode) {
        pngd_mpi_destroy_timer(pngd_chn);
    }

    if (pngd_chn->chn_state == PNGD_DESTROYING) {
        goto destroy_vb;
    }

    if (g_pngd_chn[chn_id].chn_state != PNGD_CREATED) {
        osal_up(&g_pngd_chn[chn_id].pngd_sem);
        return HI_ERR_PNGD_UNEXIST;
    }

    osal_spin_lock_irqsave(&(g_pngd_chn[chn_id].stream_spin_lock), &(stream_flag));
    pngd_chn->chn_state = PNGD_DESTROYING;
    osal_spin_unlock_irqrestore(&(g_pngd_chn[chn_id].stream_spin_lock), &(stream_flag));

    osal_wakeup(&pngd_chn->wait_stream);
    osal_wakeup(&pngd_chn->wait_pic);

destroy_vb:
    pngd_destroy_vb(device_id, device_chan_id, pngd_chn);
    osal_spin_lock(&g_pngd_chn_vf_manager.pngd_vf_lock[device_id][pngd_chn->vf_id]);
    g_pngd_chn_vf_manager.created_chn_num[device_id][pngd_chn->vf_id]--;
    osal_spin_unlock(&g_pngd_chn_vf_manager.pngd_vf_lock[device_id][pngd_chn->vf_id]);
    pngd_chn->vf_id = 0;
    osal_up(&pngd_chn->pngd_sem);
    PNGD_INFO_TRACE("pid %u usr chn %d device %d chn %d pngd_destroy end. send:%u, get:%u",
        pngd_chn->pid, pngd_chn->user_chn_id, device_id, device_chan_id,
        g_pngd_proc[chn_id].mpi_send_strm_times, g_pngd_proc[chn_id].mpi_get_pics);

    return HI_SUCCESS;
}

hi_s32 trans_pngd_chan_id_to_device_id(hi_s32 chan_id)
{
    return chan_id / PNGD_MAX_CHN_NUM_PER_DEVICE;
}

hi_s32 trans_pngd_chan_id_to_usr_chn_id(hi_s32 chan_id)
{
    if (chan_id < 0 || (chan_id >= (g_device_num * PNGD_MAX_CHN_NUM_PER_DEVICE))) {
        return -1;
    }
    return g_pngd_chn[chan_id].user_chn_id;
}

hi_s32 trans_pngd_chan_id_to_device_chan_id(hi_s32 chan_id)
{
    return chan_id % PNGD_MAX_CHN_NUM_PER_DEVICE;
}

void pngd_mpi_timer_proc(unsigned long data)
{
    hi_ulong spin_flag = 0;
    hi_bool to_submit = HI_FALSE;

    pngd_context *pngd_chn = (pngd_context *)(uintptr_t)osal_timer_get_private_data((hi_void*)(hi_uintptr_t)data);
    g_pngd_proc[pngd_chn->chn_id].mpi_timer_run++;

    osal_spin_lock_irqsave(&(pngd_chn->stream_spin_lock), &(spin_flag));
    if (pngd_chn->is_stream_buf_full == HI_TRUE) {
        pngd_chn->is_stream_buf_full = HI_FALSE;
        to_submit = HI_TRUE;
    }
    osal_spin_unlock_irqrestore(&(pngd_chn->stream_spin_lock), &(spin_flag));

    if (to_submit) {
        PNGD_INFO_TRACE("pngd chn %d send stream timeout", pngd_chn->chn_id);
        g_pngd_proc[pngd_chn->chn_id].mpi_timer_timeout++;
        pngd_send_done_event(pngd_chn->chn_id, PNGD_SEND_TIMEOUT);
    }
}

/*
 * mpi场景：启动输入定时器, 通道加锁情况下调用
 */
hi_s32 pngd_mpi_setup_timer(pngd_context *pngd_chn, hi_s32 milli_sec)
{
    hi_u32 ai_cpu_id = 0;
    hi_s32 device_id = trans_pngd_chan_id_to_device_id(pngd_chn->chn_id);
    hi_s32 device_chan_id = trans_pngd_chan_id_to_device_chan_id(pngd_chn->chn_id);

    if (!pngd_chn->mpi_timer.timer) {
        pngd_chn->mpi_timer.function = pngd_mpi_timer_proc;
        pngd_chn->mpi_timer.data = (unsigned long)(hi_uintptr_t)pngd_chn;

        if (osal_timer_init(&pngd_chn->mpi_timer)) {
            pngd_chn->mpi_timer.timer = HI_NULL;
            PNGD_ERR_TRACE("pid %u usr chn %d device %d pngd chn %d init mpi_timer fail",
                pngd_chn->pid, pngd_chn->user_chn_id, device_id, pngd_chn->chn_id);
            return HI_ERR_PNGD_NO_MEM;
        }
        ai_cpu_id = call_sys_get_cpu_id_by_index((hi_u32)device_id, (hi_u32)device_chan_id, CPU_TYPE_AI);
        pngd_chn->mpi_timer.bind_cpu_id = ai_cpu_id;
    }

    osal_set_timer_on_cpu(&pngd_chn->mpi_timer, milli_sec);
    pngd_chn->mpi_timer_pendding = HI_TRUE;

    g_pngd_proc[pngd_chn->chn_id].mpi_timer_setup++;

    return HI_SUCCESS;
}

/*
 * mpi场景：停止通道输入定时器，通道加锁情况下调用
 */
hi_void pngd_mpi_stop_timer(pngd_context *pngd_chn)
{
    if (pngd_chn->mpi_timer_pendding) {
        osal_del_timer(&pngd_chn->mpi_timer);
        pngd_chn->mpi_timer_pendding = HI_FALSE;
        g_pngd_proc[pngd_chn->chn_id].mpi_timer_kill++;
    }
}

/*
 * mpi场景：销毁通道输入定时器，通道加锁情况下调用
 */
hi_void pngd_mpi_destroy_timer(pngd_context *pngd_chn)
{
    if (pngd_chn->mpi_timer.timer) {
        pngd_mpi_stop_timer(pngd_chn);
        osal_timer_destory(&pngd_chn->mpi_timer);
        pngd_chn->mpi_timer.timer = HI_NULL;
    }
}

/*
 * mpi场景：处理输入buffer满
 * 超时 > 0  :开启mpi输入超时检查定时器
 * 超时 = 0  :返回队列满
 * 超时 = -1 :调用者只等待满到非满事件, 不开启定时器
 */
hi_s32 pngd_mpi_handle_input_full(pngd_context *pngd_chn, hi_s32 milli_sec)
{
    pngd_chn->is_stream_buf_full = HI_TRUE;
    g_pngd_proc[pngd_chn->chn_id].acl_is_full = 1;
    g_pngd_proc[pngd_chn->chn_id].acl_full_count++;

    if (milli_sec > 0) {
        hi_s32 ret = pngd_mpi_setup_timer(pngd_chn, milli_sec);
        if (ret != HI_SUCCESS) {
            return ret;
        }
    }
    return HI_ERR_PNGD_BUF_FULL;
}

hi_s32 pngd_chn_vf_manager_init(hi_void)
{
    hi_u32 device_id = 0;
    hi_u32 vf_id = 0;
    hi_s32 ret = HI_SUCCESS;
    for (device_id = 0; device_id < DEVICE_NUM; device_id++) {
        g_pngd_chn_vf_manager.calculated_chn_num[device_id] = 0;
        ret = osal_spin_lock_init(&g_pngd_chn_vf_manager.pngd_vf_manager_lock[device_id]);
        if (ret != 0) {
            PNGD_ERR_TRACE("g_pngd_chn_vf_manager.pngd_vf_lock[%d] init failed! ret = %d\n",
                device_id, ret);
            return ret;
        }

        for (vf_id = 0; vf_id < MAX_VF_ID_NUM; vf_id++) {
            g_pngd_chn_vf_manager.max_chn_num[device_id][vf_id] = 0;
            g_pngd_chn_vf_manager.created_chn_num[device_id][vf_id] = 0;
            ret = osal_spin_lock_init(&g_pngd_chn_vf_manager.pngd_vf_lock[device_id][vf_id]);
            if (ret != 0) {
                PNGD_ERR_TRACE("g_pngd_chn_vf_manager.pngd_vf_lock[%d][%d] init failed! ret = %d\n",
                    device_id, vf_id, ret);
                return ret;
            }
        }
        g_pngd_chn_vf_manager.max_chn_num[device_id][0] = PNGD_MAX_CHN_NUM_PER_DEVICE; // 0表示物理机
    }
    return HI_SUCCESS;
}

hi_void pngd_chn_vf_manager_uninit(hi_void)
{
    hi_u32 device_id = 0;
    hi_u32 vf_id = 0;
    for (device_id = 0; device_id < DEVICE_NUM; device_id++) {
        g_pngd_chn_vf_manager.calculated_chn_num[device_id] = 0;
        osal_spin_lock_destory(&g_pngd_chn_vf_manager.pngd_vf_manager_lock[device_id]);
        for (vf_id = 0; vf_id < MAX_VF_ID_NUM; vf_id++) {
            g_pngd_chn_vf_manager.max_chn_num[device_id][vf_id] = 0;
            g_pngd_chn_vf_manager.created_chn_num[device_id][vf_id] = 0;
            osal_spin_lock_destory(&g_pngd_chn_vf_manager.pngd_vf_lock[device_id][vf_id]);
        }
    }
}

hi_s32 pngd_calc_get_core_num(hi_u32 dtype, hi_u32 *core_num, hi_u32 *total_core_num)
{
    hi_s32 ret = HI_SUCCESS;
    PNGD_INFO_TRACE("dtype = %u", dtype);
    switch (dtype) {
        case VMNG_HW_TYPE_C1_4:
            *core_num = 1; // 1/4
            *total_core_num = 4; // 1/4
            break;
        case VMNG_HW_TYPE_C2_4:
            *core_num = 2; // 2/4
            *total_core_num = 4; // 2/4
            break;
        default:
            PNGD_ERR_TRACE("dtype = %u is not supported", dtype);
            ret = HI_FAILURE;
            break;
    }
    return ret;
}

hi_s32 pngd_chn_calc_init(struct vmngd_client_instance *instance)
{
    hi_u32 device_id = 0;
    hi_u32 vf_id = 0;
    hi_u32 max_chn_num = 0;
    hi_u32 vdec_core_num = 0;
    hi_u32 core_num = 0;
    hi_u32 total_core_num = 0;
    hi_s32 ret = HI_SUCCESS;
    sys_chip_type chip_type = call_sys_get_chip_type();

    device_id = instance->vdev_ctrl.dev_id;
    vf_id = instance->vdev_ctrl.vfid;
    core_num = instance->vdev_ctrl.core_num;
    total_core_num = instance->vdev_ctrl.total_core_num;

    if ((chip_type == SYS_CHIP_MINI_V2) || (chip_type == SYS_CHIP_CLOUD_V2)) {
        vdec_core_num = get_num_from_bitmap(instance->vdev_ctrl.vf_cfg.dvpp.vdec_bitmap);
    } else {
        vdec_core_num = instance->vdev_ctrl.vf_cfg.dvpp.vdec_bitmap;
    }

    if (chip_type == SYS_CHIP_MINI_V2) {
        ret = pngd_calc_get_core_num(instance->vdev_ctrl.dtype, &core_num, &total_core_num);
        if (ret != HI_SUCCESS) {
            return ret;
        }
    }

    if (total_core_num == 0) {
        PNGD_ERR_TRACE("total_core_num is 0");
        return HI_FAILURE;
    }

    // 互斥保护max_chn_num
    osal_spin_lock(&g_pngd_chn_vf_manager.pngd_vf_lock[device_id][vf_id]);

    // 计算本次切分需要分配的通道数
    if (vdec_core_num == vdec_get_ip_num_per_device()) {
        // VIR04_4C_DVPP模板,PNGD不切分通道
        max_chn_num = PNGD_MAX_CHN_NUM_PER_DEVICE;
    } else {
        max_chn_num = core_num * PNGD_MAX_CHN_NUM_PER_DEVICE / total_core_num;
    }

    // 计算已算力分配的通道数+次切分需要分配的通道数是否大于最大通道数
    if ((max_chn_num + g_pngd_chn_vf_manager.calculated_chn_num[device_id]) > PNGD_MAX_CHN_NUM_PER_DEVICE) {
        PNGD_ERR_TRACE("chn resource is not enough, dev_id = %d, vfid = %d, need chn_num = %d, left chn_num = %d",
            device_id, vf_id, max_chn_num,
            PNGD_MAX_CHN_NUM_PER_DEVICE - g_pngd_chn_vf_manager.calculated_chn_num[device_id]);

        osal_spin_unlock(&g_pngd_chn_vf_manager.pngd_vf_lock[device_id][vf_id]);
        return HI_FAILURE;
    }

    g_pngd_chn_vf_manager.max_chn_num[device_id][vf_id] = max_chn_num; // 进行通道数划分
    g_pngd_chn_vf_manager.calculated_chn_num[device_id] += max_chn_num; // 记录已经划分过的通道数

    osal_spin_unlock(&g_pngd_chn_vf_manager.pngd_vf_lock[device_id][vf_id]);

    PNGD_INFO_TRACE("pngd_chn_calc_init success, dev_id = %d, vfid = %d, max_chn_num = %d, calculated_chn_num = %d",
        device_id, vf_id, g_pngd_chn_vf_manager.max_chn_num[device_id][vf_id],
        g_pngd_chn_vf_manager.calculated_chn_num[device_id]);

    return HI_SUCCESS;
}

hi_s32 pngd_chn_calc_uninit(struct vmngd_client_instance *instance)
{
    hi_u32 device_id = 0;
    hi_u32 vf_id = 0;
    hi_u32 max_chn_num = 0;

    device_id = instance->vdev_ctrl.dev_id;
    vf_id = instance->vdev_ctrl.vfid;

    // 互斥保护max_chn_num
    osal_spin_lock(&g_pngd_chn_vf_manager.pngd_vf_lock[device_id][vf_id]);

    max_chn_num = g_pngd_chn_vf_manager.max_chn_num[device_id][vf_id];
    g_pngd_chn_vf_manager.calculated_chn_num[device_id] -= max_chn_num; // 记录已经划分过的通道数
    g_pngd_chn_vf_manager.max_chn_num[device_id][vf_id] = 0; // 通道划分清零

    osal_spin_unlock(&g_pngd_chn_vf_manager.pngd_vf_lock[device_id][vf_id]);

    PNGD_INFO_TRACE("dev_id = %d, vfid = %d, max_chn_num = %d, calculated_chn_num = %d",
        device_id, vf_id, g_pngd_chn_vf_manager.max_chn_num[device_id][vf_id],
        g_pngd_chn_vf_manager.calculated_chn_num[device_id]);
    return HI_SUCCESS;
}

hi_s32 pngd_calc_init(struct vmngd_client_instance *instance)
{
    hi_s32 ret = HI_SUCCESS;
    hi_u32 device_id = 0;
    hi_u32 vf_id = 0;
    hi_u32 core_num = 0;
    hi_u32 total_core_num = 0;
    hi_u32 vdec_core_num = 0;
    hi_u32 vpc_core_num = 0;
    hi_u32 venc_core_num = 0;
    hi_u32 jpegd_core_num = 0;
    hi_u32 jpege_core_num = 0;
    hi_u32 dvpp_core_num = 0;
    sys_chip_type chip_type = SYS_CHIP_DC;

    if (try_module_get(THIS_MODULE) == false) { // 在算力切分场景下,只有算力被销毁的情况下才允许卸载ko
        PNGD_ERR_TRACE("try_module_get fail");
        return HI_FAILURE;
    }

    if (ckfn_sys_get_chip_type() == HI_FALSE) {
        PNGD_ERR_TRACE("tcheck function ckfn_sys_get_chip_type fail");
        if (module_refcount(THIS_MODULE) > 0) {
            module_put(THIS_MODULE);
        }
        return HI_FAILURE;
    }

    chip_type = call_sys_get_chip_type();
    device_id = instance->vdev_ctrl.dev_id;
    vf_id = instance->vdev_ctrl.vfid;
    core_num = instance->vdev_ctrl.core_num;
    total_core_num = instance->vdev_ctrl.total_core_num;

    if ((chip_type == SYS_CHIP_MINI_V2) || (chip_type == SYS_CHIP_CLOUD_V2)) {
        vdec_core_num = get_num_from_bitmap(instance->vdev_ctrl.vf_cfg.dvpp.vdec_bitmap);
        venc_core_num = get_num_from_bitmap(instance->vdev_ctrl.vf_cfg.dvpp.venc_bitmap);
        vpc_core_num = get_num_from_bitmap(instance->vdev_ctrl.vf_cfg.dvpp.vpc_bitmap);
        jpegd_core_num = get_num_from_bitmap(instance->vdev_ctrl.vf_cfg.dvpp.jpegd_bitmap);
        jpege_core_num = get_num_from_bitmap(instance->vdev_ctrl.vf_cfg.dvpp.jpege_bitmap);
    } else {
        vdec_core_num = instance->vdev_ctrl.vf_cfg.dvpp.vdec_bitmap;
        venc_core_num = instance->vdev_ctrl.vf_cfg.dvpp.venc_bitmap;
        vpc_core_num = instance->vdev_ctrl.vf_cfg.dvpp.vpc_bitmap;
        jpegd_core_num = instance->vdev_ctrl.vf_cfg.dvpp.jpegd_bitmap;
        jpege_core_num = instance->vdev_ctrl.vf_cfg.dvpp.jpege_bitmap;
    }
    dvpp_core_num = vdec_core_num + venc_core_num + vpc_core_num + jpegd_core_num + jpege_core_num;

    if (chip_type == SYS_CHIP_MINI_V2) {
        ret = pngd_calc_get_core_num(instance->vdev_ctrl.dtype, &core_num, &total_core_num);
        if (ret != HI_SUCCESS) {
            if (module_refcount(THIS_MODULE) > 0) {
                module_put(THIS_MODULE);
            }
            return ret;
        }
    }

    PNGD_INFO_TRACE("start pngd_calc_init, dev_id = %u, vfid = %u, core_num = %u, total_core_num = %u",
        device_id, vf_id, core_num, total_core_num);

    // 由于PNGD没有硬件核,使用Dvpp总的核数来判断VIR04_3C_NDVPP模板
    if (dvpp_core_num == 0) {
        PNGD_INFO_TRACE("pngd_calc_init success, dev_id = %u, vfid = %u has no dvpp", device_id, vf_id);
        if (module_refcount(THIS_MODULE) > 0) {
            module_put(THIS_MODULE);
        }
        return HI_SUCCESS;
    }

    osal_spin_lock(&g_pngd_chn_vf_manager.pngd_vf_manager_lock[device_id]);

    ret = pngd_chn_calc_init(instance); // 进行Pngd通道算力切分
    if (ret != HI_SUCCESS) {
        osal_spin_unlock(&g_pngd_chn_vf_manager.pngd_vf_manager_lock[device_id]);
        PNGD_ERR_TRACE("pngd_calc_init failed");
        if (module_refcount(THIS_MODULE) > 0) {
            module_put(THIS_MODULE);
        }
        return HI_FAILURE;
    }

    osal_spin_unlock(&g_pngd_chn_vf_manager.pngd_vf_manager_lock[device_id]);
    PNGD_INFO_TRACE("start pngd_calc_init success, dev_id = %u, vfid = %u, core_num = %u, total_core_num = %u",
        device_id, vf_id, core_num, total_core_num);
    return HI_SUCCESS;
}

hi_s32 pngd_calc_uninit(struct vmngd_client_instance *instance)
{
    hi_u32 device_id = 0;
    hi_u32 vf_id = 0;

    device_id = instance->vdev_ctrl.dev_id;
    vf_id = instance->vdev_ctrl.vfid;

    PNGD_INFO_TRACE("start pngd_calc_uninit, dev_id = %u, vfid = %u", device_id, vf_id);

    osal_spin_lock(&g_pngd_chn_vf_manager.pngd_vf_manager_lock[device_id]);

    pngd_chn_calc_uninit(instance); // 通道划分去初始化

    osal_spin_unlock(&g_pngd_chn_vf_manager.pngd_vf_manager_lock[device_id]);

    PNGD_INFO_TRACE("pngd_calc_uninit success, dev_id = %u, vfid = %u", device_id, vf_id);

    if (module_refcount(THIS_MODULE) > 0) {
        module_put(THIS_MODULE);
    }
    return HI_SUCCESS;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */


