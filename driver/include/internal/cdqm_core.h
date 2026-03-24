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
* Create: 2022-7-9
*/

#ifndef CDQM_MODULE_H
#define CDQM_MODULE_H

#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/interrupt.h>
#include <linux/hashtable.h>

#include "cdqm_ioctl.h"
#include "trs_cdqm.h"

#define CDQ_VALID 1
#define CDQ_INVALID 0
#define MAX_QUEUE_NUM   128
#define CDQE_SIZE   16

#define INT_CDQM_F2NF_NS_CTRL   11

#define MAX_CDQ_DEPTH (64 * 1024)  /* max_cdq depth :64k */
#define INVALID_CDQID   0xFFFF

#define MAX_CDQ_SLICE 16 /* for host interrupters */

#ifdef CFG_SOC_PLATFORM_CLOUD_V2
#define SLICE_CDQ_NUM 8
#else
#define SLICE_CDQ_NUM 1
#endif

#define CDQ_CREATE_BY_DEVICE 1
#define CDQ_CREATE_BY_HOST 2

#define ENABLE_TIMEOUT_ERR 1
#define CDQM_ENABLE 1
#define CDQM_DISABLE 0

#define CDQM_STREAM_ID 0x1F

#define CDQ_S(qid)  ((qid) / SLICE_CDQ_NUM)
#define CDQ_PS(qid) ((qid) % SLICE_CDQ_NUM)

struct cdq_cfg_info {
    u32 devid;
    u32 tsid;
    int pid;
    u32 cdq_id;
    u32 batch_num;
    u32 batch_size;
    char name[CDQ_NAME_LEN];
    u64 mem_addr;
    u32 sid;
    u32 ssid;
};

struct cdqm_cdq_manage {
    struct cdq_cfg_info config_info;
    u8 valid;
    u8 create_side;   // 1 for host, 0 for device
    u8 time_out_flag;
    u32 last_get_batch;
    u32 use_batch_head;
    u32 use_batch_tail;
    u32 recycle_pid;
    struct mutex cdq_mutex;
    wait_queue_head_t wq;
};

#define CTX_HASH_TABLE_BIT 10
struct cdqm_dev_manage {
    u32 devid;
    u32 tsid;
    void __iomem *io_base;
    void __iomem *int_io_base;
    u32 cdq_num;
    u32 f2nf_hwirq;
    u32 f2nf_irq;
    u64 reg_mem_pa;
    void *rsv_mem_va;
    struct mutex dev_mutex;
    struct tasklet_struct f2nf_task;
    struct cdqm_cdq_manage cdq_manage[MAX_QUEUE_NUM];
    DECLARE_HASHTABLE(proc_hash_table, CTX_HASH_TABLE_BIT);

    struct cdqm_adapt_ops ops;
};

struct cdqm_destroy_msg {
    u32 cdq_id;
    int pid;
};
struct cdqm_batch_abnormal_msg {
    u32 cdq_id;
    u32 devid;
};
struct tsdrv_cdqm_init_msg {
    u32 f2nf_irq;
};

int tsdrv_cdqm_get_f2nf_irq(u32 devid, u32 *vector_id, u32 *irq);
int cdqm_dev_adapt_init(struct cdqm_dev_manage *cdq_dev);

int tsdrv_cdqm_create_cdq_id(struct cdqm_dev_manage *cdq_dev, struct cdq_cfg_info *cdq_info);
void tsdrv_cdqm_uninit_cdq_hardware(struct cdqm_dev_manage *cdq_dev, u32 cdq_id);

int tsdrv_cdqm_get_local_side(void);

void cdqm_clear_cdq_mng(struct cdqm_cdq_manage *cdq_manage);
int tsdrv_cdqm_sync_cdq_info(struct cdqm_dev_manage *cdq_dev, struct cdq_cfg_info *cdq_info);

int tsdrv_cdqm_create_msg_handle_by_side(struct cdqm_dev_manage *cdq_dev, struct cdq_cfg_info *cdq_info);
int tsdrv_cdqm_destroy_handle_by_side(u32 cdq_id, struct cdqm_dev_manage *cdq_dev);
int tsdrv_cdqm_uninit_cdq_by_side(struct cdqm_dev_manage *cdq_dev, u32 cdq_id);
int cdqm_ioctl_comm(u32 devid, int pid, struct cdqm_ioctl_arg *arg);
struct cdqm_dev_manage *cdqm_get_cdq_dev(u32 devid, u32 tsid);
int cdqm_recycle_cdq(u32 devid, u32 tsid, int pid);
void f2nf_tasklet_init(struct cdqm_dev_manage *cdq_dev);

int cdqm_sync_msg_init(struct cdqm_dev_manage *cdq_dev);
int cdqm_sync_msg_create(u32 devid, u32 tsid, struct cdq_cfg_info *para);
int cdqm_sync_msg_destroy(u32 devid, u32 tsid, struct cdqm_destroy_msg *para);

#endif

