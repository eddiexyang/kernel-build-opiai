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
* Create: 2022-7-15
*/
#include <linux/init.h>
#include <linux/kref.h>
#include <linux/slab.h>
#include <linux/pci.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/mm.h>

#include <securec.h>

#include "dbl/uda.h"
#include "soc_res.h"
#include "trs_chip_def_comm.h"
#include "trs_agent.h"
#include "trs_pm_agent.h"
#include "trs_pub_def.h"
#include "devdrv_interface.h"
#include "trs_agent_ops.h"
#include "trs_cdqm.h"
#include "trs_cache_id_recycle.h"
#include "trs_abnormal_info.h"
#include "trs_pm_adapt.h"
#include "trs_d2h_msg_ops.h"
static struct mutex trs_agent_mutex;

/* Trs agent chan type */
enum {
    TRS_AGENT_CHAN_NON_TRANS,
    TRS_AGENT_CHAN_MAX
};

struct trs_agent_chan {
    void *pci_chan;
};

struct trs_agent_dev {
    u32 devid;

    struct kref ref;
    struct trs_agent_chan chan[TRS_AGENT_CHAN_MAX];
};

static struct trs_agent_dev *g_trs_dev[TRS_DEVICE_MAX_DEV];

static struct trs_agent_dev *trs_agent_dev_create(u32 devid)
{
    struct trs_agent_dev *trs_dev = NULL;
    int chan_type;

    trs_dev = kzalloc(sizeof(struct trs_agent_dev), GFP_KERNEL);
    if (trs_dev == NULL) {
        trs_err("Alloc trs_dev fail.\n");
        return NULL;
    }
    trs_dev->devid = devid;
    kref_init(&trs_dev->ref);

    for (chan_type = 0; chan_type < TRS_AGENT_CHAN_MAX; chan_type++) {
        struct trs_agent_chan *chan = &trs_dev->chan[chan_type];
        chan->pci_chan = NULL;
    }

    return trs_dev;
}

static void trs_agent_dev_destroy(struct trs_agent_dev *trs_dev)
{
    if (trs_dev != NULL) {
        kfree(trs_dev);
    }
}

static int trs_agent_dev_add(struct trs_agent_dev *trs_dev)
{
    mutex_lock(&trs_agent_mutex);
    if (g_trs_dev[trs_dev->devid] != NULL) {
        mutex_unlock(&trs_agent_mutex);
        trs_err("Add agent fail. (devid=%u)\n", trs_dev->devid);
        return -ENODEV;
    }
    g_trs_dev[trs_dev->devid] = trs_dev;
    mutex_unlock(&trs_agent_mutex);

    return 0;
}

static void trs_agent_dev_release(struct kref *kref)
{
    struct trs_agent_dev *trs_dev = container_of(kref, struct trs_agent_dev, ref);

    trs_info("trs agent uninit. (devid=%u)\n", trs_dev->devid);
    trs_agent_dev_destroy(trs_dev);
}

static void trs_agent_dev_del(u32 devid)
{
    struct trs_agent_dev *trs_dev = NULL;

    mutex_lock(&trs_agent_mutex);
    trs_dev = g_trs_dev[devid];
    g_trs_dev[devid] = NULL;
    if (trs_dev != NULL) {
        kref_put(&trs_dev->ref, trs_agent_dev_release);
    }
    mutex_unlock(&trs_agent_mutex);
}

static struct trs_agent_chan *trs_agent_chan_get(u32 devid, int type)
{
    struct trs_agent_dev *trs_dev = NULL;
    struct trs_agent_chan *chan = NULL;

    if (devid >= TRS_DEVICE_MAX_DEV) {
        trs_err("Invalid devid. (devid=%u)\n", devid);
        return NULL;
    }

    mutex_lock(&trs_agent_mutex);
    trs_dev = g_trs_dev[devid];
    if (trs_dev != NULL) {
        kref_get(&trs_dev->ref);
        chan = &trs_dev->chan[type];
    }
    mutex_unlock(&trs_agent_mutex);

    return chan;
}

static void trs_agent_chan_put(struct trs_agent_chan *chan, int type)
{
    struct trs_agent_dev *trs_dev = container_of(chan, struct trs_agent_dev, chan[type]);

    mutex_lock(&trs_agent_mutex);
    kref_put(&trs_dev->ref, trs_agent_dev_release);
    mutex_unlock(&trs_agent_mutex);
}

int trs_agent_non_trans_msg_send(u32 devid, void *data, size_t size)
{
    const int chan_type = TRS_AGENT_CHAN_NON_TRANS;
    struct trs_agent_chan *chan = NULL;
    size_t real_out_len;
    int ret = -ENODEV;

    chan = trs_agent_chan_get(devid, chan_type);
    if (chan == NULL) {
        trs_err("Trs agent get chan fail. (devid=%u)\n", devid);
        return -ENODEV;
    }
    ret = agentdrv_sync_msg_send(chan->pci_chan, data, (u32)size, (u32)size, (u32 *)&real_out_len);
    trs_agent_chan_put(chan, chan_type);

    return ret;
}

#ifdef CFG_FEATURE_CDQM
int trs_agent_cdqm_init(u32 devid)
{
    struct cdqm_adapt_ops ops;
    struct trs_id_inst inst;
    u32 ts_num, tsid;
    int ret;

    if (devid >= 32) { /* temporary 32 */
        return 0;
    }

    ret = soc_resmng_subsys_get_num(devid, TS_SUBSYS, &ts_num);
    if ((ret != 0) || (ts_num == 0) || (ts_num > TRS_TS_MAX_NUM)) {
        trs_err("Get failed. (ret=%d; ts_num=%u)", ret, ts_num);
        return -EFAULT;
    }

    for (tsid = 0; tsid < ts_num; tsid++) {
        trs_id_inst_pack(&inst, devid, tsid);
        ops.send_sync_msg = trs_agent_non_trans_msg_send;
        (void)cdqm_ts_inst_register(&inst, &ops);
    }
    return 0;
}
#endif

static int trs_agent_init(u32 devid)
{
#ifdef CFG_FEATURE_CDQM
    int ret;
    ret = trs_agent_cdqm_init(devid);
    if (ret != 0) {
        trs_err("Cdqm init fail. (devid=%d; ret=%d)\n", devid, ret);
        return ret;
    }
#endif

    return 0;
}

static void trs_agent_uninit(u32 devid)
{
}

static int trs_agent_inst_init(u32 devid)
{
    struct trs_agent_dev *trs_dev = NULL;
    struct trs_id_inst id_inst;
    int ret;

    trs_dev = trs_agent_dev_create(devid);
    if (trs_dev == NULL) {
        return -ENOMEM;
    }

    ret = trs_agent_dev_add(trs_dev);
    if (ret != 0) {
        trs_agent_dev_destroy(trs_dev);
        return ret;
    }

    ret = trs_agent_init(trs_dev->devid);
    if (ret != 0) {
        trs_agent_dev_del(trs_dev->devid);
        trs_agent_dev_destroy(trs_dev);
        trs_err("Agent init fail. (ret=%d)\n", ret);
        return ret;
    }

    ret = trs_agent_ops_dev_instance(trs_dev->devid, 0);
    if (ret != 0) {
        trs_agent_dev_del(trs_dev->devid);
        trs_agent_dev_destroy(trs_dev);
        return ret;
    }

    trs_id_inst_pack(&id_inst, trs_dev->devid, 0);
    ret = trs_id_recycle_create(&id_inst);
    if (ret != 0) {
        trs_agent_ops_dev_uninstance(trs_dev->devid, 0);
        trs_agent_dev_del(trs_dev->devid);
        trs_agent_dev_destroy(trs_dev);
        trs_err("Recyle create failed. (devid=%u; tsid=%u)\n", trs_dev->devid, 0);
        return ret;
    }

    trs_debug("Agent init. (devid=%u)\n", trs_dev->devid);
    return 0;
}

static void trs_agent_inst_uninit(u32 devid)
{
    struct trs_id_inst id_inst;

    trs_id_inst_pack(&id_inst, devid, 0);
    trs_id_recycle_destroy(&id_inst);
    trs_agent_uninit(devid);
    trs_agent_ops_dev_uninstance(devid, 0);
    trs_agent_dev_del(devid);
}

#define TRS_DEVICE_AGENT_NOTIFIER "trs_agent"
static int trs_agent_notifier_func(u32 udevid, enum uda_notified_action action)
{
    struct device *dev = NULL;
    int ret = 0;

    if (udevid >= TRS_DEVICE_MAX_DEV) {
        trs_err("Invalid para. (udevid=%u)\n", udevid);
        return -EINVAL;
    }

    dev = uda_get_agent_device(udevid);
    if (dev != NULL) { /* obp virtual dev is null, not need to init virtual dev */
        if (action == UDA_INIT) {
            ret = trs_agent_inst_init(udevid);
        } else if (action == UDA_UNINIT) {
            trs_agent_inst_uninit(udevid);
        } else {
            /* do nothing */
        }
    }

    trs_info("notifier action. (udevid=%u; action=%d; ret=%d)\n", udevid, action, ret);

    return ret;
}

static u32 trs_agent_chan_to_devid(void *msg_chan)
{
    u32 devid;

    if (msg_chan == NULL) {
        trs_err("Trs agent msg chan is NULL.\n");
        return UINT_MAX;
    }
    if ((devid = (u32)agentdrv_get_msg_chan_devid(msg_chan)) >= TRS_DEVICE_MAX_DEV) {
        trs_err("Trs agent msg chan to devid fail. (devid=%u)\n", devid);
        return UINT_MAX;
    }

    return devid;
}

static int trs_agent_init_non_trans_msg_chan(void *msg_chan)
{
    u32 devid = trs_agent_chan_to_devid(msg_chan);
    struct trs_agent_chan *chan = NULL;

    chan = trs_agent_chan_get(devid, TRS_AGENT_CHAN_NON_TRANS);
    if (chan == NULL) {
        trs_err("Get chan fail. (devid=%u)\n", devid);
        return -ENODEV;
    }
    chan->pci_chan = msg_chan;
    trs_agent_chan_put(chan, TRS_AGENT_CHAN_NON_TRANS);

    trs_debug("Init non trans msg chan. (devid=%u)\n", devid);
    return 0;
}

static void trs_agent_uninit_non_trans_msg_chan(void *msg_chan)
{
    u32 devid = trs_agent_chan_to_devid(msg_chan);
    struct trs_agent_chan *chan = NULL;

    chan = trs_agent_chan_get(devid, TRS_AGENT_CHAN_NON_TRANS);
    if (chan == NULL) {
        trs_err("Get chan fail. (devid=%u)\n", devid);
        return;
    }
    chan->pci_chan = NULL;
    trs_agent_chan_put(chan, TRS_AGENT_CHAN_NON_TRANS);

    trs_info("Uninit non trans msg chan. (devid=%u)\n", devid);
}

int trs_agent_non_trans_msg_process(void *msg_chan, void *data, u32 in_data_len,
    u32 out_data_len, u32 *real_out_len)
{
    struct trs_agent_chan *chan = NULL;
    u32 devid;
    int ret;

    if ((data == NULL) || (real_out_len == NULL)) {
        return -EINVAL;
    }

    devid = trs_agent_chan_to_devid(msg_chan);
    chan = trs_agent_chan_get(devid, TRS_AGENT_CHAN_NON_TRANS);
    if (chan == NULL) {
        return -ENODEV;
    }
    ret = trs_agent_rcv_handle(devid, data, in_data_len);
    trs_agent_chan_put(chan, TRS_AGENT_CHAN_NON_TRANS);

    *real_out_len = 0;
    if (ret == 0) {
        *real_out_len = out_data_len;
    }

    return ret;
}

static struct agentdrv_non_trans_msg_client trs_agent_non_trans = {
    .type = agentdrv_msg_client_tsdrv,
    .flag = 0,
    .init_non_trans_msg_chan = trs_agent_init_non_trans_msg_chan,
    .uninit_non_trans_msg_chan = trs_agent_uninit_non_trans_msg_chan,
    .non_trans_msg_process = trs_agent_non_trans_msg_process,
};

int __init init_trs_agent(void)
{
    struct trs_adapt_notice_ops *ops = trs_agent_get_adapt_notice_ops();
    struct uda_dev_type type;
    int ret;

    trs_info("Trs agent init start.\n");
    trs_agent_ops_init();
    trs_pm_agent_init();

    uda_davinci_local_real_agent_type_pack(&type);
    ret = uda_real_virtual_notifier_register(TRS_DEVICE_AGENT_NOTIFIER, &type, UDA_PRI2, trs_agent_notifier_func);
    if (ret != 0) {
        trs_err("Trs agent register fail. (ret=%d)\n", ret);
        return ret;
    }

    ret = agentdrv_register_non_trans_msg_client(&trs_agent_non_trans);
    if (ret != 0) {
        (void)uda_real_virtual_notifier_unregister(TRS_DEVICE_AGENT_NOTIFIER, &type);
        trs_err("Trs agent non trans msg register fail. (ret=%d)\n", ret);
        return ret;
    }

    ret = tsmng_register_abnormal_proc_func(ops->abnormal_proc);
    if (ret != 0) {
        (void)agentdrv_unregister_non_trans_msg_client(&trs_agent_non_trans);
        (void)uda_real_virtual_notifier_unregister(TRS_DEVICE_AGENT_NOTIFIER, &type);
        trs_err("Register abnormal proc func failed. (ret=%d)\n", ret);
        return ret;
    }

    mutex_init(&trs_agent_mutex);

    trs_adapt_notice_ops_register(ops);

    trs_info("Trs agent init end.\n");
    return ret;
}

void __exit exit_trs_agent(void)
{
    struct trs_adapt_notice_ops *ops = trs_agent_get_adapt_notice_ops();
    struct uda_dev_type type;

    trs_adapt_notice_ops_unregister();
    tsmng_unregister_abnormal_proc_func(ops->abnormal_proc);
    (void)agentdrv_unregister_non_trans_msg_client(&trs_agent_non_trans);
    uda_davinci_local_real_agent_type_pack(&type);
    (void)uda_real_virtual_notifier_unregister(TRS_DEVICE_AGENT_NOTIFIER, &type);
    mutex_destroy(&trs_agent_mutex);
}

module_init(init_trs_agent);
module_exit(exit_trs_agent);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("trs adapt agent driver");

