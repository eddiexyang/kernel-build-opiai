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

#define PCIE_CORE_NUM 1u
#define PCIE_CORE_OFFSET 0

#define PCIE_AP_IOB_RX_REG_OFFSET 0x4000

#define PCIE_AMB_QOS_CTRL 0x1C08
#define PCIE_AMB_QOS_MASK 0xF
#define PCIE_AMB_QOS_WR_OFFSET 0u
#define PCIE_AMB_QOS_RD_OFFSET 4u

#define PCIE_AMB_ARUSER_SET 0x1C28
#define PCIE_AMB_ARUSER_MPAM_MASK 0xFF
#define PCIE_AMB_ARUSER_MPAM_OFFSET 8u
#define PCIE_AMB_ARUSER_PMG_MASK 0x3
#define PCIE_AMB_ARUSER_PMG_OFFSET 16u

#define PCIE_AMB_AWUSER_SET 0x1C4C
#define PCIE_AMB_AWUSER_MPAM_MASK 0xFF
#define PCIE_AMB_AWUSER_MPAM_OFFSET 0u
#define PCIE_AMB_AWUSER_PMG_MASK 0x3
#define PCIE_AMB_AWUSER_PMG_OFFSET 8u

#define PCIE_AP_DMA_REG_OFFSET 0x10000
#define PCIE_DMA_QUEUE_CTRL1 0x2024
#define PCIE_DMA_CHAN_OFFSET 0x100

#define PCIE_DMA_QOS_ENABLE_MASK 0x1
#define PCIE_DMA_QOS_ENABLE_OFFSET 16u
#define PCIE_DMA_QOS_ID_MASK 0xF
#define PCIE_DMA_QOS_ID_OFFSET 17u
#define PCIE_DMA_QOS_NS_MASK 0x1
#define PCIE_DMA_QOS_NS_OFFSET 21u
#define PCIE_DMA_QOS_PMG_MASK 0x3
#define PCIE_DMA_QOS_PMG_OFFSET 22u
#define PCIE_DMA_QOS_MPAM_MASK 0xFF
#define PCIE_DMA_QOS_MPAM_OFFSET 24u
