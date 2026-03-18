/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: ao drv proc for V7 platform
 * Author: Hisilicon multimedia software group
 * Create: 2019-09-01
 * History: 2019-09-01 Created for V7 platform
 */

#include "ao_drv_proc.h"
#include "aio_pub.h"
#include "ao_drv.h"

static td_void ao_drv_proc_show_attr(const struct osal_proc_dir_entry *s)
{
    td_u32 i;
    aio_drv_dev_ctx *aio_drv_dev = TD_NULL;

    /* 设备属性 */
    osal_seq_printf(s, "\n-----AO DEV ATTR----------------------------------------------------------------\n");
    osal_seq_printf(s, "%5s" "%8s" "%8s" "%8s" "%8s" "%8s" "%9s" "%8s" "%8s" "%8s\n",
                    "AoDev", "WorkMod", "SampR", "BitWid", "ChnCnt",
                    "ClkSel", "SoundMod", "PoiNum", "ExFlag", "FrmNum");
    for (i = OT_AO_DEV_MIN_NUM; i < OT_AO_DEV_MAX_NUM; i++) {
        aio_drv_dev = ao_drv_get_drv_dev_ctx((td_s32)i);
        if (aio_drv_dev->enable == TD_FALSE) {
            continue;
        }

        osal_seq_printf(s, "%5u" "%8s" "%8s" "%8s" "%8u" "%8u" "%9s" "%8u" "%8u" "%8u\n",
                        i,
                        audio_print_work_mode(aio_drv_dev->aio_attr.work_mode),
                        audio_print_sample_rate(aio_drv_dev->aio_attr.sample_rate),
                        audio_print_bit_width(aio_drv_dev->aio_attr.bit_width),
                        aio_drv_dev->aio_attr.chn_cnt,
                        aio_drv_dev->aio_attr.clk_share,
                        audio_print_sound_mode(aio_drv_dev->aio_attr.snd_mode),
                        aio_drv_dev->aio_attr.point_num_per_frame,
                        aio_drv_dev->aio_attr.expand_flag,
                        aio_drv_dev->aio_attr.frame_num);
    }
}

static td_void ao_drv_proc_show_status(const struct osal_proc_dir_entry *s)
{
    td_u32 i;
    aio_drv_dev_ctx *aio_drv_dev = TD_NULL;

    /* 设备状态 */
    osal_seq_printf(s, "\n-----AO DEV STATUS0-------------------------------------------------------------\n");
    osal_seq_printf(s, "%5s"   "%11s"    "%11s"    "%11s"     "%8s"     "%11s"       "%8s"     "%8s\n",
                    "AoDev", "IntCnt", "fifoCnt", "buffInt", "FrmTime", "MaxFrmTime", "TranLen", "IsrTime");
    for (i = OT_AO_DEV_MIN_NUM; i < OT_AO_DEV_MAX_NUM; i++) {
        aio_drv_dev = ao_drv_get_drv_dev_ctx((td_s32)i);
        if (aio_drv_dev->enable == TD_FALSE) {
            continue;
        }

        osal_seq_printf(s, "%5u" "%11u" "%11u" "%11u" "%8u" "%11u" "%8u" "%8u\n",
                        i,
                        aio_drv_dev->int_cnt,
                        aio_drv_dev->u32fifo_int_cnt,
                        aio_drv_dev->u32buff_int_cnt,
                        aio_drv_dev->frm_time,
                        aio_drv_dev->max_frm_time,
                        aio_drv_dev->trans_len,
                        aio_drv_dev->isr_time);
    }

    osal_seq_printf(s, "\n-----AO DEV STATUS1-------------------------------------------------------------\n");
    osal_seq_printf(s, "%5s" "%11s"       "%11s"   "%8s"   "%11s"    "%11s\n",
                    "AoDev", "MaxIsrTime", "CBPhy", "CBSize", "ROffSet", "WOffSet");
    for (i = OT_AO_DEV_MIN_NUM; i < OT_AO_DEV_MAX_NUM; i++) {
        aio_drv_dev = ao_drv_get_drv_dev_ctx((td_s32)i);
        if (aio_drv_dev->enable == TD_FALSE) {
            continue;
        }

        osal_seq_printf(s, "%5u" "%11u" "%11llx" "%8u" "%11x" "%11x\n",
                        i,
                        aio_drv_dev->max_isr_time,
                        aio_drv_dev->cir_buf.phys_addr,
                        aio_drv_dev->cir_buf.buf_size,
                        aio_drv_dev->cir_buf.rptr_off_set,
                        aio_drv_dev->cir_buf.wptr_off_set);
    }
}

static td_void ao_drv_proc_show_extend_status(const struct osal_proc_dir_entry *s)
{
    td_u32 i;
    aio_drv_dev_ctx *aio_drv_dev = TD_NULL;

    /* 设备扩展状态 */
    osal_seq_printf(s, "\n-----AO DEV EXTEND STATUS-------------------------------------------------------\n");
    osal_seq_printf(s, "%5s"    "%8s"      "%8s"    "%8s\n",
                    "AoDev", "enTrack", "bMute", "Volume");
    for (i = OT_AO_DEV_MIN_NUM; i < OT_AO_DEV_MAX_NUM; i++) {
        aio_drv_dev = ao_drv_get_drv_dev_ctx((td_s32)i);
        if (aio_drv_dev->enable == TD_FALSE) {
            continue;
        }

        osal_seq_printf(s, "%5u" "%8u" "%8s" "%8d\n",
                        i,
                        (td_u32)aio_drv_dev->track_mode,
                        audio_print_state(aio_drv_dev->mute),
                        aio_drv_dev->volume);
    }
}

td_s32 ao_drv_proc_show(struct osal_proc_dir_entry *s)
{
    /* 设备属性 */
    ao_drv_proc_show_attr(s);

    /* 设备状态 */
    ao_drv_proc_show_status(s);

    /* 设备扩展状态 */
    ao_drv_proc_show_extend_status(s);

    return 0;
}
