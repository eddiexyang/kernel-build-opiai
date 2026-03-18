/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/delay.h>
#ifndef AOS_LLVM_BUILD
#include <linux/irq.h>
#include <linux/securectype.h>
#include <linux/uio_driver.h>
#endif
#include <linux/acpi.h>
#include <linux/securec.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/version.h>
#ifdef AOS_LLVM_BUILD
#include <linux/topology.h>
#include <linux/irqflags.h>
#include <linux/of.h>
#endif
#ifndef TSDRV_UT
#include "hwts_drv_api.h"
#include "hwts_drv_config.h"
#include "ts_drv_init.h"
#include "hwts_task_info.h"
#include "event_sched_inner.h"
#include "sched_cmd.h"
#include "ascend_hal_define.h"
#include "ts_api.h"
#include "aicpu.h"
#include "devdrv_manager_comm.h"
#include "hwts_drv_miniv2.h"
#include "tsdrv_interface.h"
#include "devdrv_interface.h"
#include "ts_aisle_api.h"
#ifdef CFG_SOC_PLATFORM_MDC_V51
#include "bind_core.h"
#endif

#define AICPUINDEX_TO_AICPUID(nid, index, ts_index)  ((index) +      \
        (g_drv_ctrl_hwts[nid]->ts_info[ts_index].irq_info.aicpu_id_base))
#define TS_NUM_THREE 3
#if MAX_TS_NUM == 2
static u64 g_hwts_base_addr[MAX_TS_NUM] = {0xBA700000, 0xB9700000};
static u64 g_hwts_reg_size[MAX_TS_NUM] = {0x100000, 0x100000};
static u64 g_hwts_aicpu_ns_mb_base_addr_cfg[MAX_TS_NUM] = {0x90, 0x90};
static u64 g_aicpu_blk_cfg_ns[MAX_TS_NUM] = {0x9f000, 0x9f000};
static u64 g_aicpu_ks_status_ns[MAX_TS_NUM] = {0x9f200, 0x9f200};
static u64 g_aicpu_status_report_ns[MAX_TS_NUM] = {0x9f008, 0x9f008};
#elif MAX_TS_NUM == 1
#ifdef CFG_SOC_MDC_V51_LITE
static u64 g_hwts_base_addr[MAX_TS_NUM] = {0xB9700000};
#else
static u64 g_hwts_base_addr[MAX_TS_NUM] = {0xBA700000};
#endif

static u64 g_hwts_reg_size[MAX_TS_NUM] = {0x100000};
static u64 g_hwts_aicpu_ns_mb_base_addr_cfg[MAX_TS_NUM] = {0x90};
static u64 g_aicpu_blk_cfg_ns[MAX_TS_NUM] = {0x9f000};
static u64 g_aicpu_ks_status_ns[MAX_TS_NUM] = {0x9f200};
static u64 g_aicpu_status_report_ns[MAX_TS_NUM] = {0x9f008};

#endif
static void __iomem *g_hwts_base_va[CHIP_NUM_MAX][MAX_TS_NUM] = {{NULL}};

void __iomem *get_hwts_base_va(u32 node_id, u32 ts_id)
{
    if (unlikely(node_id >= CHIP_NUM_MAX)) {
        ts_drv_err("invalid node_id:%u\n", node_id);
        return NULL;
    }

    if (unlikely(ts_id >= MAX_TS_NUM)) {
        ts_drv_err("invalid ts id:%u\n", ts_id);
        return NULL;
    }

    return g_hwts_base_va[node_id][ts_id];
}

void __iomem *hwts_get_return_mailbox_addr(u32 mb_ind, u32 ts_ind, int node_id)
{
    u32 offset = 0;
    struct drv_hwts_ctrl *hwts = g_drv_ctrl_hwts[node_id];

    offset = mb_ind * (u32)(RET_MAILBOX_SIZE);
    return (void __iomem *)(hwts->ts_info[ts_ind].sram_info.sram_va_base + ((AICPU_MAILBOX_MAX_NUM) * MAILBOX_SIZE)
            + offset);
}

void __iomem *hwts_get_mailbox_addr(u32 mb_ind, u32 ts_ind, int node_id)
{
    unsigned int offset = 0;
    struct drv_hwts_ctrl *hwts = g_drv_ctrl_hwts[node_id];

    offset = mb_ind * (u32)MAILBOX_SIZE;
    return (void __iomem *)(hwts->ts_info[ts_ind].sram_info.sram_va_base + offset);
}

int hwts_drv_wait_ts_ready(struct drv_hwts_ctrl *hwts)
{
#define HWTS_READY_TIMEOUT (12000U)
    u32 stat = 0;
    int ret = 0;
    u32 i;
    u32 j = 0;

    if (hwts == NULL) {
        ts_drv_err("param is null.\n");
        return -EINVAL;
    }

    for (i = 0; i < MAX_TS_NUM; i++) {
        j = 0;
        while ((stat < TS_NUM_THREE) || (stat == 0xffffffffUL)) {
#ifndef AOS_LLVM_BUILD
            stat = readl_relaxed(hwts->ts_info[i].ts_aicpu_va_base + 0xbe0ULL);
#else
            stat = readl(hwts->ts_info[i].ts_aicpu_va_base + 0xbe0ULL);
#endif
            msleep(1);
            j++;
            if (j >= HWTS_READY_TIMEOUT) {
                ret = -1;
                break;
            }
        }
        if (ret != 0) {
            break;
        }
    }

    if (ret == -1) {
        ts_drv_err("hwts wait TS SRAM timeout(12s), stat=%u. ret=%d dev_id:%d\n", stat, ret,
                   hwts->numa_node);
    } else {
        ts_drv_debug("hwts drv TS SRAM init OK, then we remap sram. dev_id:%d\n", hwts->numa_node);
    }

    return ret;
}

void hwts_handle_ack_success(u32 mb_index, u32 ts_index)
{
    u64 reg_val;
    u32 mb_blk_shift = mb_index * MAILBOX_REG_SHIFT;
    struct drv_hwts_ts_info *ts_info = NULL;
    int node_id = numa_node_id();
#ifndef AOS_LLVM_BUILD
    reg_val = readq_relaxed((void *)(g_hwts_base_va[node_id][ts_index] + g_aicpu_blk_cfg_ns[ts_index] + mb_blk_shift));
#else
    reg_val = readq((void *)(g_hwts_base_va[node_id][ts_index] + g_aicpu_blk_cfg_ns[ts_index] + mb_blk_shift));
#endif
    ts_drv_debug("ack to hwts, reg_val(0x%llx), ts_index(%u), mb_index(%u), node_id(%u).\n",
        reg_val, ts_index, mb_index, node_id);

    if (reg_val <= 0xFFFF) {
        struct drv_hwts_mailbox *mb = (struct drv_hwts_mailbox *)hwts_get_mailbox_addr(mb_index, ts_index, node_id);
        mb->blockId = reg_val;
        ts_drv_debug("ack to hwts following another task, mb_index = %u, ts_index = %u\n", mb_index, ts_index);
        ts_info = &g_drv_ctrl_hwts[node_id]->ts_info[ts_index];
        tasklet_schedule(&ts_info->int_context[mb_index].find_pid_then_submit_task);
    } else {
        ts_drv_debug("ack to hwts following no task, ts_index(%u), mb_index(%u), node_id(%d).\n",
            ts_index, mb_index, node_id);
    }
}

void hwts_handle_ack_fail(u32 mb_index, u32 ts_index)
{
    u32 mb_shift = mb_index * MAILBOX_REG_SHIFT;
    u64 value = AICPU_STATUS_EXCEPTION;
    u32 node_id = numa_node_id();

    ts_drv_info("ack to hwts, aicpu execute block exception, wreg val=0x%llx", value);
#ifndef AOS_LLVM_BUILD
    writeq_relaxed(value, g_hwts_base_va[node_id][ts_index] + g_aicpu_status_report_ns[ts_index] + mb_shift);
#else
    writeq(value, g_hwts_base_va[node_id][ts_index] + g_aicpu_status_report_ns[ts_index] + mb_shift);
#endif
}

static void hwts_handle_ack_eos(u32 mb_index, u32 ts_index)
{
    u32 mb_shift = mb_index * MAILBOX_REG_SHIFT;
    u64 value = AICPU_STATUS_END_OF_SEQUENCE;
    u32 node_id = (u32)numa_node_id();

    ts_drv_info("Ack to hwts, aicpu end of sequence. (wreg_val=0x%llx)\n", value);
#ifndef AOS_LLVM_BUILD
    writeq_relaxed(value, g_hwts_base_va[node_id][ts_index] + g_aicpu_status_report_ns[ts_index] + mb_shift);
#else
    writeq(value, g_hwts_base_va[node_id][ts_index] + g_aicpu_status_report_ns[ts_index] + mb_shift);
#endif
}

int hwts_drv_sched_ack(unsigned int devid, unsigned int subevent_id, const char *msg, unsigned int msg_len, void *priv)
{
    u32 mb_index;
    u32 ts_index;
    struct hwts_response *resp = NULL;
    if (msg == NULL) {
        ts_drv_err("ack msg is null.\n");
        return -1;
    }
    resp = (struct hwts_response *)msg;

    ts_index = subevent_id;
    mb_index = resp->mailbox_id;
    if ((ts_index >= MAX_TS_NUM) || (mb_index >= AICPU_MAILBOX_MAX_NUM)) {
        ts_drv_err("ts_index %u, mb_index %u out of range max, %d, %d", ts_index, mb_index,
                   MAX_TS_NUM, AICPU_MAILBOX_MAX_NUM);
        return TS_PARA_ERR;
    }

    if (resp->result == 0) {
        hwts_handle_ack_success(mb_index, ts_index);
    } else if (resp->result == AE_STATUS_END_OF_SEQUENCE) {
        hwts_handle_ack_eos(mb_index, ts_index);
    } else {
        hwts_handle_ack_fail(mb_index, ts_index);
    }
    ts_drv_debug("Check. (tsid=%u; mb_index=%u; result=%u)\n", ts_index, mb_index, resp->result);

    return 0;
}

void hwts_drv_submit_task(unsigned long data)
{
    struct sched_published_event publish_event = {0};
    struct drv_hwts_int_context *int_context = NULL;
    struct hwts_ts_task ts_task = {0};
    struct drv_hwts_mailbox *mailbox_info = NULL;
    enum devdrv_process_type cp_type;
    void __iomem *mailbox_addr = NULL;
    u32 hpid;
    u32 idx;
    u32 ts_idx;
    int ret;
    int node_id;
    int pid;
    u32 vfid;

    int_context = (struct drv_hwts_int_context *)(uintptr_t)data;
    if ((int_context->mailbox_index >= AICPU_MAILBOX_MAX_NUM) ||
        (int_context->ts_index >= MAX_TS_NUM)) {
        ts_drv_err("mailbox_index(%u) >= AICPU_MAILBOX_MAX_NUM(%d) or ts_index(%u) >= MAX_TS_NUM(%d)\n",
            int_context->mailbox_index, AICPU_MAILBOX_MAX_NUM, int_context->ts_index, MAX_TS_NUM);
        return;
    }

    idx = int_context->mailbox_index;
    ts_idx = int_context->ts_index;
    node_id = int_context->node_id;
    mailbox_addr = hwts_get_mailbox_addr(idx, ts_idx, node_id);
    hpid = *((int *)mailbox_addr);
    mailbox_info = (struct drv_hwts_mailbox *)mailbox_addr;

    ts_task.mailbox_id = idx;
    ts_task.kernel_info.pid = mailbox_info->pid;
    ts_task.kernel_info.kernel_type = mailbox_info->kernel_type;
    ts_task.kernel_info.streamID = mailbox_info->streamID;
    ts_task.kernel_info.kernelName = mailbox_info->kernelName;
    ts_task.kernel_info.kernelSo = mailbox_info->kernelSo;
    ts_task.kernel_info.paramBase = mailbox_info->paramBase;
    ts_task.kernel_info.l2VaddrBase = mailbox_info->l2VaddrBase;
    ts_task.kernel_info.l2Ctrl = mailbox_info->l2Ctrl;
    ts_task.kernel_info.blockId = mailbox_info->blockId;
    ts_task.kernel_info.blockNum = mailbox_info->blockNum;
    ts_task.kernel_info.l2InMain = mailbox_info->l2InMain;
    ts_task.kernel_info.taskID = mailbox_info->taskID;
    vfid = mailbox_info->vfid;

    if (mailbox_info->kernel_type == KERNEL_TYPE_CUSTOMAICPU) {
        cp_type = DEVDRV_PROCESS_CP2;
    } else {
        cp_type = DEVDRV_PROCESS_CP1;
    }

    ret = devdrv_query_process_by_host_pid(hpid, (unsigned int)node_id, cp_type, vfid, &pid);
    if (unlikely(ret)) {
        ts_drv_err("query pid by host pid %u error vfid %u ret %d\n", hpid, vfid, ret);
        return;
    }

    ts_drv_debug("pid %d, hpid %u, vfid %u, cp_type %d\n", pid, hpid, vfid, (int)cp_type);
    publish_event.event_info.pid = pid;
    publish_event.event_info.gid = 0;
    publish_event.event_info.event_id = EVENT_TS_HWTS_KERNEL;
    publish_event.event_info.subevent_id = ts_idx;
    publish_event.event_info.msg = (char *)(&ts_task);
    publish_event.event_info.msg_len = sizeof(struct hwts_ts_task);

    publish_event.event_func.event_ack_func = hwts_drv_sched_ack;
    publish_event.event_func.event_finish_func = NULL;

    ret = sched_submit_event((unsigned int)node_id, &publish_event);
    if (unlikely(ret)) {
        ts_drv_err("submit kernel event error ret:%d node_id:%d pid:%d\n", ret, node_id, pid);
        return;
    }

    return;
}

irqreturn_t hwts_drv_irq_callback(int irq, void *data)
{
    /* be careful of disable interrupt ,enbale interrupt */
    struct drv_hwts_ts_info *ts_info = NULL;
    unsigned long flags;
    u64 reg_value;
    u32 irq_status;
    u32 mailbox_index;
    u32 node_id;
    u32 ts_id;

    if (data == NULL) {
        ts_drv_err("param is null. irq = %d\n", irq);
        return IRQ_NONE;
    }

    ts_info = (struct drv_hwts_ts_info *)data;
    ts_id = ts_info->ts_index;
    node_id = ts_info->int_context[0].node_id;

    local_irq_save(flags);
#ifndef AOS_LLVM_BUILD
    reg_value = readq_relaxed((void *)(g_hwts_base_va[node_id][ts_id] + g_aicpu_ks_status_ns[ts_id]));
#else
    reg_value = readq((void *)(g_hwts_base_va[node_id][ts_id] + g_aicpu_ks_status_ns[ts_id]));
#endif
    irq_status = reg_value & 0xFFFF;

    for (mailbox_index = 0; mailbox_index < AICPU_MAILBOX_MAX_NUM; mailbox_index++) {
        if ((irq_status & 0x1) == 1) {
            tasklet_schedule(&ts_info->int_context[mailbox_index].find_pid_then_submit_task);
        }
        irq_status >>= 1;
    }
#ifndef AOS_LLVM_BUILD
    writeq_relaxed(reg_value, (void *)(g_hwts_base_va[node_id][ts_id] + g_aicpu_ks_status_ns[ts_id]));
#else
    writeq(reg_value, (void *)(g_hwts_base_va[node_id][ts_id] + g_aicpu_ks_status_ns[ts_id]));
#endif
    local_irq_restore(flags);

    return IRQ_HANDLED;
}

void hwts_init_mailbox(struct drv_hwts_ctrl *hwts, int node_id)
{
    void __iomem  *mailaddr = NULL;
    u32 i = 0;
    u32 j = 0;

    for (i = 0; i < MAX_TS_NUM; i++) {
        for (j = 0; j < hwts->ts_info[i].irq_info.aicpu_num; j++) {
            mailaddr = hwts_get_return_mailbox_addr(j, i, node_id);
            hwts_drv_writel_relaxed(mailaddr, 0);
        }
    }
}

int hwts_drv_register_irq_to_cores(struct drv_hwts_ctrl *hwts,  struct platform_device *pdev)
{
    int ret = 0;
    int i;
    u32 j;
#if !defined(CFG_SOC_PLATFORM_MDC_V51) && !defined(CFG_SOC_PLATFORM_HELPER)
    u32 node_id, ccpu_first, ccpu_last;
#endif

    if ((hwts == NULL) || (pdev == NULL)) {
        ts_drv_err("hwts_ctrl or pdev is null.aicpu_ctrl = %pK, pdev = %pK\n", hwts, pdev);
        return -EINVAL;
    }

    for (i = 0; i < MAX_TS_NUM; i++) {
        for (j = 0; j < AICPU_MAILBOX_MAX_NUM; j++) {
            hwts->ts_info[i].int_context[j].ts_index = i;
            hwts->ts_info[i].int_context[j].mailbox_index = j;
            hwts->ts_info[i].int_context[j].node_id = hwts->numa_node;
            /* tasklet init */
            tasklet_init(&hwts->ts_info[i].int_context[j].find_pid_then_submit_task, hwts_drv_submit_task,
                         (unsigned long)(uintptr_t)&hwts->ts_info[i].int_context[j]);
        }

        /* request irq */
        ret = devm_request_irq(&pdev->dev, hwts->ts_info[i].irq_info.com_irq_ts_to_aicpu,
                               hwts_drv_irq_callback, IRQF_TRIGGER_NONE, "hwts_drv_interrupt", &hwts->ts_info[i]);
        if (ret != 0) {
            ts_drv_err("request irq failed, ret = %d.\n", ret);
            goto request_irq_fail;
        }

#ifdef CFG_SOC_PLATFORM_MDC_V51
        if (bind_irq_to_core(hwts->ts_info[i].irq_info.com_irq_ts_to_aicpu, "tsdrvcpu", "ts_mailbox_irq_cpu") != 0) {
            (void)irq_set_affinity_hint(hwts->ts_info[i].irq_info.com_irq_ts_to_aicpu,
                get_cpu_mask(HWTS_IRQ_AFFINITY_CPU_ID));
        }
#else
#ifndef CFG_SOC_PLATFORM_HELPER
        node_id = dev_to_node(&pdev->dev);
        ret = hwts_get_ccpu_index_range(node_id, &ccpu_first, &ccpu_last);
        ts_drv_debug("node_id(%u), ccpu_first(%u), ccput_last(%u).\n", node_id, ccpu_first, ccpu_last);
        if (ret == 0) {
            (void)irq_set_affinity_hint(hwts->ts_info[i].irq_info.com_irq_ts_to_aicpu,
                get_cpu_mask(ccpu_first));
        }
#endif
#endif
    }
    return ret;

request_irq_fail:
    for (j = 0; j < AICPU_MAILBOX_MAX_NUM; j++) {
        tasklet_kill(&hwts->ts_info[i].int_context[j].find_pid_then_submit_task);
    }
    for (i = i - 1; i >= 0; i--) {
        devm_free_irq(&pdev->dev, hwts->ts_info[i].irq_info.com_irq_ts_to_aicpu, hwts);
        for (j = 0; j < AICPU_MAILBOX_MAX_NUM; j++) {
            tasklet_kill(&hwts->ts_info[i].int_context[j].find_pid_then_submit_task);
        }
    }
    return ret;
}

int hwts_drv_get_irq(struct platform_device *pdev, struct drv_hwts_ctrl *hwts, int node_id)
{
    int ret = 0;
    u32 i;
    u32 irq_os_proxy = 0;
    struct device_node *node = NULL;
    struct device_node *ts_node = NULL;

    // read interrupt from DTS ts node
    node = pdev->dev.of_node;
    i = 0;
    for_each_child_of_node(node, ts_node) {
        if (i < MAX_TS_NUM) {
            irq_os_proxy = irq_of_parse_and_map(ts_node, 0);
            if (irq_os_proxy == 0) {
                ts_drv_err("hwts get os-proxy-irq failed.\n");
                ret = -EINVAL;
                return ret;
            }
            hwts->ts_info[i].irq_info.com_irq_ts_to_aicpu = irq_os_proxy;
#ifndef AOS_LLVM_BUILD
            ts_drv_info("*** read ts_full_name(%s), com_irq_ts_to_aicpu(%u) ***\n", \
                        ts_node->full_name, irq_os_proxy);
#else
            ts_drv_info("*** read ts_full_name(%s), com_irq_ts_to_aicpu(%u) ***\n", \
                        of_get_name(ts_node), irq_os_proxy);
#endif
        } else {
            ts_drv_err("invalid node id: %u\n", i);
            break;
        }
        i++;
    }

    return ret;
}

int hwts_drv_mailbox_addr_config(struct drv_hwts_ctrl *hwts)
{
#define CUR_MAILBOX_SHIFT     48
#define CUR_MAILBOX_ADDR_MASK 0xFFFFFFFFFFFF

    u32 node_id = hwts->numa_node;
    u64 mailbox_shift;
    u64 mailbox_addr;
    u64 reg_val;
    u32 i, j;

    for (i = 0; i < MAX_TS_NUM; i++) {
        mailbox_addr = hwts->ts_info[i].sram_info.sram_pa_base;
        mailbox_shift = MAILBOX_SHIFT;

        g_hwts_base_va[node_id][i] = ioremap(g_hwts_base_addr[i] + (node_id * HWTS_CHIP_BASEADDR_PA_OFFSET),
            g_hwts_reg_size[i]);
        if (g_hwts_base_va[node_id][i] == NULL) {
            ts_drv_err("ioremap failed. nodeid(%u), tsid(%u)\n", node_id, i);
            goto ERR;
        }

        reg_val = (mailbox_addr & CUR_MAILBOX_ADDR_MASK) | (mailbox_shift << CUR_MAILBOX_SHIFT);
#ifndef AOS_LLVM_BUILD
        writeq_relaxed(reg_val, (void *)(g_hwts_base_va[node_id][i] + g_hwts_aicpu_ns_mb_base_addr_cfg[i]));
#else
        writeq(reg_val, (void *)(g_hwts_base_va[node_id][i] + g_hwts_aicpu_ns_mb_base_addr_cfg[i]));
#endif
    }
    return 0;

ERR:
    for (j = 0; j < i; j++) {
        iounmap(g_hwts_base_va[node_id][j]);
    }
    return -EINVAL;
}

int hwts_drv_va_base_ioremap(struct platform_device *pdev, struct drv_hwts_ctrl *hwts)
{
    int i, j;

    for (i = 0; i < MAX_TS_NUM; i++) {
        hwts->ts_info[i].ts_aicpu_va_base = devm_ioremap(&pdev->dev,
            hwts->ts_info[i].ts_status_info.ts_aicpu_status_base,
            hwts->ts_info[i].ts_status_info.ts_aicpu_status_size);
        if (hwts->ts_info[i].ts_aicpu_va_base == NULL) {
            ts_drv_err("ioremap failed.\n");
            goto ts_aicpu_va_base_failed;
        }
    }

    /* wait ts ok! */
    if (hwts_drv_wait_ts_ready(hwts)) {
        goto ts_aicpu_va_base_failed;
    }
    /* ioremap sram_va_base map to device no cache property */
    for (j = 0; j < MAX_TS_NUM; j++) {
        hwts->ts_info[j].sram_info.sram_va_base = devm_ioremap(&pdev->dev,
            hwts->ts_info[j].sram_info.sram_pa_base, hwts->ts_info[j].sram_info.sram_pa_size);
        if (hwts->ts_info[j].sram_info.sram_va_base == NULL) {
            ts_drv_err("ioremap failed.\n");
            goto sram_va_base_failed;
        }
    }

    return 0;

sram_va_base_failed:
    for (j = j - 1; j >= 0; j--) {
        devm_iounmap(&pdev->dev, hwts->ts_info[j].sram_info.sram_va_base);
        hwts->ts_info[j].sram_info.sram_va_base = NULL;
    }
ts_aicpu_va_base_failed:
    for (i = i - 1; i >= 0; i--) {
        devm_iounmap(&pdev->dev, hwts->ts_info[i].ts_aicpu_va_base);
        hwts->ts_info[i].ts_aicpu_va_base = NULL;
    }

    return -EINVAL;
}

STATIC void __iomem *hwts_drv_remap_config_system_addr(u32 tsid, int node_id)
{
#ifdef CFG_SOC_PLATFORM_MDC_V51
    return ioremap(HWTS_SYSTEM_CONFIG_BASE(node_id) +
            ((u64)tsid * HWTS_SYSTEM_CONFIG_SIZE), HWTS_SYSTEM_CONFIG_SIZE);
#else
    return ioremap_cache(HWTS_SYSTEM_CONFIG_BASE(node_id) +
            ((u64)tsid * HWTS_SYSTEM_CONFIG_SIZE), HWTS_SYSTEM_CONFIG_SIZE);
#endif
}

STATIC void hwts_drv_unmap_config_system_addr(void __iomem *vaddr)
{
    iounmap(vaddr);
}

int hwts_drv_get_config_info(struct drv_hwts_ctrl *hwts, struct platform_device *pdev)
{
    struct aicpu_system_config *config_system[MAX_TS_NUM] = {0};
    void __iomem *vaddr[MAX_TS_NUM]  = {NULL};
    int ret = 0;
    u32 i = 0;
    int node_id;

    if ((hwts == NULL) || (pdev == NULL)) {
        ts_drv_err("hwts_config or pdev is null.hwts = %pK, pdev = %pK\n", hwts, pdev);
        return -EINVAL;
    }

    node_id = hwts->numa_node;

    /* get info from devdrv-manager */
    /* only read ,so map to cache property */
    ts_drv_info("start ioremap!\n");
    for (i = 0; i < MAX_TS_NUM; i++) {
        vaddr[i] = hwts_drv_remap_config_system_addr(i, node_id);
        ts_drv_info("finish ioremap,ts=%u,vaddr=%pK !\n", i, vaddr[i]);
    }

    for (i = 0; i < MAX_TS_NUM; i++) {
        ts_drv_info("start build config system i=%u, vaddr=%pK!\n", i, vaddr[i]);
        config_system[i] = (struct aicpu_system_config *)vaddr[i];
        if (config_system[i] == NULL) {
            ts_drv_err("system_config_base ioremap failed. i=%u dev_id:%d\n", i, node_id);
            ret = -EINVAL;
            goto exit;
        }

        if (config_system[i]->flag != HWTS_SYSTEM_CONFIG_FLAG) {
            ts_drv_err("hwt_config_system ts:%u is invalid. dev_id:%d\n", i, node_id);
            ret = -EINVAL;
            goto exit;
        }
    }

    for (i = 0; i < MAX_TS_NUM; i++) {
        hwts->ts_info[i].ts_index = i;
        hwts->ts_info[i].flag = config_system[i]->flag;
        hwts->ts_info[i].ts_status_info.ts_aicpu_status_base = config_system[i]->ts_aicpu_status_base;
        hwts->ts_info[i].ts_status_info.ts_aicpu_status_size = PAGE_SIZE;

        hwts->ts_info[i].sram_info.sram_pa_base = config_system[i]->sram_pa_base;
        hwts->ts_info[i].sram_info.sram_pa_size = config_system[i]->sram_pa_size;

        hwts->ts_info[i].gicd_info.gicd_pa_base = config_system[i]->gicd_pa_base;
        hwts->ts_info[i].gicd_info.gicd_pa_size = config_system[i]->gicd_pa_size;
        hwts->ts_info[i].gicd_info.gic_multichip_off = config_system[i]->gic_multichip_off;

        hwts->ts_info[i].irq_info.aicpu_id_base = config_system[i]->aicpu_id_base;
        hwts->ts_info[i].irq_info.aicpu_num = config_system[i]->aicpu_num;
        if (hwts->ts_info[i].irq_info.aicpu_num > AICPU_MAX_NUM) {
            ts_drv_err("aicpu_num:%u invalid. dev_id:%d, tsid:%u\n", hwts->ts_info[i].irq_info.aicpu_num, node_id, i);
            ret = -EINVAL;
        }
        hwts->ts_info[i].irq_info.ts_int_start_id = config_system[i]->ts_int_start_id;

        ts_drv_debug("num:%u.\n", hwts->ts_info[i].irq_info.aicpu_num);
        ts_drv_debug("start_id:%u.\n", hwts->ts_info[i].irq_info.ts_int_start_id);
    }

exit:
    for (i = 0; i < MAX_TS_NUM; i++) {
        if (vaddr[i] == NULL) {
            continue;
        }
        ts_drv_info("start iounmap vaddr!\n");
        hwts_drv_unmap_config_system_addr(vaddr[i]);
    }

    return ret;
}

int hwts_drv_resume(struct drv_hwts_ctrl *hwts)
{
#ifdef CFG_SOC_PLATFORM_MDC_V51
    int node_id = numa_node_id();
    int ret;
    u32 i;

    if (hwts == NULL) {
        ts_drv_err("hwts_ctrl is null.\n");
        return -EINVAL;
    }

    ret = hwts_drv_mailbox_addr_config(hwts);
    if (ret != 0) {
        ts_drv_err("failed to config mailbox addr. ret=%d\n", ret);
        return ret;
    }

    hwts_init_mailbox(hwts, node_id);

    for (i = 0; i < MAX_TS_NUM; i++) {
        if (bind_irq_to_core(hwts->ts_info[i].irq_info.com_irq_ts_to_aicpu, "tsdrvcpu", "ts_mailbox_irq_cpu")) {
            (void)irq_set_affinity_hint(hwts->ts_info[i].irq_info.com_irq_ts_to_aicpu,
                get_cpu_mask(HWTS_IRQ_AFFINITY_CPU_ID));
        }
    }
#endif
    return 0;
}

STATIC int ts_drv_pm_resume(u32 devid)
{
    struct drv_hwts_ctrl *hwts = NULL;
    int ret;

    if (devid >= CHIP_NUM_MAX) {
        ts_drv_err("node_id out range. node_id:%u, max:%d\n", devid, CHIP_NUM_MAX);
        return -EINVAL;
    }

    hwts = g_drv_ctrl_hwts[devid];
    if (hwts == NULL) {
        ts_drv_err("hwts is null. dev_id:%u\n", devid);
        return -EINVAL;
    }

    ret = hwts_drv_resume(hwts);
    if (ret != 0) {
        ts_drv_err("hwts resume failed. error=%d. dev_id:%u\n", ret, devid);
        return ret;
    }

    ts_drv_info("hwts resume success.\n");
    return 0;
}

STATIC int ts_drv_pm_suspend(u32 devid)
{
    ts_drv_info("ts drv pm suspend.\n");
    return 0;
}

void hwts_drv_release(struct platform_device *pdev, struct drv_hwts_ctrl *hwts)
{
    u32 i;
    u32 j;

    if ((pdev == NULL) || (hwts == NULL)) {
        return;
    }

    /* free irq and tasklet */
    for (i = 0; i < MAX_TS_NUM; i++) {
        (void)irq_set_affinity_hint(hwts->ts_info[i].irq_info.com_irq_ts_to_aicpu, NULL);
        devm_free_irq(&pdev->dev, hwts->ts_info[i].irq_info.com_irq_ts_to_aicpu, &hwts->ts_info[i]);
        for (j = 0; j < AICPU_MAILBOX_MAX_NUM; j++) {
            tasklet_kill(&hwts->ts_info[i].int_context[j].find_pid_then_submit_task);
        }
    }

    /* free iounmap */
    for (i = 0; i < MAX_TS_NUM; i++) {
        if (hwts->ts_info[i].sram_info.sram_va_base != NULL) {
            devm_iounmap(&pdev->dev, hwts->ts_info[i].sram_info.sram_va_base);
            hwts->ts_info[i].sram_info.sram_va_base = NULL;
        }
        if (hwts->ts_info[i].ts_aicpu_va_base != NULL) {
            devm_iounmap(&pdev->dev, hwts->ts_info[i].ts_aicpu_va_base);
            hwts->ts_info[i].ts_aicpu_va_base = NULL;
        }
    }

    for (i = 0; i < MAX_TS_NUM; i++) {
        if (g_hwts_base_va[hwts->numa_node][i] != NULL) {
            iounmap(g_hwts_base_va[hwts->numa_node][i]);
            g_hwts_base_va[hwts->numa_node][i] = NULL;
        }
    }
    return;
}

void hwts_drv_iounmap_resource(struct platform_device *pdev, struct drv_hwts_ctrl *hwts)
{
    int i;

    if ((pdev == NULL) || (hwts == NULL)) {
        return;
    }

    for (i = 0; i < MAX_TS_NUM; i++) {
        if (hwts->ts_info[i].sram_info.sram_va_base != NULL) {
            devm_iounmap(&pdev->dev, hwts->ts_info[i].sram_info.sram_va_base);
            hwts->ts_info[i].sram_info.sram_va_base = NULL;
        }
        if (hwts->ts_info[i].ts_aicpu_va_base != NULL) {
            devm_iounmap(&pdev->dev, hwts->ts_info[i].ts_aicpu_va_base);
            hwts->ts_info[i].ts_aicpu_va_base = NULL;
        }
    }
    return;
}

int hwts_drv_init(struct platform_device *pdev, struct drv_hwts_ctrl *hwts, int node_id)
{
    int ret;
    /* get config information */
    ret = hwts_drv_get_config_info(hwts, pdev);
    if (ret != 0) {
        ts_drv_err("hwts get config info failed. ret=%d dev_id:%d\n", ret, hwts->numa_node);
        return ret;
    }

    ret = hwts_drv_va_base_ioremap(pdev, hwts);
    if (ret != 0) {
        ts_drv_err("va base ioremap failed.\n");
        return ret;
    }

    ret = hwts_drv_mailbox_addr_config(hwts);
    if (ret != 0) {
        ts_drv_err("failed to config mailbox addr.\n");
        goto unmap;
    }

    ret = hwts_drv_get_irq(pdev, hwts, node_id);
    if (ret != 0) {
        ts_drv_err("failed to get irq.\n");
        goto unmap;
    }

    /* register irq to cores by irqs */
    ret = hwts_drv_register_irq_to_cores(hwts, pdev);
    if (ret != 0) {
        ts_drv_err("hwts register irq tp cores failed.ret=%d dev_id:%d\n", ret, hwts->numa_node);
        goto unmap;
    }

    /*
        1 init mailbox make valid clear
        2 set status to ts
    */
    hwts_init_mailbox(hwts, node_id);

    ret = tsdrv_pm_extend_set(0, ts_drv_pm_suspend, ts_drv_pm_resume);
    if (ret != 0) {
        ts_drv_err("hwts register pm function failed\n");
        goto unmap;
    }

    ts_drv_info("hwts drv load ok. \n");
    return 0;

unmap:
    hwts_drv_iounmap_resource(pdev, hwts);

    return ret;
}
#else /* TSDRV_UT */
void ut_device_hwts_drv_test(void)
{
}
#endif /* TSDRV_UT */
