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

#include "devdrv_interface.h"
#include "profiling_drv.h"

int agentdrv_profling_open_dfx(u32 dev_id, void __iomem *io_base)
{
    return hisi_pcie_pmu_enable_profiling(dev_id);
}

int agentdrv_profiling_close_dfx(u32 dev_id, void __iomem *io_base)
{
    return hisi_pcie_pmu_disable_profiling(dev_id);
}

int agentdrv_profiling_get_info(u32 dev_id, const void __iomem *io_base, struct agentdrv_profiling_buf *info)
{
    return hisi_pcie_pmu_get_profiling_info(dev_id, info);
}
