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
 * Create: 2022-08-19
 */
#ifndef AOS_LLVM_BUILD
#include <acpi/ghes.h>
#endif
/* ===must include */
#include "tsmng_interface.h"
#include "tsmng_log.h"
#include "tsmng_common.h"
/* must include=== */

#include "tsmng_pm.h"
#include "tsmng_group.h"
#include "tsmng_mem.h"
#ifdef CFG_FEATURE_TS_ABNORMAL_INFO
#include "tsmng_abnormal_info.h"
#endif

#include "devdrv_platform_resource.h"

#pragma pack(1)
struct efi_arm_error_data {
    unsigned int ValidFields;
    unsigned char Version;
    unsigned char SoCID;
    unsigned char SocketID;
    unsigned char NimbusID;
    unsigned char ModuleID;
    unsigned char SubModuleID;
    unsigned char ErrorSeverity;
    unsigned char Reserve;
    unsigned int ErrFRL;
    unsigned int ErrFRH;
    unsigned int ErrCtrlL;
    unsigned int ErrCtrlH;
    unsigned int ErrStatusL;
    unsigned int ErrStatusH;
    unsigned int ErrAddrL;
    unsigned int ErrAddrH;
    unsigned int ErrMisc0L;
    unsigned int ErrMisc0H;
    unsigned int ErrMisc1L;
    unsigned int ErrMisc1H;
};
#pragma pack()


static struct notifier_block mbox_ts_nb;
#ifdef EMU_ST
BLOCKING_NOTIFIER_HEAD(ghes_ts_err_chain);
#endif

#if defined(CFG_FEATURE_IMU_ENABLE)
static int tsmng_ts_mbx_notifier(struct notifier_block *nb, unsigned long len, void *data)
{
    struct icmdrv_msg_chan *mbx_info = NULL;
    u32 dev_id;

    if ((nb == NULL) || (data == NULL)) {
        tsmng_drv_err("Invalid parameter. (nb=%pK; data=%pK)", nb, data);
        return -EINVAL;
    }

    mbx_info = container_of(nb, struct icmdrv_msg_chan, icm_monitor);
    if (mbx_info == NULL || mbx_info->magic != ICM_MAGIC_WORD) {
        tsmng_drv_err("Invalid mbx_info. (mbx_info=%pK; magic=0x%x)",
            mbx_info, mbx_info == NULL ? 0 : mbx_info->magic);
        return -EINVAL;
    }
    dev_id = mbx_info->dev_id;
    if (dev_id >= MAX_CHIP_NUM) {
        tsmng_drv_err("Device does not exist. (dev_id=%u; max_chip_num=%d)\n", dev_id, MAX_CHIP_NUM);
        return -EINVAL;
    }

    if (icm_get_msg_version(dev_id, HISI_RPROC_MAX) == ICM_IPC_MSG_NEW_VER) {
        struct icmdrv_ipc_msg_info *ipc_msg = (struct icmdrv_ipc_msg_info *)data;
        (void)tsmng_ipc_chan_proc(ipc_msg->cmd, (void *)ipc_msg, dev_id);
    } else {
        struct ipcdrv_message *ipc_msg = (struct ipcdrv_message *)data;
        u8 type;

        type = ipc_msg->ipc_msg_header.cmd_type;
        if (type == IPCDRV_TS_INFORM_TS_IDLE) {
            (void)tsmng_inform_ts_idle((void *)ipc_msg, dev_id, 0); // tsid
        } else if (type == IPCDRV_TS_AI_COMPUTING_POWER) {
            devdrv_get_computing_power(ipc_msg, dev_id);
        } else {
            /* do nothing */
        }

        tsmng_drv_debug("ipc cmd is not support. (dev_id=%u; cmd_type=%u)\n", dev_id, type);
    }

    return 0;
}

static int tsmng_ts_req_ops(void *data, u32 devid)
{
    if (icm_get_msg_version(devid, HISI_RPROC_MAX) == ICM_IPC_MSG_NEW_VER) {
        struct icmdrv_ipc_msg_info *ipc_rx = (struct icmdrv_ipc_msg_info *)data;

        if (ipc_rx->sub_cmd == ICM_SUB_CMD_MEM_ALLOC) {
#ifdef CFG_FEATURE_TS_ALLOC_MEM
            return tsmng_ts_req_mem(data, devid);
#endif
        } else if (ipc_rx->sub_cmd == ICM_SUB_CMD_INFO) {
#ifdef CFG_FEATURE_TS_ABNORMAL_INFO
            return tsmng_ts_req_abnormal(data, devid);
#endif
        } else {
            /* do nothing */
        }

        tsmng_drv_debug("ipc cmd is not support. (dev_id=%u; cmd=%u)\n", devid, ipc_rx->sub_cmd);
    }

    return 0;
}

static int tsmng_ts_notify_ops(void *data, u32 devid)
{
    if (icm_get_msg_version(devid, HISI_RPROC_MAX) == ICM_IPC_MSG_NEW_VER) {
        struct icmdrv_ipc_msg_info *ipc_rx = (struct icmdrv_ipc_msg_info *)data;

        if (ipc_rx->sub_cmd == ICM_SUB_CMD_MEM_FREE) {
#ifdef CFG_FEATURE_TS_ALLOC_MEM
            tsmng_ts_free_mem(data, devid);
#endif
        }

        tsmng_drv_debug("ipc cmd is not support. (dev_id=%u; cmd=%u)\n", devid, ipc_rx->sub_cmd);
    }

    return 0;
}

int tsmng_mailbox_rx_register(u32 dev_id)
{
    int ret;

    if (dev_id >= MAX_CHIP_NUM) {
        tsmng_drv_err("Dev id is invalid. (dev_id=%u; max_chip_num=%d)\n", dev_id, MAX_CHIP_NUM);
        return -EINVAL;
    }

    tsmng_ipc_manager_init(dev_id);

    if (icm_get_msg_version(dev_id, HISI_RPROC_MAX) == ICM_IPC_MSG_NEW_VER) {
        tsmng_ipc_handler_register(dev_id, ICM_MAIN_CMD_TS_REQ, tsmng_ts_req_ops);
        tsmng_ipc_handler_register(dev_id, ICM_MAIN_CMD_TS_NOTIFY, tsmng_ts_notify_ops);
    }

    mbox_ts_nb.notifier_call = tsmng_ts_mbx_notifier;
    if ((ret = icm_rx_register(dev_id, HISI_RPROC_MAX, IPC_OBJ_TS, &mbox_ts_nb)) != 0) {
        tsmng_drv_err("Mailbox(%u %d) register fail...err:%d\n", dev_id, IPC_OBJ_TS, ret);
        tsmng_ipc_manager_exit(dev_id);
        return ret;
    }

    return ret;
}
EXPORT_SYMBOL(tsmng_mailbox_rx_register);

void tsmng_mailbox_rx_unregister(u32 dev_id)
{
    if (dev_id >= MAX_CHIP_NUM) {
        tsmng_drv_err("Dev id is invalid. (dev_id=%u; max_chip_num=%d)\n", dev_id, MAX_CHIP_NUM);
        return;
    }

    (void)icm_rx_unregister(dev_id, HISI_RPROC_MAX, IPC_OBJ_TS, &mbox_ts_nb);

    if (icm_get_msg_version(dev_id, HISI_RPROC_MAX) == ICM_IPC_MSG_NEW_VER) {
        tsmng_ipc_handler_unregister(dev_id, ICM_MAIN_CMD_TS_REQ);
        tsmng_ipc_handler_unregister(dev_id, ICM_MAIN_CMD_TS_NOTIFY);
    }

    tsmng_ipc_manager_exit(dev_id);
}
EXPORT_SYMBOL(tsmng_mailbox_rx_unregister);

#elif defined(CFG_FEATURE_LP_ENABLE)
int devdrv_ts_notifier(struct notifier_block *nb, unsigned long len, void *data)
{
    struct icmdrv_msg_chan *mbx_info = NULL;
    struct ipcdrv_message *ipc_msg = NULL;
    u32 tsid = 0;
    int dev_id;
    u32 m_id;
    u32 type;

    if ((nb == NULL) || (data == NULL)) {
        tsmng_drv_err("Invalid parameter. (nb=%pK; data=%pK)\n", nb, data);
        return -EINVAL;
    }

    mbx_info = container_of(nb, struct icmdrv_msg_chan, icm_monitor);
    if (mbx_info == NULL || mbx_info->magic != ICM_MAGIC_WORD) {
        tsmng_drv_err("Invalid mbx_info. (mbx_info=%pK; magic=0x%x)",
            mbx_info, mbx_info == NULL ? 0 : mbx_info->magic);
        return -EINVAL;
    }

    m_id = mbx_info->chan_id;

#ifdef CFG_FEATURE_LP_ENABLE
    devdrv_tsid_decode(m_id, &tsid);
    devdrv_handle_ipc_msg(mbx_info->dev_id, data);
#endif

    dev_id = mbx_info->dev_id;
    if (dev_id >= MAX_CHIP_NUM) {
        tsmng_drv_err("Device does not exist. (dev_id=%u; max_chip_num=%u)\n", dev_id, MAX_CHIP_NUM);
        return -EINVAL;
    }

    ipc_msg = (struct ipcdrv_message *)data;
    type = ipc_msg->ipc_msg_header.cmd_type;

    if (type == IPCDRV_TS_INFORM_TS_IDLE) {
        (void)tsmng_inform_ts_idle((void *)ipc_msg, dev_id, tsid);
    } else if ((type == IPCDRV_TS_UPPER_DDR_FREQ) || (type == IPCDRV_TS_LOWER_DDR_FREQ)) {
        (void)tsmng_ipc_chan_proc(type, (void *)ipc_msg, dev_id);
    } else if ((type == IPCDRV_TS_AI_COMPUTING_POWER) || (type == IPCDRV_TS_AIV_COMPUTING_POWER)) {
        devdrv_get_computing_power(ipc_msg, dev_id);
    }

    return 0;
}

int tsmng_mailbox_rx_register(u32 dev_id)
{
    int ret;

    if (dev_id >= MAX_CHIP_NUM) {
        tsmng_drv_err("Dev id is invalid. (dev_id=%u; max_chip_num=%d)\n", dev_id, MAX_CHIP_NUM);
        return -EINVAL;
    }

    tsmng_ipc_manager_init(dev_id);

    mbox_ts_nb.notifier_call = devdrv_ts_notifier;
    ret = icm_rx_register(dev_id, HISI_RPROC_MAX, IPC_OBJ_TSC, &mbox_ts_nb);
    if (ret != 0) {
        tsmng_drv_err("Ts mailbox register fail...err:%d\n", ret);
        return ret;
    }

    mbox_ts_nb.notifier_call = devdrv_ts_notifier;
    ret = icm_rx_register(dev_id, HISI_RPROC_MAX, IPC_OBJ_TSV, &mbox_ts_nb);
    if (ret != 0) {
        tsmng_drv_err("Ts mailbox register fail...err:%d\n", ret);
        (void)icm_rx_unregister(dev_id, HISI_RPROC_MAX, IPC_OBJ_TSC, &mbox_ts_nb);
        return ret;
    }

    return ret;
}
EXPORT_SYMBOL(tsmng_mailbox_rx_register);

void tsmng_mailbox_rx_unregister(u32 dev_id)
{
    if (dev_id >= MAX_CHIP_NUM) {
        tsmng_drv_err("Device id %u is more than %u\n", dev_id, MAX_CHIP_NUM);
        return;
    }

    (void)icm_rx_unregister(dev_id, HISI_RPROC_MAX, IPC_OBJ_TSV, &mbox_ts_nb);
    (void)icm_rx_unregister(dev_id, HISI_RPROC_MAX, IPC_OBJ_TSC, &mbox_ts_nb);
    tsmng_ipc_manager_exit(dev_id);
}
EXPORT_SYMBOL(tsmng_mailbox_rx_unregister);
#endif

#ifdef CFG_SOC_PLATFORM_CLOUD
int ghes_ts_err_callback(struct notifier_block *nb, unsigned long val, void *data)
{
    struct ipcdrv_message ipc_req_msg = {0};
    struct efi_arm_error_data *err_info = NULL;
    unsigned int device_id;
    int ret;

    tsmng_drv_info("Happen ts ras\n");

    if ((nb == NULL) || (data == NULL)) {
        tsmng_drv_err("Invalid parameter. (nb=%pK; data=%pK)\n", nb, data);
        return -EINVAL;
    }

    err_info = (struct efi_arm_error_data *)data;
    device_id = err_info->SocketID;
    if (device_id >= MAX_CHIP_NUM) {
        tsmng_drv_err("Invalid device id. (id=%u; max_device_id=%d)\n", device_id, MAX_CHIP_NUM);
        return -EINVAL;
    }

    ipc_req_msg.ipc_msg_header.msg_type = MSGTYPE_DRIVER_SEND;
    ipc_req_msg.ipc_msg_header.cmd_type = IPCDRV_TS_CORE_RAS_ERR;
    /* no need ack sync_type set 1 */
    ipc_req_msg.ipc_msg_header.sync_type = IPCDRV_MSG_ASYNC;
    ipc_req_msg.ipc_msg_header.reserved = 0;
    ipc_req_msg.ipc_msg_header.msg_length = 1;
    ipc_req_msg.ipc_msg_header.msg_index = 0;
    ipc_req_msg.ipcdrv_payload[0] = err_info->SubModuleID;

    ret = icm_msg_send_async(ICM_FD_BUILD(device_id, HISI_RPROC_TX_TS_MBX17), (rproc_msg_t *)&ipc_req_msg,
        IPCDRV_RPROC_MSG_LENGTH);
    if (ret) {
        tsmng_drv_err("Icm_msg_send_async failed.\n");
        return ret;
    }

    tsmng_drv_info("Send ras msg sucess\n");
    return 0;
}

static struct notifier_block ghes_ts_err_nb = {
    .notifier_call = ghes_ts_err_callback,
};

void tsmng_ghes_ts_err_register(void)
{
    (void)blocking_notifier_chain_register(&ghes_ts_err_chain, &ghes_ts_err_nb);
}
EXPORT_SYMBOL(tsmng_ghes_ts_err_register);

void tsmng_ghes_ts_err_unregister(void)
{
    (void)blocking_notifier_chain_unregister(&ghes_ts_err_chain, &ghes_ts_err_nb);
}
EXPORT_SYMBOL(tsmng_ghes_ts_err_unregister);
#endif

#ifdef TSDRV_UT
void tsmng_notifier_ut_stub(void)
{
    return;
}
#endif

