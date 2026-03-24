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

#define FLASH_SIZE_MINIV3_HBOOT1_A  0x20000
#define FLASH_SIZE_MINIV3_HLINK     0x20000
#define FLASH_SIZE_MINIV3_HSM       0x80000
#define FLASH_SIZE_MINIV3_DDR       0x50000
#define FLASH_SIZE_MINIV3_HBOOT2    0x400000
#define FLASH_SIZE_MINIV3_SYS_BASE_CONFIG   0x60000
#define FLASH_SIZE_MINIV3_USER_BASE_CONFIG   0x100000

#ifndef CFG_FEATURE_RC_MODE

component_area_info g_component_area_info_chip0[] = {
    {   DSMI_COMPONENT_TYPE_HBOOT1_A,
        FLASH_AREA_0,
        { 0  },
        PART_NAME_HBOOT1_A_0,
        FLASH_ADDR_OFFSET_HBOOT1_A_0,
        FLASH_SIZE_MINIV3_HBOOT1_A,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_HBOOT1_A,
        FLASH_AREA_1,
        { 0  },
        PART_NAME_HBOOT1_A_1,
        FLASH_ADDR_OFFSET_HBOOT1_A_1,
        FLASH_SIZE_MINIV3_HBOOT1_A,
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
    {   DSMI_COMPONENT_TYPE_HILINK,
        FLASH_AREA_0,
        { 0 },
        PART_NAME_HLINK_0,
        FLASH_ADDR_OFFSET_HLINK,
        FLASH_SIZE_MINIV3_HLINK,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_HILINK,
        FLASH_AREA_1,
        { 0 },
        PART_NAME_HLINK_1,
        FLASH_ADDR_OFFSET_HLINK,
        FLASH_SIZE_MINIV3_HLINK,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_MAX,
        0,
        { 0  },
        "Null",
        0,
        0,
        STORE_IN_FLASH
    }
};

#else

component_area_info g_component_area_info_chip0[] = {
    {   DSMI_COMPONENT_TYPE_HBOOT1_A,
        FLASH_AREA_0,
        { 0  },
        PART_NAME_HBOOT1_A_0,
        FLASH_ADDR_OFFSET_HBOOT1_A_0,
        FLASH_SIZE_MINIV3_HBOOT1_A,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_HBOOT1_A,
        FLASH_AREA_1,
        { 0  },
        PART_NAME_HBOOT1_A_1,
        FLASH_ADDR_OFFSET_HBOOT1_A_1,
        FLASH_SIZE_MINIV3_HBOOT1_A,
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
    {   DSMI_COMPONENT_TYPE_HILINK,
        FLASH_AREA_0,
        { 0 },
        PART_NAME_HLINK_0,
        FLASH_ADDR_OFFSET_HLINK,
        FLASH_SIZE_MINIV3_HLINK,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_HILINK,
        FLASH_AREA_1,
        { 0 },
        PART_NAME_HLINK_1,
        FLASH_ADDR_OFFSET_HLINK,
        FLASH_SIZE_MINIV3_HLINK,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_HSM,
        FLASH_AREA_0,
        { 0  },
        PART_NAME_HSM_0,
        FLASH_ADDR_OFFSET_HSM_0,
        FLASH_SIZE_MINIV3_HSM,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_HSM,
        FLASH_AREA_1,
        { 0  },
        PART_NAME_HSM_1,
        FLASH_ADDR_OFFSET_HSM_1,
        FLASH_SIZE_MINIV3_HSM,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_DDR,
        FLASH_AREA_0,
        { 0  },
        PART_NAME_DDR_0,
        FLASH_ADDR_OFFSET_DDR_0,
        FLASH_SIZE_MINIV3_DDR,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_DDR,
        FLASH_AREA_1,
        { 0  },
        PART_NAME_DDR_1,
        FLASH_ADDR_OFFSET_DDR_1,
        FLASH_SIZE_MINIV3_DDR,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_ATF,
        FLASH_AREA_0,
        { 0  },
        PART_NAME_ATF_0,
        FLASH_ADDR_OFFSET_ATF_0,
        FLASH_SIZE_ATF_0,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_ATF,
        FLASH_AREA_1,
        { 0  },
        PART_NAME_ATF_1,
        FLASH_ADDR_OFFSET_ATF_1,
        FLASH_SIZE_ATF_1,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_HBOOT2,
        FLASH_AREA_0,
        { 0  },
        PART_NAME_HBOOT2_0,
        FLASH_ADDR_OFFSET_HBOOT2_0,
        FLASH_SIZE_MINIV3_HBOOT2,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_HBOOT2,
        FLASH_AREA_1,
        { 0  },
        PART_NAME_HBOOT2_1,
        FLASH_ADDR_OFFSET_HBOOT2_1,
        FLASH_SIZE_MINIV3_HBOOT2,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_SYS_BASE_CONFIG,
        FLASH_AREA_0,
        { 0  },
        PART_NAME_SYS_BASE_CONFIG_0,
        FLASH_ADDR_OFFSET_SYS_BASE_CONFIG_0,
        FLASH_SIZE_MINIV3_SYS_BASE_CONFIG,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_SYS_BASE_CONFIG,
        FLASH_AREA_1,
        { 0  },
        PART_NAME_SYS_BASE_CONFIG_1,
        FLASH_ADDR_OFFSET_SYS_BASE_CONFIG_1,
        FLASH_SIZE_MINIV3_SYS_BASE_CONFIG,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_USER_BASE_CONFIG,
        FLASH_AREA_0,
        { 0  },
        PART_NAME_USER_BASE_CONFIG_0,
        FLASH_ADDR_OFFSET_USER_BASE_CONFIG_0,
        FLASH_SIZE_MINIV3_USER_BASE_CONFIG,
        STORE_IN_FLASH
    },
    {   DSMI_COMPONENT_TYPE_USER_BASE_CONFIG,
        FLASH_AREA_1,
        { 0  },
        PART_NAME_USER_BASE_CONFIG_1,
        FLASH_ADDR_OFFSET_USER_BASE_CONFIG_1,
        FLASH_SIZE_MINIV3_USER_BASE_CONFIG,
        STORE_IN_FLASH
    },

    {   DSMI_COMPONENT_TYPE_MAX,
        0,
        { 0  },
        "Null",
        0,
        0,
        STORE_IN_FLASH
    }
};

#endif /* CFG_FEATURE_RC_MODE */

#endif /* _DEV_UPGRADE_H_ */
