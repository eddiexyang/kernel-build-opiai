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
* Create: 2022-7-15
*/

#ifndef TRS_MAILBOX_DEF_H
#define TRS_MAILBOX_DEF_H

#include <linux/types.h>
#include "trs_pub_def.h"

#define TRS_MBOX_NOTICE_ACK_IRQ_VALUE       0U
#define TRS_MBOX_CREATE_CQSQ_CALC           1U // normal sqcq alloc
#define TRS_MBOX_RELEASE_CQSQ_CALC          2U // normal sqcq free
#define TRS_MBOX_LOG_CQSQ_CREATE            3U
#define TRS_MBOX_LOG_CQSQ_RELEASE           4U
#define TRS_MBOX_DBG_CQSQ_CREATE            5U
#define TRS_MBOX_DBG_CQSQ_RELEASE           6U
#define TRS_MBOX_CREATE_PROF_SQCQ           7U
#define TRS_MBOX_RELEASE_PROF_SQCQ          8U
#define TRS_MBOX_CREATE_HB_SQCQ             9U
#define TRS_MBOX_RELEASE_HB_SQCQ            10U
#define TRS_MBOX_SEND_RDMA_INFO             15U
#define TRS_MBOX_RESET_NOTIFY               16U
#define TRS_MBOX_RECYCLE_PID                18U
#define TRS_MBOX_CREATE_TASKSCHED_SQCQ      24U
#define TRS_MBOX_RELEASE_TASKSCHED_SQCQ     25U
#define TRS_MBOX_CREATE_CB_CQ               26U
#define TRS_MBOX_RELEASE_CB_CQ              27U
#define TRS_MBOX_CREATE_MIA                 30U
#define TRS_MBOX_DESTROY_MIA                31U
#define TRS_MBOX_SHM_SQCQ_ALLOC             33U
#define TRS_MBOX_SHM_SQCQ_FREE              34U
#define TRS_MBOX_LOGIC_CQ_ALLOC             35U
#define TRS_MBOX_LOGIC_CQ_FREE              36U
#define TRS_MBOX_CREATE_TOPIC_SQCQ          37U // esched alloc
#define TRS_MBOX_RELEASE_TOPIC_SQCQ         38U // esched alloc
#define TRS_MBOX_RES_MAP                    39U
#define TRS_MBOX_RECYCLE_CHECK              40U
#define TRS_MBOX_ALLOC_STREAM               41U
#define TRS_MBOX_FREE_STREAM                42U
#define TRS_MBOX_CREATE_KERNEL_SQCQ         43U // dvpp alloc
#define TRS_MBOX_RELEASE_KERNEL_SQCQ        44U // dvpp free
#define TRS_MBOX_NOTICE_SSID                45U // notice ssid to tsfw from device
#define TRS_MBOX_QUERY_SSID                 46U // notice ssid to tsfw from device
#define TRS_MBOX_NOTICE_TS_SQCQ_CREATE      47U
#define TRS_MBOX_NOTICE_TS_SQCQ_FREE        48U
#define TRS_MBOX_CREATE_CTRL_CQSQ           49U
#define TRS_MBOX_RELEASE_CTRL_CQSQ          50U
#define TRS_MBOX_NOTICE_SQ_TRIGGER          51U

#define TRS_MBOX_CMD_MAX                    56U

#define TRS_MBOX_INVALID_INDEX              0xFFFF
#define TRS_MBOX_MESSAGE_VALID              0x5A5A

#define TRS_MBOX_SEND_FROM_DEVICE           0
#define TRS_MBOX_SEND_FROM_HOST             1

#define TRS_DEVICE_CHAN_MBOX_TIMEOUT_MS    3000

struct trs_mb_header {
    u16 valid;    /* validity judgement, 0x5a5a is valid */
    u16 cmd_type; /* identify command or operation */
    u32 result;   /* TS's process result succ or fail: no error: 0, error: not 0 */
};

static inline void trs_mbox_init_header(struct trs_mb_header *header, u32 cmd_type)
{
    header->cmd_type = cmd_type;
    header->result = 0;
    header->valid = TRS_MBOX_MESSAGE_VALID;
}

#define SQCQ_INFO_LENGTH 5
struct trs_normal_cqsq_mailbox {
    struct trs_mb_header header;

    u64 sq_addr; /* invalid addr: 0x0 */
    u64 cq0_addr;

    u16 sq_index;  /* invalid idx: 0xFFFF */
    u16 cq0_index; /* sq's return */

    u8 app_type : 1;  /* inform TS, msg is sent from host or device, device: 0 host: 1 */
    u8 sw_reg_flag : 1; /* 1: sq saves head and tail in share memory at the last 2 sqe, 0: not save */
    u8 fid : 6;       /* 0:hsot, 1~16:virt machine */

    u8 sq_cq_side : 2;    /* bit 0 sq side, bit 1 cq side. device: 0 host: 1  */
    u8 master_pid_flag : 1;
    u8 rsv : 5;

    u8 sqesize;
    u8 cqesize;  /* calculation cq's slot size, default: 12 bytes */
    u16 cqdepth;
    u16 sqdepth;
    pid_t pid;
    u16 cq_irq;
    u16 ssid;

    u32 info[SQCQ_INFO_LENGTH];
};

struct trs_alloc_stream_mbox {
    struct trs_mb_header header;
    u32 priority;
    u32 stream_id;
    u32 vf_id;
    u32 pid;
};

struct trs_maint_sqcq_mbox {
    struct trs_mb_header header;

    u64 sq_addr; /* invalid addr: 0x0 */
    u64 cq0_addr;
    u64 cq1_addr;
    u64 cq2_addr;
    u64 cq3_addr;
    u16 sq_index;  /* invalid idx: 0xFFFF */
    u16 cq0_index; /* sq's return */
    u16 cq1_index; /* ts's return */
    u16 cq2_index; /* ai cpu's return */
    u16 cq3_index; /* reserved */
    u16 cq_irq;
    u8 plat_type;    /* inform TS, msg is sent from host or device, device: 0 host: 1 */
    u8 cq_slot_size; /* calculation cq's slot size, default: 12 bytes */
};

struct trs_task_sched_sqcq_alloc_mbox {
    struct trs_mb_header header;
    u64 sq_addr;
    u64 cq_addr;
    u32 sq_index;   /* sq idx */
    u32 cq_index;
    u16 sqe_size;
    u16 cqe_size;
    u16 sq_depth;   /* sq depth */
    u16 cq_depth;
    u8 plat_type;
    u8 reserved[3]; /* reserved */
    u32 cq_irq;
};

struct trs_task_sched_sqcq_free_mbox {
    struct trs_mb_header header;
    u32 sq_index;
    u32 cq_index;
    u8 plat_type;
    u8 reserved[3]; /* reserved */
};

struct trs_cb_cq_mbox {
    struct trs_mb_header header;
    u32 vpid;
    u32 grpid;
    u32 logic_cqid;
    u32 phy_cqid;
    u32 cq_irq;
    u32 phy_sqid;
    u8 plat_type;
    u8 reserved[3]; /* reserved */
};

struct trs_ts_sqcq_mbox {
    struct trs_mb_header header;
    u32 sqid;
    u32 cqid;
    u32 vfid;
    pid_t pid;
    u16 ssid;
    u16 rsv;
    u32 info[SQCQ_INFO_LENGTH];
};

struct trs_shm_sqcq_mbox {
    struct trs_mb_header header;

    u64 sq_addr;    /* invalid addr: 0x0 */
    u64 cq_addr;

    u16 sq_id;  /* invalid idx: 0xFFFF */
    u16 cq_id; /* sq's return */

    u8 app_type : 2;  /* inform TS, msg is sent from host or device, device: 0 host: 1 */
    u8 fid : 6;       /* 0:hsot, 1~16:virt machine */
    u8 sq_cq_side;    /* bit 0 sq side, bit 1 cq side. device: 0 host: 1  */

    u8 sqesize;
    u8 cqesize;  /* calculation cq's slot size, default: 12 bytes */
    u16 cqdepth;
    u16 sqdepth;
    pid_t pid;
    u32 cq_irq;
    u32 info[SQCQ_INFO_LENGTH];
};

struct trs_logic_cq_create_mbox {
    u64 phy_cq_addr;
    u16 cqe_size;
    u16 cq_depth;
    u32 vpid;
    u16 logic_cqid;
    u16 phy_cqid;
    u16 cq_irq;
    u8 app_flag;
    u8 thread_bind_irq_flag;
    u8 vfid;
    u8 rsv[3];
    u32 info[SQCQ_INFO_LENGTH];
};

struct trs_logic_cq_release_mbox {
    u32 vpid;
    u16 logic_cqid;
    u16 phy_cqid;
    u8 vfid;
    u8 rsv[3];
};

struct trs_logic_cq_mbox {
    struct trs_mb_header header;
    union {
        struct trs_logic_cq_create_mbox mb_alloc;
        struct trs_logic_cq_release_mbox mb_free;
    };
};

struct trs_event_msg {
    struct trs_mb_header header;
    u32 event_id;
};

struct trs_notify_msg {
    struct trs_mb_header header;
    u16 phy_notifyId;
    u16 plat_type;
    u32 tgid;
    u16 notifyId; /* ts use for covert id to notifyid */
    u8  fid;
    u8 notify_type; /* 0 notify id, 1 event id */
    u8 reserved[44];
};

struct trs_res_map_msg {
    struct trs_mb_header header;
    u8 vf_id;
    u8 resource_type;   // 0:notify id, 1:event id
    u8 operation_type;  // 0:map, 1:unmap
    u8 reserve0;
    u16 id;
    u16 phy_id;
    u32 host_pid;
    u8 reserve[44];
};

struct trs_reset_event_id_msg {
    struct trs_mb_header header;
    u32 event_id;
};

struct trs_mbox_ack_irq_msg {
    struct trs_mb_header header;
    u32 ack_irq;
};

struct trs_ssid_msg {
    struct trs_mb_header header;
    u8 vfid;
    u8 reserved;
    u16 ssid;
    u32 hostpid;
};

#define MAX_RDMA_INFO_LEN 56
struct trs_rdma_info {
    struct trs_mb_header header;
    u8 buf[MAX_RDMA_INFO_LEN];
};

/* inform   pid to ts recycle resource */
#define MAX_INFORM_PID_INFO 12
struct exit_proc_info {
    u32 app_cnt;
    int pid[MAX_INFORM_PID_INFO];
    u8 plat_type;
    u8 fid;
    u8 reserved[2];
};

struct recycle_proc_msg {
    struct trs_mb_header header;
    struct exit_proc_info proc_info;
};

struct trs_mia_cfg_msg {
    struct trs_mb_header header;
    u8 vfid;
    u8 rsv[3];
};

struct trs_sq_trigger_msg {
    struct trs_mb_header header;
    u32 db;
    u32 irq;
    u8 vfid;
    u8 rsv[3];
};

int trs_mbox_send(struct trs_id_inst *inst, u32 chan_id, void *data, size_t size, int timeout);

#endif

