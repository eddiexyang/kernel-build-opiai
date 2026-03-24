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
*/

#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/time.h>
#include <linux/time64.h>
#include <linux/timekeeping.h>
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
#include "lpm_fault_debugfs.h"
#endif
#include "lpm_fault_common.h"

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
STATIC int32_t lpm_common_debugfs_init(void);
#endif

STATIC struct lpm_fault_common_priv *lpm_fault_get_common_priv(void)
{
	static struct lpm_fault_common_priv lpm_common_priv = {0};
	return &lpm_common_priv;
}

STATIC struct lpm_fault_common_dev_info *lpm_common_dev_priv_info(uint32_t dev_id)
{
	struct lpm_fault_common_priv *common_priv = lpm_fault_get_common_priv();
	return &common_priv->dev_priv[dev_id];
}

STATIC void lpm_common_init_priv_data(uint32_t dev_num)
{
	uint32_t dev_id;
	struct lpm_fault_common_priv *common_priv = lpm_fault_get_common_priv();
	struct lpm_fault_common_dev_info *dev_priv = NULL;

	(void)memset_s(common_priv, sizeof(struct lpm_fault_common_priv),
		0, sizeof(struct lpm_fault_common_priv));

	common_priv->dev_num = dev_num;

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		dev_priv = &common_priv->dev_priv[dev_id];
		dev_priv->env_type = LPM_FAULT_PLAT_TYPE_INVALID;
	}
}

STATIC void lpm_common_put_opt_hook(void)
{
#ifndef LPM_FAULT_RUN_IN_AOS
#ifndef CFG_SOC_PLATFORM_MINI
	// aos core not support __symbol_put
	struct lpm_fault_common_priv *common_priv = lpm_fault_get_common_priv();

	if (common_priv->fn_hook.fn_get_chip_die_id != NULL) {
		__symbol_put("devdrv_get_chip_die_id");
		common_priv->fn_hook.fn_get_chip_die_id = NULL;
	}
#endif
#endif // !LPM_FAULT_RUN_IN_AOS
}

STATIC int32_t lpm_common_get_opt_hook(void)
{
#ifndef LPM_FAULT_RUN_IN_AOS
#ifndef CFG_SOC_PLATFORM_MINI
	struct lpm_fault_common_priv *common_priv = lpm_fault_get_common_priv();
	// aos core not support this api
	// mini not support this api
	common_priv->fn_hook.fn_get_chip_die_id =
		(fn_devdrv_get_chip_die_id)(uintptr_t)__symbol_get("devdrv_get_chip_die_id");
	if (common_priv->fn_hook.fn_get_chip_die_id == NULL) {
		lpm_log_err("can not find devdrv get chip die id\n");
		return -1;
	}
#endif
#endif // !LPM_FAULT_RUN_IN_AOS

	return 0;
}

STATIC int32_t lpm_common_init_chip_die_id(uint32_t dev_id)
{
	int32_t ret;
	uint32_t chip_id = 0;
	uint32_t die_id = 0;
	struct lpm_fault_common_priv *common_priv = lpm_fault_get_common_priv();

	// just mini and aos_core not support
	if (common_priv->fn_hook.fn_get_chip_die_id != NULL) {
		ret = common_priv->fn_hook.fn_get_chip_die_id(dev_id, &chip_id, &die_id);
		if (ret != 0) {
			lpm_log_err("dev_id=%u get chip die id failed, ret=%d\n", dev_id, ret);
			return -1;
		}
	}

	common_priv->dev_priv[dev_id].chip_id = chip_id;
	common_priv->dev_priv[dev_id].die_id  = die_id;
	lpm_log_info("dev_id=%u get chip_id=%u die_id=%u success\n", dev_id, chip_id, die_id);
	return 0;
}

/*
 * vrsion reg info:
 * CFG_SOC_PLATFORM_HELPER & CFG_SOC_PLATFORM_MINIV2 &
 * CFG_SOC_PLATFORM_MDC_V51 & CFG_SOC_PLATFORM_CLUD_V2
 * | env_type | [31:16] | [15: 0] |
 * |  ASIC    |   0x0   |   0x0   |
 * |  FPAG    |   0x0   | version |
 * |  EMU     |   0x1   | version |
 * |  ESL     |   0x2   | version |
 * CFG_SOC_PLATFORM_MINIV3
 * | env_type | [31:16] | [15: 4] | [ 3: 0] |
 * |  ASIC    |   0x0   |   0x0   |   0x0   |
 * |  FPAG    |   0x0   | version | version2|
 * |  EMU     |   0x1   | version | version2|
 * |  ESL     |   0x2   | version | version2|
 */
STATIC int32_t lpm_common_env_type_init(uint32_t dev_id)
{
	uint32_t reg_val;
	uint32_t type;
	struct lpm_fault_common_dev_info *dev_priv = lpm_common_dev_priv_info(dev_id);
	uintptr_t version_addr = (uintptr_t)dev_priv->sysctl_base + (uintptr_t)LPM_FAULT_SYSCTL_VERSION_REG_OFFSET;

	reg_val = readl((void __iomem *)version_addr);
	if (reg_val == 0) {
		dev_priv->env_type = LPM_FAULT_PLAT_TYPE_ASIC;
		return 0;
	}

	type = (reg_val & (uint32_t)LPM_FAULT_PLAT_MASK) >> (uint32_t)LPM_FAULT_PLAT_OFFSET;
	if (type > LPM_FAULT_PLAT_TYPE_ESL) {
		lpm_log_warn("dev_id=%u invalid plat type, reg_val=%u\n", dev_id, reg_val);
		dev_priv->env_type = LPM_FAULT_PLAT_TYPE_INVALID;
	} else {
		dev_priv->env_type = type;
	}

	return 0;
}

STATIC void lpm_common_phy_addr_unremap(uint32_t dev_id)
{
	struct lpm_fault_common_priv *common_priv = lpm_fault_get_common_priv();

#ifdef CFG_SOC_PLATFORM_MDC_V51
	if (common_priv->dev_priv[dev_id].syscnt_addr != NULL) {
		iounmap(common_priv->dev_priv[dev_id].syscnt_addr);
		common_priv->dev_priv[dev_id].syscnt_addr = NULL;
	}
#endif
	if (common_priv->dev_priv[dev_id].sysctl_base != NULL) {
		iounmap(common_priv->dev_priv[dev_id].sysctl_base);
		common_priv->dev_priv[dev_id].sysctl_base = NULL;
	}
}

STATIC int32_t lpm_common_phy_addr_remap(uint32_t dev_id)
{
	struct lpm_fault_common_dev_info *dev_priv = lpm_common_dev_priv_info(dev_id);
	unsigned long offset = (LPM_FAULT_CHIP_OFFSET * (unsigned long)dev_priv->chip_id) +
		(LPM_FAULT_DIE_OFFSET * (unsigned long)dev_priv->die_id);

#ifdef CFG_SOC_PLATFORM_MDC_V51
	dev_priv->syscnt_addr =
		ioremap(LPM_AO_SYSCNT_VALUE_ADDR + offset, LPM_AO_SYSCNT_VALUE_LENGTH);
	if (dev_priv->syscnt_addr == NULL) {
		lpm_log_err("syscount remap failed, dev_id=%u\n", dev_id);
		goto remap_failed;
	}
#endif
	dev_priv->sysctl_base =
		ioremap(LPM_FAULT_SYSCTL_REG_BASE_ADDR + offset, LPM_FAULT_SYSCTL_REG_SIZE);
	if (dev_priv->sysctl_base == NULL) {
		lpm_log_err("sysctl_base remap failed, dev_id=%u\n", dev_id);
		goto remap_failed;
	}

	return 0;

remap_failed:
	lpm_common_phy_addr_unremap(dev_id);
	return -1;
}

STATIC void lpm_fault_common_dev_uninit(uint32_t dev_num)
{
	uint32_t dev_id;

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		lpm_common_phy_addr_unremap(dev_id);
	}
}

STATIC int32_t lpm_fault_common_dev_init(uint32_t dev_num)
{
	int32_t ret;
	uint32_t dev_id;

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		ret = lpm_common_init_chip_die_id(dev_id);
		if (ret != 0) {
			lpm_log_err("lpm common init chip die id failed\n");
			return -1;
		}

		ret = lpm_common_phy_addr_remap(dev_id);
		if (ret != 0) {
			lpm_log_err("lpm common remap addr failed\n");
			goto remap_failed;
		}

		ret = lpm_common_env_type_init(dev_id);
		if (ret != 0) {
			lpm_log_err("lpm common init env type failed\n");
			goto init_pf_failed;
		}
	}
	return 0;

init_pf_failed:
	lpm_common_phy_addr_unremap(dev_id);
remap_failed:
	lpm_fault_common_dev_uninit(dev_id);
	return -1;
}

int32_t lpm_fault_common_init(uint32_t dev_num)
{
	int32_t ret;

	if (dev_num > LPM_DMS_NODE_MAX_NUM) {
		lpm_log_err("lpm common init failed, dev_num=%u shoud not large than %u\n",
			dev_num, LPM_DMS_NODE_MAX_NUM);
		return -1;
	}

	// should init first
	lpm_common_init_priv_data(dev_num);

	ret = lpm_common_get_opt_hook();
	if (ret != 0) {
		lpm_log_err("lpm common init get opt failed\n");
		return -1;
	}

	ret = lpm_fault_common_dev_init(dev_num);
	if (ret != 0) {
		lpm_common_put_opt_hook();
		lpm_log_err("lpm common init dev failed\n");
		return -1;
	}
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
	ret = lpm_common_debugfs_init();
	if (ret != 0) {
		lpm_fault_common_dev_uninit(dev_num);
		lpm_common_put_opt_hook();
		return -1;
	}
#endif
	lpm_log_info("lpm common init success, dev_num=%u\n", dev_num);
	return 0;
}

int32_t lpm_fault_common_exit(uint32_t dev_num)
{
	lpm_fault_common_dev_uninit(dev_num);
	lpm_common_put_opt_hook();
	lpm_log_info("lpm common exit success, dev_num=%u\n", dev_num);
	return 0;
}

// function desc: check dev_id is legal
// return value: true: legal, false:illegal
bool lpm_common_check_dev_id(uint32_t dev_id)
{
	struct lpm_fault_common_priv *common_priv = lpm_fault_get_common_priv();
	return (dev_id < common_priv->dev_num);
}

// function desc: get dev_num
// return value: dev num
uint32_t lpm_common_get_dev_num(void)
{
	struct lpm_fault_common_priv *common_priv = lpm_fault_get_common_priv();
	return common_priv->dev_num;
}

// function desc: obtain the timestamp of the lp fault report from the register
// return value: timestamp
uint64_t lpm_common_syscount_get_timestamp(uint32_t dev_id)
{
	uint32_t hi[2], lo;
	const uint64_t dword_bit_width = 32ULL;
	const uint64_t dword_length = (uint64_t)sizeof(uint32_t);
	const uint64_t freq = 384ULL;
	const uint64_t mul = 10ULL;
	uint64_t ticks;
	struct lpm_fault_common_priv *common_priv = lpm_fault_get_common_priv();

	if (!lpm_common_check_dev_id(dev_id)) {
		return 0U;
	}
	if (common_priv->dev_priv[dev_id].syscnt_addr == NULL) {
		return 0U;
	}

	do {
		hi[0] = readl((void __iomem *)((uintptr_t)common_priv->dev_priv[dev_id].syscnt_addr + dword_length));
		lo = readl((void __iomem *)common_priv->dev_priv[dev_id].syscnt_addr);
		hi[1] = readl((void __iomem *)((uintptr_t)common_priv->dev_priv[dev_id].syscnt_addr + dword_length));
	} while (hi[0] != hi[1]);

	ticks = ((uint64_t)hi[0] << dword_bit_width) | (uint64_t)lo;

	return (ticks * mul) / freq;
}

// function desc: query whether lp firmware is started successfully
// return value: true: success, false: failed
bool lpm_fault_query_lp_startup_status(uint32_t dev_id)
{
#if defined(CFG_SOC_PLATFORM_MDC_V51) || defined(CFG_SOC_PLATFORM_MINIV2) || defined(CFG_SOC_PLATFORM_HELPER)
	uint32_t reg_val;
	struct lpm_fault_common_dev_info *dev_priv = NULL;
	uintptr_t lp_status_addr;

	if (!lpm_common_check_dev_id(dev_id)) {
		return false;
	}

	dev_priv = lpm_common_dev_priv_info(dev_id);
	if (dev_priv->sysctl_base == NULL) {
		// do not remap, considering it need not to do check.
		return true;
	}

	lp_status_addr = (uintptr_t)dev_priv->sysctl_base + (uintptr_t)LPM_FAULT_SYSCTL_LP_STATUS_OFFSET;
	reg_val = readl((void __iomem *)lp_status_addr);
	if ((reg_val != LPM_FAULT_SYSCTL_LP_STATUS_CHECK_VALUE1) &&
		(reg_val != LPM_FAULT_SYSCTL_LP_STATUS_CHECK_VALUE2)) {
		lpm_log_err("lpm firmware start failed, ret=0x%x\n", reg_val);
		return false;
	}
#endif
	return true;
}

// function desc: get env type
// return value: fpga is 0x0, emu is 0x1, esl is 0x2, asic is 0x3, 0xff is invalid
// internal function, no need check dev_id
uint32_t lpm_common_get_env_type(uint32_t dev_id)
{
	struct lpm_fault_common_dev_info *dev_priv = lpm_common_dev_priv_info(dev_id);
	return dev_priv->env_type;
}

// function desc: get current time from os
// return value: current time
void lpm_common_get_current_time(struct timespec64 *os_time)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
	ktime_get_real_ts64(os_time);
#else
	*os_time = current_kernel_time64();
#endif
}

// function desc: get time zone info
// return value: time zone info
void lpm_common_get_time_interval(struct timespec64 *curr_time)
{
#ifndef LPM_FAULT_RUN_IN_AOS
	int32_t interval_seconds = sys_tz.tz_minuteswest * (-1);
	curr_time->tv_sec = (int64_t)interval_seconds * (int64_t)LPM_MINUTES_TO_SECOND;
	curr_time->tv_nsec = 0;
#else
	// aos core has no time zone info
	(void)memset_s(curr_time, sizeof(struct timespec64), 0, sizeof(struct timespec64));
#endif
}

void lpm_common_get_chip_die_id(uint32_t dev_id, uint32_t *chip_id, uint32_t *die_id)
{
	struct lpm_fault_common_dev_info *dev_priv = lpm_common_dev_priv_info(dev_id);
	*chip_id = dev_priv->chip_id;
	*die_id  = dev_priv->die_id;
}

#define LPM_FAULT_CRC_POLYNOMIAL          0x1021U
#define LPM_FAULT_NULL_USHORT             0xFFFFU
#define LPM_FAULT_BITS_PER_BYTE           8U
#define LPM_FAULT_BIT15                   0x8000U
uint16_t lpm_common_crc16(const uint8_t *data, uint16_t len)
{
	uint16_t val = LPM_FAULT_NULL_USHORT;
	const uint16_t poly = LPM_FAULT_CRC_POLYNOMIAL;
	uint8_t ch;
	uint32_t i;

	while (len > 0) {
		ch = *data;
		val ^= ((uint16_t)ch << LPM_FAULT_BITS_PER_BYTE);
		for (i = 0; i < LPM_FAULT_BITS_PER_BYTE; i++) {
			if ((val & LPM_FAULT_BIT15) != 0) {
				val = (val << 1) ^ poly;
			} else {
				val = val << 1;
			}
		}

		len--;
		data++;
	}
	return val;
}

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
STATIC void lpm_common_show_dev_info(struct seq_file *seq)
{
	uint32_t dev_id;
	struct timespec64 curr_time = {0};
	struct lpm_fault_common_dev_info *common_dev_info = NULL;
	struct lpm_fault_common_priv *common_priv = lpm_fault_get_common_priv();
	const char *env_desc[] = {"fpga", "emu", "esl", "asic", "invalid"};

	seq_printf(seq, "[dev_num=%u]\n", common_priv->dev_num);

	seq_printf(seq, "[time info]\n");
#ifndef LPM_FAULT_RUN_IN_AOS
	// aos core not support time zone info
	seq_printf(seq, "  [time zone info]\n");
	seq_printf(seq, "    sys_tz: tz_minuteswest=%d, tz_dsttime=%d\n",
		sys_tz.tz_minuteswest, sys_tz.tz_dsttime);
#endif

	seq_printf(seq, "  [UTC time info]\n");

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
	ktime_get_coarse_real_ts64(&curr_time);
	seq_printf(seq, "    ktime_get_coarse_real_ts64: tv_sec=%lld, tv_nsec=%ld\n",
		curr_time.tv_sec, curr_time.tv_nsec);
#else
	curr_time = current_kernel_time64();
	seq_printf(seq, "    current_kernel_time64: tv_sec=%lld, tv_nsec=%ld\n",
		curr_time.tv_sec, curr_time.tv_nsec);
#endif

	ktime_get_real_ts64(&curr_time);
	seq_printf(seq, "    ktime_get_real_ts64: tv_sec=%lld, tv_nsec=%ld\n",
		curr_time.tv_sec, curr_time.tv_nsec);

	seq_printf(seq, "[dev info]\n");
	for (dev_id = 0; dev_id < common_priv->dev_num; dev_id++) {
		common_dev_info = &common_priv->dev_priv[dev_id];

		seq_printf(seq, " dev_id=%2u, chip_id=%u, die_id=%u, env_type=%s\n",
			dev_id, common_dev_info->chip_id, common_dev_info->die_id,
			(common_dev_info->env_type <= LPM_FAULT_PLAT_TYPE_ASIC) ?
				env_desc[common_dev_info->env_type] : env_desc[0x4]);
	}
}
// for common
STATIC int32_t lpm_fault_debugfs_common_show(struct seq_file *seq, void *v)
{
	(void)v;
	lpm_common_show_dev_info(seq);
	return 0;
}

STATIC int32_t lpm_fault_debugfs_common_open(struct inode *inode, struct file *file)
{
	return single_open(file, lpm_fault_debugfs_common_show, inode->i_private);
}

STATIC int32_t lpm_common_debugfs_init(void)
{
	int32_t ret;
	struct lpm_debugfs_init_cfg init_cfg = {
		.name = "common",
		.fn_open = lpm_fault_debugfs_common_open,
		.fn_write = NULL
	};
	ret = lpm_debugfs_register_module(&init_cfg);
	if (ret != 0) {
		lpm_log_err("lpm common register debugfs failed, ret=%d\n", ret);
		return ret;
	}
	return 0;
}
#endif

#ifdef CFG_FEATURE_PARTIAL_GOOD
struct lpm_pg_info *lpm_crg_monitor_assemble_pginfo(void)
{
	static bool valid = false;
	static struct lpm_pg_info lpm_pg_info;
	pg_cmd_data cmd_data;
	int ret;
	u64 temp_data = 0;

	if (valid) {
		return &lpm_pg_info;
	}

	lpm_pg_info.cpu_mask = LPM_PG_INFO_MASK_INVAILID;
	lpm_pg_info.aic_mask = LPM_PG_INFO_MASK_INVAILID;
	lpm_pg_info.aiv_mask = LPM_PG_INFO_MASK_INVAILID;
	lpm_pg_info.aic_freq = LPM_PG_INFO_MASK_INVAILID;

	cmd_data.data = PG_DATA_TYPE_CORE_MAP;
	cmd_data.module = PG_MODULE_TYPE_CPU;
	ret = dms_get_pg_info(0, cmd_data, &temp_data);
	if (ret != 0) {
		lpm_log_err("lpm get cpu cluster info failed, ret=%d\n", ret);
		goto CHECK_RET;
	}
	lpm_pg_info.cpu_mask = (uint64_t)temp_data;

	cmd_data.module = PG_MODULE_TYPE_AIC;
	ret = dms_get_pg_info(0, cmd_data, &temp_data);
	if (ret != 0) {
		lpm_log_err("lpm get aic info failed, ret=%d\n", ret);
		goto CHECK_RET;
	}
	lpm_pg_info.aic_mask = (uint64_t)temp_data;

	cmd_data.module = PG_MODULE_TYPE_AIV;
	ret = dms_get_pg_info(0, cmd_data, &temp_data);
	if (ret != 0) {
		lpm_log_err("lpm get aiv info failed, ret=%d\n", ret);
		goto CHECK_RET;
	}
	lpm_pg_info.aiv_mask = (uint64_t)temp_data;

	cmd_data.data = PG_DATA_TYPE_FREQ;
	cmd_data.module = PG_MODULE_TYPE_AIC;
	ret = dms_get_pg_info(0, cmd_data, &temp_data);
	if (ret != 0) {
		lpm_log_err("lpm get aic freq info failed, ret=%d\n", ret);
		goto CHECK_RET;
	}
	lpm_pg_info.aic_freq = (uint64_t)temp_data;
	valid = true;

CHECK_RET:
	lpm_log_info("get pg mask cpu:0x%llx, aic:0x%llx, aiv:0x%llx, aic freq:%llu\n",
		lpm_pg_info.cpu_mask, lpm_pg_info.aic_mask, lpm_pg_info.aiv_mask, lpm_pg_info.aic_freq);
	return &lpm_pg_info;
}
#endif
