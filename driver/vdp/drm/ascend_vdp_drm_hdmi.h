#ifndef ASCEND_VDP_DRM_HDMI_H
#define ASCEND_VDP_DRM_HDMI_H

#include <linux/if_pppox.h>

#include "ot_common_hdmi.h"
#include "ot_type.h"

#define ASCEND_HDMI_AVI_INFOFRAME_SIZE 13

enum ascend_hdmi_device_id {
	ASCEND_HDMI_DEVICE_ID0,
	ASCEND_HDMI_DEVICE_ID1,
	ASCEND_HDMI_DEVICE_ID_BUTT,
};

enum ascend_hdmi_default_action {
	ASCEND_HDMI_DEFAULT_ACTION_NULL,
	ASCEND_HDMI_DEFAULT_ACTION_HDMI,
	ASCEND_HDMI_DEFAULT_ACTION_DVI,
	ASCEND_HDMI_DEFAULT_ACTION_BUTT,
};

enum ascend_hdmi_deep_color {
	ASCEND_HDMI_DEEP_COLOR_24BIT,
	ASCEND_HDMI_DEEP_COLOR_30BIT,
	ASCEND_HDMI_DEEP_COLOR_36BIT,
	ASCEND_HDMI_DEEP_COLOR_48BIT,
	ASCEND_HDMI_DEEP_COLOR_OFF = 0xff,
	ASCEND_HDMI_DEEP_COLOR_BUTT,
};

enum ascend_hdmi_video_timing {
	ASCEND_HDMI_VIDEO_TIMING_UNKNOWN,
};

enum ascend_hdmi_disp_format {
	ASCEND_HDMI_VIDEO_FORMAT_1080P_60,
	ASCEND_HDMI_VIDEO_FORMAT_BUTT = 0x7fffffff,
};

enum ascend_hdmi_infoframe_type {
	ASCEND_HDMI_INFOFRAME_TYPE_NULL,
	ASCEND_HDMI_INFOFRAME_TYPE_VENDOR = 0x81,
	ASCEND_HDMI_INFOFRAME_TYPE_AVI,
	ASCEND_HDMI_INFOFRAME_TYPE_SPD,
	ASCEND_HDMI_INFOFRAME_TYPE_AUDIO,
	ASCEND_HDMI_INFOFRAME_TYPE_MPEG,
	ASCEND_HDMI_INFOFRAME_TYPE_GBD,
	ASCEND_HDMI_INFOFRAME_TYPE_DRM,
	ASCEND_HDMI_INFOFRAME_TYPE_BUTT,
};

enum ascend_hdmi_colorspace {
	ASCEND_HDMI_COLORSPACE_RGB,
	ASCEND_HDMI_COLORSPACE_YCBCR422,
	ASCEND_HDMI_COLORSPACE_YCBCR444,
	ASCEND_HDMI_COLORSPACE_YCBCR420,
	ASCEND_HDMI_COLORSPACE_BUTT,
};

enum ascend_hdmi_scan_mode {
	ASCEND_HDMI_SCAN_MODE_NONE,
	ASCEND_HDMI_SCAN_MODE_OVERSCAN,
	ASCEND_HDMI_SCAN_MODE_UNDERSCAN,
	ASCEND_HDMI_SCAN_MODE_FUTURE,
	ASCEND_HDMI_SCAN_MODE_BUTT,
};

enum ascend_hdmi_colorimetry {
	ASCEND_HDMI_COLORIMETRY_NONE,
	ASCEND_HDMI_COLORIMETRY_ITU_601,
	ASCEND_HDMI_COLORIMETRY_ITU_709,
	ASCEND_HDMI_COLORIMETRY_EXTEND,
};

enum ascend_hdmi_picture_aspect {
	ASCEND_HDMI_PICTURE_ASPECT_NONE,
	ASCEND_HDMI_PICTURE_ASPECT_4_3,
	ASCEND_HDMI_PICTURE_ASPECT_16_9,
	ASCEND_HDMI_PICTURE_ASPECT_64_27,
	ASCEND_HDMI_PICTURE_ASPECT_256_135,
	ASCEND_HDMI_PICTURE_ASPECT_FUTURE,
	ASCEND_HDMI_PICTURE_ASPECT_BUTT,
};

enum ascend_hdmi_active_aspect {
	ASCEND_HDMI_ACTIVE_ASPECT_16_9_TOP = 2,
	ASCEND_HDMI_ACTIVE_ASPECT_14_9_TOP,
	ASCEND_HDMI_ACTIVE_ASPECT_16_9_CENTER,
	ASCEND_HDMI_ACTIVE_ASPECT_PICTURE = 8,
	ASCEND_HDMI_ACTIVE_ASPECT_4_3,
	ASCEND_HDMI_ACTIVE_ASPECT_16_9,
	ASCEND_HDMI_ACTIVE_ASPECT_14_9,
	ASCEND_HDMI_ACTIVE_ASPECT_4_3_SP_14_9 = 13,
	ASCEND_HDMI_ACTIVE_ASPECT_16_9_SP_14_9,
	ASCEND_HDMI_ACTIVE_ASPECT_16_9_SP_4_3,
	ASCEND_HDMI_ACTIVE_ASPECT_BUTT,
};

enum ascend_hdmi_extended_colormetry {
	ASCEND_HDMI_EXTENDED_COLORIMETRY_XV_YCC_601,
	ASCEND_HDMI_EXTENDED_COLORIMETRY_XV_YCC_709,
	ASCEND_HDMI_EXTENDED_COLORIMETRY_S_YCC_601,
	ASCEND_HDMI_EXTENDED_COLORIMETRY_ADOBE_YCC_601,
	ASCEND_HDMI_EXTENDED_COLORIMETRY_ADOBE_RGB,
	ASCEND_HDMI_EXTENDED_COLORIMETRY_2020_CONST_LUMINOUS,
	ASCEND_HDMI_EXTENDED_COLORIMETRY_2020_NON_CONST_LUMINOUS,
	ASCEND_HDMI_EXTENDED_COLORIMETRY_ADDITIONAL,
	ASCEND_HDMI_EXTENDED_COLORIMETRY_BUTT,
};

enum ascend_hdmi_quantization_range {
	ASCEND_HDMI_QUANTIZATION_RANGE_DEFAULT,
	ASCEND_HDMI_QUANTIZATION_RANGE_LIMITED,
	ASCEND_HDMI_QUANTIZATION_RANGE_FULL,
	ASCEND_HDMI_QUANTIZATION_RANGE_BUTT,
};

enum ascend_hdmi_nups {
	ASCEND_HDMI_NUPS_UNKNOWN,
	ASCEND_HDMI_NUPS_HORIZONTAL,
	ASCEND_HDMI_NUPS_VERTICAL,
	ASCEND_HDMI_NUPS_BOTH,
};

enum ascend_hdmi_ycc_quantization_range {
	ASCEND_HDMI_YCC_QUANTIZATION_RANGE_LIMITED,
	ASCEND_HDMI_YCC_QUANTIZATION_RANGE_FULL,
	ASCEND_HDMI_YCC_QUANTIZATION_RANGE_BUTT,
};

enum ascend_hdmi_content_type {
	ASCEND_HDMI_CONTENT_TYPE_NONE,
	ASCEND_HDMI_CONTENT_TYPE_PHOTO,
	ASCEND_HDMI_CONTENT_TYPE_CINEMA,
	ASCEND_HDMI_CONTENT_TYPE_GAME,
};

enum ascend_hdmi_audio_sample_size {
	ASCEND_HDMI_AUDIO_SAMPLE_SIZE_STREAM,
	ASCEND_HDMI_AUDIO_SAMPLE_SIZE_16,
	ASCEND_HDMI_AUDIO_SAMPLE_SIZE_20,
	ASCEND_HDMI_AUDIO_SAMPLE_SIZE_24,
};

enum ascend_hdmi_audio_sample_frequency {
	ASCEND_HDMI_AUDIO_SAMPLE_FREQUENCY_STREAM,
	ASCEND_HDMI_AUDIO_SAMPLE_FREQUENCY_32000,
	ASCEND_HDMI_AUDIO_SAMPLE_FREQUENCY_44100,
	ASCEND_HDMI_AUDIO_SAMPLE_FREQUENCY_48000,
	ASCEND_HDMI_AUDIO_SAMPLE_FREQUENCY_88200,
	ASCEND_HDMI_AUDIO_SAMPLE_FREQUENCY_96000,
	ASCEND_HDMI_AUDIO_SAMPLE_FREQUENCY_176400,
	ASCEND_HDMI_AUDIO_SAMPLE_FREQUENCY_192000,
};

enum ascend_hdmi_audio_coding_type_ext {
	ASCEND_HDMI_AUDIO_CODING_TYPE_EXT_STREAM,
	ASCEND_HDMI_AUDIO_CODING_TYPE_EXT_HE_AAC,
	ASCEND_HDMI_AUDIO_CODING_TYPE_EXT_HE_AAC_V2,
	ASCEND_HDMI_AUDIO_CODING_TYPE_EXT_MPEG_SURROUND,
};

enum ascend_hdmi_audio_lfe_playback_level {
	ASCEND_HDMI_AUDIO_LFE_PLAYBACK_NO,
	ASCEND_HDMI_AUDIO_LFE_PLAYBACK_0_DB,
	ASCEND_HDMI_AUDIO_LFE_PLAYBACK_10_DB,
	ASCEND_HDMI_AUDIO_LFE_PLAYBACK_RESERVED,
};

enum ascend_hdmi_audio_code {
	ASCEND_HDMI_AUDIO_CODING_TYPE_STREAM,
	ASCEND_HDMI_AUDIO_CODING_TYPE_PCM,
	ASCEND_HDMI_AUDIO_CODING_TYPE_AC3,
	ASCEND_HDMI_AUDIO_CODING_TYPE_MPEG1,
	ASCEND_HDMI_AUDIO_CODING_TYPE_MP3,
	ASCEND_HDMI_AUDIO_CODING_TYPE_MPEG2,
	ASCEND_HDMI_AUDIO_CODING_TYPE_AAC_LC,
	ASCEND_HDMI_AUDIO_CODING_TYPE_DTS,
	ASCEND_HDMI_AUDIO_CODING_TYPE_ATRAC,
	ASCEND_HDMI_AUDIO_CODING_TYPE_DSD,
	ASCEND_HDMI_AUDIO_CODING_TYPE_EAC3,
	ASCEND_HDMI_AUDIO_CODING_TYPE_DTS_HD,
	ASCEND_HDMI_AUDIO_CODING_TYPE_MLP,
	ASCEND_HDMI_AUDIO_CODING_TYPE_DST,
	ASCEND_HDMI_AUDIO_CODING_TYPE_WMA_PRO,
	ASCEND_HDMI_AUDIO_CODING_TYPE_RESERVED,
	ASCEND_HDMI_AUDIO_CODING_TYPE_BUTT,
};

struct ascend_hdmi_avi_infoframe {
	u32 type;
	u8 version;
	u8 length;
	u32 colorspace;
	td_bool active_info_valid;
	td_bool horizontal_bar_valid;
	td_bool vertical_bar_valid;
	u32 scan_mode;
	u32 colorimetry;
	u32 picture_aspect;
	u32 active_aspect;
	td_bool itc;
	u32 extended_colorimetry;
	u32 quantization_range;
	u32 nups;
	u32 video_timing;
	u32 disp_fmt;
	u32 ycc_quantization_range;
	u32 content_type;
	u8 pixel_repeat;
	u16 top_bar;
	u16 bottom_bar;
	u16 left_bar;
	u16 right_bar;
};

struct ascend_hdmi_audio_infoframe {
	u32 type;
	u8 version;
	u8 length;
	u8 channels;
	u32 coding_type;
	u32 sample_size;
	u32 sample_frequency;
	u32 coding_type_ext;
	u8 channel_allocation;
	u32 lfe_playback_level;
	u8 level_shift_value;
	td_bool downmix_inhibit;
};

union ascend_hdmi_infoframe_unit {
	struct ascend_hdmi_avi_infoframe avi_infoframe;
	struct ascend_hdmi_audio_infoframe audio_infoframe;
};

struct ascend_drm_hdmi_infoframe {
	u32 hdmi_id;
	u32 infoframe_id;
	union ascend_hdmi_infoframe_unit infoframe;
};

struct ascend_drm_hdmi_property_payload {
	td_bool enable_hdmi;
	td_bool enable_video;
	u32 disp_fmt;
	u32 video_timing;
	u32 pix_clk;
	u32 in_color_space;
	u32 out_color_space;
	u32 deep_color_mode;
	u32 out_csc_quantization;
	td_bool enable_audio;
	u32 sample_rate;
	u32 bit_depth;
	td_bool enable_avi_infoframe;
	td_bool enable_aud_infoframe;
	u32 hdmi_action;
	td_bool enable_vid_mode_adapt;
	td_bool enable_deep_clr_adapt;
	td_bool auth_mode;
};

struct ascend_drm_hdmi_property {
	u32 hdmi_id;
	struct ascend_drm_hdmi_property_payload prop;
};

struct ascend_drm_hdmi_open {
	u32 hdmi_id;
	u32 default_mode;
};

int hdmi_set_reg(td_u32 id, td_char *reg);
int hdmi_set_phy(td_u32 id, char *phy);
int drm_hdmi_open(void *arg, td_bool user);
int drm_hdmi_set_attr(void *arg, td_bool user);
int drm_hdmi_set_infoframe(void *arg, td_bool user);
int drm_hdmi_start(void *arg, td_bool user);
void drm_hdmi_update_edid(struct pppoe_tag *tag);
void drv_hdmi_phy_product_power_set_param(void);
int drm_hdmi_close(void *arg, td_bool user);
int drm_hdmi_format_to_hdmi_timing(td_u32 format);
int drm_hdmi_get_timing_from_mode(td_u32 vact, td_u32 hact,
				  td_u32 crtc_refresh_rate,
				  td_u32 *hdmi_intf_sync,
				  td_u32 *hdmi_pix_freq);
int drm_hdmi_get_connect_status(td_u32 hdmi_id);

#endif
