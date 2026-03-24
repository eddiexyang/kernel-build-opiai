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
 * Create: 2022-08-13
 */


#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "devdrv_platform_resource.h"

/* ===must include */
#include "tsmng_log.h"
#include "tsmng_common.h"
/* must include=== */

#include "tsmng_pm.h"
#include "icm_interface.h"
#ifdef CFG_FEATURE_TS_ABNORMAL_INFO
#include "tsmng_abnormal_info.h"
#endif

#ifdef AOS_LLVM_BUILD
#include "tsmng_group.h"
#endif

static tsmng_handler tsmng_ipc_rx_handlers[MAX_CHIP_NUM][ICM_MAIN_CMD_MAX] = {NULL};

int tsmng_ipc_handler_register(u32 dev_id, u32 cmd_type, tsmng_handler handler)
{
    u32 max_cmd_type;

#ifndef TSDRV_UT
    if (icm_get_msg_version(dev_id, HISI_RPROC_MAX) == ICM_IPC_MSG_NEW_VER) {
        max_cmd_type = ICM_MAIN_CMD_MAX;
    } else {
        max_cmd_type = IPCDRV_TS_MAX;
    }
#endif

    if (dev_id >= MAX_CHIP_NUM) {
        tsmng_drv_err("Dev id is invalid. (dev_id=%u; max_chip_num=%d)\n", dev_id, MAX_CHIP_NUM);
        return -EINVAL;
    }

    if (cmd_type >= max_cmd_type) {
        tsmng_drv_err("Cmd type is invalid. (cmd_type=%u; max_cmd_type=%u)\n", cmd_type, max_cmd_type);
        return -EINVAL;
    }

    if (tsmng_ipc_rx_handlers[dev_id][cmd_type] != NULL) {
        tsmng_drv_err("Ipc handler has been register. (dev_id=%u; cmd_type=%u)\n", dev_id, cmd_type);
        return -EINVAL;
    }

    tsmng_ipc_rx_handlers[dev_id][cmd_type] = handler;
    return 0;
}

int tsmng_handler_register_upper_ddrfreq(u32 dev_id, tsmng_handler handler)
{
    return tsmng_ipc_handler_register(dev_id, IPCDRV_TS_UPPER_DDR_FREQ, handler);
}
EXPORT_SYMBOL(tsmng_handler_register_upper_ddrfreq);

int tsmng_handler_register_lower_ddrfreq(u32 dev_id, tsmng_handler handler)
{
    return tsmng_ipc_handler_register(dev_id, IPCDRV_TS_LOWER_DDR_FREQ, handler);
}
EXPORT_SYMBOL(tsmng_handler_register_lower_ddrfreq);

int tsmng_handler_register_ts_heartbeat_toaicpu(u32 dev_id, tsmng_handler handler)
{
    return tsmng_ipc_handler_register(dev_id, IPCDRV_TS_HEARTBEAT_TOAICPU, handler);
}
EXPORT_SYMBOL(tsmng_handler_register_ts_heartbeat_toaicpu);

void tsmng_ipc_handler_unregister(u32 dev_id, u32 cmd_type)
{
    u32 max_cmd_type;

#ifndef TSDRV_UT
    if (icm_get_msg_version(dev_id, HISI_RPROC_MAX) == ICM_IPC_MSG_NEW_VER) {
        max_cmd_type = ICM_MAIN_CMD_MAX;
    } else {
        max_cmd_type = IPCDRV_TS_MAX;
    }
#endif

    if (dev_id >= MAX_CHIP_NUM) {
        tsmng_drv_err("Dev id is invalid. (dev_id=%u; max_chip_num=%d)\n", dev_id, MAX_CHIP_NUM);
        return;
    }

#ifndef TSDRV_UT
    if (cmd_type >= max_cmd_type) {
        tsmng_drv_err("Cmd type is invalid. (cmd_type=%u)\n", cmd_type);
        return;
    }
#endif

    tsmng_ipc_rx_handlers[dev_id][cmd_type] = NULL;
    return;
}

void tsmng_handler_unregister_ddrfreq(u32 dev_id)
{
    tsmng_ipc_handler_unregister(dev_id, IPCDRV_TS_UPPER_DDR_FREQ);
    tsmng_ipc_handler_unregister(dev_id, IPCDRV_TS_LOWER_DDR_FREQ);
    return;
}
EXPORT_SYMBOL(tsmng_handler_unregister_ddrfreq);

int tsmng_ipc_chan_proc(u32 cmd_type, void *data, u32 dev_id)
{
    u32 max_cmd_type;

#ifndef TSDRV_UT
    tsmng_drv_info("Ipc notifier process. (dev_id=%u; cmd_type=%u)\n", dev_id, cmd_type);
    if (icm_get_msg_version(dev_id, HISI_RPROC_MAX) == ICM_IPC_MSG_NEW_VER) {
        max_cmd_type = ICM_MAIN_CMD_MAX;
    } else {
        max_cmd_type = IPCDRV_TS_MAX;
    }
#endif

    if (cmd_type >= max_cmd_type) {
        tsmng_drv_err("Cmd type is invalid. (cmd=%u; max=%u)\n", cmd_type, max_cmd_type - 1);
        return NOTIFY_DONE;
    }

    if (tsmng_ipc_rx_handlers[dev_id][cmd_type] == NULL) {
        tsmng_drv_debug("Ipc notifier handler does not register. (dev_id=%u; cmd_type=%u)\n", dev_id, cmd_type);
        return NOTIFY_DONE;
    }

#ifndef TSDRV_UT
    tsmng_drv_debug("Ipc notifier process. (dev_id=%u; cmd_type=%u)\n", dev_id, cmd_type);
#endif
    (void)tsmng_ipc_rx_handlers[dev_id][cmd_type](data, dev_id);
    return NOTIFY_DONE;
}

int tsmng_chk_ipc_crc16(struct ipcdrv_message *ipc_msg)
{
#ifdef CFG_FEATURE_IPC_CRC
    struct ipcdrv_msg_payload *payload = NULL;
    u16 crc;

    if (ipc_msg == NULL) {
        tsmng_drv_err("Invalid input null.\n");
        return -EINVAL;
    }

    payload = (struct ipcdrv_msg_payload *)ipc_msg->ipcdrv_payload;
    crc = icm_crc16((u8 *)ipc_msg, sizeof(struct ipcdrv_msg_header) + ipc_msg->ipc_msg_header.msg_length);
    return (crc == payload->crc) ? 0 : -EBADMSG;
#else
    return 0;
#endif
}
EXPORT_SYMBOL(tsmng_chk_ipc_crc16);

#define TSMNG_LOCAL_NOTIFIER "tsmng"
int __init tsmng_init(void)
{
    int ret = 0;
#ifdef CFG_FEATURE_TS_ABNORMAL_INFO
    struct uda_dev_type type;
#endif

    tsmng_drv_info("Tsmng init start.\n");
#ifdef AOS_LLVM_BUILD
    tsmng_feature_init();
    tsmng_mailbox_rx_register(0);
#endif

#ifdef CFG_FEATURE_TS_ABNORMAL_INFO
    tsmng_abnormal_handle_list_init();
    uda_davinci_local_real_entity_type_pack(&type);
    ret = uda_notifier_register(TSMNG_LOCAL_NOTIFIER, &type, UDA_PRI3, tsmng_local_notifier_func);
    if (ret != 0) {
        tsmng_drv_err("Register notifier failed. (ret=%d)\n", ret);
    }
#endif

    return ret;
}

void __exit tsmng_exit(void)
{
#ifdef CFG_FEATURE_TS_ABNORMAL_INFO
    struct uda_dev_type type;

    uda_davinci_local_real_entity_type_pack(&type);
    (void)uda_notifier_unregister(TSMNG_LOCAL_NOTIFIER, &type);
    tsmng_abnormal_handle_list_uninit();
#endif

#ifdef AOS_LLVM_BUILD
    tsmng_feature_exit();
    tsmng_mailbox_rx_unregister(0);
#endif
    tsmng_drv_info("Tsmng exit.\n");
}

module_init(tsmng_init);
module_exit(tsmng_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("ts ipc management driver");
