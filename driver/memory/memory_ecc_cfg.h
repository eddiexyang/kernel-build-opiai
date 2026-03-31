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

#ifndef DRV_MEMORY_ECC_CFG_H
#define DRV_MEMORY_ECC_CFG_H

#define ECC_ERROR_TYPE 0
#define ECC_RESULT_NUM 2
#define SINGLE_BIT_INDEX 0
#define DOUBLE_BIT_INDEX 1

#define OFFSET_EIGHT       8
#define OFFSET_SIXTEEN     16
#define OFFSET_TWENTY_FOUR 24

#define ECC_CORE_ID 9

#ifdef CFG_DDR_REG_ADDR_310
/* ecc statistics */
#define DDR_ECC_OFFSET1 0x4fc4
#define DDR_ECC_OFFSET2 0x4fc8
#endif

#ifdef CFG_DDR_REG_ADDR_710
/* ecc statistics */
#define DDR_ECC_OFFSET1 0xfc4
#define DDR_ECC_OFFSET2 0xfc8
#endif

#ifdef CFG_DDR_REG_ADDR_910
/* ecc statistics */
#define DDR_ECC_OFFSET1 0xfc4
#define DDR_ECC_OFFSET2 0xfc8
#endif

#ifdef CFG_MEMORY_DDR_INFO_FROM_REG
#define GET_ECC_STATISTISCS get_ddr_ecc_statistics_from_reg
#elif defined(CFG_MEMORY_DDR_INFO_FROM_SHAREMEM)
#define GET_ECC_STATISTISCS get_ddr_ecc_statistics_from_sharemem
#else
#define GET_ECC_STATISTISCS get_ecc_statistics_from_lp
#endif
#endif
