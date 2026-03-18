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

#define VMNGD_VDEV_INIT_STATUS  VMNG_VDEV_STATUS_CLIENT_UNINIT

#define VMNGD_ICM_MBX_CHAN  0

/* Element flag */
#define ATTR_CREATE_CFG (1U << 1)        /* Whether to specified configuration when create vf */
#define ATTR_UPDATE_CFG (1U << 2)        /* Whether to specified configuration when update vf */
#define ATTR_VFG_SHARE (1U << 3)         /* Whether share in vfg */

#define VMNG_DTYPE_RANGE_FROM VMNG_TYPE_C1
#define VMNG_DTYPE_RANGE_TO   VMNG_TYPE_C16

/*
 * VF resource configuration
 */
/* aiv */
#define AC_AIV_ATTR (ATTR_CREATE_CFG | ATTR_UPDATE_CFG | ATTR_VFG_SHARE)
#define AC_AIV_NUM 0ULL
#define AC_AIV_NUM_PER_BIT 1U
#define AC_AIV_BITMAPNUM 48U
#define AC_AIV_BITMAP 0x0ULL

/* aic */
#define AC_AIC_ATTR (ATTR_CREATE_CFG | ATTR_VFG_SHARE)
#define AC_AIC_NUM 0ULL
#define AC_AIC_NUM_PER_BIT 1U
#define AC_AIC_BITMAPNUM 24U
#define AC_AIC_BITMAP 0x0ULL

/* c_core */
#define AC_C_CORE_ATTR (ATTR_VFG_SHARE)
#define AC_C_CORE_NUM 0ULL
#define AC_C_CORE_NUM_PER_BIT 1U
#define AC_C_CORE_BITMAPNUM 8U
#define AC_C_CORE_BITMAP 0x0ULL

/* dsa */
#define AC_DSA_ATTR (ATTR_UPDATE_CFG | ATTR_UPDATE_CFG | ATTR_VFG_SHARE)
#define AC_DSA_NUM 0ULL
#define AC_DSA_NUM_PER_BIT 1U
#define AC_DSA_BITMAPNUM 8U
#define AC_DSA_BITMAP 0x0ULL

/* ffts */
#define AC_FFTS_ATTR (ATTR_VFG_SHARE)
#define AC_FFTS_NUM 0ULL
#define AC_FFTS_NUM_PER_BIT 1U
#define AC_FFTS_BITMAPNUM 8U
#define AC_FFTS_BITMAP 0x0ULL

/* sdma */
#define AC_SDMA_ATTR (ATTR_UPDATE_CFG | ATTR_VFG_SHARE)
#define AC_SDMA_NUM 0ULL
#define AC_SDMA_NUM_PER_BIT 1U
#define AC_SDMA_BITMAPNUM 32U
#define AC_SDMA_BITMAP 0x0ULL

/* pcie_dma */
#define AC_PCIE_DMA_ATTR (ATTR_CREATE_CFG)
#define AC_PCIE_DMA_NUM 0ULL
#define AC_PCIE_DMA_NUM_PER_BIT 1U
#define AC_PCIE_DMA_BITMAPNUM 38U
#define AC_PCIE_DMA_BITMAP 0x0ULL

/* acsq */
#define AC_ACSQ_ATTR (ATTR_CREATE_CFG | ATTR_VFG_SHARE)
#define AC_ACSQ_NUM 0ULL
#define AC_ACSQ_NUM_PER_BIT 8U
#define AC_ACSQ_BITMAPNUM 16U
#define AC_ACSQ_BITMAP 0x0ULL

/* rtsq */
#define AC_RTSQ_ATTR (ATTR_CREATE_CFG)
#define AC_RTSQ_NUM 0ULL
#define AC_RTSQ_NUM_PER_BIT 128U
#define AC_RTSQ_BITMAPNUM 16U
#define AC_RTSQ_BITMAP 0x0ULL

/* event_id */
#define AC_EVENT_ID_ATTR (ATTR_CREATE_CFG)
#define AC_EVENT_ID_NUM 0ULL
#define AC_EVENT_ID_NUM_PER_BIT 128U
#define AC_EVENT_ID_BITMAPNUM 16U
#define AC_EVENT_ID_BITMAP 0x0ULL

/* notify_id */
#define AC_NOTIFY_ID_ATTR (ATTR_CREATE_CFG)
#define AC_NOTIFY_ID_NUM 0ULL
#define AC_NOTIFY_ID_NUM_PER_BIT 512U
#define AC_NOTIFY_ID_BITMAPNUM 16U
#define AC_NOTIFY_ID_BITMAP 0x0ULL

/* cdqm */
#define AC_CDQM_ATTR (ATTR_CREATE_CFG | ATTR_UPDATE_CFG)
#define AC_CDQM_NUM 0ULL
#define AC_CDQM_NUM_PER_BIT 8U
#define AC_CDQM_BITMAPNUM 16U
#define AC_CDQM_BITMAP 0x0ULL

/* cmo_id */
#define AC_CMO_ID_ATTR (ATTR_CREATE_CFG)
#define AC_CMO_ID_NUM 0ULL
#define AC_CMO_ID_NUM_PER_BIT 1U
#define AC_CMO_ID_BITMAPNUM 0U
#define AC_CMO_ID_BITMAP 0x0ULL

/* memory */
#define AC_MEMORY_ATTR (ATTR_CREATE_CFG | ATTR_UPDATE_CFG)
#define AC_MEMORY_NUM 0ULL
#define AC_MEMORY_NUM_PER_BIT 2U
#define AC_MEMORY_BITMAPNUM 15U
#define AC_MEMORY_BITMAP 0x0ULL

/* jpegd */
#define DVPP_JPEGD_ATTR (ATTR_UPDATE_CFG | ATTR_VFG_SHARE)
#define DVPP_JPEGD_NUM 0ULL
#define DVPP_JPEGD_NUM_PER_BIT 1U
#define DVPP_JPEGD_BITMAPNUM 28U
#define DVPP_JPEGD_BITMAP 0x0ULL

/* jpege */
#define DVPP_JPEGE_ATTR (ATTR_UPDATE_CFG | ATTR_VFG_SHARE)
#define DVPP_JPEGE_NUM 0ULL
#define DVPP_JPEGE_NUM_PER_BIT 1U
#define DVPP_JPEGE_BITMAPNUM 4U
#define DVPP_JPEGE_BITMAP 0x0ULL

/* vpc */
#define DVPP_VPC_ATTR (ATTR_UPDATE_CFG | ATTR_VFG_SHARE)
#define DVPP_VPC_NUM 0ULL
#define DVPP_VPC_NUM_PER_BIT 1U
#define DVPP_VPC_BITMAPNUM 10U
#define DVPP_VPC_BITMAP 0x0ULL

/* vdec */
#define DVPP_VDEC_ATTR (ATTR_UPDATE_CFG | ATTR_VFG_SHARE)
#define DVPP_VDEC_NUM 0ULL
#define DVPP_VDEC_NUM_PER_BIT 1
#define DVPP_VDEC_BITMAPNUM 2U
#define DVPP_VDEC_BITMAP 0x0ULL

/* pngd */
#define DVPP_PNGD_ATTR (ATTR_UPDATE_CFG | ATTR_VFG_SHARE)
#define DVPP_PNGD_NUM 0ULL
#define DVPP_PNGD_NUM_PER_BIT 1U
#define DVPP_PNGD_BITMAPNUM 0U
#define DVPP_PNGD_BITMAP 0x0ULL

/* venc */
#define DVPP_VENC_ATTR (ATTR_UPDATE_CFG | ATTR_VFG_SHARE)
#define DVPP_VENC_NUM 0ULL
#define DVPP_VENC_NUM_PER_BIT 1U
#define DVPP_VENC_BITMAPNUM 0U
#define DVPP_VENC_BITMAP 0x0ULL

/* topic_aicpu_slot */
#define CPU_TOPIC_AICPU_ATTR (ATTR_UPDATE_CFG | ATTR_VFG_SHARE)
#define CPU_TOPIC_AICPU_NUM 0ULL
#define CPU_TOPIC_AICPU_NUM_PER_BIT 1U
#define CPU_TOPIC_AICPU_BITMAPNUM 32U
#define CPU_TOPIC_AICPU_BITMAP 0x0ULL

/* topic_ctrl_cpu_slot */
#define CPU_TOPIC_CCPU_ATTR (ATTR_VFG_SHARE)
#define CPU_TOPIC_CCPU_NUM 0ULL
#define CPU_TOPIC_CCPU_NUM_PER_BIT 1U
#define CPU_TOPIC_CCPU_BITMAPNUM 32U
#define CPU_TOPIC_CCPU_BITMAP 0x0ULL

/* host_ctrl_cpu */
#define CPU_HOST_CCPU_ATTR (ATTR_UPDATE_CFG)
#define CPU_HOST_CCPU_NUM 0ULL
#define CPU_HOST_CCPU_NUM_PER_BIT 1U
#define CPU_HOST_CCPU_BITMAPNUM 16U
#define CPU_HOST_CCPU_BITMAP 0x0ULL

/* device_aicpu */
#define CPU_DEV_AICPU_ATTR (ATTR_UPDATE_CFG | ATTR_VFG_SHARE)
#define CPU_DEV_AICPU_NUM 0ULL
#define CPU_DEV_AICPU_NUM_PER_BIT 1U
#define CPU_DEV_AICPU_BITMAPNUM 6U
#define CPU_DEV_AICPU_BITMAP 0x0ULL

/* host_aicpu */
#define CPU_HOST_AICPU_ATTR (ATTR_UPDATE_CFG)
#define CPU_HOST_AICPU_NUM 0ULL
#define CPU_HOST_AICPU_NUM_PER_BIT 1U
#define CPU_HOST_AICPU_BITMAPNUM 64U
#define CPU_HOST_AICPU_BITMAP 0x0ULL

/* vf */
#define BASE_VF_ATTR (ATTR_CREATE_CFG)
#define BASE_VF_NUM 16
#define BASE_VF_NUM_PER_BIT 1U
#define BASE_VF_BITMAPNUM 12U
#define BASE_VF_BITMAP 0x0U

/* vfg */
#define BASE_VFG_ATTR (ATTR_CREATE_CFG)
#define BASE_VFG_NUM 1
#define BASE_VFG_NUM_PER_BIT 1U
#define BASE_VFG_BITMAPNUM 1U
#define BASE_VFG_BITMAP 0x0U

#endif
