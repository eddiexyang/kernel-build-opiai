/**
 *
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * @file soc_adss_sub_subctrl_interface.h
 */

#ifndef __SOC_ADSS_SUB_SUBCTRL_INTERFACE_H__
#define __SOC_ADSS_SUB_SUBCTRL_INTERFACE_H__

#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif

#define SOC_ADSS_SUB_SUBCTRL_SC_AUDIO_ICG_DIS_ADDR(base)      ((base) + (0x404UL))
#define SOC_ADSS_SUB_SUBCTRL_SC_AUDIO_ICG_EN_ADDR(base)       ((base) + (0x400UL))
#define SOC_ADSS_SUB_SUBCTRL_SC_AUDIO_RESET_REQ(base)         ((base) + (0xA00UL))
#define SOC_ADSS_SUB_SUBCTRL_SC_AUDIO_RESET_DREQ(base)        ((base) + (0xA04UL))

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of soc_adss_sub_subctrl_interface.h */
