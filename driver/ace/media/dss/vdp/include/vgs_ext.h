#ifndef OT_VGS_EXT_H
#define OT_VGS_EXT_H

#include "ot_common.h"
#include "gfbg.h"
#include "../../../../dvpp/dvpp/HiDvpp/cbb/include/adapt/hi_common_adapt.h"

typedef td_s32 ot_vgs_handle;

typedef enum {
    OT_PIXEL_FORMAT_ARGB_1555 = 0,
    OT_PIXEL_FORMAT_ARGB_4444 = 1,
    OT_PIXEL_FORMAT_BUTT = 0xffff
} ot_pixel_format;

typedef enum {
    OT_VIDEO_FIELD_FRAME = 0,
} ot_video_field;

typedef enum {
    OT_VIDEO_FORMAT_LINEAR = 0,
} ot_video_format;

typedef enum {
    OT_COMPRESS_MODE_NONE = 0,
} ot_compress_mode;

typedef enum {
    VGS_JOB_TYPE_NORMAL = 0,
} vgs_job_type;

typedef enum {
    VGS_JOB_FNSH_STAT_OK = 0,
} vgs_job_finish_status;

typedef struct {
    td_u32 width;
    td_u32 height;
    td_u32 stride[2];
    td_u64 phys_addr[2];
    td_s32 pixel_format;
    td_s32 field;
    td_s32 video_format;
    td_s32 compress_mode;
} ot_video_frame;

typedef struct {
    ot_video_frame video_frame;
    td_u32 pool_id;
    td_s32 mod_id;
} ot_video_frame_info;

typedef hi_mpp_chn ot_mpp_chn;

typedef struct ot_vgs_job_data {
    td_s32 job_type;
    td_s32 job_finish_stat;
    td_void (*job_call_back)(ot_mod_id, td_u32, td_u32, struct ot_vgs_job_data *);
    td_u32 private_data[2];
} ot_vgs_job_data;

typedef ot_vgs_job_data vgs_job_data;

typedef struct {
    ot_video_frame_info img_in;
    ot_video_frame_info img_out;
    td_void *call_back;
    td_s32 call_mod_id;
    td_u32 call_dev_id;
    td_u32 call_chn_id;
    td_s32 finish_stat;
} vgs_task_data;

typedef struct {
    td_s32 (*pfn_vgs_begin_job)(ot_vgs_handle *handle, td_u32 pri, const ot_mpp_chn *chn,
        vgs_job_data *job_data);
    vgs_task_data *(*pfn_vgs_get_free_task)(td_void);
    td_s32 (*pfn_vgs_add_rotation_task)(ot_vgs_handle handle, vgs_task_data *task, td_u32 rotate_mode);
    td_s32 (*pfn_vgs_cancel_job)(ot_vgs_handle handle);
    td_s32 (*pfn_vgs_end_job)(ot_vgs_handle handle, td_bool block, vgs_job_data *job_data);
    td_void (*pfn_vgs_put_free_task)(vgs_task_data *task);
} vgs_export_func;

#endif
