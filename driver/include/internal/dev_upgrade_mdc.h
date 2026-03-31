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

#ifndef _DEV_UPGRADE_MDC_H_
#define _DEV_UPGRADE_MDC_H_
#include "dev_upgrade_public.h"
#include "dev_upgrade_adapt.h"

component_area_info g_component_area_info_chip0[] = {
    {   DSMI_COMPONENT_TYPE_HBOOT1_A,
        FLASH_AREA_0,
        { 0  },
        PART_NAME_HBOOT1_A_0,
        FLASH_ADDR_OFFSET_HBOOT1_A_0,
        FLASH_SIZE_HBOOT1_A_0,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_HBOOT1_A,
        FLASH_AREA_1,
        { 0  },
        PART_NAME_HBOOT1_A_1,
        FLASH_ADDR_OFFSET_HBOOT1_A_1,
        FLASH_SIZE_HBOOT1_A_1,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_HBOOT1_B,
        FLASH_AREA_0,
        { 0  },
        PART_NAME_HBOOT1_B_0,
        FLASH_ADDR_OFFSET_HBOOT1_B_0,
        FLASH_SIZE_HBOOT1_B_0,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_HBOOT1_B,
        FLASH_AREA_1,
        { 0  },
        PART_NAME_HBOOT1_B_1,
        FLASH_ADDR_OFFSET_HBOOT1_B_1,
        FLASH_SIZE_HBOOT1_B_1,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_HBOOT2,
        FLASH_AREA_0,
        { 0  },
        PART_NAME_HBOOT2_0,
        FLASH_ADDR_OFFSET_HBOOT2_0,
        FLASH_SIZE_HBOOT2_0,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_HBOOT2,
        FLASH_AREA_1,
        { 0  },
        PART_NAME_HBOOT2_1,
        FLASH_ADDR_OFFSET_HBOOT2_1,
        FLASH_SIZE_HBOOT2_1,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_DDR,
        FLASH_AREA_0,
        { 0  },
        PART_NAME_DDR_0,
        FLASH_ADDR_OFFSET_DDR_0,
        FLASH_SIZE_DDR_0,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_DDR,
        FLASH_AREA_1,
        { 0  },
        PART_NAME_DDR_1,
        FLASH_ADDR_OFFSET_DDR_1,
        FLASH_SIZE_DDR_1,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_LP,
        FLASH_AREA_0,
        { 0  },
        PART_NAME_LP_0,
        FLASH_ADDR_OFFSET_LP_0,
        FLASH_SIZE_LP_0,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_LP,
        FLASH_AREA_1,
        { 0  },
        PART_NAME_LP_1,
        FLASH_ADDR_OFFSET_LP_1,
        FLASH_SIZE_LP_1,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_HSM,
        FLASH_AREA_0,
        { 0  },
        PART_NAME_HSM_0,
        FLASH_ADDR_OFFSET_HSM_0,
        FLASH_SIZE_HSM_0,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_HSM,
        FLASH_AREA_1,
        { 0  },
        PART_NAME_HSM_1,
        FLASH_ADDR_OFFSET_HSM_1,
        FLASH_SIZE_HSM_1,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_SAFETY_ISLAND,
        FLASH_AREA_0,
        { 0  },
        PART_NAME_SAFETY_ISLAND_0,
        FLASH_ADDR_OFFSET_SAFETY_ISLAND_0,
        FLASH_SIZE_SAFETY_ISLAND_0,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_SAFETY_ISLAND,
        FLASH_AREA_1,
        { 0  },
        PART_NAME_SAFETY_ISLAND_1,
        FLASH_ADDR_OFFSET_SAFETY_ISLAND_1,
        FLASH_SIZE_SAFETY_ISLAND_1,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_HILINK,
        FLASH_AREA_0,
        { 0 },
        PART_NAME_HLINK_0,
        FLASH_ADDR_OFFSET_HLINK,
        FLASH_SIZE_HLINK,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_HILINK,
        FLASH_AREA_1,
        { 0 },
        PART_NAME_HLINK_1,
        FLASH_ADDR_OFFSET_HLINK,
        FLASH_SIZE_HLINK,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_SYS_BASE_CONFIG,
        FLASH_AREA_0,
        { 0 },
        PART_NAME_SYS_BASE_CONFIG_0,
        FLASH_ADDR_OFFSET_SYS_BASE_CONFIG_0,
        FLASH_SIZE_SYS_BASE_CONFIG_0,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_SYS_BASE_CONFIG,
        FLASH_AREA_1,
        { 0 },
        PART_NAME_SYS_BASE_CONFIG_1,
        FLASH_ADDR_OFFSET_SYS_BASE_CONFIG_1,
        FLASH_SIZE_SYS_BASE_CONFIG_1,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_LOGIC_BIST,
        FLASH_AREA_0,
        { 0 },
        PART_NAME_LOGIC_BIST_0,
        FLASH_ADDR_OFFSET_LOGIC_BIST_0,
        FLASH_SIZE_LOGIC_BIST_0,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_LOGIC_BIST,
        FLASH_AREA_1,
        { 0 },
        PART_NAME_LOGIC_BIST_1,
        FLASH_ADDR_OFFSET_LOGIC_BIST_1,
        FLASH_SIZE_LOGIC_BIST_1,
        STORE_IN_FLASH
    },
#ifdef CFG_SOC_PLATFORM_MDC_V11
    {   DSMI_COMPONENT_TYPE_ATF,
        FLASH_AREA_0,
        { 0 },
        PART_NAME_ATF_0,
        FLASH_ADDR_OFFSET_ATF_0,
        FLASH_SIZE_ATF_0,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_ATF,
        FLASH_AREA_1,
        { 0 },
        PART_NAME_ATF_1,
        FLASH_ADDR_OFFSET_ATF_1,
        FLASH_SIZE_ATF_1,
        STORE_IN_FLASH
    },
#endif
    {   DSMI_COMPONENT_TYPE_MAX,
        0,
        { 0  },
        "Null",
        0,
        0,
        STORE_IN_FLASH
    }

};

#endif /* _DEV_UPGRADE_MDC_H_ */
