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

#ifndef L3T_DMS_INIT_H
#define L3T_DMS_INIT_H
#include "dfm_dev_register.h"

extern struct dfm_struct g_l3t_dfm[SOC_MISC_DEVICE_NUM_MAX];

/*
* l3t_sensor_scan - scan the event of l3t in the dfm's sensor fault event list.
* @private_data: composed of device id, node id, and sensor id
* @data: scan result will store to data
*
* If node_id exceeds the range of the device, -EINVAL is returned.
*/
int l3t_sensor_scan(unsigned long long private_data, struct dms_sensor_event_data *data);

/*
 * l3t_fault_manager_init - Initialize fault of the l3t module.
 *
 * This interface is used to register dms node of l3t module, initialize safety fault type to faultmng,
 * initialize irq fault type to faultmng, initialize arm ras fault type to fpdc and faultmng
 */
int l3t_fault_manager_init(void);

/*
 * l3t_fault_manager_uninit - Uninitialize fault of the l3t module.
 *
 * This interface is used to unregister dms node of l3t module, uninitialize safety fault type to faultmng,
 * uninitialize irq fault type to faultmng, uninitialize arm ras fault type to fpdc and faultmng
 */
void l3t_fault_manager_uninit(void);

#endif /* l3t_DMS_INIT_H */
