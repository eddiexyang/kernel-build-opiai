/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: Ioctl definition of HDMI.
 * Author: Hisilicon multimedia software group
 * Create: 2012/06/26
 */
#ifndef DRV_HDMI_IOCTL_H
#define DRV_HDMI_IOCTL_H

#include "ot_type.h"
#include "autoconf.h"
#include "drv_hdmi_infoframe.h"
#include "drv_hdmi_edid.h"
#include "drv_hdmi_common.h"
#include "ot_common_hdmi.h"
#include "mkp_ioctl.h"

typedef ot_hdmi_colorimetry     drv_hdmi_colorimetry;
typedef ot_hdmi_rgb_quant_range drv_hdmi_rgb_quant_range;
typedef ot_hdmi_ycc_quant_range drv_hdmi_ycc_quant_range;
typedef ot_hdmi_pic_scaline     drv_hdmi_pic_scaline;

typedef struct {
    ot_hdmi_video_format     video_fmt;
    td_u32                   clk_fs;
    td_u32                   pixel_repeat;
    hdmi_3d_mode             stereo_mode;
    ot_hdmi_video_mode       in_color_space;
    hdmi_hvsync_polarity     hv_sync_pol;
    drv_hdmi_colorimetry     colorimetry;
    drv_hdmi_rgb_quant_range rgb_quantization;
    drv_hdmi_ycc_quant_range ycc_quantization;
    ot_pic_aspect_ratio      picture_aspect;
    hdmi_active_aspect       active_aspect;
    drv_hdmi_pic_scaline     picture_scaling;
    ot_hdmi_deep_color       in_bit_depth;
} hdmi_video_attr;

typedef struct {
    hdmi_device_id      hdmi_id;
    hdmi_default_action default_mode;
} hdmi_open;

typedef struct {
    hdmi_device_id hdmi_id;
    hdmi_event     event;
} drv_hdmi_event;

typedef struct {
    hdmi_device_id      hdmi_id;
    hdmi_default_action default_action;
} drv_hdmi_default_action;

typedef struct {
    hdmi_device_id       hdmi_id;
    hdmi_sink_capability cap;
} drv_hdmi_sink_capability;

typedef struct {
    hdmi_device_id hdmi_id;
    hdmi_property  prop;
} drv_hdmi_property;

typedef struct {
    hdmi_device_id hdmi_id;
    hdmi_vo_attr   vo_attr;
} drv_hdmi_vo_attr;

typedef struct {
    hdmi_device_id    hdmi_id;
    hdmi_infoframe_id infoframe_id;
    hdmi_infoframe_unit    infoframe;
} drv_hdmi_infoframe;

typedef struct {
    hdmi_device_id hdmi_id;
    td_bool        avmute;
} drv_hdmi_avmute;

typedef struct {
    hdmi_device_id     hdmi_id;
    hdmi_edid_raw_data edid_raw;
} drv_hdmi_edid_raw_data;

typedef struct {
    hdmi_device_id hdmi_id;
    hdmi_status    status;
} drv_hdmi_status;

typedef struct {
    hdmi_device_id hdmi_id;
    hdmi_delay     delay;
} drv_hdmi_delay;

typedef struct {
    hdmi_device_id hdmi_id;
    td_bool emi_en;
    hdmi_trace_len trace_len;
} drv_hdmi_mod_param;

typedef struct {
    hdmi_device_id hdmi_id;
    hdmi_hw_spec   hw_spec;
} drv_hdmi_hw_spec;

typedef struct {
    td_u32 timing_width;
    td_u32 timing_height;
    td_u32 fps;
    td_u32 pattern_color_space;
    td_u32 hdmi_id;
} drv_hdmi_timing_format;

typedef struct {
    td_u32 pos_x;
    td_u32 pos_y;
    td_u32 color_r;
    td_u32 color_g;
    td_u32 color_b;
} drv_hdmi_capture;

typedef struct {
    td_u32 timing_width;
    td_u32 timing_height;
} drv_hdmi_detection;

typedef struct {
    td_u32 hotplug_on;
    td_u32 hdmi_id;
} drv_hdmi_hotplug_status;

typedef struct {
    td_u32 reset_mode;
    td_u32 hdmi_id;
} drv_hdmi_reset_clk;

typedef struct {
    td_u32 pattern_mode;
    td_u32 pattern_color_r;
    td_u32 pattern_color_g;
    td_u32 pattern_color_b;
} drv_hdmi_pattern_mode;

typedef struct {
    td_u32 id;
    td_u32 colorspace;
} drv_hdmi_infoframe_colorspace;

enum ioctl_hdmi {
    IOCTL_HDMI_INIT = 0x01,
    IOCTL_HDMI_DEINIT,
    IOCTL_HDMI_OPEN,
    IOCTL_HDMI_CLOSE,
    IOCTL_HDMI_START,
    IOCTL_HDMI_STOP,
    IOCTL_HDMI_GET_SINK_CAPABILITY,
    IOCTL_HDMI_POLL_EVENT,
    IOCTL_HDMI_GET_ATTR,
    IOCTL_HDMI_SET_ATTR,
    IOCTL_HDMI_GET_VO_ATTR,
    IOCTL_HDMI_SET_VO_ATTR,
    IOCTL_HDMI_GET_INFOFRAME,
    IOCTL_HDMI_SET_INFOFRAME,
    IOCTL_HDMI_AVMUTE,
    IOCTL_HDMI_UPDATE_EDID,
    IOCTL_HDMI_REG_CALLBACK_FUNC,
    IOCTL_HDMI_GET_STATUS,
    IOCTL_HDMI_REGISTER_CALLBACK,
    IOCTL_HDMI_UNREGISTER_CALLBACK,
    IOCTL_HDMI_SET_MOD_PARAM,
    IOCTL_HDMI_GET_MOD_PARAM,
    IOCTL_HDMI_SET_HW_SPEC,
    IOCTL_HDMI_GET_HW_SPEC,
    IOCTL_HDMI_TIMING_TEST,
    IOCTL_HDMI_RESET_CLOCK,
    IOCTL_HDMI_SET_PATTERN,
    IOCTL_HDMI_SET_HOTPLUG,
    IOCTL_HDMI_SET_DETECTION,
    IOCTL_HDMI_SET_CAPTURE,
    IOCTL_HDMI_AVI_INFOFRAME,
    IOCTL_HDMI_MAX
};

#define CMD_HDMI_INIT                 _IO(IOC_TYPE_HDMI, IOCTL_HDMI_INIT)
#define CMD_HDMI_DEINIT               _IO(IOC_TYPE_HDMI, IOCTL_HDMI_DEINIT)
#define CMD_HDMI_OPEN                 _IOWR(IOC_TYPE_HDMI, (td_s32)IOCTL_HDMI_OPEN, drv_hdmi_default_action)
#define CMD_HDMI_CLOSE                _IOWR(IOC_TYPE_HDMI, (td_s32)IOCTL_HDMI_CLOSE, hdmi_device_id)
#define CMD_HDMI_START                _IOWR(IOC_TYPE_HDMI, (td_s32)IOCTL_HDMI_START, hdmi_device_id)
#define CMD_HDMI_STOP                 _IOWR(IOC_TYPE_HDMI, (td_s32)IOCTL_HDMI_STOP, hdmi_device_id)
#define CMD_HDMI_SET_INFOFRAME        _IOWR(IOC_TYPE_HDMI, (td_s32)IOCTL_HDMI_SET_INFOFRAME, drv_hdmi_infoframe)
#define CMD_HDMI_SET_ATTR             _IOWR(IOC_TYPE_HDMI, (td_s32)IOCTL_HDMI_SET_ATTR, drv_hdmi_property)
#define CMD_HDMI_GET_ATTR             _IOWR(IOC_TYPE_HDMI, (td_s32)IOCTL_HDMI_GET_ATTR, drv_hdmi_property)
#define CMD_HDMI_UNREGISTER_CALLBACK  _IOWR(IOC_TYPE_HDMI, (td_s32)IOCTL_HDMI_UNREGISTER_CALLBACK, hdmi_device_id)
#define CMD_HDMI_POLL_EVENT           _IOWR(IOC_TYPE_HDMI, (td_s32)IOCTL_HDMI_POLL_EVENT, drv_hdmi_event)
#define CMD_HDMI_GET_STATUS           _IOWR(IOC_TYPE_HDMI, (td_s32)IOCTL_HDMI_GET_STATUS, drv_hdmi_status)
#define CMD_HDMI_REGISTER_CALLBACK    _IOWR(IOC_TYPE_HDMI, (td_s32)IOCTL_HDMI_REGISTER_CALLBACK, hdmi_device_id)
#define CMD_HDMI_GET_SINK_CAPABILITY  _IOWR(IOC_TYPE_HDMI, \
                                            (td_s32)IOCTL_HDMI_GET_SINK_CAPABILITY, drv_hdmi_sink_capability)
#ifdef HDMI_DFX_SUPPORT
#define CMD_HDMI_GET_INFOFRAME        _IOWR(IOC_TYPE_HDMI, (td_s32)IOCTL_HDMI_GET_INFOFRAME, drv_hdmi_infoframe)
#define CMD_HDMI_SET_AVMUTE           _IOWR(IOC_TYPE_HDMI, (td_s32)IOCTL_HDMI_AVMUTE, drv_hdmi_avmute)
#define CMD_HDMI_UPDATE_EDID          _IOWR(IOC_TYPE_HDMI, (td_s32)IOCTL_HDMI_UPDATE_EDID, drv_hdmi_edid_raw_data)
#define CMD_HDMI_SET_VO_ATTR          _IOWR(IOC_TYPE_HDMI, (td_s32)IOCTL_HDMI_SET_VO_ATTR, drv_hdmi_vo_attr)
#define CMD_HDMI_GET_VO_ATTR          _IOWR(IOC_TYPE_HDMI, (td_s32)IOCTL_HDMI_GET_VO_ATTR, drv_hdmi_vo_attr)
#define CMD_HDMI_GET_HW_SPEC          _IOWR(IOC_TYPE_HDMI, (td_s32)IOCTL_HDMI_GET_HW_SPEC, drv_hdmi_hw_spec)
#define CMD_HDMI_SET_HW_SPEC          _IOWR(IOC_TYPE_HDMI, (td_s32)IOCTL_HDMI_SET_HW_SPEC, drv_hdmi_hw_spec)
#define CMD_HDMI_SET_MOD_PARAM        _IOWR(IOC_TYPE_HDMI, (td_s32)IOCTL_HDMI_SET_MOD_PARAM, drv_hdmi_mod_param)
#define CMD_HDMI_GET_MOD_PARAM        _IOWR(IOC_TYPE_HDMI, (td_s32)IOCTL_HDMI_GET_MOD_PARAM, drv_hdmi_mod_param)

#define CMD_HDMI_SET_TIMING           _IOWR(IOC_TYPE_HDMI, (td_s32)IOCTL_HDMI_TIMING_TEST, drv_hdmi_timing_format)
#define CMD_HDMI_RESET_CLOCK          _IOWR(IOC_TYPE_HDMI, (td_s32)IOCTL_HDMI_RESET_CLOCK, drv_hdmi_reset_clk)
#define CMD_HDMI_SET_PATTERN          _IOWR(IOC_TYPE_HDMI, (td_s32)IOCTL_HDMI_SET_PATTERN, drv_hdmi_pattern_mode)
#define CMD_HDMI_SET_HOTPLUG          _IOWR(IOC_TYPE_HDMI, (td_s32)IOCTL_HDMI_SET_HOTPLUG, drv_hdmi_hotplug_status)
#define CMD_HDMI_SET_DETECTION        _IOWR(IOC_TYPE_HDMI, (td_s32)IOCTL_HDMI_SET_DETECTION, drv_hdmi_detection)
#define CMD_HDMI_SET_CAPTURE          _IOWR(IOC_TYPE_HDMI, (td_s32)IOCTL_HDMI_SET_CAPTURE, drv_hdmi_capture)
#define CMD_HDMI_AVI_INFOFRAME        _IOWR(IOC_TYPE_HDMI, (td_s32)IOCTL_HDMI_AVI_INFOFRAME, \
                                            drv_hdmi_infoframe_colorspace)
#endif

#endif

