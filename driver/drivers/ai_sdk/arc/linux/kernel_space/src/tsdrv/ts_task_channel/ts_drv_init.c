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
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/opiai_vendor_compat.h>
#include <linux/securec.h>

#ifndef AOS_LLVM_BUILD
#include <linux/securectype.h>
#endif
#include <linux/of.h>
#include <linux/io.h>
#include <linux/of_irq.h>
#ifndef AOS_LLVM_BUILD
#include <linux/uio_driver.h>
#endif
#include <linux/acpi.h>
#include <linux/ioctl.h>

#ifndef TSDRV_UT
#include "ts_drv_init.h"
#include "ts_drv_cmd.h"
#include "hwts_drv_api.h"
#include "ts_aisle_api.h"
#include "ts_api.h"
#include "ts_drv_sysfs.h"
#include "task_struct.h"
#ifdef CFG_SOC_PLATFORM_MDC_V51
#include "tsdrv_ts_node.h"
#endif
#include "tsdrv_interface.h"
struct drv_hwts_ctrl *g_drv_ctrl_hwts[CHIP_NUM_MAX] = {0};

/* used for c-dev */
struct ts_char_dev {
    struct device   *device;
    struct class    *dev_class;
    struct cdev     cdev;
    dev_t           devno;
};

struct ts_char_dev g_char_dev;
STATIC int g_chip_num = 0;

static int copy_from_user_safe(void *to, const void __user *from, unsigned long n)
{
    if ((from == NULL) || (n == 0)) {
        ts_drv_err("user pointer is NULL, from=%pK, n=%#llx.\n", (void *)(uintptr_t)from, (u64)n);
        return TS_INNER_ERR;
    }

    if (copy_from_user(to, (void *)from, n) != 0) {
        ts_drv_err("copy_from_user failed size %lu.\n", n);
        return TS_INNER_ERR;
    }

    return 0;
}

STATIC int ts_fop_send_msg_to_ts_async(struct ts_context *context, unsigned long arg)
{
    struct ts_ioctl_para para_info;

    char msg[TS_IPC_MSG_MAX_LEN * sizeof(u32)];

    if (copy_from_user_safe(&para_info, (void *)(uintptr_t)arg,
        sizeof(struct ts_ioctl_para)) != 0) {
        return TS_COPY_USER_ERR;
    }

    if (para_info.u.aicpu_msg.msg == NULL) {
        ts_drv_err("invalid aicpu_msg.msg\n");
        return TS_PARA_ERR;
    }

    if (para_info.u.aicpu_msg.msg_len > ((TS_IPC_MSG_MAX_LEN * sizeof(u32)) - MSG_HEAD_LEN)) {
        ts_drv_err("msg len error, msg_len:%u, out range:%lu\n",
            para_info.u.aicpu_msg.msg_len, ((TS_IPC_MSG_MAX_LEN * sizeof(u32)) - MSG_HEAD_LEN));
        return TS_PARA_ERR;
    }

    if (copy_from_user_safe(msg, (void *)(uintptr_t)para_info.u.aicpu_msg.msg, para_info.u.aicpu_msg.msg_len) != 0) {
        ts_drv_err("copy msg from user error\n");
        return TS_COPY_USER_ERR;
    }

    return send_msg_to_ts_async(para_info.dev_id, para_info.ts_id, para_info.u.aicpu_msg.msg_len, msg);
}

STATIC int ts_fop_aicpu_record_msg(struct ts_context *context, unsigned long arg)
{
    struct ts_ioctl_para para_info;

    if (copy_from_user_safe(&para_info, (void *)(uintptr_t)arg, sizeof(struct ts_ioctl_para)) != 0) {
        return TS_COPY_USER_ERR;
    }

    if ((para_info.u.aicpu_record.record_type != AICPU_MSG_NOTIFY_RECORD) &&
        (para_info.u.aicpu_record.record_type != AICPU_MSG_EVENT_RECORD)) {
        ts_drv_err("devid %u, tsid %u, record type %u invalid.\n", para_info.dev_id, para_info.ts_id,
            para_info.u.aicpu_record.record_type);
        return TS_PARA_ERR;
    }

    return ts_write_record_register(para_info.dev_id, para_info.ts_id, para_info.u.aicpu_record.record_type,
        para_info.u.aicpu_record.record_id);
}

STATIC int ts_fop_open(struct inode *inode, struct file *filep)
{
    struct ts_context *context = NULL;
    int node_id = iminor(inode);
    struct drv_hwts_ctrl *hwts = g_drv_ctrl_hwts[node_id];

    mutex_lock(&hwts->mutex);
    context = filep->private_data;
    if (context != NULL) {
        mutex_unlock(&hwts->mutex);
        ts_drv_info("proc pid(%d), thread pid(%d):opened.\n", current->tgid, current->pid);
        return TS_INNER_ERR;
    }

    context = (struct ts_context *)kmalloc(sizeof(struct ts_context), GFP_KERNEL | __GFP_ACCOUNT);
    if (context == NULL) {
        mutex_unlock(&hwts->mutex);
        return TS_INNER_ERR;
    }
    context->ts_id = -1;

    filep->private_data = context;
    mutex_unlock(&hwts->mutex);

    return 0;
}

STATIC int ts_fop_release(struct inode *inode, struct file *filep)
{
    struct ts_context *context = NULL;

    if (filep == NULL) {
        ts_drv_err("Release file error, filep is null.\n");
        return TS_INNER_ERR;
    }

    context = filep->private_data;
    if (context == NULL) {
        ts_drv_err("Release file error, context is null.\n");
        return TS_INNER_ERR;
    }

    kfree(filep->private_data);
    filep->private_data = NULL;
    return 0;
}

STATIC long ts_fop_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
    static int (*const ts_ioctl_handler[TS_CMD_MAX_NR])(struct ts_context *context, unsigned long arg) = {
        [_IOC_NR(TS_SEND_MSG_TO_TS_ASYNC_ID)] = ts_fop_send_msg_to_ts_async,
        [_IOC_NR(AICPU_RECORD_MSG_ID)] = ts_fop_aicpu_record_msg,
    };
    struct ts_context *context = filep->private_data;

    if (context == NULL) {
        ts_drv_err("conext is NULL\n");
        return -EINVAL;
    }

    if (_IOC_NR(cmd) >= TS_CMD_MAX_NR) {
        ts_drv_err("invalid cmd, cmd - %u\n", _IOC_NR(cmd));
        return TS_INNER_ERR;
    }
    if (ts_ioctl_handler[_IOC_NR(cmd)] == NULL) {
        ts_drv_err("invalid cmd, cmd - %u\n", _IOC_NR(cmd));
        return TS_INNER_ERR;
    }

    if (arg == 0) {
        ts_drv_err("Invalid arg.\n");
        return TS_PARA_ERR;
    }
    return ts_ioctl_handler[_IOC_NR(cmd)](context, arg);
}

char *ts_devnode(const struct device *dev, umode_t *mode)
{
    return NULL;
}

static const struct file_operations ts_fops = {
    .owner          = THIS_MODULE,
    .open           = ts_fop_open,
    .release        = ts_fop_release,
#ifndef AOS_LLVM_BUILD
    .unlocked_ioctl = ts_fop_ioctl,
#else
    .ioctl = ts_fop_ioctl,
#endif
};
#ifndef AOS_LLVM_BUILD
STATIC int ts_drv_register_cdev(void)
{
#if !defined(CFG_SOC_PLATFORM_CLOUD_V2) && !defined(CFG_SOC_PLATFORM_MINIV3)
    unsigned int ts_drv_major;
#endif
    int ret;

    ret = alloc_chrdev_region(&g_char_dev.devno, 0, MINOR_DEV_COUNT, TS_CHAR_DEV_NAME);
    if (ret < 0) {
        ts_drv_err("alloc_chrdev_region error, ret = %d.\n", ret);
        return -EINVAL;
    }

    cdev_init(&g_char_dev.cdev, &ts_fops);
    g_char_dev.cdev.owner = THIS_MODULE;

    if (cdev_add(&g_char_dev.cdev, g_char_dev.devno, 1)) {
        ts_drv_err("cdev_add error, devno = %u.\n", g_char_dev.devno);
        goto unregister_chrdev_region;
    }
    g_char_dev.dev_class = class_create(THIS_MODULE, TS_CHAR_DEV_NAME);
    if (g_char_dev.dev_class == NULL) {
        ts_drv_err("class_create error.\n");
        goto cdev_del;
    }
    g_char_dev.dev_class->devnode = ts_devnode;
    g_char_dev.device = device_create(g_char_dev.dev_class, NULL, g_char_dev.devno, NULL, TS_CHAR_DEV_NAME);
    if (g_char_dev.device == NULL) {
        ts_drv_err("device create error.\n");
        goto class_destroy;
    }

#if !defined(CFG_SOC_PLATFORM_CLOUD_V2) && !defined(CFG_SOC_PLATFORM_MINIV3)
    ret = sysfs_create_group(&g_char_dev.device->kobj, ts_drv_get_aisle_attr_group());
    if (ret != 0) {
        ts_drv_err("sysfs create failed, ret = %d\n", ret);
        goto device_destroy;
    }
#endif

    ts_drv_debug("register cdev success.\n");
    return 0;

#if !defined(CFG_SOC_PLATFORM_CLOUD_V2) && !defined(CFG_SOC_PLATFORM_MINIV3)
device_destroy:
    ts_drv_major = MAJOR(g_char_dev.devno);
    device_destroy(g_char_dev.dev_class, MKDEV(ts_drv_major, 0));
#endif
class_destroy:
    class_destroy(g_char_dev.dev_class);
cdev_del:
    cdev_del(&g_char_dev.cdev);
unregister_chrdev_region:
    unregister_chrdev_region(g_char_dev.devno, MINOR_DEV_COUNT);
    return -EINVAL;
}
#endif
STATIC void ts_unregister_cdev(void)
{
#ifndef AOS_LLVM_BUILD
    if (g_char_dev.device != NULL) {
#if !defined(CFG_SOC_PLATFORM_CLOUD_V2) && !defined(CFG_SOC_PLATFORM_MINIV3)
        sysfs_remove_group(&g_char_dev.device->kobj, ts_drv_get_aisle_attr_group());
#endif
        device_destroy(g_char_dev.dev_class, g_char_dev.devno);
        class_destroy(g_char_dev.dev_class);
        cdev_del(&g_char_dev.cdev);
        unregister_chrdev_region(g_char_dev.devno, MINOR_DEV_COUNT);
        g_char_dev.device = NULL;
    }
#else
    unregister_driver(TS_CHAR_DEV_NAME);
#endif
}


STATIC void ts_aisle_uninit(int node_id)
{
#if !defined(CFG_SOC_PLATFORM_CLOUD_V2) && !defined(CFG_SOC_PLATFORM_MINIV3)
    aisle_uninit(node_id);
#endif
    ts_unregister_cdev();
}


/* ts can send events directly to the event scheduling module through this driver  */
STATIC int ts_aisle_init(struct platform_device *pdev, int node_id)
{
    int ret;
#ifndef AOS_LLVM_BUILD
    if (g_char_dev.device == NULL) {
        ret = ts_drv_register_cdev();
        if (ret != 0) {
            ts_drv_err("ts register cdev failed, node_id = %d\n", node_id);
            return TS_INNER_ERR;
        }
    }
#else
    ret = register_driver(TS_CHAR_DEV_FULL_NAME, &ts_fops, DEVNODE_PERMISSION, NULL);
    if (ret != 0) {
        ts_drv_err("ts register_driver failed, node_id = %d, ret = %d\n", node_id, ret);
        return TS_INNER_ERR;
    }

#endif

#if !defined(CFG_SOC_PLATFORM_CLOUD_V2) && !defined(CFG_SOC_PLATFORM_MINIV3)
    ret = aisle_init(pdev, node_id);
    if (ret != 0) {
        ts_unregister_cdev();
        ts_drv_err("ts init aisle failed, node_id = %d\n", node_id);
        return TS_INNER_ERR;
    }
#endif
    return 0;
}

STATIC int ts_drv_probe(struct platform_device *pdev)
{
    int ret;
    int node_id;
    struct drv_hwts_ctrl *hwts = NULL;

    set_dev_node(&pdev->dev, g_chip_num);
    g_chip_num++;
    node_id = dev_to_node(&pdev->dev);
    ts_drv_info("hwts_drv_probe (nid:%d).\n", node_id);
    if (node_id < 0 || node_id >= CHIP_NUM_MAX) {
        ts_drv_err("hwts driver init twice! invalid dev_id:%d\n", node_id);
        return -EINVAL;
    }
    if (g_drv_ctrl_hwts[node_id] != NULL) {
        ts_drv_err("hwts driver init twice!dev_id:%d\n", node_id);
        return -EINVAL;
    }

    /* init drv_ctrl_hwts struct */
    hwts = kzalloc(sizeof(struct drv_hwts_ctrl), GFP_KERNEL);
    if (hwts == NULL) {
        ts_drv_err("kzalloc drv_ctrl_hwts err.\n");
        return -ENOMEM;
    }

    g_drv_ctrl_hwts[node_id] = hwts;
    hwts->numa_node = node_id;
    mutex_init(&hwts->mutex);

    /* init hwts driver */
    ret = hwts_drv_init(pdev, hwts, node_id);
    if (ret != 0) {
        ts_drv_err("hwts drv init fail\n");
        goto fail;
    }

    /* init ts aisle */
    ret = ts_aisle_init(pdev, node_id);
    if (ret != 0) {
        hwts_drv_release(pdev, hwts);
        ts_drv_err("ts aisle init fail\n");
        goto fail;
    }
    (void)dev_set_name(&pdev->dev, "tsdrv");

    ts_drv_info("ts drv load success. \n");
    return 0;
fail:
    kfree(hwts);
    hwts = NULL;
    g_drv_ctrl_hwts[node_id] = NULL;

    return TS_INNER_ERR;
}

STATIC void ts_drv_remove(struct platform_device *pdev)
{
    int node_id;
    struct drv_hwts_ctrl *hwts = NULL;

    node_id = dev_to_node(&pdev->dev);
    if (node_id >= CHIP_NUM_MAX) {
        ts_drv_err("node_id out range. node_id:%d, max:%d\n", node_id, CHIP_NUM_MAX);
        return;
    }

    hwts = g_drv_ctrl_hwts[node_id];
    if (hwts == NULL) {
        ts_drv_err("hwts is null. dev_id:%d\n", node_id);
        return;
    }

    hwts_drv_release(pdev, hwts);

    /* free drv_ctrl_hwts */
    kfree(hwts);
    g_drv_ctrl_hwts[node_id] = NULL;

    ts_aisle_uninit(node_id);
    ts_drv_info("ts drv remove ok. dev_id:%d\n", node_id);
}

STATIC int ts_drv_resume(struct platform_device *pdev)
{
    ts_drv_info("ts drv resume.\n");
    return 0;
}

STATIC int ts_drv_suspend(struct platform_device *pdev, pm_message_t state)
{
    ts_drv_info("ts drv suspend.\n");
    return 0;
}

static const struct of_device_id ts_drv_of_match[] = {
    { .compatible = "hisi,tsdrv_device", },
    {},
};

MODULE_DEVICE_TABLE(of, ts_drv_of_match); //lint !e508 !e532
#ifdef CFG_SOC_PLATFORM_CLOUD
static const struct acpi_device_id g_tsdrv_acpi_match[] = {{ "ACPU0000", 0 }, {}}; //lint !e42
MODULE_DEVICE_TABLE(acpi, g_tsdrv_acpi_match); //lint !e508
static struct platform_driver g_ts_platform_driver = {
    .probe = ts_drv_probe, //lint !e24
    .remove = ts_drv_remove, //lint !e24
    .suspend = ts_drv_suspend, //lint !e24
    .resume = ts_drv_resume, //lint !e24
    .driver = { //lint !e24
        .name = "ts_device_driver",
        .owner = THIS_MODULE,
        .of_match_table = ts_drv_of_match,
        .acpi_match_table = ACPI_PTR(g_tsdrv_acpi_match),
    },
};
#else
#ifndef AOS_LLVM_BUILD
static const struct acpi_device_id g_tsdrv_acpi_match[] = {{ "HISI0300", 0 }, { "HISI0302", 0 }, {}};
MODULE_DEVICE_TABLE(acpi, g_tsdrv_acpi_match);
#endif
static struct platform_driver g_ts_platform_driver = {
    .probe = ts_drv_probe, //lint !e24
    .remove = ts_drv_remove, //lint !e24
    .suspend = ts_drv_suspend, //lint !e24
    .resume = ts_drv_resume, //lint !e24
    .driver = { //lint !e24
        .name = "ts_device_driver",
        .of_match_table = ts_drv_of_match,
#ifndef AOS_LLVM_BUILD
        .acpi_match_table = ACPI_PTR(g_tsdrv_acpi_match),
#endif
    },
};
#endif

int __init ts_drv_platform_init(void)
{
    int ret;
#ifdef CFG_SOC_PLATFORM_MDC_V51
    if (devdrv_get_ts_node_num() == 0) {
        ts_drv_info("tsdrv devinit end, ts node does not exist!\n");
        return 0;
    }
#endif
    ret = platform_driver_register(&g_ts_platform_driver);
    if (ret != 0) {
        ts_drv_err("insmod ts platform driver fail. ret=%d\n", ret);
        return ret;
    }

    ts_drv_info("insmod ts platform driver succeed.\n");
    return 0;
}

void __exit ts_drv_platform_exit(void)
{
    platform_driver_unregister(&g_ts_platform_driver);
}

module_init(ts_drv_platform_init);
module_exit(ts_drv_platform_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("TS DRIVER");
#else /* TSDRV_UT */
void ut_device_ts_drv_init_test(void)
{
    return 0;
}
#endif /* TSDRV_UT */

