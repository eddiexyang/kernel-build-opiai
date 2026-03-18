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
#ifndef TS_AISLE_QUEUE_H
#define TS_AISLE_QUEUE_H

#include <linux/semaphore.h>
#include "hwts_drv_config.h"

#define AICPU_TO_TS_MSG_MAILBOX_LENGTH 64

#define AICPU_TO_TS_CREATE_CHANNEL  22
#define AICPU_TO_TS_DESTORY_CHANNEL 23
#define AICPU_TO_TS_MAILBOX_VALID   0X5a5a
#define AICPU_TO_TS_MAILBOX_INVALID  0


#define AICPU_TO_TS_MSG_CHAN_MAX 16
#define AICPU_TO_TS_SRAM_Q_HEAD (2*sizeof(int))
#define AICPU_TO_TS_MSG_CHAN_QE_LENGTH 64
#define AICPU_TO_TS_MSG_CHAN_QE_HEAD_LENGTH 8
#define AICPU_TO_TS_MSG_CHAN_QE_DATA_LENGTH (AICPU_TO_TS_MSG_CHAN_QE_LENGTH - AICPU_TO_TS_MSG_CHAN_QE_HEAD_LENGTH)


#define AICPU_TO_TS_SRAM_ADDR_SQ 0xA0D50000
#define AICPU_TO_TS_SRAM_ADDR_SQ_LEN 0x800
#define AICPU_TO_TS_SRAM_ADDR_CQ 0xA0D50800
#define AICPU_TO_TS_SRAM_ADDR_CQ_LEN 0x400
#define AICPU_TO_TS_MSG_CHAN_SQ_DEPTH ((AICPU_TO_TS_SRAM_ADDR_SQ_LEN - AICPU_TO_TS_SRAM_Q_HEAD) /     \
        AICPU_TO_TS_MSG_CHAN_QE_LENGTH)
#define AICPU_TO_TS_MSG_CHAN_CQ_DEPTH ((AICPU_TO_TS_SRAM_ADDR_CQ_LEN - AICPU_TO_TS_SRAM_Q_HEAD) /     \
        AICPU_TO_TS_MSG_CHAN_QE_LENGTH)

typedef void (*queue_call_back)(u64 irq, int node_id);

typedef struct aicpu_to_ts_queue_ {
    volatile unsigned short head;
    volatile unsigned short tail;
    unsigned int   recv;
    unsigned char data[0];
}aicpu_to_ts_queue;


typedef struct aicpu_to_ts_msg_queue_cb_ {
    unsigned short queue_depth;
    unsigned short queue_entry_length;
    unsigned int   irq;
    queue_call_back  proc;
    struct semaphore queue_sema;
    void *pa;
    aicpu_to_ts_queue *queue;
} aicpu_to_ts_msg_queue_cb;


typedef struct aicpu_to_ts_msg_cb_ {
    unsigned short channel_id;
    struct tasklet_struct wakeup_task;
    struct platform_device *pdev;
    aicpu_to_ts_msg_queue_cb sq;
    aicpu_to_ts_msg_queue_cb cq;
    atomic64_t v_irq_tx; /* send irq count */
    atomic64_t v_irq_rx; /* recv irq count */
    atomic64_t v_sq_tx; /* send msg count */
    atomic64_t v_cq_rx; /* recv msg count */
}aicpu_to_ts_msg_cb;

typedef struct aicpu_to_ts_create_cmd_ {
    void *sq_addr;
    void *cq_addr;
    unsigned short channel_id;
    unsigned short sqe_length;
    unsigned short cqe_length;
    unsigned short sq_depth;
    unsigned short cq_depth;
}aicpu_to_ts_create_cmd;

struct devdrv_mailbox_create_ts_com {
    u16 valid;      /* validity judgement, 0x5a5a is valid */
    u16 cmd_type;   /* command type */
    u32 result;     /* TS's process result succ or fail: no error: 0, error: not 0 */

    aicpu_to_ts_create_cmd cmd;
};

struct devdrv_mailbox_destory_ts_com {
    u16 valid;      /* validity judgement, 0x5a5a is valid */
    u16 cmd_type;   /* command type */
    u32 result;     /* TS's process result succ or fail: no error: 0, error: not 0 */

    unsigned short channel_id;
};


typedef struct aicpu_to_ts_config_ {
    aicpu_to_ts_create_cmd cmd;
    int ts_to_aicpu_irq;
    int aicpu_to_ts_irq;
    queue_call_back  aicpu_to_ts_proc;
}aicpu_to_ts_config;


typedef struct aicpu_to_ts_destory_cmd_ {
    unsigned short channel_id;
}aicpu_to_ts_destory_cmd;

typedef struct aicpu_to_ts_task_info_ {
    unsigned int   pid;
    unsigned char   cmdtype;
    unsigned char   vfid;
    unsigned char   tid;
    unsigned char  tsid;
    unsigned char data[AICPU_TO_TS_MSG_CHAN_QE_DATA_LENGTH];
}aicpu_to_ts_task_info;


int tsdrv_queue_cb_init(int node_id, aicpu_to_ts_config conf);
void tsdrv_queue_cb_uninit(void);

extern aicpu_to_ts_msg_cb g_aicpu_to_ts_cb[CHIP_NUM_MAX];

#endif
