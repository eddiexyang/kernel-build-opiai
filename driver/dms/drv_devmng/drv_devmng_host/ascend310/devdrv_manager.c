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

#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/module.h>
#ifndef DEVMNG_UT
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/idr.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/stat.h>
#include <linux/io.h>
#include <linux/pci.h>
#include <linux/vmalloc.h>
#include <linux/cred.h>
#include <linux/pid_namespace.h>
#include <linux/signal.h>
#include <linux/kmod.h>
#include <linux/reboot.h>
#include <linux/version.h>
#include <linux/kallsyms.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
#include <linux/sched/signal.h>
#include <linux/sched/task.h>
#endif

#include "dbl/uda.h"

#include "devdrv_manager_common.h"
#include "devdrv_pm.h"
#include "devdrv_driver_pm.h"
#include "devmng_dms_adapt.h"
#include "devdrv_manager.h"
#include "devdrv_manager_msg.h"
#include "devdrv_platform_resource.h"
#include "devdrv_manager_container.h"
#include "devdrv_manager_container_noplugin.h"
#include "devdrv_device_online.h"
#include "tsdrv_status.h"
#include "devdrv_interface.h"
#include "devdrv_manager_dev_share.h"
#include "davinci_interface.h"
#include "davinci_api.h"
#include "dms_event_distribute.h"
#include "devdrv_manager_comm.h"
#include "dms_hotreset.h"
#ifdef CFG_FEATURE_TIMESYNC
#include "dms_time.h"
#endif

#define DEVMNG_DEV_BOOT_ARG_NUM 2
#define DEVMNG_PHY_ID_LEN 16
#define DEVMNG_DEV_BOOT_INIT_SH "/usr/bin/device_boot_init.sh"


void *dev_manager_no_trasn_chan[DEVDRV_MAX_DAVINCI_NUM];
struct devdrv_manager_info *dev_manager_info;
STATIC struct devdrv_common_msg_client devdrv_manager_common_chan;
STATIC struct devdrv_info *devdrv_info_array[DEVDRV_MAX_DAVINCI_NUM];
STATIC struct rw_semaphore devdrv_ops_sem;
STATIC void devdrv_set_devdrv_info_array(u32 dev_id, struct devdrv_info *dev_info);

STATIC void devdrv_manager_set_no_trans_chan(u32 dev_id, void *no_trans_chan);

extern struct tsdrv_drv_ops devdrv_host_drv_ops;
extern struct task_struct init_task;

static DEFINE_SPINLOCK(devdrv_spinlock);
static DEFINE_MUTEX(devdrv_get_devnum_mutex);
static DEFINE_MUTEX(devdrv_mutex);

#define SET_BIT_64(x, y) ((x) |= ((u64)0x01 << (y)))
#define CLR_BIT_64(x, y) ((x) &= (~((u64)0x01 << (y))))
#define CHECK_BIT_64(x, y) ((x) & ((u64)0x01 << (y)))
#define DEVDRV_INVALID_PHY_ID 0xFFFFFFFF

STATIC void devdrv_manager_release_one_device(struct devdrv_info *dev_info);
STATIC int devdrv_manager_create_one_device(struct devdrv_info *dev_info);

int devmng_get_vdavinci_info(u32 vdev_id, u32 *phy_id, u32 *vfid)
{
    return 0;
}
EXPORT_SYMBOL(devmng_get_vdavinci_info);

int hvdevmng_get_aicore_num(u32 devid, u32 fid, u32 *aicore_num)
{
    return 1;
}
EXPORT_SYMBOL(hvdevmng_get_aicore_num);

void hvdevmng_set_dev_ts_resource(u32 devid, u32 fid, u32 tsid, void *data)
{
    return;
}
EXPORT_SYMBOL(hvdevmng_set_dev_ts_resource);

#ifdef CONFIG_SYSFS
static u32 sysfs_devid = 0;

#define DEVDRV_ATTR_RO(_name) static struct kobj_attribute _name##_attr = __ATTR_RO(_name)

#define DEVDRV_ATTR(_name) static struct kobj_attribute _name##_attr = __ATTR(_name, 0600, _name##_show, _name##_store)

bool devdrv_manager_is_pf_device(unsigned int dev_id)
{
    return true;
}

STATIC int devdrv_manager_check_permission(void)
{
    u32 root;

    root = (u32)(current->cred->euid.val);
    if ((devdrv_is_in_container() == 1) || (root != 0)) {
        devdrv_drv_err("now is in container or not root, just return.\n");
        return -EACCES;
    }

    if (!capable(CAP_SYS_ADMIN)) {
        devdrv_drv_err("permission deny, just return.\n");
        return -EACCES;
    }

    return 0;
}

STATIC int devdrv_manager_check_phy_mach(u32 dev_id)
{
    unsigned int host_flag;
    int ret;

    ret = devdrv_get_host_phy_mach_flag(dev_id, &host_flag);
    if (ret != 0) {
        devdrv_drv_err("devid %d devdrv_get_host_phy_mach_flag return value is error 0x%x\n", dev_id, ret);
        return ret;
    }

    if (host_flag != DEVDRV_HOST_PHY_MACH_FLAG) {
        devdrv_drv_info("devid %d not a phy mach! host_flag = 0x%x\n", dev_id, host_flag);
        return -EPERM;
    }
    return 0;
}

STATIC int devdrv_manager_check_all_phy_mach(void)
{
    u32 dev_num;
    u32 phys_id = 0;
    u32 vfid = 0;
    int i;
    int ret;

    dev_num = devdrv_manager_get_devnum();
    if (dev_num > DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("get invalid devicenum %d.\n", dev_num);
        return -EINVAL;
    }
    for (i = 0; i < dev_num; ++i) {
        ret = devdrv_manager_container_logical_id_to_physical_id(i, &phys_id, &vfid);
        if (ret) {
            devdrv_drv_err("can't transfor virt id %d ret(%d)\n", i, ret);
            return ret;
        }
        ret = devdrv_manager_check_phy_mach(phys_id);
        if (ret) {
            return ret;
        }
    }
    return 0;
}

STATIC ssize_t devdrv_resources_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    u32 result;
    int ret;

    if (buf == NULL) {
        devdrv_drv_err("invalid args(buf is null).\n");
        return count;
    }

    ret = kstrtouint(buf, 10, &result);
    if (ret) {
        devdrv_drv_err("unable to transform input string into devid, "
            "input string: %s",
            buf);
        return count;
    }
    devdrv_drv_info("input devid is: %d.\n", result);

    if (result >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_warn("input devid is too big, reset into 0, input string: %s", buf);
        result = 0;
    }
    sysfs_devid = result;

    return count;
}

STATIC ssize_t devdrv_resources_show_refill_buf(struct devdrv_info *dev_info, char *str)
{
    char *refill_buf = str;
    int tlen;

    tlen = snprintf_s(str, PAGE_SIZE, PAGE_SIZE - 1, "device: %u -avail-\n", dev_info->dev_id);
    if (tlen < 0) {
        devdrv_drv_info("sprintf_s is abnormal. (device_id=%u)\n", dev_info->dev_id);
        return 0;
    }
    str += tlen;

    return str - refill_buf;
}

STATIC ssize_t devdrv_resources_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct devdrv_info *dev_info = NULL;
    char *str = buf;
    int ret;

    if (buf == NULL || sysfs_devid >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("Invalid Parameter. (sysfs_devid=%d; buf_is_null=%d)\n", sysfs_devid, (buf == NULL));
        return 0;
    }

    dev_info = devdrv_manager_get_devdrv_info(sysfs_devid);
    if (dev_info == NULL) {
        devdrv_drv_info("Device is not exist. (sysfs_devid=%d)\n", sysfs_devid);
        return 0;
    }

    ret = devdrv_resources_show_refill_buf(dev_info, str);
    if (!ret) {
        devdrv_drv_info("sprintf_s is abnormal. (sysfs_devid=%d)\n", sysfs_devid);
        return 0;
    }

    return ret;
}
DEVDRV_ATTR(devdrv_resources);

static struct attribute *devdrv_manager_attrs[] = {
    &devdrv_resources_attr.attr,
    NULL,
};

static struct attribute_group devdrv_manager_attr_group = {
    .attrs = devdrv_manager_attrs,
    .name = "devdrv_manager",
};

#endif /* CONFIG_SYSFS */

int copy_from_user_safe(void *to, const void __user *from, unsigned long n)
{
    if (to == NULL || from == NULL || n == 0) {
        devdrv_drv_err("user pointer is NULL.\n");
        return -EINVAL;
    }

    if (copy_from_user(to, (void *)from, n)) {
        return -ENODEV;
    }

    return 0;
}
EXPORT_SYMBOL(copy_from_user_safe);


int copy_to_user_safe(void __user *to, const void *from, unsigned long n)
{
    if (to == NULL || from == NULL || n == 0) {
        devdrv_drv_err("user pointer is NULL.\n");
        return -EINVAL;
    }

    if (copy_to_user(to, (void *)from, n)) {
        return -ENODEV;
    }

    return 0;
}
EXPORT_SYMBOL(copy_to_user_safe);

STATIC int devdrv_manager_trans_and_check_id(u32 virt_id, u32 *phys_id)
{
    int ret;

    if (virt_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("wrong device id, dev_id = %u\n", virt_id);
        return -1;
    }

    ret = devdrv_virtual_id_to_physical_id(virt_id, phys_id);
    if (ret != 0) {
        devdrv_drv_err("can transfer dev id %u ret = %d.\n", virt_id, ret);
        return ret;
    }
    if (*phys_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("wrong device id, dev_id = %u\n", *phys_id);
        return -1;
    }
    if ((dev_manager_info == NULL) || (devdrv_info_array[*phys_id]) == NULL) {
        devdrv_drv_err("device manager is not initialized, dev_id= %u, device manager %pK .\n", *phys_id,
            dev_manager_info);
        return -1;
    }
    if (devdrv_info_array[*phys_id]->status == 1) {
        devdrv_drv_err("device status is 1. (dev_id=%u)\n", *phys_id);
        return -EBUSY;
    }
    return 0;
}

STATIC void devdrv_set_devdrv_info_array(u32 dev_id, struct devdrv_info *dev_info)
{
    if (dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("wrong device id, dev_id = %d\n", dev_id);
        return;
    }
    spin_lock(&devdrv_spinlock);
    devdrv_info_array[dev_id] = dev_info;
    spin_unlock(&devdrv_spinlock);
    return;
}

STATIC struct devdrv_info *devdrv_get_devdrv_info_array(u32 dev_id)
{
    struct devdrv_info *dev_info = NULL;

    if (dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("wrong device id, dev_id = %d\n", dev_id);
        return NULL;
    }

    spin_lock(&devdrv_spinlock);
    dev_info = devdrv_info_array[dev_id];
    spin_unlock(&devdrv_spinlock);

    return dev_info;
}

STATIC void devdrv_manager_first_get_devnum(void)
{
    int total_timeout = DEVDRV_GET_DEVNUM_STARTUP_TIMEOUT;
    int timeout = 0;
    u32 num_dev = 0;
    int stage = 0;
    int pcie_num;

    pcie_num = dev_manager_info->prob_num;
    if (pcie_num <= 0) {
        devdrv_drv_err("pcie report dev_num fail\n");
        return;
    }

    devdrv_drv_info("pcie report dev_num = %d\n", pcie_num);

    do {
        num_dev = devdrv_manager_container_get_devnum_ns(devdrv_manager_get_host_mnt_ns());
        if (pcie_num == num_dev) {
            devdrv_drv_info("get devnum succ, cost time %d(s) in stage %d \n", timeout, stage);
            break;
        }

        ssleep(1);
        timeout++;
        if ((num_dev > 0) && (num_dev < DEVDRV_MAX_DAVINCI_NUM) && (stage == 0)) {
            devdrv_drv_info("cost %d(s), enter next stage, curr_num_dev = %u\n", timeout, num_dev);
            stage = 1;
            timeout = 0;
            total_timeout = DEVDRV_GET_DEVNUM_SYNC_TIMEOUT;
        }
        if (timeout > total_timeout) {
            devdrv_drv_err("first get dev_num timeout in stage %d\n", stage);
            return;
        }
    } while (1);

    return;
}

u32 devdrv_manager_get_devnum(void)
{
    STATIC u32 devdrv_first_get_devnum_en = 1;
    u32 num_dev;
    u64 current_mnt_ns;
    int ret;

    if (dev_manager_info == NULL) {
        return (DEVDRV_MAX_DAVINCI_NUM + 1);
    }

    mutex_lock(&devdrv_get_devnum_mutex);
    if (devdrv_first_get_devnum_en) {
        devdrv_manager_first_get_devnum();
        devdrv_first_get_devnum_en = 0;
    }

    ret = devdrv_manager_get_current_mnt_ns(&current_mnt_ns);
    if (ret) {
        mutex_unlock(&devdrv_get_devnum_mutex);
        devdrv_drv_err("get current mnt ns error, ret = %d\n", ret);
        return (DEVDRV_MAX_DAVINCI_NUM + 1);
    }
    num_dev = devdrv_manager_container_get_devnum_ns(current_mnt_ns);
    mutex_unlock(&devdrv_get_devnum_mutex);
    return num_dev;
}

struct devdrv_info *devdrv_manager_get_devdrv_info(u32 dev_id)
{
    struct devdrv_info *dev_info = NULL;
    unsigned long flags;

    if (dev_manager_info == NULL || dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        return NULL;
    }

    spin_lock_irqsave(&dev_manager_info->spinlock, flags);
    dev_info = dev_manager_info->dev_info[dev_id];
    spin_unlock_irqrestore(&dev_manager_info->spinlock, flags);

    return dev_info;
}
EXPORT_SYMBOL(devdrv_manager_get_devdrv_info); //lint !e508

STATIC int devdrv_manager_set_devinfo_inc_devnum(u32 dev_id, struct devdrv_info *dev_info)
{
    unsigned long flags;

    if ((dev_id >= DEVDRV_MAX_DAVINCI_NUM) || dev_manager_info == NULL || dev_info == NULL) {
        devdrv_drv_err("invalid parameter\n");
        return -EINVAL;
    }

    spin_lock_irqsave(&dev_manager_info->spinlock, flags);
    if (dev_manager_info->dev_info[dev_id] != NULL) {
        spin_unlock_irqrestore(&dev_manager_info->spinlock, flags);
        devdrv_drv_err("dev_info is not NULL, dev_id = %d\n", dev_id);
        return -ENODEV;
    }

    if (dev_manager_info->num_dev >= DEVDRV_MAX_DAVINCI_NUM) {
        spin_unlock_irqrestore(&dev_manager_info->spinlock, flags);
        devdrv_drv_err("wrong device num, num_dev = %d\n", dev_manager_info->num_dev);
        return -EFAULT;
    }
    dev_manager_info->num_dev++;
    dev_manager_info->dev_info[dev_id] = dev_info;
    spin_unlock_irqrestore(&dev_manager_info->spinlock, flags);

    return 0;
}

STATIC int devdrv_manager_reset_devinfo_dec_devnum(u32 dev_id)
{
    unsigned long flags;

    if ((dev_id >= DEVDRV_MAX_DAVINCI_NUM) || dev_manager_info == NULL) {
        return -EINVAL;
    }

    spin_lock_irqsave(&dev_manager_info->spinlock, flags);
    if (dev_manager_info->dev_info[dev_id] == NULL) {
        spin_unlock_irqrestore(&dev_manager_info->spinlock, flags);
        devdrv_drv_err("dev_info is NULL, dev_id = %d\n", dev_id);
        return -ENODEV;
    }

    if (dev_manager_info->num_dev > DEVDRV_MAX_DAVINCI_NUM || dev_manager_info->num_dev == 0) {
        spin_unlock_irqrestore(&dev_manager_info->spinlock, flags);
        devdrv_drv_err("wrong device num, num_dev = %d\n", dev_manager_info->num_dev);
        return -EFAULT;
    }
    dev_manager_info->dev_info[dev_id] = NULL;
    dev_manager_info->num_dev--;
    dev_manager_info->device_status[dev_id] = DRV_STATUS_INITING;
    spin_unlock_irqrestore(&dev_manager_info->spinlock, flags);

    return 0;
}

STATIC int devdrv_manager_set_devdrv_info(u32 dev_id, struct devdrv_info *dev_info)
{
    unsigned long flags;

    if ((dev_id >= DEVDRV_MAX_DAVINCI_NUM) || dev_manager_info == NULL) {
        return -EINVAL;
    }

    spin_lock_irqsave(&dev_manager_info->spinlock, flags);
    dev_manager_info->dev_info[dev_id] = dev_info;
    spin_unlock_irqrestore(&dev_manager_info->spinlock, flags);

    return 0;
}

int devdrv_get_platformInfo(u32 *info)
{
    if (info == NULL) {
        return -EINVAL;
    }

    *info = DEVDRV_MANAGER_HOST_ENV;
    return 0;
}
EXPORT_SYMBOL(devdrv_get_platformInfo);

int devdrv_get_devnum(u32 *num_dev)
{
    u32 dev_num;

    dev_num = devdrv_manager_get_devnum();
    if (dev_num == 0 || dev_num > DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("invalid device number, dev_num = %d\n", dev_num);
        return -EFAULT;
    }

    *num_dev = dev_num;
    return 0;
}
EXPORT_SYMBOL(devdrv_get_devnum);

int devdrv_get_devids(u32 *devices, u32 device_num)
{
    struct devdrv_info *dev_info = NULL;
    u32 num_dev;
    u32 i;
    u32 j = 0;

    if (devices == NULL) {
        devdrv_drv_err("invalid parameter\n");
        return -EINVAL;
    }

    num_dev = devdrv_manager_get_devnum();
    if ((num_dev > DEVDRV_MAX_DAVINCI_NUM) || (num_dev == 0) || (num_dev > device_num)) {
        devdrv_drv_err("wrong dev number, num_dev = %u max_num :%u\n", num_dev, device_num);
        return -ENODEV;
    }

    for (i = 0; i < DEVDRV_MAX_DAVINCI_NUM; i++) {
        dev_info = devdrv_manager_get_devdrv_info(i);
        if (dev_info == NULL) {
            continue;
        }
        if (j >= device_num) {
            break;
        }
        devices[j] = dev_info->dev_id;
        j++;
    }
    if (j == 0) {
        devdrv_drv_err("NO dev_info!!!\n");
        return -EFAULT;
    }

    return 0;
}
EXPORT_SYMBOL(devdrv_get_devids);

int devdrv_get_devinfo(u32 devid, struct devdrv_device_info *info)
{
    struct devdrv_info *dev_info = NULL;

    if (info == NULL) {
        devdrv_drv_err("invalid parameter, dev_id = %d\n", devid);
        return -EINVAL;
    }

    dev_info = devdrv_manager_get_devdrv_info(devid);
    if (dev_info == NULL) {
        devdrv_drv_err("device is not ready, devid = %d\n", devid);
        return -ENODEV;
    }

    /* check if received device ready message from device side */
    if (dev_info->dev_ready == 0) {
        devdrv_drv_err("device %d not ready!", dev_info->dev_id);
        return -ENODEV;
    }

    info->ctrl_cpu_ip = dev_info->ctrl_cpu_ip;
    info->ctrl_cpu_id = dev_info->ctrl_cpu_id;
    info->ctrl_cpu_core_num = dev_info->ctrl_cpu_core_num;
    info->ctrl_cpu_occupy_bitmap = dev_info->ctrl_cpu_occupy_bitmap;
    info->ctrl_cpu_endian_little = dev_info->ctrl_cpu_endian_little;
    info->ts_cpu_core_num = dev_info->pdata->ts_pdata[0].ts_cpu_core_num;
    info->ai_cpu_core_num = dev_info->ai_cpu_core_num;
    info->ai_core_num = dev_info->ai_core_num;
    info->aicpu_occupy_bitmap = dev_info->aicpu_occupy_bitmap;
    info->env_type = dev_info->env_type;

    return 0;
}
EXPORT_SYMBOL(devdrv_get_devinfo);

int devdrv_manager_devid_to_nid(u32 devid, u32 mem_type)
{
    return NUMA_NO_NODE;
}
EXPORT_SYMBOL(devdrv_manager_devid_to_nid);

int devdrv_get_core_inuse(u32 devid, u32 vfid, struct devdrv_hardware_inuse *inuse)
{
    struct devdrv_info *dev_info = NULL;

    if (devid >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("devid is invalid.\n");
        return -EINVAL;
    }

    if (inuse == NULL || dev_manager_info == NULL || dev_manager_info->dev_info[devid] == NULL) {
        devdrv_drv_err("device manager is not initialized.\n");
        return -EINVAL;
    }

    dev_info = dev_manager_info->dev_info[devid];

    if (tsdrv_is_ts_work(devid, 0) == false) {
        devdrv_drv_err("device is not working.\n");
        return -ENXIO;
    }

    inuse->ai_core_num = dev_info->inuse.ai_core_num;
    inuse->ai_core_error_bitmap = dev_info->inuse.ai_core_error_bitmap;
    inuse->ai_cpu_num = dev_info->inuse.ai_cpu_num;
    inuse->ai_cpu_error_bitmap = dev_info->inuse.ai_cpu_error_bitmap;

    return 0;
}
EXPORT_SYMBOL(devdrv_get_core_inuse);

int devdrv_get_core_spec(u32 devid, u32 vfid, struct devdrv_hardware_spec *spec)
{
    struct devdrv_info *dev_info = NULL;

    if (devid >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("devid is invalid.\n");
        return -EINVAL;
    }

    if (spec == NULL || dev_manager_info == NULL || dev_manager_info->dev_info[devid] == NULL) {
        devdrv_drv_err("device manager is not initialized.\n");
        return -EINVAL;
    }

    dev_info = dev_manager_info->dev_info[devid];

    spec->ai_core_num = dev_info->ai_core_num;
    spec->first_ai_core_id = dev_info->ai_core_id;
    spec->ai_cpu_num = dev_info->ai_cpu_core_num;
    spec->first_ai_cpu_id = dev_info->ai_cpu_core_id;

    return 0;
}
EXPORT_SYMBOL(devdrv_get_core_spec);

int devdrv_manager_get_product_type(void)
{
    return devdrv_get_host_type();
}
EXPORT_SYMBOL(devdrv_manager_get_product_type);

STATIC int devdrv_manager_ipc_id_init(struct devdrv_manager_context *dev_manager_context)
{
    return 0;
}

STATIC void devdrv_manager_ipc_id_uninit(struct devdrv_manager_context *dev_manager_context) {}

STATIC struct devdrv_manager_context *devdrv_manager_context_init(void)
{
    struct devdrv_manager_context *dev_manager_context = NULL;

    if (devdrv_manager_container_task_struct_check(current)) {
        devdrv_drv_err("current is invalid.\n");
        return NULL;
    }

    dev_manager_context = kzalloc(sizeof(struct devdrv_manager_context), GFP_KERNEL | __GFP_ACCOUNT);
    if (dev_manager_context == NULL) {
        devdrv_drv_err("kmalloc dev_manager_context failed\n");
        return NULL;
    }
    dev_manager_context->pid = current->pid;
    dev_manager_context->tgid = current->tgid;
    dev_manager_context->task = current;
    dev_manager_context->mnt_ns = (u64)(uintptr_t)current->nsproxy->mnt_ns;
    dev_manager_context->start_time = current->start_time;
    dev_manager_context->real_start_time = get_start_time(current);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0)
    dev_manager_context->pid_ns = current->nsproxy->pid_ns_for_children;
#else
    dev_manager_context->pid_ns = current->nsproxy->pid_ns;
#endif

    if (devdrv_manager_ipc_id_init(dev_manager_context)) {
        devdrv_drv_err("manager ipc id init failed\n");
        kfree(dev_manager_context);
        dev_manager_context = NULL;
        return NULL;
    }

    return dev_manager_context;
}

STATIC void devdrv_manager_context_uninit(struct devdrv_manager_context *dev_manager_context)
{
    if (dev_manager_context == NULL) {
        return;
    }
    devdrv_manager_ipc_id_uninit(dev_manager_context);
    kfree(dev_manager_context);
    dev_manager_context = NULL;
}

STATIC int devdrv_manager_open(struct inode *inode, struct file *filep)
{
    struct devdrv_manager_context *dev_manager_context = NULL;
    u32 docker_id;
    int ret;

    dev_manager_context = devdrv_manager_context_init();
    if (dev_manager_context == NULL) {
        devdrv_drv_err("context init failed\n");
        return -ENOMEM;
    }

    ret = devdrv_manager_container_table_overlap(dev_manager_context, &docker_id);
    if (ret) {
        devdrv_manager_context_uninit(dev_manager_context);
        devdrv_drv_err("update_item failed, ret(%d).\n", ret);
        return -EINVAL;
    }

    filep->private_data = dev_manager_context;

    return 0;
}

STATIC void devdrv_manager_process_sign_release(pid_t pid)
{
    struct devdrv_manager_info *d_info = NULL;
    struct devdrv_process_sign *d_sign = NULL;
    struct list_head *pos = NULL;
    struct list_head *n = NULL;

    d_info = devdrv_get_manager_info();
    if (d_info == NULL) {
        devdrv_drv_err("d_info is NULL.\n");
        return;
    }
    mutex_lock(&d_info->devdrv_sign_list_lock);
    if (!list_empty_careful(&d_info->hostpid_list_header)) {
        list_for_each_safe(pos, n, &d_info->hostpid_list_header)
        {
            d_sign = list_entry(pos, struct devdrv_process_sign, list);
            if (d_sign->hostpid == pid) {
                list_del(&d_sign->list);
                d_info->devdrv_sign_count[d_sign->docker_id]--;
                devdrv_drv_info("end devdrv_get_process_sign, count = %u, docker_id = %u\n",
                    d_info->devdrv_sign_count[d_sign->docker_id], d_sign->docker_id);

                kfree(d_sign);
                d_sign = NULL;
                break;
            }
        }
    }
    mutex_unlock(&d_info->devdrv_sign_list_lock);

    return;
}

STATIC int devdrv_manager_release(struct inode *inode, struct file *filep)
{
    struct devdrv_manager_context *dev_manager_context = NULL;

    if ((filep == NULL) || (filep->private_data == NULL)) {
        devdrv_drv_err("filep is %s.\n", (filep == NULL) ? "NULL" : "OK");
        return -EINVAL;
    }

    dev_manager_context = filep->private_data;
    devdrv_manager_process_sign_release(dev_manager_context->tgid);
    devdrv_host_black_box_close_check(dev_manager_context->tgid);
    devdrv_manager_context_uninit(dev_manager_context);

    return 0;
}

struct devdrv_manager_info *devdrv_get_manager_info(void)
{
    return dev_manager_info;
}
EXPORT_SYMBOL(devdrv_get_manager_info);

STATIC int devdrv_manager_get_pci_info(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_platform_data *pdata = NULL;
    struct devdrv_pci_info pci_info = {0};
    struct devdrv_info *dev_info = NULL;
    u32 dev_id = DEVDRV_MAX_DAVINCI_NUM + 1;
    u32 virt_id;
    int ret;

    if (copy_from_user_safe(&pci_info, (void *)(uintptr_t)arg, sizeof(pci_info))) {
        return -EFAULT;
    }
    virt_id = pci_info.dev_id;
    if (devdrv_manager_trans_and_check_id(virt_id, &dev_id) != 0) {
        devdrv_drv_err("transfer device err\n");
        return -EFAULT;
    }

    dev_info = dev_manager_info->dev_info[dev_id % DEVDRV_MAX_DAVINCI_NUM];
    if (dev_info == NULL) {
        devdrv_drv_err("dev_info is NULL\n");
        return -EFAULT;
    }
    pdata = (struct devdrv_platform_data *)dev_info->pdata;
    if (pdata == NULL) {
        devdrv_drv_err("pci_dev_info is NULL\n");
        return -EFAULT;
    }

    ret = dms_hotreset_task_cnt_increase(dev_id);
    if (ret != 0) {
        devdrv_drv_err("Hotreset task cnt increase failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    pci_info.bus_number = pdata->pci_info.bus_number;
    pci_info.dev_number = pdata->pci_info.dev_number;
    pci_info.function_number = pdata->pci_info.function_number;

    if (copy_to_user_safe((void *)(uintptr_t)arg, &pci_info, sizeof(struct devdrv_pci_info))) {
        devdrv_drv_err("Copy_to_user_safe failed.\n");
        goto out;
    }
    dms_hotreset_task_cnt_decrease(dev_id);
    return 0;
out:
    dms_hotreset_task_cnt_decrease(dev_id);
    return -EFAULT;
}

STATIC int devdrv_manager_get_device_status(struct file *filep, unsigned int cmd, unsigned long arg)
{
    enum devdrv_ts_status ts_status;
    u32 loc_id = 0;
    u32 status;
    int ret;

    u32 phys_id = DEVDRV_MAX_DAVINCI_NUM + 1;
    u32 vfid = 0;
    ret = copy_from_user_safe(&loc_id, (void *)(uintptr_t)arg, sizeof(u32));
    if (ret) {
        devdrv_drv_err("copy from user failed.\n");
        return -EINVAL;
    }
    if (loc_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("devid is invalid.\n");
        return -EINVAL;
    }

    if (devdrv_manager_container_logical_id_to_physical_id(loc_id, &phys_id, &vfid) != 0) {
        devdrv_drv_err("can't transfor virt id %u.\n", loc_id);
        return -EFAULT;
    }

    ts_status = tsdrv_get_ts_status(phys_id, 0);

    if (dev_manager_info == NULL || dev_manager_info->dev_info[phys_id] == NULL) {
        status = DRV_STATUS_INITING;
    } else if (dev_manager_info->device_status[phys_id] == DRV_STATUS_COMMUNICATION_LOST) {
        status = DRV_STATUS_COMMUNICATION_LOST;
    } else if (ts_status == TS_DOWN) {
        status = DRV_STATUS_EXCEPTION;
    } else if (ts_status == TS_WORK) {
        status = DRV_STATUS_WORK;
    } else {
        status = DRV_STATUS_INITING;
    }

    ret = copy_to_user_safe((void *)(uintptr_t)arg, &status, sizeof(u32));
    if (ret) {
        devdrv_drv_err("copy to user failed.\n");
        return -EINVAL;
    }

    return 0;
}

STATIC int devdrv_manager_get_core(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_hardware_inuse inuse = {0};
    struct devdrv_hardware_spec spec = {0};
    int ret;

    u32 phys_id = DEVDRV_MAX_DAVINCI_NUM + 1;
    switch (cmd) {
        case DEVDRV_MANAGER_GET_CORE_SPEC:
            ret = copy_from_user_safe(&spec, (void *)((uintptr_t)arg), sizeof(struct devdrv_hardware_spec));
            if (ret) {
                devdrv_drv_err("copy_from_user_safe failed.\n");
                return -1;
            }
            if (devdrv_manager_trans_and_check_id(spec.devid, &phys_id) != 0) {
                devdrv_drv_err("can't transfor virt id %u \n", spec.devid);
                return -EFAULT;
            }
            ret = dms_hotreset_task_cnt_increase(phys_id);
            if (ret != 0) {
                devdrv_drv_err("Hotreset task cnt increase failed. (dev_id=%u; ret=%d)\n", phys_id, ret);
                return ret;
            }
            ret = devdrv_get_core_spec(phys_id, 0, &spec);
            if (ret) {
                devdrv_drv_err("devdrv_get_core_spec failed.\n");
                dms_hotreset_task_cnt_decrease(phys_id);
                return -1;
            }
            ret = copy_to_user_safe((void *)((uintptr_t)arg), &spec, sizeof(struct devdrv_hardware_spec));
            if (ret) {
                devdrv_drv_err("copy_to_user_safe failed.\n");
                dms_hotreset_task_cnt_decrease(phys_id);
                return -1;
            }
            dms_hotreset_task_cnt_decrease(phys_id);
            break;
        case DEVDRV_MANAGER_GET_CORE_INUSE:
            ret = copy_from_user_safe(&inuse, (void *)((uintptr_t)arg), sizeof(struct devdrv_hardware_inuse));
            if (ret) {
                devdrv_drv_err("copy_from_user_safe failed.\n");
                return -1;
            }
            if (devdrv_manager_trans_and_check_id(inuse.devid, &phys_id) != 0) {
                devdrv_drv_err("can't transfor virt id %u \n", inuse.devid);
                return -EFAULT;
            }

            ret = dms_hotreset_task_cnt_increase(phys_id);
            if (ret != 0) {
                devdrv_drv_err("Hotreset task cnt increase failed. (dev_id=%u; ret=%d)\n", phys_id, ret);
                return ret;
            }
            ret = devdrv_get_core_inuse(phys_id, 0, &inuse);
            if (ret) {
                devdrv_drv_err("devdrv_get_core_inuse failed.\n");
                dms_hotreset_task_cnt_decrease(phys_id);
                return -1;
            }
            ret = copy_to_user_safe((void *)(uintptr_t)arg, &inuse, sizeof(struct devdrv_hardware_inuse));
            if (ret) {
                devdrv_drv_err("copy_to_user_safe failed.\n");
                dms_hotreset_task_cnt_decrease(phys_id);
                return -1;
            }
            dms_hotreset_task_cnt_decrease(phys_id);
            break;
        default:
            devdrv_drv_err("invalid cmd.\n");
            return -1;
    }
    return 0;
}

STATIC int devdrv_manager_get_probe_list(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_manager_hccl_devinfo hccl_devinfo = {0};
    struct devdrv_manager_info *devmng_info = NULL;
    u32 i;
    u32 index = 0;

    if (devdrv_is_in_container()) {
        devdrv_drv_err("not support using in container.\n");
        return -EINVAL;
    }

    devmng_info = devdrv_get_manager_info();
    if (devmng_info == NULL) {
        devdrv_drv_err("devmng info is NULL\n");
        return -EINVAL;
    }

    for (i = 0; i < DEVDRV_MAX_DAVINCI_NUM; i++) {
        hccl_devinfo.devids[i] = DEVDRV_INVALID_PHY_ID;
        if (CHECK_BIT_64(devmng_info->prob_device_bitmap[i / BITS_PER_LONG_LONG], i % BITS_PER_LONG_LONG)) {
            hccl_devinfo.devids[index++] = i;
        }
    }
    hccl_devinfo.num_dev = index;

    if (copy_to_user_safe((void *)(uintptr_t)arg, &hccl_devinfo, sizeof(hccl_devinfo))) {
        devdrv_drv_err("copy to user failed.\n");
        return -EINVAL;
    }
    return 0;
}

STATIC int devdrv_manager_get_devids(unsigned long arg)
{
    struct devdrv_manager_hccl_devinfo hccl_devinfo = { 0 };
    u32 num_dev;
    int ret;
    u64 current_mnt;

    num_dev = devdrv_manager_get_devnum();
    if ((num_dev > DEVDRV_MAX_DAVINCI_NUM) || (num_dev == 0)) {
        return -EINVAL;
    }

    hccl_devinfo.num_dev = num_dev;

    ret = devdrv_manager_get_current_mnt_ns(&current_mnt);
    if (ret) {
        devdrv_drv_err("get current mnt ns error, ret = %d\n", ret);
        return -EINVAL;
    }

    ret = devdrv_manager_container_get_devids(hccl_devinfo.devids, DEVDRV_MAX_DAVINCI_NUM, &num_dev, current_mnt);
    if (ret) {
        devdrv_drv_err("item find failed, mnt_ns = %llu\n get devlist failed, ret = %d\n", current_mnt, ret);
        return -ENODEV;
    }

    if (copy_to_user_safe((void *)(uintptr_t)arg, &hccl_devinfo, sizeof(hccl_devinfo))) {
        devdrv_drv_err("copy from user failed.\n");
        return -EINVAL;
    }

    return 0;
}

STATIC int devdrv_manager_get_devinfo(unsigned long arg)
{
    struct devdrv_manager_hccl_devinfo hccl_devinfo = { 0 };
    struct devdrv_info *dev_info = NULL;
    u32 dev_id;
    u32 phys_id = DEVDRV_MAX_DAVINCI_NUM + 1;
    int ret;

    if (copy_from_user_safe(&hccl_devinfo, (void *)(uintptr_t)arg, sizeof(hccl_devinfo))) {
        devdrv_drv_err("copy from user failed\n");
        return -EINVAL;
    }

    dev_id = hccl_devinfo.dev_id;
    if (dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("wrong device id, dev_id = %d\n", hccl_devinfo.dev_id);
        return -ENODEV;
    }
    ret = devdrv_manager_trans_and_check_id(dev_id, &phys_id);
    if (ret != 0) {
        devdrv_drv_err("can't transfor virt id %u \n", dev_id);
        return ret;
    }

    dev_info = devdrv_manager_get_devdrv_info(phys_id);
    if (dev_info == NULL) {
        devdrv_drv_err("device %d is not initialized\n", phys_id);
        return -ENODEV;
    }

    ret = dms_hotreset_task_cnt_increase(phys_id);
    if (ret != 0) {
        devdrv_drv_err("Hotreset task cnt increase failed. (dev_id=%u; ret=%d)\n", phys_id, ret);
        return ret;
    }

    hccl_devinfo.ai_core_num = dev_info->ai_core_num;
    hccl_devinfo.ai_cpu_core_num = dev_info->ai_cpu_core_num;
    hccl_devinfo.ctrl_cpu_core_num = dev_info->ctrl_cpu_core_num;
    hccl_devinfo.ctrl_cpu_occupy_bitmap = dev_info->ctrl_cpu_occupy_bitmap;

    /* 1:little endian 0:big endian */
    hccl_devinfo.ctrl_cpu_endian_little = dev_info->ctrl_cpu_endian_little;
    hccl_devinfo.ctrl_cpu_id = dev_info->ctrl_cpu_id;
    hccl_devinfo.ctrl_cpu_ip = dev_info->ctrl_cpu_ip;
    hccl_devinfo.ts_cpu_core_num = dev_info->pdata->ts_pdata[0].ts_cpu_core_num;
    hccl_devinfo.env_type = dev_info->env_type;
    hccl_devinfo.ai_core_id = dev_info->ai_core_id;
    hccl_devinfo.ai_cpu_core_id = dev_info->ai_cpu_core_id;
    hccl_devinfo.ai_cpu_bitmap = dev_info->aicpu_occupy_bitmap;
    hccl_devinfo.hardware_version = dev_info->hardware_version;
    /* ts_num in mini is 1 */
    hccl_devinfo.ts_num = 1;

    devdrv_drv_debug("ctrl_cpu_ip = 0x%x, ts_cpu_core_num = %d\n", dev_info->ctrl_cpu_ip, hccl_devinfo.ts_cpu_core_num);

    if (copy_to_user_safe((void *)(uintptr_t)arg, &hccl_devinfo, sizeof(hccl_devinfo))) {
        devdrv_drv_err("copy to user error.\n");
        ret = -EFAULT;
        goto out;
    }

    dms_hotreset_task_cnt_decrease(phys_id);
    return 0;
out:
    dms_hotreset_task_cnt_decrease(phys_id);
    return ret;
}

STATIC int devdrv_manager_get_h2d_devinfo(unsigned long arg)
{
    struct devdrv_manager_hccl_devinfo hccl_devinfo = { 0 };
    struct devdrv_info *dev_info = NULL;
    u32 dev_id;
    u32 phys_id = DEVDRV_MAX_DAVINCI_NUM + 1;
    int ret;

    if (copy_from_user_safe(&hccl_devinfo, (void *)(uintptr_t)arg, sizeof(hccl_devinfo))) {
        devdrv_drv_err("copy from user failed\n");
        return -EINVAL;
    }

    dev_id = hccl_devinfo.dev_id;
    if (dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("wrong device id, dev_id = %d\n", hccl_devinfo.dev_id);
        return -ENODEV;
    }
    if (devdrv_manager_trans_and_check_id(dev_id, &phys_id) != 0) {
        devdrv_drv_err("can't transfor virt id %u \n", dev_id);
        return -EFAULT;
    }

    ret = dms_hotreset_task_cnt_increase(phys_id);
    if (ret != 0) {
        devdrv_drv_err("Hotreset task cnt increase failed. (dev_id=%u; ret=%d)\n", phys_id, ret);
        return ret;
    }

    dev_info = devdrv_manager_get_devdrv_info(phys_id);
    if (dev_info == NULL) {
        devdrv_drv_err("device %d is not initialized\n", phys_id);
        ret = -ENODEV;
        goto out;
    }

    hccl_devinfo.ai_core_num = dev_info->ai_core_num;
    hccl_devinfo.ai_cpu_core_num = dev_info->ai_cpu_core_num;
    hccl_devinfo.ctrl_cpu_core_num = dev_info->ctrl_cpu_core_num;
    hccl_devinfo.ctrl_cpu_occupy_bitmap = dev_info->ctrl_cpu_occupy_bitmap;

    /* 1:little endian 0:big endian */
    hccl_devinfo.ctrl_cpu_endian_little = dev_info->ctrl_cpu_endian_little;
    hccl_devinfo.ctrl_cpu_id = dev_info->ctrl_cpu_id;
    hccl_devinfo.ctrl_cpu_ip = dev_info->ctrl_cpu_ip;
    hccl_devinfo.ts_cpu_core_num = dev_info->pdata->ts_pdata[0].ts_cpu_core_num;

    hccl_devinfo.env_type = dev_info->env_type;
    hccl_devinfo.ai_core_id = dev_info->ai_core_id;
    hccl_devinfo.ai_cpu_core_id = dev_info->ai_cpu_core_id;
    hccl_devinfo.ai_cpu_bitmap = dev_info->aicpu_occupy_bitmap;
    hccl_devinfo.hardware_version = dev_info->hardware_version;
    /* ts_num in mini is 1 */
    hccl_devinfo.ts_num = 1;

    if ((dev_info->drv_ops == NULL) || (dev_info->drv_ops->get_dev_info(dev_info))) {
        devdrv_drv_err("device info get failed\n");
    }
    hccl_devinfo.cpu_system_count = dev_info->cpu_system_count;
    hccl_devinfo.monotonic_raw_time_ns = dev_info->monotonic_raw_time_ns;

    devdrv_drv_debug("ctrl_cpu_ip = 0x%x, ts_cpu_core_num = %d\n", dev_info->ctrl_cpu_ip, hccl_devinfo.ts_cpu_core_num);

    if (copy_to_user_safe((void *)(uintptr_t)arg, &hccl_devinfo, sizeof(hccl_devinfo))) {
        devdrv_drv_err("copy to user error.\n");
        ret = -EFAULT;
        goto out;
    }

    dms_hotreset_task_cnt_decrease(phys_id);
    return 0;
out:
    dms_hotreset_task_cnt_decrease(phys_id);
    return ret;
}

STATIC int devdrv_manager_get_dev_info_by_phyid(unsigned long arg)
{
    int ret;
    struct devdrv_phy_get_devinfo_para para = {0};

    if (devdrv_is_in_container()) {
        devdrv_drv_err("not support using in container.\n");
        return -EINVAL;
    }

    ret = copy_from_user_safe(&para, (void *)(uintptr_t)arg, sizeof(struct devdrv_phy_get_devinfo_para));
    if (ret) {
        devdrv_drv_err("copy from user failed, ret = %d\n", ret);
        return ret;
    }

    para.chip_type = devdrv_get_dev_chip_type(para.phy_id);
    if (para.chip_type == HISI_CHIP_UNKNOWN) {
        devdrv_drv_err("devdrv_get_dev_chip_type failed, unknown \n");
        return -EINVAL;
    }
    if (para.chip_type >= HISI_CHIP_NUM) {
        devdrv_drv_err("chip_type invalid(%d)\n", para.chip_type);
        return -ENODEV;
    }
    if (copy_to_user_safe((void *)(uintptr_t)arg, &para, sizeof(struct devdrv_phy_get_devinfo_para))) {
        devdrv_drv_err("[phy_id=%u]:copy to user error.\n", para.phy_id);
        return -EFAULT;
    }

    return 0;
}

STATIC int devdrv_get_pcie_id(unsigned long arg)
{
    struct dmanage_pcie_id_info pcie_id_info = {0};
    struct devdrv_pcie_id_info id_info = {0};
    unsigned int virt_id;
    unsigned int dev_id = 0;
    int ret;

    ret = copy_from_user_safe(&pcie_id_info, (void *)((uintptr_t)arg), sizeof(struct dmanage_pcie_id_info));
    if (ret) {
        devdrv_drv_err("copy_from_user_safe failed.\n");
        return ret;
    }

    virt_id = pcie_id_info.davinci_id;
    ret = devdrv_manager_trans_and_check_id(virt_id, &dev_id);
    if (ret) {
        devdrv_drv_err("can't transfor virt id %u \n", virt_id);
        return ret;
    }

    ret = dms_hotreset_task_cnt_increase(dev_id);
    if (ret != 0) {
        devdrv_drv_err("Hotreset task cnt increase failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    ret = devdrv_get_pcie_id_info(dev_id, &id_info);
    if (ret) {
        devdrv_drv_err("devdrv_get_pcie_id failed.\n");
        goto out;
    }

    pcie_id_info.venderid = id_info.venderid;
    pcie_id_info.subvenderid = id_info.subvenderid;
    pcie_id_info.deviceid = id_info.deviceid;
    pcie_id_info.subdeviceid = id_info.subdeviceid;
    pcie_id_info.bus = id_info.bus;
    pcie_id_info.device = id_info.device;
    pcie_id_info.fn = id_info.fn;

    ret = copy_to_user_safe((void *)(uintptr_t)arg, &pcie_id_info, sizeof(struct dmanage_pcie_id_info));
    if (ret) {
        devdrv_drv_err("copy_to_user_safe failed.\n");
        goto out;
    }

    dms_hotreset_task_cnt_decrease(dev_id);
    return 0;
out:
    dms_hotreset_task_cnt_decrease(dev_id);
    return ret;
}

STATIC int devdrv_manager_get_core_utilization(unsigned long arg)
{
    struct devdrv_core_utilization util_info = {0};
    struct devdrv_info *dev_info = NULL;
    u32 phys_id = DEVDRV_MAX_DAVINCI_NUM + 1;
    u32 vfid = 0;
    u32 dev_id;
    int ret;

    ret = copy_from_user_safe(&util_info, (void*)(uintptr_t)arg, sizeof(struct devdrv_core_utilization));
    if (ret != 0) {
        devdrv_drv_err("copy from user failed. (ret=%d)\n", ret);
        return ret;
    }

    dev_id = util_info.dev_id;
    if (dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("Invalid dev_id. (dev_id=%u)\n", util_info.dev_id);
        return -ENODEV;
    }

    if (util_info.core_type >= DEV_DRV_TYPE_MAX) {
        devdrv_drv_err("Core_type is wrong. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    ret = devdrv_manager_container_logical_id_to_physical_id(dev_id, &phys_id, &vfid);
    if (ret != 0) {
        devdrv_drv_err("Transfor virt id failed. (dev_id=%u, ret=%d)\n", dev_id, ret);
        return -EFAULT;
    }
    if (!devdrv_manager_is_pf_device(phys_id) || (vfid > 0)) {
        devdrv_drv_err("vdevice is not support to get core utilization. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return -EOPNOTSUPP;
    }

    dev_info = devdrv_manager_get_devdrv_info(phys_id);
    if (dev_info == NULL) {
        devdrv_drv_err("Device is not initialized. (phys_id=%u)\n", phys_id);
        return -ENODEV;
    }

    ret = dms_hotreset_task_cnt_increase(phys_id);
    if (ret != 0) {
        devdrv_drv_err("Hotreset task cnt increase failed. (phys_id=%u; ret=%d)\n", phys_id, ret);
        return ret;
    }

    atomic_inc(&dev_info->occupy_ref);
    if (dev_info->status == DEVINFO_STATUS_REMOVED) {
        atomic_dec(&dev_info->occupy_ref);
        devdrv_drv_warn("Device has been reset. (dev_id=%u)\n", dev_info->dev_id);
        dms_hotreset_task_cnt_decrease(phys_id);
        return -EINVAL;
    }

    util_info.dev_id = phys_id;
    ret = devdrv_manager_h2d_sync_get_core_utilization(&util_info);
    if (ret != 0) {
        atomic_dec(&dev_info->occupy_ref);
        devdrv_drv_err("Core utilization get failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
        dms_hotreset_task_cnt_decrease(phys_id);
        return ret;
    }

    atomic_dec(&dev_info->occupy_ref);

    ret = copy_to_user_safe((void *)(uintptr_t)arg, &util_info, sizeof(struct devdrv_core_utilization));
    if (ret != 0) {
        devdrv_drv_err("Copy to user failed. (phys_id=%u, ret=%d)\n", phys_id, ret);
        dms_hotreset_task_cnt_decrease(phys_id);
        return -EFAULT;
    }

    dms_hotreset_task_cnt_decrease(phys_id);

    return ret;
}

STATIC int devdrv_manager_devinfo_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;

    switch (cmd) {
        case DEVDRV_MANAGER_GET_CORE_UTILIZATION:
            ret = devdrv_manager_get_core_utilization(arg);
            break;
        case DEVDRV_MANAGER_GET_DEVIDS:
            ret = devdrv_manager_get_devids(arg);
            break;
        case DEVDRV_MANAGER_GET_DEVINFO:
            ret = devdrv_manager_get_devinfo(arg);
            break;
        case DEVDRV_MANAGER_GET_H2D_DEVINFO:
            ret = devdrv_manager_get_h2d_devinfo(arg);
            break;
        case DEVDRV_MANAGER_GET_DEV_INFO_BY_PHYID:
            ret = devdrv_manager_get_dev_info_by_phyid(arg);
            break;
        case DEVDRV_MANAGER_GET_PCIE_ID_INFO:
            ret = devdrv_get_pcie_id(arg);
            break;
        default:
            ret = -EINVAL;
            break;
    }

    return ret;
}

STATIC int devdrv_manager_get_black_box_exception_index(void)
{
    int i;

    /* find the same pid */
    for (i = 0; i < MAX_EXCEPTION_THREAD; i++) {
        if (dev_manager_info->black_box.black_box_pid[i] == current->tgid) {
            return i;
        }
    }

    /* no same pid and inset new pid */
    for (i = 0; i < MAX_EXCEPTION_THREAD; i++) {
        if (dev_manager_info->black_box.black_box_pid[i] == 0) {
            dev_manager_info->black_box.black_box_pid[i] = current->tgid;
            return i;
        }
    }

    /* thread num exceed the MAX_EXCEPTION_THREAD */
    return MAX_EXCEPTION_THREAD;
}

/* This interface does not support using in containers */
STATIC int devdrv_manager_black_box_get_exception(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_black_box_user black_box_user = { 0 };
    unsigned long flags;
    int ret, index;

    ret = devdrv_is_in_container();
    if (ret) {
        devdrv_drv_err("not support using in container, ret(%d).\n", ret);
        return -1;
    }

    if (dev_manager_info == NULL) {
        devdrv_drv_err("device does not exist.\n");
        return -1;
    }

    spin_lock_irqsave(&dev_manager_info->black_box.spinlock, flags);
    index = devdrv_manager_get_black_box_exception_index();
    if (index >= MAX_EXCEPTION_THREAD) {
        spin_unlock_irqrestore(&dev_manager_info->black_box.spinlock, flags);
        devdrv_drv_err("thread num exceed the support MAX_EXCEPTION_THREAD.\n");
        return -EINVAL;
    }

    if (dev_manager_info->black_box.exception_num[index] > 0) {
        spin_unlock_irqrestore(&dev_manager_info->black_box.spinlock, flags);
        devdrv_drv_info("black box exception_num[%d] :%d\n", index, dev_manager_info->black_box.exception_num[index]);
        goto no_wait_black_box_sema;
    }
    spin_unlock_irqrestore(&dev_manager_info->black_box.spinlock, flags);

    ret = down_interruptible(&dev_manager_info->black_box.black_box_sema[index]);
    if (ret == -EINTR) {
        devdrv_drv_info("interrupted. ret = %d\n", ret);
        return 0;
    }
    if (ret) {
        devdrv_drv_err("down_interrupitable fail. ret = %d\n", ret);
        return -1;
    }

no_wait_black_box_sema:
    devdrv_host_black_box_get_exception(&black_box_user, index);
    ret = copy_to_user_safe((void *)((uintptr_t)arg), &black_box_user, sizeof(struct devdrv_black_box_user));
    if (ret) {
        devdrv_drv_err("copy_to_user_safe fail.\n");
        return -1;
    }
    return 0;
}

STATIC int devdrv_manager_device_memory_dump(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_black_box_user black_box_user = { 0 };
    struct devdrv_info *dev_info = NULL;
    dma_addr_t host_addr_dma = 0;
    void *buffer = NULL;
    int dev_id;
    int ret;

    if (devdrv_is_in_container()) {
        devdrv_drv_err("not support using in container.\n");
        return -EINVAL;
    }

    ret = copy_from_user_safe(&black_box_user, (void *)((uintptr_t)arg), sizeof(struct devdrv_black_box_user));
    if (ret) {
        devdrv_drv_err("copy_from_user_safe fail.\n");
        return -1;
    }
    if (black_box_user.devid >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("invalid devid %u \n", black_box_user.devid);
        return -EINVAL;
    }

    if (black_box_user.dst_buffer == NULL) {
        devdrv_drv_err("invalid buffer.\n");
        return -1;
    }
    if (black_box_user.size <= 0 || black_box_user.size > DEVDRV_MAX_MEMORY_DUMP_SIZE) {
        devdrv_drv_err("invalid size.\n");
        return -1;
    }

    dev_info = devdrv_get_devdrv_info_array(black_box_user.devid);
    if (dev_info == NULL) {
        devdrv_drv_err("no device(%u).\n", black_box_user.devid);
        return -ENODEV;
    }

    if ((black_box_user.addr_offset >= dev_info->dump_ddr_size) ||
        (black_box_user.addr_offset + black_box_user.size > dev_info->dump_ddr_size)) {
        devdrv_drv_err("invalid phy offset addr. dev_id(%u), size(%u), info size(%u)\n",
                       black_box_user.devid, black_box_user.size, dev_info->dump_ddr_size);
        return -EFAULT;
    }

    dev_id = devdrv_get_device_index(black_box_user.devid);
    if (dev_id < 0 || dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("get device[%d] index fail index(%d).\n", black_box_user.devid, dev_id);
        return -1;
    }

    black_box_user.addr_offset += dev_info->dump_ddr_dma_addr;

    buffer = dma_alloc_coherent(dev_info->dev, black_box_user.size, &host_addr_dma, GFP_KERNEL | __GFP_ACCOUNT);
    if (buffer == NULL) {
        devdrv_drv_err("dma_alloc_coherent fail devid %d size %d.\n", black_box_user.devid, black_box_user.size);
        return -1;
    }
    devdrv_drv_debug("devid: %d, len: %d\n", black_box_user.devid, black_box_user.size);

    ret = devdrv_dma_sync_copy(dev_info->pci_dev_id, DEVDRV_DMA_DATA_COMMON, (u64)black_box_user.addr_offset,
        (u64)host_addr_dma, black_box_user.size, DEVDRV_DMA_DEVICE_TO_HOST);
    if (ret) {
        devdrv_drv_err("devdrv_dma_sync_copy fail.\n");
        ret = -1;
        goto free_alloc;
    }

    ret = copy_to_user_safe(black_box_user.dst_buffer, buffer, black_box_user.size);
    if (ret) {
        devdrv_drv_err("copy_to_user_safe fail.\n");
        ret = -1;
        goto free_alloc;
    }
free_alloc:
    dma_free_coherent(dev_info->dev, black_box_user.size, buffer, host_addr_dma);
    buffer = NULL;
    return ret;
}

STATIC int devdrv_manager_ioctl_get_devnum(struct file *filep, unsigned int cmd, unsigned long arg)
{
    u32 devnum;

    devnum = devdrv_manager_get_devnum();
    if (copy_to_user_safe((void *)((uintptr_t)arg), &devnum, sizeof(u32))) {
        return -EFAULT;
    } else {
        return 0;
    }
}

STATIC int devdrv_manager_ioctl_get_console_loglevel(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int console_loglevle_value;
    int ret;

    console_loglevle_value = log_level_get();
    ret = copy_to_user_safe((void *)((uintptr_t)arg), &console_loglevle_value, sizeof(int));

    return ret;
}

STATIC int devdrv_manager_ioctl_get_plat_info(struct file *filep, unsigned int cmd, unsigned long arg)
{
    u32 plat_info;

    plat_info = dev_manager_info->plat_info;
    if (copy_to_user_safe((void *)(uintptr_t)arg, &plat_info, sizeof(u32))) {
        return -EFAULT;
    } else {
        return 0;
    }
}

/* This interface does not support using in containers */
STATIC int devdrv_manager_device_reset_inform(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct timespec stamp;
    u32 phy_id = 0;
    u32 vfid;
    u32 devid = 0;
    int ret;
    int virt_id;

    ret = devdrv_is_in_container();
    if (ret) {
        devdrv_drv_err("not support using in container, ret(%d).\n", ret);
        return -1;
    }

    ret = copy_from_user_safe(&devid, (void *)((uintptr_t)arg), sizeof(u32));
    if (ret) {
        devdrv_drv_err("copy_from_user_safe failed.\n");
        return -1;
    }

    virt_id = devid;

    if (devdrv_is_in_container() == 1) {
        devdrv_drv_err("not support using in container.\n");
        return -1;
    }

    ret = devdrv_manager_container_logical_id_to_physical_id(devid, &phy_id, &vfid);
    if (ret) {
        devdrv_drv_err("trans logical_id (%d) to physical_id(%d) failed.\n", devid, phy_id);
        return -EINVAL;
    }

    stamp = current_kernel_time();

    ret = devdrv_host_black_box_add_exception(phy_id, DEVDRV_BB_DEVICE_RESET_INFORM, stamp, NULL);
    if (ret) {
        devdrv_drv_err("devdrv_host_black_box_add_exception failed.\n");
        return -1;
    }

    return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)
ssize_t devdrv_load_file_read(struct file *file, loff_t *pos, char *addr, size_t count)
{
    char __user *buf = (char __user *)addr;
    mm_segment_t old_fs;
    ssize_t len;

    old_fs = get_fs();
    set_fs(get_ds()); /*lint !e501 */ /* kernel source */
    len = vfs_read(file, buf, count, pos);
    set_fs(old_fs);

    return len;
}
#endif

#ifndef CAP_SYS_ADMIN
#define CAP_SYS_ADMIN 21
#endif
STATIC int devdrv_manager_pcie_pre_reset(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_pcie_pre_reset para = {0};
    int ret;

    ret = devdrv_manager_check_permission();
    if (ret) {
        devdrv_drv_err("devdrv_manager_check_permission failed, ret(%d).\n", ret);
        return ret;
    }
    ret = copy_from_user_safe(&para, (void *)((uintptr_t)arg), sizeof(struct devdrv_pcie_pre_reset));
    if (ret) {
        devdrv_drv_err("copy from user failed.\n");
        return -EINVAL;
    }
    ret = devdrv_manager_check_phy_mach(para.dev_id);
    if (ret) {
        devdrv_drv_err("permission deny or devid[%d] is invalid, ret = %d\n", para.dev_id, ret);
        return -EINVAL;
    }
    ret = devdrv_pcie_prereset(para.dev_id);

    return ret;
}

STATIC int devdrv_manager_pcie_rescan(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_pcie_rescan para = {0};
    int ret;

    ret = devdrv_manager_check_permission();
    if (ret) {
        devdrv_drv_err("devdrv_manager_check_permission failed, ret(%d).\n", ret);
        return ret;
    }
    ret = copy_from_user_safe(&para, (void *)((uintptr_t)arg), sizeof(struct devdrv_pcie_rescan));
    if (ret) {
        devdrv_drv_err("copy from user failed.\n");
        return -EINVAL;
    }
    ret = devdrv_pcie_reinit(para.dev_id);

    return ret;
}

int devdrv_manager_pcie_hot_reset(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_pcie_hot_reset para = {0};
    u32 phy_id = 0;
    u32 vfid;
    int ret;
#ifdef CFG_FEATURE_TIMESYNC
    int i;
#endif

    ret = devdrv_manager_check_permission();
    if (ret) {
        devdrv_drv_err("devdrv_manager_check_permission failed, ret(%d).\n", ret);
        return ret;
    }

    ret = copy_from_user_safe(&para, (void *)((uintptr_t)arg), sizeof(struct devdrv_pcie_hot_reset));
    if (ret) {
        devdrv_drv_err("copy from user failed, ret(%d).\n", ret);
        return -EINVAL;
    }

    /* hot reset all devices */
    if (para.dev_id == 0xff) {
        ret = devdrv_manager_check_all_phy_mach();
        if (ret) {
            devdrv_drv_err("permission deny or devid[%d] is invalid, ret = %d\n", para.dev_id, ret);
            return ret;
        }

#ifdef CFG_FEATURE_TIMESYNC
        for (i = 0; i < DEVDRV_MAX_DAVINCI_NUM; i++) {
            set_time_need_update(i);
        }
#endif
        return devdrv_hot_reset_device(para.dev_id);
    }

    /* the single device hot reset */
    ret = devdrv_manager_container_logical_id_to_physical_id(para.dev_id, &phy_id, &vfid);
    if (ret) {
        devdrv_drv_err("trans logical_id to physical_id failed, devid = %d.\n", para.dev_id);
        return ret;
    }

    ret = devdrv_manager_check_phy_mach(phy_id);
    if (ret) {
        devdrv_drv_err("permission deny or devid[%d] is invalid, ret = %d\n", para.dev_id, ret);
        return ret;
    }

#ifdef CFG_FEATURE_TIMESYNC
    set_time_need_update(phy_id);
#endif
    return devdrv_hot_reset_device(phy_id);
}

int devdrv_check_va(struct vm_area_struct *vma, unsigned long long va, unsigned int size)
{
    unsigned long long end = va + PAGE_ALIGN(size);
    unsigned long long va_check;
    unsigned long pfn;

    if (vma->vm_flags & VM_HUGETLB) {
        devdrv_drv_err("va hute table\n");
        return -EINVAL;
    }

    if (va & (PAGE_SIZE - 1)) {
        devdrv_drv_err("va(0x%llx) is invalid\n", va);
        return -EINVAL;
    }

    if ((va < vma->vm_start) || (va > vma->vm_end) || (end > vma->vm_end) || (va >= end)) {
        devdrv_drv_err("va(0x%llx) end(0x%llx) size(0x%x) error.\n", va, end, size);
        return -EINVAL;
    }

    for (va_check = va; va_check < end; va_check += PAGE_SIZE) {
        if (follow_pfn(vma, va_check, &pfn) == 0) {
            devdrv_drv_err("va(0x%llx) size(0x%x), va_check(0x%llx) is valid\n", va, size, va_check);
            return -EINVAL;
        }
    }
    return 0;
}

STATIC int drv_dma_mmap(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_alloc_host_dma_addr_para alloc_host_dma_addr_para = { 0 };
    unsigned long long bbox_resv_dmaAddr = 0;
    u32 phys_id;
    struct page *bbox_resv_dmaPages = NULL;
    unsigned int bbox_resv_size = 0;
    struct vm_area_struct *vma = NULL;
    int ret;

    ret = devdrv_manager_check_permission();
    if (ret) {
        devdrv_drv_err("check permission fail.\n");
        return ret;
    }

    ret = copy_from_user_safe(&alloc_host_dma_addr_para, (void *)(uintptr_t)arg,
        sizeof(struct devdrv_alloc_host_dma_addr_para));
    if (ret) {
        devdrv_drv_err("copy from user failed.\n");
        return -EINVAL;
    }

    phys_id = alloc_host_dma_addr_para.devId;

    down_write(get_mmap_sem(current->mm));
    vma = find_vma(current->mm, alloc_host_dma_addr_para.virAddr);
    if (vma == NULL) {
        up_write(get_mmap_sem(current->mm));
        devdrv_drv_err("find vma failed\n");
        return 0;
    }

    ret = devdrv_get_bbox_reservd_mem(phys_id, &bbox_resv_dmaAddr, &bbox_resv_dmaPages, &bbox_resv_size);
    if (ret) {
        up_write(get_mmap_sem(current->mm));
        devdrv_drv_err("devdrv_get_bbox_reservd_mem failed.\n");
        return -EINVAL;
    }

    if (bbox_resv_size > 0) {
        ret = devdrv_check_va(vma, alloc_host_dma_addr_para.virAddr, bbox_resv_size);
        if (ret) {
            up_write(get_mmap_sem(current->mm));
            devdrv_drv_err("dev_id(%u) va(0x%llx) already map, ret(%d).\n", phys_id, alloc_host_dma_addr_para.virAddr,
                ret);
            return -EINVAL;
        }
        ret = remap_pfn_range(vma, alloc_host_dma_addr_para.virAddr, page_to_pfn(bbox_resv_dmaPages), bbox_resv_size,
            PAGE_READONLY);
        if (ret) {
            up_write(get_mmap_sem(current->mm));
            devdrv_drv_err("remap_pfn_range failed,ret=%d.\n", ret);
            return -EINVAL;
        }
    }
    up_write(get_mmap_sem(current->mm));

    devdrv_drv_info("use pcie reserved buffer,"
        "page:%pK, size:0x%x.\n",
        bbox_resv_dmaPages, bbox_resv_size);

    alloc_host_dma_addr_para.size = bbox_resv_size;
    alloc_host_dma_addr_para.phyAddr = 0;

    ret = copy_to_user_safe((void *)((uintptr_t)arg), &alloc_host_dma_addr_para,
        sizeof(struct devdrv_alloc_host_dma_addr_para));
    if (ret) {
        devdrv_drv_err("copy to user failed, ret(%d), dev_id(%u).\n", ret, phys_id);
        return -EINVAL;
    }

    return 0;
}

// UEFI SRAM FLAG
#define DEVDRV_SRAM_BBOX_UEFI_DUMP_OFFSET 0x0800
#define DEVDRV_SRAM_BBOX_UEFI_DUMP_LEN 20

// BIOS SRAM log (include KEYPOINT FLAG)
#define DEVDRV_SRAM_BBOX_BIOS_OFFSET 0x3D800UL
#define DEVDRV_SRAM_BBOX_BIOS_LEN 0x800UL
/* sram in bar2 */
STATIC int drv_pcie_sram_read(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_pcie_sram_read_para pcie_sram_read_para = { 0 };
    u32 phys_id;
    void __iomem *mem_sram_base = NULL;
    u64 sram_phy_addr = 0;
    size_t sram_size = 0;
    int ret;
    u32 i = 0;

    if (devdrv_is_in_container()) {
        devdrv_drv_err("not support using in container.\n");
        return -EINVAL;
    }

    ret =
        copy_from_user_safe(&pcie_sram_read_para, (void *)((uintptr_t)arg), sizeof(struct devdrv_pcie_sram_read_para));
    if (ret) {
        devdrv_drv_err("copy pcie sram from user failed.\n");
        return -EINVAL;
    }

    phys_id = pcie_sram_read_para.devId;
    /* only can read within region for bbox */
    if (!(((pcie_sram_read_para.offset >= DEVDRV_SRAM_BBOX_UEFI_DUMP_OFFSET) && (((u64)pcie_sram_read_para.offset +
        (u64)pcie_sram_read_para.len) <= DEVDRV_SRAM_BBOX_UEFI_DUMP_OFFSET + DEVDRV_SRAM_BBOX_UEFI_DUMP_LEN)) ||
        ((pcie_sram_read_para.offset >= DEVDRV_SRAM_BBOX_BIOS_OFFSET) && (((u64)pcie_sram_read_para.offset +
        (u64)pcie_sram_read_para.len) <= DEVDRV_SRAM_BBOX_BIOS_OFFSET + DEVDRV_SRAM_BBOX_BIOS_LEN)))) {
        devdrv_drv_err("cannot access sram, id:%u, offset:%u, len:%u.\n", phys_id, pcie_sram_read_para.offset,
            pcie_sram_read_para.len);
        return -EINVAL;
    }

    ret = devdrv_get_addr_info(phys_id, DEVDRV_ADDR_LOAD_RAM, 0, &sram_phy_addr, &sram_size);
    if (ret || (!sram_phy_addr) || (!sram_size)) {
        devdrv_drv_err("get sram addr by devid error\n");
        return -EINVAL;
    }
    if ((pcie_sram_read_para.len == 0) || (pcie_sram_read_para.len > sizeof(pcie_sram_read_para.value))) {
        devdrv_drv_err("value len:%d error!offset:%d, sram_size:%d\n", pcie_sram_read_para.len,
            pcie_sram_read_para.offset, (int)sram_size);
        return -EINVAL;
    }
    mem_sram_base = ioremap(sram_phy_addr, sram_size);
    if (mem_sram_base == NULL) {
        devdrv_drv_err("sram ioremap error\n");
        return -EINVAL;
    }
    for (i = 0; i < pcie_sram_read_para.len; i++) {
        pcie_sram_read_para.value[i] = *((u8 *)mem_sram_base + pcie_sram_read_para.offset + i);
    }
    iounmap(mem_sram_base);
    mem_sram_base = NULL;

    ret = copy_to_user_safe((void *)(uintptr_t)arg, &pcie_sram_read_para, sizeof(struct devdrv_pcie_sram_read_para));

    return ret;
}

struct tsdrv_drv_ops *devdrv_manager_get_drv_ops(void)
{
    return &devdrv_host_drv_ops;
}
EXPORT_SYMBOL(devdrv_manager_get_drv_ops);

/* bbox ddr in bar4 */
STATIC int drv_pcie_bbox_ddr_read(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_pcie_ddr_read_para pcie_ddr_read_para = { 0 };
    u32 phys_id;
    void __iomem *mem_ddr_base = NULL;
    u64 bbox_ddr_phy_addr = 0;
    size_t bbox_ddr_size = 0;
    int ret;
    u32 i = 0;

    if (devdrv_is_in_container()) {
        devdrv_drv_err("not support using in container.\n");
        return -EINVAL;
    }

    ret = copy_from_user_safe(&pcie_ddr_read_para, (void *)((uintptr_t)arg), sizeof(struct devdrv_pcie_ddr_read_para));
    if (ret) {
        devdrv_drv_err("copy pcie ddr from user failed.\n");
        return -EINVAL;
    }

    phys_id = pcie_ddr_read_para.devId;
    ret = devdrv_get_addr_info(phys_id, DEVDRV_ADDR_TEST_BASE, 0, &bbox_ddr_phy_addr, &bbox_ddr_size);
    if (ret || (!bbox_ddr_phy_addr) || (!bbox_ddr_size)) {
        devdrv_drv_err("get bbox ddr addr by devid error\n");
        return -EINVAL;
    }

    if ((pcie_ddr_read_para.len == 0) || (pcie_ddr_read_para.len > sizeof(pcie_ddr_read_para.value)) ||
        ((u64)pcie_ddr_read_para.offset + (u64)pcie_ddr_read_para.len > bbox_ddr_size)) {
        devdrv_drv_err("value len:%d error!offset:%d,bbox_ddr_size:%d\n", pcie_ddr_read_para.len,
            pcie_ddr_read_para.offset, (int)bbox_ddr_size);
        return -EINVAL;
    }

    mem_ddr_base = ioremap(bbox_ddr_phy_addr, bbox_ddr_size);
    if (mem_ddr_base == NULL) {
        devdrv_drv_err("bbox ddr ioremap error\n");
        return -EINVAL;
    }
    for (i = 0; i < pcie_ddr_read_para.len; i++) {
        pcie_ddr_read_para.value[i] = *((u8 *)mem_ddr_base + pcie_ddr_read_para.offset + i);
    }
    iounmap(mem_ddr_base);
    mem_ddr_base = NULL;

    ret = copy_to_user_safe((void *)(uintptr_t)arg, &pcie_ddr_read_para, sizeof(struct devdrv_pcie_ddr_read_para));

    return ret;
}

STATIC int drv_get_device_boot_status(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_get_device_boot_status_para get_device_boot_status_para = { 0 };
    int ret;
    u32 phys_id;

    ret = copy_from_user_safe(&get_device_boot_status_para, (void *)((uintptr_t)arg),
        sizeof(struct devdrv_get_device_boot_status_para));
    if (ret) {
        devdrv_drv_err("copy from user failed.\n");
        return -EINVAL;
    }

    phys_id = get_device_boot_status_para.devId;
    if (phys_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("phys_id %d is invalid\n", phys_id);
        return -EINVAL;
    }
    /* only judge the physical id is available in container */
    if (devdrv_is_in_container() == 1) {
        ret = devdrv_manager_container_check_devid_in_container_ns(phys_id, current);
        if (ret) {
            devdrv_drv_err("device phyid %u is not belong to current docker\n", phys_id);
            return -EFAULT;
        }
    }

    ret = devdrv_get_device_boot_status(phys_id, &get_device_boot_status_para.boot_status);
    if (ret == -ENXIO) {
        devdrv_drv_warn("no such device, ret(%d), dev_id(%u).\n", ret, phys_id);
    } else if (ret) {
        devdrv_drv_err("cannot get device boot status, ret(%d), dev_id(%u).\n", ret, phys_id);
        return -EINVAL;
    }

    ret = copy_to_user_safe((void *)((uintptr_t)arg), &get_device_boot_status_para,
        sizeof(struct devdrv_get_device_boot_status_para));

    return ret;
}

STATIC int devdrv_manager_get_host_phy_mach_flag(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_get_host_phy_mach_flag_para para = { 0 };
    int ret;
    u32 phys_id = 0;
    u32 vfid = 0;

    ret = copy_from_user_safe(&para, (void *)((uintptr_t)arg), sizeof(struct devdrv_get_host_phy_mach_flag_para));
    if (ret) {
        devdrv_drv_err("copy from user failed.\n");
        return -EINVAL;
    }

    ret = devdrv_manager_container_logical_id_to_physical_id(para.devId, &phys_id, &vfid);
    if (ret) {
        devdrv_drv_err("can't transfor virt id %u ret(%d)\n", para.devId, ret);
        return -EINVAL;
    }

    ret = devdrv_get_host_phy_mach_flag(phys_id, &para.host_flag);
    if (ret) {
        devdrv_drv_err("cannot get host flag, id:%d.\n", para.devId);
        return -EINVAL;
    }

    ret = copy_to_user_safe((void *)((uintptr_t)arg), &para, sizeof(struct devdrv_get_host_phy_mach_flag_para));

    return ret;
}

/* bbox hdr(history data record) area in bar2, 512KB */
STATIC int drv_pcie_bbox_hdr_read(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_pcie_hdr_read_para pcie_hdr_read_para = { 0 };
    u32 phys_id;
    void __iomem *mem_hdr_base = NULL;
    u64 bbox_hdr_phy_addr = 0;
    size_t bbox_hdr_size = 0;
    int ret;
    u32 i = 0;

    if (devdrv_is_in_container()) {
        devdrv_drv_err("not support using in container.\n");
        return -EINVAL;
    }

    ret = copy_from_user_safe(&pcie_hdr_read_para, (void *)((uintptr_t)arg), sizeof(struct devdrv_pcie_hdr_read_para));
    if (ret) {
        devdrv_drv_err("copy bbox hdr struct from user failed.\n");
        return -EINVAL;
    }

    phys_id = pcie_hdr_read_para.devId;

    ret = devdrv_get_addr_info(phys_id, DEVDRV_ADDR_HDR_BASE, 0, &bbox_hdr_phy_addr, &bbox_hdr_size);
    if (ret || (!bbox_hdr_phy_addr) || (!bbox_hdr_size)) {
        devdrv_drv_err("get bbox hdr addr by devid error\n");
        return -EINVAL;
    }

    if ((pcie_hdr_read_para.len == 0) || (pcie_hdr_read_para.len > sizeof(pcie_hdr_read_para.value)) ||
        ((u64)pcie_hdr_read_para.offset + (u64)pcie_hdr_read_para.len > bbox_hdr_size)) {
        devdrv_drv_err("value len:%d error!offset:%d,bbox_hdr_size:%d\n", pcie_hdr_read_para.len,
            pcie_hdr_read_para.offset, (int)bbox_hdr_size);
        return -EINVAL;
    }

    mem_hdr_base = ioremap(bbox_hdr_phy_addr, bbox_hdr_size);
    if (mem_hdr_base == NULL) {
        devdrv_drv_err("bbox hdr ioremap error\n");
        return -EINVAL;
    }
    for (i = 0; i < pcie_hdr_read_para.len; i++) {
        pcie_hdr_read_para.value[i] = *((u8 *)mem_hdr_base + pcie_hdr_read_para.offset + i);
    }
    iounmap(mem_hdr_base);
    mem_hdr_base = NULL;

    ret = copy_to_user_safe((void *)((uintptr_t)arg), &pcie_hdr_read_para, sizeof(struct devdrv_pcie_hdr_read_para));

    return ret;
}

STATIC int devdrv_manager_all_devices_rdy_check(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_manager_info *manager_info = NULL;
    struct devdrv_sysrdy_info sysrdy_info = { 0 };
    int ret;

    manager_info = devdrv_get_manager_info();
    if (manager_info == NULL) {
        devdrv_drv_err("manager info is NULL\n");
        return -ENODEV;
    }

    if (devdrv_is_in_container()) { // if in container, probe_dev_num and rdy_dev_num is equal
        sysrdy_info.probe_dev_num = devdrv_manager_get_devnum();
        sysrdy_info.rdy_dev_num = sysrdy_info.probe_dev_num;
    } else {
        spin_lock(&manager_info->spinlock);
        sysrdy_info.probe_dev_num = manager_info->prob_num;
        sysrdy_info.rdy_dev_num = manager_info->num_dev;
        spin_unlock(&manager_info->spinlock);
    }

    if (sysrdy_info.probe_dev_num > DEVDRV_MAX_DAVINCI_NUM || sysrdy_info.rdy_dev_num > DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("wrong device num, "
            "probe dev num = %u, "
            "rdy  dev num = %u\n",
            sysrdy_info.probe_dev_num, sysrdy_info.rdy_dev_num);
        return -EFAULT;
    }
    if (sysrdy_info.probe_dev_num == 0 || sysrdy_info.rdy_dev_num == 0) {
        sysrdy_info.probe_dev_num = DEVDRV_MAX_DAVINCI_NUM;
        sysrdy_info.rdy_dev_num = DEVDRV_MAX_DAVINCI_NUM - 1;
    }

    ret = copy_to_user_safe((void *)((uintptr_t)arg), &sysrdy_info, sizeof(struct devdrv_sysrdy_info));
    if (ret) {
        devdrv_drv_err("copy to user failed\n");
        return -ENOMEM;
    }

    return ret;
}

STATIC int devdrv_manager_container_cmd(struct file *filep, unsigned int cmd, unsigned long arg)
{
    return devdrv_manager_container_process(filep, arg);
}

STATIC int devdrv_manager_get_probe_num(struct file *filep, unsigned int cmd, unsigned long arg)
{
    u32 num;
    int ret;
    unsigned long flags;

    if (devdrv_is_in_container()) { // if in container, num is the number of device in container
        num = devdrv_manager_get_devnum();
    } else {
        spin_lock_irqsave(&dev_manager_info->spinlock, flags);
        num = dev_manager_info->prob_num;
        spin_unlock_irqrestore(&dev_manager_info->spinlock, flags);
    }

    ret = copy_to_user_safe((void *)((uintptr_t)arg), &num, sizeof(u32));
    if (ret) {
        devdrv_drv_err("copy to user failed, ret: %d.\n", ret);
        return -1;
    }

    return 0;
}

int devdrv_manager_shm_info_check(struct devdrv_info *dev_info)
{
    if ((dev_info == NULL) || (dev_info->shm_status == NULL) ||
        (dev_info->shm_head == NULL) || (dev_info->shm_board == NULL)) {
        devdrv_drv_err("dev_info is NULL or dev_info->shm_status/shm_head is NULL.\n");
        return -EFAULT;
    }

    if ((dev_info->shm_head->head_info.version != DEVMNG_SHM_INFO_HEAD_VERSION) ||
        (dev_info->shm_head->head_info.magic != DEVMNG_SHM_INFO_HEAD_MAGIC)) {
        devdrv_drv_err("dev(%u) version of share memory in host is 0x%llx, "
                       "the version in device is 0x%llx, magic is 0x%x.\n",
                       dev_info->dev_id, DEVMNG_SHM_INFO_HEAD_VERSION,
                       dev_info->shm_head->head_info.version,
                       dev_info->shm_head->head_info.magic);
        return -EFAULT;
    }

    return 0;
}

STATIC int devdrv_get_error_code(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_error_code_para user_arg = { 0, {0}, 0 };
    struct devdrv_info *dev_info = NULL;
    u32 phys_id, vfid;
    int ret, i;

    ret = copy_from_user_safe(&user_arg, (void *)((uintptr_t)arg), sizeof(struct devdrv_error_code_para));
    if (ret) {
        devdrv_drv_err("copy_from_user_safe failed.\n");
        return ret;
    }

    if (devdrv_manager_container_logical_id_to_physical_id(user_arg.dev_id, &phys_id, &vfid)) {
        devdrv_drv_err("can't transfor virt id %u.\n", user_arg.dev_id);
        return -ENODEV;
    }

    dev_info = devdrv_manager_get_devdrv_info(phys_id);
    if (devdrv_manager_shm_info_check(dev_info)) {
        devdrv_drv_err("devid(%u) shm info check fail.\n", phys_id);
        return -EFAULT;
    }

    user_arg.error_code_count = dev_info->shm_status->error_cnt;
    for (i = 0; i < DEVMNG_SHM_INFO_ERROR_CODE_LEN; i++) {
        user_arg.error_code[i] = dev_info->shm_status->error_code[i];
    }

    ret = copy_to_user_safe((void *)((uintptr_t)arg), &user_arg, sizeof(struct devdrv_error_code_para));
    if (ret != 0) {
        devdrv_drv_err("copy_to_user_safe failed.\n");
        return ret;
    }

    return 0;
}

int devmng_dms_get_event_code(u32 devid, u32 *health_code, u32 health_len,
    struct shm_event_code *event_code, u32 event_len)
{
    struct devdrv_info *dev_info = NULL;
    int i, cnt;

    if ((devid >= DEVDRV_MAX_DAVINCI_NUM) || (health_code == NULL) ||
        (health_len != VMNG_VDEV_MAX_PER_PDEV) || (event_code == NULL) ||
        (event_len != DEVMNG_SHM_INFO_EVENT_CODE_LEN)) {
        devdrv_drv_err("Invalid parameter. (devid=%u; health_code=\"%s\"; health_len=%u; "
                       "event_code=\"%s\"; event_len=%u)\n", devid,
                       (health_code == NULL) ? "NULL" : "OK", health_len,
                       (event_code == NULL) ? "NULL" : "OK", event_len);
        return -EINVAL;
    }

    dev_info = devdrv_manager_get_devdrv_info(devid);
    if (devdrv_manager_shm_info_check(dev_info)) {
        devdrv_drv_err("The dev_info is invalid. (devid=%u)\n", devid);
        return -EFAULT;
    }

    for (i = 0; i < VMNG_VDEV_MAX_PER_PDEV; i++) {
        health_code[i] = dev_info->shm_status->dms_health_status[i];
    }
    cnt = (dev_info->shm_status->event_cnt > DEVMNG_SHM_INFO_EVENT_CODE_LEN) ?
          DEVMNG_SHM_INFO_EVENT_CODE_LEN : dev_info->shm_status->event_cnt;
    for (i = 0; i < cnt; i++) {
        event_code[i].event_code = dev_info->shm_status->event_code[i].event_code;
        event_code[i].fid = dev_info->shm_status->event_code[i].fid;
    }

    return 0;
}

int devmng_dms_get_health_code(u32 devid, u32 *health_code, u32 health_len)
{
    struct devdrv_info *dev_info = NULL;
    int i;

    if ((devid >= DEVDRV_MAX_DAVINCI_NUM) || (health_code == NULL) ||
        (health_len != VMNG_VDEV_MAX_PER_PDEV)) {
        devdrv_drv_err("Invalid parameter. (devid=%u; health_code=\"%s\"; health_len=%u)\n",
                       devid, (health_code == NULL) ? "NULL" : "OK", health_len);
        return -EINVAL;
    }

    dev_info = devdrv_manager_get_devdrv_info(devid);
    if (devdrv_manager_shm_info_check(dev_info)) {
        devdrv_drv_err("The dev_info is invalid. (devid=%u)\n", devid);
        return -EFAULT;
    }

    for (i = 0; i < VMNG_VDEV_MAX_PER_PDEV; i++) {
        health_code[i] = dev_info->shm_status->dms_health_status[i];
    }

    return 0;
}

STATIC int devdrv_creat_random_sign(char *random_sign, u32 len)
{
    char random[RANDOM_SIZE] = {0};
    int offset = 0;
    int ret;
    int i;

    for (i = 0; i < RANDOM_SIZE; i++) {
        ret = snprintf_s(random_sign + offset, len - offset, len - 1 - offset, "%02x", (u8)random[i]);
        if (ret < 0) {
            devdrv_drv_err("snprintf failed, ret(%d).\n", ret);
            return -EINVAL;
        }
        offset += ret;
    }
    random_sign[len - 1] = '\0';

    return 0;
}

STATIC int devdrv_get_tgid_by_pid(int pid, int *tgid)
{
    struct task_struct *tsk = NULL;
    struct pid_namespace *pid_ns = NULL;
    int ret;

    if (tgid == NULL) {
        devdrv_drv_err("The tgid is NULL.\n");
        return -EINVAL;
    }

    /* To get current process's pid_ns. */
    pid_ns = task_active_pid_ns(current);
    if (pid_ns == NULL) {
        devdrv_drv_err("Failed to find current process's pid_ns. (pid=%d)\n", pid);
        return -EINVAL;
    }

    /*
    * The value of tsk is NULL, which means the PID and current process are not in the same pid_ns.
    * Even if we find it, it doesn't mean it really is, because each container may has its own pid_ns.
    */
    tsk = get_pid_task(find_pid_ns(pid, pid_ns), PIDTYPE_PID);
    if (tsk == NULL) {
        devdrv_drv_err("Failed to find task struct in current process's pid_ns. (pid=%d)\n", pid);
        return -EINVAL;
    }

    /* If not in container, it return tgid directly. */
    ret = devdrv_manager_container_is_in_container();
    if (ret < 0) {
        devdrv_drv_err("Failed to invoke devdrv_is_in_container. (ret=%d)\n", ret);
        goto OUT;
    } else if (ret != true) {
        *tgid = tsk->tgid;
        ret = 0;
        goto OUT;
    }

    /* If in container, the PID mnt_ns will be equal to the current mnt_ns. */
    if ((tsk->nsproxy != NULL) && (tsk->nsproxy->mnt_ns == current->nsproxy->mnt_ns)) {
        *tgid = tsk->tgid;
        ret = 0;
        goto OUT;
    }

    ret = -EINVAL;
    devdrv_drv_err("The PID does not exist in the container. (pid=%d)\n", pid);
OUT:
    put_task_struct(tsk);
    return ret;
}

STATIC int devdrv_host_query_process_by_host_pid(struct devdrv_ioctl_para_query_pid *para_info,
    struct devdrv_info *info)
{
    int ret;
    int out_len = 0;
    pid_t host_tgid = -1;
    struct devdrv_manager_msg_info dev_manager_msg_info = {{0}, {0}};
    struct devdrv_ioctl_para_query_pid *para_info_tmp;

    if (para_info == NULL || info == NULL) {
        devdrv_drv_err("para_info or info is NULL!\n");
        return -EINVAL;
    }

    if (devdrv_get_tgid_by_pid(para_info->host_pid, &host_tgid) != 0) {
        devdrv_drv_err("Failed to get tgid by pid. (pid=%d)\n", para_info->host_pid);
        return -EINVAL;
    }
    para_info->host_pid = host_tgid;

    dev_manager_msg_info.header.msg_id = DEVDRV_MANAGER_CHAN_H2D_QUERY_DEVICE_PID;
    dev_manager_msg_info.header.valid = DEVDRV_MANAGER_MSG_VALID;
    /* give a random value for checking result later */
    dev_manager_msg_info.header.result = (u16)DEVDRV_MANAGER_MSG_INVALID_RESULT;
    /* inform corresponding devid to device side */
    dev_manager_msg_info.header.dev_id = info->dev_id;

    para_info_tmp = (struct devdrv_ioctl_para_query_pid *)dev_manager_msg_info.payload;
    para_info_tmp->host_pid = para_info->host_pid;
    para_info_tmp->vfid = para_info->vfid;
    para_info_tmp->chip_id = para_info->chip_id;
    para_info_tmp->cp_type = para_info->cp_type;

    ret = devdrv_manager_send_msg(info, &dev_manager_msg_info, &out_len);
    if (ret != 0) {
        devdrv_drv_warn("send msg to device fail ret = %d\n", ret);
        return ret;
    }
    if (out_len != (sizeof(struct devdrv_ioctl_para_query_pid) + sizeof(struct devdrv_manager_msg_head))) {
        devdrv_drv_warn("receive response len %d is not equal = %ld\n", out_len,
            (sizeof(struct devdrv_ioctl_para_query_pid) + sizeof(struct devdrv_manager_msg_head)));
        return -EINVAL;
    }
    if (dev_manager_msg_info.header.result != 0) {
        devdrv_drv_warn("Can not get device pid. (ret=%d).\n", dev_manager_msg_info.header.result);
        return dev_manager_msg_info.header.result;
    }
    para_info->pid = para_info_tmp->pid;
    return 0;
}

int devdrv_host_query_devpid(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    struct devdrv_ioctl_para_query_pid para_info = {0};
    struct devdrv_info *info = NULL;
    unsigned int phy_id = 0;
    unsigned int vfid = 0;

    if (copy_from_user_safe(&para_info, (void *)(uintptr_t)arg, sizeof(struct devdrv_ioctl_para_query_pid))) {
        devdrv_drv_err("[devdrv_query_devpid] copy_from_user error.\n");
        return -EINVAL;
    }

    ret = devdrv_manager_container_logical_id_to_physical_id(para_info.chip_id, &phy_id, &vfid);
    if (ret != 0) {
        devdrv_drv_err("can't get phys device id. virt id is %u, ret = %d\n", para_info.chip_id, ret);
        return -EINVAL;
    }

    info = devdrv_manager_get_devdrv_info(phy_id);
    if (info == NULL) {
        devdrv_drv_err("info is NULL!\n");
        return -EINVAL;
    }

    ret = dms_hotreset_task_cnt_increase(phy_id);
    if (ret != 0) {
        devdrv_drv_err("Hotreset task cnt increase failed. (dev_id=%u; ret=%d)\n", phy_id, ret);
        return ret;
    }

    ret = devdrv_host_query_process_by_host_pid(&para_info, info);
    if (ret) {
        devdrv_drv_warn("query device_pid by host_pid failed, ret(%d)\n", ret);
        goto out;
    }

    if (copy_to_user_safe((void *)((uintptr_t)arg), &para_info, sizeof(struct devdrv_ioctl_para_query_pid))) {
        devdrv_drv_err("[devdrv_query_devpid] copy_to_user error.\n");
        ret = -EINVAL;
        goto out;
    }

    dms_hotreset_task_cnt_decrease(phy_id);
    return 0;
out:
    dms_hotreset_task_cnt_decrease(phy_id);
    return ret;
}

STATIC int devdrv_host_notice_dev_process_exit(u32 phy_id, int host_pid)
{
    struct devdrv_manager_msg_info dev_manager_msg_info = {{0}, {0}};
    struct devdrv_ioctl_para_query_pid *para_info_tmp = NULL;
    struct devdrv_info *info = NULL;
    int out_len = 0;
    int ret;

    info = devdrv_manager_get_devdrv_info(phy_id);
    if (info == NULL) {
        return 0;
    }
    dev_manager_msg_info.header.msg_id = DEVDRV_MANAGER_CHAN_H2D_NOTICE_PROCESS_EXIT;
    dev_manager_msg_info.header.valid = DEVDRV_MANAGER_MSG_VALID;
    /* give a random value for checking result later */
    dev_manager_msg_info.header.result = (u16)DEVDRV_MANAGER_MSG_INVALID_RESULT;
    /* inform corresponding devid to device side */
    dev_manager_msg_info.header.dev_id = info->dev_id;

    para_info_tmp = (struct devdrv_ioctl_para_query_pid *)dev_manager_msg_info.payload;
    para_info_tmp->host_pid = host_pid;
    ret = devdrv_manager_send_msg(info, &dev_manager_msg_info, &out_len);
    if (ret != 0) {
        return 0;
    }
    if (out_len != (sizeof(struct devdrv_ioctl_para_query_pid) + sizeof(struct devdrv_manager_msg_head))) {
        return 0;
    }
    if (dev_manager_msg_info.header.result != 0) {
        return 0;
    }

    return (para_info_tmp->pid == 0) ? 0 : -ETXTBSY;
}

#define MAX_NOTICE_DEV_EXIT_TIMES 1000
STATIC void devdrv_host_release_notice_dev(int host_pid)
{
    u32 did, try_time;
    int ret;

    for (did = 0, try_time = 0; did < DEVDRV_MAX_DAVINCI_NUM; did++) {
        do {
            ret = devdrv_host_notice_dev_process_exit(did, host_pid);
            if (ret != 0) {
                usleep_range(10, 20);  /* 10-20 us */
                try_time++;
            }
        } while ((ret != 0) && (try_time < MAX_NOTICE_DEV_EXIT_TIMES));
    }
}

STATIC int devdrv_get_process_sign(struct devdrv_manager_info *d_info, char *sign, u32 len, u32 docker_id)
{
    struct devdrv_process_sign *d_sign = NULL;
    struct list_head *pos = NULL;
    struct list_head *n = NULL;
    int ret;

    if (!list_empty_careful(&d_info->hostpid_list_header)) {
        list_for_each_safe(pos, n, &d_info->hostpid_list_header)
        {
            d_sign = list_entry(pos, struct devdrv_process_sign, list);
            if (d_sign->hostpid == current->tgid) {
                ret = strcpy_s(sign, len, d_sign->sign);
                if (ret) {
                    devdrv_drv_err("strcpy_s failed, ret(%d).\n", ret);
                    return -EINVAL;
                }
                return 0;
            }
        }
    }

    if (d_info->devdrv_sign_count[docker_id] >= DEVDRV_MAX_SIGN_NUM) {
        devdrv_drv_err("list add failed, docker_id(%u).\n", docker_id);
        return -EINVAL;
    }

    d_sign = kzalloc(sizeof(struct devdrv_process_sign), GFP_KERNEL | __GFP_ACCOUNT);
    if (d_sign == NULL) {
        devdrv_drv_err("kzalloc failed, docker_id(%u).\n", docker_id);
        return -ENOMEM;
    }
    d_sign->hostpid = current->tgid;
    d_sign->docker_id = docker_id;

    ret = devdrv_creat_random_sign(d_sign->sign, PROCESS_SIGN_LENGTH);
    if (ret) {
        devdrv_drv_err("get sign failed, ret(%d), docker_id(%u).\n", ret, docker_id);
        kfree(d_sign);
        d_sign = NULL;
        return ret;
    }
    ret = strcpy_s(sign, len, d_sign->sign);
    if (ret) {
        devdrv_drv_err("strcpy_s failed, ret(%d), docker_id(%u).\n", ret, docker_id);
        kfree(d_sign);
        d_sign = NULL;
        return -EINVAL;
    }
    list_add(&d_sign->list, &d_info->hostpid_list_header);
    d_info->devdrv_sign_count[d_sign->docker_id]++;

    return 0;
}

STATIC int devdrv_manager_get_process_sign(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_manager_info *d_info = devdrv_get_manager_info();
    struct process_sign dev_sign = { 0 };
    u32 docker_id = MAX_DOCKER_NUM;
    int ret;

    if (d_info == NULL) {
        devdrv_drv_err("d_info is null.\n");
        return -EINVAL;
    }

    /*
     * check current environment :
     * non-container : docker_id is set to 64;
     * container : get docker_id , docker_id is 0 ~ 63.
     */
    ret = devdrv_is_in_container();
    if (ret) {
        ret = devdrv_manager_container_get_docker_id(&docker_id);
        if (ret) {
            devdrv_drv_err("container get docker_id failed, ret(%d).\n", ret);
            return -EINVAL;
        }
    }

    mutex_lock(&d_info->devdrv_sign_list_lock);
    ret = devdrv_get_process_sign(d_info, dev_sign.sign, PROCESS_SIGN_LENGTH, docker_id);
    if (ret) {
        mutex_unlock(&d_info->devdrv_sign_list_lock);
        devdrv_drv_err("get process_sign failed, ret(%d).\n", ret);
        return ret;
    }
    mutex_unlock(&d_info->devdrv_sign_list_lock);

    dev_sign.tgid = current->tgid;

    ret = copy_to_user_safe((void *)((uintptr_t)arg), &dev_sign, sizeof(struct process_sign));
    if (ret) {
        devdrv_drv_err("copy to user failed, ret(%d).\n", ret);
        return ret;
    }
    (void)memset_s(dev_sign.sign, PROCESS_SIGN_LENGTH, 0, PROCESS_SIGN_LENGTH);

    return 0;
}

STATIC int devdrv_manager_get_container_flag(struct file *filep, unsigned int cmd, unsigned long arg)
{
    unsigned int flag;
    int ret;

    ret = devdrv_is_in_container();
    if (ret < 0) {
        devdrv_drv_err("get contianer flag failed, ret(%d).\n", ret);
        return ret;
    }
    flag = (unsigned int)ret;
    ret = copy_to_user_safe((void *)((uintptr_t)arg), &flag, sizeof(unsigned int));
    return ret;
}

STATIC int devdrv_manager_board_info_mem(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_board_info board_info = {0};
    struct devdrv_info *dev_info = NULL;
    u32 dev_id = DEVDRV_MAX_DAVINCI_NUM + 1;
    u32 virt_id;
    int ret;

    if (copy_from_user_safe(&board_info, (void *)(uintptr_t)arg, sizeof(board_info))) {
        devdrv_drv_err("copy_from_user_safe failed.\n");
        return -EFAULT;
    }

    virt_id = board_info.dev_id;
    if (devdrv_manager_trans_and_check_id(virt_id, &dev_id) != 0) {
        devdrv_drv_err("transfer device err. (virt_id=%u)\n", virt_id);
        return -EFAULT;
    }

    dev_info = dev_manager_info->dev_info[dev_id % DEVDRV_MAX_DAVINCI_NUM];
    if (devdrv_manager_shm_info_check(dev_info)) {
        devdrv_drv_err("shm info check fail. (virt_id=%u)\n", virt_id);
        return -EFAULT;
    }

    ret = dms_hotreset_task_cnt_increase(dev_id);
    if (ret != 0) {
        devdrv_drv_err("Hotreset task cnt increase failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    board_info.board_id = (u32)dev_info->shm_board->board_id;
    board_info.slot_id = (u32)dev_info->shm_board->slot_id;
    if (copy_to_user_safe((void *)(uintptr_t)arg, &board_info, sizeof(struct devdrv_board_info))) {
        devdrv_drv_err("copy_from_user_safe failed. (virt_id=%u)\n", virt_id);
        goto out;
    }

    dms_hotreset_task_cnt_decrease(dev_id);
    return 0;
out:
    dms_hotreset_task_cnt_decrease(dev_id);
    return -EFAULT;
}

STATIC int devdrv_manager_get_device_startup_status(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    unsigned int phys_id = DEVDRV_MAX_DAVINCI_NUM;
    unsigned int vfid = 0;
    struct devdrv_info *dev_info = NULL;
    struct devdrv_device_work_status para = {0};

    ret = copy_from_user_safe(&para, (void *)(uintptr_t)arg, sizeof(struct devdrv_device_work_status));
    if (ret != 0) {
        devdrv_drv_err("copy from user failed, ret(%d).\n", ret);
        return -EINVAL;
    }

    ret = devdrv_manager_container_logical_id_to_physical_id(para.device_id, &phys_id, &vfid);
    if (ret != 0) {
        devdrv_drv_err("can't transfor virt id %u \n", para.device_id);
        return ret;
    }

    dev_info = devdrv_manager_get_devdrv_info(phys_id);
    if ((dev_info == NULL) || (dev_info->shm_head == NULL) || (dev_info->shm_status == NULL) ||
        (dev_info->shm_head->head_info.magic != DEVMNG_SHM_INFO_HEAD_MAGIC)) {
        para.dmp_started = false;
        para.device_process_status = DSMI_BOOT_STATUS_UNINIT;
    } else if (dev_info->shm_head->head_info.version != DEVMNG_SHM_INFO_HEAD_VERSION) {
        devdrv_drv_err("dev(%u) version of share memory in host is 0x%llx, "
                       "the version in device is 0x%llx, magic is 0x%x.\n",
                       dev_info->dev_id, DEVMNG_SHM_INFO_HEAD_VERSION,
                       dev_info->shm_head->head_info.version,
                       dev_info->shm_head->head_info.magic);
        return -EINVAL;
    } else {
        if (dev_info->dmp_started == false) {
            dev_info->dmp_started = devdrv_manager_h2d_query_dmp_started(phys_id);
        }

        para.dmp_started = dev_info->dmp_started;
        para.device_process_status = (unsigned int)dev_info->shm_status->os_status;
    }

    ret = copy_to_user_safe((void *)(uintptr_t)arg, &para, sizeof(struct devdrv_device_work_status));
    if (ret != 0) {
        devdrv_drv_err("copy to user failed, ret(%d)\n", ret);
        return -EINVAL;
    }

    return 0;
}

int devdrv_manager_get_docker_id(u32 *docker_id)
{
    if (devdrv_manager_container_is_in_container()) {
        return devdrv_manager_container_get_docker_id(docker_id);
    } else {
        *docker_id = MAX_DOCKER_NUM;
        return 0;
    }
}
EXPORT_SYMBOL(devdrv_manager_get_docker_id);

STATIC int devdrv_manager_get_container_pid(pid_t hostpid, pid_t *container_pid)
{
    struct task_struct *tsk = NULL;
    struct pid *pgrp = NULL;

    rcu_read_lock();
    for_each_process(tsk) {
        if ((tsk != NULL) && (tsk->pid == hostpid)) {
            pgrp = task_pid(tsk);
            if (pgrp == NULL) {
                rcu_read_unlock();
                devdrv_drv_err("The pgrp parameter is NULL.\n");
                return -EINVAL;
            }
            *container_pid = pgrp->numbers[pgrp->level].nr;
            break;
        }
    }
    rcu_read_unlock();

    return 0;
}

STATIC int devdrv_manager_get_hostpid(pid_t container_pid, pid_t *hostpid)
{
    struct pid *pgrp = NULL;

    if (devdrv_manager_container_is_in_container()) {
        pgrp = find_get_pid(container_pid);
        if (pgrp == NULL) {
            devdrv_drv_err("The pgrp parameter is NULL.\n");
            return -EINVAL;
        }
        *hostpid = pgrp->numbers[0].nr; /* 0:hostpid */
        put_pid(pgrp);
    } else {
        *hostpid = container_pid;
    }

    return 0;
}

devmm_get_device_accounting_pids_ops get_device_pids_from_devmm = NULL;
int devdrv_manager_get_process_pids_register(devmm_get_device_accounting_pids_ops func)
{
    if (func == NULL) {
        devdrv_drv_err("Register pids operation function null.\n");
        return -EINVAL;
    }

    get_device_pids_from_devmm = func;
    return 0;
}
EXPORT_SYMBOL(devdrv_manager_get_process_pids_register);

void devdrv_manager_get_process_pids_unregister(void)
{
    get_device_pids_from_devmm = NULL;
}
EXPORT_SYMBOL(devdrv_manager_get_process_pids_unregister);

STATIC int devdrv_manager_get_accounting_pid(u32 phyid, u32 vfid, struct devdrv_resource_info *dinfo)
{
    int ret;
    pid_t pid = -1;
    u32 i;
    u32 docker_id = 0;
    int out_count = 0;

    if (dinfo->buf_len > DEVDRV_MAX_PAYLOAD_LEN) {
        dinfo->buf_len = DEVDRV_MAX_PAYLOAD_LEN;
    }
    if (get_device_pids_from_devmm == NULL) {
        devdrv_drv_err("The devmm_get_device_accounting_pids is NULL.\n");
        return -EINVAL;
    }

    ret = devdrv_manager_get_docker_id(&docker_id);
    if (ret) {
        devdrv_drv_err("The devdrv_manager_container_get_docker_id failed. (devid=%u; docker_id=%d; ret=%d)\n",
            dinfo->devid, docker_id, ret);
        return ret;
    }

    out_count = get_device_pids_from_devmm(docker_id, phyid, vfid,
        (pid_t *)dinfo->buf, (dinfo->buf_len / sizeof(pid_t)));
    if (out_count < 0) {
        devdrv_drv_err("Failed to obtain the PID list of the process from SVM. (devid=%u; docker_id=%d; count=%d)\n",
            dinfo->devid, docker_id, out_count);
        return -EINVAL;
    }
    if (out_count > DEVDRV_MAX_PAYLOAD_LEN / sizeof(pid_t)) {
        out_count = DEVDRV_MAX_PAYLOAD_LEN / sizeof(pid_t);
    }

    if (docker_id < MAX_DOCKER_NUM) {
        for (i = 0; i < out_count; i++) {
            ret = devdrv_manager_get_container_pid(((pid_t *)dinfo->buf)[i], &pid);
            if (ret) {
                devdrv_drv_err("devdrv_manager_get_container_pid failed. (devid=%u; hostpid=%d; ret=%d)\n",
                    dinfo->devid, ((u32 *)dinfo->buf)[i], ret);
                return ret;
            }
            ((pid_t *)dinfo->buf)[i] = pid;
        }
    }
    dinfo->buf_len = out_count * sizeof(pid_t);

    return 0;
}

STATIC int devdrv_manager_get_process_memory(u32 fid, u32 phyid, struct devdrv_resource_info *dinfo)
{
    struct devdrv_manager_msg_resource_info resource_info;
    int ret;
    pid_t hostpid = -1;

    if (fid > VMNG_VDEV_MAX_PER_PDEV || dinfo->buf_len > DEVDRV_MAX_PAYLOAD_LEN) {
        devdrv_drv_err("Invaild parameter. (fid=%d; buf_len=%d)\n", fid, dinfo->buf_len);
        return -EINVAL;
    }

    ret = devdrv_manager_get_hostpid(dinfo->owner_id, &hostpid);
    if (ret) {
        devdrv_drv_err("devdrv_manager_get_hostpid failed. (owner_id=%d; hostpid=%d; ret=%d;)\n",
            dinfo->owner_id, hostpid, ret);
        return -EINVAL;
    }

    resource_info.info_type = dinfo->resource_type;
    resource_info.owner_id = hostpid;
    resource_info.vfid = fid;
    ret = devdrv_manager_h2d_query_resource_info(phyid, &resource_info);
    if (ret) {
        devdrv_drv_err("devdrv_manager_h2d_query_resource_info failed. (devid=%u; info_type=%d; ret=%d)\n",
            dinfo->devid, dinfo->resource_type, ret);
        return ret;
    }
    *((u64 *)dinfo->buf) = resource_info.value;
    if (dinfo->buf_len > sizeof(u64)) {
        dinfo->buf_len = sizeof(u64);
    }

    return 0;
}

STATIC int devdrv_manager_get_process_resource_info(struct devdrv_resource_info *dinfo)
{
    u32 phy_id = 0;
    u32 vfid = 0;
    int ret;

    if (dinfo->devid > DEVDRV_MAX_DAVINCI_NUM || dinfo->buf == NULL) {
        devdrv_drv_err("Invalid parameter. (devid=%u; buff=%pK)\n", dinfo->devid, dinfo->buf);
        return -EINVAL;
    }
    ret = devdrv_manager_container_logical_id_to_physical_id(dinfo->devid, &phy_id, &vfid);
    if (ret) {
        devdrv_drv_err("Logical id to physical id failed. (devid=%u; phy_id=%d; ret=%d)\n", dinfo->devid, phy_id, ret);
        return ret;
    }

    switch (dinfo->resource_type) {
        case DEVDRV_DEV_PROCESS_PID:
            ret = devdrv_manager_get_accounting_pid(phy_id, vfid, dinfo);
            break;
        case DEVDRV_DEV_PROCESS_MEM:
            ret = devdrv_manager_get_process_memory(vfid, phy_id, dinfo);
            break;
        default:
            devdrv_drv_err("Invalid dev process resource type. (type=%d)", dinfo->resource_type);
            return -EINVAL;
    }

    if (ret) {
        devdrv_drv_err("devdrv_manager_get_process_resource_info falied. (ret=%d; type=%d)\n",
            ret, dinfo->resource_type);
    }
    return ret;
}

STATIC int (*const dmanage_get_resource_handler[DEVDRV_MAX_OWNER_TYPE])(struct devdrv_resource_info *dinfo) = {
    [DEVDRV_DEV_RESOURCE] = NULL,
    [DEVDRV_VDEV_RESOURCE] = NULL,
    [DEVDRV_PROCESS_RESOURCE] = devdrv_manager_get_process_resource_info,
};

int devdrv_manager_ioctl_get_dev_resource_info(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_resource_info dinfo = {0};
    int ret;

    if (copy_from_user_safe(&dinfo, (void *)(uintptr_t)arg, sizeof(dinfo))) {
        return -EFAULT;
    }

    if (dinfo.owner_type >= DEVDRV_MAX_OWNER_TYPE) {
        devdrv_drv_err("devid(%u) invalid owner_type(%u).\n", dinfo.devid, dinfo.owner_type);
        return -EINVAL;
    }

    if (dmanage_get_resource_handler[dinfo.owner_type] == NULL) {
        devdrv_drv_err("devid(%u) unsupported input owner_type(%d).\n", dinfo.devid, dinfo.owner_type);
        return -EOPNOTSUPP;
    }

    ret = dmanage_get_resource_handler[dinfo.owner_type](&dinfo);
    if (ret) {
        devdrv_drv_err("get dev resource info failed devid(%u) owner_type(%u), ret = %d.\n",
            dinfo.devid, dinfo.owner_type, ret);
        return ret;
    }

    if (copy_to_user_safe((void *)(uintptr_t)arg, &dinfo, sizeof(dinfo))) {
        devdrv_drv_err("copy to user failed.\n");
        return -EFAULT;
    }

    return 0;
}

STATIC int (* const devdrv_manager_ioctl_handlers[DEVDRV_MANAGER_CMD_MAX_NR])(struct file *filep, unsigned int cmd,
    unsigned long arg) = {
        [_IOC_NR(DEVDRV_MANAGER_GET_PCIINFO)] = devdrv_manager_get_pci_info,
        [_IOC_NR(DEVDRV_MANAGER_GET_DEVNUM)] = devdrv_manager_ioctl_get_devnum,
        [_IOC_NR(DEVDRV_MANAGER_GET_PLATINFO)] = devdrv_manager_ioctl_get_plat_info,
        [_IOC_NR(DEVDRV_MANAGER_SVMVA_TO_DEVID)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_CHANNELINFO)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_DEVICE_STATUS)] = devdrv_manager_get_device_status,
        [_IOC_NR(DEVDRV_MANAGER_GET_CORE_SPEC)] = devdrv_manager_get_core,
        [_IOC_NR(DEVDRV_MANAGER_GET_CORE_INUSE)] = devdrv_manager_get_core,
        [_IOC_NR(DEVDRV_MANAGER_GET_DEVIDS)] = devdrv_manager_devinfo_ioctl,
        [_IOC_NR(DEVDRV_MANAGER_GET_DEVINFO)] = devdrv_manager_devinfo_ioctl,
        [_IOC_NR(DEVDRV_MANAGER_GET_DEVID_BY_LOCALDEVID)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_DEV_INFO_BY_PHYID)] = devdrv_manager_devinfo_ioctl,
        [_IOC_NR(DEVDRV_MANAGER_GET_PCIE_ID_INFO)] = devdrv_manager_devinfo_ioctl,
        [_IOC_NR(DEVDRV_MANAGER_GET_CORE_UTILIZATION)] = devdrv_manager_devinfo_ioctl,
        [_IOC_NR(DEVDRV_MANAGER_GET_FLASH_COUNT)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_FLASH_INFO)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_VOLTAGE)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_TEMPERATURE)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_TSENSOR)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_AI_USE_RATE)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_FREQUENCY)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_POWER)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_HEALTH_CODE)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_ERROR_CODE)] = devdrv_get_error_code,
        [_IOC_NR(DEVDRV_MANAGER_GET_DDR_CAPACITY)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_LPM3_SMOKE)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_BLACK_BOX_GET_EXCEPTION)] = devdrv_manager_black_box_get_exception,
        [_IOC_NR(DEVDRV_MANAGER_DEVICE_MEMORY_DUMP)] = devdrv_manager_device_memory_dump,
        [_IOC_NR(DEVDRV_MANAGER_DEVICE_RESET_INFORM)] = devdrv_manager_device_reset_inform,
        [_IOC_NR(DEVDRV_MANAGER_GET_MODULE_STATUS)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_DEVICE_DMA_ADDR)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_INTERRUPT_INFO)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_MINI_DEVID)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_MINI_BOARD_ID)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_PCIE_PRE_RESET)] = devdrv_manager_pcie_pre_reset,
        [_IOC_NR(DEVDRV_MANAGER_PCIE_RESCAN)] = devdrv_manager_pcie_rescan,
        [_IOC_NR(DEVDRV_MANAGER_ALLOC_HOST_DMA_ADDR)] = drv_dma_mmap,
        [_IOC_NR(DEVDRV_MANAGER_FREE_HOST_DMA_ADDR)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_PCIE_SRAM_READ)] = drv_pcie_sram_read,
        [_IOC_NR(DEVDRV_MANAGER_PCIE_SRAM_WRITE)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_EMMC_VOLTAGE)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_DEVICE_BOOT_STATUS)] = drv_get_device_boot_status,
        [_IOC_NR(DEVDRV_MANAGER_ENABLE_EFUSE_LDO)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_DISABLE_EFUSE_LDO)] = NULL,

        [_IOC_NR(DEVDRV_MANAGER_CONTAINER_CMD)] = devdrv_manager_container_cmd,
        [_IOC_NR(DEVDRV_MANAGER_GET_HOST_PHY_MACH_FLAG)] = devdrv_manager_get_host_phy_mach_flag,
        [_IOC_NR(DEVDRV_MANAGER_GET_KERNEL_LIB)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_LOCAL_DEVICEIDS)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_IMU_SMOKE)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_SYSTEM_RDY_CHECK)] = devdrv_manager_all_devices_rdy_check,
        [_IOC_NR(DEVDRV_MANAGER_SET_NEW_TIME)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_PCIE_DDR_READ)] = drv_pcie_bbox_ddr_read,
        [_IOC_NR(DEVDRV_MANAGER_GET_CPU_INFO)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_INVAILD_TLB)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_SEND_TO_IMU)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_RECV_FROM_IMU)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_IMU_INFO)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_CONFIG_ECC_ENABLE)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_PROBE_NUM)] = devdrv_manager_get_probe_num,
        [_IOC_NR(DEVDRV_MANAGER_GET_PROBE_LIST)] = devdrv_manager_get_probe_list,
        [_IOC_NR(DEVDRV_MANAGER_DEBUG_INFORM)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_COMPUTE_POWER)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_SYNC_MATRIX_DAEMON_READY)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_BBOX_ERRSTR)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_FLASH_USER_CMD)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_FLASH_ROOT_CMD)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_PCIE_BBOX_HDR_READ)] = drv_pcie_bbox_hdr_read,
        [_IOC_NR(DEVDRV_MANAGER_APPMON_BBOX_EXCEPTION_CMD)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_BOOT_DEV_ID)] = devdrv_manager_online_get_devids,
        [_IOC_NR(DEVDRV_MANAGER_GET_PROCESS_SIGN)] = devdrv_manager_get_process_sign,
        [_IOC_NR(DEVDRV_MANAGER_PCIE_HOT_RESET)] = devdrv_manager_pcie_hot_reset,
        [_IOC_NR(DEVDRV_MANAGER_BIND_PID_ID)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_QUERY_DEV_PID)] = devdrv_host_query_devpid,
        [_IOC_NR(DEVDRV_MANAGER_GET_H2D_DEVINFO)] = devdrv_manager_devinfo_ioctl,
        [_IOC_NR(DEVDRV_MANAGER_GET_CONSOLE_LOG_LEVEL)] = devdrv_manager_ioctl_get_console_loglevel,
        [_IOC_NR(DEVDRV_MANAGER_GET_CONTAINER_FLAG)] = devdrv_manager_get_container_flag,
        [_IOC_NR(DEVDRV_MANAGER_BOARD_INFO_MEM)] = devdrv_manager_board_info_mem,
        [_IOC_NR(DEVDRV_MANAGER_GET_STARTUP_STATUS)] = devdrv_manager_get_device_startup_status,
        [_IOC_NR(DEVDRV_MANAGER_QUERY_HOST_PID)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_DEV_RESOURCE_INFO)] = devdrv_manager_ioctl_get_dev_resource_info,
        [_IOC_NR(DEVDRV_MANAGER_CONFIG_DEVICE_SHARE)] = devdrv_manager_config_device_share,
    };

STATIC long devdrv_manager_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
    if (dev_manager_info == NULL) {
        devdrv_drv_err("invalid parameter, "
            "dev_manager_info = %pK, "
            "arg = 0x%pK\n",
            dev_manager_info, (void *)(uintptr_t)arg);
        return -EINVAL;
    }

    if (_IOC_NR(cmd) >= DEVDRV_MANAGER_CMD_MAX_NR) {
        devdrv_drv_err("invalid cmd, cmd = %u\n", _IOC_NR(cmd));
        return -EINVAL;
    }

    if (devdrv_manager_ioctl_handlers[_IOC_NR(cmd)] == NULL) {
        devdrv_drv_err("invalid cmd, cmd = %u\n", _IOC_NR(cmd));
        return -EINVAL;
    }

    return devdrv_manager_ioctl_handlers[_IOC_NR(cmd)](filep, cmd, arg);
}

STATIC ssize_t devdrv_manager_read(struct file *filep, char __user *buf, size_t count, loff_t *ppos)
{
    return 0;
}

const struct file_operations devdrv_manager_file_operations = {
    .owner = THIS_MODULE,
    .read = devdrv_manager_read,
    .open = devdrv_manager_open,
    .release = devdrv_manager_release,
    .unlocked_ioctl = devdrv_manager_ioctl,
    .poll = devdrv_manager_poll,
};

STATIC int devdrv_manager_get_dev_resource(struct devdrv_info *dev_info)
{
    struct devdrv_platform_data *pdata = NULL;
    struct devdrv_pci_dev_info dev_pci_info;
    u32 dev_id = dev_info->pci_dev_id;

    pdata = dev_info->pdata;
    if (devdrv_get_pci_dev_info(dev_id, &dev_pci_info)) {
        devdrv_drv_err("devdrv_get_pci_dev_info failed. dev_id(%u)\n", dev_id);
        return -EBUSY;
    }
    pdata->pci_info.bus_number = dev_pci_info.bus_no;
    pdata->pci_info.dev_number = dev_pci_info.device_no;
    pdata->pci_info.function_number = dev_pci_info.function_no;

    return 0;
}

int devdrv_manager_send_msg(struct devdrv_info *dev_info, struct devdrv_manager_msg_info *dev_manager_msg_info,
    int *out_len)
{
    int ret;

    ret = devdrv_common_msg_send(dev_info->pci_dev_id, dev_manager_msg_info, sizeof(struct devdrv_manager_msg_info),
        sizeof(struct devdrv_manager_msg_info), (u32 *)out_len, DEVDRV_COMMON_MSG_DEVDRV_MANAGER);

    return ret;
}

STATIC int devdrv_manager_send_devid(struct devdrv_info *dev_info)
{
    struct devdrv_manager_msg_info dev_manager_msg_info;
    u32 out_len;
    int ret;
    u32 i;

    dev_manager_msg_info.header.msg_id = DEVDRV_MANAGER_CHAN_H2D_SEND_DEVID;
    dev_manager_msg_info.header.valid = DEVDRV_MANAGER_MSG_VALID;

    /* give a random value for checking result later */
    dev_manager_msg_info.header.result = (u16)DEVDRV_MANAGER_MSG_INVALID_RESULT;

    /* inform corresponding devid to device side */
    dev_manager_msg_info.header.dev_id = dev_info->dev_id;

    for (i = 0; i < DEVDRV_MANAGER_INFO_PAYLOAD_LEN; i++) {
        dev_manager_msg_info.payload[i] = 0;
    }

    ret = devdrv_common_msg_send(dev_info->pci_dev_id, &dev_manager_msg_info, sizeof(dev_manager_msg_info),
        sizeof(dev_manager_msg_info), &out_len, DEVDRV_COMMON_MSG_DEVDRV_MANAGER);
    if (ret || dev_manager_msg_info.header.result != 0) {
        devdrv_drv_err("send dev_id to device failed, dev_id %u\n", dev_info->dev_id);
        return -EFAULT;
    }

    return 0;
}

int devdrv_manager_register(struct devdrv_info *dev_info)
{
    int ret;

    if (dev_info == NULL) {
        devdrv_drv_err("devdrv manager has not initialized\n");
        return -EINVAL;
    }

    if (devdrv_manager_get_devdrv_info(dev_info->dev_id) != NULL) {
        devdrv_drv_err("device has already registered, dev_id = %u\n", dev_info->dev_id);
        return -ENODEV;
    }

    if (devdrv_manager_get_dev_resource(dev_info)) {
        devdrv_drv_err("devdrv manager get resource failed, dev_id = %u\n", dev_info->dev_id);
        return -EINVAL;
    }
    dev_info->drv_ops = &devdrv_host_drv_ops;

    if (devdrv_manager_set_devinfo_inc_devnum(dev_info->dev_id, dev_info)) {
        devdrv_drv_err("devdrv_manager_set_devinfo_inc_devnum failed, dev_id = %u\n", dev_info->dev_id);
        return -EFAULT;
    }

    if (devdrv_manager_send_devid(dev_info)) {
        devdrv_drv_err("send devid to device failed, dev_id = %u\n", dev_info->dev_id);
        (void)devdrv_manager_reset_devinfo_dec_devnum(dev_info->dev_id);
        return -ENODEV;
    }

    if (dms_device_register(dev_info)) {
        devdrv_drv_err("Dms device register failed. (dev_id=%u)\n", dev_info->dev_id);
        (void)devdrv_manager_reset_devinfo_dec_devnum(dev_info->dev_id);
        return -EINVAL;
    }

    ret = devdrv_set_module_init_finish(dev_info->dev_id, DEVDRV_HOST_MODULE_DEVMNG);
    if (ret) {
        devdrv_drv_err("set module init finish failed, dev_id = %u\n", dev_info->dev_id);
        (void)devdrv_manager_reset_devinfo_dec_devnum(dev_info->dev_id);
        return -EFAULT;
    }

    devdrv_drv_info("devdrv_manager_register succ, dev_id = %u\n", dev_info->dev_id);
    return 0;
}
EXPORT_SYMBOL(devdrv_manager_register);

void devdrv_manager_unregister(struct devdrv_info *dev_info)
{
    devdrv_drv_debug("devdrv_manager_unregister started.\n");

    if (dev_info == NULL || dev_manager_info == NULL || dev_info->dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("devdrv_manager_unregister fail!\n");
        return;
    }

    if (dev_manager_info->dev_info[dev_info->dev_id] == NULL) {
        devdrv_drv_err("device id %u is not initialized\n", dev_info->dev_id);
        return;
    }

    if (devdrv_manager_reset_devinfo_dec_devnum(dev_info->dev_id)) {
        devdrv_drv_err("devdrv_manager_unregister fail !!!!!!!\n");
        return;
    }

    dms_device_unregister(dev_info);

    devdrv_drv_debug("devdrv_manager_unregister finished, "
        "dev_manager_info->num_dev = %d\n",
        dev_manager_info->num_dev);
}
EXPORT_SYMBOL(devdrv_manager_unregister);

STATIC int devdrv_manager_device_ready(void *msg, u32 *ack_len)
{
    struct devdrv_manager_msg_info *dev_manager_msg_info = NULL;
    struct devdrv_device_info *drv_info = NULL;
    struct devdrv_info *dev_info = NULL;
    u32 dev_id;
    u32 i;

    dev_manager_msg_info = (struct devdrv_manager_msg_info *)msg;
    dev_id = dev_manager_msg_info->header.dev_id;

    if (dev_manager_msg_info->header.valid != DEVDRV_MANAGER_MSG_VALID) {
        devdrv_drv_err("invalid message from host\n");
        return -EINVAL;
    }

    if (dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("invalid dev_id, dev_id = %d\n", dev_id);
        return -ENODEV;
    }

    /* dev send message with pcie device id, get dev_info from devdrv_info_array */
    dev_info = devdrv_get_devdrv_info_array(dev_id);
    if (dev_info == NULL) {
        devdrv_drv_warn("device(%u) is not ready.\n", dev_id);
        return -EINVAL;
    }

    if (dev_info->dev_ready != 0) {
        devdrv_drv_info("device already informed,device id : %u\n", dev_id);
        return -ENODEV;
    }

    dev_info->dev_ready = DEVDRV_DEV_READY_EXIST;

    drv_info = (struct devdrv_device_info *)dev_manager_msg_info->payload;
    if (drv_info->ts_load_fail == 0) {
        devdrv_drv_info("*** ts fw load succeed, set device ready work  device id : %u***\n", dev_id);
        tsdrv_set_ts_status(dev_info->dev_id, 0, TS_WORK);
        dev_info->dev_ready = DEVDRV_DEV_READY_WORK;
    }

    dev_info->ai_core_num = drv_info->ai_core_num;
    dev_info->ai_cpu_core_num = drv_info->ai_cpu_core_num;
    dev_info->ctrl_cpu_core_num = drv_info->ctrl_cpu_core_num;
    dev_info->ctrl_cpu_occupy_bitmap = drv_info->ctrl_cpu_occupy_bitmap;
    dev_info->ctrl_cpu_endian_little = drv_info->ctrl_cpu_endian_little;
    dev_info->ctrl_cpu_id = drv_info->ctrl_cpu_id;
    dev_info->ctrl_cpu_ip = drv_info->ctrl_cpu_ip;
    dev_info->pdata->ts_pdata[0].ts_cpu_core_num = drv_info->ts_cpu_core_num;

    dev_info->env_type = drv_info->env_type;
    dev_info->ai_core_id = drv_info->ai_core_id;
    dev_info->ai_cpu_core_id = drv_info->ai_cpu_core_id;
    dev_info->aicpu_occupy_bitmap = drv_info->aicpu_occupy_bitmap;

    dev_info->inuse.ai_core_num = drv_info->ai_core_ready_num;
    dev_info->inuse.ai_core_error_bitmap = drv_info->ai_core_broken_map;
    dev_info->inuse.ai_cpu_num = drv_info->ai_cpu_ready_num;
    dev_info->inuse.ai_cpu_error_bitmap = drv_info->ai_cpu_broken_map;
    dev_info->ai_subsys_ip_broken_map = drv_info->ai_subsys_ip_map;
    dev_info->hardware_version = drv_info->hardware_version;

    dev_info->chip_name = drv_info->chip_name;
    dev_info->chip_version = drv_info->chip_version;
    dev_info->chip_info = drv_info->chip_info;
    devdrv_drv_info("Initialize chip info. (chip_name=%u; chip_version=%u)\n",
        dev_info->chip_name, dev_info->chip_version);

    dev_info->dump_ddr_dma_addr = drv_info->dump_ddr_dma_addr;
    dev_info->dump_ddr_size = drv_info->dump_ddr_size;

    devdrv_drv_info("ai cpu num: %d, "
        "ai cpu broken bitmap: 0x%x, "
        "ai core num: %d,"
        "ai core broken bitmap: 0x%x, "
        "ai subsys broken map: 0x%x.\n",
        drv_info->ai_cpu_ready_num, drv_info->ai_cpu_broken_map, drv_info->ai_core_ready_num,
        drv_info->ai_core_broken_map, drv_info->ai_subsys_ip_map);
    if (dev_info->inuse.ai_cpu_error_bitmap) {
        for (i = 0; i < dev_info->ai_cpu_core_num; i++) {
            if (dev_info->inuse.ai_cpu_error_bitmap & (0x01U << i)) {
                devdrv_drv_err("ai cpu: %d init failed.\n", i);
            }
        }
    }

    if (dev_info->inuse.ai_core_error_bitmap) {
        for (i = 0; i < dev_info->ai_core_num; i++) {
            if (dev_info->inuse.ai_core_error_bitmap & (0x01U << i)) {
                devdrv_drv_err("ai core: %d init failed.\n", i);
            }
        }
    }

    devdrv_drv_debug("received ready message from pcie device, dev_id= %d\n", dev_id);
    devdrv_drv_debug("pcie dev_id = %d ready message received from device\n", dev_id);
    devdrv_drv_debug(" ai_core_num = %d, ai_cpu_core_num = %d, "
        "ctrl_cpu_core_num = %d, ctrl_cpu_endian_little = %d, "
        "ctrl_cpu_id = %d, ctrl_cpu_ip = %d, "
        "ts_cpu_core_num = %d, "
        "ai_core_id = %d, "
        "ai_cpu_core_id = %d\n",
        dev_info->ai_core_num, dev_info->ai_cpu_core_num, dev_info->ctrl_cpu_core_num, dev_info->ctrl_cpu_endian_little,
        dev_info->ctrl_cpu_id, dev_info->ctrl_cpu_ip,
        dev_info->pdata->ts_pdata[0].ts_cpu_core_num,
        dev_info->ai_core_id,
        dev_info->ai_cpu_core_id);
    devdrv_drv_debug("env_type = %d\n", dev_info->env_type);

    *ack_len = sizeof(*dev_manager_msg_info);
    dev_manager_msg_info->header.result = 0;

    if (dev_manager_info != NULL) {
        devdrv_drv_info("device %d ready\n", dev_id);
        queue_work(dev_manager_info->dev_rdy_work, &dev_info->work);
    }

    return 0;
}

STATIC int devdrv_manager_device_inform_handler(void *msg, u32 *ack_len, enum devdrv_ts_status status)
{
    struct devdrv_manager_msg_info *dev_manager_msg_info = NULL;
    struct devdrv_info *dev_info = NULL;
    u32 dev_id;
    int ret;

    dev_manager_msg_info = (struct devdrv_manager_msg_info *)msg;
    if (dev_manager_msg_info->header.valid != DEVDRV_MANAGER_MSG_VALID) {
        devdrv_drv_err("invalid message from host\n");
        return -EINVAL;
    }

    dev_id = dev_manager_msg_info->header.dev_id;
    if (dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("invalid dev_id, dev_id = %d\n", dev_id);
        return -ENODEV;
    }

    dev_info = devdrv_get_devdrv_info_array(dev_id);
    if (dev_info == NULL) {
        devdrv_drv_err("invalid device id\n");
        return -EINVAL;
    }

    switch (status) {
        case TS_DOWN:
            devdrv_drv_err("receive ts exception message from device.\n");
            devdrv_host_manager_device_exception(dev_info);
            ret = 0;
            break;
        case TS_WORK:
            devdrv_drv_err("receive ts resume message from device.\n");
            ret = devdrv_host_manager_device_resume(dev_info);
            break;
        case TS_FAIL_TO_SUSPEND:
            devdrv_drv_err("receive ts fail to suspend message from device.\n");
            ret = devdrv_host_manager_device_resume(dev_info);
            break;
        case TS_SUSPEND:
            devdrv_drv_err("receive ts suspend message from device.\n");
            ret = devdrv_host_manager_device_suspend(dev_info);
            break;
        default:
            devdrv_drv_err("invalid input ts status.\n");
            return -EINVAL;
    }

    *ack_len = sizeof(*dev_manager_msg_info);
    /* return result */
    dev_manager_msg_info->header.result = ret;
    return 0;
}

STATIC int devdrv_manager_device_down(void *msg, u32 *ack_len)
{
    return devdrv_manager_device_inform_handler(msg, ack_len, TS_DOWN);
}

STATIC int devdrv_manager_device_suspend(void *msg, u32 *ack_len)
{
    return devdrv_manager_device_inform_handler(msg, ack_len, TS_SUSPEND);
}

STATIC int devdrv_manager_device_resume(void *msg, u32 *ack_len)
{
    return devdrv_manager_device_inform_handler(msg, ack_len, TS_WORK);
}

STATIC int devdrv_manager_device_fail_to_suspend(void *msg, u32 *ack_len)
{
    return devdrv_manager_device_inform_handler(msg, ack_len, TS_FAIL_TO_SUSPEND);
}

STATIC int devdrv_manager_check_process_sign(void *msg, u32 *ack_len)
{
    struct devdrv_manager_msg_info *dev_manager_msg_info = NULL;
    int ret = DEVDRV_MANAGER_MSG_INVALID_RESULT;
    struct devdrv_manager_info *d_info = NULL;
    struct devdrv_process_sign *d_sign = NULL;
    struct process_sign *process_sign = NULL;
    struct list_head *pos = NULL;
    struct list_head *n = NULL;
    u32 dev_id;

    dev_manager_msg_info = (struct devdrv_manager_msg_info *)msg;
    if (dev_manager_msg_info->header.valid != DEVDRV_MANAGER_MSG_VALID) {
        devdrv_drv_err("invalid message from host\n");
        return -EINVAL;
    }

    dev_id = dev_manager_msg_info->header.dev_id;
    if ((dev_id >= DEVDRV_MAX_DAVINCI_NUM)) {
        devdrv_drv_err("invalid dev_id(%u).\n", dev_id);
        return -EINVAL;
    }
    d_info = devdrv_get_manager_info();
    if (d_info == NULL) {
        devdrv_drv_err("d_info is null.\n");
        return -EINVAL;
    }
    process_sign = (struct process_sign *)dev_manager_msg_info->payload;
    mutex_lock(&d_info->devdrv_sign_list_lock);
    if (list_empty_careful(&d_info->hostpid_list_header)) {
        devdrv_drv_err("list is empty.\n");
        goto out;
    }
    list_for_each_safe(pos, n, &d_info->hostpid_list_header)
    {
        d_sign = list_entry(pos, struct devdrv_process_sign, list);
        if (d_sign->hostpid == process_sign->tgid) {
            ret = devdrv_manager_container_check_devid_in_container(dev_id, d_sign->hostpid);
            if (ret) {
                devdrv_drv_err("dev_id(%u) and hostpid(%d) mismatch in container, ret(%d).\n", dev_id, d_sign->hostpid,
                    ret);
            }
            goto out;
        }
    }
    devdrv_drv_err("hostpid(%d) is error.\n", process_sign->tgid);

out:
    mutex_unlock(&d_info->devdrv_sign_list_lock);
    *ack_len = sizeof(*dev_manager_msg_info);
    dev_manager_msg_info->header.result = ret;
    return ret;
}

STATIC int devdrv_manager_get_pcie_id(void *msg, u32 *ack_len)
{
    struct devdrv_manager_msg_info *dev_manager_msg_info = NULL;
    struct devdrv_pcie_id_info pcie_id_info = { 0 };
    struct devdrv_pcie_id_info *host_pcie_id_info = NULL;
    struct devdrv_info *dev_info = NULL;
    u32 dev_id;
    int ret;

    dev_manager_msg_info = (struct devdrv_manager_msg_info *)msg;
    if (dev_manager_msg_info->header.valid != DEVDRV_MANAGER_MSG_VALID) {
        devdrv_drv_err("invalid message from host\n");
        return -EINVAL;
    }

    dev_id = dev_manager_msg_info->header.dev_id;
    if (dev_id >= DEVDRV_MAX_DAVINCI_NUM || devdrv_info_array[dev_id] == NULL) {
        devdrv_drv_err("invalid device id, dev_id=%u\n", dev_id);
        return -EINVAL;
    }

    dev_info = devdrv_info_array[dev_id];

    ret = devdrv_get_pcie_id_info(dev_info->pci_dev_id, &pcie_id_info);
    if (ret) {
        devdrv_drv_err("devdrv_manager_get_pcie_id_info failed.\n");
        goto out;
    }

    host_pcie_id_info = (struct devdrv_pcie_id_info *)dev_manager_msg_info->payload;

    host_pcie_id_info->venderid = pcie_id_info.venderid;
    host_pcie_id_info->subvenderid = pcie_id_info.subvenderid;
    host_pcie_id_info->deviceid = pcie_id_info.deviceid;
    host_pcie_id_info->subdeviceid = pcie_id_info.subdeviceid;
    host_pcie_id_info->bus = pcie_id_info.bus;
    host_pcie_id_info->device = pcie_id_info.device;
    host_pcie_id_info->fn = pcie_id_info.fn;

out:
    *ack_len = sizeof(*dev_manager_msg_info);
    /* return result */
    dev_manager_msg_info->header.result = ret;
    return 0;
}

STATIC int devdrv_manager_d2h_sync_matrix_ready(void *msg, u32 *ack_len)
{
    struct devdrv_manager_msg_info *dev_manager_msg_info = NULL;
    u32 dev_id;
    int ret = 0;

    dev_manager_msg_info = (struct devdrv_manager_msg_info *)msg;
    if (dev_manager_msg_info->header.valid != DEVDRV_MANAGER_MSG_VALID) {
        devdrv_drv_err("invalid message from host\n");
        return -EINVAL;
    }

    dev_id = dev_manager_msg_info->header.dev_id;
    if (dev_id >= DEVDRV_MAX_DAVINCI_NUM || devdrv_info_array[dev_id] == NULL) {
        devdrv_drv_err("invalid device id, dev_id=%u\n", dev_id);
        return -EINVAL;
    }

    *ack_len = sizeof(*dev_manager_msg_info);
    dev_manager_msg_info->header.result = ret;
    return 0;
}

STATIC int (*devdrv_manager_chan_msg_processes[])(void *msg, u32 *ack_len) = {
    [DEVDRV_MANAGER_CHAN_D2H_DEVICE_READY] = devdrv_manager_device_ready,
    [DEVDRV_MANAGER_CHAN_D2H_DOWN] = devdrv_manager_device_down,
    [DEVDRV_MANAGER_CHAN_D2H_SUSNPEND] = devdrv_manager_device_suspend,
    [DEVDRV_MANAGER_CHAN_D2H_RESUME] = devdrv_manager_device_resume,
    [DEVDRV_MANAGER_CHAN_D2H_FAIL_TO_SUSPEND] = devdrv_manager_device_fail_to_suspend,
    [DEVDRV_MANAGER_CHAN_D2H_CORE_INFO] = NULL,
    [DEVDRV_MANAGER_CHAN_D2H_GET_PCIE_ID_INFO] = devdrv_manager_get_pcie_id,
    /* [DEVDRV_MANAGER_CHAN_H2D_HEART_BEAT] = NULL, */

    [DEVDRV_MANAGER_CHAN_D2H_SYNC_MATRIX_READY] = devdrv_manager_d2h_sync_matrix_ready,
    [DEVDRV_MANAGER_CHAN_D2H_CHECK_PROCESS_SIGN] = devdrv_manager_check_process_sign,
    [DEVDRV_MANAGER_CHAN_D2H_DMS_EVENT_DISTRIBUTE] = dms_event_get_exception_from_device,
    [DEVDRV_MANAGER_CHAN_MAX_ID] = NULL,
};

STATIC int devdrv_chan_msg_dispatch(void *data, u32 *real_out_len)
{
    u32 msg_id = ((struct devdrv_manager_msg_head *)data)->msg_id;

    return devdrv_manager_chan_msg_processes[msg_id](data, real_out_len);
}

int devdrv_manager_rx_common_msg_process(u32 dev_id, void *data, u32 in_data_len, u32 out_data_len,
    u32 *real_out_len)
{
    struct devdrv_manager_msg_info *dev_manager_msg_info = NULL;
    u32 msg_id;

    if (dev_id >= DEVDRV_MAX_DAVINCI_NUM || data == NULL || real_out_len == NULL ||
        in_data_len < sizeof(struct devdrv_manager_msg_info)) {
        devdrv_drv_err("input id(%u)  %pK  out len %pK in len(%u)\n", dev_id, data, real_out_len, in_data_len);
        return -EINVAL;
    }
    msg_id = ((struct devdrv_manager_msg_head *)data)->msg_id;

    if (msg_id >= DEVDRV_MANAGER_CHAN_MAX_ID) {
        return -EINVAL;
    }
    if (devdrv_manager_chan_msg_processes[msg_id] == NULL) {
        return -EINVAL;
    }

    dev_manager_msg_info = (struct devdrv_manager_msg_info *)data;
    dev_manager_msg_info->header.dev_id = dev_id;
    return devdrv_chan_msg_dispatch(data, real_out_len);
}
EXPORT_SYMBOL_UNRELEASE(devdrv_manager_rx_common_msg_process);

int devdrv_manager_rx_msg_process(void *msg_chan, void *data, u32 in_data_len, u32 out_data_len,
    u32 *real_out_len)
{
    struct devdrv_manager_msg_info *dev_manager_msg_info = NULL;
    u32 msg_id;
    struct devdrv_info *dev_info = NULL;
    int dev_id;

    if (msg_chan == NULL || data == NULL || real_out_len == NULL ||
        in_data_len < sizeof(struct devdrv_manager_msg_info)) {
        devdrv_drv_err("input vaild msg_chan %pK date  %pK  out len %pK in len(%u)\n", msg_chan, data, real_out_len,
            in_data_len);
        return -EINVAL;
    }
    msg_id = ((struct devdrv_manager_msg_head *)data)->msg_id;

    if (msg_id >= DEVDRV_MANAGER_CHAN_MAX_ID || devdrv_manager_chan_msg_processes[msg_id] == NULL) {
        return -EINVAL;
    }

    dev_info = (struct devdrv_info *)devdrv_get_msg_chan_priv(msg_chan);
    dev_manager_msg_info = (struct devdrv_manager_msg_info *)data;

    /* get dev_id by msg_chan */
    if ((dev_id = devdrv_get_msg_chan_devid(msg_chan)) < 0) {
        devdrv_drv_err("msg_chan to devid failed\n");
        return -EINVAL;
    }

    if (dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("devid invalid(%d).\n", dev_id);
        return -EINVAL;
    }
    dev_manager_msg_info->header.dev_id = dev_id;

    return devdrv_chan_msg_dispatch(data, real_out_len);
}
EXPORT_SYMBOL_UNRELEASE(devdrv_manager_rx_msg_process);

STATIC void devdrv_manager_msg_chan_notify(u32 dev_id, int status)
{
}

STATIC void devdrv_manager_msg_chan_init_notify(u32 dev_id)
{
    devdrv_manager_msg_chan_notify(dev_id, 0);
}

#define DEV_MNG_NON_TRANS_MSG_DESC_SIZE 1024
struct devdrv_non_trans_msg_chan_info dev_manager_msg_chan_info = {
    .msg_type = devdrv_msg_client_devmanager,
    .flag = 0,
    .s_desc_size = DEV_MNG_NON_TRANS_MSG_DESC_SIZE,
    .c_desc_size = DEV_MNG_NON_TRANS_MSG_DESC_SIZE,
    .rx_msg_process = devdrv_manager_rx_msg_process,
};

STATIC void devdrv_manager_dev_ready_work(struct work_struct *work)
{
    struct devdrv_info *dev_info = NULL;
    void *no_trans_chan = NULL;
    u32 dev_id;

    dev_info = container_of(work, struct devdrv_info, work);
    dev_id = dev_info->pci_dev_id;
    if (dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("invalid dev_id, dev_id = %d\n", dev_id);
        return;
    }

    no_trans_chan = devdrv_pcimsg_alloc_non_trans_queue(dev_id, &dev_manager_msg_chan_info);
    if (no_trans_chan == NULL) {
        devdrv_drv_err("non trans msg chan alloc failed.dev_id %u\n", dev_id);
        return;
    }

    devdrv_manager_set_no_trans_chan(dev_id, no_trans_chan);
    devdrv_set_msg_chan_priv(no_trans_chan, (void *)dev_info);

    if (devdrv_manager_register(dev_info)) {
        devdrv_drv_err("devdrv_manager_register error, dev_id %u\n", dev_id);
        return;
    }

#ifdef CFG_FEATURE_TIMESYNC
    dms_time_sync_init(dev_id);
    devdrv_refresh_aicore_info_init(dev_id);
#endif

    up(&dev_info->no_trans_chan_wait_sema);
    dev_info->status = DEVINFO_STATUS_WORKING;
    if (dms_hotreset_task_init(dev_id) != 0) {
        devdrv_drv_err("Dms hotreset task init fail. (dev_id=%u)\n", dev_id);
    }
}

STATIC void devdrv_manager_common_chan_init(void)
{
    /* this function will be called at module_init, doesn't need lock */
    devdrv_manager_common_chan.type = DEVDRV_COMMON_MSG_DEVDRV_MANAGER;
    devdrv_manager_common_chan.common_msg_recv = devdrv_manager_rx_common_msg_process;
    devdrv_manager_common_chan.init_notify = devdrv_manager_msg_chan_init_notify;
}

STATIC void devdrv_manager_common_chan_uninit(void)
{
    /* this function will be called at module_init, doesn't need lock */
    devdrv_manager_common_chan.type = DEVDRV_COMMON_MSG_TYPE_MAX;
    devdrv_manager_common_chan.common_msg_recv = NULL;
    devdrv_manager_common_chan.init_notify = NULL;
}

STATIC struct devdrv_common_msg_client *devdrv_manager_get_common_chan(u32 dev_id)
{
    return &devdrv_manager_common_chan;
}

void *devdrv_manager_get_no_trans_chan(u32 dev_id)
{
    void *no_trans_chan = NULL;

    /* dev_manager_no_trasn_chan doesn't need lock */
    if (dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("get no trans chan error, dev_id = %d\n", dev_id);
        return NULL;
    }
    no_trans_chan = dev_manager_no_trasn_chan[dev_id];
    return no_trans_chan;
}

STATIC void devdrv_manager_set_no_trans_chan(u32 dev_id, void *no_trans_chan)
{
    if (dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("set no trans chan error, dev_id %u\n", dev_id);
        return;
    }
    dev_manager_no_trasn_chan[dev_id] = no_trans_chan;
}


#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 15, 0)
struct devdrv_check_start_s {
    u32 dev_id;
    struct timer_list check_timer;
};

STATIC struct devdrv_check_start_s devdrv_check_start[DEVDRV_MAX_DAVINCI_NUM];

STATIC void devdrv_check_start_event(struct timer_list *t)
{
    struct devdrv_info *dev_info;
    struct timespec stamp;
    struct devdrv_check_start_s *devdrv_start_check = from_timer(devdrv_start_check, t, check_timer);
    u32 dev_id;

    dev_id = devdrv_start_check->dev_id;
#else /* LINUX_VERSION_CODE >= KERNEL_VERSION(4,15,0) */

STATIC struct timer_list devdrv_check_start[DEVDRV_MAX_DAVINCI_NUM];

STATIC void devdrv_check_start_event(unsigned long data)
{
    struct devdrv_info *dev_info = NULL;
    struct timespec stamp;
    u32 dev_id = (u32)data;
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(4,15,0) */

    devdrv_drv_debug("*** time event for checking whether device is started or not ***\n");

    stamp = current_kernel_time();

    dev_info = devdrv_get_devdrv_info_array(dev_id);
    ;
    if (dev_info == NULL) {
        devdrv_drv_err("device is not ready.\n");
        return;
    }

    if (dev_info->dev_ready < DEVDRV_DEV_READY_EXIST) {
        devdrv_drv_err("device is not ready, "
            "dev_ready = %d, dev_id = %u\n",
            dev_info->dev_ready, dev_info->dev_id);
        (void)devdrv_host_black_box_add_exception(dev_info->dev_id, DEVDRV_BB_DEVICE_LOAD_TIMEOUT, stamp, NULL);
        tsdrv_set_ts_status(dev_info->dev_id, 0, TS_DOWN);

        return;
    }

    devdrv_drv_debug("*** device is started and working ***\n");
}

STATIC int devdrv_manager_dev_state_notify(u32 probe_num, u32 devid, u32 state)
{
    struct devdrv_black_box_state_info *bbox_state_info = NULL;
    struct timespec tstemp = { 0 };
    struct devdrv_info *dev_info = NULL;
    unsigned long flags;
    int ret;

    if (dev_manager_info == NULL || (enum devdrv_device_state)state >= STATE_TO_MAX ||
        devid >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("state notify para is invalid,"
            "with dev_manager_info(%pK),"
            "state(%d), devid(%d).\n",
            dev_manager_info, (u32)state, devid);
        return -ENODEV;
    }

    spin_lock_irqsave(&dev_manager_info->spinlock, flags);
    dev_manager_info->prob_num = probe_num;
    spin_unlock_irqrestore(&dev_manager_info->spinlock, flags);

    dev_info = devdrv_get_devdrv_info_array(devid);
    if (dev_info == NULL) {
        devdrv_drv_err("device is not ready.\n");
        return -ENODEV;
    }
    ret = down_timeout(&dev_info->no_trans_chan_wait_sema, DEVDRV_INIT_RESOURCE_TIMEOUT);
    if (ret) {
        devdrv_drv_err("get device info resource timeout, ret: %d, devid: %u!\n", ret, devid);
        return -ENODEV;
    }

    CLR_BIT_64(dev_manager_info->prob_device_bitmap[devid / BITS_PER_LONG_LONG], devid % BITS_PER_LONG_LONG);
    up(&dev_info->no_trans_chan_wait_sema);

    bbox_state_info = kzalloc(sizeof(struct devdrv_black_box_state_info), GFP_ATOMIC | __GFP_ACCOUNT);
    if (bbox_state_info == NULL) {
        devdrv_drv_err("vmalloc state_info failed.\n");
        return -ENOMEM;
    }

    bbox_state_info->devId = devid;
    bbox_state_info->state = state;

    devdrv_drv_info("dev state notified with devid(%d), state(%d)", devid, (u32)state);

    ret = devdrv_host_black_box_add_exception(0, DEVDRV_BB_DEVICE_STATE_INFORM, tstemp, (void *)bbox_state_info);
    if (ret) {
        kfree(bbox_state_info);
        bbox_state_info = NULL;
        devdrv_drv_err("devdrv_host_black_box_add_exception failed.\n");
        ssleep(1);  // add 1s for bbox to dump when unbind
        return -ENODEV;
    }
    kfree(bbox_state_info);
    bbox_state_info = NULL;
    ssleep(1);  // add 1s for bbox to dump when unbind

    return 0;
}

STATIC int devdrv_manager_dev_state_notify_compat(u32 devid, u32 state)
{
    return devdrv_manager_dev_state_notify(devdrv_manager_get_probe_num_kernel(), devid, state);
}
STATIC int devdrv_manager_dev_startup_notify(u32 prob_num, const u32 devids[], u32 array_len, u32 devnum)
{
    struct devdrv_black_box_devids *bbox_devids = NULL;
    struct timespec tstemp = { 0 };
    unsigned long flags;
    int ret;
    u32 i;

    (void)array_len;
    if (dev_manager_info == NULL || devnum > DEVDRV_MAX_DAVINCI_NUM || devids == NULL) {
        devdrv_drv_err("dev manager info is not initialized\n");
        return -ENODEV;
    }
    spin_lock_irqsave(&dev_manager_info->spinlock, flags);
    dev_manager_info->prob_num = prob_num;
    for (i = 0; i < devnum; i++) {
        SET_BIT_64(dev_manager_info->prob_device_bitmap[i / BITS_PER_LONG_LONG], devids[i] % BITS_PER_LONG_LONG);
    }
    spin_unlock_irqrestore(&dev_manager_info->spinlock, flags);

    bbox_devids = kzalloc(sizeof(struct devdrv_black_box_devids), GFP_ATOMIC | __GFP_ACCOUNT);
    if (bbox_devids == NULL) {
        devdrv_drv_err("vmalloc devids failed\n");
        return -ENOMEM;
    }

    bbox_devids->dev_num = devnum;
    for (i = 0; i < devnum; i++) {
        bbox_devids->devids[i] = devids[i];
    }

    ret = devdrv_host_black_box_add_exception(0, DEVDRV_BB_DEVICE_ID_INFORM, tstemp, (void *)bbox_devids);
    if (ret) {
        kfree(bbox_devids);
        bbox_devids = NULL;
        devdrv_drv_err("devdrv_host_black_box_add_exception failed.\n");
        return -ENODEV;
    }
    kfree(bbox_devids);
    bbox_devids = NULL;

    return 0;
}

STATIC int devmng_shm_init(struct devdrv_info *dev_info)
{
    size_t shm_size;
    u64 shm_addr;
    int ret;

    ret = devdrv_get_addr_info(dev_info->dev_id, DEVDRV_ADDR_DEVMNG_RESV_BASE,
                               0, &shm_addr, &shm_size);
    if (ret) {
        devdrv_drv_err("[devid=%u] get shm addr fail, ret=%d.\n", dev_info->dev_id, ret);
        return ret;
    }

    dev_info->shm_vaddr = ioremap(shm_addr, shm_size);
    if (dev_info->shm_vaddr == NULL) {
        devdrv_drv_err("[devid=%u] ioremap shm_vaddr fail.\n", dev_info->dev_id);
        return -ENOMEM;
    }

    dev_info->shm_head = (U_SHM_INFO_HEAD __iomem *)((uintptr_t)dev_info->shm_vaddr);
    dev_info->shm_head->head_info.offset_soc    = sizeof(U_SHM_INFO_HEAD);
    dev_info->shm_head->head_info.offset_board  = dev_info->shm_head->head_info.offset_soc +
                                                  sizeof(U_SHM_INFO_SOC);
    dev_info->shm_head->head_info.offset_status = dev_info->shm_head->head_info.offset_board +
                                                  sizeof(U_SHM_INFO_BOARD);

    dev_info->shm_soc    = (U_SHM_INFO_SOC __iomem *)((uintptr_t)((uintptr_t)dev_info->shm_vaddr +
                           dev_info->shm_head->head_info.offset_soc));
    dev_info->shm_board  = (U_SHM_INFO_BOARD __iomem *)((uintptr_t)((uintptr_t)dev_info->shm_vaddr +
                           dev_info->shm_head->head_info.offset_board));
    dev_info->shm_status = (U_SHM_INFO_STATUS __iomem *)((uintptr_t)((uintptr_t)dev_info->shm_vaddr +
                           dev_info->shm_head->head_info.offset_status));
    return 0;
}

STATIC void devmng_shm_uninit(struct devdrv_info *dev_info)
{
    if (dev_info->shm_vaddr != NULL) {
        iounmap(dev_info->shm_vaddr);
    }

    dev_info->shm_vaddr = NULL;
    dev_info->shm_head = NULL;
    dev_info->shm_soc = NULL;
    dev_info->shm_board = NULL;
    dev_info->shm_status = NULL;
    return;
}

STATIC int devmng_call_dev_boot_init(u32 phy_id)
{
    char *argv[DEVMNG_DEV_BOOT_ARG_NUM] = {NULL};
    char *envp[] = {
        "HOME=/",
        "PATH=/sbin:/bin:/usr/sbin:/usr/bin",
        NULL
    };
    int ret;

    argv[0] = (char *)kzalloc((strlen(DEVMNG_DEV_BOOT_INIT_SH) + 1), GFP_KERNEL | __GFP_ACCOUNT);
    if (argv[0] == NULL) {
        devdrv_drv_err("kzalloc argv[0] fail !\n");
        ret = -ENOMEM;
        goto kzalloc_argv0_fail;
    }
    argv[1] = (char *)kzalloc(DEVMNG_PHY_ID_LEN, GFP_KERNEL | __GFP_ACCOUNT);
    if (argv[1] == NULL) {
        devdrv_drv_err("kzalloc argv[1] fail !\n");
        ret = -ENOMEM;
        goto kzalloc_argv1_fail;
    }

    /* argument 0, for script path */
    ret = snprintf_s(argv[0], (strlen(DEVMNG_DEV_BOOT_INIT_SH) + 1),
        strlen(DEVMNG_DEV_BOOT_INIT_SH), "%s", DEVMNG_DEV_BOOT_INIT_SH);
    if (ret < 0) {
        devdrv_drv_err("sscanf_s argv[0] fail, ret = %d.\n", ret);
        goto snprintf_s_fail;
    }

    /* argument 1, for device id */
    ret = snprintf_s(argv[1], DEVMNG_PHY_ID_LEN, DEVMNG_PHY_ID_LEN - 1, "%u", phy_id);
    if (ret < 0) {
        devdrv_drv_err("sscanf_s argv[1] fail, ret = %d.\n", ret);
        goto snprintf_s_fail;
    }

    ret = call_usermodehelper(DEVMNG_DEV_BOOT_INIT_SH, argv, envp, UMH_WAIT_EXEC);
    if (ret) {
        devdrv_drv_err("call_usermodehelper fail, ret=%d\n", ret);
        goto call_usermodehelper_fail;
    }

    kfree(argv[1]);
    kfree(argv[0]);
    return 0;

call_usermodehelper_fail:
snprintf_s_fail:
    kfree(argv[1]);
kzalloc_argv1_fail:
    kfree(argv[0]);
kzalloc_argv0_fail:
    return ret;
}

STATIC struct devdrv_info *devdrv_manager_dev_info_alloc(u32 dev_id)
{
    struct devdrv_info *dev_info = NULL;
    int ret;

    dev_info = kzalloc(sizeof(*dev_info), GFP_KERNEL | __GFP_ACCOUNT);
    if (dev_info == NULL) {
        devdrv_drv_err("kzalloc dev_info failed. dev_id(%u)\n", dev_id);
        return NULL;
    }
    mutex_init(&dev_info->lock);
    mutex_init(&dev_info->container.lock);

    dev_info->dev_id = dev_id;

    dev_info->pci_dev_id = dev_id;
    dev_info->cce_ops.cce_dev = NULL;

    ret = devmng_shm_init(dev_info);
    if (ret) {
        kfree(dev_info);
        dev_info = NULL;
        devdrv_drv_err("dev_id[%u] devmng_shm_init fail, ret[%d]\n", dev_id, ret);
        return NULL;
    }

    return dev_info;
}

STATIC int devdrv_manager_init_common_chan(u32 dev_id)
{
    int ret;
    struct devdrv_common_msg_client *devdrv_commn_chan = NULL;

    devdrv_commn_chan = devdrv_manager_get_common_chan(dev_id);
    if (devdrv_commn_chan->init_notify == NULL) {
        devdrv_drv_err("common chan get failed. (dev_id=%u)\n", dev_id);
        return -ENODEV;
    }

    ret = devdrv_register_common_msg_client(devdrv_commn_chan);
    if (ret) {
        devdrv_drv_err("devdrv register common msg channel failed. (ret=%d, dev_id=%u)\n", ret, dev_id);
        return ret;
    }

    return 0;
}

STATIC int devdrv_manager_init_instance(u32 dev_id, struct device *dev)
{
    struct devdrv_platform_data *pdata = NULL;
    struct devdrv_info *dev_info = NULL;
    u32 init_flag = 1;
    int ret;

    dev_info = devdrv_get_devdrv_info_array(dev_id);
    if (dev_info != NULL) {
        init_flag = 0;
        devmng_shm_init(dev_info);
        devdrv_drv_info("dev_id(%u) repeat init instance.\n", dev_id);
    } else {
        dev_info = devdrv_manager_dev_info_alloc(dev_id);
        if (dev_info == NULL) {
            devdrv_drv_err("dev_id(%u) alloc info mem fail.\n", dev_id);
            return -ENOMEM;
        }
    }

    dev_info->dev_ready = 0;
    dev_info->driver_flag = 0;
    dev_info->dev = dev;
    dev_info->dmp_started = false;
    set_device_share_flag(dev_id, DEVICE_UNSHARE);

    tsdrv_set_ts_status(dev_info->dev_id, 0, TS_INITING);
    devdrv_drv_debug("*** set status initing, device id : %u***\n", dev_id);

    if (init_flag == 1) {
        pdata = kzalloc(sizeof(struct devdrv_platform_data), GFP_KERNEL | __GFP_ACCOUNT);
        if (pdata == NULL) {
            ret = -ENOMEM;
            goto kfree_info;
        }

        ret = devdrv_manager_create_one_device(dev_info);
        if (ret) {
            devdrv_drv_err("devdrv_manager_create_one_device return error.\n");
            goto kfree_info_pdata;
        }
    } else {
        pdata = dev_info->pdata;
    }

    if (init_flag == 1) {
        dev_info->run_mode = DEVDRV_NORMAL_MODE;

        devdrv_set_devdrv_info_array(dev_id, dev_info);
        dev_info->plat_type = (u8)DEVDRV_MANAGER_HOST_ENV;
        dev_info->pdata = (void *)pdata;

        spin_lock_init(&dev_info->spinlock);
        sema_init(&dev_info->no_trans_chan_wait_sema, 0);
    }

    INIT_WORK(&dev_info->work, devdrv_manager_dev_ready_work);
    ret = devdrv_manager_init_common_chan(dev_id);
    if (ret != 0) {
        devdrv_drv_err("common chan init failed. (dev_id=%u)\n", dev_id);
        goto release_one_device;
    }

    /* init a timer for check whether device manager is ready */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 15, 0)
    devdrv_check_start[dev_id].dev_id = dev_id;
    timer_setup(&devdrv_check_start[dev_id].check_timer, devdrv_check_start_event, 0);
    devdrv_check_start[dev_id].check_timer.expires = jiffies + LOAD_DEVICE_TIME * HZ;
    add_timer(&devdrv_check_start[dev_id].check_timer);
#else
    setup_timer(&devdrv_check_start[dev_id], devdrv_check_start_event, (unsigned long)dev_id);
    devdrv_check_start[dev_id].expires = jiffies + LOAD_DEVICE_TIME * HZ;
    add_timer(&devdrv_check_start[dev_id]);
#endif
    /* device online inform user */
    devdrv_manager_online_devid_update(dev_id);

    /* not need to return error when call device_boot_init.sh fail */
    ret = devmng_call_dev_boot_init(dev_id);
    if (ret) {
        devdrv_drv_err("dev_id[%u] devmng_call_dev_boot_init fail, ret[%d]\n", dev_id, ret);
    }

    devdrv_drv_info("devdrv_manager_init_instance dev_id :%u OUT !\n", dev_id);
    return 0;

release_one_device:
    devdrv_manager_release_one_device(dev_info);
    devdrv_set_devdrv_info_array(dev_id, NULL);
    dev_info->pdata = NULL;
kfree_info_pdata:
    kfree(pdata);
    pdata = NULL;
kfree_info:
    devmng_shm_uninit(dev_info);
    mutex_destroy(&dev_info->lock);
    mutex_destroy(&dev_info->container.lock);
    kfree(dev_info);
    dev_info = NULL;
return ret;
}

STATIC int devdrv_manager_uninit_instance(u32 dev_id)
{
    struct devdrv_common_msg_client *devdrv_commn_chan = NULL;
    struct devdrv_info *dev_info = NULL;
    void *no_trans_chan = NULL;
    int ret;

    dev_info = devdrv_get_devdrv_info_array(dev_id);
    if (dev_info == NULL) {
        devdrv_drv_err("get sema timeout,the ready of device is not ok, dev_id:%u.\n", dev_id);
        return -EINVAL;
    }

    devmng_shm_uninit(dev_info);
    dev_info->status = DEVINFO_STATUS_REMOVED;
    dev_info->dmp_started = false;

    ret = down_timeout(&dev_info->no_trans_chan_wait_sema, DEVDRV_INIT_INSTANCE_TIMEOUT);
    if (ret) {
        devdrv_drv_err("get sema from init instance timeout, ret:%d, dev%u\n", ret, dev_id);
    }
    cancel_work_sync(&dev_info->work);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 15, 0)
    del_timer_sync(&devdrv_check_start[dev_id].check_timer);
#else
    del_timer_sync(&devdrv_check_start[dev_id]);
#endif

#ifdef CFG_FEATURE_TIMESYNC
    dms_time_sync_exit(dev_id);
    devdrv_refresh_aicore_info_exit(dev_id);
#endif

    /* uninit common channel */
    devdrv_commn_chan = devdrv_manager_get_common_chan(dev_id);
    devdrv_unregister_common_msg_client(dev_id, devdrv_commn_chan);

    /* unnit non transparent channel */
    no_trans_chan = devdrv_manager_get_no_trans_chan(dev_id);
    if (no_trans_chan != NULL) {
        devdrv_pcimsg_free_non_trans_queue(no_trans_chan);
        no_trans_chan = NULL;
        devdrv_manager_set_no_trans_chan(dev_id, NULL);
    }

    devdrv_manager_unregister(dev_info);
    devdrv_manager_online_del_devids(dev_id);

    return 0;
}

STATIC void devdrv_manager_uninit_devinfo(void)
{
    struct devdrv_info *dev_info = NULL;
    u32 i;

    for (i = 0; i < DEVDRV_MAX_DAVINCI_NUM; i++) {
        dev_info = devdrv_get_devdrv_info_array(i);
        if (dev_info == NULL) {
            continue;
        }

        devdrv_manager_release_one_device(dev_info);
        devmng_shm_uninit(dev_info);
        devdrv_set_devdrv_info_array(i, NULL);

        kfree(dev_info->pdata);
        dev_info->pdata = NULL;
        mutex_destroy(&dev_info->lock);
        mutex_destroy(&dev_info->container.lock);
        kfree(dev_info);
        dev_info = NULL;
    }
}

#define DEVMNG_HOST_NOTIFIER "mng_host"
static int devdrv_manager_host_notifier_func(u32 udevid, enum uda_notified_action action)
{
    int ret = 0;

    if (udevid >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("Invalid para. (udevid=%u)\n", udevid);
        return -EINVAL;
    }

    if (action == UDA_INIT) {
        ret = devdrv_manager_init_instance(udevid, uda_get_device(udevid));
    } else if (action == UDA_UNINIT) {
        ret = devdrv_manager_uninit_instance(udevid);
    }

    devdrv_drv_info("notifier action. (udevid=%u; action=%d; ret=%d)\n", udevid, action, ret);

    return ret;
}

/*lint -e508 */
static const struct pci_device_id devdrv_driver_tbl[] = {
    {PCI_VDEVICE(HUAWEI, 0xd100), 0},
    {PCI_VDEVICE(HUAWEI, PCI_DEVICE_CLOUD), 0},
    {}
};
MODULE_DEVICE_TABLE(pci, devdrv_driver_tbl);
/*lint +e508 */

void devdrv_manager_ops_sem_down_write(void)
{
    down_write(&devdrv_ops_sem);
}
EXPORT_SYMBOL(devdrv_manager_ops_sem_down_write);

void devdrv_manager_ops_sem_up_write(void)
{
    up_write(&devdrv_ops_sem);
}
EXPORT_SYMBOL(devdrv_manager_ops_sem_up_write);

void devdrv_manager_ops_sem_down_read(void)
{
    down_read(&devdrv_ops_sem);
}
EXPORT_SYMBOL(devdrv_manager_ops_sem_down_read);

void devdrv_manager_ops_sem_up_read(void)
{
    up_read(&devdrv_ops_sem);
}
EXPORT_SYMBOL(devdrv_manager_ops_sem_up_read);

static int devdrv_davinci_open(struct inode *inode, struct file *filep)
{
#ifndef AOS_LLVM_BUILD
    u32 devid = iminor(inode);
    int err;

    if (devid >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("invalid devid=%u\n", devid);
        return -EFAULT;
    }

    err = devdrv_manager_container_table_devlist_add_ns(&devid, 1, current->nsproxy->mnt_ns);
    if (err != 0) {
        devdrv_drv_err("add to list failed. dev_id(%u)\n", devid);
        return -ENODEV;
    }
    return 0;
#else
    return -EINVAL;
#endif
}

static int devdrv_davinci_release(struct inode *inode, struct file *filep)
{
    return -EINVAL;
}

static long devdrv_davinci_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
    return -EINVAL;
}

static int devdrv_davinci_mmap(struct file *filep, struct vm_area_struct *vma)
{
    return -EINVAL;
}

unsigned int devdrv_major;
static struct class *devdrv_class;
STATIC const struct file_operations devdrv_davinci_fops = {
    .owner = THIS_MODULE,
    .open = devdrv_davinci_open,
    .release = devdrv_davinci_release,
    .unlocked_ioctl = devdrv_davinci_ioctl,
    .mmap = devdrv_davinci_mmap,
};

STATIC int devdrv_manager_create_driver_device(void)
{
    dev_t devdrv_dev;
    int ret;

    ret = alloc_chrdev_region(&devdrv_dev, 0, DEVDRV_MAX_DAVINCI_NUM, "devdrv-cdev");
    if (ret) {
        devdrv_drv_err("stream alloc_chrdev_region error.\n");
        return ret;
    }

    devdrv_major = MAJOR(devdrv_dev);
    devdrv_class = class_create(THIS_MODULE, "devdrv-class");
    if (IS_ERR(devdrv_class)) {
        unregister_chrdev_region(devdrv_dev, DEVDRV_MAX_DAVINCI_NUM);
        devdrv_drv_err("class_create error.\n");
        return -EINVAL;
    }
    return 0;
}

STATIC void devdrv_manager_release_driver_device(void)
{
    class_destroy(devdrv_class);
    unregister_chrdev_region(MKDEV(devdrv_major, 0), DEVDRV_MAX_DAVINCI_NUM);
}

STATIC int devdrv_manager_create_one_device(struct devdrv_info *dev_info)
{
    struct device *i_device = NULL;
    dev_t devno;
    int ret;

    devno = MKDEV(devdrv_major, dev_info->dev_id);

    dev_info->cce_ops.devdrv_cdev.owner = THIS_MODULE;
    cdev_init(&dev_info->cce_ops.devdrv_cdev, &devdrv_davinci_fops);
    ret = cdev_add(&dev_info->cce_ops.devdrv_cdev, devno, DEVDRV_MAX_DAVINCI_NUM);
    if (ret) {
        devdrv_drv_err("cdev_add error.\n");
        return ret;
    }

    i_device = device_create(devdrv_class, NULL, devno, NULL, "davinci%d", dev_info->dev_id);
    if (IS_ERR(i_device)) {
        cdev_del(&dev_info->cce_ops.devdrv_cdev);
        devdrv_drv_err("device_create error.\n");
        return -ENODEV;
    }
    ret = devdrv_manager_container_table_devlist_add_ns(&dev_info->dev_id, 1,
                                                        init_task.nsproxy->mnt_ns);
    if (ret) {
        devdrv_drv_err("add to list error.%d\n", dev_info->dev_id);
        return -ENODEV;
    }

    dev_info->cce_ops.cce_dev = i_device;
    return 0;
}

STATIC void devdrv_manager_release_one_device(struct devdrv_info *dev_info)
{
    dev_t devno;

    devno = MKDEV(devdrv_major, dev_info->dev_id);

    if (dev_info->cce_ops.cce_dev != NULL) {
        device_destroy(devdrv_class, devno);
        cdev_del(&dev_info->cce_ops.devdrv_cdev);
        dev_info->cce_ops.cce_dev = NULL;
    }
}

STATIC int devdrv_manager_reboot_handle(struct notifier_block *self, unsigned long event, void *data)
{
#ifdef CFG_FEATURE_TIMESYNC
    int count = 0;
#endif

    if (event != SYS_RESTART && event != SYS_HALT && event != SYS_POWER_OFF) {
        return NOTIFY_DONE;
    }
#ifdef CFG_FEATURE_TIMESYNC
    dms_time_sync_reboot_handle();
    mb();
    while (dms_is_sync_timezone()) {
        if (++count > DMS_TIMEZONE_MAX_COUNT) {
            devdrv_drv_err("wait localtime sync over 6 seconds.\n");
            return NOTIFY_BAD;
        }
        msleep(DMS_TIMEZONE_SLEEP_MS);
    }
#endif

    devdrv_drv_info("System reboot now.....\n");
    return NOTIFY_OK;
}

STATIC int devdrv_manage_release_prepare(struct file *file_op, unsigned long mode)
{
    struct devdrv_manager_context *dev_manager_context = NULL;

    if (mode != NOTIFY_MODE_RELEASE_PREPARE) {
        devdrv_drv_err("Invalid mode,(mode=%lu).\n", mode);
        return -EINVAL;
    }

    if ((file_op == NULL) || (file_op->private_data == NULL)) {
        devdrv_drv_err("filep is %s.\n", (file_op == NULL) ? "NULL" : "OK");
        return -EINVAL;
    }

    dev_manager_context = file_op->private_data;

    devdrv_host_release_notice_dev(dev_manager_context->tgid);
    devdrv_manager_process_sign_release(dev_manager_context->tgid);
    devdrv_drv_debug("Dmanage end release prepare.\n");
    return 0;
}

static struct notifier_block devdrv_manager_reboot_notifier = {
    .notifier_call = devdrv_manager_reboot_handle,
};

const struct notifier_operations notifier_ops = {
    .notifier_call =  devdrv_manage_release_prepare,
};

STATIC int devdrv_manager_register_notifier(void)
{
    int ret;

    ret = drv_ascend_register_notify(DAVINCI_INTF_MODULE_DEVMNG, &notifier_ops);
    if (ret != 0) {
        devdrv_drv_err("Register sub module fail,(ret=%d).\n", ret);
        return -ENODEV;
    }

    ret = register_reboot_notifier(&devdrv_manager_reboot_notifier);
    if (ret) {
        devdrv_drv_err("register_reboot_notifier failed.\n");
        (void)unregister_reboot_notifier(&devdrv_manager_reboot_notifier);
        return ret;
    }
    return 0;
}

STATIC void devdrv_manager_unregister_notifier(void)
{
    (void)unregister_reboot_notifier(&devdrv_manager_reboot_notifier);
}

STATIC int devdrv_manager_info_init(void)
{
    int i;

    dev_manager_info = kzalloc(sizeof(*dev_manager_info), GFP_KERNEL | __GFP_ACCOUNT);
    if (dev_manager_info == NULL) {
        devdrv_drv_err("kzalloc return NULL, failed to alloc mem for manager struct.\n");
        return -ENOMEM;
    }

    spin_lock_init(&dev_manager_info->pm_list_lock);
    INIT_LIST_HEAD(&dev_manager_info->pm_list_header);

    INIT_LIST_HEAD(&dev_manager_info->hostpid_list_header);
    mutex_init(&dev_manager_info->devdrv_sign_list_lock);
    (void)memset_s(dev_manager_info->devdrv_sign_count, MAX_DOCKER_NUM + 1, 0, MAX_DOCKER_NUM + 1);

    dev_manager_info->prob_num = 0;
    for (i = 0; i < DEVDRV_MAX_DAVINCI_NUM / BITS_PER_LONG_LONG + 1; i++) {
        dev_manager_info->prob_device_bitmap[i] = 0;
    }

    dev_manager_info->num_dev = 0;
    dev_manager_info->host_type = devdrv_get_host_type();
    dev_manager_info->drv_ops = &devdrv_host_drv_ops;

    spin_lock_init(&dev_manager_info->spinlock);

    for (i = 0; i < DEVDRV_MAX_DAVINCI_NUM; i++) {
        devdrv_manager_set_devdrv_info(i, NULL);
        devdrv_manager_set_no_trans_chan(i, NULL);
        devdrv_set_devdrv_info_array(i, NULL);
        dev_manager_info->device_status[i] = DRV_STATUS_INITING;
    }
    dev_manager_info->plat_info = DEVDRV_MANAGER_HOST_ENV;
    return 0;
}

STATIC void devdrv_manager_info_free(void)
{
    kfree(dev_manager_info);
    dev_manager_info = NULL;
}

int devdrv_manager_init(void)
{
    struct uda_dev_type type;
    struct device *i_device = NULL;
    int ret = 0;

    ret = drv_davinci_register_sub_module(DAVINCI_INTF_MODULE_DEVMNG, &devdrv_manager_file_operations);
    if (ret) {
        devdrv_drv_err("drv_davinci_register_sub_module failed! ret=%d\n", ret);
        goto register_sub_module_fail;
    }

    ret = devdrv_manager_create_driver_device();
    if (ret) {
        devdrv_drv_err("devdrv_manager_create_driver_device return error: %d, "
                       "unable to create davinci device class.\n", ret);
        goto create_driver_device_fail;
    }

    ret = devdrv_manager_info_init();
    if (ret) {
        devdrv_drv_err("Init dev_manager_info failed.\n");
        goto dev_manager_info_init_failed;
    }

    devdrv_host_black_box_init();

    ret = devdrv_manager_register_notifier();
    if (ret) {
        devdrv_drv_err("Failed to register dmanager notifier.\n");
        goto register_notifier_fail;
    }

    i_device = davinci_intf_get_owner_device();
    if (i_device == NULL) {
        devdrv_drv_err("failed to intf get owner device.\n");
        ret = -ENODEV;
        goto get_device_fail;
    }
    dev_manager_info->dev = i_device;

    tsdrv_status_init();

#ifdef CONFIG_SYSFS
    ret = sysfs_create_group(&i_device->kobj, &devdrv_manager_attr_group);
    if (ret) {
        devdrv_drv_err("sysfs create failed, ret = %d\n", ret);
        goto sysfs_create_group_failed;
    }
#endif /* CONFIG_SYSFS */

    dev_manager_info->dev_rdy_work = create_singlethread_workqueue("dev_manager_workqueue");
    if (dev_manager_info->dev_rdy_work == NULL) {
        devdrv_drv_err("create workqueue failed\n");
        ret = -ENODEV;
        goto workqueue_create_failed;
    }

    ret = devdrv_manager_container_table_init(dev_manager_info);
    if (ret) {
        devdrv_drv_err("container table init failed\n");
        ret = -ENODEV;
        goto container_table_init_failed;
    }
    ret = devdrv_manager_online_kfifo_alloc();
    if (ret) {
        devdrv_drv_err("kfifo_alloc failed, ret(%d)\n", ret);
        ret = -ENOMEM;
        goto online_kfifo_alloc_failed;
    }
    devdrv_manager_common_chan_init();
    uda_davinci_near_real_entity_type_pack(&type);
    ret = uda_notifier_register(DEVMNG_HOST_NOTIFIER, &type, UDA_PRI1, devdrv_manager_host_notifier_func);
    if (ret != 0) {
        devdrv_drv_err("uda_notifier_register failed\n");
        ret = -ENODEV;
        goto uda_notifier_register_failed;
    }

    drvdrv_dev_startup_register(devdrv_manager_dev_startup_notify);
    drvdrv_dev_state_notifier_register(devdrv_manager_dev_state_notify_compat);

    ret = log_level_file_init();
    if (ret != 0) {
        devdrv_drv_err("log_level_file_init failed!!! ret(%d), "
                       "default_log_level is ERROR.\n", ret);
    }
    init_rwsem(&devdrv_ops_sem);

    return 0;

uda_notifier_register_failed:
    devdrv_manager_online_kfifo_free();
online_kfifo_alloc_failed:
    devdrv_manager_container_table_exit(dev_manager_info);
container_table_init_failed:
    destroy_workqueue(dev_manager_info->dev_rdy_work);
workqueue_create_failed:
#ifdef CONFIG_SYSFS
    sysfs_remove_group(&i_device->kobj, &devdrv_manager_attr_group);
sysfs_create_group_failed:
#endif /* CONFIG_SYSFS */
get_device_fail:
    devdrv_manager_unregister_notifier();
register_notifier_fail:
    devdrv_manager_info_free();
dev_manager_info_init_failed:
    devdrv_manager_release_driver_device();
create_driver_device_fail:
    (void)drv_ascend_unregister_sub_module(DAVINCI_INTF_MODULE_DEVMNG);
register_sub_module_fail:
    return ret;
}

void devdrv_manager_exit(void)
{
    struct list_head *pos = NULL;
    struct list_head *n = NULL;
    struct devdrv_pm *pm = NULL;
    struct uda_dev_type type;

    log_level_file_remove();
    devdrv_dev_state_notifier_unregister();

#ifdef CONFIG_SYSFS
    sysfs_remove_group(&dev_manager_info->dev->kobj, &devdrv_manager_attr_group);
#endif /* CONFIG_SYSFS */

    devdrv_host_black_box_exit();

    if (!list_empty_careful(&dev_manager_info->pm_list_header)) {
        list_for_each_safe(pos, n, &dev_manager_info->pm_list_header)
        {
            pm = list_entry(pos, struct devdrv_pm, list);
            list_del(&pm->list);
            kfree(pm);
            pm = NULL;
        }
    }

    uda_davinci_near_real_entity_type_pack(&type);
    (void)uda_notifier_unregister(DEVMNG_HOST_NOTIFIER, &type);
    devdrv_manager_common_chan_uninit();

    devdrv_manager_uninit_devinfo();
    destroy_workqueue(dev_manager_info->dev_rdy_work);
    devdrv_manager_container_table_exit(dev_manager_info);
    kfree(dev_manager_info);
    dev_manager_info = NULL;
    (void)unregister_reboot_notifier(&devdrv_manager_reboot_notifier);
    devdrv_manager_online_kfifo_free();
    dms_hotreset_task_exit();
    devdrv_manager_release_driver_device();
    if (drv_ascend_unregister_sub_module(DAVINCI_INTF_MODULE_DEVMNG)) {
        devdrv_drv_err("drv_ascend_unregister_sub_module failed!\n");
    }
}

#else
int devdrv_manager_host_init(void)
{
    return 0;
}
#endif
