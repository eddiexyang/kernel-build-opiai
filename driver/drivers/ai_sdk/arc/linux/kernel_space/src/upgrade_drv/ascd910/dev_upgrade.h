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

#ifndef _DEV_UPGRADE_H_
#define _DEV_UPGRADE_H_
#include "dev_upgrade_public.h"
#include "dev_upgrade_adapt.h"

component_area_info g_component_area_info_chip0[] = {
    {
        DSMI_COMPONENT_TYPE_NVE,
        FLASH_AREA_0,
        { 0 },
        PART_NAME_NVE_P0,
        FLASH_ADDR_OFFSET_NVE_0,
        FLASH_SIZE_NVE_0,
        STORE_IN_FLASH
    },
    {
        DSMI_COMPONENT_TYPE_NVE,
        FLASH_AREA_1,
        { 0 },
        PART_NAME_NVE_P0,
        FLASH_ADDR_OFFSET_NVE_1,
        FLASH_SIZE_NVE_1,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_UEFI,
        FLASH_AREA_0,
        { 0 },
        PART_NAME_UEFI_1_P0,
        FLASH_ADDR_OFFSET_UEFI_0,
        FLASH_SIZE_UEFI_0,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_UEFI,
        FLASH_AREA_1,
        { 0 },
        PART_NAME_UEFI_2_P0,
        FLASH_ADDR_OFFSET_UEFI_1,
        FLASH_SIZE_UEFI_1,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_IMU,
        FLASH_AREA_0,
        { 0 },
        PART_NAME_IMU_1_P0,
        FLASH_ADDR_OFFSET_IMU_0,
        FLASH_SIZE_IMU_0,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_IMU,
        FLASH_AREA_1,
        { 0 },
        PART_NAME_IMU_2_P0,
        FLASH_ADDR_OFFSET_IMU_1,
        FLASH_SIZE_IMU_1,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_IMP,
        FLASH_AREA_0,
        { 0 },
        PART_NAME_IMP_1_P0,
        FLASH_ADDR_OFFSET_IMP_0,
        FLASH_SIZE_IMP_0,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_IMP,
        FLASH_AREA_1,
        { 0 },
        PART_NAME_IMP_2_P0,
        FLASH_ADDR_OFFSET_IMP_1,
        FLASH_SIZE_IMP_1,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_MAX,
        0,
        { 0 },
        "Null",
        0,
        0,
        STORE_IN_FLASH
    }
};

component_area_info g_component_area_info_chip1[] = {
    {
        DSMI_COMPONENT_TYPE_NVE,
        FLASH_AREA_0,
        { 0 },
        PART_NAME_NVE_P1,
        FLASH_ADDR_OFFSET_NVE_0,
        FLASH_SIZE_NVE_0,
        STORE_IN_FLASH
    },
    {
        DSMI_COMPONENT_TYPE_NVE,
        FLASH_AREA_1,
        { 0 },
        PART_NAME_NVE_P1,
        FLASH_ADDR_OFFSET_NVE_1,
        FLASH_SIZE_NVE_1,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_UEFI,
        FLASH_AREA_0,
        { 0 },
        PART_NAME_UEFI_1_P1,
        FLASH_ADDR_OFFSET_UEFI_0,
        FLASH_SIZE_UEFI_0,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_UEFI,
        FLASH_AREA_1,
        { 0 },
        PART_NAME_UEFI_2_P1,
        FLASH_ADDR_OFFSET_UEFI_1,
        FLASH_SIZE_UEFI_1,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_IMU,
        FLASH_AREA_0,
        { 0 },
        PART_NAME_IMU_1_P1,
        FLASH_ADDR_OFFSET_IMU_0,
        FLASH_SIZE_IMU_0,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_IMU,
        FLASH_AREA_1,
        { 0 },
        PART_NAME_IMU_2_P1,
        FLASH_ADDR_OFFSET_IMU_1,
        FLASH_SIZE_IMU_1,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_IMP,
        FLASH_AREA_0,
        { 0 },
        PART_NAME_IMP_1_P1,
        FLASH_ADDR_OFFSET_IMP_0,
        FLASH_SIZE_IMP_0,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_IMP,
        FLASH_AREA_1,
        { 0 },
        PART_NAME_IMP_2_P1,
        FLASH_ADDR_OFFSET_IMP_1,
        FLASH_SIZE_IMP_1,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_MAX,
        0,
        { 0 },
        "Null",
        0,
        0,
        STORE_IN_FLASH
    }
};

component_area_info g_component_area_info_chip2[] = {
    {
        DSMI_COMPONENT_TYPE_NVE,
        FLASH_AREA_0,
        { 0 },
        PART_NAME_NVE_P2,
        FLASH_ADDR_OFFSET_NVE_0,
        FLASH_SIZE_NVE_0,
        STORE_IN_FLASH
    },
    {
        DSMI_COMPONENT_TYPE_NVE,
        FLASH_AREA_1,
        { 0 },
        PART_NAME_NVE_P2,
        FLASH_ADDR_OFFSET_NVE_1,
        FLASH_SIZE_NVE_1,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_UEFI,
        FLASH_AREA_0,
        { 0 },
        PART_NAME_UEFI_1_P2,
        FLASH_ADDR_OFFSET_UEFI_0,
        FLASH_SIZE_UEFI_0,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_UEFI,
        FLASH_AREA_1,
        { 0 },
        PART_NAME_UEFI_2_P2,
        FLASH_ADDR_OFFSET_UEFI_0,
        FLASH_SIZE_UEFI_0,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_IMU,
        FLASH_AREA_0,
        { 0 },
        PART_NAME_IMU_1_P2,
        FLASH_ADDR_OFFSET_IMU_0,
        FLASH_SIZE_IMU_0,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_IMU,
        FLASH_AREA_1,
        { 0 },
        PART_NAME_IMU_2_P2,
        FLASH_ADDR_OFFSET_IMU_1,
        FLASH_SIZE_IMU_1,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_IMP,
        FLASH_AREA_0,
        { 0 },
        PART_NAME_IMP_1_P2,
        FLASH_ADDR_OFFSET_IMP_0,
        FLASH_SIZE_IMP_0,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_IMP,
        FLASH_AREA_1,
        { 0 },
        PART_NAME_IMP_2_P2,
        FLASH_ADDR_OFFSET_IMP_1,
        FLASH_SIZE_IMP_1,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_MAX,
        0,
        { 0 },
        "Null",
        0,
        0,
        STORE_IN_FLASH
    }
};

component_area_info g_component_area_info_chip3[] = {
    {
        DSMI_COMPONENT_TYPE_NVE,
        FLASH_AREA_0,
        { 0 },
        PART_NAME_NVE_P3,
        FLASH_ADDR_OFFSET_NVE_0,
        FLASH_SIZE_NVE_0,
        STORE_IN_FLASH
    },
    {
        DSMI_COMPONENT_TYPE_NVE,
        FLASH_AREA_1,
        { 0 },
        PART_NAME_NVE_P3,
        FLASH_ADDR_OFFSET_NVE_1,
        FLASH_SIZE_NVE_1,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_UEFI,
        FLASH_AREA_0,
        { 0 },
        PART_NAME_UEFI_1_P3,
        FLASH_ADDR_OFFSET_UEFI_0,
        FLASH_SIZE_UEFI_0,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_UEFI,
        FLASH_AREA_1,
        { 0 },
        PART_NAME_UEFI_2_P3,
        FLASH_ADDR_OFFSET_UEFI_0,
        FLASH_SIZE_UEFI_0,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_IMU,
        FLASH_AREA_0,
        { 0 },
        PART_NAME_IMU_1_P3,
        FLASH_ADDR_OFFSET_IMU_0,
        FLASH_SIZE_IMU_0,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_IMU,
        FLASH_AREA_1,
        { 0 },
        PART_NAME_IMU_2_P3,
        FLASH_ADDR_OFFSET_IMU_1,
        FLASH_SIZE_IMU_1,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_IMP,
        FLASH_AREA_0,
        { 0 },
        PART_NAME_IMP_1_P3,
        FLASH_ADDR_OFFSET_TEE_0,
        FLASH_SIZE_IMP_0,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_IMP,
        FLASH_AREA_1,
        { 0 },
        PART_NAME_IMP_2_P3,
        FLASH_ADDR_OFFSET_IMP_1,
        FLASH_SIZE_IMP_1,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_MAX,
        0,
        { 0 },
        "Null",
        0,
        0,
        STORE_IN_FLASH
    }
};
#endif /* _DEV_UPGRADE_H_ */
