/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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
* Create: 2019-10-15
*/

#include <linux/errno.h>
#include <linux/time.h>
#include <linux/slab.h>

#include "devdrv_manager.h"
#include "devdrv_manager_container.h"

#include "dms_define.h"
#include "dms_event.h"
#include "dms_event_distribute.h"
#include "dms_event_converge.h"
#include "dms_event_host.h"
#include "devdrv_black_box.h"
#include "hvdevmng_init.h"

static struct mutex g_hvdms_subscribe_status_mutex;
static u32 g_hvdms_subscribe_status_bitmap[DEVICE_NUM_MAX] = {0};

void devdrv_device_black_box_add_exception(u32 devid, u32 code)
{
}
int dms_event_box_add_exception(u32 devid, u32 code, struct timespec stamp)
{
    return devdrv_host_black_box_add_exception(devid, code, stamp, NULL);
}

int dms_event_subscribe_from_host(u32 devid, void *msg, u32 in_len, u32 *ack_len)
{
    return DRV_ERROR_NOT_SUPPORT;
}

int dms_event_distribute_to_bar(u32 phyid)
{
    /* not need to fresh event code to bar in host */
    return DRV_ERROR_NONE;
}

int dms_distribute_all_devices_event_to_bar(void)
{
    /* no need to fresh event code to bar in host */
    return DRV_ERROR_NONE;
}

int dms_event_subscribe_from_device(u32 phyid)
{
    struct devdrv_manager_msg_info dev_manager_msg_info = {{0}};
    void *no_trans_chan = NULL;
    u32 out_len;
    int ret;

    dev_manager_msg_info.header.msg_id = DEVDRV_MANAGER_CHAN_H2D_DMS_EVENT_SUBSCRIBE;
    dev_manager_msg_info.header.valid = DEVDRV_MANAGER_MSG_H2D_MAGIC;
    dev_manager_msg_info.header.result = DEVDRV_MANAGER_MSG_INVALID_RESULT;

    no_trans_chan = devdrv_manager_get_no_trans_chan(phyid);
    if (no_trans_chan == NULL) {
        dms_err("Get no trans chan failed. (phy_id=%u)\n", phyid);
        return DRV_ERROR_NOT_EXIST;
    }

    dev_manager_msg_info.header.dev_id = phyid;
    ret = devdrv_sync_msg_send(no_trans_chan, &dev_manager_msg_info, sizeof(dev_manager_msg_info),
                               sizeof(dev_manager_msg_info), &out_len);
    if (ret || (dev_manager_msg_info.header.result == DEVDRV_MANAGER_MSG_INVALID_RESULT) ||
        (dev_manager_msg_info.header.valid != DEVDRV_MANAGER_MSG_H2D_MAGIC)) {
        dms_err("Send message to device failed. (dev_id=%u; result=%u; valid=0x%x; ret=%d)\n",
                phyid, dev_manager_msg_info.header.result, dev_manager_msg_info.header.valid, ret);
        return DRV_ERROR_INNER_ERR;
    }

    dms_info("Subscribe from device success. (phy_id=%u)\n", phyid);
    return DRV_ERROR_NONE;
}

int dms_event_clean_to_device(u32 phyid)
{
    struct devdrv_manager_msg_info dev_manager_msg_info = {{0}};
    void *no_trans_chan = NULL;
    u32 out_len;
    int ret;

    dev_manager_msg_info.header.msg_id = DEVDRV_MANAGER_CHAN_H2D_DMS_EVENT_CLEAN;
    dev_manager_msg_info.header.valid = DEVDRV_MANAGER_MSG_H2D_MAGIC;
    dev_manager_msg_info.header.result = DEVDRV_MANAGER_MSG_INVALID_RESULT;

    no_trans_chan = devdrv_manager_get_no_trans_chan(phyid);
    if (no_trans_chan == NULL) {
        dms_err("Get no trans failed. (phy_id=%u)\n", phyid);
        return DRV_ERROR_NOT_EXIST;
    }

    dev_manager_msg_info.header.dev_id = phyid;
    ret = devdrv_sync_msg_send(no_trans_chan, &dev_manager_msg_info, sizeof(dev_manager_msg_info),
                               sizeof(dev_manager_msg_info), &out_len);
    if (ret || (dev_manager_msg_info.header.result == DEVDRV_MANAGER_MSG_INVALID_RESULT) ||
        (dev_manager_msg_info.header.valid != DEVDRV_MANAGER_MSG_H2D_MAGIC)) {
        dms_err("Send message to device failed. (phy_id=%u; result=%u; valid=0x%x; ret=%d)\n",
                phyid, dev_manager_msg_info.header.result, dev_manager_msg_info.header.valid, ret);
        return DRV_ERROR_INNER_ERR;
    }
    dms_event("Clean event to device success. (phy_id=%u)\n", phyid);

    return DRV_ERROR_NONE;
}

STATIC int dms_event_mask_to_device(u32 phyid, u32 event_code, u8 mask)
{
    struct devdrv_manager_msg_info dev_manager_msg_info = {{0}};
    void *no_trans_chan = NULL;
    u32 *data_load = NULL;
    u32 out_len;
    int ret;

    dev_manager_msg_info.header.msg_id = DEVDRV_MANAGER_CHAN_H2D_DMS_EVENT_MASK;
    dev_manager_msg_info.header.valid = DEVDRV_MANAGER_MSG_H2D_MAGIC;
    dev_manager_msg_info.header.result = DEVDRV_MANAGER_MSG_INVALID_RESULT;

    no_trans_chan = devdrv_manager_get_no_trans_chan(phyid);
    if (no_trans_chan == NULL) {
        dms_err("Get no trans failed. (phy_id=%u)\n", phyid);
        return DRV_ERROR_NOT_EXIST;
    }

    dev_manager_msg_info.header.dev_id = phyid;
    data_load = (u32 *)dev_manager_msg_info.payload;
    *data_load++ = event_code;
    *data_load = mask;
    ret = devdrv_sync_msg_send(no_trans_chan, &dev_manager_msg_info, sizeof(dev_manager_msg_info),
                               sizeof(dev_manager_msg_info), &out_len);
    if (ret || (dev_manager_msg_info.header.result == DEVDRV_MANAGER_MSG_INVALID_RESULT) ||
        (dev_manager_msg_info.header.valid != DEVDRV_MANAGER_MSG_H2D_MAGIC)) {
        dms_err("Send message to device failed. (phy_id=%u; result=%u; valid=0x%x; ret=%d)\n",
                phyid, dev_manager_msg_info.header.result, dev_manager_msg_info.header.valid, ret);
        return DRV_ERROR_INNER_ERR;
    }

    dms_event("Mask event to device success. (phyid=%u; event_code=0x%x; mask=%u)\n", phyid, event_code, mask);
    return DRV_ERROR_NONE;
}

int dms_event_mask_event_code(u32 phyid, u32 event_code, u8 mask)
{
    int ret;

    ret = dms_event_mask_by_phyid(phyid, event_code, mask);
    if (ret) {
        dms_err("Mask event code in host failed. (phyid=%u; event_code=0x%x; mask=%u; ret=%d)\n",
                phyid, event_code, mask, ret);
        return ret;
    }

    ret = dms_event_mask_to_device(phyid, event_code, mask);
    if (ret) {
        dms_err("Mask event code to device failed. (phyid=%u; event_code=0x%x; mask=%u; ret=%d)\n",
                phyid, event_code, mask, ret);
        return ret;
    }

    return DRV_ERROR_NONE;
}

int dms_event_get_exception_from_device(void *msg, u32 *ack_len)
{
    struct devdrv_manager_msg_info *dev_manager_msg_info = NULL;
    DMS_EVENT_NODE_STRU *exception_buf = NULL;
    int ret;

    dev_manager_msg_info = (struct devdrv_manager_msg_info *)msg;
    if ((dev_manager_msg_info->header.dev_id >= DEVDRV_MAX_DAVINCI_NUM) ||
        (dev_manager_msg_info->header.valid != DEVDRV_MANAGER_MSG_D2H_MAGIC)) {
        dms_err("Invalid message from device. (dev_id=%u; valid=%u)\n",
                dev_manager_msg_info->header.dev_id, dev_manager_msg_info->header.valid);
        return DRV_ERROR_INVALID_VALUE;
    }
    *ack_len = sizeof(*dev_manager_msg_info);

    exception_buf = (DMS_EVENT_NODE_STRU *)dev_manager_msg_info->payload;

    exception_buf->event.deviceid = (unsigned short)dev_manager_msg_info->header.dev_id;
    exception_buf->event.notify_serial_num = dms_event_get_notify_serial_num();

    ret = dms_event_distribute_handle(exception_buf, DMS_DISTRIBUTE_PRIORITY0);
    if (ret) {
        dms_err("Distribute handle failed. (dev_id=%u; ret=%d)\n",
                exception_buf->event.deviceid, ret);
        return ret;
    }
    dms_debug("Get event from device success. (phy_id=%u; event_id=0x%x)\n",
              dev_manager_msg_info->header.dev_id, exception_buf->event.event_id);

    dev_manager_msg_info->header.result = (u16)DEVDRV_MANAGER_MSG_VALID;
    return DRV_ERROR_NONE;
}

int dms_get_event_code_from_bar(u32 devid, u32 *health_code, u32 health_len,
    struct shm_event_code *event_code, u32 event_len)
{
    int ret;

    ret = devmng_dms_get_event_code(devid, health_code, health_len, event_code, event_len);
    if (ret) {
        dms_err("Get event code from bar zone. (ret=%d)\n", ret);
        return DRV_ERROR_INNER_ERR;
    }

    return DRV_ERROR_NONE;
}

int dms_get_health_code_from_bar(u32 devid, u32 *health_code, u32 health_len)
{
    if (devmng_dms_get_health_code(devid, health_code, health_len)) {
        dms_err("Get event code from bar zone.\n");
        return DRV_ERROR_INNER_ERR;
    }

    return DRV_ERROR_NONE;
}

void dms_event_host_init(void)
{
    u32 i;

    mutex_init(&g_hvdms_subscribe_status_mutex);
    mutex_lock(&g_hvdms_subscribe_status_mutex);
    for (i = 0; i < DEVICE_NUM_MAX; i++) {
        g_hvdms_subscribe_status_bitmap[i] = 0;
    }
    mutex_unlock(&g_hvdms_subscribe_status_mutex);
}

void dms_event_host_uninit(void)
{
    u32 i;

    mutex_lock(&g_hvdms_subscribe_status_mutex);
    for (i = 0; i < DEVICE_NUM_MAX; i++) {
        g_hvdms_subscribe_status_bitmap[i] = 0;
    }
    mutex_unlock(&g_hvdms_subscribe_status_mutex);
    mutex_destroy(&g_hvdms_subscribe_status_mutex);
}

