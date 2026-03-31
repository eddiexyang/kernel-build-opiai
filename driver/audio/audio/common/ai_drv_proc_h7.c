/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: ai drv proc for V7 platform
 * Author: Hisilicon multimedia software group
 * Create: 2019-09-01
 * History: 2019-09-01 Created for V7 platform
 */

#include "ai_drv_proc.h"
#include "aio_pub.h"
#include "ai_drv.h"

static td_void ai_drv_proc_show_attr(const struct osal_proc_dir_entry *s)
{
    td_u32 i;
    aio_drv_dev_ctx *ai_drv_dev = TD_NULL;

    /* 设备属性 */
    osal_seq_printf(s, "\n-----AI DEV ATTR----------------------------------------------------------------\n");
    osal_seq_printf(s, "%5s"   "%8s"    "%8s"   "%8s"    "%8s"    "%8s"    "%9s"     "%8s"    "%8s"    "%8s\n",
                    "AiDev", "WorkMod", "SampR", "BitWid", "ChnCnt",
                    "ClkSel", "SoundMod", "PoiNum", "ExFlag", "FrmNum");
    for (i = 0; i < OT_AI_DEV_MAX_NUM; i++) {
        ai_drv_dev = ai_drv_get_drv_dev_ctx((td_s32)i);
        if (ai_drv_dev->enable == TD_FALSE) {
            continue;
        }

        osal_seq_printf(s, "%5u" "%8s" "%8s" "%8s" "%8u" "%8u" "%9s" "%8u" "%8u" "%8u\n",
                        i,
                        audio_print_work_mode(ai_drv_dev->aio_attr.work_mode),
                        audio_print_sample_rate(ai_drv_dev->aio_attr.sample_rate),
                        audio_print_bit_width(ai_drv_dev->aio_attr.bit_width),
                        ai_drv_dev->aio_attr.chn_cnt,
                        ai_drv_dev->aio_attr.clk_share,
                        audio_print_sound_mode(ai_drv_dev->aio_attr.snd_mode),
                        ai_drv_dev->aio_attr.point_num_per_frame,
                        ai_drv_dev->aio_attr.expand_flag,
                        ai_drv_dev->aio_attr.frame_num);
    }
}

static td_void ai_drv_proc_show_status(const struct osal_proc_dir_entry *s)
{
    td_u32 i;
    aio_drv_dev_ctx *ai_drv_dev = TD_NULL;

    /* 设备状态 */
    osal_seq_printf(s, "\n-----AI DEV STATUS0-------------------------------------------------------------\n");
    osal_seq_printf(s, "%5s"   "%11s"    "%11s"    "%11s"     "%8s"     "%11s"       "%8s"     "%8s \n",
                    "AiDev", "IntCnt", "fifoCnt", "buffInt", "FrmTime", "MaxFrmTime", "TranLen", "IsrTime");
    for (i = 0; i < OT_AI_DEV_MAX_NUM; i++) {
        ai_drv_dev = ai_drv_get_drv_dev_ctx((td_s32)i);
        if (ai_drv_dev->enable == TD_FALSE) {
            continue;
        }

        osal_seq_printf(s, "%5u" "%11u" "%11u" "%11u" "%8u" "%11u" "%8u" "%8u\n",
                        i,
                        ai_drv_dev->int_cnt,
                        ai_drv_dev->u32fifo_int_cnt,
                        ai_drv_dev->u32buff_int_cnt,
                        ai_drv_dev->frm_time,
                        ai_drv_dev->max_frm_time,
                        ai_drv_dev->trans_len,
                        ai_drv_dev->isr_time);
    }

    osal_seq_printf(s, "\n-----AI DEV STATUS1-------------------------------------------------------------\n");
    osal_seq_printf(s, "%5s"  "%11s"     "%11s"   "%8s"    "%11s"    "%11s\n",
                    "AiDev", "MaxIsrTime", "CBPhy", "CBSize", "ROffSet", "WOffSet");
    for (i = 0; i < OT_AI_DEV_MAX_NUM; i++) {
        ai_drv_dev = ai_drv_get_drv_dev_ctx((td_s32)i);
        if (ai_drv_dev->enable == TD_FALSE) {
            continue;
        }

        osal_seq_printf(s, "%5u" "%11u" "%11llx" "%8u" "%11x" "%11x\n",
                        i,
                        ai_drv_dev->max_isr_time,
                        ai_drv_dev->cir_buf.phys_addr,
                        ai_drv_dev->cir_buf.buf_size,
                        ai_drv_dev->cir_buf.rptr_off_set,
                        ai_drv_dev->cir_buf.wptr_off_set);
    }
}

static td_void ai_drv_proc_show_extend_status(const struct osal_proc_dir_entry *s)
{
    td_u32 i;
    aio_drv_dev_ctx *ai_drv_dev = TD_NULL;

    /* 设备扩展状态 */
    osal_seq_printf(s, "\n-----AI DEV EXTEND STATUS-------------------------------------------------------\n");
    osal_seq_printf(s, "%5s"    "%8s"      "%8s"    "%8s\n",
                    "AiDev", "enTrack", "bMute", "Volume");
    for (i = 0; i < OT_AI_DEV_MAX_NUM; i++) {
        ai_drv_dev = ai_drv_get_drv_dev_ctx((td_s32)i);
        if (ai_drv_dev->enable == TD_FALSE) {
            continue;
        }

        osal_seq_printf(s, "%5u" "%8u" "%8s" "%8d\n",
                        i,
                        (td_u32)(ai_drv_dev->track_mode),
                        audio_print_state(ai_drv_dev->mute),
                        ai_drv_dev->volume);
    }
}

td_s32 ai_drv_proc_show(const struct osal_proc_dir_entry *s)
{
    /* 设备属性 */
    ai_drv_proc_show_attr(s);

    /* 设备状态 */
    ai_drv_proc_show_status(s);

    /* 设备扩展状态 */
    ai_drv_proc_show_extend_status(s);

    return 0;
}
