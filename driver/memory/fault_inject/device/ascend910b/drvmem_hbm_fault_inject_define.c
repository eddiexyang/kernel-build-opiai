/*
* Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
* Description: hbm fault inject declaration
* Author: Huawei Technologies Co.Ltd
* Create: 2023-11-1
*/

#include <linux/slab.h>
#include "drvmem_hbm_fault_inject_define.h"
#include "drvmem_fault_inject_define.h"
#include "drvmem_fault_inject_common.h"
#include "memory_log.h"
#include "drvmem_base_info.h"
#include "hbm_isolation_common.h"
#include "devdrv_dfm.h"
#include "memory_fault_converge.h"

/*
* @brief hbm ce th error inject
* @param [in] base: base address
* @return 0: success, else: fail
*/
int32_t drvmem_hbm_ce_th_inject(uint64_t base)
{
	size_t length;
	uint64_t rasc_base;
	struct fault_inject_process fuzz_corr_inject_proc[] = {
	{ HBM_RASC_RINT_OFFSET, 0x3f },
	{ HBM_RASC_CFG_CORR_TH, 1 },
	{ HBM_RASC_CFG_ERRINJMODE_OFFSET, 0x13 },
	{ HBM_RASC_CFG_ERRINJ_DMSK_OFFSET, INJECT_CE_BIT },
	{ HBM_RASC_CTRL_ERRINJ_OFFSET, 1 }
	};

	memory_drv_info("************* hbm ce th error inject ************\n");
	rasc_base = base + HBM_STACK0_BASE_ADDR + HBM_RASC_BC_BASE_ADDR;
	length = sizeof(fuzz_corr_inject_proc) / sizeof(struct fault_inject_process);

	return drvmem_fault_inject_write_reg(rasc_base, length, fuzz_corr_inject_proc);
}

/*
* @brief hbm ue exceed 1 error inject
* @param [in] base: base address
* @return 0: success, else: fail
*/
int32_t drvmem_hbm_ue_exceed_1_inject(uint64_t base)
{
	int32_t ret;
	struct new_multi_ecc_list *new_ecc_event = NULL;
	(void)base;

	memory_drv_info("************ hbm ue exceed 1 inject *************\n");
	new_ecc_event = (struct new_multi_ecc_list *)kzalloc(sizeof(struct new_multi_ecc_list),
		GFP_KERNEL | __GFP_ACCOUNT);
	if (new_ecc_event == NULL) {
		memory_drv_err("New ecc event kzalloc failed.\n");
		return -ENOMEM;
	}

	new_ecc_event->dev_id = 0;
	new_ecc_event->physical_addr = 0x1640000000UL;
	new_ecc_event->error_type = HBM_NEW_MULTI_BITS_ECC;
	new_ecc_event->occupied_pid = 0;

	ret = hbm_add_ecc_event(new_ecc_event);
	if (ret != 0) {
		memory_drv_err("Add new multi ecc event failed. (dev_id=%u; phy_addr=0x%llx)\n",
			new_ecc_event->dev_id, new_ecc_event->physical_addr);
		kfree(new_ecc_event);
	}
	return ret;
}

/*
* @brief hbm ue exceed 16 error inject
* @param [in] base: base address
* @return 0: success, else: fail
*/
int32_t drvmem_hbm_ue_exceed_16_inject(uint64_t base)
{
	uint32_t dev_id;
	uint32_t ecc_recorded_count;
	uint32_t except_id;
	(void)base;

	memory_drv_info("*********** hbm ue exceed 16 inject *************\n");
	dev_id = 0;
	ecc_recorded_count = NOTICE_MULTI_ECC_COUNT;
	except_id = ECC_EXCEED_16_WARNING;

	hbm_add_statistics_event(dev_id, ecc_recorded_count, except_id);
	return 0;
}

/*
* @brief hbm ue exceed 64 error inject
* @param [in] base: base address
* @return 0: success, else: fail
*/
int32_t drvmem_hbm_ue_exceed_64_inject(uint64_t base)
{
	uint32_t dev_id;
	uint32_t ecc_recorded_count;
	uint32_t except_id;
	(void)base;

	memory_drv_info("*********** hbm ue exceed 64 inject *************\n");
	dev_id = 0;
	ecc_recorded_count = ERROR_MULTI_ECC_COUNT;
	except_id = ECC_REACH_64_WARNING;

	hbm_add_statistics_event(dev_id, ecc_recorded_count, except_id);
	return 0;
}

/*
* @brief hbm fatal high temp error inject
* @param [in] base: base address
* @return 0: success, else: fail
*/
int32_t drvmem_hbm_fatal_high_temper_inject(uint64_t base)
{
	struct memory_fault_event event = {0};
	(void)base;

	memory_drv_info("******* hbm fatal high temper fault inject ******\n");
	event.device_id = 0;
	event.sec_type = RAS_SEC_GENERIC;
	event.sensor_id = MEMORY_SENSOR_HBMC;
	event.ras_err_type = RAS_UEU_FATAL_HIGH_TEMPER_INT;
	event.sensor_idx = S_IDX10;

	return memory_oem_fault_event_handler(&event);
}

/*
* @brief hbm high temp error inject
* @param [in] base: base address
* @return 0: success, else: fail
*/
int32_t drvmem_hbm_high_temper_inject(uint64_t base)
{
	struct memory_fault_event event = {0};
	(void)base;

	memory_drv_info("********** hbm high temper fault inject *********\n");
	event.device_id = 0;
	event.sec_type = RAS_SEC_GENERIC;
	event.sensor_id = MEMORY_SENSOR_HBMC;
	event.ras_err_type = RAS_UEU_HIGH_TEMPER_INT;
	event.sensor_idx = S_IDX10;

	return memory_oem_fault_event_handler(&event);
}

/*
* @brief hbm ue with process used error inject
* @param [in] base: base address
* @return 0: success, else: fail
*/
int32_t drvmem_hbm_ue_with_process_used_inject(uint64_t base)
{
	unsigned long ecc_pfn;
	struct pfn_list *new_pfn = NULL;
	struct ecc_config_tool_s *ecc_config_tool = hbm_get_ecc_config_tool();
	(void)base;

	memory_drv_info("******** hbm ue with process used inject ********\n");
	ecc_pfn = __phys_to_pfn(0x1500cfcf00UL);

	mutex_lock(&ecc_config_tool->un_online_isolated_pfn_queue[0].mutex);
	new_pfn = (struct pfn_list *)kzalloc(sizeof(struct pfn_list), GFP_KERNEL | __GFP_ACCOUNT);
	if (new_pfn == NULL) {
		mutex_unlock(&ecc_config_tool->un_online_isolated_pfn_queue[0].mutex);
		memory_drv_err("New pfn kzalloc failed.\n");
		return -ENOMEM;
	};

	new_pfn->pfn = ecc_pfn;
	list_add(&new_pfn->list, &ecc_config_tool->un_online_isolated_pfn_queue[0].pfns.list);
	ecc_config_tool->un_online_isolated_pfn_queue[0].pfn_num++;
	mutex_unlock(&ecc_config_tool->un_online_isolated_pfn_queue[0].mutex);

	return OK;
}

/*
* @brief hbm ue demand scrub error inject
* @param [in] base: base address
* @return 0: success, else: fail
*/
int32_t drvmem_hbm_ue_demand_scrub_inject(uint64_t base)
{
	struct memory_fault_event event = {0};
	(void)base;

	memory_drv_info("******** hbm ue demand scrub inject *************\n");
	event.device_id = 0;
	event.sec_type = RAS_SEC_MEM;
	event.sensor_id = MEMORY_SENSOR_HBMC;
	event.ras_err_type = STANDARD_ERR_TYPE_DSCRUB_UNCORRECT;
	event.sensor_idx = S_IDX7;

	return memory_oem_fault_event_handler(&event);
}