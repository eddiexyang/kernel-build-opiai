/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: slvs hardware abstract level methods.
 * Author: Hisilicon multimedia software group
 * Create: 2023-03-08
 */
#include "hi_osal.h"
#include "hi_mipi_rx.h"
#include "mipi_rx_hal.h"
#include "slvs_ec_adapt.h"
#include "slvs_hal.h"
#include "slvs_ec_reg.h"
#include "hi_type.h"
#include "mipi_rx_type.h"
#include "securec.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define is_valid_id(id) ((id) != -1)

static unsigned int g_reg_map_flag = 0;
static volatile slvs_regs_type_t *g_slvs_regs_va = NULL;
static unsigned int g_slvs_irq_num   = SLVS_EC0_IRQ;

static slvs_phy_int_state_t     slvs_phy_int_state[SLVS_PHY_NUM];
static slvs_link_int_state_t    slvs_link_int_state[SLVS_MAX_DEV_NUM];
/* function definition */
void slvs_drv_set_irq_num(unsigned int irq_num)
{
    g_slvs_irq_num = irq_num;
}

void proc_show_slvs_phy_info(const osal_proc_entry_t* s)
{
    hi_u32 phy_id;
    int lane_id;

    osal_seq_printf(s, "\n-----SLVS PHY ERROR INFO-----------------------------------------------------------\n");
    osal_seq_printf(s, "%8s"      "%10s"      "%12s"       "%10s"     "%10s" "\n",
                    "PhyIdx",  "LaneIdx", "AFifoAlign", "CodeErr", "DispErr");

    for (phy_id = 0U; phy_id < SLVS_PHY_NUM; phy_id++) {
        for (lane_id = 0; lane_id < SLVS_LANE_NUM; lane_id++) {
            osal_seq_printf(s, "%8u" "%10d" "%12u" "%10u" "%10u" "\n",
                            phy_id,
                            lane_id + SLVS_LANE_NUM * (int)phy_id,
                            slvs_phy_int_state[phy_id].afifo_align_cnt[lane_id],
                            slvs_phy_int_state[phy_id].code_err_cnt[lane_id],
                            slvs_phy_int_state[phy_id].disp_err_cnt[lane_id]);
        }
    }
}

void proc_show_slvs_dev_err_info(const osal_proc_entry_t* s)
{
    hi_u32 dev_index;
    unsigned int devno;
    osal_seq_printf(s, "\n-----SLVS DEV ERROR INFO-----------------------------------------------------------\n");
    osal_seq_printf(s, "%8s"      "%11s"       "%12s"        "%8s"     "%16s"
                    "%16s"          "%16s"         "%10s" "\n",
                    "Devno",  "HeaderCRC", "PayloadCRC", "EccErr", "DataFifoWrite",
                    "DataFifoRead", "CmdFifoFull", "SkewErr");

    for (devno = SLVS_DEV_NUM_START; devno < SLVS_DEV_NUM_START + SLVS_MAX_DEV_NUM; devno++) {
        dev_index = devno - SLVS_DEV_NUM_START;
        osal_seq_printf(s, "%8u" "%11u" "%12u" "%8u" "%16u" "%16u" "%16u" "%10u" "\n",
                        devno,
                        slvs_link_int_state[dev_index].header_crc_err_cnt,
                        slvs_link_int_state[dev_index].payload_crc_err_cnt,
                        slvs_link_int_state[dev_index].ecc_err_cnt,
                        slvs_link_int_state[dev_index].data_fifo_w_err_cnt,
                        slvs_link_int_state[dev_index].data_fifo_r_err_cnt,
                        slvs_link_int_state[dev_index].cmd_fifo_full_err_cnt,
                        slvs_link_int_state[dev_index].skew_err_cnt);
    }
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

static volatile slvs_link_regs_t *get_slvs_link_regs_by_dev(unsigned int devno)
{
    if ((devno == SLVS_DEV_NUM_START) || (devno == SLVS_DEV_NUM_START + 1U)) {
        return &g_slvs_regs_va->slvs_phy_link_regs[0].slvs_link_regs[devno - SLVS_DEV_NUM_START];
    } else if ((devno == SLVS_DEV_NUM_START + 2U) || (devno == SLVS_DEV_NUM_START + 3U)) { /* 2/3:dev start */
        return &g_slvs_regs_va->slvs_phy_link_regs[1].slvs_link_regs[(devno -
            2U - SLVS_DEV_NUM_START)]; /* 2:devstart */
    } else {
        mipi_rx_err_trace("get_slvs_link_regs_by_dev err devno %u, should be in [%d, %d]\n",
            devno, SLVS_DEV_NUM_START, SLVS_DEV_NUM_START + 3U);    /* 3:dev end */
        return NULL;
    }
}

static volatile slvs_phy_regs_t *get_slvs_phy_regs_by_phy(hi_u32 phy_id)
{
    MIPI_CHECK_PHY_ID_RETURN_NULL(phy_id);
    return &g_slvs_regs_va->slvs_phy_link_regs[phy_id].slvs_phy_regs;
}

static volatile slvs_sys_regs_t *get_slvs_sys_regs(HI_VOID)
{
    return &g_slvs_regs_va->slvs_sys_regs;
}

static volatile slvs_phy_regs_t *get_slvs_phy_regs_by_dev(unsigned int devno)
{
    if ((devno == SLVS_DEV_NUM_START) || (devno == SLVS_DEV_NUM_START + 1U)) {
        return &g_slvs_regs_va->slvs_phy_link_regs[0].slvs_phy_regs;
    } else if ((devno == SLVS_DEV_NUM_START + 2U) || (devno == SLVS_DEV_NUM_START + 3U)) { /* 2/3:dev start */
        return &g_slvs_regs_va->slvs_phy_link_regs[1].slvs_phy_regs;
    } else {
        mipi_rx_err_trace("get_slvs_phy_regs_by_dev err devno %u, should be in [%u, %u]\n",
            devno, SLVS_DEV_NUM_START, SLVS_DEV_NUM_START + 3U);    /* 3:dev end */
        return NULL;
    }
}

void slvs_ec_drv_set_regs(slvs_regs_type_t *regs)
{
    g_slvs_regs_va = regs;
}

#define SLVS_PIX_RESET_BIT 0x400U
#define SLVS_HS_RESET_BIT 0x100U

void slvs_drv_core_reset(hi_u32 combo_dev)
{
    mipi_rx_info_trace("combo_dev: %u\n", combo_dev);
    hi_s32 ret;
    hi_u32 value, mask;
    hi_ulong isp_subctrl_base_va;

    isp_subctrl_base_va = get_isp_subctrl_base();

    // combo_dev 在之前的函数已做了校验，确保位运算不会溢出
    value = ((hi_u32)SLVS_PIX_RESET_BIT << combo_dev) | ((hi_u32)SLVS_HS_RESET_BIT << combo_dev);
    mask = ((hi_u32)SLVS_PIX_RESET_BIT << combo_dev) | ((hi_u32)SLVS_HS_RESET_BIT << combo_dev);
    write_reg32(isp_subctrl_base_va + SLVS_EC_RESET_REG_OFFSET, value, mask);
    ret = check_reg(isp_subctrl_base_va + SLVS_EC_RESET_STATUS_REG_OFFSET, value, mask);
    if (ret != HI_SUCCESS) {
        mipi_rx_err_trace("combo_dev %u slvs-ec reset check failed\n", combo_dev);
    }
}

void slvs_drv_core_unreset(hi_u32 combo_dev)
{
    mipi_rx_info_trace("combo_dev: %u\n", combo_dev);
    hi_s32 ret;
    hi_u32 value, mask;
    hi_ulong isp_subctrl_base_va;

    isp_subctrl_base_va = get_isp_subctrl_base();

    // combo_dev 在之前的函数已做了校验，确保位运算不会溢出
    value = ((hi_u32)SLVS_PIX_RESET_BIT << combo_dev) | ((hi_u32)SLVS_HS_RESET_BIT << combo_dev);
    mask = ((hi_u32)SLVS_PIX_RESET_BIT << combo_dev) | ((hi_u32)SLVS_HS_RESET_BIT << combo_dev);
    write_reg32(isp_subctrl_base_va + SLVS_EC_UNRESET_REG_OFFSET, value, mask);
    ret = check_reg(isp_subctrl_base_va + SLVS_EC_RESET_STATUS_REG_OFFSET, 0x0U, mask);
    if (ret != HI_SUCCESS) {
        mipi_rx_err_trace("combo_dev %u slvs-ec reset check failed\n", combo_dev);
    }
}

/**
 * @brief: slvs-ec使能时钟
 *  1911 slvs-ec 没有到每个dev的时钟寄存器配置
 *  为保留和小海思一样的接口和调用流程，这里直接返回成功
 *  输入参数已在上一层函数中做了校验
 * @param: combo_dev 号
 * @return : 使能时钟成功为0，使能时钟失败为非0
 */
int slvs_drv_enable_clock(hi_u32 combo_dev)
{
    mipi_rx_info_trace("Kernel slvs-ec enable clock combo_dev: %u\n", combo_dev);
    return HI_SUCCESS;
}

/**
 * @brief: slvs-ec关闭时钟
 *  1911 slvs-ec 没有到每个dev的时钟寄存器配置
 *  为保留和小海思一样的接口和调用流程，这里直接返回成功
 *  输入参数已在上一层函数中做了校验
 * @param: combo_dev 号
 * @return : 关闭时钟成功为0，关闭时钟失败为非0
 */
int slvs_drv_disable_clock(hi_u32 combo_dev)
{
    mipi_rx_info_trace("Kernel slvs-ec disable clock combo_dev: %u\n", combo_dev);
    return HI_SUCCESS;
}

void slvs_drv_cdr_en(hi_u32 phy_id, hi_u32 en)
{
    U_PHY_CDR_EN phy_cdr_en;
    U_PHY_CDR_CTRL0 phy_cdr_ctrl0;
    U_PHY_CDR_CTRL1 phy_cdr_ctrl1;

    volatile slvs_phy_regs_t *slvs_phy_regs = get_slvs_phy_regs_by_phy(phy_id);

    if (slvs_phy_regs != NULL) {
        phy_cdr_en.u32 = slvs_phy_regs->PHY_CDR_EN.u32;
        phy_cdr_en.bits.phy_rg_cdr_en = en;
        slvs_phy_regs->PHY_CDR_EN.u32 = phy_cdr_en.u32;

        phy_cdr_ctrl0.u32 = 0x33F11010;
        slvs_phy_regs->PHY_CDR_CTRL0.u32 = phy_cdr_ctrl0.u32;

        phy_cdr_ctrl1.u32 = 0x0;
        slvs_phy_regs->PHY_CDR_CTRL1.u32 = phy_cdr_ctrl1.u32;

        mipi_rx_info_trace("phy_id:%u, U_PHY_CDR_EN %x", phy_id, slvs_phy_regs->PHY_CDR_EN.u32);
        mipi_rx_info_trace("phy_id:%u, U_PHY_CDR_CTRL0 %x", phy_id, slvs_phy_regs->PHY_CDR_CTRL0.u32);
        mipi_rx_info_trace("phy_id:%u, U_PHY_CDR_CTRL1 %x", phy_id, slvs_phy_regs->PHY_CDR_CTRL1.u32);
    }
}

void slvs_drv_phy_eq_ctrl(hi_u32 phy_id, unsigned int eq)
{
    U_PHY_EQ_CTRL0 phy_eq_ctrl0;
    U_PHY_EQ_CTRL1 phy_eq_ctrl1;
    U_PHY_EQ_CTRL2 phy_eq_ctrl2;

    volatile slvs_phy_regs_t *slvs_phy_regs = get_slvs_phy_regs_by_phy(phy_id);

    if (slvs_phy_regs != NULL) {
        phy_eq_ctrl0.u32 = slvs_phy_regs->PHY_EQ_CTRL0.u32;
        phy_eq_ctrl0.bits.phy_rg_eq_ctrl0 = eq;
        phy_eq_ctrl0.bits.phy_rg_eq_ctrl1 = eq;
        phy_eq_ctrl0.bits.phy_rg_eq_ctrl2 = eq;
        phy_eq_ctrl0.bits.phy_rg_eq_ctrl3 = eq;
        slvs_phy_regs->PHY_EQ_CTRL0.u32 = phy_eq_ctrl0.u32;

        phy_eq_ctrl1.u32 = slvs_phy_regs->PHY_EQ_CTRL1.u32;
        phy_eq_ctrl1.bits.phy_rg_eq_ctrl4 = eq;
        phy_eq_ctrl1.bits.phy_rg_eq_ctrl5 = eq;
        phy_eq_ctrl1.bits.phy_rg_eq_ctrl6 = eq;
        phy_eq_ctrl1.bits.phy_rg_eq_ctrl7 = eq;
        slvs_phy_regs->PHY_EQ_CTRL1.u32 = phy_eq_ctrl1.u32;

        phy_eq_ctrl2.u32 = 0x040029b2;
        slvs_phy_regs->PHY_EQ_CTRL2.u32 = phy_eq_ctrl2.u32;

        mipi_rx_info_trace("phy_id:%u, U_PHY_EQ_CTRL0 %x", phy_id, slvs_phy_regs->PHY_EQ_CTRL0.u32);
        mipi_rx_info_trace("phy_id:%u, U_PHY_EQ_CTRL1 %x", phy_id, slvs_phy_regs->PHY_EQ_CTRL1.u32);
        mipi_rx_info_trace("phy_id:%u, U_PHY_EQ_CTRL2 %x", phy_id, slvs_phy_regs->PHY_EQ_CTRL2.u32);
    }
}

void slvs_drv_phy_ctrl_test(unsigned int devno, unsigned int value)
{
    U_PHY_CTRL_TEST phy_ctrl_test;

    volatile slvs_phy_regs_t *slvs_phy_regs = get_slvs_phy_regs_by_dev(devno);

    if (slvs_phy_regs != NULL) {
        phy_ctrl_test.u32 = value;
        slvs_phy_regs->PHY_CTRL_TEST.u32 = phy_ctrl_test.u32;
        mipi_rx_info_trace("devno:%u, U_PHY_CTRL_TEST:%x", devno, slvs_phy_regs->PHY_CTRL_TEST.u32);
    }
}

void slvs_drv_phy_pdac_ldo(hi_u32 phy_id, unsigned int value)
{
    U_PHY_PDAC_LDO phy_pdac_ldo;

    volatile slvs_phy_regs_t *slvs_phy_regs = get_slvs_phy_regs_by_phy(phy_id);

    if (slvs_phy_regs != NULL) {
        phy_pdac_ldo.u32 = value;
        slvs_phy_regs->PHY_PDAC_LDO.u32 = phy_pdac_ldo.u32;
        mipi_rx_info_trace("phy_id:%u, U_PHY_CTRL_TEST %x", phy_id, slvs_phy_regs->PHY_PDAC_LDO.u32);
    }
}

hi_u32 slvs_drv_is_lane_valid(unsigned int devno, short lane)
{
    hi_u32 lane_valid = 0x0;

    switch (devno) {
        case (SLVS_DEV_NUM_START + 0U):
            if (lane >= 0 && lane <= 7) { /* 0:start 7:end */
                lane_valid = 0x1;
            }
            break;

        case (SLVS_DEV_NUM_START + 1U):
            if (lane >= 0 && lane <= 7) { /* 0:start 7:end */
                lane_valid = 0x1;
            }
            break;

        case (SLVS_DEV_NUM_START + 2U): /* 2:dev offset */
            if (lane >= 8 && lane <= 15) { /* 8:start 15:end */
                lane_valid = 0x1;
            }
            break;

        case (SLVS_DEV_NUM_START + 3U): /* 3:dev offset */
            if (lane >= 8 && lane <= 15) { /* 8:start 15:end */
                lane_valid = 0x1;
            }
            break;

        default:
            break;
    }

    return lane_valid;
}

void slvs_drv_set_lane_num(unsigned int devno, unsigned int lane_num)
{
    U_LINK_CTRL link_ctrl;
    volatile slvs_link_regs_t *slvs_link_regs = get_slvs_link_regs_by_dev(devno);
    if (slvs_link_regs != NULL) {
        link_ctrl.u32 = slvs_link_regs->LINK_CTRL.u32;
        if (lane_num == 0) {
            link_ctrl.bits.link_lane_num = 7U;
        } else {
            link_ctrl.bits.link_lane_num = lane_num - 1U;
        }
        slvs_link_regs->LINK_CTRL.u32 = link_ctrl.u32;
        mipi_rx_info_trace("devno:%u, U_LINK_CTRL %x", devno, slvs_link_regs->LINK_CTRL.u32);
    }
}

void slvs_drv_set_phy_en(unsigned int devno, hi_u32 en)
{
    U_PHY_EN phy_en;
    volatile slvs_phy_regs_t *slvs_phy_regs = get_slvs_phy_regs_by_dev(devno);

    if (slvs_phy_regs != NULL) {
        phy_en.u32 = slvs_phy_regs->PHY_EN.u32;
        phy_en.bits.phy_en = en;
        slvs_phy_regs->PHY_EN.u32 = phy_en.u32;
        mipi_rx_info_trace("devno:%u, U_PHY_EN %x", devno, slvs_phy_regs->PHY_EN.u32);
    }
}

void slvs_drv_set_lane_rate(unsigned int devno, slvs_lane_rate_t lane_rate)
{
    unsigned int phy_baudsel = 0;
    U_PHY_SEL phy_sel;
    volatile slvs_phy_regs_t *slvs_phy_regs = get_slvs_phy_regs_by_dev(devno);

    if (lane_rate == SLVS_LANE_RATE_LOW) {
        phy_baudsel = 1;
    } else if (lane_rate == SLVS_LANE_RATE_HIGH) {
        phy_baudsel = 0;
    } else {
        // fix sc
    }

    if (slvs_phy_regs != NULL) {
        phy_sel.u32 = slvs_phy_regs->PHY_SEL.u32;
        phy_sel.bits.phy_baudsel = phy_baudsel;
        slvs_phy_regs->PHY_SEL.u32 = phy_sel.u32;
        mipi_rx_info_trace("devno:%u, U_PHY_SEL %x", devno, slvs_phy_regs->PHY_SEL.u32);
    }
}
void slvs_clear_config(unsigned int devno, const short *p_lane_id)
{
    slvs_drv_set_lane_num(devno, 0);

    // 已与芯片确认不需要在ec属性设置时关闭ec的phy操作

    slvs_drv_set_lane_cken(devno, p_lane_id, 0x0);

    slvs_drv_set_lane_en(devno, p_lane_id, 0x0);

    (void)memset_s(&slvs_link_int_state[devno], sizeof(slvs_link_int_state_t), 0, sizeof(slvs_link_int_state_t));
}
static void slvs_drv_set_lane_rx_cken(unsigned int devno, short lane_id, hi_u32 en)
{
    U_PHY_RSTN_REQ phy_rstn_req;
    volatile slvs_phy_regs_t *slvs_phy_regs = get_slvs_phy_regs_by_dev(devno);
    if (slvs_phy_regs != NULL) {
        phy_rstn_req.u32 = slvs_phy_regs->PHY_RSTN_REQ.u32;
        if (en == 0x1) {
            // 8 slvs phy lane num
            phy_rstn_req.bits.phy_lane_rx_cken |= ((hi_u32)0x1U << (((hi_u32)lane_id % 8U)));
        } else {
            // 8 slvs phy lane num
            phy_rstn_req.bits.phy_lane_rx_cken &= (~((hi_u32)0x1U << ((hi_u32)(lane_id % 8U))));
        }

        slvs_phy_regs->PHY_RSTN_REQ.u32 = phy_rstn_req.u32;
        mipi_rx_info_trace("devno:%u, U_PHY_RSTN_REQ %x", devno, slvs_phy_regs->PHY_RSTN_REQ.u32);
    }
}

static void slvs_drv_set_phy_pcs_cken(unsigned int devno, short lane_id, hi_u32 en)
{
    U_PHY_PCS_EN phy_pcs_en;
    volatile slvs_phy_regs_t *slvs_phy_regs = get_slvs_phy_regs_by_dev(devno);

    if (slvs_phy_regs != NULL) {
        phy_pcs_en.u32 = slvs_phy_regs->PHY_PCS_EN.u32;

        switch (lane_id % 8) { /* 8:lane num per slvs */
            case 0: // SLVS_EC lane 0
                phy_pcs_en.bits.pcs_lane0_en = en;
                break;

            case 1: // SLVS_EC lane 1
                phy_pcs_en.bits.pcs_lane1_en = en;
                break;

            case 2: // SLVS_EC lane 2
                phy_pcs_en.bits.pcs_lane2_en = en;
                break;

            case 3: // SLVS_EC lane 3
                phy_pcs_en.bits.pcs_lane3_en = en;
                break;

            case 4: // SLVS_EC lane 4
                phy_pcs_en.bits.pcs_lane4_en = en;
                break;

            case 5: // SLVS_EC lane 5
                phy_pcs_en.bits.pcs_lane5_en = en;
                break;

            case 6: // SLVS_EC lane 6
                phy_pcs_en.bits.pcs_lane6_en = en;
                break;

            case 7: // SLVS_EC lane 7
                phy_pcs_en.bits.pcs_lane7_en = en;
                break;

            default:
                break;
        }

        slvs_phy_regs->PHY_PCS_EN.u32 = phy_pcs_en.u32;
        mipi_rx_info_trace("devno:%u, U_PHY_PCS_EN %x", devno, slvs_phy_regs->PHY_PCS_EN.u32);
    }
}

hi_void slvs_drv_set_lane_cken(unsigned int devno, const short *p_lane_id, hi_u32 en)
{
    int i;
    int lane_id_size;

    lane_id_size = SLVS_LANE_NUM;
    for (i = 0; i < lane_id_size; i++) {
        if (is_valid_id(p_lane_id[i])) {
            slvs_drv_set_lane_rx_cken(devno, p_lane_id[i], en);
            slvs_drv_set_phy_pcs_cken(devno, p_lane_id[i], en);
        }
    }

    return;
}

static void slvs_drv_lane_srst_reset(unsigned int devno, short lane_id, hi_u32 reset)
{
    U_PHY_RSTN_REQ phy_rstn_req;
    volatile slvs_phy_regs_t *slvs_phy_regs = get_slvs_phy_regs_by_dev(devno);

    if (slvs_phy_regs != NULL) {
        phy_rstn_req.u32 = slvs_phy_regs->PHY_RSTN_REQ.u32;

        if (reset == 0x1) {
            phy_rstn_req.bits.phy_lane_srst_req |= ((hi_u32)0x1U <<
                (((hi_u32)lane_id % 8U))); /* 8:phy lane num */
        } else {
            phy_rstn_req.bits.phy_lane_srst_req &= (~((hi_u32)0x1U <<
                (((hi_u32)lane_id % 8U)))); /* 8:phy lane num */
        }

        slvs_phy_regs->PHY_RSTN_REQ.u32 = phy_rstn_req.u32;
        mipi_rx_info_trace("devno:%u, U_PHY_RSTN_REQ %x", devno, slvs_phy_regs->PHY_RSTN_REQ.u32);
    }
}

hi_void slvs_drv_lane_reset(unsigned int devno, const short *p_lane_id)
{
    int i;
    int lane_id_size;

    lane_id_size = SLVS_LANE_NUM;

    for (i = 0; i < lane_id_size; i++) {
        if (is_valid_id(p_lane_id[i])) {
            slvs_drv_lane_srst_reset(devno, p_lane_id[i], 0x1);
        }
    }

    return;
}

hi_void slvs_drv_lane_unreset(unsigned int devno, const short *p_lane_id)
{
    int i;
    int lane_id_size;

    lane_id_size = SLVS_LANE_NUM;

    for (i = 0; i < lane_id_size; i++) {
        if (is_valid_id(p_lane_id[i])) {
            slvs_drv_lane_srst_reset(devno, p_lane_id[i], 0x0);
        }
    }

    return;
}

static void slvs_drv_set_phy_lane_en(unsigned int devno, int lane_id, hi_u32 en)
{
    U_PHY_EN phy_en;
    volatile slvs_phy_regs_t *slvs_phy_regs = get_slvs_phy_regs_by_dev(devno);

    if (slvs_phy_regs != NULL) {
        phy_en.u32 = slvs_phy_regs->PHY_EN.u32;

        switch (lane_id % 8) { /* 8:lane num per slvs */
            case 0: // lane 0
                phy_en.bits.phy_lane0_en = en;
                break;

            case 1: // lane 1
                phy_en.bits.phy_lane1_en = en;
                break;

            case 2: // lane 2
                phy_en.bits.phy_lane2_en = en;
                break;

            case 3: // lane 3
                phy_en.bits.phy_lane3_en = en;
                break;

            case 4: // lane 4
                phy_en.bits.phy_lane4_en = en;
                break;

            case 5: // lane 5
                phy_en.bits.phy_lane5_en = en;
                break;

            case 6: // lane 6
                phy_en.bits.phy_lane6_en = en;
                break;

            case 7: // lane 7
                phy_en.bits.phy_lane7_en = en;
                break;

            default:
                break;
        }

        slvs_phy_regs->PHY_EN.u32 = phy_en.u32;
        mipi_rx_info_trace("devno:%u, U_PHY_EN:%x", devno, slvs_phy_regs->PHY_EN.u32);
    }
}

static void slvs_drv_set_phy_term_en(unsigned int devno, int lane_id, hi_u32 en)
{
    U_PHY_TERM_EN phy_term_en;
    volatile slvs_phy_regs_t *slvs_phy_regs = get_slvs_phy_regs_by_dev(devno);

    if (slvs_phy_regs != NULL) {
        phy_term_en.u32 = slvs_phy_regs->PHY_TERM_EN.u32;

        switch (lane_id % 8) { /* 8:lane num per slvs */
            case 0: // lane 0
                phy_term_en.bits.phy_rg_term0_en = en;
                break;

            case 1: // lane 1
                phy_term_en.bits.phy_rg_term1_en = en;
                break;

            case 2: // lane 2
                phy_term_en.bits.phy_rg_term2_en = en;
                break;

            case 3: // lane 3
                phy_term_en.bits.phy_rg_term3_en = en;
                break;

            case 4: // lane 4
                phy_term_en.bits.phy_rg_term4_en = en;
                break;

            case 5: // lane 5
                phy_term_en.bits.phy_rg_term5_en = en;
                break;

            case 6: // lane 6
                phy_term_en.bits.phy_rg_term6_en = en;
                break;

            case 7: // lane 7
                phy_term_en.bits.phy_rg_term7_en = en;
                break;

            default:
                break;
        }

        slvs_phy_regs->PHY_TERM_EN.u32 = phy_term_en.u32;
        mipi_rx_info_trace("devno:%u, U_PHY_TERM_EN:%x", devno, slvs_phy_regs->PHY_TERM_EN.u32);
    }
}

static void slvs_drv_set_phy_align_en(unsigned int devno, int lane_id, hi_u32 en)
{
    U_PHY_ALIGN_EN_LINK phy_align_en_link;
    volatile slvs_link_regs_t *slvs_link_regs = get_slvs_link_regs_by_dev(devno);

    if (slvs_link_regs != NULL) {
        phy_align_en_link.u32 = slvs_link_regs->PHY_ALIGN_EN_LINK.u32;

        phy_align_en_link.bits.link_phy_align_en = en;

        switch (lane_id % 8) { /* 8:lane num per slvs */
            case 0: // lane 0
                phy_align_en_link.bits.link_phy_align_lane0_en = en;
                break;

            case 1: // lane 1
                phy_align_en_link.bits.link_phy_align_lane1_en = en;
                break;

            case 2: // lane 2
                phy_align_en_link.bits.link_phy_align_lane2_en = en;
                break;

            case 3: // lane 3
                phy_align_en_link.bits.link_phy_align_lane3_en = en;
                break;

            case 4: // lane 4
                phy_align_en_link.bits.link_phy_align_lane4_en = en;
                break;

            case 5: // lane 5
                phy_align_en_link.bits.link_phy_align_lane5_en = en;
                break;

            case 6: // lane 6
                phy_align_en_link.bits.link_phy_align_lane6_en = en;
                break;

            case 7: // lane 7
                phy_align_en_link.bits.link_phy_align_lane7_en = en;
                break;

            default:
                break;
        }

        slvs_link_regs->PHY_ALIGN_EN_LINK.u32 = phy_align_en_link.u32;
        mipi_rx_info_trace("devno:%u, U_PHY_ALIGN_EN_LINK:%x", devno, slvs_link_regs->PHY_ALIGN_EN_LINK.u32);
    }
}

hi_void slvs_drv_set_lane_en(unsigned int devno, const short *p_lane_id, hi_u32 en)
{
    hi_u32 i;
    hi_u32 lane_id_size;

    lane_id_size = SLVS_LANE_NUM;

    for (i = 0U; i < lane_id_size; i++) {
        if (is_valid_id(p_lane_id[i])) {
            slvs_drv_set_phy_lane_en(devno, p_lane_id[i], en);
            slvs_drv_set_phy_term_en(devno, p_lane_id[i], en);
        }
        if (en == 0x0u) {
            slvs_drv_set_phy_align_en(devno, (hi_s32)i, en);
        } else {
            if (is_valid_id(p_lane_id[i])) {
                slvs_drv_set_phy_align_en(devno, p_lane_id[i], en);
            }
        }
    }
    return;
}

/**
 * slvs_drv_set_bist_en() - Config SLVS_EC Module Build-In Self Test Function
 * @devno: SLVS_EC device number
 */
void slvs_drv_set_bist_en(unsigned int devno)
{
    U_PHY_BIST_EN bist_en;
    volatile slvs_phy_regs_t *slvs_phy_regs = get_slvs_phy_regs_by_dev(devno);
    if (slvs_phy_regs != NULL) {
        bist_en.u32 = slvs_phy_regs->PHY_BIST_EN.u32;
        bist_en.u32 = 0x410000;
        slvs_phy_regs->PHY_BIST_EN.u32 = bist_en.u32;
        mipi_rx_info_trace("devno:%u, PHY_BIST_EN:%x", devno, slvs_phy_regs->PHY_BIST_EN.u32);
    }
}

static void slvs_drv_set_phy_lane_sel(unsigned int devno, int lane_idx)
{
    unsigned int phy_lane_sel;
    U_PHY_LANE_CHN_SEL phy_lane_chn_sel;
    volatile slvs_phy_regs_t *slvs_phy_regs = get_slvs_phy_regs_by_dev(devno);

    if (slvs_phy_regs != NULL) {
        phy_lane_sel = (devno - SLVS_DEV_NUM_START) % 2U; /* 2:part */
        phy_lane_chn_sel.u32 = slvs_phy_regs->PHY_LANE_CHN_SEL.u32;

        switch (lane_idx % 8) { /* 8:phy lane num */
            case 0: // SLVS_EC lane 0
                phy_lane_chn_sel.bits.phy_lane0_chn_sel = phy_lane_sel;
                break;

            case 1: // SLVS_EC lane 1
                phy_lane_chn_sel.bits.phy_lane1_chn_sel = phy_lane_sel;
                break;

            case 2: // SLVS_EC lane 2
                phy_lane_chn_sel.bits.phy_lane2_chn_sel = phy_lane_sel;
                break;

            case 3: // SLVS_EC lane 3
                phy_lane_chn_sel.bits.phy_lane3_chn_sel = phy_lane_sel;
                break;

            case 4: // SLVS_EC lane 4
                phy_lane_chn_sel.bits.phy_lane4_chn_sel = phy_lane_sel;
                break;

            case 5: // SLVS_EC lane 5
                phy_lane_chn_sel.bits.phy_lane5_chn_sel = phy_lane_sel;
                break;

            case 6: // SLVS_EC lane 6
                phy_lane_chn_sel.bits.phy_lane6_chn_sel = phy_lane_sel;
                break;

            case 7: // SLVS_EC lane 7
                phy_lane_chn_sel.bits.phy_lane7_chn_sel = phy_lane_sel;
                break;

            default:
                break;
        }

        slvs_phy_regs->PHY_LANE_CHN_SEL.u32 = phy_lane_chn_sel.u32;
        mipi_rx_info_trace("devno:%u, U_PHY_LANE_CHN_SEL:%x", devno, slvs_phy_regs->PHY_LANE_CHN_SEL.u32);
    }
}

hi_void slvs_drv_set_lane_sel(unsigned int devno, const short *p_lane_id)
{
    int i;
    int lane_id_size;

    lane_id_size = SLVS_LANE_NUM;

    for (i = 0; i < lane_id_size; i++) {
        if (is_valid_id(p_lane_id[i])) {
            slvs_drv_set_phy_lane_sel(devno, p_lane_id[i]);
        }
    }

    return;
}

void slvs_drv_set_raw_type(unsigned int devno, data_type_t input_data_type)
{
    U_LINK_CTRL link_ctrl;
    unsigned int temp_data_type = 0;

    volatile slvs_link_regs_t *slvs_link_regs = get_slvs_link_regs_by_dev(devno);

    if (slvs_link_regs != NULL) {
        if (input_data_type == DATA_TYPE_RAW_8BIT) {
            temp_data_type = 0x0;
        }

        if (input_data_type == DATA_TYPE_RAW_10BIT) {
            temp_data_type = 0x1;
        } else if (input_data_type == DATA_TYPE_RAW_12BIT) {
            temp_data_type = 0x2;
        } else if (input_data_type == DATA_TYPE_RAW_14BIT) {
            temp_data_type = 0x3;
        } else if (input_data_type == DATA_TYPE_RAW_16BIT) {
            temp_data_type = 0x4;
        } else {
            // fix sc
        }

        link_ctrl.u32 = slvs_link_regs->LINK_CTRL.u32;
        link_ctrl.bits.link_raw_type = temp_data_type;
        slvs_link_regs->LINK_CTRL.u32 = link_ctrl.u32;
        mipi_rx_info_trace("devno:%u, U_LINK_CTRL:%x", devno, slvs_link_regs->LINK_CTRL.u32);
    }
}

void slvs_drv_set_data_rate(unsigned int devno, mipi_data_rate_t data_rate)
{
    U_LINK_CTRL link_ctrl;
    unsigned int mipi_double_pix_en = 0;

    volatile slvs_link_regs_t *slvs_link_regs = get_slvs_link_regs_by_dev(devno);

    if (data_rate == MIPI_DATA_RATE_X1) {
        mipi_double_pix_en = 0;
    } else if (data_rate == MIPI_DATA_RATE_X2) {
        mipi_double_pix_en = 0x1;
    } else {
        mipi_rx_err_trace("devno %u, unsupported data_rate:%d should be in %d or %d\n",
            devno, (hi_s32)data_rate, (hi_s32)MIPI_DATA_RATE_X1, (hi_s32)MIPI_DATA_RATE_X2);;
        return;
    }

    if (slvs_link_regs != NULL) {
        link_ctrl.u32 = slvs_link_regs->LINK_CTRL.u32;
        link_ctrl.bits.link_double_pix_en = mipi_double_pix_en;
        slvs_link_regs->LINK_CTRL.u32 = link_ctrl.u32;
        mipi_rx_info_trace("devno:%u, U_LINK_CTRL:%x", devno, slvs_link_regs->LINK_CTRL.u32);
    }
}

void slvs_drv_set_wdr_mode(unsigned int devno, wdr_mode_t wdr_mode)
{
    U_LINK_CTRL link_ctrl;
    U_LINK_DOL_FR_PAR link_dol_fr_par;
    unsigned int wdr_num = 0;
    unsigned int dol_fr0_par = 0x0;
    unsigned int dol_fr1_par = 0x0;

    volatile slvs_link_regs_t *slvs_link_regs = get_slvs_link_regs_by_dev(devno);

    if (wdr_mode == HI_WDR_MODE_NONE) {
        wdr_num = 0;
        dol_fr0_par = 0;
        dol_fr1_par = 0;
    } else if (wdr_mode == HI_WDR_MODE_DOL_2F) {
        wdr_num = 0x1;
        dol_fr0_par = 0x0;
        dol_fr1_par = 0x4;
    } else {
        mipi_rx_err_trace("devno %u unsupported  wdr_mode:%u, should be %u or %u\n",
            (hi_u32)wdr_mode, devno, (hi_u32)HI_WDR_MODE_NONE, (hi_u32)HI_WDR_MODE_DOL_2F);
        return;
    }

    if (slvs_link_regs != NULL) {
        link_ctrl.u32 = slvs_link_regs->LINK_CTRL.u32;
        link_ctrl.bits.link_wdr_num = wdr_num;
        slvs_link_regs->LINK_CTRL.u32 = link_ctrl.u32;

        link_dol_fr_par.u32 = slvs_link_regs->LINK_DOL_FR_PAR.u32;
        link_dol_fr_par.bits.link_dol_fr0_par = dol_fr0_par;
        link_dol_fr_par.bits.link_dol_fr1_par = dol_fr1_par;
        slvs_link_regs->LINK_DOL_FR_PAR.u32 = link_dol_fr_par.u32;

        mipi_rx_info_trace("devno:%u, U_LINK_CTRL:%x", devno, slvs_link_regs->LINK_CTRL.u32);
        mipi_rx_info_trace("devno:%u, U_LINK_DOL_FR_PAR:%x", devno, slvs_link_regs->LINK_DOL_FR_PAR.u32);
    }
}

void slvs_drv_set_deskew_symbol(unsigned int devno, unsigned int symbol)
{
    U_PHY_DESKEW_SYMBOL_LINK pht_deskew_symbol_link;

    volatile slvs_link_regs_t *slvs_link_regs = get_slvs_link_regs_by_dev(devno);

    if (slvs_link_regs != NULL) {
        pht_deskew_symbol_link.u32 = slvs_link_regs->PHY_DESKEW_SYMBOL_LINK.u32;
        pht_deskew_symbol_link.bits.link_phy_deskew_symbol = symbol;
        slvs_link_regs->PHY_DESKEW_SYMBOL_LINK.u32 = pht_deskew_symbol_link.u32;
        mipi_rx_info_trace("devno:%u, U_PHY_DESKEW_SYMBOL_LINK:%x", devno, slvs_link_regs->PHY_DESKEW_SYMBOL_LINK.u32);
    }
}

void slvs_drv_set_clear_en(unsigned int devno, hi_u32 en)
{
    U_LINK_CLEAR_EN link_clear_en;

    volatile slvs_link_regs_t *slvs_link_regs = get_slvs_link_regs_by_dev(devno);

    if (slvs_link_regs != NULL) {
        link_clear_en.u32 = slvs_link_regs->LINK_CLEAR_EN.u32;
        link_clear_en.bits.link_skew_clear_en = en;

        link_clear_en.bits.link_header_crc_clear_en = en;
        link_clear_en.bits.link_payload_crc_clear_en = en;
        link_clear_en.bits.link_ecc_clear_en = en;
        link_clear_en.bits.link_header_crc_filter_en = en;
        link_clear_en.bits.link_vsync_clear_en = en;

        slvs_link_regs->LINK_CLEAR_EN.u32 = link_clear_en.u32;
        mipi_rx_info_trace("devno:%u, U_LINK_CLEAR_EN:%x", devno, slvs_link_regs->LINK_CLEAR_EN.u32);
    }
}

void slvs_drv_set_mem_ck_en(unsigned int devno, hi_u32 en)
{
    U_LINK_MEMORY_CTRL link_memory_ctrl;

    volatile slvs_link_regs_t *slvs_link_regs = get_slvs_link_regs_by_dev(devno);

    if (slvs_link_regs != NULL) {
        link_memory_ctrl.u32 = slvs_link_regs->LINK_MEMORY_CTRL.u32;
        link_memory_ctrl.bits.link_mem_ck_en = en;
        slvs_link_regs->LINK_MEMORY_CTRL.u32 = link_memory_ctrl.u32;
        mipi_rx_info_trace("devno:%u, U_LINK_CLEAR_EN:%x", devno, slvs_link_regs->LINK_MEMORY_CTRL.u32);
    }
}

void slvs_drv_set_sensor_avalid_width(unsigned int devno, int width)
{
    U_LINK_PAYLOAD_SIZE link_payload_size;

    volatile slvs_link_regs_t *slvs_link_regs = get_slvs_link_regs_by_dev(devno);

    if (slvs_link_regs != NULL) {
        link_payload_size.u32 = slvs_link_regs->LINK_PAYLOAD_SIZE.u32;
        link_payload_size.bits.link_paylsize = (unsigned int)width;
        slvs_link_regs->LINK_PAYLOAD_SIZE.u32 = link_payload_size.u32;
        mipi_rx_info_trace("devno:%u, U_LINK_PAYLOAD_SIZE:%x", devno, slvs_link_regs->LINK_PAYLOAD_SIZE.u32);
    }
}

void slvs_drv_set_image_rect(unsigned int devno, const img_rect_t *p_img_rect)
{
    U_LINK_HEIGHT link_height;
    U_LINK_WIDTH link_width;

    volatile slvs_link_regs_t *slvs_link_regs = get_slvs_link_regs_by_dev(devno);

    if (slvs_link_regs != NULL) {
        link_width.u32 = slvs_link_regs->LINK_WIDTH.u32;
        link_width.bits.link_start_x = (unsigned int) p_img_rect->x;
        link_width.bits.link_imgwidth = p_img_rect->width - 1U;
        slvs_link_regs->LINK_WIDTH.u32 = link_width.u32;

        link_height.u32 = slvs_link_regs->LINK_HEIGHT.u32;
        link_height.bits.link_line_start = (unsigned int) p_img_rect->y;
        link_height.bits.link_line_end = (unsigned int) p_img_rect->y + (unsigned int) p_img_rect->height - 1U;
        slvs_link_regs->LINK_HEIGHT.u32 = link_height.u32;

        mipi_rx_info_trace("devno:%u, U_LINK_WIDTH:%x", devno, slvs_link_regs->LINK_WIDTH.u32);
        mipi_rx_info_trace("devno:%u, U_LINK_HEIGHT:%x", devno, slvs_link_regs->LINK_HEIGHT.u32);
    }
}

void slvs_drv_set_crop_en(unsigned int devno, hi_u32 en)
{
    U_LINK_CTRL link_ctrl;

    volatile slvs_link_regs_t *slvs_link_regs = get_slvs_link_regs_by_dev(devno);

    if (slvs_link_regs != NULL) {
        link_ctrl.u32 = slvs_link_regs->LINK_CTRL.u32;
        link_ctrl.bits.link_hcrop_en = en;
        link_ctrl.bits.link_vcrop_en = en;
        slvs_link_regs->LINK_CTRL.u32 = link_ctrl.u32;
        mipi_rx_info_trace("devno:%u, U_LINK_CTRL:%x", devno, slvs_link_regs->LINK_CTRL.u32);
    }
}

void slvs_drv_set_crc_enable(unsigned int devno, hi_u32 enable)
{
    U_LINK_CTRL link_ctrl;

    volatile slvs_link_regs_t *slvs_link_regs = get_slvs_link_regs_by_dev(devno);

    if (slvs_link_regs != NULL) {
        link_ctrl.u32 = slvs_link_regs->LINK_CTRL.u32;
        link_ctrl.bits.link_crc_en = enable;
        slvs_link_regs->LINK_CTRL.u32 = link_ctrl.u32;
        mipi_rx_info_trace("devno:%u, U_LINK_CTRL:%x", devno, slvs_link_regs->LINK_CTRL.u32);
    }
}

void slvs_drv_set_ecc_basic_block_size(unsigned int devno, unsigned int basic_block_size)
{
    U_LINK_BLOCK_SIZE link_block_size;

    volatile slvs_link_regs_t *slvs_link_regs = get_slvs_link_regs_by_dev(devno);

    if (slvs_link_regs != NULL) {
        link_block_size.u32 = slvs_link_regs->LINK_BLOCK_SIZE.u32;
        link_block_size.bits.link_bblk_size = basic_block_size;
        slvs_link_regs->LINK_BLOCK_SIZE.u32 = link_block_size.u32;
        mipi_rx_info_trace("devno:%u, U_LINK_BLOCK_SIZE:%x", devno, slvs_link_regs->LINK_BLOCK_SIZE.u32);
    }
}

void slvs_drv_set_ecc_extra_block_size(unsigned int devno, unsigned int extra_block_size)
{
    U_LINK_BLOCK_SIZE link_block_size;

    volatile slvs_link_regs_t *slvs_link_regs = get_slvs_link_regs_by_dev(devno);

    if (slvs_link_regs != NULL) {
        link_block_size.u32 = slvs_link_regs->LINK_BLOCK_SIZE.u32;
        link_block_size.bits.link_extblk_size = extra_block_size;
        slvs_link_regs->LINK_BLOCK_SIZE.u32 = link_block_size.u32;
        mipi_rx_info_trace("devno:%u, U_LINK_BLOCK_SIZE:%x", devno, slvs_link_regs->LINK_BLOCK_SIZE.u32);
    }
}

void slvs_drv_set_ecc_basic_block_num(unsigned int devno, unsigned int basic_block_num)
{
    U_LINK_NUM_BLOCK_SIZE link_num_block_size;

    volatile slvs_link_regs_t *slvs_link_regs = get_slvs_link_regs_by_dev(devno);

    if (slvs_link_regs != NULL) {
        link_num_block_size.u32 = slvs_link_regs->LINK_NUM_BLOCK_SIZE.u32;
        link_num_block_size.bits.link_number_of_basic_block = basic_block_num;
        slvs_link_regs->LINK_NUM_BLOCK_SIZE.u32 = link_num_block_size.u32;
        mipi_rx_info_trace("devno:%u, U_LINK_NUM_BLOCK_SIZE:%x", devno, slvs_link_regs->LINK_NUM_BLOCK_SIZE.u32);
    }
}

void slvs_drv_set_ecc_option(unsigned int devno, unsigned int ecc_option)
{
    U_LINK_CTRL link_ctrl;

    volatile slvs_link_regs_t *slvs_link_regs = get_slvs_link_regs_by_dev(devno);

    if (slvs_link_regs != NULL) {
        link_ctrl.u32 = slvs_link_regs->LINK_CTRL.u32;
        link_ctrl.bits.link_ecc_option = ecc_option;
        slvs_link_regs->LINK_CTRL.u32 = link_ctrl.u32;
        mipi_rx_info_trace("devno:%u, U_LINK_CTRL:%x", devno, slvs_link_regs->LINK_CTRL.u32);
    }
}

static void slvs_drv_set_link_align_id(unsigned int devno, unsigned int lane_idx, short lane_id)
{
    U_PHY_ALIGN_ID_LINK phy_align_id_link;

    volatile slvs_link_regs_t *slvs_link_regs = get_slvs_link_regs_by_dev(devno);

    if (slvs_link_regs != NULL) {
        phy_align_id_link.u32 = slvs_link_regs->PHY_ALIGN_ID_LINK.u32;

        switch (lane_id % 8) { /* 8:lane num per slvs */
            case 0: // lane 0
                phy_align_id_link.bits.link_phy_align_lane0_id = lane_idx;
                break;

            case 1: // lane 1
                phy_align_id_link.bits.link_phy_align_lane1_id = lane_idx;
                break;

            case 2: // lane 2
                phy_align_id_link.bits.link_phy_align_lane2_id = lane_idx;
                break;

            case 3: // lane 3
                phy_align_id_link.bits.link_phy_align_lane3_id = lane_idx;
                break;

            case 4: // lane 4
                phy_align_id_link.bits.link_phy_align_lane4_id = lane_idx;
                break;

            case 5: // lane 5
                phy_align_id_link.bits.link_phy_align_lane5_id = lane_idx;
                break;

            case 6: // lane 6
                phy_align_id_link.bits.link_phy_align_lane6_id = lane_idx;
                break;

            case 7: // lane 7
                phy_align_id_link.bits.link_phy_align_lane7_id = lane_idx;
                break;

            default:
                break;
        }

        slvs_link_regs->PHY_ALIGN_ID_LINK.u32 = phy_align_id_link.u32;

        mipi_rx_info_trace("devno:%u, PHY_ALIGN_ID_LINK:%x", devno, slvs_link_regs->PHY_ALIGN_ID_LINK.u32);
    }
}

hi_void slvs_drv_set_link_lane_order(unsigned int devno, const short *p_lane_id)
{
    unsigned int i;
    unsigned int lane_id_size;

    lane_id_size = SLVS_LANE_NUM;

    for (i = 0; i < lane_id_size; i++) {
        if (is_valid_id(p_lane_id[i])) {
            slvs_drv_set_link_align_id(devno, i, p_lane_id[i]);
        }
    }

    return;
}

void slvs_drv_phy_pcs_int_mask(hi_u32 phy_id, unsigned int mask)
{
    U_PHY_PCS_INT_MASK phy_pcs_int_mask;

    volatile slvs_phy_regs_t *slvs_phy_regs = get_slvs_phy_regs_by_phy(phy_id);

    phy_pcs_int_mask.u32 = mask;
    slvs_phy_regs->PHY_PCS_INT_MASK.u32 = phy_pcs_int_mask.u32;

    mipi_rx_info_trace("phy_id:%u, U_PHY_PCS_INT_MASK:%x", phy_id, slvs_phy_regs->PHY_PCS_INT_MASK.u32);
}

void slvs_drv_set_slvs_mask(unsigned int mask)
{
    volatile slvs_sys_regs_t *slvs_sys_regs = get_slvs_sys_regs();

    slvs_sys_regs->SLVS_INT_MASK.u32 = mask;

    mipi_rx_info_trace("U_SLVS_INT_MASK:%x", slvs_sys_regs->SLVS_INT_MASK.u32);
}

void slvs_drv_set_link_int_mask(unsigned int devno, unsigned int mask)
{
    U_LINK_CTRL_INT_MASK link0_ctrl_int_mask;

    volatile slvs_link_regs_t *slvs_link_regs = get_slvs_link_regs_by_dev(devno);

    if (slvs_link_regs != NULL) {
        link0_ctrl_int_mask.u32 = mask;
        slvs_link_regs->LINK_CTRL_INT_MASK.u32 = link0_ctrl_int_mask.u32;
        mipi_rx_info_trace("devno:%u, U_LINK_CTRL_INT_MASK:%x", devno, slvs_link_regs->LINK_CTRL_INT_MASK.u32);
    }
}

unsigned int slvs_drv_get_phy_raw_int(hi_u32 phy_id)
{
    unsigned int status;

    volatile slvs_phy_regs_t *slvs_phy_regs = get_slvs_phy_regs_by_phy(phy_id);

    status = slvs_phy_regs->PHY_PCS_INT_RAW.u32;

    return status;
}

void slvs_drv_clear_phy_raw_int(hi_u32 phy_id, unsigned int intclr)
{
    U_PHY_PCS_INT_RAW phy_pcs_int_raw;
    volatile slvs_phy_regs_t *slvs_phy_regs = get_slvs_phy_regs_by_phy(phy_id);

    phy_pcs_int_raw.u32 = intclr;
    slvs_phy_regs->PHY_PCS_INT_RAW.u32 = phy_pcs_int_raw.u32;
    mipi_rx_info_trace("phy_id:%u, U_PHY_PCS_INT_RAW:%x", phy_id, slvs_phy_regs->PHY_PCS_INT_RAW.u32);
}

unsigned int slvs_drv_get_slvs_int(hi_u32 phy_id)
{
    unsigned int status;

    volatile slvs_sys_regs_t *slvs_sys_regs = get_slvs_sys_regs();

    status = (slvs_sys_regs->SLVS_INT_RAW.u32 >> (4U * phy_id)) & 0xfU; /* 4:lane per phy */

    return status;
}

void slvs_drv_clear_slvs_int(hi_u32 phy_id, unsigned int intclr)
{
    volatile slvs_sys_regs_t *slvs_sys_regs = get_slvs_sys_regs();

    hi_mipi_rx_unused(intclr);
    if (phy_id == 0) {
        slvs_sys_regs->SLVS_INT_RAW.bits.int_phy_ctrl0_raw = 1;
        slvs_sys_regs->SLVS_INT_RAW.bits.int_link0_ctrl0_raw = 1;
        slvs_sys_regs->SLVS_INT_RAW.bits.int_link1_ctrl0_raw = 1;
    } else {
        slvs_sys_regs->SLVS_INT_RAW.bits.int_phy_ctrl1_raw = 1;
        slvs_sys_regs->SLVS_INT_RAW.bits.int_link0_ctrl1_raw = 1;
        slvs_sys_regs->SLVS_INT_RAW.bits.int_link1_ctrl1_raw = 1;
    }

    mipi_rx_info_trace("phy_id:%u, U_SLVS_INT_RAW:%x", phy_id, slvs_sys_regs->SLVS_INT_RAW.u32);
}

unsigned int slvs_drv_get_link_raw_int(unsigned int devno)
{
    unsigned int status = 0;

    volatile slvs_link_regs_t *slvs_link_regs = get_slvs_link_regs_by_dev(devno);

    if (slvs_link_regs != NULL) {
        status = slvs_link_regs->LINK_CTRL_INT_RAW.u32;
    }

    return status;
}

void slvs_drv_clear_link_raw_int(unsigned int devno, unsigned int intclr)
{
    U_LINK_CTRL_INT_RAW link_ctrl_int_raw;
    volatile slvs_link_regs_t *slvs_link_regs = get_slvs_link_regs_by_dev(devno);

    link_ctrl_int_raw.u32 = intclr;
    if (slvs_link_regs != NULL) {
        slvs_link_regs->LINK_CTRL_INT_RAW.u32 = link_ctrl_int_raw.u32;
    }
}

void slvs_drv_get_imgsize_statis(unsigned int devno, short vc, img_size_t *p_size)
{
    U_LINK_IMGSIZE0_STATUS mipi_imgsize0_statis;
    U_LINK_IMGSIZE1_STATUS mipi_imgsize1_statis;
    U_LINK_IMGSIZE2_STATUS mipi_imgsize2_statis;
    U_LINK_IMGSIZE3_STATUS mipi_imgsize3_statis;

    volatile slvs_link_regs_t *slvs_link_regs = get_slvs_link_regs_by_dev(devno);

    if (slvs_link_regs != NULL) {
        if (vc == 0) {
            mipi_imgsize0_statis.u32 = slvs_link_regs->LINK_IMGSIZE0_STATUS.u32;
            p_size->width = mipi_imgsize0_statis.bits.link_imgwidth_statis_vc0;
            p_size->height = mipi_imgsize0_statis.bits.link_imgheight_statis_vc0;
        } else if (vc == 1) {
            mipi_imgsize1_statis.u32 = slvs_link_regs->LINK_IMGSIZE1_STATUS.u32;
            p_size->width = mipi_imgsize1_statis.bits.link_imgwidth_statis_vc1;
            p_size->height = mipi_imgsize1_statis.bits.link_imgheight_statis_vc1;
        } else if (vc == 2) { /* 2:vc no. */
            mipi_imgsize2_statis.u32 = slvs_link_regs->LINK_IMGSIZE2_STATUS.u32;
            p_size->width = mipi_imgsize2_statis.bits.link_imgwidth_statis_vc2;
            p_size->height = mipi_imgsize2_statis.bits.link_imgheight_statis_vc2;
        } else if (vc == 3) { /* 3:vc no. */
            mipi_imgsize3_statis.u32 = slvs_link_regs->LINK_IMGSIZE3_STATUS.u32;
            p_size->width = mipi_imgsize3_statis.bits.link_imgwidth_statis_vc3;
            p_size->height = mipi_imgsize3_statis.bits.link_imgheight_statis_vc3;
        } else {
            // fix sc
        }
    }
}

void slvs_drv_get_phy_data(unsigned int devno, short lane_id, unsigned int *phy_data)
{
    volatile slvs_phy_regs_t *slvs_phy_regs = get_slvs_phy_regs_by_dev(devno);

    if (slvs_phy_regs != NULL) {
        switch (lane_id % 8) { /* 8:lane num per slvs */
            case 0: // lane 0
                *phy_data = slvs_phy_regs->PHY_LANE_DATA0.bits.phy_data0_slvs;
                break;
            case 1: // lane 1
                *phy_data = slvs_phy_regs->PHY_LANE_DATA0.bits.phy_data1_slvs;
                break;
            case 2: // lane 2
                *phy_data = slvs_phy_regs->PHY_LANE_DATA1.bits.phy_data2_slvs;
                break;
            case 3: // lane 3
                *phy_data = slvs_phy_regs->PHY_LANE_DATA1.bits.phy_data3_slvs;
                break;
            case 4: // lane 4
                *phy_data = slvs_phy_regs->PHY_LANE_DATA2.bits.phy_data4_slvs;
                break;
            case 5: // lane 5
                *phy_data = slvs_phy_regs->PHY_LANE_DATA2.bits.phy_data5_slvs;
                break;
            case 6: // lane 6
                *phy_data = slvs_phy_regs->PHY_LANE_DATA3.bits.phy_data6_slvs;
                break;
            case 7: // lane 7
                *phy_data = slvs_phy_regs->PHY_LANE_DATA3.bits.phy_data7_slvs;
                break;
            default:
                *phy_data = 0x0;
                break;
        }
    }
}

void slvs_drv_get_phy_aligned_data(unsigned int devno, short lane_id, unsigned int *phy_data)
{
    volatile slvs_phy_regs_t *slvs_phy_regs = get_slvs_phy_regs_by_dev(devno);

    if (slvs_phy_regs != NULL) {
        switch (lane_id % 8) { /* 8:lane num per slvs */
            case 0: // lane 0
                *phy_data = slvs_phy_regs->PHY_SYMAL_DATA0.bits.physt_symal_lane0_d;
                break;
            case 1: // lane 1
                *phy_data = slvs_phy_regs->PHY_SYMAL_DATA0.bits.physt_symal_lane1_d;
                break;
            case 2: // lane 2
                *phy_data = slvs_phy_regs->PHY_SYMAL_DATA1.bits.physt_symal_lane2_d;
                break;
            case 3: // lane 3
                *phy_data = slvs_phy_regs->PHY_SYMAL_DATA1.bits.physt_symal_lane3_d;
                break;
            case 4: // lane 4
                *phy_data = slvs_phy_regs->PHY_SYMAL_DATA2.bits.physt_symal_lane4_d;
                break;
            case 5: // lane 5
                *phy_data = slvs_phy_regs->PHY_SYMAL_DATA2.bits.physt_symal_lane5_d;
                break;
            case 6: // lane 6
                *phy_data = slvs_phy_regs->PHY_SYMAL_DATA3.bits.physt_symal_lane6_d;
                break;
            case 7: // lane 7
                *phy_data = slvs_phy_regs->PHY_SYMAL_DATA3.bits.physt_symal_lane7_d;
                break;
            default:
                *phy_data = 0x0;
                break;
        }
    }
}

unsigned int slvs_drv_get_phy_valid_lane(unsigned int devno)
{
    unsigned int status = 0;

    volatile slvs_phy_regs_t *slvs_phy_regs = get_slvs_phy_regs_by_dev(devno);

    if (slvs_phy_regs != NULL) {
        status = slvs_phy_regs->PHY_SYMAL_VALID.bits.physt_symal_valid;
    }

    return status;
}

static void slvs_phy_int_statis(hi_u32 phy_id)
{
    hi_u32 i;

    unsigned int link_int_status;
    unsigned int slvs_int_status;

    if (((phy_id) < 0U) || ((phy_id) >= SLVS_PHY_NUM)) {
        mipi_rx_err_trace("Err phy_id %u, should be in [0, %u)!\n", phy_id, SLVS_PHY_NUM);
        return;
    }

    link_int_status = slvs_drv_get_phy_raw_int(phy_id);
    slvs_int_status = slvs_drv_get_slvs_int(phy_id);
    if ((link_int_status != 0U) || (slvs_int_status != 0U)) {
        slvs_drv_clear_phy_raw_int(phy_id, 0xffffffffU);
        slvs_drv_clear_slvs_int(phy_id, 0xffffffffU);

        for (i = 0U; i < SLVS_LANE_NUM; i++) {
            if ((link_int_status & ((hi_u32)1U << (16U + i))) != 0U) { // 1: constant 16: shift left format
                slvs_phy_int_state[phy_id].afifo_align_cnt[i]++;
            }

            if ((link_int_status & ((hi_u32)1U << (1U + i * 2U))) != 0U) { // 1: constant 1, 2: shift left format
                slvs_phy_int_state[phy_id].code_err_cnt[i]++;
            }

            if ((link_int_status & ((hi_u32)1U << (i * 2U))) != 0U) { // 1: constant 2: shift left format
                slvs_phy_int_state[phy_id].disp_err_cnt[i]++;
            }
        }
    }
    return;
}

static void slvs_link_int_statis(unsigned int devno)
{
    unsigned int dev_index;
    unsigned int link_int_status;

    link_int_status = slvs_drv_get_link_raw_int(devno);
    if (link_int_status != 0U) {
        slvs_drv_clear_link_raw_int(devno, 0xffffffffU);

        dev_index = devno - SLVS_DEV_NUM_START;

        if ((link_int_status & (hi_u32)SLVS_HD_CRC_ERR) != 0U) {
            slvs_link_int_state[dev_index].header_crc_err_cnt++;
        }

        if ((link_int_status & (hi_u32)SLVS_PLD_CRC_ERR) != 0U) {
            slvs_link_int_state[dev_index].payload_crc_err_cnt++;
        }

        if ((link_int_status & (hi_u32)SLVS_ECC_ERR) != 0U) {
            slvs_link_int_state[dev_index].ecc_err_cnt++;
        }

        if ((link_int_status & (hi_u32)SLVS_DATA_FIFO_W_ERR) != 0U) {
            slvs_link_int_state[dev_index].data_fifo_w_err_cnt++;
        }

        if ((link_int_status & (hi_u32)SLVS_DATA_FIFO_R_ERR) != 0U) {
            slvs_link_int_state[dev_index].data_fifo_r_err_cnt++;
        }

        if ((link_int_status & (hi_u32)SLVS_CMD_FIFO_FULL) != 0U) {
            slvs_link_int_state[dev_index].cmd_fifo_full_err_cnt++;
        }

        if ((link_int_status & (hi_u32)SLVS_SKEW_ERR) != 0U) {
            slvs_link_int_state[dev_index].skew_err_cnt++;
        }

        if ((link_int_status & (hi_u32)SLVS_VSYNC_RAW) != 0U) {
            slvs_link_int_state[dev_index].vsync_cnt++;
        }
    }
}

static int slvs_interrupt_route(int irq, void* dev_id)
{
    unsigned int i = 0;

    /* phy int statistic */
    for (i = 0U; i < SLVS_PHY_NUM; i++) {
        slvs_phy_int_statis(i);
    }

    for (i = SLVS_DEV_NUM_START; i < SLVS_DEV_NUM_START + SLVS_MAX_DEV_NUM; i++) {
        slvs_link_int_statis(i);
    }
    return OSAL_IRQ_HANDLED;
}

static int slvs_ec_register_irq(void)
{
    int ret = HI_SUCCESS;

    ret = osal_request_irq(g_slvs_irq_num, slvs_interrupt_route, NULL, "SLVS_EC", slvs_interrupt_route);
    if (ret < 0) {
        mipi_rx_err_trace("slvs_ec: failed to register irq.\n");
        return -1;
    }
    return ret;
}

static void mipi_unregister_slvs_irq(void)
{
    osal_free_irq(g_slvs_irq_num, slvs_interrupt_route);
}

/* magic nums convert to register configs */
static void slvs_drv_hw_init(void)
{
    mipi_rx_info_trace("slvs_drv_hw_init not support.\n");
}

/* magic nums convert to register configs */
static void slvs_drv_hw_exit(void)
{
    unsigned long mipi_rx_crg_addr;
    mipi_rx_info_trace("slvs_drv_hw_exit not support.\n");
}


int slvs_drv_init(void)
{
    int ret;
    ret = slvs_ec_register_irq();
    if (ret < 0) {
        mipi_rx_err_trace("slvs_ec_register_irq fail!\n");
        goto fail0;
    }

    slvs_drv_hw_init();

    return 0;

fail0:
    return -1;
}

void slvs_rx_drv_exit(void)
{
    mipi_unregister_slvs_irq();
    slvs_drv_hw_exit();
}