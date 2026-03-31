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

#ifndef MEM_BASE_INFO_H
#define MEM_BASE_INFO_H

#include "drv_log.h"
#include <linux/types.h>
#include "memory_fault_init_cfg.h"

#define MAX_OS_DEVICE_COUNT         4
#define MAX_HBM_STACK_PER_SOCKET    4
#define MAX_HBM_PC_PER_STACK        16

#define MAX_HBM_CONTROL_COUNT	   (MAX_HBM_STACK_PER_SOCKET * MAX_HBM_PC_PER_STACK)

#define HBM_HBMC_BASE_ADDR          0x40000
#define HBM_HBMC_PC_OFFSET          0x2000
#define RASC_HIS_CORR_CNT           0x118
#define SOCKET_OFFSET_ADDR          0x200000000000
#define HBMC0_REG_BASE              0x8c440000
#define HBMC1_REG_BASE              0x8c140000
#define HBMC2_REG_BASE              0x8c540000
#define HBMC3_REG_BASE              0x8c240000
#define SK0_HBM_STACK0_BASE_ADDR    0x8c400000
#define SK0_HBM_STACK1_BASE_ADDR    0x8c100000
#define SK0_HBM_STACK2_BASE_ADDR    0x8c500000
#define SK0_HBM_STACK3_BASE_ADDR    0x8c200000
#define SK1_HBM_STACK0_BASE_ADDR    0x20008c400000
#define SK1_HBM_STACK1_BASE_ADDR    0x20008c100000
#define SK1_HBM_STACK2_BASE_ADDR    0x20008c500000
#define SK1_HBM_STACK3_BASE_ADDR    0x20008c200000
#define SK2_HBM_STACK0_BASE_ADDR    0x40008c400000
#define SK2_HBM_STACK1_BASE_ADDR    0x40008c100000
#define SK2_HBM_STACK2_BASE_ADDR    0x40008c500000
#define SK2_HBM_STACK3_BASE_ADDR    0x40008c200000
#define SK3_HBM_STACK0_BASE_ADDR    0x60008c400000
#define SK3_HBM_STACK1_BASE_ADDR    0x60008c100000
#define SK3_HBM_STACK2_BASE_ADDR    0x60008c500000
#define SK3_HBM_STACK3_BASE_ADDR    0x60008c200000
#define HBM_HBMC_RASC_ADDR          0x1000
#define RASC_HIS_CORR_ADDR_L_ADDR   0x180
#define RASC_HIS_CORR_ADDR_H_ADDR   0x184
#define HBM_HBMC_ARER_ADDR          0x1800
#define ARER_ERR_CTLR_L_OFFSET      0x8

#define INHIBIT_INTERRUPT_VAL       0x41C

#define CHIP_BASE_ADDR              0x0
#define CHIP_OFFSET                 0x200000000000
#define DIE_OFFSET                  0x0

#define ONE_DEVICE_PAGE_START_OFFSET    0x200000000000

#define HBM_ROW_MEMERY_MASK         (~0x1FFFFF)
#define ECC_CONFIG_START_OFFSET     0x20000

static const enum memory_sensor_id support_fault_type[] = {
    MEMORY_SENSOR_HBMC, MEMORY_SENSOR_DDRC, MEMORY_SENSOR_HHA, MEMORY_SENSOR_DHA
};

int32_t get_hbm_stack_base(uint32_t socket, uint32_t stack, uint64_t *base);
int32_t get_hbm_chip_die_offset(uint32_t dev_id, uint64_t *chip_base_addr, uint64_t *chip_offset,
    uint64_t *die_offset);
void hbm_mirror_cfg_iounmap(void);

#endif

