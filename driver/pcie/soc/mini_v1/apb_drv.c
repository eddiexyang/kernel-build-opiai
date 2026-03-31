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
#include <linux/delay.h>

#include "apb_drv.h"

#define RC_EP_MODE_REG 0x1100CE088
#define RC_EP_MODE_REG_SIZE 0x4
#define RC_EP_MODE_REG_MODE_OFFSET 3
#define RC_EP_MODE_REG_MODE_MASK (0x1 << RC_EP_MODE_REG_MODE_OFFSET)

int devdrv_get_devid_by_bus(unsigned char bus_number, int *devid)
{
    (void)bus_number;
    *devid = 0;

    return 0;
}

int agentdrv_get_rc_ep_mode(u32 *mode)
{
    void __iomem *io_base = NULL;
    u32 val;

    if (mode == NULL) {
        return -EINVAL;
    }

    io_base = ioremap(RC_EP_MODE_REG, RC_EP_MODE_REG_SIZE);
    if (!io_base) {
        devdrv_err("Function ioremap RC EP reg failed.\n");
        return -ENOMEM;
    }
    devdrv_apb_reg_rd(io_base, 0, &val);
    iounmap(io_base);

    if (val & RC_EP_MODE_REG_MODE_MASK)
        *mode = DEVDRV_PCIE_EP_MODE;
    else
        *mode = DEVDRV_PCIE_RC_MODE;

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
    /* timeout 400ms, given by mini PCIe FS */
    int timeout = DEVDRV_DL_DLCMSM_STATE_TIMEOUT;
    u32 dl_dlcmsm_state;
    u32 reg_val;

    devdrv_apb_reg_rd(io_base, DEVDRV_DL_DFX_FSM_STATE, &reg_val);
    dl_dlcmsm_state = reg_val & DEVDRV_DL_DLCMSM_STATE_BIT;
    while (dl_dlcmsm_state != DEVDRV_DL_DLCMSM_STATE_OK) {
        if (timeout == 0) {
            devdrv_err("Function devdrv_check_dlcmsm timeout.\n");
            return -ETIMEDOUT;
        }

        devdrv_apb_reg_rd(io_base, DEVDRV_DL_DFX_FSM_STATE, &reg_val);
        dl_dlcmsm_state = reg_val & DEVDRV_DL_DLCMSM_STATE_BIT;

        udelay(100);
        timeout -= 100;
    }

    return 0;
}

void agentdrv_flush_cache(u64 base, u32 len)
{
}