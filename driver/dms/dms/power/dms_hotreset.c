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


#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/kfifo.h>
#include <linux/delay.h>
#include <linux/time.h>
#include "devdrv_manager_comm.h"
#include "devdrv_interface.h"
#ifndef AOS_LLVM_BUILD
#include "devdrv_manager.h"
#include "virtmng_interface.h"
#endif
#include "dms_kernel_version_adapt.h"
#include "dms_time.h"
#include "dms_define.h"
#include "urd_feature.h"
#include "dms_template.h"
#include "urd_acc_ctrl.h"
#include "dms_cmd_def.h"
#include "dms_notifier.h"
#include "kernel_version_adapt.h"
#include "dms_hotreset.h"

STATIC struct hotreset_task_info *g_device_task_info[DEVDRV_MAX_DAVINCI_NUM] = {0};

BEGIN_DMS_MODULE_DECLARATION(DMS_MODULE_BASIC_POWER_INFO)
BEGIN_FEATURE_COMMAND()
#ifndef AOS_LLVM_BUILD
ADD_FEATURE_COMMAND(DMS_MODULE_BASIC_POWER_INFO,
    DMS_MAIN_CMD_BASIC,
    DMS_POWER_PRE_HOTRESET,
    NULL,
    NULL,
    DMS_SUPPORT_ROOT_PHY | DMS_PHYSICAL_ONLY,
    dms_power_pre_hotreset)
ADD_FEATURE_COMMAND(DMS_MODULE_BASIC_POWER_INFO,
    DMS_MAIN_CMD_BASIC,
    DMS_SUBCMD_POWER_PCIE_PRE_RESET,
    NULL,
    NULL,
    DMS_SUPPORT_ROOT_PHY | DMS_PHYSICAL_ONLY,
    dms_power_pcie_pre_reset)
ADD_FEATURE_COMMAND(DMS_MODULE_BASIC_POWER_INFO,
    DMS_MAIN_CMD_BASIC,
    DMS_SUBCMD_POWER_PCIE_HOT_RESET,
    NULL,
    NULL,
    DMS_SUPPORT_ROOT_PHY | DMS_PHYSICAL_ONLY,
    dms_power_pcie_hot_reset)
#endif
END_FEATURE_COMMAND()
END_MODULE_DECLARATION()

STATIC struct hotreset_task_info *dms_get_device_task_info(unsigned int dev_id)
{
    struct hotreset_task_info *device_task_info = NULL;

    if (dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        dms_err("Wrong device id. (dev_id=%u)\n", dev_id);
        return NULL;
    }
    device_task_info = g_device_task_info[dev_id];

    return device_task_info;
}

STATIC void dms_set_device_task_info(unsigned int dev_id, struct hotreset_task_info *device_task_info)
{
    g_device_task_info[dev_id] = device_task_info;
}

STATIC struct hotreset_task_info *dms_task_info_alloc(unsigned int dev_id)
{
    struct hotreset_task_info *device_task_info = NULL;
    device_task_info = kzalloc(sizeof(struct hotreset_task_info), GFP_KERNEL | __GFP_ACCOUNT);
    if (device_task_info == NULL) {
        dms_err("Kzalloc failed. (dev_id=%u)\n", dev_id);
        return NULL;
    }
    init_rwsem(&device_task_info->task_rw_sema);

    return device_task_info;
}

STATIC unsigned long get_dms_task_info_cnt(unsigned int dev_id)
{
    return g_device_task_info[dev_id]->task_ref_cnt;
}

STATIC void set_hotreset_task_flag(unsigned int dev_id)
{
    set_bit(HOTRESET_TASK_FLAG_BIT, &g_device_task_info[dev_id]->task_flag);
}

STATIC void clear_hotreset_task_flag(unsigned int dev_id)
{
    clear_bit(HOTRESET_TASK_FLAG_BIT, &g_device_task_info[dev_id]->task_flag);
}

STATIC int get_hotreset_task_flag(unsigned int dev_id)
{
    return test_bit(HOTRESET_TASK_FLAG_BIT, &g_device_task_info[dev_id]->task_flag);
}

STATIC int dms_power_hotreset_notifier_handle(struct notifier_block *self, unsigned long event, void *data)
{
    struct devdrv_info *dev_info = NULL;

    dev_info = (struct devdrv_info *)data;
    if ((data == NULL) || (event <= DMS_DEVICE_NOTIFIER_MIN) ||
        (event >= DMS_DEVICE_NOTIFIER_MAX)) {
        dms_err("Invalid parameter. (event=0x%lx; data=\"%s\")\n",
            event, data == NULL ? "NULL" : "OK");
        return NOTIFY_BAD;
    }

    dms_debug("Notifier hotreset handle success. (event=0x%lx; dev_id=%u)\n", event, dev_info->dev_id);
    return NOTIFY_DONE;
}

static struct notifier_block dms_power_hotreset_notifier = {
    .notifier_call = dms_power_hotreset_notifier_handle,
};

int dms_power_check_phy_mach(unsigned int dev_id)
{
    unsigned int host_flag;
    int ret;

    ret = devdrv_get_host_phy_mach_flag(dev_id, &host_flag);
    if (ret != 0) {
        dms_err("Devdrv_get_host_phy_mach_flag return value error. (devid=%u; ret=0x%x)\n", dev_id, ret);
        return ret;
    }

    if (host_flag != DEVDRV_HOST_PHY_MACH_FLAG) {
        dms_err("Device not phy mach. (devid=%u; host_flag=0x%x)\n", dev_id, host_flag);
        return -EPERM;
    }

#if defined(CFG_HOST_ENV) && defined(CFG_FEATURE_VFIO)
    if (vmng_get_device_split_mode(dev_id) != VMNG_NORMAL_NONE_SPLIT_MODE) {
        dms_err("Vdevice is not allowed! (dev_id=%u)\n", dev_id);
        return -EPERM;
    }
#endif

    return 0;
}

STATIC int dms_power_check_all_phy_mach(void)
{
    unsigned int dev_num;
    unsigned int phys_id = 0;
    unsigned int vfid = 0;
    int i, ret;

    dev_num = devdrv_manager_get_devnum();
    if (dev_num > DEVDRV_MAX_DAVINCI_NUM) {
        dms_err("Get invalid device num. (dev_num=%u)\n", dev_num);
        return -EINVAL;
    }
    for (i = 0; i < dev_num; ++i) {
        ret = devdrv_manager_container_logical_id_to_physical_id(i, &phys_id, &vfid);
        if (ret) {
            dms_err("Can't transfor virt id. (virt_id=%d; ret=%d)\n", i, ret);
            return ret;
        }
        ret = dms_power_check_phy_mach(phys_id);
        if (ret) {
            return ret;
        }
    }

    return 0;
}

#ifndef DEVDRV_MANAGER_HOST_UT_TEST
static void devdrv_uda_all_dev_ctrl_hotreset_cancel(u32 max_dev)
{
    u32 i;

    for (i = 0; i < max_dev; i++) {
        if (uda_is_udevid_exist(i)) {
            (void)uda_dev_ctrl(i, UDA_CTRL_HOTRESET_CANCEL);
        }
    }
}
#endif

static int devdrv_uda_all_dev_ctrl_hotreset(u32 max_dev)
{
#ifndef DEVDRV_MANAGER_HOST_UT_TEST
    int ret;
    u32 i;

    for (i = 0; i < max_dev; i++) {
        if (!uda_is_udevid_exist(i)) {
            continue;
        }

        ret = uda_dev_ctrl(i, UDA_CTRL_HOTRESET);
        if (ret != 0) {
            devdrv_uda_all_dev_ctrl_hotreset_cancel(i);
            devdrv_drv_err("Uda ctrl hotreset failed. (devid=%u; ret=%d)\n", i, ret);
            return ret;
        }
    }
#endif
    return 0;
}
STATIC int devdrv_get_brother_udevid(u32 udevid, u32 *bro_udevid)
{
    int ret;
    u32 i;
    struct devdrv_pcie_id_info id_info = {0};
    struct devdrv_pcie_id_info tmp_id_info = {0};

    ret = devdrv_get_pcie_id_info(udevid, &id_info);
    if (ret != 0) {
        dms_warn("get pcie id info failed. (udevid=%u; ret=%d)\n", udevid, ret);
        return ret;
    }

    for (i = 0; i < DEVDRV_PF_DEV_MAX_NUM; i++) {
        if (i == udevid) {
            continue;
        }
        if (!uda_is_udevid_exist(i)) {
            continue;
        }

        ret = devdrv_get_pcie_id_info(i, &tmp_id_info);
        if (ret != 0) {
            dms_warn("get pcie id info failed. (udevid=%u; ret=%d)\n", i, ret);
            continue;
        }

        if (tmp_id_info.bus == id_info.bus && tmp_id_info.device == id_info.device &&
            tmp_id_info.fn == id_info.fn && tmp_id_info.domain == id_info.domain) {
            devdrv_drv_info("get smp brother devid. (ori_devid=%u; bro_devid=%u; dbdf=%x.%x.%x.%x)\n",
                udevid, i, (unsigned int)id_info.domain, id_info.bus, id_info.device, id_info.fn);
            *bro_udevid = i;
            return 0;
        }
    }
    return -ESRCH;
}

int devdrv_uda_one_dev_ctrl_hotreset(u32 udevid)
{
    int ret;
    u32 bro_udevid;

    ret = uda_dev_ctrl(udevid, UDA_CTRL_HOTRESET);
    if (ret != 0) {
        devdrv_drv_err("Uda ctrl hotreset failed. (devid=%u; ret=%d)\n", udevid, ret);
        return ret;
    }

    ret = devdrv_get_brother_udevid(udevid, &bro_udevid);
    if (ret != 0) {
        return 0;
    }
    ret = uda_dev_ctrl(bro_udevid, UDA_CTRL_HOTRESET);
    if (ret != 0) {
        (void)uda_dev_ctrl(udevid, UDA_CTRL_HOTRESET_CANCEL);
        devdrv_drv_err("Uda ctrl hotreset failed. (bro_udevid=%u; ret=%d)\n", bro_udevid, ret);
        return ret;
    }

    return 0;
}
void devdrv_uda_one_dev_ctrl_hotreset_cancel(u32 udevid)
{
    int ret;
    u32 bro_udevid;

    (void)uda_dev_ctrl(udevid, UDA_CTRL_HOTRESET_CANCEL);

    ret = devdrv_get_brother_udevid(udevid, &bro_udevid);
    if (ret != 0) {
        return;
    }
    (void)uda_dev_ctrl(bro_udevid, UDA_CTRL_HOTRESET_CANCEL);
}

STATIC int dms_power_set_all_hot_reset(void)
{
    int ret;

#ifdef CFG_FEATURE_TIMESYNC
    int i;
#endif
    ret = dms_power_check_all_phy_mach();
    if (ret) {
        dms_err("Permission deny or devid is invalid; (ret=%d)\n", ret);
        return ret;
    }

#ifdef CFG_FEATURE_TIMESYNC
    for (i = 0; i < DEVDRV_MAX_DAVINCI_NUM; i++) {
        set_time_need_update(i);
    }
#endif

    ret = dms_notify_all_device_pre_hotreset();
    if (ret != 0) {
        dms_err("Notify all device hotreset failed. (ret=%d)\n", ret);
        return ret;
    }

    ret = devdrv_uda_all_dev_ctrl_hotreset(DEVDRV_MAX_DAVINCI_NUM);
    if (ret != 0) {
#ifndef DEVDRV_MANAGER_HOST_UT_TEST
        dms_notify_all_devices_cancel_hotreset(DEVDRV_MAX_DAVINCI_NUM);
        dms_err("Notify all uda device hotreset failed. (ret=%d)\n", ret);
        return ret;
#endif
    }

    ret = devdrv_hot_reset_device(ALL_DEVICE_HOTRESET_FLAG);
    if (ret != 0) {
        dms_err("Hotreset all devices failed. (ret=%d)\n", ret);
#ifndef DEVDRV_MANAGER_HOST_UT_TEST
        devdrv_uda_all_dev_ctrl_hotreset_cancel(DEVDRV_MAX_DAVINCI_NUM);
#endif
        dms_notify_all_devices_cancel_hotreset(DEVDRV_MAX_DAVINCI_NUM);
    }

    return ret;
}

STATIC int dms_power_set_single_hot_reset(unsigned int virt_id)
{
    int ret = 0;
    unsigned int phy_id;
    unsigned int vfid;
    /* the single device hot reset */
    ret = devdrv_manager_container_logical_id_to_physical_id(virt_id, &phy_id, &vfid);
    if (ret) {
        dms_err("Trans logical_id to physical_id failed. (devid=%u)\n", virt_id);
        return ret;
    }

    ret = dms_power_check_phy_mach(phy_id);
    if (ret) {
        dms_err("Permission deny or devid is invalid. (dev_id=%u; ret = %d\n", virt_id, ret);
        return ret;
    }

#ifdef CFG_FEATURE_TIMESYNC
    set_time_need_update(phy_id);
#endif
    ret = dms_notify_single_device_pre_hotreset(phy_id);
    if (ret != 0) {
        dms_err("Notify single device hotreset failed. (dev_id=%u; ret=%d)\n", phy_id, ret);
        return ret;
    }

#ifndef DEVDRV_MANAGER_HOST_UT_TEST
    ret = devdrv_uda_one_dev_ctrl_hotreset(phy_id);
    if (ret != 0) {
        dms_notify_single_device_cancel_hotreset(phy_id);
        devdrv_drv_err("Uda ctrl hotreset failed. (devid=%u; ret=%d)\n", phy_id, ret);
        return ret;
    }
#endif

    ret = devdrv_hot_reset_device(phy_id);
    if (ret != 0) {
        dms_err("Hotreset single device failed. (dev_id=%u; ret=%d)\n", phy_id, ret);
#ifndef DEVDRV_MANAGER_HOST_UT_TEST
        devdrv_uda_one_dev_ctrl_hotreset_cancel(phy_id);
#endif
        dms_notify_single_device_cancel_hotreset(phy_id);
    }

    return ret;
}

int dms_power_pcie_pre_reset(void *feature, char *in, unsigned int in_len, char *out, unsigned int out_len)
{
    unsigned int virt_id;
    int ret;
    unsigned int phys_id = 0;
    unsigned int vfid = 0;

    if ((in == NULL) || (in_len != sizeof(unsigned int))) {
        dms_err("Input arg is NULL or in_len is wrong. (in_len=%u)\n", in_len);
        return -EINVAL;
    }
    virt_id = *(unsigned int *)(uintptr_t)in;

    ret = devdrv_manager_container_logical_id_to_physical_id(virt_id, &phys_id, &vfid);
    if (ret != 0) {
        dms_err("Failed to transfer dev_id. (dev_id=%u)\n", virt_id);
        return -EFAULT;
    }

    if ((vfid != 0) || (!devdrv_manager_is_pf_device(phys_id))) {
        dms_err("Not support for vdevice. (phys_id=%u; vfid=%u)\n", phys_id, vfid);
        return -EOPNOTSUPP;
    }

    ret = dms_power_check_phy_mach(phys_id);
    if (ret) {
        dms_err("Permission deny or devid is invalid. (dev_id=%u; ret=%d)\n", phys_id, ret);
        return -EINVAL;
    }

    ret = dms_notify_single_device_pre_hotreset(phys_id);
    if (ret != 0) {
        dms_err("Notify single device hotreset failed. (dev_id=%u; ret=%d)\n", phys_id, ret);
        return ret;
    }

#ifndef DEVDRV_MANAGER_HOST_UT_TEST
    ret = devdrv_uda_one_dev_ctrl_hotreset(phys_id);
    if (ret != 0) {
        dms_notify_single_device_cancel_hotreset(phys_id);
        dms_err("Call uda_dev_ctrl failed, (phys_id=%u; ret=%d).\n", phys_id, ret);
        return ret;
    }
#endif

    ret = devdrv_pcie_prereset(phys_id);
    if (ret != 0) {
        dms_err("Hotreset failed. (dev_id=%u; ret=%d)\n", phys_id, ret);
#ifndef DEVDRV_MANAGER_HOST_UT_TEST
        devdrv_uda_one_dev_ctrl_hotreset_cancel(phys_id);
#endif
        dms_notify_single_device_cancel_hotreset(phys_id);
    }

    return ret;
}

int dms_power_pcie_hot_reset(void *feature, char *in, unsigned int in_len, char *out, unsigned int out_len)
{
    unsigned int virt_id;
    int ret = 0;

    if ((in == NULL) || (in_len != sizeof(unsigned int))) {
        dms_err("Input arg is NULL or in_len is wrong. (in_len=%u)\n", in_len);
        return -EINVAL;
    }
    virt_id = *(unsigned int *)(uintptr_t)in;

    /* hot reset all devices */
    if (virt_id == DEVDRV_RESET_ALL_DEVICE_ID) {
        ret = dms_power_set_all_hot_reset();
        if (ret) {
            dms_err("Permission deny or devid is invalid. (dev_id=%d; ret=%d)\n", virt_id, ret);
        }
        return ret;
    }

    /* the single device hot reset */
    ret = dms_power_set_single_hot_reset(virt_id);
    if (ret != 0) {
        dms_err("Set single device hotreset failed. (dev_id=%u; ret=%d)\n", virt_id, ret);
    }

    return ret;
}

void dms_notify_all_devices_cancel_hotreset(unsigned int dev_count)
{
    int ret;
    struct devdrv_info *dev = NULL;
    unsigned int i;

    for (i = 0; i < dev_count; i++) {
        dev = devdrv_manager_get_devdrv_info(i);
        if ((dms_get_device_task_info(i) == NULL) ||  (dev == NULL)) {
            continue;
        }

        clear_hotreset_task_flag(i);
        ret = dms_notifyer_call(DMS_DEVICE_HOTRESET_CANCEL, dev);
        if (ret != 0) {
            dms_err("Notify call hotreset cancel failed. (ret=%d; dev_id=%u)\n", ret, i);
            return;
        }
    }
}

void dms_notify_single_device_cancel_hotreset(unsigned int dev_id)
{
    int ret;
    struct devdrv_info *dev = NULL;

    dev = devdrv_manager_get_devdrv_info(dev_id);
    if ((dms_get_device_task_info(dev_id) == NULL) || (dev == NULL)) {
        dms_err("Device info is NULL. (dev_id=%u)\n", dev_id);
        return;
    }
    clear_hotreset_task_flag(dev_id);
    ret = dms_notifyer_call(DMS_DEVICE_HOTRESET_CANCEL, dev);
    if (ret != 0) {
        dms_err("Notify call hotreset cancel failed. (ret=%d; dev_id=%u)\n", ret, dev_id);
    }
}

int dms_hotreset_task_cnt_increase(unsigned int dev_id)
{
    int flag;

    if (dms_get_device_task_info(dev_id) == NULL) {
        dms_err("Device not exist. (dev_id=%u)\n", dev_id);
        return -ENODEV;
    }

    down_write(&g_device_task_info[dev_id]->task_rw_sema);
    flag = get_hotreset_task_flag(dev_id);
    if (flag != 0) {
        dms_err("Hotreset is running. (dev_id=%u; flag=%d)\n", dev_id, flag);
        up_write(&g_device_task_info[dev_id]->task_rw_sema);
        return -EBUSY;
    }
    g_device_task_info[dev_id]->task_ref_cnt++;
    up_write(&g_device_task_info[dev_id]->task_rw_sema);

    return 0;
}

void dms_hotreset_task_cnt_decrease(unsigned int dev_id)
{
    if (dms_get_device_task_info(dev_id) == NULL) {
        dms_err("Device not exist. (dev_id=%u)\n", dev_id);
        return;
    }

    down_write(&g_device_task_info[dev_id]->task_rw_sema);
    g_device_task_info[dev_id]->task_ref_cnt--;
    up_write(&g_device_task_info[dev_id]->task_rw_sema);
}

int dms_power_pre_hotreset(void *feature, char *in, unsigned int in_len, char *out, unsigned int out_len)
{
    int ret = 0;

    if ((in == NULL) || (in_len != sizeof(unsigned int))) {
        dms_err("Input arg is NULL or in_len is wrong. (in_len=%u)\n", in_len);
        return -EINVAL;
    }

    ret = dms_notify_all_device_pre_hotreset();
    if (ret != 0) {
        dms_err("Upgradtool set all device hotreset flag failed. (ret=%d)\n", ret);
    }

    return ret;
}

int dms_notify_all_device_pre_hotreset(void)
{
    unsigned int i = 0;
    int ret = 0;

    for (i = 0; i < DEVDRV_MAX_DAVINCI_NUM; i++) {
        if (dms_get_device_task_info(i) == NULL) {
            continue;
        }
        ret = dms_notify_device_hotreset(i);
        if (ret != 0) {
            dms_err("Notify all device hotreset failed. (ret=%d; dev_id=%u)\n", ret, i);
            if (ret != -EAGAIN) {
                dms_notify_all_devices_cancel_hotreset(i + 1);
            }
            dms_notify_all_devices_cancel_hotreset(i);
            return ret;
        }
    }

    dms_event("Pre-hotreset set all devices flag success.\n");
    return ret;
}

int dms_notify_single_device_pre_hotreset(unsigned int dev_id)
{
    int ret = 0;

    ret = dms_notify_device_hotreset(dev_id);
    if (ret != 0) {
        dms_err("Notify hotreset failed. (ret=%d)\n", ret);
        return ret;
    }

    dms_event("Pre-hotreset set device flag success. (dev_id=%u)\n", dev_id);
    return ret;
}

int dms_notify_device_hotreset(unsigned int dev_id)
{
    int ret, flag;
    unsigned long task_cnt;
    struct devdrv_info *dev = NULL;

    if (dms_get_device_task_info(dev_id) == NULL) {
        dms_err("Device not exsit. (dev_id=%u).\n", dev_id);
        return -EINVAL;
    }

    down_write(&g_device_task_info[dev_id]->task_rw_sema);
    flag = get_hotreset_task_flag(dev_id);
    if (flag != 0) {
        up_write(&g_device_task_info[dev_id]->task_rw_sema);
        dms_err("Hotreset try more than once. (dev_id=%u)\n", dev_id);
        return -EAGAIN;
    }
    set_hotreset_task_flag(dev_id);

    task_cnt = get_dms_task_info_cnt(dev_id);
    if (task_cnt != 0) {
        clear_hotreset_task_flag(dev_id);
        up_write(&g_device_task_info[dev_id]->task_rw_sema);
        dms_err("Dms task_cnt not zero. (task_cnt=%lu; dev_id=%u)\n", task_cnt, dev_id);
        return -EINVAL;
    }
    up_write(&g_device_task_info[dev_id]->task_rw_sema);

    dev = devdrv_manager_get_devdrv_info(dev_id);
    if (dev == NULL) {
        dms_err("Device info is NULL.\n");
        clear_hotreset_task_flag(dev_id);
        return -EINVAL;
    }

    ret = dms_notifyer_call(DMS_DEVICE_PRE_HOTRESET, dev);
    if (ret != 0) {
        dms_err("Notify call pre-hotreset failed. (ret=%d; dev_id=%u)\n", ret, dev_id);
        dms_notify_single_device_cancel_hotreset(dev_id);
        return ret;
    }

    return 0;
}

int dms_power_hotreset_init(void)
{
    int ret;

    ret = dms_register_notifier(&dms_power_hotreset_notifier);
    if (ret) {
        dms_err("Dms event register notifier failed. (ret=%d)\n", ret);
        return ret;
    }
    CALL_INIT_MODULE(DMS_MODULE_BASIC_POWER_INFO);
    dms_debug("Dms power hotreset init success.\n");
    return 0;
}

void dms_power_hotreset_exit(void)
{
    CALL_EXIT_MODULE(DMS_MODULE_BASIC_POWER_INFO);
    (void)dms_unregister_notifier(&dms_power_hotreset_notifier);
    dms_debug("Dms power hotreset exit success.\n");
}

int dms_hotreset_task_init(unsigned int dev_id)
{
    struct hotreset_task_info *device_task_info = NULL;

    device_task_info = dms_get_device_task_info(dev_id);
    if (device_task_info != NULL) {
        dms_info("Repeat init stop flag instance. (dev_id=%u)\n", dev_id);
    } else {
        device_task_info = dms_task_info_alloc(dev_id);
        if (device_task_info == NULL) {
            dms_err("Alloc stop flag info mem fail. (dev_id=%u)\n", dev_id);
            return -ENOMEM;
        }
    }

    dms_set_device_task_info(dev_id, device_task_info);
    clear_hotreset_task_flag(dev_id);
    g_device_task_info[dev_id]->task_ref_cnt = 0;
    dms_debug("Dms hotreset task init success. (dev_id=%u)\n", dev_id);
    return 0;
}

void dms_hotreset_task_exit(void)
{
    struct hotreset_task_info *device_task_info = NULL;
    unsigned int i;

    for (i = 0; i < DEVDRV_MAX_DAVINCI_NUM; i++) {
        device_task_info = dms_get_device_task_info(i);
        if (device_task_info == NULL) {
            continue;
        }

        dms_set_device_task_info(i, NULL);
        kfree(device_task_info);
        device_task_info = NULL;
    }

    dms_debug("Dms hotreset task exit success.\n");
}
