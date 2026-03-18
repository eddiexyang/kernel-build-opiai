/*
* Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
* Create: 2022-9-20
*/

#ifndef AIC_DMS_INIT_H
#define AIC_DMS_INIT_H

/*
 * AIC SUBSYS NODE
 */
#define AIC_NODE_NUM      (10U)
/*
 * sub module node number in AIC SUBSYS
 */
#define AIC_SUB_NODE_NUM_PER        (2U)
#define AIC_SUB_NODE_NUM        (AIC_SUB_NODE_NUM_PER * AIC_NODE_NUM)

/* The defined index value must be the same as the sequence of subsystems in the DMS node */
#define AIC_SUB_DISP_NODE_INDEX  (0)
#define AIC_SUB_SMMU_NODE_INDEX  (1)

/**
 * aic_safety_manager_init - init safety management feature of AIC
 *
 * Register the DMS node and DMS sensor of the AIC module, initialize the
 * configuration parameters related to safety fault reporting, and register
 * the fault handling callback function to the FPDC.
 */
int aic_safety_manager_init(u32 devid);

/**
 * aic_safety_manager_uninit - init safety management feature of AIC
 *
 * Unregister the DMS node and DMS sensor of the AIC module, and unregister
 * the fault handling callback function from the FPDC.
 */
void aic_safety_manager_uninit(u32 devid);

#endif // AIC_DMS_INIT_H
