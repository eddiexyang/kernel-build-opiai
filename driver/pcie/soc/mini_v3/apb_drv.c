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

#include <asm/io.h>

#include "apb_drv.h"

int devdrv_get_devid_by_bus(unsigned char bus_number, int *devid)
{
    *devid = 0;

    return 0;
}

int agentdrv_get_rc_ep_mode(u32 *mode)
{
    if (mode == NULL) {
        return -EINVAL;
    }
#ifdef CFG_SOC_PLATFORM_MINIV3_RC
    *mode = DEVDRV_PCIE_RC_MODE;
#else
    *mode = DEVDRV_PCIE_EP_MODE;
#endif
    return 0;
}
EXPORT_SYMBOL(agentdrv_get_rc_ep_mode);

int agentdrv_get_connect_type_by_hw_info(void)
{
    return CONNECT_PROTOCOL_PCIE;
}

int devdrv_get_pf_type(void)
{
    return DEVDRV_DAVINCI_DEV_NUM_1PF1P;
}

u32 agentdrv_soc_get_func_total(void)
{
    return 1;
}

/* No VF, pf total is equal to fun total */
u32 agentdrv_soc_get_pf_func_total(void)
{
    return agentdrv_soc_get_func_total();
}

int devdrv_check_dlcmsm(const void __iomem *io_base)
{
    return 0;
}

void agentdrv_flush_cache(u64 base, u32 len)
{
}