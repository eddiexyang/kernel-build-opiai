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

#include "can_drv_sysfs.h"
#include "can_drv_core.h"

#define SYSFS_CMD_CLEAR_STATUS  0
#define PERF_DIVIDED_BASE       100
#define PROCFS_MAX_LEN 10
#ifdef STATIC_SKIP
    #define STATIC
#else
    #define STATIC static
#endif

#ifdef RUN_IN_AOS
/* mttcan dir proc */
#define CAN_PROCFS_DIR_NAME "mttcan"
struct proc_dir_entry *g_proc_mttcan_entry = NULL;
/* procfs name */
STATIC struct proc_dir_entry *g_proc_status_entry = NULL;
#define CAN_PROCFS_STATUS_NODE_NAME "mttcan/mttcan_err_status"
#define CAN_PROCFS_STATUS_NODE_OWNER 0640

STATIC ssize_t mttcan_procfs_status_read(struct file *file, char __user *buffer, size_t count, loff_t *ppos)
{
    (void)ppos;
    int i;
    int ret;
    int offset = 0;
    struct net_device *ndev = NULL;
    struct mttcan_priv *priv = NULL;
    char *tmp_buff = NULL;

    if ((count == 0) || (count > PAGE_SIZE)) {
        mttcan_err("count must be less than 4096,and not equal to 0，but now is %u\n", count);
        return -EINVAL;
    }
    tmp_buff = kmalloc(count, GFP_KERNEL | __GFP_ACCOUNT);
    if (tmp_buff == NULL) {
        mttcan_err("kmalloc failed\n");
        return -ENOMEM;
    }

    for (i = 0; i < MAX_CAN_NUN; i++) {
        ndev = get_net_devices(i);
        if (ndev != NULL) {
            priv = netdev_priv(ndev);
            if (strlen(ndev->name) >= IFNAMSIZ) {
                mttcan_err("ndev name is invaild\n");
                return -EFAULT;
            }
            ret = snprintf_s(tmp_buff + offset, count - offset, count - offset - 1,
                "%s errcnt:\n"
                " stuff=%llu\n form=%llu\n"
                " ack=%llu\n bit0=%llu\n"
                " bit1=%llu\n crc=%llu\n"
                " rxfl=%llu\n txbf=%llu\n"
                " unknow=%llu\n"
                " rxkf=%llu\n txkf=%llu\n"
                " delay2ms=%llu\n delay5ms=%llu\n"
                " txfl=%llu\n",
                priv->ndev->name,
                priv->status_cnt.lec_type_stuff_err_cnt, priv->status_cnt.lec_type_form_err_cnt,
                priv->status_cnt.lec_type_ack_err_cnt, priv->status_cnt.lec_type_bit0_err_cnt,
                priv->status_cnt.lec_type_bit1_err_cnt, priv->status_cnt.lec_type_crc_err_cnt,
                priv->status_cnt.rx_fifo_lost_err_cnt, priv->status_cnt.tx_buffer_full_err_cnt,
                priv->status_cnt.unknow_err_cnt,
                priv->status_cnt.rx_kfifo_full_cnt, priv->status_cnt.tx_kfifo_full_cnt,
                priv->status_cnt.rx_tasklet_delay2ms_cnt, priv->status_cnt.rx_tasklet_delay5ms_cnt,
                priv->status_cnt.tx_fifo_lost_err_cnt);
            if (ret >= 0) {
                offset += ret;
            }
        }
    }

    if (copy_to_user(buffer, tmp_buff, offset)) {
        offset = -EFAULT;
        mttcan_err("copy_to_user failed\n");
    }
    kfree(tmp_buff);
    return offset;
}

STATIC void clear_all_err_status(void)
{
    int i;
    int ret;
    struct net_device *ndev = NULL;
    struct mttcan_priv *priv = NULL;
    for (i = 0; i < MAX_CAN_NUN; i++) {
        ndev = get_net_devices(i);
        if (ndev != NULL) {
            priv = netdev_priv(ndev);
            ret = memset_s(&priv->status_cnt, sizeof(struct run_status_cnt),
                0, sizeof(struct run_status_cnt));
            if (ret != EOK) {
                mttcan_err("all status_cnt clear failed\n");
            }
        }
    }
}

STATIC int mttcan_procfs_get_num(const char __user *buffer, size_t count, int *val)
{
    int ret = -1;
    char tmp_buff[PROCFS_MAX_LEN] = { 0 };

    /* control parameter length */
    if (count >= PROCFS_MAX_LEN) {
        mttcan_err("count must be less than 10\n");
        return -EINVAL;
    }

    /* buff user to core */
    if (copy_from_user(tmp_buff, buffer, count)) {
        mttcan_err("copy_from_user failed\n");
        return -EFAULT;
    }

    /* converted into easy-to-handle numbers */
    ret = kstrtoint(tmp_buff, PARSED_AS_DECIMAL, val);
    if (ret != 0) {
        mttcan_err("kstrtoint Invalid argument.\n");
        return ret;
    }
    return 0;
}

STATIC ssize_t mttcan_procfs_status_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
    (void)ppos;
    int val;
    int ret;

    ret = mttcan_procfs_get_num(buffer, count, &val);
    if (ret != 0) {
        return ret;
    }

    switch (val) {
        case SYSFS_CMD_CLEAR_STATUS:
            clear_all_err_status();
            break;
        default:
            mttcan_err("The supported parameters are currently only 0\n");
            return -EINVAL;
    }
    return count;
}

static const struct file_operations mttcan_proc_status_fops = {
    .read = mttcan_procfs_status_read,
    .write = mttcan_procfs_status_write,
};

void mttcan_proc_create_node(struct net_device *ndev)
{
    if (g_proc_mttcan_entry == NULL) {
        g_proc_mttcan_entry = proc_mkdir(CAN_PROCFS_DIR_NAME, NULL);
        if (g_proc_mttcan_entry == NULL) {
            mttcan_err("proc_create mttcan node fail\n");
        }
    }

    if (g_proc_status_entry == NULL) {
        g_proc_status_entry = proc_create(CAN_PROCFS_STATUS_NODE_NAME,
            CAN_PROCFS_STATUS_NODE_OWNER, NULL, &mttcan_proc_status_fops);
        if (g_proc_status_entry == NULL) {
            mttcan_err("proc_create status node fail.\n");
        }
    }
}

void mttcan_proc_remove_node(struct net_device *ndev)
{
    if (g_proc_status_entry != NULL) {
        proc_remove(g_proc_status_entry);
        g_proc_status_entry = NULL;
    }

    if (g_proc_mttcan_entry != NULL) {
        proc_remove(g_proc_mttcan_entry);
    }
}

#else

STATIC int mttcan_store_get_num(struct device *dev, const char *buf, size_t count, int *val)
{
    struct net_device *ndev = dev_get_drvdata(dev);
    struct mttcan_priv *priv = NULL;
    char tmp_buff[PROCFS_MAX_LEN] = { 0 };
    int ret = -1;

    /* must be a existing net device */
    if ((ndev == NULL) || (buf == NULL)) {
        mttcan_err("dev is NULL\n");
        return -EFAULT;
    }

    /* get device priv */
    priv = netdev_priv(ndev);
    if (priv == NULL) {
        mttcan_err("netdev_priv failed, priv is NULL\n");
        return -EFAULT;
    }

    /* control parameter length */
    if (count >= PROCFS_MAX_LEN) {
        mttcan_err("count must be less than 10\n");
        return -EINVAL;
    }

    /* buf safety check */
    ret = memcpy_s(tmp_buff, PROCFS_MAX_LEN, buf, count);
    if (ret != 0) {
        mttcan_err("memcpy_s failed.\n");
        return -EINVAL;
    }

    /* converted into easy-to-handle numbers */
    if (kstrtoint(tmp_buff, 0, val) < 0) {
        mttcan_err("kstrtoint failed\n");
        return -EINVAL;
    }
    return 0;
}

STATIC ssize_t status_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct net_device *ndev = dev_get_drvdata(dev);
    struct mttcan_priv *priv = NULL;
    int bytes_read;

    if ((ndev == NULL) || (buf == NULL)) {
        mttcan_err("errors_show dev is NULL\n");
        return -EFAULT;
    }

    if (strlen(ndev->name) >= IFNAMSIZ) {
        mttcan_err("ndev name is invaild\n");
        return -EFAULT;
    }

    priv = netdev_priv(ndev);
    bytes_read = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
        "%s:\n"
        " lec_type_stuff_err_cnt = %llu\n lec_type_form_err_cnt = %llu\n"
        " lec_type_ack_err_cnt = %llu\n lec_type_bit0_err_cnt = %llu\n"
        " lec_type_bit1_err_cnt = %llu\n lec_type_crc_err_cnt = %llu\n"
        " rx_fifo_lost_err_cnt = %llu\n tx_buffer_full_err_cnt = %llu\n"
        " unknow_err_cnt = %llu\n"
        " rx_kfifo_full_cnt = %llu\n tx_kfifo_full_cnt = %llu\n"
        " rx_tasklet_delay2ms_cnt = %llu\n rx_tasklet_delay5ms_cnt = %llu\n"
        " txfl=%llu\n",
        ndev->name,
        priv->status_cnt.lec_type_stuff_err_cnt, priv->status_cnt.lec_type_form_err_cnt,
        priv->status_cnt.lec_type_ack_err_cnt, priv->status_cnt.lec_type_bit0_err_cnt,
        priv->status_cnt.lec_type_bit1_err_cnt, priv->status_cnt.lec_type_crc_err_cnt,
        priv->status_cnt.rx_fifo_lost_err_cnt, priv->status_cnt.tx_buffer_full_err_cnt,
        priv->status_cnt.unknow_err_cnt,
        priv->status_cnt.rx_kfifo_full_cnt, priv->status_cnt.tx_kfifo_full_cnt,
        priv->status_cnt.rx_tasklet_delay2ms_cnt, priv->status_cnt.rx_tasklet_delay5ms_cnt,
        priv->status_cnt.tx_fifo_lost_err_cnt);
    if (bytes_read < 0) {
        mttcan_err("record buff failed\n");
    }
    mttcan_dump_reg(ndev);

    return bytes_read;
}

STATIC void clear_err_status(struct net_device *ndev)
{
    struct mttcan_priv *priv = netdev_priv(ndev);
    int err;

    err = memset_s(&priv->status_cnt, sizeof(struct run_status_cnt),
        0, sizeof(struct run_status_cnt));
    if (err != EOK) {
        mttcan_err("Set status_cnt clear failed. (uid=%u)\n", __kuid_val(current_uid()));
        return;
    }
    mttcan_info("Set status_cnt clear success. (uid=%u)\n", __kuid_val(current_uid()));
}

STATIC ssize_t status_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct net_device *ndev = dev_get_drvdata(dev);
    int ret = -1;
    int val = 0;

    ret = mttcan_store_get_num(dev, buf, count, &val);
    if (ret != 0) {
        return ret;
    }

    switch (val) {
        case SYSFS_CMD_CLEAR_STATUS:
            clear_err_status(ndev);
            break;
        default:
            mttcan_err("The supported parameters are currently only 0\n");
            break;
    }

    return count;
}

STATIC ssize_t latency_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int i, j;
    int ret;
    int bytes_read = 0;
    struct net_device *ndev = dev_get_drvdata(dev);
    struct mttcan_priv *priv = NULL;

    if ((ndev == NULL) || (buf == NULL)) {
        mttcan_err("latency_show dev is NULL\n");
        return -EFAULT;
    }
    priv = netdev_priv(ndev);

    ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "start time threshold cost(ns) action\n");
    if (ret >= 0) {
        bytes_read += ret;
    }

    for (i = 0; i < MTTCAN_ACTION_MAX; i++) {
        for (j = 0; j < PERF_RECORD_SIZE; j++) {
            if (priv->can_perf_record[i].data[j].start_time == 0) {
                continue;
            }
            ret = snprintf_s(buf + bytes_read, PAGE_SIZE - bytes_read, PAGE_SIZE - bytes_read - 1,
                "%-20llu%-10llu%-9llu%u\n", priv->can_perf_record[i].data[j].start_time,
                priv->can_perf_record[i].latency_threshold, priv->can_perf_record[i].data[j].latency, i);
            if (ret >= 0) {
                bytes_read += ret;
            }
        }
    }

    return bytes_read;
}

STATIC ssize_t latency_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct net_device *ndev = dev_get_drvdata(dev);
    int val = 0;
    MTTCAN_PERF_ACTION delay_action;
    s64 delay_threshold;
    struct mttcan_priv *priv = NULL;
    int ret = -1;

    ret = mttcan_store_get_num(dev, buf, count, &val);
    if (ret != 0) {
        return ret;
    }

    priv = netdev_priv(ndev);
    delay_action = val % PERF_DIVIDED_BASE;
    delay_threshold = val / PERF_DIVIDED_BASE;
    if (delay_action < 0 || delay_action >= MTTCAN_ACTION_MAX) {
        mttcan_err("Set delay action failed. (uid=%u)\n", __kuid_val(current_uid()));
        return -EINVAL;
    }
    mttcan_info("Set delay action=%u, latency_threshold=%lld success. (uid=%u)\n",
        delay_action, delay_threshold, __kuid_val(current_uid()));
    priv->can_perf_record[delay_action].latency_threshold = delay_threshold;

    return count;
}

static DEVICE_ATTR(latency, S_IRUSR | S_IWUSR | S_IRGRP,
    latency_show, latency_store);

static DEVICE_ATTR(status, S_IRUSR | S_IWUSR | S_IRGRP,
    status_show, status_store);

void can_drv_sysfs_add_nodes(struct device *dev)
{
    int ret;
    ret = sysfs_create_file(&dev->kobj, &dev_attr_status.attr);
    if (ret != 0) {
        mttcan_err("%s: sysfs file creation failed (err = %d)\n", __func__, ret);
    }
    ret = sysfs_create_file(&dev->kobj, &dev_attr_latency.attr);
    if (ret != 0) {
        mttcan_err("sysfs file latency creation failed (err = %d)\n", ret);
    }
}

void can_drv_sysfs_remove_nodes(struct device *dev)
{
    sysfs_remove_file(&dev->kobj, &dev_attr_status.attr);
    sysfs_remove_file(&dev->kobj, &dev_attr_latency.attr);
}

#endif
