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

#ifndef SOC_MISC_CONFIG_H
#define SOC_MISC_CONFIG_H

#ifndef CFG_SOC_PLATFORM_MINIV2
#  error "This header can only include in ascend310p"
#endif

#include "soc_misc_ring.h"
#include "soc_misc_pcie.h"
#include "soc_misc_mn.h"

#include "soc_misc_dms_def.h"
#include "soc_misc_common.h"

/**
* sensor num
*/

#define SOC_MISC_SENSOR_RING_NUM 1
#define SOC_MISC_SENSOR_PCIE_NUM 1


STATIC struct dms_node_operations g_soc_misc_ring_ops = {
    .init = soc_misc_ring_ops_init,
    .uninit = soc_misc_ring_ops_uninit,
    .scan = NULL,
    .fault_diag = NULL,
    .event_notify = NULL,
    .get_link_state = NULL,
    .set_link_state = NULL
};

STATIC struct dms_node_operations g_soc_misc_pcie_ops = {
    .init = soc_misc_pcie_ops_init,
    .uninit = soc_misc_pcie_ops_uninit,
    .scan = NULL,
    .fault_diag = NULL,
    .event_notify = NULL,
    .get_link_state = NULL,
    .set_link_state = NULL
};


#define SOC_MISC_RING_NODES(_dev_id) { \
    SOC_MISC_RING_NODE(_dev_id, 0),    \
    SOC_MISC_RING_NODE(_dev_id, 1),    \
    SOC_MISC_RING_NODE(_dev_id, 2),    \
    SOC_MISC_RING_NODE(_dev_id, 3),    \
}

#define SOC_MISC_PCIE_NODES(_dev_id) { \
    SOC_MISC_PCIE_NODE(_dev_id, 0),    \
    SOC_MISC_PCIE_NODE(_dev_id, 1),    \
    SOC_MISC_PCIE_NODE(_dev_id, 2),    \
    SOC_MISC_PCIE_NODE(_dev_id, 3),    \
}

#define SOC_MISC_RESV_NODES(_dev_id) { \
    SOC_MISC_RESV_NODE(_dev_id, 0),    \
    SOC_MISC_RESV_NODE(_dev_id, 1),    \
    SOC_MISC_RESV_NODE(_dev_id, 2),    \
    SOC_MISC_RESV_NODE(_dev_id, 3),    \
}

#define SOC_MISC_DEV_NODES(_dev_id) { \
    SOC_MISC_RESV_NODES(_dev_id),  \
    SOC_MISC_RESV_NODES(_dev_id),      \
    SOC_MISC_RESV_NODES(_dev_id),      \
    SOC_MISC_RING_NODES(_dev_id),     \
    SOC_MISC_PCIE_NODES(_dev_id),     \
}

STATIC struct dms_node
    g_soc_misc_dev_node_table[SOC_MISC_DEVICE_NUM_MAX][SOC_MISC_SENSOR_ID_MAX][SOC_MISC_MAX_NODE_NUM] = {
        SOC_MISC_DEV_NODES(SOC_MISC_NODE_0),
        SOC_MISC_DEV_NODES(SOC_MISC_NODE_1),
    };

#ifdef CFG_FEATURE_PCIE_LOCAL_FAULT
STATIC struct dms_sensor_object_cfg
g_soc_misc_pcie_sensor_table[SOC_MISC_DEVICE_NUM_MAX][SOC_MISC_MAX_NODE_NUM][SOC_MISC_SENSOR_PCIE_NUM] = {
    { /* dev_node 0 */
        { SOC_MISC_DMS_SENSOR(SOC_MISC_NODE_0, 0, DMS_SEN_TYPE_RAS_SENSOR, "pcie_local", soc_misc_pcie_sensor_scan) },
        { SOC_MISC_DMS_SENSOR(SOC_MISC_NODE_0, 1, DMS_SEN_TYPE_RAS_SENSOR, "pcie_local", soc_misc_pcie_sensor_scan) },
        { SOC_MISC_DMS_SENSOR(SOC_MISC_NODE_0, 2, DMS_SEN_TYPE_RAS_SENSOR, "pcie_local", soc_misc_pcie_sensor_scan) },
        { SOC_MISC_DMS_SENSOR(SOC_MISC_NODE_0, 3, DMS_SEN_TYPE_RAS_SENSOR, "pcie_local", soc_misc_pcie_sensor_scan) },
    }, { /* dev_node 1 */
        { SOC_MISC_DMS_SENSOR(SOC_MISC_NODE_1, 0, DMS_SEN_TYPE_RAS_SENSOR, "pcie_local", soc_misc_pcie_sensor_scan) },
        { SOC_MISC_DMS_SENSOR(SOC_MISC_NODE_1, 1, DMS_SEN_TYPE_RAS_SENSOR, "pcie_local", soc_misc_pcie_sensor_scan) },
        { SOC_MISC_DMS_SENSOR(SOC_MISC_NODE_1, 2, DMS_SEN_TYPE_RAS_SENSOR, "pcie_local", soc_misc_pcie_sensor_scan) },
        { SOC_MISC_DMS_SENSOR(SOC_MISC_NODE_1, 3, DMS_SEN_TYPE_RAS_SENSOR, "pcie_local", soc_misc_pcie_sensor_scan) },
    },
};
#endif // CFG_FEATURE_PCIE_LOCAL_FAULT

#endif /* SOC_CONFIG_H */
