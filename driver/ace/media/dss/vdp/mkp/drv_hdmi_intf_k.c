/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: hdmi intf.
 * Author: Hisilicon multimedia software group
 * Create: 2012/06/26
 */
#include "drv_hdmi_intf_k.h"
#include "drv_hdmi_intf.h"
#include "hdmi_hal.h"
#include "drv_hdmi_edid.h"
#include "drv_hdmi_event.h"
#include "hdmi_product_define.h"
#include "drv_hdmi_ioctl.h"

#ifdef ASCEND310B_DRM_HDMI_ENABLE
#include "soc_adp_hdmi_calllback.h"
#endif

static td_s32 hdmi_ext_ioctl(unsigned int cmd, void *argp)
{
    td_s32 ret;
    ret = drv_hdmi_cmd_process(cmd, argp, TD_FALSE);
    return ret;
}

static td_void hdmi_hot_plug_process(hdmi_device_id hdmi_id)
{
    td_s32 ret;
    drv_hdmi_status       current_status = {0};
    drv_hdmi_property     property       = {0};
    hdmi_device          *hdmi_dev       = TD_NULL;
    hdmi_sink_capability *sink_cap       = TD_NULL;

    hdmi_info("\n---HDMI%u kernel event(no user_call_back): HOTPLUG. --- \n", (td_u32)hdmi_id);

    hdmi_dev = get_hdmi_device(hdmi_id);
    hdmi_if_null_return_void(hdmi_dev);

    get_hdmi_default_action_set(hdmi_dev, HDMI_DEFAULT_ACTION_HDMI);
    ret = (td_s32)drv_hdmi_edid_capability_get(&hdmi_dev->edid_info, &sink_cap);
    if (ret == (td_s32)HDMI_EDID_DATA_INVALID) {
        hdmi_warn("get sink capability err!\n");
    }

    current_status.hdmi_id = hdmi_id;
    ret = hdmi_ioctrl_get_status(&current_status, TD_FALSE);
    if (ret != TD_SUCCESS) {
        hdmi_err("get HDMI status err!:0x%d\n", ret);
        return;
    }
    if (current_status.status.connected == TD_FALSE) {
        hdmi_err("no connect!\n");
        return;
    }
    hdmi_info("connect !\n");
    property.hdmi_id = hdmi_id;
    ret = hdmi_ext_ioctl(CMD_HDMI_GET_ATTR, &property);
    if (ret != TD_SUCCESS) {
        hdmi_err("get hdmi attr err!:0x%d\n", ret);
        return;
    }
    hdmi_info("CMD_HDMI_GET_ATTR ok! \n");
    ret = hdmi_ext_ioctl(CMD_HDMI_SET_ATTR, &property);
    if (ret != TD_SUCCESS) {
        hdmi_err("set attr err!:0x%d\n", ret);
    }
    hdmi_info("CMD_HDMI_SET_ATTR ok! \n");
    ret = hdmi_ext_ioctl(CMD_HDMI_START, &hdmi_id);
    if (ret != TD_SUCCESS) {
        hdmi_err("hdmi start  err!:0x%d\n", ret);
        return;
    }
    hdmi_info("CMD_HDMI_START ok! \n");

    return;
}

/* hot unplug process */
static td_void hdmi_hot_unplug_process(hdmi_device_id hdmi_id)
{
    td_s32 ret;

    ret = hdmi_ext_ioctl(CMD_HDMI_STOP, &hdmi_id);
    if (ret != TD_SUCCESS) {
        hdmi_err("hdmi%u stop err!:0x%d\n", (td_u32)hdmi_id, ret);
        return;
    }
    hdmi_info("hdmi%u stop ok!\n", (td_u32)hdmi_id);

    return;
}

static td_s32 csc_param_check(const hdmi_csc_param *csc_param)
{
    if (csc_param->colorimetry != HDMI_COLORIMETRY_ITU601 &&
        csc_param->colorimetry != HDMI_COLORIMETRY_ITU709 &&
        csc_param->colorimetry != HDMI_COLORIMETRY_2020_CONST_LUMINOUS &&
        csc_param->colorimetry != HDMI_COLORIMETRY_2020_NON_CONST_LUMINOUS) {
        hdmi_err("colorimetry err!\n");
        return TD_FAILURE;
    }

    if (csc_param->quantization != HDMI_QUANT_RANGE_LIMITED &&
        csc_param->quantization != HDMI_QUANT_RANGE_FULL) {
        hdmi_err("quantization err!\n");
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_s32 video_param_check(const hdmi_video_param *video_param)
{
    if (video_param->pixel_encoding != HDMI_EXT_COLORSPACE_RGB &&
        video_param->pixel_encoding != HDMI_EXT_COLORSPACE_YCBCR444) {
        hdmi_err("pixel_encoding err!\n");
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

td_s32 drv_hdmi_kernel_event_callback(td_void *data, hdmi_event event)
{
    hdmi_device_id hdmi_id;
    td_u32 need_hpd_process;
    hdmi_device *hdmi_dev = TD_NULL;

    hdmi_if_null_return(data, OT_ERR_HDMI_NULL_PTR);

    hdmi_id = *(hdmi_device_id *)data;
    hdmi_dev = get_hdmi_device(hdmi_id);
    hdmi_if_null_return(hdmi_dev, OT_ERR_HDMI_NULL_PTR);

    need_hpd_process = ((td_u32)hdmi_dev->run_state & HDMI_RUN_STATE_START) ||
        ((td_u32)hdmi_dev->run_state & HDMI_RUN_STATE_STOP);
    if (need_hpd_process) {
#ifdef ASCEND310B_DRM_HDMI_ENABLE
    hdmi_callback_drm(0, event, &hdmi_id);
    hdmi_debug("HDMI_EVENT_HOTPLUG or HDMI_EVENT_HOTUNPLUG\n");
#endif
        if (event == HDMI_EVENT_HOTPLUG) {
            hdmi_hot_plug_process(hdmi_id);
            hdmi_debug("HDMI_EVENT_HOTPLUG\n");
        } else if (event == HDMI_EVENT_HOTUNPLUG) {
            hdmi_hot_unplug_process(hdmi_id);
            hdmi_debug("HDMI_EVENT_HOTUNPLUG\n");
        }
    }

    return TD_SUCCESS;
}

td_s32 drv_hdmi_csc_param_set_export(hdmi_dev_id hdmi, const hdmi_csc_param *csc_param)
{
#ifdef HDMI_DFX_SUPPORT
    td_s32 ret;
    hdmi_device *hdmi_dev = TD_NULL;
    drv_hdmi_vo_attr video_attr = {0};

    hdmi_debug("in...\n");

    if (get_hdmi_device(hdmi) == TD_NULL) {
        hdmi_err("device id is wrong\n");
        return TD_FAILURE;
    }

    hdmi_dev = get_hdmi_device(hdmi);
    hdmi_if_null_return(csc_param, OT_ERR_HDMI_NULL_PTR);

    /* param check */
    ret = csc_param_check(csc_param);
    if (ret != TD_SUCCESS) {
        hdmi_err("csc_param_check fail... :0x%d\n", ret);
        return ret;
    }

    hdmi_dev->csc_param.colorimetry  = csc_param->colorimetry;
    hdmi_dev->csc_param.quantization = csc_param->quantization;
    /* HDMI not open */
    if (hdmi_dev->kernel_cnt == 0 && hdmi_dev->user_cnt == 0) {
        hdmi_warn("device not open! save param to dev. colorimetry=%u, quantization=%u\n",
            (td_u32)csc_param->colorimetry, (td_u32)csc_param->quantization);
        return TD_SUCCESS;
    } else {
        hdmi_dev->attr.app_attr.out_csc_quantization = csc_param->quantization;
        video_attr.hdmi_id = hdmi;
        ret = hdmi_ext_ioctl(CMD_HDMI_GET_VO_ATTR, &video_attr);
        if (ret != TD_SUCCESS) {
            hdmi_err("get vo attr fail... :0x%d\n", ret);
            return ret;
        }
        video_attr.vo_attr.colorimetry = csc_param->colorimetry;
        ret = hdmi_ext_ioctl(CMD_HDMI_SET_VO_ATTR, &video_attr);
        if (ret != TD_SUCCESS) {
            hdmi_err("set attr fail... :0x%d\n", ret);
            return ret;
        }
    }
    hdmi_debug("out...\n");

    return ret;
#else
    return TD_SUCCESS;
#endif
}

td_s32 drv_hdmi_video_param_set_export(hdmi_dev_id hdmi, const hdmi_video_param *video_param)
{
#ifdef HDMI_DFX_SUPPORT
    td_s32 ret;
    hdmi_device *hdmi_dev = TD_NULL;
    drv_hdmi_vo_attr vid_attr = {0};

    hdmi_debug("in...\n");

    if (get_hdmi_device(hdmi) == TD_NULL) {
        hdmi_err("device id is wrong :0x%d\n", (td_s32)hdmi);
        return TD_FAILURE;
    }

    hdmi_dev = get_hdmi_device(hdmi);
    hdmi_if_null_return(hdmi_dev, OT_ERR_HDMI_NULL_PTR);
    hdmi_if_null_return(video_param, OT_ERR_HDMI_NULL_PTR);

    /* param check */
    ret = video_param_check(video_param);
    if (ret != TD_SUCCESS) {
        hdmi_err("video_param_check fail... :0x%d\n", ret);
        return ret;
    }

    hdmi_dev->csc_param.pixel_encoding = video_param->pixel_encoding;

    /* HDMI not open */
    if (hdmi_dev->kernel_cnt == 0 && hdmi_dev->user_cnt == 0) {
        hdmi_warn("device not open! save param to dev. pixel_encoding = %u\n", (td_u32)video_param->pixel_encoding);
        return TD_SUCCESS;
    } else {
        hdmi_dev->attr.app_attr.out_color_space = video_param->pixel_encoding;
        hdmi_dev->attr.vo_attr.in_color_space = video_param->pixel_encoding;
        vid_attr.hdmi_id = hdmi;
        ret = hdmi_ext_ioctl(CMD_HDMI_GET_VO_ATTR, &vid_attr);
        if (ret != TD_SUCCESS) {
            hdmi_err("get vo attr fail... :0x%d\n", ret);
            return ret;
        }
        ret = hdmi_ext_ioctl(CMD_HDMI_SET_VO_ATTR, &vid_attr);
        if (ret != TD_SUCCESS) {
            hdmi_err("set attr fail... :0x%d\n", ret);
            return ret;
        }
    }
    hdmi_debug("out...\n");

    return ret;
#else
    return TD_SUCCESS;
#endif
}

td_s32 drv_hdmi_stop_export(hdmi_dev_id hdmi)
{
    td_s32 ret;

    hdmi_debug("in...\n");
    ret = drv_hdmi_cmd_process(CMD_HDMI_STOP, (td_void *)(&hdmi), TD_FALSE);
    if (ret != TD_SUCCESS) {
        hdmi_err("stop hdmi err!:0x%d\n", ret);
        return ret;
    }
    hdmi_debug("out...\n");

    return ret;
}

