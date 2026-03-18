/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: source of aio driver hal function
 * Author: Hisilicon multimedia software group
 * Create: 2020/05/18
 */

#include "aiao_hal.h"

#include "hi_osal.h"
#include "hi_comm_audio.h"
#include "ot_inner_common_aio.h"
#include "dev_ext.h"
#include "proc_ext.h"
#include "mod_ext.h"
#include "ai_ext.h"
#include "mkp_aio.h"
#include "sys_ext.h"

#include "aiao_hal_comm.h"
#include "aiao_reg.h"
#include "aiao_ext.h"

typedef union {
    struct {
        unsigned int    pad_tx_en           : 1; /* [0] */
        unsigned int    audio_rx_bclk_sel   : 1; /* [1] */
        unsigned int    rx_sd_sel           : 1; /* [2] */
        unsigned int    audio_mclk_sel      : 1; /* [3] */
        unsigned int    tx1_tx2_sel         : 1; /* [4] */
        unsigned int    reserved            : 27; /* [30..5] */
    } bits;
    unsigned int ul32;
} aio_i2s_mux_ctrl;

td_s32 aiao_hal_set_clk_sel_private(ot_audio_clk_select clk_select, aiao_clk_info *clk_info)
{
#ifndef OT_FPGA
    td_u32 aio_crg_clk_sel = 0;
    ot_mpp_chn mpp_chn = {0};
    mpp_chn.mod_id = OT_ID_AIO;
    mpp_chn.dev_id = 0;

    switch (clk_select) {
        case OT_AUDIO_CLK_SELECT_BASE:
            /* crg 时钟频率选择(选择786.432M) */
            clk_info->mclk_div_48k_256fs = AIO_MCLK_48K_256FS_786M;
            clk_info->mclk_div_441k_256fs = AIO_MCLK_441K_256FS_786M;
            clk_info->mclk_div_32k_256fs = AIO_MCLK_32K_256FS_786M;

            clk_info->mclk_div_48k_320fs = AIO_MCLK_48K_320FS_786M;
            clk_info->mclk_div_441k_320fs = AIO_MCLK_441K_320FS_786M;
            clk_info->mclk_div_32k_320fs = AIO_MCLK_32K_320FS_786M;

            aio_crg_clk_sel = 0x1;
            break;
        case OT_AUDIO_CLK_SELECT_SPARE:
            /* crg 时钟频率选择(选择1400M) */
            clk_info->mclk_div_48k_256fs = AIO_MCLK_48K_256FS_1400M;
            clk_info->mclk_div_441k_256fs = AIO_MCLK_441K_256FS_1400M;
            clk_info->mclk_div_32k_256fs = AIO_MCLK_32K_256FS_1400M;

            clk_info->mclk_div_48k_320fs = AIO_MCLK_48K_320FS_1400M;
            clk_info->mclk_div_441k_320fs = AIO_MCLK_441K_320FS_1400M;
            clk_info->mclk_div_32k_320fs = AIO_MCLK_32K_320FS_1400M;

            aio_crg_clk_sel = 0x0;
            break;
        default:
            return OT_ERR_AIO_ILLEGAL_PARAM;
    }

    clk_info->clk_select = clk_select;

    if (((td_bool)ckfn_sys_entry() == TD_FALSE) || ((td_bool)ckfn_sys_drv_ioctrl() == TD_FALSE)) {
        aio_err_trace("sys module may not insert!\n");
        return OT_ERR_AIO_NOT_READY;
    }

    if (call_sys_drv_ioctrl((hi_mpp_chn *)&mpp_chn, SYS_AIO_CLK_SEL, &aio_crg_clk_sel) != TD_SUCCESS) {
        aio_err_trace("set aio clk error!\n");
        return OT_ERR_AIO_ILLEGAL_PARAM;
    }
#else
    ot_unused(clk_select);
    ot_unused(clk_info);
#endif

    return TD_SUCCESS;
}

td_s32 aiao_hal_set_mod_param_private(const ot_audio_mod_param *mod_param)
{
    /* HI3536AV100 音频源头时钟有2个PLL */
    ot_unused(mod_param);
    return TD_SUCCESS;
}

td_bool check_aip_share_aop_clk_private(ot_audio_dev dev_id)
{
    /* HI3536AV100无AI与AO共时钟的需求 */
    if (dev_id == AIO_DEV_ID_0) {
        /* AIP0与AIP1只能对接内置codec或者外置codec，需要共时钟 */
        return TD_TRUE;
    }
    return TD_FALSE;
}

td_s32 aip_set_bclk_sel_private(ot_audio_dev dev_id, const ot_aio_attr *attr, td_u8 *aio_reg_base)
{
    u_aiao_switch_rx_bclk rx_switch_bclk;

    ot_unused(attr);

    /* HI3536AV100内部bclk选择 */
    rx_switch_bclk.u32 = *(volatile td_u32 *)(td_void *)(aio_reg_base + AIP_SWITCH_RX_BCLK);
    switch (dev_id) {
        case AIO_DEV_ID_0:
            rx_switch_bclk.bits.inner_bclk_ws_sel_rx_00 = 0x8; /* 共用ao0的时钟,crg8 */
            break;
        default:
            aio_err_trace("invalid ai dev:%d\n", dev_id);
            return OT_ERR_AI_INVALID_DEV_ID;
    }
    *(volatile td_u32 *)(td_void *)(aio_reg_base + AIP_SWITCH_RX_BCLK) = rx_switch_bclk.u32;

    return TD_SUCCESS;
}

td_s32 aip_set_mask_private(ot_audio_dev dev_id, const ot_aio_attr *aio_attr, td_u8 *reg_base)
{
    /* HI3536AV100的i2s_mux放在AIAO寄存器中 */
    aio_i2s_mux_ctrl i2s_mux;
    i2s_mux.ul32 = *(volatile td_u32 *)(td_void *)(reg_base + AUDIO_AIO_I2S_MUX_REG);

    if (dev_id == AIO_DEV_ID_0) {
        if (aio_attr->i2s_type == OT_AIO_I2STYPE_INNERCODEC) {
            i2s_mux.bits.rx_sd_sel = 0x1; /* 内置codec */
            i2s_mux.bits.audio_rx_bclk_sel = 0x1; /* BCLK选择CRG8 */
            i2s_mux.bits.audio_mclk_sel = 0x1; /* MCLK选择CRG8 */
        } else if (aio_attr->i2s_type == OT_AIO_I2STYPE_EXTERN) {
            i2s_mux.bits.rx_sd_sel = 0x0; /* I2S管脚输入 */
        }
    }

    *(volatile td_u32 *)(td_void *)(reg_base + AUDIO_AIO_I2S_MUX_REG) = i2s_mux.ul32;

    return TD_SUCCESS;
}

td_s32 check_ai_attr_private(ot_audio_dev ai_dev_id, const ot_aio_attr *attr)
{
    /* 检查I2STYPE. */
    switch (ai_dev_id) {
        case AIO_DEV_ID_0:
            if ((attr->i2s_type != OT_AIO_I2STYPE_INNERCODEC) && (attr->i2s_type != OT_AIO_I2STYPE_EXTERN)) {
                aio_err_trace("ai_dev 0 only support i2s_type: AIO_I2STYPE_INNERCODEC & EXTERN!\n");
                return OT_ERR_AI_ILLEGAL_PARAM;
            }
            break;
        default:
            aio_err_trace("unsupported ai_dev %d!\n", ai_dev_id);
            return OT_ERR_AI_INVALID_DEV_ID;
    }

    return TD_SUCCESS;
}

td_s32 aop_set_mask_private(ot_audio_dev dev_id, const ot_aio_attr *attr, td_u8 *reg_base)
{
    /* HI3536AV100的i2s_mux放在AIO寄存器中 */
    aio_i2s_mux_ctrl i2s_mux;
    i2s_mux.ul32 = *(volatile td_u32 *)(td_void *)(reg_base + AUDIO_AIO_I2S_MUX_REG);

    if (dev_id == AIO_DEV_ID_0) {
        if (attr->i2s_type == OT_AIO_I2STYPE_EXTERN) {
            i2s_mux.bits.pad_tx_en = 0x1; /* I2S选择tx0_sd输出 */
        } else if (attr->i2s_type == OT_AIO_I2STYPE_INNERCODEC) {
            i2s_mux.bits.pad_tx_en = 0x0; /* TX0 I2S输出为0 */
            i2s_mux.bits.audio_mclk_sel = 0x1; /* MCLK选择CRG8 */
        }
    } else if (dev_id == AIO_DEV_ID_1) {
        if (attr->i2s_type == OT_AIO_I2STYPE_EXTERN) {
            i2s_mux.bits.tx1_tx2_sel = 0x0; /* I2S1_TX管脚数据来自tx1 */
        }
    } else if (dev_id == AIO_DEV_ID_2) {
        if (attr->i2s_type == OT_AIO_I2STYPE_EXTERN) {
            i2s_mux.bits.tx1_tx2_sel = 0x1; /* I2S1_TX管脚数据来自tx2 */
        }
    }

    *(volatile td_u32 *)(td_void *)(reg_base + AUDIO_AIO_I2S_MUX_REG) = i2s_mux.ul32;

    return TD_SUCCESS;
}

td_s32 check_ao_attr_private(ot_audio_dev ao_dev_id, const ot_aio_attr *attr)
{
    ot_unused(ao_dev_id);
    ot_unused(attr);
    return TD_SUCCESS;
}

td_s32 aip_hal_set_acodec_gain_private(td_s32 gain, td_u8 *acodec_reg_base)
{
    /* 等适配内置codec时再增加处理 */
    ot_unused(gain);
    ot_unused(acodec_reg_base);
    return TD_SUCCESS;
}

td_s32 aip_hal_get_acodec_gain_private(td_s32 *gain, const td_u8 *acodec_reg_base)
{
    /* 等适配内置codec时再增加处理 */
    ot_unused(gain);
    ot_unused(acodec_reg_base);
    return TD_SUCCESS;
}

td_void aiao_module_init_private(td_u8 *aio_reg_base)
{
#ifndef OT_FPGA
    u_vhb_outstanding outstanding;
    u_i2s_crg_cfg0 i2s_crg_cfg0;
    u_tx_if_attri aop_attr_reg;

    outstanding.u32 = *(volatile td_u32 *)(td_void *)(aio_reg_base + AIAO_OUTSTANDING_REG);
    outstanding.bits.vhb_outst_num = 0x3;
    *(volatile td_u32 *)(td_void *)(aio_reg_base + AIAO_OUTSTANDING_REG) = outstanding.u32;

    /* 配置默认mclk为48k，低功耗需要，同时防止输出高频MCLK */
    i2s_crg_cfg0.bits.aiao_mclk_div = AIO_MCLK_48K_256FS_1400M;
    i2s_crg_cfg0.bits.reserved_0 = 0;
    *(volatile td_u32 *)(td_void *)(aio_reg_base + aip_i2s_reg_cfg0(0)) = i2s_crg_cfg0.u32;
    *(volatile td_u32 *)(td_void *)(aio_reg_base + aop_i2s_reg_cfg0(0)) = i2s_crg_cfg0.u32;
    *(volatile td_u32 *)(td_void *)(aio_reg_base + aop_i2s_reg_cfg0(1)) = i2s_crg_cfg0.u32;
    *(volatile td_u32 *)(td_void *)(aio_reg_base + aop_i2s_reg_cfg0(2)) = i2s_crg_cfg0.u32; /* 2: TX2 */

    /* 规避默认数据源为TX0的问题，否则TX1/TX2通道会默认出TX0的数据 */
    aop_attr_reg.u32 = *(volatile td_u32 *)(td_void *)(aio_reg_base + aop_inf_attri_reg(1));
    aop_attr_reg.bits.tx_sd_source_sel = 0x1;
    *(volatile td_u32 *)(td_void *)(aio_reg_base + aop_inf_attri_reg(1)) = aop_attr_reg.u32;

    aop_attr_reg.u32 = *(volatile td_u32 *)(td_void *)(aio_reg_base + aop_inf_attri_reg(2)); /* 2: TX2 */
    aop_attr_reg.bits.tx_sd_source_sel = 0x2;
    *(volatile td_u32 *)(td_void *)(aio_reg_base + aop_inf_attri_reg(2)) = aop_attr_reg.u32; /* 2: TX2 */
#else
    ot_unused(aio_reg_base);
#endif
}
