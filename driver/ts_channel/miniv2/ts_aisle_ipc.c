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
#ifndef TSDRV_UT
#include <linux/notifier.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#ifdef AOS_LLVM_BUILD
#include <linux/topology.h>
#endif
#include "securec.h"
#include "ts_aisle_api.h"
#include "drv_ipc.h"
#include "event_sched_inner.h"
#include "ascend_hal_define.h"
#include "ts_drv_init.h"
#include "ts_api.h"
#include "tsdrv_interface.h"
#include "devdrv_manager_comm.h"
#include "ts_aisle_ipc.h"
#include "ts_aisle_queue.h"
#include "task_struct.h"
#include "icm_interface.h"
#include "hvtsdrv_tsagent.h"
#include "cp_identity_interface.h"
#include "hwts_task_info.h"

struct aisle_ipc_info {
    u32 node_id;
    struct notifier_block ipc_monitor[IPC_MONITOR_NUM];
};
static struct aisle_ipc_info g_aisle_ipc_info[CHIP_NUM_MAX];

int ts_event_id_v2p(u32 dev_id, u32 ts_id, u32 virt_id, u32 *phy_id)
{
#ifdef CFG_FEATURE_ID_MAPING
    int ret;
    unsigned int chip_id;
    unsigned int vfid;
    unsigned int host_pid;
    enum devdrv_process_type cp_type;

    ret = devdrv_query_process_host_pid(current->tgid, &chip_id, &vfid, &host_pid, &cp_type);
    if (ret != 0) {
        ts_drv_err("devid %u, tsid %u, virt_id %u invalid.\n", dev_id, ts_id, virt_id);
        return TS_PARA_ERR;
    }

    if (vfid == 0) {
        *phy_id = virt_id;
        return 0;
    }

    ret = hvtsdrv_event_id_v2p(dev_id, ts_id, vfid, virt_id, phy_id);
    if (ret != 0) {
        ts_drv_err("devid %u, tsid %u, virt_id %u invalid.\n", dev_id, ts_id, virt_id);
        return TS_PARA_ERR;
    }
#else
    *phy_id = virt_id;
#endif
    return 0;
}

int ts_write_record_register(u32 dev_id, u32 ts_id, u32 record_type, u32 record_id)
{
    void __iomem *hwts_base_va = get_hwts_base_va(dev_id, ts_id);
    u64 hwts_offset, base_addr;
    u32 offset_id = record_id;
    pid_t pid = current->tgid;
#ifndef AOS_LLVM_BUILD
    int ret;
#endif
    if ((record_id >= HWTS_RECORD_ID_MAX)) {
        ts_drv_err("devid %u, tsid %u, record id %u invalid.\n", dev_id, ts_id, record_id);
        return TS_PARA_ERR;
    }

    if (unlikely(hwts_base_va == NULL)) {
        ts_drv_err("get HWTS base virtual address error, devid %u, tsid %u\n", dev_id, ts_id);
        return -EINVAL;
    }
    ts_drv_debug("devid %u, vfid %d, tsid %u, type %u, record id %u.\n", dev_id, 0, ts_id, record_type, record_id);

    if (record_type == AICPU_MSG_NOTIFY_RECORD) {
        base_addr = (u64)HWTS_NOTIFY_TABLE_0_REG;
    } else {
        base_addr = (u64)HWTS_EVENT_TABLE_0_REG;
        if (ts_event_id_v2p(dev_id, ts_id, record_id, &offset_id) != 0) {
            ts_drv_err("devid %u, tsid %u, record id %u v2p failed.\n", dev_id, ts_id, record_id);
            return TS_PARA_ERR;
        }
    }
#ifndef AOS_LLVM_BUILD
    ret = devdrv_get_dev_process(pid);
    if (ret != 0) {
        ts_drv_err("Can not get process. (devid=%u; tsid=%u; ret=%d).\n", dev_id, ts_id, ret);
        return -EINVAL;
    }
#endif
    hwts_offset = (u64)(base_addr + (u32)(offset_id * HWTS_RECORD_TABLE_LEN));

    *((volatile u64* volatile)((uintptr_t)(hwts_base_va + hwts_offset))) = 0ULL;
    isb();
    *((volatile u64* volatile)((uintptr_t)(hwts_base_va + hwts_offset))) = HWTS_RECORD_WAIT_FLAG;
    isb();

#ifndef AOS_LLVM_BUILD
    devdrv_put_dev_process(pid);
#endif
    ts_drv_debug("Write success. (devid=%u; tsid=%u; record_id=%u; pid=%d)\n", dev_id, ts_id, record_id, pid);
    return 0;
}

int send_msg_to_ts_async(int dev_id, int ts_id, unsigned int len, const void *msg)
{
#ifdef CFG_ASCEND_FLORENCE_ASCEND610
    const ts_aicpu_sqe_t *aicpu_sqe = NULL;
#endif
    char ipc_msg[TS_IPC_MSG_MAX_LEN * sizeof(u32)];
    rproc_id_t rproc_id;
    int ret;

    if ((msg == NULL) || (len == 0)) {
        ts_drv_err("The parameter is incorrect. (dev_id=%d; ts_id=%d; len=%u)\n", dev_id, ts_id, len);
        return TS_PARA_ERR;
    }

    switch (ts_id) {
        case TSC_ID:
            rproc_id = HISI_RPROC_TSC_TX_RPID3;
            break;
        case TSV_ID:
            rproc_id = HISI_RPROC_TSV_TX_RPID3;
            break;
        default:
            ts_drv_err("invalid ts_id : %d\n", ts_id);
            return -1;
    }

#ifdef CFG_ASCEND_FLORENCE_ASCEND610
    /* Fix The bug of ascend310p notify function in MDC ASCEND610. */
    if ((msg != NULL) && (len >= (TS_AICPU_SQE_HEAD_LEN + sizeof(ts_aicpu_notify_t)))) {
        if (((ts_aicpu_sqe_t *)(msg))->cmd_type == (int)AICPU_NOTIFY_RECORD) {
            aicpu_sqe = (const ts_aicpu_sqe_t *)msg;
            if (unlikely(aicpu_sqe->u.aicpu_notify.notify_id >= HWTS_RECORD_ID_MAX)) {
                ts_drv_err("devid %d, tsid %d, invalid notify id:%u\n", dev_id, ts_id,
                    aicpu_sqe->u.aicpu_notify.notify_id);
                return -EINVAL;
            }
            return ts_write_record_register(dev_id, ts_id, AICPU_MSG_EVENT_RECORD,
                aicpu_sqe->u.aicpu_notify.notify_id);
        }
    }
#endif

    ipc_msg[0] = MSG_TYPE_B0;
    ipc_msg[1] = MSG_TYPE_B1;
    ipc_msg[2] = MSG_TYPE_B2;
    ipc_msg[3] = MSG_TYPE_B3;

    if (memcpy_s(ipc_msg + MSG_HEAD_LEN, (TS_IPC_MSG_MAX_LEN * sizeof(u32)) - MSG_HEAD_LEN, msg, len) != 0) {
        ts_drv_err("dev_id:%d, ts_id:%d, len:%u, memcpy failed\n", dev_id, ts_id, len);
        return TS_INNER_ERR;
    }

    ret = icm_msg_send_async(ICM_FD_BUILD((u32)dev_id, rproc_id), (rproc_msg_t *)ipc_msg, ((len / sizeof(u32)) + 1));
    if (ret != 0) {
        ts_drv_err("send msg to ts async via ipc error, devid=%d, ret=%d\n", dev_id, ret);
        return TS_INNER_ERR;
    }

    return 0;
}

STATIC int ts_aisle_ipc_sched_ack(unsigned int devid, unsigned int subevent_id, const char *msg,
    unsigned int msg_len, void *priv)
{
    int dev_id = numa_node_id();
    ts_aicpu_sqe_t sqe_msg;
    struct hwts_response *resp = NULL;

    if ((msg == NULL) || (msg_len == 0)) {
        ts_drv_err("msg pointer or ipc ack msg len error, msg pointer = 0x%pK, msg_len:%u\n",
            (void *)(u64)(uintptr_t)msg, msg_len);
        return TS_PARA_ERR;
    }

    resp = (struct hwts_response *)msg;
    if ((resp->msg == NULL) || (resp->len == 0) || ((size_t)resp->len > sizeof(ts_aicpu_sqe_t))) {
        ts_drv_err("Ipc ack msg error. (len=%d)\n", resp->len);
        return TS_PARA_ERR;
    }

    if (copy_from_user((void *)&sqe_msg, (void *)resp->msg, resp->len) != 0) {
        ts_drv_err("Copy msg from user error. (len=%d)\n", resp->len);
        return TS_COPY_USER_ERR;
    }

    return send_msg_to_ts_async(dev_id, subevent_id, resp->len, (void *)&sqe_msg);
}

STATIC int submit_msg_to_event_sched(u32 node_id, u32 ts_id, unsigned long len, void *msg)
{
    struct sched_published_event publish_event = { 0 };
    aicpu_to_ts_task_info *task_info = NULL;
    struct ipcdrv_message *ipc_msg = NULL;
    u32 hpid, type;
    int ret, pid;

    if ((len > TS_IPC_MSG_MAX_LEN) || (len <= 1)) {
        ts_drv_err("msg len error len:%lu\n", len);
        return TS_PARA_ERR;
    }

    ipc_msg = (struct ipcdrv_message *)msg;
    type = ipc_msg->ipc_msg_header.cmd_type;
    if (type != IPCDRV_TS_TO_AICPU_ENGIN) {
        return 0;
    }

    task_info = (aicpu_to_ts_task_info *)ipc_msg->ipcdrv_payload;
    hpid = *((u32 *)(msg + sizeof(u32))); /* head size is 32 bits */

    ret = devdrv_query_process_by_host_pid(hpid, node_id, DEVDRV_PROCESS_CP1, task_info->vfid, &pid);
    if (unlikely(ret)) {
        ts_drv_err("query pid error ret:%d, hpid:%u, vfid:%u, tsid:%u\n",
            ret, hpid, (u32)task_info->vfid, ts_id);
        return ret;
    }

    ts_drv_debug("pid %d, hpid %u, vfid %u, tsid %u\n", pid, hpid, task_info->vfid, ts_id);
    publish_event.event_info.pid = pid;
    publish_event.event_info.gid = 0;
    publish_event.event_info.event_id = EVENT_TS_CTRL_MSG;
    publish_event.event_info.subevent_id = ts_id;
    publish_event.event_func.event_ack_func = ts_aisle_ipc_sched_ack;
    publish_event.event_func.event_finish_func = NULL;
    publish_event.event_info.msg = (char *)(msg + sizeof(u32));
    publish_event.event_info.msg_len = (len * sizeof(u32)) - sizeof(u32);

    ret = sched_submit_event(node_id, &publish_event);
    if (unlikely(ret)) {
        ts_drv_err("submit kernel event error ret:%d, pid:%d, ts_id:%u.\n", ret, pid, ts_id);
        return ret;
    }

    return 0;
}

STATIC int receive_msg_from_tsc_async(struct notifier_block *nb, unsigned long len, void *msg)
{
    struct aisle_ipc_info *ipc_info = container_of(nb, struct aisle_ipc_info, ipc_monitor[TSC_ID]);

    return submit_msg_to_event_sched(ipc_info->node_id, TSC_ID, len, msg);
}

STATIC int receive_msg_from_tsv_async(struct notifier_block *nb, unsigned long len, void *msg)
{
    struct aisle_ipc_info *ipc_info = container_of(nb, struct aisle_ipc_info, ipc_monitor[TSV_ID]);

    return submit_msg_to_event_sched(ipc_info->node_id, TSV_ID, len, msg);
}

int aisle_init(struct platform_device *pdev, int node_id)
{
    int ret;
    (void)pdev;

    g_aisle_ipc_info[node_id].ipc_monitor[TSC_ID].notifier_call = receive_msg_from_tsc_async;
    g_aisle_ipc_info[node_id].ipc_monitor[TSC_ID].next = NULL;
    g_aisle_ipc_info[node_id].ipc_monitor[TSC_ID].priority = 0;

    g_aisle_ipc_info[node_id].ipc_monitor[TSV_ID].notifier_call = receive_msg_from_tsv_async;
    g_aisle_ipc_info[node_id].ipc_monitor[TSV_ID].next = NULL;
    g_aisle_ipc_info[node_id].ipc_monitor[TSV_ID].priority = 0;

    g_aisle_ipc_info[node_id].node_id = node_id;

    ret = rproc_rx_register(node_id, HISI_RPROC_TSC_RX_RPID7, &g_aisle_ipc_info[node_id].ipc_monitor[TSC_ID]);
    if (ret != 0) {
        ts_drv_err("register tsc aisle error, ret=%d\n", ret);
        return ret;
    }

    ret = rproc_rx_register(node_id, HISI_RPROC_TSV_RX_RPID7, &g_aisle_ipc_info[node_id].ipc_monitor[TSV_ID]);
    if (ret != 0) {
        (void)rproc_rx_unregister(node_id, HISI_RPROC_TSC_RX_RPID7,
            &g_aisle_ipc_info[node_id].ipc_monitor[TSC_ID]);
        ts_drv_err("register tsv aisle error, ret=%d\n", ret);
        return ret;
    }

    return 0;
}

void aisle_uninit(int node_id)
{
    (void)rproc_rx_unregister(node_id, HISI_RPROC_TSC_RX_RPID7, &g_aisle_ipc_info[node_id].ipc_monitor[TSC_ID]);
    (void)rproc_rx_unregister(node_id, HISI_RPROC_TSV_RX_RPID7, &g_aisle_ipc_info[node_id].ipc_monitor[TSV_ID]);
    return;
}
#else /* TSDRV_UT */
void ut_device_ts_aisle_ipc_test(void)
{
}
#endif /* TSDRV_UT */

