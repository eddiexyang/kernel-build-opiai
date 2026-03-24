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

#ifndef L3T_ARM_RAS_UNMASK_H
#define L3T_ARM_RAS_UNMASK_H

#if (!defined(CFG_SOC_PLATFORM_MINIV2)) && (!defined(CFG_SOC_PLATFORM_MDC_V51))
/*
 * l3t_relieve_suppresion - Relieve suppresion of the interrupt storm for l3t module
 * @device_id: device id, different chip has the different device nums
 * @oem_sub_module: node id
 */
void l3t_relieve_suppresion(unsigned char device_id, unsigned char oem_sub_module);

/*
 * l3t_arm_ras_unmask_init - Initialize unmask task for interrupt storm
 * @device_id: device id, different chip has the different device nums
 */
void l3t_arm_ras_unmask_init(u32 dev_id);

/*
 * l3t_arm_ras_unmask_uninit - Uninitialize unmask task for interrupt storm
 * @device_id: device id, different chip has the different device nums
 */
void l3t_arm_ras_unmask_uninit(u32 dev_id);
#else
static inline void l3t_relieve_suppresion(unsigned char device_id, unsigned char oem_sub_module)
{
}
static inline void l3t_arm_ras_unmask_init(u32 dev_id)
{
}
static inline void l3t_arm_ras_unmask_uninit(u32 dev_id)
{
}
#endif

#endif // L3T_ARM_RAS_UNMASK_H
