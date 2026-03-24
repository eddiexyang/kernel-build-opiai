
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

#ifndef LPM_FAULT_AOSUB_SAFETY_H
#define LPM_FAULT_AOSUB_SAFETY_H

#ifdef LPM_FAULT_AO_SAFETY
#include <linux/types.h>
#include "fpdc.h"
#include "drvfault_user_common.h"


#define SCH_LKSTEP_MASK       ((uint32_t)BIT(0))
#define SCH_PARTY_MASK        ((uint32_t)BIT(10) | (uint32_t)GENMASK(4U, 3U) | (uint32_t)BIT(1))

#define SMMU_LKSTEP_MASK      ((uint32_t)BIT(13) | (uint32_t)BIT(0))
#define SMMU_PARITY_MASK      ((uint32_t)GENMASK(12U, 1U) | (uint32_t)GENMASK(17U, 14U) | (uint32_t)BIT(22))
#define SMMU_TIMEOUT_MASK     ((uint32_t)GENMASK(21U, 18U))
#define SMMU_MBECC_MASK       ((uint32_t)BIT(24))


#define DISP_AO_PARITY_MASK   ((uint32_t)BIT(29))
#define DISP_AO_TIMEOUT_MASK  ((uint32_t)GENMASK(27U, 21U))
#define DISP_AO_CRC_MASK      ((uint32_t)GENMASK(14U, 12U) | (uint32_t)BIT(5))
#define DISP_AO_LKSTEP_MASK1  ((uint32_t)GENMASK(4U, 0U))
#define DISP_AO_LKSTEP_MASK2  ((uint32_t)GENMASK(10U, 8U) | (uint32_t)GENMASK(2U, 0U)) // 0x868

#define AO_SUBSYS_STATUS_OFST 0x2c00U
#define PG_SENSOR_STATUS_OFST 0x2c20U

#define AOSUB_PARITY_MASK     ((uint32_t)GENMASK(18U, 17U) | (uint32_t)BIT(4) | (uint32_t)GENMASK(1U, 0U))
#define AOSUB_TSEN_EXCEP_MASK ((uint32_t)BIT(5))
#define AOSUB_LKSTEP_MASK     ((uint32_t)BIT(16))
#define AOSUB_UNACESS_MASK    ((uint32_t)GENMASK(14, 12) | (uint32_t)GENMASK(10, 8))
#define AOSUB_BUS_MASK        ((uint32_t)BIT(11))
#define AOSUB_OV_MASK         ((uint32_t)GENMASK(2U, 1U))
#define AOSUB_UV_MASK         ((uint32_t)GENMASK(18U, 17U))

struct ao_safety_info {
	uint32_t emu_id;  /* set value from @safety_fault_status */
	uint32_t src_id;  /* set value from @safety_fault_status */
	uint32_t bit_id;  /* set value from @safety_fault_status */
	uint32_t owner_node_type;
	uint8_t node_id;
	uint8_t event_severity;
};

struct ao_event_info {
	uint32_t mask;
	uint32_t sensor_type;
	uint32_t event_type;
	uint32_t event_assertion; // 0x1: occur 0x2: one_time
	uint32_t fault_status_offset; // interrupt status reg offset addr
	uint32_t node_type;
	uint8_t node_id;
	uint8_t event_severity;
};

typedef void (*lpm_fault_aosubsys_proc_func)(const uint32_t *const register_array, const uint32_t array_size);

struct func_node {
	uint8_t module_id;
	lpm_fault_aosubsys_proc_func handler;
};

int32_t lpm_fault_safety_aosub_init(void);
int32_t lpm_fault_safety_aosub_uninit(void);

void lpm_fault_safety_aosub_suspend(void);
void lpm_fault_safety_aosub_resume(void);

int32_t lpm_safety_irq_aosubsys_handler(struct safety_fault_info *safety_fault,
	uint32_t *event_num, struct safety_event **event_list);

#else
STATIC inline int32_t lpm_fault_safety_aosub_init(void)
{
	return 0;
}

STATIC inline int32_t lpm_fault_aosub_safety_uninit(void)
{
	return 0;
}
#endif

#endif /* LPM_FAULT_AOSUB_SAFETY_H */