/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
 * Create: 2023-01-03
 */
#ifndef SOC_MISC_CPUCORE_COMCFG_H
#define SOC_MISC_CPUCORE_COMCFG_H

void soc_misc_cpucore_ops_uninit(struct dms_node *device);
int soc_misc_cpucore_ops_init(struct dms_node *device);

struct dms_node_operations g_soc_misc_cpucore_ops = {
    .init = soc_misc_cpucore_ops_init,
    .uninit = soc_misc_cpucore_ops_uninit,
    .scan = NULL,
    .fault_diag = NULL,
    .event_notify = NULL,
    .get_link_state = NULL,
    .set_link_state = NULL
};

#define SOC_MISC_CPUCORE_DMS_NODE_DEFINE(_dev_id, _node_id, _ops) {    \
    .node_type = DMS_DEV_TYPE_CPU_CORE,                  \
    .node_id = _node_id,                                      \
    .node_name = "SOC-CPUCORE-" #_node_id,                    \
    .capacity = 0x1,                                     \
    .permission = 0x1,                                   \
    .owner_devid = _dev_id,                                    \
    .ops = _ops                                          \
}

#endif