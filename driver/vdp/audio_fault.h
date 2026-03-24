/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: audio fault
 * Author: Hisilicon multimedia software group
 * Create: 2022-11-21
 */

#ifndef AUDIO_FAULT_H
#define AUDIO_FAULT_H

#include "hi_comm_audio.h"
#include "dms_dev_node.h"
#include "dms_sensor_type.h"
#include "dms_interface.h"
#include "dms_sensor.h"
#include "dms_define.h"

#define R_AST_MASK 0xFFFF       /* DMS_SEN_TYPE_RAS_SENSOR assert_event_mask */
#define R_DST_MASK 0xFFFF       /* DMS_SEN_TYPE_RAS_SENSOR deassert_event_mask */

#define AUDIO_OFFSET_16BIT  16
#define AUDIO_OFFSET_32BIT  32
/* device node */
enum audio_dev_node {
    AUDIO_NODE_0 = 0,
    AUDIO_NODE_ID_MAX
};

enum audio_sensor_id {
    AUDIO_SENSOR = 0,
    AUDIO_SENSOR_ID_MAX
};

#define AUDIO_NODE_DEF(type, name, devid, id, m_ops) { \
    .node_type = type,      \
    .node_id = id,          \
    .node_name = name,      \
    .owner_devid = devid,   \
    .ops = &m_ops           \
}

td_s32 audio_fault_ops_init(struct dms_node *device);
void audio_fault_ops_exit(struct dms_node *device);

td_s32 audio_event_scan(unsigned long long private_data, struct dms_sensor_event_data *data);

#define SCAN_FUNC audio_event_scan

#define DMS_SENSOR_SCAN_INTERVAL 300 /* 300 ms */

#define AUDIO_SENSOR_DEF(type, name, devid, s_id, s_idx, assert_mask, deassert_mask, func) { \
    .sensor_type = type, .sensor_name = name, .sensor_class = DMS_DISCRETE_SENSOR_CLASS,     \
    .sensor_class_cfg = {                                                                    \
        .discrete_sensor = { .attribute = DMS_SENSOR_ATTRIB_THRES_NONE, .debounce_time = 0 } \
    },                                                                                       \
    .scan_interval = DMS_SENSOR_SCAN_INTERVAL, .proc_flag = DMS_SENSOR_PROC_ENABLE_FLAG,     \
    .enable_flag = DMS_SENSOR_ENABLE_FALG, .pf_scan_func = func,                             \
    .private_data = (((devid) << AUDIO_OFFSET_16BIT) | ((unsigned long)(s_id) << AUDIO_OFFSET_16BIT) | (s_idx)), \
    .assert_event_mask = assert_mask, .deassert_event_mask = deassert_mask                           \
}

void audio_fault_init(void);
void audio_fault_exit(void);
void audio_set_err(td_u32 err);
#endif
