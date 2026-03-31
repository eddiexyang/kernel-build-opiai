/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2011-2020. All rights reserved.
 * Description: Header of audio fb driver
 * Author: Hisilicon multimedia software group
 * Create: 2011/04/21
 */

#ifndef AUDIO_FB_H
#define AUDIO_FB_H

#include "hi_osal.h"
#include "audio_inner.h"

#define audio_fb_check_null_ptr_return(ptr) \
    do {                                    \
        if ((ptr) == TD_NULL) {             \
            return TD_FAILURE;      \
        }                                   \
    } while (0)

typedef struct {
    struct osal_list_head list;
    audio_frame_combine audio_frm;
} audio_buf_blk;

typedef struct {
    td_void *buf_vir_addr;

    struct osal_list_head free_list;
    struct osal_list_head busy_list;

    volatile td_u32 free_num;
    volatile td_u32 busy_num;
} audio_frame_buf;

td_s32 audio_fb_init(td_u32 depth, audio_frame_buf *buf);

static inline td_s32 audio_fb_release(audio_frame_buf *buf)
{
    audio_fb_check_null_ptr_return(buf);
    if (buf->buf_vir_addr != TD_NULL) {
        osal_vfree(buf->buf_vir_addr);
        buf->buf_vir_addr = TD_NULL;
    }

    return TD_SUCCESS;
}

audio_buf_blk *audio_fb_get_free(audio_frame_buf *buf);
audio_buf_blk *audio_fb_get_busy(audio_frame_buf *buf);
td_s32 audio_fb_put_free(audio_frame_buf *buf, audio_buf_blk *node);
td_s32 audio_fb_put_busy(audio_frame_buf *buf, audio_buf_blk *node);

static inline td_u32 audio_fb_query_free_num(const audio_frame_buf *buf)
{
    audio_fb_check_null_ptr_return(buf);

    return buf->free_num;
}

td_u32 audio_fb_query_busy_num(const audio_frame_buf *buf);

#endif /* end of #ifndef AUDIO_FB_H */
