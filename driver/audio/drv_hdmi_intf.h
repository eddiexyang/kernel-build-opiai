/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: hdmi intf.
 * Author: Hisilicon multimedia software group
 * Create: 2012/06/26
 */
#ifndef DRV_HDMI_INTF_H
#define DRV_HDMI_INTF_H

#include "ot_type.h"
#include "hdmi_hal.h"
#include "drv_hdmi_common.h"
#include "drv_hdmi_ioctl.h"
#include "drv_hdmi_edid.h"
#include "drv_hdmi_event.h"

#ifdef ASCEND310B_DRM_HDMI_ENABLE
#include "drv_hdmi_drm_init.h"
#endif

typedef struct {
    hdmi_thread_state thread_timer_sate;
    osal_task_t      *thread_timer;
    td_bool           thread_run;
    td_bool           thread_exit;
    osal_wait_t       wait_destroy;
} hdmi_thread_info;

typedef struct {
    td_bool         attr_busy;
    td_bool         attr_set;
    osal_spinlock_t attr_spin;
} hdmi_hdr_mutex;

typedef struct {
    td_u32                 hdmi_dev_id;
    td_u32                 kernel_cnt;
    td_u32                 user_cnt;
    td_u32                 user_callback_cnt;
    td_bool                hpd_detected;
    td_bool                hpd_notifyed;
    hdmi_run_state         run_state;
    hdmi_transition_state  transition_state;
    hdmi_tmds_mode         tmds_mode;
    hdmi_callback          k_callback;
    hdmi_thread_info       thread_info;
    hdmi_edid_info         edid_info;
    hdmi_infoframe    info_frame;
    hdmi_attr              attr;
    hdmi_delay             delay;
    hdmi_hal              *hal;
    hdmi_debug             debug;
    osal_semaphore_t       mutex_thread;
    osal_semaphore_t       mutex_proc;
    hdmi_mode_param        mode_param;
    hdmi_csc_attr          csc_param;
    hdmi_user_vsif_content vendor_data;
} hdmi_device;

td_s32 drv_hdmi_start(hdmi_device *hdmi_dev);

td_s32 drv_hdmi_stop(hdmi_device *hdmi_dev);

td_void drv_hdmi_avmute_set(const hdmi_device *hdmi_dev, td_bool av_mute);

td_s32 drv_hdmi_vo_attr_set(hdmi_device *hdmi_dev, const hdmi_vo_attr *vo_attr);

td_s32 drv_hdmi_attr_get(const hdmi_device *hdmi_dev, hdmi_attr *attr);

td_s32 drv_hdmi_attr_set(hdmi_device *hdmi_dev, hdmi_attr *attr);

td_s32 drv_hdmi_cmd_process(unsigned int cmd, td_void *arg, td_bool user);

hdmi_device *get_hdmi_device(hdmi_device_id hdmi_id);

td_void get_hdmi_default_action_set(hdmi_device *hdmi_dev, hdmi_default_action action);

td_s32 hdmi_thread_state_set(hdmi_device *hdmi_dev, hdmi_thread_state state);

td_s32 hdmi_mode_strategy(hdmi_device *hdmi_dev);

td_void hdmi_check_pid_spin_lock_init(td_void);
td_void hdmi_check_pid_spin_lock_destory(td_void);
td_s32 hdmi_set_reg(td_u32 id, td_char *reg);
td_s32 hdmi_set_phy(td_u32 id, char *phy);

td_s32 drm_hdmi_open(td_void *arg, td_bool user);
td_s32 drm_hdmi_set_attr(td_void *arg, td_bool user);
td_s32 drm_hdmi_set_infoframe(td_void *arg, td_bool user);
td_s32 drm_hdmi_start(td_void *arg, td_bool user);
td_s32 hdmi_ioctrl_get_status(td_void *arg, td_bool user);

td_void drv_hdmi_phy_product_power_set_param(td_void);

td_s32 drm_hdmi_close(td_void *arg, td_bool user);

#ifdef ASCEND310B_DRM_HDMI_ENABLE
td_void drm_hdmi_update_edid(struct pppoe_tag *tag);
#endif

td_s32 drm_hdmi_format_to_hdmi_timing(td_u32 format);

td_s32 drm_hdmi_get_timing_from_mode(td_u32 vact, td_u32 hact, td_u32 crtc_refresh_rate,
                                     td_u32 *hdmi_intf_sync, td_u32 *hdmi_pix_freq);

#ifdef ASCEND310B_DRM_HDMI_ENABLE
td_s32 drm_hdmi_get_connect_status(td_u32 hdmi_id);
#endif
#endif /* DRV_HDMI_INTF_H */