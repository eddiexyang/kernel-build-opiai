/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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
* Create: 2019-10-15
*/

#include <linux/delay.h>
#ifndef AOS_LLVM_BUILD
#include <linux/irq.h>
#endif
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <linux/workqueue.h>
#include <linux/idr.h>
#include <linux/list.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>
#ifndef AOS_LLVM_BUILD
#include <linux/smp.h>
#endif

#include "dms_template.h"
#include "dms_cmd_def.h"
#include "dev_mon_cmd_def.h"
#include "urd_acc_ctrl.h"

#include "memory_log.h"
#ifndef AOS_LLVM_BUILD
#include "memory_ecc.h"
#include "memory_ddr.h"
#include "memory_hbm.h"
#include "memory_scan.h"
#endif

#ifdef CFG_MEMORY_FAULT_INJECT
#include <drvmem_fault_inject.h>
#endif

#ifdef CFG_MEMORY_SAFETY_RAS
#include "memory_safety_ras.h"
#endif

#define DMS_FAULT_INJECT_FILTER_ON "main_cmd=0x0"
#define DMS_FAULT_INJECT_FILTER_OF "main_cmd=0x1"

#ifndef AOS_LLVM_BUILD
BEGIN_DMS_MODULE_DECLARATION(MODULE_MEMORY)
BEGIN_FEATURE_COMMAND()
ADD_FEATURE_COMMAND(MODULE_MEMORY, DMS_GET_ECC_STAT_CMD, ZERO_CMD, NULL, NULL, DMS_SUPPORT_ALL,
	memory_get_ecc_statistics)
#ifdef CFG_MEMORY_FAULT_INJECT
ADD_FEATURE_COMMAND(MODULE_MEMORY, DMS_FAULT_INJECT_CMD, DMS_FAULT_INJECT_SUB_CMD_MEMORY, DMS_FAULT_INJECT_FILTER_ON,
	"dmp_daemon", DMS_SUPPORT_ALL, drvmem_fault_inject_handler)
#endif

#ifdef CFG_MEMORY_DDR
ADD_FEATURE_COMMAND(MODULE_MEMORY, DMS_MAIN_CMD_MEMORY, DMS_SUBCMD_DDR_BW_UTIL_RATE, NULL, NULL, DMS_SUPPORT_ALL,
	memory_get_ddr_bandwidth_utilization_rate)
#ifdef CFG_MEMORY_DDR_INFO_FROM_SHAREMEM
ADD_FEATURE_COMMAND(MODULE_MEMORY, DMS_MAIN_CMD_MEMORY, DMS_SUBCMD_DDR_FREQUENCY, NULL, NULL, DMS_SUPPORT_ALL,
	memory_get_ddr_frequency)
#endif
#if defined(CFG_MEMORY_DDR_INFO_FROM_LP) || defined(CFG_MEMORY_DDR_INFO_FROM_SHAREMEM)
ADD_FEATURE_COMMAND(MODULE_MEMORY, DMS_GET_GET_DEVICE_INFO_CMD, ZERO_CMD, DMS_FILTER_MEMORY, NULL, DMS_SUPPORT_ALL,
	memory_dmsi_get_device_info)
#endif
#endif

#ifdef CFG_MEMORY_HBM
ADD_FEATURE_COMMAND(MODULE_MEMORY, DMS_MAIN_CMD_MEMORY, DMS_SUBCMD_HBM_BW_UTIL_RATE, NULL, NULL, DMS_SUPPORT_ALL,
	memory_get_hbm_bandwidth_utilization_rate)
#if defined(CFG_LPM_INFO_FROM_IPC) || defined(CFG_LPM_INFO_FROM_SHAREMEM)
ADD_FEATURE_COMMAND(MODULE_MEMORY, DMS_MAIN_CMD_MEMORY, DMS_SUBCMD_HBM_TEMPERATURE, NULL, NULL, DMS_SUPPORT_ALL,
	memory_get_hbm_temperature)
#endif

#endif
END_FEATURE_COMMAND()
END_MODULE_DECLARATION()
#endif

STATIC int __init memory_init(void)
{
	int ret;
#ifdef CFG_MEMORY_SAFETY_RAS
	ret = memory_safety_ras_init();
	if (ret != 0) {
		memory_drv_err("Failed to init memory safety_ras. (ret=%d)\n", ret);
		return ret;
	}
#endif
#ifdef CFG_MEMORY_DDR
	ret = memory_map_ddr_init();
	if (ret != 0) {
		memory_drv_err("Failed to register DDR map. (ret=%d)\n", ret);
		goto MEMORY_DDR_EXIT;
	}
#endif
#ifdef CFG_MEMORY_HBM
	ret = memory_map_hbm_init();
	if (ret != 0) {
		memory_drv_err("Failed to register HBM map. (ret=%d)\n", ret);
		goto MEMORY_HBM_EXIT;
	}
#endif
#ifdef CFG_MEMORY_FAULT_INJECT
	drvmem_fault_inject_init();
#endif

#ifndef AOS_LLVM_BUILD
	(void)memory_scan_init();
#endif

#ifndef AOS_LLVM_BUILD
	CALL_INIT_MODULE(MODULE_MEMORY);
#endif

	memory_drv_info("Memory driver init success.\n");
	return 0;

#ifndef AOS_LLVM_BUILD
	(void)memory_scan_uninit();
#endif

#ifdef CFG_MEMORY_HBM
MEMORY_HBM_EXIT:
#endif

#ifdef CFG_MEMORY_DDR
	memory_map_ddr_exit();
MEMORY_DDR_EXIT:
#endif

#ifdef CFG_MEMORY_SAFETY_RAS
	memory_safety_ras_exit();
#endif

	memory_drv_info("memory_init fail.\n");

	return ret;
}


STATIC void __exit memory_exit(void)
{
#ifdef CFG_MEMORY_FAULT_EVENT
	memory_fault_exit();
#endif

#ifdef CFG_MEMORY_DDR
	memory_map_ddr_exit();
#endif

#ifdef CFG_MEMORY_HBM
	memory_map_hbm_exit();
#endif

#ifdef CFG_MEMORY_FAULT_INJECT
	drvmem_fault_inject_uninit();
#endif

#ifndef AOS_LLVM_BUILD
	(void)memory_scan_uninit();
#endif

#ifdef CFG_MEMORY_SAFETY_RAS
	memory_safety_ras_exit();
#endif

#ifndef AOS_LLVM_BUILD
	CALL_EXIT_MODULE(MODULE_MEMORY);
#endif

	memory_drv_info("Memory driver exit success.\n");
}

module_init(memory_init);
module_exit(memory_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("DAVINCI driver");

