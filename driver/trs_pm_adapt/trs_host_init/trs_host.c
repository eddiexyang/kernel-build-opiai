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
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/kref.h>
#include <linux/spinlock.h>
#include <linux/pci.h>

#include "dbl/uda.h"
#include "soc_res.h"
#include "devdrv_interface.h"
#include "trs_chip_def_comm.h"
#include "trs_host_init.h"
#include "trs_host_msg.h"
#include "trs_host_comm.h"
#include "trs_msg.h"
#include "trs_cdqm.h"
#include "trs_stars_comm.h"
#include "trs_core.h"
#include "trs_host.h"

#define DEVDRV_DIVERSITY_PCIE_VENDOR_ID 0xFFFF
static const struct pci_device_id trs_pm_adapt_tbl[] = {
    { PCI_VDEVICE(HUAWEI, 0xd100),           0 },
    { PCI_VDEVICE(HUAWEI, 0xd105),           0 },
    { PCI_VDEVICE(HUAWEI, 0xa126), 0 },
    { PCI_VDEVICE(HUAWEI, 0xd801),           0 },
    { PCI_VDEVICE(HUAWEI, 0xd500),           0 },
    { PCI_VDEVICE(HUAWEI, 0xd501),           0 },
    { PCI_VDEVICE(HUAWEI, 0xd802),           0 },
    { PCI_VDEVICE(HUAWEI, 0xd803),           0 },
    { PCI_VDEVICE(HUAWEI, 0xd804),           0 },
    { DEVDRV_DIVERSITY_PCIE_VENDOR_ID, 0xd500, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
    {}
};
MODULE_DEVICE_TABLE(pci, trs_pm_adapt_tbl);
#ifdef CFG_FEATURE_CDQM
static void trs_host_cdqm_uninit(u32 devid);
#endif

static const trs_msg_rdv_func_t rcv_ops[TRS_MSG_MAX] = {
#ifdef CFG_FEATURE_CDQM
    [TRS_MSG_CDQM_INIT] = cdqm_proc_sync_msg,
    [TRS_MSG_CDQM_CREATE] = cdqm_proc_sync_msg,
    [TRS_MSG_CDQM_DESTROY] = cdqm_proc_sync_msg,
    [TRS_MSG_CDQM_BATCH_ABNORMAL] = cdqm_proc_sync_msg,
#endif
    [TRS_MSG_CHAN_ABNORMAL] = trs_host_ts_adapt_abnormal_proc,
    [TRS_MSG_SET_TS_STATUS] = trs_host_set_ts_status,
    [TRS_MSG_FLUSH_RES_ID] = trs_host_flush_id,
};

int trs_host_msg_chan_recv(void *msg_chan, void *data, u32 in_data_len,
    u32 out_data_len, u32 *real_out_len)
{
    struct trs_msg_data *_data = (struct trs_msg_data *)data;
    u32 devid = (u32)(uintptr_t)devdrv_get_msg_chan_priv(msg_chan);
    int ret = ENODEV;

    ret = trs_host_msg_chan_recv_check(devid, _data, in_data_len, out_data_len, real_out_len);
    if (ret != 0) {
        trs_err("Msg rcv check fail. (ret=%d)\n", ret);
        return ret;
    }
    if (rcv_ops[_data->header.cmdtype] != NULL) {
        ret = rcv_ops[_data->header.cmdtype](devid, _data);
        *real_out_len = (u32)sizeof(struct trs_msg_data);
        _data->header.valid = TRS_MSG_RCV_MAGIC;
        _data->header.result = ret;
    }
    return 0;
}

#define TRS_HOST_MSG_CHAN_SIZE (TRS_MSG_TOTAL_LEN + 128) /* 128 rsv for non trans msg head */
static struct devdrv_non_trans_msg_chan_info trs_host_msg_chan_info = {
    .msg_type = devdrv_msg_client_tsdrv,
    .flag = 0,
    .level = DEVDRV_MSG_CHAN_LEVEL_LOW,
    .s_desc_size = TRS_HOST_MSG_CHAN_SIZE,
    .c_desc_size = TRS_HOST_MSG_CHAN_SIZE,
    .rx_msg_process = trs_host_msg_chan_recv,
};

#ifdef CFG_FEATURE_CDQM
static int trs_host_cdqm_init(u32 devid)
{
    struct cdqm_adapt_ops ops;
    struct trs_id_inst inst;
    u32 ts_num, tsid;
    int ret;

    if (devid >= 100) { /* temporary 100 */
        return 0;
    }

    ret = soc_resmng_subsys_get_num(devid, TS_SUBSYS, &ts_num);
    if ((ret != 0) || (ts_num == 0) || (ts_num > TRS_TS_MAX_NUM)) {
        trs_err("Get tsnum fail. (devid=%u; ret=%d; ts_num=%u)\n", devid, ret, ts_num);
        return -EFAULT;
    }

    for (tsid = 0; tsid < ts_num; tsid++) {
        trs_id_inst_pack(&inst, devid, tsid);
        ops.send_sync_msg = trs_host_msg_send;
        ops.ssid_query = trs_host_get_ssid;
        (void)cdqm_ts_inst_register(&inst, &ops);
    }

    return 0;
}
#endif

static int trs_host_inst_init(u32 devid)
{
    int ret;

    ret = trs_host_msg_init(devid, &trs_host_msg_chan_info);
    if (ret != 0) {
        trs_err("Msg init fail. (devid=%u; ret=%d)\n", devid, ret);
        return ret;
    }
    ret = trs_host_init(devid);
    if (ret != 0) {
        trs_host_msg_uninit(devid);
        trs_err("Host init fail. (devid=%u)\n", devid);
        return ret;
    }
#ifdef CFG_FEATURE_CDQM
    ret = trs_host_cdqm_init(devid);
    if (ret != 0) {
        trs_host_uninit(devid);
        trs_host_msg_uninit(devid);
        trs_err("Cdqm init fail. (devid=%u; ret=%d)\n", devid, ret);
        return ret;
    }
#endif

    return 0;
}

#ifdef CFG_FEATURE_CDQM
void trs_host_cdqm_uninit(u32 devid)
{
    struct trs_id_inst inst;
    u32 ts_num, tsid;
    int ret;

    if (devid >= 100) { /* temporary 100 */
        return;
    }

    ret = soc_resmng_subsys_get_num(devid, TS_SUBSYS, &ts_num);
    if ((ret != 0) || (ts_num == 0) || (ts_num > TRS_TS_MAX_NUM)) {
        trs_err("Get tsnum fail. (devid=%u; ret=%d; ts_num=%u)\n", devid, ret, ts_num);
        return;
    }

    for (tsid = 0; tsid < ts_num; tsid++) {
        trs_id_inst_pack(&inst, devid, tsid);
        (void)cdqm_ts_inst_unregister(&inst);
    }
}
#endif

static void trs_host_inst_uninit(u32 devid)
{
#ifdef CFG_FEATURE_CDQM
    trs_host_cdqm_uninit(devid);
#endif
    trs_host_uninit(devid);
    trs_host_msg_uninit(devid);
}

#define TRS_NEAR_NOTIFIER "trs_near"
static int trs_near_notifier_func(u32 udevid, enum uda_notified_action action)
{
    struct trs_id_inst pm_inst;
    int ret = 0;

    if (udevid >= TRS_DEV_MAX_NUM) {
        trs_err("Invalid para. (udevid=%u)\n", udevid);
        return -EINVAL;
    }

    trs_id_inst_pack(&pm_inst, udevid, 0);
    if (action == UDA_INIT) {
        ret = trs_host_inst_init(udevid);
    } else if (action == UDA_UNINIT) {
        trs_host_inst_uninit(udevid);
    } else if (action == UDA_TO_MIA) {
        ret = trs_set_ts_inst_feature_mode(&pm_inst, TRS_INST_PART_FEATUR_MODE, 0);
        if (ret != 0) {
            trs_err("Failed to mia. (devid=%u; ret=%d)\n", udevid, ret);
            return ret;
        }
    } else if (action == UDA_TO_SIA) {
        (void)trs_set_ts_inst_feature_mode(&pm_inst, TRS_INST_ALL_FEATUR_MODE, 0);
    }

    trs_info("notifier action. (udevid=%u; action=%d; ret=%d)\n", udevid, action, ret);

    return ret;
}

int __init init_trs_adapt(void)
{
    struct uda_dev_type type;
    uda_davinci_near_real_entity_type_pack(&type);
    return uda_notifier_register(TRS_NEAR_NOTIFIER, &type, UDA_PRI2, trs_near_notifier_func);
}
module_init(init_trs_adapt);

void __exit exit_trs_adapt(void)
{
    struct uda_dev_type type;
    uda_davinci_near_real_entity_type_pack(&type);
    (void)uda_notifier_unregister(TRS_NEAR_NOTIFIER, &type);
}
module_exit(exit_trs_adapt);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("TRS HOST PM ADAPT DRVIER");
MODULE_SOFTDEP("pre: ascend_soc_platform");

