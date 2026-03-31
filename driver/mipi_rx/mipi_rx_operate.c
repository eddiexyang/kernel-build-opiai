/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: mipi operate and ioctl function
 * Author: Hisilicon multimedia software group
 * Create: 2023-03-08
 */
#include "securec.h"
#include "mipi_rx_operate.h"
#include "hi_osal.h"
#include "mipi_rx_config.h"
#include "mipi_rx_hal.h"
#include "slvs_hal.h"
#include "mipi_rx_type.h"

#define COMBO_MAX_LANE_NUM 8U
#define COMBO_MIN_WIDTH  32U
#define COMBO_MIN_HEIGHT 32U

static hi_u32 g_mipi_rx_status;
static osal_atomic_t g_mipi_rx_user_ref;

static hi_s32 mipi_do_exit(void);

unsigned char mipi_is_dev_cfged(combo_dev_t devno)
{
    unsigned char dev_cfged;

    osal_spin_lock(&g_mipi_ctx_spinlock);
    dev_cfged = g_mipi_dev_ctx.dev_cfged[devno];
    osal_spin_unlock(&g_mipi_ctx_spinlock);

    return dev_cfged;
}

/* function definition */
static unsigned char mipi_is_hs_mode_cfged(void)
{
    unsigned char hs_mode_cfged;

    osal_spin_lock(&g_mipi_ctx_spinlock);
    hs_mode_cfged = g_mipi_dev_ctx.hs_mode_cfged;
    osal_spin_unlock(&g_mipi_ctx_spinlock);

    return hs_mode_cfged;
}

static unsigned char mipi_is_dev_valid(combo_dev_t devno)
{
    unsigned char dev_valid;

    osal_spin_lock(&g_mipi_ctx_spinlock);
    dev_valid = g_mipi_dev_ctx.dev_valid[devno];
    osal_spin_unlock(&g_mipi_ctx_spinlock);

    return dev_valid;
}

static int check_lane_id(combo_dev_t devno, input_mode_t input_mode, const short p_lane_id[])
{
    int lane_num;
    int all_lane_id_invalid_flag = 1;
    int i, j;
    lane_divide_mode_t cur_lane_divide_mode;

    if (input_mode == INPUT_MODE_MIPI) {
        lane_num = MIPI_LANE_NUM;
    } else if (input_mode == INPUT_MODE_LVDS) {
        lane_num = LVDS_LANE_NUM;
    } else if (input_mode == INPUT_MODE_SLVS) {
        lane_num = SLVS_LANE_NUM;
    } else {
        return 0;
    }

    osal_spin_lock(&g_mipi_ctx_spinlock);
    cur_lane_divide_mode = g_mipi_dev_ctx.lane_divide_mode;
    osal_spin_unlock(&g_mipi_ctx_spinlock);

    for (i = 0; i < lane_num; i++) {
        short temp_id = p_lane_id[i];
        int lane_valid;

        if (temp_id < -1 || temp_id >= (hi_s32)COMBO_MAX_LANE_NUM) {
            mipi_rx_err_trace("lane_id[%d] is invalid value %d, should be [-1, %u).\n",
                i, temp_id, COMBO_MAX_LANE_NUM);
            return -1;
        }

        if (temp_id == -1) {
            continue;
        }

        all_lane_id_invalid_flag = 0;

        for (j = i + 1; j < lane_num; j++) {
            if (temp_id == p_lane_id[j]) {
                mipi_rx_err_trace("lane_id[%d] can't be same value %d as lane_id[%d]\n", i, temp_id, j);
                return -1;
            }
        }

        lane_valid = mipi_rx_drv_is_lane_valid(devno, temp_id, cur_lane_divide_mode);
        if ((input_mode != INPUT_MODE_SLVS) && (lane_valid == 0)) { // mipi 模式不是slvs 时，才判断引脚和dev绑定是不是有效
            mipi_rx_err_trace("devno:%d, lane_id[%d]:%d,is invalid in hs_mode %u\n",
                devno, i, temp_id, (hi_u32)cur_lane_divide_mode);
            switch (cur_lane_divide_mode) {
                case LANE_DIVIDE_MODE_0:
                    mipi_rx_err_trace("devno should be 0, and lane_id should be in [0,7]\n");
                    break;
                case LANE_DIVIDE_MODE_1:
                    mipi_rx_err_trace("devno should be 0, and lane_id should be in [0,3]\n");
                    mipi_rx_err_trace("or devno should be 2, and lane_id should be in [4,7]\n");
                    break;
                case LANE_DIVIDE_MODE_2:
                    mipi_rx_err_trace("devno should be 0, and lane_id should be in [0,3]\n");
                    mipi_rx_err_trace("or devno should be 2, and lane_id should be 4 or 6\n");
                    mipi_rx_err_trace("or devno should be 3, and lane_id should be 5 or 7\n");
                    break;
                case LANE_DIVIDE_MODE_3:
                    mipi_rx_err_trace("devno should be 0, and lane_id should be 0 or 2\n");
                    mipi_rx_err_trace("or devno should be 1, and lane_id should be 1 or 3\n");
                    mipi_rx_err_trace("or devno should be 2, and lane_id should be 4 or 6\n");
                    mipi_rx_err_trace("or devno should be 3, and lane_id should be 5 or 7\n");
                    break;
                default:
                    break;
            }
            return -1;
        }
    }

    if (all_lane_id_invalid_flag != 0) {
        mipi_rx_err_trace("all lane_id is invalid!\n");
        return -1;
    }

    return 0;
}

static hi_u32 mipi_get_lane_bitmap(input_mode_t input_mode,
    const short *p_lane_id, hi_u32 *p_total_lane_num)
{
    hi_u32 lane_bitmap = 0;
    hi_u32 lane_num;
    hi_u32 total_lane_num;
    hi_u32 i;

    switch (input_mode) {
        case INPUT_MODE_MIPI: {
            lane_num = MIPI_LANE_NUM;
            break;
        }
        case INPUT_MODE_SUBLVDS:
        case INPUT_MODE_LVDS:
        case INPUT_MODE_HISPI: {
            lane_num = LVDS_LANE_NUM;
            break;
        }
        case INPUT_MODE_SLVS: {
            lane_num = SLVS_LANE_NUM;
            break;
        }
        default: {
            mipi_rx_err_trace("input_mode(%u) is invalid, should be [0, %u)!\n",
                (hi_u32)input_mode, (hi_u32)INPUT_MODE_BUTT);
            return 0;
        }
    }

    total_lane_num = 0;

    for (i = 0U; i < lane_num; i++) {
        short tmp_lane_id;
        tmp_lane_id = p_lane_id[i];

        if (tmp_lane_id != -1) {
            lane_bitmap = lane_bitmap | ((hi_u32)1U << (unsigned short)(tmp_lane_id));
            total_lane_num++;
        }
    }

    *p_total_lane_num = total_lane_num;

    return lane_bitmap;
}

static void mipi_set_phy_config(combo_dev_t devno, input_mode_t input_mode, hi_u32 lane_bitmap)
{
    hi_u32 lane_mask;
    lane_divide_mode_t cur_lane_divide_mode;

    osal_spin_lock(&g_mipi_ctx_spinlock);
    cur_lane_divide_mode = g_mipi_dev_ctx.lane_divide_mode;
    osal_spin_unlock(&g_mipi_ctx_spinlock);

    lane_mask = mipi_rx_drv_get_lane_mask(devno, cur_lane_divide_mode);

    mipi_rx_drv_set_phy_config(input_mode, lane_bitmap, lane_mask);
}

static int mipi_check_comb_dev_attr(const combo_dev_attr_t *p_attr)
{
    if (p_attr->devno >= COMBO_DEV_MAX_NUM) {
        mipi_rx_err_trace("invalid combo_dev number(%u), should be less than %u.\n",
            p_attr->devno, COMBO_DEV_MAX_NUM);
        return -1;
    }

    if (((hi_s32)p_attr->input_mode < 0) || (p_attr->input_mode >= INPUT_MODE_BUTT)) {
        mipi_rx_err_trace("invalid input_mode(%d). must be in [%u, %u)\n",
            (hi_s32)p_attr->input_mode, (hi_u32)INPUT_MODE_MIPI, (hi_u32)INPUT_MODE_BUTT);
        return -1;
    }

    if (((hi_s32)p_attr->data_rate < 0) || (p_attr->data_rate > MIPI_DATA_RATE_X2)) {
        mipi_rx_err_trace("invalid data_rate(%d). must be in [%d, %d]\n",
            (hi_s32)p_attr->data_rate, (hi_s32)MIPI_DATA_RATE_X1, (hi_s32)MIPI_DATA_RATE_X2);
        return -1;
    }

    if (p_attr->img_rect.x < 0 || p_attr->img_rect.y < 0) {
        mipi_rx_err_trace("crop x and y (%d, %d) must be great than 0\n", p_attr->img_rect.x, p_attr->img_rect.y);
        return -1;
    }

    if (p_attr->img_rect.width < COMBO_MIN_WIDTH || p_attr->img_rect.height < COMBO_MIN_HEIGHT) {
        mipi_rx_err_trace("invalid img_size(%u, %u), can't be smaller than (%u, %u)\n",
            p_attr->img_rect.width, p_attr->img_rect.height, COMBO_MIN_WIDTH, COMBO_MIN_HEIGHT);
        return -1;
    }

    if (p_attr->img_rect.width % 2U != 0U || p_attr->img_rect.height % 2U != 0U) { /* 2: size must be even value */
        mipi_rx_err_trace("img_size(%u, %u) must be even value\n", p_attr->img_rect.width, p_attr->img_rect.height);
        return -1;
    }

    return 0;
}

static int check_lvds_wdr_mode(const lvds_dev_attr_t *p_attr)
{
    int ret = 0;

    switch (p_attr->wdr_mode) {
        case OT_LVDS_WDR_MODE_2F:
        case OT_LVDS_WDR_MODE_3F:
        case OT_LVDS_WDR_MODE_4F: {
            if (p_attr->vsync_attr.sync_type != LVDS_VSYNC_NORMAL && p_attr->vsync_attr.sync_type != LVDS_VSYNC_SHARE) {
                mipi_rx_err_trace("invalid sync_type, must be LVDS_VSYNC_NORMAL or LVDS_VSYNC_SHARE\n");
                ret = -1;
            }
            break;
        }

        case OT_LVDS_WDR_MODE_DOL_2F:
        case OT_LVDS_WDR_MODE_DOL_3F:
        case OT_LVDS_WDR_MODE_DOL_4F: {
            if (p_attr->vsync_attr.sync_type == LVDS_VSYNC_NORMAL) {
                if (p_attr->fid_attr.fid_type != LVDS_FID_IN_SAV && p_attr->fid_attr.fid_type != LVDS_FID_IN_DATA) {
                    mipi_rx_err_trace("invalid fid_type, must be LVDS_FID_IN_SAV or LVDS_FID_IN_DATA\n");
                    ret = -1;
                }
            } else if (p_attr->vsync_attr.sync_type == LVDS_VSYNC_HCONNECT) {
                if (p_attr->fid_attr.fid_type != LVDS_FID_NONE && p_attr->fid_attr.fid_type != LVDS_FID_IN_DATA) {
                    mipi_rx_err_trace("invalid fid_type, must be LVDS_FID_NONE or LVDS_FID_IN_DATA\n");
                    ret = -1;
                }
            } else {
                mipi_rx_err_trace("invalid sync_type, must be LVDS_VSYNC_NORMAL or LVDS_VSYNC_HCONNECT\n");
                ret = -1;
            }
            break;
        }

        default:
            break;
    }

    return ret;
}

static int check_lvds_dev_attr(combo_dev_t devno, const lvds_dev_attr_t *p_attr)
{
    int ret;

    if (((hi_s32)p_attr->input_data_type < 0)
        || (p_attr->input_data_type > DATA_TYPE_RAW_16BIT)) {
        mipi_rx_err_trace("invalid data_type(%d), must be in [%d, %d]\n",
            (hi_s32)p_attr->input_data_type, (hi_s32)DATA_TYPE_RAW_8BIT, (hi_s32)DATA_TYPE_RAW_16BIT);
        return -1;
    }

    if (((hi_s32)p_attr->wdr_mode < 0) || (p_attr->wdr_mode >= OT_LVDS_WDR_MODE_BUTT)) {
        mipi_rx_err_trace("invalid wdr_mode(%d), must be in [%u, %u)\n",
            (hi_s32)p_attr->wdr_mode, (hi_u32)OT_LVDS_WDR_MODE_NONE, (hi_u32)OT_LVDS_WDR_MODE_BUTT);
        return -1;
    }

    if (((hi_s32)p_attr->sync_mode < 0) || (p_attr->sync_mode >= LVDS_SYNC_MODE_BUTT)) {
        mipi_rx_err_trace("invalid sync_mode(%d), must be in [%d, %d)\n",
            (hi_s32)p_attr->sync_mode, (hi_s32)LVDS_SYNC_MODE_SOF, (hi_s32)LVDS_SYNC_MODE_BUTT);
        return -1;
    }

    if (((hi_s32)p_attr->vsync_attr.sync_type < 0)
        || (p_attr->vsync_attr.sync_type >= LVDS_VSYNC_BUTT)) {
        mipi_rx_err_trace("invalid vsync_code(%d), must be in [%u, %u)\n",
            (hi_s32)p_attr->vsync_attr.sync_type, (hi_u32)LVDS_VSYNC_NORMAL, (hi_u32)LVDS_VSYNC_BUTT);
        return -1;
    }

    if (((hi_s32)p_attr->fid_attr.fid_type < 0)
        || (p_attr->fid_attr.fid_type >= LVDS_FID_BUTT)) {
        mipi_rx_err_trace("invalid fid_type(%d), must be in [%u, %u)\n",
            (hi_s32)p_attr->fid_attr.fid_type, (hi_u32)LVDS_FID_NONE, (hi_u32)LVDS_FID_BUTT);
        return -1;
    }

    if (p_attr->fid_attr.output_fil != TRUE && p_attr->fid_attr.output_fil != FALSE) {
        mipi_rx_err_trace("invalid output_fil, must be OT_TURE or FALSE\n");
        return -1;
    }

    if (((hi_s32)p_attr->data_endian < 0)
        || (p_attr->data_endian >= LVDS_ENDIAN_BUTT)) {
        mipi_rx_err_trace("invalid lvds_bit_endian(%d), must be in [%u, %u)\n",
            (hi_s32)p_attr->data_endian, (hi_u32)LVDS_ENDIAN_LITTLE, (hi_u32)LVDS_ENDIAN_BUTT);
        return -1;
    }

    if (((hi_s32)p_attr->sync_code_endian < 0)
        || (p_attr->sync_code_endian >= LVDS_ENDIAN_BUTT)) {
        mipi_rx_err_trace("invalid lvds_bit_endian(%d), must be in [%u, %u)\n",
            (hi_s32)p_attr->sync_code_endian, (hi_u32)LVDS_ENDIAN_LITTLE, (hi_u32)LVDS_ENDIAN_BUTT);
        return -1;
    }

    ret = check_lvds_wdr_mode(p_attr);
    if (ret < 0) {
        mipi_rx_err_trace("check_lvds_wdr_mode failed!\n");
        return -1;
    }

    ret = check_lane_id(devno, INPUT_MODE_LVDS, p_attr->lane_id);
    if (ret < 0) {
        mipi_rx_err_trace("check_lane_id failed!\n");
        return -1;
    }

    return 0;
}

static void mipi_get_lvds_nxt_sync_code(hi_u32 lane_num, const lvds_dev_attr_t *attr,
    unsigned short sync_code[][WDR_VC_NUM][SYNC_CODE_NUM])
{
    hi_u32 i, j, k;
    hi_u32 shift_bits;

    if (attr->input_data_type == DATA_TYPE_RAW_10BIT) {
        shift_bits = 8; /* 8 bits one pixel component */
    } else {
        shift_bits = 10; /* 10 bits one pixel component */
    }
    /* SONY DOL N frame and N+1 Frame has the different sync_code
     * N+1 Frame FSET is 1, FSET is the 10th bit
     */
    for (i = 0; i < lane_num; i++) {
        for (j = 0; j < WDR_VC_NUM; j++) {
            for (k = 0; k < SYNC_CODE_NUM; k++) {
                sync_code[i][j][k] = attr->sync_code[i][j][k] + ((hi_u32)1U << shift_bits);
            }
        }
    }
}

static void mipi_set_lvds_sync_codes(combo_dev_t devno, const lvds_dev_attr_t *p_attr,
                                     hi_u32 total_lane_num, hi_u32 lane_bitmap)
{
    unsigned short(*nxt_sync_code)[WDR_VC_NUM][SYNC_CODE_NUM] = NULL;

    if (p_attr->wdr_mode >= OT_LVDS_WDR_MODE_DOL_2F && p_attr->wdr_mode <= OT_LVDS_WDR_MODE_DOL_4F) {
        /* Sony DOL Mode */
        nxt_sync_code = osal_vmalloc(sizeof(unsigned short) * total_lane_num *
            (hi_u32)WDR_VC_NUM * (hi_u32)SYNC_CODE_NUM);
        if (nxt_sync_code == NULL) {
            mipi_rx_err_trace("alloc memory for nxt_sync_code failed\n");
            return;
        }

        mipi_get_lvds_nxt_sync_code(total_lane_num, p_attr, nxt_sync_code);

        /* Set Dony DOL Line Information */
        if (p_attr->fid_attr.fid_type == LVDS_FID_IN_DATA) {
            mipi_rx_drv_set_dol_line_information(devno, p_attr->wdr_mode);
        }

        /* LVDS_CTRL Sync code */
        mipi_rx_drv_set_lvds_sync_code(devno, total_lane_num, p_attr->lane_id, p_attr->sync_code);
        mipi_rx_drv_set_lvds_nxt_sync_code(devno, total_lane_num, p_attr->lane_id, nxt_sync_code);
        /* PHY Sync code detect setting */
        mipi_rx_drv_set_phy_sync_config(p_attr, lane_bitmap, nxt_sync_code);

        osal_vfree(nxt_sync_code);
    } else {
        /* LVDS_CTRL Sync code */
        mipi_rx_drv_set_lvds_sync_code(devno, total_lane_num, p_attr->lane_id, p_attr->sync_code);
        mipi_rx_drv_set_lvds_nxt_sync_code(devno, total_lane_num, p_attr->lane_id, p_attr->sync_code);
        /* PHY Sync code detect setting */
        mipi_rx_drv_set_phy_sync_config(p_attr, lane_bitmap, p_attr->sync_code);
    }
}

static int mipi_set_lvds_dev_attr(const combo_dev_attr_t *p_combo_dev_attr)
{
    hi_u32 lane_bitmap, lane_num;
    lvds_err_int_cnt_t *lvds_err_int_cnt = NULL;
    align_err_int_cnt_t *align_err_int_cnt = NULL;
    combo_dev_t devno = p_combo_dev_attr->devno;
    const lvds_dev_attr_t *p_lvds_attr = &p_combo_dev_attr->lvds_attr;

    if (mipi_is_hs_mode_cfged() != TRUE) {
        mipi_rx_err_trace("mipi must set hs mode before set lvds attr!\n");
        return -1;
    }
    MIPI_CHECK_MIPI_DEV_RETURN(devno);
    if (mipi_is_dev_valid(devno) == 0U) {
        mipi_rx_err_trace("invalid combo dev num after set hs mode!\n");
        return -1;
    }

    int32_t ret = check_lvds_dev_attr(devno, p_lvds_attr);
    if (ret < 0) {
        mipi_rx_err_trace("check_lvds_dev_attr failed!\n");
        return -1;
    }

    lane_bitmap = mipi_get_lane_bitmap(INPUT_MODE_LVDS, p_lvds_attr->lane_id, &lane_num);
    mipi_rx_info_trace("MIPIRx Kernel Driver Input lane_bitmap: %#x\n", lane_bitmap);
    osal_spin_lock(&g_mipi_ctx_spinlock);
    g_mipi_dev_ctx.lane_bitmap[devno] = lane_bitmap;
    osal_spin_unlock(&g_mipi_ctx_spinlock);
    mipi_rx_drv_set_work_mode(devno, INPUT_MODE_LVDS);

    /* image crop */
    mipi_rx_drv_set_lvds_image_rect(devno, &p_combo_dev_attr->img_rect, lane_num);
    mipi_rx_drv_set_lvds_crop_en(devno, TRUE);

    /* data type & mode */
    ret = mipi_rx_drv_set_lvds_wdr_mode(devno, p_lvds_attr->wdr_mode, &p_lvds_attr->vsync_attr, &p_lvds_attr->fid_attr);
    if (ret < 0) {
        mipi_rx_err_trace("set lvds wdr mode failed!\n");
        return -1;
    }

    mipi_rx_drv_set_lvds_ctrl_mode(devno, p_lvds_attr->sync_mode, p_lvds_attr->input_data_type,
                                   p_lvds_attr->data_endian, p_lvds_attr->sync_code_endian);

    /* data rate */
    mipi_rx_drv_set_lvds_data_rate(devno, p_combo_dev_attr->data_rate);

    /* phy lane config */
    mipi_rx_drv_set_link_lane_id(devno, INPUT_MODE_LVDS, p_lvds_attr->lane_id);
    mipi_rx_drv_set_mem_cken(devno, TRUE);
    mipi_rx_drv_set_clr_cken(devno, TRUE);

    mipi_set_phy_config(devno, INPUT_MODE_LVDS, lane_bitmap);

    /* sync codes */
    mipi_set_lvds_sync_codes(devno, p_lvds_attr, lane_num, lane_bitmap);

    /* interrupt mask */
    lvds_err_int_cnt = mipi_rx_drv_get_lvds_err_int_cnt(devno);
    align_err_int_cnt = mipi_rx_drv_get_align_err_int_cnt(devno);
    (void)memset_s(lvds_err_int_cnt, sizeof(lvds_err_int_cnt_t), 0, sizeof(lvds_err_int_cnt_t));
    (void)memset_s(align_err_int_cnt, sizeof(align_err_int_cnt_t), 0, sizeof(align_err_int_cnt_t));

    mipi_rx_drv_set_chn_int_mask(devno);
    mipi_rx_drv_set_lvds_ctrl_int_mask(devno, LVDS_CTRL_INT_MASK);
    mipi_rx_drv_set_align_int_mask(devno, ALIGN0_INT_MASK);

    return 0;
}

static int check_mipi_dev_attr(combo_dev_t devno, const mipi_dev_attr_t *p_attr)
{
    int ret;
    int i;

    if (((hi_s32)p_attr->input_data_type < 0) || (p_attr->input_data_type >= DATA_TYPE_BUTT)) {
        mipi_rx_err_trace("invalid input_data_type(%d), must be in [%d, %d)\n",
            (hi_s32)p_attr->input_data_type, (hi_s32)DATA_TYPE_RAW_8BIT, (hi_s32)DATA_TYPE_BUTT);
        return -1;
    }

    if (((hi_s32)p_attr->wdr_mode < 0) || (p_attr->wdr_mode >= OT_MIPI_WDR_MODE_BUTT)) {
        mipi_rx_err_trace("invalid wdr_mode(%d), must be in [%u, %u)\n",
            (hi_s32)p_attr->wdr_mode, (hi_u32)OT_MIPI_WDR_MODE_NONE, (hi_u32)OT_MIPI_WDR_MODE_BUTT);
        return -1;
    }

    if ((p_attr->wdr_mode != OT_MIPI_WDR_MODE_NONE) && (p_attr->input_data_type >= DATA_TYPE_YUV420_8BIT_NORMAL)) {
        mipi_rx_err_trace("It do not support wdr mode when input_data_type is yuv format!\n");
        return -1;
    }

    if (p_attr->wdr_mode == OT_MIPI_WDR_MODE_DT) {
        for (i = 0; i < WDR_VC_NUM; i++) {
            /* data_type must be the CSI-2 reserve Type [0x38, 0x3f] */
            if (p_attr->data_type[i] < 0x38 || p_attr->data_type[i] > 0x3f) {
                mipi_rx_err_trace("invalid data_type[%d]: %d, must be in [0x38, 0x3f]\n", i, p_attr->data_type[i]);
                return -1;
            }
        }
    }

    ret = check_lane_id(devno, INPUT_MODE_MIPI, p_attr->lane_id);
    if (ret < 0) {
        mipi_rx_err_trace("check_lane_id failed!\n");
        return -1;
    }

    return 0;
}

static void mipi_set_dt_and_mode(combo_dev_t devno, const mipi_dev_attr_t *p_attr)
{
    data_type_t input_data_type;

    input_data_type = p_attr->input_data_type;

    mipi_rx_drv_set_mipi_yuv_dt(devno, input_data_type);

    if (p_attr->wdr_mode == OT_MIPI_WDR_MODE_DT) {
        mipi_rx_drv_set_mipi_wdr_user_dt(devno, input_data_type, p_attr->data_type);
    } else if (p_attr->wdr_mode == OT_MIPI_WDR_MODE_DOL) {
        mipi_rx_drv_set_mipi_dol_id(devno, input_data_type, NULL);
    } else {
        // fix sc
    }

    mipi_rx_drv_set_mipi_wdr_mode(devno, p_attr->wdr_mode);
}

static int mipi_set_mipi_dev_attr(const combo_dev_attr_t *p_combo_dev_attr)
{
    hi_u32 lane_bitmap, lane_num;
    int ret;
    combo_dev_t devno = p_combo_dev_attr->devno;
    mipi_err_int_cnt_t *mipi_err_int_cnt = NULL;
    align_err_int_cnt_t *align_err_int_cnt = NULL;
    const mipi_dev_attr_t *p_mipi_attr = &p_combo_dev_attr->mipi_attr;

    if (mipi_is_hs_mode_cfged() != TRUE) {
        mipi_rx_err_trace("mipi must set hs mode before set mipi attr!\n");
        return -1;
    }
    MIPI_CHECK_MIPI_DEV_RETURN(devno);
    if (mipi_is_dev_valid(devno) == 0U) {
        mipi_rx_err_trace("invalid combo dev num after set hs mode!\n");
        return -1;
    }

    ret = check_mipi_dev_attr(devno, p_mipi_attr);
    if (ret < 0) {
        mipi_rx_err_trace("check_mipi_dev_attr failed!\n");
        return -1;
    }

    // ISP Sub Ctrl CFG Domain
    mipi_rx_drv_set_work_mode(devno, INPUT_MODE_MIPI);

    /* image crop */
    mipi_rx_drv_set_mipi_image_rect(devno, &p_combo_dev_attr->img_rect);
    mipi_rx_drv_set_mipi_crop_en(devno, TRUE);

    /* data type & mode */
    mipi_set_dt_and_mode(devno, p_mipi_attr);

    /* data rate */
    mipi_rx_drv_set_data_rate(devno, p_combo_dev_attr->data_rate);

    /* phy lane config */
    mipi_rx_drv_set_link_lane_id(devno, INPUT_MODE_MIPI, p_mipi_attr->lane_id);

    lane_bitmap = mipi_get_lane_bitmap(INPUT_MODE_MIPI, p_mipi_attr->lane_id, &lane_num);
    mipi_rx_info_trace("MIPIRx Kernel Driver Input lane_bitmap: %#x\n", lane_bitmap);
    osal_spin_lock(&g_mipi_ctx_spinlock);
    g_mipi_dev_ctx.lane_bitmap[devno] = lane_bitmap;
    osal_spin_unlock(&g_mipi_ctx_spinlock);
    mipi_rx_drv_set_mem_cken(devno, TRUE);

    mipi_set_phy_config(devno, INPUT_MODE_MIPI, lane_bitmap);

    /* interrupt mask */
    mipi_err_int_cnt = mipi_rx_drv_get_mipi_err_int_cnt(devno);
    align_err_int_cnt = mipi_rx_drv_get_align_err_int_cnt(devno);
    (void)memset_s(mipi_err_int_cnt, sizeof(mipi_err_int_cnt_t), 0, sizeof(mipi_err_int_cnt_t));
    (void)memset_s(align_err_int_cnt, sizeof(align_err_int_cnt_t), 0, sizeof(align_err_int_cnt_t));

    mipi_rx_drv_set_chn_int_mask(devno);
    mipi_rx_drv_set_mipi_ctrl_int_mask(devno, MIPI_CTRL_INT_MASK);
    mipi_rx_drv_set_align_int_mask(devno, ALIGN0_INT_MASK);

    return 0;
}


static void slvs_set_crc(combo_dev_t devno, slvs_err_check_mode_t err_check_mode)
{
    hi_u32 enable;

    if (err_check_mode == SLVS_ERR_CHECK_MODE_CRC) {
        enable = 0x1;
    } else {
        enable = 0x0;
    }

    slvs_drv_set_crc_enable(devno, enable);
}

static hi_u32 mipi_get_bit_width_by_data_type(data_type_t data_type)
{
    hi_u32 data_width;

    switch (data_type) {
        case DATA_TYPE_RAW_8BIT:
            data_width = 8; /* 8:bit width */
            break;

        case DATA_TYPE_RAW_10BIT:
            data_width = 10; /* 10:bit width */
            break;

        case DATA_TYPE_RAW_12BIT:
            data_width = 12; /* 12:bit width */
            break;

        case DATA_TYPE_RAW_14BIT:
            data_width = 14; /* 14:bit width */
            break;

        case DATA_TYPE_RAW_16BIT:
            data_width = 16; /* 16:bit width */
            break;

        default:
            data_width = 0;
            break;
    }

    return data_width;
}
static void slvs_set_ecc(combo_dev_t devno, slvs_err_check_mode_t mode, data_type_t data_type, int payload_size)
{
    hi_u32 ecc_option;
    hi_u32 basic_block_size;
    hi_u32 extra_block_size;
    hi_u32 attach_extra_block_size;
    hi_u32 basic_block_num;
    hi_u32 data_bit_width;
    hi_u32 payload_size_byte;
    hi_u32 remainder;

    data_bit_width = mipi_get_bit_width_by_data_type(data_type);

    payload_size_byte = (hi_u32)payload_size * data_bit_width / 8U; /* 8:bit width */

    if (mode == SLVS_ERR_CHECK_MODE_ECC_2BYTE) {
        ecc_option = SLVS_ECC_OPT_MODE_2BYTE;
        basic_block_size = SLVS_ECC_BASIC_BLOCK_SIZE_2BYTE;
        attach_extra_block_size = SLVS_ECC_ATTACH_EXTRA_BLOCK_SIZE_2BYTE;
    } else if (mode == SLVS_ERR_CHECK_MODE_ECC_4BYTE) {
        ecc_option = SLVS_ECC_OPT_MODE_4BYTE;
        basic_block_size = SLVS_ECC_BASIC_BLOCK_SIZE_4BYTE;
        attach_extra_block_size = SLVS_ECC_ATTACH_EXTRA_BLOCK_SIZE_4BYTE;
    } else {
        ecc_option = SLVS_ECC_OPT_MODE_NONE;
        basic_block_size = SLVS_ECC_BASIC_BLOCK_SIZE_2BYTE;
        attach_extra_block_size = 0x0;
    }

    slvs_drv_set_ecc_option(devno, ecc_option);

    slvs_drv_set_ecc_basic_block_size(devno, basic_block_size);

    remainder = payload_size_byte % (hi_u32)SLVS_ECC_DATA_BLOCK_SIZE;

    if (remainder != 0) {
        extra_block_size = mipi_align_up(remainder, (hi_u32)SLVS_ECC_EXTRA_BLOCK_SIZE_ALIGN) + attach_extra_block_size;
    } else {
        extra_block_size = 0;
    }

    slvs_drv_set_ecc_extra_block_size(devno, extra_block_size);

    basic_block_num = payload_size_byte / (hi_u32)SLVS_ECC_DATA_BLOCK_SIZE;

    slvs_drv_set_ecc_basic_block_num(devno, basic_block_num);
}

static int check_slvs_dev_attr(combo_dev_t devno, const slvs_dev_attr_t* p_attr)
{
    int ret;

    MIPI_CHECK_SLVS_DEV_RETURN(devno);

    if ((p_attr->input_data_type < DATA_TYPE_RAW_8BIT) || (p_attr->input_data_type > DATA_TYPE_RAW_16BIT)) {
        mipi_rx_err_trace("invalid raw_data_type(%d), must be in [%d, %d)\n",
            (hi_s32)p_attr->input_data_type, (hi_s32)DATA_TYPE_RAW_8BIT, (hi_s32)DATA_TYPE_BUTT);
        return HI_FAILURE;
    }

    if ((int)p_attr->wdr_mode != (int)HI_WDR_MODE_NONE && (int)p_attr->wdr_mode != (int)HI_WDR_MODE_DOL_2F) {
        mipi_rx_err_trace("invalid wdr_mode, must be HI_WDR_MODE_NONE or HI_WDR_MODE_DOL_2F\n");
        return HI_FAILURE;
    }

    if ((p_attr->lane_rate != SLVS_LANE_RATE_LOW) && (p_attr->lane_rate != SLVS_LANE_RATE_HIGH)) {
        mipi_rx_err_trace("invalid lane_rate(%d), must be in [%d, %d]\n",
            (hi_s32)p_attr->lane_rate, (hi_s32)SLVS_LANE_RATE_LOW, (hi_s32)SLVS_LANE_RATE_HIGH);
        return HI_FAILURE;
    }

    if ((p_attr->err_check_mode < SLVS_ERR_CHECK_MODE_NONE) ||
        (p_attr->err_check_mode > SLVS_ERR_CHECK_MODE_ECC_4BYTE)) {
        mipi_rx_err_trace("invalid err_check_mode(%d), must be in [%u, %u]\n",
            (hi_s32)p_attr->err_check_mode, (hi_s32)SLVS_ERR_CHECK_MODE_NONE, (hi_s32)SLVS_ERR_CHECK_MODE_ECC_4BYTE);
        return HI_FAILURE;
    }
    // 芯片手册限制此值要写入的寄存器只有低14bit有效
    if ((p_attr->sensor_valid_width < 0) || (p_attr->sensor_valid_width > 16383)) { // 16383 即为 0x3fff
        mipi_rx_err_trace("invalid sensor_valid_width(%d), must be in [0, 16383]\n",
            p_attr->sensor_valid_width);
        return HI_FAILURE;
    }

    ret = check_lane_id(devno, INPUT_MODE_SLVS, p_attr->lane_id);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    return HI_SUCCESS;
}

static int mipi_set_slvs_dev_attr(const combo_dev_attr_t* p_dev_attr)
{
    hi_u32 lane_bitmap, lane_num;
    combo_dev_t devno;
    data_type_t input_dt;
    int sensor_valid_width;
    int ret;
    hi_u32 i;
    slvs_dev_attr_t* p_attr;

    devno               = p_dev_attr->devno;
    p_attr              = &p_dev_attr->slvs_attr;
    input_dt            = p_attr->input_data_type;
    sensor_valid_width  = p_dev_attr->slvs_attr.sensor_valid_width;

    ret = check_slvs_dev_attr(devno, p_attr);
    if (ret != HI_SUCCESS) {
        mipi_rx_err_trace("devno[%u], get_lane_info failed\n", devno);
        return ret;
    }

    /* clear last config */
    slvs_clear_config(devno,  p_dev_attr->slvs_attr.lane_id);

    lane_bitmap = mipi_get_lane_bitmap(INPUT_MODE_SLVS, p_attr->lane_id, &lane_num);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    // ISP Sub Ctrl CFG Domain
    mipi_rx_drv_set_work_mode(devno, INPUT_MODE_SLVS);

    slvs_drv_set_link_int_mask(devno, SLVS_LINK_INT_MASK);

    // lane number of this dev
    slvs_drv_set_lane_num(devno, lane_num);

    // enable slvs phy
    slvs_drv_set_phy_en(devno, 0x1);

    // slvs lane rate
    slvs_drv_set_lane_rate(devno, p_attr->lane_rate);

    // enable lane clock
    slvs_drv_set_lane_cken(devno, p_attr->lane_id, 0x1);

    // enable lane
    slvs_drv_set_lane_en(devno, p_attr->lane_id, 0x1);

    // enable EQ -- not need config

    // enabel bist-to do check it
    slvs_drv_set_bist_en(devno);

    osal_udelay(10); // 10 us

    // enable cdr
    slvs_drv_cdr_en(0, 0x1);

    // set phy ctrl test
    slvs_drv_phy_ctrl_test(devno, 0xF00040);

    // lane select this dev
    slvs_drv_set_lane_sel(devno, p_attr->lane_id);

    // raw data type
    slvs_drv_set_raw_type(devno, input_dt);

    // data rate
    slvs_drv_set_data_rate(devno, p_dev_attr->data_rate);

    // WDR
    slvs_drv_set_wdr_mode(devno, p_attr->wdr_mode);

    // deskew symbol
    slvs_drv_set_deskew_symbol(devno, 0x60);

    // enable clear
    slvs_drv_set_clear_en(devno, 0x1);

    // enable clear
    slvs_drv_set_mem_ck_en(devno, 0x1);

    // sensor clear
    slvs_drv_set_sensor_avalid_width(devno, sensor_valid_width);

    // crop info
    slvs_drv_set_image_rect(devno, &p_dev_attr->img_rect);
    slvs_drv_set_crop_en(devno, 0x1); // colorbar改为0x0

    slvs_set_crc(devno, p_attr->err_check_mode);
    slvs_set_ecc(devno, p_attr->err_check_mode, input_dt, sensor_valid_width);

    // lane order
    slvs_drv_set_link_lane_order(devno, p_attr->lane_id);

    // lane reset
    slvs_drv_lane_reset(devno, p_attr->lane_id);

    // lane unreset
    slvs_drv_lane_unreset(devno, p_attr->lane_id);

    /* interrupt mask */
    for (i = 0; i < SLVS_PHY_NUM; i++) {
        slvs_drv_phy_pcs_int_mask(i, SLVS_PHY_PCS_INT_MASK);
    }
    slvs_drv_set_slvs_mask(SLVS_EC_INT_MASK);

    return HI_SUCCESS;
}

static int mipi_set_combo_dev_attr(const combo_dev_attr_t *p_attr)
{
    int ret;

    ret = mipi_check_comb_dev_attr(p_attr);
    if (ret < 0) {
        mipi_rx_err_trace("mipi check combo_dev attr failed!\n");
        return -1;
    }

    if (osal_mutex_lock_interruptible(&g_mipi_mutex) != 0) {
        return -ERESTARTSYS;
    }

    switch (p_attr->input_mode) {
        case INPUT_MODE_LVDS:
        case INPUT_MODE_SUBLVDS:
        case INPUT_MODE_HISPI: {
            ret = mipi_set_lvds_dev_attr(p_attr);
            if (ret < 0) {
                mipi_rx_err_trace("mipi set lvds attr failed!\n");
                ret = -1;
            }
            break;
        }

        case INPUT_MODE_MIPI: {
            ret = mipi_set_mipi_dev_attr(p_attr);
            if (ret < 0) {
                mipi_rx_err_trace("mipi set mipi attr failed!\n");
                ret = -1;
            }
            break;
        }

        case INPUT_MODE_SLVS: {
            ret = mipi_set_slvs_dev_attr(p_attr);
            if (ret < 0) {
                mipi_rx_err_trace("mipi set mipi attr failed!\n");
                ret = -1;
            }
            break;
        }

        default: {
            mipi_rx_err_trace("invalid input mode\n");
            ret = -1;
            break;
        }
    }

    osal_mutex_unlock(&g_mipi_mutex);

    return ret;
}

/**
 * @brief: mipi 唤醒后combo_dev_attr设置
 * @param: 无
 * @return : 执行成功为0，未成功为非0
 */
static hi_s32 mipi_resume_combo_dev_attr(hi_void)
{
    hi_s32 ret = HI_SUCCESS;
    combo_dev_t combo_dev;
    for (combo_dev = 0; combo_dev < COMBO_DEV_MAX_NUM; ++combo_dev) {
        hi_u8 dev_cfged_flag;
        combo_dev_attr_t combo_dev_attr;
        osal_spin_lock(&g_mipi_ctx_spinlock);
        dev_cfged_flag = g_mipi_dev_ctx.dev_cfged[combo_dev];
        combo_dev_attr = g_mipi_dev_ctx.combo_dev_attr[combo_dev];
        osal_spin_unlock(&g_mipi_ctx_spinlock);
        if (dev_cfged_flag == TRUE) {
            ret = mipi_set_combo_dev_attr(&combo_dev_attr);
            if (ret < 0) {
                mipi_rx_err_trace("mipi set combo_dev:%d attr failed!\n", combo_dev);
            } else {
                mipi_rx_info_trace("mipi set combo_dev:%d attr success!\n", combo_dev);
            }
        }
    }
    return ret;
}

static int mipi_reset_sensor(sns_rst_source_t sns_reset_source)
{
    if (sns_reset_source >= SNS_MAX_RST_SOURCE_NUM) {
        mipi_rx_err_trace("invalid sns_reset_source(%u), should be less than %d.\n",
            sns_reset_source, SNS_MAX_RST_SOURCE_NUM);
        return -1;
    }

    HI_S32 ret = HI_SUCCESS;
    ret += sensor_drv_reset(sns_reset_source);
    if (ret != HI_SUCCESS) {
        mipi_rx_err_trace("sns_reset_source(%u) config failed.\n", sns_reset_source);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static int mipi_unreset_sensor(sns_rst_source_t sns_reset_source)
{
    if (sns_reset_source >= SNS_MAX_RST_SOURCE_NUM) {
        mipi_rx_err_trace("invalid sns_reset_source(%u), should be less than %d.\n",
            sns_reset_source, SNS_MAX_RST_SOURCE_NUM);
        return -1;
    }

    HI_S32 ret = HI_SUCCESS;
    ret += sensor_drv_unreset(sns_reset_source);
    if (ret != HI_SUCCESS) {
        mipi_rx_err_trace("mipi_unreset_sensor(%u) config failed.\n", sns_reset_source);
        return HI_FAILURE;
    }

    return ret;
}

/**
 * @brief: slvs-ec复位
 * @param: combo_dev 号
 * @return : 复位成功为0，复位失败为非0
 */
static int mipi_reset_slvs_ec(combo_dev_t combo_dev)
{
    MIPI_CHECK_SLVS_DEV_RETURN(combo_dev);
    slvs_drv_core_reset(combo_dev);

    return 0;
}

static int mipi_reset_mipi_rx(combo_dev_t combo_dev)
{
    if (mipi_is_hs_mode_cfged() != TRUE) {
        mipi_rx_err_trace("mipi must set hs mode before reset mipi!\n");
        return -1;
    }
    MIPI_CHECK_MIPI_DEV_RETURN(combo_dev);
    if (mipi_is_dev_valid(combo_dev) == 0U) {
        mipi_rx_err_trace("invalid combo dev num after set hs mode!\n");
        return -1;
    }

    mipi_rx_drv_core_reset(combo_dev);

    return 0;
}

/**
 * @brief: slvs-ec解复位
 * @param: combo_dev 号
 * @return : 解复位成功为0，解复位失败为非0
 */
static int mipi_unreset_slvs_ec(combo_dev_t combo_dev)
{
    MIPI_CHECK_SLVS_DEV_RETURN(combo_dev);
    slvs_drv_core_unreset(combo_dev);

    return 0;
}

static int mipi_unreset_mipi_rx(combo_dev_t combo_dev)
{
    if (mipi_is_hs_mode_cfged() != TRUE) {
        mipi_rx_err_trace("mipi must set hs mode before unreset mipi!\n");
        return -1;
    }
    MIPI_CHECK_MIPI_DEV_RETURN(combo_dev);
    if (mipi_is_dev_valid(combo_dev) == 0U) {
        mipi_rx_err_trace("invalid combo dev num after set hs mode!\n");
        return -1;
    }
    mipi_rx_drv_core_unreset(combo_dev);

    return 0;
}

/* magic numbers represents mipi_dev id numbers */
static void mipi_set_dev_valid(lane_divide_mode_t mode)
{
    osal_spin_lock(&g_mipi_ctx_spinlock);
    switch (mode) {
        case LANE_DIVIDE_MODE_0:
            g_mipi_dev_ctx.dev_valid[0] = 1; // PHY 8lane --> 0th channel
            break;
        case LANE_DIVIDE_MODE_1:
            g_mipi_dev_ctx.dev_valid[0] = 1; // PHY 4lane + 4lane --> 0th & 2ed channel
            g_mipi_dev_ctx.dev_valid[2] = 1; // PHY 4lane + 4lane --> 0th & 2ed channel
            break;
        case LANE_DIVIDE_MODE_2:
            g_mipi_dev_ctx.dev_valid[0] = 1; // PHY 4lane + 2lane + 2ane --> 0th & 2ed & 3rd channel
            g_mipi_dev_ctx.dev_valid[2] = 1; // PHY 4lane + 2lane + 2ane --> 0th & 2ed & 3rd channel
            g_mipi_dev_ctx.dev_valid[3] = 1; // PHY 4lane + 2lane + 2ane --> 0th & 2ed & 3rd channel
            break;
        case LANE_DIVIDE_MODE_3:
            g_mipi_dev_ctx.dev_valid[0] = 1; // PHY 2lane + 2lane + 2ane + 2lane --> 0th & 1st & 2ed & 3rd channel
            g_mipi_dev_ctx.dev_valid[1] = 1; // PHY 2lane + 2lane + 2ane + 2lane --> 0th & 1st & 2ed & 3rd channel
            g_mipi_dev_ctx.dev_valid[2] = 1; // PHY 2lane + 2lane + 2ane + 2lane --> 0th & 1st & 2ed & 3rd channel
            g_mipi_dev_ctx.dev_valid[3] = 1; // PHY 2lane + 2lane + 2ane + 2lane --> 0th & 1st & 2ed & 3rd channel
            break;

        default:
            break;
    }
    osal_spin_unlock(&g_mipi_ctx_spinlock);
}

static int mipi_set_hs_mode(lane_divide_mode_t lane_divide_mode)
{
    if (((hi_s32)lane_divide_mode < 0) || (lane_divide_mode >= LANE_DIVIDE_MODE_BUTT)) {
        mipi_rx_err_trace("invalid lane_divide_mode(%d), must be in [%d, %d)\n",
            (hi_s32)lane_divide_mode, (hi_s32)LANE_DIVIDE_MODE_0, (hi_s32)LANE_DIVIDE_MODE_BUTT);
        return -1;
    }

    mipi_rx_info_trace("mipi_set_hs_mode lane_divide_mode: %u\n", (hi_u32)lane_divide_mode);
    /* 在mipi启动之前，先执行一遍退出流程，把所有的mipi的时钟和sensor的时钟关闭及复位
        文档中有约束，set hs mode 是mipi启动流程的第一步                       */
    hi_s32 ret;
    mipi_rx_info_trace("reset all mipi before set hs mode \n");
    ret = mipi_do_exit();
    if (ret != HI_SUCCESS) {
        mipi_rx_err_trace("mipi reset failed before set hs mode\n");
        return ret;
    }

    mipi_rx_drv_set_hs_mode(lane_divide_mode);

    return 0;
}

static hi_s32 mipi_resume_hs_mode(void)
{
    hi_s32 ret = 0;
    osal_spin_lock(&g_mipi_ctx_spinlock);
    unsigned char hs_mode_cfged_flag = g_mipi_dev_ctx.hs_mode_cfged;
    osal_spin_unlock(&g_mipi_ctx_spinlock);
    if (hs_mode_cfged_flag == TRUE) {
        lane_divide_mode_t lane_divide_mode;
        osal_spin_lock(&g_mipi_ctx_spinlock);
        lane_divide_mode = g_mipi_dev_ctx.lane_divide_mode;
        osal_spin_unlock(&g_mipi_ctx_spinlock);
        if (((hi_s32)lane_divide_mode < 0) || (lane_divide_mode >= LANE_DIVIDE_MODE_BUTT)) {
            mipi_rx_err_trace("invalid lane_divide_mode(%d), must be in [%d, %d)\n",
                lane_divide_mode, (hi_s32)LANE_DIVIDE_MODE_0, (hi_s32)LANE_DIVIDE_MODE_BUTT);
            return -1;
        }
        mipi_rx_drv_set_hs_mode(lane_divide_mode);
    }
    return ret;
}

static int mipi_enable_mipi_rx_clock(combo_dev_t combo_dev)
{
    hi_s32 ret = HI_SUCCESS;
    if (mipi_is_hs_mode_cfged() != TRUE) {
        mipi_rx_err_trace("mipi must set hs mode before set mipi clock!\n");
        return -1;
    }
    MIPI_CHECK_MIPI_DEV_RETURN(combo_dev);
    if (mipi_is_dev_valid(combo_dev) == 0U) {
        mipi_rx_err_trace("invalid combo dev num after set hs mode!\n");
        return -1;
    }

    lane_divide_mode_t cur_lane_divide_mode;
    osal_spin_lock(&g_mipi_ctx_spinlock);
    cur_lane_divide_mode = g_mipi_dev_ctx.lane_divide_mode;
    osal_spin_unlock(&g_mipi_ctx_spinlock);

    ret += mipi_rx_drv_enable_clock(combo_dev, cur_lane_divide_mode);

    return ret;
}

/**
 * @brief: slvs-ec使能时钟
 * @param: combo_dev 号
 * @return : 使能成功为0，使能失败为非0
 */
static int mipi_enable_slvs_ec_clock(combo_dev_t combo_dev)
{
    hi_s32 ret = HI_SUCCESS;

    MIPI_CHECK_SLVS_DEV_RETURN(combo_dev);
    ret += slvs_drv_enable_clock(combo_dev);

    return ret;
}

static int mipi_disable_mipi_rx_clock(combo_dev_t combo_dev)
{
    int ret;
    if (mipi_is_hs_mode_cfged() != TRUE) {
        mipi_rx_err_trace("mipi must set hs mode before set mipi clock!\n");
        return -1;
    }

    MIPI_CHECK_MIPI_DEV_RETURN(combo_dev);

    if (mipi_is_dev_valid(combo_dev) == 0U) {
        mipi_rx_err_trace("invalid combo dev num after set hs mode!\n");
        return -1;
    }

    lane_divide_mode_t cur_lane_divide_mode;
    osal_spin_lock(&g_mipi_ctx_spinlock);
    cur_lane_divide_mode = g_mipi_dev_ctx.lane_divide_mode;
    osal_spin_unlock(&g_mipi_ctx_spinlock);

    ret = mipi_rx_drv_disable_clock(combo_dev, cur_lane_divide_mode);

    return ret;
}

/**
 * @brief: slvs-ec关闭时钟
 * @param: combo_dev 号
 * @return : 关闭成功为0，关闭失败为非0
 */
static int mipi_disable_slvs_ec_clock(combo_dev_t combo_dev)
{
    hi_s32 ret = HI_SUCCESS;
    MIPI_CHECK_SLVS_DEV_RETURN(combo_dev);
    ret = slvs_drv_disable_clock(combo_dev);

    return ret;
}

static int mipi_enable_sensor_clock(sns_clk_source_t sns_clk_source)
{
    int ret;
    if (sns_clk_source >= SNS_MAX_CLK_SOURCE_NUM) {
        mipi_rx_err_trace("invalid sns_clk_source(%u), should be less than %d.\n",
            sns_clk_source, SNS_MAX_CLK_SOURCE_NUM);
        return -1;
    }

    ret = sensor_drv_enable_clock(sns_clk_source);

    return ret;
}

static int mipi_config_sensor_clock(sns_clk_cfg_t sensor_clk_cfg)
{
    if ((sensor_clk_cfg.clk_source >= SNS_MAX_CLK_SOURCE_NUM)
        || ((hi_s32)sensor_clk_cfg.clk_freq < 0)
        || (sensor_clk_cfg.clk_freq >= SENSOR_CLK_FREQ_BUTT)) {
        mipi_rx_err_trace("invalid sensor clock config. clk_source(%u). clk_freq(%d)\n",
            sensor_clk_cfg.clk_source, (hi_s32)sensor_clk_cfg.clk_freq);
        mipi_rx_err_trace("clk_source should be less than %d, clk_freq should be [0, %u).\n",
            SNS_MAX_CLK_SOURCE_NUM, SENSOR_CLK_FREQ_BUTT);
        return -1;
    }

    int ret = sensor_drv_config_clock(sensor_clk_cfg);

    return ret;
}

static int mipi_disable_sensor_clock(sns_clk_source_t sns_clk_source)
{
    int ret;
    if (sns_clk_source >= SNS_MAX_CLK_SOURCE_NUM) {
        mipi_rx_err_trace("invalid sns_clk_source(%u), should be less than %d.\n",
            sns_clk_source, SNS_MAX_CLK_SOURCE_NUM);
        return -1;
    }

    ret = sensor_drv_disable_clock(sns_clk_source);

    return ret;
}

long mipi_rx_ioctl(unsigned int cmd, unsigned long arg, const void *private_data)
{
    (hi_void)osal_atomic_inc_return(&g_mipi_rx_user_ref);

    hi_u32 *argp = (hi_u32 *)(uintptr_t)arg;
    int ret;

    if (argp == NULL) {
        mipi_rx_err_trace("mipi ioctl parameter is NULL pointer!\n");
        (hi_void)osal_atomic_dec_return(&g_mipi_rx_user_ref);
        return HI_FAILURE;
    }

    mipi_rx_unused(private_data);

    switch (cmd) {
        case HI_MIPI_SET_DEV_ATTR: {
            combo_dev_attr_t *pstcombo_dev_attr = NULL;
            combo_dev_t devno;
            pstcombo_dev_attr = (combo_dev_attr_t *)argp;
            mipi_rx_notice_trace("combodev: %u ,set dev attr\n", pstcombo_dev_attr->devno);
            ret = mipi_set_combo_dev_attr(pstcombo_dev_attr);
            if (ret < 0) {
                mipi_rx_err_trace("mipi set combo_dev attr failed!\n");
                ret = -1;
            } else {
                devno = pstcombo_dev_attr->devno;
                osal_spin_lock(&g_mipi_ctx_spinlock);
                g_mipi_dev_ctx.dev_cfged[devno] = TRUE;
                (void)memcpy_s(&g_mipi_dev_ctx.combo_dev_attr[devno], sizeof(combo_dev_attr_t),
                               pstcombo_dev_attr, sizeof(combo_dev_attr_t));
                osal_spin_unlock(&g_mipi_ctx_spinlock);
            }
            break;
        }

        case HI_MIPI_RESET_SENSOR: {
            sns_rst_source_t sns_reset_source = *(sns_rst_source_t *)argp;
            mipi_rx_notice_trace("sns_reset_source: %u ,reset sensor\n", sns_reset_source);
            ret = mipi_reset_sensor(sns_reset_source);
            if (ret < 0) {
                mipi_rx_err_trace("mipi reset sensor failed!\n");
                ret = -1;
            } else {
                osal_spin_lock(&g_mipi_ctx_spinlock);
                g_mipi_dev_ctx.sns_rst_source_cfged[sns_reset_source] = RESET;
                osal_spin_unlock(&g_mipi_ctx_spinlock);
            }
            break;
        }

        case HI_MIPI_UNRESET_SENSOR: {
            sns_rst_source_t sns_reset_source = *(sns_rst_source_t *)argp;
            mipi_rx_notice_trace("sns_reset_source: %u ,unreset sensor\n", sns_reset_source);
            ret = mipi_unreset_sensor(sns_reset_source);
            if (ret < 0) {
                mipi_rx_err_trace("mipi unreset sensor failed!\n");
                ret = -1;
            } else {
                osal_spin_lock(&g_mipi_ctx_spinlock);
                g_mipi_dev_ctx.sns_rst_source_cfged[sns_reset_source] = UNRESET;
                osal_spin_unlock(&g_mipi_ctx_spinlock);
            }
            break;
        }
        case HI_MIPI_RESET_SLVS: {
            combo_dev_t combo_dev = *(combo_dev_t *)argp;
            mipi_rx_notice_trace("combo_dev: %u ,mipi_reset_slvs_ec\n", combo_dev);
            ret = mipi_reset_slvs_ec(combo_dev);
            if (ret < 0) {
                mipi_rx_err_trace("mipi reset slvs-ec failed!\n");
                ret = -1;
            } else {
                osal_spin_lock(&g_mipi_ctx_spinlock);
                g_mipi_dev_ctx.reset_slvs_status[combo_dev] = RESET;
                g_mipi_dev_ctx.dev_cfged[combo_dev] = FALSE;
                osal_spin_unlock(&g_mipi_ctx_spinlock);
            }
            break;
        }

        case HI_MIPI_RESET_MIPI: {
            combo_dev_t combo_dev = *(combo_dev_t *)argp;
            mipi_rx_notice_trace("combo_dev: %u ,mipi_reset_mipi_rx\n", combo_dev);
            ret = mipi_reset_mipi_rx(combo_dev);
            if (ret < 0) {
                mipi_rx_err_trace("mipi reset mipi_rx failed!\n");
                ret = -1;
            } else {
                osal_spin_lock(&g_mipi_ctx_spinlock);
                g_mipi_dev_ctx.reset_mipi_status[combo_dev] = RESET;
                g_mipi_dev_ctx.dev_cfged[combo_dev] = FALSE;
                osal_spin_unlock(&g_mipi_ctx_spinlock);
            }
            break;
        }
        case HI_MIPI_UNRESET_SLVS: {
            combo_dev_t combo_dev = *(combo_dev_t *)argp;
            mipi_rx_notice_trace("combo_dev: %u ,mipi_unreset_slvs_ec\n", combo_dev);
            ret = mipi_unreset_slvs_ec(combo_dev);
            if (ret < 0) {
                mipi_rx_err_trace("mipi unreset slvs-ec failed!\n");
                ret = -1;
            } else {
                osal_spin_lock(&g_mipi_ctx_spinlock);
                g_mipi_dev_ctx.reset_slvs_status[combo_dev] = UNRESET;
                osal_spin_unlock(&g_mipi_ctx_spinlock);
            }
            break;
        }
        case HI_MIPI_UNRESET_MIPI: {
            combo_dev_t combo_dev = *(combo_dev_t *)argp;
            mipi_rx_notice_trace("combo_dev: %u ,mipi_unreset_mipi_rx\n", combo_dev);
            ret = mipi_unreset_mipi_rx(combo_dev);
            if (ret < 0) {
                mipi_rx_err_trace("mipi unreset mipi_rx failed!\n");
                ret = -1;
            } else {
                osal_spin_lock(&g_mipi_ctx_spinlock);
                g_mipi_dev_ctx.reset_mipi_status[combo_dev] = UNRESET;
                osal_spin_unlock(&g_mipi_ctx_spinlock);
            }
            break;
        }
        case HI_MIPI_SET_HS_MODE: {
            lane_divide_mode_t lane_divide_mode = *(lane_divide_mode_t *)argp;
            mipi_rx_notice_trace("lane_divide_mode: %d ,mipi_set_hs_mode\n", (hi_s32)lane_divide_mode);
            ret = mipi_set_hs_mode(lane_divide_mode);
            if (ret < 0) {
                mipi_rx_err_trace("mipi set hs mode failed!\n");
                ret = -1;
            } else {
                hi_u32 i;
                osal_spin_lock(&g_mipi_ctx_spinlock);
                g_mipi_dev_ctx.lane_divide_mode = lane_divide_mode;
                g_mipi_dev_ctx.hs_mode_cfged = TRUE;
                (void)memset_s(g_mipi_dev_ctx.dev_valid, sizeof(g_mipi_dev_ctx.dev_valid),
                    0, sizeof(g_mipi_dev_ctx.dev_valid));
                for (i = 0; i < MIPI_RX_MAX_DEV_NUM; i++) {
                    g_mipi_dev_ctx.dev_cfged[i] = FALSE;
                    g_mipi_dev_ctx.mipi_clock_status[i] = DISABLE;
                    g_mipi_dev_ctx.reset_mipi_status[i] = RESET;
                }
                for (i = 0; i < SNS_MAX_CLK_SOURCE_NUM; i++) {
                    g_mipi_dev_ctx.sns_clk_config_cfged[i] = FALSE;
                    g_mipi_dev_ctx.sns_clk_source_cfged[i] = DISABLE;
                }
                for (i = 0; i < SNS_MAX_RST_SOURCE_NUM; i++) {
                    g_mipi_dev_ctx.sns_rst_source_cfged[i] = RESET;
                }
                osal_spin_unlock(&g_mipi_ctx_spinlock);
                mipi_set_dev_valid(lane_divide_mode);
            }
            break;
        }
        case HI_MIPI_ENABLE_SLVS_CLOCK: {
            combo_dev_t combo_dev = *(combo_dev_t *)argp;
            mipi_rx_notice_trace("combo_dev: %u ,mipi_enable_slvs_ec_clock\n", combo_dev);
            ret = mipi_enable_slvs_ec_clock(combo_dev);
            if (ret < 0) {
                mipi_rx_err_trace("mipi enable slvs_ec clock failed!\n");
                ret = -1;
            } else {
                osal_spin_lock(&g_mipi_ctx_spinlock);
                g_mipi_dev_ctx.slvs_clock_status[combo_dev] = ENABLE;
                osal_spin_unlock(&g_mipi_ctx_spinlock);
            }
            break;
        }
        case HI_MIPI_ENABLE_MIPI_CLOCK: {
            combo_dev_t combo_dev = *(combo_dev_t *)argp;
            mipi_rx_notice_trace("combo_dev: %u ,mipi_enable_mipi_rx_clock\n", combo_dev);
            ret = mipi_enable_mipi_rx_clock(combo_dev);
            if (ret < 0) {
                mipi_rx_err_trace("mipi enable mipi_rx clock failed!\n");
                ret = -1;
            } else {
                osal_spin_lock(&g_mipi_ctx_spinlock);
                g_mipi_dev_ctx.mipi_clock_status[combo_dev] = ENABLE;
                osal_spin_unlock(&g_mipi_ctx_spinlock);
            }
            break;
        }
        case HI_MIPI_DISABLE_SLVS_CLOCK: {
            combo_dev_t combo_dev = *(combo_dev_t *)argp;
            mipi_rx_notice_trace("combo_dev: %u ,mipi_disable_slvs_ec_clock\n", combo_dev);
            ret = mipi_disable_slvs_ec_clock(combo_dev);
            if (ret < 0) {
                mipi_rx_err_trace("mipi disable slvs-ec clock failed!\n");
                ret = -1;
            } else {
                osal_spin_lock(&g_mipi_ctx_spinlock);
                g_mipi_dev_ctx.slvs_clock_status[combo_dev] = DISABLE;
                osal_spin_unlock(&g_mipi_ctx_spinlock);
            }
            break;
        }
        case HI_MIPI_DISABLE_MIPI_CLOCK: {
            combo_dev_t combo_dev = *(combo_dev_t *)argp;
            mipi_rx_notice_trace("combo_dev: %u ,mipi_disable_mipi_rx_clock\n", combo_dev);
            ret = mipi_disable_mipi_rx_clock(combo_dev);
            if (ret < 0) {
                mipi_rx_err_trace("mipi disable mipi_rx clock failed!\n");
                ret = -1;
            } else {
                osal_spin_lock(&g_mipi_ctx_spinlock);
                g_mipi_dev_ctx.mipi_clock_status[combo_dev] = DISABLE;
                osal_spin_unlock(&g_mipi_ctx_spinlock);
            }
            break;
        }

        case HI_MIPI_ENABLE_SENSOR_CLOCK: {
            sns_clk_source_t sns_clk_source = *(sns_clk_source_t *)argp;
            mipi_rx_notice_trace("sns_clk_source: %u ,mipi_enable_sensor_clock\n", sns_clk_source);
            ret = mipi_enable_sensor_clock(sns_clk_source);
            if (ret < 0) {
                mipi_rx_err_trace("mipi enable sensor clock failed!\n");
                ret = -1;
            } else {
                osal_spin_lock(&g_mipi_ctx_spinlock);
                g_mipi_dev_ctx.sns_clk_source_cfged[sns_clk_source] = ENABLE;
                osal_spin_unlock(&g_mipi_ctx_spinlock);
            }
            break;
        }
        case HI_MIPI_DISABLE_SENSOR_CLOCK: {
            sns_clk_source_t sns_clk_source = *(sns_clk_source_t *)argp;
            mipi_rx_notice_trace("sns_clk_source: %u ,mipi_disable_sensor_clock\n", sns_clk_source);
            ret = mipi_disable_sensor_clock(sns_clk_source);
            if (ret < 0) {
                mipi_rx_err_trace("mipi disable sensor clock failed!\n");
                ret = -1;
            } else {
                osal_spin_lock(&g_mipi_ctx_spinlock);
                g_mipi_dev_ctx.sns_clk_source_cfged[sns_clk_source] = DISABLE;
                osal_spin_unlock(&g_mipi_ctx_spinlock);
            }
            break;
        }
        case HI_MIPI_CONFIG_SENSOR_CLOCK: {
            sns_clk_cfg_t sns_clk_config = *(sns_clk_cfg_t *)argp;
            mipi_rx_notice_trace("ioctl_set sns_clk_source: %u ,mipi_config_sensor_clock\n", sns_clk_config.clk_source);
            ret = mipi_config_sensor_clock(sns_clk_config);
            if (ret < 0) {
                mipi_rx_err_trace("mipi config sensor clock failed!\n");
                ret = -1;
            } else {
                osal_spin_lock(&g_mipi_ctx_spinlock);
                g_mipi_dev_ctx.sns_clk_config_cfged[sns_clk_config.clk_source] = TRUE;
                g_mipi_dev_ctx.clk_freq[sns_clk_config.clk_source] = sns_clk_config.clk_freq;
                osal_spin_unlock(&g_mipi_ctx_spinlock);
            }
            break;
        }

        default: {
            ret = -1;
            break;
        }
    }
    (hi_void)osal_atomic_dec_return(&g_mipi_rx_user_ref);
    return ret;
}

/**
 * @brief: mipi ioctl调用计数变量g_mipi_rx_user_ref初始化
 * @param: 无
 * @return : 初始化成功为0，初始化失败为非0
 */
hi_s32 init_mipi_rx_user_ref(void)
{
    hi_s32 ret = HI_SUCCESS;
    ret = osal_atomic_init(&g_mipi_rx_user_ref);
    if (ret != HI_SUCCESS) {
        mipi_rx_err_trace("osal_atomic_init failed !\n");
        return ret;
    }

    osal_atomic_set(&g_mipi_rx_user_ref, 0);
    return ret;
}

/**
 * @brief: mipi ioctl调用计数变量g_mipi_rx_user_ref销毁
 * @param: 无
 * @return : 无
 */
void destory_mipi_rx_user_ref(void)
{
    osal_atomic_destroy(&g_mipi_rx_user_ref);
    return ;
}

/**
 * @brief: mipi ioctl调用计数清零判断
 * @param: 无
 * @return : 以清零为0，未清零为负
 */
static hi_s32 mipi_rx_check_ioctl_ready(hi_void)
{
    hi_s32 ret = HI_SUCCESS;
    hi_s32 sleep_cnt = 0;
    hi_s32 mipi_rx_users = 0;

    do {
        if (mipi_rx_users != 0) {
            mipi_rx_info_trace("mipi_rx_ioctl_count: %d , wait\n", mipi_rx_users);
            osal_udelay(100); // 每次等100us
            sleep_cnt++;
        }
        mipi_rx_users = osal_atomic_read(&g_mipi_rx_user_ref);
    } while ((mipi_rx_users != 0) && (sleep_cnt < 20)); // 阈值最多等待20*100us= 2ms

    if (mipi_rx_users != 0) {
        ret = HI_FAILURE;
        mipi_rx_err_trace("mipi rx ioctl not ready. users=%d\n", mipi_rx_users);
    } else {
        ret = HI_SUCCESS;
    }
    return ret;
}

/**
 * @brief: mipi 休眠过程中reset_sensor函数
 * @param: 无
 * @return : 成功为0，不成功则为负
 */
static hi_s32 suspend_mipi_reset_sensor(hi_void)
{
    hi_s32 ret = HI_SUCCESS;
    sns_rst_source_t sns_reset_source = 0;
    for (sns_reset_source = 0; sns_reset_source < SNS_MAX_RST_SOURCE_NUM; ++sns_reset_source) {
        osal_spin_lock(&g_mipi_ctx_spinlock);
        reset_status_e sns_rst_source_cfged_flag = g_mipi_dev_ctx.sns_rst_source_cfged[sns_reset_source];
        osal_spin_unlock(&g_mipi_ctx_spinlock);
        if (sns_rst_source_cfged_flag == UNRESET) {
            ret = mipi_reset_sensor(sns_reset_source);
            if (ret != HI_SUCCESS) {
                mipi_rx_err_trace("mipi reset sns_reset_source:%u  failed!\n", sns_reset_source);
            } else {
                mipi_rx_info_trace("mipi reset sns_reset_source:%u  success!\n", sns_reset_source);
            }
        }
    }
    return ret;
}

/**
 * @brief: mipi 休眠过程中disable_sensor_clock函数
 * @param: 无
 * @return : 成功为0，不成功则为负
 */
static hi_s32 suspend_mipi_disable_sensor_clock(hi_void)
{
    hi_s32 ret = HI_SUCCESS;
    sns_clk_source_t sns_clk_source = 0;
    for (sns_clk_source = 0; sns_clk_source < SNS_MAX_CLK_SOURCE_NUM; ++sns_clk_source) {
        osal_spin_lock(&g_mipi_ctx_spinlock);
        clock_status_e sns_clk_source_cfged_flag = g_mipi_dev_ctx.sns_clk_source_cfged[sns_clk_source];
        osal_spin_unlock(&g_mipi_ctx_spinlock);
        if (sns_clk_source_cfged_flag == ENABLE) {
            ret = mipi_disable_sensor_clock(sns_clk_source);
            if (ret != HI_SUCCESS) {
                mipi_rx_err_trace("mipi disable sns_clk_source:%u failed!\n", sns_clk_source);
            } else {
                mipi_rx_info_trace("mipi disable sns_clk_source:%u success!\n", sns_clk_source);
            }
        }
    }
    return ret;
}

/**
 * @brief: mipi 休眠过程中reset_mipi_rx函数
 * @param: 无
 * @return : 成功为0，不成功则为负
 */
static hi_s32 suspend_mipi_reset_mipi_rx(hi_void)
{
    hi_s32 ret = HI_SUCCESS;
    combo_dev_t combo_dev = 0;
    // 判断hs mode是否有设置，没有则直接成功退出
    osal_spin_lock(&g_mipi_ctx_spinlock);
    unsigned char hs_mode_cfged_flag = g_mipi_dev_ctx.hs_mode_cfged;
    osal_spin_unlock(&g_mipi_ctx_spinlock);
    if (hs_mode_cfged_flag != TRUE) {
        return HI_SUCCESS;
    }

    for (combo_dev = 0; combo_dev < COMBO_DEV_MAX_NUM; ++combo_dev) {
        // 判断在对应的hs mode下，combo_dev是否有效，无效则跳过并继续
        if (mipi_is_dev_valid(combo_dev) == 0U) {
            continue;
        }
        osal_spin_lock(&g_mipi_ctx_spinlock);
        reset_status_e reset_mipi_status_flag = g_mipi_dev_ctx.reset_mipi_status[combo_dev];
        osal_spin_unlock(&g_mipi_ctx_spinlock);
        if (reset_mipi_status_flag == UNRESET) {
            ret += mipi_reset_mipi_rx(combo_dev);
            if (ret != HI_SUCCESS) {
                mipi_rx_err_trace("mipi reset mipi_rx combo_dev:%u failed!\n", combo_dev);
            } else {
                mipi_rx_info_trace("mipi reset mipi_rx combo_dev:%u success!\n", combo_dev);
            }
        }
    }
    return ret;
}

/**
 * @brief: mipi 休眠过程中reset_slvs_ec函数
 * @param: 无
 * @return : 成功为0，不成功则为负
 */
static hi_s32 suspend_mipi_reset_slvs_ec(hi_void)
{
    hi_s32 ret = HI_SUCCESS;
    combo_dev_t combo_dev;
    for (combo_dev = SLVS_DEV_NUM_START; combo_dev < (SLVS_DEV_NUM_START + SLVS_MAX_DEV_NUM); ++combo_dev) {
        reset_status_e reset_slvs_status_flag;
        osal_spin_lock(&g_mipi_ctx_spinlock);
        reset_slvs_status_flag = g_mipi_dev_ctx.reset_slvs_status[combo_dev];
        osal_spin_unlock(&g_mipi_ctx_spinlock);
        if (reset_slvs_status_flag == UNRESET) {
            ret += mipi_reset_slvs_ec(combo_dev);
            if (ret != HI_SUCCESS) {
                mipi_rx_err_trace("mipi reset slvs-ec combo_dev:%u failed!\n", combo_dev);
            } else {
                mipi_rx_info_trace("mipi reset slvs-ec combo_dev:%u success!\n", combo_dev);
            }
        }
    }
    return ret;
}

/**
 * @brief: mipi 休眠过程中disable_mipi_rx_clock函数
 * @param: 无
 * @return : 成功为0，不成功则为负
 */
static hi_s32 suspend_mipi_disable_mipi_rx_clock(hi_void)
{
    hi_s32 ret = HI_SUCCESS;
    combo_dev_t combo_dev = 0;
    // 判断hs mode是否有设置，没有则直接成功退出
    osal_spin_lock(&g_mipi_ctx_spinlock);
    unsigned char hs_mode_cfged_flag = g_mipi_dev_ctx.hs_mode_cfged;
    osal_spin_unlock(&g_mipi_ctx_spinlock);
    if (hs_mode_cfged_flag != TRUE) {
        return HI_SUCCESS;
    }

    for (combo_dev = 0; combo_dev < COMBO_DEV_MAX_NUM; ++combo_dev) {
        // 判断在对应的hs mode下，combo_dev是否有效，无效则跳过并继续
        if (mipi_is_dev_valid(combo_dev) == 0U) {
            continue;
        }
        osal_spin_lock(&g_mipi_ctx_spinlock);
        clock_status_e mipi_clock_status_flag =  g_mipi_dev_ctx.mipi_clock_status[combo_dev];
        osal_spin_unlock(&g_mipi_ctx_spinlock);
        if (mipi_clock_status_flag == ENABLE) {
            ret += mipi_disable_mipi_rx_clock(combo_dev);
            if (ret != HI_SUCCESS) {
                mipi_rx_err_trace("mipi disable mipi_rx combo_dev:%u clock failed!\n", combo_dev);
            } else {
                mipi_rx_info_trace("mipi disable mipi_rx combo_dev:%u clock success!\n", combo_dev);
            }
        }
    }
    return ret;
}

/**
 * @brief: mipi 休眠过程中disable_slvs_ec_clock函数
 * @param: 无
 * @return : 成功为0，不成功则为负
 */
static hi_s32 suspend_mipi_disable_slvs_ec_clock(hi_void)
{
    hi_s32 ret = HI_SUCCESS;
    combo_dev_t combo_dev;
    for (combo_dev = SLVS_DEV_NUM_START; combo_dev < (SLVS_DEV_NUM_START + SLVS_MAX_DEV_NUM); ++combo_dev) {
        clock_status_e slvs_clock_status_flag;
        osal_spin_lock(&g_mipi_ctx_spinlock);
        slvs_clock_status_flag = g_mipi_dev_ctx.slvs_clock_status[combo_dev];
        osal_spin_unlock(&g_mipi_ctx_spinlock);
        if (slvs_clock_status_flag == ENABLE) {
            ret += mipi_disable_slvs_ec_clock(combo_dev);
            if (ret != HI_SUCCESS) {
                mipi_rx_err_trace("mipi disable slvs-ec clock combo_dev:%u failed!\n", combo_dev);
            } else {
                mipi_rx_info_trace("mipi disable slvs-ec clock combo_dev:%u success!\n", combo_dev);
            }
        }
    }
    return ret;
}

/**
 * @brief: mipi 休眠函数
 * @param: 无
 * @return : 成功为0，不成功则为负
 */
hi_s32 mipi_do_suspend(void)
{
    mipi_rx_notice_trace("start mipi rx suspend\n");
    hi_s32 result = HI_SUCCESS;

    // 设置开始休眠状态
    g_mipi_rx_status = MIPI_RX_STATUS_SUSPENDING;
    mipi_rx_info_trace("waite for ioctl call count clearing\n");
    // 循环sleep等待ioctl调用计数清零
    result += mipi_rx_check_ioctl_ready();
    mipi_rx_info_trace("ioctl call count clear result = %d\n", result);
    if (result != HI_SUCCESS) {
        mipi_rx_err_trace("mipi_rx_suspend ioctl call count clear failed\n");
        return result;
    }

    // 1.复位多路对接的 SENSOR。
    result += suspend_mipi_reset_sensor();
    // 2.关闭多路SENSOR 所连接的时钟。
    result += suspend_mipi_disable_sensor_clock();
    // 3.复位多路SENSOR 所对接的MIPI Rx/SLVS。
    result += suspend_mipi_reset_mipi_rx();
    result += suspend_mipi_reset_slvs_ec();
    // 4.关闭多路MIPI/SLVS 时钟。
    result += suspend_mipi_disable_mipi_rx_clock();
    result += suspend_mipi_disable_slvs_ec_clock();
    // 休眠结束
    if (result == HI_SUCCESS) {
        mipi_rx_notice_trace("mipi_rx_suspend success\n");
        g_mipi_rx_status = MIPI_RX_STATUS_SUSPENDED;
    } else {
        mipi_rx_err_trace("mipi_rx_suspend_failed\n");
    }
    return result;
}

/**
 * @brief: mipi 退出执行函数，不管mipi和sensor的状态，全部进行复位和关闭时钟
 * @param: 无
 * @return : 成功为0，不成功则为负
 */
static hi_s32 mipi_do_exit(void)
{
    hi_s32 ret, result;
    hi_u32 i;
    ret = HI_SUCCESS;
    result = HI_SUCCESS;
    for (i = 0; i < SNS_MAX_RST_SOURCE_NUM; ++i) {
        ret = mipi_reset_sensor(i);
        if (ret != HI_SUCCESS) {
            result += ret;
            mipi_rx_err_trace("mipi reset sns_reset_source:%u  failed!\n", i);
        } else {
            mipi_rx_info_trace("mipi reset sns_reset_source:%u  success!\n", i);
        }
    }

    for (i = 0; i < SNS_MAX_CLK_SOURCE_NUM; ++i) {
        ret = mipi_disable_sensor_clock(i);
        if (ret != HI_SUCCESS) {
            result += ret;
            mipi_rx_err_trace("mipi disable sns_clk_source:%u failed!\n", i);
        } else {
            mipi_rx_info_trace("mipi disable sns_clk_source:%u success!\n", i);
        }
    }

    for (i = 0; i < COMBO_DEV_MAX_NUM; ++i) {
        mipi_rx_drv_core_reset(i);
    }

    for (i = 0; i < COMBO_DEV_MAX_NUM; ++i) {
        ret = mipi_rx_drv_disable_clock(i, LANE_DIVIDE_MODE_3);
        if (ret != HI_SUCCESS) {
            result += ret;
            mipi_rx_err_trace("mipi disable mipi_rx combo_dev:%u clock failed!\n", i);
        } else {
            mipi_rx_info_trace("mipi disable mipi_rx combo_dev:%u clock success!\n", i);
        }
    }
    return result;
}

/**
 * @brief: mipi 唤醒过程中enable_mipi_rx_clock函数
 * @param: 无
 * @return : 成功为0，不成功则为负
 */
static hi_s32 resume_mipi_enable_mipi_rx_clock(hi_void)
{
    hi_s32 ret = HI_SUCCESS;
    combo_dev_t combo_dev = 0;
    // 判断hs mode是否有设置，没有则直接成功退出
    osal_spin_lock(&g_mipi_ctx_spinlock);
    unsigned char hs_mode_cfged_flag = g_mipi_dev_ctx.hs_mode_cfged;
    osal_spin_unlock(&g_mipi_ctx_spinlock);
    if (hs_mode_cfged_flag != TRUE) {
        return HI_SUCCESS;
    }

    for (combo_dev = 0; combo_dev < COMBO_DEV_MAX_NUM; ++combo_dev) {
        // 判断在对应的hs mode下，combo_dev是否有效，无效则跳过并继续
        if (mipi_is_dev_valid(combo_dev) == 0U) {
            continue;
        }
        osal_spin_lock(&g_mipi_ctx_spinlock);
        clock_status_e mipi_clock_status_flag =  g_mipi_dev_ctx.mipi_clock_status[combo_dev];
        osal_spin_unlock(&g_mipi_ctx_spinlock);
        if (mipi_clock_status_flag == ENABLE) {
            ret += mipi_enable_mipi_rx_clock(combo_dev);
            if (ret != HI_SUCCESS) {
                mipi_rx_err_trace("mipi enable mipi_rx combo_dev:%u clock failed!\n", combo_dev);
            } else {
                mipi_rx_info_trace("mipi enable mipi_rx combo_dev:%u clock success!\n", combo_dev);
            }
        }
    }
    return ret;
}

/**
 * @brief: mipi 唤醒过程中enable_slvs_ec_clock函数
 * @param: 无
 * @return : 成功为0，不成功则为负
 */
static hi_s32 resume_mipi_enable_slvs_ec_clock(hi_void)
{
    hi_s32 ret = HI_SUCCESS;
    combo_dev_t combo_dev;
    for (combo_dev = SLVS_DEV_NUM_START; combo_dev < (SLVS_DEV_NUM_START + SLVS_MAX_DEV_NUM); ++combo_dev) {
        clock_status_e slvs_clock_status_flag;
        osal_spin_lock(&g_mipi_ctx_spinlock);
        slvs_clock_status_flag = g_mipi_dev_ctx.slvs_clock_status[combo_dev];
        osal_spin_unlock(&g_mipi_ctx_spinlock);
        if (slvs_clock_status_flag == ENABLE) {
            ret += mipi_enable_slvs_ec_clock(combo_dev);
            if (ret != HI_SUCCESS) {
                mipi_rx_err_trace("mipi enable slvs-ec clock combo_dev:%u failed!\n", combo_dev);
            } else {
                mipi_rx_info_trace("mipi enable slvs-ec clock combo_dev:%u success!\n", combo_dev);
            }
        }
    }
    return ret;
}

/**
 * @brief: mipi 唤醒过程中unreset_mipi_rx函数
 * @param: 无
 * @return : 成功为0，不成功则为负
 */
static hi_s32 resume_mipi_unreset_mipi_rx(hi_void)
{
    hi_s32 ret = HI_SUCCESS;
    combo_dev_t combo_dev = 0;
    // 判断hs mode是否有设置，没有则直接成功退出
    osal_spin_lock(&g_mipi_ctx_spinlock);
    unsigned char hs_mode_cfged_flag = g_mipi_dev_ctx.hs_mode_cfged;
    osal_spin_unlock(&g_mipi_ctx_spinlock);
    if (hs_mode_cfged_flag != TRUE) {
        return HI_SUCCESS;
    }

    for (combo_dev = 0; combo_dev < COMBO_DEV_MAX_NUM; ++combo_dev) {
        // 判断在对应的hs mode下，combo_dev是否有效，无效则跳过并继续
        if (mipi_is_dev_valid(combo_dev) == 0U) {
            continue;
        }
        osal_spin_lock(&g_mipi_ctx_spinlock);
        reset_status_e reset_mipi_status_flag = g_mipi_dev_ctx.reset_mipi_status[combo_dev];
        osal_spin_unlock(&g_mipi_ctx_spinlock);
        if (reset_mipi_status_flag == UNRESET) {
            ret += mipi_unreset_mipi_rx(combo_dev);
            if (ret != HI_SUCCESS) {
                mipi_rx_err_trace("mipi unreset mipi_rx combo_dev:%u failed!\n", combo_dev);
            } else {
                mipi_rx_info_trace("mipi unreset mipi_rx combo_dev:%u success!\n", combo_dev);
            }
        }
    }
    return ret;
}

/**
 * @brief: mipi 唤醒过程中unreset_slvs_ec函数
 * @param: 无
 * @return : 成功为0，不成功则为负
 */
static hi_s32 resume_mipi_unreset_slvs_ec(hi_void)
{
    hi_s32 ret = HI_SUCCESS;
    combo_dev_t combo_dev;

    for (combo_dev = SLVS_DEV_NUM_START; combo_dev < (SLVS_DEV_NUM_START + SLVS_MAX_DEV_NUM); ++combo_dev) {
        reset_status_e reset_slvs_status_flag;
        osal_spin_lock(&g_mipi_ctx_spinlock);
        reset_slvs_status_flag = g_mipi_dev_ctx.reset_slvs_status[combo_dev];
        osal_spin_unlock(&g_mipi_ctx_spinlock);
        if (reset_slvs_status_flag == UNRESET) {
            ret += mipi_unreset_slvs_ec(combo_dev);
            if (ret != HI_SUCCESS) {
                mipi_rx_err_trace("mipi unreset slvs-ec combo_dev:%u failed!\n", combo_dev);
            } else {
                mipi_rx_info_trace("mipi unreset slvs-ec combo_dev:%u success!\n", combo_dev);
            }
        }
    }
    return ret;
}

/**
 * @brief: mipi 唤醒过程中config_sensor_clock函数
 * @param: 无
 * @return : 成功为0，不成功则为负
 */
static hi_s32 resume_mipi_config_sensor_clock(hi_void)
{
    hi_s32 ret = HI_SUCCESS;
    sns_clk_source_t sns_clk_source = 0;
    for (sns_clk_source = 0; sns_clk_source < SNS_MAX_CLK_SOURCE_NUM; ++sns_clk_source) {
        osal_spin_lock(&g_mipi_ctx_spinlock);
        clock_status_e sns_clk_source_cfged_flag = (clock_status_e)g_mipi_dev_ctx.sns_clk_config_cfged[sns_clk_source];
        sns_clk_freq_t clk_freq = g_mipi_dev_ctx.clk_freq[sns_clk_source];
        osal_spin_unlock(&g_mipi_ctx_spinlock);
        if ((hi_s8)sns_clk_source_cfged_flag  == TRUE) {
            sns_clk_cfg_t sns_clk_config;
            sns_clk_config.clk_freq = clk_freq;
            sns_clk_config.clk_source = sns_clk_source;
            ret = mipi_config_sensor_clock(sns_clk_config);
            if (ret != HI_SUCCESS) {
                mipi_rx_err_trace("mipi config sensor clock sns_clk_source:%u failed!\n", sns_clk_source);
            } else {
                mipi_rx_info_trace("mipi config sensor clock sns_clk_source:%u success!\n", sns_clk_source);
            }
        }
    }
    return ret;
}

/**
 * @brief: mipi 唤醒过程中 enable sensor clock函数
 * @param: 无
 * @return : 成功为0，不成功则为负
 */
static hi_s32 resume_mipi_enable_sensor_clock(hi_void)
{
    hi_s32 ret = HI_SUCCESS;
    sns_clk_source_t sns_clk_source = 0;
    for (sns_clk_source = 0; sns_clk_source < SNS_MAX_CLK_SOURCE_NUM; ++sns_clk_source) {
        osal_spin_lock(&g_mipi_ctx_spinlock);
        clock_status_e sns_clk_source_cfged_flag = g_mipi_dev_ctx.sns_clk_source_cfged[sns_clk_source];
        osal_spin_unlock(&g_mipi_ctx_spinlock);
        if (sns_clk_source_cfged_flag == ENABLE) {
            ret = mipi_enable_sensor_clock(sns_clk_source);
            if (ret != HI_SUCCESS) {
                mipi_rx_err_trace("mipi enable sns_clk_source:%u failed!\n", sns_clk_source);
            } else {
                mipi_rx_info_trace("mipi enable sns_clk_source:%u success!\n", sns_clk_source);
            }
        }
    }
    return ret;
}

/**
 * @brief: mipi 唤醒过程中 unreset sensor函数
 * @param: 无
 * @return : 成功为0，不成功则为负
 */
static hi_s32 resume_mipi_unreset_sensor(hi_void)
{
    hi_s32 ret = HI_SUCCESS;
    sns_rst_source_t sns_reset_source = 0;
    for (sns_reset_source = 0; sns_reset_source < SNS_MAX_RST_SOURCE_NUM; ++sns_reset_source) {
        osal_spin_lock(&g_mipi_ctx_spinlock);
        reset_status_e sns_rst_source_cfged_flag = g_mipi_dev_ctx.sns_rst_source_cfged[sns_reset_source];
        osal_spin_unlock(&g_mipi_ctx_spinlock);
        if (sns_rst_source_cfged_flag == UNRESET) {
            ret = mipi_unreset_sensor(sns_reset_source);
            if (ret != HI_SUCCESS) {
                mipi_rx_err_trace("mipi unreset sns_reset_source:%u  failed!\n", sns_reset_source);
            } else {
                mipi_rx_info_trace("mipi unreset sns_reset_source:%u  success!\n", sns_reset_source);
            }
        }
    }
    return ret;
}

/**
 * @brief: mipi 唤醒函数
 * @param: 无
 * @return : 成功为0，不成功则为负
 */
hi_s32 mipi_do_resume(hi_void)
{
    hi_s32 result = HI_SUCCESS;

    mipi_rx_notice_trace("start mipi resume\n");
    // 设置当前为RESUMING状态

    g_mipi_rx_status = MIPI_RX_STATUS_RESUMING;

    // 1.设置模式
    result += mipi_resume_hs_mode();
    // 2.打开多路MIPI/SLVS 时钟
    result += resume_mipi_enable_mipi_rx_clock();
    result += resume_mipi_enable_slvs_ec_clock();
    // 3.复位多路SENSOR 所对接的MIPI Rx/SLVS
    result += suspend_mipi_reset_mipi_rx();
    result += suspend_mipi_reset_slvs_ec();
    // 4.SENSOR工作在从模式时，需要配置多路SENSOR的时钟频率，必须在开启Sensor时钟前配置。
    result += resume_mipi_config_sensor_clock();
    // 5.打开多路SENSOR 所连接的时钟。
    result += resume_mipi_enable_sensor_clock();
    // 6.复位对接的所有 SENSOR
    result += suspend_mipi_reset_sensor();
    // 7.配置 MIPI Rx/SLVS 设备属性
    result += mipi_resume_combo_dev_attr();
    // 8.撤销复位多路 SENSOR 所对接的 MIPI Rx/SLVS
    result += resume_mipi_unreset_mipi_rx();
    result += resume_mipi_unreset_slvs_ec();
    // 9.撤销复位对接的所有SENSOR
    result += resume_mipi_unreset_sensor();
    // 唤醒结束
    if (result == HI_SUCCESS) {
        g_mipi_rx_status = MIPI_RX_STATUS_RUNNING;
        mipi_rx_notice_trace("mipi_rx_resume_end\n");
    } else {
        mipi_rx_err_trace("mipi_rx_resume_failed\n");
    }
    return result;
}