/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2023. All rights reserved.
 * Description: ao module driver
 * Author: Hisilicon multimedia software group
 * Create: 2022/5/5
 */

#include "ao.h"
#include "hi_osal.h"
#include "securec.h"
#include "hi_comm_audio.h"
#include "dev_ext.h"
#include "ao_trans.h"
#include "ao_drv.h"
#include "mod_ext.h"
#include "ao_ext.h"
#include "proc_ext.h"
#include "mkp_aio.h"
#include "mkp_sys.h"
#include "sys_ext.h"
#include "aiao_hal_comm.h"
#include "pid_protect.h"
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
#include "ao_proc.h"
#endif

#ifdef CONFIG_OT_PLATFORM_V8
#include "ot_ao_export.h"
#else
#include "ao_exp.h"
#endif

#ifdef AUDIO_UT_VCAST
#include <linux/device.h>
#endif

#define AO_STATE_STARTED   0
#define AO_STATE_STOPING   1
#define AO_STATE_STOPED    2

#ifndef umap_get_chn
#define umap_get_chn UMAP_GET_CHN
#endif

#ifndef umap_set_chn
#define umap_set_chn UMAP_SET_CHN
#endif

static osal_dev_t *g_ao_umapd = NULL;
#define TYPE_AO_NUM 1
static osal_atomic_t g_ao_user_ref = OSAL_ATOMIC_INIT(0);
static td_u32 g_ao_state = AO_STATE_STOPED;
static osal_atomic_t g_ao_open_ref = OSAL_ATOMIC_INIT(0);
static td_s32 ao_ctx_init(ot_audio_dev audio_dev_id);
static td_void ao_ctx_exit(ot_audio_dev audio_dev_id);
aio_spin_lock_s g_ao_buf_lock;

static struct osal_semaphore g_ao_sem;

aio_spin_lock_s *ao_get_buf_lock(td_void)
{
    return &g_ao_buf_lock;
}

ot_ao_export_symbol g_ao_export_symbol = {0};

#ifdef CONFIG_OT_PLATFORM_V8
ot_ao_export_symbol *ot_ao_get_export_symbol(td_void)
{
    return &g_ao_export_symbol;
}
#endif

td_s32 ao_register_extern_call_back(ot_ao_export_callback *ao_export_callback)
{
    if (ao_export_callback == TD_NULL) {
        ao_err_trace("ao_export_callback is null.\n");
        return OT_ERR_AO_NULL_PTR;
    }

    (ao_drv_get_export_callback())->ao_notify = ao_export_callback->ao_notify;
    return TD_SUCCESS;
}

static td_void ao_dev_sem_do_destroy(ot_audio_dev dev_id)
{
    td_s32 i;
    for (i = OT_AO_DEV_MIN_NUM; i <= dev_id; i++) {
        ao_ctx_sem_exit(i);
    }
}

static td_void ao_ctx_do_destroy(ot_audio_dev dev_id)
{
    td_s32 i;
    for (i = OT_AO_DEV_MIN_NUM; i < dev_id; i++) {
        ao_ctx_exit(i);
    }
}

static int ao_open(void *private_data)
{
    int i, ret;

    if (osal_down(&g_ao_sem) != 0) {
        ao_err_trace("ao open semaphore down fail!\n");
        return TD_FAILURE;
    }

    if (osal_atomic_read(&g_ao_open_ref) >= MAX_OPEN_CNT) {
        osal_up(&g_ao_sem);
        return -1;
    }
    if (check_is_same_pid(TD_TRUE) != TD_TRUE) {
        osal_up(&g_ao_sem);
        return -1;
    }
    if (osal_atomic_inc_return(&g_ao_open_ref) != 1) {
        osal_up(&g_ao_sem);
        ao_debug_trace(" ao already initialization when first open\n");
        return TD_SUCCESS;
    }

    /* init dev context */
    for (i = OT_AO_DEV_MIN_NUM; i < OT_AO_DEV_MAX_NUM; i++) {
        ao_dev_sem_init(i);
        ret = ao_ctx_init(i);
        if (ret != TD_SUCCESS) {
            ao_ctx_do_destroy(i);
            ao_dev_sem_do_destroy(i);
            (void)osal_atomic_dec_return(&g_ao_open_ref);
            check_is_close();
            osal_up(&g_ao_sem);
            ao_err_trace("ao_ctx_init failed!\n");
            return -1;
        }
    }
    g_ao_state = AO_STATE_STARTED;
    osal_up(&g_ao_sem);
    return TD_SUCCESS;
}

static int ao_close(void *private_data)
{
    td_s32 i;
    if (osal_down(&g_ao_sem) != 0) {
        ao_err_trace("ao close semaphore down fail!\n");
        return TD_FAILURE;
    }
    /* exit dev context */
    if (osal_atomic_dec_return(&g_ao_open_ref) == 0) {
        for (i = 0; i < OT_AO_DEV_MAX_NUM; i++) {
            ao_ctx_exit(i);
            ao_ctx_sem_exit(i);
        }
        g_ao_state = AO_STATE_STOPED;
    }
    check_is_close();
    osal_up(&g_ao_sem);
    return TD_SUCCESS;
}

static td_void ao_frm_16bit_to_24bit(td_u8 *virt_addr, td_u32 point_num_per_frame)
{
    td_u16 *src_buf = (td_u16 *)(void *)virt_addr;
    td_u32 *dest_buf = (td_u32 *)(void *)virt_addr;
    td_s32 index;
    td_u32 temp;
    for (index = (td_s32)(point_num_per_frame - 1); index >= 0; index--) {
        temp = ((td_u32)(src_buf[index])) << 16; /* 16bit位宽数据左移16位成为24bit位宽数据（用4字节的高24bit储存） */
        dest_buf[index] = temp;
    }
}

static td_s32 ao_enable_chn_set_ctx(ot_audio_dev ao_dev, ot_ao_chn ao_chn)
{
    ao_chn_ctx *chn_ctx = TD_NULL;
    td_s32 chn_id;
    unsigned long lock_flag;

    chn_id = ao_get_chn_id(ao_dev, ao_chn);
    chn_ctx = ao_get_chn_ctx(chn_id);
    if (chn_ctx == TD_NULL) {
        ao_err_trace("ao_enable_chn_set_ctx ao_dev%d ao_chn%d ctx is null.\n", ao_dev, ao_chn);
        return OT_ERR_AO_INVALID_CHN_ID;
    }
    if (osal_down_interruptible(&chn_ctx->sem)) {
        return -ERESTARTSYS;
    }

    aio_spin_lock_irqsave(&g_ao_buf_lock, lock_flag);
    chn_ctx->chn_state = AIO_CHN_ENABLE;
    aio_spin_unlock_irqrestore(&g_ao_buf_lock, lock_flag);

    osal_up(&chn_ctx->sem);

    ao_info_trace("AO chn(%d,%d) enable ok!\n", ao_dev, ao_chn);

    return TD_SUCCESS;
}

static td_s32 ao_disable_chn_set_ctx(ot_audio_dev ao_dev, ot_ao_chn ao_chn, const aio_dev_ctx * const dev_ctx)
{
    ao_chn_ctx *chn_ctx = TD_NULL;
    td_s32 chn_id;
    unsigned long lock_flag;

    chn_id = ao_get_chn_id(ao_dev, ao_chn);

    chn_ctx = ao_get_chn_ctx(chn_id);
    if (chn_ctx == TD_NULL) {
        ao_err_trace("ao_dev%d ao_chn%d ctx is null.\n", ao_dev, ao_chn);
        return OT_ERR_AO_INVALID_CHN_ID;
    }
    if (osal_down_interruptible(&chn_ctx->sem)) {
        return -ERESTARTSYS;
    }
    if ((chn_ctx->chn_state == AIO_CHN_DISABLE) || (chn_ctx->chn_state == AIO_CHN_ORIGINAL)) {
        chn_ctx->chn_state = AIO_CHN_DISABLE;
        osal_up(&chn_ctx->sem);
        return TD_SUCCESS;
    }

    aio_spin_lock_irqsave(&g_ao_buf_lock, lock_flag);
    /* set disable state, and must wake the putframe proc */
    chn_ctx->chn_state = AIO_CHN_DISABLE;
    aio_spin_unlock_irqrestore(&g_ao_buf_lock, lock_flag);

    chn_ctx->save_file_info.cfg = TD_FALSE;

    osal_wakeup(&dev_ctx->wait);

    osal_up(&chn_ctx->sem);

    ao_info_trace("AO dev,chn(%d,%d) disable ok!\n", ao_dev, ao_chn);

    return TD_SUCCESS;
}

static td_s32 ao_enable_chn(ot_audio_dev aio_dev, ot_ao_chn ao_chn)
{
    aio_dev_ctx *dev_ctx = TD_NULL;
    td_s32 ret, i, chn_per_frm;
    ot_audio_snd_mode snd_mode;

    ao_check_func_entrance_return(OT_ID_AIO);
    ao_check_dev_return(aio_dev);
    ao_check_chn_id_return(ao_chn);

    dev_ctx = ao_get_dev_ctx(aio_dev);

    snd_mode = (ao_chn == OT_AO_SYS_CHN_ID) ? OT_AUDIO_SOUND_MODE_MONO : dev_ctx->aio_attr.snd_mode;

    if (osal_down_interruptible(&dev_ctx->sem)) {
        return -ERESTARTSYS;
    }

    /* 设备需要使能 */
    if (dev_ctx->enable != TD_TRUE) {
        osal_up(&dev_ctx->sem);
        ao_err_trace("AO dev %d not enable\n", aio_dev);
        return OT_ERR_AO_NOT_ENABLED;
    }

    if (ao_chn == OT_AO_SYS_CHN_ID) {
        ret = ao_enable_chn_set_ctx(aio_dev, ao_chn);
        if (ret != TD_SUCCESS) {
            osal_up(&dev_ctx->sem);
            return ret;
        }
    } else {
        chn_per_frm = aio_chn_num_per_frm(snd_mode);
        if (ao_chn >= ((td_s32)dev_ctx->aio_attr.chn_cnt / chn_per_frm)) { // chn_per_frm 为1或2，不会为0
            osal_up(&dev_ctx->sem);
            ao_err_trace("no aochn %d for aodev %d, or can't handle right channels for stereo mode\n",
                ao_chn, aio_dev);
            return OT_ERR_AO_INVALID_CHN_ID;
        }
        for (i = 0; i < chn_per_frm; i++) {
            ao_chn += (td_s32)(dev_ctx->aio_attr.chn_cnt / 2) * i; /* 2: stereo */
            ret = ao_enable_chn_set_ctx(aio_dev, ao_chn);
            if (ret != TD_SUCCESS) {
                osal_up(&dev_ctx->sem);
                return ret;
            }
        }
    }

    osal_up(&dev_ctx->sem);
    return TD_SUCCESS;
}

static td_s32 ao_disable_chn(ot_audio_dev aio_dev, ot_ao_chn ao_chn)
{
    td_s32 ret, i, chn_per_frm;
    aio_dev_ctx *dev_ctx = TD_NULL;
    ot_audio_snd_mode snd_mode;

    ao_check_func_entrance_return(OT_ID_AIO);
    ao_check_dev_return(aio_dev);
    ao_check_chn_id_return(ao_chn);

    dev_ctx = ao_get_dev_ctx(aio_dev);

    snd_mode = (ao_chn == OT_AO_SYS_CHN_ID) ? OT_AUDIO_SOUND_MODE_MONO : dev_ctx->aio_attr.snd_mode;

    if (osal_down_interruptible(&dev_ctx->sem)) {
        return -ERESTARTSYS;
    }
    chn_per_frm = aio_chn_num_per_frm(snd_mode);
    /* 如果设备当前不存在此通道则返回成功 */
    if (ao_chn >= ((td_s32)dev_ctx->aio_attr.chn_cnt / chn_per_frm)) { // chn_per_frm 为1或2，不会为0
        osal_up(&dev_ctx->sem);
        return TD_SUCCESS;
    }
    if (dev_ctx->enable != TD_TRUE) {
        osal_up(&dev_ctx->sem);
        return TD_SUCCESS;
    }

    for (i = 0; i < chn_per_frm; i++) {
        ao_chn += (td_s32)(dev_ctx->aio_attr.chn_cnt / 2) * i; /* 2: stereo */
        ret = ao_disable_chn_set_ctx(aio_dev, ao_chn, dev_ctx);
        if (ret != TD_SUCCESS) {
            osal_up(&dev_ctx->sem);
            return ret;
        }
    }

    osal_up(&dev_ctx->sem);
    return TD_SUCCESS;
}

static td_s32 ao_pause_chn(ot_audio_dev aio_dev, ot_ao_chn ao_chn)
{
    td_s32 chn_id;
    ao_chn_ctx *chn_ctx_temp = TD_NULL;
    aio_dev_ctx *dev_ctx = TD_NULL;
    td_s32 chn_per_frm;
    unsigned long lockflag;
    ot_audio_snd_mode snd_mode;

    ao_check_dev_return(aio_dev);
    ao_check_chn_id_return(ao_chn);

    chn_id = ao_get_chn_id(aio_dev, ao_chn);
    chn_ctx_temp = ao_get_chn_ctx(chn_id);
    dev_ctx = ao_get_dev_ctx(aio_dev);

    snd_mode = (ao_chn == OT_AO_SYS_CHN_ID) ? OT_AUDIO_SOUND_MODE_MONO : dev_ctx->aio_attr.snd_mode;

    chn_per_frm = aio_chn_num_per_frm(snd_mode);
    ao_check_chn_id_for_dev_return(ao_chn, aio_dev, dev_ctx->aio_attr.chn_cnt, chn_per_frm);

    if (osal_down_interruptible(&chn_ctx_temp->sem)) {
        return -ERESTARTSYS;
    }

    /* check AO chn is enable or pause state */
    if ((chn_ctx_temp->chn_state == AIO_CHN_DISABLE) || (chn_ctx_temp->chn_state == AIO_CHN_ORIGINAL)) {
        osal_up(&chn_ctx_temp->sem);
        ao_err_trace("when chn is disable, can not pause it\n");
        return OT_ERR_AO_NOT_PERM;
    }

    aio_spin_lock_irqsave(&g_ao_buf_lock, lockflag);
    chn_ctx_temp->chn_state = AIO_CHN_PAUSE;

    if (snd_mode == OT_AUDIO_SOUND_MODE_STEREO) {
        td_s32 smp_chn;
        ao_chn_ctx *other_chn_ctx_tmp = TD_NULL;
        if (ao_chn < (ot_ao_chn)(dev_ctx->aio_attr.chn_cnt / 2)) { /* 2: stereo */
            smp_chn = chn_id + (td_s32)(dev_ctx->aio_attr.chn_cnt / 2); /* 2: stereo */
        } else {
            smp_chn = chn_id - (td_s32)(dev_ctx->aio_attr.chn_cnt / 2); /* 2: stereo */
        }

        other_chn_ctx_tmp = ao_get_chn_ctx(smp_chn);
        if (other_chn_ctx_tmp == TD_NULL) {
            ao_err_trace("chn_id = %d ctx is null.\n", smp_chn);
            aio_spin_unlock_irqrestore(&g_ao_buf_lock, lockflag);
            osal_up(&chn_ctx_temp->sem);
            return OT_ERR_AO_NOT_READY;
        }

        other_chn_ctx_tmp->chn_state = AIO_CHN_PAUSE;
    }
    aio_spin_unlock_irqrestore(&g_ao_buf_lock, lockflag);

    osal_up(&chn_ctx_temp->sem);

    ao_info_trace("pause ao chn(%d,%d) ok\n", aio_dev, ao_chn);
    return TD_SUCCESS;
}

static td_s32 ao_resume_chn(ot_audio_dev aio_dev, ot_ao_chn ao_chn)
{
    td_s32 chn_id;
    ao_chn_ctx *chn_ctx = TD_NULL;
    ao_chn_ctx *right_chn_buf = TD_NULL;
    aio_dev_ctx *dev_ctx = TD_NULL;
    td_s32 chn_per_frm;
    unsigned long lockflag;
    ot_audio_snd_mode snd_mode;

    ao_check_dev_return(aio_dev);
    ao_check_chn_id_return(ao_chn);

    chn_id = ao_get_chn_id(aio_dev, ao_chn);
    chn_ctx = ao_get_chn_ctx(chn_id);
    dev_ctx = ao_get_dev_ctx(aio_dev);

    snd_mode = (ao_chn == OT_AO_SYS_CHN_ID) ? OT_AUDIO_SOUND_MODE_MONO : dev_ctx->aio_attr.snd_mode;

    chn_per_frm = aio_chn_num_per_frm(snd_mode);
    ao_check_chn_id_for_dev_return(ao_chn, aio_dev, dev_ctx->aio_attr.chn_cnt, chn_per_frm);

    if (osal_down_interruptible(&chn_ctx->sem)) {
        return -ERESTARTSYS;
    }
    /* check AO chn is enable or pause state */
    if ((chn_ctx->chn_state == AIO_CHN_DISABLE) || (chn_ctx->chn_state == AIO_CHN_ORIGINAL)) {
        osal_up(&chn_ctx->sem);
        ao_warn_trace("chn state is not pause or enable, can not resume it\n");
        return OT_ERR_AO_NOT_PERM;
    }

    aio_spin_lock_irqsave(&g_ao_buf_lock, lockflag);
    /* change chn state to enable, note:there is not resume state */
    if (snd_mode == OT_AUDIO_SOUND_MODE_STEREO) {
        ot_ao_chn ao_right_chn = ao_chn + (td_s32)(dev_ctx->aio_attr.chn_cnt / 2); /* 2: stereo */
        if (ao_right_chn < 0 || ao_right_chn >= OT_AO_MAX_CHN_NUM) {
            aio_spin_unlock_irqrestore(&g_ao_buf_lock, lockflag);
            osal_up(&chn_ctx->sem);
            ao_err_trace("ao_right_chn = %d is wrong in stereo mode.\n", ao_right_chn);
            return OT_ERR_AO_INVALID_CHN_ID;
        }

        right_chn_buf = ao_get_chn_ctx(ao_get_chn_id(aio_dev, ao_right_chn));
        right_chn_buf->chn_state = AIO_CHN_ENABLE;
    }
    chn_ctx->chn_state = AIO_CHN_ENABLE;
    aio_spin_unlock_irqrestore(&g_ao_buf_lock, lockflag);

    osal_wakeup(&dev_ctx->wait);

    osal_up(&chn_ctx->sem);

    ao_info_trace("resume ao(%d,%d) ok\n", aio_dev, ao_chn);
    return TD_SUCCESS;
}

static td_s32 ao_clear_chn_buf(ot_audio_dev aio_dev, ot_ao_chn ao_chn)
{
    td_s32 chn_id;
    ao_chn_ctx *chn_ctx = TD_NULL;
    aio_dev_ctx *dev_ctx = TD_NULL;
    td_s32 i, chn_per_frm;
    unsigned long lockflag;
    ot_audio_snd_mode snd_mode;

    ao_check_dev_return(aio_dev);
    ao_check_chn_id_return(ao_chn);

    dev_ctx = ao_get_dev_ctx(aio_dev);

    snd_mode = (ao_chn == OT_AO_SYS_CHN_ID) ? OT_AUDIO_SOUND_MODE_MONO : dev_ctx->aio_attr.snd_mode;

    chn_per_frm = aio_chn_num_per_frm(snd_mode);
    ao_check_chn_id_for_dev_return(ao_chn, aio_dev, dev_ctx->aio_attr.chn_cnt, chn_per_frm);

    for (i = 0; i < chn_per_frm; i++) {
        chn_id = ao_get_chn_id(aio_dev, ao_chn) + (td_s32)(dev_ctx->aio_attr.chn_cnt / 2) * i; /* 2: stereo */
        chn_ctx = ao_get_chn_ctx(chn_id);
        if (chn_ctx == TD_NULL) {
            ao_err_trace("ao chn(%d, %d) ctx is null.\n", aio_dev, ao_chn);
            return OT_ERR_AO_NOT_READY;
        }
        if (osal_down_interruptible(&chn_ctx->sem)) {
            return -ERESTARTSYS;
        }
        if (chn_ctx->dev_enable != TD_TRUE) {
            osal_up(&chn_ctx->sem);
            ao_err_trace("ao dev %d is not enable !\n", aio_dev);
            return OT_ERR_AO_NOT_ENABLED;
        }

        /* 通道未使能或暂停的情况下，返回出错 */
        if ((chn_ctx->chn_state != AIO_CHN_ENABLE) && (chn_ctx->chn_state != AIO_CHN_PAUSE)) {
            osal_up(&chn_ctx->sem);
            ao_err_trace("aochn %d of aodev %d is not enable !\n",
                ao_chn + (td_s32)(dev_ctx->aio_attr.chn_cnt / 2) * i, aio_dev); /* 2: stereo */
            return OT_ERR_AO_NOT_ENABLED;
        }

        /* 将读写指针都置为0 */
        aio_spin_lock_irqsave(&g_ao_buf_lock, lockflag);
        aio_buf_reset(&chn_ctx->buf);
        aio_spin_unlock_irqrestore(&g_ao_buf_lock, lockflag);
        /*
          * 必须唤醒发送音频帧的等待队列， 因为其可能正在阻塞状态中，无人将其唤醒;
          * 由于已经将缓冲区置为空，因此中断中一直没有数据，也就无法唤醒等待队列。
          */
        osal_wakeup(&dev_ctx->wait);

        osal_up(&chn_ctx->sem);
    }

    return TD_SUCCESS;
}

static td_s32 ao_get_chn_state(ot_audio_dev aio_dev, ot_ao_chn ao_chn, aio_chn_state *chn_state)
{
    td_s32 chn_id;
    ao_chn_ctx *chn_ctx = TD_NULL;

    ao_check_dev_return(aio_dev);
    ao_check_chn_id_return(ao_chn);
    ao_check_null_ptr_return(chn_state);

    chn_id = ao_get_chn_id(aio_dev, ao_chn);
    chn_ctx = ao_get_chn_ctx(chn_id);

    *chn_state = chn_ctx->chn_state;
    return TD_SUCCESS;
}

static td_s32 ao_query_chn_state(ot_audio_dev aio_dev, ot_ao_chn ao_chn, ot_ao_chn_state *status)
{
    td_s32 chn_id;
    ao_chn_ctx *chn_ctx = TD_NULL;
    unsigned long lockflag;

    ao_check_dev_return(aio_dev);
    ao_check_chn_id_return(ao_chn);
    ao_check_null_ptr_return(status);

    chn_id = ao_get_chn_id(aio_dev, ao_chn);
    chn_ctx = ao_get_chn_ctx(chn_id);
    if (osal_down_interruptible(&chn_ctx->sem)) {
        return -ERESTARTSYS;
    }

    if (chn_ctx->dev_enable != TD_TRUE) {
        osal_up(&chn_ctx->sem);
        ao_err_trace("ao dev %d is not enable.\n", aio_dev);
        return OT_ERR_AO_NOT_ENABLED;
    }

    /* 通道已经关闭或初始状态的情况下，返回出错 */
    if ((chn_ctx->chn_state == AIO_CHN_DISABLE) || (chn_ctx->chn_state == AIO_CHN_ORIGINAL)) {
        osal_up(&chn_ctx->sem);
        ao_err_trace("ao chn %d is disable or in original state, it's not permit to query chn state!\n",
            ao_chn);
        return OT_ERR_AO_NOT_ENABLED;
    }

    /* 获取通道buffer的空闲个数,使用个数，总个数 */
    aio_spin_lock_irqsave(&g_ao_buf_lock, lockflag);

    status->chn_total_num = chn_ctx->buf.pack_cnt;
    status->chn_free_num  = buf_free_count(&chn_ctx->buf);
    status->chn_busy_num  = buf_busy_count(&chn_ctx->buf);

    aio_spin_unlock_irqrestore(&g_ao_buf_lock, lockflag);

    osal_up(&chn_ctx->sem);

    return TD_SUCCESS;
}

static td_u32 ao_get_chn_buf_left_data_len(ot_audio_dev aio_dev, ot_ao_chn ao_chn)
{
    td_s32 chn_id;
    ao_chn_ctx *chn_ctx = TD_NULL;

    ao_check_dev_return(aio_dev);
    ao_check_chn_id_return(ao_chn);

    chn_id = ao_get_chn_id(aio_dev, ao_chn);
    chn_ctx = ao_get_chn_ctx(chn_id);

    return (chn_ctx->buf.pack_len * buf_busy_count(&chn_ctx->buf));
}

static td_s32 ao_get_byte_per_sample(ot_audio_bit_width bit_width, td_u32 *byte_per_sample)
{
    switch (bit_width) {
        case OT_AUDIO_BIT_WIDTH_8:
            *byte_per_sample = 1;
            break;

        case OT_AUDIO_BIT_WIDTH_16:
            *byte_per_sample = 4; /* 4: bytes */
            break;

        case OT_AUDIO_BIT_WIDTH_24:
            *byte_per_sample = 4; /* 4: bytes */
            break;

        default:
            ao_err_trace("invalid bit_width = %d.\n", (td_s32)bit_width);
            return OT_ERR_AO_ILLEGAL_PARAM;
    }
    return TD_SUCCESS;
}

static td_s32 ao_get_chn_delay(ot_audio_dev aio_dev, ot_ao_chn ao_chn, td_u32 *delay_ms)
{
    td_s32 ret;
    td_s32 chn_id;
    ao_chn_ctx *chn_ctx = TD_NULL;
    aio_dev_ctx *dev_ctx = TD_NULL;
    td_u32 chn_buf_left, dma_buf_left, byte_per_sample;

    ao_check_dev_return(aio_dev);
    ao_check_chn_id_return(ao_chn);
    ao_check_null_ptr_return(delay_ms);

    dev_ctx = ao_get_dev_ctx(aio_dev);

    chn_id = ao_get_chn_id(aio_dev, ao_chn);
    chn_ctx = ao_get_chn_ctx(chn_id);
    if (osal_down_interruptible(&chn_ctx->sem)) {
        return -ERESTARTSYS;
    }

    if (chn_ctx->dev_enable != TD_TRUE) {
        osal_up(&chn_ctx->sem);
        ao_err_trace("ao dev %d is not enable !\n", aio_dev);
        return OT_ERR_AO_NOT_ENABLED;
    }

    /* 通道已经关闭或初始状态的情况下，返回出错 */
    if ((chn_ctx->chn_state == AIO_CHN_DISABLE) || (chn_ctx->chn_state == AIO_CHN_ORIGINAL)) {
        osal_up(&chn_ctx->sem);
        ao_err_trace("ao chn %d is disable or in original state, it's not permit to query chn state!\n", ao_chn);
        return OT_ERR_AO_NOT_ENABLED;
    }

    /* 获取DMA buf的剩余数据量 */
    ret = ao_drv_get_dma_buf_left_len(aio_dev, &dma_buf_left);
    if (ret != TD_SUCCESS) {
        osal_up(&chn_ctx->sem);
        return ret;
    }

    /* 获取通道buf的剩余数据量 */
    chn_buf_left = ao_get_chn_buf_left_data_len(aio_dev, ao_chn);

    /* 获取单个采样点数据对应的字节数 */
    ret = ao_get_byte_per_sample(dev_ctx->aio_attr.bit_width, &byte_per_sample);
    if (ret != TD_SUCCESS) {
        osal_up(&chn_ctx->sem);
        return ret;
    }

    /* 计算delay */
    *delay_ms = ((chn_buf_left + dma_buf_left) * 1000) / /* 1000: 1s=1000ms */
        (byte_per_sample * (td_u32)dev_ctx->aio_attr.sample_rate);

    osal_up(&chn_ctx->sem);

    return TD_SUCCESS;
}

static td_s32 ao_put_frame_wait_condition_call_back(const td_void *param)
{
    ao_chn_ctx *chn_ctx = TD_NULL;
    td_s32 condition;

    if (param == TD_NULL) {
        return TD_FALSE;
    }

    chn_ctx = (ao_chn_ctx*)param;
    condition = (td_s32)(((!buf_is_full(&chn_ctx->buf)) && (chn_ctx->chn_state != AIO_CHN_PAUSE)) ||
                         ((chn_ctx->chn_state != AIO_CHN_ENABLE) && (chn_ctx->chn_state != AIO_CHN_PAUSE)) ||
                         (g_ao_state != AO_STATE_STARTED));

    return condition;
}

static td_s32 ao_get_put_frame_wait_condition(ot_audio_dev ao_dev, ot_ao_chn ao_chn, td_s32 milli_sec)
{
    td_s32 ret;
    td_s32 chn_id;
    aio_dev_ctx *dev_ctx = TD_NULL;
    ao_chn_ctx *chn_ctx = TD_NULL;

    chn_id = ao_get_chn_id(ao_dev, ao_chn);
    chn_ctx = ao_get_chn_ctx(chn_id);
    dev_ctx = ao_get_dev_ctx(ao_dev);

    if (milli_sec == -1) {
        ret = osal_wait_event_interruptible(&dev_ctx->wait,
            ao_put_frame_wait_condition_call_back, (td_void *)chn_ctx);
        if (ret != TD_SUCCESS) {
            return -ERESTARTSYS;
        }
    } else if (milli_sec == 0) {
        return OT_ERR_AO_BUF_FULL;
    } else {
        ret = osal_wait_event_timeout_interruptible(&dev_ctx->wait,
            ao_put_frame_wait_condition_call_back, (td_void *)chn_ctx, milli_sec);
        if (ret < 0) {
            return -ERESTARTSYS;
        } else if (ret == 0) {
            return OT_ERR_AO_BUF_FULL;
        }
    }

    return TD_SUCCESS;
}

static td_s32 ao_put_chn_frame(ot_audio_dev ao_dev, ot_ao_chn ao_chn, td_s32 milli_sec,
    const td_u8 *virt_addr, td_u32 len)
{
    td_s32 chn_id = ao_get_chn_id(ao_dev, ao_chn);
    ao_chn_ctx *chn_ctx = ao_get_chn_ctx(chn_id);
    audio_pack *pack = TD_NULL;
    unsigned long lockflag;
    td_s32 ret;

    if (chn_ctx == TD_NULL) {
        ao_err_trace("ao_dev%d ao_chn%d ctx is null.\n", ao_dev, ao_chn);
        return OT_ERR_AO_INVALID_CHN_ID;
    }

    if (osal_down_interruptible(&chn_ctx->sem)) {
        return -ERESTARTSYS;
    }

    /* 通道状态必须是使能，或者暂停 */
    if ((chn_ctx->chn_state != AIO_CHN_ENABLE) && (chn_ctx->chn_state != AIO_CHN_PAUSE)) {
        osal_up(&chn_ctx->sem);
        ao_warn_trace("AO(%d,%d) state is not enable or pause\n", ao_dev, ao_chn);
        return OT_ERR_AO_NOT_ENABLED;
    }

    /* 阻塞方式时，等到buf非空为止 */
    aio_spin_lock_irqsave(&g_ao_buf_lock, lockflag);
    while (buf_is_full(&chn_ctx->buf)) {
        aio_spin_unlock_irqrestore(&g_ao_buf_lock, lockflag);
        osal_up(&chn_ctx->sem);
        ret = ao_get_put_frame_wait_condition(ao_dev, ao_chn, milli_sec);
        if (ret != TD_SUCCESS) {
            return ret;
        }
        if (osal_down_interruptible(&chn_ctx->sem)) {
            return -ERESTARTSYS;
        }
        if (g_ao_state != AO_STATE_STARTED) {
            osal_up(&chn_ctx->sem);
            return OT_ERR_AO_NOT_READY;
        }

        if ((chn_ctx->chn_state != AIO_CHN_ENABLE) && (chn_ctx->chn_state != AIO_CHN_PAUSE)) {
            osal_up(&chn_ctx->sem);
            ao_err_trace("AO(%d,%d) state is not enable or pause\n", ao_dev, ao_chn);
            return OT_ERR_AO_NOT_ENABLED;
        }

        aio_spin_lock_irqsave(&g_ao_buf_lock, lockflag);
    }

    /* 获取通道buf的写指针 */
    pack = &(buf_get_write(&chn_ctx->buf));
    buf_finish_write(&chn_ctx->buf);
    aio_spin_unlock_irqrestore(&g_ao_buf_lock, lockflag);

    /* 由于copy_from_user不能在加自旋锁的情况下调用，故先更新写指针，再将数据拷入节点 */
    if (osal_copy_from_user(pack->data, (void *)virt_addr, len)) {
        osal_up(&chn_ctx->sem);
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

    osal_up(&chn_ctx->sem);
    return TD_SUCCESS;
}

static td_s32 ao_put_frame(ot_audio_dev aio_dev, ot_ao_chn ao_chn, const aio_frame *aio_frm)
{
    aio_dev_ctx *dev_ctx = TD_NULL;
    const ot_audio_frame *frm = TD_NULL;
    td_s32 i, chn_per_frm;
    td_u32 scr_point_num_per_frame;
    td_s32 ret;
    ot_audio_snd_mode snd_mode;
    td_u32 sample_per_bit;
    ao_check_dev_return(aio_dev);
    ao_check_chn_id_return(ao_chn);
    if (aio_frm == TD_NULL) {
        ao_err_trace("ot_audio_frame pointer argument for AO(%d,%d) is NULL.\n", aio_dev, ao_chn);
        return OT_ERR_AO_NULL_PTR;
    }

    frm = &aio_frm->audio_frm.frm;

    dev_ctx = ao_get_dev_ctx(aio_dev);

    snd_mode = (ao_chn == OT_AO_SYS_CHN_ID) ? OT_AUDIO_SOUND_MODE_MONO : dev_ctx->aio_attr.snd_mode;

    chn_per_frm = aio_chn_num_per_frm(snd_mode);
    ao_check_chn_id_for_dev_return(ao_chn, aio_dev, dev_ctx->aio_attr.chn_cnt, chn_per_frm);
    if (dev_ctx->aio_attr.bit_width != frm->bit_width) {
        ao_err_trace("AO(%d,%d) receive a frame(%d) which not match its bit_width(%d).\n",
            aio_dev, ao_chn, (td_s32)frm->bit_width, (td_s32)dev_ctx->aio_attr.bit_width);
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

    if (frm->len > (OT_MAX_AO_POINT_NUM * OT_MAX_AUDIO_POINT_BYTES)) {
        ao_err_trace("frm len ======, size is:0x%x\n", frm->len);
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

    sample_per_bit = aio_get_bit_cnt(frm->bit_width) / 8; /* 8: 1byte */
    scr_point_num_per_frame = frm->len / sample_per_bit;

    if (frm->bit_width == OT_AUDIO_BIT_WIDTH_16) {
        scr_point_num_per_frame = frm->len / 2; // 16bit src_point_num_prame 需要扩大2倍，适配底层硬件规格
    }
    if (dev_ctx->aio_attr.point_num_per_frame != scr_point_num_per_frame) {
        ao_err_trace("AO(%d,%d)frame which not match point_num_per_frame(%d to %d) frm->len = %d, bit_width = %d.\n",
                     aio_dev, ao_chn, (td_s32)scr_point_num_per_frame,
                     (td_s32)dev_ctx->aio_attr.point_num_per_frame, (td_s32)frm->len, (td_s32)frm->bit_width);
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

    for (i = 0; i < chn_per_frm; i++) {
        ao_chn += (td_s32)(dev_ctx->aio_attr.chn_cnt / 2) * i; /* 2: stereo */
        ret = ao_put_chn_frame(aio_dev, ao_chn, aio_frm->milli_sec, frm->virt_addr[i], frm->len);
        if (ret != TD_SUCCESS) {
            return ret;
        }
    }

    return TD_SUCCESS;
}

static td_s32 ao_set_chn_resmp_info(ot_audio_dev aio_dev, ot_ao_chn ao_chn, const aio_resmp_info *resmp_info)
{
    td_s32 chn_id;
    ao_chn_ctx *chn_ctx = TD_NULL;

    ao_check_dev_return(aio_dev);
    ao_check_chn_id_return(ao_chn);
    ao_check_null_ptr_return(resmp_info);

    chn_id = ao_get_chn_id(aio_dev, ao_chn);
    chn_ctx = ao_get_chn_ctx(chn_id);

    chn_ctx->resmp_dbg.resmp_enable = resmp_info->resmp_enable;
    chn_ctx->resmp_dbg.resmp_attr.in_point_num = resmp_info->resmp_attr.in_point_num;
    chn_ctx->resmp_dbg.resmp_attr.in_sample_rate = resmp_info->resmp_attr.in_sample_rate;
    chn_ctx->resmp_dbg.resmp_attr.out_sample_rate = resmp_info->resmp_attr.out_sample_rate;

    return TD_SUCCESS;
}

static td_s32 ao_get_chn_resmp_info(ot_audio_dev ao_dev, ot_ao_chn ao_chn, aio_resmp_info *resmp_info)
{
    td_s32 chn_id, ret;
    ao_chn_ctx *chn_ctx = TD_NULL;

    ao_check_dev_return(ao_dev);
    ao_check_chn_id_return(ao_chn);
    ao_check_null_ptr_return(resmp_info);

    chn_id = ao_get_chn_id(ao_dev, ao_chn);
    chn_ctx = ao_get_chn_ctx(chn_id);

    ret = memcpy_s(resmp_info, sizeof(*resmp_info), &(chn_ctx->resmp_dbg), sizeof(chn_ctx->resmp_dbg));
    if (ret != EOK) {
        ao_err_trace("ao(%d,%d) resmp_info memcpy_s fail, ret = 0x%x.\n", ao_dev, ao_chn, (td_u32)ret);
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

static td_s32 ao_set_chn_vqe_info(ot_audio_dev aio_dev, ot_ao_chn ao_chn, const ao_vqe_info *vqe_info)
{
    td_s32 chn_id;
    ao_chn_ctx *chn_ctx = TD_NULL;
    td_s32 ret;

    ao_check_chn_id_return(ao_chn);
    ao_check_dev_return(aio_dev);
    ao_check_null_ptr_return(vqe_info);

    chn_id = ao_get_chn_id(aio_dev, ao_chn);
    chn_ctx = ao_get_chn_ctx(chn_id);

    ret = memcpy_s(&chn_ctx->ao_vqe_dbg, sizeof(chn_ctx->ao_vqe_dbg), vqe_info, sizeof(*vqe_info));
    if (ret != EOK) {
        ao_warn_trace("ao_dev %d ao chn %d vqe_dbg memcpy_s fail, ret = 0x%x.\n", aio_dev, ao_chn, (td_u32)ret);
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

static unsigned int ao_poll(osal_poll_t *poll, void *data)
{
    td_u32 mask = 0;
    td_s32 chn_id = umap_get_chn(data);
    ao_chn_ctx *audio_chn = ao_get_chn_ctx(chn_id);
    aio_dev_ctx *dev_ctx = ao_get_dev_ctx(ao_get_dev_by_chn_id(chn_id));
    unsigned long lockflag;
    if ((audio_chn == TD_NULL) || (dev_ctx == TD_NULL)) {
        return mask;
    }
    if ((dev_ctx->enable != TD_TRUE) ||
        ((audio_chn->chn_state != AIO_CHN_ENABLE) && (audio_chn->chn_state != AIO_CHN_PAUSE))) {
            return mask;
        }
    osal_poll_wait(poll, &dev_ctx->wait);
    /* can write */
    aio_spin_lock_irqsave(&g_ao_buf_lock, lockflag);
    if (!buf_is_full(&audio_chn->buf)) {
        mask |= OSAL_POLLOUT | OSAL_POLLRDNORM;
    }
    aio_spin_unlock_irqrestore(&g_ao_buf_lock, lockflag);
    return mask;
}

static td_s32 ao_set_track_mode(ot_audio_dev aio_dev, ot_audio_track_mode track_mode)
{
    td_s32 ret;
    aio_dev_ctx *dev_ctx = TD_NULL;

    ao_check_func_entrance_return(OT_ID_AIO);
    ao_check_dev_return(aio_dev);
    dev_ctx = ao_get_dev_ctx(aio_dev);
    if (osal_down_interruptible(&dev_ctx->sem)) {
        return -ERESTARTSYS;
    }

    /* 设备没有使用时，不能设置track mode */
    if (dev_ctx->enable != TD_TRUE) {
        ao_err_trace("AO dev %d not enable\n", aio_dev);
        osal_up(&dev_ctx->sem);
        return OT_ERR_AO_NOT_ENABLED;
    }

    ret = ao_drv_set_track_mode(aio_dev, track_mode);

    osal_up(&dev_ctx->sem);
    return ret;
}

static td_s32 ao_get_track_mode(ot_audio_dev aio_dev, ot_audio_track_mode *track_mode)
{
    td_s32 ret;
    aio_dev_ctx *dev_ctx = TD_NULL;

    ao_check_func_entrance_return(OT_ID_AIO);
    ao_check_dev_return(aio_dev);
    ao_check_null_ptr_return(track_mode);

    dev_ctx = ao_get_dev_ctx(aio_dev);
    if (osal_down_interruptible(&dev_ctx->sem)) {
        return -ERESTARTSYS;
    }

    /* 设备没有使用时，不能设置track mode */
    if (dev_ctx->enable != TD_TRUE) {
        osal_up(&dev_ctx->sem);
        ao_err_trace("AO dev %d not enable\n", aio_dev);
        return OT_ERR_AO_NOT_ENABLED;
    }

    ret = ao_drv_get_track_mode(aio_dev, track_mode);

    osal_up(&dev_ctx->sem);
    return ret;
}

static td_s32 ao_set_volume(ot_audio_dev aio_dev, td_s32 volume_db)
{
    td_s32 ret;
    aio_dev_ctx *dev_ctx = TD_NULL;

    ao_check_func_entrance_return(OT_ID_AIO);
    ao_check_dev_return(aio_dev);
    dev_ctx = ao_get_dev_ctx(aio_dev);
    if (osal_down_interruptible(&dev_ctx->sem)) {
        return (-ERESTARTSYS);
    }

    /* 设备没有使用时，不能设置track mode */
    if (dev_ctx->enable != TD_TRUE) {
        osal_up(&dev_ctx->sem);
        ao_err_trace("AO dev %d not enable\n", aio_dev);
        return OT_ERR_AO_NOT_ENABLED;
    }

    ret = ao_drv_set_volume(aio_dev, volume_db);

    osal_up(&dev_ctx->sem);
    return ret;
}

static td_s32 ao_get_volume(ot_audio_dev aio_dev, td_s32 *volume_db)
{
    td_s32 ret;
    aio_dev_ctx *dev_ctx = TD_NULL;

    ao_check_func_entrance_return(OT_ID_AIO);
    ao_check_dev_return(aio_dev);
    ao_check_null_ptr_return(volume_db);

    dev_ctx = ao_get_dev_ctx(aio_dev);
    if (osal_down_interruptible(&dev_ctx->sem)) {
        return (-ERESTARTSYS);
    }

    /* 设备没有使用时，不能设置track mode */
    if (dev_ctx->enable != TD_TRUE) {
        osal_up(&dev_ctx->sem);
        ao_err_trace("AO dev %d not enable\n", aio_dev);
        return OT_ERR_AO_NOT_ENABLED;
    }

    ret = ao_drv_get_volume(aio_dev, volume_db);

    osal_up(&dev_ctx->sem);
    return ret;
}

static td_s32 ao_set_clk_dir(ot_audio_dev audio_dev_id, audio_clkdir clk_dir)
{
    td_s32 ret;
    aio_dev_ctx *dev_ctx = TD_NULL;

    ao_check_func_entrance_return(OT_ID_AIO);
    ao_check_dev_return(audio_dev_id);

    dev_ctx = ao_get_dev_ctx(audio_dev_id);
    if (osal_down_interruptible(&dev_ctx->sem)) {
        return -ERESTARTSYS;
    }

    /* 设备设置属性前或使能后，不能设置clock dir */
    if ((dev_ctx->cfg == TD_FALSE) || (dev_ctx->enable == TD_TRUE)) {
        osal_up(&dev_ctx->sem);
        ao_err_trace("AO dev %d cannot set clock direction\n", audio_dev_id);
        return OT_ERR_AO_NOT_PERM;
    }

    ret = ao_drv_set_clk_dir(audio_dev_id, clk_dir);

    osal_up(&dev_ctx->sem);
    return ret;
}

static td_s32 ao_get_clk_dir(ot_audio_dev audio_dev_id, audio_clkdir *clk_dir)
{
    td_s32 ret;
    aio_dev_ctx *dev_ctx = TD_NULL;

    ao_check_func_entrance_return(OT_ID_AIO);
    ao_check_dev_return(audio_dev_id);
    ao_check_null_ptr_return(clk_dir);

    dev_ctx = ao_get_dev_ctx(audio_dev_id);
    if (osal_down_interruptible(&dev_ctx->sem)) {
        return -ERESTARTSYS;
    }

    /* 设备设置属性前或使能后，不能获取clock dir */
    if ((dev_ctx->cfg == TD_FALSE) || (dev_ctx->enable == TD_TRUE)) {
        osal_up(&dev_ctx->sem);
        ao_err_trace("AO dev %d cannot get clock direction\n", audio_dev_id);
        return OT_ERR_AO_NOT_PERM;
    }

    ret = ao_drv_get_clk_dir(audio_dev_id, clk_dir);

    osal_up(&dev_ctx->sem);
    return ret;
}

static td_s32 ao_set_mute(ot_audio_dev aio_dev, const audio_mute *mute)
{
    td_s32 ret;
    aio_dev_ctx *dev_ctx = TD_NULL;

    ao_check_func_entrance_return(OT_ID_AIO);
    ao_check_dev_return(aio_dev);
    ao_check_null_ptr_return(mute);

    dev_ctx = ao_get_dev_ctx(aio_dev);
    if (osal_down_interruptible(&dev_ctx->sem)) {
        return -ERESTARTSYS;
    }

    /* 设备没有使用时，不能设置track mode */
    if (dev_ctx->enable != TD_TRUE) {
        ao_err_trace("AO dev %d not enable\n", aio_dev);
        osal_up(&dev_ctx->sem);
        return OT_ERR_AO_NOT_ENABLED;
    }

    ret = ao_drv_set_mute(aio_dev, mute->enable, &mute->fade);

    osal_up(&dev_ctx->sem);
    return ret;
}

#define  AO_SAVE_FILE_MAXSIZE     (10*1024) /* 10M */
#define  AO_SAVE_FILE_MINSIZE     1         /* 1K */
static td_s32 ao_set_save_file_check(const ao_chn_ctx * const chn_ctx, ot_ao_chn ao_chn,
    const ot_audio_save_file_info *save_file_info)
{
    if (chn_ctx->chn_state != AIO_CHN_ENABLE) {
        ao_err_trace("AO chn %d not enable\n", ao_chn);
        return OT_ERR_AO_NOT_ENABLED;
    }

    if ((save_file_info->cfg != TD_TRUE) && (save_file_info->cfg != TD_FALSE)) {
        ao_err_trace("AO chn%d save file cfg(%d) error, it should be 0 or 1.\n",
            ao_chn, (td_s32)save_file_info->cfg);
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

    if ((chn_ctx->save_file_info.cfg == TD_TRUE) && (save_file_info->cfg == TD_TRUE)) {
        ao_err_trace("AO chn %d saving file has been configured\n", ao_chn);
        return OT_ERR_AO_NOT_PERM;
    }

    if (save_file_info->file_path[0] == '\0') {
        ao_err_trace("AO chn%d save file path can't be empty!\n", ao_chn);
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

    if (osal_strnlen(save_file_info->file_path, OT_MAX_AUDIO_FILE_PATH_LEN) >= OT_MAX_AUDIO_FILE_PATH_LEN) {
        ao_err_trace("AO chn%d save file path strlen(%lu) is not less than %d!\n",
            ao_chn, osal_strnlen(save_file_info->file_path, OT_MAX_AUDIO_FILE_PATH_LEN),
            OT_MAX_AUDIO_FILE_PATH_LEN);
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

    if (save_file_info->file_name[0] == '\0') {
        ao_err_trace("AO chn%d save file name can't be empty!\n", ao_chn);
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

    if (osal_strnlen(save_file_info->file_name, OT_MAX_AUDIO_FILE_PATH_LEN) >= OT_MAX_AUDIO_FILE_NAME_LEN) {
        ao_err_trace("AO chn%d save file name strlen(%lu) is not less than %d!\n",
            ao_chn, osal_strnlen(save_file_info->file_name, OT_MAX_AUDIO_FILE_PATH_LEN),
            OT_MAX_AUDIO_FILE_NAME_LEN);
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

    if ((save_file_info->file_size < AO_SAVE_FILE_MINSIZE) || (save_file_info->file_size > AO_SAVE_FILE_MAXSIZE)) {
        ao_err_trace("AO chn%d save file size(%u) error, it should between 1 and 10240.\n",
            ao_chn, save_file_info->file_size);
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

static td_s32 ao_check_if_no_sys_bind(ot_audio_dev ao_dev, ot_ao_chn ao_chn)
{
    ot_mpp_chn src_chn = {0};
    ot_mpp_chn dest_chn = {0};
    td_s32 ret;

    /* ao_dump在系统绑定时不可用 */
    dest_chn.mod_id = OT_ID_AO;
    dest_chn.dev_id = ao_dev;
    dest_chn.chn_id = ao_chn;

    src_chn.mod_id = 0;
    if ((!ckfn_sys_entry()) || (!ckfn_sys_get_bind_by_dest())) {
        ao_err_trace("sys module may not insert.\n");
        return OT_ERR_AO_NOT_READY;
    }

    ret = call_sys_get_bind_by_dest((hi_mpp_chn *)&dest_chn, (hi_mpp_chn *)&src_chn);
    if ((ret == TD_SUCCESS) && (src_chn.mod_id == OT_ID_AI)) {
        ao_err_trace("cannot save ao file when sys bind!\n");
        return OT_ERR_AO_NOT_PERM;
    }

    return TD_SUCCESS;
}

static td_s32 ao_set_save_file(ot_audio_dev aio_dev, ot_ao_chn ao_chn, const ot_audio_save_file_info *save_file_info)
{
    td_s32 chn_id;
    ao_chn_ctx *chn_ctx = TD_NULL;
    td_s32 ret;

    ao_check_dev_return(aio_dev);
    ao_check_chn_id_return(ao_chn);
    ao_check_null_ptr_return(save_file_info);

    chn_id = ao_get_chn_id(aio_dev, ao_chn);
    chn_ctx = ao_get_chn_ctx(chn_id);
    if (osal_down_interruptible(&chn_ctx->sem)) {
        return -ERESTARTSYS;
    }

    ret = ao_set_save_file_check(chn_ctx, ao_chn, save_file_info);
    if (ret != TD_SUCCESS) {
        osal_up(&chn_ctx->sem);
        return ret;
    }

    /* ao_dump在系统绑定时不可用 */
    ret = ao_check_if_no_sys_bind(aio_dev, ao_chn);
    if (ret != TD_SUCCESS) {
        osal_up(&chn_ctx->sem);
        return ret;
    }

    if (chn_ctx->ao_vqe_dbg.vqe_enable == TD_FALSE) {
        osal_up(&chn_ctx->sem);
        ao_err_trace("cannot save ao file when vqe disabled!\n");
        return OT_ERR_AO_NOT_PERM;
    }

    ret = memcpy_s(&chn_ctx->save_file_info, sizeof(chn_ctx->save_file_info),
        save_file_info, sizeof(*save_file_info));
    if (ret != EOK) {
        osal_up(&chn_ctx->sem);
        ao_err_trace("ao_dev %d ao_chn %d save_file_info memcpy_s fail, ret = 0x%x.\n", aio_dev, ao_chn, (td_u32)ret);
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

    osal_up(&chn_ctx->sem);

    return TD_SUCCESS;
}

static td_s32 ao_get_save_file(ot_audio_dev aio_dev, ot_ao_chn ao_chn, ot_audio_save_file_info *save_file_info)
{
    td_s32 chn_id;
    ao_chn_ctx *chn_ctx = TD_NULL;
    td_s32 ret;

    ao_check_chn_id_return(ao_chn);
    ao_check_dev_return(aio_dev);
    ao_check_null_ptr_return(save_file_info);

    chn_id = ao_get_chn_id(aio_dev, ao_chn);
    chn_ctx = ao_get_chn_ctx(chn_id);
    if (osal_down_interruptible(&chn_ctx->sem)) {
        return -ERESTARTSYS;
    }

    ret = memcpy_s(save_file_info, sizeof(*save_file_info),
        &chn_ctx->save_file_info, sizeof(chn_ctx->save_file_info));
    if (ret != EOK) {
        ao_err_trace("ao_dev %d ao_chn %d save_file_info memcpy_s fail, ret = 0x%x.\n", aio_dev, ao_chn, (td_u32)ret);
        osal_up(&chn_ctx->sem);
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

    osal_up(&chn_ctx->sem);

    return TD_SUCCESS;
}

static td_s32 ao_get_mute(ot_audio_dev aio_dev, audio_mute *mute)
{
    td_s32 ret;
    aio_dev_ctx *dev_ctx = TD_NULL;

    ao_check_func_entrance_return(OT_ID_AIO);
    ao_check_null_ptr_return(mute);
    ao_check_dev_return(aio_dev);

    dev_ctx = ao_get_dev_ctx(aio_dev);
    if (osal_down_interruptible(&dev_ctx->sem)) {
        return (-ERESTARTSYS);
    }

    /* 设备没有使用时，不能设置track mode */
    if (dev_ctx->enable != TD_TRUE) {
        osal_up(&dev_ctx->sem);
        ao_err_trace("AO dev %d not enable\n", aio_dev);
        return OT_ERR_AO_NOT_ENABLED;
    }

    ret = ao_drv_get_mute(aio_dev, &mute->enable, &mute->fade);
    osal_up(&dev_ctx->sem);
    return ret;
}

static long check_support_ioctl(void)
{
#ifdef CONFIG_OT_AUDIO_ONLY_DRIVER_SUPPORT
    /* 只支持驱动的情况下无须开放ioctl，直接返回 */
    return TD_FAILURE;
#endif

    if (g_ao_state != AO_STATE_STARTED) {
        ao_warn_trace("ao_state is not STARTED \n");
        return (long)OT_ERR_AO_NOT_READY;
    }

    return TD_SUCCESS;
}

static long ioctl_process_chn(unsigned int cmd, ot_audio_dev aio_dev, ot_ao_chn aio_chn, td_uintptr_t arg)
{
    switch (cmd) {
        case PUT_AO_FRAME:
            return ao_put_frame(aio_dev, aio_chn, (aio_frame*)arg);
        case ENABLE_AO_CHN:
            return ao_enable_chn(aio_dev, aio_chn);
        case DISABLE_AO_CHN:
            return ao_disable_chn(aio_dev, aio_chn);
        case PAUSE_AO_CHN:
            return ao_pause_chn(aio_dev, aio_chn);
        case RESUME_AO_CHN:
            return ao_resume_chn(aio_dev, aio_chn);
        case AO_CLEAR_BUF_CTRL:
            return ao_clear_chn_buf(aio_dev, aio_chn);
        case AO_QUERY_CHN_STATE:
            return ao_query_chn_state(aio_dev, aio_chn, (ot_ao_chn_state*)arg);
        case SET_AO_CHN_RESMP:
            return ao_set_chn_resmp_info(aio_dev, aio_chn, (aio_resmp_info*)arg);
        case GET_AO_CHN_RESMP:
            return ao_get_chn_resmp_info(aio_dev, aio_chn, (aio_resmp_info *)arg);
        case SET_AO_CHN_VQE:
            return ao_set_chn_vqe_info(aio_dev, aio_chn, (ao_vqe_info*)arg);
        case SET_AO_SAVEFILE:
            return ao_set_save_file(aio_dev, aio_chn, (ot_audio_save_file_info*)arg);
        case GET_AO_SAVEFILE:
            return ao_get_save_file(aio_dev, aio_chn, (ot_audio_save_file_info*)arg);
        case AO_GET_CHN_STATE:
            return ao_get_chn_state(aio_dev, aio_chn, (aio_chn_state*)arg);
        case AO_GET_CHN_DELAY:
            return ao_get_chn_delay(aio_dev, aio_chn, (td_u32 *)arg);
        default:
            break;
    }

    return TD_FAILURE;
}

static long ioctl_adp_base(unsigned int cmd, ot_audio_dev aio_dev, td_uintptr_t arg)
{
    td_s32 ret;

    switch (cmd) {
        case SET_AO_ATTR:
            ret = ao_set_dev_attr(aio_dev, (ot_aio_attr*)arg);
            break;
        case GET_AO_ATTR:
            ret = ao_get_dev_attr(aio_dev, (ot_aio_attr*)arg);
            break;
        case ENABLE_AO_TRANS:
            ret = ao_enable_dev(aio_dev);
            break;
        case DISABLE_AO_TRANS:
            ret = ao_disable_dev(aio_dev);
            break;
        case AO_SET_TRACK_MODE:
            ao_check_null_ptr_return(arg);
            ret = ao_set_track_mode(aio_dev, *(ot_audio_track_mode*)arg);
            break;
        case AO_GET_TRACK_MODE:
            ret = ao_get_track_mode(aio_dev, (ot_audio_track_mode*)arg);
            break;
        case AO_SET_CLKDIR:
            ao_check_null_ptr_return(arg);
            ret = ao_set_clk_dir(aio_dev, *(audio_clkdir*)arg);
            break;
        case AO_GET_CLKDIR:
            ret = ao_get_clk_dir(aio_dev, (audio_clkdir*)arg);
            break;
        default:
            ao_err_trace("invalid ao cmd!\n");
            ret = TD_FAILURE;
            break;
    }
    return ret;
}

static td_u32 ao_unused_cmd[] = {
    AO_SET_TRACK_MODE,
    AO_GET_TRACK_MODE,
    AO_SET_CLKDIR,
    AO_GET_CLKDIR,
    AO_SET_VOLUME,
    AO_GET_VOLUME,
    AO_CLR_ATTR,
    PAUSE_AO_CHN,
    RESUME_AO_CHN
};

static long ao_ioctl_adp(unsigned int cmd, unsigned long ul_arg, void *private_data)
{
    long ret = 0;
    td_u32 i;
    td_s32 chn_id = umap_get_chn(private_data);
    ot_audio_dev aio_dev = ao_get_dev_by_chn_id(chn_id);
    ot_ao_chn aio_chn = ao_get_chn_by_chn_id(chn_id);
    td_uintptr_t arg = (td_uintptr_t)ul_arg;

    if (check_support_ioctl() != TD_SUCCESS) {
        return (long)OT_ERR_AO_NOT_READY;
    }

    for (i = 0; i < (sizeof(ao_unused_cmd) / sizeof(ao_unused_cmd[0])); i++) {
        if (cmd == ao_unused_cmd[i]) {
            ao_err_trace("invalid ao cmd!\n");
            return TD_FAILURE;
        }
    }

    switch (cmd) {
        case AO_BIND_CHANNEL2FD:
            ao_check_null_ptr_return(arg);
            umap_set_chn(private_data, *((hi_s32 *)arg));
            ret = TD_SUCCESS;
            break;
        case SET_AO_ATTR:
        case GET_AO_ATTR:
        case ENABLE_AO_TRANS:
        case DISABLE_AO_TRANS:
        case AO_SET_TRACK_MODE:
        case AO_GET_TRACK_MODE:
        case AO_SET_CLKDIR:
        case AO_GET_CLKDIR:
            ret = ioctl_adp_base(cmd, aio_dev, arg);
            break;
        case AO_SET_VOLUME:
            ao_check_null_ptr_return(arg);
            ret = ao_set_volume(aio_dev, *(td_s32*)arg);
            break;
        case AO_GET_VOLUME:
            ret = ao_get_volume(aio_dev, (td_s32*)arg);
            break;
        case AO_SET_MUTE:
            ret = ao_set_mute(aio_dev, (audio_mute*)arg);
            break;
        case AO_GET_MUTE:
            ret = ao_get_mute(aio_dev, (audio_mute*)arg);
            break;
        case AO_CLR_ATTR:
            ret = ao_clr_dev_attr(aio_dev);
            break;
        default:
            ret = ioctl_process_chn(cmd, aio_dev, aio_chn, arg);
            break;
    }
    return ret;
}

static long ao_ioctl(unsigned int cmd, unsigned long arg, void *private_data)
{
    long ret = 0;
    if (check_is_same_pid(TD_FALSE) != TD_TRUE) {
        return OT_ERR_AO_NOT_PERM;
    }
    osal_atomic_inc_return(&g_ao_user_ref);
    ret = ao_ioctl_adp(cmd, arg, private_data);
    osal_atomic_dec_return(&g_ao_user_ref);
    return ret;
}

#ifdef CONFIG_COMPAT
static long ao_compat_ioctl(unsigned int cmd, unsigned long arg, void *private_data)
{
    long ret;

    ret = ao_ioctl(cmd, arg, private_data);

    return ret;
}
#endif

static struct osal_fileops g_ao_fops = {
    .module = THIS_MODULE,
    .open       = ao_open,
    .release    = ao_close,
#ifdef TEST_AO_UNDER_RUN
    .write      = ao_write,
#endif
    .unlocked_ioctl      = ao_ioctl,
    .poll       = ao_poll,
#ifdef CONFIG_COMPAT
    .compat_ioctl    = ao_compat_ioctl
#endif
};

static td_s32 ao_ctx_init(ot_audio_dev audio_dev_id)
{
    td_s32 i;
    ao_chn_ctx *chn_ctx = TD_NULL;
    td_s32 ret;

    ao_check_dev_return(audio_dev_id);

    /* init DMA trans */
    ret = ao_dev_init(audio_dev_id);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    /* register all g_ao_umapd */
    for (i = audio_dev_id * OT_AO_MAX_CHN_NUM; i < (audio_dev_id + 1) * OT_AO_MAX_CHN_NUM; i++) {
        chn_ctx = ao_get_chn_ctx(i);
        if (chn_ctx == TD_NULL) {
            ao_err_trace("ao chn(%d,%d) ctx is null.\n", audio_dev_id, i - (audio_dev_id * OT_AO_MAX_CHN_NUM));
            return OT_ERR_AO_NOT_READY;
        }

        chn_ctx->dev_enable = TD_FALSE;
    }

    return TD_SUCCESS;
}

static td_void ao_ctx_exit(ot_audio_dev audio_dev_id)
{
    (void)ao_dev_exit(audio_dev_id);
}

static td_s32 ao_init(void *p)
{
    ot_unused(p);

    (td_void)aio_spin_lock_init(&g_ao_buf_lock);
    if (!(ckfn_sys_entry())) {
        ao_err_trace("sys module may not insert.\n");
        return OT_ERR_AO_NOT_READY;
    }

    return TD_SUCCESS;
}

static td_void ao_exit(void)
{
    aio_spin_lock_destroy(&g_ao_buf_lock);
}
#ifdef CONFIG_AUDIO_V200_SUPPORT
static td_void ao_notify(mod_notice_id notice)
{
    ot_unused(notice);
}
#else
static td_void ao_notify(mod_notice_id notice)
{
    td_s32 i;
    aio_dev_ctx *aio_dev = TD_NULL;
    ot_unused(notice);
    for (i = 0; i < OT_AO_DEV_MAX_NUM; i++) {
        aio_dev = ao_get_dev_ctx(i);
        if (osal_down_interruptible(&aio_dev->sem) == TD_SUCCESS) {
            if (ao_get_dev_ctx(i)->inited == TD_TRUE) {
                osal_wakeup(&aio_dev->wait);
            }
            osal_up(&aio_dev->sem);
        } else {
        }
    }
    return;
}
#endif
static td_void ao_query_state(mod_state *state)
{
    if (osal_atomic_read(&g_ao_user_ref) == 0) {
        *state = MOD_STATE_FREE;
    } else {
        *state = MOD_STATE_BUSY;
    }
    return;
}

static td_u32 ao_get_ver_magic(td_void)
{
    return VERSION_MAGIC;
}

static ao_export_func g_export_funcs = {
    .pfn_trans_init       = ao_dev_init,
    .pfn_trans_begin      = ao_enable_dev,
    .pfn_trans_stop       = ao_disable_dev,
    .pfn_trans_release    = ao_dev_exit,
    .pfn_get_dma_inf       = ao_get_dev_ctx,
    .pfn_get_data         = ao_get_data,
    .pfn_save_ao_rdptr     = ao_save_dma_rd_ptr,
    .pfn_set_ao_rd_offset_flag = ao_set_dma_rd_offset_init_flag,

#ifdef CONFIG_OT_AUDIO_ALSA_SUPPORT
    /* alsa */
    .pfn_ao_set_alsa_dma_attr = ao_set_alsa_dma_attr,
    .pfn_ao_set_dev_attr = ao_set_dev_attr,
    .pfn_ao_set_dma_ext_isr = ao_drv_set_dma_ext_isr,
    .pfn_ao_reset_dma_ext_isr = ao_drv_reset_dma_ext_isr,
#else
    .pfn_ao_set_alsa_dma_attr = TD_NULL,
    .pfn_ao_set_dev_attr = TD_NULL,
    .pfn_ao_set_dma_ext_isr = TD_NULL,
    .pfn_ao_reset_dma_ext_isr = TD_NULL,
#endif
};

#ifdef CONFIG_OT_AUDIO_ONLY_DRIVER_SUPPORT
/* 只支持驱动的情况下无须受base模块控制init和exit，直接返回 */
static td_s32 ao_init_empty(void *p)
{
    return TD_SUCCESS;
}

static td_void ao_exit_empty(void)
{
    return;
}

static umap_module g_ao_module = {
    .mod_id = OT_ID_AO,
    .mod_name = "ao",

    .pfn_init = ao_init_empty,
    .pfn_exit = ao_exit_empty,
    .pfn_query_state = ao_query_state,
    .pfn_notify = ao_notify,
    .pfn_ver_checker  = ao_get_ver_magic,

    .export_funcs = &g_export_funcs,
    .data = TD_NULL,
};
#else
/* 正常情况 */
static umap_module g_ao_module = {
    .mod_id = OT_ID_AO,
    .mod_name = "ao",

    .pfn_init = ao_init,
    .pfn_exit = ao_exit,
    .pfn_query_state = ao_query_state,
    .pfn_notify = ao_notify,
    .pfn_ver_checker  = ao_get_ver_magic,

    .export_funcs = &g_export_funcs,
    .data = TD_NULL,
};
#endif

static td_s32 ao_init_umap_dev(td_void)
{
    g_ao_umapd = osal_createdev(UMAP_DEVNAME_AO_BASE);
    if (g_ao_umapd == TD_NULL) {
        ao_err_trace("ao create device failed \n");
        return TD_FAILURE;
    }
    g_ao_umapd->fops  = &g_ao_fops;
    g_ao_umapd->minor = UMAP_AO_MINOR_BASE;

    if (osal_registerdevice(g_ao_umapd) != TD_SUCCESS) {
        ao_err_trace("ao register device failed \n");
        (td_void)osal_destroydev(g_ao_umapd);
        g_ao_umapd = TD_NULL;
        return TD_FAILURE;
    }

    if (cmpi_register_module(&g_ao_module)) {
        ao_err_trace("ao register module failed \n");
        osal_deregisterdevice(g_ao_umapd);
        (td_void)osal_destroydev(g_ao_umapd);
        g_ao_umapd = TD_NULL;
        return TD_FAILURE;
    }

#ifdef CONFIG_OT_AUDIO_ONLY_DRIVER_SUPPORT
    /* 只支持驱动的情况下需要模块自己调用init，之前是在注册模块时base调用的 */
    if (ao_init(TD_NULL) != TD_SUCCESS) {
        cmpi_unregister_module(OT_ID_AO);
        osal_deregisterdevice(g_ao_umapd);
        (td_void)osal_destroydev(g_ao_umapd);
        g_ao_umapd = TD_NULL;
        return TD_FAILURE;
    }
#endif

    return TD_SUCCESS;
}

static td_void ao_exit_umap_dev(td_void)
{
#ifdef CONFIG_OT_AUDIO_ONLY_DRIVER_SUPPORT
    ao_exit();
#endif
    cmpi_unregister_module(OT_ID_AO);
    osal_deregisterdevice(g_ao_umapd);
    (td_void)osal_destroydev(g_ao_umapd);
    g_ao_umapd = TD_NULL;
}

int ao_module_init(void)
{
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
    osal_proc_entry_t *sub_proc = TD_NULL;
#endif
    td_s32 ret;

    ret = ao_drv_check_aio_export_func();
    if (ret != TD_SUCCESS) {
        return ret;
    }

    /* create proc interface */
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
    sub_proc = osal_create_proc_entry(PROC_ENTRY_AO, TD_NULL);
    if (sub_proc == TD_NULL) {
        ao_info_trace("ao create proc failed\n");
        return TD_FAILURE;
    }
    sub_proc->read = ao_proc_show;
#endif

    /* init g_ao_umapd */
    ret = ao_init_umap_dev();
    if (ret != TD_SUCCESS) {
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
        osal_remove_proc_entry(PROC_ENTRY_AO, TD_NULL);
#endif
        return TD_FAILURE;
    }
    ret = osal_atomic_init(&g_ao_user_ref);
    if (ret < 0) {
        ao_exit_umap_dev();
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
        osal_remove_proc_entry(PROC_ENTRY_AO, TD_NULL);
#endif
        return TD_FAILURE;
    }
    osal_atomic_set(&g_ao_user_ref, 0);

    (td_void)memset_s(&g_ao_export_symbol, sizeof(ot_ao_export_symbol), 0, sizeof(ot_ao_export_symbol));
    (td_void)memset_s(ao_drv_get_export_callback(), sizeof(ot_ao_export_callback), 0, sizeof(ot_ao_export_callback));
    g_ao_export_symbol.register_export_callback = ao_register_extern_call_back;
    // 初始化ai初始化引用计数
    ret = osal_atomic_init(&g_ao_open_ref);
    if (ret < 0) {
        osal_atomic_destroy(&g_ao_user_ref);
        ao_exit_umap_dev();
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
        osal_remove_proc_entry(PROC_ENTRY_AO, TD_NULL);
#endif
        ao_err_trace("ao open_ref atomic init failed\n");
        return TD_FAILURE;
    }
    osal_atomic_set(&g_ao_open_ref, 0);
    ret = osal_sema_init(&g_ao_sem, 1);
    if (ret < 0) {
        osal_atomic_destroy(&g_ao_open_ref);
        osal_atomic_destroy(&g_ao_user_ref);
        ao_exit_umap_dev();
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
        osal_remove_proc_entry(PROC_ENTRY_AO, TD_NULL);
#endif
        ao_err_trace("ao osal g_ao_sem init fail\n");
        return TD_FAILURE;
    }
    ao_info_trace("load ao.ko ....OK!\n");
    return TD_SUCCESS;
}

void ao_module_exit(void)
{
    osal_sema_destroy(&g_ao_sem);
    osal_atomic_destroy(&g_ao_open_ref);
    osal_atomic_destroy(&g_ao_user_ref);

    cmpi_unregister_module(OT_ID_AO);

    if (g_ao_umapd != TD_NULL) {
        osal_deregisterdevice(g_ao_umapd);
        (td_void)osal_destroydev(g_ao_umapd);
        g_ao_umapd = TD_NULL;
    }

    (td_void)memset_s(&g_ao_export_symbol, sizeof(ot_ao_export_symbol), 0, sizeof(ot_ao_export_symbol));
    (td_void)memset_s(ao_drv_get_export_callback(), sizeof(ot_ao_export_callback), 0, sizeof(ot_ao_export_callback));

#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
    osal_remove_proc_entry(PROC_ENTRY_AO, TD_NULL);
#endif
    ao_info_trace("unload ao.ko ....OK!\n");
    return;
}
