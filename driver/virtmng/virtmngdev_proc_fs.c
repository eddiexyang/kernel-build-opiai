/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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
 * Create: 2022-12-05
 */

#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/version.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>

#include "virtmngdev_pci.h"
#include "virtmngdev_res_common.h"
#include "virtmngdev_soc_resource.h"
#include "virtmngdev_res_mng.h"
#include "virtmngdev_client_online.h"
#include "virtmngdev_proc_fs.h"

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 5, 0)
#define STATIC_PROCFS_FILE_FUNC_OPS(ops, open_func, write_func)     \
    static const struct file_operations ops = {                     \
        .owner = THIS_MODULE,                                       \
        .open = open_func,                                          \
        .read = seq_read,                                           \
        .llseek = seq_lseek,                                        \
        .release = single_release,                                  \
        .write = write_func,                                        \
    }

#else
#define STATIC_PROCFS_FILE_FUNC_OPS(ops, open_func, write_func)     \
    static const struct proc_ops ops = {                            \
        .proc_open = open_func,                                     \
        .proc_read = seq_read,                                      \
        .proc_lseek = seq_lseek,                                    \
        .proc_release = single_release,                             \
        .proc_write = write_func,                                   \
    }
#endif

// file directory : /proc/vmng_dev
static struct proc_dir_entry *vmng_dev_entry = NULL;

static struct vmngd_user_input g_device_id = {
    .dev_id = 0,
    .vfid = 1,
    .vfg_id = 0,
};

static struct vmngd_procfs_entry g_procfs_entry = {
    .dev_id = NULL,
    .vf_id = NULL,
    .vfg_id = NULL,
    .total_resource_info = NULL,
    .remain_resource_info = NULL,
    .each_resource_info = NULL,
    .vfg_resource_info = NULL,
    .vdev_ctrl_info = NULL,
    .all_resource_info = NULL,
};

STATIC void resource_info_proc_show(struct seq_file *m, void *v, vmngd_resource_unit_t *vf_cfg)
{
    vmngd_vf_cfg_info_t *cfg_info = &vf_cfg->cfg_info;
    vmngd_vf_cfg_ac_t *accelerator = &vf_cfg->accelerator;
    vmngd_vf_cfg_dvpp_t *dvpp = &vf_cfg->dvpp;
    vmngd_vf_cfg_cpu_t *cpu = &vf_cfg->cpu;

    seq_printf(m, "[cfg_info]:\n");
    seq_printf(m, "\tdev_id =               %u\n", cfg_info->dev_id);
    seq_printf(m, "\tdtype =                %u\n", cfg_info->dtype);
    seq_printf(m, "\ttoken =                %#llx\n", cfg_info->token);
    seq_printf(m, "\ttoken_max =            %#llx\n", cfg_info->token_max);
    seq_printf(m, "\ttask_timeout =         %#llx\n", cfg_info->task_timeout);
    seq_printf(m, "[accelerator]:\n");
    seq_printf(m, "\tacsq =                 %#lx\n", accelerator->acsq.bitmap);
    seq_printf(m, "\taic =                  %#lx\n", accelerator->aic.bitmap);
    seq_printf(m, "\taiv =                  %#lx\n", accelerator->aiv.bitmap);
    seq_printf(m, "\tc_core =               %#lx\n", accelerator->c_core.bitmap);
    seq_printf(m, "\tcdqm =                 %#lx\n", accelerator->cdqm.bitmap);
    seq_printf(m, "\tcmo_id =               %#lx\n", accelerator->cmo_id.bitmap);
    seq_printf(m, "\tdsa =                  %#lx\n", accelerator->dsa.bitmap);
    seq_printf(m, "\tevent_id =             %#lx\n", accelerator->event_id.bitmap);
    seq_printf(m, "\tffts =                 %#lx\n", accelerator->ffts.bitmap);
    seq_printf(m, "\tnotify_id =            %#lx\n", accelerator->notify_id.bitmap);
    seq_printf(m, "\tpcie_dma =             %#lx\n", accelerator->pcie_dma.bitmap);
    seq_printf(m, "\trtsq =                 %#lx\n", accelerator->rtsq.bitmap);
    seq_printf(m, "\tsdma =                 %#lx\n", accelerator->sdma.bitmap);
    seq_printf(m, "[dvpp]:\n");
    seq_printf(m, "\tjpegd =                %#lx\n", dvpp->jpegd.bitmap);
    seq_printf(m, "\tjpege =                %#lx\n", dvpp->jpege.bitmap);
    seq_printf(m, "\tpngd =                 %#lx\n", dvpp->pngd.bitmap);
    seq_printf(m, "\tvdec =                 %#lx\n", dvpp->vdec.bitmap);
    seq_printf(m, "\tvenc =                 %#lx\n", dvpp->venc.bitmap);
    seq_printf(m, "\tvpc =                  %#lx\n", dvpp->vpc.bitmap);
    seq_printf(m, "[cpu]:\n");
    seq_printf(m, "\tdevice_aicpu =         %#lx\n", cpu->device_aicpu.bitmap);
    seq_printf(m, "\thost_aicpu =           %#lx\n", cpu->host_aicpu.bitmap);
    seq_printf(m, "\thost_ctrl_cpu =        %#lx\n", cpu->host_ctrl_cpu.bitmap);
    seq_printf(m, "\ttopic_aicpu_slot =     %#lx\n", cpu->topic_aicpu_slot.bitmap);
    seq_printf(m, "\ttopic_ctrl_cpu_slot =  %#lx\n", cpu->topic_ctrl_cpu_slot.bitmap);

    return;
}

#define MSG_SIZE 3
#define KSTRTOL_BASE 10
STATIC ssize_t set_global_id(struct file *filp, const char *buf, size_t count, loff_t *offp, u32 *output)
{
    char *msg;
    int ret = 0;
    long temp = 0;

    if (count > MSG_SIZE) {
        ret = -EINVAL;
        vmng_err("Set_global_id count size err!\n");
        goto error;
    }

    msg = kzalloc(MSG_SIZE, GFP_KERNEL | __GFP_ACCOUNT);
    if (msg == NULL) {
        ret = -ENOMEM;
        vmng_err("Set_global_id kzalloc err!\n");
        goto error;
    }

    if (copy_from_user(msg, buf, count)) {
        ret = -EFAULT;
        vmng_err("Set_global_id copy_from_user err!\n");
        goto free;
    }
    ret = kstrtol(msg, KSTRTOL_BASE, &temp);
    if ((ret != 0) || (temp < 0) || (temp >= INT_MAX)) {
        ret = -EINVAL;
        vmng_err("Set_global_id kstrtol err!\n");
        goto free;
    }

    WRITE_ONCE(*output, (u32)temp);
    ret = count;

free:
    kfree(msg);
error:
    return ret;
}

STATIC ssize_t write_dev_id(struct file *filp, const char *buf, size_t count, loff_t *offp)
{
    u32 dev_id = 0;
    int ret = 0;
    ret = set_global_id(filp, buf, count, offp, &dev_id);

    if (dev_id >= VMNGD_SURPORT_MAX_DEV) {
        ret = -ERANGE;
    } else {
        g_device_id.dev_id = dev_id;
    }

    return ret;
}

STATIC int read_dev_id_proc_show(struct seq_file *m, void *v)
{
    seq_printf(m, "[ dev_id = %u ]\n", g_device_id.dev_id);
    return 0;
}

STATIC int read_dev_id_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, read_dev_id_proc_show, NULL);
}

STATIC ssize_t write_vfid(struct file *filp, const char *buf, size_t count, loff_t *offp)
{
    u32 vfid = 0;
    int ret = 0;
    ret = set_global_id(filp, buf, count, offp, &vfid);

    if ((vfid > BASE_VF_NUM) || (vfid == 0)) {
        ret = -ERANGE;
    } else {
        g_device_id.vfid = vfid;
    }

    return ret;
}

STATIC int read_vfid_proc_show(struct seq_file *m, void *v)
{
    seq_printf(m, "[ vfid = %u ]\n", g_device_id.vfid);
    return 0;
}

STATIC int read_vfid_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, read_vfid_proc_show, NULL);
}

STATIC ssize_t write_vfg_id(struct file *filp, const char *buf, size_t count, loff_t *offp)
{
    u32 vfg_id = 0;
    int ret = 0;
    ret = set_global_id(filp, buf, count, offp, &vfg_id);

    if (vfg_id >= BASE_VFG_NUM) {
        ret = -ERANGE;
    } else {
        g_device_id.vfg_id = vfg_id;
    }

    return ret;
}

STATIC int read_vfg_id_proc_show(struct seq_file *m, void *v)
{
    seq_printf(m, "[ vfg_id = %u ]\n", g_device_id.vfg_id);
    return 0;
}

STATIC int read_vfg_id_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, read_vfg_id_proc_show, NULL);
}

STATIC int total_resource_info_proc_show(struct seq_file *m, void *v)
{
    vmngd_resource_info_t *resource_info = NULL;
    vmngd_resource_unit_t *total = NULL;
    u32 dev_id = g_device_id.dev_id;

    if (dev_id >= VMNGD_SURPORT_MAX_DEV) {
        seq_printf(m, "Please set legal dev id before cat ops!\n");
        seq_printf(m, "VMNGD_SURPORT_MAX_DEV:%u\n", VMNGD_SURPORT_MAX_DEV);
        return 0;
    }

    resource_info = vmngd_get_resource_info(dev_id);
    if (resource_info == NULL) {
        seq_printf(m, "No such device, dev_id = %u\n", dev_id);
        return 0;
    }
    total = &resource_info->vf_cfg_total;
    seq_printf(m, "----------- dev_id = %d -----------\n", dev_id);
    resource_info_proc_show(m, v, total);
    return 0;
}

STATIC int total_resource_info_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, total_resource_info_proc_show, NULL);
}

STATIC int remain_resource_info_proc_show(struct seq_file *m, void *v)
{
    vmngd_resource_info_t *resource_info = NULL;
    vmngd_resource_unit_t *remain = NULL;
    u32 dev_id = g_device_id.dev_id;

    if (dev_id >= VMNGD_SURPORT_MAX_DEV) {
        seq_printf(m, "Please set legal dev id before cat ops!\n");
        seq_printf(m, "VMNGD_SURPORT_MAX_DEV:%u\n", VMNGD_SURPORT_MAX_DEV);
        return 0;
    }

    resource_info = vmngd_get_resource_info(dev_id);
    if (resource_info == NULL) {
        seq_printf(m, "No such device, dev_id = %u\n", dev_id);
        return 0;
    }

    remain = &resource_info->vf_cfg_remain;
    seq_printf(m, "----------- dev_id = %d -----------\n", dev_id);
    resource_info_proc_show(m, v, remain);
    return 0;
}

STATIC int remain_resource_info_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, remain_resource_info_proc_show, NULL);
}

STATIC int each_resource_info_proc_show(struct seq_file *m, void *v)
{
    vmngd_resource_info_t *resource_info = NULL;
    vmngd_resource_unit_t *each = NULL;
    u32 dev_id = g_device_id.dev_id;
    u32 vfid = g_device_id.vfid;

    if ((dev_id >= VMNGD_SURPORT_MAX_DEV) || (vfid > BASE_VF_NUM) ||
        (vfid == 0)) {
        seq_printf(m, "Please set legal dev id and vfid before cat ops!\n");
        seq_printf(m, "VMNGD_SURPORT_MAX_DEV:%u\nBASE_VF_NUM:%u\n", VMNGD_SURPORT_MAX_DEV, BASE_VF_NUM);
        return 0;
    }
    resource_info = vmngd_get_resource_info(dev_id);
    if (resource_info == NULL) {
        seq_printf(m, "No such device, dev_id = %u\n", dev_id);
        return 0;
    }

    each = &resource_info->vf_cfg_each[vfid - 1];

    seq_printf(m, "\n----------- dev_id = %d / vfid = %d -----------\n", dev_id, vfid);
    resource_info_proc_show(m, v, each);
    return 0;
}

STATIC int each_resource_info_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, each_resource_info_proc_show, NULL);
}

STATIC int vfg_resource_info_proc_show(struct seq_file *m, void *v)
{
    vmngd_resource_info_t *resource_info = NULL;
    vmngd_vfg_cfg_info_t *vfg = NULL;
    u32 dev_id = g_device_id.dev_id;
    u32 vfg_id = g_device_id.vfg_id;

    if ((dev_id >= VMNGD_SURPORT_MAX_DEV) || (vfg_id >= BASE_VFG_NUM)) {
        seq_printf(m, "Please set legal dev id and vfg id before cat ops!\n");
        seq_printf(m, "VMNGD_SURPORT_MAX_DEV:%u\nBASE_VFG_NUM:%u\n", VMNGD_SURPORT_MAX_DEV, BASE_VFG_NUM);
        return 0;
    }

    resource_info = vmngd_get_resource_info(dev_id);
    if (resource_info == NULL) {
        seq_printf(m, "No such device, dev_id = %u\n", dev_id);
        return 0;
    }

    vfg = &resource_info->vfg_cfg_each[vfg_id];
    seq_printf(m, "----------- dev_id = %d / vfg_id = %d -----------\n", dev_id, vfg_id);
    seq_printf(m, "[base]:\n");
    seq_printf(m, "status =                 %u\n", vfg->status);
    seq_printf(m, "vfg_id =                 %u\n", vfg->vfg_id);
    seq_printf(m, "vfg_type =               %u\n", vfg->vfg_type);
    seq_printf(m, "vf_bitmap =              %#lx\n", vfg->vf_bitmap);
    seq_printf(m, "[accelerator]:\n");
    seq_printf(m, "\taiv =                  %#lx\n", vfg->accelerator.aiv.bitmap);
    seq_printf(m, "\taic =                  %#lx\n", vfg->accelerator.aic.bitmap);
    seq_printf(m, "\tc_core =               %#lx\n", vfg->accelerator.c_core.bitmap);
    seq_printf(m, "\tdsa =                  %#lx\n", vfg->accelerator.dsa.bitmap);
    seq_printf(m, "\tffts =                 %#lx\n", vfg->accelerator.ffts.bitmap);
    seq_printf(m, "\tsdma =                 %#lx\n", vfg->accelerator.sdma.bitmap);
    seq_printf(m, "\tpcie_dma =             %#lx\n", vfg->accelerator.pcie_dma.bitmap);
    seq_printf(m, "\tacsq =                 %#lx\n", vfg->accelerator.acsq.bitmap);
    seq_printf(m, "\trtsq =                 %#lx\n", vfg->accelerator.rtsq.bitmap);
    seq_printf(m, "\tevent_id =             %#lx\n", vfg->accelerator.event_id.bitmap);
    seq_printf(m, "\tnotify_id =            %#lx\n", vfg->accelerator.notify_id.bitmap);
    seq_printf(m, "\tcdqm =                 %#lx\n", vfg->accelerator.cdqm.bitmap);
    seq_printf(m, "\tcmo_id =               %#lx\n", vfg->accelerator.cmo_id.bitmap);
    seq_printf(m, "[dvpp]:\n");
    seq_printf(m, "\tjpegd =                %#lx\n", vfg->dvpp.jpegd.bitmap);
    seq_printf(m, "\tjpege =                %#lx\n", vfg->dvpp.jpege.bitmap);
    seq_printf(m, "\tvpc =                  %#lx\n", vfg->dvpp.vpc.bitmap);
    seq_printf(m, "\tvdec =                 %#lx\n", vfg->dvpp.vdec.bitmap);
    seq_printf(m, "\tpngd =                 %#lx\n", vfg->dvpp.pngd.bitmap);
    seq_printf(m, "\tvenc =                 %#lx\n", vfg->dvpp.venc.bitmap);
    seq_printf(m, "[cpu]:\n");
    seq_printf(m, "\ttopic_aicpu_slot =     %#lx\n", vfg->cpu.topic_aicpu_slot.bitmap);
    seq_printf(m, "\ttopic_ctrl_cpu_slot =  %#lx\n", vfg->cpu.topic_ctrl_cpu_slot.bitmap);
    seq_printf(m, "\thost_ctrl_cpu =        %#lx\n", vfg->cpu.host_ctrl_cpu.bitmap);
    seq_printf(m, "\tdevice_aicpu =         %#lx\n", vfg->cpu.device_aicpu.bitmap);
    seq_printf(m, "\thost_aicpu =           %#lx\n", vfg->cpu.host_aicpu.bitmap);

    return 0;
}

STATIC int vfg_resource_info_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, vfg_resource_info_proc_show, NULL);
}

STATIC int vdev_ctrl_info_proc_show(struct seq_file *m, void *v)
{
    struct vmng_vdev_ctrl *vdev_ctrl = NULL;
    u32 dev_id = g_device_id.dev_id;
    u32 vfid = g_device_id.vfid;

    if ((dev_id >= VMNGD_SURPORT_MAX_DEV) || (vfid > BASE_VF_NUM) ||
        (vfid == 0)) {
        seq_printf(m, "Please set legal dev id and vfid before cat ops!\n");
        seq_printf(m, "VMNGD_SURPORT_MAX_DEV:%u\nBASE_VF_NUM:%u\n", VMNGD_SURPORT_MAX_DEV, BASE_VF_NUM);
        return 0;
    }

    vdev_ctrl = vmngd_get_ctrl(dev_id, vfid);
    if (vdev_ctrl == NULL) {
        return 0;
    }
    seq_printf(m, "----------- dev_id = %d / vfid = %d -----------\n", dev_id, vfid);
    seq_printf(m, "[base]:\n");
    seq_printf(m, "\tcapbility =            %u\n", vdev_ctrl->vf_cfg.capbility);
    seq_printf(m, "[id]:\n");
    seq_printf(m, "\tvf_id =                %u\n", vdev_ctrl->vf_cfg.id.vf_id);
    seq_printf(m, "\tvfg_mode =             %u\n", vdev_ctrl->vf_cfg.id.vfg_mode);
    seq_printf(m, "\tvfg_id =               %u\n", vdev_ctrl->vf_cfg.id.vfg_id);
    seq_printf(m, "\tvip =                  %u\n", vdev_ctrl->vf_cfg.id.vip);
    seq_printf(m, "\ttoken =                %#llx\n", vdev_ctrl->vf_cfg.id.token);
    seq_printf(m, "\ttoken_max =            %#llx\n", vdev_ctrl->vf_cfg.id.token_max);
    seq_printf(m, "\ttask_timeout =         %#llx\n", vdev_ctrl->vf_cfg.id.task_timeout);
    seq_printf(m, "[accelerator]:\n");
    seq_printf(m, "\taiv =                  %#llx\n", vdev_ctrl->vf_cfg.accelerator.aiv_bitmap);
    seq_printf(m, "\taic =                  %#x\n", vdev_ctrl->vf_cfg.accelerator.aic_bitmap);
    seq_printf(m, "\tc_core =               %#x\n", vdev_ctrl->vf_cfg.accelerator.c_core_bitmap);
    seq_printf(m, "\tdsa =                  %#x\n", vdev_ctrl->vf_cfg.accelerator.dsa_bitmap);
    seq_printf(m, "\tffts =                 %#x\n", vdev_ctrl->vf_cfg.accelerator.ffts_bitmap);
    seq_printf(m, "\tsdma =                 %#x\n", vdev_ctrl->vf_cfg.accelerator.sdma_bitmap);
    seq_printf(m, "\tpcie_dma =             %#x\n", vdev_ctrl->vf_cfg.accelerator.pcie_dma_bitmap);
    seq_printf(m, "\tacsq_slice =           %#x\n", vdev_ctrl->vf_cfg.accelerator.acsq_slice_bitmap);
    seq_printf(m, "\trtsq_slice =           %#x\n", vdev_ctrl->vf_cfg.accelerator.rtsq_slice_bitmap);
    seq_printf(m, "\tevent_slice =          %#x\n", vdev_ctrl->vf_cfg.accelerator.event_slice_bitmap);
    seq_printf(m, "\tnotify_slice =         %#x\n", vdev_ctrl->vf_cfg.accelerator.notify_slice_bitmap);
    seq_printf(m, "\tcdq_slice =            %#x\n", vdev_ctrl->vf_cfg.accelerator.cdq_slice_bitmap);
    seq_printf(m, "\tcmo_slice =            %#x\n", vdev_ctrl->vf_cfg.accelerator.cmo_slice_bitmap);
    seq_printf(m, "[cpu]:\n");
    seq_printf(m, "\ttopic_aicpu_slot =     %#x\n", vdev_ctrl->vf_cfg.cpu.topic_aicpu_slot_bitmap);
    seq_printf(m, "\ttopic_ctrl_cpu_slot =  %#x\n", vdev_ctrl->vf_cfg.cpu.topic_ctrl_cpu_slot_bitmap);
    seq_printf(m, "\thost_ctrl_cpu =        %#x\n", vdev_ctrl->vf_cfg.cpu.host_ctrl_cpu_bitmap);
    seq_printf(m, "\tdevice_aicpu =         %#x\n", vdev_ctrl->vf_cfg.cpu.device_aicpu_bitmap);
    seq_printf(m, "\thost_aicpu =           %#llx\n", vdev_ctrl->vf_cfg.cpu.host_aicpu_bitmap);
    seq_printf(m, "[dvpp]:\n");
    seq_printf(m, "\tjpegd =                %#x\n", vdev_ctrl->vf_cfg.dvpp.jpegd_bitmap);
    seq_printf(m, "\tjpege =                %#x\n", vdev_ctrl->vf_cfg.dvpp.jpege_bitmap);
    seq_printf(m, "\tvpc =                  %#x\n", vdev_ctrl->vf_cfg.dvpp.vpc_bitmap);
    seq_printf(m, "\tvdec =                 %#x\n", vdev_ctrl->vf_cfg.dvpp.vdec_bitmap);
    seq_printf(m, "\tpngd =                 %#x\n", vdev_ctrl->vf_cfg.dvpp.pngd_bitmap);
    seq_printf(m, "\tvenc =                 %#x\n", vdev_ctrl->vf_cfg.dvpp.venc_bitmap);

    return 0;
}

STATIC int vdev_ctrl_info_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, vdev_ctrl_info_proc_show, NULL);
}

STATIC void show_resource_bitmap(struct seq_file *m, void *v, vmng_resource_list resource)
{
    seq_printf(m, "\t[accelerator]:\n");
    seq_printf(m, "\taic:               %#lx\n", get_res_bitmap(&resource, MIA_AC_AIC));
    seq_printf(m, "\taiv:               %#lx\n", get_res_bitmap(&resource, MIA_AC_AIV));
    seq_printf(m, "\tc_core:            %#lx\n", get_res_bitmap(&resource, MIA_AC_C_CORE));
    seq_printf(m, "\tdsa:               %#lx\n", get_res_bitmap(&resource, MIA_AC_DSA));
    seq_printf(m, "\tffts:              %#lx\n", get_res_bitmap(&resource, MIA_AC_FFTS));
    seq_printf(m, "\tsdma:              %#lx\n", get_res_bitmap(&resource, MIA_AC_SDMA));
    seq_printf(m, "\tpcie_dma:          %#lx\n", get_res_bitmap(&resource, MIA_AC_PCIE_DMA));
    seq_printf(m, "\tacsq:              %#lx\n", get_res_bitmap(&resource, MIA_STARS_ACSQ));
    seq_printf(m, "\trtsq:              %#lx\n", get_res_bitmap(&resource, MIA_STARS_RTSQ));
    seq_printf(m, "\tstream_id:         %#lx\n", get_res_bitmap(&resource, MIA_STARS_STREAM));
    seq_printf(m, "\tevent_id:          %#lx\n", get_res_bitmap(&resource, MIA_STARS_EVENT));
    seq_printf(m, "\tnotify_id:         %#lx\n", get_res_bitmap(&resource, MIA_STARS_NOTIFY));
    seq_printf(m, "\tmodel_id:          %#lx\n", get_res_bitmap(&resource, MIA_STARS_MODEL));
    seq_printf(m, "\tcmo_id:            %#lx\n", get_res_bitmap(&resource, MIA_STARS_CMO));
    seq_printf(m, "\tcdqm:              %#lx\n", get_res_bitmap(&resource, MIA_STARS_CDQ));
    seq_printf(m, "\tnuma:              %#lx\n", get_res_bitmap(&resource, MIA_MEM_NUMA));
    seq_printf(m, "\tmemory:            %#lx\n", get_res_bitmap(&resource, MIA_SYS_MEM));
    seq_printf(m, "\t[cpu]:\n");
    seq_printf(m, "\ttopic_aicpu:       %#lx\n", get_res_bitmap(&resource, MIA_STARS_TOPIC_ACPU_SLOT));
    seq_printf(m, "\ttopic_ctrl_cpu:    %#lx\n", get_res_bitmap(&resource, MIA_STARS_TOPIC_CCPU_SLOT));
    seq_printf(m, "\thost_ctrl_cpu:     %#lx\n", get_res_bitmap(&resource, MIA_CPU_HOST_CCPU));
    seq_printf(m, "\tdev_aicpu:         %#lx\n", get_res_bitmap(&resource, MIA_CPU_DEV_ACPU));
    seq_printf(m, "\thost_aicpu:        %#lx\n", get_res_bitmap(&resource, MIA_CPU_HOST_ACPU));
    seq_printf(m, "\t[dvpp]:\n");
    seq_printf(m, "\tjpegd:             %#lx\n", get_res_bitmap(&resource, MIA_DVPP_JPEGD));
    seq_printf(m, "\tjpege:             %#lx\n", get_res_bitmap(&resource, MIA_DVPP_JPEGE));
    seq_printf(m, "\tvpc:               %#lx\n", get_res_bitmap(&resource, MIA_DVPP_VPC));
    seq_printf(m, "\tvdec:              %#lx\n", get_res_bitmap(&resource, MIA_DVPP_VDEC));
    seq_printf(m, "\tpngd:              %#lx\n", get_res_bitmap(&resource, MIA_DVPP_PNGD));
    seq_printf(m, "\tvenc:              %#lx\n", get_res_bitmap(&resource, MIA_DVPP_VENC));

    return;
}

STATIC void show_pf_resource_info(struct seq_file *m, void *v, u32 dev_id)
{
    struct vmng_pf_object *pf_node = vmngd_get_pf_object(dev_id);

    if (pf_node == NULL) {
        seq_printf(m, "Show pf resource info, get pf_node failed.\n");
        return;
    }
    seq_printf(m, "[PF total_res]\n");
    seq_printf(m, "\tdev_id:            %u\n", pf_node->dev_id);
    seq_printf(m, "\tchip_type:         %u\n", pf_node->chip_type);
    show_resource_bitmap(m, v, pf_node->total_res);
    seq_printf(m, "[PF remain_res]\n");
    show_resource_bitmap(m, v, pf_node->remain_res);

    return;
}

STATIC void show_vf_resource_info(struct seq_file *m, void *v, u32 dev_id, u32 vfid)
{
    struct vmng_vf_object *vf_node = NULL;

    vf_node = vmngd_get_vf_object(dev_id, vfid);
    if (vf_node == NULL) {
        seq_printf(m, "Show vf resource info, get vf_node failed.\n");
        return;
    }

    seq_printf(m, "[VF resource]\n");
    seq_printf(m, "\tdev_id:            %u\n", vf_node->dev_id);
    seq_printf(m, "\tvfid:              %u\n", vf_node->vfid);
    seq_printf(m, "\tstatus:            %u\n", vf_node->status);
    seq_printf(m, "\tdtype:             %u\n", vf_node->dtype);
    seq_printf(m, "\ttoken:             %#llx\n", vf_node->token);
    seq_printf(m, "\ttoken_max:         %#llx\n", vf_node->token_max);
    seq_printf(m, "\ttask_timeout:      %#llx\n", vf_node->task_timeout);

    show_resource_bitmap(m, v, vf_node->resource);

    return;
}

STATIC void show_vfg_resource_info(struct seq_file *m, void *v, u32 dev_id, u32 vfg_id)
{
    struct vmng_pf_object *pf_node = NULL;
    struct vmng_vfg_object *vfg_node = NULL;

    pf_node = vmngd_get_pf_object(dev_id);
    if (pf_node == NULL) {
        seq_printf(m, "Show vfg resource info, get pf_node failed.\n");
        return;
    }

    vfg_node = vmngd_get_vfg_object(pf_node, vfg_id);
    if (vfg_node == NULL) {
        seq_printf(m, "Show vfg resource info, get vfg_node failed.\n");
        return;
    }

    seq_printf(m, "[VFG resource]\n");
    seq_printf(m, "\tdev_id:            %u\n", vfg_node->dev_id);
    seq_printf(m, "\tvfg_id:            %u\n", vfg_node->vfg_id);
    seq_printf(m, "\tvfg_mode:          %u\n", vfg_node->vfg_mode);

    show_resource_bitmap(m, v, vfg_node->resource);

    return;
}

STATIC int all_resource_info_proc_show(struct seq_file *m, void *v)
{
    u32 dev_id = g_device_id.dev_id;
    u32 vfid = g_device_id.vfid;
    u32 vfg_id = g_device_id.vfg_id;

    if ((dev_id >= VMNGD_SURPORT_MAX_DEV) || (vfid > BASE_VF_NUM) ||
        (vfid == 0)) {
        seq_printf(m, "Please set legal dev id and vfid before cat ops!\n");
        seq_printf(m, "VMNGD_SURPORT_MAX_DEV:%u\nBASE_VF_NUM:%u\n", VMNGD_SURPORT_MAX_DEV, BASE_VF_NUM);
        return 0;
    }

    seq_printf(m, "\n----------- dev_id = %d / vfid = %d / vfg_id = %d -----------\n", dev_id, vfid, vfg_id);

    show_pf_resource_info(m, v, dev_id);
    show_vf_resource_info(m, v, dev_id, vfid);
    show_vfg_resource_info(m, v, dev_id, vfg_id);

    return 0;
}

STATIC int all_resource_info_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, all_resource_info_proc_show, NULL);
}

STATIC_PROCFS_FILE_FUNC_OPS(vmngd_dev_id_ops, read_dev_id_proc_open, write_dev_id);
STATIC_PROCFS_FILE_FUNC_OPS(vmngd_vfid_ops, read_vfid_proc_open, write_vfid);
STATIC_PROCFS_FILE_FUNC_OPS(vmngd_vfg_id_ops, read_vfg_id_proc_open, write_vfg_id);
STATIC_PROCFS_FILE_FUNC_OPS(vmngd_total_resource_ops, total_resource_info_proc_open, NULL);
STATIC_PROCFS_FILE_FUNC_OPS(vmngd_remain_resource_ops, remain_resource_info_proc_open, NULL);
STATIC_PROCFS_FILE_FUNC_OPS(vmngd_each_resource_ops, each_resource_info_proc_open, NULL);
STATIC_PROCFS_FILE_FUNC_OPS(vmngd_vfg_resource_ops, vfg_resource_info_proc_open, NULL);
STATIC_PROCFS_FILE_FUNC_OPS(vmngd_vdev_ctrl_ops, vdev_ctrl_info_proc_open, NULL);
STATIC_PROCFS_FILE_FUNC_OPS(vmngd_all_resource_ops, all_resource_info_proc_open, NULL);


STATIC int vmngd_dev_id_proc_fs(void)
{
    g_procfs_entry.dev_id = proc_create_data("dev_id", S_IRUSR | S_IWUSR, vmng_dev_entry, &vmngd_dev_id_ops, NULL);

    if (g_procfs_entry.dev_id == NULL) {
        vmng_err("Create dev_id_entry dir failed.\n");
        return VMNG_ERR;
    }

    return VMNG_OK;
}

STATIC int vmngd_vf_id_proc_fs(void)
{
    g_procfs_entry.vf_id = proc_create_data("vf_id", S_IRUSR | S_IWUSR, vmng_dev_entry, &vmngd_vfid_ops, NULL);

    if (g_procfs_entry.vf_id == NULL) {
        vmng_err("Create vf_id_entry dir failed.\n");
        return VMNG_ERR;
    }

    return VMNG_OK;
}

STATIC int vmngd_vfg_id_proc_fs(void)
{
    g_procfs_entry.vfg_id = proc_create_data("vfg_id", S_IRUSR | S_IWUSR, vmng_dev_entry, &vmngd_vfg_id_ops, NULL);

    if (g_procfs_entry.vfg_id == NULL) {
        vmng_err("Create vfg_id_entry dir failed.\n");
        return VMNG_ERR;
    }

    return VMNG_OK;
}

STATIC int vmngd_total_resource_info_proc_fs(void)
{
    g_procfs_entry.total_resource_info = proc_create_data("total_resource_info", S_IRUSR,
        vmng_dev_entry, &vmngd_total_resource_ops, NULL);

    if (g_procfs_entry.total_resource_info == NULL) {
        vmng_err("Create total_resource_info_entry dir failed.\n");
        return VMNG_ERR;
    }

    return VMNG_OK;
}

STATIC int vmngd_remain_resource_info_proc_fs(void)
{
    g_procfs_entry.remain_resource_info = proc_create_data("remain_resource_info", S_IRUSR,
        vmng_dev_entry, &vmngd_remain_resource_ops, NULL);

    if (g_procfs_entry.remain_resource_info == NULL) {
        vmng_err("Create remain_resource_info_entry dir failed.\n");
        return VMNG_ERR;
    }

    return VMNG_OK;
}

STATIC int vmngd_each_resource_info_proc_fs(void)
{
    g_procfs_entry.each_resource_info = proc_create_data("each_resource_info", S_IRUSR,
        vmng_dev_entry, &vmngd_each_resource_ops, NULL);

    if (g_procfs_entry.each_resource_info == NULL) {
        vmng_err("Create each_resource_info_entry dir failed.\n");
        return VMNG_ERR;
    }

    return VMNG_OK;
}

STATIC int vmngd_vfg_resource_info_proc_fs(void)
{
    g_procfs_entry.vfg_resource_info = proc_create_data("vfg_resource_info", S_IRUSR,
        vmng_dev_entry, &vmngd_vfg_resource_ops, NULL);

    if (g_procfs_entry.vfg_resource_info == NULL) {
        vmng_err("Create vfg_resource_info_entry dir failed.\n");
        return VMNG_ERR;
    }

    return VMNG_OK;
}

STATIC int vmngd_vdev_ctrl_info_proc_fs(void)
{
    g_procfs_entry.vdev_ctrl_info = proc_create_data("vdev_ctrl_info", S_IRUSR,
        vmng_dev_entry, &vmngd_vdev_ctrl_ops, NULL);

    if (g_procfs_entry.vdev_ctrl_info == NULL) {
        vmng_err("Create vdev_ctrl_info_entry dir failed.\n");
        return VMNG_ERR;
    }

    return VMNG_OK;
}

STATIC int vmngd_all_resource_info_proc_fs(void)
{
    g_procfs_entry.all_resource_info = proc_create_data("all_resource_info", S_IRUSR,
        vmng_dev_entry, &vmngd_all_resource_ops, NULL);

    if (g_procfs_entry.all_resource_info == NULL) {
        vmng_err("Create all_resource_info_entry dir failed.\n");
        return VMNG_ERR;
    }

    return VMNG_OK;
}

int vmngd_proc_fs_init(void)
{
    vmng_dev_entry = proc_mkdir("vmng_dev", NULL);
    if (vmng_dev_entry == NULL) {
        vmng_err("Create vmng_dev entry dir failed\n");
        return VMNG_ERR;
    }

    if ((vmngd_dev_id_proc_fs() != 0) ||
        (vmngd_vf_id_proc_fs() != 0) ||
        (vmngd_vfg_id_proc_fs() != 0) ||
        (vmngd_total_resource_info_proc_fs() != 0) ||
        (vmngd_remain_resource_info_proc_fs() != 0) ||
        (vmngd_each_resource_info_proc_fs() != 0) ||
        (vmngd_vfg_resource_info_proc_fs() != 0) ||
        (vmngd_vdev_ctrl_info_proc_fs() != 0) ||
        (vmngd_all_resource_info_proc_fs() != 0)) {
        (void)remove_proc_subtree("vmng_dev", NULL);
        vmng_err("Vmng_dev proc fs init failed.\n");
        return VMNG_ERR;
    }
    return VMNG_OK;
}

void vmngd_proc_fs_uninit(void)
{
    (void)remove_proc_subtree("vmng_dev", NULL);
}