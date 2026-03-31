/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __SOC_ASP_CFG_INTERFACE_H__
#define __SOC_ASP_CFG_INTERFACE_H__

#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif

#define SOC_ASP_CFG_R_RST_CTRLEN_ADDR(base)             ((base) + (0x0000UL))
#define SOC_ASP_CFG_R_RST_CTRLDIS_ADDR(base)            ((base) + (0x0004UL))
#define SOC_ASP_CFG_R_GATE_EN_ADDR(base)                ((base) + (0x000CUL))
#define SOC_ASP_CFG_R_GATE_CLKDIV_EN_ADDR(base)         ((base) + (0x001CUL))
#define SOC_ASP_CFG_R_GATE_R_CG_EN_ADDR(base)           ((base) + (0x005CUL))
#define SOC_ASP_CFG_R_GATE_R_AXI2TDM_CFG_ADDR(base)     ((base) + (0x02E4UL))

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of soc_asp_cfg_interface.h */
