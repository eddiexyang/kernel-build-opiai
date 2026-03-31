/*
* Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
* Create: 2022-7-29
*/
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/pci.h>
#include "dbl/feature_loader.h"

#include "trs_pub_def.h"
#include "davinci_api.h"

#include "trs_core.h"
#include "trs_shr_id.h"
#include "trs_shr_id_node.h"

#define DEVDRV_DIVERSITY_PCIE_VENDOR_ID 0xFFFF
static const struct pci_device_id trs_shrid_tbl[] = {
    { PCI_VDEVICE(HUAWEI, 0xd100),           0 },
    { PCI_VDEVICE(HUAWEI, 0xd105),           0 },
    { PCI_VDEVICE(HUAWEI, 0xa126), 0 },
    { PCI_VDEVICE(HUAWEI, 0xd801),           0 },
    { PCI_VDEVICE(HUAWEI, 0xd500),           0 },
    { PCI_VDEVICE(HUAWEI, 0xd501),           0 },
    { PCI_VDEVICE(HUAWEI, 0xd802),           0 },
    { PCI_VDEVICE(HUAWEI, 0xd803),           0 },
    { PCI_VDEVICE(HUAWEI, 0xd804),           0 },
    { DEVDRV_DIVERSITY_PCIE_VENDOR_ID, 0xd500, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
    {}
};
MODULE_DEVICE_TABLE(pci, trs_shrid_tbl);

static int (*const shr_id_ioctl_handles[SHR_ID_MAX_CMD])(struct shr_id_proc_ctx *proc_ctx,
    struct shr_id_ioctl_info *arg) = {
    [_IOC_NR(SHR_ID_CREATE)] =  shr_id_create,
    [_IOC_NR(SHR_ID_OPEN)] =    shr_id_open,
    [_IOC_NR(SHR_ID_CLOSE)] =   shr_id_close,
    [_IOC_NR(SHR_ID_DESTROY)] = shr_id_destroy,
    [_IOC_NR(SHR_ID_SET_PID)] = shr_id_set_pid,
};

static int shr_id_file_open(struct inode *inode, struct file *file)
{
    struct shr_id_proc_ctx *proc_ctx = NULL;
    int ret;

    ret = shr_id_wait_for_proc_exit(current->tgid);
    if (ret != 0) {
        trs_err("Wait for proc exit fail. (ret=%d)\n", ret);
        return ret;
    }

    proc_ctx = shr_id_proc_create(current->tgid);
    if (proc_ctx == NULL) {
        trs_err("Open failed.\n");
        return -ENOMEM;
    }

    ret = shr_id_proc_add(proc_ctx);
    if (ret != 0) {
        shr_id_proc_destroy(proc_ctx);
        return ret;
    }
    file->private_data = proc_ctx;

    return 0;
}

static int shr_id_file_release(struct inode *inode, struct file *file)
{
    struct shr_id_proc_ctx *proc_ctx = file->private_data;

    if (proc_ctx == NULL) {
        trs_err("Not open.\n");
        return -EFAULT;
    }
    file->private_data = NULL;
    shr_id_proc_del(proc_ctx);
    return 0;
}

static long shr_id_file_ioctl(struct file *file, u32 cmd, unsigned long arg)
{
    struct shr_id_proc_ctx *proc_ctx = file->private_data;
    struct shr_id_ioctl_info ioctl_arg;
    u32 cmd_nr = _IOC_NR(cmd);
    int ret;

    if ((proc_ctx == NULL) || (arg == 0) || (cmd_nr < 0) || (cmd_nr >= SHR_ID_MAX_CMD) ||
        (shr_id_ioctl_handles[cmd_nr] == NULL)) {
        trs_err("Unsupported command. (cmd=%u; arg=0x%lx)\n", cmd_nr, arg);
        return -EINVAL;
    }

    if ((_IOC_DIR(cmd) & _IOC_WRITE) != 0) {
        if (copy_from_user(&ioctl_arg, (void *)(uintptr_t)arg, sizeof(struct shr_id_ioctl_info)) != 0) {
            trs_err("Copy from user fail. (cmd=%u)\n", cmd_nr);
            return -EFAULT;
        }
    }

    ret = shr_id_ioctl_handles[cmd_nr](proc_ctx, &ioctl_arg);
    if (ret != 0) {
        return ret;
    }

    if ((_IOC_DIR(cmd) & _IOC_READ) != 0) {
        if (copy_to_user((void *)(uintptr_t)arg, &ioctl_arg, sizeof(struct shr_id_ioctl_info)) != 0) {
            trs_err("Copy to user fail. (cmd=%u)\n", cmd_nr);
            return -EFAULT;
        }
    }

    return 0;
}

static struct file_operations shr_id_notify_fops = {
    .owner = THIS_MODULE,
    .open = shr_id_file_open,
    .release = shr_id_file_release,
    .unlocked_ioctl = shr_id_file_ioctl,
};

int __init shr_id_init_module(void)
{
    int ret;

    ret = drv_davinci_register_sub_module(DAVINCI_INTF_MODULE_TRS_SHR_ID, &shr_id_notify_fops);
    if (ret != 0) {
        trs_err("Register sub module fail. (ret=%d)\n", ret);
        return ret;
    }
    trs_res_share_proc_ops_register(TRS_NOTIFY, shr_id_is_belong_to_proc);
    shr_id_node_init();

    ret = module_feature_auto_init();
    if (ret != 0) {
        (void)drv_ascend_unregister_sub_module(DAVINCI_INTF_MODULE_TRS_SHR_ID);
        trs_err("Register event update fail. (ret=%d)\n", ret);
        return ret;
    }

    trs_debug("Register module success.\n");

    return 0;
}
module_init(shr_id_init_module);

void __exit shr_id_exit_module(void)
{
    module_feature_auto_uninit();
    (void)drv_ascend_unregister_sub_module(DAVINCI_INTF_MODULE_TRS_SHR_ID);
    trs_info("Unregister module success.\n");
}
module_exit(shr_id_exit_module);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("trs_shr_id driver");

