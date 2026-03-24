/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: audio_codec_pub.h, for aenc & adec
 * Author: Hisilicon multimedia software group
 * Create: 2020/06/26
 */

#ifndef AUDIO_CODEC_PUB_H
#define AUDIO_CODEC_PUB_H

#include "hi_comm_audio.h"
#ifdef CONFIG_OT_PLATFORM_V8
#include "ot_inner_common_aio.h"
#else
#include "ot_comm_aio_adapt.h"
#endif

static inline td_char *audio_print_boolean(td_bool value)
{
    return (value) ? "y" : "n";
}

td_char *audio_print_pt(ot_payload_type type);

td_char *audio_print_adpcm_type(ot_adpcm_type adpcm_type);

td_char *audio_print_g726_bps(ot_g726_bps g726bps);

static inline td_char *audio_print_decode_mode(td_bool stream)
{
    if (stream == TD_TRUE) {
        return "stream";
    } else if (stream == TD_FALSE) {
        return "packet";
    } else {
        return "NULL";
    }
}

#endif /* end of #ifndef AUDIO_CODEC_PUB_H */