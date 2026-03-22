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

#include <linux/types.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/version.h>
#include <linux/timekeeping.h>
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
#include "lpm_devmng_debugfs.h"
#endif
#include "devdrv_manager_comm.h"
#include "lpm_devmng_common.h"

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
STATIC int32_t lpm_common_debugfs_init(void);
#endif

#define LPM_DEVMNG_SILS_CRC_POLYNOMIAL 0x1021U
#define LPM_DEVMNG_BITS_PER_BYTE       8U
#define LPM_DEVMNG_BIT15               0x8000U

// calculating crc16
uint16_t lpm_devmng_crc16(const uint8_t *data, uint16_t len)
{
	uint32_t i;
	uint16_t val = 0xFFFFU;
	const uint16_t poly = LPM_DEVMNG_SILS_CRC_POLYNOMIAL;

	if (data == NULL) {
		lpm_log_warn("data is null when calc crc16\n");
		return 0;
	}

	while (len > 0) {
		uint8_t ch = *data;
		val ^= ((uint16_t)ch << LPM_DEVMNG_BITS_PER_BYTE);
		for (i = 0; i < LPM_DEVMNG_BITS_PER_BYTE; i++) {
			if ((val & LPM_DEVMNG_BIT15) != 0) {
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

// uninitialize the list of modules
// table_list: the list of modules
// table_num: the number of modules
// param: input parameter information carried when uninitialization
// param_num: input parameter number
int32_t lpm_devmng_module_uninit(struct lpm_common_init_table *table_list,
	uint32_t table_num, uint64_t *param, uint32_t param_num)
{
	int32_t ret;
	int32_t final_ret = 0;
	uint32_t module_id;

	// reverse order deinitialization
	for (module_id = 1; module_id <= table_num; module_id++) {
		if (table_list[table_num - module_id].uninit == NULL) {
			continue;
		}
		ret = table_list[table_num - module_id].uninit(param, param_num);
		if (ret != 0) {
			lpm_log_err("lpm %s module uninit failed, ret=%d\n",
				table_list[table_num - module_id].module_name, ret);
			final_ret = -1;
		}
	}
	return final_ret;
}

// initialize the list of modules
// table_list: the list of modules
// table_num: the number of modules
// param: input parameter information carried when initialization
// param_num: input parameter number
int32_t lpm_devmng_module_init(struct lpm_common_init_table *table_list,
	uint32_t table_num, uint64_t *param, uint32_t param_num)
{
	int32_t ret;
	uint32_t module_id;
	uint32_t i;

	for (module_id = 0; module_id < table_num; module_id++) {
		if (table_list[module_id].init == NULL) {
			continue;
		}
		ret = table_list[module_id].init(param, param_num);
		if (ret != 0) {
			lpm_log_err("lpm %s module init failed, ret=%d\n",
				table_list[module_id].module_name, ret);
			goto init_failed;
		}
	}
	return 0;

init_failed:
	// sequential initialization, reverse order deinitialization
	for (i = 1; i <= module_id; i++) {
		if (table_list[module_id - i].uninit == NULL) {
			continue;
		}
		ret = table_list[module_id - i].uninit(param, param_num);
		if (ret != 0) {
			lpm_log_err("lpm %s rollback module init failed, ret=%d\n",
				table_list[module_id - i].module_name, ret);
		}
	}
	return -1;
}

STATIC struct lpm_common_priv *lpm_common_priv_info(void)
{
	static struct lpm_common_priv lpm_comm_priv = {0};
	return &lpm_comm_priv;
}

STATIC struct lpm_common_dev_info *lpm_common_dev_priv_info(uint32_t dev_id)
{
	struct lpm_common_priv *comm_priv = lpm_common_priv_info();
	return &comm_priv->dev_priv[dev_id];
}

STATIC int32_t lpm_common_put_opt_hook(uint64_t *param, uint32_t param_num)
{
	struct lpm_common_priv *comm_priv = lpm_common_priv_info();

	comm_priv->fn_hook.fn_get_chip_die_id = NULL;

#ifdef LPM_TRANSFORM_DEVID
	if (comm_priv->fn_hook.fn_trans_dev_id != NULL) {
		__symbol_put("devdrv_manager_container_logical_id_to_physical_id");
		comm_priv->fn_hook.fn_trans_dev_id = NULL;
	}
#endif

#ifdef CFG_SOC_PLATFORM_MINIV3
	if (comm_priv->fn_hook.fn_is_in_container != NULL) {
		__symbol_put("devdrv_manager_container_is_in_container");
		comm_priv->fn_hook.fn_is_in_container = NULL;
	}
#endif
	(void)param;
	(void)param_num;
	return 0;
}

STATIC void lpm_common_init_opt_hook(struct lpm_common_priv *comm_priv)
{
	comm_priv->fn_hook.fn_get_chip_die_id = NULL;
	comm_priv->fn_hook.fn_trans_dev_id    = NULL;
	comm_priv->fn_hook.fn_is_in_container = NULL;
}

STATIC int32_t lpm_common_get_opt_hook(uint64_t *param, uint32_t param_num)
{
	struct lpm_common_priv *comm_priv = lpm_common_priv_info();

	lpm_common_init_opt_hook(comm_priv);

	/*
	 * Upstream 6.18 rejects the runtime symbol_get path for this exported
	 * symbol. Bind directly so modprobe/depmod manages the dependency.
	 */
	comm_priv->fn_hook.fn_get_chip_die_id = devdrv_get_chip_die_id;

#ifdef LPM_TRANSFORM_DEVID
	comm_priv->fn_hook.fn_trans_dev_id =
		(fn_transform_dev_id)(uintptr_t) \
		__symbol_get("devdrv_manager_container_logical_id_to_physical_id");
	if (comm_priv->fn_hook.fn_trans_dev_id == NULL) {
		lpm_log_err("can not find devdrv manager container logical id to physical id\n");
		(void)lpm_common_put_opt_hook(param, param_num);
		return -1;
	}
#endif

#ifdef CFG_SOC_PLATFORM_MINIV3
	comm_priv->fn_hook.fn_is_in_container =
		(fn_devdrv_is_in_container)(uintptr_t) \
		__symbol_get("devdrv_manager_container_is_in_container");
	if (comm_priv->fn_hook.fn_is_in_container == NULL) {
		lpm_log_err("can not find devdrv manager container is in container\n");
		(void)lpm_common_put_opt_hook(param, param_num);
		return -1;
	}
#endif
	return 0;
}

STATIC int32_t lpm_common_parse_dev_id(
	uint64_t *param, uint32_t param_num, uint32_t *dev_id)
{
	struct lpm_common_priv *comm_priv = lpm_common_priv_info();

	if ((param == NULL) || (param_num != 0x1)) {
		lpm_log_err("common check dev id failed, "
			"param is null or param_num is out of range, param_num=%u\n",
			param_num);
		return -1;
	}

	*dev_id = (uint32_t)param[0];
	if (*dev_id >= comm_priv->dev_num) {
		lpm_log_err("dev_id=%u is out of range, dev_num=%u\n", *dev_id, comm_priv->dev_num);
		return -1;
	}

	return 0;
}

STATIC int32_t lpm_common_init_chip_die_id(uint64_t *param, uint32_t param_num)
{
	int32_t ret;
	uint32_t dev_id = 0;
	uint32_t chip_id = 0;
	uint32_t die_id = 0;
	struct lpm_common_priv *comm_priv = lpm_common_priv_info();

	ret = lpm_common_parse_dev_id(param, param_num, &dev_id);
	if (ret != 0) {
		lpm_log_err("lpm common param parse dev_id failed, param_num=%u\n", param_num);
		return -1;
	}

	// just mini not support
	if (comm_priv->fn_hook.fn_get_chip_die_id != NULL) {
		ret = comm_priv->fn_hook.fn_get_chip_die_id(dev_id, &chip_id, &die_id);
		if (ret != 0) {
			lpm_log_err("dev_id=%u get chip die id failed, ret=%d\n", dev_id, ret);
			return -1;
		}
	}

	comm_priv->dev_priv[dev_id].chip_id = chip_id;
	comm_priv->dev_priv[dev_id].die_id  = die_id;
	return 0;
}

STATIC int32_t lpm_common_remap_addr(uint64_t *param, uint32_t param_num)
{
	int32_t ret;
	uint32_t dev_id = 0;
	struct lpm_common_dev_info *comm_dev_priv = NULL;
	unsigned long sysctl_base_addr;

	ret = lpm_common_parse_dev_id(param, param_num, &dev_id);
	if (ret != 0) {
		lpm_log_err("lpm common param parse dev id failed, param_num=%d\n", param_num);
		return -1;
	}

	comm_dev_priv = lpm_common_dev_priv_info(dev_id);
	sysctl_base_addr = LPM_DEVMNG_SYSCTL_REG_BASE_ADDR +
		(LPM_DEVMNG_CHIP_OFFSET * (unsigned long)comm_dev_priv->chip_id) +
		(LPM_DEVMNG_DIE_OFFSET * (unsigned long)comm_dev_priv->die_id);

	comm_dev_priv->sysctl_base = ioremap(sysctl_base_addr, LPM_DEVMNG_SYSCTL_REG_SIZE);
	if (comm_dev_priv->sysctl_base == NULL) {
		lpm_log_err("ioremap sysctl fail, dev_id=%u\n", dev_id);
		return -EINVAL;
	}

	return 0;
}

STATIC int32_t lpm_common_unremap_addr(uint64_t *param, uint32_t param_num)
{
	int32_t ret;
	uint32_t dev_id = 0;
	struct lpm_common_dev_info *comm_dev_priv = NULL;

	ret = lpm_common_parse_dev_id(param, param_num, &dev_id);
	if (ret != 0) {
		lpm_log_err("lpm common param parse dev id failed, param_num=%u\n", param_num);
		return -1;
	}
	comm_dev_priv = lpm_common_dev_priv_info(dev_id);
	if (comm_dev_priv->sysctl_base != NULL) {
		iounmap(comm_dev_priv->sysctl_base);
		comm_dev_priv->sysctl_base = NULL;
	}
	return 0;
}

/*
 * vrsion reg info:
 * CFG_SOC_PLATFORM_HELPER & CFG_SOC_PLATFORM_MINIV2 &
 * CFG_SOC_PLATFORM_MDC_V51 & CFG_SOC_PLATFORM_CLOUD_V2
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
STATIC int32_t lpm_common_env_type_init(uint64_t *param, uint32_t param_num)
{
	int32_t ret;
	uint32_t dev_id = 0;
	uint32_t reg_val;
	uint32_t type;
	uintptr_t version_addr;
	struct lpm_common_dev_info *comm_dev_priv = NULL;

	ret = lpm_common_parse_dev_id(param, param_num, &dev_id);
	if (ret != 0) {
		lpm_log_err("lpm common param parse dev id failed, param_num=%u\n", param_num);
		return -1;
	}

	comm_dev_priv = lpm_common_dev_priv_info(dev_id);
	comm_dev_priv->env_type = LPM_DEVMNG_PLAT_TYPE_INVALID;

	version_addr = (uintptr_t)comm_dev_priv->sysctl_base + (uintptr_t)LPM_DEVMNG_SYSCTL_VERSION_REG_OFFSET;
	reg_val = readl((void __iomem *)version_addr);
	if (reg_val == 0) {
		comm_dev_priv->env_type = LPM_DEVMNG_PLAT_TYPE_ASIC;
		return 0;
	}

	type = (reg_val & (uint32_t)LPM_DEVMNG_PLAT_MASK) >> LPM_DEVMNG_PLAT_OFFSET;
	if (type > LPM_DEVMNG_PLAT_TYPE_ESL) {
		// use default value: LPM_DEVMNG_PLAT_TYPE_INVALID
		lpm_log_warn("dev_id=%u invalid plat type, reg_val=%u\n", dev_id, reg_val);
	} else {
		comm_dev_priv->env_type = type;
	}

	return 0;
}

STATIC struct lpm_common_init_table *lpm_common_get_init_table(uint32_t *table_num)
{
	static struct lpm_common_init_table comm_init_table[] = {
		{"common opt hook",   lpm_common_get_opt_hook,     lpm_common_put_opt_hook}
	};

	*table_num = (uint32_t)ARRAY_SIZE(comm_init_table);
	return &comm_init_table[0];
}

STATIC struct lpm_common_init_table *lpm_common_get_dev_init_table(uint32_t *table_num)
{
	static struct lpm_common_init_table dev_init_table[] = {
		{"common chip die id",  lpm_common_init_chip_die_id, NULL},
		{"common remap",        lpm_common_remap_addr,       lpm_common_unremap_addr},
		// the calls order of the above cannot be reversed
		{"common env type",     lpm_common_env_type_init,   NULL},
	};

	*table_num = (uint32_t)ARRAY_SIZE(dev_init_table);
	return &dev_init_table[0];
}

STATIC int32_t lpm_common_uninit_module(void)
{
	int32_t ret;
	uint32_t table_num = 0;
	struct lpm_common_init_table *table_list = lpm_common_get_init_table(&table_num);

	ret = lpm_devmng_module_uninit(table_list, table_num, NULL, 0);
	if (ret != 0) {
		lpm_log_err("lpm common uninit module failed, ret=%d, table_num=%u\n",
			ret, table_num);
		return -1;
	}
	return 0;
}

STATIC int32_t lpm_common_init_module(void)
{
	int32_t ret;
	uint32_t table_num = 0;
	struct lpm_common_priv *comm_priv = lpm_common_priv_info();
	struct lpm_common_init_table *table_list = lpm_common_get_init_table(&table_num);

	ret = lpm_devmng_module_init(table_list, table_num, NULL, 0);
	if (ret != 0) {
		lpm_log_err("lpm common init module failed, ret=%d, table_num=%u\n",
			ret, table_num);
		return ret;
	}

	lpm_log_info("lpm common init success, dev_num=%u\n", comm_priv->dev_num);
	return 0;
}

STATIC int32_t lpm_common_dev_uninit_module(uint32_t dev_num)
{
	int32_t ret;
	int32_t total_ret = 0;
	uint32_t table_num = 0;
	uint32_t dev_id;
	uint64_t param;
	struct lpm_common_init_table *table_list = lpm_common_get_dev_init_table(&table_num);

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		param = dev_id;
		ret = lpm_devmng_module_uninit(table_list, table_num, &param, 0x1);
		if (ret != 0) {
			lpm_log_err("lpm common uninit dev module failed, ret=%d, table_num=%u, dev_id=%u\n",
				ret, table_num, dev_id);
			// try best to uninit
			total_ret = -1;
		}
	}

	return total_ret;
}

STATIC int32_t lpm_common_dev_init_module(uint32_t dev_num)
{
	int32_t ret;
	uint32_t table_num = 0;
	uint32_t dev_id;
	uint64_t param;
	struct lpm_common_dev_info *comm_dev_priv = NULL;
	struct lpm_common_init_table *table_list = lpm_common_get_dev_init_table(&table_num);

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		param = dev_id;
		ret = lpm_devmng_module_init(table_list, table_num, &param, 0x1);
		if (ret != 0) {
			lpm_log_err("lpm common init dev module failed, ret=%d, table_num=%u, dev_id=%u\n",
				ret, table_num, dev_id);
			goto dev_init_fail;
		}

		comm_dev_priv = lpm_common_dev_priv_info(dev_id);
		lpm_log_info("lpm common dev init success, "
			"dev_id=%u, chip_id=%u, die_id=%u, env_type=%u\n",
			dev_id, comm_dev_priv->chip_id, comm_dev_priv->die_id,
			comm_dev_priv->env_type);
	}

	return 0;

dev_init_fail:
	ret = lpm_common_dev_uninit_module(dev_id);
	if (ret != 0) {
		lpm_log_err("lpm common rollback dev module init failed, ret=%d\n", ret);
	}
	return -1;
}

STATIC int32_t lpm_common_priv_init(uint32_t dev_num)
{
	int32_t ret;

	ret = lpm_common_init_module();
	if (ret != 0) {
		lpm_log_err("lpm common init module failed, ret=%d\n", ret);
		return -1;
	}

	ret = lpm_common_dev_init_module(dev_num);
	if (ret != 0) {
		(void)lpm_common_uninit_module();
		lpm_log_err("lpm common init dev module failed, ret=%d\n", ret);
		return -1;
	}

	return 0;
}

STATIC void lpm_common_priv_uninit(void)
{
	int32_t ret;
	struct lpm_common_priv *comm_priv = lpm_common_priv_info();

	ret = lpm_common_dev_uninit_module(comm_priv->dev_num);
	if (ret != 0) {
		lpm_log_err("lpm common uninit dev module failed, ret=%d\n", ret);
	}

	ret = lpm_common_uninit_module();
	if (ret != 0) {
		lpm_log_err("lpm common uninit module failed, ret=%d\n", ret);
	}
}

STATIC int32_t lpm_common_check_dev_num(
	uint64_t *param, uint32_t param_num, uint32_t *dev_num)
{
	if ((param == NULL) || (param_num != 0x1)) {
		lpm_log_err("common check dev num failed, "
			"param is null or param_num is out of range, param_num=%u\n",
			param_num);
		return -EINVAL;
	}

	*dev_num = (uint32_t)param[0];
	if (*dev_num > LPM_DEVMNG_DEV_MAX_NUM) {
		lpm_log_err("dev_num=%u is out of range, max_dev_num=%u\n",
			*dev_num, LPM_DEVMNG_DEV_MAX_NUM);
		return -EINVAL;
	}

	return 0;
}

int32_t lpm_common_probe(uint64_t *param, uint32_t param_num)
{
	int32_t ret;
	uint32_t dev_num = 0;
	struct lpm_common_priv *comm_priv = lpm_common_priv_info();

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
	ret = lpm_common_debugfs_init();
	if (ret != 0) {
		lpm_log_err("lpm common init debugfs failed, ret=%d\n", ret);
		return ret;
	}
#endif

	ret = lpm_common_check_dev_num(param, param_num, &dev_num);
	if (ret != 0) {
		lpm_log_err("lpm common probe check dev_num failed, ret=%d, param_num=%u\n",
			ret, param_num);
		return -1;
	}

	comm_priv->dev_num = dev_num;

	ret = lpm_common_priv_init(dev_num);
	if (ret != 0) {
		lpm_log_err("lpm common init priv data failed, ret=%d\n", ret);
		return -1;
	}

	lpm_log_info("lpm common probe success\n");
	return 0;
}

int32_t lpm_common_remove(uint64_t *param, uint32_t param_num)
{
	lpm_common_priv_uninit();

	(void)param;
	(void)param_num;
	lpm_log_info("lpm common remove success\n");
	return 0;
}

bool lpm_common_check_dev_id(uint32_t dev_id)
{
	struct lpm_common_priv *comm_priv = lpm_common_priv_info();
	return (dev_id < comm_priv->dev_num);
}

uint32_t lpm_common_get_dev_num(void)
{
	struct lpm_common_priv *comm_priv = lpm_common_priv_info();
	return comm_priv->dev_num;
}

void lpm_common_get_chip_die_id(uint32_t dev_id, uint32_t *chip_id, uint32_t *die_id)
{
	struct lpm_common_dev_info *comm_dev_priv = lpm_common_dev_priv_info(dev_id);
	*chip_id = comm_dev_priv->chip_id;
	*die_id  = comm_dev_priv->die_id;
}

void lpm_dev_common_get_current_time(struct timespec64 *os_time)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
	ktime_get_real_ts64(os_time);
#else
	*os_time = current_kernel_time64();
#endif
}

int32_t lpm_check_dsmi_get_param(
	uint32_t dev_id, char *in, uint32_t in_len, uint32_t expect_len, uint32_t *out_len)
{
	if (!lpm_common_check_dev_id(dev_id)) {
		lpm_log_err("check dsmi get param failed, dev_id=%u is out of range\n", dev_id);
		return -ENODEV;
	}

	if ((in == NULL) || (in_len != expect_len)) {
		lpm_log_err("check dsmi get param failed, input char is NULL or in_len is wrong, "
			"in_len=%u, expect_len=%u\n", in_len, expect_len);
		return -EINVAL;
	}

	if (out_len == NULL) {
		lpm_log_err("check dsmi get param failed, out_len is NULL\n");
		return -EINVAL;
	}

	return 0;
}

#if defined(CFG_SOC_PLATFORM_MINIV3) || defined(CFG_SOC_PLATFORM_CLOUD_V2) || defined(CFG_SOC_PLATFORM_MDC_V51)

STATIC bool lpm_common_check_core_id(uint32_t core_id)
{
	return (core_id < (uint32_t)LPM_INVALID_ID);
}

bool lpm_common_check_dsmi_in_param(struct lpm_devmng_dsmi_in_param *in_param)
{
	if ((in_param->in == NULL) || (in_param->in_len != in_param->correct_in_len)) {
		lpm_log_err("Input char is NULL or in_len is wrong. (in_len=%u; correct_in_len=%u)\n",
			in_param->in_len, in_param->correct_in_len);
		return false;
	}

	if ((in_param->out == NULL) || (in_param->out_len != in_param->correct_out_len)) {
		lpm_log_err("Output char is NULL or out_len is wrong. (out_len=%u; correct_out_len=%u)\n",
			in_param->out_len, in_param->correct_out_len);
		return false;
	}

	return true;
}

int32_t lpm_common_check_dev_and_core(uint32_t dev_id, uint32_t core_id)
{
	if (!lpm_common_check_dev_id(dev_id)) {
		lpm_log_err("lpm common check dev_id=%u failed\n", dev_id);
		return -ENODEV;
	}
	if (!lpm_common_check_core_id(core_id)) {
		lpm_log_err("lpm common check core_id=%u failed\n", core_id);
		return -EINVAL;
	}
	return 0;
}

int32_t lpm_common_trans_logical_id_to_pf_id(uint32_t dev_id, uint32_t *pf_id)
{
	int32_t ret;
	uint32_t vfid;
	uint32_t physical_dev_id = dev_id;
	struct lpm_common_priv *comm_priv = lpm_common_priv_info();

	if (comm_priv->fn_hook.fn_trans_dev_id != NULL) {
		// transform logical id
		ret = comm_priv->fn_hook.fn_trans_dev_id(dev_id, &physical_dev_id, &vfid);
		if (ret != 0) {
			lpm_log_err("transform logical dev_id to physical id failed, dev_id=%u, ret=%d\n",
				dev_id, ret);
			return -EINVAL;
		}

		if (physical_dev_id >= (uint32_t)VDAVINCI_VDEV_OFFSET) {
			// transfrom vdev_id to pf_id
			physical_dev_id =
				(physical_dev_id - (uint32_t)VDAVINCI_VDEV_OFFSET) / (uint32_t)VDAVINCI_MAX_VFID_NUM;
		}
	}
	*pf_id = physical_dev_id;

	return 0;
}

#endif

// 0: not in container, 1: in container
bool lpm_common_check_is_in_container(void)
{
	int32_t ret = 0; // default value: 0: not in container
	struct lpm_common_priv *comm_priv = lpm_common_priv_info();

	if (comm_priv->fn_hook.fn_is_in_container != NULL) {
		ret = comm_priv->fn_hook.fn_is_in_container();
	}
	return (ret == 0x1) ? true : false;
}

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
STATIC void lpm_common_show_dev_info(struct seq_file *seq)
{
	uint32_t dev_id;
	struct lpm_common_dev_info *dev_priv = NULL;
	struct lpm_common_priv *comm_priv = lpm_common_priv_info();
	const char *env_desc[] = {"fpga", "emu", "esl", "asic", "invalid"};

	seq_printf(seq, "[dev_num=%u]\n", comm_priv->dev_num);

	for (dev_id = 0; dev_id < comm_priv->dev_num; dev_id++) {
		dev_priv = &comm_priv->dev_priv[dev_id];

		seq_printf(seq, " dev_id=%2u, chip_id=%u, die_id=%u, env_type=%s\n",
			dev_id, dev_priv->chip_id, dev_priv->die_id,
			(dev_priv->env_type <= LPM_DEVMNG_PLAT_TYPE_ASIC) ? env_desc[dev_priv->env_type] : env_desc[0x4]);
	}
}

STATIC int32_t lpm_common_debugfs_show(struct seq_file *seq, void *v)
{
	(void)v;
	lpm_common_show_dev_info(seq);
	return 0;
}

STATIC int32_t lpm_common_debugfs_open(struct inode *inode, struct file *file)
{
	return single_open(file, lpm_common_debugfs_show, inode->i_private);
}

STATIC int32_t lpm_common_debugfs_init(void)
{
	int32_t ret;
	struct lpm_debugfs_init_cfg init_cfg = {
		.name = "common",
		.fn_open = lpm_common_debugfs_open,
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
