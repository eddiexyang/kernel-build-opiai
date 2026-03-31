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

#define MAX_OS_DEVICE_COUNT             4
#define MAX_HBM_STACK_PER_SOCKET        4
#define MAX_HBM_PC_PER_STACK            16
#define MAX_HBM_CONTROL_COUNT           (MAX_HBM_STACK_PER_SOCKET * MAX_HBM_PC_PER_STACK)

#define FULLMESH_CHIP_BASE_ADDR         0x0
#define FULLMESH_CHIP_OFFSET            0x80000000000ULL
#define FULLMESH_DIE_OFFSET             0x10000000000ULL

#define HCCS_CHIP_BASE_ADDR             0x200000000000ULL
#define HCCS_CHIP_OFFSET                0x20000000000ULL
#define HCCS_DIE_OFFSET                 0x10000000000ULL

#define HBMC0_REG_BASE                  0x501c80000ULL
#define HBMC1_REG_BASE                  0x501d80000ULL
#define HBMC2_REG_BASE                  0x503c80000ULL
#define HBMC3_REG_BASE                  0x503d80000ULL
#define HBM_STACK0_BASE_ADDR            0x501C00000ULL
#define HBM_STACK1_BASE_ADDR            0x503C00000ULL
#define HBM_STACK2_BASE_ADDR            0x501D00000ULL
#define HBM_STACK3_BASE_ADDR            0x503D00000ULL
#define HBM_HBMC_PC_OFFSET              0x2000ULL
#define HBM_HBMC_BASE_ADDR              0x80000ULL
#define HBM_HBMC_BC_BASE_ADDR           0xA0000ULL
#define HBM_HBMC_RASC_ADDR              0x1000ULL
#define HBM_RASC_BASE_ADDR              (HBM_HBMC_BASE_ADDR + HBM_HBMC_RASC_ADDR)
#define HBM_RASC_BC_BASE_ADDR           (HBM_HBMC_BC_BASE_ADDR + HBM_HBMC_RASC_ADDR)
#define HBM_HBMC_ARER_ADDR              0x1800ULL
#define HBM_ARER_BASE_ADDR              (HBM_HBMC_BASE_ADDR + HBM_HBMC_ARER_ADDR)

#define RASC_HIS_UNCORR_CNT             0x114ULL
#define RASC_HIS_CORR_CNT               0x118ULL
#define RASC_HIS_CORR_ADDR_L_ADDR       0x180ULL
#define RASC_HIS_CORR_ADDR_H_ADDR       0x184ULL
#define ARER_ERR_CTLR_L_OFFSET          0x8ULL

#define HBM_RASC_CFG_CLR_OFFSET                 0x040ULL
#define HBM_RASC_RINT_OFFSET                    0x104ULL
#define HBM_RASC_CFG_CORR_TH                    0X0E0ULL
#define HBM_RASC_CFG_ERRINJMODE_OFFSET          0x080ULL
#define HBM_RASC_CFG_ERRINJ_DMSK_OFFSET         0x090ULL

#define HBM_RASC_CFG_ERRINJ_ADDR_OFFSET         0x0D0ULL
#define HBM_RASC_CFG_ERRINJ_ADDRH_OFFSET        0x0D4ULL
#define HBM_RASC_CFG_ERRINJ_ADDRMSK_OFFSET      0x0D8ULL
#define HBM_RASC_CFG_ERRINJ_ADDRMSKH_OFFSET     0x0DCULL

#define HBM_RASC_CTRL_ERRINJ_OFFSET             0x00CULL

#define HBM_DDRC_CFG_COL_PAR_ERRINJ_ADDR(pc) ((HBM_HBMC_BASE_ADDR) + ((uint64_t)(pc) * (HBM_HBMC_PC_OFFSET)) + 0x720ULL)
#define HBM_DDRC_CFG_ROW_PAR_ERRINJ_ADDR(pc) ((HBM_HBMC_BASE_ADDR) + ((uint64_t)(pc) * (HBM_HBMC_PC_OFFSET)) + 0x728ULL)
#define HBM_DDRC_CFG_WDQ_PAR_ERRINJ_ADDR(pc) ((HBM_HBMC_BASE_ADDR) + ((uint64_t)(pc) * (HBM_HBMC_PC_OFFSET)) + 0x734ULL)
#define HBM_DDRC_CFG_RDQ_PAR_ERRINJ_ADDR(pc) ((HBM_HBMC_BASE_ADDR) + ((uint64_t)(pc) * (HBM_HBMC_PC_OFFSET)) + 0x73CULL)
#define HBM_DDRC_CFG_FIFO_ERRINJ_ADDR(pc)    ((HBM_HBMC_BASE_ADDR) + ((uint64_t)(pc) * (HBM_HBMC_PC_OFFSET)) + 0x038ULL)

#define HBM_PHYSIC_ADDR_CHIP_OFFSET             0x80000000000ULL
#define HBM_PHYSIC_ADDR_DIE_OFFSET              0x11000000000ULL

/* MATA寄存器基地址 */
#define MATA0_BASE_ADDR                         0x703940000ULL
#define MATA_BASE_ADDR(mata)                    ((MATA0_BASE_ADDR) + ((uint64_t)(mata) * (0x10000ULL)))
#define ALL_MATA_BASE_ADDR                      0x703F70000ULL
#define MATA_MATA_LUC_CTRL_ADDR(base)           ((base) + (0x0014ULL))

#define MATA_RAS_PFGCTL_H                       0x600ULL
#define MATA_ERR_STATUSL                        0x2010ULL
#define MATA_RAS_PFGCTL_L                       0x2808ULL

#define HBM_ROW_MEMERY_MASK						(~0xFFFFF)
#define ECC_CONFIG_START_OFFSET					0x3DA0000

#define MAX_MIRROR_WIN_NUM                      8U
#define MSD_DIE0_BASEADDR                       0x1000000000ULL
#define MSD7_WIN_SIZE                           0x40000000ULL

#define DSA_SRAM_BASE_ADDR                      0x402000000ULL
#define HBM_MIRROR_OFFSET                       0x399A0ULL
#define HBM_MIRROR_SIZE                         0xFC0ULL
#define HBM_MIRROR_START_ADDR(base)             (base + DSA_SRAM_BASE_ADDR + HBM_MIRROR_OFFSET)

#define HBM_ADDR_TRANS_INFO_OFFSET              HBM_MIRROR_OFFSET + HBM_MIRROR_SIZE
#define HBM_ADDR_TRANS_INFO_SIZE                0x40ULL
#define HBM_ADDR_TRANS_INFO_START_ADDR          (DSA_SRAM_BASE_ADDR + HBM_ADDR_TRANS_INFO_OFFSET)

#define CUR_ADDR_IS_MIRROR_ADDR                 1

struct hbm_mirror_addr {
	uint32_t en;
	uint64_t start_addr;
	uint64_t end_addr;
};

struct hbm_mirror_addr_cfg {
	struct hbm_mirror_addr mirror_addr[MAX_MIRROR_WIN_NUM];
};

struct hbm_mirror_win_config {
	uint32_t en;        // mirror window enable
	uint32_t addr_high;  // start addr[63:32]
	uint32_t addr_low;   // start addr[31:0]
	uint32_t size_high;  // size[63:32]
	uint32_t size_low;   // size[31:0]
};

struct hbm_mirror_config {
	uint32_t en;    // mirror enable
	struct hbm_mirror_win_config win[MAX_MIRROR_WIN_NUM];  // mirror window config
};


static const enum memory_sensor_id support_fault_type[] = {
    MEMORY_SENSOR_HBMC, MEMORY_SENSOR_HHA
};

int32_t get_hbm_stack_base(uint32_t socket, uint32_t stack, uint64_t *base);
int32_t get_hbm_chip_die_offset(uint32_t dev_id, uint64_t *chip_base_addr, uint64_t *chip_offset,
    uint64_t *die_offset);
void hbm_mirror_cfg_iounmap(void);

#endif

