/*
* Copyright (c) Hisilicon Technologies Co., Ltd. 2011-2019. All rights reserved.
* Description:
* Author: Hisilicon multimedia software group
* Create: 2011/06/28
*/

#ifndef __PROC_EXT_H__
#define __PROC_EXT_H__

#include "hi_osal.h"

#include "hi_type.h"
#include "hi_common.h"

#define PROC_ENTRY_VENC      MPP_MOD_VENC
#define PROC_ENTRY_VENC_REG0 MPP_MOD_VENC_REG0
#define PROC_ENTRY_VENC_REG1 MPP_MOD_VENC_REG1
#define PROC_ENTRY_VENC_REG2 MPP_MOD_VENC_REG2
#define PROC_ENTRY_VDEC      MPP_MOD_VDEC
#define PROC_ENTRY_H264E     MPP_MOD_H264E
#define PROC_ENTRY_H265E     MPP_MOD_H265E
#define PROC_ENTRY_H264D     MPP_MOD_H264D
#define PROC_ENTRY_JPEGE     MPP_MOD_JPEGE
#define PROC_ENTRY_JPEGD     MPP_MOD_JPEGD
#define PROC_ENTRY_VPC       MPP_MOD_VPC
#define PROC_ENTRY_DRV_VPC   MPP_MOD_DRV_VPC
#define PROC_ENTRY_CHNL      MPP_MOD_CHNL
#define PROC_ENTRY_VB        MPP_MOD_VB
#define PROC_ENTRY_SYS       MPP_MOD_SYS
#define PROC_ENTRY_LOG       MPP_MOD_LOG
#define PROC_ENTRY_RC        MPP_MOD_RC
#define PROC_ENTRY_PNGD      MPP_MOD_PNGD
#define PROC_ENTRY_DCC       MPP_MOD_DCC
#define PROC_ENTRY_VDEC_ADAPT MPP_MOD_VDEC_ADAPT

#define PROC_ENTRY_VI        MPP_MOD_VI
#define PROC_ENTRY_VPSS      MPP_MOD_VPSS
#define PROC_ENTRY_ISP       MPP_MOD_ISP
#define PROC_ENTRY_GDC       MPP_MOD_GDC
#define PROC_ENTRY_VGS       MPP_MOD_VGS

#define PROC_ENTRY_DIS       MPP_MOD_DIS
#define PROC_ENTRY_GYRODIS   MPP_MOD_GYRODIS

#define PROC_ENTRY_VO        MPP_MOD_VO
#define PROC_ENTRY_HDMI      MPP_MOD_HDMI

#define PROC_ENTRY_AIO       MPP_MOD_AIO
#define PROC_ENTRY_AI        MPP_MOD_AI
#define PROC_ENTRY_AO        MPP_MOD_AO
#define PROC_ENTRY_AENC      MPP_MOD_AENC
#define PROC_ENTRY_ADEC      MPP_MOD_ADEC
#define PROC_ENTRY_ACODEC    MPP_MOD_ACODEC
#define PROC_ENTRY_RGN       MPP_MOD_RGN
#define PROC_ENTRY_MFUSION   MPP_MOD_MFUSION

typedef hi_s32 (*CMPI_PROC_READ)(struct osal_proc_dir_entry *entry);
typedef hi_s32 (*CMPI_PROC_WRITE)(char *buf, int count);

#ifdef __LITEOS__
static inline hi_s32 cmpi_log_init(hi_u32 log_buf_len)
{
    HI_UNUSED(log_buf_len);
    return 0;
}

static inline hi_void cmpi_log_exit(hi_void)
{
}
#else
hi_s32 cmpi_log_init(hi_u32 log_buf_len);
hi_void cmpi_log_exit(hi_void);
#endif // __LITEOS__

#endif
