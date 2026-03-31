
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
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

#include "devdrv_util.h"
#include "agentdrv_platform.h"
#include "agentdrv_unit.h"
#include "agentdrv_pcie_link_info.h"

static u32 g_pcie_channle_status = AGENTDRV_PCIE_INIT_ERR;

void devdrv_set_pcie_channel_status(u32 value)
{
#ifdef CFG_FEATURE_PCIE_LINK_INFO
    g_pcie_channle_status = value;
#endif
    return;
}

EXPORT_SYMBOL(devdrv_set_pcie_channel_status);

static int agentdrv_get_pcie_mac_link_info(struct agentdrv_devctrl *agent_dev,
    struct devdrv_pcie_link_info_para *pcie_link_info)
{
    u32 mac_reg_link_value;
    u32 ltssm_st;

    mac_reg_link_value = readl(agent_dev->apb_base + AGENTDRV_PCIE_MAC + PCIE_HIPCIEC_MAC_REG_LINK_INFO);
    devdrv_info("read mac reg link. (reg_value=0x%x)\n", mac_reg_link_value);

    // link_status
    pcie_link_info->link_status = AGENTDRV_PCIE_LINK_STATUS_DOWN;
    ltssm_st = (mac_reg_link_value >> PCIE_MAC_REG_LINK_LTSSM_ST_OFFSET) & 0x3F;
    if (ltssm_st == PCIE_MAC_REG_LINK_LTSSM_L0) {
        pcie_link_info->link_status = AGENTDRV_PCIE_LINK_STATUS_OK;
    } else {
        return 0;
    }

    // rate_mode
    pcie_link_info->rate_mode = (mac_reg_link_value >> PCIE_MAC_REG_LINK_SPEED_OFFSET) & 0xF;
    // lane_num
    pcie_link_info->lane_num = mac_reg_link_value & 0x3F;

    return 0;
}

int devdrv_get_pcie_link_info(u32 dev_id, struct devdrv_pcie_link_info_para* pcie_link_info)
{
    int ret;
    struct agentdrv_devctrl *agent_dev = NULL;

    if (pcie_link_info == NULL) {
        devdrv_err("pcie_link_info is NULL.\n");
        return -EINVAL;
    }
    agent_dev = agentdrv_get_dev((int)dev_id);
    if (agent_dev == NULL) {
        devdrv_err("call agentdrv_get_dev failed, agent_dev is null.\n");
        return -EINVAL;
    }
    ret = agentdrv_get_pcie_mac_link_info(agent_dev, pcie_link_info);
    if (ret != 0) {
        devdrv_err("get mac link info error. (ret=%d)\n", ret);
        return ret;
    }

    // if pcie link up status is down, no need to check channel status
    if (pcie_link_info->link_status == AGENTDRV_PCIE_LINK_STATUS_DOWN) {
        return 0;
    }

    if (g_pcie_channle_status != AGENTDRV_PCIE_INIT_OK) {
        pcie_link_info->link_status = AGENTDRV_PCIE_LINK_STATUS_CHANNEL_ERR;
    }

    return 0;
}

EXPORT_SYMBOL(devdrv_get_pcie_link_info);