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

#ifndef __DEVDRV_MANAGER_MSG__HOST_H
#define __DEVDRV_MANAGER_MSG__HOST_H

#include <linux/slab.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>
#include <linux/bitops.h>

#include "devdrv_common.h"
#include "drv_ipc.h"

#ifdef CFG_SOC_PLATFORM_MDC_V51
#include <linux/kfifo.h>
#include "devdrv_ipc.h"
#include "drvfault_user_common.h"

#define SILS_IPC_STATUS_WAIT 0x1
#define SILS_IPC_STATUS_MODIFIED 0x2
#define SILS_IPC_CHANNEL_MAX 2
#define SILS_IPC_CHANNEL_MASTER 0
#define SILS_IPC_CHANNEL_SLAVE 1
#define SILS_IPC_MSG_DATA_LEN   22
#define SILS_IPC_MSG_ONE_SIZE (sizeof(struct IpcMessage))
#define SILS_IPC_MSG_KFIFO_LEN 128
#define SILS_IPC_MSG_KFIFO_SIZE (SILS_IPC_MSG_KFIFO_LEN * SILS_IPC_MSG_ONE_SIZE)
struct devIpcMessage {
    struct IpcMessage ipcMsg;
    unsigned char moduleId;
    unsigned char channelId;
    unsigned int devId;
};
struct safetyisland_ipc_recv_msg {
    u64 in_kfifo;
    u64 out_kfifo;
    u64 lost_kfifo;
    struct kfifo kfifo;
    spinlock_t kfifo_lock;
    atomic_t status;
    wait_queue_head_t wait_queue;
};
void devdrv_safetyisland_ipc_init(u32 dev_id);
void devdrv_safetyisland_ipc_exit(u32 dev_id);
void devdrv_safetyisland_ipc_bh(u32 dev_id, u32 m_id, void *data);
int devdrv_safetyisland_ipc_send(struct devIpcMessage *msg);
int devdrv_safetyisland_ipc_recv(struct devIpcMessage *msg);
#endif

#define SILS_IPC_HEAD_LEN 8
#define SILS_CRC_POLYNOMIAL     0x1021
#define NULL_USHORT             0xFFFF
#define BIT15                   0x8000
static inline uint16_t sils_crc16(uint8_t *data, uint16_t len)
{
    uint16_t val = NULL_USHORT;
    const uint16_t poly = SILS_CRC_POLYNOMIAL;
    uint8_t ch;
    unsigned int i;

    while (len--) {
        ch = *(data++);
        val ^= (ch << BITS_PER_BYTE);
        for (i = 0; i < BITS_PER_BYTE; i++) {
            if (val & BIT15) {
                val = (val << 1) ^ poly;
            } else {
                val = val << 1;
            }
        }
    }
    return (val);
}
#endif
