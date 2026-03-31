/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: ao proc for V7 platform
 * Author: Hisilicon multimedia software group
 * Create: 2019-09-01
 */

#include "ao_proc.h"
#include "ao_drv_proc.h"
#include "aio_pub.h"
#include "ao_trans.h"

typedef struct {
    td_bool anr_show;
    td_bool agc_show;
    td_bool eq_show;
    td_bool hpf_show;
} ao_proc_vqe_show_state;

static td_void ao_anr_proc(struct osal_proc_dir_entry *s, td_u32 *vqe_index)
{
    td_u32 i, j, chn;
    aio_dev_ctx *aio_dev = TD_NULL;
    ao_chn_ctx *ao_chn = TD_NULL;

    /* 通道状态--------------------------------------------------------------- */
    osal_seq_printf(s, "\n-----AO CHN VQE STATUS%u---------------------------------------------------------\n",
                    ++(*vqe_index));
    osal_seq_printf(s, "%5s"    "%6s"   "%8s"    "%8s"      "%8s"   "%12s"          "%11s"       "%9s\n",
                    "AoDev", "AoChn", "State", "bAnr", "bUsrmod", "NrIntensity", "NoiseDbThr", "SpProSwi");

    for (i = 0; i < OT_AO_DEV_MAX_NUM; i++) {
        aio_dev = ao_get_dev_ctx((td_s32)i);
        if (aio_dev->enable == TD_FALSE) {
            continue;
        }
        /* j is user chn id */
        for (j = 0; j < aio_dev->aio_attr.chn_cnt; j++) {
            chn = ao_get_chn_id(i, j);
            ao_chn = ao_get_chn_ctx((td_s32)chn);
            if (ao_chn->chn_state != AIO_CHN_ENABLE) {
                continue;
            }

            osal_seq_printf(s, "%5u" "%6u" "%8s",
                            i, j, audio_print_chn_state(ao_chn->chn_state));
            osal_seq_printf(s, "%8s" "%8s" "%12d" "%11d" "%9d\n",
                            audio_print_state(ao_chn->ao_vqe_dbg.ao_vqe_cfg.anr_open),
                            audio_print_vqe_state(ao_chn->ao_vqe_dbg.ao_vqe_cfg.anr_cfg.usr_mode),
                            ao_chn->ao_vqe_dbg.ao_vqe_cfg.anr_cfg.nr_intensity,
                            ao_chn->ao_vqe_dbg.ao_vqe_cfg.anr_cfg.noise_db_threshold,
                            ao_chn->ao_vqe_dbg.ao_vqe_cfg.anr_cfg.music_probe_switch);
        }
    }
}

static td_void ao_agc_proc(const struct osal_proc_dir_entry *s, td_u32 *vqe_index)
{
    td_u32 i, j, chn;
    aio_dev_ctx *aio_dev = TD_NULL;
    ao_chn_ctx *ao_chn = TD_NULL;

    /* 通道状态---------------------------------------------------------------*/
    osal_seq_printf(s, "\n-----AO CHN VQE STATUS%u---------------------------------------------------------\n",
                    ++(*vqe_index));
    osal_seq_printf(s, "%5s" "%6s" "%8s" "%8s" "%8s" "%12s" "%12s" "%11s" "%8s" "%11s" "%11s" "%12s" "%8s\n",
                    "AoDev", "AoChn", "State", "bAgc", "bUsrmod", "NoiseSupSwi", "AdjustSpeed",
                    "ImproveSNR", "MaxGain", "NoiseFloor", "OutputMode", "TargetLevel", "UseHPF");
    for (i = 0; i < OT_AO_DEV_MAX_NUM; i++) {
        aio_dev = ao_get_dev_ctx((td_s32)i);
        if (aio_dev->enable == TD_FALSE) {
            continue;
        }
        /* j is user chn id */
        for (j = 0; j < aio_dev->aio_attr.chn_cnt; j++) {
            chn = ao_get_chn_id(i, j);
            ao_chn = ao_get_chn_ctx((td_s32)chn);
            if (ao_chn->chn_state != AIO_CHN_ENABLE) {
                continue;
            }

            osal_seq_printf(s, "%5u" "%6u" "%8s",
                            i, j, audio_print_chn_state(ao_chn->chn_state));
            osal_seq_printf(s, "%8s" "%8s" "%12d" "%12d" "%11d" "%8d" "%11d" "%11d" "%12d" "%8d\n",
                            audio_print_state(ao_chn->ao_vqe_dbg.ao_vqe_cfg.agc_open),
                            audio_print_vqe_state(ao_chn->ao_vqe_dbg.ao_vqe_cfg.agc_cfg.usr_mode),
                            ao_chn->ao_vqe_dbg.ao_vqe_cfg.agc_cfg.noise_suppress_switch,
                            ao_chn->ao_vqe_dbg.ao_vqe_cfg.agc_cfg.adjust_speed,
                            ao_chn->ao_vqe_dbg.ao_vqe_cfg.agc_cfg.improve_snr,
                            ao_chn->ao_vqe_dbg.ao_vqe_cfg.agc_cfg.max_gain,
                            ao_chn->ao_vqe_dbg.ao_vqe_cfg.agc_cfg.noise_floor,
                            ao_chn->ao_vqe_dbg.ao_vqe_cfg.agc_cfg.output_mode,
                            ao_chn->ao_vqe_dbg.ao_vqe_cfg.agc_cfg.target_level,
                            ao_chn->ao_vqe_dbg.ao_vqe_cfg.agc_cfg.use_hpf);
        }
    }
}

static td_void ao_eq_proc(struct osal_proc_dir_entry *s, td_u32 *vqe_index)
{
    td_u32 i, j, chn;
    aio_dev_ctx *aio_dev = TD_NULL;
    ao_chn_ctx *ao_chn = TD_NULL;

    /* 通道状态---------------------------------------------------------------*/
    osal_seq_printf(s, "\n-----AO CHN VQE STATUS%u---------------------------------------------------------\n",
                    ++(*vqe_index));
    osal_seq_printf(s, "%5s" "%6s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s\n",
                    "AoDev", "AoChn", "State", "bEq",
                    "100", "200", "250", "350", "500", "800", "1.2k", "2.5k", "4k", "8k");

    for (i = 0; i < OT_AO_DEV_MAX_NUM; i++) {
        aio_dev = ao_get_dev_ctx((td_s32)i);
        if (aio_dev->enable == TD_FALSE) {
            continue;
        }
        /* j is user chn id */
        for (j = 0; j < aio_dev->aio_attr.chn_cnt; j++) {
            chn = ao_get_chn_id(i, j);
            ao_chn = ao_get_chn_ctx((td_s32)chn);
            if (ao_chn->chn_state != AIO_CHN_ENABLE) {
                continue;
            }

            osal_seq_printf(s, "%5u" "%6u" "%8s",
                            i, j, audio_print_chn_state(ao_chn->chn_state));
            osal_seq_printf(s, "%8s" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d" "%8d\n",
                            audio_print_state(ao_chn->ao_vqe_dbg.ao_vqe_cfg.eq_open),
                            ao_chn->ao_vqe_dbg.ao_vqe_cfg.eq_cfg.gain_db[0],
                            ao_chn->ao_vqe_dbg.ao_vqe_cfg.eq_cfg.gain_db[1],
                            ao_chn->ao_vqe_dbg.ao_vqe_cfg.eq_cfg.gain_db[2], /* 2:index */
                            ao_chn->ao_vqe_dbg.ao_vqe_cfg.eq_cfg.gain_db[3], /* 3:index */
                            ao_chn->ao_vqe_dbg.ao_vqe_cfg.eq_cfg.gain_db[4], /* 4:index */
                            ao_chn->ao_vqe_dbg.ao_vqe_cfg.eq_cfg.gain_db[5], /* 5:index */
                            ao_chn->ao_vqe_dbg.ao_vqe_cfg.eq_cfg.gain_db[6], /* 6:index */
                            ao_chn->ao_vqe_dbg.ao_vqe_cfg.eq_cfg.gain_db[7], /* 7:index */
                            ao_chn->ao_vqe_dbg.ao_vqe_cfg.eq_cfg.gain_db[8], /* 8:index */
                            ao_chn->ao_vqe_dbg.ao_vqe_cfg.eq_cfg.gain_db[9]); /* 9:index */
        }
    }
}

static td_void ao_hpf_proc(const struct osal_proc_dir_entry *s, td_u32 *vqe_index)
{
    td_u32 i, j, chn;
    aio_dev_ctx *aio_dev = TD_NULL;
    ao_chn_ctx *ao_chn = TD_NULL;

    /* 通道状态--------------------------------------------------------------- */
    osal_seq_printf(s, "\n-----AO CHN VQE STATUS%u---------------------------------------------------------\n",
                    ++(*vqe_index));
    osal_seq_printf(s, "%5s"    "%6s"   "%8s"   "%8s"   "%8s"     "%8s\n",
                    "AoDev", "AoChn", "State", "bHpf", "bUsrmod", "HpfFreq");

    for (i = 0; i < OT_AO_DEV_MAX_NUM; i++) {
        aio_dev = ao_get_dev_ctx((td_s32)i);
        if (aio_dev->enable == TD_FALSE) {
            continue;
        }
        /* j is user chn id */
        for (j = 0; j < aio_dev->aio_attr.chn_cnt; j++) {
            chn = ao_get_chn_id(i, j);
            ao_chn = ao_get_chn_ctx((td_s32)chn);
            if (ao_chn->chn_state != AIO_CHN_ENABLE) {
                continue;
            }

            osal_seq_printf(s, "%5u" "%6u" "%8s",
                            i, j, audio_print_chn_state(ao_chn->chn_state));
            osal_seq_printf(s, "%8s" "%8s" "%8d\n",
                            audio_print_state(ao_chn->ao_vqe_dbg.ao_vqe_cfg.hpf_open),
                            audio_print_vqe_state(ao_chn->ao_vqe_dbg.ao_vqe_cfg.hpf_cfg.usr_mode),
                            (td_s32)ao_chn->ao_vqe_dbg.ao_vqe_cfg.hpf_cfg.hpf_freq);
        }
    }
}

static td_void ao_chn_set_vqe_mod_state(const ao_vqe_info *ao_vqe_dbg, ao_proc_vqe_show_state *vqe_mod_show)
{
    if (ao_vqe_dbg->ao_vqe_cfg.anr_open) {
        vqe_mod_show->anr_show = TD_TRUE;
    }

    if (ao_vqe_dbg->ao_vqe_cfg.agc_open) {
        vqe_mod_show->agc_show = TD_TRUE;
    }

    if (ao_vqe_dbg->ao_vqe_cfg.eq_open) {
        vqe_mod_show->eq_show = TD_TRUE;
    }

    if (ao_vqe_dbg->ao_vqe_cfg.hpf_open) {
        vqe_mod_show->hpf_show = TD_TRUE;
    }
}

static td_void ao_chn_vqe_mod_proc(struct osal_proc_dir_entry *s, const ao_proc_vqe_show_state *vqe_mod_show)
{
    td_u32 vqe_index = 0;

    if (vqe_mod_show->anr_show) {
        ao_anr_proc(s, &vqe_index);
    }

    if (vqe_mod_show->agc_show) {
        ao_agc_proc(s, &vqe_index);
    }

    if (vqe_mod_show->eq_show) {
        ao_eq_proc(s, &vqe_index);
    }

    if (vqe_mod_show->hpf_show) {
        ao_hpf_proc(s, &vqe_index);
    }
}

static td_void ao_chn_vqe_proc(const struct osal_proc_dir_entry *s)
{
    td_u32 i, j, chn;

    td_bool vqe_show = TD_FALSE;
    ao_proc_vqe_show_state ao_vqe_mod_show = { 0 };

    aio_dev_ctx *aio_dev = TD_NULL;
    ao_chn_ctx *ao_chn = TD_NULL;

    /* 通道状态---------------------------------------------------------------*/
    osal_seq_printf(s, "\n-----AO CHN VQE STATUS0---------------------------------------------------------\n");
    osal_seq_printf(s, "%5s" "%6s" "%8s" "%5s" "%8s" "%8s" "%8s" "%5s" "%5s" "%5s" "%5s" "%8s\n",
                    "AoDev", "AoChn", "State", "bVqe", "workmod", "RATE",
                    "PoiNum", "bAnr", "bAgc", "bEq", "bHpf", "WrFile");

    for (i = 0; i < OT_AO_DEV_MAX_NUM; i++) {
        aio_dev = ao_get_dev_ctx((td_s32)i);
        if (aio_dev->enable == TD_FALSE) {
            continue;
        }

        /* j is user chn id */
        for (j = 0; j < aio_dev->aio_attr.chn_cnt; j++) {
            chn = ao_get_chn_id(i, j);
            ao_chn = ao_get_chn_ctx((td_s32)chn);
            if (ao_chn->chn_state != AIO_CHN_ENABLE) {
                continue;
            }

            if (ao_chn->ao_vqe_dbg.vqe_enable) {
                vqe_show = TD_TRUE;
            }

            /* 配置待显示proc信息的vqe模块 */
            ao_chn_set_vqe_mod_state(&(ao_chn->ao_vqe_dbg), &ao_vqe_mod_show);

            osal_seq_printf(s, "%5u" "%6u" "%8s",
                            i, j, audio_print_chn_state(ao_chn->chn_state));
            osal_seq_printf(s, "%5s" "%8s" "%8s" "%8d" "%5s" "%5s" "%5s" "%5s" "%8s\n",
                            audio_print_state(ao_chn->ao_vqe_dbg.vqe_enable),
                            audio_print_vqe_mode(ao_chn->ao_vqe_dbg.ao_vqe_cfg.work_state),
                            audio_print_sample_rate(ao_chn->ao_vqe_dbg.ao_vqe_cfg.work_sample_rate),
                            ao_chn->ao_vqe_dbg.ao_vqe_cfg.frame_sample,
                            audio_print_state(ao_chn->ao_vqe_dbg.ao_vqe_cfg.anr_open),
                            audio_print_state(ao_chn->ao_vqe_dbg.ao_vqe_cfg.agc_open),
                            audio_print_state(ao_chn->ao_vqe_dbg.ao_vqe_cfg.eq_open),
                            audio_print_state(ao_chn->ao_vqe_dbg.ao_vqe_cfg.hpf_open),
                            audio_print_state(ao_chn->save_file_info.cfg));
        }
    }

    if (vqe_show != TD_TRUE) {
        return;
    }

    /* 各vqe模块的proc */
    ao_chn_vqe_mod_proc(s, &ao_vqe_mod_show);

    return;
}

static td_void ao_chn_status_proc(const struct osal_proc_dir_entry *s)
{
    td_u32 i, j, chn;
    td_s16 *data0 = TD_NULL;
    td_s16 *data1 = TD_NULL;

    aio_dev_ctx *aio_dev = TD_NULL;
    ao_chn_ctx *ao_chn = TD_NULL;

    /* 通道状态---------------------------------------------------------------*/
    osal_seq_printf(s, "\n-----AO CHN STATUS--------------------------------------------------------------\n");
    osal_seq_printf(s, "%5s" "%8s" "%8s" "%8s" "%8s" "%12s" "%10s" "%10s" "%8s" "%8s" "%8s" "%10s\n",
                    "AoDev", "AoChn", "State", "Read", "Write", "BufEmp",
                    "u32Data0", "u32Data1", "bResmp", "PoiNum", "InSampR", "OutSampR");

    for (i = OT_AO_DEV_MIN_NUM; i < OT_AO_DEV_MAX_NUM; i++) {
        aio_dev = ao_get_dev_ctx((td_s32)i);
        if (aio_dev->enable == TD_FALSE) {
            continue;
        }

        for (j = 0; j < OT_AO_MAX_CHN_NUM; j++) {
            chn = i * OT_AO_MAX_CHN_NUM + j;
            ao_chn = ao_get_chn_ctx((td_s32)chn);
            if ((ao_chn->chn_state != AIO_CHN_ENABLE) && (ao_chn->chn_state != AIO_CHN_PAUSE)) {
                continue;
            }

            data0 = (td_s16*)(ao_chn->buf.pack[0].data);
            data1 = data0 + 1;
            osal_seq_printf(s, "%5u" "%8u" "%8s" "%8u" "%8u" "%12u" "%10d" "%10d" "%8s" "%8u" "%8s" "%10s\n",
                            i, j,
                            audio_print_chn_state(ao_chn->chn_state),
                            ao_chn->buf.read,
                            ao_chn->buf.write,
                            ao_chn->int_lost,
                            ot_check_signal_16bit(data0),
                            ot_check_signal_16bit(data1),
                            audio_print_state(ao_chn->resmp_dbg.resmp_enable),
                            ao_chn->resmp_dbg.resmp_attr.in_point_num,
                            audio_print_sample_rate(ao_chn->resmp_dbg.resmp_attr.in_sample_rate),
                            audio_print_sample_rate(ao_chn->resmp_dbg.resmp_attr.out_sample_rate));
        }
    }
}

td_s32 ao_proc_show(const struct osal_proc_dir_entry *s)
{
    osal_seq_printf(s, "\n[AO] Version: ["OT_MPP_VERSION"], Build Time: ["__DATE__", "__TIME__"]\n");

    /* AO设备状态 */
    ao_drv_proc_show(s);

    /* AO通道状态 */
    ao_chn_status_proc(s);

    /* AO通道的VQE状态 */
    ao_chn_vqe_proc(s);

    return TD_SUCCESS;
}
