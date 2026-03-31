/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2023. All rights reserved.
 * Description: ai proc for V7 platform
 * Author: Hisilicon multimedia software group
 * Create: 2022-09-01
 */

#include "ai_proc.h"
#include "ai_drv_proc.h"
#include "aio_pub.h"
#include "ai_trans.h"

typedef struct {
    td_bool anr_show;
    td_bool agc_show;
    td_bool aec_show;
    td_bool eq_show;
    td_bool hpf_show;
    td_bool rnr_show;
    td_bool hdr_show;
    td_bool drc_show;
    td_bool peq_show;

    /* TalkV2 VQE */
    td_bool talkv2_pnr_show;
    td_bool talkv2_nr_show;
    td_bool talkv2_agc_show;
    td_bool talkv2_eq_show;
    td_bool talkv2_drc_show;
    td_bool talkv2_limiter_show;
    td_bool talkv2_dereverb_show;
    td_bool talkv2_fmp_show;
    td_bool talkv2_aec_show;

    td_bool wnr_show;
} ai_proc_vqe_show_state;

static td_void ai_anr_proc(const struct osal_proc_dir_entry *s, td_u32 *vqe_index)
{
    td_u32 i, j, chn;
    aio_dev_ctx *aio_dev = TD_NULL;
    ai_chn_ctx *chn_ctx = TD_NULL;

    osal_seq_printf(s, "\n-----AI CHN VQE STATUS%u---------------------------------------------------------\n",
                    ++(*vqe_index));
    osal_seq_printf(s, "%5s"    "%6s"    "%8s"    "%8s"    "%8s"    "%12s"    "%11s"    "%9s\n",
                    "AiDev", "AiChn", "State", "bAnr", "bUsrmod", "NrIntensity", "NoiseDbThr", "SpProSwi");
    for (i = 0; i < OT_AI_DEV_MAX_NUM; i++) {
        aio_dev = ai_get_dev_ctx((td_s32)i);
        if (aio_dev->enable == TD_FALSE) {
            continue;
        }

        /* j is user chn id */
        for (j = 0; j < aio_dev->aio_attr.chn_cnt; j++) {
            chn = ai_get_chn_id(i, j);
            chn_ctx = ai_get_chn_ctx((td_s32)chn);
            if (chn_ctx->chn_state != AIO_CHN_ENABLE) {
                continue;
            }

            osal_seq_printf(s, "%5u" "%6u" "%8s", i, j, audio_print_chn_state(chn_ctx->chn_state));
            osal_seq_printf(s, "%8s" "%8s" "%12d" "%11d" "%9d\n",
                            audio_print_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.anr_open),
                            audio_print_vqe_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.anr_cfg.usr_mode),
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.anr_cfg.nr_intensity,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.anr_cfg.noise_db_threshold,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.anr_cfg.music_probe_switch);
        }
    }
}

static td_void ai_agc_proc(const struct osal_proc_dir_entry *s, td_u32 *vqe_index)
{
    td_u32 i, j, chn;
    aio_dev_ctx *aio_dev = TD_NULL;
    ai_chn_ctx *chn_ctx = TD_NULL;

    osal_seq_printf(s, "\n-----AI CHN VQE STATUS%u---------------------------------------------------------\n",
                    ++(*vqe_index));
    osal_seq_printf(s, "%5s" "%6s" "%8s" "%8s" "%8s" "%12s" "%12s" "%11s" "%8s" "%11s" "%11s" "%12s" "%8s\n",
                    "AiDev", "AiChn", "State", "bAgc", "bUsrmod", "NoiseSupSwi", "AdjustSpeed",
                    "ImproveSNR", "MaxGain", "NoiseFloor", "OutputMode", "TargetLevel", "UseHPF");

    for (i = 0; i < OT_AI_DEV_MAX_NUM; i++) {
        aio_dev = ai_get_dev_ctx((td_s32)i);
        if (aio_dev->enable == TD_FALSE) {
            continue;
        }

        /* j is user chn id */
        for (j = 0; j < aio_dev->aio_attr.chn_cnt; j++) {
            chn = ai_get_chn_id(i, j);
            chn_ctx = ai_get_chn_ctx((td_s32)chn);
            if (chn_ctx->chn_state != AIO_CHN_ENABLE) {
                continue;
            }

            osal_seq_printf(s, "%5u" "%6u" "%8s", i, j, audio_print_chn_state(chn_ctx->chn_state));
            osal_seq_printf(s, "%8s" "%8s" "%12d" "%12d" "%11d" "%8d" "%11d" "%11d" "%12d" "%8d\n",
                            audio_print_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.agc_open),
                            audio_print_vqe_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.agc_cfg.usr_mode),
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.agc_cfg.noise_suppress_switch,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.agc_cfg.adjust_speed,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.agc_cfg.improve_snr,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.agc_cfg.max_gain,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.agc_cfg.noise_floor,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.agc_cfg.output_mode,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.agc_cfg.target_level,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.agc_cfg.use_hpf);
        }
    }
}

static td_void ai_eq_proc(const struct osal_proc_dir_entry *s, td_u32 *vqe_index)
{
    td_u32 i, j, chn;
    aio_dev_ctx *aio_dev = TD_NULL;
    ai_chn_ctx *chn_ctx = TD_NULL;

    osal_seq_printf(s, "\n-----AI CHN VQE STATUS%u---------------------------------------------------------\n",
                    ++(*vqe_index));
    osal_seq_printf(s, "%5s" "%6s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s\n",
                    "AiDev", "AiChn", "State", "bEq",
                    "100", "200", "250", "350", "500", "800", "1.2k", "2.5k", "4k", "8k");

    for (i = 0; i < OT_AI_DEV_MAX_NUM; i++) {
        aio_dev = ai_get_dev_ctx((td_s32)i);
        if (aio_dev->enable == TD_FALSE) {
            continue;
        }

        /* j is user chn id */
        for (j = 0; j < aio_dev->aio_attr.chn_cnt; j++) {
            chn = ai_get_chn_id(i, j);
            chn_ctx = ai_get_chn_ctx((td_s32)chn);
            if (chn_ctx->chn_state != AIO_CHN_ENABLE) {
                continue;
            }
            osal_seq_printf(s, "%5u" "%6u" "%8s", i, j, audio_print_chn_state(chn_ctx->chn_state));
            osal_seq_printf(s, "%8s" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d\n",
                            audio_print_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.eq_open),
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.eq_cfg.gain_db[0],
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.eq_cfg.gain_db[1],
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.eq_cfg.gain_db[2], /* 2:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.eq_cfg.gain_db[3], /* 3:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.eq_cfg.gain_db[4], /* 4:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.eq_cfg.gain_db[5], /* 5:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.eq_cfg.gain_db[6], /* 6:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.eq_cfg.gain_db[7], /* 7:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.eq_cfg.gain_db[8], /* 8:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.eq_cfg.gain_db[9]); /* 9:index */
        }
    }
}

static td_void ai_hpf_proc(const struct osal_proc_dir_entry *s, td_u32 *vqe_index)
{
    td_u32 i, j, chn;
    aio_dev_ctx *aio_dev = TD_NULL;
    ai_chn_ctx *chn_ctx = TD_NULL;

    osal_seq_printf(s, "\n-----AI CHN VQE STATUS%u---------------------------------------------------------\n",
                    ++(*vqe_index));
    osal_seq_printf(s, "%5s" "%6s" "%8s" "%8s" "%8s" "%8s\n",
                    "AiDev", "AiChn", "State", "bHpf", "bUsrmod", "HpfFreq");
    for (i = 0; i < OT_AI_DEV_MAX_NUM; i++) {
        aio_dev = ai_get_dev_ctx((td_s32)i);
        if (aio_dev->enable == TD_FALSE) {
            continue;
        }
        /* j is user chn id */
        for (j = 0; j < aio_dev->aio_attr.chn_cnt; j++) {
            chn = ai_get_chn_id(i, j);
            chn_ctx = ai_get_chn_ctx((td_s32)chn);
            if (chn_ctx->chn_state != AIO_CHN_ENABLE) {
                continue;
            }

            osal_seq_printf(s, "%5u" "%6u" "%8s", i, j, audio_print_chn_state(chn_ctx->chn_state));
            osal_seq_printf(s, "%8s" "%8s" "%8u\n",
                            audio_print_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.hpf_open),
                            audio_print_vqe_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.hpf_cfg.usr_mode),
                            (td_u32)chn_ctx->ai_vqe_dbg.ai_vqe_cfg.hpf_cfg.hpf_freq);
        }
    }
}

static td_void ai_rnr_proc(const struct osal_proc_dir_entry *s, td_u32 *vqe_index)
{
    td_u32 i, j, chn;
    aio_dev_ctx *aio_dev = TD_NULL;
    ai_chn_ctx *chn_ctx = TD_NULL;

    osal_seq_printf(s, "\n-----AI CHN VQE STATUS%u---------------------------------------------------------\n",
                    ++(*vqe_index));
    osal_seq_printf(s, "%5s" "%6s" "%8s" "%8s" "%8s" "%11s" "%10s" "%8s\n",
                    "AiDev", "AiChn", "State", "bRnr", "bUsrmod", "MaxNrLevel", "NsThresh", "NrMode");

    for (i = 0; i < OT_AI_DEV_MAX_NUM; i++) {
        aio_dev = ai_get_dev_ctx((td_s32)i);
        if (aio_dev->enable == TD_FALSE) {
            continue;
        }

        /* j is user chn id */
        for (j = 0; j < aio_dev->aio_attr.chn_cnt; j++) {
            chn = ai_get_chn_id(i, j);
            chn_ctx = ai_get_chn_ctx((td_s32)chn);
            if (chn_ctx->chn_state != AIO_CHN_ENABLE) {
                continue;
            }

            osal_seq_printf(s, "%5u" "%6u" "%8s", i, j, audio_print_chn_state(chn_ctx->chn_state));
            osal_seq_printf(s, "%8s" "%8s" "%11d" "%10d" "%8d\n",
                            audio_print_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.rnr_open),
                            audio_print_vqe_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.rnr_cfg.usr_mode),
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.rnr_cfg.max_nr_level,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.rnr_cfg.noise_threshold,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.rnr_cfg.nr_mode);
        }
    }
}

static td_void ai_aec_base_proc(const struct osal_proc_dir_entry *s, td_u32 *vqe_index)
{
    td_u32 i, j, chn;
    aio_dev_ctx *aio_dev = TD_NULL;
    ai_chn_ctx *chn_ctx = TD_NULL;

    osal_seq_printf(s, "\n-----AI CHN VQE STATUS%u---------------------------------------------------------\n",
                    ++(*vqe_index));
    osal_seq_printf(s, "%5s" "%6s" "%8s" "%8s" "%8s" "%8s" "%11s" "%11s" "%13s" "%8s" "%8s\n",
                    "AiDev", "AiChn", "State", "bAec", "bUsrmod", "CngMode",
                    "DTHnlStQTh", "NrAlPsEngy", "NrClnSupEngy", "AecAo", "AecFail");

    for (i = 0; i < OT_AI_DEV_MAX_NUM; i++) {
        aio_dev = ai_get_dev_ctx((td_s32)i);
        if (aio_dev->enable == TD_FALSE) {
            continue;
        }
        /* j is user chn id */
        for (j = 0; j < aio_dev->aio_attr.chn_cnt; j++) {
            chn = ai_get_chn_id(i, j);
            chn_ctx = ai_get_chn_ctx((td_s32)chn);
            if (chn_ctx->chn_state != AIO_CHN_ENABLE) {
                continue;
            }

            osal_seq_printf(s, "%5u" "%6u" "%8s", i, j, audio_print_chn_state(chn_ctx->chn_state));
            osal_seq_printf(s, "%8s" "%8s" "%8s" "%11d" "%11d" "%13d" " (%2d,%2d)" "%8u\n",
                            audio_print_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.aec_open),
                            audio_print_vqe_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.aec_cfg.usr_mode),
                            audio_print_aec_mode(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.aec_cfg.cozy_noisy_mode),
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.aec_cfg.double_talk_threshold,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.aec_cfg.near_all_pass_energy,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.aec_cfg.near_clean_sup_energy,
                            chn_ctx->aec_ao_dev, chn_ctx->aec_ao_chn,
                            chn_ctx->aec_fail);
        }
    }
}

static td_void ai_aec_echo_band_proc(const struct osal_proc_dir_entry *s, td_u32 *vqe_index)
{
    td_u32 i, j, chn;
    aio_dev_ctx *aio_dev = TD_NULL;
    ai_chn_ctx *chn_ctx = TD_NULL;

    /* 通道状态--------------------------------------------------------------- */
    osal_seq_printf(s, "\n-----AI CHN VQE STATUS%u---------------------------------------------------------\n",
                    ++(*vqe_index));
    osal_seq_printf(s, "%5s" "%6s" "%8s" "%8s" "%8s" "%12s" "%13s" "%10s" "%10s" "%11s" "%11s\n",
                    "AiDev", "AiChn", "State", "bAec", "bUsrmod", "VcPrtctFrqL",
                    "VcPrtctFrqL1", "EcoBndLow", "EcoBndHgh", "EcoBndLow2", "EcoBndHgh2");

    for (i = 0; i < OT_AI_DEV_MAX_NUM; i++) {
        aio_dev = ai_get_dev_ctx((td_s32)i);
        if (aio_dev->enable == TD_FALSE) {
            continue;
        }
        /* j is user chn id */
        for (j = 0; j < aio_dev->aio_attr.chn_cnt; j++) {
            chn = ai_get_chn_id(i, j);
            chn_ctx = ai_get_chn_ctx((td_s32)chn);
            if (chn_ctx->chn_state != AIO_CHN_ENABLE) {
                continue;
            }

            osal_seq_printf(s, "%5u" "%6u" "%8s", i, j, audio_print_chn_state(chn_ctx->chn_state));
            osal_seq_printf(s, "%8s" "%8s" "%12d" "%13d" "%10d" "%10d" "%11d" "%11d\n",
                            audio_print_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.aec_open),
                            audio_print_vqe_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.aec_cfg.usr_mode),
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.aec_cfg.voice_protect_freq_l,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.aec_cfg.voice_protect_freq_l1,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.aec_cfg.echo_band_low,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.aec_cfg.echo_band_high,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.aec_cfg.echo_band_low2,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.aec_cfg.echo_band_high2);
        }
    }
}

static td_void ai_aec_erl_band_proc(const struct osal_proc_dir_entry *s, td_u32 *vqe_index)
{
    td_u32 i, j, chn;
    aio_dev_ctx *aio_dev = TD_NULL;
    ai_chn_ctx *chn_ctx = TD_NULL;

    /* 通道状态--------------------------------------------------------------- */
    osal_seq_printf(s, "\n-----AI CHN VQE STATUS%u---------------------------------------------------------\n",
                    ++(*vqe_index));
    osal_seq_printf(s, "%5s" "%6s" "%8s" "%8s" "%8s" "%11s" "%11s" "%11s" "%11s" "%11s" "%11s\n",
                    "AiDev", "AiChn", "State", "bAec", "bUsrmod",
                    "ERLBND[0]", "ERLBND[1]", "ERLBND[2]", "ERLBND[3]", "ERLBND[4]", "ERLBND[5]");

    for (i = 0; i < OT_AI_DEV_MAX_NUM; i++) {
        aio_dev = ai_get_dev_ctx((td_s32)i);
        if (aio_dev->enable == TD_FALSE) {
            continue;
        }
        /* j is user chn id */
        for (j = 0; j < aio_dev->aio_attr.chn_cnt; j++) {
            chn = ai_get_chn_id(i, j);
            chn_ctx = ai_get_chn_ctx((td_s32)chn);
            if (chn_ctx->chn_state != AIO_CHN_ENABLE) {
                continue;
            }

            osal_seq_printf(s, "%5u" "%6u" "%8s", i, j, audio_print_chn_state(chn_ctx->chn_state));
            osal_seq_printf(s, "%8s" "%8s" "%11d" "%11d" "%11d" "%11d" "%11d" "%11d\n",
                            audio_print_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.aec_open),
                            audio_print_vqe_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.aec_cfg.usr_mode),
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.aec_cfg.erl_band[0],
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.aec_cfg.erl_band[1],
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.aec_cfg.erl_band[2], /* 2:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.aec_cfg.erl_band[3], /* 3:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.aec_cfg.erl_band[4], /* 4:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.aec_cfg.erl_band[5]); /* 5:index */
        }
    }
}

static td_void ai_aec_erl_proc(const struct osal_proc_dir_entry *s, td_u32 *vqe_index)
{
    td_u32 i, j, chn;
    aio_dev_ctx *aio_dev = TD_NULL;
    ai_chn_ctx *chn_ctx = TD_NULL;

    /* 通道状态--------------------------------------------------------------- */
    osal_seq_printf(s, "\n-----AI CHN VQE STATUS%u---------------------------------------------------------\n",
                    ++(*vqe_index));
    osal_seq_printf(s, "%5s" "%6s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s\n",
                    "AiDev", "AiChn", "State", "bAec", "bUsrmod",
                    "ERL[0]", "ERL[1]", "ERL[2]", "ERL[3]", "ERL[4]", "ERL[5]", "ERL[6]");

    for (i = 0; i < OT_AI_DEV_MAX_NUM; i++) {
        aio_dev = ai_get_dev_ctx((td_s32)i);
        if (aio_dev->enable == TD_FALSE) {
            continue;
        }
        /* j is user chn id */
        for (j = 0; j < aio_dev->aio_attr.chn_cnt; j++) {
            chn = ai_get_chn_id(i, j);
            chn_ctx = ai_get_chn_ctx((td_s32)chn);
            if (chn_ctx->chn_state != AIO_CHN_ENABLE) {
                continue;
            }

            osal_seq_printf(s, "%5u" "%6u" "%8s", i, j, audio_print_chn_state(chn_ctx->chn_state));
            osal_seq_printf(s, "%8s" "%8s" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d\n",
                            audio_print_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.aec_open),
                            audio_print_vqe_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.aec_cfg.usr_mode),
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.aec_cfg.erl[0],
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.aec_cfg.erl[1],
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.aec_cfg.erl[2], /* 2:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.aec_cfg.erl[3], /* 3:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.aec_cfg.erl[4], /* 4:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.aec_cfg.erl[5], /* 5:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.aec_cfg.erl[6]); /* 6:index */
        }
    }
}

static td_void ai_aec_proc(const struct osal_proc_dir_entry *s, td_u32 *vqe_index)
{
    /* AEC基本参数 */
    ai_aec_base_proc(s, vqe_index);

    /* Echo Band参数 */
    ai_aec_echo_band_proc(s, vqe_index);

    /* ERL Band参数 */
    ai_aec_erl_band_proc(s, vqe_index);

    /* ERL参数 */
    ai_aec_erl_proc(s, vqe_index);
}

static td_void ai_drc_base_proc(const struct osal_proc_dir_entry *s, td_u32 *vqe_index)
{
    td_u32 i, j, chn;
    aio_dev_ctx *aio_dev = TD_NULL;
    ai_chn_ctx *chn_ctx = TD_NULL;

    osal_seq_printf(s, "\n-----AI CHN VQE STATUS%u---------------------------------------------------------\n",
                    ++(*vqe_index));
    osal_seq_printf(s, "%5s" "%6s" "%8s" "%8s" "%8s" "%8s" "%8s\n",
                    "AiDev", "AiChn", "State", "bDrc", "bUsrmod", "AtkTime", "RlsTime");

    for (i = 0; i < OT_AI_DEV_MAX_NUM; i++) {
        aio_dev = ai_get_dev_ctx((td_s32)i);
        if (aio_dev->enable == TD_FALSE) {
            continue;
        }
        /* j is user chn id */
        for (j = 0; j < aio_dev->aio_attr.chn_cnt; j++) {
            chn = ai_get_chn_id(i, j);
            chn_ctx = ai_get_chn_ctx((td_s32)chn);
            if (chn_ctx->chn_state != AIO_CHN_ENABLE) {
                continue;
            }

            osal_seq_printf(s, "%5u" "%6u" "%8s", i, j, audio_print_chn_state(chn_ctx->chn_state));
            osal_seq_printf(s, "%8s" "%8s" "%8d" "%8d\n",
                            audio_print_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.drc_open),
                            audio_print_vqe_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.drc_cfg.usr_mode),
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.drc_cfg.attack_time,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.drc_cfg.release_time);
        }
    }
}

static td_void ai_drc_old_level_proc(const struct osal_proc_dir_entry *s, td_u32 *vqe_index)
{
    td_u32 i, j, chn;
    aio_dev_ctx *aio_dev = TD_NULL;
    ai_chn_ctx *chn_ctx_info = TD_NULL;

    osal_seq_printf(s, "\n-----AI CHN VQE STATUS%u---------------------------------------------------------\n",
                    ++(*vqe_index));
    osal_seq_printf(s, "%5s" "%6s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s\n",
                    "AiDev", "AiChn", "State", "bDrc", "bUsrmod",
                    "OldLev0", "OldLev1", "OldLev2", "OldLev3", "OldLev4");

    for (i = 0; i < OT_AI_DEV_MAX_NUM; i++) {
        aio_dev = ai_get_dev_ctx((td_s32)i);
        if (aio_dev->enable == TD_FALSE) {
            continue;
        }
        /* j is user chn id */
        for (j = 0; j < aio_dev->aio_attr.chn_cnt; j++) {
            chn = ai_get_chn_id(i, j);
            chn_ctx_info = ai_get_chn_ctx((td_s32)chn);
            if (chn_ctx_info->chn_state != AIO_CHN_ENABLE) {
                continue;
            }

            osal_seq_printf(s, "%5u" "%6u" "%8s", i, j, audio_print_chn_state(chn_ctx_info->chn_state));
            osal_seq_printf(s, "%8s" "%8s" "%8d" "%8d" "%8d" "%8d" "%8d\n",
                            audio_print_state(chn_ctx_info->ai_vqe_dbg.ai_vqe_cfg.drc_open),
                            audio_print_vqe_state(chn_ctx_info->ai_vqe_dbg.ai_vqe_cfg.drc_cfg.usr_mode),
                            chn_ctx_info->ai_vqe_dbg.ai_vqe_cfg.drc_cfg.old_level_db[0],
                            chn_ctx_info->ai_vqe_dbg.ai_vqe_cfg.drc_cfg.old_level_db[1],
                            chn_ctx_info->ai_vqe_dbg.ai_vqe_cfg.drc_cfg.old_level_db[2], /* 2:index */
                            chn_ctx_info->ai_vqe_dbg.ai_vqe_cfg.drc_cfg.old_level_db[3], /* 3:index */
                            chn_ctx_info->ai_vqe_dbg.ai_vqe_cfg.drc_cfg.old_level_db[4]); /* 4:index */
        }
    }
}

static td_void ai_drc_new_level_proc(const struct osal_proc_dir_entry *s, td_u32 *vqe_index)
{
    td_u32 i, j, chn;
    aio_dev_ctx *aio_dev = TD_NULL;
    ai_chn_ctx *chn_ctx = TD_NULL;

    osal_seq_printf(s, "\n-----AI CHN VQE STATUS%u---------------------------------------------------------\n",
                    ++(*vqe_index));
    osal_seq_printf(s, "%5s" "%6s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s\n",
                    "AiDev", "AiChn", "State", "bDrc", "bUsrmod",
                    "NewLev0", "NewLev1", "NewLev2", "NewLev3", "NewLev4");

    for (i = 0; i < OT_AI_DEV_MAX_NUM; i++) {
        aio_dev = ai_get_dev_ctx((td_s32)i);
        if (aio_dev->enable == TD_FALSE) {
            continue;
        }
        /* j is user chn id */
        for (j = 0; j < aio_dev->aio_attr.chn_cnt; j++) {
            chn = ai_get_chn_id(i, j);
            chn_ctx = ai_get_chn_ctx((td_s32)chn);
            if (chn_ctx->chn_state != AIO_CHN_ENABLE) {
                continue;
            }

            osal_seq_printf(s, "%5u" "%6u" "%8s", i, j, audio_print_chn_state(chn_ctx->chn_state));
            osal_seq_printf(s, "%8s" "%8s" "%8d" "%8d" "%8d" "%8d" "%8d\n",
                            audio_print_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.drc_open),
                            audio_print_vqe_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.drc_cfg.usr_mode),
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.drc_cfg.new_level_db[0],
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.drc_cfg.new_level_db[1],
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.drc_cfg.new_level_db[2], /* 2:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.drc_cfg.new_level_db[3], /* 3:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.drc_cfg.new_level_db[4]); /* 4:index */
        }
    }
}

static td_void ai_drc_proc(const struct osal_proc_dir_entry *s, td_u32 *vqe_index)
{
    /* DRC基本参数 */
    ai_drc_base_proc(s, vqe_index);

    /* Old Level参数 */
    ai_drc_old_level_proc(s, vqe_index);

    /* New Level参数 */
    ai_drc_new_level_proc(s, vqe_index);
}

static td_void ai_peq_filter_type_proc(const struct osal_proc_dir_entry *s, td_u32 *vqe_index)
{
    td_u32 i, j, chn;
    aio_dev_ctx *aio_dev = TD_NULL;
    ai_chn_ctx *chn_ctx = TD_NULL;

    osal_seq_printf(s, "\n-----AI CHN VQE STATUS%u---------------------------------------------------------\n",
                    ++(*vqe_index));
    osal_seq_printf(s, "%5s" "%6s" "%8s" "%8s" "%8s" "%8s"
                    "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s\n",
                    "AiDev", "AiChn", "State", "bPeq", "bUsrmod", "BandNum",
                    "Filter0", "Filter1", "Filter2", "Filter3", "Filter4",
                    "Filter5", "Filter6", "Filter7", "Filter8", "Filter9");

    for (i = 0; i < OT_AI_DEV_MAX_NUM; i++) {
        aio_dev = ai_get_dev_ctx((td_s32)i);
        if (aio_dev->enable == TD_FALSE) {
            continue;
        }
        /* j is user chn id */
        for (j = 0; j < aio_dev->aio_attr.chn_cnt; j++) {
            chn = ai_get_chn_id(i, j);
            chn_ctx = ai_get_chn_ctx((td_s32)chn);
            if (chn_ctx->chn_state != AIO_CHN_ENABLE) {
                continue;
            }

            osal_seq_printf(s, "ai_peq_filter_type_proc %5u" "%6u" "%8s",
                            i, j, audio_print_chn_state(chn_ctx->chn_state));
            osal_seq_printf(s, "%8s" "%8s" "%8u" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d\n",
                            audio_print_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_open),
                            audio_print_vqe_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.usr_mode),
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.band_num,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.filter_type[0],
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.filter_type[1],
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.filter_type[2], /* 2:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.filter_type[3], /* 3:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.filter_type[4], /* 4:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.filter_type[5], /* 5:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.filter_type[6], /* 6:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.filter_type[7], /* 7:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.filter_type[8], /* 8:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.filter_type[9]); /* 9:index */
        }
    }
}

static td_void ai_peq_gain_proc(const struct osal_proc_dir_entry *s, td_u32 *vqe_index)
{
    td_u32 i, j, chn;
    aio_dev_ctx *aio_dev = TD_NULL;
    ai_chn_ctx *chn_ctx = TD_NULL;

    /* 通道状态--------------------------------------------------------------- */
    osal_seq_printf(s, "\n-----AI CHN VQE STATUS%u---------------------------------------------------------\n",
                    ++(*vqe_index));
    osal_seq_printf(s, "%5s" "%6s" "%8s" "%8s" "%8s" "%8s"
                    "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s\n",
                    "AiDev", "AiChn", "State", "bPeq", "bUsrmod", "BandNum",
                    "GaindB0", "GaindB1", "GaindB2", "GaindB3", "GaindB4",
                    "GaindB5", "GaindB6", "GaindB7", "GaindB8", "GaindB9");

    for (i = 0; i < OT_AI_DEV_MAX_NUM; i++) {
        aio_dev = ai_get_dev_ctx((td_s32)i);
        if (aio_dev->enable == TD_FALSE) {
            continue;
        }

        /* j is user chn id */
        for (j = 0; j < aio_dev->aio_attr.chn_cnt; j++) {
            chn = ai_get_chn_id(i, j);
            chn_ctx = ai_get_chn_ctx((td_s32)chn);
            if (chn_ctx->chn_state != AIO_CHN_ENABLE) {
                continue;
            }

            osal_seq_printf(s, "ai_peq_gain_proc %5u" "%6u" "%8s", i, j, audio_print_chn_state(chn_ctx->chn_state));
            osal_seq_printf(s, "%8s" "%8s" "%8u" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d\n",
                            audio_print_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_open),
                            audio_print_vqe_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.usr_mode),
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.band_num,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.gain_db[0],
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.gain_db[1],
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.gain_db[2], /* 2:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.gain_db[3], /* 3:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.gain_db[4], /* 4:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.gain_db[5], /* 5:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.gain_db[6], /* 6:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.gain_db[7], /* 7:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.gain_db[8], /* 8:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.gain_db[9]); /* 9:index */
        }
    }
}

static td_void ai_peq_frequency_proc(const struct osal_proc_dir_entry *s, td_u32 *vqe_index)
{
    td_u32 i, j, chn;
    aio_dev_ctx *aio_dev = TD_NULL;
    ai_chn_ctx *chn_ctx = TD_NULL;

    /* 通道状态--------------------------------------------------------------- */
    osal_seq_printf(s, "\n-----AI CHN VQE STATUS%u---------------------------------------------------------\n",
                    ++(*vqe_index));
    osal_seq_printf(s, "%5s" "%6s" "%8s" "%8s" "%8s" "%8s"
                    "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s\n",
                    "AiDev", "AiChn", "State", "bPeq", "bUsrmod", "BandNum",
                    "Freq0", "Freq1", "Freq2", "Freq3", "Freq4", "Freq5", "Freq6", "Freq7", "Freq8", "Freq9");

    for (i = 0; i < OT_AI_DEV_MAX_NUM; i++) {
        aio_dev = ai_get_dev_ctx((td_s32)i);
        if (aio_dev->enable == TD_FALSE) {
            continue;
        }
        /* j is user chn id */
        for (j = 0; j < aio_dev->aio_attr.chn_cnt; j++) {
            chn = ai_get_chn_id(i, j);
            chn_ctx = ai_get_chn_ctx((td_s32)chn);
            if (chn_ctx->chn_state != AIO_CHN_ENABLE) {
                continue;
            }

            osal_seq_printf(s, "ai_peq_frequency_proc %5u" "%6u" "%8s",
                            i, j, audio_print_chn_state(chn_ctx->chn_state));
            osal_seq_printf(s, "%8s" "%8s" "%8u" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d\n",
                            audio_print_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_open),
                            audio_print_vqe_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.usr_mode),
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.band_num,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.frequency[0],
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.frequency[1],
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.frequency[2], /* 2:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.frequency[3], /* 3:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.frequency[4], /* 4:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.frequency[5], /* 5:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.frequency[6], /* 6:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.frequency[7], /* 7:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.frequency[8], /* 8:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.frequency[9]); /* 9:index */
        }
    }
}

static td_void ai_peq_q_proc(const struct osal_proc_dir_entry *s, td_u32 *vqe_index)
{
    td_u32 i, j, chn;
    aio_dev_ctx *aio_dev = TD_NULL;
    ai_chn_ctx *chn_ctx = TD_NULL;

    /* 通道状态--------------------------------------------------------------- */
    osal_seq_printf(s, "\n-----AI CHN VQE STATUS%u---------------------------------------------------------\n",
                    ++(*vqe_index));
    osal_seq_printf(s, "%5s" "%6s" "%8s" "%8s" "%8s" "%8s"
                    "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s\n",
                    "AiDev", "AiChn", "State", "bPeq", "bUsrmod", "BandNum",
                    "Q0", "Q1", "Q2", "Q3", "Q4", "Q5", "Q6", "Q7", "Q8", "Q9");

    for (i = 0; i < OT_AI_DEV_MAX_NUM; i++) {
        aio_dev = ai_get_dev_ctx((td_s32)i);
        if (aio_dev->enable == TD_FALSE) {
            continue;
        }
        /* j is user chn id */
        for (j = 0; j < aio_dev->aio_attr.chn_cnt; j++) {
            chn = ai_get_chn_id(i, j);
            chn_ctx = ai_get_chn_ctx((td_s32)chn);
            if (chn_ctx->chn_state != AIO_CHN_ENABLE) {
                continue;
            }

            osal_seq_printf(s, "ai_peq_q_proc %5u" "%6u" "%8s", i, j, audio_print_chn_state(chn_ctx->chn_state));
            osal_seq_printf(s, "%8s" "%8s" "%8u" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d\n",
                            audio_print_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_open),
                            audio_print_vqe_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.usr_mode),
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.band_num,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.q[0],
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.q[1],
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.q[2], /* 2:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.q[3], /* 3:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.q[4], /* 4:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.q[5], /* 5:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.q[6], /* 6:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.q[7], /* 7:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.q[8], /* 8:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.peq_cfg.q[9]); /* 9:index */
        }
    }
}

static td_void ai_peq_proc(const struct osal_proc_dir_entry *s, td_u32 *vqe_index)
{
    /* PEQ Filter Type参数 */
    ai_peq_filter_type_proc(s, vqe_index);

    /* PEQ Gain参数 */
    ai_peq_gain_proc(s, vqe_index);

    /* PEQ Frequency参数 */
    ai_peq_frequency_proc(s, vqe_index);

    /* PEQ Q值参数 */
    ai_peq_q_proc(s, vqe_index);
}

static td_void ai_hdr_proc(const struct osal_proc_dir_entry *s, td_u32 *vqe_index)
{
    td_u32 i, j, chn;
    aio_dev_ctx *aio_dev = TD_NULL;
    ai_chn_ctx *chn_ctx = TD_NULL;

    osal_seq_printf(s, "\n-----AI CHN VQE STATUS%u---------------------------------------------------------\n",
        ++(*vqe_index));
    osal_seq_printf(s, "%5s" "%6s" "%8s" "%8s" "%8s" "%8s" "%8s" "%10s" "%8s" "%11s\n",
                    "AiDev", "AiChn", "State", "bHdr", "bUsrmod",
                    "MaxGain", "MinGain", "MicGStep", "MicGain", "pCallBack");

    for (i = 0; i < OT_AI_DEV_MAX_NUM; i++) {
        aio_dev = ai_get_dev_ctx((td_s32)i);
        if (aio_dev->enable == TD_FALSE) {
            continue;
        }
        /* j is user chn id */
        for (j = 0; j < aio_dev->aio_attr.chn_cnt; j++) {
            chn = ai_get_chn_id(i, j);
            chn_ctx = ai_get_chn_ctx((td_s32)chn);
            if (chn_ctx->chn_state != AIO_CHN_ENABLE) {
                continue;
            }

            osal_seq_printf(s, "%5u" "%6u" "%8s", i, j, audio_print_chn_state(chn_ctx->chn_state));
            osal_seq_printf(s, "%8s" "%8s" "%8d" "%8d" "%10d" "%8d \n",
                            audio_print_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.hdr_open),
                            audio_print_vqe_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.hdr_cfg.usr_mode),
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.hdr_cfg.max_gain_db,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.hdr_cfg.min_gain_db,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.hdr_cfg.mic_gain_step_db,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.hdr_cfg.mic_gain_db);
        }
    }
}

static td_void ai_talkv2_pnr_base_proc(const struct osal_proc_dir_entry *s, td_u32 *vqe_index)
{
    td_u32 i, j, chn;
    aio_dev_ctx *aio_dev = TD_NULL;
    ai_chn_ctx *chn_ctx = TD_NULL;

    osal_seq_printf(s, "\n-----AI CHN VQE STATUS%u---------------------------------------------------------\n",
                    ++(*vqe_index));
    osal_seq_printf(s, "%5s" "%6s" "%8s" "%8s" "%8s" "%13s" "%14s" "%8s" "%8s" "%8s" "%9s\n",
                    "AiDev", "AiChn", "State", "bPnr", "bUsrmod", "MinGainLimit", "SnrPriorLimit", "HtThre",
                    "HsThre", "AlphaPh", "AlphaPsd");

    for (i = 0; i < OT_AI_DEV_MAX_NUM; i++) {
        aio_dev = ai_get_dev_ctx((td_s32)i);
        if (aio_dev->enable == TD_FALSE) {
            continue;
        }

        /* j is user chn id */
        for (j = 0; j < aio_dev->aio_attr.chn_cnt; j++) {
            chn = ai_get_chn_id(i, j);
            chn_ctx = ai_get_chn_ctx((td_s32)chn);
            if (chn_ctx->chn_state != AIO_CHN_ENABLE) {
                continue;
            }

            osal_seq_printf(s, "%5u" "%6u" "%8s", i, j, audio_print_chn_state(chn_ctx->chn_state));
            osal_seq_printf(s, "%8s" "%8s" "%13d" "%14d" "%8d" "%8d" "%8d" "%9d\n",
                            audio_print_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_pnr_open),
                            audio_print_vqe_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_pnr_cfg.usr_mode),
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_pnr_cfg.min_gain_limit,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_pnr_cfg.snr_prior_limit,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_pnr_cfg.ht_threshold,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_pnr_cfg.hs_threshold,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_pnr_cfg.alpha_ph,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_pnr_cfg.alpha_psd);
        }
    }
}

static td_void ai_talkv2_pnr_extern_proc(const struct osal_proc_dir_entry *s, td_u32 *vqe_index)
{
    td_u32 i, j, chn;
    aio_dev_ctx *aio_dev = TD_NULL;
    ai_chn_ctx *chn_ctx = TD_NULL;

    osal_seq_printf(s, "\n-----AI CHN VQE STATUS%u---------------------------------------------------------\n",
                    ++(*vqe_index));
    osal_seq_printf(s, "%5s" "%6s" "%8s" "%8s" "%8s" "%14s" "%8s" "%18s" "%10s" "%10s\n",
                    "AiDev", "AiChn", "State", "bPnr", "bUsrmod", "PriorSnrFixed", "CepThre", "SpeechProtectThre",
                    "HemEnable", "TcsEnable");

    for (i = 0; i < OT_AI_DEV_MAX_NUM; i++) {
        aio_dev = ai_get_dev_ctx((td_s32)i);
        if (aio_dev->enable == TD_FALSE) {
            continue;
        }

        /* j is user chn id */
        for (j = 0; j < aio_dev->aio_attr.chn_cnt; j++) {
            chn = ai_get_chn_id(i, j);
            chn_ctx = ai_get_chn_ctx((td_s32)chn);
            if (chn_ctx->chn_state != AIO_CHN_ENABLE) {
                continue;
            }

            osal_seq_printf(s, "%5u" "%6u" "%8s", i, j, audio_print_chn_state(chn_ctx->chn_state));
            osal_seq_printf(s, "%8s" "%8s" "%14d" "%8d" "%18d" "%10d" "%10d\n",
                            audio_print_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_pnr_open),
                            audio_print_vqe_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_pnr_cfg.usr_mode),
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_pnr_cfg.prior_snr_fixed,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_pnr_cfg.cep_threshold,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_pnr_cfg.speech_protect_threshold,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_pnr_cfg.hem_enable,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_pnr_cfg.tcs_enable);
        }
    }
}

static td_void ai_talkv2_pnr_proc(const struct osal_proc_dir_entry *s, td_u32 *vqe_index)
{
    ai_talkv2_pnr_base_proc(s, vqe_index);
    ai_talkv2_pnr_extern_proc(s, vqe_index);
}

static td_void ai_talkv2_nr_base_proc(const struct osal_proc_dir_entry *s, td_u32 *vqe_index)
{
    td_u32 i, j, chn;
    aio_dev_ctx *aio_dev = TD_NULL;
    ai_chn_ctx *chn_ctx = TD_NULL;

    osal_seq_printf(s, "\n-----AI CHN VQE STATUS%u---------------------------------------------------------\n",
                    ++(*vqe_index));
    osal_seq_printf(s, "%5s" "%6s" "%8s" "%8s" "%8s" "%13s" "%14s" "%8s" "%8s\n",
                    "AiDev", "AiChn", "State", "bNr", "bUsrmod", "MinGainLimit", "SnrPriorLimit", "HtThre", "HsThre");

    for (i = 0; i < OT_AI_DEV_MAX_NUM; i++) {
        aio_dev = ai_get_dev_ctx((td_s32)i);
        if (aio_dev->enable == TD_FALSE) {
            continue;
        }

        /* j is user chn id */
        for (j = 0; j < aio_dev->aio_attr.chn_cnt; j++) {
            chn = ai_get_chn_id(i, j);
            chn_ctx = ai_get_chn_ctx((td_s32)chn);
            if (chn_ctx->chn_state != AIO_CHN_ENABLE) {
                continue;
            }

            osal_seq_printf(s, "%5u" "%6u" "%8s", i, j, audio_print_chn_state(chn_ctx->chn_state));
            osal_seq_printf(s, "%8s" "%8s" "%13d" "%14d" "%8d" "%8d\n",
                            audio_print_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_nr_open),
                            audio_print_vqe_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_nr_cfg.usr_mode),
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_nr_cfg.min_gain_limit,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_nr_cfg.snr_prior_limit,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_nr_cfg.ht_threshold,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_nr_cfg.hs_threshold);
        }
    }
}

static td_void ai_talkv2_nr_extern_proc(const struct osal_proc_dir_entry *s, td_u32 *vqe_index)
{
    td_u32 i, j, chn;
    aio_dev_ctx *aio_dev = TD_NULL;
    ai_chn_ctx *chn_ctx = TD_NULL;

    osal_seq_printf(s, "\n-----AI CHN VQE STATUS%u---------------------------------------------------------\n",
                    ++(*vqe_index));
    osal_seq_printf(s, "%5s" "%6s" "%8s" "%8s" "%8s" "%17s" "%14s" "%12s" "%14s\n",
                    "AiDev", "AiChn", "State", "bNr", "bUsrmod", "PriNoiseEstiMode", "PriorSnrFixed", "PrioSNRThre",
                    "SmPrioSNRThre");

    for (i = 0; i < OT_AI_DEV_MAX_NUM; i++) {
        aio_dev = ai_get_dev_ctx((td_s32)i);
        if (aio_dev->enable == TD_FALSE) {
            continue;
        }

        /* j is user chn id */
        for (j = 0; j < aio_dev->aio_attr.chn_cnt; j++) {
            chn = ai_get_chn_id(i, j);
            chn_ctx = ai_get_chn_ctx((td_s32)chn);
            if (chn_ctx->chn_state != AIO_CHN_ENABLE) {
                continue;
            }

            osal_seq_printf(s, "%5u" "%6u" "%8s", i, j, audio_print_chn_state(chn_ctx->chn_state));
            osal_seq_printf(s, "%8s" "%8s" "%17d" "%14d" "%12d" "%14d\n",
                            audio_print_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_nr_open),
                            audio_print_vqe_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_nr_cfg.usr_mode),
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_nr_cfg.pri_noise_esti_mode,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_nr_cfg.prior_snr_fixed,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_nr_cfg.prio_snr_threshold,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_nr_cfg.sm_prio_snr_threshold);
        }
    }
}

static td_void ai_talkv2_nr_proc(const struct osal_proc_dir_entry *s, td_u32 *vqe_index)
{
    ai_talkv2_nr_base_proc(s, vqe_index);
    ai_talkv2_nr_extern_proc(s, vqe_index);
}

static td_void ai_talkv2_agc_proc(const struct osal_proc_dir_entry *s, td_u32 *vqe_index)
{
    td_u32 i, j, chn;
    aio_dev_ctx *aio_dev = TD_NULL;
    ai_chn_ctx *chn_ctx = TD_NULL;

    osal_seq_printf(s, "\n-----AI CHN VQE STATUS%u---------------------------------------------------------\n",
                    ++(*vqe_index));
    osal_seq_printf(s, "%5s" "%6s" "%8s" "%8s" "%8s" "%12s" "%8s" "%8s" "%12s\n",
                    "AiDev", "AiChn", "State", "bAgcV2", "bUsrmod", "TargetLevel", "MaxGain", "MinGain", "AdjustSpeed");

    for (i = 0; i < OT_AI_DEV_MAX_NUM; i++) {
        aio_dev = ai_get_dev_ctx((td_s32)i);
        if (aio_dev->enable == TD_FALSE) {
            continue;
        }

        /* j is user chn id */
        for (j = 0; j < aio_dev->aio_attr.chn_cnt; j++) {
            chn = ai_get_chn_id(i, j);
            chn_ctx = ai_get_chn_ctx((td_s32)chn);
            if (chn_ctx->chn_state != AIO_CHN_ENABLE) {
                continue;
            }

            osal_seq_printf(s, "%5u" "%6u" "%8s", i, j, audio_print_chn_state(chn_ctx->chn_state));
            osal_seq_printf(s, "%8s" "%8s" "%12d" "%8d" "%8d" "%12d\n",
                            audio_print_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_agc_open),
                            audio_print_vqe_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_agc_cfg.usr_mode),
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_agc_cfg.target_level,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_agc_cfg.max_gain,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_agc_cfg.min_gain,
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_agc_cfg.adjust_speed);
        }
    }
}

static td_void ai_talkv2_eq_band1_proc(const struct osal_proc_dir_entry *s, td_u32 *vqe_index)
{
    td_u32 i, j, chn;
    aio_dev_ctx *aio_dev = TD_NULL;
    ai_chn_ctx *chn_ctx = TD_NULL;

    osal_seq_printf(s, "\n-----AI CHN VQE STATUS%u---------------------------------------------------------\n",
                    ++(*vqe_index));
    osal_seq_printf(s, "%5s" "%6s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s\n",
                    "AiDev", "AiChn", "State", "bEqV2", "bUsrmod",
                    "60", "100", "150", "200", "250", "350", "500", "800", "1200", "1600");

    for (i = 0; i < OT_AI_DEV_MAX_NUM; i++) {
        aio_dev = ai_get_dev_ctx((td_s32)i);
        if (aio_dev->enable == TD_FALSE) {
            continue;
        }

        /* j is user chn id */
        for (j = 0; j < aio_dev->aio_attr.chn_cnt; j++) {
            chn = ai_get_chn_id(i, j);
            chn_ctx = ai_get_chn_ctx((td_s32)chn);
            if (chn_ctx->chn_state != AIO_CHN_ENABLE) {
                continue;
            }
            osal_seq_printf(s, "ai_talkv2_eq_band1_proc %5u" "%6u" "%8s",
                            i, j, audio_print_chn_state(chn_ctx->chn_state));
            osal_seq_printf(s, "%8s" "%8s" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d\n",
                            audio_print_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_eq_open),
                            audio_print_vqe_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_eq_cfg.usr_mode),
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_eq_cfg.gain_db[0],
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_eq_cfg.gain_db[1],
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_eq_cfg.gain_db[2], /* 2:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_eq_cfg.gain_db[3], /* 3:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_eq_cfg.gain_db[4], /* 4:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_eq_cfg.gain_db[5], /* 5:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_eq_cfg.gain_db[6], /* 6:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_eq_cfg.gain_db[7], /* 7:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_eq_cfg.gain_db[8], /* 8:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_eq_cfg.gain_db[9]); /* 9:index */
        }
    }
}

static td_void ai_talkv2_eq_band2_proc(const struct osal_proc_dir_entry *s, td_u32 *vqe_index)
{
    td_u32 i, j, chn;
    aio_dev_ctx *aio_dev = TD_NULL;
    ai_chn_ctx *chn_ctx = TD_NULL;

    osal_seq_printf(s, "\n-----AI CHN VQE STATUS%u---------------------------------------------------------\n",
                    ++(*vqe_index));
    osal_seq_printf(s, "%5s" "%6s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s\n",
                    "AiDev", "AiChn", "State", "bEqV2", "bUsrmod",
                    "2000", "2400", "2500", "2800", "3100", "3400", "3600", "3800", "3900", "4000");

    for (i = 0; i < OT_AI_DEV_MAX_NUM; i++) {
        aio_dev = ai_get_dev_ctx((td_s32)i);
        if (aio_dev->enable == TD_FALSE) {
            continue;
        }

        /* j is user chn id */
        for (j = 0; j < aio_dev->aio_attr.chn_cnt; j++) {
            chn = ai_get_chn_id(i, j);
            chn_ctx = ai_get_chn_ctx((td_s32)chn);
            if (chn_ctx->chn_state != AIO_CHN_ENABLE) {
                continue;
            }
            osal_seq_printf(s, "ai_talkv2_eq_band2_proc %5u" "%6u" "%8s",
                            i, j, audio_print_chn_state(chn_ctx->chn_state));
            osal_seq_printf(s, "%8s" "%8s" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d\n",
                            audio_print_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_eq_open),
                            audio_print_vqe_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_eq_cfg.usr_mode),
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_eq_cfg.gain_db[10], /* 10:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_eq_cfg.gain_db[11], /* 11:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_eq_cfg.gain_db[12], /* 12:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_eq_cfg.gain_db[13], /* 13:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_eq_cfg.gain_db[14], /* 14:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_eq_cfg.gain_db[15], /* 15:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_eq_cfg.gain_db[16], /* 16:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_eq_cfg.gain_db[17], /* 17:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_eq_cfg.gain_db[18], /* 18:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_eq_cfg.gain_db[19]); /* 19:index */
        }
    }
}

static td_void ai_talkv2_eq_band3_proc(const struct osal_proc_dir_entry *s, td_u32 *vqe_index)
{
    td_u32 i, j, chn;
    aio_dev_ctx *aio_dev = TD_NULL;
    ai_chn_ctx *chn_ctx = TD_NULL;

    osal_seq_printf(s, "\n-----AI CHN VQE STATUS%u---------------------------------------------------------\n",
                    ++(*vqe_index));
    osal_seq_printf(s, "%5s" "%6s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s\n",
                    "AiDev", "AiChn", "State", "bEqV2", "bUsrmod",
                    "4500", "5000", "5500", "6000", "6500", "7000", "7300", "7600", "7800", "8000");

    for (i = 0; i < OT_AI_DEV_MAX_NUM; i++) {
        aio_dev = ai_get_dev_ctx((td_s32)i);
        if (aio_dev->enable == TD_FALSE) {
            continue;
        }

        /* j is user chn id */
        for (j = 0; j < aio_dev->aio_attr.chn_cnt; j++) {
            chn = ai_get_chn_id(i, j);
            chn_ctx = ai_get_chn_ctx((td_s32)chn);
            if (chn_ctx->chn_state != AIO_CHN_ENABLE) {
                continue;
            }
            osal_seq_printf(s, "ai_talkv2_eq_band3_proc %5u" "%6u" "%8s",
                            i, j, audio_print_chn_state(chn_ctx->chn_state));
            osal_seq_printf(s, "%8s" "%8s" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d\n",
                            audio_print_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_eq_open),
                            audio_print_vqe_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_eq_cfg.usr_mode),
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_eq_cfg.gain_db[20], /* 20:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_eq_cfg.gain_db[21], /* 21:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_eq_cfg.gain_db[22], /* 22:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_eq_cfg.gain_db[23], /* 23:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_eq_cfg.gain_db[24], /* 24:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_eq_cfg.gain_db[25], /* 25:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_eq_cfg.gain_db[26], /* 26:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_eq_cfg.gain_db[27], /* 27:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_eq_cfg.gain_db[28], /* 28:index */
                            chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_eq_cfg.gain_db[29]); /* 29:index */
        }
    }
}

static td_void ai_talkv2_eq_proc(const struct osal_proc_dir_entry *s, td_u32 *vqe_index)
{
    ai_talkv2_eq_band1_proc(s, vqe_index);
    ai_talkv2_eq_band2_proc(s, vqe_index);
    ai_talkv2_eq_band3_proc(s, vqe_index);
}

static td_void ai_talkv2_basic_proc(const struct osal_proc_dir_entry *s, td_u32 *vqe_index)
{
    td_u32 i, j, chn;
    aio_dev_ctx *aio_dev = TD_NULL;
    ai_chn_ctx *chn_ctx = TD_NULL;

    osal_seq_printf(s, "\n-----AI CHN VQE STATUS%u---------------------------------------------------------\n",
                    ++(*vqe_index));
    osal_seq_printf(s, "%5s" "%6s" "%8s" "%8s" "%9s" "%10s" "%5s" "%5s\n",
                    "AiDev", "AiChn", "State", "bDrc", "bLimiter", "bDereverb", "bFmp", "bWnr");

    for (i = 0; i < OT_AI_DEV_MAX_NUM; i++) {
        aio_dev = ai_get_dev_ctx((td_s32)i);
        if (aio_dev->enable == TD_FALSE) {
            continue;
        }

        /* j is user chn id */
        for (j = 0; j < aio_dev->aio_attr.chn_cnt; j++) {
            chn = ai_get_chn_id(i, j);
            chn_ctx = ai_get_chn_ctx((td_s32)chn);
            if (chn_ctx->chn_state != AIO_CHN_ENABLE) {
                continue;
            }
            osal_seq_printf(s, "%5u" "%6u" "%8s", i, j, audio_print_chn_state(chn_ctx->chn_state));
            osal_seq_printf(s, "%8s" "%9s" "%10s" "%5s" "%5s\n",
                            audio_print_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_drc_open),
                            audio_print_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_limiter_open),
                            audio_print_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_dereverb_open),
                            audio_print_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.talkv2_fmp_open),
                            audio_print_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.wnr_open));
        }
    }
}

static td_void set_talk_v2_vqe_mod_state(const ai_vqe_info *ai_vqe_dbg, ai_proc_vqe_show_state *vqe_mod_show)
{
    /* TalkV2 VQE */
    if (ai_vqe_dbg->ai_vqe_cfg.talkv2_pnr_open) {
        vqe_mod_show->talkv2_pnr_show = TD_TRUE;
    }
    if (ai_vqe_dbg->ai_vqe_cfg.talkv2_nr_open) {
        vqe_mod_show->talkv2_nr_show = TD_TRUE;
    }
    if (ai_vqe_dbg->ai_vqe_cfg.talkv2_agc_open) {
        vqe_mod_show->talkv2_agc_show = TD_TRUE;
    }
    if (ai_vqe_dbg->ai_vqe_cfg.talkv2_eq_open) {
        vqe_mod_show->talkv2_eq_show = TD_TRUE;
    }
    if (ai_vqe_dbg->ai_vqe_cfg.talkv2_drc_open) {
        vqe_mod_show->talkv2_drc_show = TD_TRUE;
    }
    if (ai_vqe_dbg->ai_vqe_cfg.talkv2_limiter_open) {
        vqe_mod_show->talkv2_limiter_show = TD_TRUE;
    }
    if (ai_vqe_dbg->ai_vqe_cfg.talkv2_dereverb_open) {
        vqe_mod_show->talkv2_dereverb_show = TD_TRUE;
    }
    if (ai_vqe_dbg->ai_vqe_cfg.talkv2_fmp_open) {
        vqe_mod_show->talkv2_fmp_show = TD_TRUE;
    }
    if (ai_vqe_dbg->ai_vqe_cfg.wnr_open) {
        vqe_mod_show->wnr_show = TD_TRUE;
    }
}

static td_void ai_chn_set_vqe_mod_state(const ai_vqe_info *ai_vqe_dbg, ai_proc_vqe_show_state *vqe_mod_show)
{
    if (ai_vqe_dbg->ai_vqe_cfg.anr_open) {
        vqe_mod_show->anr_show = TD_TRUE;
    }

    if (ai_vqe_dbg->ai_vqe_cfg.agc_open) {
        vqe_mod_show->agc_show = TD_TRUE;
    }

    if (ai_vqe_dbg->ai_vqe_cfg.aec_open) {
        vqe_mod_show->aec_show = TD_TRUE;
    }

    if (ai_vqe_dbg->ai_vqe_cfg.hpf_open) {
        vqe_mod_show->hpf_show = TD_TRUE;
    }

    if (ai_vqe_dbg->ai_vqe_cfg.eq_open) {
        vqe_mod_show->eq_show = TD_TRUE;
    }

    if (ai_vqe_dbg->ai_vqe_cfg.rnr_open) {
        vqe_mod_show->rnr_show = TD_TRUE;
    }

    if (ai_vqe_dbg->ai_vqe_cfg.hdr_open) {
        vqe_mod_show->hdr_show = TD_TRUE;
    }

    if (ai_vqe_dbg->ai_vqe_cfg.drc_open) {
        vqe_mod_show->drc_show = TD_TRUE;
    }

    if (ai_vqe_dbg->ai_vqe_cfg.peq_open) {
        vqe_mod_show->peq_show = TD_TRUE;
    }

    /* TalkV2 VQE */
    set_talk_v2_vqe_mod_state(ai_vqe_dbg, vqe_mod_show);
}

static td_void ai_chn_vqe_mod_proc(const struct osal_proc_dir_entry *s, const ai_proc_vqe_show_state *vqe_mod_show)
{
    td_u32 vqe_index = 0;

    if (vqe_mod_show->anr_show) {
        ai_anr_proc(s, &vqe_index);
    }

    if (vqe_mod_show->agc_show) {
        ai_agc_proc(s, &vqe_index);
    }

    if (vqe_mod_show->eq_show) {
        ai_eq_proc(s, &vqe_index);
    }

    if (vqe_mod_show->hpf_show) {
        ai_hpf_proc(s, &vqe_index);
    }

    if (vqe_mod_show->rnr_show) {
        ai_rnr_proc(s, &vqe_index);
    }

    if (vqe_mod_show->aec_show) {
        ai_aec_proc(s, &vqe_index);
    }

    if (vqe_mod_show->drc_show) {
        ai_drc_proc(s, &vqe_index);
    }

    if (vqe_mod_show->peq_show) {
        ai_peq_proc(s, &vqe_index);
    }

    if (vqe_mod_show->hdr_show) {
        ai_hdr_proc(s, &vqe_index);
    }

    /* TalkV2 VQE */
    if (vqe_mod_show->talkv2_pnr_show) {
        ai_talkv2_pnr_proc(s, &vqe_index);
    }

    if (vqe_mod_show->talkv2_nr_show) {
        ai_talkv2_nr_proc(s, &vqe_index);
    }

    if (vqe_mod_show->talkv2_agc_show) {
        ai_talkv2_agc_proc(s, &vqe_index);
    }

    if (vqe_mod_show->talkv2_eq_show) {
        ai_talkv2_eq_proc(s, &vqe_index);
    }

    if (vqe_mod_show->talkv2_drc_show || vqe_mod_show->talkv2_limiter_show ||
        vqe_mod_show->talkv2_dereverb_show || vqe_mod_show->talkv2_fmp_show ||
        vqe_mod_show->wnr_show) {
        ai_talkv2_basic_proc(s, &vqe_index);
    }
}

static td_void ai_chn_vqe_proc(const struct osal_proc_dir_entry *s)
{
    td_u32 i, j, chn;
    td_bool vqe_show = TD_FALSE;
    ai_proc_vqe_show_state vqe_mod_show = { 0 };
    aio_dev_ctx *aio_dev = TD_NULL;
    ai_chn_ctx *chn_ctx = TD_NULL;

    /* 通道状态---------------------------------------------------------------*/
    osal_seq_printf(s, "\n-----AI CHN VQE STATUS0---------------------------------------------------------\n");
    osal_seq_printf(s, "%5s" "%6s" "%8s" "%5s" "%8s" "%8s" "%8s" "%5s" "%5s" "%5s" "%5s" "%5s" "%5s" "%8s\n",
                    "AiDev", "AiChn", "State", "bVqe", "workmod", "RATE", "PoiNum",
                    "bAgc", "bEq", "bHpf", "bRnr", "bHdr", "bDrc", "WrFile");
    for (i = 0; i < OT_AI_DEV_MAX_NUM; i++) {
        aio_dev = ai_get_dev_ctx((td_s32)i);
        if (aio_dev->enable == TD_FALSE) {
            continue;
        }
        /* j is user chn id */
        for (j = 0; j < aio_dev->aio_attr.chn_cnt; j++)  {
            chn = ai_get_chn_id(i, j);
            chn_ctx = ai_get_chn_ctx((td_s32)chn);
            if (chn_ctx->chn_state != AIO_CHN_ENABLE) {
                continue;
            }

            if (chn_ctx->ai_vqe_dbg.vqe_enable) {
                vqe_show = TD_TRUE;
            }

            /* 配置待显示proc信息的vqe模块 */
            ai_chn_set_vqe_mod_state(&(chn_ctx->ai_vqe_dbg), &vqe_mod_show);

            osal_seq_printf(s, "%5u" "%6u" "%8s", i, j, audio_print_chn_state(chn_ctx->chn_state));
            osal_seq_printf(s, "%5s" "%8s" "%8s" "%8d" "%5s"  "%5s"  "%5s"  "%5s" "%5s"  "%5s"  "%8s\n",
                audio_print_state(chn_ctx->ai_vqe_dbg.vqe_enable),
                audio_print_vqe_mode(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.work_state),
                audio_print_sample_rate(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.work_sample_rate),
                chn_ctx->ai_vqe_dbg.ai_vqe_cfg.frame_sample,
                audio_print_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.agc_open),
                audio_print_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.eq_open),
                audio_print_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.hpf_open),
                audio_print_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.rnr_open),
                audio_print_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.hdr_open),
                audio_print_state(chn_ctx->ai_vqe_dbg.ai_vqe_cfg.drc_open),
                audio_print_state(chn_ctx->save_file_info.cfg));
        }
    }

    if (vqe_show == TD_FALSE) {
        return;
    }

    /* 各vqe模块的proc */
    ai_chn_vqe_mod_proc(s, &vqe_mod_show);

    return;
}

static td_void ai_chn_status_proc(const struct osal_proc_dir_entry *s)
{
    td_u32 i, j, chn;

    aio_dev_ctx *aio_dev = TD_NULL;
    ai_chn_ctx *chn_ctx = TD_NULL;

    /* 通道状态--------------------------------------------------------------- */
    osal_seq_printf(s, "\n-----AI CHN STATUS--------------------------------------------------------------\n");
    osal_seq_printf(s, "%5s" "%8s" "%8s" "%8s" "%11s" "%11s" "%12s" "%11s" "%11s" "%11s" "%11s\n",
                    "AiDev", "AiChn", "State", "Mode", "BufFul", "UsrQueLost",
                    "UsrFrmDepth", "u32Data0", "u32Data1", "UserGet", "UserRls");
    for (i = 0; i < OT_AI_DEV_MAX_NUM; i++) {
        td_s16 *data = TD_NULL;
        aio_dev = ai_get_dev_ctx((td_s32)i);
        if (aio_dev->enable == TD_FALSE) {
            continue;
        }
        /* j is user chn id */
        for (j = 0; j < aio_dev->aio_attr.chn_cnt; j++) {
            chn = ai_get_chn_id(i, j);
            chn_ctx = ai_get_chn_ctx((td_s32)chn);
            if (chn_ctx->chn_state != AIO_CHN_ENABLE) {
                continue;
            }

            osal_seq_printf(s, "%5u" "%8u" "%8s" "%8s" "%11u" "%11u" "%12u",
                            i, j,
                            audio_print_chn_state(chn_ctx->chn_state),
                            audio_print_ai_chn_mode(chn_ctx->chn_attr.mode),
                            chn_ctx->int_lost,
                            chn_ctx->ai_usr_que_lost,
                            chn_ctx->chn_param.usr_frame_depth);
            /* 如果AI没有时钟的话，这个指针会是空指针 */
            data = (td_s16 *)chn_ctx->audio_frm.frm.virt_addr[0];

            if (data != TD_NULL) {
                osal_seq_printf(s, "%11d", ot_check_signal_16bit(data));
                data += 1;
                osal_seq_printf(s, "%11d", ot_check_signal_16bit(data));
            } else {
                osal_seq_printf(s, "          0" "          0");
            }

            osal_seq_printf(s, "%11u" "%11u\n",
                            chn_ctx->aio_user_dbg.ai_get_cnt, chn_ctx->aio_user_dbg.ai_release_cnt);
        }
    }
}

static td_void ai_chn_resample_proc(const struct osal_proc_dir_entry *s)
{
    td_u32 i, j, chn;

    aio_dev_ctx *aio_dev = TD_NULL;
    ai_chn_ctx *chn_ctx = TD_NULL;

    /* 通道状态--------------------------------------------------------------- */
    osal_seq_printf(s, "\n-----AI CHN RESAMPLE STATUS-----------------------------------------------------\n");
    osal_seq_printf(s, "%5s"    "%6s"   "%8s"   "%8s"    "%8s"    "%10s"    "%10s\n",
                    "AiDev", "AiChn", "State", "bResmp", "PoiNum", "InSampR", "OutSampR");

    for (i = 0; i < OT_AI_DEV_MAX_NUM; i++) {
        aio_dev = ai_get_dev_ctx((td_s32)i);
        if (aio_dev->enable == TD_FALSE) {
            continue;
        }
        /* j is user chn id */
        for (j = 0; j < aio_dev->aio_attr.chn_cnt; j++) {
            chn = ai_get_chn_id(i, j);
            chn_ctx = ai_get_chn_ctx((td_s32)chn);
            if (chn_ctx->chn_state != AIO_CHN_ENABLE) {
                continue;
            }
            osal_seq_printf(s, "%5u" "%6u" "%8s",
                            i, j, audio_print_chn_state(chn_ctx->chn_state));
            osal_seq_printf(s, "%8s" "%8u" "%10s" "%10s\n",
                            audio_print_state(chn_ctx->resmp_dbg.resmp_enable),
                            chn_ctx->resmp_dbg.resmp_attr.in_point_num,
                            audio_print_sample_rate(chn_ctx->resmp_dbg.resmp_attr.in_sample_rate),
                            audio_print_sample_rate(chn_ctx->resmp_dbg.resmp_attr.out_sample_rate));
        }
    }
}

td_s32 ai_proc_show(const struct osal_proc_dir_entry *s)
{
    osal_seq_printf(s, "\n[AI] Version: ["OT_MPP_VERSION"], Build Time: ["__DATE__", "__TIME__"]\n");

    /* AI设备状态 */
    (void)ai_drv_proc_show(s);

    /* AI通道状态 */
    ai_chn_status_proc(s);

    /* AI通道的重采样状态 */
    ai_chn_resample_proc(s);

    /* AI通道的VQE状态 */
    ai_chn_vqe_proc(s);

    return TD_SUCCESS;
}
