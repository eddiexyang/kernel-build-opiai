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
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
#include <linux/workqueue.h>
#include <linux/idr.h>
#include <linux/list.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include <linux/hugetlb.h>
#include <linux/poll.h>
#include <linux/securec.h>
#include <linux/securectype.h>
#include <linux/pci.h>

#ifndef DEVMNG_UT
#ifndef COMPILE_WITH_UT
#include <linux/acpi.h>
#include <linux/property.h>
#endif

#include "devdrv_dfm.h"
#include <linux/dma-mapping.h>

#include "dbl/uda.h"

#include "devdrv_adapt_manager.h"
#include "devdrv_manager_common.h"
#include "devmng_dms_adapt.h"
#include "devdrv_platform.h"
#include "devdrv_driver_pm.h"
#include "devdrv_manager.h"
#include "devdrv_interface.h"
#include "devdrv_pm.h"
#include "devdrv_manager_msg.h"
#include "devdrv_ipc_msg.h"
#include "devdrv_platform_resource.h"
#include "devdrv_manager_container.h"
#include "devdrv_manager_time.h"
#include "devdrv_ipc.h"
#include "devdrv_platform_resource.h"
#include "devdrv_manager_freq.h"
#include "devdrv_manager_container_noplugin.h"
#include "devdrv_flash_config.h"
#include "config.h"
#include "appmon_black_box.h"
#include "devdrv_manager_base.h"
#include "tsdrv_status.h"
#include "devdrv_manager_dev_share.h"
#include "davinci_interface.h"
#include "davinci_api.h"
#include "devdrv_manager_comm.h"
#include "icm_interface.h"
#include "soc_spec.h"
#include "tsmng_interface.h"
#include "drv_whitelist.h"
#include "dms_device_time_zone.h"
#include "ascend_hal_error.h"

#ifdef CFG_FEATURE_TIMESYNC
#include "dms_common.h"
#include "dms_time.h"
#endif
#ifdef CFG_FEATURE_HEALTH_ERR_CODE
#include "heart_beat.h"
#endif

extern u8 __iomem *g_sysctl;
struct devdrv_manager_info *dev_manager_info = NULL;
extern struct tsdrv_drv_ops devdrv_platform_drv_ops;
void *devdrv_manager_msg_chan[DEVDRV_MAX_DAVINCI_NUM];
STATIC int devdrv_manager_msg_chan_initialized = 0;
STATIC int devdrv_manager_msg_notify_initialized = 0;
STATIC struct rw_semaphore devdrv_ops_sem;

const u32 g_devmng_custom_process_white_list[DEVDRV_MANAGER_CMD_MAX_NR] = {
    DEVDRV_MANAGER_GET_DEVNUM,
    DEVDRV_MANAGER_GET_PLATINFO,
    DEVDRV_MANAGER_GET_CORE_SPEC,
    DEVDRV_MANAGER_GET_CORE_INUSE,
    DEVDRV_MANAGER_GET_DEVIDS,
    DEVDRV_MANAGER_GET_DEVINFO,
    DEVDRV_MANAGER_GET_DEVID_BY_LOCALDEVID,
    DEVDRV_MANAGER_GET_DEV_INFO_BY_PHYID,
    DEVDRV_MANAGER_GET_MINI_BOARD_ID,
    DEVDRV_MANAGER_CONTAINER_CMD,
    DEVDRV_MANAGER_GET_LOCAL_DEVICEIDS,
    DEVDRV_MANAGER_GET_CPU_INFO,
    DEVDRV_MANAGER_GET_PROCESS_SIGN,
    DEVDRV_MANAGER_BIND_PID_ID,
    DEVDRV_MANAGER_QUERY_DEV_PID,
    DEVDRV_MANAGER_UNBIND_PID_ID,
    DEVDRV_MANAGER_GET_CONTAINER_FLAG,
    DEVDRV_MANAGER_QUERY_HOST_PID,
};

int devdrv_lc_gpioirq_register(void);
void devdrv_lc_gpioirq_unregister(void);

STATIC int devdrv_manager_common_msg_process(u32 devid, void *data, u32 in_data_len, u32 out_data_len,
    u32 *real_out_len);

STATIC int devdrv_manager_nfe_irq_register(struct devdrv_info *dev_info);

int devdrv_limitcur_gpioirq_register(void);
void devdrv_limitcur_gpioirq_unregister(void);

STATIC void devdrv_manager_release_one_device(struct devdrv_info *dev_info);
STATIC int devdrv_manager_create_one_device(struct devdrv_info *dev_info);


static u8 devdrv_lpm3_start_fail = 0;

struct workqueue_struct *devdrv_manager_inform_wq;

wait_queue_head_t devdrv_manage_poll_wq; /* wait queue */

#define DEVMNG_FW_WQ_RETRY_COUNT 100
#define DEVMNG_FW_WQ_DELAY_TIME 2000

#define DEVMNG_PID_INVALID (-1)
#define DEVMNG_PID_START_ONCE (-2)

#define MAX_INVAILD_TLB_SIZE 0x4000000 // 64M

#define SHOW_CONFIG_BUF_SIZE 64
/* Protect idr accesses */
static DEFINE_MUTEX(devdrv_mutex);
static DEFINE_MUTEX(devdrv_mn_mutex_dev0);
static DEFINE_MUTEX(devdrv_mn_mutex_dev1);
static DEFINE_MUTEX(devdrv_mn_mutex_dev2);
static DEFINE_MUTEX(devdrv_mn_mutex_dev3);

extern int devdrv_get_boardid(void);
extern int devdrv_get_config_index_by_name(const char *name);
extern int devdrv_get_user_config_core(int cfg_index, unsigned char *buf, unsigned int *buf_size);

#ifdef CONFIG_SYSFS
#define DEVDRV_ATTR_RO(_name) static struct kobj_attribute _name##_attr = __ATTR_RO(_name)

#define DEVDRV_ATTR(_name) static struct kobj_attribute _name##_attr = __ATTR(_name, 0600, _name##_show, _name##_store)

bool devdrv_manager_is_pf_device(unsigned int dev_id)
{
    return true;
}
EXPORT_SYMBOL(devdrv_manager_is_pf_device);

STATIC ssize_t devdrv_resources_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    if (buf == NULL) {
        devdrv_drv_err("invalid args(buf is null).\n");
        return count;
    }
    return count;
}

int devmng_get_vdavinci_info(u32 vdev_id, u32 *phy_id, u32 *vfid)
{
    if ((phy_id == NULL) || (vfid == NULL))
        return -EINVAL;

    *phy_id = vdev_id;
    *vfid = 0;
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
}EXPORT_SYMBOL(hvdevmng_set_dev_ts_resource);

STATIC ssize_t devdrv_resources_show_refill_buf(struct devdrv_info *dev_info, char *str, int i)
{
    char *refill_buf = str;
    int tlen;

    tlen = snprintf_s(str, PAGE_SIZE, PAGE_SIZE - 1, "dev_manager_info->dev_id[%d] = %u\n", i,
        dev_manager_info->dev_id[i]);
    if (tlen < 0) {
        devdrv_drv_info("sprintf_s is abnormal. (i=%d; dev_manager_info->dev_id[i]=%u)\n", i,
            dev_manager_info->dev_id[i]);
        return 0;
    }
    str += tlen;

    return str - refill_buf;
}

STATIC ssize_t devdrv_resources_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct devdrv_info *dev_info = NULL;
    char *str = buf;
    int i;
    int ret;

    for (i = 0; i < MAX_CHIP_NUM; i++) {
        dev_info = devdrv_manager_get_devdrv_info(i);
        if (dev_info == NULL) {
            devdrv_drv_err("dev_info[%d] is NULL\n", i);
            continue;
        }

        ret = devdrv_resources_show_refill_buf(dev_info, str, i);
        if (!ret) {
            devdrv_drv_info("sprintf_s is abnormal. (chip_num=%d)\n", i);
            return 0;
        }
        str += ret;
    }
    return str - buf;
}
DEVDRV_ATTR(devdrv_resources);

STATIC ssize_t devdrv_interrups_show_refill_buf(struct devdrv_platform_data *pdata, char *str, int i)
{
    int tlen;
    int wlen = 0;

    tlen = sprintf_s(str + wlen, PAGE_SIZE - wlen, "dev_manager_info->dev_id[%d] = %u\n", i,
        dev_manager_info->dev_id[i]);
    if (tlen < 0) {
        devdrv_drv_info("sprintf_s is abnormal. (i=%d; dev_manager_info->dev_id[i]=%u)\n", i,
            dev_manager_info->dev_id[i]);
        return 0;
    }
    wlen += tlen;
    tlen = sprintf_s(str + wlen, PAGE_SIZE - wlen, "sclid = %u\n", pdata->platform_info.sclid);
    if (tlen < 0) {
        devdrv_drv_info("sprintf_s is abnormal. (sclid=%u)\n", pdata->platform_info.sclid);
        return 0;
    }
    wlen += tlen;
    tlen = sprintf_s(str + wlen, PAGE_SIZE - wlen, "ts_cluster = %u\n", pdata->platform_info.ts_cluster);
    if (tlen < 0) {
        devdrv_drv_info("sprintf_s is abnormal. (ts_cluster=%u)\n", pdata->platform_info.ts_cluster);
        return 0;
    }
    wlen += tlen;
    tlen = sprintf_s(str + wlen, PAGE_SIZE - wlen, "ccpu cluster = %u\n", pdata->platform_info.ccpu_cluster);
    if (tlen < 0) {
        devdrv_drv_info("sprintf_s is abnormal. (ccpu_cluster=%u)\n", pdata->platform_info.ccpu_cluster);
        return 0;
    }
    wlen += tlen;
    tlen = sprintf_s(str + wlen, PAGE_SIZE - wlen, "aicpu_cluster = %u\n", pdata->platform_info.aicpu_cluster);
    if (tlen < 0) {
        devdrv_drv_info("sprintf_s is abnormal. (aicpu_cluster=%u)\n", pdata->platform_info.aicpu_cluster);
        return 0;
    }
    wlen += tlen;

    return wlen;
}

STATIC ssize_t devdrv_interrups_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct devdrv_platform_data *pdata = NULL;
    struct devdrv_info *dev_info = NULL;
    char *str = buf;
    int i;
    int ret;

    for (i = 0; i < MAX_CHIP_NUM; i++) {
        dev_info = devdrv_manager_get_devdrv_info(i);
        if (dev_info == NULL) {
            devdrv_drv_err("dev_info is NULL. (chip_num=%d)\n", i);
            continue;
        }

        pdata = (struct devdrv_platform_data *)dev_info->pdata;
        if (pdata == NULL) {
            devdrv_drv_err("dev_info->drv_info is NULL. (chip_num=%d)\n", i);
            return 0;
        }

        ret = devdrv_interrups_show_refill_buf(pdata, str, i);
        if (!ret) {
            devdrv_drv_info("sprintf_s is abnormal. (chip_num=%d)\n", i);
            return 0;
        }
        str += ret;
    }
    return str - buf;
}
DEVDRV_ATTR_RO(devdrv_interrups);

STATIC ssize_t devdrv_ssh_config_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    unsigned char ssh_status[UC_SSH_STATUS_SIZE] = {0};
    unsigned int ssh_size = UC_SSH_STATUS_SIZE;
    int index;
    int tlen;
    int ret;

    index = devdrv_get_config_index_by_name(SSH_CONFIG_NAME);
    if (index < 0) {
        tlen = snprintf_s(buf, SHOW_CONFIG_BUF_SIZE, SHOW_CONFIG_BUF_SIZE - 1, "ssh:can't support this boardId %d\n",
            index);
        devdrv_drv_warn("ssh:can't support this boardId %d\n", index);
        if (tlen <= 0) {
            devdrv_drv_err("ssh config show snprintf fail.\n");
            return 0;
        }
        return tlen;
    }

    ret = devdrv_get_user_config_core(index, ssh_status, &ssh_size);
    if (ret != 0) {
        tlen = snprintf_s(buf, SHOW_CONFIG_BUF_SIZE, SHOW_CONFIG_BUF_SIZE - 1, "ssh:get user config fail:%d\n", ret);
        devdrv_drv_err("ssh:get ssh config fail:%d\n", ret);
        if (tlen <= 0) {
            devdrv_drv_err("ssh config show snprintf fail.\n");
            return 0;
        }
        return tlen;
    }

    if (*ssh_status == SSH_DISENABLE) {
        tlen = snprintf_s(buf, SHOW_CONFIG_BUF_SIZE, SHOW_CONFIG_BUF_SIZE - 1, "ssh-off\n");
    } else if (*ssh_status == SSH_ENABLE) {
        tlen = snprintf_s(buf, SHOW_CONFIG_BUF_SIZE, SHOW_CONFIG_BUF_SIZE - 1, "ssh-on\n");
    } else {
        tlen = snprintf_s(buf, SHOW_CONFIG_BUF_SIZE, SHOW_CONFIG_BUF_SIZE - 1, "ssh-off\n");
        devdrv_drv_err("ssh config is invalid in flash.\n");
    }
    if (tlen <= 0) {
        devdrv_drv_err("ssh config show snprintf fail.\n");
        return 0;
    }
    return tlen;
}
DEVDRV_ATTR_RO(devdrv_ssh_config);

STATIC ssize_t devdrv_aicpu_config_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    unsigned char aicpu_config[UC_AICPU_CONFIG_SIZE] = {0};
    unsigned int aicpu_size = UC_AICPU_CONFIG_SIZE;
    int index;
    int tlen;
    int ret;

    index = devdrv_get_config_index_by_name(AICPU_CONFIG_NAME);
    if (index < 0) {
        tlen = snprintf_s(buf, SHOW_CONFIG_BUF_SIZE, SHOW_CONFIG_BUF_SIZE - 1, "aicpu_config-invalid\n");
        devdrv_drv_warn("aicpu:can't support this boardId\n");
        if (tlen <= 0) {
            devdrv_drv_err("aicpu config show snprintf fail.\n");
            return 0;
        }
        return tlen;
    }
    ret = devdrv_get_user_config_core(index, aicpu_config, &aicpu_size);
    if (ret != 0) {
        tlen = snprintf_s(buf, SHOW_CONFIG_BUF_SIZE, SHOW_CONFIG_BUF_SIZE - 1, "aicpu_config-invalid\n");
        devdrv_drv_err("aicpu:get ssh config fail:%d\n", ret);
        if (tlen <= 0) {
            devdrv_drv_err("aicpu config show snprintf fail.\n");
            return 0;
        }
        return tlen;
    }
    switch (aicpu_config[0]) {
        case AICPU2_CTRLCPU6_CONFIG:
            tlen = snprintf_s(buf, SHOW_CONFIG_BUF_SIZE, SHOW_CONFIG_BUF_SIZE - 1, "6-7\n");
            break;
        case AICPU4_CTRLCPU4_CONFIG:
            tlen = snprintf_s(buf, SHOW_CONFIG_BUF_SIZE, SHOW_CONFIG_BUF_SIZE - 1, "4-7\n");
            break;
        case AICPU6_CTRLCPU2_CONFIG:
            tlen = snprintf_s(buf, SHOW_CONFIG_BUF_SIZE, SHOW_CONFIG_BUF_SIZE - 1, "2-7\n");
            break;
        default:
            tlen = snprintf_s(buf, SHOW_CONFIG_BUF_SIZE, SHOW_CONFIG_BUF_SIZE - 1, "aicpu_config-invalid\n");
            devdrv_drv_err("the aicpu config is invalid in flash, aicpu config:%u\n", aicpu_config[0]);
            break;
    }
    if (tlen <= 0) {
        devdrv_drv_err("aicpu config show snprintf fail.\n");
        return 0;
    }
    return tlen;
}
DEVDRV_ATTR_RO(devdrv_aicpu_config);

STATIC ssize_t devdrv_ctrlcpu_config_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    unsigned char aicpu_config[UC_AICPU_CONFIG_SIZE] = {0};
    unsigned int aicpu_size = UC_AICPU_CONFIG_SIZE;
    int index;
    int tlen;
    int ret;

    index = devdrv_get_config_index_by_name(AICPU_CONFIG_NAME);
    if (index < 0) {
        tlen = snprintf_s(buf, SHOW_CONFIG_BUF_SIZE, SHOW_CONFIG_BUF_SIZE - 1, "ctrlcpu_config-invalid\n");
        devdrv_drv_warn("ctrlcpu:can't support this boardId\n");
        if (tlen <= 0) {
            devdrv_drv_err("ctrlcpu config show snprintf fail.\n");
            return 0;
        }
        return tlen;
    }
    ret = devdrv_get_user_config_core(index, aicpu_config, &aicpu_size);
    if (ret != 0) {
        tlen = snprintf_s(buf, SHOW_CONFIG_BUF_SIZE, SHOW_CONFIG_BUF_SIZE - 1, "ctrlcpu_config-invalid\n");
        devdrv_drv_err("ctrlcpu:get user config fail:%d\n", ret);
        if (tlen <= 0) {
            devdrv_drv_err("ctrlcpu config show snprintf fail.\n");
            return 0;
        }
        return tlen;
    }
    switch (aicpu_config[0]) {
        case AICPU2_CTRLCPU6_CONFIG:
            tlen = snprintf_s(buf, SHOW_CONFIG_BUF_SIZE, SHOW_CONFIG_BUF_SIZE - 1, "0-5\n");
            break;
        case AICPU4_CTRLCPU4_CONFIG:
            tlen = snprintf_s(buf, SHOW_CONFIG_BUF_SIZE, SHOW_CONFIG_BUF_SIZE - 1, "0-3\n");
            break;
        case AICPU6_CTRLCPU2_CONFIG:
            tlen = snprintf_s(buf, SHOW_CONFIG_BUF_SIZE, SHOW_CONFIG_BUF_SIZE - 1, "0-1\n");
            break;
        default:
            tlen = snprintf_s(buf, SHOW_CONFIG_BUF_SIZE, SHOW_CONFIG_BUF_SIZE - 1, "ctrlcpu_config-invalid\n");
            devdrv_drv_err("the aicpu config is invalid in flash, aicpu config:%u\n", aicpu_config[0]);
            break;
    }
    if (tlen <= 0) {
        devdrv_drv_err("ctrlcpu config show snprintf fail.\n");
        return 0;
    }
    return tlen;
}
DEVDRV_ATTR_RO(devdrv_ctrlcpu_config);

static struct attribute *devdrv_manager_attrs[] = {
    &devdrv_resources_attr.attr,
    &devdrv_interrups_attr.attr,
    &devdrv_ssh_config_attr.attr,
    &devdrv_aicpu_config_attr.attr,
    &devdrv_ctrlcpu_config_attr.attr,
    NULL,
};

static struct attribute_group devdrv_manager_attr_group = {
    .attrs = devdrv_manager_attrs,
    .name = "devdrv_manager",
};

#endif /* CONFIG_SYSFS */

struct tsdrv_drv_ops* devdrv_manager_get_drv_ops(void)
{
    return &devdrv_platform_drv_ops;
}
EXPORT_SYMBOL(devdrv_manager_get_drv_ops);

/*lint -e454 */
void devdrv_mn_mutex_lock(int dev_id)
{
    if (dev_id < 0 || dev_id >= DEVICE_ID_MAX) {
        devdrv_drv_err("devdrv_mn_mutex_lock id error.\n");
    }

    switch (dev_id) {
        case DEVICE_ID0:
            mutex_lock(&devdrv_mn_mutex_dev0);
            break;
        case DEVICE_ID1:
            mutex_lock(&devdrv_mn_mutex_dev1);
            break;
        case DEVICE_ID2:
            mutex_lock(&devdrv_mn_mutex_dev2);
            break;
        case DEVICE_ID3:
            mutex_lock(&devdrv_mn_mutex_dev3);
            break;
    }
}
EXPORT_SYMBOL(devdrv_mn_mutex_lock);
/*lint +e454 */
/*lint -e455 */
void devdrv_mn_mutex_unlock(int dev_id)
{
    if (dev_id < 0 || dev_id >= DEVICE_ID_MAX) {
        devdrv_drv_err("devdrv_mn_mutex_unlock id error.\n");
    }

    switch (dev_id) {
        case DEVICE_ID0:
            mutex_unlock(&devdrv_mn_mutex_dev0);
            break;
        case DEVICE_ID1:
            mutex_unlock(&devdrv_mn_mutex_dev1);
            break;
        case DEVICE_ID2:
            mutex_unlock(&devdrv_mn_mutex_dev2);
            break;
        case DEVICE_ID3:
            mutex_unlock(&devdrv_mn_mutex_dev3);
            break;
    }
}
EXPORT_SYMBOL(devdrv_mn_mutex_unlock);
/*lint +e455 */
int copy_from_user_safe(void *to, const void __user *from, unsigned long n)
{
    if (from == NULL || to == NULL) {
        devdrv_drv_err("user pointer is NULL.\n");
        return -EINVAL;
    }

    if (n <= 0) {
        devdrv_drv_err("n is zero.\n");
        return -EINVAL;
    }

    if (copy_from_user(to, (void *)from, n))
        return -ENODEV;

    return 0;
}
EXPORT_SYMBOL(copy_from_user_safe);

int copy_to_user_safe(void __user *to, const void *from, unsigned long n)
{
    if (from == NULL || to == NULL) {
        devdrv_drv_err("user pointer is NULL.\n");
        return -EINVAL;
    }

    if (n <= 0) {
        devdrv_drv_err("n is zero.\n");
        return -EINVAL;
    }

    if (copy_to_user(to, (void *)from, n))
        return -ENODEV;

    return 0;
}
EXPORT_SYMBOL(copy_to_user_safe);

int devdrv_get_pcie_id_info(u32 devid, struct dmanage_pcie_id_info *pcie_id_info)
{
    struct devdrv_manager_msg_info dev_manager_msg_info = { { 0 }, { 0 } };
    struct dmanage_pcie_id_info *host_pcie_id_info = NULL;
    u32 out_len;
    int ret;

    if (pcie_id_info == NULL) {
        devdrv_drv_err("invalid input handler.\n");
        return -EINVAL;
    }

    dev_manager_msg_info.header.msg_id = DEVDRV_MANAGER_CHAN_D2H_GET_PCIE_ID_INFO;
    dev_manager_msg_info.header.valid = (u16)DEVDRV_MANAGER_MSG_VALID;
    dev_manager_msg_info.header.result = (u16)DEVDRV_MANAGER_MSG_INVALID_RESULT;

    host_pcie_id_info = (struct dmanage_pcie_id_info *)dev_manager_msg_info.payload;
    host_pcie_id_info->venderid = 0;
    host_pcie_id_info->subvenderid = 0;
    host_pcie_id_info->deviceid = 0;
    host_pcie_id_info->subdeviceid = 0;
    host_pcie_id_info->bus = 0;
    host_pcie_id_info->device = 0;
    host_pcie_id_info->fn = 0;

    ret = agentdrv_common_msg_send(devid, &dev_manager_msg_info, sizeof(dev_manager_msg_info),
        sizeof(dev_manager_msg_info), &out_len, AGENTDRV_COMMON_MSG_DEVDRV_MANAGER);
    if ((ret) || dev_manager_msg_info.header.result != 0) {
        devdrv_drv_err("there is no host or host driver doesn't insmod\n");
        return -ENODEV;
    }

    host_pcie_id_info = (struct dmanage_pcie_id_info *)dev_manager_msg_info.payload;

    pcie_id_info->venderid = host_pcie_id_info->venderid;
    pcie_id_info->subvenderid = host_pcie_id_info->subvenderid;
    pcie_id_info->deviceid = host_pcie_id_info->deviceid;
    pcie_id_info->subdeviceid = host_pcie_id_info->subdeviceid;
    pcie_id_info->bus = host_pcie_id_info->bus;
    pcie_id_info->device = host_pcie_id_info->device;
    pcie_id_info->fn = host_pcie_id_info->fn;

    return 0;
}

STATIC void devdrv_tscpu_free_memory(struct devdrv_info *dev_info, struct device *dev, int size, void *addr,
    dma_addr_t ts_dma_handle)
{
    devdrv_free_tsfw_mem(dev, size, addr, ts_dma_handle);
}

int devdrv_get_platformInfo(u32 *info)
{
    if (info == NULL)
        return -EINVAL;
    *info = DEVDRV_MANAGER_DEVICE_ENV;

    return 0;
}
EXPORT_SYMBOL(devdrv_get_platformInfo);

#ifdef CONFIG_ACPI
int devdrv_get_l2_buffer(u32 devid, u64 *base, u64 *len)
{
    struct devdrv_info *dev_info = NULL;
    u32 l2buff_size;
    int err;

    if (base == NULL || len == NULL || (devid >= DEVDRV_MAX_DAVINCI_NUM)) {
        devdrv_drv_err("invalid input handler.\n");
        return -EINVAL;
    }

    dev_info = dev_manager_info->dev_info[devid];
    if (dev_info == NULL) {
        devdrv_drv_err("device  does not exist, devid = %u.\n", devid);
        return -EINVAL;
    }

    err = device_property_read_u64(dev_info->dev, "l2buffer-region", base);
    if (err) {
        devdrv_drv_err("device_property_read_u64 failed, devid = %u.\n", devid);
        return -1;
    }

    err = device_property_read_u32(dev_info->dev, "l2buffer-size", &l2buff_size);
    if (err) {
        devdrv_drv_err("of_address_to_resource failed, devid = %u.\n", devid);
        return -1;
    }

    *len = l2buff_size;
    devdrv_drv_debug("L2 buffer base addr, size: 0x%llx.\n", *len);

    return 0;
}

#else
int devdrv_get_l2_buffer(u32 devid, u64 *base, u64 *len)
{
    struct devdrv_info *dev_info = NULL;
    struct device_node *l2buff = NULL;
    struct resource r;
    int err;

    if (base == NULL || len == NULL || (devid >= DEVDRV_MAX_DAVINCI_NUM)) {
        devdrv_drv_err("invalid input handler.\n");
        return -EINVAL;
    }

    dev_info = dev_manager_info->dev_info[devid];
    if (dev_info == NULL) {
        devdrv_drv_err("device does not exist, devid = %u.\n", devid);
        return -EINVAL;
    }

    l2buff = of_parse_phandle(dev_info->dev->of_node, "l2buffer-region", 0);
    if (l2buff == NULL) {
        devdrv_drv_err("of_parse_phandle failed, devid = %u.\n", devid);
        return -1;
    }

    err = of_address_to_resource(l2buff, 0, &r);
    if (err) {
        devdrv_drv_err("of_address_to_resource failed, devid = %u.\n", devid);
        return -1;
    }

    *base = r.start;
    *len = resource_size(&r);

    devdrv_drv_debug("L2 buffer base addr: %pK, size: 0x%llx.\n", (void *)*base, *len);

    return 0;
}
#endif
EXPORT_SYMBOL(devdrv_get_l2_buffer);

int devdrv_get_devnum(u32 *num_dev)
{
    u32 dev_num;

    if (num_dev == NULL)
        return -EINVAL;

    dev_num = devdrv_manager_get_devnum();
    if (dev_num == 0 || dev_num > DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("invalid device number, dev_num = %d\n", dev_num);
        return -EFAULT;
    }
    *num_dev = dev_num;

    return 0;
}
EXPORT_SYMBOL(devdrv_get_devnum);

u32 devdrv_manager_get_devid(u32 local_devid)
{
    int retry_cnt = 0;
    u32 cnt_max;
    u32 dev_id;

    if ((dev_manager_info == NULL) || (local_devid >= DEVDRV_MAX_DAVINCI_NUM)) {
        devdrv_drv_err("invalid param, local_devid = %u, dev_manager_info = %pK.\n", local_devid, dev_manager_info);
        return DEVDRV_MAX_DAVINCI_NUM;
    }

    /* rc mode isn't need get devid,return directly */
    if (dev_manager_info->machine_mode == DEVDRV_PCIE_RC_MODE) {
        return DEVDRV_MAX_DAVINCI_NUM;
    }

    /*
     * wait the devid that host send to device
     * if the waiting time exceeds (DEVDRV_WAIT_TIME_HOST_DEVID)ms, report error and return.
     */
    cnt_max = DEVDRV_WAIT_TIME_DEVID / DEVDRV_WAIT_TIME_DEVID_ONCE;
    while (!dev_manager_info->dev_id_flag[local_devid]) {
        retry_cnt++;
        if (retry_cnt > cnt_max) {
            devdrv_drv_err("get devId from host failed, because the dev_id_flag[%u]=%u, retry_cnt=%d.\n",
                local_devid, dev_manager_info->dev_id_flag[local_devid], retry_cnt);
            return DEVDRV_MAX_DAVINCI_NUM;
        }
        msleep(DEVDRV_WAIT_TIME_DEVID_ONCE);
    }
    dev_id = dev_manager_info->dev_id[local_devid];

    return dev_id;
}
EXPORT_SYMBOL(devdrv_manager_get_devid);

u32 devdrv_manager_get_devid_flag(u32 local_devid)
{
    unsigned long flags;
    u32 devid_flag;

    if ((dev_manager_info == NULL) || (local_devid >= DEVDRV_MAX_DAVINCI_NUM)) {
        devdrv_drv_err("invalid param, local_devid = %u, dev_manager_info = %pK.\n", local_devid, dev_manager_info);
        return 0;
    }

    /* get device id flag set by host, default dev_id is 0 if there is no host */
    spin_lock_irqsave(&dev_manager_info->spinlock, flags);
    devid_flag = dev_manager_info->dev_id_flag[local_devid];
    spin_unlock_irqrestore(&dev_manager_info->spinlock, flags);

    return devid_flag;
}
EXPORT_SYMBOL(devdrv_manager_get_devid_flag);

/* ensure devid is valid and the device is initialized */
int devdrv_manager_devid_to_nid(u32 devid, u32 mem_type)
{
    int nid = devid;
    struct devdrv_info *dev_info = NULL;

    if (devid >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("invalid param, devid = %u, return the default numa_id: %u.\n", devid, nid);
        return NUMA_NO_NODE;
    }

    dev_info = devdrv_manager_get_devdrv_info(devid);
    if (dev_info == NULL) {
        devdrv_drv_err("device(%u) is not initialized, return the default numa_id: %u.\n", devid, nid);
        return NUMA_NO_NODE;
    }
    if (dev_info->ts_mem_restrict_valid == TS_MEM_RESTRICT_VALID) {
        nid = DEVDRV_TS_NODE_DDR_ID_OFFSET + devid;
    }

    return nid;
}
EXPORT_SYMBOL(devdrv_manager_devid_to_nid);

u32 devdrv_manager_get_devnum(void)
{
    if (dev_manager_info == NULL)
        return (DEVDRV_MAX_DAVINCI_NUM + 1);

    return dev_manager_info->num_dev;
}

int devdrv_get_devids(u32 *devices, u32 device_num)
{
    u32 num_dev;
    u8 i;
    u8 j = 0;

    if (devices == NULL)
        return -EINVAL;

    num_dev = devdrv_manager_get_devnum();
    if (num_dev == 0 || num_dev > DEVDRV_MAX_DAVINCI_NUM || num_dev > device_num) {
        devdrv_drv_err("wrong device number, num_dev = %u input len : %u \n", num_dev, device_num);
        return -EINVAL;
    }

    /* get device id assigned from host, default dev_id is 0 if there is no host */
    for (i = 0; i < DEVDRV_MAX_DAVINCI_NUM; i++) {
        if (dev_manager_info->dev_id[i] == INVAILD_DEVICE_ID)
            continue;
        if (j >= device_num)
            break;
        devices[j] = dev_manager_info->dev_id[i];
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
        devdrv_drv_err("invalid parameter, dev_id = %d.\n", devid);
        return -EINVAL;
    }

    dev_info = devdrv_manager_get_devdrv_info(devid);
    if (dev_info == NULL) {
        devdrv_drv_err("device manager is not initialized, dev_id = %d\n", devid);
        return -EINVAL;
    }

    info->ai_core_num = dev_info->ai_core_num;
    info->ai_cpu_core_num = dev_info->ai_cpu_core_num;
    info->ctrl_cpu_core_num = dev_info->ctrl_cpu_core_num;
    info->ctrl_cpu_occupy_bitmap = dev_info->ctrl_cpu_occupy_bitmap;

    info->ai_cpu_core_id = dev_info->ai_cpu_core_id; //
    info->ai_core_id = dev_info->ai_core_id;         //

    /* 1:little endian 0:big endian */
    info->ctrl_cpu_endian_little = dev_info->ctrl_cpu_endian_little;
    info->ctrl_cpu_id = dev_info->ctrl_cpu_id;
    info->ctrl_cpu_ip = dev_info->ctrl_cpu_ip;
    info->ts_cpu_core_num = dev_info->pdata->ts_pdata[0].ts_cpu_core_num;
    info->aicpu_occupy_bitmap = dev_info->aicpu_occupy_bitmap;
    info->env_type = dev_info->env_type;

    return 0;
}
EXPORT_SYMBOL(devdrv_get_devinfo);

int devdrv_get_core_inuse(u32 devid, u32 vfid, struct devdrv_hardware_inuse *inuse)
{
    struct devdrv_info *dev_info = NULL;

    if (inuse == NULL || devid >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("invalid parameter, dev_id = %d\n", devid);
        return -EINVAL;
    }

    if (dev_manager_info == NULL || dev_manager_info->dev_info[devid] == NULL) {
        devdrv_drv_err("device manager is not initialized dev_id = %d.\n", devid);
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
EXPORT_SYMBOL(devdrv_get_core_spec); //lint !e508

struct devdrv_info *devdrv_manager_get_devdrv_info(u32 dev_id)
{
    struct devdrv_info *dev_info = NULL;
    unsigned long flags;

    if (dev_manager_info == NULL || dev_id >= DEVDRV_MAX_DAVINCI_NUM)
        return NULL;

    spin_lock_irqsave(&dev_manager_info->spinlock, flags);
    dev_info = dev_manager_info->dev_info[dev_id];
    spin_unlock_irqrestore(&dev_manager_info->spinlock, flags);

    return dev_info;
}
EXPORT_SYMBOL(devdrv_manager_get_devdrv_info); //lint !e508

struct devdrv_info *devdrv_get_default_devdrv_info(void)
{
    if (dev_manager_info == NULL)
        return NULL;

    return dev_manager_info->dev_info[CHIP0_ID];
}

struct devdrv_manager_info *devdrv_get_manager_info(void)
{
    return dev_manager_info;
}
EXPORT_SYMBOL(devdrv_get_manager_info);

STATIC int devdrv_check_process_sign(pid_t hostpid, const char *sign, u32 len)
{
    struct devdrv_manager_msg_info dev_manager_msg_info = { { 0 }, { 0 } };
    struct process_sign *process_sign = NULL;
    u32 out_len;
    u32 dev_id;
    int ret;

    dev_manager_msg_info.header.msg_id = DEVDRV_MANAGER_CHAN_D2H_CHECK_PROCESS_SIGN;
    dev_manager_msg_info.header.valid = (u16)DEVDRV_MANAGER_MSG_VALID;
    dev_manager_msg_info.header.result = (u16)DEVDRV_MANAGER_MSG_INVALID_RESULT;

    process_sign = (struct process_sign *)dev_manager_msg_info.payload;
    process_sign->tgid = hostpid;

    ret = strcpy_s(process_sign->sign, PROCESS_SIGN_LENGTH, sign);
    if (ret) {
        devdrv_drv_err("strcpy_s failed, ret(%d).\n", ret);
        return -EINVAL;
    }
    dev_id = cpu_to_node((int)smp_processor_id()); /*lint !e666  !e453*/
    devdrv_drv_info("hostpid(%d) devid(%u).\n", hostpid, dev_id);

    ret = agentdrv_common_msg_send(dev_id, &dev_manager_msg_info, sizeof(dev_manager_msg_info),
        sizeof(dev_manager_msg_info), &out_len, AGENTDRV_COMMON_MSG_DEVDRV_MANAGER);
    if (ret) {
        devdrv_drv_err("common msg send failed, ret(%d)\n", ret);
        return -ENODEV;
    }
    if (dev_manager_msg_info.header.result != 0) {
        devdrv_drv_err("check process_sign failed, result(%u)\n", dev_manager_msg_info.header.result);
        return -ENODEV;
    }

    return 0;
}
struct devdrv_process_sign *devdrv_find_process_sign(struct devdrv_manager_info *d_info,
    pid_t hostpid)
{
    struct devdrv_process_sign *proc_sign = NULL;
    u32 key;

    key = (u32)hostpid & DEVDRV_PROC_HASH_TABLE_MASK;
    /*lint -e666 */
    hash_for_each_possible(d_info->proc_hash_table, proc_sign, link, key) {
        if (proc_sign->hostpid == hostpid) {
            return proc_sign;
        }
    }
    return NULL;
}

STATIC struct devdrv_process_sign *devdrv_create_process_sign(struct devdrv_manager_info *d_info,
    struct devdrv_ioctl_para_bind_host_pid para_info)
{
    struct devdrv_process_sign *d_sign = NULL;
    int ret;
    enum devdrv_process_type k;
    int i, j;

    d_sign = kzalloc(sizeof(struct devdrv_process_sign), GFP_ATOMIC | __GFP_ACCOUNT);
    if (d_sign == NULL) {
        return NULL;
    }
    d_sign->hostpid = para_info.host_pid;
    d_sign->cp_count = 0;
    for (i = 0; i < DEVDRV_MAX_NODE_NUM; i++) {
        for (j = 0; j < VFID_NUM_MAX; j++) {
            for (k = DEVDRV_PROCESS_CP1; k < DEVDRV_PROCESS_CPTYPE_MAX; k++) {
                d_sign->devpid[i][j][k] = DEVMNG_PID_INVALID;
            }
        }
    }
    ret = strcpy_s(d_sign->sign, PROCESS_SIGN_LENGTH, para_info.sign);
    if (ret) {
        kfree(d_sign);
        d_sign = NULL;
        return NULL;
    }
    return d_sign;
}

STATIC inline int devdrv_check_and_bind_hostpid(struct devdrv_process_sign *d_sign,
    struct devdrv_ioctl_para_bind_host_pid para_info, pid_t devpid, pid_t *bound_devpid)
{
    if (d_sign->devpid[para_info.chip_id][para_info.vfid][para_info.cp_type] == DEVMNG_PID_INVALID ||
        d_sign->devpid[para_info.chip_id][para_info.vfid][para_info.cp_type] == DEVMNG_PID_START_ONCE) {
        if ((para_info.cp_type == DEVDRV_PROCESS_CP2) &&
            (d_sign->devpid[para_info.chip_id][para_info.vfid][DEVDRV_PROCESS_CP1] == DEVMNG_PID_INVALID)) {
            return -EPERM;
        }
        d_sign->devpid[para_info.chip_id][para_info.vfid][para_info.cp_type] = devpid;
        d_sign->cp_count += (para_info.cp_type == DEVDRV_PROCESS_CP1 ? 1 : 0);
        d_sign->cp_count += (para_info.cp_type == DEVDRV_PROCESS_DEV_ONLY ? 1 : 0);
        d_sign->cp_count += (para_info.cp_type == DEVDRV_PROCESS_QS ? 1 : 0);
        d_sign->cp_count += (para_info.cp_type == DEVDRV_PROCESS_HCCP ? 1 : 0);
        d_sign->host_process_status = 0;
        return 0;
    }
    *bound_devpid = d_sign->devpid[para_info.chip_id][para_info.vfid][para_info.cp_type];
    d_sign->host_process_status = 0;
    return -EINVAL;
}

STATIC int devdrv_verify_sign(char *sign, int mode, pid_t host_pid, pid_t *dev_pid)
{
    int ret;
    pid_t pid_tmp = -1;
#ifndef CFG_BUILD_DEBUG
    const char *process_name[WHITE_LIST_PROCESS_NUM_FOR_BIND_PID] = {PROCESS_NAME_TSD};
#endif

    /* sign is used as dev_pid. */
    ret = memcpy_s(&pid_tmp, sizeof(pid_t), sign, sizeof(pid_t));
    if (ret != 0 || pid_tmp == -1) {
        devdrv_drv_err("Failed to get the dev_pid. (ret=%d; pid=%d)\n", ret, pid_tmp);
        return -EINVAL;
    }

    /* Compatible with early HIVA scenarios and RC mode */
    if ((mode == AICPUFW_OFFLINE_PLAT) && (host_pid == current->tgid)) {
        *dev_pid = current->tgid;
        return 0;
    } else {
#ifndef CFG_BUILD_DEBUG
        ret = whitelist_process_handler(process_name, WHITE_LIST_PROCESS_NUM_FOR_BIND_PID);
        if (ret != 0) {
            devdrv_drv_err("whitelist_process_handler error or Invalid parameter."
                " (ret=%d; dev_pid=%d)\n", ret, pid_tmp);
            return ret;
        }
#endif
        *dev_pid = pid_tmp;
    }

    return 0;
}
#define MAX_BIND_WAIT_TIMES  300  /* max wait time 3s */
#define MAX_BIND_WAIT_ONCE   10
int devdrv_wait_pid_init(pid_t proc_pid, int mode)
{
    u32 times = 0;
    struct pid *pid_proc = NULL;
    if (mode == AICPUFW_OFFLINE_PLAT) {
        return 0;
    }
    do {
        times++;
        /* check process alive */
        pid_proc = find_get_pid(proc_pid);
        if (pid_proc == NULL) {
            return -ESRCH;
        }
        put_pid(pid_proc);

        /* check process init status */
        if (ascend_intf_is_pid_init(proc_pid, DAVINCI_INTF_MODULE_DEVMNG)) {
            return 0;
        }
        (void)msleep(MAX_BIND_WAIT_ONCE);
    } while (times < MAX_BIND_WAIT_TIMES);

    return -ETIME;
}

STATIC int devdrv_bind_hostpid(struct devdrv_ioctl_para_bind_host_pid para_info)
{
    struct devdrv_manager_info *d_info = devdrv_get_manager_info();
    struct devdrv_process_sign *d_sign = NULL;
    struct devdrv_process_sign *d_sign_create = NULL;
    u32 key;
    int ret;
    pid_t bound_devpid = -1;
    pid_t dev_pid = -1;

    if ((para_info.len != PROCESS_SIGN_LENGTH) || (d_info == NULL) ||
        (para_info.mode >= AICPUFW_MAX_PLAT) || (para_info.cp_type < 0) ||
        (para_info.cp_type >= DEVDRV_PROCESS_CPTYPE_MAX) ||
        (para_info.chip_id >= DEVDRV_MAX_NODE_NUM) || (para_info.vfid >= VFID_NUM_MAX)) {
        devdrv_drv_err("invalid para length(%u) cp_type(%d) chip_id(%u) hostpid(%d) vfid(%u).\n",
            para_info.len, para_info.cp_type, para_info.chip_id, para_info.host_pid, para_info.vfid);
        return -EINVAL;
    }

    if ((para_info.mode == AICPUFW_ONLINE_PLAT) && (para_info.vfid == 0)) {
        ret = devdrv_check_process_sign(para_info.host_pid, para_info.sign, para_info.len);
        if (ret) {
            devdrv_drv_err("d2h check sign failed, ret(%d).\n", ret);
            return -EINVAL;
        }
    }

    ret = devdrv_verify_sign(para_info.sign, para_info.mode, para_info.host_pid, &dev_pid);
    if (ret != 0) {
        devdrv_drv_err("Failed to get the dev_pid. (ret=%d; dev_pid=%d)\n", ret, dev_pid);
        return -EINVAL;
    }
#ifndef CFG_HOST_ENV
    ret = devdrv_wait_pid_init(dev_pid, para_info.mode);
    if (ret != 0) {
        devdrv_drv_err("wait process init failed. (ret=%d; dev_pid=%d)\n", ret, dev_pid);
        return -EINVAL;
    }
#endif
    key = (u32)para_info.host_pid & DEVDRV_PROC_HASH_TABLE_MASK;
    d_sign_create = devdrv_create_process_sign(d_info, para_info);
    if (d_sign_create == NULL) {
        devdrv_drv_err("Create sign failed or invalid sign. (chip_id=%u; hostpid=%d)\n",
            para_info.chip_id, para_info.host_pid);
        return -EINVAL;
    }

    spin_lock_bh(&d_info->proc_hash_table_lock);
    d_sign = devdrv_find_process_sign(d_info, para_info.host_pid);
    if (d_sign != NULL) {
        ret = devdrv_check_and_bind_hostpid(d_sign, para_info, dev_pid, &bound_devpid);
        spin_unlock_bh(&d_info->proc_hash_table_lock);
        kfree(d_sign_create);
        d_sign_create = NULL;
        if (ret) {
            devdrv_drv_err("Bind failed or already bound another process. "
                "(ret=%d; host_pid=%d; cp_type=%d; bound_devpid=%d; dev_pid=%d; dev_id=%u; vfid=%u).\n",
                ret, para_info.host_pid, para_info.cp_type, bound_devpid, dev_pid, para_info.chip_id, para_info.vfid);
            return -EINVAL;
        }

        goto bind_succ;
    }

    ret = devdrv_check_and_bind_hostpid(d_sign_create, para_info, dev_pid, &bound_devpid);
    if (ret) {
        spin_unlock_bh(&d_info->proc_hash_table_lock);
        devdrv_drv_err("Bind failed or already bound another process. "
            "(ret=%d; host_pid=%d; cp_type=%d; bound_devpid=%d; dev_pid=%d; dev_id=%u; vfid=%u).\n",
            ret, para_info.host_pid, para_info.cp_type, bound_devpid, dev_pid, para_info.chip_id, para_info.vfid);
        kfree(d_sign_create);
        d_sign_create = NULL;
        return -EINVAL;
    }
    hash_add(d_info->proc_hash_table, &d_sign_create->link, key); //lint !e666
    spin_unlock_bh(&d_info->proc_hash_table_lock);

bind_succ:
    devdrv_drv_info("Bind pid success. (host_pid=%d; dev_pid=%d; cp_type=%d; dev_id=%u; vfid=%u; mode=%d)\n",
        para_info.host_pid, dev_pid, para_info.cp_type, para_info.chip_id, para_info.vfid, para_info.mode);
    return 0;
}


int devdrv_fop_bind_host_pid(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_ioctl_para_bind_host_pid para_info = {0};
    int node_id = numa_node_id();
    int ret;

    if (copy_from_user_safe(&para_info, (void *)(uintptr_t)arg, sizeof(struct devdrv_ioctl_para_bind_host_pid))) {
        devdrv_drv_err("copy_from_user error. dev_id:%u\n", node_id);
        return -EINVAL;
    }

    para_info.sign[DEVDRV_SIGN_LEN - 1] = '\0';

    ret = devdrv_bind_hostpid(para_info);
    if (ret) {
        devdrv_drv_err("bind_hostpid error. dev_id:%u, ret:%d, host_pid:%d, cp_type:%d\n", node_id, ret,
            para_info.host_pid, para_info.cp_type);
        return ret;
    }

    return 0;
}

int devdrv_query_process_by_host_pid(unsigned int host_pid,
    unsigned int chip_id, enum devdrv_process_type cp_type, unsigned int vfid, int *pid)
{
    struct devdrv_manager_info *d_info = devdrv_get_manager_info();
    struct devdrv_process_sign *d_sign = NULL;
    u32 key;

    if ((d_info == NULL) || (chip_id >= DEVDRV_MAX_NODE_NUM) || (pid == NULL) ||
        (cp_type >= DEVDRV_PROCESS_CPTYPE_MAX) || (vfid >= VFID_NUM_MAX)) {
        devdrv_drv_err("host_pid %d null ptr or invalid cp_type(%d) chip_id(%u) vfid(%u).\n",
            host_pid, cp_type, chip_id, vfid);
        return -EINVAL;
    }

    key = (u32)host_pid & DEVDRV_PROC_HASH_TABLE_MASK;
    spin_lock_bh(&d_info->proc_hash_table_lock);
    /*lint -e666 */
    hash_for_each_possible(d_info->proc_hash_table, d_sign, link, key)
    {
        if (d_sign->hostpid == host_pid) {
            *pid = d_sign->devpid[chip_id][vfid][cp_type];
            if ((cp_type == DEVDRV_PROCESS_CP2) && (*pid == DEVMNG_PID_INVALID)) {
                *pid = d_sign->devpid[chip_id][vfid][DEVDRV_PROCESS_CP1];
            }
            spin_unlock_bh(&d_info->proc_hash_table_lock);
            if (*pid == DEVMNG_PID_INVALID) {
                devdrv_drv_warn("The hostpid didn't bind this type device pid."
                    "(hostpid=%u, device id=%u, vfid=%u, cptype=%u)\n", host_pid, chip_id, vfid, cp_type);
                return DRV_ERROR_NO_PROCESS;
            }
            return 0;
        }
    }
    spin_unlock_bh(&d_info->proc_hash_table_lock);
    devdrv_drv_warn("Cannot find the host pid in hashtable, (host_pid=%u, cp_type=%u, chip_id=%u, vfid=%u)\n",
        host_pid, cp_type, chip_id, vfid);
    return DRV_ERROR_NO_PROCESS;
}
EXPORT_SYMBOL(devdrv_query_process_by_host_pid);

int devdrv_query_devpid(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    int node_id = numa_node_id();
    struct devdrv_ioctl_para_query_pid para_info = {0};

    if (copy_from_user_safe(&para_info, (void *)(uintptr_t)arg, sizeof(struct devdrv_ioctl_para_query_pid))) {
        devdrv_drv_err("copy_from_user error. dev_id:%u\n", node_id);
        return -EINVAL;
    }

    ret = devdrv_query_process_by_host_pid(para_info.host_pid, para_info.chip_id,
                                           para_info.cp_type, para_info.vfid, &(para_info.pid));
    if (ret) {
        devdrv_drv_warn("query device pid failed, ret(%d).\n", ret);
        return ret;
    }

    if (copy_to_user_safe((void *)((uintptr_t)arg), &para_info, sizeof(struct devdrv_ioctl_para_query_pid))) {
        devdrv_drv_err("copy_to_user error. dev_id:%u\n", node_id);
        return -EINVAL;
    }

    return 0;
}

int devdrv_query_process_host_pid(int pid, unsigned int *chip_id, unsigned int *vfid, unsigned int *host_pid,
    enum devdrv_process_type *cp_type)
{
    struct devdrv_manager_info *d_info = devdrv_get_manager_info();
    struct devdrv_process_sign *d_sign = NULL;
    u32 bkt = 0;
    int i, j;

    if ((chip_id == NULL) || (vfid == NULL) || (host_pid == NULL) || (cp_type == NULL)) {
        devdrv_drv_err("null ptr.\n");
        return -EINVAL;
    }

    spin_lock_bh(&d_info->proc_hash_table_lock);

    hash_for_each(d_info->proc_hash_table, bkt, d_sign, link) {
        for (i = 0; i < DEVDRV_MAX_NODE_NUM; i++) {
            for (j = 0; j < VFID_NUM_MAX; j++) {
                if (d_sign->devpid[i][j][DEVDRV_PROCESS_CP1] == pid)
                    *cp_type = DEVDRV_PROCESS_CP1;
                else if (d_sign->devpid[i][j][DEVDRV_PROCESS_CP2] == pid)
                    *cp_type = DEVDRV_PROCESS_CP2;
                else if (d_sign->devpid[i][j][DEVDRV_PROCESS_QS] == pid)
                    *cp_type = DEVDRV_PROCESS_QS;
                else if (d_sign->devpid[i][j][DEVDRV_PROCESS_DEV_ONLY] == pid)
                    *cp_type = DEVDRV_PROCESS_DEV_ONLY;
                else if (d_sign->devpid[i][j][DEVDRV_PROCESS_HCCP] == pid)
                    *cp_type = DEVDRV_PROCESS_HCCP;
                else
                    continue;
                *chip_id = i;
                *vfid = j;
                *host_pid = d_sign->hostpid;
                spin_unlock_bh(&d_info->proc_hash_table_lock);
                return 0;
            }
        }
    }

    spin_unlock_bh(&d_info->proc_hash_table_lock);

    return -EINVAL;
}
EXPORT_SYMBOL(devdrv_query_process_host_pid);

int devdrv_fop_query_host_pid(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_ioctl_para_query_pid para_info = {0};
    int node_id = numa_node_id();
    int ret;

    if (copy_from_user_safe(&para_info, (void *)(uintptr_t)arg, sizeof(struct devdrv_ioctl_para_query_pid))) {
        devdrv_drv_err("copy_from_user error. dev_id:%u\n", node_id);
        return -EINVAL;
    }

    if (para_info.pid <= 0) {
        devdrv_drv_err("invalid para pid(%d).\n", para_info.pid);
        return -EINVAL;
    }

    ret = devdrv_query_process_host_pid(para_info.pid, &para_info.chip_id, &para_info.vfid, &para_info.host_pid,
        &para_info.cp_type);
    if (ret) {
        return -ESRCH;
    }
    devdrv_drv_debug("Query host_pid information. "
        "(node_id=%d; dev_id=%u, dev_pid=%d, tgid=%d, vfid=%u, hostpid=%d)",
        node_id, para_info.chip_id, para_info.pid, current->tgid, para_info.vfid, para_info.host_pid);

    ret = copy_to_user_safe((void *)(uintptr_t)arg, &para_info, sizeof(struct devdrv_ioctl_para_query_pid));
    if (ret != 0) {
        devdrv_drv_err("copy to user failed, ret(%d)\n", ret);
        return -EINVAL;
    }

    return 0;
}

int devdrv_check_hostpid(pid_t hostpid, unsigned int chip_id, unsigned int vfid)
{
    struct devdrv_manager_info *d_info = NULL;
    struct devdrv_process_sign *d_sign = NULL;
    u32 key, i;

    if (vfid >= VFID_NUM_MAX || chip_id >= DEVDRV_MAX_NODE_NUM) {
        devdrv_drv_err("vfid(%u) or chip_id(%u) is invalid\n", vfid, chip_id);
        return -EINVAL;
    }

    d_info = devdrv_get_manager_info();
    if (d_info == NULL) {
        devdrv_drv_err("d_info is null, hostpid:%d.\n", hostpid);
        return -EINVAL;
    }

    key = (u32)hostpid & DEVDRV_PROC_HASH_TABLE_MASK;
    spin_lock_bh(&d_info->proc_hash_table_lock);
    /*lint -e666 */
    hash_for_each_possible(d_info->proc_hash_table, d_sign, link, key)
    {
        if (d_sign->hostpid != hostpid)
            continue;
        for (i = DEVDRV_PROCESS_CP1; i < DEVDRV_PROCESS_CPTYPE_MAX; i++) {
            if (d_sign->devpid[chip_id][vfid][i] == current->tgid) {
                spin_unlock_bh(&d_info->proc_hash_table_lock);
                return 0;
            }
        }
    }
    spin_unlock_bh(&d_info->proc_hash_table_lock);
    devdrv_drv_err("hostpid(%d) or chip_id(%u) or vfid(%u) is error.\n", hostpid, chip_id, vfid);
    return -EINVAL;
}
EXPORT_SYMBOL(devdrv_check_hostpid);

int devdrv_check_sign(pid_t hostpid, const char *sign, u32 len)
{
    struct devdrv_manager_info *d_info = devdrv_get_manager_info();
    struct devdrv_process_sign *d_sign = NULL;
    u32 key;

    if ((sign == NULL) || (len != PROCESS_SIGN_LENGTH) || (d_info == NULL)) {
        devdrv_drv_err("sign is NULL(%d) or invalid length(%u) or d_info is NULL(%d).\n",
            (sign == NULL), len, (d_info == NULL));
        return -EINVAL;
    }

    key = (u32)hostpid & DEVDRV_PROC_HASH_TABLE_MASK;
    spin_lock_bh(&d_info->proc_hash_table_lock);
    /*lint -e666 */
    hash_for_each_possible(d_info->proc_hash_table, d_sign, link, key) {
        if (d_sign->hostpid == hostpid) {
            /* The sign is not used, not need check. */
            spin_unlock_bh(&d_info->proc_hash_table_lock);
            return 0;
        }
    }
    spin_unlock_bh(&d_info->proc_hash_table_lock);

    devdrv_drv_err("hostpid(%d) is not exist in hashtable\n", hostpid);
    return -EINVAL;
}
EXPORT_SYMBOL(devdrv_check_sign);

int devdrv_get_dev_process(pid_t devpid)
{
    struct devdrv_manager_info *d_info = devdrv_get_manager_info();
    struct devdrv_process_sign *d_sign = NULL;
    u32 bkt = 0;
    int i, j;

    if (d_info == NULL) {
        return 0;
    }

    spin_lock_bh(&d_info->proc_hash_table_lock);
    hash_for_each(d_info->proc_hash_table, bkt, d_sign, link) {
        for (i = 0; i < DEVDRV_MAX_NODE_NUM; i++) {
            for (j = 0; j < VFID_NUM_MAX; j++) {
                if ((d_sign->devpid[i][j][DEVDRV_PROCESS_CP1] != devpid) &&
                    (d_sign->devpid[i][j][DEVDRV_PROCESS_CP2] != devpid) &&
                    (d_sign->devpid[i][j][DEVDRV_PROCESS_QS] != devpid) &&
                    (d_sign->devpid[i][j][DEVDRV_PROCESS_DEV_ONLY] != devpid) &&
                    (d_sign->devpid[i][j][DEVDRV_PROCESS_HCCP] != devpid)) {
                    continue;
                }

                if (d_sign->host_process_status != 0) {
                    spin_unlock_bh(&d_info->proc_hash_table_lock);
                    return -EOWNERDEAD;
                }
                d_sign->in_use_count++;
                spin_unlock_bh(&d_info->proc_hash_table_lock);
                return 0;
            }
        }
    }
    spin_unlock_bh(&d_info->proc_hash_table_lock);

    return -ESRCH;
}
EXPORT_SYMBOL(devdrv_get_dev_process);

void devdrv_put_dev_process(pid_t devpid)
{
    struct devdrv_manager_info *d_info = devdrv_get_manager_info();
    struct devdrv_process_sign *d_sign = NULL;
    u32 bkt = 0;
    int i, j;

    if (d_info == NULL) {
        return;
    }

    spin_lock_bh(&d_info->proc_hash_table_lock);
    hash_for_each(d_info->proc_hash_table, bkt, d_sign, link) {
        for (i = 0; i < DEVDRV_MAX_NODE_NUM; i++) {
            for (j = 0; j < VFID_NUM_MAX; j++) {
                if ((d_sign->devpid[i][j][DEVDRV_PROCESS_CP1] != devpid) &&
                    (d_sign->devpid[i][j][DEVDRV_PROCESS_CP2] != devpid) &&
                    (d_sign->devpid[i][j][DEVDRV_PROCESS_QS] != devpid) &&
                    (d_sign->devpid[i][j][DEVDRV_PROCESS_DEV_ONLY] != devpid) &&
                    (d_sign->devpid[i][j][DEVDRV_PROCESS_HCCP] != devpid)) {
                    continue;
                }
                d_sign->in_use_count =
                    (d_sign->in_use_count == 0) ? 0 : d_sign->in_use_count - 1;
                spin_unlock_bh(&d_info->proc_hash_table_lock);
                return;
            }
        }
    }
    spin_unlock_bh(&d_info->proc_hash_table_lock);

    return;
}
EXPORT_SYMBOL(devdrv_put_dev_process);

int devdrv_notice_process_exit(unsigned int host_pid)
{
    struct devdrv_manager_info *d_info = devdrv_get_manager_info();
    struct devdrv_process_sign *d_sign = NULL;
    u32 key = (u32)host_pid & DEVDRV_PROC_HASH_TABLE_MASK;

    if (d_info == NULL) {
        return 0;
    }

    spin_lock_bh(&d_info->proc_hash_table_lock);
    hash_for_each_possible(d_info->proc_hash_table, d_sign, link, key) {
        if (d_sign->hostpid == host_pid) {
            int ret = d_sign->in_use_count == 0 ? 0 : -EBUSY;
            d_sign->host_process_status = EOWNERDEAD;
            spin_unlock_bh(&d_info->proc_hash_table_lock);
            return ret;
        }
    }
    spin_unlock_bh(&d_info->proc_hash_table_lock);

    return 0;
}

int devdrv_manager_get_product_type(void)
{
    return HOST_TYPE_NORMAL;
}
EXPORT_SYMBOL(devdrv_manager_get_product_type); //lint !e508

#ifdef CFG_FEATURE_IPC_NOTIFY
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
#endif

STATIC int devdrv_manager_open(struct inode *inode, struct file *filep)
{
    struct devdrv_manager_context *dev_manager_context = NULL;
    u32 docker_id;
    int ret;

    if (filep == NULL) {
        devdrv_drv_err("filep is NULL.\n");
        return -EINVAL;
    }

    if (devdrv_manager_container_task_struct_check(current)) {
        devdrv_drv_err("current is invalid.\n");
        return -EINVAL;
    }

    dev_manager_context = kzalloc(sizeof(struct devdrv_manager_context), GFP_KERNEL | __GFP_ACCOUNT);
    if (dev_manager_context == NULL) {
        devdrv_drv_err("kzalloc context failed.\n");
        return -ENOMEM;
    }

    dev_manager_context->pid = current->pid;
    dev_manager_context->tgid = current->tgid;
    dev_manager_context->start_time = current->start_time;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
    dev_manager_context->real_start_time = current->start_boottime;
#else
    dev_manager_context->real_start_time = current->real_start_time;
#endif
    dev_manager_context->mnt_ns = (u64)(uintptr_t)current->nsproxy->mnt_ns;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0)
    dev_manager_context->pid_ns = current->nsproxy->pid_ns_for_children;
#else
    dev_manager_context->pid_ns = current->nsproxy->pid_ns;
#endif
#ifdef CFG_FEATURE_IPC_NOTIFY
    ret = devdrv_manager_ipc_notify_init(dev_manager_context);
    if (ret != 0) {
        devdrv_drv_err("Manager ipc id init failed. (ret=%d)\n", ret);
        kfree(dev_manager_context);
        dev_manager_context = NULL;
        return ret;
    }
#endif
    dev_manager_context->dev = devdrv_get_manager_info()->dev;
    dev_manager_context->gpioirq = NULL;

    ret = devdrv_manager_container_table_overlap(dev_manager_context, &docker_id);
    if (ret) {
        kfree(dev_manager_context);
        dev_manager_context = NULL;
        devdrv_drv_err("update_item failed, ret(%d).\n", ret);
        return -EINVAL;
    }

    filep->private_data = dev_manager_context;

    return 0;
}

STATIC inline void devdrv_manager_set_sign_print(struct devdrv_process_sign *d_sign, u32 cp_type,
    u32 chip_id, u32 vfid, struct devdrv_process_sign *sign_print)
{
    sign_print->hostpid = d_sign->hostpid;
    sign_print->devpid[chip_id][vfid][cp_type] = d_sign->devpid[chip_id][vfid][cp_type];
}

STATIC void devdrv_manager_release_devpid(struct devdrv_process_sign *d_sign, pid_t devpid,
    struct devdrv_process_sign *sign_print, u32 *out_chip_id, u32 *out_vfid)
{
    u32 chip_id;
    u32 vfid;

    for (chip_id = 0; chip_id < DEVDRV_MAX_NODE_NUM; chip_id++) {
        for (vfid = 0; vfid < VFID_NUM_MAX; vfid++) {
            if ((d_sign->devpid[chip_id][vfid][DEVDRV_PROCESS_CP1] == DEVMNG_PID_INVALID) &&
                (d_sign->devpid[chip_id][vfid][DEVDRV_PROCESS_CP2] == DEVMNG_PID_INVALID) &&
                (d_sign->devpid[chip_id][vfid][DEVDRV_PROCESS_DEV_ONLY] == DEVMNG_PID_INVALID) &&
                (d_sign->devpid[chip_id][vfid][DEVDRV_PROCESS_QS] == DEVMNG_PID_INVALID) &&
                (d_sign->devpid[chip_id][vfid][DEVDRV_PROCESS_HCCP] == DEVMNG_PID_INVALID)) {
                continue;
            }
            if (d_sign->devpid[chip_id][vfid][DEVDRV_PROCESS_CP1] == devpid) {
                devdrv_manager_set_sign_print(d_sign, DEVDRV_PROCESS_CP1, chip_id, vfid, sign_print);
                devdrv_manager_set_sign_print(d_sign, DEVDRV_PROCESS_CP2, chip_id, vfid, sign_print);
                d_sign->devpid[chip_id][vfid][DEVDRV_PROCESS_CP1] = DEVMNG_PID_INVALID;
                d_sign->devpid[chip_id][vfid][DEVDRV_PROCESS_CP2] = DEVMNG_PID_INVALID;
                d_sign->cp_count--;
                *out_chip_id = chip_id;
                *out_vfid = vfid;
            } else if ((d_sign->devpid[chip_id][vfid][DEVDRV_PROCESS_CP2] == devpid) &&
                (d_sign->devpid[chip_id][vfid][DEVDRV_PROCESS_CP1] != DEVMNG_PID_INVALID)) {
                devdrv_manager_set_sign_print(d_sign, DEVDRV_PROCESS_CP2, chip_id, vfid, sign_print);
                d_sign->devpid[chip_id][vfid][DEVDRV_PROCESS_CP2] = DEVMNG_PID_START_ONCE;
                *out_chip_id = chip_id;
                *out_vfid = vfid;
            } else if (d_sign->devpid[chip_id][vfid][DEVDRV_PROCESS_CP2] == devpid) {
                devdrv_manager_set_sign_print(d_sign, DEVDRV_PROCESS_CP2, chip_id, vfid, sign_print);
                d_sign->devpid[chip_id][vfid][DEVDRV_PROCESS_CP2] = DEVMNG_PID_INVALID;
                *out_chip_id = chip_id;
                *out_vfid = vfid;
            }
            if (d_sign->devpid[chip_id][vfid][DEVDRV_PROCESS_DEV_ONLY] == devpid) {
                devdrv_manager_set_sign_print(d_sign, DEVDRV_PROCESS_DEV_ONLY, chip_id, vfid, sign_print);
                d_sign->devpid[chip_id][vfid][DEVDRV_PROCESS_DEV_ONLY] = DEVMNG_PID_INVALID;
                d_sign->cp_count--;
                *out_chip_id = chip_id;
                *out_vfid = vfid;
            }
            if (d_sign->devpid[chip_id][vfid][DEVDRV_PROCESS_QS] == devpid) {
                devdrv_manager_set_sign_print(d_sign, DEVDRV_PROCESS_QS, chip_id, vfid, sign_print);
                d_sign->devpid[chip_id][vfid][DEVDRV_PROCESS_QS] = DEVMNG_PID_INVALID;
                d_sign->cp_count--;
                *out_chip_id = chip_id;
                *out_vfid = vfid;
            }
            if (d_sign->devpid[chip_id][vfid][DEVDRV_PROCESS_HCCP] == devpid) {
                devdrv_manager_set_sign_print(d_sign, DEVDRV_PROCESS_HCCP, chip_id, vfid, sign_print);
                d_sign->devpid[chip_id][vfid][DEVDRV_PROCESS_HCCP] = DEVMNG_PID_INVALID;
                d_sign->cp_count--;
                *out_chip_id = chip_id;
                *out_vfid = vfid;
            }
        }
    }

    if (d_sign->cp_count == 0) {
        hash_del(&d_sign->link);
        kfree(d_sign);
        d_sign = NULL;
    }
}

STATIC void devdrv_manager_process_sign_release(pid_t devpid)
{
    struct devdrv_manager_info *d_info = devdrv_get_manager_info();
    struct devdrv_process_sign *d_sign = NULL;
    struct hlist_node *local_sign = NULL;
    struct devdrv_process_sign *sign_print;
    u32 bkt;
    u32 chip_id = 0;
    u32 vfid = 0;

    if (d_info == NULL) {
        devdrv_drv_err("dev_manager_info is NULL, devpid %d.\n", devpid);
        return;
    }
    sign_print = kzalloc(sizeof(struct devdrv_process_sign), GFP_KERNEL | __GFP_ACCOUNT);
    if (sign_print == NULL) {
        devdrv_drv_err("kzalloc devdrv_process_sign failed.\n");
        return;
    }
    spin_lock_bh(&d_info->proc_hash_table_lock);
    hash_for_each_safe(d_info->proc_hash_table, bkt, local_sign, d_sign, link) {
        /* release devpid if match */
        devdrv_manager_release_devpid(d_sign, devpid, sign_print, &chip_id, &vfid);
    }
    spin_unlock_bh(&d_info->proc_hash_table_lock);

    /* print information about hashnode when release devpid */
    if (sign_print->hostpid != 0) {
        devdrv_drv_info("release hostpid(%d), devpid(%d), cp1_pid(%d), cp2_pid(%d), "
                        "dev_only_pid(%d), qs_pid(%d), hccp_pid(%d), chip_id(%u), vfid(%u).\n",
                        sign_print->hostpid, devpid,
                        sign_print->devpid[chip_id][vfid][DEVDRV_PROCESS_CP1],
                        sign_print->devpid[chip_id][vfid][DEVDRV_PROCESS_CP2],
                        sign_print->devpid[chip_id][vfid][DEVDRV_PROCESS_DEV_ONLY],
                        sign_print->devpid[chip_id][vfid][DEVDRV_PROCESS_QS],
                        sign_print->devpid[chip_id][vfid][DEVDRV_PROCESS_HCCP],
                        chip_id, vfid);
    }
    kfree(sign_print);
    sign_print = NULL;
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
    filep->private_data = NULL;
#ifdef CFG_FEATURE_IPC_NOTIFY
    devdrv_manager_ipc_notify_uninit(dev_manager_context);
#endif
    kfree(dev_manager_context);
    dev_manager_context = NULL;

    return 0;
}

STATIC int devdrv_manager_get_device_status(struct file *filep, unsigned int cmd, unsigned long arg)
{
    enum devdrv_ts_status status;
    int para = 0;
    int ret;

    ret = copy_from_user_safe(&para, (void *)((uintptr_t)arg), sizeof(int));
    if (ret) {
        devdrv_drv_err("copy from user failed.\n");
        return -EINVAL;
    }

    if (para < 0 || para >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("devid is invalid.\n");
        return -EINVAL;
    }

    status = tsdrv_get_ts_status(para, 0);
    if (dev_manager_info == NULL || dev_manager_info->dev_info[para] == NULL)
        para = DRV_STATUS_INITING;
    else if (status == TS_DOWN)
        para = DRV_STATUS_EXCEPTION;
    else if (status == TS_WORK)
        para = DRV_STATUS_WORK;
    else
        para = DRV_STATUS_INITING;

    ret = copy_to_user_safe((void *)((uintptr_t)arg), &para, sizeof(int));
    if (ret) {
        devdrv_drv_err("copy from user failed.\n");
        return -EINVAL;
    }

    return 0;
}

STATIC int devdrv_manager_get_core(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_hardware_inuse inuse = {0};
    struct devdrv_hardware_spec spec = {0};
    int ret;
    switch (cmd) {
        case DEVDRV_MANAGER_GET_CORE_SPEC:
            ret = copy_from_user_safe(&spec, (void *)((uintptr_t)arg), sizeof(struct devdrv_hardware_spec));
            if (ret) {
                devdrv_drv_err("copy_from_user_safe failed.\n");
                return -1;
            }
            ret = devdrv_get_core_spec(spec.devid, 0, &spec);
            if (ret) {
                devdrv_drv_err("devdrv_get_core_spec failed.\n");
                return -1;
            }
            ret = copy_to_user_safe((void *)((uintptr_t)arg), &spec, sizeof(struct devdrv_hardware_spec));
            if (ret) {
                devdrv_drv_err("copy_to_user_safe failed.\n");
                return -1;
            }
            break;
        case DEVDRV_MANAGER_GET_CORE_INUSE:
            ret = copy_from_user_safe(&inuse, (void *)((uintptr_t)arg), sizeof(struct devdrv_hardware_inuse));
            if (ret) {
                devdrv_drv_err("copy_from_user_safe failed.\n");
                return -1;
            }
            ret = devdrv_get_core_inuse(inuse.devid, 0, &inuse);
            if (ret) {
                devdrv_drv_err("devdrv_get_core_inuse failed.\n");
                return -1;
            }
            ret = copy_to_user_safe((void *)((uintptr_t)arg), &inuse, sizeof(struct devdrv_hardware_inuse));
            if (ret) {
                devdrv_drv_err("copy_to_user_safe failed.\n");
                return -1;
            }
            break;
        default:
            devdrv_drv_err("invalid cmd.\n");
            return -1;
    }

    return 0;
}

STATIC int devdrv_manager_get_devids(unsigned long arg)
{
    struct devdrv_manager_hccl_devinfo hccl_devinfo = { 0 };

    hccl_devinfo.num_dev = devdrv_manager_get_devnum();
    if (hccl_devinfo.num_dev == 0 || hccl_devinfo.num_dev > DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("wrong device number, num_dev = %d\n", hccl_devinfo.num_dev);
        return -EINVAL;
    }

    if (devdrv_get_devids(hccl_devinfo.devids, DEVDRV_MAX_DAVINCI_NUM)) {
        devdrv_drv_err("devdrv_get_devids failed.\n");
        return -EINVAL;
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

    if (copy_from_user_safe(&hccl_devinfo, (void *)(uintptr_t)arg, sizeof(hccl_devinfo))) {
        devdrv_drv_err("copy from user failed\n");
        return -EINVAL;
    }

    dev_info = devdrv_manager_get_devdrv_info(hccl_devinfo.dev_id);
    if (dev_info == NULL) {
        devdrv_drv_err("get default dev_info failed, dev_id = %u\n", hccl_devinfo.dev_id);
        return -ENODEV;
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
    hccl_devinfo.cpu_system_count = devdrv_manager_get_cpu_tick();
    hccl_devinfo.monotonic_raw_time_ns = ktime_get_raw_ns();

    if (copy_to_user_safe((void *)(uintptr_t)arg, &hccl_devinfo, sizeof(hccl_devinfo))) {
        devdrv_drv_err("copy to user error.\n");
        return -EFAULT;
    }

    return 0;
}

STATIC int devdrv_manager_get_devid_by_localdevid(unsigned long arg)
{
    u32 local_devid = 0;
    u32 dev_id;

    if (copy_from_user_safe(&local_devid, (void *)(uintptr_t)arg, sizeof(u32))) {
        devdrv_drv_err("copy from user failed\n");
        return -EINVAL;
    }

    dev_id = devdrv_manager_get_devid(local_devid);
    if (dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_warn("dev_id invalid(%d)\n", dev_id);
        return -EAGAIN;
    }
    if (copy_to_user_safe((void *)(uintptr_t)arg, &dev_id, sizeof(u32))) {
        devdrv_drv_err("copy to user error.\n");
        return -EFAULT;
    }

    return 0;
}

STATIC int devdrv_manager_get_dev_info_by_phyid(unsigned long arg)
{
    int ret;
    struct devdrv_phy_get_devinfo_para para = {0};

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

STATIC int devdrv_manager_devinfo_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;

    switch (cmd) {
        case DEVDRV_MANAGER_GET_DEVIDS:
            ret = devdrv_manager_get_devids(arg);
            break;
        case DEVDRV_MANAGER_GET_DEVINFO:
            ret = devdrv_manager_get_devinfo(arg);
            break;
        case DEVDRV_MANAGER_GET_DEVID_BY_LOCALDEVID:
            ret = devdrv_manager_get_devid_by_localdevid(arg);
            break;
        case DEVDRV_MANAGER_GET_H2D_DEVINFO:
            ret = devdrv_manager_get_devinfo(arg);
            break;
        case DEVDRV_MANAGER_GET_DEV_INFO_BY_PHYID:
            ret = devdrv_manager_get_dev_info_by_phyid(arg);
            break;
        default:
            ret = -EINVAL;
            break;
    }

    return ret;
}

STATIC int devdrv_manager_ioctl_get_devnum(struct file *filep, unsigned int cmd, unsigned long arg)
{
    u32 devnum;

    devnum = devdrv_manager_get_devnum();
    if (copy_to_user_safe((void *)(uintptr_t)arg, &devnum, sizeof(u32)))
        return -EFAULT;
    else
        return 0;
}

STATIC int devdrv_manager_ioctl_get_plat_info(struct file *filep, unsigned int cmd, unsigned long arg)
{
    u32 plat_info;

    plat_info = dev_manager_info->plat_info;
    if (copy_to_user_safe((void *)(uintptr_t)arg, &plat_info, sizeof(u32)))
        return -EFAULT;
    else
        return 0;
}

STATIC int devdrv_manager_get_module_status(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_module_status status;
    struct devdrv_info *dev_info = NULL;
    int ret;

    dev_info = dev_manager_info->dev_info[0];
    if (dev_info == NULL) {
        devdrv_drv_err("no device.\n");
        return -ENODEV;
    }

    status.lpm3_start_fail = devdrv_lpm3_start_fail;
    status.lpm3_lost_heart_beat = 0; /* get m3 heart beat breken */
    status.ts_start_fail = dev_info->pdata->ts_pdata[0].ts_start_fail;
    status.ts_lost_heart_beat = (tsdrv_is_ts_work(dev_info->dev_id, 0) == false) ? 1 : 0;
    status.ts_sram_broken = 0x01 & (dev_info->ai_subsys_ip_broken_map >> DEVDRV_AI_SUBSYS_INIT_CHECK_SRAM_OFFSET);
    status.ts_sdma_broken = 0x01 & (dev_info->ai_subsys_ip_broken_map >> DEVDRV_AI_SUBSYS_INIT_CHECK_SDMA_OFFSET);
    status.ts_bs_broken = 0x01 & (dev_info->ai_subsys_ip_broken_map >> DEVDRV_AI_SUBSYS_INIT_CHECK_BS_OFFSET);
    status.ts_l2_buf0_broken = 0x01 & (dev_info->ai_subsys_ip_broken_map >> DEVDRV_AI_SUBSYS_INIT_CHECK_L2_BUF0_OFFSET);
    status.ts_l2_buf1_broken = 0x01 & (dev_info->ai_subsys_ip_broken_map >> DEVDRV_AI_SUBSYS_INIT_CHECK_L2_BUF1_OFFSET);

    status.ts_sdma_broken = 0x01 & (dev_info->ai_subsys_ip_broken_map >> DEVDRV_AI_SUBSYS_SDMA_WORKING_STATUS_OFFSET);
    status.ts_spcie_broken = 0x01 & (dev_info->ai_subsys_ip_broken_map >> DEVDRV_AI_SUBSYS_SPCIE_WORKING_STATUS_OFFSET);
    ret = copy_to_user_safe((void *)((uintptr_t)arg), &status, sizeof(struct devdrv_module_status));
    if (ret) {
        devdrv_drv_err("copy_to_user_safe failed.\n");
        return -1;
    }

    return 0;
}

STATIC int devdrv_manager_get_mini_devid(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    u32 id;
    int dev_id = 0;
    unsigned long flags;

    if (dev_manager_info == NULL) {
        devdrv_drv_err("device manager is not inited.\n");
        return -EINVAL;
    }

    spin_lock_irqsave(&dev_manager_info->spinlock, flags);
    if (!dev_manager_info->dev_id_flag[dev_id]) {
        spin_unlock_irqrestore(&dev_manager_info->spinlock, flags);
        devdrv_drv_err("never receive id from host.\n");
        return -EINVAL;
    }

    id = dev_manager_info->dev_id[dev_id];
    spin_unlock_irqrestore(&dev_manager_info->spinlock, flags);

    ret = copy_to_user_safe((void *)((uintptr_t)arg), &id, sizeof(u32));
    if (ret) {
        devdrv_drv_err("copy_to_user_safe failed.\n");
        return -EINVAL;
    }

    return 0;
}

STATIC int devdrv_manager_get_mini_board_id(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct device_node *np = NULL;
    unsigned int boardid[4];
    u32 board_id;
    int ret;

    np = of_find_compatible_node(NULL, NULL, "hisilicon,mini");
    if (np == NULL) {
        devdrv_drv_err("failed to find hisilicon,sysctrl node\n");
        return -1;
    }

    ret = of_property_read_u32_array(np, "hisi,boardid", boardid, 4);
    if (ret) {
        devdrv_drv_err("failed to read 'hisi,boardid', ret: %d.\n", ret);
        return -1;
    }

    board_id = boardid[0] * 1000 + boardid[1] * 100 + boardid[2] * 10 + boardid[3];

    ret = copy_to_user_safe((void *)((uintptr_t)arg), &board_id, sizeof(u32));
    if (ret) {
        devdrv_drv_err("copy_to_user_safe failed.\n");
        return -EINVAL;
    }

    return 0;
}

extern int hisi_adc_get_value(unsigned int channel);
extern int get_second_pmu_buck_volt(unsigned int device_id, unsigned int channel, unsigned int *volt_mv);
extern int get_main_pmu_buck_volt(unsigned int channel, unsigned int *volt_mv);
extern int get_main_pmu_ldo_volt(unsigned int channel, unsigned int *volt_mv);

STATIC int devdrv_manager_get_emmc_voltage(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_emmc_voltage_para devdrv_emmc_voltage = { 0, 0 };
    devdrv_drv_debug("devdrv_manager_get_emmc_voltage start\n");
    devdrv_emmc_voltage.emmc_vcc = hisi_adc_get_value(PMU_EMMC_VCC_CHANNEL);
    devdrv_emmc_voltage.emmc_vccq = hisi_adc_get_value(ADCIN8_SLOT1);

    if (copy_to_user_safe((void *)((uintptr_t)arg), &devdrv_emmc_voltage, sizeof(struct devdrv_emmc_voltage_para))) {
        devdrv_drv_err("copy_to_user_safe failed\n");
        return -EFAULT;
    }

    devdrv_drv_debug("devdrv_manager_get_emmc_voltage finish\n");

    return 0;
}

#if defined(CFG_SOC_PLATFORM_MINI)
extern int pmu_ldo2_enable(void);
extern int pmu_ldo2_disable(void);
#endif

#if defined(CFG_TEE_EFUSE_LDO_DBG)
STATIC int devdrv_manager_enable_efuse_ldo2(struct file *filep, unsigned int cmd, unsigned long arg)
{
#if defined(CFG_SOC_PLATFORM_MINI)
    int ret;

    devdrv_drv_debug("devdrv_manager_enable_efuse_ldo2 start\n");

    ret = pmu_ldo2_enable();
    if (ret) {
        devdrv_drv_err("enable efuse ldo2 failed!!\n");
        return ret;
    }
    devdrv_drv_debug("devdrv_manager_disable_efuse_ldo finish\n");
#endif
    return 0;
}

STATIC int devdrv_manager_disable_efuse_ldo2(struct file *filep, unsigned int cmd, unsigned long arg)
{
#if defined(CFG_SOC_PLATFORM_MINI)
    int ret;

    devdrv_drv_debug("devdrv_manager_disable_efuse_ldo2 start\n");

    ret = pmu_ldo2_disable();
    if (ret) {
        devdrv_drv_err("disable efuse ldo2 failed!!\n");
        return ret;
    }
    devdrv_drv_debug("devdrv_manager_disable_efuse_ld02 finish\n");
#endif
    return 0;
}

#else
STATIC int devdrv_manager_enable_efuse_ldo2(struct file *filep, unsigned int cmd, unsigned long arg)
{
    (void)filep;
    (void)cmd;
    (void)arg;
    return 0;
}

STATIC int devdrv_manager_disable_efuse_ldo2(struct file *filep, unsigned int cmd, unsigned long arg)
{
    (void)filep;
    (void)cmd;
    (void)arg;
    return 0;
}
#endif

STATIC int devdrv_manager_get_local_devid(struct file *filep, unsigned int cmd, unsigned long arg)
{
    u32 local_devids[DEVDRV_MAX_DAVINCI_NUM] = {0};
    u32 num_dev;
    u8 i;
    u8 j = 0;

    num_dev = devdrv_manager_get_devnum();
    if (num_dev == 0 || num_dev > DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("wrong device number, num_dev = %d\n", num_dev);
        return -EINVAL;
    }

    /* get device id assigned by local */
    for (i = 0; i < DEVDRV_MAX_DAVINCI_NUM; i++) {
        if (dev_manager_info->dev_info[i] == NULL)
            continue;
        local_devids[j++] = i;
    }

    if (j == 0) {
        devdrv_drv_err("get device number err, (%d != %d)\n", num_dev, j);
        return -EINVAL;
    }

    if (copy_to_user_safe((void *)((uintptr_t)arg), local_devids, (long)(unsigned)sizeof(u32) * num_dev)) {
        devdrv_drv_err("copy to user failed.\n");
        return -EINVAL;
    }

    return 0;
}

STATIC int devdrv_manager_get_cpu_info(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_info *info = NULL;
    struct agentdrv_cpu_info cpu_info = { 0 };
    u32 chip_id = 0;
    int ret;

    ret = copy_from_user_safe(&chip_id, (void *)(uintptr_t)arg, sizeof(u32));
    if (ret) {
        devdrv_drv_err("copy from user failed.\n");
        return -EINVAL;
    }

    if (chip_id >= MAX_CHIP_NUM) {
        devdrv_drv_err("invalid device id(%d).\n", chip_id);
        return -EINVAL;
    }

    info = devdrv_manager_get_devdrv_info(chip_id);
    if (info == NULL) {
        devdrv_drv_err("invalid device id(%d), devdrv info is null.\n", chip_id);
        return -EINVAL;
    }

    cpu_info.aicpu_num = info->ai_cpu_core_num;
    cpu_info.aicpu_os_sched = 1;
    cpu_info.ccpu_num = info->ctrl_cpu_core_num;
    cpu_info.ccpu_os_sched = 1;
    cpu_info.tscpu_num = info->pdata->ts_pdata[0].ts_cpu_core_num;
    cpu_info.tscpu_os_sched = 1;

    ret = copy_to_user_safe((void *)(uintptr_t)arg, &cpu_info, sizeof(struct agentdrv_cpu_info));
    if (ret) {
        devdrv_drv_err("[dev_id = %u]:copy tp user failed.\n", chip_id);
        return -EINVAL;
    }

    return 0;
}


STATIC int devdrv_manager_get_host_phy_mach_flag(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_get_host_phy_mach_flag_para para = {0};
    int ret;

    ret = copy_from_user_safe(&para, (void *)(uintptr_t)arg, sizeof(struct devdrv_get_host_phy_mach_flag_para));
    if (ret) {
        devdrv_drv_err("copy from user failed.\n");
        return -EINVAL;
    }

#if defined(CFG_SOC_PLATFORM_MINI)
    ret = agentdrv_get_host_phy_mach_flag(para.devId, &para.host_flag);
    if (ret) {
        devdrv_drv_err("cannot get host flag.\n");
        return -EINVAL;
    }
#endif

    ret = copy_to_user_safe((void *)(uintptr_t)arg, &para, sizeof(struct devdrv_get_host_phy_mach_flag_para));
    if (ret) {
        devdrv_drv_err("copy_to_user_safe failed.\n");
        return -EINVAL;
    }

    return ret;
}

STATIC int devdrv_manager_container_cmd(struct file *filep, unsigned int cmd, unsigned long arg)
{
    return devdrv_manager_container_process(filep, arg);
}

STATIC int device_manager_sync_matrix_ready(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_manager_msg_info dev_manager_msg_info = { { 0 }, { 0 } };
    u32 devid = 0;
    u32 out_len;
    int ret, matrix_flag;

    devdrv_drv_info("*** begin to inform device matrix state***\n");

    ret = copy_from_user_safe(&devid, (void *)((uintptr_t)arg), sizeof(u32));
    if (ret) {
        devdrv_drv_err("copy from user failed.\n");
        return -EINVAL;
    }

    if (devid >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("invaild device id. devid = %u.\n", devid);
        return -EINVAL;
    }

    dev_manager_msg_info.header.dev_id = devid;
    dev_manager_msg_info.header.msg_id = DEVDRV_MANAGER_CHAN_D2H_SYNC_MATRIX_READY;
    dev_manager_msg_info.header.valid = (u16)DEVDRV_MANAGER_MSG_VALID;
    dev_manager_msg_info.header.result = (u16)DEVDRV_MANAGER_MSG_INVALID_RESULT;

    ret = agentdrv_common_msg_send(devid, &dev_manager_msg_info, sizeof(dev_manager_msg_info),
        sizeof(dev_manager_msg_info), &out_len, AGENTDRV_COMMON_MSG_DEVDRV_MANAGER);
    if (ret != 0) {
        devdrv_drv_err("common msg send failed, ret = %d\n", ret);
        return -ENODEV;
    }
    if (dev_manager_msg_info.header.result != 0) {
        devdrv_drv_err("matrix daemon sync failed, result = %u\n", dev_manager_msg_info.header.result);
        return -ENODEV;
    }

    matrix_flag = DEVDRV_MANAGER_MATRIX_VALID;
    ret = copy_to_user_safe((void *)(uintptr_t)arg, &matrix_flag, sizeof(int));
    if (ret) {
        devdrv_drv_err("copy_to_user_safe failed.\n");
        return -EINVAL;
    }

    devdrv_drv_info("*** inform device matrix ready info ***\n");

    return 0;
}
STATIC int devdrv_manager_appmon_bbox_exception_hndl(struct file *filep, unsigned int cmd, unsigned long arg)
{
    u32 excepid = 0;
    int ret;
    struct timespec os_time;
    excep_time timestamp;

    ret = copy_from_user_safe(&excepid, (void *)((uintptr_t)arg), sizeof(u32));
    if (ret) {
        devdrv_drv_err("ascend_monitor copy_from_user_safe failed.,ret = %d\n", ret);
        return ret;
    }
    devdrv_drv_info("*** ascend_monitor excepid = 0x%0x ***\n", excepid);
    ret = appmon_except_id_check(excepid);
    if (ret) {
        devdrv_drv_err("ascend_monitor except id check failed, ret = %d\n", ret);
        return ret;
    }

    os_time = current_kernel_time();
    timestamp.tv_sec = os_time.tv_sec;
    timestamp.tv_usec = os_time.tv_nsec / 1000; /* 1us is 1000ns */

#ifdef CFG_SOC_PLATFORM_CLOUD
    bbox_system_error(0, excepid, &timestamp, 0);
#else
    mntn_system_error(excepid, timestamp, 0);
#endif

    return 0;
}

STATIC int devdrv_manager_get_process_sign(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct process_sign dev_sign = { 0 };
    int ret;

    dev_sign.tgid = current->tgid;
    devdrv_drv_info("tgid=%d\n", (int)dev_sign.tgid);

    ret = copy_to_user_safe((void *)((uintptr_t)arg), &dev_sign, sizeof(struct process_sign));
    if (ret) {
        devdrv_drv_err("copy to user failed, ret(%d).\n", ret);
        return ret;
    }

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

STATIC int devdrv_manager_update_device_startup_status(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    int num_os_status_set = 0;
    unsigned int dev_index;
    unsigned int num_dev;
    struct devdrv_info *dev_info = NULL;
    struct devdrv_device_work_status para = {0};
    const char *process_name[WHITE_LIST_PROCESS_NUM] = {PROCESS_NAME_DMP};

    /* check process whitelist */
    ret = whitelist_process_handler(process_name, WHITE_LIST_PROCESS_NUM);
    if (ret != 0) {
        devdrv_drv_err("whitelist_process_handler error. (ret=%d)\n", ret);
        return ret;
    }

    if (devdrv_is_pci_rc_mode() == true) {
        return 0;
    }

    ret = copy_from_user_safe(&para, (void *)(uintptr_t)arg, sizeof(struct devdrv_device_work_status));
    if (ret != 0) {
        devdrv_drv_err("copy from user failed, ret(%d).\n", ret);
        return -EINVAL;
    }

    ret = devdrv_get_devnum(&num_dev);
    if (ret != 0) {
        devdrv_drv_err("devdrv_get_devnum failed, ret(%d).\n", ret);
        return -EINVAL;
    }

    for (dev_index = 0; dev_index < num_dev; dev_index++) {
        dev_info = devdrv_manager_get_devdrv_info(dev_index);
        if ((dev_info == NULL) || (dev_info->shm_status == NULL)) {
            devdrv_drv_err("device[%d] dev_info or shm_status is NULL!.\n", dev_index);
            continue;
        }
        if (devdrv_is_pci_rc_mode() == false)
            dev_info->shm_status->os_status = (u16)para.device_process_status;
        dev_info->dmp_started = true;
        num_os_status_set++;
    }

    ret = copy_to_user_safe((void *)(uintptr_t)arg, &para, sizeof(struct devdrv_device_work_status));
    if (ret != 0) {
        devdrv_drv_err("copy to user failed, ret(%d)\n", ret);
        return -EINVAL;
    }

    return num_os_status_set == num_dev ? 0 : EFAULT;
}

#ifdef CFG_FEATURE_IPC_NOTIFY
STATIC inline int devdrv_manager_drv_ops_check(void)
{
    if (devdrv_platform_drv_ops.ipc_notify_create == NULL || devdrv_platform_drv_ops.ipc_notify_open == NULL ||
        devdrv_platform_drv_ops.ipc_notify_close == NULL || devdrv_platform_drv_ops.ipc_notify_destroy == NULL ||
        devdrv_platform_drv_ops.ipc_notify_set_pid == NULL) {
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
            ret = devdrv_platform_drv_ops.ipc_notify_create((void*)context, arg, (void*)(uintptr_t)&notify_ioctl_info);
            break;
        case DEVDRV_MANAGER_IPC_NOTIFY_OPEN:
            ret = devdrv_platform_drv_ops.ipc_notify_open((void*)context, arg, (void*)(uintptr_t)&notify_ioctl_info);
            break;
        case DEVDRV_MANAGER_IPC_NOTIFY_CLOSE:
            ret = devdrv_platform_drv_ops.ipc_notify_close((void*)context, (void*)(uintptr_t)&notify_ioctl_info);
            break;
        case DEVDRV_MANAGER_IPC_NOTIFY_DESTROY:
            ret = devdrv_platform_drv_ops.ipc_notify_destroy((void*)context, (void*)(uintptr_t)&notify_ioctl_info);
            break;
        case DEVDRV_MANAGER_IPC_NOTIFY_SET_PID:
            ret = devdrv_platform_drv_ops.ipc_notify_set_pid((void*)context, (void*)(uintptr_t)&notify_ioctl_info);
            break;

        default:
            devdrv_drv_err("invalid cmd, cmd = %d\n", _IOC_NR(cmd));
            ret = -EFAULT;
            break;
    };
    devdrv_manager_ops_sem_up_read();
    (void)memset_s(notify_ioctl_info.name, DEVDRV_IPC_NAME_SIZE, 0, DEVDRV_IPC_NAME_SIZE);

    return ret;
}
#endif

STATIC int (*devdrv_manager_ioctl_handlers[DEVDRV_MANAGER_CMD_MAX_NR])(struct file *filep, unsigned int cmd,
    unsigned long arg) = {
        [_IOC_NR(DEVDRV_MANAGER_GET_PCIINFO)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_DEVNUM)] = devdrv_manager_ioctl_get_devnum,
        [_IOC_NR(DEVDRV_MANAGER_GET_PLATINFO)] = devdrv_manager_ioctl_get_plat_info,
        [_IOC_NR(DEVDRV_MANAGER_SVMVA_TO_DEVID)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_CHANNELINFO)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_DEVICE_STATUS)] = devdrv_manager_get_device_status,
        [_IOC_NR(DEVDRV_MANAGER_GET_CORE_SPEC)] = devdrv_manager_get_core,
        [_IOC_NR(DEVDRV_MANAGER_GET_CORE_INUSE)] = devdrv_manager_get_core,
        [_IOC_NR(DEVDRV_MANAGER_GET_DEVIDS)] = devdrv_manager_devinfo_ioctl,
        [_IOC_NR(DEVDRV_MANAGER_GET_DEVINFO)] = devdrv_manager_devinfo_ioctl,
        [_IOC_NR(DEVDRV_MANAGER_GET_DEVID_BY_LOCALDEVID)] = devdrv_manager_devinfo_ioctl,
        [_IOC_NR(DEVDRV_MANAGER_GET_DEV_INFO_BY_PHYID)] = devdrv_manager_devinfo_ioctl,
        [_IOC_NR(DEVDRV_MANAGER_GET_PCIE_ID_INFO)] = devdrv_manager_inquiry_info,
        [_IOC_NR(DEVDRV_MANAGER_GET_FLASH_COUNT)] = devdrv_manager_inquiry_info,
        [_IOC_NR(DEVDRV_MANAGER_GET_FLASH_INFO)] = devdrv_manager_inquiry_info,
        [_IOC_NR(DEVDRV_MANAGER_GET_VOLTAGE)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_TEMPERATURE)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_AI_USE_RATE)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_FREQUENCY)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_TSENSOR)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_POWER)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_HEALTH_CODE)] = devdrv_manager_inquiry_info,
        [_IOC_NR(DEVDRV_MANAGER_GET_ERROR_CODE)] = devdrv_manager_inquiry_info,
        [_IOC_NR(DEVDRV_MANAGER_GET_DDR_CAPACITY)] = devdrv_manager_inquiry_info,
        [_IOC_NR(DEVDRV_MANAGER_LPM3_SMOKE)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_COMPUTE_POWER)] = devdrv_manager_imu_cmd,
        [_IOC_NR(DEVDRV_MANAGER_DEBUG_INFORM)] = devdrv_manager_imu_cmd,
        [_IOC_NR(DEVDRV_MANAGER_IMU_SMOKE)] = devdrv_manager_imu_cmd,
        [_IOC_NR(DEVDRV_MANAGER_BLACK_BOX_GET_EXCEPTION)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_DEVICE_MEMORY_DUMP)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_DEVICE_RESET_INFORM)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_MODULE_STATUS)] = devdrv_manager_get_module_status,
        [_IOC_NR(DEVDRV_MANAGER_GET_DEVICE_DMA_ADDR)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_INTERRUPT_INFO)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_MINI_DEVID)] = devdrv_manager_get_mini_devid,
        [_IOC_NR(DEVDRV_MANAGER_GET_MINI_BOARD_ID)] = devdrv_manager_get_mini_board_id,
        [_IOC_NR(DEVDRV_MANAGER_PCIE_PRE_RESET)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_PCIE_RESCAN)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_ALLOC_HOST_DMA_ADDR)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_FREE_HOST_DMA_ADDR)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_PCIE_SRAM_READ)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_PCIE_SRAM_WRITE)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_EMMC_VOLTAGE)] = devdrv_manager_get_emmc_voltage,
        [_IOC_NR(DEVDRV_MANAGER_GET_DEVICE_BOOT_STATUS)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_ENABLE_EFUSE_LDO)] = devdrv_manager_enable_efuse_ldo2,
        [_IOC_NR(DEVDRV_MANAGER_DISABLE_EFUSE_LDO)] = devdrv_manager_disable_efuse_ldo2,

        [_IOC_NR(DEVDRV_MANAGER_CONTAINER_CMD)] = devdrv_manager_container_cmd,
        [_IOC_NR(DEVDRV_MANAGER_GET_HOST_PHY_MACH_FLAG)] = devdrv_manager_get_host_phy_mach_flag,
        [_IOC_NR(DEVDRV_MANAGER_GET_LOCAL_DEVICEIDS)] = devdrv_manager_get_local_devid,
#ifdef CFG_FEATURE_IPC_NOTIFY
        [_IOC_NR(DEVDRV_MANAGER_IPC_NOTIFY_CREATE)] = devdrv_manager_ipc_notify_ioctl,
        [_IOC_NR(DEVDRV_MANAGER_IPC_NOTIFY_OPEN)] = devdrv_manager_ipc_notify_ioctl,
        [_IOC_NR(DEVDRV_MANAGER_IPC_NOTIFY_CLOSE)] = devdrv_manager_ipc_notify_ioctl,
        [_IOC_NR(DEVDRV_MANAGER_IPC_NOTIFY_DESTROY)] = devdrv_manager_ipc_notify_ioctl,
        [_IOC_NR(DEVDRV_MANAGER_IPC_NOTIFY_SET_PID)] = devdrv_manager_ipc_notify_ioctl,
#endif
        [_IOC_NR(DEVDRV_MANAGER_SET_NEW_TIME)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_PCIE_DDR_READ)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_PMU_VOLTAGE)] = devdrv_manager_get_pmu_voltage,
        [_IOC_NR(DEVDRV_MANAGER_GET_PMU_DIEID)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_CPU_INFO)] = devdrv_manager_get_cpu_info,
        [_IOC_NR(DEVDRV_MANAGER_SEND_TO_IMU)] = devdrv_manager_imu_cmd,
        [_IOC_NR(DEVDRV_MANAGER_RECV_FROM_IMU)] = devdrv_manager_imu_cmd,
        [_IOC_NR(DEVDRV_MANAGER_GET_IMU_INFO)] = devdrv_manager_imu_cmd,
        [_IOC_NR(DEVDRV_MANAGER_CONFIG_ECC_ENABLE)] = devdrv_manager_imu_cmd,
        [_IOC_NR(DEVDRV_MANAGER_GET_PROBE_NUM)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_PROBE_LIST)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_SYNC_MATRIX_DAEMON_READY)] = device_manager_sync_matrix_ready,
        [_IOC_NR(DEVDRV_MANAGER_GET_BBOX_ERRSTR)] = devdrv_manager_imu_cmd,
        [_IOC_NR(DEVDRV_MANAGER_GET_CHIP_INFO)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_RST_I2C_CTROLLER)] = devdrv_manager_inquiry_info_ex,
        [_IOC_NR(DEVDRV_MANAGER_GET_XLOADER_BOOT_INFO)] = devdrv_manager_inquiry_info_ex,
        [_IOC_NR(DEVDRV_MANAGER_GET_GPIO_STATE)] = devdrv_manager_inquiry_info_ex,
        [_IOC_NR(DEVDRV_MANAGER_GET_SOC_DIE_ID)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_FLASH_USER_CMD)] = devdrv_flash_user_ioctl,
        [_IOC_NR(DEVDRV_MANAGER_FLASH_ROOT_CMD)] = devdrv_flash_root_ioctl,
        [_IOC_NR(DEVDRV_MANAGER_PCIE_BBOX_HDR_READ)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_APPMON_BBOX_EXCEPTION_CMD)] = devdrv_manager_appmon_bbox_exception_hndl,
        [_IOC_NR(DEVDRV_MANAGER_GET_PROCESS_SIGN)] = devdrv_manager_get_process_sign,
        [_IOC_NR(DEVDRV_MANAGER_PCIE_HOT_RESET)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_BIND_PID_ID)] = devdrv_fop_bind_host_pid,
        [_IOC_NR(DEVDRV_MANAGER_QUERY_DEV_PID)] = devdrv_query_devpid,
        [_IOC_NR(DEVDRV_MANAGER_GET_H2D_DEVINFO)] = devdrv_manager_devinfo_ioctl,
        [_IOC_NR(DEVDRV_MANAGER_GET_CONSOLE_LOG_LEVEL)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_CONTAINER_FLAG)] = devdrv_manager_get_container_flag,
        [_IOC_NR(DEVDRV_MANAGER_IPC_UNIFY_MSG)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_BOARD_INFO_MEM)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_UPDATE_STARTUP_STATUS)] = devdrv_manager_update_device_startup_status,
        [_IOC_NR(DEVDRV_MANAGER_QUERY_HOST_PID)] = devdrv_fop_query_host_pid,
        [_IOC_NR(DEVDRV_MANAGER_CONFIG_DEVICE_SHARE)] = devdrv_manager_config_device_share,
    };

void devdrv_manager_set_smoke_handler(int (*h)(struct file *filep, unsigned int cmd, unsigned long arg))
{
    devdrv_manager_ioctl_handlers[_IOC_NR(DEVDRV_MANAGER_LPM3_SMOKE)] = h;
}
EXPORT_SYMBOL(devdrv_manager_set_smoke_handler);

STATIC int devmng_custom_white_list_check(unsigned int cmd)
{
    enum devdrv_process_type cp_type = DEVDRV_PROCESS_CPTYPE_MAX;
    unsigned int i, chip_id, vfid, host_pid;

    (void)devdrv_query_process_host_pid(current->tgid, &chip_id, &vfid, &host_pid, &cp_type);

    if (cp_type == DEVDRV_PROCESS_CP2) {
        for (i = 0; i < DEVDRV_MANAGER_CMD_MAX_NR; i++) {
            if (cmd == g_devmng_custom_process_white_list[i])
                return 0;
        }

        devdrv_drv_err("this cmd is not support for custom_process, cmd = %u\n", _IOC_NR(cmd));
        return -EOPNOTSUPP;
    }

    return 0;
}

STATIC long devdrv_manager_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
    void __user *argp = (void __user *)((uintptr_t)arg);

    if ((dev_manager_info == NULL) || (argp == NULL) ||
        (_IOC_NR(cmd) >= DEVDRV_MANAGER_CMD_MAX_NR)) {
        devdrv_drv_err("invalid parameter, dev_manager_info = %pK, argp = %pK, "
                       "cmd = %u, max value = %u.\n",
                       dev_manager_info, argp, _IOC_NR(cmd), DEVDRV_MANAGER_CMD_MAX_NR);
        return -EINVAL;
    }

    if (devdrv_manager_ioctl_handlers[_IOC_NR(cmd)] == NULL) {
        devdrv_drv_err("invalid cmd, cmd = %u\n", _IOC_NR(cmd));
        return -EINVAL;
    }

    if (devmng_custom_white_list_check(cmd))
        return -EOPNOTSUPP;

    return devdrv_manager_ioctl_handlers[_IOC_NR(cmd)](filep, cmd, arg);
}

#ifndef DEVMNG_UT
void devdrv_manage_wake_up_msg_poll(void)
{
    wake_up_interruptible(&devdrv_manage_poll_wq);
}

unsigned int devdrv_manager_msg_poll(struct file *filep, struct poll_table_struct *wait)
{
    unsigned int ret = 0;
    unsigned long req_events;
    int rd_flag;

    req_events = poll_requested_events(wait);
    if (req_events & (POLLIN | POLLRDNORM)) {
        rd_flag = devdrv_ipc_msg_check();
        if (rd_flag == 0)
            poll_wait(filep, &devdrv_manage_poll_wq, wait);

        rd_flag = devdrv_ipc_msg_check();
        if (rd_flag == 1)
            ret = POLLIN | POLLRDNORM;

        return ret;
    }

    return POLLERR;
}
#endif

const struct file_operations devdrv_manager_file_operations = {
    .owner = THIS_MODULE,
    .open = devdrv_manager_open,
    .release = devdrv_manager_release,
#ifndef DEVMNG_UT
    .poll = devdrv_manager_msg_poll,
#endif
    .unlocked_ioctl = devdrv_manager_ioctl,
};
#ifndef CFG_SOC_PLATFORM_MINIV2
#define DEVDRV_MEMDUMP_ADDR 0x06E00000
#define DEVDRV_MEMDUMP_SIZE 0x00F00000
#else
#define DEVDRV_MEMDUMP_ADDR 0x2F900000
#define DEVDRV_MEMDUMP_SIZE 0x01E00000
#endif

#define SLOT_ID_BUFFER_KERNEL_LEN 4096
#define SLOT_ID_STRLEN 7

STATIC void devdrv_manager_get_slot_id(struct devdrv_info *dev_info)
{
    struct file *fp = NULL;
    char *ptr = NULL;
    ssize_t read_num;
    loff_t pos = 0;
    char *buffer=(char *)kzalloc(SLOT_ID_BUFFER_KERNEL_LEN * sizeof(char), GFP_KERNEL | __GFP_ACCOUNT);
    if (buffer == NULL) {
        devdrv_drv_err("kzalloc failed!\n");
        return;
    }

    fp = filp_open("/proc/cmdline", O_RDONLY, 0);
    if (IS_ERR(fp)) {
        devdrv_drv_err("open /proc/cmdline failed.\n");
        kfree(buffer);
        buffer = NULL;
        return;
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 16, 0)
    read_num = kernel_read(fp, buffer, SLOT_ID_BUFFER_KERNEL_LEN, &pos);
#else
    mm_segment_t old_fs;
    old_fs = get_fs();
    set_fs((mm_segment_t)KERNEL_DS);
    read_num = vfs_read(fp, buffer, SLOT_ID_BUFFER_KERNEL_LEN, &pos);
    set_fs(old_fs);
#endif
    if (read_num <= 0) {
        devdrv_drv_err("read /proc/cmdline failed.\n");
        filp_close(fp, NULL);
        fp = NULL;
        kfree(buffer);
        buffer = NULL;
        return;
    }

    buffer[SLOT_ID_BUFFER_KERNEL_LEN - 1] = '\0';
    ptr = strstr(buffer, "slotid=");
    if (ptr == NULL) {
        devdrv_drv_err("slotid is not found in cmdline\n");
        filp_close(fp, NULL);
        fp = NULL;
        kfree(buffer);
        buffer = NULL;
        return;
    }

    ptr += SLOT_ID_STRLEN;
    dev_info->shm_board->slot_id = (u16)simple_strtoul(ptr, NULL, 0);

    filp_close(fp, NULL);
    fp = NULL;
    kfree(buffer);
    buffer = NULL;
    return;
}

void devdrv_manager_get_dump_ddr_addr(u32 dev_id, u64 *dump_ddr_dma_addr, u32 *dump_ddr_size)
{
    u64 phy_addr = DEVDRV_MEMDUMP_ADDR + dev_id * CHIP_BASEADDR_PA_OFFSET;
    dma_addr_t dma_addr_d;
    struct device *dev = dev_manager_info->dma_dev[dev_id];
    u32 retry_times = 30;

    /* retry 30 times until dma_dev (get from pcie) is ready */
    while (dev == NULL && retry_times > 0) {
        msleep(2000);
        retry_times--;
        devdrv_drv_warn("Dma_dev is not ready, retry 2s later. (dev_id=%d)\n", dev_id);
        dev = dev_manager_info->dma_dev[dev_id];
    }

    if (dev == NULL) {
        devdrv_drv_warn("Dma_dev is not ready. (dev_id=%d)\n", dev_id);
        return;
    }

    *dump_ddr_dma_addr = phy_addr;
    *dump_ddr_size = DEVDRV_MEMDUMP_SIZE;

    dma_addr_d = dma_map_resource(dev, phy_addr, *dump_ddr_size, DMA_BIDIRECTIONAL, 0);
    if (dma_mapping_error(dev, dma_addr_d)) {
        devdrv_drv_warn("dev_id %d dma map error\n", dev_id);
        return;
    }
    *dump_ddr_dma_addr = (u64)dma_addr_d;
}

void devdrv_manager_inform_device_status(struct devdrv_info *info, enum devdrv_ts_status status)
{
    struct devdrv_manager_msg_info dev_manager_msg_info = { { 0 }, { 0 } };
    u32 out_len;
    int ret;

    switch (status) {
        case TS_WORK:
            dev_manager_msg_info.header.msg_id = DEVDRV_MANAGER_CHAN_D2H_RESUME;
            break;
        case TS_SUSPEND:
            dev_manager_msg_info.header.msg_id = DEVDRV_MANAGER_CHAN_D2H_SUSNPEND;
            break;
        case TS_DOWN:
            dev_manager_msg_info.header.msg_id = DEVDRV_MANAGER_CHAN_D2H_DOWN;
            break;
        case TS_FAIL_TO_SUSPEND:
            dev_manager_msg_info.header.msg_id = DEVDRV_MANAGER_CHAN_D2H_FAIL_TO_SUSPEND;
            break;
        default:
            devdrv_drv_err("invalid input status.\n");
            return;
    }

    dev_manager_msg_info.header.valid = (u16)DEVDRV_MANAGER_MSG_VALID;
    dev_manager_msg_info.header.result = (u16)DEVDRV_MANAGER_MSG_INVALID_RESULT;

    ret = agentdrv_common_msg_send(info->dev_id, &dev_manager_msg_info, sizeof(dev_manager_msg_info),
        sizeof(dev_manager_msg_info), &out_len, AGENTDRV_COMMON_MSG_DEVDRV_MANAGER);
    if ((ret) || dev_manager_msg_info.header.result != 0)
        devdrv_drv_err("inform host failed\n");
}

STATIC void devdrv_manager_inform_ai_system_ready(struct work_struct *work)
{
    struct devdrv_manager_msg_info dev_manager_msg_info = { { 0 }, { 0 } };
    struct delayed_work *dwork = to_delayed_work(work);
    struct devdrv_device_info *drv_info = NULL;
    struct devdrv_info *dev_info = NULL;
    unsigned long timeout;
    int boardid;
    u32 out_len;
    int ret;

    devdrv_drv_info("inform ai system ready delay start\n");

    dev_info = container_of(dwork, struct devdrv_info, fw_load_wq);
    if (dev_info->fw_wq_retry-- == 0) {
        devdrv_drv_info("finish inform retry\n");
        return;
    }

    boardid = devdrv_get_boardid();
    /* try to inform host ai subsystem is ready, if boardid indicates it is minirc, set timeout to 50 */
    if (dev_info->env_type == DEVDRV_PLAT_TYPE_EMU || boardid == 1000 || boardid == 2000 || boardid == 3004 ||
        boardid == 4004 || boardid == 1004) {
        timeout = msecs_to_jiffies(50);
    } else {
        timeout = msecs_to_jiffies(300000);
    }

    ret = wait_event_interruptible_timeout(dev_manager_info->msg_chan_wait[dev_info->dev_id],
        dev_manager_info->msg_chan_rdy[dev_info->dev_id] == 1, timeout);
    if (ret <= 0) {
        devdrv_drv_warn("Msg channel unable to setup, there is no host. (dev_id=%u)\n", dev_info->dev_id);
        return;
    }

    dev_manager_msg_info.header.msg_id = DEVDRV_MANAGER_CHAN_D2H_DEVICE_READY;
    dev_manager_msg_info.header.valid = (u16)DEVDRV_MANAGER_MSG_VALID;
    dev_manager_msg_info.header.result = (u16)DEVDRV_MANAGER_MSG_INVALID_RESULT;

    drv_info = (struct devdrv_device_info *)dev_manager_msg_info.payload;
    drv_info->ai_core_num = dev_info->ai_core_num;
    drv_info->ai_cpu_core_num = dev_info->ai_cpu_core_num;
    drv_info->ctrl_cpu_core_num = dev_info->ctrl_cpu_core_num;
    drv_info->ctrl_cpu_occupy_bitmap = dev_info->ctrl_cpu_occupy_bitmap;
    drv_info->ctrl_cpu_endian_little = dev_info->ctrl_cpu_endian_little;
    drv_info->ctrl_cpu_id = dev_info->ctrl_cpu_id;
    drv_info->ctrl_cpu_ip = dev_info->ctrl_cpu_ip;
    drv_info->ts_cpu_core_num = dev_info->pdata->ts_pdata[0].ts_cpu_core_num;
    drv_info->ai_core_id = dev_info->ai_core_id;
    drv_info->ai_cpu_core_id = dev_info->ai_cpu_core_id;
    drv_info->aicpu_occupy_bitmap = dev_info->aicpu_occupy_bitmap;
    drv_info->env_type = dev_info->env_type;

    drv_info->ai_core_ready_num = dev_info->inuse.ai_core_num;
    drv_info->ai_core_broken_map = dev_info->inuse.ai_core_error_bitmap;
    drv_info->ai_cpu_ready_num = dev_info->inuse.ai_cpu_num;
    drv_info->ai_cpu_broken_map = dev_info->inuse.ai_cpu_error_bitmap;
    drv_info->ai_subsys_ip_map = dev_info->ai_subsys_ip_broken_map;
    drv_info->hardware_version = dev_info->hardware_version;
    drv_info->ts_load_fail = dev_info->pdata->ts_pdata[0].ts_load_fail;
    drv_info->chip_name = dev_info->chip_name;
    drv_info->chip_version = dev_info->chip_version;
    drv_info->chip_info = dev_info->chip_info;

    devdrv_drv_debug("Initialize chip info. (chip_name=%u; chip_version=%u)\n",
        dev_info->chip_name, dev_info->chip_version);

    devdrv_manager_get_dump_ddr_addr(dev_info->dev_id, &drv_info->dump_ddr_dma_addr, &drv_info->dump_ddr_size);

    devdrv_drv_debug(" ai_core_num = %d, ai_cpu_core_num = %d, "
        "ctrl_cpu_core_num = %d, "
        "ctrl_cpu_endian_little = %d, "
        "ctrl_cpu_id = %d, "
        "ctrl_cpu_ip = %d, "
        "ts_cpu_core_num = %d, "
        "ai_core_id = %d, "
        "ai_cpu_core_id = %d\n",
        dev_info->ai_core_num, dev_info->ai_cpu_core_num, dev_info->ctrl_cpu_core_num, dev_info->ctrl_cpu_endian_little,
        dev_info->ctrl_cpu_id, dev_info->ctrl_cpu_ip, 0, dev_info->ai_core_id,
        dev_info->ai_cpu_core_id);

    ret = agentdrv_common_msg_send(dev_info->dev_id, &dev_manager_msg_info, sizeof(dev_manager_msg_info),
        sizeof(dev_manager_msg_info), &out_len, AGENTDRV_COMMON_MSG_DEVDRV_MANAGER);
    if (ret != 0) {
        devdrv_drv_info("Delay inform device ready abnormal, try again. (ret=%d; result=%d)\n",
            ret, dev_manager_msg_info.header.result);
        queue_delayed_work(devdrv_manager_inform_wq, &dev_info->fw_load_wq, DEVMNG_FW_WQ_DELAY_TIME);
        return;
    }

    devdrv_drv_info("*** delay inform device ready info succeed***\n");
    return;
}

STATIC void devdrv_manager_send_device_info_to_host(struct devdrv_info *dev_info)
{
    INIT_DELAYED_WORK(&dev_info->fw_load_wq, devdrv_manager_inform_ai_system_ready);
    queue_delayed_work(devdrv_manager_inform_wq, &dev_info->fw_load_wq, 0);
}

/* the log module wants to call this api in order to normalization code, we just do nothing */
int devdrv_manager_send_tslog_addr_to_host(u32 devid, u64 phy_addr, u32 mem_size, bool dynamic_alloc)
{
    return 0;
}
EXPORT_SYMBOL(devdrv_manager_send_tslog_addr_to_host);

STATIC int devdrv_manager_nfe_irq_register(struct devdrv_info *dev_info)
{
    struct devdrv_platform_data *pdata = NULL;
    int ret;

    pdata = (struct devdrv_platform_data *)dev_info->pdata;
    tasklet_init(&dev_info->nfe_task, devdrv_ts_exception_task, ((unsigned long)(uintptr_t)dev_info));
    ret = request_irq(pdata->ts_pdata[0].disp_nfe_irq, devdrv_nfe_handler, IRQF_TRIGGER_RISING | IRQF_SHARED,
        "nfe-exception", dev_info);
    if (ret)
        devdrv_drv_err("request irq failed\n");

    return ret;
}

STATIC void devdrv_check_lpm3(void)
{
    u32 __iomem *lpm3_status = NULL;
    struct timespec stamp;
    excep_time timestamp;
    u8 __iomem *status = NULL;
    u32 value;

#define LPM3_STATUS_SIZE 0x1000
#define LPM3_STATUS_OFFSET 0x0414
#define LPM3_STATUS_SUCC 0xFFFF1F1F
#define LPM3_STARTUP_EXCEPTION 0xA819FFFF

    status = devm_ioremap(dev_manager_info->dev, DEVDRV_MANAGER_PERI_LPM3_STATUS_REG,
        DEVDRV_MANAGER_PERI_LPM3_STATUS_SIZE);
    if (status == NULL) {
        devdrv_drv_err("ioremap lpm3's status failed.\n");
        return;
    }
    mb();
    lpm3_status = (u32 __iomem *)(status + LPM3_STATUS_OFFSET);
    value = readl_relaxed(lpm3_status);
    if (value != LPM3_STATUS_SUCC) {
        devdrv_drv_err("lpm3 status: exception.\n");

        devdrv_lpm3_start_fail = 1;

        stamp = current_kernel_time();
        timestamp.tv_sec = stamp.tv_sec;
        timestamp.tv_usec = stamp.tv_nsec / 1000;
        mntn_system_error(LPM3_STARTUP_EXCEPTION, timestamp, 0);
    }

    devm_iounmap(dev_manager_info->dev, status);
}

STATIC int devmng_shm_init(struct devdrv_info *dev_info)
{
    size_t shm_size;
    u64 shm_addr;
    int ret, i;

    /* rc mode isn't need init shm */
    if ((dev_manager_info == NULL) ||
        (dev_manager_info->machine_mode == DEVDRV_PCIE_RC_MODE)) {
        return 0;
    }

    ret = agentdrv_get_addr_info(dev_info->dev_id, DEVDRV_ADDR_DEVMNG_RESV_BASE,
                                 0, &shm_addr, &shm_size);
    if (ret) {
        devdrv_drv_err("[devid=%u] get shm addr fail, ret=%d.\n", dev_info->dev_id, ret);
        return ret;
    }

    dev_info->shm_vaddr = ioremap_cache(shm_addr, shm_size);
    if (dev_info->shm_vaddr == NULL) {
        devdrv_drv_err("[devid=%u] ioremap shm_vaddr fail.\n", dev_info->dev_id);
        return -ENOMEM;
    }

    dev_info->shm_head = (U_SHM_INFO_HEAD __iomem *)((uintptr_t)dev_info->shm_vaddr);
    dev_info->shm_head->head_info.magic = DEVMNG_SHM_INFO_HEAD_MAGIC;
    dev_info->shm_head->head_info.version = DEVMNG_SHM_INFO_HEAD_VERSION;
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

    dev_info->shm_status->event_cnt = 0;
    for (i = 0; i < DEVMNG_SHM_INFO_EVENT_CODE_LEN; i++) {
        dev_info->shm_status->event_code[i].event_code = 0;
        dev_info->shm_status->event_code[i].fid = 0;
    }
    for (i = 0; i < VMNG_VDEV_MAX_PER_PDEV; i++) {
        dev_info->shm_status->dms_health_status[i] = 0;
    }
    return 0;
}

STATIC void devmng_shm_uninit(struct devdrv_info *dev_info)
{
    /* rc mode isn't need uninit shm */
    if ((dev_manager_info == NULL) ||
        (dev_manager_info->machine_mode == DEVDRV_PCIE_RC_MODE)) {
        return;
    }

    if (dev_info->shm_vaddr != NULL)
        iounmap(dev_info->shm_vaddr);

    dev_info->shm_vaddr = NULL;
    dev_info->shm_head = NULL;
    dev_info->shm_soc = NULL;
    dev_info->shm_board = NULL;
    dev_info->shm_status = NULL;
    return;
}

#ifdef CFG_FEATURE_HEALTH_ERR_CODE
STATIC void devmng_refresh_error_code_work_init(struct devdrv_info *dev_info)
{
    /* rc mode isn't need init heart beat work */
    if ((dev_manager_info == NULL) ||
        (dev_manager_info->machine_mode == DEVDRV_PCIE_RC_MODE)) {
        return;
    }

    dev_info->heart_beat_wq = create_workqueue("heart_beat_work");
    if (dev_info->heart_beat_wq == NULL) {
        devdrv_drv_err("create_workqueue error. device(%u)\n", dev_info->dev_id);
        return;
    }

    INIT_WORK(&dev_info->work, devdrv_fresh_error_code_to_shm);
    return;
}

STATIC void devmng_heart_beat_work_uninit(struct devdrv_info *dev_info)
{
    /* rc mode isn't need uninit heart beat work */
    if ((dev_manager_info == NULL) ||
        (dev_manager_info->machine_mode == DEVDRV_PCIE_RC_MODE)) {
        return;
    }

    cancel_work_sync(&dev_info->work);
    if (dev_info->heart_beat_wq != NULL) {
        destroy_workqueue(dev_info->heart_beat_wq);
        dev_info->heart_beat_wq = NULL;
    }
    return;
}
#endif

STATIC int devdrv_manager_set_chip_info(struct devdrv_info *dev_info)
{
    int ret;
    soc_chip_ver_reg_t chip_info = {0};

    ret = devdrv_reg_op(DEVDRV_REG_RD, (SOC_CHIP_INFO_REG_BASE + dev_info->dev_id * SOC_CHIP_OFFSET),
        SOC_CHIP_INFO_REG_OFFSET, SOC_CHIP_INFO_MAP_SIZE, (unsigned int *)&chip_info);
    if (ret != 0) {
        devdrv_drv_err("Read chip info register failed. (dev_id=%u; ret=%d)\n", dev_info->dev_id, ret);
        return ret;
    }

    dev_info->chip_name = chip_info.chip_name;
    dev_info->chip_version = chip_info.chip_ver;
    dev_info->chip_info = *(unsigned int *)&chip_info;

    devdrv_drv_info("Get chip info. (chip_name=%u; chip_version=%u)\n", dev_info->chip_name, dev_info->chip_version);
    return 0;
}

int devdrv_manager_register(struct devdrv_info *dev_info)
{
    struct devdrv_platform_data *pdata = NULL;
    bbox_tsconfig *config = NULL;
    unsigned long flags;
    u8 flag_judge;
    int ret;

    devdrv_check_lpm3();

    if (dev_manager_info == NULL || dev_info == NULL || dev_info->pdata == NULL) {
        devdrv_drv_err("devdrv manager has not initialized\n");
        return -EINVAL;
    }

    ret = dms_device_register(dev_info);
    if (ret) {
        devdrv_drv_err("Dms device register failed. (dev_id=%u)\n", dev_info->dev_id);
        return -ENODEV;
    }

    flag_judge =
        ((dev_info->dev_id >= DEVDRV_MAX_DAVINCI_NUM) || (dev_manager_info->dev_info[dev_info->dev_id] != NULL));
    if (flag_judge) {
        dms_device_unregister(dev_info);
        devdrv_drv_err("dev_id = %u, has been registered or id invalid.\n", dev_info->dev_id);
        return -ENODEV;
    }

    ret = devdrv_manager_create_one_device(dev_info);
    if (ret) {
        dms_device_unregister(dev_info);
        devdrv_drv_err("devdrv_manager_create_one_device return error, ret: %d.\n", ret);
        return -ENOMEM;
    }

    ret = devmng_shm_init(dev_info);
    if (ret) {
        devdrv_manager_release_one_device(dev_info);
        dms_device_unregister(dev_info);
        devdrv_drv_err("[dev_id = %u]:shm init fail, ret = %d\n", dev_info->dev_id, ret);
        return -ENOMEM;
    }

    if (devdrv_is_pci_rc_mode() == false) {
        dev_info->shm_board->board_id = (u16)dev_info->board_id;
        devdrv_manager_get_slot_id(dev_info);
    }

    (void)ipc_mailbox_rx_register(dev_info->dev_id);
    devdrv_freq_control_register(dev_info->dev_id);
    devdrv_ipc_aicpu_register(dev_info->dev_id);
    (void)tsmng_mailbox_rx_register(dev_info->dev_id);

    dev_info->fw_wq_retry = DEVMNG_FW_WQ_RETRY_COUNT;
    dev_info->plat_type = (u8)DEVDRV_MANAGER_DEVICE_ENV;
    dev_info->dmp_started = false;
    dev_info->drv_ops = &devdrv_platform_drv_ops;

    pdata = (struct devdrv_platform_data *)dev_info->pdata;

    spin_lock_irqsave(&dev_manager_info->spinlock, flags);
    dev_manager_info->dev_info[dev_info->dev_id] = dev_info;
    dev_manager_info->dev_id[dev_info->dev_id] = dev_info->dev_id;
    dev_manager_info->num_dev++;
    spin_unlock_irqrestore(&dev_manager_info->spinlock, flags);

    tsdrv_set_ts_status(dev_info->dev_id, 0, TS_BOOTING);
    devdrv_drv_info("*** set status booting ***\n");

    /* bbox_get_tsconfig should be called before tsdrv_firmware_load */
    config = bbox_get_tsconfig(dev_info->dev_id);
    if (config == NULL) {
        devdrv_drv_info("bbox_get_tsconfig return NULL.\n");
        dev_info->fw_info.ts_blackbox_base = 0;
        dev_info->fw_info.ts_blackbox_size = 0;
        dev_info->fw_info.ts_start_log_base = 0;
        dev_info->fw_info.ts_start_log_size = 0;
        dev_info->fw_info.enable_bbox = 0;
    } else {
        devdrv_drv_info("bbox_get_tsconfig return valid config.\n");

        dev_info->fw_info.ts_blackbox_base = config->ts_paddr[TS_MNTN_BUFFER].addr;
        dev_info->fw_info.ts_blackbox_size = (u64)config->ts_paddr[TS_MNTN_BUFFER].len;
        dev_info->fw_info.ts_start_log_base = config->ts_paddr[TS_MNTN_START_LOG_BUFFER].addr;
        dev_info->fw_info.ts_start_log_size = (u64)config->ts_paddr[TS_MNTN_START_LOG_BUFFER].len;

        dev_info->fw_info.enable_bbox = config->enable_bbox;

        devdrv_drv_info("ts blackbox base: %pK, size: %lld.\n", (void *)(uintptr_t)dev_info->fw_info.ts_blackbox_base,
            dev_info->fw_info.ts_blackbox_size);
        devdrv_drv_info("ts start log base: %pK, size: %lld.\n", (void *)(uintptr_t)dev_info->fw_info.ts_start_log_base,
            dev_info->fw_info.ts_start_log_size);

        devdrv_drv_info("enable bbox: %d.\n", dev_info->fw_info.enable_bbox);
    }

    if (dev_info->drv_ops->tsdrv_firmware_load != NULL) {
        dev_info->drv_ops->tsdrv_firmware_load(dev_info);
    } else {
        devdrv_drv_err("load ts firmware error. handler is null.\n");
    }

#ifdef CFG_FEATURE_TIMESYNC
    dms_time_sync_info_init(dev_info->dev_id);
#endif

#ifdef CFG_FEATURE_HEALTH_ERR_CODE
    devmng_refresh_error_code_work_init(dev_info);
    devdrv_refresh_error_code_init();
#endif

    (void)devdrv_manager_set_chip_info(dev_info);

    /* send device info to host */
    devdrv_manager_send_device_info_to_host(dev_info);

    ret = devdrv_manager_nfe_irq_register(dev_info);
    if (ret) {
        devdrv_drv_err("request irq failed.\n");
#ifdef CFG_FEATURE_HEALTH_ERR_CODE
        devdrv_refresh_error_code_exit();
        devmng_heart_beat_work_uninit(dev_info);
#endif
        devmng_shm_uninit(dev_info);
#ifdef CFG_FEATURE_TIMESYNC
        dms_time_sync_info_free(dev_info->dev_id);
#endif
        devdrv_tscpu_free_memory(dev_info, dev_info->dev, DEVDRV_TS_MEMORY_SIZE,
            pdata->ts_pdata[0].ts_load_addr, pdata->ts_pdata[0].ts_dma_handle);
        spin_lock_irqsave(&dev_manager_info->spinlock, flags);
        dev_manager_info->dev_info[dev_info->dev_id] = NULL;
        dev_manager_info->num_dev--;
        spin_unlock_irqrestore(&dev_manager_info->spinlock, flags);
        dms_device_unregister(dev_info);
        return -ENOMEM;
    }

    devdrv_drv_info("register gpio irq for lc...(%d)\n", devdrv_lc_gpioirq_register());

    return 0;
}
EXPORT_SYMBOL(devdrv_manager_register); //lint !e508

void devdrv_manager_unregister(struct devdrv_info *dev_info)
{
    struct devdrv_platform_data *pdata = NULL;
    unsigned long flags;

    if (dev_info == NULL || dev_manager_info == NULL || dev_info->dev_id >= DEVDRV_MAX_DAVINCI_NUM)
        return;

    if (dev_manager_info->dev_info[dev_info->dev_id] == NULL) {
        devdrv_drv_err("device id %u is not initialized\n", dev_info->dev_id);
        return;
    }

    pdata = (struct devdrv_platform_data *)dev_info->pdata;
    if (pdata == NULL) {
        devdrv_drv_err("The pdata is NULL. (devid=%u)\n", dev_info->dev_id);
        return;
    }
    spin_lock_irqsave(&dev_manager_info->spinlock, flags);
    dev_manager_info->dev_info[dev_info->dev_id] = NULL;
    dev_manager_info->num_dev--;
    spin_unlock_irqrestore(&dev_manager_info->spinlock, flags);

#ifdef CFG_FEATURE_HEALTH_ERR_CODE
    devdrv_refresh_error_code_exit();
    devmng_heart_beat_work_uninit(dev_info);
#endif
    devmng_shm_uninit(dev_info);
    free_irq(pdata->ts_pdata[0].disp_nfe_irq, dev_info);
    devdrv_tscpu_free_memory(dev_info, dev_info->dev, DEVDRV_TS_MEMORY_SIZE,
        pdata->ts_pdata[0].ts_load_addr, pdata->ts_pdata[0].ts_dma_handle);
    ipc_mailbox_rx_unregister(dev_info->dev_id);
    tsmng_mailbox_rx_unregister(dev_info->dev_id);
    devdrv_manager_release_one_device(dev_info);
    dms_device_unregister(dev_info);
}
EXPORT_SYMBOL(devdrv_manager_unregister);

STATIC int devdrv_manager_get_devid_from_host(u32 devid, void *msg, u32 in_len, u32 *ack_len)
{
    struct devdrv_manager_msg_info *dev_manager_msg_info;
    unsigned long flags;

    dev_manager_msg_info = (struct devdrv_manager_msg_info *)msg;
    if ((dev_manager_msg_info->header.valid != DEVDRV_MANAGER_MSG_VALID) ||
        (in_len < sizeof(struct devdrv_manager_msg_info))) {
        devdrv_drv_err("Invalid message from host. (valid=%u; in_len=%u)\n",
                       dev_manager_msg_info->header.valid, in_len);
        return -EINVAL;
    }

    spin_lock_irqsave(&dev_manager_info->spinlock, flags);
    dev_manager_info->dev_id_flag[devid] = 1;
    dev_manager_info->dev_id[devid] = dev_manager_msg_info->header.dev_id;
    spin_unlock_irqrestore(&dev_manager_info->spinlock, flags);

    dev_manager_msg_info->header.result = 0;
    *ack_len = sizeof(*dev_manager_msg_info);

    return 0;
}

STATIC int devdrv_manager_refresh_aicore_info(u32 devid, void *msg, u32 in_len, u32 *ack_len)
{
    struct devdrv_manager_msg_info *dev_manager_msg_info = NULL;
    struct devdrv_aicore_msg *h2d_msg = NULL;
    struct devdrv_info *dev_info = NULL;

    dev_manager_msg_info = (struct devdrv_manager_msg_info *)msg;
    if ((dev_manager_msg_info->header.valid != DEVDRV_MANAGER_MSG_VALID) ||
        (in_len < sizeof(struct devdrv_manager_msg_info))) {
        devdrv_drv_err("Invalid message from host. (valid=%u; in_len=%u)\n",
                       dev_manager_msg_info->header.valid, in_len);
        return -EINVAL;
    }

    dev_manager_msg_info->header.result = 0;
    *ack_len = sizeof(*dev_manager_msg_info);

    h2d_msg = (struct devdrv_aicore_msg *)dev_manager_msg_info->payload;

    if (dev_manager_info == NULL || dev_manager_info->dev_info[devid] == NULL) {
        devdrv_drv_err("no device.\n");
        h2d_msg->ts_status = 0;
        h2d_msg->syspcie_sysdma_status = 0;
        h2d_msg->aicore_bitmap = 0;
        h2d_msg->aicpu_heart_beat_exception = 0;
        return 0;
    }

    dev_info = dev_manager_info->dev_info[devid];

    h2d_msg->aicore_bitmap = dev_info->inuse.ai_core_error_bitmap;
    h2d_msg->aicpu_heart_beat_exception = dev_info->inuse.ai_cpu_error_bitmap;
    h2d_msg->ts_status = 0;
    h2d_msg->syspcie_sysdma_status = 0;

    return 0;
}

int devdrv_manager_get_task_status(u32 devid, void *msg, u32 in_len, u32 *ack_len)
{
    struct devdrv_manager_msg_info *dev_manager_msg_info = NULL;
    u32 *is_cancelled = NULL;
    struct devdrv_info *dev_info = NULL;
    int dev_id;

    dev_manager_msg_info = (struct devdrv_manager_msg_info *)msg;
    if ((dev_manager_msg_info->header.valid != DEVDRV_MANAGER_MSG_H2D_MAGIC) ||
        (in_len < sizeof(struct devdrv_manager_msg_info))) {
        devdrv_drv_err("Invalid message from host. (valid=%u; in_len=%u)\n",
                       dev_manager_msg_info->header.valid, in_len);
        return -EINVAL;
    }

    is_cancelled = (u32 *)dev_manager_msg_info->payload;
    *is_cancelled = 0UL;
    dev_id = dev_manager_msg_info->header.dev_id;
    dev_info = devdrv_manager_get_devdrv_info(devid);
    if (dev_id != devdrv_manager_get_devid(devid) || !dev_info) {
        devdrv_drv_warn("invalid device id, dev_id = %d, "
            "dev_manager_info->dev_id = %d, "
            "dev_info is %s\n",
            dev_id, devdrv_manager_get_devid(devid), (!dev_info) ? "NULL" : "not NULL");
        return -EAGAIN;
    }

    dev_manager_msg_info->header.result = 0;
    dev_manager_msg_info->header.valid = DEVDRV_MANAGER_MSG_D2H_MAGIC;
    *ack_len = sizeof(*dev_manager_msg_info);
    return 0;
}

int devdrv_manager_get_device_pid(u32 devid, void *msg, u32 in_len, u32 *ack_len)
{
    struct devdrv_manager_msg_info *dev_manager_msg_info = NULL;
    struct devdrv_ioctl_para_query_pid *para_info = NULL;
    int ret;

    dev_manager_msg_info = (struct devdrv_manager_msg_info *)msg;
    if ((dev_manager_msg_info->header.valid != DEVDRV_MANAGER_MSG_VALID) ||
        (in_len < sizeof(struct devdrv_manager_msg_info))) {
        devdrv_drv_err("Invalid message from host. (valid=%u; in_len=%u)\n",
                       dev_manager_msg_info->header.valid, in_len);
        return -EINVAL;
    }

    para_info = (struct devdrv_ioctl_para_query_pid *)dev_manager_msg_info->payload;
    ret = devdrv_query_process_by_host_pid(para_info->host_pid, devid,
          para_info->cp_type, para_info->vfid, &(para_info->pid));
    if (ret != 0 && ret != DRV_ERROR_NO_PROCESS) {
        devdrv_drv_warn("query device pid failed, ret(%d), devid(%u)\n", ret, devid);
        return -EINVAL;
    }

    dev_manager_msg_info->header.result = ret;
    dev_manager_msg_info->header.valid = DEVDRV_MANAGER_MSG_VALID;
    *ack_len = sizeof(struct devdrv_ioctl_para_query_pid) + sizeof(struct devdrv_manager_msg_head);
    return 0;
}

static int check_hostpid_devpid_bind_status(struct devdrv_process_sign *d_sign, unsigned int dev_id, int dev_pid,
                                            struct dev_pid_info* dev_pid_info_list, u32 in_size, u32* out_size)
{
    u32 vfid;
    enum devdrv_process_type type;

    for (vfid = 0; vfid < VFID_NUM_MAX; vfid++) {
        for (type = 0; type < DEVDRV_PROCESS_CPTYPE_MAX; type++) {
            if (d_sign->devpid[dev_id][vfid][type] != dev_pid) {
                continue;
            }
            if (in_size == 0) {
                return 0;
            }
            dev_pid_info_list[*out_size].dev_id = dev_id;
            dev_pid_info_list[*out_size].vfid = vfid;
            dev_pid_info_list[*out_size].cp_type = type;
            *out_size += 1;
            if (*out_size >= in_size) {
                return -ENOMEM;
            }
        }
    }

    if (*out_size == 0) {
        return -ESRCH;
    }
    return 0;
}

/*
* 1. This API is used to check the bind status between the two inputs, host_pid and dev_pid.
* 2. Also, if the host_pid and dev_pid were bound,
* this API will output the dev_pid info in the structure array, "dev_pid_info",
* and output the array length by "out_size".
* 3. If Users don't care the dev_pid info, the input para "in_size" can set to 0,
* and the "dev_pid_info" and "out_size" set to NULL.
*/
bool devdrv_process_is_bind(pid_t host_pid, pid_t dev_pid, struct dev_pid_info* dev_pid_info_list,
                            u32 in_size, u32* out_size)
{
    struct devdrv_manager_info *d_info = devdrv_get_manager_info();
    struct devdrv_process_sign *d_sign = NULL;
    u32 key;
    int ret;
    u32 i;
    u32 size = 0;

    if (in_size != 0 && (dev_pid_info_list == NULL || out_size == NULL)) {
        devdrv_drv_err("Invalid para. (in_size=%d, dev_pid_info_list is NULL=%d, out_size is NULL=%d)\n",
            in_size, dev_pid_info_list == NULL, out_size == NULL);
        return false;
    }

    key = (u32)host_pid & DEVDRV_PROC_HASH_TABLE_MASK;
    spin_lock_bh(&d_info->proc_hash_table_lock);
    hash_for_each_possible(d_info->proc_hash_table, d_sign, link, key) {
        if (host_pid != d_sign->hostpid) {
            continue;
        }
        for (i = 0; i < DEVDRV_MAX_NODE_NUM; i++) {
            ret = check_hostpid_devpid_bind_status(d_sign, i, dev_pid, dev_pid_info_list, in_size, &size);
            if (ret == 0 && in_size == 0) {
                spin_unlock_bh(&d_info->proc_hash_table_lock);
                return true;
            }

            if (ret == -ENOMEM) {
                spin_unlock_bh(&d_info->proc_hash_table_lock);
                *out_size = size;
                devdrv_drv_event("The size of the dev_pid_info_list is too small. (in_size=%u, out_size=%u)\n",
                    in_size, *out_size);
                return true;
            }
        }
        break;
    }

    spin_unlock_bh(&d_info->proc_hash_table_lock);
    if (size == 0) {
        return false;
    }
    *out_size = size;
    return true;
}
EXPORT_SYMBOL(devdrv_process_is_bind);

/* not surport, stub for others */
int devdrv_query_master_pid_by_host_slave(int slave_pid, u32 *master_pid)
{
    return -EINVAL;
}
EXPORT_SYMBOL(devdrv_query_master_pid_by_host_slave);

int devdrv_manager_notice_device_exit(u32 devid, void *msg, u32 in_len, u32 *ack_len)
{
    struct devdrv_manager_msg_info *dev_manager_msg_info = NULL;
    struct devdrv_ioctl_para_query_pid *para_info = NULL;

    dev_manager_msg_info = (struct devdrv_manager_msg_info *)msg;
    if ((dev_manager_msg_info->header.valid != DEVDRV_MANAGER_MSG_VALID) ||
        (in_len < sizeof(struct devdrv_manager_msg_info))) {
        devdrv_drv_err("Invalid message from host. (dev_id=%u; valid=%u; in_len=%u)\n",
                       devid, dev_manager_msg_info->header.valid, in_len);
        return -EINVAL;
    }
    para_info = (struct devdrv_ioctl_para_query_pid *)dev_manager_msg_info->payload;
    para_info->pid = devdrv_notice_process_exit(para_info->host_pid);
    dev_manager_msg_info->header.result = 0;
    dev_manager_msg_info->header.valid = DEVDRV_MANAGER_MSG_VALID;
    *ack_len = sizeof(struct devdrv_ioctl_para_query_pid) + sizeof(struct devdrv_manager_msg_head);

    return 0;
}

STATIC int devdrv_manager_get_dmp_started(u32 devid, void *msg, u32 in_len, u32 *ack_len)
{
    struct devdrv_manager_msg_info *dev_manager_msg_info = NULL;
    struct devdrv_info *dev_info = NULL;
    u32 *dmp_started = NULL;

    dev_manager_msg_info = (struct devdrv_manager_msg_info *)msg;
    if ((dev_manager_msg_info->header.valid != DEVDRV_MANAGER_MSG_VALID) ||
        (in_len < sizeof(struct devdrv_manager_msg_info))) {
        devdrv_drv_err("Invalid message from host. (valid=%u; in_len=%u)\n",
                       dev_manager_msg_info->header.valid, in_len);
        return -EINVAL;
    }

    *ack_len = sizeof(*dev_manager_msg_info);
    dmp_started = (u32 *)dev_manager_msg_info->payload;

    dev_info = devdrv_get_default_devdrv_info();
    if (dev_info == NULL)
        *dmp_started = false;
    else
        *dmp_started = dev_info->dmp_started;

    dev_manager_msg_info->header.result = DEVDRV_MANAGER_MSG_VALID;
    return 0;
}

devmm_get_device_process_memory_ops get_process_mem_from_devmm = NULL;
int devdrv_manager_get_process_memory_register(devmm_get_device_process_memory_ops func)
{
    if (func == NULL) {
        devdrv_drv_err("Register process memory operation function null.\n");
        return -EINVAL;
    }

    get_process_mem_from_devmm = func;
    return 0;
}
EXPORT_SYMBOL(devdrv_manager_get_process_memory_register);

void devdrv_manager_get_process_memory_unregister(void)
{
    get_process_mem_from_devmm = NULL;
}
EXPORT_SYMBOL(devdrv_manager_get_process_memory_unregister);

STATIC int devdrv_get_dev_mem_process(u32 devid, struct devdrv_manager_msg_resource_info *res_info)
{
    u64 memory_size = 0;
    int ret;

    if (get_process_mem_from_devmm == NULL) {
        devdrv_drv_err("The devmm_get_device_process_memory is NULL");
        return -EINVAL;
    }
    ret = get_process_mem_from_devmm(devid, res_info->vfid, res_info->owner_id, &memory_size);
    if (ret) {
        devdrv_drv_err("The devmm_get_device_process_memory failed. (device_id=%u; vfid=%u; hostpid=%d)\n",
            devid, res_info->vfid, res_info->owner_id);
        return -EINVAL;
    }
    res_info->value = memory_size;
    return ret;
}

STATIC int (*devdrv_get_resource_info[DEVDRV_DEV_INFO_TYPE_MAX])(u32 devid,
    struct devdrv_manager_msg_resource_info *resource_info) = {
        [DEVDRV_DEV_DDR_TOTAL] = NULL,
        [DEVDRV_DEV_DDR_FREE] = NULL,
        [DEVDRV_DEV_HBM_TOTAL] = NULL,
        [DEVDRV_DEV_HBM_FREE] = NULL,
        [DEVDRV_DEV_PROCESS_MEM] = devdrv_get_dev_mem_process,
};

STATIC int devdrv_manager_get_resource_info(u32 devid, void *msg, u32 in_len, u32 *ack_len)
{
    struct devdrv_manager_msg_info *dev_manager_msg_info = NULL;
    struct devdrv_manager_msg_resource_info *resource_info = NULL;
    int ret;

    dev_manager_msg_info = (struct devdrv_manager_msg_info *)msg;
    if ((dev_manager_msg_info->header.valid != DEVDRV_MANAGER_MSG_VALID) ||
        (in_len < sizeof(struct devdrv_manager_msg_info))) {
        devdrv_drv_err("Invalid message from host. (device_id=%u; valid=%u; in_len=%u)\n",
                       devid, dev_manager_msg_info->header.valid, in_len);
        return -EINVAL;
    }
    *ack_len = sizeof(*dev_manager_msg_info);
    resource_info  = (struct devdrv_manager_msg_resource_info *)dev_manager_msg_info->payload;

    if (resource_info->info_type >= DEVDRV_DEV_INFO_TYPE_MAX ||
        resource_info->info_type < DEVDRV_DEV_HBM_TOTAL) {
        devdrv_drv_err("Invalid resource type. (device_id=%u; resource_type=%d)\n",
                       devid, resource_info->info_type);
        return -EINVAL;
    }

    if (devdrv_get_resource_info[resource_info->info_type] == NULL) {
        devdrv_drv_err("Not support resource type. (device_id=%u; resource_type=%d)\n",
                       devid, resource_info->info_type);
        return -EOPNOTSUPP;
    }

    ret = devdrv_get_resource_info[resource_info->info_type](devid, resource_info);
    if (ret) {
        devdrv_drv_err("Get resource info failed. (device_id=%u; ret=%d)\n",
                       devid, ret);
        return -EINVAL;
    }

    dev_manager_msg_info->header.result = 0;
    return 0;
}

STATIC int (*devdrv_manager_chan_msg_processes[])(u32 devid, void *msg, u32 in_len, u32 *ack_len) = {
    [DEVDRV_MANAGER_CHAN_H2D_SEND_DEVID] = devdrv_manager_get_devid_from_host,
    [DEVDRV_MANAGER_CHAN_H2D_RERESH_AICORE_INFO] = devdrv_manager_refresh_aicore_info,
    [DEVDRV_MANAGER_CHAN_H2D_SYNC_GET_DEVINFO] = devdrv_manager_get_devinfo_from_host,
    [DEVDRV_MANAGER_CHAN_H2D_SYNC_GET_CORE_UTILIZATION] = devdrv_manager_get_core_utilization_from_host,
    [DEVDRV_MANAGER_CHAN_H2D_CONTAINER] = devdrv_manager_h2d_container,
    [DEVDRV_MANAGER_CHAN_H2D_GET_TASK_STATUS] = devdrv_manager_get_task_status,
    [DEVDRV_MANAGER_CHAN_H2D_SYNC_LOW_POWER] = devdrv_manager_h2d_sync_low_power,
#ifdef CFG_FEATURE_TIMESYNC
    [DEVDRV_MANAGER_CHAN_H2D_WALL_TIME_SYNC] = dms_get_walltime_from_host,
    [DEVDRV_MANAGER_CHAN_H2D_LOCALTIME_SYNC] = dms_get_timezone_from_host,
#endif
    [DEVDRV_MANAGER_CHAN_H2D_QUERY_DMP_STARTED] = devdrv_manager_get_dmp_started,
    [DEVDRV_MANAGER_CHAN_H2D_QUERY_DEVICE_PID] = devdrv_manager_get_device_pid,
    [DEVDRV_MANAGER_CHAN_H2D_NOTICE_PROCESS_EXIT] = devdrv_manager_notice_device_exit,
    [DEVDRV_MANAGER_CHAN_H2D_DMS_EVENT_SUBSCRIBE] = dms_event_subscribe_from_host,
    [DEVDRV_MANAGER_CHAN_H2D_DMS_EVENT_CLEAN] = dms_event_clean_from_host,
    [DEVDRV_MANAGER_CHAN_H2D_DMS_EVENT_MASK] = dms_event_mask_from_host,
    [DEVDRV_MANAGER_CHAN_MAX_ID] = NULL,
    [DEVDRV_MANAGER_CHAN_H2D_GET_RESOURCE_INFO] = devdrv_manager_get_resource_info,
};


int devdrv_manager_common_msg_process(u32 devid, void *data, u32 in_data_len, u32 out_data_len,
    u32 *real_out_len)
{
    u32 msg_id;
    u32 data_min_len = sizeof(struct devdrv_manager_msg_info);

    if (data == NULL || devid >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("invalid parameters data = %pK, devid = %u\n", data, devid);
        return -EINVAL;
    }

    if (in_data_len < sizeof(struct devdrv_manager_msg_head)) {
        devdrv_drv_err("Invalid msg data head len. (devid=%u; len=%u)\n", devid, in_data_len);
        return -EINVAL;
    }

    msg_id = ((struct devdrv_manager_msg_head *)data)->msg_id;

    if (msg_id >= DEVDRV_MANAGER_CHAN_MAX_ID) {
        devdrv_drv_err("invalid parameters\n");
        return -EINVAL;
    }
#ifdef CFG_FEATURE_TIMESYNC
    if ((msg_id == DEVDRV_MANAGER_CHAN_H2D_WALL_TIME_SYNC) ||
        (msg_id == DEVDRV_MANAGER_CHAN_H2D_LOCALTIME_SYNC)) {
        data_min_len = sizeof(struct dms_h2d_msg);
    }
#endif
    if (in_data_len < data_min_len) {
        devdrv_drv_err("Invalid msg data len. (len=%u)\n", in_data_len);
        return -EINVAL;
    }

    if (real_out_len == NULL) {
        devdrv_drv_err("input invalid date lenth %pK\n", real_out_len);
        return -EINVAL;
    }
    if (devdrv_manager_chan_msg_processes[msg_id] == NULL) {
        devdrv_drv_err("invalid parameters\n");
        return -EINVAL;
    }
    if (devdrv_manager_chan_msg_processes[msg_id](devid, data, in_data_len, real_out_len))
        devdrv_drv_err("get device id from host failed\n");

    return 0;
}
EXPORT_SYMBOL_UNRELEASE(devdrv_manager_common_msg_process);

STATIC void devdrv_manager_common_msg_notify(u32 dev_id)
{
    if (dev_id >= MAX_CHIP_NUM || dev_manager_info->msg_chan_rdy[dev_id] == 1) {
        devdrv_drv_err("common msg notify fail, dev_id = %u\n", dev_id);
        return;
    }
    dev_manager_info->msg_chan_rdy[dev_id] = 1;

    if (waitqueue_active(&dev_manager_info->msg_chan_wait[dev_id]))
        wake_up_interruptible(&dev_manager_info->msg_chan_wait[dev_id]);
}

STATIC int devdrv_manager_agent_init_instance(u32 dev_id, struct device *dev)
{
    if (dev_id < MAX_CHIP_NUM) {
        dev_manager_info->dma_dev[dev_id] = dev;
    }

    return 0;
}

#define DEVMNG_DEVICE_NOTIFIER "mng_device"
static int devdrv_manager_device_notifier_func(u32 udevid, enum uda_notified_action action)
{
    int ret = 0;

    if (action == UDA_INIT) {
        ret = devdrv_manager_agent_init_instance(udevid, uda_get_agent_device(udevid));
    }

    devdrv_drv_info("notifier action. (udevid=%u; action=%d; ret=%d)\n", udevid, action, ret);

    return ret;
}

STATIC int devdrv_manager_agent_init_non_trans_msg_chan(void *msg_chan)
{
    int devid = 0;
    if (msg_chan == NULL) {
        devdrv_drv_err("non trans msg chan init notify failed\n");
        return -EBUSY;
    }

    if ((devid = agentdrv_get_msg_chan_devid(msg_chan)) < 0) {
        devdrv_drv_err("msg_chan to devid failed\n");
        return -EINVAL;
    }
    if (devid >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("devid invalid(%d).\n", devid);
        return -EINVAL;
    }
    devdrv_manager_msg_chan[devid] = msg_chan;
    return 0;
}

STATIC int devdrv_manager_agent_non_trans_msg_process(void *msg_chan, void *data, u32 in_data_len, u32 out_data_len,
    u32 *real_out_len)
{
    u32 msg_id;
    int devid = 0;

    if (data == NULL || real_out_len == NULL) {
        devdrv_drv_err("invalid in len(%u)\n", in_data_len);
        return -EINVAL;
    }

    if (in_data_len < sizeof(struct devdrv_manager_msg_head)) {
        devdrv_drv_err("Invalid msg data head len. (len=%u)\n", in_data_len);
        return -EINVAL;
    }

    msg_id = ((struct devdrv_manager_msg_head *)data)->msg_id;
    if (msg_id >= DEVDRV_MANAGER_CHAN_MAX_ID)
        return -EINVAL;

    if (devdrv_manager_chan_msg_processes[msg_id] == NULL) {
        devdrv_drv_err("invalid parameters\n");

        return -EINVAL;
    }

    if ((devid = agentdrv_get_msg_chan_devid(msg_chan)) < 0) {
        devdrv_drv_err("msg_chan to devid failed\n");
        return -EINVAL;
    }
    if (devid >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("devid invalid(%d).\n", devid);
        return -EINVAL;
    }
    if (devdrv_manager_chan_msg_processes[msg_id](devid, data, in_data_len, real_out_len))
        devdrv_drv_err("get device id from host failed\n");

    return 0;
}

static struct agentdrv_common_msg_client devdrv_manager_agent_common_msg_client = {
    .type = AGENTDRV_COMMON_MSG_DEVDRV_MANAGER,
    .common_msg_recv = devdrv_manager_common_msg_process,
    .init_notify = devdrv_manager_common_msg_notify,
};

struct agentdrv_non_trans_msg_client devdrv_manager_agent_msg_client = {
    .type = agentdrv_msg_client_devmanager,
    .flag = 0,
    .init_non_trans_msg_chan = devdrv_manager_agent_init_non_trans_msg_chan,
    .non_trans_msg_process = devdrv_manager_agent_non_trans_msg_process,
};

STATIC void devdrv_manager_common_msg_chan_init(void)
{
    struct uda_dev_type type;
    int ret;
    int i;
    for (i = 0; i < DEVDRV_MAX_DAVINCI_NUM; i++) {
        devdrv_manager_msg_chan[i] = NULL;
    }

    uda_davinci_local_real_agent_type_pack(&type);
    ret = uda_notifier_register(DEVMNG_DEVICE_NOTIFIER, &type, UDA_PRI1, devdrv_manager_device_notifier_func);
    if (ret) {
        devdrv_drv_err("agentdrv register client failed\n");
        return;
    }

    ret = agentdrv_register_non_trans_msg_client(&devdrv_manager_agent_msg_client);
    if (ret) {
        devdrv_drv_err("agentdrv register client failed\n");
        goto agentdrv_register_non_trans_msg_client_failed;
    }

    ret = agentdrv_register_common_msg_client(&devdrv_manager_agent_common_msg_client);
    if (ret) {
        devdrv_drv_err("register agentdrv common msg chan failed.\n");
        goto agentdrv_register_common_msg_client_failed;
    }
    devdrv_manager_msg_chan_initialized = 1;

    return;

agentdrv_register_common_msg_client_failed:
    agentdrv_unregister_non_trans_msg_client(&devdrv_manager_agent_msg_client);
agentdrv_register_non_trans_msg_client_failed:
    (void)uda_notifier_unregister(DEVMNG_DEVICE_NOTIFIER, &type);

    devdrv_manager_agent_common_msg_client.init_notify = NULL;
    devdrv_manager_agent_common_msg_client.common_msg_recv = NULL;
    devdrv_manager_agent_common_msg_client.type = AGENTDRV_COMMON_MSG_PCIVNIC;
}

STATIC void devdrv_lpm_exception_info(void *data, unsigned long len)
{
    struct devdrv_lpm3_exception_info *exception_info = NULL;
    excep_time timestamp;
    struct timespec64 interval;

    exception_info = (struct devdrv_lpm3_exception_info *)data;
    timestamp.tv_sec = exception_info->tv_sec;
    timestamp.tv_usec = exception_info->tv_usec;

    devdrv_drv_err("receive exception info from lpm3, code: 0x%x.\n", exception_info->exception_code);

    dms_get_time_interval(&interval);
    timestamp.tv_sec -= interval.tv_sec;

    mntn_system_error(exception_info->exception_code, timestamp, 0);
}

int devdrv_lpm3_notifier_handle(u32 dev_id, unsigned long len, void *data)
{
    struct devdrv_info *info = NULL;
    struct devdrv_ipc_lpm3 *msg = NULL;
    u8 type;

    info = dev_manager_info->dev_info[dev_id];
    msg = (struct devdrv_ipc_lpm3 *)data;
    type = msg->cmd_type1;
    switch (type) {
        case LPM3_HEART_BEAT:
            break;
        case LPM3_TSENSOR_RESET_ALARM:
            devdrv_drv_warn("receive TSENSOR RESET ALARM msg from lpm3.\n");
            break;
        case LPM3_CANCEL_RESET_ALARM:
            devdrv_drv_warn("receive CANCEL TSENSOR RESET ALARM msg from lpm3.\n");
            break;
        case LPM3_EXCEPTION_INFO:
            devdrv_drv_warn("receive exception info msg from lpm3.\n");
            devdrv_lpm_exception_info(data, len);
            break;
        default:
            devdrv_drv_err("[dev_id = %d]:it's not a supportted lpm3 ipc msg command type: %d.\n", info->dev_id, type);
            break;
    }

    return 0;
}

int devdrv_lpm3_notifier_chan2(u32 dev_id, unsigned long len, void *data)
{
    struct devdrv_ipc_lpm3 *msg = NULL;
    struct devdrv_info *info = NULL;
    u8 type;

    info = dev_manager_info->dev_info[dev_id];
    if (info == NULL) {
        devdrv_drv_err("device does not exist:dev_manager_info->dev_info[%u] = NULL.\n", dev_id);
        return 0;
    }
    msg = (struct devdrv_ipc_lpm3 *)data;

    type = msg->cmd_type1;
    switch (type) {
        case LPM3_SUGGEST_LIMIT:
            devdrv_drv_warn("receive LPM3_SUGGEST_LIMIT msg from lpm3.\n");
            (void)tsmng_inform_ts_limit_aicore(dev_id);
            return 0;
        case LPM3_CANCEL_LIMIT:
            devdrv_drv_warn("receive LPM3_CANCEL_LIMIT msg from lpm3.\n");
            (void)tsmng_inform_ts_cancel_limit_aicore(dev_id);
            return 0;
        default:
            devdrv_drv_err("[dev_id = %d]:it's not a supportted lpm3 ipc msg command type: %d.\n", dev_id, type);
            return 0;
    }
}

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
        devdrv_drv_err("stream alloc_chrdev_region error, ret: %d.\n", ret);
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

    dev_info->cce_ops.cce_dev = i_device;

    ret = devdrv_manager_container_table_devlist_add_ns(&dev_info->dev_id, 1,
                                                        init_task.nsproxy->mnt_ns);
    if (ret) {
        device_destroy(devdrv_class, devno);
        cdev_del(&dev_info->cce_ops.devdrv_cdev);
        devdrv_drv_err("add to list error.%d,ret = %d\n", dev_info->dev_id, ret);
        return ret;
    }

    return 0;
}

STATIC void devdrv_manager_release_one_device(struct devdrv_info *dev_info)
{
    dev_t devno;

    (void)devdrv_manager_container_table_devlist_del_ns(&dev_info->dev_id, 1,
                                                        init_task.nsproxy->mnt_ns);
    devno = MKDEV(devdrv_major, dev_info->dev_id);

    if (dev_info->cce_ops.cce_dev != NULL) {
        device_destroy(devdrv_class, devno);
        cdev_del(&dev_info->cce_ops.devdrv_cdev);
        dev_info->cce_ops.cce_dev = NULL;
    }
}

STATIC int black_box_init(void)
{
    int ret;
    ret = devdrv_black_box_init();
    if (ret) {
        devdrv_drv_err("devdrv_black_box_init return error: %d.\n", ret);
        return -ENOMEM;
    }

    ret = appmon_black_box_init();
    if (ret) {
        devdrv_black_box_exit();
        devdrv_drv_err("appmon_black_box_init return error: %d.\n", ret);
        return -ENOMEM;
    }

    return 0;
}
STATIC void black_box_exit(void)
{
    devdrv_black_box_exit();
    appmon_black_box_exit();
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
    devdrv_manager_process_sign_release(dev_manager_context->tgid);
    devdrv_drv_debug("Dmanage end release prepare.\n");
    return 0;
}

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

    return 0;
}


STATIC int devdrv_manager_info_init(void)
{
    u32 i;

    dev_manager_info = kzalloc(sizeof(*dev_manager_info), GFP_KERNEL | __GFP_ACCOUNT);
    if (dev_manager_info == NULL) {
        devdrv_drv_err("kzalloc for dev_manager_info failed.\n");
        return -ENOMEM;
    }

    spin_lock_init(&dev_manager_info->pm_list_lock);
    INIT_LIST_HEAD(&dev_manager_info->pm_list_header);

    spin_lock_init(&dev_manager_info->proc_hash_table_lock);
    hash_init(dev_manager_info->proc_hash_table);
    mutex_init(&dev_manager_info->devdrv_sign_list_lock);

    spin_lock_init(&dev_manager_info->msg_pm_list_lock);
    INIT_LIST_HEAD(&dev_manager_info->msg_pm_list_header);

    dev_manager_info->num_dev = 0;
    for (i = 0; i < MAX_CHIP_NUM; i++) {
        dev_manager_info->msg_chan_rdy[i] = 0;
        init_waitqueue_head(&dev_manager_info->msg_chan_wait[i]);
    }

    spin_lock_init(&dev_manager_info->spinlock);

    for (i = 0; i < DEVDRV_MAX_DAVINCI_NUM; i++) {
        dev_manager_info->dev_info[i] = NULL;
        dev_manager_info->dev_id_flag[i] = 0;
        dev_manager_info->dev_id[i] = INVAILD_DEVICE_ID;
    }

    dev_manager_info->host_type = 0;
    dev_manager_info->machine_mode = DEVDRV_PCIE_EP_MODE;
    /* dev manager on device side */
    dev_manager_info->plat_info = DEVDRV_MANAGER_DEVICE_ENV;
    return 0;
}

STATIC void devdrv_manager_info_free(void)
{
    kfree(dev_manager_info);
    dev_manager_info = NULL;
}

int devdrv_manager_init(void)
{
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

    devdrv_manager_inform_wq = create_workqueue("host_inform_wq");
    if (devdrv_manager_inform_wq == NULL) {
        devdrv_drv_err("create host_inform_wq failed.\n");
        ret = -EINVAL;
        goto create_workqueue_fail;
    }

    ret = black_box_init();
    if (ret) {
        devdrv_drv_err("black_box_init failed return error: %d.\n", ret);
        goto black_box_init_fail;
    }

    ret = devdrv_manager_info_init();
    if (ret) {
        devdrv_drv_err("Init dev_manager_info failed.\n");
        goto dev_manager_info_init_failed;
    }

    init_waitqueue_head(&devdrv_manage_poll_wq);

    dev_manager_info->dev = davinci_intf_get_owner_device();
    if (dev_manager_info->dev == NULL) {
        devdrv_drv_err("failed to intf get owner device.\n");
        ret = -ENODEV;
        goto get_owner_device_fail;
    }

#ifdef CONFIG_SYSFS
    ret = sysfs_create_group(&dev_manager_info->dev->kobj, &devdrv_manager_attr_group);
    if (ret) {
        devdrv_drv_err("sysfs create failed, ret = %d\n", ret);
        goto sysfs_create_group_failed;
    }
#endif /* CONFIG_SYSFS */

    devdrv_manager_common_msg_chan_init();
    ret = agentdrv_get_rc_ep_mode(&dev_manager_info->machine_mode);
    if (ret) {
        devdrv_drv_err("agentdrv_get_rc_ep_mode failed, ret(%d).\n", ret);
        goto container_table_init_failed;
    }

#ifdef CONFIG_ACPI
    acpi_dma_configure(dev_manager_info->dev, DEV_DMA_COHERENT);
#else
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 12, 0)

    of_dma_configure(dev_manager_info->dev, NULL, true);
#else
    of_dma_configure(dev_manager_info->dev, NULL);
#endif

#endif
    dev_manager_info->dev->coherent_dma_mask = 0xffffffffffffffffUL;

    ret = devdrv_manager_container_table_init(dev_manager_info);
    if (ret) {
        devdrv_drv_err("devdrv_manager_container_table_init failed\n");
        ret = -ENOMEM;
        goto container_table_init_failed;
    }
    tsdrv_status_init();

    ret = devdrv_manager_register_notifier();
    if (ret != 0) {
        devdrv_drv_err("Failed to register notifier. (ret=%d)\n", ret);
        goto register_notifier_fail;
    }

    init_rwsem(&devdrv_ops_sem);

    return ret;
register_notifier_fail:
    devdrv_manager_container_table_exit(dev_manager_info);
container_table_init_failed:
#ifdef CONFIG_SYSFS
    sysfs_remove_group(&dev_manager_info->dev->kobj, &devdrv_manager_attr_group);
sysfs_create_group_failed:
#endif /* CONFIG_SYSFS */
get_owner_device_fail:
    devdrv_manager_info_free();
dev_manager_info_init_failed:
    black_box_exit();
black_box_init_fail:
    destroy_workqueue(devdrv_manager_inform_wq);
create_workqueue_fail:
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
    struct devdrv_msg_pm *msg_pm = NULL;
    struct devdrv_process_sign *d_sign = NULL;
    struct hlist_node *local_sign = NULL;
    u32 bkt;

#ifdef CONFIG_SYSFS
    sysfs_remove_group(&dev_manager_info->dev->kobj, &devdrv_manager_attr_group);
#endif /* CONFIG_SYSFS */

    devdrv_lc_gpioirq_unregister();

    if (!list_empty_careful(&dev_manager_info->pm_list_header)) {
        list_for_each_safe(pos, n, &dev_manager_info->pm_list_header)
        {
            pm = list_entry(pos, struct devdrv_pm, list);
            list_del(&pm->list);
            kfree(pm);
            pm = NULL;
        }
    }

    if (!list_empty_careful(&dev_manager_info->msg_pm_list_header)) {
        list_for_each_safe(pos, n, &dev_manager_info->msg_pm_list_header)
        {
            msg_pm = list_entry(pos, struct devdrv_msg_pm, list);
            list_del(&msg_pm->list);
            kfree(msg_pm);
            msg_pm = NULL;
        }
    }

    hash_for_each_safe(dev_manager_info->proc_hash_table, bkt, local_sign, d_sign, link)
    {
        hash_del(&d_sign->link);
        kfree(d_sign);
        d_sign = NULL;
    }
    destroy_workqueue(dev_manager_info->heart_beat_wq);

    flush_workqueue(devdrv_manager_inform_wq);
    destroy_workqueue(devdrv_manager_inform_wq);

    if (devdrv_manager_msg_chan_initialized == 1) {
        struct uda_dev_type type;
        uda_davinci_local_real_agent_type_pack(&type);
        (void)uda_real_virtual_notifier_unregister(DEVMNG_DEVICE_NOTIFIER, &type);
        agentdrv_unregister_common_msg_client(&devdrv_manager_agent_common_msg_client);
        devdrv_manager_msg_chan_initialized = 0;
        agentdrv_unregister_non_trans_msg_client(&devdrv_manager_agent_msg_client);
        devdrv_manager_agent_common_msg_client.common_msg_recv = NULL;
    }
    devdrv_manager_msg_notify_initialized = 0;

    devdrv_manager_container_table_exit(dev_manager_info);
    kfree(dev_manager_info);
    dev_manager_info = NULL;

    devdrv_black_box_exit();
    devdrv_manager_release_driver_device();
    if (drv_ascend_unregister_sub_module(DAVINCI_INTF_MODULE_DEVMNG)) {
        devdrv_drv_err("drv_ascend_unregister_sub_module failed!\n");
    }

    g_sysctl = NULL;
}

#else
int devdrv_manager_device_init(void)
{
    return 0;
}
#endif
