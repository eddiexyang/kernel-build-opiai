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
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/version.h>
#include <linux/of.h>
#include <linux/delay.h>
#include <linux/acpi.h>
#include <linux/of.h>
#include <linux/irq.h>
#include <linux/irqchip/arm-gic-v3.h>
#include <linux/msi.h>
#include <linux/iommu.h>

#include "dbl/runenv_config.h"
#include "dbl/uda.h"
#include "agentdrv_pci.h"
#include "agentdrv_dfx.h"
#include "agentdrv_unit.h"
#include "agentdrv_ctrl.h"
#include "agentdrv_platform.h"
#include "agentdrv_common_msg.h"
#include "agentdrv_doorbell.h"
#include "agentdrv_msg.h"
#include "agentdrv_sysfs.h"
#include "agentdrv_multi_chip.h"
#include "devdrv_util.h"
#include "devdrv_interface.h"
#include "dma_drv.h"
#include "nvme_comm_drv.h"
#include "resource_drv.h"
#include "kernel_version_adapt.h"
#include "devdrv_msg_def.h"
#include "agentdrv_pcie_link_info.h"

struct work_struct g_agent_dev_init;

svm_register_pci_device_syms devdrv_svm_register_pci_device_syms_func = NULL;
svm_unregister_pci_device_syms devdrv_svm_unregister_pci_device_syms_func = NULL;
svm_enable_sva_enforce_syms devdrv_svm_enable_sva_enforce_syms_func = NULL;

struct pci_saved_state *pci_saved_state = NULL;

struct agentdrv_dev_status {
    int local_status;
    int agent_status;
};

static struct agentdrv_dev_status dev_status[MAX_AGENTDEV_CNT];

struct agentdrv_handshake_status {
    atomic_t ep_status;
    atomic_t rc_status;
    atomic_t ep_wait_status;
    wait_queue_head_t ep_wait_queue;
};
static struct agentdrv_handshake_status g_suspend_status;

STATIC const struct agentdrv_non_trans_msg_client g_common_msg_client = {
    .type = agentdrv_msg_client_common,
    .flag = AGENTDRV_MSG_SYNC,
    .init_non_trans_msg_chan = agentdrv_init_common_msg_chan,
    .uninit_non_trans_msg_chan = agentdrv_uninit_common_msg_chan,
    .non_trans_msg_process = agentdrv_rx_msg_common_msg_process,
};

const char *g_pcie_db_irq_name[MAX_AGENTCHIP_CNT] = {
    "dev_0_pcie_db_irq", "dev_1_pcie_db_irq", "dev_2_pcie_db_irq", "dev_3_pcie_db_irq"
};

STATIC void agentdrv_dev_online_proc(const struct agentdrv_msg_dev *msg_dev, u32 online_devid, u32 status)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    int host_dev_id;

    agent_dev = agentdrv_get_dev(msg_dev->dev_id);
    host_dev_id = agent_dev->shr_para[msg_dev->func_id]->host_dev_id;

    devdrv_info("Update the online status. (dev_id=%d; host_id=%d; status=%u; online_devid=%u)\n",
        msg_dev->dev_id, host_dev_id, status, online_devid);

    if ((status == DEVDRV_DEV_OFFLINE) && (host_dev_id == online_devid)) {
        devdrv_info("Device driver unregistered. (dev_id=%d; host_id=%d)\n", msg_dev->dev_id, host_dev_id);
        agentdrv_dev_unregister(agent_dev, msg_dev->func_id);
    }
}

int agentdrv_notify_dev_online(struct agentdrv_msg_dev *msg_dev, void *data)
{
    struct devdrv_notify_dev_online_cmd *cmd_data = (struct devdrv_notify_dev_online_cmd *)data;
    struct devdrv_admin_msg_reply *reply = (struct devdrv_admin_msg_reply *)data;

    agentdrv_dev_online_proc(msg_dev, cmd_data->devid, cmd_data->status);

    /* no response message */
    reply->len = sizeof(struct devdrv_admin_msg_reply);

    return 0;
}

#ifdef CFG_FEATURE_PM
void agentdrv_dfm_dump(u32 excep_id, u32 etype, u32 module_id, char **black_box_info)
{
    UNUSED(excep_id);
    UNUSED(etype);
    UNUSED(module_id);

    if (black_box_info == NULL) {
        devdrv_info("Input parameter invalid.\n");
        return;
    }
    *black_box_info = g_dfx_info.bbox_storage_str;

    return;
}
#else
void agentdrv_dfm_dump(u32 dev_id, u32 excep_id, u32 etype, u32 module_id, char **black_box_info)
{
    UNUSED(dev_id);
    UNUSED(excep_id);
    UNUSED(etype);
    UNUSED(module_id);

    if (black_box_info == NULL) {
        devdrv_info("Input parameter invalid.\n");
        return;
    }
    *black_box_info = g_dfx_info.bbox_storage_str;

    return;
}
#endif

struct dfm_module_register g_agentdrv_ops_pub = {
    .module_id = DFM_MODULE_ID_DRIVER,
    .sub_module_id = DFM_SUBMODULE_ID_PCIE,
    .ops_dump = agentdrv_dfm_dump,
};

int agentdrv_cfg_pdev(struct pci_dev *pdev)
{
    int err;

    err = pci_enable_device_mem(pdev);
    if (err) {
        devdrv_err("PCI enable device failed. (err=%d)\n", err);
        return err;
    }
    /*lint -e598 -e648 */
    if (dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(AGENTDRV_DMA_BIT_MASK_64))) {
        /*lint +e598 +e648 */
        dev_dbg(&pdev->dev, "pci probe,set mask to 64bit fail,try 32bit...\n");
        err = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(AGENTDRV_DMA_BIT_MASK_32));
        if (err) {
            devdrv_err("DMA set 32bit mask failed. (err=%d)\n", err);
            goto set_dma_mask_fail;
        }
    }
    err = pci_request_regions(pdev, "agentdrv");
    if (err) {
        devdrv_err("PCI request regions failed. (err=%d)\n", err);
        goto request_regions_fail;
    }

    pci_set_master(pdev);

    return 0;

request_regions_fail:
set_dma_mask_fail:
    pci_disable_device(pdev);
    return err;
}

void agentdrv_uncfg_pdev(struct pci_dev *pdev)
{
    pci_clear_master(pdev);
    pci_release_regions(pdev);
    pci_disable_device(pdev);
}

int agentdrv_alloc_msi_interrupts(struct pci_dev *pdev, int min, int max)
{
    int ret;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)
    /* request msi interrupt */
    ret = pci_alloc_irq_vectors(pdev, min, max, PCI_IRQ_MSI);
#else
    ret = pci_enable_msi_range(pdev, min, max);
#endif
    return ret;
}

void agentdrv_release_msi_interrupts(struct pci_dev *pdev)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)
    pci_free_irq_vectors(pdev);
#else
    pci_disable_msi(pdev);
#endif
}

int devdrv_dma_chan_remote_op_func(struct agentdrv_msg_dev *msg_dev, void *data)
{
    int ret = 0;
    struct devdrv_dma_chan_remote_op *cmd_data = (struct devdrv_dma_chan_remote_op *)data;
    struct devdrv_admin_msg_reply *reply = (struct devdrv_admin_msg_reply *)data;
    struct devdrv_dma_dev *dma_dev = msg_dev->dma_dev;
    u32 chan_id = cmd_data->chan_id;
    void __iomem *io_base = (void __iomem *)((u8 *)dma_dev->dma_chan_base + (u64)chan_id * DEVDRV_DMA_CHAN_OFFSET);
    u32 queue_init_sts;
    struct devdrv_dma_cfg_info dma_info = {0};
    struct agentdrv_devctrl *agent_dev = NULL;

    agent_dev = agentdrv_get_dev(msg_dev->dev_id);

    ret = agentdrv_is_remote_dma_chan(dma_dev, chan_id);
    if (ret != 0) {
        devdrv_err("chan_id is out of range. (dev_id=%u; func_id=%u; chan_id=%u)\n",
            msg_dev->dev_id, msg_dev->func_id, chan_id);
        return -EINVAL;
    }

    if (cmd_data->op == DMA_CHAN_REMOTE_OP_RESET) {
        ret = devdrv_dma_ch_cfg_reset(io_base, cmd_data->sriov_flag);
        if (ret != 0) {
            devdrv_err("Reset channel config failed. (chan_id=%u; ret=%d)\n", chan_id, ret);
        }
    } else if (cmd_data->op == DMA_CHAN_REMOTE_OP_INIT) {
        dma_info.pf_num = cmd_data->pf_num;
        dma_info.vf_num = cmd_data->vf_num;
        if ((cmd_data->sriov_flag == DEVDRV_MDEV_VF_VM_BOOT) ||
            (cmd_data->sriov_flag == DEVDRV_MDEV_FULL_SPEC_VF_VM_BOOT)) {
            dma_info.sq_addr = agent_dev->shr_para[msg_dev->func_id]->sq_desc_dma;
        } else {
            dma_info.sq_addr = cmd_data->sq_desc_dma;
        }
        dma_info.cq_addr = cmd_data->cq_desc_dma;
        dma_info.sq_depth = cmd_data->sq_depth;
        dma_info.cq_depth = cmd_data->cq_depth;
        dma_info.sqcq_side = cmd_data->sqcq_side;

        devdrv_dma_ch_cfg_init(io_base, &dma_info);
        devdrv_dma_check_sram_init_status(dma_dev->io_base, DEVDRV_DMA_TIMEOUT);

        /* enable DMA channel */
        devdrv_set_dma_chan_en(io_base, 1);
    } else {
        devdrv_record_dma_dxf_info(io_base, &queue_init_sts);
    }

    /* no response message */
    reply->len = sizeof(struct devdrv_admin_msg_reply);

    return ret;
}

int devdrv_dma_chan_init(struct devdrv_dma_channel *dma_chan)
{
    struct devdrv_dma_cfg_info dma_info = {0};

    dma_info.pf_num = dma_chan->dma_dev->dma_pf_num;
    dma_info.vf_num = dma_chan->dma_dev->dma_vf_num;
    dma_info.sq_addr = dma_chan->sq_desc_dma;
    dma_info.cq_addr = dma_chan->cq_desc_dma;
    dma_info.sq_depth = dma_chan->sq_depth;
    dma_info.cq_depth = dma_chan->cq_depth;
    dma_info.sqcq_side = dma_chan->dma_dev->sq_cq_side;

    devdrv_dma_ch_cfg_init(dma_chan->io_base, &dma_info);
    devdrv_dma_check_sram_init_status(dma_chan->dma_dev->io_base, DEVDRV_DMA_TIMEOUT);
    /* enable DMA channel */
    devdrv_set_dma_chan_en(dma_chan->io_base, 1);

    return 0;
}

int devdrv_dma_chan_reset(struct devdrv_dma_channel *dma_chan, u32 sriov_flag)
{
    return devdrv_dma_ch_cfg_reset(dma_chan->io_base, sriov_flag);
}

int devdrv_dma_chan_err_proc(struct devdrv_dma_channel *dma_chan)
{
    u32 queue_init_sts;

    devdrv_record_dma_dxf_info(dma_chan->io_base, &queue_init_sts);
    devdrv_dfx_dma_report_to_bbox(dma_chan, queue_init_sts);

    return 0;
}

int agentdrv_dma_proc_func_init(struct pci_dev *pdev, struct agentdrv_devctrl *agent_dev, u32 func_idx)
{
    struct devdrv_dma_func_para para_in = {0};
    struct devdrv_res_dma_common_info rm_out = {0};
    struct agentdrv_pfvf_info pfvf_info = { 0 };

    para_in.drvdata = (void *)agent_dev;
    para_in.dev = &pdev->dev;
    para_in.io_base = agent_dev->dma_base;
    para_in.dma_chan_base = agent_dev->dma_chan_base;
    para_in.dev_id = agent_dev->agent_id;

    agentdrv_dma_res_common(func_idx, pdev->irq, &rm_out);
    para_in.chan_num = rm_out.chan_num;
    para_in.done_irq_base = rm_out.done_irq_base;
    para_in.err_irq_base = rm_out.err_irq_base;
    para_in.err_flag = rm_out.err_flag;
    para_in.sq_cq_info.sq_depth = rm_out.sq_cq_info.sq_depth;
    para_in.sq_cq_info.sq_rsv_num = rm_out.sq_cq_info.sq_rsv_num;
    para_in.sq_cq_info.cq_depth = rm_out.sq_cq_info.cq_depth;

    devdrv_soc_func2pfvf(agent_dev->agent_id, func_idx, 1, &pfvf_info);
    para_in.dma_pf_num = pfvf_info.pf_num;
    para_in.dma_vf_num = pfvf_info.vf_num;
    para_in.dma_vf_en = pfvf_info.vf_en;
    para_in.chip_type = devdrv_get_chip_type();

    agent_dev->p_agentdrv_dma_dev[func_idx] = devdrv_dma_init(&para_in, DEVDRV_DMA_LOCAL_SIDE, func_idx);
    if (agent_dev->p_agentdrv_dma_dev[func_idx] == NULL) {
        devdrv_err("DMA init failed. (dev_id=%u)\n", agent_dev->agent_id);
        return -1;
    }

    return 0;
}

int devdrv_get_runtime_runningplat(u32 devid, u64 *running_plat)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    u32 chip_id;
    u32 func_id;
    u32 val;
    int ret;

    if (devid >= MAX_AGENTDEV_CNT) {
        devdrv_err("Input parameter devid error.\n");
        return -EINVAL;
    }

    if (running_plat == NULL) {
        devdrv_err("Input parameter running_plat error.\n");
        return -EINVAL;
    }

    ret = agentdrv_get_rc_ep_mode(&val);
    if (ret) {
        devdrv_err("Call agentdrv_get_rc_ep_mode failed.\n");
        return -EINVAL;
    }
    if (val == DEVDRV_PCIE_RC_MODE) {
        *running_plat = 0;
        return 0;
    }

    devdrv_dev2chipfunc(devid, &chip_id, &func_id);

    agent_dev = agentdrv_get_dev(chip_id);
    if ((agent_dev != NULL) && (agentdrv_is_dev_valid(agent_dev, func_id) == true)) {
        *running_plat = agent_dev->shr_para[func_id]->runtime_runningplat;
        rmb();
    } else {
        devdrv_err("Call agentdrv_get_dev failed, agent_dev is error. (devid=%u)\n", devid);
        return -EINVAL;
    }

    return 0;
}
EXPORT_SYMBOL(devdrv_get_runtime_runningplat);

int devdrv_set_runtime_runningplat(u32 devid, u64 running_plat)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    u32 chip_id;
    u32 func_id;
    int ret;
    u32 val;

    if (devid >= MAX_AGENTDEV_CNT) {
        devdrv_err("Device ID is invalid.\n");
        return -EINVAL;
    }

    ret = agentdrv_get_rc_ep_mode(&val);
    if (ret) {
        devdrv_err("Call agentdrv_get_rc_ep_mode failed.\n");
        return -EINVAL;
    }
    if (val == DEVDRV_PCIE_RC_MODE) {
        return 0;
    }

    devdrv_dev2chipfunc(devid, &chip_id, &func_id);

    agent_dev = agentdrv_get_dev(chip_id);
    if ((agent_dev != NULL) && (agentdrv_is_dev_valid(agent_dev, func_id) == true)) {
        agent_dev->shr_para[func_id]->runtime_runningplat = running_plat;
        wmb();
    } else {
        devdrv_err("Call agentdrv_get_dev failed, agent_dev is error. (devid=%u)\n", devid);
        return -EINVAL;
    }

    return 0;
}
EXPORT_SYMBOL(devdrv_set_runtime_runningplat);

void agentdrv_dma_proc_func_uninit(u32 func_num, struct agentdrv_devctrl *agent_dev)
{
    u32 func_idx;
    for (func_idx = 0; func_idx < func_num; func_idx++) {
        devdrv_dma_exit(agent_dev->p_agentdrv_dma_dev[func_idx], DEVDRV_SRIOV_DISABLE);
    }
}

STATIC int agentdrv_dma_dev_init(struct agentdrv_devctrl *agent_dev, struct pci_dev *pdev)
{
    int i;
    int ret;
    int channel_id;
    int chip_type = devdrv_get_chip_type();
    u32 func_idx = 0;

    agent_dev->func_pf_num = agentdrv_res_get_func_pf();
    for (func_idx = 0; func_idx < agent_dev->func_pf_num; func_idx++) {
        ret = agentdrv_dma_proc_func_init(pdev, agent_dev, func_idx);
        if (ret != 0) {
            devdrv_err("dma_dev alloc failed. (devid=%u; fun=%u; ret=%d)\n", agent_dev->agent_id, func_idx, ret);
            agentdrv_dma_proc_func_uninit(func_idx, agent_dev);
            return ret;
        }
    }

    if (chip_type == HISI_MINI_V1) {
        channel_id = DMA_CHAN_REMOTE_USED_START_INDEX;
        for (i = 0; i < DMA_CHAN_REMOTE_USED_NUM; i++) {
            (void)devdrv_register_irq_func(NULL, pdev->irq + channel_id, devdrv_remote_dma_err_interrupt, agent_dev,
                                           "dma_err_irq");
            channel_id++;
        }
    } else {
        (void)devdrv_register_irq_func(NULL, pdev->irq + DMA_ERR_IRQ_BASE, devdrv_all_dma_err_interrupt, agent_dev,
                                       "dma_err_irq");
    }
    devdrv_dma_err_interrupt_unmask(agent_dev->dma_base);
    return 0;
}

void agentdrv_get_pf_from_shr_para(u32 devid)
{
    struct devdrv_shr_para __iomem *shr_para = NULL;
    struct agent_res_sdi_addr sdi_addr = {0};
    u32 pf_num;

    agentdrv_res_sdi_addr(devid, 0, &sdi_addr);
    shr_para =(struct devdrv_shr_para __iomem *)ioremap(sdi_addr.shr_base, sdi_addr.shr_size);
    if (shr_para == NULL) {
        devdrv_err("Ioremap share mem base failed. (devid=%u)\n", devid);
        agentdrv_dfx_put_string("ioremap share mem base fail\n");
        devdrv_set_nvme_pf_num(devid, DEVDRV_PF_NUM);
        return;
    }
    pf_num = shr_para->ep_pf_index;
    devdrv_set_nvme_pf_num(devid, pf_num);
    iounmap(shr_para);
    shr_para = NULL;

    return;
}

STATIC int agentdrv_register_svm_pci_device(struct device *dev, int dev_id)
{
#ifdef CFG_FEATURE_PASSID
    int ret;

    if (dev_iommu_priv_get(dev) == NULL) {
        devdrv_warn("Smmu not enabled, not support passid.\n");
        return 0;
    }

    if (devdrv_svm_enable_sva_enforce_syms_func == NULL) {
        devdrv_svm_enable_sva_enforce_syms_func = (svm_enable_sva_enforce_syms)(uintptr_t)\
            __kallsyms_lookup_name("arm_smmu_master_enable_sva_enforce");
    }

    if (devdrv_svm_enable_sva_enforce_syms_func != NULL) {
        ret = devdrv_svm_enable_sva_enforce_syms_func(dev);
        if (ret) {
            devdrv_err("svm_enable_sva_enforce failed. (ret=%d)\n", ret);
            return ret;
        }
    }

    if (devdrv_svm_register_pci_device_syms_func == NULL) {
        devdrv_svm_register_pci_device_syms_func = (svm_register_pci_device_syms)(uintptr_t)\
            __kallsyms_lookup_name("svm_register_pci_device");
    }

    if (devdrv_svm_register_pci_device_syms_func != NULL) {
        ret = devdrv_svm_register_pci_device_syms_func(dev, dev_id + AGENTDRV_SVM0_PCI_DEV_INDEX_BASE);
        if (ret) {
            devdrv_err("svm_register_pci_device failed. (ret=%d)\n", ret);
            return ret;
        }
    }
#endif
    return 0;
}

STATIC void agentdrv_unregister_svm_pci_device(struct device *dev, int dev_id)
{
#ifdef CFG_FEATURE_PASSID
    int ret;

    if (dev_iommu_priv_get(dev) == NULL) {
        devdrv_warn("Smmu not enabled, not support passid.\n");
        return;
    }

    if (devdrv_svm_unregister_pci_device_syms_func == NULL) {
        devdrv_svm_unregister_pci_device_syms_func = (svm_unregister_pci_device_syms)(uintptr_t)\
            __kallsyms_lookup_name("svm_unregister_pci_device");
    }

    if (devdrv_svm_unregister_pci_device_syms_func != NULL) {
        ret = devdrv_svm_unregister_pci_device_syms_func(dev, dev_id + AGENTDRV_SVM0_PCI_DEV_INDEX_BASE);
        if (ret) {
            devdrv_err("svm_unregister_pci_device failed. (ret=%d)\n", ret);
            return;
        }
    }
#endif
    return;
}

STATIC int agentdrv_dma_probe(struct pci_dev *pdev, const struct pci_device_id *data)
{
    int ret;
    int dev_id = 0;
    struct agentdrv_devctrl *agent_dev = NULL;

    if (devdrv_get_devid_by_bus(pdev->bus->number, &dev_id) != 0) {
        devdrv_info("DMA probe bus not support. (bus=%d)\n", pdev->bus->number);
        agentdrv_dfx_put_string("dma probe bus not support\n");
        agentdrv_notify_blackbox_exception(dev_id, AGENTDRV_DMA_PROBE_FAIL, g_dfx_info.bbox_storage_str);
        return -EINVAL;
    }

    devdrv_info("DMA driver has match succeeded, probe start. (dev_id=%d)\n", dev_id);

    agent_dev = agentdrv_get_dev(dev_id);
    agent_dev->agent_id = dev_id;
    agent_dev->pdev = pdev;

    ret = agentdrv_cfg_pdev(pdev);
    if (ret) {
        devdrv_err("agentdrv_cfg_pdev failed. (dev_id=%d)\n", dev_id);
        agentdrv_dfx_put_string("agentdrv_cfg_pdev fail\n");
        agentdrv_notify_blackbox_exception(dev_id, AGENTDRV_DMA_PROBE_FAIL, g_dfx_info.bbox_storage_str);
        return ret;
    }

    ret = agentdrv_register_svm_pci_device(&pdev->dev, dev_id);
    if (ret != 0) {
        devdrv_err("svm_register_pci_device failed. (dev_id=%d)\n", dev_id);
        goto uncfg_pdev;
    }

    agent_dev->dma_base = ioremap(pci_resource_start(pdev, AGENTDRV_PCI_BAR_2),
                                  pci_resource_len(pdev, AGENTDRV_PCI_BAR_2));
    if (agent_dev->dma_base == NULL) {
        devdrv_err("DMA ioremap bar 2 failed. (dev_id=%d)\n", dev_id);
        ret = -ENOMEM;
        agentdrv_dfx_put_string("dma ioremap bar 2 fail\n");
        goto unregister_svm;
    }

    agent_dev->dma_chan_base = devdrv_get_dma_chan_base(agent_dev->dma_base);

    ret = agentdrv_alloc_msi_interrupts(pdev, DEVDRV_DMA_MSI_MAX_VECTORS, DEVDRV_DMA_MSI_MAX_VECTORS);
    if (ret < 0) {
        devdrv_err("DMA PCI alloc msi interrupts failed. (dev_id=%d; ret=%d)\n", dev_id, ret);
        agentdrv_dfx_put_string("dma pci alloc msi irqs failed\n");
        goto iounmap_dma_base;
    }
    agentdrv_get_pf_from_shr_para((u32)dev_id);

    atomic_inc(&agent_dev->subdev_num);
    devdrv_info("DMA probe success. (dev_id=%d)\n", dev_id);

    return 0;

iounmap_dma_base:
    iounmap(agent_dev->dma_base);
    agent_dev->dma_base = NULL;
unregister_svm:
    agentdrv_unregister_svm_pci_device(&pdev->dev, dev_id);
uncfg_pdev:
    agentdrv_uncfg_pdev(pdev);
    agentdrv_notify_blackbox_exception(dev_id, AGENTDRV_DMA_PROBE_FAIL, g_dfx_info.bbox_storage_str);

    return ret;
}

STATIC void agentdrv_dma_remove(struct pci_dev *pdev)
{
    int channel_id;
    int dev_id;
    struct agentdrv_devctrl *agent_dev = NULL;
    u32 i;

    if (devdrv_get_devid_by_bus(pdev->bus->number, &dev_id) != 0) {
        devdrv_info("DMA remove bus not support. (bus=%d)\n", pdev->bus->number);
        return;
    }

    agent_dev = agentdrv_get_dev(dev_id);
    for (i = 0; i < agent_dev->func_totl_num; i++) {
        agentdrv_dma_unbind_irq(agent_dev, i);
    }

    if (devdrv_get_chip_type() == HISI_MINI_V1) {
        channel_id = DMA_CHAN_REMOTE_USED_START_INDEX;
        for (i = 0; i < DMA_CHAN_REMOTE_USED_NUM; i++) {
            (void)devdrv_unregister_irq_func(NULL, pdev->irq + channel_id, agent_dev);
            channel_id++;
        }
    } else {
        (void)devdrv_unregister_irq_func(NULL, pdev->irq + DMA_ERR_IRQ_BASE, agent_dev);
    }

    agentdrv_dma_proc_func_uninit(agent_dev->func_totl_num, agent_dev);

    agentdrv_release_msi_interrupts(pdev);

    if (agent_dev->dma_base != NULL) {
        iounmap(agent_dev->dma_base);
        agent_dev->dma_base = NULL;
        agent_dev->dma_chan_base = NULL;
    }

    agentdrv_unregister_svm_pci_device(&pdev->dev, dev_id);

    agentdrv_uncfg_pdev(pdev);
}
void agentdrv_wake_up_ep_suspend(void)
{
    if (atomic_read(&g_suspend_status.rc_status) == 1) {
        atomic_set(&g_suspend_status.ep_wait_status, 1);
        wake_up_interruptible(&g_suspend_status.ep_wait_queue);
    }
    return;
}
int agentdrv_get_ep_suspend_status(struct agentdrv_msg_dev *msg_dev, void *data)
{
    struct devdrv_admin_msg_reply *reply = (struct devdrv_admin_msg_reply *)data;
    struct devdrv_ep_suspend_status *status = NULL;

    status = (struct devdrv_ep_suspend_status *)reply->data;
    if (atomic_read(&g_suspend_status.ep_status) == 0) {
        status->status = 0;
    } else {
        devdrv_info("Ep suspend is ready.\n");
        atomic_set(&g_suspend_status.rc_status, 1);
        status->status = 1;
    }
    reply->len = (u32)sizeof(struct devdrv_admin_msg_reply) + (u32)sizeof(struct devdrv_ep_suspend_status);

    return 0;
}

static char g_agentdrv_dma_driver_name[] = "agentdrv_pci_dma_device_driver";
static const struct pci_device_id g_agentdrv_dma_tbl[] = {{ PCI_VDEVICE(HUAWEI, HISI_IEP_DMA_DEVICE_ID), 0 }, {}};
MODULE_DEVICE_TABLE(pci, g_agentdrv_dma_tbl);

void agentdrv_msg_func_uinit(struct agentdrv_devctrl *agent_dev);
static int agentdrv_dev_ctrl_notify(struct agentdrv_devctrl *agent_dev, enum uda_dev_ctrl_cmd cmd);

#ifdef CFG_SOC_FEATURE_SR
static void agentdrv_clear_suspend_status(void)
{
    atomic_set(&g_suspend_status.ep_wait_status, 0);
    atomic_set(&g_suspend_status.ep_status, 0);
    atomic_set(&g_suspend_status.rc_status, 0);
    return;
}

static int agentdrv_suspend_wait_handshake_proc(struct agentdrv_devctrl *agent_dev, int dev_id)
{
    int count = 0;
    int ret;

    atomic_set(&g_suspend_status.ep_status, 1);
    devdrv_info("Wait for RC suspend to be ready. \n");
    while (atomic_read(&g_suspend_status.rc_status) != 1) {
        msleep(AGENTDRV_GET_EP_SUSPEND_STATUS_DELAY);
        count++;
        if (count >= AGENTDRV_GET_EP_SUSPEND_STATUS_TIMEOUT) {
            atomic_set(&g_suspend_status.ep_status, 0);
            devdrv_err("Rc suspend timeout.\n");
            return -EBUSY;
        }
    }
    agentdrv_mask_suspend_fault(agent_dev->apb_base);

    ret = wait_event_interruptible_timeout(g_suspend_status.ep_wait_queue,
        (atomic_read(&g_suspend_status.ep_wait_status) == 1),
        msecs_to_jiffies(AGENTDRV_SEND_EP_HANDSHAKE_MSG_TIMEOUT));
    if (ret <= 0) {
        agentdrv_clear_suspend_status();
        devdrv_err("unable to reply handshake msg to host. (ret=%d, dev_id=%d)\n", ret, dev_id);
        return -EBUSY;
    }
    agentdrv_clear_suspend_status();
    devdrv_info("rc & ep handshake ok. \n");
    return 0;
}
STATIC int agentdrv_dma_suspend(struct pci_dev *dev, pm_message_t state)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    int dev_id = 0;
    int ret;

    devdrv_info("Suspend dma driver start.\n");
    devdrv_set_pcie_channel_status(AGENTDRV_PCIE_INIT_ERR);
    if (devdrv_get_devid_by_bus(dev->bus->number, &dev_id) != 0) {
        devdrv_info("DMA remove bus not support. (bus=%d)\n", dev->bus->number);
        return 0;
    }

    if (dev_status[dev_id].agent_status != AGENTDRV_STATUS_OK) {
        devdrv_err("agent status is not ok. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    agent_dev = agentdrv_get_dev(dev_id);
    if (IS_ERR_OR_NULL(agent_dev->apb_base)) {
        devdrv_err("apb base is invalid. (dev_id=%d)\n", dev_id);
        return -ENOMEM;
    }

    ret = pci_save_state(dev);
    if (ret != 0) {
        devdrv_err("pci_save_state failed. (ret=%d)\n", ret);
        return -ENOMEM;
    }

    pci_saved_state = pci_store_saved_state(dev);
    if (pci_saved_state == NULL) {
        devdrv_err("pci_store_saved_state failed.\n");
        return -ENOMEM;
    }

    ret = agentdrv_suspend_wait_handshake_proc(agent_dev, dev_id);
    if (ret != 0) {
        return ret;
    }

    ret = agentdrv_dev_ctrl_notify(agent_dev, UDA_CTRL_SUSPEND);
    if (ret != 0) {
        devdrv_err("uda ctrl fail. (dev_id=%d; ret=%d)\n", dev_id, ret);
        return ret;
    }

    if (agentdrv_get_soc_doorbell_capability() == true) {
        agentdrv_pci_unbind_irq(agent_dev);
    }
    agentdrv_msg_func_uinit(agent_dev);

    agentdrv_dma_remove(dev);

    devdrv_info("Suspend dma driver success.\n");
    return 0;
}
#endif

static struct pci_driver g_agentdrv_pci_dma_driver = {
    .name = g_agentdrv_dma_driver_name,
    .id_table = g_agentdrv_dma_tbl,
    .probe = agentdrv_dma_probe,
    .remove = agentdrv_dma_remove,
    .driver = {
        .name = "agentdrv_pci_dma_device_driver",
    },
#ifdef CFG_SOC_FEATURE_SR
    .suspend = agentdrv_dma_suspend,
#endif
};

int agentdrv_sdi_proc_func_init(struct agentdrv_devctrl *agent_dev, u32 func_id)
{
    int ret = 0;
    struct agentdrv_msg_dev *msg_dev = NULL;
    struct agent_res_sdi_addr sdi_addr = {0};

    /* alloc msg dev */
    agent_dev->p_agentdrv_msg_dev[func_id] = agentdrv_alloc_msg_dev(func_id);
    if (agent_dev->p_agentdrv_msg_dev[func_id] == NULL) {
        devdrv_err("Device alloc msg_dev failed. (dev_id=%u; func=%u)\n", agent_dev->agent_id, func_id);
        agentdrv_dfx_put_string("alloc msg_dev failed!\n");
        ret = -ENOMEM;
        return ret;
    }

    /* --- msg */
    msg_dev = agent_dev->p_agentdrv_msg_dev[func_id];
    msg_dev->priv_pci_unit = (void *)agent_dev;
    msg_dev->dev_id = agent_dev->agent_id;
    msg_dev->func_id = func_id;

    /* for pf only, dma probe should before than sdi probe, otherwise sdi have no dma dev use */
    if (agent_dev->p_agentdrv_dma_dev[func_id] != NULL) {
        msg_dev->dma_dev = agent_dev->p_agentdrv_dma_dev[func_id];
    }
    msg_dev->dev = &agent_dev->pdev->dev;

    /* init nvme global and pf local regs base */
    msg_dev->sdi_base = agent_dev->sdi_base[func_id];
    msg_dev->raise_int_base = agent_dev->raise_int_base[func_id];
    msg_dev->msi_irq_base = agent_dev->msi_irq_base[func_id][0];

    /* --- init doorbell regs base */
    agentdrv_res_sdi_addr(msg_dev->dev_id, msg_dev->func_id, &sdi_addr);

    ret = agentdrv_ioremap_doorbell_base(&msg_dev->doorbell_base, sdi_addr.db_base, sdi_addr.db_size);
    if (ret != 0) {
        devdrv_err("Ioremap doorbell_base failed. (dev_id=%u; func=%u)\n", agent_dev->agent_id, func_id);
        agentdrv_dfx_put_string("ioremap doorbell_base fail\n");
        goto free_msg_dev;
    }

    /* init reserve mem base */
    msg_dev->reserve_mem_base = ioremap_cache(sdi_addr.msg_base, sdi_addr.msg_size);
    if (msg_dev->reserve_mem_base == NULL) {
        devdrv_err("Ioremap reserver_mem_base failed. (dev_id=%u; func=%u)\n", agent_dev->agent_id, func_id);
        agentdrv_dfx_put_string("ioremap reserver_mem_base fail\n");
        ret = -ENOMEM;
        goto iounmap_doorbell;
    }

    if (sdi_addr.shr_type == DEVDRV_SHR_MEM_CACHE) {
        agent_dev->shr_para[func_id] = (struct devdrv_shr_para __iomem *)ioremap_cache(sdi_addr.shr_base,
            sdi_addr.shr_size);
    } else {
        agent_dev->shr_para[func_id] = (struct devdrv_shr_para __iomem *)ioremap(sdi_addr.shr_base, sdi_addr.shr_size);
    }
    if (agent_dev->shr_para[func_id] == NULL) {
        devdrv_err("Ioremap share_mem_base failed. (dev_id=%u; func=%u)\n", agent_dev->agent_id, func_id);
        agentdrv_dfx_put_string("ioremap share_mem_base fail\n");
        ret = -ENOMEM;
        goto iounmap_reserve_mem;
    }

    /* --- init msg chan */
    ret = agentdrv_msg_dev_init(msg_dev);
    if (ret) {
        devdrv_err("msg_dev init failed. (dev_id=%u; func=%u)\n", agent_dev->agent_id, func_id);
        agentdrv_dfx_put_string("msg_dev init failed!\n");
        goto iounmap_share_mem;
    }

    return ret;

iounmap_share_mem:
    iounmap(agent_dev->shr_para[func_id]);
    agent_dev->shr_para[func_id] = NULL;

iounmap_reserve_mem:
    iounmap(msg_dev->reserve_mem_base);
    msg_dev->reserve_mem_base = NULL;

iounmap_doorbell:
    if (msg_dev->doorbell_base != NULL) {
        iounmap(msg_dev->doorbell_base);
        msg_dev->doorbell_base = NULL;
    }

free_msg_dev:
    agentdrv_free_msg_dev(agent_dev->p_agentdrv_msg_dev[func_id]);

    return ret;
}

void agentdrv_sdi_proc_func_uninit(u32 func_num_finished, struct agentdrv_devctrl *agent_dev)
{
    u32 func_idx = 0;

    for (func_idx = 0; func_idx < func_num_finished; func_idx++) {
        if (agent_dev->p_agentdrv_msg_dev[func_idx] != NULL) {
            agentdrv_msg_dev_exit(agent_dev->p_agentdrv_msg_dev[func_idx]);
            iounmap(agent_dev->shr_para[func_idx]);
            agent_dev->shr_para[func_idx] = NULL;
            iounmap(agent_dev->p_agentdrv_msg_dev[func_idx]->reserve_mem_base);
            agent_dev->p_agentdrv_msg_dev[func_idx]->reserve_mem_base = NULL;
            if (agent_dev->p_agentdrv_msg_dev[func_idx]->doorbell_base != NULL) {
                iounmap(agent_dev->p_agentdrv_msg_dev[func_idx]->doorbell_base);
                agent_dev->p_agentdrv_msg_dev[func_idx]->doorbell_base = NULL;
            }
            agentdrv_free_msg_dev(agent_dev->p_agentdrv_msg_dev[func_idx]);
            agent_dev->p_agentdrv_msg_dev[func_idx] = NULL;
        }
    }
}

int agentdrv_nvme_proc_init(struct pci_dev *pdev, struct agentdrv_devctrl *agent_dev)
{
    int ret = -ENOMEM;
    u32 func_index;
    u32 func_totl = agentdrv_res_get_func_total();

    /* get sdi base */
    agent_dev->sdi_base[0] = ioremap(pci_resource_start(pdev, AGENTDRV_PCI_BAR_2),
        pci_resource_len(pdev, AGENTDRV_PCI_BAR_2));
    if (agent_dev->sdi_base[0] == NULL) {
        devdrv_err("Ioremap bar IO failed. (dev_id=%u)\n", agent_dev->agent_id);
        agentdrv_dfx_put_string("ioremap bar IO fail\n");
        return ret;
    }

    /* nvme doorbell only has one irq platform, this 'for loop' compatibles for mini v2 2p */
    for (func_index = 0; func_index < func_totl; func_index++) {
        agent_dev->sdi_base[func_index] = agent_dev->sdi_base[0];
        agent_dev->raise_int_base[func_index] = agent_dev->sdi_base[0];
    }

    ret = agentdrv_alloc_msi_interrupts(pdev, AGENTDRV_PCI_MSI_NUM, AGENTDRV_PCI_MSI_NUM);
    if (ret < 0) {
        devdrv_err("agentdrv_alloc_msi_interrupts failed. (dev_id=%u)\n", agent_dev->agent_id);
        agentdrv_dfx_put_string("alloc msi interrupt fail\n");
        goto alloc_msi_interrupts_fail;
    }

    /* nvme doorbell only has one irq platform, this 'for loop' compatibles for mini v2 2p */
    for (func_index = 0; func_index < func_totl; func_index++) {
        agent_dev->msi_irq_base[func_index][0] = pdev->irq;
    }

    return 0;

alloc_msi_interrupts_fail:
    for (func_index = 0; func_index < func_totl; func_index++) {
        if (agent_dev->sdi_base[func_index] != NULL) {
            iounmap(agent_dev->sdi_base[func_index]);
        }
        agent_dev->sdi_base[func_index] = NULL;
        agent_dev->raise_int_base[func_index] = NULL;
    }

    return ret;
}

void agentdrv_nvme_proc_uinit(struct agentdrv_devctrl *agent_dev)
{
    u32 func_index;
    u32 func_totl = agentdrv_res_get_func_total();

    if (agent_dev->sdi_base[0] != NULL) {
        iounmap(agent_dev->sdi_base[0]);
    }

    for (func_index = 0; func_index < func_totl; func_index++) {
        agent_dev->sdi_base[func_index] = NULL;
        agent_dev->raise_int_base[func_index] = NULL;
    }

    agentdrv_release_msi_interrupts(agent_dev->sdi_pdev);
}

STATIC void agentdrv_alloc_soc_db_interrupt_callback(struct msi_desc *desc, struct msi_msg *msg)
{
}

STATIC void agentdrv_free_soc_db_interrupt(void *data)
{
    struct device *dev = data;
    /* platform_msi_domain_free_irqs stubbed for 6.x */
}

int agentdrv_alloc_soc_db_interrupt(struct agentdrv_devctrl *agent_dev, struct platform_device *pdev)
{
    /* MSI platform domain API not available in kernel 6.x */
    return -ENOTSUPP;
#if 0 /* disabled for 6.x */

    int irq[AGENTDRV_SOC_DB_IRQ_NUM] = {0};
    struct msi_desc *desc = NULL;
    u32 soc_doorbell_num;
    u32 pfvf_type;
    u32 irq_func_idx;
    int ret;
    u32 i;

    if (agent_dev->func_index >= MAX_AGENTFUNC_CNT) {
        devdrv_err("soc doorbell num invalid.\n");
        return -EINVAL;
    }

    if (!acpi_disabled) {
        ret = platform_msi_domain_alloc_irqs(&pdev->dev, AGENTDRV_SOC_DB_IRQ_NUM,
            agentdrv_alloc_soc_db_interrupt_callback);
        if (ret) {
            devdrv_err("Allocate soc doorbell Failed. (ret=%d)\n", ret);
            return ret;
        }
        for_each_msi_entry(desc, &pdev->dev) {
            if (desc->msi_index >= AGENTDRV_SOC_DB_IRQ_NUM) {
                break;
            }
            agent_dev->msi_irq_base[agent_dev->func_index][desc->msi_index] =
                desc->irq;
        }

        (void)devm_add_action(&pdev->dev, agentdrv_free_soc_db_interrupt, &pdev->dev);
        agent_dev->func_index++;
    } else {
        agentdrv_get_pdev_type_info(pdev, &pfvf_type, &irq_func_idx);

        if (pfvf_type == DEVDRV_PDEV_TYPE_PF_DOORBELL) {
            soc_doorbell_num = AGENTDRV_SOC_DB_IRQ_NUM;
        } else {
            soc_doorbell_num = AGENTDRV_VF_SOC_DB_IRQ_NUM;
        }
        for (i = 0; i < soc_doorbell_num; i++) {
            irq[i] = platform_get_irq(pdev, i);
            if (irq[i] < 0) {
                devdrv_err("Allocate soc doorbell Failed. (ret=%d; idx=%u)\n", irq[i], i);
                return irq[i];
            }
            agent_dev->msi_irq_base[agent_dev->func_index][i] = (u32)irq[i];
        }
        agent_dev->func_index++;
    }

    return 0;
#endif
}

STATIC int agentdrv_soc_doorbell_proc_init(struct agentdrv_devctrl *agent_dev, struct platform_device *pdev,
    u32 irq_func_idx)
{
    int ret;
    struct agent_res_sdi_addr sdi_addr = {0};
    u32 func_idx;
    u32 func_totl_num = agentdrv_res_get_func_total();

    agentdrv_res_sdi_addr(agent_dev->agent_id, irq_func_idx, &sdi_addr);

    /* get sdi base */
    agent_dev->sdi_base[irq_func_idx] = ioremap(sdi_addr.db_state_base, sdi_addr.db_state_size);
    if (agent_dev->sdi_base[irq_func_idx] == NULL) {
        devdrv_err("Ioremap sdi base failed. (dev_id=%u)\n", agent_dev->agent_id);
        agentdrv_dfx_put_string("ioremap sdi base fail\n");
        return -ENOMEM;
    }

    agent_dev->raise_int_base[irq_func_idx] = ioremap(sdi_addr.raise_int_base, sdi_addr.raise_int_size);
    if (agent_dev->raise_int_base[irq_func_idx] == NULL) {
        devdrv_err("Ioremap raise int base failed. (dev_id=%u)\n", agent_dev->agent_id);
        agentdrv_dfx_put_string("ioremap raise int base fail\n");
        ret = -ENOMEM;
        goto alloc_irq_base_fail;
    }

    ret = agentdrv_alloc_soc_db_interrupt(agent_dev, pdev);
    if (ret) {
        devdrv_err("Alloc soc db interrupt failed. (dev_id=%u)\n", agent_dev->agent_id);
        agentdrv_dfx_put_string("alloc soc db interrupt fail\n");
        goto alloc_irq_base_fail;
    }

    return 0;

alloc_irq_base_fail:
    for (func_idx = 0; func_idx < func_totl_num; func_idx++) {
        if (agent_dev->raise_int_base[func_idx] != NULL) {
            iounmap(agent_dev->raise_int_base[func_idx]);
            agent_dev->raise_int_base[func_idx] = NULL;
        }

        if (agent_dev->sdi_base[func_idx] != NULL) {
            iounmap(agent_dev->sdi_base[func_idx]);
            agent_dev->sdi_base[func_idx] = NULL;
        }
    }

    return ret;
}

void agentdrv_soc_doorbell_proc_uinit(struct agentdrv_devctrl *agent_dev)
{
    u32 func_idx;
    u32 func_totl_num = agentdrv_res_get_func_total();

    for (func_idx = 0; func_idx < func_totl_num; func_idx++) {
        if (agent_dev->sdi_base[func_idx] != NULL) {
            iounmap(agent_dev->sdi_base[func_idx]);
            agent_dev->sdi_base[func_idx] = NULL;
        }

        if (agent_dev->raise_int_base[func_idx] != NULL) {
            iounmap(agent_dev->raise_int_base[func_idx]);
            agent_dev->raise_int_base[func_idx] = NULL;
        }
    }
}

void agentdrv_sdi_proc_before_func_uninit(struct pci_dev *pdev, struct agentdrv_devctrl *agent_dev)
{
    agentdrv_uncfg_pdev(pdev);
    if (agentdrv_get_soc_doorbell_capability() == false) {
        agentdrv_nvme_proc_uinit(agent_dev);
    }
}

int agentdrv_sdi_db_hang_init(struct agentdrv_devctrl *agent_dev, u32 func_id)
{
    u32 soc_doorbell_num;
    u32 irq_vector = 0;
    int irq_num = 0;
    int ret = 0;

    /* when use nvme doorbell, 2P only use func0's nvme, so only register func0's irq */
    if ((agentdrv_get_soc_doorbell_capability() == false) && (func_id > 0)) {
        return 0;
    }

    if (devdrv_is_sriov_enabled(agent_dev->func_totl_num) && func_id > 0) {
        soc_doorbell_num = AGENTDRV_VF_SOC_DB_IRQ_NUM;
    } else {
        soc_doorbell_num = AGENTDRV_NVME_DB_IRQ_NUM;
    }

    for (irq_vector = 0; irq_vector < soc_doorbell_num; irq_vector++) {
        irq_num = devdrv_nvme_agent_irq_vector2num(irq_vector);
        ret = devdrv_register_irq_func(NULL, agent_dev->msi_irq_base[func_id][0] + irq_num, agentdrv_doorbell_interrupt,
            agent_dev, g_pcie_db_irq_name[agent_dev->agent_id]);
        if (ret) {
            devdrv_err("Register irq failed. (dev_id=%u, funcid=%u)\n", agent_dev->agent_id, func_id);
            return ret;
        }
        devdrv_set_nvme_irq_enbale(agent_dev->sdi_base[func_id], func_id, irq_vector);
    }

    return ret;
}

void agentdrv_sdi_db_hang_uninit(struct agentdrv_devctrl *agent_dev)
{
    u32 func_totl = agentdrv_res_get_func_total();
    int irq_vector = 0;
    int irq_num = 0;
    void *dummp_ptr = 0;
    u32 func_index;

    for (func_index = 0; func_index < func_totl; func_index++) {
        for (irq_vector = 0; irq_vector < AGENTDRV_NVME_DB_IRQ_NUM; irq_vector++) {
            irq_num = devdrv_nvme_agent_irq_vector2num(irq_vector);
            (void)devdrv_unregister_irq_func(dummp_ptr, agent_dev->msi_irq_base[func_index][0] + irq_num, agent_dev);
        }
    }
}

void agentdrv_sdi_mdc_p2p_init(struct agentdrv_devctrl *agent_dev)
{
    /* support mdc p2p, generate interrupt infomation for user process to use. */
    int i = 0;
    for (i = 0; i < AGENTDRV_GENERAL_INTERRUPT_NUM; i++) {
        init_waitqueue_head(&agent_dev->interrupt_info[i].wait);
        agent_dev->interrupt_info[i].status = AGENTDRV_DISABLE;
        agent_dev->interrupt_info[i].id = AGENTDRV_GENERAL_INTR_USED_DB_START * AGENTDRV_QUEUE_CNT + i;

        /* when use mdc p2p, msg_dev must be 1 chan. */
        agent_dev->interrupt_info[i].db_addr = (void *)((char *)agent_dev->p_agentdrv_msg_dev[0]->doorbell_base +
            AGENTDRV_MSG_CHAN_USED_DB_START + (u64)agent_dev->interrupt_info[i].id * AGENTDRV_SQ_CQ_QUEUE_OFFSET);
    }
}

int agentdrv_sdi_proc_after_func_init(struct agentdrv_devctrl *agent_dev)
{
    int ret;
    u32 chip_id;

    /* mdc  */
    (void)agentdrv_sdi_mdc_p2p_init(agent_dev);

    chip_id = agent_dev->agent_id;
    if (chip_id == 0) {
        ret = agentdrv_register_non_trans_msg_client(&g_common_msg_client);
        if (ret) {
            devdrv_err("Call register_non_trans_msg_client failed. (dev_id=%u; ret=%d)\n", chip_id, ret);
            agentdrv_dfx_put_string("register_non_trans_msg_client failed\n");
            goto sdi_db_hang_un;
        }
    }

    return 0;
sdi_db_hang_un:
    agentdrv_sdi_db_hang_uninit(agent_dev);
    return ret;
}

void agentdrv_sdi_proc_after_func_uninit(struct agentdrv_devctrl *agent_dev)
{
    u32 chip_id;

    chip_id = agent_dev->agent_id;
    if (chip_id == 0) {
        (void)agentdrv_unregister_non_trans_msg_client(&g_common_msg_client);
    }
    agentdrv_sdi_db_hang_uninit(agent_dev);
}

int agentdrv_msg_func_init(struct agentdrv_devctrl *agent_dev)
{
    u32 func_idx;
    int ret;

    /* msg func */
    agent_dev->func_totl_num = agentdrv_res_get_func_total();
    for (func_idx = 0; func_idx < agent_dev->func_totl_num; func_idx++) {
        ret = agentdrv_sdi_proc_func_init(agent_dev, func_idx);
        if (ret) {
            devdrv_err("Call sdi_proc_func_init failed. (dev_id=%d; func=%u; ret=%d)\n",
                agent_dev->agent_id, func_idx, ret);
            agentdrv_sdi_proc_func_uninit(func_idx, agent_dev);
            return ret;
        }

        ret = agentdrv_sdi_db_hang_init(agent_dev, func_idx);
        if (ret) {
            devdrv_err("Register irq failed. (dev_id=%d, func_idx=%u; ret=%d)\n",
                agent_dev->agent_id, func_idx, ret);
            return ret;
        }
    }

    ret = agentdrv_sdi_proc_after_func_init(agent_dev);
    if (ret) {
        agentdrv_sdi_proc_func_uninit(func_idx, agent_dev);
        devdrv_err("Call agentdrv_sdi_proc_after_func_init failed. (dev_id=%d; ret=%d)\n", agent_dev->agent_id, ret);
        return ret;
    }

    return 0;
}

void agentdrv_msg_func_uinit(struct agentdrv_devctrl *agent_dev)
{
    agentdrv_sdi_proc_after_func_uninit(agent_dev);
    agentdrv_sdi_proc_func_uninit(agent_dev->func_totl_num, agent_dev);
}

int agentdrv_pci_probe(struct pci_dev *pdev, const struct pci_device_id *data)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    int dev_id;
    int ret;

    /* Get device id and agent_dev */
    if (devdrv_get_devid_by_bus(pdev->bus->number, &dev_id) != 0) {
        devdrv_info("PCI probe bus not surport. (bus=%d)\n", pdev->bus->number);
        agentdrv_dfx_put_string("pci probe bus not surport\n");
        agentdrv_notify_blackbox_exception(0, AGENTDRV_SDIO_PROBE_FAIL, g_dfx_info.bbox_storage_str);
        return -EINVAL;
    }
    devdrv_info("PCI nvme driver match success, probe start. (dev_id=%d)\n", dev_id);

    agent_dev = agentdrv_get_dev(dev_id);
    agent_dev->agent_id = dev_id;

    pci_set_drvdata(pdev, agent_dev);
    agent_dev->sdi_pdev = pdev;

    ret = agentdrv_cfg_pdev(pdev);
    if (ret) {
        devdrv_err("Call agentdrv_cfg_pdev failed. (dev_id=%u)\n", agent_dev->agent_id);
        agentdrv_dfx_put_string("agentdrv_cfg_pdev fail\n");
        agentdrv_notify_blackbox_exception(0, AGENTDRV_SDIO_PROBE_FAIL, g_dfx_info.bbox_storage_str);
        return ret;
    }
    if (agentdrv_get_soc_doorbell_capability() == false) {
        ret = agentdrv_nvme_proc_init(agent_dev->sdi_pdev, agent_dev);
        if (ret) {
            agentdrv_uncfg_pdev(pdev);
            devdrv_err("Call agentdrv_nvme_proc_init failed. (dev_id=%u)\n", agent_dev->agent_id);
            agentdrv_dfx_put_string("Call agentdrv_nvme_proc_init failed\n");
            agentdrv_notify_blackbox_exception(0, AGENTDRV_SDIO_PROBE_FAIL, g_dfx_info.bbox_storage_str);
            return ret;
        }
        devdrv_info("Call agentdrv_nvme_proc_init success. (dev_id=%u)\n", agent_dev->agent_id);
    }
    /* --- last */
    devdrv_info("PCI probe success. (dev_id=%d)\n", dev_id);
    atomic_inc(&agent_dev->subdev_num);
    return 0;
}

void agentdrv_pci_remove(struct pci_dev *pdev)
{
    int dev_id;
    struct agentdrv_devctrl *agent_dev = NULL;

    if (devdrv_get_devid_by_bus(pdev->bus->number, &dev_id) != 0) {
        devdrv_info("PCI remove bus not surport. (bus=%d)\n", pdev->bus->number);
        return;
    }

    agent_dev = agentdrv_get_dev(dev_id);
    if (agent_dev == NULL) {
        devdrv_info("Can not get agentdrv_get_dev, agent_dev is NULL. (devid=%d)\n", dev_id);
        return;
    }
    if (agentdrv_get_soc_doorbell_capability() == false) {
        agentdrv_pci_unbind_irq(agent_dev);
    }
    agentdrv_msg_func_uinit(agent_dev);
    agentdrv_sdi_proc_before_func_uninit(pdev, agent_dev);

    return;
}

static char g_agentdrv_driver_name[] = "agentdrv_pci_device_driver";
static const struct pci_device_id g_agentdrv_tbl[] = {{ PCI_VDEVICE(HUAWEI, HISI_IEP_NVME_DEVICE_ID), 0 }, {}};
MODULE_DEVICE_TABLE(pci, g_agentdrv_tbl);
static struct pci_driver g_agentdrv_pci_driver = {
    .name = g_agentdrv_driver_name,
    .id_table = g_agentdrv_tbl,
    .probe = agentdrv_pci_probe,
    .remove = agentdrv_pci_remove,
    .driver = {
        .name = "agentdrv_pci_device_driver",
    },
};

int agentdrv_platform_probe(struct platform_device *pdev)
{
    struct resource *res = NULL;
    u32 res_idx = AGENTDRV_APB_REGION_BASE_INDEX;
    void __iomem *io_base = NULL;
    int dev_id, ret;
    struct agentdrv_devctrl *agent_dev = NULL;
    u32 type = 0;
    u32 irq_func_idx = 0;

    dev_id = agentdrv_platform_get_devid(pdev);
    if (dev_id < 0) {
        devdrv_err("Call agentdrv_platform_get_devid failed. (devid=%d)\n", dev_id);
        return -EINVAL;
    }

    devdrv_info("Platform driver match device success, probe start. (dev_id=%d)\n", dev_id);
    agent_dev = agentdrv_get_dev(dev_id);
    agent_dev->agent_id = dev_id;

    agentdrv_get_pdev_type_info(pdev, &type, &irq_func_idx);
    if ((type != DEVDRV_PDEV_TYPE_PLATFORM) && (agentdrv_get_soc_doorbell_capability() == true)) {
        /* Second parameter must use pdev, can not use agent_dev->apb_pdev */
        ret = agentdrv_soc_doorbell_proc_init(agent_dev, pdev, irq_func_idx);
        if (ret != 0) {
            devdrv_err("Call agentdrv_soc_doorbell_proc_init failed. (dev_id=%u)\n", agent_dev->agent_id);
            return -EINVAL;
        }
        devdrv_info("Call agentdrv_soc_doorbell_proc_init success. (dev_id=%u)\n", agent_dev->agent_id);
        devdrv_info("Platform type(%d) only for irq, probe success, return now\n", type);
        return 0;
    }

    agent_dev->apb_pdev = pdev;

    (void)agentdrv_get_ts_cpu_info(agent_dev);

    /* get apb address */
    res = platform_get_resource(pdev, IORESOURCE_MEM, res_idx);
    if (IS_ERR_OR_NULL(res)) {
        devdrv_err("Get resource mem failed. (dev_id=%d)\n", dev_id);
        agentdrv_dfx_put_string("get resource mem fail!\n");
        agentdrv_notify_blackbox_exception(dev_id, AGENTDRV_PM_PROBE_FAIL, g_dfx_info.bbox_storage_str);
        return -ENOMEM;
    }
    agent_dev->apb_base = devm_ioremap_resource(&pdev->dev, res);
    if (IS_ERR_OR_NULL(agent_dev->apb_base)) {
        devdrv_err("Fail to ioremap apb base. (dev_id=%d)\n", dev_id);
        agentdrv_dfx_put_string("fail to ioremap apb base!\n");
        agentdrv_notify_blackbox_exception(dev_id, AGENTDRV_PM_PROBE_FAIL, g_dfx_info.bbox_storage_str);
        return -ENOMEM;
    }

    /* pcie dma stash config */
    io_base = agent_dev->apb_base + AGENTDRV_AP_SDI_AXIM_REG;
    devdrv_dma_config_axim_aruser_mode(io_base);

    agent_dev->platform_dev = agentdrv_platform_init(agent_dev->apb_pdev, dev_id, agent_dev->apb_base);
    if (agent_dev->platform_dev == NULL) {
        devdrv_err("Agentdrv platform init failed. (dev_id=%d)\n", dev_id);
        ret = -ENOMEM;
        goto platform_init_fail;
    }

    atomic_inc(&agent_dev->subdev_num);
    devdrv_info("Platform driver probe success. (dev_id=%d)\n", dev_id);

    return 0;

platform_init_fail:
    if (agentdrv_get_soc_doorbell_capability() == true) {
        agentdrv_soc_doorbell_proc_uinit(agent_dev);
    }

    devm_iounmap(&pdev->dev, agent_dev->apb_base);
    agent_dev->apb_base = NULL;
    agentdrv_notify_blackbox_exception(dev_id, AGENTDRV_PM_PROBE_FAIL, g_dfx_info.bbox_storage_str);
    return ret;
}

void agentdrv_platform_remove(struct platform_device *pdev)
{
    struct agentdrv_devctrl *agent_dev = NULL;

    agent_dev = agentdrv_get_dev_by_apb_dev(pdev);
    if (agent_dev == NULL) {
        devdrv_info("agent_dev is NULL.\n");
        
    }
    agentdrv_platform_unbind_irq(agent_dev);

    if (agentdrv_get_soc_doorbell_capability() == true) {
        agentdrv_pci_unbind_irq(agent_dev);
        agentdrv_soc_doorbell_proc_uinit(agent_dev);
    }
    (void)agentdrv_platform_uninit(agent_dev->platform_dev, agent_dev->apb_pdev);

    devm_iounmap(&pdev->dev, agent_dev->apb_base);
    agent_dev->apb_base = NULL;
    
}

static const struct of_device_id pcie_of_match[] = {
    {
        .compatible = "hisi,hi1910-platform-device", /* mini v1, mini v2 */
    },
    {
        .compatible = "hisi,pcie-platform-device",
    },
    {},
};
MODULE_DEVICE_TABLE(of, pcie_of_match);

struct platform_driver agentdrv_platform_of_driver = {
    .probe = agentdrv_platform_probe,
    .remove = agentdrv_platform_remove,
#if (defined(CFG_FEATURE_PM) || defined(CFG_SOC_FEATURE_SR))
    .suspend = agentdrv_platform_suspend,
    .resume = agentdrv_platform_resume,
#endif
    .driver = {
        .name = "agentdrv_platform",
        .of_match_table = pcie_of_match,
    },
};

static const struct acpi_device_id pcie_acpi_match[] = {
    { "PCI0A00", 0 }, /* cloud v1, cloud v2 */
    {},
};
MODULE_DEVICE_TABLE(acpi, pcie_acpi_match);

struct platform_driver agentdrv_platform_acpi_driver = {
    .probe = agentdrv_platform_probe,
    .remove = agentdrv_platform_remove,
    .driver = {
        .name = "agentdrv_acpi_platform",
        .acpi_match_table = pcie_acpi_match,
    },
};

void agentdrv_devi_status_probed_proc(struct agentdrv_devctrl *agent_dev, u32 chip_type, u32 board_type, u32 dev_num)
{
    u32 func_idx = 0;
    struct devdrv_shr_para __iomem *shr_para = NULL;

    /* shr set */
    for (func_idx = 0; func_idx < agent_dev->func_totl_num; func_idx++) {
        shr_para = agent_dev->shr_para[func_idx];
        shr_para->chip_type = chip_type;
        if (chip_type == HISI_MINI_V1) {
            shr_para->board_type = board_type;
        }

        /* for host use */
        if (devdrv_get_pf_type() == DEVDRV_DAVINCI_DEV_NUM_1PF2P) {
            shr_para->total_func_num = 1;
        } else {
            shr_para->total_func_num = (int)agent_dev->func_totl_num;
        }

        shr_para->host_interrupt_flag = 0;

        /* when func_totl_num > AGENTDRV_1PF_OF_FUNC_TOTAL, is vf; Otherwise, is pf */
        if (devdrv_is_sriov_enabled(agent_dev->func_totl_num)) {
            shr_para->dev_num = (int)agent_dev->func_totl_num;
            shr_para->vf_id = func_idx;
        } else {
            shr_para->dev_num = (int)dev_num;
            shr_para->vf_id = 0;
        }

        if (devdrv_is_sriov_enabled(agent_dev->func_totl_num) == true) {
            /* sriov, pf enable default, vf not enable default */
            agent_dev->valid[0] = AGENTDRV_ENABLE;
        } else {
            /* 2die:1pcie 2pf, every die is enable default */
            agent_dev->valid[func_idx] = AGENTDRV_ENABLE;
        }
    }

    /* agent do */
    agent_dev->platform_dev->hot_reset_flag_addr = &(agent_dev->shr_para[0]->hot_reset_flag);
    agentdrv_bind_irq(agent_dev);
    agentdrv_set_host_phy_mach_flag(agent_dev->agent_id, 0x0);
}

STATIC void agentdrv_dev_send_int_to_host(struct agentdrv_devctrl *agent_dev)
{
    /* if support vf, only int msi-x to pf here */
    u32 func_totl_num = devdrv_is_sriov_enabled(agent_dev->func_totl_num) ? 1 : agent_dev->func_totl_num;
    u32 func_idx = 0;

    /* int to host */
    for (func_idx = 0; func_idx < func_totl_num; func_idx++) {
        if ((agent_dev->shr_para[func_idx]->host_interrupt_flag == 0) &&
            (agent_dev->shr_para[func_idx]->rc_msix_ready_flag == DEVDRV_MSIX_READY_FLAG)) {
            /* considering that when 1pf2p p1 send int to p0 pf0 */
            agentdrv_iocmd_raise_int_to_h(agent_dev->p_agentdrv_msg_dev[func_idx], 0x0);
        }
    }
}

STATIC u32 agentdrv_devi_check_host_flag(const struct agentdrv_devctrl *agent_dev)
{
    u32 func_idx = 0;
    u32 host_all_rdy_flag = 1;
    /* if support vf, only check pf here */
    u32 func_totl_num = devdrv_is_sriov_enabled(agent_dev->func_totl_num) ? 1 : agent_dev->func_totl_num;

    /* all flag is 1, then device rdy */
    for (func_idx = 0; func_idx < func_totl_num; func_idx++) {
        if (agent_dev->shr_para[func_idx]->host_interrupt_flag != 1) {
            host_all_rdy_flag = 0;
            break;
        }
        // here means host has received the msix, no need send again, init the rc_msix_ready_flag
        agent_dev->shr_para[func_idx]->rc_msix_ready_flag = 0;
    }
    return host_all_rdy_flag;
}

u32 agentdrv_is_host_msix_ready(const struct agentdrv_devctrl *agent_dev)
{
    u32 func_idx = 0;
    u32 host_msix_rdy_flag = 1;
    /* if support vf, only check pf here */
    u32 func_totl_num = devdrv_is_sriov_enabled(agent_dev->func_totl_num) ? 1 : agent_dev->func_totl_num;

    /* all flag is 1, then device rdy */
    for (func_idx = 0; func_idx < func_totl_num; func_idx++) {
        if (agent_dev->shr_para[func_idx]->rc_msix_ready_flag != DEVDRV_MSIX_READY_FLAG) {
            host_msix_rdy_flag = 0;
            break;
        }
    }
    return host_msix_rdy_flag;
}

STATIC int agentdrv_res_init_proc(struct agentdrv_devctrl *agent_dev)
{
    int ret;

    /* DMA dev init */
    ret = agentdrv_dma_dev_init(agent_dev, agent_dev->pdev);
    if (ret) {
        agentdrv_dfx_put_string("agentdrv_dma_dev_init failed\n");
        goto dma_init_fail;
    }

    /* msg func */
    ret = agentdrv_msg_func_init(agent_dev);
    if (ret) {
        devdrv_err("Call agentdrv_msg_func_init failed. (dev_id=%d; ret=%d)\n", agent_dev->agent_id, ret);
        agentdrv_dfx_put_string("agentdrv_msg_func_init failed\n");
        goto msg_func_init_fail;
    }
    return 0;

msg_func_init_fail:
    agentdrv_dma_proc_func_uninit(agent_dev->func_totl_num, agent_dev);
dma_init_fail:
    agentdrv_notify_blackbox_exception(agent_dev->agent_id, AGENTDRV_DMA_PROBE_FAIL, g_dfx_info.bbox_storage_str);
    return ret;
}

STATIC void agentdrv_dev_register_proc(struct agentdrv_devctrl *agent_dev)
{
    /* if support vf, only register pf here */
    u32 func_totl_num = devdrv_is_sriov_enabled(agent_dev->func_totl_num) ? 1 : agent_dev->func_totl_num;
    u32 func_index = 0;

    for (func_index = 0; func_index < func_totl_num; func_index++) {
        (void)agentdrv_dev_register(agent_dev, (int)func_index);
    }
}

static bool agentdrv_is_local_dev_ready(struct agentdrv_devctrl *agent_dev)
{
    /* if support vf, only check pf here */
    u32 func_totl_num = devdrv_is_sriov_enabled(agent_dev->func_totl_num) ? 1 : agent_dev->func_totl_num;
    u32 func_index;

    for (func_index = 0; func_index < func_totl_num; func_index++) {
        int dev_id;
        devdrv_chipfunc2dev(&dev_id, (int)agent_dev->agent_id, func_index);
        if (dev_status[dev_id].local_status != AGENTDRV_LOCAL_STATUS_ONLINE) {
            return false;
        }
    }

    return true;
}

static int agentdrv_dev_add(struct agentdrv_devctrl *agent_dev)
{
    /* if support vf, only add pf here */
    u32 func_totl_num = devdrv_is_sriov_enabled(agent_dev->func_totl_num) ? 1 : agent_dev->func_totl_num;
    int func_index;

    for (func_index = 0; func_index < (int)func_totl_num; func_index++) {
        struct uda_dev_type type;
        struct uda_dev_para para;
        u32 dev_id;
        int ret;

        devdrv_chipfunc2dev(&dev_id, (int)agent_dev->agent_id, func_index);

        uda_davinci_local_real_agent_type_pack(&type);
        uda_dev_para_pack(&para, dev_id, agent_dev->shr_para[func_index]->host_dev_id,
            uda_get_chip_type(dev_id), &agent_dev->pdev->dev);
        ret = uda_add_dev(&type, &para, &dev_id);
        if (ret != 0) {
            devdrv_err("Add fail. (ret=%d; dev_id=%u; agent_id=%u; func_index=%d)\n",
                ret, dev_id, agent_dev->agent_id, func_index);
            return ret;
        }
    }

    return 0;
}

static int agentdrv_dev_ctrl_notify(struct agentdrv_devctrl *agent_dev, enum uda_dev_ctrl_cmd cmd)
{
    u32 func_totl_num = devdrv_is_sriov_enabled(agent_dev->func_totl_num) ? 1 : agent_dev->func_totl_num;
    int func_index;

    for (func_index = 0; func_index < (int)func_totl_num; func_index++) {
        int dev_id;
        int ret;

        devdrv_chipfunc2dev(&dev_id, (int)agent_dev->agent_id, func_index);
        ret = uda_agent_dev_ctrl((u32)dev_id, cmd);
        if (ret != 0) {
            devdrv_err("ctrl fail. (ret=%d; dev_id=%u; agent_id=%u; func_index=%d, cmd = %d)\n",
                ret, dev_id, agent_dev->agent_id, func_index, cmd);
            return ret;
        }
    }
    return 0;
}

bool agentdrv_single_dev_init_proc(int dev_id, int chip_type, int board_type, int dev_num)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    u32 host_func_all_rdy_flag = 0;
    static u32 host_msix_ready_flag = 0;

    agent_dev = agentdrv_get_dev(dev_id);
    if (dev_status[dev_id].agent_status == AGENTDRV_STATUS_WAIT_DEV_PROBE ||
        dev_status[dev_id].agent_status == AGENTDRV_STATUS_WAIT_DEV_RESUME) {
        if (atomic_read(&agent_dev->subdev_num) >= AGENTDRV_SUBDEV_NUM) {
            devdrv_info("Device ID probe success. (dev_id=%d)\n", agent_dev->agent_id);
            if (agentdrv_res_init_proc(agent_dev) != 0) {
                return false;
            }
            agentdrv_atu_proc(agent_dev);
            mutex_init(&agent_dev->mutex);
            agentdrv_devi_status_probed_proc(agent_dev, (u32)chip_type, (u32)board_type, (u32)dev_num);
            devdrv_info("Device ID valid. (dev_id=%d)\n", agent_dev->agent_id);
            if (dev_status[dev_id].agent_status == AGENTDRV_STATUS_WAIT_DEV_PROBE) {
                dev_status[dev_id].agent_status = AGENTDRV_STATUS_WAIT_LOCAL_DEV_ADD;
                devdrv_info("Start status change to wait local dev add. (dev_id=%d)\n", agent_dev->agent_id);
            } else {
                dev_status[dev_id].agent_status = AGENTDRV_STATUS_WAIT_NOTIFY_RESUME;
                devdrv_info("Start status change to wait notify resume. (dev_id=%d)\n", agent_dev->agent_id);
            }
        }
    }

    if (dev_status[dev_id].agent_status == AGENTDRV_STATUS_WAIT_LOCAL_DEV_ADD ||
        dev_status[dev_id].agent_status == AGENTDRV_STATUS_WAIT_NOTIFY_RESUME) {
        if (agentdrv_is_local_dev_ready(agent_dev)) {
            devdrv_info("Host half probe handling, call init_instance.(dev_id=%d; agent_id=%u)\n",
                dev_id, agent_dev->agent_id);
            agentdrv_dev_register_proc(agent_dev);
            devdrv_info("Device ID init success. (dev_id=%d; agent_id=%u; agent_status=%d)\n",
                        dev_id, agent_dev->agent_id, dev_status[dev_id].agent_status);

            if (dev_status[dev_id].agent_status == AGENTDRV_STATUS_WAIT_LOCAL_DEV_ADD) {
                (void)agentdrv_dev_add(agent_dev);
            } else {
                (void)agentdrv_dev_ctrl_notify(agent_dev, UDA_CTRL_RESUME);
            }

            dev_status[dev_id].agent_status = AGENTDRV_STATUS_WAIT_HOST;
            host_msix_ready_flag = 0;
            devdrv_info("Start status change to wait host. (dev_id=%d)\n", agent_dev->agent_id);
        }
    }

    if (dev_status[dev_id].agent_status == AGENTDRV_STATUS_WAIT_HOST) {
        host_func_all_rdy_flag = agentdrv_devi_check_host_flag(agent_dev);
        if (host_func_all_rdy_flag == 1) {
            dev_status[dev_id].agent_status = AGENTDRV_STATUS_OK;
            devdrv_set_pcie_channel_status(AGENTDRV_PCIE_INIT_OK);
            devdrv_info("Start status change to ok. (dev_id=%d)\n", agent_dev->agent_id);
            return true;
        } else {
#ifdef CFG_SOC_PLATFORM_MDC_V11
            if (agentdrv_is_host_msix_ready(agent_dev) != 1) {
                devdrv_debug("host msix is not ready.\n");
                return false;
            }

            if (host_msix_ready_flag == 0) {
                /* Wait until the controller initialization is complete. Otherwise,
                 * an AA-IO fault occurs when an MSIX interrupt is sent to the RC. */
                msleep(AGENTDRV_SECOND_TO_MSECOND);
                host_msix_ready_flag = 1;
            }
#endif
            /* If host have not set half_probe_flag, then send int repeatly. */
            agentdrv_dev_send_int_to_host(agent_dev);
        }
    }

    return false;
}

static void agentdrv_dev_status_init(void)
{
    int i;

    for (i = 0; i < MAX_AGENTDEV_CNT; i++) {
        dev_status[i].local_status = AGENTDRV_LOCAL_STATUS_OFFLINE;
        dev_status[i].agent_status = AGENTDRV_STATUS_WAIT_DEV_PROBE;
    }
}

static int agentdrv_get_config_init_time(int chip_type)
{
    int dev_id = 0;
    int count = 5000;
    u32 init_time_config = AGENTDEV_WAIT_DEV_READY_TIME;
    struct agentdrv_devctrl *agent_dev = NULL;
    int ret;

#ifndef DRV_UT
    if (chip_type != HISI_MINI_V3) {
        return AGENTDEV_WAIT_DEV_READY_TIME;
    }

    agent_dev = agentdrv_get_dev(dev_id);
    while (true) {
        if (atomic_read(&agent_dev->subdev_num) >= AGENTDRV_SUBDEV_NUM) {
            atomic_set(&agent_dev->subdev_num, AGENTDRV_SUBDEV_NUM);
            break;
        }
        usleep_range(AGENTDRV_GET_INIT_TIME_DELAY, AGENTDRV_GET_INIT_TIME_DELAY + AGENTDRV_GET_INIT_TIME_RANGE);
        count--;
        if (count < 0) {
            devdrv_err("wait subdev_num timeout.\n");
            return AGENTDEV_WAIT_DEV_READY_TIME;
        }
    }
    ret = of_property_read_u32(agent_dev->apb_pdev->dev.of_node, "init_time", &init_time_config);
    if (ret != 0) {
        devdrv_info("can not read init_time by dts.\n");
    } else {
        init_time_config = init_time_config * AGENTDRV_SECOND_TO_MSECOND;
        devdrv_info("read init_time success. (time=%d(ms)).\n", init_time_config);
    }
    return (int)init_time_config;
#else
    return AGENTDEV_WAIT_DEV_READY_TIME;
#endif
}

void agentdrv_dev_init_proc(int chip_type, int board_type, int dev_num)
{
    int i;
    int ready_dev[MAX_AGENTDEV_CNT] = {0};
    int ready_dev_num = 0;
    int time = AGENTDEV_WAIT_DEV_READY_TIME;

    // mdc 11 support init time config
    time = agentdrv_get_config_init_time(chip_type);

    while ((ready_dev_num < dev_num) && (time > 0)) {
        for (i = 0; i < dev_num; i++) {
            if (ready_dev[i] == AGENTDEV_DEV_IS_INIT) {
                continue;
            }
            if (agentdrv_single_dev_init_proc(i, chip_type, board_type, dev_num) == true) {
                ready_dev[i] = AGENTDEV_DEV_IS_INIT;
                ready_dev_num++;
            }
        }

        usleep_range(AGENTDRV_USLEEP_2000, AGENTDRV_USLEEP_2000 + AGENTDRV_USLEEP_RANGE_10);
        time -= AGENTDRV_MSLEEP_2;
    }

    if (time <= 0) {
        devdrv_info("Wait init time out. \n");
    }
    devdrv_info("Device accept host return. (dev_num=%d; ready_dev_num=%d; time=%d)\n", dev_num, ready_dev_num, time);
}

void agentdrv_dev_init(struct work_struct *p_work)
{
    int chip_type;
    int board_type;
    int dev_num;

    (void)p_work;

    chip_type = devdrv_get_chip_type();
    board_type = devdrv_get_board_type();
    if ((chip_type >= HISI_CHIP_NUM) || (board_type > 0xFFFF)) {
        devdrv_err("Get chip type or get board type failed.\n");
    }

    dev_num = devdrv_get_dev_num();
    agentdrv_dev_init_proc(chip_type, board_type, dev_num);

    agentdrv_sysfs_comm_client_register();

    devdrv_info("Device init half finish.\n");
}

STATIC int agentdrv_register_ieps(void)
{
    int ret;

    ret = dfm_register_module(&g_agentdrv_ops_pub);
    if (ret) {
        devdrv_err("Register dfm module failed. (ret=%d)\n", ret);
        return -EINVAL;
    }

    ret = pci_register_driver(&g_agentdrv_pci_dma_driver);
    if (ret) {
        devdrv_err("Register PCI DMA agentdrv driver failed. (ret=%d)\n", ret);
        goto dma_register_failed;
    }

    ret = pci_register_driver(&g_agentdrv_pci_driver);
    if (ret) {
        devdrv_err("Register PCI agentdrv driver failed. (ret=%d)\n", ret);
        goto pci_register_failed;
    }

    ret = platform_driver_register(&agentdrv_platform_of_driver);
    if (ret) {
        devdrv_err("Register platform agentdrv driver failed. (ret=%d)\n", ret);
        goto platform_register_failed;
    }

    ret = platform_driver_register(&agentdrv_platform_acpi_driver);
    if (ret) {
        platform_driver_unregister(&agentdrv_platform_of_driver);
        devdrv_err("Register platform agentdrv driver failed. (ret=%d)\n", ret);
        goto platform_register_failed;
    }

    INIT_WORK(&g_agent_dev_init, agentdrv_dev_init);

    schedule_work(&g_agent_dev_init);

    return 0;
platform_register_failed:
    pci_unregister_driver(&g_agentdrv_pci_driver);
pci_register_failed:
    pci_unregister_driver(&g_agentdrv_pci_dma_driver);
dma_register_failed:
#ifdef CFG_SOC_PCIE_DFM
    (void)dfm_unregister_module(DFM_MODULE_ID_DRIVER, DFM_SUBMODULE_ID_PCIE);
#endif
    return ret;
}

void schedule_agent_dev_init(void)
{
    struct pci_dev *pdev = NULL;
    int ret, i;
    int wait_time = 0;

    pdev = pci_get_device(PCI_VENDOR_ID_HUAWEI, HISI_IEP_DMA_DEVICE_ID, pdev);
    if (pdev == NULL) {
        devdrv_err("Call pci_get_device failed.\n");
        return;
    }

    while (pdev->state_saved) {
        wait_time++;
        msleep(AGENTDRV_WAIT_STATE_STATUS_DELAY);
        if (wait_time > AGENTDRV_WAIT_STATE_STATUS_TIMEOUT) {
            devdrv_err("Wait dev state timeout.\n");
            break;
        }
    }

    ret = pci_load_and_free_saved_state(pdev, &pci_saved_state);
    if (ret != 0) {
        devdrv_err("Load and free saved state failed.\n");
        return;
    }
    pci_restore_state(pdev);

    ret = agentdrv_dma_probe(pdev, g_agentdrv_dma_tbl);
    if (ret != 0) {
        devdrv_info("Resume PCIe DMA driver fail.\n");
        return;
    }

    for (i = 0; i < MAX_AGENTDEV_CNT; i++) {
        dev_status[i].agent_status = AGENTDRV_STATUS_WAIT_DEV_RESUME;
    }

    schedule_work(&g_agent_dev_init);
}
EXPORT_SYMBOL(schedule_agent_dev_init);

STATIC void agentdrv_unregister_ieps(void)
{
    pci_unregister_driver(&g_agentdrv_pci_driver);
    pci_unregister_driver(&g_agentdrv_pci_dma_driver);

#ifdef CFG_SOC_PCIE_DFM
    (void)dfm_unregister_module(DFM_MODULE_ID_DRIVER, DFM_SUBMODULE_ID_PCIE);
#endif

    platform_driver_unregister(&agentdrv_platform_acpi_driver);
    platform_driver_unregister(&agentdrv_platform_of_driver);
}

STATIC void agentdrv_init_dev_num(void)
{
    int id_num = (int)(sizeof(g_agentdrv_dma_tbl) / sizeof(struct pci_device_id));

    devdrv_init_dev_num(g_agentdrv_dma_tbl, id_num);
}

STATIC int agentdrv_init(void)
{
    int ret;

    agentdrv_dev_status_init();
    agentdrv_init_dev_num();
    agentdrv_res_init_func_pf_num();
    agentdrv_res_init_func_total_num();
    agentdrv_res_set_slot_num();
    if ((agentdrv_unit_init() != 0) || (agentdrv_ctrl_init() != 0)) {
        devdrv_err("Call unit_init or ctrl_init failed.\n");
        return -EINVAL;
    }
    devdrv_tx_atu_init();
    agentdrv_init_common_msg();

    ret = agentdrv_dfx_init_struct();
    if (ret) {
        devdrv_err("DFX init failed. (ret=%d)\n", ret);
        return ret;
    }

    return 0;
}

STATIC void agentdrv_uninit(void)
{
    agentdrv_dfx_free_struct();
}

#define AGENTDRV_REAL_NOTIFIER "pcie"
#define AGENTDRV_VIRTUAL_NOTIFIER "pcie_virtual"
static int agentdrv_notifier_func(u32 udevid, enum uda_notified_action action)
{
    if (udevid >= MAX_AGENTDEV_CNT) {
        devdrv_err("Invalid para. (udevid=%u)\n", udevid);
        return -EINVAL;
    }

    if (action == UDA_INIT) {
        dev_status[udevid].local_status = AGENTDRV_LOCAL_STATUS_ONLINE;
    } else if (action == UDA_UNINIT) {
        devdrv_warn("Phy dev cannot be remove. (udevid=%u)\n", udevid);
    }

    return 0;
}

static int agentdrv_virtual_notifier_func(u32 udevid, enum uda_notified_action action)
{
    struct uda_dev_type type;

    if (udevid >= MAX_AGENTDEV_CNT) {
        return -EINVAL;
    }

    uda_dev_type_pack(&type, UDA_DAVINCI, UDA_AGENT, UDA_LOCAL, UDA_VIRTUAL);

    if (action == UDA_INIT) {
        struct uda_dev_para para;
        struct device *dev = NULL;
        int ret;

        if ((devdrv_get_chip_type() == HISI_CLOUD_V2) || (devdrv_get_chip_type() == HISI_MINI_V3)) {
            struct agentdrv_devctrl *agent_dev = NULL;
            u32 chip_id = 0;
            u32 func_id = 0;

            devdrv_dev2chipfunc(udevid, &chip_id, &func_id);
            agent_dev = agentdrv_get_dev(chip_id);
            dev = (agent_dev != NULL) ? &agent_dev->pdev->dev : NULL;
        }

        uda_dev_para_pack(&para, udevid, UDA_INVALID_UDEVID, uda_get_chip_type(udevid), dev);

        ret = uda_add_dev(&type, &para, &udevid);
        if (ret != 0) {
            devdrv_err("Add fail. (dev_id=%u; ret=%d)\n", udevid, ret);
            return ret;
        }
    } else if (action == UDA_UNINIT) {
        (void)uda_remove_dev(&type, udevid);
    } else {
        return 0;
    }

    return 0;
}

static int agentdrv_uda_notifier_init(void)
{
    struct uda_dev_type type;
    int ret;

    uda_davinci_local_real_entity_type_pack(&type);
    ret = uda_notifier_register(AGENTDRV_REAL_NOTIFIER, &type, UDA_PRI3, agentdrv_notifier_func);
    if (ret != 0) {
        devdrv_err("Register real notifier failed. (ret=%d)\n", ret);
        return ret;
    }

    uda_davinci_local_virtual_entity_type_pack(&type);
    ret = uda_notifier_register(AGENTDRV_VIRTUAL_NOTIFIER, &type, UDA_PRI3, agentdrv_virtual_notifier_func);
    if (ret != 0) {
        devdrv_err("Register virtual notifier failed. (ret=%d)\n", ret);
        return ret;
    }

    return 0;
}

static void agentdrv_uda_notifier_uninit(void)
{
    struct uda_dev_type type;

    uda_davinci_local_virtual_entity_type_pack(&type);
    (void)uda_notifier_unregister(AGENTDRV_VIRTUAL_NOTIFIER, &type);
    uda_davinci_local_real_entity_type_pack(&type);
    (void)uda_notifier_unregister(AGENTDRV_REAL_NOTIFIER, &type);
}

int __init agentdrv_init_module(void)
{
    int ret;
    u32 mode;

    devdrv_info("Insmod agent driver start. (driver_name=\"%s\")\n", g_agentdrv_driver_name);

    ret = agentdrv_get_rc_ep_mode(&mode);
    if (ret != 0) {
        devdrv_err("Fail to get rc_ep mode. (ret=%d)\n", ret);
    } else {
        (void)dbl_set_rc_ep_mode(mode);
    }

    ret = agentdrv_init();
    if (ret) {
        devdrv_err("Call agentdrv_init failed. (ret=%d)\n", ret);
        return -EINVAL;
    }

    agentdrv_init_multi_chip();

    ret = agentdrv_register_ieps();
    if (ret) {
        devdrv_err("Call agentdrv_register_ieps failed. (ret=%d)\n", ret);
        goto register_ieps_failed;
    }

    ret = agentdrv_uda_notifier_init();
    if (ret != 0) {
        goto uda_notifier_init_failed;
    }

    init_waitqueue_head(&g_suspend_status.ep_wait_queue);
    atomic_set(&g_suspend_status.ep_wait_status, 0);
    devdrv_info("Agentdrv module init success. (dev_num=%d)\n", devdrv_get_dev_num());

    return 0;

uda_notifier_init_failed:
    agentdrv_unregister_ieps();

register_ieps_failed:
    agentdrv_uninit();
    return ret;
}
module_init(agentdrv_init_module);

STATIC void __exit agentdrv_exit_module(void)
{
    agentdrv_uda_notifier_uninit();
    agentdrv_unregister_ieps();
    agentdrv_uninit();
    return;
}
module_exit(agentdrv_exit_module);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("AGENT_DEV slave driver");
