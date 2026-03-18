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

#include "bbox_ioctl_drv.h"

#include <asm/ioctl.h>
#include <asm/uaccess.h>
#include <linux/cdev.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
#include <linux/device/class.h>
#else
#include <linux/device.h>
#endif

#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/securec.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <linux/delay.h>

#include "bbox_ioctl.h"
#include "common/bbox_common.h"
#include "common/bbox_print.h"
#include "device/bbox_types.h"

#ifdef DEBUG
#define CHECK_FAIL_TIME        5
#else
#define CHECK_FAIL_TIME        5000
#endif

struct dev_manage {
    u32 dev_major;
    dev_class_t *dev_class;
    struct dev_info cdevs[IOCTL_CHANNEL_MAX_NUM];
    spinlock_t lock;
};

static wait_queue_head_t     g_send_queue;
STATIC struct dev_manage     g_bbox_devs_manage;

STATIC s32 bbox_dev_open(struct inode *inode, struct file *file);
STATIC s32 bbox_dev_release(struct inode *inode, struct file *file);
STATIC __poll_t bbox_dev_poll(struct file *file, struct poll_table_struct *wait);
STATIC bbox_long_t bbox_dev_ioctl(struct file *file, u32 cmd, bbox_ioctl_arg_t arg);

static const struct file_operations g_bbox_dev_fops = {
    .owner = THIS_MODULE,
    .open = bbox_dev_open,
    .release = bbox_dev_release,
    .poll = bbox_dev_poll,
    .unlocked_ioctl = bbox_dev_ioctl,
};

static inline u32 bbox_get_cdev_id(u32 devid, u32 type)
{
    const u32 base = 2;
    return ((devid * base) + ((type == IOCTL_PKT_UP) ? 0U : 1U));
}

/**
 * @brief       : check whether the given file pointer is recorded
 * @param [in]  : struct file *file        file info structure
 * @return      : true or false
 */
STATIC bool bbox_filep_is_recored(const struct file *filep)
{
    s32 i;
    const struct dev_info *cur = NULL;

    BB_CHECK_PTR(filep, return false, "invalid param, filep is NULL.\n");

    spin_lock(&g_bbox_devs_manage.lock);
    for (i = 0; i < (s32)IOCTL_CHANNEL_MAX_NUM; i++) {
        cur = &g_bbox_devs_manage.cdevs[i];
        if (cur->accepted_filep == filep) {
            spin_unlock(&g_bbox_devs_manage.lock);
            return true;
        }
    }
    spin_unlock(&g_bbox_devs_manage.lock);

    return false;
}

/**
 * @brief       : notify user poll func there's new event happened
 * @return      : NA
 */
void bbox_dev_ioctl_wakeup_poll(void)
{
    wake_up_interruptible(&g_send_queue);
}

/**
 * @brief       : Called when a process open the device file
 * @param [in]  : struct inode *inode      char dev inode info
 * @param [in]  : struct file *file        file info structure
 * @return      : success ==0; fail: minus errno, -EINVAL
 */
STATIC s32 bbox_dev_open(struct inode *inode, struct file *file)
{
    BB_CHECK_PTR(file, return -EINVAL, "invalid param, file is NULL.\n");

    UNUSED(inode);
    if ((file->f_flags & (u32)CDEV_OPEN_FLAG) == 0) {
        BB_PRINT_ERR("try to open dev with unsupported mode.\n");
        return -EINVAL;
    }
    return EOK;
}

/**
 * @brief       : Called when a process closes the device file
 * @param [in]  : struct inode *inode      char dev inode info
 * @param [in]  : struct file *file        file info structure
 * @return      : success ==0; fail: minus errno, -EINVAL
 */
STATIC s32 bbox_dev_release(struct inode *inode, struct file *file)
{
    struct ioctl_channel_info *info = NULL;

    BB_CHECK_PTR(file, return -EINVAL, "invalid param, file is NULL.\n");
#ifndef BBOX_UT
    BB_PRINT_INFO("<%s:%d,%d> Bbox release\n", current->comm, current->tgid, current->pid);
#endif
    UNUSED(inode);
    if (!bbox_filep_is_recored(file)) {
        return EOK; // unaccepted dev closed
    }

    info = (struct ioctl_channel_info *)file->private_data;
    if (info == NULL) {
        return EOK;
    }

    file->private_data = NULL;
    mutex_lock(&info->lock);

    if (info->dev != NULL) {
        info->dev->accepted_filep = NULL;
    }
    info->dev = NULL;
    // recv status should be cleared, if connection broken
    info->recv_node.clear_flag = true;
    info->status = WAIT_CONNECTION;
    sema_init(&info->recv_node.sem, 0);
    info->magic = OPEN_CHAR_DEV_MAGIC;
    mutex_unlock(&info->lock);
    return EOK;
}

/**
 * @brief       : char device status poll function when a process called poll/epoll/select
 * @param [in]  : struct file *file                 file info structure
 * @param [in]  : struct poll_table_struct *wait    poll process table
 * @return      : success >=0; fail: ==0
 */
STATIC __poll_t bbox_dev_poll(struct file *file, struct poll_table_struct *wait)
{
    __poll_t mask = 0;
    const struct ioctl_channel_info *info = NULL;

    BB_CHECK_PTR(file, return mask, "invalid param, file is NULL.\n");

    poll_wait(file, &g_send_queue, wait);
    if (file->private_data == NULL) {
        mask |= (__poll_t)BBOX_EPOLL_CONN_IN;
        return mask;
    }

#ifndef BBOX_UT
    if (!bbox_filep_is_recored(file)) {
        BB_PRINT_WARN("anonymous dev filep access in poll\n");
        return mask;
    }
#endif
    info = (struct ioctl_channel_info *)file->private_data;

    if (info->status == INITIALISED) {
        mask |= (__poll_t)BBOX_EPOLL_HANG_UP;
    } else if ((info->status == WAIT_CONNECTION) || (info->status == FIRST_WAIT_CONN)) {
        mask |= (__poll_t)BBOX_EPOLL_CONN_IN;
    } else if (!info->send_node.clear_flag) {
        mask |= (__poll_t)BBOX_EPOLL_DATA_IN;
    }

    return mask;
}

/**
 * @brief       : autherise identification, recorded filep register cdev.
 *                caller guarantee all pointers not be NULL.
 *                cmd_arg.devid & cmd_arg.type was guaranteed in bbox_dev_ioctl
 * @param [in]  : struct file *file                 file struct pointer
 * @param [in]  : struct ioctl_channel_info *info   channel info
 * @param [in]  : struct ioctl_para *cmd_arg        user given arguments
 * @param [in]  : void *in_arg                      user arg add in user spaces
 * @return      : success ==0; fail: minus errno
 */
STATIC s32 bbox_dev_register(struct file *file, struct ioctl_channel_info *info,
                             struct ioctl_para *cmd_arg, void *in_arg)
{
    u32 id;
    bbox_byte_t uncp_bytes;
    u32 new_magic = BBOX_GEN_COMMU_ID;
    struct dev_info *dev_info = NULL;
    if ((info->status != WAIT_CONNECTION) && (info->status != FIRST_WAIT_CONN)) {
        BB_PRINT_ERR("related channel is not waiting connection.\n");
        return -EINVAL;
    }

    id = bbox_get_cdev_id(cmd_arg->devid, cmd_arg->type);
    BB_CHECK_EXP_ACT(id >= IOCTL_CHANNEL_MAX_NUM, return -EINVAL, "channel id exceed range.\n");

    cmd_arg->magic = new_magic;
    uncp_bytes = copy_to_user(in_arg, cmd_arg, sizeof(struct ioctl_para));
    BB_CHECK_EXP_ACT(uncp_bytes != 0, return -EFAULT, "ioctl copy register info to user failed.\n");

    spin_lock(&g_bbox_devs_manage.lock);
    dev_info = &g_bbox_devs_manage.cdevs[id];
    dev_info->accepted_filep = file;
    spin_unlock(&g_bbox_devs_manage.lock);

    mutex_lock(&info->lock);
    info->dev = dev_info;
    info->magic = new_magic;
    info->status = (info->status == FIRST_WAIT_CONN) ? CONNECTED : RECONNECTED;
    file->private_data = info; // record channel info
    mutex_unlock(&info->lock);
    return EOK;
}

/**
 * @brief       : get size of next data node which wait sending to user spaces
 *                caller guarantee all pointers not be NULL
 * @param [in]  : struct ioctl_channel_info *info   channel info
 * @param [in]  : struct ioctl_para *cmd_arg        user given arguments
 * @param [in]  : void *in_arg                      user arg add in user spaces
 * @return      : success ==0; fail: minus errno
 */
STATIC s32 bbox_dev_get_pending_data_size(const struct ioctl_channel_info *info,
                                          struct ioctl_para *cmd_arg, void *in_arg)
{
    bbox_byte_t uncp_bytes;

    BB_CHECK_EXP_ACT(info->status != CONNECTED, return -EINVAL, "current channel is not connected.\n");
    BB_CHECK_EXP_ACT(info->send_node.clear_flag, return -EFAULT, "no data is waiting for user receiving.\n");

    cmd_arg->len = info->send_node.len;
    uncp_bytes = copy_to_user(in_arg, cmd_arg, sizeof(struct ioctl_para));
    BB_CHECK_EXP_ACT(uncp_bytes != 0, return -EFAULT, "ioctl copy sending data size to user failed.\n");

    return EOK;
}

/**
 * @brief       : copy send data which wait sending to user spaces, then notify channel
 *                caller guarantee all pointers not be NULL
 * @param [in]  : struct ioctl_channel_info *info   channel info
 * @param [in]  : struct ioctl_para *cmd_arg        user given arguments
 * @return      : success ==0; fail: minus errno
 */
STATIC s32 bbox_dev_ioctl_to_user(struct ioctl_channel_info *info, const struct ioctl_para *cmd_arg)
{
    bbox_byte_t uncp_bytes;
    char *data = NULL;
    u32 len;
    s32 ret;

    BB_CHECK_EXP_ACT(info->status != CONNECTED, return -EINVAL, "current send channel is not connected.\n");
    BB_CHECK_PTR(cmd_arg->buf, return -EFAULT, "user buffer address NULL.\n");
    BB_CHECK_EXP_ACT(cmd_arg->len == 0, return -EINVAL, "zero data length.\n");

    data = (char *)bbox_vmalloc(BBOX_CHANNEL_CAPACITY);
    if (data == NULL) {
        BB_PRINT_ERR("bbox vmalloc data failed.\n");
        return -ENOMEM;
    }

    spin_lock(&info->send_node.lock);
    if (info->send_node.clear_flag) {
        spin_unlock(&info->send_node.lock);
        bbox_vfree(data);
        BB_PRINT_ERR("no data is waiting for sending.\n");
        return -EFAULT;
    }

    if (cmd_arg->len != info->send_node.len) {
        spin_unlock(&info->send_node.lock);
        bbox_vfree(data);
        BB_PRINT_ERR("bad user recv buffer length.\n");
        return -EFAULT;
    }

    ret = memcpy_s(data, BBOX_CHANNEL_CAPACITY, info->send_node.data, info->send_node.len);
    if (ret != EOK) {
        spin_unlock(&info->send_node.lock);
        bbox_vfree(data);
        BB_PRINT_ERR("copy send data failed.\n");
        return -EFAULT;
    }

    len = info->send_node.len;
    info->send_node.clear_flag = true;
    spin_unlock(&info->send_node.lock);

    uncp_bytes = copy_to_user(cmd_arg->buf, data, len);
    if (uncp_bytes != 0) {
        bbox_vfree(data);
        BB_PRINT_ERR("ioctl copy sending data to user failed.\n");
        return -EFAULT;
    }
    up(&info->send_node.sem);

    bbox_vfree(data);
    return EOK;
}

/**
 * @brief       : copy user spaces sending data to channel, then notify channel
 *                caller guarantee all pointers not be NULL
 * @param [in]  : struct ioctl_channel_info *info   channel info
 * @param [in]  : struct ioctl_para *cmd_arg        user given arguments
 * @return      : success ==0; fail: minus errno
 */
STATIC s32 bbox_dev_ioctl_from_user(struct ioctl_channel_info *info, const struct ioctl_para *cmd_arg)
{
    bbox_byte_t uncp_bytes;
    char *data = NULL;
    s32 ret;

    BB_CHECK_EXP_ACT((info->status != CONNECTED) && (info->status != RECONNECTED), return -EINVAL,
        "current recv channel is not connected.\n");
    BB_CHECK_PTR(cmd_arg->buf, return -EFAULT, "user buffer address NULL.\n");
    BB_CHECK_EXP_ACT(cmd_arg->len == 0, return -EINVAL, "zero data length.\n");
    BB_CHECK_EXP_ACT(cmd_arg->len > IOCTL_SEGMENT, return -EINVAL, "data len exceed capacity.\n");

    data = (char *)bbox_vmalloc(BBOX_CHANNEL_CAPACITY);
    if (data == NULL) {
        BB_PRINT_ERR("bbox vmalloc data failed.\n");
        return -ENOMEM;
    }

    uncp_bytes = copy_from_user(data, cmd_arg->buf, cmd_arg->len);
    if (uncp_bytes != 0) {
        bbox_vfree(data);
        BB_PRINT_ERR("ioctl copy recving data from user failed.\n");
        return -EFAULT;
    }

    spin_lock(&info->recv_node.lock);
    if (!info->recv_node.clear_flag) {
        spin_unlock(&info->recv_node.lock);
        bbox_vfree(data);
        BB_PRINT_ERR("previous received data not process yet.\n");
        return -EAGAIN; // notify user process to send again
    }

    ret = memcpy_s(info->recv_node.data, BBOX_CHANNEL_CAPACITY, data, cmd_arg->len);
    if (ret != EOK) {
        spin_unlock(&info->recv_node.lock);
        bbox_vfree(data);
        BB_PRINT_ERR("copy recv data failed.\n");
        return -EFAULT;
    }
    info->recv_node.len = cmd_arg->len;
    info->recv_node.clear_flag = false;
    up(&info->recv_node.sem);
    spin_unlock(&info->recv_node.lock);
    bbox_vfree(data);
    return EOK;
}

/**
 * @brief       : choose ioctl function for given cmd
 * @param [in]  : struct file *file                 file struct pointer
 * @param [in]  : u32 in_cmd                        user given ioctl command
 * @param [in]  : struct ioctl_channel_info *info   channel info
 * @param [in]  : struct ioctl_para *cmd_arg        user given arguments
 * @param [in]  : void *in_arg                      user arg add in user spaces
 * @return      : success ==0; fail: minus errno
 */
STATIC s32 bbox_dev_ioctl_cmd_choose(struct file *file, u32 in_cmd, struct ioctl_channel_info *info,
                                     struct ioctl_para *cmd_arg, void *in_arg)
{
    s32 ret;
    switch (in_cmd) {
        case IOCTL_USER_REGISTER_DEV:
            ret = bbox_dev_register(file, info, cmd_arg, in_arg);
            break;
        case IOCTL_USER_QUERY_READ_INFO:
            ret = bbox_dev_get_pending_data_size(info, cmd_arg, in_arg);
            break;
        case IOCTL_USER_READ:
            ret = bbox_dev_ioctl_to_user(info, cmd_arg);
            break;
        case IOCTL_USER_WRITE:
            ret = bbox_dev_ioctl_from_user(info, cmd_arg);
            break;
        default:
            BB_PRINT_ERR("invalid cmd.\n");
            ret = -EINVAL;
            break;
    }
    return ret;
}

/**
 * @brief       : convert user cmd to magic
 * @param [in]  : u32 cmd                  user given commands
 * @return      : u32 magic
 */
static inline u32 bbox_dev_ioctl_cmd_to_magic(u32 cmd)
{
    return (cmd | (u32)(0xFU << _IOC_NRSHIFT));
}

/**
 * @brief       : char device status ioctl function when a process called ioctl
 * @param [in]  : struct file *file        file info structure
 * @param [in]  : u32 cmd                  user given commands
 * @param [in]  : bbox_ioctl_arg_t arg     user given argument in number or addr of argmuents' pointer
 * @return      : success ==0; fail: minus errno
 */
STATIC bbox_long_t bbox_dev_ioctl(struct file *file, u32 cmd, bbox_ioctl_arg_t arg)
{
    struct ioctl_para cmd_arg = {0};
    void *in_arg = NULL;
    struct ioctl_channel_info *info = NULL;
    u32 in_cmd, sid;
    bbox_byte_t uncp_bytes;
    s32 ret;

    BB_CHECK_PTR(file, return -EINVAL, "invalid param, file is NULL.\n");
    BB_CHECK_PTR((void *)(uintptr_t)arg, return -EINVAL, "invalid param, arg is NULL.\n");

    if (bbox_dev_ioctl_cmd_to_magic(cmd) != BBOX_IOCTL_CMD_MAGIC) {
        BB_PRINT_ERR("invalid cmd pattern.\n");
        msleep(CHECK_FAIL_TIME);
        return -EINVAL;
    }

    in_cmd = _IOC_NR(cmd);
    if (in_cmd != (u32)IOCTL_USER_REGISTER_DEV) {
        BB_CHECK_EXP_ACT(!bbox_filep_is_recored(file), return -EPERM, "anonymous dev filep access.\n");
    }

    in_arg = (void *)(uintptr_t)arg;
    uncp_bytes = copy_from_user(&cmd_arg, in_arg, sizeof(struct ioctl_para));
    BB_CHECK_EXP_ACT(uncp_bytes != 0, return -EFAULT, "ioctl copy_from_user failed.\n");

    info = bbox_ioctl_get_channel(cmd_arg.devid, cmd_arg.type, &sid);
    if ((info == NULL) || (info->magic != cmd_arg.magic)) {
        BB_PRINT_ERR("invalid authentication head info.\n");
        msleep(CHECK_FAIL_TIME);
        return -EPERM;
    }

    ret = bbox_dev_ioctl_cmd_choose(file, in_cmd, info, &cmd_arg, in_arg);
    BB_CHECK_EXP_ACT(ret != EOK, return ret, "ioctl execute cmd failed.\n");

    return EOK;
}

/**
 * @brief       : destroy ioctl character devices
 * @param [in]  : u32 devid     character device id
 * @return      : NA
 */
STATIC void bbox_dev_ioctl_device_destroy(u32 devid)
{
    struct dev_info *dev = NULL;

    BB_CHECK_EXP_ACT(devid >= IOCTL_CHANNEL_MAX_NUM, return, "invalid param, devid is %u.\n", devid);
    dev = &g_bbox_devs_manage.cdevs[devid];
    cdev_del(&dev->cdev);
    device_destroy(g_bbox_devs_manage.dev_class, dev->devno);
}

/**
 * @brief       : create ioctl character devices
 * @param [in]  : u32 devid     character device id
 * @return      : !=0 failure; ==0 success
 */
STATIC s32 bbox_dev_ioctl_device_create(u32 devid)
{
    s32 ret;
    char cdev_name[CDEV_NAME_LEN] = {0};
    const struct device *dev_node = NULL;
    struct dev_info *dev = NULL;

    BB_CHECK_EXP_ACT(devid >= IOCTL_CHANNEL_MAX_NUM, return BBOX_FAILURE, "invalid param, devid is %u.\n", devid);

    dev = &g_bbox_devs_manage.cdevs[devid];
    dev->accepted_filep = NULL;
    dev->devno = MKDEV(g_bbox_devs_manage.dev_major, (u32)devid);
    ret = sprintf_s(cdev_name, CDEV_NAME_LEN, "%s%u", CHAR_DRIVER_NAME, devid);
    BB_CHECK_SPRINTF(ret, return BBOX_FAILURE);

    cdev_init(&dev->cdev, &g_bbox_dev_fops);
    ret = cdev_add(&dev->cdev, dev->devno, 1);
    BB_CHECK_EXP_ACT(ret != 0, return BBOX_FAILURE, "cdev_add(%u) failed with %d.\n", devid, ret);

    dev_node = device_create(g_bbox_devs_manage.dev_class, NULL, dev->devno, NULL, "%s", cdev_name);
    if (IS_ERR(dev_node)) {
        cdev_del(&dev->cdev);
        BB_PRINT_ERR("device_create(%u) failed with %d.\n", devid, ret);
        return BBOX_FAILURE;
    }
    return BBOX_SUCCESS;
}

/**
 * @brief       : init ioctl character devices
 * @param [in]  : int num               channel num
 * @return      : !=0 failure; ==0 success
 */
s32 bbox_dev_ioctl_init(u32 num)
{
    u32 i, j;
    dev_t devno;
    s32 ret = alloc_chrdev_region(&devno, 0, num, CHAR_DRIVER_NAME);
    BB_CHECK_EXP_ACT(ret != 0, return BBOX_FAILURE, "alloc_chrdev_region failed.\n");

    g_bbox_devs_manage.dev_major = MAJOR(devno);
    g_bbox_devs_manage.dev_class = class_create(THIS_MODULE, CHAR_DRIVER_NAME);
    if (IS_ERR(g_bbox_devs_manage.dev_class)) {
        BB_PRINT_ERR("class_create failed.\n");
        unregister_chrdev_region(MKDEV(g_bbox_devs_manage.dev_major, 0), num);
        g_bbox_devs_manage.dev_major = 0;
        return BBOX_FAILURE;
    }

    for (i = 0; i < num; i++) {
        ret = bbox_dev_ioctl_device_create(i);
        if (ret == BBOX_SUCCESS) {
            continue;
        }

        for (j = 0; j < i; j++) {
            bbox_dev_ioctl_device_destroy(j);
        }
        class_destroy(g_bbox_devs_manage.dev_class);
        g_bbox_devs_manage.dev_class = NULL;
        unregister_chrdev_region(MKDEV(g_bbox_devs_manage.dev_major, 0), num);
        g_bbox_devs_manage.dev_major = 0;
        return BBOX_FAILURE;
    }
    spin_lock_init(&g_bbox_devs_manage.lock);
    init_waitqueue_head(&g_send_queue);
    return BBOX_SUCCESS;
}

/**
 * @brief       : exit, release ioctl character device resources
 * @return      : NA
 */
void bbox_dev_ioctl_exit(u32 num)
{
    u32 i;

    if (num > IOCTL_CHANNEL_MAX_NUM) {
        num = IOCTL_CHANNEL_MAX_NUM;
    }

    if (g_bbox_devs_manage.dev_major == 0) {
        return;
    }

    for (i = 0; i < num; i++) {
        bbox_dev_ioctl_device_destroy(i);
    }
    class_destroy(g_bbox_devs_manage.dev_class);
    g_bbox_devs_manage.dev_class = NULL;
    unregister_chrdev_region(MKDEV(g_bbox_devs_manage.dev_major, 0), num);
    g_bbox_devs_manage.dev_major = 0;
}
