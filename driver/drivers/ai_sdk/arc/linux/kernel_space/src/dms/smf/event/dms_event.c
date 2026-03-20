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

#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/kfifo.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/jiffies.h>
#include "dms_kernel_version_adapt.h"
#include "dms_define.h"
#include "urd_feature.h"
#include "dms_template.h"
#include "dms_event.h"
#include "devdrv_user_common.h"
#include "dms_event_distribute.h"
#include "dms_event_converge.h"
#include "dms_event_distribute_proc.h"
#include "smf_event_adapt.h"
#include "urd_acc_ctrl.h"
#include "dbl/uda.h"
#include "dms_cmd_def.h"
#include "dms_notifier.h"
#include "kernel_version_adapt.h"
#include "drv_systime.h"
#ifdef CFG_SOC_PLATFORM_MDC_V51
#include "kthread_affinity.h"
#endif

#include "devdrv_common.h"

#define DMS_EVENT_KFIFO_CELL (sizeof(struct dms_event_obj))
#define DMS_EVENT_KFIFO_SIZE (DMS_EVENT_KFIFO_CELL * DMS_MAX_EVENT_NUM)
#define DMS_EVENT_NUM_KERNEL ((DMS_MAX_EVENT_NUM * 80) / 100)

#define DMS_EVENT_POLL_WAIT_TIME 500
#define DMS_EVENT_TASK_WAIT_TIMEOUT_MS 200
#define DMS_EVENT_FRESH_TO_BAR 500

enum dms_event_poll_flag {
    EVENT_POLL_EXIT = 0,
    EVENT_POLL_WORK,
    EVENT_POLL_STOP,
    EVENT_POLL_MAX
};

struct {
    struct task_struct *poll_task;
    atomic_t poll_flag;

    struct kfifo kfifo;
    u32 event_num;
    struct mutex lock;
    wait_queue_head_t wait;
} g_event_task;
int dms_event_get_fault_event(void *feature, char *in, u32 in_len,
    char *out, u32 out_len);
int dms_event_get_history_fault_event(void *feature, char *in, u32 in_len,
    char *out, u32 out_len);
int dms_get_error_code(void *feature, char *in, u32 in_len,
    char *out, u32 out_len);
int dms_get_device_health(void *feature, char *in, u32 in_len,
    char *out, u32 out_len);
int dms_query_error_str(void *feature, char *in, u32 in_len,
    char *out, u32 out_len);
int dms_event_clear_fault_event(void *feature, char *in, u32 in_len,
    char *out, u32 out_len);
int dms_event_disable_fault_event(void *feature, char *in, u32 in_len,
    char *out, u32 out_len);
int dms_event_enable_fault_event(void *feature, char *in, u32 in_len,
    char *out, u32 out_len);

BEGIN_DMS_MODULE_DECLARATION(DMS_EVENT_CMD_NAME)
BEGIN_FEATURE_COMMAND()
ADD_FEATURE_COMMAND(DMS_EVENT_CMD_NAME, DMS_MAIN_CMD_BASIC, DMS_SUBCMD_GET_FAULT_EVENT, NULL, NULL,
                    DMS_ACC_ALL | DMS_ENV_ALL | DMS_VDEV_PHYSICAL, dms_event_get_fault_event)
ADD_FEATURE_COMMAND(DMS_EVENT_CMD_NAME, DMS_MAIN_CMD_BASIC, DMS_SUBCMD_GET_HISTORY_FAULT_EVENT, NULL, NULL,
                    DMS_ACC_ALL | DMS_ENV_NOT_DOCKER | DMS_VDEV_PHYSICAL, dms_event_get_history_fault_event)
ADD_FEATURE_COMMAND(DMS_EVENT_CMD_NAME, DMS_MAIN_CMD_BASIC, DMS_SUBCMD_GET_EVENT_CODE, NULL, NULL,
                    DMS_ACC_ALL | DMS_ENV_ALL | DMS_VDEV_ALL, dms_get_error_code)
ADD_FEATURE_COMMAND(DMS_EVENT_CMD_NAME, DMS_MAIN_CMD_BASIC, DMS_SUBCMD_GET_HEALTH_CODE, NULL, NULL,
                    DMS_ACC_ALL | DMS_ENV_ALL | DMS_VDEV_ALL, dms_get_device_health)
ADD_FEATURE_COMMAND(DMS_EVENT_CMD_NAME, DMS_MAIN_CMD_BASIC, DMS_SUBCMD_QUERY_STR, NULL, NULL,
                    DMS_ACC_ALL | DMS_ENV_ALL | DMS_VDEV_ALL, dms_query_error_str)
ADD_FEATURE_COMMAND(DMS_EVENT_CMD_NAME, DMS_MAIN_CMD_BASIC, DMS_SUBCMD_CLEAR_EVENT, NULL, NULL,
                    DMS_ACC_ROOT_ONLY | DMS_ENV_NOT_DOCKER | DMS_VDEV_PHYSICAL, dms_event_clear_fault_event)
ADD_FEATURE_COMMAND(DMS_EVENT_CMD_NAME, DMS_MAIN_CMD_BASIC, DMS_SUBCMD_DISABLE_EVENT, NULL, NULL,
                    DMS_ACC_ROOT_ONLY | DMS_ENV_NOT_DOCKER | DMS_VDEV_PHYSICAL, dms_event_disable_fault_event)
ADD_FEATURE_COMMAND(DMS_EVENT_CMD_NAME, DMS_MAIN_CMD_BASIC, DMS_SUBCMD_ENABLE_EVENT, NULL, NULL,
                    DMS_ACC_ROOT_ONLY | DMS_ENV_NOT_DOCKER | DMS_VDEV_PHYSICAL, dms_event_enable_fault_event)
END_FEATURE_COMMAND()
END_MODULE_DECLARATION()

static int dms_event_add_recv_to_dfx(struct dms_event_obj *event_obj)
{
    struct dms_event_dfx_table *dfx_table = NULL;
    struct dms_dev_ctrl_block *dev_cb = NULL;
    unsigned char assertion;

    dev_cb = dms_get_dev_cb(event_obj->deviceid);
    if (dev_cb == NULL) {
        dms_err("Get dev ctrl block failed. (dev_id=%u)\n", event_obj->deviceid);
        return DRV_ERROR_NO_DEVICE;
    }

    dfx_table = &dev_cb->dev_event_cb.dfx_table;
    assertion = event_obj->event.sensor_event.assertion;
    if (assertion < DMS_EVENT_TYPE_MAX) {
        atomic_inc(&dfx_table->recv_from_sensor[assertion]);
        return DRV_ERROR_NONE;
    }

    dms_err("Invalid parameter. (assertion=%u)\n", assertion);
    return DRV_ERROR_PARA_ERROR;
}

int dms_event_report(struct dms_event_obj *event_obj)
{
    struct dms_event_obj obj_buf;
    u32 event_num = kfifo_len(&g_event_task.kfifo) / DMS_EVENT_KFIFO_CELL;

    if ((event_obj == NULL) || dms_event_add_recv_to_dfx(event_obj)) {
        dms_err("The pointer of event_obj is null or add to dfx failed. (event_obj='%s')\n",
                event_obj == NULL ? "NULL" : "OK");
        return DRV_ERROR_PARA_ERROR;
    }

    if ((DMS_EVENT_OBJ_TYPE(event_obj->node_type) != DMS_EVENT_OBJ_KERNEL) && (event_num >= DMS_EVENT_NUM_KERNEL)) {
        dms_event("Dms event kfifo is over 80 percent, drop user event. (dev_id=%u; node_type=0x%X; node_id=0x%X; \
            sensor_type=0x%X; event_state=%u; assertion=%u; alarm_serial_num=%u; event_num=%u)\n",
            event_obj->deviceid, event_obj->node_type, event_obj->node_id,
            event_obj->event.sensor_event.sensor_type, event_obj->event.sensor_event.event_state,
            event_obj->event.sensor_event.assertion, event_obj->alarm_serial_num, event_num);
        return DRV_ERROR_NONE;
    }

    mutex_lock(&g_event_task.lock);
    while (kfifo_avail(&g_event_task.kfifo) < DMS_EVENT_KFIFO_CELL) {
        if (!kfifo_out(&g_event_task.kfifo, &obj_buf, DMS_EVENT_KFIFO_CELL)) {
            dms_warn("kfifo_out failed.\n");
        }
        dms_event("Dms event is covered. (dev_id=%u; node_type=0x%X; node_id=0x%X; "
                  "sensor_type=0x%X; event_state=%u; assertion=%u; alarm_serial_num=%u)\n",
                  obj_buf.deviceid, obj_buf.node_type, obj_buf.node_id,
                  obj_buf.event.sensor_event.sensor_type, obj_buf.event.sensor_event.event_state,
                  obj_buf.event.sensor_event.assertion, obj_buf.alarm_serial_num);
    }

    kfifo_in(&g_event_task.kfifo, event_obj, DMS_EVENT_KFIFO_CELL);
    g_event_task.event_num = kfifo_len(&g_event_task.kfifo) / DMS_EVENT_KFIFO_CELL;
    mutex_unlock(&g_event_task.lock);

    wake_up(&g_event_task.wait);
    return DRV_ERROR_NONE;
}
#ifndef DMS_SHARED_NO_EXPORT
EXPORT_SYMBOL(dms_event_report);
#endif

static inline unsigned char dms_event_convert_assertion(const struct dms_sensor_object_cb *p_sensor_obj_cb,
    unsigned char event_state)
{
    const struct dms_sensor_object_cfg *obj_cfg = &p_sensor_obj_cb->sensor_object_cfg;

    if ((obj_cfg->assert_event_mask & (1 << event_state)) &&
        !(obj_cfg->deassert_event_mask & (1 << event_state))) {
        return DMS_EVENT_TYPE_ONE_TIME;
    }

    return DMS_EVENT_TYPE_OCCUR;
}

STATIC int dms_event_get_events_by_sensor(const struct dms_sensor_object_cb *psensor_obj_cb,
    struct dms_event_para *fault_event_buf, int max_event_num, int *curr_event_num)
{
    DMS_EVENT_LIST_ITEM *event_item = psensor_obj_cb->p_event_list;
    DMS_EVENT_NODE_STRU  exception_node;
    struct dms_event_obj sensor_event;
    unsigned char assertion = 0;
    int event_index = (*curr_event_num);
    int ret;

    while (event_item != NULL) {
        if (event_index >= max_event_num) {
            dms_info("has get(%d) fault event number. not need to get continue.\n", max_event_num);
            return 0;
        }

        /* get dms_event_para data from sensor event item */
        assertion = dms_event_convert_assertion(psensor_obj_cb, event_item->event_data);
        ret = dms_fill_event_data(psensor_obj_cb, event_item, assertion, &sensor_event);
        if (ret != DRV_ERROR_NONE) {
            dms_err("dms_fill_event_data failed, ret = %d.\n", ret);
            return ret;
        }
        ret = dms_event_obj_to_exception(&sensor_event, &exception_node);
        if (ret != DRV_ERROR_NONE) {
            dms_err("dms_event_obj_to_exception failed, ret = %d.\n", ret);
            return ret;
        }

        /* copy the current exception node to fault_event_buf */
        ret = memcpy_s((void *)&fault_event_buf[event_index], sizeof(struct dms_event_para),
            (void *)&exception_node.event, sizeof(struct dms_event_para));
        if (ret) {
            dms_err("Call memcpy_s failed. (ret=%d)\n", ret);
            return ret;
        }

        *curr_event_num = (++event_index);

        /* fetch the next event */
        event_item = event_item->p_next;
    }

    return 0;
}

STATIC int dms_event_get_events_by_node(const struct dms_node_sensor_cb *pnode_sensor_cb,
    struct dms_event_para *fault_event_buf, int max_event_num, int *curr_event_num)
{
    struct dms_sensor_object_cb *tmp_sensor_ctl = NULL;
    struct dms_sensor_object_cb *psensor_obj_cb = NULL;
    int ret;

    list_for_each_entry_safe(psensor_obj_cb, tmp_sensor_ctl, &(pnode_sensor_cb->sensor_object_table), list) {
        if ((psensor_obj_cb == NULL) || (psensor_obj_cb->p_event_list == NULL)) {
            continue;
        }

        dms_debug("get event by sensor. (node_type=0x%x, node_id=%u, sensor type=0x%x)\n",
            pnode_sensor_cb->node_type, pnode_sensor_cb->node_id,
            psensor_obj_cb->sensor_object_cfg.sensor_type);
        ret = dms_event_get_events_by_sensor(psensor_obj_cb, fault_event_buf, max_event_num, curr_event_num);
        if (ret) {
            dms_err("get event by sensor failed. (node_type=0x%x, node_id=%u, sensor=0x%x, ret=%d)\n",
                pnode_sensor_cb->node_type, pnode_sensor_cb->node_id,
                psensor_obj_cb->sensor_object_cfg.sensor_type, ret);
            return ret;
        }
    }

    return 0;
}

STATIC int dms_event_get_history_fault_events(int dev_id, struct dms_event_para *fault_event_buf, int max_event_num)
{
    struct dms_dev_ctrl_block *dev_cb;
    struct dms_dev_sensor_cb *dev_sensor_cb;
    struct dms_node_sensor_cb *pnode_sensor_cb = NULL;
    struct dms_node_sensor_cb *tmp_ctl = NULL;
    int  event_count = 0;
    int  ret = DRV_ERROR_NONE;

    if (dev_id != 0) {
        dms_err("only support dev_id  = 0, but now is %d .\n", dev_id);
        return DRV_ERROR_PARA_ERROR;
    }
    dev_cb = dms_get_dev_cb(dev_id);
    if (dev_cb  == NULL) {
        dms_err("dms_get_dev_cb(devid=%d) failed.\n", dev_id);
        return DRV_ERROR_INNER_ERR;
    }

    dev_sensor_cb = &dev_cb->dev_sensor_cb;
    mutex_lock(&dev_sensor_cb->dms_sensor_mutex);
    list_for_each_entry_safe(pnode_sensor_cb, tmp_ctl, &(dev_sensor_cb->dms_node_sensor_cb_list), list) {
        if (event_count >= max_event_num) {
            dms_info("has get(%d) fault event number. not need to get continue.\n", max_event_num);
            goto out;
        }

        /* get all the events from current dms node */
        dms_debug("get events from one dms node. (node_type=0x%x, node id=%u)\n",
            pnode_sensor_cb->node_type, pnode_sensor_cb->node_id);
        ret = dms_event_get_events_by_node(pnode_sensor_cb, fault_event_buf, max_event_num, &event_count);
        if (ret != 0) {
            dms_err("get events from one dms node failed. (node_type=0x%x, node id=%u, ret=%d)\n",
                pnode_sensor_cb->node_type, pnode_sensor_cb->node_id, ret);
            goto out;
        }
    }

    if (event_count < max_event_num) {
        fault_event_buf[event_count].event_id = 0xFFFFFFFF;  // set event is invalid flag
        dms_debug("The number of %d fault event is invaliad.\n", event_count);
    }
    dms_info("There is %d fault events.\n", event_count);

out:
    mutex_unlock(&dev_sensor_cb->dms_sensor_mutex);
    return ret;
}

#define MAX_EVENT_COUNT_OF_GET_FAULT_EVENT (1024)
#define MIN_EVENT_COUNT_OF_GET_FAULT_EVENT (1)
int dms_event_get_history_fault_event(void *feature, char *in, u32 in_len,
    char *out, u32 out_len)
{
    struct dms_event_para *event_para_buf;
    int event_buf_size = 0;
    int dev_id = 0;
    int ret;

    if ((in == NULL) || (out == NULL) || (in_len != sizeof(int))) {
        dms_err("Invalid parameter. (in_buff=%s; out_buff=%s; in_len=%u)\n",
            (in == NULL) ? "NULL" : "OK", (out == NULL) ? "NULL" : "OK", in_len);
        return DRV_ERROR_PARA_ERROR;
    }

    ret = memcpy_s((void *)&dev_id, sizeof(int), (void *)in, in_len);
    if (ret) {
        dms_err("Call memcpy_s failed. (ret=%d)\n", ret);
        return DRV_ERROR_INNER_ERR;
    }
    event_buf_size = out_len / sizeof(struct dms_event_para);
    if  (event_buf_size < MIN_EVENT_COUNT_OF_GET_FAULT_EVENT  ||
         event_buf_size > MAX_EVENT_COUNT_OF_GET_FAULT_EVENT) {
        dms_err("history_max_event_size(%d)  is invalid.\n", event_buf_size);
        return DRV_ERROR_PARA_ERROR;
    }

    if  (event_buf_size * sizeof(struct dms_event_para)  != out_len)  {
        dms_err("out_len(%d)  is not equal to expect(%d).\n", out_len,
            (int)(event_buf_size * sizeof(struct dms_event_para)));
        return DRV_ERROR_PARA_ERROR;
    }

    dms_info("history_max_event_size is %d.\n", event_buf_size);
    event_para_buf  = (struct dms_event_para*)out;
    ret = dms_event_get_history_fault_events(dev_id, event_para_buf, event_buf_size);
    if (ret != DRV_ERROR_NONE) {
        return DRV_ERROR_INNER_ERR;
    }
    return  DRV_ERROR_NONE;
}

int dms_event_get_fault_event(void *feature, char *in, u32 in_len,
    char *out, u32 out_len)
{
    struct dms_event_para fault_event = {0};
    u64 start_syscnt;
    int timeout = 0;
    int ret;
    if ((in == NULL) || (out == NULL) || (in_len != sizeof(int)) || (out_len != sizeof(struct dms_event_para))) {
        dms_err("Invalid parameter. (in_buff=%s; in_len=%u;out_buff=%s; out_len=%u)\n",
                (in == NULL) ? "NULL" : "OK", in_len, (out == NULL) ? "NULL" : "OK", out_len);
        return DRV_ERROR_PARA_ERROR;
    }
    start_syscnt = get_syscnt();
    ret = memcpy_s((void *)&timeout, sizeof(int), (void *)in, in_len);
    if (ret) {
        dms_err("Call memcpy_s failed. (ret=%d start_syscnt=%llu)\n", ret, start_syscnt);
        return DRV_ERROR_INNER_ERR;
    }

    ret = dms_event_get_exception(&fault_event, timeout);
    if (ret == -ETIMEDOUT) {
        return ret;
    } else if (ret == -ERESTARTSYS) {
        dms_warn("Get exception is interrupted. (timeout=%d; ret=%d)\n", timeout, ret);
        return ret;
    } else if (ret) {
        dms_err("Get exception failed. (timeout=%d; ret=%d)\n", timeout, ret);
        return ret;
    }

    ret = memcpy_s((void *)out, out_len, (void *)&fault_event, sizeof(struct dms_event_para));
    if (ret) {
        dms_err("Call memcpy_s failed. (ret=%d)\n", ret);
        return DRV_ERROR_INNER_ERR;
    }
    dms_warn("read fault event success.(start=%llu, end=%llu)\n", start_syscnt, get_syscnt());

    return DRV_ERROR_NONE;
}

int dms_event_clear_by_phyid(u32 phyid)
{
    struct dms_dev_ctrl_block *dev_cb = NULL;
    int ret;

    dev_cb = dms_get_dev_cb(phyid);
    if (dev_cb == NULL) {
        dms_err("Get dev_ctrl block failed. (phyid=%u)\n", phyid);
        return DRV_ERROR_NO_DEVICE;
    }

    ret = dms_event_clear_exception(phyid);
    if (ret) {
        dms_err("Clear exception failed. (phyid=%u; ret=%d)\n", phyid, ret);
        return ret;
    }

    if (dms_event_is_converge()) {
        ret = dms_event_convergent_diagrams_clear(phyid, false);
        if (ret) {
            dms_err("Clear convergent diagrams failed. (phyid=%u; ret=%d)\n", phyid, ret);
            return ret;
        }
    }

    ret = dms_sensor_clean_health_events(&dev_cb->dev_sensor_cb);
    if (ret) {
        dms_err("Clean health event failed. (phyid=%u; ret=%d)\n", phyid, ret);
        return ret;
    }

    if (smf_event_distribute_to_bar(phyid)) {
        dms_err("Distribute event to bar failed. (dev_id=%u)\n", phyid);
        return DRV_ERROR_INNER_ERR;
    }

    dms_event("Clear event success. (phyid=%u)\n", phyid);
    return DRV_ERROR_NONE;
}
#ifndef DMS_SHARED_NO_EXPORT
EXPORT_SYMBOL(dms_event_clear_by_phyid);
#endif

int dms_event_get_logic_id_and_covert_to_phy_id(char *in, u32 in_len, u32 *logic_id, u32 *phy_id, u32 *fid)
{
    int ret;

    ret = memcpy_s((void *)logic_id, sizeof(u32), (void *)in, in_len);
    if (ret) {
        dms_err("Call memcpy_s failed. (ret=%d)\n", ret);
        return ret;
    }
#ifndef AOS_LLVM_BUILD
    ret = smf_logical_id_to_physical_id(*logic_id, phy_id, fid);
    if (ret || (*fid >= VMNG_VDEV_MAX_PER_PDEV)) {
        dms_err("Transform logical id to physical id failed. "
                "(devid=%u; phyid=%u; fid=%u; ret=%d)\n",
                *logic_id, *phy_id, *fid, ret);
        return DRV_ERROR_NO_DEVICE;
    }

#else
    *phy_id = *logic_id;
#endif
    return DRV_ERROR_NONE;
}

int dms_event_clear_fault_event(void *feature, char *in, u32 in_len,
    char *out, u32 out_len)
{
    u32 devid = 0;
    u32 phyid = 0;
    u32 fid = 0;
    int ret;
    if ((in == NULL) || (in_len != sizeof(u32))) {
        dms_err("Invalid parameter. (in_buff=%s; in_len=%u)\n", (in == NULL) ? "NULL" : "OK", in_len);
        return DRV_ERROR_PARA_ERROR;
    }
    ret = dms_event_get_logic_id_and_covert_to_phy_id(in, in_len, &devid, &phyid, &fid);
    if (ret != 0) {
        dms_err("Get logical id or convert to physical id failed. (ret=%d)\n", ret);
        return ret;
    }

    /* clean event to device when on host side */
    ret = smf_event_clean_to_device(phyid);
    if (ret) {
        dms_err("Clean event to device failed. (phyid=%u)\n", phyid);
        return ret;
    }

    ret = dms_event_clear_by_phyid(phyid);
    if (ret) {
        dms_err("Clear exception failed. (phyid=%u; ret=%d)\n", phyid, ret);
        return ret;
    }

    return DRV_ERROR_NONE;
}

int dms_event_mask_by_phyid(u32 phyid, u32 event_id, u8 mask)
{
    struct dms_dev_ctrl_block *dev_cb = NULL;
    u32 node_type, sensor_type, event_state;
    int ret;

    if (dms_event_code_validity_check(event_id)) {
        return DRV_ERROR_NO_EVENT;
    }

    dev_cb = dms_get_dev_cb(phyid);
    if (dev_cb == NULL) {
        dms_err("Get dev_ctrl block failed. (phyid=%u)\n", phyid);
        return DRV_ERROR_NO_DEVICE;
    }

    node_type = DMS_EVENT_ID_TO_NODE_TYPE(event_id);
    sensor_type = DMS_EVENT_ID_TO_SENSOR_TYPE(event_id);
    event_state = DMS_EVENT_ID_TO_EVENT_STATE(event_id);
    ret = dms_sensor_mask_events(&dev_cb->dev_sensor_cb, mask, (u16)node_type, (u8)sensor_type, (u8)event_state);
    if (ret) {
        dms_err("Mask event code failed. (phyid=%u; event_id=0x%x; mask=%u; ret=%d)\n",
                phyid, event_id, mask, ret);
        return ret;
    }

    if (dms_event_is_converge()) {
        if (mask == EVENT_CONVERGE_NODE_DISENABLE) {
            dms_event_mask_del_to_event_cb(phyid, dev_cb, event_id);
            dms_event_add_to_mask_list(dev_cb, event_id);
        } else if (mask == EVENT_CONVERGE_NODE_ENABLE) {
            dms_event_mask_add_to_event_cb(phyid, dev_cb, event_id);
            dms_event_del_to_mask_list(dev_cb, event_id);
        }
        dms_event_convergent_diagrams_mask(phyid, event_id, mask);
    }

    if (smf_event_distribute_to_bar(phyid)) {
        dms_err("Distribute event to bar failed. (dev_id=%u)\n", phyid);
        return DRV_ERROR_INNER_ERR;
    }

    dms_event("Mask event_id success. (phyid=%u; event_id=0x%X; mask=%u; ret=%d)\n",
              phyid, event_id, mask, ret);
    return DRV_ERROR_NONE;
}
#ifndef DMS_SHARED_NO_EXPORT
EXPORT_SYMBOL(dms_event_mask_by_phyid);
#endif

int dms_event_disable_fault_event(void *feature, char *in, u32 in_len,
    char *out, u32 out_len)
{
    struct dms_event_ioctrl para = {0};
    u32 locid, phyid, fid;
    int ret;

    if ((in == NULL) || (in_len != sizeof(struct dms_event_ioctrl))) {
        dms_err("Invalid parameter. (in_buff=%s; in_len=%u)\n", (in == NULL) ? "NULL" : "OK", in_len);
        return DRV_ERROR_PARA_ERROR;
    }
    ret = memcpy_s((void *)&para, sizeof(struct dms_event_ioctrl), (void *)in, in_len);
    if (ret) {
        dms_err("Disable memcpy_s failed. (ret=%d)\n", ret);
        return ret;
    }

    ret = dms_event_get_logic_id_and_covert_to_phy_id((char *)&para.devid,
                                                      sizeof(para.devid), &locid, &phyid, &fid);
    if (ret != 0) {
        dms_err("Disable logical id or convert to physical id failed. (ret=%d)\n", ret);
        return ret;
    }

    ret = smf_event_mask_event_code(phyid, para.event_code, EVENT_CONVERGE_NODE_DISENABLE);
    if (ret) {
        dms_err("Disable event code failed. (phyid=%u; event_code=0x%x; ret=%d)\n",
                phyid, para.event_code, ret);
        return ret;
    }

    return DRV_ERROR_NONE;
}

int dms_event_enable_fault_event(void *feature, char *in, u32 in_len,
    char *out, u32 out_len)
{
    struct dms_event_ioctrl para = {0};
    u32 locid, phyid, fid;
    int ret;

    if ((in == NULL) || (in_len != sizeof(struct dms_event_ioctrl))) {
        dms_err("Invalid parameter. (in_buff=%s; in_len=%u)\n", (in == NULL) ? "NULL" : "OK", in_len);
        return DRV_ERROR_PARA_ERROR;
    }
    ret = memcpy_s((void *)&para, sizeof(struct dms_event_ioctrl), (void *)in, in_len);
    if (ret) {
        dms_err("Enable memcpy_s failed. (ret=%d)\n", ret);
        return ret;
    }

    ret = dms_event_get_logic_id_and_covert_to_phy_id((char *)&para.devid,
                                                      sizeof(para.devid), &locid, &phyid, &fid);
    if (ret != 0) {
        dms_err("Enable logical id or convert to physical id failed. (ret=%d)\n", ret);
        return ret;
    }

    ret = smf_event_mask_event_code(phyid, para.event_code, EVENT_CONVERGE_NODE_ENABLE);
    if (ret) {
        dms_err("Enable event code failed. (phyid=%u; event_code=0x%x; ret=%d)\n",
                phyid, para.event_code, ret);
        return ret;
    }

    return DRV_ERROR_NONE;
}

int dms_get_code_from_sensor_check(u32 devid, u32 *health_code, u32 health_len,
    struct shm_event_code *event_code, u32 event_len)
{
    if ((devid >= DEVICE_NUM_MAX) || (health_code == NULL) ||
        (health_len != VMNG_VDEV_MAX_PER_PDEV) || (event_code == NULL) ||
        (event_len != DEVMNG_SHM_INFO_EVENT_CODE_LEN)) {
        dms_err("Invalid parameter. (devid=%u; health_code=\"%s\"; health_len=%u; "
                "event_code=\"%s\"; event_len=%u;)\n", devid, (health_code == NULL) ? "NULL" : "OK",
                health_len, (event_code == NULL) ? "NULL" : "OK", event_len);
        return DRV_ERROR_PARA_ERROR;
    }

    (void)memset_s(event_code, sizeof(struct shm_event_code) * event_len,
                   0, sizeof(struct shm_event_code) * event_len);
    (void)memset_s(health_code, sizeof(u32) * health_len, 0, sizeof(u32) * health_len);

    return DRV_ERROR_NONE;
}

static int dms_event_obj_to_health_code(struct dms_event_obj *event_buff,
    u32 event_cnt, u32 *health_code, struct shm_event_code *event_code)
{
    u32 fid_buf = 0;
    u32 i, severity;

    if (event_cnt > DEVMNG_SHM_INFO_EVENT_CODE_LEN) {
        dms_err("The number of events is invalid. (cnt=%u)\n", event_cnt);
        return DRV_ERROR_INNER_ERR;
    }

    for (i = 0; i < event_cnt; i++) {
        /* need virtmng_get_vfid_by_sensor_info(event_buff, &fid_buf) */
        if (fid_buf >= VMNG_VDEV_MAX_PER_PDEV) {
            dms_err("The fid is invalid. (fid=%u)\n", fid_buf);
            continue;
        }
        (void)dms_event_obj_to_error_code(event_buff[i], &event_code[i].event_code);
        severity = event_buff[i].severity;
        health_code[fid_buf] = (severity > health_code[fid_buf]) ? severity : health_code[fid_buf];
        event_code[i].event_code = DMS_EVENT_OBJ_NODETYPE_TO_CODE(event_buff[i].node_type) |
            DMS_EVENT_CODE_CLEAR_SERVERITY_ASSERTION(event_code[i].event_code);
        event_code[i].fid = (u8)fid_buf;
    }
    for (i = 0; i < VMNG_VDEV_MAX_PER_PDEV; i++) {
        health_code[0] = (health_code[i] > health_code[0]) ? health_code[i] : health_code[0];
    }

    return DRV_ERROR_NONE;
}

int dms_get_event_code_from_sensor(u32 devid, u32 *health_code, u32 health_len,
    struct shm_event_code *event_code, u32 event_len)
{
    struct dms_dev_ctrl_block *dev_cb = NULL;
    struct dms_event_obj *event_buff = NULL;
    u32 out_cnt = 0;
    int ret;

    if (dms_get_code_from_sensor_check(devid, health_code, health_len, event_code, event_len)) {
        dms_err("Invalid parameter.\n");
        return DRV_ERROR_PARA_ERROR;
    }

    dev_cb = dms_get_dev_cb(devid);
    if (dev_cb == NULL) {
        dms_err("Get dev_ctrl block failed. (dev_id=%u)\n", devid);
        return DRV_ERROR_NO_DEVICE;
    }

    event_buff = (struct dms_event_obj *)kzalloc(sizeof(struct dms_event_obj) *
                                                 DMS_EVENT_ERROR_ARRAY_NUM, GFP_KERNEL | __GFP_ACCOUNT);
    if (event_buff == NULL) {
        dms_err("Call kzalloc failed. (dev_id=%u)\n", devid);
        return DRV_ERROR_OUT_OF_MEMORY;
    }

    ret = dms_sensor_get_health_events(&dev_cb->dev_sensor_cb, event_buff, DMS_EVENT_ERROR_ARRAY_NUM, &out_cnt);
    if (ret) {
        dms_err("Get health events failed. (dev_id=%u; ret=%d)\n", devid, ret);
        goto free_buff;
    }

    ret = dms_event_obj_to_health_code(event_buff, out_cnt, health_code, event_code);
    if (ret) {
        dms_err("Transform event_obj to health code failed. (dev_id=%u; ret=%d)\n",
                devid, ret);
        goto free_buff;
    }

free_buff:
    kfree(event_buff);
    event_buff = NULL;
    return ret;
}
#ifndef DMS_SHARED_NO_EXPORT
EXPORT_SYMBOL(dms_get_event_code_from_sensor);
#endif

static int dms_get_event_code_para_check(u32 devid, u32 fid, u32 *health_code,
    struct devdrv_error_code_para *code_para)
{
    if ((devid >= DEVICE_NUM_MAX) || (fid >= VMNG_VDEV_MAX_PER_PDEV) ||
        (health_code == NULL) || (code_para == NULL)) {
        dms_err("Invalid parameter. (dev_id=%u; fid=%u; health_code=\"%s\"; code_para=\"%s\")\n",
                devid, fid, (health_code == NULL) ? "NULL" : "OK", (code_para == NULL) ? "NULL" : "OK");
        return DRV_ERROR_PARA_ERROR;
    }

    (void)memset_s(code_para, sizeof(struct devdrv_error_code_para),
                   0, sizeof(struct devdrv_error_code_para));
    return DRV_ERROR_NONE;
}

static void dms_event_combine_event_code(u32 fid, struct shm_event_code *sensor_event,
    struct shm_event_code *bar_event, struct devdrv_error_code_para *code_para)
{
    int sum_cnt = 0;
    int i;

    for (i = 0; (i < DEVMNG_SHM_INFO_EVENT_CODE_LEN) &&
        (sum_cnt < DMANAGE_ERROR_ARRAY_NUM); i++) {
        if (sensor_event[i].event_code == 0) {
            break;
        }
        if ((fid == 0) || (fid == sensor_event[i].fid)) {
            code_para->error_code[sum_cnt++] = sensor_event[i].event_code;
        }
    }

    for (i = 0; (i < DEVMNG_SHM_INFO_EVENT_CODE_LEN) &&
        (sum_cnt < DMANAGE_ERROR_ARRAY_NUM); i++) {
        if (bar_event[i].event_code == 0) {
            break;
        }
        if ((fid == 0) || (fid == bar_event[i].fid)) {
            code_para->error_code[sum_cnt++] = bar_event[i].event_code;
        }
    }
    code_para->error_code_count = sum_cnt;
}

int dms_get_event_code(u32 devid, u32 fid, u32 *health_code,
    struct devdrv_error_code_para *code_para)
{
    struct shm_event_code *sensor_event, *bar_event;
    u32 sensor_health[VMNG_VDEV_MAX_PER_PDEV] = {0};
    u32 bar_health[VMNG_VDEV_MAX_PER_PDEV] = {0};
    int ret;

    if (dms_get_event_code_para_check(devid, fid, health_code, code_para)) {
        dms_err("Invalid parameter.\n");
        return DRV_ERROR_PARA_ERROR;
    }

    sensor_event = (struct shm_event_code *)kzalloc(sizeof(struct shm_event_code) * \
                                                    DEVMNG_SHM_INFO_EVENT_CODE_LEN, GFP_KERNEL | __GFP_ACCOUNT);
    if (sensor_event == NULL) {
        dms_err("Call kzalloc sensor_event failed. (dev_id=%u)\n", devid);
        return DRV_ERROR_OUT_OF_MEMORY;
    }

    bar_event = (struct shm_event_code *)kzalloc(sizeof(struct shm_event_code) * \
                                                 DEVMNG_SHM_INFO_EVENT_CODE_LEN, GFP_KERNEL | __GFP_ACCOUNT);
    if (bar_event == NULL) {
        kfree(sensor_event);
        dms_err("Call kzalloc bar_event failed. (dev_id=%u)\n", devid);
        return DRV_ERROR_OUT_OF_MEMORY;
    }

    if (dms_event_is_converge()) {
        ret = dms_get_event_code_from_event_cb(devid, sensor_health, VMNG_VDEV_MAX_PER_PDEV,
                                               sensor_event, DEVMNG_SHM_INFO_EVENT_CODE_LEN);
        if (ret) {
            dms_err("Get event code from event cb failed. (dev_id=%u; ret=%d)\n", devid, ret);
            goto out;
        }
    } else {
        ret = dms_get_event_code_from_sensor(devid, sensor_health, VMNG_VDEV_MAX_PER_PDEV,
                                             sensor_event, DEVMNG_SHM_INFO_EVENT_CODE_LEN);
        if (ret) {
            dms_err("Get event code from sensor manager failed. (dev_id=%u; ret=%d)\n", devid, ret);
            goto out;
        }
    }

    ret = smf_get_event_code_from_bar(devid, bar_health, VMNG_VDEV_MAX_PER_PDEV,
                                      bar_event, DEVMNG_SHM_INFO_EVENT_CODE_LEN);
    if (ret) {
        dms_err("Get event code from bar zone failed. (dev_id=%u; ret=%d)\n", devid, ret);
        goto out;
    }

    dms_event_combine_event_code(fid, sensor_event, bar_event, code_para);
    *health_code = (sensor_health[fid] > bar_health[fid]) ? sensor_health[fid] : bar_health[fid];

out:
    kfree(sensor_event);
    kfree(bar_event);
    return ret;
}

int dms_get_error_code(void *feature, char *in, u32 in_len,
    char *out, u32 out_len)
{
    int ret;
    u32 devid = 0;
    u32 phyid = 0;
    u32 fid = 0;
    u32 health_code = 0;
    struct devdrv_error_code_para code_para = {0};

    if ((in == NULL) || (out == NULL) || (in_len != sizeof(u32)) ||
        (out_len != sizeof(struct devdrv_error_code_para))) {
        dms_err("Invalid parameter. (in_buff=%s; in_len=%u;out_buff=%s; out_len=%u)\n",
                (in == NULL) ? "NULL" : "OK", in_len, (out == NULL) ? "NULL" : "OK", out_len);
        return DRV_ERROR_PARA_ERROR;
    }
    ret = dms_event_get_logic_id_and_covert_to_phy_id(in, in_len, &devid, &phyid, &fid);
    if (ret != 0) {
        dms_err("Get logical id or convert to physical id failed. (ret=%d)\n", ret);
        return ret;
    }

    fid = 0;
    ret = dms_get_event_code(phyid, fid, &health_code, &code_para);
    if (ret) {
        dms_err("Get event code failed. (phyid=%u; fid=%u; ret=%d)\n", phyid, fid, ret);
        return ret;
    }

    ret = memcpy_s((void *)out, out_len, (void *)&code_para, sizeof(struct devdrv_error_code_para));
    if (ret) {
        dms_err("Call memcpy_s failed. (ret=%d)\n", ret);
        return ret;
    }

    return DRV_ERROR_NONE;
}

int dms_get_health_code(u32 phyid, u32 fid, u32 *health_code)
{
    u32 bar_health[VMNG_VDEV_MAX_PER_PDEV] = {0};
    struct dms_dev_ctrl_block *dev_cb = NULL;
    int ret;

    ret = smf_get_health_code_from_bar(phyid, bar_health, VMNG_VDEV_MAX_PER_PDEV);
    if (ret) {
        dms_err("Get health code failed. (phyid=%u; fid=%u; ret=%d)\n", phyid, fid, ret);
        return ret;
    }

    dev_cb = dms_get_dev_cb(phyid);
    if (dev_cb == NULL) {
        dms_err("Get dev_ctrl block failed. (phyid=%u)\n", phyid);
        return DRV_ERROR_NO_DEVICE;
    }

    if (dms_event_is_converge()) {
        *health_code = dev_cb->dev_event_cb.event_list.health_code;
    } else {
        *health_code = dms_sensor_get_dev_health(&dev_cb->dev_sensor_cb);
    }
    *health_code = (bar_health[fid] > *health_code) ? bar_health[fid] : *health_code;
    return DRV_ERROR_NONE;
}

int dms_get_device_health(void *feature, char *in, u32 in_len,
    char *out, u32 out_len)
{
    int ret;
    u32 devid = 0;
    u32 phyid = 0;
    u32 fid = 0;
    u32 health_code = 0;

    if ((in == NULL) || (out == NULL) || (in_len != sizeof(u32)) || (out_len != sizeof(u32))) {
        dms_err("Invalid parameter. (in_buff=%s; in_len=%u;out_buff=%s; out_len=%u)\n",
                (in == NULL) ? "NULL" : "OK", in_len, (out == NULL) ? "NULL" : "OK", out_len);
        return DRV_ERROR_PARA_ERROR;
    }
    ret = dms_event_get_logic_id_and_covert_to_phy_id(in, in_len, &devid, &phyid, &fid);
    if (ret != 0) {
        dms_err("Get logical id or convert to physical id failed. (ret=%d)\n", ret);
        return ret;
    }

    fid = 0;
    ret = dms_get_health_code(phyid, fid, &health_code);
    if (ret) {
        dms_err("Get health code failed. (phyid=%u; fid=%u; ret=%d)\n", phyid, fid, ret);
        return ret;
    }

    ret = memcpy_s((void *)out, out_len, (void *)&health_code, sizeof(u32));
    if (ret) {
        dms_err("Call memcpy_s failed. (ret=%d)\n", ret);
        return ret;
    }

    return DRV_ERROR_NONE;
}

int dms_query_error_str(void *feature, char *in, u32 in_len,
    char *out, u32 out_len)
{
    struct dms_event_ioctrl para = {0};
    u32 locid, phyid, fid;
    int ret;

    if ((in == NULL) || (out == NULL) || (in_len != sizeof(struct dms_event_ioctrl))) {
        dms_err("Invalid parameter. (in_buff=%s; out_buff=%s; in_len=%u)\n",
            (in == NULL) ? "NULL" : "OK", (out == NULL) ? "NULL" : "OK", in_len);
        return DRV_ERROR_PARA_ERROR;
    }
    ret = memcpy_s((void *)&para, sizeof(struct dms_event_ioctrl), (void *)in, in_len);
    if (ret) {
        dms_err("Call memcpy_s failed. (ret=%d)\n", ret);
        return ret;
    }

    if (DMS_EVENT_ID_IS_BBOX_CODE(para.event_code)) {
        dms_debug("The event code is bbox code. (devi_id=%u; event_code=0x%x)\n", para.devid, para.event_code);
        return DRV_ERROR_NO_EVENT;
    }

    ret = dms_event_get_logic_id_and_covert_to_phy_id((char *)&para.devid,
                                                      sizeof(para.devid), &locid, &phyid, &fid);
    if (ret != 0) {
        dms_err("Get logical id or convert to physical id failed. (ret=%d)\n", ret);
        return ret;
    }

    ret = dms_event_id_to_error_string(phyid, para.event_code, out, out_len);
    if (ret) {
        dms_err("Transform event_id to event_name failed. "
                "(devid=%u; phyid=%u; event_id=%u; ret=%d)\n",
                para.devid, phyid, para.event_code, ret);
        return ret;
    }

    return DRV_ERROR_NONE;
}

static int dms_event_poll_event(struct dms_event_obj *event_obj, u32 wait_time, u32 *event_num)
{
    if (wait_event_timeout(g_event_task.wait, (g_event_task.event_num > 0), wait_time) == 0) {
        return ETIMEDOUT;
    }

    if (atomic_read(&g_event_task.poll_flag) != EVENT_POLL_WORK) {
        dms_info("Dms_event_poll_task will exit.\n");
        return DRV_ERROR_UNINIT;
    }

    mutex_lock(&g_event_task.lock);
    if (kfifo_len(&g_event_task.kfifo) < DMS_EVENT_KFIFO_CELL) {
        g_event_task.event_num = kfifo_len(&g_event_task.kfifo) / DMS_EVENT_KFIFO_CELL;
        mutex_unlock(&g_event_task.lock);
        dms_warn("There is no event in event fifo.\n");
        return DRV_ERROR_INNER_ERR;
    }

    if (!kfifo_out(&g_event_task.kfifo, event_obj, DMS_EVENT_KFIFO_CELL)) {
        g_event_task.event_num = kfifo_len(&g_event_task.kfifo) / DMS_EVENT_KFIFO_CELL;
        mutex_unlock(&g_event_task.lock);
        dms_err("kfifo_out failed.\n");
        return DRV_ERROR_INNER_ERR;
    }

    g_event_task.event_num = kfifo_len(&g_event_task.kfifo) / DMS_EVENT_KFIFO_CELL;
    *event_num = g_event_task.event_num;
    mutex_unlock(&g_event_task.lock);

    return DRV_ERROR_NONE;
}

static int dms_event_poll_task(void *arg)
{
    DMS_EVENT_NODE_STRU exception_node = {{0}};
    struct dms_event_obj event_obj = {0};
    u32 time_old = 0;
    u32 time_new = 0;
    u32 wait_time, event_num = 0;

    wait_time = msecs_to_jiffies(DMS_EVENT_TASK_WAIT_TIMEOUT_MS);
    dms_info("Dms vent poll task start.\n");
    while (!kthread_should_stop()) {
        time_new = jiffies_to_msecs(jiffies);
        /* When there is a 500ms interval or timing flip, refresh the bar space immediately */
        if ((time_new < time_old) || ((time_new - time_old) > DMS_EVENT_FRESH_TO_BAR)) {
            if (smf_distribute_all_devices_event_to_bar()) {
                dms_err("Distribute event to bar failed.\n");
            }
            time_old = time_new;
        }

        if (dms_event_poll_event(&event_obj, wait_time, &event_num)) {
            continue;
        }

        dms_debug("Dms poll event success. (event_num=%u; dev_id=%u; node_type=0x%X; node_id=0x%X; "
                  "sensor_type=0x%X; event_state=%u; assertion=%u; alarm_serial_num=%u)\n",
                  event_num, event_obj.deviceid, event_obj.node_type, event_obj.node_id,
                  event_obj.event.sensor_event.sensor_type, event_obj.event.sensor_event.event_state,
                  event_obj.event.sensor_event.assertion, event_obj.alarm_serial_num);

        if (dms_event_converge_to_exception(&event_obj, &exception_node)) {
            continue;
        }

        if (dms_event_converge_add_to_event_cb(&exception_node)) {
            continue;
        }

        if (dms_event_distribute_handle(&exception_node, DMS_DISTRIBUTE_PRIORITY0)) {
            dms_err("Distribute exception failed. (dev_id=%u; event_id=0x%X; assertion=%u)\n",
                    exception_node.event.deviceid, exception_node.event.event_id, exception_node.event.assertion);
        }
    }

    atomic_set(&g_event_task.poll_flag, EVENT_POLL_EXIT);

    dms_info("Dms event poll task exited.\n");
    return DRV_ERROR_NONE;
}

static int dms_event_task_init(void)
{
    init_waitqueue_head(&g_event_task.wait);
    mutex_init(&g_event_task.lock);
    mutex_lock(&g_event_task.lock);
    g_event_task.event_num = 0;
    if (kfifo_alloc(&g_event_task.kfifo, DMS_EVENT_KFIFO_SIZE, GFP_KERNEL)) {
        mutex_unlock(&g_event_task.lock);
        mutex_destroy(&g_event_task.lock);
        dms_err("kfifo_alloc failed.\n");
        return DRV_ERROR_INNER_ERR;
    }
    kfifo_reset(&g_event_task.kfifo);
    mutex_unlock(&g_event_task.lock);

    atomic_set(&g_event_task.poll_flag, EVENT_POLL_WORK);
    g_event_task.poll_task = kthread_create(dms_event_poll_task, (void *)NULL, "dms_poll_task");
    if (IS_ERR(g_event_task.poll_task)) {
        mutex_lock(&g_event_task.lock);
        kfifo_free(&g_event_task.kfifo);
        mutex_unlock(&g_event_task.lock);
        mutex_destroy(&g_event_task.lock);
        dms_err("kthread_create failed.\n");
        return DRV_ERROR_INNER_ERR;
    }

    /* Start the work task */
#ifdef CFG_SOC_PLATFORM_MDC_V51
    kthread_bind_to_ctrl_cpu(g_event_task.poll_task);
#endif
    (void)wake_up_process(g_event_task.poll_task);
    return DRV_ERROR_NONE;
}

static void dms_event_task_exit(void)
{
    int wait_time = 0;

    atomic_set(&g_event_task.poll_flag, EVENT_POLL_STOP);
    mutex_lock(&g_event_task.lock);
    g_event_task.event_num = DMS_MAX_EVENT_NUM;
    mutex_unlock(&g_event_task.lock);
    if (!IS_ERR(g_event_task.poll_task)) {
        kthread_stop(g_event_task.poll_task);
    }
    wake_up(&g_event_task.wait);
    while (atomic_read(&g_event_task.poll_flag) != EVENT_POLL_EXIT) {
        msleep(1);
        if (++wait_time >= DMS_EVENT_POLL_WAIT_TIME) {
            dms_err("Wait dms_event_poll_task exit timeout.\n");
            goto out;
        }
    }

    dms_info("Dms_event_poll_task exit success.\n");
out:
    mutex_lock(&g_event_task.lock);
    kfifo_free(&g_event_task.kfifo);
    g_event_task.event_num = 0;
    mutex_unlock(&g_event_task.lock);
    mutex_destroy(&g_event_task.lock);
    g_event_task.poll_task = NULL;
    return;
}

void dms_event_ctrl_converge_list_free(struct dms_converge_event_list *event_list)
{
    DMS_EVENT_NODE_STRU *exception_node = NULL;
    struct list_head *pos = NULL, *n = NULL;

    mutex_lock(&event_list->lock);
    event_list->event_num = 0;
    event_list->health_code = 0;
    if (!list_empty_careful(&event_list->head)) {
        list_for_each_safe(pos, n, &event_list->head) {
            exception_node = list_entry(pos, DMS_EVENT_NODE_STRU, node);
            list_del(&exception_node->node);
            kfree(exception_node);
            exception_node = NULL;
        }
    }

    INIT_LIST_HEAD(&event_list->head);
    mutex_unlock(&event_list->lock);
}

void dms_event_sensor_reported_list_free(struct dms_sensor_reported_list *reported_list)
{
    struct dms_event_sensor_reported *event_node = NULL;
    struct list_head *pos = NULL, *n = NULL;

    mutex_lock(&reported_list->lock);
    reported_list->reported_num = 0;
    if (!list_empty_careful(&reported_list->head)) {
        list_for_each_safe(pos, n, &reported_list->head) {
            event_node = list_entry(pos, struct dms_event_sensor_reported, node);
            list_del(&event_node->node);
            kfree(event_node);
            event_node = NULL;
        }
    }

    INIT_LIST_HEAD(&reported_list->head);
    mutex_unlock(&reported_list->lock);
}

static int dms_event_ctrl_init(void)
{
    struct dms_dev_ctrl_block *dev_cb = NULL;
    u32 i;

    for (i = 0; i < DEVICE_NUM_MAX; i++) {
        dev_cb = dms_get_dev_cb(i);
        if (dev_cb == NULL) {
            dms_err("Get dev_ctrl block failed. (dev_id=%u)\n", i);
            return DRV_ERROR_NO_DEVICE;
        }
        mutex_lock(&dev_cb->node_lock);
        mutex_init(&dev_cb->dev_event_cb.event_list.lock);
        INIT_LIST_HEAD(&dev_cb->dev_event_cb.event_list.head);
        mutex_init(&dev_cb->dev_event_cb.reported_list.lock);
        INIT_LIST_HEAD(&dev_cb->dev_event_cb.reported_list.head);
        mutex_init(&dev_cb->dev_event_cb.dfx_table.lock);
        INIT_LIST_HEAD(&dev_cb->dev_event_cb.dfx_table.mask_list);
        mutex_unlock(&dev_cb->node_lock);
    }

    return DRV_ERROR_NONE;
}

void dms_event_ctrl_uninit(void)
{
    struct dms_dev_ctrl_block *dev_cb = NULL;
    u32 i;

    for (i = 0; i < DEVICE_NUM_MAX; i++) {
        dev_cb = dms_get_dev_cb(i);
        if (dev_cb == NULL) {
            dms_err("Get dev_ctrl block failed. (dev_id=%u)\n", i);
            continue;
        }
        mutex_lock(&dev_cb->node_lock);
        dms_event_ctrl_converge_list_free(&dev_cb->dev_event_cb.event_list);
        mutex_destroy(&dev_cb->dev_event_cb.event_list.lock);
        dms_event_sensor_reported_list_free(&dev_cb->dev_event_cb.reported_list);
        mutex_destroy(&dev_cb->dev_event_cb.reported_list.lock);
        dms_event_mask_list_clear(dev_cb);
        mutex_destroy(&dev_cb->dev_event_cb.dfx_table.lock);
        mutex_unlock(&dev_cb->node_lock);
    }
}

static int dms_event_ctrl_up(u32 devid)
{
    struct dms_dev_ctrl_block *dev_cb = NULL;
    struct dms_event_ctrl *event_ctrl = NULL;
    u32 i;

    dev_cb = dms_get_dev_cb(devid);
    if (dev_cb == NULL) {
        dms_err("Get dev_ctrl block failed. (dev_id=%u)\n", devid);
        return DRV_ERROR_NO_DEVICE;
    }
    event_ctrl = &dev_cb->dev_event_cb;

    /* converge event list init */
    (void)dms_event_convergent_diagrams_clear(devid, true);

    /* dfx table init */
    for (i = 0; i < DMS_EVENT_TYPE_MAX; i++) {
        atomic_set(&event_ctrl->dfx_table.recv_from_sensor[i], 0);
        atomic_set(&event_ctrl->dfx_table.report_to_consumer[i], 0);
    }
    dms_event_mask_list_clear(dev_cb);

    return DRV_ERROR_NONE;
}

void dms_event_ctrl_down(u32 devid)
{
    struct dms_dev_ctrl_block *dev_cb = NULL;
    struct dms_event_ctrl *event_ctrl = NULL;
    u32 i;

    dev_cb = dms_get_dev_cb(devid);
    if (dev_cb == NULL) {
        return;
    }
    event_ctrl = &dev_cb->dev_event_cb;

    (void)dms_event_convergent_diagrams_clear(devid, true);

    for (i = 0; i < DMS_EVENT_TYPE_MAX; i++) {
        atomic_set(&event_ctrl->dfx_table.recv_from_sensor[i], 0);
        atomic_set(&event_ctrl->dfx_table.report_to_consumer[i], 0);
    }
    dms_event_mask_list_clear(dev_cb);
}

static int dms_event_device_up2(struct devdrv_info *dev_info)
{
    int ret;

    ret = dms_event_ctrl_up(dev_info->dev_id);
    if (ret) {
        dms_err("Dms event ctrl init failed. (dev_id=%u)\n", dev_info->dev_id);
        return ret;
    }

    return DRV_ERROR_NONE;
}

static int dms_event_device_up3(struct devdrv_info *dev_info)
{
    int ret;

    ret = smf_event_subscribe_from_device(dev_info->dev_id);
    if (ret) {
        dms_err("Subscribe from device failed. (dev_id=%u)\n", dev_info->dev_id);
        return ret;
    }

    return DRV_ERROR_NONE;
}

static int dms_event_device_down2(struct devdrv_info *dev_info)
{
    dms_event_ctrl_down(dev_info->dev_id);

    return DRV_ERROR_NONE;
}

static int (*const dms_event_notifier_handle_func[DMS_DEVICE_NOTIFIER_MAX]) \
    (struct devdrv_info *dev_info) = {
        [DMS_DEVICE_REBOOT] = NULL,
        [DMS_DRIVER_REMOVE] = NULL,
        [DMS_DEVICE_SUSPEND] = NULL,
        [DMS_DEVICE_RESUME] = NULL,
        [DMS_DEVICE_UP0] = NULL,
        [DMS_DEVICE_UP1] = NULL,
        [DMS_DEVICE_UP2] = dms_event_device_up2,
        [DMS_DEVICE_UP3] = dms_event_device_up3,
        [DMS_DEVICE_DOWN0] = NULL,
        [DMS_DEVICE_DOWN1] = NULL,
        [DMS_DEVICE_DOWN2] = dms_event_device_down2,
        [DMS_DEVICE_DOWN3] = NULL,
};

static int dms_event_notifier_handle(struct notifier_block *self,
    unsigned long event, void *data)
{
    struct devdrv_info *dev_info = (struct devdrv_info *)data;
    int ret;

    if ((data == NULL) || (event <= DMS_DEVICE_NOTIFIER_MIN) ||
        (event >= DMS_DEVICE_NOTIFIER_MAX)) {
        dms_err("Invalid parameter. (event=0x%lx; data=\"%s\")\n",
                event, data == NULL ? "NULL" : "OK");
        return NOTIFY_BAD;
    }

    if (dms_event_notifier_handle_func[event] == NULL) {
        return NOTIFY_DONE;
    }

    ret = dms_event_notifier_handle_func[event](dev_info);
    if (ret) {
        dms_err("Notifier handle failed. (event=0x%lx; dev_id=%u)\n",
                event, dev_info->dev_id);
        return NOTIFY_BAD;
    }

    dms_debug("Notifier handle success. (event=0x%lx; dev_id=%u)\n",
              event, dev_info->dev_id);
    return NOTIFY_DONE;
}

static struct notifier_block dms_event_notifier = {
    .notifier_call = dms_event_notifier_handle,
};

int dms_event_init(void)
{
    int ret;

    ret = dms_event_convergent_diagrams_init();
    if (ret) {
        dms_event_convergent_diagrams_exit();
        dms_warn("Convergent diagrams init failed. (ret=%d)\n", ret);
    }

    ret = dms_event_ctrl_init();
    if (ret) {
        dms_err("Event ctrl init failed. (ret=%d)\n", ret);
        goto event_ctrl_init_fail;
    }

    dms_event_distribute_stru_init();

    ret = dms_event_distribute_handle_init();
    if (ret) {
        dms_err("Distribute handle init failed. (ret=%d)\n", ret);
        goto distribute_handle_init_fail;
    }

    ret = dms_event_task_init();
    if (ret) {
        dms_err("Dms event task init failed. (ret=%d)\n", ret);
        goto event_task_init_fail;
    }

    ret = dms_register_notifier(&dms_event_notifier);
    if (ret) {
        dms_err("Dms event register notifier failed. (ret=%d)\n", ret);
        goto register_notifier_fail;
    }

    CALL_INIT_MODULE(DMS_EVENT_CMD_NAME);
    dms_debug("Dms event init successful.\n");
    return DRV_ERROR_NONE;

register_notifier_fail:
    dms_event_task_exit();
event_task_init_fail:
    dms_event_subscribe_unregister_all();
distribute_handle_init_fail:
    dms_event_distribute_stru_exit();
    dms_event_ctrl_uninit();
event_ctrl_init_fail:
    dms_event_convergent_diagrams_exit();
    return ret;
}

void dms_event_exit(void)
{
    CALL_EXIT_MODULE(DMS_EVENT_CMD_NAME);

    (void)dms_unregister_notifier(&dms_event_notifier);

    dms_event_task_exit();
    dms_event_subscribe_unregister_all();
    dms_event_distribute_stru_exit();
    dms_event_ctrl_uninit();
    dms_event_convergent_diagrams_exit();
}
