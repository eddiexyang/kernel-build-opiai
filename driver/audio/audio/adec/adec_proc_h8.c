/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2022. All rights reserved.
 * Description: adec proc for V8 platform
 * Author: Hisilicon multimedia software group
 * Create: 2019-09-01
 */

#include "adec_proc.h"
#include "audio_codec_pub.h"
#include "adec.h"
#include "sys_ext.h"

td_s32 adec_proc_show(struct osal_proc_dir_entry *s)
{
    td_s32 i;
    adec_chn_ctx *adec_chn_temp = TD_NULL;

    if (!ckfn_sys_entry()) {
        adec_err_trace("sys module not insert!\n");
        return OT_ERR_ADEC_NOT_READY;
    }

    if (!ckfn_sys_print_proc_title()) {
        adec_err_trace("print proc title is null!\n");
        return OT_ERR_ADEC_NOT_READY;
    }

    osal_seq_printf(s, "\n[ADEC] Version: [" OT_MPP_VERSION "], Build Time["__DATE__", "__TIME__"]\n\n");

    call_sys_print_proc_title(s, "adec chn attr");
    osal_seq_printf(s, "%6s" "%14s" "%12s" "%10s" "%16s" "%8s" "%15s" "%11s" "%11s" "%11s\n",
                    "chn_id", "payload_type", "adpcm_type", "buf_size", "g726_rate",
                    "mode", "orig_send_cnt", "send_cnt", "get_cnt", "put_cnt");

    for (i = 0; i < OT_ADEC_MAX_CHN_NUM; i++) {
        adec_chn_temp = adec_get_chn_ctx(i);
        if (adec_chn_temp->created == TD_FALSE) {
            continue;
        }

        osal_seq_printf(s, "%6d" "%14s" "%12s" "%10u" "%16s" "%8s" "%15d" "%11d" "%11d" "%11d\n",
                        i, audio_print_pt(adec_chn_temp->chn_attr.type),
                        audio_print_adpcm_type(adec_chn_temp->dbg_info.adpcm_type),
                        adec_chn_temp->chn_attr.buf_size,
                        audio_print_g726_bps(adec_chn_temp->dbg_info.g726_rate),
                        audio_print_decode_mode(adec_chn_temp->dbg_info.stream),
                        adec_chn_temp->dbg_info.ori_send_cnt,
                        adec_chn_temp->dbg_info.send_cnt,
                        adec_chn_temp->dbg_info.get_cnt,
                        adec_chn_temp->dbg_info.put_cnt);
    }

    return TD_SUCCESS;
}