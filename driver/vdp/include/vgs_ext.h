/*
 * vgs_ext.h - VGS (Video Graphics Scaler) module external interface
 *
 * Derived from IDA Pro reverse engineering of ascend_vdp_hifb.ko and
 * source code analysis of gfbg_rotate_vgs.c.
 *
 * The VGS module provides rotation/scaling services. This header defines
 * the export function table that gfbg queries via func_entry(vgs_export_func, OT_ID_VGS).
 */
#ifndef VGS_EXT_H
#define VGS_EXT_H

#include "ot_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* VGS job priority - confirmed from source: VGS_JOB_PRI_HIGH used in gfbg_rotate_vgs.c */
typedef enum {
    VGS_JOB_PRI_LOW = 0,
    VGS_JOB_PRI_NORMAL,
    VGS_JOB_PRI_HIGH,
    VGS_JOB_PRI_BUTT,
} vgs_job_pri;

/* VGS job type - confirmed from source: VGS_JOB_TYPE_NORMAL used in gfbg_rotate_vgs.c */
typedef enum {
    VGS_JOB_TYPE_NORMAL = 0,
    VGS_JOB_TYPE_BUTT,
} vgs_job_type;

/* VGS job finish status - confirmed from gfbg_main.c:5642 */
typedef enum {
    VGS_JOB_FNSH_STAT_OK = 0,
    VGS_JOB_FNSH_STAT_FAIL,
    VGS_JOB_FNSH_STAT_BUTT,
} vgs_job_fnsh_stat;

/* Forward declaration */
struct ot_vgs_job_data;

/*
 * VGS job data - confirmed from source usage:
 *   job_data.job_type = VGS_JOB_TYPE_NORMAL
 *   job_data.job_call_back = gfbg_drv_get_vgs_rotate_callback()
 *   job_data.private_data[1] = vgs_handle
 */
typedef struct ot_vgs_job_data {
    vgs_job_type job_type;
    td_void (*job_call_back)(ot_mod_id, td_u32, td_u32, struct ot_vgs_job_data *);
    vgs_job_fnsh_stat job_finish_stat;
    td_u64 private_data[4];
} vgs_job_data;

/*
 * VGS task data - opaque structure, used as pointer only
 * Size confirmed via memset_s(task, sizeof(vgs_task_data), 0, sizeof(vgs_task_data))
 * Contains at minimum: img_in (ot_video_frame_info) and img_out (ot_video_frame_info)
 */
typedef struct {
    ot_video_frame_info img_in;
    ot_video_frame_info img_out;
    td_u8 reserved[256]; /* padding - exact size unknown */
} vgs_task_data;

/*
 * VGS export function table - confirmed from gfbg_rotate_vgs.c:
 *   vgs_func->pfn_vgs_begin_job()
 *   vgs_func->pfn_vgs_get_free_task()
 *   vgs_func->pfn_vgs_put_free_task()
 *   vgs_func->pfn_vgs_add_rotation_task()
 *   vgs_func->pfn_vgs_cancel_job()
 *   vgs_func->pfn_vgs_end_job()
 */
typedef struct {
    td_s32 (*pfn_vgs_begin_job)(ot_vgs_handle *handle, vgs_job_pri pri,
                                 ot_mpp_chn *chn, vgs_job_data *job_data);
    vgs_task_data *(*pfn_vgs_get_free_task)(td_void);
    td_void (*pfn_vgs_put_free_task)(vgs_task_data *task);
    td_s32 (*pfn_vgs_add_rotation_task)(ot_vgs_handle handle,
                                         vgs_task_data *task, td_u32 rotate_mode);
    td_s32 (*pfn_vgs_cancel_job)(ot_vgs_handle handle);
    td_s32 (*pfn_vgs_end_job)(ot_vgs_handle handle, td_bool sync,
                               vgs_job_data *job_data);
} vgs_export_func;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* VGS_EXT_H */
