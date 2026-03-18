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

#ifndef SOC_MISC_CHIP_H
#define SOC_MISC_CHIP_H

typedef struct soc_chip_ver_reg {
    unsigned int chip_ver : 12;
    unsigned int chip_name : 16;
    unsigned int reserved : 4;
} soc_chip_ver_reg_t;

/* Board id */
#define DEV_NODE_COMPATIBLE_STR  "hisilicon,mini"

/* Die id */
#define SYS_CTRL_BASE_ADDR 0x01100CE000UL
#define SYS_CTRL_SC_DIE_ID0_OFFSET 0x200
#define SYS_CTRL_SC_DIE_ID_REG_OFFSET 0x4
#define SOC_DIEID_MAP_SIZE 0x2000UL
#define PMU_DIEID_GET_VALUE 8

/* Chip info */
#define SOC_CHIP_OFFSET 0x0UL
#define SOC_DIE_OFFSET 0x0UL
#define SOC_CHIP_INFO_REG_BASE 0x10015e000UL
#define SOC_CHIP_INFO_REG_OFFSET 0X1C
#define SOC_CHIP_INFO_MAP_SIZE 0x1000
#endif
