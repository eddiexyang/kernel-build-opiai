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
#include <linux/slab.h>
#include <linux/securec.h>
#include <linux/securectype.h>
#include <linux/irq.h>
#include <linux/uio_driver.h>
#include <linux/atomic.h>
#include <linux/delay.h>
#ifndef TSDRV_UT

#include "ts_api.h"
#include "ts_aisle_queue.h"
#include "ts_aisle_api.h"
#include "hwts_drv_api.h"
#include "ascend_hal_define.h"
#include "event_sched_inner.h"
#include "devdrv_manager_comm.h"
#include "devdrv_functional_cqsq_api.h"
#include "hwts_task_info.h"
#include "task_struct.h"

#define WAIT_QUEUE_NOT_FULL_TIME_OUT 1000
#define WAIT_QUEUE_NOT_FULL_CNT 1000000

aicpu_to_ts_msg_cb g_aicpu_to_ts_cb = { 0 };
struct tasklet_struct g_msg_wakeup_task;
static unsigned long g_wait_queue_time_out = 0;
static inline int check_queue_empty(const aicpu_to_ts_msg_queue_cb *queue)
{
    aicpu_to_ts_queue *q = queue->queue;
    if (q->head == q->tail) {
        return true;
    }
    return false;
}

static inline int check_queue_full(const aicpu_to_ts_msg_queue_cb *queue)
{
    aicpu_to_ts_queue *q = queue->queue;
    if (((q->tail + 1) % (queue->queue_depth)) == q->head) {
        return true;
    }
    return false;
}

STATIC int pull_data_from_queue(aicpu_to_ts_msg_queue_cb *queue, void *out_buf, unsigned short buf_len)
{
    int ret;
    aicpu_to_ts_queue *q = queue->queue;
    unsigned long long queue_head = (unsigned long long)(uintptr_t)q->data;
    queue_head = (unsigned long long)queue_head +
        (unsigned long long)((unsigned int)q->head * (unsigned int)queue->queue_entry_length);

    if (check_queue_empty(queue)) {
        return false;
    }
    ret = memcpy_s((void *)out_buf, buf_len, (void *)(uintptr_t)queue_head, queue->queue_entry_length);
    if (ret != 0) {
        ts_drv_err("memcpy_s failed:%d\n", ret);
        return false;
    }

    mb();

    q->head = ((q->head + 1) % (queue->queue_depth));

    return true;
}

STATIC int push_data_to_queue(aicpu_to_ts_msg_queue_cb *queue, const void *in_buf, unsigned short buf_len)
{
#define USLEEP_MIN 1
#define USLEEP_MAX 2

    int ret;
    aicpu_to_ts_queue *q = queue->queue;
    int wait_cnt = 0;
    unsigned long long queue_tail = (unsigned long long)(uintptr_t)q->data;
    /* check input data length */
    if (buf_len > queue->queue_entry_length) {
        ts_drv_err("input data to long,buf_len:%u,entry_length:%u\n", (u32)buf_len,
            (u32)queue->queue_entry_length);
        return false;
    }

    /* wait queue not full */
    do {
        if (check_queue_full(queue) == false) {
            break;
        }
        if (wait_cnt++ > WAIT_QUEUE_NOT_FULL_CNT) {
            ts_drv_err("queue full,head:%u,tail:%u\n", (u32)q->head, (u32)q->tail);
            return false;
        }
        usleep_range(USLEEP_MIN, USLEEP_MAX);
    } while (1);

    /* copy data to queue */
    queue_tail = (unsigned long long)queue_tail +
        (unsigned long long)((unsigned int)q->tail * (unsigned int)queue->queue_entry_length);
    ret = memcpy_s((void *)(uintptr_t)queue_tail, queue->queue_entry_length, (void *)(uintptr_t)in_buf, buf_len);
    if (ret != 0) {
        ts_drv_err("memcpy_s failed:%d\n", ret);
        return false;
    }

    wmb();

    q->tail = ((q->tail + 1) % (queue->queue_depth));
    wmb();

    return true;
}

int send_msg_to_ts_async(int dev_id, int ts_id, unsigned int len, const void *msg)
{
    int ret;

    if ((msg == NULL) || (len == 0)) {
        ts_drv_err("in_buf failed, dev_id(%d), ts_id(%d), len(%u)\n", dev_id, ts_id, len);
        return TS_PARA_ERR;
    }

    ret = down_timeout(&g_aicpu_to_ts_cb.sq.queue_sema, g_wait_queue_time_out);
    if (ret != 0) {
        ts_drv_err("down_timeout, dev_id(%d), ts_id(%d),head:%u,tail:%u\n",
            dev_id, ts_id, (u32)g_aicpu_to_ts_cb.sq.queue->head, (u32)g_aicpu_to_ts_cb.sq.queue->tail);
        return -EAGAIN;
    }
    if (push_data_to_queue(&(g_aicpu_to_ts_cb.sq), msg, (unsigned short)len) == false) {
        up(&g_aicpu_to_ts_cb.sq.queue_sema);
        ts_drv_err("push_data_to_queue failed, dev_id(%d), ts_id(%d)\n", dev_id, ts_id);
        return -EDOM;
    }
    /* send irq to ts */
    if (g_aicpu_to_ts_cb.sq.proc != NULL) {
        g_aicpu_to_ts_cb.sq.proc(g_aicpu_to_ts_cb.sq.irq);
    }
    up(&g_aicpu_to_ts_cb.sq.queue_sema);

    atomic64_inc(&g_aicpu_to_ts_cb.v_sq_tx);
    return TS_INNER_SUCCESS;
}

STATIC int tsdrv_send_ack_msg(unsigned int devid, unsigned int subevent_id, const char *msg,
    unsigned int msg_len, void *priv)
{
    ts_aicpu_sqe_t sqe_msg;
    struct hwts_response *resp = NULL;

    if ((msg == NULL) || (msg_len == 0)) {
        ts_drv_err("Msg para error. (msg_len=%u)\n", msg_len);
        return TS_PARA_ERR;
    }

    resp = (struct hwts_response *)msg;
    if ((resp->msg == NULL) || (resp->len == 0) || (resp->len > sizeof(ts_aicpu_sqe_t))) {
        ts_drv_err("Msg para error. (len=%d)\n", resp->len);
        return TS_PARA_ERR;
    }

    if (copy_from_user((void *)&sqe_msg, (void *)resp->msg, resp->len) != 0) {
        ts_drv_err("Copy msg from user error. (len=%d)\n", resp->len);
        return TS_COPY_USER_ERR;
    }

    return send_msg_to_ts_async(0, 0, resp->len, (void *)&sqe_msg);
}

STATIC void create_msg_chan_to_ts(aicpu_to_ts_msg_cb *msg_cb)
{
    unsigned char buff[AICPU_TO_TS_MSG_MAILBOX_LENGTH] = { 0 };
    struct devdrv_mailbox_create_ts_com *data = (struct devdrv_mailbox_create_ts_com *)buff;
    data->valid = AICPU_TO_TS_MAILBOX_VALID;
    data->cmd_type = AICPU_TO_TS_CREATE_CHANNEL;
    data->cmd.channel_id = msg_cb->channel_id;
    data->cmd.sq_addr = msg_cb->sq.pa;
    data->cmd.cq_addr = msg_cb->cq.pa;
    data->cmd.sqe_length = msg_cb->sq.queue_entry_length;
    data->cmd.cqe_length = msg_cb->cq.queue_entry_length;
    data->cmd.sq_depth = msg_cb->sq.queue_depth;
    data->cmd.cq_depth = msg_cb->cq.queue_depth;
    /* call send cmd to ts */
    (void)devdrv_mailbox_send_cqsq(0, 0, (struct devdrv_mailbox_cqsq *)data);
}

STATIC void destory_msg_chan_to_ts(aicpu_to_ts_msg_cb *msg_cb)
{
    unsigned char buff[AICPU_TO_TS_MSG_MAILBOX_LENGTH] = { 0 };
    struct devdrv_mailbox_destory_ts_com *data = (struct devdrv_mailbox_destory_ts_com *)buff;
    data->valid = AICPU_TO_TS_MAILBOX_VALID;
    data->cmd_type = AICPU_TO_TS_DESTORY_CHANNEL;
    data->channel_id = msg_cb->channel_id;

    /* call send cmd to ts */
    (void)devdrv_mailbox_send_cqsq(0, 0, (struct devdrv_mailbox_cqsq *)data);
}


STATIC void ts_queue_release(aicpu_to_ts_msg_cb *msg_cb)
{
    if (msg_cb == NULL) {
        ts_drv_err("msg_cb is null\n");
        return;
    }
    /* free irq */
    devm_free_irq(&msg_cb->pdev->dev, msg_cb->cq.irq, msg_cb);
    tasklet_kill(&msg_cb->wakeup_task);
    destory_msg_chan_to_ts(msg_cb);
    iounmap(msg_cb->sq.queue);
    msg_cb->sq.queue = NULL;
    iounmap(msg_cb->cq.queue);
    msg_cb->cq.queue = NULL;

    return;
}


STATIC void tsdrv_read_event_task(unsigned long data)
{
    struct sched_published_event event = { 0 };
    aicpu_to_ts_task_info out_buf = { 0 };
    unsigned short buff_len;
    int pid = 0;
    u32 node_id = 0;
    int ret_queue;
    int ret;
    u32 vfid = 0;

    event.event_info.event_id = EVENT_TS_CTRL_MSG;
    event.event_info.subevent_id = 0;
    event.event_func.event_ack_func = tsdrv_send_ack_msg;
    event.event_func.event_finish_func = NULL;
    buff_len = sizeof(aicpu_to_ts_task_info);

    do {
        /* read ts ctrl msg */
        ret_queue = pull_data_from_queue(&g_aicpu_to_ts_cb.cq, &out_buf, buff_len);
        if (ret_queue == true) {
            atomic64_inc(&g_aicpu_to_ts_cb.v_cq_rx);
            /* fill data to event sched */
            node_id = cpu_to_node(smp_processor_id()); /*lint !e666  !e453*/
            ret = devdrv_query_process_by_host_pid(out_buf.pid, node_id, DEVDRV_PROCESS_CP1, vfid, &pid);
            if (unlikely(ret)) {
                ts_drv_err("query pid by host pid error ret:%d,host_pid:%u\n", ret, out_buf.pid);
                continue;
            }
            event.event_info.pid = pid;
            event.event_info.gid = 0;
            event.event_info.msg_len = buff_len;
            event.event_info.msg = (char *)&out_buf;
            ret = sched_submit_event(node_id, &event);
            if (unlikely(ret)) {
                ts_drv_err("sched_submit_event failed, ret:%d,node_id:%u.\n", ret, node_id);
            }
        }
    } while (ret_queue == true);
    return;
}


STATIC irqreturn_t tsdrv_tsirq_callback(int irq, void *data)
{
    /* be careful of disable interrupt ,enbale interrupt */
    unsigned long flags;

    if (data == NULL) {
        ts_drv_err("param is null. irq = %d\n", irq);
        return IRQ_NONE;
    }
    local_irq_save(flags);

    tasklet_schedule(&g_msg_wakeup_task);

    local_irq_restore(flags);

    atomic64_inc(&g_aicpu_to_ts_cb.v_irq_rx);
    return IRQ_HANDLED;
}

STATIC void tsdrv_writel_relaxed(void *reg, unsigned int val)
{
    if (reg == NULL) {
        ts_drv_err("the param is null.\n");
        return;
    }
    writel_relaxed(val, reg);
    dsb(sy);
}

STATIC void tsdrv_send_interrupt_cmd_to_ts(unsigned int irq)
{
    u32 shiftoffset;
    u32 val;
    u32 igp;
    void __iomem *reg;

    int node_id = numa_node_id();
    struct drv_hwts_ctrl *hwts = g_drv_ctrl_hwts[node_id];

    shiftoffset = (irq % REG_LEN);
    val = (1U << shiftoffset);
    igp = (irq / (u32)REG_LEN) * (u32)REG_ALIGN;
    reg = (void __iomem *)(hwts->ts_info[0].gicd_info.gicd_va_base + GICD_ISPENDR + igp);

    tsdrv_writel_relaxed(reg, val);
    atomic64_inc(&g_aicpu_to_ts_cb.v_irq_tx);
}

STATIC int tsdrv_register_tsirq_to_cores(aicpu_to_ts_msg_cb *msg_cb)
{
    int ret;
    if ((msg_cb == NULL) || (msg_cb->pdev == NULL)) {
        ts_drv_err("msg_cb or pdev is null.\n");
        return TS_PARA_ERR;
    }
    /* tasklet init */
    tasklet_init(&g_msg_wakeup_task, tsdrv_read_event_task, (unsigned long)(uintptr_t)&g_aicpu_to_ts_cb);
    /* request irq */
    ret = devm_request_irq(&g_aicpu_to_ts_cb.pdev->dev, g_aicpu_to_ts_cb.cq.irq, tsdrv_tsirq_callback,
        IRQF_TRIGGER_RISING, "aicpudrv_driver", &g_aicpu_to_ts_cb);
    if (ret != 0) {
        ts_drv_err("request irq failed, ret = %d.\n", ret);
        tasklet_kill(&g_msg_wakeup_task);
        return TS_INNER_ERR;
    }
    return ret;
}

STATIC int ts_queue_init(struct platform_device *pdev, unsigned short channel_id, aicpu_to_ts_msg_queue_cb sq,
    aicpu_to_ts_msg_queue_cb cq)
{
    aicpu_to_ts_msg_cb *msg_cb = &g_aicpu_to_ts_cb;
    int ret;

    msg_cb->pdev = pdev;
    msg_cb->channel_id = channel_id;

    /* init sq */
    msg_cb->sq.queue_depth = sq.queue_depth;
    msg_cb->sq.queue_entry_length = sq.queue_entry_length;
    msg_cb->sq.pa = sq.pa;
    msg_cb->sq.queue = ioremap((uintptr_t)sq.pa, AICPU_TO_TS_SRAM_ADDR_SQ_LEN);
    if (msg_cb->sq.queue == NULL) {
        ts_drv_err("init sq failed");
        return TS_INNER_ERR;
    }
    msg_cb->sq.proc = sq.proc;
    msg_cb->sq.irq = sq.irq;

    /* init cq */
    msg_cb->cq.queue_depth = cq.queue_depth;
    msg_cb->cq.queue_entry_length = cq.queue_entry_length;
    msg_cb->cq.pa = cq.pa;
    msg_cb->cq.queue = ioremap((uintptr_t)cq.pa, AICPU_TO_TS_SRAM_ADDR_CQ_LEN);
    if (msg_cb->cq.queue == NULL) {
        ts_drv_err("init cq failed");
        iounmap(msg_cb->sq.queue);
        msg_cb->sq.queue = NULL;
        return TS_INNER_ERR;
    }
    msg_cb->cq.proc = NULL;
    msg_cb->cq.irq = cq.irq;

    /* init queue head */
    msg_cb->sq.queue->head = 0;
    msg_cb->sq.queue->tail = 0;
    msg_cb->cq.queue->head = 0;
    msg_cb->cq.queue->tail = 0;

    /* init count */
    atomic64_set(&msg_cb->v_irq_tx, 0); //lint !e65 !e102 !e144 !e145 !e151 !e446 !e1112
    atomic64_set(&msg_cb->v_irq_rx, 0); //lint !e65 !e102 !e144 !e145 !e151 !e446 !e1112
    atomic64_set(&msg_cb->v_sq_tx, 0); //lint !e65 !e102 !e144 !e145 !e151 !e446 !e1112
    atomic64_set(&msg_cb->v_cq_rx, 0); //lint !e65 !e102 !e144 !e145 !e151 !e446 !e1112

    /* register irq */
    ret = tsdrv_register_tsirq_to_cores(msg_cb);
    if (ret != 0) {
        iounmap(msg_cb->sq.queue);
        msg_cb->sq.queue = NULL;
        iounmap(msg_cb->cq.queue);
        msg_cb->cq.queue = NULL;
        ts_drv_err("register irq failed:%d\n", ret);
        return ret;
    }
    sema_init(&msg_cb->sq.queue_sema, 1);
    sema_init(&msg_cb->cq.queue_sema, 1);

    create_msg_chan_to_ts(msg_cb);
    ts_drv_info("init aicpu2ts channel:%d ok.\n", channel_id);
    return TS_INNER_SUCCESS;
}
void aisle_uninit(int node_id)
{
    ts_queue_release(&g_aicpu_to_ts_cb);
}


int aisle_init(struct platform_device *pdev, int node_id)
{
    aicpu_to_ts_msg_queue_cb sq = { 0 };
    aicpu_to_ts_msg_queue_cb cq = { 0 };
    aicpu_to_ts_config conf = { 0 };
    g_wait_queue_time_out = msecs_to_jiffies(WAIT_QUEUE_NOT_FULL_TIME_OUT);
    conf.ts_to_aicpu_irq = platform_get_irq(pdev, TS_AICPU_IRQ_INDEX);
    conf.aicpu_to_ts_proc = tsdrv_send_interrupt_cmd_to_ts;

    if (conf.cmd.channel_id >= AICPU_TO_TS_MSG_CHAN_MAX) {
        ts_drv_err("channel_id is out range:%d\n", conf.cmd.channel_id);
        return TS_PARA_ERR;
    }

    sq.pa = (void *)(uintptr_t)AICPU_TO_TS_SRAM_ADDR_SQ;
    sq.irq = AICPU_TO_TS_SQ_IRQ;
    sq.proc = conf.aicpu_to_ts_proc;
    sq.queue_depth = AICPU_TO_TS_MSG_CHAN_SQ_DEPTH;
    sq.queue_entry_length = AICPU_TO_TS_MSG_CHAN_QE_LENGTH;

    cq.pa = (void *)(uintptr_t)AICPU_TO_TS_SRAM_ADDR_CQ;
    cq.queue_depth = AICPU_TO_TS_MSG_CHAN_CQ_DEPTH;
    cq.queue_entry_length = AICPU_TO_TS_MSG_CHAN_QE_LENGTH;
    cq.irq = conf.ts_to_aicpu_irq;
    cq.proc = NULL;

    return ts_queue_init(pdev, conf.cmd.channel_id, sq, cq);
}
#else
void ut_device_aisle_queue_test(void)
{
}
#endif
