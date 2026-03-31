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
    unsigned int chip_ver : 4;
    unsigned int chip_name : 16;
    unsigned int reserved : 12;
} soc_chip_ver_reg_t;

#define SOC_CHIP_OFFSET 0x80000000000UL
#define SOC_DIE_OFFSET  0x10000000000UL

/* Die id */
#ifdef CFG_FEATURE_ESL_NOT_SUPPORT
#define DIE_ID_ADDR 0x80004600UL /* stub using broadcast_die*_addr* register */
#else
#define DIE_ID_ADDR 0x703B4E200UL
#endif

#define N_DIE_ID_ADDR DIE_ID_ADDR
#define DIE_ID_NUM 5

/* Chip info */
#define SOC_CHIP_INFO_REG_BASE 0x8000F000UL
#define SOC_CHIP_INFO_REG_OFFSET 0xFF8
#define SOC_CHIP_INFO_MAP_SIZE 0x1000

/* PCIe reg info */
#define PCIE_RAS_APB_SLAVE_BASE_ADDR 0x801000000ULL

/* Board id */
#define DEV_NODE_COMPATIBLE_STR  "hisilicon,ascend920"
static inline bool soc_misc_is_pcie_card(unsigned int board_id)
{
    /* Board id rules: bit[7:0] are valid;
     * Expect bit[7:4]: 0x3 for 910B POD, 0x5 for 910B Dual-P module.
     */
    return ((((board_id) >> 8u) == 0) && (((board_id) >> 4u) != 0x3) && (((board_id) >> 4u) != 0x5));
}

#endif /* end of __SOC_MISC_CHIP_H__ */
