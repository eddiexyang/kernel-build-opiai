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

#ifndef TS_DMS_INIT_H
#define TS_DMS_INIT_H
/*
 * TS SUBSYS NODE
 */
#define TS_NODE_0              (0U)

/*
 * sub module node number in TS SUBSYS
 */
#define TS_SUB_NODE_NUM        (3U)

/* The defined index value must be the same as the sequence of subsystems in the DMS node */
#define TS_SUB_DISP_NODE_INDEX  (0)
#define TS_SUB_SCHE_NODE_INDEX  (1)
#define TS_SUB_SMMU_NODE_INDEX  (2)

/**
 * ts_safety_manager_init - init safety management feature of TS
 *
 * Register the DMS node and DMS sensor of the TS module, initialize the
 * configuration parameters related to safety fault reporting, and register
 * the fault handling callback function to the FPDC.
 */
int ts_safety_manager_init(u32 devid);

/**
 * ts_safety_manager_uninit - init safety management feature of TS
 *
 * Unregister the DMS node and DMS sensor of the TS module, and unregister
 * the fault handling callback function from the FPDC.
 */
void ts_safety_manager_uninit(u32 devid);

#endif // TS_DMS_INIT_H
