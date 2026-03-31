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

#include <linux/types.h>
#include <linux/kernel.h>

#include "soc_misc_cpucluster.h"
#include "soc_misc_io_subsys.h"
#include "soc_misc_hac_subsys.h"
#include "soc_misc_peri_subsys.h"
#include "soc_misc_mbigen_peri.h"
#include "soc_misc_mbigen_io.h"
#ifdef AOS_LLVM_BUILD
#  include "soc_misc_gic_ao.h"
#endif

#ifndef CFG_SOC_PLATFORM_MDC_LITE
struct soc_misc_safety_cfg *g_soc_misc_safety_cfgs[] = {
    &g_cpucluster_safety_cfg,
    &g_io_subsys_safety_cfg,
    &g_hac_subsys_safety_cfg,
    &g_peri_subsys_safety_cfg,
    &g_mbigen_peri_safety_cfg,
    &g_mbigen_io_safety_cfg,
#ifdef AOS_LLVM_BUILD
    &g_gic_ao_safety_cfg,
#endif
};
#endif

struct soc_misc_safety_cfg **soc_misc_get_safety_cfg_array(u32 *num)
{
#ifndef CFG_SOC_PLATFORM_MDC_LITE
    *num = ARRAY_SIZE(g_soc_misc_safety_cfgs);
    return g_soc_misc_safety_cfgs;
#else
    *num = 0;
    return NULL;
#endif
}
