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
#include <asm/uaccess.h>
#include <linux/slab.h>
#include "devdrv_manager_common.h"
#include "devdrv_pm.h"
#include "dms_define.h"
#include "memory_log.h"
#include "memory_hbm_cfg.h"
#include "memory_hbm.h"
#include "devdrv_manager_comm.h"
#include "memory_scan.h"
#include "ascend_kernel_hal.h"
#include "dms_cmd_def.h"
#include "drvmem_base_info.h"
#include "memory_adapter.h"
#ifdef CFG_LPM_INFO_FROM_IPC
#include "devdrv_ipc.h"
#include "icm_interface.h"
#endif

#ifndef CFG_MEMORY_HBM_STUB
STATIC hbm_hw_info_t g_hbm_hw_info[MAX_CHIP_NUM] = {{0}};
STATIC hbm_prof_cfg_t g_hbm_prof_cfg[MAX_CHIP_NUM] = {{0}};
STATIC hbm_flux_info_t g_hbm_flux_info_bk[MAX_CHIP_NUM] = {0};
STATIC struct timespec64 g_timestamp_start[MAX_CHIP_NUM] = {0};
STATIC struct mutex g_hbmc_reg_lock[MAX_CHIP_NUM];
STATIC struct mutex g_hbmc_init_lock[MAX_CHIP_NUM];

STATIC void init_hbm_statistics(unsigned int dev_id);

STATIC int map_hbm_reg(unsigned int dev_id)
{
	int i, j;
	unsigned long long base = 0;
	unsigned long reg_base[HBM_TOTAL_NUM] = {
		HBMC0_REG_BASE, HBMC1_REG_BASE,
		HBMC2_REG_BASE, HBMC3_REG_BASE
	};
	int ret;

	ret = memory_get_adapter()->get_base_addr(dev_id, &base);
	if (ret != 0) {
		memory_drv_err("get hbm base addr failed. (dev_id=%u)\n", dev_id);
		return ret;
	}

	mutex_lock(&g_hbmc_reg_lock[dev_id]);
	g_hbm_hw_info[dev_id].hbm_num = HBM_TOTAL_NUM;
	g_hbm_hw_info[dev_id].chan_num = HBM_MAX_CHAN_NUM;
	g_hbm_prof_cfg[dev_id].master_id = HBMC_MASTER_ID_ALL;

	for (i = 0;  i < HBM_TOTAL_NUM; i++) {
		for (j = 0; j < HBM_MAX_CHAN_NUM; j++) {
			g_hbm_hw_info[dev_id].reg_base[i][j] =
				(unsigned long)(reg_base[i] + ((unsigned long)HBMC_CHAN_REG_BASE_OFFSET * j));
			g_hbm_hw_info[dev_id].reg_base[i][j] += base;
			g_hbm_hw_info[dev_id].reg_vir_addr[i][j] = ioremap(g_hbm_hw_info[dev_id].reg_base[i][j], HBMC_MAP_SIZE);
			if (g_hbm_hw_info[dev_id].reg_vir_addr[i][j] == NULL) {
				memory_drv_err("Failed to ioremap hbm reg. (dev_id=%u)\n", dev_id);
				mutex_unlock(&g_hbmc_reg_lock[dev_id]);
				return -ENOMEM;
			}
		}
	}
	mutex_unlock(&g_hbmc_reg_lock[dev_id]);

	return 0;
}

STATIC void unmap_hbm_reg(unsigned int dev_id)
{
	int i, j;

	mutex_lock(&g_hbmc_reg_lock[dev_id]);
	for (i = 0;  i < HBM_TOTAL_NUM; i++) {
		for (j = 0; j < HBM_MAX_CHAN_NUM; j++) {
			if (g_hbm_hw_info[dev_id].reg_vir_addr[i][j] != NULL) {
				iounmap(g_hbm_hw_info[dev_id].reg_vir_addr[i][j]);
				g_hbm_hw_info[dev_id].reg_vir_addr[i][j] = NULL;
			}
		}
	}
	mutex_unlock(&g_hbmc_reg_lock[dev_id]);
}

int memory_map_hbm_init(void)
{
	int ret;
	unsigned int i;
	unsigned int dev_num = 0;

	/* Avoid releasing uninitialized locks when map reg fails */
	for (i = 0; i < MAX_CHIP_NUM; i++) {
		mutex_init(&g_hbmc_reg_lock[i]);
		mutex_init(&g_hbmc_init_lock[i]);
	}

	ret = devdrv_get_devnum(&dev_num);
	if (ret != 0) {
		memory_drv_err("Get device num failed. (ret=%d)\n", ret);
		return ret;
	}

	for (i = 0; i < dev_num; i++) {
		/* initialize reg lock */
		ret = map_hbm_reg(i);
		if (ret != 0) {
			memory_drv_err("Failed to init hbm. (dev_id=%u; ret=%d)\n", i, ret);
			memory_map_hbm_exit();
			return ret;
		}
	}

	memory_drv_debug("Memory map hbm init success.\n");
	return 0;
}

void memory_map_hbm_exit(void)
{
	int i;

	for (i = 0; i < MAX_CHIP_NUM; i++) {
		unmap_hbm_reg(i);
		mutex_destroy(&g_hbmc_init_lock[i]);
		mutex_destroy(&g_hbmc_reg_lock[i]);
	}
	memory_drv_debug("Exit hbm map success.\n");
}

/*
 * hbmc register operation, read or write
 */
STATIC void hbm_reg_op(unsigned char op_type, const void __iomem *vir_addr, unsigned long reg_offset, unsigned int *val)
{
	if (op_type == HBMC_REG_WR) {
		writel(*val, (void __iomem *)(uintptr_t)((long)(uintptr_t)vir_addr + reg_offset));
	} else {
		*val = readl((void __iomem *)(uintptr_t)((long)(uintptr_t)vir_addr + reg_offset));
	}
	return;
}

STATIC unsigned int get_hbm_flux_diff(unsigned int start, unsigned int end)
{
	unsigned int flux_diff;

	flux_diff = (unsigned int)((unsigned long long)end + ((unsigned long long)UINT_TYPE_MAX + 1) - start) % \
		((unsigned long long)UINT_TYPE_MAX + 1);

	return flux_diff;
}

STATIC unsigned long get_hbm_time_interval(struct timespec64 start, struct timespec64 end)
{
	unsigned long time_use;

	time_use = (end.tv_sec - start.tv_sec) * US_PER_SECOND + (end.tv_nsec - start.tv_nsec) / NSEC_PER_USEC;
	return time_use;
}

STATIC void clear_hbm_flux_reg(const void __iomem *reg_base)
{
	unsigned int i;
	unsigned int op_val = 0;
	unsigned long reg_offset[HBMC_REG_CLR_NUM] = { HBMC_HISI_FLUX_RD_OFFSET, HBMC_HISI_FLUX_WR_OFFSET };

	for (i = 0; i < HBMC_REG_CLR_NUM; i++) {
		hbm_reg_op(HBMC_REG_WR, reg_base, reg_offset[i], &op_val);
	}

	return;
}

STATIC void enable_hbm_flux_statistics(unsigned int dev_id, const hbm_hw_info_t *info)
{
	int i, j;
	unsigned int op_val;
	void __iomem *reg_base = NULL;

	mutex_lock(&g_hbmc_reg_lock[dev_id]);
	for (i = 0; i < HBM_TOTAL_NUM; i++) {
		for (j = 0; j < HBM_MAX_CHAN_NUM; j++) {
			/* disable flux statistic */
			op_val = 0;
			reg_base = g_hbm_hw_info[dev_id].reg_vir_addr[i][j];

			memory_drv_debug("hbm_reg_op stat. (dev_id=%u)\n", dev_id);
			hbm_reg_op(HBMC_REG_WR, reg_base, HBMC_CTRL_PERF_OFFSET, &op_val);
			memory_drv_debug("hbm_reg_op end. (dev_id=%u)\n", dev_id);

			clear_hbm_flux_reg(reg_base);

			/* Enable flux statistic */
			op_val = HBMC_PERF_EN_SET;
			hbm_reg_op(HBMC_REG_WR, reg_base, HBMC_CTRL_PERF_OFFSET, &op_val);
		}
	}
	mutex_unlock(&g_hbmc_reg_lock[dev_id]);

	return;
}

STATIC void init_hbm_statistics(unsigned int dev_id)
{
	int i, j;
	unsigned long long base = 0;
	STATIC unsigned int init_flag[MAX_CHIP_NUM] = {0};
	unsigned long reg_base[HBM_TOTAL_NUM] = {
		HBMC0_REG_BASE, HBMC1_REG_BASE,
		HBMC2_REG_BASE, HBMC3_REG_BASE
	};
	int ret;

	ret = memory_get_adapter()->get_base_addr(dev_id, &base);
	if (ret != 0) {
		memory_drv_err("get hbm base addr failed. (dev_id=%u)\n", dev_id);
		return;
	}

	mutex_lock(&g_hbmc_init_lock[dev_id]);
	if (init_flag[dev_id] == 0) {
		g_hbm_hw_info[dev_id].hbm_num = HBM_TOTAL_NUM;
		g_hbm_hw_info[dev_id].chan_num = HBM_MAX_CHAN_NUM;
		g_hbm_prof_cfg[dev_id].master_id = HBMC_MASTER_ID_ALL;
		for (i = 0; i < HBM_TOTAL_NUM; i++) {
			for (j = 0; j < HBM_MAX_CHAN_NUM; j++) {
				(g_hbm_hw_info[dev_id]).reg_base[i][j] =
					(unsigned long)(reg_base[i] + ((unsigned long)HBMC_CHAN_REG_BASE_OFFSET * j));
				(g_hbm_hw_info[dev_id]).reg_base[i][j] += base;
			}
		}

		memory_drv_debug("enable_hbm_flux_statistics stat. (dev_id=%u)\n", dev_id);
		enable_hbm_flux_statistics(dev_id, &g_hbm_hw_info[dev_id]);
		memory_drv_debug("enable_hbm_flux_statistics end. (dev_id=%u)\n", dev_id);

		init_flag[dev_id] = 1;
	}
	mutex_unlock(&g_hbmc_init_lock[dev_id]);

	memory_drv_debug("Init hbm stat success. (dev_id=%u)\n", dev_id);
	return;
}

STATIC int get_hbm_stat_info(unsigned int dev_id, hbm_flux_info_t *flux_info, int timestamp_flag)
{
	unsigned int i, j;
	unsigned long reg_offset[HBMC_INFO_ITEM_NUM] = {HBMC_HISI_FLUX_RD_OFFSET, HBMC_HISI_FLUX_WR_OFFSET,
		HBMC_HISI_FLUXID_WR_OFFSET, HBMC_HISI_FLUXID_RD_OFFSET};
	struct timespec64 tmp_timestamp = {0};
	void __iomem *base_addr = NULL;

	if ((timestamp_flag != HBM_TIMESTAMP_BEFORE_READ) && (timestamp_flag != HBM_TIMESTAMP_AFTER_READ)) {
		memory_drv_err("Timestamp_flag error. (dev_id=%u)\n", dev_id);
		return -EINVAL;
	}

	if (timestamp_flag == HBM_TIMESTAMP_BEFORE_READ) {
		ktime_get_raw_ts64(&tmp_timestamp);
	}

	for (i = 0; i < HBM_TOTAL_NUM; i++) {
		for (j = 0; j < HBM_MAX_CHAN_NUM; j++) {
			base_addr = g_hbm_hw_info[dev_id].reg_vir_addr[i][j];
			hbm_reg_op(HBMC_REG_RD, base_addr, reg_offset[RD_FLUX_INDEX], &(flux_info->flux_rd[i][j]));
			hbm_reg_op(HBMC_REG_RD, base_addr, reg_offset[WR_FLUX_INDEX], &(flux_info->flux_wr[i][j]));
			hbm_reg_op(HBMC_REG_RD, base_addr, reg_offset[ID_RD_FLUX_INDEX], &(flux_info->id_flux_wr[i][j]));
			hbm_reg_op(HBMC_REG_RD, base_addr, reg_offset[ID_WR_FLUX_INDEX], &(flux_info->id_flux_rd[i][j]));
		}
	}

	if (timestamp_flag == HBM_TIMESTAMP_AFTER_READ) {
		ktime_get_raw_ts64(&tmp_timestamp);
	}

	for (i = 0; i < HBM_TOTAL_NUM; i++) {
		flux_info->time_stamp[i] = tmp_timestamp;
	}

	return 0;
}

STATIC hbm_prof_data_t *update_hbm_buff_index(hbm_prof_data_t *buf, int *data_len, int max_len)
{
	hbm_prof_data_t *prof_data_pt = NULL;

	*data_len += sizeof(hbm_prof_data_t);

	if (*data_len < max_len) {
		prof_data_pt = (hbm_prof_data_t *)(uintptr_t)((unsigned long)(uintptr_t)buf + *data_len);
	} else {
		memory_drv_err("Buffer is overflow.\n");
		return NULL;
	}

	return prof_data_pt;
}

STATIC void set_prof_flux_count(unsigned int event_flag, unsigned long long *count, unsigned int flux_rd,
	unsigned int flux_wr)
{
	if (event_flag == HBM_PROF_EVENT_RD) {
		*count += flux_rd;
	} else {
		*count += flux_wr;
	}
}

/*
 * Set hbm profiling data buffer, for the user.
 */
STATIC int set_prof_buff(unsigned int dev_id, hbm_prof_data_t *buf, int len, hbm_flux_info_t *info)
{
	hbm_prof_data_t *prof_data_pt = NULL;
	int data_len = 0;
	int i, j, k;

	prof_data_pt = buf;
	for (i = 0; i < g_hbm_prof_cfg[dev_id].event_num; i++) {
		for (j = 0; j < HBM_TOTAL_NUM; j++) {
			/* set write event data */
			prof_data_pt->timestamp = (u32)get_hbm_time_interval(g_timestamp_start[dev_id], info->time_stamp[j]);
			prof_data_pt->event = g_hbm_prof_cfg[dev_id].event[i];
			prof_data_pt->hbm_id = j;
			prof_data_pt->count = 0;

			for (k = 0; k < HBM_MAX_CHAN_NUM; k++) {
				set_prof_flux_count(g_hbm_prof_cfg[dev_id].event[i], &(prof_data_pt->count), info->flux_rd[j][k],
					info->flux_wr[j][k]);
			}

			prof_data_pt = update_hbm_buff_index(buf, &data_len, len);
			if (prof_data_pt == NULL) {
				memory_drv_err("Buffer is overflow. (dev_id=%u)\n", dev_id);
				return len;
			}
		}
	}

	return data_len;
}

STATIC int malloc_flux_info(unsigned int dev_id, hbm_flux_info_t **cur_flux_pt, hbm_flux_info_t **tx_flux_pt)
{
	*tx_flux_pt = kzalloc(sizeof(hbm_flux_info_t), GFP_KERNEL | __GFP_ACCOUNT);
	if (*tx_flux_pt == NULL) {
		memory_drv_err("Kmalloc tx_flux_pt failed. (dev_id=%u)\n", dev_id);
		return -ENOMEM;
	}

	*cur_flux_pt = kzalloc(sizeof(hbm_flux_info_t), GFP_KERNEL | __GFP_ACCOUNT);
	if (*cur_flux_pt == NULL) {
		kfree(*tx_flux_pt);
		*tx_flux_pt = NULL;
		memory_drv_err("Kmalloc cur_flux_pt failed. (dev_id=%u)\n", dev_id);
		return -ENOMEM;
	}

	return 0;
}

STATIC void calculate_flux(unsigned int dev_id, hbm_flux_info_t *tx_flux_pt, hbm_flux_info_t *cur_flux_pt)
{
	int i, j;

	for (i = 0; i < HBM_TOTAL_NUM; i++) {
		for (j = 0; j < HBM_MAX_CHAN_NUM; j++) {
			tx_flux_pt->flux_rd[i][j] = get_hbm_flux_diff(g_hbm_flux_info_bk[dev_id].flux_rd[i][j],
				cur_flux_pt->flux_rd[i][j]);
			tx_flux_pt->flux_wr[i][j] = get_hbm_flux_diff(g_hbm_flux_info_bk[dev_id].flux_wr[i][j],
				cur_flux_pt->flux_wr[i][j]);
			tx_flux_pt->id_flux_rd[i][j] = get_hbm_flux_diff(g_hbm_flux_info_bk[dev_id].id_flux_rd[i][j],
				cur_flux_pt->id_flux_rd[i][j]);
			tx_flux_pt->id_flux_wr[i][j] = get_hbm_flux_diff(g_hbm_flux_info_bk[dev_id].id_flux_wr[i][j],
				cur_flux_pt->id_flux_wr[i][j]);
		}
		tx_flux_pt->time_stamp[i] = cur_flux_pt->time_stamp[i];
	}
}

/*
 * HBM profiling sample func, callback for the profiling driver.
 */
int memory_hbm_prof_sample(struct prof_peri_para para)
{
	int ret;
	hbm_flux_info_t *cur_flux_pt = NULL;
	hbm_flux_info_t *tx_flux_pt = NULL;
	hbm_flux_info_t *bk_info_pt = NULL;
	unsigned int dev_id = para.device_id;
	void *buf = para.buff;
	unsigned int len = para.buff_len;
	unsigned int min_buff_len;

	if ((buf == NULL) || (dms_is_devid_valid(dev_id) == false)) {
		memory_drv_err("Invalid parameter. (dev_id=%u; buf=%pK)\n", dev_id, buf);
		return -EINVAL;
	}

	min_buff_len = g_hbm_prof_cfg[dev_id].event_num * HBM_TOTAL_NUM * COMMON_MASTER;
	if (len < min_buff_len) {
		memory_drv_err("Invalid parameter. (dev_id=%u; len=%u, min_len=%u)\n", dev_id, len, min_buff_len);
		return -EINVAL;
	}

	ret = malloc_flux_info(dev_id, &cur_flux_pt, &tx_flux_pt);
	if (ret < 0) {
		memory_drv_err("Failed to malloc flux info. (dev_id=%u; ret=%d)\n", dev_id, ret);
		return ret;
	}

	/* 1. get hbm flux statistic info */
	ret = get_hbm_stat_info(dev_id, cur_flux_pt, HBM_TIMESTAMP_BEFORE_READ);
	if (ret < 0) {
		memory_drv_err("Failed to get hbm stat info. (dev_id=%u; ret=%d)\n", dev_id, ret);
		goto OUT;
	}

	/* 2. calculate flux difference and record it */
	calculate_flux(dev_id, tx_flux_pt, cur_flux_pt);

	/* 3. backup current flux info */
	bk_info_pt = &(g_hbm_flux_info_bk[dev_id]);
	ret = memcpy_s(bk_info_pt, sizeof(hbm_flux_info_t), cur_flux_pt, sizeof(hbm_flux_info_t));
	if (ret != 0) {
		memory_drv_err("Failed to backup current flux info. (dev_id=%u; ret=%d)\n", dev_id, ret);
		ret = -ENOMEM;
		goto OUT;
	}

	/* 4. set data buff and return data_len to profiling module */
	ret = set_prof_buff(dev_id, (hbm_prof_data_t *)buf, len, tx_flux_pt);
	memory_drv_debug("Sample hbm prof success. (dev_id=%u; buf_len=%d)\n", dev_id, ret);

OUT:
	kfree(tx_flux_pt);
	tx_flux_pt = NULL;
	kfree(cur_flux_pt);
	cur_flux_pt = NULL;
	return ret;
}
EXPORT_SYMBOL(memory_hbm_prof_sample);

STATIC int check_cfg_data(unsigned int dev_id, hbm_prof_cfg_t *cfg_data, unsigned int data_size)
{
	if ((cfg_data->event_num == 0) || (cfg_data->event_num > MAX_EVENT_NUM)) {
		memory_drv_err("Cfg event_num error. (dev_id=%u)\n", dev_id);
		return -EINVAL;
	}

	if (((cfg_data->event_num == 1) && ((data_size != (sizeof(hbm_prof_cfg_t) - sizeof(unsigned int))) ||
		(cfg_data->event[0] >= MAX_EVENT_NUM))) || ((cfg_data->event_num == MAX_EVENT_NUM) &&
		((data_size != sizeof(hbm_prof_cfg_t)) || (cfg_data->event[0] >= MAX_EVENT_NUM) ||
		(cfg_data->event[1] >= MAX_EVENT_NUM)))) {
		memory_drv_err("Event value is error. (event_num=%u; data_size=%u)\n", cfg_data->event_num, data_size);
		return -EINVAL;
	}

	return 0;
}
/*
 * Get HBM profiling cfg data, callback for the profiling driver.
 */
int memory_hbm_prof_init_user_cfg(struct prof_peri_para para)
{
	unsigned int dev_id = para.device_id;
	void *data = para.user_data;
	unsigned int data_size = para.user_data_len;
	hbm_prof_cfg_t *cfg_data = (hbm_prof_cfg_t *)data;
	int ret;

	if ((data == NULL) || (dms_is_devid_valid(dev_id) == false) ||
		((data_size != sizeof(hbm_prof_cfg_t)) && (data_size != (sizeof(hbm_prof_cfg_t) - sizeof(unsigned int))))) {
		memory_drv_err("Invalid para data. (dev_id=%u; data=%pK; data_size=%u)\n", dev_id, data, data_size);
		return -EINVAL;
	}

	ret = check_cfg_data(dev_id, cfg_data, data_size);
	if (ret) {
		memory_drv_err("Invalid cfg data. (dev_id=%u)\n", dev_id);
		return ret;
	}

	ret = memcpy_s(&g_hbm_prof_cfg[dev_id], sizeof(hbm_prof_cfg_t), data, data_size);
	if (ret) {
		memory_drv_err("Memcpy_s error. (dev_id=%u; ret=%d)\n", dev_id, ret);
		return ret;
	}

	/* HBMC don't support master id flux statistic */
	g_hbm_prof_cfg[dev_id].master_id = HBMC_MASTER_ID_ALL;

	memory_drv_info("Device_id:%u.\n", dev_id);
	memory_drv_info("Hbm_prof_cfg_period:%u.\n", g_hbm_prof_cfg[dev_id].period);
	memory_drv_info("Hbm_prof_cfg_master_id:%u.\n", g_hbm_prof_cfg[dev_id].master_id);
	memory_drv_info("Hbm_prof_cfg_event_num:%u.\n", g_hbm_prof_cfg[dev_id].event_num);
	memory_drv_info("Hbm_prof_cfg_event[0]:%u.\n", g_hbm_prof_cfg[dev_id].event[0]);
	memory_drv_info("Hbm_prof_cfg_event[1]:%u.\n", g_hbm_prof_cfg[dev_id].event[1]);

	init_hbm_statistics(dev_id);

	/* get statistic info and fill backup struct after statistic enable */
	ret = get_hbm_stat_info(dev_id, &g_hbm_flux_info_bk[dev_id], HBM_TIMESTAMP_BEFORE_READ);
	if (ret < 0) {
		memory_drv_err("Failed to set user cfg. (dev_id=%u; ret=%d)\n", dev_id, ret);
		return ret;
	}

	g_timestamp_start[dev_id] = g_hbm_flux_info_bk[dev_id].time_stamp[0];

	memory_drv_debug("Init hbm prof user config success. (dev_id=%u)\n", dev_id);
	return 0;
}
EXPORT_SYMBOL(memory_hbm_prof_init_user_cfg);

STATIC int calculate_perf_para(hbm_flux_info_t *info_t1, hbm_flux_info_t *info_t2, unsigned int *rate)
{
	unsigned long total_flux = 0;
	int i, j;
	unsigned long total_time_stamp = 0;
	unsigned long hbm_bandwidth;
	unsigned long actual_hbm_bandwidth = 0;

	for (i = 0; i < HBM_TOTAL_NUM; i++) {
		for (j = 0; j < HBM_MAX_CHAN_NUM; j++) {
			// calculate total flux, including rollover case
			total_flux += get_hbm_flux_diff(info_t1->flux_rd[i][j], info_t2->flux_rd[i][j]);
			total_flux += get_hbm_flux_diff(info_t1->flux_wr[i][j], info_t2->flux_wr[i][j]);
		}

		total_time_stamp += get_hbm_time_interval(info_t1->time_stamp[i], info_t2->time_stamp[i]);
	}

	total_time_stamp = total_time_stamp / (HBM_TOTAL_NUM);

	hbm_bandwidth = HBMC_BANDWIDTH_PER_CHAN * HBM_TOTAL_NUM * HBM_MAX_CHAN_NUM;
	memory_drv_debug("Hbm_bandwidth. (per=%d; num=%d; chan=%d; hbm_bandwidth=%lu)\n",
		HBMC_BANDWIDTH_PER_CHAN, HBM_TOTAL_NUM, HBM_MAX_CHAN_NUM, hbm_bandwidth);

	/* actual bandwidth formulate: (total flux/32768)*1000000/time_stamp (MB/s) */
	if (total_time_stamp > 0) {
		actual_hbm_bandwidth = total_flux * HBMC_TIME_STAMP_PER_SECOND / (total_time_stamp * HBMC_FLUX_MB_DIV);
	} else {
		memory_drv_err("Timestamp error.\n");
		return -ETIME;
	}

	/* hbm bandwidth rate = actual bandwidth/theory bandwidth */
	*rate = (actual_hbm_bandwidth * HBMC_STAT_PERTAGE) / hbm_bandwidth;
	if (*rate > HBMC_STAT_PERTAGE) {
		memory_drv_info("rate val is too large. (rate=%u)\n", *rate);
		*rate = HBMC_STAT_PERTAGE;
	}

	memory_drv_debug("Calc hbm bw util rate. (flux=%lu; time=%lu; hbm_bandwidth=%lu; actual_bw=%lu, rate=%u)\n",
		total_flux, total_time_stamp, hbm_bandwidth, actual_hbm_bandwidth, *rate);

	return 0;
}

int get_bandwidth_util_rate(unsigned int dev_id, unsigned int *rate)
{
	int ret;
	hbm_flux_info_t *flux_t1_pt = NULL;
	hbm_flux_info_t *flux_t2_pt = NULL;

	init_hbm_statistics(dev_id);

	flux_t1_pt = kzalloc(sizeof(hbm_flux_info_t), GFP_KERNEL | __GFP_ACCOUNT);
	if (flux_t1_pt == NULL) {
		memory_drv_err("Kmalloc failed. (dev_id=%u)\n", dev_id);
		return -ENOMEM;
	}

	flux_t2_pt = kzalloc(sizeof(hbm_flux_info_t), GFP_KERNEL | __GFP_ACCOUNT);
	if (flux_t2_pt == NULL) {
		kfree(flux_t1_pt);
		flux_t1_pt = NULL;
		memory_drv_err("Kmalloc failed. (dev_id=%u)\n", dev_id);
		return -ENOMEM;
	}

	mutex_lock(&g_hbmc_reg_lock[dev_id]);
	ret = get_hbm_stat_info(dev_id, flux_t1_pt, HBM_TIMESTAMP_BEFORE_READ);
	if (ret < 0) {
		memory_drv_err("Failed to get hbm stat info for the first time. (dev_id=%u; ret=%d)\n", dev_id, ret);
		goto out;
	}

	msleep(HBMC_FLUX_STAT_TIME);

	ret = get_hbm_stat_info(dev_id, flux_t2_pt, HBM_TIMESTAMP_AFTER_READ);
	if (ret < 0) {
		memory_drv_err("Failed to get hbm stat info for the second time. (dev_id=%u; ret=%d)\n", dev_id, ret);
		goto out;
	}

	ret = calculate_perf_para(flux_t1_pt, flux_t2_pt, rate);
	if (ret < 0) {
		memory_drv_err("Failed to calculate perf pafa. (dev_id=%u; ret=%d)\n", dev_id, ret);
		goto out;
	}

out:
	mutex_unlock(&g_hbmc_reg_lock[dev_id]);
	kfree(flux_t1_pt);
	flux_t1_pt = NULL;
	kfree(flux_t2_pt);
	flux_t2_pt = NULL;
	return ret;
}
#else

int memory_map_hbm_init(void)
{
	memory_drv_debug("Memory map hbm init success.\n");
	return 0;
}

void memory_map_hbm_exit(void)
{
	memory_drv_debug("Memory map hbm exit success.\n");
}

int get_bandwidth_util_rate(unsigned int dev_id, unsigned int *rate)
{
	*rate = HBM_BANDWIDTH_DEFAULT_RATE;
	memory_drv_debug("Get hbm bw util rate. (dev_id=%u; rate=%u)\n", dev_id, *rate);
	return 0;
}
#endif

int memory_get_hbm_bandwidth_utilization_rate(void *feature, char *in, unsigned int in_len, char *out,
	unsigned int out_len)
{
	int ret;
	unsigned int rate = 0;
	unsigned int dev_id;
	struct memory_scan_list *item = NULL;

	if ((in == NULL) || (in_len != sizeof(unsigned int)) || (out == NULL) || (out_len != in_len)) {
		memory_drv_err("Invalid parameter. (in=%pK; in_len=%u; out=%pK; out_len=%u)\n", in, in_len, out, out_len);
		return -EINVAL;
	}

	ret = memcpy_s(&dev_id, sizeof(unsigned int), in, in_len);
	if (ret) {
		memory_drv_err("Memcpy_s failed. (ret=%d)\n", ret);
		return ret;
	}
	if (dms_is_devid_valid(dev_id) == false) {
		memory_drv_err("Invalid parameter. (dev_id=%u)\n", dev_id);
		return -EINVAL;
	}

	item = find_operate_info_str(MEMORY_BW_UTIL, MEMORY_HBM);
	if (item == NULL) {
		memory_drv_err("Get hbm bw util handle struct failed. (dev_id=%u)\n", dev_id);
		return -EINVAL;
	}

	if (item->scan_flag == 0) {
		ret = get_bandwidth_util_rate(dev_id, &rate);
		if (ret < 0) {
			memory_drv_err("Failed get hbm bw util rate. (dev_id=%u; ret=%d)\n", dev_id, ret);
			return ret;
		}
	} else {
		rate = item->value[dev_id].hbm_bw_util;
	}

	ret = memcpy_s(out, out_len, &rate, sizeof(unsigned int));
	if (ret) {
		memory_drv_err("Memcpy_s failed. (ret=%d)\n", ret);
		return ret;
	}
	memory_drv_debug("Get hbm bw util rate success. (dev_id=%u; rate=%u)\n", dev_id, rate);
	return 0;
}

STATIC int32_t hbm_id_check(uint32_t dev_id)
{
	int32_t ret;
	uint32_t dev_num;

	ret = devdrv_get_devnum(&dev_num);
	if (ret != 0) {
		memory_drv_err("Get device num failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
		return ret;
	}

	if (dev_id >= dev_num) {
		memory_drv_err("Device id not valied. (dev_id=%u; dev_num=%u)\n", dev_id, dev_num);
		return -ENODEV;
	}

	return 0;
}

#ifdef CFG_LPM_INFO_FROM_IPC
#define IPC_IMU_TEMP_1			 11
#define IPC_IMU_TEMP			   6
#define DMS_IMU_TX_MBX25		   5
#define IPCDRV_RPROC_MSG_LENGTH	8

STATIC int32_t hbm_base_ipc_sync_send(uint32_t dev_id, struct hbm_ipc *ack_msg)
{
	int32_t ret;
	struct hbm_ipc ipc_send_msg = {
		.cmd_type1 = IPC_IMU_TEMP_1,
		.cmd_type0 = IPC_IMU_TEMP,
		.target_id = OBJ_IMU_DMP,
		.source_id = OBJ_AP_DMP
	};

	ret = icm_msg_send_sync(ICM_FD_BUILD(dev_id, DMS_IMU_TX_MBX25),
		(rproc_msg_t *)&ipc_send_msg, 2U,
		(rproc_msg_t *)ack_msg, IPCDRV_RPROC_MSG_LENGTH);
	if (ret != 0) {
		/* ret is checked by the upper layer. */
		return ret;
	}

	/* Check ack msg format */
	if ((ipc_send_msg.cmd_type0 != ack_msg->cmd_type0) || (ipc_send_msg.cmd_type1 != ack_msg->cmd_type1)) {
		ret = -1;
		memory_drv_err("Icm base msg send failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
	}

	return ret;
}

STATIC int32_t hbm_get_info_from_ipc(uint32_t dev_id, uint8_t *data)
{
	int32_t ret;
	struct hbm_ipc ack_msg = {0};

	/* Send msg */
	ret = hbm_base_ipc_sync_send(dev_id, &ack_msg);
	if (ret != 0) {
		if (ret == ETIMEDOUT) {
			memory_drv_err("Icm base msg send timeout. (dev_id=%u; ret=%d)\n", dev_id, ret);
		} else {
			memory_drv_err("Icm base msg send failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
		}
		return ret;
	}

	/* Set value */
	*data = *((uint8_t *)&ack_msg.cmd_data1);

	return 0;
}

STATIC int32_t hbm_get_temperature_from_ipc(uint32_t dev_id, uint32_t *temperature)
{
	int32_t ret;
	uint64_t value;

	ret = hbm_id_check(dev_id);
	if (ret != 0) {
		memory_drv_err("Parameter error. (dev_id=%u; ret=%d)\n", dev_id, ret);
		return ret;
	}

	ret = hbm_get_info_from_ipc(dev_id, (uint8_t *)&value);
	if (ret != 0) {
		memory_drv_err("Failed to obtain the power through the ipc."
			"(dev_id=%u; ret=%d)\n", dev_id, ret);
		return ret;
	}

	*temperature = (uint32_t)value;

	return 0;
}
#endif

#ifdef CFG_LPM_INFO_FROM_SHAREMEM
#define HBM_MEM_MAX		   32
#define DMS_NULL_USHORT	   0xFFFF
#define DMS_CRC_POLYNOMIAL	0x1021
#define DMS_BITS_PER_BYTE	 8
#define DMS_BIT15			 0x8000

struct hbm_info_entry {
	uint32_t type;
	uint32_t offset;
	uint32_t len;
};

struct hbm_sharemem_header {
	uint32_t magic_num;
	uint32_t version;
	uint32_t entry_num;
	struct hbm_info_entry entry[HBM_MEM_MAX];
	uint32_t crc16;
};

STATIC unsigned short hbm_crc16(unsigned char *data, unsigned short len)
{
	unsigned short val = DMS_NULL_USHORT;
	const unsigned short poly = DMS_CRC_POLYNOMIAL;
	unsigned char ch;
	unsigned int i;

	while (len != 0) {
		ch = *(data++);
		val ^= (ch << DMS_BITS_PER_BYTE);
		for (i = 0; i < DMS_BITS_PER_BYTE; i++) {
			if (val & DMS_BIT15) {
				val = (val << 1) ^ poly;
			} else {
				val = val << 1;
			}
		}
		len--;
	}
	return (val);
}

STATIC int32_t hbm_get_sharemem_head_info(unsigned long base_phy_addr, unsigned long map_size,
	struct hbm_sharemem_header *head)
{
	int32_t ret;
	void __iomem *viraddr = NULL;
	unsigned long type_addr;
	uint32_t i;
	unsigned short crc16_value = 0;
	unsigned short crc16_check_len = 0;

	if (head == NULL) {
		memory_drv_err("Parameter error.\n");
		return -EINVAL;
	}

	viraddr = ioremap(base_phy_addr, map_size);
	if (viraddr == NULL) {
		memory_drv_err("Remap memory address space failed.\n");
		return -EFAULT;
	}

	head->magic_num = readl(viraddr);
	head->version = readl(viraddr + sizeof(uint32_t));
	head->entry_num = readl(viraddr + sizeof(uint32_t) * 2U);
	if (head->entry_num > HBM_MEM_MAX) {
		memory_drv_err("Incorrect entry_num. (entry_num=%u)\n", head->entry_num);
		ret = -EINVAL;
		goto OUT;
	}
	crc16_check_len = sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t);

	for (i = 0; i < head->entry_num; i++) {
		type_addr = (unsigned long)viraddr + (sizeof(uint32_t) + sizeof(uint32_t)+ sizeof(uint32_t)) +
			(sizeof(struct hbm_info_entry) * i);

		head->entry[i].type = readl((void __iomem *)(uintptr_t)type_addr);
		head->entry[i].offset = readl((void __iomem *)(uintptr_t)(type_addr + sizeof(uint32_t)));
		head->entry[i].len = readl((void __iomem *)(uintptr_t)(type_addr + (sizeof(uint32_t) * 2)));
		crc16_check_len += sizeof(struct hbm_info_entry);
	}
	head->crc16 = readl(viraddr + crc16_check_len);

	crc16_value = hbm_crc16((uint8_t *)head, crc16_check_len);
	if (crc16_value != head->crc16) {
		memory_drv_err("The CRC16 check fail, and the memory is overwritten. (actual=%u; expect=%u)\n",
			crc16_value, head->crc16);
		ret = -EINVAL;
		goto OUT;
	}

	ret = 0;
OUT:
	(void)iounmap(viraddr);
	viraddr = NULL;
	return ret;
}

/* sharemem head magic num */
#define SHAREMEM_MAGIC_NUM	0x4c50534d
#define SHAREMEM_MAX_SIZE	 0x1F000

STATIC int32_t hbm_get_sharemem_entry_info(struct hbm_info_entry *info)
{
	int32_t ret;
	struct hbm_sharemem_header head = {0};

	if (info == NULL) {
		memory_drv_err("Parameter error.\n");
		return -EINVAL;
	}

	if (info->type >= HBM_MEM_MAX) {
		memory_drv_err("Get share memory entry info type error.\n");
		return -EINVAL;
	}

	ret = hbm_get_sharemem_head_info(SHAREMEM_BASE_ADDR, sizeof(struct hbm_sharemem_header), &head);
	if ((ret != 0) || (head.magic_num != SHAREMEM_MAGIC_NUM)) {
		memory_drv_err("Get head information failed. (ret=%d; magic_num=%#x)\n", ret, head.magic_num);
		return -ENODATA;
	}

	if (info->type != head.entry[info->type].type) {
		memory_drv_err("Get head information type not match. (info.type=%u; entry.type=%u)\n", info->type,
			head.entry[info->type].type);
		return -ENODATA;
	}

	info->offset = head.entry[info->type].offset;
	info->len = head.entry[info->type].len;

	if ((info->offset > SHAREMEM_MAX_SIZE) || (info->len > SHAREMEM_MAX_SIZE) ||
		((info->offset + info->len) > SHAREMEM_MAX_SIZE)) {
		memory_drv_err("Offset or length over max value. (offset=%u; len=%u)\n", info->offset, info->len);
		return -ENODATA;
	}

	return 0;
}

#define HBM_MEM_TEMP	  0
#define REG_OP_TYPE_WR	0
#define REG_OP_TYPE_RD	1
#define HBM_TEMP_OFFSET   4

STATIC int32_t hbm_sharemem_op(unsigned char op_type, unsigned long base_phy_addr, unsigned long offset,
	unsigned long map_size, uint32_t *val)
{
	void __iomem *viraddr = NULL;

	if ((map_size < sizeof(uint32_t)) || (offset > (map_size - sizeof(uint32_t)))) {
		memory_drv_err("Invalid offset. (offset = %lu; map_size = %lu)\n", offset, map_size);
		return -EINVAL;
	}

	viraddr = ioremap(base_phy_addr, map_size);
	if (viraddr == NULL) {
		memory_drv_err("Remap memory address space failed.\n");
		return -EFAULT;
	}

	if (op_type == REG_OP_TYPE_WR) {
		writel(*val, viraddr + offset);
	} else if (op_type == REG_OP_TYPE_RD) {
		*val = readl(viraddr + offset);
	}

	(void)iounmap(viraddr);
	viraddr = NULL;
	return 0;
}

STATIC int32_t hbm_get_temperature_from_sharemem(uint32_t dev_id, uint32_t *temperature)
{
	int32_t ret;
	struct hbm_info_entry info = {0};

	ret = hbm_id_check(dev_id);
	if (ret != 0) {
		memory_drv_err("Parameter error. (dev_id=%u; ret=%d)\n", dev_id, ret);
		return ret;
	}

	info.type = HBM_MEM_TEMP;
	ret = hbm_get_sharemem_entry_info(&info);
	if (ret != 0) {
		memory_drv_err("Get temperature entry information failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
		return ret;
	}

	ret = hbm_sharemem_op(REG_OP_TYPE_RD, SHAREMEM_BASE_ADDR + info.offset, HBM_TEMP_OFFSET, info.len, temperature);
	if (ret != 0) {
		memory_drv_err("Get temperature failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
		return ret;
	}
	*temperature &= 0xFFFF;

	return 0;
}
#endif

#if defined(CFG_LPM_INFO_FROM_IPC) || defined(CFG_LPM_INFO_FROM_SHAREMEM)
int32_t memory_get_hbm_temperature(void *feature, char *in, uint32_t in_len, char *out, uint32_t out_len)
{
	int32_t ret;
	uint32_t dev_id;
	uint32_t temperature = 0;

	if ((in == NULL) || (in_len != sizeof(uint32_t)) || (out == NULL) || (out_len != in_len)) {
		memory_drv_err("Invalid parameter. (in=%pK; in_len=%u; out=%pK; out_len=%u)\n", in, in_len, out, out_len);
		return -EINVAL;
	}

	ret = memcpy_s(&dev_id, sizeof(uint32_t), in, in_len);
	if (ret != 0) {
		memory_drv_err("Memcpy_s failed. (ret=%d)\n", ret);
		return ret;
	}

	if (dms_is_devid_valid(dev_id) == false) {
		memory_drv_err("Invalid parameter. (dev_id=%u)\n", dev_id);
		return -EINVAL;
	}

#ifdef CFG_LPM_INFO_FROM_IPC
	ret = hbm_get_temperature_from_ipc(dev_id, &temperature);
	if (ret != 0) {
		memory_drv_err("Get hbm temperature from ipc failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
		return ret;
	}
#endif

#ifdef CFG_LPM_INFO_FROM_SHAREMEM
	ret = hbm_get_temperature_from_sharemem(dev_id, &temperature);
	if (ret != 0) {
		memory_drv_err("Get hbm temperature from sharemem failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
		return ret;
	}
#endif

	ret = memcpy_s(out, out_len, &temperature, sizeof(uint32_t));
	if (ret != 0) {
		memory_drv_err("Memcpy_s failed. (ret=%d)\n", ret);
		return ret;
	}
	memory_drv_debug("Get hbm temperature success. (dev_id=%u; temperature=%u)\n", dev_id, temperature);
	return 0;
}
#endif
