/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2009-2020. All rights reserved.
 * Description: ai trans driver
 * Author: Hisilicon multimedia software group
 * Create: 2009/5/5
 */

#include "ao_trans.h"

#include "hi_osal.h"
#include "securec.h"

#include "hi_comm_audio.h"
#ifdef CONFIG_OT_PLATFORM_V8
#include "ot_inner_common_aio.h"
#else
#include "ot_comm_aio_adapt.h"
#endif
#include "ao_drv.h"
#include "proc_ext.h"
#include "mkp_aio.h"
#include "mod_ext.h"
#include "aiao_ext.h"
#include "sys_ext.h"
#include "mm_ext.h"
#include "aiao_hal_comm.h"
#include "asp_dma_hal.h"
#include "pid_protect.h"

#define AO_MMZ_NAME_LEN 16
#define AO_MMZ_FILE_INIT 100
#define AO_MMZ_BYTES 1024

aio_dev_ctx g_ao_dev[OT_AO_DEV_MAX_NUM];
ao_chn_ctx g_ao_chn[OT_AO_MAX_CHN_NUM * OT_AO_DEV_MAX_NUM];

extern td_void ao_dma_callback(ot_audio_dev audio_dev_id);

#ifdef CONFIG_OT_AUDIO_ALSA_SUPPORT
td_s32 ao_set_alsa_dma_attr(ot_audio_dev ao_dev, cir_buf cir_buf_attr, td_u32 trans_len)
{
    td_s32 ret;
    ret = ao_drv_set_dma_attr(ao_dev, cir_buf_attr, trans_len);
    if (ret != TD_SUCCESS) {
        ao_warn_trace("ao_drv_set_dma_attr fail with ret = 0x%x.\n", ret);
    }
    return ret;
}
#endif

static td_void ao_chn_buf_init(ot_audio_dev ao_dev, const ot_aio_attr *ao_attr, const td_u8 *buf_vir_addr)
{
    td_u32 chn, i;
    td_u32 chn_buf_bytes;
    const td_u8 *chn_buf_vir_addr = TD_NULL;
    td_u32 chn_cnt = ASP_SUPPORT_MAX_CHN_CNT;
    td_u32 sample_per_bit = (ao_attr->bit_width == OT_AUDIO_BIT_WIDTH_24) ? 4 : 2; /* 2,4: bytes */

    chn_buf_bytes = sample_per_bit * (ao_attr->point_num_per_frame) * (ao_attr->frame_num);

    /* 初始化各个实体通道的chn buf */
    for (i = 0; i < chn_cnt; i++) {
        chn_buf_vir_addr = buf_vir_addr + chn_buf_bytes * i;
        chn = (td_u32)ao_get_chn_id((td_u32)ao_dev, i);
        buf_init(&g_ao_chn[chn].buf, (td_u32 *)chn_buf_vir_addr,
            ao_attr->frame_num, ao_attr->point_num_per_frame * sample_per_bit);
    }

#ifndef CONFIG_AUDIO_V200_SUPPORT
    /* 初始化系统音通道的chn buf */
    chn_buf_vir_addr = buf_vir_addr + chn_buf_bytes * chn_cnt;
    chn = (td_u32)ao_get_chn_id((td_u32)ao_dev, OT_AO_SYS_CHN_ID);
    buf_init(&g_ao_chn[chn].buf, (td_u32 *)chn_buf_vir_addr,
        ao_attr->frame_num, ao_attr->point_num_per_frame * sample_per_bit);
#endif
}

static td_void ao_buf_get_total_bytes(const ot_aio_attr *attr, td_u32 dma_buf_bytes, td_u32 *total_bytes)
{
    td_u32 chn_buf_bytes;
    td_u32 chn_cnt = 2; /* 2: all mode is 2 channel */
    td_u32 sample_per_bit = 4; /* 4: bytes */

    chn_buf_bytes = sample_per_bit * (attr->point_num_per_frame) * (attr->frame_num);

    if (attr->snd_mode == OT_AUDIO_SOUND_MODE_STEREO) {
        *total_bytes = dma_buf_bytes + chn_buf_bytes * (chn_cnt + 2); /* 2: chn num */
    } else {
        *total_bytes = dma_buf_bytes + chn_buf_bytes * (chn_cnt + 1);
    }

#ifdef CONFIG_AUDIO_V200_SUPPORT
    if (*total_bytes % 128 != 0) { /* 128: 128字节对齐 */
        *total_bytes = (*total_bytes / 128 + 1) * 128; /* 128: 128字节对齐 */
    }
#endif
    ao_info_trace("ao_buf_get_total_bytes:total_bytes=0x%x, dma_buf_bytes=0x%x", *total_bytes, dma_buf_bytes);
}

td_s32 ao_buf_init(ot_audio_dev audio_dev_id, const ot_aio_attr *attr)
{
    td_s32 ret;
    /* DMA buf是ping-pong buf, 每个通道均有一个通道buf, DMA buf和所有通道的chn_buf是作为一个整体分配内存的 */
    td_u32 dma_buf_bytes, total_bytes;
    char ac_name[AO_MMZ_NAME_LEN] = {0};
    td_u8 *all_chn_vir_addr = TD_NULL; /* 所有通道的buf的虚拟地址 */
    td_char *mmz_name = TD_NULL;
    ot_mpp_chn mpp_chn = {0};
    aio_dev_ctx *aio_dev = &g_ao_dev[audio_dev_id];

    ret = ao_drv_dma_buf_info_init(audio_dev_id, &dma_buf_bytes);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ao_buf_get_total_bytes(attr, dma_buf_bytes, &total_bytes);

    mpp_chn.mod_id = OT_ID_AO;
    mpp_chn.dev_id = audio_dev_id;
    mpp_chn.chn_id = 0;

    if ((((td_bool)ckfn_sys_entry()) == TD_FALSE) || (((td_bool)ckfn_sys_get_mmz_name()) == TD_FALSE)) {
        ao_err_trace("sys module may not insert.\n");
        return OT_ERR_AO_NOT_READY;
    }

    if (call_sys_get_mmz_name((hi_mpp_chn *)&mpp_chn, (hi_void**)&mmz_name) != TD_SUCCESS) {
        ao_err_trace("get mmz name fail!\n");
        return OT_ERR_AO_NO_MEM;
    }

    /* 分配整块mmz，供DMA Buf和通道Buf用 */
    ret = snprintf_s(ac_name, AO_MMZ_NAME_LEN, AO_MMZ_NAME_LEN - 1, "ao(%d)_dma&frm", audio_dev_id);
    if (ret < EOK) {
        ao_err_trace("dma buf name snprintf_s err, ret = %d \n", ret);
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

#ifndef CONFIG_OT_PLATFORM_V8
    ret = cmpi_mmz_malloc_nocache(mmz_name, ac_name, (td_phys_addr_t *)&aio_dev->dma_phy_addr,
        (td_void *)&aio_dev->dma_vir_addr, total_bytes);
#else
    ret = cmpi_mmz_malloc_nocache(mmz_name, ac_name, &aio_dev->dma_phy_addr,
        (td_void *)&aio_dev->dma_vir_addr, ((td_ulong)total_bytes + (td_ulong)AO_MMZ_BYTES));
#endif
    if (ret != TD_SUCCESS) {
        ao_emerg_trace("alloc or ioremap dma buf and all channel buf err\n");
        return OT_ERR_AO_NO_MEM;
    }

    /* 将整块buf清0 */
    (td_void)memset_s(aio_dev->dma_vir_addr, total_bytes, 0, total_bytes);

    /* 计算DMA buf的物理地址和虚拟地址，第一块DMA buf的虚拟地址即整块buf的起始虚拟地址 */
    ret = ao_drv_set_dma_buf_addr(audio_dev_id, aio_dev->dma_phy_addr, aio_dev->dma_vir_addr);
    if (ret != 0) {
        cmpi_mmz_free((td_phys_addr_t)aio_dev->dma_phy_addr, aio_dev->dma_vir_addr);
        ao_err_trace("ao set dma buf addr fail in ao dma buf init \n");
        return ret;
    }

    /* 映射所有通道的buf */
    all_chn_vir_addr = aio_dev->dma_vir_addr + dma_buf_bytes;

    /* 初始化各个通道的chn buf */
    ao_chn_buf_init(audio_dev_id, attr, all_chn_vir_addr);

    aio_drv_dev_ctx *ao_drv_dev = ao_drv_get_drv_dev_ctx(audio_dev_id);

    ret = hi19xx_asp_dma_init(ao_drv_dev, total_bytes, audio_dev_id, TD_TRUE);
    if (ret != 0) {
        cmpi_mmz_free((td_phys_addr_t)aio_dev->dma_phy_addr, aio_dev->dma_vir_addr);
        ao_err_trace("dma init fail in ao dma buf init\n");
        return ret;
    }

    ret = hi19xx_asp_dma_open(audio_dev_id, TD_TRUE, attr->chn_cnt);
    if (ret != 0) {
        cmpi_mmz_free((td_phys_addr_t)aio_dev->dma_phy_addr, aio_dev->dma_vir_addr);
        ao_err_trace("asp dma open fail in ao dma buf init\n");
        return ret;
    }
    hi3xxx_asp_dmac_prepare(audio_dev_id, TD_TRUE);
    return TD_SUCCESS;
}

td_void ao_buf_exit(ot_audio_dev audio_dev_id)
{
    td_s32 ret;

    aio_dev_ctx *aio_dev = &g_ao_dev[audio_dev_id];

    cmpi_mmz_free((td_phys_addr_t)aio_dev->dma_phy_addr, aio_dev->dma_vir_addr);
    aio_dev->dma_phy_addr = 0;
    aio_dev->dma_vir_addr = TD_NULL;

    ret = ao_drv_clr_dma_buf_addr(audio_dev_id);
    if (ret != 0)
        ao_warn_trace("ao drv clr dma buf addr fail\n");

    return;
}

static __inline td_s16 clip_to_short(td_s32 x)
{
    td_s32 sign;

    /* clip to [-32768, 32767] */
    sign = x >> 31; /* 31: offset */
    if (sign != (x >> 15)) { /* 15: offset */
        x = sign ^ ((1 << 15) - 1); /* 15: offset */
    }

    return (td_s16)x;
}

static td_void util_mixer_pro(td_void* mix_buf, const td_void* in_buf, td_u32 samples)
{
    td_u32 i;
    td_s16 mix, data;
    const td_s16 *in = TD_NULL;
    td_s16 *out = TD_NULL;

    /* optimize, 性能优化 */
    if (!samples) {
        return;
    }

    in  = (td_s16 *)in_buf;
    out = (td_s16 *)mix_buf;

    for (i = 0; i < samples; i++) {
        data = *in;
        mix  = *out;
        *out = clip_to_short((td_s32)mix + data);
        out++;
        in++;
    }
}

static td_void ao_put_chn_dma_data(ot_audio_dev ao_dev, ot_ao_chn ao_chn, const audio_pack *sys_pack)
{
    td_s32 chn = ao_get_chn_id(ao_dev, ao_chn);
    aio_dev_ctx *aio_dev = &g_ao_dev[ao_dev];
    if (g_ao_chn[chn].chn_state == AIO_CHN_ENABLE) {
        if (!buf_is_empty(&g_ao_chn[chn].buf)) {
            audio_pack *pack = &(buf_get_read(&g_ao_chn[chn].buf));

            /* mix sys. */
            if (sys_pack != TD_NULL) {
                /* util_mixer_pro第三个参数为sample(单位:2Byte)，pack_len为单个音频frame的长度(单位:Byte) */
                util_mixer_pro(pack->data, sys_pack->data, g_ao_chn[chn].buf.pack_len >> 1);
            }

            /* put channel buffer data to DMA buffer */
            ao_drv_put_chn_data(ao_dev, pack, ao_chn);

            buf_finish_read(&g_ao_chn[chn].buf);

            g_ao_chn[chn].int_lost_time = 0;
            osal_wakeup(&aio_dev->wait);
        } else {
            /* put last frame in this channel of DMA buffer */
            g_ao_chn[chn].int_lost_time++;
#ifndef CONFIG_AUDIO_V200_SUPPORT
            if (sys_pack != TD_NULL) {
                ao_drv_put_chn_data(ao_dev, sys_pack, ao_chn);
            } else {
                ao_drv_put_last_data(ao_dev, ao_chn);
            }

            ao_warn_trace("AO buf empty, put last data, serial times:%d, chn=%d\n",
                g_ao_chn[chn].int_lost_time, ao_chn);
#endif
            g_ao_chn[chn].int_lost++;
        }
    } else if (g_ao_chn[chn].chn_state == AIO_CHN_PAUSE) {
        if (sys_pack != TD_NULL) {
            ao_drv_put_chn_data(ao_dev, sys_pack, ao_chn);
        } else {
            ao_drv_put_zero_data(ao_dev, ao_chn);
        }
    } else if (g_ao_chn[chn].chn_state == AIO_CHN_DISABLE) {
        ao_drv_put_zero_data(ao_dev, ao_chn);
        g_ao_chn[chn].buf.read = 0;
        g_ao_chn[chn].buf.write = 0;
    }
}

static td_s32 ao_put_dma_data(ot_audio_dev audio_dev_id)
{
    td_s32 chn;
    td_u32 i;
    unsigned long lockflag;
    aio_dev_ctx *aio_dev = &g_ao_dev[audio_dev_id];
    audio_pack *sys_pack = TD_NULL;
    aio_spin_lock_s *ao_buf_lock = ao_get_buf_lock();

    aio_spin_lock_irqsave(ao_buf_lock, lockflag);
    if (aio_dev->enable != TD_TRUE) {
        aio_spin_unlock_irqrestore(ao_buf_lock, lockflag);
        return OT_ERR_AO_NOT_ENABLED;
    }

    /* mix channel data */
    for (i = 0; i < aio_dev->aio_attr.chn_cnt; i++) {
        ao_put_chn_dma_data(audio_dev_id, i, sys_pack);
    }

    ao_drv_flush_data(audio_dev_id);

    /* 通知用户操作 */
    ao_drv_notify_user(audio_dev_id);

    aio_spin_unlock_irqrestore(ao_buf_lock, lockflag);
    return TD_SUCCESS;
}

td_void ao_save_dma_rd_ptr(ot_audio_dev ao_dev)
{
    /* 获取CB此时的读指针 */
    ao_drv_get_dma_rdpt(ao_dev, &((ao_drv_get_drv_dev_ctx(ao_dev))->rptr_offset_aec));
}

td_void ao_set_dma_rd_offset_init_flag(ot_audio_dev ao_dev, ot_ao_chn ao_chn)
{
    td_s32 chn = ao_get_chn_id(ao_dev, ao_chn);
    g_ao_chn[chn].aec_offset_init = TD_FALSE;
}

static td_void ao_get_data_update_aec_offset(ot_audio_dev ao_dev, ot_ao_chn ao_chn)
{
    td_u32 offset, rd_offset, wr_offset;
    td_u32 frame_time, align_bytes, time_offset, frame_offset;
    td_bool update_aec_offset = TD_FALSE;
    td_s32 chn = ao_get_chn_id(ao_dev, ao_chn);
    ot_aio_attr *ao_attr = &(g_ao_dev[ao_dev].aio_attr);
    aio_drv_dev_ctx *ao_drv_dev = ao_drv_get_drv_dev_ctx(ao_dev);

    frame_time = ao_attr->point_num_per_frame * 1000 * 1000 / ao_attr->sample_rate; /* 1000: kHz */
    if (frame_time > 10 * 1000) { /* 10, 1000: 10ms */
        time_offset = 8 * 1000; /* 8, 1000: 8ms */
    } else {
        time_offset = frame_time / 2; /* 2: half */
    }

    offset = ao_drv_dev->trans_len + ao_drv_dev->trans_len / ao_attr->chn_cnt * time_offset / frame_time;
    rd_offset = ao_drv_dev->rptr_offset_aec;

    /* ddr要求地址按32byte对齐 */
    align_bytes = 32; /* 32: align */
    if (rd_offset >= offset) {
        offset = ((rd_offset - offset) / align_bytes)  * align_bytes;
    } else {
        offset = ((ao_drv_dev->cir_buf.buf_size - (offset - rd_offset)) / align_bytes) * align_bytes;
    }

    wr_offset = ao_drv_dev->cir_buf.wptr_off_set;
    if ((wr_offset > rd_offset && (g_ao_chn[chn].aec_offset > rd_offset && g_ao_chn[chn].aec_offset < wr_offset)) ||
        (wr_offset < rd_offset && (g_ao_chn[chn].aec_offset > rd_offset || g_ao_chn[chn].aec_offset < wr_offset))) {
        update_aec_offset = TD_TRUE;
    } else {
        td_u32 distance;

        if (offset > g_ao_chn[chn].aec_offset) {
            distance = ((offset - g_ao_chn[chn].aec_offset) > ao_drv_dev->trans_len * 2) ? /* 2: bytes */
                (ao_drv_dev->cir_buf.buf_size - offset + g_ao_chn[chn].aec_offset) :
                (offset - g_ao_chn[chn].aec_offset);
        } else {
            distance = ((g_ao_chn[chn].aec_offset - offset) > ao_drv_dev->trans_len * 2) ? /* 2: bytes */
                (ao_drv_dev->cir_buf.buf_size + offset - g_ao_chn[chn].aec_offset) :
                (g_ao_chn[chn].aec_offset - offset);
        }

        time_offset = distance * frame_time * ao_attr->chn_cnt / ao_drv_dev->trans_len;
        frame_offset = distance / ao_drv_dev->trans_len;
        if ((time_offset > 8 * 1000) || (frame_offset > 2)) { /* 2,8,1000: 8ms */
            update_aec_offset = TD_TRUE;
        }
    }

    if ((g_ao_chn[chn].aec_offset_init == TD_FALSE) || (update_aec_offset == TD_TRUE)) {
        g_ao_chn[chn].aec_offset = offset;
        g_ao_chn[chn].aec_offset_init = TD_TRUE;
    }
}

td_s32 ao_get_data(ot_audio_dev ao_dev, ot_ao_chn ao_chn, ot_audio_frame *frm)
{
    td_s32 chn;
    aio_dev_ctx *dev_ctx = TD_NULL;
    td_s32 ret;
    aio_drv_dev_ctx *ao_drv_dev = ao_drv_get_drv_dev_ctx(ao_dev);

    ao_check_null_ptr_return(frm);
    ao_check_dev_return(ao_dev);
    ao_check_chn_id_return(ao_chn);

    dev_ctx = &g_ao_dev[ao_dev];

    /* 对AI的音频帧属性与AO的音频帧属性进行检查，是否一致，AO的音频帧长是否已经大于VB的大小 */
    if (frm->len != (dev_ctx->aio_attr.point_num_per_frame << ((td_u32)dev_ctx->aio_attr.bit_width))) {
        ao_err_trace("ao(%d, %d) frame size(%d) is not equal to ai frame size(%u)!\n",
            ao_dev, ao_chn,
            (dev_ctx->aio_attr.point_num_per_frame << ((td_u32)dev_ctx->aio_attr.bit_width)), frm->len);
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

    /* 检查数据位宽及声音模式是否匹配 */
    if (frm->bit_width != dev_ctx->aio_attr.bit_width) {
        ao_err_trace("ao frame bit_width: %d is not equal to ai bit_width: %d!\n", \
            dev_ctx->aio_attr.bit_width, frm->bit_width);
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

    if (frm->snd_mode != dev_ctx->aio_attr.snd_mode) {
        ao_err_trace("ao frame snd_mode: %d is not equal to ai snd_mode: %d!\n", \
            dev_ctx->aio_attr.snd_mode, frm->snd_mode);
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

    chn = ao_get_chn_id(ao_dev, ao_chn);
    /* 如果设备或通道没有使能则出错返回 */
    if ((dev_ctx->enable != TD_TRUE) || (g_ao_chn[chn].chn_state != AIO_CHN_ENABLE)) {
        ao_err_trace("ao dev or chn is not enable\n");
        frm->len = 0;
        return OT_ERR_AO_NOT_ENABLED;
    }

    /* 更新AEC的offset */
    ao_get_data_update_aec_offset(ao_dev, ao_chn);

    ret = ao_drv_get_aec_data(ao_dev, ao_chn, frm, g_ao_chn[chn].aec_offset);
    if (ret != TD_SUCCESS) {
        ao_err_trace("get aec ref frame failed!\n");
        return ret;
    }

    if ((g_ao_chn[chn].aec_offset + ao_drv_dev->trans_len) >= ao_drv_dev->cir_buf.buf_size) {
        g_ao_chn[chn].aec_offset = (g_ao_chn[chn].aec_offset + ao_drv_dev->trans_len) -
            ao_drv_dev->cir_buf.buf_size;
    } else {
        g_ao_chn[chn].aec_offset = g_ao_chn[chn].aec_offset + ao_drv_dev->trans_len;
    }

    return TD_SUCCESS;
}

td_s32 ao_set_dev_attr(ot_audio_dev audio_dev_id, const ot_aio_attr *attr)
{
    td_s32 ret;
    aio_dev_ctx *aio_dev = TD_NULL;

    ao_check_func_entrance_return(OT_ID_AIO);
    ao_check_dev_return(audio_dev_id);

    aio_dev = &g_ao_dev[audio_dev_id];

    if (attr == TD_NULL) {
        ao_err_trace("the ot_aio_attr pointer is NULL for audio_dev_id %d\n", (td_s32)audio_dev_id);
        return OT_ERR_AO_NULL_PTR;
    }
    if (osal_down_interruptible(&aio_dev->sem)) {
        return -ERESTARTSYS;
    }
    /* check whether dev is disable */
    if (g_ao_dev[audio_dev_id].enable != TD_FALSE) {
        osal_up(&aio_dev->sem);
        ao_err_trace("must disable dev when config dev attr\n");
        return OT_ERR_AO_NOT_PERM;
    }

    (void)audio_config_smmu();

    /* 配置设备属性 */
    ret = ao_drv_set_dev_attr(audio_dev_id, attr);
    if (ret != TD_SUCCESS) {
        ao_err_trace(KERN_ERR "ao_drv_set_dev_attr fail ret = %d \n", ret);
        osal_up(&aio_dev->sem);
        return ret;
    }

    ret = memcpy_s(&g_ao_dev[audio_dev_id].aio_attr, sizeof(g_ao_dev[audio_dev_id].aio_attr), attr, sizeof(*attr));
    if (ret != EOK) {
        osal_up(&aio_dev->sem);
        ao_err_trace("ao_dev %d attr memcpy_s fail, ret = 0x%x.\n", audio_dev_id, ret);
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

    g_ao_dev[audio_dev_id].cfg = TD_TRUE;

    osal_up(&aio_dev->sem);

    return TD_SUCCESS;
}

td_s32 ao_get_dev_attr(ot_audio_dev audio_dev_id, ot_aio_attr *attr)
{
    aio_dev_ctx *aio_dev = TD_NULL;
    td_s32 ret;

    ao_check_func_entrance_return(OT_ID_AIO);
    ao_check_dev_return(audio_dev_id);

    aio_dev = &g_ao_dev[audio_dev_id];

    if (attr == TD_NULL) {
        ao_err_trace("the ot_aio_attr pointer is NULL for aodev %d\n", audio_dev_id);
        return OT_ERR_AO_NULL_PTR;
    }
    if (osal_down_interruptible(&aio_dev->sem)) {
        return -ERESTARTSYS;
    }

    if (g_ao_dev[audio_dev_id].cfg != TD_TRUE) {
        osal_up(&aio_dev->sem);
        ao_err_trace("audio_dev_id %d not configured\n", audio_dev_id);
        return OT_ERR_AO_NOT_CFG;
    }

    ret = memcpy_s(attr, sizeof(*attr), &g_ao_dev[audio_dev_id].aio_attr, sizeof(g_ao_dev[audio_dev_id].aio_attr));
    if (ret != EOK) {
        osal_up(&aio_dev->sem);
        ao_err_trace("audio_dev_id %d attr memcpy_s fail, ret = 0x%x.\n", audio_dev_id, ret);
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

    osal_up(&aio_dev->sem);

    return TD_SUCCESS;
}

static td_void ao_chn_ctx_init(ot_audio_dev ao_dev)
{
    td_u32 i, chn;
    td_u8 *addr = TD_NULL;
    size_t offset = sizeof(struct osal_semaphore);

    /* init chn info of this dev */
    for (i = 0; i < OT_AO_MAX_CHN_NUM; i++) {
        chn = ao_get_chn_id(ao_dev, i);
        addr = (td_u8*)&g_ao_chn[chn];

        if (memset_s(addr + offset, sizeof(ao_chn_ctx) - offset, 0, sizeof(ao_chn_ctx) - offset) != EOK) {
            ao_warn_trace("ao dev %d chn %d memset_s failed.\n", ao_dev, i);
        }

        g_ao_chn[chn].chn_state = AIO_CHN_ORIGINAL;
        g_ao_chn[chn].dev_enable = TD_TRUE;
        g_ao_chn[chn].ao_vqe_dbg.vqe_enable = TD_FALSE;
        g_ao_chn[chn].ao_vqe_dbg.ao_vqe_cfg.anr_open = TD_FALSE;
        g_ao_chn[chn].ao_vqe_dbg.ao_vqe_cfg.frame_sample = 0;
        g_ao_chn[chn].ao_vqe_dbg.ao_vqe_cfg.work_sample_rate = OT_AUDIO_SAMPLE_RATE_BUTT;
        g_ao_chn[chn].resmp_dbg.resmp_enable = TD_FALSE;
        g_ao_chn[chn].resmp_dbg.resmp_attr.in_point_num = 0;
        g_ao_chn[chn].resmp_dbg.resmp_attr.in_sample_rate = OT_AUDIO_SAMPLE_RATE_BUTT;
        g_ao_chn[chn].resmp_dbg.resmp_attr.out_sample_rate = OT_AUDIO_SAMPLE_RATE_BUTT;
        g_ao_chn[chn].aec_offset = 0;
        g_ao_chn[chn].aec_offset_init = TD_FALSE;
        g_ao_chn[chn].save_file_info.cfg = TD_FALSE;
        g_ao_chn[chn].save_file_info.file_path[0] = '\0';
        g_ao_chn[chn].save_file_info.file_name[0] = '\0';
        g_ao_chn[chn].save_file_info.file_size = 0;
    }
}

static td_void ao_chn_ctx_exit(ot_audio_dev ao_dev, const ot_aio_attr *ao_attr)
{
    td_u32 i, chn;

    /* 重置通道上下文 */
    for (i = 0; i < ao_attr->chn_cnt; i++) {
        chn = ao_get_chn_id(ao_dev, i);
        g_ao_chn[chn].dev_enable = TD_FALSE;
        g_ao_chn[chn].chn_state = AIO_CHN_ORIGINAL;
        g_ao_chn[chn].int_lost = 0;
    }
}

td_s32 ao_enable_dev(ot_audio_dev audio_dev_id)
{
    td_s32 ret;
    aio_dev_ctx *aio_dev = TD_NULL;
    ot_aio_attr *attr = TD_NULL;
    unsigned long lockflag;

    ao_check_func_entrance_return(OT_ID_AIO);
    ao_check_dev_return(audio_dev_id);
    aio_dev = &g_ao_dev[audio_dev_id];

    if (!aio_dev->inited) {
        ao_err_trace("aodev %d not initializeed\n", (td_s32)audio_dev_id);
        return OT_ERR_AO_NOT_PERM;
    }

    if (osal_down_interruptible(&aio_dev->sem)) {
        return -ERESTARTSYS;
    }

    if (aio_dev->enable == TD_TRUE) {
        osal_up(&aio_dev->sem);
        return TD_SUCCESS;
    }
    if (aio_dev->cfg != TD_TRUE) {
        osal_up(&aio_dev->sem);
        ao_err_trace("aodev %d not configured\n", audio_dev_id);
        return OT_ERR_AO_NOT_CFG;
    }

    attr = &aio_dev->aio_attr;

    /* 初始化通道上下文 */
    ao_chn_ctx_init(audio_dev_id);

    /* 都使用该接口实现 */
    ret = ao_buf_init(audio_dev_id, attr);
    if (ret != TD_SUCCESS) {
        osal_up(&aio_dev->sem);
        return ret;
    }

    /* 所有芯片使用同一个接口，使能DMA或设备 */
    ret = ao_drv_enable_dev(audio_dev_id);
    if (ret != TD_SUCCESS) {
        osal_up(&aio_dev->sem);
        ao_buf_exit(audio_dev_id);
        return ret;
    }

    aio_spin_lock_irqsave(ao_get_buf_lock(), lockflag);
    aio_dev->enable = TD_TRUE;
    aio_spin_unlock_irqrestore(ao_get_buf_lock(), lockflag);

    osal_up(&aio_dev->sem);

    ao_warn_trace("aodev %d enable ok!\n", audio_dev_id);
    return TD_SUCCESS;
}

/* for ot_mpi_ao_disable() */
td_s32 ao_disable_dev(ot_audio_dev audio_dev_id)
{
    td_u32  i, chn;
    td_s32  ret;
    aio_dev_ctx *aio_dev = TD_NULL;
    unsigned long lockflag;

    ao_check_func_entrance_return(OT_ID_AIO);
    ao_check_dev_return(audio_dev_id);

    aio_dev = &g_ao_dev[audio_dev_id];
    if (!aio_dev->inited) {
        ao_info_trace("ao is not initializeed\n");
        return OT_ERR_AO_NOT_PERM;
    }
    if (osal_down_interruptible(&aio_dev->sem)) {
        return -ERESTARTSYS;
    }
    /* if device not enable ,return success */
    if (aio_dev->enable == TD_FALSE) {
        osal_up(&aio_dev->sem);
        return TD_SUCCESS;
    }

    /* should disable all ao channel first */
    for (i = 0; i < aio_dev->aio_attr.chn_cnt; i++) {
        chn = ao_get_chn_id(audio_dev_id, i);
        if ((g_ao_chn[chn].chn_state == AIO_CHN_ENABLE) || (g_ao_chn[chn].chn_state == AIO_CHN_PAUSE)) {
            osal_up(&aio_dev->sem);
            ao_err_trace("AO chn %u not disable,must disable all AO chn,then disable AO device\n", chn);
            return OT_ERR_AO_NOT_PERM;
        }
    }

    aio_spin_lock_irqsave(ao_get_buf_lock(), lockflag);
    /* stop DMA trans */
    aio_dev->enable = TD_FALSE;
    aio_spin_unlock_irqrestore(ao_get_buf_lock(), lockflag);

    /* 所有芯片都使用这个接口 */
    ao_drv_disable_dev(audio_dev_id);

    aio_spin_lock_irqsave(ao_get_buf_lock(), lockflag);
    /* 重置通道上下文 */
    ao_chn_ctx_exit(audio_dev_id, &(aio_dev->aio_attr));
    aio_spin_unlock_irqrestore(ao_get_buf_lock(), lockflag);

    osal_wakeup(&aio_dev->wait);

    /* 所有芯片都使用这个接口，释放分配的buffer */
    ao_buf_exit(audio_dev_id);

    /* clear dev pub attr:when enable dev */
    if (aio_dev->cfg && (aio_dev->enable == TD_FALSE)) {
        /* 清属性 */
        ret = ao_drv_clr_dev_attr(audio_dev_id);
        if (ret != TD_SUCCESS) {
            osal_up(&aio_dev->sem);
            return ret;
        }
    }

    aio_dev->cfg = TD_FALSE;  // disable之后，下一次enable需要重新配置属性
    osal_up(&aio_dev->sem);

    ao_info_trace("AO dev %d disable ok!\n", audio_dev_id);
    return TD_SUCCESS;
}

td_void ao_dev_sem_init(ot_audio_dev audio_dev_id)
{
    td_s32 ret, i, chn;
    aio_dev_ctx *aio_dev = TD_NULL;
    aio_dev = &g_ao_dev[audio_dev_id];
    (td_void)memset_s(aio_dev, sizeof(aio_dev_ctx), 0, sizeof(aio_dev_ctx));
    (td_void)osal_wait_init(&aio_dev->wait);

    (td_void)osal_sema_init(&aio_dev->sem, 1);

    for (i = 0; i < OT_AO_MAX_CHN_NUM; i++) {
        chn = ao_get_chn_id(audio_dev_id, i);
        (td_void)memset_s(&g_ao_chn[chn], sizeof(ao_chn_ctx), 0, sizeof(ao_chn_ctx));
        (td_void)osal_sema_init(&g_ao_chn[chn].sem, 1);
    }

    aio_spin_lock_init(&aio_dev->spinlock);
}

td_s32 ao_dev_init(ot_audio_dev audio_dev_id)
{
    td_s32 ret;
    aio_dev_ctx *aio_dev = TD_NULL;
    td_s32 i;
    td_u32 chn;

    ao_check_dev_return(audio_dev_id);

    aio_dev = &g_ao_dev[audio_dev_id];

    aio_dev->enable = TD_FALSE;

    ret = ao_drv_dev_init(audio_dev_id);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ao_drv_set_dma_isr(ao_dma_callback);

    aio_dev->inited = TD_TRUE;
    aio_dev->cfg = TD_FALSE;

    return TD_SUCCESS;
}

td_void ao_ctx_sem_exit(ot_audio_dev audio_dev_id)
{
    td_s32 ret;
    td_u32 chn, i;
    aio_dev_ctx *aio_dev = TD_NULL;
    aio_dev = &g_ao_dev[audio_dev_id];
    for (i = 0; i < OT_AO_MAX_CHN_NUM; i++) {
        chn = ao_get_chn_id(audio_dev_id, i);
        osal_sema_destroy(&g_ao_chn[chn].sem);
    }

    osal_wait_destroy(&aio_dev->wait);
    osal_sema_destroy(&aio_dev->sem);
    aio_spin_lock_destroy(&aio_dev->spinlock);
}

td_s32 ao_dev_exit(ot_audio_dev audio_dev_id)
{
    td_s32 ret;
    td_u32 chn, i;
    aio_dev_ctx *aio_dev = TD_NULL;

    ao_check_dev_return(audio_dev_id);

    aio_dev = &g_ao_dev[audio_dev_id];

    if (!aio_dev->inited) {
        return TD_SUCCESS;
    }

    ret = ao_drv_dev_deinit(audio_dev_id);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    /* disable all AO channel */
    for (i = 0; i < aio_dev->aio_attr.chn_cnt; i++) {
        chn = ao_get_chn_id(audio_dev_id, i);
        g_ao_chn[chn].chn_state = AIO_CHN_DISABLE;

        /* 系统退出时 唤醒可能一直阻塞的等待队列
            (否则在某些异常情况下，再次运行程序时，由于重新初始化等待队列导致正阻塞中的队列挂死) */
        osal_wakeup(&aio_dev->wait);
    }

    /* disable AO device, stop AO strans */
    ret = ao_disable_dev(audio_dev_id);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    aio_dev->inited = TD_FALSE;

    return TD_SUCCESS;
}

/* register DMA ISR */
td_void ao_dma_callback(ot_audio_dev audio_dev_id)
{
    unsigned long lock_flag;
    td_s32 ret;

    aio_spin_lock_irqsave(&g_ao_dev[audio_dev_id].spinlock, lock_flag);
    /* if device had stopped,return */
    if (g_ao_dev[audio_dev_id].enable == TD_FALSE) {
        aio_spin_unlock_irqrestore(&g_ao_dev[audio_dev_id].spinlock, lock_flag);
        return;
    }

    /* 调用函数拼接数据，发送数据 */
    ret = ao_put_dma_data(audio_dev_id);
    if (ret != TD_SUCCESS) {
        ao_warn_trace("AO dev %d put dma data fail\n", audio_dev_id);
    }

    aio_spin_unlock_irqrestore(&g_ao_dev[audio_dev_id].spinlock, lock_flag);

    return;
}

ao_chn_ctx *ao_get_chn_ctx(td_s32 chn_id)
{
    return &g_ao_chn[chn_id];
}

aio_dev_ctx *ao_get_dev_ctx(ot_audio_dev audio_dev_id)
{
    return &g_ao_dev[audio_dev_id];
}

td_s32 ao_clr_dev_attr(ot_audio_dev audio_dev_id)
{
    td_s32 ret;
    aio_dev_ctx *aio_dev = TD_NULL;

    ao_check_func_entrance_return(OT_ID_AIO);
    ao_check_dev_return(audio_dev_id);

    aio_dev = &g_ao_dev[audio_dev_id];
    if (osal_down_interruptible(&aio_dev->sem)) {
        return -ERESTARTSYS;
    }

    /* 设备没有禁止时不能清设备属性 */
    if (aio_dev->enable != TD_FALSE) {
        osal_up(&aio_dev->sem);
        ao_err_trace("AO dev %d is busy, disable it first\n", audio_dev_id);
        return OT_ERR_AO_NOT_PERM;
    }

    ret = ao_drv_clr_dev_attr(audio_dev_id);
    if (ret != TD_SUCCESS) {
        osal_up(&aio_dev->sem);
        return ret;
    }

    aio_dev->cfg = TD_FALSE;
    osal_up(&aio_dev->sem);

    return ret;
}
