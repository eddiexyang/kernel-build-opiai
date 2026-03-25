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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include "pkicms/pkicms_common.h"
#include "linux/securec.h"
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/io.h>
#include <asm/ioctl.h>
#include <linux/uaccess.h>
#include <linux/types.h>
#include <linux/fs.h>
#include "pkicms_module.h"
#include "pkicms/soc_verify.h"

#ifdef CFG_SOC_PLATFORM_MDC_V51
#include <linux/moduleparam.h>
#include <linux/string.h>
#include "pkicms/pkicms_api.h"
#include "pkicms/pkicms.h"
#include <linux/fcntl.h>
#include <linux/file.h>
#include <linux/syscalls.h>

static char *g_img_path = NULL;

module_param(g_img_path, charp, S_IRUSR | S_IWUSR);
#endif

cdev_st *g_pkicms_devp = NULL;

int pkicms_open(struct inode *inode, struct file *filp)
{
    return 0;
}

int pkicms_release(struct inode *inode, struct file *filp)
{
    return 0;
}

STATIC long pkicms_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int ret;
    int check_flag = PKICMS_SEC_CHECK_ENABLE;
    unsigned int l2_nvcnt = 0;
    void *arg_temp = (void *)(uintptr_t)arg;

    if (_IOC_TYPE(cmd) != PKICMS_CMD_MAGIC || _IOC_NR(cmd) > PKICMS_CMD_MAX_NR) {
        pkicms_err("pkicms_ioctl failed, cmd is illegal\n");
        return -EINVAL;
    }
    if (arg_temp == NULL) {
        pkicms_err("pkicms_ioctl failed, arg is NULL\n");
        return -EINVAL;
    }

    switch (cmd) {
        case PKICMS_CMD_GET_EFUSE_FLAG:
            ret = pkicms_get_sec_check_enable_flag(&check_flag);
            if (ret != 0) {
                pkicms_err("get check_enable flag failed,ret = %d\n", ret);
                return -EIO;
            }
            ret = copy_to_user(arg_temp, &check_flag, sizeof(check_flag));
            if (ret != 0) {
                pkicms_err("copy to user failed,ret = %d\n", ret);
                return -EIO;
            }
            break;
        case PKICMS_CMD_GET_EFUSE_NVCNT:
            ret = pkicms_nvcnt_read(0, &l2_nvcnt);
            if (ret != 0) {
                pkicms_err("get l2_nvcnt failed, ret = %d\n", ret);
                return -EIO;
            }
            ret = copy_to_user(arg_temp, &l2_nvcnt, sizeof(l2_nvcnt));
            if (ret != 0) {
                pkicms_err("copy l2_nvcnt(%d) to user failed, ret = %d\n", l2_nvcnt, ret);
                return -EIO;
            }
            break;
        default:
            pkicms_err("ioctl cmd is illegal,cmd = 0x%x\n", cmd);
            return -EINVAL;
    }

    return ret;
}

struct file_operations g_pkicms_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = pkicms_ioctl,
    .open = pkicms_open,
    .release = pkicms_release,
};

/* set cdev node 0600 */
char *pkicms_devnode(const struct device *d, umode_t *mode)
{
    (void)d;
    if (mode != NULL) {
        *mode = DEVNODE_MODE;
    }
    return NULL;
}

#ifdef CFG_SOC_PLATFORM_MDC_V51
static int pkicms_init_cdev(cdev_st *pdev, struct file_operations *pfoprs)
{
    int ret;
    dev_t dev_no;
    struct device *devc = NULL;
    struct class *dev_class = NULL;

    if ((pdev == NULL) || (pfoprs == NULL)) {
        pkicms_err("pkicms_init_cdev input para is NULL, pdev: %pK, pfoprs: %pK\r\n", pdev, pfoprs);
        return -EINVAL;
    }

    ret = alloc_chrdev_region(&dev_no, 0, DEV_COUNT, DEV_NAME);
    if (ret < 0) {
        pkicms_err("get dev_no failed, ret = %d\n", ret);
        return ret;
    }

    /* create device class and add device to fs */
    dev_class = class_create(THIS_MODULE, DEV_CLASS);
    if (IS_ERR(dev_class)) {
        ret = PTR_ERR(dev_class);
        pkicms_err("Failed to create class, ret = %d\n", ret);
        goto exit_unregister_chrdev_region;
    }

    dev_class->devnode = pkicms_devnode;

    devc = device_create(dev_class, NULL, dev_no, NULL, DEV_NAME);
    if (IS_ERR(devc)) {
        ret = PTR_ERR(devc);
        pkicms_err("Failed to create device, ret = %d\n", ret);
        goto exit_class_destroy;
    }

    cdev_init(&pdev->cdev, pfoprs);
    pdev->cdev.owner = THIS_MODULE;
    ret = cdev_add(&pdev->cdev, dev_no, DEV_COUNT);
    if (ret != 0) {
        pkicms_err("adding pkicms device failed, ret = %d\n", ret);
        goto exit_device_create;
    }

    pdev->dev_class = dev_class;
    pdev->dev_major = MAJOR(dev_no);
    pdev->dev_minor = MINOR(dev_no);

    return 0;

exit_device_create:
    device_destroy(dev_class, dev_no);
exit_class_destroy:
    class_destroy(dev_class);
exit_unregister_chrdev_region:
    unregister_chrdev_region(dev_no, DEV_COUNT);

    return ret;
}
#endif

static void pkicms_cleanup_dev(cdev_st *pdev)
{
    dev_t dev_no;

    if ((pdev == NULL) || (pdev->dev_class == NULL)) {
        pkicms_err("pkicms_cleanup_dev input para is NULL, pdev: %pK\r\n", pdev);
        return;
    }

    dev_no = MKDEV(pdev->dev_major, pdev->dev_minor);
    cdev_del(&pdev->cdev);
    device_destroy(pdev->dev_class, dev_no);
    class_destroy(pdev->dev_class);
    unregister_chrdev_region(dev_no, DEV_COUNT);
    return;
}

static int __init pkicms_dev_init(void)
{
    int ret;
#ifdef CFG_SOC_PLATFORM_MDC_V51
    crl_info_st crl_info = {NULL, 0};
#endif

    pkicms_debug("pkicms module loaded...\n");
    ret = soc_verify_init();
    if (ret != 0) {
        pkicms_err("Soc verify init failed. (ret=%d)\n", ret);
        return PKICMS_ERR_PARAM;
    }

#ifdef CFG_SOC_PLATFORM_MDC_V51
    g_pkicms_devp = (cdev_st *)kzalloc(sizeof(cdev_st), GFP_KERNEL | __GFP_ACCOUNT);
    if (g_pkicms_devp == NULL) {
        pkicms_err(" kmalloc failed!\n");
        return PKICMS_ERR_MEMORY;
    }

    ret = pkicms_init_cdev(g_pkicms_devp, &g_pkicms_fops);
    if (ret != 0) {
        pkicms_err("call pkicms_init_cdev failed! ret: %d\n", ret);
        kfree(g_pkicms_devp);
        g_pkicms_devp = NULL;
        return -EIO;
    }

    if (g_img_path == NULL) {
        pkicms_info("input_img_path is empty\n");
        return 0;
    }

    if ((strcmp(g_img_path, ROOTFS_PATH_MASTER) != 0) && (strcmp(g_img_path, ROOTFS_PATH_BACKUP) != 0)) {
        pkicms_err("input_img_path [%s] is illegal\n", g_img_path);
        pkicms_cleanup_dev(g_pkicms_devp);
        kfree(g_pkicms_devp);
        g_pkicms_devp = NULL;
        return PKICMS_ERR_PARAM;
    }

    ret = pkicms_verify_cms(TYPE_ROOTFS, g_img_path, &crl_info, NULL);
    if (ret != 0) {
        pkicms_err("image verify failed!\n");
        pkicms_cleanup_dev(g_pkicms_devp);
        kfree(g_pkicms_devp);
        g_pkicms_devp = NULL;
        return PKICMS_ERR_HASH;
    }
    pkicms_debug("image verify success!\n");
#endif

    return 0;
}

static void __exit pkicms_dev_exit(void)
{
    pkicms_debug("%s: pkicms module unload...\n", __func__);

    if (g_pkicms_devp == NULL) {
        return;
    }

    pkicms_cleanup_dev(g_pkicms_devp);

    kfree(g_pkicms_devp);
    g_pkicms_devp = NULL;
    return;
}

module_init(pkicms_dev_init);
module_exit(pkicms_dev_exit);

MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("CMS VERIFY");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_VERSION("V1.0");
