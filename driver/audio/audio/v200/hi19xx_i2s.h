/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2009-2020. All rights reserved.
 * Description: aenc_ext.h
 * Author: Hisilicon multimedia software group
 * Create: 2009-05-05
 */

#ifndef __HI19XX_I2S_H__
#define __HI19XX_I2S_H__

#include <linux/clk.h>
#include "hi_comm_audio.h"
#include "ot_inner_common_aio.h"

#define AUDIO_I2S_RATES \
        (SNDRV_PCM_RATE_8000 |  SNDRV_PCM_RATE_16000 | \
        SNDRV_PCM_RATE_48000 |  SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_192000)

/* Register offsets from AUDIO_I2S1_BASE and AUDIO_I2S2_BASE */
#define I2S_REG_ADDR 0x403152000
#define I2S_REG_SIZE 0x290
#define I2S_REG_AXITDM_ADDR 0x040314E400
#define I2S_REG_AXITDM_SIZE 0x100

#define AUDIO_I2S1_CTRL                0x04
#define AUDIO_TDM_CTRL0                0x18
#define AUDIO_TDM_CTRL1                0x1C
#define AUDIO_I2S2_PCM_CTRL            0x20
#define AUDIO_I2S3_PCM_CTRL            0x24

/* Fields in AUDIO_I2S_CTRL */
#define AUDIO_I2S_CTRL_FIFO2_TX_ENABLE         (1 << 30)
#define AUDIO_I2S_CTRL_FIFO1_ENABLE            (1 << 29)
#define AUDIO_I2S_CTRL_FIFO2_ENABLE            (1 << 28)
#define AUDIO_I2S_CTRL_FIFO1_RX_ENABLE         (1 << 27)
#define AUDIO_I2S_CTRL_FIFO_LPBK_ENABLE        (1 << 26)
#define AUDIO_I2S_CTRL_MASTER_ENABLE           (1 << 25)

#define AUDIO_I2S_LRCK_LEFT_LOW            0
#define AUDIO_I2S_LRCK_RIGHT_LOW           1

#define AUDIO_I2S_CTRL_LRCK_SHIFT          26
#define AUDIO_I2S_CTRL_LRCK_MASK           (1 << AUDIO_I2S_CTRL_LRCK_SHIFT)
#define AUDIO_I2S_CTRL_LRCK_L_LOW          (AUDIO_I2S_LRCK_LEFT_LOW  << AUDIO_I2S_CTRL_LRCK_SHIFT)
#define AUDIO_I2S_CTRL_LRCK_R_LOW          (AUDIO_I2S_LRCK_RIGHT_LOW << AUDIO_I2S_CTRL_LRCK_SHIFT)

#define AUDIO_I2S_BIT_FORMAT_I2S           0
#define AUDIO_I2S_BIT_FORMAT_PCM           2
#define AUDIO_I2S_BIT_FORMAT_LJM           4
#define AUDIO_I2S_BIT_FORMAT_RJM           5

#define AUDIO_I2S_CTRL_BIT_FORMAT_SHIFT        21
#define AUDIO_I2S_CTRL_BIT_FORMAT_MASK         (7 << AUDIO_I2S_CTRL_BIT_FORMAT_SHIFT)
#define AUDIO_I2S_CTRL_BIT_FORMAT_I2S          (AUDIO_I2S_BIT_FORMAT_I2S << AUDIO_I2S_CTRL_BIT_FORMAT_SHIFT)
#define AUDIO_I2S_CTRL_BIT_FORMAT_PCM          (AUDIO_I2S_BIT_FORMAT_PCM << AUDIO_I2S_CTRL_BIT_FORMAT_SHIFT)
#define AUDIO_I2S_CTRL_BIT_FORMAT_LJM          (AUDIO_I2S_BIT_FORMAT_LJM << AUDIO_I2S_CTRL_BIT_FORMAT_SHIFT)
#define AUDIO_I2S_CTRL_BIT_FORMAT_RJM          (AUDIO_I2S_BIT_FORMAT_RJM << AUDIO_I2S_CTRL_BIT_FORMAT_SHIFT)

#define AUDIO_I2S_BIT_SIZE_16              0
#define AUDIO_I2S_BIT_SIZE_20              1
#define AUDIO_I2S_BIT_SIZE_24              2
#define AUDIO_I2S_BIT_SIZE_32              3

#define I2S_GAIN_LEVEL1  0x06e01b50
#define I2S_GAIN_LEVEL2  0x16e20a50

struct asp_i2s_priv {
        struct device *dev;
        spinlock_t rw_lock;
        struct resource *res;
        void __iomem *asp_codec_reg_base_addr;
        struct snd_soc_dai_driver *dai;
#ifdef CONFIG_DEBUG_FS
        struct dentry *dbg_dir;
#endif
};

#define BITS_MASK(start, end) \
        (((((unsigned int)0x1) << (((end) - (start)) + 1)) - 1) << (start))

#define GET_REG_MEMBER_VALUE(type, tmp_value, offset, member) \
        (((tmp_value) & BITS_MASK(SOC_##type##_##offset##_##member##_START, SOC_##type##_##offset##_##member##_END)) \
                    >> SOC_##type##_##offset##_##member##_START)

#define GET_MEMBER(type, temp, offset, base) do { \
    temp = readl(SOC_##type##_##offset##_ADDR(base)); \
} while (0)

#define SET_MEMBER_CACHE(type, temp, offset, member, value) do { \
    (temp) &= ~BITS_MASK(SOC_##type##_##offset##_##member##_START, SOC_##type##_##offset##_##member##_END); \
    (temp) |= (unsigned)((((unsigned int)(value)) << (SOC_##type##_##offset##_##member##_START)) & \
    (BITS_MASK(SOC_##type##_##offset##_##member##_START, SOC_##type##_##offset##_##member##_END))); \
} while (0)

#define GET_MEMBER_VALUE(type, temp, value, offset, member, base) do { \
                temp = readl(SOC_##type##_##offset##_ADDR(base)); \
                (value) = (temp & BIT_MASK(SOC_##type##_##offset##_##member##_START, \
                SOC_##type##_##offset##_##member##_END)) \
                        >> SOC_##type##_##offset##_##member##_START; \
} while (0)


#define IN_FUNCTION pr_info("audio[I]:%s:%d: begin\n", __func__, __LINE__)
#define OUT_FUNCTION pr_info("audio[I]:%s:%d: end\n", __func__, __LINE__)

void audio_ao_i2s_trigger(ot_audio_dev audio_device_id, void *i2s_reg_base);
void audio_ao_i2s_stop(ot_audio_dev audio_device_id, void *i2s_reg_base);
void audio_ai_i2s_trigger(ot_audio_dev audio_device_id, void *i2s_reg_base);
void audio_ai_i2s_stop(ot_audio_dev audio_device_id, void *i2s_reg_base);
void audio_i2s_init(ot_audio_dev audio_device_id, void *i2s_reg_base);
#endif
