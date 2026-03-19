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

#ifdef DRV_CENTRE_NOTIFY_UT
#define STATIC
#define __init
#define __exit
#else
#define STATIC static
#endif

#include "drv_comm.h"
#include "drv_notify.h"
#include "ascend_platform.h"

#define DRV_NOTIFY_CDEV_NAME "centre_notify"
#define DRV_NOTIFY_CDEV_FULL_NAME "/dev/centre_notify"
#define DRV_NOTIFY_NONE_ROOT_ACCESS (0640)
/* Chip info */
#if (defined(CFG_SOC_PLATFORM_MDC_V11))
#define SOC_CHIP_INFO_REG_BASE 0xC0140000UL
#define SOC_CHIP_INFO_REG_OFFSET 0xFFF8
#define CHIP_TYPE_REG_VALUE_MASK 0xFFFFF
#elif (defined(CFG_SOC_MDC_V51_LITE))
#define SOC_CHIP_INFO_REG_BASE 0x80000000UL
#define SOC_CHIP_INFO_REG_OFFSET 0xFFF8
#define CHIP_TYPE_REG_VALUE_MASK 0xFFFFF
#else
#define SOC_CHIP_INFO_REG_BASE 0x8000F000UL
#define SOC_CHIP_INFO_REG_OFFSET 0xFF8
#define CHIP_TYPE_REG_VALUE_MASK 0xF
#endif

#define CHIP_TYPE_BS9SX1A 2

typedef enum val_is_actstd_mode {
    VAL_IS_ACTSTD_MODE_NO = 0,
    VAL_IS_ACTSTD_MODE_YES
} VAL_IS_ACTSTD_MODE;


typedef enum val_actstd_state {
    VAL_ACTSTD_STATE_ACT = 0,
    VAL_ACTSTD_STATE_STD
} VAL_ACTSTD_STATE;

typedef enum center_key_list {
    KEY_IS_ACTSTD = 0,
    KEY_IS_LOADBALANC = 1,
    KEY_TOTAL_CHIPNUM = 2,
    KEY_PHY_LOG_MAP = 3,
    KEY_DOMAIN_MODE = 4,
    // Default is KEY_CUR_ACTSTD when KEY_IS_ACTSTD = VAL_IS_ACTSTD_MODE_NO
    // Or default is KEY_CUR_ACTSTD when KEY_IS_ACTSTD = VAL_IS_ACTSTD_MODE_YES
    KEY_CUR_ACTSTD = 5,
    KEY_CUR_PHYPOS = 6,
    KEY_BOARD_ID = 7,
    KEY_CHIP_TYPE = 8,
    KEY_MAX
} CENTER_KEY_LIST;

struct drv_notify_cdev *g_notify_cdev = NULL;
struct drv_notify_centre_ctrl *g_notify_ctl = NULL;

int drv_notify_ops_set(struct drv_notify_cmd *notify_set)
{
    /* slotid index(1023) can not be setted */
    if ((notify_set->index >= DRV_NOTIFY_SLOTID_INDEX) || (notify_set->index < 0)) {
        notify_err("index %d is invalid.\n", notify_set->index);
        return -EINVAL;
    }

    atomic_set(&g_notify_ctl->node_info[notify_set->index].value, notify_set->value);
    g_notify_ctl->node_info[notify_set->index].flag = DRV_NOTIFY_STATUS_VALID;

    return 0;
}

int drv_notify_ops_get(struct drv_notify_cmd *notify_get)
{
    if ((notify_get->index >= DRV_NOTIFY_CENTRE_NUMBER) || (notify_get->index < 0)) {
        notify_err("index %d is invalid\n", notify_get->index);
        return -EINVAL;
    }

    if (g_notify_ctl->node_info[notify_get->index].flag == 0) {
        notify_warn("can not get, because no data, (index=%d, flag=%d).\n",
                    notify_get->index, g_notify_ctl->node_info[notify_get->index].flag);
        return -EPERM;
    }

    notify_get->value = atomic_read(&g_notify_ctl->node_info[notify_get->index].value);

    return 0;
}

ssize_t drv_notify_read(struct file *file, char *data, size_t len, loff_t *off)
{
    int ret;
    struct drv_notify_cmd cmd_data;

    if ((data == NULL) || (len < sizeof(cmd_data))) {
        notify_err("input data is NULL or input len invalid. (data_is_null=%d, len=%lu, correct_len=%lu)\n",
            (data == NULL), len, sizeof(cmd_data));
        return -EINVAL;
    }

    ret = copy_from_user(&cmd_data, data, sizeof(cmd_data));
    if (ret) {
        notify_err("copy_from_user failed\n");
        return -EINVAL;
    }

    ret = drv_notify_ops_get(&cmd_data);
    if (ret) {
        notify_err("drv_notify_ops_get failed\n");
        return -EINVAL;
    }

    ret = copy_to_user(data, &cmd_data, sizeof(cmd_data));
    if (ret) {
        notify_err("copy_to_user failed\n");
        return -EINVAL;
    }

    return EOK;
}

ssize_t drv_notify_write(struct file *file, const char *data, size_t len, loff_t *off)
{
    int ret;
    struct drv_notify_cmd cmd_data;

    if ((data == NULL) || (len < sizeof(cmd_data))) {
        notify_err("input data is NULL or input len invalid. (data_is_null=%d, len=%lu, correct_len=%lu)\n",
            (data == NULL), len, sizeof(cmd_data));
        return -EINVAL;
    }

    ret = copy_from_user(&cmd_data, data, sizeof(cmd_data));
    if (ret) {
        notify_err("copy_from_user failed\n");
        return -EINVAL;
    }

    notify_info("centre notify write, index:%d, value:%d.\n", cmd_data.index, cmd_data.value);

    ret = drv_notify_ops_set(&cmd_data);
    if (ret) {
        notify_err("drv_notify_ops_set failed\n");
        return -EINVAL;
    }

    return EOK;
}

int centre_notify_set_val(int index, int value)
{
    struct drv_notify_cmd notify_set = {0};
    int ret;

    notify_set.index = index;
    notify_set.value = value;

    ret = drv_notify_ops_set(&notify_set);
    if (ret != 0) {
        notify_err("kernel notify set value fail, ret = %d, index = %d, value = %d.\n",
                   ret, index, value);
        return ret;
    }

    notify_info("kernel notfiy set value success, index = %d, value = %d.\n", index, value);
    return 0;
}
EXPORT_SYMBOL(centre_notify_set_val);

int centre_notify_get_val(int index, int *value)
{
    struct drv_notify_cmd notify_get = {0};
    int ret;

    if (value == NULL) {
        notify_err("value is null\n");
        return -EINVAL;
    }

    notify_get.index = index;

    ret = drv_notify_ops_get(&notify_get);
    if (ret != 0) {
        notify_err("kernel notify get value fail, (ret=%d, index=%d)\n", ret, index);
        return ret;
    }

    *value = notify_get.value;
    return 0;
}
EXPORT_SYMBOL(centre_notify_get_val);

ssize_t drv_notify_sysfs_get_slotid(struct device *dev, struct device_attribute *attr, char *buf)
{
    ssize_t offset = 0;
    int ret;

    if (g_notify_ctl->node_info[DRV_NOTIFY_SLOTID_INDEX].flag == 0) {
        notify_warn("get fail, because not set\n");
        return offset;
    }

    ret = snprintf_s(buf, sizeof(int), sizeof(int), "%d\n",
        atomic_read(&g_notify_ctl->node_info[DRV_NOTIFY_SLOTID_INDEX].value));
    if (ret < 0) {
        notify_err("snprintf_s fail\n");
        return offset;
    }

    offset += ret;

    return offset;
}

STATIC DEVICE_ATTR(slotid, S_IRUSR | S_IRGRP, drv_notify_sysfs_get_slotid, NULL);

STATIC struct attribute *g_notify_sysfs_attrs[] = {
    &dev_attr_slotid.attr,
    NULL,
};

STATIC const struct attribute_group g_notify_sysfs_group = {
    .attrs = g_notify_sysfs_attrs,
    .name = "notify",
};

int drv_notify_sysfs_init(struct device *dev)
{
    int ret;

    ret = sysfs_create_group(&dev->kobj, &g_notify_sysfs_group);
    if (ret != 0) {
        notify_err("sysfs create group failed, ret %d\n", ret);
    }

    return ret;
}

void drv_notify_sysfs_uninit(struct device *dev)
{
    sysfs_remove_group(&dev->kobj, &g_notify_sysfs_group);
}

STATIC int drv_notify_get_slot_id(void)
{
    int slotid;
    int ret;
#ifdef CFG_SOC_MDC_V51_LITE
    void __iomem *reg_addr = NULL;
    int reg_value;
#else
    int board_id;
#endif

#ifdef CFG_SOC_MDC_V51_LITE
    reg_addr = ioremap(SYSCTL_REG_BASE_ADDR + DRV_SLOT_ID_REG_OFFSET, sizeof(int));
    if (reg_addr == NULL) {
        notify_err("devm_ioremap failed.\n");
        return -EINVAL;
    }
    reg_value = readl(reg_addr);
    iounmap(reg_addr);
    slotid = reg_value & SLOT_ID_REG_VALUE_MASK;
    notify_info("get slot_id success, slot_id = %d.\n", slotid);
    ret = centre_notify_set_val(KEY_CUR_PHYPOS, slotid);
    if (ret != 0) {
        notify_err("centre_notify_set_val error, value=%d.\n", slotid);
        return ret;
    }
    atomic_set(&g_notify_ctl->node_info[DRV_NOTIFY_SLOTID_INDEX].value, slotid);
    g_notify_ctl->node_info[DRV_NOTIFY_SLOTID_INDEX].flag = DRV_NOTIFY_STATUS_VALID;
    return 0;
#else
    ret = centre_notify_get_val(KEY_BOARD_ID, &board_id);
    if (ret != 0) {
        notify_warn("cannot get board_id.\n");
        return -EINVAL;
    }

    if ((board_id >= NOTIFY_BOARDID_900) && (board_id <= NOTIFY_BOARDID_999)) {
        slotid = 0;
    } else {
        if (!gpio_is_valid(DRV_NOTIFY_SLOTID_GPIO_NUM)) {
            notify_warn("invalid gpio(%d).\n", DRV_NOTIFY_SLOTID_GPIO_NUM);
            return -EINVAL;
        }

        ret = gpio_request(DRV_NOTIFY_SLOTID_GPIO_NUM, "gpio-read");
        if (ret) {
            notify_warn("gpio(%d) request failed\n", DRV_NOTIFY_SLOTID_GPIO_NUM);
            return ret;
        }

        slotid = gpio_get_value(DRV_NOTIFY_SLOTID_GPIO_NUM);
        (void)gpio_free(DRV_NOTIFY_SLOTID_GPIO_NUM);
    }

    notify_info("get slotid success, (slotid=%d)\n", slotid);
    atomic_set(&g_notify_ctl->node_info[DRV_NOTIFY_SLOTID_INDEX].value, slotid);
    g_notify_ctl->node_info[DRV_NOTIFY_SLOTID_INDEX].flag = DRV_NOTIFY_STATUS_VALID;
    (void)centre_notify_set_val(KEY_CUR_PHYPOS, slotid);

    return 0;
#endif
}

const struct file_operations drv_notify_fops = {
    .owner = THIS_MODULE,
    .read = drv_notify_read,
    .write = drv_notify_write,
};

int drv_notify_register_cdev(struct drv_notify_cdev *cdev, const struct file_operations *fops)
{
    struct device *dev = NULL;
    int ret;

    ret = alloc_chrdev_region(&cdev->dev_no, 0, DRV_NOTIFY_CDEV_COUNT, DRV_NOTIFY_CDEV_NAME);
    if (ret != 0) {
        notify_err("alloc char dev failed. ret = %d\n", ret);
        return ret;
    }

    /* init and add char device */
    cdev_init(&cdev->cdev, fops);
    cdev->cdev.owner = THIS_MODULE;

    ret = cdev_add(&cdev->cdev, cdev->dev_no, DRV_NOTIFY_CDEV_COUNT);
    if (ret != 0) {
        notify_err("add char dev failed. ret = %d\n", ret);
        goto CDEV_ADD_FAILED;
    }

    cdev->cdev_class = class_create(DRV_NOTIFY_CDEV_NAME);
    if (cdev->cdev_class == NULL) {
        notify_err("class create failed.\n");
        ret = -EINVAL;
        goto CLASS_CREATE_FAILED;
    }

    dev = device_create(cdev->cdev_class, NULL, cdev->dev_no, NULL, DRV_NOTIFY_CDEV_NAME);
    if (IS_ERR(dev)) {
        notify_err("device create failed.\n");
        ret = -EINVAL;
        goto DEV_CREATE_FAILED;
    }

    cdev->dev = dev;

    return 0;

DEV_CREATE_FAILED:
    class_destroy(cdev->cdev_class);
CLASS_CREATE_FAILED:
    cdev_del(&cdev->cdev);
CDEV_ADD_FAILED:
    unregister_chrdev_region(cdev->dev_no, DRV_NOTIFY_CDEV_COUNT);

    return ret;
}

void drv_notify_free_cdev(struct drv_notify_cdev *cdev)
{
    (void)device_destroy(cdev->cdev_class, cdev->dev_no);
    (void)class_destroy(cdev->cdev_class);
    (void)unregister_chrdev_region(cdev->dev_no, DRV_NOTIFY_CDEV_COUNT);
    (void)cdev_del(&cdev->cdev);

    cdev->cdev_class = NULL;
    cdev->dev = NULL;
}

STATIC void drv_notify_get_soc_workmode(void)
{
    int ret, i;
    struct device_node *np = NULL;
    int soc_workmode_list[KEY_CUR_ACTSTD + 1] = {-1, -1, -1, -1, -1, -1};
    char *soc_workmode_name[] = { "is_actstd", "is_loadbalanc", "total_chipnum", "phy_log_map", "domain_mode", NULL };

    np = of_find_compatible_node(NULL, NULL, "socworkmode");
    if (np == NULL) {
        notify_warn("can't find valid socworkmode node.\n");
    }

    for (i = 0; soc_workmode_name[i] != NULL; i++) {
        ret = of_property_read_u32(np, soc_workmode_name[i], (u32 *)&soc_workmode_list[i]);
        if (ret != 0) {
            notify_warn("can't find valid soc workmode[%d]: %s, ret: %d.\n", i, soc_workmode_name[i], ret);
        }
    }

    switch (soc_workmode_list[KEY_IS_ACTSTD]) {
        case VAL_IS_ACTSTD_MODE_YES:
            soc_workmode_list[KEY_CUR_ACTSTD] = VAL_ACTSTD_STATE_STD;
            break;
        case VAL_IS_ACTSTD_MODE_NO:
            soc_workmode_list[KEY_CUR_ACTSTD] = VAL_ACTSTD_STATE_ACT;
            break;
        default:
            notify_warn("can't find valid KEY_CUR_ACTSTD.\n");
            break;
    }

    for (i = 0; i <= KEY_CUR_ACTSTD; i++) {
        if (soc_workmode_list[i] == DRV_NOTIFY_WORKMODE_INVALID) {
            continue;
        }
        ret = centre_notify_set_val(i, soc_workmode_list[i]);
        if (ret != 0) {
            notify_err("centre_notify_set_val error, index=%d, value=%d.\n", i, soc_workmode_list[i]);
        }
    }

    return;
}

STATIC int drv_notify_get_chip_type(void)
{
    int chip_type;
    int ret;
    void __iomem *reg_addr = NULL;
    int reg_value;

    reg_addr = ioremap(SOC_CHIP_INFO_REG_BASE + SOC_CHIP_INFO_REG_OFFSET, sizeof(int));
    if (reg_addr == NULL) {
        notify_err("devm_ioremap failed.\n");
        return -EINVAL;
    }

    reg_value = readl(reg_addr);
    iounmap(reg_addr);
    chip_type = reg_value & CHIP_TYPE_REG_VALUE_MASK;
    notify_info("get chip_type success, chip_type = 0x%X.\n", chip_type);

    ret = centre_notify_set_val(KEY_CHIP_TYPE, chip_type);
    if (ret != 0) {
        notify_err("centre_notify_set_val error, value=%d.\n", chip_type);
    }

    return ret;
}

STATIC int drv_notify_get_board_id(void)
{
    void __iomem *reg_addr = NULL;
    int reg_value;
    int board_id;
    int ret;

    reg_addr = ioremap(SYSCTL_REG_BASE_ADDR + DRV_BOARD_ID_REG_OFFSET, sizeof(int));
    if (reg_addr == NULL) {
        notify_err("devm_ioremap failed.\n");
        return -EINVAL;
    }
    reg_value = readl(reg_addr);
    iounmap(reg_addr);
    board_id = reg_value & BOARD_ID_REG_VALUE_MASK;
    notify_info("get board_id success, board_id = %d.\n", board_id);

    ret = centre_notify_set_val(KEY_BOARD_ID, board_id);
    if (ret != 0) {
        notify_err("centre_notify_set_val error, value=%d.\n", board_id);
    }

    return 0;
}

STATIC void drv_notify_get_index_info(void)
{
    int ret;

    ret = drv_notify_get_board_id();
    if (ret != 0) {
        notify_warn("get boardid failed. ret = %d\n", ret);
    }

    ret = drv_notify_get_slot_id();
    if (ret != 0) {
        notify_warn("get slotid failed. ret = %d\n", ret);
    }

    ret = drv_notify_get_chip_type();
    if (ret != 0) {
        notify_warn("devdrv_get_chip_type_by_reg failed, ret(%d).\n", ret);
    }

    drv_notify_get_soc_workmode();

    return;
}

int drv_notify_centre_init(void)
{
    int ret;

    g_notify_ctl = (struct drv_notify_centre_ctrl *)kzalloc(sizeof(struct drv_notify_centre_ctrl), GFP_KERNEL);
    if (g_notify_ctl == NULL) {
        notify_err("alloc notify_centre failed, size = %lu\n", sizeof(struct drv_notify_centre_ctrl));
        return -ENOMEM;
    }

    g_notify_cdev = (struct drv_notify_cdev *)kzalloc(sizeof(struct drv_notify_cdev), GFP_KERNEL);
    if (g_notify_cdev == NULL) {
        notify_err("alloc notify_cdev failed, size = %lu\n", sizeof(struct drv_notify_cdev));
        ret = -ENOMEM;
        goto NOT_STATUS_FREE;
    }
#ifdef AOS_LLVM_BUILD
    ret = register_driver(DRV_NOTIFY_CDEV_FULL_NAME, &drv_notify_fops, DRV_NOTIFY_NONE_ROOT_ACCESS, NULL);
#else
    ret = drv_notify_register_cdev(g_notify_cdev, &drv_notify_fops);
#endif
    if (ret != 0) {
        notify_err("not create char dev failed. ret = %d\n", ret);
        goto NOT_CDEV_FREE;
    }

    drv_notify_get_index_info();

    ret = drv_notify_sysfs_init(g_notify_cdev->dev);
    if (ret != 0) {
        notify_err("sysfs create failed.\n");
        goto DEVICE_DESTORY;
    }

    g_notify_ctl->is_valid = DRV_NOTIFY_STATUS_VALID;

    return 0;

DEVICE_DESTORY:
#ifdef AOS_LLVM_BUILD
    unregister_driver(DRV_NOTIFY_CDEV_FULL_NAME);
#else
    drv_notify_free_cdev(g_notify_cdev);
#endif
NOT_CDEV_FREE:
    kfree(g_notify_cdev);
    g_notify_cdev = NULL;
NOT_STATUS_FREE:
    kfree(g_notify_ctl);
    g_notify_ctl = NULL;

    return ret;
}

void drv_notify_centre_uninit(void)
{
    drv_notify_sysfs_uninit(g_notify_cdev->dev);
#ifdef AOS_LLVM_BUILD
    unregister_driver(DRV_NOTIFY_CDEV_FULL_NAME);
#else
    drv_notify_free_cdev(g_notify_cdev);
#endif
    kfree(g_notify_cdev);
    g_notify_cdev = NULL;
    kfree(g_notify_ctl);
    g_notify_ctl = NULL;
}

STATIC int __init drv_notify_init(void)
{
    return drv_notify_centre_init();
}

STATIC void __exit drv_notify_exit(void)
{
    drv_notify_centre_uninit();
}

module_init(drv_notify_init);
module_exit(drv_notify_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("centre notify driver");
