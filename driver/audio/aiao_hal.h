/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: header of aiao hal
 * Author: Hisilicon multimedia software group
 * Create: 2020/05/18
 */

#ifndef AIAO_HAL_H
#define AIAO_HAL_H

#include "aiao_hal_comm.h"

#define AUDIO_CODEC_ANAREG0 0x0

#define AUDIO_CODEC_ANAREG_BOOST    0x08
#define AUDIO_CODEC_ANAREG_GAINL    0x00
#define AUDIO_CODEC_ANAREG_GAINR    0x04

#define AUDIO_AIO_I2S_MUX_REG 0x60U

#define AIO_FIFO_BITWIDTH_MAX 384U

#define AIO_I2S_MUX_TYPE I2S_MUX_AIAO

/* DDR缓存 高32位地址 */
#define aip_buff_saddr_reg_h(n)   ((td_u32)(0x10B0 + 0x100 * (n))) /* 接收通道的DDR缓存起始地址寄存器(高32位) */
#define aop_buff_saddr_reg_h(m)   ((td_u32)(0x20B0 + 0x100 * (m))) /* 发送通道的DDR缓存起始地址寄存器(高32位) */

/* HI3536AV100默认使用小数分频的1400MHz PLL */
#define AIO_DEFAULT_CLK_SEL     OT_AUDIO_CLK_SELECT_SPARE

/* HI3536AV100不支持TX时分复用，仅支持Normal多根数据线输出 */
#define AOP_SUPPORT_MULTI_SLOT  TD_FALSE

td_s32 aiao_hal_set_clk_sel_private(ot_audio_clk_select clk_select, aiao_clk_info *clk_info);

td_s32 aiao_hal_set_mod_param_private(const ot_audio_mod_param *mod_param);

td_bool check_aip_share_aop_clk_private(ot_audio_dev dev_id);

td_s32 aip_set_bclk_sel_private(ot_audio_dev dev_id, const ot_aio_attr *attr, td_u8 *aio_reg_base);

td_s32 aip_set_mask_private(ot_audio_dev dev_id, const ot_aio_attr *attr, td_u8 *reg_base);
td_s32 aop_set_mask_private(ot_audio_dev dev_id, const ot_aio_attr *attr, td_u8 *reg_base);

td_s32 check_ai_attr_private(ot_audio_dev ai_dev_id, const ot_aio_attr *attr);
td_s32 check_ao_attr_private(ot_audio_dev dev_id, const ot_aio_attr *attr);

td_s32 aip_hal_set_acodec_gain_private(td_s32 gain, td_u8 *acodec_reg_base);
td_s32 aip_hal_get_acodec_gain_private(td_s32 *gain, const td_u8 *acodec_reg_base);

td_void aiao_module_init_private(td_u8 *aio_reg_base);

#endif  /* AIAO_HAL_H */