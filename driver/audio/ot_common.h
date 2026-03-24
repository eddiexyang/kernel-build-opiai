/* Stub ot_common.h - HiSilicon OT SDK common definitions */
#ifndef __OT_COMMON_H__
#define __OT_COMMON_H__

#include "ot_type.h"

typedef enum {
    OT_ID_CMPI = 0, OT_ID_VB, OT_ID_SYS, OT_ID_RGN, OT_ID_CHNL,
    OT_ID_VDEC, OT_ID_GRP, OT_ID_VPSS, OT_ID_VENC, OT_ID_H264E,
    OT_ID_JPEGE, OT_ID_H265E, OT_ID_JPEGD, OT_ID_VO, OT_ID_VI,
    OT_ID_DIS, OT_ID_RC, OT_ID_AIO, OT_ID_AI, OT_ID_AO,
    OT_ID_AENC, OT_ID_ADEC, OT_ID_VGS, OT_ID_ISP, OT_ID_IVE,
    OT_ID_FB, OT_ID_TDE, OT_ID_HDMI, OT_ID_MIPI_TX, OT_ID_MIPI_RX,
    OT_ID_PNGD, OT_ID_ACODEC,
    OT_ID_MAX
} ot_mod_id;

#define OT_MAX_DEV_NUM     16
#define OT_MAX_CHN_NUM     64
#define OT_ALIGN_UP(x, a)  (((x) + ((a) - 1)) & (~((a) - 1)))
#define OT_ALIGN_DOWN(x,a) ((x) & (~((a) - 1)))

typedef struct {
    ot_mod_id mod_id;
    td_s32    dev_id;
    td_s32    chn_id;
} ot_mpp_chn;

#define ot_unused(x) ((void)(x))

/* Trace macros - map to kernel printk */
#define ot_trace(level, mod, fmt, ...) printk(fmt, ##__VA_ARGS__)

#endif
