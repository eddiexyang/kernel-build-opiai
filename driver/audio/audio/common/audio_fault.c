/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: audio fault
 * Author: Hisilicon multimedia software group
 * Create: 2022-11-21
 */

#include "audio_fault.h"
#include "mkp_aio.h"
#include "hi_comm_audio.h"
#include "securec.h"

td_u32 g_audio_fault;
#define FAULT_DATA_SIZE 32

struct dms_node_operations g_audio_ops = {
    .init = audio_fault_ops_init,
    .uninit = audio_fault_ops_exit
};
struct dms_sensor_object_cfg g_audio_sensor_table[1U][1U] = {
    { /* dev_node 0 */
        AUDIO_SENSOR_DEF(DMS_SEN_TYPE_GENERAL_SOFTWARE_FAULT, "audio", 0UL, AUDIO_SENSOR, 0UL,
                         R_AST_MASK, R_DST_MASK, SCAN_FUNC),
    }
};

/* audio dev node table */
struct dms_node g_audio_dev_node_table[AUDIO_NODE_ID_MAX][AUDIO_SENSOR_ID_MAX] = {
    { /* dev_node 0 */
        AUDIO_NODE_DEF(DMS_DEV_TYPE_AUDIO, "audio", AUDIO_NODE_0, AUDIO_SENSOR, g_audio_ops),
    }
};

/* set audio fault */
void audio_set_err(td_u32 err)
{
    g_audio_fault = err;
}
EXPORT_SYMBOL(audio_set_err);

/* get audio fault */
static td_u32 audio_get_err(void)
{
    return g_audio_fault;
}

/* audio fault ops init */
td_s32 audio_fault_ops_init(struct dms_node *device)
{
    if (device != NULL) {
        aio_debug_trace("node name = %s", device->node_name);
    }
    return 0;
}

/* audio fault ops exit func */
void audio_fault_ops_exit(struct dms_node *device)
{
    if (device != NULL) {
        aio_debug_trace("node name = %s", device->node_name);
    }
    return;
}

/* scan audio fault */
td_s32 audio_event_scan(unsigned long long private_data, struct dms_sensor_event_data *data)
{
    data->event_count = 0;
    td_s32 ret;
    td_u8 describe[] = "audio fault err";
    data->sensor_data[data->event_count].current_value = audio_get_err();
    data->sensor_data[data->event_count].data_size = FAULT_DATA_SIZE;
    ret = memcpy_s(data->sensor_data[data->event_count].event_data, DMS_MAX_EVENT_DATA_LENGTH,
                   describe, sizeof(describe));
    if (ret != EOK) {
        aio_debug_trace("audio fault memcpy fail!\n");
        return ret;
    }

    return 0;
}

/* register audio node and audio sensor */
void audio_fault_init(void)
{
    td_s32 ret;
    struct dms_node *node = &g_audio_dev_node_table[0U][0U];
    struct dms_sensor_object_cfg *audio_sensor_obj_cfg = g_audio_sensor_table[0U];
    ret = dms_register_dev_node(node); /* register audio node */
    if (ret != TD_SUCCESS) {
        aio_err_trace("dms register dev node failed");
        return;
    }

    ret = dms_sensor_register(node, audio_sensor_obj_cfg); /* register audio sensor */
    if (ret != TD_SUCCESS) {
        aio_err_trace("dms sensor register failed");
        dms_unregister_dev_node(node);
    }
}
EXPORT_SYMBOL(audio_fault_init);

/* unregister audio node and audio sensor */
void audio_fault_exit(void)
{
    struct dms_node *node = &g_audio_dev_node_table[0U][0U];
    (void)dms_sensor_node_unregister(node);
    (void)dms_unregister_dev_node(node);
    return;
}
EXPORT_SYMBOL(audio_fault_exit);