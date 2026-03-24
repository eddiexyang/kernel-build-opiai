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

#ifndef TSDRV_CDQM_MODULE_H
#define TSDRV_CDQM_MODULE_H
#ifdef CFG_FEATURE_CDQM

#include "tsdrv_ioctl.h"
#include "tsdrv_cdqm_module.h"
#include "tsdrv_ctx.h"
#include "devdrv_common.h"

#define CDQ_VALID 1
#define CDQ_INVALID 0
#define MAX_QUEUE_NUM   128
#define CDQE_SIZE   16

#define INT_CDQM_F2NF_NS_CTRL   11

#define MAX_CDQ_DEPTH (64 * 1024)  /* max_cdq depth :64k */
#define INVALID_CDQID   0xFFFF

#define MAX_CDQ_GRP 4
#define GRP_CDQ_NUM 32
#define MAX_CDQ_SLICE 16
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
    u32 ts_id;
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

struct cdqm_dev_manage {
    u32 devid;
    u32 tsid;
    void __iomem *io_base;
    void __iomem *int_io_base;
    u32 cdq_num;
    u32 f2nf_vector_id;
    u32 f2nf_irq;
    u64 reg_mem_pa;
    void *rsv_mem_va;
    struct mutex dev_mutex;
    struct tasklet_struct f2nf_task;
    struct cdqm_cdq_manage cdq_manage[MAX_QUEUE_NUM];
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

int tsdrv_cdqm_get_cdqm_reg(u32 devid, u64 *addr, size_t *size);
int tsdrv_cdqm_get_intr_reg(u32 devid, u64 *addr, size_t *size);
int tsdrv_cdqm_get_f2nf_irq(u32 devid, u32 *vector_id, u32 *irq);
int tsdrv_cdqm_drv_manage_adapt_init(struct cdqm_dev_manage *cdq_dev);
int tsdrv_cdqm_drv_init(u32 devid, u32 tsnum);
void tsdrv_cdqm_drv_uninit(u32 devid, u32 tsnum);

int tsdrv_cdqm_create_cdq_id(struct cdqm_dev_manage *cdq_dev, struct cdq_cfg_info *cdq_info, u32 *cdq_id);
void tsdrv_cdqm_uninit_cdq_hardware(struct cdqm_dev_manage *cdq_dev, u32 cdq_id);

int tsdrv_cdqm_get_local_side(void);
int tsdrv_sync_cdqm_init_proc(u32 devid, struct tsdrv_msg_info *msg_head, u32 *ack_len);
int tsdrv_sync_cdqm_create_proc(u32 devid, struct tsdrv_msg_info *msg_head, u32 *ack_len);
int tsdrv_sync_cdqm_destroy_proc(u32 devid, struct tsdrv_msg_info *msg_head, u32 *ack_len);
int tsdrv_sync_cdqm_batch_abnormal_proc(u32 devid, struct tsdrv_msg_info *msg_head, u32 *ack_len);
int tsdrv_ioctl_cdqm_handlers(struct tsdrv_ctx *ctx, struct devdrv_ioctl_arg *arg);
void cdqm_init_cdq_info(struct cdqm_cdq_manage *cdq_manage, struct cdq_cfg_info *create_msg);
void cdqm_clear_cdq_info(struct cdqm_cdq_manage *cdq_manage);

int tsdrv_cdqm_sync_cdq_info(struct cdqm_dev_manage *cdq_dev, struct cdq_cfg_info *create_msg, u32 cdq_id);

int tsdrv_cdqm_drv_manage_init(u32 devid, u32 ts_id, struct cdqm_dev_manage *dev_manage, int *irq_num);
int tsdrv_cdqm_create_msg_handle_by_side(struct cdqm_dev_manage *cdq_dev, struct cdq_cfg_info *cdq_info);

int tsdrv_cdqm_destroy_handle_by_side(u32 devid, u32 tsid, u32 cdq_id, struct cdqm_dev_manage *cdq_dev);
int tsdrv_cdqm_uninit_cdq_by_side(u32 devid, u32 tsid, u32 cdq_id, struct cdqm_dev_manage *cdq_dev);

int tsdrv_cdqm_recycle_cdq(u32 devid, u32 tsid, int pid);

/* Query name by cdq_id */
int tsdrv_cdqm_get_name_by_cdqid(u32 devid, u32 tsid, u32 cdq_id, char *name, int buf_len);

#endif /* CFG_FEATURE_CDQM */

#endif

