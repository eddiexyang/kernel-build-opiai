/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2011-2020. All rights reserved.
 * Description: audio fb driver
 * Author: Hisilicon multimedia software group
 * Create: 2011/04/21
 */

#include "audio_fb.h"
#include "hi_osal.h"
#include "securec.h"
#include "mkp_aio.h"

td_s32 audio_fb_init(td_u32 depth, audio_frame_buf *buf)
{
    td_u32 i;
    audio_buf_blk *node = TD_NULL;

    audio_fb_check_null_ptr_return(buf);

    OSAL_INIT_LIST_HEAD(&buf->free_list);
    OSAL_INIT_LIST_HEAD(&buf->busy_list);

    node = (audio_buf_blk *)osal_vmalloc(sizeof(audio_buf_blk) * depth);
    if (node == TD_NULL) {
        return TD_FAILURE;
    }
    (td_void)memset_s(node, sizeof(audio_buf_blk) * depth, 0, sizeof(audio_buf_blk) * depth);

    /* 记录分配的buf地址 */
    buf->buf_vir_addr = node;

    /* 将所有节点挂到buf的freelist上 */
    for (i = 0; i < depth; i++, node++) {
        osal_list_add_tail(&node->list, &buf->free_list);
    }

    buf->busy_num = 0;
    buf->free_num = depth;

    return TD_SUCCESS;
}

audio_buf_blk *audio_fb_get_free(audio_frame_buf *buf)
{
    struct osal_list_head *list = TD_NULL;
    audio_buf_blk *node = TD_NULL;

    if (buf == TD_NULL) {
        return TD_NULL;
    }

    if (osal_list_empty(&buf->free_list)) {
        return TD_NULL;
    }

    list = buf->free_list.next;
    osal_list_del(list);
    buf->free_num--;

    node = osal_list_entry(list, audio_buf_blk, list);
    return node;
}

audio_buf_blk *audio_fb_get_busy(audio_frame_buf *buf)
{
    struct osal_list_head *list = TD_NULL;
    audio_buf_blk *node = TD_NULL;

    if (buf == TD_NULL) {
        return TD_NULL;
    }

    if (osal_list_empty(&buf->busy_list)) {
        return TD_NULL;
    }

    list = buf->busy_list.next;
    osal_list_del(list);
    buf->busy_num--;

    node = osal_list_entry(list, audio_buf_blk, list);
    return node;
}

td_s32 audio_fb_put_free(audio_frame_buf *buf, audio_buf_blk *node)
{
    audio_fb_check_null_ptr_return(buf);
    audio_fb_check_null_ptr_return(node);

    osal_list_add_tail(&node->list, &buf->free_list);

    buf->free_num++;

    return TD_SUCCESS;
}

td_s32 audio_fb_put_busy(audio_frame_buf *buf, audio_buf_blk *node)
{
    audio_fb_check_null_ptr_return(buf);
    audio_fb_check_null_ptr_return(node);

    osal_list_add_tail(&node->list, &buf->busy_list);

    buf->busy_num++;

    return TD_SUCCESS;
}

td_u32 audio_fb_query_busy_num(const audio_frame_buf *buf)
{
    audio_fb_check_null_ptr_return(buf);

    return buf->busy_num;
}
