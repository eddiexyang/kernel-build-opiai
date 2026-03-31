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
#ifndef TSDRV_UT
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/uio_driver.h>
#include "ts_api.h"

#include "hwts_drv_api.h"
#include "hwts_drv_config.h"
#include "hwts_task_info.h"
#include "event_sched_inner.h"
#include "ascend_hal_define.h"
#include "aicpu.h"
#include "devdrv_manager_comm.h"
#include "ts_aisle_queue.h"
#include "hwts_drv_ascend310.h"

#define AICPUINDEX_TO_AICPUID(nid, index) ((index) + (g_drv_ctrl_hwts[nid]->ts_info[0].irq_info.aicpu_id_base))

void __iomem *hwts_get_return_mailbox_addr(u32 mb_ind)
{
    u32 offset = 0;
    int node_id = numa_node_id();
    struct drv_hwts_ctrl *hwts = g_drv_ctrl_hwts[node_id];

    offset = mb_ind * (u32)(RET_MAILBOX_SIZE);
    return (void __iomem *)(hwts->ts_info[0].sram_info.sram_va_base + (AICPU_MAILBOX_MAX_NUM)*MAILBOX_SIZE + offset);
}

void __iomem *hwts_get_mailbox_addr(u32 mb_ind)
{
    unsigned int offset = 0;
    int node_id = numa_node_id();
    struct drv_hwts_ctrl *hwts = g_drv_ctrl_hwts[node_id];

    offset = mb_ind * (u32)MAILBOX_SIZE;
    return (void __iomem *)(hwts->ts_info[0].sram_info.sram_va_base + offset);
}

STATIC int hwts_drv_read_ts_ready_state(struct drv_hwts_ctrl *hwts, u32 *stat)
{
    int ret = 0;
    u32 i = 0;

    while ((*stat < TS_READY_MIN_VALUE) || (*stat == 0xffffffffUL)) {
        *stat = readl_relaxed(hwts->ts_info[0].ts_aicpu_va_base + TS_READY_STATE_REG_OFFSET);
        msleep(1);
        i++;
        if (i >= TS_READY_WAIT_COUNT) {
            ret = TS_INNER_ERR;
            break;
        }
    }

    return ret;
}

int hwts_drv_wait_ts_ready(struct drv_hwts_ctrl *hwts)
{
    u32 stat = 0;
    int ret;

    if (hwts == NULL) {
        ts_drv_err("param is null.\n");
        return TS_PARA_ERR;
    }

    ret = hwts_drv_read_ts_ready_state(hwts, &stat);
    if (ret == TS_INNER_ERR) {
        ts_drv_err("hwts wait TS SRAM timeout(12s), stat=%u. ret=%d dev_id:%d\n", stat, ret, hwts->numa_node);
    } else {
        ts_drv_debug("hwts drv TS SRAM init OK, then we remap sram. dev_id:%d\n", hwts->numa_node);
    }

    return ret;
}

void hwts_handle_ack_success(u32 cpu_index, u32 ts_index)
{
    return;
}

void hwts_clean_mailbox_flag(u32 cpu_index)
{
    struct drv_hwts_mailbox *mailbox_info = NULL;
    mailbox_info = (struct drv_hwts_mailbox *)hwts_get_mailbox_addr(cpu_index);
    /* mailbox clear to 0 */
    mailbox_info->valid = MAILBOX_INVALID;
}

STATIC void hwts_send_interrupt_to_ts(u32 cpu_index)
{
    u32 shiftoffset;
    u32 val;
    u32 irq;
    u32 igp;
    void __iomem *reg;
    int node_id = numa_node_id();
    struct drv_hwts_ctrl *hwts = g_drv_ctrl_hwts[node_id];

    irq = hwts->ts_info[0].irq_info.irq_aicpu_to_ts[cpu_index];
    shiftoffset = (irq % REG_LEN);
    val = (1U << shiftoffset);
    igp = (irq / (u32)REG_LEN) * (u32)REG_ALIGN;
    reg = (void __iomem *)(hwts->ts_info[0].gicd_info.gicd_va_base + GICD_ISPENDR + igp);

    hwts_drv_writel_relaxed(reg, val);
}

STATIC int hwts_write_ret_mail(u32 cpu_index, u32 type, u32 state, u64 serial_no)
{
    drv_hwts_task_response_t *msg = NULL;

    msg = (drv_hwts_task_response_t *)hwts_get_return_mailbox_addr(cpu_index);
    if (*(volatile unsigned int *)msg == MAILBOX_CONFIRM) {
        msg->valid = MAILBOX_VALID;
    } else {
        ts_drv_warn("this mailbox will miss, cpu_index=%u\n", cpu_index);
    }
    msg->state = state;
    msg->serial_no = serial_no;

    wmb();
    hwts_send_interrupt_to_ts(cpu_index);
    return 0;
}

int hwts_drv_sched_ack(unsigned int devid, unsigned int subevent_id, const char *msg, unsigned int msg_len, void *priv)
{
    u32 cpu_index;
    u32 ts_index;
    int ret;
    struct hwts_response *resp = NULL;

    if (msg == NULL) {
        ts_drv_err("ack msg is null.\n");
        return TS_PARA_ERR;
    }
    resp = (struct hwts_response *)msg;
    ts_index = subevent_id;
    cpu_index = resp->mailbox_id;
    if (cpu_index >= AICPU_MAILBOX_MAX_NUM) {
        ts_drv_err("invalid mb_index(%u)\n", cpu_index);
        return TS_PARA_ERR;
    }

    ts_drv_debug("mb_id=%u, ts_id=%u, pid=%d, serial=%llx.\n", cpu_index, ts_index, current->tgid, resp->serial_no);

    hwts_clean_mailbox_flag(cpu_index);
    ret = hwts_write_ret_mail(cpu_index, 0, resp->result, resp->serial_no);

    return ret;
}

int hwts_drv_thread_receive_confirm(volatile unsigned long long serial_no,
    unsigned int mailbox_index, unsigned int ts_index)
{
    struct drv_hwts_task_response *msg = NULL;

    msg = (struct drv_hwts_task_response *)hwts_get_return_mailbox_addr(mailbox_index);
    msg->valid = MAILBOX_CONFIRM;
    msg->state = 0;
    msg->serial_no = serial_no;

    return 0;
}

void hwts_drv_submit_task(unsigned long data)
{
    struct sched_published_event publish_event = { 0 };
    struct drv_hwts_int_context *int_context = NULL;
    struct drv_hwts_mailbox *mailbox_info = NULL;
    void __iomem *mailbox_addr = NULL;
    struct hwts_ts_task ts_task = { 0 };
    enum devdrv_process_type cp_type;
    u32 hpid;
    u32 node_id;
    int pid;
    int ret;
    u32 vfid = 0;
    u32 idx;
    u32 ts_idx;

    int_context = (struct drv_hwts_int_context *)(uintptr_t)data;
    if (int_context->mailbox_index >= AICPU_MAILBOX_MAX_NUM) {
        ts_drv_err("mailbox_index(%u) >= AICPU_MAILBOX_MAX_NUM(%d)\n", int_context->mailbox_index,
            AICPU_MAILBOX_MAX_NUM);
        return;
    }
    mailbox_addr = hwts_get_mailbox_addr(int_context->mailbox_index);
    mailbox_info = (struct drv_hwts_mailbox *)mailbox_addr;
    idx = int_context->mailbox_index;
    ts_idx = int_context->ts_index;
    hpid = mailbox_info->pid;

    ts_task.mailbox_id = int_context->mailbox_index;
    ts_task.serial_no = mailbox_info->serial_no;
    ts_task.kernel_info.pid = mailbox_info->pid;
    ts_task.kernel_info.kernel_type = mailbox_info->kernel_info.kernelType;
    if (mailbox_info->kernel_info.kernelType == KERNEL_TYPE_CCE ||
        mailbox_info->kernel_info.kernelType == KERNEL_TYPE_CTL ||
        mailbox_info->kernel_info.kernelType == KERNEL_TYPE_CUSTOMAICPU) {
        ts_task.kernel_info.kernelName = mailbox_info->kernel_info.kernelBase.cce_kernel.kernelName;
        ts_task.kernel_info.kernelSo = mailbox_info->kernel_info.kernelBase.cce_kernel.kernelSo;
        ts_task.kernel_info.paramBase = mailbox_info->kernel_info.kernelBase.cce_kernel.paramBase;
        ts_task.kernel_info.l2VaddrBase = mailbox_info->kernel_info.kernelBase.cce_kernel.l2VaddrBase;
        ts_task.kernel_info.blockId = mailbox_info->kernel_info.kernelBase.cce_kernel.blockId;
        ts_task.kernel_info.blockNum = mailbox_info->kernel_info.kernelBase.cce_kernel.blockNum;
        ts_task.kernel_info.l2InMain = mailbox_info->kernel_info.kernelBase.cce_kernel.l2InMain;
        ts_task.kernel_info.streamID = mailbox_info->kernel_info.kernelBase.cce_kernel.streamID;
        ts_task.kernel_info.taskID = mailbox_info->kernel_info.kernelBase.cce_kernel.taskID;
    } else if (mailbox_info->kernel_info.kernelType == KERNEL_TYPE_TF) {
        ts_task.kernel_info.paramBase = mailbox_info->kernel_info.kernelBase.fwk_kernel.kernel;
        ts_task.kernel_info.streamID = mailbox_info->kernel_info.kernelBase.fwk_kernel.streamID;
        ts_task.kernel_info.taskID = mailbox_info->kernel_info.kernelBase.fwk_kernel.taskID;
    }

    if (mailbox_info->kernel_info.kernelType == KERNEL_TYPE_CUSTOMAICPU) {
        cp_type = DEVDRV_PROCESS_CP2;
    } else {
        cp_type = DEVDRV_PROCESS_CP1;
    }
    node_id = cpu_to_node(smp_processor_id()); /*lint !e666  !e453*/
    ret = devdrv_query_process_by_host_pid(hpid, node_id, cp_type, vfid, &pid);
    if (unlikely(ret)) {
        ts_drv_err("query pid by host pid %u error ret:%d\n", hpid, ret);
        return;
    }

    ts_drv_debug("submit_task, node_id=%u, mb_id=%u, ts_id=%u, pid=%d, hpid=%u, vfid=%u, cp_type=%d, serial=%llx.\n",
        node_id, idx, ts_idx, pid, hpid, vfid, (int)cp_type, mailbox_info->serial_no);

    publish_event.event_info.pid = pid;
    publish_event.event_info.gid = 0;
    publish_event.event_info.event_id = EVENT_TS_HWTS_KERNEL;
    publish_event.event_info.subevent_id = 0;
    publish_event.event_info.msg = (char *)(&ts_task);
    publish_event.event_info.msg_len = sizeof(struct hwts_ts_task);
    publish_event.event_func.event_ack_func = hwts_drv_sched_ack;
    publish_event.event_func.event_finish_func = NULL;

    ret = hwts_drv_thread_receive_confirm(mailbox_info->serial_no, idx, ts_idx);
    if (ret != 0) {
        ts_drv_warn("HostPid[%u], pid[%d], aicpu[%u], send confirm to ts[%u] failed, error: %d.\n",
            hpid, pid, idx, ts_idx, ret);
    }

    ret = sched_submit_event(node_id, &publish_event);
    if (unlikely(ret)) {
        ts_drv_err("submit kernel event error ret:%d\n", ret);
        return;
    }

    return;
}

irqreturn_t hwts_drv_irq_callback(int irq, void *data)
{
    /* be careful of disable interrupt ,enbale interrupt */
    struct drv_hwts_int_context *int_context = NULL;
    int node_id = numa_node_id();
    unsigned long flags;

    if (data == NULL) {
        ts_drv_err("param is null. irq = %d node_id = %d\n", irq, node_id);
        return IRQ_NONE;
    }

    local_irq_save(flags);
    int_context = (struct drv_hwts_int_context *)data;
    tasklet_schedule(&int_context->find_pid_then_wakeup_task);
    local_irq_restore(flags);

    return IRQ_HANDLED;
}

void hwts_init_mailbox(struct drv_hwts_ctrl *hwts, int node_id)
{
    void __iomem *mailaddr = NULL;
    u32 i = 0;

    for (i = 0; i < hwts->ts_info[0].irq_info.aicpu_num; i++) {
        mailaddr = hwts_get_return_mailbox_addr(i);
        hwts_drv_writel_relaxed(mailaddr, 0);
    }
}

int hwts_drv_register_irq_to_cores(struct drv_hwts_ctrl *hwts, struct platform_device *pdev)
{
    int ret;
    u32 i, j;
    int node_id;
    u32 cpu_id;
    u32 aicpu_num;

    if ((hwts == NULL) || (pdev == NULL)) {
        ts_drv_err("hwts ctrl or pdev is null.hwts_ctrl = %pK, pdev = %pK\n", hwts, pdev);
        return TS_PARA_ERR;
    }

    node_id = hwts->numa_node;
    aicpu_num = hwts->ts_info[0].irq_info.aicpu_num;
    if (aicpu_num > AICPU_MAX_NUM) {
        ts_drv_err("aicpu_num(%u) > AICPU_MAX_NUM(%d) please check!\n", aicpu_num, AICPU_MAX_NUM);
        return TS_PARA_ERR;
    }

    /* request irq */
    for (i = 0; i < hwts->ts_info[0].irq_info.aicpu_num; i++) {
        hwts->ts_info[0].int_context[i].mailbox_index = i;
        /* tasklet init */
        tasklet_init(&hwts->ts_info[0].int_context[i].find_pid_then_wakeup_task, hwts_drv_submit_task,
            (unsigned long)(uintptr_t)&hwts->ts_info[0].int_context[i]);
        ret = devm_request_irq(&pdev->dev, hwts->ts_info[0].irq_info.irq_ts_to_aicpu_os_proxy[i], hwts_drv_irq_callback,
            IRQF_TRIGGER_RISING, "hwts_drv_interrupt", &hwts->ts_info[0].int_context[i]);
        if (ret != 0) {
            ts_drv_err("request irq failed, ret=%d dev_id:%d.\n", ret, node_id);
            goto request_irq_fail;
        }
        /*
                   performance improvement, we affinity
                   interrupts to aicpu-PEs
        */
        if (hwts->ts_info[0].chip_info.chip_type == AICPU_PLATFORM_FPGA) {
            cpu_id = (u32)(CPU_NUM_EACH_NODE_OF_FPGA * node_id + 1); /* OS uses logic core id */
        } else {
            cpu_id = (hwts->ts_info[0].irq_info.aicpu_id_base + hwts->ts_info[0].irq_info.aicpu_num) * node_id +
                AICPUINDEX_TO_AICPUID(node_id, i);
        }
        cpumask_set_cpu(cpu_id, &(hwts->ts_info[0].int_context[i].cpumask));
        (void)irq_set_affinity_hint((u32)hwts->ts_info[0].irq_info.irq_ts_to_aicpu_os_proxy[i],
            &(hwts->ts_info[0].int_context[i].cpumask));
    }
    return 0;

request_irq_fail:
    j = i;
    for (i = 0; i < j; i++) {
        (void)irq_set_affinity_hint((u32)hwts->ts_info[0].irq_info.irq_ts_to_aicpu_os_proxy[i], NULL);
        devm_free_irq(&pdev->dev, hwts->ts_info[0].irq_info.irq_ts_to_aicpu_os_proxy[i],
            &hwts->ts_info[0].int_context[i]);
    }
    for (i = 0; i <= j; i++) {
        tasklet_kill(&hwts->ts_info[0].int_context[i].find_pid_then_wakeup_task);
    }
    return ret;
}

int hwts_drv_get_irq(struct platform_device *pdev, struct drv_hwts_ctrl *hwts, int node_id)
{
    int irq_os_proxy = -1;
    u32 aicpu_id;
    u32 hw_irq;
    u32 i;

    for (i = 0; i < hwts->ts_info[0].irq_info.aicpu_num; i++) {
        aicpu_id = (AICPUINDEX_TO_AICPUID(node_id, i));
        hw_irq = hwts->ts_info[0].irq_info.irq_ts_to_aicpu[i];
        irq_os_proxy = platform_get_irq(pdev, aicpu_id);
        if (irq_os_proxy < 0) {
            ts_drv_err("hwts get os-proxy-irq failed.\n");
            return TS_INNER_ERR;
        }
        hwts->ts_info[0].irq_info.irq_ts_to_aicpu_os_proxy[i] = irq_os_proxy;
        ts_drv_debug("*** read cpu_id(%u), os proxy-irq(%d), hw irq(%u) ***\n", aicpu_id, irq_os_proxy, hw_irq);
    }

    return 0;
}

int hwts_drv_mailbox_addr_config(struct drv_hwts_ctrl *hwts)
{
    return 0;
}

int hwts_drv_va_base_ioremap(struct platform_device *pdev, struct drv_hwts_ctrl *hwts)
{
    int ret = 0;
    hwts->ts_info[0].ts_aicpu_va_base = devm_ioremap(&pdev->dev,
        hwts->ts_info[0].ts_status_info.ts_aicpu_status_base, hwts->ts_info[0].ts_status_info.ts_aicpu_status_size);
    if (hwts->ts_info[0].ts_aicpu_va_base == NULL) {
        ts_drv_err("ioremap failed.\n");
        goto ts_aicpu_va_base_failed;
    }
    hwts->ts_info[0].gicd_info.gicd_va_base = devm_ioremap(&pdev->dev, hwts->ts_info[0].gicd_info.gicd_pa_base,
        hwts->ts_info[0].gicd_info.gicd_pa_size);
    if (hwts->ts_info[0].gicd_info.gicd_va_base == NULL) {
        ts_drv_err("ioremap failed.\n");
        goto gicd_va_base_failed;
    }
    /* wait ts ok! */
    ret = hwts_drv_wait_ts_ready(hwts);
    if (ret != 0) {
        goto sram_va_base_failed;
    }
    /* ioremap sram_va_base map to device no cache property */
    hwts->ts_info[0].sram_info.sram_va_base = devm_ioremap(&pdev->dev, hwts->ts_info[0].sram_info.sram_pa_base,
        hwts->ts_info[0].sram_info.sram_pa_size);
    if (hwts->ts_info[0].sram_info.sram_va_base == NULL) {
        ts_drv_err("ioremap failed.\n");
        goto sram_va_base_failed;
    }
    return 0;

sram_va_base_failed:
gicd_va_base_failed:
    devm_iounmap(&pdev->dev, hwts->ts_info[0].gicd_info.gicd_va_base);
    hwts->ts_info[0].gicd_info.gicd_va_base = NULL;
ts_aicpu_va_base_failed:
    devm_iounmap(&pdev->dev, hwts->ts_info[0].ts_aicpu_va_base);
    hwts->ts_info[0].ts_aicpu_va_base = NULL;

    return ret;
}

STATIC void hwts_drv_get_chip_info(u32 *chip_id, u32 *chip_type)
{
    if (chip_id != NULL) {
        *chip_id = AICPU_CHIP_ID_MINI;
    }
    if (chip_type != NULL) {
        *chip_type = AICPU_PLATFORM_ASIC;
    }
}

STATIC void hwts_drv_init_common_config(struct drv_hwts_ts_info *ts_info, struct aicpu_system_config *config_system,
    int node_id)
{
    hwts_drv_get_chip_info(&(ts_info->chip_info.chip_id), &(ts_info->chip_info.chip_type));
    ts_info->ts_index = 0;
    ts_info->flag = config_system->flag;
    ts_info->ts_status_info.ts_aicpu_status_base = config_system->ts_aicpu_status_base;
    ts_info->ts_status_info.ts_aicpu_status_size = PAGE_SIZE;
    ts_info->sram_info.sram_pa_base = config_system->sram_pa_base;
    ts_info->sram_info.sram_pa_size = config_system->sram_pa_size;
    ts_info->gicd_info.gicd_pa_base = config_system->gicd_pa_base;
    ts_info->gicd_info.gicd_pa_size = config_system->gicd_pa_size;
    ts_info->gicd_info.gic_multichip_off = config_system->gic_multichip_off;
    ts_info->irq_info.aicpu_id_base = config_system->aicpu_id_base;
    ts_info->irq_info.aicpu_num = config_system->aicpu_num;
    ts_info->irq_info.ts_int_start_id = config_system->ts_int_start_id;
    ts_drv_debug("num:%u.\n", ts_info->irq_info.aicpu_num);
    ts_drv_debug("start_id:%u.\n", ts_info->irq_info.ts_int_start_id);
}


int hwts_drv_get_config_info(struct drv_hwts_ctrl *hwts, struct platform_device *pdev)
{
    struct aicpu_system_config *config_system = NULL;
    void __iomem *vaddr = NULL;
    int node_id;
    u32 i = 0;
    u32 aicpu_start_int = 0;
    node_id = hwts->numa_node;
    vaddr = devm_ioremap(&pdev->dev, SYSTEM_CONFIG_BASE_PLAT(node_id), SYSTEM_CONFIG_SIZE);
    if (vaddr == NULL) {
        ts_drv_err("config_system ioremap failed. dev_id:%d\n", node_id);
        return TS_INNER_ERR;
    }
    config_system = (struct aicpu_system_config *)vaddr;
    if (config_system->flag != SYSTEM_CONFIG_FLAG) {
        ts_drv_err("aicpu_config_system is invalid. dev_id:%d\n", node_id);
        devm_iounmap(&pdev->dev, vaddr);
        return TS_INNER_ERR;
    }

    hwts_drv_init_common_config(&hwts->ts_info[0], config_system, node_id);
    if (hwts->ts_info[0].irq_info.aicpu_num > AICPU_MAX_NUM) {
        ts_drv_err("aicpu_num:%u is invalid. dev_id:%d\n", hwts->ts_info[0].irq_info.aicpu_num, node_id);
        devm_iounmap(&pdev->dev, vaddr);
        return TS_INNER_ERR;
    }
    /* calculate irqs */
    for (i = 0; i < hwts->ts_info[0].irq_info.aicpu_num; i++) {
        aicpu_start_int = hwts->ts_info[0].irq_info.ts_int_start_id + hwts->ts_info[0].gicd_info.gic_multichip_off;
        hwts->ts_info[0].irq_info.irq_aicpu_to_ts[i] =
            (aicpu_start_int + IRQ_AICPU_TO_TS_OFFSET * (AICPUINDEX_TO_AICPUID(node_id, i)));
        hwts->ts_info[0].irq_info.irq_ts_to_aicpu[i] = hwts->ts_info[0].irq_info.irq_aicpu_to_ts[i] + 1;
        ts_drv_debug("_to_ts:%u.\n", hwts->ts_info[0].irq_info.irq_aicpu_to_ts[i]);
        ts_drv_debug("_to_aicpu:%u.\n", hwts->ts_info[0].irq_info.irq_ts_to_aicpu[i]);
    }

    devm_iounmap(&pdev->dev, vaddr);
    vaddr = NULL;

    return TS_INNER_SUCCESS;
}

void hwts_drv_iounmap_resource(struct platform_device *pdev, struct drv_hwts_ctrl *hwts)
{
    devm_iounmap(&pdev->dev, hwts->ts_info[0].sram_info.sram_va_base);
    hwts->ts_info[0].sram_info.sram_va_base = NULL;
    devm_iounmap(&pdev->dev, hwts->ts_info[0].ts_aicpu_va_base);
    hwts->ts_info[0].ts_aicpu_va_base = NULL;

    devm_iounmap(&pdev->dev, hwts->ts_info[0].gicd_info.gicd_va_base);
    hwts->ts_info[0].gicd_info.gicd_va_base = NULL;
}

STATIC void hwts_drv_free_irq(struct platform_device *pdev, struct drv_hwts_ctrl *hwts)
{
    u32 i = 0;

    for (i = 0; i < hwts->ts_info[0].irq_info.aicpu_num; i++) {
        (void)irq_set_affinity_hint((u32)hwts->ts_info[0].irq_info.irq_ts_to_aicpu_os_proxy[i], NULL);
        devm_free_irq(&pdev->dev, hwts->ts_info[0].irq_info.irq_ts_to_aicpu_os_proxy[i],
            &hwts->ts_info[0].int_context[i]);
        tasklet_kill(&hwts->ts_info[0].int_context[i].find_pid_then_wakeup_task);
    }
}

void hwts_drv_release(struct platform_device *pdev, struct drv_hwts_ctrl *hwts)
{
    /* free irq and tasklet */
    hwts_drv_free_irq(pdev, hwts);
    /* iounmap pdev resource */
    hwts_drv_iounmap_resource(pdev, hwts);
}

int hwts_drv_init(struct platform_device *pdev, struct drv_hwts_ctrl *hwts, int node_id)
{
    int ret;

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
    ret = hwts_drv_get_irq(pdev, hwts, node_id);
    if (ret != 0) {
        ts_drv_err("failed to get irq.\n");
        goto unmap;
    }
    ret = hwts_drv_register_irq_to_cores(hwts, pdev);
    if (ret != 0) {
        ts_drv_err("failed to register irq.\n");
        goto unmap;
    }
    hwts_init_mailbox(hwts, node_id);

    ts_drv_info("hwts_drv load ok.\n");
    return TS_INNER_SUCCESS;

unmap:
    hwts_drv_iounmap_resource(pdev, hwts);

    return ret;
}

#else
void ut_device_hwts_drv_test(void)
{
    return;
}
#endif
