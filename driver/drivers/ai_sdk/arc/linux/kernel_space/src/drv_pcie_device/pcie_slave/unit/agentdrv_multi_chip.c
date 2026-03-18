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
#include <linux/fs.h>
#include <linux/property.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/of.h>
#include <linux/acpi.h>

#ifdef CFG_FEATURE_SUPPORT_CPUDOMAIN
#include <linux/aos/cpu_domain_info.h>
#endif

#include "devdrv_util.h"
#include "agentdrv_msg.h"
#include "agentdrv_platform.h"
#include "agentdrv_pci.h"
#include "agentdrv_unit.h"
#include "resource_drv.h"
#include "agentdrv_multi_chip.h"

int agentdrv_parse_cpu_num(int dev_id, char buf[], u32 *cpu_num)
{
    int i, start_num, end_num, len;
    char *start = NULL;
    char *end = NULL;

    start = buf;
    for (i = 0; i < dev_id; i++) {
        start = strchr(start, ',');
        if (start == NULL) {
            devdrv_err("String parse failed. (dev_id=%d; buf=\"%s\")\n", dev_id, buf);
            *cpu_num = 0;
            return 0; /* parse fail by format not support, buf return 0 to let the system start complete */
        }
        start++;
    }

    end = strchr(start, ',');
    if (end == NULL) {
        len = strlen(start);
    } else {
        len = strlen(start) - strlen(end);
        start[len] = '\0';
    }

    end = strchr(start, '-');
    if (end != NULL) {
        len = strlen(start);
        end[0] = '\0';
        end++;
        start_num = (int)simple_strtol(start, NULL, 0);
        end_num = (int)simple_strtol(end, NULL, 0);
        *cpu_num = end_num - start_num + 1;
    } else {
        *cpu_num = 1;
    }

    return 0;
}

#define CPU_INFO_SIZE 64
int agentdrv_get_cpu_num(int dev_id, const char *cpu_type, u32 *cpu_num)
{
    struct file *file = NULL;
    char buf[CPU_INFO_SIZE];
    loff_t pos = 0;
    int len;

    file = filp_open(cpu_type, O_RDONLY, 0);
    if (IS_ERR(file)) {
        devdrv_info("Open file abnormal. (dev_id=%d; cpu_type=\"%s\")\n", dev_id, cpu_type);
        return -EINVAL;
    }

    len = kernel_read(file, buf, CPU_INFO_SIZE - 1, &pos);
    (void)filp_close(file, NULL);
    file = NULL;

    if (len == 1) {
        /* file don't have content */
        devdrv_info("File not processed. (dev_id=%d; cpu_type=\"%s\"; read_len=%d)\n", dev_id, cpu_type, len);
        *cpu_num = 0;
        return 0;
    } else if (len <= 0) {
        devdrv_err("File read failed. (dev_id=%d; cpu_type=\"%s\"; read_len=%d)\n", dev_id, cpu_type, len);
        return -EINVAL;
    }
    buf[len - 1] = '\0';

    devdrv_info("File read success. (dev_id=%d; cpu_group=\"%s\"; read_len=%d; buf=%s)\n", dev_id, cpu_type, len, buf);

    return agentdrv_parse_cpu_num(dev_id, buf, cpu_num);
}

void agentdrv_get_pdev_type_info(struct platform_device *pdev, u32 *type, u32 *irq_func_idx)
{
#ifndef DRV_UT
    struct device_node *node = NULL;
    int ret;

    if (!acpi_disabled) {
        ret = device_property_read_u32(&pdev->dev, "type", type);
        if (ret != 0) {
            *type = 0;
            devdrv_debug("can not get type, not support\n");
        }

        ret = device_property_read_u32(&pdev->dev, "irq_idx", irq_func_idx);
        if (ret != 0) {
            *irq_func_idx = 0;
            devdrv_debug("can not get irq_func_idx, not support\n");
        }
    } else {
        node = pdev->dev.of_node;
        if (node == NULL) {
            devdrv_debug("node is null\n");
            return;
        }

        ret = of_property_read_u32(node, "type", type);
        if (ret != 0) {
            *type = 0;
            devdrv_debug("can not get type, not support\n");
        }

        ret = of_property_read_u32(node, "irq_idx", irq_func_idx);
        if (ret != 0) {
            *irq_func_idx = 0;
            devdrv_debug("can not get irq_func_idx, not support\n");
        }
    }
#else
    *type = 0;
    *irq_func_idx = 0;
#endif
    return;
}

int agentdrv_get_ts_cpu_info(struct agentdrv_devctrl *agent_dev)
{
    int chip_id = agent_dev->agent_id;
    int dev_id;
    int ret;
    struct platform_device *pdev = agent_dev->apb_pdev;
    u32 val[AGENTDRV_PARA_NUM] = {0};
    int func_index;
    u32 func_totl = agentdrv_res_get_func_total();

    for (func_index = 0; func_index < (int)func_totl; func_index++) {
        devdrv_chipfunc2dev(&dev_id, chip_id, func_index);
        agent_dev->cpu_info[func_index].tscpu_num = 0;
        agent_dev->cpu_info[func_index].tscpu_os_sched = 0;

        if (pdev != NULL) {
            ret = device_property_read_u32_array(&pdev->dev, "tscpu_num", val, AGENTDRV_PARA_NUM);
            if (ret) {
                devdrv_warn("TS cpu info no res. (dev_id=%d; chip_id=%d; func_id=%d; ret=%d)\n",
                    dev_id, chip_id, func_index, ret);
                agent_dev->cpu_info_valid = DEVDRV_VALID;
                return ret;
            }
            agent_dev->cpu_info[func_index].tscpu_num = val[0];
            agent_dev->cpu_info[func_index].tscpu_os_sched = val[1];
            agent_dev->cpu_info_valid = DEVDRV_VALID;
        }
#if defined(CFG_SOC_PLATFORM_MDC_LITE_ESL) || defined(CFG_SOC_PLATFORM_MDC_LITE_DECOUPLING)
        agent_dev->cpu_info_valid = DEVDRV_VALID;
#endif
        devdrv_info("Get ts cpu info. (dev_id=%d; chip_id=%d; func_id=%d; tscpu_num=%d; os_sched=%d)\n",
            dev_id, chip_id, func_index, val[0], val[1]);
    }

    return 0;
}

STATIC void agentdrv_sriov_init_cpu_info(struct agentdrv_devctrl *agent_dev, u32 func_id)
{
    agent_dev->cpu_info[func_id].ccpu_num = agent_dev->cpu_info[0].ccpu_num;
    agent_dev->cpu_info[func_id].ccpu_os_sched = agent_dev->cpu_info[0].ccpu_os_sched;
    agent_dev->cpu_info[func_id].dcpu_num = agent_dev->cpu_info[0].dcpu_num;
    agent_dev->cpu_info[func_id].dcpu_os_sched = agent_dev->cpu_info[0].dcpu_os_sched;
    agent_dev->cpu_info[func_id].aicpu_num = agent_dev->cpu_info[0].aicpu_num;
    agent_dev->cpu_info[func_id].aicpu_os_sched = agent_dev->cpu_info[0].aicpu_os_sched;
    agent_dev->cpu_info[func_id].tscpu_num = agent_dev->cpu_info[0].tscpu_num;
    agent_dev->cpu_info[func_id].tscpu_os_sched = agent_dev->cpu_info[0].tscpu_os_sched;

    devdrv_info("Sriov init CPU info. (dev_id=%u; func_id=%u; ccpu_num=%u; dcpu_num=%u; aicpu_num=%u; tscpu_num=%u)\n",
        agent_dev->agent_id, func_id, agent_dev->cpu_info[func_id].ccpu_num, agent_dev->cpu_info[func_id].dcpu_num,
        agent_dev->cpu_info[func_id].aicpu_num, agent_dev->cpu_info[func_id].tscpu_num);
}

#ifdef CFG_FEATURE_SUPPORT_CPUDOMAIN
int agentdrv_get_cpudomain_info(int dev_id, int chip_id, int func_index,
    struct agentdrv_cpu_info *cpu_info)
{
    struct cpu_domain_info domain_info;
    int err;

    err = get_cpudomain_info(&domain_info);
    if (err != 0) {
        devdrv_err("get_cpudomain_info failed,err = %d.\n", err);
        return -EINVAL;
    }

    cpu_info->ccpu_num = domain_info.ctrlcpu_num;
    cpu_info->ccpu_os_sched = AGENTDRV_ENABLE;
    devdrv_info("Get cpu_num. (dev_id=%d; chip_id=%d; func_id=%d; ctrlcpu_num=%u)\n",
        dev_id, chip_id, func_index, cpu_info->ccpu_num);

    cpu_info->dcpu_num = domain_info.datacpu_num;
    cpu_info->dcpu_os_sched = AGENTDRV_ENABLE;
    devdrv_info("Get cpu_num. (dev_id=%d; chip_id=%d; func_id=%d; datacpu_num=%u)\n",
        dev_id, chip_id, func_index, cpu_info->dcpu_num);

    cpu_info->aicpu_num = domain_info.aicpu_num;
    cpu_info->aicpu_os_sched = AGENTDRV_ENABLE;
    devdrv_info("Get aicpu_num. (dev_id=%d; chip_id=%d; func_id=%d; aicpu_num=%u)\n",
        dev_id, chip_id, func_index, cpu_info->aicpu_num);
    return 0;
}
#else
int agentdrv_get_cpudomain_info(int dev_id, int chip_id, int func_index,
    struct agentdrv_cpu_info *cpu_info)
{
    u32 cpu_num = 0;
    int ret;

    ret = agentdrv_get_cpu_num(dev_id, "/sys/fs/cgroup/cpuset/CtrlCPU/cpuset.cpus", &cpu_num);
    if (ret) {
        return ret;
    }
    cpu_info->ccpu_num = cpu_num;
    cpu_info->ccpu_os_sched = AGENTDRV_ENABLE;
    devdrv_info("Get cpu_num. (dev_id=%d; chip_id=%d; func_id=%d; ctrlcpu_num=%u)\n",
        dev_id, chip_id, func_index, cpu_info->ccpu_num);

    ret = agentdrv_get_cpu_num(dev_id, "/sys/fs/cgroup/cpuset/DataCPU/cpuset.cpus", &cpu_num);
    if (ret) {
        devdrv_err("Data cpu information got failed. (dev_id=%d; chip_id=%d; func_id=%d; ret=%d)\n",
            dev_id, chip_id, func_index, ret);
        return ret;
    }
    cpu_info->dcpu_num = cpu_num;
    cpu_info->dcpu_os_sched = AGENTDRV_ENABLE;
    devdrv_info("Get cpu_num. (dev_id=%d; chip_id=%d; func_id=%d; datacpu_num=%u)\n",
        dev_id, chip_id, func_index, cpu_info->dcpu_num);

    ret = agentdrv_get_cpu_num(dev_id, "/sys/fs/cgroup/cpuset/AICPU/cpuset.cpus", &cpu_num);
    if (ret) {
        devdrv_err("Data cpu information got failed. (dev_id=%d; chip_id=%d; func_id=%d; ret=%d)\n",
            dev_id, chip_id, func_index, ret);
        return ret;
    }
    cpu_info->aicpu_num = cpu_num;
    cpu_info->aicpu_os_sched = AGENTDRV_ENABLE;
    devdrv_info("Get aicpu_num. (dev_id=%d; chip_id=%d; func_id=%d; aicpu_num=%u)\n",
        dev_id, chip_id, func_index, cpu_info->aicpu_num);

    return 0;
}
#endif
EXPORT_SYMBOL(agentdrv_get_cpudomain_info);

int agentdrv_init_cpu_info(struct agentdrv_devctrl *agent_dev)
{
    int ret;
    int chip_id = (int)agent_dev->agent_id;
    int dev_id;
    int func_index;
    u32 func_totl = agentdrv_res_get_func_total();

    for (func_index = 0; func_index < (int)func_totl; func_index++) {
        if ((devdrv_is_sriov_enabled(func_totl) == true) && (func_index > 0)) {
            agentdrv_sriov_init_cpu_info(agent_dev, (u32)func_index);
            continue;
        }
        devdrv_chipfunc2dev(&dev_id, chip_id, func_index);
        devdrv_info("Get cpu information from the cgroup config. (dev_id=%d; chip_id=%d; func_id=%d)\n",
            dev_id, chip_id, func_index);

        ret = agentdrv_get_cpudomain_info(dev_id, chip_id, func_index, &(agent_dev->cpu_info[func_index]));
        if (ret != 0) {
            devdrv_err("cpu information got failed. (dev_id=%d; chip_id=%d; func_id=%d; ret=%d)\n",
                dev_id, chip_id, func_index, ret);
            return ret;
        }
    }

    ret = agentdrv_get_ts_cpu_info(agent_dev);
    if (ret != 0) {
        devdrv_err("TS cpu information got failed. (dev_id=%d)\n", chip_id);
        return ret;
    }

    return 0;
}

void agentdrv_dma_bind_irq(const struct agentdrv_devctrl *agent_dev, u32 func_id,
    struct agentdrv_cpu_data *drv_cpu_info)
{
    u32 dma_low_num = DEVDRV_DMA_DATA_COMM_CHAN_NUM + DEVDRV_DMA_DATA_PCIE_MSG_CHAN_NUM;
    struct devdrv_dma_dev *dma_dev = NULL;
    u32 chip_id = agent_dev->agent_id;
    u32 cpuid, irq, chan_id;
    u32 i, local_end;

    if (drv_cpu_info->ccpu_num == 0) {
        devdrv_warn("Input ccpu_num is zero. (chip_id=%u; cpu_num=%u)\n", chip_id, drv_cpu_info->ccpu_num);
        return;
    }
    if (drv_cpu_info->dcpu_num == 0) {
        devdrv_warn("Input dcpu_num is zero. (chip_id=%u; dcpu_num=%u)\n", chip_id, drv_cpu_info->dcpu_num);
        drv_cpu_info->dcpu_num = drv_cpu_info->ccpu_num;
        drv_cpu_info->start_dcpuid = drv_cpu_info->start_ccpuid;
    }

    dma_dev = agent_dev->p_agentdrv_dma_dev[func_id];
    if (dma_dev == NULL) {
        devdrv_warn("dma_dev is NULL. (chip_id=%u; func_id=%u)\n", chip_id, func_id);
        return;
    }

    /* dma done low level to ctrl cpu */
    local_end = (dma_dev->local_chan_num >= dma_low_num) ? dma_low_num : dma_dev->local_chan_num;
    for (i = 0; i < local_end; i++) {
        chan_id = dma_dev->local_chan[i];
        irq = agent_dev->pdev->irq + DMA_DONE_IRQ_BASE + chan_id;
        cpuid = drv_cpu_info->start_ccpuid + chan_id % drv_cpu_info->ccpu_num;
        devdrv_info("DMA done chan bind to cpu. (chip_id=%u; func_id=%u; chan=%u; cpu_id=%u)\n",
            chip_id, func_id, chan_id, cpuid);
        (void)irq_set_affinity_hint(irq, get_cpu_mask(cpuid));
    }

    /* dma done high level to traffic cpu */
    local_end = dma_dev->local_chan_num;
    for (; i < local_end; i++) {
        chan_id = dma_dev->local_chan[i];
        irq = agent_dev->pdev->irq + DMA_DONE_IRQ_BASE + chan_id;
        cpuid = drv_cpu_info->start_dcpuid + chan_id % drv_cpu_info->dcpu_num;
        devdrv_info("DMA done chan bind to cpu. (chip_id=%u; func_id=%u; chan=%u; cpu_id=%u)\n",
            chip_id, func_id, chan_id, cpuid);
        (void)irq_set_affinity_hint(irq, get_cpu_mask(cpuid));
    }

    /* dma error to ctrl cpu */
    if (func_id == 0) {
        irq = agent_dev->pdev->irq + DMA_ERR_IRQ_BASE;
        cpuid = drv_cpu_info->start_ccpuid + DMA_ERR_IRQ_BASE % drv_cpu_info->ccpu_num;
        (void)irq_set_affinity_hint(irq, get_cpu_mask(cpuid));
    }

    return;
}

void agentdrv_dma_unbind_irq(const struct agentdrv_devctrl *agent_dev, u32 func_id)
{
    struct devdrv_dma_dev *dma_dev = NULL;
    u32 i, local_end, chan_id;
    u32 irq;

    if (agent_dev->cpu_info_valid != DEVDRV_VALID) {
        return;
    }

    dma_dev = agent_dev->p_agentdrv_dma_dev[func_id];
    if (dma_dev == NULL) {
        return;
    }

    /* dma done unbind to ctrl cpu */
    local_end = dma_dev->local_chan_num;
    for (i = 0; i < local_end; i++) {
        chan_id = dma_dev->local_chan[i];
        irq = agent_dev->pdev->irq + DMA_DONE_IRQ_BASE + chan_id;
        (void)irq_set_affinity_hint(irq, NULL);
    }

    /* dma error unbind to ctrl cpu */
    if (func_id == 0) {
        irq = agent_dev->pdev->irq + DMA_ERR_IRQ_BASE;
        (void)irq_set_affinity_hint(irq, NULL);
    }

    return;
}

void agentdrv_pci_bind_irq(const struct agentdrv_devctrl *agent_dev,
    u32 func_id, struct agentdrv_cpu_data *drv_cpu_info)
{
    u32 chip_id = agent_dev->agent_id;
    u32 i, irq, cpuid, irq_start;
    u32 func_totl = agentdrv_res_get_func_total();
    u32 func_irq_num = AGENTDRV_NVME_DB_IRQ_NUM;

    if (drv_cpu_info->ccpu_num == 0) {
        devdrv_warn("Input ccpu_num is zero. (chip_id=%u; cpu_num=%u)\n", chip_id, drv_cpu_info->ccpu_num);
        return;
    }
    if (drv_cpu_info->dcpu_num == 0) {
        devdrv_warn("Input dcpu_num is zero. (chip_id=%u; dcpu_num=%u)\n", chip_id, drv_cpu_info->dcpu_num);
        drv_cpu_info->dcpu_num = drv_cpu_info->ccpu_num;
        drv_cpu_info->start_dcpuid = drv_cpu_info->start_ccpuid;
    }
    if (func_totl != 0) {
        func_irq_num = (u32)(AGENTDRV_NVME_DB_IRQ_NUM / func_totl);
    }
    irq_start = func_id * AGENTDRV_NVME_DB_IRQ_STRDE;
    /* doorbell low level to ctrl cpu */
    for (i = irq_start; i < AGENTDRV_NVME_LOW_LEVEL_DB_IRQ_NUM + irq_start; i++) {
        irq = agent_dev->p_agentdrv_msg_dev[func_id]->msi_irq_base + devdrv_nvme_agent_irq_vector2num(i);
        /* msg bind to data cpu for cloud v1 */
        if (devdrv_get_chip_type() == HISI_CLOUD_V1) {
            cpuid = devdrv_nvme_get_cpu_id(drv_cpu_info->start_dcpuid, drv_cpu_info->dcpu_num, i);
        } else {
            cpuid = devdrv_nvme_get_cpu_id(drv_cpu_info->start_ccpuid, drv_cpu_info->ccpu_num, i);
        }
        devdrv_info("Doorbell bind to cpu. (chip_id=%u; func_id=%d; irq=%u; doorball=%u; cpu_id=%u)\n",
            chip_id, func_id, irq, i, cpuid);
        (void)irq_set_affinity_hint(irq, get_cpu_mask(cpuid));
    }

    /* doorbell high level to traffic cpu */
    for (i = AGENTDRV_NVME_LOW_LEVEL_DB_IRQ_NUM + irq_start; i < func_irq_num + irq_start; i++) {
        irq = agent_dev->p_agentdrv_msg_dev[func_id]->msi_irq_base + devdrv_nvme_agent_irq_vector2num(i);
        cpuid = devdrv_nvme_get_cpu_id(drv_cpu_info->start_dcpuid, drv_cpu_info->dcpu_num, i);
        devdrv_info("Doorbell bind to cpu. (chip_id=%u; func_id=%u; irq=%u; doorball=%u; cpu_id=%u)\n",
            chip_id, func_id, irq, i, cpuid);
        (void)irq_set_affinity_hint(irq, get_cpu_mask(cpuid));
    }

    return;
}

void agentdrv_pci_unbind_irq(const struct agentdrv_devctrl *agent_dev)
{
    u32 i, irq, func_index;
    u32 pf_total = agentdrv_res_get_func_pf();

    if (agent_dev->cpu_info_valid != DEVDRV_VALID) {
        return;
    }

    /* doorbell unbind to ctrl cpu */
    for (func_index = 0; func_index < pf_total; func_index++) {
        if (agent_dev->p_agentdrv_msg_dev[func_index]->msi_irq_base == 0) {
            continue;
        }
        for (i = 0; i < AGENTDRV_NVME_DB_IRQ_NUM; i++) {
            irq = agent_dev->p_agentdrv_msg_dev[func_index]->msi_irq_base + devdrv_nvme_agent_irq_vector2num(i);
            (void)irq_set_affinity_hint(irq, NULL);
        }
    }

    return;
}

void agentdrv_platform_bind_irq(const struct agentdrv_devctrl *agent_dev, u32 func_id, u32 start_dcpuid, u32 dcpu_num)
{
    u32 irq, cpuid;

    if ((dcpu_num == 0) || (func_id != 0)) {
        devdrv_warn("Input parameter is zero.\n");
        return;
    }

    /* spi to traffic cpu */
    irq = agent_dev->platform_dev->irq_spi0;
    cpuid = start_dcpuid + dcpu_num - 1;
    (void)irq_set_affinity_hint(irq, get_cpu_mask(cpuid));

    irq = agent_dev->platform_dev->irq_spi1;
    cpuid = start_dcpuid + dcpu_num - 1;
    (void)irq_set_affinity_hint(irq, get_cpu_mask(cpuid));

    return;
}

void agentdrv_platform_unbind_irq(const struct agentdrv_devctrl *agent_dev)
{
    u32 irq;

    if (devdrv_get_chip_type() == HISI_MINI_V1) {
        return;
    }
    if (agent_dev->cpu_info_valid == DEVDRV_VALID) {
        /* spi to traffic cpu */
        irq = agent_dev->platform_dev->irq_spi0;
        (void)irq_set_affinity_hint(irq, NULL);

        irq = agent_dev->platform_dev->irq_spi1;
        (void)irq_set_affinity_hint(irq, NULL);
    }

    return;
}

void agentdrv_get_cpu_data_info(const struct agentdrv_devctrl *agent_dev, u32 func_index,
    struct agentdrv_cpu_data *drv_cpu_info)
{
    int devid;
    u32 total_num;
    u32 chip_id = agent_dev->agent_id;
    const struct agentdrv_cpu_info *info = &agent_dev->cpu_info[func_index];

    if (agent_dev->cpu_info_valid == DEVDRV_INVALID) {
        devdrv_info("No cpu information. (chip_id=%u)\n", chip_id);
        return;
    }

    drv_cpu_info->ccpu_num = info->ccpu_num;
    total_num = (u32)(info->ccpu_num * info->ccpu_os_sched + info->dcpu_num *
        info->dcpu_os_sched + info->aicpu_num * info->aicpu_os_sched +
        info->tscpu_num * info->tscpu_os_sched);

    if ((devdrv_is_sriov_enabled(agent_dev->func_totl_num) == true) && (func_index > 0)) {
        drv_cpu_info->start_ccpuid = total_num * chip_id;
        drv_cpu_info->dcpu_num = 0; // data cpu will not be used after enable sriov
    } else {
        devdrv_chipfunc2dev(&devid, (int)chip_id, (int)func_index);
        drv_cpu_info->start_ccpuid = (u32)(total_num * devid);
        drv_cpu_info->dcpu_num = info->dcpu_num;
        drv_cpu_info->start_dcpuid = drv_cpu_info->start_ccpuid + drv_cpu_info->ccpu_num;
    }

    devdrv_info("Get cpus information. (dev_id=%d; chip_id=%u; func_id=%u; start_ccpuid=%u; ccpu_num=%u;"
        " start_dcpuid=%u; dcpu_num=%u)\n", devid, chip_id, func_index, drv_cpu_info->start_ccpuid,
        drv_cpu_info->ccpu_num, drv_cpu_info->start_dcpuid, drv_cpu_info->dcpu_num);
}

void agentdrv_bind_irq(struct agentdrv_devctrl *agent_dev)
{
    u32 func_index;
    u32 pf_total = agentdrv_res_get_func_pf();
    struct agentdrv_cpu_data drv_cpu_info;

    if (devdrv_get_chip_type() == HISI_MINI_V1) {
        return;
    }

    for (func_index = 0; func_index < pf_total; func_index++) {
        agentdrv_get_cpu_data_info(agent_dev, func_index, &drv_cpu_info);
        agentdrv_dma_bind_irq(agent_dev, func_index, &drv_cpu_info);
        agentdrv_pci_bind_irq(agent_dev, func_index, &drv_cpu_info);
        agentdrv_platform_bind_irq(agent_dev, func_index, drv_cpu_info.start_dcpuid, drv_cpu_info.dcpu_num);
    }
}

void agentdrv_init_multi_chip(void)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    u32 i, online_cpus, node_id;
    u32 chip_id;
    u32 func_id;

    for (i = 0; i < MAX_AGENTCHIP_CNT; i++) {
        agent_dev = agentdrv_get_dev(i);
        agent_dev->agent_id = MAX_AGENTCHIP_CNT;
    }

    online_cpus = num_online_cpus();

    for (i = 0; i < online_cpus; i++) {
        node_id = cpu_to_node(i);
        if (node_id >= MAX_AGENTCHIP_CNT) {
            devdrv_err("Node ID is out of range. (node_id=%u; MAX_AGENTCHIP_CNT=%d)\n", node_id, MAX_AGENTCHIP_CNT);
            return;
        }

        devdrv_dev2chipfunc(node_id, &chip_id, &func_id);

        agent_dev = agentdrv_get_dev(chip_id);
        if (agent_dev->agent_id == MAX_AGENTCHIP_CNT) {
            agent_dev->agent_id = chip_id;

            (void)agentdrv_init_cpu_info(agent_dev);
        }
    }
}

