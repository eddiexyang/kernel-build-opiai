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
* Create: 2022-7-16
*/
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/fs.h>

#include "trs_pub_def.h"
#include "devdrv_manager_comm.h"
#include "davinci_api.h"

#include "cdqm_proc.h"
#include "cdqm_fops.h"

static int (*const cdqm_ioctl_handles[CDQM_MAX_CMD])(u32 devid, int pid, struct cdqm_ioctl_arg *arg) = {
    [_IOC_NR(CDQM_COMMAND)] = cdqm_ioctl_comm,
};

static long cdqm_ioctl(struct file *file, u32 cmd, unsigned long arg)
{
    struct cdqm_proc_ctx *proc_ctx = file->private_data;
    struct cdqm_ioctl_arg ioctl_arg;
    u32 cmd_nr = _IOC_NR(cmd);
    int ret;

    return -EOPNOTSUPP;
    if ((proc_ctx == NULL) || (arg == 0) || (cmd_nr < 0) || (cmd_nr >= CDQM_MAX_CMD) ||
        (cdqm_ioctl_handles[cmd_nr] == NULL)) {
        trs_err("Unsupported command. (cmd=%u; arg=0x%lx)\n", cmd, arg);
        return -EINVAL;
    }

    if ((_IOC_DIR(cmd) & _IOC_WRITE) != 0) {
        if (copy_from_user(&ioctl_arg, (void *)(uintptr_t)arg, sizeof(struct cdqm_ioctl_arg)) != 0) {
            trs_err("copy from user fail cmd(%u)\n", _IOC_NR(cmd));
            return -EFAULT;
        }
    }

    if (ioctl_arg.tsid >= proc_ctx->tsnum) {
        trs_err("invalid tsid(%u)\n", ioctl_arg.tsid);
        return -EINVAL;
    }

    ret = cdqm_ioctl_handles[_IOC_NR(cmd)](proc_ctx->devid, proc_ctx->pid, &ioctl_arg);
    if (ret != 0) {
        return ret;
    }
    if ((_IOC_DIR(cmd) & _IOC_READ) != 0) {
        if (copy_to_user((void *)(uintptr_t)arg, &ioctl_arg, sizeof(struct cdqm_ioctl_arg)) != 0) {
            trs_err("copy to user fail cmd(%u)\n", _IOC_NR(cmd));
            return -EFAULT;
        }
    }

    return 0;
}

/* 后续显示包含对应头文件 */
int devmng_get_vdavinci_info(u32 vdev_id, u32 *phy_id, u32 *vfid);
static int cdqm_open(struct inode *inode, struct file *file)
{
    struct cdqm_proc_ctx *proc_ctx = NULL;
    struct cdqm_dev_manage *cdq_dev = NULL;
    u32 vdevid = drv_davinci_get_device_id(file);
    u32 devid, fid;
    int ret;

    return -EOPNOTSUPP;
    if (vdevid >= TRS_DEV_MAX_NUM) {
        trs_err("Invalid para. (vdevid=%u)\n", vdevid);
        return -EINVAL;
    }

    ret = devdrv_manager_container_check_devid_in_container_ns(vdevid, current);
    if (ret != 0) {
        trs_err("Check device in container failed. (vdevid=%u)\n", vdevid);
        return -ENODEV;
    }

    ret = devmng_get_vdavinci_info(vdevid, &devid, &fid);
    if ((ret != 0) || (devid >= CDQM_MAX_DAVINCI_NUM)) {
        trs_err("Invalid para. (vdevid=%u; devid=%u; fid=%u)\n", vdevid, devid, fid);
        return -EINVAL;
    }

    cdq_dev = cdqm_get_cdq_dev(devid, 0);
    if (cdq_dev == NULL) {
        trs_err("Not register. (devid=%u)\n", devid);
        return -EFAULT;
    }

    mutex_lock(&cdq_dev->dev_mutex);
    proc_ctx = cdqm_proc_ctx_find(cdq_dev, current->tgid);
    if (proc_ctx != NULL) {
        mutex_unlock(&cdq_dev->dev_mutex);
        trs_err("Repeat open. (devid=%u; pid=%d)\n", devid, current->tgid);
        return -EEXIST;
    }

    /* 算力切分场景还需要适配 */
    proc_ctx = cdqm_proc_ctx_create(devid);
    if (proc_ctx == NULL) {
        mutex_unlock(&cdq_dev->dev_mutex);
        trs_err("Proc ctx create failed. (devid=%u)\n", devid);
        return -ENOMEM;
    }

    file->private_data = proc_ctx;
    cdqm_proc_ctx_add(cdq_dev, proc_ctx);
    mutex_unlock(&cdq_dev->dev_mutex);

    return 0;
}

static int cdqm_release(struct inode *inode, struct file *file)
{
    struct cdqm_proc_ctx *proc_ctx = file->private_data;
    struct cdqm_dev_manage *cdq_dev = NULL;
    u32 tsid;

    return -EOPNOTSUPP;
    if (proc_ctx == NULL) {
        trs_err("Not open.\n");
        return -EFAULT;
    }

    for (tsid = 0; tsid < proc_ctx->tsnum; tsid++) {
        (void)cdqm_recycle_cdq(proc_ctx->devid, tsid, proc_ctx->pid);
    }

    cdq_dev = cdqm_get_cdq_dev(proc_ctx->devid, 0);
    if (cdq_dev == NULL) {
        trs_err("Not register. (devid=%u)\n", proc_ctx->devid);
        return -EFAULT;
    }

    file->private_data = NULL;

    mutex_lock(&cdq_dev->dev_mutex);
    cdqm_proc_ctx_del(proc_ctx);
    mutex_unlock(&cdq_dev->dev_mutex);

    cdqm_proc_ctx_destroy(proc_ctx);

    return 0;
}

static struct file_operations cdqm_fops = {
    .owner = THIS_MODULE,
    .open = cdqm_open,
    .release = cdqm_release,
    .unlocked_ioctl = cdqm_ioctl,
};

int __init cdqm_init_module(void)
{
    int ret;

    ret = drv_davinci_register_sub_module(DAVINCI_INTF_MODULE_CDQM, &cdqm_fops);
    if (ret != 0) {
        trs_err("Register sub module fail. (ret=%d)\n", ret);
        return ret;
    }
    trs_info("Register cdqm module success.\n");

    return 0;
}
module_init(cdqm_init_module);

void __exit cdqm_exit_module(void)
{
    (void)drv_ascend_unregister_sub_module(DAVINCI_INTF_MODULE_CDQM);
    trs_info("Unregister cdqm module success.\n");
}
module_exit(cdqm_exit_module);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("cdqm driver");

