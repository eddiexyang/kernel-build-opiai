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
#include <linux/of_address.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/securec.h>
#include <linux/securectype.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/uio_driver.h>
#include <linux/msi.h>
#include <linux/mm.h>

#include "hwts_drv_api.h"
#include "hwts_drv_config.h"
#include "ts_drv_init.h"
#include "devdrv_dfm.h"
#include "hwts_task_info.h"
#include "event_sched_inner.h"
#include "sched_cmd.h"
#include "ascend_hal_define.h"
#include "aicpu.h"
#include "ts_api.h"
#include "devdrv_manager_comm.h"
#include "hwts_drv_cloud.h"

#define AICPUINDEX_TO_AICPUID(nid, index) ((index) + (g_drv_ctrl_hwts[nid]->ts_info[0].irq_info.aicpu_id_base))

void __iomem *hwts_get_return_mailbox_addr(u32 mb_ind, u32 ts_ind, int node_id)
{
    u32 offset = 0;
    struct drv_hwts_ctrl *hwts = g_drv_ctrl_hwts[node_id];

    offset = mb_ind * (u32)(RET_MAILBOX_SIZE);
    return (void __iomem *)(hwts->ts_info[0].sram_va_base + (AICPU_MAILBOX_MAX_NUM) * MAILBOX_SIZE + offset);
}

void __iomem *hwts_get_mailbox_addr(u32 mb_ind, u32 ts_ind, int node_id)
{
    unsigned int offset = 0;
    struct drv_hwts_ctrl *hwts = g_drv_ctrl_hwts[node_id];

    offset = mb_ind * (u32)MAILBOX_SIZE;
    return (void __iomem *)(hwts->ts_info[0].sram_va_base + offset);
}

STATIC void hwts_drv_free_irq(struct platform_device *pdev, struct drv_hwts_ctrl *hwts)
{
    u32 i = 0;

    for (i = 0; i < hwts->ts_info[0].irq_info.aicpu_num; i++) {
        (void)irq_set_affinity_hint((u32)hwts->ts_info[0].irq_info.irq_ts_to_aicpu_os_proxy[i], NULL);
        devm_free_irq(&pdev->dev, (u32)hwts->ts_info[0].irq_info.irq_ts_to_aicpu_os_proxy[i],
            &hwts->ts_info[0].int_context[i]);
        tasklet_kill(&hwts->ts_info[0].int_context[i].find_pid_then_wakeup_task);
    }

    platform_msi_domain_free_irqs(&pdev->dev);
}

void hwts_drv_iounmap_resource(struct platform_device *pdev, struct drv_hwts_ctrl *hwts)
{
    /* free iounmap */
    devm_iounmap(&pdev->dev, hwts->ts_info[0].sram_va_base);
    hwts->ts_info[0].sram_va_base = NULL;
    devm_iounmap(&pdev->dev, hwts->ts_info[0].gicd_va_base);
    hwts->ts_info[0].gicd_va_base = NULL;
    devm_iounmap(&pdev->dev, hwts->ts_info[0].ts_gicr_va_base);
    hwts->ts_info[0].ts_gicr_va_base = NULL;
    devm_iounmap(&pdev->dev, hwts->ts_info[0].ts_aicpu_va_base);
    hwts->ts_info[0].ts_aicpu_va_base = NULL;
}

STATIC void hwts_drv_write_lpi_msg(struct msi_desc *desc, struct msi_msg *msg)
{
    return;
}

STATIC int hwts_drv_read_ts_ready_state(struct drv_hwts_ctrl *hwts, u32 *stat)
{
    int ret = 0;
    u32 j = 0;

    do {
        *stat = readl_relaxed(hwts->ts_info[0].ts_aicpu_va_base + TS_READY_STATE_REG_OFFSET);
        msleep(1);
        j++;
        if (j >= TS_READY_WAIT_COUNT) {
            ret = -1;
            break;
        }
    } while ((*stat < TS_READY_MIN_VALUE) || (*stat == 0xffffffffUL));

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
    if (ret == -1) {
        ts_drv_err("wait TS SRAM timeout(12s), stat=%u. ret=%d dev_id:%d\n", stat, ret, hwts->numa_node);
    } else {
        ts_drv_debug("TS SRAM init OK, then we remap sram. dev_id:%d\n", hwts->numa_node);
    }

    return ret;
}

void hwts_handle_ack_success(u32 cpu_index, u32 ts_index)
{
    return;
}

void hwts_handle_ack_fail(u32 mb_index, u32 ts_index)
{
    return;
}

STATIC void hwts_drv_send_interrupt_to_ts(u32 cpu_index, int node_id)
{
    u64 irq;
    void __iomem *reg = NULL;
    struct drv_hwts_ctrl *hwts = g_drv_ctrl_hwts[node_id];
    irq = hwts->ts_info[0].irq_info.irq_aicpu_to_ts[cpu_index];
    reg = (void __iomem *)(((char *)(hwts->ts_info[0].ts_gicr_va_base)) + GICR_SETLPIR);
    hwts_drv_writeq_relaxed(reg, irq);
}

#define MAX_FID_NUM 17
STATIC u32 hwts_get_vfid_by_mbox_info(struct drv_hwts_mailbox *mailbox_info)
{
    if (mailbox_info->kernel_info.kernelType == KERNEL_TYPE_CCE ||
        mailbox_info->kernel_info.kernelType == KERNEL_TYPE_CTL ||
        mailbox_info->kernel_info.kernelType == KERNEL_TYPE_CUSTOMAICPU) {
        return mailbox_info->kernel_info.kernelBase.cce_kernel.vfid;
    } else if (mailbox_info->kernel_info.kernelType == KERNEL_TYPE_TF) {
        return mailbox_info->kernel_info.kernelBase.fwk_kernel.vfid;
    }

    return MAX_FID_NUM;
}

STATIC int hwts_drv_mbox_info_chk(u32 mb_index, u32 ts_index, int node_id)
{
    struct drv_hwts_mailbox *mailbox_info = NULL;
    enum devdrv_process_type cp_type = DEVDRV_PROCESS_CP1;
    u32 host_pid, vfid, chip_id;
    int ret;

    if (mb_index >= AICPU_MAILBOX_MAX_NUM) {
        ts_drv_err("invalid mb_index(%u)\n", mb_index);
        return -EINVAL;
    }

    ret = devdrv_query_process_host_pid(current->tgid, &chip_id, &vfid, &host_pid, &cp_type);
    if (ret != 0) {
        ts_drv_err("process %d get vfid and hostpid failed. ret %d\n", current->tgid, ret);
        return ret;
    }

    mailbox_info = (struct drv_hwts_mailbox *)hwts_get_mailbox_addr(mb_index, ts_index, node_id);
    if ((vfid != hwts_get_vfid_by_mbox_info(mailbox_info)) || (host_pid != (u32)mailbox_info->pid)) {
        ts_drv_err("vfid(%u %u) or host_pid(%u %d) mismatch\n",
            vfid, hwts_get_vfid_by_mbox_info(mailbox_info), host_pid, mailbox_info->pid);
        return -ENODEV;
    }

    /* clear mailbox to invalid, or ts will not send next */
    mailbox_info->valid = MAILBOX_INVALID;
    ts_drv_debug("sched_ack, node_id=%d, mb_id=%u, ts_id=%u, dpid=%d, hpid=%u, vfid=%u, cp_type=%d.\n",
        node_id, mb_index, ts_index, current->tgid, host_pid, vfid, (int)cp_type);

    return 0;
}

int hwts_drv_sched_ack(unsigned int devid, unsigned int subevent_id, const char *msg, unsigned int msg_len, void *priv)
{
    struct drv_hwts_task_response *res_mailbox = NULL;
    struct hwts_response *resp = NULL;
    u32 mb_index, ts_index, state;
    int node_id = numa_node_id();

    if (msg == NULL) {
        ts_drv_err("ack msg is null.\n");
        return -1;
    }
    resp = (struct hwts_response *)msg;
    ts_index = subevent_id;
    mb_index = resp->mailbox_id;
    state = resp->result;

    if (hwts_drv_mbox_info_chk(mb_index, ts_index, node_id)) {
        ts_drv_err("mbox_info_chk fail, chip_id=%d, aicpu_index=%u.\n", node_id, mb_index);
        return TS_PARA_ERR;
    }

    res_mailbox = (struct drv_hwts_task_response *)hwts_get_return_mailbox_addr(mb_index, ts_index, node_id);
    if (res_mailbox == NULL) {
        ts_drv_err("return mailbox is null, chip_id=%d, aicpu_index=%u.\n", node_id, mb_index);
        return -1;
    }
     /* before write valid flag to rsp mailbox ,judge rsp-mailbox ready */
    if (*(volatile unsigned int *)res_mailbox == MAILBOX_CONFIRM) {
        res_mailbox->valid = MAILBOX_VALID;
    } else {
        ts_drv_warn("this mailbox vaild check failed,chip_id=%d, aicpu_index=%u\n", node_id, mb_index);
    }
    res_mailbox->state = state;
    res_mailbox->serial_no = resp->serial_no;
    wmb();
    hwts_drv_send_interrupt_to_ts(mb_index, node_id);

    return 0;
}

int hwts_drv_thread_receive_confirm(volatile unsigned long long serial_no,
    unsigned int mailbox_index, unsigned int ts_index, int node_id)
{
    struct drv_hwts_task_response *msg = NULL;
    msg = (struct drv_hwts_task_response *)hwts_get_return_mailbox_addr(mailbox_index, ts_index, node_id);
    msg->valid = MAILBOX_CONFIRM;
    msg->state = 0;
    msg->serial_no = serial_no;

    return 0;
}

void hwts_drv_submit_task(unsigned long data)
{
    struct sched_published_event publish_event = {0};
    struct drv_hwts_int_context *int_context = NULL;
    struct drv_hwts_mailbox *mailbox_info = NULL;
    void __iomem *mailbox_addr = NULL;
    struct hwts_ts_task ts_task = {0};
    enum devdrv_process_type cp_type;
    u32 node_id;
    int confirmRet;
    u32 hpid;
    int pid;
    int ret;
    u32 idx;
    u32 ts_idx;
    u32 vfid = 0;

    int_context = (struct drv_hwts_int_context *)(uintptr_t)data;
    if (int_context->mailbox_index >= AICPU_MAILBOX_MAX_NUM) {
        ts_drv_err("mailbox_index(%u) >= AICPU_MAILBOX_MAX_NUM(%d)\n",
                   int_context->mailbox_index, AICPU_MAILBOX_MAX_NUM);
        return;
    }
    mailbox_addr = hwts_get_mailbox_addr(int_context->mailbox_index, 0, int_context->node_id);
    mailbox_info = (struct drv_hwts_mailbox *)mailbox_addr;
    idx = int_context->mailbox_index;
    ts_idx = int_context->ts_index;
    hpid = (u32)mailbox_info->pid;

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
        vfid = mailbox_info->kernel_info.kernelBase.cce_kernel.vfid;
    } else if (mailbox_info->kernel_info.kernelType == KERNEL_TYPE_TF) {
        ts_task.kernel_info.paramBase = mailbox_info->kernel_info.kernelBase.fwk_kernel.kernel;
        ts_task.kernel_info.streamID = mailbox_info->kernel_info.kernelBase.fwk_kernel.streamID;
        ts_task.kernel_info.taskID = mailbox_info->kernel_info.kernelBase.fwk_kernel.taskID;
        vfid = mailbox_info->kernel_info.kernelBase.fwk_kernel.vfid;
    }

    if (mailbox_info->kernel_info.kernelType == KERNEL_TYPE_CUSTOMAICPU) {
        cp_type = DEVDRV_PROCESS_CP2;
    } else {
        cp_type = DEVDRV_PROCESS_CP1;
    }
    node_id = (u32)cpu_to_node(smp_processor_id()); /*lint !e666  !e453*/
    ret = devdrv_query_process_by_host_pid(hpid, node_id, cp_type, vfid, &pid);
    if (unlikely(ret)) {
        ts_drv_err("query pid by host pid %u error ret:%d\n", hpid, ret);
        return;
    }
    ts_drv_debug("submit_task, node_id=%u, mb_id=%u, ts_id=%u, dpid=%d, hpid=%u, vfid=%u, cp_type=%d.\n",
        node_id, idx, ts_idx, pid, hpid, vfid, (int)cp_type);

    publish_event.event_info.pid = pid;
    publish_event.event_info.gid = 0;
    publish_event.event_info.event_id = EVENT_TS_HWTS_KERNEL;
    publish_event.event_info.subevent_id = 0;
    publish_event.event_info.msg = (char *)(&ts_task);
    publish_event.event_info.msg_len = (u32)sizeof(struct hwts_ts_task);
    publish_event.event_func.event_ack_func = hwts_drv_sched_ack;
    publish_event.event_func.event_finish_func = NULL;

    confirmRet = hwts_drv_thread_receive_confirm(mailbox_info->serial_no, idx, ts_idx, node_id);
    if (confirmRet) {
        ts_drv_warn("HostPid[%u], node_id[%u], pid[%d], aicpu[%u], send confirm to ts[%u] failed, error: %d.\n",
                    hpid, node_id, pid, idx, ts_idx, confirmRet);
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
        mailaddr = hwts_get_return_mailbox_addr(i, 0, node_id);
        hwts_drv_writel_relaxed(mailaddr, 0);
    }
}

STATIC void hwts_drv_flush_cache(u64 base, u32 len)
{
    u64 addr_loop, addr_end;

    addr_loop = base & (~AICPU_CACHELINE_MASK);
    addr_end = (base + len) & (~AICPU_CACHELINE_MASK);

    wmb();
    for (; addr_loop < addr_end;) {
        HWTS_DRV_FLUSH_CACHE(addr_loop);
        mb();
        addr_loop += AICPU_CACHELINE_SIZE;
    }
    wmb();
}

STATIC void hwts_drv_gicr_lpi_tbl_init_cfg(struct drv_hwts_ctrl *hwts)
{
    void __iomem *gicd_lpi_tbl_base = NULL;
    void __iomem *rd_base = NULL;
    u32 hwirq_aicpu_to_ts;
    u64 val_of_rd_base;
    u8 *cfg = NULL;
    u32 i = 0;

    rd_base = ioremap(SOC_GIC_LPI_TABLE_BASE, TS_GICR_SIZE);
    if (rd_base == NULL) {
        ts_drv_err("gicd_lpi_tbl_base failed. dev_id:%d\n", hwts->numa_node);
        return;
    }

    /* get ddr base address */
    val_of_rd_base = readq_relaxed(rd_base + TS_GICR_PROPBASER_OFFSET);
    val_of_rd_base = (val_of_rd_base & (0x000ffffffffff000));
    ts_drv_info("val of rd base. dev_id:%d\n", hwts->numa_node);

    if (unlikely(!pfn_valid(__phys_to_pfn(val_of_rd_base)))) {
        iounmap(rd_base);
        rd_base = NULL;
        ts_drv_err("os memory map LPIs config base failed. dev_id:%d\n", hwts->numa_node);
        return;
    }

    /* os has mmaped for us ,just use it */
    gicd_lpi_tbl_base = phys_to_virt(val_of_rd_base);
    if (gicd_lpi_tbl_base == NULL) {
        iounmap(rd_base);
        rd_base = NULL;
        ts_drv_err("gicd_lpi_tbl_base failed. dev_id:%d\n", hwts->numa_node);
        return;
    }
    for (i = 0; i < hwts->ts_info[0].irq_info.aicpu_num; i++) {
        hwirq_aicpu_to_ts = hwts->ts_info[0].irq_info.irq_aicpu_to_ts[i];
        cfg = (u8 *)((uintptr_t)gicd_lpi_tbl_base) + hwirq_aicpu_to_ts - OS_HWI_LPI_MIN;
        /* enable interrupt */
        *cfg |= LPI_PROP_ENABLED;
        dsb(sy);
        /* invalid LPIR */
        hwts_drv_writeq_relaxed(rd_base + TS_GICR_INVLPIR_OFFSET, (u64)hwirq_aicpu_to_ts);
        udelay(1);
    }
    /* hwirq decreasing in the irq_aicpu_to_ts array ,LPIs size used less than 64bytes */
    hwts_drv_flush_cache((u64)(uintptr_t)cfg, AICPU_CACHELINE_SIZE);
    iounmap(rd_base);
    rd_base = NULL;
}

int hwts_drv_register_irq_to_cores(struct drv_hwts_ctrl *hwts,  struct platform_device *pdev)
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
    for (i = 0; i < aicpu_num; i++) {
        hwts->ts_info[0].int_context[i].mailbox_index = i;
        hwts->ts_info[0].int_context[i].node_id = node_id;
        /* tasklet init */
        tasklet_init(&hwts->ts_info[0].int_context[i].find_pid_then_wakeup_task, hwts_drv_submit_task,
                     (unsigned long)(uintptr_t)&hwts->ts_info[0].int_context[i]);
        ret = devm_request_irq(&pdev->dev, (u32)hwts->ts_info[0].irq_info.irq_ts_to_aicpu_os_proxy[i],
                               hwts_drv_irq_callback, IRQF_TRIGGER_RISING, "hwts_drv_interrupt",
                               &hwts->ts_info[0].int_context[i]);
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
            cpu_id = (hwts->ts_info[0].irq_info.aicpu_id_base + hwts->ts_info[0].irq_info.aicpu_num) *
                     (u32)node_id + AICPUINDEX_TO_AICPUID(node_id, i);
        }
        cpumask_set_cpu(cpu_id, &(hwts->ts_info[0].int_context[i].cpumask));
        (void)irq_set_affinity_hint((u32)hwts->ts_info[0].irq_info.irq_ts_to_aicpu_os_proxy[i],
            &(hwts->ts_info[0].int_context[i].cpumask));
    }
    /* enable LPI table for ts core-hw irqs */
    hwts_drv_gicr_lpi_tbl_init_cfg(hwts);
    return 0;

request_irq_fail:
    j = i;
    for (i = 0; i < j; i++) {
        (void)irq_set_affinity_hint((u32)hwts->ts_info[0].irq_info.irq_ts_to_aicpu_os_proxy[i], NULL);
        devm_free_irq(&pdev->dev, (u32)hwts->ts_info[0].irq_info.irq_ts_to_aicpu_os_proxy[i],
            &hwts->ts_info[0].int_context[i]);
    }
    for (i = 0; i <= j; i++) {
        tasklet_kill(&hwts->ts_info[0].int_context[i].find_pid_then_wakeup_task);
    }
    return ret;
}

int hwts_drv_get_irq(struct platform_device *pdev, struct drv_hwts_ctrl *hwts, int node_id)
{
    int ret = 0;
    u32 i;
    u32 irq_os_proxy = 0;
    u32 aicpu_id;
    u32 hw_irq;
    struct msi_desc *desc = NULL;

    for (i = 0; i < hwts->ts_info[0].irq_info.aicpu_num; i++) {
        aicpu_id = (AICPUINDEX_TO_AICPUID(node_id, i));
        hw_irq = hwts->ts_info[0].irq_info.irq_ts_to_aicpu[i];
        for_each_msi_entry(desc, &pdev->dev) //lint !e666
        {
            if (desc->msi_index == aicpu_id) {
                irq_os_proxy = desc->irq;
                break;
            }
        }
        if (irq_os_proxy == 0) {
            ts_drv_err("get os-proxy-irq failed.\n");
            ret = -EINVAL;
            return ret;
        }
        hwts->ts_info[0].irq_info.irq_ts_to_aicpu_os_proxy[i] = irq_os_proxy;
        ts_drv_debug("*** read cpu_id(%u), os proxy-irq(%u), hw irq(%u) ***\n", aicpu_id, irq_os_proxy, hw_irq);
    }

    return ret;
}

int hwts_drv_mailbox_addr_config(struct drv_hwts_ctrl *hwts)
{
    return 0;
}

int hwts_drv_va_base_ioremap(struct platform_device *pdev, struct drv_hwts_ctrl *hwts)
{
    int ret = 0;

    /* ioremap ts_aicpu_va_base map to device no cache property */
    hwts->ts_info[0].ts_aicpu_va_base = devm_ioremap(&pdev->dev,
        hwts->ts_info[0].ts_status_info.ts_aicpu_status_base,
        hwts->ts_info[0].ts_status_info.ts_aicpu_status_size);
    if (hwts->ts_info[0].ts_aicpu_va_base == NULL) {
        ts_drv_err("ioremap failed.\n");
        goto ts_aicpu_va_base_failed;
    }
    hwts->ts_info[0].gicd_va_base = devm_ioremap(&pdev->dev, hwts->ts_info[0].gicd_info.gicd_pa_base,
        hwts->ts_info[0].gicd_info.gicd_pa_size);
    if (hwts->ts_info[0].gicd_va_base == NULL) {
        ts_drv_err("ioremap failed.\n");
        goto gicd_va_base_failed;
    }
    hwts->ts_info[0].ts_gicr_va_base =
        devm_ioremap(&pdev->dev, hwts->ts_info[0].gicd_info.gicd_pa_base + TS_GICR_OFFSET, TS_GICR_SIZE);
    if (hwts->ts_info[0].ts_gicr_va_base == NULL) {
        ts_drv_err("ioremap failed.\n");
        goto ts_gicr_va_base_failed;
    }

    /* wait ts ok! */
    ret = hwts_drv_wait_ts_ready(hwts);
    if (ret != 0) {
        goto sram_va_base_failed;
    }
    /* ioremap sram_va_base map to device no cache property */
    hwts->ts_info[0].sram_va_base = devm_ioremap(&pdev->dev, hwts->ts_info[0].sram_info.sram_pa_base,
        hwts->ts_info[0].sram_info.sram_pa_size);
    if (hwts->ts_info[0].sram_va_base == NULL) {
        ts_drv_err("ioremap failed.\n");
        goto sram_va_base_failed;
    }
    return 0;

sram_va_base_failed:
ts_gicr_va_base_failed:
    devm_iounmap(&pdev->dev, hwts->ts_info[0].ts_gicr_va_base);
    hwts->ts_info[0].ts_gicr_va_base = NULL;
gicd_va_base_failed:
    devm_iounmap(&pdev->dev, hwts->ts_info[0].gicd_va_base);
    hwts->ts_info[0].gicd_va_base = NULL;
ts_aicpu_va_base_failed:
    devm_iounmap(&pdev->dev, hwts->ts_info[0].ts_aicpu_va_base);
    hwts->ts_info[0].ts_aicpu_va_base = NULL;

    return TS_INNER_ERR;
}

STATIC void hwts_drv_get_chip_info(u32 *chip_id, u32 *chip_type)
{
    u32 __iomem *ptr = NULL;
    u32 val;
    u32 offset;
    ptr = (u32 *)ioremap((SC_CHIP_INFO & PAGE_MASK), PAGE_SIZE);
    if (ptr == NULL) {
        ts_drv_err("ioremap err\n");
        return;
    }
    offset = (u32)((SC_CHIP_INFO & (~PAGE_MASK)) / sizeof(u32));
    val = readl(ptr + offset);
    if (chip_id != NULL) {
        *chip_id = ((val >> AICPU_PLATFORM_CHIP_ID_OFFSET) & 0xffff);
    }

    offset += (u32)((SC_VER_NUM - SC_CHIP_INFO) / sizeof(u32));
    val = readl(ptr + offset);
    if (chip_type != NULL) {
        if (val == 0) {
            *chip_type = AICPU_PLATFORM_ASIC;
        } else {
            *chip_type = (val >> AICPU_PLATFORM_CHIP_TYPE_OFFSET) & 0xf;
        }
    }

    iounmap(ptr);
    ptr = NULL;
}

STATIC void hwts_drv_init_common_config(struct drv_hwts_ts_info *ts_info,
    struct aicpu_system_config *config_system, int node_id)
{
    hwts_drv_get_chip_info(&(ts_info->chip_info.chip_id), &(ts_info->chip_info.chip_type));
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

    ts_drv_debug("dev_id:%d, aicpu_num:%u, aicpu_id_base:%u, start_id:%u\n",
        node_id,
        ts_info->irq_info.aicpu_num,
        ts_info->irq_info.aicpu_id_base,
        ts_info->irq_info.ts_int_start_id);
}

int hwts_drv_get_config_info(struct drv_hwts_ctrl *hwts, struct platform_device *pdev)
{
    struct aicpu_system_config *config_system  = NULL;
    void __iomem *vaddr = NULL;
    u32 aicpu_start_int = 0;
    int node_id;
    u32 i = 0;

    node_id = hwts->numa_node;
    vaddr = ioremap_cache(SYSTEM_MULTI_CONFIG_BASE((u32)node_id), SYSTEM_CONFIG_SIZE);
    if (vaddr == NULL) {
        ts_drv_err("system_config_base ioremap failed. dev_id:%d\n", node_id);
        return TS_INNER_ERR;
    }

    config_system = (struct aicpu_system_config *)vaddr;
    if (config_system->flag != SYSTEM_CONFIG_FLAG) {
        ts_drv_err("aicpu_config-system ts: is invalid. dev_id:%d\n", node_id);
        iounmap(vaddr);
        return TS_INNER_ERR;
    }

    hwts_drv_init_common_config(&hwts->ts_info[0], config_system, node_id);
    if (hwts->ts_info[0].irq_info.aicpu_num > AICPU_MAX_NUM) {
        ts_drv_err("aicpu_num:%u is invalid. dev_id:%d\n", hwts->ts_info[0].irq_info.aicpu_num, node_id);
        iounmap(vaddr);
        return TS_INNER_ERR;
    }

    /* calculate irqs */
    for (i = 0; i < hwts->ts_info[0].irq_info.aicpu_num; i++) {
        aicpu_start_int = hwts->ts_info[0].irq_info.ts_int_start_id + hwts->ts_info[0].gicd_info.gic_multichip_off;
        hwts->ts_info[0].irq_info.irq_aicpu_to_ts[i] = aicpu_start_int - AICPUINDEX_TO_AICPUID(node_id, i);
        hwts->ts_info[0].irq_info.irq_ts_to_aicpu[i]
            = hwts->ts_info[0].irq_info.irq_aicpu_to_ts[i] - IRQ_AICPU_TO_TS_OFFSET;
        ts_drv_debug("_to_ts:%u.\n", hwts->ts_info[0].irq_info.irq_aicpu_to_ts[i]);
        ts_drv_debug("_to_aicpu:%u.\n", hwts->ts_info[0].irq_info.irq_ts_to_aicpu[i]);
    }
    ts_drv_info("hwts_drv_init finish calculate irqs!node_id = %d, aicpu_start_int = %u\n", node_id, aicpu_start_int);

    iounmap(vaddr);
    vaddr = NULL;

    return 0;
}

int hwts_drv_resume(struct drv_hwts_ctrl *hwts)
{
    ts_drv_info("hwts resume isn't implement.dev_id:%d\n", hwts->numa_node);
    return 0;
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
    ret = platform_msi_domain_alloc_irqs(&pdev->dev, AICPU_MAX_NUM, hwts_drv_write_lpi_msg);
    if (ret != 0) {
        ts_drv_err("failed to allocate LPIs.\n");
        goto alloc_lpi_fail;
    }
    ret = hwts_drv_get_irq(pdev, hwts, node_id);
    if (ret != 0) {
        ts_drv_err("failed to get irq.\n");
        goto get_irq_os_proxy_failed;
    }
    ret = hwts_drv_register_irq_to_cores(hwts, pdev);
    if (ret != 0) {
        ts_drv_err("failed to register irq.\n");
        goto register_irq_fail;
    }
    hwts_init_mailbox(hwts, node_id);
    ts_drv_info("hwts drv load ok.\n");

    return 0;
register_irq_fail:
get_irq_os_proxy_failed:
    platform_msi_domain_free_irqs(&pdev->dev);
alloc_lpi_fail:
    hwts_drv_iounmap_resource(pdev, hwts);

    return TS_INNER_ERR;
}
#else /* TSDRV_UT */
void ut_device_hwts_drv_test(void)
{
}
#endif /* TSDRV_UT */

