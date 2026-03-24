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

#include <linux/io.h>
#include "devdrv_interface.h"
#include "profiling_drv.h"

int agentdrv_profling_open_dfx(u32 dev_id, void __iomem *io_base)
{
    /* open TX dfx bandwidth statistic */
    writel(BW_CTRL_ENABLE_CMD, io_base + AGENTDRV_TX_P_BW_CTRL);
    writel(BW_CTRL_ENABLE_CMD, io_base + AGENTDRV_TX_NP_BW_CTRL);
    writel(BW_CTRL_ENABLE_CMD, io_base + AGENTDRV_TX_CPL_BW_CTRL);

    writel(LAN_CTRL_ENABLE_CMD, io_base + AGENTDRV_TX_NP_LAN_CTRL);

    /* open RX dfx bandwidth static. note rx only one ctrl register */
    writel(RX_BW_ENABLE_CMD, io_base + AGENTDRV_RX_BW_CTRL_REG);

    return 0;
}

int agentdrv_profiling_close_dfx(u32 dev_id, void __iomem *io_base)
{
    /* close TX DFX */
    writel(BW_CTRL_DISABLE_CMD, io_base + AGENTDRV_TX_P_BW_CTRL);
    writel(BW_CTRL_DISABLE_CMD, io_base + AGENTDRV_TX_NP_BW_CTRL);
    writel(BW_CTRL_DISABLE_CMD, io_base + AGENTDRV_TX_CPL_BW_CTRL);

    writel(LAN_CTRL_DISABLE_CMD, io_base + AGENTDRV_TX_NP_LAN_CTRL);

    /* close RX DFX and clear TX DFX */
    writel(RX_BW_DISABLE_CMD, io_base + AGENTDRV_RX_BW_CTRL_REG);
    writel(RX_BW_CLEAR_CMD, io_base + AGENTDRV_RX_BW_CLR_REG);

    return 0;
}

int agentdrv_profiling_get_info(u32 dev_id, const void __iomem *io_base, struct agentdrv_profiling_buf *info)
{
    struct timespec64 uptime;
    u32 regval = 0;

    ktime_get_raw_ts64(&uptime);
    info->dev_id = dev_id;
    info->time = (u64)timespec64_to_ns(&uptime);
    info->tx_p_bw[0] = readl(io_base + TX_P_BW_MIN);
    info->tx_p_bw[1] = readl(io_base + TX_P_BW_MAX);
    info->tx_p_bw[2] = readl(io_base + TX_P_BW_AVERAGE);
    info->tx_np_bw[0] = readl(io_base + TX_NP_BW_MIN);
    info->tx_np_bw[1] = readl(io_base + TX_NP_BW_MAX);
    info->tx_np_bw[2] = readl(io_base + TX_NP_BW_AVERAGE);
    info->tx_cpl_bw[0] = readl(io_base + TX_CPL_BW_MIN);
    info->tx_cpl_bw[1] = readl(io_base + TX_CPL_BW_MAX);
    info->tx_cpl_bw[2] = readl(io_base + TX_CPL_BW_AVERAGE);
    info->tx_np_lantency[0] = readl(io_base + TX_NP_LAN_MIN);
    info->tx_np_lantency[1] = readl(io_base + TX_NP_LAN_MAX);
    info->tx_np_lantency[2] = readl(io_base + TX_NP_LAN_AVERAGE);

    regval = readl(io_base + RX_P_BW_CUR);
    info->rx_p_bw[0] = regval & 0xffff;
    regval = readl(io_base + RX_P_BW);
    info->rx_p_bw[1] = regval >> 16;
    info->rx_p_bw[2] = regval & 0xffff;

    regval = readl(io_base + RX_NP_BW_CUR);
    info->rx_np_bw[0] = regval & 0xffff;
    regval = readl(io_base + RX_NP_BW);
    info->rx_np_bw[1] = regval >> 16;
    info->rx_np_bw[2] = regval & 0xffff;

    regval = readl(io_base + RX_CPL_BW_CUR);
    info->rx_cpl_bw[0] = regval & 0xffff;
    regval = readl(io_base + RX_CPL_BW);
    info->rx_cpl_bw[1] = regval >> 16;
    info->rx_cpl_bw[2] = regval & 0xffff;

    return 0;
}
