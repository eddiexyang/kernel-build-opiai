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
* Create: 2022-11-09
*/
#include <linux/types.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/pci.h>

#include "dbl/uda.h"

#include "devdrv_interface.h"
#include "virtmng_interface.h"
#include "trs_msg.h"
#include "trs_host_msg.h"
#include "trs_host_comm.h"
#include "trs_pub_def.h"
#include "trs_sec_eh_core.h"
#include "trs_sec_eh_vpc.h"
#include "trs_sec_eh_init.h"
#include "trs_sec_eh.h"

static const struct pci_device_id sec_eh_adapt_tbl[] = {
    {PCI_VDEVICE(HUAWEI, 0xd802), 0},
    {}
};
MODULE_DEVICE_TABLE(pci, sec_eh_adapt_tbl);

static const trs_msg_rdv_func_t rcv_ops[TRS_MSG_MAX] = {
    [TRS_MSG_CHAN_ABNORMAL] = trs_host_ts_adapt_abnormal_proc,
    [TRS_MSG_SET_TS_STATUS] = trs_host_set_ts_status,
    [TRS_MSG_FLUSH_RES_ID] = trs_host_flush_id,
};

static int trs_sec_eh_msg_chan_recv(void *msg_chan, void *data, u32 in_data_len,
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
        _data->header.result = (s16)ret;
    }

    return 0;
}

#define TRS_SEC_EH_MSG_CHAN_SIZE (TRS_MSG_TOTAL_LEN + 128) /* 128 rsv for non trans msg head */
static struct devdrv_non_trans_msg_chan_info trs_sec_eh_msg_chan_info = {
    .msg_type = devdrv_msg_client_tsdrv,
    .flag = 0,
    .level = DEVDRV_MSG_CHAN_LEVEL_LOW,
    .s_desc_size = TRS_SEC_EH_MSG_CHAN_SIZE,
    .c_desc_size = TRS_SEC_EH_MSG_CHAN_SIZE,
    .rx_msg_process = trs_sec_eh_msg_chan_recv,
};

static int trs_sec_eh_init_inst(u32 devid)
{
    int ret;

    ret = trs_host_msg_init(devid, &trs_sec_eh_msg_chan_info);
    if (ret != 0) {
        return ret;
    }

    ret = trs_sec_eh_vpc_init(devid);
    if (ret != 0) {
        trs_host_msg_uninit(devid);
        trs_err("Vpc init fail. (devid=%u; ret=%d)\n", devid, ret);
        return ret;
    }
    ret = trs_sec_eh_init(devid);
    if (ret != 0) {
        trs_sec_eh_vpc_uninit(devid);
        trs_host_msg_uninit(devid);
        trs_err("Init fail. (devid=%u; ret=%d)\n", devid, ret);
        return ret;
    }

    trs_info("Init. (devid=%u)\n", devid);
    return 0;
}

static int trs_sec_eh_uninit_inst(u32 devid)
{
    trs_sec_eh_unint(devid);
    trs_sec_eh_vpc_uninit(devid);
    trs_host_msg_uninit(devid);
    return 0;
}

#define TRS_SEC_EH_NOTIFIER "trs_sec_eh"
static int trs_sec_eh_notifier_func(u32 udevid, enum uda_notified_action action)
{
    int ret = 0;

    if (udevid >= TRS_DEV_MAX_NUM) {
        trs_err("Invalid para. (udevid=%u)\n", udevid);
        return -EINVAL;
    }

    if (action == UDA_INIT) {
        ret = trs_sec_eh_init_inst(udevid);
    } else if (action == UDA_UNINIT) {
        ret = trs_sec_eh_uninit_inst(udevid);
    }

    trs_info("notifier action. (udevid=%u; action=%d; ret=%d)\n", udevid, action, ret);

    return ret;
}

int __init init_trs_sec_eh(void)
{
    struct uda_dev_type type;
    uda_davinci_near_real_entity_type_pack(&type);
    return uda_notifier_register(TRS_SEC_EH_NOTIFIER, &type, UDA_PRI2, trs_sec_eh_notifier_func);
}
module_init(init_trs_sec_eh);

void __exit exit_trs_sec_eh(void)
{
    struct uda_dev_type type;
    uda_davinci_near_real_entity_type_pack(&type);
    (void)uda_notifier_unregister(TRS_SEC_EH_NOTIFIER, &type);
}
module_exit(exit_trs_sec_eh);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("TRS SEC EH ADAPT DRVIER");
MODULE_SOFTDEP("pre: ascend_soc_platform");

