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


#include "devdrv_common.h"

#define DHA_BASE_TO_MPAM_BASE 0x002E0000
#define CHIP_OFFSET 0x200000000000

#define HBMC_DHA0_MPAM_BASE_ADDR 0x892F0000
#define HBMC_DHA2_MPAM_BASE_ADDR 0x89300000
#define HBMC_DHA4_MPAM_BASE_ADDR 0x89310000
#define HBMC_DHA6_MPAM_BASE_ADDR 0x89320000
#define HBMC_DHA1_MPAM_BASE_ADDR 0x8B2F0000
#define HBMC_DHA3_MPAM_BASE_ADDR 0x8B300000
#define HBMC_DHA5_MPAM_BASE_ADDR 0x8B310000
#define HBMC_DHA7_MPAM_BASE_ADDR 0x8B320000
#define SOC_DHA_MPAMCFG_PART_SEL_ADDR(base) ((base) + (0x0100))
#define SOC_DHA_MPAMCFG_MBW_MIN_ADDR(base) ((base) + (0x0200))
#define SOC_DHA_MPAMCFG_MBW_MAX_ADDR(base) ((base) + (0x0208))
#define SOC_DHA_CTRL_ADDR(base) ((base) + (0x0000))
#define SOC_DHA_FLOW_MODE_ADDR(base) ((base) + (0x03D8))

typedef union {
    u32 value;
    struct {
        u32 partid_sel : 16;
        u32 internal : 1;
        u32 reserved : 15;
    } reg;
} SOC_DHA_MPAMCFG_PART_SEL_UNION;

typedef union {
    u32 value;
    struct {
        u32 unimpl : 10;
        u32 min : 6;
        u32 reserved : 16;
    } reg;
} SOC_DHA_MPAMCFG_MBW_MIN_UNION;

typedef union {
    u32 value;
    struct {
        u32 unimpl : 10;
        u32 max : 6;
        u32 reserved : 15;
        u32 hardlim : 1;
    } reg;
} SOC_DHA_MPAMCFG_MBW_MAX_UNION;

typedef union {
    u32 value;
    struct {
        u32 reg_ctrl_linkdown : 1;
        u32 reg_ctrl_eccen : 1;
        u32 reg_ctrl_excl_eventen : 1;
        u32 reg_ctrl_excl_clear_dis : 1;
        u32 reg_ctrl_rdatabyp : 1;
        u32 reg_ctrl_dmcassign : 1;
        u32 reg_ctrl_prefetch_drop : 1;
        u32 reg_ctrl_writeevict_drop : 1;
        u32 reg_ctrl_data_reside : 1;
        u32 reg_ctrl_compress : 1;
        u32 reg_ctrl_compress_spec : 1;
        u32 reg_ctrl_prefetchtgt_full : 1;
        u32 reg_ctrl_defrpt : 1;
        u32 reg_ctrl_defpoison : 1;
        u32 reg_ctrl_defvalue : 1;
        u32 reg_ctrl_errrpt : 1;
        u32 reg_ctrl_errpoison : 1;
        u32 reg_ctrl_errvalue : 1;
        u32 reg_ctrl_defokrpt : 1;
        u32 reg_ctrl_poison : 1;
        u32 reg_ctrl_mpamqos : 1;
        u32 reg_ctrl_mpamen : 1;
        u32 reg_ctrl_spillprefetch : 1;
        u32 reserved_0 : 1;
        u32 reg_ctrl_datpush : 1;
        u32 reg_ctrl_atomicrpt : 1;
        u32 reg_ctrl_transerr : 1;
        u32 reg_ctrl_poisonerr : 1;
        u32 reg_ctrl_rdreceiptcomb : 1;
        u32 reg_ctrl_l2allocopt : 2;
        u32 reserved_1 : 1;
    } reg;
} SOC_DHA_CTRL_UNION;

typedef union {
    u32 value;
    struct {
        u32 reg_flow_pq : 1;
        u32 reg_flow_cmd : 1;
        u32 reg_flow_iq : 1;
        u32 reserved_0 : 1;
        u32 reg_txdat_detect_en : 1;
        u32 reg_txdat_push_en : 1;
        u32 reserved_1 : 2;
        u32 reg_detect_sub_th : 3;
        u32 reserved_2 : 1;
        u32 reg_push_sub_th : 3;
        u32 reserved_3 : 17;
    } reg;
} SOC_DHA_FLOW_MODE_UNION;

#define DDR_HHA_MPAM_BASE_ADDR 0x892E0000
#define HHA_BASE_TO_MPAM_BASE 0x002E0000
#define DDR_HHA_BASE_ADDR 0x89000000

#define SOC_HHA_MPAMCFG_PART_SEL_ADDR(base) ((base) + (0x0100))
#define SOC_HHA_MPAMCFG_MBW_MIN_ADDR(base) ((base) + (0x0200))
#define SOC_HHA_MPAMCFG_MBW_MAX_ADDR(base) ((base) + (0x0208))
#define SOC_HHA_CTRL_ADDR(base) ((base) + (0x0000))
#define SOC_HHA_FLOW_MODE_ADDR(base) ((base) + (0x03D8))

typedef union {
    u32 value;
    struct {
        u32 partid_sel : 16;
        u32 internal : 1;
        u32 reserved : 15;
    } reg;
} SOC_HHA_MPAMCFG_PART_SEL_UNION;

typedef union {
    u32 value;
    struct {
        u32 unimpl : 10;
        u32 min : 6;
        u32 reserved : 16;
    } reg;
} SOC_HHA_MPAMCFG_MBW_MIN_UNION;

typedef union {
    u32 value;
    struct {
        u32 unimpl : 10;
        u32 max : 6;
        u32 reserved : 15;
        u32 hardlim : 1;
    } reg;
} SOC_HHA_MPAMCFG_MBW_MAX_UNION;

typedef union {
    u32 value;
    struct {
        u32 reg_ctrl_linkdown : 1;
        u32 reg_ctrl_eccen : 1;
        u32 reg_ctrl_excl_eventen : 1;
        u32 reg_ctrl_excl_clear_dis : 1;
        u32 reg_ctrl_rdatabyp : 1;
        u32 reg_ctrl_dmcassign : 1;
        u32 reg_ctrl_prefetch_drop : 1;
        u32 reg_ctrl_writeevict_drop : 1;

        u32 reg_ctrl_data_reside : 1;
        u32 reg_ctrl_compress : 1;
        u32 reg_ctrl_compress_spec : 1;
        u32 reg_ctrl_prefetchtgt_full : 1;
        u32 reg_ctrl_defrpt : 1;
        u32 reg_ctrl_defpoison : 1;
        u32 reg_ctrl_defvalue : 1;
        u32 reg_ctrl_errrpt : 1;

        u32 reg_ctrl_errpoison : 1;
        u32 reg_ctrl_errvalue : 1;
        u32 reg_ctrl_defokrpt : 1;
        u32 reg_ctrl_poison : 1;
        u32 reg_ctrl_mpamqos : 1;
        u32 reg_ctrl_mpamen : 1;
        u32 reg_ctrl_spillprefetch : 1;
        u32 reg_data_merge_byp : 1;

        u32 reg_ctrl_datpush : 1;
        u32 reserved_0 : 1;
        u32 reg_ctrl_transerr : 1;
        u32 reg_ctrl_poisonerr : 1;
        u32 reserved_1 : 4;
    } reg;
} SOC_HHA_CTRL_UNION;

typedef union {
    u32 value;
    struct {
        u32 reg_push_sub_th : 3;
        u32 reserved_0 : 2;
        u32 reg_txdat_push_en : 1;
        u32 reserved_1 : 2;
        u32 reg_txdbid_bandwidth : 5;
        u32 reserved_2 : 19;
    } reg;
} SOC_HHA_FLOW_MODE_UNION;

void devdrv_ddr_mpam_and_push_config(u32 dev_id);
void devdrv_hbm_mpam_and_push_config(u32 dev_id);
