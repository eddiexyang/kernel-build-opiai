/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
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
#include <linux/delay.h>
#include <linux/securec.h>

#include "agentdrv_dfx.h"
#include "agentdrv_unit.h"
#include "devdrv_util.h"

struct agentdrv_dfx_log_info g_dfx_info;

struct AGENTDRV_LTSSM_STATE_T g_ltssm_state_enum[] = {
    { 0x0, "ltssm_init" },
    { 0x1, "reset_pipe_afifo" },
    { 0x2, "detect_quiet" },
    { 0x3, "detect_active" },
    { 0x4, "detect_wait" },
    { 0x5, "detect_pwr_p0" },
    { 0x6, "poll_active" },
    { 0x7, "poll_config" },
    { 0x8, "poll_comp" },
    { 0x9, "config_lw_str" },
    { 0xa, "config_lw_acc" },
    { 0xb, "config_ln_wait" },
    { 0xc, "config_ln_acc" },
    { 0xd, "config_complete" },
    { 0xe, "config_idle1" },
    { 0xf, "config_idle2" },
    { 0x10, "l0" },
    { 0x11, "rx_l0s" },
    { 0x14, "tx_l0s_entry" },
    { 0x15, "tx_l0s_idle" },
    { 0x16, "tx_l0s_fts" },
    { 0x17, "l1" },
    { 0x18, "l2" },
    { 0x19, "tx_beacon_begin" },
    { 0x1a, "tx_beacon_end" },
    { 0x20, "recovery_lock" },
    { 0x21, "recovery_cfg" },
    { 0x22, "recovery_speed" },
    { 0x23, "recovery_idle1" },
    { 0x24, "recovery_idle2" },
    { 0x25, "recovery_eq_p0" },
    { 0x26, "recovery_eq_p1" },
    { 0x27, "recovery_eq_p2" },
    { 0x28, "recovery_eq_p3" },
    { 0x30, "tx_eios_st" },
    { 0x31, "change_rate_gen1" },
    { 0x32, "change_power" },
    { 0x33, "hot_reset" },
    { 0x34, "disable_phase1" },
    { 0x35, "disable_phase2" },
    { 0x38, "loopback_entry" },
    { 0x39, "loopback_active" },
    { 0x3a, "oopback_exit" },
};

void agentdrv_notify_blackbox_exception(u32 dev_id, u32 excep_id, const char *str)
{
    /* when we call dfm_system_error_report, dfm will call agentdrv_dfm_dump for bbox_storage_str,
       so if notify str is equal to bbox_storage_str, we don't need copy to bbox_storage_str, otherwise copy */
    if (g_dfx_info.bbox_storage_str != str) {
        agentdrv_dfx_put_string(str);
    }

    dfm_system_error_report_ex(dev_id, excep_id, 0);
}

/* value: 0,success; 1,fail */
int agentdrv_dfx_init_struct(void)
{
    g_dfx_info.bbox_storage_str = (char *)kzalloc(AGENTDRV_BLACK_BOX_BUF_LEN, GFP_KERNEL);
    if (g_dfx_info.bbox_storage_str == NULL) {
        devdrv_err("Call alloc failed, bbox_storage_str is null.\n");
        return -ENOMEM;
    }
    spin_lock_init(&(g_dfx_info.spinlock));

    return 0;
}

void agentdrv_dfx_free_struct(void)
{
    kfree(g_dfx_info.bbox_storage_str);
    g_dfx_info.bbox_storage_str = NULL;
}

/* set dl bp en for cycle record dfx info into str */
void agentdrv_set_dl_bp_en(struct agentdrv_devctrl *agent_dev)
{
    u32 reg_val;
    reg_val = readl(agent_dev->platform_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_DL_BP_EN);
    reg_val |= BIT(AGENTDRV_REG_BIT_19);
    writel(reg_val, agent_dev->platform_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_DL_BP_EN);
}

void agentdrv_dfx_put_string(const char *srcStr)
{
    int bbox_store_size_left;
    int snprintf_s_ret;
    unsigned long flags = 0;

    spin_lock_irqsave(&(g_dfx_info.spinlock), flags);
    bbox_store_size_left = AGENTDRV_BLACK_BOX_BUF_LEN - g_dfx_info.bbox_storage_str_offset;
    snprintf_s_ret = snprintf_s(g_dfx_info.bbox_storage_str + g_dfx_info.bbox_storage_str_offset, bbox_store_size_left,
                                bbox_store_size_left - 1, "%s", srcStr);
    if (snprintf_s_ret != -1) {
        g_dfx_info.bbox_storage_str_offset += snprintf_s_ret;
    }
    spin_unlock_irqrestore(&(g_dfx_info.spinlock), flags);
}

/* start of dfx linkdown */
STATIC void agentdrv_dfx_linkdown_get_info(const struct agentdrv_platform_dev *p_dev)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
    ktime_get_real_ts64(&(g_dfx_info.current_time));
#else
    do_gettimeofday(&(g_dfx_info.current_time));
#endif

    g_dfx_info.dfx_linkdown_info.bme_err = readl(p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_INT_STATUS0_DFX);
    g_dfx_info.dfx_linkdown_info.cfg_bme_en = readl(p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + 0x4);
    g_dfx_info.dfx_linkdown_info.tl_tx_int = readl(p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_INT_STATUS1_DFX);
    g_dfx_info.dfx_linkdown_info.tl_fc_left =
        readl(p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_TX_VC0_P_FC_LEFT);
    g_dfx_info.dfx_linkdown_info.tl_tx_fifo_sts =
        readl(p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_TX_FIFO_STS);
    g_dfx_info.dfx_linkdown_info.tl_tx_msg_cnt = readl(p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_TX_MSG_CNT);
    g_dfx_info.dfx_linkdown_info.tl_tx_total_cnt =
        readl(p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_RX_TOTAL_CNT);
    g_dfx_info.dfx_linkdown_info.mac_bp_timer =
        readl(p_dev->io_base + AGENTDRV_PCIE_DL_REG + AGENTDRV_DFX_MAC_BP_TIMER);
    g_dfx_info.dfx_linkdown_info.mac_link_info = readl(p_dev->io_base + AGENTDRV_PCIE_MAC + AGENTDRV_MAC_REG_LINK_INFO);
    g_dfx_info.dfx_linkdown_info.mac_symbol_unlocl_counter =
        readl(p_dev->io_base + AGENTDRV_PCIE_MAC + AGENTDRV_MAC_SYMBOL_UNLOCL_COUNTER);
    g_dfx_info.dfx_linkdown_info.mac_debug_pipe9 =
        readl(p_dev->io_base + AGENTDRV_PCIE_MAC + AGENTDRV_MAC_REG_DEBUG_PIPE9);
    g_dfx_info.dfx_linkdown_info.mac_debug_pipe10 =
        readl(p_dev->io_base + AGENTDRV_PCIE_MAC + AGENTDRV_MAC_REG_DEBUG_PIPE10);
    g_dfx_info.dfx_linkdown_info.mac_debug_pipe11 =
        readl(p_dev->io_base + AGENTDRV_PCIE_MAC + AGENTDRV_MAC_REG_DEBUG_PIPE11);
    g_dfx_info.dfx_linkdown_info.mac_pcs_err_cnt =
        readl(p_dev->io_base + AGENTDRV_PCIE_MAC + AGENTDRV_MAC_PCS_RX_ERR_CNT);
    g_dfx_info.dfx_linkdown_info.mac_init_status = readl(p_dev->io_base + AGENTDRV_PCIE_MAC + AGENTDRV_MAC_INT_STATUS);
    g_dfx_info.dfx_linkdown_info.tl_tx_post_cnt =
        readl(p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_RX_POST_CNT);
    g_dfx_info.dfx_linkdown_info.tl_tx_nonpost_cnt =
        readl(p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_RX_NONPOST_CNT);
    g_dfx_info.dfx_linkdown_info.tl_tx_cpl_cnt = readl(p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_RX_CPL_CNT);
    g_dfx_info.dfx_linkdown_info.tl_tx_errcpl_cnt =
        readl(p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_RX_ERRCPL_CNT);
    g_dfx_info.dfx_linkdown_info.tl_tx_err_cnt = readl(p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_RX_ERR_CNT);
    g_dfx_info.dfx_linkdown_info.buffer_status =
        readl(p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_RX_RX_BUFFER_STATUS);
    g_dfx_info.dfx_linkdown_info.tl_tx_dl_bp_cnt =
        readl(p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_TX_DL_BP_CNT);
}

STATIC void agentdrv_dfx_linkdown_put_into_g_str(void)
{
    int snprintf_s_ret;
    int str_size_left = AGENTDRV_BLACK_BOX_BUF_LEN;

    char *pcie_bbox_str_tmp = (char *)kzalloc(AGENTDRV_BLACK_BOX_BUF_LEN, GFP_ATOMIC);
    if (pcie_bbox_str_tmp == NULL) {
        devdrv_err("Call kzalloc failed, pcie_bbox_str_tmp is null.\n");
        return;
    }
    /* put the linkdown happened time into global string */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
    snprintf_s_ret =
        snprintf_s(pcie_bbox_str_tmp, str_size_left, str_size_left - 1,
                   "pme_turn_off_times:%llu, current time is:%llu.\n", g_dfx_info.dfx_linkdown_info.pme_turn_off_times,
                   ((u64)(g_dfx_info.current_time.tv_sec)) * AGENTDRV_TV_SEC +
                   (u64)g_dfx_info.current_time.tv_nsec / AGENTDRV_TV_NANOSEC);
    if (snprintf_s_ret == -1) {
        devdrv_err("Data snprintf_s failed.\n");
        kfree(pcie_bbox_str_tmp);
        pcie_bbox_str_tmp = NULL;
        return;
    }
#else
    snprintf_s_ret =
        snprintf_s(pcie_bbox_str_tmp, str_size_left, str_size_left - 1,
                   "pme_turn_off_times:%llu, current time is:%llu.\n", g_dfx_info.dfx_linkdown_info.pme_turn_off_times,
                   ((u64)(g_dfx_info.current_time.tv_sec)) * AGENTDRV_TV_SEC + g_dfx_info.current_time.tv_usec);
    if (snprintf_s_ret == -1) {
        devdrv_err("Data snprintf_s failed.\n");
        kfree(pcie_bbox_str_tmp);
        pcie_bbox_str_tmp = NULL;
        return;
    }
#endif

    str_size_left -= snprintf_s_ret;
    /* put the register info into global string */
    snprintf_s_ret =
        snprintf_s(pcie_bbox_str_tmp + AGENTDRV_BLACK_BOX_BUF_LEN - str_size_left, str_size_left, str_size_left - 1,
                   "bme_err:0x%x, cfg_bme_en:0x%x, tl_tx_int:0x%x, tl_fc_left:0x%x, "
                   "tl_tx_fifo_status:0x%x, mac link:0x%08x, mac_bp_timer:0x%x, rxidle<54:0x%x; "
                   "8c:0x%x; 250:0x%x; 254:0x%x; 258:0x%x; 2bc:0x%x>, tl_rx_total:0x%x, post:0x%x, "
                   "nonpost:0x%x, cpl:0x%x, errcpl:0x%x, err:0x%x, buf_sts:0x%x, dl_bp_cnt:0x%x, msg_cnt:0x%x\n",
                   g_dfx_info.dfx_linkdown_info.bme_err, g_dfx_info.dfx_linkdown_info.cfg_bme_en,
                   g_dfx_info.dfx_linkdown_info.tl_tx_int, g_dfx_info.dfx_linkdown_info.tl_fc_left,
                   g_dfx_info.dfx_linkdown_info.tl_tx_fifo_sts, g_dfx_info.dfx_linkdown_info.mac_link_info,
                   g_dfx_info.dfx_linkdown_info.mac_bp_timer, g_dfx_info.dfx_linkdown_info.mac_init_status,
                   g_dfx_info.dfx_linkdown_info.mac_symbol_unlocl_counter, g_dfx_info.dfx_linkdown_info.mac_debug_pipe9,
                   g_dfx_info.dfx_linkdown_info.mac_debug_pipe10, g_dfx_info.dfx_linkdown_info.mac_debug_pipe11,
                   g_dfx_info.dfx_linkdown_info.mac_pcs_err_cnt, g_dfx_info.dfx_linkdown_info.tl_tx_total_cnt,
                   g_dfx_info.dfx_linkdown_info.tl_tx_post_cnt, g_dfx_info.dfx_linkdown_info.tl_tx_nonpost_cnt,
                   g_dfx_info.dfx_linkdown_info.tl_tx_cpl_cnt, g_dfx_info.dfx_linkdown_info.tl_tx_errcpl_cnt,
                   g_dfx_info.dfx_linkdown_info.tl_tx_err_cnt, g_dfx_info.dfx_linkdown_info.buffer_status,
                   g_dfx_info.dfx_linkdown_info.tl_tx_dl_bp_cnt, g_dfx_info.dfx_linkdown_info.tl_tx_msg_cnt);
    if (snprintf_s_ret == -1) {
        devdrv_err("Data snprintf_s failed.\n");
    } else {
        agentdrv_dfx_put_string(pcie_bbox_str_tmp);
    }

    kfree(pcie_bbox_str_tmp);
    pcie_bbox_str_tmp = NULL;
}

/* collect the message reported to bbox, when need such as linkdown */
void agentdrv_dfx_linkdown_put_into_bbox(struct agentdrv_platform_dev *p_dev)
{
    agentdrv_dfx_linkdown_get_info(p_dev);
    agentdrv_dfx_linkdown_put_into_g_str();
}

STATIC void agentdev_pcie_reg_rd(const struct agentdrv_platform_dev *p_dev, u32 reg, u32 *val)
{
    *val = readl(p_dev->io_base + reg);
}

STATIC void agentdev_pcie_reg_wr(struct agentdrv_platform_dev *p_dev, u32 reg, u32 val)
{
    writel(val, p_dev->io_base + reg);
}

STATIC void agentdev_pcie_get_sel_trace_rx_data_mode(struct agentdrv_platform_dev *p_dev, u32 *val)
{
    PCIE_MAC_REG_LTSSM_TRACER_CFG0 reg_val = {0};

    agentdev_pcie_reg_rd(p_dev, PCIE_MAC_REG_LTSSM_TRACER_CFG0_REG, &reg_val.u32);
    *val = reg_val.bits.sel_trace_rx_data_mode;
}

void agentdev_pcie_set_reg_ltssm_tracer_recap(struct agentdrv_platform_dev *p_dev, u32 val)
{
    PCIE_MAC_REG_LTSSM_TRACER_INPUT reg_val = {0};

    agentdev_pcie_reg_rd(p_dev, PCIE_MAC_REG_LTSSM_TRACER_INPUT_REG, &reg_val.u32);
    reg_val.bits.reg_ltssm_tracer_recap = val;
    reg_val.bits.reg_ltssm_tracer_cap_mode = val >> 1;
    agentdev_pcie_reg_wr(p_dev, PCIE_MAC_REG_LTSSM_TRACER_INPUT_REG, reg_val.u32);
}

STATIC void agentdev_pcie_get_mac_reg_ltssm_tracer_output_1(struct agentdrv_platform_dev *p_dev, u32 *val)
{
    PCIE_MAC_REG_LTSSM_TRACER_OUTPUT_1 reg_val = {0};

    agentdev_pcie_reg_rd(p_dev, PCIE_MAC_REG_LTSSM_TRACER_OUTPUT_1_REG, &reg_val.u32);
    *val = reg_val.u32;
}

STATIC void agentdev_pcie_set_reg_ltssm_tracer_raddr(struct agentdrv_platform_dev *p_dev, u32 val)
{
    PCIE_MAC_REG_LTSSM_TRACERADDR reg_val = {0};

    agentdev_pcie_reg_rd(p_dev, PCIE_MAC_REG_LTSSM_TRACER_ADDR_REG, &reg_val.u32);
    reg_val.bits.reg_ltssm_tracer_raddr = val;
    agentdev_pcie_reg_wr(p_dev, PCIE_MAC_REG_LTSSM_TRACER_ADDR_REG, reg_val.u32);
}

STATIC void agentdev_pcie_get_ltssm_tracer_last_waddr(struct agentdrv_platform_dev *p_dev, u32 *val)
{
    PCIE_MAC_REG_LTSSM_TRACER_LAST_ADDR reg_val = {0};

    agentdev_pcie_reg_rd(p_dev, PCIE_MAC_REG_LTSSM_TRACER_LAST_ADDR_REG, &reg_val.u32);
    *val = reg_val.bits.ltssm_tracer_last_waddr;
}

STATIC void agentdev_pcie_get_ltssm_tracer_addr_rollback(struct agentdrv_platform_dev *p_dev, u32 *val)
{
    PCIE_MAC_REG_LTSSM_TRACER_LAST_ADDR reg_val = {0};

    agentdev_pcie_reg_rd(p_dev, PCIE_MAC_REG_LTSSM_TRACER_LAST_ADDR_REG, &reg_val.u32);
    *val = reg_val.bits.ltssm_tracer_addr_rollback;
}

STATIC void agentdev_pcie_get_mac_reg_ltssm_tracer_output_2(struct agentdrv_platform_dev *p_dev, u32 *val)
{
    PCIE_MAC_REG_LTSSM_TRACER_OUTPUT_2 reg_val = {0};

    agentdev_pcie_reg_rd(p_dev, PCIE_MAC_REG_LTSSM_TRACER_OUTPUT_2_REG, &reg_val.u32);
    *val = reg_val.u32;
}

STATIC void agentdev_pcie_get_ltssm_tracer_data_ok(struct agentdrv_platform_dev *p_dev, u32 *val)
{
    PCIE_MAC_REG_LTSSM_TRACER_OUTPUT_OK reg_val = {0};

    agentdev_pcie_reg_rd(p_dev, PCIE_MAC_REG_LTSSM_TRACER_OUTPUT_OK_REG, &reg_val.u32);
    *val = reg_val.bits.ltssm_tracer_data_ok;
}

STATIC char *agentdrv_get_ltssm_value(int tag)
{
    u32 i = 0;

    for (i = 0; i < sizeof(g_ltssm_state_enum) / sizeof(struct AGENTDRV_LTSSM_STATE_T); i++) {
        if (tag == g_ltssm_state_enum[i].tag)
            return g_ltssm_state_enum[i].value;
    }

    /* this branch will never step into */
    return NULL;
}

STATIC int agentdrv_pcie_format_ltssm_trace(char *pcie_bbox_str, int pcie_bbox_len,
    const u64 *ltssm_input, u32 ltssm_num)
{
    int snprintf_s_ret;
    int pcie_bbox_str_offset_tmp = 0;
    char *ltssm_value = NULL;
    u32 ltssm_tag = 0;
    int str_size_left;
    u32 i = 0;

    snprintf_s_ret = snprintf_s(pcie_bbox_str, pcie_bbox_len, pcie_bbox_len - 1,
                                LTSSM_TRACER_HEAD);
    if (snprintf_s_ret == -1) {
        devdrv_err("Data snprintf_s  failed.(snprintf_s_ret=%d)\n", snprintf_s_ret);
        return snprintf_s_ret;
    }
    pcie_bbox_str_offset_tmp += snprintf_s_ret;
    for (i = 1; i < ltssm_num; i++) {
        ltssm_tag = (unsigned int)(*(ltssm_input + i)) & 0x3f;
        ltssm_value = agentdrv_get_ltssm_value(ltssm_tag);
        str_size_left = pcie_bbox_len - pcie_bbox_str_offset_tmp;
        snprintf_s_ret =
            snprintf_s(pcie_bbox_str + pcie_bbox_str_offset_tmp, str_size_left, str_size_left - 1,
                       LTSSM_TRACE_STR_FORMAT, i,
                       (unsigned int)(*(ltssm_input + i) >> AGENTDRV_LTSSM_INPUT_48) & AGENTDRV_LTSSM_INPUT_BIT_48,
                       (unsigned int)(*(ltssm_input + i) >> AGENTDRV_LTSSM_INPUT_32) & AGENTDRV_LTSSM_INPUT_BIT_32,
                       (unsigned int)(*(ltssm_input + i) >> AGENTDRV_LTSSM_INPUT_31) & AGENTDRV_LTSSM_INPUT_BIT_31,
                       (unsigned int)(*(ltssm_input + i) >> AGENTDRV_LTSSM_INPUT_30) & 0x1,
                       (unsigned int)(*(ltssm_input + i) >> AGENTDRV_LTSSM_INPUT_29) & 0x1,
                       (unsigned int)(*(ltssm_input + i) >> AGENTDRV_LTSSM_INPUT_28) & 0x1,
                       (unsigned int)(*(ltssm_input + i) >> AGENTDRV_LTSSM_INPUT_27) & 0x1,
                       (unsigned int)(*(ltssm_input + i) >> AGENTDRV_LTSSM_INPUT_26) & 0x1,
                       (unsigned int)(*(ltssm_input + i) >> AGENTDRV_LTSSM_INPUT_25) & 0x1,
                       (unsigned int)(*(ltssm_input + i) >> AGENTDRV_LTSSM_INPUT_24) & 0x1,
                       (unsigned int)(*(ltssm_input + i) >> AGENTDRV_LTSSM_INPUT_23) & 0x1,
                       (unsigned int)(*(ltssm_input + i) >> AGENTDRV_LTSSM_INPUT_22) & 0x1,
                       (unsigned int)(*(ltssm_input + i) >> AGENTDRV_LTSSM_INPUT_20) & AGENTDRV_LTSSM_INPUT_BIT_20,
                       (unsigned int)(*(ltssm_input + i) >> AGENTDRV_LTSSM_INPUT_12) & AGENTDRV_LTSSM_INPUT_BIT_12,
                       (unsigned int)(*(ltssm_input + i) >> AGENTDRV_LTSSM_INPUT_10) & AGENTDRV_LTSSM_INPUT_BIT_10,
                       (unsigned int)(*(ltssm_input + i) >> AGENTDRV_LTSSM_INPUT_6) & AGENTDRV_LTSSM_INPUT_BIT_6,
                       ltssm_tag, (ltssm_value == NULL) ? "null" : ltssm_value);
        /* if snprintf wrong, then break and record msg before */
        if (snprintf_s_ret == -1) {
            devdrv_err("Data snprint_s failed.\n");
            break;
        }
        pcie_bbox_str_offset_tmp += snprintf_s_ret;
    }
    return 0;
}

STATIC void agentdrv_pcie_set_ltssm_trace(u64 *ltssm_input, u32 ltssm_input_len, u32 ltssm_num)
{
    char *pcie_bbox_str_tmp = NULL;
    int ret;

    if (ltssm_num > ltssm_input_len) {
        devdrv_info("ltssm_num exceed MAX length.\n");
        return;
    }

    pcie_bbox_str_tmp = (char *)kzalloc(AGENTDRV_BLACK_BOX_BUF_LEN, GFP_ATOMIC);
    if (pcie_bbox_str_tmp == NULL) {
        devdrv_err(" Call kzalloc failed, pcie_bbox_str_tmp is null.\n");
        return;
    }

    ret = agentdrv_pcie_format_ltssm_trace(pcie_bbox_str_tmp, AGENTDRV_BLACK_BOX_BUF_LEN, ltssm_input, ltssm_num);
    if (ret == 0) {
        agentdrv_dfx_put_string(pcie_bbox_str_tmp);
    }

    kfree(pcie_bbox_str_tmp);
    pcie_bbox_str_tmp = NULL;
}

void agentdrv_wait_trace_data_ok(struct agentdrv_platform_dev *p_dev, u32 addr_index, u64 *ltssm_status,
                                 u32 ltssm_status_index)
{
    u32 timeout = AGENTDRV_GET_TRACE_DATA_TIMEOUT;
    u32 temp_val = 0;

    agentdev_pcie_set_reg_ltssm_tracer_raddr(p_dev, addr_index);
    do {
        agentdev_pcie_get_ltssm_tracer_data_ok(p_dev, &temp_val);
        if (!temp_val) {
            mdelay(2);
            timeout--;
        }
    } while (!temp_val && timeout);

    agentdev_pcie_get_mac_reg_ltssm_tracer_output_2(p_dev, &temp_val);
    *(ltssm_status + ltssm_status_index) = temp_val;

    agentdev_pcie_get_mac_reg_ltssm_tracer_output_1(p_dev, &temp_val);
    *(ltssm_status + ltssm_status_index) = (*(ltssm_status + ltssm_status_index) << 32) | temp_val;
}

STATIC void agentdev_pcie_get_ltssm_trace(struct agentdrv_platform_dev *p_dev, u64 *ltssm_status, u32 ltssm_status_len,
                                          u32 *ltssm_num)
{
    u32 trace_addr = 0x0;
    u32 addr_index = 0x0;
    u32 temp_val = 0;
    u32 i = 0;

    agentdev_pcie_get_sel_trace_rx_data_mode(p_dev, &temp_val);
    *ltssm_status = temp_val & 0x8;
    i++;

    agentdev_pcie_get_ltssm_tracer_last_waddr(p_dev, &trace_addr);
    agentdev_pcie_get_ltssm_tracer_addr_rollback(p_dev, &temp_val);

    if (temp_val == 1) {
        addr_index = trace_addr;
        do {
            if (i >= ltssm_status_len) {
                devdrv_info("Exceed MAX length.\n");
                break;
            }

            agentdrv_wait_trace_data_ok(p_dev, addr_index, ltssm_status, i);

            i++;

            if (addr_index == PCIE_LTSSM_TRACER_DEPTH - 1)
                addr_index = 0;
            else
                addr_index++;
        } while (addr_index != trace_addr);
    } else {
        while (addr_index < trace_addr) {
            if (i >= ltssm_status_len) {
                devdrv_info("Exceed MAX length.\n");
                break;
            }

            agentdrv_wait_trace_data_ok(p_dev, addr_index, ltssm_status, i);

            i++;
            addr_index++;
        }
    }
    *ltssm_num = i;

    /* DFX always record even if memory is full and overwite the old data. */
    agentdev_pcie_set_reg_ltssm_tracer_recap(p_dev, 0x3);
}

void agentdrv_dfx_linkdown_collect_link_state(struct agentdrv_platform_dev *p_dev)
{
    u64 ltssm_st_save[PCIE_LTSSM_TRACER_DEPTH + 1] = {0};
    u32 ltssm_num = 0;

    agentdev_pcie_get_ltssm_trace(p_dev, ltssm_st_save, PCIE_LTSSM_TRACER_DEPTH + 1, &ltssm_num);
    agentdrv_pcie_set_ltssm_trace(ltssm_st_save, PCIE_LTSSM_TRACER_DEPTH + 1, ltssm_num);
}

STATIC u32 agentdrv_dfx_dma_reg_rd(const void __iomem *io_base, u32 offset)
{
    return readl(io_base + offset);
}

STATIC void agentdrv_dfx_dma_get_info(struct devdrv_dma_channel *dma_chan, u32 queue_init_sts)
{
    void __iomem *p_dev_io_base = NULL;
    struct agentdrv_platform_dev *platform_dev = NULL;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
    ktime_get_real_ts64(&(g_dfx_info.current_time));
#else
    do_gettimeofday(&(g_dfx_info.current_time));
#endif

    g_dfx_info.dfx_dma_info.queue_desp0_50 = agentdrv_dfx_dma_reg_rd(dma_chan->io_base, DEVDRV_DMA_QUEUE_DESP0);
    g_dfx_info.dfx_dma_info.queue_desp1_54 = agentdrv_dfx_dma_reg_rd(dma_chan->io_base, DEVDRV_DMA_QUEUE_DESP1);
    g_dfx_info.dfx_dma_info.queue_desp2_58 = agentdrv_dfx_dma_reg_rd(dma_chan->io_base, DEVDRV_DMA_QUEUE_DESP2);
    g_dfx_info.dfx_dma_info.queue_desp3_5c = agentdrv_dfx_dma_reg_rd(dma_chan->io_base, DEVDRV_DMA_QUEUE_DESP3);
    g_dfx_info.dfx_dma_info.queue_desp4_74 = agentdrv_dfx_dma_reg_rd(dma_chan->io_base, DEVDRV_DMA_QUEUE_DESP4);
    g_dfx_info.dfx_dma_info.queue_desp5_78 = agentdrv_dfx_dma_reg_rd(dma_chan->io_base, DEVDRV_DMA_QUEUE_DESP5);
    g_dfx_info.dfx_dma_info.queue_desp6_7c = agentdrv_dfx_dma_reg_rd(dma_chan->io_base, DEVDRV_DMA_QUEUE_DESP6);
    g_dfx_info.dfx_dma_info.queue_desp7_80 = agentdrv_dfx_dma_reg_rd(dma_chan->io_base, DEVDRV_DMA_QUEUE_DESP7);
    g_dfx_info.dfx_dma_info.queue_err_src_code_h = agentdrv_dfx_dma_reg_rd(dma_chan->io_base,
                                                                           DEVDRV_DMA_QUEUE_ERR_ADDR_H);
    g_dfx_info.dfx_dma_info.queue_err_src_code_l = agentdrv_dfx_dma_reg_rd(dma_chan->io_base,
                                                                           DEVDRV_DMA_QUEUE_ERR_ADDR_L);
    g_dfx_info.dfx_dma_info.queue_sq_read_err = agentdrv_dfx_dma_reg_rd(dma_chan->io_base,
                                                                        DEVDRV_DMA_QUEUE_SQ_READ_ERR_PTR);
    g_dfx_info.dfx_dma_info.hard_sq_head = agentdrv_dfx_dma_reg_rd(dma_chan->io_base, DEVDRV_DMA_QUEUE_SQ_STS) & 0xFFFF;
    g_dfx_info.dfx_dma_info.hard_sq_tail = agentdrv_dfx_dma_reg_rd(dma_chan->io_base, DEVDRV_DMA_QUEUE_SQ_TAIL);
    g_dfx_info.dfx_dma_info.hard_cq_head = agentdrv_dfx_dma_reg_rd(dma_chan->io_base, DEVDRV_DMA_QUEUE_CQ_HEAD);
    g_dfx_info.dfx_dma_info.hard_cq_tail = agentdrv_dfx_dma_reg_rd(dma_chan->io_base, DEVDRV_DMA_QUEUE_CQ_TAIL);
    g_dfx_info.dfx_dma_info.queue_init_sts = queue_init_sts;
    g_dfx_info.dfx_dma_info.sq_vir_base_src_code = (u64)((uintptr_t)dma_chan->sq_desc_base);
    g_dfx_info.dfx_dma_info.cq_vir_base_src_code = (u64)((uintptr_t)dma_chan->cq_desc_base);
    g_dfx_info.dfx_dma_info.soft_sq_tail = dma_chan->sq_tail;
    g_dfx_info.dfx_dma_info.soft_sq_head = dma_chan->sq_head;
    g_dfx_info.dfx_dma_info.soft_cq_head = dma_chan->cq_head;

    platform_dev = agentdrv_get_platform_dev_by_dma_chan(dma_chan);
    if (platform_dev == NULL) {
        devdrv_err("Got platform device failed.\n");
        return;
    }
    p_dev_io_base = platform_dev->io_base;

    g_dfx_info.dfx_dma_info.uncr_err_status =
        agentdrv_dfx_dma_reg_rd(p_dev_io_base, AGENTDRV_EPF_CFG_REG + AGENTDRV_EPF_CFG_UNCR_ERR_STATUS);
    g_dfx_info.dfx_dma_info.cor_err_status =
        agentdrv_dfx_dma_reg_rd(p_dev_io_base, AGENTDRV_EPF_CFG_REG + AGENTDRV_EPF_CFG_COR_ERR_STATUS);
    g_dfx_info.dfx_dma_info.tl_int_status0 = agentdrv_dfx_dma_reg_rd(p_dev_io_base,
                                                                     AGENTDRV_TL_REG + AGENTDRV_TL_INT_STATUS0_DFX);
    g_dfx_info.dfx_dma_info.tl_rx_err_status = agentdrv_dfx_dma_reg_rd(p_dev_io_base,
                                                                       AGENTDRV_TL_REG + AGENTDRV_TL_RX_ERR_STATUS_DMA);

    g_dfx_info.dfx_dma_info.tx_port_err_cnt =
        agentdrv_dfx_dma_reg_rd(p_dev_io_base, AGENTDRV_IOB_TX_REG + AGENTDRV_OB_TX_PORT_ERR_CNT);
    g_dfx_info.dfx_dma_info.tx_port_np_cnt = agentdrv_dfx_dma_reg_rd(p_dev_io_base,
                                                                     AGENTDRV_IOB_TX_REG + AGENTDRV_OB_TX_PORT_NP_CNT);
}

STATIC void agentdrv_dfx_dma_put_into_g_str(void)
{
    int snprintf_s_ret;
    u32 str_size_left = AGENTDRV_BLACK_BOX_BUF_LEN;

    char *pcie_bbox_str_tmp = (char *)kzalloc(AGENTDRV_BLACK_BOX_BUF_LEN, GFP_ATOMIC);
    if (pcie_bbox_str_tmp == NULL) {
        devdrv_err("Call kzalloc failed, pcie_bbox_str_tmp is null.\n");
        return;
    }

    /* put the dma interrupt happened time into global string */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
    snprintf_s_ret =
        snprintf_s(pcie_bbox_str_tmp, str_size_left, str_size_left - 1, "DMA exception: current time is %llu.\n",
                   ((u64)(g_dfx_info.current_time.tv_sec)) * AGENTDRV_TV_SEC +
                   (u64)g_dfx_info.current_time.tv_nsec / AGENTDRV_TV_NANOSEC);
    if (snprintf_s_ret == -1) {
        devdrv_err("Data snprintf_s failed.\n");
        kfree(pcie_bbox_str_tmp);
        pcie_bbox_str_tmp = NULL;
        return;
    }
#else
    snprintf_s_ret =
        snprintf_s(pcie_bbox_str_tmp, str_size_left, str_size_left - 1, "DMA exception: current time is %llu.\n",
                   ((u64)(g_dfx_info.current_time.tv_sec)) * AGENTDRV_TV_SEC + g_dfx_info.current_time.tv_usec);
    if (snprintf_s_ret == -1) {
        devdrv_err("Data snprintf_s failed.\n");
        kfree(pcie_bbox_str_tmp);
        pcie_bbox_str_tmp = NULL;
        return;
    }
#endif
    str_size_left -= snprintf_s_ret;

    /* put the reg info into global string */
    snprintf_s_ret = snprintf_s(
        pcie_bbox_str_tmp + AGENTDRV_BLACK_BOX_BUF_LEN - str_size_left, str_size_left, str_size_left - 1,
        "tx_port_err_cnt:0x%x, tx_port_np_cnt:0x%x,\nuncr_err_status:0x%x, cor_err_status:0x%x,\n"
        "tl_int_status0:0x%x, tl_rx_err_status:0x%x,\n"
        "queue_desp<50:0x%x; 54:0x%x; 58:0x%x; 5c:0x%x; 74:0x%x; 78:0x%x; 7c:0x%x; 80:0x%x>,\n"
        "queue_sq_read_err:0x%x, queue_sq_sts:0x%x,\nsq_vir_base_src_code:0x%pK, cq_vir_base_src_code:0x%pK, "
        "soft_sq_tail:0x%x, soft_sq_head:0x%x, soft_cq_head:0x%x,\n"
        "hard_sq_head:0x%x, hard_sq_tail:0x%x, hard_cq_head:0x%x, hard_cq_tail:0x%x,\n"
        "queue_init_sts:0x%x,\nqueue_err_src_code_h:0x%x, queue_err_src_code_l:0x%x\n",
        g_dfx_info.dfx_dma_info.tx_port_err_cnt, g_dfx_info.dfx_dma_info.tx_port_np_cnt,
        g_dfx_info.dfx_dma_info.uncr_err_status, g_dfx_info.dfx_dma_info.cor_err_status,
        g_dfx_info.dfx_dma_info.tl_int_status0, g_dfx_info.dfx_dma_info.tl_rx_err_status,
        g_dfx_info.dfx_dma_info.queue_desp0_50, g_dfx_info.dfx_dma_info.queue_desp1_54,
        g_dfx_info.dfx_dma_info.queue_desp2_58, g_dfx_info.dfx_dma_info.queue_desp3_5c,
        g_dfx_info.dfx_dma_info.queue_desp4_74, g_dfx_info.dfx_dma_info.queue_desp5_78,
        g_dfx_info.dfx_dma_info.queue_desp6_7c, g_dfx_info.dfx_dma_info.queue_desp7_80,
        g_dfx_info.dfx_dma_info.queue_sq_read_err, g_dfx_info.dfx_dma_info.queue_sq_sts,
        g_dfx_info.dfx_dma_info.sq_vir_base_src_code, g_dfx_info.dfx_dma_info.cq_vir_base_src_code,
        g_dfx_info.dfx_dma_info.soft_sq_tail, g_dfx_info.dfx_dma_info.soft_sq_head,
        g_dfx_info.dfx_dma_info.soft_cq_head, g_dfx_info.dfx_dma_info.hard_sq_head,
        g_dfx_info.dfx_dma_info.hard_sq_tail, g_dfx_info.dfx_dma_info.hard_cq_head,
        g_dfx_info.dfx_dma_info.hard_cq_tail, g_dfx_info.dfx_dma_info.queue_init_sts,
        g_dfx_info.dfx_dma_info.queue_err_src_code_h, g_dfx_info.dfx_dma_info.queue_err_src_code_l);
    if (snprintf_s_ret == -1) {
        devdrv_err("Data snprintf_s failed.\n");
    } else {
        agentdrv_dfx_put_string(pcie_bbox_str_tmp);
    }

    kfree(pcie_bbox_str_tmp);
    pcie_bbox_str_tmp = NULL;
}

void devdrv_dfx_dma_report_to_bbox(struct devdrv_dma_channel *dma_chan, u32 queue_init_sts)
{
    if (devdrv_get_chip_type() == HISI_MINI_V1) {
        agentdrv_dfx_dma_get_info(dma_chan, queue_init_sts);
        agentdrv_dfx_dma_put_into_g_str();
        agentdrv_notify_blackbox_exception(0, AGENTDRV_DMA_FAIL, g_dfx_info.bbox_storage_str);
    }
}
