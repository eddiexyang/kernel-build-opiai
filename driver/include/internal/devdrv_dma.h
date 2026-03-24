/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
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

#ifndef _DMA_COMMON_H_
#define _DMA_COMMON_H_

#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/spinlock.h>
#include <linux/semaphore.h>

#include "dma_adapt.h"
#include "dma_drv.h"
#include "devdrv_interface.h"

#define DMA_DONE_BUDGET 64

#define DEVDRV_DMA_MAX_REMOTE_IRQ 128

#define DEVDRV_DMA_SQ_LDIE_ENABEL 1

#define DEVDRV_MAX_TS_DMA_CH_SQ_DEPTH 4096
#define DEVDRV_MAX_TS_DMA_CH_CQ_DEPTH 4096

#ifdef CFG_SOC_PLATFORM_ESL_FPGA /* for fpga */
#define DEVDRV_DMA_COPY_TIMEOUT (HZ * 1000)
#define DEVDRV_DMA_WAIT_CHAN_AVAIL_TIMEOUT 20000
#else
/* Due to the impact of the previously unfinished dma task, the time-out period cannot
  be given based on the amount of data moved by the dma. Consider the influence of
  the PCIE bus bandwidth and the multi-channel of the DMA, giving a larger waiting time  */
#define DEVDRV_DMA_COPY_TIMEOUT (HZ * 10) /* 10s */
/* wait for dma chan SQ queue when full */
#define DEVDRV_DMA_WAIT_CHAN_AVAIL_TIMEOUT 10000
#endif

/* 10s */
#define DEVDRV_DMA_QUERY_MAX_WAIT_TIME 10000000

#define DEVDRV_SECOND_TO_MICROSECOND 1000000
#define DEVDRV_MICROSECOND_TO_NANOSECOND 1000

#define DEVDRV_DMA_SQCQ_SIDE_BIT 0U
#define DEVDRV_DMA_SML_PKT_BIT 1U

#define DEVDRV_DMA_SYNC 1
#define DEVDRV_DMA_ASYNC 2

#define DEVDRV_INVALID_INSTANCE (-1)

#define DEVDRV_DMA_SO_RELEX_ORDER 0x0
#define DEVDRV_DMA_RO_RELEX_ORDER 0x2

#define DEVDRV_SOFTBD_STATUS_INVALID (-2)
#define DEVDRV_DELAY_US 1000

/* device is alive or dead */
#define DEVDRV_DMA_ALIVE 0
#define DEVDRV_DMA_DEAD 1

/* dma chan is enabled or disabled */
#define DEVDRV_DMA_CHAN_ENABLED   0
#define DEVDRV_DMA_CHAN_DISABLED  1

/* DMA completion status */
enum {
    DEVDRV_DMA_SUCCESS = 0x0,
    DEVDRV_DMA_FAILED = 0x1
};

/* the side of the SQ and CQ of a DMA channel */
enum devdrv_dma_sqcq_side {
    DEVDRV_DMA_LOCAL_SIDE = 0x0,
    DEVDRV_DMA_REMOTE_SIDE = 0x1,
    DEVDRV_DMA_TS_SIDE = 0x2
};

/* dma process status */
enum devdrv_dma_process_status {
    DEVDRV_DMA_PROCESS_INIT = 0x0,
    DEVDRV_DMA_PROCESS_HANDLING = 0x1,
    DEVDRV_DMA_PROCESS_WAIT_TIMEOUT = 0x2
};

#define DEVDRV_DMA_SQ_DESC_SIZE sizeof(struct devdrv_dma_sq_node)
#define DEVDRV_DMA_CQ_DESC_SIZE sizeof(struct devdrv_dma_cq_node)

struct devdrv_dma_soft_bd {
    int valid;
    int copy_type;
    int wait_type;
    int owner_bd; /* The number of the last bd sent by the chain  */
    int status;
    u32 trans_id;
    struct semaphore sync_sem;
    void *priv;
    void (*callback_func)(void *, u32, u32);
    atomic_t process_flag;
};

struct devdrv_dma_soft_bd_wait_status {
    int status;
    int valid;
};

struct devdrv_sync_dma_stat {
    u64 dma_chan_copy_cnt;
    u64 sync_submit_cnt;
    u64 async_submit_cnt;
    u64 sml_submit_cnt;
    u64 trigger_remot_int_cnt;
    u64 trigger_local_128;
    u64 done_int_cnt;
    u64 done_int_in_time;
    u64 re_schedule_cnt;
    u64 done_tasklet_in_cnt;
    u64 done_tasklet_in_time;
    u64 done_tasklet_out_time;
    u64 err_int_cnt;
    u64 err_work_cnt;
    u64 sync_sem_up_cnt;
    u64 async_proc_cnt;
    u64 max_task_op_time;
    u64 last_soft_bd_proced;
    u64 sq_idle_bd_cnt;
};

#define DEVDRV_IRQ_IS_INIT    1
#define DEVDRV_IRQ_IS_UNINIT  0
struct devdrv_dma_channel {
    struct device *dev;
    void __iomem *io_base; /* the base address of DMA channel */
    u32 func_id;
    u32 chan_id; /* the actual index of DMA channel in DMA controller */
    u32 flag;    /* bit0: SQ and CQ side, remote or local;
                 bit1: DMA small packet is supported or not; */
    struct devdrv_dma_sq_node *sq_desc_base;
    struct devdrv_dma_cq_node *cq_desc_base;
    dma_addr_t sq_desc_dma;
    dma_addr_t cq_desc_dma;
    u32 sq_depth;
    u32 cq_depth;
    u32 sq_tail;
    u32 cq_head;
    u32 sq_head;

    struct devdrv_dma_soft_bd *dma_soft_bd;
    struct tasklet_struct dma_done_task;
    struct work_struct err_work;
    int done_irq;
    int done_irq_state;
    int err_irq;
    int err_irq_flag;
    int err_irq_state;
    spinlock_t lock;
    spinlock_t cq_lock;
    u32 rounds;
    u32 remote_irq_cnt; /* the count of remote interrupt */
    struct devdrv_sync_dma_stat status;
    struct devdrv_dma_dev *dma_dev;
    u32 last_irq_type;
    u32 chan_status;
};

struct devdrv_dma_chan_irq_info {
    u32 done_irq;
    u32 err_irq;
    int err_irq_flag;
};

struct data_type_chan {
    u32 chan_start_id;
    u32 chan_num;
    u32 last_use_chan;
};

struct devdrv_dma_ops {
    bool (*devdrv_dma_get_cq_valid)(struct devdrv_dma_cq_node *cq_desc, u32 rounds);
    void (*devdrv_dma_set_cq_invalid)(struct devdrv_dma_cq_node *cq_desc);
};

struct devdrv_dma_sq_cq_info {
    u32 sq_depth;
    u32 sq_rsv_num;
    u32 cq_depth;
};

struct devdrv_dma_dev {
    u32 dev_id;
    u32 func_id;
    struct device *dev;
    void __iomem *dma_chan_base;
    void __iomem *io_base;
    void *drvdata;
    u32 dma_pf_num;
    u32 dma_vf_en;
    u32 dma_vf_num;
    u32 sq_cq_side;
    u32 dev_status;
    u32 done_irq_base;
    u32 err_irq_base;
    u32 err_flag;
    struct devdrv_dma_sq_cq_info sq_cq_info;
    struct devdrv_dma_ops ops;
    unsigned long dma_bitmap;   /* 0-irrelevant 1-allocated */
    u32 local_chan_num;
    u32 local_chan[DEVDRV_DMA_CHAN_NUM];
    u32 remote_chan_begin;               /* first remote dma channel in device hardware */
    u32 remote_bar_begin;                /* first remote dma channel in bar space */
    u32 remote_chan_num;
    u32 remote_chan[DEVDRV_DMA_CHAN_NUM];
    u32 ts_chan_num;
    u32 ts_chan[DEVDRV_DMA_CHAN_NUM];
    struct data_type_chan data_chan[DEVDRV_DMA_DATA_TYPE_MAX];
    struct devdrv_dma_channel dma_chan[0];  /* host:remote channel, device: local channel */
};

struct devdrv_dma_func_para {
    u32 dev_id;
    u32 chip_id;
    u32 func_id;
    u32 dma_pf_num;
    u32 dma_vf_en;
    u32 dma_vf_num;
    struct device *dev;
    void __iomem *io_base;
    void __iomem *dma_chan_base;
    void *drvdata;
    u32 chan_num;
    u32 done_irq_base;
    u32 err_irq_base;
    u32 err_flag;
    u32 chip_type;
    struct devdrv_dma_sq_cq_info sq_cq_info;
};

void devdrv_dma_config_axim_aruser_mode(void __iomem *io_base);
struct devdrv_dma_dev *devdrv_dma_init(struct devdrv_dma_func_para *para_in, u32 sq_cq_side, u32 func_id);
void devdrv_dma_exit(struct devdrv_dma_dev *dma_dev, int sriov_flag);

/* these functions is both used in host and device */
struct devdrv_dma_dev *devdrv_get_dma_dev(u32 dev_id);
void devdrv_dfx_dma_report_to_bbox(struct devdrv_dma_channel *dma_chan, u32 queue_init_sts);
void devdrv_dma_check_sram_init_status(const void __iomem *io_base, unsigned long timeout);
int devdrv_register_irq_func(void *drvdata, int vector_index, irqreturn_t (*callback_func)(int, void *), void *para,
                             const char *name);
int devdrv_unregister_irq_func(void *drvdata, int vector_index, void *para);
int devdrv_notify_dma_err_irq(void *drvdata, u32 dma_chan_id, int err_irq);
int devdrv_check_dl_dlcmsm_state(void *drvdata);
int devdrv_dma_copy(struct devdrv_dma_dev *dma_dev, enum devdrv_dma_data_type type, int instance,
                    struct devdrv_dma_node *dma_node, u32 node_cnt, int wait_type, int copy_type,
                    struct devdrv_asyn_dma_para_info *para_info);
int devdrv_dma_copy_sml_pkt(struct devdrv_dma_dev *dma_dev, enum devdrv_dma_data_type type, dma_addr_t dst,
                            const void *data, u32 size);
void devdrv_dma_err_proc(struct devdrv_dma_channel *dma_chan);
int devdrv_dma_para_check(u32 dev_id, enum devdrv_dma_data_type type, int copy_type,
    const struct devdrv_asyn_dma_para_info *para_info);
int devdrv_dma_node_check(u32 dev_id, const struct devdrv_dma_node *dma_node, u32 node_cnt,
    const struct devdrv_dma_dev *dma_dev);
void devdrv_set_dma_status(struct devdrv_dma_dev *dma_dev, u32 status);
void devdrv_set_dma_chan_status(struct devdrv_dma_channel *dma_chan, u32 status);

void devdrv_dma_ops_init(struct devdrv_dma_dev *dma_dev, u32 chip_type);
int devdrv_dma_chan_init(struct devdrv_dma_channel *dma_chan);
int devdrv_dma_chan_reset(struct devdrv_dma_channel *dma_chan, u32 sriov_flag);
int devdrv_dma_chan_err_proc(struct devdrv_dma_channel *dma_chan);
void devdrv_res_dma_traffic(struct devdrv_dma_dev *dma_dev);
int agentdrv_is_remote_dma_chan(struct devdrv_dma_dev *dma_dev, u32 chan_id);
void devdrv_sriov_pf_dma_traffic(struct devdrv_dma_dev *dma_dev);
int devdrv_sriov_dma_init_pf_chan(struct devdrv_dma_dev *dma_dev);
int agentdrv_sriov_init_dma(u32 dev_id, u32 computility, u32 total, unsigned long *dma_bitmap);
void agentdrv_sriov_uninit_dma(u32 dev_id);
int agentdrv_get_connect_type(void);

#endif
