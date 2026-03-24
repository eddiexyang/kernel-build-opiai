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
*/
#ifndef LPM_DEVMNG_COMMON_H
#define LPM_DEVMNG_COMMON_H

#include <linux/types.h>
#include <linux/printk.h>
#include <linux/securec.h>
#include <linux/errno.h>
#include <linux/time64.h>
#include <linux/platform_device.h>
#include "lpm_log_base.h"
#include "drv_ipc.h"

#define MODULE_LPM_DEVMNG "[DRV_LPM_DEVMNG]"
#define MODULE_LPM_DEVMNG_MATCH "hisi,lpm_dev_manager"

#ifndef LPM_VPRINTK
#define lpm_log_err(...)   (void)printk(KERN_ERR MODULE_LPM_DEVMNG __VA_ARGS__)
#define lpm_log_warn(...)  (void)printk(KERN_WARNING MODULE_LPM_DEVMNG __VA_ARGS__)
#define lpm_log_event(...) (void)printk(KERN_NOTICE MODULE_LPM_DEVMNG __VA_ARGS__)
#define lpm_log_info(...)  (void)printk(KERN_INFO MODULE_LPM_DEVMNG __VA_ARGS__)
#else
#define lpm_log_err(...)   (void)lpm_log_vprintk_emit(LOGLEVEL_ERR,     MODULE_LPM_DEVMNG __VA_ARGS__)
#define lpm_log_warn(...)  (void)lpm_log_vprintk_emit(LOGLEVEL_WARNING, MODULE_LPM_DEVMNG __VA_ARGS__)
#define lpm_log_event(...) (void)lpm_log_vprintk_emit(LOGLEVEL_NOTICE,  MODULE_LPM_DEVMNG __VA_ARGS__)
#define lpm_log_info(...)  (void)lpm_log_vprintk_emit(LOGLEVEL_INFO,    MODULE_LPM_DEVMNG __VA_ARGS__)
#endif

#ifdef LPM_DEVMNG_UT
#define STATIC
#else
#define STATIC static
#endif

#define LPM_DEVMNG_SUPPORT 1
#define LPM_DEVMNG_NOT_SUPPORT 0

#define LPM_DELAY_10_MS 10
#define LPM_DELAY_100_US 100
#define LPM_DELAY_1000_US 1000

#define LPM_NS_TO_US 1000U // 1us = 1000ns

#define LPM_DEVMNG_WORK_QUEUE_CPU  0

#ifdef CFG_SOC_PLATFORM_MINIV3
// one chip, each chip has one die
#define LPM_DEVMNG_DIE_OFFSET            0ULL
#define LPM_DEVMNG_CHIP_OFFSET           0ULL
#define LPM_DEVMNG_DEV_MAX_NUM           1U

// config for sysctl reg
#define LPM_DEVMNG_SYSCTL_REG_BASE_ADDR      0xC0140000ULL
#define LPM_DEVMNG_SYSCTL_REG_SIZE           0x10000ULL
#define LPM_DEVMNG_SYSCTL_VERSION_REG_OFFSET 0xFFFCULL

// config for IPC
#define LPM_IPC_TX_LP_ACPU0  HISI_RPROC_TX_LP_ACPU0

// CMD source & dest define
#define LPM_DEV_OBJ_LP       OBJ_CMD_LP
#define LPM_DEV_OBJ_TAISHAN0 OBJ_CMD_CCPU

#elif defined(CFG_SOC_PLATFORM_CLOUD_V2)
// multiple chips, each chip has one die
// or multiple chips, each chip has two die
#define LPM_DEVMNG_DIE_OFFSET            0x10000000000ULL
#define LPM_DEVMNG_CHIP_OFFSET           0x80000000000ULL
#define LPM_DEVMNG_DEV_MAX_NUM           2U

// config for sysctl reg
#define LPM_DEVMNG_SYSCTL_REG_BASE_ADDR      0x80000000ULL
#define LPM_DEVMNG_SYSCTL_REG_SIZE           0x10000ULL
#define LPM_DEVMNG_SYSCTL_VERSION_REG_OFFSET 0xFFFCULL

// config for IPC
#define LPM_IPC_TX_LP_ACPU0  HISI_RPROC_TX_IMU_MBX28

// CMD source & dest define
#define LPM_DEV_OBJ_LP       OBJ_CMD_LP
#define LPM_DEV_OBJ_TAISHAN0 OBJ_CMD_CCPU

#elif defined(CFG_SOC_PLATFORM_MDC_V51)
// one chips, each chip has one die
#define LPM_DEVMNG_DIE_OFFSET            0ULL
#define LPM_DEVMNG_CHIP_OFFSET           0x8000000000ULL
#define LPM_DEVMNG_DEV_MAX_NUM           1U

// config for sysctl reg
#define LPM_DEVMNG_SYSCTL_REG_BASE_ADDR      0x80000000ULL
#define LPM_DEVMNG_SYSCTL_REG_SIZE           0x10000ULL
#define LPM_DEVMNG_SYSCTL_VERSION_REG_OFFSET 0xFFFCULL

// config for IPC
#define LPM_IPC_TX_LP_ACPU0  HISI_RPROC_LP_Q_TX_RPID4_ACPU0

// sub_cmd
#define LPM_DEV_SUB_CMD_QUERY_SOC_TEMP     81
#define LPM_DEV_SUB_CMD_QUERY_AICORE_FREQ  83
#define LPM_DEV_SUB_CMD_QUERY_VECTOR_FREQ  84
#define LPM_DEV_SUB_CMD_QUERY_CTRLCPU_FREQ 85
#define LPM_DEV_SUB_CMD_QUERY_DDR_FREQ     87

// cmd
#define LPM_DEV_CMD_INQUIRY  1

// CMD source & dest define
#define LPM_DEV_OBJ_LP       4
#define LPM_DEV_OBJ_TAISHAN0 0

#elif defined(CFG_SOC_PLATFORM_MINIV2) || defined(CFG_SOC_PLATFORM_HELPER)

// multiple chips, each chip has one die
// or two chips share one os
#define LPM_DEVMNG_DIE_OFFSET            0ULL
#define LPM_DEVMNG_CHIP_OFFSET           0x8000000000ULL
#define LPM_DEVMNG_DEV_MAX_NUM           2U

// config for sysctl reg
#define LPM_DEVMNG_SYSCTL_REG_BASE_ADDR      0x80000000ULL
#define LPM_DEVMNG_SYSCTL_REG_SIZE           0x10000ULL
#define LPM_DEVMNG_SYSCTL_VERSION_REG_OFFSET 0xFFFCULL

#elif defined(CFG_SOC_PLATFORM_MDC_V51_LITE)
// one chips, each chip has one die
#define LPM_DEVMNG_DIE_OFFSET            0ULL
#define LPM_DEVMNG_CHIP_OFFSET           0x8000000000ULL
#define LPM_DEVMNG_DEV_MAX_NUM           1U

// config for sysctl reg
#define LPM_DEVMNG_SYSCTL_REG_BASE_ADDR      0x80000000ULL
#define LPM_DEVMNG_SYSCTL_REG_SIZE           0x10000ULL
#define LPM_DEVMNG_SYSCTL_VERSION_REG_OFFSET 0xFFFCULL

// config for IPC
#define LPM_IPC_TX_LP_ACPU0  HISI_RPROC_LP_Q_TX_RPID4_ACPU0

// CMD source & dest define
#define LPM_DEV_OBJ_LP       OBJ_CMD_LP
#define LPM_DEV_OBJ_TAISHAN0 OBJ_CMD_CCPU

#endif

// emu is 0x1
#define LPM_DEVMNG_PLAT_TYPE_FPGA        0x0
#define LPM_DEVMNG_PLAT_TYPE_ESL         0x2
#define LPM_DEVMNG_PLAT_TYPE_ASIC        0x3
#define LPM_DEVMNG_PLAT_TYPE_INVALID     0xFF

#define LPM_DEVMNG_PLAT_MASK             0x000F0000
#define LPM_DEVMNG_PLAT_OFFSET           16

#define LPM_DEVMNG_BOARD_ID_LEN 4

typedef int32_t (*fn_lpm_init)(uint64_t *param, uint32_t param_num);

typedef int32_t (*fn_devdrv_get_chip_die_id)(uint32_t dev_id, uint32_t *chip_id, uint32_t *die_id);
typedef bool (*fn_devdrv_is_pf_device)(uint32_t dev_id);
typedef void (*fn_devdrv_get_time_interval)(struct timespec64 *time);

typedef int32_t (*fn_lpm_report_alarm)(
	uint32_t dev_id, uint32_t alarm_type, uint32_t *alarm_info, uint32_t info_len);

typedef int32_t (*fn_transform_dev_id)(
	uint32_t logical_dev_id, uint32_t *physical_dev_id, uint32_t *vfid);

typedef int32_t (*fn_devdrv_is_in_container)(void);

#define LPM_DEVMNG_MODULE_NAME_LEN 30

struct lpm_common_init_table {
	// naming rules: "module function"
	char module_name[LPM_DEVMNG_MODULE_NAME_LEN];
	fn_lpm_init init;
	fn_lpm_init uninit;
};

struct lpm_common_dev_info {
	uint32_t chip_id;
	uint32_t die_id;
	uint32_t env_type; // read from version reg
	void __iomem *sysctl_base;
};

struct lpm_common_fn_hook {
	fn_devdrv_get_chip_die_id fn_get_chip_die_id;
	fn_transform_dev_id fn_trans_dev_id;
	fn_devdrv_is_in_container fn_is_in_container;
};

struct lpm_common_priv {
	uint32_t dev_num;
	struct lpm_common_fn_hook fn_hook;
	struct lpm_common_dev_info dev_priv[LPM_DEVMNG_DEV_MAX_NUM];
};

#if defined(CFG_SOC_PLATFORM_MINIV3) || defined(CFG_SOC_PLATFORM_CLOUD_V2) || defined(CFG_SOC_PLATFORM_MDC_V51) || \
	defined(CFG_SOC_PLATFORM_MDC_V51_LITE)
enum lpm_devmng_core_id {
	LPM_CLUSTER_ID = 0,
	LPM_PERI_ID = 1,
	LPM_TS_ID = 2,
	LPM_DDR_ID = 3,
	LPM_AICORE0_ID = 4,
	LPM_AICORE1_ID = 5,
	LPM_HBM_ID = 6,
	LPM_VECTOR_ID = 7,
	LPM_SOC_ID = 8,
	LPM_N_DIE_ID,
	LPM_THERMAL_THRESHOLD_ID,
	LPM_INVALID_ID,
};

// CFG_SOC_PLATFORM_MINIV3 (rc)
#define VDAVINCI_VDEV_OFFSET  32U
#define VDAVINCI_MAX_VFID_NUM 4U

struct lpm_devmng_dsmi_in_param {
	char *in;
	uint32_t in_len;
	uint32_t correct_in_len;
	char *out;
	uint32_t out_len;
	uint32_t correct_out_len;
};

struct lpm_devmng_dsmi_cfg_in {
	uint32_t dev_id;
	uint32_t vfid;
	uint32_t core_id;
	uint32_t sub_cmd;
};

#endif


int32_t lpm_common_probe(uint64_t *param, uint32_t param_num);
int32_t lpm_common_remove(uint64_t *param, uint32_t param_num);

uint16_t lpm_devmng_crc16(const uint8_t *data, uint16_t len);
int32_t lpm_devmng_module_uninit(struct lpm_common_init_table *table_list,
	uint32_t table_num, uint64_t *param, uint32_t param_num);
int32_t lpm_devmng_module_init(struct lpm_common_init_table *table_list,
	uint32_t table_num, uint64_t *param, uint32_t param_num);

bool lpm_common_check_dev_id(uint32_t dev_id);
uint32_t lpm_common_get_dev_num(void);
void lpm_common_get_chip_die_id(uint32_t dev_id, uint32_t *chip_id, uint32_t *die_id);

void lpm_dev_common_get_current_time(struct timespec64 *os_time);

int32_t lpm_check_dsmi_get_param(
	uint32_t dev_id, char *in, uint32_t in_len, uint32_t expect_len, uint32_t *out_len);
bool lpm_common_check_is_in_container(void);

#if defined(CFG_SOC_PLATFORM_MINIV3) || defined(CFG_SOC_PLATFORM_CLOUD_V2) || defined(CFG_SOC_PLATFORM_MDC_V51) || \
	defined(CFG_SOC_PLATFORM_MDC_V51_LITE)
bool lpm_common_check_dsmi_in_param(struct lpm_devmng_dsmi_in_param *in_param);
int32_t lpm_common_check_dev_and_core(uint32_t dev_id, uint32_t core_id);
int32_t lpm_common_trans_logical_id_to_pf_id(uint32_t dev_id, uint32_t *pf_id);
#endif

#endif
