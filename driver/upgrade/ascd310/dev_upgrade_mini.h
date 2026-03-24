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

#ifndef DEV_UPGRADE_MINI_H
#define DEV_UPGRADE_MINI_H
#include "dev_upgrade_public.h"
#include "dev_upgrade_adapt.h"


component_area_info g_component_area_info_chip0[] = {
    /*  组件类型Id,
        flash区域标识,
        组件对应分区设备,
        分区名称,
        在Flash中的存储地址,
        存储空间大小
    */
    {
        DSMI_COMPONENT_TYPE_NVE,
        FLASH_AREA_0,
        {0},
        PART_NAME_NVE,
        FLASH_ADDR_OFFSET_NVE_0,
        FLASH_SIZE_NVE_0,
        STORE_IN_FLASH
    },
    {
        DSMI_COMPONENT_TYPE_XLOADER,
        FLASH_AREA_0,
        {0},
        PART_NAME_XLOADER_1,
        FLASH_ADDR_OFFSET_XLOADER_0,
        FLASH_SIZE_XLOADER_0,
        STORE_IN_FLASH
    },
    {
        DSMI_COMPONENT_TYPE_XLOADER,
        FLASH_AREA_1,
        {0},
        PART_NAME_XLOADER_2,
        FLASH_ADDR_OFFSET_XLOADER_1,
        FLASH_SIZE_XLOADER_1,
        STORE_IN_FLASH
    },
    {
        DSMI_COMPONENT_TYPE_UEFI,
        FLASH_AREA_0,
        {0},
        PART_NAME_UEFI,
        FLASH_ADDR_OFFSET_UEFI_0,
        FLASH_SIZE_UEFI_0,
        STORE_IN_FLASH
    },
    {
        DSMI_COMPONENT_TYPE_UEFI,
        FLASH_AREA_1,
        {0},
        PART_NAME_UEFI_2,
        FLASH_ADDR_OFFSET_UEFI_1,
        FLASH_SIZE_UEFI_1,
        STORE_IN_FLASH
    },
    {
        DSMI_COMPONENT_TYPE_M3FW,
        FLASH_AREA_0,
        {0},
        PART_NAME_M3FW,
        FLASH_ADDR_OFFSET_M3_FW_0,
        FLASH_SIZE_M3_FW_0,
        STORE_IN_FLASH
    },
    {
        DSMI_COMPONENT_TYPE_TEE,
        FLASH_AREA_0,
        {0},
        PART_NAME_TEE,
        FLASH_ADDR_OFFSET_TEE_0,
        FLASH_SIZE_TEE_0,
        STORE_IN_FLASH
    },
    {
        DSMI_COMPONENT_TYPE_KERNEL,
        FLASH_AREA_0,
        {0},
        PART_NAME_KERNEL,
        FLASH_ADDR_OFFSET_KERNEL_0,
        FLASH_SIZE_CHECK_IGNORE,
        STORE_IN_FLASH
    },
    {
        DSMI_COMPONENT_TYPE_DTB,
        FLASH_AREA_0,
        {0},
        PART_NAME_DTB,
        FLASH_ADDR_OFFSET_DTB_0,
        FLASH_SIZE_DTB_0,
        STORE_IN_FLASH
    },
    {
        DSMI_COMPONENT_TYPE_ROOTFS,
        FLASH_AREA_0,
        {0},
        PART_NAME_ROOTFS,
        FLASH_ADDR_OFFSET_ROOTFS_0,
        FLASH_SIZE_ROOTFS_0,
        STORE_IN_FLASH
    },
    {
        DSMI_COMPONENT_TYPE_BOOTROM,
        FLASH_AREA_0,
        {0},
        PART_NAME_BOOTROM,
        FLASH_ADDR_OFFSET_BOOTROM_0,
        FLASH_SIZE_BOOTROM_0,
        STORE_IN_FLASH
    },
    {
        DSMI_COMPONENT_TYPE_MAX, /* 最后一个空文件，代表结束 */
        0,
        {0},
        "Null",
        0,
        0,
        STORE_IN_FLASH
    }
};


#endif /* _DEV_UPGRADE_MINI_H_ */
