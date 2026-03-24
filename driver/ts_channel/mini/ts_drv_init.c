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
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/securec.h>
#include <linux/securectype.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/of_irq.h>
#include <linux/uio_driver.h>
#include <linux/ioctl.h>
#ifndef TSDRV_UT
#include "ts_drv_init.h"
#include "ts_drv_cmd.h"
#include "hwts_drv_api.h"
#include "devdrv_dfm.h"
#include "ts_aisle_api.h"
#include "ts_api.h"
#include "ts_aisle_queue.h"
#include "ts_drv_sysfs.h"

struct drv_hwts_ctrl *g_drv_ctrl_hwts[CHIP_NUM_MAX] = {0};
struct ts_char_dev char_dev;

/* used for c-dev */
struct ts_char_dev {
    struct device *device;
    struct class *dev_class;
    struct cdev cdev;
    dev_t devno;
};

static int copy_from_user_safe(void *to, const void __user *from, unsigned long n)
{
    if (from == NULL || n == 0) {
        ts_drv_err("user pointer is NULL.\n");
        return TS_INNER_ERR;
    }

    if (copy_from_user(to, (void *)from, n)) {
        ts_drv_err("copy_from_user failed size %ld.\n", n);
        return TS_INNER_ERR;
    }

    return 0;
}


STATIC int ts_fop_send_msg_to_ts_async(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct ts_ioctl_para para_info = {0};

    char msg[TS_IPC_MSG_MAX_LEN * sizeof(u32)];

    if (copy_from_user_safe(&para_info, (void *)(uintptr_t)arg, sizeof(struct ts_ioctl_para))) {
        return TS_COPY_USER_ERR;
    }

    if (para_info.u.aicpu_msg.msg_len > (TS_IPC_MSG_MAX_LEN * sizeof(u32) - MSG_HEAD_LEN)) {
        ts_drv_err("msg len error, msg_len:%u, out range:%lu\n", para_info.u.aicpu_msg.msg_len,
            (TS_IPC_MSG_MAX_LEN * sizeof(u32) - MSG_HEAD_LEN));
        return TS_PARA_ERR;
    }

    if (copy_from_user_safe(msg, (void *)(uintptr_t)para_info.u.aicpu_msg.msg, para_info.u.aicpu_msg.msg_len)) {
        ts_drv_err("copy msg from user error\n");
        return TS_COPY_USER_ERR;
    }

    return send_msg_to_ts_async(para_info.dev_id, para_info.ts_id, para_info.u.aicpu_msg.msg_len, msg);
}

static int (* const ts_ioctl_handler[TS_CMD_MAX_NR])(struct file *filep, unsigned int cmd, unsigned long arg) = {
    [_IOC_NR(TS_SEND_MSG_TO_TS_ASYNC_ID)] = ts_fop_send_msg_to_ts_async,
};

STATIC int ts_fop_open(struct inode *inode, struct file *filep)
{
    struct ts_context *context = NULL;

    context = filep->private_data;
    if (context != NULL) {
        ts_drv_info("proc pid(%d), thread pid(%d):opened.\n", current->tgid, current->pid);
        return TS_INNER_ERR;
    }

    context = (struct ts_context *)kmalloc(sizeof(struct ts_context), GFP_KERNEL | __GFP_ACCOUNT);
    if (context == NULL) {
        return TS_INNER_ERR;
    }
    context->ts_id = TS_INVALID_ID;

    filep->private_data = context;

    return 0;
}

STATIC int ts_fop_release(struct inode *inode, struct file *filep)
{
    struct ts_context *context = filep->private_data;

    if (context == NULL) {
        ts_drv_err("ts drv release error, context is null.\n");
        return TS_INNER_ERR;
    }

    kfree(filep->private_data);
    filep->private_data = NULL;
    return 0;
}

STATIC long ts_fop_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
    if (_IOC_NR(cmd) >= TS_CMD_MAX_NR) {
        ts_drv_err("invalid cmd, cmd = %u\n", _IOC_NR(cmd));
        return TS_INNER_ERR;
    }

    if (ts_ioctl_handler[_IOC_NR(cmd)] == NULL) {
        ts_drv_err("invalid cmd, cmd = %u\n", _IOC_NR(cmd));
        return TS_INNER_ERR;
    }

    return ts_ioctl_handler[_IOC_NR(cmd)](filep, cmd, arg);
}


const struct file_operations ts_fops = {
    .owner = THIS_MODULE,
    .open = ts_fop_open,
    .release = ts_fop_release,
    .unlocked_ioctl = ts_fop_ioctl,
};

char *ts_devnode(struct device *dev, umode_t *mode)
{
    if (mode != NULL) {
        *mode = DEVNODE_PERMISSION;
    }
    return NULL;
}


STATIC int ts_drv_register_cdev(void)
{
    unsigned int ts_drv_major;
    int ret;

    ret = alloc_chrdev_region(&char_dev.devno, 0, MINOR_DEV_COUNT, TS_CHAR_DEV_NAME);
    if (ret < 0) {
        ts_drv_err("alloc_chrdev_region error, ret = %d.\n", ret);
        return TS_INNER_ERR;
    }

    cdev_init(&char_dev.cdev, &ts_fops);
    char_dev.cdev.owner = THIS_MODULE;

    if (cdev_add(&char_dev.cdev, char_dev.devno, 1)) {
        ts_drv_err("cdev_add error, devno = %u.\n", char_dev.devno);
        goto unregister_chrdev_region;
    }

    char_dev.dev_class = class_create(THIS_MODULE, TS_CHAR_DEV_NAME);
    if (char_dev.dev_class == NULL) {
        ts_drv_err("class_create error.\n");
        goto cdev_del;
    }

    char_dev.dev_class->devnode = ts_devnode;

    char_dev.device = device_create(char_dev.dev_class, NULL, char_dev.devno, NULL, TS_CHAR_DEV_NAME);
    if (char_dev.device == NULL) {
        ts_drv_err("device create error.\n");
        goto class_destroy;
    }

    ret = sysfs_create_group(&char_dev.device->kobj, ts_drv_get_aisle_attr_group());
    if (ret != 0) {
        ts_drv_err("sysfs create failed, ret = %d\n", ret);
        goto device_destroy;
    }
    ts_drv_debug("register cdev success.\n");
    return 0;

device_destroy:
    ts_drv_major = MAJOR(char_dev.devno);
    device_destroy(char_dev.dev_class, MKDEV(ts_drv_major, 0));
class_destroy:
    class_destroy(char_dev.dev_class);
cdev_del:
    cdev_del(&char_dev.cdev);
unregister_chrdev_region:
    unregister_chrdev_region(char_dev.devno, 1);
    return TS_INNER_ERR;
}

STATIC void ts_unregister_cdev(void)
{
    device_destroy(char_dev.dev_class, char_dev.devno);
    class_destroy(char_dev.dev_class);
    cdev_del(&char_dev.cdev);
    unregister_chrdev_region(char_dev.devno, 1);
}


STATIC void ts_aisle_uninit(int node_id)
{
    aisle_uninit(node_id);
    ts_unregister_cdev();
}


/* ts can send events directly to the event scheduling module through this driver  */
STATIC int ts_aisle_init(struct platform_device *pdev, int node_id)
{
    int ret;

    ret = ts_drv_register_cdev();
    if (ret != 0) {
        ts_drv_err("ts register cdev failed\n");
        return TS_INNER_ERR;
    }

    ret = aisle_init(pdev, node_id);
    if (ret != 0) {
        ts_drv_err("ts init aisle failed\n");
        return TS_INNER_ERR;
    }

    return 0;
}

STATIC int ts_drv_probe(struct platform_device *pdev)
{
    int ret = 0;
    int node_id = numa_node_id();
    struct drv_hwts_ctrl *hwts = NULL;

    ts_drv_info("hwts_drv_probe nid:%d\n", node_id);

    if (g_drv_ctrl_hwts[node_id] != NULL) {
        ts_drv_err("hwts driver init twice!dev_id:%d\n", node_id);
        return TS_INNER_ERR;
    }

    /* init drv_ctrl_hwts struct */
    hwts = kzalloc(sizeof(struct drv_hwts_ctrl), GFP_KERNEL);
    if (hwts == NULL) {
        ts_drv_err("kzalloc drv_ctrl_hwts err.\n");
        return TS_INNER_ERR;
    }

    g_drv_ctrl_hwts[node_id] = hwts;
    hwts->numa_node = node_id;

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

    ts_drv_info("ts drv load success.\n");
    return 0;


fail:
    kfree(hwts);
    hwts = NULL;
    g_drv_ctrl_hwts[node_id] = NULL;

    return TS_INNER_ERR;
}

STATIC int ts_drv_remove(struct platform_device *pdev)
{
    int node_id;
    struct drv_hwts_ctrl *hwts = NULL;

    sysfs_remove_group(&char_dev.device->kobj, ts_drv_get_aisle_attr_group());

    node_id = numa_node_id();
    if (node_id < 0 || node_id >= CHIP_NUM_MAX) {
        ts_drv_err("node_id out range. node_id:%d, max:%d\n", node_id, CHIP_NUM_MAX);
        goto remove_exit;
    }

    hwts = g_drv_ctrl_hwts[node_id];
    if (hwts == NULL) {
        ts_drv_err("hwts is null. dev_id:%d\n", node_id);
        goto remove_exit;
    }

    hwts_drv_release(pdev, hwts);

    /* free drv_ctrl_hwts */
    kfree(hwts);
    g_drv_ctrl_hwts[node_id] = NULL;

    ts_aisle_uninit(node_id);
    ts_drv_info("ts drv remove ok. dev_id:%d\n", node_id);
    return 0;

remove_exit:
    return TS_INNER_ERR;
}

STATIC int ts_drv_resume(struct platform_device *pdev)
{
    return 0;
}

STATIC int ts_drv_suspend(struct platform_device *pdev, pm_message_t state)
{
    return 0;
}

static const struct of_device_id ts_drv_of_match[] = {
    { .compatible = "hisi,tsdrv_device", },
    {},
};

MODULE_DEVICE_TABLE(of, ts_drv_of_match); //lint !e508 !e532

static struct platform_driver ts_platform_driver = {
    .probe = ts_drv_probe, //lint !e24
    .remove = ts_drv_remove, //lint !e24
    .suspend = ts_drv_suspend, //lint !e24
    .resume = ts_drv_resume, //lint !e24
    .driver = { //lint !e24
        .name = "ts_device_driver",
        .of_match_table = ts_drv_of_match,
    },
};

int __init ts_drv_platform_init(void)
{
    int ret;

    ret = platform_driver_register(&ts_platform_driver);
    if (ret != 0) {
        ts_drv_err("insmod ts platform driver fail. ret=%d\n", ret);
        return ret;
    }

    ts_drv_info("insmod ts platform driver succeed.\n");
    return 0;
}

void __exit ts_drv_platform_exit(void)
{
    platform_driver_unregister(&ts_platform_driver);
}

module_init(ts_drv_platform_init);
module_exit(ts_drv_platform_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("TS DRIVER");
#else
void ut_device_ts_drv_init_test(void)
{
}
#endif
