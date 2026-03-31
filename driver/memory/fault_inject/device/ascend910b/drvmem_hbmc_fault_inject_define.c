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
* Description: hbmc fault inject declaration
* Author: Huawei Technologies Co.Ltd
* Create: 2023-11-1
*/

#include <linux/slab.h>
#include "drvmem_hbmc_fault_inject_define.h"
#include "drvmem_fault_inject_define.h"
#include "drvmem_base_info.h"
#include "drvmem_fault_inject_common.h"
#include "memory_fault_converge.h"
#include "memory_log.h"

/*
* @brief hbmc sbram ce error inject
* @param [in] base: base address
* @return 0: success, else: fail
*/
int32_t drvmem_hbmc_sbram_ce_inject(uint64_t base)
{
	size_t length;
	struct fault_inject_process proc[] = {
		{HBM_STACK0_BASE_ADDR + HBM_DDRC_CFG_FIFO_ERRINJ_ADDR(0), 0x1}
	};

	memory_drv_info("********** hbmc sbram ce fault inject ***********\n");
	length = sizeof(proc) / sizeof(proc[0]);

	return drvmem_fault_inject_write_reg(base, length, proc);
}

/*
* @brief hbmc sbram ue error inject
* @param [in] base: base address
* @return 0: success, else: fail
*/
int32_t drvmem_hbmc_sbram_ue_inject(uint64_t base)
{
	struct memory_fault_event event = {0};
	(void)base;

	memory_drv_info("******** hbmc sbram ue fault inject *************\n");
	event.device_id = 0;
	event.sec_type = RAS_SEC_GENERIC;
	event.sensor_id = MEMORY_SENSOR_HBMC;
	event.ras_err_type = RAS_UEU_SBRAM_UNCORRECT;

	return memory_oem_fault_event_handler(&event);
}

/*
* @brief hbmc rpram ue error inject
* @param [in] base: base address
* @return 0: success, else: fail
*/
int32_t drvmem_hbmc_rpram_ue_inject(uint64_t base)
{
	struct memory_fault_event event = {0};
	(void)base;

	memory_drv_info("******** hbmc rpram ue fault inject *************\n");
	event.device_id = 0;
	event.sec_type = RAS_SEC_GENERIC;
	event.sensor_id = MEMORY_SENSOR_HBMC;
	event.ras_err_type = RAS_UEU_RPRAM_UNCORRECT;

	return memory_oem_fault_event_handler(&event);
}

/*
* @brief hbmc sram ce error inject
* @param [in] base: base address
* @return 0: success, else: fail
*/
int32_t drvmem_hbmc_sram_ce_inject(uint64_t base)
{
	struct memory_fault_event event = {0};
	(void)base;

	memory_drv_info("******** hbmc sram ce fault inject **************\n");
	event.device_id = 0;
	event.sec_type = RAS_SEC_GENERIC;
	event.sensor_id = MEMORY_SENSOR_HBMC;
	event.ras_err_type = RAS_CE_SRAM_CORRECT;

	return memory_oem_fault_event_handler(&event);
}

/*
* @brief hbmc sram ue error inject
* @param [in] base: base address
* @return 0: success, else: fail
*/
int32_t drvmem_hbmc_sram_ue_inject(uint64_t base)
{
	struct memory_fault_event event = {0};
	(void)base;

	memory_drv_info("******** hbmc sram ue fault inject **************\n");
	event.device_id = 0;
	event.sec_type = RAS_SEC_GENERIC;
	event.sensor_id = MEMORY_SENSOR_HBMC;
	event.ras_err_type = RAS_UEU_SRAM_UNCORRECT;

	return memory_oem_fault_event_handler(&event);
}

/*
* @brief hbmc rdq parity error inject
* @param [in] base: base address
* @return 0: success, else: fail
*/
int32_t drvmem_hbmc_rdq_parity_inject(uint64_t base)
{
	size_t length;
	struct fault_inject_process proc[] = {
		{HBM_STACK0_BASE_ADDR + HBM_DDRC_CFG_RDQ_PAR_ERRINJ_ADDR(0), 0x1001} /* error address, error value */
	};

	memory_drv_info("********** hbmc rdq parity fault inject *********\n");
	length = sizeof(proc) / sizeof(proc[0]);

	return drvmem_fault_inject_write_reg(base, length, proc);
}

/*
* @brief hbmc wdq parity error inject
* @param [in] base: base address
* @return 0: success, else: fail
*/
int32_t drvmem_hbmc_wdq_parity_inject(uint64_t base)
{
	size_t length;
	struct fault_inject_process proc[] = {
		{HBM_STACK0_BASE_ADDR + HBM_DDRC_CFG_WDQ_PAR_ERRINJ_ADDR(0), 0x1001} /* error address, error value */
	};

	memory_drv_info("********** hbmc wdq parity fault inject *********\n");
	length = sizeof(proc) / sizeof(proc[0]);

	return drvmem_fault_inject_write_reg(base, length, proc);
}

/*
* @brief hbmc ca parity error inject
* @param [in] base: base address
* @return 0: success, else: fail
*/
int32_t drvmem_hbmc_ca_parity_inject(uint64_t base)
{
	size_t length;
	struct fault_inject_process proc[] = {
		{HBM_STACK0_BASE_ADDR + HBM_HBMC_BC_BASE_ADDR + 0x728ULL, 0xFF001}
	};

	memory_drv_info("********** hbmc ca parity fault inject **********\n");
	length = sizeof(proc) / sizeof(proc[0]);

	return drvmem_fault_inject_write_reg(base, length, proc);
}

/*
* @brief hbmc lack sref error inject
* @param [in] base: base address
* @return 0: success, else: fail
*/
int32_t drvmem_hbmc_lack_sref_error_inject(uint64_t base)
{
	struct memory_fault_event event = {0};
	(void)base;

	memory_drv_info("********** hbmc lack sref fault inject **********\n");
	event.device_id = 0;
	event.sec_type = RAS_SEC_GENERIC;
	event.sensor_id = MEMORY_SENSOR_HBMC;
	event.ras_err_type = RAS_UEU_DRAM_LACK_SREF;

	return memory_oem_fault_event_handler(&event);
}

/*
* @brief hbmc rdata timeout error inject
* @param [in] base: base address
* @return 0: success, else: fail
*/
int32_t drvmem_hbmc_rdata_timeout_error_inject(uint64_t base)
{
	struct memory_fault_event event = {0};
	(void)base;

	memory_drv_info("******** hbmc rdata timeout fault inject ********\n");
	event.device_id = 0;
	event.sec_type = RAS_SEC_GENERIC;
	event.sensor_id = MEMORY_SENSOR_HBMC;
	event.ras_err_type = RAS_UER_DFI_RDDAT_VALID_TIMEOUT;

	return memory_oem_fault_event_handler(&event);
}