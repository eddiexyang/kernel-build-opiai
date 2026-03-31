/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2009-2020. All rights reserved.
 * Description: Header of ao trans driver
 * Author: Hisilicon multimedia software group
 * Create: 2009/5/5
 */

#ifndef AO_TRANS_H
#define AO_TRANS_H

#include "aio_pub.h"
#include "aiao_ext.h"

#define ao_check_func_entrance_return(id) \
do { \
    if (func_entry_null(id)) { \
        return OT_ERR_AO_NOT_READY; \
    } \
} while (0)

td_s32 ao_set_dev_attr(ot_audio_dev audio_dev_id, const ot_aio_attr *attr);
td_s32 ao_get_dev_attr(ot_audio_dev audio_dev_id, ot_aio_attr *attr);
td_s32 ao_enable_dev(ot_audio_dev audio_dev_id);
td_s32 ao_disable_dev(ot_audio_dev audio_dev_id);

td_s32 ao_dev_init(ot_audio_dev audio_dev_id);
td_s32 ao_dev_exit(ot_audio_dev audio_dev_id);
td_void ao_ctx_sem_exit(ot_audio_dev audio_dev_id);
td_void ao_dev_sem_init(ot_audio_dev audio_dev_id);
aio_dev_ctx *ao_get_dev_ctx(ot_audio_dev audio_dev_id);
ao_chn_ctx *ao_get_chn_ctx(ot_ao_chn ao_chn);
td_s32 ao_get_data(ot_audio_dev ao_dev, ot_ao_chn ao_chn, ot_audio_frame *frm);
td_s32 ao_clr_dev_attr(ot_audio_dev audio_dev_id);

td_void ao_save_dma_rd_ptr(ot_audio_dev ao_dev);
td_void ao_set_dma_rd_offset_init_flag(ot_audio_dev ao_dev, ot_ao_chn ao_chn);

td_s32 ao_set_alsa_dma_attr(ot_audio_dev ao_dev, cir_buf cir_buf_attr, td_u32 trans_len);
#endif
