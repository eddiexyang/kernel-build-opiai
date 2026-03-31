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


#ifdef CFG_SOC_PLATFORM_CLOUD
#define SOC_PERI_BASE              0x0
#define SOC_IO_BASE                0x0
#define SOC_PERI_GIC0_REG          (SOC_PERI_BASE + 0x109000000)
#define SOC_PERI_DISP_REG_REG      (SOC_PERI_BASE + 0x102060000)
#define SOC_TS_SYSCTRL0_REG        (SOC_TS_BASE + 0xAF030000)
#define SOC_TS_DOORBELL_CFG0_REG   (SOC_TS_BASE + 0xAF400000)
#define SOC_TS_SRAM_REG0_REG       (SOC_TS_BASE + 0xAF2C0000)
#define SOC_IO_SYSCTRL_REG_REG     (SOC_IO_BASE + 0x1100C0000)

#define DEVDRV_MANAGER_PERI_GIC0_REG                SOC_PERI_GIC0_REG
#define DEVDRV_MANAGER_PERI_GIC0_SIZE               0x10000
#define DEVDRV_MANAGER_PERI_DISPATCH_REG            SOC_PERI_DISP_REG_REG
#define DEVDRV_MANAGER_PERI_DISPATCH_SIZE           0x10000
#define DEVDRV_MANAGER_TS_SUBSYSCTL_REG             SOC_TS_SYSCTRL0_REG
#define DEVDRV_MANAGER_TS_SUBSYSCTL_SIZE            0x10000
#define DEVDRV_MANAGER_TS_DOORBELL_REG              SOC_TS_DOORBELL_CFG0_REG
#define DEVDRV_MANAGER_TS_DOORBELL_SIZE             0x400000
#define DEVDRV_MANAGER_TS_SRAM_REG                  SOC_TS_SRAM_REG0_REG
#define DEVDRV_MANAGER_TS_SRAM_SIZE                 0x10000
#define DEVDRV_MANAGER_PERI_SYSCTL_REG              SOC_IO_SYSCTRL_REG_REG
#define DEVDRV_MANAGER_PERI_SYSCTL_SIZE             0x10000
#define DEVDRV_MANAGER_PERI_LPM3_STATUS_REG         SOC_IO_SYSCTRL_REG_REG + 0x3000
#define DEVDRV_MANAGER_PERI_LPM3_STATUS_SIZE        0x1000
#define DEVDRV_MANAGER_TSENSOR_SHAREMEM_REG         0x000A20000
#define DEVDRV_MANAGER_TSENSOR_SHAREMEM_SIZE        0x1000
#endif

#endif
