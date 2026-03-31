/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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
 * Create: 2022-12-05
 */

#ifndef __VIRTMNGDEV_SOC_RESOURCE_H__
#define __VIRTMNGDEV_SOC_RESOURCE_H__

#include <linux/mutex.h>
#include "icm_interface.h"
#include "virtmng_interface.h"

#define VMNGD_VDEV_INIT_STATUS  VMNG_VDEV_STATUS_FREE

/* Element flag */
#define ATTR_CREATE_CFG (1U << 1)        /* Whether to specified configuration when create vf */
#define ATTR_UPDATE_CFG (1U << 2)        /* Whether to specified configuration when update vf */
#define ATTR_VFG_SHARE (1U << 3)         /* Whether share in vfg */

#define VMNG_DTYPE_RANGE_FROM VMNG_HW_TYPE_C1_4
#define VMNG_DTYPE_RANGE_TO   VMNG_HW_TYPE_C2_4

/*
 * VF resource configuration
 */
/* aiv */
#define AC_AIV_ATTR (ATTR_CREATE_CFG | ATTR_UPDATE_CFG | ATTR_VFG_SHARE)
#define AC_AIV_NUM 1ULL
#define AC_AIV_NUM_PER_BIT 1U
#define AC_AIV_BITMAPNUM 1U
#define AC_AIV_BITMAP 0x1UL
#define AC_AIV_DIVIDE_1_4 1U /* Divide into 1/4 */
#define AC_AIV_DIVIDE_2_4 1U /* Divide into 1/2 */

/* aic */
#define AC_AIC_ATTR (ATTR_CREATE_CFG | ATTR_VFG_SHARE)
#define AC_AIC_NUM 1ULL
#define AC_AIC_NUM_PER_BIT 1U
#define AC_AIC_BITMAPNUM 1U
#define AC_AIC_BITMAP 0x1UL
#define AC_AIC_DIVIDE_1_4 1U /* Divide into 1/4 */
#define AC_AIC_DIVIDE_2_4 1U /* Divide into 1/2 */

/* c_core */
#define AC_C_CORE_ATTR (ATTR_VFG_SHARE)
#define AC_C_CORE_NUM 1ULL
#define AC_C_CORE_NUM_PER_BIT 1U
#define AC_C_CORE_BITMAPNUM 1U
#define AC_C_CORE_BITMAP 0x1UL
#define AC_C_CORE_DIVIDE_1_4 1U  /* Divide into 1/4 */
#define AC_C_CORE_DIVIDE_2_4 1U  /* Divide into 1/2 */

/* dsa */
#define AC_DSA_ATTR (ATTR_UPDATE_CFG | ATTR_UPDATE_CFG | ATTR_VFG_SHARE)
#define AC_DSA_NUM 1ULL
#define AC_DSA_NUM_PER_BIT 1U
#define AC_DSA_BITMAPNUM 1U
#define AC_DSA_BITMAP 0x1UL
#define AC_DSA_DIVIDE_1_4 1U  /* Divide into 1/4 */
#define AC_DSA_DIVIDE_2_4 1U  /* Divide into 1/2 */

/* ffts */
#define AC_FFTS_ATTR (ATTR_VFG_SHARE)
#define AC_FFTS_NUM 2ULL
#define AC_FFTS_NUM_PER_BIT 1U
#define AC_FFTS_BITMAPNUM 2U
#define AC_FFTS_BITMAP 0x3UL
#define AC_FFTS_DIVIDE_1_4 2U  /* Divide into 1, each pool 1 */
#define AC_FFTS_DIVIDE_2_4 2U  /* Divide into 1/2 */

/* sdma */
#define AC_SDMA_ATTR (ATTR_UPDATE_CFG | ATTR_VFG_SHARE)
#define AC_SDMA_NUM 4ULL
#define AC_SDMA_NUM_PER_BIT 1U
#define AC_SDMA_BITMAPNUM 4U
#define AC_SDMA_BITMAP 0xFUL
#define AC_SDMA_DIVIDE_1_4 4U /* Divide into 1/4 */
#define AC_SDMA_DIVIDE_2_4 4U /* Divide into 1/2 */

/* stars pcie_dma (RC not support) */
#define AC_PCIE_DMA_ATTR (ATTR_CREATE_CFG | ATTR_VFG_SHARE)
#define AC_PCIE_DMA_NUM 0ULL
#define AC_PCIE_DMA_NUM_PER_BIT 1U
#define AC_PCIE_DMA_BITMAPNUM 0U
#define AC_PCIE_DMA_BITMAP 0x0UL
#define AC_PCIE_DMA_DIVIDE_1_4 0U  /* Divide into 1/4 */
#define AC_PCIE_DMA_DIVIDE_2_4 0U  /* Divide into 1/2 */

/* acsq */
#define AC_ACSQ_ATTR (ATTR_CREATE_CFG | ATTR_VFG_SHARE)
#define AC_ACSQ_NUM 32ULL
#define AC_ACSQ_NUM_PER_BIT 2U
#define AC_ACSQ_BITMAPNUM 16U
#define AC_ACSQ_BITMAP 0xFFFFUL
#define AC_ACSQ_DIVIDE_1_4 32U /* Divide into 1/4 */
#define AC_ACSQ_DIVIDE_2_4 32U /* Divide into 1/2 */

/* rtsq */
#define AC_RTSQ_ATTR (ATTR_CREATE_CFG)
#define AC_RTSQ_NUM 512ULL
#define AC_RTSQ_NUM_PER_BIT 32U
#define AC_RTSQ_BITMAPNUM 16U
#define AC_RTSQ_BITMAP 0xFFFFUL
#define AC_RTSQ_DIVIDE_1_4 128U /* Divide into 1/4 */
#define AC_RTSQ_DIVIDE_2_4 256U /* Divide into 1/2 */

/* event_id */
#define AC_EVENT_ID_ATTR (ATTR_CREATE_CFG)
#define AC_EVENT_ID_NUM 65536ULL
#define AC_EVENT_ID_NUM_PER_BIT 4096U
#define AC_EVENT_ID_BITMAPNUM 16U
#define AC_EVENT_ID_BITMAP 0xFFFFUL
#define AC_EVENT_ID_DIVIDE_1_4 16384U  /* Divide into 1/4 */
#define AC_EVENT_ID_DIVIDE_2_4 32768U /* Divide into 1/2 */

/* notify_id */
#define AC_NOTIFY_ID_ATTR (ATTR_CREATE_CFG)
#define AC_NOTIFY_ID_NUM 2048ULL
#define AC_NOTIFY_ID_NUM_PER_BIT 128U
#define AC_NOTIFY_ID_BITMAPNUM 16U
#define AC_NOTIFY_ID_BITMAP 0xFFFFUL
#define AC_NOTIFY_ID_DIVIDE_1_4 512U  /* Divide into 1/4 */
#define AC_NOTIFY_ID_DIVIDE_2_4 1024U /* Divide into 1/2 */

/* cdqm */
#define AC_CDQM_ATTR (ATTR_CREATE_CFG | ATTR_UPDATE_CFG | ATTR_VFG_SHARE)
#define AC_CDQM_NUM 0ULL
#define AC_CDQM_NUM_PER_BIT 2U
#define AC_CDQM_BITMAPNUM 0U
#define AC_CDQM_BITMAP 0x0UL
#define AC_CDQM_DIVIDE_1_4 0U /* Divide into 1/4 */
#define AC_CDQM_DIVIDE_2_4 0U  /* Divide into 1/2 */

/* cmo_id */ // 16bit, total resource need to be determined. cur = 64K
#define AC_CMO_ID_ATTR (ATTR_CREATE_CFG)
#define AC_CMO_ID_NUM 65536ULL
#define AC_CMO_ID_NUM_PER_BIT 4096U
#define AC_CMO_ID_BITMAPNUM 16U
#define AC_CMO_ID_BITMAP 0xFFFFUL
#define AC_CMO_ID_DIVIDE_1_4 16384U  /* Divide into 1/4 */
#define AC_CMO_ID_DIVIDE_2_4 32768U  /* Divide into 1/2 */

/* memory */
#define AC_MEMORY_ATTR (ATTR_CREATE_CFG | ATTR_UPDATE_CFG)
#define AC_MEMORY_NUM 0ULL
#define AC_MEMORY_NUM_PER_BIT 1U
#define AC_MEMORY_BITMAPNUM 0U
#define AC_MEMORY_BITMAP 0x0UL
#define AC_MEMORY_DIVIDE_1_4 0U /* Divide into 1, totally 15 node, each 2G */
#define AC_MEMORY_DIVIDE_2_4 0U  /* Divide into 1/2, legal val only 7,8 */

/* jpegd */
#define DVPP_JPEGD_ATTR (ATTR_UPDATE_CFG | ATTR_VFG_SHARE)
#define DVPP_JPEGD_NUM 2ULL
#define DVPP_JPEGD_NUM_PER_BIT 1U
#define DVPP_JPEGD_BITMAPNUM 2U
#define DVPP_JPEGD_BITMAP 0x3UL
#define DVPP_JPEGD_DIVIDE_1_4 2U /* Divide into 1/4 */
#define DVPP_JPEGD_DIVIDE_2_4 2U /* Divide into 1/2 */

/* jpege */
#define DVPP_JPEGE_ATTR (ATTR_UPDATE_CFG | ATTR_VFG_SHARE)
#define DVPP_JPEGE_NUM 1ULL
#define DVPP_JPEGE_NUM_PER_BIT 1U
#define DVPP_JPEGE_BITMAPNUM 1U
#define DVPP_JPEGE_BITMAP 0x1UL
#define DVPP_JPEGE_DIVIDE_1_4 1U  /* Divide into 1/4 */
#define DVPP_JPEGE_DIVIDE_2_4 1U  /* Divide into 1/2 */

/* vpc */
#define DVPP_VPC_ATTR (ATTR_UPDATE_CFG | ATTR_VFG_SHARE)
#define DVPP_VPC_NUM 2ULL
#define DVPP_VPC_NUM_PER_BIT 1U
#define DVPP_VPC_BITMAPNUM 2U
#define DVPP_VPC_BITMAP 0x3UL
#define DVPP_VPC_DIVIDE_1_4 2U /* Divide into 1/4 */
#define DVPP_VPC_DIVIDE_2_4 2U  /* Divide into 1/2 */

/* vdec */
#define DVPP_VDEC_ATTR (ATTR_UPDATE_CFG)
#define DVPP_VDEC_NUM 4ULL
#define DVPP_VDEC_NUM_PER_BIT 1U
#define DVPP_VDEC_BITMAPNUM 4U
#define DVPP_VDEC_BITMAP 0xFUL
#define DVPP_VDEC_DIVIDE_1_4 1U  /* Divide into 1/4 */
#define DVPP_VDEC_DIVIDE_2_4 2U  /* Divide into 1/2 */

/* pngd */
#define DVPP_PNGD_ATTR (ATTR_UPDATE_CFG | ATTR_VFG_SHARE)
#define DVPP_PNGD_NUM 2ULL
#define DVPP_PNGD_NUM_PER_BIT 1U
#define DVPP_PNGD_BITMAPNUM 2U
#define DVPP_PNGD_BITMAP 0x3ULL
#define DVPP_PNGD_DIVIDE_1_4 2U  /* Divide into 1/4 */
#define DVPP_PNGD_DIVIDE_2_4 2U  /* Divide into 1/2 */

/* venc */
#define DVPP_VENC_ATTR (ATTR_UPDATE_CFG)
#define DVPP_VENC_NUM 3ULL
#define DVPP_VENC_NUM_PER_BIT 1U
#define DVPP_VENC_BITMAPNUM 3U
#define DVPP_VENC_BITMAP 0x7ULL
#define DVPP_VENC_DIVIDE_1_4 0U  /* Divide into 1/4 */
#define DVPP_VENC_DIVIDE_2_4 1U  /* Divide into 1/2 */

/* topic_aicpu_slot */
#define CPU_TOPIC_AICPU_ATTR (ATTR_UPDATE_CFG | ATTR_VFG_SHARE)
#define CPU_TOPIC_AICPU_NUM 8ULL
#define CPU_TOPIC_AICPU_NUM_PER_BIT 1U
#define CPU_TOPIC_AICPU_BITMAPNUM 8U
#define CPU_TOPIC_AICPU_BITMAP 0xFFUL
#define CPU_TOPIC_AICPU_DIVIDE_1_4 8U /* Divide into 1/4 */
#define CPU_TOPIC_AICPU_DIVIDE_2_4 8U /* Divide into 1/2 */

/* topic_ctrl_cpu_slot */
#define CPU_TOPIC_CCPU_ATTR (ATTR_VFG_SHARE)
#define CPU_TOPIC_CCPU_NUM 8ULL
#define CPU_TOPIC_CCPU_NUM_PER_BIT 1U
#define CPU_TOPIC_CCPU_BITMAPNUM 8U
#define CPU_TOPIC_CCPU_BITMAP 0xFFUL
#define CPU_TOPIC_CCPU_DIVIDE_1_4 8U  /* Divide into 1/4 */
#define CPU_TOPIC_CCPU_DIVIDE_2_4 8U  /* Divide into 1/2 */

/* host_ctrl_cpu */
#define CPU_HOST_CCPU_ATTR (ATTR_UPDATE_CFG)
#define CPU_HOST_CCPU_NUM 0ULL
#define CPU_HOST_CCPU_NUM_PER_BIT 1U
#define CPU_HOST_CCPU_BITMAPNUM 0U
#define CPU_HOST_CCPU_BITMAP 0x0UL
#define CPU_HOST_CCPU_DIVIDE_1_4 0U /* Divide into 1/4 */
#define CPU_HOST_CCPU_DIVIDE_2_4 0U  /* Divide into 1/2 */

/* device_aicpu */
#define CPU_DEV_AICPU_ATTR (ATTR_UPDATE_CFG | ATTR_VFG_SHARE)
#define CPU_DEV_AICPU_NUM 1ULL
#define CPU_DEV_AICPU_NUM_PER_BIT 1U
#define CPU_DEV_AICPU_BITMAPNUM 1U
#define CPU_DEV_AICPU_BITMAP 0x8UL
#define CPU_DEV_AICPU_DIVIDE_1_4 1U  /* Divide into 1/4 */
#define CPU_DEV_AICPU_DIVIDE_2_4 1U  /* Divide into 1/2 */

/* host_aicpu */
#define CPU_HOST_AICPU_ATTR (ATTR_UPDATE_CFG)
#define CPU_HOST_AICPU_NUM 0ULL
#define CPU_HOST_AICPU_NUM_PER_BIT 1U
#define CPU_HOST_AICPU_BITMAPNUM 0U
#define CPU_HOST_AICPU_BITMAP 0x0UL
#define CPU_HOST_AICPU_DIVIDE_1_4 0U /* Divide into 1/4 */
#define CPU_HOST_AICPU_DIVIDE_2_4 0U /* Divide into 1/2 */

/* vf */
#define BASE_VF_ATTR (ATTR_CREATE_CFG)
#define BASE_VF_NUM 4
#define BASE_VF_NUM_PER_BIT 1U
#define BASE_VF_BITMAPNUM 4U
#define BASE_VF_BITMAP 0xFUL

/* vfg */
#define BASE_VFG_ATTR (ATTR_CREATE_CFG)
#define BASE_VFG_NUM 1
#define BASE_VFG_NUM_PER_BIT 1U
#define BASE_VFG_BITMAPNUM 1U
#define BASE_VFG_BITMAP 0x1U

#endif
