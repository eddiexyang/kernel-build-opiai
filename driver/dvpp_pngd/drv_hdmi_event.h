/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: hdmi event.
 * Author: Hisilicon multimedia software group
 * Create: 2012/06/26
 */
#ifndef DRV_HDMI_EVENT_H
#define DRV_HDMI_EVENT_H

#include "ot_type.h"
#include "drv_hdmi_common.h"
#include "hdmi_product_define.h"

#define HDMI_EVENT_POOL_SIZE 10
#define HDMI_EVENT_ID_EXIST  0xff
#define HDMI_EVENT_POOL_CNT  10U

typedef struct {
    td_u32 hpd_rd_cnt;
    td_u32 hpd_wr_cnt;
    td_u32 unhpd_rd_cnt;
    td_u32 unhpd_wr_cnt;
    td_u32 edid_fail_rd_cnt;
    td_u32 edid_fail_wr_cnt;
    td_u32 rsen_con_rd_cnt;
    td_u32 rsen_con_wr_cnt;
    td_u32 rsen_dis_rd_cnt;
    td_u32 rsen_dis_wr_cnt;
    td_u32 err_rd_cnt;
    td_u32 err_wd_cnt;
} hdmi_event_run_cnt;

typedef struct {
    td_u32     pool_id;
    td_bool    wakeup_flag;
    td_u32     readable_cnt;
    td_u32     read_ptr;
    td_u32     write_ptr;
    hdmi_event event_pool[HDMI_EVENT_POOL_SIZE];
} hdmi_event_run_ctrl;

typedef struct {
    hdmi_event_run_cnt run_cnt;
    hdmi_event_run_ctrl ctrl;
} hdmi_event_pool;

typedef struct {
    osal_semaphore_t event_mutex; /* lock to avoid competition */
    osal_wait_t      wr_queue;    /* wait queue to wake up the sleeping process */
    td_bool          wakeup_all;  /* wakeup all pool flag */
    td_bool          init;        /* is the struct init */
    td_u32           total;       /* total pools in the list */
    hdmi_event_pool  pool[HDMI_EVENT_POOL_CNT];
    hdmi_event       event;
} hdmi_event_info;

typedef struct {
    hdmi_event_info *evt_info;
    hdmi_event_pool *tmp_pool;
} hdmi_event_wait_callback;

td_s32 drv_hdmi_event_init(hdmi_device_id hdmi_id);
td_s32 drv_hdmi_event_deinit(hdmi_device_id hdmi_id);
td_s32 drv_hdmi_event_pool_malloc(hdmi_device_id hdmi_id, td_u32 *pool_id);
td_s32 drv_hdmi_event_pool_free(hdmi_device_id hdmi_id, td_u32 pool_id);
td_s32 drv_hdmi_event_pool_write(hdmi_device_id hdmi_id, hdmi_event event);
td_s32 drv_hdmi_event_pool_read(hdmi_device_id hdmi_id, td_u32 pool_id, hdmi_event *event);
td_s32 drv_hdmi_event_pool_status_get(hdmi_device_id hdmi_id, td_u32 pool_num,
                                      hdmi_event_run_ctrl *ctrl, hdmi_event_run_cnt *cnt);
td_s32 drv_hdmi_event_pool_total_get(hdmi_device_id hdmi_id, td_u32 *total);

#endif  /* DRV_HDMI_EVENT_H */

