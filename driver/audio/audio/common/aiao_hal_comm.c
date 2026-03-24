/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Description: source of aio driver common function
 * Author: Hisilicon multimedia software group
 * Create: 2012/10/14
 * History:
 *   1.Date        : 2012/10/14
 *     Modification: Created file
 *   2.Date        : 2019/08/08
 *     Modification: Update file for CSEC
 */

#include <linux/kernel.h>
#include "hi_osal.h"
#include "securec.h"
#include "autoconf.h"
#include "hi_comm_audio.h"
#ifdef CONFIG_OT_PLATFORM_V8
#include "ot_inner_common_aio.h"
#else
#include "ot_comm_aio_adapt.h"
#endif
#include "hi_board.h"
#include "dev_ext.h"
#include "mod_ext.h"
#include "sys_ext.h"
#include "proc_ext.h"
#include "ai_ext.h"
#include "mkp_aio.h"

#include "aiao_hal_comm.h"
#include "aiao_hal.h"
#include "aiao_reg.h"
#include "aiao_ext.h"
#include "asp_dma.h"
#include "asp_dma_hal.h"

#ifdef CONFIG_AUDIO_V200_SUPPORT
#include "hi19xx_i2s.h"
#endif

#ifdef AUDIO_UT_VCAST
#include <linux/device.h>
#endif

#define AIO_STATE_STARTED 0U
#define AIO_STATE_STOPING 1U
#define AIO_STATE_STOPED  2U

#define AI_CHN_CNT_20 20U
#define AO_CHN_CNT_20 20U

#ifdef CONFIG_AUDIO_V200_SUPPORT
void *g_reg_i2s_base         = TD_NULL;
void *g_reg_aix2tdm_i2s_base = TD_NULL;

static td_bool g_i2s_reg_aix2tdm_by_dts = TD_TRUE;
static td_bool g_i2s_reg_by_dts         = TD_TRUE;
#endif

static aio_ctx g_aio_ctx;
static aio_state g_aio_state[OT_AIO_MAX_NUM];

void    *g_reg_acodec_base = TD_NULL;
void    *g_acodec_base     = TD_NULL;
td_void *g_aio_base        = TD_NULL;

static td_u32 g_aio_cur_state = AIO_STATE_STOPED;
td_u32 g_aiao_irq             = AIO_IRQ_NR;

td_bool g_soft_differential         = TD_FALSE;
static  td_bool g_aiao_reg_by_dts   = TD_TRUE;
static  td_bool g_acodec_reg_by_dts = TD_TRUE;

static td_u32 g_au32_ai_int_status[OT_AIO_MAX_NUM] = {0};
static td_u32 g_au32_ao_int_status[OT_AIO_MAX_NUM] = {0};
static td_u32 g_ai_en_status[OT_AI_DEV_MAX_NUM]    = {0};  // 0:disable; 1:enable;
static td_u32 g_ao_en_status[OT_AO_DEV_MAX_NUM]    = {0};
td_u32 g_aip_buff_rptr[AI_CHN_CNT_20]              = {0};
td_u32 g_aop_buff_wptr[AO_CHN_CNT_20]              = {0};

static osal_atomic_t g_aio_user_ref = OSAL_ATOMIC_INIT(0);
static struct osal_semaphore g_sem_for_aio_ctx;

#ifdef OT_FPGA
aiao_clk_info g_aio_clk_info = {
    .clk_select          = OT_AUDIO_CLK_SELECT_BASE,
    .mclk_div_48k_256fs  = AIO_MCLK_48K,
    .mclk_div_441k_256fs = AIO_MCLK_441K,
    .mclk_div_32k_256fs  = AIO_MCLK_32K,
    .mclk_div_48k_320fs  = AIO_MCLK_48K_320FS,
    .mclk_div_441k_320fs = AIO_MCLK_441K_320FS,
    .mclk_div_32k_320fs  = AIO_MCLK_32K_320FS,
};
#else
aiao_clk_info g_aio_clk_info = {
    .clk_select          = OT_AUDIO_CLK_SELECT_BASE,
    .mclk_div_48k_256fs  = AIO_MCLK_48K_256FS_786M,
    .mclk_div_441k_256fs = AIO_MCLK_441K_256FS_786M,
    .mclk_div_32k_256fs  = AIO_MCLK_32K_256FS_786M,
    .mclk_div_48k_320fs  = AIO_MCLK_48K_320FS_786M,
    .mclk_div_441k_320fs = AIO_MCLK_441K_320FS_786M,
    .mclk_div_32k_320fs  = AIO_MCLK_32K_320FS_786M,
};
#endif

#ifndef SYS_AIO_SAMPLE_CLK320
#define SYS_AIO_SAMPLE_CLK320 0x06U
#endif

#ifndef SYS_AIO_SAMPLE_CLK384
#define SYS_AIO_SAMPLE_CLK384 0x07U
#endif

td_void aiao_set_aio_base(td_void *aio_base)
{
    if (aio_base == TD_NULL) {
        aio_err_trace("aio_base is null.\n");
        return;
    }
    g_aio_base = aio_base;
}

td_void *aiao_get_aio_base(td_void)
{
    return g_aio_base;
}

td_void aiao_set_acodec_base(td_void *acodec_base)
{
    if (acodec_base == TD_NULL) {
        aio_err_trace("acodec_base is null.\n");
        return;
    }
    g_acodec_base = acodec_base;
}

td_void *aiao_get_acodec_base(td_void)
{
    return g_acodec_base;
}

td_void aiao_set_reg_acodec_base(td_void *reg_acodec_base)
{
    if (reg_acodec_base == TD_NULL) {
        aio_err_trace("reg_acodec_base is null.\n");
        return;
    }
    g_reg_acodec_base = reg_acodec_base;
}

td_void aiao_set_aiao_irq(td_u32 aiao_irq)
{
    g_aiao_irq = aiao_irq;
}

static td_s32 aiao_reg_addr_init(td_void)
{
    g_i2s_reg_by_dts = TD_TRUE;
    g_i2s_reg_aix2tdm_by_dts = TD_TRUE;

    if (g_reg_i2s_base != TD_NULL) {
        return TD_SUCCESS;
    }

    g_reg_i2s_base = osal_ioremap(I2S_REG_ADDR, I2S_REG_SIZE);
    if (g_reg_i2s_base == TD_NULL) {
        aio_err_trace("i2s ioremap failed\n");
        return TD_FAILURE;
    }
    g_i2s_reg_by_dts = TD_FALSE;

    if (g_reg_aix2tdm_i2s_base != TD_NULL) {
        return TD_SUCCESS;
    }

    g_reg_aix2tdm_i2s_base = osal_ioremap(I2S_REG_AXITDM_ADDR, I2S_REG_AXITDM_SIZE);
    if (g_reg_aix2tdm_i2s_base == TD_NULL) {
        osal_iounmap(g_reg_i2s_base);
        aio_err_trace("i2s axi2tdm ioremap failed\n");
        return TD_FAILURE;
    }
    g_i2s_reg_aix2tdm_by_dts = TD_FALSE;

    return TD_SUCCESS;
}

static td_void aiao_reg_addr_deinit(td_void)
{
    if ((g_i2s_reg_by_dts == TD_FALSE) && (g_reg_i2s_base != TD_NULL)) {
        osal_iounmap(g_reg_i2s_base);
        g_reg_i2s_base = TD_NULL;
    }

    if ((g_i2s_reg_aix2tdm_by_dts == TD_FALSE) && (g_reg_aix2tdm_i2s_base != TD_NULL)) {
        osal_iounmap(g_reg_aix2tdm_i2s_base);
        g_reg_aix2tdm_i2s_base = TD_NULL;
    }
}

static inline td_u8 *aiao_hal_get_i2s_mux_reg(i2s_mux_type type)
{
    if (type == I2S_MUX_ACODEC) {
        return (td_u8 *)g_reg_acodec_base;
    } else if (type == I2S_MUX_AIAO) {
        return (td_u8 *)g_aio_base;
    } else {
        return TD_NULL;
    }
}

static td_u32 aiao_hal_read_reg(td_u32 offset)
{
    if (g_aio_base == TD_NULL) {
        aio_err_trace("g_aio_base is null.\n");
        return 0U;
    }
    return *(volatile td_u32 *)((td_u8 *)g_aio_base + offset);
}

static td_void aiao_hal_write_reg(td_u32 offset, td_u32 value)
{
    if (g_aio_base == TD_NULL) {
        aio_err_trace("g_aio_base is null.\n");
        return;
    }
    *(volatile td_u32 *)((td_u8 *)g_aio_base + offset) = value;
}

/* 将物理基址映射为虚拟基址 */
static td_s32 aiao_hal_sys_init(td_void)
{
#ifndef CONFIG_AUDIO_V200_SUPPORT
    return aiao_reg_addr_init();
#else
    return TD_SUCCESS;
#endif
}

static td_u32 aiao_hal_get_int_status(td_void)
{
    return aiao_hal_read_reg(AIAO_INT_STATUS_REG);
}

/* 解映射 */
static td_void aiao_hal_sys_deinit(td_void)
{
#ifndef CONFIG_AUDIO_V200_SUPPORT
    aiao_reg_addr_deinit();
#endif
}

/* chip code. different chip has different clksel. */
static td_s32 aiao_hal_set_clk_sel(ot_audio_clk_select clk_select)
{
#ifndef OT_FPGA
    return aiao_hal_set_clk_sel_private(clk_select, &g_aio_clk_info);
#else
    return TD_SUCCESS;
#endif
}

static td_s32 aiao_hal_get_clk_sel(ot_audio_clk_select *clk_select)
{
    if (clk_select == TD_NULL) {
        aio_err_trace("clk_select is null.\n");
        return OT_ERR_AIO_NULL_PTR;
    }

    *clk_select = g_aio_clk_info.clk_select;

    return TD_SUCCESS;
}

static td_s32 aiao_hal_set_mod_param(const ot_audio_mod_param *mod_param)
{
    if (mod_param == TD_NULL) {
        return OT_ERR_AIO_NULL_PTR;
    }

    td_s32 ret;
    ret = aiao_hal_set_mod_param_private(mod_param);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = aiao_hal_set_clk_sel(mod_param->clk_select);
    return ret;
}

static td_s32 aiao_hal_get_mod_param(ot_audio_mod_param *mod_param)
{
    if (mod_param == TD_NULL) {
        return OT_ERR_AIO_NULL_PTR;
    }

    return aiao_hal_get_clk_sel(&mod_param->clk_select);
}

static td_bool check_attr_sample_rate(ot_audio_sample_rate sample_rate)
{
    if ((sample_rate != OT_AUDIO_SAMPLE_RATE_8000) && (sample_rate != OT_AUDIO_SAMPLE_RATE_12000) &&
        (sample_rate != OT_AUDIO_SAMPLE_RATE_11025) && (sample_rate != OT_AUDIO_SAMPLE_RATE_16000) &&
        (sample_rate != OT_AUDIO_SAMPLE_RATE_22050) && (sample_rate != OT_AUDIO_SAMPLE_RATE_24000) &&
        (sample_rate != OT_AUDIO_SAMPLE_RATE_32000) && (sample_rate != OT_AUDIO_SAMPLE_RATE_44100) &&
        (sample_rate != OT_AUDIO_SAMPLE_RATE_48000) && (sample_rate != OT_AUDIO_SAMPLE_RATE_64000) &&
        (sample_rate != OT_AUDIO_SAMPLE_RATE_96000)) {
        return TD_FALSE;
    }
    return TD_TRUE;
}

static td_s32 aiao_hal_check_aiao_clk(const ot_aio_attr *ai_attr, const ot_aio_attr *ao_attr)
{
    if (ai_attr->work_mode != ao_attr->work_mode) {
        aio_err_trace("work_mode of AI and AO should be same when clk_share=1!\n");
        return TD_FAILURE;
    }

    if (ai_attr->sample_rate != ao_attr->sample_rate) {
        aio_err_trace("sample_rate of AI and AO should be same when clk_share=1!\n");
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static const ot_aio_attr *aiao_hal_get_aiao_clk_share_attr(const ot_aio_attr *ai_attr, const ot_aio_attr *ao_attr)
{
    td_u32 ai_chn_cnt, ao_chn_cnt;
    td_u32 ai_bclk_cnt, ao_bclk_cnt;

    /* chn_cnt=1时,实际bclk是按2chn来计算 */
    if (ai_attr->chn_cnt == 1) {
        ai_chn_cnt = 2; /* 2: 2chn */
    } else {
        ai_chn_cnt = ai_attr->chn_cnt;
    }

    /* chn_cnt=1时, 实际bclk是按2chn来计算;chn_cnt=8时也按2chn计算, 因为采用了多根数据线. */
    if (ao_attr->chn_cnt == 1) {
        ao_chn_cnt = 2; /* 2: 2chn */
    } else if (ao_attr->chn_cnt == 8) { /* 8: 2chn*4sd */
        if (AOP_SUPPORT_MULTI_SLOT == TD_TRUE) {
            ao_chn_cnt = 8; /* 8: 8chn, TDM */
        } else {
            ao_chn_cnt = 2; /* 2: 2chn, Normal */
        }
    } else {
        ao_chn_cnt = ao_attr->chn_cnt;
    }

    ai_bclk_cnt = ai_chn_cnt * aio_get_bit_cnt(ai_attr->bit_width);
    ao_bclk_cnt = ao_chn_cnt * aio_get_bit_cnt(ao_attr->bit_width);
    if (ai_bclk_cnt > ao_bclk_cnt) {
        return ai_attr;
    } else {
        return ao_attr;
    }
}

static const ot_aio_attr *aip_get_i2s_clk_attr(ot_audio_dev ai_dev_id, const ot_aio_attr *ai_attr)
{
    if ((g_aio_state[ai_dev_id].ao_config == TD_TRUE) &&
        ((ai_attr->clk_share == AIO_CLK_SEL_INSEPARATE) ||
        (check_aip_share_aop_clk_private(ai_dev_id) == TD_TRUE))) {
        /* AO已配置且AI与AO时钟共享时需要比较AOP的ATTR */
        return aiao_hal_get_aiao_clk_share_attr(ai_attr, &(g_aio_state[ai_dev_id].ao_attr));
    } else {
        return ai_attr;
    }
}

static const ot_aio_attr *aop_get_i2s_clk_attr(ot_audio_dev ao_dev_id, const ot_aio_attr *ao_attr)
{
    if ((g_aio_state[ao_dev_id].ai_config == TD_TRUE) &&
        ((g_aio_state[ao_dev_id].ai_attr.clk_share == AIO_CLK_SEL_INSEPARATE) ||
        (check_aip_share_aop_clk_private(ao_dev_id) == TD_TRUE))) {
        /* AI已配置且AI与AO时钟共享时需要比较AIP的ATTR */
        return aiao_hal_get_aiao_clk_share_attr(&(g_aio_state[ao_dev_id].ai_attr), ao_attr);
    } else {
        return ao_attr;
    }
}

static aio_type aip_get_i2s_dev_type(ot_audio_dev ai_dev_id, const ot_aio_attr *ai_attr)
{
    if (aip_get_i2s_clk_attr(ai_dev_id, ai_attr) == ai_attr) {
        return AIO_TYPE_AI;
    } else {
        return AIO_TYPE_AO;
    }
}

static aio_type aop_get_i2s_dev_type(ot_audio_dev ao_dev_id, const ot_aio_attr *ao_attr)
{
    if (aop_get_i2s_clk_attr(ao_dev_id, ao_attr) == ao_attr) {
        return AIO_TYPE_AO;
    } else {
        return AIO_TYPE_AI;
    }
}

static td_s32 aiao_get_mclk_cfg(const ot_aio_attr *attr, td_u32 *mclk_sel)
{
    switch (attr->sample_rate) {
        case OT_AUDIO_SAMPLE_RATE_8000:
        case OT_AUDIO_SAMPLE_RATE_16000:
        case OT_AUDIO_SAMPLE_RATE_32000:
        case OT_AUDIO_SAMPLE_RATE_64000: {
            *mclk_sel = (attr->chn_cnt == AI_CHN_CNT_20) ?
                g_aio_clk_info.mclk_div_32k_320fs : g_aio_clk_info.mclk_div_32k_256fs;
            break;
        }

        case OT_AUDIO_SAMPLE_RATE_12000:
        case OT_AUDIO_SAMPLE_RATE_24000:
        case OT_AUDIO_SAMPLE_RATE_48000:
        case OT_AUDIO_SAMPLE_RATE_96000: {
            *mclk_sel = (attr->chn_cnt == AI_CHN_CNT_20) ?
                g_aio_clk_info.mclk_div_48k_320fs : g_aio_clk_info.mclk_div_48k_256fs;
            break;
        }

        case OT_AUDIO_SAMPLE_RATE_11025:
        case OT_AUDIO_SAMPLE_RATE_22050:
        case OT_AUDIO_SAMPLE_RATE_44100: {
            *mclk_sel = (attr->chn_cnt == AI_CHN_CNT_20) ?
                g_aio_clk_info.mclk_div_441k_320fs : g_aio_clk_info.mclk_div_441k_256fs;
            break;
        }

        default: {
            aio_err_trace("not support this sample rate\n");
            return OT_ERR_AIO_ILLEGAL_PARAM;
        }
    }

    return TD_SUCCESS;
}

static td_void aiao_get_fs_bit(aio_type dev_type, const ot_aio_attr *attr, td_u32 *fs_bit)
{
    /* 采样位宽 */
    if (attr->chn_cnt == 1) {
        if ((attr->bit_width == OT_AUDIO_BIT_WIDTH_16) && (attr->expand_flag == OT_AI_CUT)) {
            *fs_bit = 2 * aio_get_bit_cnt(OT_AUDIO_BIT_WIDTH_24); /* 2: 2chn */
        } else {
            *fs_bit = 2 * aio_get_bit_cnt(attr->bit_width); /* 2: 2chn */
        }
    } else {
        if ((attr->bit_width == OT_AUDIO_BIT_WIDTH_16) && (attr->expand_flag == OT_AI_CUT)) {
            if ((dev_type == AIO_TYPE_AO) && (attr->chn_cnt > AIO_CHN_CNT_2) && (AOP_SUPPORT_MULTI_SLOT == TD_FALSE)) {
                /* AO Normal多通道实现采用多根I2S数据线，非时分复用 */
                *fs_bit = 2 * aio_get_bit_cnt(OT_AUDIO_BIT_WIDTH_24); /* 2: 2chn */
            } else {
                *fs_bit = attr->chn_cnt * aio_get_bit_cnt(OT_AUDIO_BIT_WIDTH_24); /* 2: 2chn */
            }
        } else {
            if ((dev_type == AIO_TYPE_AO) && (attr->chn_cnt > AIO_CHN_CNT_2) && (AOP_SUPPORT_MULTI_SLOT == TD_FALSE)) {
                /* AO Normal多通道实现采用多根I2S数据线，非时分复用 */
                *fs_bit = 2 * aio_get_bit_cnt(OT_AUDIO_BIT_WIDTH_24); /* 2: 2chn */
            } else {
                *fs_bit = attr->chn_cnt * aio_get_bit_cnt(attr->bit_width);
            }
        }
    }
}

td_void aip_hal_int_en(ot_audio_dev ai_dev_id, td_bool en)
{
    if (ai_dev_id >= OT_AI_DEV_MAX_NUM) {
        aio_err_trace("ai_dev %d is invalid!\n", ai_dev_id);
        return;
    }

    u_aiao_int_ena un_tmp;
    un_tmp.u32 = aiao_hal_read_reg(AIAO_INT_ENA_REG);

    switch (ai_dev_id) {
        case AIO_DEV_ID_0:
            un_tmp.bits.rx_ch0_int_ena = en;
            break;
        case AIO_DEV_ID_1:
            un_tmp.bits.rx_ch1_int_ena = en;
            break;
        case AIO_DEV_ID_2:
            un_tmp.bits.rx_ch2_int_ena = en;
            break;
        default:
            break;
    }

    aiao_hal_write_reg(AIAO_INT_ENA_REG, un_tmp.u32);
}

#ifdef AIAO_BAK_FUNC
td_u32 aip_hal_get_buff_wptr(ot_audio_dev ai_dev_id)
{
    if (ai_dev_id >= OT_AI_DEV_MAX_NUM) {
        aio_err_trace("ai_dev %d is invalid!\n", ai_dev_id);
        return 0U;
    }

    u_rx_buff_wptr un_tmp;
    un_tmp.u32 = aiao_hal_read_reg(aip_buff_wptr_reg(ai_dev_id));

    td_u32 status;
    status = un_tmp.bits.rx_buff_wptr;

    return status;
}
#endif

td_void aip_hal_set_buff_rptr(ot_audio_dev ai_dev_id, td_u32 value)
{
    if (ai_dev_id >= OT_AI_DEV_MAX_NUM) {
        aio_err_trace("ai_dev %d is invalid!\n", ai_dev_id);
        return;
    }

    g_aip_buff_rptr[ai_dev_id] = value;
}


td_u32 aip_hal_get_buff_rptr(ot_audio_dev ai_dev_id)
{
    if (ai_dev_id >= OT_AI_DEV_MAX_NUM) {
        aio_err_trace("ai_dev %d is invalid!\n", ai_dev_id);
        return 0U;
    }

    td_u32 status;
    status = g_aip_buff_rptr[ai_dev_id];

    return status;
}

td_void aip_hal_set_child_int_mask(ot_audio_dev ai_dev_id)
{
    if (ai_dev_id >= OT_AI_DEV_MAX_NUM) {
        aio_err_trace("ai_dev %d is invalid!\n", ai_dev_id);
        return;
    }

    /* 使能传输完成中断，buf满中断，fifo上溢中断 */
    u_rx_int_ena un_tmp;
    un_tmp.u32                        = aiao_hal_read_reg(aip_int_ena_reg(ai_dev_id));
    un_tmp.bits.rx_trans_int_ena      = 1U;
    un_tmp.bits.rx_full_int_ena       = 1U;
    un_tmp.bits.rx_alfull_int_ena     = 0U;
    un_tmp.bits.rx_bfifo_full_int_ena = 1U;
    un_tmp.bits.rx_ififo_full_int_ena = 0U;
    un_tmp.bits.rx_stop_int_ena       = 0U;
    un_tmp.bits.reserved_0            = 0U;

    aiao_hal_write_reg(aip_int_ena_reg(ai_dev_id), un_tmp.u32);
}

td_u32 aip_hal_get_child_int_all_status(ot_audio_dev ai_dev_id)
{
    if (ai_dev_id >= OT_AI_DEV_MAX_NUM) {
        aio_err_trace("ai_dev %d is invalid!\n", ai_dev_id);
        return 0U;
    }

    return aiao_hal_read_reg(aip_int_status_reg(ai_dev_id));
}

td_void aip_hal_clr_child_int_all_status(ot_audio_dev ai_dev_id)
{
    if (ai_dev_id >= OT_AI_DEV_MAX_NUM) {
        aio_err_trace("ai_dev %d is invalid!\n", ai_dev_id);
        return;
    }

    /* 清除使能的所有中断 */
    u_rx_int_clr int_mask;
    int_mask.bits.rx_trans_int_clear      = 1U;
    int_mask.bits.rx_full_int_clear       = 1U;
    int_mask.bits.rx_alfull_int_clear     = 1U;
    int_mask.bits.rx_bfifo_full_int_clear = 1U;
    int_mask.bits.rx_ififo_full_int_clear = 1U;
    int_mask.bits.rx_stop_int_clear       = 1U;

    aiao_hal_write_reg(aip_int_clr_reg(ai_dev_id), int_mask.u32);
}

td_void aip_hal_set_child_int_clr(ot_audio_dev ai_dev_id)
{
    if (ai_dev_id >= OT_AI_DEV_MAX_NUM) {
        aio_err_trace("ai_dev %d is invalid!\n", ai_dev_id);
        return;
    }

    u_rx_int_clr int_mask;
    int_mask.u32                          = aiao_hal_read_reg(aip_int_status_reg(ai_dev_id));
    int_mask.bits.rx_bfifo_full_int_clear = 0U;
    int_mask.bits.rx_full_int_clear       = 0U;
    int_mask.bits.rx_trans_int_clear      = 0U;

    if (g_au32_ai_int_status[ai_dev_id] & 0x08U) {  // fifo over
        int_mask.bits.rx_bfifo_full_int_clear = 1U;
    }

    if (g_au32_ai_int_status[ai_dev_id] & 0x02U) {  // buffer full
        int_mask.bits.rx_full_int_clear = 1U;
    }

    if (g_au32_ai_int_status[ai_dev_id] & 0x01U) {  // trans finish
        int_mask.bits.rx_trans_int_clear = 1U;
    }

    aiao_hal_write_reg(aip_int_clr_reg(ai_dev_id), int_mask.u32);
}

td_s32 aip_hal_clr_int_status(ot_audio_dev ai_dev_id)
{
#ifndef CONFIG_AUDIO_V200_SUPPORT
    aip_hal_set_child_int_clr(ai_dev_id);
#endif
    return TD_SUCCESS;
}

td_void aip_hal_set_rx_start(ot_audio_dev ai_dev_id, td_bool en)
{
    if (ai_dev_id >= OT_AI_DEV_MAX_NUM) {
        aio_err_trace("ai_dev %d is invalid!\n", ai_dev_id);
        return;
    }

    u_rx_dsp_ctrl un_tmp;
    un_tmp.u32            = aiao_hal_read_reg(aip_ctrl_reg(ai_dev_id));
    un_tmp.bits.rx_enable = (td_u32)en;

    aiao_hal_write_reg(aip_ctrl_reg(ai_dev_id), un_tmp.u32);
}

td_u32 aip_hal_get_dis_done(ot_audio_dev ai_dev_id)
{
    if (ai_dev_id >= OT_AI_DEV_MAX_NUM) {
        aio_err_trace("ai_dev %d is invalid!\n", ai_dev_id);
        return 0U;
    }

    u_rx_dsp_ctrl un_tmp;
    un_tmp.u32 = aiao_hal_read_reg(aip_ctrl_reg(ai_dev_id));

    td_u32 status;
    status = un_tmp.bits.rx_disable_done;

    return status;
}

td_s32 aip_hal_get_int_status(ot_audio_dev ai_dev_id, td_u32 * const int_status)
{
    ai_check_null_ptr_return(int_status);
#ifndef CONFIG_AUDIO_V200_SUPPORT
    *int_status = aip_hal_get_child_int_all_status(ai_dev_id);
    g_au32_ai_int_status[ai_dev_id] = *int_status;
#endif
    return TD_SUCCESS;
}

static td_s32 aip_set_mask(ot_audio_dev ai_dev_id, const ot_aio_attr *attr)
{
    return aip_set_mask_private(ai_dev_id, attr, aiao_hal_get_i2s_mux_reg(AIO_I2S_MUX_TYPE));
}

static td_s32 check_ai_attr_basic(const ot_aio_attr *attr)
{
    /* 主模式下检查采样率 */
    if (aio_is_master_mode(attr->work_mode) && (check_attr_sample_rate(attr->sample_rate) == TD_FALSE)) {
        aio_err_trace("invalid sample_rate\n");
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    if ((attr->chn_cnt != AIO_CHN_CNT_1) && (attr->chn_cnt != AIO_CHN_CNT_2) &&
        (attr->chn_cnt != AIO_CHN_CNT_4) && (attr->chn_cnt != AIO_CHN_CNT_8) &&
        (attr->chn_cnt != AIO_CHN_CNT_16) && (attr->chn_cnt != AIO_CHN_CNT_20)) {
        aio_err_trace("invalid chn_cnt:%u\n", attr->chn_cnt);
        return OT_ERR_AI_INVALID_CHN_ID;
    }

    if (attr->chn_cnt > OT_AI_MAX_CHN_NUM) {
        aio_err_trace("invalid chn_cnt:%u\n", attr->chn_cnt);
        return OT_ERR_AI_INVALID_CHN_ID;
    }

    if ((attr->chn_cnt == 1) && (attr->snd_mode == OT_AUDIO_SOUND_MODE_STEREO)) {
        aio_err_trace("when chn_cnt=1, can't use STEREO mode\n");
        return OT_ERR_AI_INVALID_CHN_ID;
    }

    /* 只支持16bit/24bit */
    if ((attr->bit_width != OT_AUDIO_BIT_WIDTH_16) && (attr->bit_width != OT_AUDIO_BIT_WIDTH_24)) {
        aio_err_trace("invalid bit_width:%d\n", attr->bit_width);
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    /* 内置codec最大支持2通道输入 */
    if ((attr->i2s_type == OT_AIO_I2STYPE_INNERCODEC) && (attr->chn_cnt > AIO_CHN_CNT_2)) {
        aio_err_trace("invalid chn_cnt:%u for inner codec\n", attr->chn_cnt);
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    if ((attr->i2s_type == OT_AIO_I2STYPE_INNERCODEC) && (attr->work_mode != OT_AIO_MODE_I2S_MASTER)) {
        aio_err_trace("inner codec only support i2s master mode\n");
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

static td_s32 check_ai_attr_other(const ot_aio_attr *attr)
{
    td_u32 chn_cnt;
    td_u32 bit_cnt;
    chn_cnt = attr->chn_cnt;
    bit_cnt = aio_get_bit_cnt(attr->bit_width);

    /* I2S模式时，音频输出最大支持左右声道共256bit */
    if (aio_is_i2s_mode(attr->work_mode)) {
        if (chn_cnt * bit_cnt > AIO_FIFO_BITWIDTH_MAX) {
            aio_err_trace("I2S mode, not support chn_cnt = %u, bit_cnt = %u!\n", chn_cnt, bit_cnt);
            return OT_ERR_AI_ILLEGAL_PARAM;
        }
    } else if (aio_is_pcm_mode(attr->work_mode)) {
        /* PCM模式时，只支持单声道的总共256bit */
        if (chn_cnt * bit_cnt > AIO_FIFO_BITWIDTH_MAX) {
            aio_err_trace("PCM mode, not support chn_cnt = %u, bit_cnt = %u!\n", chn_cnt, bit_cnt);
            return OT_ERR_AI_ILLEGAL_PARAM;
        }
    }

    if (attr->work_mode >= OT_AIO_MODE_BUTT) {
        aio_err_trace("invalid work_mode:%d\n", attr->work_mode);
        return OT_ERR_AI_ILLEGAL_PARAM;
    }
    if (attr->snd_mode >= OT_AUDIO_SOUND_MODE_BUTT) {
        aio_err_trace("invalid snd_mode:%d\n", attr->snd_mode);
        return OT_ERR_AI_ILLEGAL_PARAM;
    }
    if ((attr->frame_num > OT_MAX_AUDIO_FRAME_NUM) || (attr->frame_num < 2)) { /* 2: 最小frame */
        aio_err_trace("invalid frame_num:%u\n", attr->frame_num);
        return OT_ERR_AI_ILLEGAL_PARAM;
    }
    if ((attr->point_num_per_frame > OT_MAX_AI_POINT_NUM) || (attr->point_num_per_frame < OT_MIN_AUDIO_POINT_NUM)) {
        aio_err_trace("invalid point_num_per_frame:%u\n", attr->point_num_per_frame);
        return OT_ERR_AI_ILLEGAL_PARAM;
    }
    if ((attr->expand_flag != 0U) && (attr->expand_flag != OT_AI_EXPAND) && (attr->expand_flag != OT_AI_CUT)) {
        aio_err_trace("invalid expand_flag! expand_flag = %u\n", attr->expand_flag);
        return OT_ERR_AI_ILLEGAL_PARAM;
    }
    if ((attr->clk_share != AIO_CLK_SEL_SEPARATE) && (attr->clk_share != AIO_CLK_SEL_INSEPARATE)) {
        aio_err_trace("invalid clk_share:%u\n", attr->clk_share);
        return OT_ERR_AI_ILLEGAL_PARAM;
    }
    if ((attr->i2s_type != OT_AIO_I2STYPE_INNERCODEC) && (attr->i2s_type != OT_AIO_I2STYPE_INNERHDMI) &&
        (attr->i2s_type != OT_AIO_I2STYPE_EXTERN)) {
        aio_err_trace("invalid i2s_type:%d\n", attr->i2s_type);
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

static td_s32 check_ai_attr(ot_audio_dev ai_dev_id, const ot_aio_attr *attr)
{
    td_s32 ret;

    /* 检测attr基础参数 */
    ret = check_ai_attr_basic(attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    /* 检测attr其它参数 */
    ret = check_ai_attr_other(attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }
#ifndef CONFIG_AUDIO_V200_SUPPORT
    /* 检测芯片的私有参数 */
    ret = check_ai_attr_private(ai_dev_id, attr);
#endif
    return ret;
}

static td_s32 aip_set_attr_reg_rx_mode(const ot_aio_attr *attr, u_rx_if_attri *aip_attr_reg)
{
    /* 设置工作模式 */
    if ((attr->work_mode == OT_AIO_MODE_I2S_SLAVE) || (attr->work_mode == OT_AIO_MODE_I2S_MASTER)) {
        aip_attr_reg->bits.rx_mode = 0U; /* I2S mode */
        aip_attr_reg->bits.rx_sd_offset = 0U;
    } else if ((attr->work_mode == OT_AIO_MODE_PCM_SLAVE_STD) || (attr->work_mode == OT_AIO_MODE_PCM_MASTER_STD)) {
        aip_attr_reg->bits.rx_mode = 1U; /* PCM standard mode */
        aip_attr_reg->bits.rx_sd_offset = 1U;
    } else if ((attr->work_mode == OT_AIO_MODE_PCM_SLAVE_NON_STD) ||
               (attr->work_mode == OT_AIO_MODE_PCM_MASTER_NON_STD)) {
        aip_attr_reg->bits.rx_mode = 1U; /* PCM non standard mode */
        aip_attr_reg->bits.rx_sd_offset = 0U;
    } else {
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

static td_s32 aip_set_attr_reg_rx_chn_num(const ot_aio_attr *attr, u_rx_if_attri *aip_attr_reg)
{
    /* 接收通道数量设置 */
    if (attr->chn_cnt == AIO_CHN_CNT_1) {
        /* 时分复用无效 */
        aip_attr_reg->bits.rx_multislot_en = 0U;
        aip_attr_reg->bits.rx_ch_num = 0U;
    } else {
        /*
         * 时分复用有效:rx_ch_num： 0->2路,1->4路，2->8路，3->路
         * 时分复用无效:rx_ch_num： 0->1路,1->2路，2->-路，3->8路
         * 设置通道数目
         */
        if (attr->chn_cnt == AIO_CHN_CNT_2) {
            if (aio_is_i2s_mode(attr->work_mode)) {
                /* track mode 在时分复用无效时才能使用 */
                aip_attr_reg->bits.rx_multislot_en = 0U;
                aip_attr_reg->bits.rx_ch_num       = 1U;
            } else {
                aip_attr_reg->bits.rx_multislot_en = 1U;
                aip_attr_reg->bits.rx_ch_num       = 0U;
            }
        } else if (attr->chn_cnt == AIO_CHN_CNT_4) {
            /* 时分复用有效 */
            aip_attr_reg->bits.rx_multislot_en = 1U;
            aip_attr_reg->bits.rx_ch_num       = 1U;
        } else if (attr->chn_cnt == AIO_CHN_CNT_8) {
            /* 时分复用有效 */
            aip_attr_reg->bits.rx_multislot_en = 1U;
            aip_attr_reg->bits.rx_ch_num       = 2U;  /* 2: 时分复用8chn */
        } else if (attr->chn_cnt == AIO_CHN_CNT_16) {
            /* 时分复用有效 */
            aip_attr_reg->bits.rx_multislot_en = 1U;
            aip_attr_reg->bits.rx_ch_num       = 3U;  /* 3: 时分复用16chn */
        } else if (attr->chn_cnt == AIO_CHN_CNT_20) {
            /* 时分复用有效 */
            aip_attr_reg->bits.rx_multislot_en = 1U;
            aip_attr_reg->bits.rx_ch_num       = 4U;  /* 4: 时分复用20chn */
        } else {
            return OT_ERR_AI_INVALID_CHN_ID;
        }
    }

    return TD_SUCCESS;
}

static td_s32 aip_set_attr_reg(ot_audio_dev ai_dev_id, const ot_aio_attr *attr)
{
    u_rx_if_attri aip_attr_reg;
    aip_attr_reg.u32 = aiao_hal_read_reg(aip_inf_attri_reg(ai_dev_id));
    aip_attr_reg.bits.reserved_0 = 0U;

    /* 设置工作模式 */
    if (aip_set_attr_reg_rx_mode(attr, &aip_attr_reg) != TD_SUCCESS) {
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    /* 接收通道数量设置 */
    if (aip_set_attr_reg_rx_chn_num(attr, &aip_attr_reg) != TD_SUCCESS) {
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    /* 采样精度设置 */
    if (attr->bit_width == OT_AUDIO_BIT_WIDTH_8) {
        aip_attr_reg.bits.rx_i2s_precision = 0U;
    } else if (attr->bit_width == OT_AUDIO_BIT_WIDTH_16) {
        aip_attr_reg.bits.rx_i2s_precision = 1U;
    } else if (attr->bit_width == OT_AUDIO_BIT_WIDTH_24) {
        aip_attr_reg.bits.rx_i2s_precision = 2U; /* 2: 24bit */
    } else {
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    /* 数据源选择 */
    switch (ai_dev_id) {
        case AIO_DEV_ID_0:
            aip_attr_reg.bits.rx_sd_source_sel = 0x8U; /* 选择RX0 */
            break;
        case AIO_DEV_ID_1:
            aip_attr_reg.bits.rx_sd_source_sel = 0x9U; /* 选择RX1 */
            break;
        case AIO_DEV_ID_2:
            aip_attr_reg.bits.rx_sd_source_sel = 0xaU; /* 选择RX2 */
            break;
        default:
            return OT_ERR_AI_INVALID_DEV_ID;
    }

    /* sd0-sd3 输入数据线 */
    aip_attr_reg.bits.rx_sd0_sel   = 0U;
    aip_attr_reg.bits.rx_sd1_sel   = 1U;
    aip_attr_reg.bits.rx_sd2_sel   = 2U;  /* 2: sd2 */
    aip_attr_reg.bits.rx_sd3_sel   = 3U;  /* 3: sd3 */
    aip_attr_reg.bits.rx_trackmode = 0U;  /* I2S模式下，左右声道模式控制,默认不处理 */

    aiao_hal_write_reg(aip_inf_attri_reg(ai_dev_id), aip_attr_reg.u32);

    return TD_SUCCESS;
}

static td_void aip_set_ctrl_reg(ot_audio_dev ai_dev_id)
{
    u_rx_dsp_ctrl aip_ctrl_reg;

    aip_ctrl_reg.u32 = aiao_hal_read_reg(aip_ctrl_reg(ai_dev_id));

    /* 接收通道控制寄存器 */
    aip_ctrl_reg.bits.mute_en         = 0U;
    aip_ctrl_reg.bits.mute_fade_en    = 0U;
    aip_ctrl_reg.bits.pause_en        = 0U;
    aip_ctrl_reg.bits.pause_fade_en   = 0U;
    aip_ctrl_reg.bits.reserved_3      = 0U;
    aip_ctrl_reg.bits.volume          = 0U;
    aip_ctrl_reg.bits.reserved_2      = 0U;
    aip_ctrl_reg.bits.fade_in_rate    = 0U;
    aip_ctrl_reg.bits.fade_out_rate   = 0U;
    aip_ctrl_reg.bits.reserved_1      = 0U;
    aip_ctrl_reg.bits.bypass_en       = 0U;
    aip_ctrl_reg.bits.rx_enable       = 0U;
    aip_ctrl_reg.bits.rx_disable_done = 0U;
    aip_ctrl_reg.bits.reserved_0      = 0U;

    aiao_hal_write_reg(aip_ctrl_reg(ai_dev_id), aip_ctrl_reg.u32);
}

/* 设置AIP属性 */
td_s32 aip_hal_set_dev_attr(ot_audio_dev ai_dev_id, const ot_aio_attr *attr)
{
    ai_check_null_ptr_return(attr);

    if (ai_dev_id >= OT_AI_DEV_MAX_NUM) {
        aio_err_trace("ai_dev %d is invalid!\n", ai_dev_id);
        return OT_ERR_AI_INVALID_DEV_ID;
    }

    /* check AI attr */
    td_s32 ret;
    ret = check_ai_attr(ai_dev_id, attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    if (osal_down_interruptible(&g_sem_for_aio_ctx)) {
        return -ERESTARTSYS;
    }

    void *i2s_base = g_reg_i2s_base;
    if (ai_dev_id >= I2S_ANA) {
        i2s_base = g_reg_aix2tdm_i2s_base;
    }
    audio_i2s_init(ai_dev_id, i2s_base);

    ret = memcpy_s(&g_aio_state[ai_dev_id].ai_attr, sizeof(g_aio_state[ai_dev_id].ai_attr), attr, sizeof(*attr));
    if (ret != EOK) {
        ai_err_trace("ai_dev %d attr memcpy_s fail, ret = 0x%x.\n", ai_dev_id, ret);
        osal_up(&g_sem_for_aio_ctx);
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    g_aio_state[ai_dev_id].ai_config = TD_TRUE;
    osal_up(&g_sem_for_aio_ctx);

    return TD_SUCCESS;
}

td_s32 aip_hal_clr_ai_attr(ot_audio_dev ai_dev_id)
{
    if (ai_dev_id >= OT_AI_DEV_MAX_NUM) {
        aio_err_trace("ai_dev %d is invalid!\n", ai_dev_id);
        return OT_ERR_AI_INVALID_DEV_ID;
    }

    if (osal_down_interruptible(&g_sem_for_aio_ctx)) {
        return -ERESTARTSYS;
    }

    g_aio_state[ai_dev_id].ai_config = TD_FALSE;

    osal_up(&g_sem_for_aio_ctx);

    return TD_SUCCESS;
}

/* 使能中断，清除原始中断，启动设备 */
td_s32 aip_hal_dev_enable(ot_audio_dev ai_dev_id)
{
    void *i2s_base = g_reg_i2s_base;
    if (ai_dev_id >= I2S_ANA) {
        i2s_base = g_reg_aix2tdm_i2s_base;
    }
    audio_ai_i2s_trigger(ai_dev_id, i2s_base);

    td_s32 ret;
    ret = asp_dmac_trigger(ai_dev_id, TD_FALSE);
    if (ret != 0) {
        aio_err_trace("asp dmac trigger fail in aip hal ret = %d, ai_dev_id = %d\n", ret, ai_dev_id);
        return ret;
    }

    g_ai_en_status[ai_dev_id] = 1U;
#ifndef CONFIG_AUDIO_V200_SUPPORT
    /* 启动DMA及设备 */
    aip_hal_set_rx_start(ai_dev_id, TD_TRUE);
#endif

    return TD_SUCCESS;
}

td_void aip_hal_dev_disable(ot_audio_dev ai_dev_id)
{
    void *i2s_base = g_reg_i2s_base;
    if (ai_dev_id >= I2S_ANA) {
        i2s_base = g_reg_aix2tdm_i2s_base;
    }

    td_s32 ret;
    ret = asp_dmac_stop(ai_dev_id, TD_FALSE);
    if (ret != 0) {
        aio_err_trace("asp dmac stop fail ai_dev_id = %d, ret = %d!\n", ai_dev_id, ret);
    }

    audio_ai_i2s_stop(ai_dev_id, i2s_base);

    g_ai_en_status[ai_dev_id] = 0U;
}

td_void aop_hal_int_en(ot_audio_dev ao_dev_id, td_bool en)
{
    if (ao_dev_id >= OT_AO_DEV_MAX_NUM) {
        aio_err_trace("ao_dev %d is invalid!\n", ao_dev_id);
        return;
    }

    u_aiao_int_ena un_tmp;
    un_tmp.u32 = aiao_hal_read_reg(AIAO_INT_ENA_REG);

    switch (ao_dev_id) {
        case AIO_DEV_ID_0:
            un_tmp.bits.tx_ch0_int_ena = (td_u32)en;
            break;
        case AIO_DEV_ID_1:
            un_tmp.bits.tx_ch1_int_ena = (td_u32)en;
            break;
        case AIO_DEV_ID_2:
            un_tmp.bits.tx_ch2_int_ena = (td_u32)en;
            break;
        default:
            break;
    }

    aiao_hal_write_reg(AIAO_INT_ENA_REG, un_tmp.u32);
}

td_u32 aop_hal_get_buff_wptr(ot_audio_dev ao_dev_id)
{
    if (ao_dev_id >= OT_AO_DEV_MAX_NUM) {
        aio_err_trace("ao_dev %d is invalid!\n", ao_dev_id);
        return 0U;
    }

    td_u32 status;
    status = g_aop_buff_wptr[ao_dev_id];
    return status;
}

td_u32 aop_hal_get_buff_rptr(ot_audio_dev ao_dev_id)
{
#ifndef CONFIG_AUDIO_V200_SUPPORT
    if (ao_dev_id >= OT_AO_DEV_MAX_NUM) {
        aio_err_trace("ao_dev %d is invalid!\n", ao_dev_id);
        return 0U;
    }

    u_tx_buff_rptr un_tmp;
    un_tmp.u32 = aiao_hal_read_reg(aop_buff_rptr_reg(ao_dev_id));

    td_u32 status;
    status = un_tmp.bits.tx_buff_rptr;
#endif
    // 正常模式读指针有硬件维护
    return 0U;
}

td_void aop_hal_set_buff_wptr(ot_audio_dev ao_dev_id, td_u32 value)
{
    if (ao_dev_id >= OT_AO_DEV_MAX_NUM) {
        aio_err_trace("ao_dev %d is invalid!\n", ao_dev_id);
        return;
    }
    g_aop_buff_wptr[ao_dev_id] = value;
}

td_void aop_hal_set_tx_start(ot_audio_dev ao_dev_id, td_bool en)
{
    if (ao_dev_id >= OT_AO_DEV_MAX_NUM) {
        aio_err_trace("ao_dev %d is invalid!\n", ao_dev_id);
        return;
    }

    u_tx_dsp_ctrl un_tmp;
    un_tmp.u32            = aiao_hal_read_reg(aop_ctrl_reg(ao_dev_id));
    un_tmp.bits.tx_enable = (td_u32)en;

    aiao_hal_write_reg(aop_ctrl_reg(ao_dev_id), un_tmp.u32);
}

td_u32 aop_hal_get_dis_done(ot_audio_dev ao_dev_id)
{
    if (ao_dev_id >= OT_AO_DEV_MAX_NUM) {
        aio_err_trace("ao_dev %d is invalid!\n", ao_dev_id);
        return 0U;
    }

    u_tx_dsp_ctrl un_tmp;
    un_tmp.u32 = aiao_hal_read_reg(aop_ctrl_reg(ao_dev_id));

    td_u32 status;
    status = un_tmp.bits.tx_disable_done;

    return status;
}

td_void aop_hal_set_child_int_mask(ot_audio_dev ao_dev_id)
{
    if (ao_dev_id >= OT_AO_DEV_MAX_NUM) {
        aio_err_trace("ao_dev %d is invalid!\n", ao_dev_id);
        return;
    }

    u_tx_int_ena un_tmp;
    /* 使能传输完成中断，buf空中断，fifo下溢中断 */
    un_tmp.u32                         = aiao_hal_read_reg(aop_int_ena_reg(ao_dev_id));
    un_tmp.bits.tx_trans_int_ena       = 1U;
    un_tmp.bits.tx_empty_int_ena       = 1U;
    un_tmp.bits.tx_alempty_int_ena     = 0U;
    un_tmp.bits.tx_bfifo_empty_int_ena = 1U;
    un_tmp.bits.tx_ififo_empty_int_ena = 1U;
    un_tmp.bits.tx_stop_int_ena        = 0U;
    un_tmp.bits.tx_mfade_int_ena       = 0U;
    un_tmp.bits.tx_dat_break_int_ena   = 0U;
    un_tmp.bits.reserved_0             = 0U;

    aiao_hal_write_reg(aop_int_ena_reg(ao_dev_id), un_tmp.u32);
}

td_void aop_hal_set_child_int_clr(ot_audio_dev ao_dev_id)
{
    if (ao_dev_id >= OT_AO_DEV_MAX_NUM) {
        aio_err_trace("ao_dev %d is invalid!\n", ao_dev_id);
        return;
    }

    u_tx_int_clr int_mask;
    int_mask.u32                           = aiao_hal_read_reg(aop_int_status_reg(ao_dev_id));
    int_mask.bits.tx_bfifo_empty_int_clear = 0U;
    int_mask.bits.tx_ififo_empty_int_clear = 0U;
    int_mask.bits.tx_empty_int_clear       = 0U;
    int_mask.bits.tx_trans_int_clear       = 0U;

    if (g_au32_ao_int_status[ao_dev_id] & 0x18U) { /* fifo over */
        int_mask.bits.tx_bfifo_empty_int_clear = 1U;
        int_mask.bits.tx_ififo_empty_int_clear = 1U;
    }

    if (g_au32_ao_int_status[ao_dev_id] & 0x02U) { /* buffer full */
        int_mask.bits.tx_empty_int_clear = 1U;
    }

    if (g_au32_ao_int_status[ao_dev_id] & 0x01U) { /* trans finish */
        int_mask.bits.tx_trans_int_clear = 1U;
    }

    aiao_hal_write_reg(aop_int_clr_reg(ao_dev_id), int_mask.u32);
}

td_u32 aop_hal_get_child_int_all_status(ot_audio_dev ao_dev_id)
{
    if (ao_dev_id >= OT_AO_DEV_MAX_NUM) {
        aio_err_trace("ao_dev %d is invalid!\n", ao_dev_id);
        return 0U;
    }

    return aiao_hal_read_reg(aop_int_status_reg(ao_dev_id));
}

td_void aop_hal_clr_child_int_all_status(ot_audio_dev ao_dev_id)
{
    if (ao_dev_id >= OT_AO_DEV_MAX_NUM) {
        aio_err_trace("ao_dev %d is invalid!\n", ao_dev_id);
        return;
    }

    u_tx_int_clr int_mask;
    /* 清除使能的所有中断 */
    int_mask.bits.tx_trans_int_clear       = 1U;
    int_mask.bits.tx_empty_int_clear       = 1U;
    int_mask.bits.tx_alempty_int_clear     = 1U;
    int_mask.bits.tx_bfifo_empty_int_clear = 1U;
    int_mask.bits.tx_ififo_empty_int_clear = 1U;
    int_mask.bits.tx_stop_int_clear        = 1U;
    int_mask.bits.tx_mfade_int_clear       = 1U;
    int_mask.bits.tx_dat_break_int_clear   = 1U;

    aiao_hal_write_reg(aop_int_clr_reg(ao_dev_id), int_mask.u32);
}

/* 清除中断状态位 */
td_s32 aop_hal_clr_int_status(ot_audio_dev ao_dev_id)
{
    if (ao_dev_id >= OT_AO_DEV_MAX_NUM) {
        aio_err_trace("ao_dev %d is invalid!\n", ao_dev_id);
        return OT_ERR_AO_INVALID_DEV_ID;
    }

    return TD_SUCCESS;
}

td_s32 aop_hal_set_volume(ot_audio_dev ao_dev_id, td_s32 volume_db)
{
    if (ao_dev_id >= OT_AO_DEV_MAX_NUM) {
        aio_err_trace("ao_dev %d is invalid!\n", ao_dev_id);
        return OT_ERR_AO_INVALID_DEV_ID;
    }

    /* 0x7F->6db 0x7E->5db 0x29->-80db 0~0x28静音, */
    if ((-121 > volume_db) || (volume_db > 6)) { /* -121,6: 音量边界值 */
        aio_err_trace("ao_dev %d volume %d error!\n", ao_dev_id, volume_db);
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

td_s32 aop_hal_get_volume(ot_audio_dev ao_dev_id, td_s32 * const volume_db)
{
    if (ao_dev_id >= OT_AO_DEV_MAX_NUM) {
        aio_err_trace("ao_dev %d is invalid!\n", ao_dev_id);
        return OT_ERR_AO_INVALID_DEV_ID;
    }

    ao_check_null_ptr_return(volume_db);

    return TD_SUCCESS;
}

td_s32 aop_hal_set_mute(ot_audio_dev ao_dev_id, td_bool mute, const ot_audio_fade *fade)
{
    if (ao_dev_id >= OT_AO_DEV_MAX_NUM) {
        aio_err_trace("ao_dev %d is invalid!\n", ao_dev_id);
        return OT_ERR_AO_INVALID_DEV_ID;
    }

    if ((mute != TD_TRUE) && (mute != TD_FALSE)) {
        aio_err_trace("ao_dev %d set mute param error!\n", ao_dev_id);
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

    if (fade != TD_NULL) {
        if ((fade->fade != TD_TRUE) && (fade->fade != TD_FALSE)) {
            aio_err_trace("ao_dev %d set mute param error!\n", ao_dev_id);
            return OT_ERR_AO_ILLEGAL_PARAM;
        }

        if ((fade->fade_in_rate >= OT_AUDIO_FADE_RATE_BUTT) || (fade->fade_out_rate >= OT_AUDIO_FADE_RATE_BUTT)) {
            aio_err_trace("ao_dev %d set mute param error!\n", ao_dev_id);
            return OT_ERR_AO_ILLEGAL_PARAM;
        }
    }

    return TD_SUCCESS;
}

td_s32 aop_hal_get_mute(ot_audio_dev ao_dev_id, td_bool * const mute, ot_audio_fade * const fade)
{
    if (ao_dev_id >= OT_AO_DEV_MAX_NUM) {
        aio_err_trace("ao_dev %d is invalid!\n", ao_dev_id);
        return OT_ERR_AO_INVALID_DEV_ID;
    }

    if (mute == TD_NULL) {
        aio_err_trace("ao_dev %d mute is null!\n", ao_dev_id);
        return OT_ERR_AO_NULL_PTR;
    }

    if (fade == TD_NULL) {
        aio_err_trace("ao_dev %d fade is null!\n", ao_dev_id);
        return OT_ERR_AO_NULL_PTR;
    }

    return TD_SUCCESS;
}

static td_s32 aop_set_switch_tx_bclk(ot_audio_dev ao_dev_id)
{
    u_aiao_switch_tx_bclk switch_tx_bclk;

    /* 内部bclk选择,时钟共用的情况 */
    switch_tx_bclk.u32 = aiao_hal_read_reg(AOP_SWITCH_TX_BCLK);
    switch (ao_dev_id) {
        case AIO_DEV_ID_0: {
            switch_tx_bclk.bits.inner_bclk_ws_sel_tx_00 = 0x8U;
            break;
        }
        case AIO_DEV_ID_1: {
            switch_tx_bclk.bits.inner_bclk_ws_sel_tx_01 = 0x9U;
            break;
        }
        case AIO_DEV_ID_2: {
            switch_tx_bclk.bits.inner_bclk_ws_sel_tx_02 = 0xaU;
            break;
        }
        default: {
            aio_err_trace("invalid ao dev = %d!\n", ao_dev_id);
            return OT_ERR_AO_INVALID_DEV_ID;
        }
    }
    aiao_hal_write_reg(AOP_SWITCH_TX_BCLK, switch_tx_bclk.u32);
    return TD_SUCCESS;
}

static td_s32 aop_set_mask(ot_audio_dev audio_dev_id, const ot_aio_attr *attr)
{
    return aop_set_mask_private(audio_dev_id, attr, aiao_hal_get_i2s_mux_reg(AIO_I2S_MUX_TYPE));
}

static td_s32 check_ao_attr_basic(const ot_aio_attr *attr)
{
    /* 主模式下检查采样率 */
    if (aio_is_master_mode(attr->work_mode) && check_attr_sample_rate(attr->sample_rate) == TD_FALSE) {
        aio_err_trace("invalid sample_rate\n");
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

    if ((attr->chn_cnt != AIO_CHN_CNT_1) && (attr->chn_cnt != AIO_CHN_CNT_2) && (attr->chn_cnt != AIO_CHN_CNT_8)) {
        aio_err_trace("invalid chn_cnt:%u\n", attr->chn_cnt);
        return OT_ERR_AO_INVALID_CHN_ID;
    }

    if (attr->chn_cnt > OT_AO_MAX_CHN_NUM) {
        aio_err_trace("invalid chn_cnt:%u\n", attr->chn_cnt);
        return OT_ERR_AO_INVALID_CHN_ID;
    }

    if ((attr->chn_cnt == AIO_CHN_CNT_1) && (attr->snd_mode == OT_AUDIO_SOUND_MODE_STEREO)) {
        aio_err_trace("when chn_cnt=1, can't use STEREO mode\n");
        return OT_ERR_AO_INVALID_CHN_ID;
    }

    /* 在pcm模式下，不支持通道数大于等于2的情况 */
    if ((attr->chn_cnt >= AIO_CHN_CNT_2) && aio_is_pcm_mode(attr->work_mode)) {
        aio_err_trace("not support chn_cnt >=2 in pcm mode\n");
        return OT_ERR_AO_INVALID_CHN_ID;
    }

    /* pcm模式不支持24bit位宽 */
    if ((attr->bit_width == OT_AUDIO_BIT_WIDTH_24) && aio_is_pcm_mode(attr->work_mode)) {
        aio_err_trace("invalid bit_width:%d in pcm mode\n", attr->bit_width);
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

    /* 内置codec最大支持2通道输出 */
    if ((attr->i2s_type == OT_AIO_I2STYPE_INNERCODEC) && (attr->chn_cnt > AIO_CHN_CNT_2)) {
        aio_err_trace("invalid chn_cnt:%u for inner codec\n", attr->chn_cnt);
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

    if ((attr->i2s_type == OT_AIO_I2STYPE_INNERCODEC) && (attr->work_mode != OT_AIO_MODE_I2S_MASTER)) {
        aio_err_trace("inner codec only support i2s master mode\n");
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

    /* 音频输出，I2S模式支持16/24/32bit，PCM模式支持8/16bit */
    if ((attr->bit_width != OT_AUDIO_BIT_WIDTH_16) && (attr->bit_width != OT_AUDIO_BIT_WIDTH_24)) {
        aio_err_trace("invalid bit_width:%d\n", attr->bit_width);
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

static td_s32 check_ao_attr_other(const ot_aio_attr *attr)
{
    td_u32 chn_cnt;
    td_u32 bit_cnt;
    chn_cnt = attr->chn_cnt;
    bit_cnt = aio_get_bit_cnt(attr->bit_width);

    if (aio_is_i2s_mode(attr->work_mode)) {
        /* I2S模式时，音频输出最大支持左右声道共256/384bit */
        if (chn_cnt * bit_cnt > AIO_FIFO_BITWIDTH_MAX) {
            aio_err_trace("I2S mode, not support chn_cnt = %u, bit_cnt = %u!\n", chn_cnt, bit_cnt);
            return OT_ERR_AO_ILLEGAL_PARAM;
        }
    } else if (aio_is_pcm_mode(attr->work_mode)) {
        /* PCM模式时，只支持单声道的总共256/384bit */
        if (chn_cnt * bit_cnt > AIO_FIFO_BITWIDTH_MAX) {
            aio_err_trace("PCM mode, not support chn_cnt = %u, bit_cnt = %u!\n", chn_cnt, bit_cnt);
            return OT_ERR_AO_ILLEGAL_PARAM;
        }
    }
    if (attr->work_mode >= OT_AIO_MODE_BUTT) {
        aio_err_trace("invalid work_mode\n");
        return OT_ERR_AO_ILLEGAL_PARAM;
    }
    if (attr->snd_mode >= OT_AUDIO_SOUND_MODE_BUTT) {
        aio_err_trace("invalid snd_mode\n");
        return OT_ERR_AO_ILLEGAL_PARAM;
    }
    if ((attr->frame_num > OT_MAX_AUDIO_FRAME_NUM) || (attr->frame_num < 2)) { /* 2: 最小frame */
        aio_err_trace("invalid frame_num\n");
        return OT_ERR_AO_ILLEGAL_PARAM;
    }
    if ((attr->point_num_per_frame > OT_MAX_AO_POINT_NUM) || (attr->point_num_per_frame < OT_MIN_AUDIO_POINT_NUM)) {
        aio_err_trace("invalid point_num_per_frame:%u\n", attr->point_num_per_frame);
        return OT_ERR_AO_ILLEGAL_PARAM;
    }
    if ((attr->expand_flag != 0U) && (attr->expand_flag != OT_AI_EXPAND) && (attr->expand_flag != OT_AI_CUT)) {
        aio_err_trace("invalid expand_flag\n");
        return OT_ERR_AO_ILLEGAL_PARAM;
    }
    if ((attr->clk_share != AIO_CLK_SEL_SEPARATE) && (attr->clk_share != AIO_CLK_SEL_INSEPARATE)) {
        aio_err_trace("invalid clk_share\n");
        return OT_ERR_AO_ILLEGAL_PARAM;
    }
    if ((attr->i2s_type != OT_AIO_I2STYPE_INNERCODEC) && (attr->i2s_type != OT_AIO_I2STYPE_INNERHDMI) &&
        (attr->i2s_type != OT_AIO_I2STYPE_EXTERN)) {
        aio_err_trace("invalid i2s_type:%d\n", attr->i2s_type);
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

td_s32 check_ao_attr(ot_audio_dev ao_dev_id, const ot_aio_attr *attr)
{
    td_s32 ret;

    /* 检测attr基础参数 */
    ret = check_ao_attr_basic(attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    /* 检测attr其它参数 */
    ret = check_ao_attr_other(attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    /* 检测芯片的私有参数 */
    ret = check_ao_attr_private(ao_dev_id, attr);

    return ret;
}

static td_s32 aop_set_attr_reg_tx_mode(const ot_aio_attr *attr, u_tx_if_attri *aop_attr_reg)
{
    /* 设置工作模式 */
    if ((attr->work_mode == OT_AIO_MODE_I2S_SLAVE) || (attr->work_mode == OT_AIO_MODE_I2S_MASTER)) {
        aop_attr_reg->bits.tx_mode      = 0U;  /* I2S mode */
        aop_attr_reg->bits.tx_sd_offset = 0U;
    } else if ((attr->work_mode == OT_AIO_MODE_PCM_SLAVE_STD) || (attr->work_mode == OT_AIO_MODE_PCM_MASTER_STD)) {
        aop_attr_reg->bits.tx_mode      = 1U;  /* PCM standard mode */
        aop_attr_reg->bits.tx_sd_offset = 1U;
    } else if ((attr->work_mode == OT_AIO_MODE_PCM_SLAVE_NON_STD) ||
               (attr->work_mode == OT_AIO_MODE_PCM_MASTER_NON_STD)) {
        aop_attr_reg->bits.tx_mode      = 1U;  /* PCM non standard mode */
        aop_attr_reg->bits.tx_sd_offset = 0U;
    } else {
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

static td_s32 aop_set_attr_reg_tx_i2s_precision(const ot_aio_attr *attr, u_tx_if_attri *aop_attr_reg)
{
    /* 采样精度设置 */
    if (attr->bit_width == OT_AUDIO_BIT_WIDTH_8) {
        aop_attr_reg->bits.tx_i2s_precision = 0U;
    } else if (attr->bit_width == OT_AUDIO_BIT_WIDTH_16) {
        aop_attr_reg->bits.tx_i2s_precision = 1U;
    } else if (attr->bit_width == OT_AUDIO_BIT_WIDTH_24) {
        aop_attr_reg->bits.tx_i2s_precision = 2U; /* 2: 24bit */
    } else {
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

static td_s32 aop_set_attr_reg(ot_audio_dev ao_dev_id, const ot_aio_attr *attr)
{
    td_s32 ret;
    u_tx_if_attri aop_attr_reg;

    aop_attr_reg.u32                    = aiao_hal_read_reg(aop_inf_attri_reg(ao_dev_id));
    aop_attr_reg.bits.tx_underflow_ctrl = 1U;  /* 没有数据时默认发送最后一个采样点 */
    aop_attr_reg.bits.reserved_0        = 0U;

    /* 设置工作模式 */
    if (aop_set_attr_reg_tx_mode(attr, &aop_attr_reg) != TD_SUCCESS) {
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

    /* 设置通道数目, 发送通道时分复用无效 */
    aop_attr_reg.bits.tx_multislot_en = 0U;
    if (attr->chn_cnt == AIO_CHN_CNT_1) {
        aop_attr_reg.bits.tx_ch_num = 0U;
    } else if (attr->chn_cnt == AIO_CHN_CNT_2) {
        aop_attr_reg.bits.tx_ch_num = 1U;
    } else if (attr->chn_cnt == AIO_CHN_CNT_8) {
        if (AOP_SUPPORT_MULTI_SLOT == TD_TRUE) {
            /* 时分复用有效 */
            aop_attr_reg.bits.tx_multislot_en = 1U;
        }
        aop_attr_reg.bits.tx_ch_num = 3U; /* 3: Normal模式多根sd线的8chn，TDM模式8chn */
    } else {
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

    /* 采样精度设置 */
    ret = aop_set_attr_reg_tx_i2s_precision(attr, &aop_attr_reg);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    /* 数据源选择 */
    switch (ao_dev_id) {
        case AIO_DEV_ID_0:
            aop_attr_reg.bits.tx_sd_source_sel = 0U; /* 选择发送通道 TX0 */
            break;
        case AIO_DEV_ID_1:
            aop_attr_reg.bits.tx_sd_source_sel = 1U; /* 选择发送通道 TX1 */
            break;
        case AIO_DEV_ID_2:
            aop_attr_reg.bits.tx_sd_source_sel = 2U; /* 选择发送通道 TX2 */
            break;
        default:
            return OT_ERR_AO_INVALID_DEV_ID;
    }

    /* sd0-sd3 输入数据线 */
    aop_attr_reg.bits.tx_sd0_sel = 0U;
    aop_attr_reg.bits.tx_sd1_sel = 1U;
    aop_attr_reg.bits.tx_sd2_sel = 2U; /* 2: sd2 */
    aop_attr_reg.bits.tx_sd3_sel = 3U; /* 3: sd3 */

    aop_attr_reg.bits.tx_trackmode = 0U; /* I2S模式下，左右声道模式控制,默认不处理 */

    aiao_hal_write_reg(aop_inf_attri_reg(ao_dev_id), aop_attr_reg.u32);

    return TD_SUCCESS;
}

static td_void aop_set_ctrl_reg(ot_audio_dev ao_dev_id)
{
    u_tx_dsp_ctrl aop_ctrl_reg;
    aop_ctrl_reg.u32 = aiao_hal_read_reg(aop_ctrl_reg(ao_dev_id));

    /* 发送控制寄存器 */
    aop_ctrl_reg.bits.mute_en       = 0U;
    aop_ctrl_reg.bits.mute_fade_en  = 0U;
    aop_ctrl_reg.bits.reserved_3    = 0U;
    aop_ctrl_reg.bits.volume        = 0x79U;  /* 0db */
    aop_ctrl_reg.bits.reserved_2    = 0U;
    aop_ctrl_reg.bits.fade_in_rate  = 0U;
    aop_ctrl_reg.bits.fade_out_rate = 0U;
    aop_ctrl_reg.bits.reserved_1    = 0U;
    aop_ctrl_reg.bits.bypass_en     = 0U;
    aop_ctrl_reg.bits.tx_enable     = 0U;
    aop_ctrl_reg.bits.reserved_0    = 0U;

    aiao_hal_write_reg(aop_ctrl_reg(ao_dev_id), aop_ctrl_reg.u32);
}

td_s32 aop_hal_set_dev_attr(ot_audio_dev ao_dev_id, const ot_aio_attr *attr)
{
    if (ao_dev_id >= OT_AO_DEV_MAX_NUM) {
        aio_err_trace("ao_dev %d is invalid!\n", ao_dev_id);
        return OT_ERR_AO_INVALID_DEV_ID;
    }
    ao_check_null_ptr_return(attr);

    td_s32 ret;

    /* check AO attr */
    ret = check_ao_attr(ao_dev_id, attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    if (osal_down_interruptible(&g_sem_for_aio_ctx)) {
        return -ERESTARTSYS;
    }

    void *i2s_base = g_reg_i2s_base;
    if (ao_dev_id >= I2S_ANA) {
        i2s_base = g_reg_aix2tdm_i2s_base;
    }
    audio_i2s_init(ao_dev_id, i2s_base);

    ret = memcpy_s(&g_aio_state[ao_dev_id].ao_attr, sizeof(g_aio_state[ao_dev_id].ao_attr), attr, sizeof(*attr));
    if (ret != EOK) {
        osal_up(&g_sem_for_aio_ctx);
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

    g_aio_state[ao_dev_id].ao_config = TD_TRUE;

    osal_up(&g_sem_for_aio_ctx);

    return TD_SUCCESS;
}

td_s32 aop_hal_clr_ao_attr(ot_audio_dev ao_dev_id)
{
    if (ao_dev_id >= OT_AO_DEV_MAX_NUM) {
        aio_err_trace("ao_dev %d is invalid!\n", ao_dev_id);
        return OT_ERR_AO_INVALID_DEV_ID;
    }

    if (osal_down_interruptible(&g_sem_for_aio_ctx)) {
        return -ERESTARTSYS;
    }

    g_aio_state[ao_dev_id].ao_config = TD_FALSE;

    osal_up(&g_sem_for_aio_ctx);

    return TD_SUCCESS;
}

/* 使能中断，清除原始中断，启动设备 */
td_s32 aop_hal_dev_enable(ot_audio_dev ao_dev_id)
{
    void *i2s_base = g_reg_i2s_base;
    if (ao_dev_id >= I2S_ANA) {
        i2s_base = g_reg_aix2tdm_i2s_base;
    }
    audio_ao_i2s_trigger(ao_dev_id, i2s_base);
    // dma trigger
    td_s32 ret;
    ret = asp_dmac_trigger(ao_dev_id, TD_TRUE);
    if (ret != 0) {
        aio_err_trace("asp dmac trigger fail\n");
        return ret;
    }

    return TD_SUCCESS;
}

td_void aop_hal_dev_disable(ot_audio_dev ao_dev_id)
{
    void *i2s_base = g_reg_i2s_base;
    if (ao_dev_id >= I2S_ANA) {
        i2s_base = g_reg_aix2tdm_i2s_base;
    }

    // dma stop
    td_s32 ret;
    ret = asp_dmac_stop(ao_dev_id, TD_TRUE);
    if (ret != 0) {
        aio_err_trace("asp dmac stop fail\n");
    }
    audio_ao_i2s_stop(ao_dev_id, i2s_base);
    g_ao_en_status[ao_dev_id] = 0U;
}

td_s32 aop_hal_get_int_status(ot_audio_dev ao_dev_id, td_u32 * const int_status)
{
    ao_check_null_ptr_return(int_status);
#ifndef CONFIG_AUDIO_V200_SUPPORT
    *int_status = aop_hal_get_child_int_all_status(ao_dev_id);
    g_au32_ao_int_status[ao_dev_id] = *int_status;
#endif
    return TD_SUCCESS;
}

static td_s32 aop_hal_get_tx_multislot(td_bool *multislot_enable)
{
    if (multislot_enable == TD_NULL) {
        aio_err_trace("enable is null!\n");
        return OT_ERR_AIO_NULL_PTR;
    }

    *multislot_enable = AOP_SUPPORT_MULTI_SLOT;
    return TD_SUCCESS;
}

td_s32 aiao_init(void *p)
{
    ot_unused(p);

    if (g_aio_cur_state == AIO_STATE_STARTED) {
        aio_info_trace("initialize again!\n");
        return TD_SUCCESS;
    }
    if (g_aio_cur_state == AIO_STATE_STOPING) {
        aio_err_trace("aio is busy now!\n");
        return OT_ERR_AIO_BUSY;
    }

    td_s32 ret;
    ret = aiao_hal_sys_init();
    if (ret != TD_SUCCESS) {
        aio_err_trace("aiao_hal_sys_init fail!\n");
        return OT_ERR_AIO_NOT_READY;
    }

    ot_audio_dev audio_dev_id;
    for (audio_dev_id = 0; audio_dev_id < OT_AIO_MAX_NUM; audio_dev_id++) {
        g_aio_state[audio_dev_id].ai_config   = TD_FALSE;
        g_aio_state[audio_dev_id].ao_config   = TD_FALSE;
        g_aio_state[audio_dev_id].initialized = TD_TRUE;
    }

    g_aio_cur_state = AIO_STATE_STARTED;

    return TD_SUCCESS;
}

td_void aiao_exit(void)
{
    if (g_aio_cur_state == AIO_STATE_STOPED) {
        return;
    }

    aiao_hal_sys_deinit();

    ot_audio_dev audio_dev_id;
    for (audio_dev_id = 0; audio_dev_id < OT_AIO_MAX_NUM; audio_dev_id++) {
        g_aio_state[audio_dev_id].ai_config   = TD_FALSE;
        g_aio_state[audio_dev_id].ao_config   = TD_FALSE;
        g_aio_state[audio_dev_id].initialized = TD_FALSE;
    }

    g_aio_cur_state = AIO_STATE_STOPED;
}

static td_void aiao_notify(mod_notice_id notice)
{
    ot_unused(notice);
    g_aio_cur_state = AIO_STATE_STOPING;

    return;
}

static td_void aiao_query_state(mod_state *state)
{
    if (osal_atomic_read(&g_aio_user_ref) == 0) {
        *state = MOD_STATE_FREE;
    } else {
        *state = MOD_STATE_BUSY;
    }

    return;
}

static td_u32 aiao_get_ver_magic(td_void)
{
    return (td_u32)VERSION_MAGIC;
}

td_void aip_hal_set_ai_isr(fn_aio_callback pf_callback)
{
    g_aio_ctx.pfn_ai_isr_callback = pf_callback;
}

td_void aop_hal_set_ao_isr(fn_aio_callback pf_callback)
{
    g_aio_ctx.pfn_ao_isr_callback = pf_callback;
}

td_s32 aip_hal_set_acodec_gain(td_s32 gain)
{
    return aip_hal_set_acodec_gain_private(gain, (td_u8 *)g_reg_acodec_base);
}

td_s32 aip_hal_get_acodec_gain(td_s32 *gain)
{
    return aip_hal_get_acodec_gain_private(gain, (td_u8 *)g_reg_acodec_base);
}

/* 设置软件差分方案使能状态 */
td_s32 aip_hal_set_acodec_soft_diff(td_bool enable)
{
    if (enable != TD_TRUE && enable != TD_FALSE) {
        aio_err_trace("wrong input enable(%d), not among [0, 1]!\n", enable);
        return OT_ERR_AIO_ILLEGAL_PARAM;
    }
    g_soft_differential = enable;
    return TD_SUCCESS;
}

/* 获取软件差分方案使能状态 */
td_s32 aip_hal_get_acodec_soft_diff(td_bool *enable)
{
    if (enable == TD_NULL) {
        aio_err_trace("enable is null!\n");
        return OT_ERR_AIO_NULL_PTR;
    }
    *enable = g_soft_differential;
    return TD_SUCCESS;
}

static aio_export_func g_export_funcs = {
    .pfn_set_ai_attr = aip_hal_set_dev_attr,
    .pfn_enable_ai_dev = aip_hal_dev_enable,
    .pfn_disable_ai_dev = aip_hal_dev_disable,
    .pfn_set_ai_dev_buf_rptr = aip_hal_set_buff_rptr,
    .pfn_get_ai_dev_buf_rptr = aip_hal_get_buff_rptr,
    .pfn_set_ai_dev_buf_wptr = TD_NULL,
    .pfn_get_ai_dev_buf_wptr = TD_NULL,
    .pfn_set_ai_dev_buf_addr = TD_NULL,
    .pfn_set_ai_dev_buf_size = TD_NULL,
    .pfn_set_ai_dev_trans_size = TD_NULL,
    .pfn_get_ai_int_status = aip_hal_get_int_status,
    .pfn_clr_ai_int_status = aip_hal_clr_int_status,
    .pfn_set_ai_isr = aip_hal_set_ai_isr,
    .pfn_clr_ai_attr = aip_hal_clr_ai_attr,
    .pfn_set_ai_track_mode = TD_NULL,
    .pfn_get_ai_track_mode = TD_NULL,
    .pfn_set_ai_clk_dir = TD_NULL,
    .pfn_get_ai_clk_dir = TD_NULL,

    .pfn_set_acodec_gain = aip_hal_set_acodec_gain,
    .pfn_get_acodec_gain = aip_hal_get_acodec_gain,
    .pfn_set_acodec_soft_diff = aip_hal_set_acodec_soft_diff,
    .pfn_get_acodec_soft_diff = aip_hal_get_acodec_soft_diff,

    .pfn_set_ao_attr = aop_hal_set_dev_attr,
    .pfn_enable_ao_dev = aop_hal_dev_enable,
    .pfn_disable_ao_dev = aop_hal_dev_disable,
    .pfn_set_ao_dev_buf_wptr = aop_hal_set_buff_wptr,
    .pfn_get_ao_dev_buf_wptr = aop_hal_get_buff_wptr,
    .pfn_set_ao_dev_buf_rptr = TD_NULL,
    .pfn_get_ao_dev_buf_rptr = aop_hal_get_buff_rptr, // 播放读指针由硬件维护，录音写指针由硬件维护
    .pfn_set_ao_dev_buf_addr = TD_NULL,
    .pfn_set_ao_dev_buf_size = TD_NULL,
    .pfn_set_ao_dev_trans_size = TD_NULL,
    .pfn_get_ao_int_status = aop_hal_get_int_status,
    .pfn_clr_ao_int_status = aop_hal_clr_int_status,
    .pfn_set_ao_isr = aop_hal_set_ao_isr,
    .pfn_clr_ao_attr = aop_hal_clr_ao_attr,
    .pfn_set_ao_track_mode = TD_NULL,
    .pfn_get_ao_track_mode = TD_NULL,
    .pfn_set_ao_clk_dir = TD_NULL,
    .pfn_get_ao_clk_dir = TD_NULL,
    .pfn_set_ao_mute = aop_hal_set_mute,
    .pfn_get_ao_mute = aop_hal_get_mute,
    .pfn_set_ao_volume = aop_hal_set_volume,
    .pfn_get_ao_volume = aop_hal_get_volume,
    .pfn_get_ao_tx_multislot = aop_hal_get_tx_multislot,
};

static umap_module g_aio_module = {
    .mod_id = OT_ID_AIO,
    .mod_name = "aio",

    .pfn_init = aiao_init,
    .pfn_exit = aiao_exit,
    .pfn_query_state = aiao_query_state,
    .pfn_notify = aiao_notify,
    .pfn_ver_checker = aiao_get_ver_magic,

    .export_funcs = &g_export_funcs,
    .data = TD_NULL,
};

td_s32 aio_interrupt_route(td_u32 top_int_status)
{
    if (g_aio_ctx.pfn_ai_isr_callback != TD_NULL) {
        if (top_int_status & 0x4U) {
            g_aio_ctx.pfn_ai_isr_callback(0x2); /* AIP通道2 */
        }

        if (top_int_status & 0x40U) {
            g_aio_ctx.pfn_ai_isr_callback(0x1); /* AIP通道1 */
        }

        if (top_int_status & 0x1U) {
            g_aio_ctx.pfn_ai_isr_callback(0x0); /* AIP通道0 */
        }
    }

    if (g_aio_ctx.pfn_ao_isr_callback != TD_NULL) {
        if (top_int_status & 0x2U) {
            g_aio_ctx.pfn_ao_isr_callback(0x0); /* AOP通道0 */
        }

        if (top_int_status & 0x80U) {
            g_aio_ctx.pfn_ao_isr_callback(0x1); /* AOP通道1 */
        }

        if (top_int_status & 0x8U) {
            g_aio_ctx.pfn_ao_isr_callback(0x2); /* AOP通道2 */
        }

        if (top_int_status & 0x10U) {
            g_aio_ctx.pfn_ao_isr_callback(0x3); /* AOP通道3 */
        }

        if (top_int_status & 0x20U) {
            g_aio_ctx.pfn_ao_isr_callback(0x4); /* AOP通道4 */
        }
    }
    return OSAL_IRQ_HANDLED;
}

static osal_dev_t *g_aiao_umap_dev = TD_NULL;

static unsigned int aiao_poll(osal_poll_t * const poll, void * const data)
{
    ot_unused(poll);
    ot_unused(data);
    return (td_u32)TD_SUCCESS;
}

static long aiao_ioctl(unsigned int cmd, unsigned long arg, void * const private_data)
{
    td_s32 ret;
    td_uintptr_t arg_adp = (td_uintptr_t)arg;

    ot_unused(private_data);

    switch (cmd) {
        case AIAO_SETMODPARAM:
            ret = aiao_hal_set_mod_param((ot_audio_mod_param *)arg_adp);
            break;

        case AIAO_GETMODPARAM:
            ret = aiao_hal_get_mod_param((ot_audio_mod_param *)arg_adp);
            break;

        default:
            aio_err_trace("ERR IOCTL CMD 0x%x, NR:%d!\n", cmd, (td_s32)_IOC_NR(cmd));
            ret = TD_FAILURE;
    }

    return ret;
}

static int aiao_open(void *private_data)
{
    ot_unused(private_data);
    return TD_SUCCESS;
}

static int aiao_close(void *private_data)
{
    ot_unused(private_data);
    return TD_SUCCESS;
}

#ifdef CONFIG_COMPAT
static long aiao_compat_ioctl(unsigned int cmd, unsigned long arg, void *private_data)
{
    return aiao_ioctl(cmd, arg, private_data);
}
#endif

static struct osal_fileops g_aiao_fops = {
    .module = THIS_MODULE,
    .open = aiao_open,
    .release = aiao_close,
    .unlocked_ioctl = aiao_ioctl,
    .poll = aiao_poll,
#ifdef CONFIG_COMPAT
    .compat_ioctl = aiao_compat_ioctl
#endif
};

#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
td_s32 aiao_proc_show(const struct osal_proc_dir_entry *s)
{
    ot_unused(s);
    return TD_SUCCESS;
}
#endif

static td_s32 aiao_module_register_process(td_void)
{
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
    osal_proc_entry_t *sub_proc = TD_NULL;

    /* create proc interface */
    sub_proc = osal_create_proc_entry(PROC_ENTRY_AIO, TD_NULL);
    if (sub_proc == TD_NULL) {
        aio_err_trace("aio create proc failed\n");
        return TD_FAILURE;
    }
    sub_proc->read = aiao_proc_show;
#endif

    g_aiao_umap_dev = osal_createdev(UMAP_DEVNAME_AIO_BASE);
    if (g_aiao_umap_dev == TD_NULL) {
        aio_err_trace("aio create dev failed\n");
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
        osal_remove_proc_entry(PROC_ENTRY_AIO, TD_NULL);
#endif
        return TD_FAILURE;
    }
    g_aiao_umap_dev->fops = &g_aiao_fops;
    g_aiao_umap_dev->minor = UMAP_AIO_MINOR_BASE;
    if (osal_registerdevice(g_aiao_umap_dev) != TD_SUCCESS) {
        aio_err_trace("aio register dev failed\n");
        (td_void)osal_destroydev(g_aiao_umap_dev);
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
        osal_remove_proc_entry(PROC_ENTRY_AIO, TD_NULL);
#endif
        g_aiao_umap_dev = TD_NULL;
        return TD_FAILURE;
    }

    if (cmpi_register_module(&g_aio_module)) {
        aio_err_trace("aio register module failed\n");
        osal_deregisterdevice(g_aiao_umap_dev);
        (td_void)osal_destroydev(g_aiao_umap_dev);
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
        osal_remove_proc_entry(PROC_ENTRY_AIO, TD_NULL);
#endif
        g_aiao_umap_dev = TD_NULL;
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_void aiao_module_unregister_process(td_void)
{
    cmpi_unregister_module(OT_ID_AIO);

    if (g_aiao_umap_dev != TD_NULL) {
        osal_deregisterdevice(g_aiao_umap_dev);
        (td_void)osal_destroydev(g_aiao_umap_dev);
        g_aiao_umap_dev = TD_NULL;
    }

#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
    osal_remove_proc_entry(PROC_ENTRY_AIO, TD_NULL);
#endif
}

static td_s32 aiao_module_init_clk(td_void)
{
    ot_mpp_chn mpp_chn = {0};
    td_bool reset = TD_FALSE;
    td_bool clk_en = TD_TRUE;

    /* AIAO源头时钟选择 */
    if (aiao_hal_set_clk_sel(AIO_DEFAULT_CLK_SEL) != TD_SUCCESS) {
        aio_err_trace("set_clk_sel fail.\n");
        return TD_FAILURE;
    }

    /* AIAO解除复位 */
    if (((td_bool)(ckfn_sys_entry()) == TD_FALSE) || (((td_bool)ckfn_sys_drv_ioctrl()) == TD_FALSE)) {
        aio_err_trace("sys module not insert.\n");
        return TD_FAILURE;
    }

    mpp_chn.mod_id = OT_ID_AIO;
    mpp_chn.dev_id = 0;
    call_sys_drv_ioctrl((hi_mpp_chn *)&mpp_chn, SYS_AIO_CLK_EN, &clk_en);
    call_sys_drv_ioctrl((hi_mpp_chn *)&mpp_chn, SYS_AIO_RESET_SEL, &reset);

    return TD_SUCCESS;
}

int aiao_module_init(void)
{
    td_s32 ret;
    if (aiao_reg_addr_init() != TD_SUCCESS) {
        return TD_FAILURE;
    }

    /* 注册proc,device,module */
    if (aiao_module_register_process() != TD_SUCCESS) {
        aiao_reg_addr_deinit();
        return TD_FAILURE;
    }

    (td_void)osal_sema_init(&g_sem_for_aio_ctx, 1);
    ret = osal_atomic_init(&g_aio_user_ref);
    if (ret < 0) {
        osal_sema_destroy(&g_sem_for_aio_ctx);
        aiao_module_unregister_process();
        aiao_reg_addr_deinit();
        return TD_FAILURE;
    }

    osal_atomic_set(&g_aio_user_ref, 0);
    aio_info_trace("load aio.ko ....OK!\n");

    return TD_SUCCESS;
}

void aiao_module_exit(void)
{
    ot_mpp_chn mpp_chn = {0};
    td_bool reset = TD_TRUE;
    td_bool clk_en = TD_FALSE;

    /* 低功耗策略，模块卸载时复位所有AIO */
    mpp_chn.mod_id = OT_ID_AIO;
    mpp_chn.dev_id = 0;
    if (((td_bool)(ckfn_sys_entry()) == TD_TRUE) && (((td_bool)ckfn_sys_drv_ioctrl()) == TD_TRUE)) {
        call_sys_drv_ioctrl((hi_mpp_chn *)&mpp_chn, SYS_AIO_RESET_SEL, &reset);
        call_sys_drv_ioctrl((hi_mpp_chn *)&mpp_chn, SYS_AIO_CLK_EN, &clk_en);
    }
#ifndef CONFIG_AUDIO_V200_SUPPORT
    osal_free_irq(g_aiao_irq, aio_interrupt_route);
#endif
    osal_atomic_destroy(&g_aio_user_ref);
    osal_sema_destroy(&g_sem_for_aio_ctx);

    aiao_module_unregister_process();

    aiao_reg_addr_deinit();
    aio_info_trace("unload aio.ko ....OK!\n");

    return;
}
