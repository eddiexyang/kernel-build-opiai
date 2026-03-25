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

#ifdef CONFIG_GENERIC_BUG
#undef CONFIG_GENERIC_BUG
#endif
#ifdef CONFIG_BUG
#undef CONFIG_BUG
#endif
#ifdef CONFIG_DEBUG_BUGVERBOSE
#undef CONFIG_DEBUG_BUGVERBOSE
#endif

#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/idr.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/stat.h>
#include <linux/io.h>
#include <linux/pci.h>
#include <linux/vmalloc.h>
#include <linux/kmod.h>
#include <linux/reboot.h>
#include <linux/version.h>
#include <linux/kallsyms.h>
#include <linux/kthread.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
#include <linux/sched/signal.h>
#include <linux/sched/task.h>
#endif

#include "dbl/uda.h"
#include "soc_res.h"

#include "devdrv_manager_common.h"
#include "devdrv_pm.h"
#include "devdrv_driver_pm.h"
#include "devmng_dms_adapt.h"
#include "devdrv_manager.h"
#include "devdrv_manager_msg.h"
#include "devdrv_platform_resource.h"
#include "devdrv_black_box.h"
#include "devdrv_pcie.h"
#include "devdrv_device_online.h"
#include "devdrv_common.h"
#include "devdrv_interface.h"
#include "dms_event_distribute.h"
#include "hvdevmng_init.h"
#include "tsdrv_status.h"
#include "dev_mnt_vdevice.h"
#include "virtmng_interface.h"
#include "devdrv_manager_pid_map.h"
#include "davinci_interface.h"
#include "devdrv_user_common.h"
#include "davinci_api.h"
#include "devdrv_manager_comm.h"
#include "svm_interface.h"
#include "dms_hotreset.h"

#ifdef CFG_FEATURE_DEVICE_SHARE
#include "devdrv_manager_dev_share.h"
#endif

#ifdef CFG_FEATURE_CHIP_DIE
#include "devdrv_chip_dev_map.h"
#endif

#ifdef CFG_FEATURE_TIMESYNC
#include "dms_time.h"
#endif

#define DEVMNG_DEV_BOOT_ARG_NUM 2
#define DEVMNG_PHY_ID_LEN 16
#define DEVMNG_DEV_BOOT_INIT_SH "/usr/bin/device_boot_init.sh"
#define DEVMNG_SET_ALL_DEV 0xFFFFFFFFU
#define DEVMNG_TSLOG_MAX_SIZE (1024 * 1024) /* 1M bytes */

void *dev_manager_no_trasn_chan[DEVDRV_MAX_DAVINCI_NUM];
struct devdrv_manager_info *dev_manager_info;
STATIC struct devdrv_common_msg_client devdrv_manager_common_chan;
STATIC struct tsdrv_drv_ops devdrv_host_drv_ops;
STATIC struct rw_semaphore devdrv_ops_sem;
STATIC struct devdrv_info *devdrv_info_array[DEVDRV_MAX_DAVINCI_NUM];
STATIC struct devdrv_ts_log *g_devdrv_ts_log_array[DEVDRV_MAX_DAVINCI_NUM] = {NULL};
static int module_init_finish = 0;
static u8 device_hotreset_flag[DEVDRV_MAX_DAVINCI_NUM] = {0, };

STATIC void devdrv_set_devdrv_info_array(u32 dev_id, struct devdrv_info *dev_info);
STATIC void devdrv_manager_set_no_trans_chan(u32 dev_id, void *no_trans_chan);
STATIC int devdrv_manager_ipc_notify_ioctl(struct file *filep, unsigned int cmd, unsigned long arg);
STATIC void devdrv_manager_uninit_one_device_info(unsigned int dev_id);

static DEFINE_SPINLOCK(devdrv_spinlock);
static DEFINE_MUTEX(devdrv_get_devnum_mutex);
static DEFINE_MUTEX(devdrv_mutex);

#ifdef CFG_SOC_PLATFORM_CLOUD_V2
#define MAX_BIT_NUM 48
#else
#define MAX_BIT_NUM 32
#endif
#define SET_BIT_64(x, y) ((x) |= ((u64)0x01 << (y)))
#define CLR_BIT_64(x, y) ((x) &= (~((u64)0x01 << (y))))
#define CHECK_BIT_64(x, y) ((x) & ((u64)0x01 << (y)))
#define DEVDRV_INVALID_PHY_ID 0xFFFFFFFF

#ifdef CFG_FEATURE_OLD_DEVID_TRANS
STATIC void devdrv_manager_release_one_device(struct devdrv_info *dev_info);
STATIC int devdrv_manager_create_one_device(struct devdrv_info *dev_info);
#endif
u32 devdrv_manager_get_ts_num(struct devdrv_info *dev_info);

#ifdef CONFIG_SYSFS

static u32 sysfs_devid = 0;

#define DEVDRV_ATTR_RO(_name) static struct kobj_attribute _name##_attr = __ATTR_RO(_name)

#define DEVDRV_ATTR(_name) static struct kobj_attribute _name##_attr = __ATTR(_name, 0600, _name##_show, _name##_store)

STATIC ssize_t devdrv_resources_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    u32 result;
    int ret;

    if (buf == NULL) {
        devdrv_drv_err("invalid args(buf is null).\n");
        return count;
    }

    ret = kstrtouint(buf, 10, &result); /* base is 10, buf is converted to decimal number */
    if (ret) {
        devdrv_drv_err("unable to transform input string into devid, input string: %s, ret(%d)", buf, ret);
        return count;
    }

    devdrv_drv_info("input devid is: %u.\n", result);

    if (result >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_warn("input devid(%u) is too big, reset into 0, input string: %s", result, buf);
        result = 0;
    }

    sysfs_devid = result;

    return count;
}

STATIC ssize_t devdrv_resources_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return 0;
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

void devdrv_set_device_hotreset_flag(u32 devid, u8 flag)
{
    if (devid < DEVDRV_MAX_DAVINCI_NUM) {
        device_hotreset_flag[devid] = flag;
    } else {
        u32 i;
        for (i = 0; i < DEVDRV_PF_DEV_MAX_NUM; i++) {
#ifndef DEVDRV_MANAGER_HOST_UT_TEST
            if (uda_is_udevid_exist(i)) {
                device_hotreset_flag[i] = flag;
            }
#endif
        }
    }
}

#ifndef DEVDRV_MANAGER_HOST_UT_TEST
#ifdef CFG_FEATURE_OLD_DEVID_TRANS
static u8 devdrv_get_device_hotreset_flag(u32 devid)
{
    return device_hotreset_flag[devid];
}
#endif
#endif

bool devdrv_check_device_hotreset_finish(u32 devid)
{
    if (devid < DEVDRV_MAX_DAVINCI_NUM) {
        return (device_hotreset_flag[devid] == 0);
    } else {
        u32 i;
        for (i = 0; i < DEVDRV_PF_DEV_MAX_NUM; i++) {
            if (device_hotreset_flag[i] != 0) {
                return false;
            }
        }
        return true;
    }
}

int copy_from_user_safe(void *to, const void __user *from, unsigned long n)
{
    if ((to == NULL) || (from == NULL) || (n == 0)) {
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
    if ((to == NULL) || (from == NULL) || (n == 0)) {
        devdrv_drv_err("user pointer is NULL.\n");
        return -EINVAL;
    }

    if (copy_to_user(to, (void *)from, n)) {
        return -ENODEV;
    }

    return 0;
}
EXPORT_SYMBOL(copy_to_user_safe);

STATIC int devdrv_manager_trans_and_check_id(u32 logical_dev_id, u32 *physical_dev_id, u32 *vfid)
{
    int ret;

    if (logical_dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("wrong device id, dev_id = %u\n", logical_dev_id);
        return -EINVAL;
    }

    ret = devdrv_manager_container_logical_id_to_physical_id(logical_dev_id, physical_dev_id, vfid);
    if (ret != 0) {
        devdrv_drv_err("can transfer dev id %u ret = %d.\n", logical_dev_id, ret);
        return ret;
    }
    if (*physical_dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("wrong device id, dev_id = %u\n", *physical_dev_id);
        return -EINVAL;
    }
    if ((dev_manager_info == NULL) || devdrv_info_array[*physical_dev_id] == NULL) {
        devdrv_drv_err("device manager is not initialized, dev_id= %u, device manager %pK .\n",
            *physical_dev_id, dev_manager_info);
        return -EINVAL;
    }
    if (devdrv_info_array[*physical_dev_id]->status == 1) {
        devdrv_drv_err("device status is 1. (dev_id=%u)\n", *physical_dev_id);
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

struct devdrv_info *devdrv_get_devdrv_info_array(u32 dev_id)
{
    struct devdrv_info *dev_info = NULL;

    if (dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("invalid dev_id(%u)\n", dev_id);
        return NULL;
    }

    spin_lock(&devdrv_spinlock);
    dev_info = devdrv_info_array[dev_id];
    spin_unlock(&devdrv_spinlock);

    return dev_info;
}

#define DEV_READY_WAIT_ONCE_TIME_MS 100
#define UNIT_100MS_PER_SECOND 10
int devdrv_wait_device_ready(u32 dev_id, u32 timeout_second)
{
    struct devdrv_info *dev_info = NULL;
    u32 cycle_time = timeout_second * UNIT_100MS_PER_SECOND;

    while (cycle_time > 0) {
        dev_info = devdrv_get_devdrv_info_array(dev_id);
        if (dev_info == NULL) {
            devdrv_drv_err("device info is null. (dev_id=%u)\n", dev_id);
            return -ENODEV;
        }

        if (dev_info->dev_ready == DEVDRV_DEV_READY_WORK) {
            break;
        }

        cycle_time--;
        msleep(DEV_READY_WAIT_ONCE_TIME_MS);
    }

    if (cycle_time == 0) {
        devdrv_drv_err("Wait device ready timeout. (dev_id=%u; timeout=%u)\n", dev_id, timeout_second);
        return -ENODEV;
    }

    return 0;
}

#ifdef CFG_FEATURE_OLD_DEVID_TRANS
STATIC void devdrv_manager_first_get_devnum(void)
{
    int total_timeout = DEVDRV_GET_DEVNUM_STARTUP_TIMEOUT;
    int timeout = 0;
    u32 num_dev = 0;
    int stage = 0;
    int pcie_num;

    pcie_num = devdrv_get_dev_num();
    if (pcie_num <= 0) {
        devdrv_drv_err("pcie report dev_num fail\n");
        return;
    }

    devdrv_drv_info("pcie report dev_num = %d\n", pcie_num);

    do {
        /* arm centos environment, host_mnt_ns is NULL */
        if (devdrv_manager_get_host_mnt_ns() == NULL) {
            num_dev = devdrv_manager_container_get_devnum_bare(DEVDRV_PF_TYPE);
        } else {
            num_dev = devdrv_manager_container_get_devnum(devdrv_manager_get_host_mnt_ns(), DEVDRV_PF_TYPE);
        }
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
            devdrv_drv_warn("first get dev_num timeout in stage %d\n", stage);
            return;
        }
    } while (1);

    return;
}
#endif

STATIC int devdrv_manager_fresh_amp_smp_mode(void)
{
    u32 num_dev, dev_id, phyId, masterId, vfid;
    int ret;

#ifdef CFG_FEATURE_OLD_DEVID_TRANS
    num_dev = devdrv_manager_container_get_devnum(current->nsproxy->mnt_ns, DEVDRV_PF_TYPE);
    if (num_dev > DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("get device number fail, dev_num=%u.\n", num_dev);
        return -EINVAL;
    }
#else
    num_dev = devdrv_manager_get_devnum();
#endif

    for (dev_id = 0; dev_id < num_dev; dev_id++) {
        ret = devdrv_manager_container_logical_id_to_physical_id(dev_id, &phyId, &vfid);
        if (ret) {
            devdrv_drv_err("logical_id to phy_id fail, dev_id=%u dev_num=%u.\n", dev_id, num_dev);
            return ret;
        }

        ret = devdrv_get_master_devid_in_the_same_os(phyId, &masterId);
        if (ret) {
            devdrv_drv_err("[phyId=%u] get masterId fail, ret=%d.\n", phyId, ret);
            return ret;
        }

        if (phyId != masterId) {
            dev_manager_info->amp_or_smp = DEVMNG_SMP_MODE;
            devdrv_drv_info("this machine is smp mode.\n");
            return 0;
        }
    }

    dev_manager_info->amp_or_smp = DEVMNG_AMP_MODE;
    devdrv_drv_info("this machine is amp mode.\n");
    return 0;
}

int devdrv_manager_get_amp_smp_mode(u32 *amp_or_smp)
{
    if ((dev_manager_info == NULL) || (amp_or_smp == NULL)) {
        return -EINVAL;
    }

#ifndef CFG_FEATURE_OLD_DEVID_TRANS
    (void)devdrv_manager_fresh_amp_smp_mode();
#endif
    *amp_or_smp = dev_manager_info->amp_or_smp;
    return 0;
}

#ifdef CFG_FEATURE_OLD_DEVID_TRANS
u32 devdrv_manager_get_devnum(void)
{
    STATIC u32 devdrv_first_get_devnum_en = 1;
    u32 num_dev;

    if (dev_manager_info == NULL) {
        return (DEVDRV_MAX_DAVINCI_NUM + 1);
    }

    mutex_lock(&devdrv_get_devnum_mutex);
    if (devdrv_first_get_devnum_en) {
        devdrv_manager_first_get_devnum();

        /* fresh amp or smp mode after getting first devnum */
        if (devdrv_manager_fresh_amp_smp_mode()) {
            devdrv_drv_err("fresh amp or smp mode fail.\n");
        }
        devdrv_first_get_devnum_en = 0;
    }

    num_dev = devdrv_manager_container_get_devnum(current->nsproxy->mnt_ns, DEVDRV_PF_TYPE);

    mutex_unlock(&devdrv_get_devnum_mutex);
    return num_dev;
}

STATIC u32 devdrv_manager_get_vdevnum(void)
{
    u32 num_vdev;

    if (dev_manager_info == NULL) {
        return (DEVDRV_MAX_DAVINCI_NUM + 1);
    }

    num_vdev = devdrv_manager_container_get_devnum(current->nsproxy->mnt_ns, DEVDRV_VF_TYPE);

    return num_vdev;
}
#endif

struct devdrv_info *devdrv_manager_get_devdrv_info(u32 dev_id)
{
    struct devdrv_info *dev_info = NULL;
    unsigned long flags;

    if ((dev_manager_info == NULL) || (dev_id >= DEVDRV_MAX_DAVINCI_NUM)) {
        return NULL;
    }

    spin_lock_irqsave(&dev_manager_info->spinlock, flags);
    dev_info = dev_manager_info->dev_info[dev_id];
    spin_unlock_irqrestore(&dev_manager_info->spinlock, flags);

    return dev_info;
}
EXPORT_SYMBOL(devdrv_manager_get_devdrv_info);

STATIC void devdrv_manager_dev_num_increase(unsigned int dev_id)
{
    dev_manager_info->num_dev++;

    if (devdrv_manager_is_pf_device(dev_id)) {
        dev_manager_info->pf_num++;
    } else {
        dev_manager_info->vf_num++;
    }
}

STATIC void devdrv_manager_dev_num_decrease(unsigned int dev_id)
{
    dev_manager_info->num_dev--;

    if (devdrv_manager_is_pf_device(dev_id)) {
        dev_manager_info->pf_num--;
    } else {
        dev_manager_info->vf_num--;
    }
}

STATIC void devdrv_manager_dev_num_reset(void)
{
    dev_manager_info->num_dev = 0;
    dev_manager_info->pf_num = 0;
    dev_manager_info->vf_num = 0;
}

STATIC int devdrv_manager_set_devinfo_inc_devnum(u32 dev_id, struct devdrv_info *dev_info)
{
    unsigned long flags;

    if ((dev_id >= DEVDRV_MAX_DAVINCI_NUM) || (dev_manager_info == NULL) || (dev_info == NULL)) {
        devdrv_drv_err("invalid dev_id(%u), or dev_manager_info(%pK) or dev_info(%pK) is NULL\n", dev_id,
                       dev_manager_info, dev_info);
        return -EINVAL;
    }

    spin_lock_irqsave(&dev_manager_info->spinlock, flags);
    if (dev_manager_info->dev_info[dev_id] != NULL) {
        spin_unlock_irqrestore(&dev_manager_info->spinlock, flags);
        devdrv_drv_err("dev_info is not NULL, dev_id(%u)\n", dev_id);
        return -ENODEV;
    }

    if (dev_manager_info->num_dev >= DEVDRV_MAX_DAVINCI_NUM) {
        spin_unlock_irqrestore(&dev_manager_info->spinlock, flags);
        devdrv_drv_err("wrong device num, num_dev(%u). dev_id(%u)\n", dev_manager_info->num_dev, dev_id);
        return -EFAULT;
    }

    devdrv_manager_dev_num_increase(dev_id);
    dev_manager_info->dev_info[dev_id] = dev_info;
    dev_manager_info->dev_id[dev_id] = dev_id;
    spin_unlock_irqrestore(&dev_manager_info->spinlock, flags);

    return 0;
}

STATIC int devdrv_manager_reset_devinfo_dec_devnum(u32 dev_id)
{
    unsigned long flags;

    if ((dev_id >= DEVDRV_MAX_DAVINCI_NUM) || (dev_manager_info == NULL)) {
        devdrv_drv_err("invalid dev_id(%u), or dev_manager_info is NULL\n", dev_id);
        return -EINVAL;
    }

    spin_lock_irqsave(&dev_manager_info->spinlock, flags);
    if (dev_manager_info->dev_info[dev_id] == NULL) {
        spin_unlock_irqrestore(&dev_manager_info->spinlock, flags);
        devdrv_drv_err("dev_info is not NULL, dev_id(%u)\n", dev_id);
        return -ENODEV;
    }

    if ((dev_manager_info->num_dev > DEVDRV_MAX_DAVINCI_NUM) || (dev_manager_info->num_dev == 0)) {
        spin_unlock_irqrestore(&dev_manager_info->spinlock, flags);
        devdrv_drv_err("wrong device num, num_dev(%u). dev_id(%u)\n", dev_manager_info->num_dev, dev_id);
        return -EFAULT;
    }

    dev_manager_info->dev_id[dev_id] = 0;
    dev_manager_info->dev_info[dev_id] = NULL;
    devdrv_manager_dev_num_decrease(dev_id);
    dev_manager_info->device_status[dev_id] = DRV_STATUS_INITING;
    spin_unlock_irqrestore(&dev_manager_info->spinlock, flags);

    return 0;
}

STATIC int devdrv_manager_set_devdrv_info(u32 dev_id, struct devdrv_info *dev_info)
{
    unsigned long flags;

    if ((dev_id >= DEVDRV_MAX_DAVINCI_NUM) || (dev_manager_info == NULL)) {
        devdrv_drv_err("invalid dev_id(%u), or dev_manager_info is NULL\n", dev_id);
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

#ifdef CFG_FEATURE_OLD_DEVID_TRANS
int devdrv_get_devnum(u32 *num_dev)
{
    u32 dev_num;

    dev_num = devdrv_manager_get_devnum();
    if ((dev_num == 0) || (dev_num > DEVDRV_MAX_DAVINCI_NUM)) {
        devdrv_drv_err("invalid device number, dev_num = %d\n", dev_num);
        return -EFAULT;
    }

    *num_dev = dev_num;
    return 0;
}
EXPORT_SYMBOL(devdrv_get_devnum);

int devdrv_get_vdevnum(u32 *num_dev)
{
    u32 vdev_num;

    vdev_num = devdrv_manager_get_vdevnum();
    if ((vdev_num == 0) || (vdev_num > MAX_VDEV_NUM)) {
        devdrv_drv_err("invalid vdevice number. (vdev_num=%d)\n", vdev_num);
        return -EFAULT;
    }

    *num_dev = vdev_num;
    return 0;
}
EXPORT_SYMBOL(devdrv_get_vdevnum);
#endif

#ifdef CFG_FEATURE_OLD_DEVID_TRANS
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
    if ((num_dev > DEVDRV_PF_DEV_MAX_NUM) || (num_dev == 0) || (num_dev > device_num)) {
        devdrv_drv_err("wrong dev number, num_dev = %u max_num :%u\n", num_dev, device_num);
        return -ENODEV;
    }

    for (i = 0; i < DEVDRV_PF_DEV_MAX_NUM; i++) {
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

int devdrv_get_vdevids(u32 *devices, u32 dev_num)
{
    struct devdrv_info *dev_info = NULL;
    u32 num_vdev;
    u32 i;
    u32 j = 0;

    if (devices == NULL) {
        devdrv_drv_err("Invalid parameter.\n");
        return -EINVAL;
    }

    num_vdev = devdrv_manager_get_vdevnum();
    if ((num_vdev > MAX_VDEV_NUM) || (num_vdev == 0) || (num_vdev > dev_num)) {
        devdrv_drv_err("Invalid device number. (num_vdev=%u; max_num=%u)\n", num_vdev, dev_num);
        return -ENODEV;
    }

    for (i = VDAVINCI_VDEV_OFFSET; i < DEVDRV_MAX_DAVINCI_NUM; i++) {
        dev_info = devdrv_manager_get_devdrv_info(i);
        if (dev_info == NULL) {
            continue;
        }

        if (j >= dev_num) {
            break;
        }

        devices[j] = dev_info->dev_id;
        j++;
    }

    if (j == 0) {
        devdrv_drv_err("Get dev_info for vdevice failed.\n");
        return -EFAULT;
    }

    return 0;
}
EXPORT_SYMBOL(devdrv_get_vdevids);
#endif

int devdrv_get_devinfo(unsigned int devid, struct devdrv_device_info *info)
{
    struct devdrv_platform_data *pdata = NULL;
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
    pdata = dev_info->pdata;
    /* check if received device ready message from device side */
    if (dev_info->dev_ready == 0) {
        devdrv_drv_err("device(%u) not ready!", dev_info->dev_id);
        return -ENODEV;
    }

    info->ctrl_cpu_ip = dev_info->ctrl_cpu_ip;
    info->ctrl_cpu_id = dev_info->ctrl_cpu_id;
    info->ctrl_cpu_core_num = dev_info->ctrl_cpu_core_num;
    info->ctrl_cpu_occupy_bitmap = dev_info->ctrl_cpu_occupy_bitmap;
    info->ctrl_cpu_endian_little = dev_info->ctrl_cpu_endian_little;
    info->ts_cpu_core_num = pdata->ts_pdata[0].ts_cpu_core_num;
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
    u32 vf_aicore_num_inused = 0;
    u32 vf_aicpu_num_inused = 0;
    u32 aicpu_bitmap = 0;

    if (devid >= DEVDRV_MAX_DAVINCI_NUM || vfid > VDAVINCI_MAX_VFID_NUM) {
        devdrv_drv_err("Invalid para,(devid=%u,vfid=%u).\n", devid, vfid);
        return -EINVAL;
    }

    if ((inuse == NULL) || (dev_manager_info == NULL) || (dev_manager_info->dev_info[devid] == NULL)) {
        devdrv_drv_err("inuse(%pK) or dev_manager_info(%pK) or dev_manager_info->dev_info[devid(%u)] is NULL.\n", inuse,
                       dev_manager_info, devid);
        return -EINVAL;
    }

    dev_info = dev_manager_info->dev_info[devid];

    if (tsdrv_is_ts_work(devid, 0) == false) {
        devdrv_drv_err("device(%u) is not working.\n", devid);
        return -ENXIO;
    }

    (void)hvdevmng_get_aicore_num(devid, vfid, &vf_aicore_num_inused);
    (void)hvdevmng_get_aicpu_num(devid, vfid, &vf_aicpu_num_inused, &aicpu_bitmap);

    inuse->ai_core_num = ((vfid == 0) ? (dev_info->inuse.ai_core_num) : (vf_aicore_num_inused));
    inuse->ai_core_error_bitmap = ((vfid == 0) ? (dev_info->inuse.ai_core_error_bitmap) : (0x0));
    inuse->ai_cpu_num = ((vfid == 0) ? (dev_info->inuse.ai_cpu_num) : (vf_aicpu_num_inused));
    inuse->ai_cpu_error_bitmap = ((vfid == 0) ? (dev_info->inuse.ai_cpu_error_bitmap) : (0x0));

    return 0;
}
EXPORT_SYMBOL(devdrv_get_core_inuse);

int devdrv_get_core_spec(u32 devid, u32 vfid, struct devdrv_hardware_spec *spec)
{
    struct devdrv_info *dev_info = NULL;
    u32 aicpu_bitmap = 0;

    if ((devid >= DEVDRV_MAX_DAVINCI_NUM) || (vfid > VDAVINCI_MAX_VFID_NUM)) {
        devdrv_drv_err("invalid dev_id(%u) vfid(%u).\n", devid, vfid);
        return -EINVAL;
    }

    if ((spec == NULL) || (dev_manager_info == NULL) || (dev_manager_info->dev_info[devid] == NULL)) {
        devdrv_drv_err("spec(%pK) or dev_manager_info(%pK) or dev_manager_info->dev_info[devid(%u)] is NULL.\n", spec,
                       dev_manager_info, devid);
        return -EINVAL;
    }

    dev_info = dev_manager_info->dev_info[devid];

    (void)hvdevmng_get_aicore_num(dev_info->dev_id, vfid, &spec->ai_core_num);
    (void)hvdevmng_get_aicpu_num(dev_info->dev_id, vfid, &spec->ai_cpu_num, &aicpu_bitmap);
    spec->first_ai_core_id = dev_info->ai_core_id;
    spec->first_ai_cpu_id = dev_info->ai_cpu_core_id;
    spec->ai_core_num_level = dev_info->pdata->ai_core_num_level;
    spec->ai_core_freq_level = dev_info->pdata->ai_core_freq_level;

    return 0;
}
EXPORT_SYMBOL(devdrv_get_core_spec);

STATIC inline void *devdrv_manager_vzalloc(size_t alloc_size)
{
    return ka_vmalloc(alloc_size, GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT, PAGE_KERNEL);
}

STATIC int devdrv_manager_ipc_notify_init(struct devdrv_manager_context *dev_manager_context)
{
    struct ipc_notify_info *ipc_notify_info = NULL;
    size_t ipc_size = sizeof(struct ipc_notify_info);

    ipc_notify_info = (struct ipc_notify_info *)kzalloc(ipc_size, GFP_KERNEL | __GFP_ACCOUNT);
    if (ipc_notify_info == NULL) {
        devdrv_drv_err("vmalloc ipc_notify_info is NULL.\n");
        return -ENOMEM;
    }

    ipc_notify_info->create_fd_num = 0;
    ipc_notify_info->open_fd_num = 0;
    INIT_LIST_HEAD(&ipc_notify_info->create_list_head);
    INIT_LIST_HEAD(&ipc_notify_info->open_list_head);
    mutex_init(&ipc_notify_info->info_mutex);

    dev_manager_context->ipc_notify_info = ipc_notify_info;

    return 0;
}

STATIC void devdrv_manager_ipc_notify_uninit(struct devdrv_manager_context *dev_manager_context)
{
    struct ipc_notify_info *ipc_notify_info = NULL;

    ipc_notify_info = dev_manager_context->ipc_notify_info;
    if (ipc_notify_info == NULL) {
        return;
    }

    mutex_destroy(&ipc_notify_info->info_mutex);
    kfree(ipc_notify_info);
    ipc_notify_info = NULL;
    dev_manager_context->ipc_notify_info = NULL;
}

STATIC struct devdrv_manager_context *devdrv_manager_context_init(void)
{
    struct devdrv_manager_context *dev_manager_context = NULL;
    size_t ctx_size = sizeof(struct devdrv_manager_context);

#ifdef CFG_FEATURE_OLD_DEVID_TRANS
    if (devdrv_manager_container_task_struct_check(current)) {
        devdrv_drv_err("current is invalid.\n");
        return NULL;
    }
#endif

    dev_manager_context = (struct devdrv_manager_context *)kzalloc(ctx_size, GFP_KERNEL | __GFP_ACCOUNT);
    if (dev_manager_context == NULL) {
        devdrv_drv_err("vmalloc dev_manager_context is NULL.\n");
        return NULL;
    }

    dev_manager_context->pid = current->pid;
    dev_manager_context->tgid = current->tgid;
    dev_manager_context->task = current;
    dev_manager_context->start_time = current->start_time;
    dev_manager_context->real_start_time = get_start_time(current);
    dev_manager_context->mnt_ns = current->nsproxy->mnt_ns;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0)
    dev_manager_context->pid_ns = current->nsproxy->pid_ns_for_children;
#else
    dev_manager_context->pid_ns = current->nsproxy->pid_ns;
#endif

    if (devdrv_manager_ipc_notify_init(dev_manager_context)) {
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
    devdrv_manager_ipc_notify_uninit(dev_manager_context);
    kfree(dev_manager_context);
    dev_manager_context = NULL;
}

STATIC int devdrv_manager_open(struct inode *inode, struct file *filep)
{
    struct devdrv_manager_context *dev_manager_context = NULL;
#ifdef CFG_FEATURE_OLD_DEVID_TRANS
    u32 docker_id;
    int ret;
#endif

    dev_manager_context = devdrv_manager_context_init();
    if (dev_manager_context == NULL) {
        devdrv_drv_err("context init failed\n");
        return -ENOMEM;
    }

#ifdef CFG_FEATURE_OLD_DEVID_TRANS
    /* The current environment is in a privilege docker, don't need to overlap table */
    if (devdrv_manager_container_is_admin()) {
        goto out;
    }

    ret = devdrv_manager_container_table_overlap(dev_manager_context, &docker_id);
    if (ret) {
        devdrv_manager_context_uninit(dev_manager_context);
        devdrv_drv_err("update_item failed, ret(%d).\n", ret);
        return -EINVAL;
    }
    dev_manager_context->docker_id = docker_id;

out:
#endif
    filep->private_data = dev_manager_context;
    return 0;
}

STATIC void devdrv_manager_resource_recycle(struct devdrv_manager_context *dev_manager_context)
{
    struct ipc_notify_info *ipc_notify_info = NULL;
    u32 ipc_notify_create_num = 0;
    u32 ipc_notify_open_num = 0;

    ipc_notify_info = dev_manager_context->ipc_notify_info;
    if (ipc_notify_info != NULL) {
        ipc_notify_open_num = ipc_notify_info->open_fd_num;
        ipc_notify_create_num = ipc_notify_info->create_fd_num;
    }

    if ((ipc_notify_open_num > 0) || (ipc_notify_create_num > 0)) {
        devdrv_drv_info("ipc resource leak, "
                        "ipc_notify_create_num = %u, "
                        "ipc_notify_open_num = %u\n",
                        ipc_notify_create_num, ipc_notify_open_num);
        devdrv_manager_ops_sem_down_read();
        if (devdrv_host_drv_ops.ipc_notify_release_recycle != NULL) {
            devdrv_host_drv_ops.ipc_notify_release_recycle(dev_manager_context);
        }
        devdrv_manager_ops_sem_up_read();
    }

    devdrv_flush_p2p(dev_manager_context->pid);
    devdrv_manager_context_uninit(dev_manager_context);
}

STATIC int devdrv_manager_release(struct inode *inode, struct file *filep)
{
    struct devdrv_manager_context *dev_manager_context = NULL;

    if ((filep == NULL) || (filep->private_data == NULL)) {
        devdrv_drv_err("filep is %s.\n", (filep == NULL) ? "NULL" : "OK");
        return -EINVAL;
    }

    dev_manager_context = filep->private_data;
    devdrv_host_black_box_close_check(dev_manager_context->tgid);
    devdrv_manager_resource_recycle(dev_manager_context);

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
    u32 dev_id = DEVDRV_MAX_DAVINCI_NUM + 1;
    struct devdrv_info *dev_info = NULL;
    struct devdrv_pci_info pci_info;
    u32 virt_id;
    u32 vfid = 0;
    int ret;
    unsigned long flags;

    if (copy_from_user_safe(&pci_info, (void *)(uintptr_t)arg, sizeof(struct devdrv_pci_info))) {
        return -EFAULT;
    }
    virt_id = pci_info.dev_id;
    ret = devdrv_manager_trans_and_check_id(virt_id, &dev_id, &vfid);
    if (ret != 0) {
        devdrv_drv_err("can't transfor virt id %u,ret=%d\n", virt_id, ret);
        return ret;
    }

    if (dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("invalid dev_id(%u)\n", dev_id);
        return -ENODEV;
    }

    spin_lock_irqsave(&dev_manager_info->spinlock, flags);
    dev_info = dev_manager_info->dev_info[dev_id];
    if ((dev_info == NULL) || (dev_info->pdata == NULL)) {
        spin_unlock_irqrestore(&dev_manager_info->spinlock, flags);
        devdrv_drv_err("dev_info[%u] is NULL\n", dev_id);
        return -EFAULT;
    }
    pdata = dev_info->pdata;
    pci_info.bus_number = pdata->pci_info.bus_number;
    pci_info.dev_number = pdata->pci_info.dev_number;
    pci_info.function_number = pdata->pci_info.function_number;
    spin_unlock_irqrestore(&dev_manager_info->spinlock, flags);

    if (copy_to_user_safe((void *)(uintptr_t)arg, &pci_info, sizeof(struct devdrv_pci_info)))
        return -EFAULT;

    return 0;
}

STATIC int devdrv_manager_get_device_status(struct file *filep, unsigned int cmd, unsigned long arg)
{
    enum devdrv_ts_status ts_status;
    u32 loc_id;
    u32 vfid = 0;
    u32 status;
    int ret;

    u32 phys_id = DEVDRV_MAX_DAVINCI_NUM + 1;
    ret = copy_from_user_safe(&loc_id, (void *)(uintptr_t)arg, sizeof(u32));
    if (ret) {
        devdrv_drv_err("copy from user failed, ret(%d).\n", ret);
        return -EINVAL;
    }
    if (loc_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("invalid dev_id(%u).\n", loc_id);
        return -EINVAL;
    }

    if (devdrv_manager_container_logical_id_to_physical_id(loc_id, &phys_id, &vfid) != 0) {
        devdrv_drv_err("can't transfor virt id %u.\n", loc_id);
        return -EFAULT;
    }

    ts_status = tsdrv_get_ts_status(phys_id, 0);

    if ((dev_manager_info == NULL) || (dev_manager_info->dev_info[phys_id] == NULL)) {
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
        devdrv_drv_err("copy to user failed, ret(%d). dev_id(%u)\n", ret, phys_id);
        return -EINVAL;
    }

    return 0;
}

STATIC int devdrv_manager_get_core(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_hardware_inuse inuse;
    struct devdrv_hardware_spec spec;
    u32 phys_id = DEVDRV_MAX_DAVINCI_NUM + 1;
    u32 vfid = 0;
    int ret;

    switch (cmd) {
        case DEVDRV_MANAGER_GET_CORE_SPEC:
            ret = copy_from_user_safe(&spec, (void *)((uintptr_t)arg), sizeof(struct devdrv_hardware_spec));
            if (ret) {
                devdrv_drv_err("copy_from_user_safe failed, ret(%d).\n", ret);
                return ret;
            }
            if (devdrv_manager_container_logical_id_to_physical_id(spec.devid, &phys_id, &vfid) != 0) {
                devdrv_drv_err("can't transfor virt id %u \n", spec.devid);
                return -EFAULT;
            }

            ret = devdrv_get_core_spec(phys_id, vfid, &spec);
            if (ret) {
                devdrv_drv_err("devdrv_get_core_spec failed, ret(%d), dev_id(%u).\n", ret, phys_id);
                return ret;
            }
            ret = copy_to_user_safe((void *)((uintptr_t)arg), &spec, sizeof(struct devdrv_hardware_spec));
            if (ret) {
                devdrv_drv_err("copy_to_user_safe failed, ret(%d). dev_id(%u)\n", ret, phys_id);
                return ret;
            }
            break;
        case DEVDRV_MANAGER_GET_CORE_INUSE:
            ret = copy_from_user_safe(&inuse, (void *)((uintptr_t)arg), sizeof(struct devdrv_hardware_inuse));
            if (ret) {
                devdrv_drv_err("copy_from_user_safe failed, ret(%d).\n", ret);
                return ret;
            }

            if (devdrv_manager_container_logical_id_to_physical_id(inuse.devid, &phys_id, &vfid) != 0) {
                devdrv_drv_err("can't transfor virt id %u \n", inuse.devid);
                return -EFAULT;
            }

            ret = devdrv_get_core_inuse(phys_id, vfid, &inuse);
            if (ret) {
                devdrv_drv_err("devdrv_get_core_inuse failed, ret(%d), dev_id(%u).\n", ret, phys_id);
                return ret;
            }
            ret = copy_to_user_safe((void *)(uintptr_t)arg, &inuse, sizeof(struct devdrv_hardware_inuse));
            if (ret) {
                devdrv_drv_err("copy_to_user_safe failed, ret(%d). dev_id(%u)\n", ret, phys_id);
                return ret;
            }
            break;
        default:
            devdrv_drv_err("invalid cmd.\n");
            return -EINVAL;
    }
    return 0;
}

STATIC int devdrv_manager_get_probe_list(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_manager_hccl_devinfo *hccl_devinfo = NULL;
    struct devdrv_manager_info *devmng_info = NULL;
    u32 i;
    u32 index = 0;

    if (devdrv_manager_container_is_in_container()) {
        devdrv_drv_err("not support using in container.\n");
        return -EINVAL;
    }

    devmng_info = devdrv_get_manager_info();
    if (devmng_info == NULL) {
        devdrv_drv_err("devmng info is NULL\n");
        return -EINVAL;
    }

    hccl_devinfo = (struct devdrv_manager_hccl_devinfo *)kzalloc(sizeof(struct devdrv_manager_hccl_devinfo),
        GFP_KERNEL | __GFP_ACCOUNT);
    if (hccl_devinfo == NULL) {
        devdrv_drv_err("Alloc memory for hccl device info failed.\n");
        return -ENOMEM;
    }

    for (i = 0; i < DEVDRV_MAX_DAVINCI_NUM; i++) {
        hccl_devinfo->devids[i] = DEVDRV_INVALID_PHY_ID;
        if (CHECK_BIT_64(devmng_info->prob_device_bitmap[i / BITS_PER_LONG_LONG], i % BITS_PER_LONG_LONG)) {
            hccl_devinfo->devids[index++] = i;
        }
    }
    hccl_devinfo->num_dev = index;

    if (copy_to_user_safe((void *)(uintptr_t)arg, hccl_devinfo, sizeof(struct devdrv_manager_hccl_devinfo))) {
        devdrv_drv_err("copy to user failed.\n");
        kfree(hccl_devinfo);
        hccl_devinfo = NULL;
        return -EINVAL;
    }

    kfree(hccl_devinfo);
    hccl_devinfo = NULL;
    return 0;
}

#ifdef CFG_FEATURE_OLD_DEVID_TRANS
STATIC int devdrv_manager_get_devids(unsigned long arg)
{
    struct devdrv_manager_hccl_devinfo *hccl_devinfo = NULL;
    u32 num_dev;
    int ret;

    num_dev = devdrv_manager_get_devnum();
    if ((num_dev > DEVDRV_PF_DEV_MAX_NUM) || (num_dev == 0)) {
        devdrv_drv_err("invalid num_dev(%u)\n", num_dev);
        return -EINVAL;
    }

    hccl_devinfo =
        (struct devdrv_manager_hccl_devinfo *)kzalloc(sizeof(struct devdrv_manager_hccl_devinfo),
            GFP_KERNEL | __GFP_ACCOUNT);
    if (hccl_devinfo == NULL) {
        devdrv_drv_err("Alloc memory for hccl device info failed.\n");
        return -ENOMEM;
    }

    devdrv_drv_debug("dev_num = %u\n", num_dev);
    hccl_devinfo->num_dev = num_dev;

    ret = devdrv_manager_container_get_devids(hccl_devinfo->devids, DEVDRV_MAX_DAVINCI_NUM, &num_dev,
        current->nsproxy->mnt_ns, DEVDRV_PF_TYPE);
    if (ret != 0) {
        devdrv_drv_err("get devlist failed, ret(%d)\n", ret);
        kfree(hccl_devinfo);
        hccl_devinfo = NULL;
        return -ENODEV;
    }

    if (copy_to_user_safe((void *)(uintptr_t)arg, hccl_devinfo, sizeof(struct devdrv_manager_hccl_devinfo))) {
        devdrv_drv_err("copy from user failed.\n");
        kfree(hccl_devinfo);
        hccl_devinfo = NULL;
        return -EINVAL;
    }

    kfree(hccl_devinfo);
    hccl_devinfo = NULL;
    return 0;
}

STATIC int devdrv_manager_get_vdevids(unsigned long arg)
{
    struct devdrv_manager_hccl_devinfo *hccl_devinfo = NULL;
    u32 num_vdev;
    int ret;

    num_vdev = devdrv_manager_get_vdevnum();
    if ((num_vdev > MAX_VDEV_NUM) || (num_vdev == 0)) {
        devdrv_drv_err("Invalid device number. (num_dev=%u)\n", num_vdev);
        return -EINVAL;
    }

    hccl_devinfo = (struct devdrv_manager_hccl_devinfo *)kzalloc(sizeof(struct devdrv_manager_hccl_devinfo),
        GFP_KERNEL | __GFP_ACCOUNT);
    if (hccl_devinfo == NULL) {
        devdrv_drv_err("Alloc memory for hccl device info failed.\n");
        return -ENOMEM;
    }

    devdrv_drv_debug("(vdev_num=%u)\n", num_vdev);
    hccl_devinfo->num_dev = num_vdev;

    ret = devdrv_manager_container_get_devids(hccl_devinfo->devids, DEVDRV_MAX_DAVINCI_NUM, &num_vdev,
        current->nsproxy->mnt_ns, DEVDRV_VF_TYPE);
    if (ret != 0) {
        devdrv_drv_err("Get devlist failed. (ret=%d)\n", ret);
        kfree(hccl_devinfo);
        hccl_devinfo = NULL;
        return -ENODEV;
    }

    if (copy_to_user_safe((void *)(uintptr_t)arg, hccl_devinfo, sizeof(struct devdrv_manager_hccl_devinfo))) {
        devdrv_drv_err("Copy from user failed.\n");
        kfree(hccl_devinfo);
        hccl_devinfo = NULL;
        return -EINVAL;
    }

    kfree(hccl_devinfo);
    hccl_devinfo = NULL;
    return 0;
}
#endif

STATIC int devdrv_manager_get_container_devids(unsigned long arg)
{
    struct devdrv_manager_hccl_devinfo *hccl_devinfo = NULL;
    int ret;

    if ((current->nsproxy == NULL) || (!devdrv_manager_container_is_host_system(current->nsproxy->mnt_ns))) {
        devdrv_drv_err("Do not have permission in container or virtual machine.\n");
        return -EPERM;
    }

    hccl_devinfo =
        (struct devdrv_manager_hccl_devinfo *)kzalloc(sizeof(struct devdrv_manager_hccl_devinfo),
            GFP_KERNEL | __GFP_ACCOUNT);
    if (hccl_devinfo == NULL) {
        devdrv_drv_err("Alloc memory for hccl device info failed.\n");
        return -ENOMEM;
    }

#ifdef CFG_FEATURE_OLD_DEVID_TRANS
    ret = devdrv_manager_container_get_devids_list(hccl_devinfo);
#else
    ret = devdrv_get_devids(hccl_devinfo->devids, DEVDRV_MAX_DAVINCI_NUM);
#endif
    if (ret) {
        devdrv_drv_err("get container devlist failed, ret(%d)\n", ret);
        kfree(hccl_devinfo);
        hccl_devinfo = NULL;
        return -ENODEV;
    }

    if (copy_to_user_safe((void *)(uintptr_t)arg, hccl_devinfo, sizeof(struct devdrv_manager_hccl_devinfo))) {
        devdrv_drv_err("copy from user failed, ret(%d).\n", ret);
        kfree(hccl_devinfo);
        hccl_devinfo = NULL;
        return -EINVAL;
    }

    kfree(hccl_devinfo);
    hccl_devinfo = NULL;
    return 0;
}

STATIC int devdrv_manager_get_devinfo(unsigned long arg)
{
    struct devdrv_manager_hccl_devinfo *hccl_devinfo = NULL;
    struct devdrv_platform_data *pdata = NULL;
    struct devdrv_info *dev_info = NULL;
    u32 phys_id = DEVDRV_MAX_DAVINCI_NUM + 1;
    u32 aicpu_occupy_bitmap = 0;
    u32 vfid = 0;
    u32 dev_id;
    int ret;

    hccl_devinfo = (struct devdrv_manager_hccl_devinfo *)kzalloc(sizeof(struct devdrv_manager_hccl_devinfo),
        GFP_KERNEL | __GFP_ACCOUNT);
    if (hccl_devinfo == NULL) {
        devdrv_drv_err("Alloc memory for hccl device info failed.\n");
        return -ENOMEM;
    }

    if (copy_from_user_safe(hccl_devinfo, (void *)(uintptr_t)arg, sizeof(struct devdrv_manager_hccl_devinfo))) {
        devdrv_drv_err("Copy from user failed.\n");
        ret = -EINVAL;
        goto FREE_DEV_INFO_EXIT;
    }

    dev_id = hccl_devinfo->dev_id;
    if (dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("invalid dev_id(%u)\n", hccl_devinfo->dev_id);
        ret = -ENODEV;
        goto FREE_DEV_INFO_EXIT;
    }
    ret = devdrv_manager_trans_and_check_id(dev_id, &phys_id, &vfid);
    if (ret != 0) {
        devdrv_drv_err("can't transfor virt id %u, ret=%d\n", dev_id, ret);
        goto FREE_DEV_INFO_EXIT;
    }

    if ((phys_id >= DEVDRV_MAX_DAVINCI_NUM) || (vfid > VDAVINCI_MAX_VFID_NUM)) {
        devdrv_drv_err("get invalid output phys_id(%u) vfid(%u)\n", phys_id, vfid);
        ret = -EINVAL;
        goto FREE_DEV_INFO_EXIT;
    }

    dev_info = devdrv_manager_get_devdrv_info(phys_id);
    if (dev_info == NULL) {
        devdrv_drv_err("device(%u) is not initialized\n", phys_id);
        ret = -ENODEV;
        goto FREE_DEV_INFO_EXIT;
    }

    ret = dms_hotreset_task_cnt_increase(phys_id);
    if (ret != 0) {
        devdrv_drv_err("Hotreset task cnt increase failed. (dev_id=%u; ret=%d)\n", phys_id, ret);
        goto FREE_DEV_INFO_EXIT;
    }

    atomic_inc(&dev_info->occupy_ref);
    if (dev_info->status == DEVINFO_STATUS_REMOVED) {
        atomic_dec(&dev_info->occupy_ref);
        devdrv_drv_warn("dev %d has been reset\n", dev_info->dev_id);
        ret = -EINVAL;
        goto HOT_RESET_CNT_EXIT;
    }

    pdata = dev_info->pdata;
    (void)hvdevmng_get_aicore_num(dev_info->dev_id, vfid, &hccl_devinfo->ai_core_num);
    (void)hvdevmng_get_aicpu_num(dev_info->dev_id, vfid, &hccl_devinfo->ai_cpu_core_num, &aicpu_occupy_bitmap);

    hccl_devinfo->aicore_freq = dev_info->aicore_freq;
    hccl_devinfo->ctrl_cpu_core_num = dev_info->ctrl_cpu_core_num;
    hccl_devinfo->ctrl_cpu_occupy_bitmap = dev_info->ctrl_cpu_occupy_bitmap;
    hccl_devinfo->ctrl_cpu_id = dev_info->ctrl_cpu_id;
    hccl_devinfo->ctrl_cpu_ip = dev_info->ctrl_cpu_ip;

    /* 1:little endian 0:big endian */
    hccl_devinfo->ctrl_cpu_endian_little = dev_info->ctrl_cpu_endian_little;
    hccl_devinfo->ts_cpu_core_num = pdata->ts_pdata[0].ts_cpu_core_num;
    hccl_devinfo->env_type = dev_info->env_type;
    hccl_devinfo->ai_core_id = dev_info->ai_core_id;
    hccl_devinfo->ai_cpu_core_id = dev_info->ai_cpu_core_id;
    hccl_devinfo->ai_cpu_bitmap = (vfid == 0) ? dev_info->aicpu_occupy_bitmap : aicpu_occupy_bitmap;
    hccl_devinfo->hardware_version = dev_info->hardware_version;
    hccl_devinfo->ts_num = devdrv_manager_get_ts_num(dev_info);
    hccl_devinfo->ffts_type = dev_info->ffts_type;
    hccl_devinfo->chip_id = dev_info->chip_id;
    hccl_devinfo->die_id = dev_info->die_id;
#ifdef CFG_SOC_PLATFORM_MINIV2
    hccl_devinfo->vector_core_num = (vfid == 0) ? dev_info->vector_core_num : hccl_devinfo->ai_cpu_core_num;
#else
    hccl_devinfo->vector_core_num = dev_info->vector_core_num;
#endif
    hccl_devinfo->vector_core_freq = dev_info->vector_core_freq;
    devdrv_drv_debug("ctrl_cpu_ip(0x%x), ts_cpu_core_num(%d), dev_id(%u)\n", dev_info->ctrl_cpu_ip,
                     hccl_devinfo->ts_cpu_core_num, phys_id);

    atomic_dec(&dev_info->occupy_ref);

    if (copy_to_user_safe((void *)(uintptr_t)arg, hccl_devinfo, sizeof(struct devdrv_manager_hccl_devinfo))) {
        devdrv_drv_err("copy to user failed. dev_id(%u)\n", phys_id);
        ret = -EFAULT;
    }

HOT_RESET_CNT_EXIT:
    dms_hotreset_task_cnt_decrease(phys_id);
FREE_DEV_INFO_EXIT:
    kfree(hccl_devinfo);
    hccl_devinfo = NULL;
    return ret;
}

STATIC void devdrv_manager_set_computing_value(struct devdrv_manager_hccl_devinfo *hccl_devinfo,
    struct devdrv_info *dev_info, bool valid)
{
    int i;

    if (valid) {
        for (i = 0; i < DEVDRV_MAX_COMPUTING_POWER_TYPE; i++) {
            hccl_devinfo->computing_power[i] = dev_info->computing_power[i];
        }
    } else {
        for (i = 0; i < DEVDRV_MAX_COMPUTING_POWER_TYPE; i++) {
            hccl_devinfo->computing_power[i] = DEVDRV_COMPUTING_VALUE_ERROR;
        }
    }
}

STATIC int devdrv_manager_get_h2d_devinfo(unsigned long arg)
{
    struct devdrv_manager_hccl_devinfo *hccl_devinfo = NULL;
    struct devdrv_platform_data *pdata = NULL;
    struct devdrv_info *dev_info = NULL;
    u32 phys_id = DEVDRV_MAX_DAVINCI_NUM + 1;
    u32 vfid = 0;
    u32 dev_id;
    int ret;

    hccl_devinfo = (struct devdrv_manager_hccl_devinfo *)kzalloc(sizeof(struct devdrv_manager_hccl_devinfo),
        GFP_KERNEL | __GFP_ACCOUNT);
    if (hccl_devinfo == NULL) {
        devdrv_drv_err("Alloc memory for hccl device info failed.\n");
        return -ENOMEM;
    }

    if (copy_from_user_safe(hccl_devinfo, (void *)(uintptr_t)arg, sizeof(struct devdrv_manager_hccl_devinfo))) {
        devdrv_drv_err("copy from user failed.\n");
        ret = -EINVAL;
        goto FREE_DEV_INFO_EXIT;
    }

    dev_id = hccl_devinfo->dev_id;
    if (dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("invalid dev_id(%u)\n", hccl_devinfo->dev_id);
        ret = -ENODEV;
        goto FREE_DEV_INFO_EXIT;
    }
    if (devdrv_manager_container_logical_id_to_physical_id(dev_id, &phys_id, &vfid) != 0) {
        devdrv_drv_err("can't transfor virt id %u \n", dev_id);
        ret = -EFAULT;
        goto FREE_DEV_INFO_EXIT;
    }

    dev_info = devdrv_manager_get_devdrv_info(phys_id);
    if (dev_info == NULL) {
        devdrv_drv_err("device(%u) is not initialized\n", phys_id);
        ret = -ENODEV;
        goto FREE_DEV_INFO_EXIT;
    }

    ret = dms_hotreset_task_cnt_increase(phys_id);
    if (ret != 0) {
        devdrv_drv_err("Hotreset task cnt increase failed. (dev_id=%u; ret=%d)\n", phys_id, ret);
        goto FREE_DEV_INFO_EXIT;
    }

    atomic_inc(&dev_info->occupy_ref);
    if (dev_info->status == DEVINFO_STATUS_REMOVED) {
        atomic_dec(&dev_info->occupy_ref);
        devdrv_drv_warn("dev %d has been reset\n", dev_info->dev_id);
        dms_hotreset_task_cnt_decrease(phys_id);
        ret = -EINVAL;
        goto FREE_DEV_INFO_EXIT;
    }

    pdata = dev_info->pdata;
    hccl_devinfo->ai_core_num = dev_info->ai_core_num;
    hccl_devinfo->aicore_freq = dev_info->aicore_freq;
    hccl_devinfo->ai_cpu_core_num = dev_info->ai_cpu_core_num;
    hccl_devinfo->ctrl_cpu_core_num = dev_info->ctrl_cpu_core_num;
    hccl_devinfo->ctrl_cpu_occupy_bitmap = dev_info->ctrl_cpu_occupy_bitmap;

    /* 1:little endian 0:big endian */
    hccl_devinfo->ctrl_cpu_endian_little = dev_info->ctrl_cpu_endian_little;
    hccl_devinfo->ctrl_cpu_id = dev_info->ctrl_cpu_id;
    hccl_devinfo->ctrl_cpu_ip = dev_info->ctrl_cpu_ip;
    hccl_devinfo->ts_cpu_core_num = pdata->ts_pdata[0].ts_cpu_core_num;
    hccl_devinfo->env_type = dev_info->env_type;
    hccl_devinfo->ai_core_id = dev_info->ai_core_id;
    hccl_devinfo->ai_cpu_core_id = dev_info->ai_cpu_core_id;
    hccl_devinfo->ai_cpu_bitmap = dev_info->aicpu_occupy_bitmap;
    hccl_devinfo->hardware_version = dev_info->hardware_version;
    hccl_devinfo->ts_num = devdrv_manager_get_ts_num(dev_info);

    if (devdrv_manager_h2d_sync_get_devinfo(dev_info)) {
        devdrv_drv_err("device info get failed. dev_id(%u)\n", phys_id);
        devdrv_manager_set_computing_value(hccl_devinfo, dev_info, false);
    } else {
        devdrv_manager_set_computing_value(hccl_devinfo, dev_info, true);
    }
    hccl_devinfo->cpu_system_count = dev_info->cpu_system_count;
    hccl_devinfo->monotonic_raw_time_ns = dev_info->monotonic_raw_time_ns;
    hccl_devinfo->ffts_type = dev_info->ffts_type;
    hccl_devinfo->vector_core_num = dev_info->vector_core_num;
    hccl_devinfo->vector_core_freq = dev_info->vector_core_freq;
    devdrv_drv_debug("ctrl_cpu_ip(0x%x), ts_cpu_core_num(%d), dev_id(%u)\n", dev_info->ctrl_cpu_ip,
                     hccl_devinfo->ts_cpu_core_num, phys_id);

    atomic_dec(&dev_info->occupy_ref);

    if (copy_to_user_safe((void *)(uintptr_t)arg, hccl_devinfo, sizeof(struct devdrv_manager_hccl_devinfo))) {
        devdrv_drv_err("copy to user failed. dev_id(%u)\n", phys_id);
        dms_hotreset_task_cnt_decrease(phys_id);
        ret = -EFAULT;
        goto FREE_DEV_INFO_EXIT;
    }

    dms_hotreset_task_cnt_decrease(phys_id);

FREE_DEV_INFO_EXIT:
    kfree(hccl_devinfo);
    hccl_devinfo = NULL;
    return ret;
}

STATIC int devdrv_manager_get_dev_info_by_phyid(unsigned long arg)
{
    int ret;
    struct devdrv_phy_get_devinfo_para para = {0};

    if (devdrv_manager_container_is_in_container()) {
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
    unsigned int dev_id, virt_id, vfid;
    int ret;

    ret = copy_from_user_safe(&pcie_id_info, (void *)((uintptr_t)arg), sizeof(struct dmanage_pcie_id_info));
    if (ret) {
        devdrv_drv_err("copy_from_user_safe failed.\n");
        return ret;
    }

    virt_id = pcie_id_info.davinci_id;
    ret = devdrv_manager_trans_and_check_id(virt_id, &dev_id, &vfid);
    if (ret != 0) {
        devdrv_drv_err("can't transfor virt id %u, ret=%d\n", virt_id, ret);
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
        dms_hotreset_task_cnt_decrease(dev_id);
        return ret;
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
        dms_hotreset_task_cnt_decrease(dev_id);
        return ret;
    }
    dms_hotreset_task_cnt_decrease(dev_id);
    return 0;
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
#ifdef CFG_FEATURE_OLD_DEVID_TRANS
        case DEVDRV_MANAGER_GET_DEVIDS:
            ret = devdrv_manager_get_devids(arg);
            break;
#endif
        case DEVDRV_MANAGER_GET_CONTAINER_DEVIDS:
            ret = devdrv_manager_get_container_devids(arg);
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
#ifdef CFG_FEATURE_OLD_DEVID_TRANS
        case DEVDRV_MANAGER_GET_VDEVIDS:
            ret = devdrv_manager_get_vdevids(arg);
            break;
#endif
        default:
            ret = -EINVAL;
            break;
    }

    return ret;
}

STATIC int devdrv_manager_get_tsdrv_dev_com_info(struct file *filep,
    unsigned int cmd, unsigned long arg)
{
    struct tsdrv_dev_com_info dev_com_info;

    dev_com_info.mach_type = PHY_MACHINE_TYPE;
    dev_com_info.ts_num = 1;

    if (copy_to_user_safe((void *)(uintptr_t)arg, &dev_com_info, sizeof(struct tsdrv_dev_com_info))) {
        devdrv_drv_err("copy to user failed.\n");
        return -EFAULT;
    }

    return 0;
}

int devdrv_manager_shm_info_check(struct devdrv_info *dev_info)
{
    if ((dev_info == NULL) || (dev_info->shm_status == NULL) ||
        (dev_info->shm_head == NULL)) {
        devdrv_drv_err("dev_info is NULL or dev_info->shm_status/shm_head is NULL.\n");
        return -EFAULT;
    }

    if ((dev_info->shm_head->head_info.version != DEVMNG_SHM_INFO_HEAD_VERSION) ||
        (dev_info->shm_head->head_info.magic != DEVMNG_SHM_INFO_HEAD_MAGIC)) {
        devdrv_drv_warn("dev(%u) version of share memory in host is 0x%llx, "
                        "the version in device is 0x%llx, magic is 0x%x.\n",
                        dev_info->dev_id, DEVMNG_SHM_INFO_HEAD_VERSION,
                        dev_info->shm_head->head_info.version,
                        dev_info->shm_head->head_info.magic);
        return -EFAULT;
    }

    return 0;
}

STATIC int devdrv_manager_get_device_startup_status(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    unsigned int phys_id;
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
        return -EFAULT;
    }

    dev_info = devdrv_manager_get_devdrv_info(phys_id);
    if (dev_info == NULL) {
        para.dmp_started = false;
        para.device_process_status = DSMI_BOOT_STATUS_UNINIT;
        goto startup_status_out;
    }

    atomic_inc(&dev_info->occupy_ref);
    if (dev_info->status == DEVINFO_STATUS_REMOVED) {
        atomic_dec(&dev_info->occupy_ref);
        para.dmp_started = false;
        para.device_process_status = DSMI_BOOT_STATUS_UNINIT;
        goto startup_status_out;
    }

    if ((dev_info->shm_head == NULL) || (dev_info->shm_status == NULL) ||
        (dev_info->shm_head->head_info.magic != DEVMNG_SHM_INFO_HEAD_MAGIC)) {
        para.dmp_started = false;
        para.device_process_status = DSMI_BOOT_STATUS_UNINIT;
    } else if (dev_info->shm_head->head_info.version != DEVMNG_SHM_INFO_HEAD_VERSION) {
        devdrv_drv_err("dev(%u) version of share memory in host is 0x%llx, "
                       "the version in device is 0x%llx, magic is 0x%x.\n",
                       dev_info->dev_id, DEVMNG_SHM_INFO_HEAD_VERSION,
                       dev_info->shm_head->head_info.version,
                       dev_info->shm_head->head_info.magic);
        atomic_dec(&dev_info->occupy_ref);
        return -EINVAL;
    } else {
        if (dev_info->dmp_started == false)
            dev_info->dmp_started = devdrv_manager_h2d_query_dmp_started(phys_id);

        para.dmp_started = dev_info->dmp_started;
        para.device_process_status = (unsigned int)dev_info->shm_status->os_status;
    }
    atomic_dec(&dev_info->occupy_ref);

startup_status_out:
    ret = copy_to_user_safe((void *)(uintptr_t)arg, &para, sizeof(struct devdrv_device_work_status));
    if (ret != 0) {
        devdrv_drv_err("copy to user failed, ret(%d)\n", ret);
        return -EINVAL;
    }

    return 0;
}

STATIC int devdrv_try_get_dev_info_occupy(struct devdrv_info *dev_info)
{
    if (dev_info == NULL) {
        devdrv_drv_err("The dev_info is NULL\n");
        return -EFAULT;
    }

    atomic_inc(&dev_info->occupy_ref);
    if (dev_info->status == DEVINFO_STATUS_REMOVED) {
        atomic_dec(&dev_info->occupy_ref);
        devdrv_drv_err("The dev_info has been remove.\n");
        return -EFAULT;
    }

    return 0;
}

STATIC void devdrv_put_dev_info_occupy(struct devdrv_info *dev_info)
{
    if (dev_info == NULL) {
        devdrv_drv_err("The dev_info is NULL\n");
        return;
    }

    atomic_dec(&dev_info->occupy_ref);
}

STATIC int devdrv_manager_get_device_health_status(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    unsigned int phys_id;
    unsigned int vfid = 0;
    struct devdrv_info *dev_info = NULL;
    struct devdrv_device_health_status para;

    ret = copy_from_user_safe(&para, (void *)(uintptr_t)arg, sizeof(struct devdrv_device_health_status));
    if (ret != 0) {
        devdrv_drv_err("copy from user failed, ret(%d).\n", ret);
        return -EINVAL;
    }

    ret = devdrv_manager_container_logical_id_to_physical_id(para.device_id, &phys_id, &vfid);
    if (ret != 0) {
        devdrv_drv_err("can't transfor virt id %u \n", para.device_id);
        return -EFAULT;
    }

    dev_info = devdrv_manager_get_devdrv_info(phys_id);
    ret = devdrv_try_get_dev_info_occupy(dev_info);
    if (ret != 0) {
        devdrv_drv_err("Get dev_info occupy failed. (ret=%d; devid=%u)\n", ret, phys_id);
        return ret;
    }

    if (devdrv_manager_shm_info_check(dev_info)) {
        devdrv_drv_err("devid(%u) shm info check fail.\n", phys_id);
        devdrv_put_dev_info_occupy(dev_info);
        return -EFAULT;
    }

    para.device_health_status = (unsigned int)dev_info->shm_status->health_status;
    devdrv_put_dev_info_occupy(dev_info);

    ret = copy_to_user_safe((void *)(uintptr_t)arg, &para, sizeof(struct devdrv_device_health_status));
    if (ret != 0) {
        devdrv_drv_err("copy to user failed, ret(%d)\n", ret);
        return -EINVAL;
    }

    return 0;
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

STATIC int devdrv_manager_get_group_para(struct devdrv_ioctl_info *ioctl_buf, struct get_ts_group_para *group_para,
                                         unsigned long arg)
{
    int ret;

    ret = copy_from_user_safe((void *)ioctl_buf, (void *)((uintptr_t)arg), sizeof(struct devdrv_ioctl_info));
    if (ret != 0) {
        devdrv_drv_err("copy from user failed %d\n", ret);
        return ret;
    }
    if ((ioctl_buf->input_len != sizeof(struct get_ts_group_para)) ||
        (ioctl_buf->input_len > DEVDRV_MANAGER_INFO_PAYLOAD_LEN)) {
        devdrv_drv_err("input_len %d is invalid should equal %ld, and less than %ld\n", ioctl_buf->input_len,
                       sizeof(struct get_ts_group_para), DEVDRV_MANAGER_INFO_PAYLOAD_LEN);
        return -EINVAL;
    }
    ret = copy_from_user_safe((void *)group_para, (void *)(ioctl_buf->input_buf), ioctl_buf->input_len);
    if (ret != 0) {
        devdrv_drv_err("copy from user failed %d\n", ret);
        return ret;
    }
    return 0;
}

STATIC int devdrv_manager_host_get_group_info(struct devdrv_manager_msg_info *dev_manager_msg_info,
    struct get_ts_group_para *group_para, struct devdrv_info *info)
{
    int ret;
    int out_len = 0;

    dev_manager_msg_info->header.msg_id = DEVDRV_MANAGER_CHAN_H2D_GET_TS_GROUP_INFO;
    dev_manager_msg_info->header.valid = DEVDRV_MANAGER_MSG_VALID;
    /* give a random value for checking result later */
    dev_manager_msg_info->header.result = (u16)DEVDRV_MANAGER_MSG_INVALID_RESULT;
    /* inform corresponding devid to device side */
    dev_manager_msg_info->header.dev_id = info->dev_id;

    ret = memcpy_s(dev_manager_msg_info->payload, DEVDRV_MANAGER_INFO_PAYLOAD_LEN,
                   group_para, sizeof(struct get_ts_group_para));
    if (ret != 0) {
        devdrv_drv_err("memcpy failed ret = %d\n", ret);
        return -EFAULT;
    }

    ret = devdrv_manager_send_msg(info, dev_manager_msg_info, &out_len);
    if (ret != 0) {
        devdrv_drv_err("send msg to device fail ret = %d\n", ret);
        return ret;
    }
    if (out_len != (DEVDRV_TS_GROUP_NUM * sizeof(struct ts_group_info) + sizeof(struct devdrv_manager_msg_head))) {
        devdrv_drv_err("receive response len %d is not equal = %ld\n", out_len,
                       DEVDRV_TS_GROUP_NUM * sizeof(struct ts_group_info));
        return -EINVAL;
    }
    if (dev_manager_msg_info->header.result != 0) {
        devdrv_drv_err("get response from host error ret = %d\n", dev_manager_msg_info->header.result);
        return dev_manager_msg_info->header.result;
    }
    return 0;
}

STATIC int devdrv_manager_get_ts_group_info(struct file *filep,
    unsigned int cmd, unsigned long arg)
{
    struct devdrv_ioctl_info ioctl_buf;
    struct get_ts_group_para group_para = {0};
    int ret;
    struct devdrv_manager_msg_info dev_manager_msg_info;
    struct devdrv_info *info = NULL;
    struct devdrv_manager_info *d_info = NULL;
    unsigned int phy_id = 0;
    unsigned int vfid = 0;

    ret = devdrv_manager_get_group_para(&ioctl_buf, &group_para, arg);
    if (ret != 0) {
        devdrv_drv_err("get group para fail ret = %d\n", ret);
        return ret;
    }
    ret = devdrv_manager_container_logical_id_to_physical_id(group_para.device_id, &phy_id, &vfid);
    if (ret != 0) {
        devdrv_drv_err("can't get phys device id. virt id is %u, ret = %d\n", group_para.device_id, ret);
        return -EINVAL;
    }

    d_info = devdrv_get_manager_info();
    if (d_info == NULL) {
        devdrv_drv_err("info is NULL! the wrong dev_id is null\n");
        return -EINVAL;
    }
    if (phy_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("group_para phy device_id %d must less than %d\n", phy_id, DEVDRV_MAX_DAVINCI_NUM);
        return -EINVAL;
    }
    info = d_info->dev_info[phy_id];
    if (info == NULL) {
        devdrv_drv_err("info is NULL! the wrong vir device id = %d, phy dev_id is %u\n",
                       group_para.device_id, phy_id);
        return -EINVAL;
    }
    ret = devdrv_manager_host_get_group_info(&dev_manager_msg_info, &group_para, info);
    if (ret != 0) {
        devdrv_drv_err("host get ts group info fail %u\n", ret);
        return ret;
    }
    if (ioctl_buf.out_len > DEVDRV_MANAGER_INFO_PAYLOAD_LEN) {
        devdrv_drv_err("out len %d is invalid should less than %ld\n", ioctl_buf.out_len,
                       DEVDRV_MANAGER_INFO_PAYLOAD_LEN);
        return -EINVAL;
    }
    if (copy_to_user_safe((void *)(ioctl_buf.out_buf), (void *)dev_manager_msg_info.payload, ioctl_buf.out_len)) {
        devdrv_drv_err("copy to user failed.\n");
        return -EFAULT;
    }
    return 0;
}

/* This interface does not support using in containers */
STATIC int devdrv_manager_black_box_get_exception(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_black_box_user *black_box_user = NULL;
    unsigned long flags;
    int ret, index;

    ret = devdrv_manager_container_is_in_container();
    if (ret) {
        devdrv_drv_err("not support using in container, ret(%d).\n", ret);
        return -EINVAL;
    }

    if (dev_manager_info == NULL) {
        devdrv_drv_err("device does not exist.\n");
        return -EINVAL;
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
        devdrv_drv_info("interrupted. ret(%d)\n", ret);
        return 0;
    }
    if (ret) {
        devdrv_drv_err("down_interrupitable fail. ret(%d)\n", ret);
        return ret;
    }

no_wait_black_box_sema:
    black_box_user = (struct devdrv_black_box_user *)kzalloc(sizeof(struct devdrv_black_box_user),
        GFP_KERNEL | __GFP_ACCOUNT);
    if (black_box_user == NULL) {
        devdrv_drv_err("Allocate memory for black box failed.\n");
        return -ENOMEM;
    }

    devdrv_host_black_box_get_exception(black_box_user, index);
    ret = copy_to_user_safe((void *)((uintptr_t)arg), black_box_user, sizeof(struct devdrv_black_box_user));
    if (ret) {
        devdrv_drv_err("copy_to_user_safe fail.\n");
        kfree(black_box_user);
        black_box_user = NULL;
        return ret;
    }

    kfree(black_box_user);
    black_box_user = NULL;
    return 0;
}

STATIC int devdrv_manager_check_black_box_info(struct devdrv_black_box_user *black_box_user, unsigned int check_size)
{
    if (black_box_user->devid >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("Invalid device id. (dev_id=%u)\n", black_box_user->devid);
        return -EINVAL;
    }

    if (!devdrv_manager_is_pf_device(black_box_user->devid)) {
        devdrv_drv_err("Not in physical env. (dev_id=%u)\n", black_box_user->devid);
        return -EOPNOTSUPP;
    }

    if (black_box_user->dst_buffer == NULL) {
        devdrv_drv_err("Invalid buffer. (dev_id=%u)\n", black_box_user->devid);
        return -EFAULT;
    }

    if ((black_box_user->size <= 0) || (black_box_user->size > check_size)) {
        devdrv_drv_err("Invalid size. (dev_id=%u)\n\n", black_box_user->devid);
        return -EINVAL;
    }

    return 0;
}

STATIC int devdrv_manager_device_memory_dump(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_black_box_user *black_box_user = NULL;
    struct devdrv_info *dev_info = NULL;
    dma_addr_t host_addr_dma = 0;
    void *buffer = NULL;
    int dev_id;
    int ret;

    if (devdrv_manager_container_is_in_container()) {
        devdrv_drv_err("not support using in container.\n");
        return -EINVAL;
    }

    black_box_user = (struct devdrv_black_box_user *)kzalloc(sizeof(struct devdrv_black_box_user),
        GFP_KERNEL | __GFP_ACCOUNT);
    if (black_box_user == NULL) {
        devdrv_drv_err("Allocate memory for black box failed.\n");
        return -ENOMEM;
    }

    ret = copy_from_user_safe(black_box_user, (void *)((uintptr_t)arg), sizeof(struct devdrv_black_box_user));
    if (ret) {
        devdrv_drv_err("copy_from_user_safe fail, ret(%d).\n", ret);
        goto free_black_box_exit;
    }

    ret = devdrv_manager_check_black_box_info(black_box_user, DEVDRV_MAX_MEMORY_DUMP_SIZE);
    if (ret != 0) {
        devdrv_drv_err("Check black box parameter fail. (dev_id=%u)\n", black_box_user->devid);
        goto free_black_box_exit;
    }

    dev_info = devdrv_get_devdrv_info_array(black_box_user->devid);
    if (dev_info == NULL) {
        devdrv_drv_err("no device(%u).\n", black_box_user->devid);
        ret = -ENODEV;
        goto free_black_box_exit;
    }

    if ((black_box_user->addr_offset >= dev_info->dump_ddr_size) ||
        (black_box_user->addr_offset + black_box_user->size > dev_info->dump_ddr_size)) {
        devdrv_drv_err("invalid phy offset addr. dev_id(%u), size(%u), info size(%u)\n",
                       black_box_user->devid, black_box_user->size, dev_info->dump_ddr_size);
        ret = -EFAULT;
        goto free_black_box_exit;
    }

    dev_id = devdrv_get_device_index(black_box_user->devid);
    if (dev_id < 0 || dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("get device(%u) index(%d) fail.\n", black_box_user->devid, dev_id);
        ret = -EINVAL;
        goto free_black_box_exit;
    }

    black_box_user->addr_offset += dev_info->dump_ddr_dma_addr;

    atomic_inc(&dev_info->occupy_ref);
    if (dev_info->status == DEVINFO_STATUS_REMOVED) {
        atomic_dec(&dev_info->occupy_ref);
        devdrv_drv_warn("dev %d has been reset\n", dev_info->dev_id);
        ret = -EINVAL;
        goto free_black_box_exit;
    }

    buffer = devdrv_dma_alloc_coherent(dev_info->dev, black_box_user->size, &host_addr_dma, GFP_KERNEL | __GFP_ACCOUNT);
    if (buffer == NULL) {
        atomic_dec(&dev_info->occupy_ref);
        devdrv_drv_err("dma_alloc_coherent fail devid(%u), size(%d).\n", black_box_user->devid, black_box_user->size);
        ret = -ENOMEM;
        goto free_black_box_exit;
    }
    devdrv_drv_debug("devid: %u, len: %d.\n", black_box_user->devid, black_box_user->size);

    ret = devdrv_dma_sync_copy(dev_info->pci_dev_id, DEVDRV_DMA_DATA_COMMON, (u64)black_box_user->addr_offset,
                               (u64)host_addr_dma, black_box_user->size, DEVDRV_DMA_DEVICE_TO_HOST);
    if (ret) {
        devdrv_drv_err("devdrv_dma_sync_copy fail, ret(%d). dev_id(%u)\n", ret, black_box_user->devid);
        ret = -1;
        goto free_alloc;
    }

    ret = copy_to_user_safe(black_box_user->dst_buffer, buffer, black_box_user->size);
    if (ret) {
        devdrv_drv_err("copy_to_user_safe fail, ret(%d). dev_id(%u)\n", ret, black_box_user->devid);
        ret = -1;
        goto free_alloc;
    }

free_alloc:
    atomic_dec(&dev_info->occupy_ref);
    devdrv_dma_free_coherent(dev_info->dev, black_box_user->size, buffer, host_addr_dma);
    buffer = NULL;
free_black_box_exit:
    kfree(black_box_user);
    black_box_user = NULL;
    return ret;
}

STATIC struct devdrv_ts_log *devdrv_get_tslog_info(u32 dev_id)
{
    if (dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("Invalid parameter. (device id=%u)\n", dev_id);
        return NULL;
    }

    return g_devdrv_ts_log_array[dev_id];
}

STATIC int devdrv_manager_tslog_dump_process(struct devdrv_black_box_user *black_box_user, void **buffer)
{
    int ret = 0;
    dma_addr_t host_addr_dma = 0;
    struct devdrv_info *dev_info = NULL;

    dev_info = devdrv_get_devdrv_info_array(black_box_user->devid);
    if (dev_info == NULL) {
        devdrv_drv_err("Device is nonexistent. (device id=%u)\n", black_box_user->devid);
        return -ENODEV;
    }

    atomic_inc(&dev_info->occupy_ref);
    if (dev_info->status == DEVINFO_STATUS_REMOVED) {
        devdrv_drv_warn("Devcie has been reseted. (device id=%u)\n", dev_info->dev_id);
        ret = -ENODEV;
        goto FLAG_DEC;
    }

    *buffer = devdrv_dma_alloc_coherent(dev_info->dev, black_box_user->size,
                                        &host_addr_dma, GFP_KERNEL | __GFP_ACCOUNT);
    if (*buffer == NULL) {
        devdrv_drv_err("Dma alloc coherent failed. (device id=%u; size=%d)\n",
            black_box_user->devid, black_box_user->size);
        ret = -ENOMEM;
        goto FLAG_DEC;
    }

    ret = devdrv_dma_sync_copy(dev_info->pci_dev_id, DEVDRV_DMA_DATA_COMMON, (u64)black_box_user->addr_offset,
                               (u64)host_addr_dma, black_box_user->size, DEVDRV_DMA_DEVICE_TO_HOST);
    if (ret) {
        devdrv_drv_err("Dma sync copy failed. (ret=%d; device id=%u)\n", ret, black_box_user->devid);
        goto DMA_FREE;
    }

    ret = copy_to_user_safe(black_box_user->dst_buffer, *buffer, black_box_user->size);
    if (ret) {
        devdrv_drv_err("Copy to user failed. (ret=%d; device id=%u)\n", ret, black_box_user->devid);
    }

DMA_FREE:
    devdrv_dma_free_coherent(dev_info->dev, black_box_user->size, *buffer, host_addr_dma);
FLAG_DEC:
    atomic_dec(&dev_info->occupy_ref);
    return ret;
}

STATIC int devdrv_manager_tslog_dump(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    void *buffer = NULL;
    struct devdrv_ts_log *ts_log = NULL;
    struct devdrv_black_box_user *black_box_user = NULL;

    if (devdrv_manager_container_is_in_container()) {
        devdrv_drv_err("Not support using in container.\n");
        return -EOPNOTSUPP;
    }

    black_box_user = (struct devdrv_black_box_user *)kzalloc(sizeof(struct devdrv_black_box_user),
        GFP_KERNEL | __GFP_ACCOUNT);
    if (black_box_user == NULL) {
        devdrv_drv_err("Allocate memory for black box failed.\n");
        return -ENOMEM;
    }

    ret = copy_from_user_safe(black_box_user, (void *)((uintptr_t)arg), sizeof(struct devdrv_black_box_user));
    if (ret) {
        devdrv_drv_err("Copy from user failed. (ret=%d).\n", ret);
        goto FREE_BBOX_EXIT;
    }

    if (!devdrv_manager_is_pf_device(black_box_user->devid)) {
        devdrv_drv_err("Virtual device query is not supported. (dev_id=%u)\n", black_box_user->devid);
        ret = -EOPNOTSUPP;
        goto FREE_BBOX_EXIT;
    }

    ts_log = devdrv_get_tslog_info(black_box_user->devid);
    if (ts_log == NULL || ts_log->mem_size == 0) {
        devdrv_drv_err("Ts log dma addr info is not refreshed by device yet. (device id=%u).\n", black_box_user->devid);
        ret = -ENODEV;
        goto FREE_BBOX_EXIT;
    }

    if ((ts_log->mem_size > DEVMNG_TSLOG_MAX_SIZE) || (black_box_user->addr_offset >= ts_log->mem_size) ||
        (black_box_user->addr_offset + black_box_user->size > ts_log->mem_size)) {
        devdrv_drv_err("Invalid user input parameter. (dev_id=%u; addr offset=%llu; size=%u; info size=%u)\n",
                       black_box_user->devid, black_box_user->addr_offset, black_box_user->size, ts_log->mem_size);
        ret = -EFAULT;
        goto FREE_BBOX_EXIT;
    }

    black_box_user->addr_offset += ts_log->dma_addr;

    ret = devdrv_manager_tslog_dump_process(black_box_user, &buffer);
    if (ret) {
        devdrv_drv_err("Ts log dump failed. (ret=%d; device id=%u)\n", ret, black_box_user->devid);
    }

FREE_BBOX_EXIT:
    kfree(black_box_user);
    black_box_user = NULL;
    buffer = NULL;
    return ret;
}


STATIC int devdrv_manager_reg_ddr_read(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_black_box_user *black_box_user = NULL;
    struct devdrv_info *dev_info = NULL;
    dma_addr_t host_addr_dma = 0;
    void *buffer = NULL;
    int dev_id;
    int ret;

    if (devdrv_manager_container_is_in_container()) {
        devdrv_drv_err("not support using in container.\n");
        return -EINVAL;
    }

    black_box_user = (struct devdrv_black_box_user *)kzalloc(sizeof(struct devdrv_black_box_user),
        GFP_KERNEL | __GFP_ACCOUNT);
    if (black_box_user == NULL) {
        devdrv_drv_err("Allocate memory for black box failed.\n");
        return -ENOMEM;
    }

    ret = copy_from_user_safe(black_box_user, (void *)((uintptr_t)arg), sizeof(struct devdrv_black_box_user));
    if (ret) {
        devdrv_drv_err("copy_from_user_safe fail, ret(%d).\n", ret);
        goto free_black_box_exit;
    }

    ret = devdrv_manager_check_black_box_info(black_box_user, DEVDRV_MAX_REG_DDR_READ_SIZE);
    if (ret != 0) {
        devdrv_drv_err("Check black box parameter fail. (dev_id=%u)\n", black_box_user->devid);
        goto free_black_box_exit;
    }

    ret = devdrv_manager_check_capability(black_box_user->devid, DEVDRV_CAP_IMU_REG_EXPORT);
    if (ret) {
        devdrv_drv_err("do not support read reg ddr. dev_id(%u)\n", black_box_user->devid);
        ret = -EINVAL;
        goto free_black_box_exit;
    }

    dev_info = devdrv_get_devdrv_info_array(black_box_user->devid);
    if (dev_info == NULL) {
        devdrv_drv_err("no device(%u).\n", black_box_user->devid);
        ret = -ENODEV;
        goto free_black_box_exit;
    }

    if ((black_box_user->addr_offset >= dev_info->reg_ddr_size) ||
        (black_box_user->addr_offset + black_box_user->size > dev_info->reg_ddr_size)) {
        devdrv_drv_err("invalid phy offset addr. dev_id(%u), size(%u), info size(%u)\n",
                       black_box_user->devid, black_box_user->size, dev_info->reg_ddr_size);
        ret = -EFAULT;
        goto free_black_box_exit;
    }

    dev_id = devdrv_get_device_index(black_box_user->devid);
    if (dev_id < 0 || dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("get device(%u) index(%d) fail.\n", black_box_user->devid, dev_id);
        ret = -EINVAL;
        goto free_black_box_exit;
    }

    black_box_user->addr_offset += dev_info->reg_ddr_dma_addr;

    atomic_inc(&dev_info->occupy_ref);
    if (dev_info->status == DEVINFO_STATUS_REMOVED) {
        atomic_dec(&dev_info->occupy_ref);
        devdrv_drv_warn("dev %d has been reset\n", dev_info->dev_id);
        ret = -EINVAL;
        goto free_black_box_exit;
    }

    buffer = devdrv_dma_alloc_coherent(dev_info->dev, black_box_user->size, &host_addr_dma, GFP_KERNEL | __GFP_ACCOUNT);
    if (buffer == NULL) {
        atomic_dec(&dev_info->occupy_ref);
        devdrv_drv_err("dma_alloc_coherent fail devid(%u), size(%d).\n", black_box_user->devid, black_box_user->size);
        ret = -ENOMEM;
        goto free_black_box_exit;
    }
    devdrv_drv_debug("devid: %u, len: %d.\n", black_box_user->devid, black_box_user->size);

    ret = devdrv_dma_sync_copy(dev_info->pci_dev_id, DEVDRV_DMA_DATA_COMMON, (u64)black_box_user->addr_offset,
                               (u64)host_addr_dma, black_box_user->size, DEVDRV_DMA_DEVICE_TO_HOST);
    if (ret) {
        devdrv_drv_err("devdrv_dma_sync_copy fail, ret(%d). dev_id(%u)\n", ret, black_box_user->devid);
        ret = -1;
        goto free_alloc;
    }

    ret = copy_to_user_safe(black_box_user->dst_buffer, buffer, black_box_user->size);
    if (ret) {
        devdrv_drv_err("copy_to_user_safe fail, ret(%d). dev_id(%u)\n", ret, black_box_user->devid);
        ret = -1;
        goto free_alloc;
    }

free_alloc:
    atomic_dec(&dev_info->occupy_ref);
    devdrv_dma_free_coherent(dev_info->dev, black_box_user->size, buffer, host_addr_dma);
    buffer = NULL;
free_black_box_exit:
    kfree(black_box_user);
    black_box_user = NULL;
    return ret;
}

int devdrv_fop_bind_host_pid(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_ioctl_para_bind_host_pid para_info;
    int node_id = numa_node_id();
    unsigned int phy_id = 0;
    unsigned int vfid = 0;
    int ret;

    if (copy_from_user_safe(&para_info, (void *)(uintptr_t)arg, sizeof(struct devdrv_ioctl_para_bind_host_pid))) {
        devdrv_drv_err("copy_from_user error. dev_id:%u\n", node_id);
        return -EINVAL;
    }

#ifndef DEVDRV_MANAGER_HOST_UT_TEST
    if (para_info.chip_id < PID_MAP_DEVNUM) {
        ret = devdrv_manager_container_logical_id_to_physical_id(para_info.chip_id, &phy_id, &vfid);
        if (ret != 0) {
            devdrv_drv_err("Transfer logical id to physical id failed. (ret=%d)\n", ret);
            return ret;
        }

        if (!devdrv_manager_is_pf_device(phy_id) || (vfid > 0)) {
            devdrv_drv_err("Not support split mode. (phy_id=%u; vfid=%u)\n", phy_id, vfid);
            return -EPERM;
        }
        para_info.chip_id = phy_id;
    }

    para_info.sign[DEVDRV_SIGN_LEN - 1] = '\0';

    if ((para_info.len != PROCESS_SIGN_LENGTH) ||
        (para_info.mode >= AICPUFW_MAX_PLAT) || (para_info.cp_type < 0) ||
        (para_info.cp_type >= DEVDRV_PROCESS_CPTYPE_MAX) ||
        ((para_info.cp_type != DEVDRV_PROCESS_USER) && (para_info.chip_id >= PID_MAP_DEVNUM)) ||
        (para_info.vfid >= VFID_NUM_MAX)) {
        devdrv_drv_err("invalid para length(%u) cp_type(%d) chip_id(%u) hostpid(%d) vfid(%u).\n",
            para_info.len, para_info.cp_type, para_info.chip_id, para_info.host_pid, para_info.vfid);
        return -EINVAL;
    }
#endif

    ret = devdrv_bind_hostpid(para_info);
    if (ret) {
        devdrv_drv_err("bind_hostpid error. dev_id:%u, ret:%d, host_pid:%d, cp_type:%d, current_pid:%d\n",
            node_id, ret, para_info.host_pid, para_info.cp_type, current->tgid);
        return ret;
    }

    return 0;
}

#ifdef CFG_FEATURE_OLD_DEVID_TRANS
STATIC int devdrv_manager_ioctl_get_devnum(struct file *filep, unsigned int cmd, unsigned long arg)
{
    u32 devnum;

    devnum = devdrv_manager_get_devnum();

    return copy_to_user_safe((void *)((uintptr_t)arg), &devnum, sizeof(u32));
}

STATIC int devdrv_manager_ioctl_get_vdevnum(struct file *filep, unsigned int cmd, unsigned long arg)
{
    u32 vdevnum;

    vdevnum = devdrv_manager_get_vdevnum();

    return copy_to_user_safe((void *)((uintptr_t)arg), &vdevnum, sizeof(u32));
}
#endif

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
    return copy_to_user_safe((void *)((uintptr_t)arg), &plat_info, sizeof(u32));
}

/* This interface does not support using in containers */
STATIC int devdrv_manager_device_reset_inform(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct timespec stamp;
    u32 devid;
    int virt_id;
    u32 vfid = 0;
    int ret;

    ret = copy_from_user_safe(&devid, (void *)((uintptr_t)arg), sizeof(u32));
    if (ret) {
        devdrv_drv_err("copy_from_user_safe failed, ret(%d).\n", ret);
        return ret;
    }

    virt_id = devid;
    if (devdrv_manager_container_logical_id_to_physical_id(virt_id, &devid, &vfid)) {
        devdrv_drv_err("can't transfor virt id %u \n", virt_id);
        return -EINVAL;
    }

    if (vfid != 0) {
        devdrv_drv_err("Not support for vdevice. (vfid=%u)\n", vfid);
        return -EOPNOTSUPP;
    }

#ifndef CFG_FEATURE_SRIOV
    ret = devdrv_manager_check_permission();
    if (ret != 0) {
        devdrv_drv_err("Failed to invoke devdrv_manager_check_permission. (ret=%d)\n", ret);
    #ifdef CFG_FEATURE_ERRORCODE_ON_NEW_CHIPS
        return ret;
    #else
        return -EINVAL;
    #endif
    }
#endif
    stamp = current_kernel_time();

    ret = devdrv_host_black_box_add_exception(devid, DEVDRV_BB_DEVICE_RESET_INFORM, stamp, NULL);
    if (ret) {
        devdrv_drv_err("devdrv_host_black_box_add_exception failed, ret(%d). dev_id(%u)\n", ret, devid);
        return ret;
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

STATIC int devdrv_manager_get_container_flag(struct file *filep, unsigned int cmd, unsigned long arg)
{
    unsigned int flag;
    int ret;

    flag = (unsigned int)devdrv_manager_container_is_in_container();

    ret = copy_to_user_safe((void *)((uintptr_t)arg), &flag, sizeof(unsigned int));

    return ret;
}

STATIC int devdrv_manager_container_cmd(struct file *filep, unsigned int cmd, unsigned long arg)
{
    return devdrv_manager_container_process(filep, arg);
}

u32 devdrv_manager_get_probe_num_kernel(void)
{
    u32 probe_num;
    int ret;
    unsigned long flags;

    ret = devdrv_manager_container_is_in_container();
    if (ret != 0) { // if in container, num is the number of device in container
        probe_num = devdrv_manager_get_devnum();
    } else {
        spin_lock_irqsave(&dev_manager_info->spinlock, flags);
        probe_num = dev_manager_info->prob_num;
        spin_unlock_irqrestore(&dev_manager_info->spinlock, flags);
    }

    return probe_num;
}

STATIC int devdrv_manager_get_probe_num(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    u32 num;

    num = devdrv_manager_get_probe_num_kernel();
    ret = copy_to_user_safe((void *)((uintptr_t)arg), &num, sizeof(u32));
    if (ret) {
        devdrv_drv_err("copy to user failed, ret(%d).\n", ret);
        return ret;
    }

    return 0;
}

STATIC int devdrv_get_error_code(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_error_code_para user_arg = { 0, { 0 }, 0 };
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
    ret = devdrv_try_get_dev_info_occupy(dev_info);
    if (ret != 0) {
        devdrv_drv_err("Get dev_info occupy failed. (ret=%d; devid=%u)\n", ret, phys_id);
        return ret;
    }

    if (devdrv_manager_shm_info_check(dev_info)) {
        devdrv_drv_err("devid(%u) shm info check fail.\n", phys_id);
        devdrv_put_dev_info_occupy(dev_info);
        return -EFAULT;
    }

    user_arg.error_code_count = dev_info->shm_status->error_cnt;
    for (i = 0; i < DEVMNG_SHM_INFO_ERROR_CODE_LEN; i++) {
        user_arg.error_code[i] = dev_info->shm_status->error_code[i];
    }
    devdrv_put_dev_info_occupy(dev_info);

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
    int i, cnt, ret;

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
    ret = devdrv_try_get_dev_info_occupy(dev_info);
    if (ret != 0) {
        devdrv_drv_err("Get dev_info occupy failed. (ret=%d; devid=%u)\n", ret, devid);
        return ret;
    }

    if (devdrv_manager_shm_info_check(dev_info)) {
        devdrv_drv_err("The dev_info is invalid. (devid=%u)\n", devid);
        devdrv_put_dev_info_occupy(dev_info);
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
    devdrv_put_dev_info_occupy(dev_info);

    return 0;
}

int devmng_dms_get_health_code(u32 devid, u32 *health_code, u32 health_len)
{
    struct devdrv_info *dev_info = NULL;
    int i, ret;

    if ((devid >= DEVDRV_MAX_DAVINCI_NUM) || (health_code == NULL) ||
        (health_len != VMNG_VDEV_MAX_PER_PDEV)) {
        devdrv_drv_err("Invalid parameter. (devid=%u; health_code=\"%s\"; health_len=%u)\n",
                       devid, (health_code == NULL) ? "NULL" : "OK", health_len);
        return -EINVAL;
    }

    dev_info = devdrv_manager_get_devdrv_info(devid);
    ret = devdrv_try_get_dev_info_occupy(dev_info);
    if (ret != 0) {
        devdrv_drv_err("Get dev_info occupy failed. (ret=%d; devid=%u)\n", ret, devid);
        return ret;
    }

    if (devdrv_manager_shm_info_check(dev_info)) {
        devdrv_drv_err("The dev_info is invalid. (devid=%u)\n", devid);
        devdrv_put_dev_info_occupy(dev_info);
        return -EFAULT;
    }

    for (i = 0; i < VMNG_VDEV_MAX_PER_PDEV; i++) {
        health_code[i] = dev_info->shm_status->dms_health_status[i];
    }
    devdrv_put_dev_info_occupy(dev_info);

    return 0;
}

#ifdef CFG_FEATURE_CHIP_DIE
int devdrv_manager_get_random_from_bar(u32 devid, char *random_number, u32 random_len)
{
    struct devdrv_info *dev_info = NULL;
    u32 phys_id, vfid;
    int ret;

    if ((random_len < DEVMNG_SHM_INFO_RANDOM_SIZE) || (random_number == NULL)) {
        devdrv_drv_err("Invalid parameter. (random_len=%u; random_number=\"%s\")\n",
                       random_len, (random_number == NULL) ? "NULL" : "OK");
        return -EINVAL;
    }

    if (devdrv_manager_container_logical_id_to_physical_id(devid, &phys_id, &vfid)) {
        devdrv_drv_err("Logical id transfor physical id failed. (devid=%u; phys_id=%u)\n", devid, phys_id);
        return -ENODEV;
    }

    dev_info = devdrv_manager_get_devdrv_info(phys_id);
    ret = devdrv_try_get_dev_info_occupy(dev_info);
    if (ret != 0) {
        devdrv_drv_err("Get dev_info occupy failed. (ret=%d; phys_id=%u)\n", ret, phys_id);
        return ret;
    }

    if (devdrv_manager_shm_info_check(dev_info)) {
        devdrv_drv_warn("The dev_info is invalid. (devid=%u; phys_id=%u)\n", devid, phys_id);
        ret = memset_s(random_number, random_len, 0, DEVMNG_SHM_INFO_RANDOM_SIZE);
        if (ret != 0) {
            devdrv_drv_err("Memset for random number failed. (dev_id=%u; ret=%d)\n", devid, ret);
            devdrv_put_dev_info_occupy(dev_info);
            return -ENOMEM;
        }

        /*
        * if one device pcie bar space has exception,
        * return device id to ramdom number to return this device as single chip
        */
        random_number[0] = devid;
        devdrv_put_dev_info_occupy(dev_info);
        return 0;
    }

    ret = memcpy_s(random_number, random_len, dev_info->shm_status->random_number, DEVMNG_SHM_INFO_RANDOM_SIZE);
    if (ret) {
        devdrv_drv_err("Memcpy random from shm failed. (devid=%d)\n", devid);
        devdrv_put_dev_info_occupy(dev_info);
        return ret;
    }
    devdrv_put_dev_info_occupy(dev_info);

    return 0;
}
#endif

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

STATIC int devdrv_get_process_sign(struct devdrv_manager_info *d_info, char *sign, u32 len, u32 docker_id)
{
    struct devdrv_process_sign *d_sign = NULL;
    struct list_head *pos = NULL;
    struct list_head *n = NULL;
    int ret;

    if (!list_empty_careful(&d_info->hostpid_list_header)) {
        list_for_each_safe(pos, n, &d_info->hostpid_list_header) {
            d_sign = list_entry(pos, struct devdrv_process_sign, list);
            if (d_sign->hostpid == current->tgid) {
                ret = strcpy_s(sign, len, d_sign->sign);
                if (ret) {
                    devdrv_drv_err("Copy hostpid sign failed. (docker_id=%u; hostpid=%d; ret=%d)\n",
                        docker_id, d_sign->hostpid, ret);
                    return -EINVAL;
                }

                return 0;
            }
        }
    }

    if (d_info->devdrv_sign_count[docker_id] >= DEVDRV_MAX_SIGN_NUM) {
        devdrv_drv_err("List add failed. (docker_id=%u).\n", docker_id);
        return -EINVAL;
    }

    d_sign = ka_vmalloc(sizeof(struct devdrv_process_sign), GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT, PAGE_KERNEL);
    if (d_sign == NULL) {
        devdrv_drv_err("vzalloc failed. (docker_id=%u)\n", docker_id);
        return -ENOMEM;
    }
    d_sign->hostpid = current->tgid;
    d_sign->docker_id = docker_id;

    ret = devdrv_creat_random_sign(d_sign->sign, PROCESS_SIGN_LENGTH);
    if (ret) {
        devdrv_drv_err("Get sign failed. (ret=%d; docker_id=%u)\n", ret, docker_id);
        vfree(d_sign);
        d_sign = NULL;
        return ret;
    }
    ret = strcpy_s(sign, len, d_sign->sign);
    if (ret) {
        devdrv_drv_err("strcpy_s failed. (ret=%d; docker_id=%u)\n", ret, docker_id);
        vfree(d_sign);
        d_sign = NULL;
        return -EINVAL;
    }

    list_add(&d_sign->list, &d_info->hostpid_list_header);
    d_info->devdrv_sign_count[d_sign->docker_id]++;
    devdrv_drv_info("Process sign list add success. (docker_id=%u; hostpid=%d)\n", docker_id, d_sign->hostpid);
    return 0;
}

STATIC int devdrv_manager_get_process_sign(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_manager_info *d_info = devdrv_get_manager_info();
    u32 docker_id = MAX_DOCKER_NUM;
    struct process_sign dev_sign = {0};
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
    ret = devdrv_manager_container_is_in_container();
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

STATIC int devdrv_host_query_process_by_host_pid(struct devdrv_ioctl_para_query_pid *para_info,
    struct devdrv_info *info)
{
    int ret;
    int out_len = 0;
    pid_t host_tgid = -1;
    struct devdrv_manager_msg_info dev_manager_msg_info = {{0}, {0}};
    struct devdrv_ioctl_para_query_pid *para_info_tmp;

    if (para_info == NULL || info == NULL) {
        devdrv_drv_err("para_info or info is NULL!.\n");
        return -EINVAL;
    }

    if (devdrv_get_tgid_by_pid(para_info->host_pid, &host_tgid) != 0) {
        devdrv_drv_err("Failed to get tgid by pid. (pid=%d)\n", para_info->host_pid);
        return -EINVAL;
    }
    para_info->host_pid = host_tgid;

#ifndef DEVDRV_MANAGER_HOST_UT_TEST
    if ((para_info->cp_type == DEVDRV_PROCESS_CP1) && (para_info->vfid == 0)) { /* not surport host cp query */
        /* host has cp, store device cp in dev_only */
        ret = devdrv_query_process_by_host_pid(para_info->host_pid, info->dev_id, DEVDRV_PROCESS_DEV_ONLY,
            para_info->vfid, &para_info->pid);
        if (ret == 0) {
            return 0;
        }
    }
#endif

    dev_manager_msg_info.header.dev_id = info->dev_id;
    dev_manager_msg_info.header.msg_id = DEVDRV_MANAGER_CHAN_H2D_QUERY_DEVICE_PID;
    dev_manager_msg_info.header.valid = DEVDRV_MANAGER_MSG_VALID;
    /* give a random value for checking result later */
    dev_manager_msg_info.header.result = (u16)DEVDRV_MANAGER_MSG_INVALID_RESULT;
    /* inform corresponding devid to device side */

    para_info_tmp = (struct devdrv_ioctl_para_query_pid *)dev_manager_msg_info.payload;
    para_info_tmp->cp_type = para_info->cp_type;
    para_info_tmp->host_pid = para_info->host_pid;
    para_info_tmp->vfid = para_info->vfid;
    para_info_tmp->chip_id = para_info->chip_id;

    ret = devdrv_manager_send_msg(info, &dev_manager_msg_info, &out_len);
    if (ret != 0) {
        devdrv_drv_warn("send msg to device fail ret = %d.\n", ret);
        return ret;
    }
    if (out_len != (sizeof(struct devdrv_ioctl_para_query_pid) + sizeof(struct devdrv_manager_msg_head))) {
        devdrv_drv_warn("receive response len %d is not equal = %ld.\n", out_len,
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
    struct devdrv_ioctl_para_query_pid para_info;
    struct devdrv_info *info = NULL;
    unsigned int phy_id = 0;
    unsigned int vfid = 0;

    if (copy_from_user_safe(&para_info, (void *)(uintptr_t)arg, sizeof(struct devdrv_ioctl_para_query_pid))) {
        devdrv_drv_err("[devdrv_query_devpid] copy_from_user error\n");
        return -EINVAL;
    }

    ret = devdrv_manager_container_logical_id_to_physical_id(para_info.chip_id, &phy_id, &vfid);
    if (ret != 0) {
        devdrv_drv_err("can't get phys device id. virt id is %u, ret = %d.\n", para_info.chip_id, ret);
        return -EINVAL;
    }

    info = devdrv_manager_get_devdrv_info(phy_id);
    if (info == NULL) {
        devdrv_drv_err("info is NULL!.\n");
        return -EINVAL;
    }

    ret = dms_hotreset_task_cnt_increase(phy_id);
    if (ret != 0) {
        devdrv_drv_err("Hotreset task cnt increase failed. (dev_id=%u; ret=%d)\n", phy_id, ret);
        return ret;
    }

    ret = devdrv_host_query_process_by_host_pid(&para_info, info);
    if (ret) {
#ifndef DEVDRV_MANAGER_HOST_UT_TEST
        devdrv_drv_warn("Can not query device_pid by host_pid. (ret=%d; host_pid=%u; cp_type=%u)\n",
            ret, para_info.host_pid, para_info.cp_type);
#endif
        goto out;
    }

    if (copy_to_user_safe((void *)((uintptr_t)arg), &para_info, sizeof(struct devdrv_ioctl_para_query_pid))) {
        devdrv_drv_err("[devdrv_query_devpid] copy_to_user error\n");
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
#ifndef DEVDRV_MANAGER_HOST_UT_TEST
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
#endif
}

#ifdef CFG_FEATURE_NOTIFY_REBOOT
STATIC int devdrv_host_notice_reboot(u32 phy_id)
{
    struct devdrv_manager_msg_info dev_manager_msg_info = {{0}, {0}};
    struct devdrv_info *info = NULL;
    int out_len = 0;
    int ret;

    info = devdrv_manager_get_devdrv_info(phy_id);
    if (info == NULL) {
        devdrv_drv_err("Get devinfo is null. (phy_id=%u)\n", phy_id);
        return -EAGAIN;
    }
    dev_manager_msg_info.header.msg_id = DEVDRV_MANAGER_CHAN_H2D_NOTICE_REBOOT;
    dev_manager_msg_info.header.valid = DEVDRV_MANAGER_MSG_VALID;
    dev_manager_msg_info.header.result = (u16)DEVDRV_MANAGER_MSG_INVALID_RESULT;
    dev_manager_msg_info.header.dev_id = info->dev_id;

    ret = devdrv_manager_send_msg(info, &dev_manager_msg_info, &out_len);
    if (ret != 0) {
        devdrv_drv_err("send msg fail. (ret=%d)\n", ret);
        return -EAGAIN;
    }
    if (out_len != sizeof(struct devdrv_manager_msg_head)) {
        devdrv_drv_err("send msg out_len invalid. (out_len=%d)\n", out_len);
        return -EAGAIN;
    }
    if (dev_manager_msg_info.header.result != 0) {
        devdrv_drv_err("send msg header result fail. (result=%u)\n", dev_manager_msg_info.header.result);
        return -EAGAIN;
    }
    return 0;
}
STATIC void devdrv_notify_all_dev_reboot(void)
{
    unsigned int dev_num;
    unsigned int phys_id = 0;
    unsigned int vfid = 0;
    unsigned int i;
    int ret;

    dev_num = devdrv_manager_get_devnum();
    if (dev_num > DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("Get invalid device num. (dev_num=%u)\n", dev_num);
        return;
    }
    for (i = 0; i < dev_num; ++i) {
        ret = devdrv_manager_container_logical_id_to_physical_id(i, &phys_id, &vfid);
        if (ret != 0) {
            devdrv_drv_err("Can't transfor virt id. (virt_id=%u; ret=%d)\n", i, ret);
            return;
        }
        ret = devdrv_host_notice_reboot(phys_id);
        if (ret != 0) {
            return;
        }
    }
}
#endif

#define MAX_NOTICE_DEV_EXIT_TIMES 1000
STATIC void devdrv_host_release_notice_dev(int host_pid)
{
    u32 did, try_time;
    int ret;

    for (did = 0, try_time = 0; did < DEVDRV_MAX_DAVINCI_NUM; did++) {
        do {
            ret = devdrv_host_notice_dev_process_exit(did, host_pid);
            if (ret != 0) {
                usleep_range(10, 20); /* 10-20 us */
                try_time++;
            }
        } while ((ret != 0) && (try_time < MAX_NOTICE_DEV_EXIT_TIMES));
    }
}

STATIC int devdrv_manager_get_dev_resource_info(struct devdrv_resource_info *dinfo)
{
    struct devdrv_manager_msg_resource_info info;
    u32 phy_id = 0;
    u32 vfid = 0;
    int ret;

    if ((dinfo->resource_type == DEVDRV_DEV_PROCESS_PID) || (dinfo->resource_type == DEVDRV_DEV_PROCESS_MEM)) {
        devdrv_drv_err("Invalid dev resource type. (dev_id=%u, type=%d)", dinfo->devid, dinfo->resource_type);
        return -EOPNOTSUPP;
    }

#ifndef CFG_FEATURE_DDR
    if ((dinfo->resource_type == DEVDRV_DEV_DDR_TOTAL) || (dinfo->resource_type == DEVDRV_DEV_DDR_FREE)) {
        devdrv_drv_err("Unsupport dev resource type. (dev_id=%u, type=%d)", dinfo->devid, dinfo->resource_type);
        return -EOPNOTSUPP;
    }
#endif

    ret = devdrv_manager_container_logical_id_to_physical_id(dinfo->devid, &phy_id, &vfid);
    if (ret) {
        devdrv_drv_err("logical_id_to_physical_id fail, devid(%u) ret(%d).\n", dinfo->devid, ret);
        return ret;
    }

    info.vfid = vfid;
    info.info_type = dinfo->resource_type;
    info.owner_id = dinfo->owner_id;
    ret = hvdevmng_get_dev_resource(phy_id, dinfo->tsid, &info);
    if (ret) {
        devdrv_drv_err("get resource info failed, devid(%u), resource type (%u) ret(%d).\n",
            dinfo->devid, dinfo->resource_type, ret);
        return ret;
    }
    *((u64 *)dinfo->buf) = info.value;

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

STATIC int (*dms_set_dev_info_handlers[DMS_DEV_INFO_TYPE_MAX])(u32 devid, const void *buf, u32 buf_size);

#ifndef DEVDRV_MANAGER_HOST_UT_TEST
int dms_register_set_dev_info_handler(DMS_DEV_INFO_TYPE type, dms_set_dev_info_ops func)
{
    if (func == NULL) {
        devdrv_drv_err("Register function of setting device infomation is null.\n");
        return -EINVAL;
    }

    if (type >= DMS_DEV_INFO_TYPE_MAX) {
        devdrv_drv_err("Register type is error. (type=%u)\n", type);
        return -EINVAL;
    }
    dms_set_dev_info_handlers[type] = func;
    return 0;
}
EXPORT_SYMBOL(dms_register_set_dev_info_handler);
#endif

int dms_unregister_set_dev_info_handler(DMS_DEV_INFO_TYPE type)
{
    if (type >= DMS_DEV_INFO_TYPE_MAX) {
        devdrv_drv_err("Unregister type is error. (type=%u)\n", type);
        return -EINVAL;
    }
    dms_set_dev_info_handlers[type] = NULL;
    return 0;
}
EXPORT_SYMBOL(dms_unregister_set_dev_info_handler);

STATIC int (*dms_get_svm_dev_info_handlers[DMS_DEV_INFO_TYPE_MAX])(u32 devid, void *buf, u32 *buf_size);

int dms_register_get_svm_dev_info_handler(DMS_DEV_INFO_TYPE type, dms_get_dev_info_ops func)
{
    if (func == NULL) {
        devdrv_drv_err("Register function of getting device infomation is null.\n");
        return -EINVAL;
    }

    if (type >= DMS_DEV_INFO_TYPE_MAX) {
        devdrv_drv_err("Register type is error. (type=%u)\n", type);
        return -EINVAL;
    }
    dms_get_svm_dev_info_handlers[type] = func;
    return 0;
}
EXPORT_SYMBOL(dms_register_get_svm_dev_info_handler);

int dms_unregister_get_svm_dev_info_handler(DMS_DEV_INFO_TYPE type)
{
    if (type >= DMS_DEV_INFO_TYPE_MAX) {
        devdrv_drv_err("Unregister type is error. (type=%u)\n", type);
        return -EINVAL;
    }
    dms_get_svm_dev_info_handlers[type] = NULL;
    return 0;
}
EXPORT_SYMBOL(dms_unregister_get_svm_dev_info_handler);

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
    [DEVDRV_DEV_RESOURCE] = devdrv_manager_get_dev_resource_info,
    [DEVDRV_VDEV_RESOURCE] = devdrv_manager_get_vdev_resource_info,
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

#ifdef CFG_FEATURE_CHIP_DIE
int devdrv_manager_ioctl_get_chip_count(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    int count = 0;
    unsigned int phy_id = 0;
    unsigned int vfid = 0;
    unsigned int dev_num;

    ret = devdrv_manager_container_logical_id_to_physical_id(0, &phy_id, &vfid);
    if (ret != 0) {
        devdrv_drv_err("Transfer logical id to physical id failed.\n");
        return -EPERM;
    }

    if (!devdrv_manager_is_pf_device(phy_id) || (vfid > 0) || devdrv_manager_container_is_in_container()) {
        ret = devdrv_get_devnum(&dev_num);
        if ((ret != 0) || (dev_num > DEVDRV_MAX_CHIP_NUM)) {
            devdrv_drv_err("Get device number failed or out of range. (dev_num=%u; ret=%d)\n", dev_num, ret);
            return ret;
        }

        count = dev_num;
    } else {
        ret = devdrv_manager_get_chip_count(&count);
        if (ret != 0) {
            devdrv_drv_err("Get chip count fail. (ret=%d).\n", ret);
            return ret;
        }
    }

    ret = copy_to_user_safe((void *)(uintptr_t)arg, &count, sizeof(int));
    if (ret != 0) {
        devdrv_drv_err("copy to user failed, ret=%d.\n", ret);
        return -EFAULT;
    }

    return 0;
}

int devdrv_manager_ioctl_get_chip_list(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    unsigned int i;
    unsigned int phy_id = 0;
    unsigned int vfid = 0;
    unsigned int dev_num;
    struct devdrv_chip_list chip_list = {0};

    ret = copy_from_user_safe(&chip_list, (void *)((uintptr_t)arg), sizeof(struct devdrv_chip_list));
    if (ret != 0) {
        devdrv_drv_err("copy_from_user_safe fail, ret(%d).\n", ret);
        return ret;
    }

    ret = devdrv_manager_container_logical_id_to_physical_id(0, &phy_id, &vfid);
    if (ret != 0) {
        devdrv_drv_err("Transfer logical id to physical id failed. (ret=%d)\n", ret);
        return -EPERM;
    }

    if (!devdrv_manager_is_pf_device(phy_id) || (vfid > 0) || devdrv_manager_container_is_in_container()) {
        ret = devdrv_get_devnum(&dev_num);
        if ((ret != 0) || (dev_num > DEVDRV_MAX_CHIP_NUM)) {
            devdrv_drv_err("Get device number failed or out of range. (dev_num=%u; ret=%d)\n", dev_num, ret);
            return ret;
        }

        chip_list.count = dev_num;
        for (i = 0; i < chip_list.count; i++) {
            chip_list.chip_list[i] = i;
        }
    } else {
        ret = devdrv_manager_get_chip_list(&chip_list);
        if (ret != 0) {
            devdrv_drv_err("Get chip list failed. (ret=%d)\n", ret);
            return ret;
        }
    }

    ret = copy_to_user_safe((void *)(uintptr_t)arg, &chip_list, sizeof(struct devdrv_chip_list));
    if (ret != 0) {
        devdrv_drv_err("copy to user failed, ret=%d.\n", ret);
        return -EFAULT;
    }

    return 0;
}

int devdrv_manager_ioctl_get_device_from_chip(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    unsigned int dev_num;
    unsigned int phy_id = 0;
    unsigned int vfid = 0;
    struct devdrv_chip_dev_list *chip_dev_list = NULL;

    chip_dev_list = (struct devdrv_chip_dev_list *)kzalloc(sizeof(struct devdrv_chip_dev_list),
        GFP_KERNEL | __GFP_ACCOUNT);
    if (chip_dev_list == NULL) {
        devdrv_drv_err("Allocate memory for chip device list failed.\n");
        return -ENOMEM;
    }

    ret = copy_from_user_safe(chip_dev_list, (void *)((uintptr_t)arg), sizeof(struct devdrv_chip_dev_list));
    if (ret != 0) {
        devdrv_drv_err("copy_from_user_safe fail, ret(%d).\n", ret);
        goto FREE_CHIP_DEV_LIST;
    }

    ret = devdrv_manager_container_logical_id_to_physical_id(0, &phy_id, &vfid);
    if (ret != 0) {
        devdrv_drv_err("Transfer logical id to physical id failed. (ret=%d)\n", ret);
        goto FREE_CHIP_DEV_LIST;
    }

    if (!devdrv_manager_is_pf_device(phy_id) || (vfid > 0) || devdrv_manager_container_is_in_container()) {
        ret = devdrv_get_devnum(&dev_num);
        if (ret != 0) {
            devdrv_drv_err("Get device number failed. (ret=%d)\n", ret);
            goto FREE_CHIP_DEV_LIST;
        }

        if ((chip_dev_list->chip_id < dev_num) && (chip_dev_list->chip_id < DEVDRV_MAX_CHIP_NUM)) {
            chip_dev_list->count = 1;
            chip_dev_list->dev_list[0] = chip_dev_list->chip_id;
        } else {
            devdrv_drv_err("Chip id is invalid. (chip_id=%u)\n", chip_dev_list->chip_id);
            ret = -EINVAL;
            goto FREE_CHIP_DEV_LIST;
        }
    } else {
        ret = devdrv_manager_get_device_from_chip(chip_dev_list);
        if (ret != 0) {
            devdrv_drv_err("Get device list from chip id failed. (ret=%d)\n", ret);
            goto FREE_CHIP_DEV_LIST;
        }
    }

    ret = copy_to_user_safe((void *)(uintptr_t)arg, chip_dev_list, sizeof(struct devdrv_chip_dev_list));
    if (ret != 0) {
        devdrv_drv_err("copy to user failed, ret=%d.\n", ret);
        goto FREE_CHIP_DEV_LIST;
    }

FREE_CHIP_DEV_LIST:
    kfree(chip_dev_list);
    chip_dev_list = NULL;
    return ret;
}

int devdrv_manager_ioctl_get_chip_from_device(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    unsigned int phy_id = 0;
    unsigned int vfid = 0;
    unsigned int dev_num;
    struct devdrv_get_dev_chip_id chip_from_dev = {0};

    ret = copy_from_user_safe(&chip_from_dev, (void *)((uintptr_t)arg), sizeof(struct devdrv_get_dev_chip_id));
    if (ret != 0) {
        devdrv_drv_err("copy_from_user_safe fail, ret(%d).\n", ret);
        return ret;
    }

    ret = devdrv_manager_container_logical_id_to_physical_id(0, &phy_id, &vfid);
    if (ret != 0) {
        devdrv_drv_err("Transfer logical id to physical id failed.\n");
        return -EPERM;
    }

    if (!devdrv_manager_is_pf_device(phy_id) || (vfid > 0) || devdrv_manager_container_is_in_container()) {
        ret = devdrv_get_devnum(&dev_num);
        if (ret != 0) {
            devdrv_drv_err("Get device number failed. (ret=%d)\n", ret);
            return ret;
        }

        if ((chip_from_dev.dev_id < dev_num) && (chip_from_dev.dev_id < DEVDRV_MAX_CHIP_NUM)) {
            chip_from_dev.chip_id = chip_from_dev.dev_id;
        } else {
            devdrv_drv_err("Device id is invalid. (dev_id=%u)\n", chip_from_dev.dev_id);
            return -EINVAL;
        }
    } else {
        ret = devdrv_manager_get_chip_from_device(&chip_from_dev);
        if (ret != 0) {
            devdrv_drv_err("devdrv_manager_get_chip_from_device failed. (ret=%d)\n", ret);
            return ret;
        }
    }

    ret = copy_to_user_safe((void *)(uintptr_t)arg, &chip_from_dev, sizeof(struct devdrv_get_dev_chip_id));
    if (ret != 0) {
        devdrv_drv_err("copy to user failed, ret=%d.\n", ret);
        return -EFAULT;
    }

    return 0;
}
#endif

STATIC int devdrv_manager_ioctl_set_vdevinfo(struct file *filep, unsigned int cmd, unsigned long arg)
{
#ifdef CFG_FEATURE_DMS_SVM_DEV
    int ret;
    u32 vfid = 0;
    u32 phy_id = 0;
    struct devdrv_svm_vdev_info vinfo = {0};
    struct devmm_set_convert_len_para len_para = {0};

    if (devdrv_manager_check_permission()) {
        devdrv_drv_err("Check permission failed.\n");
        return -EOPNOTSUPP;
    }

    ret = copy_from_user_safe(&vinfo, (void *)((uintptr_t)arg), sizeof(struct devdrv_svm_vdev_info));
    if (ret != 0) {
        devdrv_drv_err("Copy from user failed. (ret=%d)\n", ret);
        return -EFAULT;
    }

    if (vinfo.type >= DMS_DEV_INFO_TYPE_MAX) {
        devdrv_drv_err("Invalid type. (dev_id=%u; type=%u)\n", vinfo.devid, vinfo.type);
        return -EINVAL;
    }

    if (dms_set_dev_info_handlers[vinfo.type] == NULL) {
        devdrv_drv_err("Unsupported input type. (dev_id=%u; type=%u)\n", vinfo.devid, vinfo.type);
        return -EOPNOTSUPP;
    }

    if (vinfo.devid != DEVMNG_SET_ALL_DEV) {
        ret = devdrv_manager_container_logical_id_to_physical_id(vinfo.devid, &phy_id, &vfid);
        if (ret != 0) {
            devdrv_drv_err("logical_id_to_physical_id fail, devid(%u) ret(%d).\n", vinfo.devid, ret);
            return ret;
        }
        vinfo.devid = phy_id;
    }

    if (vinfo.buf_size != sizeof(len_para)) {
        devdrv_drv_err("Buffer size is error. (dev_id=%u; buf_size=%u;)\n",  vinfo.devid, vinfo.buf_size);
        return -EINVAL;
    }

    ret = memcpy_s(&len_para, sizeof(len_para), vinfo.buf, vinfo.buf_size);
    if (ret != 0) {
        devdrv_drv_err("Memcpy failed. (ret=%d)\n", ret);
        return -EFAULT;
    }

    ret = dms_set_dev_info_handlers[vinfo.type](vinfo.devid, (void *)&len_para, sizeof(len_para));
    if (ret != 0) {
        devdrv_drv_err("Set vdevice information failed. (dev_id=%u; type=%u)\n", vinfo.devid, vinfo.type);
        return ret;
    }

    return 0;
#else
    return -EOPNOTSUPP;
#endif
}

STATIC int devdrv_manager_ioctl_get_svm_vdevinfo(struct file *filep, unsigned int cmd, unsigned long arg)
{
#ifdef CFG_FEATURE_DMS_SVM_DEV
    int ret;
    u32 vfid = 0;
    u32 phy_id = 0;
    struct devdrv_svm_vdev_info vinfo = {0};

    if (devdrv_manager_check_permission()) {
        devdrv_drv_err("Check permission failed.\n");
        return -EOPNOTSUPP;
    }

    ret = copy_from_user_safe(&vinfo, (void *)((uintptr_t)arg), sizeof(struct devdrv_svm_vdev_info));
    if (ret != 0) {
        devdrv_drv_err("Copy from user failed. (ret=%d)\n", ret);
        return -EFAULT;
    }

    if (vinfo.type >= DMS_DEV_INFO_TYPE_MAX) {
        devdrv_drv_err("Invalid type. (dev_id=%u; type=%u)\n", vinfo.devid, vinfo.type);
        return -EINVAL;
    }

    if (dms_get_svm_dev_info_handlers[vinfo.type] == NULL) {
        devdrv_drv_err("Unsupported output type. (dev_id=%u; type=%u)\n", vinfo.devid, vinfo.type);
        return -EOPNOTSUPP;
    }
    ret = devdrv_manager_container_logical_id_to_physical_id(vinfo.devid, &phy_id, &vfid);
    if (ret != 0) {
        devdrv_drv_err("logical_id_to_physical_id fail, devid(%u) ret(%d).\n", vinfo.devid, ret);
        return ret;
    }
    vinfo.devid = phy_id;

    if (vinfo.buf_size != DEVDRV_SVM_VDEV_LEN) {
        devdrv_drv_err("Buffer size is error. (dev_id=%u; buf_size=%u;)\n",  vinfo.devid, vinfo.buf_size);
        return -EINVAL;
    }

    ret = dms_get_svm_dev_info_handlers[vinfo.type](vinfo.devid, (void *)vinfo.buf, &vinfo.buf_size);
    if (ret != 0) {
        devdrv_drv_err("Get vdevice information failed. (dev_id=%u; type=%u)\n", vinfo.devid, vinfo.type);
        return ret;
    }

    ret = copy_to_user_safe((void *)((uintptr_t)arg), &vinfo, sizeof(struct devdrv_svm_vdev_info));
    if (ret != 0) {
        devdrv_drv_err("devid=%d copy to user failed.,ret = %d\n", vinfo.devid, ret);
        return ret;
    }

    return 0;
#else
    return -EOPNOTSUPP;
#endif
}

#ifdef CFG_FEATURE_VASCEND
STATIC int devdrv_manager_ioctl_set_vdevmode(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    int mode;

    if (devdrv_manager_check_permission()) {
        devdrv_drv_err("Check permission failed.\n");
        return -EOPNOTSUPP;
    }

    ret = copy_from_user_safe(&mode, (void *)((uintptr_t)arg), sizeof(int));
    if (ret != 0) {
        devdrv_drv_err("Copy from user failed. (ret=%d)\n", ret);
        return -EFAULT;
    }

    ret = hw_dvt_set_mode(mode);
    if (ret != 0) {
        devdrv_drv_err("Set vdevice mode failed. (mode=%d)\n", mode);
        return ret;
    }

    return 0;
}

STATIC int devdrv_manager_ioctl_get_vdevmode(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    int mode;

    if (devdrv_manager_check_permission()) {
        devdrv_drv_err("Check permission failed.\n");
        return -EOPNOTSUPP;
    }

    ret = hw_dvt_get_mode(&mode);
    if (ret != 0) {
        devdrv_drv_err("hw_dvt_get_mode fail, ret=%d.\n", ret);
        return ret;
    }

    ret = copy_to_user_safe((void *)(uintptr_t)arg, &mode, sizeof(int));
    if (ret != 0) {
        devdrv_drv_err("copy to user failed, ret=%d.\n", ret);
        return -EFAULT;
    }

    return 0;
}
#endif

STATIC int (*const devdrv_manager_ioctl_handlers[DEVDRV_MANAGER_CMD_MAX_NR])(struct file *filep, unsigned int cmd,
    unsigned long arg) = {
        [_IOC_NR(DEVDRV_MANAGER_GET_PCIINFO)] = devdrv_manager_get_pci_info,
#ifdef CFG_FEATURE_OLD_DEVID_TRANS
        [_IOC_NR(DEVDRV_MANAGER_GET_DEVNUM)] = devdrv_manager_ioctl_get_devnum,
#endif
        [_IOC_NR(DEVDRV_MANAGER_GET_PLATINFO)] = devdrv_manager_ioctl_get_plat_info,
        [_IOC_NR(DEVDRV_MANAGER_DEVICE_STATUS)] = devdrv_manager_get_device_status,
        [_IOC_NR(DEVDRV_MANAGER_GET_CORE_SPEC)] = devdrv_manager_get_core,
        [_IOC_NR(DEVDRV_MANAGER_GET_CORE_INUSE)] = devdrv_manager_get_core,
#ifdef CFG_FEATURE_OLD_DEVID_TRANS
        [_IOC_NR(DEVDRV_MANAGER_GET_DEVIDS)] = devdrv_manager_devinfo_ioctl,
#endif
        [_IOC_NR(DEVDRV_MANAGER_GET_CONTAINER_DEVIDS)] = devdrv_manager_devinfo_ioctl,
        [_IOC_NR(DEVDRV_MANAGER_GET_DEVINFO)] = devdrv_manager_devinfo_ioctl,
        [_IOC_NR(DEVDRV_MANAGER_GET_DEVID_BY_LOCALDEVID)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_DEV_INFO_BY_PHYID)] = devdrv_manager_devinfo_ioctl,
        [_IOC_NR(DEVDRV_MANAGER_GET_PCIE_ID_INFO)] = devdrv_manager_devinfo_ioctl,
        [_IOC_NR(DEVDRV_MANAGER_GET_CORE_UTILIZATION)] = devdrv_manager_devinfo_ioctl,
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
        [_IOC_NR(DEVDRV_MANAGER_REG_DDR_READ)] = devdrv_manager_reg_ddr_read,
        [_IOC_NR(DEVDRV_MANAGER_GET_MINI_BOARD_ID)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_PCIE_PRE_RESET)] = devdrv_manager_pcie_pre_reset,
        [_IOC_NR(DEVDRV_MANAGER_PCIE_RESCAN)] = devdrv_manager_pcie_rescan,
        [_IOC_NR(DEVDRV_MANAGER_PCIE_HOT_RESET)] = devdrv_manager_pcie_hot_reset_ioctl,
        [_IOC_NR(DEVDRV_MANAGER_P2P_ATTR)] = devdrv_manager_p2p_attr_op,
        [_IOC_NR(DEVDRV_MANAGER_ALLOC_HOST_DMA_ADDR)] = drv_dma_mmap,
        [_IOC_NR(DEVDRV_MANAGER_PCIE_READ)] = drv_pcie_read,
        [_IOC_NR(DEVDRV_MANAGER_PCIE_SRAM_WRITE)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_EMMC_VOLTAGE)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_DEVICE_BOOT_STATUS)] = drv_get_device_boot_status,
        [_IOC_NR(DEVDRV_MANAGER_ENABLE_EFUSE_LDO)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_DISABLE_EFUSE_LDO)] = NULL,

        [_IOC_NR(DEVDRV_MANAGER_CONTAINER_CMD)] = devdrv_manager_container_cmd,
        [_IOC_NR(DEVDRV_MANAGER_GET_HOST_PHY_MACH_FLAG)] = devdrv_manager_get_host_phy_mach_flag,
        [_IOC_NR(DEVDRV_MANAGER_GET_LOCAL_DEVICEIDS)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_IMU_SMOKE)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_SET_NEW_TIME)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_IPC_NOTIFY_CREATE)] = devdrv_manager_ipc_notify_ioctl,
        [_IOC_NR(DEVDRV_MANAGER_IPC_NOTIFY_OPEN)] = devdrv_manager_ipc_notify_ioctl,
        [_IOC_NR(DEVDRV_MANAGER_IPC_NOTIFY_CLOSE)] = devdrv_manager_ipc_notify_ioctl,
        [_IOC_NR(DEVDRV_MANAGER_IPC_NOTIFY_DESTROY)] = devdrv_manager_ipc_notify_ioctl,
        [_IOC_NR(DEVDRV_MANAGER_IPC_NOTIFY_SET_PID)] = devdrv_manager_ipc_notify_ioctl,
        [_IOC_NR(DEVDRV_MANAGER_IPC_NOTIFY_RECORD)] = devdrv_manager_ipc_notify_ioctl,
        [_IOC_NR(DEVDRV_MANAGER_GET_CPU_INFO)] = NULL,
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
        [_IOC_NR(DEVDRV_MANAGER_PCIE_IMU_DDR_READ)] = drv_pcie_bbox_imu_ddr_read,
        [_IOC_NR(DEVDRV_MANAGER_GET_SLOT_ID)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_APPMON_BBOX_EXCEPTION_CMD)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_CONTAINER_FLAG)] = devdrv_manager_get_container_flag,
        [_IOC_NR(DEVDRV_MANAGER_GET_PROCESS_SIGN)] = devdrv_manager_get_process_sign,
        [_IOC_NR(DEVDRV_MANAGER_GET_MASTER_DEV_IN_THE_SAME_OS)] = devdrv_manager_get_master_device_in_the_same_os,
        [_IOC_NR(DEVDRV_MANAGER_GET_LOCAL_DEV_ID_BY_HOST_DEV_ID)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_BOOT_DEV_ID)] = devdrv_manager_online_get_devids,
        [_IOC_NR(DEVDRV_MANAGER_GET_TSDRV_DEV_COM_INFO)] = devdrv_manager_get_tsdrv_dev_com_info,
        [_IOC_NR(DEVDRV_MANAGER_GET_CAPABILITY_GROUP_INFO)] = devdrv_manager_get_ts_group_info,
        [_IOC_NR(DEVDRV_MANAGER_PASSTHRU_MCU)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_P2P_CAPABILITY)] = devdrv_manager_get_p2p_capability,
        [_IOC_NR(DEVDRV_MANAGER_GET_ETH_ID)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_BIND_PID_ID)] = devdrv_fop_bind_host_pid,
        [_IOC_NR(DEVDRV_MANAGER_QUERY_HOST_PID)] = devdrv_fop_query_host_pid,
        [_IOC_NR(DEVDRV_MANAGER_QUERY_DEV_PID)] = devdrv_host_query_devpid,
        [_IOC_NR(DEVDRV_MANAGER_GET_H2D_DEVINFO)] = devdrv_manager_devinfo_ioctl,
        [_IOC_NR(DEVDRV_MANAGER_GET_CONSOLE_LOG_LEVEL)] = devdrv_manager_ioctl_get_console_loglevel,
        [_IOC_NR(DEVDRV_MANAGER_CREATE_VDEV)] = devdrv_manager_ioctl_create_vdev,
        [_IOC_NR(DEVDRV_MANAGER_DESTROY_VDEV)] = devdrv_manager_ioctl_destroy_vdev,
        [_IOC_NR(DEVDRV_MANAGER_GET_VDEVINFO)] = devdrv_manager_ioctl_get_vdevinfo,
        [_IOC_NR(DEVDRV_MANAGER_GET_STARTUP_STATUS)] = devdrv_manager_get_device_startup_status,
        [_IOC_NR(DEVDRV_MANAGER_GET_DEVICE_HEALTH_STATUS)] = devdrv_manager_get_device_health_status,
        [_IOC_NR(DEVDRV_MANAGER_GET_DEV_RESOURCE_INFO)] = devdrv_manager_ioctl_get_dev_resource_info,
#ifdef CFG_FEATURE_CHIP_DIE
        [_IOC_NR(DEVDRV_MANAGER_GET_CHIP_COUNT)] = devdrv_manager_ioctl_get_chip_count,
        [_IOC_NR(DEVDRV_MANAGER_GET_CHIP_LIST)] = devdrv_manager_ioctl_get_chip_list,
        [_IOC_NR(DEVDRV_MANAGER_GET_DEVICE_FROM_CHIP)] = devdrv_manager_ioctl_get_device_from_chip,
        [_IOC_NR(DEVDRV_MANAGER_GET_CHIP_FROM_DEVICE)] = devdrv_manager_ioctl_get_chip_from_device,
#endif
        [_IOC_NR(DEVDRV_MANAGER_SET_SVM_VDEVINFO)] = devdrv_manager_ioctl_set_vdevinfo,
        [_IOC_NR(DEVDRV_MANAGER_GET_SVM_VDEVINFO)] = devdrv_manager_ioctl_get_svm_vdevinfo,
#ifdef CFG_FEATURE_VASCEND
        [_IOC_NR(DEVDRV_MANAGER_SET_VDEVMODE)] = devdrv_manager_ioctl_set_vdevmode,
        [_IOC_NR(DEVDRV_MANAGER_GET_VDEVMODE)] = devdrv_manager_ioctl_get_vdevmode,
#endif
#ifdef CFG_FEATURE_OLD_DEVID_TRANS
        [_IOC_NR(DEVDRV_MANAGER_GET_VDEVNUM)] = devdrv_manager_ioctl_get_vdevnum,
#endif
        [_IOC_NR(DEVDRV_MANAGER_GET_VDEVIDS)] = devdrv_manager_devinfo_ioctl,
        [_IOC_NR(DEVDRV_MANAGER_TS_LOG_DUMP)] = devdrv_manager_tslog_dump,
#ifdef CFG_FEATURE_DEVICE_SHARE
        [_IOC_NR(DEVDRV_MANAGER_CONFIG_DEVICE_SHARE)] = devdrv_manager_config_device_share,
#endif
};

STATIC long devdrv_manager_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
    if (dev_manager_info == NULL) {
        devdrv_drv_err("invalid parameter, "
                       "dev_manager_info = %pK, "
                       "arg = 0x%lx\n",
                       dev_manager_info, arg);
        return -EINVAL;
    }

    if (_IOC_NR(cmd) >= DEVDRV_MANAGER_CMD_MAX_NR) {
        devdrv_drv_err("cmd out of range, cmd = %u, max value = %u\n", _IOC_NR(cmd), DEVDRV_MANAGER_CMD_MAX_NR);
        return -EINVAL;
    }

    if (devdrv_manager_ioctl_handlers[_IOC_NR(cmd)] == NULL) {
        devdrv_drv_err("invalid cmd, cmd = %u\n", _IOC_NR(cmd));
        return -EINVAL;
    }

    return devdrv_manager_ioctl_handlers[_IOC_NR(cmd)](filep, cmd, arg);
}

STATIC inline int devdrv_manager_drv_ops_check(void)
{
    if (devdrv_host_drv_ops.ipc_notify_create == NULL || devdrv_host_drv_ops.ipc_notify_open == NULL ||
        devdrv_host_drv_ops.ipc_notify_close == NULL || devdrv_host_drv_ops.ipc_notify_destroy == NULL ||
        devdrv_host_drv_ops.ipc_notify_set_pid == NULL) {
        return -EINVAL;
    }
    return 0;
}

STATIC int devdrv_manager_ipc_notify_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_notify_ioctl_info notify_ioctl_info;
    struct devdrv_manager_context *context = NULL;
    int ret;

    if ((filep == NULL) || (filep->private_data == NULL)) {
        devdrv_drv_err("filep is %s.\n", (filep == NULL) ? "NULL" : "OK");
        return -EINVAL;
    }

    context = filep->private_data;
    if (context->ipc_notify_info == NULL) {
        devdrv_drv_err("context->ipc_notify_info is NULL.\n");
        return -ENODEV;
    }

    ret = copy_from_user_safe(&notify_ioctl_info, (void *)((uintptr_t)arg),
        sizeof(struct devdrv_notify_ioctl_info));
    if (ret) {
        devdrv_drv_err("copy from user failed, ret(%d).\n", ret);
        return -EFAULT;
    }
    notify_ioctl_info.name[DEVDRV_IPC_NAME_SIZE - 1] = '\0';

    devdrv_manager_ops_sem_down_read();
    if (devdrv_manager_drv_ops_check()) {
        devdrv_manager_ops_sem_up_read();
        devdrv_drv_err("ipc notify function not init\n");
        return -EINVAL;
    }

    switch (cmd) {
        case DEVDRV_MANAGER_IPC_NOTIFY_CREATE:
            ret = devdrv_host_drv_ops.ipc_notify_create((void*)context, arg, (void*)(uintptr_t)&notify_ioctl_info);
            break;
        case DEVDRV_MANAGER_IPC_NOTIFY_OPEN:
            ret = devdrv_host_drv_ops.ipc_notify_open((void*)context, arg, (void*)(uintptr_t)&notify_ioctl_info);
            break;
        case DEVDRV_MANAGER_IPC_NOTIFY_CLOSE:
            ret = devdrv_host_drv_ops.ipc_notify_close((void*)context, (void*)(uintptr_t)&notify_ioctl_info);
            break;
        case DEVDRV_MANAGER_IPC_NOTIFY_DESTROY:
            ret = devdrv_host_drv_ops.ipc_notify_destroy((void*)context, (void*)(uintptr_t)&notify_ioctl_info);
            break;
        case DEVDRV_MANAGER_IPC_NOTIFY_SET_PID:
            ret = devdrv_host_drv_ops.ipc_notify_set_pid((void*)context, (void*)(uintptr_t)&notify_ioctl_info);
            break;
#ifndef DEVDRV_MANAGER_HOST_UT_TEST
        case DEVDRV_MANAGER_IPC_NOTIFY_RECORD:
            ret = devdrv_host_drv_ops.ipc_notify_record((void*)context, (void*)(uintptr_t)&notify_ioctl_info);
            break;
#endif
        default:
            devdrv_drv_err("invalid cmd, cmd = %d\n", _IOC_NR(cmd));
            ret = -EFAULT;
            break;
    };
    devdrv_manager_ops_sem_up_read();
    (void)memset_s(notify_ioctl_info.name, DEVDRV_IPC_NAME_SIZE, 0, DEVDRV_IPC_NAME_SIZE);

    return ret;
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

    devdrv_drv_debug("device(%u) ts doorbell addr.\n", dev_id);
    return 0;
}

int devdrv_manager_send_msg(struct devdrv_info *dev_info, struct devdrv_manager_msg_info *dev_manager_msg_info,
                            int *out_len)
{
    int ret;

    ret = devdrv_common_msg_send(dev_info->pci_dev_id, dev_manager_msg_info, sizeof(struct devdrv_manager_msg_info),
                                 sizeof(struct devdrv_manager_msg_info), (u32 *)out_len,
                                 DEVDRV_COMMON_MSG_DEVDRV_MANAGER);

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
    if (ret || (dev_manager_msg_info.header.result != 0)) {
        devdrv_drv_err("send dev_id(%u) to device(%u) failed, ret(%d)\n", dev_info->dev_id, dev_info->pci_dev_id, ret);
        return -EFAULT;
    }

    return 0;
}

u32 devdrv_manager_get_ts_num(struct devdrv_info *dev_info)
{
    if (dev_info == NULL) {
        devdrv_drv_err("invalid input handler.\n");
        return (u32)-1;
    }
    if (dev_info->pdata == NULL) {
        devdrv_drv_err("invalid input handler.\n");
        return (u32)-1;
    }

    if (dev_info->pdata->ts_num > DEVDRV_MAX_TS_NUM) {
        devdrv_drv_err("ts_num(%u).\n", dev_info->pdata->ts_num);
        return (u32)-1;
    }
    dev_info->pdata->ts_num = 1;

    return dev_info->pdata->ts_num;
}
EXPORT_SYMBOL(devdrv_manager_get_ts_num);

int devdrv_manager_register(struct devdrv_info *dev_info)
{
    int ret;

    if (dev_info == NULL) {
        devdrv_drv_err("devdrv manager has not initialized\n");
        return -EINVAL;
    }

    if (devdrv_manager_get_devdrv_info(dev_info->dev_id) != NULL) {
        devdrv_drv_err("device(%u) has already registered\n", dev_info->dev_id);
        return -ENODEV;
    }

    if (devdrv_manager_get_dev_resource(dev_info)) {
        devdrv_drv_err("devdrv manager get device(%u) resource failed\n", dev_info->dev_id);
        return -EINVAL;
    }
    dev_info->drv_ops = &devdrv_host_drv_ops;

    if (devdrv_manager_set_devinfo_inc_devnum(dev_info->dev_id, dev_info)) {
        devdrv_drv_err("devdrv_manager_set_devinfo_inc_devnum failed, deviceid : %u\n", dev_info->dev_id);
        goto devinfo_iounmap;
    }

    if (devdrv_manager_send_devid(dev_info)) {
        devdrv_drv_err("send devid to device(%u) failed.\n", dev_info->dev_id);
        goto devinfo_unregister_client;
    }

    if (dms_device_register(dev_info)) {
        devdrv_drv_err("Dms device register failed. (dev_id=%u)\n", dev_info->dev_id);
        goto devinfo_unregister_client;
    }

    ret = devdrv_set_module_init_finish(dev_info->dev_id, DEVDRV_HOST_MODULE_DEVMNG);
    if (ret) {
        devdrv_drv_err("set module init finish failed, dev_id = %u\n", dev_info->dev_id);
        goto devinfo_unregister_client;
    }

    return 0;
devinfo_unregister_client:
    (void)devdrv_manager_reset_devinfo_dec_devnum(dev_info->dev_id);
devinfo_iounmap:
    return -EINVAL;
}
EXPORT_SYMBOL(devdrv_manager_register);

void devdrv_manager_unregister(struct devdrv_info *dev_info)
{
    devdrv_drv_debug("devdrv_manager_unregister started.\n");

    if ((dev_info == NULL) || (dev_manager_info == NULL) || (dev_info->dev_id >= DEVDRV_MAX_DAVINCI_NUM)) {
        devdrv_drv_err("dev_info(%pK) or dev_manager_info(%pK) is NULL, dev_id(%u)\n", dev_info, dev_manager_info,
                       (dev_info == NULL) ? DEVDRV_MAX_DAVINCI_NUM : dev_info->dev_id);
        return;
    }

    if (dev_manager_info->dev_info[dev_info->dev_id] == NULL) {
        devdrv_drv_err("device(%u) is not initialized\n", dev_info->dev_id);
        return;
    }
    if (devdrv_manager_reset_devinfo_dec_devnum(dev_info->dev_id)) {
        devdrv_drv_err("devdrv_manager_unregister device(%u) fail !!!!!!!\n", dev_info->dev_id);
        return;
    }

    dms_device_unregister(dev_info);

    devdrv_drv_debug("devdrv_manager_unregister device(%u) finished, "
                     "dev_manager_info->num_dev = %d\n",
                     dev_info->dev_id, dev_manager_info->num_dev);
}
EXPORT_SYMBOL(devdrv_manager_unregister);

STATIC int devdrv_manager_device_ready_info(struct devdrv_info *dev_info, struct devdrv_device_info *drv_info)
{
    struct devdrv_platform_data *pdata = dev_info->pdata;
    u32 tsid = 0;
    int ret;

    spin_lock_bh(&dev_info->spinlock);
    dev_info->ai_core_num = drv_info->ai_core_num;
    dev_info->aicore_freq = drv_info->aicore_freq;
    pdata->ai_core_num_level = drv_info->ai_core_num_level;
    pdata->ai_core_freq_level = drv_info->ai_core_freq_level;
    dev_info->ai_cpu_core_num = drv_info->ai_cpu_core_num;
    dev_info->ctrl_cpu_core_num = drv_info->ctrl_cpu_core_num;
    dev_info->ctrl_cpu_occupy_bitmap = drv_info->ctrl_cpu_occupy_bitmap;
    dev_info->ctrl_cpu_endian_little = drv_info->ctrl_cpu_endian_little;
    dev_info->ctrl_cpu_id = drv_info->ctrl_cpu_id;
    dev_info->ctrl_cpu_ip = drv_info->ctrl_cpu_ip;
    pdata->ts_pdata[tsid].ts_cpu_core_num = drv_info->ts_cpu_core_num;
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
    dev_info->ffts_type = drv_info->ffts_type;
    dev_info->vector_core_num = drv_info->vector_core_num;
    dev_info->vector_core_freq = drv_info->vector_core_freq;
    dev_info->aicore_bitmap = drv_info->aicore_bitmap;
    spin_unlock_bh(&dev_info->spinlock);

    devdrv_drv_info("device: %u"
                    "ai cpu num: %d, "
                    "ai cpu broken bitmap: 0x%x, "
                    "ai core num: %d,"
                    "ai core broken bitmap: 0x%x, "
                    "ai subsys broken map: 0x%x.\n",
                    dev_info->dev_id, drv_info->ai_cpu_ready_num, drv_info->ai_cpu_broken_map,
                    drv_info->ai_core_ready_num, drv_info->ai_core_broken_map, drv_info->ai_subsys_ip_map);

    if ((dev_info->ai_cpu_core_num > MAX_BIT_NUM) || (dev_info->ai_core_num > MAX_BIT_NUM)) {
        devdrv_drv_err("invalid num : aicpu %u aicore  %u, \n", dev_info->ai_cpu_core_num, dev_info->ai_core_num);
        return -ENODEV;
    }
    (void)hvdevmng_set_core_num(dev_info->dev_id, 0, 0);

#ifdef CFG_FEATURE_VFIO
    vmngh_set_total_core_num(dev_info->dev_id, dev_info->ai_core_num);
#endif
    ret = soc_resmng_dev_set_mia_res(dev_info->dev_id, MIA_AC_AIC, dev_info->aicore_bitmap, 1);
    devdrv_drv_info("Set aicore bitmap. (devid=%u; aicore_num=%u; bitmap=%llx; ret=%d)\n",
        dev_info->dev_id, dev_info->ai_core_num, dev_info->aicore_bitmap, ret);

    dev_info->dump_ddr_dma_addr = drv_info->dump_ddr_dma_addr;
    dev_info->dump_ddr_size = drv_info->dump_ddr_size;

    dev_info->capability = drv_info->capability;
    dev_info->reg_ddr_dma_addr = drv_info->reg_ddr_dma_addr;
    dev_info->reg_ddr_size = drv_info->reg_ddr_size;
    dev_info->chip_name = drv_info->chip_name;
    dev_info->chip_version = drv_info->chip_version;
    dev_info->chip_info = drv_info->chip_info;
    dev_info->chip_id = drv_info->chip_id;
    dev_info->multi_chip = drv_info->multi_chip;
    dev_info->multi_die = drv_info->multi_die;
    dev_info->mainboard_id = drv_info->mainboard_id;
    dev_info->connect_type = drv_info->connect_type;
    dev_info->board_id = drv_info->board_id;
    dev_info->die_id = drv_info->die_id;
    dev_info->dev_nominal_osc_freq = drv_info->dev_nominal_osc_freq;

#ifdef CFG_FEATURE_PG
    ret = strcpy_s(dev_info->pg_info.spePgInfo.socVersion, MAX_CHIP_NAME, drv_info->soc_version);
    if (ret) {
        devdrv_drv_err("Call strcpy_s failed.\n");
        return 0;
    }
#endif
#ifdef CFG_FEATURE_SRIOV
    ret = strcpy_s(dev_info->template_name, TEMPLATE_NAME_LEN, drv_info->template_name);
    if (ret != 0) {
        devdrv_drv_err("Copy name length failed.(devid=%u; ret=%d)\n", dev_info->dev_id, ret);
        return -EINVAL;
    }
#endif

    devdrv_drv_info("Initialize chip info. (chip_name=%u; chip_version=%u; nominal_osc_freq=%llu)\n",
        dev_info->chip_name, dev_info->chip_version, dev_info->dev_nominal_osc_freq);

    devdrv_drv_debug("received ready message from pcie device(%u)\n", dev_info->dev_id);
    devdrv_drv_debug(" ai_core_num = %d, ai_cpu_core_num = %d, "
                     "ctrl_cpu_core_num = %d, ctrl_cpu_endian_little = %d, "
                     "ctrl_cpu_id = %d, ctrl_cpu_ip = %d, "
                     "ts_cpu_core_num = %d, "
                     "ai_core_id = %d, "
                     "ai_cpu_core_id = %d\n",
                     dev_info->ai_core_num,
                     dev_info->ai_cpu_core_num,
                     dev_info->ctrl_cpu_core_num,
                     dev_info->ctrl_cpu_endian_little,
                     dev_info->ctrl_cpu_id,
                     dev_info->ctrl_cpu_ip,
                     pdata->ts_pdata[tsid].ts_cpu_core_num,
                     dev_info->ai_core_id,
                     dev_info->ai_cpu_core_id);
    devdrv_drv_debug("env_type = %d\n", dev_info->env_type);
    return 0;
}

STATIC u32 devdrv_get_ts_num(void)
{
    u32 tsid;

#ifndef CFG_SOC_PLATFORM_MINIV2
    tsid = DEVDRV_MAX_TS_NUM;
#else
    tsid = 1;  // tsid should be transfered from dev side later
#endif /* CFG_SOC_PLATFORM_MINIV2 */
    return tsid;
}

STATIC int devdrv_manager_device_ready(void *msg, u32 *ack_len)
{
    struct devdrv_manager_msg_info *dev_manager_msg_info = NULL;
    struct devdrv_device_info *drv_info = NULL;
    struct devdrv_platform_data *pdata = NULL;
    struct devdrv_info *dev_info = NULL;
    u32 dev_id;
    int ret;

#ifndef DEVDRV_MANAGER_HOST_UT_TEST
    if (module_init_finish == 0) {
        devdrv_drv_warn("Init not finish.\n");
        return -EINVAL;
    }
#endif

    dev_manager_msg_info = (struct devdrv_manager_msg_info *)msg;
    dev_id = dev_manager_msg_info->header.dev_id;
    devdrv_drv_info("Receive device ready notify. (dev_id=%u)\n", dev_id);
    if (dev_manager_msg_info->header.valid != DEVDRV_MANAGER_MSG_VALID) {
        devdrv_drv_err("invalid message from host\n");
        return -EINVAL;
    }

    if (dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("invalid dev_id(%u)\n", dev_id);
        return -ENODEV;
    }

    /* dev send message with pcie device id, get dev_info from devdrv_info_array */
    dev_info = devdrv_get_devdrv_info_array(dev_id);
    if (dev_info == NULL) {
        devdrv_drv_warn("device(%u) is not ready.\n", dev_id);
        return -EINVAL;
    }

    if (dev_info->dev_ready != 0) {
        devdrv_drv_info("device(%u) already informed\n", dev_id);
        return -EINVAL;
    }

    dev_info->dev_ready = DEVDRV_DEV_READY_EXIST;

    pdata = dev_info->pdata;
    if (pdata == NULL) {
        devdrv_drv_err("pata is NULL\n");
        return -ENOMEM;
    }
    pdata->dev_id = dev_info->dev_id;
    pdata->ts_num = devdrv_get_ts_num();

    drv_info = (struct devdrv_device_info *)dev_manager_msg_info->payload;
    if (drv_info->ts_load_fail == 0) {
        devdrv_drv_info("*** ts fw load succeed, set device(%u) ready work ***\n", dev_id);
    }
    dev_info->ts_num = pdata->ts_num;

    ret = devdrv_manager_device_ready_info(dev_info, drv_info);
    if (ret) {
        devdrv_drv_err("device(%u) ready info failed\n", dev_id);
        return -EINVAL;
    }
    *ack_len = sizeof(*dev_manager_msg_info);
    dev_manager_msg_info->header.result = 0;

    if (dev_manager_info != NULL) {
        devdrv_drv_info("device(%u) ready\n", dev_id);
        queue_work(dev_manager_info->dev_rdy_work, &dev_info->work);
    }

    devdrv_drv_info("Receive device ready notify End. (dev_id=%u)\n", dev_id);

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
        devdrv_drv_err("invalid dev_id(%u)\n", dev_id);
        return -ENODEV;
    }

    dev_info = devdrv_get_devdrv_info_array(dev_id);
    if (dev_info == NULL) {
        devdrv_drv_err("device(%u) is not ready.\n", dev_id);
        return -EINVAL;
    }

    switch (status) {
        case TS_DOWN:
            devdrv_drv_err("receive ts exception message from device(%u).\n", dev_id);
            devdrv_host_manager_device_exception(dev_info);
            ret = 0;
            break;
        case TS_WORK:
            devdrv_drv_err("receive ts resume message from device(%u).\n", dev_id);
            ret = devdrv_host_manager_device_resume(dev_info);
            break;
        case TS_FAIL_TO_SUSPEND:
            devdrv_drv_err("receive ts fail to suspend message from device(%u).\n", dev_id);
            ret = devdrv_host_manager_device_resume(dev_info);
            break;
        case TS_SUSPEND:
            devdrv_drv_err("receive ts suspend message from device(%u).\n", dev_id);
            ret = devdrv_host_manager_device_suspend(dev_info);
            break;
        default:
            devdrv_drv_err("invalid input ts status. dev_id(%u)\n", dev_id);
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
        devdrv_drv_err("Invalid device id. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }
    d_info = devdrv_get_manager_info();
    if (d_info == NULL) {
        devdrv_drv_err("d_info is null. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }
    process_sign = (struct process_sign *)dev_manager_msg_info->payload;
    mutex_lock(&d_info->devdrv_sign_list_lock);
    if (list_empty_careful(&d_info->hostpid_list_header)) {
        devdrv_drv_err("Hostpid sign list is empty. (dev_id=%u; hostpid=%d)\n", dev_id, process_sign->tgid);
        goto out;
    }
    list_for_each_safe(pos, n, &d_info->hostpid_list_header) {
        d_sign = list_entry(pos, struct devdrv_process_sign, list);
        if (d_sign->hostpid == process_sign->tgid) {
            ret = devdrv_manager_container_check_devid_in_container(dev_id, d_sign->hostpid);
            if (ret) {
                devdrv_drv_err("Device id and hostpid mismatch in container. (dev_id=%u; hostpid=%d; ret=%d)\n",
                    dev_id, d_sign->hostpid, ret);
            }

            devdrv_drv_info("Process sign check success. (dev_id=%u; hostpid=%d)\n", dev_id, process_sign->tgid);
            goto out;
        }
    }

    devdrv_drv_err("Host pid is not in process sign list. (dev_id=%u; hostpid=%d)\n", dev_id, process_sign->tgid);

out:
    mutex_unlock(&d_info->devdrv_sign_list_lock);
    *ack_len = sizeof(*dev_manager_msg_info);
    dev_manager_msg_info->header.result = ret;
    return ret;
}

STATIC int devdrv_manager_get_pcie_id(void *msg, u32 *ack_len)
{
    struct devdrv_manager_msg_info *dev_manager_msg_info = NULL;
    struct devdrv_pcie_id_info pcie_id_info = {0};
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
    if ((dev_id >= DEVDRV_MAX_DAVINCI_NUM) || (devdrv_info_array[dev_id] == NULL)) {
        devdrv_drv_err("invalid dev_id(%u), or devdrv_info_array[dev_id] is NULL\n", dev_id);
        return -EINVAL;
    }

    dev_info = devdrv_info_array[dev_id];

    ret = devdrv_get_pcie_id_info(dev_info->pci_dev_id, &pcie_id_info);
    if (ret) {
        devdrv_drv_err("devdrv_manager_get_pcie_id_info failed. dev_id(%u)\n", dev_id);
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
    if ((dev_id >= DEVDRV_MAX_DAVINCI_NUM) || (devdrv_info_array[dev_id] == NULL)) {
        devdrv_drv_err("invalid dev_id(%u), or devdrv_info_array[dev_id] is NULL\n", dev_id);
        return -EINVAL;
    }

    *ack_len = sizeof(*dev_manager_msg_info);
    dev_manager_msg_info->header.result = ret;
    return 0;
}

STATIC int devdrv_manager_d2h_get_device_index(void *msg, u32 *ack_len)
{
    struct devdrv_manager_msg_info *dev_manager_msg_info = NULL;
    u32 *host_devid_buf = NULL;
    int device_index;

    dev_manager_msg_info = (struct devdrv_manager_msg_info *)msg;
    if (dev_manager_msg_info->header.valid != DEVDRV_MANAGER_MSG_VALID) {
        devdrv_drv_err("invalid message from host\n");
        return -EINVAL;
    }
    *ack_len = sizeof(*dev_manager_msg_info);

    host_devid_buf = (u32 *)dev_manager_msg_info->payload;
    device_index = devdrv_get_device_index(*host_devid_buf);
    if (device_index < 0) {
        devdrv_drv_err("invalid host devid(%u).\n", *host_devid_buf);
        dev_manager_msg_info->header.result = DEVDRV_MANAGER_MSG_INVALID_RESULT;
    } else {
        *host_devid_buf = (u32)device_index;
        dev_manager_msg_info->header.result = 0;
    }

    return 0;
}

STATIC int devdrv_manager_receive_tslog_addr(void *msg, u32 *ack_len)
{
    u32 dev_id;
    struct devdrv_ts_log *ts_log = NULL;
    struct devdrv_manager_msg_info *dev_manager_msg_info = NULL;

    dev_manager_msg_info = (struct devdrv_manager_msg_info *)msg;
    dev_id = dev_manager_msg_info->header.dev_id;

    if (dev_manager_msg_info->header.valid != DEVDRV_MANAGER_MSG_VALID) {
        devdrv_drv_err("Invalid message from device. (device id=%u)\n", dev_id);
        return -EINVAL;
    }

    if (dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("Invalid device id. (device id=%u)\n", dev_id);
        return -ENODEV;
    }

    ts_log = (struct devdrv_ts_log *)dev_manager_msg_info->payload;
    g_devdrv_ts_log_array[dev_id]->devid = ts_log->devid;
    g_devdrv_ts_log_array[dev_id]->dma_addr = ts_log->dma_addr;
    g_devdrv_ts_log_array[dev_id]->mem_size = ts_log->mem_size;

    *ack_len = sizeof(*dev_manager_msg_info);
    dev_manager_msg_info->header.result = 0;

    return 0;
}

static int devdrv_set_device_info_aicpu(u32 dev_id, u32 aicpu_num)
{
    struct devdrv_info *dev_info = NULL;
    u32 ai_cpu_core_id;
    u32 aicpu_occupy_bitmap;

    /* aicpu_num set to 7 when SRIOV enable */
    if (aicpu_num == 7) {
        ai_cpu_core_id = 1;
        aicpu_occupy_bitmap = 0xFE;
    } else {
        /* when SRIOV disable, aicpu_num set to 6, aicpu ID begin from 2 */
        ai_cpu_core_id = 2;
        aicpu_occupy_bitmap = 0xFC;
    }

    dev_info = devdrv_manager_get_devdrv_info(dev_id);
    if (dev_info == NULL) {
        return -EINVAL;
    }

    dev_info->ai_cpu_core_num = aicpu_num;
    dev_info->ai_cpu_core_id = ai_cpu_core_id;
    dev_info->aicpu_occupy_bitmap = aicpu_occupy_bitmap;
    dev_info->inuse.ai_cpu_num = aicpu_num;
    return 0;
}

STATIC int devdrv_set_host_aicpu_num_from_device(void *msg, u32 *ack_len)
{
    u32 dev_id;
    u32 *aicpu_num = NULL;
    struct devdrv_manager_msg_info *dev_manager_msg_info = NULL;
    int ret;

    if (msg == NULL || ack_len == NULL) {
        devdrv_drv_err("Invalid para. (msg is NULL=%d; ack_len is NULL=%d)\n", msg == NULL, ack_len == NULL);
        return -EINVAL;
    }

    dev_manager_msg_info = (struct devdrv_manager_msg_info *)msg;
    dev_id = dev_manager_msg_info->header.dev_id;

    if (dev_manager_msg_info->header.valid != DEVDRV_MANAGER_MSG_D2H_MAGIC) {
        devdrv_drv_err("Invalid message from device. (device id=%u)\n", dev_id);
        return -EINVAL;
    }

    if (dev_id >= DEVDRV_PF_DEV_MAX_NUM) {
        devdrv_drv_err("Invalid device id. (device id=%u)\n", dev_id);
        return -ENODEV;
    }

    aicpu_num = (int *)dev_manager_msg_info->payload;

    /* aicpu_num must only be 6 or 7 */
    if (*aicpu_num > 7 || *aicpu_num < 6) {
        devdrv_drv_err("Invalid aicpu number. (dev_id=%u, aicpu_num=%u)\n", dev_id, *aicpu_num);
        return -EINVAL;
    }

    ret = devdrv_set_device_info_aicpu(dev_id, *aicpu_num);
    if (ret != 0) {
        devdrv_drv_err("Set aicpu_num failed. (dev_id=%u, aicpu_num=%u)\n", dev_id, *aicpu_num);
        return ret;
    }

    ret = hvdevmng_set_core_num(dev_id, 0, 0);
    if (ret != 0) {
        devdrv_drv_err("hvdevmng_set_core_num failed. (dev_id=%u, aicpu_num=%u)\n", dev_id, *aicpu_num);
        return ret;
    }

    *ack_len = sizeof(u32) + sizeof(struct devdrv_manager_msg_head);
    dev_manager_msg_info->header.result = 0;

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
    [DEVDRV_MANAGER_CHAN_D2H_SYNC_MATRIX_READY] = devdrv_manager_d2h_sync_matrix_ready,
    [DEVDRV_MANAGER_CHAN_D2H_CHECK_PROCESS_SIGN] = devdrv_manager_check_process_sign,
    [DEVDRV_MANAGER_CHAN_D2H_GET_DEVICE_INDEX] = devdrv_manager_d2h_get_device_index,
    [DEVDRV_MANAGER_CHAN_D2H_DMS_EVENT_DISTRIBUTE] = dms_event_get_exception_from_device,
    [DEVDRV_MANAGER_CHAN_D2H_SEND_TSLOG_ADDR] = devdrv_manager_receive_tslog_addr,
#if (!defined (DEVMNG_UT)) && (!defined (DEVDRV_MANAGER_HOST_UT_TEST))
    [DEVDRV_MANAGER_CHAN_PID_MAP_SYNC] = devdrv_pid_map_sync_proc,
#endif
    [DEVDRV_MANAGER_CHAN_D2H_SET_HOST_AICPU_NUM] = devdrv_set_host_aicpu_num_from_device,
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

    if ((dev_id >= DEVDRV_MAX_DAVINCI_NUM) || (data == NULL) || (real_out_len == NULL) ||
        (in_data_len < sizeof(struct devdrv_manager_msg_info))) {
        devdrv_drv_err("date(%pK) or real_out_len(%pK) is NULL, devid(%u), in_data_len(%u)\n",
            data, real_out_len, dev_id, in_data_len);
        return -EINVAL;
    }
    msg_id = ((struct devdrv_manager_msg_head *)data)->msg_id;

    if (msg_id >= DEVDRV_MANAGER_CHAN_MAX_ID) {
        devdrv_drv_err("invalid msg_id(%u)\n", msg_id);
        return -EINVAL;
    }
    if (devdrv_manager_chan_msg_processes[msg_id] == NULL) {
        devdrv_drv_err("devdrv_manager_chan_msg_processes[%u] is NULL\n", msg_id);
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
    struct devdrv_info *dev_info = NULL;
    u32 msg_id;
    int dev_id;

    if ((msg_chan == NULL) || (data == NULL) || (real_out_len == NULL) ||
        (in_data_len < sizeof(struct devdrv_manager_msg_info))) {
        devdrv_drv_err("msg_chan(%pK) or data(%pK) or real_out_len(%pK) is NULL, in_date_len(%u)\n",
            msg_chan, data, real_out_len, in_data_len);
        return -EINVAL;
    }
    msg_id = ((struct devdrv_manager_msg_head *)data)->msg_id;

    if ((msg_id >= DEVDRV_MANAGER_CHAN_MAX_ID) || (devdrv_manager_chan_msg_processes[msg_id] == NULL)) {
        devdrv_drv_err("invalid msg_id(%u) or devdrv_manager_chan_msg_processes[msg_id] is NULL\n", msg_id);
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
        devdrv_drv_err("invalid dev_id(%d).\r\n", dev_id);
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
    .level = DEVDRV_MSG_CHAN_LEVEL_LOW,
    .s_desc_size = DEV_MNG_NON_TRANS_MSG_DESC_SIZE,
    .c_desc_size = DEV_MNG_NON_TRANS_MSG_DESC_SIZE,
    .rx_msg_process = devdrv_manager_rx_msg_process,
};

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

STATIC void devdrv_manager_dev_ready_work(struct work_struct *work)
{
    struct devdrv_info *dev_info = NULL;
    void *no_trans_chan = NULL;
    u32 tsid = 0;
    u32 dev_id;
    u32 chip_type;
    int ret;

    dev_info = container_of(work, struct devdrv_info, work);
    dev_id = dev_info->pci_dev_id;
    if (dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("invalid dev_id(%u)\n", dev_id);
        return;
    }

    devdrv_drv_info("Device ready work start. (dev_id=%u)\n", dev_id);
    no_trans_chan = devdrv_pcimsg_alloc_non_trans_queue(dev_id, &dev_manager_msg_chan_info);
    if (no_trans_chan == NULL) {
        devdrv_drv_err("device(%u) no_trans_chan alloc failed.\n", dev_id);
        return;
    }

    devdrv_manager_set_no_trans_chan(dev_id, no_trans_chan);
    devdrv_set_msg_chan_priv(no_trans_chan, (void *)dev_info);

    if (devdrv_manager_register(dev_info)) {
        devdrv_drv_err("devdrv_manager_register failed. dev_id(%u)\n", dev_id);
        return;
    }

    tsdrv_set_ts_status(dev_info->dev_id, tsid, TS_WORK);
    dev_info->dev_ready = DEVDRV_DEV_READY_WORK;

    chip_type = devdrv_get_dev_chip_type(dev_id);

#ifdef CFG_FEATURE_TIMESYNC
    dms_time_sync_init(dev_id);
    devdrv_refresh_aicore_info_init(dev_id);
#endif

    up(&dev_info->no_trans_chan_wait_sema);
    atomic_set(&dev_info->occupy_ref, 0);
    dev_info->status = DEVINFO_STATUS_WORKING;
    /* not need to return error when call device_boot_init.sh fail */
    ret = devmng_call_dev_boot_init(dev_id);
    if (ret) {
        devdrv_drv_err("dev_id[%u] devmng_call_dev_boot_init fail, ret[%d]\n", dev_id, ret);
    }
    ret = dms_hotreset_task_init(dev_id);
    if (ret != 0) {
        devdrv_drv_err("Dms hotreset task init fail. (dev_id=%u; ret=%d)\n", dev_id, ret);
    }
}

STATIC inline void devdrv_manager_common_chan_init(void)
{
    /* this function will be called at module_init, doesn't need lock */
    devdrv_manager_common_chan.type = DEVDRV_COMMON_MSG_DEVDRV_MANAGER;
    devdrv_manager_common_chan.common_msg_recv = devdrv_manager_rx_common_msg_process;
    devdrv_manager_common_chan.init_notify = devdrv_manager_msg_chan_init_notify;
}

STATIC inline void devdrv_manager_common_chan_uninit(void)
{
    /* this function will be called at module_init, doesn't need lock */
    devdrv_manager_common_chan.type = DEVDRV_COMMON_MSG_TYPE_MAX;
    devdrv_manager_common_chan.common_msg_recv = NULL;
    devdrv_manager_common_chan.init_notify = NULL;
}

STATIC inline struct devdrv_common_msg_client *devdrv_manager_get_common_chan(u32 dev_id)
{
    return &devdrv_manager_common_chan;
}

void *devdrv_manager_get_no_trans_chan(u32 dev_id)
{
    void *no_trans_chan = NULL;

    /* dev_manager_no_trasn_chan doesn't need lock */
    if (dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("get no trans chan failed, dev_id(%u)\n", dev_id);
        return NULL;
    }
    no_trans_chan = dev_manager_no_trasn_chan[dev_id];
    return no_trans_chan;
}

STATIC void devdrv_manager_set_no_trans_chan(u32 dev_id, void *no_trans_chan)
{
    if (dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("set no trans chan failed, dev_id(%u)\n", dev_id);
        return;
    }
    dev_manager_no_trasn_chan[dev_id] = no_trans_chan;
}

int devdrv_agent_sync_msg_send(u32 dev_id, struct devdrv_manager_msg_info *msg_info, u32 payload_len, u32 *out_len)
{
    u32 in_len;
    void *no_trans_chan = NULL;

    if ((dev_id >= DEVDRV_MAX_DAVINCI_NUM) || (msg_info == NULL) || (out_len == NULL) ||
        (payload_len > sizeof(msg_info->payload))) {
        devdrv_drv_err("invaild dev_id(%u) or msg_info(%pK) is null or out_len(%pK) is null.\n",
            dev_id, msg_info, out_len);
        return -EINVAL;
    }

    no_trans_chan = devdrv_manager_get_no_trans_chan(dev_id);
    if (no_trans_chan == NULL) {
        devdrv_drv_err("get device(%u) no trans chan failed", dev_id);
        return -ENODEV;
    }

    in_len = sizeof(struct devdrv_manager_msg_head) + payload_len;

    return devdrv_sync_msg_send(no_trans_chan, msg_info, in_len, in_len, out_len);
}
EXPORT_SYMBOL(devdrv_agent_sync_msg_send);

struct tsdrv_drv_ops *devdrv_manager_get_drv_ops(void)
{
    return &devdrv_host_drv_ops;
}
EXPORT_SYMBOL(devdrv_manager_get_drv_ops);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 15, 0)
struct devdrv_check_start_s {
    u32 dev_id;
    struct timer_list check_timer;
};

STATIC struct devdrv_check_start_s devdrv_check_start[DEVDRV_MAX_DAVINCI_NUM];

STATIC void devdrv_check_start_event(struct timer_list *t)
{
    struct devdrv_info *dev_info = NULL;
    struct timespec stamp;
    struct devdrv_check_start_s *devdrv_start_check = from_timer(devdrv_start_check, t, check_timer);
    u32 dev_id;
    u32 tsid = 0;

    dev_id = devdrv_start_check->dev_id;
#else  /* LINUX_VERSION_CODE >= KERNEL_VERSION(4,15,0) */

STATIC struct timer_list devdrv_check_start[DEVDRV_MAX_DAVINCI_NUM];

STATIC void devdrv_check_start_event(unsigned long data)
{
    struct devdrv_info *dev_info = NULL;
    struct timespec stamp;
    u32 dev_id = (u32)data;
    u32 tsid = 0;
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(4,15,0) */

    devdrv_drv_debug("*** time event for checking whether device is started or not ***\n");

    stamp = current_kernel_time();

    dev_info = devdrv_get_devdrv_info_array(dev_id);
    ;
    if (dev_info == NULL) {
        devdrv_drv_err("device(%u) is not ready.\n", dev_id);
        return;
    }

    if (dev_info->dev_ready < DEVDRV_DEV_READY_EXIST) {
        devdrv_drv_err("device(%u) is not ready, "
                       "dev_ready = %d, dev_id = %u\n",
                       dev_id, dev_info->dev_ready, dev_info->dev_id);
        (void)devdrv_host_black_box_add_exception(dev_info->dev_id, DEVDRV_BB_DEVICE_LOAD_TIMEOUT, stamp, NULL);
        tsdrv_set_ts_status(dev_info->dev_id, tsid, TS_DOWN);
        return;
    }

    devdrv_drv_debug("*** device(%u) is started and working ***\n", dev_id);
}

STATIC int devdrv_manager_dev_state_notify(u32 probe_num, u32 devid, u32 state)
{
    struct devdrv_black_box_state_info *bbox_state_info = NULL;
    struct timespec tstemp = {0};
    struct devdrv_info *dev_info = NULL;
    unsigned long flags;
    int ret;

    if ((dev_manager_info == NULL) || ((enum devdrv_device_state)state >= STATE_TO_MAX) ||
        (devid >= DEVDRV_MAX_DAVINCI_NUM)) {
        devdrv_drv_err("state notify para is invalid,"
                       "with dev_manager_info(%pK),"
                       "state(%d), devid(%u).\n",
                       dev_manager_info, (u32)state, devid);
        return -ENODEV;
    }

    spin_lock_irqsave(&dev_manager_info->spinlock, flags);
    dev_manager_info->prob_num = probe_num;
    spin_unlock_irqrestore(&dev_manager_info->spinlock, flags);

    dev_info = devdrv_get_devdrv_info_array(devid);
    if (dev_info == NULL) {
        devdrv_drv_warn("device is not ready.\n");
        return -ENODEV;
    }

    CLR_BIT_64(dev_manager_info->prob_device_bitmap[devid / BITS_PER_LONG_LONG], devid % BITS_PER_LONG_LONG);

    bbox_state_info = kzalloc(sizeof(struct devdrv_black_box_state_info), GFP_ATOMIC | __GFP_ACCOUNT);
    if (bbox_state_info == NULL) {
        devdrv_drv_err("malloc state_info failed. dev_id(%u)\n", devid);
        return -ENOMEM;
    }

    bbox_state_info->devId = devid;
    bbox_state_info->state = state;

    devdrv_drv_info("dev state notified with devid(%u), state(%d)", devid, state);

    ret = devdrv_host_black_box_add_exception(0, DEVDRV_BB_DEVICE_STATE_INFORM, tstemp, (void *)bbox_state_info);
    if (ret) {
        kfree(bbox_state_info);
        bbox_state_info = NULL;
        devdrv_drv_err("devdrv_host_black_box_add_exception failed. dev_id(%u)\n", devid);
        ssleep(1); // add 1s for bbox to dump when unbind
        return -ENODEV;
    }

    kfree(bbox_state_info);
    bbox_state_info = NULL;
    ssleep(1); // add 1s for bbox to dump when unbind

    return 0;
}

STATIC int devdrv_manager_dev_state_notify_compat(u32 devid, u32 state)
{
    return devdrv_manager_dev_state_notify(devdrv_manager_get_probe_num_kernel(), devid, state);
}

STATIC int devdrv_manager_dev_startup_notify(u32 prob_num, const u32 devids[], u32 array_len, u32 devnum)
{
#ifndef DEVDRV_MANAGER_HOST_UT_TEST
    struct devdrv_black_box_devids *bbox_devids = NULL;
    struct timespec tstemp = {0};
    unsigned long flags;
    int ret;
    u32 i;

    (void)array_len;
    if ((dev_manager_info == NULL) || (devnum > DEVDRV_MAX_DAVINCI_NUM) || (devids == NULL)) {
        devdrv_drv_err("dev manager info is not initialized\n");
        return -ENODEV;
    }

    spin_lock_irqsave(&dev_manager_info->spinlock, flags);
    dev_manager_info->prob_num = prob_num;
    for (i = 0; i < devnum; i++) {
        if (devdrv_manager_is_pf_device(devids[i]) || devdrv_manager_is_mdev_vm_mode(devids[i])) {
            SET_BIT_64(dev_manager_info->prob_device_bitmap[devids[i] / BITS_PER_LONG_LONG],
                devids[i] % BITS_PER_LONG_LONG);
        }
    }
    spin_unlock_irqrestore(&dev_manager_info->spinlock, flags);

    bbox_devids = kzalloc(sizeof(struct devdrv_black_box_devids), GFP_ATOMIC | __GFP_ACCOUNT);
    if (bbox_devids == NULL) {
        devdrv_drv_err("malloc devids failed\n");
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
        devdrv_drv_err("devdrv_host_black_box_add_exception failed, ret(%d).\n", ret);
        return -ENODEV;
    }
    kfree(bbox_devids);
    bbox_devids = NULL;
#endif
    return 0;
}

STATIC void devmng_tslog_addr_uninit(void)
{
    int devid;

    for (devid = 0; devid < DEVDRV_MAX_DAVINCI_NUM; devid++) {
        if (g_devdrv_ts_log_array[devid] != NULL) {
            kfree(g_devdrv_ts_log_array[devid]);
            g_devdrv_ts_log_array[devid] = NULL;
        }
    }
}

STATIC int devmng_tslog_addr_init(void)
{
    int devid;

    for (devid = 0; devid < DEVDRV_MAX_DAVINCI_NUM; devid++) {
        g_devdrv_ts_log_array[devid] = kzalloc(sizeof(struct devdrv_ts_log), GFP_KERNEL | __GFP_ACCOUNT);
        if (g_devdrv_ts_log_array[devid] == NULL) {
            devmng_tslog_addr_uninit();
            return -ENOMEM;
        }
    }

    return 0;
}

STATIC int devmng_shm_init(struct devdrv_info *dev_info)
{
#if (!defined CFG_SOC_PLATFORM_MDC_V51)
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
#endif
    return 0;
}

STATIC void devmng_shm_uninit(struct devdrv_info *dev_info)
{
#if (!defined CFG_SOC_PLATFORM_MDC_V51)
    if (dev_info->shm_vaddr != NULL) {
        iounmap(dev_info->shm_vaddr);
    }

    dev_info->shm_vaddr = NULL;
    dev_info->shm_head = NULL;
    dev_info->shm_soc = NULL;
    dev_info->shm_board = NULL;
    dev_info->shm_status = NULL;
#endif
    return;
}

int devdrv_manager_check_capability(u32 dev_id, devdrv_capability_type type)
{
    struct devdrv_info *dev_info = NULL;

    dev_info = devdrv_get_devdrv_info_array(dev_id);
    if (dev_info == NULL) {
        devdrv_drv_err("dev_id(%u) get device info failed.\n", dev_id);
        return -EINVAL;
    }

    return (dev_info->capability & (unsigned int)(1U << (unsigned int)type)) ? 0 : -EOPNOTSUPP;
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

#ifndef DEVDRV_MANAGER_HOST_UT_TEST
STATIC int devdrv_manager_init_instance(u32 dev_id, struct device *dev)
{
    struct devdrv_platform_data *pdata = NULL;
    struct devdrv_info *dev_info = NULL;
    u32 tsid = 0;
    int ret;
    u32 init_flag = 1;

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

    sema_init(&dev_info->no_trans_chan_wait_sema, 0);
    dev_info->dev_ready = 0;
    dev_info->driver_flag = 0;
    dev_info->dev = dev;
    dev_info->capability = 0;
    dev_info->dmp_started = false;
#ifdef CFG_FEATURE_DEVICE_SHARE
    set_device_share_flag(dev_id, DEVICE_UNSHARE);
#endif

    tsdrv_set_ts_status(dev_info->dev_id, tsid, TS_INITING);
    devdrv_drv_debug("*** set status initing device id :%u***\n", dev_id);

    if (init_flag == 1) {
        pdata = kzalloc(sizeof(struct devdrv_platform_data), GFP_KERNEL | __GFP_ACCOUNT);
        if (pdata == NULL) {
            ret = -ENOMEM;
            goto kfree_info;
        }

        /* host has one ts in any scenes */
        pdata->ts_num = 1;

#ifdef CFG_FEATURE_OLD_DEVID_TRANS
        ret = devdrv_manager_create_one_device(dev_info);
        if (ret) {
            devdrv_drv_err("devdrv_manager_create_one_device failed, ret(%d). dev_id(%u)\n", ret, dev_id);
            goto kfree_info_pdata;
        }
#endif
    } else {
        pdata = dev_info->pdata;
    }

    if (init_flag == 1) {
        devdrv_set_devdrv_info_array(dev_id, dev_info);

        dev_info->plat_type = (u8)DEVDRV_MANAGER_HOST_ENV;
        dev_info->pdata = pdata;

        spin_lock_init(&dev_info->spinlock);
    }
    devdrv_manager_set_no_trans_chan(dev_id, NULL);
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
    devdrv_set_device_hotreset_flag(dev_id, 0);

    devdrv_drv_info("devdrv_manager_init_instance dev_id :%u OUT !\n", dev_id);
    return 0;

release_one_device:
#ifdef CFG_FEATURE_OLD_DEVID_TRANS
    devdrv_manager_release_one_device(dev_info);
    devdrv_set_devdrv_info_array(dev_id, NULL);
    dev_info->pdata = NULL;
kfree_info_pdata:
#endif
    kfree(pdata);
    pdata = NULL;
kfree_info:
    devmng_shm_uninit(dev_info);
    mutex_destroy(&dev_info->lock);
    kfree(dev_info);
    dev_info = NULL;
    return ret;
}
#endif

STATIC int devdrv_manager_uninit_instance(u32 dev_id)
{
    struct devdrv_common_msg_client *devdrv_commn_chan = NULL;
    struct devdrv_info *dev_info = NULL;
    void *no_trans_chan = NULL;
    int ret;
    u32 retry_cnt = 0;

    dev_info = devdrv_get_devdrv_info_array(dev_id);
    if ((dev_id >= DEVDRV_MAX_DAVINCI_NUM) || (dev_info == NULL)) {
        devdrv_drv_err("get sema timeout,the ready of device is not ok, dev_id:%u. dev_info = %pK.\n",
            dev_id, dev_info);
        return -EINVAL;
    }

    dev_info->status = DEVINFO_STATUS_REMOVED;
    dev_info->dmp_started = false;
    while (retry_cnt < WAIT_PROCESS_EXIT_TIME) {
        if (atomic_read(&dev_info->occupy_ref) == 0) {
            break;
        }
        retry_cnt++;
        msleep(1);
    }
    devmng_shm_uninit(dev_info);

    ret = down_timeout(&dev_info->no_trans_chan_wait_sema, DEVDRV_INIT_INSTANCE_TIMEOUT);
    if (ret) {
        devdrv_drv_err("devid %d get sema from init instance timeout, ret:%d\n", dev_id, ret);
    }
    cancel_work_sync(&dev_info->work);
    devdrv_drv_info("dev_id(%u) wait ioctrl retry_cnt %d.\n", dev_id, retry_cnt);

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
        devdrv_set_msg_chan_priv(no_trans_chan, NULL);
        devdrv_pcimsg_free_non_trans_queue(no_trans_chan);
        devdrv_manager_set_no_trans_chan(dev_id, NULL);
        no_trans_chan = NULL;
    }

    devdrv_manager_unregister(dev_info);
    devdrv_manager_online_del_devids(dev_id);

    if (!devdrv_manager_is_pf_device(dev_id)) {
        /* VF device need to destory all device info when destory vdevice */
        dev_manager_info->device_status[dev_id] = DRV_STATUS_INITING;
    }

    return 0;
}

void devdrv_manager_uninit_one_device_info(unsigned int dev_id)
{
    struct devdrv_info *dev_info = NULL;

    dev_info = devdrv_get_devdrv_info_array(dev_id);
    if (dev_info == NULL) {
        return;
    }

#ifdef CFG_FEATURE_OLD_DEVID_TRANS
    devdrv_manager_release_one_device(dev_info);
#endif
    devmng_shm_uninit(dev_info);

    devdrv_set_devdrv_info_array(dev_id, NULL);

    kfree(dev_info->pdata);
    dev_info->pdata = NULL;
    mutex_destroy(&dev_info->lock);
    kfree(dev_info);
    dev_info = NULL;
}

void devdrv_manager_uninit_devinfo(void)
{
    u32 i;

    for (i = 0; i < DEVDRV_MAX_DAVINCI_NUM; i++) {
        devdrv_manager_uninit_one_device_info(i);
    }
}

#define DEVMNG_HOST_NOTIFIER "mng_host"
#ifndef DEVDRV_MANAGER_HOST_UT_TEST
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
#endif
static const struct pci_device_id devdrv_driver_tbl[] = {{ PCI_VDEVICE(HUAWEI, 0xd100), 0 },
                                                         { PCI_VDEVICE(HUAWEI, 0xd105), 0 },
                                                         { PCI_VDEVICE(HUAWEI, PCI_DEVICE_CLOUD), 0 },
                                                         { PCI_VDEVICE(HUAWEI, 0xd801), 0 },
                                                         { PCI_VDEVICE(HUAWEI, 0xd500), 0 },
                                                         { PCI_VDEVICE(HUAWEI, 0xd501), 0 },
                                                         { PCI_VDEVICE(HUAWEI, 0xd802), 0 },
                                                         { PCI_VDEVICE(HUAWEI, 0xd803), 0 },
                                                         { PCI_VDEVICE(HUAWEI, 0xd804), 0 },
                                                         { PCI_VDEVICE(HUAWEI, 0xd805), 0 },
                                                         { DEVDRV_DIVERSITY_PCIE_VENDOR_ID, 0xd500,
                                                           PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
                                                         {}};
MODULE_DEVICE_TABLE(pci, devdrv_driver_tbl);

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

#ifdef CFG_FEATURE_OLD_DEVID_TRANS
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

static unsigned int devdrv_major;
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
        devdrv_drv_err("stream alloc_chrdev_region failed, ret(%d).\n", ret);
        return ret;
    }

    devdrv_major = MAJOR(devdrv_dev);
    devdrv_class = class_create(THIS_MODULE, "devdrv-class");
    if (IS_ERR(devdrv_class)) {
        unregister_chrdev_region(devdrv_dev, DEVDRV_MAX_DAVINCI_NUM);
        devdrv_drv_err("class_create failed.\n");
        return -EINVAL;
    }

    return 0;
}

STATIC void devdrv_manager_release_driver_device(void)
{
    class_destroy(devdrv_class);
    unregister_chrdev_region(MKDEV(devdrv_major, 0), DEVDRV_MAX_DAVINCI_NUM);
}

STATIC int devdrv_manager_get_file_uid_gid(const char *file_name, gid_t *uid, gid_t *gid)
{
    struct file *filp = NULL;

    filp = filp_open(file_name, O_RDONLY, 0);
    if (IS_ERR_OR_NULL(filp)) {
        devdrv_drv_err("File open failed. (PTR_ERR=%ld)\n", PTR_ERR(filp));
        return -ENOENT;
    }

    if (filp->f_inode == NULL) {
        filp_close(filp, NULL);
        filp = NULL;
        devdrv_drv_err("File inode is null.\n");
        return -ENOENT;
    }

    *uid = filp->f_inode->i_uid.val;
    *gid = filp->f_inode->i_gid.val;
    devdrv_drv_debug("Get file user and group id. (uid=%u; gid=%u)\n", *uid, *gid);
    filp_close(filp, NULL);
    filp = NULL;
    return 0;
}

#define DEV_NAME_LEN 20
#define DEV_UID_GID_CMD_LEN 20
STATIC int devdrv_manager_change_device_owner(unsigned int dev_id)
{
    int ret;
    gid_t uid = 0;
    gid_t gid = 0;
    char *cmd_path = "/bin/chown";
    char argv_dev_name[DEV_NAME_LEN] = {0};
    char *argv[] = {cmd_path, NULL, NULL, NULL};
    char argv_uid_gid[DEV_UID_GID_CMD_LEN] = {0};
    const char *manager_file_name = "/dev/davinci_manager";
    char *envp[] = {"PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL};

    ret = devdrv_manager_get_file_uid_gid(manager_file_name, &uid, &gid);
    if (ret != 0) {
        devdrv_drv_err("Get file uid and gid failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    ret = sprintf_s(argv_dev_name, DEV_NAME_LEN, "/dev/vdavinci%u", dev_id);
    if (ret < 0) {
        devdrv_drv_err("Call sprintf_s to get device name failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return -ENODEV;
    }

    ret = sprintf_s(argv_uid_gid, DEV_UID_GID_CMD_LEN, "%u:%u", (unsigned int)uid, (unsigned int)gid);
    if (ret < 0) {
        devdrv_drv_err("Calling sprintf_s to get device name failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return -ENODEV;
    }

    argv[1] = argv_uid_gid;
    argv[2] = argv_dev_name; /* argument 2 for device name */
    ret = call_usermodehelper(cmd_path, argv, envp, UMH_WAIT_EXEC);
    if (ret != 0) {
        devdrv_drv_err("Calling call_usermodehelper failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    return 0;
}

#define CHECK_DEVICE_FILE_INTERVAL_MS 10
#define CHECK_DEVICE_FILE_TIME 300
STATIC int devdrv_manager_check_device_file(unsigned int dev_id)
{
    int ret;
    struct file *filp = NULL;
    int file_exist_timeout = CHECK_DEVICE_FILE_TIME;
    char dev_name[DEV_NAME_LEN] = {0};

    ret = sprintf_s(dev_name, DEV_NAME_LEN, "/dev/vdavinci%u", dev_id);
    if (ret < 0) {
        devdrv_drv_err("Call sprintf_s to get device name failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return -ENODEV;
    }

    while (file_exist_timeout > 0) {
        msleep(CHECK_DEVICE_FILE_INTERVAL_MS);
        filp = filp_open(dev_name, O_RDONLY, 0);
        if (IS_ERR_OR_NULL(filp)) {
            if (PTR_ERR(filp) == -ENOENT) {
                devdrv_drv_err("File open failed. (dev_id=%u; timeout=%d)\n", dev_id, file_exist_timeout);
                file_exist_timeout--;
                continue;
            } else {
                devdrv_drv_err("File open failed. (dev_id=%u; PTR_ERR=%ld)\n", dev_id, PTR_ERR(filp));
                return -ENOENT;
            }
        } else {
            devdrv_drv_info("Device file create success. (dev_id=%u)\n", dev_id);
            filp_close(filp, NULL);
            filp = NULL;
            return 0;
        }
    }

    devdrv_drv_err("Check device file timeout. (dev_id=%u)\n", dev_id);
    return -ENOENT;
}

STATIC int devdrv_manager_change_vdev_owner_proc(unsigned int dev_id)
{
    int ret;
    ret = devdrv_manager_check_device_file(dev_id);
    if (ret != 0) {
        devdrv_drv_err("Device file is abnormal. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    ret = devdrv_manager_change_device_owner(dev_id);
    if (ret != 0) {
        devdrv_drv_err("To change virtual device owner failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }
    return 0;
}

STATIC int devdrv_manager_change_vdev_owner_thread(void *data)
{
    int ret;
    unsigned int dev_id = *(unsigned int *)data;

    ret = devdrv_manager_change_vdev_owner_proc(dev_id);
    if (ret != 0) {
        devdrv_drv_err("Change vdev file owner failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
        do_exit(0);
        return ret;
    }
    do_exit(0);
    return 0;
}

STATIC int devdrv_manager_create_one_device(struct devdrv_info *dev_info)
{
    struct task_struct *change_owner_thread = NULL;
    struct device *i_device = NULL;
    dev_t devno;
    int ret;

    devno = MKDEV(devdrv_major, dev_info->dev_id);
    dev_info->cce_ops.devdrv_cdev.owner = THIS_MODULE;
    cdev_init(&dev_info->cce_ops.devdrv_cdev, &devdrv_davinci_fops);
    ret = cdev_add(&dev_info->cce_ops.devdrv_cdev, devno, DEVDRV_MAX_DAVINCI_NUM);
    if (ret) {
        devdrv_drv_err("cdev_add failed, ret(%d). dev_id(%u)\n", ret, dev_info->dev_id);
        return ret;
    }

    if (devdrv_manager_is_pf_device(dev_info->dev_id) || devdrv_manager_is_mdev_vm_mode(dev_info->dev_id)) {
        i_device = device_create(devdrv_class, NULL, devno, NULL, "davinci%d", dev_info->dev_id);
        if (IS_ERR(i_device)) {
            cdev_del(&dev_info->cce_ops.devdrv_cdev);
            devdrv_drv_err("device_create failed. (device_id=%u)\n", dev_info->dev_id);
            return -ENODEV;
        }
    } else {
        i_device = device_create(devdrv_class, NULL, devno, NULL, "vdavinci%d", dev_info->dev_id);
        if (IS_ERR(i_device)) {
            cdev_del(&dev_info->cce_ops.devdrv_cdev);
            devdrv_drv_err("device_create failed. (device_id=%u)\n", dev_info->dev_id);
            return -ENODEV;
        }

        change_owner_thread = kthread_create(devdrv_manager_change_vdev_owner_thread,
            (void *)&dev_info->dev_id, "devdrv_change_owner_%u", dev_info->dev_id);
        if (IS_ERR_OR_NULL(change_owner_thread)) {
            if (devdrv_manager_change_vdev_owner_proc(dev_info->dev_id)) {
                device_destroy(devdrv_class, devno);
                cdev_del(&dev_info->cce_ops.devdrv_cdev);
                devdrv_drv_err("Create thread for change vdevice owner failed. (dev_id=%u)\n", dev_info->dev_id);
                return -EINVAL;
            }
        } else {
            (void)wake_up_process(change_owner_thread);
        }
    }

#ifndef DEVDRV_MANAGER_HOST_UT_TEST
    if (devdrv_get_device_hotreset_flag(dev_info->dev_id) == 0) {
        ret = devdrv_manager_container_table_devlist_add_ns(&dev_info->dev_id, 1,
                                                            init_task.nsproxy->mnt_ns);
        if (ret) {
            device_destroy(devdrv_class, devno);
            cdev_del(&dev_info->cce_ops.devdrv_cdev);
            devdrv_drv_err("add to list failed. dev_id(%u)\n", dev_info->dev_id);
            return -ENODEV;
        }
    }
#endif

    dev_info->cce_ops.cce_dev = i_device;

    return 0;
}

STATIC void devdrv_manager_release_one_device(struct devdrv_info *dev_info)
{
    dev_t devno;

#ifndef DEVDRV_MANAGER_HOST_UT_TEST
    if (devdrv_get_device_hotreset_flag(dev_info->dev_id) == 0) {
        (void)devdrv_manager_container_table_devlist_del_ns(&dev_info->dev_id, 1, init_task.nsproxy->mnt_ns);
    }
#endif

    devno = MKDEV(devdrv_major, dev_info->dev_id);

    if (dev_info->cce_ops.cce_dev != NULL) {
        device_destroy(devdrv_class, devno);
        cdev_del(&dev_info->cce_ops.devdrv_cdev);
        dev_info->cce_ops.cce_dev = NULL;
    }
}
#endif

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

#ifdef CFG_FEATURE_NOTIFY_REBOOT
    devdrv_notify_all_dev_reboot();
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

    spin_lock_init(&dev_manager_info->proc_hash_table_lock);
#ifndef DEVDRV_MANAGER_HOST_UT_TEST
    hash_init(dev_manager_info->proc_hash_table);
#endif
    INIT_LIST_HEAD(&dev_manager_info->hostpid_list_header);
    mutex_init(&dev_manager_info->devdrv_sign_list_lock);
    (void)memset_s(dev_manager_info->devdrv_sign_count, MAX_DOCKER_NUM + 1, 0, MAX_DOCKER_NUM + 1);

    dev_manager_info->prob_num = 0;

    for (i = 0; i < DEVDRV_MAX_DAVINCI_NUM / BITS_PER_LONG_LONG + 1; i++) {
        dev_manager_info->prob_device_bitmap[i] = 0;
    }

    devdrv_manager_dev_num_reset();
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

#ifdef CFG_FEATURE_OLD_DEVID_TRANS
    ret = devdrv_manager_create_driver_device();
    if (ret) {
        devdrv_drv_err("devdrv_manager_create_driver_device return error: %d, "
                       "unable to create davinci device class.\n", ret);
        goto create_driver_device_fail;
    }
#endif

    ret = devdrv_manager_info_init();
    if (ret) {
        devdrv_drv_err("Init dev_manager_info failed.\n");
        goto dev_manager_info_init_failed;
    }

    ret = devmng_tslog_addr_init();
    if (ret) {
        devdrv_drv_err("Ts log addr init failed. (ret=%d)\n", ret);
        goto tslog_addr_init_fail;
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
        devdrv_drv_err("sysfs create failed, ret(%d)\n", ret);
        goto sysfs_create_group_failed;
    }
#endif /* CONFIG_SYSFS */

    dev_manager_info->dev_rdy_work = create_singlethread_workqueue("dev_manager_workqueue");
    if (dev_manager_info->dev_rdy_work == NULL) {
        devdrv_drv_err("create workqueue failed\n");
        ret = -EINVAL;
        goto workqueue_create_failed;
    }

    ret = devdrv_manager_container_table_init(dev_manager_info);
    if (ret) {
        devdrv_drv_err("container table init failed, ret(%d)\n", ret);
        ret = -ENODEV;
        goto container_table_init_failed;
    }

    ret = devdrv_manager_online_kfifo_alloc();
    if (ret) {
        devdrv_drv_err("kfifo_alloc failed, ret(%d)\n", ret);
        ret = -ENOMEM;
        goto online_kfifo_alloc_failed;
    }

    ret = dev_mnt_vdevice_init();
    if (ret != 0) {
        devdrv_drv_err("vdevice init failed, ret = %d\n", ret);
        goto vdevice_init_failed;
    }

    devdrv_manager_common_chan_init();
#ifndef DEVDRV_MANAGER_HOST_UT_TEST
    uda_davinci_near_real_entity_type_pack(&type);
    ret = uda_notifier_register(DEVMNG_HOST_NOTIFIER, &type, UDA_PRI1, devdrv_manager_host_notifier_func);
    if (ret) {
        devdrv_drv_err("uda_notifier_register failed, ret=%d\n", ret);
        goto uda_notifier_register_failed;
    }
#endif
    ret = hvdevmng_init();
    if (ret != 0) {
        devdrv_drv_err("vmngh_register vmnh client failed, ret = %d\n", ret);
        goto vmngh_register_failed;
    }

    drvdrv_dev_startup_register(devdrv_manager_dev_startup_notify);
    drvdrv_dev_state_notifier_register(devdrv_manager_dev_state_notify_compat);

    ret = log_level_file_init();
    if (ret != 0) {
        devdrv_drv_err("log_level_file_init failed!!! ret(%d), default_log_level is ERROR.\n", ret);
    }
    init_rwsem(&devdrv_ops_sem);
#ifdef CFG_FEATURE_CHIP_DIE
    devdrv_manager_chip_dev_map_init();
#endif
#ifndef DEVDRV_MANAGER_HOST_UT_TEST
    module_init_finish = 1;
#endif
    return 0;

vmngh_register_failed:
#ifndef DEVDRV_MANAGER_HOST_UT_TEST
    (void)uda_notifier_unregister(DEVMNG_HOST_NOTIFIER, &type);
#endif
uda_notifier_register_failed:
    dev_mnt_vdevice_uninit();
vdevice_init_failed:
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
    devmng_tslog_addr_uninit();
tslog_addr_init_fail:
    devdrv_manager_info_free();
dev_manager_info_init_failed:
#ifdef CFG_FEATURE_OLD_DEVID_TRANS
    devdrv_manager_release_driver_device();
create_driver_device_fail:
#endif
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

#ifdef CFG_FEATURE_CHIP_DIE
    devdrv_manager_chip_dev_map_exit();
#endif
    log_level_file_remove();
    devdrv_dev_state_notifier_unregister();

#ifdef CONFIG_SYSFS
    sysfs_remove_group(&dev_manager_info->dev->kobj, &devdrv_manager_attr_group);
#endif /* CONFIG_SYSFS */

    hvdevmng_uninit();
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
    devdrv_manager_free_hashtable();

    uda_davinci_near_real_entity_type_pack(&type);
    (void)uda_notifier_unregister(DEVMNG_HOST_NOTIFIER, &type);
    devdrv_manager_common_chan_uninit();

    devdrv_manager_uninit_devinfo();

    destroy_workqueue(dev_manager_info->dev_rdy_work);
    devdrv_manager_container_table_exit(dev_manager_info);
    kfree(dev_manager_info);
    dev_manager_info = NULL;
    devdrv_manager_unregister_notifier();
    devdrv_manager_online_kfifo_free();
    dev_mnt_vdevice_uninit();
    dms_hotreset_task_exit();
#ifdef CFG_FEATURE_OLD_DEVID_TRANS
    devdrv_manager_release_driver_device();
#endif
    if (drv_ascend_unregister_sub_module(DAVINCI_INTF_MODULE_DEVMNG)) {
        devdrv_drv_err("drv_ascend_unregister_sub_module failed!\n");
    }

    devmng_tslog_addr_uninit();
}
