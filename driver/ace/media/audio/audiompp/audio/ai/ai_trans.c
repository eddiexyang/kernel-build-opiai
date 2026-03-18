/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2023. All rights reserved.
 * Description: ai trans driver
 * Author: Hisilicon multimedia software group
 * Create: 2022/5/5
 */

#include <linux/printk.h>
#include "ai_trans.h"

#include "hi_osal.h"
#include "securec.h"

#include "hi_comm_audio.h"
#include "ai_drv.h"
#include "proc_ext.h"
#include "ao_ext.h"
#include "mkp_aio.h"
#include "mod_ext.h"

#include "sys_ext.h"
#include "mm_ext.h"
#ifdef CONFIG_OT_PLATFORM_V8
#include "hi_comm_vb.h"
#else
#include "ot_comm_vb.h"
#endif
#include "vb_ext.h"
#include "asp_dma_hal.h"

#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/delay.h>
#include "pid_protect.h"
#define AI_MMZ_NAME_LEN     16
#define AI_VB_POOL_NAME_LEN 16
#define DMA_BUF_BYTES_SIZE 1024

aio_dev_ctx g_ai_dev[OT_AI_DEV_MAX_NUM] = {0};
ai_chn_ctx g_ai_chn[OT_AI_MAX_CHN_NUM * OT_AI_DEV_MAX_NUM] = {0};

extern td_void ai_dma_callback(ot_audio_dev audio_dev_id);

#ifdef CONFIG_OT_AUDIO_ALSA_SUPPORT
td_s32 ai_set_alsa_dma_attr(ot_audio_dev ai_dev, cir_buf cir_buf_attr, td_u32 trans_len)
{
    td_s32 ret;
    ret = ai_drv_set_dma_attr(ai_dev, cir_buf_attr, trans_len);
    if (ret != TD_SUCCESS) {
        ai_warn_trace("ai_drv_set_dma_attr fail with ret = 0x%x.\n", ret);
    }
    return ret;
}
#endif

static td_bool ai_check_vqe(const aio_dev_ctx *ao_dev_ctx, ot_ao_chn ao_chn)
{
    if (ao_dev_ctx->aio_attr.bit_width > OT_AUDIO_BIT_WIDTH_16) {
        ai_warn_trace("ao bit_width invalid:%u\n", (td_u32)ao_dev_ctx->aio_attr.bit_width);
        return TD_FALSE;
    }

    if (ao_chn >= (ot_ao_chn)ao_dev_ctx->aio_attr.chn_cnt) {
        ai_warn_trace("ao chn for aec invalid,chn:%d\n", ao_chn);
        return TD_FALSE;
    }

    return TD_TRUE;
}

/* 各通道单独获取VB，申请到的VB信息放到左声道中 */
static td_s32 ai_get_frm_vb_blk(const aio_dev_ctx *dev_ctx, ot_audio_frame *frm)
{
    vb_blk_handle vb_handle;

    ai_check_null_ptr_return(dev_ctx);
    ai_check_null_ptr_return(frm);

    vb_handle = call_vb_get_blk_by_pool_id(dev_ctx->pool_id, OT_VB_UID_AI);
    if (vb_handle == OT_VB_INVALID_HANDLE) {
        return TD_FAILURE;
    }

    frm->bit_width = dev_ctx->aio_attr.bit_width;
    frm->snd_mode = dev_ctx->aio_attr.snd_mode;
    frm->virt_addr[0] = (td_u8 *)(td_uintptr_t)(call_vb_handle_to_kern(vb_handle));
    frm->phys_addr[0] = call_vb_handle_to_phys(vb_handle);
    frm->pool_id[0] = call_vb_handle_to_pool_id(vb_handle);
    frm->virt_addr[1] = TD_NULL;
    frm->phys_addr[1] = 0;
    frm->pool_id[1] = 0;

    return TD_SUCCESS;
}

static td_s32 ai_put_frm_vb_blk(const ot_audio_frame *frm)
{
    ai_check_null_ptr_return(frm);

    call_vb_user_sub(frm->pool_id[0], frm->phys_addr[0], OT_VB_UID_AI);

    return TD_SUCCESS;
}

/* 需确保传进来的结构体中的VB有效 */
td_s32 ai_add_vb(const audio_frame_combine *audio_frm, td_u32 uid)
{
    td_s32 i, chn_per_frm, ret;

    ai_check_null_ptr_return(audio_frm);
    chn_per_frm = aio_chn_num_per_frm(audio_frm->frm.snd_mode);

    for (i = 0; i < chn_per_frm; i++) {
        ret = call_vb_user_add(audio_frm->frm.pool_id[i], audio_frm->frm.phys_addr[i], uid);
        if (ret != TD_SUCCESS) {
            ai_err_trace("call_vb_user_add failed with 0x%x!\n", (td_u32)ret);
        }
        if (audio_frm->ref_frm.valid == TD_TRUE) {
            ret = call_vb_user_add(audio_frm->ref_frm.ref_frame.pool_id[i],
                                   audio_frm->ref_frm.ref_frame.phys_addr[i], uid);
            if (ret != TD_SUCCESS) {
                ai_err_trace("call_vb_user_add failed with 0x%x!\n", (td_u32)ret);
            }
        }
    }

    return TD_SUCCESS;
}

td_s32 ai_sub_vb(const audio_frame_combine *audio_frm, td_u32 uid)
{
    td_s32 i, chn_per_frm, ret;
    td_u32 cnt = 0;
    td_u32 ref_cnt = 0;

    ai_check_null_ptr_return(audio_frm);
    chn_per_frm = aio_chn_num_per_frm(audio_frm->frm.snd_mode);

    for (i = 0; i < chn_per_frm; i++) {
        ret = call_vb_inquire_pool_user_cnt(audio_frm->frm.pool_id[i], uid, &cnt);
        if (ret != TD_SUCCESS) {
            ai_err_trace("get user cnt of the audio frame failed!\n");
            return TD_FAILURE;
        }

        if (cnt == 0) {
            ai_err_trace("vb user count of the audio frame is 0, it maybe has been released!\n");
            return TD_FAILURE;
        }

        ret = call_vb_user_sub(audio_frm->frm.pool_id[i], audio_frm->frm.phys_addr[i], uid);
        if (ret != TD_SUCCESS) {
            ai_err_trace("audio_frm:0x%x, pool_id:%u, phys_addr:0x%llx, uid:%u, i:%d.\n",
                         (td_u32)(td_uintptr_t)audio_frm, audio_frm->frm.pool_id[i],
                         (td_u64)audio_frm->frm.phys_addr[i], uid, i);
            return TD_FAILURE;
        }

        if (audio_frm->ref_frm.valid != TD_TRUE) {
            continue;
        }

        ret = call_vb_inquire_pool_user_cnt(audio_frm->ref_frm.ref_frame.pool_id[i], uid, &ref_cnt);
        if (ret != TD_SUCCESS) {
            ai_err_trace("get user cnt of the audio ref_frame failed!\n");
            return TD_FAILURE;
        }

        if (ref_cnt > 0) {
            ret = call_vb_user_sub(audio_frm->ref_frm.ref_frame.pool_id[i],
                                   audio_frm->ref_frm.ref_frame.phys_addr[i], uid);
            if (ret != TD_SUCCESS) {
                ai_err_trace("ret: 0x%x, i: %d, pool_id[0]:%u, phys_addr[0]:0x%llx, uid: %u.\n",
                             (td_u32)ret, i,
                             audio_frm->ref_frm.ref_frame.pool_id[i],
                             (td_u64)audio_frm->ref_frm.ref_frame.phys_addr[i], uid);
                return TD_FAILURE;
            }
        }
    }

    return TD_SUCCESS;
}

td_s32 ai_get_pool_user_cnt(ot_audio_dev ai_dev, td_u32 *vb_user_cnt)
{
    aio_dev_ctx *dev_ctx = TD_NULL;
    td_u32 i, use_cnt, all_use_cnt;
    td_s32 ret;

    ai_check_dev_return(ai_dev);
    ai_check_null_ptr_return(vb_user_cnt);

    dev_ctx = ai_get_dev_ctx(ai_dev);

    /* 统计其它模块在使用AI的总VB数量 */
    all_use_cnt = 0;
    for (i = 0; i < OT_VB_MAX_USER; i++) {
        use_cnt = 0;

        ret = call_vb_inquire_pool_user_cnt(dev_ctx->pool_id, i, &use_cnt);
        if (ret != TD_SUCCESS) {
            ai_info_trace("get user cnt of pool failed\n");
            continue;
        }

        if ((use_cnt != 0) && (i != OT_VB_UID_AI)) {
            all_use_cnt += use_cnt;
        }
    }

    *vb_user_cnt = all_use_cnt;

    return TD_SUCCESS;
}

static td_s32 ai_free_exceed_busy_frm(ot_audio_dev ai_dev, ot_ai_chn ai_chn,
    td_u32 exceed_num, td_u32 aie_busy_num)
{
    td_s32 ret;
    td_u32 busy_num, i;
    audio_buf_blk *node = TD_NULL;
    td_s32 chn_id = ai_get_chn_id(ai_dev, ai_chn);
    if (g_ai_chn[chn_id].chn_attr.mode == OT_AI_CHN_MODE_NORMAL) {
        /* free aie 段. */
        busy_num = (exceed_num > aie_busy_num) ? aie_busy_num : exceed_num;
        for (i = 0; i < busy_num; i++) {
            node = audio_fb_get_busy(&g_ai_chn[chn_id].aie_frm_buf);
            if (node == TD_NULL) {
                ai_err_trace("ai chn(%d,%d) aie frame node is null.\n", ai_dev, ai_chn);
                return OT_ERR_AI_NULL_PTR;
            }

            ret = ai_sub_vb(&node->audio_frm, OT_VB_UID_AI);
            if (ret != TD_SUCCESS) {
                ai_err_trace("AI dev %d chn %d sub vb fail\n", ai_dev, ai_chn);
                return OT_ERR_AI_NOT_PERM;
            }

            ret = audio_fb_put_free(&g_ai_chn[chn_id].aie_frm_buf, node);
            if (ret != 0) {
                ai_err_trace("free aie fb free fail in ai free exceed busy frm ret = %d\n", ret);
                return ret;
            }
        }
    } else {
        busy_num = 0;
    }

    /* free user 段. */
    busy_num = exceed_num - busy_num;
    for (i = 0; i < busy_num; i++) {
        node = audio_fb_get_busy(&g_ai_chn[chn_id].user_frm_buf);
        if (node == TD_NULL) {
            ai_err_trace("ai chn(%d,%d) user frame node is null.\n", ai_dev, ai_chn);
            return OT_ERR_AI_NULL_PTR;
        }

        ret = ai_sub_vb(&node->audio_frm, OT_VB_UID_AI);
        if (ret != TD_SUCCESS) {
            ai_err_trace("AI dev %d chn %d sub vb fail\n", ai_dev, ai_chn);
            return OT_ERR_AI_NOT_PERM;
        }

        ret = audio_fb_put_free(&g_ai_chn[chn_id].user_frm_buf, node);
        if (ret != TD_SUCCESS) {
            ai_err_trace("ai put user fb free fail ret = %d\n", ret);
            return ret;
        }
    }

    return TD_SUCCESS;
}

static td_s32 ai_usr_frm_rsv_data(ot_audio_dev audio_dev_id, ot_ai_chn ai_chn, const audio_frame_combine *audio_frm)
{
    td_s32 ret;
    td_u32 depth, user_busy_num, aie_busy_num, total_busy_num, exceed_num;
    audio_buf_blk *node = TD_NULL;

    td_s32 chn_id = ai_get_chn_id(audio_dev_id, ai_chn);
    audio_frame_buf *user_buf = &g_ai_chn[chn_id].user_frm_buf;
    audio_frame_buf *aie_buf = &g_ai_chn[chn_id].aie_frm_buf;

    ai_check_null_ptr_return(audio_frm);

    depth = g_ai_chn[chn_id].chn_param.usr_frame_depth;
    user_busy_num = audio_fb_query_busy_num(user_buf);
    if (g_ai_chn[chn_id].chn_attr.mode == OT_AI_CHN_MODE_NORMAL) {
        aie_busy_num = audio_fb_query_busy_num(aie_buf);
    } else {
        aie_busy_num = 0;
    }
    total_busy_num = user_busy_num + aie_busy_num;

    /*
     * 如果total_busy_num<depth，从freelist队头获取一个节点挂到busylist队尾；
     * 如果total_busy_num>=depth，先从busylist摘busy_num-depth个节点到freelist，
     * 再判断depth是否为0，否则从busylist队头摘一个节点到队尾。
     */
    if (total_busy_num >= depth) {
        exceed_num = (total_busy_num - depth) + 1;
        ret = ai_free_exceed_busy_frm(audio_dev_id, ai_chn, exceed_num, aie_busy_num);
        if (ret != TD_SUCCESS) {
            return ret;
        }
    }

    node = audio_fb_get_free(user_buf);
    if (node == TD_NULL) {
        ai_err_trace("ai chn(%d,%d) frame node is null.\n", audio_dev_id, ai_chn);
        return OT_ERR_AI_NULL_PTR;
    }

    (td_void)memcpy_s(&node->audio_frm, sizeof(node->audio_frm), audio_frm, sizeof(*audio_frm));
    ret = audio_fb_put_busy(user_buf, node);
    if (ret != TD_SUCCESS) {
        ai_err_trace("audio fb put busy faile ret = %d\n", ret);
        return ret;
    }
    ret = ai_add_vb(audio_frm, OT_VB_UID_AI);
    if (ret != TD_SUCCESS) {
        ai_err_trace("AI dev %d chn %d add vb fail\n", audio_dev_id, ai_chn);
        return OT_ERR_AI_NOT_PERM;
    }

    /* 成功获取音频帧后唤醒等待队列 */
    osal_wakeup(&g_ai_chn[chn_id].user_wait);
    return TD_SUCCESS;
}

static td_void ai_frm_24bit_to_16bit(td_u8 *virt_addr, td_u32 point_num_per_frame)
{
    td_u32 temp, i;
    td_u32 *src_buf = (td_u32 *)(td_void *)virt_addr;
    td_u16 *dest_buf = (td_u16 *)(td_void *)virt_addr;
    for (i = 0; i < point_num_per_frame; i++) {
        temp = (*src_buf) >> 16; /* 24bit位宽数据（用4字节的高24bit储存）右移16位成为16bit位宽数据 */
        *dest_buf = (td_u16)temp;
        src_buf++;
        dest_buf++;
    }
}

static td_void ai_frm_extra_proc(ot_audio_dev ai_dev, ot_ai_chn ai_chn, audio_frame_combine *audio_frm)
{
    aio_dev_ctx *dev_ctx = &g_ai_dev[ai_dev];

    /* 立体声处理 */
    if (dev_ctx->aio_attr.snd_mode == OT_AUDIO_SOUND_MODE_STEREO) {
        td_s32 chn_id = ai_get_chn_id(ai_dev, ai_chn);
        ot_audio_frame *frm_tmp = TD_NULL;

        chn_id += (td_s32)(dev_ctx->aio_attr.chn_cnt / 2); /* 2: stereo */
        if (g_ai_chn[chn_id].frm_valid == TD_FALSE) {
            goto cut_24bit_proc;
        }

        /* 如果是立体声，还要额外拷贝右声道的vb信息 */
        frm_tmp = &g_ai_chn[chn_id].audio_frm.frm;
        audio_frm->frm.pool_id[1] = frm_tmp->pool_id[0];
        audio_frm->frm.phys_addr[1] = frm_tmp->phys_addr[0];
        audio_frm->frm.virt_addr[1] = frm_tmp->virt_addr[0];

        /* 立体声的回声抵消参考帧恒无效 */
        audio_frm->ref_frm.valid = TD_FALSE;
    }

cut_24bit_proc:
    /* 24bit裁剪处理 */
    if ((dev_ctx->aio_attr.bit_width == OT_AUDIO_BIT_WIDTH_24) && (dev_ctx->aio_attr.expand_flag == OT_AI_CUT)) {
        ai_frm_24bit_to_16bit(audio_frm->frm.virt_addr[0], dev_ctx->aio_attr.point_num_per_frame);
        if ((dev_ctx->aio_attr.snd_mode == OT_AUDIO_SOUND_MODE_STEREO) && (audio_frm->frm.virt_addr[1] != TD_NULL)) {
            ai_frm_24bit_to_16bit(audio_frm->frm.virt_addr[1], dev_ctx->aio_attr.point_num_per_frame);
        }
        audio_frm->frm.bit_width = OT_AUDIO_BIT_WIDTH_16;
        audio_frm->frm.len = dev_ctx->aio_attr.point_num_per_frame << 1;
    }
}

static td_void ai_trans_call_sys_send(ot_audio_dev aio_dev, ot_ai_chn ai_chn, audio_frame_combine *audio_frm)
{
#ifndef CONFIG_OT_PLATFORM_V8
    ot_mpp_chn mpp_chn;
    call_sys_set_mpp_chn(&mpp_chn, OT_ID_AI, aio_dev, ai_chn);
    call_sys_send_data(&mpp_chn, TD_FALSE, MPP_DATA_AUDIO_FRAME, audio_frm);
#else
    call_sys_send_data(OT_ID_AI, aio_dev, ai_chn, TD_FALSE, MPP_DATA_AUDIO_FRAME, audio_frm);
#endif
}

static td_s32 ai_send_chn_data(ot_audio_dev audio_dev_id)
{
    td_s32 ai_chn, ret, chn_id, chn_per_frm;
    aio_dev_ctx *aio_dev = TD_NULL;
    audio_frame_combine audio_frm;

    if ((((td_bool)ckfn_sys_entry()) == TD_FALSE) || (((td_bool)ckfn_sys_send_data()) == TD_FALSE)) {
        ai_err_trace("sys module may not insert.\n");
        return OT_ERR_AI_NOT_READY;
    }

    aio_dev = &g_ai_dev[audio_dev_id];
    /* 调用函数已做检测，这里的检测没必要 chn_per_frm 只会为1或2，不会为0 */
    chn_per_frm = aio_chn_num_per_frm(aio_dev->aio_attr.snd_mode);
    for (ai_chn = 0; ai_chn < (td_s32)((td_s32)aio_dev->aio_attr.chn_cnt / chn_per_frm); ai_chn++) {
        chn_id = ai_get_chn_id(audio_dev_id, ai_chn);
        /* 如果某通道没有使能，则不对其进行操作 */
        if (g_ai_chn[chn_id].chn_state != AIO_CHN_ENABLE) {
            continue;
        }

        /* 检查音频帧有效性，并合并左右声道音频帧到一个结构体中 */
        if (g_ai_chn[chn_id].frm_valid == TD_FALSE) {
            continue;
        }

        (td_void)memcpy_s(&audio_frm, sizeof(audio_frm),
            &g_ai_chn[chn_id].audio_frm, sizeof(g_ai_chn[chn_id].audio_frm));

        ai_frm_extra_proc(audio_dev_id, ai_chn, &audio_frm);

        /* 发送数据给user_buf */
        ret = ai_usr_frm_rsv_data(audio_dev_id, ai_chn, &audio_frm);
        if (ret != TD_SUCCESS) {
            ai_warn_trace("AI dev %d chn %d send usr frame fail\n", audio_dev_id, ai_chn);
        }
#ifndef CONFIG_AUDIO_V200_SUPPORT
        /* Fast模式下通过系统绑定发送给AENC/AO */
        if (g_ai_chn[chn_id].chn_attr.mode == OT_AI_CHN_MODE_FAST) {
            ai_trans_call_sys_send(audio_dev_id, ai_chn, &audio_frm);
        }
#endif
    }

    /* 之所以不在上面的循环体内发送完音频帧之后释放vb，是考虑到可能右声道vb没分到，直接break */
    for (ai_chn = 0; ai_chn < (td_s32)(aio_dev->aio_attr.chn_cnt); ai_chn++) {
        chn_id = ai_get_chn_id(audio_dev_id, ai_chn);
        if (g_ai_chn[chn_id].frm_valid == TD_TRUE) {
            /* 每个通道的音频帧都存放在左声道，只释放左声道VB */
            g_ai_chn[chn_id].audio_frm.frm.snd_mode = OT_AUDIO_SOUND_MODE_MONO;
            ret = ai_sub_vb(&g_ai_chn[chn_id].audio_frm, OT_VB_UID_AI);
            if (ret != TD_SUCCESS) {
                ai_warn_trace("ai sub vb fail in ai send chn data\n");
            }
        }
    }

    return TD_SUCCESS;
}

static td_void save_chn_read_ptr(const aio_dev_ctx *aio_dev, ot_audio_dev audio_dev_id)
{
    td_s32 ai_chn, chn_id;
    ai_chn_ctx *chn_ctx = TD_NULL;

    for (ai_chn = 0; ai_chn < (td_s32)(aio_dev->aio_attr.chn_cnt); ai_chn++) {
        chn_id = ai_get_chn_id(audio_dev_id, ai_chn);
        chn_ctx = &g_ai_chn[chn_id];

        /* AEC延时方案：获取AO的读指针 */
        if (chn_ctx->aec_ao_dev != -1) {
            ot_audio_dev ao_dev = chn_ctx->aec_ao_dev;
            ai_warn_trace("save_chn_read_ptr entry\n");
            ao_export_func *pfn_ao_export_func = TD_NULL;
            pfn_ao_export_func = (ao_export_func *)cmpi_get_module_func_by_id(OT_ID_AO);
            if ((pfn_ao_export_func == TD_NULL) || (pfn_ao_export_func->pfn_save_ao_rdptr == TD_NULL)) {
                ai_warn_trace("AI chn %d: AO has no export functions!\n", ai_chn);
                continue;
            }

            pfn_ao_export_func->pfn_save_ao_rdptr(ao_dev);
        }
    }
}

static td_void get_vb_blk(aio_dev_ctx *aio_dev, ot_audio_dev audio_dev_id,
    ot_audio_frame **ai_frm_handle, ot_aec_frame **aec_frm_handle)
{
    td_s32 ai_chn, chn_id, ret;
    ai_chn_ctx *chn_ctx = TD_NULL;
    ot_audio_frame *ai_frm = *ai_frm_handle;
    ot_aec_frame *aec_frm = *aec_frm_handle;

    for (ai_chn = 0; ai_chn < (td_s32)ASP_SUPPORT_MAX_CHN_CNT; ai_chn++) {
        chn_id = ai_get_chn_id(audio_dev_id, ai_chn);
        chn_ctx = &g_ai_chn[chn_id];

        if (chn_ctx->ai_vqe_dbg.vqe_enable || chn_ctx->resmp_dbg.resmp_enable) {
            chn_ctx->audio_frm.enable_vqe = TD_TRUE;
        } else {
            chn_ctx->audio_frm.enable_vqe = TD_FALSE;
        }

        ai_frm = &chn_ctx->audio_frm.frm;
        aec_frm = &chn_ctx->audio_frm.ref_frm;

        chn_ctx->frm_valid = TD_FALSE;
        aio_dev->ast_chn_data_addr[ai_chn].valid = TD_FALSE;
        aec_frm->valid = TD_FALSE;
        aec_frm->sys_bind = TD_FALSE;
        ai_frm->len = 0;             /* set len to 0,means this pack is invalid */
        aec_frm->ref_frame.len = 0; /* set len to 0,means this pack is invalid */

        /* 如果某通道没有使能，则不对其进行操作 */
        if (chn_ctx->chn_state != AIO_CHN_ENABLE) {
            continue;
        }

        /* 为音频帧分配VB */
        ret = ai_get_frm_vb_blk(aio_dev, ai_frm);
        if (ret != TD_SUCCESS) {
            chn_ctx->int_lost++;
            ai_err_trace("AI chn %d get vb failed!\n", ai_chn);
            continue;
        } else {
            chn_ctx->frm_valid = TD_TRUE;

            aio_dev->ast_chn_data_addr[ai_chn].valid = TD_TRUE;
            aio_dev->ast_chn_data_addr[ai_chn].phys_addr = ai_frm->phys_addr[0];
            aio_dev->ast_chn_data_addr[ai_chn].virt_addr = ai_frm->virt_addr[0];
        }
    }

    *ai_frm_handle = ai_frm;
    *aec_frm_handle = aec_frm;
}

static td_void save_aec_frm(const aio_dev_ctx *ai_dev_ctx, ai_chn_ctx *chn_ctx, td_s32 ai_chn,
    const ot_audio_frame *ai_frm, ot_aec_frame *aec_frm)
{
    td_s32 ret;

    aio_dev_ctx *dev_ctx = TD_NULL;
    ot_audio_dev ao_dev = chn_ctx->aec_ao_dev;
    ot_ao_chn ao_chn = chn_ctx->aec_ao_chn;
    ao_export_func *pfn_ao_export_func = TD_NULL;
    pfn_ao_export_func = (ao_export_func *)cmpi_get_module_func_by_id(OT_ID_AO);
    if ((pfn_ao_export_func == TD_NULL) || (pfn_ao_export_func->pfn_get_dma_inf == TD_NULL) ||
        (pfn_ao_export_func->pfn_get_data == TD_NULL)) {
        ai_warn_trace("AI chn %d: AO has no export functions!\n", ai_chn);
        return;
    }

    dev_ctx = pfn_ao_export_func->pfn_get_dma_inf(ao_dev);
    if ((dev_ctx != TD_NULL) && (dev_ctx->enable == TD_TRUE) && (((chn_ctx->vqe_enable == TD_TRUE) &&
        (ai_check_vqe(dev_ctx, ao_chn) == TD_TRUE)) ||
        (chn_ctx->enable_aec_ref_frame == TD_TRUE))) {
        /* 回声抵消参考帧获取不成功，不影响正常音频帧 */
        ret = ai_get_frm_vb_blk(ai_dev_ctx, &aec_frm->ref_frame);
        if (ret != TD_SUCCESS) {
            chn_ctx->aec_fail++;
            ai_err_trace("AI chn %d get aec vb failed!\n", ai_chn);
            return;
        }
        aec_frm->ref_frame.len = ai_frm->len;
        aec_frm->ref_frame.bit_width = ai_frm->bit_width;
        aec_frm->ref_frame.snd_mode = ai_frm->snd_mode;

        if (pfn_ao_export_func->pfn_get_data(ao_dev, ao_chn, &aec_frm->ref_frame) == TD_SUCCESS) {
            /* pts and seq is equal to ai pack */
            aec_frm->ref_frame.time_stamp = ai_frm->time_stamp;
            aec_frm->ref_frame.seq = ai_frm->seq;
            aec_frm->valid = TD_TRUE;
            aec_frm->sys_bind = TD_TRUE;
        } else {
            (void)ai_put_frm_vb_blk(&aec_frm->ref_frame);
            aec_frm->ref_frame.len = 0; /* set len to 0,means this pack is invalid */
            aec_frm->valid = TD_FALSE;
            aec_frm->sys_bind = TD_FALSE;
            chn_ctx->aec_fail++;
            ai_err_trace("AI chn %d get aec frame failed!\n", ai_chn);
        }
    }
}

static td_s32 ai_get_chn_data(ot_audio_dev audio_dev_id)
{
    aio_dev_ctx *aio_dev = &g_ai_dev[audio_dev_id];
    ai_chn_ctx *chn_ctx = TD_NULL;
    ot_audio_frame *ai_frm = TD_NULL;
    ot_aec_frame *aec_frm = TD_NULL;
    td_s32 ai_chn, chn_id, ret;
    td_u64 pts = 0;

    /* 准备各通道音频帧VB */
    save_chn_read_ptr(aio_dev, audio_dev_id);

    /* 准备各通道音频帧VB */
    get_vb_blk(aio_dev, audio_dev_id, &ai_frm, &aec_frm);
    /* 从DMA BUF中获取各通道音频帧数据 */
    ret = ai_drv_get_chn_data(audio_dev_id, aio_dev->ast_chn_data_addr, ASP_SUPPORT_MAX_CHN_CNT);
    if (ret != TD_SUCCESS) {
        /* 释放各通道vb */
        for (ai_chn = 0; ai_chn < (td_s32)ASP_SUPPORT_MAX_CHN_CNT; ai_chn++) {
            chn_id = ai_get_chn_id(audio_dev_id, ai_chn);
            chn_ctx = &g_ai_chn[chn_id];

            if (chn_ctx->frm_valid == TD_TRUE) {
                (td_void)ai_put_frm_vb_blk(&chn_ctx->audio_frm.frm);
                chn_ctx->frm_valid = TD_FALSE;
                chn_ctx->int_lost++;
            }
        }

        ai_err_trace("AI dev:%d separate channel data from dma buffer failed!\n", audio_dev_id);
        return ret;
    }

    if ((((td_bool)ckfn_sys_entry()) == TD_TRUE) && (((td_bool)ckfn_sys_get_time_stamp()) == TD_TRUE)) {
        pts = call_sys_get_time_stamp();
    }

    for (ai_chn = 0; ai_chn < (td_s32)ASP_SUPPORT_MAX_CHN_CNT; ai_chn++) {
        chn_id = ai_get_chn_id(audio_dev_id, ai_chn);
        chn_ctx = &g_ai_chn[chn_id];

        if (chn_ctx->frm_valid != TD_TRUE) {
            continue;
        }

        ai_frm = &chn_ctx->audio_frm.frm;
        aec_frm = &chn_ctx->audio_frm.ref_frm;
        /* 时间戳等信息 */
        ai_frm->time_stamp = pts;
        ai_frm->seq++;

        /* 计算音频帧的长度 */
        if ((aio_dev->aio_attr.expand_flag == OT_AI_EXPAND) && (aio_dev->aio_attr.bit_width == OT_AUDIO_BIT_WIDTH_8)) {
            ai_frm->len = (aio_dev->aio_attr.point_num_per_frame << 1);
            ai_frm->bit_width = OT_AUDIO_BIT_WIDTH_16;
        } else if (aio_dev->aio_attr.bit_width == OT_AUDIO_BIT_WIDTH_24) {
            // 将24bit数据转换为32bit
            ai_frm->len = (aio_dev->aio_attr.point_num_per_frame <<
                          ((td_u32)aio_dev->aio_attr.bit_width)) * 3 / 4; // 3: 24bit 数据占3bytes,4:32bit数据占4bytes
            ai_frm->bit_width = aio_dev->aio_attr.bit_width;
        } else {
            ai_frm->len = (aio_dev->aio_attr.point_num_per_frame << ((td_u32)aio_dev->aio_attr.bit_width));
            ai_frm->bit_width = aio_dev->aio_attr.bit_width;
        }

        /* 从AO获取回声抵消参考帧数据 */
        if (chn_ctx->aec_ao_dev != -1) {
            save_aec_frm(aio_dev, chn_ctx, ai_chn, ai_frm, aec_frm);
        }
    }

    return TD_SUCCESS;
}

td_s32 ai_dma_buf_init(ot_audio_dev audio_dev_id)
{
    td_u32 dma_buf_bytes; /* DMA buf是ping-pong buf */
    td_u32 total_bytes;
    aio_dev_ctx *aio_dev = &g_ai_dev[audio_dev_id];
    td_char ac_name[AI_MMZ_NAME_LEN] = { 0 };
    td_s32 ret;
    td_char *mmz_name = TD_NULL;
    ot_mpp_chn chn;

    ret = ai_drv_dma_buf_info_init(audio_dev_id, &dma_buf_bytes);
    if (ret != TD_SUCCESS) {
        ai_warn_trace("calculate dma buffer size fail!\n");
        return ret;
    }

    total_bytes = dma_buf_bytes;
    if (total_bytes % 128 != 0) { /* 128: 128字节对齐 */
        total_bytes = (total_bytes / 128 + 1) * 128; /* 128: 128字节对齐 */
    }
    /* 分配buf大小 */
    chn.mod_id = OT_ID_AI;
    chn.dev_id = audio_dev_id;
    chn.chn_id = 0;
    if ((((td_bool)ckfn_sys_entry()) == TD_FALSE) || (((td_bool)ckfn_sys_get_mmz_name()) == TD_FALSE)) {
        ai_err_trace("sys module may not insert.\n");
        return OT_ERR_AI_NOT_READY;
    }

    if (call_sys_get_mmz_name((hi_mpp_chn *)&chn, (hi_void **)&mmz_name) != TD_SUCCESS) {
        ai_warn_trace("get mmz name fail!\n");
        return OT_ERR_AI_NO_MEM;
    }

    ret = snprintf_s(ac_name, AI_MMZ_NAME_LEN, AI_MMZ_NAME_LEN - 1, "ai(%d)_dma", audio_dev_id);
    if (ret < EOK) {
        ai_err_trace("dma buf name snprintf_s err, ret = 0x%d\n", ret);
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

#ifndef CONFIG_OT_PLATFORM_V8
    ret = cmpi_mmz_malloc_nocache(mmz_name, ac_name, (td_phys_addr_t *)(td_void *)&aio_dev->dma_phy_addr,
        (td_void *)&aio_dev->dma_vir_addr, total_bytes);
#else
    ret = cmpi_mmz_malloc_nocache(mmz_name, ac_name, &aio_dev->dma_phy_addr,
        (td_void *)&aio_dev->dma_vir_addr, ((td_ulong)total_bytes + (td_ulong)DMA_BUF_BYTES_SIZE));
#endif
    if (ret != TD_SUCCESS) {
        ai_emerg_trace("alloc dma buf err\n");
        return OT_ERR_AI_NO_MEM;
    }

    /* 消除acodec上电后跑业务后的初次pop音，但11k无效? */
    (td_void)memset_s(aio_dev->dma_vir_addr, total_bytes, 0, total_bytes);
    ret = ai_drv_set_dma_buf_addr(audio_dev_id, aio_dev->dma_phy_addr, aio_dev->dma_vir_addr);
    if (ret != 0) {
        cmpi_mmz_free((td_phys_addr_t)aio_dev->dma_phy_addr, aio_dev->dma_vir_addr);
        ai_err_trace("ai set dma buf addr fail in ai dma buf init \n");
        return ret;
    }

    aio_drv_dev_ctx *ai_drv_dev = ai_drv_get_drv_dev_ctx(audio_dev_id);

    ret = hi19xx_asp_dma_init(ai_drv_dev, total_bytes, audio_dev_id, TD_FALSE);
    if (ret != 0) {
        cmpi_mmz_free((td_phys_addr_t)aio_dev->dma_phy_addr, aio_dev->dma_vir_addr);
        ai_err_trace("dma init fail in ai dma buf init\n");
        return ret;
    }
    ret = hi19xx_asp_dma_open(audio_dev_id, TD_FALSE, aio_dev->aio_attr.chn_cnt);
    if (ret != 0) {
        cmpi_mmz_free((td_phys_addr_t)aio_dev->dma_phy_addr, aio_dev->dma_vir_addr);
        ai_err_trace("dma open fail in ai dma buf init\n");
        return ret;
    }
    hi3xxx_asp_dmac_prepare(audio_dev_id, TD_FALSE);

    return TD_SUCCESS;
}

td_void ai_dma_buf_exit(ot_audio_dev audio_dev_id)
{
    td_s32 ret;

    aio_dev_ctx *aio_dev = &g_ai_dev[audio_dev_id];

    cmpi_mmz_free((td_phys_addr_t)aio_dev->dma_phy_addr, aio_dev->dma_vir_addr);
    aio_dev->dma_phy_addr = 0;
    aio_dev->dma_vir_addr = TD_NULL;

    ret = ai_drv_clr_dma_buf_addr(audio_dev_id);
    if (ret != 0) {
        ai_err_trace("ai clr dma buf addr fail in ai dma buf exit\n");
    }

    return;
}

static td_void get_ai_vb_blk_info(td_u32 *blk_size, td_u32 *blk_cnt, const ot_aio_attr *attr)
{
    /* 分配vb缓存池时需要考虑到为回声抵消分配vb，故而乘2。兼容重采样，按照最大buff分配 */
    td_u32 sample_per_bit = (attr->bit_width == OT_AUDIO_BIT_WIDTH_24) ? 4 : 2; /* 2,4: bytes */
    /* 重采样上采样仅支持到48k，位宽仅支持16bit. */
    if ((attr->sample_rate <= OT_AUDIO_SAMPLE_RATE_48000) && (attr->bit_width == OT_AUDIO_BIT_WIDTH_16)) {
        /* 计算公式: 48->重采样到48k，bit位归一化到16. */
        if ((attr->point_num_per_frame * (td_u32)OT_AUDIO_SAMPLE_RATE_48000 / (td_u32)attr->sample_rate + 1) >
            OT_MAX_AUDIO_POINT_NUM) {
            *blk_size = sample_per_bit * OT_MAX_AUDIO_POINT_NUM;
        } else {
            *blk_size = sample_per_bit * ((attr->point_num_per_frame) *
                (td_u32)OT_AUDIO_SAMPLE_RATE_48000 / (td_u32)attr->sample_rate + 1);
        }
    } else {
        *blk_size = OT_MAX_AUDIO_POINT_BYTES * (attr->point_num_per_frame);
    }
     *blk_size = *blk_size * 2;  // 2: expand block
    /* 考虑AEC通路的buf. */
    *blk_cnt = 2 * (attr->frame_num + 1) * attr->chn_cnt; /* 2: add aec buf */
}

td_s32 ai_vb_init(ot_audio_dev audio_dev_id, const ot_aio_attr *attr)
{
    aio_dev_ctx *aio_dev = &g_ai_dev[audio_dev_id];
    td_u32 blk_size, blk_cnt;
    td_s32 ret;
    td_char *mmz_name = TD_NULL;
    ot_mpp_chn chn;
#ifndef CONFIG_OT_PLATFORM_V8
    vb_info info = { 0 };
#else
    td_char ac_name[AI_VB_POOL_NAME_LEN] = { 0 };
#endif

    get_ai_vb_blk_info(&blk_size, &blk_cnt, attr);

    /* 分配buf大小 */
    chn.mod_id = OT_ID_AI;
    chn.dev_id = audio_dev_id;
    chn.chn_id = 0;
    if ((((td_bool)ckfn_sys_entry()) == TD_FALSE) || (((td_bool)ckfn_sys_get_mmz_name()) == TD_FALSE)) {
        ai_err_trace("sys module may not insert.\n");
        return OT_ERR_AI_NOT_READY;
    }

    if (call_sys_get_mmz_name((hi_mpp_chn *)&chn, (hi_void **)&mmz_name) != TD_SUCCESS) {
        ai_err_trace("get mmz name fail!\n");
        return OT_ERR_AI_NO_MEM;
    }
    ai_info_trace("ai_vb_init entry audio_dev_id = %d\n", audio_dev_id);
#ifndef CONFIG_OT_PLATFORM_V8
    ret = snprintf_s(info.buf_name, sizeof(info.buf_name), sizeof(info.buf_name) - 1, "ai(%d)_frm", audio_dev_id);
#else
    ret = snprintf_s(ac_name, AI_VB_POOL_NAME_LEN, AI_VB_POOL_NAME_LEN - 1, "ai(%d)_frm", audio_dev_id);
#endif
    if (ret < EOK) {
        ai_err_trace("vb pool name snprintf_s err, ret = %d.\n", ret);
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

#ifndef CONFIG_OT_PLATFORM_V8
    info.blk_cnt = blk_cnt;
    info.blk_size = blk_size;
    info.vb_remap_mode = OT_VB_REMAP_MODE_CACHED;
    info.uid = OT_VB_UID_AI;
    ret = call_vb_create_pool(&aio_dev->pool_id, mmz_name, &info);
#else
    ret = call_vb_create_pool(&aio_dev->pool_id, blk_cnt, blk_size, mmz_name, ac_name,
                              VB_REMAP_MODE_NOCACHE, TD_NULL, 0);
#endif
    if (ret != TD_SUCCESS) {
        ai_emerg_trace("init vb pool failed!, blk_size %u, blk_cnt %u, ret 0x%x\n", blk_size, blk_cnt, (td_u32)ret);
        aio_dev->pool_id = (td_u32)OT_VB_INVALID_POOL_ID;
        return OT_ERR_AI_NO_MEM;
    }

    return TD_SUCCESS;
}

td_s32 ai_vb_free(ot_audio_dev audio_dev_id)
{
    td_s32 ret;
    aio_dev_ctx *aio_dev = TD_NULL;

    aio_dev = &g_ai_dev[audio_dev_id];

    if ((td_s32)aio_dev->pool_id == OT_VB_INVALID_POOL_ID) {
        ai_err_trace("vb pool haven't been created!\n");
        return OT_ERR_AI_NO_BUF;
    }

    ret = call_vb_destroy_pool(aio_dev->pool_id);
    if (ret != TD_SUCCESS) {
        ai_err_trace("vb destroy pool failed, ret:%d!\n", ret);
        return OT_ERR_AI_NOT_PERM;
    }

    aio_dev->pool_id = (td_u32)OT_VB_INVALID_POOL_ID;

    return TD_SUCCESS;
}

td_s32 ai_set_dev_attr(ot_audio_dev audio_dev_id, const ot_aio_attr *attr)
{
    ai_check_func_entrance_return(OT_ID_AIO);
    ai_check_dev_return(audio_dev_id);
    ai_check_null_ptr_return(attr);

    td_s32 ret;
    aio_dev_ctx *aio_dev = &g_ai_dev[audio_dev_id];

    if (osal_down_interruptible(&aio_dev->sem)) {
        return -ERESTARTSYS;
    }

    if (aio_dev->inited == TD_FALSE) {
        osal_up(&aio_dev->sem);
        ai_err_trace("audio_dev_id%d not inited\n", audio_dev_id);
        return OT_ERR_AI_NOT_PERM;
    }

    /* 设置设备属性时，设备需要禁用 */
    if (aio_dev->enable != TD_FALSE) {
        osal_up(&aio_dev->sem);
        ai_err_trace("must disable audio_dev_id%d when config dev attr\n", audio_dev_id);
        return OT_ERR_AI_NOT_PERM;
    }

    (void)audio_config_smmu();
    ret = ai_drv_set_dev_attr(audio_dev_id, attr);
    if (ret != TD_SUCCESS) {
        osal_up(&aio_dev->sem);
        return ret;
    }

    ret = memcpy_s(&aio_dev->aio_attr, sizeof(aio_dev->aio_attr), attr, sizeof(*attr));
    if (ret != EOK) {
        ai_err_trace("ai_dev %d attr memcpy_s fail, ret = 0x%x.\n", audio_dev_id, (td_u32)ret);
        osal_up(&aio_dev->sem);
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    aio_dev->cfg = TD_TRUE;

    osal_up(&aio_dev->sem);
    return TD_SUCCESS;
}

td_s32 ai_get_dev_attr(ot_audio_dev audio_dev_id, ot_aio_attr *attr)
{
    ai_check_func_entrance_return(OT_ID_AIO);
    ai_check_dev_return(audio_dev_id);
    ai_check_null_ptr_return(attr);

    aio_dev_ctx *aio_dev = &g_ai_dev[audio_dev_id];
    td_s32 ret;

    if (osal_down_interruptible(&aio_dev->sem)) {
        return -ERESTARTSYS;
    }

    if (aio_dev->inited == TD_FALSE) {
        ai_err_trace("audio_dev_id%d not inited\n", audio_dev_id);
        osal_up(&aio_dev->sem);
        return OT_ERR_AI_NOT_PERM;
    }

    if (aio_dev->cfg != TD_TRUE) {
        ai_err_trace("audio_dev_id%d not configured\n", audio_dev_id);
        osal_up(&aio_dev->sem);
        return OT_ERR_AI_NOT_CFG;
    }

    ret = memcpy_s(attr, sizeof(*attr), &aio_dev->aio_attr, sizeof(aio_dev->aio_attr));
    if (ret != EOK) {
        ai_err_trace("ai_dev %d attr memcpy_s fail, ret = 0x%x.\n", audio_dev_id, (td_u32)ret);
        osal_up(&aio_dev->sem);
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    osal_up(&aio_dev->sem);

    return TD_SUCCESS;
}

static td_void ai_chn_ctx_init(ot_audio_dev ai_dev)
{
    td_u32 i, chn;
    td_u8 *addr = TD_NULL;
    const size_t offset = 2 * sizeof(osal_wait_t) + sizeof(struct osal_semaphore); /* 2: two osal_wait_t */

    /* init chn info of this dev */
    for (i = 0; i < OT_AI_MAX_CHN_NUM; i++) {
        chn = (td_u32)ai_get_chn_id((td_u32)ai_dev, i);
        addr = (td_u8 *)&g_ai_chn[chn];

        if (memset_s(addr + offset, sizeof(ai_chn_ctx) - offset, 0, sizeof(ai_chn_ctx) - offset) != EOK) {
            ai_warn_trace("ai dev %d chn %u memset_s failed.\n", ai_dev, i);
        }

        g_ai_chn[chn].chn_attr.mode = OT_AI_CHN_MODE_NORMAL;

        g_ai_chn[chn].aec_ao_dev = -1;
        g_ai_chn[chn].aec_ao_chn = -1;
        g_ai_chn[chn].chn_state = AIO_CHN_ORIGINAL;
        g_ai_chn[chn].dev_enable = TD_TRUE;
        g_ai_chn[chn].ai_vqe_dbg.vqe_enable = TD_FALSE;
        g_ai_chn[chn].ai_vqe_dbg.ai_vqe_cfg.anr_open = (td_s32)TD_FALSE;
        g_ai_chn[chn].ai_vqe_dbg.ai_vqe_cfg.aec_open = (td_s32)TD_FALSE;
        g_ai_chn[chn].ai_vqe_dbg.ai_vqe_cfg.agc_open = (td_s32)TD_FALSE;
        g_ai_chn[chn].ai_vqe_dbg.ai_vqe_cfg.rnr_open = (td_s32)TD_FALSE;
        g_ai_chn[chn].ai_vqe_dbg.ai_vqe_cfg.hpf_open = (td_s32)TD_FALSE;
        g_ai_chn[chn].ai_vqe_dbg.ai_vqe_cfg.eq_open = (td_s32)TD_FALSE;

        g_ai_chn[chn].ai_vqe_dbg.ai_vqe_cfg.work_sample_rate = 0;
        g_ai_chn[chn].ai_vqe_dbg.ai_vqe_cfg.work_state = OT_VQE_WORK_STATE_COMMON;
        g_ai_chn[chn].ai_vqe_dbg.ai_vqe_cfg.work_sample_rate = (td_s32)OT_AUDIO_SAMPLE_RATE_BUTT;

        g_ai_chn[chn].ai_vqe_dbg.ai_vqe_cfg.aec_cfg.usr_mode = 0;
        g_ai_chn[chn].ai_vqe_dbg.ai_vqe_cfg.aec_cfg.cozy_noisy_mode = AUDIO_AEC_MODE_BUTT;

        g_ai_chn[chn].ai_vqe_dbg.ai_vqe_cfg.agc_cfg.usr_mode = 0;
        g_ai_chn[chn].ai_vqe_dbg.ai_vqe_cfg.agc_cfg.noise_suppress_switch = 0;
        g_ai_chn[chn].ai_vqe_dbg.ai_vqe_cfg.agc_cfg.adjust_speed = 0;
        g_ai_chn[chn].ai_vqe_dbg.ai_vqe_cfg.agc_cfg.improve_snr = 0;
        g_ai_chn[chn].ai_vqe_dbg.ai_vqe_cfg.agc_cfg.max_gain = 0;
        g_ai_chn[chn].ai_vqe_dbg.ai_vqe_cfg.agc_cfg.noise_floor = -50; /* -50: floor */
        g_ai_chn[chn].ai_vqe_dbg.ai_vqe_cfg.agc_cfg.output_mode = 0;
        g_ai_chn[chn].ai_vqe_dbg.ai_vqe_cfg.agc_cfg.target_level = -40; /* -40: level */
        g_ai_chn[chn].ai_vqe_dbg.ai_vqe_cfg.agc_cfg.use_hpf = 0;

        g_ai_chn[chn].ai_vqe_dbg.ai_vqe_cfg.anr_cfg.usr_mode = 0;
        g_ai_chn[chn].ai_vqe_dbg.ai_vqe_cfg.anr_cfg.nr_intensity = 0;
        g_ai_chn[chn].ai_vqe_dbg.ai_vqe_cfg.agc_cfg.improve_snr = 0;

        g_ai_chn[chn].ai_vqe_vol = 0;

        g_ai_chn[chn].resmp_dbg.resmp_enable = TD_FALSE;
        g_ai_chn[chn].resmp_dbg.resmp_attr.in_point_num = 0;
        g_ai_chn[chn].resmp_dbg.resmp_attr.in_sample_rate = OT_AUDIO_SAMPLE_RATE_BUTT;
        g_ai_chn[chn].resmp_dbg.resmp_attr.out_sample_rate = OT_AUDIO_SAMPLE_RATE_BUTT;
        g_ai_chn[chn].save_file_info.cfg = TD_FALSE;
        g_ai_chn[chn].save_file_info.file_path[0] = '\0';
        g_ai_chn[chn].save_file_info.file_name[0] = '\0';
        g_ai_chn[chn].save_file_info.file_size = 0;
    }
}

static td_void ai_chn_ctx_exit(ot_audio_dev ai_dev, const ot_aio_attr *ai_attr)
{
    td_u32 i, chn;

    /* 重置通道上下文 */
    for (i = 0; i < ai_attr->chn_cnt; i++) {
        chn = (td_u32)ai_get_chn_id((td_u32)ai_dev, i);
        g_ai_chn[chn].dev_enable = TD_FALSE;
        g_ai_chn[chn].chn_state = AIO_CHN_ORIGINAL;
        g_ai_chn[chn].aec_ao_dev = -1;
        g_ai_chn[chn].int_lost = 0;
        g_ai_chn[chn].aec_fail = 0;
        g_ai_chn[chn].ai_usr_que_lost = 0;
    }
}

static td_s32 ai_chn_fb_init(ot_audio_dev ai_dev, const ot_aio_attr *ai_attr)
{
    td_s32 ret;
    td_u32 i, k, chn;

    /* 分配USER队列节点buf */
    for (i = 0; i < ai_attr->chn_cnt; i++) {
        chn = ai_get_chn_id((td_u32)ai_dev, i);
        ret = audio_fb_init(OT_MAX_AI_USRFRM_DEPTH, &g_ai_chn[chn].user_frm_buf);
        if (ret != TD_SUCCESS) {
            for (k = 0; k < i; k++) {
                chn = ai_get_chn_id((td_u32)ai_dev, k);
                (td_void)audio_fb_release(&g_ai_chn[chn].user_frm_buf);
            }
            return OT_ERR_AI_NO_MEM;
        }
    }

    /* 分配AIE队列节点buf */
    for (i = 0; i < ai_attr->chn_cnt; i++) {
        chn = ai_get_chn_id((td_u32)ai_dev, i);
        ret = audio_fb_init(OT_MAX_AI_USRFRM_DEPTH, &g_ai_chn[chn].aie_frm_buf);
        if (ret != TD_SUCCESS) {
            for (k = 0; k < i; k++) {
                chn = ai_get_chn_id((td_u32)ai_dev, k);
                (td_void)audio_fb_release(&g_ai_chn[chn].aie_frm_buf);
            }

            for (i = 0; i < ai_attr->chn_cnt; i++) {
                chn = ai_get_chn_id((td_u32)ai_dev, i);
                (td_void)audio_fb_release(&g_ai_chn[chn].user_frm_buf);
            }

            return OT_ERR_AI_NO_MEM;
        }
    }

    return TD_SUCCESS;
}

static td_void ai_chn_fb_put_free(ot_audio_dev ai_dev, const ot_aio_attr *ai_attr)
{
    td_s32 ret;
    td_u32 busy_num, i, j, chn;
    audio_frame_buf *buf = TD_NULL;
    audio_buf_blk *node = TD_NULL;

    /* 关闭DMA后，就没有中断了，可以直接释放 */
    for (i = 0; i < ASP_SUPPORT_MAX_CHN_CNT; i++) {
        chn = ai_get_chn_id((td_u32)ai_dev, i);

        osal_wakeup(&g_ai_chn[chn].user_wait);

        /* USER队列 */
        buf = &g_ai_chn[chn].user_frm_buf;
        busy_num = audio_fb_query_busy_num(buf);
        for (j = 0; j < busy_num; j++) {
            node = audio_fb_get_busy(buf);
            /* ai_sub_vb出错无须处理 */
            ret = ai_sub_vb(&node->audio_frm, OT_VB_UID_AI);
            if (ret != TD_SUCCESS) {
                ai_info_trace("ai_sub_vb failed with ret = 0x%x.\n", (td_u32)ret);
            }
            ret = audio_fb_put_free(buf, node);
            if (ret != TD_SUCCESS) {
                ai_info_trace("audio fb put user buf free fail ret = %d\n", ret);
            }
        }

        osal_wakeup(&g_ai_chn[chn].aie_wait);

        /* AIE队列 */
        buf = &g_ai_chn[chn].aie_frm_buf;
        busy_num = audio_fb_query_busy_num(buf);
        for (j = 0; j < busy_num; j++) {
            node = audio_fb_get_busy(buf);
            /* ai_sub_vb出错无须处理 */
            ret = ai_sub_vb(&node->audio_frm, OT_VB_UID_AI);
            if (ret != TD_SUCCESS) {
                ai_info_trace("ai_sub_vb failed with ret = 0x%x.\n", (td_u32)ret);
            }
            ret = audio_fb_put_free(buf, node);
            if (ret != TD_SUCCESS) {
                ai_info_trace("audio fb put aie buf free fail ret = %d\n", ret);
            }
        }
    }
}

static td_void ai_chn_fb_exit(ot_audio_dev ai_dev, const ot_aio_attr *ai_attr)
{
    td_s32 ret;
    td_u32 i, chn;

    for (i = 0; i < ai_attr->chn_cnt; i++) {
        chn = ai_get_chn_id((td_u32)ai_dev, i);
        ret = audio_fb_release(&g_ai_chn[chn].user_frm_buf);
        if (ret != TD_SUCCESS) {
            ai_info_trace("audio user buf fb release failed in ai chn fb exit\n");
        }
        ret = audio_fb_release(&g_ai_chn[chn].aie_frm_buf);
        if (ret != TD_SUCCESS) {
            ai_info_trace("audio aie buf fb release failed in ai chn fb exit\n");
        }
    }
}

static td_void ai_chn_fb_set_null(ot_audio_dev ai_dev, const ot_aio_attr *ai_attr)
{
    td_u32 i, chn;

    for (i = 0; i < ai_attr->chn_cnt; i++) {
        chn = (td_u32)ai_get_chn_id((td_u32)ai_dev, i);
        /* 备份虚拟地址用于后续真正的release，此处仅对原来的虚拟地址置空 */
        g_ai_chn[chn].user_buf_virt_addr = g_ai_chn[chn].user_frm_buf.buf_vir_addr;
        g_ai_chn[chn].user_frm_buf.buf_vir_addr = TD_NULL;

        g_ai_chn[chn].aie_buf_virt_addr = g_ai_chn[chn].aie_frm_buf.buf_vir_addr;
        g_ai_chn[chn].aie_frm_buf.buf_vir_addr = TD_NULL;
    }
}

static td_void ai_chn_fb_free(ot_audio_dev ai_dev, const ot_aio_attr *ai_attr)
{
    td_u32 i, chn;

    for (i = 0; i < ai_attr->chn_cnt; i++) {
        chn = ai_get_chn_id((td_u32)ai_dev, i);
        /* 对备份的虚拟地址进行真正的free操作 */
        if (g_ai_chn[chn].user_buf_virt_addr != TD_NULL) {
            osal_vfree(g_ai_chn[chn].user_buf_virt_addr);
            g_ai_chn[chn].user_buf_virt_addr = TD_NULL;
        }

        if (g_ai_chn[chn].aie_buf_virt_addr != TD_NULL) {
            osal_vfree(g_ai_chn[chn].aie_buf_virt_addr);
            g_ai_chn[chn].aie_buf_virt_addr = TD_NULL;
        }
    }
}

static td_s32 enable_dev_inner(ot_audio_dev audio_dev_id, const ot_aio_attr *attr)
{
    td_s32 ret;

    /* 分配队列节点buf */
    ret = ai_chn_fb_init(audio_dev_id, attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    /* 分配并初始化DMA buf */
    ret = ai_dma_buf_init(audio_dev_id);
    if (ret != TD_SUCCESS) {
        ai_chn_fb_exit(audio_dev_id, attr);
        return ret;
    }

    /* 分配并初始化vb缓存池 */
    ret = ai_vb_init(audio_dev_id, attr);
    if (ret != TD_SUCCESS) {
        ai_dma_buf_exit(audio_dev_id);
        ai_chn_fb_exit(audio_dev_id, attr);
        return ret;
    }

    ret = ai_drv_enable_dev(audio_dev_id);
    if (ret != TD_SUCCESS) {
        (td_void)ai_vb_free(audio_dev_id);
        ai_dma_buf_exit(audio_dev_id);
        ai_chn_fb_exit(audio_dev_id, attr);
        return ret;
    }

    return TD_SUCCESS;
}

td_s32 ai_enable_dev(ot_audio_dev audio_dev_id)
{
    td_s32 ret;
    aio_dev_ctx *aio_dev = TD_NULL;
    ot_aio_attr *attr = TD_NULL;
    unsigned long lock_flag;

    ai_check_func_entrance_return(OT_ID_AIO);
    ai_check_dev_return(audio_dev_id);

    aio_dev = &g_ai_dev[audio_dev_id];

    if (osal_down_interruptible(&aio_dev->sem)) {
        return -ERESTARTSYS;
    }

    if (aio_dev->inited == TD_FALSE) {
        ai_err_trace("aidev%d is not initialized\n", audio_dev_id);
        osal_up(&aio_dev->sem);
        return OT_ERR_AI_NOT_PERM;
    }

    if (aio_dev->enable == TD_TRUE) {
        osal_up(&aio_dev->sem);
        return TD_SUCCESS;
    }

    if (aio_dev->cfg != TD_TRUE) {
        ai_err_trace("aidev%d is not configured\n", audio_dev_id);
        osal_up(&aio_dev->sem);
        return OT_ERR_AI_NOT_CFG;
    }

    attr = &(aio_dev->aio_attr);

    /* 初始化通道上下文 */
    ai_chn_ctx_init(audio_dev_id);

    ret = enable_dev_inner(audio_dev_id, attr);
    if (ret != TD_SUCCESS) {
        osal_up(&aio_dev->sem);
        return ret;
    }

    aio_spin_lock_irqsave(&aio_dev->spinlock, lock_flag);
    aio_dev->enable = TD_TRUE;
    aio_spin_unlock_irqrestore(&aio_dev->spinlock, lock_flag);

    ai_info_trace("AI dev %d enable ok!\n", audio_dev_id);

    osal_up(&aio_dev->sem);
    return TD_SUCCESS;
}

static td_s32 ai_disable_dev_check(ot_audio_dev ai_dev, const aio_dev_ctx *aio_dev, td_bool force_exit)
{
    td_s32 ret;
    td_u32 i, chn, use_cnt;

    /* 需要首先禁用所有通道 */
    for (i = 0; i < aio_dev->aio_attr.chn_cnt; i++) {
        chn = (td_u32)ai_get_chn_id((td_u32)ai_dev, i);
        if ((g_ai_chn[chn].chn_state == AIO_CHN_ENABLE) || (g_ai_chn[chn].chn_state == AIO_CHN_PAUSE)) {
            ai_err_trace("ai_dev %d ai_chn %u not disabled, must disable all chn before disable AI device\n",
                         ai_dev, i);
            return OT_ERR_AI_NOT_PERM;
        }
    }

    /* 检查是否有模块在使用AI的VB */
    for (i = 0; i < OT_VB_MAX_USER; i++) {
        use_cnt = 0;

        ret = call_vb_inquire_pool_user_cnt(aio_dev->pool_id, i, &use_cnt);
        if (ret != TD_SUCCESS) {
            continue;
        }

        if ((use_cnt != 0) && (i != OT_VB_UID_AI)) {
            ai_warn_trace("someone(%u) occupies aidev%d's vb %u, force_exit = %u!\n",
                i, ai_dev, use_cnt, (td_u32)force_exit);
            if (force_exit == TD_FALSE) {
                return OT_ERR_AI_BUSY;
            }
        }
    }

    return TD_SUCCESS;
}

td_s32 ai_disable_dev(ot_audio_dev audio_dev_id, td_bool force_exit)
{
    td_s32 ret;
    aio_dev_ctx *aio_dev = TD_NULL;
    unsigned long lock_flag;

    ai_check_func_entrance_return(OT_ID_AIO);
    ai_check_dev_return(audio_dev_id);

    aio_dev = &g_ai_dev[audio_dev_id];

    if (osal_down_interruptible(&aio_dev->sem)) {
        return -ERESTARTSYS;
    }

    /* 设备没有使能或配置时调此接口，不视为失败 */
    if ((aio_dev->inited == TD_FALSE) || (aio_dev->enable == TD_FALSE)) {
        osal_up(&aio_dev->sem);
        return TD_SUCCESS;
    }

    /* 检查通道是否全部禁用以及VB是否在占用 */
    ret = ai_disable_dev_check(audio_dev_id, aio_dev, force_exit);
    if (ret != TD_SUCCESS) {
        osal_up(&aio_dev->sem);
        return ret;
    }

    ai_drv_disable_dev(audio_dev_id);

    /* iounmap函数会休眠，故不能加锁 */
    ai_dma_buf_exit(audio_dev_id);

    aio_spin_lock_irqsave(&aio_dev->spinlock, lock_flag);

    /* 释放busy fb */
    ai_chn_fb_put_free(audio_dev_id, &(aio_dev->aio_attr));

    /* 关闭DMA后，就没有中断了，可以直接释放。vfee会导致睡眠，因此这里只置空，在锁外面free */
    ai_chn_fb_set_null(audio_dev_id, &(aio_dev->aio_attr));

    aio_dev->enable = TD_FALSE;
    aio_spin_unlock_irqrestore(&aio_dev->spinlock, lock_flag);

    /* audio_fb_release内部用到了vfree，会导致睡眠，不能放在锁里面，因此单独free */
    ai_chn_fb_free(audio_dev_id, &(aio_dev->aio_attr));

    /* 销毁VB缓存池，前面已检查过是否有模块在使用，所以这里不检查返回值 */
    ret = ai_vb_free(audio_dev_id);
    if (ret != TD_SUCCESS) {
        ai_warn_trace("AI dev %d free vb fail\n", audio_dev_id);
    }

    /* 重置通道上下文 */
    ai_chn_ctx_exit(audio_dev_id, &(aio_dev->aio_attr));

    /* clear dev pub attr:when enable dev */
    if (aio_dev->cfg && (aio_dev->enable == TD_FALSE)) {
        /* reggie  清属性 */
        ret = ai_drv_clr_dev_attr(audio_dev_id);
        if (ret != TD_SUCCESS) {
            osal_up(&aio_dev->sem);
            return ret;
        }
    }

    aio_dev->cfg = TD_FALSE;  // disable之后，下一次enable需要重新配置属性
    osal_up(&aio_dev->sem);

    ai_info_trace("AI dev %d disable ok!\n", audio_dev_id);
    return TD_SUCCESS;
}

td_void ai_dev_sem_init(ot_audio_dev audio_dev_id)
{
    aio_dev_ctx *aio_dev = &g_ai_dev[audio_dev_id];
    td_s32 i, chn_id;
    (td_void)memset_s(aio_dev, sizeof(aio_dev_ctx), 0, sizeof(aio_dev_ctx));
    for (i = 0; i < OT_AI_MAX_CHN_NUM; i++) {
        chn_id = ai_get_chn_id(audio_dev_id, i);
        (td_void)memset_s(&g_ai_chn[chn_id], sizeof(ai_chn_ctx), 0, sizeof(ai_chn_ctx));
        (td_void)osal_sema_init(&g_ai_chn[chn_id].sem, 1);

        (td_void)osal_wait_init(&g_ai_chn[chn_id].user_wait);
        (td_void)osal_wait_init(&g_ai_chn[chn_id].aie_wait);
    }

    (td_void)osal_sema_init(&aio_dev->sem, 1);

    (td_void)aio_spin_lock_init(&aio_dev->spinlock);
}

td_s32 ai_dev_init(ot_audio_dev audio_dev_id)
{
    aio_dev_ctx *aio_dev = &g_ai_dev[audio_dev_id];
    td_s32 i, chn_id;
    td_s32 ret;

    if ((aio_dev->inited == TD_TRUE) || (aio_dev->enable == TD_TRUE)) {
        return TD_SUCCESS;
    }

    aio_dev->enable = TD_FALSE;

    ret = ai_drv_dev_init(audio_dev_id);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ai_drv_set_dma_isr(ai_dma_callback);

    aio_dev->inited = TD_TRUE;
    aio_dev->cfg = TD_FALSE;

    return TD_SUCCESS;
}

td_void ai_dev_sem_exit(ot_audio_dev audio_dev_id)
{
    td_u32 chn, i;
    aio_dev_ctx *aio_dev = &g_ai_dev[audio_dev_id];
    for (i = 0; i < OT_AI_MAX_CHN_NUM; i++) {
        chn = ai_get_chn_id((td_u32)audio_dev_id, i);
        osal_wait_destroy(&g_ai_chn[chn].user_wait);
        osal_wait_destroy(&g_ai_chn[chn].aie_wait);
        osal_sema_destroy(&g_ai_chn[chn].sem);
    }

    aio_spin_lock_destroy(&aio_dev->spinlock);
    osal_sema_destroy(&aio_dev->sem);
}

td_s32 ai_dev_exit(ot_audio_dev audio_dev_id)
{
    td_s32 ret;
    td_u32 chn, i;
    aio_dev_ctx *aio_dev = &g_ai_dev[audio_dev_id];
    td_bool force_exit;

    if (aio_dev->inited == TD_FALSE) {
        return TD_SUCCESS;
    }

    ret = ai_drv_dev_deinit(audio_dev_id);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    /* disable all AI channel */
    for (i = 0; i < aio_dev->aio_attr.chn_cnt; i++) {
        chn = ai_get_chn_id((td_u32)audio_dev_id, i);
        if (g_ai_chn[chn].chn_state != AIO_CHN_ORIGINAL) {
            g_ai_chn[chn].aec_ao_dev = -1;
            g_ai_chn[chn].chn_state = AIO_CHN_DISABLE;

            /* 系统退出时 唤醒可能一直阻塞的等待队列
            (否则在某些异常情况下，再次运行程序时，由于重新初始化等待队列导致正阻塞中的队列挂死) */
            osal_wakeup(&g_ai_chn[chn].user_wait);
            osal_wakeup(&g_ai_chn[chn].aie_wait);
        }
    }

    force_exit = TD_TRUE;
    ret = ai_disable_dev(audio_dev_id, force_exit);
    if (ret != TD_SUCCESS) {
        ai_err_trace("AI dev %d exit failed! err 0x%x! \n", audio_dev_id, (td_u32)ret);
        return ret;
    }

    aio_dev->inited = TD_FALSE;

    return TD_SUCCESS;
}

/* register DMA ISR */
td_void ai_dma_callback(ot_audio_dev audio_dev_id)
{
    unsigned long lock_flag;
    td_s32 ret;

    aio_spin_lock_irqsave(&g_ai_dev[audio_dev_id].spinlock, lock_flag);

    /* if device had stopped,return */
    if (g_ai_dev[audio_dev_id].enable == TD_FALSE) {
        aio_spin_unlock_irqrestore(&g_ai_dev[audio_dev_id].spinlock, lock_flag);
        return;
    }

    ret = ai_get_chn_data(audio_dev_id);
    if (ret != TD_SUCCESS) {
        ai_warn_trace("AI dev %d get chn data fail\n", audio_dev_id);
    }

    ret = ai_send_chn_data(audio_dev_id);
    if (ret != TD_SUCCESS) {
        ai_warn_trace("AI dev %d send chn data fail\n", audio_dev_id);
    }

    aio_spin_unlock_irqrestore(&g_ai_dev[audio_dev_id].spinlock, lock_flag);

    return;
}

ai_chn_ctx *ai_get_chn_ctx(td_s32 chn_id)
{
    return &g_ai_chn[chn_id];
}

aio_dev_ctx *ai_get_dev_ctx(ot_audio_dev audio_dev_id)
{
    return &g_ai_dev[audio_dev_id];
}

td_s32 ai_clr_dev_attr(ot_audio_dev audio_dev_id)
{
    td_s32 ret;
    aio_dev_ctx *aio_dev = TD_NULL;

    ai_check_func_entrance_return(OT_ID_AIO);
    ai_check_dev_return(audio_dev_id);

    aio_dev = &g_ai_dev[audio_dev_id];

    if (osal_down_interruptible(&aio_dev->sem)) {
        return -ERESTARTSYS;
    }

    /* 设备没有禁止时不能清设备属性 */
    if (aio_dev->enable == TD_TRUE) {
        ai_err_trace("AI dev %d is busy, disable it first\n", audio_dev_id);
        osal_up(&aio_dev->sem);
        return OT_ERR_AI_NOT_PERM;
    }

    ret = ai_drv_clr_dev_attr(audio_dev_id);
    if (ret != TD_SUCCESS) {
        osal_up(&aio_dev->sem);
        return ret;
    }

    aio_dev->cfg = TD_FALSE;
    osal_up(&aio_dev->sem);

    return ret;
}
