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
 * Create: 2022-08-13
 */

#ifndef __TSMNG_PM_H__
#define __TSMNG_PM_H__


#define GET_DATA_WAIT_TIMEOUT 3000

/*
 * ipc message
 */
#ifdef CFG_FEATURE_IPC_CRC
#define IPC_MSG_RESERVE_LENGTH (IPCDRV_MSG_LENGTH - 4)
#else
#define IPC_MSG_RESERVE_LENGTH (IPCDRV_MSG_LENGTH - 2)
#endif

#define DEVDRV_WAKELOCK_TIMEOUT_SECOND  10 /* second */
#define DEVDRV_COMPUTE_POWER_LENGTH     72
#ifdef CFG_FEATURE_IPC_CRC
#define DEVDRV_IPC_PER_FRAM           (28 - TS_IPC_MSG_CRC_LEN)
#else
#define DEVDRV_IPC_PER_FRAM           28
#endif
#define DEVDRV_COMPUTE_POWER_LAST_LEN  ((DEVDRV_COMPUTE_POWER_LENGTH) % (DEVDRV_IPC_PER_FRAM))

#define AICORE_CYCLE_INDEX 56
#define TS_AICORE_RET_INDEX 0
#define TIME_STAMP1_INDEX 8
#define TIME_STAMP2_INDEX 16
#define SYSTEM_FLAG_INDEX 64

#define AICORE_COMPUTE_SIZE 56


struct computing_power_arg {
    unsigned int dev_id;
    struct tag_computing_power_msg compute_power_msg;
};

struct ipcdrv_msg_payload {
    u8 result;
    u8 vfid;
    u8 reserved[IPC_MSG_RESERVE_LENGTH];
#ifdef CFG_FEATURE_IPC_CRC
    u16 crc;
#endif
};

struct devdrv_mailbox_info {
    u32 dev_id;
    u32 m_id;
    wait_queue_head_t mailbox_wait_queque;
    u32 wait_time;
    u32 wait_timeout_count;
    atomic_t wait_flag;
};


int tsmng_inform_ts_idle(void *data, u32 dev_id, u32 tsid);
void devdrv_get_computing_power(void *data, u32 dev_id);
void tsmng_ipc_manager_init(u32 dev_id);
void tsmng_ipc_manager_exit(u32 dev_id);
#endif  /* __TSMNG_PM_H__ */
