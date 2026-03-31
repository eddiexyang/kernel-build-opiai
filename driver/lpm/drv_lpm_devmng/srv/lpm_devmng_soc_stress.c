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

#include <linux/securec.h>
#include <linux/uaccess.h>
#include <linux/math64.h>
#include <linux/mutex.h>
#include "dsmi_common_interface.h"
#include "lpm_devmng_common.h"
#include "lpm_devmng_ipc.h"
#include "lpm_devmng_sharemem.h"
#include "lpm_devmng_sharemem_v2.h"
#include "lpm_devmng_soc_stress.h"

struct lpm_soc_stress_drv_dev_priv {
	struct soc_cpm_data soc_cpm_data;
	struct mutex dev_mutex;
};

struct lpm_soc_stress_drv_priv {
	struct lpm_soc_stress_drv_dev_priv dev_data[LPM_DEVMNG_DEV_MAX_NUM];
};

static struct lpm_soc_stress_drv_priv *lpm_soc_stress_get_drv_priv_info(void)
{
	static struct lpm_soc_stress_drv_priv lpm_soc_stress_drv_priv_info = {0};
	return &lpm_soc_stress_drv_priv_info;
}

static void lpm_soc_stress_drv_init_data(void)
{
	struct lpm_soc_stress_drv_priv *soc_stress_priv = lpm_soc_stress_get_drv_priv_info();
	uint32_t dev_id;

	for (dev_id = 0; dev_id < LPM_DEVMNG_DEV_MAX_NUM; dev_id++) {
		mutex_init(&soc_stress_priv->dev_data[dev_id].dev_mutex);
	}
}

static void lpm_soc_stress_drv_uninit(void)
{
	uint32_t dev_id;
	struct lpm_soc_stress_drv_priv *soc_stress_priv = lpm_soc_stress_get_drv_priv_info();

	for (dev_id = 0; dev_id < LPM_DEVMNG_DEV_MAX_NUM; dev_id++) {
		mutex_destroy(&soc_stress_priv->dev_data[dev_id].dev_mutex);
	}
}

static void lpm_soc_stress_dev_lock(uint32_t dev_id)
{
	struct lpm_soc_stress_drv_priv *soc_stress_priv = lpm_soc_stress_get_drv_priv_info();
	mutex_lock(&soc_stress_priv->dev_data[dev_id].dev_mutex);
}

static void lpm_soc_stress_dev_unlock(uint32_t dev_id)
{
	struct lpm_soc_stress_drv_priv *soc_stress_priv = lpm_soc_stress_get_drv_priv_info();
	mutex_unlock(&soc_stress_priv->dev_data[dev_id].dev_mutex);
}

STATIC int32_t lpm_stress_cfg_para_check(uint32_t dev_id, const struct soc_stress_cfg *cfg)
{
	if (!lpm_common_check_dev_id(dev_id)) {
		lpm_log_err("check set stress failed, dev_id=%u is out of range\n", dev_id);
		return -EINVAL;
	}

	// check type and set_restore here, value checked by lpmcu
	if (cfg->type >= STRESS_ADJ_MAX) {
		lpm_log_err("check set stress failed, type=%u is not support\n", cfg->type);
		return -EOPNOTSUPP;
	}

	if (cfg->set_restore >= STRESS_SET_RESTORE_MAX) {
		lpm_log_err("check set stress failed, set_restore=%u is not support\n", cfg->set_restore);
		return -EOPNOTSUPP;
	}

	lpm_log_info("dev[%u] set soc stress, type=%u set_restore=%u value=%u\n",
		dev_id, cfg->type, cfg->set_restore, cfg->value);
	return 0;
}

STATIC int32_t ipc_result_err_trans(uint8_t result)
{
	int32_t ret;

	switch (result) {
	case IPC_STRESS_NOSUPPORT:
		ret = -EOPNOTSUPP;
		break;
	case IPC_STRESS_PARAM_ERR:
		ret = -EINVAL;
		break;
	case IPC_STRESS_CFG_FAIL:
		ret = -1;
		break;
	case IPC_STRESS_RES_OK: // fall through
	default:
		ret = 0;
		break;
	}

	return ret;
}

STATIC int32_t lpm_stress_ipc_send_cfg(uint32_t dev_id, const struct soc_stress_cfg *cfg)
{
	int32_t ret;
	struct ipc_stress_cfg_ack ipc_ack;
	struct lpm_devmng_ipc_msg ipc_msg;

	ipc_msg.send_type = LPM_IPC_SET_SOC_STRESS;
	ipc_msg.in        = (void *)cfg;
	ipc_msg.in_len    = (uint32_t)sizeof(struct soc_stress_cfg);
	ipc_msg.out       = (void *)&ipc_ack;
	ipc_msg.out_pos   = 0;
	ipc_msg.out_len   = (uint32_t)sizeof(ipc_ack);

	ret = lpm_ipc_send_sync_msg(dev_id, &ipc_msg);
	if (ret != 0) {
		lpm_log_err("lpm stress send ipc failed, ret=%d\n", ret);
		return -EREMOTEIO;
	}

	if (ipc_ack.result != 0) {
		lpm_log_err("ipc set stress ack failed: result=%u, dev_id=%u\n", ipc_ack.result, dev_id);
		return ipc_result_err_trans(ipc_ack.result);
	}

	return 0;
}

STATIC int32_t lpm_stress_set_get_input(
	const char *in, uint32_t in_len, struct lpm_soc_stress_dsmi_cfg_in *para)
{
	int32_t ret;

	if ((in == NULL) || (in_len != sizeof(struct lpm_soc_stress_dsmi_cfg_in))) {
		lpm_log_err("get stress set input failed, input char is NULL or in_len is wrong, in_len=%u\n", in_len);
		return -EINVAL;
	}

	ret = copy_from_user((void *)para, (const void *)in, in_len);
	if (ret != 0) {
		lpm_log_err("get stress set input copy_from_user failed, ret=%d, in_len=%u\n", ret, in_len);
		return -EIO;
	}

	return 0;
}

int32_t lpm_set_soc_stress(uint32_t dev_id, const char *in, uint32_t in_len)
{
	int32_t ret;
	struct lpm_soc_stress_dsmi_cfg_in para = {0};

	ret = lpm_stress_set_get_input(in, in_len, &para);
	if (ret != 0) {
		lpm_log_err("lpm stress set get input failed, ret=%d\n", ret);
		return -EINVAL;
	}

	ret = lpm_stress_cfg_para_check(dev_id, &para.cfg);
	if (ret != 0) {
		return ret;
	}

	lpm_soc_stress_dev_lock(dev_id);
	ret = lpm_stress_ipc_send_cfg(dev_id, &para.cfg);
	lpm_soc_stress_dev_unlock(dev_id);
	if (ret != 0) {
		lpm_log_err("lpm stress set send ipc failed, ret=%d\n", ret);
		return ret;
	}

	lpm_log_info("lpm stress set success, dev_id=%u\n", dev_id);
	return 0;
}

static uint8_t cal_fall(uint8_t init, uint8_t min)
{
	return (init > min) ? (init - min) : 0;
}

static uint8_t get_cpm_fall(const struct cpm_data *cpm_data)
{
	uint8_t cpm_fall_by_falldata = cal_fall(cpm_data->init_fall, cpm_data->min_fall);
	uint8_t cpm_fall_by_risedata = cal_fall(cpm_data->init_rise, cpm_data->min_rise);
	return max(cpm_fall_by_falldata, cpm_fall_by_risedata);
}

static uint8_t get_volt_fall(const struct cpm_data *cpm_data)
{
	return (uint8_t)max(cpm_data->volt_fall_by_risedata, cpm_data->volt_fall_by_falldata);
}

STATIC int32_t lpm_notify_cpm_update_and_get_profile_voltage(
	uint32_t dev_id, uint32_t cpm_type, uint8_t *profile, uint16_t *voltage)
{
	int32_t ret;
	struct ipc_get_cpm_ack ipc_ack;
	struct lpm_devmng_ipc_msg ipc_msg;

	ipc_msg.send_type = LPM_IPC_GET_ACG_CPM;
	ipc_msg.in        = NULL;
	ipc_msg.in_len    = 0;
	ipc_msg.out       = (void *)&ipc_ack;
	ipc_msg.out_pos   = 0;
	ipc_msg.out_len   = (uint32_t)sizeof(ipc_ack);

	ret = lpm_ipc_send_sync_msg(dev_id, &ipc_msg);
	if (ret != 0) {
		lpm_log_err("ipc send get cpm ipc failed, ret=%d\n", ret);
		return -EREMOTEIO;
	}

	if (ipc_ack.result != 0) {
		lpm_log_err("ipc get cpm ack failed: result=%u, dev_id=%u\n", ipc_ack.result, dev_id);
		return ipc_result_err_trans(ipc_ack.result);
	}

	if ((ipc_ack.aic_profile >= AIC_PROFILE_NUM_MAX) || (ipc_ack.cpu_profile >= CPU_PROFILE_NUM_MAX)) {
		lpm_log_err("aic profile %u, cpu profile %u invalid\n", ipc_ack.aic_profile, ipc_ack.cpu_profile);
		return -1;
	}

	if (cpm_type == 0) {
		*profile = ipc_ack.aic_profile;
		*voltage = ipc_ack.aic_volt;
	} else {
		*profile = ipc_ack.cpu_profile;
		*voltage = ipc_ack.bus_volt;
	}

	return 0;
}

STATIC int32_t lpm_update_and_read_cpm_data(uint32_t dev_id, uint32_t cpm_type, char *in, uint32_t *out_len)
{
	int32_t ret;
	uint8_t profile;
	uint32_t num, i;
	unsigned long cpy_len;
	struct cpm_data *data = NULL;
	struct dsmi_cpm_out_info cpm_out = {0};
	struct lpm_soc_stress_drv_priv *soc_stress_priv = lpm_soc_stress_get_drv_priv_info();
	struct soc_cpm_data *soc_cpm_data = &soc_stress_priv->dev_data[dev_id].soc_cpm_data;

	ret = lpm_notify_cpm_update_and_get_profile_voltage(dev_id, cpm_type, &profile, &cpm_out.voltage);
	if (ret != 0) {
		return ret;
	}

	ret = lpm_sharemem_get_value(dev_id, LPM_SHAREMEM_CPM_DATA_OFFSET,
		(uint8_t *)soc_cpm_data, sizeof(struct soc_cpm_data));
	if (ret != 0) {
		lpm_log_err("get cpm data from sharemem failed ret=%d\n", ret);
		return -1;
	}

	if (soc_cpm_data->magic != ACG_CPM_MAGIC) {
		lpm_log_err("sharemem cpm data magic 0x%x invalid\n", soc_cpm_data->magic);
		return -1;
	}

	num = (cpm_type == 0) ? AIC_NUM : CPU_NUM;
	for (i = 0; i < num; i++) {
		data = (cpm_type == 0) ? &soc_cpm_data->aic[i][profile] : &soc_cpm_data->cpu[i][profile];
		cpm_out.cpm_data[i] = get_cpm_fall(data);
		cpm_out.max_volt_fall = max(cpm_out.max_volt_fall, get_volt_fall(data));
	}
	cpm_out.core_num = num;

	cpy_len = copy_to_user(in, (void *)&cpm_out, sizeof(cpm_out));
	if (cpy_len != 0) {
		lpm_log_err("get cpm copy to user failed. (dev_id=%u, cpm_type=%u, cpy_len=%lu)\n", dev_id, cpm_type, cpy_len);
		return -EIO;
	}

	*out_len = (uint32_t)sizeof(cpm_out);
	return 0;
}

int32_t lpm_get_aicore_cpm(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len)
{
	int32_t ret = lpm_check_dsmi_get_param(dev_id, in, in_len, (uint32_t)sizeof(struct dsmi_cpm_out_info), out_len);
	if (ret != 0) {
		lpm_log_err("get aicore cpm, check param failed, ret=%d\n", ret);
		return -EINVAL;
	}

	lpm_soc_stress_dev_lock(dev_id);
	ret = lpm_update_and_read_cpm_data(dev_id, 0x0, in, out_len);
	lpm_soc_stress_dev_unlock(dev_id);
	return ret;
}

int32_t lpm_get_bus_cpm(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len)
{
	int32_t ret = lpm_check_dsmi_get_param(dev_id, in, in_len, (uint32_t)sizeof(struct dsmi_cpm_out_info), out_len);
	if (ret != 0) {
		lpm_log_err("get bus cpm, check param failed, ret=%d\n", ret);
		return -EINVAL;
	}

	lpm_soc_stress_dev_lock(dev_id);
	ret = lpm_update_and_read_cpm_data(dev_id, 0x1, in, out_len);
	lpm_soc_stress_dev_unlock(dev_id);
	return ret;
}

int32_t lpm_soc_stress_probe(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	lpm_soc_stress_drv_init_data();
	return 0;
}

int32_t lpm_soc_stress_remove(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	lpm_soc_stress_drv_uninit();
	return 0;
}
