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

#ifndef __DEVDRV_MANAGER_REG_DEFINE_H
#define __DEVDRV_MANAGER_REG_DEFINE_H


#if defined(CFG_SOC_PLATFORM_MINIV2)
#define SOC_PERI_BASE                       0x0
#define SOC_PERI0_BASE                      0x0
#define SOC_IO_BASE                         0x0
#define SOC_TS_AIC_SYSCTRL_BASE             0xba080000
#define SOC_TS_VEC_SYSCTRL_BASE             0xb9080000
#define SOC_PERI_GIC0_REG                   (SOC_PERI_BASE + 0x109000000)
#define SOC_PERI_DISP_REG_REG               (SOC_PERI_BASE + 0x102060000)
#define SOC_PERI0_LBUS_HSLV_REG             (SOC_PERI0_BASE + 0x80000000)
#define SOC_IO_SYSCTRL_REG_REG              (SOC_IO_BASE + 0x1100C0000)

#define DEVDRV_MANAGER_TS_AIC_SYSCTRL_BASE          SOC_TS_AIC_SYSCTRL_BASE
#define DEVDRV_MANAGER_TS_AIV_SYSCTRL_BASE          SOC_TS_VEC_SYSCTRL_BASE
#define DEVDRV_MANAGER_TS_SYSCTRL_SIZE              0x1000
#define DEVDRV_MANAGER_PERI_GIC0_REG                SOC_PERI_GIC0_REG
#define DEVDRV_MANAGER_PERI_GIC0_SIZE               0x10000
#define DEVDRV_MANAGER_PERI_DISPATCH_REG            SOC_PERI_DISP_REG_REG
#define DEVDRV_MANAGER_PERI_DISPATCH_SIZE           0x10000
#define DEVDRV_MANAGER_PERI_SYSCTL_REG              SOC_PERI0_LBUS_HSLV_REG
#define DEVDRV_MANAGER_PERI_SYSCTL_SIZE             0x10000
#define DEVDRV_MANAGER_PERI_LPM3_STATUS_REG         SOC_IO_SYSCTRL_REG_REG + 0x3000
#define DEVDRV_MANAGER_PERI_LPM3_STATUS_SIZE        0x1000
#endif

#endif
