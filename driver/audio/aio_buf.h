/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2009-2020. All rights reserved.
 * Description: Header of aio buf driver
 * Author: Hisilicon multimedia software group
 * Create: 2009/5/5
 */

#ifndef AIO_BUF_H
#define AIO_BUF_H

#include "ot_type.h"
#include "securec.h"

typedef struct  {
    td_u8 *data;   /* 音频帧数据的虚拟地址 */
} audio_pack;

typedef struct  {
    audio_pack pack[OT_MAX_AUDIO_FRAME_NUM]; /* 存放pack的空间 */
    td_u32 pack_cnt; /* BUF的实际缓存帧数 */
    td_u32 pack_len; /* 一帧数据中的字节数 */
    td_u32 threshold; /* len的一半 */
    td_u32 threshold2; /* len的四分之一 */
    td_u32 write; /* BUF的读指针偏移 */
    td_u32 read; /* BUF的写指针偏移 */
    td_u32 seq;
    td_u32 direct; /* BUF empty: write == read && direct == 0; BUF full : write == read && direct == 1 */
} audio_buf;

/* 默认二级水线值 */
#define AIO_BUF_THRESHOLD2  15

#define aio_buf_reset(audio_buf) \
    do { \
        (audio_buf)->write = 0; \
        (audio_buf)->read = 0; \
        (audio_buf)->direct = 0; \
    } while (0)

/* 注意: pack内存的分配原则是假定每个采样点长度为4个字节 */
static inline td_void buf_init(audio_buf *buf, td_u32 *channel_data, td_u32 frm_num, td_u32 point_num)
{
    td_u32 index;
    (td_void)memset_s(buf, sizeof(audio_buf), 0, sizeof(audio_buf));
    buf->pack_cnt = frm_num;
    buf->pack_len = point_num;
    buf->threshold = frm_num >> 1;
    buf->threshold2 = (buf->threshold) >> 1;
    aio_buf_reset(buf);
    for (index = 0; index < buf->pack_cnt; index++) {
        buf->pack[index].data = (td_u8 *)(td_void *)channel_data + index * point_num;
    }
}

/* 判断缓冲区是否为满，1、写指针跟上读指针 */
#define buf_is_full(audio_buf) \
    (((audio_buf)->read == (audio_buf)->write) && ((audio_buf)->direct == 1))

/* 获取缓冲区的写指针(必须先判断是否非满) */
#define buf_get_write(audio_buf) \
    ((audio_buf)->pack[(audio_buf)->write])

/* 更新缓冲区的写指针 */
#define buf_finish_write(audio_buf) \
    do { \
        if (++((audio_buf)->write) == ((audio_buf)->pack_cnt)) { \
            (audio_buf)->write = 0; \
        } \
        if (((audio_buf)->write) == ((audio_buf)->read)) { \
            (audio_buf)->direct = 1; \
        } \
    } while (0)

#define buf_free_count(audio_buf) \
    (((audio_buf)->read == (audio_buf)->write) ? \
     (((audio_buf)->direct == 0) ? (audio_buf)->pack_cnt:0) : \
     (((audio_buf)->read > (audio_buf)->write) ? \
     ((audio_buf)->read - (audio_buf)->write) : \
     ((audio_buf)->read + (audio_buf)->pack_cnt - (audio_buf)->write)))

/* 判断缓冲区是否为空，即写指针与读指针相等 */
#define buf_is_empty(audio_buf) \
    (((audio_buf)->write == (audio_buf)->read) && ((audio_buf)->direct == 0))

/* 获取缓冲区的读指针(必须先判断是否非空) */
#define buf_get_read(audio_buf) \
    ((audio_buf)->pack[(audio_buf)->read])

/* 更新缓冲区的读指针 */
#define buf_finish_read(audio_buf) \
    do { \
        if (++((audio_buf)->read) == ((audio_buf)->pack_cnt)) { \
            (audio_buf)->read = 0; \
        } \
        if (((audio_buf)->read) == ((audio_buf)->write)) { \
            (audio_buf)->direct = 0; \
        } \
    } while (0)

#define buf_busy_count(audio_buf) \
    (((audio_buf)->read == (audio_buf)->write) ? \
     (((audio_buf)->direct == 1) ? (audio_buf)->pack_cnt:0) : \
     (((audio_buf)->write >= (audio_buf)->read) ? \
     ((audio_buf)->write - (audio_buf)->read) : ( \
     (audio_buf)->write + (audio_buf)->pack_cnt - (audio_buf)->read)))

#define buf_is_overflow(audio_buf) \
    ((BUF_BUSY_COUNT(audio_buf) >= (audio_buf)->threshold) && \
     (BUF_BUSY_COUNT(audio_buf) < (audio_buf)->threshold2))

#define buf_is_overflow2(audio_buf) \
    (BUF_BUSY_COUNT(audio_buf) >= (audio_buf)->threshold2)

#define buf_is_underflow(audio_buf) \
    (BUF_BUSY_COUNT(audio_buf) < (audio_buf)->threshold)

#define buf_is_underflow2(audio_buf) \
    ((BUF_BUSY_COUNT(audio_buf) >= (audio_buf)->threshold) && \
     (BUF_BUSY_COUNT(audio_buf) < (audio_buf)->threshold2))

#endif
