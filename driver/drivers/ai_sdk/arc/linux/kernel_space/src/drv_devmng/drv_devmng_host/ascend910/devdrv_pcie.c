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

#include <linux/fs.h>
#include <linux/delay.h>

#include "devdrv_pcie.h"

#ifndef CFG_FEATURE_RC_MODE
#include "dbl/uda.h"
#include "devdrv_manager.h"
#include "dms_time.h"
#include "dms_hotreset.h"
#include "virtmng_interface.h"
#endif

int devdrv_manager_check_permission(void)
{
    u32 root;

    root = (u32)(current->cred->euid.val);
    if (root != 0) {
        devdrv_drv_err("Only the root user can invoke the function.\n");
#ifdef CFG_FEATURE_ERRORCODE_ON_NEW_CHIPS
        return -EPERM;
#else
        return -EACCES;
#endif
    }

    if ((devdrv_manager_container_is_in_container() == true) || !capable(CAP_SYS_ADMIN)) {
        devdrv_drv_err("Common containers cannot invoke the function.\n");
#ifdef CFG_FEATURE_ERRORCODE_ON_NEW_CHIPS
        return -EOPNOTSUPP;
#else
        return -EACCES;
#endif
    }

    return 0;
}

#ifndef CFG_FEATURE_RC_MODE
STATIC int devdrv_manager_check_phy_mach(u32 dev_id)
{
    return dms_power_check_phy_mach(dev_id);
}

STATIC int devdrv_manager_check_all_phy_mach(void)
{
    u32 dev_num;
    u32 phys_id = 0;
    u32 vfid = 0;
    int ret;
    int i;

    dev_num = devdrv_manager_get_devnum();
    if (dev_num > DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("get invalid devnum %d.\n", dev_num);
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

STATIC bool devdrv_manager_check_is_vf(unsigned int dev_id, unsigned int vfid)
{
    if ((vfid != 0) || (!devdrv_manager_is_pf_device(dev_id))) {
        return true;
    }

    return false;
}

int devdrv_manager_pcie_pre_reset(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    u32 phys_id = 0;
    u32 vfid = 0;
    struct devdrv_pcie_pre_reset para;

    ret = copy_from_user_safe(&para, (void *)((uintptr_t)arg), sizeof(struct devdrv_pcie_pre_reset));
    if (ret) {
        devdrv_drv_err("Failed to invoke copy_from_user_safe. (ret=%d)\n", ret);
        return -EINVAL;
    }

    if (devdrv_manager_container_logical_id_to_physical_id(para.dev_id, &phys_id, &vfid) != 0) {
        devdrv_drv_err("Failed to transfer dev_id. (dev_id=%u)\n", para.dev_id);
        return -EFAULT;
    }

    if (devdrv_manager_check_is_vf(phys_id, vfid)) {
        devdrv_drv_err("Not support for vdevice. (phys_id=%u; vfid=%u)\n", phys_id, vfid);
        return -EOPNOTSUPP;
    }
#ifdef CFG_SOC_PLATFORM_CLOUD
    return 0;
#else
    ret = devdrv_manager_check_permission();
    if (ret) {
        devdrv_drv_err("devdrv_manager_check_permission failed, ret(%d).\n", ret);
        return ret;
    }

    ret = devdrv_manager_check_phy_mach(phys_id);
    if (ret) {
        devdrv_drv_err("permission deny or devid[%d] is invalid, ret = %d\n", para.dev_id, ret);
#ifdef CFG_FEATURE_ERRORCODE_ON_NEW_CHIPS
        return -EOPNOTSUPP;
#else
        return -EINVAL;
#endif
    }

#ifndef DEVDRV_MANAGER_HOST_UT_TEST
    ret = devdrv_uda_one_dev_ctrl_hotreset(phys_id);
    if (ret != 0) {
        devdrv_drv_err("Call uda_dev_ctrl failed, (phys_id=%u; ret=%d).\n", phys_id, ret);
        return ret;
    }
#endif

    ret = devdrv_pcie_prereset(phys_id);
    if (ret != 0) {
#ifndef DEVDRV_MANAGER_HOST_UT_TEST
        devdrv_uda_one_dev_ctrl_hotreset_cancel(phys_id);
#endif
    }

    return ret;
#endif
}

int devdrv_manager_pcie_rescan(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    u32 udevid = 0;
    struct devdrv_pcie_rescan para;

    ret = copy_from_user_safe(&para, (void *)((uintptr_t)arg), sizeof(struct devdrv_pcie_rescan));
    if (ret) {
        devdrv_drv_err("Failed to invoke copy_from_user_safe. (ret=%d)\n", ret);
        return -EINVAL;
    }
    ret = uda_ns_node_devid_to_udevid(para.dev_id, &udevid);
    if (ret != 0) {
        devdrv_drv_err("Failed to transfer dev_id to udevid. (dev_id=%u)\n", para.dev_id);
        return -EFAULT;
    }
    if (!uda_is_phy_dev(udevid)) {
        devdrv_drv_err("Not support for vdevice. (udevid=%u)\n", udevid);
        return -EOPNOTSUPP;
    }

#ifdef CFG_SOC_PLATFORM_CLOUD
    return 0;
#else
    ret = devdrv_manager_check_permission();
    if (ret) {
        devdrv_drv_err("devdrv_manager_check_permission failed, ret(%d).\n", ret);
        return ret;
    }

    ret = devdrv_pcie_reinit(udevid);

    return ret;
#endif
}

#define ALL_DEVICE_RESET_FLAG 0xff

#ifdef CFG_FEATURE_SRIOV
STATIC int devdrv_manager_disable_sriov(unsigned int dev_id)
{
    int ret;
    int vf_num = 0;

    ret = devdrv_get_pci_enabled_vf_num(dev_id, &vf_num);
    if (ret != 0) {
        devdrv_drv_err("Get enable vf number failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return -ENODEV;
    }

    devdrv_drv_info("vf_num. (dev_id=%u; vf_num=%d)\n", dev_id, vf_num);
    if (vf_num > 0) {
        devdrv_drv_info("Disable sriov. (dev_id=%u)\n", dev_id);
        ret = vmngh_disable_sriov(dev_id);
        if (ret != 0) {
            devdrv_drv_err("Disable sriov failed. (dev_id=%u; ret=%d)", dev_id, ret);
            return -ENODEV;
        }
    }

    return 0;
}

STATIC int devdrv_manager_check_and_disable_sriov(unsigned int dev_id)
{
    int ret;
    int dev_num, i;
    u32 phy_id = 0;
    u32 vfid = 0;

    /*
     *  1. if device id is DEVDRV_RESET_ALL_DEVICE_ID, check all device's vf num,
     *     if one device sriov is enable, disable it
     *  2. if device id is single device's id, check and disable sriov when it's enable.
     */
    if (dev_id == DEVDRV_RESET_ALL_DEVICE_ID) {
        dev_num = devdrv_get_dev_num();
        if (dev_num <= 0) {
            devdrv_drv_err("There is no pcie device. (dev_id=%u)\n", dev_id);
            return -ENODEV;
        }

        devdrv_drv_info("Device number. (dev_num=%d)\n", dev_num);
        for (i = 0; i < dev_num; i++) {
            ret = devdrv_manager_container_logical_id_to_physical_id(i, &phy_id, &vfid);
            if (ret) {
                devdrv_drv_err("Transfor logical id to physical id failed. (logical_id=%d; ret=%d)\n", i, ret);
                return ret;
            }

            ret = devdrv_manager_disable_sriov(phy_id);
            if (ret != 0) {
                devdrv_drv_err("Disable sriov failed. (dev_id=%d; ret=%d)\n", i, ret);
                return ret;
            }
        }
    } else {
        ret = devdrv_manager_disable_sriov(dev_id);
        if (ret != 0) {
            devdrv_drv_err("Disable sriov failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
            return ret;
        }
    }

    return 0;
}
#endif

STATIC int devdrv_manager_check_hotreset_dev_num(void)
{
    u32 dev_num;
    u32 probe_dev_num;

    dev_num = devdrv_manager_get_devnum();
    probe_dev_num = devdrv_manager_get_probe_num_kernel();
    if (dev_num != probe_dev_num) {
        devdrv_drv_err("dev_num is not equal to probe_dev_num, devices not all ready.(dev_num=%u; probe_dev_num=%u)\n",
            dev_num, probe_dev_num);
        return -EBUSY;
    }

    return 0;
}

int devdrv_manager_pcie_hot_reset(struct devdrv_pcie_hot_reset para)
{
    u32 phy_id = 0;
    u32 vfid;
    int ret;
#ifdef CFG_FEATURE_TIMESYNC
    int i;
#endif

    devdrv_drv_event("Hot reset operation. (dev_id=%u)\n", para.dev_id);

    /* hot reset all devices */
    if (para.dev_id == DEVDRV_RESET_ALL_DEVICE_ID) {
        ret = devdrv_manager_check_all_phy_mach();
        if (ret != 0) {
            devdrv_drv_err("Permission deny or devid is invalid. (dev_id=%u; ret=%d)\n", para.dev_id, ret);
            return ret;
        }

#ifdef CFG_FEATURE_TIMESYNC
        for (i = 0; i < DEVDRV_MAX_DAVINCI_NUM; i++) {
            set_time_need_update(i);
        }
#endif

#ifdef CFG_FEATURE_SRIOV
        ret = devdrv_manager_check_and_disable_sriov(para.dev_id);
        if (ret != 0) {
            devdrv_drv_err("Check disable sriov failed. (dev_id=%u; ret=%d)\n", para.dev_id, ret);
            return ret;
        }
#endif

        ret = devdrv_manager_check_hotreset_dev_num();
        if (ret != 0) {
            devdrv_drv_err("Check hot reset device number fail. (dev_id=%u; ret=%d)\n", para.dev_id, ret);
            return ret;
        }

        /* all device hotreset not trigger uda hotreset */
        devdrv_set_device_hotreset_flag(DEVDRV_MAX_DAVINCI_NUM, 1);
        return devdrv_hot_reset_device(ALL_DEVICE_RESET_FLAG);
    }

    /* the single device hot reset */
    ret = devdrv_manager_container_logical_id_to_physical_id(para.dev_id, &phy_id, &vfid);
    if (ret != 0) {
        devdrv_drv_err("Transfer logical_id to physical_id failed. (devid=%u; ret=%d)\n", para.dev_id, ret);
        return ret;
    }

    devdrv_set_device_hotreset_flag(phy_id, 1);

    if (devdrv_manager_is_pf_device(phy_id)) {
        ret = devdrv_manager_check_phy_mach(phy_id);
        if (ret != 0) {
            devdrv_drv_err("Permission deny or devid is invalid. (dev_id=%u; ret=%d)\n", para.dev_id, ret);
            return ret;
        }

#ifdef CFG_FEATURE_TIMESYNC
        set_time_need_update(phy_id);
#endif

#ifdef CFG_FEATURE_SRIOV
        ret = devdrv_manager_check_and_disable_sriov(phy_id);
        if (ret != 0) {
            devdrv_drv_err("Check disable sriov failed. (dev_id=%u; phy_id=%u; ret=%d)\n", para.dev_id, phy_id, ret);
            return ret;
        }
#endif
        return devdrv_hot_reset_device(phy_id);
    } else if (devdrv_manager_is_mdev_vm_mode(phy_id)) {
        return devdrv_hot_reset_device(phy_id);
    } else {
        ret = vmngh_sriov_reset_vdev(phy_id, vfid);
        if (ret != 0) {
            devdrv_drv_err("vmngh_sriov_reset_vdev failed. (devid=%d; phy_id=%u; vfid=%u)\n",
                para.dev_id, phy_id, vfid);
        }

        return ret;
    }
}

#ifndef DEVDRV_MANAGER_HOST_UT_TEST
static void devdrv_uda_dev_ctrl_hotreset_restore(u32 last_devid)
{
    u32 i;
    for (i = 0; i < last_devid; i++) {
        if (uda_is_udevid_exist(i)) {
            (void)uda_dev_ctrl(i, UDA_CTRL_HOTRESET_CANCEL);
        }
    }
}
#endif

static int devdrv_uda_dev_ctrl_hotreset(u32 devid)
{
#ifndef DEVDRV_MANAGER_HOST_UT_TEST
    int ret;
    u32 i, udevid;

    if (devid < DEVDRV_MAX_DAVINCI_NUM) {
        ret = uda_devid_to_udevid(devid, &udevid);
        if (ret != 0) {
            devdrv_drv_err("Uda devid trans failed. (devid=%u; ret=%d)\n", devid, ret);
            return ret;
        }

        ret = devdrv_uda_one_dev_ctrl_hotreset(udevid);
        if (ret != 0) {
            devdrv_drv_err("Uda ctrl hotreset failed. (devid=%u; ret=%d)\n", devid, ret);
            return ret;
        }

        return 0;
    }

    for (i = 0; i < DEVDRV_PF_DEV_MAX_NUM; i++) {
        if (!uda_is_udevid_exist(i)) {
            continue;
        }

        ret = uda_dev_ctrl(i, UDA_CTRL_HOTRESET);
        if (ret != 0) {
            devdrv_uda_dev_ctrl_hotreset_restore(i);
            devdrv_drv_err("Uda ctrl hotreset failed. (devid=%u; ret=%d)\n", i, ret);
            return ret;
        }
    }
#endif

    return 0;
}

static void devdrv_uda_dev_ctrl_hotreset_cancel(u32 devid)
{
#ifndef DEVDRV_MANAGER_HOST_UT_TEST
    int ret;
    if (devid < DEVDRV_MAX_DAVINCI_NUM) {
        u32 udevid;
        ret = uda_devid_to_udevid(devid, &udevid);
        if (ret == 0) {
            devdrv_uda_one_dev_ctrl_hotreset_cancel(udevid);
        }
    } else {
        u32 i;
        for (i = 0; i < DEVDRV_PF_DEV_MAX_NUM; i++) {
            if (uda_is_udevid_exist(i)) {
                (void)uda_dev_ctrl(i, UDA_CTRL_HOTRESET_CANCEL);
            }
        }
    }
#endif
}

int devdrv_manager_pcie_hot_reset_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_pcie_hot_reset para;
    int ret, wait_time = 0;

    ret = devdrv_manager_check_permission();
    if (ret) {
        devdrv_drv_err("devdrv_manager_check_permission failed, ret(%d).\n", ret);
        return ret;
    }

    ret = copy_from_user_safe(&para, (void *)((uintptr_t)arg), sizeof(struct devdrv_pcie_hot_reset));
    if (ret != 0) {
        devdrv_drv_err("Copy from user failed. (ret=%d)\n", ret);
        return -EINVAL;
    }

    ret = devdrv_uda_dev_ctrl_hotreset(para.dev_id);
    if (ret != 0) {
        return ret;
    }

    ret = devdrv_manager_pcie_hot_reset(para);
    if (ret != 0) {
        devdrv_uda_dev_ctrl_hotreset_cancel(para.dev_id);
        return ret;
    }

    while (wait_time++ < 10000) { /* wait 10000 us for device init */
        if (devdrv_check_device_hotreset_finish(para.dev_id)) {
            devdrv_drv_info("Wait hotreset finish. (dev_id=%u; wait_time=%u(ms))\n", para.dev_id, wait_time);
            return 0;
        }

        msleep(1);
    }

    devdrv_drv_warn("Wait hotreset finish timeout. (dev_id=%u; wait_time=%u(ms))\n", para.dev_id, wait_time);
    return 0;
}

// UEFI SRAM FLAG
#ifdef CFG_SOC_PLATFORM_CLOUD_V2
#define DEVDRV_SRAM_BBOX_UEFI_DUMP_OFFSET 0x29004
#else
#define DEVDRV_SRAM_BBOX_UEFI_DUMP_OFFSET 0x0800
#endif
#ifndef CFG_SOC_PLATFORM_CLOUD
#define DEVDRV_SRAM_BBOX_UEFI_DUMP_LEN 20
#else
#define DEVDRV_SRAM_BBOX_UEFI_DUMP_LEN 28
#endif

// BIOS SRAM log (include POINT FLAG)
#ifdef CFG_SOC_PLATFORM_MINIV2
#define DEVDRV_SRAM_BBOX_BIOS_OFFSET 0x14000UL
#else
#define DEVDRV_SRAM_BBOX_BIOS_OFFSET 0x3DC00UL
#endif
#define DEVDRV_SRAM_BBOX_BIOS_LEN 0x400UL

int drv_pcie_para_check(u32 phys_id, struct devdrv_pcie_read_para *pcie_read_para)
{
    if (pcie_read_para->type == DEVDRV_PCIE_READ_TYPE_SRAM &&
        !(((pcie_read_para->offset >= DEVDRV_SRAM_BBOX_UEFI_DUMP_OFFSET) &&
           (((u64)pcie_read_para->offset + (u64)pcie_read_para->len) <=
            DEVDRV_SRAM_BBOX_UEFI_DUMP_OFFSET + DEVDRV_SRAM_BBOX_UEFI_DUMP_LEN)) ||
          ((pcie_read_para->offset >= DEVDRV_SRAM_BBOX_BIOS_OFFSET) &&
           (((u64)pcie_read_para->offset + (u64)pcie_read_para->len) <=
            DEVDRV_SRAM_BBOX_BIOS_OFFSET + DEVDRV_SRAM_BBOX_BIOS_LEN)))) {
        devdrv_drv_err("cannot access sram, dev_id:%u, offset:%u, len:%u.\n", phys_id, pcie_read_para->offset,
                       pcie_read_para->len);
        return -EINVAL;
    }

    if ((pcie_read_para->type == DEVDRV_PCIE_READ_TYPE_REG_SRAM) &&
        devdrv_manager_check_capability(phys_id, DEVDRV_CAP_IMU_REG_EXPORT)) {
        devdrv_drv_err("do not support read reg sram, dev_id:%u, read type:%u.\n",
            phys_id, pcie_read_para->type);
        return -EINVAL;
    }

    return 0;
}

int drv_get_data_addr_info(u32 phys_id, enum devdrv_pcie_read_type type, u32 *addr_type)
{
    if (type == DEVDRV_PCIE_READ_TYPE_SRAM) {
        *addr_type = DEVDRV_ADDR_LOAD_RAM;
    } else if (type == DEVDRV_PCIE_READ_TYPE_DDR) {
#ifdef CFG_FEATURE_PCIE_BBOX_ADDR
        *addr_type = DEVDRV_ADDR_BBOX_BASE;
#else
        *addr_type = DEVDRV_ADDR_TEST_BASE;
#endif
    } else if (type == DEVDRV_PCIE_READ_TYPE_REG_SRAM) {
        *addr_type = DEVDRV_ADDR_REG_SRAM_BASE;
    } else {
        *addr_type = DEVDRV_ADDR_HDR_BASE;
    }

    return 0;
}

int drv_pcie_read_proc(struct devdrv_pcie_read_para* para)
{
    u32 type;
    int ret;
    u32 phys_id = para->devId;

    ret = drv_get_data_addr_info(phys_id, para->type, &type);
    if (ret) {
        devdrv_drv_err("get addr by dev_id(%u) failed, type %d, ret(%d).\n", phys_id, para->type, ret);
        return ret;
    }

    ret = devdrv_pcie_read_proc(para->devId, (enum devdrv_addr_type)type, para->offset, para->value, para->len);
    if (ret) {
        devdrv_drv_err("get addr by dev_id(%u) failed, type %d, ret(%d).\n", phys_id, para->type, ret);
        return ret;
    }

    return 0;
}

int drv_pcie_read(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_pcie_read_para pcie_read_para = {0};
    u32 phys_id;
    int ret;

    /* bbox is not support container */
    if (devdrv_manager_container_is_in_container()) {
        devdrv_drv_err("bbox read interface is not support container\n");
        return -EPERM;
    }
    ret = copy_from_user_safe(&pcie_read_para, (void *)((uintptr_t)arg),
                              sizeof(struct devdrv_pcie_read_para));
    if (ret) {
        devdrv_drv_err("copy pcie from user failed, ret(%d).\n", ret);
        return -EINVAL;
    }

    phys_id = pcie_read_para.devId;

    /* only can read within region for bbox */
    if (drv_pcie_para_check(phys_id, &pcie_read_para)) {
        devdrv_drv_err("dev_id:%u cannot access sram.\n", phys_id);
        return -EINVAL;
    }

    ret = drv_pcie_read_proc(&pcie_read_para);
    if (ret) {
        devdrv_drv_err("dev %d read proc fail, ret %d.\n", phys_id, ret);
        return ret;
    }

    ret = copy_to_user_safe((void *)(uintptr_t)arg, &pcie_read_para, sizeof(struct devdrv_pcie_read_para));
    return ret;
}

int drv_pcie_bbox_imu_ddr_read_proc(struct devdrv_pcie_imu_ddr_read_para* para)
{
    int ret;

    ret = devdrv_pcie_read_proc(para->devId, DEVDRV_ADDR_IMU_LOG_BASE, para->offset, para->value, para->len);
    if (ret) {
        devdrv_drv_err("dev_id %u offset %d, ret %d.\n", para->devId, para->offset, ret);
        return ret;
    }

    return 0;
}

/* bbox imu ddr in bar0 ATU12 */
int drv_pcie_bbox_imu_ddr_read(struct file *filep, unsigned int cmd, unsigned long arg)
{
#ifdef CFG_SOC_PLATFORM_CLOUD
    struct devdrv_pcie_imu_ddr_read_para imu_ddr_read_para = {0};
    int ret;

    /* bbox is not support container */
    if (devdrv_manager_container_is_in_container()) {
        devdrv_drv_err("bbox imu ddr read interface is not support container\n");
        return -EPERM;
    }
    ret = copy_from_user_safe(&imu_ddr_read_para, (void *)((uintptr_t)arg),
                              sizeof(struct devdrv_pcie_imu_ddr_read_para));
    if (ret) {
        devdrv_drv_err("copy pcie ddr from user failed, ret(%d).\n", ret);
        return -EINVAL;
    }

    ret = drv_pcie_bbox_imu_ddr_read_proc(&imu_ddr_read_para);
    if (ret) {
        devdrv_drv_err("dev %d imu ddr read proc fail, ret %d.", imu_ddr_read_para.devId, ret);
        return ret;
    }

    ret = copy_to_user_safe((void *)(uintptr_t)arg, &imu_ddr_read_para, sizeof(struct devdrv_pcie_imu_ddr_read_para));

    return ret;
#else
    return 0;
#endif
}

int devdrv_manager_p2p_attr_op(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_p2p_attr p2p_attr;
    int ret, pid;
    struct devdrv_manager_context *dev_manager_context = NULL;
    u32 peer_phys_id;
    u32 phys_id = 0;
    u32 vfid = 0;

    if ((filep == NULL) || (filep->private_data == NULL)) {
        devdrv_drv_err("filep is %s.\n", (filep == NULL) ? "NULL" : "OK");
        return -EINVAL;
    }

    dev_manager_context = filep->private_data;
    pid = dev_manager_context->pid;

    ret = copy_from_user_safe(&p2p_attr, (void *)((uintptr_t)arg), sizeof(struct devdrv_p2p_attr));
    if (ret) {
        devdrv_drv_err("copy from user failed, ret(%d).\n", ret);
        return -EINVAL;
    }

    if (devdrv_manager_container_logical_id_to_physical_id(p2p_attr.dev_id, &phys_id, &vfid) != 0) {
        devdrv_drv_err("can't transfor virt id %u \n", p2p_attr.dev_id);
        return -EFAULT;
    }

    if (devdrv_manager_check_is_vf(phys_id, vfid)) {
        devdrv_drv_err("Not support for vdevice. (phys_id=%u; vfid=%u)\n", phys_id, vfid);
        return -EOPNOTSUPP;
    }

    peer_phys_id = p2p_attr.peer_dev_id;

    ret = dms_hotreset_task_cnt_increase(phys_id);
    if (ret != 0) {
        devdrv_drv_err("Hotreset task cnt increase failed. (dev_id=%u; ret=%d)\n", phys_id, ret);
        return ret;
    }

    ret = -EINVAL;

    switch (p2p_attr.op) {
        case DEVDRV_P2P_ADD:
            ret = devdrv_enable_p2p(pid, phys_id, peer_phys_id);
            break;
        case DEVDRV_P2P_DEL:
            ret = devdrv_disable_p2p(pid, phys_id, peer_phys_id);
            break;
        case DEVDRV_P2P_QUERY:
            if (devdrv_is_p2p_enabled(phys_id, peer_phys_id)) {
                ret = 0;
            }
            break;
        case DEVDRV_P2P_CAPABILITY_QUERY:
            ret = devdrv_get_p2p_access_status(phys_id, peer_phys_id, &p2p_attr.status);
            if (ret) {
                devdrv_drv_err("devid %d devdrv_get_p2p_access_status fail %u\n", phys_id, ret);
                dms_hotreset_task_cnt_decrease(phys_id);
                return ret;
            }
            ret = copy_to_user_safe((void *)(uintptr_t)arg, &p2p_attr, sizeof(struct devdrv_p2p_attr));
            break;
        default:
            break;
    }
    dms_hotreset_task_cnt_decrease(phys_id);
    return ret;
}

int devdrv_manager_get_p2p_capability(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_p2p_capability p2p_capability;
    u32 phys_id = 0;
    u32 vfid = 0;
    int ret;

    ret = copy_from_user_safe(&p2p_capability, (void *)((uintptr_t)arg), sizeof(struct devdrv_p2p_capability));
    if (ret) {
        devdrv_drv_err("copy from user failed, ret(%d).\n", ret);
        return -EINVAL;
    }
    if (devdrv_manager_container_logical_id_to_physical_id(p2p_capability.dev_id, &phys_id, &vfid) != 0) {
        devdrv_drv_err("can't transfor virt id %u \n", p2p_capability.dev_id);
        return -EFAULT;
    }

    ret = dms_hotreset_task_cnt_increase(phys_id);
    if (ret != 0) {
        devdrv_drv_err("Hotreset task cnt increase failed. (dev_id=%u; ret=%d)\n", phys_id, ret);
        return ret;
    }

    ret = devdrv_get_p2p_capability(phys_id, &p2p_capability.capability);
    if (ret) {
        devdrv_drv_err("devid %d devdrv_get_p2p_capability fail %u\n", p2p_capability.dev_id, ret);
        dms_hotreset_task_cnt_decrease(phys_id);
        return ret;
    }
    ret = copy_to_user_safe((void *)(uintptr_t)arg, &p2p_capability, sizeof(struct devdrv_p2p_capability));

    dms_hotreset_task_cnt_decrease(phys_id);
    return ret;
}

/*
 * The original vendor tree expects a lower-level host PCIe helper to provide
 * this query. That helper is not present in the trimmed 6.18 build yet, so
 * keep the ioctl wired up with a conservative fallback: advertise no P2P
 * capability instead of leaving an unresolved symbol at link time.
 */
int devdrv_get_p2p_capability(u32 dev_id, u64 *capability)
{
    if (capability == NULL)
        return -EINVAL;

    *capability = 0;
    return 0;
}
EXPORT_SYMBOL(devdrv_get_p2p_capability);

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

    if ((va < vma->vm_start) || (va > vma->vm_end) ||
        (end > vma->vm_end) || (va >= end)) {
        devdrv_drv_err("va(0x%llx) end(0x%llx) size(0x%x) error.\n", va, end, size);
        return -EINVAL;
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
    /*
     * follow_pfn() is no longer available to external modules on modern
     * kernels. Keep the VMA range checks and let the later remap helpers
     * validate the actual PFN mapping operation.
     */
    (void)pfn;
#else
    for (va_check = va; va_check < end; va_check += PAGE_SIZE) {
        if (follow_pfn(vma, va_check, &pfn) == 0) {
            devdrv_drv_err("va(0x%llx) size(0x%x), va_check(0x%llx) is valid\n", va, size, va_check);
            return -EINVAL;
        }
    }
#endif
    return 0;
}

int drv_dma_mmap(struct file *filep, unsigned int cmd, unsigned long arg)
{
#ifdef CFG_FEATURE_ERRORCODE_ON_NEW_CHIPS
    return -EOPNOTSUPP;
#else
    struct devdrv_alloc_host_dma_addr_para alloc_host_dma_addr_para = {0};
    unsigned long long bbox_resv_dmaAddr = 0;
    struct page *bbox_resv_dmaPages = NULL;
    unsigned int bbox_resv_size = 0;
    struct vm_area_struct *vma = NULL;
    u32 phys_id;
    int ret;

    ret = devdrv_manager_check_permission();
    if (ret) {
        devdrv_drv_err("check permission fail.\n");
        return ret;
    }

    ret = copy_from_user_safe(&alloc_host_dma_addr_para, (void *)(uintptr_t)arg,
                              sizeof(struct devdrv_alloc_host_dma_addr_para));
    if (ret) {
        devdrv_drv_err("copy from user failed, ret(%d).\n", ret);
        return -EINVAL;
    }

    phys_id = alloc_host_dma_addr_para.devId;
#ifdef CFG_FEATURE_SRIOV
    if (devdrv_manager_is_mdev_vm_mode(phys_id)) {
        return -EOPNOTSUPP;
    }
#endif

    down_write(get_mmap_sem(current->mm));
    vma = find_vma(current->mm, alloc_host_dma_addr_para.virAddr);
    if (vma == NULL) {
        up_write(get_mmap_sem(current->mm));
        devdrv_drv_err("find vma failed. dev_id(%u)\n", phys_id);
        return 0;
    }

    ret = devdrv_get_bbox_reservd_mem(phys_id, &bbox_resv_dmaAddr, &bbox_resv_dmaPages, &bbox_resv_size);
    if (ret) {
        up_write(get_mmap_sem(current->mm));
        devdrv_drv_err("devdrv_get_bbox_reservd_mem failed, ret(%d). dev_id(%u)\n", ret, phys_id);
        return -EINVAL;
    }

    if (bbox_resv_size > 0) {
        ret = devdrv_check_va(vma, alloc_host_dma_addr_para.virAddr, bbox_resv_size);
        if (ret) {
            up_write(get_mmap_sem(current->mm));
            devdrv_drv_err("dev_id(%u) va(0x%llx) already map, ret(%d).\n",
                phys_id, alloc_host_dma_addr_para.virAddr, ret);
            return -EINVAL;
        }
        ret = remap_pfn_range(vma, alloc_host_dma_addr_para.virAddr,
            page_to_pfn(bbox_resv_dmaPages), bbox_resv_size, PAGE_READONLY);
        if (ret) {
            up_write(get_mmap_sem(current->mm));
            devdrv_drv_err("remap_pfn_range failed, ret(%d). dev_id(%u)\n", ret, phys_id);
            return -EINVAL;
        }
    }
    up_write(get_mmap_sem(current->mm));

    devdrv_drv_info("use pcie reserved buffer,"
                    "page:%pK, size:0x%pK, devid:%u.\n",
                    bbox_resv_dmaPages, (void *)(uintptr_t)bbox_resv_size, phys_id);

    alloc_host_dma_addr_para.size = bbox_resv_size;
    alloc_host_dma_addr_para.phyAddr = 0;

    ret = copy_to_user_safe((void *)((uintptr_t)arg), &alloc_host_dma_addr_para,
                            sizeof(struct devdrv_alloc_host_dma_addr_para));
    if (ret) {
        devdrv_drv_err("copy to user failed, ret(%d), dev_id(%u).\n", ret, phys_id);
        return -EINVAL;
    }
    return 0;
#endif
}

#define DEVDRV_MAX_FILE_SIZE (1024 * 100)
#define DEVDRV_STR_MAX_LEN 100
#define DEVDRV_CONFIG_OK 0
#define DEVDRV_CONFIG_FAIL 1
#define DEVDRV_CONFIG_NO_MATCH 1

int drv_get_device_boot_status(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_get_device_boot_status_para get_device_boot_status_para = {0};
    int ret;
    u32 phys_id;
    ret = copy_from_user_safe(&get_device_boot_status_para, (void *)((uintptr_t)arg),
                              sizeof(struct devdrv_get_device_boot_status_para));
    if (ret) {
        devdrv_drv_err("copy from user failed, ret(%d).\n", ret);
        return -EINVAL;
    }

    phys_id = get_device_boot_status_para.devId;
    if (phys_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("phys_id %d is invalid\n", phys_id);
        return -EINVAL;
    }

    /* only judge the physical id is available in container */
    if (devdrv_manager_container_is_in_container()) {
        if (!uda_task_can_access_udevid_inherit(current, phys_id)) {
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

int devdrv_manager_get_host_phy_mach_flag(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_get_host_phy_mach_flag_para para = {0};
    u32 phys_id = 0;
    u32 vfid = 0;
    int ret;

    ret = copy_from_user_safe(&para, (void *)((uintptr_t)arg), sizeof(struct devdrv_get_host_phy_mach_flag_para));
    if (ret) {
        devdrv_drv_err("copy from user failed, ret(%d).\n", ret);
        return -EINVAL;
    }

    ret = devdrv_manager_container_logical_id_to_physical_id(para.devId, &phys_id, &vfid);
    if (ret) {
        devdrv_drv_err("can't transfor virt id %u ret(%d)\n", para.devId, ret);
        return -EFAULT;
    }

    if (devdrv_manager_is_mdev_vm_mode(phys_id)) {
        para.host_flag = DEVDRV_HOST_VM_MACH_FLAG;
    } else {
        ret = devdrv_get_host_phy_mach_flag(phys_id, &para.host_flag);
        if (ret) {
            devdrv_drv_err("cannot get host flag, dev_id(%u).\n", para.devId);
            return -EINVAL;
        }
    }

    ret = copy_to_user_safe((void *)((uintptr_t)arg), &para, sizeof(struct devdrv_get_host_phy_mach_flag_para));

    return ret;
}

int devdrv_manager_get_master_device_in_the_same_os(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_get_device_os_para get_device_os_para = {0};
    u32 phyid = 0;
    u32 vfid = 0;
    int ret;

    ret = devdrv_manager_container_logical_id_to_physical_id(0, &phyid, &vfid);
    if (ret) {
        devdrv_drv_err("logical_id to physical id failed, ret(%d).\n", ret);
        return -EINVAL;
    }

    if (devdrv_manager_check_is_vf(phyid, vfid)) {
        devdrv_drv_err("Not support for vdevice. (phys_id=%u; vfid=%u)\n", phyid, vfid);
        return -EOPNOTSUPP;
    }

    ret = copy_from_user_safe(&get_device_os_para, (void *)((uintptr_t)arg),
                              sizeof(struct devdrv_get_device_os_para));
    if (ret) {
        devdrv_drv_err("copy from user failed, ret(%d).\n", ret);
        return -EINVAL;
    }

    ret = devdrv_get_master_devid_in_the_same_os(get_device_os_para.dev_id, &(get_device_os_para.master_dev_id));
    if (ret) {
        devdrv_drv_err("failed, ret(%d), dev_id(%u).\n", ret, get_device_os_para.dev_id);
        return ret;
    }

    ret = copy_to_user_safe((void *)((uintptr_t)arg), &get_device_os_para,
                            sizeof(struct devdrv_get_device_os_para));

    return ret;
}

bool devdrv_manager_is_pf_device(unsigned int dev_id)
{
#ifdef CFG_FEATURE_SRIOV
    if (devdrv_get_pfvf_type_by_devid(dev_id) == DEVDRV_SRIOV_TYPE_VF) {
        return false;
    }
#endif

    return true;
}

bool devdrv_manager_is_mdev_vm_mode(unsigned int dev_id)
{
    return devdrv_is_mdev_vm_boot_mode(dev_id);
}

bool devdrv_manager_is_mdev_vf_vm_mode(unsigned int dev_id)
{
#ifndef DEVDRV_MANAGER_HOST_UT_TEST
    /*
     * Older vendor trees distinguished VF/full-spec VM boot types through
     * devdrv_get_env_boot_type(). The exported helper available in the 6.18
     * port only preserves the higher-level "mdev VM boot mode" semantic.
     */
    return devdrv_is_mdev_vm_boot_mode(dev_id);
#endif
}

int devdrv_manager_is_pm_boot_mode(unsigned int dev_id, bool *is_pm_boot)
{
    unsigned int host_flag;
    int ret;

    ret = devdrv_get_host_phy_mach_flag(dev_id, &host_flag);
    if (ret != 0) {
        devdrv_drv_err("Get host phy_mach_flag failed. (devid=%u; ret=0x%x)\n", dev_id, ret);
        return ret;
    }

    if (host_flag == DEVDRV_HOST_PHY_MACH_FLAG) {
        *is_pm_boot = true;
        return 0;
    }

    *is_pm_boot = false;
    return 0;
}

bool devdrv_manager_is_sriov_support(unsigned int dev_id)
{
    return devdrv_is_sriov_support(dev_id);
}

#endif /* CFG_FEATURE_RC_MODE */

