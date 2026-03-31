/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2023. All rights reserved.
 * Description: Header of ai trans driver
 * Author: Hisilicon multimedia software group
 * Create: 2022/5/5
 */

#ifndef AI_TRANS_H
#define AI_TRANS_H

#include "aio_pub.h"
#include "aiao_ext.h"

#define ai_check_func_entrance_return(id) \
    do { \
        if (func_entry_null(id)) { \
            return OT_ERR_AI_NOT_READY; \
        } \
    } while (0)

td_s32 ai_set_dev_attr(ot_audio_dev audio_dev_id, const ot_aio_attr *attr);
td_s32 ai_get_dev_attr(ot_audio_dev audio_dev_id, ot_aio_attr *attr);
td_s32 ai_clr_dev_attr(ot_audio_dev audio_dev_id);

td_s32 ai_enable_dev(ot_audio_dev audio_dev_id);
td_s32 ai_disable_dev(ot_audio_dev audio_dev_id, td_bool force_exit);

td_s32 ai_dev_init(ot_audio_dev audio_dev_id);
td_s32 ai_dev_exit(ot_audio_dev audio_dev_id);
td_void ai_dev_sem_exit(ot_audio_dev audio_dev_id);
td_void ai_dev_sem_init(ot_audio_dev audio_dev_id);
ai_chn_ctx *ai_get_chn_ctx(ot_ai_chn ai_chn);
aio_dev_ctx *ai_get_dev_ctx(ot_audio_dev audio_dev_id);

td_s32 ai_add_vb(const audio_frame_combine *audio_frm, td_u32 uid);
td_s32 ai_sub_vb(const audio_frame_combine *audio_frm, td_u32 uid);
td_s32 ai_get_pool_user_cnt(ot_audio_dev ai_dev, td_u32 *vb_user_cnt);

td_s32 ai_set_alsa_dma_attr(ot_audio_dev ai_dev, cir_buf cir_buf_attr, td_u32 trans_len);

#endif
