/*
* Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
* Create: 2022-11-4
*/
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/pci.h>

#include "devdrv_interface.h"
#include "virtmng_interface.h"
#include "dbl/uda.h"

#include "trs_host_init.h"

#include "trs_sec_eh_agent_msg.h"
#include "trs_sec_eh_msg.h"
#include "trs_sec_eh_cfg.h"
#include "trs_sec_eh_id.h"
#include "trs_sec_eh_sq.h"

static const struct pci_device_id sec_eh_agent_tbl[] = {
    {PCI_VDEVICE(HUAWEI, 0xd802), 0},
    {}
};
MODULE_DEVICE_TABLE(pci, sec_eh_agent_tbl);

static struct mutex sec_eh_cfg_mutex;
static int trx_sec_eh_msg_para_check(u32 dev_id, u32 fid, struct vmng_rx_msg_proc_info *proc_info)
{
    if ((proc_info == NULL) || (proc_info->real_out_len == NULL) || (proc_info->data == NULL) ||
        (dev_id >= TRS_DEV_MAX_NUM)) {
        trs_err("Para error. (devid=%u; fid=%u)\n", dev_id, fid);
        return -EINVAL;
    }

    if (proc_info->in_data_len < sizeof(struct trs_sec_eh_msg_head)) {
        trs_err("Check faild. (in_data_len=%u; expected_len=%u\n", proc_info->in_data_len,
            (u32)sizeof(struct trs_sec_eh_msg_head));
        return -EINVAL;
    }

    return 0;
}

static int sec_eh_vpc_msg_recv(u32 dev_id, u32 fid, struct vmng_rx_msg_proc_info *proc_info)
{
    struct trs_sec_eh_msg_head *head = NULL;
    int ret = -EINVAL;

    if (trx_sec_eh_msg_para_check(dev_id, fid, proc_info) != 0) {
        return -EINVAL;
    }

    head = (struct trs_sec_eh_msg_head *)proc_info->data;
    if ((head->cmd_type < TRS_SEC_EH_MAX) && (head->cmd_type >= 0) &&
        (trs_sec_eh_get_vpc_func(head->cmd_type) != NULL)) {
        ret = trs_sec_eh_get_vpc_func(head->cmd_type)(dev_id, proc_info);
    }

    head->result = ret;
    trs_debug("Rsv vpc msg. (devid=%u; tsid=%u; fid=%u; cmd=%d)\n", dev_id, head->tsid, fid, head->cmd_type);

    return 0;
}

static struct vmng_vpc_client trs_sec_eh_vpc_client = {
    .vpc_type = VMNG_VPC_TYPE_TSDRV,
    .init = NULL,
    .msg_recv = sec_eh_vpc_msg_recv,
};

struct trs_irq_inject {
    u32 devid;
    u32 vector_id;
    u32 irq;
};

#define TRS_VDEV_MAX_SQ_TRIGGER_IRQ_NUM 1
#define TRS_VDEV_MAX_CQ_UPDATE_IRQ_NUM 8
struct trs_inject_irq_info {
    int cq_update_irq_cnt;
    struct trs_irq_inject cq_update[TRS_VDEV_MAX_CQ_UPDATE_IRQ_NUM];
    int sq_trigger_irq_cnt;
    struct trs_irq_inject sq_trigger[TRS_VDEV_MAX_SQ_TRIGGER_IRQ_NUM];
};
static struct trs_inject_irq_info *trs_inject_irq[TRS_DEV_MAX_NUM];

static irqreturn_t trs_sec_eh_irq_inject_proc(int irq, void *para)
{
    struct trs_irq_inject *inject = (struct trs_irq_inject *)para;

    (void)vmngh_hypervisor_inject_msix(inject->devid, inject->vector_id);
    trs_debug("Irq proc. (devid=%u; vector=%u; irq=%u)\n", inject->devid, inject->vector_id, inject->irq);
    return IRQ_HANDLED;
}

static void trs_sec_eh_irq_unrequest(u32 devid)
{
    struct trs_inject_irq_info *inject = trs_inject_irq[devid];
    int i;

    if (inject == NULL) {
        trs_err("Irq is repeated free. (devid=%u)\n", devid);
        return;
    }

    for (i = 0; i < inject->sq_trigger_irq_cnt; i++) {
        (void)free_irq(inject->sq_trigger[i].irq, &inject->sq_trigger[i]);
    }

    for (i = 0; i < inject->cq_update_irq_cnt; i++) {
        (void)free_irq(inject->cq_update[i].irq, &inject->cq_update[i]);
    }

    trs_inject_irq[devid] = NULL;
    kfree(inject);
}

static int trs_sec_eh_trigger_irq_request(u32 devid, struct trs_inject_irq_info *inject)
{
    u32 irq, irq_request;
    int ret;

    ret = devdrv_get_ts_drv_irq_vector_id(devid, 1, &irq);
    if (ret != 0) {
        trs_err("Get sq trigger irq vector failed. (devid=%u; ret=%d)\n", devid, ret);
        return ret;
    }

    ret = devdrv_get_irq_vector(devid, irq, &irq_request);
    if (ret != 0) {
        trs_err("Get sq trigger req_irq failed. (devid=%u; ret=%d)\n", devid, ret);
        return ret;
    }

    inject->sq_trigger[0].devid = devid;
    inject->sq_trigger[0].vector_id = irq;
    inject->sq_trigger[0].irq = irq_request;
    ret = request_irq(irq_request, trs_sec_eh_irq_inject_proc, 0, "sq_trigger_inject", &inject->sq_trigger[0]);
    if (ret != 0) {
        trs_err("Request irq failed. (irq=%u; ret=%d)\n", irq, ret);
        return ret;
    }
    inject->sq_trigger_irq_cnt++;

    trs_info("Request sq trigger irq success. (devid=%u)\n", devid);
    return 0;
}

static int trs_sec_eh_irq_request(u32 devid)
{
    struct trs_inject_irq_info *inject = NULL;
    int ret, i;
    u32 irq, irq_request, idx;

    inject = kzalloc(sizeof(struct trs_inject_irq_info), GFP_KERNEL);
    if (inject == NULL) {
        trs_err("Kmalloc failed. (devid=%u)\n", devid);
        return -ENOMEM;
    }
    trs_inject_irq[devid] = inject;

    ret = trs_sec_eh_trigger_irq_request(devid, inject);
    if (ret != 0) {
        trs_err("Request irq failed. (irq=%u)\n", irq);
        goto free_irq;
    }

    idx = 2; /* cq update start from 2 */
    for (i = 0; i < TRS_VDEV_MAX_CQ_UPDATE_IRQ_NUM; i++, idx++) {
        ret = devdrv_get_ts_drv_irq_vector_id(devid, idx, &irq);
        if (ret != 0) {
            trs_err("Get mb irq vector failed. (devid=%u)\n", devid);
            goto  free_irq;
        }

        ret = devdrv_get_irq_vector(devid, irq, &irq_request);
        if (ret != 0) {
            trs_err("Get mb req_irq failed. (devid=%u)\n", devid);
            goto  free_irq;
        }

        inject->cq_update[i].devid = devid;
        inject->cq_update[i].vector_id = irq;
        inject->cq_update[i].irq = irq_request;
        ret = request_irq(irq_request, trs_sec_eh_irq_inject_proc, 0, "cqe_done_inject", &inject->cq_update[i]);
        if (ret != 0) {
            trs_err("Request irq failed. (irq=%u)\n", irq);
            goto  free_irq;
        }
        inject->cq_update_irq_cnt++;
    }

    return 0;
free_irq:
    trs_sec_eh_irq_unrequest(devid);

    return -EINVAL;
}

static int trs_sec_eh_init_instance(u32 udevid)
{
    struct trs_id_inst inst;
    u32 fid = 0;
    u32 tsid = 0;
    int ret;

    trs_id_inst_pack(&inst, udevid, tsid);
    mutex_lock(&sec_eh_cfg_mutex);
    ret = trs_sec_eh_ts_inst_create(&inst);
    mutex_unlock(&sec_eh_cfg_mutex);
    if (ret != 0) {
        trs_err("Failed. (devid=%u, fid=%u)\n", udevid, fid);
        return ret;
    }

    ret = trs_sec_eh_irq_request(udevid);
    if (ret != 0) {
        trs_sec_eh_ts_inst_destroy(&inst);
        trs_err("Irq failed. (dev=%u; fid=%u; ret=%d)\n", udevid, fid, ret);
        return ret;
    }

    ret = trs_ts_hw_init(&inst);
    if (ret != 0) {
        trs_sec_eh_irq_unrequest(udevid);
        trs_sec_eh_ts_inst_destroy(&inst);
        trs_err("Mbox cfg failed. (dev=%u; fid=%u; ret=%d)\n", udevid, fid, ret);
        return ret;
    }

    trs_sec_eh_id_config(&inst);

    ret = vmngh_vpc_register_client(udevid, fid, &trs_sec_eh_vpc_client);
    if (ret != 0) {
        trs_sec_eh_id_deconfig(&inst);
        trs_ts_hw_uninit(&inst);
        trs_sec_eh_irq_unrequest(udevid);
        trs_sec_eh_ts_inst_destroy(&inst);
        trs_err("Vpc client register failed. (dev=%u; fid=%u; ret=%d)\n", udevid, fid, ret);
        return ret;
    }

    trs_info("Init instance success. (dev=%u; fid=%u)\n", udevid, fid);
    return 0;
}

static void trs_sec_eh_uninit_instance(u32 udevid)
{
    struct trs_id_inst inst;
    u32 fid = 0;
    u32 tsid = 0;

    vmngh_vpc_unregister_client(udevid, fid, &trs_sec_eh_vpc_client);

    trs_id_inst_pack(&inst, udevid, tsid);
    trs_sec_eh_id_deconfig(&inst);
    trs_ts_hw_uninit(&inst);

    trs_sec_eh_free_sq_mem_all(&inst);
    trs_sec_eh_irq_unrequest(udevid);

    mutex_lock(&sec_eh_cfg_mutex);
    trs_sec_eh_ts_inst_destroy(&inst);
    mutex_unlock(&sec_eh_cfg_mutex);

    trs_info("Uninit instance success. (dev=%u; fid=%u)\n", udevid, fid);
}

#define TRS_SEC_EH_AGENT_NOTIFIER "sec_eh_trs_agent"
static int trs_sec_eh_agent_notifier_func(u32 udevid, enum uda_notified_action action)
{
    int ret = 0;

    if (udevid >= TRS_DEV_MAX_NUM) {
        trs_err("Invalid para. (udevid=%u; action=%d)\n", udevid, action);
        return -EINVAL;
    }

    if (action == UDA_INIT) {
        ret = trs_sec_eh_init_instance(udevid);
    } else if (action == UDA_UNINIT) {
        trs_sec_eh_uninit_instance(udevid);
    }

    trs_info("notifier action. (udevid=%u; action=%d; ret=%d)\n", udevid, action, ret);

    return ret;
}

int __init init_sec_eh_trs_agent(void)
{
    struct uda_dev_type type;
    int ret;

    uda_dev_type_pack(&type, UDA_DAVINCI, UDA_ENTITY, UDA_NEAR, UDA_REAL_SEC_EH);
    ret = uda_notifier_register(TRS_SEC_EH_AGENT_NOTIFIER, &type, UDA_PRI1, trs_sec_eh_agent_notifier_func);
    if (ret != 0) {
        trs_err("Register notifier failed. (ret=%d)\n", ret);
        return ret;
    }

    mutex_init(&sec_eh_cfg_mutex);

    trs_info("Sec eh agent init success\n");
    return 0 ;
}

void __exit exit_sec_eh_trs_agent(void)
{
    struct uda_dev_type type;

    mutex_destroy(&sec_eh_cfg_mutex);

    uda_dev_type_pack(&type, UDA_DAVINCI, UDA_ENTITY, UDA_NEAR, UDA_REAL_SEC_EH);
    (void)uda_notifier_unregister(TRS_SEC_EH_AGENT_NOTIFIER, &type);

    trs_info("Sec eh agent exit success\n");
    return;
}

module_init(init_sec_eh_trs_agent);
module_exit(exit_sec_eh_trs_agent);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("trs sec enhanced agent driver");

