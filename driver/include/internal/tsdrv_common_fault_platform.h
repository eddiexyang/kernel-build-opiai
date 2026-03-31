/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
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

#include <linux/workqueue.h>
#include "dms_define.h"
#include "dms_sensor.h"
#include "tsdrv_fault_init.h"
#include "dfm_dev_register.h"
#include "tsdrv_ras.h"

#ifdef CFG_FEATURE_FAULT_FPDC
#define GEN_RAS_FAULT_ITEM(_subsys_id, _module_id, _sec_type, _err_status, _describe, _sensor_type, _err_type) \
    { \
        .subsys_id = _subsys_id, \
        .module_id = _module_id, \
        .section_type = _sec_type, \
        .ras_code.err_status = _err_status, \
        .describe = _describe, \
        .sensor_type = _sensor_type, \
        .error_type = _err_type \
    }

struct ras_fault_converge_item *tsdrv_get_fault_converge_table(void);
int tsdrv_get_fault_converge_table_size(void);
void tsdrv_fill_tscpu_sensor_status(struct ras_error *error);
void fault_unmask_ras_irq(struct work_struct *work);
int fault_remap_l2buf(u32 devid);
void fault_unmap_l2buf(u32 devid);
bool fault_is_enable_unmask_ras_irp(u32 devid);
struct dfm_struct *tsdrv_get_dfm_by_node_type(u32 devid, int node_type);
struct dms_node *tsdrv_get_dms_node(u32 devid, int node_type, u32 node_id);
struct dms_sensor_object_cfg *tsdrv_get_sensor_cfg(int node_type, u32 *sensor_num);
void tsdrv_get_ras_node_types(struct tsdrv_ras_node_info **ras_nodes, int *num);

#endif

