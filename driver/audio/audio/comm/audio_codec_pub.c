/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2020-2022. All rights reserved.
 * Description: audio_codec_pub.c, for aenc & adec
 * Author: Hisilicon multimedia software group
 * Create: 2020/06/26
 */

#include "audio_codec_pub.h"

td_char *audio_print_pt(ot_payload_type type)
{
    if (type == OT_PT_G711A) {
        return "g711a";
    } else if (type == OT_PT_G711U) {
        return "g711u";
    } else if (type == OT_PT_ADPCMA) {
        return "adpcm";
    } else if (type == OT_PT_G726) {
        return "g726";
    } else if (type == OT_PT_AMR) {
        return "amrnb";
    } else if (type == OT_PT_AMRWB) {
        return "amrwb";
    } else if (type == OT_PT_AAC) {
        return "aac";
    } else if (type == OT_PT_LPCM) {
        return "lpcm";
    } else if (type == OT_PT_MP3) {
        return "mp3";
    } else {
        return " ";
    }
}

td_char *audio_print_adpcm_type(ot_adpcm_type adpcm_type)
{
    if (adpcm_type == OT_ADPCM_TYPE_DVI4) {
        return "DVI4";
    } else if (adpcm_type == OT_ADPCM_TYPE_IMA) {
        return "IMA";
    } else if (adpcm_type == OT_ADPCM_TYPE_ORG_DVI4) {
        return "ORG_DVI4";
    } else {
        return "NULL";
    }
}

td_char *audio_print_g726_bps(ot_g726_bps g726bps)
{
    if (g726bps == OT_G726_16K) {
        return "G726_16K";
    } else if (g726bps == OT_G726_24K) {
        return "G726_24K";
    } else if (g726bps == OT_G726_32K) {
        return "G726_32K";
    } else if (g726bps == OT_G726_40K) {
        return "G726_40K";
    } else if (g726bps == OT_MEDIA_G726_16K) {
        return "MEDIA_G726_16K";
    } else if (g726bps == OT_MEDIA_G726_24K) {
        return "MEDIA_G726_24K";
    } else if (g726bps == OT_MEDIA_G726_32K) {
        return "MEDIA_G726_32K";
    } else if (g726bps == OT_MEDIA_G726_40K) {
        return "MEDIA_G726_40K";
    } else {
        return "NULL";
    }
}