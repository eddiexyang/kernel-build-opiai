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

#include "sdk_hilink_common.h"
#include "sdk_hilink_init.h"
#include "sdk_hilink_snapshot.h"

#include <linux/types.h>

const struct hilink_macro_cfg evb_board_900[MACRO_MAX] = {
    {
        .lane_order = 0,
        .cs = {
            {.ssc_enable = 1, .type = SERDES_TYPE_SATA, .frequency = SERDES_FREQ_SATA_6G},
            {.ssc_enable = 0, .type = SERDES_TYPE_NULL, .frequency = SERDES_FREQ_NULL},
        },
        .ds = {
            {.cs_select = CS0, .ssc_enable = 1, .polarity_tx = 0, .polarity_rx = 0, .type = SERDES_TYPE_SATA},
            {.cs_select = CS0, .ssc_enable = 1, .polarity_tx = 0, .polarity_rx = 0, .type = SERDES_TYPE_SATA},
            {.cs_select = CS0, .ssc_enable = 1, .polarity_tx = 0, .polarity_rx = 0, .type = SERDES_TYPE_SATA},
            {.cs_select = CS0, .ssc_enable = 1, .polarity_tx = 0, .polarity_rx = 0, .type = SERDES_TYPE_SATA},
        },
    },
    {
        .lane_order = 0,
        .cs = {
            {.ssc_enable = 0, .type = SERDES_TYPE_NULL, .frequency = SERDES_FREQ_NULL},
            {.ssc_enable = 0, .type = SERDES_TYPE_USB, .frequency = SERDES_FREQ_USB_5G},
        },
        .ds = {
            {.cs_select = CS1, .ssc_enable = 0, .polarity_tx = 0, .polarity_rx = 0, .type = SERDES_TYPE_USB},
            {.cs_select = CS1, .ssc_enable = 0, .polarity_tx = 0, .polarity_rx = 0, .type = SERDES_TYPE_USB},
            {.cs_select = CS1, .ssc_enable = 0, .polarity_tx = 0, .polarity_rx = 0, .type = SERDES_TYPE_USB},
            {.cs_select = CS1, .ssc_enable = 0, .polarity_tx = 0, .polarity_rx = 0, .type = SERDES_TYPE_USB},
        },
    }
};

const struct hilink_macro_cfg evb_board_901[MACRO_MAX] = {
    {
        .lane_order = 0,
        .cs = {
            {.ssc_enable = 0, .type = SERDES_TYPE_PCIE, .frequency = SERDES_FREQ_PCIE_GEN_3},
            {.ssc_enable = 0, .type = SERDES_TYPE_PCIE, .frequency = SERDES_FREQ_PCIE_GEN_1_2},
        },
        .ds = {
            {.cs_select = CS1, .ssc_enable = 0, .polarity_tx = 0, .polarity_rx = 0, .type = SERDES_TYPE_PCIE},
            {.cs_select = CS1, .ssc_enable = 0, .polarity_tx = 0, .polarity_rx = 0, .type = SERDES_TYPE_PCIE},
            {.cs_select = CS1, .ssc_enable = 0, .polarity_tx = 0, .polarity_rx = 0, .type = SERDES_TYPE_PCIE},
            {.cs_select = CS1, .ssc_enable = 0, .polarity_tx = 0, .polarity_rx = 0, .type = SERDES_TYPE_PCIE},
        },
    },
    {
        .lane_order = 1,
        .cs = {
            {.ssc_enable = 0, .type = SERDES_TYPE_ETH, .frequency = SERDES_FREQ_ETH_1_25G},
            {.ssc_enable = 0, .type = SERDES_TYPE_PCIE, .frequency = SERDES_FREQ_PCIE_GEN_1_2},
        },
        .ds = {
            {.cs_select = CS1, .ssc_enable = 0, .polarity_tx = 0, .polarity_rx = 0, .type = SERDES_TYPE_PCIE},
            {.cs_select = CS1, .ssc_enable = 0, .polarity_tx = 0, .polarity_rx = 0, .type = SERDES_TYPE_PCIE},
            {.cs_select = CS1, .ssc_enable = 0, .polarity_tx = 0, .polarity_rx = 0, .type = SERDES_TYPE_PCIE},
            {.cs_select = CS0, .ssc_enable = 0, .polarity_tx = 0, .polarity_rx = 0, .type = SERDES_TYPE_ETH},
        },
    }
};

STATIC int hilink_cfg_get_node_pos_info(void __iomem *base, u32 *cfg_area_offset, u32 *serdes_offset)
{
    struct drv_cfg_shmg_head *shmg_head;
    void __iomem *current_pos = base;
    struct drv_cfg_uni_node_head *uni_node_head;
    struct drv_cfg_serdes_node_info *uni_node;

    // check SHMG head
    shmg_head = (struct drv_cfg_shmg_head *)current_pos;
    if (shmg_head->magic != DRV_SRAM_ELASTIC_CFG_BIOS_MAGIC) {
        HILINK_ERR_SNAPSHOT(SERDES_CFG_SHMG_HEAD_CHECK_FAIL, "The SHMG header information is incorrect!\n");
        return -EINVAL;
    }
    current_pos += shmg_head->head_size;

    uni_node_head = (struct drv_cfg_uni_node_head *)current_pos;
    while (uni_node_head->node_id != SERDES_INFO_ID) {
        if (uni_node_head->node_id == DRV_SRAM_ELASTIC_CFG_END_NODE_ID) {
            HILINK_ERR_SNAPSHOT(SERDES_CFG_FIND_NODE_FAIL, "Not find valid node in SRAM.\n");
            return -EINVAL;
        }
        current_pos += uni_node_head->node_size;
        if ((current_pos - base) >= (shmg_head->share_mem_size)) {
            HILINK_ERR_SNAPSHOT(SERDES_CFG_NODE_ADDR_EXCEED_FAIL, "Current offset exceeds space size!\n");
            return -EINVAL;
        }
        uni_node_head = (struct drv_cfg_uni_node_head *)current_pos;
    }

    if ((uni_node_head->magic != DRV_SRAM_ELASTIC_CFG_BIOS_MAGIC) || (uni_node_head->node_id != SERDES_INFO_ID) ||
        (uni_node_head->node_size != sizeof(struct drv_cfg_serdes_node_info))) {
        HILINK_ERR_SNAPSHOT(SERDES_CFG_NODE_CHECK_FAIL,
            "Cfg uni node head check failed. (magic=%u; node_id=%u; node_size=%u)\n",
            uni_node_head->magic, uni_node_head->node_id, uni_node_head->node_size);
        return -EINVAL;
    }

    uni_node = (struct drv_cfg_serdes_node_info *)current_pos;
    if ((uni_node->raw_data.user_addr != 0) && (uni_node->raw_data.user_addr < DRV_DDR_ELASTIC_CFG_SIZE)) {
        *cfg_area_offset = DRV_DDR_ELASTIC_CFG_USER_OFFSET;
        *serdes_offset = uni_node->raw_data.user_addr;
        HILINK_INFO("Get user cfg.\n");
    } else if ((uni_node->raw_data.sys_addr != 0) && (uni_node->raw_data.sys_addr < DRV_DDR_ELASTIC_CFG_SIZE)) {
        *cfg_area_offset = DRV_DDR_ELASTIC_CFG_SYS_OFFSET;
        *serdes_offset = uni_node->raw_data.sys_addr;
        HILINK_INFO("Get sys cfg.\n");
    } else {
        HILINK_ERR_SNAPSHOT(SERDES_CFG_NODE_INVALID_FAIL, "Cfg uni node addr invalid.)\n");
        return -EINVAL;
    }
    return 0;
}

STATIC bool hilink_cfg_is_lane_valid(struct drv_cfg_serdes_lane_info *lane_info, u32 lane_id)
{
    if (lane_info->flag == 0) {
        HILINK_ERR_SNAPSHOT(SERDES_INFO_CHECK_FLAG_FAIL, "Serdes lane info flag invalid.\n");
        return false;
    }
    if (lane_info->lan_index != lane_id) {
        HILINK_ERR_SNAPSHOT(SERDES_INFO_CHECK_INDEX_FAIL,
            "Serdes lan_index invalid. (lan_index=%u; lane_id=%u)\n", lane_info->lan_index, lane_id);
        return false;
    }
    // every type has specific frequency plan
    if ((lane_info->type == SERDES_TYPE_PCIE) &&
        (lane_info->frequency != SERDES_FREQ_PCIE_GEN_1_2) && (lane_info->frequency != SERDES_FREQ_PCIE_GEN_3)) {
        HILINK_ERR_SNAPSHOT(SERDES_INFO_CHECK_PCIE_FREQ_FAIL,
            "Serdes PCIE freq invalid. (lane_id=%u; frequency=%u)\n", lane_id, lane_info->frequency);
        return false;
    }
    if ((lane_info->type == SERDES_TYPE_SATA) && (lane_info->frequency != SERDES_FREQ_SATA_6G)) {
        HILINK_ERR_SNAPSHOT(SERDES_INFO_CHECK_SATA_FREQ_FAIL,
            "Serdes SATA freq invalid. (lane_id=%u; frequency=%u)\n", lane_id, lane_info->frequency);
        return false;
    }
    if ((lane_info->type == SERDES_TYPE_USB) && (lane_info->frequency != SERDES_FREQ_USB_5G)) {
        HILINK_ERR_SNAPSHOT(SERDES_INFO_CHECK_USB_FREQ_FAIL,
            "Serdes USB freq invalid. (lane_id=%u; frequency=%u)\n", lane_id, lane_info->frequency);
        return false;
    }
    if ((lane_info->type == SERDES_TYPE_ETH) &&
        (lane_info->frequency != SERDES_FREQ_ETH_1_25G) && (lane_info->frequency != SERDES_FREQ_ETH_3_125G)) {
        HILINK_ERR_SNAPSHOT(SERDES_INFO_CHECK_ETH_FREQ_FAIL,
            "Serdes ETH freq invalid. (lane_id=%u; frequency=%u)\n", lane_id, lane_info->frequency);
        return false;
    }
    // only pcie need bandwidth parameter
    if ((lane_info->type != SERDES_TYPE_PCIE) && (lane_info->bandwidth != 0xFF)) {
        HILINK_ERR_SNAPSHOT(SERDES_INFO_CHECK_BANDWIDTH_FAIL,
            "Only pcie need bandwidth parameter. (type=%u; bandwidth=%u)\n", lane_info->type, lane_info->bandwidth);
        return false;
    }
    // only pcie & sata can enable ssc
    if ((lane_info->type != SERDES_TYPE_PCIE) &&
        (lane_info->type != SERDES_TYPE_SATA) &&
        (lane_info->ssc_enable != 0)) {
        HILINK_ERR_SNAPSHOT(SERDES_INFO_CHECK_SSC_FAIL,
            "Only pcie & sata can enable ssc. (lane_id=%u; type=%u; ssc_enable=%u)\n",
            lane_id, lane_info->type, lane_info->ssc_enable);
        return false;
    }
    return true;
}

bool hilink_cfg_is_exist_prot(struct hilink_macro_cfg *macro_cfg, DRV_SERDES_TYPE type)
{
    u32 ds;

    // if there exists particular protocol in this macro, then return TRUE
    for (ds = 0; ds < DS_MAX; ds++) {
        if (macro_cfg->ds[ds].type == type) {
            return true;
        }
    }
    return false;
}

STATIC int hilink_cfg_get_lane_prot(struct hilink_macro_cfg *macro_cfg,
    struct drv_cfg_serdes_lane_info *lane_info, u32 macro)
{
    u32 ds;

    for (ds = 0; ds < DS_MAX; ds++) {
        if (hilink_cfg_is_lane_valid(&lane_info[ds], macro * DS_MAX + ds) == false) {
            HILINK_ERR_SNAPSHOT(SERDES_INFO_CHECK_FAIL, "Serdes lane cfg invalid. (macro=%u; ds=%u)\n", macro, ds);
            return -EINVAL;
        }
        macro_cfg->ds[ds].type = lane_info[ds].type;
        if ((hilink_cfg_is_exist_prot(macro_cfg, SERDES_TYPE_PCIE) == true) ||
            (hilink_cfg_is_exist_prot(macro_cfg, SERDES_TYPE_ETH) == true)) {
                macro_cfg->lane_order = lane_info[ds].lan_order;
        }
        macro_cfg->ds[ds].port = lane_info[ds].port_index;
        macro_cfg->ds[ds].ssc_enable = lane_info[ds].ssc_enable;
        macro_cfg->ds[ds].polarity_tx = lane_info[ds].polarity_tx;
        macro_cfg->ds[ds].polarity_rx = lane_info[ds].polarity_rx;
    }
    // sata must only used in M0
    if ((macro == MACRO_1) && (hilink_cfg_is_exist_prot(macro_cfg, SERDES_TYPE_SATA) == true)) {
        HILINK_ERR_SNAPSHOT(SERDES_INFO_CHECK_M1_PROTOCOL_TYPE_FAIL,
            "SATA must only used in M0. (macro=%u; ds=%u)\n", macro, ds);
        return -EINVAL;
    }
    // USB/ETH must only used in M1
    if ((macro == MACRO_0) &&
        ((hilink_cfg_is_exist_prot(macro_cfg, SERDES_TYPE_USB) == true) ||
        (hilink_cfg_is_exist_prot(macro_cfg, SERDES_TYPE_ETH) == true))) {
        HILINK_ERR_SNAPSHOT(SERDES_INFO_CHECK_M0_PROTOCOL_TYPE_FAIL,
            "USB/ETH must only used in M1. (macro=%u; ds=%u)\n", macro, ds);
        return -EINVAL;
    }
    // PCIE/USB/ETH only support two at most in M1
    if ((macro == MACRO_1) &&
        ((hilink_cfg_is_exist_prot(macro_cfg, SERDES_TYPE_PCIE) == true) &&
        (hilink_cfg_is_exist_prot(macro_cfg, SERDES_TYPE_USB) == true) &&
        (hilink_cfg_is_exist_prot(macro_cfg, SERDES_TYPE_ETH) == true))) {
        HILINK_ERR_SNAPSHOT(SERDES_INFO_CHECK_M1_PROTOCOL_NUM_FAIL,
            "PCIE/USB/ETH only support two at most in M1. (macro=%u; ds=%u)\n", macro, ds);
        return -EINVAL;
    }

    return 0;
}

STATIC int hilink_cfg_get_ssc_enable(struct drv_cfg_serdes_lane_info *lane_info,
    u32 *pcie_ssc, u32 *sata_ssc)
{
    u32 ds;

    *pcie_ssc = 0;
    *sata_ssc = 0;
    for (ds = 0; ds < DS_MAX; ds++) {
        if ((lane_info[ds].type == SERDES_TYPE_PCIE) && (lane_info[ds].ssc_enable == 1)) {
            *pcie_ssc = 1;
        }
        if ((lane_info[ds].type == SERDES_TYPE_SATA) && (lane_info[ds].ssc_enable == 1)) {
            *sata_ssc = 1;
        }
    }
    // in one macro, ssc of pcie or sata lanes should be enabled at the same time
    for (ds = 0; ds < DS_MAX; ds++) {
        if ((lane_info[ds].type == SERDES_TYPE_PCIE) && (lane_info[ds].ssc_enable != *pcie_ssc)) {
            HILINK_ERR_SNAPSHOT(SERDES_INFO_CHECK_PCIE_SSC_FAIL,
                "PCIe ssc status of the macro is not the same. (ds=%u)\n", ds);
            return -EINVAL;
        }
        if ((lane_info[ds].type == SERDES_TYPE_SATA) && (lane_info[ds].ssc_enable != *sata_ssc)) {
            HILINK_ERR_SNAPSHOT(SERDES_INFO_CHECK_SATA_SSC_FAIL,
                "SATA ssc status of the macro is not the same. (ds=%u)\n", ds);
            return -EINVAL;
        }
    }
    return 0;
}

STATIC int hilink_cfg_get_eth_freq_plan(struct drv_cfg_serdes_lane_info *lane_info,
    DRV_SERDES_FREQUENCY_LEVEL *eth_freq_plan)
{
    DRV_SERDES_FREQUENCY_LEVEL freq;
    u32 ds;

    for (ds = 0; ds < DS_MAX; ds++) {
        if (lane_info[ds].type == SERDES_TYPE_ETH) {
            freq = lane_info[ds].frequency;
        }
    }
    // in one macro, freq of eth should be the same
    for (ds = 0; ds < DS_MAX; ds++) {
        if ((lane_info[ds].type == SERDES_TYPE_ETH) && (lane_info[ds].frequency != freq)) {
            HILINK_ERR_SNAPSHOT(SERDES_INFO_CHECK_ETH_SAME_FREQ_FAIL,
                "ETH freq in one macro is not the same (ds=%u).\n", ds);
            return -EINVAL;
        }
    }
    // transform freq of syscfg to h25 freq plan
    *eth_freq_plan = freq;
    return 0;
}

STATIC int  hilink_cfg_build_freq_plan(struct hilink_macro_cfg *macro_cfg,
    u32 pcie_ssc, u32 sata_ssc, DRV_SERDES_FREQUENCY_LEVEL eth_freq_plan)
{
    u32 ds;

    // build freq plan
    macro_cfg->cs[CS0].type = SERDES_TYPE_NULL;
    macro_cfg->cs[CS0].frequency = SERDES_FREQ_NULL;
    macro_cfg->cs[CS1].type = SERDES_TYPE_NULL;
    macro_cfg->cs[CS1].frequency = SERDES_FREQ_NULL;
    if (hilink_cfg_is_exist_prot(macro_cfg, SERDES_TYPE_PCIE) == true) {
        macro_cfg->cs[CS0].type = SERDES_TYPE_PCIE;
        macro_cfg->cs[CS0].frequency = SERDES_FREQ_PCIE_GEN_3;
        macro_cfg->cs[CS0].ssc_enable = pcie_ssc;
        macro_cfg->cs[CS1].type = SERDES_TYPE_PCIE;
        macro_cfg->cs[CS1].frequency = SERDES_FREQ_PCIE_GEN_1_2;
        macro_cfg->cs[CS1].ssc_enable = pcie_ssc;
    }
    if (hilink_cfg_is_exist_prot(macro_cfg, SERDES_TYPE_SATA) == true) {
        macro_cfg->cs[CS0].type = SERDES_TYPE_SATA;
        macro_cfg->cs[CS0].frequency = SERDES_FREQ_SATA_6G;
        macro_cfg->cs[CS0].ssc_enable = sata_ssc;
    }
    if (hilink_cfg_is_exist_prot(macro_cfg, SERDES_TYPE_USB) == true) {
        macro_cfg->cs[CS1].type = SERDES_TYPE_USB;
        macro_cfg->cs[CS1].frequency = SERDES_FREQ_USB_5G;
        macro_cfg->cs[CS1].ssc_enable = 0;
        if (macro_cfg->cs[CS0].ssc_enable == 1) {
            HILINK_ERR_SNAPSHOT(SERDES_INFO_CHECK_USB_AND_PCIE_SSC_FAIL,
                "SSC is not allowed to be enabled in pcie & usb mux scene.\n");
            return -EINVAL;
        }
    }
    if (hilink_cfg_is_exist_prot(macro_cfg, SERDES_TYPE_ETH) == true) {
        macro_cfg->cs[CS0].type = SERDES_TYPE_ETH;
        macro_cfg->cs[CS0].frequency = eth_freq_plan;
        macro_cfg->cs[CS0].ssc_enable = 0;
    }

    // select mclk
    for (ds = 0; ds < DS_MAX; ds++) {
        switch (macro_cfg->ds[ds].type) {
            case SERDES_TYPE_PCIE:
            case SERDES_TYPE_USB:
                macro_cfg->ds[ds].cs_select = CS1;
                break;
            case SERDES_TYPE_SATA:
            case SERDES_TYPE_ETH:
                macro_cfg->ds[ds].cs_select = CS0;
                break;
            default:
                macro_cfg->ds[ds].cs_select = CS0;
                break;
        }
    }

    return 0;
}

STATIC int hilink_cfg_get_serdes_lane_info(struct drv_cfg_serdes_raw_data *serdes_cfg)
{
    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    struct drv_cfg_serdes_lane_info *lane_info;
    struct hilink_macro_cfg *macro_cfg;
    DRV_SERDES_FREQUENCY_LEVEL eth_freq_plan = 0;
    u32 macro;
    u32 pcie_ssc;
    u32 sata_ssc;

    if ((serdes_cfg->mod_head.magic != DRV_DDR_ELASTIC_CFG_MODULE_MAGIC) ||
        (serdes_cfg->mod_head.mod_id != 0)) {
        HILINK_ERR_SNAPSHOT(SERDES_INFO_CHECK_DDR_NODE_FAIL, "DDR cfg area head check failed. (magic=%u; mod_id=%u)\n",
            serdes_cfg->mod_head.magic, serdes_cfg->mod_head.mod_id);
        return -EINVAL;
    }

    lane_info = serdes_cfg->data;
    for (macro = 0; macro < MACRO_MAX; macro++) {
        macro_cfg = &(hilinkPara->macro_cfg[macro]);
        if (hilink_cfg_get_lane_prot(macro_cfg, &lane_info[macro * DS_MAX], macro) != 0) {
            HILINK_ERR("DDR cfg get lane prot failed. (macro=%u)\n", macro);
            return -EINVAL;
        }
        if (hilink_cfg_get_ssc_enable(&lane_info[macro * DS_MAX], &pcie_ssc, &sata_ssc) != 0) {
            HILINK_ERR("DDR cfg get ssc failed. (macro=%u)\n", macro);
            return -EINVAL;
        }
        if (hilink_cfg_get_eth_freq_plan(&lane_info[macro * DS_MAX], &eth_freq_plan) != 0) {
            HILINK_ERR("DDR cfg get eth ferq plan failed. (macro=%u)\n", macro);
            return -EINVAL;
        }
        if (hilink_cfg_build_freq_plan(macro_cfg, pcie_ssc, sata_ssc, eth_freq_plan) != 0) {
            HILINK_ERR("DDR cfg build freq plan failed. (macro=%u)\n", macro);
            return -EINVAL;
        }
    }

    return 0;
}

int hilink_elastic_cfg_init(void)
{
    void __iomem *hilink_sram_cfg_base; // only pos info in SRAM
    void __iomem *hilink_ddr_cfg_base;  // real cfg data in DDR
    u32 cfg_area_offset = 0; // whole cfg area
    u32 serdes_offset = 0; // serdes cfg offset in whole cfg area
    int ret;

    hilink_sram_cfg_base = ioremap(DRV_SRAM_BASE + DRV_SRAM_ELASTIC_CFG_OFFSET, DRV_SRAM_ELASTIC_CFG_SIZE);
    if (hilink_sram_cfg_base == NULL) {
        HILINK_ERR_SNAPSHOT(SERDES_CFG_REMAP_SRAM_FAIL, "SRAM cfg area ioremap failed.\n");
        return -ENOMEM;
    }

    ret = hilink_cfg_get_node_pos_info(hilink_sram_cfg_base, &cfg_area_offset, &serdes_offset);
    if (ret != 0) {
        HILINK_ERR("Get cfg pos info from SRAM failed.\n");
        goto GET_CFG_NODE_POS_ERR;
    }

    hilink_ddr_cfg_base = ioremap(cfg_area_offset, DRV_DDR_ELASTIC_CFG_SIZE);
    if (hilink_ddr_cfg_base == NULL) {
        ret = -ENOMEM;
        HILINK_ERR_SNAPSHOT(SERDES_CFG_REMAP_DDR_FAIL, "DDR cfg area ioremap failed.\n");
        goto GET_CFG_NODE_POS_ERR;
    }
    ret = hilink_cfg_get_serdes_lane_info((struct drv_cfg_serdes_raw_data *)(hilink_ddr_cfg_base + serdes_offset));
    if (ret != 0) {
        HILINK_ERR("Get Serdes lane info from DDR failed.\n");
        goto GET_CFG_NODE_DATA_ERR;
    }

    HILINK_INFO("Get serdes lane info from DDR success.\n");
GET_CFG_NODE_DATA_ERR:
    iounmap(hilink_ddr_cfg_base);
    hilink_ddr_cfg_base = NULL;

GET_CFG_NODE_POS_ERR:
    iounmap(hilink_sram_cfg_base);
    hilink_sram_cfg_base = NULL;
    return ret;
}

int hilink_get_macro_cfg_by_board_id(struct hilink_macro_cfg *macro_cfg, uint32_t macro, uint32_t board_id)
{
    const struct hilink_macro_cfg *src;
    int ret;
    if (macro_cfg == NULL) {
        HILINK_ERR("macro_cfg is null.\n");
        return -ENOMEM;
    }

    CHECK_RETURN(macro, MACRO_MAX, -EINVAL);
    CHECK_RETURN(board_id, EVB_BOARD_ID_BOM2_20T_1, -EINVAL);

    if (board_id == EVB_BOARD_ID_BOM1_20T) {
        src = &(evb_board_900[macro]);
    } else {
        src = &(evb_board_901[macro]);
    }
    ret = memcpy_s(macro_cfg, sizeof(struct hilink_macro_cfg), src, sizeof(struct hilink_macro_cfg));
    if (ret != EOK) {
        HILINK_ERR("Copy cfg failed.\n");
        return -ENOMEM;
    }
    return 0;
}
