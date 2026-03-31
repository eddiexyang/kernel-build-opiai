/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2009-2019. All rights reserved.
 * Description: header of aio driver common function
 * Author: Hisilicon multimedia software group
 * Create: 2009/5/5
 * History:
 *   1.Date        : 2009/5/5
 *     Modification: Created file
 *   2.Date        : 2019/08/08
 *     Modification: Update file for CSEC
 */

#ifndef AIAO_HAL_COMM_H
#define AIAO_HAL_COMM_H

#include "hi_comm_audio.h"
#ifdef CONFIG_OT_PLATFORM_V8
#include "ot_inner_common_aio.h"
#else
#include "ot_comm_aio_adapt.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define AIO_DATA_WIDTH_8BIT         0x0
#define AIO_DATA_WIDTH_16BIT        0x1
#define AIO_DATA_WIDTH_18BIT        0x2
#define AIO_DATA_WIDTH_20BIT        0x3
#define AIO_DATA_WIDTH_24BIT        0x4
#define AIO_DATA_WIDTH_32BIT        0x5

#define AIO_PCM_DATA_WIDTH_8BIT     0x0
#define AIO_PCM_DATA_WIDTH_16BIT    0x1
#define AIO_PCM_DATA_WIDTH_32BIT    0x2
#define AIO_PCM_DATA_WIDTH_64BIT    0x3
#define AIO_PCM_DATA_WIDTH_128BIT   0x4

#define AIO_CHN_NUM_2CHN            0x0
#define AIO_CHN_NUM_4CHN            0x1
#define AIO_CHN_NUM_8CHN            0x2
#define AIO_CHN_NUM_16CHN           0x3

#define AIO_CLK_SEL_SEPARATE        0x0U
#define AIO_CLK_SEL_INSEPARATE      0x1U

#define AIO_BCLK_TO_FSCLK_RATIO_16  16
#define AIO_BCLK_TO_FSCLK_RATIO_32  32
#define AIO_BCLK_TO_FSCLK_RATIO_48  48
#define AIO_BCLK_TO_FSCLK_RATIO_64  64
#define AIO_BCLK_TO_FSCLK_RATIO_128 128
#define AIO_BCLK_TO_FSCLK_RATIO_256 256
#define AIO_BCLK_TO_FSCLK_RATIO_320 320
#define AIO_BCLK_TO_FSCLK_RATIO_384 384

#define AIO_MCLK_TO_BCLK_RATIO_1    1
#define AIO_MCLK_TO_BCLK_RATIO_2    2
#define AIO_MCLK_TO_BCLK_RATIO_3    3
#define AIO_MCLK_TO_BCLK_RATIO_4    4
#define AIO_MCLK_TO_BCLK_RATIO_6    6
#define AIO_MCLK_TO_BCLK_RATIO_8    8
#define AIO_MCLK_TO_BCLK_RATIO_12   12
#define AIO_MCLK_TO_BCLK_RATIO_16   16
#define AIO_MCLK_TO_BCLK_RATIO_24   24
#define AIO_MCLK_TO_BCLK_RATIO_32   32
#define AIO_MCLK_TO_BCLK_RATIO_48   48
#define AIO_MCLK_TO_BCLK_RATIO_64   64

#define AIO_CHN_CNT_1               1U
#define AIO_CHN_CNT_2               2U
#define AIO_CHN_CNT_4               4U
#define AIO_CHN_CNT_8               8U
#define AIO_CHN_CNT_16              16U
#define AIO_CHN_CNT_20              20U

#define AIO_DEV_ID_0                0
#define AIO_DEV_ID_1                1
#define AIO_DEV_ID_2                2

#define AIO_FIFO_BIT_WIDTH_256      256
#define AIO_FIFO_BIT_WIDTH_320      320
#define AIO_FIFO_BIT_WIDTH_384      384

#define AIO_ONE_FIFO_BITWIDTH       128

#define aio_time_diff_us(a, b)      ((((a).tv_sec) - ((b).tv_sec)) * 1000000 + (((a).tv_usec) - ((b).tv_usec)))

#define aio_is_pcm_mode(mode) \
    (((mode) == OT_AIO_MODE_PCM_SLAVE_STD) || (((mode) == OT_AIO_MODE_PCM_SLAVE_NON_STD)) || \
     ((mode) == OT_AIO_MODE_PCM_MASTER_STD) || ((mode) == OT_AIO_MODE_PCM_MASTER_NON_STD))
#define aio_is_i2s_mode(mode) \
    (((mode) == OT_AIO_MODE_I2S_MASTER) || ((mode) == OT_AIO_MODE_I2S_SLAVE))
#define aio_is_master_mode(mode) \
    (((mode) == OT_AIO_MODE_I2S_MASTER) || ((mode) == OT_AIO_MODE_PCM_MASTER_STD) || \
     ((mode) == OT_AIO_MODE_PCM_MASTER_NON_STD))
#define aio_is_slave_mode(mode) \
    (((mode) == OT_AIO_MODE_I2S_SLAVE) || ((mode) == OT_AIO_MODE_PCM_SLAVE_STD) || \
     ((mode) == OT_AIO_MODE_PCM_SLAVE_NON_STD))

#define ai_check_dev_return(dev)                                           \
    do {                                                                   \
        if (((dev) >= OT_AI_DEV_MAX_NUM) || ((dev) < 0)) {                 \
            ai_err_trace("ai dev %d is invalid\n", dev);                   \
            return OT_ERR_AI_INVALID_DEV_ID;                       \
        }                                                                  \
    } while (0)

#define ao_check_dev_return(dev)                                           \
    do {                                                                   \
        if (((dev) >= OT_AO_DEV_MAX_NUM) || ((dev) < OT_AO_DEV_MIN_NUM)) { \
            ao_err_trace("ao dev %d is invalid\n", dev);                   \
            return OT_ERR_AO_INVALID_DEV_ID;                       \
        }                                                                  \
    } while (0)

#define AIP0_INTMASK 1
#define AOP0_INTMASK 1
#define AOP1_INTMASK 1

#ifdef OT_FPGA
/* 307.2M */
#define AIO_MCLK_48K                0x0051EB85 /* 48k * 256 */
#define AIO_MCLK_441K               0x004B4396 /* 44.1k * 256 */
#define AIO_MCLK_32K                0x00369D03 /* 32k * 256 */
#define AIO_MCLK_24K                0x0028F5C2 /* 24k * 256 */
#define AIO_MCLK_16K                0x001B4E82 /* 16K*256 */
#define AIO_MCLK_8K                 0x000DA741 /* 8K*256 */

#define AIO_MCLK_96K_320FS          0x00CCCCCD  /* 96k * 320 */
#define AIO_MCLK_48K_320FS          0x00666666  /* 48k * 320 */
#define AIO_MCLK_441K_320FS         0x005E147B  /* 44.1k * 320 */
#define AIO_MCLK_32K_320FS          0x00444444  /* 32k * 320 */
#else
#ifdef AIO_DEBUG
/* 1500M */
#define AIO_MCLK_96K                0x00218DEF /* 96k * 256 */
#define AIO_MCLK_48K                0x0010C6F8 /* 48k * 256 */
#define AIO_MCLK_441K               0x00F6A00  /* 44.1k * 256 */
#define AIO_MCLK_32K                0x00B2F50  /* 32k * 256 */

/* 500M */
#define AIO_MCLK_96K                0x0064A9CE /* 96k * 256 */
#define AIO_MCLK_48K                0x003254E7 /* 48k * 256 */
#define AIO_MCLK_441K               0x002E3E01 /* 44.1k * 256 */
#define AIO_MCLK_32K                0x00218DEF /* 32k * 256 */
#else
/* 1800M */
/* (MCLK0/AIO 时钟源头频率) x 2^27 */
#define AIO_MCLK_48K_256FS_1800M    0x000DFB24 /* 48k * 256 */
#define AIO_MCLK_441K_256FS_1800M   0x000CD856 /* 44.1k * 256 */
#define AIO_MCLK_32K_256FS_1800M    0x00095218 /* 32k * 256 */

#define AIO_MCLK_48K_320FS_1800M    0x001179ED /* 48k * 320 */
#define AIO_MCLK_441K_320FS_1800M   0x00100E6B /* 44.1k * 320 */
#define AIO_MCLK_32K_320FS_1800M    0x000BA69E /* 32k * 320 */

/* 1400M */
/* (MCLK0/AIO 时钟源头频率) x 2^27 */
#define AIO_MCLK_48K_256FS_1400M    0x0011F9C0 /* 48k * 256 */
#define AIO_MCLK_441K_256FS_1400M   0x001083DC /* 44.1k * 256 */
#define AIO_MCLK_32K_256FS_1400M    0x000BFBD5 /* 32k * 256 */

#define AIO_MCLK_48K_320FS_1400M    0x00167830 /* 48k * 320 */
#define AIO_MCLK_441K_320FS_1400M   0x0014A4D3 /* 44.1k * 320 */
#define AIO_MCLK_32K_320FS_1400M    0x000EFACB /* 32k * 320 */

/* 1188M */
/* (MCLK0/AIO 时钟源头频率) x 2^27 */
#define AIO_MCLK_48K_256FS_1188M    0x00152EF0 /* 48k * 256 */
#define AIO_MCLK_441K_256FS_1188M   0x00137653 /* 44.1k * 256 */
#define AIO_MCLK_32K_256FS_1188M    0x000E1F4B /* 32k * 256 */

#define AIO_MCLK_48K_320FS_1188M    0x001A7AAC /* 48k * 320 */
#define AIO_MCLK_441K_320FS_1188M   0x00185FA0 /* 44.1k * 320 */
#define AIO_MCLK_32K_320FS_1188M    0x0011A71E /* 32k * 320 */

/* 786.432M */
/* (MCLK0/AIO 时钟源头频率) x 2^27 */
#define AIO_MCLK_48K_256FS_786M     0x00200000 /* 48k * 256 */
#define AIO_MCLK_441K_256FS_786M    0x001D6666 /* 44.1k * 256 */
#define AIO_MCLK_32K_256FS_786M     0x00155555 /* 32k * 256 */

#define AIO_MCLK_48K_320FS_786M     0x00280000 /* 48k * 320 */
#define AIO_MCLK_441K_320FS_786M    0x0024C000 /* 44.1k * 320 */
#define AIO_MCLK_32K_320FS_786M     0x001AAAAB /* 32k * 320 */

#endif
#endif

#define AIAO_INT_AIP0_MASK 0x1U
#define AIAO_INT_AIP1_MASK (0x1U << 1)
#define AIAO_INT_AIP2_MASK (0x1U << 2)

#define AIAO_INT_AOP0_MASK (0x1U << 16)
#define AIAO_INT_AOP1_MASK (0x1U << 17)
#define AIAO_INT_AOP2_MASK (0x1U << 18)

#define ASP_SUPPORT_MAX_CHN_CNT 2U /* 芯片硬件支持的通道数 */

typedef enum {
    AIO_TYPE_AI = 0,
    AIO_TYPE_AO,
    AIO_TYPE_BUTT
} aio_type;

typedef enum {
    I2S_MUX_ACODEC = 0,
    I2S_MUX_AIAO,
    I2S_MUX_MISC,
    I2S_MUX_BUTT
} i2s_mux_type;

typedef enum {
    I2S_DIGIT_0 = 0,
    I2S_DIGIT_1,
    I2S_ANA,
    I2S_HDMI_0,
    I2S_HDMI_1
} i2s_dev_type_num;

typedef struct {
    td_bool     initialized; /* initialed flag */
    td_bool     ai_config;
    td_bool     ao_config;
    ot_aio_attr ai_attr;
    ot_aio_attr ao_attr;
} aio_state;

typedef td_void (*fn_aio_callback)(ot_audio_dev audio_dev_id);

typedef struct {
    fn_aio_callback pfn_ai_isr_callback;
    fn_aio_callback pfn_ao_isr_callback;
} aio_ctx;

typedef struct {
    ot_audio_clk_select clk_select;

    /* 256fs */
    td_u32 mclk_div_48k_256fs;
    td_u32 mclk_div_441k_256fs;
    td_u32 mclk_div_32k_256fs;

    /* 320fs */
    td_u32 mclk_div_48k_320fs;
    td_u32 mclk_div_441k_320fs;
    td_u32 mclk_div_32k_320fs;
} aiao_clk_info;

/* 寄存器读写，对外接口，对内使用aio_hal.c中定义的宏 */
td_u32 aio_hal_read_reg(td_u32 offset);
td_void aio_hal_write_reg(td_u32 offset, td_u32 value);
/* 物理地址映射为虚拟地址 */
td_s32 aio_hal_sys_init(td_void);
td_void aio_hal_sys_deinit(td_void);
/* 返回aio中断状态寄存器内容，本函数并没有丰富处理，简单的将数值返回 */
td_u32 aio_hal_get_int_status(td_void);
/* 接收通道使能操作 */
td_void aip_hal_int_en(ot_audio_dev ai_dev_id, td_bool en);
/* 发送通道使能操作 */
td_void aop_hal_int_en(ot_audio_dev ao_dev_id, td_bool en);
td_u32 aip_hal_get_buff_wptr(ot_audio_dev ai_dev_id);
/* 更新缓存读指针 */
td_void aip_hal_set_buff_rptr(ot_audio_dev ai_dev_id, td_u32 value);
td_u32 aip_hal_get_buff_rptr(ot_audio_dev ai_dev_id);
td_void aip_hal_set_child_int_mask(ot_audio_dev ai_dev_id);
/* 获得某中断状态寄存器状态 */
td_void aip_hal_set_child_int_status(ot_audio_dev ai_dev_id, td_u32 value);
td_u32 aip_hal_get_child_int_all_status(ot_audio_dev ai_dev_id);
td_void aip_hal_clr_child_int_all_status(ot_audio_dev ai_dev_id);
/* 设置清除原始中断寄存器 */
td_void aip_hal_set_child_int_clr(ot_audio_dev ai_dev_id);
td_s32 aip_hal_clr_int_status(ot_audio_dev ai_dev_id);
/* 设置通道使能 */
td_void aip_hal_set_rx_start(ot_audio_dev ai_dev_id, td_bool en);
/* 查询disable done状态 */
td_u32 aip_hal_get_dis_done(ot_audio_dev ai_dev_id);
/* 通道缓存可读数据读 */
td_s32 aip_hal_set_dev_attr(ot_audio_dev ai_dev_id, const ot_aio_attr *attr);
/* 使能中断，清除原始中断，启动设备 */
td_s32 aip_hal_dev_enable(ot_audio_dev ai_dev_id);
td_void aip_hal_dev_disable(ot_audio_dev ai_dev_id);
td_s32 aip_hal_get_int_status(ot_audio_dev ai_dev_id, td_u32 * const int_status);
/* AOP驱动 */
td_u32 aop_hal_get_buff_wptr(ot_audio_dev ao_dev_id);
td_u32 aop_hal_get_buff_rptr(ot_audio_dev ao_dev_id);
td_void aop_hal_set_buff_wptr(ot_audio_dev ao_dev_id, td_u32 value);
td_void aop_hal_set_tx_start(ot_audio_dev ao_dev_id, td_bool en);
td_u32 aop_hal_get_dis_done(ot_audio_dev ao_dev_id);
td_void aop_hal_set_child_int_clr(ot_audio_dev ao_dev_id);
td_u32 aop_hal_get_child_int_all_status(ot_audio_dev ao_dev_id);
td_void aop_hal_clr_child_int_all_status(ot_audio_dev ao_dev_id);
td_void aop_hal_set_child_int_mask(ot_audio_dev ao_dev_id);
td_s32 aop_hal_clr_int_status(ot_audio_dev ao_dev_id);
td_s32 aop_hal_set_attr(ot_audio_dev ao_dev_id, ot_aio_attr *attr);
td_s32 aop_hal_dev_enable(ot_audio_dev ao_dev_id);
td_void aop_hal_dev_disable(ot_audio_dev ao_dev_id);
td_s32 aop_hal_get_int_status(ot_audio_dev ao_dev_id, td_u32 * const int_status);
td_s32 aio_interrupt_route(td_u32 top_int_status);

static inline td_u32 aio_get_bit_cnt(ot_audio_bit_width bit_width)
{
    if (bit_width == OT_AUDIO_BIT_WIDTH_8) {
        return 8U; /* 8 bit */
    } else if (bit_width == OT_AUDIO_BIT_WIDTH_16) {
#ifdef CONFIG_AUDIO_V200_SUPPORT
        return 32U; /* 16 bit hardware as 32 bit */
#else
        return 16U; /* 16 bit */
#endif
    } else {
        /* 24bit/32bit位宽均采用32bit进行储存 */
        return 32U; /* 32 bit */
    }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
