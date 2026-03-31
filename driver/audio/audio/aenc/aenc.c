/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: aenc module driver
 * Author: Hisilicon multimedia software group
 * Create: 2022/8/15
 */

#include "aenc.h"

#include "ot_osal.h"
#include "securec.h"

#include "dev_ext.h"
#include "mm_ext.h"
#include "mod_ext.h"
#include "proc_ext.h"
#include "mkp_aenc.h"
#include "aio_pub.h"
#include "mkp_sys.h"
#include "audio_fb.h"
#include "sys_ext.h"
#include "proc_ext.h"
#include "aenc_drv.h"
#include "voie.h"
#include "osal_mmz.h"
#include "pid_protect.h"
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
#include "aenc_proc.h"
#endif

#include "hi_comm_audio.h"
#include "hi_audio_aenc.h"
#include "media_mem_file.h"

#ifndef AUDIO_UT_VCAST
#include <asm/current.h>
#include <linux/device.h>
#endif
#ifndef umap_get_chn
#define umap_get_chn UMAP_GET_CHN
#endif

#ifndef umap_set_chn
#define umap_set_chn UMAP_SET_CHN
#endif

static osal_atomic_t g_aenc_user_ref = OSAL_ATOMIC_INIT(0);
static osal_atomic_t g_aenc_open_ref = OSAL_ATOMIC_INIT(0);
static td_u32 g_aenc_state = AENC_STATE_STOPPED;
static osal_dev_t *g_aenc_umap_dev = TD_NULL;
static osal_wait_t g_wait_user_frm; /* wait_queue for user mode frame to release */

static struct osal_semaphore g_aenc_sem;

aenc_chn_ctx g_aenc_chn[OT_AENC_MAX_CHN_NUM];
struct media_mem_file g_aenc_mfile;
aenc_chn_ctx *aenc_get_chn_ctx(td_s32 chn_id)
{
    return &g_aenc_chn[chn_id];
}

#ifndef VOIE_SUPPORT
inline td_s32 voie_get_type(const ot_aenc_chn_attr *chn_attr, voie_code_type *voie_type)
{
    ot_unused(chn_attr);
    ot_unused(voie_type);
    return 0;
}
#endif

static td_s32 aenc_strm_buf_remap(ot_aenc_chn ae_chn, const aenc_cb_info *buf_info,
    valg_crcl_buf **ref_cb, td_void **virt_base)
{
    td_s32 ret;
    td_u64 iova_temp = 0xfU;

    aenc_chn_ctx *aenc_chn = &g_aenc_chn[ae_chn];
    hi_void *u_virt_addr = (hi_void *)buf_info->cb_phy; // get user virt_addr base
    hi_void *k_virt_addr = TD_NULL;

    ret = media_mem_acquire_iova(&g_aenc_mfile, u_virt_addr,
                                ((td_ulong)buf_info->strm_offset + (td_ulong)buf_info->cb_all_pack_len), 1,
                                &k_virt_addr, &iova_temp);
    if (ret != TD_SUCCESS) {
        aenc_err_trace("cmpi_remap_nocache fail, ref_cb is null, ae_chn:%d\n", ae_chn);
        return OT_ERR_AENC_NO_MEM;
    }

    *ref_cb = (valg_crcl_buf *)k_virt_addr;
    *virt_base = k_virt_addr + buf_info->strm_offset;

    return TD_SUCCESS;
}

static td_s32 aenc_strm_buf_init(ot_aenc_chn ae_chn, const aenc_cb_info *buf_info)
{
    td_s32 ret;
    aenc_chn_ctx *aenc_chn = TD_NULL;
    valg_crcl_buf *ref_cb = TD_NULL;
    td_void *virt_base = TD_NULL;
    td_u32 offset_bytes;
    unsigned long u_chn_lock_flag;

    aenc_check_null_ptr_return(buf_info);

    aenc_check_chn_return(ae_chn);
    aenc_chn = &g_aenc_chn[ae_chn];

    if (osal_down_interruptible(&aenc_chn->sem)) {
        return -ERESTARTSYS;
    }

    offset_bytes = buf_info->strm_offset;

    /* 统一映射为cached方式，方便退出时注销cache */
    ret = aenc_strm_buf_remap(ae_chn, buf_info, &ref_cb, &virt_base);
    if (ret != TD_SUCCESS) {
        osal_up(&aenc_chn->sem);
        return ret;
    }
    /* 初始化共享缓存 */
    ret = valg_cb_init(&aenc_chn->strm_buf, virt_base, buf_info->cb_all_pack_len, buf_info->cb_pack_len);
    if (ret != TD_SUCCESS) {
        media_mem_release_iova(&g_aenc_mfile, (td_ulong)buf_info->strm_offset + (td_ulong)buf_info->cb_all_pack_len,
                              (hi_void *)(&buf_info->cb_phy), (hi_void *)ref_cb);
        osal_up(&aenc_chn->sem);
        aenc_err_trace("valg_cb_init fail, ae_chn:%d\n", ae_chn);
        return OT_ERR_AENC_ILLEGAL_PARAM;
    }
    /* 设置码流BUF的物理内存地址 */
    valg_cb_set_phy_base(&aenc_chn->strm_buf, buf_info->cb_phy + offset_bytes);
    aenc_chn->cb_ctl_vir = (td_u8 *)ref_cb;
    aenc_chn->cb_base_vir = (td_u8 *)virt_base;
    aenc_chn->cb_pack_len = buf_info->cb_pack_len;
    aenc_chn->offset = offset_bytes;
    aenc_chn->buf_info.cb_phy = buf_info->cb_phy;
    aenc_chn->buf_info.cb_all_pack_len = buf_info->cb_all_pack_len;
    aenc_chn->buf_info.strm_offset = buf_info->strm_offset;

    aenc_spin_lock_irqsave(&aenc_chn->spin_lock, u_chn_lock_flag);
    aenc_chn->buf_init = TD_TRUE;
    aenc_spin_unlock_irqrestore(&aenc_chn->spin_lock, u_chn_lock_flag);

    osal_up(&aenc_chn->sem);
    return TD_SUCCESS;
}

static td_s32 aenc_strm_buf_exit(ot_aenc_chn ae_chn)
{
    aenc_chn_ctx *aenc_chn = TD_NULL;
    unsigned long u_chn_lock_flag;
    aenc_check_chn_return(ae_chn);

    aenc_chn = &g_aenc_chn[ae_chn];
    if (osal_down_interruptible(&aenc_chn->sem)) {
        return -ERESTARTSYS;
    }
    aenc_spin_lock_irqsave(&aenc_chn->spin_lock, u_chn_lock_flag);

    if (aenc_chn->buf_init == TD_FALSE) {
        aenc_spin_unlock_irqrestore(&aenc_chn->spin_lock, u_chn_lock_flag);
        osal_up(&aenc_chn->sem);
        return TD_SUCCESS;
    }

    aenc_chn->buf_init = TD_FALSE; /* 先置标志，再unmap */
    aenc_spin_unlock_irqrestore(&aenc_chn->spin_lock, u_chn_lock_flag);
    media_mem_release_iova(&g_aenc_mfile,
        (td_ulong)aenc_chn->buf_info.strm_offset + (td_ulong)aenc_chn->buf_info.cb_all_pack_len,
        (hi_void *)(&aenc_chn->buf_info.cb_phy), (hi_void *)aenc_chn->cb_ctl_vir);
    osal_up(&aenc_chn->sem);
    return TD_SUCCESS;
}

static td_s32 aenc_get_stream_wait_condition_call_back(const td_void *param)
{
    aenc_chn_ctx *aenc_chn = (aenc_chn_ctx *)param;

    return (td_s32)((aenc_chn->created != TD_TRUE) || (valg_cb_get_data_len(&aenc_chn->strm_buf) > 0) ||
                    (g_aenc_state != AENC_STATE_STARTED));
}

static td_s32 aenc_get_frame_wait_condition_call_back(const td_void *param)
{
    aenc_chn_ctx *aenc_chn = (aenc_chn_ctx *)param;
    td_s32 condition;

    condition = (td_s32)((aenc_chn->created != TD_TRUE) || (g_aenc_state != AENC_STATE_STARTED) ||
        ((audio_fb_query_busy_num(&aenc_chn->ai_frm_buf) != 0) &&
        (aenc_chn->voie_code_type == VOIE_CODE_TYPE_NONE)));

    return condition;
}

static td_s32 aenc_get_stream_wait_condition(ot_aenc_chn ae_chn, td_s32 milli_sec)
{
    td_s32 ret;
    aenc_chn_ctx *aenc_chn = TD_NULL;
    aenc_chn = &g_aenc_chn[ae_chn];

    if (milli_sec == 0) {
        return OT_ERR_AENC_BUF_EMPTY;
    } else if (milli_sec == -1) {
        if (osal_wait_event_interruptible(&aenc_chn->strm_wait, aenc_get_stream_wait_condition_call_back,
            (td_void *)aenc_chn)) {
            return -ERESTARTSYS;
        }
    } else {
        ret = osal_wait_event_timeout_interruptible(&aenc_chn->strm_wait,
            aenc_get_stream_wait_condition_call_back, (td_void *)aenc_chn, milli_sec);
        if (ret == 0) {
            aenc_err_trace("aenc chn %d stream buf is empty and timeout\n", ae_chn);
            return OT_ERR_AENC_BUF_EMPTY;
        } else if (ret < 0) {
            return -ERESTARTSYS;
        }
    }

    return TD_SUCCESS;
}

static td_s32 aenc_set_stream_info(ot_aenc_chn ae_chn, ot_audio_stream *strm)
{
    aenc_stream_header *strm_header = TD_NULL;
    valg_cb_rdinfo rd_info;
    aenc_chn_ctx *aenc_chn = TD_NULL;
    aenc_chn = &g_aenc_chn[ae_chn];

    /* 有数据时，首先读取数据头 */
    strm_header = (aenc_stream_header *)valg_cb_get_rd_head(&aenc_chn->strm_buf);

    /* 读取数据 */
    if (valg_cb_read(&aenc_chn->strm_buf, aenc_chn->cb_pack_len, &rd_info)) {
        return OT_ERR_AENC_ILLEGAL_PARAM;
    }

    /* 检查读取数据信息，地址和长度不能为0，且不应该有CB折回情况出现 */
    if ((rd_info.len[0] == 0) || (rd_info.phy_addr[0] == 0) || (rd_info.len[1] != 0)) {
        return OT_ERR_AENC_ILLEGAL_PARAM;
    }

#ifdef CONFIG_AUDIO_V200_SUPPORT
    td_u8 *k_virt_addr = (td_u8 *)rd_info.src[0] + sizeof(aenc_stream_header); // kernel virt_addr
    td_u64 k_offset = (td_u64)(k_virt_addr - aenc_chn->cb_base_vir);
    strm->phys_addr = k_offset;
#else
    /* 码流信息赋值 */
    strm->phys_addr = (td_phys_addr_t)(rd_info.phy_addr[0] + sizeof(aenc_stream_header));
    strm->stream = (td_u8 *)rd_info.src[0] + sizeof(aenc_stream_header);
#endif
    strm->len = strm_header->len;
    strm->seq = strm_header->seq;
    strm->time_stamp = strm_header->time_stamp;

    return TD_SUCCESS;
}

static td_s32 aenc_get_stream(ot_aenc_chn ae_chn, ot_audio_stream *strm, td_s32 milli_sec)
{
    aenc_chn_ctx *aenc_chn = TD_NULL;
    unsigned long lock_flag;
    td_s32 ret;

    aenc_check_chn_return(ae_chn);
    aenc_check_null_ptr_return(strm);
    aenc_chn = &g_aenc_chn[ae_chn];
    aenc_check_cb_null_ptr_return(aenc_chn->strm_buf);

    if (osal_down_interruptible(&aenc_chn->sem)) {
        return -ERESTARTSYS;
    }

    aenc_spin_lock_irqsave(&aenc_chn->spin_lock, lock_flag);

    if ((aenc_chn->created == TD_FALSE) || (aenc_chn->buf_init == TD_FALSE)) {
        aenc_spin_unlock_irqrestore(&aenc_chn->spin_lock, lock_flag);
        aenc_err_trace("aenc chn %d has not been created!\n", ae_chn);
        ret = OT_ERR_AENC_UNEXIST;
        goto err;
    }

    /* 查询共享缓存中是否有可读数据 */
    while (valg_cb_get_data_len(&aenc_chn->strm_buf) == 0) {
        aenc_spin_unlock_irqrestore(&aenc_chn->spin_lock, lock_flag);
        osal_up(&aenc_chn->sem);

        /* 等待码流数据可读 */
        ret = aenc_get_stream_wait_condition(ae_chn, milli_sec);
        if (ret != TD_SUCCESS) {
            return ret;
        }

        if (osal_down_interruptible(&aenc_chn->sem)) {
            return -ERESTARTSYS;
        }

        if (g_aenc_state != AENC_STATE_STARTED) {
            osal_up(&aenc_chn->sem);
            return OT_ERR_AENC_NOT_READY;
        }

        aenc_spin_lock_irqsave(&aenc_chn->spin_lock, lock_flag);

        if ((aenc_chn->created == TD_FALSE) || (aenc_chn->buf_init == TD_FALSE)) {
            aenc_spin_unlock_irqrestore(&aenc_chn->spin_lock, lock_flag);
            aenc_err_trace("aenc chn %d has been destroyed\n", ae_chn);
            ret = OT_ERR_AENC_UNEXIST;
            goto err;
        }
    }

    /* 读取数据并对码流信息赋值 */
    ret = aenc_set_stream_info(ae_chn, strm);
    if (ret != TD_SUCCESS) {
        aenc_spin_unlock_irqrestore(&aenc_chn->spin_lock, lock_flag);
        goto err;
    }

    aenc_chn->send_dbg.get_strm++;
    ret = TD_SUCCESS;
    aenc_spin_unlock_irqrestore(&aenc_chn->spin_lock, lock_flag);

err:
    osal_up(&aenc_chn->sem);
    return ret;
}

static td_s32 aenc_put_stream(ot_aenc_chn ae_chn, const ot_audio_stream *strm)
{
    aenc_chn_ctx *aenc_chn = TD_NULL;
    unsigned long lock_flag;

    aenc_check_null_ptr_return(strm);

    aenc_check_chn_return(ae_chn);
    aenc_chn = &g_aenc_chn[ae_chn];
    aenc_check_cb_null_ptr_return(aenc_chn->strm_buf);

    if (osal_down_interruptible(&aenc_chn->sem)) {
        return -ERESTARTSYS;
    }
    aenc_spin_lock_irqsave(&aenc_chn->spin_lock, lock_flag);

    if (aenc_chn->buf_init != TD_TRUE) {
        aenc_spin_unlock_irqrestore(&aenc_chn->spin_lock, lock_flag);
        osal_up(&aenc_chn->sem);
        aenc_err_trace("aenc chn %d has not been created!\n", ae_chn);
        return OT_ERR_AENC_UNEXIST;
    }

#ifndef CONFIG_AUDIO_V200_SUPPORT
    /* 对用户传入码流信息作检测：要求strm指向的地址必须落在CB的范围内，并且长度不能超出cb_pack_len */
    if ((strm->phys_addr < aenc_chn->strm_buf.phy_base) ||
        (strm->phys_addr > (aenc_chn->strm_buf.phy_base + aenc_chn->strm_buf.buf_len))) {
        aenc_spin_unlock_irqrestore(&aenc_chn->spin_lock, lock_flag);
        osal_up(&aenc_chn->sem);
        aenc_err_trace("the stream's phys_addr(0x%llx) is illegal for ae_chn %d!\n",
            (td_u64)strm->phys_addr, ae_chn);
        return OT_ERR_AENC_ILLEGAL_PARAM;
    }
#endif
    if ((strm->len > aenc_chn->cb_pack_len) || (strm->len == 0)) {
        aenc_spin_unlock_irqrestore(&aenc_chn->spin_lock, lock_flag);
        osal_up(&aenc_chn->sem);
        aenc_err_trace("the stream's len(%u) should be (0, cb_pack_len(%u)] for ae_chn %d!\n",
            strm->len, aenc_chn->cb_pack_len, ae_chn);
        return OT_ERR_AENC_ILLEGAL_PARAM;
    }

    if (valg_cb_update_rp(&aenc_chn->strm_buf, aenc_chn->cb_pack_len)) {
        aenc_spin_unlock_irqrestore(&aenc_chn->spin_lock, lock_flag);
        osal_up(&aenc_chn->sem);
        aenc_err_trace("aenc_put_stream valg cb update rp fail\n");
        return OT_ERR_AENC_NOT_PERM;
    }

    aenc_chn->send_dbg.rls_strm++;

    aenc_spin_unlock_irqrestore(&aenc_chn->spin_lock, lock_flag);
    osal_up(&aenc_chn->sem);
    return TD_SUCCESS;
}

td_s32 aenc_wakeup_get_stream(ot_aenc_chn ae_chn)
{
    aenc_chn_ctx *aenc_chn = TD_NULL;

    aenc_check_chn_return(ae_chn);
    aenc_chn = &g_aenc_chn[ae_chn];
    aenc_check_cb_null_ptr_return(aenc_chn->strm_buf);

    /*
     * 三处需要唤醒等待队列:
     * 1、用户态编码成功，通过ioctl唤醒；
     * 用户态通过pthread_mutex_lock(&aenc_chn->lock);确保了调用此函数时，destroy函数互斥
     * 2、voie编码完成中断
     * 中断通过aenc_spin_lock_irqsave(&aenc_chn->spin_lock, u_chn_lock_flag);确保了通道处于已经创建；
     * 3、销毁通道前
     * 在销毁前调用；
     * 以上三种运行上下文，均可确保:
     * 1、通道处于创建状态；
     * 2、通道的锁处于保护状态；
     */
    aenc_chn->cb_cur_len = valg_cb_get_data_len(&aenc_chn->strm_buf);
    if (aenc_chn->cb_cur_len > 0) {
        osal_wakeup(&aenc_chn->strm_wait);
    } else {
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_s32 aenc_add_vb(const audio_frame_combine *audio_frm_info, td_u32 uid)
{
    td_s32 i, chn_per_frm, ret;
    chn_per_frm = aio_chn_num_per_frm(audio_frm_info->frm.snd_mode);

    for (i = 0; i < chn_per_frm; i++) {
        ret = call_vb_user_add(audio_frm_info->frm.pool_id[i], audio_frm_info->frm.phys_addr[i], uid);
        if (ret != TD_SUCCESS) {
            aenc_err_trace("call_vb_user_add fail with 0x%x.\n", (td_u32)ret);
        }
        if (audio_frm_info->ref_frm.valid == TD_TRUE) {
            ret = call_vb_user_add(audio_frm_info->ref_frm.ref_frame.pool_id[i],
                audio_frm_info->ref_frm.ref_frame.phys_addr[i], uid);
            if (ret != TD_SUCCESS) {
                aenc_err_trace("call_vb_user_add fail with 0x%x.\n", (td_u32)ret);
            }
        }
    }

    return TD_SUCCESS;
}

td_s32 aenc_sub_vb(const audio_frame_combine *audio_frm, td_u32 uid)
{
    td_s32 i, chn_per_frm, ret;
    chn_per_frm = aio_chn_num_per_frm(audio_frm->frm.snd_mode);

    for (i = 0; i < chn_per_frm; i++) {
        ret = call_vb_user_sub(audio_frm->frm.pool_id[i], audio_frm->frm.phys_addr[i], uid);
        if (ret != TD_SUCCESS) {
            aenc_err_trace("call_vb_user_sub fail with 0x%x.\n", (td_u32)ret);
        }
        if (audio_frm->ref_frm.valid == TD_TRUE) {
            ret = call_vb_user_sub(audio_frm->ref_frm.ref_frame.pool_id[i],
                audio_frm->ref_frm.ref_frame.phys_addr[i], uid);
            if (ret != TD_SUCCESS) {
                aenc_err_trace("call_vb_user_sub fail with 0x%x.\n", (td_u32)ret);
            }
        }
    }

    return TD_SUCCESS;
}

static td_s32 aenc_chn_buf_init(ot_aenc_chn ae_chn, const ot_aenc_chn_attr *chn_attr)
{
    td_s32 ret;
    aenc_chn_ctx *aenc_chn = TD_NULL;
    aenc_chn = &g_aenc_chn[ae_chn];

    ret = audio_fb_init(chn_attr->buf_size, &aenc_chn->ai_frm_buf);
    if (ret != TD_SUCCESS) {
        return OT_ERR_AENC_NO_MEM;
    }

    if ((aenc_chn->voie_code_type != VOIE_CODE_TYPE_NONE) &&
        (aenc_chn->voie_code_type != VOIE_CODE_TYPE_BUTT)) {
        /* 初始化user_frm_list以保存用户态发送下来的帧 */
        ret = audio_fb_init(AENC_USER_FRM_MAXCNT, &aenc_chn->user_frm_buf);
        if (ret != TD_SUCCESS) {
            (td_void)audio_fb_release(&aenc_chn->ai_frm_buf);
            return OT_ERR_AENC_NO_MEM;
        }
        if (aenc_is_support_voie()) {
#ifdef VOIE_SUPPORT
            ret = voie_open();
            if (ret != TD_SUCCESS) {
                audio_fb_release(&aenc_chn->user_frm_buf);
                audio_fb_release(&aenc_chn->ai_frm_buf);
                return ret;
            }
#endif
        }
    }

    return TD_SUCCESS;
}

static td_void aenc_frame_buf_put_free(audio_frame_buf *frm_buf)
{
    td_s32 ret;
    td_u32 busy_num, i;
    audio_buf_blk *node = TD_NULL;

    busy_num = audio_fb_query_busy_num(frm_buf);
    for (i = 0; i < busy_num; i++) {
        node = audio_fb_get_busy(frm_buf);
        if (node == TD_NULL) {
            aenc_err_trace("frame node is null.\n");
            continue;
        }

        ret = aenc_sub_vb(&node->audio_frm, OT_VB_UID_AENC);
        if (ret != TD_SUCCESS) {
            aenc_info_trace("aenc_sub_vb fail, ret:0x%x.\n", (td_u32)ret);
        }
        (td_void)audio_fb_put_free(frm_buf, node);
    }
}

static td_void aenc_chn_buf_exit(ot_aenc_chn ae_chn)
{
    aenc_chn_ctx *aenc_chn = TD_NULL;
    audio_frame_buf *buf = TD_NULL;
    td_void *buf_vir_addr_tmp = TD_NULL;
    unsigned long lock_flag;

    aenc_chn = &g_aenc_chn[ae_chn];

    if ((aenc_chn->voie_code_type != VOIE_CODE_TYPE_NONE) &&
        (aenc_chn->voie_code_type != VOIE_CODE_TYPE_BUTT)) {
        if (aenc_is_support_voie()) {
#ifdef VOIE_SUPPORT
            voie_close(ae_chn);
#endif
        }

        aenc_spin_lock_irqsave(&aenc_chn->spin_lock, lock_flag);

        /* 销毁user_frm_buf */
        buf = &(aenc_chn->user_frm_buf);
        aenc_frame_buf_put_free(buf);

        /*
         * spin lock中不允许直接调用vfree，所以不能直接调用audio_fb_release。
         * 这里先将地址置为空，然后在spin unlock之后再进行vfree。
         */
        if (buf->buf_vir_addr != TD_NULL) {
            buf_vir_addr_tmp = buf->buf_vir_addr;
            buf->buf_vir_addr = TD_NULL;
        }

        aenc_spin_unlock_irqrestore(&aenc_chn->spin_lock, lock_flag);
        if (buf_vir_addr_tmp != TD_NULL) {
            osal_vfree(buf_vir_addr_tmp);
            buf_vir_addr_tmp = TD_NULL;
        }
    }

    aenc_spin_lock_irqsave(&aenc_chn->spin_lock, lock_flag);

    /* 销毁ai_frm_buf */
    buf = &(aenc_chn->ai_frm_buf);
    aenc_frame_buf_put_free(&(aenc_chn->ai_frm_buf));

    /*
     * spin lock中不允许直接调用vfree，所以不能直接调用audio_fb_release。
     * 这里先将地址置为空，然后在spin unlock之后再进行vfree。
     */
    if (buf->buf_vir_addr != TD_NULL) {
        buf_vir_addr_tmp = buf->buf_vir_addr;
        buf->buf_vir_addr = TD_NULL;
    }

    aenc_spin_unlock_irqrestore(&aenc_chn->spin_lock, lock_flag);
    if (buf_vir_addr_tmp != TD_NULL) {
        osal_vfree(buf_vir_addr_tmp);
        buf_vir_addr_tmp = TD_NULL;
    }
}

static td_s32 aenc_create_chn(ot_aenc_chn ae_chn, const ot_aenc_chn_attr *chn_attr)
{
    td_s32 ret;
    aenc_chn_ctx *aenc_chn = TD_NULL;
    unsigned long lock_flag;

    aenc_check_null_ptr_return(chn_attr);

    aenc_check_chn_return(ae_chn);
    aenc_chn = &g_aenc_chn[ae_chn];

    if (osal_down_interruptible(&aenc_chn->sem)) {
        return -ERESTARTSYS;
    }

    if (aenc_chn->created == TD_TRUE) {
        osal_up(&aenc_chn->sem);
        return TD_SUCCESS;
    }

    ret = memcpy_s(&aenc_chn->chn_attr, sizeof(aenc_chn->chn_attr), chn_attr, sizeof(*chn_attr));
    if (ret != EOK) {
        aenc_err_trace("aenc_chn %d chn_attr memcpy_s fail, ret = 0x%x.\n", ae_chn, (td_u32)ret);
        osal_up(&aenc_chn->sem);
        return OT_ERR_AENC_ILLEGAL_PARAM;
    }

    ret = voie_get_type(chn_attr, &aenc_chn->voie_code_type);
    if (ret != TD_SUCCESS) {
        osal_up(&aenc_chn->sem);
        return ret;
    }

    /* buf_size等于1时，会导致获取不到空闲buf,因使用一个buf作为预留空间 */
    if ((chn_attr->buf_size < 2) || (chn_attr->buf_size > OT_MAX_ADEC_AENC_FRAME_NUM)) { /* 2: min size */
        aenc_err_trace("invalid buf_size(%u)\n", chn_attr->buf_size);
        osal_up(&aenc_chn->sem);
        return OT_ERR_AENC_ILLEGAL_PARAM;
    }

    /* 创建frmae buf和VOIE专用的buf */
    ret = aenc_chn_buf_init(ae_chn, chn_attr);
    if (ret != TD_SUCCESS) {
        osal_up(&aenc_chn->sem);
        return ret;
    }

    /* 初始化dbg信息 */
    (td_void)memset_s(&aenc_chn->send_dbg, sizeof(aenc_send_frm_dbg), 0, sizeof(aenc_send_frm_dbg));

    aenc_chn->send_dbg.g726_rate = OT_G726_BUTT;
    aenc_chn->send_dbg.adpcm_type = OT_ADPCM_TYPE_BUTT;
    aenc_chn->has_frm_in_user_mode = TD_FALSE;
    aenc_chn->usr_que_lost = 0;
    aenc_chn->mute = TD_FALSE;

    aenc_spin_lock_irqsave(&aenc_chn->spin_lock, lock_flag);
    aenc_chn->created = TD_TRUE;
    aenc_spin_unlock_irqrestore(&aenc_chn->spin_lock, lock_flag);

    osal_up(&aenc_chn->sem);

    return TD_SUCCESS;
}

static td_s32 aenc_destroy_chn(ot_aenc_chn ae_chn)
{
    aenc_chn_ctx *aenc_chn = TD_NULL;
    unsigned long lock_flag;
    aenc_check_chn_return(ae_chn);
    aenc_chn = &g_aenc_chn[ae_chn];

    if (osal_down_interruptible(&aenc_chn->sem)) {
        return -ERESTARTSYS;
    }

    if (aenc_chn->created != TD_TRUE) {
        osal_up(&aenc_chn->sem);
        return TD_SUCCESS;
    }

    /* 由于VOIE中断也会访问created标志，所以要用自旋锁进行保护 */
    aenc_spin_lock_irqsave(&aenc_chn->spin_lock, lock_flag);
    aenc_chn->created = TD_FALSE;
    aenc_spin_unlock_irqrestore(&aenc_chn->spin_lock, lock_flag);

    osal_wakeup(&aenc_chn->ai_frm_wait);
    osal_wakeup(&aenc_chn->strm_wait);

    /* 销毁buf */
    aenc_chn_buf_exit(ae_chn);

    osal_up(&aenc_chn->sem);
    return TD_SUCCESS;
}

static td_void aenc_put_ai_frm_to_buf(ot_aenc_chn ae_chn, const audio_frame_combine *audio_frm)
{
    td_s32 ret;
    td_u32 depth, busy_num, i;
    aenc_chn_ctx *aenc_chn = &g_aenc_chn[ae_chn];
    audio_frame_buf *buf = TD_NULL;
    audio_buf_blk *node = TD_NULL;

    depth = aenc_chn->chn_attr.buf_size;
    buf = &aenc_chn->ai_frm_buf;
    busy_num = audio_fb_query_busy_num(buf);

    /*
     *  如果freelist不为空，从freelist队头获取一个节点挂到busylist队尾
     *  如果busy_num>=depth，先从busylist摘busy_num-depth个节点到freelist，
     *  再判断depth是否为0，否则从busylist队头摘一个节点到队尾
     */
    if (audio_fb_query_free_num(buf) != 0) {
        node = audio_fb_get_free(buf);
        if (node == TD_NULL) {
            aenc_err_trace("aenc_chn %d audio_fb node is null.\n", ae_chn);
            return;
        }

        ret = memcpy_s(&node->audio_frm, sizeof(node->audio_frm), audio_frm, sizeof(*audio_frm));
        if (ret != EOK) {
            aenc_warn_trace("aenc_chn %d audio_frm memcpy_s fail, ret = 0x%x.\n", ae_chn, (td_u32)ret);
            return;
        }

        (td_void)audio_fb_put_busy(buf, node);
        (td_void)aenc_add_vb(audio_frm, OT_VB_UID_AENC);
    } else if (busy_num >= depth) {
        for (i = 0; i < (busy_num - depth); i++) {
            node = audio_fb_get_busy(buf);
            if (node == TD_NULL) {
                aenc_err_trace("aenc_chn %d audio_fb node is null.\n", ae_chn);
                continue;
            }

            (td_void)aenc_sub_vb(&node->audio_frm, OT_VB_UID_AENC);

            (td_void)audio_fb_put_free(buf, node);
        }
        if (depth != 0) {
            node = audio_fb_get_busy(buf);
            if (node == TD_NULL) {
                aenc_err_trace("aenc_chn %d audio_fb node is null.\n", ae_chn);
                return;
            }

            (td_void)aenc_sub_vb(&node->audio_frm, OT_VB_UID_AENC);

            ret = memcpy_s(&node->audio_frm, sizeof(node->audio_frm), audio_frm, sizeof(*audio_frm));
            if (ret != EOK) {
                aenc_warn_trace("aenc_chn %d audio_frm memcpy_s fail, ret = 0x%x.\n", ae_chn, (td_u32)ret);
                return;
            }

            (td_void)audio_fb_put_busy(buf, node);
            (td_void)aenc_add_vb(audio_frm, OT_VB_UID_AENC);

            aenc_chn->usr_que_lost++;
        }
    }
}

static td_s32 aenc_receive_frm(td_s32 dev_id, td_s32 chn_id, td_bool block,
    mpp_data_type data_type, td_void *data)
{
    ot_aenc_chn ae_chn;
    aenc_chn_ctx *aenc_chn = TD_NULL;
    audio_frame_combine *audio_frm = TD_NULL;
    unsigned long lock_flag;
    td_u32 scr_point_num_per_frame;

    ot_unused(dev_id);
    ot_unused(block);

    /* 解析SYS_BIND传递来的参数 */
    aenc_check_null_ptr_return(data);
    if (data_type != MPP_DATA_AUDIO_FRAME) {
        aenc_err_trace("data_type = %d is not audio frame type!\n", (td_s32)data_type);
        return OT_ERR_AENC_ILLEGAL_PARAM;
    }

    audio_frm = (audio_frame_combine *)data;
    ae_chn = (ot_aenc_chn)chn_id;
    aenc_check_chn_return(ae_chn);

    aenc_chn = &g_aenc_chn[ae_chn];
    if (aenc_chn->created != TD_TRUE) {
        osal_wakeup(&aenc_chn->ai_frm_wait);
        aenc_warn_trace("aenc chn %d has not been created!\n", ae_chn);
        return OT_ERR_AENC_NOT_READY;
    }

    /* check point num for voie type */
    if (aenc_chn->voie_code_type != VOIE_CODE_TYPE_NONE) {
        td_u32 sample_per_bit = (audio_frm->frm.bit_width == OT_AUDIO_BIT_WIDTH_24) ? 4 : /* 4: bytes */
                                ((td_u32)(audio_frm->frm.bit_width) + 1U);
        scr_point_num_per_frame = audio_frm->frm.len / sample_per_bit;

        if (scr_point_num_per_frame > OT_MAX_VOICE_POINT_NUM) {
            aenc_err_trace("point num (%u) of this frame is larger than MAX_VOICE_POINT_NUM(%d) for voie encode\n",
                scr_point_num_per_frame, OT_MAX_VOICE_POINT_NUM);
            return OT_ERR_AENC_NOT_SUPPORT;
        }
    }
    aenc_spin_lock_irqsave(&aenc_chn->spin_lock, lock_flag);

    /* 写入ai_frm_buf */
    aenc_put_ai_frm_to_buf(ae_chn, audio_frm);

    /* 对于要做AEC或是非VOIE的编码的帧，则唤醒用户态获取音频帧的线程 */
    if (aenc_chn->voie_code_type == VOIE_CODE_TYPE_NONE) {
        /* 由于到用户态后，会在其他地方进行rcv_frm++，所以这里不用rcv_frm++ */
        osal_wakeup(&aenc_chn->ai_frm_wait);
    } else {
        aenc_chn->send_dbg.rcv_frm++;
    }

    aenc_spin_unlock_irqrestore(&aenc_chn->spin_lock, lock_flag);

    return TD_SUCCESS;
}

static td_s32 aenc_get_frm_from_buf(audio_frame_buf *buf, audio_frame_combine *audio_frm)
{
    td_s32 ret;
    audio_buf_blk *node = TD_NULL;

    node = audio_fb_get_busy(buf);
    if (node == TD_NULL) {
        aenc_err_trace("busy list empty\n");
        return OT_ERR_AENC_BUF_EMPTY;
    }

    ret = memcpy_s(audio_frm, sizeof(*audio_frm), &node->audio_frm, sizeof(node->audio_frm));
    if (ret != EOK) {
        aenc_err_trace("audio_frm memcpy_s fail, ret = 0x%x.\n", (td_u32)ret);
        return OT_ERR_AENC_ILLEGAL_PARAM;
    }

    ret = aenc_add_vb(audio_frm, OT_VB_UID_USER);
    if (ret != TD_SUCCESS) {
        aenc_info_trace("aenc_add_vb fail, ret:0x%x.\n", (td_u32)ret);
    }
    ret = aenc_sub_vb(audio_frm, OT_VB_UID_AENC);
    if (ret != TD_SUCCESS) {
        aenc_info_trace("aenc_sub_vb fail, ret:0x%x.\n", (td_u32)ret);
    }

    (td_void)audio_fb_put_free(buf, node);

    return TD_SUCCESS;
}

static td_s32 aenc_get_frame(ot_aenc_chn ae_chn, audio_frame_combine *audio_frm)
{
    aenc_chn_ctx *aenc_chn = TD_NULL;
    audio_frame_buf *buf = TD_NULL;
    unsigned long lock_flag;
    td_s32 ret;

    aenc_check_null_ptr_return(audio_frm);

    aenc_check_chn_return(ae_chn);
    aenc_chn = &g_aenc_chn[ae_chn];

    if (osal_down_interruptible(&aenc_chn->sem)) {
        return -ERESTARTSYS;
    }

    buf = &aenc_chn->ai_frm_buf;

    /*
     * 同时满足下面两个条件时需要等待：
     *  1）当通道使能时；
     *  2）队列为空，或者队列不为空但它属性VOIE编码类型且不需要做AEC
     */
    while ((aenc_chn->created == TD_TRUE) &&
           ((audio_fb_query_busy_num(buf) == 0) ||
           ((audio_fb_query_busy_num(buf) != 0) && (aenc_chn->voie_code_type != VOIE_CODE_TYPE_NONE)))) {
        /* 等待时不能占有互斥锁 */
        osal_up(&aenc_chn->sem);

        /*
         * 出现下面三种情况时则等待返回：
         *  1）当出现通道禁用时；
         *  2）通道BUF不为空且通道为非VOIE编码类型；
         *  3）通道BUF不为空且需要做AEC时
         */
        if (osal_wait_event_interruptible(&aenc_chn->ai_frm_wait, aenc_get_frame_wait_condition_call_back,
            (td_void *)aenc_chn)) {
            aenc_warn_trace("aenc %d get_frame failed\n", ae_chn);
            return -ERESTARTSYS;
        }

        if (osal_down_interruptible(&aenc_chn->sem)) {
            return -ERESTARTSYS;
        }

        if (g_aenc_state != AENC_STATE_STARTED) {
            osal_up(&aenc_chn->sem);
            return OT_ERR_AENC_NOT_READY;
        }
    }

    /* 等待队列被唤醒后，检查是否是因为通道禁用而引起的唤醒 */
    if (aenc_chn->created != TD_TRUE) {
        osal_up(&aenc_chn->sem);
        aenc_warn_trace("aenc chn %d is disable!\n", ae_chn);
        return OT_ERR_AENC_NOT_READY;
    }

    aenc_spin_lock_irqsave(&aenc_chn->spin_lock, lock_flag);

    ret = aenc_get_frm_from_buf(buf, audio_frm);
    if (ret != TD_SUCCESS) {
        aenc_spin_unlock_irqrestore(&aenc_chn->spin_lock, lock_flag);
        osal_up(&aenc_chn->sem);
        return ret;
    }

    aenc_chn->has_frm_in_user_mode = TD_TRUE;

    aenc_spin_unlock_irqrestore(&aenc_chn->spin_lock, lock_flag);

    osal_up(&aenc_chn->sem);

    return TD_SUCCESS;
}

static td_s32 aenc_release_frame(ot_aenc_chn ae_chn, audio_frame_combine *audio_frm)
{
    td_s32 i, chn_per_frm, ret;
    ot_audio_frame *frm_tmp = TD_NULL;
    aenc_chn_ctx *aenc_chn = TD_NULL;

    aenc_check_null_ptr_return(audio_frm);

    aenc_check_chn_return(ae_chn);
    aenc_chn = &g_aenc_chn[ae_chn];
    chn_per_frm = aio_chn_num_per_frm(audio_frm->frm.snd_mode);

    if (osal_down_interruptible(&aenc_chn->sem)) {
        return -ERESTARTSYS;
    }

    for (i = 0; i < chn_per_frm; i++) {
        /* 此处不检查通道是否DISABLE，以防止用户disable_chn后释放frame的情况 */
        frm_tmp = &audio_frm->frm;
        if (call_vb_is_blk_valid(frm_tmp->pool_id[i], frm_tmp->phys_addr[i]) != TD_TRUE) {
            aenc_err_trace("ERR!!! aenc chn%d invalid addr:0x%llx\r\n",
                           ae_chn, (td_u64)frm_tmp->phys_addr[i]);
            osal_up(&aenc_chn->sem);
            return OT_ERR_AENC_ILLEGAL_PARAM;
        }

        if (audio_frm->ref_frm.valid == TD_TRUE) {
            frm_tmp = &audio_frm->ref_frm.ref_frame;
            if (call_vb_is_blk_valid(frm_tmp->pool_id[i], frm_tmp->phys_addr[i]) != TD_TRUE) {
                aenc_err_trace("ERR!!! aenc chn%d invalid addr:0x%llx\r\n",
                               ae_chn, (td_u64)frm_tmp->phys_addr[i]);
                osal_up(&aenc_chn->sem);
                return OT_ERR_AENC_ILLEGAL_PARAM;
            }
        }
    }

    ret = aenc_sub_vb(audio_frm, OT_VB_UID_USER);
    if (ret != TD_SUCCESS) {
        aenc_info_trace("aenc_sub_vb fail, ret:0x%x.\n", (td_u32)ret);
    }

    aenc_chn->has_frm_in_user_mode = TD_FALSE;
    osal_wakeup(&g_wait_user_frm);

    osal_up(&aenc_chn->sem);

    return TD_SUCCESS;
}

#ifdef VOIE_SUPPORT
static td_s32 send_frm_to_user_fb(ot_aenc_chn ae_chn, aenc_chn_ctx *chn_ctx, const ot_audio_frame *user_frm)
{
    audio_buf_blk *node = TD_NULL;
    audio_frame_buf *buf = &chn_ctx->user_frm_buf;

    if (audio_fb_query_free_num(buf) != 0) {
        node = audio_fb_get_free(buf);
        if (node == TD_NULL) {
            aenc_err_trace("aenc chn %d frame node is null.\n", ae_chn);
            return OT_ERR_AENC_NULL_PTR;
        }

        if (memcpy_s(&node->audio_frm.frm, sizeof(node->audio_frm.frm), user_frm, sizeof(*user_frm)) != EOK) {
            aenc_err_trace("aenc chn%d frm memcpy_s fail\n", ae_chn);
            return OT_ERR_AENC_ILLEGAL_PARAM;
        }

        node->audio_frm.ref_frm.valid = TD_FALSE;
        audio_fb_put_busy(buf, node);

        aenc_add_vb(&node->audio_frm, OT_VB_UID_AENC);
    } else {
        /* 没有空闲空间的时候出错返回 */
        aenc_warn_trace("aenc chn%d user_frm_buf is full\n", ae_chn);
        return OT_ERR_AENC_NO_BUF;
    }

    return TD_SUCCESS;
}

/* 把音频帧添加到user_frm_list中 */
static td_s32 aenc_send_frm_to_voie(ot_aenc_chn ae_chn, const ot_audio_frame *user_frm)
{
    aenc_chn_ctx *aenc_chn = TD_NULL;
    unsigned long lock_flag;
    td_s32 chn_per_frm, i, ret;

    aenc_check_null_ptr_return(user_frm);
    aenc_check_chn_return(ae_chn);

    chn_per_frm = aio_chn_num_per_frm(user_frm->snd_mode);
    for (i = 0; i < chn_per_frm; i++) {
        /* VOIE编码的音频帧要求用VB来存储 */
        if (call_vb_is_blk_valid(user_frm->pool_id[i], user_frm->phys_addr[i]) != TD_TRUE) {
            aenc_err_trace("the frame is not a VB block for VOIE\n");
            return OT_ERR_AENC_ILLEGAL_PARAM;
        }
    }

    aenc_chn = &g_aenc_chn[ae_chn];

    if (osal_down_interruptible(&aenc_chn->sem)) {
        return -ERESTARTSYS;
    }

    if (aenc_chn->created == TD_FALSE) {
        osal_up(&aenc_chn->sem);
        aenc_err_trace("aenc chn%d is disable\n", ae_chn);
        return OT_ERR_AENC_NOT_CFG;
    }

    aenc_spin_lock_irqsave(&aenc_chn->spin_lock, lock_flag);

    ret = send_frm_to_user_fb(ae_chn, aenc_chn, user_frm);
    if (ret != TD_SUCCESS) {
        aenc_spin_unlock_irqrestore(&aenc_chn->spin_lock, lock_flag);
        osal_up(&aenc_chn->sem);
        return ret;
    }

    aenc_chn->send_dbg.rcv_frm++;
    aenc_spin_unlock_irqrestore(&aenc_chn->spin_lock, lock_flag);

    osal_up(&aenc_chn->sem);

    return TD_SUCCESS;
}
#endif

static td_s32 aenc_set_send_info(ot_aenc_chn ae_chn, const aenc_send_frm_dbg *dbg_info)
{
    aenc_chn_ctx *aenc_chn = TD_NULL;

    aenc_check_null_ptr_return(dbg_info);

    aenc_check_chn_return(ae_chn);
    aenc_chn = &g_aenc_chn[ae_chn];

    aenc_chn->send_dbg.rcv_frm = dbg_info->rcv_frm;
    aenc_chn->send_dbg.enc_ok = dbg_info->enc_ok;
    aenc_chn->send_dbg.buf_full = dbg_info->buf_full;
    aenc_chn->send_dbg.frm_err = dbg_info->frm_err;
    aenc_chn->send_dbg.g726_rate = dbg_info->g726_rate;
    aenc_chn->send_dbg.adpcm_type = dbg_info->adpcm_type;

    return TD_SUCCESS;
}

static td_s32 aenc_update_stream_buf_wr_tail(ot_aenc_chn ae_chn, td_u32 *wr_tail)
{
    aenc_chn_ctx *aenc_chn = TD_NULL;

    aenc_check_null_ptr_return(wr_tail);

    aenc_check_chn_return(ae_chn);
    aenc_chn = &g_aenc_chn[ae_chn];
    aenc_check_cb_null_ptr_return(aenc_chn->strm_buf);

    if (aenc_chn->strm_buf.buf_len == *wr_tail) {
        *wr_tail = 0;
    }

    *(aenc_chn->strm_buf.write_tail) = *wr_tail;
    *(aenc_chn->strm_buf.write_head) = *wr_tail;

    return TD_SUCCESS;
}

static td_s32 aenc_set_mute(ot_aenc_chn ae_chn, const td_bool *mute)
{
    aenc_chn_ctx *ae_chn_ctx_tmp = TD_NULL;

    aenc_check_chn_return(ae_chn);
    aenc_check_null_ptr_return(mute);
    ae_chn_ctx_tmp = &g_aenc_chn[ae_chn];

    if (osal_down_interruptible(&(ae_chn_ctx_tmp->sem))) {
        return -ERESTARTSYS;
    }

    if (ae_chn_ctx_tmp->created == TD_FALSE) {
        osal_up(&ae_chn_ctx_tmp->sem);
        aenc_err_trace("Aenc chn%d is disable!\n", ae_chn);
        return OT_ERR_AENC_NOT_CFG;
    }

    ae_chn_ctx_tmp->mute = *mute;

    osal_up(&(ae_chn_ctx_tmp->sem));

    return TD_SUCCESS;
}

static td_s32 aenc_get_mute(ot_aenc_chn ae_chn, td_bool *mute)
{
    aenc_chn_ctx *ae_chn_ctx_tmp = TD_NULL;

    aenc_check_null_ptr_return(mute);
    aenc_check_chn_return(ae_chn);
    ae_chn_ctx_tmp = &g_aenc_chn[ae_chn];

    if (osal_down_interruptible(&(ae_chn_ctx_tmp->sem))) {
        return -ERESTARTSYS;
    }

    if (ae_chn_ctx_tmp->created == TD_FALSE) {
        osal_up(&ae_chn_ctx_tmp->sem);
        aenc_err_trace("Aenc chn%d is disable\n", ae_chn);
        return OT_ERR_AENC_NOT_CFG;
    }

    *mute = ae_chn_ctx_tmp->mute;

    osal_up(&(ae_chn_ctx_tmp->sem));

    return TD_SUCCESS;
}

static td_s32 aenc_clr_buf(ot_aenc_chn ae_chn)
{
    aenc_chn_ctx *chn_ctx = TD_NULL;
    unsigned long lock_flag;

    aenc_check_chn_return(ae_chn);
    chn_ctx = &g_aenc_chn[ae_chn];
    aenc_check_cb_null_ptr_return(chn_ctx->strm_buf);

    if (osal_down_interruptible(&chn_ctx->sem)) {
        return -ERESTARTSYS;
    }

    if ((chn_ctx->created == TD_FALSE) || (chn_ctx->buf_init == TD_FALSE)) {
        osal_up(&chn_ctx->sem);
        aenc_err_trace("aenc chn %d is disable\n", ae_chn);
        return OT_ERR_AENC_NOT_CFG;
    }

    aenc_spin_lock_irqsave(&chn_ctx->spin_lock, lock_flag);

    valg_cb_reset(&(chn_ctx->strm_buf));

    aenc_frame_buf_put_free(&(chn_ctx->ai_frm_buf));

    if ((chn_ctx->voie_code_type != VOIE_CODE_TYPE_NONE) && (chn_ctx->voie_code_type != VOIE_CODE_TYPE_BUTT)) {
        aenc_frame_buf_put_free(&(chn_ctx->user_frm_buf));
    }

    aenc_spin_unlock_irqrestore(&chn_ctx->spin_lock, lock_flag);

    osal_up(&(chn_ctx->sem));

    return TD_SUCCESS;
}

static td_s32 aenc_open(void *private_data)
{
    if (osal_down(&g_aenc_sem) != 0) {
        aenc_err_trace("aenc open semaphore down fail!\n");
        return TD_FAILURE;
    }
    if (osal_atomic_read(&g_aenc_open_ref) >= MAX_OPEN_CNT) {
        osal_up(&g_aenc_sem);
        return TD_FAILURE;
    }
    if (check_is_same_pid(TD_TRUE) != TD_TRUE) {
        osal_up(&g_aenc_sem);
        return TD_FAILURE;
    }
    if (osal_atomic_inc_return(&g_aenc_open_ref) != 1) {
        osal_up(&g_aenc_sem);
        aenc_debug_trace(" aenc already initialization when first open\n");
        return TD_SUCCESS;
    }
    g_aenc_state = AENC_STATE_STARTED;
    osal_up(&g_aenc_sem);
    return TD_SUCCESS;
}

static td_s32 aenc_close(void *private_data)
{
    td_s32 i, ret;

    if (osal_down(&g_aenc_sem) != 0) {
        aenc_err_trace("aenc close semaphore down fail!\n");
        return TD_FAILURE;
    }

    if (osal_atomic_dec_return(&g_aenc_open_ref) == 0) {
        for (i = 0; i < OT_AENC_MAX_CHN_NUM; i++) {
            ret = aenc_strm_buf_exit(i);
            if (ret != TD_SUCCESS) {
                aenc_info_trace("aenc_strm_buf_exit fail, ret:0x%x.\n", (td_u32)ret);
            }
            ret = aenc_destroy_chn(i);
            if (ret != TD_SUCCESS) {
                aenc_info_trace("aenc_destroy_chn fail, ret:0x%x.\n", (td_u32)ret);
            }
        }
        g_aenc_state = AENC_STATE_STOPPED;
    }
    check_is_close();
    osal_up(&g_aenc_sem);
    return TD_SUCCESS;
}

static td_s32 aenc_get_voie_support(aenc_voie_support *voie_support)
{
    aenc_check_null_ptr_return(voie_support);

    voie_support->voie_support = TD_FALSE;
    if (aenc_is_support_voie()) {
#ifdef VOIE_SUPPORT
        voie_code_type code_type = VOIE_CODE_TYPE_NONE;
        voie_get_type(&voie_support->chn_attr, &code_type);

        if ((code_type != VOIE_CODE_TYPE_NONE) && (code_type != VOIE_CODE_TYPE_BUTT)) {
            voie_support->voie_support = TD_TRUE;
        }
#endif
    }

    return TD_SUCCESS;
}

static td_s32 aenc_ioctl_get_stream(ot_aenc_chn ae_chn, td_uintptr_t arg)
{
    aenc_stream_get *strm = (aenc_stream_get *)arg;
    aenc_check_null_ptr_return(strm);
    return aenc_get_stream(ae_chn, &(strm->stream), strm->mill_sec);
}

static td_u32 aenc_unused_cmd[] = {
    AENC_SET_MUTE,
    AENC_GET_MUTE,
    AENC_CLR_BUF
};

static td_s32 aenc_do_ioctl(unsigned int cmd, unsigned long ul_arg, void *private_data)
{
    td_u32 i;
    ot_aenc_chn ae_chn = umap_get_chn(private_data);
    td_uintptr_t arg = (td_uintptr_t)ul_arg;

#ifdef CONFIG_OT_AUDIO_ONLY_DRIVER_SUPPORT
    /* 只支持驱动的情况下无须开放ioctl，直接返回 */
    return TD_FAILURE;
#endif

    for (i = 0; i < (sizeof(aenc_unused_cmd) / sizeof(aenc_unused_cmd[0])); i++) {
        if (cmd == aenc_unused_cmd[i]) {
            aenc_err_trace("invalid aenc cmd!\n");
            return TD_FAILURE;
        }
    }

    switch (cmd) {
        /* follows cmd call local func --------------------------------------- */
        case AENC_READBUF_INIT_CTRL:
            return aenc_strm_buf_init(ae_chn, (aenc_cb_info *)arg);
        case AENC_READBUF_EXIT_CTRL:
            return aenc_strm_buf_exit(ae_chn);
        case AENC_CHN_GET_STRM_CTRL:
            return aenc_ioctl_get_stream(ae_chn, arg);
        case AENC_CHN_PUT_STRM_CTRL:
            return aenc_put_stream(ae_chn, (ot_audio_stream *)arg);
        case AENC_SET_MUTE:
            return aenc_set_mute(ae_chn, (td_bool *)arg);
        case AENC_GET_MUTE:
            return aenc_get_mute(ae_chn, (td_bool *)arg);
        case AENC_CLR_BUF:
            return aenc_clr_buf(ae_chn);

        /* -------------------------------------------------------------------- */
        case AENC_CREATE_CHN_CTRL:
            return aenc_create_chn(ae_chn, (ot_aenc_chn_attr *)arg);
        case AENC_DESTROY_CHN_CTRL:
            return aenc_destroy_chn(ae_chn);
        case AENC_GET_FRAME:
            return aenc_get_frame(ae_chn, (audio_frame_combine *)arg);
        case AENC_RELEASE_FRAME:
            return aenc_release_frame(ae_chn, (audio_frame_combine *)arg);
        case AENC_SEND_FRM_DBG_CTRL:
            return aenc_set_send_info(ae_chn, (aenc_send_frm_dbg *)arg);

        /* -------------------------------------------------------------------- */
#ifdef VOIE_SUPPORT
        case AENC_SEND_FRM_VOIE_CTRL:
            return aenc_send_frm_to_voie(ae_chn, (ot_audio_frame *)arg);
#endif
        case AENC_BINDCHN2FD_CTRL:
            aenc_check_null_ptr_return(arg);
            umap_set_chn(private_data, *((td_s32 *)arg));
            break;
        case AENC_GET_VOIE_SUPPORT:
            return aenc_get_voie_support((aenc_voie_support *)arg);
        case AENC_UPDATE_STREAM_WR_TAIL:
            return aenc_update_stream_buf_wr_tail(ae_chn, (td_u32 *)arg);
        case AENC_WAKEUP_GETSTREAM:
            return aenc_wakeup_get_stream(ae_chn);
        default:
            aenc_err_trace("aenc_do_ioctl: cmd 0x%x err, type:%u, NR:%u.\n", cmd, _IOC_TYPE(cmd), _IOC_NR(cmd));
            return TD_FAILURE;
    }

    return TD_SUCCESS;
}

#ifdef CONFIG_COMPAT
static long aenc_compat_ioctl(unsigned int cmd, unsigned long arg_org, void *private_data)
{
    td_s32 ret;
    td_uintptr_t arg = (td_uintptr_t)arg_org;

    if (g_aenc_state != AENC_STATE_STARTED) {
        aenc_warn_trace("MPP sys not init !\n");
        return OT_ERR_AENC_NOT_READY;
    }

    switch (cmd) {
        /* follows cmd call local func --------------------------------------- */
        case AENC_READBUF_INIT_CTRL:
        case AENC_READBUF_EXIT_CTRL:
        case AENC_CHN_GET_STRM_CTRL:
        case AENC_CHN_PUT_STRM_CTRL: {
            ot_audio_stream *strm = (ot_audio_stream *)arg;
            OT_COMPAT_POINTER(strm->stream, td_u8 *);
            break;
        }

        /* -------------------------------------------------------------------- */
        case AENC_CREATE_CHN_CTRL: {
            ot_aenc_chn_attr *attr = (ot_aenc_chn_attr *)arg;
            OT_COMPAT_POINTER(attr->value, td_void *);
            break;
        }

        case AENC_GET_VOIE_SUPPORT: {
            aenc_voie_support *voie_support = (aenc_voie_support *)arg;
            OT_COMPAT_POINTER(voie_support->chn_attr.value, td_void *);
            break;
        }

        default:
            break;
    }

    (td_void)osal_atomic_inc_return(&g_aenc_user_ref);
    ret = aenc_do_ioctl(cmd, arg_org, private_data);
    (td_void)osal_atomic_dec_return(&g_aenc_user_ref);

    return ret;
}
#endif

static unsigned int aenc_do_poll(osal_poll_t *poll, const void *data)
{
    td_s32 ae_chn = umap_get_chn(data);
    aenc_chn_ctx *aenc_chn = TD_NULL;
    unsigned int mask = 0;

    aenc_check_chn_return(ae_chn);

    aenc_chn = &g_aenc_chn[ae_chn];

    osal_poll_wait(poll, &aenc_chn->strm_wait);

    if (aenc_chn->buf_init == TD_FALSE) {
        aenc_notice_trace("aenc poll, chn %d not create\n", ae_chn);
        return mask;
    }

    if (valg_cb_get_data_len(&aenc_chn->strm_buf) > 0) {
        mask |= OSAL_POLLIN | OSAL_POLLRDNORM;
    }

    return mask;
}

static long aenc_ioctl(unsigned int cmd, unsigned long arg, void *private_data)
{
    int ret;
    if (check_is_same_pid(TD_FALSE) != TD_TRUE) {
        return OT_ERR_AENC_NOT_PERM;
    }
    if (g_aenc_state != AENC_STATE_STARTED) {
        aenc_warn_trace("MPP sys not init !\n");
        return OT_ERR_AENC_NOT_READY;
    }

    (td_void)osal_atomic_inc_return(&g_aenc_user_ref);
    ret = aenc_do_ioctl(cmd, arg, private_data);
    (td_void)osal_atomic_dec_return(&g_aenc_user_ref);

    return ret;
}

static unsigned int aenc_poll(osal_poll_t *poll, void *data)
{
    unsigned int ret;
    (td_void)osal_atomic_inc_return(&g_aenc_user_ref);
    ret = aenc_do_poll(poll, data);
    (td_void)osal_atomic_dec_return(&g_aenc_user_ref);

    return ret;
}

static struct osal_fileops g_aenc_fops = {
    .module = THIS_MODULE,
    .open = aenc_open,
    .release = aenc_close,
    .unlocked_ioctl = aenc_ioctl,
    .poll = aenc_poll,
#ifdef CONFIG_COMPAT
    .compat_ioctl = aenc_compat_ioctl
#endif
};

/* AENC 的MPP业务初始化函数 */
static td_s32 aenc_init(void *p)
{
    td_s32 ret;
    bind_receiver_info receiver_info = {0};

    ot_unused(p);
    if ((!ckfn_sys_entry()) || (!ckfn_sys_register_receiver())) {
        aenc_err_trace("sys module may not insert.\n");
        return OT_ERR_AENC_NOT_READY;
    }

    /* 向SYS模块注册接收端口 */
    receiver_info.mod_id = OT_ID_AENC;
    receiver_info.max_dev_cnt = 1;
    receiver_info.max_chn_cnt = OT_AENC_MAX_CHN_NUM;
    receiver_info.support_delay_data = TD_FALSE;
    receiver_info.call_back = aenc_receive_frm;
    ret = call_sys_register_receiver(&receiver_info);
    if (ret != TD_SUCCESS) {
        aenc_err_trace("register receiver failed with 0x%x!\n", (td_u32)ret);
        return OT_ERR_AENC_NOT_READY;
    }

    (td_void)osal_wait_init(&g_wait_user_frm);

    return TD_SUCCESS;
}

/* AENC 的MPP业务去初始化函数 */
static td_void aenc_exit(void)
{
    if ((ckfn_sys_entry()) && (ckfn_sys_unregister_receiver())) {
        call_sys_unregister_receiver(OT_ID_AENC);
    }
    osal_wait_destroy(&g_wait_user_frm);
    return;
}

static td_u32 aenc_get_ver_magic(td_void)
{
    return VERSION_MAGIC;
}

static td_void aenc_notify(mod_notice_id notice)
{
    td_s32 i;
    ot_unused(notice);
    for (i = 0; i < OT_AENC_MAX_CHN_NUM; i++) {
        if (osal_down_interruptible(&g_aenc_chn[i].sem) == TD_SUCCESS) {
            if (g_aenc_chn[i].created == TD_TRUE) {
                osal_wakeup(&g_aenc_chn[i].ai_frm_wait);
                osal_wakeup(&g_aenc_chn[i].strm_wait);
            }
            osal_up(&g_aenc_chn[i].sem);
        }
    }
    return;
}

static umap_module g_aenc_module = {
    .mod_id = OT_ID_AENC,
    .mod_name = "aenc",

    .pfn_init = aenc_init,
    .pfn_exit = aenc_exit,
    .pfn_ver_checker = aenc_get_ver_magic,
    .pfn_notify = aenc_notify,

    .data = TD_NULL,
};

static td_s32 aenc_init_chn(void)
{
    td_s32 i;
    aenc_chn_ctx *aenc_chn = TD_NULL;

    for (i = 0; i < OT_AENC_MAX_CHN_NUM; i++) {
        aenc_chn = &g_aenc_chn[i];

        (td_void)memset_s(aenc_chn, sizeof(aenc_chn_ctx), 0, sizeof(aenc_chn_ctx));
        aenc_chn->created = TD_FALSE;
        (td_void)osal_sema_init(&aenc_chn->sem, 1);
        (td_void)osal_wait_init(&aenc_chn->strm_wait);
        (td_void)osal_wait_init(&aenc_chn->ai_frm_wait);
        (td_void)aenc_spin_lock_init(&aenc_chn->spin_lock);
    }

    return TD_SUCCESS;
}

static td_void aenc_exit_chn(void)
{
    td_s32 i;
    aenc_chn_ctx *aenc_chn = TD_NULL;

    for (i = 0; i < OT_AENC_MAX_CHN_NUM; i++) {
        aenc_chn = &g_aenc_chn[i];
        aenc_spin_lock_destroy(&aenc_chn->spin_lock);
        osal_wait_destroy(&aenc_chn->ai_frm_wait);
        osal_wait_destroy(&aenc_chn->strm_wait);
        osal_sema_destroy(&aenc_chn->sem);
    }
}

static td_s32 aenc_register_dev(td_void)
{
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
    osal_proc_entry_t *sub_proc = TD_NULL;

    /* create proc interface */
    sub_proc = osal_create_proc_entry(PROC_ENTRY_AENC, TD_NULL);
    if (sub_proc == TD_NULL) {
        aenc_err_trace("aenc create proc failed\n");
        return TD_FAILURE;
    }
    sub_proc->read = aenc_proc_show;
#endif

    g_aenc_umap_dev = osal_createdev(UMAP_DEVNAME_AENC_BASE);
    if (g_aenc_umap_dev == TD_NULL) {
        aenc_err_trace("aenc create dev failed\n");
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
        osal_remove_proc_entry(PROC_ENTRY_AENC, TD_NULL);
#endif
        return TD_FAILURE;
    }

    g_aenc_umap_dev->fops = &g_aenc_fops;
    g_aenc_umap_dev->minor = UMAP_AENC_MINOR_BASE;

    if (osal_registerdevice(g_aenc_umap_dev) != TD_SUCCESS) {
        (td_void)osal_destroydev(g_aenc_umap_dev);
        g_aenc_umap_dev = TD_NULL;
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
        osal_remove_proc_entry(PROC_ENTRY_AENC, TD_NULL);
#endif
        return TD_FAILURE;
    }

    if (cmpi_register_module(&g_aenc_module)) {
        osal_deregisterdevice(g_aenc_umap_dev);
        (td_void)osal_destroydev(g_aenc_umap_dev);
        g_aenc_umap_dev = TD_NULL;
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
        osal_remove_proc_entry(PROC_ENTRY_AENC, TD_NULL);
#endif
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_void aenc_unregister_dev(td_void)
{
    cmpi_unregister_module(OT_ID_AENC);

    if (g_aenc_umap_dev != TD_NULL) {
        osal_deregisterdevice(g_aenc_umap_dev);
        (td_void)osal_destroydev(g_aenc_umap_dev);
        g_aenc_umap_dev = TD_NULL;
    }

#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
    osal_remove_proc_entry(PROC_ENTRY_AENC, TD_NULL);
#endif
}

td_s32 aenc_module_init(void)
{
    td_s32 ret;

    if (aenc_register_dev() != TD_SUCCESS) {
        return TD_FAILURE;
    }

    if (aenc_init_chn() != TD_SUCCESS) {
        aenc_unregister_dev();
        return TD_FAILURE;
    }

    ret = media_mem_file_init(&g_aenc_mfile, current->tgid, 1, (td_s32)OT_ID_AENC);
    if (ret != HI_SUCCESS) {
        aenc_exit_chn();
        aenc_unregister_dev();
        aenc_err_trace("dvpp mem file init fail ret = %d\n", ret);
        return ret;
    }
    ret = osal_atomic_init(&g_aenc_user_ref);
    if (ret < 0) {
        media_mem_file_uninit(&g_aenc_mfile);
        aenc_exit_chn();
        aenc_unregister_dev();
        return TD_FAILURE;
    }
    osal_atomic_set(&g_aenc_user_ref, 0);
    // 初始化aenc初始化引用计数
    ret = osal_atomic_init(&g_aenc_open_ref);
    if (ret < 0) {
        osal_atomic_destroy(&g_aenc_user_ref);
        media_mem_file_uninit(&g_aenc_mfile);
        aenc_exit_chn();
        aenc_unregister_dev();
        aenc_err_trace("aenc open_ref atomic init failed\n");
        return TD_FAILURE;
    }
    osal_atomic_set(&g_aenc_open_ref, 0);
    ret = osal_sema_init(&g_aenc_sem, 1);
    if (ret < 0) {
        osal_atomic_destroy(&g_aenc_open_ref);
        osal_atomic_destroy(&g_aenc_user_ref);
        media_mem_file_uninit(&g_aenc_mfile);
        aenc_exit_chn();
        aenc_unregister_dev();
        aenc_err_trace("aenc osal g_aenc_sem init fail\n");
        return TD_FAILURE;
    }
    aenc_info_trace("load aenc.ko ....OK!\n");

    return TD_SUCCESS;
}

void aenc_module_exit(void)
{
    td_s32 i;
    aenc_chn_ctx *aenc_chn = TD_NULL;
    osal_sema_destroy(&g_aenc_sem);
    cmpi_unregister_module(OT_ID_AENC);

    osal_atomic_destroy(&g_aenc_user_ref);
    osal_atomic_destroy(&g_aenc_open_ref);
    if (g_aenc_mfile.mm != TD_NULL) {
        media_mem_file_uninit(&g_aenc_mfile);
    }

    for (i = 0; i < OT_AENC_MAX_CHN_NUM; i++) {
        aenc_chn = &g_aenc_chn[i];

        aenc_chn->created = TD_FALSE;
        osal_sema_destroy(&aenc_chn->sem);
        osal_wait_destroy(&aenc_chn->strm_wait);
        osal_wait_destroy(&aenc_chn->ai_frm_wait);
        aenc_spin_lock_destroy(&aenc_chn->spin_lock);
    }

    if (g_aenc_umap_dev != TD_NULL) {
        osal_deregisterdevice(g_aenc_umap_dev);
        (td_void)osal_destroydev(g_aenc_umap_dev);
        g_aenc_umap_dev = TD_NULL;
    }

#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
    osal_remove_proc_entry(PROC_ENTRY_AENC, TD_NULL);
#endif

    if (aenc_is_support_voie()) {
#ifdef VOIE_SUPPORT
        voie_exit();
#endif
    }
    aenc_info_trace("unload aenc.ko ....OK!\n");

    return;
}
