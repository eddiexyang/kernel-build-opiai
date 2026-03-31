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


#include <linux/types.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/securec.h>
#include <linux/securectype.h>

#ifndef DEVMNG_UT
#include "devdrv_manager.h"
#include "devdrv_manager_container.h"
#include "devdrv_manager_common.h"
#include "devdrv_user_common.h"
#include "devdrv_common.h"
#include "devdrv_manager_container_noplugin.h"

int devdrv_is_in_container(void)
{
    u64 current_mnt;
    u64 host_mnt;
    int is_in;
    int ret;

    ret = devdrv_manager_get_current_mnt_ns(&current_mnt);
    if (ret) {
        devdrv_drv_err("get current mnt ns error, ret = %d\n", ret);
        return -EINVAL;
    }
    host_mnt = devdrv_manager_get_host_mnt_ns();
    if (host_mnt == 0)
        is_in = false;
    else if (!devdrv_manager_container_is_admin() && current_mnt != host_mnt)
        is_in = true;
    else
        is_in = false;

    return is_in;
}

int devdrv_manager_container_is_in_container(void)
{
    return devdrv_is_in_container();
}
EXPORT_SYMBOL_GPL(devdrv_manager_container_is_in_container);

int devdrv_virtual_id_to_physical_id(u32 virtual_id, u32 *physical_id)
{
    return 0;
}

STATIC int devdrv_manager_container_send_para(struct devdrv_info *dev_info, u16 group_id, u16 tflops)
{
    struct devdrv_container_mailbox_msg mail_msg;
    u32 tsid = 0;
    int result;
    int ret;
    u32 len;

    devdrv_drv_info("send container info, group_id: %d, tflops: %d.\n", group_id, tflops);

    mail_msg.header.valid = DEVDRV_MAILBOX_MESSAGE_VALID;
    mail_msg.header.cmd_type = DEVDRV_MAILBOX_SEND_CONTAINER_TFLOP;
    mail_msg.header.result = 0;

    mail_msg.group_id = group_id;
    mail_msg.tflops = tflops;

    mail_msg.plat_type = dev_info->plat_type;

    len = sizeof(struct devdrv_container_mailbox_msg);
    if (dev_info->drv_ops->mailbox_kernel_sync_no_feedback == NULL) {
        devdrv_drv_err("devdrv_mailbox_kernel_sync_no_feedback is null.\n");
        return -1;
    }
    ret = dev_info->drv_ops->mailbox_kernel_sync_no_feedback(dev_info->dev_id, tsid, (u8 *)&mail_msg, len, &result);
    if (ret || result) {
        devdrv_drv_err("devdrv_mailbox_kernel_sync_no_feedback return error: ret(%d), result(%d).\n", ret, result);
        return -1;
    }

    return 0;
}

STATIC int devdrv_manager_container_para_check(struct devdrv_info *dev_info)
{
    u32 unit;

    if (dev_info == NULL || dev_info->container.total_tflop > DEVDRV_MINI_TOTAL_TFLOP) {
        devdrv_drv_err("total tflop %d is bigger than max %d.\n",
            (dev_info == NULL) ? 0 : dev_info->container.total_tflop, DEVDRV_MINI_TOTAL_TFLOP);
        return -1;
    }

    if (dev_info->container.tflop_mode == DEVDRV_FP16) {
        unit = DEVDRV_MINI_FP16_UNIT;
    } else if (dev_info->container.tflop_mode == DEVDRV_INT8) {
        unit = DEVDRV_MINI_INT8_UNIT;
    } else {
        devdrv_drv_err("tflop mode %d is not supported.\n", dev_info->container.tflop_mode);
        return -1;
    }

    if (unit != dev_info->container.alloc_unit) {
        devdrv_drv_err("tflop unit %d is invalid.\n", dev_info->container.alloc_unit);
        return -1;
    }

    if (dev_info->container.tflop_num != (dev_info->container.total_tflop / unit)) {
        devdrv_drv_err("tflop num %d is invalid.\n", dev_info->container.tflop_num);
        return -1;
    }

    return 0;
}

STATIC int devdrv_manager_container_init(struct devdrv_info *dev_info)
{
    struct devdrv_container_tflops_table *table = NULL;
    u32 size;
    int ret;

    ret = devdrv_manager_container_para_check(dev_info);
    if (ret) {
        devdrv_drv_err("devdrv_manager_container_para_check return error: %d.\n", ret);
        return -1;
    }

    size = sizeof(struct devdrv_container_tflops_table);
    dev_info->container.alloc_table = kzalloc(size, GFP_KERNEL | __GFP_ACCOUNT);
    if (dev_info->container.alloc_table == NULL) {
        devdrv_drv_err("alloc memory for tflop table failed.\n");
        return -1;
    }

    table = (struct devdrv_container_tflops_table *)dev_info->container.alloc_table;
    table->entry_num = 0;
    table->total_tflops = 0;
    table->max_entry = dev_info->container.tflop_num;
    table->max_tflops = dev_info->container.total_tflop;
    table->group_id_generater = 0;

    return 0;
}

STATIC void devdrv_manager_container_exit(struct devdrv_info *dev_info)
{
    int ret;
    if (dev_info->container.alloc_table != NULL) {
        kfree(dev_info->container.alloc_table);
        dev_info->container.alloc_table = NULL;
    }

    dev_info->run_mode = DEVDRV_NORMAL_MODE;
    ret = memset_s(&dev_info->container, sizeof(struct devdrv_container_info), 0, sizeof(struct devdrv_container_info));
    if (ret)
        devdrv_drv_err("set dev_info->containe to 0 failed.\n");
}

STATIC int devdrv_uuid_compare(u8 *id1, u8 *id2, int size)
{
    int i;

    for (i = 0; i < size; i++) {
        if (id1[i] != id2[i]) {
            return 1;
        }
    }

    return 0;
}

STATIC void devdrv_get_uuid_str(u8 uuid[], int uuid_size, char *str, int str_size)
{
    int i;
    int tlen;

    if ((uuid_size > (INT_MAX / 2)) || (str_size <= (uuid_size * 2))) {
        devdrv_drv_err("param size invalid. (str_size=%d; uuid_size=%d)\n", str_size, uuid_size);
        return;
    }

    for (i = 0; i < uuid_size; i++) {
        tlen = sprintf_s(&str[(long)i * 2], str_size - (i * 2), "%02x", uuid[i]);
        if (tlen < 0) {
            devdrv_drv_err("devdrv_get_uuid_str:%02x snprintf_s fail\n", uuid[i]);
            return;
        }
    }
}

STATIC int devdrv_manager_container_erase_released(struct devdrv_info *dev_info)
{
    struct devdrv_container_tflops_table *table = NULL;
    int ret;
    u32 i;

    if (dev_info == NULL || dev_info->run_mode != DEVDRV_CONTAINER_MODE) {
        devdrv_drv_err("device info (%pK),device %d(device id) is not in container mode.\n", dev_info,
            (dev_info == NULL) ? 0 : dev_info->dev_id);
        return -1;
    }

    table = (struct devdrv_container_tflops_table *)dev_info->container.alloc_table;

    if (table->entry_num > DEVDRV_MINI_TOTAL_TFLOP) {
        devdrv_drv_err("table entry num %d is invalid.\n", table->entry_num);
        return -1;
    }

    for (i = 0; i < table->entry_num;) {
        if (table->entry[i].exited) {
            ret = devdrv_manager_container_send_para(dev_info, table->entry[i].group_id, 0);
            if (ret) {
                devdrv_drv_err("devdrv_manager_container_send_para return error: %d.\n", ret);
                return -1;
            }

            table->total_tflops -= table->entry[i].tflops;
            ret = memcpy_s(&table->entry[i], sizeof(struct devdrv_container_tflops_entry),
                &table->entry[table->entry_num - 1], sizeof(struct devdrv_container_tflops_entry));
            if (ret) {
                devdrv_drv_err("copy table->entry[table->entry_num - 1] to table->entry[i] failed.\n");
                return ret;
            }
            table->entry_num--;
        } else {
            i++;
        }
    }

    return 0;
}

STATIC int(*const devdrv_manager_container_process_handler[DEVDRV_CONTAINER_MAX_CMD])(struct devdrv_info *dev_info,
    struct devdrv_container_msg *msg) = {
        [DEVDRV_CONTAINER_NOTIFY] = NULL,
        [DEVDRV_CONTAINER_ALLOCATE_TFLOPS] = NULL,
        [DEVDRV_CONTAINER_IS_CONTAINER] = NULL,
        [DEVDRV_CONTAINER_DOCKER_EXIT] = NULL,
        [DEVDRV_CONTAINER_DOCKER_CREATE] = NULL,
        [DEVDRV_CONTAINER_ASSIGN_NOTIFY] = NULL,
        [DEVDRV_CONTAINER_ASSIGN_ALLOCATE_DEVICES] = NULL,
        [DEVDRV_CONTAINER_ASSIGN_IS_ASSIGN_MODE] = NULL,
        [DEVDRV_CONTAINER_ASSIGN_SET_UUID] = NULL,
        [DEVDRV_CONTAINER_IS_IN_CONTAINER] = NULL,
};

int devdrv_manager_h2d_container(u32 devid, void *msg, u32 in_len, u32 *ack_len)
{
    struct devdrv_manager_msg_info *dev_manager_msg_info = NULL;
    struct devdrv_container_msg *container_msg = NULL;
    struct devdrv_info *dev_info = NULL;
    int ret;

    dev_manager_msg_info = (struct devdrv_manager_msg_info *)msg;
    if ((dev_manager_msg_info->header.valid != DEVDRV_MANAGER_MSG_VALID) ||
        (in_len < sizeof(struct devdrv_manager_msg_info))) {
        devdrv_drv_err("Invalid message from host. (valid=%u; in_len=%u)\n",
                       dev_manager_msg_info->header.valid, in_len);
        return -EINVAL;
    }

    dev_manager_msg_info->header.result = 0;
    *ack_len = sizeof(*dev_manager_msg_info);

    dev_info = devdrv_manager_get_devdrv_info(0);
    if (dev_info == NULL) {
        devdrv_drv_err("device is not exists.\n");
        return -EINVAL;
    }

    container_msg = (struct devdrv_container_msg *)dev_manager_msg_info->payload;
    if (container_msg->cmd >= DEVDRV_CONTAINER_MAX_CMD) {
        devdrv_drv_err("invalid cmd: %d.\n", container_msg->cmd);
        return -EINVAL;
    }

    if (devdrv_manager_container_process_handler[container_msg->cmd] == NULL) {
        devdrv_drv_err("not supported cmd: %d.\n", container_msg->cmd);
        return -EINVAL;
    }

    ret = devdrv_manager_container_process_handler[container_msg->cmd](dev_info, container_msg);
    if (ret) {
        devdrv_drv_err("process cmd: %d failed.\n", container_msg->cmd);
        return -EINVAL;
    }

    return 0;
}

/*
 * check if uuid is already exited
 */
STATIC int devdrv_manager_container_check_repeat(struct devdrv_info *dev_info, u8 uuid[], int uuid_size)
{
    struct devdrv_container_tflops_table *table = NULL;
    int ret;
    u32 i;

    if (dev_info == NULL || dev_info->run_mode != DEVDRV_CONTAINER_MODE || uuid_size < DEVDRV_MANAGER_UUID_NUM) {
        devdrv_drv_err("device %d(device id) uuid_size %d is not in container mode.\n",
            (dev_info == NULL) ? 0 : dev_info->dev_id, uuid_size);
        return -1;
    }

    table = (struct devdrv_container_tflops_table *)dev_info->container.alloc_table;

    if (table->entry_num > DEVDRV_MINI_TOTAL_TFLOP) {
        devdrv_drv_err("table entry num %d is invalid.\n", table->entry_num);
        return -1;
    }

    for (i = 0; i < table->entry_num; i++) {
        if (devdrv_uuid_compare(uuid, table->entry[i].uuid, DEVDRV_MANAGER_UUID_NUM) == 0) {
            devdrv_drv_info("uuid is already in container tflops table.\n");
            if (table->entry[i].exited) {
                devdrv_drv_err("uuid is already exited.\n");

                ret = devdrv_manager_container_erase_released(dev_info);
                if (ret) {
                    devdrv_drv_err("devdrv_manager_container_erase_released return error: %d.\n", ret);
                }

                return -1;
            }
        }
    }

    return 0;
}

/* register new uuid */
int devdrv_manager_container_register_session(u32 devid, u8 uuid[], int uuid_size, u32 tflops)
{
    struct devdrv_container_tflops_entry *entry = NULL;
    struct devdrv_container_tflops_table *table = NULL;
    struct devdrv_info *dev_info = NULL;
    char str[128];
    u32 remain;
    int ret;
    u32 i;

    if (uuid == NULL || uuid_size < DEVDRV_MANAGER_UUID_NUM) {
        devdrv_drv_err("input uuid array is null,size %d.\n", uuid_size);
        return -1;
    }

    devdrv_get_uuid_str(uuid, DEVDRV_MANAGER_UUID_NUM, str, 128 * sizeof(char));
    devdrv_drv_info("devid: %d, tflops: %d, uuid: %s.\n", devid, tflops, str);

    dev_info = devdrv_manager_get_devdrv_info(0);
    if (dev_info == NULL) {
        devdrv_drv_err("device is not exists.\n");
        return -1;
    }

    mutex_lock(&dev_info->container.lock);

    if (dev_info->run_mode != DEVDRV_CONTAINER_MODE) {
        mutex_unlock(&dev_info->container.lock);
        devdrv_drv_err("device [%d(host id),%d(device id)] is not in "
            "container mode.\n",
            devid, dev_info->dev_id);
        return -1;
    }

    if (dev_info->container.alloc_table == NULL) {
        ret = devdrv_manager_container_init(dev_info);
        if (ret) {
            devdrv_manager_container_exit(dev_info);
            mutex_unlock(&dev_info->container.lock);
            devdrv_drv_err("device [%d(host id),%d(device id)] init container "
                "tflops allocation table failed.\n",
                devid, dev_info->dev_id);
            return -1;
        }
    }

    ret = devdrv_manager_container_check_repeat(dev_info, uuid, uuid_size);
    if (ret) {
        mutex_unlock(&dev_info->container.lock);
        devdrv_drv_err("device [%d(host id),%d(device id)]'s uuid is already exited.\n", devid, dev_info->dev_id);
        return -1;
    }

    ret = devdrv_manager_container_erase_released(dev_info);
    if (ret) {
        mutex_unlock(&dev_info->container.lock);
        devdrv_drv_err("devdrv_manager_container_erase_released return error: %d.\n", ret);
        return -1;
    }

    table = (struct devdrv_container_tflops_table *)dev_info->container.alloc_table;

    if (table->entry_num > DEVDRV_MINI_TOTAL_TFLOP) {
        mutex_unlock(&dev_info->container.lock);
        devdrv_drv_err("table entry num %d is invalid.\n", table->entry_num);
        return -1;
    }

    for (i = 0; i < table->entry_num; i++) {
        if (devdrv_uuid_compare(table->entry[i].uuid, uuid, DEVDRV_MANAGER_UUID_NUM) == 0) {
            if (table->entry[i].registered) {
                devdrv_drv_info("device [%d(host id),%d(device id)] is already register.\n", devid, dev_info->dev_id);
                mutex_unlock(&dev_info->container.lock);
                return 0;
            }

            devdrv_drv_info("device [%d(host id),%d(device id)] is in table.\n", devid, dev_info->dev_id);

            remain = table->max_tflops - table->total_tflops;
            if (remain < tflops) {
                mutex_unlock(&dev_info->container.lock);
                devdrv_drv_err("device [%d(host id),%d(device id)] only have %d tflops, "
                    "not enough, which alloc %d.\n",
                    devid, dev_info->dev_id, remain, tflops);
                return -1;
            }

            ret = devdrv_manager_container_send_para(dev_info, table->entry[i].group_id, tflops);
            if (ret) {
                mutex_unlock(&dev_info->container.lock);
                devdrv_drv_err("devdrv_manager_container_send_para return error: %d.\n", ret);
                return -1;
            }

            entry = &table->entry[i];
            entry->tflops = tflops;
            entry->registered = 1;
            table->total_tflops += tflops;

            mutex_unlock(&dev_info->container.lock);
            devdrv_drv_info("new entry, group_id: %d, tflops: %d, current entry_num: %d, current total tflops: %d.\n",
                entry->group_id, entry->tflops, table->entry_num, table->total_tflops);
            return 0;
        }
    }

    mutex_unlock(&dev_info->container.lock);
    devdrv_drv_err("device [%d(host id),%d(device id)] is not in table.\n", devid, dev_info->dev_id);

    return -1;
}
EXPORT_SYMBOL(devdrv_manager_container_register_session);

STATIC int devdrv_manager_container_add_parameter(struct devdrv_info *dev_info, pid_t pid, u16 group_id, u32 tflops)
{
    return 0;
}

/*
 * register process
 */
int devdrv_manager_container_register_process(u32 devid, u8 uuid[], int uuid_size, pid_t pid)
{
    struct devdrv_container_tflops_table *table = NULL;
    struct devdrv_info *dev_info = NULL;
    char str[128];
    int ret;
    u32 i;

    if (uuid == NULL || uuid_size < DEVDRV_MANAGER_UUID_NUM) {
        devdrv_drv_err("input uuid array is null size  %d .\n", uuid_size);
        return -1;
    }

    devdrv_get_uuid_str(uuid, DEVDRV_MANAGER_UUID_NUM, str, 128 * sizeof(char));
    devdrv_drv_info("devid: %d, pid: %d, uuid: %s.\n", devid, pid, str);

    dev_info = devdrv_manager_get_devdrv_info(0);
    if (dev_info == NULL) {
        devdrv_drv_err("device is not exists.\n");
        return -1;
    }

    mutex_lock(&dev_info->container.lock);

    if (dev_info->run_mode != DEVDRV_CONTAINER_MODE) {
        mutex_unlock(&dev_info->container.lock);
        devdrv_drv_err("device [%d(host id),%d(device id)] is not in "
            "container mode.\n",
            devid, dev_info->dev_id);
        return -1;
    }

    if (dev_info->container.alloc_table == NULL) {
        mutex_unlock(&dev_info->container.lock);
        devdrv_drv_err("device [%d(host id),%d(device id)] alloc table "
            "is not inited.\n",
            devid, dev_info->dev_id);
        return -1;
    }

    table = (struct devdrv_container_tflops_table *)dev_info->container.alloc_table;

    if (table->entry_num > DEVDRV_MINI_TOTAL_TFLOP) {
        mutex_unlock(&dev_info->container.lock);
        devdrv_drv_err("table entry num %d is invalid.\n", table->entry_num);
        return -1;
    }

    for (i = 0; i < table->entry_num; i++) {
        if (devdrv_uuid_compare(uuid, table->entry[i].uuid, DEVDRV_MANAGER_UUID_NUM) == 0) {
            ret =
                devdrv_manager_container_add_parameter(dev_info, pid, table->entry[i].group_id, table->entry[i].tflops);
            if (ret) {
                mutex_unlock(&dev_info->container.lock);
                devdrv_drv_err("device [%d(host id),%d(device id)] add parameter "
                    "for context failed.\n",
                    devid, dev_info->dev_id);

                return -1;
            }

            mutex_unlock(&dev_info->container.lock);

            return 0;
        }
    }

    mutex_unlock(&dev_info->container.lock);
    devdrv_drv_err("device [%d(host id),%d(device id)] uuid is not "
        "found in alloc table, please register session is first.\n",
        devid, dev_info->dev_id);

    return -1;
}
EXPORT_SYMBOL(devdrv_manager_container_register_process);

int devdrv_manager_check_running_mode(u32 devid, u32 *mode)
{
    struct devdrv_info *dev_info = NULL;

    if (devid >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("invalid input devid: %d.\n", devid);
        return -1;
    }

    if (mode == NULL) {
        devdrv_drv_err("invalid output argument: handler is null.\n");
        return -1;
    }

    dev_info = devdrv_manager_get_devdrv_info(0);
    if (dev_info == NULL) {
        devdrv_drv_err("device is not exists.\n");
        return -1;
    }

    *mode = dev_info->run_mode;
    devdrv_drv_info("current running mode: %d.\n", *mode);

    return 0;
}
EXPORT_SYMBOL(devdrv_manager_check_running_mode);


STATIC int(*CONST devdrv_manager_container_ioctl_process_handler[DEVDRV_CONTAINER_MAX_CMD])(
    struct devdrv_container_para *cmd) = {
    [DEVDRV_CONTAINER_NOTIFY] = NULL,
    [DEVDRV_CONTAINER_ALLOCATE_TFLOPS] = NULL,
    [DEVDRV_CONTAINER_IS_CONTAINER] = NULL,
    [DEVDRV_CONTAINER_DOCKER_EXIT] = NULL,
    [DEVDRV_CONTAINER_DOCKER_CREATE] = NULL,
    [DEVDRV_CONTAINER_ASSIGN_NOTIFY] = NULL,
    [DEVDRV_CONTAINER_ASSIGN_ALLOCATE_DEVICES] = NULL,
    [DEVDRV_CONTAINER_ASSIGN_IS_ASSIGN_MODE] = NULL,
    [DEVDRV_CONTAINER_ASSIGN_SET_UUID] = NULL,
    [DEVDRV_CONTAINER_IS_IN_CONTAINER] = NULL,
    [DEVDRV_CONTAINER_GET_DAVINCI_DEVLIST] = devdrv_manager_container_get_davinci_devlist,
    [DEVDRV_CONTAINER_GET_BARE_PID] = devdrv_manager_container_get_bare_pid,
    [DEVDRV_CONTAINER_GET_BARE_TGID] = devdrv_manager_container_get_bare_tgid,
};

int devdrv_manager_container_process(struct file *filep, unsigned long arg)
{
    struct devdrv_container_para container_cmd;
    int ret;

    if (!arg || filep == NULL || filep->private_data == NULL) {
        devdrv_drv_err("arg = %lu, filep = %pK\n", arg, filep);
        return -1;
    }

    ret = copy_from_user_safe(&container_cmd, (void *)(uintptr_t)arg, sizeof(struct devdrv_container_para));
    if (ret) {
        devdrv_drv_err("copy_from_user return error: %d.\n", ret);
        return -1;
    }

    if (container_cmd.para.cmd >= DEVDRV_CONTAINER_MAX_CMD) {
        devdrv_drv_err("invalid input container process cmd: %d.\n", container_cmd.para.cmd);
        return -1;
    }

    if (devdrv_manager_container_ioctl_process_handler[container_cmd.para.cmd] == NULL) {
        devdrv_drv_err("not supported cmd: %d.\n", container_cmd.para.cmd);
        return -EINVAL;
    }

    ret = devdrv_manager_container_ioctl_process_handler[container_cmd.para.cmd](&container_cmd);

    return ret;
}
#else
int devdrv_manager_container_process(struct file *filep, unsigned long arg)
{
    return 0;
}
#endif
