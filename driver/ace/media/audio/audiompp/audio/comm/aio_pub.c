/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: aio pub driver
 * Author: Hisilicon multimedia software group
 * Create: 2023/03/13
 */

#include "aio_pub.h"

td_char *audio_print_work_mode(ot_aio_mode workmode)
{
    if (workmode == OT_AIO_MODE_I2S_MASTER) {
        return "i2s_mas";
    } else if (workmode == OT_AIO_MODE_I2S_SLAVE) {
        return "i2s_sla";
    } else if (workmode == OT_AIO_MODE_PCM_SLAVE_STD) {
        return "pcm0_sl";
    } else if (workmode == OT_AIO_MODE_PCM_SLAVE_NON_STD) {
        return "pcm1_sl";
    } else if (workmode == OT_AIO_MODE_PCM_MASTER_STD) {
        return "pcm0_mt";
    } else if (workmode == OT_AIO_MODE_PCM_MASTER_NON_STD) {
        return "pcm1_mt";
    } else {
        return "";
    }
}

td_char *audio_print_sample_rate(ot_audio_sample_rate sample_rate)
{
    if (sample_rate == OT_AUDIO_SAMPLE_RATE_8000) {
        return "8kHz";
    } else if (sample_rate == OT_AUDIO_SAMPLE_RATE_11025) {
        return "11kHz";
    } else if (sample_rate == OT_AUDIO_SAMPLE_RATE_12000) {
        return "12kHz";
    } else if (sample_rate == OT_AUDIO_SAMPLE_RATE_16000) {
        return "16kHz";
    } else if (sample_rate == OT_AUDIO_SAMPLE_RATE_22050) {
        return "22kHz";
    } else if (sample_rate == OT_AUDIO_SAMPLE_RATE_24000) {
        return "24kHz";
    } else if (sample_rate == OT_AUDIO_SAMPLE_RATE_32000) {
        return "32kHz";
    } else if (sample_rate == OT_AUDIO_SAMPLE_RATE_44100) {
        return "44.1kHz";
    } else if (sample_rate == OT_AUDIO_SAMPLE_RATE_48000) {
        return "48kHz";
    } else if (sample_rate == OT_AUDIO_SAMPLE_RATE_64000) {
        return "64kHz";
    } else if (sample_rate == OT_AUDIO_SAMPLE_RATE_96000) {
        return "96kHz";
    } else {
        return "";
    }
}

td_char *audio_print_bit_width(ot_audio_bit_width bit_width)
{
    if (bit_width == OT_AUDIO_BIT_WIDTH_8) {
        return "8bit";
    } else if (bit_width == OT_AUDIO_BIT_WIDTH_16) {
        return "16bit";
    } else if (bit_width == OT_AUDIO_BIT_WIDTH_24) {
        return "24bit";
    } else {
        return "";
    }
}

td_char *audio_print_chn_state(aio_chn_state chn_state)
{
    if (chn_state == AIO_CHN_ORIGINAL) {
        return "orig";
    } else if (chn_state == AIO_CHN_ENABLE) {
        return "enable";
    } else if (chn_state == AIO_CHN_PAUSE) {
        return "pause";
    } else if (chn_state == AIO_CHN_DISABLE) {
        return "disable";
    } else {
        return "";
    }
}

td_char *audio_print_vqe_mode(ot_vqe_work_state vqe_mode)
{
    if (vqe_mode == OT_VQE_WORK_STATE_COMMON) {
        return "comm";
    } else if (vqe_mode == OT_VQE_WORK_STATE_MUSIC) {
        return "music";
    } else if (vqe_mode == OT_VQE_WORK_STATE_NOISY) {
        return "noisy";
    } else {
        return "";
    }
}