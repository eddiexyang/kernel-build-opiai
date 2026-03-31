/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2020-2022. All rights reserved.
 * Description: hdmi product define.
 * Author: Hisilicon multimedia software group
 * Create: 2020/05/15
 */

#include "hdmi_product_define.h"
#include "hdmi_reg_crg.h"
#include "mod_ext.h"
#include "proc_ext.h"
#include "dev_ext.h"
#include "ot_common.h"
#include "sys_ext.h"

#define CRG_RESET_DELAY 2
#define HDMI_IO_CFG_FUNCTION1_SEL 0x1431

td_void hdmi_tx_reg_write(td_u32 *reg_addr, td_u32 value)
{
    if (reg_addr == TD_NULL) {
        return;
    }
    *(volatile td_u32 *)reg_addr = value;
    return;
}

td_u32 hdmi_tx_reg_read(const td_u32 *reg_addr)
{
    if (reg_addr == TD_NULL) {
        return 0U;
    }
    return *(volatile td_u32 *)(reg_addr);
}

td_void hdmi_reg_write_u32(td_u32 reg_addr, td_u32 value)
{
    volatile td_u32 *addr = TD_NULL;

    addr = (volatile td_u32 *)osal_ioremap_nocache((td_u64)reg_addr, HDMI_REGISTER_SIZE);
    if (addr != TD_NULL) {
        *addr = value;
        osal_iounmap((void *)addr);
    } else {
        hdmi_err("osal_ioremap_nocache addr=0x%x err!\n", reg_addr);
    }

    return;
}

td_void drv_hdmi_prod_io_cfg_set(td_u32 id)
{
    hdmi_if_fpga_return_void();

    if (id == 0) {
        hdmi_reg_write_u32(HDMI_ADDR_IO_CFG_HOTPLUG, HDMI_IO_CFG_FUNCTION1_SEL);
        hdmi_reg_write_u32(HDMI_ADDR_IO_CFG_SDA, HDMI_IO_CFG_FUNCTION1_SEL);
        hdmi_reg_write_u32(HDMI_ADDR_IO_CFG_SCL, HDMI_IO_CFG_FUNCTION1_SEL);
    } else {
        hdmi_reg_write_u32(HDMI1_ADDR_IO_CFG_HOTPLUG, HDMI_IO_CFG_FUNCTION1_SEL);
        hdmi_reg_write_u32(HDMI1_ADDR_IO_CFG_SDA, HDMI_IO_CFG_FUNCTION1_SEL);
        hdmi_reg_write_u32(HDMI1_ADDR_IO_CFG_SCL, HDMI_IO_CFG_FUNCTION1_SEL);
    }

    return;
}

td_void drv_hdmi_prod_crg_gate_set(td_u32 id, td_bool enable)
{
    hdmi_if_fpga_return_void();

    hdmi_reg_ctrl_osc_24m_cken_set(id, enable);
    hdmi_reg_ctrl_cec_cken_set(id, enable);
    hdmi_reg_ctrl_os_cken_set(id, enable);
    hdmi_reg_ctrl_as_cken_set(id, enable);
    hdmi_reg_hdmitx_phy_tmds_cken_set(id, enable);
    hdmi_reg_hdmitx_phy_modclk_cken_set(id, enable);
    hdmi_reg_ac_ctrl_modclk_cken_set(id, enable);
    hdmi_reg_pixel_cken_set(id, enable);

    return;
}

td_void drv_hdmi_prod_crg_all_reset_set(td_u32 id, td_bool enable)
{
    td_bool reverse = (td_bool)(!enable);
    hdmi_reg_top_reset_set(id, enable);
    hdmi_reg_ctrl_reset_set(id, enable);
    hdmi_reg_phy_reset_set(id, enable);
    /*
     * 2us. to ensure ctrl reset success.
     * because internal clock of HDMI is smaller than APB clock.
     */
    osal_udelay(CRG_RESET_DELAY);
    hdmi_reg_top_reset_set(id, reverse);
    hdmi_reg_ctrl_reset_set(id, reverse);
    hdmi_reg_phy_reset_set(id, reverse);
    return;
}

td_void drv_hdmi_low_power_set(td_u32 id, td_bool enable)
{
    hdmi_if_fpga_return_void();

    enable = (td_bool)(!enable);
    hdmi_reg_ctrl_os_cken_set(id, enable);
    hdmi_reg_ctrl_as_cken_set(id, enable);

    return;
}

td_void drv_hdmi_prod_crg_init(td_u32 id)
{
    drv_hdmi_prod_crg_gate_set(id, TD_TRUE);
    drv_hdmi_prod_crg_all_reset_set(id, TD_TRUE);
    drv_hdmi_low_power_set(id, TD_TRUE);

    return;
}

td_void drv_hdmi_hardware_reset(td_u32 id)
{
    hdmi_if_fpga_return_void();

    if (hdmi_reg_crg_init(id) != TD_SUCCESS) {
        hdmi_err("CRG reg io map error!\n");
        return;
    }
    /* reset all module */
    hdmi_reg_ctrl_bus_srst_req_set(id, TD_TRUE);
    hdmi_reg_ctrl_srst_req_set(id, TD_TRUE);
    hdmi_reg_ctrl_cec_srst_req_set(id, TD_TRUE);
    hdmi_reg_phy_srst_req_set(id, TD_TRUE);
    hdmi_reg_phy_bus_srst_req_set(id, TD_TRUE);
    hdmi_reg_ac_ctrl_srst_req_set(id, TD_TRUE);
    hdmi_reg_ac_ctrl_bus_srst_req_set(id, TD_TRUE);
    /* close all clk */
    drv_hdmi_prod_crg_gate_set(id, TD_FALSE);
    hdmi_reg_crg_deinit(id);

    return;
}

