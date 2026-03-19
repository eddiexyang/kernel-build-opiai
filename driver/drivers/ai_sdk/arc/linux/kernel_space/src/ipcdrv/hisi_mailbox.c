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

#include <linux/wait.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/notifier.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/semaphore.h>
#include <linux/kthread.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/jiffies.h>
#include <linux/list.h>

#ifdef AOS_LLVM_BUILD
#include <linux/export.h>
#include <linux/sched.h>
#include <securec.h>
#include <securectype.h>
#else
#include <linux/sched/rt.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)) && (LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0))
#include <securec.h>
#include <securectype.h>
#else
#include <linux/securec.h>
#include <linux/securectype.h>
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 10, 0)
#include <uapi/linux/sched/types.h>
#endif
#endif

#include "hisi_mailbox.h"
#include "hisi_mailbox_dev.h"
#include "hisi_rproc_adapt.h"
#include "drv_ipc.h"
#include "drv_cpu_type.h"

#define MODULE_NAME "hisi_mailbox"

STATIC spinlock_t g_task_buffer_lock[MAX_IPCDEV_NUM];
struct hisi_mbox_task *g_TxTaskBuffer[MAX_IPCDEV_NUM] = {NULL};
/* use the g_ContinuousFailCnt to control the Continuous ipc timeout times which may overflow the kmesg log */
int g_ContinuousFailCnt = 0;
/* mailbox device resource pool */
STATIC struct list_head g_mdevices[MAX_IPCDEV_NUM] = {0};
STATIC int g_mbx_int_start_id[MAX_IPCDEV_NUM] = {0};
STATIC int g_irq_cnt[MAX_IPCDEV_NUM][HISI_RPROC_MAX] = {0};

void set_mbx_int_start_id(int dev_id, int var)
{
    if ((dev_id < 0) || (dev_id >= MAX_IPCDEV_NUM)) {
        return;
    }

    g_mbx_int_start_id[dev_id] = var;
    return;
}

int hisi_mbx_int_start_id(int dev_id)
{
    if ((dev_id < 0) || (dev_id >= MAX_IPCDEV_NUM)) {
        return 0;
    }

    return g_mbx_int_start_id[dev_id];
}
EXPORT_SYMBOL(hisi_mbx_int_start_id);

struct hisi_mbox_task *mbox_node_alloc(int ipc_id)
{
    int index = 0;
    struct hisi_mbox_task *ptask = NULL;
    unsigned long flags;

    if ((ipc_id < 0) || (ipc_id >= MAX_IPCDEV_NUM)) {
        ipcdrv_drv_err("invalid parameter, ipc_id = %d\n", ipc_id);
        return NULL;
    }

    ptask = g_TxTaskBuffer[ipc_id];
    spin_lock_irqsave(&g_task_buffer_lock[ipc_id], flags);
    for (index = 0; index < TX_TASK_DDR_NODE_NUM; index++) {
        if (ptask->tx_buffer_status == TX_TASK_DDR_NODE_VALID) {
            break;
        }
        ptask++;
    }

    if (likely(index != TX_TASK_DDR_NODE_NUM)) {
        ptask->tx_buffer_status = TX_TASK_DDR_NODE_OCCUPIED; /* set the node occupied */
    } else {
        ptask = NULL;
    }

    spin_unlock_irqrestore(&g_task_buffer_lock[ipc_id], flags);

    return ptask;
}

void mbox_task_free(int ipc_id, struct hisi_mbox_task **tx_task)
{
    unsigned long flags;

    if ((ipc_id < 0) || (ipc_id >= MAX_IPCDEV_NUM)) {
        ipcdrv_drv_err("invalid parameter, ipc_id = %d\n", ipc_id);
        return;
    }

    if ((tx_task == NULL) || (*tx_task == NULL)) {
        ipcdrv_drv_err("null pointer\n");
        return;
    }
    spin_lock_irqsave(&g_task_buffer_lock[ipc_id], flags);
    /* use the tx_buffer[0] as the available flag */
    (*tx_task)->tx_buffer_status = TX_TASK_DDR_NODE_VALID;
    spin_unlock_irqrestore(&g_task_buffer_lock[ipc_id], flags);

    return;
}

EXPORT_SYMBOL(mbox_task_free);

struct hisi_mbox_task *mbox_task_alloc(int ipc_id, const struct hisi_mbox *mbox, const char *rp_name,
    const mbox_mesg_t *tx_buffer, mbox_msg_length tx_buffer_len, int need_auto_ack)
{
    struct hisi_mbox_task *tx_task = NULL;
    int ret;

    if (rp_name == NULL) {
        ipcdrv_drv_err("null pointer rp_name!\n");
        return NULL;
    }

    if ((mbox == NULL) || (mbox->tx == NULL) || (tx_buffer == NULL)) {
        if (mbox == NULL) {
            ipcdrv_drv_err("null pointer mbox!\n");
        } else {
            ipcdrv_drv_err("%s  no tx ability or no tx_buffer\n", mbox->name);
        }
        goto out;
    }

    tx_task = mbox_node_alloc(ipc_id);
    if (tx_task == NULL) {
        ipcdrv_drv_err("tx task no mem\n");
        goto out;
    }
    ret = memcpy_s((void *)tx_task->tx_buffer, MBOX_CHAN_DATA_SIZE * (sizeof(mbox_mesg_t)), (void *)tx_buffer,
                   tx_buffer_len * (sizeof(mbox_mesg_t)));
    if (ret != 0) {
        mbox_task_free(ipc_id, &tx_task);
        tx_task = NULL;
        ipcdrv_drv_err("copy tx_buffer fail,ret=%d.\n", ret);
        goto out;
    }
    tx_task->tx_buffer_len = tx_buffer_len;
    tx_task->need_auto_ack = need_auto_ack;
    tx_task->send_timestamp  = (unsigned long)jiffies;

out:
    return tx_task;
}
EXPORT_SYMBOL(mbox_task_alloc);

STATIC int set_status(struct hisi_mbox_device *mdev, int status)
{
    unsigned long flags;
    int ret = 0;

    spin_lock_irqsave(&mdev->status_lock, flags);
    if ((MDEV_DEACTIVATED & mdev->status)) {
        spin_unlock_irqrestore(&mdev->status_lock, flags);
        ipcdrv_drv_info("an unexpected ipc caused by %s\n", mdev->name);
        ret = -ENODEV;
        goto out;
    } else {
        mdev->status |= (unsigned int)status;
    }

    spin_unlock_irqrestore(&mdev->status_lock, flags);
out:
    return ret;
}

static inline void clr_status(struct hisi_mbox_device *mdev, unsigned int status)
{
    unsigned long flags;

    spin_lock_irqsave(&mdev->status_lock, flags);
    mdev->status &= ~status;
    spin_unlock_irqrestore(&mdev->status_lock, flags);
    return;
}

STATIC void mbox_update_delay_cnt(struct mbox_proc_delay *delay_dfx, unsigned long curr_delay)
{
    if (curr_delay < MBOX_DELAY_LEVEL1) {
        delay_dfx->level1_num++;
    } else if (curr_delay < MBOX_DELAY_LEVEL2) {
        delay_dfx->level2_num++;
        delay_dfx->print_flag = 1;
    } else if (curr_delay < MBOX_DELAY_LEVEL3) {
        delay_dfx->level3_num++;
        delay_dfx->print_flag = 1;
    } else {
        delay_dfx->level4_num++;
        delay_dfx->print_flag = 1;
    }
    return;
}

STATIC void mbox_send_statistic(struct hisi_mbox_device *mdev,
    unsigned long send_timestamp, unsigned long schd_timestamp)
{
    unsigned int sched_delay, tx_delay, delay;
    unsigned long curr_timestamp = (unsigned long)jiffies;

    sched_delay = (unsigned int)jiffies_to_msecs(schd_timestamp - send_timestamp);
    mbox_update_delay_cnt(&mdev->tx_sched_delay, sched_delay);

    tx_delay = (unsigned int)jiffies_to_msecs(curr_timestamp - send_timestamp);
    mbox_update_delay_cnt(&mdev->tx_delay, tx_delay);

    delay = (unsigned int)jiffies_to_msecs(curr_timestamp - mdev->pre_tx_print_timestamp);
    if ((delay > MBOX_DELAY_STATIC_PERIOD) && (mdev->tx_delay.print_flag != 0)) {
        mdev->tx_delay.print_flag = 0;
        mdev->pre_tx_print_timestamp = curr_timestamp;
        ipcdrv_drv_event("ipc send proc statistic.(name=%s, rproc_id=%d)"
            "tx delay(0-10ms=%u, 10-50ms=%u, 50-100ms=%u, over 100ms=%u)"
            "sched delay(0-10ms=%u, 10-50ms=%u, 50-100ms=%u, over 100ms=%u)\n",
            mdev->name, (u32)mdev->rproc_id,
            mdev->tx_delay.level1_num, mdev->tx_delay.level2_num,
            mdev->tx_delay.level3_num, mdev->tx_delay.level4_num,
            mdev->tx_sched_delay.level1_num, mdev->tx_sched_delay.level2_num,
            mdev->tx_sched_delay.level3_num, mdev->tx_sched_delay.level4_num);
    }
    return;
}

STATIC int mbox_task_send_async(struct hisi_mbox_device *mdev, struct hisi_mbox_task *tx_task)
{
    int ret;
    unsigned long schd_timestamp = (unsigned long)jiffies;

    mdev->ops->ensure_channel(mdev);

    ret = mdev->ops->send(mdev, tx_task->tx_buffer, tx_task->tx_buffer_len, tx_task->need_auto_ack);
    if (ret != 0) {
        ipcdrv_drv_err("mdev %s can not be sent\n", mdev->name);
    }
    mbox_send_statistic(mdev, tx_task->send_timestamp, schd_timestamp);
    return ret;
}

int mbox_msg_send_async(struct hisi_mbox *mbox, const struct hisi_mbox_task *tx_task)
{
    struct hisi_mbox_device *mdev = NULL;
    int ret;
    unsigned long flags;

    if ((tx_task == NULL) || (mbox == NULL) || (mbox->tx == NULL)) {
        ipcdrv_drv_err("invalid parameters, (tx_task == NULL)=%d, (mbox == NULL)=%d.\n",
            (tx_task == NULL), (mbox == NULL));
        ret = -EINVAL;
        goto out;
    }
    mdev = mbox->tx;
    START_TTS(tx_task);

    /* ASYNC_ENQUEUE start */
    ret = set_status(mdev, MDEV_ASYNC_ENQUEUE);
    if (ret != 0) {
        ipcdrv_drv_warn("ret=%d, MSG{0x%08x, 0x%08x}\n", ret, tx_task->tx_buffer[0], tx_task->tx_buffer[1]);
        goto out;
    }

    /* enqueue */
    spin_lock_irqsave(&mdev->fifo_lock, flags);
    if (kfifo_avail(&mdev->fifo) < TX_FIFO_CELL_SIZE) {
        spin_unlock_irqrestore(&mdev->fifo_lock, flags);
        wake_up_interruptible(&mdev->tx_wait);
        ret = -ENOMEM;
        goto clearstatus;
    }
    kfifo_in(&mdev->fifo, &tx_task, TX_FIFO_CELL_SIZE);
    spin_unlock_irqrestore(&mdev->fifo_lock, flags);
    wake_up_interruptible(&mdev->tx_wait);

clearstatus:
    /* ASYNC_ENQUEUE end */
    clr_status(mdev, MDEV_ASYNC_ENQUEUE);
out:
    return ret;
}

EXPORT_SYMBOL(mbox_msg_send_async);

STATIC struct hisi_mbox_task *hisi_mbox_dequeue_task(struct hisi_mbox_device *mdev)
{
    struct hisi_mbox_task *tx_task = NULL;
    unsigned long flags;

    spin_lock_irqsave(&mdev->fifo_lock, flags);
    if (kfifo_len(&mdev->fifo) >= TX_FIFO_CELL_SIZE) {
        if (!kfifo_out(&mdev->fifo, &tx_task, TX_FIFO_CELL_SIZE)) {
            tx_task = NULL;
        }
    }
    spin_unlock_irqrestore(&mdev->fifo_lock, flags);
    return tx_task;
}

void mbox_empty_task(struct hisi_mbox_device *mdev)
{
    struct hisi_mbox_task *tx_task = NULL;
    unsigned long flags;

    if (mdev == NULL) {
        ipcdrv_drv_err("null pointer mdev\n");
        return;
    }

    spin_lock_irqsave(&mdev->fifo_lock, flags);
    while (kfifo_len(&mdev->fifo) >= TX_FIFO_CELL_SIZE) {
        if (kfifo_out(&mdev->fifo, &tx_task, TX_FIFO_CELL_SIZE)) {
            mbox_task_free(mdev->ipc_id, &tx_task);
        }
    }
    spin_unlock_irqrestore(&mdev->fifo_lock, flags);

    mutex_lock(&mdev->dev_lock);
    /* do nothing here just to wait for the already-kfifo-out's tx_task finish */
    mutex_unlock(&mdev->dev_lock);
}

EXPORT_SYMBOL(mbox_empty_task);

STATIC int mbox_tx_thread(void *context)
{
    struct hisi_mbox_device *mdev = (struct hisi_mbox_device *)context;
    struct hisi_mbox_task *tx_task = NULL;
    int ret;

    while (!kthread_should_stop()) {
        /*lint -e666*/
        ret = wait_event_interruptible(mdev->tx_wait,
                                       ((mdev->thread_flag == 1) || (kfifo_len(&mdev->fifo) >= TX_FIFO_CELL_SIZE)));
        /*lint +e666*/
        if (unlikely(ret)) {
            ipcdrv_drv_err("ret=%d, wait event failed.\n", ret);
            continue;
        }

        /* kick out the async send request from	mdev's kfifo one by one and send it out */
        while ((tx_task = hisi_mbox_dequeue_task(mdev))) {
            mutex_lock(&mdev->dev_lock);
            ret = mbox_task_send_async(mdev, tx_task);
            PRINT_TTS(tx_task);
            mbox_task_free(mdev->ipc_id, &tx_task);
            mutex_unlock(&mdev->dev_lock);
        }
    }
    return 0;
}

STATIC void mbox_recv_statistic(struct hisi_mbox_device *mdev,
    unsigned long irq_timestamp, unsigned long call_timestamp, unsigned long curr_timestamp)
{
    unsigned int delay;
    unsigned int call_delay;
    unsigned int bh_delay;

    /* interrrupt -> bh delay */
    bh_delay = (unsigned int)jiffies_to_msecs(curr_timestamp - irq_timestamp);
    mbox_update_delay_cnt(&mdev->rx_sched_delay, bh_delay);

    /* call back delay */
    call_delay = (unsigned int)jiffies_to_msecs(curr_timestamp - call_timestamp);
    mbox_update_delay_cnt(&mdev->rx_callback_delay, call_delay);

    delay = (unsigned int)jiffies_to_msecs(curr_timestamp - mdev->pre_rx_print_timestamp);
    if ((delay > MBOX_DELAY_STATIC_PERIOD) && (mdev->rx_sched_delay.print_flag != 0)) {
        mdev->rx_sched_delay.print_flag = 0;
        mdev->pre_rx_print_timestamp = curr_timestamp;
        ipcdrv_drv_event("ipc recv irq proc statistic.(name=%s, rproc_id=%d, fifo_full_cnt=%d)"
            "bh delay(0-10ms=%u, 10-50ms=%u, 50-100ms=%u, over 100ms=%u)"
            "call delay(0-10ms=%u, 10-50ms=%u, 50-100ms=%u, over 100ms=%u)\n",
            mdev->name, (u32)mdev->rproc_id, mdev->rx_fifo_full_cnt,
            mdev->rx_sched_delay.level1_num, mdev->rx_sched_delay.level2_num,
            mdev->rx_sched_delay.level3_num, mdev->rx_sched_delay.level4_num,
            mdev->rx_callback_delay.level1_num, mdev->rx_callback_delay.level2_num,
            mdev->rx_callback_delay.level3_num, mdev->rx_callback_delay.level4_num);
    }

    return;
}

STATIC struct ipcdrv_recv_data* get_data_from_fifo(struct hisi_mbox_device *mdev)
{
    struct ipcdrv_recv_data *recv_data = NULL;
    unsigned long flags;

    if (mdev == NULL) {
        ipcdrv_drv_err("null pointer mdev\n");
        return NULL;
    }

    spin_lock_irqsave(&mdev->recv_data.spinlock, flags);
    if (!list_empty_careful(&mdev->recv_data.recv_data_list)) {
        recv_data = list_first_entry(&mdev->recv_data.recv_data_list, struct ipcdrv_recv_data, list);
        list_del(&recv_data->list);
        mdev->recv_data.recv_data_num--;
    }
    spin_unlock_irqrestore(&mdev->recv_data.spinlock, flags);

    return recv_data;
}

STATIC void mbox_rx_bh(unsigned long context)
{
    struct hisi_mbox_device *mdev = (struct hisi_mbox_device *)(uintptr_t)context;
    struct ipcdrv_recv_data *recv_data = NULL;
    unsigned long call_timestamp, irq_timestamp, end_timestamp;

    while ((recv_data = get_data_from_fifo(mdev)) != NULL) {
        /* @stub_ipc_drv_03 */
        /* @stub_ipc_drv_03_TAIL */
        irq_timestamp = recv_data->irq_timestamp;
        call_timestamp  = (unsigned long)jiffies;
        (void)atomic_notifier_call_chain(&mdev->notifier, recv_data->size, (void *)recv_data->buffer);
        kfree(recv_data);
        recv_data = NULL;
        end_timestamp = (unsigned long)jiffies;
        if (mdev->enable_irq_stage == ENABLE_IRQ_IN_BH) {
            mdev->enable_irq_stage = ENABLE_IRQ_IN_INTERRUPT;
            mdev->ops->enable_irq(mdev);
        }
        mbox_recv_statistic(mdev, irq_timestamp, call_timestamp, end_timestamp);
    }

    return;
}

STATIC int add_data_to_fifo(struct hisi_mbox_device *mdev, const mbox_mesg_t* data, mbox_msg_length len)
{
    struct ipcdrv_recv_data *recv_data = NULL;
    unsigned long flags;
    int ret;
    unsigned long curr_time = (unsigned long)jiffies;

    if (len * sizeof(mbox_mesg_t) != MAX_BUFFER_LEN) {
        ipcdrv_drv_err("the size is err, len(%u)\n", len);
        return -EINVAL;
    }

    recv_data = kzalloc(sizeof(struct ipcdrv_recv_data), GFP_ATOMIC);
    if (recv_data == NULL) {
        ipcdrv_drv_err("alloc recv_data node fail.\n");
        return -ENOMEM;
    }
    ret = memcpy_s(recv_data->buffer, MAX_BUFFER_LEN, data, len * sizeof(mbox_mesg_t));
    if (ret != 0) {
        kfree(recv_data);
        ipcdrv_drv_err("memcpy fail\n");
        return -ENODEV;
    }
    recv_data->size = len;
    recv_data->irq_timestamp = curr_time;

    spin_lock_irqsave(&mdev->recv_data.spinlock, flags);
    if (mdev->recv_data.recv_data_num >= (MAX_FIFO_LEN - 1)) {
        mdev->enable_irq_stage = ENABLE_IRQ_IN_BH;
        mdev->rx_fifo_full_cnt++;
    }
    list_add_tail(&recv_data->list, &mdev->recv_data.recv_data_list);
    mdev->recv_data.recv_data_num++;
    spin_unlock_irqrestore(&mdev->recv_data.spinlock, flags);

    return 0;
}

STATIC int mbox_interrupt(int irq, void *p)
{
    mbox_mesg_t *rx_buffer = NULL;
    mbox_msg_length rx_len = 0;
    int rproc_id;
    int ipc_id;

    struct hisi_mbox_device *mdev = (struct hisi_mbox_device *)p;
    /* @stub_ipc_drv_02 */
    /* @stub_ipc_drv_02_TAIL */
    if (mdev == NULL) {
        ipcdrv_drv_err("Mdev is null. (irq=%d)\n", irq);
        return IRQ_NONE;
    }
    ipc_id = mdev->ipc_id;
    rproc_id = (int)mdev->rproc_id;

    if (g_irq_cnt[ipc_id][rproc_id]++ > IRQ_IRQ_NONE_RETRY_TIME) {
        disable_irq_nosync((unsigned int)irq);
        g_irq_cnt[ipc_id][rproc_id] = 0;
        return IRQ_HANDLED;
    }

    mdev = mdev->ops->irq_to_mdev(mdev, &g_mdevices[ipc_id], irq);
    if (mdev == NULL) {
        ipcdrv_drv_warn("An unexpected interrupt occured. (irq=%d, ipc_id=%d, rproc_id=%d)\n", irq, ipc_id, rproc_id);
        return IRQ_NONE;
    }

    g_irq_cnt[ipc_id][rproc_id] = 0;
    if (mdev->configured == 0) {
        ipcdrv_drv_err("Mdev %s has not startup yet\n", mdev->name);
        return IRQ_NONE;
    }

    /* ipc rcv */
    if ((mdev->ops->is_stm(mdev, DESTINATION_STATUS)) != 0) {
        disable_irq_nosync((unsigned int)irq);
        mdev->cur_irq = irq;
        mdev->enable_irq_stage = ENABLE_IRQ_IN_INTERRUPT;

        rx_len = mdev->ops->recv(mdev, &rx_buffer);
        (void)add_data_to_fifo(mdev, rx_buffer, rx_len);
        mdev->ops->ack(mdev, NULL, 0);

        wmb();

        tasklet_schedule(&mdev->rx_bh);
        if (mdev->enable_irq_stage == ENABLE_IRQ_IN_INTERRUPT) {
            mdev->ops->enable_irq(mdev);
        }
        return IRQ_HANDLED;
    }

    /* ack */
    if ((mdev->ops->is_stm(mdev, ACK_STATUS)) != 0) {
        mdev->ops->clr_ack(mdev);
        mdev->ops->refresh(mdev);
    } else {
        /* an unexpected interrupt status */
        ipcdrv_drv_err("mdev %s rproc_id=%d an unexpected interrupt status\n", mdev->name, rproc_id);
    }

    return IRQ_HANDLED;
}

STATIC void hisi_mbox_shutdown(struct hisi_mbox_device *mdev, mbox_mail_type_t mail_type)
{
    if (!--mdev->configured) {
        mdev->ops->free_irq(mdev, (void *)mdev);
        mdev->ops->shutdown(mdev);
        switch (mail_type) {
            case TX_MAIL:
                mdev->thread_flag = 1;
                wake_up_interruptible(&mdev->tx_wait);
                (void)kthread_stop(mdev->tx_kthread);
                kfifo_free(&mdev->fifo);
                tasklet_kill(&mdev->rx_bh);
                break;
            case RX_MAIL:
                tasklet_kill(&mdev->rx_bh);
                break;
            default:
                break;
        }
    }
    return;
}

static void hisi_mbox_free(struct hisi_mbox **mbox)
{
    struct hisi_mbox *_mbox = *mbox;

    kfree(_mbox->name);
    _mbox->name = NULL;

    kfree(_mbox);
    _mbox = NULL;

    *mbox = _mbox;
    return;
}

void hisi_mbox_put(struct hisi_mbox **mbox)
{
    struct hisi_mbox *_mbox = NULL;
    struct hisi_mbox_device *mdev[MAIL_TYPE_MAX] = {NULL};
    int i;

    if (mbox == NULL) {
        return;
    }

    _mbox = *mbox;
    if (_mbox == NULL) {
        return;
    }

    /* tx & rx mailbox devices deinit */
    mdev[TX_MAIL] = _mbox->tx;
    mdev[RX_MAIL] = _mbox->rx;

    for (i = TX_MAIL; i < MAIL_TYPE_MAX; i++) {
        if (mdev[i]) {
            hisi_mbox_shutdown(mdev[i], (mbox_mail_type_t)i);
        }
    }

    if (mdev[RX_MAIL] && _mbox->nb) {
        (void)atomic_notifier_chain_unregister(&mdev[RX_MAIL]->notifier, _mbox->nb);
    }

    hisi_mbox_free(mbox);
    return;
}
EXPORT_SYMBOL(hisi_mbox_put);

STATIC void mbox_ts_tx_bind_cpu(struct hisi_mbox_device *mdev)
{
#ifdef CFG_SOC_PLATFORM_MINIV2_MDC
#ifndef AOS_LLVM_BUILD
    static struct cpumask g_cpumask_var;
    int ret;

    ret = drv_get_ctrlcpu_mask(&g_cpumask_var);
    if (ret) {
        ipcdrv_drv_err("get cpulist buf failed. (ret = %d)\n", ret);
        return;
    }

    if (mdev->ops->check(mdev, TX_MAIL, IPC_TSC_MBX1_RPID0_TX_ACPU0) != RPUNACCESSIBLE) {
        kthread_bind(mdev->tx_kthread, IPC_TX_TS_BIND_CPU_ID);
    } else if (cpumask_available(&g_cpumask_var)) {
        set_cpus_allowed_ptr(mdev->tx_kthread, &g_cpumask_var);
    }
#else
    if (mdev->ops->check(mdev, TX_MAIL, IPC_TSC_MBX1_RPID0_TX_ACPU0) != RPUNACCESSIBLE) {
        kthread_bind(mdev->tx_kthread, IPC_TX_TS_BIND_CPU_ID);
    }
#endif
#endif
    return;
}

STATIC int mbox_startup(struct hisi_mbox_device *mdev, mbox_mail_type_t mail_type)
{
    int ret = 0;
    unsigned int tx_buff = 0;

    mutex_lock(&mdev->dev_lock);
    if (!mdev->configured++) {
        switch (mail_type) {
            case TX_MAIL:
                tx_buff = MAILBOX_MAX_TX_BUFFER;
                if (kfifo_alloc(&mdev->fifo, tx_buff, GFP_KERNEL)) {
                    ipcdrv_drv_err("tx mdev %s alloc kfifo failed\n", mdev->name);
                    ret = -ENOMEM;
                    goto deconfig;
                }
                init_waitqueue_head(&mdev->tx_wait);
                /* create the async tx thread */
                mdev->tx_kthread = kthread_create(mbox_tx_thread, mdev, "%s", mdev->name);
                if (unlikely(IS_ERR(mdev->tx_kthread))) {
                    ipcdrv_drv_err("create kthread tx_kthread failed!\n");
                    ret = -EINVAL;
                    kfifo_free(&mdev->fifo);
                    goto deconfig;
                } else {
                    /*
                     * External modules cannot call sched_setscheduler() on
                     * 6.18 because it is no longer exported. Use the
                     * exported FIFO helper to keep the TX worker in RT class.
                     */
                    sched_set_fifo(mdev->tx_kthread);

                    mbox_ts_tx_bind_cpu(mdev);
                    (void)wake_up_process(mdev->tx_kthread);
                }
                tasklet_init(&mdev->rx_bh, mbox_rx_bh, (unsigned long)((uintptr_t)mdev));
                break;
            case RX_MAIL:
                tasklet_init(&mdev->rx_bh, mbox_rx_bh, (unsigned long)((uintptr_t)mdev));
                break;
            default:
                ret = -EINVAL;
                goto shutdown;
        }

#ifdef AOS_LLVM_BUILD
        ret = mdev->ops->hisi_request_irq(mdev, (irq_handler_t)mbox_interrupt);
#else
        ret = mdev->ops->request_irq(mdev, (irq_handler_t)mbox_interrupt);
#endif
        if (ret != 0) {
            ipcdrv_drv_err("ret=%d, mdev %s request irq failed.\n", ret, mdev->name);
            ret = -ENODEV;
            goto shutdown;
        }
    }

    mutex_unlock(&mdev->dev_lock);
    return ret;

shutdown:
    mdev->ops->shutdown(mdev);

    switch (mail_type) {
        case TX_MAIL:
            (void)kthread_stop(mdev->tx_kthread);
            kfifo_free(&mdev->fifo);
            tasklet_kill(&mdev->rx_bh);
            break;
        case RX_MAIL:
            tasklet_kill(&mdev->rx_bh);
            break;
        default:
            break;
    }

deconfig:
    mdev->configured--;
    mutex_unlock(&mdev->dev_lock);
    return ret;
}

STATIC struct hisi_mbox *mbox_alloc(struct hisi_mbox_device *tx_mdev, struct hisi_mbox_device *rx_mdev,
    const char *rp_name)
{
    struct hisi_mbox *mbox = NULL;
    char *mbox_name = NULL;
    size_t len = strlen("->") + strlen(rp_name);
    int tlen;

    mbox_name = kzalloc(sizeof(char) * (len + 1), GFP_KERNEL);
    if (mbox_name == NULL) {
        ipcdrv_drv_err("no memory for mbox name <->%s>\n", rp_name);
        goto out;
    }
    tlen = snprintf_s(mbox_name, len + 1, len, "->%s", rp_name);
    if (tlen < 0) {
        ipcdrv_drv_err("tlen=%d, mbox_alloc: ->%s snprintf_s fail.\n", tlen, rp_name);
        kfree(mbox_name);
        mbox_name = NULL;
        goto out;
    }

    mbox = kzalloc(sizeof(*mbox), GFP_KERNEL);
    if (mbox == NULL) {
        ipcdrv_drv_err("no memory for mbox <%s>\n", mbox_name);
        kfree(mbox_name);
        mbox_name = NULL;
        goto out;
    }

    mbox->name = (const char *)mbox_name;
    mbox->tx = tx_mdev;
    mbox->rx = rx_mdev;
    mbox->nb = NULL;

out:
    return mbox;
}

/*
 * search a certain mdev,
 * which could access the remote proccessor, "rp_name", in the list.
 */
STATIC struct hisi_mbox_device *hisi_mbox_device_get(struct list_head *list, mbox_mail_type_t mtype,
                                                     const char *rp_name)
{
    struct hisi_mbox_device *mdev = NULL;
    struct hisi_mbox_device *_mdev = NULL;

    list_for_each_entry(_mdev, list, node)
    {
        /*
         * though remote processor of mailboxes could be uncertain,
         * when mailboxes prepare for use, it won't be an uncertain one
         * or an unaccessible one, when a tx task of mailbox is alloced.
         *
         * the regulation is compatible for some mailboxes, whose remote
         * processor could not be assigned, until mailboxes is used
         * in an ipc.
         */
        if (_mdev->ops->check(_mdev, mtype, rp_name) != RPUNACCESSIBLE) {
            mdev = _mdev;
            mdev->mbox_type = (int)mtype;
            break;
        }
    }

    return mdev;
}

struct hisi_mbox *mbox_get(int ipc_id, const char *rp_name, struct notifier_block *nb)
{
    struct hisi_mbox_device *mdev[MAIL_TYPE_MAX] = {NULL};
    struct hisi_mbox *mbox = NULL;
    struct list_head *list = NULL;
    int i;
    unsigned int checker = 0;

    if ((ipc_id < 0) || (ipc_id >= MAX_IPCDEV_NUM)) {
        ipcdrv_drv_err("invalid parameter, ipc_id = %d\n", ipc_id);
        return NULL;
    }

    list = &g_mdevices[ipc_id];
    if (list_empty(list)) {
        ipcdrv_drv_err("mailboxes not ready\n");
        goto out;
    }

    if (rp_name == NULL) {
        ipcdrv_drv_err("null pointer\n");
        goto out;
    }

    mdev[TX_MAIL] = hisi_mbox_device_get(list, TX_MAIL, rp_name);
    mdev[RX_MAIL] = hisi_mbox_device_get(list, RX_MAIL, rp_name);

    checker = (mdev[TX_MAIL] == NULL) && (mdev[RX_MAIL] == NULL);
    if (checker) {
        ipcdrv_drv_err("ipcid:%d,|%s| neither tx nor rx mboxes were gotten, may get later\n", ipc_id, rp_name);
        goto out;
    }

    for (i = TX_MAIL; i < MAIL_TYPE_MAX; i++) {
        checker = (i == RX_MAIL) && mdev[i] && (nb != NULL);
        if (checker) {
            (void)atomic_notifier_chain_register(&mdev[i]->notifier, nb);
        }

        checker = (mdev[i] != NULL) && mbox_startup(mdev[i], (mbox_mail_type_t)i);
        if (checker) {
            ipcdrv_drv_err("%s mdev %s startup failed\n", ((i == TX_MAIL) ? "tx" : "rx"), mdev[i]->name);
            goto shutdown;
        }
    }

    mbox = mbox_alloc(mdev[TX_MAIL], mdev[RX_MAIL], rp_name);
    if (mbox == NULL) {
        ipcdrv_drv_err("failed to alloc mbox\n");
        goto shutdown;
    }
    mbox->nb = nb;

    return mbox;

shutdown:
    if ((i == RX_MAIL) && (mdev[i] != NULL) && (nb != NULL)) {
        (void)atomic_notifier_chain_unregister(&mdev[i]->notifier, nb);
    }
    while (i--) {
        if (mdev[i] != NULL) {
            hisi_mbox_shutdown(mdev[i], (mbox_mail_type_t)i);
        }
    }
out:

    return mbox;
}
EXPORT_SYMBOL(mbox_get);

void mbox_device_deactivate(struct hisi_mbox_device **list)
{
    struct hisi_mbox_device *mdev = NULL;
    struct hisi_mbox_task *tx_task = NULL;
    int i, ret;

    if (list == NULL) {
        ipcdrv_drv_err("no mboxes registered\n");
        return;
    }

    for (i = 0; (mdev = list[i]); i++) {
        ret = set_status(mdev, MDEV_DEACTIVATED);
        if (ret != 0) {
            ipcdrv_drv_err("ret=%d, mbox_device_deactivate error!\n", ret);
            return;
        }
        if (!mdev->configured) {
            mutex_unlock(&mdev->dev_lock);
            continue;
        }

        /* flush tx work & tx task list synchronously */
        while ((tx_task = hisi_mbox_dequeue_task(mdev))) {
            mbox_task_free(mdev->ipc_id, &tx_task);
        }
#ifndef CFG_SOC_PLATFORM_MINIV3
        mdev->ops->free_irq(mdev, (void *)mdev);
#endif
    }

    return;
}
EXPORT_SYMBOL(mbox_device_deactivate);

void mbox_device_activate(struct hisi_mbox_device **list)
{
    struct hisi_mbox_device *mdev = NULL;
    struct hisi_mbox_device_priv *priv = NULL;
    int i;
#ifndef CFG_SOC_PLATFORM_MINIV3
    int ret;
#endif
    if (list == NULL) {
        ipcdrv_drv_err("no mboxes registered\n");
        return;
    }

    for (i = 0; (mdev = list[i]); i++) {
        clr_status(mdev, MDEV_DEACTIVATED);
        priv = mdev->priv;
#ifdef CFG_SOC_PLATFORM_MINIV3
        if ((mdev->mbox_type == RX_MAIL) && (mdev->ops->is_stm(mdev, DESTINATION_STATUS) != 0)) {
            mdev->ops->ack(mdev, NULL, 0);
        }
        mbox_clear_mbix_eoi_irq((u32)priv->irq);
#else
#ifdef AOS_LLVM_BUILD
        ret = mdev->ops->hisi_request_irq(mdev, (irq_handler_t)mbox_interrupt);
#else
        ret = mdev->ops->request_irq(mdev, (irq_handler_t)mbox_interrupt);
#endif
        if (ret) {
            ipcdrv_drv_err("ret=%d, mdev %s request irq failed.\n", ret, mdev->name);
        }
#endif
    }
    return;
}
EXPORT_SYMBOL(mbox_device_activate);

static struct class *g_hisi_mbox_class;

int mbox_device_unregister(struct hisi_mbox_device **list)
{
    struct hisi_mbox_device *mdev = NULL;
    int i;

    if (list == NULL) {
        ipcdrv_drv_err("no mboxes registered\n");
        return -EINVAL;
    }

    for (i = 0; (mdev = list[i]); i++) {
        mutex_destroy(&mdev->dev_lock);
        device_destroy(g_hisi_mbox_class, (dev_t)i);
        list_del(&mdev->node);
    }

    list = NULL;

    return 0;
}

EXPORT_SYMBOL(mbox_device_unregister);

static int hisi_mbox_device_is_valid(const struct hisi_mbox_device *mdev)
{
    if ((mdev == NULL) || (mdev->ops == NULL) || (mdev->ops->startup == NULL) ||
                          (mdev->ops->shutdown == NULL) || (mdev->ops->check == NULL) || (mdev->ops->recv == NULL) ||
                          (mdev->ops->send == NULL) || (mdev->ops->refresh == NULL) ||
#ifdef AOS_LLVM_BUILD
                          (mdev->ops->hisi_request_irq == NULL) ||
#else
                          (mdev->ops->request_irq == NULL) ||
#endif
                          (mdev->ops->free_irq == NULL) ||
                          (mdev->ops->irq_to_mdev == NULL) || (mdev->ops->is_stm == NULL)) {
        return 0;
    }

    return 1;
}

int mbox_device_register(int ipc_id, struct device *parent, struct hisi_mbox_device **list)
{
    struct hisi_mbox_device *mdev = NULL;
    unsigned long flags;
    int ret, i;

    if ((list == NULL) || (ipc_id < 0) || (ipc_id >= MAX_IPCDEV_NUM)) {
        ipcdrv_drv_err("null pointer or ipc id(%d) invalid\n", ipc_id);
        return -EINVAL;
    }

    for (i = 0; (mdev = list[i]); i++) {
        if (!hisi_mbox_device_is_valid(mdev)) {
            ipcdrv_drv_err("invalid mdev\n");
            ret = -EINVAL;
            goto err_out;
        }
        spin_lock_init(&mdev->fifo_lock);
        spin_lock_init(&mdev->status_lock);
        spin_lock_init(&mdev->recv_data.spinlock);
        mutex_init(&mdev->dev_lock);

        mdev->recv_data.recv_data_num = 0;
        INIT_LIST_HEAD(&mdev->recv_data.recv_data_list);

        ATOMIC_INIT_NOTIFIER_HEAD(&mdev->notifier);

        spin_lock_irqsave(&mdev->status_lock, flags);
        mdev->status = 0;
        spin_unlock_irqrestore(&mdev->status_lock, flags);

        list_add_tail(&mdev->node, &g_mdevices[ipc_id]);
    }

    return 0;

err_out:
    while (i--) {
        mdev = list[i];
        list_del(&mdev->node);
        mutex_destroy(&mdev->dev_lock);
    }
    return ret;
}

EXPORT_SYMBOL(mbox_device_register);

int mbox_init(void)
{
    int i;

    g_hisi_mbox_class = class_create("hisi-mailbox");
    if (IS_ERR(g_hisi_mbox_class)) {
        return (int)PTR_ERR(g_hisi_mbox_class);
    }

    for (i = 0; i < MAX_IPCDEV_NUM; i++) {
        g_mbx_int_start_id[i] = 0;
        INIT_LIST_HEAD(&g_mdevices[i]);
        spin_lock_init(&g_task_buffer_lock[i]);
    }

    ipcdrv_drv_info("ipcdrv init success\n");
    return 0;
}

void hisi_mbox_exit(void)
{
    if (g_hisi_mbox_class) {
        class_destroy(g_hisi_mbox_class);
    }

    return;
}

int mbox_is_exist_by_rp(int ipc_id, const char *rp_name)
{
    struct list_head *list = NULL;
    struct hisi_mbox_device *mdev[MAIL_TYPE_MAX] = {NULL};
    unsigned int checker = 0;

    list = &g_mdevices[ipc_id];
    if (rp_name == NULL) {
        ipcdrv_drv_err("null pointer\n");
        return -1;
    }

    mdev[TX_MAIL] = hisi_mbox_device_get(list, TX_MAIL, rp_name);
    mdev[RX_MAIL] = hisi_mbox_device_get(list, RX_MAIL, rp_name);

    checker = (mdev[TX_MAIL] == NULL) && (mdev[RX_MAIL] == NULL);
    if (checker != 0) {
        return -1;
    }

    return 0;
}
