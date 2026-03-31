/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2020-2022. All rights reserved.
 * Description: hdmi crg reg function defined.
 * Author: Hisilicon multimedia software group
 * Create: 2020/05/15
 */

#include <linux/io.h>
#include "hdmi_reg_crg.h"
#include "hdmi_product_define.h"
#include "hdmi_osal.h"
#include "drv_hdmi_compatibility.h"
#include "ot_osal.h"

volatile hdmi_reg_crg *g_crg_reg_en = NULL;
volatile hdmi_reg_crg *g_crg_reg_dis = NULL;
volatile hdmi_reg_crg *g_crg_reg_st = NULL;
volatile hdmi_reg_srst *g_srst_reg_req = NULL;
volatile hdmi_reg_srst *g_srst_reg_dreq = NULL;
volatile hdmi_reg_srst *g_srst_reg_st = NULL;

static u32 hdmi_crg_init_flag[(td_s32)HDMI_DEVICE_ID_BUTT] = {TD_FALSE};

static td_bool is_hdmi_reg_crg_init_done(void)
{
    if ((hdmi_crg_init_flag[HDMI_DEVICE_ID0] == TD_TRUE) ||
        (hdmi_crg_init_flag[HDMI_DEVICE_ID1] == TD_TRUE)) {
        return TD_TRUE;
    } else {
        return TD_FALSE;
    }
}

static void hdmi_reg_crg_deinit_inner(void)
{
    if (g_crg_reg_en != TD_NULL) {
        osal_iounmap((void *)g_crg_reg_en);
        g_crg_reg_en = TD_NULL;
    }
    if (g_crg_reg_dis != TD_NULL) {
        osal_iounmap((void *)g_crg_reg_dis);
        g_crg_reg_dis = TD_NULL;
    }
    if (g_crg_reg_st != TD_NULL) {
        osal_iounmap((void *)g_crg_reg_st);
        g_crg_reg_st = TD_NULL;
    }
    if (g_srst_reg_req != TD_NULL) {
        osal_iounmap((void *)g_srst_reg_req);
        g_srst_reg_req = TD_NULL;
    }
    if (g_srst_reg_dreq != TD_NULL) {
        osal_iounmap((void *)g_srst_reg_dreq);
        g_srst_reg_dreq = TD_NULL;
    }
    if (g_srst_reg_st != TD_NULL) {
        osal_iounmap((void *)g_srst_reg_st);
        g_srst_reg_st = TD_NULL;
    }
}

int hdmi_reg_crg_init(td_u32 id)
{
    if (is_hdmi_reg_crg_init_done() == TD_TRUE) {
        hdmi_crg_init_flag[id] = TD_TRUE;
        return TD_SUCCESS;
    }
    g_crg_reg_en = (volatile hdmi_reg_crg *)osal_ioremap_nocache(HDMI_CRG_EN_ADDR, sizeof(hdmi_reg_crg));
    g_crg_reg_dis = (volatile hdmi_reg_crg *)osal_ioremap_nocache(HDMI_CRG_DIS_ADDR, sizeof(hdmi_reg_crg));
    g_crg_reg_st = (volatile hdmi_reg_crg *)osal_ioremap_nocache(HDMI_CRG_ST_ADDR, sizeof(hdmi_reg_crg));
    g_srst_reg_req = (volatile hdmi_reg_srst *)osal_ioremap_nocache(HDMI_RESET_REQ_ADDR, sizeof(hdmi_reg_crg));
    g_srst_reg_dreq = (volatile hdmi_reg_srst *)osal_ioremap_nocache(HDMI_RESET_DREQ_ADDR, sizeof(hdmi_reg_crg));
    g_srst_reg_st = (volatile hdmi_reg_srst *)osal_ioremap_nocache(HDMI_RESET_ST_ADDR, sizeof(hdmi_reg_crg));
    if ((g_crg_reg_en == NULL) || (g_crg_reg_dis == NULL) || (g_crg_reg_st == NULL) ||
        (g_srst_reg_req == NULL) || (g_srst_reg_dreq == NULL) || (g_srst_reg_st == NULL)) {
        hdmi_reg_crg_deinit_inner();
        hdmi_err("hdmi %u crg addr ioremap failed!\n", id);
        return TD_FAILURE;
    }
    hdmi_crg_init_flag[id] = TD_TRUE;
    return TD_SUCCESS;
}

int hdmi_reg_crg_deinit(td_u32 id)
{
    hdmi_crg_init_flag[id] = TD_FALSE;
    if (is_hdmi_reg_crg_init_done() != TD_TRUE) {
        hdmi_reg_crg_deinit_inner();
    }
    return TD_SUCCESS;
}

static td_void hdmi_reg_ctrl_crg_config(td_bool state, td_u32 value)
{
    if (state == TD_TRUE) {
        hdmi_tx_reg_write((td_u32 *)g_crg_reg_en, value);
    } else {
        hdmi_tx_reg_write((td_u32 *)g_crg_reg_dis, value);
    }
}

static td_void hdmi_reg_ctrl_srst_config(td_bool state, td_u32 value)
{
    if (state == TD_TRUE) {
        hdmi_tx_reg_write((td_u32 *)g_srst_reg_req, value);
    } else {
        hdmi_tx_reg_write((td_u32 *)g_srst_reg_dreq, value);
    }
}

int hdmi_reg_pixel_cken_set(td_u32 id, td_bool enable)
{
    hdmi_reg_crg value;
    value.u32 = 0;
    value.bits.icg_en_hdmi_pixel = (1U << id);
    hdmi_reg_ctrl_crg_config(enable, value.u32);
    return TD_SUCCESS;
}

int hdmi_reg_ctrl_osc_24m_cken_set(td_u32 id, td_bool enable)
{
    hdmi_reg_crg value;
    value.u32 = 0;
    value.bits.icg_en_24m_core = (1U << id);
    value.bits.icg_en_hdmitx_ctrl_osc_24m = (1U << id);
    hdmi_reg_ctrl_crg_config(enable, value.u32);
    return TD_SUCCESS;
}

int hdmi_reg_ctrl_cec_cken_set(td_u32 id, td_bool enable)
{
    hdmi_reg_crg value;
    value.u32 = 0;
    value.bits.icg_en_hdmitx_ctrl_cec_com = (1U << id);
    value.bits.icg_en_hdmitx_ctrl_cec = (1U << id);
    hdmi_reg_ctrl_crg_config(enable, value.u32);
    return TD_SUCCESS;
}

int hdmi_reg_ctrl_os_cken_set(td_u32 id, td_bool enable)
{
    hdmi_reg_crg value;
    value.u32 = 0;
    value.bits.icg_en_hdmitx_ctrl_os = (1U << id);
    hdmi_reg_ctrl_crg_config(enable, value.u32);
    return TD_SUCCESS;
}

int hdmi_reg_ctrl_as_cken_set(td_u32 id, td_bool enable)
{
    hdmi_reg_crg value;
    value.u32 = 0;
    value.bits.icg_en_hdmitx_ctrl_as = (1U << id);
    hdmi_reg_ctrl_crg_config(enable, value.u32);
    return TD_SUCCESS;
}

int hdmi_reg_ctrl_bus_srst_req_set(td_u32 id, td_bool enable)
{
    hdmi_reg_srst value;
    value.u32 = 0;
    value.bits.srst_req_hdmitx_ctrl_bus = (1U << id);
    hdmi_reg_ctrl_srst_config(enable, value.u32);
    return TD_SUCCESS;
}

int hdmi_reg_ctrl_srst_req_set(td_u32 id, td_bool enable)
{
    hdmi_reg_srst value;
    value.u32 = 0;
    value.bits.srst_req_hdmitx_ctrl = (1U << id);
    hdmi_reg_ctrl_srst_config(enable, value.u32);
    return TD_SUCCESS;
}

int hdmi_reg_ctrl_cec_srst_req_set(td_u32 id, td_bool enable)
{
    hdmi_reg_srst value;
    value.u32 = 0;
    value.bits.srst_req_hdmitx_ctrl_cec = (1U << id);
    hdmi_reg_ctrl_srst_config(enable, value.u32);
    return TD_SUCCESS;
}

int hdmi_reg_hdmitx_phy_tmds_cken_set(td_u32 id, td_bool enable)
{
    hdmi_reg_crg value;
    value.u32 = 0;
    value.bits.icg_en_hdmitx_phy_tmds = (1U << id);
    hdmi_reg_ctrl_crg_config(enable, value.u32);
    return TD_SUCCESS;
}

int hdmi_reg_hdmitx_phy_modclk_cken_set(td_u32 id, td_bool enable)
{
    hdmi_reg_crg value;
    value.u32 = 0;
    value.bits.icg_en_hdmitx_phy_modclk = (1U << id);
    hdmi_reg_ctrl_crg_config(enable, value.u32);
    return TD_SUCCESS;
}

int hdmi_reg_ac_ctrl_modclk_cken_set(td_u32 id, td_bool enable)
{
    hdmi_reg_crg value;
    value.u32 = 0;
    value.bits.icg_en_ac_ctrl_modclk = (1U << id);
    hdmi_reg_ctrl_crg_config(enable, value.u32);
    return TD_SUCCESS;
}

int hdmi_reg_phy_srst_req_set(td_u32 id, td_bool enable)
{
    hdmi_reg_srst value;
    value.u32 = 0;
    value.bits.srst_req_hdmitx_phy = (1U << id);
    hdmi_reg_ctrl_srst_config(enable, value.u32);
    return TD_SUCCESS;
}

int hdmi_reg_phy_bus_srst_req_set(td_u32 id, td_bool enable)
{
    hdmi_reg_srst value;
    value.u32 = 0;
    value.bits.srst_req_hdmitx_phy_bus = (1U << id);
    hdmi_reg_ctrl_srst_config(enable, value.u32);
    return TD_SUCCESS;
}

int hdmi_reg_ac_ctrl_srst_req_set(td_u32 id, td_bool enable)
{
    hdmi_reg_srst value;
    value.u32 = 0;
    value.bits.srst_req_ac_ctrl = (1U << id);
    hdmi_reg_ctrl_srst_config(enable, value.u32);
    return TD_SUCCESS;
}

int hdmi_reg_ac_ctrl_bus_srst_req_set(td_u32 id, td_bool enable)
{
    hdmi_reg_srst value;
    value.u32 = 0;
    value.bits.srst_req_ac_ctrl_bus = (1U << id);
    hdmi_reg_ctrl_srst_config(enable, value.u32);
    return TD_SUCCESS;
}

int hdmi_reg_ctrl_reset_set(td_u32 id, td_bool enable)
{
    hdmi_reg_ctrl_bus_srst_req_set(id, enable);
    hdmi_reg_ctrl_srst_req_set(id, enable);
    hdmi_reg_ctrl_cec_srst_req_set(id, enable);
    return TD_SUCCESS;
}

int hdmi_reg_phy_reset_set(td_u32 id, td_bool enable)
{
    hdmi_reg_phy_srst_req_set(id, enable);
    hdmi_reg_phy_bus_srst_req_set(id, enable);
    hdmi_reg_ac_ctrl_srst_req_set(id, enable);
    hdmi_reg_ac_ctrl_bus_srst_req_set(id, enable);
    return TD_SUCCESS;
}

int hdmi_reg_top_reset_set(td_u32 id, td_bool enable)
{
    hdmi_reg_srst value;
    value.u32 = 0;
    value.bits.srst_req_hdmi_com = (1U << id);
    hdmi_reg_ctrl_srst_config(enable, value.u32);
    return TD_SUCCESS;
}

int hdmi_reg_subctrl_clk_set(td_u32 id)
{
    hdmi_reg_sc_icg_dis sc_icg_dis;
    sc_icg_dis.u32 = hdmi_tx_reg_read((td_u32 *)g_crg_reg_dis);
    sc_icg_dis.bits.icg_dis_hdmitx_ctrl_osc_24m = (1U << id);
    sc_icg_dis.bits.icg_dis_hdmitx_ctrl_cec = (1U << id);
    sc_icg_dis.bits.icg_dis_hdmitx_ctrl_as = (1U << id);
    hdmi_tx_reg_write((td_u32 *)g_crg_reg_dis, sc_icg_dis.u32);
    
    return TD_SUCCESS;
}