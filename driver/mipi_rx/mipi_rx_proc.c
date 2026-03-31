/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: mipi proc function
 * Author: Hisilicon multimedia software group
 * Create: 2023-03-08
 */

#include "mipi_rx_proc.h"
#include "hi_mipi_rx.h"
#include "mipi_rx_config.h"
#include "mipi_rx_operate.h"
#include "mipi_rx_hal.h"
#include "slvs_hal.h"

static const char *mipi_print_mipi_wdr_mode(mipi_wdr_mode_t wdr_mode)
{
    if (wdr_mode == OT_MIPI_WDR_MODE_NONE) {
        return "none";
    } else if (wdr_mode == OT_MIPI_WDR_MODE_VC) {
        return "vc";
    } else if (wdr_mode == OT_MIPI_WDR_MODE_DT) {
        return "dt";
    } else if (wdr_mode == OT_MIPI_WDR_MODE_DOL) {
        return "dol";
    } else {
        return "n/a";
    }
}

static const char *mipi_print_lvds_wdr_mode(lvds_wdr_mode_t wdr_mode)
{
    if (wdr_mode == OT_LVDS_WDR_MODE_NONE) {
        return "none";
    } else if (wdr_mode == OT_LVDS_WDR_MODE_2F) {
        return "2f";
    } else if (wdr_mode == OT_LVDS_WDR_MODE_3F) {
        return "3f";
    } else if (wdr_mode == OT_LVDS_WDR_MODE_4F) {
        return "4f";
    } else if (wdr_mode == OT_LVDS_WDR_MODE_DOL_2F) {
        return "dol_2f";
    } else if (wdr_mode == OT_LVDS_WDR_MODE_DOL_3F) {
        return "dol_3f";
    } else if (wdr_mode == OT_LVDS_WDR_MODE_DOL_4F) {
        return "dol_4f";
    } else {
        return "n/a";
    }
}

static const char *mipi_get_intput_mode_name(input_mode_t input_mode)
{
    if ((input_mode == INPUT_MODE_SUBLVDS) || (input_mode == INPUT_MODE_LVDS) ||
        (input_mode == INPUT_MODE_HISPI)) {
        return "lvds";
    } else if (input_mode == INPUT_MODE_MIPI) {
        return "mipi";
    } else {
        return "n/a";
    }
}

static const char *mipi_get_raw_data_type_name(data_type_t type)
{
    if (type == DATA_TYPE_RAW_8BIT) {
        return "raw8";
    } else if (type == DATA_TYPE_RAW_10BIT) {
        return "raw10";
    } else if (type == DATA_TYPE_RAW_12BIT) {
        return "raw12";
    } else if (type == DATA_TYPE_RAW_14BIT) {
        return "raw14";
    } else if (type == DATA_TYPE_RAW_16BIT) {
        return "raw16";
    } else if (type == DATA_TYPE_YUV420_8BIT_NORMAL) {
        return "yuv420_8bit_normal";
    } else if (type == DATA_TYPE_YUV420_8BIT_LEGACY) {
        return "yuv420_8bit_legacy";
    } else if (type == DATA_TYPE_YUV422_8BIT) {
        return "yuv422_8bit";
    } else if (type == DATA_TYPE_YUV422_PACKED) {
        return "yuv422_packed";
    } else {
        return "n/a";
    }
}

static const char *mipi_get_data_rate_name(mipi_data_rate_t data_rate)
{
    if (data_rate == MIPI_DATA_RATE_X1) {
        return "X1";
    } else if (data_rate == MIPI_DATA_RATE_X2) {
        return "X2";
    } else {
        return "n/a";
    }
}

static void proc_show_mipi_device(const osal_proc_entry_t *s)
{
    const char *wdr_mode = NULL;
    unsigned int devno;
    input_mode_t input_mode;
    data_type_t data_type;

    osal_seq_printf(s,
                    "\n-----mipi dev attr--------------------"
                    "---------------------------------------------------------------------------------\n");

    osal_seq_printf(s, "%8s" "%11s" "%11s" "%20s" "%10s" "%8s" "%8s" "%8s" "%8s" "\n",
                    "devno", "work_mode", "data_rate", "data_type", "wdr_mode", "img_x", "img_y", "img_w", "img_h");

    for (devno = 0; devno < MIPI_RX_MAX_DEV_NUM; devno++) {
        osal_spin_lock(&g_mipi_ctx_spinlock);
        combo_dev_attr_t *pdev_attr = &g_mipi_dev_ctx.combo_dev_attr[devno];
        osal_spin_unlock(&g_mipi_ctx_spinlock);
        input_mode = pdev_attr->input_mode;

        if (mipi_is_dev_cfged(devno) == 0U) {
            continue;
        }

        if (input_mode == INPUT_MODE_MIPI) {
            data_type = pdev_attr->mipi_attr.input_data_type;
            wdr_mode = mipi_print_mipi_wdr_mode(pdev_attr->mipi_attr.wdr_mode);
        } else {
            data_type = pdev_attr->lvds_attr.input_data_type;
            wdr_mode = mipi_print_lvds_wdr_mode(pdev_attr->lvds_attr.wdr_mode);
        }

        osal_seq_printf(s, "%8u" "%11s" "%11s" "%20s" "%10s" "%8d" "%8d" "%8u" "%8u" "\n",
                        devno,
                        mipi_get_intput_mode_name(input_mode),
                        mipi_get_data_rate_name(pdev_attr->data_rate),
                        mipi_get_raw_data_type_name(data_type),
                        wdr_mode,
                        pdev_attr->img_rect.x,
                        pdev_attr->img_rect.y,
                        pdev_attr->img_rect.width,
                        pdev_attr->img_rect.height);
    }
}

static const char *mipi_print_lane_divide_mode(lane_divide_mode_t mode)
{
    if (mode == LANE_DIVIDE_MODE_0) {
        return "8";
    } else if (mode == LANE_DIVIDE_MODE_1) {
        return "4+4";
    } else if (mode == LANE_DIVIDE_MODE_2) {
        return "4+2+2";
    } else if (mode == LANE_DIVIDE_MODE_3) {
        return "2+2+2+2";
    } else {
        return "n/a";
    }
}

static void proc_show_mipi_hs_mode(const osal_proc_entry_t *s)
{
    lane_divide_mode_t lane_divide_mode;

    osal_spin_lock(&g_mipi_ctx_spinlock);
    lane_divide_mode = g_mipi_dev_ctx.lane_divide_mode;
    osal_spin_unlock(&g_mipi_ctx_spinlock);

    osal_seq_printf(s,
                    "\n-----mipi lane divide mdoe-----------"
                    "----------------------------------------------------------------------------------\n");
    osal_seq_printf(s, "%6s"
                    "%20s"
                    "\n",
                    "mode", "lane divide");
    osal_seq_printf(s, "%6d"
                    "%20s"
                    "\n",
                    (hi_s32)lane_divide_mode, mipi_print_lane_divide_mode(lane_divide_mode));
}

/* magic numbers represents the indexes of lane_id array */
static void proc_show_mipi_lane(const osal_proc_entry_t *s)
{
    unsigned int devno;
    input_mode_t input_mode;

    osal_seq_printf(s,
                    "\n-----mipi lane info-------------------"
                    "----------------------------------------------------------------------------------\n");

    osal_seq_printf(s, "%8s" "%24s" "\n",
                    "devno", "lane_id");

    for (devno = 0; devno < MIPI_RX_MAX_DEV_NUM; devno++) {
        osal_spin_lock(&g_mipi_ctx_spinlock);
        combo_dev_attr_t *pdev_attr = &g_mipi_dev_ctx.combo_dev_attr[devno];
        osal_spin_unlock(&g_mipi_ctx_spinlock);
        input_mode = pdev_attr->input_mode;

        if (mipi_is_dev_cfged(devno) == 0U) {
            continue;
        }

        if (input_mode == INPUT_MODE_MIPI) {
            osal_seq_printf(s, "%8u" "%10d,%3d,%3d,%3d,%3d,%3d,%3d,%3d" "\n",
                            devno,
                            pdev_attr->mipi_attr.lane_id[0], // Whether MIPI lane 0 is enable
                            pdev_attr->mipi_attr.lane_id[1], // Whether MIPI lane 1 is enable
                            pdev_attr->mipi_attr.lane_id[2], // Whether MIPI lane 2 is enable
                            pdev_attr->mipi_attr.lane_id[3], // Whether MIPI lane 3 is enable
                            pdev_attr->mipi_attr.lane_id[4], // Whether MIPI lane 4 is enable
                            pdev_attr->mipi_attr.lane_id[5], // Whether MIPI lane 5 is enable
                            pdev_attr->mipi_attr.lane_id[6], // Whether MIPI lane 6 is enable
                            pdev_attr->mipi_attr.lane_id[7]); // Whether MIPI lane 7 is enable
        } else if (input_mode == INPUT_MODE_LVDS ||
                   input_mode == INPUT_MODE_SUBLVDS ||
                   input_mode == INPUT_MODE_HISPI) {
            osal_seq_printf(s, "%8u" "%10d,%3d,%3d,%3d,%3d,%3d,%3d,%3d" "\n",
                            devno,
                            pdev_attr->lvds_attr.lane_id[0], // Whether sublvds lane 0 is enable
                            pdev_attr->lvds_attr.lane_id[1], // Whether sublvds lane 1 is enable
                            pdev_attr->lvds_attr.lane_id[2], // Whether sublvds lane 2 is enable
                            pdev_attr->lvds_attr.lane_id[3], // Whether sublvds lane 3 is enable
                            pdev_attr->lvds_attr.lane_id[4], // Whether sublvds lane 4 is enable
                            pdev_attr->lvds_attr.lane_id[5], // Whether sublvds lane 5 is enable
                            pdev_attr->lvds_attr.lane_id[6], // Whether sublvds lane 6 is enable
                            pdev_attr->lvds_attr.lane_id[7]); // Whether sublvds lane 7 is enable
        } else {
            // fix sc
        }
    }
}

static void proc_show_mipi_phy_data(const osal_proc_entry_t *s)
{
    int i;
    unsigned int freq;

    osal_seq_printf(s, "\n-----mipi phy data info------------------------------------------------------\n");
    osal_seq_printf(s, "%8s" "%15s" "%19s" "%24s" "%22s" "%16s" "\n",
                    "phy_id", "lane_id", "phy_data", "mipi_data", "lvds_data", "freq");

    for (i = 0; i < (hi_s32)MIPI_RX_MAX_PHY_NUM; i++) {
        osal_seq_printf(s,
                        "%8d%8d,%2d,%2d,%2d    "
                        "0x%02x,0x%02x,0x%02x,0x%02x    "
                        "0x%02x,0x%02x,0x%02x,0x%02x    "
                        "0x%02x,0x%02x,0x%02x,0x%02x    ",
                        i,
                        4 * i, 4 * i + 1, 4 * i + 2, 4 * i + 3, /* Both PHY has 4 Lane, value: 0, 1, 2, 3 */
                        mipi_rx_drv_get_phy_data(i, 0),
                        mipi_rx_drv_get_phy_data(i, 1),
                        mipi_rx_drv_get_phy_data(i, 2),            /* Lane 2 in PHY_i */
                        mipi_rx_drv_get_phy_data(i, 3),            /* Lane 3 in PHY_i */
                        mipi_rx_drv_get_phy_mipi_link_data(i, 0),
                        mipi_rx_drv_get_phy_mipi_link_data(i, 1),
                        mipi_rx_drv_get_phy_mipi_link_data(i, 2),  /* Lane 2 in PHY_i */
                        mipi_rx_drv_get_phy_mipi_link_data(i, 3),  /* Lane 3 in PHY_i */
                        mipi_rx_drv_get_phy_lvds_link_data(i, 0),
                        mipi_rx_drv_get_phy_lvds_link_data(i, 1),
                        mipi_rx_drv_get_phy_lvds_link_data(i, 2),  /* Lane 2 in PHY_i */
                        mipi_rx_drv_get_phy_lvds_link_data(i, 3)); /* Lane 3 in PHY_i */
        freq = mipi_rx_drv_get_phy_freq(i);
        if (freq == 0) {
            osal_seq_printf(s, "%6s\n", "n/a");
        } else {
            osal_seq_printf(s, "%3uMHZ\n", freq);
        }
    }
}

static void proc_show_mipi_detect_info(const osal_proc_entry_t *s, const unsigned int devno_array[], int mipi_cnt)
{
    img_size_t image_size;
    short vc_num;
    int devno_idx;

    osal_seq_printf(s, "\n-----mipi detect info----------------------------------------------------\n");
    osal_seq_printf(s, "%6s%3s%8s%8s\n", "devno", "vc", "width", "height");

    for (devno_idx = 0; devno_idx < mipi_cnt; devno_idx++) {
        for (vc_num = 0; vc_num < WDR_VC_NUM; vc_num++) {
            mipi_rx_drv_get_mipi_imgsize_statis(devno_array[devno_idx], vc_num, &image_size);
            osal_seq_printf(s, "%6u%3d%8u%8u\n",
                            devno_array[devno_idx], vc_num, image_size.width, image_size.height);
        }
    }
}


static void proc_show_lvds_detect_info(const osal_proc_entry_t *s, const unsigned int devno_array[], int mipi_cnt)
{
    img_size_t image_size;
    short vc_num;
    int devno_idx;

    osal_seq_printf(s, "\n-----lvds detect info----------------------------------------------------\n");
    osal_seq_printf(s, "%6s%3s%8s%8s\n", "devno", "vc", "width", "height");

    for (devno_idx = 0; devno_idx < mipi_cnt; devno_idx++) {
        for (vc_num = 0; vc_num < WDR_VC_NUM; vc_num++) {
            mipi_rx_drv_get_lvds_imgsize_statis(devno_array[devno_idx], vc_num, &image_size);
            osal_seq_printf(s, "%6u%3d%8u%8u\n",
                            devno_array[devno_idx], vc_num, image_size.width, image_size.height);
        }
    }
}

static void proc_show_lvds_lane_detect_info(const osal_proc_entry_t *s, const unsigned int devno_array[], int mipi_cnt)
{
    img_size_t image_size;
    short lane;
    int devno_idx;
    combo_dev_attr_t *pstcombo_dev_attr = NULL;

    osal_seq_printf(s, "\n-----lvds lane detect info----------------------------------------------------\n");
    osal_seq_printf(s, "%6s%6s%8s%8s\n", "devno", "lane", "width", "height");

    for (devno_idx = 0; devno_idx < mipi_cnt; devno_idx++) {
        osal_spin_lock(&g_mipi_ctx_spinlock);
        pstcombo_dev_attr = &g_mipi_dev_ctx.combo_dev_attr[devno_array[devno_idx]];
        osal_spin_unlock(&g_mipi_ctx_spinlock);
        for (lane = 0; lane < LVDS_LANE_NUM; lane++) {
            if (pstcombo_dev_attr->lvds_attr.lane_id[lane] != -1) {
                mipi_rx_drv_get_lvds_lane_imgsize_statis(devno_array[devno_idx], lane, &image_size);
                osal_seq_printf(s, "%6u%6d%8u%8u\n",
                                devno_array[devno_idx],
                                pstcombo_dev_attr->lvds_attr.lane_id[lane],
                                image_size.width, image_size.height);
            }
        }
    }
}

// PHY CIL Proc Error Info.
static void proc_show_phy_cil_int_err_cnt(const osal_proc_entry_t *s)
{
    unsigned int phy_id;
    phy_err_int_cnt_t *phy_err_int_cnt = NULL;

    osal_seq_printf(s, "\n-----phy cil err int info---------------------------------------------\n");
    osal_seq_printf(s, "%8s%11s%10s%12s%12s%12s%12s%9s%8s%10s%10s%10s%10s\n",
                    "phy_id",
                    "clk2_tmout", "clk_tmout", "lane0_tmout", "lane1_tmout", "lane2_tmout", "lane3_tmout",
                    "clk2_esc", "clk_esc", "lane0_esc", "lane1_esc", " lane2_esc", "lane3_esc");

    for (phy_id = 0; phy_id < MIPI_RX_MAX_PHY_NUM; phy_id++) {
        phy_err_int_cnt = mipi_rx_drv_get_phy_err_int_cnt(phy_id);
        osal_seq_printf(s, "%8u%11u%10u%12u%12u%12u%12u%9u%8u%10u%10u%10u%10u\n",
                        phy_id,
                        phy_err_int_cnt->clk2_fsm_timeout_err_cnt,
                        phy_err_int_cnt->clk_fsm_timeout_err_cnt,
                        phy_err_int_cnt->d0_fsm_timeout_err_cnt,
                        phy_err_int_cnt->d1_fsm_timeout_err_cnt,
                        phy_err_int_cnt->d2_fsm_timeout_err_cnt,
                        phy_err_int_cnt->d3_fsm_timeout_err_cnt,

                        phy_err_int_cnt->clk2_fsm_escape_err_cnt,
                        phy_err_int_cnt->clk_fsm_escape_err_cnt,
                        phy_err_int_cnt->d0_fsm_escape_err_cnt,
                        phy_err_int_cnt->d1_fsm_escape_err_cnt,
                        phy_err_int_cnt->d2_fsm_escape_err_cnt,
                        phy_err_int_cnt->d3_fsm_escape_err_cnt);
    }
}

static void proc_show_mipi_err_info1(const osal_proc_entry_t *s, const unsigned int devno_array[], int mipi_cnt)
{
    int devno_idx;
    unsigned int devno;
    mipi_err_int_cnt_t *mipi_err_int_cnt = NULL;

    osal_seq_printf(s, "\n-----mipi error int info1-----------------------------------------------------------\n");
    osal_seq_printf(s, "%8s%6s%9s%9s%9s%9s%16s%16s%16s%16s\n",
                    "devno",
                    "ecc2",
                    "vc0_crc", "vc1_crc", "vc2_crc", "vc3_crc",
                    "vc0_ecc_corrct", "vc1_ecc_corrct", "vc2_ecc_corrct", "vc3_ecc_corrct");

    for (devno_idx = 0; devno_idx < mipi_cnt; devno_idx++) {
        devno = devno_array[devno_idx];
        mipi_err_int_cnt = mipi_rx_drv_get_mipi_err_int_cnt(devno);
        osal_seq_printf(s, "%8u%6u%9u%9u%9u%9u%16u%16u%16u%16u\n",
                        devno,
                        mipi_err_int_cnt->err_ecc_double_cnt,
                        mipi_err_int_cnt->vc0_err_crc_cnt,
                        mipi_err_int_cnt->vc1_err_crc_cnt,
                        mipi_err_int_cnt->vc2_err_crc_cnt,
                        mipi_err_int_cnt->vc3_err_crc_cnt,
                        mipi_err_int_cnt->vc0_err_ecc_corrected_cnt,
                        mipi_err_int_cnt->vc1_err_ecc_corrected_cnt,
                        mipi_err_int_cnt->vc2_err_ecc_corrected_cnt,
                        mipi_err_int_cnt->vc3_err_ecc_corrected_cnt);
    }
}

static void proc_show_mipi_err_info2(const osal_proc_entry_t *s, const unsigned int devno_array[], int mipi_cnt)
{
    int devno_idx;
    unsigned int devno;
    mipi_err_int_cnt_t *mipi_err_int_cnt = NULL;

    osal_seq_printf(s, "\n-----mipi error int info2-----------------------------------------------------------\n");
    osal_seq_printf(s, "%8s%12s%12s%12s%12s%15s%15s%15s%15s\n",
                    "devno",
                    "vc0_frmnum", "vc1_frmnum", "vc2_frmnum", "vc3_frmnum",
                    "vc0_frmmmatch", "vc1_frmmmatch", "vc2_frmmmatch", "vc3_frmmmatch");

    for (devno_idx = 0; devno_idx < mipi_cnt; devno_idx++) {
        devno = devno_array[devno_idx];
        mipi_err_int_cnt = mipi_rx_drv_get_mipi_err_int_cnt(devno);
        osal_seq_printf(s, "%8u%12u%12u%12u%12u%15u%15u%15u%15u\n",
                        devno,
                        mipi_err_int_cnt->vc0_err_frame_num_cnt,
                        mipi_err_int_cnt->vc1_err_frame_num_cnt,
                        mipi_err_int_cnt->vc2_err_frame_num_cnt,
                        mipi_err_int_cnt->vc3_err_frame_num_cnt,
                        mipi_err_int_cnt->vc0_err_frame_s_e_num_mismatch_cnt,
                        mipi_err_int_cnt->vc1_err_frame_s_e_num_mismatch_cnt,
                        mipi_err_int_cnt->vc2_err_frame_s_e_num_mismatch_cnt,
                        mipi_err_int_cnt->vc3_err_frame_s_e_num_mismatch_cnt);
    }
}

static void proc_show_mipi_err_info3(const osal_proc_entry_t *s, const unsigned int devno_array[], int mipi_cnt)
{
    int devno_idx;
    unsigned int devno;
    mipi_err_int_cnt_t *mipi_err_int_cnt = NULL;

    osal_seq_printf(s, "\n-----mipi error int info3-----------------------------------------------------------\n");
    osal_seq_printf(s, "%8s%17s%16s%7s%16s%17s\n",
                    "devno",
                    "data_fifo_rderr", "cmd_fifo_rderr", "vsync", "cmd_fifo_wrerr", "data_fifo_wrerr");

    for (devno_idx = 0; devno_idx < mipi_cnt; devno_idx++) {
        devno = devno_array[devno_idx];
        mipi_err_int_cnt = mipi_rx_drv_get_mipi_err_int_cnt(devno);
        osal_seq_printf(s, "%8u%17u%16u%7u%16u%17u\n",
                        devno,
                        mipi_err_int_cnt->data_fifo_rderr_cnt,
                        mipi_err_int_cnt->cmd_fifo_rderr_cnt,
                        mipi_err_int_cnt->vsync_cnt,
                        mipi_err_int_cnt->data_fifo_wrerr_cnt,
                        mipi_err_int_cnt->cmd_fifo_wrerr_cnt);
    }
}

static void proc_show_mipi_int_err(const osal_proc_entry_t *s, const unsigned int devno_array[], int mipi_cnt)
{
    proc_show_mipi_err_info1(s, devno_array, mipi_cnt);
    proc_show_mipi_err_info2(s, devno_array, mipi_cnt);
    proc_show_mipi_err_info3(s, devno_array, mipi_cnt);
}

static void proc_show_lvds_int_err(const osal_proc_entry_t *s, const unsigned int devno_array[], int lvds_cnt)
{
    unsigned int devno;
    int devno_idx;
    lvds_err_int_cnt_t *lvds_err_int_cnt = NULL;

    osal_seq_printf(s, "\n-----lvds error int info1-----------------------------------------------------------\n");
    osal_seq_printf(s, "%8s%7s%11s%11s%9s%10s\n", "devno", "vsync", "cmd_rderr", "cmd_wrerr", "pop_err", "stat_err");

    for (devno_idx = 0; devno_idx < lvds_cnt; devno_idx++) {
        devno = devno_array[devno_idx];
        lvds_err_int_cnt = mipi_rx_drv_get_lvds_err_int_cnt(devno);
        osal_seq_printf(s, "%8u%7u%11u%11u%9u%10u\n", devno,
                        lvds_err_int_cnt->lvds_vsync_cnt,
                        lvds_err_int_cnt->cmd_rd_err_cnt,
                        lvds_err_int_cnt->cmd_wr_err_cnt,
                        lvds_err_int_cnt->pop_err_cnt,
                        lvds_err_int_cnt->lvds_state_err_cnt);
    }

    osal_seq_printf(s, "\n-----lvds error int info2-----------------------------------------------------------\n");
    osal_seq_printf(s, "%8s%13s%13s%13s%13s%13s%13s\n",
                    "devno", "link0_wrerr", "link1_wrerr", "link2_wrerr", "link0_rderr", "link1_rderr", "link2_rderr");

    for (devno_idx = 0; devno_idx < lvds_cnt; devno_idx++) {
        devno = devno_array[devno_idx];
        lvds_err_int_cnt = mipi_rx_drv_get_lvds_err_int_cnt(devno);
        osal_seq_printf(s, "%8u%13u%13u%13u%13u%13u%13u\n", devno,
                        lvds_err_int_cnt->link0_wr_err_cnt,
                        lvds_err_int_cnt->link1_wr_err_cnt,
                        lvds_err_int_cnt->link2_wr_err_cnt,
                        lvds_err_int_cnt->link0_rd_err_cnt,
                        lvds_err_int_cnt->link1_rd_err_cnt,
                        lvds_err_int_cnt->link2_rd_err_cnt);
    }

    osal_seq_printf(s, "\n-----lvds error int info3-----------------------------------------------------------\n");
    osal_seq_printf(s, "%8s%11s%11s%11s%11s%11s%11s%11s%11s\n",
                    "devno", "lane0_err", "lane1_err", "lane2_err", "lane3_err",
                    "lane4_err", "lane5_err", "lane6_err", "lane7_err");

    for (devno_idx = 0; devno_idx < lvds_cnt; devno_idx++) {
        devno = devno_array[devno_idx];
        lvds_err_int_cnt = mipi_rx_drv_get_lvds_err_int_cnt(devno);
        osal_seq_printf(s, "%8u%11u%11u%11u%11u%11u%11u%11u%11u\n", devno,
                        lvds_err_int_cnt->lane0_sync_err_cnt,
                        lvds_err_int_cnt->lane1_sync_err_cnt,
                        lvds_err_int_cnt->lane2_sync_err_cnt,
                        lvds_err_int_cnt->lane3_sync_err_cnt,
                        lvds_err_int_cnt->lane4_sync_err_cnt,
                        lvds_err_int_cnt->lane5_sync_err_cnt,
                        lvds_err_int_cnt->lane6_sync_err_cnt,
                        lvds_err_int_cnt->lane7_sync_err_cnt);
    }
}

static void proc_show_align_int_err(const osal_proc_entry_t *s)
{
    unsigned int devno;
    align_err_int_cnt_t *align_err_int_cnt = NULL;

    osal_seq_printf(s, "\n-----align error int info--------------------------------------\n");

    for (devno = 0; devno < MIPI_RX_MAX_DEV_NUM; devno++) {
        if (mipi_is_dev_cfged(devno) == 0U) {
            continue;
        }
        align_err_int_cnt = mipi_rx_drv_get_align_err_int_cnt(devno);
        osal_seq_printf(s, "%8s%14s%11s%11s%11s%11s%11s%11s%11s%11s\n",
                        "devno", "fifo_fullerr",
                        "lane0_err", "lane1_err", "lane2_err", "lane3_err", "lane4_err", "lane5_err",
                        "lane6_err", "lane7_err");

        osal_seq_printf(s, "%8u%14u%11u%11u%11u%11u%11u%11u%11u%11u\n",
                        devno,
                        align_err_int_cnt->fifo_full_err_cnt,
                        align_err_int_cnt->lane0_align_err_cnt,
                        align_err_int_cnt->lane1_align_err_cnt,
                        align_err_int_cnt->lane2_align_err_cnt,
                        align_err_int_cnt->lane3_align_err_cnt,
                        align_err_int_cnt->lane4_align_err_cnt,
                        align_err_int_cnt->lane5_align_err_cnt,
                        align_err_int_cnt->lane6_align_err_cnt,
                        align_err_int_cnt->lane7_align_err_cnt);
    }
}

static hi_void mipi_proc_show(const osal_proc_entry_t *s, unsigned int devno_mipi[], int mipi_cnt,
                              const unsigned int devno_lvds[], int lvds_cnt)
{
    if (mipi_cnt > 0 || lvds_cnt > 0) {
        proc_show_mipi_hs_mode(s);
        proc_show_mipi_device(s);
        proc_show_mipi_lane(s);
        proc_show_mipi_phy_data(s);
    }

    if (mipi_cnt > 0) {
        proc_show_mipi_detect_info(s, devno_mipi, mipi_cnt);
    }

    if (lvds_cnt > 0) {
        proc_show_lvds_detect_info(s, devno_lvds, lvds_cnt);
        proc_show_lvds_lane_detect_info(s, devno_lvds, lvds_cnt);
    }

    if (mipi_cnt > 0 || lvds_cnt > 0) {
        proc_show_phy_cil_int_err_cnt(s);
    }

    if (mipi_cnt > 0) {
        proc_show_mipi_int_err(s, devno_mipi, mipi_cnt);
    }

    if (lvds_cnt > 0) {
        proc_show_lvds_int_err(s, devno_lvds, lvds_cnt);
    }

    if (mipi_cnt > 0 || lvds_cnt > 0) {
        proc_show_align_int_err(s);
    }

    return;
}

hi_void mipi_rx_proc_show(const osal_proc_entry_t *s)
{
    unsigned int devno;
    int mipi_cnt = 0;
    int lvds_cnt = 0;
    input_mode_t input_mode;
    combo_dev_attr_t *pdev_attr = NULL;
    unsigned int devno_mipi[MIPI_RX_MAX_DEV_NUM] = {0};
    unsigned int devno_lvds[MIPI_RX_MAX_DEV_NUM] = {0};

    for (devno = 0; devno < MIPI_RX_MAX_DEV_NUM; devno++) {
        if (mipi_is_dev_cfged(devno) == 0U) {
            continue;
        }

        osal_spin_lock(&g_mipi_ctx_spinlock);
        pdev_attr = &g_mipi_dev_ctx.combo_dev_attr[devno];
        input_mode = pdev_attr->input_mode;
        osal_spin_unlock(&g_mipi_ctx_spinlock);

        if (input_mode == INPUT_MODE_MIPI) {
            devno_mipi[mipi_cnt] = devno;
            mipi_cnt++;
        } else if ((input_mode == INPUT_MODE_LVDS) || (input_mode == INPUT_MODE_SUBLVDS) ||
                   (input_mode == INPUT_MODE_HISPI)) {
            devno_lvds[lvds_cnt] = devno;
            lvds_cnt++;
        } else {
            // fix sc
        }
    }

    osal_seq_printf(s, "\nModule: [MIPI_RX]\n\n");

    mipi_proc_show(s, devno_mipi, mipi_cnt, devno_lvds, lvds_cnt);

    return;
}


static const char* slvs_print_slvs_lane_rate(slvs_lane_rate_t lane_rate)
{
    if (lane_rate == SLVS_LANE_RATE_LOW) {
        return "LOW";
    } else if (lane_rate == SLVS_LANE_RATE_HIGH) {
        return "HIGHT";
    } else {
        return "N/A";
    }
}

static const char* slvs_print_slvs_err_check_mode(slvs_err_check_mode_t err_check_mode)
{
    if (err_check_mode == SLVS_ERR_CHECK_MODE_NONE) {
        return "NONE";
    } else if (err_check_mode == SLVS_ERR_CHECK_MODE_CRC) {
        return "CRC";
    }  else if (err_check_mode == SLVS_ERR_CHECK_MODE_ECC_2BYTE) {
        return "2ByteECC";
    }  else if (err_check_mode == SLVS_ERR_CHECK_MODE_ECC_4BYTE) {
        return "4ByteECC";
    } else {
        return "N/A";
    }
}

static void proc_show_slvs_device(const osal_proc_entry_t* s)
{
    unsigned int devno;
    data_type_t data_type = DATA_TYPE_BUTT;

    osal_seq_printf(s, "\n\n-----SLVS DEV ATTR---------------------------------"
                    "--------------------------------------------------------------------\n");

    osal_seq_printf(s, "%8s"  "%10s"      "%10s"      "%10s"      "%10s"
                    "%8s"   "%8s"   "%8s"    "%8s"    "%10s"      "%10s"     "%10s""\n",
                    "Devno", "WorkMode", "DataRate", "DataType", "WDRMode",
                    "ImgX", "ImgY", "ImgW",  "ImgH", "LaneRate",  "ValidW",  "ErrMode");

    for (devno = SLVS_DEV_NUM_START; devno < SLVS_DEV_NUM_START + SLVS_MAX_DEV_NUM; devno++) {
        osal_spin_lock(&g_mipi_ctx_spinlock);
        combo_dev_attr_t *pdev_attr = &g_mipi_dev_ctx.combo_dev_attr[devno];
        osal_spin_unlock(&g_mipi_ctx_spinlock);
        if (mipi_is_dev_cfged(devno) != 0x1) {
            continue;
        }

        data_type = pdev_attr->slvs_attr.input_data_type;

        osal_seq_printf(s, "%8u" "%10s" "%10s" "%10s" "%10s" "%8d" "%8d" "%8u" "%8u" "%10s" "%10d" "%10s" "\n",
                        devno,
                        mipi_get_intput_mode_name(WORK_MODE_SLVS),
                        mipi_get_data_rate_name(pdev_attr->data_rate),
                        mipi_get_raw_data_type_name(data_type),
                        mipi_print_lvds_wdr_mode(pdev_attr->slvs_attr.wdr_mode),
                        pdev_attr->img_rect.x,
                        pdev_attr->img_rect.y,
                        pdev_attr->img_rect.width,
                        pdev_attr->img_rect.height,
                        slvs_print_slvs_lane_rate(pdev_attr->slvs_attr.lane_rate),
                        pdev_attr->slvs_attr.sensor_valid_width,
                        slvs_print_slvs_err_check_mode(pdev_attr->slvs_attr.err_check_mode));
    }
}

static void proc_show_slvs_lane(const osal_proc_entry_t* s)
{
    unsigned int devno;
    osal_seq_printf(s, "\n-----SLVS LANE INFO------------------------------"
                    "-----------------------------------------------------------------------\n");

    osal_seq_printf(s, "%8s"     "%24s" "\n",
                    "Devno", "LaneID");

    for (devno = SLVS_DEV_NUM_START; devno < SLVS_DEV_NUM_START + SLVS_MAX_DEV_NUM; devno++) {
        osal_spin_lock(&g_mipi_ctx_spinlock);
        combo_dev_attr_t *pdev_attr = &g_mipi_dev_ctx.combo_dev_attr[devno];
        osal_spin_unlock(&g_mipi_ctx_spinlock);
        if (mipi_is_dev_cfged(devno) != 0x1) {
            continue;
        }

        osal_seq_printf(s, "%8u" "%10d,%3d,%3d,%3d,%3d,%3d,%3d,%3d" "\n",
                        devno,
                        pdev_attr->slvs_attr.lane_id[0], // lane 0
                        pdev_attr->slvs_attr.lane_id[1], // lane 1
                        pdev_attr->slvs_attr.lane_id[2], // lane 2
                        pdev_attr->slvs_attr.lane_id[3], // lane 3
                        pdev_attr->slvs_attr.lane_id[4], // lane 4
                        pdev_attr->slvs_attr.lane_id[5], // lane 5
                        pdev_attr->slvs_attr.lane_id[6], // lane 6
                        pdev_attr->slvs_attr.lane_id[7]); // lane 7
    }
}

static void proc_show_slvs_link_info(const osal_proc_entry_t* s)
{
    unsigned int devno;
    HI_S32 idx;
    unsigned int valid_mask;
    unsigned int phy_data = 0;
    unsigned int aligned_data = 0;
    short        lane_id;

    osal_seq_printf(s, "\n-----SLVS DATA INFO------------------------------------------------------\n");

    osal_seq_printf(s, "%8s"  "%10s"       "%12s"      "%14s"         "%12s"  "\n",
                    "Devno",  "LaneID",    "PhyData", "AlignedData", "ValidLane");

    for (devno = SLVS_DEV_NUM_START;
        devno < SLVS_DEV_NUM_START + SLVS_MAX_DEV_NUM; devno++) {
        osal_spin_lock(&g_mipi_ctx_spinlock);
        combo_dev_attr_t *pdev_attr = &g_mipi_dev_ctx.combo_dev_attr[devno];
        osal_spin_unlock(&g_mipi_ctx_spinlock);
        if (mipi_is_dev_cfged(devno) != 0x1) {
            continue;
        }

        valid_mask = slvs_drv_get_phy_valid_lane(devno);

        for (idx = 0; idx < SLVS_LANE_NUM; idx++) {
            if (-1 != pdev_attr->slvs_attr.lane_id[idx]) {
                lane_id = pdev_attr->slvs_attr.lane_id[idx];

                slvs_drv_get_phy_data(devno, lane_id, &phy_data);
                slvs_drv_get_phy_aligned_data(devno, lane_id, &aligned_data);

                osal_seq_printf(s, "%8u" "%10d" "%#12x" "%#12x" "\n",
                                devno,
                                lane_id,
                                phy_data,
                                aligned_data);
            }
        }
    }
}

static void proc_show_slvs_detect_info(const osal_proc_entry_t* s)
{
    img_size_t image_size;
    unsigned int devno;
    short vc_num;

    osal_seq_printf(s, "\n-----SLVS DETECT INFO-----------------------------------------------------------\n");
    osal_seq_printf(s, "%8s"    "%4s" "%8s"    "%8s"  "\n",
                    "Devno", "VC", "width", "height");

    for (devno = SLVS_DEV_NUM_START; devno < SLVS_DEV_NUM_START + SLVS_MAX_DEV_NUM; devno++) {
        if (mipi_is_dev_cfged(devno) != 0x1) {
            continue;
        }

        for (vc_num = 0; vc_num < WDR_VC_NUM; vc_num++) {
            slvs_drv_get_imgsize_statis(devno, vc_num, &image_size);
            osal_seq_printf(s, "%8u"    "%4d" "%8u"    "%8u" "\n",
                            devno, vc_num, image_size.width, image_size.height);
        }
    }
}

hi_void slvs_proc_show(const osal_proc_entry_t* s)
{
    unsigned int devno_idx;
    int slvs_cnt = 0;
    input_mode_t input_mode;
    combo_dev_attr_t* pdev_attr;

    for (devno_idx = SLVS_DEV_NUM_START; devno_idx < SLVS_DEV_NUM_START + SLVS_MAX_DEV_NUM; devno_idx++) {
        osal_spin_lock(&g_mipi_ctx_spinlock);
        pdev_attr = &g_mipi_dev_ctx.combo_dev_attr[devno_idx];
        osal_spin_unlock(&g_mipi_ctx_spinlock);
        if (mipi_is_dev_cfged(devno_idx) != 0x1) {
            continue;
        }

        input_mode = pdev_attr->input_mode;

        if (input_mode == INPUT_MODE_SLVS) {
            slvs_cnt++;
        }
    }
    osal_seq_printf(s, "\n-----slvs_proc_show 1, slvs_cnt=%d-----------\n", slvs_cnt);
    if (slvs_cnt > 0) {
        proc_show_slvs_device(s);
        proc_show_slvs_lane(s);

        proc_show_slvs_link_info(s);

        proc_show_slvs_detect_info(s);

        proc_show_slvs_dev_err_info(s);
        proc_show_slvs_phy_info(s);
    }
    return;
}