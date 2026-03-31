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

#ifndef _AGENTDRV_MSG_H_
#define _AGENTDRV_MSG_H_

#include <linux/types.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/irqreturn.h>

#include "devdrv_interface.h"
#include "devdrv_msg_def.h"
#include "apb_drv.h"

#define AGENTDRV_ADMIN_CHAN_ID 0
#define AGENTDRV_QUEUE_CNT 2
#define AGENTDRV_MSG_SEND_NUM 10
#define AGENTDRV_DMA_NODE_NUM 2
#define AGENTDRV_SQ_CQ_QUEUE_OFFSET 4
#define AGENTDRV_WORK_RESQ_TIME 10


enum agentdrv_mem_type {
    AGENTDRV_MEM_SRAM = 0x0,
    AGENTDRV_MEM_DDR,
    AGENTDRV_MEM_HBM
};

/* sync or async msg type */
#define AGENTDRV_MSG_SYNC (0 << 0)
#define AGENTDRV_MSG_ASYNC (1 << 0)

struct agentdrv_p2p_msg {
    u32 msg_type; /* enum agentdrv_p2p_msg_type */
    u32 buf_len;  /* input real length */
    u32 in_len;   /* output max length */
    u32 out_len;  /* output real length */
    u32 status;   /* DEVDRV_MSG_CMD_* */
    char data[0];
};

#define DEVDRV_P2P_MSG_HEAD_LEN sizeof(struct agentdrv_p2p_msg)

struct agentdrv_p2p_msg_chan {
    u32 valid;
    u32 devid;
    u32 chan_id;
    u32 send_num;
    struct device *dev;
    struct agentdrv_p2p_msg *recv_msg;
    struct agentdrv_p2p_msg *send_msg;
    u64 send_msg_dma_addr;
    u64 recv_msg_dma_addr;
    u64 remote_send_msg_dma_addr;
    u64 remote_recv_msg_dma_addr;
    u64 remote_db_addr;
    struct work_struct msg_proc_task;
    struct mutex lock;
};

struct agentdrv_msg_chan;

struct agentdrv_msg_queue_info {
    void *db_base;
    u32 desc_size;
    u32 depth;
    dma_addr_t base_h;     /* host msg queue dma addr */
    void *base_d;          /* device msg queue virt addr */
    dma_addr_t dma_base_d; /* device msg queue dma addr */
    void *base_reserve_d;
    dma_addr_t dma_reserve_d;
    u32 head;
    u32 tail;
    u32 head_res;
    u32 tail_res;
    int (*db_fun)(struct agentdrv_msg_chan *);
};

struct agentdrv_msg_chan_stat {
    u64 msg_type;
    u64 tx_total_cnt;
    u64 tx_success_cnt;
    u64 tx_no_callback;
    u64 tx_len_check_err;
    u64 tx_reply_len_check_err;
    u64 tx_dma_copy_err;
    u64 tx_timeout_err;
    u64 tx_process_err;
    u64 tx_invalid_para_err;
    u64 rx_total_cnt;
    u64 rx_success_cnt;
    u64 rx_para_err;
    u64 rx_work_max_time;
    u64 rx_work_delay_cnt;
};

struct agentdrv_msg_chan {
    u32 init_flg; /* if host driver has notice me the sq cq buf base */
    u32 chan_id;
    u64 seq_num;
    u32 status;
    struct agentdrv_msg_dev *msg_dev;
    struct agentdrv_msg_queue_info sq;
    struct agentdrv_msg_queue_info cq;
    enum agentdrv_msg_client_type msg_type;
    enum msg_queue_type queue_type;
    int irq_rx_msg_notify;
    int irq_tx_finish_notify;
    void *priv;
    u32 flag;
    void (*rx_trans_msg_notify)(void *msg_chan);
    void (*tx_trans_finish_notify)(void *msg_ch);
    int (*rx_msg_process)(void *msg_chan, void *data, u32 in_data_len, u32 out_data_len, u32 *real_out_len);
    int rx_work_flag;
    struct work_struct rx_work;
    struct mutex mutex;
    struct agentdrv_msg_chan_stat chan_stat;
    u32 stamp;
};

struct agentdrv_msg_dev {
    void __iomem *sdi_base;
    void __iomem *raise_int_base;
    void __iomem *doorbell_base;
    void __iomem *reserve_mem_base;
    void *priv_pci_unit;
    struct device *dev;
    struct devdrv_dma_dev *dma_dev;
    int dev_id;
    int func_id;
    u32 io_chan_cnt;
    u32 msi_irq_base;
    u32 irq_nvme_ctrl;
    u32 dev_status;
    struct workqueue_struct *work_queue;
    struct agentdrv_p2p_msg_chan p2p_chan[DEVDRV_P2P_SUPPORT_MAX_DEVICE];
    /* io_chan must be the last element */
    struct agentdrv_msg_chan io_chan[0];
};

int devdrv_register_irq_func(void *drvdata, int vector_index, irqreturn_t (*callback_func)(int, void *), void *para,
                             const char *name);
int devdrv_unregister_irq_func(void *drvdata, int vector_index, void *para);
irqreturn_t devdrv_remote_dma_err_interrupt(int irq, void *data);
irqreturn_t devdrv_all_dma_err_interrupt(int irq, void *data);
struct agentdrv_msg_dev *agentdrv_alloc_msg_dev(u32 func_id);
int agentdrv_msg_dev_init(struct agentdrv_msg_dev *msg_dev);
void agentdrv_free_msg_dev(const struct agentdrv_msg_dev *msg_dev);
void agentdrv_msg_dev_exit(struct agentdrv_msg_dev *msg_dev);
void agentdrv_iocmd_raise_int_to_h(struct agentdrv_msg_dev *dev, int irq);
u32 agentdrv_get_agentid_by_msg_dev(const struct agentdrv_msg_dev *msg_dev);

/* These two function for admin msg chan */
int agentdrv_msg_alloc_msg_queue(struct agentdrv_msg_dev *msg_dev, void *data);
int agentdrv_msg_free_msg_queue(struct agentdrv_msg_dev *msg_dev, void *data);

void agentdrv_set_dma_host_err_irq(int dev_id, u32 dma_chan_id, int err_irq);
int agentdrv_msg_alloc_local_sq(struct agentdrv_msg_chan *chan, u32 depth, u32 bd_size);
u64 agentdrv_get_dev_rsv_mem_base(int chip_id, int func_id);

/* doorbell proc functions */
void agentdrv_msg_chan_sq_doorbell_proc(struct agentdrv_msg_chan *chan);
void agentdrv_msg_chan_cq_doorbell_proc(struct agentdrv_msg_chan *chan);

int agentdrv_sriov_event_notify(struct agentdrv_msg_dev *msg_dev, void *data);
int devdrv_respond_msg_to_remote_side(u32 dev_id, u64 src, u64 dst, const void *data, u32 size);

#endif
