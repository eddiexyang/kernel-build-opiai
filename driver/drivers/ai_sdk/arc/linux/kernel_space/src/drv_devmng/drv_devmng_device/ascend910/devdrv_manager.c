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
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/irqchip/arm-gic-v3.h>
#include <linux/workqueue.h>
#include <linux/idr.h>
#include <linux/list.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include <linux/hugetlb.h>
#include <linux/poll.h>
#include <linux/gpio/consumer.h>
#include <linux/gpio.h>
#include <linux/securec.h>
#include <linux/securectype.h>
#include <linux/dma-mapping.h>
#include <linux/pci.h>
#include <linux/topology.h>
#include <linux/smp.h>
#include <linux/pfn.h>
#include <linux/mmzone.h>
#include <linux/kallsyms.h>
#include <linux/version.h>
#include <linux/bitmap.h>
#include <linux/of.h>

#ifndef COMPILE_WITH_UT
#include <linux/acpi.h>
#include <linux/property.h>
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
#include <linux/random.h>
#endif

#include "dbl/uda.h"

#include "devdrv_dfm.h"
#include "devdrv_manager_common.h"
#include "devmng_dms_adapt.h"
#include "devdrv_manager.h"
#include "devdrv_pm.h"
#include "devdrv_manager_msg.h"
#include "devdrv_platform_resource.h"
#include "devdrv_manager_container.h"
#include "devdrv_manager_time.h"
#include "devdrv_manager_base.h"
#include "devdrv_ipc.h"
#include "icm_interface.h"
#ifdef CFG_FEATURE_DEVICE_SHARE
#include "devdrv_manager_dev_share.h"
#endif
#include "devdrv_platform_resource.h"
#include "devdrv_manager_freq.h"
#include "devdrv_flow_control_mpam.h"
#include "config.h"
#include "devdrv_info.h"
#include "devdrv_interface.h"
#include "devdrv_pcie.h"
#include "appmon_black_box.h"
#include "devdrv_adapt_manager.h"
#include "devdrv_adapt_manager.h"
#include "devdrv_driver_pm.h"
#include "devdrv_user_common.h"
#include "devdrv_manager_comm.h"
#include "ascend_kernel_hal.h"
#include "soc_spec.h"
#include "kernel_version_adapt.h"
#include "drv_whitelist.h"
#include "dms_mem_info.h"
#include "dms_device_time_zone.h"
#include "ascend_hal_error.h"

#if (defined CFG_FEATURE_VFIO) && (defined CFG_FEATURE_RC_MODE)
#include "dev_mnt_vdevice.h"
#endif

#ifdef CFG_FEATURE_TIMESYNC
#include "dms_common.h"
#include "dms_time.h"
#endif
#if (defined CFG_FEATURE_HEART_BEAT) || (defined CFG_FEATURE_HEALTH_ERR_CODE)
#include "heart_beat.h"
#endif
#ifdef CFG_SOC_PLATFORM_MDC_V51
#include "devmng_dts.h"
#include "devdrv_os_power.h"
#endif
#include "tsdrv_status.h"
#include "drv_comm.h"
#include "devdrv_manager_pid_map.h"
#include "virtmng_interface.h"
#include "davinci_interface.h"
#include "davinci_api.h"
#ifdef CFG_FEATURE_CHIP_DIE
#include "devdrv_chip_dev_map.h"
#endif
#include "devdrv_manager_rand.h"
#include "devdrv_manager_container.h"

#ifdef CFG_SOC_PLATFORM_MINIV2
#include "user_cfg_management.h"
#include "user_cfg_interface.h"
#elif defined(CFG_FEATURE_PSS_SIGN)
#include "user_cfg_interface.h"
#endif

#include "dms_notifier.h"
#include "tsmng_interface.h"
#include "ascend_platform.h"
#include "dms_init.h"

struct devdrv_manager_info *dev_manager_info = NULL;
void *devdrv_manager_msg_chan[DEVDRV_MAX_DAVINCI_NUM];
#ifdef CFG_SURPPORT_PCIE_HOST_DEVICE_COMM
STATIC int devdrv_manager_msg_chan_initialized = 0;
#endif
STATIC int devdrv_manager_msg_notify_initialized = 0;
STATIC struct tsdrv_drv_ops devdrv_platform_drv_ops;
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
    DEVDRV_MANAGER_GET_VDEVNUM,
    DEVDRV_MANAGER_GET_VDEVIDS,
    DEVDRV_MANAGER_GET_CONTAINER_FLAG,
    DEVDRV_MANAGER_BIND_PID_ID,
    DEVDRV_MANAGER_QUERY_DEV_PID,
    DEVDRV_MANAGER_UNBIND_PID_ID,
    DEVDRV_MANAGER_QUERY_HOST_PID,
    DEVDRV_MANAGER_GET_HOST_PHY_MACH_FLAG,
};

int devdrv_lc_gpioirq_register(void);
void devdrv_lc_gpioirq_unregister(void);

#ifdef CFG_FEATURE_OLD_DEVID_TRANS
STATIC void devdrv_manager_release_one_device(struct devdrv_info *dev_info);
STATIC int devdrv_manager_create_one_device(struct devdrv_info *dev_info);
#endif
STATIC int devdrv_manager_ioctl_get_dev_resource_info(struct file *filep, unsigned int cmd, unsigned long arg);
#if (defined CFG_FEATURE_SRIOV) || (defined CFG_FEATURE_VF_USE_DEVID)
STATIC int devdrv_manager_vmngd_register(void);
STATIC int devdrv_manager_vmngd_unregister(void);
#endif
#ifdef CFG_FEATURE_REBOOT_REASON
void __iomem *g_l3sram_base_vaddr = NULL;
#endif

static u8 devdrv_lpm3_start_fail = 0;

struct workqueue_struct *devdrv_manager_inform_wq;

wait_queue_head_t devdrv_manage_poll_wq; /* wait queue */

#define DEVMNG_FW_WQ_RETRY_COUNT 100
#define DEVMNG_FW_WQ_VF_RETRY_COUNT 500
#define DEVMNG_FW_WQ_DELAY_TIME 2000
#define DEVMNG_FW_WQ_VF_DELAY_TIME 250

#define MAX_INVAILD_TLB_SIZE 0x4000000  // 64M

#define SHOW_CONFIG_BUF_SIZE  64

#define OFFSET_DOUBLE      2
#define OFFSET_TWO         2
#define OFFSET_THREE       3
#define OFFSET_FOUR        4
#define OFFSET_FIVE        5
#define OFFSET_EIGHT       8
#define OFFSET_SIXTEEN     16
#define OFFSET_EIGHTEEN    18
#define OFFSET_NINETEEN    19
#define OFFSET_TWENTY_ONE  21

#define DIVISOR_SIXTEEN    16
#define DIVISOR_THIRTY_TWO 32

#define DC_CTRL_CPU_NUM     2
#define DC_AI_CPU_NUM       6

#define CLUSTER_CPUID_BIT_NUM 8
#define SCL_CLUID_BIT_NUM     16
#define DIE_ID_BIT_NUM        19
#define SOCKET_ID_BIT_NUM     20

#ifndef OK
#define OK 0
#endif

/* Protect idr accesses */
static DEFINE_MUTEX(devdrv_mn_mutex_dev0);
static DEFINE_MUTEX(devdrv_mn_mutex_dev1);
static DEFINE_MUTEX(devdrv_mn_mutex_dev2);
static DEFINE_MUTEX(devdrv_mn_mutex_dev3);

#ifdef CONFIG_SYSFS

#define DEVDRV_ATTR_RO(_name) static struct kobj_attribute _name##_attr = __ATTR_RO(_name)

#define DEVDRV_ATTR(_name) static struct kobj_attribute _name##_attr = __ATTR(_name, 0600, _name##_show, _name##_store)

STATIC ssize_t devdrv_resources_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return 0;
}
DEVDRV_ATTR_RO(devdrv_resources);

STATIC ssize_t devdrv_interrups_show_refill_buf(struct devdrv_platform_data *pdata, char *str, int i)
{
    int avl_len = PAGE_SIZE;
    char *refill_buf = str;
    int tlen;

    tlen = snprintf_s(str, avl_len, avl_len - 1, "dev_manager_info->dev_id[%d] = %u\n", i,
                      dev_manager_info->dev_id[i]);
    if (tlen < 0) {
        devdrv_drv_info("sprintf_s is abnormal. (i=%d; dev_manager_info->dev_id[i]=%u)\n", i,
                        dev_manager_info->dev_id[i]);
        return 0;
    }
    str += tlen;
    avl_len -= tlen;
    tlen = snprintf_s(str, avl_len, avl_len - 1, "sclid = %u\n", pdata->platform_info.sclid);
    if (tlen < 0) {
        devdrv_drv_info("sprintf_s is abnormal. (sclid=%u)\n", pdata->platform_info.sclid);
        return 0;
    }
    str += tlen;
    avl_len -= tlen;
    tlen = snprintf_s(str, avl_len, avl_len - 1, "ts_cluster = %u\n", pdata->platform_info.ts_cluster);
    if (tlen < 0) {
        devdrv_drv_info("sprintf_s is abnormal. (ts_cluster=%u)\n", pdata->platform_info.ts_cluster);
        return 0;
    }
    str += tlen;
    avl_len -= tlen;
    tlen = snprintf_s(str, avl_len, avl_len - 1, "ccpu cluster = %u\n", pdata->platform_info.ccpu_cluster);
    if (tlen < 0) {
        devdrv_drv_info("sprintf_s is abnormal. (ccpu cluster=%u)\n", pdata->platform_info.ccpu_cluster);
        return 0;
    }
    str += tlen;
    avl_len -= tlen;
    tlen = snprintf_s(str, avl_len, avl_len - 1, "aicpu_cluster = %u\n", pdata->platform_info.aicpu_cluster);
    if (tlen < 0) {
        devdrv_drv_info("sprintf_s is abnormal. (aicpu_cluster=%u)\n", pdata->platform_info.aicpu_cluster);
        return 0;
    }
    str += tlen;

    return str - refill_buf;
}

STATIC ssize_t devdrv_interrups_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct devdrv_platform_data *pdata = NULL;
    struct devdrv_info *dev_info = NULL;
    char *str = buf;
    int i;
    int ret;

    for (i = 0; i < DEVDRV_MAX_DAVINCI_NUM; i++) {
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
            devdrv_drv_info("dev_info sprintf_s abnormal. (chip_num=%d)\n", i);
            return 0;
        }
        str += ret;
    }
    return str - buf;
}
DEVDRV_ATTR_RO(devdrv_interrups);

STATIC ssize_t devdrv_check_modules_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return 0;
}

static ssize_t devdrv_check_modules_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf,
                                          size_t count)
{
    char *check_str = "check_module_success";
    int ret;

    if (buf == NULL) {
        devdrv_drv_err("invalid args(buf is null).\n");
        return count;
    }

    ret = memcmp(check_str, buf, strlen(check_str));
    if (ret == 0) {
        set_last_ko_insert_flag();
    }
    return count;
}
DEVDRV_ATTR(devdrv_check_modules);

#if ((defined CFG_SOC_PLATFORM_MINIV2) && (!defined CFG_SOC_PLATFORM_MDC_V51))
#define GET_SSH_CFG_WAIT_TIMEOUT 3000
#define GET_SSH_CFG_TASK_SLEEP 10
struct ssh_config_task {
    unsigned char ssh_status[UC_SSH_STATUS_SIZE];
    unsigned int ssh_size;
    struct semaphore ssh_cfg_sema;
};

STATIC int devdrv_get_ssh_config_task(void *data)
{
    struct ssh_config_task *ssh_task = (struct ssh_config_task *)data;
    int index, ret;

    if (ssh_task == NULL) {
        devdrv_drv_err("Invalid parameter.\n");
        ret = -EINVAL;
        goto invalid_para;
    }

    index = devdrv_get_config_index_by_name(SSH_CONFIG_NAME);
    if (index < 0) {
        devdrv_drv_err("Can't support ssh this boardId. (index=%d)\n", index);
        ret = -EINVAL;
        goto up_out;
    }

    ret = devdrv_get_user_config_core(DEVDRV_UC_DEV_ID_0, index,
                                      ssh_task->ssh_status, &ssh_task->ssh_size);
    if (ret != 0) {
        devdrv_drv_err("Get ssh config failed. (ret=%d)\n", ret);
        goto up_out;
    }

up_out:
    up(&ssh_task->ssh_cfg_sema);
invalid_para:
    while (!kthread_should_stop()) {
        msleep(GET_SSH_CFG_TASK_SLEEP);
    }
    return ret;
}

STATIC ssize_t devdrv_ssh_config_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct ssh_config_task ssh_task = {0};
    struct task_struct *task = NULL;
    int tlen = 0;
    int ret;

    ssh_task.ssh_size = UC_SSH_STATUS_SIZE;
    sema_init(&ssh_task.ssh_cfg_sema, 0);
    task = kthread_create(devdrv_get_ssh_config_task, (void *)&ssh_task, "get_ssh_config_task");
    if (IS_ERR(task) || (task == NULL)) {
        devdrv_drv_err("kthread_create failed. (errno=%ld).\n", PTR_ERR(task));
    } else {
        (void)wake_up_process(task);
        ret = down_timeout(&ssh_task.ssh_cfg_sema, msecs_to_jiffies(GET_SSH_CFG_WAIT_TIMEOUT));
        if (ret != 0) {
            devdrv_drv_err("Wait respone timeout. (ret=%d; timeout=%d).\n",
                           ret, GET_SSH_CFG_WAIT_TIMEOUT);
        }
        kthread_stop(task);
    }

    if (*ssh_task.ssh_status == SSH_DISENABLE) {
        tlen = snprintf_s(buf, SHOW_CONFIG_BUF_SIZE, SHOW_CONFIG_BUF_SIZE - 1, "ssh-off\n");
    } else if (*ssh_task.ssh_status == SSH_ENABLE) {
        tlen = snprintf_s(buf, SHOW_CONFIG_BUF_SIZE, SHOW_CONFIG_BUF_SIZE - 1, "ssh-on\n");
    } else {
        tlen = snprintf_s(buf, SHOW_CONFIG_BUF_SIZE, SHOW_CONFIG_BUF_SIZE - 1, "ssh-off\n");
        devdrv_drv_warn("Ssh config is invalid in flash. (ssh_status=%u)\n", *ssh_task.ssh_status);
    }
    if (tlen <= 0) {
        devdrv_drv_err("Ssh config show snprintf failed. (ret=%d; ssh_status=%u)\n",
                       tlen, *ssh_task.ssh_status);
        return 0;
    }

    devdrv_drv_info("Get ssh config end. (ssh_status=%u)\n", *ssh_task.ssh_status);
    return tlen;
}
DEVDRV_ATTR_RO(devdrv_ssh_config);
#endif

#if ((defined CFG_SOC_PLATFORM_MINI) && (!defined CFG_SOC_PLATFORM_MINIV2))
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
    ret = devdrv_get_user_config_core(DEVDRV_UC_DEV_ID_0, index, aicpu_config, &aicpu_size);
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
    ret = devdrv_get_user_config_core(DEVDRV_UC_DEV_ID_0, index, aicpu_config, &aicpu_size);
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
#endif

static struct attribute *devdrv_manager_attrs[] = {
    &devdrv_resources_attr.attr,
    &devdrv_interrups_attr.attr,
    &devdrv_check_modules_attr.attr,
#if ((defined CFG_SOC_PLATFORM_MINIV2) && (!defined CFG_SOC_PLATFORM_MDC_V51))
    &devdrv_ssh_config_attr.attr,
#endif
#if ((defined CFG_SOC_PLATFORM_MINI) && (!defined CFG_SOC_PLATFORM_MINIV2))
    &devdrv_aicpu_config_attr.attr,
    &devdrv_ctrlcpu_config_attr.attr,
#endif
    NULL,
};

static struct attribute_group devdrv_manager_attr_group = {
    .attrs = devdrv_manager_attrs,
    .name = "devdrv_manager",
};

#endif /* CONFIG_SYSFS */

int hvdevmng_get_aicore_num(u32 devid, u32 fid, u32 *aicore_num)
{
    struct devdrv_info *dev_info = NULL;

    if ((aicore_num == NULL) || (fid != 0)) {
        devdrv_drv_err("invalid parameter, dev_id = %u, fid = %u.\n", devid, fid);
        return -EINVAL;
    }

    dev_info = devdrv_manager_get_devdrv_info(devid);
    if (dev_info == NULL) {
        devdrv_drv_err("device manager is not initialized, dev_id = %u\n", devid);
        return -ENODEV;
    }

    *aicore_num = dev_info->ai_core_num;
    return 0;
}
EXPORT_SYMBOL(hvdevmng_get_aicore_num);

void hvdevmng_set_dev_ts_resource(u32 devid, u32 fid, u32 tsid, void *data)
{
    return;
}EXPORT_SYMBOL(hvdevmng_set_dev_ts_resource);

#ifdef CFG_SURPPORT_PCIE_HOST_DEVICE_COMM
int devdrv_agent_sync_msg_send(u32 dev_id, struct devdrv_manager_msg_info *msg_info, u32 payload_len, u32 *out_len)
{
    u32 in_len;

    if ((dev_id >= DEVDRV_MAX_DAVINCI_NUM) || (msg_info == NULL) || (out_len == NULL) ||
        (payload_len > sizeof(msg_info->payload))) {
        devdrv_drv_err("invaild dev_id(%u) or msg_info(%pK) is null or out_len(%pK) is null.\n",
            dev_id, msg_info, out_len);
        return -EINVAL;
    }

    in_len = sizeof(struct devdrv_manager_msg_head) + payload_len;

    return agentdrv_sync_msg_send(devdrv_manager_msg_chan[dev_id], msg_info, in_len, in_len, out_len);
}
EXPORT_SYMBOL(devdrv_agent_sync_msg_send);
#endif

struct tsdrv_drv_ops *devdrv_manager_get_drv_ops(void)
{
    return &devdrv_platform_drv_ops;
}
EXPORT_SYMBOL(devdrv_manager_get_drv_ops);

void devdrv_mn_mutex_lock(int dev_id)
{
    if ((dev_id < 0) || (dev_id >= DEVICE_ID_MAX)) {
        devdrv_drv_err("[dev_id = %d]:devdrv_mn_mutex_lock id error.\n", dev_id);
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

void devdrv_mn_mutex_unlock(int dev_id)
{
    if ((dev_id < 0) || (dev_id >= DEVICE_ID_MAX)) {
        devdrv_drv_err("[dev_id = %d]:devdrv_mn_mutex_unlock id error.\n", dev_id);
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

int copy_from_user_safe(void *to, const void __user *from, unsigned long n)
{
    if ((from == NULL) || (to == NULL)) {
        devdrv_drv_err("user pointer is NULL.\n");
        return -EINVAL;
    }

    if (n == 0) {
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
    if ((from == NULL) || (to == NULL)) {
        devdrv_drv_err("user pointer is NULL.\n");
        return -EINVAL;
    }

    if (n == 0) {
        devdrv_drv_err("n is zero.\n");
        return -EINVAL;
    }

    if (copy_to_user(to, (void *)from, n))
        return -ENODEV;

    return 0;
}
EXPORT_SYMBOL(copy_to_user_safe);

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

    return dev_info->pdata->ts_num;
}
EXPORT_SYMBOL(devdrv_manager_get_ts_num);

#ifdef CFG_SURPPORT_PCIE_HOST_DEVICE_COMM
STATIC int devdrv_check_process_sign(int chip_id, pid_t hostpid, const char *sign, u32 len)
{
    struct devdrv_manager_msg_info dev_manager_msg_info = {{0}, {0}};
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

#ifdef CFG_FEATURE_SRIOV
    dev_id = chip_id;
#else
    dev_id = cpu_to_node((int)smp_processor_id()); /*lint !e666  !e453*/
#endif
    ret = agentdrv_common_msg_send(dev_id, &dev_manager_msg_info, sizeof(dev_manager_msg_info),
                                   sizeof(dev_manager_msg_info), &out_len, AGENTDRV_COMMON_MSG_DEVDRV_MANAGER);
    if (ret) {
        devdrv_drv_err("common msg send failed, ret(%d)\n", ret);
        return -ENODEV;
    }
    if (dev_manager_msg_info.header.result != 0) {
        devdrv_drv_err("check process_sign failed, result(%u)\n",
                       dev_manager_msg_info.header.result);
        return -ENODEV;
    }

    return 0;
}
#endif

int devdrv_get_pcie_id_info(u32 devid, struct dmanage_pcie_id_info *pcie_id_info)
{
#ifdef CFG_SURPPORT_PCIE_HOST_DEVICE_COMM
    struct devdrv_manager_msg_info dev_manager_msg_info = {{0}, {0}};
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
    if (ret || (dev_manager_msg_info.header.result != 0)) {
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
#else
    devdrv_drv_err("not support in current enveroment\n");
    return -ENOTSUPP;
#endif
}

int devdrv_manager_get_device_index(u32 host_dev_id, u32 *local_dev_id)
{
#ifdef CFG_SURPPORT_PCIE_HOST_DEVICE_COMM
    struct devdrv_manager_msg_info dev_manager_msg_info = {{0}, {0}};
    u32 *host_devid_buf = NULL;
    u32 *local_devid_buf = NULL;
    u32 out_len;
    u32 dev_id;
    int ret;

    if (local_dev_id == NULL) {
        devdrv_drv_err("invalid input para, local_dev_id is NULL\n");
        return -ENODEV;
    }

    dev_manager_msg_info.header.msg_id = DEVDRV_MANAGER_CHAN_D2H_GET_DEVICE_INDEX;
    dev_manager_msg_info.header.valid = (u16)DEVDRV_MANAGER_MSG_VALID;
    dev_manager_msg_info.header.result = (u16)DEVDRV_MANAGER_MSG_INVALID_RESULT;

    host_devid_buf = (u32 *)dev_manager_msg_info.payload;
    *host_devid_buf = host_dev_id;

    dev_id = cpu_to_node((int)smp_processor_id()); /*lint !e666  !e453*/

    ret = agentdrv_common_msg_send(dev_id, &dev_manager_msg_info, sizeof(dev_manager_msg_info),
                                   sizeof(dev_manager_msg_info), &out_len, AGENTDRV_COMMON_MSG_DEVDRV_MANAGER);
    if (ret) {
        devdrv_drv_err("common msg send failed, ret(%d)\n", ret);
        return -ENODEV;
    }
    if (dev_manager_msg_info.header.result != 0) {
        devdrv_drv_err("get device index failed, result(%u)\n",
                       dev_manager_msg_info.header.result);
        return -ENODEV;
    }

    local_devid_buf = (u32 *)dev_manager_msg_info.payload;
    *local_dev_id = *local_devid_buf;
    return 0;
#else
    devdrv_drv_err("not support in current enveroment\n");
    return -ENOTSUPP;
#endif
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

#ifdef CFG_FEATURE_OLD_DEVID_TRANS
int devdrv_get_devnum(u32 *num_dev)
{
    u32 dev_num;

    if (num_dev == NULL) {
        return -EINVAL;
    }

    dev_num = devdrv_manager_get_devnum();
    if ((dev_num == 0) || (dev_num > DEVDRV_MAX_DAVINCI_NUM)) {
        devdrv_drv_err("invalid device number, dev_num = %u\n", dev_num);
        return -EINVAL;
    }
    *num_dev = dev_num;

    return 0;
}
EXPORT_SYMBOL(devdrv_get_devnum);

int devdrv_get_vdevnum(u32 *num)
{
    u32 vdev_num;

    if (num == NULL) {
        return -EINVAL;
    }

    vdev_num = devdrv_manager_get_vdevnum();
    if (vdev_num > MAX_VDEV_NUM) {
        devdrv_drv_err("Invalid device number. (vdev_num=%u)\n", vdev_num);
        return -EINVAL;
    }
    *num = vdev_num;

    return 0;
}
EXPORT_SYMBOL(devdrv_get_vdevnum);
#endif

u32 devdrv_manager_get_devid(u32 local_devid)
{
    unsigned int retry_cnt = 0;
    u32 node_num = 0;
    u32 cnt_max;
    u32 dev_id;
    int ret;

    if ((dev_manager_info == NULL) || (local_devid >= DEVDRV_MAX_DAVINCI_NUM)) {
        devdrv_drv_err("invalid param, local_devid = %u, dev_manager_info = %pK.\n", local_devid, dev_manager_info);
        return DEVDRV_MAX_DAVINCI_NUM;
    }

    /* RC mode returns its own dev id. */
    if (dev_manager_info->machine_mode == DEVDRV_PCIE_RC_MODE) {
        ret = devdrv_get_devnum(&node_num);
        if (ret || (local_devid >= node_num)) {
            devdrv_drv_err("Local devid out of range. (ret=%d; local_id=%u)\n", ret, local_devid);
            return DEVDRV_MAX_DAVINCI_NUM;
        }
        return local_devid;
    }

    /*
     * wait the devid that host send to device
     * if the waiting time exceeds (DEVDRV_WAIT_TIME_HOST_DEVID)ms, report error and return.
     */
    cnt_max = DEVDRV_WAIT_TIME_DEVID / DEVDRV_WAIT_TIME_DEVID_ONCE;
    while (!dev_manager_info->dev_id_flag[local_devid]) {
        retry_cnt++;
        if (retry_cnt > cnt_max) {
            devdrv_drv_err("get devId from host failed, because the dev_id_flag[%u]=%u, retry_cnt=%u.\n",
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

void devdrv_get_chip_version(struct devdrv_info *dev_info, u32 *chip_id, u32 *die_id)
{
#ifdef CFG_SOC_PLATFORM_CLOUD_V2
    void *info_reg = NULL;
    u32 reg_value;
    u32 reg_offset = 0x1C;
    u32 die_bit = 8;

    info_reg = (u32 *)((unsigned long)(uintptr_t)dev_info->pdata->ts_pdata[0].ts_sysctl_vaddr + reg_offset);
    reg_value = readl_relaxed(info_reg);

    *chip_id = reg_value & 0xF;
    *die_id = (reg_value >> die_bit) & 0x3;
#else
    *chip_id = 0;
    *die_id = 0;
#endif
}

STATIC int devdrv_manager_get_ts_ddr_nid(struct devdrv_info *dev_info, u32 slot_num, u32 chip_type)
{
    int nid = dev_info->dev_id;

#ifndef CFG_SOC_PLATFORM_HELPER
    if (dev_info->ts_mem_restrict_valid == TS_MEM_RESTRICT_VALID) {
        nid = DEVDRV_TS_NODE_DDR_ID_OFFSET * slot_num + dev_info->dev_id;
    } else {
        if (chip_type == HISI_MINI_V2) {
            nid = DEVDRV_TS_NODE_DDR_ID_OFFSET * slot_num + dev_info->dev_id;
            return nid;
        }

        if (chip_type == HISI_CLOUD_V2) {
            if (devdrv_manager_is_pf_device(dev_info->dev_id)) {
                u32 num_online = num_online_nodes();
                if (num_online == slot_num) { /* asic 16G */
                    nid = dev_info->dev_id;
                } else if (num_online == 2 * slot_num) { /* fpga */
                    nid = dev_info->dev_id + slot_num;
                } else {      /* asic 32G/64G */
                    nid = dev_info->dev_id * 32 + 32;
                }

                devdrv_drv_info("Online_node. (num_online=%u)\n", num_online);
#ifdef CFG_FEATURE_SRIOV
            } else {
                int ret;
                struct vmng_soc_resource_enquire info = {0};

                ret = vmngd_enquire_soc_resource(dev_info->dev_id, 0, &info);
                if (ret != 0) {
                    devdrv_drv_err("Get resource from vmng fail. (dev_id=%u; ret=%d)\n", dev_info->dev_id, ret);
                    return NUMA_NO_NODE;
                }

                nid = __fls(info.each.memory.numa_id.bitmap);
#endif
            }

            devdrv_drv_info("(devid=%u; slot_num=%u; nid=%d)\n", dev_info->dev_id, slot_num, nid);
        }
    }
#else
    nid = NUMA_NO_NODE;
    if (dev_info->dev_id == 0) {
        nid = 2;
    }

    if (dev_info->dev_id == 1) {
        nid = 3;
    }
#endif

    return nid;
}

/* only support HISI_CLOUD_V1 */
STATIC int devdrv_manager_get_hbm_nid(struct devdrv_info *dev_info, u32 slot_num, u32 chip_type)
{
    int nid = NUMA_NO_NODE;

    if (chip_type != HISI_CLOUD_V1) {
        return nid;
    }
    nid = slot_num + dev_info->dev_id * DEVDRV_HBM_ID_OFFSET;

    return nid;
}

/* only support HISI_CLOUD_V1 */
STATIC int devdrv_manager_get_p2p_hbm_nid(struct devdrv_info *dev_info, u32 slot_num, u32 chip_type)
{
    int nid = NUMA_NO_NODE;

    if (chip_type != HISI_CLOUD_V1) {
        return nid;
    }
    nid = slot_num + dev_info->dev_id * DEVDRV_HBM_ID_OFFSET + 1;

    return nid;
}

STATIC int devdrv_manager_get_ddr_nid(struct devdrv_info *dev_info)
{
    return dev_info->dev_id;
}

/* ensure devid is valid and the device is initialized */
int devdrv_manager_devid_to_nid(u32 devid, u32 mem_type)
{
    struct devdrv_info *dev_info = NULL;
    u32 slot_num, chip_type;
    int nid = NUMA_NO_NODE;

    if (devid >= DEVDRV_MAX_DAVINCI_NUM || mem_type >= DEVDRV_MEM_TYPE_MAX) {
        devdrv_drv_err("invalid param, devid(%u) mem_type(%u), return the default numa_id: %d.\n",
            devid, mem_type, nid);
        return nid;
    }

    dev_info = devdrv_manager_get_devdrv_info(devid);
    if (dev_info == NULL) {
        devdrv_drv_err("device(%u) is not initialized, return the default numa_id: %d.\n", devid, nid);
        return nid;
    }

#ifdef CFG_SURPPORT_PCIE_HOST_DEVICE_COMM
    slot_num = devdrv_get_slot_num();
    chip_type = devdrv_get_dev_chip_type(dev_info->dev_id);
#else
    slot_num = 0;
    chip_type = uda_get_chip_type(dev_info->dev_id);
#endif

    switch (mem_type) {
        case DEVDRV_TS_NODE_DDR_MEM:
            nid = devdrv_manager_get_ts_ddr_nid(dev_info, slot_num, chip_type);
            break;
        case DEVDRV_HBM_MEM:
            nid = devdrv_manager_get_hbm_nid(dev_info, slot_num, chip_type);
            break;
        case DEVDRV_P2P_HBM_MEM:
            nid = devdrv_manager_get_p2p_hbm_nid(dev_info, slot_num, chip_type);
            break;
        case DEVDRV_DDR_MEM:
            nid = devdrv_manager_get_ddr_nid(dev_info);
            break;
        default :
            devdrv_drv_err("devid: %u not support mem_type %u.\n", devid, mem_type);
            break;
    }

    return nid;
}
EXPORT_SYMBOL(devdrv_manager_devid_to_nid);

#ifdef CFG_FEATURE_OLD_DEVID_TRANS
u32 devdrv_manager_get_devnum(void)
{
    u32 num_dev;

    if (dev_manager_info == NULL)
        return (DEVDRV_MAX_DAVINCI_NUM + 1);

    if (devdrv_manager_container_task_struct_check(current)) {
        devdrv_drv_err("invalid current pointer.\n");
        return (DEVDRV_MAX_DAVINCI_NUM + 1);
    }

    num_dev = devdrv_manager_container_get_devnum(current->nsproxy->mnt_ns, DEVDRV_PF_TYPE);

    return num_dev;
}

u32 devdrv_manager_get_vdevnum(void)
{
    u32 num_vdev;

    if (dev_manager_info == NULL)
        return (DEVDRV_MAX_DAVINCI_NUM + 1);

    if (devdrv_manager_container_task_struct_check(current)) {
        devdrv_drv_err("invalid current pointer.\n");
        return (DEVDRV_MAX_DAVINCI_NUM + 1);
    }

    num_vdev = devdrv_manager_container_get_devnum(current->nsproxy->mnt_ns, DEVDRV_VF_TYPE);

    return num_vdev;
}
#endif

int devdrv_fop_bind_host_pid(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_ioctl_para_bind_host_pid para_info = {0};
    int node_id = numa_node_id();
    int ret;
    u32 phys_id = DEVDRV_MAX_DAVINCI_NUM + 1, vfid = 0;

    if (copy_from_user_safe(&para_info, (void *)(uintptr_t)arg, sizeof(struct devdrv_ioctl_para_bind_host_pid))) {
        devdrv_drv_err("copy_from_user error. dev_id:%d\n", node_id);
        return -EINVAL;
    }

    para_info.sign[DEVDRV_SIGN_LEN - 1] = '\0';
    ret = devdrv_manager_trans_and_check_id(para_info.chip_id, &phys_id, &vfid, ALSO_DOES_SUPPORT_VF);
    if (ret != 0) {
        devdrv_drv_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n", para_info.chip_id, ret);
        return ret;
    }

    para_info.chip_id = phys_id;

#ifndef DEVMNG_UT
    if ((para_info.len != PROCESS_SIGN_LENGTH) ||
        (para_info.mode >= AICPUFW_MAX_PLAT) || (para_info.cp_type < 0) ||
        (para_info.cp_type >= DEVDRV_PROCESS_CPTYPE_MAX) ||
        (para_info.chip_id >= DEVDRV_MAX_NODE_NUM) ||(para_info.vfid >= VFID_NUM_MAX)) {
        devdrv_drv_err("invalid para length(%u) cp_type(%d) chip_id(%u) hostpid(%d) vfid(%u).\n",
            para_info.len, para_info.cp_type, para_info.chip_id, para_info.host_pid, para_info.vfid);
        return -EINVAL;
    }

#ifdef CFG_SURPPORT_PCIE_HOST_DEVICE_COMM
    if ((para_info.mode == AICPUFW_ONLINE_PLAT) && (para_info.vfid == 0)) {
        ret = devdrv_check_process_sign(para_info.chip_id, para_info.host_pid, para_info.sign, para_info.len);
        if (ret) {
            devdrv_drv_err("d2h check sign failed, ret(%d).\n", ret);
            return -EINVAL;
        }
    }
#endif
#endif

    ret = devdrv_bind_hostpid(para_info);
    if (ret) {
        devdrv_drv_err("bind_hostpid error. dev_id:%d, ret:%d, host_pid:%d, cp_type:%d, devpid:%d\n",
                        node_id, ret, para_info.host_pid, para_info.cp_type, current->tgid);
        return ret;
    }

    return 0;
}

#ifdef CFG_FEATURE_HOST_UNBIND
STATIC int devdrv_fop_unbind_host_pid(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    struct devdrv_ioctl_para_bind_host_pid para_info = {0};

    ret = copy_from_user_safe(&para_info, (void *)(uintptr_t)arg, sizeof(struct devdrv_ioctl_para_bind_host_pid));
    if (ret != 0) {
        devdrv_drv_err("copy_from_user error. ret:%d\n", ret);
        return -EINVAL;
    }

    para_info.sign[DEVDRV_SIGN_LEN - 1] = '\0';

    if ((para_info.len != PROCESS_SIGN_LENGTH) ||
        (para_info.mode >= AICPUFW_MAX_PLAT) || (para_info.cp_type < 0) ||
        (para_info.cp_type >= DEVDRV_PROCESS_CPTYPE_MAX) ||
        (para_info.chip_id >= DEVDRV_MAX_NODE_NUM) ||(para_info.vfid >= VFID_NUM_MAX)) {
        devdrv_drv_err("invalid para length(%u) cp_type(%d) chip_id(%u) hostpid(%d) vfid(%u).\n",
            para_info.len, para_info.cp_type, para_info.chip_id, para_info.host_pid, para_info.vfid);
        return -EINVAL;
    }

    ret = devdrv_unbind_hostpid(para_info);
    if (ret != 0) {
        devdrv_drv_err("devdrv_unbind_hostpid failed, ret = %d.\n", ret);
        return ret;
    }

    return 0;
}
#endif

int devdrv_query_devpid(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    int node_id = numa_node_id();
    struct devdrv_ioctl_para_query_pid para_info = {0};
    u32 phys_id = DEVDRV_MAX_DAVINCI_NUM + 1, vfid = 0;

    if (copy_from_user_safe(&para_info, (void *)(uintptr_t)arg, sizeof(struct devdrv_ioctl_para_query_pid))) {
        devdrv_drv_err("copy_from_user error. dev_id:%d\n", node_id);
        return -EINVAL;
    }

    ret = devdrv_manager_trans_and_check_id(para_info.chip_id, &phys_id, &vfid, ALSO_DOES_SUPPORT_VF);
    if (ret != 0) {
        devdrv_drv_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n", para_info.chip_id, ret);
        return ret;
    }

    ret = devdrv_query_process_by_host_pid_user(para_info.host_pid, phys_id,
                                           para_info.cp_type, para_info.vfid, &(para_info.pid));
    if (ret) {
        devdrv_drv_warn("query device pid failed, ret(%d).\n", ret);
        return ret;
    }

    if (copy_to_user_safe((void *)((uintptr_t)arg), &para_info, sizeof(struct devdrv_ioctl_para_query_pid))) {
        devdrv_drv_err("copy_to_user error. dev_id:%d\n", node_id);
        return -EINVAL;
    }

    return 0;
}

/* stub for device container.c */
int devmng_get_vdavinci_info(u32 vdev_id, u32 *phy_id, u32 *vfid)
{
    if ((phy_id == NULL) || (vfid == NULL))
        return -EINVAL;

    *phy_id = vdev_id;
    *vfid = 0;
    return 0;
}
EXPORT_SYMBOL(devmng_get_vdavinci_info);

#ifndef CFG_FEATURE_VFIO_SOC
/* dev_mnt_vdevice_add_inform and dev_mnt_vdevice_inform are stub-function for ep-device. */
int dev_mnt_vdevice_add_inform(unsigned int vdev_id, vdev_action action, struct mnt_namespace *ns, u64 container_id)
{
    devdrv_drv_info("stub\n");
    return 0;
}

void dev_mnt_vdevice_inform(void)
{
}
#endif

#ifdef CFG_FEATURE_OLD_DEVID_TRANS
int devdrv_get_devids(u32 *devices, u32 device_num)
{
    u32 num_dev;
    u8 i;
    u8 j = 0;

    if (devices == NULL) {
        devdrv_drv_err("Input devices is null.\n");
        return -EINVAL;
    }

    num_dev = devdrv_manager_get_devnum();
    if ((num_dev == 0) || (num_dev > DEVDRV_MAX_DAVINCI_NUM) || (num_dev > device_num)) {
        devdrv_drv_err("Invalid device number. (num_dev=%u; dev_num_len=%u)\n", num_dev, device_num);
        return -EINVAL;
    }

    /* get device id assigned from host, default dev_id is 0 if there is no host */
    for (i = 0; i < DEVDRV_MAX_DAVINCI_NUM; i++) {
        if (dev_manager_info->dev_id[i] == INVAILD_DEVICE_ID) {
            continue;
        }
        if (j >= device_num) {
            break;
        }
        if (devdrv_manager_is_pf_device(i)) {
            devices[j++] = dev_manager_info->dev_id[i];
        }
    }

    if (j == 0) {
        devdrv_drv_err("NO dev_info!!!\n");
        return -EFAULT;
    }

    return 0;
}
EXPORT_SYMBOL(devdrv_get_devids);

int devdrv_get_vdevids(u32 *devices, u32 device_num)
{
    u32 num_vdev;
    u8 i;
    u8 j = 0;

    if (devices == NULL) {
        devdrv_drv_err("Input devices is null.\n");
        return -EINVAL;
    }

    num_vdev = devdrv_manager_get_vdevnum();
    if ((num_vdev == 0) || (num_vdev > DEVDRV_MAX_DAVINCI_NUM) || (num_vdev > device_num)) {
        devdrv_drv_err("Invalid device number. (num_vdev=%u; dev_num_len=%u)\n", num_vdev, device_num);
        return -EINVAL;
    }

    /* get device id assigned from host, default dev_id is 0 if there is no host */
    for (i = 0; i < DEVDRV_MAX_DAVINCI_NUM; i++) {
        if (dev_manager_info->dev_id[i] == INVAILD_DEVICE_ID) {
            continue;
        }
        if (j >= device_num) {
            break;
        }
        if (!devdrv_manager_is_pf_device(i)) {
            devices[j++] = dev_manager_info->dev_id[i];
        }
    }

    if (j == 0) {
        devdrv_drv_err("NO dev_info!!!\n");
        return -EFAULT;
    }

    return 0;
}
EXPORT_SYMBOL(devdrv_get_vdevids);
#endif

int devdrv_get_devinfo(u32 devid, struct devdrv_device_info *info)
{
    struct devdrv_platform_data *pdata = NULL;
    struct devdrv_info *dev_info = NULL;

    if (info == NULL) {
        devdrv_drv_err("invalid parameter, dev_id = %u.\n", devid);
        return -EINVAL;
    }

    dev_info = devdrv_manager_get_devdrv_info(devid);
    if (dev_info == NULL) {
        devdrv_drv_err("device manager is not initialized, dev_id = %u\n", devid);
        return -EINVAL;
    }
    pdata = dev_info->pdata;
    info->ai_core_num = dev_info->ai_core_num;
    info->ai_cpu_core_num = dev_info->ai_cpu_core_num;
    info->ctrl_cpu_core_num = dev_info->ctrl_cpu_core_num;
    info->ctrl_cpu_occupy_bitmap = dev_info->ctrl_cpu_occupy_bitmap;

    info->ai_cpu_core_id = dev_info->ai_cpu_core_id;
    info->ai_core_id = dev_info->ai_core_id;

    /* 1:little endian 0:big endian */
    info->ctrl_cpu_endian_little = dev_info->ctrl_cpu_endian_little;
    info->ctrl_cpu_id = dev_info->ctrl_cpu_id;
    info->ctrl_cpu_ip = dev_info->ctrl_cpu_ip;
    info->ts_cpu_core_num = pdata->ts_pdata[0].ts_cpu_core_num;
    info->aicpu_occupy_bitmap = dev_info->aicpu_occupy_bitmap;
    info->env_type = dev_info->env_type;

    return 0;
}
EXPORT_SYMBOL(devdrv_get_devinfo);

int devdrv_get_core_inuse(u32 devid, u32 vfid, struct devdrv_hardware_inuse *inuse)
{
    struct devdrv_info *dev_info = NULL;
    enum devdrv_ts_status ts_status;

    if ((inuse == NULL) || (devid >= DEVDRV_MAX_DAVINCI_NUM)) {
        devdrv_drv_err("invalid parameter, dev_id = %u\n", devid);
        return -EINVAL;
    }

    if ((dev_manager_info == NULL) || (dev_manager_info->dev_info[devid] == NULL)) {
        devdrv_drv_err("device manager is not initialized dev_id = %u.\n", devid);
        return -EINVAL;
    }

    dev_info = dev_manager_info->dev_info[devid];
    if (devdrv_manager_ts_is_enable() == false) {
        devdrv_drv_err("ts driver is not enable.\n");
        return -EOPNOTSUPP;
    }
#if (defined CFG_FEATURE_VFIO) && (defined CFG_FEATURE_RC_MODE)
    if (VDAVINCI_IS_VDEV(devid)) {
        /* If it is a vf in rc mode, the pf ts-status will be used. */
        ts_status = tsdrv_get_ts_status(VDAVINCI_GET_PFID(devid), 0);
    } else {
        ts_status = tsdrv_get_ts_status(devid, 0);
    }
#else
    ts_status = tsdrv_get_ts_status(devid, 0);
#endif
    if (ts_status != TS_WORK) {
        devdrv_drv_err("[dev_id = %u]:device is not working.\n", devid);
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
        devdrv_drv_err("[dev_id = %u]:devid is invalid.\n", devid);
        return -EINVAL;
    }

    if ((spec == NULL) || (dev_manager_info == NULL) || (dev_manager_info->dev_info[devid] == NULL)) {
        devdrv_drv_err("[dev_id = %u]:device manager is not initialized.\n", devid);
        return -EINVAL;
    }

    dev_info = dev_manager_info->dev_info[devid];

    spec->ai_core_num = dev_info->ai_core_num;
    spec->first_ai_core_id = dev_info->ai_core_id;
    spec->ai_cpu_num = dev_info->ai_cpu_core_num;
    spec->first_ai_cpu_id = dev_info->ai_cpu_core_id;
    spec->ai_core_num_level = dev_info->pdata->ai_core_num_level;
    spec->ai_core_freq_level = dev_info->pdata->ai_core_freq_level;

    return 0;
}
EXPORT_SYMBOL(devdrv_get_core_spec);

#ifdef CFG_FEATURE_LP_ENABLE
STATIC int check_to_lp_cmd_type1(unsigned char cmd_type1)
{
    unsigned int cmd_type = cmd_type1;
    if (!(((cmd_type >= IPC_CMD_DVPP_MIN) && (cmd_type <= IPC_CMD_DVPP_MAX)) ||
       ((cmd_type >= IPC_CMD_RETR_MIN) && (cmd_type <= IPC_CMD_RETR_MAX)))) {
        devdrv_drv_err("Invalid argument, cmd_type1 = %u.\n", cmd_type1);
        return -EINVAL;
    }
    return OK;
}
#endif

int hal_kernel_send_ipc_to_lp_async(unsigned int devid, unsigned char cmd_type0,
    unsigned char cmd_type1, unsigned char *data, unsigned int data_len)
{
#ifdef CFG_FEATURE_LP_ENABLE
    struct devdrv_ipc_imu ipc_msg = {0};
    int ret;

    if ((cmd_type0 != LPM3_IDLE_CMD) || (data_len > TO_LP_MAX_DATA_LEN)) {
        devdrv_drv_err("Invalid argument, cmd_type0 = %u, data_len = %u.\n", cmd_type0, data_len);
        return -EINVAL;
    }

    ret = check_to_lp_cmd_type1(cmd_type1);
    if (ret != OK) {
        devdrv_drv_err("Invalid argument, cmd_type1 = %u.\n", cmd_type1);
        return -EINVAL;
    }

    ipc_msg.cmd_type0 = cmd_type0;
    ipc_msg.cmd_type1 = cmd_type1;
    ipc_msg.target_id = LPR52_TARGET_ID;
    ipc_msg.source_id = LPR52_SOURECE_ID;

    ret = memcpy_s(&ipc_msg.cmd_para0, TO_LP_MAX_DATA_LEN, data, data_len);
    if (ret != OK) {
        devdrv_drv_err("data memcpy_s error, ret = %d.\n", ret);
        return ret;
    }

    ret = icm_msg_send_async(ICM_FD_BUILD(devid, HISI_RPROC_LP_Q_TX_RPID4_ACPU2),
        (rproc_msg_t *)&ipc_msg, sizeof(ipc_msg) / sizeof(rproc_msg_len_t));
    if (ret != OK) {
        devdrv_drv_err("ipc message send failed, ret = %d.\n", ret);
        return ret;
    }
#endif
    return OK;
}
EXPORT_SYMBOL(hal_kernel_send_ipc_to_lp_async);

struct devdrv_info *devdrv_manager_get_devdrv_info(u32 dev_id)
{
    struct devdrv_info *dev_info = NULL;
    unsigned long flags;

    if ((dev_manager_info == NULL) || (dev_id >= DEVDRV_MAX_DAVINCI_NUM))
        return NULL;

    spin_lock_irqsave(&dev_manager_info->spinlock, flags);
    dev_info = dev_manager_info->dev_info[dev_id];
    spin_unlock_irqrestore(&dev_manager_info->spinlock, flags);

    return dev_info;
}
EXPORT_SYMBOL(devdrv_manager_get_devdrv_info);

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

int devdrv_get_chip_die_id(u32 dev_id, u32 *chip_id, u32 *die_id)
{
    struct devdrv_info *dev_info = NULL;

    if (chip_id == NULL || die_id == NULL) {
        devdrv_drv_err("Invaild paramemter. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    dev_info = devdrv_manager_get_devdrv_info(dev_id);
    if (dev_info == NULL) {
        devdrv_drv_err("dev_info is NULL. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    *chip_id = dev_info->chip_id;
    *die_id = dev_info->die_id;

    return 0;
}
EXPORT_SYMBOL(devdrv_get_chip_die_id);

int devdrv_get_device_ids(u32 dev_id, u32 *board_id, u32 *slot_id)
{
    if (dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("invalid device id\n");
        return -EINVAL;
    }

    if (dev_manager_info == NULL || dev_manager_info->dev_info[dev_id] == NULL) {
        return -EINVAL;
    }

    if (board_id == NULL || slot_id == NULL) {
        devdrv_drv_err("parameters invalid.\n");
        return -EINVAL;
    }

    *board_id = dev_manager_info->dev_info[dev_id]->board_id;
    *slot_id = dev_manager_info->dev_info[dev_id]->slot_id;
    return 0;
}
EXPORT_SYMBOL(devdrv_get_device_ids);

#ifdef CFG_SOC_PLATFORM_CLOUD
#define TSDRV_CHIP_NUM_MAX 4
int devdrv_get_tsdrv_cq_aisle_irq(int node_id)
{
    if (node_id >= TSDRV_CHIP_NUM_MAX) {
        devdrv_drv_err("tsdrv aisle cq node_id out range. node_id:%d, max:%d\n", node_id, TSDRV_CHIP_NUM_MAX);
        return -EINVAL;
    }
    return dev_manager_info->dev_info[node_id]->pdata->ts_pdata[0].irq_mailbox_data_ack_request;
}
EXPORT_SYMBOL(devdrv_get_tsdrv_cq_aisle_irq);
#endif

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
#ifdef CFG_FEATURE_OLD_DEVID_TRANS
    u32 docker_id;
#endif
#ifdef CFG_FEATURE_IPC_NOTIFY
    int ret;
#endif

    if (filep == NULL) {
        devdrv_drv_err("filep is NULL.\n");
        return -EINVAL;
    }

    if (dev_manager_info == NULL) {
        devdrv_drv_err("dev_manager_info initialization not completed.\n");
        return -EINVAL;
    }

#ifdef CFG_FEATURE_OLD_DEVID_TRANS
    if (devdrv_manager_container_task_struct_check(current)) {
        devdrv_drv_err("current is invalid.\n");
        return -EINVAL;
    }
#endif

    dev_manager_context = kzalloc(sizeof(struct devdrv_manager_context), GFP_KERNEL | __GFP_ACCOUNT);
    if (dev_manager_context == NULL)
        return -ENOMEM;

    dev_manager_context->pid = current->pid;
    dev_manager_context->tgid = current->tgid;
    dev_manager_context->start_time = current->start_time;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
    dev_manager_context->real_start_time = current->start_boottime;
#else
    dev_manager_context->real_start_time = current->real_start_time;
#endif
    dev_manager_context->mnt_ns = current->nsproxy->mnt_ns;
#ifndef DEVMNG_UT
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0)
    dev_manager_context->pid_ns = current->nsproxy->pid_ns_for_children;
#else
    dev_manager_context->pid_ns = current->nsproxy->pid_ns;
#endif
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

#ifdef CFG_FEATURE_OLD_DEVID_TRANS
    ret = devdrv_manager_container_table_overlap(dev_manager_context, &docker_id);
    if (ret) {
        kfree(dev_manager_context);
        dev_manager_context = NULL;
        devdrv_drv_err("update_item failed, ret(%d).\n", ret);
        return -EINVAL;
    }
#endif

    filep->private_data = dev_manager_context;

    return 0;
}

#ifdef CFG_FEATURE_IPC_NOTIFY
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
        if (devdrv_platform_drv_ops.ipc_notify_release_recycle != NULL) {
            devdrv_platform_drv_ops.ipc_notify_release_recycle(dev_manager_context);
        }
        devdrv_manager_ops_sem_up_read();
    }

    devdrv_manager_ipc_notify_uninit(dev_manager_context);
}
#endif

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
    devdrv_manager_resource_recycle(dev_manager_context);
#endif
    kfree(dev_manager_context);
    dev_manager_context = NULL;

    return 0;
}

#ifdef CFG_SOC_PLATFORM_MDC_V51
#ifndef DEVMNG_UT
STATIC int devdrv_manager_get_ts_node_num(void)
{
    int ts_num = 0;
    struct device_node *node = NULL;
    struct device_node *son = NULL;

    node = of_find_compatible_node(NULL, NULL, "hisi,mini-devdrv-device");
    if (node == NULL) {
        devdrv_drv_info("fail to find mini-devdrv-device\n");
        return 0;
    }

    son = of_find_node_by_name(node, "ts-0");
    if (son != NULL) {
        devdrv_drv_info("sucess to find ts-0\n");
        ts_num++;
    }

    son = of_find_node_by_name(node, "ts-1");
    if (son != NULL) {
        devdrv_drv_info("sucess to find ts-1\n");
        ts_num++;
    }
    devdrv_drv_info("find ts (%d)\n", ts_num);
    return ts_num;
}
#endif
#endif

#define TS_NUM_INVALID -1
bool devdrv_manager_ts_is_enable(void)
{
#ifdef CFG_SOC_PLATFORM_MDC_V51
    STATIC int ts_node_num = TS_NUM_INVALID;

    if (ts_node_num == TS_NUM_INVALID) {
        ts_node_num = devdrv_manager_get_ts_node_num();
    }

    if (ts_node_num == 0) {
        return false;
    }
    return true;
#else
    return true;
#endif
}

STATIC int devdrv_manager_get_device_status(struct file *filep, unsigned int cmd, unsigned long arg)
{
    enum devdrv_ts_status status;
    int para = 0;
    int ret;
    u32 phys_id = DEVDRV_MAX_DAVINCI_NUM + 1, vfid = 0;

    ret = copy_from_user_safe(&para, (void *)((uintptr_t)arg), sizeof(int));
    if (ret) {
        devdrv_drv_err("copy from user failed.,ret = %d\n", ret);
        return ret;
    }

    ret = devdrv_manager_trans_and_check_id(para, &phys_id, &vfid, TRANS_PHYID_TO_PFID);
    if (ret != 0) {
        devdrv_drv_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n", para, ret);
        return ret;
    }

    para = phys_id;
#ifdef CFG_SOC_PLATFORM_MDC_V51
#ifndef DEVMNG_UT
    if (devdrv_manager_ts_is_enable()) {
        status = tsdrv_get_ts_status(para, 0);
    } else {
        status = TS_WORK;
    }
#endif
#else
    status = tsdrv_get_ts_status(para, 0);
#endif

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
        devdrv_drv_err("devid=%d copy from user failed.,ret = %d\n", para, ret);
        return ret;
    }

    return 0;
}

STATIC int devdrv_manager_get_core(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_hardware_inuse inuse = {0};
    struct devdrv_hardware_spec spec = {0};
    int ret;
    u32 phys_id = DEVDRV_MAX_DAVINCI_NUM + 1, vfid = 0;

    switch (cmd) {
        case DEVDRV_MANAGER_GET_CORE_SPEC:
            ret = copy_from_user_safe(&spec, (void *)((uintptr_t)arg), sizeof(struct devdrv_hardware_spec));
            if (ret) {
                devdrv_drv_err("copy_from_user_safe failed.,ret = %d\n", ret);
                return ret;
            }
            ret = devdrv_manager_trans_and_check_id(spec.devid, &phys_id, &vfid, ALSO_DOES_SUPPORT_VF);
            if (ret != 0) {
                devdrv_drv_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n",
                    spec.devid, ret);
                return ret;
            }
            ret = devdrv_get_core_spec(phys_id, 0, &spec);
            if (ret) {
                devdrv_drv_err("devdrv_get_core_spec failed.,ret = %d\n", ret);
                return ret;
            }
            ret = copy_to_user_safe((void *)((uintptr_t)arg), &spec, sizeof(struct devdrv_hardware_spec));
            if (ret) {
                devdrv_drv_err("copy_to_user_safe failed.,ret = %d\n", ret);
                return ret;
            }
            break;
        case DEVDRV_MANAGER_GET_CORE_INUSE:
            ret = copy_from_user_safe(&inuse, (void *)((uintptr_t)arg), sizeof(struct devdrv_hardware_inuse));
            if (ret) {
                devdrv_drv_err("copy_from_user_safe failed.,ret = %d\n", ret);
                return ret;
            }
            ret = devdrv_manager_trans_and_check_id(inuse.devid, &phys_id, &vfid, ALSO_DOES_SUPPORT_VF);
            if (ret != 0) {
                devdrv_drv_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n",
                    inuse.devid, ret);
                return ret;
            }
            ret = devdrv_get_core_inuse(phys_id, 0, &inuse);
            if (ret) {
                devdrv_drv_err("devdrv_get_core_inuse failed.,ret = %d\n", ret);
                return ret;
            }
            ret = copy_to_user_safe((void *)((uintptr_t)arg), &inuse, sizeof(struct devdrv_hardware_inuse));
            if (ret) {
                devdrv_drv_err("copy_to_user_safe failed.,ret = %d\n", ret);
                return ret;
            }
            break;
        default:
            devdrv_drv_err("invalid cmd.\n");
            return -EINVAL;
    }

    return 0;
}

#ifdef CFG_FEATURE_OLD_DEVID_TRANS
STATIC int devdrv_manager_get_devids(unsigned long arg)
{
    struct devdrv_manager_hccl_devinfo hccl_devinfo = {0};
    int ret;

    hccl_devinfo.num_dev = devdrv_manager_get_devnum();
    if ((hccl_devinfo.num_dev == 0) || (hccl_devinfo.num_dev > DEVDRV_MAX_DAVINCI_NUM)) {
        devdrv_drv_err("wrong device number, num_dev = %u\n", hccl_devinfo.num_dev);
        return -EINVAL;
    }

    ret = devdrv_manager_container_get_devids(hccl_devinfo.devids, DEVDRV_MAX_DAVINCI_NUM, &hccl_devinfo.num_dev,
        current->nsproxy->mnt_ns, DEVDRV_PF_TYPE);
    if (ret != 0) {
        devdrv_drv_err("Failed to get device list. (ret=%d)\n", ret);
        return -ENODEV;
    }

    if (copy_to_user_safe((void *)(uintptr_t)arg, &hccl_devinfo, sizeof(hccl_devinfo))) {
        devdrv_drv_err("copy from user failed.\n");
        return -EINVAL;
    }

    return 0;
}

STATIC int devdrv_manager_get_vdevids(unsigned long arg)
{
    struct devdrv_manager_hccl_devinfo hccl_devinfo = {0};
    int ret;

    hccl_devinfo.num_dev = devdrv_manager_get_vdevnum();
    if ((hccl_devinfo.num_dev == 0) || (hccl_devinfo.num_dev > DEVDRV_MAX_DAVINCI_NUM)) {
        devdrv_drv_err("Invalid device number. (num_dev=%u)\n", hccl_devinfo.num_dev);
        return -EINVAL;
    }

    ret = devdrv_get_vdevids(hccl_devinfo.devids, DEVDRV_MAX_DAVINCI_NUM);
    if (ret) {
        devdrv_drv_err("devdrv_get_devids failed. (ret=%d)\n", ret);
        return ret;
    }
    if (copy_to_user_safe((void *)(uintptr_t)arg, &hccl_devinfo, sizeof(hccl_devinfo))) {
        devdrv_drv_err("Copy from user failed.\n");
        return -EINVAL;
    }

    return 0;
}
#endif

int devdrv_manager_trans_and_check_id(u32 logical_dev_id, u32 *physical_dev_id, u32 *vfid, pfvf_deal_flag deal)
{
    int ret = 0;
#ifdef CFG_FEATURE_DEVICE_DEVID_CONVERT
    ret = devdrv_manager_container_logical_id_to_physical_id(logical_dev_id, physical_dev_id, vfid);
    if (ret != 0) {
        devdrv_drv_err("Failed to get physical_dev_id and vfid by logical_dev_id. (logical_dev_id=%u)\n", logical_dev_id);
        return ret;
    }

    switch (deal) {
        case ALSO_DOES_SUPPORT_VF:
            /* Do nothing. */
            break;
        case ONLY_DOES_SUPPORT_PF:
            if (VDAVINCI_IS_VDEV(*physical_dev_id)) {
                devdrv_drv_err("Operation not permitted. (dev_id=%u; phys_id=%u)\n", logical_dev_id, *physical_dev_id);
                return -EPERM;
            }
            break;
        case TRANS_PHYID_TO_PFID:
            if (VDAVINCI_IS_VDEV(*physical_dev_id)) {
                *physical_dev_id = VDAVINCI_GET_PFID(*physical_dev_id);
            }
            break;
        default:
            devdrv_drv_err("The deal flag does not exist. (deal=%u).\n", deal);
            ret = -EINVAL;
            break;
    }
#else
    if (logical_dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("Logical device id is out of range. (logical_dev_id=%u)\n", logical_dev_id);
        return -EINVAL;
    }

    *physical_dev_id = logical_dev_id;
    *vfid = 0;
#endif

    return ret;
}

#ifdef CFG_FEATURE_DEVICE_CONTAINER
#define CPU_INFO_SIZE 256
#define AICPU_BITMAP_LEN 32
STATIC int get_available_cpumask(cpumask_var_t *cpumask)
{
    struct file *file = NULL;
    char *buf = NULL;
    loff_t pos = 0;
    int len, ret;

    /* read cpuset info from file */
    file = filp_open("/sys/fs/cgroup/cpuset/cpuset.cpus", O_RDONLY, 0);
    if (IS_ERR(file)) {
        return -ENOENT;
    }
    /* kzalloc */
    buf = kzalloc(CPU_INFO_SIZE, GFP_KERNEL | __GFP_ACCOUNT);
    if (buf == NULL) {
        filp_close(file, NULL);
        file = NULL;
        return -ENOMEM;
    }
    /* read cpumask config */
    len = kernel_read(file, buf, CPU_INFO_SIZE - 1, &pos);
    filp_close(file, NULL);
    file = NULL;
    /* if len small or equal 0, return */
    if (len <= 0) {
        kfree(buf);
        buf = NULL;
        return -ENOENT;
    }
    /* alloc cpumask var */
    if (!zalloc_cpumask_var(cpumask, GFP_KERNEL)) {
        kfree(buf);
        buf = NULL;
        return -ENOMEM;
    }
    /* parse the cpumask */
    ret = cpulist_parse(buf, *cpumask);
    if (ret != 0) {
        free_cpumask_var(*cpumask);
    }
    kfree(buf);
    buf = NULL;
    return 0;
}
#endif

STATIC void devdrv_get_available_ctrlcpu(u32 phy_bitmap, u32 *number, u32 *bitmap)
{
#ifdef CFG_FEATURE_DEVICE_CONTAINER
    int ret, i;
    u32 container_bitmap = 0;
    unsigned long bitmap_tmp = 0;
    cpumask_var_t available_cpumask;

    if (!devdrv_manager_container_is_in_container()) {
        return;
    }

    /* Obtain the available ctrlcpu in the container scenario. */
    ret = get_available_cpumask(&available_cpumask);
    if (ret != 0) {
        devdrv_drv_err("Failed to get cpu mask.");
        return;
    }

    for_each_cpu(i, available_cpumask) {
        container_bitmap |= (1U << i);
    }

    (*bitmap) = phy_bitmap & container_bitmap;
    bitmap_tmp = (unsigned long)(*bitmap);
    *number = bitmap_weight(&bitmap_tmp, AICPU_BITMAP_LEN);
#endif
    return;
}

STATIC int devdrv_manager_get_devinfo(unsigned long arg)
{
    int ret;
    struct devdrv_manager_hccl_devinfo hccl_devinfo = {0};
    struct devdrv_platform_data *pdata = NULL;
    struct devdrv_info *dev_info = NULL;
    u32 phys_id = DEVDRV_MAX_DAVINCI_NUM + 1, vfid = 0;

    ret = copy_from_user_safe(&hccl_devinfo, (void *)(uintptr_t)arg, sizeof(hccl_devinfo));
    if (ret) {
        devdrv_drv_err("copy from user failed, ret = %d\n", ret);
        return ret;
    }

    ret = devdrv_manager_trans_and_check_id(hccl_devinfo.dev_id, &phys_id, &vfid, ALSO_DOES_SUPPORT_VF);
    if (ret != 0) {
        devdrv_drv_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n",
            hccl_devinfo.dev_id, ret);
        return ret;
    }

    dev_info = devdrv_manager_get_devdrv_info(phys_id);
    if (dev_info == NULL) {
        devdrv_drv_err("get default dev_info failed, dev_id = %u\n", hccl_devinfo.dev_id);
        return -ENODEV;
    }
    pdata = dev_info->pdata;

    hccl_devinfo.ai_core_num = dev_info->ai_core_num;
    hccl_devinfo.aicore_freq = dev_info->aicore_freq;

    devdrv_drv_debug("aicore frequence freq=%llu.\n", hccl_devinfo.aicore_freq);
    hccl_devinfo.ai_cpu_core_num = dev_info->ai_cpu_core_num;
    hccl_devinfo.ctrl_cpu_core_num = dev_info->ctrl_cpu_core_num;
    hccl_devinfo.ctrl_cpu_occupy_bitmap = dev_info->ctrl_cpu_occupy_bitmap;
    devdrv_get_available_ctrlcpu(hccl_devinfo.ctrl_cpu_occupy_bitmap,
        &hccl_devinfo.ctrl_cpu_core_num, &hccl_devinfo.ctrl_cpu_occupy_bitmap);

    /* 1:little endian 0:big endian */
    hccl_devinfo.ctrl_cpu_endian_little = dev_info->ctrl_cpu_endian_little;
    hccl_devinfo.ctrl_cpu_id = dev_info->ctrl_cpu_id;
    hccl_devinfo.ctrl_cpu_ip = dev_info->ctrl_cpu_ip;
    hccl_devinfo.ts_cpu_core_num = pdata->ts_pdata[0].ts_cpu_core_num;
    hccl_devinfo.env_type = dev_info->env_type;
    hccl_devinfo.ai_core_id = dev_info->ai_core_id;
    hccl_devinfo.ai_cpu_core_id = dev_info->ai_cpu_core_id;
    hccl_devinfo.ai_cpu_bitmap = dev_info->aicpu_occupy_bitmap;
    hccl_devinfo.ai_cpu_core_num = dev_info->ai_cpu_core_num;
    hccl_devinfo.hardware_version = dev_info->hardware_version;
    hccl_devinfo.ts_num = devdrv_manager_get_ts_num(dev_info);
    hccl_devinfo.cpu_system_count = devdrv_manager_get_cpu_tick();
    hccl_devinfo.monotonic_raw_time_ns = ktime_get_raw_ns();
    ret = devdrv_get_ffts_type(&hccl_devinfo.ffts_type);
    if (ret != 0) {
        devdrv_drv_err("Get ffts type failed. (dev_id=%u; ret=%d)\n", hccl_devinfo.dev_id, ret);
        return ret;
    }

    hccl_devinfo.vector_core_num = dev_info->vector_core_num;
    hccl_devinfo.vector_core_freq = dev_info->vector_core_freq;
    hccl_devinfo.chip_id = dev_info->chip_id;
    hccl_devinfo.die_id = dev_info->die_id;
    ret = copy_to_user_safe((void *)(uintptr_t)arg, &hccl_devinfo, sizeof(hccl_devinfo));
    if (ret) {
        devdrv_drv_err("[dev_id = %u]:copy to user error.,ret = %d\n", hccl_devinfo.dev_id, ret);
        return ret;
    }

    return 0;
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

#ifdef CFG_FEATURE_OLD_DEVID_TRANS
STATIC int devdrv_manager_get_devid_by_localdevid(unsigned long arg)
{
    int ret;
    u32 local_devid = 0;
    u32 dev_id;

#ifdef CFG_SOC_PLATFORM_MDC_V51
    return -EOPNOTSUPP;
#endif
    ret = copy_from_user_safe(&local_devid, (void *)(uintptr_t)arg, sizeof(u32));
    if (ret) {
        devdrv_drv_err("copy from user failed, ret = %d\n", ret);
        return ret;
    }

    dev_id = devdrv_manager_get_devid(local_devid);
    if (dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_warn("dev_id invalid(%u)\n", dev_id);
        return -EAGAIN;
    }
    if (copy_to_user_safe((void *)(uintptr_t)arg, &dev_id, sizeof(u32))) {
        devdrv_drv_err("[dev_id=%u]:copy to user error.\n", local_devid);
        return -EFAULT;
    }

    return 0;
}
#endif

STATIC int devdrv_manager_get_dev_info_by_phyid(unsigned long arg)
{
    int ret;
    struct devdrv_phy_get_devinfo_para para = {0};

    ret = copy_from_user_safe(&para, (void *)(uintptr_t)arg, sizeof(struct devdrv_phy_get_devinfo_para));
    if (ret) {
        devdrv_drv_err("copy from user failed, ret = %d\n", ret);
        return ret;
    }

#ifdef CFG_SURPPORT_PCIE_HOST_DEVICE_COMM
    para.chip_type = devdrv_get_dev_chip_type(para.phy_id);
#else
    para.chip_type = uda_get_chip_type(para.phy_id);
#endif
    if (para.chip_type == HISI_CHIP_UNKNOWN) {
        devdrv_drv_err("devmng get dev chip_type failed, unknown \n");
        return -EINVAL;
    }
    if (para.chip_type >= HISI_CHIP_NUM) {
        devdrv_drv_err("chip_type invalid(%u)\n", para.chip_type);
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
#ifdef CFG_FEATURE_OLD_DEVID_TRANS
        case DEVDRV_MANAGER_GET_DEVIDS:
            ret = devdrv_manager_get_devids(arg);
            break;
#endif
        case DEVDRV_MANAGER_GET_DEVINFO:
            ret = devdrv_manager_get_devinfo(arg);
            break;
#ifdef CFG_FEATURE_OLD_DEVID_TRANS
        case DEVDRV_MANAGER_GET_DEVID_BY_LOCALDEVID:
            ret = devdrv_manager_get_devid_by_localdevid(arg);
            break;
#endif
        case DEVDRV_MANAGER_GET_H2D_DEVINFO:
            ret = devdrv_manager_get_devinfo(arg);
            break;
        case DEVDRV_MANAGER_GET_DEV_INFO_BY_PHYID:
            ret = devdrv_manager_get_dev_info_by_phyid(arg);
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

#ifdef CFG_FEATURE_OLD_DEVID_TRANS
STATIC int devdrv_manager_ioctl_get_devnum(struct file *filep, unsigned int cmd, unsigned long arg)
{
    u32 devnum;

    devnum = devdrv_manager_get_devnum();
    if (copy_to_user_safe((void *)(uintptr_t)arg, &devnum, sizeof(u32)))
        return -EFAULT;
    else
        return 0;
}

STATIC int devdrv_manager_ioctl_get_vdevnum(struct file *filep, unsigned int cmd, unsigned long arg)
{
    u32 vdevnum;

    vdevnum = devdrv_manager_get_vdevnum();
    if (copy_to_user_safe((void *)(uintptr_t)arg, &vdevnum, sizeof(u32))) {
        devdrv_drv_err("Get vdev num copy_to_user_safe failed.\n");
        return -EFAULT;
    }

    return 0;
}
#endif

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
    struct devdrv_module_status status = {0};
    struct devdrv_platform_data *pdata = NULL;
    struct devdrv_info *dev_info = NULL;
    int ret;
    u32 phys_id = DEVDRV_MAX_DAVINCI_NUM + 1, vfid = 0;

    ret = copy_from_user_safe(&status, (void *)(uintptr_t)arg, sizeof(struct devdrv_module_status));
    if (ret) {
        devdrv_drv_err("copy from user failed.\n");
        return -EINVAL;
    }

    ret = devdrv_manager_trans_and_check_id(status.devid, &phys_id, &vfid, ALSO_DOES_SUPPORT_VF);
    if (ret != 0) {
        devdrv_drv_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n",
            status.devid, ret);
        return ret;
    }

    dev_info = dev_manager_info->dev_info[phys_id];
    if (dev_info == NULL) {
        devdrv_drv_err("no device.\n");
        return -ENODEV;
    }
    pdata = dev_info->pdata;
    status.ai_core_error_bitmap = dev_info->inuse.ai_core_error_bitmap;
    status.lpm3_start_fail = devdrv_lpm3_start_fail;
    status.lpm3_lost_heart_beat = 0;
    status.ts_start_fail = pdata->ts_pdata[0].ts_start_fail;
    status.ts_lost_heart_beat = (tsdrv_is_ts_work(dev_info->dev_id, 0) == false) ? 1 : 0;
    status.ts_sram_broken = 0x01 & (dev_info->ai_subsys_ip_broken_map >> DEVDRV_AI_SUBSYS_INIT_CHECK_SRAM_OFFSET);
    status.ts_sdma_broken = 0x01 & (dev_info->ai_subsys_ip_broken_map >> DEVDRV_AI_SUBSYS_INIT_CHECK_SDMA_OFFSET);
    status.ts_bs_broken = 0x01 & (dev_info->ai_subsys_ip_broken_map >> DEVDRV_AI_SUBSYS_INIT_CHECK_BS_OFFSET);
    status.ts_l2_buf0_broken = 0x01 & (dev_info->ai_subsys_ip_broken_map >> DEVDRV_AI_SUBSYS_INIT_CHECK_L2_BUF0_OFFSET);
    status.ts_l2_buf1_broken = 0x01 & (dev_info->ai_subsys_ip_broken_map >> DEVDRV_AI_SUBSYS_INIT_CHECK_L2_BUF1_OFFSET);
    status.ts_sdma_broken = 0x01 & (dev_info->ai_subsys_ip_broken_map >> DEVDRV_AI_SUBSYS_SDMA_WORKING_STATUS_OFFSET);
    status.ts_spcie_broken = 0x01 & (dev_info->ai_subsys_ip_broken_map >> DEVDRV_AI_SUBSYS_SPCIE_WORKING_STATUS_OFFSET);
    status.ts_ai_core_broken = 0x01 & (dev_info->ai_subsys_ip_broken_map >> DEVDRV_AI_SUBSYS_INIT_CHECK_AI_CORE_OFFSET);
    status.ts_hwts_broken = 0x01 & (dev_info->ai_subsys_ip_broken_map >> DEVDRV_AI_SUBSYS_INIT_CHECK_HWTS_OFFSET);
    status.ts_doorbell_broken = 0x01 &
                                (dev_info->ai_subsys_ip_broken_map >> DEVDRV_AI_SUBSYS_INIT_CHECK_DOORBELL_OFFSET);
    ret = copy_to_user_safe((void *)((uintptr_t)arg), &status, sizeof(struct devdrv_module_status));
    if (ret) {
        devdrv_drv_err("[dev_id=%u]:copy_to_user_safe failed.\n", status.devid);
        return ret;
    }

    return 0;
}

STATIC int devdrv_manager_get_board_id(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_info *dev_info = NULL;
    u32 board_id;
    u32 dev_id;
    u32 id = 0;
    int ret;
    u32 phys_id = DEVDRV_MAX_DAVINCI_NUM + 1, vfid = 0;

    ret = copy_from_user_safe(&id, (void *)(uintptr_t)arg, sizeof(u32));
    if (ret) {
        devdrv_drv_err("copy from user failed.\n");
        return -EINVAL;
    }

    ret = devdrv_manager_trans_and_check_id(id, &phys_id, &vfid, TRANS_PHYID_TO_PFID);
    if (ret != 0) {
        devdrv_drv_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n",
            id, ret);
        return ret;
    }

    dev_id = phys_id;
    if (dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("devid(%u) err.\n", dev_id);
        return -EINVAL;
    }

    if ((dev_manager_info == NULL) || (dev_manager_info->dev_info[dev_id] == NULL)) {
        devdrv_drv_err("[dev_id = %u]:device does not exist.\n", dev_id);
        return -EINVAL;
    }

    dev_info = dev_manager_info->dev_info[dev_id];
    board_id = dev_info->board_id;

    ret = copy_to_user_safe((void *)((uintptr_t)arg), &board_id, sizeof(u32));
    if (ret) {
        devdrv_drv_err("[dev_id = %u]:copy_to_user_safe failed.\n", dev_id);
        return -EINVAL;
    }

    return 0;
}

STATIC int devdrv_manager_get_slot_id(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_info *dev_info = NULL;
    struct ioctl_arg user_arg = {0};
    u32 dev_id;
    int ret;
    u32 phys_id = DEVDRV_MAX_DAVINCI_NUM + 1, vfid = 0;

    ret = copy_from_user_safe(&user_arg, (void *)(uintptr_t)arg, sizeof(struct ioctl_arg));
    if (ret) {
        devdrv_drv_err("copy from user failed.\n");
        return -EINVAL;
    }

    ret = devdrv_manager_trans_and_check_id(user_arg.dev_id, &phys_id, &vfid, TRANS_PHYID_TO_PFID);
    if (ret != 0) {
        devdrv_drv_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n",
            user_arg.dev_id, ret);
        return ret;
    }

    dev_id = phys_id;
    if (dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("[dev_id = %u]:devid(%u) err.\n", dev_id, dev_id);
        return -EINVAL;
    }

    if ((dev_manager_info == NULL) || (dev_manager_info->dev_info[dev_id] == NULL)) {
        devdrv_drv_err("[dev_id = %u]:device does not exist.\n", dev_id);
        return -EINVAL;
    }

    dev_info = dev_manager_info->dev_info[dev_id];
    user_arg.data1 = dev_info->slot_id;

    ret = copy_to_user_safe((void *)((uintptr_t)arg), &user_arg, sizeof(struct ioctl_arg));
    if (ret) {
        devdrv_drv_err("[dev_id = %u]:copy_to_user_safe failed.\n", dev_id);
        return -EINVAL;
    }
    return 0;
}

#ifdef CFG_SOC_PLATFORM_CLOUD
STATIC int devdrv_manager_get_eth_id(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_info *dev_info = NULL;
    struct ioctl_arg user_arg = {0};
    u32 local_devid;
    u32 dev_id;
    u32 port_id;
    u32 eth_id;
    u32 board_id;
    int ret;

    ret = copy_from_user_safe(&user_arg, (void *)(uintptr_t)arg, sizeof(struct ioctl_arg));
    if (ret) {
        devdrv_drv_err("copy from user failed.\n");
        return -EINVAL;
    }
    local_devid = user_arg.dev_id;
    if (local_devid >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("devid(%u) err.\n", local_devid);
        return -EINVAL;
    }

    if ((dev_manager_info == NULL) || (dev_manager_info->dev_info[local_devid] == NULL)) {
        devdrv_drv_err("[dev_id = %u]:device does not exist.\n", local_devid);
        return -EINVAL;
    }

    dev_info = dev_manager_info->dev_info[local_devid];
    board_id = dev_info->board_id;

    port_id = user_arg.data1;
    if (soc_misc_is_pcie_card(board_id)) {
        eth_id = port_id;
    } else {
        dev_id = devdrv_manager_get_devid(local_devid);
        if (dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
            devdrv_drv_warn("dev_id invalid(%u)\n", dev_id);
            return -EAGAIN;
        }
        eth_id = dev_id;
    }

    user_arg.data1 = eth_id;
    ret = copy_to_user_safe((void *)((uintptr_t)arg), &user_arg, sizeof(struct ioctl_arg));
    if (ret) {
        devdrv_drv_err("[dev_id = %u]:copy_to_user_safe failed.\n", local_devid);
        return -EINVAL;
    }
    return 0;
}
#endif

STATIC int devdrv_manager_register_vmng_client(struct file *filep, unsigned int cmd, unsigned long arg)
{
#if (defined CFG_FEATURE_VFIO && !defined CFG_FEATURE_SRIOV)
    int ret;

    ret = vmngd_register_vmng_client();
    if (ret != 0) {
        devdrv_drv_err("vmngd register vmng client failed.\n");
        return -EINVAL;
    }
#endif
    devdrv_drv_info("vmngd register vmng client.\n");
    return 0;
}

STATIC int devdrv_manager_get_device_vf_max(struct file *filep, unsigned int cmd, unsigned long arg)
{
    u32 vf_max_num = 0;
    u32 devid = 0;
    int ret;
    u32 phys_id = DEVDRV_MAX_DAVINCI_NUM + 1, vfid = 0;

    ret = copy_from_user_safe(&devid, (void *)(uintptr_t)arg, sizeof(u32));
    if (ret) {
        devdrv_drv_err("Copy from user failed. (ret=%d)\n", ret);
        return -EINVAL;
    }

    if (devid >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("Invalid device id. (dev_id=%u)\n", devid);
        return -EINVAL;
    }

    ret = devdrv_manager_trans_and_check_id(devid, &phys_id, &vfid, ONLY_DOES_SUPPORT_PF);
    if (ret != 0) {
        devdrv_drv_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n", devid, ret);
        return ret;
    }

#if (defined CFG_FEATURE_VFIO)
    ret = vmngd_get_device_vf_max(phys_id, &vf_max_num);
    if (ret) {
        devdrv_drv_err("Get device vf max failed. (dev_id=%u)\n", devid);
        return -EINVAL;
    }
#endif
    ret = copy_to_user_safe((void *)((uintptr_t)arg), &vf_max_num, sizeof(u32));
    if (ret) {
        devdrv_drv_err("Copy_to_user_safe failed. (dev_id=%u; ret=%d)\n", devid, ret);
        return -EINVAL;
    }

    return 0;
}

STATIC int devdrv_manager_get_device_vf_list(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct virtmng_vf_list get_vf_list = {0};
    u32 devid;
    int ret;
    u32 phys_id = DEVDRV_MAX_DAVINCI_NUM + 1, vfid = 0;

    ret = copy_from_user_safe(&get_vf_list, (void *)(uintptr_t)arg, sizeof(struct virtmng_vf_list));
    if (ret) {
        devdrv_drv_err("Copy from user failed. (ret=%d)\n", ret);
        return -EINVAL;
    }

    devid = get_vf_list.dev_id;
    if (devid >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("Invalid device id. (dev_id=%u)\n", devid);
        return -EINVAL;
    }

    ret = devdrv_manager_trans_and_check_id(devid, &phys_id, &vfid, ONLY_DOES_SUPPORT_PF);
    if (ret != 0) {
        devdrv_drv_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n", devid, ret);
        return ret;
    }

#if (defined CFG_FEATURE_VFIO)
    ret = vmngd_get_device_vf_list(phys_id, get_vf_list.vf_list, VDAVINCI_MAX_VFID_NUM, &get_vf_list.vf_num);
    if (ret) {
        devdrv_drv_err("Get device vf max failed. (dev_id=%u)\n", devid);
        return -EINVAL;
    }
#endif
    ret = copy_to_user_safe((void *)((uintptr_t)arg), &get_vf_list, sizeof(struct virtmng_vf_list));
    if (ret) {
        devdrv_drv_err("Copy_to_user_safe failed. (dev_id=%u; ret=%d)\n", devid, ret);
        return -EINVAL;
    }

    return 0;
}

#ifndef CFG_SOC_PLATFORM_CLOUD_V2
STATIC void devdrv_manager_set_flow_control_reg(void)
{
#if !defined(DEVMNG_UT) && defined(CFG_SOC_PLATFORM_CLOUD)
    unsigned long reg_base_addr[DEVDRV_L3T_REG_NUM] = {
        DEVDRV_L3_TAG0_REG,
        DEVDRV_L3_TAG1_REG,
        DEVDRV_L3_TAG2_REG,
        DEVDRV_L3_TAG3_REG
    };
    void *addr = NULL;
    int i, j;

    for (i = 0; i < DEVDRV_MAX_DAVINCI_NUM; i++) {
        if (dev_manager_info->dev_id[i] == INVAILD_DEVICE_ID) {
            continue;
        }

        for (j = 0; j < DEVDRV_L3T_REG_NUM; j++) {
            addr = ioremap(reg_base_addr[j] + DEVDRV_L3T_REG_OFFSEF +
                               (dev_manager_info->dev_id[i] % MAX_CHIP_NUM) * DEVDRV_CHIP_ADDR_OFFSET,
                           sizeof(u32));
            if (addr == NULL) {
                devdrv_drv_err("set devid(%u) DEVDRV_L3_TAG(%d)_REG failed.\n", dev_manager_info->dev_id[i], j);
                return;
            }
            writel(readl(addr) | DEVDRV_REQ_ENABLE_BIT | DEVDRV_DATA_ENABLE_BIT, addr);
            iounmap(addr);
        }
        devdrv_ddr_mpam_and_push_config(dev_manager_info->dev_id[i] % MAX_CHIP_NUM);
        devdrv_hbm_mpam_and_push_config(dev_manager_info->dev_id[i] % MAX_CHIP_NUM);
    }
    return;
#endif
}
#endif

#ifdef CFG_FEATURE_OLD_DEVID_TRANS
STATIC int devdrv_manager_get_local_devid(struct file *filep, unsigned int cmd, unsigned long arg)
{
    u32 local_devids[DEVDRV_MAX_DAVINCI_NUM] = {0};
    u32 num_dev;
    u8 i;
    int ret;

    ret = devdrv_manager_container_get_devids(local_devids, DEVDRV_MAX_DAVINCI_NUM, &num_dev,
        current->nsproxy->mnt_ns, DEVDRV_PF_TYPE);
    if (ret != 0) {
        devdrv_drv_err("Failed to get device list. (ret=%d)\n", ret);
        return -ENODEV;
    }

    if (num_dev == 0 || num_dev > DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("Failed to get device number. (num_dev=%u)\n", num_dev);
        return -EINVAL;
    }

    /* get device id assigned by local */
    for (i = 0; i < num_dev; i++) {
        local_devids[i] = i;
    }

    if (copy_to_user_safe((void *)((uintptr_t)arg), local_devids, (long)(unsigned)sizeof(u32) * num_dev)) {
        devdrv_drv_err("[dev_id = %u]:copy to user failed.\n", num_dev);
        return -EINVAL;
    }

    return 0;
}
#endif

STATIC int devdrv_manager_container_cmd(struct file *filep, unsigned int cmd, unsigned long arg)
{
    return devdrv_manager_container_process(filep, arg);
}

#ifdef CFG_SURPPORT_PCIE_HOST_DEVICE_COMM
STATIC int device_manager_sync_matrix_ready(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_manager_msg_info dev_manager_msg_info = {{0}, {0}};
    u32 devid = 0, phys_id = DEVDRV_MAX_DAVINCI_NUM + 1, vfid = 0;
    u32 out_len;
    int ret, matrix_flag;

    devdrv_drv_info("*** begin to inform device matrix state***\n");

    ret = copy_from_user_safe(&devid, (void *)((uintptr_t)arg), sizeof(u32));
    if (ret) {
        devdrv_drv_err("copy from user failed, ret = %d.\n", ret);
        return ret;
    }

    if (devid >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("invaild device id. devid = %u.\n", devid);
        return -EINVAL;
    }

    ret = devdrv_manager_trans_and_check_id(devid, &phys_id, &vfid, ONLY_DOES_SUPPORT_PF);
    if (ret != 0) {
        devdrv_drv_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n", devid, ret);
        return ret;
    }

    dev_manager_msg_info.header.dev_id = phys_id;
    dev_manager_msg_info.header.msg_id = DEVDRV_MANAGER_CHAN_D2H_SYNC_MATRIX_READY;
    dev_manager_msg_info.header.valid = (u16)DEVDRV_MANAGER_MSG_VALID;
    dev_manager_msg_info.header.result = (u16)DEVDRV_MANAGER_MSG_INVALID_RESULT;

    ret = agentdrv_common_msg_send(devid, &dev_manager_msg_info, sizeof(dev_manager_msg_info),
                                   sizeof(dev_manager_msg_info), &out_len, AGENTDRV_COMMON_MSG_DEVDRV_MANAGER);
    if (ret != 0) {
        devdrv_drv_err("[dev_id = %u]:common msg send failed, ret = %d\n", phys_id, ret);
        return -ENODEV;
    }
    if (dev_manager_msg_info.header.result != 0) {
        devdrv_drv_err("[dev_id = %u]:matrix daemon sync failed, result = %u\n", phys_id,
                       dev_manager_msg_info.header.result);
        return -ENODEV;
    }

    matrix_flag = DEVDRV_MANAGER_MATRIX_VALID;
    ret = copy_to_user_safe((void *)(uintptr_t)arg, &matrix_flag, sizeof(int));
    if (ret) {
        devdrv_drv_err("[dev_id = %u]:copy_to_user_safe failed.\n", phys_id);
        return -EINVAL;
    }

    devdrv_drv_info("*** inform device matrix ready info ***\n");

    return 0;
}
#endif

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

#if (defined(CFG_SOC_PLATFORM_CLOUD) || defined(CFG_SOC_PLATFORM_MINIV2))
    bbox_system_error(0, excepid, &timestamp, 0);
#else
    mntn_system_error(excepid, timestamp, 0);
#endif

    return 0;
}

STATIC int devdrv_manager_get_container_flag(struct file *filep, unsigned int cmd, unsigned long arg)
{
    unsigned int flag;
    int ret;

    ret = devdrv_manager_container_is_in_container();
    if (ret < 0) {
        devdrv_drv_err("get contianer flag failed, ret(%d).\n", ret);
        return ret;
    }

    flag = (unsigned int)ret;
    ret = copy_to_user_safe((void *)((uintptr_t)arg), &flag, sizeof(unsigned int));

    return ret;
}

STATIC int devdrv_manager_get_process_sign(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct process_sign dev_sign = {0};
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

#ifndef CFG_FEATURE_RC_MODE
STATIC int devdrv_manager_set_startfinish(unsigned int dev_index, unsigned int device_process_status)
{
    struct devdrv_info *dev_info = NULL;
    dev_info = devdrv_manager_get_devdrv_info(dev_index);
    if ((dev_info == NULL) || (dev_info->shm_status == NULL)) {
        return EFAULT;
    }
    dev_info->shm_status->os_status = (u16)device_process_status;
    dev_info->dmp_started = true;
    return 0;
}

STATIC int devdrv_manager_set_all_startfinish(unsigned int device_process_status)
{
    struct devdrv_info *dev_info = NULL;
    int num_os_status_set = 0;
    unsigned int dev_index;
    unsigned int num_dev;
    unsigned int num_vdev;
    int ret;
    ret = devdrv_get_devnum(&num_dev);
    if (ret != 0) {
        devdrv_drv_err("devdrv_get_devnum failed, ret(%d).\n", ret);
        return -EINVAL;
    }
    ret = devdrv_get_vdevnum(&num_vdev);
    if (ret != 0) {
        devdrv_drv_err("Get VF num failed. (ret=%d)\n", ret);
        return -EINVAL;
    }

    for (dev_index = 0; dev_index < DEVDRV_MAX_DAVINCI_NUM; dev_index++) {
        dev_info = devdrv_manager_get_devdrv_info(dev_index);
        if ((dev_info == NULL) || (dev_info->shm_status == NULL)) {
            continue;
        }
        dev_info->shm_status->os_status = (u16)device_process_status;
        dev_info->dmp_started = true;
        num_os_status_set++;
    }
    return num_os_status_set == (num_dev + num_vdev) ? 0 : EFAULT;
}
#endif

STATIC int devdrv_manager_update_device_startup_status(struct file *filep, unsigned int cmd, unsigned long arg)
{
#ifdef CFG_FEATURE_RC_MODE
    return 0;
#else
    int ret;
    struct devdrv_device_work_status para = {0};
    const char *process_name[WHITE_LIST_PROCESS_NUM] = {PROCESS_NAME_DMP};
    ret = copy_from_user_safe(&para, (void *)(uintptr_t)arg, sizeof(struct devdrv_device_work_status));
    if (ret != 0) {
       devdrv_drv_err("copy from user failed, ret(%d).\n", ret);
       return -EINVAL;
    }

    if (para.device_id == 0) {
        /* check process whitelist */
        ret = whitelist_process_handler(process_name, WHITE_LIST_PROCESS_NUM);
        if (ret != 0) {
            devdrv_drv_err("whitelist_process_handler error. (ret=%d)\n", ret);
            return ret;
        }
        ret = devdrv_manager_set_all_startfinish(para.device_process_status);
    } else {
        ret = devdrv_manager_set_startfinish(para.device_id, para.device_process_status);
    }

    if (ret != 0) {
        devdrv_drv_err("set finish flag failed, ret(%d).\n", ret);
        return ret;
    }

    return 0;
#endif
}

#ifdef CFG_SOC_PLATFORM_MDC_V51
#include "devdrv_ipc_msg.h"
STATIC int devdrv_manager_safetyisland_ipc_check(struct devIpcMessage *msg)
{
    struct IpcMessage *ipc_info = NULL;

    if (msg == NULL) {
        devdrv_drv_err("msg is null.\n");
        return -EINVAL;
    }

    ipc_info = &msg->ipcMsg;

    if (ipc_info->head.msg_type != MSGTYPE_MS_STATUS) {
        devdrv_drv_err("invalid msg_type, msg_type:%u.\n", ipc_info->head.msg_type);
        return -EINVAL;
    }

    if ((ipc_info->head.cmd_type != CMDTYPE_INQUERY)
        && (ipc_info->head.cmd_type != CMDTYPE_SETTING)
        && (ipc_info->head.cmd_type != CMDTYPE_NOTIFY)) {
        devdrv_drv_err("invalid cmd_type, cmd_type:%u.\n", ipc_info->head.cmd_type);
        return -EINVAL;
    }

    if ((ipc_info->head.cmdDest != CPUID_SAFETYISLAND) || (ipc_info->head.cmdSrc != CPUID_FM_TAISHAN)) {
        devdrv_drv_err("invalid cmd, cmdSrc:%u, cmdDest:%u.\n", ipc_info->head.cmdSrc, ipc_info->head.cmdDest);
        return -EINVAL;
    }

    ipc_info->crc = sils_crc16((unsigned char *)ipc_info, ipc_info->head.msg_length + SILS_IPC_HEAD_LEN);

    return 0;
}
STATIC int devdrv_manager_safetyisland_ipc_send(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    struct devIpcMessage ipc_msg = {
        .ipcMsg = {
            .head = {0},
            .payload = {0},
            .crc = 0
        },
        .moduleId = 0,
        .channelId = 0,
        .devId = 0
    };

    ret = copy_from_user_safe(&ipc_msg, (void *)((uintptr_t)arg), sizeof(struct devIpcMessage));
    if (ret != 0) {
        devdrv_drv_err("copy from user failed, ret:%d\n", ret);
        return ret;
    }

    ret = devdrv_manager_safetyisland_ipc_check(&ipc_msg);
    if (ret != 0) {
        devdrv_drv_err("safetyisland ipc check failed, devid:%u.\n", ipc_msg.devId);
        return -EINVAL;
    }

    ret = devdrv_safetyisland_ipc_send(&ipc_msg);
    if (ret != 0) {
        devdrv_drv_err("safetyisland ipc send failed, ret:%d.\n", ret);
        return ret;
    }

    return 0;
}

STATIC int devdrv_manager_safetyisland_ipc_recv(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    struct devIpcMessage ipc_msg = {
        .ipcMsg = {
            .head = {0},
            .payload = {0},
            .crc = 0
        },
        .moduleId = 0,
        .channelId = 0,
        .devId = 0
    };

    ret = copy_from_user_safe(&ipc_msg, (void *)((uintptr_t)arg), sizeof(struct devIpcMessage));
    if (ret != 0) {
        devdrv_drv_err("copy from user failed, ret:%d\n", ret);
        return -EINVAL;
    }

    ret = devdrv_safetyisland_ipc_recv(&ipc_msg);
    if (ret != 0) {
        devdrv_drv_err("safetyisland ipc recv failed, ret:%d\n", ret);
        return ret;
    }

    ret = copy_to_user_safe((void *)((uintptr_t)arg), (void *)&ipc_msg, sizeof(struct devIpcMessage));
    if (ret != 0) {
        devdrv_drv_err("copy to user failed, ret:%d devid:%u\n", ret, ipc_msg.devId);
        return ret;
    }

    return 0;
}

#define BOOTSTRAP_REG_PHY_ADDR  0x8000E08C
#define BOOTSTRAP_REG_READ_LEN  4
#define GET_BOOTSTRAP_OFFSET    9
#define BOOTSTRAP_REG_MASK      0x00000600

int devdrv_manager_get_bootstrap(unsigned int *bootstrap)
{
    void __iomem *bootstrap_vir_addr = NULL;
    unsigned int reg_data;

    bootstrap_vir_addr = (void __iomem *)ioremap_wc(BOOTSTRAP_REG_PHY_ADDR, BOOTSTRAP_REG_READ_LEN);
    if (bootstrap_vir_addr == NULL) {
        devdrv_drv_err("bootstrap_vir_addr memory req fail");
        return -EINVAL;
    }

    reg_data = readl_relaxed(bootstrap_vir_addr);
    iounmap(bootstrap_vir_addr);
    bootstrap_vir_addr = NULL;

    // get reg_data bit10-bit9
    *bootstrap = (reg_data & BOOTSTRAP_REG_MASK) >> GET_BOOTSTRAP_OFFSET;
    return 0;
}
EXPORT_SYMBOL(devdrv_manager_get_bootstrap);

STATIC int devdrv_manager_send_bootstrap_to_user(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    unsigned int bootstrap;

    ret = devdrv_manager_get_bootstrap(&bootstrap);
    if (ret != 0) {
        devdrv_drv_err("get bootstrap failed, ret:%d\n", ret);
        return ret;
    }

    ret = copy_to_user_safe((void *)((uintptr_t)arg), (void *)&bootstrap, sizeof(unsigned int));
    if (ret != 0) {
        devdrv_drv_err("copy to user failed, ret:%d\n", ret);
        return ret;
    }

    return 0;
}

#endif

#ifdef CFG_FEATURE_CHIP_DIE
STATIC char *g_random = NULL;
STATIC int devdrv_manager_get_random_save_to_bar(u32 dev_id)
{
    int ret;

    if (devdrv_manager_is_pf_device(dev_id) == false) {
        /* vdevice no need to do this */
        return 0;
    }

    if (g_random == NULL) {
        g_random = (char *)kzalloc(DEVDRV_RANDOM_SIZE * sizeof(char), GFP_KERNEL | __GFP_ACCOUNT);
        if (g_random == NULL) {
            devdrv_drv_err("Kzalloc memory for g_random failed.\n");
            return -EINVAL;
        }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
        get_random_bytes(g_random, (int)DEVDRV_RANDOM_SIZE);
#else
        ret = devdrv_get_random(g_random, DEVDRV_RANDOM_SIZE);
        if (ret) {
            devdrv_drv_err("Get random failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
            kfree(g_random);
            g_random = NULL;
            return ret;
        }
#endif
    }

    ret = devdrv_fresh_random_to_shm(dev_id, g_random, DEVDRV_RANDOM_SIZE);
    if (ret) {
        devdrv_drv_err("Fresh random to shm failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    return 0;
}

int devdrv_manager_get_random_from_bar(u32 devid, char *random_number, u32 random_len)
{
    int ret;

    if ((random_len < DEVMNG_SHM_INFO_RANDOM_SIZE) || (random_number == NULL)) {
        devdrv_drv_err("Invalid parameter. (random_len=%d; random_number=\"%s\")\n",
                       random_len, (random_number == NULL) ? "NULL" : "OK");
        return -EINVAL;
    }

    if (g_random == NULL) {
        devdrv_drv_err("g_random is NULL. (devid=%u)\n", devid);
        return -EINVAL;
    }

    ret = memcpy_s(random_number, random_len, g_random, DEVMNG_SHM_INFO_RANDOM_SIZE);
    if (ret) {
        devdrv_drv_err("Memcpy random from shm failed. (devid=%u; ret=%d)\n", devid, ret);
        return ret;
    }

    return 0;
}

int devdrv_manager_ioctl_get_chip_count(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    int count = 0;

    ret = devdrv_manager_get_chip_count(&count);
    if (ret != 0) {
        devdrv_drv_err("devdrv_manager_get_chip_count fail, ret=%d.\n", ret);
        return ret;
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
    struct devdrv_chip_list chip_list = {0};

    ret = copy_from_user_safe(&chip_list, (void *)((uintptr_t)arg), sizeof(struct devdrv_chip_list));
    if (ret != 0) {
        devdrv_drv_err("copy_from_user_safe fail, ret(%d).\n", ret);
        return ret;
    }

    ret = devdrv_manager_get_chip_list(&chip_list);
    if (ret != 0) {
        devdrv_drv_err("devdrv_manager_get_chip_list fail, ret=%d.\n", ret);
        return ret;
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
    struct devdrv_chip_dev_list chip_dev_list = {0};

    ret = copy_from_user_safe(&chip_dev_list, (void *)((uintptr_t)arg), sizeof(struct devdrv_chip_dev_list));
    if (ret != 0) {
        devdrv_drv_err("copy_from_user_safe fail, ret(%d).\n", ret);
        return ret;
    }

    ret = devdrv_manager_get_device_from_chip(&chip_dev_list);
    if (ret != 0) {
        devdrv_drv_err("devdrv_manager_get_device_from_chip fail, ret=%d.\n", ret);
        return ret;
    }

    ret = copy_to_user_safe((void *)(uintptr_t)arg, &chip_dev_list, sizeof(struct devdrv_chip_dev_list));
    if (ret != 0) {
        devdrv_drv_err("copy to user failed, ret=%d.\n", ret);
        return -EFAULT;
    }

    return 0;
}

int devdrv_manager_ioctl_get_chip_from_device(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    struct devdrv_get_dev_chip_id chip_from_dev = {0};

    ret = copy_from_user_safe(&chip_from_dev, (void *)((uintptr_t)arg), sizeof(struct devdrv_get_dev_chip_id));
    if (ret != 0) {
        devdrv_drv_err("copy_from_user_safe fail, ret(%d).\n", ret);
        return ret;
    }

    ret = devdrv_manager_get_chip_from_device(&chip_from_dev);
    if (ret != 0) {
        devdrv_drv_err("devdrv_manager_get_chip_from_device fail, ret=%d.\n", ret);
        return ret;
    }

    ret = copy_to_user_safe((void *)(uintptr_t)arg, &chip_from_dev, sizeof(struct devdrv_get_dev_chip_id));
    if (ret != 0) {
        devdrv_drv_err("copy to user failed, ret=%d.\n", ret);
        return -EFAULT;
    }

    return 0;
}
#endif

int devdrv_get_ffts_type(unsigned int *op_val)
{
#ifdef CFG_SOC_PLATFORM_CLOUD_V2
    *op_val = 1; /* ffts+: 1 */
#else
    *op_val = 0; /* ffts: 0 */
#endif

    return 0;
}

#ifdef CFG_FEATURE_IPC_NOTIFY
#ifndef CFG_TRS_REFACTOR_FEATURE
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
            devdrv_drv_err("invalid cmd, cmd = %u\n", _IOC_NR(cmd));
            ret = -EFAULT;
            break;
    };
    devdrv_manager_ops_sem_up_read();
    (void)memset_s(notify_ioctl_info.name, DEVDRV_IPC_NAME_SIZE, 0, DEVDRV_IPC_NAME_SIZE);

    return ret;
}
#endif
#endif

#ifdef CFG_FEATURE_PSS_SIGN
#define PKCS_SIGN_TYPE_OFF  1
#define PKCS_SIGN_TYPE_ON   0
STATIC int devdrv_manager_ioctl_set_sign_type(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    struct devdrv_sign_verification_info sign_info = {0};
    const char *process_name[WHITE_LIST_PROCESS_NUM] = {PROCESS_NAME_DMP};

    /* check process whitelist */
    ret = whitelist_process_handler(process_name, WHITE_LIST_PROCESS_NUM);
    if (ret != 0) {
        devdrv_drv_err("whitelist_process_handler error. (ret=%d)\n", ret);
        return ret;
    }

    ret = copy_from_user_safe(&sign_info, (void *)((uintptr_t)arg), sizeof(struct devdrv_sign_verification_info));
    if (ret != 0) {
        devdrv_drv_err("Copy from user failed. (ret=%d)\n", ret);
        return -EFAULT;
    }

    if (sign_info.sign_buf != PKCS_SIGN_TYPE_OFF && sign_info.sign_buf != PKCS_SIGN_TYPE_ON) {
        devdrv_drv_err("Invalid parameters. (dev_id=%u; size_buf=%u)\n", sign_info.dev_id, sign_info.sign_buf);
        return -EFAULT;
    }

    ret = devdrv_config_set_pss_cfg(sign_info.dev_id, sign_info.sign_buf);
    if (ret != 0) {
        devdrv_drv_err("Failed to invoke devdrv_config_set_pss_cfg. (ret=%d)\n", ret);
        return -EFAULT;
    }

    return ret;
}

STATIC int devdrv_manager_ioctl_get_sign_type(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    int sign;
    struct devdrv_sign_verification_info sign_info = {0};

    ret = copy_from_user_safe(&sign_info, (void *)((uintptr_t)arg), sizeof(struct devdrv_sign_verification_info));
    if (ret != 0) {
        devdrv_drv_err("Copy from user failed. (ret=%d)\n", ret);
        return -EFAULT;
    }

    ret = devdrv_config_get_pss_cfg(sign_info.dev_id, &sign);
    if (ret != 0) {
        devdrv_drv_err("Failed to invoke devdrv_config_get_pss_cfg. (ret=%d)\n", ret);
        return -EFAULT;
    }

    sign_info.sign_buf = (unsigned char)sign;
    ret = copy_to_user_safe((void *)((uintptr_t)arg), &sign_info, sizeof(struct devdrv_sign_verification_info));
    if (ret) {
        devdrv_drv_err("copy_to_user_safe failed. (ret=%d)\n", ret);
        return ret;
    }

    return ret;
}
#endif


STATIC int (*devdrv_manager_ioctl_handlers[DEVDRV_MANAGER_CMD_MAX_NR])(struct file *filep, unsigned int cmd,
    unsigned long arg) = {
        [_IOC_NR(DEVDRV_MANAGER_GET_PCIINFO)] = NULL,
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
        [_IOC_NR(DEVDRV_MANAGER_GET_CONTAINER_DEVIDS)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_DEVINFO)] = devdrv_manager_devinfo_ioctl,
#ifdef CFG_FEATURE_OLD_DEVID_TRANS
        [_IOC_NR(DEVDRV_MANAGER_GET_DEVID_BY_LOCALDEVID)] = devdrv_manager_devinfo_ioctl,
#endif
        [_IOC_NR(DEVDRV_MANAGER_GET_DEV_INFO_BY_PHYID)] = devdrv_manager_devinfo_ioctl,
        [_IOC_NR(DEVDRV_MANAGER_GET_PCIE_ID_INFO)] = devdrv_manager_inquiry_info,
        [_IOC_NR(DEVDRV_MANAGER_GET_FLASH_COUNT)] = devdrv_manager_inquiry_info,
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
        [_IOC_NR(DEVDRV_MANAGER_GET_HBM_BW_UTILIZATION)] = devdrv_manager_imu_cmd,
        [_IOC_NR(DEVDRV_MANAGER_DEBUG_INFORM)] = devdrv_manager_imu_cmd,
        [_IOC_NR(DEVDRV_MANAGER_IMU_SMOKE)] = devdrv_manager_imu_cmd,
        [_IOC_NR(DEVDRV_MANAGER_BLACK_BOX_GET_EXCEPTION)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_DEVICE_MEMORY_DUMP)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_DEVICE_RESET_INFORM)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_MODULE_STATUS)] = devdrv_manager_get_module_status,
        [_IOC_NR(DEVDRV_MANAGER_REG_DDR_READ)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_MINI_BOARD_ID)] = devdrv_manager_get_board_id,
        [_IOC_NR(DEVDRV_MANAGER_PCIE_PRE_RESET)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_PCIE_RESCAN)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_PCIE_HOT_RESET)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_P2P_ATTR)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_ALLOC_HOST_DMA_ADDR)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_PCIE_READ)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_PCIE_SRAM_WRITE)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_EMMC_VOLTAGE)] = devdrv_manager_get_emmc_voltage,
        [_IOC_NR(DEVDRV_MANAGER_GET_DEVICE_BOOT_STATUS)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_ENABLE_EFUSE_LDO)] = devdrv_manager_enable_efuse_ldo2,
        [_IOC_NR(DEVDRV_MANAGER_DISABLE_EFUSE_LDO)] = devdrv_manager_disable_efuse_ldo2,

        [_IOC_NR(DEVDRV_MANAGER_CONTAINER_CMD)] = devdrv_manager_container_cmd,
        [_IOC_NR(DEVDRV_MANAGER_GET_HOST_PHY_MACH_FLAG)] = devdrv_manager_get_host_phy_mach_flag,
#ifdef CFG_FEATURE_OLD_DEVID_TRANS
        [_IOC_NR(DEVDRV_MANAGER_GET_LOCAL_DEVICEIDS)] = devdrv_manager_get_local_devid,
#endif
#ifdef CFG_FEATURE_IPC_NOTIFY
#ifndef CFG_TRS_REFACTOR_FEATURE
        [_IOC_NR(DEVDRV_MANAGER_IPC_NOTIFY_CREATE)] = devdrv_manager_ipc_notify_ioctl,
        [_IOC_NR(DEVDRV_MANAGER_IPC_NOTIFY_OPEN)] = devdrv_manager_ipc_notify_ioctl,
        [_IOC_NR(DEVDRV_MANAGER_IPC_NOTIFY_CLOSE)] = devdrv_manager_ipc_notify_ioctl,
        [_IOC_NR(DEVDRV_MANAGER_IPC_NOTIFY_DESTROY)] = devdrv_manager_ipc_notify_ioctl,
        [_IOC_NR(DEVDRV_MANAGER_IPC_NOTIFY_SET_PID)] = devdrv_manager_ipc_notify_ioctl,
#endif
#endif
        [_IOC_NR(DEVDRV_MANAGER_SET_NEW_TIME)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_PMU_VOLTAGE)] = devdrv_manager_get_pmu_voltage,
        [_IOC_NR(DEVDRV_MANAGER_GET_PMU_DIEID)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_CPU_INFO)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_SEND_TO_IMU)] = devdrv_manager_imu_cmd,
        [_IOC_NR(DEVDRV_MANAGER_RECV_FROM_IMU)] = devdrv_manager_imu_cmd,
        [_IOC_NR(DEVDRV_MANAGER_GET_IMU_INFO)] = devdrv_manager_imu_cmd,
        [_IOC_NR(DEVDRV_MANAGER_CONFIG_ECC_ENABLE)] = devdrv_manager_imu_cmd,
        [_IOC_NR(DEVDRV_MANAGER_GET_LLC_PERF_PARA)] = devdrv_manager_inquiry_info_ex,
        [_IOC_NR(DEVDRV_MANAGER_GET_PROBE_NUM)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_PROBE_LIST)] = NULL,
#ifdef CFG_SURPPORT_PCIE_HOST_DEVICE_COMM
        [_IOC_NR(DEVDRV_MANAGER_SYNC_MATRIX_DAEMON_READY)] = device_manager_sync_matrix_ready,
#endif
        [_IOC_NR(DEVDRV_MANAGER_GET_BBOX_ERRSTR)] = devdrv_manager_imu_cmd,
        [_IOC_NR(DEVDRV_MANAGER_PCIE_IMU_DDR_READ)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_SLOT_ID)] = devdrv_manager_get_slot_id,
        [_IOC_NR(DEVDRV_MANAGER_GET_SOC_DIE_ID)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_CHIP_INFO)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_RST_I2C_CTROLLER)] = devdrv_manager_inquiry_info_ex,
        [_IOC_NR(DEVDRV_MANAGER_GET_XLOADER_BOOT_INFO)] = devdrv_manager_inquiry_info_ex,
        [_IOC_NR(DEVDRV_MANAGER_GET_GPIO_STATE)] = devdrv_manager_inquiry_info_ex,
        [_IOC_NR(DEVDRV_MANAGER_APPMON_BBOX_EXCEPTION_CMD)] = devdrv_manager_appmon_bbox_exception_hndl,
        [_IOC_NR(DEVDRV_MANAGER_GET_CONTAINER_FLAG)] = devdrv_manager_get_container_flag,
        [_IOC_NR(DEVDRV_MANAGER_GET_PROCESS_SIGN)] = devdrv_manager_get_process_sign,
        [_IOC_NR(DEVDRV_MANAGER_GET_MASTER_DEV_IN_THE_SAME_OS)] = NULL,
#ifdef CFG_SURPPORT_PCIE_HOST_DEVICE_COMM
        [_IOC_NR(DEVDRV_MANAGER_GET_LOCAL_DEV_ID_BY_HOST_DEV_ID)] = devdrv_manager_get_local_devid_by_host_devid,
#endif
#ifdef CFG_FEATURE_HISS
        [_IOC_NR(DEVDRV_MANAGER_GET_HISSSTATUS)] = devdrv_manager_get_hiss_status,
        [_IOC_NR(DEVDRV_MANAGER_GET_FLASH_INFO)] = devdrv_manager_get_flash_info,
#endif
#ifdef CFG_FEATURE_LP_ENABLE
        [_IOC_NR(DEVDRV_MANAGER_SET_POWER_STATE)] = devdrv_manager_set_power_state,
        [_IOC_NR(DEVDRV_MANAGER_GET_TS_GROUP_NUM)] = devdrv_manager_get_ts_group_num,
        [_IOC_NR(DEVDRV_MANAGER_GET_DVPP_RATIO)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_DVPP_STATUS)] = NULL,
#ifdef CFG_FEATURE_CAPABILITY_GROUP
        [_IOC_NR(DEVDRV_MANAGER_GET_CAPABILITY_GROUP_INFO)] = devdrv_manager_get_capability_group_info,
        [_IOC_NR(DEVDRV_MANAGER_DELETE_CAPABILITY_GROUP)] = devdrv_manager_delete_capability_group,
        [_IOC_NR(DEVDRV_MANAGER_CREATE_CAPABILITY_GROUP)] = devdrv_manager_create_capability_group,
#endif
#ifdef CFG_SOC_PLATFORM_MDC_V51
        [_IOC_NR(DEVDRV_MANAGER_GET_CANSTATUS)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_UFSSTATUS)] = devdrv_manager_get_ufs_status,
        [_IOC_NR(DEVDRV_MANAGER_GET_SENSORHUBSTATUS)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_UFSINFO)] = devdrv_manager_get_ufs_info,
        [_IOC_NR(DEVDRV_MANAGER_SET_UFSINFO)] = devdrv_manager_set_ufs_info,
        [_IOC_NR(DEVDRV_MANAGER_GET_SENSORHUBCONFIG)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_ISPSTATUS)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_ISPCONFIG)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_ALL_TEMPERATURE)] = devdrv_manager_inquiry_info,
        [_IOC_NR(DEVDRV_MANAGER_GET_EMU_SUBSYS_STATUS)] = devdrv_manager_get_emu_subsys_status,
        [_IOC_NR(DEVDRV_MANAGER_GET_SAFETYISLAND_STATUS)] = devdrv_manager_get_safetyisland_status,
        [_IOC_NR(DEVDRV_MANAGER_SAFETYISLAND_IPC_MSG_SEND)] = devdrv_manager_safetyisland_ipc_send,
        [_IOC_NR(DEVDRV_MANAGER_SAFETYISLAND_IPC_MSG_RECV)] = devdrv_manager_safetyisland_ipc_recv,
        [_IOC_NR(DEVDRV_MANAGER_GET_CAN_CONFIG)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_SET_CAN_CONFIG)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_SUBSYS_TEMPERATURE)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_GET_POWER_STATE)] = devdrv_manager_get_power_state,
        [_IOC_NR(DEVDRV_MANAGER_GET_TEMP_THOLD)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_SET_TEMP_THOLD)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_EQUIPMENT_SET_SILS_INFO)] = devdrv_manager_equipment_set_safety_island_info,
        [_IOC_NR(DEVDRV_MANAGER_EQUIPMENT_GET_SILS_INFO)] = devdrv_manager_equipment_get_safety_island_info,
        [_IOC_NR(DEVDRV_MANAGER_GET_BOOTSTRAP)] = devdrv_manager_send_bootstrap_to_user,
        [_IOC_NR(DEVDRV_MANAGER_GET_SILSINFO)] = devdrv_manager_get_sils_info,
        [_IOC_NR(DEVDRV_MANAGER_SET_SILSINFO)] = devdrv_manager_set_sils_info,
#endif
#endif

#ifdef CFG_FEATURE_HOST_UNBIND
        [_IOC_NR(DEVDRV_MANAGER_UNBIND_PID_ID)] = devdrv_fop_unbind_host_pid,
#endif

        [_IOC_NR(DEVDRV_MANAGER_GET_TSDRV_DEV_COM_INFO)] = devdrv_manager_get_tsdrv_dev_com_info,
        [_IOC_NR(DEVDRV_MANAGER_GET_P2P_CAPABILITY)] = NULL,
#ifdef CFG_SOC_PLATFORM_CLOUD
        [_IOC_NR(DEVDRV_MANAGER_GET_ETH_ID)] = devdrv_manager_get_eth_id,
#endif
        [_IOC_NR(DEVDRV_MANAGER_REG_VMNG_CLIENT)] = devdrv_manager_register_vmng_client,
        [_IOC_NR(DEVDRV_MANAGER_BIND_PID_ID)] = devdrv_fop_bind_host_pid,
        [_IOC_NR(DEVDRV_MANAGER_QUERY_DEV_PID)] = devdrv_query_devpid,
        [_IOC_NR(DEVDRV_MANAGER_GET_H2D_DEVINFO)] = devdrv_manager_devinfo_ioctl,
        [_IOC_NR(DEVDRV_MANAGER_GET_CONSOLE_LOG_LEVEL)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_IPC_UNIFY_MSG)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_UPDATE_STARTUP_STATUS)] = devdrv_manager_update_device_startup_status,
        [_IOC_NR(DEVDRV_MANAGER_GET_DEVICE_HEALTH_STATUS)] = NULL,
        [_IOC_NR(DEVDRV_MANAGER_QUERY_HOST_PID)] = devdrv_fop_query_host_pid,
        [_IOC_NR(DEVDRV_MANAGER_GET_DEV_RESOURCE_INFO)] = devdrv_manager_ioctl_get_dev_resource_info,
#if (defined CFG_FEATURE_VFIO) && (defined CFG_FEATURE_RC_MODE)
        [_IOC_NR(DEVDRV_MANAGER_CREATE_VDEV)] = devdrv_manager_ioctl_create_vdev,
        [_IOC_NR(DEVDRV_MANAGER_DESTROY_VDEV)] = devdrv_manager_ioctl_destroy_vdev,
        [_IOC_NR(DEVDRV_MANAGER_GET_VDEVINFO)] = devdrv_manager_ioctl_get_vdevinfo,
#endif
#ifdef CFG_FEATURE_CHIP_DIE
        [_IOC_NR(DEVDRV_MANAGER_GET_CHIP_COUNT)] = devdrv_manager_ioctl_get_chip_count,
        [_IOC_NR(DEVDRV_MANAGER_GET_CHIP_LIST)] = devdrv_manager_ioctl_get_chip_list,
        [_IOC_NR(DEVDRV_MANAGER_GET_DEVICE_FROM_CHIP)] = devdrv_manager_ioctl_get_device_from_chip,
        [_IOC_NR(DEVDRV_MANAGER_GET_CHIP_FROM_DEVICE)] = devdrv_manager_ioctl_get_chip_from_device,
#endif
        [_IOC_NR(DEVDRV_MANAGER_GET_DEVICE_VF_MAX)] = devdrv_manager_get_device_vf_max,
        [_IOC_NR(DEVDRV_MANAGER_GET_DEVICE_VF_LIST)] = devdrv_manager_get_device_vf_list,
#ifdef CFG_FEATURE_PSS_SIGN
        [_IOC_NR(DEVDRV_MANAGER_SET_SIGN)] = devdrv_manager_ioctl_set_sign_type,
        [_IOC_NR(DEVDRV_MANAGER_GET_SIGN)] = devdrv_manager_ioctl_get_sign_type,
#endif
#ifdef CFG_FEATURE_OLD_DEVID_TRANS
        [_IOC_NR(DEVDRV_MANAGER_GET_VDEVNUM)] = devdrv_manager_ioctl_get_vdevnum,
#endif
        [_IOC_NR(DEVDRV_MANAGER_GET_VDEVIDS)] = devdrv_manager_devinfo_ioctl,
#ifdef CFG_SOC_PLATFORM_MDC_V11
        [_IOC_NR(DEVDRV_MANAGER_SET_POWER_STATE)] = devdrv_manager_set_power_state_v2,
#endif
#ifdef CFG_FEATURE_DEVICE_SHARE
        [_IOC_NR(DEVDRV_MANAGER_CONFIG_DEVICE_SHARE)] = devdrv_manager_config_device_share,
#endif
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
                       "cmd = %u, max value = %d.\n",
                       dev_manager_info, argp, _IOC_NR(cmd), DEVDRV_MANAGER_CMD_MAX_NR);
        return -EINVAL;
    }

    if (devdrv_manager_ioctl_handlers[_IOC_NR(cmd)] == NULL) {
        devdrv_drv_err("invalid cmd, cmd = %u\n", _IOC_NR(cmd));
        return -EOPNOTSUPP;
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

void devdrv_manager_get_dump_ddr_addr(u32 dev_id, u64 *dump_ddr_dma_addr, u32 *dump_ddr_size)
{
    int ret;
    u64 phy_addr;
    u32 chip_id = 0;
    u32 die_id = 0;
    dma_addr_t dma_addr_d;
    devdrv_hardware_info_t hardware_info = {0};
    struct device *dev = dev_manager_info->dma_dev[dev_id];

    if (devdrv_manager_is_pf_device(dev_id) == false) {
        return;
    }

    if (devdrv_get_chip_die_id(dev_id, &chip_id, &die_id) != 0) {
        devdrv_drv_err("Get chip die id failed. (device_id=%u)\n", dev_id);
        *dump_ddr_dma_addr = (u64)(uintptr_t)NULL;
        return;
    }

    ret = hal_kernel_get_hardware_info(dev_id, &hardware_info);
    if (ret != 0) {
        devdrv_drv_err("Failed to invoke hal_kernel_get_hardware_info. (devid=%u; ret=%d)\n", dev_id, ret);
        *dump_ddr_dma_addr = (u64)(uintptr_t)NULL;
        return;
    }
#ifdef CFG_FEATURE_CHIP_OFFSET_OUTSIDE_2G
    /*
     * if phycical address is inside 2G：
     * 1. case 1: for die 0, no need to add chip offset;
     * 2. case 2: for die 1, need to add chip/die offset and die high offset.
     * otherwise, need to add chip and die offset
     */
    if (die_id == 0) {
        phy_addr = ASCEND_PLATFORM_MEMDUMP_ADDR;
    } else {
        phy_addr = ASCEND_PLATFORM_MEMDUMP_ADDR + hardware_info.phy_addr_offset + die_id * DIE_BASEADDR_HIGH_OFFSET_WITHIN_2G;
    }
#else
    phy_addr = ASCEND_PLATFORM_MEMDUMP_ADDR + hardware_info.phy_addr_offset;
#endif
    *dump_ddr_dma_addr = phy_addr;
    *dump_ddr_size = ASCEND_PLATFORM_MEMDUMP_SIZE;

    dma_addr_d = dma_map_resource(dev, phy_addr, *dump_ddr_size, DMA_BIDIRECTIONAL, 0);
    if (dma_mapping_error(dev, dma_addr_d)) {
        devdrv_drv_warn("Dump ddr address dma map error. (device_id=%u)\n", dev_id);
        *dump_ddr_dma_addr = (u64)(uintptr_t)NULL;
        return;
    }

    *dump_ddr_dma_addr = (u64)dma_addr_d;
}

#define CAP_REG_EXPORT_MASK 0x1
#define CAP_REG_CLEAR_MASK (~CAP_REG_EXPORT_MASK)

void devdrv_manager_get_reg_ddr_addr(u32 dev_id, struct devdrv_device_info *drv_info)
{
    int ret;
    u32 chip_id = 0;
    u32 die_id = 0;
    u64 phy_addr;
    devdrv_hardware_info_t hardware_info = {0};
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

    if (devdrv_manager_is_pf_device(dev_id) == false) {
        return;
    }

    if ((drv_info->capability & CAP_REG_EXPORT_MASK) == 0) {
        drv_info->reg_ddr_size = 0;
        return;
    }

    if (devdrv_get_chip_die_id(dev_id, &chip_id, &die_id) != 0) {
        devdrv_drv_err("Get chip die id failed. (device_id=%u)\n", dev_id);
        return;
    }

    ret = hal_kernel_get_hardware_info(dev_id, &hardware_info);
    if (ret != 0) {
        devdrv_drv_err("Failed to invoke hal_kernel_get_hardware_info. (devid=%u; ret=%d)\n", dev_id, ret);
        return;
    }

#ifdef CFG_FEATURE_CHIP_OFFSET_OUTSIDE_2G
    /*
     * if phycical address is inside 2G：
     * 1. case 1: for die 0, no need to add chip offset;
     * 2. case 2: for die 1, need to add chip/die offset and die high offset.
     * otherwise, need to add chip and die offset
     */
    if (die_id == 0) {
        phy_addr = PCIE_DDR_READ_REG_BASE;
    } else {
        phy_addr = PCIE_DDR_READ_REG_BASE + hardware_info.phy_addr_offset + die_id * DIE_BASEADDR_HIGH_OFFSET_WITHIN_2G;
    }
#else
    phy_addr = PCIE_DDR_READ_REG_BASE + hardware_info.phy_addr_offset;
#endif

    // avoid invalid phy_addr input to dma_map_resource
    if (pfn_valid(PHYS_PFN(phy_addr))) {
        drv_info->reg_ddr_size = 0;
        drv_info->capability &= CAP_REG_CLEAR_MASK;
        devdrv_drv_warn("dev_id %u firmware do not support reg export\n", dev_id);
        return;
    }

    drv_info->reg_ddr_dma_addr = phy_addr;
    drv_info->reg_ddr_size = PCIE_DDR_READ_REG_SIZE;

    dma_addr_d = dma_map_resource(dev, phy_addr, drv_info->reg_ddr_size, DMA_BIDIRECTIONAL, 0);
    if (dma_mapping_error(dev, dma_addr_d)) {
        drv_info->reg_ddr_size = 0;
        devdrv_drv_warn("dev_id %u reg ddr dma map error\n", dev_id);
        return;
    }

    drv_info->reg_ddr_dma_addr = (u64)dma_addr_d;
}

void devdrv_manager_inform_device_status(struct devdrv_info *info, enum devdrv_ts_status status)
{
#ifdef CFG_SURPPORT_PCIE_HOST_DEVICE_COMM
    struct devdrv_manager_msg_info dev_manager_msg_info = {{0}, {0}};
    u32 out_len;
    int ret;

    if (info == NULL) {
        devdrv_drv_err("info is NULL.\n");
        return;
    }

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
            devdrv_drv_err("[dev_id = %u]:invalid input status.\n", info->dev_id);
            return;
    }

    dev_manager_msg_info.header.valid = (u16)DEVDRV_MANAGER_MSG_VALID;
    dev_manager_msg_info.header.result = (u16)DEVDRV_MANAGER_MSG_INVALID_RESULT;

    ret = agentdrv_common_msg_send(info->dev_id, &dev_manager_msg_info, sizeof(dev_manager_msg_info),
                                   sizeof(dev_manager_msg_info), &out_len, AGENTDRV_COMMON_MSG_DEVDRV_MANAGER);
    if ((ret) || (dev_manager_msg_info.header.result != 0))
        devdrv_drv_err("[dev_id = %u]:inform host failed\n", info->dev_id);
#else
    devdrv_drv_err("not support in current enveroment\n");
#endif
}

#ifndef CFG_FEATURE_RC_MODE
STATIC void devdrv_manager_update_capability_info(u32 dev_id, struct devdrv_device_info *drv_info)
{
#ifdef CFG_SURPPORT_PCIE_HOST_DEVICE_COMM
    int ret;
    ret = agentdrv_read_capability(dev_id, &(drv_info->capability));
    if (ret) {
        drv_info->capability = 0;
        devdrv_drv_warn("Read device capability have problem. (dev_id=%u)\n", dev_id);
        return;
    }
#else
    devdrv_drv_info("Not support in current enviroment\n");
#endif
}

STATIC void devdrv_manager_clear_capability_info(u32 dev_id)
{
#ifdef CFG_SURPPORT_PCIE_HOST_DEVICE_COMM
    int ret;

    ret = agentdrv_clear_capability(dev_id);
    if (ret) {
        devdrv_drv_warn("Clear device capability have problem. (dev_id=%u)\n", dev_id);
        return ;
    }
#endif
}
#endif

#ifdef CFG_FEATURE_SRIOV
STATIC int devdrv_manager_get_template_name(u32 devid, u8 *name, u32 name_len)
{
    int ret;
    struct vmng_soc_resource_enquire info;

    if (name_len < VMNG_VF_TEMP_NAME_LEN) {
        devdrv_drv_err("Input name length is invalid. (devid=%u; name_len=%u)\n", devid, name_len);
        return -EINVAL;
    }

    (void)memset_s(&info, sizeof(struct vmng_soc_resource_enquire), 0, sizeof(struct vmng_soc_resource_enquire));
    ret = vmngd_enquire_soc_resource(devid, 0, &info);
    if (ret != 0) {
        devdrv_drv_err("Failed to invoke vmngd_enquire_soc_resource. (devid=%u; ret=%d)\n", devid, ret);
        return -EINVAL;
    }

    ret = memcpy_s(name, name_len, info.each.name, VMNG_VF_TEMP_NAME_LEN);
    if (ret != 0) {
        devdrv_drv_err("Copy name length failed.(devid=%u; ret=%d)\n", devid, ret);
        return -ENOMEM;
    }

    return 0;
}
#endif

int hal_kernel_dms_get_pg_info(unsigned int dev_id, HAL_DMS_PG_INFO_TYPE info_type,
    char* data, unsigned int size, unsigned int *ret_size)
{
#ifdef CFG_FEATURE_PG
    struct devdrv_info *dev_info = NULL;
    hal_dms_common_pg_info_t ret_pg_info = {0};

    if ((dev_id >= DEVDRV_MAX_DAVINCI_NUM) || (data == NULL) || (ret_size == NULL) || (info_type >= PG_INFO_TYPE_MAX)) {
        devdrv_drv_err("Input para is invalid. (dev_id=%u; data_is_null=%d; ret_size_is_null=%d; info_type=%d)\n",
            dev_id, (data == NULL), (ret_size == NULL), info_type);
        return -EINVAL;
    }

    dev_info = devdrv_manager_get_devdrv_info(dev_id);
    if (dev_info == NULL) {
        devdrv_drv_err("Get default dev_info failed. (dev_id=%u)\n", dev_id);
        return -ENODEV;
    }

    switch (info_type) {
        case PG_INFO_TYPE_AIC:
            ret_pg_info.num = dev_info->pg_info.comPgInfo.aicPara.minNum;
            ret_pg_info.freq = dev_info->pg_info.comPgInfo.aicPara.freq;
            ret_pg_info.bitmap = dev_info->pg_info.comPgInfo.aicPara.bitMap;
            *ret_size = sizeof(hal_dms_common_pg_info_t);
            break;
        case PG_INFO_TYPE_HBM:
            ret_pg_info.num = dev_info->pg_info.comPgInfo.hbmPara.minNum;
            ret_pg_info.freq = dev_info->pg_info.comPgInfo.hbmPara.freq;
            ret_pg_info.bitmap = dev_info->pg_info.comPgInfo.hbmPara.bitMap;
            *ret_size = sizeof(hal_dms_common_pg_info_t);
            break;
        case PG_INFO_TYPE_MATA:
            ret_pg_info.num = dev_info->pg_info.comPgInfo.mataPara.minNum;
            ret_pg_info.freq = dev_info->pg_info.comPgInfo.mataPara.freq;
            ret_pg_info.bitmap = dev_info->pg_info.comPgInfo.mataPara.bitMap;
            *ret_size = sizeof(hal_dms_common_pg_info_t);
            break;
        default:
            devdrv_drv_err("Input info type is not support. (dev_id=%u; info_type=%d)\n", dev_id, info_type);
            return -EOPNOTSUPP;
    }

    if (*ret_size > size) {
        devdrv_drv_err("Return size is larger than input data size. (dev_id=%u; size=%u; ret_size=%u)\n",
            dev_id, size, *ret_size);
        return -EINVAL;
    }

    *(hal_dms_common_pg_info_t *)data = ret_pg_info;
    return 0;
#else
    devdrv_drv_err("Not support to get pg info. (dev_id=%u)\n", dev_id);
    return -EOPNOTSUPP;
#endif
}

EXPORT_SYMBOL(hal_kernel_dms_get_pg_info);

STATIC u64 devdrv_get_dev_nominal_osc_freq(void)
{
    u64 freq = 0;

#if defined(__aarch64__)
    asm volatile("mrs %0, cntfrq_el0" : "=r" (freq));
#endif
    return freq;
}

#ifndef CFG_FEATURE_RC_MODE
STATIC void devdrv_manager_inform_ai_system_ready(struct work_struct *work)
{
    struct devdrv_manager_msg_info dev_manager_msg_info = {{0}, {0}};
    struct delayed_work *dwork = to_delayed_work(work);
    struct devdrv_device_info *drv_info = NULL;
    struct devdrv_platform_data *pdata = NULL;
    struct devdrv_info *dev_info = NULL;
    unsigned long timeout;
#ifdef CFG_SURPPORT_PCIE_HOST_DEVICE_COMM
    u32 out_len;
#endif
    int ret;

    dev_info = container_of(dwork, struct devdrv_info, fw_load_wq);
    devdrv_drv_info("Inform ai system ready delay start. (dev_id=%u)\n", dev_info->dev_id);
    if (dev_info->fw_wq_retry-- == 0) {
        devdrv_drv_info("Finish inform retry. (dev_id=%u)\n", dev_info->dev_id);
        return;
    }

    timeout = devdrv_manager_get_wait_time(dev_info);
#ifdef CFG_FEATURE_RC_MODE
    timeout = 0;
#endif
    ret = wait_event_interruptible_timeout(dev_manager_info->msg_chan_wait[dev_info->dev_id],
        dev_manager_info->msg_chan_rdy[dev_info->dev_id] == 1, timeout);
    if (ret <= 0) {
        devdrv_drv_warn("Msg channel unable to setup, there is no host. (dev_id=%u)\n", dev_info->dev_id);
        return;
    }
    pdata = dev_info->pdata;
    dev_manager_msg_info.header.msg_id = DEVDRV_MANAGER_CHAN_D2H_DEVICE_READY;
    dev_manager_msg_info.header.valid = (u16)DEVDRV_MANAGER_MSG_VALID;
    dev_manager_msg_info.header.result = (u16)DEVDRV_MANAGER_MSG_INVALID_RESULT;

    drv_info = (struct devdrv_device_info *)dev_manager_msg_info.payload;

#ifdef CFG_FEATURE_PG
    ret = strcpy_s(drv_info->soc_version, MAX_CHIP_NAME, (u8 *)dev_info->pg_info.spePgInfo.socVersion);
    if (ret) {
        devdrv_drv_err("Call strcpy_s failed. (dev_id=%u)\n", dev_info->dev_id);
        return;
    }
#endif

    drv_info->ai_core_num = dev_info->ai_core_num;
    drv_info->aicore_freq = dev_info->aicore_freq;
    drv_info->ai_core_num_level = pdata->ai_core_num_level;
    drv_info->ai_core_freq_level = pdata->ai_core_freq_level;
    drv_info->ai_cpu_core_num = dev_info->ai_cpu_core_num;
    drv_info->ctrl_cpu_core_num = dev_info->ctrl_cpu_core_num;
    drv_info->ctrl_cpu_occupy_bitmap = dev_info->ctrl_cpu_occupy_bitmap;
    drv_info->ctrl_cpu_endian_little = dev_info->ctrl_cpu_endian_little;
    drv_info->ctrl_cpu_id = dev_info->ctrl_cpu_id;
    drv_info->ctrl_cpu_ip = dev_info->ctrl_cpu_ip;
    drv_info->ts_cpu_core_num = pdata->ts_pdata[0].ts_cpu_core_num;
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
    drv_info->chip_name = dev_info->chip_name;
    drv_info->chip_version = dev_info->chip_version;
    drv_info->chip_info = dev_info->chip_info;
    drv_info->aicore_bitmap = dev_info->aicore_bitmap;

    drv_info->chip_id = dev_info->chip_id;
    drv_info->multi_chip = dev_info->multi_chip;
    drv_info->multi_die = dev_info->multi_die;
    drv_info->mainboard_id = dev_info->mainboard_id;
    drv_info->connect_type = dev_info->connect_type;
    drv_info->board_id = dev_info->board_id;
    drv_info->die_id = dev_info->die_id;

    devdrv_drv_debug("Initialize chip info. (dev_id=%u; chip_name=%u; chip_version=%u)\n",
        dev_info->dev_id, dev_info->chip_name, dev_info->chip_version);

    drv_info->vector_core_num = dev_info->vector_core_num;
    drv_info->vector_core_freq = dev_info->vector_core_freq;
    drv_info->ts_load_fail = pdata->ts_pdata[0].ts_load_fail;

    drv_info->dev_nominal_osc_freq = dev_info->dev_nominal_osc_freq;

#ifdef CFG_FEATURE_SRIOV
    if (!devdrv_manager_is_pf_device(dev_info->dev_id)) {
        devdrv_manager_get_template_name(dev_info->dev_id, drv_info->template_name, TEMPLATE_NAME_LEN);
    }
#endif
    devdrv_manager_update_capability_info(dev_info->dev_id, drv_info);
    devdrv_manager_get_reg_ddr_addr(dev_info->dev_id, drv_info);

    devdrv_manager_get_dump_ddr_addr(dev_info->dev_id, &drv_info->dump_ddr_dma_addr, &drv_info->dump_ddr_size);

#ifdef CFG_SURPPORT_PCIE_HOST_DEVICE_COMM
    ret = agentdrv_common_msg_send(dev_info->dev_id, &dev_manager_msg_info, sizeof(dev_manager_msg_info),
                                   sizeof(dev_manager_msg_info), &out_len, AGENTDRV_COMMON_MSG_DEVDRV_MANAGER);
    if (ret != 0) {
        devdrv_drv_info("Delay inform device ready abnormal, try again. (dev_id=%u; ret=%d; result=%d)\n",
            dev_info->dev_id, ret, dev_manager_msg_info.header.result);
        /* If failed to notify the host, it will retry every 2s on PF and every 250ms on VF. */
        if (devdrv_manager_is_pf_device(dev_info->dev_id)) {
            queue_delayed_work(devdrv_manager_inform_wq, &dev_info->fw_load_wq, msecs_to_jiffies(DEVMNG_FW_WQ_DELAY_TIME));
        } else {
            queue_delayed_work(devdrv_manager_inform_wq, &dev_info->fw_load_wq, msecs_to_jiffies(DEVMNG_FW_WQ_VF_DELAY_TIME));
        }
        return;
    }
#endif

    devdrv_manager_clear_capability_info(dev_info->dev_id);
    devdrv_drv_info("Delay inform device ready info success. (dev_id=%u; retry_time=%u)\n",
        dev_info->dev_id, dev_info->fw_wq_retry);

    return;
}
#endif

STATIC void devdrv_manager_send_device_info_to_host(struct devdrv_info *dev_info)
{
#ifndef CFG_FEATURE_RC_MODE
    INIT_DELAYED_WORK(&dev_info->fw_load_wq, devdrv_manager_inform_ai_system_ready);
    queue_delayed_work(devdrv_manager_inform_wq, &dev_info->fw_load_wq, 0);
#endif
}

#ifdef CFG_SURPPORT_PCIE_HOST_DEVICE_COMM
#define RETRY_TIMES 100
STATIC int devdrv_manager_get_tslog_dma_addr(u32 dev_id, u64 phy_addr, u32 mem_size,
    dma_addr_t *dma_addr, bool dynamic_alloc)
{
    int retry_times = RETRY_TIMES;
    struct device *dev = dev_manager_info->dma_dev[dev_id];

    /* retry 100 times until dma_dev (get from pcie) is ready */
    while (dev == NULL && retry_times > 0) {
        msleep(2000);
        retry_times--;
        devdrv_drv_warn("Dma device is not ready, retry 2s later. (device id=%u)\n", dev_id);
        dev = dev_manager_info->dma_dev[dev_id];
    }

    if (dev == NULL) {
        devdrv_drv_err("Dma device is not ready. (device id=%u)\n", dev_id);
        return -ENODEV;
    }

    if (dynamic_alloc == true) {
        *dma_addr = dma_map_page(dev, phys_to_page((phys_addr_t)phy_addr), 0, mem_size, DMA_BIDIRECTIONAL);
    } else {
        *dma_addr = dma_map_resource(dev, phy_addr, mem_size, DMA_BIDIRECTIONAL, 0);
    }

    if (dma_mapping_error(dev, *dma_addr)) {
        devdrv_drv_err("Ts log address dma map failed. (device_id=%u)\n", dev_id);
        return -ENOMEM;
    }

    return 0;
}
#endif

int devdrv_manager_send_tslog_addr_to_host(u32 devid, u64 phy_addr, u32 mem_size, bool dynamic_alloc)
{
#ifdef CFG_SURPPORT_PCIE_HOST_DEVICE_COMM
    int ret;
    u32 out_len = 0;
    u32 retry_times;
    unsigned long timeout;
    dma_addr_t dma_addr = 0;
    struct devdrv_ts_log *ts_log = NULL;
    struct devdrv_manager_msg_info dev_manager_msg_info = {{0}, {0}};

    if (dms_get_rc_ep_mode() == DMS_RC_MODE) {
        return 0;
    }

    ret = devdrv_manager_get_tslog_dma_addr(devid, phy_addr, mem_size, &dma_addr, dynamic_alloc);
    if (ret != 0) {
        devdrv_drv_warn("Dma map error. (device_id=%u; ret=%d)\n", devid, ret);
        return ret;
    }

    timeout = msecs_to_jiffies(300000);
    ret = wait_event_interruptible_timeout(dev_manager_info->msg_chan_wait[devid],
        dev_manager_info->msg_chan_rdy[devid] == 1, timeout);
    if (ret <= 0) {
        devdrv_drv_warn("Msg channel unable to setup, there is no host. (device id=%u)\n", devid);
        return ret;
    }

    dev_manager_msg_info.header.msg_id = DEVDRV_MANAGER_CHAN_D2H_SEND_TSLOG_ADDR;
    dev_manager_msg_info.header.valid = (u16)DEVDRV_MANAGER_MSG_VALID;
    dev_manager_msg_info.header.result = (u16)DEVDRV_MANAGER_MSG_INVALID_RESULT;

    ts_log = (struct devdrv_ts_log *)dev_manager_msg_info.payload;
    ts_log->devid = devid;
    ts_log->mem_size = mem_size;
    ts_log->dma_addr = dma_addr;

    retry_times = 100; /* The message is resent every second for 100 times. */
    do {
        ret = agentdrv_common_msg_send(devid, &dev_manager_msg_info, sizeof(dev_manager_msg_info),
            sizeof(dev_manager_msg_info), &out_len, AGENTDRV_COMMON_MSG_DEVDRV_MANAGER);
        if (ret != 0) {
            retry_times--;
        } else {
            break;
        }
        ssleep(2); /* Delay 2s */
    } while(retry_times);

    if (ret != 0) {
        devdrv_drv_err("D2H Send ts log addr to host failed. (device id=%u; ret=%d)\n", devid, ret);
    }

    return ret;
#else
    if (dms_get_rc_ep_mode() == DMS_RC_MODE) {
        return 0;
    }
    devdrv_drv_err("not support in current enveroment\n");
    return -ENOTSUPP;
#endif
}
EXPORT_SYMBOL(devdrv_manager_send_tslog_addr_to_host);

#ifndef CFG_SOC_PLATFORM_MINIV2
STATIC void devdrv_check_lpm3(void)
{
#if defined(CFG_SOC_PLATFORM_MINI)
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
        timestamp.tv_usec = stamp.tv_nsec / 1000; /* 1us is 1000ns */
#ifndef CFG_SOC_PLATFORM_MINIV2
        mntn_system_error(LPM3_STARTUP_EXCEPTION, timestamp, 0);
#endif
    }
    devm_iounmap(dev_manager_info->dev, status);
#endif /* CFG_SOC_PLATFORM_MINI */
}
#endif /* CFG_SOC_PLATFORM_MINIV2 */

STATIC int devdrv_manager_register_param_check(struct devdrv_info *dev_info)
{
    u8 flag_judge;

    if ((dev_manager_info == NULL) || (dev_info == NULL) || (dev_info->pdata == NULL)) {
        devdrv_drv_err("devdrv manager has not initialized. (manage_info=%d; dev_info=%d)\n", dev_manager_info != NULL,
            dev_info != NULL);
        return -EINVAL;
    }

    flag_judge =
        (((dev_info->dev_id >= DEVDRV_MAX_DAVINCI_NUM) || (dev_manager_info->dev_info[dev_info->dev_id] != NULL)));
    if (flag_judge) {
        devdrv_drv_err("Has been registered or invalid id. (dev_id=%u)\n", dev_info->dev_id);
        return -ENODEV;
    }

    return 0;
}

STATIC void devdrv_bbox_get_tsconfig(struct devdrv_info *dev_info)
{
#if ((!defined CFG_SOC_PLATFORM_MINIV2) && (!defined CFG_SOC_PLATFORM_CLOUD_V2))
    bbox_tsconfig *config = NULL;

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

        devdrv_drv_info("ts blackbox base: %llu, size: %llu.\n", dev_info->fw_info.ts_blackbox_base,
                        dev_info->fw_info.ts_blackbox_size);
        devdrv_drv_info("ts start log base: %llu, size: %llu.\n", dev_info->fw_info.ts_start_log_base,
                        dev_info->fw_info.ts_start_log_size);

        devdrv_drv_info("enable bbox: %d.\n", dev_info->fw_info.enable_bbox);
    }
#endif
}

#ifdef CFG_FEATURE_EP_MODE
STATIC int devmng_shm_init(struct devdrv_info *dev_info)
{
    size_t shm_size;
    u64 shm_addr;
    int i;
#ifndef CFG_FEATURE_DEVMNG_BAR
    int ret;

    ret = agentdrv_get_addr_info(dev_info->dev_id, DEVDRV_ADDR_DEVMNG_RESV_BASE,
                                 0, &shm_addr, &shm_size);
    if (ret) {
        devdrv_drv_err("[devid=%u] get shm addr fail, ret=%d.\n", dev_info->dev_id, ret);
        return ret;
    }
#else
    shm_addr = DEVDRV_ADDR_DEVMNG_OFFSET;
    shm_size = DEVDRV_ADDR_DEVMNG_SIZE;
#endif

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

    dev_info->shm_status->health_status = 0;
    for (i = 0; i < VMNG_VDEV_MAX_PER_PDEV; i++) {
        dev_info->shm_status->dms_health_status[i] = 0;
    }

    return 0;
}

STATIC void devmng_shm_uninit(struct devdrv_info *dev_info)
{
    if (dev_info->shm_vaddr != NULL)
        iounmap(dev_info->shm_vaddr);

    dev_info->shm_vaddr = NULL;
    dev_info->shm_head = NULL;
    dev_info->shm_soc = NULL;
    dev_info->shm_board = NULL;
    dev_info->shm_status = NULL;
    return;
}
#endif

#ifdef CFG_FEATURE_HEALTH_ERR_CODE
STATIC void devmng_refresh_error_code_work_init(struct devdrv_info *dev_info)
{
#if (!defined CFG_SOC_PLATFORM_MDC_V51)
    dev_info->heart_beat_wq = create_workqueue("heart_beat_work");
    if (dev_info->heart_beat_wq == NULL) {
        devdrv_drv_err("create_workqueue error. device(%u)\n", dev_info->dev_id);
        return;
    }

    INIT_WORK(&dev_info->work, devdrv_fresh_error_code_to_shm);
#endif

    return;
}

STATIC void devmng_heart_beat_work_uninit(struct devdrv_info *dev_info)
{
#if (!defined CFG_SOC_PLATFORM_MDC_V51)
    (void)cancel_work_sync(&dev_info->work);
    if (dev_info->heart_beat_wq != NULL) {
        destroy_workqueue(dev_info->heart_beat_wq);
        dev_info->heart_beat_wq = NULL;
    }
#endif

    return;
}
#endif

#ifdef CFG_FEATURE_HW_INFO_FROM_BIOS
#define CFG_HW_DATA_MEM_ADDR 0x36EFD800
STATIC int devdrv_manager_get_hw_info_from_bios(struct devdrv_info *dev_info)
{
    void __iomem *hw_info_vaddr = NULL;
    devdrv_base_hw_info_t *hw_data = NULL;

    hw_info_vaddr =
        ioremap(CFG_HW_DATA_MEM_ADDR, sizeof(devdrv_base_hw_info_t));
    if (hw_info_vaddr == NULL) {
        devdrv_drv_err("Remap hardware info fail. (dev_id=%u)\n", dev_info->dev_id);
        return -ENOMEM;
    }

    hw_data = (devdrv_base_hw_info_t __iomem *)hw_info_vaddr;
    dev_info->chip_id = hw_data->chip_id;
    dev_info->multi_chip = hw_data->multi_chip;
    dev_info->multi_die = hw_data->multi_die;
    dev_info->mainboard_id = hw_data->mainboard_id;
    dev_info->connect_type = hw_data->connect_type;
    dev_info->board_id = hw_data->board_id;
    iounmap(hw_info_vaddr);
    hw_info_vaddr = NULL;

    devdrv_drv_info("Get hardware info from share memory." \
        "(chip_id=%u; multi_chip=%u; multi_die=%u; mainboard_id=0x%x; connect_type=%u; board_id=0x%x)\n", \
            dev_info->chip_id, dev_info->multi_chip, dev_info->multi_die, dev_info->mainboard_id,
                dev_info->connect_type, dev_info->board_id);

    return 0;
}
#endif

STATIC int devdrv_manager_get_chip_info(struct devdrv_info *dev_info)
{
    int ret;
    u64 base_offset = 0;
    soc_chip_ver_reg_t chip_info = {0};

    ret = dms_get_dev_phy_base_addr(dev_info, &base_offset);
    if (ret != 0) {
        dms_err("Failed to get device base offset. (dev_id=%u; ret=%d)\n", dev_info->dev_id, ret);
        return -EINVAL;
    }

    ret = devdrv_reg_op(DEVDRV_REG_RD, (SOC_CHIP_INFO_REG_BASE + base_offset),
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

int devdrv_manager_get_hw_info(struct devdrv_info *dev_info)
{
    int ret;

#ifdef CFG_FEATURE_HW_INFO_FROM_BIOS
    ret = devdrv_manager_get_hw_info_from_bios(dev_info);
    if (ret != 0) {
        devdrv_drv_info("Get hardware data from bios failed. (ret=%d)\n", ret);
        return ret;
    }
#endif
    ret = devdrv_manager_get_chip_info(dev_info);
    if (ret != 0) {
        devdrv_drv_info("Get chip_info failed. (ret=%d)\n", ret);
        return ret;
    }

    return 0;
}
EXPORT_SYMBOL(devdrv_manager_get_hw_info);

STATIC void devdrv_manager_dev_num_increase(unsigned int dev_id)
{
    dev_manager_info->num_dev++;

    if (devdrv_manager_is_pf_device(dev_id) == true) {
        dev_manager_info->pf_num++;
    } else {
        dev_manager_info->vf_num++;
    }
}

STATIC void devdrv_manager_dev_num_decrease(unsigned int dev_id)
{
    dev_manager_info->num_dev--;

    if (devdrv_manager_is_pf_device(dev_id) == true) {
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

int devdrv_manager_register(struct devdrv_info *dev_info)
{
    struct devdrv_platform_data *pdata = NULL;
    unsigned long flags;
    u32 tsid = 0;
    u32 idx;
    int ret, i;

    DRV_PRINT_START();

    ret = dms_device_register(dev_info);
    if (ret) {
        devdrv_drv_err("Dms device register failed. (dev_id=%u)\n", dev_info->dev_id);
        return -ENODEV;
    }

#ifndef CFG_SOC_PLATFORM_MINIV2
    devdrv_check_lpm3();
#endif /* !CFG_SOC_PLATFORM_MINIV2 */
    ret = devdrv_manager_register_param_check(dev_info);
    if (ret) {
        goto dms_unregister;
    }

    pdata = dev_info->pdata;
#ifdef CFG_FEATURE_OLD_DEVID_TRANS
    ret = devdrv_manager_create_one_device(dev_info);
    if (ret) {
        devdrv_drv_err("Create one device failed. (dev_id=%u; ret=%d)\n", dev_info->dev_id, ret);
        goto dms_unregister;
    }
#endif

    dev_info->plat_type = (u8)DEVDRV_MANAGER_DEVICE_ENV;
    dev_info->dmp_started = false;
    dev_info->drv_ops = &devdrv_platform_drv_ops;

    for (i = 0; i < DEVDRV_MAX_COMPUTING_POWER_TYPE; i++) {
        dev_info->computing_power[i] = DEVDRV_COMPUTING_VALUE_INVALID;
    }

    spin_lock_irqsave(&dev_manager_info->spinlock, flags);
    dev_manager_info->dev_info[dev_info->dev_id] = dev_info;
    dev_manager_info->dev_id[dev_info->dev_id] = dev_info->dev_id;
    devdrv_manager_dev_num_increase(dev_info->dev_id);
    spin_unlock_irqrestore(&dev_manager_info->spinlock, flags);

    if (devdrv_manager_is_pf_device(dev_info->dev_id) == true) {
        dev_info->fw_wq_retry = DEVMNG_FW_WQ_RETRY_COUNT;
        /* bbox_get_tsconfig should be called befor fw_load_start */
        ret = devdrv_map_register_memory(dev_info->dev_id);
        if (ret != 0) {
            devdrv_drv_err("devdrv_map_register_memory failed. (dev_id=%u; ret=%d)\n", dev_info->dev_id, ret);
            goto map_register_memory_failed;
        }
        devdrv_bbox_get_tsconfig(dev_info);
        (void)ipc_mailbox_rx_register(dev_info->dev_id);
        devdrv_freq_control_register(dev_info->dev_id);
        (void)tsmng_mailbox_rx_register(dev_info->dev_id);

        devdrv_drv_debug("--- devid(%u), ts_num(%u) ---\n", dev_info->dev_id, pdata->ts_num);
        devdrv_drv_debug("ts aicore config: core id(%u), num(%u), bitmap(0x%llx) freq(%llu)\n",
            dev_info->ai_core_id, dev_info->ai_core_num, dev_info->aicore_bitmap, dev_info->aicore_freq);
        devdrv_drv_debug("ts vector_core_config: num(%u), bitmap(0x%llx) freq(%llu)\n",
            dev_info->vector_core_num, dev_info->vector_core_bitmap, dev_info->vector_core_freq);
        if (dev_info->drv_ops->tsdrv_firmware_load != NULL) {
            if (dev_info->drv_ops->tsdrv_firmware_load(dev_info)) {
                devdrv_drv_err("ts firmware load failed, devid(%u)\n", dev_info->dev_id);
            }
        }

    #ifdef CFG_FEATURE_TIMESYNC
        ret = dms_time_sync_info_init(dev_info->dev_id);
        if (ret != 0) {
            devdrv_drv_err("Time sync init fail. (dev_id=%u; ret=%d)\n", dev_info->dev_id, ret);
            goto time_sync_init_failed;
        }
    #endif

#ifndef CFG_FEATURE_PG
        (void)devdrv_manager_get_hw_info(dev_info);
#endif
        dev_info->dev_nominal_osc_freq = devdrv_get_dev_nominal_osc_freq();
    } else {
        dev_info->fw_wq_retry = DEVMNG_FW_WQ_VF_RETRY_COUNT;
    }

    /* send device info to host */
    devdrv_manager_send_device_info_to_host(dev_info);
#ifdef CFG_FEATURE_EP_MODE
    ret = devmng_shm_init(dev_info);
    if (ret) {
        devdrv_drv_err("[dev_id = %u]:shm init fail, ret = %d\n", dev_info->dev_id, ret);
        goto shm_init_failed;
    }
#endif

#ifdef CFG_FEATURE_HEALTH_ERR_CODE
    devmng_refresh_error_code_work_init(dev_info);
#endif
#ifdef CFG_FEATURE_CHIP_DIE
    ret = devdrv_manager_get_random_save_to_bar(dev_info->dev_id);
    if (ret) {
        devdrv_drv_err("devdrv_manager_get_random_save_to_bar failed. (dev_id=%u; ret=%d)\n",
            dev_info->dev_id, ret);
        goto random_save_to_bar_failed;
    }
#endif

    devdrv_drv_info("[dev_id = %u]: init finish\n", dev_info->dev_id);

    DRV_PRINT_END();
    return 0;

#ifdef CFG_FEATURE_CHIP_DIE
random_save_to_bar_failed:
#endif
#ifdef CFG_FEATURE_HEALTH_ERR_CODE
    devmng_heart_beat_work_uninit(dev_info);
#endif
#ifdef CFG_FEATURE_EP_MODE
    devmng_shm_uninit(dev_info);
shm_init_failed:
#endif

#ifdef CFG_FEATURE_TIMESYNC
    if (devdrv_manager_is_pf_device(dev_info->dev_id) == true) {
        dms_time_sync_info_free(dev_info->dev_id);
    }
time_sync_init_failed:
#endif

    if (devdrv_manager_is_pf_device(dev_info->dev_id) == true) {
        tsid = pdata->ts_num;
        for (idx = 0; idx < tsid; idx++) {
            devdrv_tscpu_free_memory(dev_info, dev_info->dev, DEVDRV_TS_MEMORY_SIZE,
                pdata->ts_pdata[idx].ts_load_addr, pdata->ts_pdata[idx].ts_dma_handle);
        }
        ipc_mailbox_rx_unregister(dev_info->dev_id);
        devdrv_unmap_register_memory(dev_info->dev_id);
    }

map_register_memory_failed:
    spin_lock_irqsave(&dev_manager_info->spinlock, flags);
    dev_manager_info->dev_info[dev_info->dev_id] = NULL;
    devdrv_manager_dev_num_decrease(dev_info->dev_id);
    dev_manager_info->msg_chan_rdy[dev_info->dev_id] = 1;
    spin_unlock_irqrestore(&dev_manager_info->spinlock, flags);
    if (waitqueue_active(&dev_manager_info->msg_chan_wait[dev_info->dev_id])) {
        wake_up_interruptible(&dev_manager_info->msg_chan_wait[dev_info->dev_id]);
    }

#ifndef CFG_FEATURE_RC_MODE
    flush_delayed_work(&dev_info->fw_load_wq);
    cancel_delayed_work(&dev_info->fw_load_wq);
#endif
    spin_lock_irqsave(&dev_manager_info->spinlock, flags);
    dev_manager_info->msg_chan_rdy[dev_info->dev_id] = 0;
    spin_unlock_irqrestore(&dev_manager_info->spinlock, flags);
#ifdef CFG_FEATURE_OLD_DEVID_TRANS
    devdrv_manager_release_one_device(dev_info);
#endif
dms_unregister:
    dms_device_unregister(dev_info);
    return ret;
}
EXPORT_SYMBOL(devdrv_manager_register);

void devdrv_manager_unregister(struct devdrv_info *dev_info)
{
    struct devdrv_platform_data *pdata = NULL;
    unsigned long flags;
    int ret;
    u32 tsid = 0;

    if ((dev_info == NULL) || (dev_manager_info == NULL) || (dev_info->dev_id >= DEVDRV_MAX_DAVINCI_NUM))
        return;

    if (dev_manager_info->dev_info[dev_info->dev_id] == NULL) {
        return;
    }

    devdrv_drv_info("devdrv_manager_unregister start. (dev_id=%u)\n", dev_info->dev_id);
    pdata = dev_info->pdata;
    if (pdata == NULL) {
        devdrv_drv_err("The pdata is NULL. (devid=%u)\n", dev_info->dev_id);
        return;
    }

    if (dev_info->status == DEVINFO_STATUS_SHUTDOWN) {
        if (devdrv_manager_shutdown(dev_info)) {
            devdrv_drv_err("dev manager shutdown error.\n");
            return;
        }
        devdrv_drv_info("dev manager shutdown finished.\n");

        /*
        * In reboot, the mailbox has exited. In this case,
        * the operation below isn't need to run.
        */
        return;
    }

    spin_lock_irqsave(&dev_manager_info->spinlock, flags);
    dev_manager_info->dev_info[dev_info->dev_id] = NULL;
    devdrv_manager_dev_num_decrease(dev_info->dev_id);
    dev_manager_info->msg_chan_rdy[dev_info->dev_id] = 1;
    spin_unlock_irqrestore(&dev_manager_info->spinlock, flags);
    if (waitqueue_active(&dev_manager_info->msg_chan_wait[dev_info->dev_id])) {
        wake_up_interruptible(&dev_manager_info->msg_chan_wait[dev_info->dev_id]);
    }

#ifndef CFG_FEATURE_RC_MODE
    flush_delayed_work(&dev_info->fw_load_wq);
    cancel_delayed_work(&dev_info->fw_load_wq);
#endif
    spin_lock_irqsave(&dev_manager_info->spinlock, flags);
    dev_manager_info->msg_chan_rdy[dev_info->dev_id] = 0;
    spin_unlock_irqrestore(&dev_manager_info->spinlock, flags);

#ifdef CFG_FEATURE_HEALTH_ERR_CODE
    devmng_heart_beat_work_uninit(dev_info);
#endif

#ifdef CFG_FEATURE_EP_MODE
    devmng_shm_uninit(dev_info);
#endif
    if (devdrv_manager_is_pf_device(dev_info->dev_id)) {
        ret = dfm_stub_print(dev_info->dev_id, DFM_MODULE_ID_DRIVER, UNINIT_SUCCESS);
        if (ret != 0) {
            devdrv_drv_err("dfm uninit stub print failed. (ret=%d)\n", ret);
        } else {
            devdrv_drv_info("dfm uninit stub print succ.\n");
        }

        for (tsid = 0; tsid < pdata->ts_num; tsid++) {
            devdrv_tscpu_free_memory(dev_info, dev_info->dev, DEVDRV_TS_MEMORY_SIZE, pdata->ts_pdata[tsid].ts_load_addr,
                                    pdata->ts_pdata[tsid].ts_dma_handle);
        }
        ipc_mailbox_rx_unregister(dev_info->dev_id);
        tsmng_mailbox_rx_unregister(dev_info->dev_id);
        devdrv_unmap_register_memory(dev_info->dev_id);
    }
#ifdef CFG_FEATURE_OLD_DEVID_TRANS
    devdrv_manager_release_one_device(dev_info);
#endif
    dms_device_unregister(dev_info);
    devdrv_drv_info("devdrv_manager_unregister success. (dev_id=%u)\n", dev_info->dev_id);
}
EXPORT_SYMBOL(devdrv_manager_unregister);

STATIC int devdrv_manager_get_devid_from_host(u32 devid, void *msg, u32 in_len, u32 *ack_len)
{
    struct devdrv_manager_msg_info *dev_manager_msg_info = (struct devdrv_manager_msg_info *)msg;
    unsigned long flags;
    struct devdrv_info *dev_info = NULL;

    if ((dev_manager_msg_info->header.valid != DEVDRV_MANAGER_MSG_VALID) ||
        (in_len < sizeof(struct devdrv_manager_msg_info))) {
        devdrv_drv_err("Invalid message from host. (dev_id=%u; valid=%u; in_len=%u)\n",
                       devid, dev_manager_msg_info->header.valid, in_len);
        return -EINVAL;
    }

    spin_lock_irqsave(&dev_manager_info->spinlock, flags);
    dev_manager_info->dev_id_flag[devid] = 1;
    dev_manager_info->dev_id[devid] = dev_manager_msg_info->header.dev_id;
    dev_info = dev_manager_info->dev_info[devid];
    spin_unlock_irqrestore(&dev_manager_info->spinlock, flags);

#ifdef CFG_SOC_PLATFORM_CLOUD
    /* slot id of pcie card in dev_info need to be updated from host side, the original slot id is not right */
    if ((dev_info != NULL) && soc_misc_is_pcie_card(dev_info->board_id)) {
        if (dev_info->pdata != NULL) {
            dev_info->pdata->platform_info.slot_id = dev_manager_msg_info->header.dev_id;
        }
        dev_info->slot_id = dev_manager_msg_info->header.dev_id;
    }
#endif

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

    h2d_msg->ts_status = 0;
    h2d_msg->syspcie_sysdma_status = 0;
    h2d_msg->aicore_bitmap = dev_info->inuse.ai_core_error_bitmap;
    h2d_msg->aicpu_heart_beat_exception = dev_info->inuse.ai_cpu_error_bitmap;

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
        devdrv_drv_err("Invalid message from host. (dev_id=%u; valid=%u; in_len=%u)\n",
                       devid, dev_manager_msg_info->header.valid, in_len);
        return -EINVAL;
    }

    para_info = (struct devdrv_ioctl_para_query_pid *)dev_manager_msg_info->payload;
    ret = devdrv_query_process_by_host_pid(para_info->host_pid, devid, para_info->cp_type,
        para_info->vfid, &(para_info->pid));
    if (ret != 0 && ret != DRV_ERROR_NO_PROCESS) {
        devdrv_drv_warn("query device pid failed, ret(%d), devid(%u)\n", ret, devid);
        return -EINVAL;
    }

    dev_manager_msg_info->header.result = ret;
    dev_manager_msg_info->header.valid = DEVDRV_MANAGER_MSG_VALID;
    *ack_len = sizeof(struct devdrv_ioctl_para_query_pid) + sizeof(struct devdrv_manager_msg_head);

    return 0;
}

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
#ifdef CFG_FEATURE_NOTIFY_REBOOT

#ifdef CFG_SOC_PLATFORM_MINIV2
#define HOST_REBOOT_FLAG_ADDR_BASE 0xc6f70074
#define SMP_REG_BASE_OFFSET        0x8000000000
#elif (defined CFG_SOC_PLATFORM_CLOUD)
#define HOST_REBOOT_FLAG_ADDR_BASE 0xa0d22838
#define SMP_REG_BASE_OFFSET        0x200000000000
#endif

#define REG_LEN 4
#define HOST_REBOOT_FLAG 0X5A

STATIC void write_host_reboot_flag(u32 devid)
{
    void __iomem *vaddr = NULL;

    vaddr = ioremap(HOST_REBOOT_FLAG_ADDR_BASE + (devid * SMP_REG_BASE_OFFSET), REG_LEN);
    if (vaddr == NULL) {
        devdrv_drv_err("ioremap fail.\n");
        return;
    }

    writel_relaxed(HOST_REBOOT_FLAG, vaddr);
    iounmap(vaddr);
    vaddr = NULL;
}

int devdrv_manager_notice_reboot(u32 devid, void *msg, u32 in_len, u32 *ack_len)
{
    struct devdrv_manager_msg_info *dev_manager_msg_info = NULL;

    dev_manager_msg_info = (struct devdrv_manager_msg_info *)msg;
    if ((dev_manager_msg_info->header.valid != DEVDRV_MANAGER_MSG_VALID) ||
        (in_len < sizeof(struct devdrv_manager_msg_info))) {
        devdrv_drv_err("Invalid message from host. (dev_id=%u; valid=%u; in_len=%u)\n",
                       devid, dev_manager_msg_info->header.valid, in_len);
        return -EINVAL;
    }
    write_host_reboot_flag(devid);
    dev_manager_msg_info->header.result = 0;
    dev_manager_msg_info->header.valid = DEVDRV_MANAGER_MSG_VALID;
    *ack_len = sizeof(struct devdrv_manager_msg_head);

    return 0;
}
#endif

int devdrv_manager_get_task_status(u32 devid, void *msg, u32 in_len, u32 *ack_len)
{
    struct devdrv_manager_msg_info *dev_manager_msg_info = NULL;
    u32 *is_cancelled = NULL;
    struct devdrv_info *dev_info = NULL;
    int dev_id;

    if ((msg == NULL) || (ack_len == NULL)) {
        devdrv_drv_err("invalid param, (msg == NULL)=%d, (ack_len == NULL)=%d.\n", (msg == NULL), (ack_len == NULL));
        return -EINVAL;
    }

    dev_manager_msg_info = (struct devdrv_manager_msg_info *)msg;
    if ((dev_manager_msg_info->header.valid != DEVDRV_MANAGER_MSG_H2D_MAGIC) ||
        (in_len < sizeof(struct devdrv_manager_msg_info))) {
        devdrv_drv_err("Invalid message from host. (dev_id=%u; valid=%u; in_len=%u)\n",
                       devid, dev_manager_msg_info->header.valid, in_len);
        return -EINVAL;
    }
    is_cancelled = (u32 *)dev_manager_msg_info->payload;
    *is_cancelled = 0UL;
    dev_id = dev_manager_msg_info->header.dev_id;
    dev_info = devdrv_manager_get_devdrv_info(devid);
    if ((dev_id != devdrv_manager_get_devid(devid)) || (dev_info == NULL)) {
        devdrv_drv_warn("invalid device id, dev_id = %d, "
                       "dev_manager_info->dev_id = %u, "
                       "dev_info is %s\n",
                       dev_id, devdrv_manager_get_devid(devid), (!dev_info) ? "NULL" : "not NULL");
        return -EAGAIN;
    }

    dev_manager_msg_info->header.result = 0;
    dev_manager_msg_info->header.valid = DEVDRV_MANAGER_MSG_D2H_MAGIC;
    *ack_len = sizeof(*dev_manager_msg_info);

    return 0;
}

STATIC int devdrv_get_dev_ddr_total(u32 devid, struct devdrv_manager_msg_resource_info *res_info)
{
    u64 total;
    u64 free_size;
    int ret;

    ret = mem_get_cgroup_and_cdm_memory(devid, res_info->vfid, &total, &free_size);
    if (ret != 0) {
        devdrv_drv_err("Get dev ddr info failed; (devid=%u; vfid=%u)\n", devid, res_info->vfid);
        return -EINVAL;
    }
    res_info->value = total;
    return ret;
}

STATIC int devdrv_get_dev_ddr_free(u32 devid, struct devdrv_manager_msg_resource_info *res_info)
{
    u64 total;
    u64 free_size;
    int ret;

    ret = mem_get_cgroup_and_cdm_memory(devid, res_info->vfid, &total, &free_size);
    if (ret != 0) {
        devdrv_drv_err("Get dev ddr info failed. (devid=%u; vfid=%u)\n", devid, res_info->vfid);
        return -EINVAL;
    }
    res_info->value = free_size;
    return ret;
}

STATIC int devdrv_get_dev_hbm_total(u32 devid, struct devdrv_manager_msg_resource_info *res_info)
{
    struct mem_info info = {0};
    int ret;

    ret = mem_get_hbm_info(devid, res_info->vfid, MEM_SERVICE, &info);
    if (ret != 0) {
        devdrv_drv_err("Get dev hbm info failed. (dev_id=%u; vfid=%u)\n", devid, res_info->vfid);
        return -EINVAL;
    }
    res_info->value = info.total_size;

    info.total_size = 0;
    ret = mem_get_dev_sliceable_memory(devid, &info);
    if (ret != 0) {
        devdrv_drv_err("Failed to obtain the separable memory of the HBM. (dev_id=%u; vfid=%u; ret=%d)\n",
            devid, res_info->vfid, ret);
        return -EINVAL;
    }
    res_info->value_ext = info.total_size;

    return ret;
}

STATIC int devdrv_get_dev_hbm_free(u32 devid, struct devdrv_manager_msg_resource_info *res_info)
{
    struct mem_info info = {0};
    int ret;

    ret = mem_get_hbm_info(devid, res_info->vfid, MEM_SERVICE, &info);
    if (ret) {
        devdrv_drv_err("Get dev hbm info failed. (dev_id=%u; vfid=%u)\n", devid, res_info->vfid);
        return -EINVAL;
    }
    res_info->value = info.free_size;
    return ret;
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
        devdrv_drv_err("The devmm_get_device_process_memory failed. (device_id=%u; vfid=%u; hostpid=%u)\n",
            devid, res_info->vfid, res_info->owner_id);
        return ret;
    }
    res_info->value = memory_size;
    return ret;
}

STATIC int (*devdrv_get_resource_info[DEVDRV_DEV_INFO_TYPE_MAX])(u32 devid,
    struct devdrv_manager_msg_resource_info *resource_info) = {
        [DEVDRV_DEV_DDR_TOTAL] = devdrv_get_dev_ddr_total,
        [DEVDRV_DEV_DDR_FREE] = devdrv_get_dev_ddr_free,
        [DEVDRV_DEV_HBM_TOTAL] = devdrv_get_dev_hbm_total,
        [DEVDRV_DEV_HBM_FREE] = devdrv_get_dev_hbm_free,
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
        devdrv_drv_err("Invalid resource type. (device_id=%u; resource_type=%u)\n",
                       devid, resource_info->info_type);
        return -EINVAL;
    }

    if (devdrv_get_resource_info[resource_info->info_type] == NULL) {
        devdrv_drv_err("Not support resource type. (device_id=%u; resource_type=%u)\n",
                       devid, resource_info->info_type);
        return -EOPNOTSUPP;
    }

    ret = devdrv_get_resource_info[resource_info->info_type](devid, resource_info);
    if (ret) {
        devdrv_drv_err("Get resource info failed. (device_id=%u; ret=%d)\n",
                       devid, ret);
        if (ret < 0) {
            ret = -ret;
        }

        dev_manager_msg_info->header.result = ret;
        return 0;
    }

    dev_manager_msg_info->header.result = 0;
    return 0;
}

STATIC int devdrv_manager_get_dev_resource_info(struct devdrv_resource_info *dinfo)
{
    struct devdrv_manager_msg_resource_info resource_info;
    u32 dev_id = 0;
    u32 vf_id = 0;
    int ret;

    if (dinfo->resource_type >= DEVDRV_DEV_INFO_TYPE_MAX) {
        devdrv_drv_err("Invalid resource type. (device_id=%u; resource_type=%u)\n",
                       dinfo->devid, dinfo->resource_type);
        return -EINVAL;
    }

    if (dinfo->resource_type != DEVDRV_DEV_DDR_TOTAL && dinfo->resource_type != DEVDRV_DEV_DDR_FREE) {
        devdrv_drv_err("Not support resource type. (device_id=%u; resource_type=%u)\n",
                       dinfo->devid, dinfo->resource_type);
        return -EOPNOTSUPP;
    }

    ret = devdrv_manager_trans_and_check_id(dinfo->devid, &dev_id, &vf_id, ALSO_DOES_SUPPORT_VF);
    if (ret != 0) {
        devdrv_drv_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n", dinfo->devid, ret);
        return ret;
    }

    resource_info.vfid = vf_id;
    resource_info.info_type = dinfo->resource_type;
    resource_info.owner_id = dinfo->owner_id;

    if (devdrv_get_resource_info[resource_info.info_type] == NULL) {
        devdrv_drv_err("Not support resource type. (dev_id=%u; resource_type=%u)\n",
                       dev_id, resource_info.info_type);
        return -EOPNOTSUPP;
    }

    ret = devdrv_get_resource_info[resource_info.info_type](dev_id, &resource_info);
    if (ret) {
        devdrv_drv_err("Get resource info failed. (dev_id=%u; ret=%d)\n",
                       dev_id, ret);
        return -EINVAL;
    }

    *((u64 *)dinfo->buf) = resource_info.value;

    return 0;
}

STATIC int (*const dmanage_get_resource_handler[DEVDRV_MAX_OWNER_TYPE])(struct devdrv_resource_info *dinfo) = {
    [DEVDRV_DEV_RESOURCE] = devdrv_manager_get_dev_resource_info,
    [DEVDRV_VDEV_RESOURCE] = NULL,
    [DEVDRV_PROCESS_RESOURCE] = NULL,
};

STATIC int devdrv_manager_ioctl_get_dev_resource_info(struct file *filep, unsigned int cmd, unsigned long arg)
{
#ifdef CFG_FEATURE_DEVICE_QUERY_RESOURCE
    struct devdrv_resource_info dinfo = {0};
    int ret;

    if (copy_from_user_safe(&dinfo, (void *)(uintptr_t)arg, sizeof(dinfo))) {
        return -EFAULT;
    }

    if (dinfo.owner_type >= DEVDRV_MAX_OWNER_TYPE) {
        devdrv_drv_err("Invalid parameter. (devid=%u; owner_type=%u)\n", dinfo.devid, dinfo.owner_type);
        return -EINVAL;
    }

    if (dmanage_get_resource_handler[dinfo.owner_type] == NULL) {
        devdrv_drv_err("Unsupported parameter. (devid=%u; owner_type=%u)\n", dinfo.devid, dinfo.owner_type);
        return -EOPNOTSUPP;
    }

    ret = dmanage_get_resource_handler[dinfo.owner_type](&dinfo);
    if (ret) {
        devdrv_drv_err("Failed to obtain device resource. (devid=%u; owner_type=%u; ret=%d)\n",
            dinfo.devid, dinfo.owner_type, ret);
        return ret;
    }

    if (copy_to_user_safe((void *)(uintptr_t)arg, &dinfo, sizeof(dinfo))) {
        devdrv_drv_err("copy to user failed.\n");
        return -EFAULT;
    }

    return 0;
#else
    return -EOPNOTSUPP;
#endif
}


STATIC int devdrv_manager_get_dmp_started(u32 devid, void *msg, u32 in_len, u32 *ack_len)
{
    struct devdrv_manager_msg_info *dev_manager_msg_info = NULL;
    struct devdrv_info *dev_info = NULL;
    u32 *dmp_started = NULL;

    dev_manager_msg_info = (struct devdrv_manager_msg_info *)msg;
    if ((dev_manager_msg_info->header.valid != DEVDRV_MANAGER_MSG_VALID) ||
        (in_len < sizeof(struct devdrv_manager_msg_info))) {
        devdrv_drv_err("Invalid message from host. (dev_id=%u; valid=%u; in_len=%u)\n",
                       devid, dev_manager_msg_info->header.valid, in_len);
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

#ifdef CFG_SURPPORT_PCIE_HOST_DEVICE_COMM
STATIC
#endif
int (*devdrv_manager_chan_msg_processes[])(u32 devid, void *msg, u32 in_len, u32 *ack_len) = {
    [DEVDRV_MANAGER_CHAN_H2D_SEND_DEVID] = devdrv_manager_get_devid_from_host,
    [DEVDRV_MANAGER_CHAN_H2D_RERESH_AICORE_INFO] = devdrv_manager_refresh_aicore_info,
    [DEVDRV_MANAGER_CHAN_H2D_SYNC_GET_DEVINFO] = devdrv_manager_get_devinfo_from_host,
    [DEVDRV_MANAGER_CHAN_H2D_SYNC_GET_CORE_UTILIZATION] = devdrv_manager_get_core_utilization_from_host,
    [DEVDRV_MANAGER_CHAN_H2D_GET_TASK_STATUS] = devdrv_manager_get_task_status,
    [DEVDRV_MANAGER_CHAN_H2D_SYNC_LOW_POWER] = devdrv_manager_h2d_sync_low_power,
#ifdef CFG_SOC_PLATFORM_MINIV2
    [DEVDRV_MANAGER_CHAN_H2D_GET_TS_GROUP_INFO] = NULL,
#endif
#ifdef CFG_FEATURE_TIMESYNC
    [DEVDRV_MANAGER_CHAN_H2D_WALL_TIME_SYNC] = dms_get_walltime_from_host,
    [DEVDRV_MANAGER_CHAN_H2D_LOCALTIME_SYNC] = dms_get_timezone_from_host,
#endif
    [DEVDRV_MANAGER_CHAN_H2D_GET_RESOURCE_INFO] = devdrv_manager_get_resource_info,
    [DEVDRV_MANAGER_CHAN_H2D_QUERY_DMP_STARTED] = devdrv_manager_get_dmp_started,
    [DEVDRV_MANAGER_CHAN_H2D_QUERY_DEVICE_PID] = devdrv_manager_get_device_pid,
#if (!defined (DEVMNG_UT)) && (!defined (DEVDRV_MANAGER_HOST_UT_TEST))
    [DEVDRV_MANAGER_CHAN_PID_MAP_SYNC] = devdrv_pid_map_sync_proc,
#endif
    [DEVDRV_MANAGER_CHAN_H2D_NOTICE_PROCESS_EXIT] = devdrv_manager_notice_device_exit,
    [DEVDRV_MANAGER_CHAN_H2D_DMS_EVENT_SUBSCRIBE] = dms_event_subscribe_from_host,
    [DEVDRV_MANAGER_CHAN_H2D_DMS_EVENT_CLEAN] = dms_event_clean_from_host,
    [DEVDRV_MANAGER_CHAN_H2D_DMS_EVENT_MASK] = dms_event_mask_from_host,
#ifdef CFG_FEATURE_NOTIFY_REBOOT
    [DEVDRV_MANAGER_CHAN_H2D_NOTICE_REBOOT] = devdrv_manager_notice_reboot,
#endif
    [DEVDRV_MANAGER_CHAN_MAX_ID] = NULL,
};

#ifdef CFG_SURPPORT_PCIE_HOST_DEVICE_COMM
int devdrv_manager_common_msg_process(u32 devid, void *data, u32 in_data_len, u32 out_data_len,
    u32 *real_out_len)
{
    u32 msg_id;
    u32 data_min_len = sizeof(struct devdrv_manager_msg_info);

    if ((devid >= DEVDRV_MAX_DAVINCI_NUM) || (data == NULL)) {
        devdrv_drv_err("invalid parameters data(%pK), devid(%u)\n", data, devid);
        return -EINVAL;
    }

    if (in_data_len < sizeof(struct devdrv_manager_msg_head)) {
        devdrv_drv_err("Invalid msg data head len. (devid=%u; len=%u)\n", devid, in_data_len);
        return -EINVAL;
    }

    msg_id = ((struct devdrv_manager_msg_head *)data)->msg_id;

    if (msg_id >= DEVDRV_MANAGER_CHAN_MAX_ID) {
        devdrv_drv_err("[dev_id = %u]:invalid parameters, msg_id(%u)\n", devid, msg_id);
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
        devdrv_drv_err("[dev_id = %u]:input invalid date length %pK\n", devid, real_out_len);
        return -EINVAL;
    }
    if (devdrv_manager_chan_msg_processes[msg_id] == NULL) {
        devdrv_drv_err("[dev_id = %u]:invalid parameters\n", devid);
        return -EINVAL;
    }
    if (devdrv_manager_chan_msg_processes[msg_id](devid, data, in_data_len, real_out_len))
        devdrv_drv_err("[dev_id = %u]:get device id from host failed\n", devid);

    return 0;
}
EXPORT_SYMBOL_UNRELEASE(devdrv_manager_common_msg_process);

STATIC void devdrv_manager_common_msg_notify(u32 dev_id)
{
    devdrv_drv_info("Common message notify. (dev_id=%u)\n", dev_id);
    if ((dev_id >= DEVDRV_MAX_DAVINCI_NUM) || (dev_manager_info->msg_chan_rdy[dev_id] == 1)) {
        devdrv_drv_err("common msg notify fail, dev_id = %u\n", dev_id);
        return;
    }
    dev_manager_info->msg_chan_rdy[dev_id] = 1;

    if (waitqueue_active(&dev_manager_info->msg_chan_wait[dev_id]))
        wake_up_interruptible(&dev_manager_info->msg_chan_wait[dev_id]);
}

STATIC int devdrv_manager_agent_init_instance(u32 dev_id, struct device *dev)
{
    if (dev_id < DEVDRV_MAX_DAVINCI_NUM) {
        dev_manager_info->dma_dev[dev_id] = dev;
    }

    return 0;
}

#define DEVMNG_DEVICE_NOTIFIER "mng_device"
static int devdrv_manager_device_notifier_func(u32 udevid, enum uda_notified_action action)
{
    struct device *dev = NULL;
    int ret = 0;

    dev = uda_get_agent_device(udevid);
    if (dev != NULL) { /* obp virtual dev is null, not need to init virtual dev */
        if (action == UDA_INIT) {
            ret = devdrv_manager_agent_init_instance(udevid, dev);
        }
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

#if (defined(CFG_SOC_PLATFORM_CLOUD) || defined(CFG_SOC_PLATFORM_MINI))
    if ((devid = agentdrv_get_msg_chan_devid(msg_chan)) < 0) {
        devdrv_drv_err("msg_chan to devid failed\n");
        return -EINVAL;
    }
#endif
    if (devid >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("devid invalid(%d).\r\n", devid);
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

    if ((data == NULL) || (real_out_len == NULL)) {
        devdrv_drv_err("invalid parameters data(%pK), real_out_len(%pK), in_data_len(%u)\n",
            data, real_out_len, in_data_len);
        return -EINVAL;
    }

    if (in_data_len < sizeof(struct devdrv_manager_msg_head)) {
        devdrv_drv_err("Invalid msg data head len. (len=%u)\n", in_data_len);
        return -EINVAL;
    }

    msg_id = ((struct devdrv_manager_msg_head *)data)->msg_id;
    if (msg_id >= DEVDRV_MANAGER_CHAN_MAX_ID) {
        devdrv_drv_err("invalid msg_id(%u)\n", msg_id);
        return -EINVAL;
    }

    if (devdrv_manager_chan_msg_processes[msg_id] == NULL) {
        devdrv_drv_err("invalid parameters\n");
        return -EINVAL;
    }

    if ((devid = agentdrv_get_msg_chan_devid(msg_chan)) < 0) {
        devdrv_drv_err("msg_chan to devid failed\n");
        return -EINVAL;
    }

    if (devid >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("devid invalid(%d).\r\n", devid);
        return -EINVAL;
    }
    if (devdrv_manager_chan_msg_processes[msg_id](devid, data, in_data_len, real_out_len))
        devdrv_drv_err("[dev_id = %d]:get device id from host failed\n", devid);

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
    ret = uda_real_virtual_notifier_register(DEVMNG_DEVICE_NOTIFIER, &type,
        UDA_PRI1, devdrv_manager_device_notifier_func);
    if (ret) {
        devdrv_drv_err("agentdrv register client failed.,ret = %d\n", ret);
        return;
    }

    ret = agentdrv_register_non_trans_msg_client(&devdrv_manager_agent_msg_client);
    if (ret) {
        devdrv_drv_err("agentdrv register client failed.,ret = %d\n", ret);
        goto agentdrv_register_non_trans_msg_client_failed;
    }

    ret = agentdrv_register_common_msg_client(&devdrv_manager_agent_common_msg_client);
    if (ret) {
        devdrv_drv_err("register agentdrv common msg chan failed.,ret = %d\n", ret);
        goto agentdrv_register_common_msg_client_failed;
    }
    devdrv_manager_msg_chan_initialized = 1;

    return;

agentdrv_register_common_msg_client_failed:
    agentdrv_unregister_non_trans_msg_client(&devdrv_manager_agent_msg_client);
agentdrv_register_non_trans_msg_client_failed:
    (void)uda_real_virtual_notifier_unregister(DEVMNG_DEVICE_NOTIFIER, &type);

    devdrv_manager_agent_common_msg_client.init_notify = NULL;
    devdrv_manager_agent_common_msg_client.common_msg_recv = NULL;
    devdrv_manager_agent_common_msg_client.type = AGENTDRV_COMMON_MSG_PCIVNIC;
}
#endif

void devdrv_lpm_exception_info(void *data, unsigned long len)
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

#ifdef CFG_SOC_PLATFORM_CLOUD
    bbox_system_error(0, exception_info->exception_code, &timestamp, 0);
#else
#ifndef CFG_SOC_PLATFORM_MINIV2
    mntn_system_error(exception_info->exception_code, timestamp, 0);
#endif
#endif
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

#if (defined CFG_FEATURE_VFIO) && (defined CFG_FEATURE_RC_MODE)
#define DEV_VF_NODE_MODE 0660
STATIC char *dev_vf_node(struct device *d, umode_t *mode)
{
    if (mode != NULL) {
        *mode = DEV_VF_NODE_MODE;
    }
    return NULL;
}
#endif

STATIC int devdrv_manager_create_one_device(struct devdrv_info *dev_info)
{
    struct device *i_device = NULL;
    dev_t devno = MKDEV(devdrv_major, dev_info->dev_id);
    int ret;

    dev_info->cce_ops.devdrv_cdev.owner = THIS_MODULE;
    cdev_init(&dev_info->cce_ops.devdrv_cdev, &devdrv_davinci_fops);
    ret = cdev_add(&dev_info->cce_ops.devdrv_cdev, devno, DEVDRV_MAX_DAVINCI_NUM);
    if (ret) {
        devdrv_drv_err("[dev_id = %u]:cdev_add error.,ret = %d\n", dev_info->dev_id, ret);
        return ret;
    }

#if (defined CFG_FEATURE_VFIO) && (defined CFG_FEATURE_RC_MODE)
    if (VDAVINCI_IS_VDEV(dev_info->dev_id)) {
        devdrv_class->devnode = dev_vf_node;
    }
#endif
    i_device = device_create(devdrv_class, NULL, devno, NULL, "davinci%d", dev_info->dev_id);
    if (IS_ERR(i_device)) {
        cdev_del(&dev_info->cce_ops.devdrv_cdev);
        devdrv_drv_err("[dev_id = %u]:device_create error.\n", dev_info->dev_id);
        return -ENODEV;
    }

    dev_info->cce_ops.cce_dev = i_device;
    ret = devdrv_manager_container_table_devlist_add_ns(&dev_info->dev_id, 1, init_task.nsproxy->mnt_ns);
    if (ret) {
        device_destroy(devdrv_class, devno);
        cdev_del(&dev_info->cce_ops.devdrv_cdev);
        devdrv_drv_err("add to list error.%u,ret = %d\n", dev_info->dev_id, ret);
        return ret;
    }

    return 0;
}

STATIC void devdrv_manager_release_one_device(struct devdrv_info *dev_info)
{
    dev_t devno;

    (void)devdrv_manager_container_table_devlist_del_ns(&dev_info->dev_id, 1, init_task.nsproxy->mnt_ns);
    devno = MKDEV(devdrv_major, dev_info->dev_id);

    if (dev_info->cce_ops.cce_dev != NULL) {
        device_destroy(devdrv_class, devno);
        cdev_del(&dev_info->cce_ops.devdrv_cdev);
        dev_info->cce_ops.cce_dev = NULL;
    }
}
#endif

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



#ifdef CFG_SOC_PLATFORM_MDC_V51
STATIC int devdrv_manager_probe(struct platform_device *pdev)
{
    int ret;

    /* pinctrl set gpio as default io */
    devdrv_gpio_iomux_init(&pdev->dev);

    ret = devdrv_sub_os_init(pdev);
    if (ret != 0) {
        devdrv_drv_err("devdrv_sub_os_init err.\n");
        return ret;
    }

    return 0;
}

STATIC int devdrv_manager_remove(struct platform_device *pdev)
{
    devdrv_sub_os_uninit();
    return 0;
}

STATIC const struct of_device_id devdrv_manager_of_match[] = {
    { .compatible = "hisi,devdrv_manager" },
    {}
};
MODULE_DEVICE_TABLE(of, devdrv_manager_of_match);

STATIC struct platform_driver devdrv_manager_platform_driver = {
    .probe = devdrv_manager_probe,
    .remove = devdrv_manager_remove,
    .driver = {
        .name = "devdrv_manager",
        .owner = THIS_MODULE,
        .of_match_table = devdrv_manager_of_match,
    },
};
#endif
#ifdef CFG_FEATURE_REBOOT_REASON
void devdrv_manager_record_reset_reason(unsigned int addr_offset, unsigned int reason)
{
    void __iomem *vaddr = NULL;
    unsigned int reset_reason;

    if (g_l3sram_base_vaddr == NULL) {
        devdrv_drv_err("ioremap reset_reason(0x%x) addr fail.\n", reason);
        return;
    }
    vaddr = (void __iomem *)(g_l3sram_base_vaddr + addr_offset);

    reset_reason = readl_relaxed(vaddr);
    reset_reason |= reason;
    writel_relaxed(reset_reason, vaddr);

    devdrv_drv_info("record reset_reason(0x%x).\n", reset_reason);
    return;
}

STATIC int devdrv_manager_reboot_handle(struct notifier_block *self, unsigned long event, void *data)
{
    devdrv_manager_record_reset_reason(RST_REASON_CUR_L3SRAM_OFFSET1, RST_REASON_BIT_REBOOT_CMD);
    devdrv_drv_info("System reboot now.....\n");
    return NOTIFY_OK;
}

STATIC int devdrv_manager_panic_handle(struct notifier_block *self, unsigned long event, void *data)
{
    devdrv_manager_record_reset_reason(RST_REASON_CUR_L3SRAM_OFFSET1, RST_REASON_BIT_PANIC);
    devdrv_drv_info("Os panic now.....\n");
    return NOTIFY_OK;
}

static struct notifier_block devdrv_manager_reboot_notifier = {
    .notifier_call = devdrv_manager_reboot_handle,
};

STATIC struct notifier_block devdrv_manager_panic_notifier = {
    .notifier_call = devdrv_manager_panic_handle,
};
#endif

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

#ifdef CFG_SOC_PLATFORM_CLOUD
    tsmng_ghes_ts_err_register();
#endif

#ifdef CFG_FEATURE_REBOOT_REASON
    ret = register_reboot_notifier(&devdrv_manager_reboot_notifier);
    if (ret) {
        devdrv_drv_err("register_reboot_notifier failed.\n");
        (void)unregister_reboot_notifier(&devdrv_manager_reboot_notifier);
        return ret;
    }
#ifndef UT_VCAST
    (void)atomic_notifier_chain_register(&panic_notifier_list, &devdrv_manager_panic_notifier);
#endif
#endif
    return 0;
}

STATIC void devdrv_manager_unregister_notifier(void)
{
#ifdef CFG_SOC_PLATFORM_CLOUD
    tsmng_ghes_ts_err_unregister();
#endif

#ifdef CFG_FEATURE_REBOOT_REASON
    (void)unregister_reboot_notifier(&devdrv_manager_reboot_notifier);
#ifndef UT_VCAST
    (void)atomic_notifier_chain_unregister(&panic_notifier_list, &devdrv_manager_panic_notifier);
#endif
#endif
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
#ifndef DEVMNG_UT
    hash_init(dev_manager_info->proc_hash_table);
#endif
    mutex_init(&dev_manager_info->devdrv_sign_list_lock);
    INIT_LIST_HEAD(&dev_manager_info->hostpid_list_header);

    spin_lock_init(&dev_manager_info->msg_pm_list_lock);
    INIT_LIST_HEAD(&dev_manager_info->msg_pm_list_header);

    devdrv_manager_dev_num_reset();
    for (i = 0; i < DEVDRV_MAX_DAVINCI_NUM; i++) {
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
#ifdef CFG_FEATURE_RC_MODE
    dev_manager_info->machine_mode = DEVDRV_PCIE_RC_MODE;
#else
    dev_manager_info->machine_mode = DEVDRV_PCIE_EP_MODE;
#endif
    /* dev manager on device side */
    dev_manager_info->plat_info = DEVDRV_MANAGER_DEVICE_ENV;
    return 0;
}

STATIC void devdrv_manager_info_free(void)
{
    kfree(dev_manager_info);
    dev_manager_info = NULL;
}

STATIC void devdrv_manager_l3sram_addr_init(void)
{
#ifdef CFG_FEATURE_REBOOT_REASON
    g_l3sram_base_vaddr = ioremap(DMS_RESET_REASON_L3SRAM_ADDR_BASE, DMS_RESET_REASON_SIZE);
#endif
    return;
}

STATIC void devdrv_manager_l3sram_addr_uninit(void)
{
#ifdef CFG_FEATURE_REBOOT_REASON
    if (g_l3sram_base_vaddr != NULL) {
        iounmap(g_l3sram_base_vaddr);
        g_l3sram_base_vaddr = NULL;
    }
#endif
    return;
}

int devdrv_manager_init(void)
{
    int ret;

    DRV_PRINT_START();
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

#ifdef CFG_SURPPORT_PCIE_HOST_DEVICE_COMM
    devdrv_manager_common_msg_chan_init();
#endif

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
        devdrv_drv_err("devdrv_manager_container_table_init failed.,ret = %d\n", ret);
        ret = -ENOMEM;
        goto container_table_init_failed;
    }
#if (defined CFG_FEATURE_VFIO) && (defined CFG_FEATURE_RC_MODE)
    ret = dev_mnt_vdevice_init();
    if (ret != 0) {
        devdrv_drv_err("Failed to invoke dev_mnt_vdevice_init. (ret=%d)\n", ret);
        goto container_table_init_failed;
    }
#endif
    tsdrv_status_init();
#ifndef CFG_SOC_PLATFORM_CLOUD_V2
    devdrv_manager_set_flow_control_reg();
#endif

#ifdef CFG_FEATURE_HEALTH_ERR_CODE
    devdrv_refresh_error_code_init();
#endif

#ifdef CFG_SOC_PLATFORM_MDC_V51
    ret = platform_driver_register(&devdrv_manager_platform_driver);
    if (ret != 0) {
        devdrv_drv_err("devdrv_manager_platform_driver register fail, ret(%d)\n", ret);
        goto platform_driver_register_failed;
    }
#endif
    devdrv_manager_l3sram_addr_init();

    ret = devdrv_manager_register_notifier();
    if (ret != 0) {
        devdrv_drv_err("Failed to register notifier. (ret=%d)\n", ret);
        goto register_notifier_fail;
    }

    init_rwsem(&devdrv_ops_sem);
#ifdef CFG_FEATURE_CHIP_DIE
    devdrv_manager_chip_dev_map_init();
#endif

#if (defined CFG_FEATURE_SRIOV) || (defined CFG_FEATURE_VF_USE_DEVID)
    devdrv_manager_vmngd_register();
#endif
    DRV_PRINT_END();
    return ret;
register_notifier_fail:
#ifdef CFG_SOC_PLATFORM_MDC_V11
    devdrv_manager_l3sram_addr_uninit();
#endif
#ifdef CFG_SOC_PLATFORM_MDC_V51
    platform_driver_unregister(&devdrv_manager_platform_driver);
    devdrv_manager_l3sram_addr_uninit();
platform_driver_register_failed:
#endif
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
    struct devdrv_msg_pm *msg_pm = NULL;

#if (defined CFG_FEATURE_SRIOV) || (defined CFG_FEATURE_VF_USE_DEVID)
    devdrv_manager_vmngd_unregister();
#endif
#ifdef CFG_FEATURE_CHIP_DIE
    devdrv_manager_chip_dev_map_exit();
#endif
#ifdef CFG_SOC_PLATFORM_MDC_V51
    platform_driver_unregister(&devdrv_manager_platform_driver);
#endif

#ifdef CONFIG_SYSFS
    sysfs_remove_group(&dev_manager_info->dev->kobj, &devdrv_manager_attr_group);
#endif /* CONFIG_SYSFS */

#ifndef CFG_SOC_PLATFORM_MINIV2
    devdrv_lc_gpioirq_unregister();
#endif /* CFG_SOC_PLATFORM_MINIV2 */

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
    devdrv_manager_free_hashtable();
#ifdef CFG_FEATURE_HEALTH_ERR_CODE
    devdrv_refresh_error_code_exit();
#endif
    destroy_workqueue(dev_manager_info->heart_beat_wq);
    destroy_workqueue(devdrv_manager_inform_wq);
#ifdef CFG_SURPPORT_PCIE_HOST_DEVICE_COMM
    if (devdrv_manager_msg_chan_initialized == 1) {
        struct uda_dev_type type;
        uda_davinci_local_real_agent_type_pack(&type);
        (void)uda_real_virtual_notifier_unregister(DEVMNG_DEVICE_NOTIFIER, &type);
        agentdrv_unregister_common_msg_client(&devdrv_manager_agent_common_msg_client);
        devdrv_manager_msg_chan_initialized = 0;
        agentdrv_unregister_non_trans_msg_client(&devdrv_manager_agent_msg_client);
        devdrv_manager_agent_common_msg_client.common_msg_recv = NULL;
    }
#endif
    devdrv_manager_msg_notify_initialized = 0;

    devdrv_manager_container_table_exit(dev_manager_info);
#if (defined CFG_FEATURE_VFIO) && (defined CFG_FEATURE_RC_MODE)
    dev_mnt_vdevice_uninit();
#endif
    kfree(dev_manager_info);
    dev_manager_info = NULL;

    devdrv_black_box_exit();
    appmon_black_box_exit();
#ifdef CFG_FEATURE_OLD_DEVID_TRANS
    devdrv_manager_release_driver_device();
#endif
    devdrv_manager_unregister_notifier();
    devdrv_manager_l3sram_addr_uninit();
    if (drv_ascend_unregister_sub_module(DAVINCI_INTF_MODULE_DEVMNG)) {
        devdrv_drv_err("drv_ascend_unregister_sub_module failed!\n");
    }
}

#if (defined CFG_FEATURE_SRIOV) || (defined CFG_FEATURE_VF_USE_DEVID)
STATIC struct devdrv_info *g_devdrv_vf_info_array[DEVDRV_MAX_DAVINCI_NUM] = { NULL };
static DEFINE_SPINLOCK(g_devdrv_vf_spinlock);

STATIC struct devdrv_info *devdrv_get_devdrv_vf_info_array(u32 dev_id)
{
    struct devdrv_info *dev_info = NULL;

    if (dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("Incorrect device ID. (dev_id=%u)\n", dev_id);
        return NULL;
    }

    spin_lock(&g_devdrv_vf_spinlock);
    dev_info = g_devdrv_vf_info_array[dev_id];
    spin_unlock(&g_devdrv_vf_spinlock);

    return dev_info;
}

STATIC void devdrv_set_devdrv_vf_info_array(u32 dev_id, struct devdrv_info *dev_info)
{
    if (dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("Incorrect device ID. (dev_id=%u)\n", dev_id);
        return;
    }
    spin_lock(&g_devdrv_vf_spinlock);
    g_devdrv_vf_info_array[dev_id] = dev_info;
    spin_unlock(&g_devdrv_vf_spinlock);
    return;
}

int devdrv_manager_vdev_init_instance(struct vmngd_client_instance *instance)
{
    int ret;
    int vfid = 0;
    int pf_dev_id = 0;
    int vf_dev_id;
    unsigned long bitmap_tmp = 0;
    struct devdrv_info *dev_info_vf = NULL;
    struct devdrv_info *dev_info_pf = NULL;

    if (instance == NULL) {
        devdrv_drv_err("Input instance is null.\n");
        return -EINVAL;
    }

    vf_dev_id = instance->vdev_ctrl.dev_id;
    ret = vmngd_get_pfvf_id_by_devid(vf_dev_id, &pf_dev_id, &vfid);
    if (ret != 0) {
        devdrv_drv_err("Get pf device id failed. (vf_dev_id=%u)\n", vf_dev_id);
        return ret;
    }

    dev_info_pf = devdrv_manager_get_devdrv_info(pf_dev_id);
    if (dev_info_pf == NULL) {
        devdrv_drv_err("Device info structure is null. (vf_dev_id=%u; pf_dev_id=%u)\n", vf_dev_id, pf_dev_id);
        return -EINVAL;
    }

    dev_info_vf = devdrv_get_devdrv_vf_info_array(vf_dev_id);
    if (dev_info_vf == NULL) {
        dev_info_vf = kzalloc(sizeof(struct devdrv_info), GFP_KERNEL | __GFP_ACCOUNT);
        if (dev_info_vf == NULL) {
            devdrv_drv_err("Allocate memory for dev_info failed.\n");
            return -ENOMEM;
        }
        /* It only needs to be initialized once. */
        mutex_init(&dev_info_vf->lock);
        sema_init(&dev_info_vf->sem, 0);
        devdrv_set_devdrv_vf_info_array(vf_dev_id, dev_info_vf);
    } else {
        devdrv_drv_info("Repeat init instance. (vf_dev_id=%d)\n", vf_dev_id);
    }

    *dev_info_vf = *dev_info_pf;
    dev_info_vf->dev_id = vf_dev_id;
    bitmap_tmp = instance->vdev_ctrl.vf_cfg.accelerator.aic_bitmap;
    dev_info_vf->ai_core_num = bitmap_weight(&bitmap_tmp, BITS_PER_LONG_LONG);
    bitmap_tmp = instance->vdev_ctrl.vf_cfg.accelerator.aiv_bitmap;
    dev_info_vf->vector_core_num = bitmap_weight(&bitmap_tmp, BITS_PER_LONG_LONG);
    bitmap_tmp = instance->vdev_ctrl.vf_cfg.cpu.device_aicpu_bitmap;
    dev_info_vf->ai_cpu_core_num = bitmap_weight(&bitmap_tmp, BITS_PER_LONG_LONG);
    dev_info_vf->aicore_bitmap = instance->vdev_ctrl.vf_cfg.accelerator.aic_bitmap;
    dev_info_vf->aicpu_occupy_bitmap = instance->vdev_ctrl.vf_cfg.cpu.device_aicpu_bitmap;

    dev_info_vf->ai_cpu_core_id = 1;
    dev_info_vf->inuse.ai_cpu_num = dev_info_vf->ai_cpu_core_num;
    dev_info_vf->inuse.ai_core_num = dev_info_vf->ai_core_num;

    devdrv_drv_info("Device online. (dev_id=%u; aicore_num=%u; aicore_bitmap=0x%llx; aicpu_num=%u; aicpu_bitmap=0x%x"
                    "ctrl_cpu_num=%u; ctrl_cpu_bitmap=0x%x; ctrl_cpu_endian_little=%u; ctrl_cpu_id=0x%x)\n",
        dev_info_vf->dev_id, dev_info_vf->ai_core_num, dev_info_vf->aicore_bitmap,
        dev_info_vf->ai_cpu_core_num, dev_info_vf->aicpu_occupy_bitmap,
        dev_info_vf->ctrl_cpu_core_num, dev_info_vf->ctrl_cpu_occupy_bitmap,
        dev_info_vf->ctrl_cpu_endian_little, dev_info_vf->ctrl_cpu_id);
    ret = devdrv_manager_register(dev_info_vf);
    if (ret != 0) {
        devdrv_drv_err("Device manager vdevice initialize failed. (dev_id=%u)\n", dev_info_vf->dev_id);
        return ret;
    }

    return 0;
}

int devdrv_manager_vdev_uninit_instance(struct vmngd_client_instance *instance)
{
    unsigned int dev_id;
    struct devdrv_info *dev_info = NULL;

    if (instance == NULL) {
        devdrv_drv_err("Input instance is null.\n");
        return -EINVAL;
    }

    dev_id = instance->vdev_ctrl.dev_id;
    dev_info = devdrv_manager_get_devdrv_info(dev_id);
    if (dev_info == NULL) {
        devdrv_drv_err("Device info is null.\n");
        return -EINVAL;
    }

    devdrv_manager_unregister(dev_info);
    return 0;
}

int devdrv_manager_vdev_reset_instance(struct vmngd_client_instance *instance)
{
    return 0;
}

static int devdrv_manager_set_dev_info(u32 dev_id, u32 aicpu_num)
{
    struct devdrv_info *dev_info = NULL;
    u32 ai_cpu_core_id;
    u32 aicpu_occupy_bitmap;

    /* aicpu_num set to 7 when SRIOV enable*/
    if (aicpu_num == 7) {
        ai_cpu_core_id = 1;
        aicpu_occupy_bitmap = 0xFE;
    } else {
        /* when SRIOV disable, aicpu_num set to 6, aicpu ID begin from 2*/
        ai_cpu_core_id = 2;
        aicpu_occupy_bitmap = 0xFC;
    }

    dev_info = devdrv_manager_get_devdrv_info(dev_id);
    if (dev_info == NULL) {
        devdrv_drv_err("dev_info is NULL\n");
        return -EINVAL;
    }

    dev_info->ai_cpu_core_num = aicpu_num;
    dev_info->ai_cpu_core_id = ai_cpu_core_id;
    dev_info->aicpu_occupy_bitmap = aicpu_occupy_bitmap;
    dev_info->inuse.ai_cpu_num = aicpu_num;

    return 0;

}

#ifdef CFG_SURPPORT_PCIE_HOST_DEVICE_COMM
static int set_host_aicpu_num_result_check(struct devdrv_manager_msg_info* dev_manager_msg_info, int out_len)
{
    if (out_len != (sizeof(u32) + sizeof(struct devdrv_manager_msg_head))) {
        devdrv_drv_err("receive response len %d is not equal = %ld.\n", out_len,
            (sizeof(u32) + sizeof(struct devdrv_manager_msg_head)));
        return -EINVAL;
    }

    if (dev_manager_msg_info->header.result != 0) {
        devdrv_drv_err("Failed. (ret=%u).\n", dev_manager_msg_info->header.result);
        return dev_manager_msg_info->header.result;
    }

    return 0;
}
#endif

static int devdrv_manager_sync_cpu_info_to_host(struct vmng_sriov_info *sriov_info)
{
#ifdef CFG_SURPPORT_PCIE_HOST_DEVICE_COMM
    struct devdrv_manager_msg_info dev_manager_msg_info = {{0}, {0}};
    u32 out_len = 0;
    u32 aicpu_num;
    int ret;

    if (sriov_info->sriov_status == VMNGH_PF_SRIOV_ENABLE) {
        aicpu_num = 7; /* aicpu_num set to 7 when SRIOV enable*/
    } else {
        aicpu_num = 6; /* aicpu_num set to 7 when SRIOV enable*/
    }

    dev_manager_msg_info.header.msg_id = DEVDRV_MANAGER_CHAN_D2H_SET_HOST_AICPU_NUM;
    dev_manager_msg_info.header.valid = (u16)DEVDRV_MANAGER_MSG_D2H_MAGIC;
    dev_manager_msg_info.header.result = (u16)DEVDRV_MANAGER_MSG_INVALID_RESULT;

    *(u32 *)dev_manager_msg_info.payload = aicpu_num;
    ret = agentdrv_common_msg_send(sriov_info->dev_id, &dev_manager_msg_info, sizeof(dev_manager_msg_info),
                                   sizeof(dev_manager_msg_info), &out_len, AGENTDRV_COMMON_MSG_DEVDRV_MANAGER);
    if (ret != 0) {
        devdrv_drv_err("Send msg to host failed. (dev_id=%u; aicpu_num=%u; ret=%d)\n",
                    sriov_info->dev_id, aicpu_num, ret);
        return ret;
    }

    ret = set_host_aicpu_num_result_check(&dev_manager_msg_info, out_len);
    if (ret != 0) {
        devdrv_drv_err("Check result failed.\n");
        return ret;
    }
#endif
    return 0;
}

static int sriov_info_para_check(struct vmng_sriov_info *sriov_info)
{
    if (sriov_info == NULL) {
        devdrv_drv_err("Sriov info is NULL.\n");
        return -EINVAL;
    }

    if (sriov_info->sriov_status >= VMNGH_PF_STATUS_MAX) {
        devdrv_drv_err("Invalid sriov_status. (sriov_status=%u)\n", sriov_info->sriov_status);
        return -EINVAL;
    }

    if (sriov_info->dev_id >= DEVDRV_PF_DEV_MAX_NUM) {
        devdrv_drv_err("Invalid device id. (dev_id=%u)\n", sriov_info->dev_id);
        return -EINVAL;
    }

    return 0;
}

static int devdrv_set_dev_info(struct vmng_sriov_info *sriov_info)
{
    int ret;
    u32 aicpu_num;

    if (sriov_info->sriov_status == VMNGH_PF_SRIOV_ENABLE) {
        /* aicpu_num set to 7 when SRIOV enable */
        aicpu_num = 7;
    } else {
        /* aicpu_num set to 6 when SRIOV disable */
        aicpu_num = 6;
    }

    ret = devdrv_manager_set_dev_info(sriov_info->dev_id, aicpu_num);
    if (ret != 0) {
        devdrv_drv_err("Set aicpu num failed. (ret=%d; dev_id=%u; aicpu_num=%u)\n", ret, sriov_info->dev_id, aicpu_num);
        return ret;
    }

    return 0;
}

static int devdrv_set_soc_misc_info(struct vmng_sriov_info *sriov_info)
{
    int ret;

    ret = dms_notifyer_call(DMS_DEVICE_SET_AICPU_NUM, sriov_info);
    if (ret != 0) {
        devdrv_drv_err("Notify call failed. (ret=%d; dev_id=%u)\n", ret, sriov_info->dev_id);
        return ret;
    }

    return 0;
}

struct sub_sriov_module_ops {
    int (*init) (struct vmng_sriov_info *sriov_info);
    int (*uninit)(struct vmng_sriov_info *sriov_info);
};

static struct sub_sriov_module_ops g_sub_ops_table[] = {
    {devdrv_set_dev_info, devdrv_set_dev_info},
    {devdrv_set_soc_misc_info, devdrv_set_soc_misc_info},
    {devdrv_manager_sync_cpu_info_to_host, devdrv_manager_sync_cpu_info_to_host},
};

int devdrv_manager_vdev_sriov_instance(struct vmng_sriov_info *sriov_info)
{
    int ret, ret_tmp;
    int i;
    struct vmng_sriov_info p_sriov_info;
    int table_size = sizeof(g_sub_ops_table) / sizeof(struct sub_module_ops);

    ret = sriov_info_para_check(sriov_info);
    if (ret != 0) {
        devdrv_drv_err("Invalid Para.\n");
        return -EINVAL;
    }

    p_sriov_info.dev_id = sriov_info->dev_id;
    p_sriov_info.sriov_status = sriov_info->sriov_status;

    for (i = 0; i < table_size; i++) {
        ret = g_sub_ops_table[i].init(&p_sriov_info);
        if  (ret != 0) {
            devdrv_drv_err("Set Sriov status failed. (ret=%d, index=%d)\n", ret, i);
            goto FAILED;
        }
    }

    devdrv_drv_event("Set aicpu number success. (dev_id=%u; sriov_status=%u)\n",
                     sriov_info->dev_id, sriov_info->sriov_status);
    return 0;

FAILED:
    /* When roll back, set the sriov_status from enable to disable, or from disable to enable */
    p_sriov_info.sriov_status = VMNGH_PF_SRIOV_ENABLE - p_sriov_info.sriov_status;
    for (; i >= 0; i--) {
        ret_tmp = g_sub_ops_table[i].uninit(&p_sriov_info);
        if (ret_tmp != 0) {
            devdrv_drv_err("Roll back failed. (ret=%d; ret_tmp=%d; index=%d.)\n", ret, ret_tmp, i);
            return ret_tmp;
        }
    }
    return ret;
}

struct vmngd_client g_devmng_vmngd_client = {
    .type = VMNGD_CLIENT_TYPE_DEVMNG,
    .init_instance = devdrv_manager_vdev_init_instance,
    .uninit_instance = devdrv_manager_vdev_uninit_instance,
    .reset_instance = devdrv_manager_vdev_reset_instance,
    .sriov_instance = devdrv_manager_vdev_sriov_instance,
};

int devdrv_manager_vmngd_register(void)
{
    int ret;

    ret = vmngd_register_client(&g_devmng_vmngd_client);
    if (ret != 0) {
        devdrv_drv_err("vmngd_register_client failed. (ret=%d)\n", ret);
        return ret;
    }

    return ret;
}

int devdrv_manager_vmngd_unregister(void)
{
    u32 i;
    struct devdrv_info *dev_info = NULL;

    for (i = 0; i < DEVDRV_MAX_DAVINCI_NUM; i++) {
        dev_info = devdrv_get_devdrv_vf_info_array(i);
        if (dev_info == NULL) {
            continue;
        }
        devdrv_set_devdrv_vf_info_array(i, NULL);
        mutex_destroy(&dev_info->lock);
        kfree(dev_info);
        dev_info = NULL;
    }

    vmngd_unregister_client(&g_devmng_vmngd_client);
    return 0;
}
#endif
