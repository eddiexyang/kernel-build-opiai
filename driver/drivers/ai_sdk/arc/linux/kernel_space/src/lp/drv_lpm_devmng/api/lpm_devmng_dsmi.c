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

#include <linux/of.h>
#include <linux/io.h>
#include "dms_define.h"
#include "dms_template.h"
#include "dms_cmd_def.h"
#include "urd_acc_ctrl.h"
#include "lpm_devmng_common.h"
#include "lpm_devmng_idle.h"
#include "lpm_devmng_profile.h"
#include "lpm_devmng_suspend.h"
#include "lpm_devmng_aic_freq.h"
#include "lpm_devmng_tool.h"
#include "lpm_devmng_turbo.h"
#include "lpm_devmng_power.h"
#include "lpm_devmng_frequency.h"
#include "lpm_devmng_temperature.h"
#include "lpm_devmng_volt_current.h"
#include "lpm_devmng_health_status.h"
#include "lpm_devmng_acg_cnt.h"
#include "lpm_devmng_temp_threshold.h"
#include "lpm_devmng_vrd_upgrade.h"
#include "lpm_devmng_soc_stress.h"
#include "lpm_devmng_dsmi.h"

// initialized in probe
static struct lpm_dsmi_feature_filter g_dsmi_filter[(uint32_t)LPM_DSMI_API_TYPE_MAX];

#if defined(LPM_DSMI_MAIN_CMD_LP) || defined(LPM_TEMP_THRESHOLD)
STATIC int32_t lpm_dsmi_get_info(char *in, uint32_t in_len, char *out, uint32_t out_len,
	const struct lpm_dsmi_get_device_info *get_info)
{
	int32_t ret;
	uint32_t result_len = 0;
	struct dms_get_device_info_in *in_cfg = NULL;
	struct dms_get_device_info_out *cfg_out = NULL;

	if ((in == NULL) || (in_len != sizeof(struct dms_get_device_info_in))) {
		lpm_log_err("get device info failed, input char is NULL or in_len is wrong, in_len=%u\n", in_len);
		return -EINVAL;
	}

	if ((out == NULL) || (out_len != sizeof(struct dms_get_device_info_out))) {
		lpm_log_err("get device info failed, output char is NULL or out_len is wrong, out_len=%u\n", out_len);
		return -EINVAL;
	}

	in_cfg = (struct dms_get_device_info_in *)in;
	cfg_out = (struct dms_get_device_info_out *)out;

	ret = get_info->fn_get(in_cfg->dev_id, in_cfg->buff, in_cfg->buff_size, &result_len);
	lpm_dsmi_update_statistics(ret, get_info->api_type);

	if (ret == -EOPNOTSUPP) {
		return ret;
	}
	if (ret != 0) {
		lpm_log_err("get device info failed, ret=%d, dev_id=%u, sub_cmd=%u\n",
			ret, in_cfg->dev_id, in_cfg->sub_cmd);
		return ret;
	}

	cfg_out->out_size = result_len;
	return 0;
}
#endif

#if defined(LPM_DSMI_MAIN_CMD_LP)
STATIC int32_t lpm_dsmi_set_info(char *in, uint32_t in_len,
	const struct lpm_dsmi_set_device_info *set_info)
{
	int32_t ret;
	struct dms_set_device_info_in *in_cfg = NULL;

	if ((in == NULL) || (in_len != (uint32_t)sizeof(struct dms_set_device_info_in))) {
		lpm_log_err("set device info failed, input char is NULL or in_len is wrong, in_len=%u\n", in_len);
		return -EINVAL;
	}
	in_cfg = (struct dms_set_device_info_in *)in;

	ret = set_info->fn_set(in_cfg->dev_id, in_cfg->buff, in_cfg->buff_size);
	lpm_dsmi_update_statistics(ret, set_info->api_type);

	if ((ret != 0) && (ret != -EOPNOTSUPP)) {
		lpm_log_err("set device info failed, ret=%d, dev_id=%u, sub_cmd=%u\n",
			ret, in_cfg->dev_id, in_cfg->sub_cmd);
	}
	return ret;
}

STATIC int32_t lpm_dsmi_set_tops(
	void *feature, char *in, uint32_t in_len, char *out, uint32_t out_len)
{
	struct lpm_dsmi_set_device_info set_info = {LPM_DSMI_SET_TOPS, lpm_profile_set_tops};
	(void)feature;
	(void)out;
	(void)out_len;
	return lpm_dsmi_set_info(in, in_len, &set_info);
}

STATIC int32_t lpm_dsmi_set_lptest_info(
	void *feature, char *in, uint32_t in_len, char *out, uint32_t out_len)
{
	struct lpm_dsmi_set_device_info set_info = {LPM_DSMI_SET_LPTEST, lpm_dsmi_set_lptest};
	(void)feature;
	(void)out;
	(void)out_len;
	return lpm_dsmi_set_info(in, in_len, &set_info);
}


STATIC int32_t lpm_dsmi_set_soc_stress(
	void *feature, char *in, uint32_t in_len, char *out, uint32_t out_len)
{
	struct lpm_dsmi_set_device_info set_info = {LPM_DSMI_SET_SOC_STRESS, lpm_set_soc_stress};
	(void)feature;
	(void)out;
	(void)out_len;
	return lpm_dsmi_set_info(in, in_len, &set_info);
}

STATIC int32_t lpm_dsmi_set_idle_switch(
	void *feature, char *in, uint32_t in_len, char *out, uint32_t out_len)
{
	struct lpm_dsmi_set_device_info set_info = {LPM_DSMI_SET_IDLE_SWITCH, lpm_idle_set_switch};
	(void)feature;
	(void)out;
	(void)out_len;
	return lpm_dsmi_set_info(in, in_len, &set_info);
}


STATIC int32_t lpm_dsmi_get_volt_current_aicore(
	void *feature, char *in, uint32_t in_len, char *out, uint32_t out_len)
{
	struct lpm_dsmi_get_device_info get_info = {LPM_DSMI_VOLTAGE_CURRENT_AICORE, lpm_volt_current_get_aicore};
	(void)feature;
	return lpm_dsmi_get_info(in, in_len, out, out_len, &get_info);
}

STATIC int32_t lpm_dsmi_get_volt_current_hybrid(
	void *feature, char *in, uint32_t in_len, char *out, uint32_t out_len)
{
	struct lpm_dsmi_get_device_info get_info = {LPM_DSMI_VOLTAGE_CURRENT_HYBRID, lpm_volt_current_get_hybrid};
	(void)feature;
	return lpm_dsmi_get_info(in, in_len, out, out_len, &get_info);
}

STATIC int32_t lpm_dsmi_get_volt_current_taishan(
	void *feature, char *in, uint32_t in_len, char *out, uint32_t out_len)
{
	struct lpm_dsmi_get_device_info get_info = {LPM_DSMI_VOLTAGE_CURRENT_TAISHAN, lpm_volt_current_get_taishan};
	(void)feature;
	return lpm_dsmi_get_info(in, in_len, out, out_len, &get_info);
}

STATIC int32_t lpm_dsmi_get_volt_current_ddr(
	void *feature, char *in, uint32_t in_len, char *out, uint32_t out_len)
{
	struct lpm_dsmi_get_device_info get_info = {LPM_DSMI_VOLTAGE_CURRENT_DDR, lpm_volt_current_get_ddr};
	(void)feature;
	return lpm_dsmi_get_info(in, in_len, out, out_len, &get_info);
}

STATIC int32_t lpm_dsmi_get_acg_cnt(
	void *feature, char *in, uint32_t in_len, char *out, uint32_t out_len)
{
	struct lpm_dsmi_get_device_info get_info = {LPM_DSMI_ACG_CNT, lpm_get_acg_cnt};
	(void)feature;
	return lpm_dsmi_get_info(in, in_len, out, out_len, &get_info);
}

STATIC int32_t lpm_dsmi_get_aic_acg_cpm(
	void *feature, char *in, uint32_t in_len, char *out, uint32_t out_len)
{
	struct lpm_dsmi_get_device_info get_info = {LPM_DSMI_GET_AIC_ACG_CPM, lpm_get_aicore_cpm};
	(void)feature;
	return lpm_dsmi_get_info(in, in_len, out, out_len, &get_info);
}

STATIC int32_t lpm_dsmi_get_bus_acg_cpm(
	void *feature, char *in, uint32_t in_len, char *out, uint32_t out_len)
{
	struct lpm_dsmi_get_device_info get_info = {LPM_DSMI_GET_BUS_ACG_CPM, lpm_get_bus_cpm};
	(void)feature;
	return lpm_dsmi_get_info(in, in_len, out, out_len, &get_info);
}

STATIC int32_t lpm_dsmi_get_idle_status(
	void *feature, char *in, uint32_t in_len, char *out, uint32_t out_len)
{
	struct lpm_dsmi_get_device_info get_info = {LPM_DSMI_IDLE_STATUS, lpm_idle_get_status};
	(void)feature;
	return lpm_dsmi_get_info(in, in_len, out, out_len, &get_info);
}

STATIC int32_t lpm_dsmi_get_current_tops(
	void *feature, char *in, uint32_t in_len, char *out, uint32_t out_len)
{
	struct lpm_dsmi_get_device_info get_info = {LPM_DSMI_GET_CUR_TOPS, lpm_profile_get_tops};
	(void)feature;
	return lpm_dsmi_get_info(in, in_len, out, out_len, &get_info);
}

STATIC int32_t lpm_dsmi_get_all_tops(
	void *feature, char *in, uint32_t in_len, char *out, uint32_t out_len)
{
	struct lpm_dsmi_get_device_info get_info = {LPM_DSMI_GET_ALL_TOPS, lpm_profile_get_tops_detail};
	(void)feature;
	return lpm_dsmi_get_info(in, in_len, out, out_len, &get_info);
}

STATIC int32_t lpm_dsmi_get_aicore_freq_adj_cause(
	void *feature, char *in, uint32_t in_len, char *out, uint32_t out_len)
{
	struct lpm_dsmi_get_device_info get_info = {LPM_DSMI_AICORE_FREQREDUC_CAUSE, lpm_aic_freq_get_adj_cause};
	(void)feature;
	return lpm_dsmi_get_info(in, in_len, out, out_len, &get_info);
}

STATIC int32_t lpm_dsmi_get_power_info(
	void *feature, char *in, uint32_t in_len, char *out, uint32_t out_len)
{
	struct lpm_dsmi_get_device_info get_info = {LPM_DSMI_GET_POWER_INFO, lpm_power_get_info};
	(void)feature;
	return lpm_dsmi_get_info(in, in_len, out, out_len, &get_info);
}

STATIC int32_t lpm_dsmi_get_lptest_info(
	void *feature, char *in, uint32_t in_len, char *out, uint32_t out_len)
{
	struct lpm_dsmi_get_device_info get_info = {LPM_DSMI_GET_LPTEST, lpm_dsmi_get_lptest};
	(void)feature;
	return lpm_dsmi_get_info(in, in_len, out, out_len, &get_info);
}
#endif

#ifdef LPM_TEMP_THRESHOLD
STATIC int32_t lpm_dsmi_get_ddr_gear(
	void *feature, char *in, uint32_t in_len, char *out, uint32_t out_len)
{
	struct lpm_dsmi_get_device_info get_info = {LPM_DSMI_TEMP_DDR, lpm_temp_threshold_get_ddr_gear};
	(void)feature;
	return lpm_dsmi_get_info(in, in_len, out, out_len, &get_info);
}

STATIC int32_t lpm_dsmi_get_ddr_high_temp_threshold(
	void *feature, char *in, uint32_t in_len, char *out, uint32_t out_len)
{
	struct lpm_dsmi_get_device_info get_info = {LPM_DSMI_TEMP_DDR_THOLD, lpm_temp_threshold_get_ddr_high};
	(void)feature;
	return lpm_dsmi_get_info(in, in_len, out, out_len, &get_info);
}

STATIC int32_t lpm_dsmi_get_soc_high_temp_threshold(
	void *feature, char *in, uint32_t in_len, char *out, uint32_t out_len)
{
	struct lpm_dsmi_get_device_info get_info = {LPM_DSMI_TEMP_SOC_THOLD, lpm_temp_threshold_get_soc_high};
	(void)feature;
	return lpm_dsmi_get_info(in, in_len, out, out_len, &get_info);
}

STATIC int32_t lpm_dsmi_get_soc_min_temp_threshold(
	void *feature, char *in, uint32_t in_len, char *out, uint32_t out_len)
{
	struct lpm_dsmi_get_device_info get_info = {LPM_DSMI_TEMP_SOC_MIN_THOLD, lpm_temp_threshold_get_soc_min};
	(void)feature;
	return lpm_dsmi_get_info(in, in_len, out, out_len, &get_info);
}
#endif

#ifdef LPM_HEALTH_STATUS
STATIC int32_t lpm_dsmi_get_lp_status(
	void *feature, char *in, uint32_t in_len, char *out, uint32_t out_len)
{
	int32_t ret = lpm_get_health_status(in, in_len, out, out_len);
	lpm_dsmi_update_statistics(ret, LPM_DSMI_GET_LP_STATUS);
	(void)feature;
	return ret;
}
#endif

#ifdef LPM_SUSPEND
STATIC int32_t lpm_dsmi_set_suspend(
	void *feature, char *in, uint32_t in_len, char *out, uint32_t out_len)
{
	int32_t ret = lpm_suspend_set_suspend(in, in_len, out, out_len);
	lpm_dsmi_update_statistics(ret, LPM_DSMI_SET_SUSPEND);
	(void)feature;
	return ret;
}
#endif

#ifdef LPM_FREQUENCY
STATIC int32_t lpm_dsmi_get_frequency(
	void *feature, char *in, uint32_t in_len, char *out, uint32_t out_len)
{
	int32_t ret = lpm_frequency_query(in, in_len, out, out_len);
	lpm_dsmi_update_statistics(ret, LPM_DSMI_GET_FREQUENCY);
	(void)feature;
	return ret;
}
#endif

#ifdef LPM_TEMPERATURE
STATIC int32_t lpm_dsmi_get_temperature(
	void *feature, char *in, u32 in_len, char *out, u32 out_len)
{
	int32_t ret = lpm_temperature_query_soc(in, in_len, out, out_len);
	lpm_dsmi_update_statistics(ret, LPM_DSMI_GET_TEMPERATURE);
	(void)feature;
	return ret;
}
#endif

#ifdef LPM_VRD_UPGRADE
STATIC int32_t lpm_dsmi_get_vrd_info(
	void *feature, char *in, uint32_t in_len, char *out, uint32_t out_len)
{
	int32_t ret = lpm_vrd_upgrade_get_status_info(in, in_len, out, out_len);
	lpm_dsmi_update_statistics(ret, LPM_DSMI_GET_VRD_INFO);
	(void)feature;
	return ret;
}
#endif

// pay attention
// 1. non-permission failure, do not return -1 to dsmi module
// 2. set interface needs to record log
// 3. not support should return -EOPNOTSUPP to dsmi module
BEGIN_DMS_MODULE_DECLARATION(MODULE_LPM_DEVMNG)
BEGIN_FEATURE_COMMAND()
#if defined(LPM_DSMI_MAIN_CMD_LP)
ADD_FEATURE_COMMAND(MODULE_LPM_DEVMNG, DMS_GET_SET_DEVICE_INFO_CMD, ZERO_CMD,
	g_dsmi_filter[(uint32_t)LPM_DSMI_SET_TOPS].filter, "dmp_daemon",
	DMS_SUPPORT_MANAGE_PHY, lpm_dsmi_set_tops)

ADD_FEATURE_COMMAND(MODULE_LPM_DEVMNG, DMS_GET_SET_DEVICE_INFO_CMD, ZERO_CMD,
	g_dsmi_filter[(uint32_t)LPM_DSMI_SET_LPTEST].filter, "dmp_daemon",
	DMS_SUPPORT_MANAGE_PHY, lpm_dsmi_set_lptest_info)

ADD_FEATURE_COMMAND(MODULE_LPM_DEVMNG, DMS_GET_SET_DEVICE_INFO_CMD, ZERO_CMD,
	g_dsmi_filter[(uint32_t)LPM_DSMI_SET_IDLE_SWITCH].filter, "dmp_daemon",
	DMS_SUPPORT_MANAGE_PHY, lpm_dsmi_set_idle_switch)

ADD_FEATURE_COMMAND(MODULE_LPM_DEVMNG, DMS_GET_SET_DEVICE_INFO_CMD, ZERO_CMD,
	g_dsmi_filter[(uint32_t)LPM_DSMI_SET_SOC_STRESS].filter, "dmp_daemon",
	DMS_SUPPORT_MANAGE_PHY, lpm_dsmi_set_soc_stress)

ADD_FEATURE_COMMAND(MODULE_LPM_DEVMNG, DMS_GET_GET_DEVICE_INFO_CMD, ZERO_CMD,
	g_dsmi_filter[(uint32_t)LPM_DSMI_VOLTAGE_CURRENT_AICORE].filter, "dmp_daemon",
	DMS_SUPPORT_ALL, lpm_dsmi_get_volt_current_aicore)

ADD_FEATURE_COMMAND(MODULE_LPM_DEVMNG, DMS_GET_GET_DEVICE_INFO_CMD, ZERO_CMD,
	g_dsmi_filter[(uint32_t)LPM_DSMI_VOLTAGE_CURRENT_HYBRID].filter, "dmp_daemon",
	DMS_SUPPORT_ALL, lpm_dsmi_get_volt_current_hybrid)

ADD_FEATURE_COMMAND(MODULE_LPM_DEVMNG, DMS_GET_GET_DEVICE_INFO_CMD, ZERO_CMD,
	g_dsmi_filter[(uint32_t)LPM_DSMI_VOLTAGE_CURRENT_TAISHAN].filter, "dmp_daemon",
	DMS_SUPPORT_ALL, lpm_dsmi_get_volt_current_taishan)

ADD_FEATURE_COMMAND(MODULE_LPM_DEVMNG, DMS_GET_GET_DEVICE_INFO_CMD, ZERO_CMD,
	g_dsmi_filter[(uint32_t)LPM_DSMI_VOLTAGE_CURRENT_DDR].filter, "dmp_daemon",
	DMS_SUPPORT_ALL, lpm_dsmi_get_volt_current_ddr)

ADD_FEATURE_COMMAND(MODULE_LPM_DEVMNG, DMS_GET_GET_DEVICE_INFO_CMD, ZERO_CMD,
	g_dsmi_filter[(uint32_t)LPM_DSMI_ACG_CNT].filter, "dmp_daemon",
	DMS_SUPPORT_ALL, lpm_dsmi_get_acg_cnt)

ADD_FEATURE_COMMAND(MODULE_LPM_DEVMNG, DMS_GET_GET_DEVICE_INFO_CMD, ZERO_CMD,
	g_dsmi_filter[(uint32_t)LPM_DSMI_IDLE_STATUS].filter, "dmp_daemon",
	DMS_SUPPORT_ALL, lpm_dsmi_get_idle_status)

ADD_FEATURE_COMMAND(MODULE_LPM_DEVMNG, DMS_GET_GET_DEVICE_INFO_CMD, ZERO_CMD,
	g_dsmi_filter[(uint32_t)LPM_DSMI_GET_CUR_TOPS].filter, "dmp_daemon",
	DMS_ACC_ALL | DMS_PHYSICAL_ONLY, lpm_dsmi_get_current_tops)

ADD_FEATURE_COMMAND(MODULE_LPM_DEVMNG, DMS_GET_GET_DEVICE_INFO_CMD, ZERO_CMD,
	g_dsmi_filter[(uint32_t)LPM_DSMI_GET_ALL_TOPS].filter, "dmp_daemon",
	DMS_ACC_ALL | DMS_PHYSICAL_ONLY, lpm_dsmi_get_all_tops)

ADD_FEATURE_COMMAND(MODULE_LPM_DEVMNG, DMS_GET_GET_DEVICE_INFO_CMD, ZERO_CMD,
	g_dsmi_filter[(uint32_t)LPM_DSMI_AICORE_FREQREDUC_CAUSE].filter, "dmp_daemon",
	DMS_SUPPORT_ALL, lpm_dsmi_get_aicore_freq_adj_cause)

ADD_FEATURE_COMMAND(MODULE_LPM_DEVMNG, DMS_GET_GET_DEVICE_INFO_CMD, ZERO_CMD,
	g_dsmi_filter[(uint32_t)LPM_DSMI_GET_POWER_INFO].filter, "dmp_daemon",
	DMS_SUPPORT_ALL, lpm_dsmi_get_power_info)

ADD_FEATURE_COMMAND(MODULE_LPM_DEVMNG, DMS_GET_GET_DEVICE_INFO_CMD, ZERO_CMD,
	g_dsmi_filter[(uint32_t)LPM_DSMI_GET_AIC_ACG_CPM].filter, "dmp_daemon",
	DMS_SUPPORT_ALL, lpm_dsmi_get_aic_acg_cpm)

ADD_FEATURE_COMMAND(MODULE_LPM_DEVMNG, DMS_GET_GET_DEVICE_INFO_CMD, ZERO_CMD,
	g_dsmi_filter[(uint32_t)LPM_DSMI_GET_BUS_ACG_CPM].filter, "dmp_daemon",
	DMS_SUPPORT_ALL, lpm_dsmi_get_bus_acg_cpm)

ADD_FEATURE_COMMAND(MODULE_LPM_DEVMNG, DMS_GET_GET_DEVICE_INFO_CMD, ZERO_CMD,
	g_dsmi_filter[(uint32_t)LPM_DSMI_GET_LPTEST].filter, "dmp_daemon",
	DMS_SUPPORT_ALL, lpm_dsmi_get_lptest_info)
#endif

#ifdef LPM_TEMP_THRESHOLD
ADD_FEATURE_COMMAND(MODULE_LPM_DEVMNG, DMS_GET_GET_DEVICE_INFO_CMD, ZERO_CMD,
	g_dsmi_filter[(uint32_t)LPM_DSMI_TEMP_DDR].filter, "dmp_daemon",
	DMS_SUPPORT_ALL, lpm_dsmi_get_ddr_gear)

ADD_FEATURE_COMMAND(MODULE_LPM_DEVMNG, DMS_GET_GET_DEVICE_INFO_CMD, ZERO_CMD,
	g_dsmi_filter[(uint32_t)LPM_DSMI_TEMP_DDR_THOLD].filter, "dmp_daemon",
	DMS_SUPPORT_ALL, lpm_dsmi_get_ddr_high_temp_threshold)

ADD_FEATURE_COMMAND(MODULE_LPM_DEVMNG, DMS_GET_GET_DEVICE_INFO_CMD, ZERO_CMD,
	g_dsmi_filter[(uint32_t)LPM_DSMI_TEMP_SOC_THOLD].filter, "dmp_daemon",
	DMS_SUPPORT_ALL, lpm_dsmi_get_soc_high_temp_threshold)

ADD_FEATURE_COMMAND(MODULE_LPM_DEVMNG, DMS_GET_GET_DEVICE_INFO_CMD, ZERO_CMD,
	g_dsmi_filter[(uint32_t)LPM_DSMI_TEMP_SOC_MIN_THOLD].filter, "dmp_daemon",
	DMS_SUPPORT_ALL, lpm_dsmi_get_soc_min_temp_threshold)
#endif

#ifdef LPM_HEALTH_STATUS
ADD_FEATURE_COMMAND(MODULE_LPM_DEVMNG, (uint32_t)DMS_MAIN_CMD_LPM, DMS_SUBCMD_GET_LP_STATUS,
	NULL, "dmp_daemon", DMS_SUPPORT_ALL, lpm_dsmi_get_lp_status)
#endif

#ifdef LPM_SUSPEND
ADD_FEATURE_COMMAND(MODULE_LPM_DEVMNG, DMS_GET_SET_POWER_STATE_V2_CMD, DMS_SUBCMD_LP_SUSPEND,
	NULL, "dmp_daemon", DMS_SUPPORT_MANAGE_PHY, lpm_dsmi_set_suspend)
#endif

#ifdef LPM_FREQUENCY
ADD_FEATURE_COMMAND(MODULE_LPM_DEVMNG, (uint32_t)DMS_MAIN_CMD_LPM, DMS_SUBCMD_GET_FREQUENCY,
	NULL, NULL, DMS_SUPPORT_ALL, lpm_dsmi_get_frequency)
#endif

#ifdef LPM_TEMPERATURE
ADD_FEATURE_COMMAND(MODULE_LPM_DEVMNG, (uint32_t)DMS_MAIN_CMD_LPM, DMS_SUBCMD_GET_TEMPERATURE,
	NULL, NULL, DMS_SUPPORT_ALL, lpm_dsmi_get_temperature)
#endif

#ifdef LPM_VRD_UPGRADE
ADD_FEATURE_COMMAND(MODULE_LPM_DEVMNG, (uint32_t)DMS_MAIN_CMD_PRODUCT, DMS_SUBCMD_GET_VRD_INFO,
	NULL, NULL, DMS_SUPPORT_ROOT_PHY, lpm_dsmi_get_vrd_info)
#endif

END_FEATURE_COMMAND()
END_MODULE_DECLARATION()

STATIC const struct lpm_dsmi_sub_cmd_cfg *lpm_dsmi_get_sub_cmd_lp_list(uint32_t *cmd_cnt)
{
	static struct lpm_dsmi_sub_cmd_cfg sub_cmd_list[] = {
		// sub_cmd, api_type
		{DMS_SUBCMD_LP_SET_TOPS,                LPM_DSMI_SET_TOPS},
		{DMS_SUBCMD_LP_SET_LPTEST,              LPM_DSMI_SET_LPTEST},
		{DMS_SUBCMD_LP_SET_IDLE_SWITCH,         LPM_DSMI_SET_IDLE_SWITCH},
		{DMS_SUBCMD_LP_VOLTAGE_CURRENT_AICORE,  LPM_DSMI_VOLTAGE_CURRENT_AICORE},
		{DMS_SUBCMD_LP_VOLTAGE_CURRENT_HYBRID,  LPM_DSMI_VOLTAGE_CURRENT_HYBRID},
		{DMS_SUBCMD_LP_VOLTAGE_CURRENT_TAISHAN, LPM_DSMI_VOLTAGE_CURRENT_TAISHAN},
		{DMS_SUBCMD_LP_VOLTAGE_CURRENT_DDR,     LPM_DSMI_VOLTAGE_CURRENT_DDR},
		{DMS_SUBCMD_LP_ACG,                     LPM_DSMI_ACG_CNT},
		{DMS_SUBCMD_LP_STATUS,                  LPM_DSMI_IDLE_STATUS},
		{DMS_SUBCMD_LP_GET_CUR_TOPS,            LPM_DSMI_GET_CUR_TOPS},
		{DMS_SUBCMD_LP_GET_ALL_TOPS,            LPM_DSMI_GET_ALL_TOPS},
		{DMS_SUBCMD_LP_AICORE_FREQREDUC_CAUSE,  LPM_DSMI_AICORE_FREQREDUC_CAUSE},
		{DMS_SUBCMD_LP_GET_POWER_INFO,          LPM_DSMI_GET_POWER_INFO},
		{DMS_SUBCMD_LP_SET_STRESS_TEST,         LPM_DSMI_SET_SOC_STRESS},
		{DMS_SUBCMD_LP_GET_AIC_CPM,             LPM_DSMI_GET_AIC_ACG_CPM},
		{DMS_SUBCMD_LP_GET_BUS_CPM,             LPM_DSMI_GET_BUS_ACG_CPM},
		{DMS_SUBCMD_LP_GET_LPTEST,              LPM_DSMI_GET_LPTEST},
	};
	*cmd_cnt = (uint32_t)ARRAY_SIZE(sub_cmd_list);
	return &sub_cmd_list[0];
}

STATIC const struct lpm_dsmi_sub_cmd_cfg *lpm_dsmi_get_sub_cmd_temp_list(uint32_t *cmd_cnt)
{
	static struct lpm_dsmi_sub_cmd_cfg sub_cmd_list[] = {
		// sub_cmd, api_type
		{DMS_SUBCMD_TEMP_DDR,           LPM_DSMI_TEMP_DDR},
		{DMS_SUBCMD_TEMP_DDR_THOLD,     LPM_DSMI_TEMP_DDR_THOLD},
		{DMS_SUBCMD_TEMP_SOC_THOLD,     LPM_DSMI_TEMP_SOC_THOLD},
		{DMS_SUBCMD_TEMP_SOC_MIN_THOLD, LPM_DSMI_TEMP_SOC_MIN_THOLD},
	};
	*cmd_cnt = (uint32_t)ARRAY_SIZE(sub_cmd_list);
	return &sub_cmd_list[0];
}

STATIC int32_t lpm_dsmi_init_filter_string(void)
{
	int32_t ret;
	uint32_t i;
	uint32_t j;
	uint32_t filter_idx;
	struct lpm_dsmi_cmd_cfg cmd_cfg[] = {
		{DMS_MAIN_CMD_LP,   lpm_dsmi_get_sub_cmd_lp_list},
		{DMS_MAIN_CMD_TEMP, lpm_dsmi_get_sub_cmd_temp_list},
	};
	const struct lpm_dsmi_sub_cmd_cfg *sub_cmd_list = NULL;
	uint32_t sub_cmd_cnt = 0;

	(void)memset_s(g_dsmi_filter, sizeof(g_dsmi_filter), 0, sizeof(g_dsmi_filter));

	for (i = 0; i < (uint32_t)ARRAY_SIZE(cmd_cfg); i++) {
		sub_cmd_list = cmd_cfg[i].fn_get_sub_cmd(&sub_cmd_cnt);
		for (j = 0; j < sub_cmd_cnt; j++) {
			filter_idx = (uint32_t)sub_cmd_list[j].api_type;

			ret = sprintf_s(g_dsmi_filter[filter_idx].filter, sizeof(g_dsmi_filter[filter_idx].filter),
				"main_cmd=0x%x,sub_cmd=0x%x", cmd_cfg[i].main_cmd, sub_cmd_list[j].sub_cmd);
			if (ret <= 0) {
				lpm_log_err("sprintf_s failed, ret=%d, main_cmd=0x%x, sub_cmd=0x%x\n",
					ret, cmd_cfg[i].main_cmd, sub_cmd_list[j].sub_cmd);
				return -1;
			}
		}
	}

	return 0;
}

int32_t lpm_dsmi_probe(uint64_t *param, uint32_t param_num)
{
	int32_t ret;

	ret = lpm_dsmi_init_filter_string();
	if (ret != 0) {
		lpm_log_err("lpm dsmi init filter string failed, ret=%d\n", ret);
		return ret;
	}

	CALL_INIT_MODULE(MODULE_LPM_DEVMNG);

	lpm_log_info("lpm devmng dsmi probe success\n");

	(void)param;
	(void)param_num;
	return 0;
}

int32_t lpm_dsmi_remove(uint64_t *param, uint32_t param_num)
{
	CALL_EXIT_MODULE(MODULE_LPM_DEVMNG);

	(void)param;
	(void)param_num;
	lpm_log_info("lpm devmng dsmi remove success\n");
	return 0;
}
