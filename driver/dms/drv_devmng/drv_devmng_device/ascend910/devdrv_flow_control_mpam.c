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


#include "devdrv_flow_control_mpam.h"

STATIC int read_reg32(u64 addr, u32 *data)
{
    void __iomem *vir_addr = NULL;

    vir_addr = ioremap(addr, sizeof(u32));
    if (vir_addr == NULL) {
        devdrv_drv_err("ioremap failed. \n");
        return -EINVAL;
    }
    *data = readl(vir_addr);
    iounmap(vir_addr);
    return 0;
}

STATIC int write_reg32(u64 addr, u32 data)
{
    void __iomem *vir_addr = NULL;

    vir_addr = ioremap(addr, sizeof(u32));
    if (vir_addr == NULL) {
        devdrv_drv_err("ioremap failed. \n");
        return -EINVAL;
    }
    writel(data, vir_addr);
    iounmap(vir_addr);
    return 0;
}

STATIC int hbm_mpam_schedul_per_stack(u16 mpam_id, u32 mbw_min, u32 mbw_max, u64 skt_offset, u64 base_addr)
{
    SOC_DHA_MPAMCFG_PART_SEL_UNION mpam_cfg_part_sel;
    SOC_DHA_MPAMCFG_MBW_MIN_UNION mpam_mbw_min;
    SOC_DHA_MPAMCFG_MBW_MAX_UNION mpam_mbw_max;
    int ret;

    ret = read_reg32(skt_offset + SOC_DHA_MPAMCFG_PART_SEL_ADDR(base_addr), &mpam_cfg_part_sel.value);
    if (ret) {
        devdrv_drv_err("read_reg32 failed.ret(%d)\n", ret);
        return ret;
    }
    mpam_cfg_part_sel.reg.partid_sel = mpam_id;
    ret = write_reg32(skt_offset + SOC_DHA_MPAMCFG_PART_SEL_ADDR(base_addr), mpam_cfg_part_sel.value);
    if (ret) {
        devdrv_drv_err("write_reg32 failed.ret(%d)\n", ret);
        return ret;
    }
    ret = read_reg32(skt_offset + SOC_DHA_MPAMCFG_MBW_MIN_ADDR(base_addr), &mpam_mbw_min.value);
    if (ret) {
        devdrv_drv_err("read_reg32 failed.ret(%d)\n", ret);
        return ret;
    }
    mpam_mbw_min.reg.min = mbw_min;
    ret = write_reg32(skt_offset + SOC_DHA_MPAMCFG_MBW_MIN_ADDR(base_addr), mpam_mbw_min.value);
    if (ret) {
        devdrv_drv_err("write_reg32 failed.ret(%d)\n", ret);
        return ret;
    }
    ret = read_reg32(skt_offset + SOC_DHA_MPAMCFG_MBW_MAX_ADDR(base_addr), &mpam_mbw_max.value);
    if (ret) {
        devdrv_drv_err("read_reg32 failed.ret(%d)\n", ret);
        return ret;
    }
    mpam_mbw_max.reg.max = mbw_max;
    ret = write_reg32(skt_offset + SOC_DHA_MPAMCFG_MBW_MAX_ADDR(base_addr), mpam_mbw_max.value);
    if (ret) {
        devdrv_drv_err("write_reg32 failed.ret(%d)\n", ret);
        return ret;
    }
    return 0;
}

STATIC int hbm_mpam_config_per_stack(u64 skt_offset, u64 base_addr)
{
    const u16 dvpp_mpam_id = 0x0;
    const u32 dvpp_mbw_min = 0x0; /* 0GB */
    const u32 dvpp_mbw_max = 0x3; /* 6GB */

    const u16 tsid = 0x1;
    const u32 ts_mbw_min = 0x1; /* 2GB */
    const u32 ts_mbw_max = 0x1; /* 2GB */

    const u16 sdma_id = 0x2;
    const u32 sdma_mbw_min = 0x8; /* 16GB */
    const u32 sdma_mbw_max = 0xF; /* 30GB */

    const u16 aicore_ins_id = 0x3;
    const u16 aicore_data_id = 0x4;
    const u32 aicore_inst_mbw_min = 0xF;  /* 30GB */
    const u32 aicore_inst_mbw_max = 0x1E; /* 60GB */
    const u32 aicore_data_mbw_min = 0xF;  /* 30GB */
    const u32 aicore_data_mbw_max = 0x1E; /* 60GB */

    const u16 taishan_inst_id = 0x5;
    const u16 taishan_data_id = 0x6;
    const u32 taishan_inst_mbw_min = 0x1; /* 2GB */
    const u32 taishan_inst_mbw_max = 0x1; /* 2GB */
    const u32 taishan_data_mbw_min = 0x1; /* 2GB */
    const u32 taishan_data_mbw_max = 0x1; /* 2GB */

    const u16 sllc_id = 0x7;
    const u32 sllc_mbw_min = 0x0; /* 0GB */
    const u32 sllc_mbw_max = 0xF; /* 30GB */

    int ret;
    SOC_DHA_CTRL_UNION dha_ctrl;

    ret = hbm_mpam_schedul_per_stack(dvpp_mpam_id, dvpp_mbw_min, dvpp_mbw_max, skt_offset, base_addr);
    if (ret) {
        devdrv_drv_err("hbm mpam schedul failed.ret(%d)\n", ret);
        return ret;
    }
    ret = hbm_mpam_schedul_per_stack(tsid, ts_mbw_min, ts_mbw_max, skt_offset, base_addr);
    if (ret) {
        devdrv_drv_err("hbm mpam schedul failed.ret(%d)\n", ret);
        return ret;
    }
    ret = hbm_mpam_schedul_per_stack(sdma_id, sdma_mbw_min, sdma_mbw_max, skt_offset, base_addr);
    if (ret) {
        devdrv_drv_err("hbm mpam schedul failed.ret(%d)\n", ret);
        return ret;
    }
    ret = hbm_mpam_schedul_per_stack(aicore_ins_id, aicore_inst_mbw_min, aicore_inst_mbw_max, skt_offset, base_addr);
    if (ret) {
        devdrv_drv_err("hbm mpam schedul failed.ret(%d)\n", ret);
        return ret;
    }
    ret = hbm_mpam_schedul_per_stack(aicore_data_id, aicore_data_mbw_min, aicore_data_mbw_max, skt_offset, base_addr);
    if (ret) {
        devdrv_drv_err("hbm mpam schedul failed.ret(%d)\n", ret);
        return ret;
    }
    ret = hbm_mpam_schedul_per_stack(taishan_inst_id, taishan_inst_mbw_min, taishan_inst_mbw_max, skt_offset,
                                     base_addr);
    if (ret) {
        devdrv_drv_err("hbm mpam schedul failed.ret(%d)\n", ret);
        return ret;
    }
    ret = hbm_mpam_schedul_per_stack(taishan_data_id, taishan_data_mbw_min, taishan_data_mbw_max, skt_offset,
                                     base_addr);
    if (ret) {
        devdrv_drv_err("hbm mpam schedul failed.ret(%d)\n", ret);
        return ret;
    }
    ret = hbm_mpam_schedul_per_stack(sllc_id, sllc_mbw_min, sllc_mbw_max, skt_offset, base_addr);
    if (ret) {
        devdrv_drv_err("hbm mpam schedul failed.ret(%d)\n", ret);
        return ret;
    }

    ret = read_reg32(skt_offset + SOC_DHA_CTRL_ADDR(base_addr - DHA_BASE_TO_MPAM_BASE), &dha_ctrl.value);
    if (ret) {
        devdrv_drv_err("read_reg32 failed.ret(%d)\n", ret);
        return ret;
    }
    dha_ctrl.reg.reg_ctrl_mpamen = 0x1;
    ret = write_reg32(skt_offset + SOC_DHA_CTRL_ADDR(base_addr - DHA_BASE_TO_MPAM_BASE), dha_ctrl.value);
    if (ret) {
        devdrv_drv_err("write_reg32 failed.ret(%d)\n", ret);
        return ret;
    }
    return 0;
}

STATIC int hbm_dha_push_per_stack(u64 skt_offset, u64 base_addr)
{
    SOC_DHA_FLOW_MODE_UNION dha_flow_mode;
    int ret;

    ret = read_reg32(skt_offset + SOC_DHA_FLOW_MODE_ADDR(base_addr - DHA_BASE_TO_MPAM_BASE), &dha_flow_mode.value);
    if (ret) {
        devdrv_drv_err("read_reg32 failed.ret(%d)\n", ret);
        return ret;
    }
    dha_flow_mode.reg.reg_txdat_push_en = 1;
    ret = write_reg32(skt_offset + SOC_DHA_FLOW_MODE_ADDR(base_addr - DHA_BASE_TO_MPAM_BASE), dha_flow_mode.value);
    if (ret) {
        devdrv_drv_err("write_reg32 failed.ret(%d)\n", ret);
        return ret;
    }
    return 0;
}
STATIC int devdrv_hbm_mpam_config(u64 skt_offset)
{
    int ret;

    ret = hbm_mpam_config_per_stack(skt_offset, HBMC_DHA0_MPAM_BASE_ADDR);
    if (ret) {
        devdrv_drv_err("hbm mpam config failed.ret(%d)\n", ret);
        return ret;
    }
    ret = hbm_mpam_config_per_stack(skt_offset, HBMC_DHA4_MPAM_BASE_ADDR);
    if (ret) {
        devdrv_drv_err("hbm mpam config failed.ret(%d)\n", ret);
        return ret;
    }

    ret = hbm_mpam_config_per_stack(skt_offset, HBMC_DHA1_MPAM_BASE_ADDR);
    if (ret) {
        devdrv_drv_err("hbm mpam config failed.ret(%d)\n", ret);
        return ret;
    }
    ret = hbm_mpam_config_per_stack(skt_offset, HBMC_DHA5_MPAM_BASE_ADDR);
    if (ret) {
        devdrv_drv_err("hbm mpam config failed.ret(%d)\n", ret);
        return ret;
    }

    ret = hbm_mpam_config_per_stack(skt_offset, HBMC_DHA2_MPAM_BASE_ADDR);
    if (ret) {
        devdrv_drv_err("hbm mpam config failed.ret(%d)\n", ret);
        return ret;
    }
    ret = hbm_mpam_config_per_stack(skt_offset, HBMC_DHA6_MPAM_BASE_ADDR);
    if (ret) {
        devdrv_drv_err("hbm mpam config failed.ret(%d)\n", ret);
        return ret;
    }

    ret = hbm_mpam_config_per_stack(skt_offset, HBMC_DHA3_MPAM_BASE_ADDR);
    if (ret) {
        devdrv_drv_err("hbm mpam config failed.ret(%d)\n", ret);
        return ret;
    }
    ret = hbm_mpam_config_per_stack(skt_offset, HBMC_DHA7_MPAM_BASE_ADDR);
    if (ret) {
        devdrv_drv_err("hbm mpam config failed.ret(%d)\n", ret);
        return ret;
    }
    return 0;
}
STATIC int devdrv_hbm_dha_push(u64 skt_offset)
{
    int ret;

    ret = hbm_dha_push_per_stack(skt_offset, HBMC_DHA0_MPAM_BASE_ADDR);
    if (ret) {
        devdrv_drv_err("hbm_dha_push_per_stack failed.ret(%d)\n", ret);
        return ret;
    }
    ret = hbm_dha_push_per_stack(skt_offset, HBMC_DHA4_MPAM_BASE_ADDR);
    if (ret) {
        devdrv_drv_err("hbm dha push failed.ret(%d)\n", ret);
        return ret;
    }

    ret = hbm_dha_push_per_stack(skt_offset, HBMC_DHA1_MPAM_BASE_ADDR);
    if (ret) {
        devdrv_drv_err("hbm dha push failed.ret(%d)\n", ret);
        return ret;
    }
    ret = hbm_dha_push_per_stack(skt_offset, HBMC_DHA5_MPAM_BASE_ADDR);
    if (ret) {
        devdrv_drv_err("hbm dha push failed.ret(%d)\n", ret);
        return ret;
    }

    ret = hbm_dha_push_per_stack(skt_offset, HBMC_DHA2_MPAM_BASE_ADDR);
    if (ret) {
        devdrv_drv_err("hbm dha push failed.ret(%d)\n", ret);
        return ret;
    }
    ret = hbm_dha_push_per_stack(skt_offset, HBMC_DHA6_MPAM_BASE_ADDR);
    if (ret) {
        devdrv_drv_err("hbm dha push failed.ret(%d)\n", ret);
        return ret;
    }

    ret = hbm_dha_push_per_stack(skt_offset, HBMC_DHA3_MPAM_BASE_ADDR);
    if (ret) {
        devdrv_drv_err("hbm dha push failed.ret(%d)\n", ret);
        return ret;
    }
    ret = hbm_dha_push_per_stack(skt_offset, HBMC_DHA7_MPAM_BASE_ADDR);
    if (ret) {
        devdrv_drv_err("hbm dha push failed.ret(%d)\n", ret);
        return ret;
    }
    return 0;
}
void devdrv_hbm_mpam_and_push_config(u32 dev_id)
{
    u64 skt_offset;
    int ret;
    skt_offset = ((u64)dev_id) * CHIP_OFFSET;
    ret = devdrv_hbm_mpam_config(skt_offset);
    if (ret) {
        devdrv_drv_err("hbm mpam config failed.ret(%d)\n", ret);
        return;
    }
    ret = devdrv_hbm_dha_push(skt_offset);
    if (ret) {
        devdrv_drv_err("hbm dha push failed.ret(%d)\n", ret);
        return;
    }
}

STATIC int ddr_mpam_schedule_per_skt(u8 mpam_id, u32 mbw_min, u32 mbw_max, u64 skt_offset, u64 base_addr)
{
    SOC_HHA_MPAMCFG_PART_SEL_UNION mpam_cfg_part_sel;
    SOC_HHA_MPAMCFG_MBW_MIN_UNION mpam_mbw_min;
    SOC_HHA_MPAMCFG_MBW_MAX_UNION mpam_mbw_max;
    int ret;

    ret = read_reg32(skt_offset + SOC_HHA_MPAMCFG_PART_SEL_ADDR(base_addr), &mpam_cfg_part_sel.value);
    if (ret) {
        devdrv_drv_err("read_reg32 failed.ret(%d)\n", ret);
        return ret;
    }
    mpam_cfg_part_sel.reg.partid_sel = mpam_id;

    ret = write_reg32(skt_offset + SOC_HHA_MPAMCFG_PART_SEL_ADDR(base_addr), mpam_cfg_part_sel.value);
    if (ret) {
        devdrv_drv_err("write_reg32 failed.ret(%d)\n", ret);
        return ret;
    }

    ret = read_reg32(skt_offset + SOC_HHA_MPAMCFG_MBW_MIN_ADDR(base_addr), &mpam_mbw_min.value);
    if (ret) {
        devdrv_drv_err("read_reg32 failed.ret(%d)\n", ret);
        return ret;
    }
    mpam_mbw_min.reg.min = mbw_min;
    ret = write_reg32(skt_offset + SOC_HHA_MPAMCFG_MBW_MIN_ADDR(base_addr), mpam_mbw_min.value);
    if (ret) {
        devdrv_drv_err("write_reg32 failed.ret(%d)\n", ret);
        return ret;
    }

    ret = read_reg32(skt_offset + SOC_HHA_MPAMCFG_MBW_MAX_ADDR(base_addr), &mpam_mbw_max.value);
    if (ret) {
        devdrv_drv_err("read_reg32 failed.ret(%d)\n", ret);
        return ret;
    }
    mpam_mbw_max.reg.max = mbw_max;
    ret = write_reg32(skt_offset + SOC_HHA_MPAMCFG_MBW_MAX_ADDR(base_addr), mpam_mbw_max.value);
    if (ret) {
        devdrv_drv_err("write_reg32 failed.ret(%d)\n", ret);
        return ret;
    }
    return 0;
}

STATIC int ddr_mpam_config_per_skt(u64 skt_offset, u64 base_addr)
{
    const u8 dvpp_mpam_id = 0x0;
    const u32 dvpp_mbw_min = 0x10; /* 10GB */
    const u32 dvpp_mbw_max = 0x1F; /* 19GB */

    const u8 tsid = 0x1;
    const u32 ts_mbw_min = 0x1; /* 2GB */
    const u32 ts_mbw_max = 0x1; /* 2GB */

    const u8 sdma_id = 0x2;
    const u32 sdma_mbw_min = 0x0;  /* 0GB */
    const u32 sdma_mbw_max = 0x10; /* 10GB */

    const u8 aicore_ins_id = 0x3;
    const u8 aicore_data_id = 0x4;
    const u32 aicore_inst_mbw_min = 0x0; /* 0GB */
    const u32 aicore_inst_mbw_max = 0x0; /* 0GB */
    const u32 aicore_data_mbw_min = 0x0; /* 0GB */
    const u32 aicore_data_mbw_max = 0x0; /* 0GB */

    const u8 taishan_inst_id = 0x5;
    const u8 taishan_data_id = 0x6;
    const u32 taishan_inst_mbw_min = 0x5; /* 3GB */
    const u32 taishan_inst_mbw_max = 0x5; /* 3GB */
    const u32 taishan_data_mbw_min = 0x5; /* 3GB */
    const u32 taishan_data_mbw_max = 0x5; /* 3GB */

    const u8 sllc_id = 0x7;
    const u32 sllc_mbw_min = 0xD;  /* 8GB */
    const u32 sllc_mbw_max = 0x1A; /* 16GB */

    int ret;
    SOC_HHA_CTRL_UNION hha_ctrl;

    ret = ddr_mpam_schedule_per_skt(dvpp_mpam_id, dvpp_mbw_min, dvpp_mbw_max, skt_offset, base_addr);
    if (ret) {
        devdrv_drv_err("ddr mpam schedule failed.ret(%d)\n", ret);
        return ret;
    }
    ret = ddr_mpam_schedule_per_skt(tsid, ts_mbw_min, ts_mbw_max, skt_offset, base_addr);
    if (ret) {
        devdrv_drv_err("ddr mpam schedule failed.ret(%d)\n", ret);
        return ret;
    }
    ret = ddr_mpam_schedule_per_skt(sdma_id, sdma_mbw_min, sdma_mbw_max, skt_offset, base_addr);
    if (ret) {
        devdrv_drv_err("ddr mpam schedule failed.ret(%d)\n", ret);
        return ret;
    }
    ret = ddr_mpam_schedule_per_skt(aicore_ins_id, aicore_inst_mbw_min, aicore_inst_mbw_max, skt_offset, base_addr);
    if (ret) {
        devdrv_drv_err("ddr mpam schedule failed.ret(%d)\n", ret);
        return ret;
    }
    ret = ddr_mpam_schedule_per_skt(aicore_data_id, aicore_data_mbw_min, aicore_data_mbw_max, skt_offset, base_addr);
    if (ret) {
        devdrv_drv_err("ddr mpam schedule failed.ret(%d)\n", ret);
        return ret;
    }
    ret = ddr_mpam_schedule_per_skt(taishan_inst_id, taishan_inst_mbw_min, taishan_inst_mbw_max, skt_offset, base_addr);
    if (ret) {
        devdrv_drv_err("ddr mpam schedule failed.ret(%d)\n", ret);
        return ret;
    }
    ret = ddr_mpam_schedule_per_skt(taishan_data_id, taishan_data_mbw_min, taishan_data_mbw_max, skt_offset, base_addr);
    if (ret) {
        devdrv_drv_err("ddr mpam schedule failed.ret(%d)\n", ret);
        return ret;
    }
    ret = ddr_mpam_schedule_per_skt(sllc_id, sllc_mbw_min, sllc_mbw_max, skt_offset, base_addr);
    if (ret) {
        devdrv_drv_err("ddr mpam schedule failed.ret(%d)\n", ret);
        return ret;
    }
    ret = read_reg32(skt_offset + SOC_HHA_CTRL_ADDR(base_addr - HHA_BASE_TO_MPAM_BASE), &hha_ctrl.value);
    if (ret) {
        devdrv_drv_err("read_reg32 failed.ret(%d)\n", ret);
        return ret;
    }
    hha_ctrl.reg.reg_ctrl_mpamen = 0x1;
    ret = write_reg32(skt_offset + SOC_HHA_CTRL_ADDR(base_addr - HHA_BASE_TO_MPAM_BASE), hha_ctrl.value);
    if (ret) {
        devdrv_drv_err("write_reg32 failed.ret(%d)\n", ret);
        return ret;
    }
    return 0;
}

void devdrv_ddr_mpam_and_push_config(u32 dev_id)
{
    SOC_HHA_FLOW_MODE_UNION hha_flow_mode;
    u64 skt_offset;
    int ret;

    skt_offset = dev_id * CHIP_OFFSET;
    ret = ddr_mpam_config_per_skt(skt_offset, DDR_HHA_MPAM_BASE_ADDR);
    if (ret) {
        devdrv_drv_err("ddr mpam config failed.ret(%d)\n", ret);
        return;
    }

    ret = read_reg32(skt_offset + SOC_HHA_FLOW_MODE_ADDR(DDR_HHA_BASE_ADDR), &hha_flow_mode.value);
    if (ret) {
        devdrv_drv_err("read_reg32 failed.ret(%d)\n", ret);
        return;
    }
    hha_flow_mode.reg.reg_txdat_push_en = 1;
    ret = write_reg32(skt_offset + SOC_HHA_FLOW_MODE_ADDR(DDR_HHA_BASE_ADDR), hha_flow_mode.value);
    if (ret) {
        devdrv_drv_err("write_reg32 failed.ret(%d)\n", ret);
        return;
    }
}
