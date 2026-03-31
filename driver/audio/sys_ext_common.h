/*
 * Copyright (C) Hisilicon Technologies Co., Ltd. 2022-2025. All rights reserved.
 * Description: The internal interface for other modules
 * Author: Hisilicon multimedia software group
 * Create: 2022-1-30
 */
#ifndef SYS_EXT_COMMON_H__
#define SYS_EXT_COMMON_H__
#include "mkp_ioctl.h"
#include "hi_comm_sys_adapt.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#define SYS_CHECK_NULL_PTR(ptr)                        \
    do {                                               \
        if ((ptr) == NULL) {                           \
            HI_TRACE_SYS(HI_DBG_ERR, "Null point \n"); \
            return HI_ERR_SYS_NULL_PTR;                \
        }                                              \
    } while (0)

/* cur sender:VIU,VOU,VDEC,VPSS,AI
 * cur receive:VOU,VPSS,GRP,AO
 */
#define BIND_ADJUST_SRC_DEVID(mod_id, dev_id) \
    do {                                         \
        if ((mod_id) == HI_ID_VDEC) {           \
            (dev_id) = 0;                      \
        }                                        \
    } while (0)

#define BIND_ADJUST_SRC_CHNID(mod_id, chn_id) \
    do {                                         \
        if ((mod_id) == HI_ID_VO) {             \
            (chn_id) = 0;                      \
        }                                        \
    } while (0)

#define BIND_ADJUST_DEST_DEVID(mod_id, dev_id) \
    do {                                          \
        if ((mod_id) == HI_ID_VENC) {            \
            (dev_id) = 0;                       \
        }                                         \
    } while (0)

typedef enum {
    SYS_CHIP_DC = 0,
    SYS_CHIP_MDC_V1,
    SYS_CHIP_MDC_V2,
    SYS_CHIP_MDC_V51_LITE,
    SYS_CHIP_CLOUD_V2,
    SYS_CHIP_MINI_V2,
    SYS_CHIP_MINI_V2_MDC,
    SYS_CHIP_BUTT
} sys_chip_type;

typedef struct {
    hi_mpp_chn src_chn;
    hi_mpp_chn dest_chn;
} sys_bind_args;

typedef struct {
    hi_mpp_chn src_chn;
    hi_mpp_bind_dest dest_chns;
} sys_bind_src_args;

typedef struct {
    hi_mpp_chn mpp_chn;
    hi_char mmz_name[MAX_MMZ_NAME_LEN];
} sys_mem_args;

typedef struct {
    hi_s32 hr_timer;
    hi_s32 rr_mode;
} kernel_config;

typedef struct {
    hi_u32 size;
    hi_u64 phy_addr;
    hi_void ATTRIBUTE *vir_addr;
} sys_mem_cache_info;

typedef struct {
    hi_vpss_venc_wrap_param wrap_param;
    hi_u32 buf_line;
} vpss_venc_wrap_args;

typedef enum {
    IOC_NR_SYS_INIT = 0,
    IOC_NR_SYS_EXIT,
    IOC_NR_SYS_SETCONFIG,
    IOC_NR_SYS_GETCONFIG,
    IOC_NR_SYS_INITPTSBASE,
    IOC_NR_SYS_SYNCPTS,
    IOC_NR_SYS_GETCURPTS,
    IOC_NR_SYS_DUMP_INFO,

    IOC_NR_SYS_BIND,
    IOC_NR_SYS_UNBIND,
    IOC_NR_SYS_GETBINDBYDEST,
    IOC_NR_SYS_GETBINDBYSRC,

    IOC_NR_SYS_MEM_SET,
    IOC_NR_SYS_MEM_GET,

    IOC_NR_SYS_GET_CUST_CODE,

    IOC_NR_SYS_GET_KERNELCONFIG,

    IOC_NR_SYS_GET_CHIPID,

    IOC_NR_SYS_SET_VIVPSS_MODE,
    IOC_NR_SYS_GET_VIVPSS_MODE,

    IOC_NR_SYS_SET_TUNING_CONNECT,
    IOC_NR_SYS_GET_TUNING_CONNECT,

    IOC_NR_SYS_MFLUSH_CACHE,

    IOC_NR_SYS_SET_TIME_ZONE,
    IOC_NR_SYS_GET_TIME_ZONE,

    IOC_NR_SYS_SET_GPS_INFO,
    IOC_NR_SYS_GET_GPS_INFO,

    IOC_NR_SYS_GET_VPSSVENC_WRAP_BUF_LINE,

    IOC_NR_SYS_SET_RAW_FRAME_COMPRESS_RATE,
    IOC_NR_SYS_GET_RAW_FRAME_COMPRESS_RATE,

    IOC_NR_SYS_SET_CSC_MATRIX,
    IOC_NR_SYS_GET_CSC_MATRIX,
    IOC_NR_SYS_GET_CHIP_TYPE,
} ioc_nr_sys;

typedef enum {
    MPP_DATA_VI_FRAME,
    MPP_DATA_VOU_FRAME,
    MPP_DATA_VDEC_FRAME,
    MPP_DATA_VIDEO_FRAME,
    MPP_DATA_VOU_WBC_FRAME,
    MPP_DATA_AUDIO_FRAME,
    MPP_DATA_VPSS_FRAME,
    MPP_DATA_DPU_RECT_FRAME,
    MPP_DATA_AVS_FRAME,
    MPP_DATA_MCF_FRAME,
    MPP_DATA_BUTT
} mpp_data_type;

typedef struct {
    hi_mod_id mod_id;
    hi_u32 max_dev_cnt;
    hi_u32 max_chn_cnt;
    hi_s32 (*give_bind_call_back)(hi_s32 dev_id, hi_s32 chn_id, hi_mpp_bind_dest *bind_send);
} bind_sender_info;

typedef struct {
    hi_mod_id mod_id;
    hi_u32 max_dev_cnt;
    hi_u32 max_chn_cnt;
    hi_s32 (*call_back)(hi_s32 dev_id, hi_s32 chn_id, hi_bool block, mpp_data_type data_type, hi_void *pv_data);
    hi_s32 (*reset_call_back)(hi_s32 dev_id, hi_s32 chn_id, hi_void *pv_data);
    hi_bool support_delay_data;
} bind_receiver_info;

#define SYS_INIT_CTRL                   _IO(IOC_TYPE_SYS, IOC_NR_SYS_INIT)
#define SYS_EXIT_CTRL                   _IO(IOC_TYPE_SYS, IOC_NR_SYS_EXIT)
#define SYS_SET_CONFIG_CTRL             _IOW(IOC_TYPE_SYS, IOC_NR_SYS_SETCONFIG, hi_mpp_sys_config)
#define SYS_GET_CONFIG_CTRL             _IOR(IOC_TYPE_SYS, IOC_NR_SYS_GETCONFIG, hi_mpp_sys_config)
#define SYS_INIT_PTSBASE                _IOW(IOC_TYPE_SYS, IOC_NR_SYS_INITPTSBASE, hi_u64)
#define SYS_SYNC_PTS                    _IOW(IOC_TYPE_SYS, IOC_NR_SYS_SYNCPTS, hi_u64)
#define SYS_GET_CURPTS                  _IOR(IOC_TYPE_SYS, IOC_NR_SYS_GETCURPTS, hi_u64)

#define SYS_BIND_CTRL                   _IOW(IOC_TYPE_SYS, IOC_NR_SYS_BIND, sys_bind_args)
#define SYS_UNBIND_CTRL                 _IOW(IOC_TYPE_SYS, IOC_NR_SYS_UNBIND, sys_bind_args)
#define SYS_GETBINDBYDEST               _IOWR(IOC_TYPE_SYS, IOC_NR_SYS_GETBINDBYDEST, sys_bind_args)
#define SYS_GETBINDBYSRC                _IOWR(IOC_TYPE_SYS, IOC_NR_SYS_GETBINDBYSRC, sys_bind_src_args)

#define SYS_GET_CUST_CODE               _IOR(IOC_TYPE_SYS, IOC_NR_SYS_GET_CUST_CODE, hi_u32)

#define SYS_GET_KERNELCONFIG            _IOR(IOC_TYPE_SYS, IOC_NR_SYS_GET_KERNELCONFIG, kernel_config)

#define SYS_SET_TUNING_CONNECT          _IOW(IOC_TYPE_SYS, IOC_NR_SYS_SET_TUNING_CONNECT, hi_s32)
#define SYS_GET_TUNING_CONNECT          _IOR(IOC_TYPE_SYS, IOC_NR_SYS_GET_TUNING_CONNECT, hi_s32)

#define SYS_MFLUSH_CACHE                _IOW(IOC_TYPE_SYS, IOC_NR_SYS_MFLUSH_CACHE, sys_mem_cache_info)

#define SYS_SET_TIME_ZONE               _IOW(IOC_TYPE_SYS, IOC_NR_SYS_SET_TIME_ZONE, hi_s32)
#define SYS_GET_TIME_ZONE               _IOR(IOC_TYPE_SYS, IOC_NR_SYS_GET_TIME_ZONE, hi_s32)

#define SYS_SET_GPS_INFO                _IOW(IOC_TYPE_SYS, IOC_NR_SYS_SET_GPS_INFO, hi_gps_info)
#define SYS_GET_GPS_INFO                _IOR(IOC_TYPE_SYS, IOC_NR_SYS_GET_GPS_INFO, hi_gps_info)

#define SYS_SET_CSC_MATRIX              _IOW(IOC_TYPE_SYS, IOC_NR_SYS_SET_CSC_MATRIX, hi_csc_param)
#define SYS_GET_CSC_MATRIX              _IOWR(IOC_TYPE_SYS, IOC_NR_SYS_GET_CSC_MATRIX, hi_csc_param)

#define SYS_GET_CHIP_TYPE               _IOR(IOC_TYPE_SYS, IOC_NR_SYS_GET_CHIP_TYPE, hi_u32)

#define SYS_GET_VPSSVENC_WRAP_BUF_LINE  _IOWR(IOC_TYPE_SYS, \
    IOC_NR_SYS_GET_VPSSVENC_WRAP_BUF_LINE, vpss_venc_wrap_args)

#define SYS_SET_RAW_FRAME_COMPRESS_RATE _IOW(IOC_TYPE_SYS, IOC_NR_SYS_SET_RAW_FRAME_COMPRESS_RATE, \
    hi_raw_frame_compress_param)
#define SYS_GET_RAW_FRAME_COMPRESS_RATE _IOR(IOC_TYPE_SYS, IOC_NR_SYS_GET_RAW_FRAME_COMPRESS_RATE, \
    hi_raw_frame_compress_param)

#ifdef HI_DEBUG
#define SYS_SET_COMPRESS_RATE           _IOW(IOC_TYPE_SYS, IOC_NR_SYS_SET_COMPRESS_RATE, sys_compress_param)
#define SYS_GET_COMPRESS_RATE           _IOR(IOC_TYPE_SYS, IOC_NR_SYS_GET_COMPRESS_RATE, sys_compress_param)

#define SYS_SET_COMPRESSV2_RATE         _IOW(IOC_TYPE_SYS, IOC_NR_SYS_SET_COMPRESSV2_RATE, sys_compress_v2_param)
#define SYS_GET_COMPRESSV2_RATE         _IOR(IOC_TYPE_SYS, IOC_NR_SYS_GET_COMPRESSV2_RATE, sys_compress_v2_param)
#endif

#define SYS_DUMP_INFO                   _IOW(IOC_TYPE_SYS, IOC_NR_SYS_DUMP_INFO, hi_mod_id)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* SYS_EXT_COMMON_H__ */
