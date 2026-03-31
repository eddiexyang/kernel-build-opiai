/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: mipi_rx hardware abstract level methods.
 * Author: Hisilicon multimedia software group
 * Create: 2023-03-08
 */

#include "mipi_rx_hal.h"
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/list.h>
#include "securec.h"
#include "hi_osal.h"
#include "hi_debug.h"
#include "mipi_rx_type.h"
#include "hi_mipi_rx.h"
#include "mipi_rx_reg.h"

// ISP SubCtrl Register (MIPIRx Sensor Reset & Clock & Workmode)
static const hi_u64 ISP_SUBCTRL_BASE_REG = 0x300130000U;
static const hi_u64 ISP_SUBCTRL_SIZE = 0x10000U;
static hi_ulong g_isp_subctrl_base_va;

// MIPIRx Module Register
#define MIPI_RX_REGS_ADDR 0x300280000
#define MIPI_RX_REGS_SIZE 0x40000

#define MIPI_INTR_COUNT_UPPER_LIMIT          10000U

static volatile mipi_rx_regs_type_t *g_mipi_rx_regs_va = NULL;
typedef struct {
    unsigned int phy_rg_ext_en;
    unsigned int phy_rg_ext2_en;
    unsigned int phy_rg_int_en;
    unsigned int phy_rg_drveclk2_enz;
    unsigned int phy_rg_drveclk_enz;
} phy_mode_link_t;

typedef enum {
    MIPI_ESC_D0 = (hi_u32)0x1U << 0U,
    MIPI_ESC_D1 = (hi_u32)0x1U << 1U,
    MIPI_ESC_D2 = (hi_u32)0x1U << 2U,
    MIPI_ESC_D3 = (hi_u32)0x1U << 3U,
    MIPI_ESC_CLK = (hi_u32)0x1U << 4U,
    MIPI_ESC_CLK2 = (hi_u32)0x1U << 5U,

    MIPI_TIMEOUT_D0 = (hi_u32)0x1U << 8U,
    MIPI_TIMEOUT_D1 = (hi_u32)0x1U << 9U,
    MIPI_TIMEOUT_D2 = (hi_u32)0x1U << 10U,
    MIPI_TIMEOUT_D3 = (hi_u32)0x1U << 11U,
    MIPI_TIMEOUT_CLK = (hi_u32)0x1U << 12U,
    MIPI_TIMEOUT_CLK2 = (hi_u32)0x1U << 13U
} phy_err_int_state;

typedef enum {
    MIPI_VC0_MISMATCH = (hi_u32)0x1U << 20U, /* VC0,frame's start and frame's end do not match */
    MIPI_VC1_MISMATCH = (hi_u32)0x1U << 21U, /* VC1,frame's start and frame's end do not match */
    MIPI_VC2_MISMATCH = (hi_u32)0x1U << 22U, /* VC2,frame's start and frame's end do not match */
    MIPI_VC3_MISMATCH = (hi_u32)0x1U << 23U, /* VC3,frame's start and frame's end do not match */

    MIPI_VC0_FRAME_NUM_ERR = (hi_u32)0x1U << 16U, /* VC0'S frame order error */
    MIPI_VC1_FRAME_NUM_ERR = (hi_u32)0x1U << 17U, /* VC1'S frame order error */
    MIPI_VC2_FRAME_NUM_ERR = (hi_u32)0x1U << 18U, /* VC2'S frame order error */
    MIPI_VC3_FRAME_NUM_ERR = (hi_u32)0x1U << 19U, /* VC3'S frame order error */

    MIPI_VC0_ECC_ERR = (hi_u32)0x1U << 4U, /* VC0'S header has errors,and ECC error correction is ok */
    MIPI_VC1_ECC_ERR = (hi_u32)0x1U << 5U,
    MIPI_VC2_ECC_ERR = (hi_u32)0x1U << 6U,
    MIPI_VC3_ECC_ERR = (hi_u32)0x1U << 7U,
    MIPI_MULT_ECC_ERR = (hi_u32)0x1U << 8U, /* Header has two error at least ,and ECC error correction is invalid */

    MIPI_VC0_CRC_ERR = (hi_u32)0x1U << 0U, /* in the last frame, VC0'S data has a CRC ERROR at least */
    MIPI_VC1_CRC_ERR = (hi_u32)0x1U << 1U, /* in the last frame, VC1'S data has a CRC ERROR at least */
    MIPI_VC2_CRC_ERR = (hi_u32)0x1U << 2U, /* in the last frame, VC2'S data has a CRC ERROR at least */
    MIPI_VC3_CRC_ERR = (hi_u32)0x1U << 3U /* in the last frame, VC3'S data has a CRC ERROR at least */
} mipi_csi_int_state;

typedef enum {
    CMD_FIFO_WRITE_ERR = (hi_u32)0x1U << 0U, /* MIPI_CTRL write command FIFO error */
    DATA_FIFO_WRITE_ERR = (hi_u32)0x1U << 1U,
    INT_VSYNC           = (hi_u32)0x1U << 4U,             // 帧同步中断
    CMD_FIFO_READ_ERR = (hi_u32)0x1U << 16U,
    DATA_FIFO_READ_ERR = (hi_u32)0x1U << 17U
} mipi_ctrl_int_state;

typedef enum {
    LANE0_SYNC_ERR = (hi_u32)0x1U << 0U,
    LANE1_SYNC_ERR = (hi_u32)0x1U << 1U,
    LANE2_SYNC_ERR = (hi_u32)0x1U << 2U,
    LANE3_SYNC_ERR = (hi_u32)0x1U << 3U,
    LANE4_SYNC_ERR = (hi_u32)0x1U << 4U,
    LANE5_SYNC_ERR = (hi_u32)0x1U << 5U,
    LANE6_SYNC_ERR = (hi_u32)0x1U << 6U,
    LANE7_SYNC_ERR = (hi_u32)0x1U << 7U,

    LINK0_WRITE_ERR = (hi_u32)0x1U << 16U,
    LINK1_WRITE_ERR = (hi_u32)0x1U << 17U,
    LINK2_WRITE_ERR = (hi_u32)0x1U << 18U,
    LINK0_READ_ERR = (hi_u32)0x1U << 20U,
    LINK1_READ_ERR = (hi_u32)0x1U << 21U,
    LINK2_READ_ERR = (hi_u32)0x1U << 22U,
    LVDS_STAT_ERR = (hi_u32)0x1U << 24U,
    LVDS_POP_ERR = (hi_u32)0x1U << 25U,
    CMD_WR_ERR = (hi_u32)0x1U << 26U,
    CMD_RD_ERR = (hi_u32)0x1U << 27U,
    LVDS_VSYNC = (hi_u32)0x1U << 28U
} lvds_int_state;

typedef enum {
    ALIGN_FIFO_FULL_ERR = (hi_u32)0x1U << 0U,
    ALIGN_LANE0_ERR = (hi_u32)0x1U << 1U,
    ALIGN_LANE1_ERR = (hi_u32)0x1U << 2U,
    ALIGN_LANE2_ERR = (hi_u32)0x1U << 3U,
    ALIGN_LANE3_ERR = (hi_u32)0x1U << 4U,
    ALIGN_LANE4_ERR = (hi_u32)0x1U << 5U,
    ALIGN_LANE5_ERR = (hi_u32)0x1U << 6U,
    ALIGN_LANE6_ERR = (hi_u32)0x1U << 7U,
    ALIGN_LANE7_ERR = (hi_u32)0x1U << 8U,
} align_int_state;

#ifdef OT_FPGA
#define MIPI_RX_CRG_ADDR 0x173F1008
#else
#define MIPI_RX_CRG_ADDR 0x11018540
#endif

#define MIPI_CIL_FREQ 150 /* MIPI CIL FREQ 150MHZ */

static unsigned int g_reg_map_flag = 0;

#define is_valid_id(id) ((id) != -1)
#define is_data_type_yuv(input_data_type) (((input_data_type) >= DATA_TYPE_YUV420_8BIT_NORMAL) && \
                                           ((input_data_type) <= DATA_TYPE_YUV422_PACKED))

#define SKEW_LINK       0x0
#define MIPI_DESKEW_CAL 0xffff000f
#define MIPI_FSMO_VALUE 0x000d1d0c

static unsigned int g_mipi_rx_irq_num = MIPI_RX_IRQ;

static const phy_mode_link_t g_phy_mode[][MIPI_RX_MAX_PHY_NUM] = {
#ifndef OT_FPGA
    {{ 1, 0, 0, 1, 0 }, { 1, 0, 0, 1, 1 }},
#else
    {{ 0, 0, 1, 0, 1 }, { 0, 0, 0, 1, 1 }},
#endif
    {{ 0, 1, 0, 0, 1 }, { 0, 1, 0, 0, 1 }},
    {{ 0, 1, 0, 0, 1 }, { 0, 0, 1, 1, 1 }},
    {{ 0, 0, 1, 1, 1 }, { 0, 0, 1, 1, 1 }},
};

static const unsigned int g_hs_mode[] = {0x0, 0x7, 0xA, 0xB};

static phy_err_int_cnt_t g_phy_err_int_cnt[MIPI_RX_MAX_PHY_NUM];
static mipi_err_int_cnt_t g_mipi_err_int_cnt[MIPI_RX_MAX_DEV_NUM];
static lvds_err_int_cnt_t g_lvds_err_int_cnt[MIPI_RX_MAX_DEV_NUM];
static align_err_int_cnt_t g_align_err_int_cnt[MIPI_RX_MAX_DEV_NUM];

/* function definition */
void mipi_rx_drv_set_irq_num(unsigned int irq_num)
{
    g_mipi_rx_irq_num = irq_num;
}

void mipi_rx_drv_set_regs(mipi_rx_regs_type_t *regs)
{
    g_mipi_rx_regs_va = regs;
}

phy_err_int_cnt_t *mipi_rx_drv_get_phy_err_int_cnt(unsigned int phy_id)
{
    return &g_phy_err_int_cnt[phy_id];
}

mipi_err_int_cnt_t *mipi_rx_drv_get_mipi_err_int_cnt(unsigned int devno)
{
    return &g_mipi_err_int_cnt[devno];
}

lvds_err_int_cnt_t *mipi_rx_drv_get_lvds_err_int_cnt(unsigned int devno)
{
    return &g_lvds_err_int_cnt[devno];
}

align_err_int_cnt_t *mipi_rx_drv_get_align_err_int_cnt(unsigned int devno)
{
    return &g_align_err_int_cnt[devno];
}

hi_ulong get_isp_subctrl_base(hi_void)
{
    return g_isp_subctrl_base_va;
}


static void mipi_set_bit(unsigned long value, unsigned long offset,
                         unsigned long addr)
{
    unsigned long t, mask;

    mask = 1UL << offset;
    t = osal_readl((const volatile void *)addr);
    t &= ~mask;
    t |= (value << offset) & mask;
    osal_writel(t, (volatile void *)addr);
}

hi_void write_reg32(hi_ulong addr, hi_u32 value, hi_u32 mask)
{
    hi_u32 t;

    t = osal_readl((void *)addr);
    t &= ~mask;
    t |= value & mask;
    osal_writel(t, (void *)addr);
}

hi_u32 read_reg32(hi_ulong addr)
{
    return osal_readl((void *)addr);
}

hi_s32 check_reg(hi_ulong addr, hi_u32 value, hi_u32 mask)
{
    hi_u32 t;
    hi_s32 ret;

    t = osal_readl((void *)addr);
    t &= mask;
    ret = ((t == value) ? HI_SUCCESS : HI_FAILURE);

    return ret;
}
static volatile mipi_rx_phy_cfg_t *get_mipi_rx_phy_regs(int phy_id)
{
    return &g_mipi_rx_regs_va->mipi_rx_phy_cfg[phy_id];
}

static volatile mipi_rx_sys_regs_t *get_mipi_rx_sys_regs(void)
{
    return &g_mipi_rx_regs_va->mipi_rx_sys_regs;
}

static volatile mipi_ctrl_regs_t *get_mipi_ctrl_regs(unsigned int devno)
{
    return &g_mipi_rx_regs_va->mipi_rx_ctrl_regs[devno].mipi_ctrl_regs;
}

static volatile lvds_ctrl_regs_t *get_lvds_ctrl_regs(unsigned int devno)
{
    return &g_mipi_rx_regs_va->mipi_rx_ctrl_regs[devno].lvds_ctrl_regs;
}

static volatile global_ctrl_regs_t *get_global_ctrl_regs(unsigned int devno)
{
    return &g_mipi_rx_regs_va->mipi_rx_ctrl_regs[devno].global_ctrl_regs;
}

#ifndef OT_FPGA
static void mipi_rx_set_cil_int_mask(int phy_id, unsigned int mask)
{
    u_mipi_int_msk mipi_int_msk;
    volatile mipi_rx_phy_cfg_t *mipi_rx_phy_cfg = NULL;
    volatile mipi_rx_sys_regs_t *mipi_rx_sys_regs = get_mipi_rx_sys_regs();

    mipi_int_msk.u32 = mipi_rx_sys_regs->mipi_int_msk.u32;

    if (phy_id == 0) {
        mipi_int_msk.bits.int_phycil0_mask = 0x0;
    } else if (phy_id == 1) {
        mipi_int_msk.bits.int_phycil1_mask = 0x0;
    } else {
        mipi_rx_err_trace("Invail phy_id: %d\n", phy_id);
        return;
    }
    mipi_rx_sys_regs->mipi_int_msk.u32 = mipi_int_msk.u32;
    // 配置中断掩码前，清除中断计算
    (void)memset_s(&g_phy_err_int_cnt[phy_id], sizeof(phy_err_int_cnt_t), 0, sizeof(phy_err_int_cnt_t));
    mipi_rx_phy_cfg = get_mipi_rx_phy_regs(phy_id);
    mipi_rx_phy_cfg->mipi_cil_int_msk_link.u32 = mask;
}

static void mipi_rx_set_phy_skew_link(int phy_id, unsigned int value)
{
    volatile u_phy_skew_link phy_skew_link;
    volatile mipi_rx_phy_cfg_t *mipi_rx_phy_cfg = NULL;

    mipi_rx_phy_cfg = get_mipi_rx_phy_regs(phy_id);
    phy_skew_link.u32 = value;
    mipi_rx_phy_cfg->phy_skew_link.u32 = phy_skew_link.u32;
}

static void mipi_rx_set_phy_deskew_cal_link(int phy_id, unsigned int value)
{
    volatile u_phy_deskew_cal_link phy_deskew_cal_link;
    volatile mipi_rx_phy_cfg_t *mipi_rx_phy_cfg = NULL;

    mipi_rx_phy_cfg = get_mipi_rx_phy_regs(phy_id);
    phy_deskew_cal_link.u32 = value;
    mipi_rx_phy_cfg->phy_deskew_cal_link.u32 = phy_deskew_cal_link.u32;
}

static void mipi_rx_set_phy_fsmo_link(int phy_id, unsigned int value)
{
    volatile mipi_rx_phy_cfg_t *mipi_rx_phy_cfg = NULL;

    mipi_rx_phy_cfg = get_mipi_rx_phy_regs(phy_id);
    mipi_rx_phy_cfg->cil_fsm0_link.u32 = value;
}

#endif

static void mipi_rx_set_phy_rg_ext_en(int phy_id, unsigned int enable)
{
    volatile u_phy_mode_link phy_mode_link;
    volatile mipi_rx_phy_cfg_t *mipi_rx_phy_cfg = NULL;

    mipi_rx_phy_cfg = get_mipi_rx_phy_regs(phy_id);
    phy_mode_link.u32 = mipi_rx_phy_cfg->phy_mode_link.u32;
    phy_mode_link.bits.phy_rg_ext_en = enable;
    mipi_rx_phy_cfg->phy_mode_link.u32 = phy_mode_link.u32;
}

static void mipi_rx_set_phy_rg_ext2_en(int phy_id, unsigned int enable)
{
    volatile u_phy_mode_link phy_mode_link;
    volatile mipi_rx_phy_cfg_t *mipi_rx_phy_cfg = NULL;

    mipi_rx_phy_cfg = get_mipi_rx_phy_regs(phy_id);
    phy_mode_link.u32 = mipi_rx_phy_cfg->phy_mode_link.u32;
    phy_mode_link.bits.phy_rg_ext2_en = enable;
    mipi_rx_phy_cfg->phy_mode_link.u32 = phy_mode_link.u32;
}

static void mipi_rx_set_phy_rg_int_en(int phy_id, unsigned int enable)
{
    volatile u_phy_mode_link phy_mode_link;
    volatile mipi_rx_phy_cfg_t *mipi_rx_phy_cfg = NULL;

    mipi_rx_phy_cfg = get_mipi_rx_phy_regs(phy_id);
    phy_mode_link.u32 = mipi_rx_phy_cfg->phy_mode_link.u32;
    phy_mode_link.bits.phy_rg_int_en = enable;
    mipi_rx_phy_cfg->phy_mode_link.u32 = phy_mode_link.u32;
}

static void mipi_rx_set_phy_rg_drveclk2_enz(int phy_id, unsigned int enable)
{
    volatile u_phy_mode_link phy_mode_link;
    volatile mipi_rx_phy_cfg_t *mipi_rx_phy_cfg = NULL;

    mipi_rx_phy_cfg = get_mipi_rx_phy_regs(phy_id);
    phy_mode_link.u32 = mipi_rx_phy_cfg->phy_mode_link.u32;
    phy_mode_link.bits.phy_rg_drveclk2_enz = enable;
    mipi_rx_phy_cfg->phy_mode_link.u32 = phy_mode_link.u32;
}

static void mipi_rx_set_phy_rg_drveclk_enz(int phy_id, unsigned int enable)
{
    volatile u_phy_mode_link phy_mode_link;
    volatile mipi_rx_phy_cfg_t *mipi_rx_phy_cfg = NULL;

    mipi_rx_phy_cfg = get_mipi_rx_phy_regs(phy_id);
    phy_mode_link.u32 = mipi_rx_phy_cfg->phy_mode_link.u32;
    phy_mode_link.bits.phy_rg_drveclk_enz = enable;
    mipi_rx_phy_cfg->phy_mode_link.u32 = phy_mode_link.u32;
}

void mipi_rx_drv_set_work_mode(unsigned int devno, input_mode_t input_mode)
{
    mipi_rx_info_trace("devno: %u, input_mode: %u\n", devno, (hi_u32)input_mode);

    const hi_u32 mipi_slvs_sel_reg_offset = 0x3004;
    const hi_u32 mipirx_workmode_reg_offset = 0x3008;
    const hi_u32 mipirx_misc_ctrl_reg_offset = 0x341C;

    if (input_mode == INPUT_MODE_MIPI) {
        // Set bit 0:1 to 0; Config Port 0 & 1 to MIPI Mode
        write_reg32(g_isp_subctrl_base_va + mipi_slvs_sel_reg_offset, 0x0U, 0x3U);
        write_reg32(g_isp_subctrl_base_va + mipirx_workmode_reg_offset,
            (hi_u32)0x0U << (2U * devno), (hi_u32)0x1U << (2U * devno));
        write_reg32(g_isp_subctrl_base_va + mipirx_misc_ctrl_reg_offset,
            (hi_u32)0x0U << (2U + devno), (hi_u32)0x1U << (2U + devno));
        write_reg32(g_isp_subctrl_base_va + mipirx_misc_ctrl_reg_offset, 0x0U, 0x3U);
    } else if ((input_mode == INPUT_MODE_SUBLVDS) ||
               (input_mode == INPUT_MODE_HISPI) ||
               (input_mode == INPUT_MODE_LVDS)) {
        // Set bit 0:1 to 0; Config Port 0 & 1 to MIPI Mode
        write_reg32(g_isp_subctrl_base_va + mipi_slvs_sel_reg_offset, 0x0U, 0x3U);
        write_reg32(g_isp_subctrl_base_va + mipirx_workmode_reg_offset,
            (hi_u32)0x1U << (2U * devno), (hi_u32)0x1U << (2U * devno));
        write_reg32(g_isp_subctrl_base_va + mipirx_misc_ctrl_reg_offset,
            (hi_u32)0x1U << (2U + devno), (hi_u32)0x1U << (2U + devno));
        write_reg32(g_isp_subctrl_base_va + mipirx_misc_ctrl_reg_offset, 0x0U, 0x3U);
    } else {
        // Set bit 0:1 to 1; Config Port 0 & 1 to SLVS_EC Mode
        write_reg32(g_isp_subctrl_base_va + mipi_slvs_sel_reg_offset, 0x3, 0x3);
    }
}

void mipi_rx_drv_set_mipi_image_rect(unsigned int devno, const img_rect_t *p_img_rect)
{
    u_mipi_crop_start_chn0 crop_start_chn0;
    u_mipi_crop_start_chn1 crop_start_chn1;
    u_mipi_crop_start_chn2 crop_start_chn2;
    u_mipi_crop_start_chn3 crop_start_chn3;
    u_mipi_imgsize mipi_imgsize;

    volatile mipi_ctrl_regs_t *mipi_ctrl_regs = get_mipi_ctrl_regs(devno);

    crop_start_chn0.u32 = mipi_ctrl_regs->mipi_crop_start_chn0.u32;
    crop_start_chn1.u32 = mipi_ctrl_regs->mipi_crop_start_chn1.u32;
    crop_start_chn2.u32 = mipi_ctrl_regs->mipi_crop_start_chn2.u32;
    crop_start_chn3.u32 = mipi_ctrl_regs->mipi_crop_start_chn3.u32;
    mipi_imgsize.u32 = mipi_ctrl_regs->mipi_imgsize.u32;

    mipi_imgsize.bits.mipi_imgwidth = p_img_rect->width - 1U;
    mipi_imgsize.bits.mipi_imgheight = p_img_rect->height - 1U;

    crop_start_chn0.bits.mipi_start_x_chn0 = (unsigned int) p_img_rect->x;
    crop_start_chn0.bits.mipi_start_y_chn0 = (unsigned int) p_img_rect->y;

    crop_start_chn1.bits.mipi_start_x_chn1 = (unsigned int) p_img_rect->x;
    crop_start_chn1.bits.mipi_start_y_chn1 = (unsigned int) p_img_rect->y;

    crop_start_chn2.bits.mipi_start_x_chn2 = (unsigned int) p_img_rect->x;
    crop_start_chn2.bits.mipi_start_y_chn2 = (unsigned int) p_img_rect->y;

    crop_start_chn3.bits.mipi_start_x_chn3 = (unsigned int) p_img_rect->x;
    crop_start_chn3.bits.mipi_start_y_chn3 = (unsigned int) p_img_rect->y;

    mipi_ctrl_regs->mipi_crop_start_chn0.u32 = crop_start_chn0.u32;
    mipi_ctrl_regs->mipi_crop_start_chn1.u32 = crop_start_chn1.u32;
    mipi_ctrl_regs->mipi_crop_start_chn2.u32 = crop_start_chn2.u32;
    mipi_ctrl_regs->mipi_crop_start_chn3.u32 = crop_start_chn3.u32;
    mipi_ctrl_regs->mipi_imgsize.u32 = mipi_imgsize.u32;
}

void mipi_rx_drv_set_mipi_crop_en(unsigned int devno, unsigned int enable)
{
    u_mipi_ctrl_mode_pixel mipi_ctrl_mode_pixel;
    volatile mipi_ctrl_regs_t *mipi_ctrl_regs = get_mipi_ctrl_regs(devno);

    mipi_ctrl_mode_pixel.u32 = mipi_ctrl_regs->mipi_ctrl_mode_pixel.u32;
    mipi_ctrl_mode_pixel.bits.crop_en = enable;
    mipi_ctrl_regs->mipi_ctrl_mode_pixel.u32 = mipi_ctrl_mode_pixel.u32;
}

static short mipi_rx_drv_get_data_type(data_type_t input_data_type)
{
    if (input_data_type == DATA_TYPE_RAW_8BIT) {
        return 0x2a;
    } else if (input_data_type == DATA_TYPE_RAW_10BIT) {
        return 0x2b;
    } else if (input_data_type == DATA_TYPE_RAW_12BIT) {
        return 0x2c;
    } else if (input_data_type == DATA_TYPE_RAW_14BIT) {
        return 0x2d;
    } else if (input_data_type == DATA_TYPE_RAW_16BIT) {
        return 0x2e;
    } else if (input_data_type == DATA_TYPE_YUV420_8BIT_NORMAL) {
        return 0x18;
    } else if (input_data_type == DATA_TYPE_YUV420_8BIT_LEGACY) {
        return 0x1a;
    } else if (input_data_type == DATA_TYPE_YUV422_8BIT) {
        return 0x1e;
    } else if (input_data_type == DATA_TYPE_YUV422_PACKED) {
        return 0x1e;
    } else {
        return 0x0;
    }
}

static short mipi_rx_drv_get_data_bit_width(data_type_t input_data_type)
{
    if (input_data_type == DATA_TYPE_RAW_8BIT) {
        return 0x0;
    } else if (input_data_type == DATA_TYPE_RAW_10BIT) {
        return 0x1;
    } else if (input_data_type == DATA_TYPE_RAW_12BIT) {
        return 0x2;
    } else if (input_data_type == DATA_TYPE_RAW_14BIT) {
        return 0x3;
    } else if (input_data_type == DATA_TYPE_RAW_16BIT) {
        return 0x4;
    } else if (input_data_type == DATA_TYPE_YUV420_8BIT_NORMAL) {
        return 0x0;
    } else if (input_data_type == DATA_TYPE_YUV420_8BIT_LEGACY) {
        return 0x0;
    } else if (input_data_type == DATA_TYPE_YUV422_8BIT) {
        return 0x0;
    } else if (input_data_type == DATA_TYPE_YUV422_PACKED) {
        return 0x4;
    } else {
        return 0x0;
    }
}

/* magic num mean bit width, convert to register condfig */
static short mipi_rx_drv_get_ext_data_bit_width(unsigned int ext_data_bit_width)
{
    if (ext_data_bit_width == 8U) { /* 8 bits width */
        return 0x0;
    } else if (ext_data_bit_width == 10U) { /* 10 bits width */
        return 0x1;
    } else if (ext_data_bit_width == 12U) { /* 12 bits width */
        return 0x2;
    } else if (ext_data_bit_width == 14U) { /* 14 bits width */
        return 0x3;
    } else if (ext_data_bit_width == 16U) { /* 16 bits width */
        return 0x4;
    } else {
        return 0x0;
    }
}

/* magic num mean data type index, convert to register config */
static void mipi_rx_drv_set_mipi_user_dt(unsigned int devno, int type_index, short data_type, short bit_width)
{
    u_mipi_userdef_dt user_def_dt;
    u_mipi_user_def user_def;
    volatile mipi_ctrl_regs_t *mipi_ctrl_regs = get_mipi_ctrl_regs(devno);

    if (mipi_ctrl_regs != NULL) {
        user_def_dt.u32 = mipi_ctrl_regs->mipi_userdef_dt.u32;
        user_def.u32 = mipi_ctrl_regs->mipi_user_def.u32;

        if (type_index == 0) { // id = 0
            user_def_dt.bits.user_def0_dt = (unsigned short)bit_width;
            user_def.bits.user_def0 = (unsigned short)data_type;
        } else if (type_index == 1) { // id = 1
            user_def_dt.bits.user_def1_dt = (unsigned short)bit_width;
            user_def.bits.user_def1 = (unsigned short)data_type;
        } else if (type_index == 2) { // id = 2
            user_def_dt.bits.user_def2_dt = (unsigned short)bit_width;
            user_def.bits.user_def2 = (unsigned short)data_type;
        } else if (type_index == 3) { // id = 3
            user_def_dt.bits.user_def3_dt = (unsigned short)bit_width;
            user_def.bits.user_def3 = (unsigned short)data_type;
        } else {
            // fix sc
        }

        mipi_ctrl_regs->mipi_userdef_dt.u32 = user_def_dt.u32;
        mipi_ctrl_regs->mipi_user_def.u32 = user_def.u32;
    }
}

static void mipi_rx_drv_set_user_ef(unsigned int devno, data_type_t input_data_type)
{
    u_mipi_userdef_dt user_def_dt;
    u_mipi_user_def user_def;
    volatile mipi_ctrl_regs_t *mipi_ctrl_regs = get_mipi_ctrl_regs(devno);
    unsigned char bit_width;
    unsigned int temp_data_type;

    if (input_data_type == DATA_TYPE_YUV420_8BIT_NORMAL) {
        bit_width = 0;
        temp_data_type = 0x18;
    } else if (input_data_type == DATA_TYPE_YUV420_8BIT_LEGACY) {
        bit_width = 0;
        temp_data_type = 0x1a;
    } else if (input_data_type == DATA_TYPE_YUV422_8BIT) {
        bit_width = 0;
        temp_data_type = 0x1e;
    } else {
        bit_width = 4; /* 4 bits width */
        temp_data_type = 0x1e;
    }

    user_def_dt.bits.user_def0_dt = bit_width;
    user_def_dt.bits.user_def1_dt = bit_width;
    user_def_dt.bits.user_def2_dt = bit_width;
    user_def_dt.bits.user_def3_dt = bit_width;

    user_def.bits.user_def0 = temp_data_type;
    user_def.bits.user_def1 = temp_data_type;
    user_def.bits.user_def2 = temp_data_type;
    user_def.bits.user_def3 = temp_data_type;

    mipi_ctrl_regs->mipi_userdef_dt.u32 = user_def_dt.u32;
    mipi_ctrl_regs->mipi_user_def.u32 = user_def.u32;
}

void mipi_rx_drv_set_mipi_yuv_dt(unsigned int devno, data_type_t input_data_type)
{
    u_mipi_ctrl_mode_hs mipi_ctrl_mode_hs;
    u_mipi_ctrl_mode_pixel mipi_ctrl_mode_pixel;
    volatile mipi_ctrl_regs_t *mipi_ctrl_regs = get_mipi_ctrl_regs(devno);

    /* mipi_ctrl_mode_hs */
    mipi_ctrl_mode_hs.u32 = mipi_ctrl_regs->mipi_ctrl_mode_hs.u32;

    if (!is_data_type_yuv(input_data_type)) {
        mipi_ctrl_mode_hs.bits.user_def_en = 0;
        mipi_ctrl_regs->mipi_ctrl_mode_hs.u32 = mipi_ctrl_mode_hs.u32;
    } else {
        mipi_ctrl_mode_hs.bits.user_def_en = 1;
        mipi_ctrl_regs->mipi_ctrl_mode_hs.u32 = mipi_ctrl_mode_hs.u32;

        /* user_def && user_def_dt */
        mipi_rx_drv_set_user_ef(devno, input_data_type);
    }

    /* mipi_ctrl_mode_pixel */
    mipi_ctrl_mode_pixel.u32 = mipi_ctrl_regs->mipi_ctrl_mode_pixel.u32;

    if (input_data_type == DATA_TYPE_YUV420_8BIT_NORMAL) {
        mipi_ctrl_mode_pixel.bits.mipi_yuv_420_nolegacy_en = 1;
        mipi_ctrl_mode_pixel.bits.mipi_yuv_420_legacy_en = 0;
        mipi_ctrl_mode_pixel.bits.mipi_yuv_422_en = 0;
    } else if (input_data_type == DATA_TYPE_YUV420_8BIT_LEGACY) {
        mipi_ctrl_mode_pixel.bits.mipi_yuv_420_nolegacy_en = 0;
        mipi_ctrl_mode_pixel.bits.mipi_yuv_420_legacy_en = 1;
        mipi_ctrl_mode_pixel.bits.mipi_yuv_422_en = 0;
    } else if (input_data_type == DATA_TYPE_YUV422_8BIT) {
        mipi_ctrl_mode_pixel.bits.mipi_yuv_420_nolegacy_en = 0;
        mipi_ctrl_mode_pixel.bits.mipi_yuv_420_legacy_en = 0;
        mipi_ctrl_mode_pixel.bits.mipi_yuv_422_en = 1;
    } else { /* reset raw type */
        mipi_ctrl_mode_pixel.bits.mipi_yuv_420_nolegacy_en = 0;
        mipi_ctrl_mode_pixel.bits.mipi_yuv_420_legacy_en = 0;
        mipi_ctrl_mode_pixel.bits.mipi_yuv_422_en = 0;
    }
    mipi_ctrl_regs->mipi_ctrl_mode_pixel.u32 = mipi_ctrl_mode_pixel.u32;
}

void mipi_rx_drv_set_mipi_wdr_user_dt(unsigned int devno, data_type_t input_data_type,
                                      const short data_type[WDR_VC_NUM])
{
    u_mipi_userdef_dt user_def_dt;
    u_mipi_user_def user_def;
    volatile mipi_ctrl_regs_t *mipi_ctrl_regs = get_mipi_ctrl_regs(devno);

    user_def_dt.u32 = mipi_ctrl_regs->mipi_userdef_dt.u32;
    user_def.u32 = mipi_ctrl_regs->mipi_user_def.u32;

    user_def_dt.bits.user_def0_dt = (hi_u32)input_data_type;
    user_def_dt.bits.user_def1_dt = (hi_u32)input_data_type;

    user_def.bits.user_def0 = (unsigned short)data_type[0];
    user_def.bits.user_def1 = (unsigned short)data_type[1];

    mipi_ctrl_regs->mipi_userdef_dt.u32 = user_def_dt.u32;
    mipi_ctrl_regs->mipi_user_def.u32 = user_def.u32;
}

void mipi_rx_drv_set_mipi_dol_id(unsigned int devno, data_type_t input_data_type, const short dol_id[])
{
    u_mipi_dol_id_code0 dol_id0;
    u_mipi_dol_id_code1 dol_id1;
    u_mipi_dol_id_code2 dol_id2;
    unsigned int lef, sef1, sef2;
    unsigned int nxt_lef, nxt_sef1, nxt_sef2;
    volatile mipi_ctrl_regs_t *mipi_ctrl_regs = get_mipi_ctrl_regs(devno);

    mipi_rx_unused(input_data_type);
    mipi_rx_unused(dol_id);

    dol_id0.u32 = mipi_ctrl_regs->mipi_dol_id_code0.u32;
    dol_id1.u32 = mipi_ctrl_regs->mipi_dol_id_code1.u32;
    dol_id2.u32 = mipi_ctrl_regs->mipi_dol_id_code2.u32;

    lef = 0x241;
    sef1 = 0x242;
    sef2 = 0x244;

    nxt_lef = 0x251;
    nxt_sef1 = 0x252;
    nxt_sef2 = 0x254;

    dol_id0.bits.id_code_reg0 = lef;
    dol_id0.bits.id_code_reg1 = sef1;
    dol_id1.bits.id_code_reg2 = sef2;

    dol_id1.bits.id_code_reg3 = nxt_lef;
    dol_id2.bits.id_code_reg4 = nxt_sef1;
    dol_id2.bits.id_code_reg5 = nxt_sef2;

    mipi_ctrl_regs->mipi_dol_id_code0.u32 = dol_id0.u32;
    mipi_ctrl_regs->mipi_dol_id_code1.u32 = dol_id1.u32;
    mipi_ctrl_regs->mipi_dol_id_code2.u32 = dol_id2.u32;
}

void mipi_rx_drv_set_mipi_wdr_mode(unsigned int devno, mipi_wdr_mode_t wdr_mode)
{
    u_mipi_ctrl_mode_hs mode_hs;
    u_mipi_ctrl_mode_pixel mode_pixel;
    volatile mipi_ctrl_regs_t *mipi_ctrl_regs = get_mipi_ctrl_regs(devno);

    mode_hs.u32 = mipi_ctrl_regs->mipi_ctrl_mode_hs.u32;
    mode_pixel.u32 = mipi_ctrl_regs->mipi_ctrl_mode_pixel.u32;

    if (wdr_mode == OT_MIPI_WDR_MODE_NONE) {
        mode_pixel.bits.mipi_dol_mode = 0;
    }
    if (wdr_mode == OT_MIPI_WDR_MODE_VC) {
        mode_pixel.bits.mipi_dol_mode = 0;
    } else if (wdr_mode == OT_MIPI_WDR_MODE_DT) {
        mode_hs.bits.user_def_en = 1;
    } else if (wdr_mode == OT_MIPI_WDR_MODE_DOL) {
        mode_pixel.bits.mipi_dol_mode = 1;
    } else {
        // fix sc
    }

    mipi_ctrl_regs->mipi_ctrl_mode_hs.u32 = mode_hs.u32;
    mipi_ctrl_regs->mipi_ctrl_mode_pixel.u32 = mode_pixel.u32;
}

static void mipi_rx_drv_enable_user_define_dt(unsigned int devno, unsigned int enable)
{
    u_mipi_ctrl_mode_hs mode_hs;
    volatile mipi_ctrl_regs_t *mipi_ctrl_regs = get_mipi_ctrl_regs(devno);

    if (mipi_ctrl_regs != NULL) {
        mode_hs.u32 = mipi_ctrl_regs->mipi_ctrl_mode_hs.u32;
        mode_hs.bits.user_def_en = enable;

        mipi_ctrl_regs->mipi_ctrl_mode_hs.u32 = mode_hs.u32;
    }
}

unsigned int mipi_rx_drv_get_phy_data(int phy_id, int lane_id)
{
    volatile u_phy_data_link phy_data_link;
    volatile mipi_rx_phy_cfg_t *mipi_rx_phy_cfg = NULL;
    unsigned int lane_data = 0x0;

    mipi_rx_phy_cfg = get_mipi_rx_phy_regs(phy_id);
    phy_data_link.u32 = mipi_rx_phy_cfg->phy_data_link.u32;

    if (lane_id == 0) { // lane 0
        lane_data = phy_data_link.bits.phy_data0_mipi;
    } else if (lane_id == 1) { // lane 1
        lane_data = phy_data_link.bits.phy_data1_mipi;
    } else if (lane_id == 2) { // lane 2
        lane_data = phy_data_link.bits.phy_data2_mipi;
    } else if (lane_id == 3) { // lane 3
        lane_data = phy_data_link.bits.phy_data3_mipi;
    } else {
        // fix sc
    }

    return lane_data;
}

unsigned int mipi_rx_drv_get_phy_mipi_link_data(int phy_id, int lane_id)
{
    volatile u_phy_ph_mipi_link phy_ph_mipi_link;
    volatile mipi_rx_phy_cfg_t *mipi_rx_phy_cfg = NULL;
    unsigned int lane_data = 0x0;

    mipi_rx_phy_cfg = get_mipi_rx_phy_regs(phy_id);
    phy_ph_mipi_link.u32 = mipi_rx_phy_cfg->phy_ph_mipi_link.u32;

    if (lane_id == 0) { // lane 0
        lane_data = phy_ph_mipi_link.bits.phy_ph0_mipi;
    } else if (lane_id == 1) { // lane 1
        lane_data = phy_ph_mipi_link.bits.phy_ph1_mipi;
    } else if (lane_id == 2) { // lane 2
        lane_data = phy_ph_mipi_link.bits.phy_ph2_mipi;
    } else if (lane_id == 3) { // lane 3
        lane_data = phy_ph_mipi_link.bits.phy_ph3_mipi;
    } else {
        // fix sc
    }

    return lane_data;
}

unsigned int mipi_rx_drv_get_phy_freq(int phy_id)
{
    volatile u_freq_measure freq_measure;
    volatile mipi_rx_phy_cfg_t *mipi_rx_phy_cfg = NULL;
    unsigned int hs_cnt, cil_cnt;

    mipi_rx_phy_cfg = get_mipi_rx_phy_regs(phy_id);
    freq_measure.u32 = mipi_rx_phy_cfg->freq_measure.u32;
    hs_cnt = freq_measure.bits.hs_cnt;
    cil_cnt = freq_measure.bits.cil_cnt;

    if (hs_cnt == 0 || cil_cnt == 0) {
        return 0;
    }

    return hs_cnt * (hi_u32)MIPI_CIL_FREQ / cil_cnt; /* freq: MHZ >= 1MHZ */
}

unsigned int mipi_rx_drv_get_phy_lvds_link_data(int phy_id, int lane_id)
{
    volatile u_phy_data_lvds_link phy_data_lvds_link;
    volatile mipi_rx_phy_cfg_t *mipi_rx_phy_cfg = NULL;
    unsigned int lane_data = 0x0;

    mipi_rx_phy_cfg = get_mipi_rx_phy_regs(phy_id);
    phy_data_lvds_link.u32 = mipi_rx_phy_cfg->phy_data_lvds_link.u32;

    if (lane_id == 0) { // lane 0
        lane_data = phy_data_lvds_link.bits.phy_data0_lvds_hs;
    } else if (lane_id == 1) { // lane 1
        lane_data = phy_data_lvds_link.bits.phy_data1_lvds_hs;
    } else if (lane_id == 2) { // lane 2
        lane_data = phy_data_lvds_link.bits.phy_data2_lvds_hs;
    } else if (lane_id == 3) { // lane 3
        lane_data = phy_data_lvds_link.bits.phy_data3_lvds_hs;
    } else {
        // fix sc
    }

    return lane_data;
}

void mipi_rx_drv_set_data_rate(unsigned int devno, mipi_data_rate_t data_rate)
{
    u_mipi_ctrl_mode_pixel mipi_ctrl_mode_pixel;
    unsigned int mipi_double_pix_en = 0;
    volatile mipi_ctrl_regs_t *mipi_ctrl_regs = get_mipi_ctrl_regs(devno);

    if (data_rate == MIPI_DATA_RATE_X1) {
        mipi_double_pix_en = 0;
    } else if (data_rate == MIPI_DATA_RATE_X2) {
        mipi_double_pix_en = 1;
    } else {
        mipi_rx_err_trace("devno %u, unsupported data_rate:%d should be %d or %d\n",
            devno, (hi_s32)data_rate, (hi_s32)MIPI_DATA_RATE_X1, (hi_s32)MIPI_DATA_RATE_X2);
        return;
    }

    mipi_ctrl_mode_pixel.u32 = mipi_ctrl_regs->mipi_ctrl_mode_pixel.u32;
    mipi_ctrl_mode_pixel.bits.mipi_double_pix_en = mipi_double_pix_en;
    mipi_ctrl_mode_pixel.bits.sync_clear_en = 0x1;
    mipi_ctrl_regs->mipi_ctrl_mode_pixel.u32 = mipi_ctrl_mode_pixel.u32;
}

static void mipi_rx_set_lane_id(unsigned int devno, unsigned int lane_idx, short lane_id)
{
    u_lane_id0_chn lane_id0_chn;
    u_lane_id1_chn lane_id1_chn;

    volatile global_ctrl_regs_t *global_ctrl_regs = get_global_ctrl_regs(devno);

    lane_id0_chn.u32 = global_ctrl_regs->lane_id0_chn.u32;
    lane_id1_chn.u32 = global_ctrl_regs->lane_id1_chn.u32;

    if (devno == 0) {
    } else if (devno == 1) {
        lane_id = lane_id / 2; /* 2: 1, 3 convert to 0, 1 */
    } else if (devno == 2) { /* dev2 */
        lane_id = lane_id - (hi_s16)MIPI_RX_PHY_LANE_NUM;
    } else { /* dev3 */
        lane_id = (lane_id - (hi_s16)MIPI_RX_PHY_LANE_NUM) / 2; /* 2: 1, 3 convert to 0, 1 */
    }

    switch (lane_id) {
        case 0: /* lane 0 */
            lane_id0_chn.bits.lane0_id = lane_idx;
            break;

        case 1: /* lane 1 */
            lane_id0_chn.bits.lane1_id = lane_idx;
            break;

        case 2: /* lane 2 */
            lane_id0_chn.bits.lane2_id = lane_idx;
            break;

        case 3: /* lane 3 */
            lane_id0_chn.bits.lane3_id = lane_idx;
            break;

        case 4: /* lane 4 */
            lane_id1_chn.bits.lane4_id = lane_idx;
            break;

        case 5: /* lane 5 */
            lane_id1_chn.bits.lane5_id = lane_idx;
            break;

        case 6: /* lane 6 */
            lane_id1_chn.bits.lane6_id = lane_idx;
            break;

        case 7: /* lane 7 */
            lane_id1_chn.bits.lane7_id = lane_idx;
            break;

        default:
            break;
    }

    global_ctrl_regs->lane_id0_chn.u32 = lane_id0_chn.u32;
    global_ctrl_regs->lane_id1_chn.u32 = lane_id1_chn.u32;
}

void mipi_rx_drv_set_link_lane_id(unsigned int devno, input_mode_t input_mode, const short *p_lane_id)
{
    unsigned int i;
    unsigned int lane_num;

    if (input_mode == INPUT_MODE_MIPI) {
        lane_num = MIPI_LANE_NUM;
    } else {
        lane_num = LVDS_LANE_NUM;
    }

    for (i = 0; i < lane_num; i++) {
        if (is_valid_id(p_lane_id[i])) {
            mipi_rx_set_lane_id(devno, i, p_lane_id[i]);
        }
    }
}

void mipi_rx_drv_set_mem_cken(unsigned int devno, unsigned int enable)
{
    u_chn0_mem_ctrl chn0_mem_ctrl;
    u_chn1_mem_ctrl chn1_mem_ctrl;
    u_chn2_mem_ctrl chn2_mem_ctrl;
    u_chn3_mem_ctrl chn3_mem_ctrl;
    volatile mipi_rx_sys_regs_t *mipi_rx_sys_regs = NULL;

    mipi_rx_sys_regs = get_mipi_rx_sys_regs();

    switch (devno) {
        case 0: /* chn 0 */
            chn0_mem_ctrl.u32 = mipi_rx_sys_regs->chn0_mem_ctrl.u32;
            chn0_mem_ctrl.bits.chn0_mem_ck_gt = enable;
            mipi_rx_sys_regs->chn0_mem_ctrl.u32 = chn0_mem_ctrl.u32;
            break;
        case 1: /* chn 1 */
            chn1_mem_ctrl.u32 = mipi_rx_sys_regs->chn1_mem_ctrl.u32;
            chn1_mem_ctrl.bits.chn1_mem_ck_gt = enable;
            mipi_rx_sys_regs->chn1_mem_ctrl.u32 = chn1_mem_ctrl.u32;
            break;
        case 2: /* chn 2 */
            chn2_mem_ctrl.u32 = mipi_rx_sys_regs->chn2_mem_ctrl.u32;
            chn2_mem_ctrl.bits.chn2_mem_ck_gt = enable;
            mipi_rx_sys_regs->chn2_mem_ctrl.u32 = chn2_mem_ctrl.u32;
            break;
        case 3: /* chn 3 */
            chn3_mem_ctrl.u32 = mipi_rx_sys_regs->chn3_mem_ctrl.u32;
            chn3_mem_ctrl.bits.chn3_mem_ck_gt = enable;
            mipi_rx_sys_regs->chn3_mem_ctrl.u32 = chn3_mem_ctrl.u32;
            break;
        default:
            break;
    }
}

void mipi_rx_drv_set_clr_cken(unsigned int devno, unsigned int enable)
{
    u_chn0_clr_en chn0_clr_en;
    u_chn1_clr_en chn1_clr_en;
    u_chn2_clr_en chn2_clr_en;
    u_chn3_clr_en chn3_clr_en;
    volatile mipi_rx_sys_regs_t *mipi_rx_sys_regs = NULL;

    mipi_rx_sys_regs = get_mipi_rx_sys_regs();

    switch (devno) {
        case 0: /* chn 0 */
            chn0_clr_en.u32 = mipi_rx_sys_regs->chn0_clr_en.u32;
            chn0_clr_en.bits.chn0_clr_en_lvds = enable;
            chn0_clr_en.bits.chn0_clr_en_align = enable;
            mipi_rx_sys_regs->chn0_clr_en.u32 = chn0_clr_en.u32;
            break;
        case 1: /* chn 1 */
            chn1_clr_en.u32 = mipi_rx_sys_regs->chn1_clr_en.u32;
            chn1_clr_en.bits.chn1_clr_en_lvds = enable;
            chn1_clr_en.bits.chn1_clr_en_align = enable;
            mipi_rx_sys_regs->chn1_clr_en.u32 = chn1_clr_en.u32;
            break;
        case 2: /* chn 2 */
            chn2_clr_en.u32 = mipi_rx_sys_regs->chn2_clr_en.u32;
            chn2_clr_en.bits.chn2_clr_en_lvds = enable;
            chn2_clr_en.bits.chn2_clr_en_align = enable;
            mipi_rx_sys_regs->chn2_clr_en.u32 = chn2_clr_en.u32;
            break;
        case 3: /* chn 3 */
            chn3_clr_en.u32 = mipi_rx_sys_regs->chn3_clr_en.u32;
            chn3_clr_en.bits.chn3_clr_en_lvds = enable;
            chn3_clr_en.bits.chn3_clr_en_align = enable;
            mipi_rx_sys_regs->chn3_clr_en.u32 = chn3_clr_en.u32;
            break;
        default:
            break;
    }
}

/* magic numbers convert to register configs  */
static void mipi_rx_drv_set_phy_en_link(int phy_id, unsigned int lane_bitmap)
{
    u_phy_en_link phy_en_link;
    volatile mipi_rx_phy_cfg_t *mipi_rx_phy_cfg = NULL;

    mipi_rx_phy_cfg = get_mipi_rx_phy_regs(phy_id);
    phy_en_link.u32 = mipi_rx_phy_cfg->phy_en_link.u32;

    if ((lane_bitmap & 0x5U) != 0U) {
        phy_en_link.bits.phy_da_d0_valid = lane_bitmap & 0x1U;
        phy_en_link.bits.phy_da_d2_valid = (lane_bitmap & 0x4U) >> 2U; // 2: shift left by 2 bits
        phy_en_link.bits.phy_d0_term_en = lane_bitmap & 0x1U;
        phy_en_link.bits.phy_d2_term_en = (lane_bitmap & 0x4U) >> 2U; // 2: shift left by 2 bits
        phy_en_link.bits.phy_clk_term_en = 1U;
    }

    if ((lane_bitmap & 0xaU) != 0U) {
        phy_en_link.bits.phy_da_d1_valid = (lane_bitmap & 0x2U) >> 1U; // 1: shift left by 1 bit
        phy_en_link.bits.phy_da_d3_valid = (lane_bitmap & 0x8U) >> 3U; // 3: shift left by 3 bits
        phy_en_link.bits.phy_d1_term_en = (lane_bitmap & 0x2U) >> 1U; // 1: shift left by 1 bit
        phy_en_link.bits.phy_d3_term_en = (lane_bitmap & 0x8U) >> 3U; // 3: shift left by 3 bits
        phy_en_link.bits.phy_clk2_term_en = 1U;
    }

    mipi_rx_phy_cfg->phy_en_link.u32 = phy_en_link.u32;
    mipi_rx_info_trace("MIPIRx Kernel Driver Config mipi_rx_phy_cfg->phy_en_link: %#x\n",
        mipi_rx_phy_cfg->phy_en_link.u32);
}

static void mipi_rx_drv_set_phy_mode(int phy_id, input_mode_t input_mode, unsigned int lane_bitmap)
{
    u_phy_mode_link phy_mode_link;
    volatile mipi_rx_phy_cfg_t *mipi_rx_phy_cfg = NULL;
    const int cmos_en = 0;

    mipi_rx_unused(input_mode);

    mipi_rx_phy_cfg = get_mipi_rx_phy_regs(phy_id);
    mipi_rx_info_trace("mipi_rx_drv_set_phy_mode\n");

    phy_mode_link.u32 = mipi_rx_phy_cfg->phy_mode_link.u32;

    phy_mode_link.bits.phy_rg_en_d = phy_mode_link.bits.phy_rg_en_d | (lane_bitmap & 0xfU);
    phy_mode_link.bits.phy_rg_en_cmos = cmos_en;
    phy_mode_link.bits.phy_rg_en_clk = 1;
    phy_mode_link.bits.phy_rg_mipi_mode = 1;

    if ((lane_bitmap & 0xaU) != 0U) {
        phy_mode_link.bits.phy_rg_en_clk2 = 1;
        phy_mode_link.bits.phy_rg_mipi_mode2 = 1;
    }

    mipi_rx_phy_cfg->phy_mode_link.u32 = phy_mode_link.u32;
    mipi_rx_info_trace("MIPIRx Kernel Driver mipi_rx_phy_cfg->phy_mode_link.u32: %#x\n",
        mipi_rx_phy_cfg->phy_mode_link.u32);
    mipi_rx_info_trace("MIPIRx Kernel Driver phy_mode_link.u32: %#x\n", phy_mode_link.u32);

    uint32_t value = read_reg32((hi_ulong)g_mipi_rx_regs_va + 0x0U);
    mipi_rx_info_trace("MIPIRx Kernel Driver Register 0x0000: %#x\n", value);
}

static void mipi_rx_drv_set_phy_en(unsigned int lane_bitmap)
{
    u_phy_en phy_en;
    volatile mipi_rx_sys_regs_t *mipi_rx_sys_regs = NULL;

    mipi_rx_sys_regs = get_mipi_rx_sys_regs();
    phy_en.u32 = mipi_rx_sys_regs->phy_en.u32;

    if ((lane_bitmap & 0xfU) != 0U) {
        phy_en.bits.phy0_en = 1U;
    }

    if ((lane_bitmap & 0xf0U) != 0U) {
        phy_en.bits.phy1_en = 1U;
    }

    if ((lane_bitmap & 0xf00U) != 0U) {
        phy_en.bits.phy2_en = 1U;
    }

    mipi_rx_sys_regs->phy_en.u32 = phy_en.u32;
}

static void mipi_rx_drv_set_lane_en(unsigned int lane_bitmap, unsigned int lane_mask)
{
    u_lane_en lane_en;
    volatile mipi_rx_sys_regs_t *mipi_rx_sys_regs = NULL;

    mipi_rx_sys_regs = get_mipi_rx_sys_regs();
    lane_en.u32 = mipi_rx_sys_regs->lane_en.u32;
    lane_en.u32 = lane_en.u32 & (~lane_mask);
    lane_en.u32 = lane_en.u32 | (lane_bitmap & lane_mask);
    mipi_rx_sys_regs->lane_en.u32 = lane_en.u32;
}

static void mipi_rx_drv_set_phy_cil_en(unsigned int lane_bitmap, unsigned int enable)
{
    u_phy_cil_ctrl phy_cil_ctrl;
    volatile mipi_rx_sys_regs_t *mipi_rx_sys_regs = NULL;

    mipi_rx_sys_regs = get_mipi_rx_sys_regs();
    phy_cil_ctrl.u32 = mipi_rx_sys_regs->phy_cil_ctrl.u32;

    if ((lane_bitmap & 0xfU) != 0U) {
        phy_cil_ctrl.bits.phycil0_cken = enable;
    }

    if ((lane_bitmap & 0xf0U) != 0U) {
        phy_cil_ctrl.bits.phycil1_cken = enable;
    }

    if ((lane_bitmap & 0xf00U) != 0U) {
        phy_cil_ctrl.bits.phycil2_cken = enable;
    }

    mipi_rx_sys_regs->phy_cil_ctrl.u32 = phy_cil_ctrl.u32;
}

static void mipi_rx_drv_set_phy_cfg_mode(input_mode_t input_mode, unsigned int lane_bitmap)
{
    u_phycfg_mode phycfg_mode;
    volatile mipi_rx_sys_regs_t *mipi_rx_sys_regs = NULL;
    unsigned int cfg_mode;
    unsigned int cfg_mode_sel;

    mipi_rx_sys_regs = get_mipi_rx_sys_regs();
    phycfg_mode.u32 = mipi_rx_sys_regs->phycfg_mode.u32;

    if (input_mode == INPUT_MODE_MIPI) {
        cfg_mode = 0;
        cfg_mode_sel = 0;
    } else if (input_mode == INPUT_MODE_SUBLVDS ||
               input_mode == INPUT_MODE_LVDS ||
               input_mode == INPUT_MODE_HISPI) {
        cfg_mode = 1;
        cfg_mode_sel = 0;
    } else {
        cfg_mode = 2; /* 2: other input_mode */
        cfg_mode_sel = 1;
    }

    if ((lane_bitmap & 0x5U) != 0U) {
        phycfg_mode.bits.phycil0_0_cfg_mode = cfg_mode;
        phycfg_mode.bits.phycil0_cfg_mode_sel = cfg_mode_sel;
    }

    if ((lane_bitmap & 0xaU) != 0U) {
        phycfg_mode.bits.phycil0_1_cfg_mode = cfg_mode;
        phycfg_mode.bits.phycil0_cfg_mode_sel = cfg_mode_sel;
    }

    if ((lane_bitmap & 0x50U) != 0U) {
        phycfg_mode.bits.phycil1_0_cfg_mode = cfg_mode;
        phycfg_mode.bits.phycil1_cfg_mode_sel = cfg_mode_sel;
    }

    if ((lane_bitmap & 0xa0U) != 0U) {
        phycfg_mode.bits.phycil1_1_cfg_mode = cfg_mode;
        phycfg_mode.bits.phycil1_cfg_mode_sel = cfg_mode_sel;
    }

    mipi_rx_sys_regs->phycfg_mode.u32 = phycfg_mode.u32;
}

static void mipi_rx_drv_set_phy_cfg_en(unsigned int lane_bitmap, unsigned int enable)
{
    u_phycfg_en phycfg_en;
    volatile mipi_rx_sys_regs_t *mipi_rx_sys_regs = NULL;

    mipi_rx_sys_regs = get_mipi_rx_sys_regs();
    phycfg_en.u32 = mipi_rx_sys_regs->phycfg_en.u32;

    if ((lane_bitmap & 0xfU) != 0U) {
        phycfg_en.bits.phycil0_cfg_en = enable;
    }

    if ((lane_bitmap & 0xf0U) != 0U) {
        phycfg_en.bits.phycil1_cfg_en = enable;
    }

    if ((lane_bitmap & 0xf00U) != 0U) {
        phycfg_en.bits.phycil2_cfg_en = enable;
    }

    mipi_rx_sys_regs->phycfg_en.u32 = phycfg_en.u32;
}

void mipi_rx_drv_set_phy_config(input_mode_t input_mode, unsigned int lane_bitmap, unsigned int lane_mask)
{
    hi_u32 i;
    unsigned int mask;
    unsigned int phy_lane_bitmap;

    for (i = 0U; i < MIPI_RX_MAX_PHY_NUM; i++) {
        mask = (hi_u32)0xFU << (MIPI_RX_PHY_LANE_NUM * i);
        if ((lane_bitmap & mask) != 0U) {
            phy_lane_bitmap = (lane_bitmap & mask) >> (MIPI_RX_PHY_LANE_NUM * i);
            mipi_rx_drv_set_phy_en_link((hi_s32)i, phy_lane_bitmap);
            mipi_rx_drv_set_phy_mode((hi_s32)i, input_mode, phy_lane_bitmap);
        }
    }

    mipi_rx_drv_set_phy_en(lane_bitmap);
    mipi_rx_drv_set_lane_en(lane_bitmap, lane_mask);
    mipi_rx_drv_set_phy_cil_en(lane_bitmap, 1);
    mipi_rx_drv_set_phy_cfg_mode(input_mode, lane_bitmap);
    mipi_rx_drv_set_phy_cfg_en(lane_bitmap, 1);
}

void mipi_rx_drv_set_lvds_image_rect(unsigned int devno, const img_rect_t *p_img_rect, unsigned int total_lane_num)
{
    volatile lvds_ctrl_regs_t *ctrl_reg_attr = NULL;
    u_lvds_imgsize lvds_img_size;
    u_lvds_crop_start0 crop_start0;
    u_lvds_crop_start1 crop_start1;
    u_lvds_crop_start2 crop_start2;
    u_lvds_crop_start3 crop_start3;
    unsigned int width_per_lane, x_per_lane;

    ctrl_reg_attr = get_lvds_ctrl_regs(devno);

    if (total_lane_num == 0) {
        return;
    }

    width_per_lane = (p_img_rect->width / total_lane_num);
    x_per_lane = (unsigned int) ((hi_u32)p_img_rect->x / total_lane_num);

    lvds_img_size.u32 = ctrl_reg_attr->lvds_imgsize.u32;
    crop_start0.u32 = ctrl_reg_attr->lvds_crop_start0.u32;
    crop_start1.u32 = ctrl_reg_attr->lvds_crop_start1.u32;
    crop_start2.u32 = ctrl_reg_attr->lvds_crop_start2.u32;
    crop_start3.u32 = ctrl_reg_attr->lvds_crop_start3.u32;

    lvds_img_size.bits.lvds_imgwidth_lane = width_per_lane - 1U;
    lvds_img_size.bits.lvds_imgheight = (unsigned int) p_img_rect->height - 1U;

    crop_start0.bits.lvds_start_x0_lane = x_per_lane;
    crop_start0.bits.lvds_start_y0 = (unsigned int) p_img_rect->y;

    crop_start1.bits.lvds_start_x1_lane = x_per_lane;
    crop_start1.bits.lvds_start_y1 = (unsigned int) p_img_rect->y;

    crop_start2.bits.lvds_start_x2_lane = x_per_lane;
    crop_start2.bits.lvds_start_y2 = (unsigned int) p_img_rect->y;

    crop_start3.bits.lvds_start_x3_lane = x_per_lane;
    crop_start3.bits.lvds_start_y3 = (unsigned int) p_img_rect->y;

    ctrl_reg_attr->lvds_imgsize.u32 = lvds_img_size.u32;
    ctrl_reg_attr->lvds_crop_start0.u32 = crop_start0.u32;
    ctrl_reg_attr->lvds_crop_start1.u32 = crop_start1.u32;
    ctrl_reg_attr->lvds_crop_start2.u32 = crop_start2.u32;
    ctrl_reg_attr->lvds_crop_start3.u32 = crop_start3.u32;
}

void mipi_rx_drv_set_lvds_crop_en(unsigned int devno, unsigned int enable)
{
    volatile lvds_ctrl_regs_t *ctrl_reg_attr = NULL;
    u_lvds_ctrl lvds_ctrl;

    ctrl_reg_attr = get_lvds_ctrl_regs(devno);
    if (ctrl_reg_attr == NULL) {
        return;
    }

    lvds_ctrl.u32 = ctrl_reg_attr->lvds_ctrl.u32;

    lvds_ctrl.bits.lvds_crop_en = enable;

    ctrl_reg_attr->lvds_ctrl.u32 = lvds_ctrl.u32;
}

static int mipi_rx_drv_set_lvds_wdr_normal(const lvds_vsync_attr_t *vsync_attr, u_lvds_wdr *lvds_wdr)
{
    if (vsync_attr->sync_type == LVDS_VSYNC_NORMAL) {
        /* SOF-EOF WDR, long exposure frame and short exposure frame has independent sync code */
        lvds_wdr->bits.lvds_wdr_mode = 0x0;
    } else if (vsync_attr->sync_type == LVDS_VSYNC_SHARE) {
        /* SOF-EOF WDR, long exposure frame and short exposure frame share the SOF and EOF */
        lvds_wdr->bits.lvds_wdr_mode = 0x2;
    } else {
        mipi_rx_err_trace("not support vsync type: %u, should be %u or %u.\n",
            (hi_u32)vsync_attr->sync_type, (hi_u32)LVDS_VSYNC_NORMAL, (hi_u32)LVDS_VSYNC_SHARE);
        return -1;
    }
    return 0;
}

static int mipi_rx_drv_set_lvds_wdr_dol(const lvds_vsync_attr_t *vsync_attr, const lvds_fid_attr_t *fid_attr,
    u_lvds_wdr *lvds_wdr, u_lvds_dolscd_hblk *scd_hblk)
{
    /* Sony DOL WDR */
    if (vsync_attr->sync_type == LVDS_VSYNC_NORMAL) {
        /*
         * SAV-EAV WDR, 4 sync code, fid embedded in 4th sync code
         * long exposure fame and short exposure frame has independent sync code
         */
        if (fid_attr->fid_type == LVDS_FID_IN_SAV) {
            lvds_wdr->bits.lvds_wdr_mode = 0x4;
        } else if (fid_attr->fid_type == LVDS_FID_IN_DATA) {
            /*
             * SAV-EAV WDR, 5 sync code(Line Information), fid in the fist DATA,
             * fid in data, line information
             */
            if (fid_attr->output_fil != 0U) {
                /* Frame Information Line is included in the image data */
                lvds_wdr->bits.lvds_wdr_mode = 0xd;
            } else {
                /* Frame Information Line is not included in the image data */
                lvds_wdr->bits.lvds_wdr_mode = 0x6;
            }
        } else {
            mipi_rx_err_trace("not support fid type: %u, should be %u or %u\n",
                (hi_u32)fid_attr->fid_type, (hi_u32)LVDS_FID_IN_SAV, (hi_u32)LVDS_FID_IN_DATA);
            return -1;
        }
    } else if (vsync_attr->sync_type == LVDS_VSYNC_HCONNECT) {
        /*
         * SAV-EAV H-Connection DOL, long exposure frame and short exposure frame
         * share the same SAV EAV, the H-Blank is assigned by the dol_hblank1 and dol_hblank2
         */
        if (fid_attr->fid_type == LVDS_FID_NONE) {
            lvds_wdr->bits.lvds_wdr_mode = 0x5;
        } else {
            mipi_rx_err_trace("not support fid type: %u, should be %u\n",
                (hi_u32)fid_attr->fid_type, (hi_u32)LVDS_FID_NONE);
            return -1;
        }
        scd_hblk->bits.dol_hblank1 = vsync_attr->hblank1;
        scd_hblk->bits.dol_hblank2 = vsync_attr->hblank2;
    } else {
        mipi_rx_err_trace("not support vsync type: %u, should be %u or %u\n",
            (hi_u32)vsync_attr->sync_type, (hi_u32)LVDS_VSYNC_NORMAL, (hi_u32)LVDS_VSYNC_HCONNECT);
        return -1;
    }
    return 0;
}

/* magic num mean wdr mode, convert to register config */
static int mipi_rx_drv_set_lvds_wdr_num(lvds_wdr_mode_t wdr_mode, u_lvds_wdr *lvds_wdr)
{
    /* set the wdr frame number */
    switch (wdr_mode) {
        case OT_LVDS_WDR_MODE_NONE:
            lvds_wdr->bits.lvds_wdr_num = 0; // 0: wdr num
            break;
        case OT_LVDS_WDR_MODE_2F:
        case OT_LVDS_WDR_MODE_DOL_2F:
            lvds_wdr->bits.lvds_wdr_num = 1; // 1: wdr num
            break;

        case OT_LVDS_WDR_MODE_3F:
        case OT_LVDS_WDR_MODE_DOL_3F:
            lvds_wdr->bits.lvds_wdr_num = 2; // 2: wdr num
            break;

        case OT_LVDS_WDR_MODE_4F:
        case OT_LVDS_WDR_MODE_DOL_4F:
            lvds_wdr->bits.lvds_wdr_num = 3; // 3: wdr num
            break;

        default:
            mipi_rx_err_trace("not support WDR_MODE: %u, should be [0, %u)\n",
                (hi_u32)wdr_mode, (hi_u32)OT_LVDS_WDR_MODE_BUTT);
            return -1;
    }
    return 0;
}

int mipi_rx_drv_set_lvds_wdr_mode(unsigned int devno, lvds_wdr_mode_t wdr_mode,
                                  const lvds_vsync_attr_t *vsync_attr, const lvds_fid_attr_t *fid_attr)
{
    int ret;
    volatile lvds_ctrl_regs_t *ctrl_reg_attr = NULL;
    u_lvds_wdr lvds_wdr;
    u_lvds_dolscd_hblk scd_hblk;

    ctrl_reg_attr = get_lvds_ctrl_regs(devno);

    lvds_wdr.u32 = ctrl_reg_attr->lvds_wdr.u32;
    scd_hblk.u32 = ctrl_reg_attr->lvds_dolscd_hblk.u32;

    if (wdr_mode == OT_LVDS_WDR_MODE_NONE) {
        lvds_wdr.bits.lvds_wdr_en = 0;
    } else {
        lvds_wdr.bits.lvds_wdr_en = 1;
    }

    ret = mipi_rx_drv_set_lvds_wdr_num(wdr_mode, &lvds_wdr);
    if (ret < 0) {
        return ret;
    }

    /* set wdr mode */
    if (OT_LVDS_WDR_MODE_2F <= wdr_mode && wdr_mode <= OT_LVDS_WDR_MODE_4F) {
        ret = mipi_rx_drv_set_lvds_wdr_normal(vsync_attr, &lvds_wdr);
        if (ret < 0) {
            return ret;
        }
    } else if (OT_LVDS_WDR_MODE_DOL_2F <= wdr_mode && wdr_mode <= OT_LVDS_WDR_MODE_DOL_4F) {
        ret = mipi_rx_drv_set_lvds_wdr_dol(vsync_attr, fid_attr, &lvds_wdr, &scd_hblk);
        if (ret < 0) {
            return ret;
        }
    } else {
        // fix sc
    }

    ctrl_reg_attr->lvds_wdr.u32 = lvds_wdr.u32;
    ctrl_reg_attr->lvds_dolscd_hblk.u32 = scd_hblk.u32;

    return 0;
}

void mipi_rx_drv_set_lvds_ctrl_mode(unsigned int devno, lvds_sync_mode_t sync_mode,
                                    data_type_t input_data_type,
                                    lvds_bit_endian_t data_endian,
                                    lvds_bit_endian_t sync_code_endian)
{
    volatile lvds_ctrl_regs_t *ctrl_reg_attr = NULL;
    u_lvds_ctrl lvds_ctrl;
    unsigned short raw_type = 0x0U;

    ctrl_reg_attr = get_lvds_ctrl_regs(devno);

    lvds_ctrl.u32 = ctrl_reg_attr->lvds_ctrl.u32;

    switch (input_data_type) {
        case DATA_TYPE_RAW_8BIT:
            raw_type = 0x1;
            break;

        case DATA_TYPE_RAW_10BIT:
            raw_type = 0x2;
            break;

        case DATA_TYPE_RAW_12BIT:
            raw_type = 0x3;
            break;

        case DATA_TYPE_RAW_14BIT:
            raw_type = 0x4;
            break;

        case DATA_TYPE_RAW_16BIT:
            raw_type = 0x5;
            break;

        default:
            break;
    }

    if (raw_type == 0x0U) {
        return;
    }

    lvds_ctrl.bits.lvds_sync_mode = (hi_u32)sync_mode;
    lvds_ctrl.bits.lvds_raw_type = raw_type;
    lvds_ctrl.bits.lvds_pix_big_endian = (hi_u32)data_endian;
    lvds_ctrl.bits.lvds_code_big_endian = (hi_u32)sync_code_endian;

    ctrl_reg_attr->lvds_ctrl.u32 = lvds_ctrl.u32;
}

void mipi_rx_drv_set_lvds_data_rate(unsigned int devno, mipi_data_rate_t data_rate)
{
    u_lvds_output_pix_num lvds_output_pixel_num;
    unsigned int lvds_double_pix_en = 0;
    volatile lvds_ctrl_regs_t *lvds_ctrl_regs = get_lvds_ctrl_regs(devno);

    if (data_rate == MIPI_DATA_RATE_X1) {
        lvds_double_pix_en = 0;
    } else if (data_rate == MIPI_DATA_RATE_X2) {
        lvds_double_pix_en = 0x1;
    } else {
        mipi_rx_err_trace("devno %u, unsupported data_rate:%d should be in %d or %d\n",
            devno, (hi_s32)data_rate, (hi_s32)MIPI_DATA_RATE_X1, (hi_s32)MIPI_DATA_RATE_X2);;
        return;
    }

    lvds_output_pixel_num.u32 = lvds_ctrl_regs->lvds_output_pix_num.u32;
    lvds_output_pixel_num.bits.lvds_double_pix_en = lvds_double_pix_en;
    lvds_ctrl_regs->lvds_output_pix_num.u32 = lvds_output_pixel_num.u32;
}

void mipi_rx_drv_set_dol_line_information(unsigned int devno, lvds_wdr_mode_t wdr_mode)
{
    volatile lvds_ctrl_regs_t *ctrl_reg_attr = NULL;

    ctrl_reg_attr = get_lvds_ctrl_regs(devno);

    if (wdr_mode >= OT_LVDS_WDR_MODE_DOL_2F) {
        ctrl_reg_attr->lvds_li_word0.bits.li_word0_0 = 0x0201;  /* LEF n frame */
        ctrl_reg_attr->lvds_li_word0.bits.li_word0_1 = 0x0211;  /* LEF n + 1 frame */

        ctrl_reg_attr->lvds_li_word1.bits.li_word1_0 = 0x0202;  /* SEF1 n frame */
        ctrl_reg_attr->lvds_li_word1.bits.li_word1_1 = 0x0212;  /* SEF1 n + 1 frame */
    }

    if (wdr_mode >= OT_LVDS_WDR_MODE_DOL_3F) {
        ctrl_reg_attr->lvds_li_word2.bits.li_word2_0 = 0x0204;  /* SEF2 n frame */
        ctrl_reg_attr->lvds_li_word2.bits.li_word2_1 = 0x0214;  /* SEF2 n + 1 frame */
    }

    if (wdr_mode >= OT_LVDS_WDR_MODE_DOL_4F) {
        ctrl_reg_attr->lvds_li_word3.bits.li_word3_0 = 0x0208;  /* SEF3 n frame */
        ctrl_reg_attr->lvds_li_word3.bits.li_word3_1 = 0x0218;  /* SEF3 n + 1 frame */
    }
}

/* magic nums means vc num and sync num */
static void set_lvds_sync_sof_eof(volatile lvds_sync_code_cfg_t *p_sync_code,
    const unsigned short sync_code[][WDR_VC_NUM][SYNC_CODE_NUM], int land_id)
{
    {
        u_lvds_lane_sof_01 lvds_sof_01;
        lvds_sof_01.u32 = p_sync_code->lvds_lane_sof_01.u32;
        lvds_sof_01.bits.lane_sof_0 = sync_code[land_id][0][0]; // Lane: lane_id; Virtual Channel: 0; SyncCode: 0 (SOF)
        lvds_sof_01.bits.lane_sof_1 = sync_code[land_id][1][0]; // Lane: lane_id; Virtual Channel: 1; SyncCode: 0 (SOF)
        p_sync_code->lvds_lane_sof_01.u32 = lvds_sof_01.u32;
    }
    {
        u_lvds_lane_sof_23 lvds_sof_23;
        lvds_sof_23.u32 = p_sync_code->lvds_lane_sof_23.u32;
        lvds_sof_23.bits.lane_sof_2 = sync_code[land_id][2][0]; // Lane: lane_id; Virtual Channel: 2; SyncCode: 0 (SOF)
        lvds_sof_23.bits.lane_sof_3 = sync_code[land_id][3][0]; // Lane: lane_id; Virtual Channel: 3; SyncCode: 0 (SOF)
        p_sync_code->lvds_lane_sof_23.u32 = lvds_sof_23.u32;
    }
    {
        u_lvds_lane_eof_01 lvds_eof_01;
        lvds_eof_01.u32 = p_sync_code->lvds_lane_eof_01.u32;
        lvds_eof_01.bits.lane_eof_0 = sync_code[land_id][0][1]; // Lane: lane_id; Virtual Channel: 0; SyncCode: 1 (EOF)
        lvds_eof_01.bits.lane_eof_1 = sync_code[land_id][1][1]; // Lane: lane_id; Virtual Channel: 1; SyncCode: 1 (EOF)
        p_sync_code->lvds_lane_eof_01.u32 = lvds_eof_01.u32;
    }
    {
        u_lvds_lane_eof_23 lvds_eof_23;
        lvds_eof_23.u32 = p_sync_code->lvds_lane_eof_23.u32;
        lvds_eof_23.bits.lane_eof_2 = sync_code[land_id][2][1]; // Lane: lane_id; Virtual Channel: 2; SyncCode: 1 (EOF)
        lvds_eof_23.bits.lane_eof_3 = sync_code[land_id][3][1]; // Lane: lane_id; Virtual Channel: 3; SyncCode: 1 (EOF)
        p_sync_code->lvds_lane_eof_23.u32 = lvds_eof_23.u32;
    }
}

/* magic nums means vc num and sync num */
static void set_lvds_sync_sol_eol(volatile lvds_sync_code_cfg_t *p_sync_code,
    const unsigned short sync_code[][WDR_VC_NUM][SYNC_CODE_NUM], int land_id)
{
    {
        u_lvds_lane_sol_01 lvds_sol_01;
        lvds_sol_01.u32 = p_sync_code->lvds_lane_sol_01.u32;
        lvds_sol_01.bits.lane_sol_0 = sync_code[land_id][0][2]; // Lane: lane_id; Virtual Channel: 0; SyncCode: 2 (SOL)
        lvds_sol_01.bits.lane_sol_1 = sync_code[land_id][1][2]; // Lane: lane_id; Virtual Channel: 1; SyncCode: 2 (SOL)
        p_sync_code->lvds_lane_sol_01.u32 = lvds_sol_01.u32;
    }
    {
        u_lvds_lane_sol_23 lvds_sol_23;
        lvds_sol_23.u32 = p_sync_code->lvds_lane_sol_23.u32;
        lvds_sol_23.bits.lane_sol_2 = sync_code[land_id][2][2]; // Lane: lane_id; Virtual Channel: 2; SyncCode: 2 (SOL)
        lvds_sol_23.bits.lane_sol_3 = sync_code[land_id][3][2]; // Lane: lane_id; Virtual Channel: 3; SyncCode: 2 (SOL)
        p_sync_code->lvds_lane_sol_23.u32 = lvds_sol_23.u32;
    }
    {
        u_lvds_lane_eol_01 lvds_eol_01;
        lvds_eol_01.u32 = p_sync_code->lvds_lane_eol_01.u32;
        lvds_eol_01.bits.lane_eol_0 = sync_code[land_id][0][3]; // Lane: lane_id; Virtual Channel: 0; SyncCode: 3 (EOL)
        lvds_eol_01.bits.lane_eol_1 = sync_code[land_id][1][3]; // Lane: lane_id; Virtual Channel: 1; SyncCode: 3 (EOL)
        p_sync_code->lvds_lane_eol_01.u32 = lvds_eol_01.u32;
    }
    {
        u_lvds_lane_eol_23 lvds_eol_23;
        lvds_eol_23.u32 = p_sync_code->lvds_lane_eol_23.u32;
        lvds_eol_23.bits.lane_eol_2 = sync_code[land_id][2][3]; // Lane: lane_id; Virtual Channel: 2; SyncCode: 3 (EOL)
        lvds_eol_23.bits.lane_eol_3 = sync_code[land_id][3][3]; // Lane: lane_id; Virtual Channel: 3; SyncCode: 3 (EOL)
        p_sync_code->lvds_lane_eol_23.u32 = lvds_eol_23.u32;
    }
}

static void set_lvds_sync_code(unsigned int devno, int n_frame, unsigned int lane_cnt,
    const short lane_id[LVDS_LANE_NUM], const unsigned short sync_code[][WDR_VC_NUM][SYNC_CODE_NUM])
{
    unsigned int i;
    short lane_idx;
    volatile lvds_sync_code_cfg_t *p_sync_code = NULL;
    volatile lvds_ctrl_regs_t *ctrl_reg_attr = get_lvds_ctrl_regs(devno);

    for (i = 0U; i < lane_cnt; i++) {
        lane_idx = lane_id[i];
        if (lane_idx != -1) {
            if (n_frame == TRUE) {
                p_sync_code = &ctrl_reg_attr->lvds_this_frame_sync_code[i];
            } else {
                p_sync_code = &ctrl_reg_attr->lvds_next_frame_sync_code[i];
            }

            set_lvds_sync_sof_eof(p_sync_code, sync_code, (hi_s32)i);
            set_lvds_sync_sol_eol(p_sync_code, sync_code, (hi_s32)i);
        }
    }
}

void mipi_rx_drv_set_lvds_sync_code(unsigned int devno, unsigned int lane_cnt, const short lane_id[LVDS_LANE_NUM],
                                    const unsigned short sync_code[][WDR_VC_NUM][SYNC_CODE_NUM])
{
    set_lvds_sync_code(devno, TRUE, lane_cnt, lane_id, sync_code);
}

void mipi_rx_drv_set_lvds_nxt_sync_code(unsigned int devno, unsigned int lane_cnt, const short lane_id[LVDS_LANE_NUM],
                                        const unsigned short sync_code[][WDR_VC_NUM][SYNC_CODE_NUM])
{
    set_lvds_sync_code(devno, FALSE, lane_cnt, lane_id, sync_code);
}

static void mipi_rx_drv_set_phy_sync_dct(int phy_id, unsigned int raw_type,
                                         lvds_bit_endian_t code_endian, unsigned int phy_lane_bitmap)
{
    u_phy_sync_dct_link phy_sync_dct_link;
    volatile mipi_rx_phy_cfg_t *mipi_rx_phy_cfg = NULL;

    mipi_rx_phy_cfg = get_mipi_rx_phy_regs(phy_id);
    phy_sync_dct_link.u32 = mipi_rx_phy_cfg->phy_sync_dct_link.u32;

    if ((phy_lane_bitmap & 0x5U) != 0U) {
        phy_sync_dct_link.bits.cil_raw_type0 = raw_type;
        phy_sync_dct_link.bits.cil_code_big_endian0 = (hi_u32)code_endian;
    }

    if ((phy_lane_bitmap & 0xaU) != 0U) {
        phy_sync_dct_link.bits.cil_raw_type1 = raw_type;
        phy_sync_dct_link.bits.cil_code_big_endian1 = (hi_u32)code_endian;
    }

    mipi_rx_phy_cfg->phy_sync_dct_link.u32 = phy_sync_dct_link.u32;
}

static short get_sensor_lane_index(short lane, const short lane_id[LVDS_LANE_NUM])
{
    short i;

    for (i = 0; i < LVDS_LANE_NUM; i++) {
        if (lane_id[i] == lane) {
            break;
        }
    }

    return i;
}

static void mipi_rx_drv_set_lvds_phy_sync_code(int phy_id,
                                               const short lane_id[LVDS_LANE_NUM],
                                               const unsigned short n_sync_code[][WDR_VC_NUM][SYNC_CODE_NUM],
                                               const unsigned short nxt_sync_code[][WDR_VC_NUM][SYNC_CODE_NUM],
                                               unsigned int phy_lane_bitmap)
{
    u_phy_sync_sof0_link phy_sync_sof0_link;
    u_phy_sync_sof1_link phy_sync_sof1_link;
    u_phy_sync_sof2_link phy_sync_sof2_link;
    u_phy_sync_sof3_link phy_sync_sof3_link;
    volatile mipi_rx_phy_cfg_t *mipi_rx_phy_cfg = NULL;
    short sensor_lane_idx;
    short lane;

    mipi_rx_phy_cfg = get_mipi_rx_phy_regs(phy_id);
    phy_sync_sof0_link.u32 = mipi_rx_phy_cfg->phy_sync_sof0_link.u32;
    phy_sync_sof1_link.u32 = mipi_rx_phy_cfg->phy_sync_sof1_link.u32;
    phy_sync_sof2_link.u32 = mipi_rx_phy_cfg->phy_sync_sof2_link.u32;
    phy_sync_sof3_link.u32 = mipi_rx_phy_cfg->phy_sync_sof3_link.u32;

    if ((phy_lane_bitmap & 0x1U) != 0U) {
        lane = 0 + (hi_s16)MIPI_RX_PHY_LANE_NUM * (hi_s16)phy_id;
        sensor_lane_idx = get_sensor_lane_index(lane, lane_id);
        phy_sync_sof0_link.bits.cil_sof0_word4_0 = n_sync_code[sensor_lane_idx][0][0];
        phy_sync_sof0_link.bits.cil_sof1_word4_0 = nxt_sync_code[sensor_lane_idx][0][0];
    }

    if ((phy_lane_bitmap & 0x2U) != 0U) {
        lane = 1 + (hi_s16)MIPI_RX_PHY_LANE_NUM * (hi_s16)phy_id;
        sensor_lane_idx = get_sensor_lane_index(lane, lane_id);
        phy_sync_sof1_link.bits.cil_sof0_word4_1 = n_sync_code[sensor_lane_idx][0][0];
        phy_sync_sof1_link.bits.cil_sof1_word4_1 = nxt_sync_code[sensor_lane_idx][0][0];
    }

    if ((phy_lane_bitmap & 0x4U) != 0U) {
        lane = 2 + (hi_s16)MIPI_RX_PHY_LANE_NUM * (hi_s16)phy_id; /* 2nd lane */
        sensor_lane_idx = get_sensor_lane_index(lane, lane_id);
        phy_sync_sof2_link.bits.cil_sof0_word4_2 = n_sync_code[sensor_lane_idx][0][0];
        phy_sync_sof2_link.bits.cil_sof1_word4_2 = nxt_sync_code[sensor_lane_idx][0][0];
    }

    if ((phy_lane_bitmap & 0x8U) != 0U) {
        lane = 3 + (hi_s16)MIPI_RX_PHY_LANE_NUM * (hi_s16)phy_id; /* 3rd lane */
        sensor_lane_idx = get_sensor_lane_index(lane, lane_id);
        phy_sync_sof3_link.bits.cil_sof0_word4_3 = n_sync_code[sensor_lane_idx][0][0];
        phy_sync_sof3_link.bits.cil_sof1_word4_3 = nxt_sync_code[sensor_lane_idx][0][0];
    }

    mipi_rx_phy_cfg->phy_sync_sof0_link.u32 = phy_sync_sof0_link.u32;
    mipi_rx_phy_cfg->phy_sync_sof1_link.u32 = phy_sync_sof1_link.u32;
    mipi_rx_phy_cfg->phy_sync_sof2_link.u32 = phy_sync_sof2_link.u32;
    mipi_rx_phy_cfg->phy_sync_sof3_link.u32 = phy_sync_sof3_link.u32;
}

void mipi_rx_drv_set_phy_sync_config(const lvds_dev_attr_t *p_attr, unsigned int lane_bitmap,
                                     const unsigned short nxt_sync_code[][WDR_VC_NUM][SYNC_CODE_NUM])
{
    unsigned int raw_type;
    hi_u32 i;
    unsigned int mask;
    unsigned int phy_lane_bitmap;

    switch (p_attr->input_data_type) {
        case DATA_TYPE_RAW_8BIT:
            raw_type = 0x1;
            break;

        case DATA_TYPE_RAW_10BIT:
            raw_type = 0x2;
            break;

        case DATA_TYPE_RAW_12BIT:
            raw_type = 0x3;
            break;

        case DATA_TYPE_RAW_14BIT:
            raw_type = 0x4;
            break;

        case DATA_TYPE_RAW_16BIT:
            raw_type = 0x5;
            break;

        default:
            return;
    }

    for (i = 0U; i < MIPI_RX_MAX_PHY_NUM; i++) {
        mask = (hi_u32)0xFU << (MIPI_RX_PHY_LANE_NUM * i);
        if ((lane_bitmap & mask) != 0U) {
            phy_lane_bitmap = (lane_bitmap & mask) >> (MIPI_RX_PHY_LANE_NUM * i);
            mipi_rx_drv_set_phy_sync_dct((hi_s32)i, raw_type, p_attr->sync_code_endian, phy_lane_bitmap);
            mipi_rx_drv_set_lvds_phy_sync_code((hi_s32)i, p_attr->lane_id,
                p_attr->sync_code, nxt_sync_code, phy_lane_bitmap);
        }
    }
}

unsigned int mipi_rx_drv_get_lane_mask(unsigned int devno, lane_divide_mode_t mode)
{
    unsigned int mask = 0x0;

    switch (mode) {
        case LANE_DIVIDE_MODE_0:
            if (devno == 0) { /* mipi_dev 0 */
                mask = 0xff;
            }
            break;
        case LANE_DIVIDE_MODE_1:
            if (devno == 0) { /* mipi_dev 0 */
                mask = 0xf;
            } else if (devno == 2) { /* mipi_dev 2 */
                mask = 0xf0;
            } else {
                // fix sc
            }
            break;
        case LANE_DIVIDE_MODE_2:
            if (devno == 0) { /* mipi_dev 0 */
                mask = 0xf;
            } else if (devno == 2) { /* mipi_dev 2 */
                mask = 0x50;
            } else if (devno == 3) { /* mipi_dev 3 */
                mask = 0xa0;
            } else {
                // fix sc
            }
            break;
        case LANE_DIVIDE_MODE_3:
            if (devno == 0) { /* mipi_dev 0 */
                mask = 0x5;
            } else if (devno == 1) { /* mipi_dev 1 */
                mask = 0xa;
            } else if (devno == 2) { /* mipi_dev 2 */
                mask = 0x50;
            } else if (devno == 3) { /* mipi_dev 3 */
                mask = 0xa0;
            } else {
                // fix sc
            }
            break;
        default:
            break;
    }

    return mask;
}

/* magic numbers means land_id and mipi_dev */
int mipi_rx_drv_is_lane_valid(unsigned int devno, short lane_id, lane_divide_mode_t mode)
{
    int lane_valid = 0;

    switch (mode) {
        case LANE_DIVIDE_MODE_0:
            if (devno == 0 && (lane_id >= 0 && lane_id <= 7)) { // chn 0, lane 0 to 7
                lane_valid = 1;
            }
            break;
        case LANE_DIVIDE_MODE_1:
            if (devno == 0 && (lane_id >= 0 && lane_id <= 4)) { // chn 0, lane 0 to 4
                lane_valid = 1;
            } else if (devno == 2 && (lane_id >= 4 && lane_id <= 7)) { // chn 2, lane 4 to 7
                lane_valid = 1;
            } else {
                // fix sc
            }
            break;
        case LANE_DIVIDE_MODE_2:
            if (devno == 0 && (lane_id >= 0 && lane_id <= 4)) { // chn 0, lane 0 to 4
                lane_valid = 1;
            } else if (devno == 2 && (lane_id == 4 || lane_id == 6)) { // chn 2, lane 4 to 6
                lane_valid = 1;
            } else if (devno == 3 && (lane_id == 5 || lane_id == 7)) { // chn 3, lane 5 to 7
                lane_valid = 1;
            } else {
                // fix sc
            }
            break;
        case LANE_DIVIDE_MODE_3:
            if (devno == 0 && (lane_id == 0 || lane_id == 2)) { // chn 0, lane 0 to 2
                lane_valid = 1;
            } else if (devno == 1 && (lane_id == 1 || lane_id == 3)) { // chn 1, lane 1 to 3
                lane_valid = 1;
            } else if (devno == 2 && (lane_id == 4 || lane_id == 6)) { // chn 2, lane 4 to 6
                lane_valid = 1;
            } else if (devno == 3 && (lane_id == 5 || lane_id == 7)) { // chn 3, lane 5 to 7
                lane_valid = 1;
            } else {
                // fix sc
            }
            break;
        default:
            break;
    }

    return lane_valid;
}


void mipi_rx_drv_set_hs_mode(lane_divide_mode_t mode)
{
    u_hs_mode_select hs_mode_sel;
    volatile mipi_rx_sys_regs_t *mipi_rx_sys_regs = NULL;
    unsigned int i;
    HI_S32 ret = HI_SUCCESS;

    for (i = 0U; i < MIPI_RX_MAX_PHY_NUM; ++i) {
        // cil中断掩码设置为 1f1f,继承小海思
        mipi_rx_set_cil_int_mask((hi_s32)i, MIPI_CIL_INT_MASK);
    }

    for (i = 0; i < MIPI_RX_MAX_PHY_NUM; i++) {
        mipi_rx_set_phy_rg_ext_en((hi_s32)i, g_phy_mode[mode][i].phy_rg_ext_en);
        mipi_rx_set_phy_rg_ext2_en((hi_s32)i, g_phy_mode[mode][i].phy_rg_ext2_en);
        mipi_rx_set_phy_rg_int_en((hi_s32)i, g_phy_mode[mode][i].phy_rg_int_en);
        mipi_rx_set_phy_rg_drveclk2_enz((hi_s32)i, g_phy_mode[mode][i].phy_rg_drveclk2_enz);
        mipi_rx_set_phy_rg_drveclk_enz((hi_s32)i, g_phy_mode[mode][i].phy_rg_drveclk_enz);
    }

    const hi_u32 mipirx_reset_reg_offset = 0xA58;
    // 复位mipi_sys_rst_com，只能修改Bit 12
    write_reg32(g_isp_subctrl_base_va + mipirx_reset_reg_offset,
        (hi_u32)0x1U << 12U, (hi_u32)0x1U << 12U);
    const hi_u32 mipirx_reset_status_reg_offset = 0x5A58;
    // 回读确认,只对Bit 12 确认
    ret = check_reg(g_isp_subctrl_base_va + mipirx_reset_status_reg_offset,
        (hi_u32)0x1U << 12U, (hi_u32)0x1U << 12U);
    if (ret != HI_SUCCESS) {
        mipi_rx_err_trace("MIPI-RX Unreset Check Failed\n");
    }

    mipi_rx_sys_regs = get_mipi_rx_sys_regs();
    hs_mode_sel.u32 = mipi_rx_sys_regs->hs_mode_select.u32;
    hs_mode_sel.bits.hs_mode = g_hs_mode[mode];
    mipi_rx_sys_regs->hs_mode_select.u32 = hs_mode_sel.u32;
    mipi_rx_info_trace("hs_mode_sel.u32: %#x\n", hs_mode_sel.u32);
    ret = check_reg((unsigned long)g_mipi_rx_regs_va + 0x800U, hs_mode_sel.u32, 0xFU);
    if (ret != 0) {
        mipi_rx_err_trace("Check g_mipi_rx_regs_va + 0x800 Failed\n");
    }

    const hi_u32 mipirx_unreset_reg_offset = 0xA5C;
    // 解复位mipi_sys_rst_com，只能修改Bit 12
    write_reg32(g_isp_subctrl_base_va + mipirx_unreset_reg_offset,
        (hi_u32)0x1U << 12U, (hi_u32)0x1U << 12U);
    // 回读确认,只对Bit 12 确认
    ret = check_reg(g_isp_subctrl_base_va + mipirx_reset_status_reg_offset,
        (hi_u32)0x0U << 12U, (hi_u32)0x1U << 12U);
    if (ret != HI_SUCCESS) {
        mipi_rx_err_trace("MIPI-RX Unreset Check Failed\n");
    }
}

void mipi_rx_drv_set_chn_int_mask(unsigned int devno)
{
    u_mipi_int_msk mipi_int_msk;
    volatile mipi_rx_sys_regs_t *mipi_rx_sys_regs = get_mipi_rx_sys_regs();

    mipi_int_msk.u32 = mipi_rx_sys_regs->mipi_int_msk.u32;

    if (devno == 0) { /* chn 0 */
        mipi_int_msk.bits.int_chn0_mask = 0x1;
    } else if (devno == 1) { /* chn 1 */
        mipi_int_msk.bits.int_chn1_mask = 0x1;
    } else if (devno == 2) { /* chn 2 */
        mipi_int_msk.bits.int_chn2_mask = 0x1;
    } else if (devno == 3) { /* chn 3 */
        mipi_int_msk.bits.int_chn3_mask = 0x1;
    } else {
        // fix sc
    }

    mipi_rx_sys_regs->mipi_int_msk.u32 = mipi_int_msk.u32;
}

void mipi_rx_drv_set_lvds_ctrl_int_mask(unsigned int devno, unsigned int mask)
{
    volatile lvds_ctrl_regs_t *lvds_ctrl_regs = get_lvds_ctrl_regs(devno);

    lvds_ctrl_regs->lvds_ctrl_int_msk.u32 = mask;
}

void mipi_rx_drv_set_mipi_ctrl_int_mask(unsigned int devno, unsigned int mask)
{
    volatile mipi_ctrl_regs_t *mipi_ctrl_regs = get_mipi_ctrl_regs(devno);

    mipi_ctrl_regs->mipi_ctrl_int_msk.u32 = mask;
}

void mipi_rx_drv_set_align_int_mask(unsigned int devno, unsigned int mask)
{
    volatile global_ctrl_regs_t *global_ctrl_regs = get_global_ctrl_regs(devno);

    global_ctrl_regs->align_int_msk.u32 = mask;
    global_ctrl_regs->chn_int_mask.u32 = 0xf;
}

#define MIPI_HS_CLOCK_BIT   16U
#define MIPI_PIX_BIT        0x1000U
#define MIPI_PIX_LVDS_BIT   0x100U
#define MIPI_PIX_MIPI_BIT   0x10U
// slvs-ec模式pclk时钟使能位，暂时全部打开
#define MIPI_SLVS_PHY_CDR   0xFF00000

int mipi_rx_drv_enable_clock(hi_u32 combo_dev, lane_divide_mode_t mode)
{
    mipi_rx_info_trace("combo_dev: %u\n", combo_dev);
    // MIPI PIPE Clock Config & Check
    const hi_u32 mipi_clk_en_reg_offset = 0x348;
    unsigned int value, mask;
    hi_u32 mipi_phy_hs_clock_bit = 0U;
    value = (hi_u32)0x1U << combo_dev;
    mask = (hi_u32)0x1U << combo_dev;
    write_reg32(g_isp_subctrl_base_va + mipi_clk_en_reg_offset, value, mask);
    const hi_u32 mipi_clk_status_reg_offset = 0x5348;
    int ret = check_reg(g_isp_subctrl_base_va + mipi_clk_status_reg_offset, value, mask);
    if (ret != HI_SUCCESS) {
        mipi_rx_err_trace("MIPI PIPE Clock Check Failed\n");
        return ret;
    }

    switch (mode) {
        case LANE_DIVIDE_MODE_0:
            if (combo_dev == 0U) { // chn 0, lane 0 to 7
                mipi_phy_hs_clock_bit = 0xFU; // 0xF 1111b
            }
            break;
        case LANE_DIVIDE_MODE_1:
            if (combo_dev == 0U) { // chn 0, lane 0 to 4
                mipi_phy_hs_clock_bit = 0x3U; // 0x3 0011b
            } else if (combo_dev == 2U) { // chn 2, lane 4 to 7
                mipi_phy_hs_clock_bit = 0xCU; // 0xC 1100b
            } else {
                // fix sc
            }
            break;
        case LANE_DIVIDE_MODE_2:
            if (combo_dev == 0U) { // chn 0, lane 0 to 4
                mipi_phy_hs_clock_bit = 0x3U; // 0x3 0011b
            } else if (combo_dev == 2U) { // chn 2, lane 4 and 6
                mipi_phy_hs_clock_bit = 0x4U; // 0x4 0100b
            } else if (combo_dev == 3U) { // chn 3, lane 5 and 7
                mipi_phy_hs_clock_bit = 0x8U; // 0x8 1000b
            } else {
                // fix sc
            }
            break;
        case LANE_DIVIDE_MODE_3:
            if (combo_dev == 0U) { // chn 0, lane 0 and 2
                mipi_phy_hs_clock_bit = 0x1U; // 0x1 0001b
            } else if (combo_dev == 1U) { // chn 1, lane 1 and 3
                mipi_phy_hs_clock_bit = 0x2U; // 0x2 0010b
            } else if (combo_dev == 2U) { // chn 2, lane 4 and 6
                mipi_phy_hs_clock_bit = 0x4U; // 0x4 0100b
            } else if (combo_dev == 3U) { // chn 3, lane 5 and 7
                mipi_phy_hs_clock_bit = 0x8U; // 0x8 1000b
            } else {
                // fix sc
            }
            break;
        default:
            mipi_rx_err_trace("invalid lane_divide_mode(%d), must be in [%d, %d)\n",
                (hi_s32)mode, (hi_s32)LANE_DIVIDE_MODE_0, (hi_s32)LANE_DIVIDE_MODE_BUTT);
            break;
    }

    // MIPIRx Clock Config & Check
    const hi_u32 mipirx_clk_en_reg_offset = 0x360;
    value = (mipi_phy_hs_clock_bit << MIPI_HS_CLOCK_BIT) | (MIPI_PIX_BIT << combo_dev) |
        (MIPI_PIX_LVDS_BIT << combo_dev) | (MIPI_PIX_MIPI_BIT << combo_dev);
    mask = (mipi_phy_hs_clock_bit << MIPI_HS_CLOCK_BIT) | (MIPI_PIX_BIT << combo_dev) |
        (MIPI_PIX_LVDS_BIT << combo_dev) | (MIPI_PIX_MIPI_BIT << combo_dev);
    write_reg32(g_isp_subctrl_base_va + mipirx_clk_en_reg_offset, value, mask);
    const hi_u32 mipirx_clk_status_reg_offset = 0x5360;
    ret = check_reg(g_isp_subctrl_base_va + mipirx_clk_status_reg_offset, value, mask);
    if (ret != HI_SUCCESS) {
        mipi_rx_err_trace("MIPIRx Clock Check Failed\n");
        return ret;
    }

    return HI_SUCCESS;
}

int mipi_rx_drv_disable_clock(hi_u32 combo_dev, lane_divide_mode_t mode)
{
    mipi_rx_info_trace("combo_dev: %u\n", combo_dev);
    // MIPI PIPE Clock Config & Check
    unsigned int value, mask;
    hi_u32 mipi_phy_hs_clock_bit = 0U;
    value = (hi_u32)0x1U << combo_dev;
    mask = (hi_u32)0x1U << combo_dev;
    const hi_u32 mipi_clk_dis_reg_offset = 0x34C;
    write_reg32(g_isp_subctrl_base_va + mipi_clk_dis_reg_offset, value, mask);
    const hi_u32 mipi_clk_status_reg_offset = 0x5348;
    int ret = check_reg(g_isp_subctrl_base_va + mipi_clk_status_reg_offset, 0x0U, mask);
    if (ret != HI_SUCCESS) {
        mipi_rx_err_trace("MIPI PIPE Clock Check Failed\n");
        return ret;
    }

    switch (mode) {
        case LANE_DIVIDE_MODE_0:
            if (combo_dev == 0U) { // chn 0, lane 0 to 7
                mipi_phy_hs_clock_bit = 0xFU; // 0xF 1111b
            }
            break;
        case LANE_DIVIDE_MODE_1:
            if (combo_dev == 0U) { // chn 0, lane 0 to 4
                mipi_phy_hs_clock_bit = 0x3U; // 0x3 0011b
            } else if (combo_dev == 2U) { // chn 2, lane 4 to 7
                mipi_phy_hs_clock_bit = 0xCU; // 0xC 1100b
            } else {
                // fix sc
            }
            break;
        case LANE_DIVIDE_MODE_2:
            if (combo_dev == 0U) { // chn 0, lane 0 to 4
                mipi_phy_hs_clock_bit = 0x3U; // 0x3 0011b
            } else if (combo_dev == 2U) { // chn 2, lane 4 and 6
                mipi_phy_hs_clock_bit = 0x4U; // 0x4 0100b
            } else if (combo_dev == 3U) { // chn 3, lane 5 and 7
                mipi_phy_hs_clock_bit = 0x8U; // 0x8 1000b
            } else {
                // fix sc
            }
            break;
        case LANE_DIVIDE_MODE_3:
            if (combo_dev == 0U) { // chn 0, lane 0 and 2
                mipi_phy_hs_clock_bit = 0x1U; // 0x1 0001b
            } else if (combo_dev == 1U) { // chn 1, lane 1 and 3
                mipi_phy_hs_clock_bit = 0x2U; // 0x2 0010b
            } else if (combo_dev == 2U) { // chn 2, lane 4 and 6
                mipi_phy_hs_clock_bit = 0x4U; // 0x4 0100b
            } else if (combo_dev == 3U) { // chn 3, lane 5 and 7
                mipi_phy_hs_clock_bit = 0x8U; // 0x8 1000b
            } else {
                // fix sc
            }
            break;
        default:
            mipi_rx_err_trace("invalid lane_divide_mode(%d), must be in [%d, %d)\n",
                (hi_s32)mode, (hi_s32)LANE_DIVIDE_MODE_0, (hi_s32)LANE_DIVIDE_MODE_BUTT);
            break;
    }

    // MIPIRx Clock Config & Check
    const hi_u32 mipirx_clk_dis_reg_offset = 0x364;
    value = (mipi_phy_hs_clock_bit << MIPI_HS_CLOCK_BIT) | (MIPI_PIX_BIT << combo_dev) |
        (MIPI_PIX_LVDS_BIT << combo_dev) | (MIPI_PIX_MIPI_BIT << combo_dev);
    mask = (mipi_phy_hs_clock_bit << MIPI_HS_CLOCK_BIT) | (MIPI_PIX_BIT << combo_dev) |
        (MIPI_PIX_LVDS_BIT << combo_dev) | (MIPI_PIX_MIPI_BIT << combo_dev);
    write_reg32(g_isp_subctrl_base_va + mipirx_clk_dis_reg_offset, value, mask);
    const hi_u32 mipirx_clk_status_reg_offset = 0x5360;
    value = 0x0;
    ret = check_reg(g_isp_subctrl_base_va + mipirx_clk_status_reg_offset, 0x0, mask);
    if (ret != HI_SUCCESS) {
        mipi_rx_err_trace("MIPIRx Clock Check Failed\n");
        return ret;
    }

    return HI_SUCCESS;
}

static hi_u8 sensor_clk_reg_val_freq_map(const sns_clk_freq_t sensor_clk_freq)
{
    hi_u8 reg_val = 0x0;
    switch (sensor_clk_freq) {
        case SENSOR_CLK_74P25MHz:
            reg_val = 0x9;
            break;
        case SENSOR_CLK_72MHz:
            reg_val = 0x7;
            break;
        case SENSOR_CLK_54MHz:
            reg_val = 0x3;
            break;
        case SENSOR_CLK_50MHz:
            reg_val = 0x1;
            break;
        case SENSOR_CLK_24MHz:
            reg_val = 0x5;
            break;
        case SENSOR_CLK_37P125MHz:
            reg_val = 0x8;
            break;
        case SENSOR_CLK_36MHz:
            reg_val = 0x6;
            break;
        case SENSOR_CLK_27MHz:
            reg_val = 0x2;
            break;
        case SENSOR_CLK_25MHz:
            reg_val = 0x0;
            break;
        case SENSOR_CLK_12MHz:
            reg_val = 0x4;
            break;
        default:
            mipi_rx_err_trace("unsupported sensor_clk_freq(%u), should be in [0, %u)\n",
                (hi_u32)sensor_clk_freq, (hi_u32)SENSOR_CLK_FREQ_BUTT);
    }
    mipi_rx_info_trace("sensor clock frequency config, reg_val=0x%x\n", reg_val);
    return reg_val;
}

int sensor_drv_config_clock(sns_clk_cfg_t sns_clk_config)
{
    mipi_rx_info_trace("sns_clk_config clk_source: %u, clk_freq: %u\n",
        (hi_u32)sns_clk_config.clk_source, (hi_u32)sns_clk_config.clk_freq);
    // 配置sensor时钟前，需要保证时钟关闭
    HI_U32 sns_clk_source = sns_clk_config.clk_source;
    // Sensor Clock Disable Check
    const hi_u32 clk_status_reg_offset = 0x5368;
    int ret = check_reg(g_isp_subctrl_base_va + clk_status_reg_offset,
        (hi_u32)0x0U << sns_clk_source, (hi_u32)0x1U << sns_clk_source);
    if (ret != HI_SUCCESS) {
        mipi_rx_info_trace("Sensor Clock Source %u not disable before config clock\n", sns_clk_source);
        mipi_rx_info_trace("disable Sensor clock %u \n", sns_clk_source);
        ret = sensor_drv_disable_clock(sns_clk_source);
        if (ret != HI_SUCCESS) {
            return ret;
        }
    }

    unsigned int reg_value = sensor_clk_reg_val_freq_map(sns_clk_config.clk_freq);
    // Register Define: 0~3 bit -- source 0; 4~7 bit for source 1; 8~11 bit for source 2; 12~15 bit for source 3
    // Every 4 bit correspond to the configuration function of a source port
    const hi_u32 source_cfg_bit_offset = (sns_clk_config.clk_source * 4U);
    reg_value = reg_value << source_cfg_bit_offset;
    unsigned int mask = (hi_u32)0xFU << source_cfg_bit_offset;
    const hi_u32 clk_sel_reg_offset = 0x3418;
    write_reg32(g_isp_subctrl_base_va + clk_sel_reg_offset, reg_value, mask);
    return 0;
}

int sensor_drv_enable_clock(unsigned int sns_clk_source)
{
    mipi_rx_info_trace("sns_clk_source: %u\n", sns_clk_source);
    // Sensor Clock Enable Config
    const hi_u32 clk_enable_reg_offset  = 0x368;
    const unsigned enbale_value = 0x1;
    const unsigned offset = sns_clk_source;
    mipi_set_bit(enbale_value, offset, g_isp_subctrl_base_va + clk_enable_reg_offset);

    // Sensor Clock Enable Check
    const hi_u32 clk_status_reg_offset = 0x5368;
    int ret = check_reg(g_isp_subctrl_base_va + clk_status_reg_offset,
        (hi_u32)0x1U << sns_clk_source, (hi_u32)0x1U << sns_clk_source);
    if (ret != HI_SUCCESS) {
        mipi_rx_err_trace("Sensor Clock Source %u Check Failed\n", sns_clk_source);
        return ret;
    }

    return HI_SUCCESS;
}

int sensor_drv_disable_clock(unsigned int sns_clk_source)
{
    mipi_rx_info_trace("sns_clk_source: %u\n", sns_clk_source);
    // Sensor Clock Disable Config
    const hi_u32 clk_disable_reg_offset = 0x36C;
    const unsigned enbale_value = 0x1;
    const unsigned offset = sns_clk_source;
    mipi_set_bit(enbale_value, offset, g_isp_subctrl_base_va + clk_disable_reg_offset);

    // Sensor Clock Disable Check
    const hi_u32 clk_status_reg_offset = 0x5368;
    int ret = check_reg(g_isp_subctrl_base_va + clk_status_reg_offset,
        (hi_u32)0x0U << sns_clk_source, (hi_u32)0x1U << sns_clk_source);
    if (ret != HI_SUCCESS) {
        mipi_rx_err_trace("Sensor Clock Source %u Check Failed\n", sns_clk_source);
        return ret;
    }

    return HI_SUCCESS;
}

#define MIPI_CORE_RESET_REG_OFFSET              0x0A38U
#define MIPI_CORE_UNRESET_REG_OFFSET            0x0A3CU
#define MIPI_CORE_RESET_STATUS_REG_OFFSET       0x5A38U
#define MIPI_PIX_RESET_RGE_OFFSET               0x0A58U
#define MIPI_PIX_UNRESET_RGE_OFFSET             0x0A5CU
#define MIPI_PIX_RESET_STATUS_REG_OFFEST        0x5A58U
#define MIPI_PIX_RESET_REG_SHIFT_BIT            4U

void mipi_rx_drv_core_reset(hi_u32 combo_dev)
{
    mipi_rx_info_trace("combo_dev: %u\n", combo_dev);
    hi_s32 ret;
    hi_u32 value, mask;
    value = (hi_u32)0x1U << (combo_dev + MIPI_PIX_RESET_REG_SHIFT_BIT);
    mask = (hi_u32)0x1U << (combo_dev + MIPI_PIX_RESET_REG_SHIFT_BIT);
    write_reg32(g_isp_subctrl_base_va + MIPI_PIX_RESET_RGE_OFFSET, value, mask);
    ret = check_reg(g_isp_subctrl_base_va + MIPI_PIX_RESET_STATUS_REG_OFFEST, value, mask);
    if (ret != HI_SUCCESS) {
        mipi_rx_err_trace("combo_dev %u mipi pix reset check failed\n", combo_dev);
    }
    value = (hi_u32)0x1U << combo_dev;
    mask = (hi_u32)0x1U << combo_dev;
    write_reg32(g_isp_subctrl_base_va + MIPI_CORE_RESET_REG_OFFSET, value, mask);
    ret = check_reg(g_isp_subctrl_base_va + MIPI_CORE_RESET_STATUS_REG_OFFSET, value, mask);
    if (ret != HI_SUCCESS) {
        mipi_rx_err_trace("combo_dev %u mipi core reset check failed\n", combo_dev);
    }
}

void mipi_rx_drv_core_unreset(hi_u32 combo_dev)
{
    mipi_rx_info_trace("combo_dev: %u\n", combo_dev);
    hi_s32 ret;
    hi_u32 value, mask;
    value = (hi_u32)0x1U << (combo_dev + MIPI_PIX_RESET_REG_SHIFT_BIT);
    mask = (hi_u32)0x1U << (combo_dev + MIPI_PIX_RESET_REG_SHIFT_BIT);
    write_reg32(g_isp_subctrl_base_va + MIPI_PIX_UNRESET_RGE_OFFSET, value, mask);
    ret = check_reg(g_isp_subctrl_base_va + MIPI_PIX_RESET_STATUS_REG_OFFEST, 0x0U, mask);
    if (ret != HI_SUCCESS) {
        mipi_rx_err_trace("combo_dev %u mipi pix unreset check failed\n", combo_dev);
    }
    value = (hi_u32)0x1U << combo_dev;
    mask = (hi_u32)0x1U << combo_dev;
    write_reg32(g_isp_subctrl_base_va + MIPI_CORE_UNRESET_REG_OFFSET, value, mask);
    ret = check_reg(g_isp_subctrl_base_va + MIPI_CORE_RESET_STATUS_REG_OFFSET, 0x0U, mask);
    if (ret != HI_SUCCESS) {
        mipi_rx_err_trace("combo_dev %u mipi core unreset check failed\n", combo_dev);
    }
}

#define SRST_REQ_ISP_SENSOR         ((hi_u32)0x10U)
#define SRST_REQ_ISP_SENSOR_CTRL    ((hi_u32)0x01U)
int sensor_drv_reset(unsigned int sns_reset_source)
{
    mipi_rx_info_trace("sns_reset_source: %u\n", sns_reset_source);
    // Sensor Reset Config
    unsigned int value, mask;
    const hi_u32 sns_reset_reg_offset = 0xA60;
    // req_isp_sensor 配置
    // 5:4 为管脚复位控制bit位，有两个，SRST_REQ_ISP_SENSOR
    // 3:0为内部用的sensor软复位bit位，有4个，SRST_REQ_ISP_SENSOR_CTRL
    // SRST_REQ_ISP_SENSOR_CTRL只用0和2，对应管脚0和1；因此乘2
    value = (SRST_REQ_ISP_SENSOR << sns_reset_source) | (SRST_REQ_ISP_SENSOR_CTRL << (sns_reset_source * 2U));
    // SRST_REQ_ISP_SENSOR_CTRL只用0和2，对应管脚0和1；因此乘2
    mask =  (SRST_REQ_ISP_SENSOR << sns_reset_source) | (SRST_REQ_ISP_SENSOR_CTRL << (sns_reset_source * 2U));
    write_reg32(g_isp_subctrl_base_va + sns_reset_reg_offset, value, mask);

    // Sensor Reset Check
    const hi_u32 sns_reset_status_reg_offset = 0x5A60;
    int ret = check_reg(g_isp_subctrl_base_va + sns_reset_status_reg_offset, value, mask);
    if (ret != HI_SUCCESS) {
        mipi_rx_err_trace("Sensor Reset Source %u Check Failed\n", sns_reset_source);
        return ret;
    }

    return HI_SUCCESS;
}

int sensor_drv_unreset(unsigned int sns_reset_source)
{
    mipi_rx_info_trace("sns_reset_source: %u\n", sns_reset_source);
    unsigned int value, mask;
    // Sensor Unreset Config
    const hi_u32 sns_unreset_reg_offset = 0xA64;
    // req_isp_sensor 配置
    // 5:4 为管脚复位控制bit位，有两个，SRST_REQ_ISP_SENSOR
    // 3:0为内部用的sensor软复位bit位，有4个，SRST_REQ_ISP_SENSOR_CTRL
    // SRST_REQ_ISP_SENSOR_CTRL只用0和2，对应管脚0和1；因此乘2
    value = (SRST_REQ_ISP_SENSOR << sns_reset_source) | (SRST_REQ_ISP_SENSOR_CTRL << (sns_reset_source * 2U));
    // SRST_REQ_ISP_SENSOR_CTRL只用0和2，对应管脚0和1；因此乘2
    mask =  (SRST_REQ_ISP_SENSOR << sns_reset_source) | (SRST_REQ_ISP_SENSOR_CTRL << (sns_reset_source * 2U));
    write_reg32(g_isp_subctrl_base_va + sns_unreset_reg_offset, value, mask);

    // Sensor Unreset Check
    const hi_u32 sns_reset_status_reg_offset = 0x5A60;
    int ret = check_reg(g_isp_subctrl_base_va + sns_reset_status_reg_offset,
        0x0, mask);
    if (ret != HI_SUCCESS) {
        mipi_rx_err_trace("Sensor Unreset Source %u Check Failed\n", sns_reset_source);
        return ret;
    }

    return HI_SUCCESS;
}

void mipi_rx_drv_get_mipi_imgsize_statis(unsigned int devno, short vc, img_size_t *p_size)
{
    u_mipi_imgsize0_statis mipi_imgsize0_statis;
    u_mipi_imgsize1_statis mipi_imgsize1_statis;
    u_mipi_imgsize2_statis mipi_imgsize2_statis;
    u_mipi_imgsize3_statis mipi_imgsize3_statis;

    volatile mipi_ctrl_regs_t *mipi_ctrl_regs = get_mipi_ctrl_regs(devno);

    if (vc == 0) {
        mipi_imgsize0_statis.u32 = mipi_ctrl_regs->mipi_imgsize0_statis.u32;
        p_size->width = mipi_imgsize0_statis.bits.imgwidth_statis_vc0;
        p_size->height = mipi_imgsize0_statis.bits.imgheight_statis_vc0;
    } else if (vc == 1) {
        mipi_imgsize1_statis.u32 = mipi_ctrl_regs->mipi_imgsize1_statis.u32;
        p_size->width = mipi_imgsize1_statis.bits.imgwidth_statis_vc1;
        p_size->height = mipi_imgsize1_statis.bits.imgheight_statis_vc1;
    } else if (vc == 2) { /* 2: vc2 */
        mipi_imgsize2_statis.u32 = mipi_ctrl_regs->mipi_imgsize2_statis.u32;
        p_size->width = mipi_imgsize2_statis.bits.imgwidth_statis_vc2;
        p_size->height = mipi_imgsize2_statis.bits.imgheight_statis_vc2;
    } else if (vc == 3) { /* 3: vc3 */
        mipi_imgsize3_statis.u32 = mipi_ctrl_regs->mipi_imgsize3_statis.u32;
        p_size->width = mipi_imgsize3_statis.bits.imgwidth_statis_vc3;
        p_size->height = mipi_imgsize3_statis.bits.imgheight_statis_vc3;
    } else {
        // fix sc
    }
}

void mipi_rx_drv_get_lvds_imgsize_statis(unsigned int devno, short vc, img_size_t *p_size)
{
    u_lvds_imgsize0_statis lvds_imgsize0_statis;
    u_lvds_imgsize1_statis lvds_imgsize1_statis;
    u_lvds_imgsize2_statis lvds_imgsize2_statis;
    u_lvds_imgsize3_statis lvds_imgsize3_statis;

    volatile lvds_ctrl_regs_t *lvds_ctrl_regs = get_lvds_ctrl_regs(devno);

    if (vc == 0) {
        lvds_imgsize0_statis.u32 = lvds_ctrl_regs->lvds_imgsize0_statis.u32;
        p_size->width = lvds_imgsize0_statis.bits.lvds_imgwidth0;
        p_size->height = lvds_imgsize0_statis.bits.lvds_imgheight0;
    } else if (vc == 1) {
        lvds_imgsize1_statis.u32 = lvds_ctrl_regs->lvds_imgsize1_statis.u32;
        p_size->width = lvds_imgsize1_statis.bits.lvds_imgwidth1;
        p_size->height = lvds_imgsize1_statis.bits.lvds_imgheight1;
    } else if (vc == 2) { /* 2: vc2 */
        lvds_imgsize2_statis.u32 = lvds_ctrl_regs->lvds_imgsize2_statis.u32;
        p_size->width = lvds_imgsize2_statis.bits.lvds_imgwidth2;
        p_size->height = lvds_imgsize2_statis.bits.lvds_imgheight2;
    } else if (vc == 3) { /* 3: vc3 */
        lvds_imgsize3_statis.u32 = lvds_ctrl_regs->lvds_imgsize3_statis.u32;
        p_size->width = lvds_imgsize3_statis.bits.lvds_imgwidth3;
        p_size->height = lvds_imgsize3_statis.bits.lvds_imgheight3;
    } else {
        // fix sc
    }
}

void mipi_rx_drv_get_lvds_lane_imgsize_statis(unsigned int devno, short lane, img_size_t *p_size)
{
    u_lvds_lane_imgsize_statis lvds_lane_imgsize_statis;

    volatile lvds_ctrl_regs_t *lvds_ctrl_regs = get_lvds_ctrl_regs(devno);

    lvds_lane_imgsize_statis.u32 = lvds_ctrl_regs->lvds_lane_imgsize_statis[lane].u32;
    p_size->width = lvds_lane_imgsize_statis.bits.lane_imgwidth + (hi_u32)1U;
    p_size->height = lvds_lane_imgsize_statis.bits.lane_imgheight;
}

/**
 * @brief: MIPI CIL 原始中断 （0x01f0） 超过上限屏蔽函数
 * @param: phy_id phy设备号， count 中断计数， intr_bit 中断bit位
 * @return : 无
 */
static void mipi_phy_cil_intr_exceed_clear(hi_s32 phy_id, hi_u32 count, hi_u32 intr_bit)
{
    if (count >= MIPI_INTR_COUNT_UPPER_LIMIT) {
        hi_u32 mipi_cil_int_msk_link;
        volatile mipi_rx_phy_cfg_t *mipi_rx_phy_cfg = NULL;

        mipi_rx_err_trace("phy_id(%d): mipi cil interrupt (bit %u) count exceeds upper limit (%u), masked interrupt\n",
            phy_id, intr_bit, MIPI_INTR_COUNT_UPPER_LIMIT);
        mipi_rx_phy_cfg = get_mipi_rx_phy_regs(phy_id);
        mipi_cil_int_msk_link = mipi_rx_phy_cfg->mipi_cil_int_msk_link.u32;
        mipi_cil_int_msk_link &= ~intr_bit;
        mipi_rx_phy_cfg->mipi_cil_int_msk_link.u32 = mipi_cil_int_msk_link;
    }
}

/**
 * @brief: MIPI CRTL 读数据错误 原始中断 （0x12f0） 超过上限屏蔽函数
 * @param: devno 设备号， count 中断计数， intr_bit 中断bit位
 * @return : 无
 */
static void mipi_ctrl_intr_exceed_clear(hi_u32 devno, hi_u32 count, hi_u32 intr_bit)
{
    if (count >= MIPI_INTR_COUNT_UPPER_LIMIT) {
        hi_u32 mipi_ctrl_int_msk;
        volatile mipi_ctrl_regs_t *mipi_ctrl_regs = NULL;

        mipi_rx_err_trace("devno(%u): mipi ctrl interrupt (bit %u) count exceeds upper limit (%u), masked interrupt\n",
            devno, intr_bit, MIPI_INTR_COUNT_UPPER_LIMIT);
        mipi_ctrl_regs = get_mipi_ctrl_regs(devno);
        mipi_ctrl_int_msk = mipi_ctrl_regs->mipi_ctrl_int_msk.u32;
        mipi_ctrl_int_msk &= ~intr_bit;
        mipi_ctrl_regs->mipi_ctrl_int_msk.u32 = mipi_ctrl_int_msk;
    }
}

/**
 * @brief: LVDS CRTL 读数据错误 原始中断 （0x17f0） 超过上限屏蔽函数
 * @param: devno 设备号， count 中断计数， intr_bit 中断bit位
 * @return : 无
 */
static void lvds_ctrl_intr_exceed_clear(hi_u32 devno, hi_u32 count, hi_u32 intr_bit)
{
    if (count >= MIPI_INTR_COUNT_UPPER_LIMIT) {
        hi_u32 lvds_ctrl_int_msk;
        volatile lvds_ctrl_regs_t *lvds_ctrl_regs = NULL;

        mipi_rx_err_trace("devno(%u): lvds ctrl interrupt (bit %u) count exceeds upper limit (%u), masked interrupt\n",
            devno, intr_bit, MIPI_INTR_COUNT_UPPER_LIMIT);
        lvds_ctrl_regs = get_lvds_ctrl_regs(devno);
        lvds_ctrl_int_msk = lvds_ctrl_regs->lvds_ctrl_int_msk.u32;
        lvds_ctrl_int_msk &= ~intr_bit;
        lvds_ctrl_regs->lvds_ctrl_int_msk.u32 = lvds_ctrl_int_msk;
    }
}

/**
 * @brief: MIPI ALIGN 原始中断 （0x18f0） 超过上限屏蔽函数
 * @param: devno 设备号， count 中断计数， intr_bit 中断bit位
 * @return : 无
 */
static void mipi_align_intr_exceed_clear(hi_u32 devno, hi_u32 count, hi_u32 intr_bit)
{
    if (count >= MIPI_INTR_COUNT_UPPER_LIMIT) {
        hi_u32 align_int_msk;
        volatile global_ctrl_regs_t *global_ctrl_regs = NULL;

        mipi_rx_err_trace("devno(%u): mipi align interrupt (bit %u) count exceeds upper limit (%u), masked interrupt\n",
            devno, intr_bit, MIPI_INTR_COUNT_UPPER_LIMIT);
        global_ctrl_regs = get_global_ctrl_regs(devno);
        align_int_msk = global_ctrl_regs->align_int_msk.u32;
        align_int_msk &= ~intr_bit;
        global_ctrl_regs->align_int_msk.u32 = align_int_msk;
    }
}

/**
 * @brief: MIPI CRC 原始中断 （0x1020） 超过上限屏蔽函数
 * @param: devno 设备号， count 中断计数， intr_bit 中断bit位
 * @return : 无
 */
static void mipi_crc_intr_exceed_clear(hi_u32 devno, hi_u32 count, hi_u32 intr_bit)
{
    if (count >= MIPI_INTR_COUNT_UPPER_LIMIT) {
        hi_u32 mipi_crc_intr_msk;
        volatile mipi_ctrl_regs_t *mipi_ctrl_regs = NULL;

        mipi_rx_err_trace("devno(%u): mipi crc interrupt (bit %u) count exceeds upper limit (%u), masked interrupt\n",
            devno, intr_bit, MIPI_INTR_COUNT_UPPER_LIMIT);
        mipi_ctrl_regs = get_mipi_ctrl_regs(devno);
        mipi_crc_intr_msk = mipi_ctrl_regs->mipi_crc_intr_msk.u32;
        mipi_crc_intr_msk &= ~intr_bit;
        mipi_ctrl_regs->mipi_crc_intr_msk.u32 = mipi_crc_intr_msk;
    }
}

static void mipi_rx_phy_cil_int_statis(int phy_id)
{
    unsigned int phy_int_status;

    volatile mipi_rx_phy_cfg_t *mipi_rx_phy_cfg = NULL;

    mipi_rx_phy_cfg = get_mipi_rx_phy_regs(phy_id);
    phy_int_status = mipi_rx_phy_cfg->mipi_cil_int_link.u32;

    if (phy_int_status != 0U) {
        mipi_rx_phy_cfg->mipi_cil_int_raw_link.u32 = 0xffffffffU;

        if ((phy_int_status & (hi_u32)MIPI_ESC_CLK2) != 0U) {
            g_phy_err_int_cnt[phy_id].clk2_fsm_escape_err_cnt++;
            mipi_phy_cil_intr_exceed_clear(phy_id,
                g_phy_err_int_cnt[phy_id].clk2_fsm_escape_err_cnt, MIPI_ESC_CLK2);
        }

        if ((phy_int_status & (hi_u32)MIPI_ESC_CLK) != 0U) {
            g_phy_err_int_cnt[phy_id].clk_fsm_escape_err_cnt++;
            mipi_phy_cil_intr_exceed_clear(phy_id,
                g_phy_err_int_cnt[phy_id].clk_fsm_escape_err_cnt, MIPI_ESC_CLK);
        }

        if ((phy_int_status & (hi_u32)MIPI_ESC_D0) != 0U) {
            g_phy_err_int_cnt[phy_id].d0_fsm_escape_err_cnt++;
            mipi_phy_cil_intr_exceed_clear(phy_id,
                g_phy_err_int_cnt[phy_id].d0_fsm_escape_err_cnt, MIPI_ESC_D0);
        }

        if ((phy_int_status & (hi_u32)MIPI_ESC_D1) != 0U) {
            g_phy_err_int_cnt[phy_id].d1_fsm_escape_err_cnt++;
            mipi_phy_cil_intr_exceed_clear(phy_id,
                g_phy_err_int_cnt[phy_id].d1_fsm_escape_err_cnt, MIPI_ESC_D1);
        }

        if ((phy_int_status & (hi_u32)MIPI_ESC_D2) != 0U) {
            g_phy_err_int_cnt[phy_id].d2_fsm_escape_err_cnt++;
            mipi_phy_cil_intr_exceed_clear(phy_id,
                g_phy_err_int_cnt[phy_id].d2_fsm_escape_err_cnt, MIPI_ESC_D2);
        }

        if ((phy_int_status & (hi_u32)MIPI_ESC_D3) != 0U) {
            g_phy_err_int_cnt[phy_id].d3_fsm_escape_err_cnt++;
            mipi_phy_cil_intr_exceed_clear(phy_id,
                g_phy_err_int_cnt[phy_id].d3_fsm_escape_err_cnt, MIPI_ESC_D3);
        }

        if ((phy_int_status & (hi_u32)MIPI_TIMEOUT_CLK2) != 0U) {
            g_phy_err_int_cnt[phy_id].clk2_fsm_timeout_err_cnt++;
            mipi_phy_cil_intr_exceed_clear(phy_id,
                g_phy_err_int_cnt[phy_id].clk2_fsm_timeout_err_cnt, MIPI_TIMEOUT_CLK2);
        }

        if ((phy_int_status & (hi_u32)MIPI_TIMEOUT_CLK) != 0U) {
            g_phy_err_int_cnt[phy_id].clk_fsm_timeout_err_cnt++;
            mipi_phy_cil_intr_exceed_clear(phy_id,
                g_phy_err_int_cnt[phy_id].clk_fsm_timeout_err_cnt, MIPI_TIMEOUT_CLK);
        }

        if ((phy_int_status & (hi_u32)MIPI_TIMEOUT_D0) != 0U) {
            g_phy_err_int_cnt[phy_id].d0_fsm_timeout_err_cnt++;
            mipi_phy_cil_intr_exceed_clear(phy_id,
                g_phy_err_int_cnt[phy_id].d0_fsm_timeout_err_cnt, MIPI_TIMEOUT_D0);
        }

        if ((phy_int_status & (hi_u32)MIPI_TIMEOUT_D1) != 0U) {
            g_phy_err_int_cnt[phy_id].d1_fsm_timeout_err_cnt++;
            mipi_phy_cil_intr_exceed_clear(phy_id,
                g_phy_err_int_cnt[phy_id].d1_fsm_timeout_err_cnt, MIPI_TIMEOUT_D1);
        }

        if ((phy_int_status & (hi_u32)MIPI_TIMEOUT_D2) != 0U) {
            g_phy_err_int_cnt[phy_id].d2_fsm_timeout_err_cnt++;
            mipi_phy_cil_intr_exceed_clear(phy_id,
                g_phy_err_int_cnt[phy_id].d2_fsm_timeout_err_cnt, MIPI_TIMEOUT_D2);
        }

        if ((phy_int_status & (hi_u32)MIPI_TIMEOUT_D3) != 0U) {
            g_phy_err_int_cnt[phy_id].d3_fsm_timeout_err_cnt++;
            mipi_phy_cil_intr_exceed_clear(phy_id,
                g_phy_err_int_cnt[phy_id].d3_fsm_timeout_err_cnt, MIPI_TIMEOUT_D3);
        }
    }
}

static void mipi_int_statics(unsigned int devno)
{
    unsigned int mipi_ctrl_int;
    volatile mipi_ctrl_regs_t *mipi_ctrl_regs = get_mipi_ctrl_regs(devno);

    mipi_ctrl_int = mipi_ctrl_regs->mipi_ctrl_int.u32;

    if (mipi_ctrl_int != 0U) {
        mipi_ctrl_regs->mipi_ctrl_int_raw.u32 = 0xffffffffU;
    }

    if ((mipi_ctrl_int & (hi_u32)INT_VSYNC) != 0U) {
        g_mipi_err_int_cnt[devno].vsync_cnt++;
    }

    if ((mipi_ctrl_int & (hi_u32)CMD_FIFO_WRITE_ERR) != 0U) {
        g_mipi_err_int_cnt[devno].cmd_fifo_wrerr_cnt++;
        mipi_ctrl_intr_exceed_clear(devno,
            g_mipi_err_int_cnt[devno].cmd_fifo_wrerr_cnt, CMD_FIFO_WRITE_ERR);
    }

    if ((mipi_ctrl_int & (hi_u32)DATA_FIFO_WRITE_ERR) != 0U) {
        g_mipi_err_int_cnt[devno].data_fifo_wrerr_cnt++;
        mipi_ctrl_intr_exceed_clear(devno,
            g_mipi_err_int_cnt[devno].data_fifo_wrerr_cnt, DATA_FIFO_WRITE_ERR);
    }

    if ((mipi_ctrl_int & (hi_u32)CMD_FIFO_READ_ERR) != 0U) {
        g_mipi_err_int_cnt[devno].cmd_fifo_rderr_cnt++;
        mipi_ctrl_intr_exceed_clear(devno,
            g_mipi_err_int_cnt[devno].cmd_fifo_rderr_cnt, CMD_FIFO_READ_ERR);
    }

    if ((mipi_ctrl_int & (hi_u32)DATA_FIFO_READ_ERR) != 0U) {
        g_mipi_err_int_cnt[devno].data_fifo_rderr_cnt++;
        mipi_ctrl_intr_exceed_clear(devno,
            g_mipi_err_int_cnt[devno].data_fifo_rderr_cnt, DATA_FIFO_READ_ERR);
    }
}

static void mipi_crc_intr_count1(unsigned int devno, unsigned int mipi_crc_intr_int)
{
    if ((mipi_crc_intr_int & (hi_u32)MIPI_VC0_CRC_ERR) != 0U) {
        g_mipi_err_int_cnt[devno].vc0_err_crc_cnt++;
        mipi_crc_intr_exceed_clear(devno,
            g_mipi_err_int_cnt[devno].vc0_err_crc_cnt, MIPI_VC0_CRC_ERR);
    }

    if ((mipi_crc_intr_int & (hi_u32)MIPI_VC1_CRC_ERR) != 0U) {
        g_mipi_err_int_cnt[devno].vc1_err_crc_cnt++;
        mipi_crc_intr_exceed_clear(devno,
            g_mipi_err_int_cnt[devno].vc1_err_crc_cnt, MIPI_VC1_CRC_ERR);
    }

    if ((mipi_crc_intr_int & (hi_u32)MIPI_VC2_CRC_ERR) != 0U) {
        g_mipi_err_int_cnt[devno].vc2_err_crc_cnt++;
        mipi_crc_intr_exceed_clear(devno,
            g_mipi_err_int_cnt[devno].vc2_err_crc_cnt, MIPI_VC2_CRC_ERR);
    }

    if ((mipi_crc_intr_int & (hi_u32)MIPI_VC3_CRC_ERR) != 0U) {
        g_mipi_err_int_cnt[devno].vc3_err_crc_cnt++;
        mipi_crc_intr_exceed_clear(devno,
            g_mipi_err_int_cnt[devno].vc3_err_crc_cnt, MIPI_VC3_CRC_ERR);
    }

    if ((mipi_crc_intr_int & (hi_u32)MIPI_VC0_ECC_ERR) != 0U) {
        g_mipi_err_int_cnt[devno].vc0_err_ecc_corrected_cnt++;
        mipi_crc_intr_exceed_clear(devno,
            g_mipi_err_int_cnt[devno].vc0_err_ecc_corrected_cnt, MIPI_VC0_ECC_ERR);
    }

    if ((mipi_crc_intr_int & (hi_u32)MIPI_VC1_ECC_ERR) != 0U) {
        g_mipi_err_int_cnt[devno].vc1_err_ecc_corrected_cnt++;
        mipi_crc_intr_exceed_clear(devno,
            g_mipi_err_int_cnt[devno].vc1_err_ecc_corrected_cnt, MIPI_VC1_ECC_ERR);
    }

    if ((mipi_crc_intr_int & (hi_u32)MIPI_VC2_ECC_ERR) != 0U) {
        g_mipi_err_int_cnt[devno].vc2_err_ecc_corrected_cnt++;
        mipi_crc_intr_exceed_clear(devno,
            g_mipi_err_int_cnt[devno].vc2_err_ecc_corrected_cnt, MIPI_VC2_ECC_ERR);
    }

    if ((mipi_crc_intr_int & (hi_u32)MIPI_VC3_ECC_ERR) != 0U) {
        g_mipi_err_int_cnt[devno].vc3_err_ecc_corrected_cnt++;
        mipi_crc_intr_exceed_clear(devno,
            g_mipi_err_int_cnt[devno].vc3_err_ecc_corrected_cnt, MIPI_VC3_ECC_ERR);
    }

    if ((mipi_crc_intr_int & (hi_u32)MIPI_MULT_ECC_ERR) != 0U) {
        g_mipi_err_int_cnt[devno].err_ecc_double_cnt++;
        mipi_crc_intr_exceed_clear(devno,
            g_mipi_err_int_cnt[devno].err_ecc_double_cnt, MIPI_MULT_ECC_ERR);
    }
}

static void mipi_crc_intr_count2(unsigned int devno, unsigned int mipi_crc_intr_int)
{
    if ((mipi_crc_intr_int & (hi_u32)MIPI_VC0_FRAME_NUM_ERR) != 0U) {
        g_mipi_err_int_cnt[devno].vc0_err_frame_num_cnt++;
        mipi_crc_intr_exceed_clear(devno,
            g_mipi_err_int_cnt[devno].vc0_err_frame_num_cnt, MIPI_VC0_FRAME_NUM_ERR);
    }

    if ((mipi_crc_intr_int & (hi_u32)MIPI_VC1_FRAME_NUM_ERR) != 0U) {
        g_mipi_err_int_cnt[devno].vc1_err_frame_num_cnt++;
        mipi_crc_intr_exceed_clear(devno,
            g_mipi_err_int_cnt[devno].vc1_err_frame_num_cnt, MIPI_VC1_FRAME_NUM_ERR);
    }

    if ((mipi_crc_intr_int & (hi_u32)MIPI_VC2_FRAME_NUM_ERR) != 0U) {
        g_mipi_err_int_cnt[devno].vc2_err_frame_num_cnt++;
        mipi_crc_intr_exceed_clear(devno,
            g_mipi_err_int_cnt[devno].vc2_err_frame_num_cnt, MIPI_VC2_FRAME_NUM_ERR);
    }

    if ((mipi_crc_intr_int & (hi_u32)MIPI_VC3_FRAME_NUM_ERR) != 0U) {
        g_mipi_err_int_cnt[devno].vc3_err_frame_num_cnt++;
        mipi_crc_intr_exceed_clear(devno,
            g_mipi_err_int_cnt[devno].vc3_err_frame_num_cnt, MIPI_VC3_FRAME_NUM_ERR);
    }

    if ((mipi_crc_intr_int & (hi_u32)MIPI_VC0_MISMATCH) != 0U) {
        g_mipi_err_int_cnt[devno].vc0_err_frame_s_e_num_mismatch_cnt++;
        mipi_crc_intr_exceed_clear(devno,
            g_mipi_err_int_cnt[devno].vc0_err_frame_s_e_num_mismatch_cnt, MIPI_VC0_MISMATCH);
    }

    if ((mipi_crc_intr_int & (hi_u32)MIPI_VC1_MISMATCH) != 0U) {
        g_mipi_err_int_cnt[devno].vc1_err_frame_s_e_num_mismatch_cnt++;
        mipi_crc_intr_exceed_clear(devno,
            g_mipi_err_int_cnt[devno].vc1_err_frame_s_e_num_mismatch_cnt, MIPI_VC1_MISMATCH);
    }

    if ((mipi_crc_intr_int & (hi_u32)MIPI_VC2_MISMATCH) != 0U) {
        g_mipi_err_int_cnt[devno].vc2_err_frame_s_e_num_mismatch_cnt++;
        mipi_crc_intr_exceed_clear(devno,
            g_mipi_err_int_cnt[devno].vc2_err_frame_s_e_num_mismatch_cnt, MIPI_VC2_MISMATCH);
    }

    if ((mipi_crc_intr_int & (hi_u32)MIPI_VC3_MISMATCH) != 0U) {
        g_mipi_err_int_cnt[devno].vc3_err_frame_s_e_num_mismatch_cnt++;
        mipi_crc_intr_exceed_clear(devno,
            g_mipi_err_int_cnt[devno].vc3_err_frame_s_e_num_mismatch_cnt, MIPI_VC3_MISMATCH);
    }
}

static void mipi_crc_intr_statics(unsigned int devno)
{
    unsigned int mipi_crc_intr_int;
    volatile mipi_ctrl_regs_t *mipi_ctrl_regs = get_mipi_ctrl_regs(devno);

    mipi_crc_intr_int = mipi_ctrl_regs->mipi_crc_intr_st.u32;

    if (mipi_crc_intr_int != 0U) {
        mipi_ctrl_regs->mipi_crc_intr_raw.u32 = 0xffffffffU;
    }

    mipi_crc_intr_count1(devno, mipi_crc_intr_int);
    mipi_crc_intr_count2(devno, mipi_crc_intr_int);
}

static void lvds_err_int_count1(unsigned int devno, unsigned int lvds_ctrl_int)
{
    if ((lvds_ctrl_int & (hi_u32)LVDS_VSYNC) != 0U) {
        g_lvds_err_int_cnt[devno].lvds_vsync_cnt++;
        lvds_ctrl_intr_exceed_clear(devno,
            g_lvds_err_int_cnt[devno].lvds_vsync_cnt, LVDS_VSYNC);
    }

    if ((lvds_ctrl_int & (hi_u32)CMD_RD_ERR) != 0U) {
        g_lvds_err_int_cnt[devno].cmd_rd_err_cnt++;
        lvds_ctrl_intr_exceed_clear(devno,
            g_lvds_err_int_cnt[devno].cmd_rd_err_cnt, CMD_RD_ERR);
    }

    if ((lvds_ctrl_int & (hi_u32)CMD_WR_ERR) != 0U) {
        g_lvds_err_int_cnt[devno].cmd_wr_err_cnt++;
        lvds_ctrl_intr_exceed_clear(devno,
            g_lvds_err_int_cnt[devno].cmd_wr_err_cnt, CMD_WR_ERR);
    }

    if ((lvds_ctrl_int & (hi_u32)LVDS_POP_ERR) != 0U) {
        g_lvds_err_int_cnt[devno].pop_err_cnt++;
        lvds_ctrl_intr_exceed_clear(devno,
            g_lvds_err_int_cnt[devno].pop_err_cnt, LVDS_POP_ERR);
    }

    if ((lvds_ctrl_int & (hi_u32)LVDS_STAT_ERR) != 0U) {
        g_lvds_err_int_cnt[devno].lvds_state_err_cnt++;
        lvds_ctrl_intr_exceed_clear(devno,
            g_lvds_err_int_cnt[devno].lvds_state_err_cnt, LVDS_STAT_ERR);
    }

    if ((lvds_ctrl_int & (hi_u32)LINK0_READ_ERR) != 0U) {
        g_lvds_err_int_cnt[devno].link0_rd_err_cnt++;
        lvds_ctrl_intr_exceed_clear(devno,
            g_lvds_err_int_cnt[devno].link0_rd_err_cnt, LINK0_READ_ERR);
    }

    if ((lvds_ctrl_int & (hi_u32)LINK1_READ_ERR) != 0U) {
        g_lvds_err_int_cnt[devno].link1_rd_err_cnt++;
        lvds_ctrl_intr_exceed_clear(devno,
            g_lvds_err_int_cnt[devno].link1_rd_err_cnt, LINK1_READ_ERR);
    }

    if ((lvds_ctrl_int & (hi_u32)LINK2_READ_ERR) != 0U) {
        g_lvds_err_int_cnt[devno].link2_rd_err_cnt++;
        lvds_ctrl_intr_exceed_clear(devno,
            g_lvds_err_int_cnt[devno].link2_rd_err_cnt, LINK2_READ_ERR);
    }

    if ((lvds_ctrl_int & (hi_u32)LINK0_WRITE_ERR) != 0U) {
        g_lvds_err_int_cnt[devno].link0_wr_err_cnt++;
        lvds_ctrl_intr_exceed_clear(devno,
            g_lvds_err_int_cnt[devno].link0_wr_err_cnt, LINK0_WRITE_ERR);
    }

    if ((lvds_ctrl_int & (hi_u32)LINK1_WRITE_ERR) != 0U) {
        g_lvds_err_int_cnt[devno].link1_wr_err_cnt++;
        lvds_ctrl_intr_exceed_clear(devno,
            g_lvds_err_int_cnt[devno].link1_wr_err_cnt, LINK1_WRITE_ERR);
    }

    if ((lvds_ctrl_int & (hi_u32)LINK2_WRITE_ERR) != 0U) {
        g_lvds_err_int_cnt[devno].link2_wr_err_cnt++;
        lvds_ctrl_intr_exceed_clear(devno,
            g_lvds_err_int_cnt[devno].link2_wr_err_cnt, LINK2_WRITE_ERR);
    }
}

static void lvds_err_int_count2(unsigned int devno, unsigned int lvds_ctrl_int)
{
    if ((lvds_ctrl_int & (hi_u32)LANE0_SYNC_ERR) != 0U) {
        g_lvds_err_int_cnt[devno].lane0_sync_err_cnt++;
        lvds_ctrl_intr_exceed_clear(devno,
            g_lvds_err_int_cnt[devno].lane0_sync_err_cnt, LANE0_SYNC_ERR);
    }

    if ((lvds_ctrl_int & (hi_u32)LANE1_SYNC_ERR) != 0U) {
        g_lvds_err_int_cnt[devno].lane1_sync_err_cnt++;
        lvds_ctrl_intr_exceed_clear(devno,
            g_lvds_err_int_cnt[devno].lane1_sync_err_cnt, LANE1_SYNC_ERR);
    }

    if ((lvds_ctrl_int & (hi_u32)LANE2_SYNC_ERR) != 0U) {
        g_lvds_err_int_cnt[devno].lane2_sync_err_cnt++;
        lvds_ctrl_intr_exceed_clear(devno,
            g_lvds_err_int_cnt[devno].lane2_sync_err_cnt, LANE2_SYNC_ERR);
    }

    if ((lvds_ctrl_int & (hi_u32)LANE3_SYNC_ERR) != 0U) {
        g_lvds_err_int_cnt[devno].lane3_sync_err_cnt++;
        lvds_ctrl_intr_exceed_clear(devno,
            g_lvds_err_int_cnt[devno].lane3_sync_err_cnt, LANE3_SYNC_ERR);
    }

    if ((lvds_ctrl_int & (hi_u32)LANE4_SYNC_ERR) != 0U) {
        g_lvds_err_int_cnt[devno].lane4_sync_err_cnt++;
        lvds_ctrl_intr_exceed_clear(devno,
            g_lvds_err_int_cnt[devno].lane4_sync_err_cnt, LANE4_SYNC_ERR);
    }

    if ((lvds_ctrl_int & (hi_u32)LANE5_SYNC_ERR) != 0U) {
        g_lvds_err_int_cnt[devno].lane5_sync_err_cnt++;
        lvds_ctrl_intr_exceed_clear(devno,
            g_lvds_err_int_cnt[devno].lane5_sync_err_cnt, LANE5_SYNC_ERR);
    }

    if ((lvds_ctrl_int & (hi_u32)LANE6_SYNC_ERR) != 0U) {
        g_lvds_err_int_cnt[devno].lane6_sync_err_cnt++;
        lvds_ctrl_intr_exceed_clear(devno,
            g_lvds_err_int_cnt[devno].lane6_sync_err_cnt, LANE6_SYNC_ERR);
    }

    if ((lvds_ctrl_int & (hi_u32)LANE7_SYNC_ERR) != 0U) {
        g_lvds_err_int_cnt[devno].lane7_sync_err_cnt++;
        lvds_ctrl_intr_exceed_clear(devno,
            g_lvds_err_int_cnt[devno].lane7_sync_err_cnt, LANE7_SYNC_ERR);
    }
}

static void lvds_int_statics(unsigned int devno)
{
    unsigned int lvds_ctrl_int;
    volatile lvds_ctrl_regs_t *lvds_ctrl_regs = get_lvds_ctrl_regs(devno);

    lvds_ctrl_int = lvds_ctrl_regs->lvds_ctrl_int.u32;

    if (lvds_ctrl_int != 0U) {
        lvds_ctrl_regs->lvds_ctrl_int_raw.u32 = 0xffffffffU;
    }

    lvds_err_int_count1(devno, lvds_ctrl_int);
    lvds_err_int_count2(devno, lvds_ctrl_int);
}

static void align_int_statis(unsigned int devno)
{
    unsigned int align_int;
    volatile global_ctrl_regs_t *global_ctrl_regs = get_global_ctrl_regs(devno);

    align_int = global_ctrl_regs->align_int.u32;

    if (align_int != 0U) {
        global_ctrl_regs->align_int_raw.u32 = 0xffffffffU;
    }

    if ((align_int & (hi_u32)ALIGN_FIFO_FULL_ERR) != 0U) {
        g_align_err_int_cnt[devno].fifo_full_err_cnt++;
        mipi_align_intr_exceed_clear(devno, g_align_err_int_cnt[devno].fifo_full_err_cnt, ALIGN_FIFO_FULL_ERR);
    }

    if ((align_int & (hi_u32)ALIGN_LANE0_ERR) != 0U) {
        g_align_err_int_cnt[devno].lane0_align_err_cnt++;
        mipi_align_intr_exceed_clear(devno, g_align_err_int_cnt[devno].lane0_align_err_cnt, ALIGN_LANE0_ERR);
    }

    if ((align_int & (hi_u32)ALIGN_LANE1_ERR) != 0U) {
        g_align_err_int_cnt[devno].lane1_align_err_cnt++;
        mipi_align_intr_exceed_clear(devno, g_align_err_int_cnt[devno].lane1_align_err_cnt, ALIGN_LANE1_ERR);
    }

    if ((align_int & (hi_u32)ALIGN_LANE2_ERR) != 0U) {
        g_align_err_int_cnt[devno].lane2_align_err_cnt++;
        mipi_align_intr_exceed_clear(devno, g_align_err_int_cnt[devno].lane2_align_err_cnt, ALIGN_LANE2_ERR);
    }

    if ((align_int & (hi_u32)ALIGN_LANE3_ERR) != 0U) {
        g_align_err_int_cnt[devno].lane3_align_err_cnt++;
        mipi_align_intr_exceed_clear(devno, g_align_err_int_cnt[devno].lane3_align_err_cnt, ALIGN_LANE3_ERR);
    }

    if ((align_int & (hi_u32)ALIGN_LANE4_ERR) != 0U) {
        g_align_err_int_cnt[devno].lane4_align_err_cnt++;
        mipi_align_intr_exceed_clear(devno, g_align_err_int_cnt[devno].lane4_align_err_cnt, ALIGN_LANE4_ERR);
    }

    if ((align_int & (hi_u32)ALIGN_LANE5_ERR) != 0U) {
        g_align_err_int_cnt[devno].lane5_align_err_cnt++;
        mipi_align_intr_exceed_clear(devno, g_align_err_int_cnt[devno].lane5_align_err_cnt, ALIGN_LANE5_ERR);
    }

    if ((align_int & (hi_u32)ALIGN_LANE6_ERR) != 0U) {
        g_align_err_int_cnt[devno].lane6_align_err_cnt++;
        mipi_align_intr_exceed_clear(devno, g_align_err_int_cnt[devno].lane6_align_err_cnt, ALIGN_LANE6_ERR);
    }

    if ((align_int & (hi_u32)ALIGN_LANE7_ERR) != 0U) {
        g_align_err_int_cnt[devno].lane7_align_err_cnt++;
        mipi_align_intr_exceed_clear(devno, g_align_err_int_cnt[devno].lane7_align_err_cnt, ALIGN_LANE7_ERR);
    }
}

// MIPI Rx Interrupt Handler Function
static int mipi_rx_interrupt_route(int irq, const void *dev_id)
{
    hi_u32 i;
    volatile mipi_rx_sys_regs_t *mipi_rx_sys_regs = get_mipi_rx_sys_regs();
    volatile global_ctrl_regs_t *global_ctrl_regs = NULL;

    mipi_rx_unused(irq);
    mipi_rx_unused(dev_id);

    for (i = 0U; i < MIPI_RX_MAX_PHY_NUM; i++) {
        mipi_rx_phy_cil_int_statis((hi_s32)i);
    }

    for (i = 0U; i < MIPI_RX_MAX_DEV_NUM; i++) {
        global_ctrl_regs = get_global_ctrl_regs(i);
        if (global_ctrl_regs->chn_int_raw.u32 != 0U) {
        } else {
            continue;
        }

        mipi_int_statics(i);
        mipi_crc_intr_statics(i);
        lvds_int_statics(i);
        align_int_statis(i);
        global_ctrl_regs->chn_int_raw.u32 = 0xffffffffU;
    }

    mipi_rx_sys_regs->mipi_int_raw.u32 = 0xffffffffU;

    return OSAL_IRQ_HANDLED;
}

static int mipi_rx_register_irq(void)
{
    int ret;
    ret = osal_request_irq(g_mipi_rx_irq_num, mipi_rx_interrupt_route, NULL, "MIPI_RX", mipi_rx_interrupt_route);
    if (ret < 0) {
        mipi_rx_err_trace("mipi_rx: failed to register irq.\n");
        return -1;
    }

    return 0;
}

static void mipi_rx_unregister_irq(void)
{
    osal_free_irq(g_mipi_rx_irq_num, mipi_rx_interrupt_route);
}

static int mipi_rx_unreset_req(void)
{
    HI_S32 ret = HI_SUCCESS;
    // mipi 解复位总开关
    const hi_u32 mipirx_unreset_reg_offset = 0xA5C;
    write_reg32(g_isp_subctrl_base_va + mipirx_unreset_reg_offset, 0x3FF1, 0x3FF1);
    // 回读确认
    const hi_u32 mipirx_unreset_status_reg_offset = 0x5A58;
    ret = check_reg(g_isp_subctrl_base_va + mipirx_unreset_reg_offset, 0x0, 0x3FF1);
    if (ret != HI_SUCCESS) {
        mipi_rx_err_trace("MIPI-RX Unreset Check Failed\n");
    }
    return ret;
}

static int mipi_rx_reset_req(void)
{
    HI_S32 ret = HI_SUCCESS;
    // mipi 复位总开关
    const hi_u32 mipirx_unreset_reg_offset = 0xA58;
    write_reg32(g_isp_subctrl_base_va + mipirx_unreset_reg_offset, 0x3FF1, 0x3FF1);
    // 回读确认
    const hi_u32 mipirx_unreset_status_reg_offset = 0x5A58;
    ret = check_reg(g_isp_subctrl_base_va + mipirx_unreset_status_reg_offset, 0x3FF1, 0x3FF1);
    if (ret != HI_SUCCESS) {
        mipi_rx_err_trace("MIPI-RX Reset Check Failed\n");
    }
    return ret;
}

static int mipi_rx_enable_global_clock(void)
{
    HI_S32 ret = HI_SUCCESS;
    // mipi 时钟总开关
    const hi_u32 mipirx_clock_reg_offset = 0x360;
    write_reg32(g_isp_subctrl_base_va + mipirx_clock_reg_offset, 0x7, 0x7);
    // 回读确认
    const hi_u32 mipirx_clock_status_reg_offset = 0x5360;
    ret = check_reg(g_isp_subctrl_base_va + mipirx_clock_status_reg_offset, 0x7, 0x7);
    if (ret != HI_SUCCESS) {
        mipi_rx_err_trace("MIPI-RX clock Check Failed\n");
    }
    return ret;
}

static int mipi_rx_disable_global_clock(void)
{
    HI_S32 ret = HI_SUCCESS;
    // mipi 时钟总开关
    const hi_u32 mipirx_clock_reg_offset = 0x364;
    write_reg32(g_isp_subctrl_base_va + mipirx_clock_reg_offset, 0x7, 0x7);
    // 回读确认
    const hi_u32 mipirx_clock_status_reg_offset = 0x5360;
    ret = check_reg(g_isp_subctrl_base_va + mipirx_clock_status_reg_offset, 0x0, 0x7);
    if (ret != HI_SUCCESS) {
        mipi_rx_err_trace("MIPI-RX clock Check Failed\n");
    }
    return ret;
}

/* magic nums convert to register configs */
// mipi ko 插入时硬件寄存器及初始化操作
// 打开mipi解复位总开关，和cil apb时钟开关
// cil中断掩码设置为 1f1f,继承小海思
int mipi_rx_drv_hw_init(void)
{
    hi_u32 i;
    HI_S32 ret = HI_SUCCESS;
    // mipi 解复位总开关
    ret = mipi_rx_unreset_req();
    if (ret != HI_SUCCESS) {
        mipi_rx_err_trace("MIPI-RX Unreset Check Failed\n");
        goto fail0;
    }

    // mipi cil and apb clock enable
    ret = mipi_rx_enable_global_clock();
    if (ret != HI_SUCCESS) {
        mipi_rx_err_trace("MIPI-RX enable clock Check Failed\n");
        goto fail1;
    }

    /* autodeskew default value */
    for (i = 0U; i < MIPI_RX_MAX_PHY_NUM; ++i) {
        // cil中断掩码设置为 1f1f,继承小海思
        mipi_rx_set_cil_int_mask((hi_s32)i, MIPI_CIL_INT_MASK);
        // 写入默认值 0x0
        mipi_rx_set_phy_skew_link((hi_s32)i, SKEW_LINK);
        // 删除对phy_deskew_cal_link和phy_fsmo_link设置，在启动流程中没有
    }
    return ret;
fail1:
    mipi_rx_reset_req();
fail0:
    return HI_FAILURE;
}

/* magic nums convert to register configs */
int mipi_rx_drv_hw_exit(void)
{
    HI_S32 ret = HI_SUCCESS;
    // mipi 时钟总开关
    ret =  mipi_rx_disable_global_clock();
    if (ret != HI_SUCCESS) {
        mipi_rx_err_trace("MIPI-RX disable clock Check Failed\n");
        return ret;
    }
    // mipi 复位总开关
    ret = mipi_rx_reset_req();
    if (ret != HI_SUCCESS) {
        mipi_rx_err_trace("MIPI-RX Unreset Check Failed\n");
        return ret;
    }
    return ret;
}

int isp_subctrl_reg_remap(void)
{
    if (g_isp_subctrl_base_va == (hi_ulong)NULL) {
        g_isp_subctrl_base_va = (unsigned long)osal_ioremap(ISP_SUBCTRL_BASE_REG, ISP_SUBCTRL_SIZE);
        if (g_isp_subctrl_base_va == (hi_ulong)NULL) {
            mipi_rx_err_trace("remap isp subctrl reg addr fail\n");
            return -1;
        }
    }

    return 0;
}

hi_void isp_subctrl_reg_unremap(void)
{
    if (g_isp_subctrl_base_va != (hi_ulong)NULL) {
        osal_iounmap((void *)g_isp_subctrl_base_va);
        g_isp_subctrl_base_va = (hi_ulong)NULL;
    }

    return;
}

int mipi_rx_drv_init(void)
{
    HI_S32 ret = HI_SUCCESS;

    ret = mipi_rx_register_irq();
    if (ret < 0) {
        mipi_rx_err_trace("mipi_rx_register_irq fail!\n");
        goto fail0;
    }

    ret = mipi_rx_drv_hw_init();
    if (ret != HI_SUCCESS) {
        mipi_rx_err_trace("MIPI-RX hw init Failed\n");
        goto fail1;
    }
    return 0;

fail1:
    mipi_rx_unregister_irq();
fail0:
    return -1;
}

void mipi_rx_drv_exit(void)
{
    (void)mipi_rx_drv_hw_exit();
    mipi_rx_unregister_irq();
}

#ifdef __cplusplus
#if __cplusplus
}

#endif
#endif /* End of #ifdef __cplusplus */
