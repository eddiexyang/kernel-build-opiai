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

#include "linux/gpio.h"
#include "dms_define.h"
#include "dms_template.h"
#include "dms_cmd_def.h"
#include "urd_acc_ctrl.h"
#ifndef CFG_HOST_ENV
#include "dms_device_time_zone.h"
#endif
#ifdef AOS_LLVM_BUILD
#include "dms_pid_map.h"
#endif
#ifdef SUPPORT_ASCEND_PM
#include "dms_pm.h"
#endif
#include "devdrv_user_common.h"
#include "devdrv_manager_common.h"
#include "dms_chip_info.h"
#include "dms_vdev.h"
#ifndef AOS_LLVM_BUILD
#include "ascend_kernel_hal.h"
#endif
#include "dms_basic_info.h"
#ifdef CFG_FEATURE_DEV_TOPOLOGY
#include "dms_dev_topology.h"
#endif
#ifdef CFG_FEATURE_MEM
#include "dms_mem_info.h"
#endif

#ifndef AOS_LLVM_BUILD
#include "virtmng_interface.h"

int dms_trans_and_check_id(u32 logical_dev_id, u32 *physical_dev_id, u32 *vfid)
{
    int ret;

    if (logical_dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        dms_err("Wrong logic id. (logic_id=%u)\n", logical_dev_id);
        return -EINVAL;
    }

    ret = devdrv_manager_container_logical_id_to_physical_id(logical_dev_id, physical_dev_id, vfid);
    if (ret != 0) {
        dms_err("Transfer logic id to phy id failed. (logic_id=%u; ret=%d)\n", logical_dev_id, ret);
        return ret;
    }

    if (*physical_dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        dms_err("wrong phy id. (logic_id=%u; phy_id=%u)\n", logical_dev_id, *physical_dev_id);
        return -EINVAL;
    }

    return 0;
}

STATIC int dms_get_device_split_mode(void *feature, char *in, u32 in_len, char *out, u32 out_len)
{
    int ret;
    unsigned int dev_id, phy_id, vfid;

    if ((in == NULL) || (in_len != sizeof(u32))) {
        dms_err("input arg is NULL, or in_len is wrong. (in_len=%u)\n", in_len);
        return -EINVAL;
    }

    if ((out == NULL) || (out_len != sizeof(u32))) {
        dms_err("output arg is NULL, or out_len is wrong. (out_len=%u)\n", out_len);
        return -EINVAL;
    }

    dev_id = *(unsigned int *)in;

    ret = dms_trans_and_check_id(dev_id, &phy_id, &vfid);
    if (ret != 0) {
        dms_err("can't transfor virt id. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }
#if defined(CFG_HOST_ENV) && defined(CFG_FEATURE_VFIO)
    *(unsigned int *)out = vmng_get_device_split_mode(phy_id);
#else
    #ifdef CFG_FEATURE_VFIO_SOC
    *(unsigned int *)out = vmng_get_device_split_mode(phy_id);
    #else
    *(unsigned int *)out = VMNG_NORMAL_NONE_SPLIT_MODE;
    #endif
#endif
    return 0;
}

STATIC int dms_soc_get_pcie_info(void *feature, char *in, u32 in_len,
    char *out, u32 out_len)
{
    struct dmanage_pcie_id_info pcie_id_info = {0};
    struct devdrv_pcie_id_info id_info = {0};
    unsigned int dev_id, virt_id, vfid;
    int ret;

    if ((in == NULL) || (in_len != sizeof(u32))) {
        dms_err("input arg is NULL, or in_len is wrong. (in_len=%u)\n", in_len);
        return -EINVAL;
    }

    if ((out == NULL) || (out_len != sizeof(struct dmanage_pcie_id_info))) {
        dms_err("output arg is NULL, or out_len is wrong. (out_len=%u)\n", out_len);
        return -EINVAL;
    }

    ret = memcpy_s((void *)&virt_id, sizeof(u32), (void *)in, in_len);
    if (ret != 0) {
        dms_err("call memcpy_s failed. (ret=%d)\n", ret);
        return ret;
    }

    ret = dms_trans_and_check_id(virt_id, &dev_id, &vfid);
    if (ret != 0) {
        dms_err("can't transfor virt id. (virt_id=%u; ret=%d)\n", virt_id, ret);
        return ret;
    }

    ret = devdrv_get_pcie_id_info(dev_id, &id_info);
    if (ret != 0) {
        dms_err("get pcie id info failed. (ret=%d)\n", ret);
        return ret;
    }

    pcie_id_info.venderid = id_info.venderid;
    pcie_id_info.subvenderid = id_info.subvenderid;
    pcie_id_info.deviceid = id_info.deviceid;
    pcie_id_info.subdeviceid = id_info.subdeviceid;
    pcie_id_info.bus = id_info.bus;
    pcie_id_info.device = id_info.device;
    pcie_id_info.fn = id_info.fn;

    ret = memcpy_s((void *)out, out_len, (void *)&pcie_id_info, sizeof(struct dmanage_pcie_id_info));
    if (ret != 0) {
        dms_err("call memcpys_s failed. (ret=%d)\n", ret);
        return ret;
    }

    return 0;
}

#define HCCS_SWITCH_BASE_OFFSET         0x200000000000ULL /* 32T */
#define HCCS_SWITCH_EACH_CHIP_OFFSET    0x20000000000ULL /* 2T */
#define PCIE_CONNECT_TYPE               0
#define HCCS_CONNECT_TYPE               1

int dms_get_dev_phy_base_addr(struct devdrv_info *dev_info, u64 *base_addr)
{
    u64 phy_base_addr = 0;
    u64 chip_base_addr = CHIP_BASEADDR_PA_OFFSET; /* 8T */

    if (dev_info == NULL || base_addr == NULL) {
        dms_err("Invalid parameter. (dev_info_is_null=%d; base_addr_is_NULL=%d)\n",
            (dev_info == NULL), (base_addr == NULL));
        return -EINVAL;
    }

    /* If the connect_type is hccs, add 32T. */
    if (dev_info->connect_type == HCCS_CONNECT_TYPE) {
        phy_base_addr = HCCS_SWITCH_BASE_OFFSET; /* 32T */
        chip_base_addr = HCCS_SWITCH_EACH_CHIP_OFFSET; /* 2T */
    }

    *base_addr = phy_base_addr +
        dev_info->chip_id * chip_base_addr + dev_info->die_id * DIE_BASEADDR_PA_OFFSET;

    return 0;
}

int hal_kernel_get_hardware_info(unsigned int phy_id, devdrv_hardware_info_t *hardware_info)
{
    int ret = 0;
    struct devdrv_info *dev_info = NULL;

    if (hardware_info == NULL) {
        dms_err("Invalid parameter. (hardware_info_is_null=%d)\n", hardware_info == NULL);
        return -EINVAL;
    }

    dev_info = devdrv_manager_get_devdrv_info(phy_id);
    if (dev_info == NULL) {
        dms_err("Failed to invoke devdrv_manager_get_devdrv_info to get dev_info. (phy_id=%u)\n", phy_id);
        return -EINVAL;
    }

    hardware_info->base_hw_info.chip_id = dev_info->chip_id;
    hardware_info->base_hw_info.multi_chip = dev_info->multi_chip;
    hardware_info->base_hw_info.multi_die = dev_info->multi_die;
    hardware_info->base_hw_info.mainboard_id = dev_info->mainboard_id;
    hardware_info->base_hw_info.connect_type = dev_info->connect_type;
    hardware_info->base_hw_info.board_id = dev_info->board_id;
    ret = dms_get_dev_phy_base_addr(dev_info, &hardware_info->phy_addr_offset);
    if (ret != 0) {
        dms_err("Failed to get device base offset. (phy_id=%u; ret=%d)\n", phy_id, ret);
        return -EINVAL;
    }

    return 0;
}
EXPORT_SYMBOL(hal_kernel_get_hardware_info);

#endif
STATIC s32 dms_get_gpio_status(void *feature, char *in,
    u32 in_len, char *out, u32 out_len)
{
    struct dms_get_gpio *arg = (struct dms_get_gpio *)in;
    unsigned int *status = (unsigned int *)out;
    unsigned int gpio_val;
    unsigned int gpio_num;
    int ret;

    if ((in == NULL) || (in_len != sizeof(struct dms_get_gpio)) ||
        (out == NULL) || (out_len != sizeof(unsigned int))) {
        dms_err("Invalid para. (in=%s; in_len=%u; out=%s; out_len=%u)\n",
            (in == NULL) ? "NULL" : "OK", in_len, (out == NULL) ? "NULL" : "OK", out_len);
        return -EINVAL;
    }
    if (arg->dev_id >= DEVICE_NUM_MAX) {
        dms_err("Invalid device id. (dev_id=%u)\n", arg->dev_id);
        return -EINVAL;
    }
    gpio_num = arg->gpio_num;
    if (!gpio_is_valid(gpio_num)) {
        dms_err("Invalid GPIO. (num=%u)\n", gpio_num);
        return -EINVAL;
    }

    ret = gpio_request(gpio_num, DEVDRV_GPIO_NAME);
    if (ret) {
        dms_err("GPIO request failed. (num=%u)\n", gpio_num);
        return ret;
    }

    gpio_val = gpio_get_value(gpio_num);

    (void)gpio_free(gpio_num);

    *status = gpio_val;
    return 0;
}

#ifdef CFG_TRS_HB_REFACTOR_FEATURE
STATIC int dms_get_device_init_status(void *feature, char *in, u32 in_len, char *out, u32 out_len)
{
    struct devdrv_manager_info *manager_info = devdrv_get_manager_info();
    u32 logic_id, ret;
    u32 init_status = DRV_STATUS_INITING;
    u32 phys_id = DEVDRV_MAX_DAVINCI_NUM;
    u32 vf_id = 0;

    if ((in == NULL) || (in_len != sizeof(u32)) || (out == NULL) || (out_len != sizeof(u32))) {
        dms_err("Invalid parameter. (in=%s; in_len=%u; out=%s; out_len=%u)\n",
            (in == NULL) ? "NULL" : "OK", in_len, (out == NULL) ? "NULL" : "OK", out_len);
        return -EINVAL;
    }

    logic_id = *(u32 *)in;
    ret = devdrv_manager_container_logical_id_to_physical_id(logic_id, &phys_id, &vf_id);
    if (ret != 0) {
        devdrv_drv_err("Failed to transfer logical ID to physical ID. (logic_id=%u; ret=%d)\n", logic_id, ret);
        return ret;
    }

    if (manager_info == NULL || manager_info->dev_info[phys_id] == NULL) {
        init_status = DRV_STATUS_INITING;
#ifdef CFG_HOST_ENV
    } else if (manager_info->device_status[phys_id] == DRV_STATUS_COMMUNICATION_LOST) {
        init_status = DRV_STATUS_COMMUNICATION_LOST;
#endif
    } else {
        /* TS heartbeat needs to be checked to determine if the device is normal. */
        init_status = DRV_STATUS_WORK;
    }

    *(u32 *)out = init_status;

    return 0;
}
#endif

BEGIN_DMS_MODULE_DECLARATION(DMS_MODULE_BASIC_INFO)
BEGIN_FEATURE_COMMAND()
#ifndef AOS_LLVM_BUILD
ADD_FEATURE_COMMAND(DMS_MODULE_BASIC_INFO,
    DMS_MAIN_CMD_BASIC,
    DMS_SUBCMD_GET_PCIE_INFO,
    NULL,
    NULL,
    DMS_SUPPORT_ALL,
    dms_soc_get_pcie_info)
ADD_FEATURE_COMMAND(DMS_MODULE_BASIC_INFO,
    DMS_GET_CHIP_INFO_CMD,
    ZERO_CMD,
    NULL,
    NULL,
    DMS_SUPPORT_ALL,
    dms_feature_get_chip_info)
#ifdef CFG_FEATURE_REBOOT_REASON
ADD_FEATURE_COMMAND(DMS_MODULE_BASIC_INFO,
    DMS_GET_GET_REBOOT_REASON_CMD,
    ZERO_CMD,
    NULL,
    NULL,
    DMS_SUPPORT_ALL,
    dms_get_reboot_reason)
#endif
#ifdef CFG_SOC_PLATFORM_MDC_V51
ADD_FEATURE_COMMAND(DMS_MODULE_BASIC_INFO,
    DMS_GET_BIST_INFO_CMD,
    ZERO_CMD,
    NULL,
    NULL,
    DMS_SUPPORT_ALL,
    dms_get_bist_info)
ADD_FEATURE_COMMAND(DMS_MODULE_BASIC_INFO,
    DMS_SET_BIST_INFO_CMD,
    DMS_BIST_CMD_SET_MODE,
    NULL,
    "dmp_daemon",
    DMS_SUPPORT_ALL,
    dms_set_bist_mode)
#endif
ADD_FEATURE_COMMAND(DMS_MODULE_BASIC_INFO,
    DMS_MAIN_CMD_BASIC,
    DMS_SUBCMD_GET_DEV_SPLIT_MODE,
    NULL,
    NULL,
    DMS_SUPPORT_ALL,
    dms_get_device_split_mode)
#endif
ADD_FEATURE_COMMAND(DMS_MODULE_BASIC_INFO,
    DMS_GET_GET_GPIO_STATUS_CMD,
    ZERO_CMD,
    NULL,
    NULL,
    DMS_SUPPORT_ROOT_ONLY,
    dms_get_gpio_status)
#ifdef AOS_LLVM_BUILD
ADD_FEATURE_COMMAND(DMS_MODULE_BASIC_INFO,
    DMS_BIND_HOST_PID_CMD,
    ZERO_CMD,
    NULL,
    NULL,
    DMS_SUPPORT_ALL,
    dms_drv_bind_host_pid)
ADD_FEATURE_COMMAND(DMS_MODULE_BASIC_INFO,
    DMS_UNBIND_HOST_PID_CMD,
    ZERO_CMD,
    NULL,
    NULL,
    DMS_SUPPORT_ALL,
    dms_drv_unbind_host_pid)
ADD_FEATURE_COMMAND(DMS_MODULE_BASIC_INFO,
    DMS_QUERY_DEV_PID_CMD,
    ZERO_CMD,
    NULL,
    NULL,
    DMS_SUPPORT_ALL,
    dms_drv_query_devpid_by_hostpid)
ADD_FEATURE_COMMAND(DMS_MODULE_BASIC_INFO,
    DMS_QUERY_HOST_PID_CMD,
    ZERO_CMD,
    NULL,
    NULL,
    DMS_SUPPORT_ALL,
    dms_drv_query_hostpid_by_devpid)
ADD_FEATURE_COMMAND(DMS_MODULE_BASIC_INFO,
    DMS_GET_PROCESS_SIGN,
    ZERO_CMD,
    NULL,
    NULL,
    DMS_SUPPORT_ALL,
    dms_drv_get_process_sign)
ADD_FEATURE_COMMAND(DMS_MODULE_BASIC_INFO,
    DMS_MAIN_CMD_BASIC,
    DMS_SUBCMD_GET_HCCL_DEVICE_INFO,
    NULL,
    NULL,
    DMS_SUPPORT_ALL,
    dms_get_hccl_device_info)
ADD_FEATURE_COMMAND(DMS_MODULE_BASIC_INFO,
    DMS_MAIN_CMD_BASIC,
    DMS_SUBCMD_GET_CORE_SPEC_INFO,
    NULL,
    NULL,
    DMS_SUPPORT_ALL,
    dms_get_core_spec_info)
ADD_FEATURE_COMMAND(DMS_MODULE_BASIC_INFO,
    DMS_MAIN_CMD_BASIC,
    DMS_SUBCMD_GET_CORE_INUSE_INFO,
    NULL,
    NULL,
    DMS_SUPPORT_ALL,
    dms_get_core_inuse_info)
#endif
#ifdef SUPPORT_ASCEND_PM
ADD_FEATURE_COMMAND(DMS_MODULE_BASIC_INFO,
    DMS_GET_SET_POWER_STATE_CMD,
    ZERO_CMD,
    NULL,
    "dmp_daemon",
    DMS_SUPPORT_ROOT_ONLY,
    dms_set_power_state)
#endif
#ifdef SUPPORT_VDEV_MEM
ADD_FEATURE_COMMAND(DMS_MODULE_BASIC_INFO,
    DMS_MAIN_CMD_BASIC,
    DMS_GET_VDEVICE_INFO,
    NULL,
    "dmp_daemon",
    DMS_SUPPORT_ALL,
    dms_drv_get_vdevice_info)
#endif
#ifdef CFG_FEATURE_DEV_TOPOLOGY
ADD_FEATURE_COMMAND(DMS_MODULE_BASIC_INFO,
    DMS_MAIN_CMD_BASIC,
    DMS_SUBCMD_GET_DEV_TOPOLOGY,
    NULL,
    NULL,
    DMS_SUPPORT_ALL,
    dms_feature_get_dev_topology)

ADD_FEATURE_COMMAND(DMS_MODULE_BASIC_INFO,
    DMS_MAIN_CMD_BASIC,
    DMS_SUBCMD_GET_DEVICES_TOPOLOGY,
    NULL,
    NULL,
    DMS_SUPPORT_ALL,
    dms_feature_get_phy_devices_topology)

#endif
#if defined(CFG_HOST_ENV) && defined(CFG_FEATURE_SRIOV)
ADD_FEATURE_COMMAND(DMS_MODULE_BASIC_INFO,
    DMS_MAIN_CMD_BASIC,
    DMS_SUBCMD_SRIOV_SWITCH,
    NULL,
    NULL,
    DMS_SUPPORT_ROOT_PHY,
    dms_feature_set_sriov_switch)
#endif
#ifndef CFG_HOST_ENV
ADD_FEATURE_COMMAND(DMS_MODULE_BASIC_INFO,
    DMS_MAIN_CMD_BASIC,
    DMS_SET_TIME_ZONE_SYNC,
    NULL,
    "dmp_daemon",
    DMS_SUPPORT_ALL,
    dms_process_time_sync)
#endif
#ifdef CFG_TRS_HB_REFACTOR_FEATURE
ADD_FEATURE_COMMAND(DMS_MODULE_BASIC_INFO,
    DMS_MAIN_CMD_BASIC,
    DMS_SUBCMD_GET_DEV_INIT_STATUS,
    NULL,
    NULL,
    DMS_SUPPORT_ALL,
    dms_get_device_init_status)
#endif
END_FEATURE_COMMAND()
END_MODULE_DECLARATION()