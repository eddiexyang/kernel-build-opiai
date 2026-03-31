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

#ifndef AIV_DMS_INIT_H
#define AIV_DMS_INIT_H
/*
 * AIV SUBSYS NODE
 */
#define AIV_NODE_NUM      (8U)
/*
 * sub module node number in AIV SUBSYS
 */
#define AIV_SUB_NODE_NUM_PER        (2U)
#define AIV_SUB_NODE_NUM        (AIV_SUB_NODE_NUM_PER * AIV_NODE_NUM)

/* The defined index value must be the same as the sequence of subsystems in the DMS node */
#define AIV_SUB_DISP_NODE_INDEX  (0)
#define AIV_SUB_SMMU_NODE_INDEX  (1)

/**
 * aiv_safety_manager_init - init safety management feature of AIV
 *
 * Register the DMS node and DMS sensor of the AIV module, initialize the
 * configuration parameters related to safety fault reporting, and register
 * the fault handling callback function to the FPDC.
 */
int aiv_safety_manager_init(u32 devid);

/**
 * aiv_safety_manager_uninit - init safety management feature of AIV
 *
 * Unregister the DMS node and DMS sensor of the AIV module, and unregister
 * the fault handling callback function from the FPDC.
 */
void aiv_safety_manager_uninit(u32 devid);

#endif // AIV_DMS_INIT_H
