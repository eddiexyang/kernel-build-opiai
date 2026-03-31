/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2020-2022. All rights reserved.
 * Description: hdmi crg reg function defined.
 * Author: Hisilicon multimedia software group
 * Create: 2020/05/15
 */

#ifndef HDMI_REG_CRG_H
#define HDMI_REG_CRG_H

#include "ot_type.h"

typedef struct {
    struct {
        unsigned int icg_en_ac_ctrl_modclk      : 2;  /* [1..0] */
        unsigned int icg_en_hdmitx_phy_modclk   : 2;  /* [3..2] */
        unsigned int icg_en_hdmitx_phy_tmds     : 2;  /* [5..4] */
        unsigned int icg_en_hdmitx_ctrl_osc_24m : 2;  /* [7..6] */
        unsigned int icg_en_hdmitx_ctrl_cec     : 2;  /* [9..8] */
        unsigned int icg_en_hdmitx_ctrl_as      : 2;  /* [11..10] */
        unsigned int icg_en_hdmitx_ctrl_os      : 2;  /* [13..12] */
        unsigned int icg_en_24m_core            : 2;  /* [15..14] */
        unsigned int icg_en_hdmitx_ctrl_cec_com : 2;  /* [17..16] */
        unsigned int icg_en_hdmi_pixel          : 2;  /* [19..18] */
        unsigned int rsv                        : 12; /* [31..20] */
    } bits;
    unsigned int u32;
} hdmi_reg_crg;

typedef struct {
    struct {
        unsigned int srst_req_hdmi_com          : 2;  /* [1..0] */
        unsigned int srst_req_ac_ctrl           : 2;  /* [3..2] */
        unsigned int srst_req_hdmitx_phy_bus    : 2;  /* [5..4] */
        unsigned int srst_req_ac_ctrl_bus       : 2;  /* [7..6] */
        unsigned int srst_req_hdmitx_phy        : 2;  /* [9..8] */
        unsigned int srst_req_hdmitx_ctrl       : 2;  /* [11..10] */
        unsigned int srst_req_hdmitx_ctrl_bus   : 2;  /* [13..12] */
        unsigned int srst_req_hdmitx_ctrl_cec   : 2;  /* [15..14] */
        unsigned int rsv                        : 16; /* [31..16] */
    } bits;
    unsigned int u32;
} hdmi_reg_srst;

typedef struct {
    struct {
        unsigned int icg_dis_ac_ctrl_modclk              : 2;  /* [1..0] */
        unsigned int icg_dis_hdmitx_phy_modclk           : 2;  /* [3..2] */
        unsigned int icg_dis_hdmitx_phy_tmds             : 2;  /* [5..4] */
        unsigned int icg_dis_hdmitx_ctrl_osc_24m         : 2;  /* [7..6] */
        unsigned int icg_dis_hdmitx_ctrl_cec             : 2;  /* [9..8] */
        unsigned int icg_dis_hdmitx_ctrl_as              : 2;  /* [11..10] */
        unsigned int icg_dis_hdmitx_ctrl_os              : 2;  /* [13..12] */
        unsigned int icg_dis_24m_core                    : 2;  /* [15..14] */
        unsigned int icg_dis_hdmitx_ctrl_cec_com         : 2;  /* [17..16] */
        unsigned int icg_dis_hdmi_pixel                  : 2;  /* [19..18] */
        unsigned int reserved                            : 12;  /* [31..20] */
    } bits;
    unsigned int u32;
} hdmi_reg_sc_icg_dis;

int hdmi_reg_crg_init(td_u32 id);
int hdmi_reg_crg_deinit(td_u32 id);
int hdmi_reg_pixel_cken_set(td_u32 id, td_bool enable);
int hdmi_reg_ctrl_osc_24m_cken_set(td_u32 id, td_bool enable);
int hdmi_reg_ctrl_cec_cken_set(td_u32 id, td_bool enable);
int hdmi_reg_ctrl_os_cken_set(td_u32 id, td_bool enable);
int hdmi_reg_ctrl_as_cken_set(td_u32 id, td_bool enable);
int hdmi_reg_ctrl_bus_srst_req_set(td_u32 id, td_bool enable);
int hdmi_reg_ctrl_srst_req_set(td_u32 id, td_bool enable);
int hdmi_reg_ctrl_cec_srst_req_set(td_u32 id, td_bool enable);
int hdmi_reg_hdmitx_phy_tmds_cken_set(td_u32 id, td_bool enable);
int hdmi_reg_hdmitx_phy_modclk_cken_set(td_u32 id, td_bool enable);
int hdmi_reg_ac_ctrl_modclk_cken_set(td_u32 id, td_bool enable);
int hdmi_reg_phy_srst_req_set(td_u32 id, td_bool enable);
int hdmi_reg_phy_bus_srst_req_set(td_u32 id, td_bool enable);
int hdmi_reg_ac_ctrl_srst_req_set(td_u32 id, td_bool enable);
int hdmi_reg_ac_ctrl_bus_srst_req_set(td_u32 id, td_bool enable);
int hdmi_reg_phy_clk_pctrl_set(td_u32 id, td_bool enable);
int hdmi_reg_ctrl_reset_set(td_u32 id, td_bool enable);
int hdmi_reg_phy_reset_set(td_u32 id, td_bool enable);
int hdmi_reg_top_reset_set(td_u32 id, td_bool enable);
int hdmi_reg_subctrl_clk_set(td_u32 id);

#endif

