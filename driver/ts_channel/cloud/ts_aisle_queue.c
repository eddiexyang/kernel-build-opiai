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
#include <linux/notifier.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/securec.h>
#include <linux/securectype.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/uio_driver.h>
#include <linux/types.h>
#include <asm/barrier.h>
#include <linux/atomic.h>
#ifndef TSDRV_UT

#include "securec.h"
#include "hwts_drv_api.h"
#include "ts_aisle_api.h"
#include "drv_ipc.h"
#include "event_sched_inner.h"
#include "ascend_hal_define.h"
#include "ts_drv_init.h"
#include "ts_api.h"
#include "ts_aisle_queue.h"
#include "devdrv_functional_cqsq_api.h"
#include "aicpu.h"
#include "devdrv_manager_comm.h"
#include "task_struct.h"
#include "hwts_task_info.h"

#define WAIT_QUEUE_NOT_FULL_TIME_OUT 1000
#define WAIT_QUEUE_NOT_FULL_CNT 1000000

extern int devdrv_get_tsdrv_cq_aisle_irq(int node_id);
int g_aicpu_to_ts_sq_irq[] = {65534, 65499, 65449, 65399};
aicpu_to_ts_msg_cb g_aicpu_to_ts_cb[CHIP_NUM_MAX] = {0};
unsigned long g_wait_queue_time_out = 0;

int ts_write_record_register(u32 dev_id, u32 ts_id, u32 record_type, u32 record_id)
{
    return 0;
}

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
    if (((q->tail + 1U) % (queue->queue_depth)) == q->head) {
        return true;
    }
    return false;
}

static int pull_data_from_queue(aicpu_to_ts_msg_queue_cb *queue, void *out_buf, unsigned short buf_len)
{
    int ret;
    aicpu_to_ts_queue *q = queue->queue;
    unsigned long long queue_head = (unsigned long long)(uintptr_t)q->data;
    queue_head = (unsigned long long)queue_head +
        (unsigned long long)((unsigned int)q->head * (unsigned int)queue->queue_entry_length);

    if (check_queue_empty(queue)) {
        return false;
    }
    ret = memcpy_s((void *)out_buf, buf_len,
                   (void *)(uintptr_t)queue_head,
                   queue->queue_entry_length);
    if (ret != 0) {
        ts_drv_err("memcpy_s failed:%d\n", ret);
        return false;
    }

    mb();

    q->head = ((q->head + 1U) % (queue->queue_depth));

    return true;
}

STATIC int push_data_to_queue(aicpu_to_ts_msg_queue_cb *queue, const void *in_buf, unsigned short buf_len)
{
#define USLEEP_MIN 1
#define USLEEP_MAX 2

    aicpu_to_ts_queue *q = queue->queue;
    unsigned long long queue_tail = (unsigned long long)(uintptr_t)q->data;
    ts_aicpu_sqe_t *msg = (ts_aicpu_sqe_t *)in_buf;
    enum devdrv_process_type cp_type = DEVDRV_PROCESS_CP1;
    u32 host_pid, vfid, chip_id;
    int wait_cnt = 0;
    int ret;

    /* check input data length */
    if (buf_len > queue->queue_entry_length) {
        ts_drv_err("input data to long,buf_len:%u,entry_length:%u\n", (u32)buf_len, (u32)queue->queue_entry_length);
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

    ret = devdrv_query_process_host_pid(current->tgid, &chip_id, &vfid, &host_pid, &cp_type);
    if (ret != 0) {
        ts_drv_err("process %d get vfid failed. ret %d\n", current->tgid, ret);
        return ret;
    }

    ts_drv_debug("vfid old=%u, new=%u\n", (u32)msg->vf_id, vfid);
    msg->vf_id = vfid;

    ret = memcpy_s((void *)(uintptr_t)queue_tail, queue->queue_entry_length, (void *)(uintptr_t)in_buf, buf_len);
    if (ret != 0) {
        ts_drv_err("memcpy_s failed:%d\n", ret);
        return false;
    }

    wmb();

    /* update tail */
    q->tail = ((q->tail + 1U) % (queue->queue_depth));
    wmb();

    return true;
}

int send_msg_to_ts_async(int node_id, int ts_id, unsigned int len, const void *msg)
{
    int ret;

    if ((msg == NULL) || (len == 0)) {
        ts_drv_err("in_buf failed, node_id(%d), ts_id(%d), len(%u)\n", node_id, ts_id, len);
        return TS_PARA_ERR;
    }

    if ((node_id < 0 || node_id >= CHIP_NUM_MAX) || (ts_id < 0 || ts_id >= MAX_TS_NUM)) {
        ts_drv_err("in_buf failed, node_id(%d), ts_id(%d), len(%u)\n", node_id, ts_id, len);
        return TS_PARA_ERR;
    }
    if (g_aicpu_to_ts_cb[node_id].sq.queue == NULL) {
        ts_drv_err("sq buf is NULL, node_id(%d), ts_id(%d), len(%u)\n", node_id, ts_id, len);
        return TS_PARA_ERR;
    }
    ret = down_timeout(&g_aicpu_to_ts_cb[node_id].sq.queue_sema, g_wait_queue_time_out);
    if (ret != 0) {
        ts_drv_err("down_timeout, node_id(%d), ts_id(%d),head:%u,tail:%u\n", node_id, ts_id,
            (u32)g_aicpu_to_ts_cb[node_id].sq.queue->head, (u32)g_aicpu_to_ts_cb[node_id].sq.queue->tail);
        return -EAGAIN;
    }
    if (push_data_to_queue(&(g_aicpu_to_ts_cb[node_id].sq), msg, len) == false) {
        up(&g_aicpu_to_ts_cb[node_id].sq.queue_sema);
        ts_drv_err("push_data_to_queue failed, node_id(%d), ts_id(%d)\n", node_id, ts_id);
        return -EDOM;
    }
    /* send irq to ts */
    if (g_aicpu_to_ts_cb[node_id].sq.proc != NULL) {
        g_aicpu_to_ts_cb[node_id].sq.proc(g_aicpu_to_ts_cb[node_id].sq.irq, node_id);
    }
    up(&g_aicpu_to_ts_cb[node_id].sq.queue_sema);

    atomic64_inc(&g_aicpu_to_ts_cb[node_id].v_sq_tx);

    return 0;
}
EXPORT_SYMBOL(send_msg_to_ts_async); //lint !e508 !e532

STATIC int tsdrv_send_ack_msg(unsigned int devid, unsigned int subevent_id, const char *msg,
    unsigned int msg_len, void *priv)
{
    int node_id = numa_node_id();
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

    return send_msg_to_ts_async(node_id, subevent_id, resp->len, (void *)&sqe_msg);
}

STATIC void tsdrv_read_event_task(unsigned long data)
{
    struct sched_published_event event = { 0 };
    aicpu_to_ts_task_info out_buf = { 0 };
    aicpu_to_ts_msg_cb *msg_cb = NULL;
    unsigned short buff_len;
    u32 node_id = 0;
    int ret_query;
    int pid = 0;
    int ret;
    u32 vfid = 0;

    msg_cb = (aicpu_to_ts_msg_cb *)(uintptr_t)data;
    event.event_info.event_id = EVENT_TS_CTRL_MSG;
    event.event_info.subevent_id = 0;
    event.event_func.event_ack_func = tsdrv_send_ack_msg;
    event.event_func.event_finish_func = NULL;
    buff_len = sizeof(aicpu_to_ts_task_info);

    do {
        /* read ts ctrl msg */
        ret = pull_data_from_queue(&msg_cb->cq, &out_buf, buff_len);
        if (ret == true) {
            /* fill data to event sched */
            node_id = (u32)cpu_to_node(smp_processor_id()); /*lint !e666  !e453*/
            atomic64_inc(&g_aicpu_to_ts_cb[node_id].v_cq_rx);
            vfid = out_buf.vfid;
            ret_query = devdrv_query_process_by_host_pid(out_buf.pid, node_id, DEVDRV_PROCESS_CP1, vfid, &pid);
            if (unlikely(ret_query)) {
                ts_drv_err("query pid by host pid error ret:%d,host_pid:%u\n", ret_query, out_buf.pid);
                continue;
            }
            event.event_info.pid = pid;
            event.event_info.gid = 0;
            event.event_info.msg_len = buff_len;
            event.event_info.msg = (char *)&out_buf;
            ret_query = sched_submit_event(node_id, &event);
            if (unlikely(ret_query)) {
                ts_drv_err("sched_submit_event failed, ret:%d, node_id:%u.\n", ret_query, node_id);
            }
        }
    } while (ret == true);

    return;
}

STATIC irqreturn_t tsdrv_tsirq_callback(int irq, void *data)
{
    /* be careful of disable interrupt ,enbale interrupt */
    aicpu_to_ts_msg_cb *msg_cb = (aicpu_to_ts_msg_cb *)data;
    u32 node_id;

    if (data == NULL) {
        ts_drv_err("param is null. irq = %d\n", irq);
        return IRQ_NONE;
    }
    tasklet_schedule(&msg_cb->wakeup_task);

    node_id = (u32)numa_node_id();
    atomic64_inc(&g_aicpu_to_ts_cb[node_id].v_irq_rx);

    return IRQ_HANDLED;
}

STATIC int tsdrv_register_tsirq_to_cores(aicpu_to_ts_msg_cb *msg_cb, int node_id)
{
    int ret;
    u32 cpu_id;

    /* tasklet init */
    tasklet_init(&msg_cb->wakeup_task, tsdrv_read_event_task,
        (unsigned long)(uintptr_t)msg_cb);
    /* request irq */
    ret = devm_request_irq(&msg_cb->pdev->dev,
                           msg_cb->cq.irq,
                           tsdrv_tsirq_callback,
                           IRQF_TRIGGER_RISING,
                           "ts_aisle_driver",
                           msg_cb);
    if (ret != 0) {
        ts_drv_err("request irq failed, ret = %d.\n", ret);
        tasklet_kill(&msg_cb->wakeup_task);
        return ret;
    }

    if (g_drv_ctrl_hwts[node_id]->ts_info[0].chip_info.chip_type == AICPU_PLATFORM_FPGA) {
        cpu_id = (u32)(CPU_NUM_EACH_NODE_OF_FPGA * node_id);
    } else {
        cpu_id = ((g_drv_ctrl_hwts[node_id]->ts_info[0].irq_info.aicpu_id_base +
        g_drv_ctrl_hwts[node_id]->ts_info[0].irq_info.aicpu_num) * (u32)node_id);
    }
    ts_drv_info("tsdrv_register_tsirq_to_cores cpu_id = %u, irq = %u",
        cpu_id, msg_cb->cq.irq);

    ret = irq_set_affinity_hint(msg_cb->cq.irq, get_cpu_mask(cpu_id));
    if (ret != 0) {
        devm_free_irq(&msg_cb->pdev->dev, msg_cb->cq.irq, msg_cb);
        tasklet_kill(&msg_cb->wakeup_task);
        ts_drv_err("irq %u set affinity to cpu_id %d err:%d\n", msg_cb->cq.irq, cpu_id, ret);
        return ret;
    }

    return ret;
}

STATIC void create_msg_chan_to_ts(aicpu_to_ts_msg_cb *msg_cb, int node_id)
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
    ts_drv_info("aisle init send msg_chan info to ts node_id = %d", node_id);
    /* call send cmd to ts */
    (void)devdrv_mailbox_send_cqsq((u32)node_id, 0, (struct devdrv_mailbox_cqsq *)data);
}

STATIC int tsdrv_queue_init(unsigned short channel_id,
                            aicpu_to_ts_msg_queue_cb sq,
                            aicpu_to_ts_msg_queue_cb cq,
                            int node_id)
{
    aicpu_to_ts_msg_cb *msg_cb = &g_aicpu_to_ts_cb[node_id];
    int ret;

    sema_init(&msg_cb->sq.queue_sema, 1);
    sema_init(&msg_cb->cq.queue_sema, 1);
    msg_cb->channel_id = channel_id;

    /* init sq */
    msg_cb->sq.queue_depth = sq.queue_depth;
    msg_cb->sq.queue_entry_length = sq.queue_entry_length;
    msg_cb->sq.pa = sq.pa;
    msg_cb->sq.queue = ioremap((uintptr_t)((char *)sq.pa + node_id * SYSTEM_MUL_CHIP_CONFIG_BASE),
        AICPU_TO_TS_SRAM_ADDR_SQ_LEN);
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
    msg_cb->cq.queue = ioremap((uintptr_t)((char *)cq.pa + node_id * SYSTEM_MUL_CHIP_CONFIG_BASE),
        AICPU_TO_TS_SRAM_ADDR_CQ_LEN);
    if (msg_cb->cq.queue == NULL) {
        iounmap(msg_cb->sq.queue);
        msg_cb->sq.queue = NULL;
        ts_drv_err("init cq failed");
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
    atomic64_set(&msg_cb->v_irq_tx, 0);
    atomic64_set(&msg_cb->v_irq_rx, 0);
    atomic64_set(&msg_cb->v_sq_tx, 0);
    atomic64_set(&msg_cb->v_cq_rx, 0);

    /* register irq */
    ret = tsdrv_register_tsirq_to_cores(msg_cb, node_id);
    if (ret != 0) {
        iounmap(msg_cb->sq.queue);
        msg_cb->sq.queue = NULL;
        iounmap(msg_cb->cq.queue);
        msg_cb->cq.queue = NULL;
        ts_drv_err("register irq failed:%d\n", ret);
        return ret;
    }
    create_msg_chan_to_ts(msg_cb, node_id);

    ts_drv_info("node_id %d init aicpu2ts channel:%u ok.\n", node_id, (u32)channel_id);

    return 0;
}

int tsdrv_queue_cb_init(int node_id, aicpu_to_ts_config conf)
{
#define AICPU_TO_TS_SQ_IRQ_INDEX_MAX 4

    aicpu_to_ts_msg_queue_cb sq = { 0 };
    aicpu_to_ts_msg_queue_cb cq = { 0 };
    int ret;

    if ((node_id < 0) || (node_id >= AICPU_TO_TS_SQ_IRQ_INDEX_MAX)) {
        ts_drv_err("node_id error. node_id:%d\n", node_id);
        return TS_PARA_ERR;
    }

    if (conf.cmd.channel_id >= AICPU_TO_TS_MSG_CHAN_MAX) {
        ts_drv_err("channel_id is out range:%d\n", conf.cmd.channel_id);
        return TS_PARA_ERR;
    }

    sq.pa = (void *)(uintptr_t)AICPU_TO_TS_SRAM_ADDR_SQ;
    sq.irq = (u32)g_aicpu_to_ts_sq_irq[node_id];
    sq.proc = conf.aicpu_to_ts_proc;
    sq.queue_depth = (unsigned short)AICPU_TO_TS_MSG_CHAN_SQ_DEPTH;
    sq.queue_entry_length = AICPU_TO_TS_MSG_CHAN_QE_LENGTH;

    cq.pa = (void *)(uintptr_t)AICPU_TO_TS_SRAM_ADDR_CQ;
    cq.queue_depth = (unsigned short)AICPU_TO_TS_MSG_CHAN_CQ_DEPTH;
    cq.queue_entry_length = AICPU_TO_TS_MSG_CHAN_QE_LENGTH;
    ret = devdrv_get_tsdrv_cq_aisle_irq(node_id);
    if (ret < 0) {
        ts_drv_err("get cq aisle irq failed! irq = %d\n", ret);
        return TS_INNER_ERR;
    }
    cq.irq = (u32)ret;
    cq.proc = NULL;

    ts_drv_info("aisle init sq.irq = %u, cq.irq = %u",
        sq.irq, cq.irq);
    return tsdrv_queue_init(conf.cmd.channel_id, sq, cq, node_id);
}

void aisle_send_interrupt_to_ts(u64 irq, int node_id)
{
    void __iomem *reg;

    reg = (void __iomem *)(((char *)(g_drv_ctrl_hwts[node_id]->ts_info[0].ts_gicr_va_base)) + GICR_SETLPIR);

    hwts_drv_writeq_relaxed(reg, irq);
    atomic64_inc(&g_aicpu_to_ts_cb[node_id].v_irq_tx);
}

STATIC void destory_msg_chan_to_ts(aicpu_to_ts_msg_cb *msg_cb, int node_id)
{
    unsigned char buff[AICPU_TO_TS_MSG_MAILBOX_LENGTH] = { 0 };
    struct devdrv_mailbox_destory_ts_com *data = (struct devdrv_mailbox_destory_ts_com *)buff;
    data->valid = AICPU_TO_TS_MAILBOX_VALID;
    data->cmd_type = AICPU_TO_TS_DESTORY_CHANNEL;
    data->channel_id = msg_cb->channel_id;

    /* call send cmd to ts */
    (void)devdrv_mailbox_send_cqsq((u32)node_id, 0, (struct devdrv_mailbox_cqsq *)data);
}

int aisle_init(struct platform_device *pdev, int node_id)
{
    int ret;
    aicpu_to_ts_config conf = {0};

    g_wait_queue_time_out = msecs_to_jiffies(WAIT_QUEUE_NOT_FULL_TIME_OUT);
    g_aicpu_to_ts_cb[node_id].pdev = pdev;
    conf.aicpu_to_ts_proc = aisle_send_interrupt_to_ts;
    ret = tsdrv_queue_cb_init(node_id, conf);
    if (ret != 0) {
        ts_drv_err("ts queue aisle init failed.\n");
    }
    return ret;
}

void aisle_uninit(int node_id)
{
    aicpu_to_ts_msg_cb *msg_cb = &g_aicpu_to_ts_cb[node_id];

    /* free irq */
    (void)irq_set_affinity_hint(msg_cb->cq.irq, NULL);
    devm_free_irq(&msg_cb->pdev->dev, msg_cb->cq.irq, msg_cb);
    tasklet_kill(&msg_cb->wakeup_task);
    destory_msg_chan_to_ts(msg_cb, node_id);
    iounmap(msg_cb->sq.queue);
    msg_cb->sq.queue = NULL;
    iounmap(msg_cb->cq.queue);
    msg_cb->cq.queue = NULL;

    return ;
}
#else /* TSDRV_UT */
void ut_device_aisle_queue_test(void)
{
}
#endif /* TSDRV_UT */

