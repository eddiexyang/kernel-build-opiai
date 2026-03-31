/*
* Copyright (c) Hisilicon Technologies Co., Ltd. 2011-2019. All rights reserved.
* Description:
* Author: Hisilicon multimedia software group
* Create: 2011/06/28
*/

#ifndef __VALG_EXT_H__
#define __VALG_EXT_H__

#include "hi_type.h"
#include "hi_debug.h"
#include "hi_osal.h"
#include "securec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

// I帧头部包含VPS,SPS,PPS,SEI这些slice的最大长度
#define MAX_HEADER_SLICE_LEN 640U

/************************************************************************
  module:         valg_quenue
  Description:    circular quenue
  Author:
*************************************************************************/
typedef struct {
    hi_ulong *base; /* queue base addr    */
    hi_s32 max_len; /* queue max length   */
    hi_s32 tail; /* queue tail pointer */
    hi_s32 head; /* queue head pointer */
    hi_s32 real_len; /* queue real lentth  */
} valg_queue;

__inline static hi_void valg_queue_init(valg_queue *que, hi_void *base, hi_s32 max_len);
__inline static hi_s32 valg_queue_put_to_head(valg_queue *que, hi_ulong ul_data);
__inline static hi_s32 valg_queue_get_from_head(valg_queue *que, hi_ulong *data);
__inline static hi_s32 valg_queue_put_to_tail(valg_queue *que, hi_ulong data);
__inline static hi_s32 valg_queue_get_from_tail(valg_queue *que, hi_ulong *data);
__inline static hi_s32 valg_queue_search(const valg_queue *que, hi_ulong *data, hi_s32 *index);
__inline static hi_s32 valg_queue_search_from_head(const valg_queue *que, hi_ulong *data, hi_s32 index);
__inline static hi_s32 valg_queue_search_from_tail(const valg_queue *que, hi_ulong *data, hi_s32 index);
__inline static hi_s32 valg_queue_get_real_len(const valg_queue *que);
__inline static hi_bool valg_queue_is_full(const valg_queue *que);
__inline static hi_bool valg_queue_is_empty(const valg_queue *que);

/* 读循环buffer的输出数据类型 */
typedef struct {
    hi_void *src[2];
    hi_u64 phy_addr[2];
    hi_u32 len[2];
} valg_cb_rdinfo;

/* 写循环buffer的输出数据类型 */
typedef struct {
    hi_void *dst[2];
    hi_u32 len[2];
} valg_cb_wrinfo;

/* 循环buffer */
typedef struct {
    hi_u64 phy_base; /* 0循环buffer首地址物理地址, call valg_cb_set_phy_base to set. */

    hi_u32 buf_len; /* 8循环buffer的总长度，单位为字节 */
    hi_u32 rsv_byte; /* 12用于判断buffer空满的保留空间，单位为字节 */
    hi_u64 ctl_iova_base; /* 16循环buffer控制结构体 iova首地址, call valg_cb_set_ctl_base to set. */
    hi_void *ctl_kva_base; /* 24循环buffer控制结构体 kva首地址, call valg_cb_set_ctl_base to set. */
    hi_u32 au32res0[8]; /* 32占空位 */
    hi_u32 rd_head; /* 64循环buffer读头，偏移量，单位为字节 */
    hi_u32 au32res1[15]; /* 68占空位 */
    hi_u32 rd_tail; /* 128循环buffer读尾，偏移量，单位为字节 */
    hi_u32 au32res2[15]; /* 132占空位 */
    hi_u32 wr_tail; /* 192循环buffer写头，偏移量，单位为字节 */
    hi_u32 au32res3[15]; /* 196占空位 */
    hi_u32 wr_head; /* 256循环buffer写尾，偏移量，单位为字节 */
    hi_u32 au32res4[15]; /* 260占空位,逻辑数据的写出每次是以128bit为单位,所以这里至少128bit */
    hi_u32 *read_head; /* 320循环buffer读头，偏移量，单位为字节 */
    hi_u32 *read_tail; /* 328循环buffer读尾，偏移量，单位为字节 */
    hi_u32 *write_head; /* 336循环buffer写头，偏移量，单位为字节 */
    hi_u32 *write_tail; /* 344循环buffer写尾，偏移量，单位为字节 */
    hi_void *base; /* 352虚拟基地址 */
    /* 360 */
} valg_crcl_buf;

/* 供外部调用的接口函数 */
__inline static hi_s32 valg_cb_init_ex(valg_crcl_buf *cb, hi_void *virt_base, hi_u32 buf_len, hi_u32 rsv_byte,
                                       hi_u32 *rd_head, hi_u32 *rd_tail, hi_u32 *wr_head, hi_u32 *wr_tail);
__inline static hi_s32 valg_cb_init(valg_crcl_buf *cb, hi_void *virt_base, hi_u32 buf_len, hi_u32 rsv_byte);
__inline static hi_s32 valg_cb_init_master_ex(valg_crcl_buf *dest_cb, valg_crcl_buf *ref_cb,
                                              hi_void *virt_base);
__inline static hi_s32 valg_cb_init_master(valg_crcl_buf *dest_cb, valg_crcl_buf *ref_cb, hi_void *virt_base);
__inline static hi_s32 valg_cb_write(const valg_crcl_buf *cb, hi_void *virt_src, hi_u32 wr_len);
__inline static hi_s32 valg_cb_read(const valg_crcl_buf *cb, hi_u32 rd_len, valg_cb_rdinfo *rd_info);
__inline static hi_s32 valg_cb_update_rp(const valg_crcl_buf *cb, hi_u32 rd_len);
__inline static hi_void valg_cb_update_wp(const valg_crcl_buf *cb);
__inline static hi_u32 valg_cb_retrace_wp(const valg_crcl_buf *cb); //lint !e618
__inline static hi_void valg_cb_back_wp(const valg_crcl_buf *cb);
__inline static hi_void *valg_cb_get_rd_head(const valg_crcl_buf *cb);
__inline static hi_void *valg_cb_get_rd_tail(const valg_crcl_buf *cb);
__inline static hi_u32 valg_cb_get_data_len(const valg_crcl_buf *cb);
__inline static hi_u32 valg_cb_get_free_len(const valg_crcl_buf *cb);
__inline static hi_u32 valg_cb_rd_region_len(const valg_crcl_buf *cb);
__inline static hi_u32 valg_cb_wr_region_len(const valg_crcl_buf *cb);
__inline static hi_void valg_cb_reset(valg_crcl_buf *cb);
__inline static hi_void valg_cb_set_phy_base(valg_crcl_buf *cb, hi_u64 phy_base);

/************************************************
 处理器相关的定义

 注意事项：
 1)所有的长度单位均是处理器的寻址步长，ARM处理器的地址单位是"字节";
 2)所有的读写操作按照word进行，所以要求：
   buffer中数据包的长度应按word对齐;
   队列中每个元素的长度应按word对齐;
*************************************************/
/* 处理器的寻址步长。表示地址长度的单位 */
typedef hi_u8 ADDR_UNIT;

/* 处理器字长。表示读写操作的单位 */
typedef hi_u64 CPU_WORD;

/* 每一个word包含的字节数 */
#define WORD_ALIGN       0x04U

__inline static hi_void valg_queue_init(valg_queue *que, hi_void *base, hi_s32 max_len)
{
    CHECK_DO_SOMETHING(que == NULL, HI_ERR_TRACE(HI_ID_VB, "que is NULL\n"); return);
    CHECK_DO_SOMETHING(base == NULL, HI_ERR_TRACE(HI_ID_VB, "base is NULL\n"); return);
    CHECK_DO_SOMETHING(max_len <= 0, HI_ERR_TRACE(HI_ID_VB, "max_len[%d] is abnormal\n", max_len); return);

    que->base = (hi_ulong *)base;
    que->max_len = max_len;
    que->tail = 0;
    que->head = 0;
    que->real_len = 0;
}

__inline static hi_s32 valg_queue_put_to_head(valg_queue *que, hi_ulong ul_data)
{
    hi_s32 head;
    hi_s32 real_len;

    CHECK_DO_SOMETHING(que == NULL, HI_ERR_TRACE(HI_ID_VB, "que is NULL\n"); return HI_FAILURE);

    head = que->head;
    real_len = que->real_len;

    if (real_len < que->max_len) {
        que->base[head] = ul_data;
        if ((++head) >= que->max_len) {
            head = 0;
        }
        que->head = head;
        que->real_len = (real_len + 1);

        return HI_SUCCESS;
    }

    return HI_FAILURE;
}

__inline static hi_s32 valg_queue_get_from_head(valg_queue *que, hi_ulong *data)
{
    hi_s32 head;
    hi_s32 real_len;

    CHECK_DO_SOMETHING(que == NULL, HI_ERR_TRACE(HI_ID_VB, "que is NULL\n"); return HI_FAILURE);
    CHECK_DO_SOMETHING(data == NULL, HI_ERR_TRACE(HI_ID_VB, "data is NULL\n"); return HI_FAILURE);

    head = que->head;
    real_len = que->real_len;

    if (real_len > 0) {
        if ((--head) < 0) {
            head += que->max_len;
        }
        *data = que->base[head];
        que->head = head;
        que->real_len = (real_len - 1);

        return HI_SUCCESS;
    }

    return HI_FAILURE;
}

__inline static hi_s32 valg_queue_put_to_tail(valg_queue *que, hi_ulong data)
{
    hi_s32 tail;
    hi_s32 real_len;

    CHECK_DO_SOMETHING(que == NULL, HI_ERR_TRACE(HI_ID_VB, "que is NULL\n"); return HI_FAILURE);

    tail = que->tail;
    real_len = que->real_len;

    if (real_len < que->max_len) {
        if ((--tail) < 0) {
            tail += que->max_len;
        }
        que->base[tail] = data;

        que->tail = tail;
        que->real_len = (real_len + 1);

        return HI_SUCCESS;
    }

    return HI_FAILURE;
}

__inline static hi_s32 valg_queue_get_from_tail(valg_queue *que, hi_ulong *data)
{
    hi_s32 tail;
    hi_s32 real_len;

    CHECK_DO_SOMETHING(que == NULL, HI_ERR_TRACE(HI_ID_VB, "que is NULL\n"); return HI_FAILURE);
    CHECK_DO_SOMETHING(data == NULL, HI_ERR_TRACE(HI_ID_VB, "data is NULL\n"); return HI_FAILURE);

    tail = que->tail;
    real_len = que->real_len;

    if (real_len > 0) {
        *data = que->base[tail];
        if ((++tail) >= que->max_len) {
            tail = 0;
        }
        que->tail = tail;
        que->real_len = (real_len - 1);

        return HI_SUCCESS;
    }

    return HI_FAILURE;
}

__inline static hi_s32 valg_queue_search(const valg_queue *que, hi_ulong *data, hi_s32 *index)
{
    hi_s32 tail;
    hi_s32 real_len;

    CHECK_DO_SOMETHING(que == NULL, HI_ERR_TRACE(HI_ID_VB, "que is NULL\n"); return HI_FAILURE);
    CHECK_DO_SOMETHING(data == NULL, HI_ERR_TRACE(HI_ID_VB, "data is NULL\n"); return HI_FAILURE);
    CHECK_DO_SOMETHING(index == NULL, HI_ERR_TRACE(HI_ID_VB, "index is NULL\n"); return HI_FAILURE);

    tail = que->tail + (*index);
    real_len = que->real_len;

    if ((real_len > 0) && ((*index) < real_len)) {
        if ((tail) >= que->max_len) {
            tail -= que->max_len;
        }

        *data = que->base[tail];

        (*index)++;

        return HI_SUCCESS;
    }

    return HI_FAILURE;
}

__inline static hi_s32 valg_queue_search_from_head(const valg_queue *que, hi_ulong *data, hi_s32 index)
{
    hi_s32 head;
    hi_s32 real_len;

    CHECK_DO_SOMETHING(que == NULL, HI_ERR_TRACE(HI_ID_VB, "que is NULL\n"); return HI_FAILURE);
    CHECK_DO_SOMETHING(data == NULL, HI_ERR_TRACE(HI_ID_VB, "data is NULL\n"); return HI_FAILURE);
    CHECK_DO_SOMETHING(index < 0, HI_ERR_TRACE(HI_ID_VB, "index[%d] is invalid\n", index); return HI_FAILURE);

    head = que->head - index - 1;
    real_len = que->real_len;

    if ((real_len > 0) && (index < real_len)) {
        if ((head) < 0) {
            head += que->max_len;
        }

        *data = que->base[head];

        return HI_SUCCESS;
    }
    return HI_FAILURE;
}

__inline static hi_s32 valg_queue_search_from_tail(const valg_queue *que, hi_ulong *data, hi_s32 index)
{
    hi_s32 tail;
    hi_s32 real_len;

    CHECK_DO_SOMETHING(que == NULL, HI_ERR_TRACE(HI_ID_VB, "que is NULL\n"); return HI_FAILURE);
    CHECK_DO_SOMETHING(data == NULL, HI_ERR_TRACE(HI_ID_VB, "data is NULL\n"); return HI_FAILURE);
    CHECK_DO_SOMETHING(index < 0, HI_ERR_TRACE(HI_ID_VB, "index[%d] is invalid\n", index); return HI_FAILURE);

    tail = que->tail + index;
    real_len = que->real_len;

    if ((real_len > 0) && (index < real_len)) {
        if ((tail) >= que->max_len) {
            tail -= que->max_len;
        }

        *data = que->base[tail];

        return HI_SUCCESS;
    }

    return HI_FAILURE;
}

__inline static hi_s32 valg_queue_get_real_len(const valg_queue *que)
{
    CHECK_DO_SOMETHING(que == NULL, HI_ERR_TRACE(HI_ID_VB, "que is NULL\n"); return HI_FAILURE);

    return que->real_len;
}

__inline static hi_bool valg_queue_is_full(const valg_queue *que)
{
    CHECK_DO_SOMETHING(que == NULL, HI_ERR_TRACE(HI_ID_VB, "que is NULL\n"); return HI_FALSE);

    return (hi_bool)(que->real_len == que->max_len);
}

__inline static hi_s32 valg_queue_get_from_head_for_index(valg_queue *que, hi_ulong *data, hi_s32 index)
{
    hi_s32 head, tail;
    hi_s32 real_len;
    hi_s32 i = 0;

    CHECK_DO_SOMETHING(que == NULL, HI_ERR_TRACE(HI_ID_VB, "que is NULL\n"); return HI_FAILURE);
    CHECK_DO_SOMETHING(data == NULL, HI_ERR_TRACE(HI_ID_VB, "data is NULL\n"); return HI_FAILURE);

    head = que->head - index - 1;
    real_len = que->real_len;
    tail = que->tail;

    if ((real_len > 0) && (index < real_len)) {
        if ((head) < 0) {
            head += que->max_len;
        }

        *data = que->base[head];
        que->real_len = (real_len - 1);

        if (head > tail) {
            for (i = head; i > tail; i--) {
                que->base[i] = que->base[i - 1];
            }
            if ((++tail) >= que->max_len) {
                tail = 0;
            }
            que->tail = tail;
        } else if (head < tail) {
            for (i = head; i > 0; i--) {
                que->base[i] = que->base[i - 1];
            }
            que->base[0] = que->base[que->max_len - 1];
            for (i = que->max_len - 1; i > tail; i--) {
                que->base[i] = que->base[i - 1];
            }
            if ((++tail) >= que->max_len) {
                tail = 0;
            }
            que->tail = tail;
        } else {
            if ((++tail) >= que->max_len) {
                tail = 0;
            }
            que->tail = tail;
        }

        return HI_SUCCESS;
    }

    return HI_FAILURE;
}

__inline static hi_bool valg_queue_is_empty(const valg_queue *que)
{
    CHECK_DO_SOMETHING(que == NULL, HI_ERR_TRACE(HI_ID_VB, "que is NULL\n"); return HI_FALSE);

    return (hi_bool)(que->real_len == 0);
}

#define NOT_SET_PHY_BASE 0U
#define VIRT_TO_PHY(va, V, P) ((va) - (V) + (P))
__inline static hi_s32 valg_cb_init_ex(valg_crcl_buf *cb,
                                       hi_void *virt_base,
                                       hi_u32 buf_len,
                                       hi_u32 rsv_byte,
                                       hi_u32 *rd_head,
                                       hi_u32 *rd_tail,
                                       hi_u32 *wr_head,
                                       hi_u32 *wr_tail)
{
    hi_s32 s32_ret = 0;

    if (((buf_len & (WORD_ALIGN - 1U)) != 0U) || (buf_len < rsv_byte)) {
        HI_TRACE(HI_DBG_ERR, HI_ID_VALG, "buf_len must be 4B aligned!");
        return HI_FAILURE;
    }

    if ((rsv_byte == 0) || ((rsv_byte & (WORD_ALIGN - 1)) != 0)) {
        HI_TRACE(HI_DBG_ERR, HI_ID_VALG, "rsv_byte must be 4B aligned!");
        return HI_FAILURE;
    }

    cb->base = virt_base;
    cb->phy_base = NOT_SET_PHY_BASE;
    cb->read_head = rd_head;
    cb->read_tail = rd_tail;
    cb->write_head = wr_head;
    cb->write_tail = wr_tail;

    cb->buf_len = buf_len;
    cb->rsv_byte = rsv_byte;
    *(cb->read_head) = 0;
    *(cb->read_tail) = 0;
    *(cb->write_head) = 0;
    *(cb->write_tail) = 0;
    s32_ret = memset_s(cb->base, cb->buf_len, 0, cb->buf_len);
    if (s32_ret != 0) {
        HI_TRACE(HI_DBG_WARN, HI_ID_VALG, "memset_s failed! ret = %d\n", s32_ret);
    }
    return HI_SUCCESS;
}
/* 从arm侧的初始化使用,使用本身的默认内存 */
__inline static hi_s32 valg_cb_init(valg_crcl_buf *cb,
                                    hi_void *virt_base,
                                    hi_u32 buf_len,
                                    hi_u32 rsv_byte)
{
    return valg_cb_init_ex(cb,
                           virt_base,
                           buf_len,
                           rsv_byte,
                           &(cb->rd_head),
                           &(cb->rd_tail),
                           &(cb->wr_head),
                           &(cb->wr_tail));
}
/* 此初始化用在主arm侧的编码初始化 ，此初始化方式比较特殊，使用中需要注意使用场合 */
__inline static hi_s32 valg_cb_init_master_ex(valg_crcl_buf *dest_cb,
                                              valg_crcl_buf *ref_cb,
                                              hi_void *virt_base)
{
    return valg_cb_init_ex(dest_cb,
                           virt_base,
                           ref_cb->buf_len,
                           ref_cb->rsv_byte,
                           &(dest_cb->rd_head), /* 使用主arm结构体中的读头指针作为读头指针 */
                           &(ref_cb->rd_head), /* 使用从arm结构体中的读头指针作为读尾指针,
                                                  因为只有主arm释放完成后，从arm才能释放,所以用从的读头作为主的读尾 */
                           &(ref_cb->wr_head), /* 使用从arm结构体中的写头指针作为写头指针 */
                           &(ref_cb->wr_tail)); /* 使用从arm结构体中的写尾指针作为写尾指针 */
}

/* 此初始化方式用在主arm侧的解码初始化 */
__inline static hi_s32 valg_cb_init_master(valg_crcl_buf *dest_cb,
                                           valg_crcl_buf *ref_cb,
                                           hi_void *virt_base)
{
    return valg_cb_init_ex(dest_cb,
                           virt_base,
                           ref_cb->buf_len,
                           ref_cb->rsv_byte,
                           &(ref_cb->rd_head),
                           &(ref_cb->rd_tail),
                           &(ref_cb->wr_head),
                           &(ref_cb->wr_tail));
}

__inline static hi_void valg_cb_set_phy_base(valg_crcl_buf *cb, hi_u64 phy_base)
{
    cb->phy_base = phy_base;
}

__inline static hi_void valg_cb_set_ctl_base(valg_crcl_buf *cb, hi_u64 iova_base, hi_void *kva_base)
{
    cb->ctl_iova_base = iova_base;
    cb->ctl_kva_base = kva_base;
}

__inline static hi_s32 valg_cb_write(const valg_crcl_buf *cb, hi_void *virt_src, hi_u32 wr_len)
{
    hi_u32 free_len;
    valg_cb_wrinfo wr_info;
    hi_u32 wr_head_new = 0;
    hi_u32 rd_tail = (*(cb->read_tail));
    hi_u32 wr_head = (*(cb->write_head));
#ifdef __HuaweiLite__
    hi_s32 ret = 0;
#endif

    if ((wr_len & (WORD_ALIGN - 1U)) != 0U) {
        HI_TRACE(HI_DBG_ERR, HI_ID_VALG, "\n_length writing in must be 4B aligned!");
        return HI_FAILURE;
    }

    /* 调用获取buffer数据长度函数，获取buffer中的实际长度; */
    if (wr_head >= rd_tail) { /* 写头在读尾前面，说明buffer未折回 */
        /* 计算写头未折回时buffer中的数据长度; */
        free_len = cb->buf_len - (wr_head - rd_tail) - cb->rsv_byte;
    } else {
        /* 计算写头折回时buffer中的数据长度; */
        free_len = rd_tail - wr_head - cb->rsv_byte;
    }

    /* 剩余空间不够写入新的数据包 */
    if (free_len < wr_len) {
        HI_TRACE(HI_DBG_WARN, HI_ID_VALG, "\n_warning! buffer left is small than the data writing in!");
        return HI_FAILURE;
    }

    /* 调用写头前进函数，获取可以写入的两段数据包长度; */
    /* copy from valg_cb_wr_head_run */
    /* 根据写入数据后的写头是否超过buffer底部，来判断循环buffer是否折回; */
    if ((wr_head + wr_len) >= cb->buf_len) /* buffer折回,恰好到达buffer底部也要折回 */ {
        /* 计算两段数据包的地址和长度 */
        wr_info.dst[0] = (ADDR_UNIT *)cb->base + wr_head;
        wr_info.len[0] = cb->buf_len - wr_head;
        wr_info.dst[1] = cb->base;
        wr_info.len[1] = wr_len - wr_info.len[0];
        wr_head_new = wr_info.len[1]; /* 写头指针指向下一数据包起始位置;  */
    } else {
        wr_info.dst[0] = (ADDR_UNIT *)cb->base + wr_head;
        wr_info.len[0] = wr_len;
        wr_info.dst[1] = (ADDR_UNIT *)wr_info.dst[0] + wr_len;
        wr_info.len[1] = 0; /* 第二段数据包长度等于0; */
        wr_head_new = wr_head + wr_len; /* 写头指针指向下一数据包起始位置; */
    }

    /* copy end */

    {
        hi_u32 i;
        CPU_WORD *dst;
        CPU_WORD *src;

        /* 第一段输入数据复制到buffer; */
        src = (CPU_WORD *)virt_src;
        dst = (CPU_WORD *)wr_info.dst[0];
        i = wr_info.len[0] / sizeof(CPU_WORD);

#ifdef __HuaweiLite__

        while (i--) {
            ret = memcpy_s(dst, sizeof(CPU_WORD), src, sizeof(CPU_WORD));
            if (ret != 0) {
                HI_TRACE(HI_DBG_WARN, HI_ID_VALG, "memcpy_s fail! ret = %d\n", ret);
            }
            dst++;
            src++;
        }

#else
        while (i--) {
            *dst++ = *src++;
        }
#endif

        /* 第二段输入数据复制到buffer; */
        dst = (CPU_WORD *)wr_info.dst[1];
        i = wr_info.len[1] / sizeof(CPU_WORD);
#ifdef __HuaweiLite__
        while (i--) {
            ret = memcpy_s(dst, sizeof(CPU_WORD), src, sizeof(CPU_WORD));
            if (ret != 0) {
                HI_TRACE(HI_DBG_WARN, HI_ID_VALG, "memcpy_s fail! ret = %d\n", ret);
            }
            dst++;
            src++;
        }

#else
        while (i--) {
            *dst++ = *src++;
        }
#endif
    }

    (*(cb->write_head)) = wr_head_new;
    return HI_SUCCESS;
}

static __inline hi_s32 valg_cb_read_jpege(const valg_crcl_buf *cb, hi_u32 rd_len)
{
    hi_u32 rh;
    hi_u32 rd_head = (*(cb->read_head));
    hi_u32 wr_tail = (*(cb->write_tail));

    /* 判断输入的读取长度是否正确。
     * 要求:更新后的读头不超过写尾。
     * 注意区分两种情况:
     * 1、写尾指针未发生回转。需判断;
     * 2、写尾指针发生回转。
     *    1)读头没有回转，读头不会超过写尾，无需判断;
     *    2)读头也发生回转，需判断;
     */
    // 其实当前jpege循环缓冲区适配情况，不会出现回转情况
    rh = rd_head + rd_len;
    if (rd_head <= wr_tail) {
        if (rh > wr_tail) {
            HI_TRACE(HI_DBG_ERR, HI_ID_VALG, "VALG_CB_Read rd_head:%u wr_tail:%u rh:%u\n", rd_head, wr_tail, rh);
            return HI_FAILURE;
        }
    } else if (rh >= cb->buf_len) {
        rh -= cb->buf_len;
        if (rh > wr_tail) {
            HI_TRACE(HI_DBG_ERR, HI_ID_VALG, "VALG_CB_Read buf_len:%u wr_tail:%u rh:%u\n", cb->buf_len, wr_tail, rh);
            return HI_FAILURE;
        }
    }

    /* 根据输入的读出数据包长度，判断循环buffer是否折回; */
    if ((rd_head + rd_len) > cb->buf_len) /* buffer折回，恰好到达buffer底部也要折回 */ {
        rd_head = (rd_head + rd_len) - cb->buf_len;
    } else if ((rd_head + rd_len) == cb->buf_len) { // 空pack情况
        rd_head = 0;
    } else { // 软件替硬件移动码流长度情况
        rd_head += rd_len; /* 读头指针指向下一数据包起始位置; */
    }

    (*(cb->read_head)) = rd_head;
    return HI_SUCCESS;
}
__inline static hi_s32 valg_cb_read_with_slice(
    const valg_crcl_buf *cb, hi_u32 rd_len, valg_cb_rdinfo *rd_info, hi_u8 last_slice)
{
    hi_u32 rh;
    hi_u32 rd_head = (*(cb->read_head));
    hi_u32 wr_tail = (*(cb->write_tail));
    hi_bool fream_end = HI_FALSE;
    if ((last_slice & 0x1U) != 0U) {
        fream_end = HI_TRUE;
    }

    /* 判断输入的读取长度是否正确。
     * 要求:更新后的读头不超过写尾。
     * 注意区分两种情况:
     * 1、写尾指针未发生回转。需判断;
     * 2、写尾指针发生回转。
     *    1)读头没有回转，读头不会超过写尾，无需判断;
     *    2)读头也发生回转，需判断;
     */
    rh = rd_head + rd_len;
    if (rd_head <= wr_tail) { /* 读头指针 < 写尾指针，buffer未折回 */
        if (rh > wr_tail) {   /* 读尾 > 写尾指针，读的数据太多 */
            HI_TRACE(HI_DBG_ERR, HI_ID_VALG, "VALG_CB_Read rd_head:%u wr_tail:%u rh:%u\n", rd_head, wr_tail, rh);
            return HI_FAILURE;
        }
    } else if (rh >= cb->buf_len) { /* 读尾 > buffer长度，buffer已经折回 */
        rh -= cb->buf_len;
        if (rh > wr_tail) {
            HI_TRACE(HI_DBG_ERR, HI_ID_VALG, "VALG_CB_Read buf_len:%u wr_tail:%u rh:%u\n", cb->buf_len, wr_tail, rh);
            return HI_FAILURE;
        }
    }

    /* 根据输入的读出数据包长度，判断循环buffer是否折回; */
    if (((rd_head + rd_len) >= cb->buf_len) && fream_end) { /* buffer折回，恰好到达buffer底部也要折回 */
        /* 计算两段数据包地址和长度 */
        rd_info->src[0] = (ADDR_UNIT *)cb->base + rd_head;
        rd_info->len[0] = cb->buf_len - rd_head;
        rd_info->src[1] = cb->base; /* 第二段数据包地址等于buffer首地址 */
        rd_info->len[1] = rd_len - rd_info->len[0];
        rd_head = rd_info->len[1]; /* 读头指针指向下一数据包起始位置 */
    } else {
        rd_info->src[0] = (ADDR_UNIT *)cb->base + rd_head;
        rd_info->len[0] = rd_len;
        rd_info->src[1] = (ADDR_UNIT *)rd_info->src[0] + rd_len;
        rd_info->len[1] = 0; /* 第二段数据包长度为0 */
        rd_head += rd_len; /* 读头指针指向下一数据包起始位置; */
    }

    /* 虚拟地址转换为物理地址 */
    if (cb->phy_base == NOT_SET_PHY_BASE) {
        /* if phy base addr equal 0, means not set, must return 0. */
        rd_info->phy_addr[0] = NOT_SET_PHY_BASE;
        rd_info->phy_addr[1] = NOT_SET_PHY_BASE;
    } else {
        /* 只要中间值不溢出，无符号数/有符号数加减不影响最后结果 */
        rd_info->phy_addr[0] = VIRT_TO_PHY((hi_u64)(hi_uintptr_t)rd_info->src[0],
                                              (hi_u64)(hi_uintptr_t)cb->base, cb->phy_base);
        rd_info->phy_addr[1] = VIRT_TO_PHY((hi_u64)(hi_uintptr_t)rd_info->src[1],
                                              (hi_u64)(hi_uintptr_t)cb->base, cb->phy_base);
    }

    (*(cb->read_head)) = rd_head;
    return HI_SUCCESS;
}

__inline static hi_s32 valg_cb_read(const valg_crcl_buf *cb, hi_u32 rd_len, valg_cb_rdinfo *rd_info)
{
    hi_u32 rh;
    hi_u32 rd_head = (*(cb->read_head));
    hi_u32 wr_tail = (*(cb->write_tail));

    /* 判断输入的读取长度是否正确。
     * 要求:更新后的读头不超过写尾。
     * 注意区分两种情况:
     * 1、写尾指针未发生回转。需判断;
     * 2、写尾指针发生回转。
     *    1)读头没有回转，读头不会超过写尾，无需判断;
     *    2)读头也发生回转，需判断;
     */
    rh = rd_head + rd_len;
    if (rd_head <= wr_tail) {	/* 读头指针 < 写尾指针，buffer未折回 */
        if (rh > wr_tail) {	/* 读尾 > 写尾指针，读的数据太多 */
            HI_TRACE(HI_DBG_ERR, HI_ID_VALG, "VALG_CB_Read rd_head:%u wr_tail:%u rh:%u\n", rd_head, wr_tail, rh);
            return HI_FAILURE;
        }
    } else if (rh >= cb->buf_len) {	/* 读尾 > buffer长度，buffer已经折回 */
        rh -= cb->buf_len;
        if (rh > wr_tail) {
            HI_TRACE(HI_DBG_ERR, HI_ID_VALG, "VALG_CB_Read buf_len:%u wr_tail:%u rh:%u\n", cb->buf_len, wr_tail, rh);
            return HI_FAILURE;
        }
    }

    /* 根据输入的读出数据包长度，判断循环buffer是否折回; */
    if ((rd_head + rd_len) >= cb->buf_len) /* buffer折回，恰好到达buffer底部也要折回 */ {
        /* 计算两段数据包地址和长度 */
        rd_info->src[0] = (ADDR_UNIT *)cb->base + rd_head;
        rd_info->len[0] = cb->buf_len - rd_head;
        rd_info->src[1] = cb->base; /* 第二段数据包地址等于buffer首地址 */
        rd_info->len[1] = rd_len - rd_info->len[0];
        rd_head = rd_info->len[1]; /* 读头指针指向下一数据包起始位置 */
    } else {
        rd_info->src[0] = (ADDR_UNIT *)cb->base + rd_head;
        rd_info->len[0] = rd_len;
        rd_info->src[1] = (ADDR_UNIT *)rd_info->src[0] + rd_len;
        rd_info->len[1] = 0; /* 第二段数据包长度为0 */
        rd_head += rd_len; /* 读头指针指向下一数据包起始位置; */
    }

    /* 虚拟地址转换为物理地址 */
    if (cb->phy_base == NOT_SET_PHY_BASE) {
        /* if phy base addr equal 0, means not set, must return 0. */
        rd_info->phy_addr[0] = NOT_SET_PHY_BASE;
        rd_info->phy_addr[1] = NOT_SET_PHY_BASE;
    } else {
        /* 只要中间值不溢出，无符号数/有符号数加减不影响最后结果 */
        rd_info->phy_addr[0] = VIRT_TO_PHY((hi_u64)(hi_uintptr_t)rd_info->src[0],
                                              (hi_u64)(hi_uintptr_t)cb->base, cb->phy_base);
        rd_info->phy_addr[1] = VIRT_TO_PHY((hi_u64)(hi_uintptr_t)rd_info->src[1],
                                              (hi_u64)(hi_uintptr_t)cb->base, cb->phy_base);
    }

    (*(cb->read_head)) = rd_head;
    return HI_SUCCESS;
}

__inline static hi_bool valg_cb_is_empty(const valg_crcl_buf *cb)
{
    hi_u32 wr_head = (*(cb->write_head));
    hi_u32 rd_tail = (*(cb->read_tail));

    if (wr_head == rd_tail) {
        return HI_TRUE;
    } else {
        return HI_FALSE;
    }
}

__inline static hi_bool valg_cb_is_full(const valg_crcl_buf *cb)
{
    hi_u32 wh;
    hi_u32 rd_tail = (*(cb->read_tail));
    hi_u32 wr_head = (*(cb->write_head));

    wh = wr_head + cb->rsv_byte;
    if (wh >= cb->buf_len) {
        wh -= cb->buf_len;
    }

    if (wh == rd_tail) {
        return HI_TRUE;
    } else {
        return HI_FALSE;
    }
}

__inline static hi_s32 valg_cb_is_valid(const valg_crcl_buf *cb)
{
    /*  hi_u32 rh; */
    hi_u32 rd_head = (*(cb->read_head));
    hi_u32 rd_tail = (*(cb->read_tail));
    hi_u32 wr_head = (*(cb->write_head));
    hi_u32 wr_tail = (*(cb->write_tail));

    /* 判断输入的读取长度是否正确。
     * 要求:更新后的读头不超过写尾。
     * 注意区分两种情况:
     * 1、写尾指针未发生回转。需判断;
     * 2、写尾指针发生回转。
     *    1)读头没有回转，读头不会超过写尾，无需判断;
     *    2)读头也发生回转，需判断;
     */
    if (((rd_tail <= rd_head) && (rd_head <= wr_tail) && (wr_tail <= wr_head)) ||
        ((wr_head <= rd_tail) && (rd_tail <= rd_head) && (rd_head <= wr_tail)) ||
        ((wr_tail <= wr_head) && (wr_head <= rd_tail) && (rd_tail <= rd_head)) ||
        ((rd_head <= wr_tail) && (wr_tail <= wr_head) && (wr_head <= rd_tail))) {
            return HI_SUCCESS;
        } else {
            return HI_FAILURE;
        }
}

__inline static hi_u32 valg_cb_get_data_len(const valg_crcl_buf *cb)
{
    hi_u32 data_len = 0;
    hi_u32 rd_head = (*(cb->read_head));
    hi_u32 wr_tail = (*(cb->write_tail));

    if (wr_tail >= rd_head) /* 写尾在读头前面，说明buffer未折回 */ {
        /* 计算写尾折回时buffer中的数据长度 */
        data_len = wr_tail - rd_head;
    } else {
        /* 计算写尾折回时buffer中的数据长度 */
        data_len = cb->buf_len - (rd_head - wr_tail);
    }
    return data_len;
}

__inline static hi_u32 valg_cb_get_free_len(const valg_crcl_buf *cb)
{
    hi_u32 free_len;
    hi_u32 rd_tail = (*(cb->read_tail));
    hi_u32 wr_head = (*(cb->write_head));

    if (wr_head >= rd_tail) { /* 写头在读尾前面，说明buffer未折回 */
        /* 计算写头未折回时buffer中的数据长度; */
        free_len = cb->buf_len - (wr_head - rd_tail) - cb->rsv_byte;
    } else {
        /* 计算写头折回时buffer中的数据长度; */
        free_len = rd_tail - wr_head - cb->rsv_byte;
    }

    return free_len;
}

static __inline hi_u32 valg_cb_get_free_len_jpege(const valg_crcl_buf *cb, hi_u32* remain_left, hi_u32* remain_right)
{
    // 如果写头已经折回，直接计算rt-wh；否则，分别计算右边与左边的剩余空间
    // cb->rsv_byt为循环buf至少需要剩余的空间，不能写满
    // status: 0. 写头未折回，remain_right没有减去rsv_byte； 1. 写头未折回，remain_right减去rsv_byte； 2. 写头折回
    hi_u32 status;
    hi_u32 rd_tail = (*(cb->read_tail));
    hi_u32 wr_head = (*(cb->write_head));

    if (wr_head >= rd_tail) { /* 写头在读尾前面，说明buffer未折回 */
        /* 计算写头未折回时buffer中的数据长度; */
        if (rd_tail > cb->rsv_byte) {
            *remain_left = rd_tail - cb->rsv_byte;
            *remain_right = cb->buf_len - wr_head;
            status = 0;
        } else { // 这种情况rt就是在base上
            *remain_left = rd_tail;
            *remain_right = cb->buf_len - wr_head - cb->rsv_byte;
            status = 1;
        }
    } else {
        /* 计算写头折回时buffer中的数据长度; */
        *remain_left  = 0;
        *remain_right = rd_tail - wr_head - cb->rsv_byte;
        status = 2;
    }

    return status;
}

__inline static hi_u32 valg_cb_rd_region_len(const valg_crcl_buf *cb)
{
    hi_u32 rd_region_len;
    hi_u32 rd_head = (*(cb->read_head));
    hi_u32 rd_tail = (*(cb->read_tail));

    if (rd_head >= rd_tail) {
        rd_region_len = rd_head - rd_tail;
    } else {
        rd_region_len = cb->buf_len - (rd_tail - rd_head);
    }
    return rd_region_len;
}

__inline static hi_u32 valg_cb_wr_region_len(const valg_crcl_buf *cb)
{
    hi_u32 wr_region_len;
    hi_u32 wr_head = (*(cb->write_head));
    hi_u32 wr_tail = (*(cb->write_tail));

    if (wr_head >= wr_tail) {
        wr_region_len = wr_head - wr_tail;
    } else {
        wr_region_len = cb->buf_len - (wr_tail - wr_head);
    }
    return wr_region_len;
}

__inline static hi_void *valg_cb_get_rd_head(const valg_crcl_buf *cb)
{
    hi_void *rh;
    hi_u32 rd_head = (*(cb->read_head));

    rh = (ADDR_UNIT *)(cb->base) + rd_head;
    return rh;
}

__inline static hi_u64 valg_cb_get_rd_head_phy(const valg_crcl_buf *cb)
{
    hi_u32 rd_head = (*(cb->read_head));
    return cb->phy_base + rd_head;
}

__inline static hi_void *valg_cb_get_rd_tail(const valg_crcl_buf *cb)
{
    hi_void *rt;
    hi_u32 rd_tail = (*(cb->read_tail));

    rt = (ADDR_UNIT *)(cb->base) + rd_tail;
    return rt;
}

static __inline hi_void valg_cb_update_wh_by_software(const valg_crcl_buf *cb, hi_u32 len)
{
    // 编码前做过空闲buffer检测，所以不会有折回的情况
    hi_u32 wr_head = (*(cb->write_head));
    wr_head = wr_head + len;
    if (wr_head >= cb->buf_len) {
        *(cb->write_head) = wr_head - cb->buf_len;
    } else {
        *(cb->write_head) = wr_head;
    }
}

static __inline hi_void valg_cb_reset_wh_by_software(const valg_crcl_buf *cb)
{
    // JPEGE在右边剩余buf不足情况，右边补充空pack，然后将wh移动至buf开头
    hi_u32 wr_head = (*(cb->write_head));
    hi_u32 wr_tail = (*(cb->write_tail));
    wr_head = 0;
    wr_tail = wr_head;
    *(cb->write_head) = wr_head;
    *(cb->write_tail) = wr_tail;
}

__inline static hi_s32 valg_cb_update_rp(const valg_crcl_buf *cb, hi_u32 rd_len)
{
    hi_u32 rt;
    hi_u32 rd_head = (*(cb->read_head));
    hi_u32 rd_tail = (*(cb->read_tail));

    if ((rd_len & (WORD_ALIGN - 1U)) != 0U) {
        HI_TRACE(HI_DBG_ERR, HI_ID_VALG,
                 "%s err! len must be multiples of word(32-bit)!",
                 __FUNCTION__);
        return HI_FAILURE;
    }

    /* 输入长度正确性判断 */
    rt = rd_tail + rd_len;

    /* 判断输入的读取长度是否正确。
     * 要求:更新后的读头不超过读尾。
     * 注意区分两种情况:
     * 1、读头未发生回转。需判断;
     * 2、读头发生回转。
     *    1)读尾没有回转，读头不会超过读尾，无需判断;
     *    2)读尾也发生回转，需判断;
     */
    if (rd_head >= rd_tail) {
        if (rt > rd_head) {
            return HI_FAILURE;
        }
    } else if (rt >= cb->buf_len) {
        rt -= cb->buf_len;
        if (rt > rd_head) {
            return HI_FAILURE;
        }
    }

    (*(cb->read_tail)) = rt;
    return HI_SUCCESS;
}

__inline static hi_void valg_cb_update_wp(const valg_crcl_buf *cb)
{
    (*(cb->write_tail)) = (*(cb->write_head));
    return;
}

// 1.虚拟地址场景在wr_head超出buf_len时，软件将wr_head和wr_tail折回到buff头部
// 2.虚拟地址场景在wr_head离buff末尾不到640字节时，软件将wh，wt，rh折回到buff头部
//   防止I帧的vps，sps这些slice和I slice地址不连续，用户访问出现异常
__inline static hi_u32 valg_cb_retrace_wp(const valg_crcl_buf *cb) //lint !e618
{
    hi_u32 offset = 0;
    if (cb->wr_head >= cb->buf_len) {
        (*(cb->write_head)) = cb->wr_head - cb->buf_len;
        (*(cb->write_tail)) = (*(cb->write_head));
    }
    if ((cb->buf_len - cb->wr_head) <= MAX_HEADER_SLICE_LEN) {
        offset = cb->buf_len - cb->wr_head;
        (*(cb->write_head)) = 0;
        (*(cb->write_tail)) = 0;
        (*(cb->read_head)) = 0;
    }
    return offset;
}

__inline static hi_void valg_cb_back_wp(const valg_crcl_buf *cb)
{
    (*(cb->write_head)) = (*(cb->write_tail));
    return;
}

__inline static hi_void valg_cb_back_rp(const valg_crcl_buf *cb)
{
    (*(cb->read_head)) = (*(cb->read_tail));
    return;
}

__inline static hi_void valg_cb_reset(valg_crcl_buf *cb)
{
    (*(cb->read_head)) = 0;
    (*(cb->read_tail)) = 0;
    (*(cb->write_head)) = 0;
    (*(cb->write_tail)) = 0;

    return;
}

__inline static hi_void valg_cb_reset_master_ex(const valg_crcl_buf *cb)
{
    (*(cb->read_head)) = 0;
    return;
}

__inline static hi_s32 valg_cb_get_copy(const valg_crcl_buf *org_cb, valg_crcl_buf *copy_cb)
{
    if ((org_cb == HI_NULL) || (copy_cb == HI_NULL)) {
        return HI_FAILURE;
    }

    /* make a copy of original CB. */
    copy_cb->base = org_cb->base;
    copy_cb->phy_base = org_cb->phy_base;
    copy_cb->rd_head = org_cb->rd_head;
    copy_cb->rd_tail = org_cb->rd_tail;
    copy_cb->wr_head = org_cb->wr_head;
    copy_cb->wr_tail = org_cb->wr_tail;
    copy_cb->buf_len = org_cb->buf_len;
    copy_cb->rsv_byte = org_cb->rsv_byte;

    /* reinitiate the pointer to the copy CB. */
    copy_cb->read_head = &copy_cb->rd_head;
    copy_cb->read_tail = &copy_cb->rd_tail;
    copy_cb->write_head = &copy_cb->wr_head;
    copy_cb->write_tail = &copy_cb->wr_tail;

    return HI_SUCCESS;
}

__inline static hi_void *valg_cb_get_wr_head(const valg_crcl_buf *cb)
{
    hi_void *wh;
    hi_u32 wr_head = (*(cb->write_head));

    wh = (ADDR_UNIT *)(cb->base) + wr_head;
    return wh;
}

__inline static hi_u64 valg_cb_get_wr_head_phy(const valg_crcl_buf *cb)
{
    hi_u32 wr_head = (*(cb->write_head));
    return cb->phy_base + wr_head;
}

__inline static hi_void *valg_cb_get_wr_tail(const valg_crcl_buf *cb)
{
    hi_void *wt;
    hi_u32 wr_tail = (*(cb->write_tail));

    wt = (ADDR_UNIT *)(cb->base) + wr_tail;
    return wt;
}

__inline static hi_u64 valg_cb_get_wr_tail_phy(const valg_crcl_buf *cb)
{
    hi_u32 wr_tail = (*(cb->write_tail));
    return cb->phy_base + wr_tail;
}

__inline static hi_s32 valg_cb_update_wr_head(const valg_crcl_buf *cb, hi_u32 wr_len)
{
    hi_u32 wh; /* 更新后的写头 */
    hi_u32 wr_head = (*(cb->write_head));
    hi_u32 rd_tail = (*(cb->read_tail));

    if ((wr_len & (WORD_ALIGN - 1U)) != 0U) {
        HI_TRACE(HI_DBG_ERR, HI_ID_VALG,
                 "%s err! len(%d) must be 4B aligned!",
                 __FUNCTION__, wr_len);
        return HI_FAILURE;
    }

    /* 输入长度正确性判断 */
    /* 判断输入参数(长度)是否正确。
     * 要求:
     * 1, 若写头回绕，更新后的写头不超过读尾;
     * 2, 若写头没有回绕，则更新后的写头不允许超过buffer底部。
     * 3, 若写头触底，则回绕到buffer顶部。
     */
    wh = wr_head + wr_len;
    if (wr_head < rd_tail) {
        if (wh > rd_tail) {
            return HI_FAILURE;
        }
    } else if (wh > cb->buf_len) {
        /* update_wr_head操作，不允许写头回绕 */
        return HI_FAILURE;
    } else if (wh == cb->buf_len) {
        wh = 0;
    }

    (*(cb->write_head)) = wh;
    return HI_SUCCESS;
}

__inline static hi_void valg_cb_flush_read(valg_crcl_buf *cb)
{
    osal_flush_dcache_area(&cb->rd_head, 0, sizeof(hi_u32));
    osal_flush_dcache_area(&cb->rd_tail, 0, sizeof(hi_u32));
}

__inline static hi_void valg_cb_invalid_read(valg_crcl_buf *cb)
{
    osal_invalid_dcache_area(&cb->rd_head, 0, sizeof(hi_u32));
    osal_invalid_dcache_area(&cb->rd_tail, 0, sizeof(hi_u32));
}

__inline static hi_void valg_cb_flush_write(valg_crcl_buf *cb)
{
    osal_flush_dcache_area(&cb->wr_tail, 0, sizeof(hi_u32));
    osal_flush_dcache_area(&cb->wr_head, 0, sizeof(hi_u32));
}

__inline static hi_void valg_cb_invalid_write(valg_crcl_buf *cb)
{
    osal_invalid_dcache_area(&cb->wr_tail, 0, sizeof(hi_u32));
    osal_invalid_dcache_area(&cb->wr_head, 0, sizeof(hi_u32));
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

