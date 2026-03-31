/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: aenc proc for V8 platform
 * Author: Hisilicon multimedia software group
 * Create: 2019-09-01
 */

#include "aenc_proc.h"
#include "audio_codec_pub.h"
#include "aenc.h"
#include "sys_ext.h"

td_s32 aenc_proc_show(struct osal_proc_dir_entry *s)
{
    td_s32 i;
    aenc_chn_ctx *aenc_chn = TD_NULL;

    if (!ckfn_sys_entry()) {
        aenc_err_trace("sys module not insert!\n");
        return OT_ERR_AENC_NOT_READY;
    }

    if (!ckfn_sys_print_proc_title()) {
        aenc_err_trace("print proc title is null!\n");
        return OT_ERR_AENC_NOT_READY;
    }

    osal_seq_printf(s, "\n[AENC] Version: ["OT_MPP_VERSION"], Build Time["__DATE__", "__TIME__"]\n\n");

    call_sys_print_proc_title(s, "aenc chn attr");
    osal_seq_printf(s, "%6s" "%14s" "%12s" "%11s" "%10s" "%16s\n",
                    "chn_id", "payload_type", "adpcm_type", "point_num", "buf_size", "g726_rate");

    for (i = 0; i < OT_AENC_MAX_CHN_NUM; i++) {
        aenc_chn = aenc_get_chn_ctx(i);
        if (aenc_chn->created == TD_FALSE)
            continue;

        osal_seq_printf(s, "%6d" "%14s" "%12s" "%11u" "%10u" "%16s\n",
                        i, audio_print_pt(aenc_chn->chn_attr.type),
                        audio_print_adpcm_type(aenc_chn->send_dbg.adpcm_type),
                        aenc_chn->chn_attr.point_num_per_frame,
                        aenc_chn->chn_attr.buf_size,
                        audio_print_g726_bps(aenc_chn->send_dbg.g726_rate));
    }

    call_sys_print_proc_title(s, "aenc chn status");
    osal_seq_printf(s, "%6s" "%12s" "%15s" "%11s" "%11s" "%11s" "%12s" "%16s" "%6s\n",
                    "chn_id", "recv_frame", "ai_queue_lost", "enc_ok", "frame_err",
                    "buf_full", "get_stream", "release_stream", "mute");
    for (i = 0; i < OT_AENC_MAX_CHN_NUM; i++) {
        aenc_chn = aenc_get_chn_ctx(i);
        if (aenc_chn->created == TD_FALSE) {
            continue;
        }

        osal_seq_printf(s, "%6d" "%12u" "%15u" "%11u" "%11u" "%11u" "%12u" "%16u" "%6s\n",
                        i, aenc_chn->send_dbg.rcv_frm,
                        aenc_chn->usr_que_lost,
                        aenc_chn->send_dbg.enc_ok,
                        aenc_chn->send_dbg.frm_err,
                        aenc_chn->send_dbg.buf_full,
                        aenc_chn->send_dbg.get_strm,
                        aenc_chn->send_dbg.rls_strm,
                        audio_print_boolean(aenc_chn->mute));
    }

    return TD_SUCCESS;
}
