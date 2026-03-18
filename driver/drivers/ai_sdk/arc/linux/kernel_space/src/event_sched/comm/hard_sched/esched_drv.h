/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
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

#ifndef ESCHED_DRV_H
#define ESCHED_DRV_H

#include <linux/spinlock.h>
#include <linux/sched.h>

#include "devdrv_interface.h"
#include "trs_chan.h"

#define SCHED_DRV_REPORT_NONE 0
#define SCHED_DRV_REPORT_ACK 1
#define SCHED_DRV_REPORT_GET_EVENT 2

enum esched_topic_type {
    TOPIC_TYPE_AICPU_DEVICE_ONLY = 0,
    TOPIC_TYPE_AICPU_DEVICE_FIRST = 1,
    TOPIC_TYPE_AICPU_HOST_ONLY = 2,
    TOPIC_TYPE_AICPU_HOST_FIRST = 3,
    TOPIC_TYPE_CCPU_HOST = 4,
    TOPIC_TYPE_DCPU_DEVICE = 5,
    TOPIC_TYPE_CCPU_DEVICE = 6,
    TOPIC_TYPE_TSCPU = 7,
    TOPIC_TYPE_DVPPCPU = 8,
    TOPIC_TYPE_MAX
};

#define TOPIC_FINISH_REPORT_ABNORMAL 1

#define HOST_SIDE_SET_PID 0
#define DEVICE_SIDE_SET_PID 1

#define HOST_STD_PROC 0
#define HOST_USER_PROC 1
#define DEVICE_STD_PROC 2
#define DEVICE_USER_PROC 3
#define PID_TYPE_MAX 4

#define TOPIC_FINISH_STATUS_NORMAL    0x1
#define TOPIC_FINISH_STATUS_DEBUG     0x2
#define TOPIC_FINISH_STATUS_EXCEPTION 0x4
#define TOPIC_FINISH_STATUS_TRAP      0x8
#define TOPIC_FINISH_STATUS_WARNING   0x10

#define STATUS_REPORT_VAL(status, error_code) (((error_code) << 4) | (status))

/* For details about the maximum specifications, see tsdrv_kernel_common.h
 * (DEVDER_MAX_SQ_DEPTH or DEVDER_MAX_CQ_DEPTH) */
#define TOPIC_SCHED_TASK_SUBMIT_SQ_DEPTH (2 * 1024)
#define TOPIC_SCHED_TASK_SUBMIT_CQ_DEPTH (1024)
#define TOPIC_SCHED_TASK_SQE_SIZE (64)
#define TOPIC_SCHED_TASK_CQE_SIZE (16)

#define TOPIC_SCHED_SQE_TYPE 1

/* 0xFF means task will not timeout and STARS don't report timeout interrupt to TSFW */
#define TOPIC_SCHED_SQE_KERNEL_CREDIT 100

#define TOPIC_SCHED_USER_DATA_LEN 10
#define TOPIC_SCHED_TS_PID_INDEX 6
#define TOPIC_SCHED_TS_CALLBACK_PID_INDEX 8
#define TOPIC_SCHED_ESCHED_PID_HIGH_OFFSET 16
#define TOPIC_SCHED_ESCHED_COMM_PID 0
#define TOPIC_SCHED_USER_DATA_PAYLOAD_LEN 40
/* custom process kernel type config */
#define TOPIC_SCHED_CUSTOM_KERNEL_TYPE     4
#define TOPIC_SCHED_DEFAULT_KERNEL_TYPE    127

#define TOPIC_SCHED_MAX_PRIORITY      8U

#ifndef ESCHED_HOST
/* Reserve one for the softirq context. */
#define TOPIC_SCHED_RTSQ_CLASS_NUM    (TOPIC_SCHED_MAX_PRIORITY + 1U)

#define TOPIC_SCHED_RTSQ_NUM_FOR_IRQ  1
#define TOPIC_SCHED_RTSQ_FOR_IRQ      (TOPIC_SCHED_RTSQ_CLASS_NUM - 1U)
#else
#define TOPIC_SCHED_RTSQ_CLASS_NUM    TOPIC_SCHED_MAX_PRIORITY
#endif
#define TOPIC_SCHED_MAX_RTSQ_NUM_PER_CLASS   8U

/* The RTSQ priority ranges from 0 to 7, 0 is the highest priority. */
#define TOPIC_SCHED_RTSQ_PRI          4

#define TOPIC_SCHED_MB_STATUS_IDLE 0
#define TOPIC_SCHED_MB_STATUS_BUSY 1

#define TOPIC_SCHED_MAX_SUBEVENT_ID (1UL << 12) /* same as struct topic_sched_sqe */

#define TOPIC_SCHED_HOST_POOL_ID 0

/* CLOUDV2 host chan num: AICPU 64, CCPU 16, total 80
   CLOUDV2 device chan num: AICPU 8, CCPU 1, total 9
   MINIV3 host chan num:  AICPU 0, CCPU 1, total 1
   MINIV3 device chan num:  AICPU 4, CCPU 1, total 5 */
#define TOPIC_SCHED_MAX_CHAN_NUM 80
#define TOPIC_SCHED_HOST_AICPU_CHAN_NUM 64

struct topic_sched_mailbox {
    u8  mailbox_id; /* only used for: host is 1630,
                                      and STARS works with EMS to schedule tasks Host CPU */
    u32 vfid : 6;
    u16 rsp_mode : 1;
    u16 sat_mode : 1;
    u16 blk_dim;
    /********4 bytes**********/

    u16 stream_id;
    u16 task_id;
    /********8 bytes**********/

    u16 blk_id;
    u16 kernel_type : 7;
    u16 batch_mode : 1;
    u32 topic_type : 4;
    u32 qos : 3;
    u16 sqe_dest_pid_vid : 1;
    /********12 bytes**********/

    u32 pid;
    /********16 bytes**********/

    /* user_data format: "user msg" + 2bit devid(if devid_flag == 1) + 2bit tid(if tid_flag == 1) */
    u32 user_data[TOPIC_SCHED_USER_DATA_LEN];
    /********56 bytes**********/

    u32 subtopic_id : 12;
    u32 topic_id : 6;
    u32 gid : 6;
    /* user_data_len: chip define 8 bits, not care value just transparently transmitted from sqe to mb,
       40 byte only need 6 bits, so we use 2 bits indicate that the user_data has devid and tid */
    u8 user_data_len : 6;
    u8 devid_flag : 1;
    u8 tid_flag : 1;
    /********60 bytes**********/

    u16 hac_sn : 5;
    u16 rsv1 : 11;
    u16 tq_id; /* report to host cpu in msg que mode, ai cpu not use */
    /********64 bytes**********/
};

/* Used by a single thread of the split operator. The number of CPUs must be greater
   than or equal to twice the number of CPUs. */
#define TOPIC_SCHED_CPU_PORT_DEPTH 32
struct topic_sched_cpu_port {
    spinlock_t lock; /* Callers may submit tasks in software interrupts.  */
    u32 port_id;
    u32 status;
    void *sq_base;
    u16 tail;
    u16 depth;
};

struct sqe_submit_chan_res {
    bool need_destroy;
    int chan_id;
};

struct rtsq_sched_res {
    struct sqe_submit_chan_res sqe_submit[TOPIC_SCHED_MAX_RTSQ_NUM_PER_CLASS];
    u32 rtsq_num;
    u32 init_rtsq_index;
    atomic_t cur_rtsq_index;
};

struct rtsq_non_sched_res {
    int chan_id;
};

struct sched_rtsq_res {
    struct rtsq_sched_res sched_rtsq[TOPIC_SCHED_RTSQ_CLASS_NUM];
    struct rtsq_non_sched_res non_sched_rtsq;
};

struct topic_data_chan {
    struct sched_hard_res *hard_res;
    struct topic_sched_mailbox *wait_mb;
    struct topic_sched_mailbox *get_mb;
    struct topic_sched_cpu_port *cpu_port;
    struct tasklet_struct sched_task;
    int valid;
    u32 mb_id;
    u32 mb_type;
    int wait_mb_status;
    int irq;
    u32 report_flag;
    u64 serial_no;
    struct sched_cpu_ctx *cpu_ctx;
    struct sched_event *event;
};

struct sched_hard_res {
    struct mutex mutex;
    void __iomem *io_base;
    void __iomem *int_io_base;
    struct sched_rtsq_res rtsq;
    u32 dev_id;
    int irq_base;
    int irq_reg_flag;
    u64 rsv_mem_pa;
    void *rsv_mem_va;
    struct delayed_work init;
    u32 init_flag;
    u32 retry_times;
    u32 sub_dev_num;  /* Number of subdevices created based on the device, like vf dev */
    struct topic_data_chan *topic_chan[TOPIC_SCHED_MAX_CHAN_NUM];
    void *priv;       /* for host */
    u32 topic_sched_chan_num; /* The number of topic channels' resource be created for SCHED. */
    u32 topic_sched_chan_start_id; /* The start id of topic channels' resource be created for SCHED. */
    u32 aicpu_chan_num;
    u32 aicpu_chan_start_id;
    u32 ccpu_chan_id;
};

struct topic_sched_rts_task_info {
    u64 task_so_name_ptr; /* kernelSo */
    u64 para_ptr; /* paramBase */
    u64 task_name_ptr; /* kernelName */
    u64 l2_struct_ptr; /* l2Ctrl */
    u64 extra_field_ptr;
};

struct topic_sched_sqe {
    u16 type : 6; /* must set 1, topic sched sqe */
    u16 res0 : 2;
    u16 ie : 2;
    u16 pre_p : 2;
    u16 post_p : 2;
    u16 wr_cqe : 1;
    u16 rd_cond : 1;
    u16 blk_dim;
    /********4 bytes**********/

    u16 rt_streamid;
    u16 task_id;
    /********8 bytes**********/

    u16 block_id;
    u16 kernel_type : 7;
    u16 batch_mode : 1;
    u16 topic_type : 4;
    u16 qos : 3;
    u16 resv : 1;
    /********12 bytes**********/

    u16 sqe_index;
    u8  kernel_credit;
    u8  res1;
    /********16 bytes**********/

    /* user_data format: "user msg" + 2bit devid(if devid_flag == 1) + 2bit tid(if tid_flag == 1) */
    u32 user_data[TOPIC_SCHED_USER_DATA_LEN];
    /********56 bytes**********/

    u32 subtopic_id : 12;
    u32 topic_id : 6;
    u32 gid : 6;
    /* user_data_len: chip define 8 bits, not care value just transparently transmitted from sqe to mb,
       40 byte only need 6 bits, so we use 2 bits indicate that the user_data has devid and tid */
    u8 user_data_len : 6;
    u8 devid_flag : 1;
    u8 tid_flag : 1;
    /********60 bytes**********/

    u32 pid;
    /********64 bytes**********/
};

#define TOPIC_SCHED_MB_SIZE sizeof(struct topic_sched_mailbox)
#define TOPIC_SCHED_SQE_SIZE sizeof(struct topic_sched_sqe)

struct sched_hard_res *esched_get_hard_res(u32 chip_id);
struct topic_data_chan *esched_drv_get_topic_chan(u32 dev_id, u32 chan_id);
struct topic_data_chan *esched_drv_create_one_topic_chan(u32 devid, u32 chan_id);
void esched_drv_destroy_one_topic_chan(u32 devid, u32 chan_id);
void esched_drv_destroy_topic_chans(u32 devid, u32 start_chan_id, u32 chan_num);
int esched_drv_create_topic_chans(u32 devid, u32 start_chan_id, u32 chan_num);
int esched_publish_event_to_topic(u32 chip_id, u32 event_src,
    struct sched_published_event_info *event_info,
    struct sched_published_event_func *event_func);

void esched_init_topic_types(void);
void esched_ccpu_sched_task(unsigned long data);
void esched_aicpu_sched_task(unsigned long data);
void esched_drv_mb_intr_enable(struct topic_data_chan *topic_chan);
int esched_drv_init_non_sched_task_submit_chan(u32 chip_id, u32 pool_id);
void esched_drv_uninit_non_sched_task_submit_chan(u32 chip_id);
int esched_drv_init_sched_task_submit_chan(u32 chip_id, u32 pool_id, u32 available_chan_num, u32 aicpu_chan_num);
void esched_drv_uninit_sched_task_submit_chan(u32 chip_id);
int esched_drv_abnormal_task_handle(struct trs_id_inst *inst, u32 sqid, void *sqe, void *info);
#endif
