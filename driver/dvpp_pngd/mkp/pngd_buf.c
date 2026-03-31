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

#include "osal_list.h"
#include "pngd_buf.h"
#include "hi_common.h"
#include "hi_debug.h"
#include "pngd.h"
#include "securec.h"

void pngd_buf_init(pngd_buf *buf, hi_u32 item_size)
{
    OSAL_INIT_LIST_HEAD(&buf->free_list);
    OSAL_INIT_LIST_HEAD(&buf->busy_list);

    buf->item_size = item_size;
    buf->count = 0;
    buf->free_count = 0;
    buf->busy_count = 0;
}

void *pngd_buf_alloc_free_node(pngd_buf *buf)
{
    int ret;
    int size;
    pngd_node *node = HI_NULL;

    size = sizeof(pngd_node) + buf->item_size;
    node = (pngd_node *)osal_kmalloc(size, osal_gfp_kernel);
    if (node == NULL) {
        PNGD_ALERT_TRACE("malloc pngd_node failed!\n");
        return NULL;
    }
    ret = memset_s(node, size, 0, size);
    CHECK_DO_SOMETHING(ret != 0, PNGD_ERR_TRACE("memset_s pngd_node failed! ret=%d\n", ret));

    /* 把节点加入到free链表 */
    node->value = (void *)((char *)node + sizeof(pngd_node));

    osal_list_add_tail(&node->list, &buf->free_list);

    buf->count++;
    buf->free_count++;

    return node->value;
}

void pngd_buf_release(pngd_buf *buf)
{
    void *value = HI_NULL;

    while (!pngd_buf_is_free_empty(buf)) {
        value = pngd_buf_get_free(buf);
        osal_kfree((pngd_node *)((hi_u8 *)(value) - sizeof(pngd_node)));
    }

    while (!pngd_buf_is_busy_empty(buf)) {
        value = pngd_buf_get_busy(buf);
        osal_kfree((pngd_node *)((hi_u8 *)(value) - sizeof(pngd_node)));
    }

    buf->count = 0;
    buf->free_count = 0;
    buf->busy_count = 0;
}

void *pngd_buf_get_free(pngd_buf *buf)
{
    struct osal_list_head *plist = HI_NULL;
    pngd_node *node = NULL;

    if (osal_list_empty(&buf->free_list)) {
        return NULL;
    }

    plist = buf->free_list.next;
    osal_list_del(plist);

    node = osal_list_entry(plist, pngd_node, list);
    buf->free_count--;

    return node->value;
}

void *pngd_buf_get_busy(pngd_buf *buf)
{
    struct osal_list_head *plist = HI_NULL;
    pngd_node *node = NULL;

    if (osal_list_empty(&buf->busy_list)) {
        return NULL;
    }
    plist = buf->busy_list.next;
    osal_list_del(plist);

    node = osal_list_entry(plist, pngd_node, list);
    buf->busy_count--;

    return node->value;
}

void pngd_buf_put_free(pngd_buf *buf, void *value)
{
    pngd_node *node = HI_NULL;
    if ((buf == NULL) || (value == NULL)) {
        return;
    }

    node = (pngd_node *)((hi_u8 *)(value) - sizeof(pngd_node));

    osal_list_add_tail(&node->list, &buf->free_list);
    buf->free_count++;
}

void pngd_buf_put_busy(pngd_buf *buf, void *value)
{
    pngd_node *node = HI_NULL;

    if ((buf == NULL) || (value == NULL)) {
        return;
    }

    node = (pngd_node *)((hi_u8 *)(value) - sizeof(pngd_node));
    osal_list_add_tail(&node->list, &buf->busy_list);
    buf->busy_count++;
}

hi_bool pngd_buf_is_busy_empty(pngd_buf *buf)
{
    return osal_list_empty(&buf->busy_list);
}

hi_bool pngd_buf_is_free_empty(pngd_buf *buf)
{
    return osal_list_empty(&buf->free_list);
}

hi_u32 pngd_buf_busy_count(pngd_buf *buf)
{
    return buf->busy_count;
}
hi_u32 pngd_buf_free_count(pngd_buf *buf)
{
    return buf->free_count;
}

