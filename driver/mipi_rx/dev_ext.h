/*
* Copyright (c) Hisilicon Technologies Co., Ltd. 2011-2019. All rights reserved.
* Description:
* Author: Hisilicon multimedia software group
* Create: 2011/06/28
*/

#ifndef _DEV_EXT_H_
#define _DEV_EXT_H_

#include "hi_type.h"

#define UMAP_DEV_NUM                64
#define UMAP_NAME_MAXLEN            32

#define UMAP_VENC_MINOR_BASE        2
#define UMAP_VDEC_MINOR_BASE        3
#define UMAP_JPEGD_MINOR_BASE       4

#define UMAP_VI_MINOR_BASE          1
#define UMAP_ISP_MINOR_BASE         16

#define UMAP_VO_MINOR_BASE          18
#define UMAP_HDMI_MINOR_BASE        19

#define UMAP_AI_MINOR_BASE          6
#define UMAP_AO_MINOR_BASE          7
#define UMAP_AIO_MINOR_BASE         13
#define UMAP_AENC_MINOR_BASE        14
#define UMAP_ADEC_MINOR_BASE        15

#define UMAP_SYS_MINOR_BASE         8
#define UMAP_VB_MINOR_BASE          9
#define UMAP_VPSS_MINOR_BASE        11


#define UMAP_LOG_MINOR_BASE         12
#define UMAP_RGN_MINOR_BASE         17
#define UMAP_VGS_MINOR_BASE         20
#define UMAP_RC_MINOR_BASE          21
#define UMAP_H264E_MINOR_BASE       22
#define UMAP_H265E_MINOR_BASE       23
#define UMAP_JPEGE_MINOR_BASE       24
#define UMAP_CHNL_MINOR_BASE        25
#define UMAP_GDC_MINOR_BASE         28
#define UMAP_VPU_MINOR_BASE         32
#define UMAP_VPC_MINOR_BASE         43
#define UMAP_PNGD_MINOR_BASE        44

#define UMAP_TDE_MINOR_BASE         60
#define UMAP_MIPI_RX_MINOR_BASE     61
#define UMAP_MIPI_TX_MINOR_BASE     62

#define UMAP_MSENSOR_CHIP_MINOR_BASE 64
#define UMAP_MSENSOR_MNG_MINOR_BASE 65
#define UMAP_MFUSION_MINOR_BASE 66

#define UMAP_GET_CHN(f) (*((hi_s32*)(f)))
#define UMAP_SET_CHN(f, chn) (*((hi_s32*)(f)) = (chn))

#define UMAP_DEVNAME_SYSCTL         MPP_MOD_SYS
#define UMAP_DEVNAME_LOG_BASE       MPP_MOD_LOG
#define UMAP_DEVNAME_VENC_BASE      MPP_MOD_VENC
#define UMAP_DEVNAME_VDEC_BASE      MPP_MOD_VDEC
#define UMAP_DEVNAME_JPEGD_BASE     MPP_MOD_JPEGD
#define UMAP_DEVNAME_VPC_BASE       MPP_MOD_VPC
#define UMAP_DEVNAME_VB_BASE        MPP_MOD_VB
#define UMAP_DEVNAME_H264E_BASE     MPP_MOD_H264E
#define UMAP_DEVNAME_H265E_BASE     MPP_MOD_H265E
#define UMAP_DEVNAME_JPEGE_BASE     MPP_MOD_JPEGE
#define UMAP_DEVNAME_CHNL_BASE      MPP_MOD_CHNL
#define UMAP_DEVNAME_RC_BASE        MPP_MOD_RC
#define UMAP_DEVNAME_VPU_BASE       MPP_MOD_VPU
#define UMAP_DEVNAME_PNGD_BASE      MPP_MOD_PNGD
#define UMAP_DEVNAME_VGS_BASE       MPP_MOD_VGS
#define UMAP_DEVNAME_VI_BASE        MPP_MOD_VI
#define UMAP_DEVNAME_VPSS_BASE      MPP_MOD_VPSS
#define UMAP_DEVNAME_GDC_BASE       MPP_MOD_GDC
#define UMAP_DEVNAME_DIS_BASE       MPP_MOD_DIS
#define UMAP_DEVNAME_GYRODIS_BASE   MPP_MOD_GYRODIS

#define UMAP_DEVNAME_ACODEC_BASE    MPP_MOD_ACODEC
#define UMAP_DEVNAME_AIO_BASE       MPP_MOD_AIO
#define UMAP_DEVNAME_AI_BASE        MPP_MOD_AI
#define UMAP_DEVNAME_AO_BASE        MPP_MOD_AO
#define UMAP_DEVNAME_AENC_BASE      MPP_MOD_AENC
#define UMAP_DEVNAME_ADEC_BASE      MPP_MOD_ADEC

#define UMAP_DEVNAME_VO_BASE        MPP_MOD_VO
#define UMAP_DEVNAME_HDMI_BASE      MPP_MOD_HDMI
#define UMAP_DEVNAME_MFUSION_BASE   MPP_MOD_MFUSION

#define UMAP_DEVNAME_RGN_BASE       MPP_MOD_RGN

#endif /* _DEV_EXT_H_ */
