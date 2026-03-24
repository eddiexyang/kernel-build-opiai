/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
*/

#ifdef LPM_FAULT_SAFETY

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/delay.h>
#include "lpm_fault_aosub_safety.h"
#include "lpm_fault_crg.h"
#include "lpm_init.h"
#include "lpm_fault_safety.h"
#include "dms_interface.h"
#include "dms_node_type.h"
#include "dms_sensor_type.h"
#include "dfm_safety_report.h"
#include "drvfault_user_common.h"
#include "lpm_fault_crg.h"

#define CHIP_VERSION2_ID               2u
#define SYSCTRL_BASE_ADDR              0x80000000u
#define SYSCTRL_BASE_ADDR_SIZE         0x10000u
#define SC_CHIP_INF                    0xfff8u
#define SC_CHIP_VERSION                ((uint32_t)GENMASK(3u, 0u))
#define EFUSE0_SUBCTRL_BASE_ADDR       0x80060000u
#define EFUSE0_SUBCTRL_BASE_ADDR_SIZE  0x10000u
#define SC_EFUSE_PGSENSOR              0xE000u
#define SC_PGSENSOR_CTRL0              0x2700u
#define SC_PGSENSOR_CTRL4              0x2710u
#define SC_PGSENSOR_CTRL5              0x2714u
#define SC_PGSENSOR_CTRL_VAL           0x00090009u
#define SC_PGSENSOR_CTR_CFG            (uint32_t)0x6u
#define SC_PGSENSOR_CTR0_MASK          0xff00ffffu
#define SC_PGSENSOR_ST0                0x2780u
#define TRIM_CODE_MASK                 0xff00u
#define TRIM_CODE_TO_CTRL0_MASK        0x00ffffffu
#define TRIM_CODE_TO_CTRL0_OFST        (uint32_t)16u
#define CRG_MONITOR_BASE_MASK          (uint32_t)0xff00UL

static struct safety_irq_hw_info g_safety_irq_ao_hwinfo[0x7][0x2] = {
	{
		{
			.emu_id = AO_SUB_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 2,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 24,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0x80000000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4f20u, 0x200u },  /* interrupt status reg */
			.clear =  { 0x4f34u, 0x200u },  /* interrupt clear reg */
			.mask =   { 0x4f2cu, 0x200u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AO_SUB_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 2,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 25,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0x80000000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4f20u, 0x1000000u },  /* interrupt status reg */
			.clear  = { 0x4f34u, 0x1000000u },  /* interrupt clear reg */
			.mask   = { 0x4f2cu, 0x1000000u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AO_SUB_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 2,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 30,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0x80000000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x200u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x200u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x200u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AO_SUB_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 2,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 26,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0x80000000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4d20u, 0x200u },  /* interrupt status reg */
			.clear  = { 0x4d34u, 0x200u },  /* interrupt clear reg */
			.mask   = { 0x4d2cu, 0x200u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AO_SUB_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 2,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 27,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0x80000000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4d20u, 0x1000000u },  /* interrupt status reg */
			.clear  = { 0x4d34u, 0x1000000u },  /* interrupt clear reg */
			.mask   = { 0x4d2cu, 0x1000000u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AO_SUB_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 2,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 28,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0x80000000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4e20u, 0x200u },  /* interrupt status reg */
			.clear  = { 0x4e34u, 0x200u },  /* interrupt clear reg */
			.mask   = { 0x4e2cu, 0x200u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AO_SUB_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 2,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 29,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0x80000000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4e20u, 0x1000000u },  /* interrupt status reg */
			.clear  = { 0x4e34u, 0x1000000u },  /* interrupt clear reg */
			.mask   = { 0x4e2cu, 0x1000000u },  /* interrupt mask reg */
		}
	}
};

static struct safety_irq_hw_info g_safety_irq_dvpp_hwinfo[0x2][0x2] = {
	{
		{
			.emu_id = DVPP1_SUB_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 30,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xc90c0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x200u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x200u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x200u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = DVPP1_SUB_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 31,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xc90c0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x1000000u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x1000000u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x1000000u },  /* interrupt mask reg */
		}
	}
};

static struct safety_irq_hw_info g_safety_irq_io_hwinfo[0x5][0x2] = {
	{
		{
			.emu_id = IO_SUB_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 3,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 11,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xa20c0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x200u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x200u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x200u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = IO_SUB_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 3,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 12,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xa20c0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x1000000u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x1000000u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x1000000u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = IO_SUB_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 3,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 9,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xa20c0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4d20u, 0x200u },  /* interrupt status reg */
			.clear  = { 0x4d34u, 0x200u },  /* interrupt clear reg */
			.mask   = { 0x4d2cu, 0x200u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = IO_SUB_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 3,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 10,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xa20c0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4e20u, 0x200u },  /* interrupt status reg */
			.clear  = { 0x4e34u, 0x200u },  /* interrupt clear reg */
			.mask   = { 0x4e2cu, 0x200u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = IO_SUB_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 3,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 8,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xa20c0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4f20u, 0x1000000u },  /* interrupt status reg */
			.clear  = { 0x4f34u, 0x1000000u },  /* interrupt clear reg */
			.mask   = { 0x4f2cu, 0x1000000u },  /* interrupt mask reg */
		}
	}
};

static struct safety_irq_hw_info g_safety_irq_isp_hwinfo[0x4][0x2] = {
	{
		{
			.emu_id = ISP0_SUB_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 28,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xa80c0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x200u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x200u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x200u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = ISP0_SUB_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 29,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xa80c0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x1000000u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x1000000u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x1000000u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = ISP0_SUB_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 30,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xa80c0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4d20u, 0x200u },  /* interrupt status reg */
			.clear  = { 0x4d34u, 0x200u },  /* interrupt clear reg */
			.mask   = { 0x4d2cu, 0x200u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = ISP0_SUB_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 31,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xa80c0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4d20u, 0x1000000u },  /* interrupt status reg */
			.clear  = { 0x4d34u, 0x1000000u },  /* interrupt clear reg */
			.mask   = { 0x4d2cu, 0x1000000u },  /* interrupt mask reg */
		}
	}
};

// DDR sub
static struct safety_irq_hw_info g_safety_irq_ddr_hwinfo[0x4][0x2] = {
	{
		{
			.emu_id = PERI_SUB_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 2,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 12,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0x81320000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x200u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x200u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x200u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = PERI_SUB_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 2,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 13,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0x81320000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x1000000u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x1000000u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x1000000u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = HAC_SUB_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 26,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0x81310000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x200u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x200u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x200u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = HAC_SUB_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 27,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0x81310000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x1000000u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x1000000u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x1000000u },  /* interrupt mask reg */
		}
	}
};

// peri sub
static struct safety_irq_hw_info g_safety_irq_peri_hwinfo[0x4][0x2] = {
	{
		{
			.emu_id = PERI_SUB_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 2,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 8,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xA40c0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4d20u, 0x200u },  /* interrupt status reg */
			.clear  = { 0x4d34u, 0x200u },  /* interrupt clear reg */
			.mask   = { 0x4d2cu, 0x200u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = PERI_SUB_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 2,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 9,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xA40c0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x200u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x200u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x200u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = PERI_SUB_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 2,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 10,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xA40c0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x1000000u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x1000000u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x1000000u },  /* interrupt mask reg */
		}
	}
};

// sils sub
static struct safety_irq_hw_info g_safety_irq_sils_hwinfo[0x2][0x2] = {
	{
		{
			.emu_id = SILS_SUB_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 28,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xCF0C0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x200u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x200u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x200u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = SILS_SUB_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 29,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xCF0C0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x1000000u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x1000000u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x1000000u },  /* interrupt mask reg */
		}
	}
};

// aic sub
static struct safety_irq_hw_info g_safety_irq_aic_hwinfo[0x14][0x2] = {
	{
		{
			.emu_id = AICORE0_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 21,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xC21C0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x200u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x200u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x200u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AICORE0_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 22,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xC21C0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x1000000u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x1000000u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x1000000u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AICORE1_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 21,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xc23c0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x200u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x200u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x200u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AICORE1_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 22,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xC23C0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x1000000u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x1000000u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x1000000u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AICORE2_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 21,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xc25c0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x200u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x200u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x200u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AICORE2_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 22,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xc25c0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x1000000u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x1000000u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x1000000u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AICORE3_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 21,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xc27c0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x200u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x200u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x200u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AICORE3_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 22,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xc27c0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x1000000u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x1000000u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x1000000u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AICORE4_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 21,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xc29c0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x200u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x200u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x200u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AICORE4_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 22,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xc29c0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x1000000u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x1000000u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x1000000u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AICORE5_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 21,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xc2bc0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x200u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x200u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x200u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AICORE5_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 22,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xc2bc0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x1000000u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x1000000u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x1000000u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AICORE6_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 21,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xc2dc0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x200u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x200u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x200u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AICORE6_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 22,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xc2dc0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x1000000u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x1000000u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x1000000u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AICORE7_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 21,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xc2fc0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x200u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x200u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x200u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AICORE7_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 22,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xc2fc0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x1000000u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x1000000u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x1000000u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AICORE8_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 21,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xc31c0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x200u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x200u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x200u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AICORE8_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 22,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xc31c0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x1000000u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x1000000u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x1000000u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AICORE9_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 21,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xc33c0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x200u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x200u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x200u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AICORE9_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 22,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xc33c0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x1000000u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x1000000u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x1000000u },  /* interrupt mask reg */
		}
	}
};

static struct safety_irq_hw_info g_safety_irq_aiv_hwinfo[16][0x2] = {
	{
		{
			.emu_id = AIVECTOR0_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 21,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xc11c0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x200u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x200u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x200u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AIVECTOR0_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 22,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xc11c0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x1000000u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x1000000u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x1000000u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AIVECTOR1_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 21,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xc13c0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x200u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x200u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x200u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AIVECTOR1_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 22,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xc13c0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x1000000u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x1000000u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x1000000u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AIVECTOR2_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 21,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xc15c0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x200u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x200u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x200u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AIVECTOR2_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 22,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xc15c0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x1000000u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x1000000u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x1000000u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AIVECTOR3_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 21,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xc17c0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x200u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x200u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x200u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AIVECTOR3_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 22,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xc17c0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x1000000u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x1000000u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x1000000u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AIVECTOR4_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 21,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xc19c0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x200u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x200u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x200u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AIVECTOR4_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 22,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xc19c0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x1000000u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x1000000u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x1000000u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AIVECTOR5_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 21,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xc1bc0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x200u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x200u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x200u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AIVECTOR5_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 22,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xc1bc0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x1000000u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x1000000u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x1000000u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AIVECTOR6_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 21,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xc1dc0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x200u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x200u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x200u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AIVECTOR6_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 22,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xc1dc0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x1000000u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x1000000u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x1000000u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AIVECTOR7_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 21,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xc1fc0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x200u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x200u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x200u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = AIVECTOR7_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 22,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0xc1fc0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x1000000u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x1000000u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x1000000u },  /* interrupt mask reg */
		}
	}
};

static struct safety_irq_hw_info g_safety_irq_cpu_hwinfo[8][0x2] = {
	{
		{
			.emu_id = CPU_CLUSTER0_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 24,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0x811b0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x200u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x200u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x200u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = CPU_CLUSTER0_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 25,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0x811b0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x1000000u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x1000000u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x1000000u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = CPU_CLUSTER1_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 24,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0x811c0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x200u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x200u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x200u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = CPU_CLUSTER1_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 25,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0x811c0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x1000000u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x1000000u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x1000000u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = CPU_CLUSTER2_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 24,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0x811d0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x200u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x200u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x200u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = CPU_CLUSTER2_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 25,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0x811d0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x1000000u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x1000000u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x1000000u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = CPU_CLUSTER3_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 24,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0x811e0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x200u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x200u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x200u },  /* interrupt mask reg */
		}
	}, {
		{
			.emu_id = CPU_CLUSTER3_EMU_ID,    /* reference to drvfault_emu_id */
			.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
			.bit_id = 25,    /* value equal to:emu_irq_number % 32 */
			.base_paddr = (uint64_t)0x811e0000u, /* interrupt reg base addr */
			.base_size = 0x10000,  /* interrupt reg base size */
			.status = { 0x4c20u, 0x1000000u },  /* interrupt status reg */
			.clear  = { 0x4c34u, 0x1000000u },  /* interrupt clear reg */
			.mask   = { 0x4c2cu, 0x1000000u },  /* interrupt mask reg */
		}
	}
};

#ifdef LPM_FAULT_AO_SAFETY
// aosubsys
static struct safety_irq_hw_info g_ao_subctrl_safety_hwinfo[] = {
	{
		.emu_id = AO_SUB_EMU_ID,    /* reference to drvfault_emu_id */
		.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
		.bit_id = 7,    /* value equal to:emu_irq_number % 32 */
		.base_paddr = 0x80000000U, /* interrupt reg base addr */
		.base_size = 0x10000,  /* interrupt reg base size */
		.status = { 0x2c00, 0x67733 },  /* interrupt status reg */
		.clear  = { 0x2c00, 0x67733 },  /* interrupt clear reg */
		.mask   = { 0x2c04, 0x67733 },  /* interrupt mask reg */
		.check_mask[0] = { DRVFAULT_FILED_VALID, 0x2c0c, 0xfff880ccU }
	}, {
		.emu_id = AO_SUB_EMU_ID,    /* reference to drvfault_emu_id */
		.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
		.bit_id = 23,    /* value equal to:emu_irq_number % 32 */
		.base_paddr = 0x80000000U, /* interrupt reg base addr */
		.base_size = 0x10000,  /* interrupt reg base size */
		.status = { 0x2c00, 0x10800 },  /* interrupt status reg */
		.clear  = { 0x2c00, 0x10800 },  /* interrupt clear reg */
		.mask   = { 0x2c08, 0x10800 },  /* interrupt mask reg */
		.check_mask[0] = { DRVFAULT_FILED_VALID, 0x2c0c, 0xfff880ccU }
	}, {
		.emu_id = AO_SUB_EMU_ID,    /* reference to drvfault_emu_id */
		.src_id = 1,    /* value equal to:1 + emu_irq_number / 32  */
		.bit_id = 23,    /* value equal to:emu_irq_number % 32 */
		.base_paddr = 0x80000000U, /* interrupt reg base addr */
		.base_size = 0x10000,  /* interrupt reg base size */
		.status = { 0x2c20, 0x60006 },  /* interrupt status reg */
		.clear  = { 0x2c20, 0x60006 },  /* interrupt clear reg */
		.mask   = { 0x2c28, 0x60006 },  /* interrupt mask reg */
		.check_mask[0] = { DRVFAULT_FILED_VALID, 0x2c2c, 0xfff9fff9U }
	}
};
#endif

#ifdef LPM_FAULT_AO_SAFETY
static struct safety_irq_info g_safety_aosub_irq_info[] = {
	{
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_ao_sub",
		.safety_irq_func = lpm_safety_irq_aosubsys_handler,
		.irq_hwinfo_num = 0x3u,
		.irq_hwinfo_list = g_ao_subctrl_safety_hwinfo,
	}
};
#endif

static uint32_t g_crg_irq_num = 0;
static struct safety_irq_info g_safety_crg_irq_info[] = {
	// AO sub
	{
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_clk_tcxo",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_ao_hwinfo[0],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_rst_tcxo",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_ao_hwinfo[1],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_clk_32k",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_ao_hwinfo[2],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_clk_tcxo_sils",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_ao_hwinfo[3],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_rst_tcxo_sils",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_ao_hwinfo[4],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_clk_pll3_div2",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_ao_hwinfo[5],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_rst_pll3_div2",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_ao_hwinfo[6],
	},
	// DVPP sub
	{
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_clk_pll7_div3",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_dvpp_hwinfo[0],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_rst_pll7_div3",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_dvpp_hwinfo[1],
	},
	// IO sub
	{
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_clk_pll11_div1",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_io_hwinfo[0],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_rst_pll11_div1",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_io_hwinfo[0x1],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_clk_hilink3_ref0",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_io_hwinfo[0x2],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_clk_hilink3_ref1",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_io_hwinfo[0x3],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_clk_pll9_div2",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_io_hwinfo[0x4],
	},
	// ISP sub
	{
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_clk_pll4_div2",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_isp_hwinfo[0],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_rst_pll4_div2",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_isp_hwinfo[0x1],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_clk_pll5_div2",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_isp_hwinfo[0x2],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_rst_pll5_div2",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_isp_hwinfo[0x3],
	},
	// DDR sub
	{
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_clk_pll_ddr_div1_left",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_ddr_hwinfo[0],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_rst_pll_ddr_div1_left",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_ddr_hwinfo[0x1],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_clk_pll_ddr_div1_right",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_ddr_hwinfo[0x2],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_rst_pll_ddr_div1_right",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_ddr_hwinfo[0x3],
	},
	// PERI sub
	{
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_clk_ufs_ref",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_peri_hwinfo[0],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_clk_pll1_ring_peri",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_peri_hwinfo[0x1],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_rst_pll1_ring_peri",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_peri_hwinfo[0x2],
	},
	// SILS sub
	{
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_clk_pll10_div1",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_sils_hwinfo[0],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_rst_pll10_div1",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_sils_hwinfo[0x1],
	},
	// AIC sub
	{
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_clk_pll6_div1_aic0",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_aic_hwinfo[0],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_rst_pll6_div1_aic0",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_aic_hwinfo[0x1],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_clk_pll6_div1_aic1",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_aic_hwinfo[0x2],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_rst_pll6_div1_aic1",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_aic_hwinfo[0x3],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_clk_pll6_div1_aic2",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_aic_hwinfo[0x4],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_rst_pll6_div1_aic2",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_aic_hwinfo[0x5],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_clk_pll6_div1_aic3",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_aic_hwinfo[0x6],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_rst_pll6_div1_aic3",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_aic_hwinfo[0x7],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_clk_pll6_div1_aic4",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_aic_hwinfo[0x8],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_rst_pll6_div1_aic4",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_aic_hwinfo[0x9],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_clk_pll6_div1_aic5",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_aic_hwinfo[0xa],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_rst_pll6_div1_aic5",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_aic_hwinfo[0xb],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_clk_pll6_div1_aic6",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_aic_hwinfo[0xc],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_rst_pll6_div1_aic6",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_aic_hwinfo[0xd],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_clk_pll6_div1_aic7",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_aic_hwinfo[0xe],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_rst_pll6_div1_aic7",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_aic_hwinfo[0xf],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_clk_pll6_div1_aic8",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_aic_hwinfo[0x10],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_rst_pll6_div1_aic8",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_aic_hwinfo[0x11],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_clk_pll6_div1_aic9",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_aic_hwinfo[0x12],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_rst_pll6_div1_aic9",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_aic_hwinfo[0x13],
	},
	// AIV sub
	{
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_clk_pll1_mesh_aiv0",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_aiv_hwinfo[0],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_rst_pll1_mesh_aiv0",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_aiv_hwinfo[1],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_clk_pll1_mesh_aiv1",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_aiv_hwinfo[2],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_rst_pll1_mesh_aiv1",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_aiv_hwinfo[3],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_clk_pll1_mesh_aiv2",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_aiv_hwinfo[4],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_rst_pll1_mesh_aiv2",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_aiv_hwinfo[5],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_clk_pll1_mesh_aiv3",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_aiv_hwinfo[6],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_rst_pll1_mesh_aiv3",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_aiv_hwinfo[7],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_clk_pll1_mesh_aiv4",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_aiv_hwinfo[8],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_rst_pll1_mesh_aiv4",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_aiv_hwinfo[9],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_clk_pll1_mesh_aiv5",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_aiv_hwinfo[10],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_rst_pll1_mesh_aiv5",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_aiv_hwinfo[11],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_clk_pll1_mesh_aiv6",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_aiv_hwinfo[12],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_rst_pll1_mesh_aiv6",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_aiv_hwinfo[13],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_clk_pll1_mesh_aiv7",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_aiv_hwinfo[14],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_rst_pll1_mesh_aiv7",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_aiv_hwinfo[15],
	},
	// CPU Cluster sub
	{
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_clk_pll0_cpu0",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_cpu_hwinfo[0],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_rst_pll0_cpu0",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_cpu_hwinfo[1],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_clk_pll0_cpu1",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_cpu_hwinfo[2],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_rst_pll0_cpu1",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_cpu_hwinfo[3],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_clk_pll0_cpu2",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_cpu_hwinfo[4],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_rst_pll0_cpu2",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_cpu_hwinfo[5],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_clk_pll0_cpu3",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_cpu_hwinfo[6],
	}, {
		.dev_id = 0,
		.irq_type = SAFETY_IRQ_TYPE_LPI,
		.irq_name = (char *)"int_ras_safety_det_rst_pll0_cpu3",
		.safety_irq_func = lpm_safety_irq_crg_handler,
		.irq_hwinfo_num = 0x1u,
		.irq_hwinfo_list = g_safety_irq_cpu_hwinfo[7],
	}
};

STATIC bool lpm_safety_is_chip_ver2(void)
{
	static uint32_t ver = 0;
	uint32_t val;
	void __iomem *vaddr = NULL;

	if (ver == 0) {
		vaddr = (void __iomem *)ioremap((uintptr_t)SYSCTRL_BASE_ADDR, SYSCTRL_BASE_ADDR_SIZE);
		if (vaddr == NULL) {
			return false;
		}
		val = readl((void __iomem *)((uintptr_t)vaddr + SC_CHIP_INF));
		ver = val & SC_CHIP_VERSION;
		iounmap(vaddr);
		lpm_log_info("lpm safety get verion:0x%x\n", val);
	}

	return (ver == CHIP_VERSION2_ID);
}

void lpm_safety_suspend_prepare(void)
{
	if (!lpm_safety_is_chip_ver2()) {
		return;
	}

	lpm_fault_safety_crg_uninit();
	lpm_fault_safety_aosub_suspend();
}

void lpm_safety_post_suspend(void)
{
	int32_t ret;

	if (!lpm_safety_is_chip_ver2()) {
		return;
	}

	ret = lpm_fault_safety_crg_init();
	if (ret != 0) {
		lpm_log_err("lpm_fault_safety_crg_init failed after resume, ret=%d\n", ret);
	}

	lpm_fault_safety_aosub_resume();
}

STATIC int32_t lpm_fault_pgsensor_int(void)
{
	uint32_t val = 0;
	uint32_t val2 = 0;
	void __iomem *vaddr = NULL;
	vaddr = (void __iomem *)ioremap((uintptr_t)EFUSE0_SUBCTRL_BASE_ADDR, EFUSE0_SUBCTRL_BASE_ADDR_SIZE);
	if (vaddr == NULL) {
		lpm_log_err("lpm fault map efuse failed\n");
		return -1;
	}

	// step0 write trim_code to pgsensor ctrl0
	val = readl((void __iomem *)((uintptr_t)vaddr + SC_EFUSE_PGSENSOR));
	val = (val & TRIM_CODE_MASK) << TRIM_CODE_TO_CTRL0_OFST;
	iounmap(vaddr);
	vaddr = (void __iomem *)ioremap((uintptr_t)SYSCTRL_BASE_ADDR, SYSCTRL_BASE_ADDR_SIZE);
	if (vaddr == NULL) {
		lpm_log_err("lpm fault map sys failed\n");
		return -1;
	}
	val2 = readl((void __iomem *)((uintptr_t)vaddr + SC_PGSENSOR_CTRL0));
	val2 = (val2 & TRIM_CODE_TO_CTRL0_MASK) | val;
	writel(val2, (void __iomem *)((uintptr_t)vaddr + SC_PGSENSOR_CTRL0));
	// wait 1.5ms
	usleep_range(1400U, 1500U);
	// step1 write pgsensor ctrl4 ctrl5 cfg
	writel(SC_PGSENSOR_CTRL_VAL, (void __iomem *)((uintptr_t)vaddr + SC_PGSENSOR_CTRL4));
	writel(SC_PGSENSOR_CTRL_VAL, (void __iomem *)((uintptr_t)vaddr + SC_PGSENSOR_CTRL5));
	// step2 enable pgsensor vde
	val2 = readl((void __iomem *)((uintptr_t)vaddr + SC_PGSENSOR_CTRL0));
	val2 = (val2 & SC_PGSENSOR_CTR0_MASK) | (SC_PGSENSOR_CTR_CFG << TRIM_CODE_TO_CTRL0_OFST);
	writel(val2, (void __iomem *)((uintptr_t)vaddr + SC_PGSENSOR_CTRL0));
	// wait 1.5ms
	usleep_range(1400U, 1500U);
	// step3 check pgsensor ready or not
	val = readl((void __iomem *)((uintptr_t)vaddr + SC_PGSENSOR_ST0));
	iounmap(vaddr);
	if ((val & SC_PGSENSOR_CTR_CFG) != 0) {
		lpm_log_info("lpm fault pgsensor init success\n");
		return 0;
	} else {
		lpm_log_err("lpm fault pgsensor init failed, val=0x%x\n", val);
		return -1;
	}
}

STATIC int32_t lpm_fault_safety_irq_reg(void)
{
	int32_t ret;
	uint32_t i;
	uint32_t j;
	struct safety_irq_hw_info *hwinfo_list = NULL;

	j = 0;
	for (i = 0; i < (uint32_t)ARRAY_SIZE(g_safety_crg_irq_info); i++) {
		hwinfo_list = &g_safety_crg_irq_info[i].irq_hwinfo_list[0];
		if (!lpm_fault_safety_crg_sub_monitor_enable(
			(uintptr_t)hwinfo_list->base_paddr, (uint32_t)hwinfo_list->status.offset & CRG_MONITOR_BASE_MASK)) {
			continue;
		}

		if (i != j) {
			(void)memcpy_s(&g_safety_crg_irq_info[j], sizeof(struct safety_irq_info),
				&g_safety_crg_irq_info[i], sizeof(struct safety_irq_info));
		}
		j++;
	}

	g_crg_irq_num = j;
	ret = dfm_register_safety_irq_hwinfo(0, g_safety_crg_irq_info, j);
	if (ret != 0) {
		lpm_log_err("dfm register crg safety failed, ret=%d\n", ret);
		return ret;
	}
	lpm_log_info("crg irq register succ, total num=%u, actual num=%u\n", i, j);

#ifdef LPM_FAULT_AO_SAFETY
	ret = dfm_register_safety_irq_hwinfo(0, g_safety_aosub_irq_info, (uint32_t)ARRAY_SIZE(g_safety_aosub_irq_info));
	if (ret != 0) {
		lpm_log_err("dfm register aosub safety failed, ret=%d\n", ret);
		return ret;
	}
#endif

	return 0;
}

STATIC void lpm_fault_safety_irq_unreg(void)
{
	dfm_unregister_safety_irq_hwinfo(0, g_safety_crg_irq_info, g_crg_irq_num);

#ifdef LPM_FAULT_AO_SAFETY
	dfm_unregister_safety_irq_hwinfo(0, g_safety_aosub_irq_info, ARRAY_SIZE(g_safety_aosub_irq_info));
#endif
}

int32_t lpm_fault_safety_init(uint32_t dev_num)
{
	int32_t ret;

	(void)dev_num;

	if (!lpm_safety_is_chip_ver2()) {
		lpm_log_info("version1 not support safety\n");
		return 0;
	}

	ret = lpm_fault_safety_irq_reg();
	if (ret != 0) {
		lpm_log_err("lpm register safety irq failed, ret=%d\n", ret);
		return ret;
	}

	ret = lpm_fault_safety_crg_init();
	if (ret != 0) {
		lpm_log_err("lpm_fault_safety_crg_init failed, ret=%d\n", ret);
		goto init_crg_fail;
	}

	ret = lpm_fault_safety_aosub_init();
	if (ret != 0) {
		lpm_log_err("lpm_fault_safety_aosub_init failed, ret=%d\n", ret);
		goto init_aosub_fail;
	}
	ret = lpm_fault_pgsensor_int();
	if (ret != 0) {
		lpm_log_err("lpm_fault_safety_PGsensor_init failed, ret=%d\n", ret);
		goto init_pgsensor_fail;
	}
	lpm_log_info("lpm fault safety init success\n");

	return 0;

init_pgsensor_fail:
	lpm_fault_safety_aosub_uninit();
init_aosub_fail:
	lpm_fault_safety_crg_uninit();
init_crg_fail:
	lpm_fault_safety_irq_unreg();
	return ret;
}

int32_t lpm_fault_safety_exit(uint32_t dev_num)
{
	int32_t ret;
	if (!lpm_safety_is_chip_ver2()) {
		lpm_log_info("version1 not support safety\n");
		return 0;
	}

	(void)dev_num;
	lpm_fault_safety_crg_uninit();
	lpm_fault_safety_irq_unreg();
	ret = lpm_fault_safety_aosub_uninit();
	if (ret != 0) {
		lpm_log_err("lpm_fault_aosub_safety_uninit failed, ret is%d\n", ret);
		return ret;
	}
	lpm_log_info("lpm fault safety exit success\n");
	return 0;
}

#endif