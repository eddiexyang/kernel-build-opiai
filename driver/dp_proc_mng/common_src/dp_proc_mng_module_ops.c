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
 * Create: 2022-08-13
 */
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/memory.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/list.h>
#include <linux/hashtable.h>
#include <linux/atomic.h>
#include <linux/nsproxy.h>
#include <linux/uaccess.h>

#include "dp_proc_mng_cmd.h"
#include "dp_proc_mng_log.h"
#include "dp_proc_mng_register_ops.h"
#include "dp_proc_mng_proc_info.h"

STATIC int dp_proc_mng_open(struct inode *inode, struct file *file)
{
    return 0;
}

STATIC int dp_proc_mng_release(struct inode *inode, struct file *filp)
{
    return 0;
}

STATIC int dp_proc_mng_convert_id_from_vir_to_phy(struct dp_proc_mng_ioctl_arg *buffer)
{
    int ret;
    u32 logic_id = DP_PROC_MNG_INVALID_DEVICE_PHYID;
    u32 vfid = DP_PROC_MNG_INVALID_DEVICE_PHYID;
    u32 phyid = DP_PROC_MNG_INVALID_DEVICE_PHYID;

    logic_id = buffer->head.logical_devid;

    ret = devdrv_manager_container_logical_id_to_physical_id(logic_id, &phyid, &vfid);
    if ((ret != 0) || (phyid >= DP_PROC_MNG_MAX_DEVICE_NUM) || (vfid >= DP_PROC_MNG_MAX_VF_NUM)) {
        dp_proc_mng_drv_err("Convert devid failed. (ret=%d; devid=%u; phyid=%u; vfid=%u)\n", ret, logic_id, phyid,
            vfid);
        return -EINVAL;
    }

    buffer->head.logical_devid = logic_id;
    buffer->head.devid = phyid;
    buffer->head.vfid = vfid;
    return 0;
}

STATIC int dp_proc_mng_dispatch_ioctl(struct file *file, u32 cmd, struct dp_proc_mng_ioctl_arg *buffer)
{
    int ret;
    u32 cmd_id = _IOC_NR(cmd);
    if (dp_proc_mng_ioctl_handlers[cmd_id] == NULL) {
        dp_proc_mng_drv_err("Cmd not support. (cmd=0x%x; cmd_id=0x%x)\n", cmd, cmd_id);
        return -EOPNOTSUPP;
    }

    ret = dp_proc_mng_convert_id_from_vir_to_phy(buffer);
    if (ret != 0) {
        return ret;
    }

    ret = dp_proc_mng_ioctl_handlers[_IOC_NR(cmd)](file, buffer);

    return ret;
}

STATIC long dp_proc_mng_ioctl(struct file *file, u32 cmd, unsigned long arg)
{
    struct dp_proc_mng_ioctl_arg buffer = {{0}};
    u32 cmd_id = _IOC_NR(cmd);
    int ret;

    if ((file == NULL) || (arg == 0)) {
        dp_proc_mng_drv_err("File is NULL, check dp_proc_mng init. (cmd=0x%x; _IOC_NR(cmd)=0x%x)\n", cmd, _IOC_NR(cmd));
        return -EINVAL;
    }

    if ((_IOC_TYPE(cmd) != DP_PROC_MNG_MAGIC) || (cmd_id >= DP_PROC_MNG_CMD_MAX_CMD)) {
        dp_proc_mng_drv_err("Cmd not support. (cmd=0x%x; cmd_id=0x%x)\n", cmd, cmd_id);
        return -EINVAL;
    }

    if ((_IOC_DIR(cmd) & _IOC_WRITE) != 0) {
        if (copy_from_user(&buffer, (void __user *)(uintptr_t)arg, sizeof(struct dp_proc_mng_ioctl_arg)) != 0) {
            dp_proc_mng_drv_err("Copy_from_user fail. (cmd=0x%x; cmd_id=0x%x)\n", cmd, cmd_id);
            return -EINVAL;
        }
    }

    ret = dp_proc_mng_dispatch_ioctl(file, cmd, &buffer);
    if (ret != 0) {
        return ret;
    }

    if ((_IOC_DIR(cmd) & _IOC_READ) != 0) {
        if (copy_to_user((void __user *)(uintptr_t)arg, &buffer, sizeof(struct dp_proc_mng_ioctl_arg)) != 0) {
            dp_proc_mng_drv_err("Copy_to_user fail. (cmd=0x%x; cmd_id=0x%x)\n", cmd, cmd_id);
            return -EINVAL;
        }
    }

    return 0;
}

STATIC struct file_operations dp_proc_mng_fops = {
    .owner = THIS_MODULE,
    .open = dp_proc_mng_open,
    .release = dp_proc_mng_release,
    .unlocked_ioctl = dp_proc_mng_ioctl,
};

STATIC int dp_proc_mng_init(void)
{
    int ret;

    dp_proc_mng_drv_info("Data plane proc mng module init.\n");

    ret = dp_proc_mng_info_init();
    if (ret != 0) {
        return ret;
    }

    ret = dp_proc_mng_create_work();
    if (ret != 0) {
        goto create_work_fail;
    }

    ret = dp_proc_mng_davinci_module_init(&dp_proc_mng_fops);
    if (ret != 0) {
        goto register_davinci_fail;
    }

    ret = dp_proc_mng_register_ops_init();
    if (ret != 0) {
        goto register_ops_fail;
    }

    dp_proc_mng_drv_info("Data plane proc mng module init success.\n");
    return 0;

register_ops_fail:
    dp_proc_mng_davinci_module_uninit();
register_davinci_fail:
    dp_proc_mng_destroy_work();
create_work_fail:
    dp_proc_mng_info_unint();

    return ret;
}

STATIC void dp_proc_mng_exit(void)
{
    dp_proc_mng_unregister_ops_init();
    dp_proc_mng_davinci_module_uninit();
    dp_proc_mng_destroy_work();
    dp_proc_mng_info_unint();
}

module_init(dp_proc_mng_init);
module_exit(dp_proc_mng_exit);
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Data plane process manage driver.");
