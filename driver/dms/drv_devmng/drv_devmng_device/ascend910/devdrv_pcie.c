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

#include "devdrv_pcie.h"
#include "devdrv_manager_comm.h"


int devdrv_manager_get_host_phy_mach_flag(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_get_host_phy_mach_flag_para para = {0};
    int ret;

    ret = copy_from_user_safe(&para, (void *)(uintptr_t)arg, sizeof(struct devdrv_get_host_phy_mach_flag_para));
    if (ret) {
        devdrv_drv_err("copy from user failed.\n");
        return -EINVAL;
    }

#ifdef CFG_SURPPORT_PCIE_HOST_DEVICE_COMM
    ret = agentdrv_get_host_phy_mach_flag(para.devId, &para.host_flag);
    if (ret) {
        devdrv_drv_err("cannot get host flag.\n");
        return -EINVAL;
    }
#else
    para.host_flag = DEVDRV_HOST_PHY_MACH_FLAG;
#endif
    ret = copy_to_user_safe((void *)(uintptr_t)arg, &para, sizeof(struct devdrv_get_host_phy_mach_flag_para));
    if (ret) {
        devdrv_drv_err("copy_to_user_safe failed.\n");
        return -EINVAL;
    }

    return ret;
}
#ifdef CFG_SURPPORT_PCIE_HOST_DEVICE_COMM
int devdrv_manager_get_local_devid_by_host_devid(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct devdrv_get_local_devid_para para = {0};
    int ret;
    int i;
    u32 phys_id = DEVDRV_MAX_DAVINCI_NUM + 1, vfid = 0;

    ret = copy_from_user_safe(&para, (void *)(uintptr_t)arg, sizeof(struct devdrv_get_local_devid_para));
    if (ret) {
        devdrv_drv_err("copy from user failed.\n");
        return -EINVAL;
    }

    ret = devdrv_manager_trans_and_check_id(para.host_dev_id, &phys_id, &vfid, ONLY_DOES_SUPPORT_PF);
    if (ret != 0) {
        devdrv_drv_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n", para.host_dev_id, ret);
        return ret;
    }

    if (phys_id >= DEVDRV_HOST_MAX_DEV_NUM) {
        devdrv_drv_err("host devid %u is invalid.\n", phys_id);
        return -EINVAL;
    }
    for (i = 0; i < DEVDRV_DEVICE_MAX_DEV_NUM; i++) {
        ret = agentdrv_get_host_devid(i);
        if (ret == (int)para.host_dev_id) {
            para.local_dev_id = i;
            goto out;
        }
    }
    ret = devdrv_manager_get_device_index(phys_id, &para.local_dev_id);
    if (ret) {
        devdrv_drv_err("host devid %u no found local devid.\n", para.host_dev_id);
        return ret;
    }

out:
    ret = copy_to_user_safe((void *)((uintptr_t)arg), &para, sizeof(struct devdrv_get_local_devid_para));
    if (ret) {
        devdrv_drv_err("copy to user failed, host devid %u.\n", para.host_dev_id);
    }
    return ret;
}
#endif

bool devdrv_manager_is_pf_device(unsigned int dev_id)
{
#ifdef CFG_FEATURE_SRIOV
    if (devdrv_get_pfvf_type_by_devid(dev_id) == DEVDRV_SRIOV_TYPE_VF) {
        return false;
    }
#elif (defined CFG_FEATURE_RC_MODE)
    if (VDAVINCI_IS_VDEV(dev_id)) {
        return false;
    }
#endif

    return true;
}

EXPORT_SYMBOL(devdrv_manager_is_pf_device);
