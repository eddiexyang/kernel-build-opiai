/*
* Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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
* Create: 2022-8-15
*/
#include <linux/mutex.h>
#include <linux/kref.h>
#include <linux/mm.h>
#include <linux/slab.h>

#include "trs_chip_def_comm.h"
#include "trs_host_msg.h"

static DEFINE_MUTEX(host_msg_mutex);

struct trs_host_msg {
    u32 devid;
    void *chan;
    struct kref ref;
};

static struct trs_host_msg *g_host_msg[TRS_DEV_MAX_NUM];

static struct trs_host_msg *trs_host_msg_create(u32 devid, struct devdrv_non_trans_msg_chan_info *chan_info)
{
    struct trs_host_msg *host_msg = kmalloc(sizeof(struct trs_host_msg), GFP_KERNEL);
    void *chan = NULL;
    int ret;

    if (host_msg == NULL) {
        return NULL;
    }

    chan = devdrv_pcimsg_alloc_non_trans_queue(devid, chan_info);
    if (chan == NULL) {
        kfree(host_msg);
        trs_err("Msg chan alloc fail. (devid=%u)\n", devid);
        return NULL;
    }
    ret = devdrv_set_msg_chan_priv(chan, (void *)(uintptr_t)devid);
    if (ret != 0) {
        (void)devdrv_pcimsg_free_non_trans_queue(chan);
        kfree(host_msg);
        trs_err("Set msg chan priv fail. (devid=%u; ret=%d)\n", devid, ret);
        return NULL;
    }
    host_msg->devid = devid;
    host_msg->chan = chan;
    kref_init(&host_msg->ref);

    return host_msg;
}

static void trs_host_msg_destroy(struct trs_host_msg *host_msg)
{
    if (host_msg->chan != NULL) {
        (void)devdrv_pcimsg_free_non_trans_queue(host_msg->chan);
        host_msg->chan = NULL;
    }
    kfree(host_msg);
}

static void trs_host_msg_release(struct kref *kref)
{
    struct trs_host_msg *host_msg = container_of(kref, struct trs_host_msg, ref);

    trs_host_msg_destroy(host_msg);
}

static int trs_host_msg_add(struct trs_host_msg *host_msg)
{
    mutex_lock(&host_msg_mutex);
    if (g_host_msg[host_msg->devid] != NULL) {
        mutex_unlock(&host_msg_mutex);
        return -ENODEV;
    }
    g_host_msg[host_msg->devid] = host_msg;
    mutex_unlock(&host_msg_mutex);

    return 0;
}

static void trs_host_msg_del(u32 devid)
{
    struct trs_host_msg *host_msg = NULL;

    mutex_lock(&host_msg_mutex);
    host_msg = g_host_msg[devid];
    g_host_msg[devid] = NULL;
    mutex_unlock(&host_msg_mutex);

    if (host_msg != NULL) {
        kref_put(&host_msg->ref, trs_host_msg_release);
    }
}

static struct trs_host_msg *trs_host_msg_get(u32 devid)
{
    struct trs_host_msg *host_msg = NULL;

    if (devid >= TRS_DEV_MAX_NUM) {
        trs_err("Invalid devid. (devid=%u)\n", devid);
        return NULL;
    }
    mutex_lock(&host_msg_mutex);
    host_msg = g_host_msg[devid];
    if (host_msg != NULL) {
        kref_get(&host_msg->ref);
    }
    mutex_unlock(&host_msg_mutex);

    return host_msg;
}

static void trs_host_msg_put(struct trs_host_msg *host_msg)
{
    kref_put(&host_msg->ref, trs_host_msg_release);
}

int trs_host_msg_chan_recv_check(u32 devid, struct trs_msg_data *data, u32 in_data_len,
    u32 out_data_len, u32 *real_out_len)
{
    if (devid >= TRS_DEV_MAX_NUM) {
        trs_err("Invalid devid. (devid=%u)\n", devid);
        return -EINVAL;
    }

    if ((data == NULL) || (in_data_len != (u32)sizeof(struct trs_msg_data))) {
        return -EINVAL;
    }

    if (data->header.valid != TRS_MSG_SEND_MAGIC) {
        trs_err("Invalid magic. (magic=0x%x)\n", data->header.valid);
        return -EINVAL;
    }

    if (data->header.tsid >= TRS_TS_MAX_NUM) {
        trs_err("Invalid tsid. (tsid=%u)\n", data->header.tsid);
        return -EINVAL;
    }

    if (real_out_len == NULL) {
        return -EINVAL;
    }

    if (data->header.cmdtype >= TRS_MSG_MAX) {
        trs_err("Invalid cmd. (cmd=%u)\n", data->header.cmdtype);
        return -EINVAL;
    }

    return 0;
}

int trs_host_msg_send(u32 devid, void *msg, size_t size)
{
    struct trs_msg_data *tmp_msg = (struct trs_msg_data *)msg;
    struct trs_host_msg *host_msg = NULL;
    u32 out_len;
    int ret;

    if ((msg == NULL) || (size != sizeof(struct trs_msg_data))) {
        return -EINVAL;
    }

    host_msg = trs_host_msg_get(devid);
    if (host_msg == NULL) {
        return -ENODEV;
    }
    ret = devdrv_sync_msg_send(host_msg->chan, tmp_msg, sizeof(struct trs_msg_data),
        sizeof(struct trs_msg_data), &out_len);
    trs_host_msg_put(host_msg);

    return (ret == 0) ? (int)tmp_msg->header.result : ret;
}
EXPORT_SYMBOL(trs_host_msg_send);

int trs_host_msg_init(u32 devid, struct devdrv_non_trans_msg_chan_info *chan_info)
{
    struct trs_host_msg *host_msg = trs_host_msg_create(devid, chan_info);
    int ret;

    if (host_msg == NULL) {
        return -ENOMEM;
    }
    ret = trs_host_msg_add(host_msg);
    if (ret != 0) {
        trs_host_msg_destroy(host_msg);
        trs_err("Add host msg fail. (devid=%u; ret=%d)\n", devid, ret);
    }
    return ret;
}

void trs_host_msg_uninit(u32 devid)
{
    trs_host_msg_del(devid);
}

