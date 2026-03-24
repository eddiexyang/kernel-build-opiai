/*
* Copyright (c) Huawei Technologies Co., Ltd. 2021-2022. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 and
* only version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* Description:
* Author: huawei
* Create: 2021-09-06
*/

#ifndef __VIRTMNGDEV_SOC_RESOURCE_H__
#define __VIRTMNGDEV_SOC_RESOURCE_H__

#include <linux/mutex.h>
#include "icm_interface.h"
#include "virtmng_interface.h"

#define VMNGD_ICM_MBX_CHAN  HISI_RPROC_TX_TS_MBX2
#define VMNGD_VDEV_INIT_STATUS  VMNG_VDEV_STATUS_FREE

/* Element flag */
#define ATTR_CREATE_CFG (1U << 1)        /* Whether to specified configuration when create vf */
#define ATTR_UPDATE_CFG (1U << 2)        /* Whether to specified configuration when update vf */
#define ATTR_VFG_SHARE (1U << 3)         /* Whether share in vfg */
#define ATTR_NECESSARY (1U << 4)         /* At least has one unit resource */

#define TOKEN_VAL 0x5A5A5A5AULL
#define TOKEN_MAX 0x15A5A5A5AULL
#define TIMEOUT_TOKEN_VAL 0x25A5A5A5AULL

#define VMNG_DTYPE_RANGE_FROM VMNG_HW_TYPE_C1
#define VMNG_DTYPE_RANGE_TO   VMNG_HW_TYPE_C24

/*
 * VF resource configuration
 */
/* aiv */
#define AC_AIV_ATTR (ATTR_CREATE_CFG | ATTR_UPDATE_CFG | ATTR_VFG_SHARE)
#define AC_AIV_NUM 50ULL
#define AC_AIV_NUM_PER_BIT 1U
#define AC_AIV_BITMAPNUM 50U
#define AC_AIV_BITMAP 0x3FFFFFFFFFFFFUL
#define AC_AIV_DIVIDE_1 48U /* Divide into 1 */
#define AC_AIV_DIVIDE_2 24U /* Divide into 1/2 */
#define AC_AIV_DIVIDE_4 12U /* Divide into 1/4 */
#define AC_AIV_DIVIDE_6 8U  /* Divide into 1/6 */
#define AC_AIV_DIVIDE_12 4U /* Divide into 1/12 */

/* aic */
#define AC_AIC_ATTR (ATTR_CREATE_CFG | ATTR_VFG_SHARE)
#define AC_AIC_NUM 25ULL
#define AC_AIC_NUM_PER_BIT 1U
#define AC_AIC_BITMAPNUM 25U
#define AC_AIC_BITMAP 0x1FFFFFFUL
#define AC_AIC_DIVIDE_1 24U /* Divide into 1 */
#define AC_AIC_DIVIDE_2 12U /* Divide into 1/2 */
#define AC_AIC_DIVIDE_4 6U  /* Divide into 1/4 */
#define AC_AIC_DIVIDE_6 4U  /* Divide into 1/6 */
#define AC_AIC_DIVIDE_12 2U /* Divide into 1/12 */

/* c_core */
#define AC_C_CORE_ATTR (ATTR_VFG_SHARE | ATTR_NECESSARY)
#define AC_C_CORE_NUM 8ULL
#define AC_C_CORE_NUM_PER_BIT 1U
#define AC_C_CORE_BITMAPNUM 8U
#define AC_C_CORE_BITMAP 0xFFUL
#define AC_C_CORE_DIVIDE_1 8U  /* Divide into 1 */
#define AC_C_CORE_DIVIDE_2 4U  /* Divide into 1/2 */
#define AC_C_CORE_DIVIDE_4 2U  /* Divide into 1/4 */
#define AC_C_CORE_DIVIDE_6 1U  /* Divide into 1/6 */
#define AC_C_CORE_DIVIDE_12 0U /* Divide into 1/12 */

/* dsa */
#define AC_DSA_ATTR (ATTR_UPDATE_CFG | ATTR_UPDATE_CFG | ATTR_VFG_SHARE | ATTR_NECESSARY)
#define AC_DSA_NUM 8ULL
#define AC_DSA_NUM_PER_BIT 1U
#define AC_DSA_BITMAPNUM 8U
#define AC_DSA_BITMAP 0xFFUL
#define AC_DSA_DIVIDE_1 8U  /* Divide into 1 */
#define AC_DSA_DIVIDE_2 4U  /* Divide into 1/2 */
#define AC_DSA_DIVIDE_4 2U  /* Divide into 1/4 */
#define AC_DSA_DIVIDE_6 1U  /* Divide into 1/6 */
#define AC_DSA_DIVIDE_12 0U /* Divide into 1/12 */

/* ffts */
#define AC_FFTS_ATTR (ATTR_VFG_SHARE | ATTR_NECESSARY)
#define AC_FFTS_NUM 8ULL
#define AC_FFTS_NUM_PER_BIT 1U
#define AC_FFTS_BITMAPNUM 8U
#define AC_FFTS_BITMAP 0xFFUL
#define AC_FFTS_DIVIDE_1 1U  /* Divide into 1, each pool 1 */
#define AC_FFTS_DIVIDE_2 1U  /* Divide into 1/2 */
#define AC_FFTS_DIVIDE_4 1U  /* Divide into 1/4 */
#define AC_FFTS_DIVIDE_6 1U  /* Divide into 1/6 */
#define AC_FFTS_DIVIDE_12 0U /* Divide into 1/12 */

/* sdma */
#define AC_SDMA_ATTR (ATTR_UPDATE_CFG | ATTR_VFG_SHARE)
#define AC_SDMA_NUM 32ULL
#define AC_SDMA_NUM_PER_BIT 1U
#define AC_SDMA_BITMAPNUM 32U
#define AC_SDMA_BITMAP 0xFFFFFFFFUL
#define AC_SDMA_DIVIDE_1 32U /* Divide into 1 */
#define AC_SDMA_DIVIDE_2 16U /* Divide into 1/2 */
#define AC_SDMA_DIVIDE_4 8U  /* Divide into 1/4 */
#define AC_SDMA_DIVIDE_6 5U  /* Divide into 1/6 */
#define AC_SDMA_DIVIDE_12 2U /* Divide into 1/12 */

/* stars pcie_dma totally 12, 13-24 for stars (0-12 & 25-37 for pcie) */
#define AC_PCIE_DMA_ATTR (ATTR_CREATE_CFG | ATTR_VFG_SHARE)
#define AC_PCIE_DMA_NUM 12ULL
#define AC_PCIE_DMA_NUM_PER_BIT 1U
#define AC_PCIE_DMA_BITMAPNUM 25U
#define AC_PCIE_DMA_BITMAP 0x1FFE000UL
#define AC_PCIE_DMA_DIVIDE_1 1U  /* Divide into 1 */
#define AC_PCIE_DMA_DIVIDE_2 1U  /* Divide into 1/2 */
#define AC_PCIE_DMA_DIVIDE_4 1U  /* Divide into 1/4 */
#define AC_PCIE_DMA_DIVIDE_6 1U  /* Divide into 1/6 */
#define AC_PCIE_DMA_DIVIDE_12 1U /* Divide into 1/12 */

/* acsq */
#define AC_ACSQ_ATTR (ATTR_CREATE_CFG | ATTR_VFG_SHARE)
#define AC_ACSQ_NUM 128ULL
#define AC_ACSQ_NUM_PER_BIT 8U
#define AC_ACSQ_BITMAPNUM 16U
#define AC_ACSQ_BITMAP 0xFFFFUL
#define AC_ACSQ_DIVIDE_1 128U /* Divide into 1 */
#define AC_ACSQ_DIVIDE_2 64U  /* Divide into 1/2 */
#define AC_ACSQ_DIVIDE_4 32U  /* Divide into 1/4 */
#define AC_ACSQ_DIVIDE_6 16U  /* Divide into 1/6 */
#define AC_ACSQ_DIVIDE_12 8U  /* Divide into 1/12 */

/* rtsq */
#define AC_RTSQ_ATTR (ATTR_CREATE_CFG)
#define AC_RTSQ_NUM 2048ULL
#define AC_RTSQ_NUM_PER_BIT 128U
#define AC_RTSQ_BITMAPNUM 16U
#define AC_RTSQ_BITMAP 0xFFFFUL
#define AC_RTSQ_DIVIDE_1 2048U /* Divide into 1 */
#define AC_RTSQ_DIVIDE_2 1024U /* Divide into 1/2 */
#define AC_RTSQ_DIVIDE_4 512U  /* Divide into 1/4 */
#define AC_RTSQ_DIVIDE_6 256U  /* Divide into 1/6 */
#define AC_RTSQ_DIVIDE_12 128U /* Divide into 1/12 */

/* event_id */
#define AC_EVENT_ID_ATTR (ATTR_CREATE_CFG)
#define AC_EVENT_ID_NUM 65536ULL
#define AC_EVENT_ID_NUM_PER_BIT 4096U
#define AC_EVENT_ID_BITMAPNUM 16U
#define AC_EVENT_ID_BITMAP 0xFFFFUL
#define AC_EVENT_ID_DIVIDE_1 65536U /* Divide into 1 */
#define AC_EVENT_ID_DIVIDE_2 32768U /* Divide into 1/2 */
#define AC_EVENT_ID_DIVIDE_4 16384U  /* Divide into 1/4 */
#define AC_EVENT_ID_DIVIDE_6 8192U  /* Divide into 1/6 */
#define AC_EVENT_ID_DIVIDE_12 4096U /* Divide into 1/12 */

/* notify_id */
#define AC_NOTIFY_ID_ATTR (ATTR_CREATE_CFG)
#define AC_NOTIFY_ID_NUM 8192ULL
#define AC_NOTIFY_ID_NUM_PER_BIT 512U
#define AC_NOTIFY_ID_BITMAPNUM 16U
#define AC_NOTIFY_ID_BITMAP 0xFFFFUL
#define AC_NOTIFY_ID_DIVIDE_1 8192U /* Divide into 1 */
#define AC_NOTIFY_ID_DIVIDE_2 4096U /* Divide into 1/2 */
#define AC_NOTIFY_ID_DIVIDE_4 2048U /* Divide into 1/4 */
#define AC_NOTIFY_ID_DIVIDE_6 1024U /* Divide into 1/6 */
#define AC_NOTIFY_ID_DIVIDE_12 512U /* Divide into 1/12 */

/* cdqm */
#define AC_CDQM_ATTR (ATTR_CREATE_CFG | ATTR_UPDATE_CFG)
#define AC_CDQM_NUM 128ULL
#define AC_CDQM_NUM_PER_BIT 8U
#define AC_CDQM_BITMAPNUM 16U
#define AC_CDQM_BITMAP 0xFFFFUL
#define AC_CDQM_DIVIDE_1 128U /* Divide into 1 */
#define AC_CDQM_DIVIDE_2 64U  /* Divide into 1/2 */
#define AC_CDQM_DIVIDE_4 32U  /* Divide into 1/4 */
#define AC_CDQM_DIVIDE_6 16U  /* Divide into 1/6 */
#define AC_CDQM_DIVIDE_12 8U  /* Divide into 1/12 */

/* cmo_id */
#define AC_CMO_ID_ATTR (ATTR_CREATE_CFG)
#define AC_CMO_ID_NUM 0ULL
#define AC_CMO_ID_NUM_PER_BIT 1U
#define AC_CMO_ID_BITMAPNUM 0U
#define AC_CMO_ID_BITMAP 0x0ULL
#define AC_CMO_ID_DIVIDE_1 0U /* Divide into 1 */
#define AC_CMO_ID_DIVIDE_2 0U /* Divide into 1/2 */
#define AC_CMO_ID_DIVIDE_4 0U  /* Divide into 1/4 */
#define AC_CMO_ID_DIVIDE_6 0U  /* Divide into 1/6 */
#define AC_CMO_ID_DIVIDE_12 0U /* Divide into 1/12 */

/* memory */
#define AC_MEMORY_ATTR (ATTR_CREATE_CFG | ATTR_UPDATE_CFG)
#define AC_MEMORY_NUM 30UL
#define AC_MEMORY_NUM_PER_BIT 1U
#define AC_MEMORY_BITMAPNUM 32U
#define AC_MEMORY_BITMAP 0xFFFFFFFEUL   // the first node is reserve
#define AC_MEMORY_DIVIDE_1 15U /* Divide into 1, totally 15 node, each 2G */
#define AC_MEMORY_DIVIDE_2 7U  /* Divide into 1/2, legal val only 7,8 */
#define AC_MEMORY_DIVIDE_4 3U  /* Divide into 1/4, legal val only 3,4 */
#define AC_MEMORY_DIVIDE_6 2U  /* Divide into 1/6, legal val only 2,3 */
#define AC_MEMORY_DIVIDE_12 1U /* Divide into 1/12, legal val only 1,2 */

/* jpegd */
#define DVPP_JPEGD_ATTR (ATTR_UPDATE_CFG | ATTR_VFG_SHARE)
#define DVPP_JPEGD_NUM 28ULL
#define DVPP_JPEGD_NUM_PER_BIT 1U
#define DVPP_JPEGD_BITMAPNUM 28U
#define DVPP_JPEGD_BITMAP 0xFFFFFFFUL
#define DVPP_JPEGD_DIVIDE_1 28U /* Divide into 1 */
#define DVPP_JPEGD_DIVIDE_2 14U /* Divide into 1/2 */
#define DVPP_JPEGD_DIVIDE_4 7U  /* Divide into 1/4 */
#define DVPP_JPEGD_DIVIDE_6 4U  /* Divide into 1/6 */
#define DVPP_JPEGD_DIVIDE_12 2U /* Divide into 1/12 */

/* jpege */
#define DVPP_JPEGE_ATTR (ATTR_UPDATE_CFG | ATTR_VFG_SHARE)
#define DVPP_JPEGE_NUM 4ULL
#define DVPP_JPEGE_NUM_PER_BIT 1U
#define DVPP_JPEGE_BITMAPNUM 4U
#define DVPP_JPEGE_BITMAP 0xFUL
#define DVPP_JPEGE_DIVIDE_1 4U  /* Divide into 1 */
#define DVPP_JPEGE_DIVIDE_2 2U  /* Divide into 1/2 */
#define DVPP_JPEGE_DIVIDE_4 1U  /* Divide into 1/4 */
#define DVPP_JPEGE_DIVIDE_6 0U  /* Divide into 1/6 */
#define DVPP_JPEGE_DIVIDE_12 0U /* Divide into 1/12 */

/* vpc */
#define DVPP_VPC_ATTR (ATTR_UPDATE_CFG | ATTR_VFG_SHARE | ATTR_NECESSARY)
#define DVPP_VPC_NUM 10ULL
#define DVPP_VPC_NUM_PER_BIT 1U
#define DVPP_VPC_BITMAPNUM 10U
#define DVPP_VPC_BITMAP 0x3FFUL
#define DVPP_VPC_DIVIDE_1 10U /* Divide into 1 */
#define DVPP_VPC_DIVIDE_2 5U  /* Divide into 1/2 */
#define DVPP_VPC_DIVIDE_4 2U  /* Divide into 1/4 */
#define DVPP_VPC_DIVIDE_6 1U  /* Divide into 1/6 */
#define DVPP_VPC_DIVIDE_12 0U /* Divide into 1/12 */

/* vdec */
#define DVPP_VDEC_ATTR (ATTR_UPDATE_CFG | ATTR_VFG_SHARE)
#define DVPP_VDEC_NUM 2ULL
#define DVPP_VDEC_NUM_PER_BIT 1
#define DVPP_VDEC_BITMAPNUM 2U
#define DVPP_VDEC_BITMAP 0x3UL
#define DVPP_VDEC_DIVIDE_1 2U  /* Divide into 1 */
#define DVPP_VDEC_DIVIDE_2 1U  /* Divide into 1/2 */
#define DVPP_VDEC_DIVIDE_4 0U  /* Divide into 1/4 */
#define DVPP_VDEC_DIVIDE_6 0U  /* Divide into 1/6 */
#define DVPP_VDEC_DIVIDE_12 0U /* Divide into 1/12 */

/* pngd */
#define DVPP_PNGD_ATTR (ATTR_UPDATE_CFG | ATTR_VFG_SHARE)
#define DVPP_PNGD_NUM 0ULL
#define DVPP_PNGD_NUM_PER_BIT 1U
#define DVPP_PNGD_BITMAPNUM 0U
#define DVPP_PNGD_BITMAP 0UL
#define DVPP_PNGD_DIVIDE_1 0U  /* Divide into 1 */
#define DVPP_PNGD_DIVIDE_2 0U  /* Divide into 1/2 */
#define DVPP_PNGD_DIVIDE_4 0U  /* Divide into 1/4 */
#define DVPP_PNGD_DIVIDE_6 0U  /* Divide into 1/6 */
#define DVPP_PNGD_DIVIDE_12 0U /* Divide into 1/12 */

/* venc */
#define DVPP_VENC_ATTR (ATTR_UPDATE_CFG | ATTR_VFG_SHARE)
#define DVPP_VENC_NUM 0ULL
#define DVPP_VENC_NUM_PER_BIT 1U
#define DVPP_VENC_BITMAPNUM 0U
#define DVPP_VENC_BITMAP 0UL
#define DVPP_VENC_DIVIDE_1 0U  /* Divide into 1 */
#define DVPP_VENC_DIVIDE_2 0U  /* Divide into 1/2 */
#define DVPP_VENC_DIVIDE_4 0U  /* Divide into 1/4 */
#define DVPP_VENC_DIVIDE_6 0U  /* Divide into 1/6 */
#define DVPP_VENC_DIVIDE_12 0U /* Divide into 1/12 */

/* topic_aicpu_slot */
#define CPU_TOPIC_AICPU_ATTR (ATTR_UPDATE_CFG | ATTR_VFG_SHARE)
#define CPU_TOPIC_AICPU_NUM 32ULL
#define CPU_TOPIC_AICPU_NUM_PER_BIT 1U
#define CPU_TOPIC_AICPU_BITMAPNUM 32U
#define CPU_TOPIC_AICPU_BITMAP 0xFFFFFFFFUL
#define CPU_TOPIC_AICPU_DIVIDE_1 32U /* Divide into 1 */
#define CPU_TOPIC_AICPU_DIVIDE_2 16U /* Divide into 1/2 */
#define CPU_TOPIC_AICPU_DIVIDE_4 8U  /* Divide into 1/4 */
#define CPU_TOPIC_AICPU_DIVIDE_6 4U  /* Divide into 1/6 */
#define CPU_TOPIC_AICPU_DIVIDE_12 2U /* Divide into 1/12 */

/* topic_ctrl_cpu_slot */
#define CPU_TOPIC_CCPU_ATTR (ATTR_VFG_SHARE)
#define CPU_TOPIC_CCPU_NUM 32ULL
#define CPU_TOPIC_CCPU_NUM_PER_BIT 1U
#define CPU_TOPIC_CCPU_BITMAPNUM 32U
#define CPU_TOPIC_CCPU_BITMAP 0xFFFFFFFFUL
#define CPU_TOPIC_CCPU_DIVIDE_1 4U  /* Divide into 1, each pool 4, totaly 32 */
#define CPU_TOPIC_CCPU_DIVIDE_2 4U  /* Divide into 1/2 */
#define CPU_TOPIC_CCPU_DIVIDE_4 4U  /* Divide into 1/4 */
#define CPU_TOPIC_CCPU_DIVIDE_6 4U  /* Divide into 1/6 */
#define CPU_TOPIC_CCPU_DIVIDE_12 2U /* Divide into 1/12 */

/* host_ctrl_cpu */
#define CPU_HOST_CCPU_ATTR (ATTR_UPDATE_CFG)
#define CPU_HOST_CCPU_NUM 16ULL
#define CPU_HOST_CCPU_NUM_PER_BIT 1U
#define CPU_HOST_CCPU_BITMAPNUM 16U
#define CPU_HOST_CCPU_BITMAP 0xFFFFUL
#define CPU_HOST_CCPU_DIVIDE_1 16U /* Divide into 1 */
#define CPU_HOST_CCPU_DIVIDE_2 8U  /* Divide into 1/2 */
#define CPU_HOST_CCPU_DIVIDE_4 4U  /* Divide into 1/4 */
#define CPU_HOST_CCPU_DIVIDE_6 2U  /* Divide into 1/6 */
#define CPU_HOST_CCPU_DIVIDE_12 1U /* Divide into 1/12 */

/* device_aicpu */
#define CPU_DEV_AICPU_ATTR (ATTR_UPDATE_CFG | ATTR_VFG_SHARE | ATTR_NECESSARY)
#define CPU_DEV_AICPU_NUM 6ULL
#define CPU_DEV_AICPU_NUM_PER_BIT 1U
#define CPU_DEV_AICPU_BITMAPNUM 8U
#define CPU_DEV_AICPU_BITMAP 0xFCULL
#define CPU_DEV_AICPU_DIVIDE_1 6U  /* Divide into 1 */
#define CPU_DEV_AICPU_DIVIDE_2 3U  /* Divide into 1/2 */
#define CPU_DEV_AICPU_DIVIDE_4 1U  /* Divide into 1/4 */
#define CPU_DEV_AICPU_DIVIDE_6 1U  /* Divide into 1/6 */
#define CPU_DEV_AICPU_DIVIDE_12 0U /* Divide into 1/12 */

/* host_aicpu */
#define CPU_HOST_AICPU_ATTR (ATTR_UPDATE_CFG)
#define CPU_HOST_AICPU_NUM 64ULL
#define CPU_HOST_AICPU_NUM_PER_BIT 1U
#define CPU_HOST_AICPU_BITMAPNUM 64U
#define CPU_HOST_AICPU_BITMAP 0xFFFFFFFFFFFFFFFFUL
#define CPU_HOST_AICPU_DIVIDE_1 64U /* Divide into 1 */
#define CPU_HOST_AICPU_DIVIDE_2 32U /* Divide into 1/2 */
#define CPU_HOST_AICPU_DIVIDE_4 16U /* Divide into 1/4 */
#define CPU_HOST_AICPU_DIVIDE_6 8U  /* Divide into 1/6 */
#define CPU_HOST_AICPU_DIVIDE_12 4U /* Divide into 1/12 */

/* vf */
#define BASE_VF_ATTR (ATTR_CREATE_CFG)
#define BASE_VF_NUM 8
#define BASE_VF_NUM_PER_BIT 1U
#define BASE_VF_BITMAPNUM 8U
#define BASE_VF_BITMAP 0xFFUL

/* vfg */
#define BASE_VFG_ATTR (ATTR_CREATE_CFG)
#define BASE_VFG_NUM 7
#define BASE_VFG_NUM_PER_BIT 1U
#define BASE_VFG_BITMAPNUM 8U
#define BASE_VFG_BITMAP 0x7FUL

#endif
