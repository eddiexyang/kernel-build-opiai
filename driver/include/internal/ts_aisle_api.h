/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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
#ifndef TS_AISLE_API_H
#define TS_AISLE_API_H

#include <linux/types.h>
#include <linux/platform_device.h>

#define TS_IPC_MSG_MAX_LEN  24      /* 4 bytes as unit */
#define MSG_TYPE_B0         0x1E
#define MSG_TYPE_B1         0x71
#define MSG_TYPE_B2         0
#define MSG_TYPE_B3         0
#define MSG_HEAD_LEN        4       /* 1 byte as unit */

#define IPCDRV_TS_TO_AICPU_ENGIN 14

#define TS_AICPU_SQE_HEAD_LEN          8        /* size of aicpu sqe header */
#define HWTS_RECORD_TABLE_LEN          0x08
#define HWTS_RECORD_ID_MAX             1024     /* maybe notifyid or eventid */
#define HWTS_RECORD_WAIT_FLAG          0x01ULL
#define HWTS_NOTIFY_TABLE_0_REG        0x94000
#define HWTS_EVENT_TABLE_0_REG         0x90000

/*
 * The physical address of the HWTS has been mapped to the virtual address by ioremap.
 * The virtual address is obtained directly.
 */
void __iomem *get_hwts_base_va(u32 node_id, u32 ts_id);
int send_msg_to_ts_async(int dev_id, int ts_id, unsigned int len, const void *msg);
int aisle_init(struct platform_device *pdev, int node_id);
void aisle_uninit(int node_id);
int ts_write_record_register(u32 dev_id, u32 ts_id, u32 record_type, u32 record_id);

#endif
