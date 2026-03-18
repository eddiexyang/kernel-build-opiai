/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2023. All rights reserved.
 * Description: ai module driver
 * Author: Hisilicon multimedia software group
 * Create: 2022/5/5
 */
#include <linux/printk.h>
#include "ai.h"
#include "hi_osal.h"
#include "securec.h"
#include "hi_comm_audio.h"
#include "dev_ext.h"
#include "ai_trans.h"
#include "ai_drv.h"
#include "mod_ext.h"
#include "ai_ext.h"
#include "ao_ext.h"
#include "proc_ext.h"
#include "mkp_aio.h"
#include "vb_ext.h"
#include "audio_fb.h"
#include "sys_ext.h"
#include "proc_ext.h"
#include "mm_ext.h"
#include "aio_pub.h"
#include "pid_protect.h"
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
#include "ai_proc.h"
#endif

#ifdef AUDIO_UT_VCAST
#include <linux/device.h>
#endif

#define AI_STATE_STARTED 0
#define AI_STATE_STOPING 1
#define AI_STATE_STOPED  2

#define AI_STEREO_CHN_NUM 2

#ifndef umap_get_chn
#define umap_get_chn UMAP_GET_CHN
#endif

#ifndef umap_set_chn
#define umap_set_chn UMAP_SET_CHN
#endif
#define TYPE_AI_NUM 0
typedef enum {
    AI_FRAMETYPE_USER,
    AI_FRAMETYPE_AIE
} ai_frametype;
static td_void ai_ctx_exit(ot_audio_dev audio_dev_id);
static td_s32 ai_ctx_init(ot_audio_dev audio_dev_id);
typedef td_s32 (*fn_wait_call_bak)(const td_void *param);
static osal_atomic_t g_ai_open_ref = OSAL_ATOMIC_INIT(0);
static osal_atomic_t g_user_ref = OSAL_ATOMIC_INIT(0);
static osal_dev_t *g_ai_umapd = TD_NULL;

static td_u32 g_ai_state = AI_STATE_STOPED;

static struct osal_semaphore g_ai_sem;

static td_void ai_dev_sem_do_destroy(ot_audio_dev dev_id)
{
    td_s32 i;
    for (i = 0; i <= dev_id; i++) {
        ai_dev_sem_exit(i);
    }
}

static td_void ai_ctx_do_destroy(ot_audio_dev dev_id)
{
    td_s32 i;
    for (i = 0; i < dev_id; i++) {
        ai_ctx_exit(i);
    }
}

static int ai_open(void *private_data)
{
    td_s32 i, ret;

    if (osal_down(&g_ai_sem) != 0) {
        ai_err_trace("ai open semaphore down fail!\n");
        return TD_FAILURE;
    }

    if (osal_atomic_read(&g_ai_open_ref) >= MAX_OPEN_CNT) {
        osal_up(&g_ai_sem);
        return -1;
    }
    if (check_is_same_pid(TD_TRUE) != TD_TRUE) {
        osal_up(&g_ai_sem);
        return -1;
    }
    if (osal_atomic_inc_return(&g_ai_open_ref) != 1) {
        osal_up(&g_ai_sem);
        ai_debug_trace(" ai already initialization when first open\n");
        return TD_SUCCESS;
    }
    for (i = 0; i < OT_AI_DEV_MAX_NUM; i++) {
        ai_dev_sem_init(i);
        ret = ai_ctx_init(i);
        if (ret != TD_SUCCESS) {
            ai_ctx_do_destroy(i);
            ai_dev_sem_do_destroy(i);
            (void)osal_atomic_dec_return(&g_ai_open_ref);
            check_is_close();
            osal_up(&g_ai_sem);
            ai_err_trace("ai_ctx_init failed!\n");
            return -1;
        }
    }
    g_ai_state = AI_STATE_STARTED;

    osal_up(&g_ai_sem);

    return TD_SUCCESS;
}

static int ai_close(void *private_data)
{
    td_s32 i;
    if (osal_down(&g_ai_sem) != 0) {
        ai_err_trace("ai close semaphore down fail!\n");
        return TD_FAILURE;
    }

    if (osal_atomic_dec_return(&g_ai_open_ref) == 0) {
        for (i = 0; i < OT_AI_DEV_MAX_NUM; i++) {
            ai_ctx_exit(i);
            ai_dev_sem_exit(i);
        }
        g_ai_state = AI_STATE_STOPED;
    }
    check_is_close();

    osal_up(&g_ai_sem);

    return TD_SUCCESS;
}

static td_s32 ai_enable_chn(ot_audio_dev aio_dev, ot_ai_chn ai_chn)
{
    aio_dev_ctx *dev_ctx = TD_NULL;
    ai_chn_ctx *chn_ctx = TD_NULL;
    td_s32 chn_id, n, chn_per_frm;
    unsigned long lock_flag;

    ai_check_func_entrance_return(OT_ID_AIO);
    ai_check_chn_id_return(ai_chn);
    ai_check_dev_return(aio_dev);
    dev_ctx = ai_get_dev_ctx(aio_dev);
    if (osal_down_interruptible(&dev_ctx->sem)) {
        return (-ERESTARTSYS);
    }

    /* check device is enable */
    if (dev_ctx->enable != TD_TRUE) {
        ai_err_trace("AI dev %d not enable\n", aio_dev);
        osal_up(&dev_ctx->sem);
        return OT_ERR_AI_NOT_ENABLED;
    }

    chn_per_frm = aio_chn_num_per_frm(dev_ctx->aio_attr.snd_mode); /* chn_per_frm 只会为1或2，不会为0 */
    if (ai_chn >= (ot_ai_chn)(((td_s32)dev_ctx->aio_attr.chn_cnt) / chn_per_frm)) {
        osal_up(&dev_ctx->sem);
        ai_err_trace("no aichn %d for aidev %d, or can't handle right channels for stereo mode\n", ai_chn, aio_dev);
        return OT_ERR_AI_INVALID_CHN_ID;
    }

    for (n = 0; n < chn_per_frm; n++) {
        ai_chn += ((td_s32)dev_ctx->aio_attr.chn_cnt / AI_STEREO_CHN_NUM) * n;

        chn_id = ai_get_chn_id(aio_dev, ai_chn);
        chn_ctx = ai_get_chn_ctx(chn_id);
        if (osal_down_interruptible(&chn_ctx->sem)) {
            osal_up(&dev_ctx->sem);
            return -ERESTARTSYS;
        }

        chn_ctx->aio_user_dbg.ai_get_cnt = 0;
        chn_ctx->aio_user_dbg.ai_release_cnt = 0;

        chn_ctx->chn_param.usr_frame_depth = (dev_ctx->aio_attr.frame_num > OT_MAX_AI_USRFRM_DEPTH) ?
            OT_MAX_AI_USRFRM_DEPTH : dev_ctx->aio_attr.frame_num;

        aio_spin_lock_irqsave(&(ai_get_dev_ctx(aio_dev)->spinlock), lock_flag);
        chn_ctx->chn_state = AIO_CHN_ENABLE;
        aio_spin_unlock_irqrestore(&(ai_get_dev_ctx(aio_dev)->spinlock), lock_flag);

        osal_up(&chn_ctx->sem);

        ai_info_trace("enable AI chn(%d,%d) ok\n", aio_dev, ai_chn);
    }

    osal_up(&dev_ctx->sem);
    return TD_SUCCESS;
}

static td_void ai_release_chn_fb(ai_chn_ctx *chn_ctx)
{
    td_u32 busy_num, j;
    td_s32 ret;
    audio_frame_buf *buf = TD_NULL;
    audio_buf_blk *node = TD_NULL;

    /* 释放USER FB */
    buf = &chn_ctx->user_frm_buf;
    busy_num = audio_fb_query_busy_num(buf);
    for (j = 0; j < busy_num; j++) {
        node = audio_fb_get_busy(buf);
        ret = ai_sub_vb(&node->audio_frm, OT_VB_UID_AI);
        if (ret != TD_SUCCESS) {
            ai_err_trace("AI dev sub vb fail in ai release chn user fb\n");
        }

        ret = audio_fb_put_free(buf, node);
        if (ret != TD_SUCCESS) {
            ai_err_trace("audio fb put user free fail in ai release chn fb\n");
        }
    }

    /* 释放AIE FB */
    buf = &chn_ctx->aie_frm_buf;
    busy_num = audio_fb_query_busy_num(buf);
    for (j = 0; j < busy_num; j++) {
        node = audio_fb_get_busy(buf);
        ret = ai_sub_vb(&node->audio_frm, OT_VB_UID_AI);
        if (ret != TD_SUCCESS) {
            ai_err_trace("AI dev sub vb fail in ai release chn aie fb\n");
        }
        ret = audio_fb_put_free(buf, node);
        if (ret != TD_SUCCESS) {
            ai_err_trace("audio fb put aie free fail in ai release chn fb\n");
        }
    }
}

static td_s32 ai_disable_chn(ot_audio_dev aio_dev, ot_ai_chn ai_chn)
{
    aio_dev_ctx *dev_ctx = TD_NULL;
    ai_chn_ctx *chn_ctx = TD_NULL;
    td_s32 chn_id, i, chn_per_frm;
    unsigned long lock_flag;

    ai_check_func_entrance_return(OT_ID_AIO);
    ai_check_dev_return(aio_dev);
    ai_check_chn_id_return(ai_chn);
    dev_ctx = ai_get_dev_ctx(aio_dev);
    if (osal_down_interruptible(&dev_ctx->sem)) {
        return -ERESTARTSYS;
    }

    /* 如果设备当前没有使能则返回成功 */
    if (dev_ctx->enable != TD_TRUE) {
        osal_up(&dev_ctx->sem);
        return TD_SUCCESS;
    }
    /* chn_per_frm 只会为1或2，不会为0 */
    chn_per_frm = aio_chn_num_per_frm(dev_ctx->aio_attr.snd_mode);
    /* 如果设备当前不存在此通道则返回成功 */
    if (ai_chn >= ((td_s32)dev_ctx->aio_attr.chn_cnt / chn_per_frm)) {
        osal_up(&dev_ctx->sem);
        return TD_SUCCESS;
    }

    for (i = 0; i < chn_per_frm; i++) {
        ai_chn += ((td_s32)dev_ctx->aio_attr.chn_cnt / AI_STEREO_CHN_NUM) * i;
        chn_id = ai_get_chn_id(aio_dev, ai_chn);
        chn_ctx = ai_get_chn_ctx(chn_id);
        if (osal_down_interruptible(&chn_ctx->sem)) {
            osal_up(&dev_ctx->sem);
            return -ERESTARTSYS;
        }

        if ((chn_ctx->chn_state == AIO_CHN_DISABLE) || (chn_ctx->chn_state == AIO_CHN_ORIGINAL)) {
            chn_ctx->chn_state = AIO_CHN_DISABLE;
            osal_up(&chn_ctx->sem);
            osal_up(&dev_ctx->sem);
            return TD_SUCCESS;
        }

        aio_spin_lock_irqsave(&dev_ctx->spinlock, lock_flag);

        ai_release_chn_fb(chn_ctx);

        chn_ctx->chn_state = AIO_CHN_DISABLE;
        aio_spin_unlock_irqrestore(&dev_ctx->spinlock, lock_flag);

        chn_ctx->save_file_info.cfg = TD_FALSE;

        osal_wakeup(&chn_ctx->user_wait);
        osal_wakeup(&chn_ctx->aie_wait);
        osal_up(&chn_ctx->sem);
    }

    osal_up(&dev_ctx->sem);
    return TD_SUCCESS;
}

static td_s32 ai_enable_vqe(ot_audio_dev aio_dev, ot_ai_chn ai_chn_id, const ai_enable_aec *frame_combine)
{
    ai_check_dev_return(aio_dev);
    ai_check_chn_id_return(ai_chn_id);
    ai_check_null_ptr_return(frame_combine);

    ai_chn_ctx *chn_ctx = TD_NULL;
    ot_audio_dev ao_dev;
    unsigned long lock_flag;
    td_s32 chn_id = ai_get_chn_id(aio_dev, ai_chn_id);

    chn_ctx = ai_get_chn_ctx(chn_id);
    ao_dev = frame_combine->ao_dev;
    ao_check_dev_return(ao_dev);
    ao_check_chn_id_return(frame_combine->ao_chn);

    if (osal_down_interruptible(&chn_ctx->sem)) {
        return -ERESTARTSYS;
    }

    /* AI不使能时要返回错误，不允许用户这样操作 */
    if (chn_ctx->chn_state != AIO_CHN_ENABLE) {
        osal_up(&chn_ctx->sem);
        ai_err_trace("ai chn(%d,%d) not enable \n", aio_dev, ai_chn_id);
        return OT_ERR_AI_NOT_PERM;
    }

    /* 确认是否已指定AO */
    if (chn_ctx->aec_ao_dev != -1) {
        /* 多次启用，属性相同视为成功 */
        if ((chn_ctx->aec_ao_dev == frame_combine->ao_dev) && (chn_ctx->aec_ao_chn == frame_combine->ao_chn)) {
            osal_up(&chn_ctx->sem);
            return TD_SUCCESS;
        } else {
            osal_up(&chn_ctx->sem);
            ai_err_trace("ai chn(%d,%d) have enable aec already, but the attributes not the same as before\n",
                         aio_dev, ai_chn_id);
            return OT_ERR_AI_NOT_PERM;
        }
    }

    /* 将回声抵消参考的ao_dev ao_chn记录在通道上下文 */
    aio_spin_lock_irqsave(&(ai_get_dev_ctx(aio_dev)->spinlock), lock_flag);
    chn_ctx->aec_ao_dev = frame_combine->ao_dev;
    chn_ctx->aec_ao_chn = frame_combine->ao_chn;
    chn_ctx->vqe_enable = TD_TRUE;

    if (chn_ctx->aec_ao_dev != -1) {
        ao_export_func *pfn_ao_export_func = (ao_export_func *)cmpi_get_module_func_by_id(OT_ID_AO);
        if ((pfn_ao_export_func == TD_NULL) || (pfn_ao_export_func->pfn_set_ao_rd_offset_flag == TD_NULL)) {
            ai_warn_trace("AO has no export functions!\n");
        } else {
            pfn_ao_export_func->pfn_set_ao_rd_offset_flag(chn_ctx->aec_ao_dev, chn_ctx->aec_ao_chn);
        }
    }
    aio_spin_unlock_irqrestore(&(ai_get_dev_ctx(aio_dev)->spinlock), lock_flag);

    osal_up(&chn_ctx->sem);
    return TD_SUCCESS;
}

static td_s32 ai_disable_vqe(ot_audio_dev aio_dev, ot_ai_chn ai_chn_id)
{
    td_s32 chn_id;
    ai_chn_ctx *chn_ctx = TD_NULL;
    unsigned long lock_flag;

    ai_check_dev_return(aio_dev);
    ai_check_chn_id_return(ai_chn_id);

    chn_id = ai_get_chn_id(aio_dev, ai_chn_id);
    chn_ctx = ai_get_chn_ctx(chn_id);

    aio_spin_lock_irqsave(&(ai_get_dev_ctx(aio_dev)->spinlock), lock_flag);
    chn_ctx->aec_ao_dev = -1;
    chn_ctx->aec_ao_chn = -1;
    chn_ctx->vqe_enable = TD_FALSE;
    aio_spin_unlock_irqrestore(&(ai_get_dev_ctx(aio_dev)->spinlock), lock_flag);

    return TD_SUCCESS;
}

static td_s32 ai_get_pool_id(ot_audio_dev aio_dev_id, td_u32 *pool_id)
{
    aio_dev_ctx *dev_ctx = TD_NULL;

    ai_check_dev_return(aio_dev_id);
    ai_check_null_ptr_return(pool_id);

    dev_ctx = ai_get_dev_ctx(aio_dev_id);

    *pool_id = dev_ctx->pool_id;

    return TD_SUCCESS;
}

static td_s32 ai_get_pool_info(ot_audio_dev aio_dev_id, vb_pool_info *info)
{
    aio_dev_ctx *dev_ctx = TD_NULL;
    td_u32 pool_id;

    ai_check_dev_return(aio_dev_id);
    ai_check_null_ptr_return(info);

    dev_ctx = ai_get_dev_ctx(aio_dev_id);
    pool_id = dev_ctx->pool_id;
    if (pool_id != info->pool_id) {
        ai_err_trace("AI pool id does not match!\n");
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    return call_vb_get_pool_info(info);
}

static td_s32 ai_get_chn_param(ot_audio_dev aio_dev, ot_ai_chn ai_chn, ot_ai_chn_param *chn_param)
{
    aio_dev_ctx *dev_ctx = TD_NULL;
    ai_chn_ctx *chn_ctx = TD_NULL;
    td_s32 chn_id;

    ai_check_dev_return(aio_dev);
    ai_check_chn_id_return(ai_chn);
    ai_check_null_ptr_return(chn_param);

    dev_ctx = ai_get_dev_ctx(aio_dev);

    chn_id = ai_get_chn_id(aio_dev, ai_chn);
    chn_ctx = ai_get_chn_ctx(chn_id);

    if (osal_down_interruptible(&dev_ctx->sem)) {
        return -ERESTARTSYS;
    }

    /* check device is enable */
    if (dev_ctx->enable != TD_TRUE) {
        osal_up(&dev_ctx->sem);
        ai_err_trace("AI dev %d not enable!!\n", aio_dev);
        return OT_ERR_AI_NOT_ENABLED;
    }

    /* 立体声的用户音频帧buf的深度，分别记在左右声道的通道上下文中 aio_chn_num_per_frm获取的值 只会为1或2，不会为0 */
    if (ai_chn >= (ot_ai_chn)(((td_s32)dev_ctx->aio_attr.chn_cnt) / aio_chn_num_per_frm(dev_ctx->aio_attr.snd_mode))) {
        osal_up(&dev_ctx->sem);
        ai_err_trace("no aichn %d for aidev %d, or can't handle right channels for stereo mode!!\n",
                     ai_chn, aio_dev);
        return OT_ERR_AI_INVALID_CHN_ID;
    }

    chn_param->usr_frame_depth = chn_ctx->chn_param.usr_frame_depth;

    osal_up(&dev_ctx->sem);

    return TD_SUCCESS;
}

static td_s32 ai_set_chn_param(ot_audio_dev aio_dev, ot_ai_chn ai_chn, const ot_ai_chn_param *chn_param)
{
    aio_dev_ctx *dev_ctx = TD_NULL;
    ai_chn_ctx *chn_ctx = TD_NULL;
    td_s32 chn_id, i, chn_per_frm;

    ai_check_func_entrance_return(OT_ID_AIO);
    ai_check_dev_return(aio_dev);
    ai_check_chn_id_return(ai_chn);
    ai_check_null_ptr_return(chn_param);
    dev_ctx = ai_get_dev_ctx(aio_dev);
    if (osal_down_interruptible(&dev_ctx->sem)) {
        return -ERESTARTSYS;
    }
    /* check device is enable */
    if (dev_ctx->enable != TD_TRUE) {
        osal_up(&dev_ctx->sem);
        ai_err_trace("ai_set_chn_param AI dev %d not enable\n", aio_dev);
        return OT_ERR_AI_NOT_ENABLED;
    }

    /* 立体声的用户音频帧buf的深度，分别记在左右声道的通道上下文中 chn_per_frm 只会为1或2，不会为0 */
    chn_per_frm = aio_chn_num_per_frm(dev_ctx->aio_attr.snd_mode);
    if (ai_chn >= (ot_ai_chn)(((td_s32)dev_ctx->aio_attr.chn_cnt) / chn_per_frm)) {
        osal_up(&dev_ctx->sem);
        ai_err_trace("no aichn %d for aidev %d or can't handle right channels for stereo mode\n",
                     ai_chn, aio_dev);
        return OT_ERR_AI_INVALID_CHN_ID;
    }

    for (i = 0; i < chn_per_frm; i++) {
        ai_chn += ((td_s32)dev_ctx->aio_attr.chn_cnt / AI_STEREO_CHN_NUM) * i;
        chn_id = ai_get_chn_id(aio_dev, ai_chn);
        chn_ctx = ai_get_chn_ctx(chn_id);
        if (osal_down_interruptible(&chn_ctx->sem)) {
            osal_up(&dev_ctx->sem);
            return -ERESTARTSYS;
        }

        if ((chn_param->usr_frame_depth > OT_MAX_AI_USRFRM_DEPTH) ||
            (dev_ctx->aio_attr.frame_num < chn_param->usr_frame_depth) ||
            (chn_param->usr_frame_depth < 5)) { /* 5: min frame depth */
            ai_err_trace("the depth (%u) of user frame buf should [5, 30 or frame_num(%u)]!\n",
                         chn_param->usr_frame_depth, dev_ctx->aio_attr.frame_num);
            osal_up(&chn_ctx->sem);
            osal_up(&dev_ctx->sem);
            return OT_ERR_AI_ILLEGAL_PARAM;
        }

        chn_ctx->chn_param.usr_frame_depth = chn_param->usr_frame_depth;
        osal_up(&chn_ctx->sem);
    }

    osal_up(&dev_ctx->sem);
    return TD_SUCCESS;
}

td_s32 ai_get_frame_user_wait_condition_call_back(const td_void *param)
{
    ai_chn_ctx *chn_ctx = TD_NULL;
    td_s32 condition;

    if (param == TD_NULL) {
        return TD_FALSE;
    }

    chn_ctx = (ai_chn_ctx *)param;
    condition = (td_s32)((g_ai_state != AI_STATE_STARTED) ||
        ((audio_fb_query_busy_num(&chn_ctx->user_frm_buf) != 0) && (chn_ctx->chn_state != AIO_CHN_PAUSE)) ||
        ((chn_ctx->chn_state != AIO_CHN_ENABLE) && (chn_ctx->chn_state != AIO_CHN_PAUSE)));

    return condition;
}

td_s32 ai_get_frame_aie_wait_condition_call_back(const td_void *param)
{
    ai_chn_ctx *chn_ctx = TD_NULL;
    td_s32 condition;

    if (param == TD_NULL) {
        return TD_FALSE;
    }

    chn_ctx = (ai_chn_ctx *)param;
    condition = (td_s32)((g_ai_state != AI_STATE_STARTED) ||
        ((audio_fb_query_busy_num(&chn_ctx->aie_frm_buf) != 0) && (chn_ctx->chn_state != AIO_CHN_PAUSE)) ||
        ((chn_ctx->chn_state != AIO_CHN_ENABLE) && (chn_ctx->chn_state != AIO_CHN_PAUSE)));

    return condition;
}

static td_s32 check_mmz_phy_addr(td_u64 phys_addr, td_u64 addr_len)
{
    return TD_SUCCESS;
}

static td_void *ai_vb_phy2_vir(td_u64 phys_addr)
{
    return (td_void *)(td_uintptr_t)call_vb_handle_to_kern((call_vb_phy_to_handle((td_phys_addr_t)phys_addr)));
}

static td_s32 check_frame_addr_valid(const aio_dev_ctx *dev_ctx, const ot_audio_frame *frame)
{
    if (frame->len > (OT_MAX_AUDIO_POINT_BYTES * OT_MAX_AUDIO_POINT_NUM)) {
        ai_err_trace("len of frame is too large, size is:%u.\n", frame->len);
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    if (check_mmz_phy_addr(frame->phys_addr[0], frame->len) != TD_SUCCESS) {
        ai_err_trace("virt_addr[0](0x%llx) of frame is invalid\n", (td_u64)frame->phys_addr[0]);
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    if (frame->pool_id[0] != dev_ctx->pool_id) {
        ai_err_trace("pool_id[0](%u) of frame is not equal to %u\n",
                     frame->pool_id[0], dev_ctx->pool_id);
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    if (call_vb_is_blk_valid(frame->pool_id[0], frame->phys_addr[0]) != TD_TRUE) {
        ai_err_trace("frame error, invalid pool_id :%u, invalid addr:0x%llx\n",
                     frame->pool_id[0], (td_u64)frame->phys_addr[0]);
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    if (frame->snd_mode == OT_AUDIO_SOUND_MODE_STEREO) {
        if (check_mmz_phy_addr(frame->phys_addr[1], frame->len) != TD_SUCCESS) {
            ai_err_trace("virt_addr[1](0x%llx) of frm is invalid\n", (td_u64)frame->phys_addr[1]);
            return OT_ERR_AI_ILLEGAL_PARAM;
        }

        if (frame->pool_id[1] != dev_ctx->pool_id) {
            ai_err_trace("pool_id[1](%u) of frame is not equal to %u\n",
                         frame->pool_id[1], dev_ctx->pool_id);
            return OT_ERR_AI_ILLEGAL_PARAM;
        }

        if (call_vb_is_blk_valid(frame->pool_id[1], frame->phys_addr[1]) != TD_TRUE) {
            ai_err_trace("frame error, invalid pool_id :%u, invalid addr:0x%llx\n",
                         frame->pool_id[1], (td_u64)frame->phys_addr[1]);
            return OT_ERR_AI_ILLEGAL_PARAM;
        }
    }

    return TD_SUCCESS;
}

static td_s32 check_aie_frame(ot_audio_dev aio_dev, ot_ai_chn ai_chn, const audio_frame_combine *audio_frm)
{
    td_s32 ret, chn_id;
    aio_dev_ctx *dev_ctx = TD_NULL;
    ai_chn_ctx *chn_ctx = TD_NULL;

    ai_check_null_ptr_return(audio_frm);

    dev_ctx = ai_get_dev_ctx(aio_dev);

    chn_id = ai_get_chn_id(aio_dev, ai_chn);
    chn_ctx = ai_get_chn_ctx(chn_id);
    if (chn_ctx->chn_state != AIO_CHN_ENABLE) {
        return OT_ERR_AI_INVALID_CHN_ID;
    }

    ret = check_frame_addr_valid(dev_ctx, &(audio_frm->frm));
    if (ret != TD_SUCCESS) {
        return ret;
    }

    if (audio_frm->ref_frm.valid == TD_TRUE) {
        ret = check_frame_addr_valid(dev_ctx, &(audio_frm->ref_frm.ref_frame));
        if (ret != TD_SUCCESS) {
            return ret;
        }
    }

    return TD_SUCCESS;
}

static audio_buf_blk *get_aie_node(audio_frame_buf *buf, td_s32 chn_id)
{
    td_u32 i;
    td_u32 depth, busy_num;
    td_s32 ret;
    audio_buf_blk *node = TD_NULL;

    depth = ai_get_chn_ctx(chn_id)->chn_param.usr_frame_depth;
    busy_num = audio_fb_query_busy_num(buf);
    /* 出现卡顿时，可能存在多帧的情况，需要丢弃多余帧 */
    if (busy_num >= depth) {
        i = ((busy_num - depth) + 2); /* 2: extra farme */
        while (i--) {
            node = audio_fb_get_busy(buf);
            if (node == TD_NULL) {
                ai_err_trace("busy aie node of ai chn_id = %d is null.\n", chn_id);
                return TD_NULL;
            }

            ret = ai_sub_vb(&node->audio_frm, OT_VB_UID_AI);
            if (ret != 0) {
                ai_err_trace("ai sub vb fail in get aie node \n");
            }

            ret = audio_fb_put_free(buf, node);
            if (ret != 0) {
                ai_err_trace("fb put free in get aie node \n");
            }
        }
    }

    node = audio_fb_get_free(buf);

    return node;
}

static td_void update_aie_frame_virt_addr(audio_frame_combine *audio_frm)
{
    audio_frm->frm.virt_addr[0] = ai_vb_phy2_vir(audio_frm->frm.phys_addr[0]);

    if (audio_frm->frm.snd_mode == OT_AUDIO_SOUND_MODE_STEREO) {
        audio_frm->frm.virt_addr[1] = ai_vb_phy2_vir(audio_frm->frm.phys_addr[1]);
    }

    if (audio_frm->ref_frm.valid == TD_TRUE) {
        audio_frm->ref_frm.ref_frame.virt_addr[0] = ai_vb_phy2_vir(audio_frm->ref_frm.ref_frame.phys_addr[0]);

        if (audio_frm->frm.snd_mode == OT_AUDIO_SOUND_MODE_STEREO) {
            audio_frm->ref_frm.ref_frame.virt_addr[1] = ai_vb_phy2_vir(
                audio_frm->ref_frm.ref_frame.phys_addr[1]);
        }
    }
}

static td_void ai_call_sys_send(ot_audio_dev aio_dev, ot_ai_chn ai_chn, audio_frame_combine *audio_frm)
{
#ifndef CONFIG_OT_PLATFORM_V8
    ot_mpp_chn mpp_chn;
    call_sys_set_mpp_chn(&mpp_chn, OT_ID_AI, aio_dev, ai_chn);
    call_sys_send_data(&mpp_chn, TD_FALSE, MPP_DATA_AUDIO_FRAME, audio_frm);
#else
    call_sys_send_data(OT_ID_AI, aio_dev, ai_chn, TD_FALSE, MPP_DATA_AUDIO_FRAME, audio_frm);
#endif
}

static td_s32 ai_put_aie_frame(ot_audio_dev aio_dev, ot_ai_chn ai_chn, audio_frame_combine *audio_frm)
{
    td_s32 ret, chn_id;
    unsigned long lock_flag;
    audio_frame_buf *buf = TD_NULL;
    audio_buf_blk *node = TD_NULL;
    aio_dev_ctx *dev_ctx = TD_NULL;

    ai_check_dev_return(aio_dev);
    ai_check_chn_id_return(ai_chn);
    ai_check_null_ptr_return(audio_frm);

    if ((((td_bool)ckfn_sys_entry()) == TD_FALSE) || (((td_bool)ckfn_sys_send_data()) == TD_FALSE)) {
        ai_err_trace("sys module may not insert.\n");
        return OT_ERR_AI_NOT_READY;
    }

    ret = check_aie_frame(aio_dev, ai_chn, audio_frm);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    dev_ctx = ai_get_dev_ctx(aio_dev);
    chn_id = ai_get_chn_id(aio_dev, ai_chn);
    buf = &(ai_get_chn_ctx(chn_id)->aie_frm_buf);

    aio_spin_lock_irqsave(&dev_ctx->spinlock, lock_flag);
    node = get_aie_node(buf, chn_id);
    if (node == TD_NULL) {
        ai_err_trace("ai chn(%d,%d) aie node is null.\n", aio_dev, ai_chn);
        aio_spin_unlock_irqrestore(&dev_ctx->spinlock, lock_flag);
        return OT_ERR_AI_NULL_PTR;
    }

    update_aie_frame_virt_addr(audio_frm);

    ret = memcpy_s(&node->audio_frm, sizeof(node->audio_frm), audio_frm, sizeof(*audio_frm));
    if (ret != EOK) {
        ai_err_trace("ai_dev %d ai_chn %d audio_frm memcpy_s fail, ret = 0x%x.\n", aio_dev, ai_chn, (td_u32)ret);
        aio_spin_unlock_irqrestore(&dev_ctx->spinlock, lock_flag);
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    ret = audio_fb_put_busy(buf, node);
    if (ret != 0) {
        ai_warn_trace("fb put busy fail in ai put aie frame");
    }

    ret = ai_add_vb(audio_frm, OT_VB_UID_AI);
    if (ret != TD_SUCCESS) {
        ai_warn_trace("AI dev %d add vb fail\n", aio_dev);
    }
    ret = ai_sub_vb(audio_frm, OT_VB_UID_USER);
    if (ret != TD_SUCCESS) {
        ai_warn_trace("AI dev %d sub vb fail\n", aio_dev);
    }

    ai_call_sys_send(aio_dev, ai_chn, audio_frm);
    aio_spin_unlock_irqrestore(&dev_ctx->spinlock, lock_flag);

    /* 成功获取音频帧后唤醒等待队列 */
    osal_wakeup(&(ai_get_chn_ctx(chn_id)->aie_wait));

    return TD_SUCCESS;
}

static td_s32 check_get_frame_status(const aio_dev_ctx *dev_ctx, ot_audio_dev aio_dev, const ai_chn_ctx *chn_ctx,
    td_s32 chn_id, ot_ai_chn ai_chn)
{
    td_s32 chn_per_frm;

    /* check device is enable */
    if (dev_ctx->enable != TD_TRUE) {
        ai_err_trace("AI dev %d not enable\n", aio_dev);
        return OT_ERR_AI_NOT_ENABLED;
    }

    chn_per_frm = aio_chn_num_per_frm(dev_ctx->aio_attr.snd_mode);
    if (ai_chn >= (ot_ai_chn)(((td_s32)dev_ctx->aio_attr.chn_cnt) / chn_per_frm)) { /* chn_per_frm 只会为1或2，不会为0 */
        ai_err_trace("no aichn %d for aidev %d, or can't handle right channels for stereo mode\n", ai_chn, aio_dev);
        return OT_ERR_AI_INVALID_CHN_ID;
    }

    /* 通道状态必须是使能，或者暂停 */
    if ((chn_ctx->chn_state != AIO_CHN_ENABLE) && (chn_ctx->chn_state != AIO_CHN_PAUSE)) {
        ai_err_trace("AI chn %d state is not enable or pause\n", chn_id);
        return OT_ERR_AI_NOT_ENABLED;
    }
    if (chn_ctx->chn_param.usr_frame_depth == 0) {
        ai_err_trace("AI chn %d user frame depth is 0, please set it first\n", chn_id);
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

static td_s32 ai_wait_frame(const audio_frame_buf *buf, const aio_dev_ctx *dev_ctx, ai_chn_ctx *chn_ctx,
    const aio_frame *aio_frm, ai_frametype frame_type)
{
    td_s32 ret;
    osal_wait_t *wait = TD_NULL;
    fn_wait_call_bak fn_wait = TD_NULL;

    if (frame_type == AI_FRAMETYPE_USER) {
        wait = &chn_ctx->user_wait;
        fn_wait = ai_get_frame_user_wait_condition_call_back;
    } else {
        wait = &chn_ctx->aie_wait;
        fn_wait = ai_get_frame_aie_wait_condition_call_back;
    }

    while (audio_fb_query_busy_num(buf) == 0) {
        osal_up(&dev_ctx->sem);

        /*
         * 如果buf为空，就陷入等待队列中。
         * 当出现通道BUF不为空且通道不是暂停状态时，唤醒等待。
         * 当通道不是使能状态且通道不是暂停状态时(即通道停掉)，唤醒等待。
         * 即如果暂停状态，等待；如果使能状态而buf为空，等待。
         * 陷入等待时需要先释放互斥锁。
         */
        if (aio_frm->milli_sec == -1) {
            ret = osal_wait_event_interruptible(wait, fn_wait, (td_void *)chn_ctx);
            if (ret != TD_SUCCESS) {
                ai_warn_trace("state:%d\n", (td_s32)chn_ctx->chn_state);
                return -ERESTARTSYS;
            }
        } else if (aio_frm->milli_sec == 0) {
            ai_warn_trace("milli_sec = 0\n");
            return OT_ERR_AI_BUF_EMPTY;
        } else {
            ret = osal_wait_event_timeout_interruptible(wait, fn_wait, (td_void *)chn_ctx, aio_frm->milli_sec);
            if (ret < 0) {
                ai_warn_trace("state:%d\n", (td_s32)chn_ctx->chn_state);
                return -ERESTARTSYS;
            } else if (ret == 0) {
                ai_err_trace("frame buf is empty and timeout\n");
                return OT_ERR_AI_BUF_EMPTY;
            }
        }

        if (osal_down_interruptible(&dev_ctx->sem)) {
            return -ERESTARTSYS;
        }

        if (g_ai_state != AI_STATE_STARTED) {
            osal_up(&dev_ctx->sem);
            return OT_ERR_AI_NOT_READY;
        }

        /* 等待队列被唤醒后，检查是否是因为通道禁用而引起的唤醒 */
        if ((chn_ctx->chn_state != AIO_CHN_ENABLE) && (chn_ctx->chn_state != AIO_CHN_PAUSE)) {
            ai_warn_trace("AI chn not enabled\n");
            osal_up(&dev_ctx->sem);
            return OT_ERR_AI_NOT_ENABLED;
        }
    }

    return TD_SUCCESS;
}

static td_s32 get_frame_inner(const aio_dev_ctx *dev_ctx, aio_frame *aio_frm, audio_frame_buf *buf)
{
    audio_buf_blk *node = TD_NULL;
    unsigned long lock_flag;
    td_s32 ret;

    aio_spin_lock_irqsave(&dev_ctx->spinlock, lock_flag);
    node = audio_fb_get_busy(buf);
    if (node == TD_NULL) {
        ai_err_trace("busy list empty\n");
        aio_spin_unlock_irqrestore(&dev_ctx->spinlock, lock_flag);
        return OT_ERR_AI_BUF_EMPTY;
    }

    ret = memcpy_s(&aio_frm->audio_frm, sizeof(aio_frm->audio_frm), &node->audio_frm, sizeof(node->audio_frm));
    if (ret != EOK) {
        ai_err_trace("audio_frm memcpy_s fail, ret = 0x%x.\n", (td_u32)ret);
        aio_spin_unlock_irqrestore(&dev_ctx->spinlock, lock_flag);
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    ret = ai_add_vb(&aio_frm->audio_frm, OT_VB_UID_USER);
    if (ret != TD_SUCCESS) {
        ai_warn_trace("ai add vb fail in get frame inner\n");
    }
    ret = ai_sub_vb(&aio_frm->audio_frm, OT_VB_UID_AI);
    if (ret != TD_SUCCESS) {
        ai_warn_trace("AI dev sub vb fail\n");
    }

    ret = audio_fb_put_free(buf, node);
    if (ret != 0) {
        ai_warn_trace("fb put free fail in get frame inner\n");
    }

    aio_spin_unlock_irqrestore(&dev_ctx->spinlock, lock_flag);

    return TD_SUCCESS;
}

static td_s32 ai_get_frame(ot_audio_dev aio_dev, ot_ai_chn ai_chn, aio_frame *aio_frm, ai_frametype frame_type)
{
    td_s32 chn_id;
    td_s32 ret;
    ai_chn_ctx *chn_ctx = TD_NULL;
    audio_frame_buf *buf = TD_NULL;
    aio_dev_ctx *dev_ctx = TD_NULL;

    ai_check_func_entrance_return(OT_ID_AIO);
    ai_check_dev_return(aio_dev);
    ai_check_chn_id_return(ai_chn);
    ai_check_null_ptr_return(aio_frm);

    if (aio_frm->milli_sec < -1) {
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    dev_ctx = ai_get_dev_ctx(aio_dev);

    chn_id = ai_get_chn_id(aio_dev, ai_chn);
    chn_ctx = ai_get_chn_ctx(chn_id);

    if (osal_down_interruptible(&dev_ctx->sem)) {
        return -ERESTARTSYS;
    }

    ret = check_get_frame_status(dev_ctx, aio_dev, chn_ctx, chn_id, ai_chn);
    if (ret != TD_SUCCESS) {
        osal_up(&dev_ctx->sem);
        return ret;
    }

    if (frame_type == AI_FRAMETYPE_USER) {
        buf = &chn_ctx->user_frm_buf;
    } else {
        buf = &chn_ctx->aie_frm_buf;
    }

    /* wait frame会对锁进行操作，异常时内部会销毁锁，外部无需再销毁 */
    ret = ai_wait_frame(buf, dev_ctx, chn_ctx, aio_frm, frame_type);
    if (ret != TD_SUCCESS) {
        osal_up(&dev_ctx->sem);
        return ret;
    }

    ret = get_frame_inner(dev_ctx, aio_frm, buf);
    if (ret != TD_SUCCESS) {
        osal_up(&dev_ctx->sem);
        return ret;
    }

    if (frame_type == AI_FRAMETYPE_USER) {
        chn_ctx->aio_user_dbg.ai_get_cnt++;
    }

    osal_up(&dev_ctx->sem);
    return TD_SUCCESS;
}

static td_s32 ai_release_frame_check(ot_audio_dev ai_dev, ot_ai_chn ai_chn,
    const audio_frame_combine *audio_frm)
{
    if ((audio_frm->frm.snd_mode != OT_AUDIO_SOUND_MODE_MONO) &&
        (audio_frm->frm.snd_mode != OT_AUDIO_SOUND_MODE_STEREO)) {
        ai_err_trace("ai dev %d chn%d invalid frm.snd_mode = %d.\n", ai_dev,
            ai_chn, (td_s32)audio_frm->frm.snd_mode);
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    if ((audio_frm->ref_frm.valid != TD_TRUE) &&
        (audio_frm->ref_frm.valid != TD_FALSE)) {
        ai_err_trace("ai dev %d chn%d invalid ref_frm.valid = %d.\n", ai_dev,
            ai_chn, (td_s32)audio_frm->ref_frm.valid);
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

static td_s32 ai_release_frame_inner(ot_audio_dev aio_dev, ot_ai_chn ai_chn, const audio_frame_combine *audio_frm,
    td_s32 chn_per_frm)
{
    td_s32 i;
    const ot_audio_frame *frm_tmp = TD_NULL;

    for (i = 0; i < chn_per_frm; i++) {
        /* 此处不检查通道是否DISABLE，以防止用户disable_chn后释放frame的情况 */
        frm_tmp = &audio_frm->frm;
        if (call_vb_is_blk_valid(frm_tmp->pool_id[i], frm_tmp->phys_addr[i]) != TD_TRUE) {
            ai_err_trace("ERR!!! ai dev %d chn%d invalid addr:0x%llx\r\n",
                aio_dev, ai_chn, (td_u64)frm_tmp->phys_addr[i]);
            return OT_ERR_AI_ILLEGAL_PARAM;
        }

        if (audio_frm->ref_frm.valid == TD_TRUE) {
            frm_tmp = &audio_frm->ref_frm.ref_frame;
            if (call_vb_is_blk_valid(frm_tmp->pool_id[i], frm_tmp->phys_addr[i]) != TD_TRUE) {
                ai_err_trace("ERR!!! ai dev %d chn%d invalid addr:0x%llx\r\n",
                             aio_dev, ai_chn, (td_u64)frm_tmp->phys_addr[i]);
                return OT_ERR_AI_ILLEGAL_PARAM;
            }
        }
    }

    return TD_SUCCESS;
}

static td_s32 ai_release_frame(ot_audio_dev aio_dev, ot_ai_chn ai_chn, const audio_frame_combine *audio_frm)
{
    td_s32 chn_id, chn_per_frm, ret;
    aio_dev_ctx *dev_ctx = TD_NULL;
    ai_chn_ctx *chn_ctx = TD_NULL;

    ai_check_func_entrance_return(OT_ID_AIO);
    ai_check_dev_return(aio_dev);
    ai_check_chn_id_return(ai_chn);
    ai_check_null_ptr_return(audio_frm);

    dev_ctx = ai_get_dev_ctx(aio_dev);
    chn_id = ai_get_chn_id(aio_dev, ai_chn);
    chn_ctx = ai_get_chn_ctx(chn_id);

    ret = ai_release_frame_check(aio_dev, ai_chn, audio_frm);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    if (osal_down_interruptible(&dev_ctx->sem)) {
        return -ERESTARTSYS;
    }

    /* check device is enable */
    if (dev_ctx->enable != TD_TRUE) {
        osal_up(&dev_ctx->sem);
        ai_err_trace("AI dev %d not enable\n", aio_dev);
        return OT_ERR_AI_NOT_ENABLED;
    }
    /* chn_per_frm 只会为1或2，不会为0 */
    chn_per_frm = aio_chn_num_per_frm(audio_frm->frm.snd_mode);
    if (ai_chn >= (ot_ai_chn)(((td_s32)dev_ctx->aio_attr.chn_cnt) / chn_per_frm)) {
        osal_up(&dev_ctx->sem);
        ai_err_trace("no aichn %d for aidev %d, or can't handle right channels for stereo mode\n", ai_chn, aio_dev);
        return OT_ERR_AI_INVALID_CHN_ID;
    }

    if (ai_release_frame_inner(aio_dev, ai_chn, audio_frm, chn_per_frm) != TD_SUCCESS) {
        osal_up(&dev_ctx->sem);
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    if (ai_sub_vb(audio_frm, OT_VB_UID_USER) != TD_SUCCESS) {
        osal_up(&dev_ctx->sem);
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    if (dev_ctx->pool_id == audio_frm->frm.pool_id[0]) {
        chn_ctx->aio_user_dbg.ai_release_cnt++;
    }

    osal_up(&dev_ctx->sem);

    return TD_SUCCESS;
}

static td_s32 ai_set_chn_resmp_info(ot_audio_dev aio_dev, ot_ai_chn ai_chn, const aio_resmp_info *resmp_info)
{
    td_s32 chn_id;
    ai_chn_ctx *chn_ctx = TD_NULL;

    ai_check_dev_return(aio_dev);
    ai_check_chn_id_return(ai_chn);
    ai_check_null_ptr_return(resmp_info);

    chn_id = ai_get_chn_id(aio_dev, ai_chn);
    chn_ctx = ai_get_chn_ctx(chn_id);

    chn_ctx->resmp_dbg.resmp_enable = resmp_info->resmp_enable;
    chn_ctx->resmp_dbg.resmp_attr.in_point_num = resmp_info->resmp_attr.in_point_num;
    chn_ctx->resmp_dbg.resmp_attr.in_sample_rate = resmp_info->resmp_attr.in_sample_rate;
    chn_ctx->resmp_dbg.resmp_attr.out_sample_rate = resmp_info->resmp_attr.out_sample_rate;

    return TD_SUCCESS;
}

static td_s32 ai_get_chn_resmp_info(ot_audio_dev aio_dev, ot_ai_chn ai_chn, aio_resmp_info *resmp_info)
{
    td_s32 chn_id;
    ai_chn_ctx *chn_ctx = TD_NULL;

    ai_check_dev_return((td_s32)aio_dev);
    ai_check_chn_id_return(ai_chn);
    ai_check_null_ptr_return(resmp_info);

    chn_id = ai_get_chn_id(aio_dev, ai_chn);
    chn_ctx = ai_get_chn_ctx(chn_id);

    resmp_info->resmp_enable = chn_ctx->resmp_dbg.resmp_enable;
    resmp_info->resmp_attr.in_point_num = chn_ctx->resmp_dbg.resmp_attr.in_point_num;
    resmp_info->resmp_attr.in_sample_rate = chn_ctx->resmp_dbg.resmp_attr.in_sample_rate;
    resmp_info->resmp_attr.out_sample_rate = chn_ctx->resmp_dbg.resmp_attr.out_sample_rate;

    return TD_SUCCESS;
}

static td_s32 ai_set_chn_vqe_info(ot_audio_dev aio_dev, ot_ai_chn ai_chn, const ai_vqe_info *vqe_info)
{
    td_s32 chn_id;
    ai_chn_ctx *chn_ctx = TD_NULL;
    td_s32 ret;

    ai_check_dev_return(aio_dev);
    ai_check_chn_id_return(ai_chn);
    ai_check_null_ptr_return(vqe_info);

    chn_id = ai_get_chn_id(aio_dev, ai_chn);
    chn_ctx = ai_get_chn_ctx(chn_id);

    ret = memcpy_s(&chn_ctx->ai_vqe_dbg, sizeof(chn_ctx->ai_vqe_dbg), vqe_info, sizeof(*vqe_info));
    if (ret != EOK) {
        ai_err_trace("ai_dev %d ai_chn %d vqe_info memcpy_s fail, ret = 0x%x.\n", aio_dev, ai_chn, (td_u32)ret);
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

static td_s32 ai_set_chn_vqe_vol_info(ot_audio_dev aio_dev, ot_ai_chn ai_chn, const td_s32 *vqe_vol_info)
{
    td_s32 chn_id;
    ai_chn_ctx *chn_ctx = TD_NULL;

    ai_check_dev_return(aio_dev);
    ai_check_chn_id_return(ai_chn);
    ai_check_null_ptr_return(vqe_vol_info);

    chn_id = ai_get_chn_id(aio_dev, ai_chn);
    chn_ctx = ai_get_chn_ctx(chn_id);

    chn_ctx->ai_vqe_vol = *vqe_vol_info;
    return TD_SUCCESS;
}

static td_s32 ai_set_track_mode(ot_audio_dev audio_dev_id, ot_audio_track_mode track_mode)
{
    td_s32 ret;
    aio_dev_ctx *dev_ctx = TD_NULL;

    ai_check_func_entrance_return(OT_ID_AIO);
    ai_check_dev_return(audio_dev_id);

    dev_ctx = ai_get_dev_ctx(audio_dev_id);
    if (osal_down_interruptible(&dev_ctx->sem)) {
        return -ERESTARTSYS;
    }

    /* 设备没有使用时，不能设置track mode */
    if (dev_ctx->enable != TD_TRUE) {
        osal_up(&dev_ctx->sem);
        ai_err_trace("AI dev %d not enable\n", audio_dev_id);
        return OT_ERR_AI_NOT_ENABLED;
    }

    ret = ai_drv_set_track_mode(audio_dev_id, track_mode);

    osal_up(&dev_ctx->sem);
    return ret;
}

static td_s32 ai_get_track_mode(ot_audio_dev audio_dev_id, ot_audio_track_mode *track_mode)
{
    td_s32 ret;
    aio_dev_ctx *dev_ctx = TD_NULL;

    ai_check_func_entrance_return(OT_ID_AIO);
    ai_check_dev_return(audio_dev_id);

    dev_ctx = ai_get_dev_ctx(audio_dev_id);

    ai_check_null_ptr_return(track_mode);

    if (osal_down_interruptible(&dev_ctx->sem)) {
        return -ERESTARTSYS;
    }

    /* 设备没有使用时，不能设置track mode */
    if (dev_ctx->enable != TD_TRUE) {
        osal_up(&dev_ctx->sem);
        ai_err_trace("AI dev %d not enable\n", audio_dev_id);
        return OT_ERR_AI_NOT_ENABLED;
    }

    ret = ai_drv_get_track_mode(audio_dev_id, track_mode);

    osal_up(&dev_ctx->sem);
    return ret;
}

static td_s32 ai_set_clk_dir(ot_audio_dev audio_dev_id, audio_clkdir clk_dir)
{
    td_s32 ret;
    aio_dev_ctx *dev_ctx = TD_NULL;

    ai_check_func_entrance_return(OT_ID_AIO);
    ai_check_dev_return(audio_dev_id);

    dev_ctx = ai_get_dev_ctx(audio_dev_id);
    if (osal_down_interruptible(&dev_ctx->sem)) {
        return -ERESTARTSYS;
    }

    /* 设备设置属性前或使能后，不能设置clock dir */
    if ((dev_ctx->cfg == TD_FALSE) || (dev_ctx->enable == TD_TRUE)) {
        osal_up(&dev_ctx->sem);
        ai_err_trace("cfg:%d, enable:%d, AI dev %d cannot set clock direction\n", (td_s32)dev_ctx->cfg,
                     (td_s32)dev_ctx->enable, audio_dev_id);
        return OT_ERR_AI_NOT_PERM;
    }

    ret = ai_drv_set_clk_dir(audio_dev_id, clk_dir);

    osal_up(&dev_ctx->sem);
    return ret;
}

static td_s32 ai_enable_aec_ref_frame(ot_audio_dev aio_dev, ot_ai_chn ai_chn, const ai_enable_aec *frame_combine)
{
    ai_chn_ctx *chn_ctx = TD_NULL;
    unsigned long lock_flag;

    ai_check_dev_return(aio_dev);
    ai_check_chn_id_return(ai_chn);
    ai_check_null_ptr_return(frame_combine);
    ao_check_dev_return(frame_combine->ao_dev);
    ao_check_chn_id_return(frame_combine->ao_chn);

    td_s32 chn_id = ai_get_chn_id(aio_dev, ai_chn);
    chn_ctx = ai_get_chn_ctx(chn_id);
    if (osal_down_interruptible(&chn_ctx->sem)) {
        return -ERESTARTSYS;
    }

    /* AI不使能时要返回错误，不允许用户这样操作 */
    if (chn_ctx->chn_state != AIO_CHN_ENABLE) {
        osal_up(&chn_ctx->sem);
        ai_err_trace("ai chn(%d,%d) not enable \n", aio_dev, ai_chn);
        return OT_ERR_AI_NOT_PERM;
    }

    /* 确认是否已指定AO */
    if (chn_ctx->aec_ao_dev != -1) {
        /* 多次启用，属性相同视为成功 */
        if ((chn_ctx->aec_ao_dev == frame_combine->ao_dev) && (chn_ctx->aec_ao_chn == frame_combine->ao_chn)) {
            osal_up(&chn_ctx->sem);
            return TD_SUCCESS;
        } else {
            osal_up(&chn_ctx->sem);
            ai_err_trace("ai chn(%d,%d) have enable aec already,but the attributes not the same as before\n",
                aio_dev, ai_chn);
            return OT_ERR_AI_NOT_PERM;
        }
    }

    /* 将回声抵消参考的ao_dev ao_chn记录在通道上下文 */
    aio_spin_lock_irqsave(&(ai_get_dev_ctx(aio_dev)->spinlock), lock_flag);
    chn_ctx->aec_ao_dev = frame_combine->ao_dev;
    chn_ctx->aec_ao_chn = frame_combine->ao_chn;
    chn_ctx->enable_aec_ref_frame = TD_TRUE;
    aio_spin_unlock_irqrestore(&(ai_get_dev_ctx(aio_dev)->spinlock), lock_flag);

    osal_up(&chn_ctx->sem);
    return TD_SUCCESS;
}

static td_s32 ai_disable_aec_ref_frame(ot_audio_dev aio_dev, ot_ai_chn ai_chn)
{
    td_s32 chn_id;
    ai_chn_ctx *chn_ctx = TD_NULL;
    unsigned long lock_flag;

    ai_check_dev_return(aio_dev);
    ai_check_chn_id_return(ai_chn);

    chn_id = ai_get_chn_id(aio_dev, ai_chn);
    chn_ctx = ai_get_chn_ctx(chn_id);

    aio_spin_lock_irqsave(&(ai_get_dev_ctx(aio_dev)->spinlock), lock_flag);
    chn_ctx->aec_ao_dev = -1;
    chn_ctx->aec_ao_chn = -1;
    chn_ctx->enable_aec_ref_frame = TD_FALSE;
    aio_spin_unlock_irqrestore(&(ai_get_dev_ctx(aio_dev)->spinlock), lock_flag);

    return TD_SUCCESS;
}

static td_s32 ai_set_acodec_gain(td_s32 gain)
{
    return ai_drv_set_acodec_gain(gain);
}

static td_s32 ai_get_acodec_gain(td_s32 *gain)
{
    ai_check_null_ptr_return(gain);
    return ai_drv_get_acodec_gain(gain);
}

static td_s32 ai_get_clk_dir(ot_audio_dev audio_dev_id, audio_clkdir *clk_dir)
{
    td_s32 ret;
    aio_dev_ctx *dev_ctx = TD_NULL;

    ai_check_func_entrance_return(OT_ID_AIO);
    ai_check_dev_return(audio_dev_id);
    ai_check_null_ptr_return(clk_dir);

    dev_ctx = ai_get_dev_ctx(audio_dev_id);
    if (osal_down_interruptible(&dev_ctx->sem)) {
        return -ERESTARTSYS;
    }

    /* 设备设置属性前或使能后，不能获取clock dir */
    if ((dev_ctx->cfg == TD_FALSE) || (dev_ctx->enable == TD_TRUE)) {
        osal_up(&dev_ctx->sem);
        ai_err_trace("AI dev %d cannot get clock direction\n", audio_dev_id);
        return(td_s32) OT_ERR_AI_NOT_PERM;
    }

    ret = ai_drv_get_clk_dir(audio_dev_id, clk_dir);

    osal_up(&dev_ctx->sem);
    return ret;
}

#define AI_SAVE_FILE_MAXSIZE (10 * 1024) /* 10M */
#define AI_SAVE_FILE_MINSIZE 1           /* 1K */
static td_s32 ai_set_save_file_check(const ai_chn_ctx *chn_ctx, ot_ai_chn ai_chn,
    const ot_audio_save_file_info *save_file_info)
{
    if (chn_ctx->chn_state != AIO_CHN_ENABLE) {
        ai_err_trace("AI chn %d not enable\n", ai_chn);
        return OT_ERR_AI_NOT_ENABLED;
    }

    if ((save_file_info->cfg != TD_TRUE) && (save_file_info->cfg != TD_FALSE)) {
        ai_err_trace("AI chn%d save file cfg(%d) error, it should be 0 or 1.\n",
            ai_chn, (td_s32)save_file_info->cfg);
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    if ((chn_ctx->save_file_info.cfg == TD_TRUE) && (save_file_info->cfg == TD_TRUE)) {
        ai_err_trace("AI chn %d saving file has been configured\n", ai_chn);
        return OT_ERR_AI_NOT_PERM;
    }

    if (save_file_info->file_path[0] == '\0') {
        ai_err_trace("AI chn%d save file path can't be empty!\n", ai_chn);
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    if (osal_strnlen(save_file_info->file_path, OT_MAX_AUDIO_FILE_PATH_LEN) >= OT_MAX_AUDIO_FILE_PATH_LEN) {
        ai_err_trace("AI chn%d save file path strlen(%lu) is not less than %d!\n",
            ai_chn, osal_strnlen(save_file_info->file_path, OT_MAX_AUDIO_FILE_PATH_LEN), OT_MAX_AUDIO_FILE_PATH_LEN);
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    if (save_file_info->file_name[0] == '\0') {
        ai_err_trace("AI chn%d save file name can't be empty!\n", ai_chn);
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    if (osal_strnlen(save_file_info->file_name, OT_MAX_AUDIO_FILE_PATH_LEN) >= OT_MAX_AUDIO_FILE_NAME_LEN) {
        ai_err_trace("AI chn%d save file name strlen(%lu) is not less than %d!\n",
            ai_chn, osal_strnlen(save_file_info->file_name, OT_MAX_AUDIO_FILE_PATH_LEN), OT_MAX_AUDIO_FILE_NAME_LEN);
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    if ((save_file_info->file_size < AI_SAVE_FILE_MINSIZE) || (save_file_info->file_size > AI_SAVE_FILE_MAXSIZE)) {
        ai_err_trace("AI chn%d save file size(%u) error, it should between 1 and 10240.\n", ai_chn,
            save_file_info->file_size);
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

static td_s32 ai_check_if_no_sys_bind(ot_audio_dev aio_dev, ot_ai_chn ai_chn)
{
    ot_mpp_chn src_chn = {0};
    ot_mpp_bind_dst *bind_src = TD_NULL;
    td_s32 ret;

    src_chn.mod_id = OT_ID_AI;
    src_chn.dev_id = aio_dev;
    src_chn.chn_id = ai_chn;

    bind_src = (ot_mpp_bind_dst *)osal_vmalloc(sizeof(ot_mpp_bind_dst));
    if (bind_src == TD_NULL) {
        ai_err_trace("AI chn%d osal_vmalloc failed!\n", ai_chn);
        return OT_ERR_AI_NO_MEM;
    }
    (td_void)memset_s(bind_src, sizeof(ot_mpp_bind_dst), 0, sizeof(ot_mpp_bind_dst));

    if ((((td_bool)ckfn_sys_entry()) == TD_FALSE) || (((td_bool)ckfn_sys_get_bind_by_src()) == TD_FALSE)) {
        osal_vfree(bind_src);
        ai_err_trace("sys module may not insert.\n");
        return OT_ERR_AI_NOT_READY;
    }

    ret = call_sys_get_bind_by_src((hi_mpp_chn *)&src_chn, (hi_mpp_bind_dest *)bind_src);
    if ((ret == TD_SUCCESS) && (bind_src->num > 0)) {
        osal_vfree(bind_src);
        ai_err_trace("cannot save ai file when sys bind!\n");
        return OT_ERR_AI_NOT_PERM;
    }

    osal_vfree(bind_src);
    return TD_SUCCESS;
}

static td_s32 ai_set_save_file(ot_audio_dev aio_dev, ot_ai_chn ai_chn, const ot_audio_save_file_info *save_file_info)
{
    td_s32 chn_id;
    ai_chn_ctx *chn_ctx_temp = TD_NULL;
    td_s32 ret;

    ai_check_dev_return(aio_dev);
    ai_check_chn_id_return(ai_chn);
    ai_check_null_ptr_return(save_file_info);

    chn_id = ai_get_chn_id(aio_dev, ai_chn);
    chn_ctx_temp = ai_get_chn_ctx(chn_id);
    if (osal_down_interruptible(&chn_ctx_temp->sem)) {
        return -ERESTARTSYS;
    }

    ret = ai_set_save_file_check(chn_ctx_temp, ai_chn, save_file_info);
    if (ret != TD_SUCCESS) {
        osal_up(&chn_ctx_temp->sem);
        return ret;
    }

    /* ai_dump在系统绑定时不可用 */
    if (chn_ctx_temp->chn_attr.mode == OT_AI_CHN_MODE_FAST) {
        ret = ai_check_if_no_sys_bind(aio_dev, ai_chn);
        if (ret != TD_SUCCESS) {
            osal_up(&chn_ctx_temp->sem);
            return ret;
        }
    }
    if (chn_ctx_temp->ai_vqe_dbg.vqe_enable == TD_FALSE) {
        osal_up(&chn_ctx_temp->sem);
        ai_err_trace("cannot save ai file when vqe disabled!\n");
        return OT_ERR_AI_NOT_PERM;
    }

    ret = memcpy_s(&chn_ctx_temp->save_file_info, sizeof(chn_ctx_temp->save_file_info),
        save_file_info, sizeof(*save_file_info));
    if (ret != EOK) {
        ai_err_trace("ai_dev %d ai_chn %d save_file_info memcpy_s fail, ret = 0x%x.\n", aio_dev, ai_chn, (td_u32)ret);
        osal_up(&chn_ctx_temp->sem);
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    osal_up(&chn_ctx_temp->sem);
    return TD_SUCCESS;
}

static td_s32 ai_get_save_file(ot_audio_dev aio_dev, ot_ai_chn ai_chn, ot_audio_save_file_info *save_file_info)
{
    td_s32 chn_id;
    ai_chn_ctx *chn_ctx = TD_NULL;
    td_s32 ret;

    ai_check_dev_return(aio_dev);
    ai_check_chn_id_return(ai_chn);
    ai_check_null_ptr_return(save_file_info);

    chn_id = ai_get_chn_id(aio_dev, ai_chn);
    chn_ctx = ai_get_chn_ctx(chn_id);
    if (osal_down_interruptible(&chn_ctx->sem)) {
        return -ERESTARTSYS;
    }

    ret = memcpy_s(save_file_info, sizeof(*save_file_info),
        &chn_ctx->save_file_info, sizeof(chn_ctx->save_file_info));
    if (ret != EOK) {
        ai_err_trace("ai_dev %d ai_chn %d save_file_info memcpy_s fail, ret = 0x%x.\n", aio_dev, ai_chn, (td_u32)ret);
        osal_up(&chn_ctx->sem);
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    osal_up(&chn_ctx->sem);

    return TD_SUCCESS;
}

static td_s32 ai_get_chn_attr(ot_audio_dev aio_dev, ot_ai_chn ai_chn, ot_ai_chn_attr *chn_attr)
{
    aio_dev_ctx *dev_ctx = TD_NULL;
    ai_chn_ctx *chn_ctx = TD_NULL;
    td_s32 chn_id;

    ai_check_dev_return(aio_dev);
    ai_check_chn_id_return(ai_chn);
    ai_check_null_ptr_return(chn_attr);

    dev_ctx = ai_get_dev_ctx(aio_dev);

    chn_id = ai_get_chn_id(aio_dev, ai_chn);
    chn_ctx = ai_get_chn_ctx(chn_id);

    if (osal_down_interruptible(&dev_ctx->sem)) {
        return -ERESTARTSYS;
    }

    /* check device is enable */
    if (dev_ctx->enable != TD_TRUE) {
        osal_up(&dev_ctx->sem);
        ai_err_trace("AI dev %d not enable\n", aio_dev);
        return OT_ERR_AI_NOT_ENABLED;
    }

    /* 立体声的通道属性，分别记在左右声道的通道上下文中 aio_chn_num_per_frm 只会为1或2，不会为0 */
    if (ai_chn >= (ot_ai_chn)(((td_s32)dev_ctx->aio_attr.chn_cnt) / aio_chn_num_per_frm(dev_ctx->aio_attr.snd_mode))) {
        osal_up(&dev_ctx->sem);
        ai_err_trace("no aichn %d for aidev %d, or can't handle right channels for stereo mode\n",
                     ai_chn, aio_dev);
        return OT_ERR_AI_INVALID_CHN_ID;
    }

    chn_attr->mode = chn_ctx->chn_attr.mode;

    osal_up(&dev_ctx->sem);

    return TD_SUCCESS;
}

static td_s32 ai_set_chn_attr(ot_audio_dev aio_dev, ot_ai_chn ai_chn, const ot_ai_chn_attr *chn_attr)
{
    aio_dev_ctx *dev_ctx = TD_NULL;
    ai_chn_ctx *chn_ctx = TD_NULL;
    td_s32 i, chn_per_frm;

    ai_check_dev_return(aio_dev);
    ai_check_chn_id_return(ai_chn);
    ai_check_null_ptr_return(chn_attr);

    if (chn_attr->mode >= OT_AI_CHN_MODE_BUTT) {
        ai_err_trace("mode(%d) of ai chn(%d,%d) attr is wrong\n", (td_s32)chn_attr->mode, aio_dev, ai_chn);
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    dev_ctx = ai_get_dev_ctx(aio_dev);
    if (osal_down_interruptible(&dev_ctx->sem)) {
        return -ERESTARTSYS;
    }

    /* check device is enable */
    if (dev_ctx->enable != TD_TRUE) {
        osal_up(&dev_ctx->sem);
        ai_err_trace("AI dev %d not enable\n", aio_dev);
        return OT_ERR_AI_NOT_ENABLED;
    }

    /* 立体声的通道属性，分别记在左右声道的通道上下文中 chn_per_frm 只会为1或2，不会为0 */
    chn_per_frm = aio_chn_num_per_frm(dev_ctx->aio_attr.snd_mode);
    if (ai_chn >= (ot_ai_chn)(((td_s32)dev_ctx->aio_attr.chn_cnt) / chn_per_frm)) {
        osal_up(&dev_ctx->sem);
        ai_err_trace("no aichn %d for aidev %d or can't handle right channels for stereo mode\n", ai_chn, aio_dev);
        return OT_ERR_AI_INVALID_CHN_ID;
    }

    for (i = 0; i < chn_per_frm; i++) {
        ai_chn += (((td_s32)dev_ctx->aio_attr.chn_cnt) / AI_STEREO_CHN_NUM) * i;
        chn_ctx = ai_get_chn_ctx(ai_get_chn_id(aio_dev, ai_chn));
        if (chn_ctx == TD_NULL) {
            osal_up(&dev_ctx->sem);
            ai_err_trace("aidev %d aichn %d chn_ctx is null\n", aio_dev, ai_chn);
            return OT_ERR_AI_INVALID_CHN_ID;
        }

        if (osal_down_interruptible(&chn_ctx->sem)) {
            osal_up(&dev_ctx->sem);
            return -ERESTARTSYS;
        }

        if (chn_ctx->chn_state == AIO_CHN_ENABLE) {
            /* 通道属性须在使能通道前设置 */
            osal_up(&chn_ctx->sem);
            osal_up(&dev_ctx->sem);
            ai_err_trace("ai chn(%d,%d) is enable\n", aio_dev, ai_chn);
            return OT_ERR_AI_NOT_PERM;
        }

        chn_ctx->chn_attr.mode = chn_attr->mode;
        osal_up(&chn_ctx->sem);
    }

    osal_up(&dev_ctx->sem);
    return TD_SUCCESS;
}

static long ai_ioctl_adp_chn_vqe_cmd(unsigned int cmd, ot_audio_dev aio_dev, ot_ai_chn aio_chn, td_uintptr_t arg)
{
    td_slong ret;
    switch (cmd) {
        case AI_ENABLE_VQE:
            ret = ai_enable_vqe(aio_dev, aio_chn, (ai_enable_aec *)arg);
            break;
        case AI_DISABLE_VQE:
            ret = ai_disable_vqe(aio_dev, aio_chn);
            break;
        case SET_AI_CHN_RESMP:
            ret = ai_set_chn_resmp_info(aio_dev, aio_chn, (aio_resmp_info *)arg);
            break;
        case GET_AI_CHN_RESMP:
            ret = ai_get_chn_resmp_info(aio_dev, aio_chn, (aio_resmp_info *)arg);
            break;
        case SET_AI_CHN_VQE:
            ret = ai_set_chn_vqe_info(aio_dev, aio_chn, (ai_vqe_info *)arg);
            break;
        case SET_AI_CHN_VQEGAIN:
            ret = ai_set_chn_vqe_vol_info(aio_dev, aio_chn, (td_s32 *)arg);
            break;
        case SET_AI_SAVEFILE:
            ret = ai_set_save_file(aio_dev, aio_chn, (ot_audio_save_file_info *)arg);
            break;
        case GET_AI_SAVEFILE:
            ret = ai_get_save_file(aio_dev, aio_chn, (ot_audio_save_file_info *)arg);
            break;
        default:
            ai_err_trace("invalid ai cmd!\n");
            ret = TD_FAILURE;
    }

    return ret;
}

static long ai_ioctl_adp_chn_base_cmd(unsigned int cmd, ot_audio_dev aio_dev, ot_ai_chn aio_chn, td_uintptr_t arg)
{
    td_slong ret;
    switch (cmd) {
        case GET_AI_CHNPARAM:
            ret = ai_get_chn_param(aio_dev, aio_chn, (ot_ai_chn_param *)arg);
            break;
        case SET_AI_CHNPARAM:
            ret = ai_set_chn_param(aio_dev, aio_chn, (ot_ai_chn_param *)arg);
            break;
        case ENABLE_AI_CHN:
            ret = ai_enable_chn(aio_dev, aio_chn);
            break;
        case DISABLE_AI_CHN:
            ret = ai_disable_chn(aio_dev, aio_chn);
            break;
        case SET_AI_CHN_ATTR:
            ret = ai_set_chn_attr(aio_dev, aio_chn, (ot_ai_chn_attr *)arg);
            break;
        case GET_AI_CHN_ATTR:
            ret = ai_get_chn_attr(aio_dev, aio_chn, (ot_ai_chn_attr *)arg);
            break;
        default:
            ai_err_trace("invalid ai cmd!\n");
            ret = TD_FAILURE;
    }

    return ret;
}

static long ai_ioctl_adp_chn_cmd(unsigned int cmd, ot_audio_dev aio_dev, ot_ai_chn aio_chn, td_uintptr_t arg)
{
    td_slong ret;
    switch (cmd) {
        case GET_AI_CHNPARAM:
        case SET_AI_CHNPARAM:
        case ENABLE_AI_CHN:
        case DISABLE_AI_CHN:
        case SET_AI_CHN_ATTR:
        case GET_AI_CHN_ATTR:
            ret = ai_ioctl_adp_chn_base_cmd(cmd, aio_dev, aio_chn, arg);
            break;
        case GET_AI_FRAME:
            ret = ai_get_frame(aio_dev, aio_chn, (aio_frame *)arg, AI_FRAMETYPE_USER);
            break;
        case RELEASE_AI_FRAME:
            ret = ai_release_frame(aio_dev, aio_chn, (audio_frame_combine *)arg);
            break;
        case AI_ENABLE_AECREFFRAME:
            ret = ai_enable_aec_ref_frame(aio_dev, aio_chn, (ai_enable_aec *)arg);
            break;
        case AI_DISABLE_AECREFFRAME:
            ret = ai_disable_aec_ref_frame(aio_dev, aio_chn);
            break;
        case PUT_AI_AIE_FRAME:
            ret = ai_put_aie_frame(aio_dev, aio_chn, (audio_frame_combine *)arg);
            break;
        case GET_AI_AIE_FRAME:
            ret = ai_get_frame(aio_dev, aio_chn, (aio_frame *)arg, AI_FRAMETYPE_AIE);
            break;
        case AI_ENABLE_VQE:
        case AI_DISABLE_VQE:
        case SET_AI_CHN_RESMP:
        case GET_AI_CHN_RESMP:
        case SET_AI_CHN_VQE:
        case SET_AI_CHN_VQEGAIN:
        case SET_AI_SAVEFILE:
        case GET_AI_SAVEFILE:
            ret = ai_ioctl_adp_chn_vqe_cmd(cmd, aio_dev, aio_chn, arg);
            break;
        default:
            ai_err_trace("invalid ai cmd!\n");
            ret = TD_FAILURE;
    }

    return ret;
}

static long ai_ioctl_adp_base(unsigned int cmd, ot_audio_dev aio_dev, td_uintptr_t arg)
{
    td_slong ret;

    switch (cmd) {
        case SET_AI_ATTR:
            ret = ai_set_dev_attr(aio_dev, (ot_aio_attr *)arg);
            break;
        case GET_AI_ATTR:
            ret = ai_get_dev_attr(aio_dev, (ot_aio_attr *)arg);
            break;
        case ENABLE_AI_TRANS:
            ret = ai_enable_dev(aio_dev);
            break;
        case DISABLE_AI_TRANS:
            ret = ai_disable_dev(aio_dev, TD_FALSE);
            break;
        case GET_AI_POOLID:
            ret = ai_get_pool_id(aio_dev, (td_u32 *)arg);
            break;
        case GET_AI_VB_POOLINFO:
            ret = ai_get_pool_info(aio_dev, (vb_pool_info *)arg);
            break;
        case AI_SET_TRACK_MODE:
            ai_check_null_ptr_return(arg);
            ret = ai_set_track_mode(aio_dev, *(ot_audio_track_mode *)arg);
            break;
        case AI_GET_TRACK_MODE:
            ret = ai_get_track_mode(aio_dev, (ot_audio_track_mode *)arg);
            break;
        case AI_SET_CLKDIR:
            ai_check_null_ptr_return(arg);
            ret = ai_set_clk_dir(aio_dev, *(audio_clkdir *)arg);
            break;
        case AI_GET_CLKDIR:
            ret = ai_get_clk_dir(aio_dev, (audio_clkdir *)arg);
            break;
        case AI_CLR_ATTR:
            ret = ai_clr_dev_attr(aio_dev);
            break;
        case GET_AI_POOL_USER_CNT:
            ret = ai_get_pool_user_cnt(aio_dev, (td_u32 *)arg);
            break;
        default:
            ai_err_trace("invalid ai cmd!\n");
            ret = TD_FAILURE;
            break;
    }
    return ret;
}

static td_u32 ai_unused_cmd[] = {
    AI_SET_TRACK_MODE,
    AI_GET_TRACK_MODE,
    AI_SET_CLKDIR,
    AI_GET_CLKDIR,
    AI_CLR_ATTR,
    SET_AI_CHN_VQEGAIN
};

static long ai_ioctl_adp(unsigned int cmd, unsigned long ul_arg, void *private_data)
{
    td_s32 ret;
    td_u32 i;
    td_s32 chn_id = umap_get_chn(private_data);
    ot_audio_dev aio_dev = ai_get_dev_by_chn_id(chn_id);
    ot_ai_chn aio_chn = ai_get_chn_by_chn_id(chn_id);
    td_uintptr_t arg = (td_uintptr_t)ul_arg;
    td_s32 type_ai = TYPE_AI_NUM;
    td_bool flags = TD_TRUE;
#ifdef CONFIG_OT_AUDIO_ONLY_DRIVER_SUPPORT
    /* 只支持驱动的情况下无须开放ioctl，直接返回 */
    return TD_FAILURE;
#endif
    if (g_ai_state != AI_STATE_STARTED) {
        /* 双核时 从ARM在初始化时调用ai的接口 但此时系统可能并未初始化 dbg级别降低 */
        return OT_ERR_AI_NOT_READY;
    }

    for (i = 0; i < (sizeof(ai_unused_cmd) / sizeof(ai_unused_cmd[0])); i++) {
        if (cmd == ai_unused_cmd[i]) {
            ai_err_trace("invalid ai cmd!\n");
            return TD_FAILURE;
        }
    }

    switch (cmd) {
        case AI_BIND_CHANNEL2FD:
            ai_check_null_ptr_return(arg);
            umap_set_chn(private_data, *((hi_s32 *)arg));
            ret = TD_SUCCESS;
            break;
        case SET_AI_ATTR:
        case GET_AI_ATTR:
        case ENABLE_AI_TRANS:
        case DISABLE_AI_TRANS:
        case GET_AI_POOLID:
        case GET_AI_VB_POOLINFO:
        case AI_SET_TRACK_MODE:
        case AI_GET_TRACK_MODE:
        case AI_SET_CLKDIR:
        case AI_GET_CLKDIR:
        case AI_CLR_ATTR:
        case GET_AI_POOL_USER_CNT:
            ret = (td_s32)ai_ioctl_adp_base(cmd, aio_dev, arg);
            break;
        case AI_SET_ACODECGAIN:
            ai_check_null_ptr_return(arg);
            ret = ai_set_acodec_gain(*(td_s32 *)arg);
            break;
        case AI_GET_ACODECGAIN:
            ret = ai_get_acodec_gain((td_s32 *)arg);
            break;
        default:
            ret = (td_s32)ai_ioctl_adp_chn_cmd(cmd, aio_dev, aio_chn, arg);
    }

    return ret;
}

static unsigned int ai_poll(osal_poll_t *poll, void *data)
{
    unsigned long lock_flag;
    td_u32 mask = 0;
    td_u32 busy_num;
    td_s32 chn_id = umap_get_chn(data);
    ai_chn_ctx *chn_ctx = ai_get_chn_ctx(chn_id);
    aio_dev_ctx *dev_ctx = ai_get_dev_ctx(ai_get_dev_by_chn_id(chn_id));
    if ((chn_ctx == TD_NULL) || (dev_ctx == TD_NULL)) {
        return mask;
    }

    if ((dev_ctx->enable != TD_TRUE) || (chn_ctx->chn_state != AIO_CHN_ENABLE) ||
        (chn_ctx->chn_param.usr_frame_depth == 0)) {
        return mask;
    }

    if (chn_ctx->chn_attr.mode == OT_AI_CHN_MODE_NORMAL) {
        osal_poll_wait(poll, &chn_ctx->aie_wait);
    } else {
        osal_poll_wait(poll, &chn_ctx->user_wait);
    }

    aio_spin_lock_irqsave(&dev_ctx->spinlock, lock_flag);

    if (chn_ctx->chn_attr.mode == OT_AI_CHN_MODE_NORMAL) {
        busy_num = audio_fb_query_busy_num(&chn_ctx->aie_frm_buf);
    } else {
        busy_num = audio_fb_query_busy_num(&chn_ctx->user_frm_buf);
    }
    if (busy_num != 0) {
        mask |= OSAL_POLLIN | OSAL_POLLRDNORM; /* 可读取 */
    }

    aio_spin_unlock_irqrestore(&dev_ctx->spinlock, lock_flag);
    return mask;
}

static long ai_ioctl(unsigned int cmd, unsigned long arg, void *private_data)
{
    td_slong ret;

    if (check_is_same_pid(TD_FALSE) != TD_TRUE) {
        return OT_ERR_AI_NOT_PERM;
    }

    osal_atomic_inc_return(&g_user_ref);
    ret = ai_ioctl_adp(cmd, arg, private_data);
    osal_atomic_dec_return(&g_user_ref);

    return ret;
}

#ifdef CONFIG_COMPAT
static long ai_compat_ioctl(unsigned int cmd, unsigned long arg, void *private_data)
{
    long ret;

    ret = ai_ioctl(cmd, arg, private_data);

    return ret;
}
#endif

static struct osal_fileops g_ai_fops = {
    .module = THIS_MODULE,
    .open = ai_open,
    .release = ai_close,
    .unlocked_ioctl = ai_ioctl,
    .poll = ai_poll,
#ifdef CONFIG_COMPAT
    .compat_ioctl = ai_compat_ioctl
#endif
};

static td_s32 ai_ctx_init(ot_audio_dev audio_dev_id)
{
    td_u32 i;
    ai_chn_ctx *chn_ctx = TD_NULL;
    td_s32 ret;

    ai_check_dev_return(audio_dev_id);

    /* init DMA trans */
    ret = ai_dev_init(audio_dev_id);
    if (ret != TD_SUCCESS) {
        ai_warn_trace("ai_dev_init failed :%d error\n", audio_dev_id);
        return ret;
    }

    /* register all g_ai_umapd */
    for (i = (td_u32)(audio_dev_id * OT_AI_MAX_CHN_NUM); i < (td_u32)((audio_dev_id + 1) * OT_AI_MAX_CHN_NUM); i++) {
        chn_ctx = ai_get_chn_ctx((td_s32)i);
        if (chn_ctx == TD_NULL) {
            ai_err_trace("ai chn(%d,%u) ctx is null.\n", audio_dev_id, i - (td_u32)(audio_dev_id * OT_AI_MAX_CHN_NUM));
            return OT_ERR_AI_NOT_READY;
        }

        chn_ctx->dev_enable = TD_FALSE;
        /* 这段代码和ai_trans.c中的ai_enable_dev重复 */
        chn_ctx->aec_ao_dev = -1;
        chn_ctx->aec_ao_chn = -1;
    }

    return TD_SUCCESS;
}

static td_void ai_ctx_exit(ot_audio_dev audio_dev_id)
{
    /* if audio_dev_id >= OT_AI_DEV_MAX_NUM,return */
    if ((audio_dev_id < 0) || (audio_dev_id >= OT_AI_DEV_MAX_NUM)) {
        return;
    }

    /* release DMA trans */
    (td_void)ai_dev_exit(audio_dev_id);
}

static td_s32 ai_init(void *p)
{
    td_s32 ret, i;
    bind_sender_info sender_info = { 0 };
    ot_unused(p);

    if ((((td_bool)ckfn_sys_entry()) == TD_FALSE) || (((td_bool)ckfn_sys_register_sender()) == TD_FALSE)) {
        ai_err_trace("sys module may not insert.\n");
        return OT_ERR_AI_NOT_READY;
    }

    /* 向SYS模块注册发送端口 */
    sender_info.mod_id = OT_ID_AI;
    sender_info.max_dev_cnt = OT_AI_DEV_MAX_NUM;
    sender_info.max_chn_cnt = OT_AI_MAX_CHN_NUM;
    sender_info.give_bind_call_back = TD_NULL;
    ret = call_sys_register_sender(&sender_info);
    if (ret != TD_SUCCESS) {
        ai_err_trace("register sender failed with 0x%x!\n", (td_u32)ret);
        return OT_ERR_AI_NOT_READY;
    }

    return TD_SUCCESS;
}

static td_void ai_exit(void)
{
    td_s32 i;
    if ((((td_bool)(ckfn_sys_entry())) == TD_TRUE) && (((td_bool)ckfn_sys_unregister_sender()) == TD_TRUE)) {
        call_sys_unregister_sender(OT_ID_AI);
    }
}
#ifdef CONFIG_AUDIO_V200_SUPPORT
static td_void ai_notify(mod_notice_id notice)
{
    ot_unused(notice);
}
#else
static td_void ai_notify(mod_notice_id notice)
{
    td_s32 i;
    ot_unused(notice);
    for (i = 0; i < OT_AI_MAX_CHN_NUM * OT_AI_DEV_MAX_NUM; i++) {
        if (osal_down_interruptible(&(ai_get_chn_ctx(i)->sem)) == TD_SUCCESS) {
            if (ai_get_chn_ctx(i)->chn_state == AIO_CHN_ENABLE) {
                osal_wakeup(&(ai_get_chn_ctx(i)->user_wait));
                osal_wakeup(&(ai_get_chn_ctx(i)->aie_wait));
            }
            osal_up(&(ai_get_chn_ctx(i)->sem));
        }
    }
    return;
}
#endif
static td_void ai_query_state(mod_state *state)
{
    if (osal_atomic_read(&g_user_ref) == 0) {
        *state = MOD_STATE_FREE;
    } else {
        *state = MOD_STATE_BUSY;
    }
    return;
}

static td_s32 ai_disable_dev_with_force_exit(ot_audio_dev audio_dev_id)
{
    td_bool force_exit = TD_TRUE;
    return ai_disable_dev(audio_dev_id, force_exit);
}

static ai_export_func g_export_funcs = {
    .pfn_trans_init = ai_dev_init,
    .pfn_trans_begin = ai_enable_dev,
    .pfn_trans_stop = ai_disable_dev_with_force_exit,
    .pfn_trans_release = ai_dev_exit,

#ifdef CONFIG_OT_AUDIO_ALSA_SUPPORT
    /* alsa */
    .pfn_ai_set_alsa_dma_attr = ai_set_alsa_dma_attr,
    .pfn_ai_set_dev_attr = ai_set_dev_attr,
    .pfn_ai_set_dma_ext_isr = ai_drv_set_dma_ext_isr,
    .pfn_ai_reset_dma_ext_isr = ai_drv_reset_dma_ext_isr,
#else
    .pfn_ai_set_alsa_dma_attr = TD_NULL,
    .pfn_ai_set_dev_attr = TD_NULL,
    .pfn_ai_set_dma_ext_isr = TD_NULL,
    .pfn_ai_reset_dma_ext_isr = TD_NULL,
#endif
};

static td_u32 ai_get_ver_magic(td_void)
{
    return VERSION_MAGIC;
}

#ifdef CONFIG_OT_AUDIO_ONLY_DRIVER_SUPPORT
/* 只支持驱动的情况下无须受base模块控制init和exit，直接返回 */
static td_s32 ai_init_empty(void *p)
{
    return TD_SUCCESS;
}

static td_void ai_exit_empty(void)
{
    return;
}

static umap_module g_ai_module = {
    .mod_id = OT_ID_AI,
    .mod_name = "ai",

    .pfn_init = ai_init_empty,
    .pfn_exit = ai_exit_empty,
    .pfn_query_state = ai_query_state,
    .pfn_notify = ai_notify,
    .pfn_ver_checker = ai_get_ver_magic,

    .export_funcs = &g_export_funcs,
    .data = TD_NULL,
};
#else
/* 正常情况 */
static umap_module g_ai_module = {
    .mod_id = OT_ID_AI,
    .mod_name = "ai",

    .pfn_init = ai_init,
    .pfn_exit = ai_exit,
    .pfn_query_state = ai_query_state,
    .pfn_notify = ai_notify,
    .pfn_ver_checker = ai_get_ver_magic,

    .export_funcs = &g_export_funcs,
    .data = TD_NULL,
};
#endif

static td_s32 ai_init_umap_dev(td_void)
{
   /* init g_ai_umapd */
    g_ai_umapd = osal_createdev(UMAP_DEVNAME_AI_BASE);
    if (g_ai_umapd == TD_NULL) {
        ai_err_trace("ai create device failed \n");
        return TD_FAILURE;
    }
    g_ai_umapd->fops = &g_ai_fops;
    g_ai_umapd->minor = UMAP_AI_MINOR_BASE;

    if (osal_registerdevice(g_ai_umapd) != TD_SUCCESS) {
        ai_err_trace("ai register device failed \n");
        (td_void)osal_destroydev(g_ai_umapd);
        g_ai_umapd = TD_NULL;
        return TD_FAILURE;
    }

    if (cmpi_register_module(&g_ai_module)) {
        ai_err_trace("ai register module failed \n");
        osal_deregisterdevice(g_ai_umapd);
        (td_void)osal_destroydev(g_ai_umapd);
        g_ai_umapd = TD_NULL;
        return TD_FAILURE;
    }

#ifdef CONFIG_OT_AUDIO_ONLY_DRIVER_SUPPORT
    /* 只支持驱动的情况下需要模块自己调用init，之前是在注册模块时base调用的 */
    if (ai_init(TD_NULL) != TD_SUCCESS) {
        cmpi_unregister_module(OT_ID_AI);
        osal_deregisterdevice(g_ai_umapd);
        (td_void)osal_destroydev(g_ai_umapd);
        g_ai_umapd = TD_NULL;
        return TD_FAILURE;
    }
#endif

    return TD_SUCCESS;
}

static td_void ai_exit_umap_dev(td_void)
{
#ifdef CONFIG_OT_AUDIO_ONLY_DRIVER_SUPPORT
    ai_exit();
#endif
    cmpi_unregister_module(OT_ID_AI);
    osal_deregisterdevice(g_ai_umapd);
    (td_void)osal_destroydev(g_ai_umapd);
    g_ai_umapd = TD_NULL;
}

int ai_module_init(void)
{
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
    osal_proc_entry_t *sub_proc = TD_NULL;
#endif

    td_s32 ret;

    ret = ai_drv_check_aio_export_func();
    if (ret != TD_SUCCESS) {
        return ret;
    }

    /* create proc interface */
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
    sub_proc = osal_create_proc_entry(PROC_ENTRY_AI, TD_NULL);
    if (sub_proc == TD_NULL) {
        ai_info_trace("ai create proc failed\n");
        return TD_FAILURE;
    }
    sub_proc->read = ai_proc_show;
#endif

    /* init g_ai_umapd */
    ret = ai_init_umap_dev();
    if (ret != TD_SUCCESS) {
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
        osal_remove_proc_entry(PROC_ENTRY_AI, TD_NULL);
#endif
        return TD_FAILURE;
    }
    ret = osal_atomic_init(&g_user_ref);
    if (ret < 0) {
        ai_exit_umap_dev();
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
        osal_remove_proc_entry(PROC_ENTRY_AI, TD_NULL);
#endif
        ai_err_trace("osal atomic init fail in ai module init\n");
        return ret;
    }

    osal_atomic_set(&g_user_ref, 0);

    // 初始化ai初始化引用计数
    ret = osal_atomic_init(&g_ai_open_ref);
    if (ret < 0) {
        osal_atomic_destroy(&g_user_ref);
        ai_exit_umap_dev();
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
        osal_remove_proc_entry(PROC_ENTRY_AI, TD_NULL);
#endif
        ai_err_trace("ai open_ref atomic init failed\n");
        return TD_FAILURE;
    }
    osal_atomic_set(&g_ai_open_ref, 0);
    ret = osal_sema_init(&g_ai_sem, 1);
    if (ret < 0) {
        osal_atomic_destroy(&g_ai_open_ref);
        osal_atomic_destroy(&g_user_ref);
        ai_exit_umap_dev();
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
        osal_remove_proc_entry(PROC_ENTRY_AI, TD_NULL);
#endif
        ai_err_trace("ai osal g_ai_sem init fail\n");
        return TD_FAILURE;
    }
    ai_info_trace("load ai.ko ....OK!\n");

    return TD_SUCCESS;
}

void ai_module_exit(void)
{
    osal_sema_destroy(&g_ai_sem);
    osal_atomic_destroy(&g_ai_open_ref);
    osal_atomic_destroy(&g_user_ref);

    cmpi_unregister_module(OT_ID_AI);

    if (g_ai_umapd != TD_NULL) {
        osal_deregisterdevice(g_ai_umapd);
        (td_void)osal_destroydev(g_ai_umapd);
        g_ai_umapd = TD_NULL;
    }
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
    osal_remove_proc_entry(PROC_ENTRY_AI, TD_NULL);
#endif

    ai_info_trace("unload ai.ko ....OK!\n");
    return;
}
