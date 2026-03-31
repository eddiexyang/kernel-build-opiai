/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * Description:
 * Author: huawei
 * Create: 2020-4-1
 */

#ifndef _PNGD_BUF_H__
#define _PNGD_BUF_H__

#include "hi_type.h"
#include "hi_osal.h"

typedef struct {
    void *value; /* 指向实际空间的指针 */
    struct osal_list_head list;
} pngd_node;

typedef struct {
    hi_u32 count;
    hi_u32 item_size;
    hi_u32 free_count;
    hi_u32 busy_count;
    struct osal_list_head free_list; /* free链表头 */
    struct osal_list_head busy_list; /* busy链表头 */
} pngd_buf;

void pngd_buf_init(pngd_buf *buf, hi_u32 item_size);
void *pngd_buf_alloc_free_node(pngd_buf *buf);

void *pngd_buf_get_free(pngd_buf *buf);
void *pngd_buf_get_busy(pngd_buf *buf);

void pngd_buf_put_free(pngd_buf *buf, void *value);
void pngd_buf_put_busy(pngd_buf *buf, void *value);

hi_bool pngd_buf_is_busy_empty(pngd_buf *buf);
hi_bool pngd_buf_is_free_empty(pngd_buf *buf);

hi_u32 pngd_buf_busy_count(pngd_buf *buf);
hi_u32 pngd_buf_free_count(pngd_buf *buf);

void pngd_buf_release(pngd_buf *buf);

#endif // end #ifndef _PNGD_BUF_H__
