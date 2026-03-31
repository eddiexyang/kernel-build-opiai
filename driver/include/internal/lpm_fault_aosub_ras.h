/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
*/
#ifndef LPM_FAULT_AOSUB_RAS_H
#define LPM_FAULT_AOSUB_RAS_H

// for disp_ao
#define DISP_AO_SBECC_MASK      ((uint32_t)0x1)
#define DISP_AO_MBECC_MASK      ((uint32_t)0x2)
#define DISP_AO_INPUT_ERR_MASK  ((uint32_t)0x12)
#define DISP_AO_WRITE_ERR_MASK  ((uint32_t)0x15)
#define DISP_AO_CONFIG_MASK     ((uint32_t)0xe)

#if defined(CFG_SOC_PLATFORM_MDC_V11)
// for aa_ao
// for AA_INTRAW(0x800)
#define AA_AO_ERR_FILTER_MASK  ((uint32_t)0x7)
#define AA_AO_DEC_MISS         ((uint32_t)0x1)
#define AA_AO_DAW_OVERLAP      ((uint32_t)0x2)
#define AA_AO_MSD_OVERLAP      ((uint32_t)0x4)

// for smmu_ao
// for RAS ERR STATUS0(0x2010)
// SERR(bit7~0)/IERR(bit15~8)
#define SMMU_AO_IERR_SERR_FILTER_MASK       ((uint32_t)0xFFFF)
#define SMMU_AO_STE_FETCH_ERR_MASK          ((uint32_t)0x100)
#define SMMU_AO_CD_FETCH_ERR_MASK           ((uint32_t)0x200)
#define SMMU_AO_WALK_EABT_ERR_MASK          ((uint32_t)0x300)
#define SMMU_AO_CMDQ_FETCH_ERR_MASK         ((uint32_t)0x400)
#define SMMU_AO_WRITE_EVENTQ_ABORD_ERR_MASK ((uint32_t)0x500)
#define SMMU_AO_WRITE_PRIQ_ABORD_ERR_MASK   ((uint32_t)0x600)
#define SMMU_AO_AXI_TIMEOUT_ERR_MASK        ((uint32_t)0x700)
#define SMMU_AO_SKYROS_TIMEOUT_ERR_MASK     ((uint32_t)0x800)
#define SMMU_AO_VMS_FETCH_ERR_MASK          ((uint32_t)0x900)
#define SMMU_AO_ECC_MASK                    ((uint32_t)0xA)

// UE(bit29)/CE(bit25~24)/DE(Bit23)
#define SMMU_AO_ECC_FILTER_MASK             ((uint32_t)0x23800000)
// at least one transient error was corrected
#define SMMU_AO_SBECC_TRANSIENT_MASK        ((uint32_t)0x01000000)
// at least one error was corrected
#define SMMU_AO_SBECC_ERR_MASK              ((uint32_t)0x02000000)
// at least one persistent error was corrected
#define SMMU_AO_SBECC_PERSISTENT_MASK       ((uint32_t)0x03000000)
// deferred error
#define SMMU_AO_MBECC_DE_MASK               ((uint32_t)0x00800000)
// uncorrected errors
#define SMMU_AO_MBECC_UE_MASK               ((uint32_t)0x20000000)

// event report error code
#define FAULT_EVENT_NOT_FOUND   (-1) // not found event from event_mask_table
#define FAULT_EVENT_REPORT_FAIL (-2) // lpm report fault failed

struct ras_event_info {
	uint8_t offset; // offset for register_array
	uint8_t node_id;
	uint8_t assertion; // 0x0: DMS_EVENT_TYPE_RESUME, 0x1: DMS_EVENT_TYPE_OCCUR 0x2: DMS_EVENT_TYPE_ONE_TIME
	uint8_t rsv;
	uint32_t filter_mask;
	uint32_t err_mask;
	uint32_t node_type;
	uint32_t sensor_type;
	uint32_t event_type;
};
#endif

int32_t lpm_fault_ras_aosub_init(void);
int32_t lpm_fault_ras_aosub_uninit(void);

#endif
