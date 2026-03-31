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

#define CHIP_0_ECAM_BUS_ID 0x7B
#define CHIP_1_ECAM_BUS_ID 0xBB
#define CHIP_2_ECAM_BUS_ID 0xDB
#define CHIP_3_ECAM_BUS_ID 0xFB

int g_pf_type = -1;

int devdrv_get_devid_by_bus(unsigned char bus_number, int *devid)
{
    int ret = 0;

    switch (bus_number) {
        case CHIP_0_ECAM_BUS_ID:
            *devid = 0;
            break;
        case CHIP_1_ECAM_BUS_ID:
            *devid = 1;
            break;
        case CHIP_2_ECAM_BUS_ID:
            *devid = DEVDRV_DEVID_2;
            break;
        case CHIP_3_ECAM_BUS_ID:
            *devid = DEVDRV_DEVID_3;
            break;
        default:
            ret = -1;
            break;
    }

    return ret;
}

int agentdrv_get_connect_type_by_hw_info(void)
{
    return CONNECT_PROTOCOL_PCIE;
}

int agentdrv_get_rc_ep_mode(u32 *mode)
{
    if (mode == NULL) {
        return -EINVAL;
    }

#ifndef CFG_SOC_PLATFORM_MDC_V2
    *mode = DEVDRV_PCIE_EP_MODE;
#else
    *mode = DEVDRV_PCIE_RC_MODE;
#endif
    return 0;
}
EXPORT_SYMBOL(agentdrv_get_rc_ep_mode);

int devdrv_get_pf_type(void)
{
    void __iomem *rd_base = NULL;
    u32 sub_device_id = 0;

    if (g_pf_type != -1) {
        return g_pf_type;
    }

    rd_base = ioremap(DEVDRV_CFGSPACE_SUB_DEVICE_ID, sizeof(u32));
    if (rd_base == NULL) {
        devdrv_warn("Function ioremap err, force pf type to 1pf1p.\n");
        return DEVDRV_DAVINCI_DEV_NUM_1PF1P;
    }

    sub_device_id = readl(rd_base);
    if ((sub_device_id >> DEVDRV_PCI_SUBSYS_DEV_MASK_BIT_IN_SBUSYS) ==
        (DEVDRV_1PF2P_SUBSYS_DEV >> DEVDRV_PCI_SUBSYS_DEV_MASK_BIT)) {
        g_pf_type = DEVDRV_DAVINCI_DEV_NUM_1PF2P;
    } else {
        g_pf_type = DEVDRV_DAVINCI_DEV_NUM_1PF1P;
    }

    iounmap(rd_base);
    rd_base = NULL;

    return g_pf_type;
}

u32 agentdrv_soc_get_func_total(void)
{
#ifdef CFG_SOC_PLATFORM_MDC_LITE_ESL
    devdrv_info("lite esl return 1.\n");
    return 1;
#else
    void __iomem *rd_base = NULL;
    u32 pf_support_enable;
    u32 pf_total_num = 0;

    rd_base = ioremap(DEVDRV_RM_PF_SUPPOT_ENABLE, sizeof(u32));
    if (rd_base == NULL) {
        devdrv_warn("Function ioremap err, force pf_total_num to 1.\n");
        return 1;
    }

    pf_support_enable = readl(rd_base);
    if (pf_support_enable == 0x1) {
        pf_total_num = 1;
    } else if (pf_support_enable == 0x3) {
        pf_total_num = 2;
    } else {
        pf_total_num = 1;
        devdrv_warn("Variable pf_support invalid, force pf_total_num to 1.\n");
    }

    if (devdrv_get_pf_type() == DEVDRV_DAVINCI_DEV_NUM_1PF2P) {
        pf_total_num = DEVDRV_DAVINCI_DEV_NUM_1PF2P;
    }
    devdrv_info("Init to get pf totl num in device. (pf_support_enable=%d; num=%d)\n", pf_support_enable, pf_total_num);
    iounmap(rd_base);
    rd_base = NULL;

    return pf_total_num;
#endif
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