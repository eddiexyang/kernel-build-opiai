/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: ot_mpi_audio.h
 * Author: Hisilicon multimedia software group
 * Create: 2019/06/15
 */

#ifndef OT_MPI_AUDIO_H
#define OT_MPI_AUDIO_H

#include "ot_common_aenc.h"
#include "ot_common_adec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

#define OT_AENC_ADAPT_MAGIC 0Xfcfcfcfc

/* Audio function api. */
hi_s32 ot_mpi_audio_init_aio(hi_void);
hi_s32 ot_mpi_audio_exit_aio(hi_void);

hi_s32 ot_mpi_audio_set_mod_param(const ot_audio_mod_param *mod_param);
hi_s32 ot_mpi_audio_get_mod_param(const ot_audio_mod_param *mod_param);

hi_s32 ot_mpi_audio_register_vqe_mod(const ot_audio_vqe_register *vqe_register);

/* AI function api. */
hi_s32 ot_mpi_ai_set_pub_attr(ot_audio_dev ai_dev, ot_aio_attr *attr);
hi_s32 ot_mpi_ai_get_pub_attr(ot_audio_dev ai_dev, ot_aio_attr *attr);
hi_s32 ot_mpi_ai_clr_pub_attr(ot_audio_dev ai_dev);

hi_s32 ot_mpi_ai_enable(ot_audio_dev ai_dev);
hi_s32 ot_mpi_ai_disable(ot_audio_dev ai_dev);

hi_s32 ot_mpi_ai_set_chn_attr(ot_audio_dev ai_dev, ot_ai_chn ai_chn, const ot_ai_chn_attr *chn_attr);
hi_s32 ot_mpi_ai_get_chn_attr(ot_audio_dev ai_dev, ot_ai_chn ai_chn, ot_ai_chn_attr *chn_attr);

hi_s32 ot_mpi_ai_set_chn_param(ot_audio_dev ai_dev, ot_ai_chn ai_chn, const ot_ai_chn_param *chn_param);
hi_s32 ot_mpi_ai_get_chn_param(ot_audio_dev ai_dev, ot_ai_chn ai_chn, ot_ai_chn_param *chn_param);

hi_s32 ot_mpi_ai_enable_chn(ot_audio_dev ai_dev, ot_ai_chn ai_chn);
hi_s32 ot_mpi_ai_disable_chn(ot_audio_dev ai_dev, ot_ai_chn ai_chn);

hi_s32 ot_mpi_ai_get_frame(ot_audio_dev ai_dev, ot_ai_chn ai_chn,
                           ot_audio_frame *frame, ot_aec_frame *aec_frame, hi_s32 milli_sec);
hi_s32 ot_mpi_ai_release_frame(ot_audio_dev ai_dev, ot_ai_chn ai_chn,
                               const ot_audio_frame *frame, const ot_aec_frame *aec_frame);

hi_s32 ot_mpi_ai_set_talk_vqe_v2_attr(ot_audio_dev ai_dev, ot_ai_chn ai_chn,
    ot_audio_dev ao_dev, ot_ao_chn ao_chn, const ot_ai_talk_vqe_v2_cfg *vqe_cfg);
hi_s32 ot_mpi_ai_get_talk_vqe_v2_attr(ot_audio_dev ai_dev, ot_ai_chn ai_chn, ot_ai_talk_vqe_v2_cfg *vqe_cfg);

hi_s32 ot_mpi_ai_set_record_vqe_attr(ot_audio_dev ai_dev, ot_ai_chn ai_chn, const ot_ai_record_vqe_cfg *vqe_cfg);
hi_s32 ot_mpi_ai_get_record_vqe_attr(ot_audio_dev ai_dev, ot_ai_chn ai_chn, ot_ai_record_vqe_cfg *vqe_cfg);

hi_s32 ot_mpi_ai_set_talk_vqe_attr(ot_audio_dev ai_dev, ot_ai_chn ai_chn,
                                   ot_audio_dev ao_dev, ot_ao_chn ao_chn, const ot_ai_talk_vqe_cfg *vqe_cfg);
hi_s32 ot_mpi_ai_get_talk_vqe_attr(ot_audio_dev ai_dev, ot_ai_chn ai_chn, ot_ai_talk_vqe_cfg *vqe_cfg);

hi_s32 ot_mpi_ai_enable_vqe(ot_audio_dev ai_dev, ot_ai_chn ai_chn);
hi_s32 ot_mpi_ai_disable_vqe(ot_audio_dev ai_dev, ot_ai_chn ai_chn);

hi_s32 ot_mpi_ai_enable_resample(ot_audio_dev ai_dev, ot_ai_chn ai_chn, ot_audio_sample_rate out_sample_rate);
hi_s32 ot_mpi_ai_disable_resample(ot_audio_dev ai_dev, ot_ai_chn ai_chn);

hi_s32 ot_mpi_ai_set_track_mode(ot_audio_dev ai_dev, ot_audio_track_mode track_mode);
hi_s32 ot_mpi_ai_get_track_mode(ot_audio_dev ai_dev, ot_audio_track_mode *track_mode);

hi_s32 ot_mpi_ai_save_file(ot_audio_dev ai_dev, ot_ai_chn ai_chn, const ot_audio_save_file_info *save_file_info);
hi_s32 ot_mpi_ai_query_file_status(ot_audio_dev ai_dev, ot_ai_chn ai_chn, ot_audio_file_status *file_status);

hi_s32 ot_mpi_ai_get_fd(ot_audio_dev ai_dev, ot_ai_chn ai_chn);

hi_s32 ot_mpi_ai_enable_aec_ref_frame(ot_audio_dev ai_dev, ot_ai_chn ai_chn, ot_audio_dev ao_dev, ot_ao_chn ao_chn);
hi_s32 ot_mpi_ai_disable_aec_ref_frame(ot_audio_dev ai_dev, ot_ai_chn ai_chn);

/* AO function api. */
hi_s32 ot_mpi_ao_set_pub_attr(ot_audio_dev ao_dev, ot_aio_attr *attr);
hi_s32 ot_mpi_ao_get_pub_attr(ot_audio_dev ao_dev, ot_aio_attr *attr);
hi_s32 ot_mpi_ao_clr_pub_attr(ot_audio_dev ao_dev);

hi_s32 ot_mpi_ao_enable(ot_audio_dev ao_dev);
hi_s32 ot_mpi_ao_disable(ot_audio_dev ao_dev);

hi_s32 ot_mpi_ao_enable_chn(ot_audio_dev ao_dev, ot_ao_chn ao_chn);
hi_s32 ot_mpi_ao_disable_chn(ot_audio_dev ao_dev, ot_ao_chn ao_chn);

hi_s32 ot_mpi_ao_pause_chn(ot_audio_dev ao_dev, ot_ao_chn ao_chn);
hi_s32 ot_mpi_ao_resume_chn(ot_audio_dev ao_dev, ot_ao_chn ao_chn);

hi_s32 ot_mpi_ao_send_frame(ot_audio_dev ao_dev, ot_ao_chn ao_chn, const ot_audio_frame *data, hi_s32 milli_sec);

hi_s32 ot_mpi_ao_enable_resample(ot_audio_dev ao_dev, ot_ao_chn ao_chn, ot_audio_sample_rate in_sample_rate);
hi_s32 ot_mpi_ao_disable_resample(ot_audio_dev ao_dev, ot_ao_chn ao_chn);

hi_s32 ot_mpi_ao_clr_chn_buf(ot_audio_dev ao_dev, ot_ao_chn ao_chn);
hi_s32 ot_mpi_ao_query_chn_status(ot_audio_dev ao_dev, ot_ao_chn ao_chn, ot_ao_chn_state *status);

hi_s32 ot_mpi_ao_get_chn_delay(ot_audio_dev ao_dev, ot_ao_chn ao_chn, hi_u32 *milli_sec);

hi_s32 ot_mpi_ao_set_volume(ot_audio_dev ao_dev, hi_s32 volume_db);
hi_s32 ot_mpi_ao_get_volume(ot_audio_dev ao_dev, hi_s32 *volume_db);

hi_s32 ot_mpi_ao_set_mute(ot_audio_dev ao_dev, hi_bool enable, const ot_audio_fade *fade);
hi_s32 ot_mpi_ao_get_mute(ot_audio_dev ao_dev, hi_bool *enable, ot_audio_fade *fade);

hi_s32 ot_mpi_ao_set_track_mode(ot_audio_dev ao_dev, ot_audio_track_mode track_mode);
hi_s32 ot_mpi_ao_get_track_mode(ot_audio_dev ao_dev, ot_audio_track_mode *track_mode);

hi_s32 ot_mpi_ao_get_fd(ot_audio_dev ao_dev, ot_ao_chn ao_chn);

hi_s32 ot_mpi_ao_set_vqe_attr(ot_audio_dev ao_dev, ot_ao_chn ao_chn, const ot_ao_vqe_cfg *vqe_cfg);
hi_s32 ot_mpi_ao_get_vqe_attr(ot_audio_dev ao_dev, ot_ao_chn ao_chn, ot_ao_vqe_cfg *vqe_cfg);

hi_s32 ot_mpi_ao_enable_vqe(ot_audio_dev ao_dev, ot_ao_chn ao_chn);
hi_s32 ot_mpi_ao_disable_vqe(ot_audio_dev ao_dev, ot_ao_chn ao_chn);

hi_s32 ot_mpi_ao_save_file(ot_audio_dev ao_dev, ot_ao_chn ao_chn, const ot_audio_save_file_info *save_file_info);
hi_s32 ot_mpi_ao_query_file_status(ot_audio_dev ao_dev, ot_ao_chn ao_chn, ot_audio_file_status *file_status);

/* AENC function api. */
hi_s32 ot_mpi_aenc_register_encoder(hi_s32 *handle, const ot_aenc_encoder *encoder);
hi_s32 ot_mpi_aenc_unregister_encoder(hi_s32 handle);

hi_s32 ot_mpi_aenc_create_chn(ot_aenc_chn ae_chn, const ot_aenc_chn_attr *attr);
hi_s32 ot_mpi_aenc_destroy_chn(ot_aenc_chn ae_chn);

hi_s32 ot_mpi_aenc_send_frame(ot_aenc_chn ae_chn, const ot_audio_frame *frm, const ot_aec_frame *aec_frm);

hi_s32 ot_mpi_aenc_get_stream(ot_aenc_chn ae_chn, ot_audio_stream *stream, hi_s32 milli_sec);
hi_s32 ot_mpi_aenc_release_stream(ot_aenc_chn ae_chn, const ot_audio_stream *stream_org);

hi_s32 ot_mpi_aenc_get_fd(ot_aenc_chn ae_chn);

hi_s32 ot_mpi_aenc_get_stream_buf_info(ot_aenc_chn ae_chn, hi_u64 *phys_addr, hi_u32 *size);

hi_s32 ot_mpi_aenc_set_mute(ot_aenc_chn ae_chn, hi_bool enable);
hi_s32 ot_mpi_aenc_get_mute(ot_aenc_chn ae_chn, hi_bool *enable);

hi_s32 ot_mpi_aenc_clr_chn_buf(ot_aenc_chn ae_chn);

/* ADEC function api. */
hi_s32 ot_mpi_adec_register_decoder(hi_s32 *handle, const ot_adec_decoder *decoder);
hi_s32 ot_mpi_adec_unregister_decoder(hi_s32 handle);

hi_s32 ot_mpi_adec_create_chn(ot_adec_chn ad_chn, const ot_adec_chn_attr *attr);
hi_s32 ot_mpi_adec_destroy_chn(ot_adec_chn ad_chn);

hi_s32 ot_mpi_adec_send_stream(ot_adec_chn ad_chn, const ot_audio_stream *stream, hi_bool block);

hi_s32 ot_mpi_adec_clr_chn_buf(ot_adec_chn ad_chn);

hi_s32 ot_mpi_adec_get_frame(ot_adec_chn ad_chn, ot_audio_frame_info *frame_info, hi_bool block);
hi_s32 ot_mpi_adec_release_frame(ot_adec_chn ad_chn, const ot_audio_frame_info *frame_info);

hi_s32 ot_mpi_adec_send_end_of_stream(ot_adec_chn ad_chn, hi_bool instant);
hi_s32 ot_mpi_adec_query_chn_status(ot_adec_chn ad_chn, ot_adec_chn_state *buffer_status);

hi_s32 ot_mpi_adec_get_frame_info(ot_adec_chn ad_chn, hi_void *decode_frame_info);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* OT_MPI_AUDIO_H */
