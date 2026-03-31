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
#ifdef CONFIG_GENERIC_BUG
#undef CONFIG_GENERIC_BUG
#endif
#ifdef CONFIG_BUG
#undef CONFIG_BUG
#endif

#ifdef CONFIG_DEBUG_BUGVERBOSE
#undef CONFIG_DEBUG_BUGVERBOSE
#endif

#ifdef CFG_HOST
#include "securec.h"
#else
#include <linux/securec.h>
#endif

#include <asm/io.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/vmalloc.h>
#include <asm/ptrace.h>
#include <linux/timex.h>
#include <linux/rtc.h>
#include <linux/version.h>

#include "nvme_comm_drv.h"
#include "resource_drv.h"
#include "devdrv_util.h"
#include "devdrv_dma.h"

/* dma channel sq submit interface */
void devdrv_dma_ch_sq_submit(struct devdrv_dma_channel *dma_chan)
{
    devdrv_set_dma_sq_tail(dma_chan->io_base, dma_chan->sq_tail);
}

STATIC u32 devdrv_get_dma_sqcq_side(const struct devdrv_dma_channel *dma_chan)
{
    return (dma_chan->flag >> DEVDRV_DMA_SQCQ_SIDE_BIT) & 1U;
}

void devdrv_set_dma_status(struct devdrv_dma_dev *dma_dev, u32 status)
{
    if (dma_dev != NULL) {
        dma_dev->dev_status = status;
    }
}

void devdrv_set_dma_chan_status(struct devdrv_dma_channel *dma_chan, u32 status)
{
    if (dma_chan != NULL) {
        spin_lock_bh(&dma_chan->lock);
        dma_chan->chan_status = status;
        spin_unlock_bh(&dma_chan->lock);
    }
}

void devdrv_dma_done_proc(struct devdrv_dma_soft_bd *soft_bd, struct devdrv_dma_channel *dma_chan)
{
    struct devdrv_dma_soft_bd_wait_status *wait_status = NULL;

    if (soft_bd->copy_type == DEVDRV_DMA_SYNC) {
        wait_status = (struct devdrv_dma_soft_bd_wait_status *)soft_bd->priv;
        if (wait_status != NULL) {
            wait_status->status = soft_bd->status;
            wmb();
            wait_status->valid = DEVDRV_DISABLE;
            soft_bd->priv = NULL;
        }
        wmb();
        /* synchronous mode release semaphore wake-up waiting task */
        if (soft_bd->wait_type == DEVDRV_DMA_WAIT_INTR) {
            up(&soft_bd->sync_sem);
            dma_chan->status.sync_sem_up_cnt++;
        }
        atomic_set(&soft_bd->process_flag, DEVDRV_DMA_PROCESS_INIT);
    } else {
        /* asynchronous mode callback completion function */
        if (soft_bd->callback_func) {
            soft_bd->callback_func(soft_bd->priv, soft_bd->trans_id, soft_bd->status);
            dma_chan->status.async_proc_cnt++;
        }
    }
}

/* not all sq bds will respond to cq, and multiple sq may be merged. Need to consider the merged sq */
void devdrv_dma_done_task_proc(struct devdrv_dma_channel *dma_chan, u32 cq_sqhd, u32 status)
{
    struct devdrv_dma_soft_bd *owner_soft_bd = NULL;
    struct devdrv_dma_soft_bd *soft_bd = NULL;
    u32 process_flag;
    u32 sq_index;
    u32 cur_status;
    u32 sq_cnt = (cq_sqhd + dma_chan->sq_depth - dma_chan->sq_head + 1) % dma_chan->sq_depth;

    for (sq_index = 0; sq_index < sq_cnt; sq_index++) {
        /* merged cq status is ok */
        cur_status = (dma_chan->sq_head == cq_sqhd) ? status : 0;
        soft_bd = dma_chan->dma_soft_bd + dma_chan->sq_head;

        /*
         * do not pay attention to soft bd, like the second bd sent by small packet;
         * do not pay attention to timeout bd
         */
        process_flag = atomic_cmpxchg(&soft_bd->process_flag, DEVDRV_DMA_PROCESS_INIT,
            DEVDRV_DMA_PROCESS_HANDLING);
        if ((soft_bd->valid == DEVDRV_DISABLE) || (process_flag == DEVDRV_DMA_PROCESS_WAIT_TIMEOUT)) {
            dma_chan->sq_head = (dma_chan->sq_head + 1) % dma_chan->sq_depth;
            continue;
        }
        rmb();
        /* the front bd in chain copy */
        if (soft_bd->owner_bd >= 0) {
            /* status error needs to be set to the last bd */
            if (cur_status != 0) {
                owner_soft_bd = dma_chan->dma_soft_bd + soft_bd->owner_bd;
                owner_soft_bd->status = cur_status;
            }
        } else {
            /* if there is no error in front of bd, assign the status of the last sq. */
            if (soft_bd->status == -1) {
                soft_bd->status = cur_status;
            }

            wmb();
            devdrv_dma_done_proc(soft_bd, dma_chan);
        }

        soft_bd->valid = DEVDRV_DISABLE;
        dma_chan->sq_head = (dma_chan->sq_head + 1) % dma_chan->sq_depth;
    }

    if (status != 0) {
        devdrv_err("DMA copy failed. (local_id=%u; sq=%d; error_code=0x%x)\n", dma_chan->chan_id, cq_sqhd, status);
    }
}

void devdrv_dma_done_task(unsigned long data)
{
    struct devdrv_dma_cq_node *p_cur_last_cq = NULL;
    struct devdrv_dma_channel *dma_chan = (struct devdrv_dma_channel *)((uintptr_t)data);
    u32 head;
    int cnt = 0;
    u64 ivl;

    if (spin_trylock_bh(&dma_chan->cq_lock) == 0) {
        return;
    }

    if (dma_chan->chan_status != DEVDRV_DMA_CHAN_ENABLED) {
        spin_unlock_bh(&dma_chan->cq_lock);
        return;
    }

    dma_chan->status.done_tasklet_in_cnt++;
    dma_chan->status.done_tasklet_in_time = jiffies;

    while (1) {
        head = (dma_chan->cq_head + 1) % (dma_chan->cq_depth);
        p_cur_last_cq = dma_chan->cq_desc_base + head;

        /* invalid cq, break */
        if (!dma_chan->dma_dev->ops.devdrv_dma_get_cq_valid(p_cur_last_cq, dma_chan->rounds)) {
            break;
        }
        rmb();
        /* Reach the threshold and schedule out */
        if (cnt >= DMA_DONE_BUDGET) {
            tasklet_schedule(&dma_chan->dma_done_task);
            dma_chan->status.re_schedule_cnt++;
            break;
        }

        devdrv_dma_done_task_proc(dma_chan, devdrv_dma_get_cq_sqhd(p_cur_last_cq),
            devdrv_dma_get_cq_status(p_cur_last_cq));

        dma_chan->dma_dev->ops.devdrv_dma_set_cq_invalid(p_cur_last_cq);
        dma_chan->cq_head = head;

        if (dma_chan->cq_head == (dma_chan->cq_depth - 1)) {
            dma_chan->rounds++;
        }

        cnt++;
    }

    dma_chan->status.done_tasklet_out_time = jiffies;
    ivl = jiffies_to_msecs(dma_chan->status.done_tasklet_out_time - dma_chan->status.done_tasklet_in_time);
    if (ivl > dma_chan->status.max_task_op_time) {
        dma_chan->status.max_task_op_time = ivl;
    }
    mb();
    if (cnt > 0) {
        devdrv_set_dma_cq_head(dma_chan->io_base, dma_chan->cq_head);
    }

    spin_unlock_bh(&dma_chan->cq_lock);
}

irqreturn_t devdrv_dma_done_interrupt(int irq, void *data)
{
    struct devdrv_dma_channel *dma_chan = (struct devdrv_dma_channel *)data;

    if ((dma_chan->dma_dev->dev_status != DEVDRV_DMA_ALIVE) || (dma_chan->chan_status != DEVDRV_DMA_CHAN_ENABLED)) {
        devdrv_err("DMA chan disable. (chan_id=%d)\n", dma_chan->chan_id);
        return IRQ_HANDLED;
    }

    rmb();

    dma_chan->status.done_int_cnt++;
    dma_chan->status.done_int_in_time = jiffies;
    tasklet_schedule(&dma_chan->dma_done_task);

    return IRQ_HANDLED;
}

void devdrv_show_soft_sqe(struct devdrv_dma_channel *dma_chan)
{
    u32 *sq_desc = NULL;
    u32 sq_index = devdrv_get_sq_err_ptr(dma_chan->io_base);
    int i, num;

    devdrv_err("Get sq index. (sq_index=%u)\n", sq_index);
    if (sq_index < dma_chan->sq_depth) {
        sq_desc = (u32 *)(dma_chan->sq_desc_base + sq_index);
        num = (int)(sizeof(struct devdrv_dma_sq_node) / sizeof(u32));

        for (i = 0; i < num; i++) {
            devdrv_err("Get the DW of current descriptor. (dw_index=%d; reg_val=0x%x)\n", i, sq_desc[i]);
        }
    }
}

void devdrv_dma_err_proc(struct devdrv_dma_channel *dma_chan)
{
    devdrv_dma_done_task((unsigned long)(uintptr_t)dma_chan);
    (void)devdrv_dma_chan_err_proc(dma_chan);

    devdrv_show_soft_sqe(dma_chan);
    devdrv_err("Get err chan_id. (chan_id=%u)\n", dma_chan->chan_id);
    devdrv_err("Get sq vir base addr. (sq_desc_base=0x%pK)\n", dma_chan->sq_desc_base);
    devdrv_err("Get cq vir base addr. (cq_desc_base=0x%pK)\n", dma_chan->cq_desc_base);
    devdrv_err("Get software sq head. (sq_head=%d)\n", dma_chan->sq_head);
    devdrv_err("Get software sq tail. (sq_tail=%d)\n", dma_chan->sq_tail);
    devdrv_err("Get software cq head. (cq_head=%d)\n", dma_chan->cq_head);
}

void devdrv_dma_err_task(struct work_struct *p_work)
{
    struct devdrv_dma_channel *dma_chan = container_of(p_work, struct devdrv_dma_channel, err_work);

    dma_chan->status.err_work_cnt++;

    devdrv_dma_err_proc(dma_chan);
}

irqreturn_t devdrv_dma_err_interrupt(int irq, void *data)
{
    struct devdrv_dma_channel *dma_chan = (struct devdrv_dma_channel *)data;

    if ((dma_chan->dma_dev->dev_status != DEVDRV_DMA_ALIVE) || (dma_chan->chan_status != DEVDRV_DMA_CHAN_ENABLED)) {
        devdrv_err("DMA chan disable. (chan_id=%d)\n", dma_chan->chan_id);
        return IRQ_HANDLED;
    }

    rmb();

    dma_chan->status.err_int_cnt++;

    /* start work queue */
    (void)schedule_work(&dma_chan->err_work);

    return IRQ_HANDLED;
}

STATIC void devdrv_dma_parse_sq_interrupt_info(struct devdrv_dma_channel *dma_chan,
    const struct devdrv_asyn_dma_para_info *para_info, u32 *ldie, u32 *rdie, u32 *msi)
{
    if (devdrv_get_dma_sqcq_side(dma_chan) == DEVDRV_DMA_REMOTE_SIDE) {
        *rdie = 1;
        *msi = dma_chan->done_irq;
        dma_chan->status.trigger_remot_int_cnt++;
        return;
    }
    if (para_info != NULL) {
        if (para_info->interrupt_and_attr_flag & DEVDRV_REMOTE_IRQ_FLAG) {
            *rdie = 1;
            *msi = para_info->remote_msi_vector;
            dma_chan->remote_irq_cnt++;
            dma_chan->status.trigger_remot_int_cnt++;

            /* add a local irq to update local SQ head and tail */
            if (dma_chan->remote_irq_cnt == DEVDRV_DMA_MAX_REMOTE_IRQ) {
                *ldie = 1;
                dma_chan->remote_irq_cnt = 0;
                dma_chan->status.trigger_local_128++;
            }
        }
        if (para_info->interrupt_and_attr_flag & DEVDRV_LOCAL_IRQ_FLAG) {
            *ldie = 1;
        }
    } else {
        *ldie = 1;
    }
}

/* Avoid cq merge cause int loss when the previous sq's ldie==1, and current sq's ldie==0 & rdie==1, set ldie=1. */
u32 devdrv_dma_get_amended_ldie(struct devdrv_dma_channel *dma_chan, u32 ldie, u32 rdie)
{
    u32 amended_ldie = ldie;
    u32 irq_type = dma_chan->last_irq_type;

    if ((irq_type == DEVDRV_DMA_SQ_LDIE_ENABEL) && (rdie == 1) && (ldie == 0)) {
        amended_ldie = 1;
    }
    dma_chan->last_irq_type = ldie + (rdie << 1);

    return amended_ldie;
}

STATIC int devdrv_dma_iova_addr_check(u32 dev_id, struct devdrv_dma_node *dma_node)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    u32 chip_id = 0;
    u32 func_id = 0;
    u64 dma_src_start_addr;
    u64 dma_src_end_addr;

    devdrv_dev2chipfunc(dev_id, &chip_id, &func_id);
    if (func_id >= MAX_AGENTFUNC_CNT) {
        devdrv_err_spinlock("Func id is invalid. (dev_id=%d, func_id=%u)\n", dev_id, func_id);
        return -EINVAL;
    }
    agent_dev = agentdrv_get_dev((int)chip_id);
    if (agent_dev == NULL) {
        devdrv_err_spinlock("Agent_dev is NULL. (dev_id=%d, chip_id=%u)\n", dev_id, chip_id);
        return -EINVAL;
    }

    if (agent_dev->iova_range[func_id].init_flag != DEVDRV_DMA_IOVA_RANGE_INIT) {
        return 0;
    }

    dma_src_start_addr = agent_dev->iova_range[func_id].start_addr;
    dma_src_end_addr = agent_dev->iova_range[func_id].end_addr;
    if (dma_node->direction == DEVDRV_DMA_HOST_TO_DEVICE) {
        if (((dma_node->src_addr >= dma_src_start_addr) && (dma_node->src_addr < dma_src_end_addr)) ||
            ((dma_node->src_addr < dma_src_start_addr) &&
            ((dma_node->src_addr + dma_node->size > dma_src_start_addr) ||
            (dma_node->src_addr + dma_node->size <= dma_node->src_addr)))) {
            devdrv_err_spinlock("Dma H2D, sq src addr check failed.\n");
            return -EINVAL;
        }
    } else if (dma_node->direction == DEVDRV_DMA_DEVICE_TO_HOST) {
        if (((dma_node->dst_addr >= dma_src_start_addr) && (dma_node->dst_addr < dma_src_end_addr)) ||
            ((dma_node->dst_addr < dma_src_start_addr) &&
            ((dma_node->dst_addr + dma_node->size > dma_src_start_addr) ||
            (dma_node->dst_addr + dma_node->size <= dma_node->dst_addr)))) {
            devdrv_err_spinlock("Dma D2H, sq dst addr check failed.\n");
            return -EINVAL;
        }
    } else {
        devdrv_err_spinlock("Dma direction is invalid.\n");
        return -EINVAL;
    }

    return 0;
}

STATIC void devdrv_dma_fill_sq_desc(struct devdrv_dma_channel *dma_chan, struct devdrv_dma_sq_node *sq_desc,
    const struct devdrv_dma_node *dma_node, struct devdrv_asyn_dma_para_info *para_info, int intr_flag)
{
    u32 rdie = 0;
    u32 ldie = 0;
    u32 msi = 0;
    u32 attr = DEVDRV_DMA_SO_RELEX_ORDER;
    u32 wd_barrier = 0;
    u32 rd_barrier = 0;
    u32 chip_type = HISI_CHIP_NUM;
    u32 opcode;
    int connect_type;

    connect_type = devdrv_get_connect_protocol(dma_chan->dma_dev->dev_id);
    if (dma_node->direction == DEVDRV_DMA_DEVICE_TO_HOST) {
        opcode = DEVDRV_DMA_WRITE;
    } else {
        opcode = DEVDRV_DMA_READ;
    }

    chip_type = devdrv_get_dev_chip_type(dma_chan->dma_dev->dev_id);
    if (chip_type == HISI_CHIP_UNKNOWN) {
        devdrv_err_spinlock("Got chip type failed. (dev_id=%u)\n", dma_chan->dma_dev->dev_id);
    }

    if ((chip_type == HISI_CLOUD_V2) || (chip_type == HISI_MINI_V3)) {
        attr = DEVDRV_DMA_RO_RELEX_ORDER;
    }

    if (para_info != NULL) {
        if (!(para_info->interrupt_and_attr_flag & DEVDRV_ATTR_FLAG)) {
            attr = DEVDRV_DMA_RO_RELEX_ORDER;
        }
        if (para_info->interrupt_and_attr_flag & DEVDRV_WD_BARRIER_FLAG) {
            wd_barrier = 1;
        }
        if (para_info->interrupt_and_attr_flag & DEVDRV_RD_BARRIER_FLAG) {
            rd_barrier = 1;
        }
    }

    if (intr_flag == 1) {
        devdrv_dma_parse_sq_interrupt_info(dma_chan, para_info, &ldie, &rdie, &msi);
    }

    if (chip_type == HISI_MINI_V1) {
        ldie = devdrv_dma_get_amended_ldie(dma_chan, ldie, rdie);
    }

    /* fill addr */
    devdrv_dma_set_sq_addr_info(sq_desc, dma_node->src_addr, dma_node->dst_addr, dma_node->size);

    /* fill attr */
    devdrv_dma_set_sq_attr(sq_desc, opcode, attr, dma_chan->dma_dev, wd_barrier, rd_barrier);

    /* fill interrupt info */
    devdrv_dma_set_sq_irq(sq_desc, rdie, ldie, msi);

    /* fill passid info */
    devdrv_dma_set_passid(sq_desc, dma_node->loc_passid);
}

void devdrv_dma_fill_soft_bd(int wait_type, int copy_type, struct devdrv_dma_soft_bd *soft_bd,
    struct devdrv_asyn_dma_para_info *para_info)
{
    if (para_info != NULL) {
        soft_bd->priv = para_info->priv;
        soft_bd->trans_id = para_info->trans_id;
        soft_bd->callback_func = para_info->finish_notify;
    } else {
        soft_bd->priv = NULL;
        soft_bd->trans_id = 0;
        soft_bd->callback_func = NULL;
    }
    soft_bd->copy_type = copy_type;
    soft_bd->wait_type = wait_type;
    soft_bd->owner_bd = -1;
    soft_bd->status = -1;
    atomic_set(&soft_bd->process_flag, DEVDRV_DMA_PROCESS_INIT);
    sema_init(&soft_bd->sync_sem, 0);
    soft_bd->valid = DEVDRV_ENABLE;
}

int devdrv_dma_para_check(u32 dev_id, enum devdrv_dma_data_type type, int copy_type,
    const struct devdrv_asyn_dma_para_info *para_info)
{
    int type_tmp;

    type_tmp = (int)type;

    if ((type_tmp >= DEVDRV_DMA_DATA_TYPE_MAX) || (type_tmp < DEVDRV_DMA_DATA_COMMON)) {
        devdrv_err_spinlock("Device type is invalid. (dev_id=%d; type=%d)\n", dev_id, type_tmp);
        return -EINVAL;
    }

    if (copy_type == DEVDRV_DMA_ASYNC) {
        if (para_info == NULL) {
            devdrv_err_spinlock("Device async mode para_info is null. (dev_id=%d)\n", dev_id);
            return -EINVAL;
        }

        if (!(para_info->interrupt_and_attr_flag & DEVDRV_LOCAL_REMOTE_IRQ_FLAG)) {
            if (para_info->finish_notify) {
                devdrv_err_spinlock("Device para_info is error. (dev_id=%d)\n", dev_id);
                return -EINVAL;
            }
        }
    }
    return 0;
}

int devdrv_dma_node_check(u32 dev_id, const struct devdrv_dma_node *dma_node, u32 node_cnt,
    const struct devdrv_dma_dev *dma_dev)
{
    u32 i;
    u32 sq_space_max;

    if (dma_dev == NULL) {
        devdrv_err_spinlock("Dma_dev is null. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    sq_space_max = dma_dev->sq_cq_info.sq_depth - dma_dev->sq_cq_info.sq_rsv_num;

    if ((node_cnt == 0) || (node_cnt > sq_space_max)) {
        devdrv_err_spinlock("Device node_cnt is illegal. (dev_id=%d; node_cnt=%d)\n", dev_id, node_cnt);
        return -EINVAL;
    }

    if (dma_node == NULL) {
        devdrv_err_spinlock("Device dma_node is null. (dev_id=%d)\n", dev_id);
        return -EINVAL;
    }

    for (i = 0; i < node_cnt; i++) {
        if (dma_node[i].size == 0) {
            devdrv_err_spinlock("Input Parameter is error. (dma_node=%d; size=%x)\n", i, dma_node[i].size);
            return -EINVAL;
        }

        if ((dma_node[i].direction != DEVDRV_DMA_DEVICE_TO_HOST) &&
            (dma_node[i].direction != DEVDRV_DMA_HOST_TO_DEVICE)) {
            devdrv_err_spinlock("Dma_node direction is error. (dma_node=%d; direction=%d)\n", i, dma_node[i].direction);
            return -EINVAL;
        }
    }
    return 0;
}

struct devdrv_dma_channel *devdrv_dma_get_chan(u32 dev_id, enum devdrv_dma_data_type type)
{
    struct devdrv_dma_dev *dma_dev = NULL;
    struct data_type_chan *data_chan = NULL;
    int entry_id;

    if (type >= DEVDRV_DMA_DATA_TYPE_MAX) {
        return NULL;
    }

    dma_dev = devdrv_get_dma_dev(dev_id);
    if (dma_dev == NULL) {
        devdrv_err("Function devdrv_get_dma_dev failed, dma_dev is NULL. (dev_id=%u)\n", dev_id);
        return NULL;
    }

    data_chan = &dma_dev->data_chan[type];

    entry_id = data_chan->chan_start_id + ((data_chan->last_use_chan + 1) % data_chan->chan_num);
    data_chan->last_use_chan = (u32)entry_id;

    return &dma_dev->dma_chan[entry_id];
}

int devdrv_dma_get_sq_idle_bd_cnt(struct devdrv_dma_channel *dma_chan)
{
    struct devdrv_dma_dev *dma_dev = dma_chan->dma_dev;
    u32 sq_tail, sq_head, sq_depth, sq_access, sq_rsv_num;

    sq_tail = dma_chan->sq_tail;
    sq_head = dma_chan->sq_head;
    sq_depth = dma_chan->sq_depth;
    sq_rsv_num = dma_dev->sq_cq_info.sq_rsv_num;

    sq_access = sq_depth - ((sq_tail + sq_depth - sq_head) % sq_depth) - sq_rsv_num;
    dma_chan->status.sq_idle_bd_cnt = sq_access;
    return (int)sq_access;
}

struct devdrv_dma_soft_bd *devdrv_get_soft_bd(struct devdrv_dma_channel *dma_chan)
{
    struct devdrv_dma_soft_bd *soft_bd = NULL;

    soft_bd = dma_chan->dma_soft_bd + dma_chan->sq_tail;

    return soft_bd;
}

STATIC int devdrv_dma_chan_sync_wait_intr(u32 dev_id, struct devdrv_dma_channel *dma_chan,
    struct devdrv_dma_soft_bd *soft_bd, const struct devdrv_dma_soft_bd_wait_status *wait_status)
{
    u32 process_flag;
    int ret;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
    struct timespec64 start_time;
    struct timespec64 end_time;

    ktime_get_real_ts64(&(start_time));
    ret = down_timeout(&soft_bd->sync_sem, DEVDRV_DMA_COPY_TIMEOUT);
    ktime_get_real_ts64(&(end_time));

#else
    struct timeval start_time;
    struct timeval end_time;

    do_gettimeofday(&(start_time));
    ret = down_timeout(&soft_bd->sync_sem, DEVDRV_DMA_COPY_TIMEOUT);
    do_gettimeofday(&(end_time));

#endif

    if (ret) {
        /* call done task if timeout */
        ret = 0;
RETRY_SYNC_WAIT:
        if (dma_chan->chan_status != DEVDRV_DMA_CHAN_ENABLED) {
            return -EINVAL;
        }
        devdrv_dma_done_task((unsigned long)(uintptr_t)dma_chan);
        /* check soft_bd_wait_status valid */
        if (wait_status->valid == DEVDRV_ENABLE) {
            process_flag = atomic_cmpxchg(&soft_bd->process_flag, DEVDRV_DMA_PROCESS_INIT,
                DEVDRV_DMA_PROCESS_WAIT_TIMEOUT);
            if (process_flag == DEVDRV_DMA_PROCESS_HANDLING) {
                goto RETRY_SYNC_WAIT;
            }
            ret = -ETIMEDOUT;
            devdrv_dma_chan_ptr_show(dma_chan);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
            devdrv_err("DMA timeout. (dev_id=%u; dma_channel=%u; ret=%d; wait_time=%llu)\n",
                dev_id, dma_chan->chan_id, ret,
                (((u64)(end_time.tv_sec)) * DEVDRV_SECOND_TO_MICROSECOND +
                (u64)end_time.tv_nsec / DEVDRV_MICROSECOND_TO_NANOSECOND) -
                (((u64)(start_time.tv_sec)) * DEVDRV_SECOND_TO_MICROSECOND +
                (u64)start_time.tv_nsec / DEVDRV_MICROSECOND_TO_NANOSECOND));
#else
            devdrv_err("DMA timeout. (dev_id=%u; dma_channel=%u; ret=%d; wait_time=%llu)\n",
                dev_id, dma_chan->chan_id, ret,
                (((u64)(end_time.tv_sec)) * DEVDRV_SECOND_TO_MICROSECOND + end_time.tv_usec) -
                (((u64)(start_time.tv_sec)) * DEVDRV_SECOND_TO_MICROSECOND + start_time.tv_usec));
#endif
        }
    }

    return ret;
}

STATIC int devdrv_dma_chan_sync_wait_query(u32 dev_id, const struct devdrv_dma_channel *dma_chan,
    struct devdrv_dma_soft_bd *soft_bd, const struct devdrv_dma_soft_bd_wait_status *wait_status)
{
    int wait_cnt = 0;
    u32 process_flag;
    int ret = 0;

    do {
        if (dma_chan->chan_status != DEVDRV_DMA_CHAN_ENABLED) {
            break;
        }
        /* check cq status,update soft_bd and soft_bd_wait_status */
        devdrv_dma_done_task((unsigned long)(uintptr_t)dma_chan);
        /* check soft_bd_wait_status valid */
        if (wait_status->valid == DEVDRV_DISABLE) {
            break;
        }

        rmb();

        if (wait_cnt++ > DEVDRV_DMA_QUERY_MAX_WAIT_TIME) {
            process_flag = atomic_cmpxchg(&soft_bd->process_flag, DEVDRV_DMA_PROCESS_INIT,
                DEVDRV_DMA_PROCESS_WAIT_TIMEOUT);
            if (process_flag == DEVDRV_DMA_PROCESS_HANDLING) {
                continue;
            }
            ret = -EINVAL;
            devdrv_err("DMA channel wait timeout. (dev_id=%u; dma_chan=%u)\n", dev_id, dma_chan->chan_id);
            break;
        }
        usleep_range(1, 2);
    } while (1);

    return ret;
}

int devdrv_dma_chan_sync_wait(u32 dev_id, struct devdrv_dma_channel *dma_chan,
    struct devdrv_dma_soft_bd *soft_bd, struct devdrv_dma_soft_bd_wait_status *wait_status)
{
    int ret;

    /* interrupt mode */
    if (soft_bd->wait_type == DEVDRV_DMA_WAIT_INTR) {
        ret = devdrv_dma_chan_sync_wait_intr(dev_id, dma_chan, soft_bd, wait_status);
    } else {
    /* query mode */
        ret = devdrv_dma_chan_sync_wait_query(dev_id, dma_chan, soft_bd, wait_status);
    }

    mb();
    if (wait_status->status != 0) {
        devdrv_dma_chan_ptr_show(dma_chan);
        devdrv_err("DMA copy failed. (devid=%u; funcid=%u; dma_channel=%u; valid=%d; status=%x)\n",
            dma_chan->dma_dev->dev_id, dma_chan->func_id, dma_chan->chan_id, wait_status->valid, wait_status->status);
        dump_stack();
        ret = -EINVAL;
    }

    return ret;
}

int devdrv_dma_chan_copy(u32 dev_id, struct devdrv_dma_channel *dma_chan, struct devdrv_dma_node *dma_node,
    u32 node_cnt, int wait_type, int copy_type, struct devdrv_asyn_dma_para_info *para_info)
{
    struct devdrv_dma_soft_bd_wait_status wait_status;
    struct devdrv_dma_soft_bd *soft_bd = NULL;
    struct devdrv_dma_sq_node *sq_desc = NULL;
    u32 chan_id, last_sq_id, sq_index;
    int sq_idle_bd_cnt;
    int ret = 0;
    int intr_flag = (wait_type == DEVDRV_DMA_WAIT_INTR) ? 1 : 0;

    /* wait till chan space enough */
    spin_lock_bh(&dma_chan->lock);

    if (dma_chan->chan_status != DEVDRV_DMA_CHAN_ENABLED) {
        spin_unlock_bh(&dma_chan->lock);
        return -EINVAL;
    }

    chan_id = dma_chan->chan_id;
    dma_chan->status.dma_chan_copy_cnt++;

    sq_idle_bd_cnt = devdrv_dma_get_sq_idle_bd_cnt(dma_chan);
    if ((sq_idle_bd_cnt < 0) || ((u32)sq_idle_bd_cnt < node_cnt)) {
        spin_unlock_bh(&dma_chan->lock);
        devdrv_warn_spinlock("Channel no space. (dev_id=%u; chan_id=%u; idle_bd=%d; node_cnt=%u)\n",
            dev_id, chan_id, sq_idle_bd_cnt, node_cnt);
        return -ENOSPC;
    }

    last_sq_id = (dma_chan->sq_tail + node_cnt - 1) % dma_chan->sq_depth;
    for (sq_index = 0; sq_index < node_cnt; sq_index++) {
        ret = devdrv_dma_iova_addr_check(dev_id, &dma_node[sq_index]);
        if (ret != 0) {
            spin_unlock_bh(&dma_chan->lock);
            devdrv_err_spinlock("Dma iova addr check failed. (devid=%u; chan_id=%u)\n", dev_id, chan_id);
            return -EINVAL;
        }

        sq_desc = dma_chan->sq_desc_base + dma_chan->sq_tail;
        soft_bd = devdrv_get_soft_bd(dma_chan);

        if (memset_s((void *)sq_desc, DEVDRV_DMA_SQ_DESC_SIZE, 0, DEVDRV_DMA_SQ_DESC_SIZE) != 0) {
            spin_unlock_bh(&dma_chan->lock);
            devdrv_err_spinlock("Call memset_s failed. (devid=%u; chan_id=%u)\n", dev_id, chan_id);
            return -EIO;
        }

        if (sq_index < node_cnt - 1) {
            devdrv_dma_fill_sq_desc(dma_chan, sq_desc, &dma_node[sq_index], para_info, 0);
            soft_bd->owner_bd = last_sq_id;
            soft_bd->valid = DEVDRV_ENABLE;
        } else {
            devdrv_dma_fill_sq_desc(dma_chan, sq_desc, &dma_node[sq_index], para_info, intr_flag);
            devdrv_dma_fill_soft_bd(wait_type, copy_type, soft_bd, para_info);
        }

        dma_chan->sq_tail = (dma_chan->sq_tail + 1) % dma_chan->sq_depth;
    }

    if (copy_type == DEVDRV_DMA_SYNC) {
        wait_status.status = -1;
        wait_status.valid = DEVDRV_ENABLE;
        soft_bd->priv = &wait_status;
        dma_chan->status.sync_submit_cnt++;
    } else {
        dma_chan->status.async_submit_cnt++;
    }

    wmb();
    devdrv_dma_ch_sq_submit(dma_chan);

    spin_unlock_bh(&dma_chan->lock);

    if ((copy_type == DEVDRV_DMA_SYNC) && (soft_bd != NULL)) {
        ret = devdrv_dma_chan_sync_wait(dev_id, dma_chan, soft_bd, &wait_status);
    }

    return ret;
}

int devdrv_dma_copy(struct devdrv_dma_dev *dma_dev, enum devdrv_dma_data_type type, int instance,
    struct devdrv_dma_node *dma_node, u32 node_cnt, int wait_type, int copy_type,
    struct devdrv_asyn_dma_para_info *para_info)
{
    struct devdrv_dma_channel *dma_chan = NULL;
    struct data_type_chan *data_chan = NULL;
    struct devdrv_dma_node *real_dma_node = dma_node;
    u32 entry, i;
    u32 dev_id;
    int ret = 0;

    if (dma_dev == NULL) {
        devdrv_err_spinlock("Input parameter is error.\n");
        return -EINVAL;
    }
    dev_id = dma_dev->dev_id;

    if (dma_dev->dev_status == DEVDRV_DMA_DEAD) {
        devdrv_event("Dma is reset now, can not be used, retry later. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    devdrv_debug_spinlock("Get copy_type. (type=%x; instance=%d; node_cnt=%x; copy_type=%d)\n",
                          type, instance, node_cnt, copy_type);
    data_chan = &dma_dev->data_chan[type];

    /* If wait spinlock in the tasklet, the cq interrupt that updates the sq head also be processed in the tasklet,
        which will form a deadlock. So let the caller waits */
    if (instance == DEVDRV_INVALID_INSTANCE) {
        for (i = 0; i < data_chan->chan_num; i++) {
            entry = data_chan->chan_start_id + ((i + data_chan->last_use_chan + 1) % data_chan->chan_num);
            dma_chan = &dma_dev->dma_chan[entry];
            ret = devdrv_dma_chan_copy(dev_id, dma_chan, real_dma_node, node_cnt, wait_type, copy_type, para_info);
            if (ret != -ENOSPC) {
                data_chan->last_use_chan = entry;
                break;
            }
        }
    } else {
        entry = data_chan->chan_start_id + ((u32)instance % data_chan->chan_num);
        dma_chan = &dma_dev->dma_chan[entry];
        ret = devdrv_dma_chan_copy(dev_id, dma_chan, real_dma_node, node_cnt, wait_type, copy_type, para_info);
    }

    return ret;
}

struct devdrv_dma_channel *devdrv_dma_get_chan_by_type(struct devdrv_dma_dev *dma_dev, enum devdrv_dma_data_type type)
{
    struct data_type_chan *data_chan = NULL;
    int entry_id;

    if (type >= DEVDRV_DMA_DATA_TYPE_MAX) {
        devdrv_err("Type is out of range. (type=%u)\n", (u32)type);
        return NULL;
    }
    if (dma_dev == NULL) {
        devdrv_err("Input Parameter is null.\n");
        return NULL;
    }

    data_chan = &dma_dev->data_chan[type];
    entry_id = data_chan->chan_start_id + ((data_chan->last_use_chan + 1) % data_chan->chan_num);
    data_chan->last_use_chan = (u32)entry_id;

    return &dma_dev->dma_chan[entry_id];
}

STATIC int devdrv_dma_chan_copy_sml_pkt(int dev_id, struct devdrv_dma_channel *dma_chan, dma_addr_t dst,
    const void *data, u32 size)
{
    struct devdrv_dma_soft_bd_wait_status wait_status;
    struct devdrv_dma_sq_node *sq_desc = NULL;
    struct devdrv_dma_soft_bd *soft_bd = NULL;
    int ret;
    u32 attr = DEVDRV_DMA_SO_RELEX_ORDER;
    u32 chip_type;

    spin_lock_bh(&dma_chan->lock);

    if (devdrv_dma_get_sq_idle_bd_cnt(dma_chan) < DEVDRV_DMA_SML_PKT_SQ_DESC_NUM) {
        spin_unlock_bh(&dma_chan->lock);
        devdrv_warn("SQ space not enough. (devid=%d; dma_chan=%d; sq_tail=%d; sq_head=%d; sq_depth=%d)\n",
            dev_id, dma_chan->chan_id, dma_chan->sq_tail, dma_chan->sq_head, dma_chan->sq_depth);
        return -ENOSPC;
    }

    sq_desc = dma_chan->sq_desc_base + dma_chan->sq_tail;
    if (memset_s((void *)sq_desc, DEVDRV_DMA_SQ_DESC_SIZE, 0, DEVDRV_DMA_SQ_DESC_SIZE) != 0) {
        spin_unlock_bh(&dma_chan->lock);
        devdrv_err("Call memset_s failed. (dev_id=%d)\n", dev_id);
        return -ENOMEM;
    }

    chip_type = devdrv_get_dev_chip_type(dma_chan->dma_dev->dev_id);
    if (chip_type == HISI_CLOUD_V2) {
        attr = DEVDRV_DMA_RO_RELEX_ORDER;
    }

    /* fill addr */
    devdrv_dma_set_sq_addr_info(sq_desc, 0, dst, size);

    /* fill attr */
    devdrv_dma_set_sq_attr(sq_desc, DEVDRV_DMA_SMALL_PACKET, attr, dma_chan->dma_dev, 1, 1);

    /* fill interrupt info */
    devdrv_dma_set_sq_irq(sq_desc, 0, 1, 0);

    /* fill passid info */
    devdrv_dma_set_passid(sq_desc, DEVDRV_DMA_PASSID_DEFAULT);

    soft_bd = dma_chan->dma_soft_bd + dma_chan->sq_tail;
    devdrv_dma_fill_soft_bd(DEVDRV_DMA_WAIT_QUREY, DEVDRV_DMA_SYNC, soft_bd, NULL);

    dma_chan->sq_tail = (dma_chan->sq_tail + 1) % dma_chan->sq_depth;
    sq_desc = dma_chan->sq_desc_base + dma_chan->sq_tail;

    if (memcpy_s((void *)sq_desc, sizeof(struct devdrv_dma_sq_node), data, size) != 0) {
        spin_unlock_bh(&dma_chan->lock);
        devdrv_err("Call memcpy_s failed. (dev_id=%d)\n", dev_id);
        return -ENOMEM;
    }
    dma_chan->sq_tail = (dma_chan->sq_tail + 1) % dma_chan->sq_depth;

    wait_status.status = -1;
    wait_status.valid = DEVDRV_ENABLE;
    soft_bd->priv = &wait_status;

    wmb();
    devdrv_dma_ch_sq_submit(dma_chan);

    dma_chan->status.sml_submit_cnt++;
    spin_unlock_bh(&dma_chan->lock);

    ret = devdrv_dma_chan_sync_wait(dev_id, dma_chan, soft_bd, &wait_status);

    return ret;
}

int devdrv_dma_copy_sml_pkt(struct devdrv_dma_dev *dma_dev, enum devdrv_dma_data_type type, dma_addr_t dst,
    const void *data, u32 size)
{
    struct devdrv_dma_channel *dma_chan = NULL;
    int dev_id = -1;
    int ret;

    dma_chan = devdrv_dma_get_chan_by_type(dma_dev, type);
    if (dma_chan == NULL) {
        devdrv_err("Call devdrv_dma_get_chan failed. (dev_id=%d)\n", dev_id);
        return -EINVAL;
    }
    dev_id = dma_dev->dev_id;

    if (((dma_chan->flag >> DEVDRV_DMA_SML_PKT_BIT) & 1) == DEVDRV_DISABLE) {
        devdrv_err("This channel not support small packet. (devid=%d)\n", dev_id);
        return -EINVAL;
    }

    if (size > DEVDRV_DMA_SML_PKT_DATA_SIZE) {
        devdrv_err("Packet size is too big. (dev_id=%d; size=%u)\n", dev_id, size);
        return -EINVAL;
    }

    ret = devdrv_dma_chan_copy_sml_pkt(dev_id, dma_chan, dst, data, size);

    return ret;
}

STATIC void devdrv_free_dma_sq_cq(struct devdrv_dma_channel *dma_chan)
{
    u64 sq_size, cq_size;

    if (dma_chan->sq_desc_base != NULL) {
        sq_size = ((u64)sizeof(struct devdrv_dma_sq_node)) * dma_chan->sq_depth;

        devdrv_dma_free_coherent(dma_chan->dev, sq_size, dma_chan->sq_desc_base, dma_chan->sq_desc_dma);

        dma_chan->sq_desc_base = NULL;
    }

    if (dma_chan->cq_desc_base != NULL) {
        cq_size = ((u64)sizeof(struct devdrv_dma_cq_node)) * dma_chan->cq_depth;

        devdrv_dma_free_coherent(dma_chan->dev, cq_size, dma_chan->cq_desc_base, dma_chan->cq_desc_dma);

        dma_chan->cq_desc_base = NULL;
    }

    if (dma_chan->dma_soft_bd != NULL) {
        vfree(dma_chan->dma_soft_bd);
        dma_chan->dma_soft_bd = NULL;
    }
}

STATIC int devdrv_alloc_dma_sq_cq(struct devdrv_dma_channel *dma_chan)
{
    struct devdrv_dma_soft_bd *soft_virt_addr = NULL;
    void *sq_virt_addr = NULL;
    void *cq_virt_addr = NULL;
    struct device *dev = NULL;
    u64 soft_size;
    u64 sq_size, cq_size;
    u32 i;
    struct devdrv_dma_dev *dma_dev = dma_chan->dma_dev;

    dev = dma_chan->dev;
    sq_size = DEVDRV_DMA_SQ_DESC_SIZE * (dma_dev->sq_cq_info.sq_depth);
    cq_size = DEVDRV_DMA_CQ_DESC_SIZE * (dma_dev->sq_cq_info.cq_depth);
    soft_size = sizeof(struct devdrv_dma_soft_bd) * (dma_dev->sq_cq_info.sq_depth);

    sq_virt_addr = devdrv_dma_zalloc_coherent(dev, sq_size, &dma_chan->sq_desc_dma, GFP_KERNEL);
    if (sq_virt_addr == NULL) {
        devdrv_err("DMA channel sq alloc failed. (chan_id=%d)\n", dma_chan->chan_id);
        return -ENOMEM;
    }
    dma_chan->sq_desc_base = (struct devdrv_dma_sq_node *)sq_virt_addr;
    dma_chan->sq_depth = dma_dev->sq_cq_info.sq_depth;

    cq_virt_addr = devdrv_dma_zalloc_coherent(dev, cq_size, &dma_chan->cq_desc_dma, GFP_KERNEL);
    if (cq_virt_addr == NULL) {
        devdrv_err("DMA channel cq alloc failed. (chan_id=%d)\n", dma_chan->chan_id);
        devdrv_free_dma_sq_cq(dma_chan);
        return -ENOMEM;
    }
    dma_chan->cq_desc_base = (struct devdrv_dma_cq_node *)cq_virt_addr;
    dma_chan->cq_depth = dma_dev->sq_cq_info.cq_depth;

    /* DMA_QUEUE_SQ_BASE/DMA_QUEUE_CQ_BASE Note:the address must be 64Bytes aligned. */
    if (((dma_chan->sq_desc_dma % DEVDRV_DMA_REG_ALIGN_SIZE) != 0) ||
        ((dma_chan->cq_desc_dma % DEVDRV_DMA_REG_ALIGN_SIZE) != 0)) {
        devdrv_err("Device dma channel address dont aligned with 64B. (driver=\"%s\"; chan_id=%d)\n",
            dev_driver_string(dev), dma_chan->chan_id);
        devdrv_free_dma_sq_cq(dma_chan);
        return -EFAULT;
    }

    devdrv_debug("Got DMA channel. (chan_id=%d)\n", dma_chan->chan_id);
    soft_virt_addr = (struct devdrv_dma_soft_bd *)vzalloc(soft_size);
    if (soft_virt_addr == NULL) {
        devdrv_err("DMA channel cq alloc failed. (chan_id=%d)\n", dma_chan->chan_id);
        devdrv_free_dma_sq_cq(dma_chan);
        return -ENOMEM;
    }
    dma_chan->dma_soft_bd = soft_virt_addr;

    for (i = 0; i < dma_dev->sq_cq_info.sq_depth; i++) {
        soft_virt_addr[i].valid = DEVDRV_DISABLE;
    }

    return 0;
}

STATIC void devdrv_dma_interrupt_init_chan(struct devdrv_dma_dev *dma_dev, int entry_id)
{
    struct devdrv_dma_channel *dma_chan = &dma_dev->dma_chan[entry_id];
    u32 dma_chan_id = dma_chan->chan_id;
    int ret = 0;

    tasklet_init(&dma_chan->dma_done_task, devdrv_dma_done_task, (uintptr_t)dma_chan);
    (void)devdrv_register_irq_func(dma_dev->drvdata, dma_chan->done_irq, devdrv_dma_done_interrupt, dma_chan,
        "dma_done_irq");
    dma_chan->done_irq_state = DEVDRV_IRQ_IS_INIT;

    INIT_WORK(&dma_chan->err_work, devdrv_dma_err_task);
    if (dma_chan->err_irq_flag != 0) {
        /* err interrupt we do some dfx words, so use wordqueue which can sleep */
        (void)devdrv_register_irq_func(dma_dev->drvdata, dma_chan->err_irq, devdrv_dma_err_interrupt, dma_chan,
            "dma_err_irq");
        dma_chan->err_irq_state = DEVDRV_IRQ_IS_INIT;
    } else {
        dma_chan->err_irq = -1;
    }

    if (dma_dev->sq_cq_side == DEVDRV_DMA_REMOTE_SIDE) {
        ret = devdrv_notify_dma_err_irq(dma_dev->drvdata, dma_chan_id, dma_chan->err_irq);
        if (ret != 0) {
            devdrv_err("Notify err_irq failed. (dev_id=%u; chan_entry=%d; dma_chan_id=%u; ret=%d)\n", dma_dev->dev_id,
                entry_id, dma_chan_id, ret);
        }
    }
    return;
}

STATIC int devdrv_dma_init_chan(struct devdrv_dma_dev *dma_dev, u32 entry_id, u32 dma_chan_id, u32 sriov_flag)
{
    int ret;
    struct devdrv_dma_channel *dma_chan = &dma_dev->dma_chan[entry_id];

    dma_chan->dma_dev = dma_dev;
    dma_chan->dev = dma_dev->dev;
    dma_chan->io_base = dma_dev->dma_chan_base + (u64)dma_chan_id * DEVDRV_DMA_CHAN_OFFSET; //lint !e571
    dma_chan->chan_id = dma_chan_id;
    dma_chan->func_id = dma_dev->func_id;
    dma_chan->sq_tail = 0;
    dma_chan->sq_head = 0;
    dma_chan->cq_head = dma_chan->cq_depth - 1;
    dma_chan->rounds = 0;
    dma_chan->chan_status = DEVDRV_DMA_CHAN_ENABLED;

    if (dma_dev->sq_cq_side == DEVDRV_DMA_REMOTE_SIDE) {
        /* flags of DMA chan used in host */
        dma_chan->flag =
            (DEVDRV_DMA_REMOTE_SIDE << DEVDRV_DMA_SQCQ_SIDE_BIT) | (DEVDRV_DISABLE << DEVDRV_DMA_SML_PKT_BIT);
    } else {
        /* flags of DMA chan used in device */
        dma_chan->flag =
            ((u32)DEVDRV_DMA_LOCAL_SIDE << DEVDRV_DMA_SQCQ_SIDE_BIT) | (DEVDRV_ENABLE << DEVDRV_DMA_SML_PKT_BIT);
    }

    /* func_id 0 is pf, if support sriov, only pf init dma default when probe, other vf not init when probe */
    if ((sriov_flag == DEVDRV_SRIOV_DISABLE) &&
        (devdrv_is_sriov_enabled(agentdrv_res_get_func_total()) == true) && (dma_dev->func_id != 0)) {
        return 0;
    }

    /* reset DMA channel before init */
    ret = devdrv_dma_chan_reset(dma_chan, sriov_flag);
    if (ret != 0) {
        devdrv_err("Function dma_ch_cfg_reset failed. (devid=%u; chan_id=%d; ret=%d)\n",
            dma_dev->dev_id, dma_chan_id, ret);
        dma_chan->dma_dev = NULL;
        dma_chan->dev = NULL;
        return ret;
    }

    ret = devdrv_alloc_dma_sq_cq(dma_chan);
    if (ret != 0) {
        devdrv_err("Function alloc_dma_sq_cq failed. (devid=%u; chan_id=%d)\n", dma_dev->dev_id, dma_chan_id);
        dma_chan->dma_dev = NULL;
        dma_chan->dev = NULL;
        return ret;
    }

    ret = devdrv_dma_chan_init(dma_chan);
    if (ret != 0) {
        devdrv_err("Channel remote init failed. (devid=%u; chan_id=%d; ret=%d)\n",
            dma_dev->dev_id, dma_chan_id, ret);
        devdrv_free_dma_sq_cq(dma_chan);
        dma_chan->dma_dev = NULL;
        dma_chan->dev = NULL;
        return ret;
    }

    devdrv_dma_interrupt_init_chan(dma_dev, (int)entry_id);

    return ret;
}

int devdrv_sriov_dma_init_pf_chan(struct devdrv_dma_dev *dma_dev)
{
    struct devdrv_dma_channel *dma_chan = NULL;
    int ret;
    u32 i;

    for (i = 0; i < dma_dev->local_chan_num; i++) {
        /* chanel 0 is used by pf always, no need init again */
        if ((dma_dev->func_id == 0) && (i == 0)) {
            continue;
        }
        dma_chan = &dma_dev->dma_chan[i];

        ret = devdrv_dma_init_chan(dma_dev, i, dma_chan->chan_id, DEVDRV_SRIOV_ENABLE);
        if (ret != 0) {
            devdrv_err("dma_ch_cfg_reset failed. (dev_id=%u; ret=%d)\n", dma_dev->dev_id, ret);
            return ret;
        }
    }

    return 0;
}

void devdrv_res_dma_traffic(struct devdrv_dma_dev *dma_dev)
{
    /* init data type to dma chan map */
    dma_dev->data_chan[DEVDRV_DMA_DATA_COMMON].chan_start_id = 0;
    dma_dev->data_chan[DEVDRV_DMA_DATA_COMMON].chan_num = DEVDRV_DMA_DATA_COMM_CHAN_NUM;
    dma_dev->data_chan[DEVDRV_DMA_DATA_COMMON].last_use_chan = dma_dev->data_chan[DEVDRV_DMA_DATA_COMMON].chan_start_id;

    /* msg dma chan map */
    dma_dev->data_chan[DEVDRV_DMA_DATA_PCIE_MSG].chan_start_id =
        dma_dev->data_chan[DEVDRV_DMA_DATA_COMMON].chan_start_id + dma_dev->data_chan[DEVDRV_DMA_DATA_COMMON].chan_num;
    dma_dev->data_chan[DEVDRV_DMA_DATA_PCIE_MSG].chan_num = DEVDRV_DMA_DATA_PCIE_MSG_CHAN_NUM;
    if (dma_dev->local_chan_num <= DEVDRV_DMA_DATA_COMM_CHAN_NUM + DEVDRV_DMA_DATA_PCIE_MSG_CHAN_NUM) {
        dma_dev->data_chan[DEVDRV_DMA_DATA_PCIE_MSG].chan_start_id = 0;
    }
    dma_dev->data_chan[DEVDRV_DMA_DATA_PCIE_MSG].last_use_chan =
        dma_dev->data_chan[DEVDRV_DMA_DATA_PCIE_MSG].chan_start_id;

    /* traffic dma chan map */
    dma_dev->data_chan[DEVDRV_DMA_DATA_TRAFFIC].chan_start_id =
        dma_dev->data_chan[DEVDRV_DMA_DATA_PCIE_MSG].chan_start_id +
        dma_dev->data_chan[DEVDRV_DMA_DATA_PCIE_MSG].chan_num;
    if (dma_dev->local_chan_num <= dma_dev->data_chan[DEVDRV_DMA_DATA_TRAFFIC].chan_start_id) {
        dma_dev->data_chan[DEVDRV_DMA_DATA_TRAFFIC].chan_start_id = 0;
    }
    dma_dev->data_chan[DEVDRV_DMA_DATA_TRAFFIC].chan_num =
        dma_dev->local_chan_num - dma_dev->data_chan[DEVDRV_DMA_DATA_TRAFFIC].chan_start_id;
    dma_dev->data_chan[DEVDRV_DMA_DATA_TRAFFIC].last_use_chan =
        dma_dev->data_chan[DEVDRV_DMA_DATA_TRAFFIC].chan_start_id;
}

void devdrv_sriov_pf_dma_traffic(struct devdrv_dma_dev *dma_dev)
{
    /* init data type to dma chan map */
    dma_dev->data_chan[DEVDRV_DMA_DATA_COMMON].chan_start_id = 0;
    dma_dev->data_chan[DEVDRV_DMA_DATA_COMMON].chan_num = DEVDRV_DMA_DATA_COMM_CHAN_NUM;
    dma_dev->data_chan[DEVDRV_DMA_DATA_COMMON].last_use_chan = 0;

    /* msg dma chan map */
    dma_dev->data_chan[DEVDRV_DMA_DATA_PCIE_MSG].chan_start_id = 0;
    dma_dev->data_chan[DEVDRV_DMA_DATA_PCIE_MSG].chan_num = DEVDRV_DMA_DATA_PCIE_MSG_CHAN_NUM;
    dma_dev->data_chan[DEVDRV_DMA_DATA_PCIE_MSG].last_use_chan = 0;

    /* traffic dma chan map */
    dma_dev->data_chan[DEVDRV_DMA_DATA_TRAFFIC].chan_start_id = 0;
    dma_dev->data_chan[DEVDRV_DMA_DATA_TRAFFIC].chan_num = 1;
    dma_dev->data_chan[DEVDRV_DMA_DATA_TRAFFIC].last_use_chan = 0;
}

STATIC void devdrv_dma_irq_clear(struct devdrv_dma_dev *dma_dev)
{
    u32 i;
    struct devdrv_dma_channel *dma_chan = NULL;

    for (i = 0; i < dma_dev->local_chan_num; i++) {
        dma_chan = &dma_dev->dma_chan[i];
        dma_chan->done_irq = -1;
        dma_chan->err_irq = -1;
        dma_chan->done_irq_state = DEVDRV_IRQ_IS_UNINIT;
        dma_chan->err_irq_state = DEVDRV_IRQ_IS_UNINIT;
    }
    return;
}

struct devdrv_dma_dev *devdrv_dma_init(struct devdrv_dma_func_para *para_in, u32 sq_cq_side, u32 func_id)
{
    struct devdrv_dma_dev *dma_dev = NULL;
    u32 dma_dev_size;
    u32 chan_id;
    u32 i;

    /* 1 check dlcmsm */
    if (devdrv_check_dl_dlcmsm_state(para_in->drvdata) != 0) {
        devdrv_err("Check dlcmsm state failed.\n");
        return NULL;
    }

    /* 2 create dma dev */
    dma_dev_size = sizeof(struct devdrv_dma_dev) + sizeof(struct devdrv_dma_channel) * para_in->chan_num;
    dma_dev = (struct devdrv_dma_dev *)kzalloc(dma_dev_size, GFP_KERNEL);
    if (dma_dev == NULL) {
        devdrv_err("DMA device alloc failed. (dev_id=%u)\n", para_in->dev_id);
        return NULL;
    }

    /* 3 init dma dev */
    dma_dev->dev = para_in->dev;
    dma_dev->io_base = para_in->io_base;
    dma_dev->dma_chan_base = para_in->dma_chan_base;
    dma_dev->drvdata = para_in->drvdata;
    dma_dev->sq_cq_side = sq_cq_side;
    dma_dev->dev_id = para_in->dev_id;
    dma_dev->func_id = func_id; /* for miniv2 nid */
    dma_dev->dma_pf_num = para_in->dma_pf_num;
    dma_dev->dma_vf_en = para_in->dma_vf_en;
    dma_dev->dma_vf_num = para_in->dma_vf_num;
    dma_dev->dev_status = DEVDRV_DMA_ALIVE;
    dma_dev->sq_cq_info.sq_depth = para_in->sq_cq_info.sq_depth;
    dma_dev->sq_cq_info.sq_rsv_num = para_in->sq_cq_info.sq_rsv_num;
    dma_dev->sq_cq_info.cq_depth = para_in->sq_cq_info.cq_depth;
    dma_dev->done_irq_base = para_in->done_irq_base;
    dma_dev->err_irq_base = para_in->err_irq_base;
    dma_dev->err_flag = para_in->err_flag;

    agentdrv_res_pf_dma_chan(dma_dev);
    devdrv_dma_irq_clear(dma_dev);
    devdrv_dma_ops_init(dma_dev, para_in->chip_type);

    for (i = 0; i < dma_dev->local_chan_num; i++) {
        chan_id = dma_dev->local_chan[i];
        dma_dev->dma_chan[i].done_irq = (int)(dma_dev->done_irq_base + chan_id);
        dma_dev->dma_chan[i].err_irq = (int)(dma_dev->err_irq_base + chan_id);
        dma_dev->dma_chan[i].err_irq_flag = (int)dma_dev->err_flag;
        spin_lock_init(&dma_dev->dma_chan[i].lock);
        spin_lock_init(&dma_dev->dma_chan[i].cq_lock);
        if (devdrv_dma_init_chan(dma_dev, i, chan_id, DEVDRV_SRIOV_DISABLE) != 0) {
            devdrv_err("DMA init channel failed. (dev_id=%u; chan=%d)\n", para_in->dev_id, i);
            devdrv_dma_exit(dma_dev, DEVDRV_SRIOV_DISABLE);
            return NULL;
        }
    }

    devdrv_res_dma_traffic(dma_dev);

    return dma_dev;
}

void devdrv_dma_exit(struct devdrv_dma_dev *dma_dev, int sriov_flag)
{
    u32 i;
    struct devdrv_dma_channel *dma_chan = NULL;

    if (dma_dev == NULL) {
        return;
    }

    for (i = 0; i < dma_dev->local_chan_num; i++) {
        if ((sriov_flag == DEVDRV_SRIOV_ENABLE) && (dma_dev->func_id == 0) && (i == 0)) {
            /* if eriov enable or disable, pf's 0 dma not exit and free */
            continue;
        }

        dma_chan = &dma_dev->dma_chan[i];

        /* chan has not init */
        if ((dma_chan == NULL) || (dma_chan->dev == NULL)) {
            continue;
        }

        if (dma_chan->err_irq_state == DEVDRV_IRQ_IS_INIT) {
            dma_chan->err_irq_state = DEVDRV_IRQ_IS_UNINIT;
            (void)devdrv_unregister_irq_func(dma_dev->drvdata, dma_chan->err_irq, dma_chan);
            (void)cancel_work_sync(&dma_chan->err_work);
        }

        if (dma_chan->done_irq_state == DEVDRV_IRQ_IS_INIT) {
            dma_chan->done_irq_state = DEVDRV_IRQ_IS_UNINIT;
            (void)devdrv_unregister_irq_func(dma_dev->drvdata, dma_chan->done_irq, dma_chan);
            tasklet_kill(&dma_chan->dma_done_task);
        }

        if (sriov_flag == DEVDRV_SRIOV_DISABLE) {
            dma_chan->err_irq = -1;
            dma_chan->done_irq = -1;
        }
        (void)devdrv_dma_chan_reset(dma_chan, (u32)sriov_flag);

        devdrv_free_dma_sq_cq(dma_chan);
    }

    if (sriov_flag == DEVDRV_SRIOV_DISABLE) {
        kfree(dma_dev);
        dma_dev = NULL;
    }
}

int agentdrv_is_remote_dma_chan(struct devdrv_dma_dev *dma_dev, u32 chan_id)
{
    u32 i;

    for (i = 0; i < dma_dev->remote_chan_num; i++) {
        if (chan_id == dma_dev->remote_chan[i]) {
            return 0;
        }
    }
    return -EINVAL;
}

STATIC int agentdrv_sriov_check_dma_idle(unsigned long idle_bitmap, u32 dma_alloc)
{
    unsigned long host_bitmap = idle_bitmap & HOST_VF_DMA_MASK;
    unsigned long dev_bitmap = idle_bitmap & DEV_VF_DMA_MASK;
    u32 host_dma_idle, dev_dma_idle;

    host_dma_idle = bitmap_weight(&host_bitmap, DEVDRV_DMA_CHAN_NUM);
    if (dma_alloc > host_dma_idle) {
        devdrv_err("Not enough idle host dma channel. (host_dma_idle=%u; dma_alloc=%u)\n", host_dma_idle, dma_alloc);
        return -EINVAL;
    }

    dev_dma_idle = bitmap_weight(&dev_bitmap, DEVDRV_DMA_CHAN_NUM);
    if (dma_alloc > dev_dma_idle) {
        devdrv_err("Not enough idle dev dma channel. (dev_dma_idle=%u; dma_alloc=%u)\n", dev_dma_idle, dma_alloc);
        return -EINVAL;
    }

    return 0;
}

STATIC int agentdrv_sriov_alloc_dma_dev(struct agentdrv_devctrl *agent_dev, u32 func_id, u32 dma_alloc)
{
    struct devdrv_res_dma_common_info rm_out = {0};
    struct agentdrv_pfvf_info pfvf_info = {0};
    struct devdrv_dma_dev *dma_dev = NULL;
    u32 dma_dev_size;

    if (agent_dev->p_agentdrv_dma_dev[func_id] != NULL) {
        devdrv_err("DMA device is already existed. (dev_id=%u; func_id=%u)\n", agent_dev->agent_id, func_id);
        return -EINVAL;
    }

    dma_dev_size = sizeof(struct devdrv_dma_dev) + sizeof(struct devdrv_dma_channel) * dma_alloc;
    dma_dev = (struct devdrv_dma_dev *)kzalloc(dma_dev_size, GFP_KERNEL);
    if (dma_dev == NULL) {
        devdrv_err("DMA device alloc failed. (dev_id=%u; func_id=%u; dma_alloc=%u)\n",
            agent_dev->agent_id, func_id, dma_alloc);
        return -ENOMEM;
    }

    dma_dev->dev_id = agent_dev->agent_id;
    dma_dev->func_id = func_id;

    devdrv_soc_func2pfvf(agent_dev->agent_id, func_id, 1, &pfvf_info);
    dma_dev->dma_pf_num = pfvf_info.pf_num;
    dma_dev->dma_vf_en = pfvf_info.vf_en;
    dma_dev->dma_vf_num = pfvf_info.vf_num;
    dma_dev->io_base = agent_dev->dma_base;
    dma_dev->dma_chan_base = agent_dev->dma_chan_base;
    dma_dev->dev = &agent_dev->pdev->dev;

    dma_dev->local_chan_num = dma_alloc;
    dma_dev->sq_cq_side = DEVDRV_DMA_LOCAL_SIDE;
    dma_dev->drvdata = (void *)agent_dev;
    dma_dev->dev_status = DEVDRV_DMA_ALIVE;

    agentdrv_dma_res_common(func_id, agent_dev->pdev->irq, &rm_out);
    dma_dev->done_irq_base = rm_out.done_irq_base;
    dma_dev->err_irq_base = rm_out.err_irq_base;
    dma_dev->err_flag = rm_out.err_flag;
    dma_dev->sq_cq_info.sq_depth = rm_out.sq_cq_info.sq_depth;
    dma_dev->sq_cq_info.sq_rsv_num = rm_out.sq_cq_info.sq_rsv_num;
    dma_dev->sq_cq_info.cq_depth = rm_out.sq_cq_info.cq_depth;

    devdrv_dma_ops_init(dma_dev, (u32)devdrv_get_chip_type());
    devdrv_dma_irq_clear(dma_dev);

    agent_dev->p_agentdrv_dma_dev[func_id] = dma_dev;
    agent_dev->p_agentdrv_msg_dev[func_id]->dma_dev = dma_dev;
    return 0;
}

STATIC void agentdrv_sriov_free_dma_dev(struct agentdrv_devctrl *agent_dev, u32 func_id)
{
    if (agent_dev->p_agentdrv_dma_dev[func_id] != NULL) {
        kfree(agent_dev->p_agentdrv_dma_dev[func_id]);
        agent_dev->p_agentdrv_dma_dev[func_id] = NULL;
    }
}

STATIC int agentdrv_sriov_init_dev_dma_chan(struct devdrv_dma_dev *dma_dev)
{
    u32 chan_id, i;
    int ret;

    for (i = 0; i < dma_dev->local_chan_num; i++) {
        chan_id = dma_dev->local_chan[i];
        dma_dev->dma_chan[i].done_irq = dma_dev->done_irq_base + chan_id;
        dma_dev->dma_chan[i].err_irq = (int)(dma_dev->err_irq_base + chan_id);
        dma_dev->dma_chan[i].err_irq_flag = (int)(dma_dev->err_flag);
        spin_lock_init(&dma_dev->dma_chan[i].lock);
        spin_lock_init(&dma_dev->dma_chan[i].cq_lock);
        ret = devdrv_dma_init_chan(dma_dev, i, chan_id, DEVDRV_SRIOV_ENABLE);
        if (ret != 0) {
            devdrv_err("DMA init channel failed. (dev_id=%u; chan=%d; ret=%d)\n", dma_dev->dev_id, chan_id, ret);
            devdrv_dma_exit(dma_dev, DEVDRV_SRIOV_ENABLE);
            return ret;
        }
    }

    return 0;
}

STATIC void agentdrv_sriov_reset_host_dma_chan(struct devdrv_dma_dev *dma_dev, u32 chan_id)
{
    void __iomem *chan_base = dma_dev->dma_chan_base + chan_id * DEVDRV_DMA_CHAN_OFFSET;
    (void)devdrv_dma_ch_cfg_reset(chan_base, DEVDRV_SRIOV_ENABLE);
    devdrv_set_dma_pfvf_num(chan_base, dma_dev->dma_pf_num, dma_dev->dma_vf_num);
}

STATIC int agentdrv_sriov_alloc_dma_chan(struct agentdrv_devctrl *agent_dev, u32 func_id, u32 dma_alloc,
    unsigned long *dma_bitmap)
{
    struct devdrv_dma_dev *dma_dev = agent_dev->p_agentdrv_dma_dev[func_id];
    u32 nbits = 1; /* handle 1 bit each time */
    u32 idx, i;
    int ret;

    bitmap_zero(dma_bitmap, BITS_PER_LONG);

    /* alloc device dma channel */
    dma_dev->local_chan_num = dma_alloc;
    idx = DMA_CHAN_LOCAL_USED_START_INDEX;
    for (i = 0; i < dma_alloc; i++) {
        idx = find_next_bit(&agent_dev->dma_idle_bitmap, DEVDRV_DMA_CHAN_NUM, idx);
        bitmap_clear(&agent_dev->dma_idle_bitmap, idx, nbits);   /* inner bitmap:  0-occupied   1-idle */
        bitmap_set(dma_bitmap, idx, nbits);                      /* output bitmap: 0-irrelevant 1-allocated */
        dma_dev->local_chan[i] = idx;
        idx++;
    }

    ret = agentdrv_sriov_init_dev_dma_chan(dma_dev);
    if (ret != 0) {
        devdrv_err("Sriov init dma channel failed. (dev_id=%u; func_id=%u; bitmap=0x%lx; ret=%d)\n",
            agent_dev->agent_id, func_id, *dma_bitmap, ret);
        idx = DMA_CHAN_LOCAL_USED_START_INDEX;
        for (i = 0; i < dma_alloc; i++) {
            idx = find_next_bit(dma_bitmap, DEVDRV_DMA_CHAN_NUM, idx);
            bitmap_set(&agent_dev->dma_idle_bitmap, idx, nbits);   /* inner bitmap:  0-occupied   1-idle */
            bitmap_clear(dma_bitmap, idx, nbits);                  /* output bitmap: 0-irrelevant 1-allocated */
            idx++;
        }
        return ret;
    }

    /* alloc host dma channel and set rmt_vf_cfg for bar space read & write */
    dma_dev->remote_chan_num = dma_alloc;
    idx = DMA_CHAN_REMOTE_USED_START_INDEX;
    for (i = 0; i < dma_alloc; i++) {
        idx = find_next_bit(&agent_dev->dma_idle_bitmap, DEVDRV_DMA_CHAN_NUM, idx);
        bitmap_clear(&agent_dev->dma_idle_bitmap, idx, nbits);    /* inner bitmap:  0-occupied   1-idle */
        bitmap_set(dma_bitmap, idx, nbits);                       /* output bitmap: 0-irrelevant 1-allocated */
        dma_dev->remote_chan[i] = idx;
        agentdrv_sriov_reset_host_dma_chan(dma_dev, idx);
        idx++;
    }

    devdrv_info("Sriov alloc dma channel success. (dev_id=%u; func_id=%u; dma_alloc=%u; bitmap=0x%lx; idle=0x%lx)\n",
                agent_dev->agent_id, func_id, dma_alloc, *dma_bitmap, agent_dev->dma_idle_bitmap);

    /* ts use channel */
    dma_dev->ts_chan_num = 1;
    dma_dev->ts_chan[0] = DMA_CHAN_TS_USED_START_INDEX + func_id;

    dma_dev->dma_bitmap = *dma_bitmap;
    devdrv_res_dma_traffic(dma_dev);

    return 0;
}

STATIC void agentdrv_sriov_free_dma_chan(struct agentdrv_devctrl *agent_dev, u32 func_id)
{
    struct devdrv_dma_dev *dma_dev = agent_dev->p_agentdrv_dma_dev[func_id];
    unsigned long dma_bitmap = dma_dev->dma_bitmap;
    u32 nbits = 1; /* handle 1 bit each time */
    u32 dma_free, idx, i;

    devdrv_info("Sriov free dma channel begin. (dev_id=%u; func_id=%u; dma_free=%u; bitmap=0x%lx; idle=0x%lx)\n",
                agent_dev->agent_id, func_id, dma_dev->local_chan_num, dma_bitmap, agent_dev->dma_idle_bitmap);

    /* alloc device dma channel */
    dma_free = dma_dev->local_chan_num;
    idx = DMA_CHAN_LOCAL_USED_START_INDEX;
    for (i = 0; i < dma_free; i++) {
        idx = find_next_bit(&dma_bitmap, DEVDRV_DMA_CHAN_NUM, idx);
        bitmap_set(&agent_dev->dma_idle_bitmap, idx, nbits);        /* inner bitmap:  0-occupied   1-idle */
        bitmap_clear(&dma_bitmap, idx, nbits);                      /* output bitmap: 0-irrelevant 1-allocated */
        idx++;
    }

    /* alloc host dma channel and set rmt_vf_cfg for bar space read & write */
    dma_free = dma_dev->remote_chan_num;
    idx = DMA_CHAN_REMOTE_USED_START_INDEX;
    for (i = 0; i < dma_free; i++) {
        idx = find_next_bit(&dma_bitmap, DEVDRV_DMA_CHAN_NUM, idx);
        bitmap_set(&agent_dev->dma_idle_bitmap, idx, nbits);        /* inner bitmap:  0-occupied   1-idle */
        bitmap_clear(&dma_bitmap, idx, nbits);                      /* output bitmap: 0-irrelevant 1-allocated */
        idx++;
    }

    dma_dev->dma_bitmap = dma_bitmap;
    devdrv_info("Sriov free dma channel success. (dev_id=%u; func_id=%u; dma_alloc=%u; bitmap=0x%lx; idle=0x%lx)\n",
                agent_dev->agent_id, func_id, dma_free, dma_bitmap, agent_dev->dma_idle_bitmap);
}


STATIC int agentdrv_sriov_init_dma_dev(struct agentdrv_devctrl *agent_dev, u32 func_id, u32 computility,
    u32 total, unsigned long *dma_bitmap)
{
    u32 vf_chan_total = DMA_CHAN_LOCAL_TOTAL_NUM - 1;
    u32 dma_alloc;
    int ret;

    if (total == 0) {
        devdrv_err("Total computility must be positive integer. (dev_id=%u; total=%u)\n", agent_dev->agent_id, total);
        return -EINVAL;
    }

    dma_alloc = (computility * vf_chan_total) / total;
    dma_alloc = (dma_alloc == 0) ? 1 : dma_alloc;

    if (agentdrv_sriov_check_dma_idle(agent_dev->dma_idle_bitmap, dma_alloc) != 0) {
        devdrv_err("Not enough idle dma channel. (dev_id=%u; func_id=%u; dma_alloc=%u)\n",
            agent_dev->agent_id, func_id, dma_alloc);
        return -EINVAL;
    }

    ret = agentdrv_sriov_alloc_dma_dev(agent_dev, func_id, dma_alloc);
    if (ret != 0) {
        devdrv_err("Sriov alloc dma dev failed. (dev_id=%u; func_id=%u; dma_alloc=%u)\n",
            agent_dev->agent_id, func_id, dma_alloc);
        return ret;
    }

    ret = agentdrv_sriov_alloc_dma_chan(agent_dev, func_id, dma_alloc, dma_bitmap);
    if (ret != 0) {
        devdrv_err("Sriov alloc dma chan failed. (dev_id=%u; func_id=%u; dma_alloc=%u)\n",
            agent_dev->agent_id, func_id, dma_alloc);
        agentdrv_sriov_free_dma_dev(agent_dev, func_id);
        return ret;
    }

    return 0;
}

STATIC void devdrv_remote_dma_chan_reset(struct devdrv_dma_dev *dma_dev, u32 sriov_flag)
{
    void __iomem *io_base = NULL;
    u32 i;

    for (i = 0; i < dma_dev->remote_chan_num; i++) {
        io_base = dma_dev->dma_chan_base + dma_dev->remote_chan[i] * DEVDRV_DMA_CHAN_OFFSET;
        (void)devdrv_dma_ch_cfg_reset(io_base, sriov_flag);
    }
}

STATIC void agentdrv_sriov_uninit_dma_dev(struct agentdrv_devctrl *agent_dev, u32 func_id)
{
    devdrv_remote_dma_chan_reset(agent_dev->p_agentdrv_dma_dev[func_id], DEVDRV_SRIOV_DISABLE);
    devdrv_dma_exit(agent_dev->p_agentdrv_dma_dev[func_id], DEVDRV_SRIOV_DISABLE);
    agent_dev->p_agentdrv_dma_dev[func_id] = NULL;
}

int agentdrv_sriov_init_dma(u32 dev_id, u32 computility, u32 total, unsigned long *dma_bitmap)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    u32 chip_id, func_id;
    int ret;

    if (devdrv_get_pfvf_type_by_devid(dev_id) != DEVDRV_SRIOV_TYPE_VF) {
        devdrv_err("Pf instance only by pcie. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    devdrv_dev2chipfunc(dev_id, &chip_id, &func_id);

    agent_dev = agentdrv_get_dev((int)chip_id);
    if (agent_dev == NULL) {
        devdrv_err("Agent_dev is NULL. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    ret = agentdrv_sriov_init_dma_dev(agent_dev, func_id, computility, total, dma_bitmap);
    if (ret != 0) {
        devdrv_err("Alloc dma channel failed. (dev_id=%u; func_id=%u)\n", agent_dev->agent_id, func_id);
        return -EINVAL;
    }

    agent_dev->shr_para[func_id]->dma_bitmap = *dma_bitmap;

    return 0;
}

void agentdrv_sriov_uninit_dma(u32 dev_id)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    u32 chip_id, func_id;

    if (devdrv_get_pfvf_type_by_devid(dev_id) != DEVDRV_SRIOV_TYPE_VF) {
        devdrv_err("Pf instance only by pcie. (dev_id=%u)\n", dev_id);
        return;
    }

    devdrv_dev2chipfunc(dev_id, &chip_id, &func_id);

    agent_dev = agentdrv_get_dev((int)chip_id);
    if (agent_dev == NULL) {
        devdrv_err("Agent_dev is NULL. (dev_id=%u)\n", dev_id);
        return;
    }

    agent_dev->shr_para[func_id]->dma_bitmap = 0;
    agentdrv_sriov_free_dma_chan(agent_dev, func_id);
    agentdrv_sriov_uninit_dma_dev(agent_dev, func_id);
}
