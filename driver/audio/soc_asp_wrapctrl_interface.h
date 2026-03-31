/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __SOC_ASP_WRAPCTRL_INTERFACE_H__
#define __SOC_ASP_WRAPCTRL_INTERFACE_H__

#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif

#define SOC_ASP_WRAPCTRL_SC_AUDIO_BUS_CTRL_ADDR(base)   ((base) + (0x3B00UL))

#define SOC_ASP_WRAPCTRL_SC_AWUSER_M_AUDIO_ADDR(base)   ((base) + (0x3C2CUL))

#define SOC_ASP_WRAPCTRL_SC_ARUSER_M_AUDIO_ADDR(base)   ((base) + (0x3C38UL))

#define SOC_ASP_WRAPCTRL_SC_AUDIO_DWAXI_ST_ADDR(base)   ((base) + (0x6100UL))

#define SOC_ASP_WRAPCTRL_SC_AUDIO_BUS_CTRL_audio_bus_stop_START      (1)

#define SOC_ASP_WRAPCTRL_SC_AUDIO_DWAXI_ST_audio_bus_stop_ok_START    (28)

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of soc_ASP_WRAPCTRL_interface.h */
