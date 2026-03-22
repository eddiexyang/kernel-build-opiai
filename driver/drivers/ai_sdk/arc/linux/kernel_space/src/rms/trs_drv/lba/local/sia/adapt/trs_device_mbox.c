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
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/kref.h>
#include <linux/mm.h>

#include "soc_res.h"

#include "trs_mailbox_def.h"
#include "trs_mbox.h"
#include "trs_device_mbox.h"
#include "trs_ts_db.h"
#include "trs_chip_def.h"

#define TRS_MBXO_DEFAULT_TIMEOUT    3000
#define TRS_MB_SIZE    64

struct trs_ts_mbox {
    struct trs_id_inst inst;

    atomic_t retry;
    struct kref ref;

    phys_addr_t base;   // shared memory base
    size_t size;        // shared memory size

    u32 db_index;
    int db_type;

    char *name;         // irq name
    u32 irq;            // ack irq
    void *chan;

    u32 val;            // value for wring to db
};

static irqreturn_t trs_ts_mbox_ack_handler(int irq, void *data)
{
    struct trs_ts_mbox *ts_mbox = (struct trs_ts_mbox *)data;

    trs_mbox_chan_txdone(ts_mbox->chan);
    return IRQ_HANDLED;
}

static int trs_ts_mbox_irq_setup(struct trs_ts_mbox *ts_mbox)
{
    char *name = NULL;
    int ret;

    /* name allocated inside of kasprintf with kmalloc. name should be freed with kfree */
    name = kasprintf(GFP_KERNEL, "trs-mbox-%u-%u", ts_mbox->inst.devid, ts_mbox->inst.tsid);
    if (name == NULL) {
        trs_err("Irq name pack fail.\n");
        return -ENOMEM;
    }

    ret = request_irq(ts_mbox->irq, trs_ts_mbox_ack_handler, 0, name, ts_mbox);
    if (ret != 0) {
        kfree(name);
        trs_err("Requet irq fail. (devid=%u; tsid=%u; irq=%u; ret=%d)\n",
            ts_mbox->inst.devid, ts_mbox->inst.tsid, ts_mbox->irq, ret);
        return ret;
    }
#ifdef CFG_FEATURE_IRQ_BIND
    (void)irq_set_affinity_hint(ts_mbox->irq, get_cpu_mask(0));
#endif

    trs_debug("Request irq success. (irq=%u)\n", ts_mbox->irq);

    ts_mbox->name = name;
    return ret;
}

static void trs_ts_mbox_irq_cleanup(struct trs_ts_mbox *ts_mbox)
{
#ifdef CFG_FEATURE_IRQ_BIND
    (void)irq_set_affinity_hint(ts_mbox->irq, NULL);
#endif
    (void)free_irq(ts_mbox->irq, ts_mbox);
    if (ts_mbox->name != NULL) {
        kfree(ts_mbox->name);
        ts_mbox->name = NULL;
    }
    trs_info("Free irq success. (irq=%u)\n", ts_mbox->irq);
}

static int trs_ts_mbox_trigger_irq(void *data)
{
    struct trs_ts_mbox *ts_mbox = (struct trs_ts_mbox *)data;
    int ret = -ENODEV;

    if (ts_mbox != NULL) {
        ret = trs_ring_ts_db(&ts_mbox->inst, ts_mbox->db_type, ts_mbox->db_index, ++ts_mbox->val);
    }
    return ret;
}

static int trs_ts_mbox_setup(struct trs_ts_mbox *ts_mbox)
{
    int ret;

    ret = trs_ts_mbox_irq_setup(ts_mbox);
    if (ret != 0) {
        trs_err("Mbox irq setup fail. (ret=%d)\n", ret);
    }

    return ret;
}

static void trs_ts_mbox_cleanup(void *priv)
{
    struct trs_ts_mbox *ts_mbox = (struct trs_ts_mbox *)priv;
    trs_ts_mbox_irq_cleanup(ts_mbox);
}

static int trs_ts_mbox_db_init(struct trs_id_inst *inst)
{
    u32 start, end;
    int ret;

    trs_get_mbox_db_cfg(inst->devid, inst->tsid, &start, &end);
    ret = trs_ts_db_init(inst, TRS_DB_OFFLINE_MBOX, start, end);
    if (ret != 0) {
        trs_err("Trs db init fail. (devid=%u; tsid=%u; ret=%d)\n", inst->devid, inst->tsid, ret);
    }

    return ret;
}

static void trs_ts_mbox_db_uninit(struct trs_id_inst *inst)
{
    trs_ts_db_uninit(inst, TRS_DB_OFFLINE_MBOX);
}

static struct trs_ts_mbox *trs_ts_mbox_create(struct trs_id_inst *inst)
{
    struct trs_ts_mbox *ts_mbox = NULL;
    struct res_inst_info res_inst;
    struct soc_rsv_mem_info sram;
    u32 irq;
    int ret;

    ts_mbox = kzalloc(sizeof(struct trs_ts_mbox), GFP_KERNEL);
    if (ts_mbox == NULL) {
        return NULL;
    }

    ret = trs_ts_mbox_db_init(inst);
    if (ret != 0) {
        kfree(ts_mbox);
        return NULL;
    }

    soc_resmng_inst_pack(&res_inst, inst->devid, TS_SUBSYS, inst->tsid);
    ret = soc_resmng_get_rsv_mem(&res_inst, "TS_SRAM_MEM", &sram);
    ret |= soc_resmng_get_irq_by_index(&res_inst, TS_MAILBOX_ACK_IRQ, 0, &irq);
    if ((ret != 0) || (sram.rsv_mem_size < TRS_MB_SIZE)) {
        trs_ts_mbox_db_uninit(inst);
        kfree(ts_mbox);
        trs_err("Get soc resmng fail. (devid=%u; tsid=%u; ret=%d)\n", inst->devid, inst->tsid, ret);
        return NULL;
    }
    ts_mbox->db_index = 0;
    ts_mbox->db_type = TRS_DB_OFFLINE_MBOX;
    ts_mbox->inst = *inst;
    ts_mbox->irq = irq;
    ts_mbox->base = sram.rsv_mem;
    ts_mbox->size = TRS_MB_SIZE;
    ts_mbox->val = 0;
    kref_init(&ts_mbox->ref);

    return ts_mbox;
}

static void trs_ts_mbox_destroy(struct trs_ts_mbox *ts_mbox)
{
    trs_ts_mbox_db_uninit(&ts_mbox->inst);
    kfree(ts_mbox);
}

static void trs_ts_mbox_release(void *priv)
{
    struct trs_ts_mbox *ts_mbox = (struct trs_ts_mbox *)priv;

    trs_info("Trs mbox uninit. (devid=%u; tsid=%u)\n", ts_mbox->inst.devid, ts_mbox->inst.tsid);
    trs_ts_mbox_destroy(ts_mbox);
}

static void trs_mbox_chan_attr_pack(struct trs_ts_mbox *ts_mbox, struct trs_mbox_chan_attr *attr)
{
    attr->base = ts_mbox->base;
    attr->size = ts_mbox->size;
    attr->priv = ts_mbox;
    attr->ops.mbox_release = trs_ts_mbox_release;
    attr->ops.trigger_irq = trs_ts_mbox_trigger_irq;
    attr->ops.free_irq = trs_ts_mbox_cleanup;
}

int trs_mbox_config(struct trs_id_inst *inst)
{
    struct trs_ts_mbox *ts_mbox = NULL;
    struct trs_mbox_chan_attr attr;
    int ret;

    ts_mbox = trs_ts_mbox_create(inst);
    if (ts_mbox == NULL) {
        trs_err("Trs ts mbox create fail. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return -ENODEV;
    }

    ret = trs_ts_mbox_setup(ts_mbox);
    if (ret != 0) {
        trs_ts_mbox_destroy(ts_mbox);
        trs_err("Trs ts mbox setup fail. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return ret;
    }

    trs_mbox_chan_attr_pack(ts_mbox, &attr);
    ts_mbox->chan = trs_mbox_chan_init(&ts_mbox->inst, &attr);
    if (ts_mbox->chan == NULL) {
        trs_ts_mbox_cleanup(ts_mbox);
        trs_ts_mbox_destroy(ts_mbox);
        return -EFAULT;
    }
    return 0;
}

void trs_mbox_deconfig(struct trs_id_inst *inst)
{
    trs_mbox_chan_uninit(inst);
}

int devdrv_send_rdmainfo_to_ts(u32 devid, const u8 *buf, u32 len, int *result)
{
    struct trs_id_inst inst;
    struct trs_rdma_info msg;
    u32 tsid = 0;
    int ret;

    if ((buf == NULL) || (result == NULL) || (len > MAX_RDMA_INFO_LEN)) {
        return -EINVAL;
    }

    trs_mbox_init_header(&msg.header, TRS_MBOX_SEND_RDMA_INFO);
    memcpy(msg.buf, buf, len);

    trs_id_inst_pack(&inst, devid, tsid);
    ret = trs_mbox_send(&inst, 0, (void *)&msg, sizeof(msg), TRS_MBXO_DEFAULT_TIMEOUT);
    if ((ret != 0) || (msg.header.result != 0)) {
        trs_err("Trs mbox send fail. (devid=%u; tsid=%u; ret=%d; result=%d)\n", devid, tsid, ret, msg.header.result);
        return -EFAULT;
    }
    *result = 0;
    return 0;
}
