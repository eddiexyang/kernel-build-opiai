/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 * Description: hclge_plf_tm
 * Author: huawei
 * Create: 2022-01-20
 */

#include <linux/etherdevice.h>
#include <linux/of.h>

#include "hclge_plf_main.h"
#include "hclge_plf_tm.h"

#ifndef DEFINE_HNS_LLT
#define STATIC static
#else
#define STATIC
#endif

enum hclge_shaper_level {
    HCLGE_SHAPER_LVL_TC = 0,
    HCLGE_SHAPER_LVL_PORT = 1,
    HCLGE_TX_SCH_SHAPER_LVL_TC = 2,
    HCLGE_TX_SCH_SHAPER_LVL_PORT = 3,
    HCLGE_SHAPER_LVL_CNT = 4,
};

static const u16 tick_array[HCLGE_SHAPER_LVL_CNT] = {
    160, /* TC level */
    20, /* Port level */
    80, /* TX_SCH TC level */
    20, /* TX_SCH Port level */
};

#define hclge_tm_write_reg(base, reg, value)                                     \
    do {                                                                         \
        hclge_write_reg(base, reg, value);                                       \
    } while (0)

/* hclge_shaper_para_calc: calculate ir parameter for the shaper
 * @ir: Rate to be config, its unit is Mbps
 * @shaper_level: the shaper level. eg: port, pg, priority, queueset
 * @ir_b: IR_B parameter of IR shaper
 * @ir_u: IR_U parameter of IR shaper
 * @ir_s: IR_S parameter of IR shaper
 * @max_tm_rate: max tm rate is available to config
 * the formula:
 *              IR_b * (2 ^ IR_u) * 8
 * IR(Mbps) = -------------------------  *  CLOCK(250Mbps)
 *              Tick * (2 ^ IR_s)
 * @return: 0: calculate sucessful, negative: fail
 */
static int hclge_plf_shaper_para_calc(u32 ir, u8 shaper_level, struct hclge_shaper_ir_para *ir_para, u32 max_tm_rate)
{
#define SHAPER_DEFAULT_IR_B 126
#define DIVISOR_CLK (250 * 8)
#define DIVISOR_IR_B_126 (126 * DIVISOR_CLK)

    u8 ir_u_calc = 0;
    u8 ir_s_calc = 0;
    u32 ir_calc;
    u32 tick;

    /* Calc tick */
    if (shaper_level >= HCLGE_SHAPER_LVL_CNT || ir > max_tm_rate) {
        return -EINVAL;
    }

    tick = tick_array[shaper_level];

    /**
     * Calc the speed if ir_b = 126, ir_u = 0 and ir_s = 0
     * the formula is changed to:
     *		        126 * 1 * 8
     * ir_calc = ---------------- * 250
     *		        tick * 1
     */
    ir_calc = (DIVISOR_IR_B_126 + (tick >> 1) - 1) / tick;
    if (ir_calc == ir) {
        ir_para->ir_b = SHAPER_DEFAULT_IR_B;
        ir_para->ir_u = 0;
        ir_para->ir_s = 0;

        return 0;
    } else if (ir_calc > ir) {
        /* Increasing the denominator to select ir_s value */
        while (ir_calc >= ir && ir) {
            ir_s_calc++;
            ir_calc = DIVISOR_IR_B_126 / (tick * (1 << ir_s_calc));
        }

        ir_para->ir_b = (ir * tick * (1 << ir_s_calc) + (DIVISOR_CLK >> 1)) / DIVISOR_CLK;
    } else {
        /* Increasing the numerator to select ir_u value */
        u32 numerator;

        while (ir_calc < ir) {
            ir_u_calc++;
            numerator = DIVISOR_IR_B_126 * (1 << ir_u_calc);
            ir_calc = (numerator + (tick >> 1)) / tick;
        }

        if (ir_calc == ir) {
            ir_para->ir_b = SHAPER_DEFAULT_IR_B;
        } else {
            u32 denominator = DIVISOR_CLK * (1 << --ir_u_calc);
            ir_para->ir_b = (ir * tick + (denominator >> 1)) / denominator;
        }
    }

    ir_para->ir_u = ir_u_calc;
    ir_para->ir_s = ir_s_calc;

    return 0;
}

void hclge_plf_tm_prio_tc_info_update(struct hclge_plf_dev *hdev, u8 *prio_tc)
{
    struct hclge_plf_vport *vport = hdev->vport;
    struct hnae3_knic_private_info *kinfo;
    u16 i, j;

    for (i = 0; i < HNAE3_MAX_USER_PRIO; i++) {
        hdev->tm_info.prio_tc[i] = prio_tc[i];

        for (j = 0; j < hdev->num_alloc_vport; j++) {
            kinfo = &vport[j].nic.kinfo;
            kinfo->tc_info.prio_tc[i] = prio_tc[i];
        }
    }

    for (i = 0; i < HNAE3_MAX_TC; i++) {
        if (!(hdev->hw_tc_map & BIT(i)))
            continue;
        hdev->tm_info.tc_info[i].tc_dwrr = hdev->tm_info.pg_info[0].tc_dwrr[i];
    }
}

int hclge_plf_pause_setup_hw(struct hclge_plf_dev *hdev, bool init)
{
    return 0;
}

int hclge_plf_pfc_tx_stats_get(struct hclge_plf_dev *hdev, u64 *stats)
{
    return 0;
}

int hclge_plf_pfc_rx_stats_get(struct hclge_plf_dev *hdev, u64 *stats)
{
    return 0;
}

void hclge_plf_tm_pfc_info_update(struct hclge_plf_dev *hdev)
{}

static void hclge_plf_up_to_tc_map(struct hclge_plf_dev *hdev)
{
    u32 reg_addr;
    u32 data = 0;
    u8 pri_id;

    for (pri_id = 0; pri_id < HNAE3_MAX_USER_PRIO; pri_id++) {
        hnae3_set_field(data, (0x7 << (pri_id * 3)), /* each pri shift 3; mask 7 = (8 - 1) */
            (pri_id * 3), hdev->tm_info.prio_tc[pri_id]); /* each pri shift 3; mask 7 = (8 - 1) */
    }

    reg_addr = RX_PA_REG_BASE + PA_PRI_MAP_TC + hdev->id * 0x30;
    hclge_tm_write_reg(hdev->hw.io_base, reg_addr, data);

    reg_addr = TX_PA_REG_BASE + PA_PRI_MAP_TC + hdev->id * 0x30;
    hclge_tm_write_reg(hdev->hw.io_base, reg_addr, data);
}

static void hclge_plf_tm_tc_to_port_map(struct hclge_plf_dev *hdev)
{
    u32 reg_addr;
    u32 i;

    if (hdev->tx_sch_mode != HCLGE_FLAG_TC_BASE_SCH_MODE)
        return;

    /* TX SCHDULER */
    reg_addr = RCB_COM_REG_BASE + RCB_COM_ETS_TCG_MAPING + hdev->hw.mac.mac_id * 0x4;
    hclge_tm_write_reg(hdev->hw.io_base, reg_addr, (hdev->hw_tc_map << hdev->tc_offset));

    /* TX SCHDULER */
    for (i = 0; i < HNAE3_MAX_TC; i++) {
        if (!(hdev->hw_tc_map & BIT(i)))
            continue;

        reg_addr = RCB_COM_REG_BASE + RCB_COM_ETS_TC_LINK_TCG + (i + hdev->tc_offset) * 0x4;
        hclge_tm_write_reg(hdev->hw.io_base, reg_addr, hdev->hw.mac.mac_id);
    }
}

STATIC void hclge_plf_tm_queue_to_tc_map(void __iomem *io_base, u32 tqp_offset, u32 tqp_max, u32 tc_offset, u32 tc_max)
{
    u32 tqp_rss = tqp_max / tc_max;
    u32 tqp_index;
    u32 reg_addr;
    u32 i, j;
    u32 data;

    for (i = 0; i < tc_max; i++) {
        reg_addr = RCB_COM_REG_BASE + RCB_COM_ETS_TC_MAPING + (i + tc_offset) * 0x4;
        writel(0, (void __iomem *)(io_base + reg_addr));

        for (j = 0; j < tqp_rss; j++) {
            tqp_index = j + (i * tqp_rss) + tqp_offset;

            /* TX SCHDULER */
            reg_addr = RCB_COM_REG_BASE + RCB_COM_ETS_TC_MAPING + (i + tc_offset) * 0x4;
            data = hclge_read_reg(io_base, reg_addr);
            hnae3_set_field(data, (1 << tqp_index), tqp_index, 1);
            hclge_tm_write_reg(io_base, reg_addr, data);

            reg_addr = RCB_COM_REG_BASE + RCB_COM_ETS_QUEUE_LINK_TC + tqp_index * 0x4;
            hclge_tm_write_reg(io_base, reg_addr, (i + tc_offset));
        }
    }
}

static void hclge_plf_tm_map_cfg(struct hclge_plf_dev *hdev)
{
    hclge_plf_up_to_tc_map(hdev);

    hclge_plf_tm_queue_to_tc_map(hdev->hw.io_base, hdev->tqp_offset,
        hdev->num_tqps, hdev->tc_offset, hdev->tm_info.num_tc);

    hclge_plf_tm_tc_to_port_map(hdev);
}

STATIC u32 hclge_plf_get_tc_speed(struct hclge_plf_dev *hdev)
{
    u32 tc_speed_sel;

    switch (hdev->hw.mac.speed) {
        case HCLGE_MAC_SPEED_10M:
            tc_speed_sel = 0; /* 0 : 10Mbps */
            break;
        case HCLGE_MAC_SPEED_100M:
            tc_speed_sel = 1; /* 1 : 100Mbps */
            break;
        case HCLGE_MAC_SPEED_1G:
            tc_speed_sel = 2; /* 2 : 1000Mbps */
            break;
        case HCLGE_MAC_SPEED_2_5G:
            tc_speed_sel = 3; /* 3 : 2500Mbps */
            break;
        default:
            tc_speed_sel = 2; /* 2 : 1000Mbps */
            break;
    }

    return tc_speed_sel;
}

STATIC void hclge_plf_tm_cbs_cfg(struct hclge_plf_dev *hdev, u32 tc)
{
    u32 ssu_tc_offset = 0;
    u32 sendslope;
    u32 idleslope;
    u32 reg_addr;

    ssu_tc_offset = tc + (hdev->id * 8); /* Each port of ssu has fixed 8 TC */
    /* Formula：400 * （1 - speed limit ratio）= 4 * (100 - speed limit weight) */
    sendslope = 4 * (100 - hdev->tm_info.tc_info[tc].tc_dwrr);
    /* Formula：400 * speed limit ratio= 4 * speed limit weight */
    idleslope = 4 * hdev->tm_info.tc_info[tc].tc_dwrr;

    reg_addr = SSU_REG_BASE + SSU_MAC_ETS_TC_SENDSLOPE + ssu_tc_offset * 0x4;
    hclge_tm_write_reg(hdev->hw.io_base, reg_addr, sendslope);
    reg_addr = SSU_REG_BASE + SSU_MAC_ETS_TC_IDLESLOPE + ssu_tc_offset * 0x4;
    hclge_tm_write_reg(hdev->hw.io_base, reg_addr, idleslope);
    reg_addr = SSU_REG_BASE + SSU_MAC_ETS_TC_HICREDIT + ssu_tc_offset * 0x4;
    hclge_tm_write_reg(hdev->hw.io_base, reg_addr, 0xffffffff);
    reg_addr = SSU_REG_BASE + SSU_MAC_ETS_TC_LOCREDIT + ssu_tc_offset * 0x4;
    hclge_tm_write_reg(hdev->hw.io_base, reg_addr, 0xffffffff);
    reg_addr = SSU_REG_BASE + SSU_MAC_ETS_TC_SPEED_SEL + ssu_tc_offset * 0x4;
    hclge_tm_write_reg(hdev->hw.io_base, reg_addr, hclge_plf_get_tc_speed(hdev));
    reg_addr = SSU_REG_BASE + SSU_MAC_ETS_TC_CBS_EN + ssu_tc_offset * 0x4;
    hclge_tm_write_reg(hdev->hw.io_base, reg_addr, 1);

    reg_addr = RCB_COM_REG_BASE + RCB_COMM_ETS_TC_SENDSLOPE + (tc + hdev->tc_offset) * 0x4;
    hclge_tm_write_reg(hdev->hw.io_base, reg_addr, sendslope);
    reg_addr = RCB_COM_REG_BASE + RCB_COMM_ETS_TC_IDLESLOPE + (tc + hdev->tc_offset) * 0x4;
    hclge_tm_write_reg(hdev->hw.io_base, reg_addr, idleslope);
    reg_addr = RCB_COM_REG_BASE + RCB_COMM_ETS_TC_HICREDIT + (tc + hdev->tc_offset) * 0x4;
    hclge_tm_write_reg(hdev->hw.io_base, reg_addr, 0xffffffff);
    reg_addr = RCB_COM_REG_BASE + RCB_COMM_ETS_TC_LOCREDIT + (tc + hdev->tc_offset) * 0x4;
    hclge_tm_write_reg(hdev->hw.io_base, reg_addr, 0xffffffff);
    reg_addr = RCB_COM_REG_BASE + RCB_COMM_ETS_TC_SPEED_SEL + (tc + hdev->tc_offset) * 0x4;
    hclge_tm_write_reg(hdev->hw.io_base, reg_addr, hclge_plf_get_tc_speed(hdev));
    reg_addr = RCB_COM_REG_BASE + RCB_COMM_ETS_TC_CBS_EN + (tc + hdev->tc_offset) * 0x4;
    hclge_tm_write_reg(hdev->hw.io_base, reg_addr, 1);
}

static void hclge_plf_tm_disable_cbs(struct hclge_plf_dev *hdev, u32 tc)
{
    u32 ssu_tc_offset = 0;
    u32 reg_addr;

    ssu_tc_offset = tc + (hdev->id * 8); /* Each port of ssu has fixed 8 TC */
    reg_addr = SSU_REG_BASE + SSU_MAC_ETS_TC_CBS_EN + ssu_tc_offset * 0x4;
    hclge_tm_write_reg(hdev->hw.io_base, reg_addr, 0);

    reg_addr = RCB_COM_REG_BASE + RCB_COMM_ETS_TC_CBS_EN + (tc + hdev->tc_offset) * 0x4;
    hclge_tm_write_reg(hdev->hw.io_base, reg_addr, 0);
}

static void hclge_plf_tm_ets_tc_dwrr_cfg(struct hclge_plf_dev *hdev)
{
    u32 ssu_tc_offset = 0;
    u32 reg_addr;
    u32 i;

    for (i = 0; i < HNAE3_MAX_TC; i++) {
        if (!(hdev->hw_tc_map & BIT(i)))
            continue;

        if (hdev->tm_info.tc_info[i].tc_sch_mode == HCLGE_SCH_MODE_CBS) {
            hclge_plf_tm_cbs_cfg(hdev, i);
            continue;
        }

        hclge_plf_tm_disable_cbs(hdev, i);

        ssu_tc_offset = i + (hdev->id * 8); /* Each port of ssu has fixed 8 TC */
        reg_addr = SSU_REG_BASE + SSU_MAC_ETS_TC_WEIGHT + ssu_tc_offset * 0x4;
        if (hdev->tm_info.tc_info[i].tc_sch_mode == HCLGE_SCH_MODE_DWRR)
            hclge_tm_write_reg(hdev->hw.io_base, reg_addr, hdev->tm_info.tc_info[i].tc_dwrr);
        else
            hclge_write_reg(hdev->hw.io_base, reg_addr, 0);

        reg_addr = RCB_COM_REG_BASE + RCB_COM_ETS_TC_WEIGHT + (i + hdev->tc_offset) * 0x4;
        if (hdev->tm_info.tc_info[i].tc_sch_mode == HCLGE_SCH_MODE_DWRR)
            hclge_tm_write_reg(hdev->hw.io_base, reg_addr, hdev->tm_info.tc_info[i].tc_dwrr);
        else
            hclge_write_reg(hdev->hw.io_base, reg_addr, 0);
    }
}

int hclge_plf_tm_dwrr_cfg(struct hclge_plf_dev *hdev)
{
    if (hdev->tx_sch_mode == HCLGE_FLAG_TC_BASE_SCH_MODE) {
        if (!hnae3_dev_dcb_supported(hdev))
            return 0;

        hclge_plf_tm_ets_tc_dwrr_cfg(hdev);
    }

    return 0;
}

static int hclge_plf_tm_tc_shaper_cfg(struct hclge_plf_dev *hdev)
{
    u32 max_tm_rate = hdev->ae_dev->dev_specs.max_tm_rate;
    struct hclge_shaper_ir_para ir_para;
    u32 ssu_tc_offset = 0;
    u32 reg_addr;
    u32 data = 0;
    int ret;
    u32 i;

    for (i = 0; i < hdev->tm_info.num_tc; i++) {
        u32 rate = hdev->hw.mac.speed;
        ret = hclge_plf_shaper_para_calc(rate, HCLGE_SHAPER_LVL_TC, &ir_para, max_tm_rate);
        if (ret)
            return ret;

        ssu_tc_offset = i + (hdev->id * 8); /* Each port of ssu has fixed 8 TC */
        /* ETS TX */
        reg_addr = SSU_REG_BASE + SSU_MAC_ETS_TC_SHAPING + ssu_tc_offset * 0x4;
        data = hclge_read_reg(hdev->hw.io_base, reg_addr);
        data &= 0xffff0000;
        hnae3_set_field(data, 0xff, 0, ir_para.ir_b);
        hnae3_set_field(data, (0xf << HCLGE_TM_SHAP_IR_U_LSH), HCLGE_TM_SHAP_IR_U_LSH, ir_para.ir_u);
        hnae3_set_field(data, (0xf << HCLGE_TM_SHAP_IR_S_LSH), HCLGE_TM_SHAP_IR_S_LSH, ir_para.ir_s);
        hclge_tm_write_reg(hdev->hw.io_base, reg_addr, data);

        ret = hclge_plf_shaper_para_calc(rate, HCLGE_TX_SCH_SHAPER_LVL_TC, &ir_para, max_tm_rate);
        if (ret)
            return ret;

        /* TX SCHEDULER */
        reg_addr = RCB_COM_REG_BASE + RCB_COM_ETS_TC_SHAPING + (i + hdev->tc_offset) * 0x4;
        data = hclge_read_reg(hdev->hw.io_base, reg_addr);
        data &= 0xffff0000;
        hnae3_set_field(data, 0xff, 0, ir_para.ir_b);
        hnae3_set_field(data, (0xf << HCLGE_TM_SHAP_IR_U_LSH), HCLGE_TM_SHAP_IR_U_LSH, ir_para.ir_u);
        hnae3_set_field(data, (0xf << HCLGE_TM_SHAP_IR_S_LSH), HCLGE_TM_SHAP_IR_S_LSH, ir_para.ir_s);
        hclge_tm_write_reg(hdev->hw.io_base, reg_addr, data);
    }

    return 0;
}

STATIC int hclge_plf_tm_port_shaper_ets_cfg(struct hclge_plf_dev *hdev)
{
    struct hclge_shaper_ir_para ir_para;
    u32 reg_addr;
    u32 data = 0;
    int ret;

    ret = hclge_plf_shaper_para_calc(
        hdev->hw.mac.speed, HCLGE_SHAPER_LVL_PORT, &ir_para, hdev->ae_dev->dev_specs.max_tm_rate);
    if (ret)
        return ret;

    /* ETS TX port */
    reg_addr = SSU_REG_BASE + SSU_MAC_ETS_PORT_SHAPING + hdev->hw.mac.mac_id * 0x4;
    data = hclge_read_reg(hdev->hw.io_base, reg_addr);
    data &= 0xffff0000;
    hnae3_set_field(data, 0xff, 0, ir_para.ir_b);
    hnae3_set_field(data, (0xf << HCLGE_TM_SHAP_IR_U_LSH), HCLGE_TM_SHAP_IR_U_LSH, ir_para.ir_u);
    hnae3_set_field(data, (0xf << HCLGE_TM_SHAP_IR_S_LSH), HCLGE_TM_SHAP_IR_S_LSH, ir_para.ir_s);
    hclge_tm_write_reg(hdev->hw.io_base, reg_addr, data);

    /* ETS TX tcg */
    reg_addr = SSU_REG_BASE + SSU_MAC_ETS_TCG_SHAPING + hdev->hw.mac.mac_id * 0x4;
    data = hclge_read_reg(hdev->hw.io_base, reg_addr);
    data &= 0xffff0000;
    hnae3_set_field(data, 0xff, 0, ir_para.ir_b);
    hnae3_set_field(data, (0xf << HCLGE_TM_SHAP_IR_U_LSH), HCLGE_TM_SHAP_IR_U_LSH, ir_para.ir_u);
    hnae3_set_field(data, (0xf << HCLGE_TM_SHAP_IR_S_LSH), HCLGE_TM_SHAP_IR_S_LSH, ir_para.ir_s);
    hclge_tm_write_reg(hdev->hw.io_base, reg_addr, data);

    return 0;
}

STATIC int hclge_plf_tm_port_shaper_txsch_cfg(struct hclge_plf_dev *hdev)
{
/* tune the RCB shaper with suitable value */
#define RCB_SHAPER_CFG_PARAM_10M 10
#define RCB_SHAPER_CFG_PARAM_100M 99
#define RCB_SHAPER_CFG_PARAM_1G 987
#define RCB_SHAPER_CFG_PARAM_2_5G 2475
    struct hclge_shaper_ir_para ir_para;
    u32 reg_addr;
    u32 data = 0;
    u32 speed;
    int ret;

    switch (hdev->hw.mac.speed) {
        case HCLGE_MAC_SPEED_10M:
            speed = RCB_SHAPER_CFG_PARAM_10M;
            break;
        case HCLGE_MAC_SPEED_100M:
            speed = RCB_SHAPER_CFG_PARAM_100M;
            break;
        case HCLGE_MAC_SPEED_1G:
            speed = RCB_SHAPER_CFG_PARAM_1G;
            break;
        case HCLGE_MAC_SPEED_2_5G:
            speed = RCB_SHAPER_CFG_PARAM_2_5G;
            break;
        default:
            speed = RCB_SHAPER_CFG_PARAM_1G;
            break;
    }

    ret = hclge_plf_shaper_para_calc(
        speed, HCLGE_TX_SCH_SHAPER_LVL_PORT, &ir_para, hdev->ae_dev->dev_specs.max_tm_rate);
    if (ret)
        return ret;

    /* TX SCHDULER port */
    reg_addr = RCB_COM_REG_BASE + RCB_COM_ETS_PORT_SHAPING + hdev->hw.mac.mac_id * 0x4;
    data = hclge_read_reg(hdev->hw.io_base, reg_addr);
    data &= 0xffff0000;
    hnae3_set_field(data, 0xff, 0, ir_para.ir_b);
    hnae3_set_field(data, (0xf << HCLGE_TM_SHAP_IR_U_LSH), HCLGE_TM_SHAP_IR_U_LSH, ir_para.ir_u);
    hnae3_set_field(data, (0xf << HCLGE_TM_SHAP_IR_S_LSH), HCLGE_TM_SHAP_IR_S_LSH, ir_para.ir_s);
    hclge_tm_write_reg(hdev->hw.io_base, reg_addr, data);

    /* TX SCHDULER tcg */
    reg_addr = RCB_COM_REG_BASE + RCB_COM_ETS_TCG_SHAPING + hdev->hw.mac.mac_id * 0x4;
    data = hclge_read_reg(hdev->hw.io_base, reg_addr);
    data &= 0xffff0000;
    hnae3_set_field(data, 0xff, 0, ir_para.ir_b);
    hnae3_set_field(data, (0xf << HCLGE_TM_SHAP_IR_U_LSH), HCLGE_TM_SHAP_IR_U_LSH, ir_para.ir_u);
    hnae3_set_field(data, (0xf << HCLGE_TM_SHAP_IR_S_LSH), HCLGE_TM_SHAP_IR_S_LSH, ir_para.ir_s);
    hclge_tm_write_reg(hdev->hw.io_base, reg_addr, data);

    return 0;
}

static int hclge_plf_tm_port_shaper_cfg(struct hclge_plf_dev *hdev)
{
    int ret;

    ret = hclge_plf_tm_port_shaper_ets_cfg(hdev);
    if (ret)
        return ret;

    return hclge_plf_tm_port_shaper_txsch_cfg(hdev);
}

STATIC int hclge_plf_tm_shaper_cfg(struct hclge_plf_dev *hdev)
{
    int ret;

    ret = hclge_plf_tm_port_shaper_cfg(hdev);
    if (ret)
        return ret;

    return hclge_plf_tm_tc_shaper_cfg(hdev);
}

int hclge_plf_tm_schd_setup_hw(struct hclge_plf_dev *hdev)
{
    int ret;

    /* Cfg tm mapping  */
    hclge_plf_tm_map_cfg(hdev);

    /* Cfg tm shaper */
    ret = hclge_plf_tm_shaper_cfg(hdev);
    if (ret)
        return ret;

    /* Cfg dwrr */
    return hclge_plf_tm_dwrr_cfg(hdev);
}

int hclge_plf_tm_init_hw(struct hclge_plf_dev *hdev, bool init)
{
    int ret;

    if (hdev->tx_sch_mode != HCLGE_FLAG_TC_BASE_SCH_MODE)
        return -ENOTSUPP;

    ret = hclge_plf_tm_schd_setup_hw(hdev);
    if (ret)
        return ret;

    ret = hclge_plf_pause_setup_hw(hdev, init);
    if (ret)
        return ret;

    return 0;
}

static void hclge_plf_tm_tc_info_init(struct hclge_plf_dev *hdev)
{
#define BW_PERCENT 100
    u8 i;

    for (i = 0; i < hdev->tm_info.num_tc; i++) {
        hdev->tm_info.tc_info[i].tc_id = i;
        hdev->tm_info.tc_info[i].tc_sch_mode = HCLGE_SCH_MODE_DWRR;
        hdev->tm_info.tc_info[i].pgid = 0;
    }

    for (i = 0; i < HNAE3_MAX_USER_PRIO; i++)
        hdev->tm_info.prio_tc[i] = (i >= hdev->tm_info.num_tc) ? 0 : i;

    for (i = 0; i < hdev->tm_info.num_tc; i++)
        hdev->tm_info.tc_info[i].tc_dwrr = BW_PERCENT;
    for (; i < HNAE3_MAX_TC; i++)
        hdev->tm_info.tc_info[i].tc_dwrr = 0;

    /* DCB is enabled if we have more than 1 TC is
     * non-zero.
     */
    if (hdev->tm_info.num_tc > 1)
        hdev->flag |= HCLGE_FLAG_DCB_ENABLE;
    else
        hdev->flag &= ~HCLGE_FLAG_DCB_ENABLE;
}

static u16 hclge_plf_mqprio_get_max_rss_size(struct hclge_plf_vport *vport)
{
    struct hnae3_knic_private_info *kinfo = &vport->nic.kinfo;
    u16 max_rss_size = 0;
    u8 i;

    for (i = 0; i < kinfo->tc_info.num_tc; i++)
        if (max_rss_size < kinfo->tc_info.tqp_count[i])
            max_rss_size = kinfo->tc_info.tqp_count[i];

    return max_rss_size;
}

static u16 hclge_plf_vport_get_max_rss_size(struct hclge_plf_vport *vport)
{
    struct hnae3_knic_private_info *kinfo = &vport->nic.kinfo;

    if (kinfo->tc_info.mqprio_active)
        return hclge_plf_mqprio_get_max_rss_size(vport);
    else
        return vport->alloc_tqps / kinfo->tc_info.num_tc;
}

static u16 hclge_plf_vport_get_tqp_num(struct hclge_plf_vport *vport)
{
    struct hnae3_knic_private_info *kinfo = &vport->nic.kinfo;
    u8 num_tc = kinfo->tc_info.num_tc;
    int sum = 0;
    int i;

    if (kinfo->tc_info.mqprio_active) {
        for (i = 0; i < num_tc; i++)
            sum += kinfo->tc_info.tqp_count[i];
        return sum;
    } else {
        return kinfo->tc_info.num_tc * kinfo->rss_size;
    }
}

static u16 hclge_plf_vport_get_tc_tqp_offset(struct hclge_plf_vport *vport, u8 tc_index)
{
    struct hnae3_knic_private_info *kinfo = &vport->nic.kinfo;

    if (kinfo->tc_info.mqprio_active)
        return kinfo->tc_info.tqp_offset[tc_index];
    else
        return tc_index * kinfo->rss_size;
}

static u16 hclge_plf_vport_get_tc_tqp_count(struct hclge_plf_vport *vport, u8 tc_index)
{
    struct hnae3_knic_private_info *kinfo = &vport->nic.kinfo;

    if (kinfo->tc_info.mqprio_active)
        return kinfo->tc_info.tqp_count[tc_index];
    else
        return kinfo->rss_size;
}

static void hclge_plf_tm_update_kinfo_rss_size(struct hclge_plf_vport *vport)
{
    struct hnae3_knic_private_info *kinfo = &vport->nic.kinfo;
    struct hclge_plf_dev *hdev = vport->back;
    u16 vport_max_rss_size;
    u16 max_rss_size;

    kinfo->tc_info.num_tc = min_t(u16, vport->alloc_tqps, hdev->tm_info.num_tc);
    vport_max_rss_size = hdev->pf_rss_size_max;

    max_rss_size = hclge_plf_vport_get_max_rss_size(vport);
    max_rss_size = min_t(u16, vport_max_rss_size, max_rss_size);
    /* Set to user value, no larger than max_rss_size. */
    if (kinfo->req_rss_size != kinfo->rss_size && kinfo->req_rss_size && kinfo->req_rss_size <= max_rss_size) {
        dev_info(&hdev->pdev->dev, "rss changes from %d to %d\n", kinfo->rss_size, kinfo->req_rss_size);
        kinfo->rss_size = kinfo->req_rss_size;
    } else if (kinfo->rss_size > max_rss_size || (!kinfo->req_rss_size && kinfo->rss_size < max_rss_size)) {
        /* Set to the maximum specification value (max_rss_size). */
        kinfo->rss_size = max_rss_size;
    }
}

static void hclge_plf_tm_vport_tc_info_update(struct hclge_plf_vport *vport)
{
    struct hnae3_knic_private_info *kinfo = &vport->nic.kinfo;
    struct hclge_plf_dev *hdev = vport->back;
    u8 i;

    hclge_plf_tm_update_kinfo_rss_size(vport);
    kinfo->num_tqps = hclge_plf_vport_get_tqp_num(vport);
    vport->dwrr = 100; /* 100 percent as init */
    vport->alloc_rss_size = kinfo->rss_size;

    for (i = 0; i < HNAE3_MAX_TC; i++) {
        if ((hdev->hw_tc_map & BIT(i)) && i < kinfo->tc_info.num_tc) {
            kinfo->tc_info.tqp_offset[i] = hclge_plf_vport_get_tc_tqp_offset(vport, i);
            kinfo->tc_info.tqp_count[i] = hclge_plf_vport_get_tc_tqp_count(vport, i);
        } else {
            /* Set to default queue if TC is disable */
            kinfo->tc_info.tqp_offset[i] = 0;
            kinfo->tc_info.tqp_count[i] = 1;
        }
    }

    (void)memcpy_s(kinfo->tc_info.prio_tc, sizeof_field(struct hnae3_tc_info, prio_tc),
                   hdev->tm_info.prio_tc, sizeof_field(struct hnae3_tc_info, prio_tc));
}

int hclge_plf_tm_vport_map_update(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_vport *vport = hdev->vport;
    struct hnae3_knic_private_info *kinfo = &vport->nic.kinfo;

    hclge_plf_tm_vport_tc_info_update(vport);

    hclge_plf_tm_queue_to_tc_map(hdev->hw.io_base, hdev->tqp_offset,
        kinfo->num_tqps, hdev->tc_offset, hdev->tm_info.num_tc);

    return 0;
}

static void hclge_plf_tm_schd_info_init(struct hclge_plf_dev *hdev)
{
    hclge_plf_tm_tc_info_init(hdev);

    hclge_plf_tm_vport_tc_info_update(hdev->vport);
}

void hclge_plf_tm_schd_info_update(struct hclge_plf_dev *hdev, u8 num_tc)
{
    u8 bit_map = 0;
    u8 i;

    hdev->tm_info.num_tc = num_tc;

    for (i = 0; i < hdev->tm_info.num_tc; i++)
        bit_map |= BIT(i);

    if (!bit_map) {
        bit_map = 1;
        hdev->tm_info.num_tc = 1;
    }

    hdev->hw_tc_map = bit_map;

    hclge_plf_tm_schd_info_init(hdev);
}

/* 1:1 */
STATIC void hclge_plf_tm_init_queue_to_tc_map(struct hclge_plf_dev *hdev)
{
#define SSU_MAX_TC_NUM 32
    u32 reg_addr;
    u32 i;

    for (i = 0; i < SSU_MAX_TC_NUM; i++) {
        /* ETS TX */
        reg_addr = SSU_REG_BASE + SSU_MAC_ETS_TC_MAPING + i * 0x4;
        hclge_tm_write_reg(hdev->hw.io_base, reg_addr, (1 << i));

        /* ETS RX */
        reg_addr = SSU_REG_BASE + SSU_HOST_ETS_TC_MAPING + i * 0x4;
        hclge_tm_write_reg(hdev->hw.io_base, reg_addr, (1 << i));

        reg_addr = SSU_REG_BASE + SSU_MAC_ETS_QUEUE_LINK_TC_CFG + i * 0x4;
        hclge_tm_write_reg(hdev->hw.io_base, reg_addr, i);

        reg_addr = SSU_REG_BASE + SSU_HOST_ETS_QUEUE_LINK_TC_CFG + i * 0x4;
        hclge_tm_write_reg(hdev->hw.io_base, reg_addr, i);
    }
}

STATIC void hclge_plf_tm_init_tcg_to_port_map(struct hclge_plf_dev *hdev)
{
#define MAX_PORT 4
    u32 reg_addr;
    u32 i;

    /* TCG : Port ： 1 ：1 */
    for (i = 0; i < MAX_PORT; i++) {
        /* ETS TX */
        reg_addr = SSU_REG_BASE + SSU_MAC_ETS_PORT_MAPING + i * 0x4;
        hclge_tm_write_reg(hdev->hw.io_base, reg_addr, (1 << i));

        /* ETS RX */
        reg_addr = SSU_REG_BASE + SSU_HOST_ETS_PORT_MAPING + i * 0x4;
        hclge_tm_write_reg(hdev->hw.io_base, reg_addr, (1 << i));

        /* TX SCHDULER */
        reg_addr = RCB_COM_REG_BASE + RCB_COM_ETS_PORT_MAPING + i * 0x4;
        hclge_tm_write_reg(hdev->hw.io_base, reg_addr, (1 << i));

        /* TX SCHDULER */
        reg_addr = RCB_COM_REG_BASE + RCB_COM_ETS_TCG_LINK_PORT + i * 0x4;
        hclge_tm_write_reg(hdev->hw.io_base, reg_addr, i);
    }
}

STATIC void hclge_plf_tm_init_tc_to_port_map(struct hclge_plf_dev *hdev)
{
#define MAX_PORT 4
    u32 reg_addr;
    u32 i;

    for (i = 0; i < MAX_PORT; i++) {
        reg_addr = SSU_REG_BASE + SSU_MAC_ETS_TCG_MAPING + i * 0x4;
        hclge_tm_write_reg(hdev->hw.io_base, reg_addr, (0xff << (i * 8))); // 约束：每个port固定8个TC

        reg_addr = SSU_REG_BASE + SSU_HOST_ETS_TCG_MAPING + i * 0x4;
        hclge_tm_write_reg(hdev->hw.io_base, reg_addr, (0xff << (i * 8))); // 约束：每个port固定8个TC
    }
}

STATIC void hclge_plf_tm_init_ets_up_len(struct hclge_plf_dev *hdev)
{
#define RCB_MAX_QUEUE_NUM 16
    u32 reg_addr;
    u32 data;
    u32 i;

    reg_addr = SSU_REG_BASE + SSU_ETS_UP_LENTH_OFFSET;
    data = hclge_read_reg(hdev->hw.io_base, reg_addr);
    for (i = 0; i < RCB_MAX_QUEUE_NUM; i++) {
        reg_addr = RCB_COM_REG_BASE + RCB_COMM_ETS_QUEUE_OFFSET_LEN + i * 0x4;
        hclge_tm_write_reg(hdev->hw.io_base, reg_addr, data);
    }
}

STATIC void hclge_plf_tm_global_init(struct hclge_plf_dev *hdev)
{
    /* MPF */
    if (hdev->tx_sch_mode != HCLGE_FLAG_TC_BASE_SCH_MODE || hdev->id != hclge_plf_get_first_probe_id())
        return;

    /* TCG : Port ： 1 ：1 */
    hclge_plf_tm_init_tcg_to_port_map(hdev);

    /* ssu queue : tc 1:1 */
    hclge_plf_tm_init_queue_to_tc_map(hdev);

    /* ssu TC : TCG&Port 8:1 */
    hclge_plf_tm_init_tc_to_port_map(hdev);

    hclge_plf_tm_init_ets_up_len(hdev);
}

int hclge_plf_tm_schd_init(struct hclge_plf_dev *hdev)
{
    /* fc_mode is HCLGE_FC_FULL on reset */
    hdev->tm_info.fc_mode = HCLGE_FC_FULL;
    hdev->fc_mode_last_time = hdev->tm_info.fc_mode;

    if (hdev->tx_sch_mode != HCLGE_FLAG_TC_BASE_SCH_MODE)
        return -EINVAL;

    hclge_plf_tm_global_init(hdev);

    hclge_plf_tm_schd_info_init(hdev);

    return hclge_plf_tm_init_hw(hdev, true);
}
