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

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/delay.h>
#include "lpm_init.h"
#include "lpm_fault_safety.h"
#include "dms_interface.h"
#include "dms_node_type.h"
#include "dms_sensor_type.h"
#include "dfm_safety_report.h"
#include "drvfault_user_common.h"
#include "lpm_fault_crg.h"
#include "lpm_fault_common.h"

#define LP_SAFETY_MAX_EVENT_NUM    2u
#define LP_SAFETY_EVENT_MODULE_ERROR_CANNOT_FIXED  1u

#define CRG_MAX_BASE_OFFSET_CNT 4u
#define CRG_CLK_MASK_OFFSET     ((uint32_t)BIT(9))
#define CRG_RST_MASK_OFFSET     ((uint32_t)BIT(24))
#define CRG_DET_INT_ENABLE_REG  0x0u
#define CRG_DET_INT_STATUS_REG  0x20u
#define CRG_DET_INT_MASK_REG    0x24u
#define CRG_DET_ERR_MASK_REG    0x28u
#define CRG_DET_FUNC_MASK_REG   0x2cu
#define CRG_DET_CLR_REG         0x34u

enum crg_aic_freq {
	LPM_CRG_AIC_1230M    = 0,
	LPM_CRG_AIC_1000M    = 1,
	LPM_CRG_AIC_960M     = 2,
	LPM_CRG_AIC_750M     = 3,
	LPM_CRG_AIC_INVALID
};

enum crg_ddr_freq {
	LPM_CRG_DDR_800M     = 0,
	LPM_CRG_DDR_687M     = 1,
	LPM_CRG_DDR_533M     = 2,
	LPM_CRG_DDR_INVALID
};

struct base_offset_list {
	uint32_t cnt;
	uint32_t offset[CRG_MAX_BASE_OFFSET_CNT];
};

struct module_info {
	uintptr_t phy_addr;
	void __iomem *vaddr;
	uint32_t size;
};

static struct module_info g_crg_module_info[] = {
	{0x80000000u, (void *)0x0, 0x10000 },  // ao sub
	{0xc90c0000u, (void *)0x0, 0x10000 },  // dvpp sub
	{0xa20c0000u, (void *)0x0, 0x10000 },  // io sub
	{0xa80c0000u, (void *)0x0, 0x10000 },  // isp sub
	{0x81320000u, (void *)0x0, 0x10000 },  // ddr left
	{0x81310000u, (void *)0x0, 0x10000 },  // ddr right
	{0xA40c0000u, (void *)0x0, 0x10000 },  // peri sub
	{0xCF0C0000u, (void *)0x0, 0x10000 },  // sils sub
	{0xc21c0000u, (void *)0x0, 0x10000 },  // aic0 sub
	{0xc23c0000u, (void *)0x0, 0x10000 },  // aic1 sub
	{0xc25c0000u, (void *)0x0, 0x10000 },  // aic2 sub
	{0xc27c0000u, (void *)0x0, 0x10000 },  // aic3 sub
	{0xc29c0000u, (void *)0x0, 0x10000 },  // aic4 sub
	{0xc2bc0000u, (void *)0x0, 0x10000 },  // aic5 sub
	{0xc2dc0000u, (void *)0x0, 0x10000 },  // aic6 sub
	{0xc2fc0000u, (void *)0x0, 0x10000 },  // aic7 sub
	{0xc31c0000u, (void *)0x0, 0x10000 },  // aic8 sub
	{0xc33c0000u, (void *)0x0, 0x10000 },  // aic9 sub
	{0xc11c0000u, (void *)0x0, 0x10000 },  // aiv0 sub
	{0xc13c0000u, (void *)0x0, 0x10000 },  // aiv1 sub
	{0xc15c0000u, (void *)0x0, 0x10000 },  // aiv2 sub
	{0xc17c0000u, (void *)0x0, 0x10000 },  // aiv3 sub
	{0xc19c0000u, (void *)0x0, 0x10000 },  // aiv4 sub
	{0xc1bc0000u, (void *)0x0, 0x10000 },  // aiv5 sub
	{0xc1dc0000u, (void *)0x0, 0x10000 },  // aiv6 sub
	{0xc1fc0000u, (void *)0x0, 0x10000 },  // aiv7 sub
	{0x811b0000u, (void *)0x0, 0x10000 },  // cpucluster0 sub
	{0x811c0000u, (void *)0x0, 0x10000 },  // cpucluster1 sub
	{0x811d0000u, (void *)0x0, 0x10000 },  // cpucluster2 sub
	{0x811e0000u, (void *)0x0, 0x10000 },  // cpucluster3 sub
};

#define SC_MONITOR_CONFIG_REG_TIMEOUT                      (uint32_t)200   // units:us
#define SC_MONITOR_CFG_RST_DET_REG                         (uint32_t)0x04
#define SC_MONITOR_CFG_RST_DET                             ((uint32_t)GENMASK(2u, 0u))
#define SC_MONITOR_CFG_RST_DET_SHIFT                       (uint32_t)0
#define SC_MONITOR_STAT_RST_DET_REG                        (uint32_t)0x08
#define SC_MONITOR_PLL_DET_REF_SEL                         (uint32_t)0xAC
#define MONITOR_PLL_DET_REQ_SFT                            ((uint32_t)GENMASK(2u, 0u))
#define MONITOR_PLL_DET_REQ_SFT_SHIFT                      (uint32_t)0
#define SC_MONITOR_PLL_DET_REF_EN                          (uint32_t)0xB0
#define MONITOR_PLL_DET_FREQ_CFG                           ((uint32_t)GENMASK(23u, 0u))
#define MONITOR_PLL_DET_FREQ_CFG_SHIFT                     (uint32_t)0
#define MONITOR_CFG_PLL_DET_EN                             ((uint32_t)BIT(24))
#define MONITOR_CFG_PLL_DET_EN_SHIFT                       (uint32_t)24
#define SC_MONITOR_PLL_CLK_EN                              (uint32_t)0xB4
#define MONITOR_PLL_DET_CLK_EN_MASK                        ((uint32_t)GENMASK(5u, 0u))
#define MONITOR_PLL_DET_CLK_EN_SHIFT                       (uint32_t)0
#define SC_MONITOR_PLL_FAST_OOF_DET_DIV                    (uint32_t)0xB8
#define MONITOR_PLL_CFG_FAST_OOF_DET_MASK                  ((uint32_t)GENMASK(24u, 0u))
#define MONITOR_PLL_CFG_FAST_OOF_DET_SHIFT                 (uint32_t)0
#define SC_MONITOR_PLL_FAST_OOF_ALM_OVER_THR               (uint32_t)0xC8
#define SC_MONITOR_CFG_FAST_OOF_ALM_OVER_MASK              ((uint32_t)GENMASK(7u, 0u))
#define SC_MONITOR_CFG_FAST_OOF_ALM_OVER_SHIFT             (uint32_t)0
#define SC_MONITOR_PLL_FAST_OOF_ALM_BELOW_THR              (uint32_t)0xCC
#define SC_MONITOR_CFG_FAST_OOF_ALM_BELOW_MASK             ((uint32_t)GENMASK(7u, 0u))
#define SC_MONITOR_CFG_FAST_OOF_ALM_BELOW_SHIFT            (uint32_t)0
#define SC_MONITOR_PLL_FAST_OOF_NOALM_OVER_THR             (uint32_t)0xD0
#define SC_MONITOR_CFG_FAST_OOF_NOALM_OVER_MASK            ((uint32_t)GENMASK(7u, 0u))
#define SC_MONITOR_CFG_FAST_OOF_NOALM_OVER_SHIFT           (uint32_t)0
#define SC_MONITOR_PLL_FAST_OOF_NOALM_BELOW_THR            (uint32_t)0xD4
#define SC_MONITOR_CFG_FAST_OOF_NOALM_BELOW_MASK           ((uint32_t)GENMASK(7u, 0u))
#define SC_MONITOR_CFG_FAST_OOF_NOALM_BELOW_SHIFT          (uint32_t)0
#define SC_MONITOR_PLL_PRE_OOF_DET_DIV_H                   (uint32_t)0xBC
#define SC_MONITOR_CFG_PRE_OOF_DET_DIV_MASK                ((uint32_t)GENMASK(28u, 0u))
#define SC_MONITOR_CFG_PRE_OOF_DET_DIV_SHIFT               (uint32_t)0
#define SC_MONITOR_PLL_PRE_FAST_OOF_ALM_OVER_THR           (uint32_t)0xD8
#define SC_MONITOR_PLL_CFG_PRE_OOF_ALM_OVER_THR_MASK       ((uint32_t)GENMASK(14u, 0u))
#define SC_MONITOR_PLL_CFG_PRE_OOF_ALM_OVER_THR_SHIFT      (uint32_t)0
#define SC_MONITOR_PLL_PRE_FAST_OOF_ALM_BELOW_THR          (uint32_t)0xDC
#define SC_MONITOR_PLL_CFG_PRE_OOF_ALM_BELOW_THR_MASK      ((uint32_t)GENMASK(14u, 0u))
#define SC_MONITOR_PLL_CFG_PRE_OOF_ALM_BELOW_THR_SHIFT     (uint32_t)0
#define SC_MONITOR_PLL_PRE_FAST_OOF_NOALM_OVER_THR         (uint32_t)0xE0
#define SC_MONITOR_PLL_CFG_PRE_OOF_NOALM_OVER_THR_MASK     ((uint32_t)GENMASK(14u, 0u))
#define SC_MONITOR_PLL_CFG_PRE_OOF_NOALM_OVER_THR_SHIFT    (uint32_t)0
#define SC_MONITOR_PLL_PRE_FAST_OOF_NOALM_BELOW_THR        (uint32_t)0xE4
#define SC_MONITOR_PLL_CFG_PRE_OOF_NOALM_BELOW_THR_MASK    ((uint32_t)GENMASK(14u, 0u))
#define SC_MONITOR_PLL_CFG_PRE_OOF_NOALM_BELOW_THR_SHIFT   (uint32_t)0
#define SC_MONITOR_PLL_LOS_DET_DIV                         (uint32_t)0xC0
#define SC_MONITOR_PLL_CFG_LOS_DET_DIV_MASK                ((uint32_t)GENMASK(22u, 0u))
#define SC_MONITOR_PLL_CFG_LOS_DET_DIV_SHIFT               (uint32_t)0
#define SC_MONITOR_PLL_LOS_ALM_THR                         (uint32_t)0xE8
#define SC_MONITOR_PLL_CFG_LOS_ALM_THR_MASK                ((uint32_t)GENMASK(19u, 0u))
#define SC_MONITOR_PLL_CFG_LOS_ALM_THR_SHIFT               (uint32_t)0
#define SC_MONITOR_PLL_LOS_NOALM_THR                       (uint32_t)0xEC
#define SC_MONITOR_PLL_CFG_LOS_NOAML_THR_MASK              ((uint32_t)GENMASK(19u, 0u))
#define SC_MONITOR_PLL_CFG_LOS_NORML_THR_SHIFT             (uint32_t)0
#define SC_MONITOR_PLL_LOS_NOINT_THR                       (uint32_t)0xF0
#define SC_MONITOR_PLL_CFG_LOS_NOINT_THR_MASK              ((uint32_t)GENMASK(24u, 0u))
#define SC_MONITOR_PLL_CFG_LOS_NOINT_THR_SHIFT             (uint32_t)0
#define SC_MONITOR_PLL_DET_INT_MASK                        (uint32_t)0xC4
#define SC_MONITOR_PLL_CFG_DET_INT_MASK                    ((uint32_t)GENMASK(2u, 0u))
#define SC_MONITOR_PLL_CFG_DET_INT_SHIFT                   (uint32_t)0

#define LPM_CRG_DDR_FREQ_BASE_ADDR                         (uintptr_t)0x31200078UL
#define LPM_CRG_DDR_FREQ_ADDR_SIZE                         (uint32_t)0x4UL
#define LPM_CRG_DDR_FREQ_1600M                             (uint32_t)1600UL
#define LPM_CRG_DDR_FREQ_2745M                             (uint32_t)2745UL
#define LPM_CRG_DDR_FREQ_2133M                             (uint32_t)2133UL
#define LPM_CRG_DDR_FREQ_3200M                             (uint32_t)3200UL
#define LPM_CRG_DDR_FREQ_DEVIATION                         (uint32_t)50UL

struct crg_monitor_param {
	uint32_t cfg_fast_oof_det_div;
	uint32_t cfg_fast_oof_alm_over_thr;
	uint32_t cfg_fast_oof_alm_below_thr;
	uint32_t cfg_fast_oof_noalm_over_thr;
	uint32_t cfg_fast_oof_noalm_below_thr;
	uint32_t cfg_pre_oof_det_div;
	uint32_t cfg_pre_oof_alm_over_thr;
	uint32_t cfg_pre_oof_alm_below_thr;
	uint32_t cfg_pre_oof_noalm_over_thr;
	uint32_t cfg_pre_oof_noalm_below_thr;
	uint32_t cfg_los_det_div;
	uint32_t cfg_los_alm_thr;
	uint32_t cfg_los_noalm_thr;
	uint32_t cfg_los_noint_thr;
};

struct crg_config {
	uintptr_t phy_base_addr;
	uint32_t monitor_ofst;
	uint32_t ref_clk_cfg;
	uint32_t det_clk_en;
	uint8_t  module_name[16];
	enum lpm_pg_module_type type;
	uint64_t mask;
	struct crg_monitor_param param;
};

STATIC struct crg_config g_crg_moniter_cfg[] = {
	// ao
	{0x80000000u, 0x4c00u, 0xBC9EBFu, 0x7u, "ao_m1", LPM_PG_TYPE_OTHER, 0,
		{0x11u, 0x2Au, 0x26u, 0x29u, 0x27u, 0xC9u, 0x401u, 0x3CFu, 0x3F2u, 0x3DEu, 0x9u, 0x14u, 0xAu, 0x3Du}},
	{0x80000000u, 0x4d00u, 0x818181u, 0x7u, "ao_m2", LPM_PG_TYPE_OTHER, 0,
		{0x141u,  0x2Au, 0x26u, 0x29u, 0x27u, 0x1F41u,  0x401u, 0x3CFu, 0x3F2u, 0x3DEu, 0x51u,  0xAu,  0x6u, 0xF00u}},
	{0x80000000u, 0x4e00u, 0x818181u, 0x7u, "ao_m3", LPM_PG_TYPE_OTHER, 0,
		{0x1A11u, 0x2Au, 0x26u, 0x29u, 0x27u, 0x28B11u, 0x401u, 0x3CFu, 0x3F2u, 0x3DEu, 0x689u, 0xCu,  0x6u, 0xF00u}},
	{0x80000000u, 0x4f00u, 0x818181u, 0x7u, "ao_m4", LPM_PG_TYPE_OTHER, 0,
		{0x1Cu,   0x33u, 0x2Fu, 0x32u, 0x30u, 0x1F4u,   0x40Bu, 0x3D9u, 0x3FBu, 0x3E7u, 0xCu,   0x1Du, 0x13u, 0x10u}},
	// aic
	{0xC21C0000u, 0x4c00u, 0x818181u, 0x7u, "aic0", LPM_PG_TYPE_AIC, BIT(0),
		{0x140Au, 0x34u, 0x1Cu, 0x33u, 0x1Du, 0x1F482u, 0x4FBu, 0x2D5u, 0x4F1u, 0x2DFu, 0x50Au, 0xCu, 0x6u, 0xF00u}},
	{0xC23C0000u, 0x4c00u, 0x818181u, 0x7u, "aic1", LPM_PG_TYPE_AIC, BIT(1),
		{0x140Au, 0x34u, 0x1Cu, 0x33u, 0x1Du, 0x1F482u, 0x4FBu, 0x2D5u, 0x4F1u, 0x2DFu, 0x50Au, 0xCu, 0x6u, 0xF00u}},
	{0xC25C0000u, 0x4c00u, 0x818181u, 0x7u, "aic2", LPM_PG_TYPE_AIC, BIT(2),
		{0x140Au, 0x34u, 0x1Cu, 0x33u, 0x1Du, 0x1F482u, 0x4FBu, 0x2D5u, 0x4F1u, 0x2DFu, 0x50Au, 0xCu, 0x6u, 0xF00u}},
	{0xC27C0000u, 0x4c00u, 0x818181u, 0x7u, "aic3", LPM_PG_TYPE_AIC, BIT(3),
		{0x140Au, 0x34u, 0x1Cu, 0x33u, 0x1Du, 0x1F482u, 0x4FBu, 0x2D5u, 0x4F1u, 0x2DFu, 0x50Au, 0xCu, 0x6u, 0xF00u}},
	{0xC29C0000u, 0x4c00u, 0x818181u, 0x7u, "aic4", LPM_PG_TYPE_AIC, BIT(4),
		{0x140Au, 0x34u, 0x1Cu, 0x33u, 0x1Du, 0x1F482u, 0x4FBu, 0x2D5u, 0x4F1u, 0x2DFu, 0x50Au, 0xCu, 0x6u, 0xF00u}},
	{0xC2BC0000u, 0x4c00u, 0x818181u, 0x7u, "aic5", LPM_PG_TYPE_AIC, BIT(5),
		{0x140Au, 0x34u, 0x1Cu, 0x33u, 0x1Du, 0x1F482u, 0x4FBu, 0x2D5u, 0x4F1u, 0x2DFu, 0x50Au, 0xCu, 0x6u, 0xF00u}},
	{0xC2DC0000u, 0x4c00u, 0x818181u, 0x7u, "aic6", LPM_PG_TYPE_AIC, BIT(6),
		{0x140Au, 0x34u, 0x1Cu, 0x33u, 0x1Du, 0x1F482u, 0x4FBu, 0x2D5u, 0x4F1u, 0x2DFu, 0x50Au, 0xCu, 0x6u, 0xF00u}},
	{0xC2FC0000u, 0x4c00u, 0x818181u, 0x7u, "aic7", LPM_PG_TYPE_AIC, BIT(7),
		{0x140Au, 0x34u, 0x1Cu, 0x33u, 0x1Du, 0x1F482u, 0x4FBu, 0x2D5u, 0x4F1u, 0x2DFu, 0x50Au, 0xCu, 0x6u, 0xF00u}},
	{0xC31C0000u, 0x4c00u, 0x818181u, 0x7u, "aic8", LPM_PG_TYPE_AIC, BIT(8),
		{0x140Au, 0x34u, 0x1Cu, 0x33u, 0x1Du, 0x1F482u, 0x4FBu, 0x2D5u, 0x4F1u, 0x2DFu, 0x50Au, 0xCu, 0x6u, 0xF00u}},
	{0xC33C0000u, 0x4c00u, 0x818181u, 0x7u, "aic9", LPM_PG_TYPE_AIC, BIT(9),
		{0x140Au, 0x34u, 0x1Cu, 0x33u, 0x1Du, 0x1F482u, 0x4FBu, 0x2D5u, 0x4F1u, 0x2DFu, 0x50Au, 0xCu, 0x6u, 0xF00u}},
	// aiv
	{0xC11C0000u, 0x4c00u, 0x818181u, 0x7u, "aiv0", LPM_PG_TYPE_AIV, BIT(0),
		{0x1D51u, 0x2Au, 0x26u, 0x29u, 0x27u, 0x2DC71u, 0x401u, 0x3CFu, 0x3F2u, 0x3DEu, 0x759u, 0xCu, 0x6u, 0xF00u}},
	{0xC13C0000u, 0x4c00u, 0x818181u, 0x7u, "aiv1", LPM_PG_TYPE_AIV, BIT(1),
		{0x1D51u, 0x2Au, 0x26u, 0x29u, 0x27u, 0x2DC71u, 0x401u, 0x3CFu, 0x3F2u, 0x3DEu, 0x759u, 0xCu, 0x6u, 0xF00u}},
	{0xC15C0000u, 0x4c00u, 0x818181u, 0x7u, "aiv2", LPM_PG_TYPE_AIV, BIT(2),
		{0x1D51u, 0x2Au, 0x26u, 0x29u, 0x27u, 0x2DC71u, 0x401u, 0x3CFu, 0x3F2u, 0x3DEu, 0x759u, 0xCu, 0x6u, 0xF00u}},
	{0xC17C0000u, 0x4c00u, 0x818181u, 0x7u, "aiv3", LPM_PG_TYPE_AIV, BIT(3),
		{0x1D51u, 0x2Au, 0x26u, 0x29u, 0x27u, 0x2DC71u, 0x401u, 0x3CFu, 0x3F2u, 0x3DEu, 0x759u, 0xCu, 0x6u, 0xF00u}},
	{0xC19C0000u, 0x4c00u, 0x818181u, 0x7u, "aiv4", LPM_PG_TYPE_AIV, BIT(4),
		{0x1D51u, 0x2Au, 0x26u, 0x29u, 0x27u, 0x2DC71u, 0x401u, 0x3CFu, 0x3F2u, 0x3DEu, 0x759u, 0xCu, 0x6u, 0xF00u}},
	{0xC1BC0000u, 0x4c00u, 0x818181u, 0x7u, "aiv5", LPM_PG_TYPE_AIV, BIT(5),
		{0x1D51u, 0x2Au, 0x26u, 0x29u, 0x27u, 0x2DC71u, 0x401u, 0x3CFu, 0x3F2u, 0x3DEu, 0x759u, 0xCu, 0x6u, 0xF00u}},
	{0xC1DC0000u, 0x4c00u, 0x818181u, 0x7u, "aiv6", LPM_PG_TYPE_AIV, BIT(6),
		{0x1D51u, 0x2Au, 0x26u, 0x29u, 0x27u, 0x2DC71u, 0x401u, 0x3CFu, 0x3F2u, 0x3DEu, 0x759u, 0xCu, 0x6u, 0xF00u}},
	{0xC1FC0000u, 0x4c00u, 0x818181u, 0x7u, "aiv7", LPM_PG_TYPE_AIV, BIT(7),
		{0x1D51u, 0x2Au, 0x26u, 0x29u, 0x27u, 0x2DC71u, 0x401u, 0x3CFu, 0x3F2u, 0x3DEu, 0x759u, 0xCu, 0x6u, 0xF00u}},
	// cpu
	{0x811B0000u, 0x4c00u, 0x818181u, 0x7u, "cpu_cluster0", LPM_PG_TYPE_CPU, GENMASK(3u, 0u),
		{0x1EF2u, 0x2Au, 0x26u, 0x29u, 0x27u, 0x30522u, 0x401u, 0x3CFu, 0x3F2u, 0x3DEu, 0x7C2u, 0xCu, 0x6u, 0xF00u}},
	{0x811C0000u, 0x4c00u, 0x818181u, 0x7u, "cpu_cluster1", LPM_PG_TYPE_CPU, GENMASK(7u, 4u),
		{0x1EF2u, 0x2Au, 0x26u, 0x29u, 0x27u, 0x30522u, 0x401u, 0x3CFu, 0x3F2u, 0x3DEu, 0x7C2u, 0xCu, 0x6u, 0xF00u}},
	{0x811D0000u, 0x4c00u, 0x818181u, 0x7u, "cpu_cluster2", LPM_PG_TYPE_CPU, GENMASK(11u, 8u),
		{0x1EF2u, 0x2Au, 0x26u, 0x29u, 0x27u, 0x30522u, 0x401u, 0x3CFu, 0x3F2u, 0x3DEu, 0x7C2u, 0xCu, 0x6u, 0xF00u}},
	{0x811E0000u, 0x4c00u, 0x818181u, 0x7u, "cpu_cluster3", LPM_PG_TYPE_CPU, GENMASK(15u, 12u),
		{0x1EF2u, 0x2Au, 0x26u, 0x29u, 0x27u, 0x30522u, 0x401u, 0x3CFu, 0x3F2u, 0x3DEu, 0x7C2u, 0xCu, 0x6u, 0xF00u}},
	// dvpp
	{0xc90c0000u, 0x4c00u, 0x818181u, 0x7u, "dvpp", LPM_PG_TYPE_OTHER, 0,
		{0x1A11u, 0x2Au, 0x26u, 0x29u, 0x27u, 0x28B11u, 0x401u, 0x3CFu, 0x3F2u, 0x3DEu, 0x689u, 0xCu, 0x6u, 0xF00u}},
	// io
	{0xa20c0000u, 0x4c00u, 0x818181u, 0x7u, "io0", LPM_PG_TYPE_OTHER, 0,
		{0x104Au, 0x2Au, 0x26u, 0x29u, 0x27u, 0x196EAu, 0x401u, 0x3CFu, 0x3F2u, 0x3DEu, 0x41Au, 0xCu, 0x6u, 0xF00u}},
	{0xa20c0000u, 0x4d00u, 0x818181u, 0x7u, "io1", LPM_PG_TYPE_OTHER, 0,
		{0x14Cu,  0x2Au, 0x26u, 0x29u, 0x27u, 0x1FD4u, 0x402u, 0x3D0u, 0x3F3u, 0x3DFu, 0x14u,  0x15u, 0xBu, 0x4u}},
	{0xa20c0000u, 0x4e00u, 0x818181u, 0x7u, "io2", LPM_PG_TYPE_OTHER, 0,
		{0x519u,  0x2Au, 0x26u, 0x29u, 0x27u, 0x7F31u, 0x401u, 0x3CFu, 0x3F2u, 0x3DEu, 0x149u, 0xCu, 0x6u, 0xF00u}},
	{0xa20c0000u, 0x4f00u, 0x818181u, 0x7u, "io3", LPM_PG_TYPE_OTHER, 0,
		{0x1461u, 0x2Au, 0x26u, 0x29u, 0x27u, 0x1FCA9u, 0x401u, 0x3CFu, 0x3F2u, 0x3DEu, 0x519u, 0xCu, 0x6u, 0xF00u}},
	// isp
	{0xa80c0000u, 0x4c00u, 0x818181u, 0x7u, "isp0", LPM_PG_TYPE_OTHER, 0,
		{0x104Au, 0x2Au, 0x26u, 0x29u, 0x27u, 0x196EAu, 0x401u, 0x3CFu, 0x3F2u, 0x3DEu, 0x41Au, 0xCu, 0x6u, 0xF00u}},
	{0xa80c0000u, 0x4d00u, 0x818181u, 0x7u, "isp1", LPM_PG_TYPE_OTHER, 0,
		{0x1389u, 0x2Au, 0x26u, 0x29u, 0x27u, 0x1E849u, 0x401u, 0x3CFu, 0x3F2u, 0x3DEu, 0x4E9u, 0xCu, 0x6u, 0xF00u}},
	// ddr
	{0x81320000u, 0x4c00u, 0x818181u, 0x7u, "ddr0", LPM_PG_TYPE_DDR, BIT(0),
		{0x1161u, 0x2Au, 0x26u, 0x29u, 0x27u, 0x1B1F9u, 0x401u, 0x3CFu, 0x3F2u, 0x3DEu, 0x459u, 0xCu, 0x6u, 0xF00u}},
	{0x81310000u, 0x4c00u, 0x818181u, 0x7u, "ddr1", LPM_PG_TYPE_DDR, BIT(1),
		{0x1161u, 0x2Au, 0x26u, 0x29u, 0x27u, 0x1B1F9u, 0x401u, 0x3CFu, 0x3F2u, 0x3DEu, 0x459u, 0xCu, 0x6u, 0xF00u}},
	// peri
	{0xA40C0000u, 0x4d00u, 0x818181u, 0x7u, "peri0", LPM_PG_TYPE_OTHER, 0,
		{0x141u, 0x2Au, 0x26u, 0x29u, 0x27u, 0x1F41u, 0x401u, 0x3CFu, 0x3F2u, 0x3DEu, 0x51u, 0xAu, 0x6u, 0xF00u}},
	{0xA40C0000u, 0x4c00u, 0x818181u, 0x7u, "peri1", LPM_PG_TYPE_OTHER, 0,
		{0xEA9u, 0x2Au, 0x26u, 0x29u, 0x27u, 0x16E39u, 0x401u, 0x3CFu, 0x3F2u, 0x3DEu, 0x3B1u, 0xCu, 0x6u, 0xF00u}},
	// sils
	{0xCF0C0000u, 0x4c00u, 0x818181u, 0x7u, "sils", LPM_PG_TYPE_OTHER, 0,
		{0x10F1u, 0x2Au, 0x26u, 0x29u, 0x27u, 0x1A731u, 0x401u, 0x3CFu, 0x3F2u, 0x3DEu, 0x441u, 0xCu, 0x6u, 0xF00u}}
};

STATIC struct crg_monitor_param g_aic_freq_cfg[] = {
	// 1230MHz
	{0x140Au, 0x34u, 0x1Cu, 0x33u, 0x1Du, 0x1F482u, 0x4FBu, 0x2D5u, 0x4F1u, 0x2DFu, 0x50Au, 0xCu, 0x6u, 0xF00u},
	// 1000MHz
	{0x104Au, 0x34u, 0x1Cu, 0x33u, 0x1Du, 0x196EAu, 0x4FBu, 0x2D5u, 0x4F1u, 0x2DFu, 0x41Au, 0xCu, 0x6u, 0xF00u},
	// 960MHz
	{0x1F41u, 0x34u, 0x1Cu, 0x33u, 0x1Du, 0x30D41u, 0x4FBu, 0x2D5u, 0x4F1u, 0x2DFu, 0x7D1u, 0xAu, 0x6u, 0xF00u},
	// 750MHz
	{0x1871u, 0x34u, 0x1Cu, 0x33u, 0x1Du, 0x26261u, 0x4FBu, 0x2D5u, 0x4F1u, 0x2DFu, 0x621u, 0xCu, 0x6u, 0xF00u}
};

STATIC struct crg_monitor_param g_ddr_freq_cfg[] = {
	// 800MHz
	{0x1A11u, 0x2Au, 0x26u, 0x29u, 0x27u, 0x28B11u, 0x401u, 0x3CFu, 0x3F2u, 0x3DEu, 0x689u, 0xCu, 0x6u, 0xF00u},
	// 687.5MHz
	{0x1669u, 0x2Au, 0x26u, 0x29u, 0x27u, 0x22F81u, 0x401u, 0x3CFu, 0x3F2u, 0x3DEu, 0x5A1u, 0xCu, 0x6u, 0xF00u},
	// 533.25MHz
	{0x1161u, 0x2Au, 0x26u, 0x29u, 0x27u, 0x1B1F9u, 0x401u, 0x3CFu, 0x3F2u, 0x3DEu, 0x459u, 0xCu, 0x6u, 0xF00u},
};

STATIC void lpm_crg_set_reg(void __iomem *base_addr, uint32_t offset, uint32_t reg_mask, bool clr)
{
	uint32_t val;

	val = readl((void __iomem *)((uintptr_t)base_addr + offset));
	if (clr) {
		val &= ~reg_mask;
	} else {
		val |= reg_mask;
	}
	writel(val, (void __iomem *)((uintptr_t)base_addr + offset));
}

// set reg and read back
STATIC int32_t lpm_crg_wb_reg(void __iomem *addr, uint32_t mask, uint32_t ofst, uint32_t val, uint32_t timeout)
{
	uint32_t reg_val = readl((void __iomem *)addr);
	uint32_t reg_val_wb;

	reg_val &= (~mask);
	reg_val |= (val << ofst) & mask;
	writel(reg_val, (void __iomem *)addr);
	usleep_range(timeout, timeout);
	reg_val_wb = readl((void __iomem *)addr);
	if ((reg_val_wb & mask) != (reg_val & mask)) {
		return -1;
	}
	return 0;
}

STATIC void __iomem *lpm_crg_get_vaddr(uintptr_t phy_addr)
{
	uint32_t i;

	for (i = 0; i < (uint32_t)ARRAY_SIZE(g_crg_module_info); i++) {
		if (phy_addr == g_crg_module_info[i].phy_addr) {
			return g_crg_module_info[i].vaddr;
		}
	}

	return NULL;
}

STATIC int32_t lpm_crg_clk_monitor_enable(void __iomem *base_addr)
{
	int32_t ret;

	ret = lpm_crg_wb_reg((void __iomem *)((uintptr_t)base_addr + SC_MONITOR_PLL_DET_REF_EN),
		MONITOR_CFG_PLL_DET_EN, MONITOR_CFG_PLL_DET_EN_SHIFT, 0x1, SC_MONITOR_CONFIG_REG_TIMEOUT);
	if (ret != 0) {
		return -1;
	}

	ret = lpm_crg_wb_reg((void __iomem *)((uintptr_t)base_addr + SC_MONITOR_PLL_DET_REF_SEL),
		MONITOR_PLL_DET_REQ_SFT, MONITOR_PLL_DET_REQ_SFT_SHIFT, 0x0, SC_MONITOR_CONFIG_REG_TIMEOUT);
	if (ret != 0) {
		return -1;
	}

	writel(0x1, (void __iomem *)((uintptr_t)base_addr + CRG_DET_INT_ENABLE_REG));

	return 0;
}

STATIC int32_t lpm_crg_clk_monitor_disable(void __iomem *base_addr)
{
	int32_t ret;

	ret = lpm_crg_wb_reg((void __iomem *)((uintptr_t)base_addr + SC_MONITOR_PLL_DET_REF_SEL),
		MONITOR_PLL_DET_REQ_SFT, MONITOR_PLL_DET_REQ_SFT_SHIFT, 0x4, SC_MONITOR_CONFIG_REG_TIMEOUT);
	if (ret != 0) {
		return -1;
	}

	ret = lpm_crg_wb_reg((void __iomem *)((uintptr_t)base_addr + SC_MONITOR_PLL_DET_REF_EN),
		(uint32_t)MONITOR_CFG_PLL_DET_EN, MONITOR_CFG_PLL_DET_EN_SHIFT, 0, SC_MONITOR_CONFIG_REG_TIMEOUT);
	if (ret != 0) {
		return -1;
	}

	writel(0x1, (void __iomem *)((uintptr_t)base_addr + CRG_DET_INT_ENABLE_REG));
	return 0;
}

STATIC int32_t lpm_crg_enable_ref_clock(void __iomem *base_addr, struct crg_config *crg_moniter_cfg)
{
	int32_t ret;

	ret = lpm_crg_wb_reg((void __iomem *)((uintptr_t)base_addr + SC_MONITOR_PLL_DET_REF_EN),
		MONITOR_PLL_DET_FREQ_CFG, MONITOR_PLL_DET_FREQ_CFG_SHIFT,
		crg_moniter_cfg->ref_clk_cfg, SC_MONITOR_CONFIG_REG_TIMEOUT);
	if (ret != 0) {
		return -1;
	}

	ret = lpm_crg_wb_reg((void __iomem *)((uintptr_t)base_addr + SC_MONITOR_PLL_CLK_EN),
		MONITOR_PLL_DET_CLK_EN_MASK, MONITOR_PLL_DET_CLK_EN_SHIFT,
		crg_moniter_cfg->det_clk_en, SC_MONITOR_CONFIG_REG_TIMEOUT);
	if (ret != 0) {
		return -1;
	}

	return 0;
}

STATIC int32_t lpm_crg_config_fast_mode(void __iomem *base_addr, struct crg_config *crg_moniter_cfg)
{
	int32_t ret;

	ret = lpm_crg_wb_reg((void __iomem *)((uintptr_t)base_addr + SC_MONITOR_PLL_FAST_OOF_DET_DIV),
		MONITOR_PLL_CFG_FAST_OOF_DET_MASK, MONITOR_PLL_CFG_FAST_OOF_DET_SHIFT,
		crg_moniter_cfg->param.cfg_fast_oof_det_div, SC_MONITOR_CONFIG_REG_TIMEOUT);
	if (ret != 0) {
		return -1;
	}

	ret = lpm_crg_wb_reg((void __iomem *)((uintptr_t)base_addr + SC_MONITOR_PLL_FAST_OOF_ALM_OVER_THR),
		SC_MONITOR_CFG_FAST_OOF_ALM_OVER_MASK, SC_MONITOR_CFG_FAST_OOF_ALM_OVER_SHIFT,
		crg_moniter_cfg->param.cfg_fast_oof_alm_over_thr, SC_MONITOR_CONFIG_REG_TIMEOUT);
	if (ret != 0) {
		return -1;
	}

	ret = lpm_crg_wb_reg((void __iomem *)((uintptr_t)base_addr + SC_MONITOR_PLL_FAST_OOF_ALM_BELOW_THR),
		SC_MONITOR_CFG_FAST_OOF_ALM_BELOW_MASK, SC_MONITOR_CFG_FAST_OOF_ALM_BELOW_SHIFT,
		crg_moniter_cfg->param.cfg_fast_oof_alm_below_thr, SC_MONITOR_CONFIG_REG_TIMEOUT);
	if (ret != 0) {
		return -1;
	}

	ret = lpm_crg_wb_reg((void __iomem *)((uintptr_t)base_addr + SC_MONITOR_PLL_FAST_OOF_NOALM_OVER_THR),
		SC_MONITOR_CFG_FAST_OOF_NOALM_OVER_MASK, SC_MONITOR_CFG_FAST_OOF_NOALM_OVER_SHIFT,
		crg_moniter_cfg->param.cfg_fast_oof_noalm_over_thr, SC_MONITOR_CONFIG_REG_TIMEOUT);
	if (ret != 0) {
		return -1;
	}

	ret = lpm_crg_wb_reg((void __iomem *)((uintptr_t)base_addr + SC_MONITOR_PLL_FAST_OOF_NOALM_BELOW_THR),
		SC_MONITOR_CFG_FAST_OOF_NOALM_BELOW_MASK, SC_MONITOR_CFG_FAST_OOF_NOALM_BELOW_SHIFT,
		crg_moniter_cfg->param.cfg_fast_oof_noalm_below_thr, SC_MONITOR_CONFIG_REG_TIMEOUT);
	if (ret != 0) {
		return -1;
	}

	return 0;
}

STATIC int32_t lpm_crg_config_pre_mode(void __iomem *base_addr, struct crg_config *crg_moniter_cfg)
{
	int32_t ret;

	ret = lpm_crg_wb_reg((void __iomem *)((uintptr_t)base_addr + SC_MONITOR_PLL_PRE_OOF_DET_DIV_H),
		SC_MONITOR_CFG_PRE_OOF_DET_DIV_MASK, SC_MONITOR_CFG_PRE_OOF_DET_DIV_SHIFT,
		crg_moniter_cfg->param.cfg_pre_oof_det_div, SC_MONITOR_CONFIG_REG_TIMEOUT);
	if (ret != 0) {
		return -1;
	}

	ret = lpm_crg_wb_reg((void __iomem *)((uintptr_t)base_addr + SC_MONITOR_PLL_PRE_FAST_OOF_ALM_OVER_THR),
		SC_MONITOR_PLL_CFG_PRE_OOF_ALM_OVER_THR_MASK, SC_MONITOR_PLL_CFG_PRE_OOF_ALM_OVER_THR_SHIFT,
		crg_moniter_cfg->param.cfg_pre_oof_alm_over_thr, SC_MONITOR_CONFIG_REG_TIMEOUT);
	if (ret != 0) {
		return -1;
	}

	ret = lpm_crg_wb_reg((void __iomem *)((uintptr_t)base_addr + SC_MONITOR_PLL_PRE_FAST_OOF_ALM_BELOW_THR),
		SC_MONITOR_PLL_CFG_PRE_OOF_ALM_BELOW_THR_MASK, SC_MONITOR_PLL_CFG_PRE_OOF_ALM_BELOW_THR_SHIFT,
		crg_moniter_cfg->param.cfg_pre_oof_alm_below_thr, SC_MONITOR_CONFIG_REG_TIMEOUT);
	if (ret != 0) {
		return -1;
	}

	ret = lpm_crg_wb_reg((void __iomem *)((uintptr_t)base_addr + SC_MONITOR_PLL_PRE_FAST_OOF_NOALM_OVER_THR),
		SC_MONITOR_PLL_CFG_PRE_OOF_NOALM_OVER_THR_MASK, SC_MONITOR_PLL_CFG_PRE_OOF_NOALM_OVER_THR_SHIFT,
		crg_moniter_cfg->param.cfg_pre_oof_noalm_over_thr, SC_MONITOR_CONFIG_REG_TIMEOUT);
	if (ret != 0) {
		return -1;
	}

	ret = lpm_crg_wb_reg((void __iomem *)((uintptr_t)base_addr + SC_MONITOR_PLL_PRE_FAST_OOF_NOALM_BELOW_THR),
		SC_MONITOR_PLL_CFG_PRE_OOF_NOALM_BELOW_THR_MASK, SC_MONITOR_PLL_CFG_PRE_OOF_NOALM_BELOW_THR_SHIFT,
		crg_moniter_cfg->param.cfg_pre_oof_noalm_below_thr, SC_MONITOR_CONFIG_REG_TIMEOUT);
	if (ret != 0) {
		return -1;
	}

	return 0;
}

STATIC int32_t lpm_crg_config_los_mode(void __iomem *base_addr, struct crg_config *crg_moniter_cfg)
{
	int32_t ret;

	ret = lpm_crg_wb_reg((void __iomem *)((uintptr_t)base_addr + SC_MONITOR_PLL_LOS_DET_DIV),
		SC_MONITOR_PLL_CFG_LOS_DET_DIV_MASK, SC_MONITOR_PLL_CFG_LOS_DET_DIV_SHIFT,
		crg_moniter_cfg->param.cfg_los_det_div, SC_MONITOR_CONFIG_REG_TIMEOUT);
	if (ret != 0) {
		return -1;
	}

	ret = lpm_crg_wb_reg((void __iomem *)((uintptr_t)base_addr + SC_MONITOR_PLL_LOS_ALM_THR),
		SC_MONITOR_PLL_CFG_LOS_ALM_THR_MASK, SC_MONITOR_PLL_CFG_LOS_ALM_THR_SHIFT,
		crg_moniter_cfg->param.cfg_los_alm_thr, SC_MONITOR_CONFIG_REG_TIMEOUT);
	if (ret != 0) {
		return -1;
	}

	ret = lpm_crg_wb_reg((void __iomem *)((uintptr_t)base_addr + SC_MONITOR_PLL_LOS_NOALM_THR),
		SC_MONITOR_PLL_CFG_LOS_NOAML_THR_MASK, SC_MONITOR_PLL_CFG_LOS_NORML_THR_SHIFT,
		crg_moniter_cfg->param.cfg_los_noalm_thr, SC_MONITOR_CONFIG_REG_TIMEOUT);
	if (ret != 0) {
		return -1;
	}

	ret = lpm_crg_wb_reg((void __iomem *)((uintptr_t)base_addr + SC_MONITOR_PLL_LOS_NOINT_THR),
		SC_MONITOR_PLL_CFG_LOS_NOINT_THR_MASK, SC_MONITOR_PLL_CFG_LOS_NOINT_THR_SHIFT,
		crg_moniter_cfg->param.cfg_los_noint_thr, SC_MONITOR_CONFIG_REG_TIMEOUT);
	if (ret != 0) {
		return -1;
	}

	return 0;
}

STATIC int32_t lpm_crg_config_alarm_mask(void __iomem *base_addr, struct crg_config *crg_moniter_cfg)
{
	int32_t ret;

	ret = lpm_crg_wb_reg((void __iomem *)((uintptr_t)base_addr + (uint64_t)SC_MONITOR_PLL_DET_INT_MASK),
		SC_MONITOR_PLL_CFG_DET_INT_MASK, SC_MONITOR_PLL_CFG_DET_INT_SHIFT,
		0x0u, SC_MONITOR_CONFIG_REG_TIMEOUT);
	if (ret != 0) {
		return -1;
	}

	writel(0x1, (void __iomem *)((uintptr_t)base_addr + CRG_DET_INT_ENABLE_REG));

	return 0;
}

STATIC int32_t lpm_crg_config_rst_monitor(void __iomem *base_addr)
{
	int32_t ret;

	// Step1 clear origin interrupt
	ret = lpm_crg_wb_reg((void __iomem *)((uintptr_t)base_addr + SC_MONITOR_CFG_RST_DET_REG),
		SC_MONITOR_CFG_RST_DET, SC_MONITOR_CFG_RST_DET_SHIFT, 0x0, SC_MONITOR_CONFIG_REG_TIMEOUT);
	if (ret != 0) {
		return -1;
	}
	writel(0x1, (void __iomem *)((uintptr_t)base_addr + CRG_DET_INT_ENABLE_REG));

	ret = lpm_crg_wb_reg((void __iomem *)((uintptr_t)base_addr + SC_MONITOR_CFG_RST_DET_REG),
		SC_MONITOR_CFG_RST_DET, SC_MONITOR_CFG_RST_DET_SHIFT, 0x4, SC_MONITOR_CONFIG_REG_TIMEOUT);
	if (ret != 0) {
		return -1;
	}
	writel(0x1, (void __iomem *)((uintptr_t)base_addr + CRG_DET_INT_ENABLE_REG));

	// Step2 enable rst monitor
	ret = lpm_crg_wb_reg((void __iomem *)((uintptr_t)base_addr + SC_MONITOR_CFG_RST_DET_REG),
		SC_MONITOR_CFG_RST_DET, SC_MONITOR_CFG_RST_DET_SHIFT, 0x0, SC_MONITOR_CONFIG_REG_TIMEOUT);
	if (ret != 0) {
		return -1;
	}
	writel(0x1, (void __iomem *)((uintptr_t)base_addr + CRG_DET_INT_ENABLE_REG));

	ret = lpm_crg_wb_reg((void __iomem *)((uintptr_t)base_addr + SC_MONITOR_CFG_RST_DET_REG),
		SC_MONITOR_CFG_RST_DET, SC_MONITOR_CFG_RST_DET_SHIFT, 0x3, SC_MONITOR_CONFIG_REG_TIMEOUT);
	if (ret != 0) {
		return -1;
	}
	writel(0x1, (void __iomem *)((uintptr_t)base_addr + CRG_DET_INT_ENABLE_REG));

	ret = lpm_crg_wb_reg((void __iomem *)((uintptr_t)base_addr + SC_MONITOR_CFG_RST_DET_REG),
		SC_MONITOR_CFG_RST_DET, SC_MONITOR_CFG_RST_DET_SHIFT, 0x7, SC_MONITOR_CONFIG_REG_TIMEOUT);
	if (ret != 0) {
		return -1;
	}
	writel(0x1, (void __iomem *)((uintptr_t)base_addr + CRG_DET_INT_ENABLE_REG));
	return 0;
}

STATIC int32_t lpm_crg_config_specify_monitor(struct crg_config *crg_moniter_cfg)
{
	int32_t ret;
	void __iomem *vaddr = lpm_crg_get_vaddr(crg_moniter_cfg->phy_base_addr);
	void __iomem *base_addr = NULL;

	if (vaddr == NULL) {
		lpm_log_err("paddr not mapping\n");
		return -1;
	}
	base_addr = (void __iomem *)((uintptr_t)vaddr + crg_moniter_cfg->monitor_ofst);

	// Step1 config clk monitor shutdown
	ret = lpm_crg_clk_monitor_disable(base_addr);
	if (ret != 0) {
		lpm_log_err("lpm_crg_clk_monitor_disable failed\n");
		return -1;
	}

	// Step2 config point signal
	ret = lpm_crg_enable_ref_clock(base_addr, crg_moniter_cfg);
	if (ret != 0) {
		lpm_log_err("lpm_fault_enable_ref_clock failed\n");
		return -1;
	}

	ret = lpm_crg_config_fast_mode(base_addr, crg_moniter_cfg);
	if (ret != 0) {
		lpm_log_err("lpm_crg_config_fast_mode failed\n");
		return -1;
	}

	ret = lpm_crg_config_pre_mode(base_addr, crg_moniter_cfg);
	if (ret != 0) {
		lpm_log_err("lpm_crg_config_pre_mode failed\n");
		return -1;
	}

	ret = lpm_crg_config_los_mode(base_addr, crg_moniter_cfg);
	if (ret != 0) {
		lpm_log_err("lpm_crg_config_los_mode failed\n");
		return -1;
	}

	ret = lpm_crg_config_alarm_mask(base_addr, crg_moniter_cfg);
	if (ret != 0) {
		lpm_log_err("lpm_crg_config_alarm_mask failed\n");
		return -1;
	}

	// Step3 enable clock monitor
	ret = lpm_crg_clk_monitor_enable(base_addr);
	if (ret != 0) {
		lpm_log_err("lpm_crg_clk_monitor_enable failed\n");
		return -1;
	}

	// init rst monitor
	ret = lpm_crg_config_rst_monitor(base_addr);
	if (ret != 0) {
		lpm_log_err("lpm_crg_config_rst_monitor failed\n");
		return -1;
	}

	return 0;
}

STATIC void lpm_fault_safety_irq_hw_mask_cfg(struct crg_config *crg_moniter_cfg, bool en)
{
	void __iomem *vaddr = lpm_crg_get_vaddr(crg_moniter_cfg->phy_base_addr);
	void __iomem *base_addr = NULL;

	if (vaddr == NULL) {
		lpm_log_err("lpm_fault_safety_irq_hw_mask_cfg paddr not mapping\n");
		return;
	}
	base_addr = (void __iomem *)((uintptr_t)vaddr + crg_moniter_cfg->monitor_ofst);

	if (en) {
		// clear err irq before enable crg monitor.
		writel(CRG_CLK_MASK_OFFSET | CRG_RST_MASK_OFFSET,
			(void __iomem *)((uintptr_t)base_addr + (uintptr_t)CRG_DET_CLR_REG));
		writel(0x1, (void __iomem *)((uintptr_t)base_addr + (uintptr_t)CRG_DET_INT_ENABLE_REG));
	}

	lpm_crg_set_reg(base_addr, CRG_DET_ERR_MASK_REG,
		(CRG_CLK_MASK_OFFSET | CRG_RST_MASK_OFFSET), en);
	lpm_crg_set_reg(base_addr, CRG_DET_FUNC_MASK_REG,
		(CRG_CLK_MASK_OFFSET | CRG_RST_MASK_OFFSET), en);

	writel(0x1, (void __iomem *)((uintptr_t)base_addr + (uintptr_t)CRG_DET_INT_ENABLE_REG));
}

STATIC void lpm_fault_safety_irq_mask_errpin(void)
{
	uint32_t i;
	void __iomem *vaddr = NULL;
	void __iomem *base_addr = NULL;

	for (i = 0; i < (uint32_t)ARRAY_SIZE(g_crg_moniter_cfg); i++) {
		vaddr = lpm_crg_get_vaddr(g_crg_moniter_cfg[i].phy_base_addr);
		if (vaddr == NULL) {
			lpm_log_err("lpm_fault_safety_irq_hw_mask_cfg paddr not mapping\n");
			continue;
		}
		base_addr = (void __iomem *)((uintptr_t)vaddr + (uintptr_t)g_crg_moniter_cfg[i].monitor_ofst);

		lpm_crg_set_reg(base_addr, CRG_DET_ERR_MASK_REG,
			(CRG_CLK_MASK_OFFSET | CRG_RST_MASK_OFFSET), false);
		lpm_crg_set_reg(base_addr, CRG_DET_FUNC_MASK_REG,
			(CRG_CLK_MASK_OFFSET | CRG_RST_MASK_OFFSET), false);
		writel(0x1, (void __iomem *)((uintptr_t)base_addr + (uintptr_t)CRG_DET_INT_ENABLE_REG));
	}
}

STATIC uint64_t lpm_get_pg_mask_by_type(enum lpm_pg_module_type type, const struct lpm_pg_info *pg_info)
{
	uint64_t mask;

	switch (type) {
	case LPM_PG_TYPE_CPU:
		mask = pg_info->cpu_mask;
		break;
	case LPM_PG_TYPE_AIC:
		mask = pg_info->aic_mask;
		break;
	case LPM_PG_TYPE_AIV:
		mask = pg_info->aiv_mask;
		break;
	case LPM_PG_TYPE_DDR:
		// DDR use all enable mask
		mask = (uint64_t)0x3U;
		break;
	default:
		mask = LPM_PG_INFO_MASK_INVAILID;
		break;
	}

	return mask;
}

STATIC enum crg_aic_freq lpm_crg_get_aic_freq(uint64_t aic_freq)
{
	if (aic_freq == 1230ULL) {
		return LPM_CRG_AIC_1230M;
	} else if (aic_freq == 1000ULL) {
		return LPM_CRG_AIC_1230M;
	} else if (aic_freq == 960ULL) {
		return LPM_CRG_AIC_960M;
	} else if (aic_freq == 750ULL) {
		return LPM_CRG_AIC_750M;
	} else {
		return LPM_CRG_AIC_INVALID;
	}
}

STATIC void lpm_crg_moniter_fusion_pginfo(struct lpm_pg_info *pg_info)
{
	uint32_t i;
	enum crg_aic_freq freq;

	freq = lpm_crg_get_aic_freq(pg_info->aic_freq);
	// if aic frequrence unrecognized, invalid aic crg monitor by unmask all aic
	if (freq == LPM_CRG_AIC_INVALID) {
		for (i = 0; i < (uint32_t)ARRAY_SIZE(g_crg_moniter_cfg); i++) {
			if (g_crg_moniter_cfg[i].type == LPM_PG_TYPE_AIC) {
				g_crg_moniter_cfg[i].mask = 0u;
			}
		}
		lpm_log_err("AIC get freq=%llu invalid\n", pg_info->aic_freq);
		return;
	}

	for (i = 0; i < (uint32_t)ARRAY_SIZE(g_crg_moniter_cfg); i++) {
		if (g_crg_moniter_cfg[i].type != LPM_PG_TYPE_AIC) {
			continue;
		}

		(void)memcpy_s(&g_crg_moniter_cfg[i].param, sizeof(struct crg_monitor_param),
			&g_aic_freq_cfg[freq], sizeof(struct crg_monitor_param));
	}
}

STATIC enum crg_ddr_freq lpm_crg_moniter_get_ddrfreq(void)
{
	static enum crg_ddr_freq ddr_crg_freq = LPM_CRG_DDR_INVALID;
	uint32_t ddr_freq;
	void __iomem *ddr_freq_vaddr = NULL;

	if (ddr_crg_freq != LPM_CRG_DDR_INVALID) {
		return ddr_crg_freq;
	}

	ddr_freq_vaddr = (void __iomem *)ioremap(LPM_CRG_DDR_FREQ_BASE_ADDR, LPM_CRG_DDR_FREQ_ADDR_SIZE);
	if (ddr_freq_vaddr == NULL) {
		lpm_log_err("ioremap ddr base addr failed\n");
		return LPM_CRG_DDR_INVALID;
	}

	ddr_freq = readl(ddr_freq_vaddr);
	if ((ddr_freq < (LPM_CRG_DDR_FREQ_2745M + LPM_CRG_DDR_FREQ_DEVIATION)) &&
		(ddr_freq > (LPM_CRG_DDR_FREQ_2745M - LPM_CRG_DDR_FREQ_DEVIATION))) {
		ddr_crg_freq = LPM_CRG_DDR_687M;
	} else if ((ddr_freq < (LPM_CRG_DDR_FREQ_2133M + LPM_CRG_DDR_FREQ_DEVIATION)) &&
			   (ddr_freq > (LPM_CRG_DDR_FREQ_2133M - LPM_CRG_DDR_FREQ_DEVIATION))) {
		ddr_crg_freq = LPM_CRG_DDR_533M;
	} else if ((ddr_freq < (LPM_CRG_DDR_FREQ_3200M + LPM_CRG_DDR_FREQ_DEVIATION)) &&
			   (ddr_freq > (LPM_CRG_DDR_FREQ_3200M - LPM_CRG_DDR_FREQ_DEVIATION))) {
		ddr_crg_freq = LPM_CRG_DDR_800M;
	} else if ((ddr_freq < (LPM_CRG_DDR_FREQ_1600M + LPM_CRG_DDR_FREQ_DEVIATION)) &&
			   (ddr_freq > (LPM_CRG_DDR_FREQ_1600M - LPM_CRG_DDR_FREQ_DEVIATION))) {
		// 1600MHz use same freq with 3200MHz
		ddr_crg_freq = LPM_CRG_DDR_800M;
	} else {
		ddr_crg_freq = LPM_CRG_DDR_INVALID;
	}

	iounmap(ddr_freq_vaddr);
	lpm_log_info("DDR get freq=%u, enum=%u\n", ddr_freq, ddr_crg_freq);
	return ddr_crg_freq;
}

STATIC void lpm_crg_moniter_fusion_ddrfreq(void)
{
	enum crg_ddr_freq freq = lpm_crg_moniter_get_ddrfreq();
	uint32_t i;

	// if ddr frequrence unrecognized, invalid ddr crg monitor by unmask all ddr
	if (freq >= LPM_CRG_DDR_INVALID) {
		for (i = 0; i < (uint32_t)ARRAY_SIZE(g_crg_moniter_cfg); i++) {
			if (g_crg_moniter_cfg[i].type == LPM_PG_TYPE_DDR) {
				g_crg_moniter_cfg[i].mask = 0u;
			}
		}
		lpm_log_err("DDR get freq invalid\n");
		return;
	}

	for (i = 0; i < (uint32_t)ARRAY_SIZE(g_crg_moniter_cfg); i++) {
		if (g_crg_moniter_cfg[i].type != LPM_PG_TYPE_DDR) {
			continue;
		}

		(void)memcpy_s(&g_crg_moniter_cfg[i].param, sizeof(struct crg_monitor_param),
			&g_ddr_freq_cfg[freq], sizeof(struct crg_monitor_param));
	}
}

STATIC int32_t lpm_crg_moniter_config(void)
{
	uint32_t i;
	int32_t ret;
	uint64_t mask;
	struct lpm_pg_info *pg_info = NULL;

	// add pg info
	pg_info = lpm_crg_monitor_assemble_pginfo();
	lpm_crg_moniter_fusion_pginfo(pg_info);
	lpm_crg_moniter_fusion_ddrfreq();

	for (i = 0; i < (uint32_t)ARRAY_SIZE(g_crg_moniter_cfg); i++) {
		if (g_crg_moniter_cfg[i].type != LPM_PG_TYPE_OTHER) {
			mask = lpm_get_pg_mask_by_type(g_crg_moniter_cfg[i].type, pg_info);
			if ((g_crg_moniter_cfg[i].mask & mask) != g_crg_moniter_cfg[i].mask) {
				lpm_log_warn("lpm %s monitor not config for pg, idx:%u, mask:0x%llx:0x%llx\n",
					g_crg_moniter_cfg[i].module_name, i, g_crg_moniter_cfg[i].mask, mask);
				continue;
			}
		}
		ret = lpm_crg_config_specify_monitor(&g_crg_moniter_cfg[i]);
		if (ret != 0) {
			lpm_log_warn("lpm %s monitor not config, idx:%u\n", g_crg_moniter_cfg[i].module_name, i);
			continue;
		} else {
			lpm_log_info("lpm %s monitor config succ\n", g_crg_moniter_cfg[i].module_name);
		}

		lpm_fault_safety_irq_hw_mask_cfg(&g_crg_moniter_cfg[i], true);
	}
	return 0;
}

STATIC int32_t lpm_safety_report_fault(uint32_t dev_id, uint32_t sensor_type, uint32_t event_type, bool assertion)
{
	struct lpm_inner_fault_event event = {0};
	int32_t ret;

	event.dev_id = (uint8_t)dev_id;
	event.assertion = assertion ? (uint32_t)DMS_EVENT_TYPE_OCCUR : (uint32_t)DMS_EVENT_TYPE_RESUME;
	event.node_type = DMS_DEV_TYPE_LPM;
	event.sensor_type = sensor_type;
	event.event_type = event_type;

	ret = lpm_handle_fault(&event);
	if (ret != 0) {
		lpm_log_err("safety report fault err, dev_id=%u, sensor_type=%u, event_type=%u, assertion=%u\n",
			dev_id, sensor_type, event_type, assertion);
		return ret;
	}
	return 0;
}

STATIC int32_t lpm_safety_check_irq_param(struct safety_fault_info *safety_fault,
	uint32_t *event_num, struct safety_event **event_list)
{
	if (safety_fault == NULL) {
		lpm_log_err("safety fault is null\n");
		return -1;
	}

	if (event_num == NULL) {
		lpm_log_err("event_num is null\n");
		return -1;
	}

	if (event_list == NULL) {
		lpm_log_err("event_list is null\n");
		return -1;
	}

	if (safety_fault->fault_status_list == NULL) {
		lpm_log_err("fault status list is null\n");
		return -1;
	}

	return 0;
}

STATIC int32_t lpm_safety_fill_event_data(struct safety_fault_info *safety_fault,
	uint32_t *event_num, struct safety_event **event_list)
{
	uint32_t event_list_size = (uint32_t)(sizeof(struct safety_event) * LP_SAFETY_MAX_EVENT_NUM);
	struct safety_event *fault_event = NULL;
	uint32_t i;

	// the memory alloc here will be free by caller.
	*event_num = 0;
	*event_list = (struct safety_event *)kzalloc(event_list_size, GFP_KERNEL);
	if ((*event_list) == NULL) {
		lpm_log_err("kzalloc lpm safety event list is null\n");
		return -1;
	}

	for (i = 0; i < safety_fault->fault_reg_num; i++) {
		if (i >= LP_SAFETY_MAX_EVENT_NUM) {
			break;
		}
		fault_event = &(*event_list)[i];
		fault_event->node_type = DMS_DEV_TYPE_LPM;
		fault_event->sensor_type = (uint8_t)DMS_SEN_TYPE_RAS_SENSOR;
		fault_event->event_type = (unsigned short)LP_SAFETY_EVENT_MODULE_ERROR_CANNOT_FIXED;
		fault_event->node_id = (uint8_t)0;  // the index of devices
		fault_event->sub_node_type = (uint8_t)0;
		fault_event->sub_node_id = (uint8_t)0;
		fault_event->event_severity = 0x3; // severity of CRG fault is 0x3
		fault_event->event_assertion = (uint8_t)DFM_EVENT_OCCUR;  // 0:RESUME 1:OCCUR 2:ONE_TIME

		fault_event->emu_id = safety_fault->fault_status_list[i].emu_id;
		fault_event->src_id = safety_fault->fault_status_list[i].src_id;
		fault_event->bit_id = safety_fault->fault_status_list[i].bit_id;
		*event_num = (*event_num) + 1U;
	}
	return 0;
}

STATIC void lpm_safety_crg_priv_proc(struct safety_fault_info *safety_fault)
{
	uint32_t i;
	uintptr_t base_offset;
	void __iomem *vbase_addr = NULL;

	for (i = 0; i < safety_fault->fault_reg_num; i++) {
		vbase_addr = lpm_crg_get_vaddr((uintptr_t)safety_fault->fault_status_list[i].base_paddr);
		if (vbase_addr == NULL) {
			lpm_log_err("lpm_crg_get_vaddr failed\n");
			continue;
		}

		base_offset = (uintptr_t)safety_fault->fault_status_list[i].fault_status_offset - (uintptr_t)CRG_DET_INT_STATUS_REG;

		// clear err irq before enable crg monitor
		writel(CRG_CLK_MASK_OFFSET | CRG_RST_MASK_OFFSET,
			(void __iomem *)((uintptr_t)vbase_addr + (base_offset + (uintptr_t)CRG_DET_CLR_REG)));
		writel(0x1, (void __iomem *)((uintptr_t)vbase_addr + (base_offset + (uintptr_t)CRG_DET_INT_ENABLE_REG)));

		// mask the err mask after alarm occur, cause the alarm is unfixable. Shutdown to avoid interruption storms.
		lpm_crg_set_reg((void __iomem *)((uintptr_t)vbase_addr + base_offset), CRG_DET_ERR_MASK_REG,
			(CRG_CLK_MASK_OFFSET | CRG_RST_MASK_OFFSET), false);
		lpm_crg_set_reg((void __iomem *)((uintptr_t)vbase_addr + base_offset), CRG_DET_FUNC_MASK_REG,
			(CRG_CLK_MASK_OFFSET | CRG_RST_MASK_OFFSET), false);
		writel(0x1, (void __iomem *)((uintptr_t)vbase_addr + (base_offset + (uintptr_t)CRG_DET_INT_ENABLE_REG)));
	}
}

int32_t lpm_safety_irq_crg_handler(struct safety_fault_info *safety_fault,
	uint32_t *event_num, struct safety_event **event_list)
{
	int32_t ret;

	if (lpm_safety_check_irq_param(safety_fault, event_num, event_list) != 0) {
		return -1;
	}

	ret = lpm_safety_fill_event_data(safety_fault, event_num, event_list);
	if (ret != 0) {
		return ret;
	}

	lpm_safety_crg_priv_proc(safety_fault);

	ret = lpm_safety_report_fault(0, DMS_SEN_TYPE_RAS_SENSOR,
		LP_SAFETY_EVENT_MODULE_ERROR_CANNOT_FIXED, true);
	if (ret != 0) {
		lpm_log_err("lpm_safety_report_fault failed, ret=%d\n", ret);
		return ret;
	}
	lpm_log_err("lpm crg error occur, id:0x80E38001\n");
	return 0;
}

STATIC void lpm_fault_paddr_unmapping(struct module_info *module_info, uint32_t cnt)
{
	uint32_t i;

	for (i = 0; i < cnt; i++) {
		if (module_info[i].vaddr != NULL) {
			iounmap(module_info[i].vaddr);
			module_info[i].vaddr = NULL;
		}
	}
}

STATIC int32_t lpm_fault_paddr_mapping(struct module_info *module_info, uint32_t cnt)
{
	uint32_t i;

	for (i = 0; i < cnt; i++) {
		module_info[i].vaddr = (void __iomem *)ioremap(module_info[i].phy_addr, module_info[i].size);
		if (module_info[i].vaddr == NULL) {
			lpm_fault_paddr_unmapping(module_info, cnt);
			lpm_log_err("lpm ioremap failed, size=0x%x\n",
				module_info[i].size);
			return -1;
		}
	}

	return 0;
}

STATIC void lpm_fault_safety_irq_unmask(void)
{
	uint32_t i;

	for (i = 0; i < (uint32_t)ARRAY_SIZE(g_crg_moniter_cfg); i++) {
		lpm_fault_safety_irq_hw_mask_cfg(&g_crg_moniter_cfg[i], false);
	}
}

bool lpm_fault_safety_crg_sub_monitor_enable(uintptr_t base_addr, uint32_t offset)
{
	uint32_t i;
	uint64_t mask;
	struct lpm_pg_info *pg_info = NULL;

	for (i = 0; i < ARRAY_SIZE(g_crg_moniter_cfg); i++) {
		if ((g_crg_moniter_cfg[i].phy_base_addr != base_addr) ||
			(g_crg_moniter_cfg[i].monitor_ofst != offset)) {
			continue;
		}

		if (g_crg_moniter_cfg[i].type == LPM_PG_TYPE_OTHER) {
			return true;
		}

		pg_info = lpm_crg_monitor_assemble_pginfo();
		mask = lpm_get_pg_mask_by_type(g_crg_moniter_cfg[i].type, pg_info);
		if ((g_crg_moniter_cfg[i].mask & mask) != g_crg_moniter_cfg[i].mask) {
			return false;
		} else {
			return true;
		}
	}
	return false;
}

int32_t lpm_fault_safety_crg_init(void)
{
	int32_t ret;

	ret = lpm_fault_paddr_mapping(g_crg_module_info, (uint32_t)ARRAY_SIZE(g_crg_module_info));
	if (ret != 0) {
		lpm_log_err("lpm_fault_paddr_mapping failed\n");
		return -1;
	}

	lpm_fault_safety_irq_mask_errpin();

	ret = lpm_crg_moniter_config();
	if (ret != 0) {
		lpm_fault_paddr_unmapping(g_crg_module_info, (uint32_t)ARRAY_SIZE(g_crg_module_info));
		lpm_log_err("lpm_crg_moniter_config failed\n");
		return -1;
	}
	lpm_log_info("lpm fault safety crg init success\n");
	return 0;
}

void lpm_fault_safety_crg_uninit(void)
{
	lpm_fault_safety_irq_unmask();
	lpm_fault_paddr_unmapping(g_crg_module_info, (uint32_t)ARRAY_SIZE(g_crg_module_info));
	lpm_log_info("lpm fault safety crg exit success\n");
}
