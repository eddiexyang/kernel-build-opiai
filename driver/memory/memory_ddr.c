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
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <asm/uaccess.h>
#include <linux/time.h>
#include <linux/of_gpio.h>
#include "devdrv_pm.h"
#include "devdrv_common.h"
#include "devdrv_manager.h"
#include "devdrv_manager_common.h"
#include "dms_define.h"
#include "memory_log.h"
#include "memory_ddr_cfg.h"
#include "memory_ecc_cfg.h"
#include "memory_ddr_info.h"
#include "memory_ddr_reg.h"
#include "memory_ddr.h"
#include "memory_scan.h"
#if defined (CFG_SOC_PLATFORM_MDC_V51) || defined (CFG_SOC_PLATFORM_MDC_V51LITE)
#include "devdrv_info.h"
#endif
#include "dms_cmd_def.h"
#if defined(CFG_FEATURE_LP_ENABLE)
#include "memory_dev_manage.h"
#endif

#if !defined (CFG_SOC_PLATFORM_MDC_V51) && !defined (CFG_SOC_PLATFORM_MDC_V51LITE)
#define DDR_FREQ		 1
#define DDR_INIT		 6
#define DDR_STATISTIC	7
#endif

#ifdef CFG_INIT_DDR_STATISTICS
#define DDR_CHAN_MASK_IOREMAP_SIZE    0x20U
#endif

#ifndef CFG_MEMORY_DDR_STUB
STATIC ddr_prof_cfg_t g_ddr_prof_cfg[MAX_CHIP_NUM] = {0};
STATIC ddr_flux_info_t g_ddr_flux_info_bk[MAX_CHIP_NUM] = {0};
STATIC struct timespec64 g_time_stamp_start[MAX_CHIP_NUM] = {0};
STATIC struct mutex g_ddr_reg_lock[MAX_CHIP_NUM];
STATIC struct mutex g_ddr_init_lock[MAX_CHIP_NUM];
void __iomem *g_map_regs[MAX_CHIP_NUM][DDR_MAX_CHAN_NUM] = {NULL};

#ifdef CFG_MEMORY_DDR_INFO_FROM_REG
#define DDRC_SSCANF_NUM 1
STATIC void set_ddr_bit_map(struct ddrc_info_t *info)
{
	unsigned long ddrc_reg_base[DDR_MAX_CHAN_NUM] = {DDRC_CHAN0_REG_BASE, DDRC_CHAN1_REG_BASE, DDRC_CHAN2_REG_BASE,
		DDRC_CHAN3_REG_BASE, DDRC_CHAN4_REG_BASE, DDRC_CHAN5_REG_BASE, DDRC_CHAN6_REG_BASE, DDRC_CHAN7_REG_BASE};
	unsigned int i, j;

	info->bitmap = 0;
	for (i = 0; i < info->chan_num; i++) {
		for (j = 0; j < DDR_MAX_CHAN_NUM; j++) {
			if (info->reg_base_array[i] == ddrc_reg_base[j]) {
				info->bitmap |= (1 << j);
			}
		}
	}
}

int get_ddr_info_from_dts(struct ddrc_info_t *info)
{
	int ret;
	int i = 0;
	struct device_node *np = NULL;
	struct device_node *soc = NULL;
	struct device_node *ddr = NULL;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,mini");
	if (np == NULL) {
		memory_drv_err("Can't find node.\n");
		return -ENOENT;
	}
	soc = of_find_compatible_node(np, NULL, "simple-bus");
	if (soc == NULL) {
		memory_drv_err("Can't find node.\n");
		return -ENOENT;
	}
	ddr = of_find_node_by_name(soc, "ddrc-os");
	if (ddr == NULL) {
		memory_drv_err("Can't find node.\n");
		return -ENOENT;
	}
	for (i = 0; i < DDR_MAX_CHAN_NUM; i++) {
		ret = sscanf_s(ddr->full_name, "ddrc-os@%lx", &info->reg_base_array[i]);
		if (ret != DDRC_SSCANF_NUM) {
			memory_drv_err("Sscanf_s error. (sscanf_s_return_value=%d)\n", ret);
			return -ENOENT;
		}

		ddr = of_get_next_child(soc, ddr);
		if (ddr == NULL) {
			memory_drv_err("Can't find node.\n");
			return -ENOENT;
		}
	}
	of_node_put(ddr);
	of_node_put(soc);
	of_node_put(np);

	info->chan_num = DDR_MAX_CHAN_NUM;
	set_ddr_bit_map(info);

	memory_drv_debug("Get ddr info from dts success. (chan_num=%u; bitmap=%u)\n", info->chan_num, info->bitmap);
	return 0;
}
#endif

STATIC void unmap_ddr_reg(void)
{
	unsigned int i;
	unsigned int j;

	for (i = 0; i < MAX_CHIP_NUM; i++) {
		mutex_destroy(&g_ddr_reg_lock[i]);
		mutex_destroy(&g_ddr_init_lock[i]);
		for (j = 0; j < DDR_MAX_CHAN_NUM; j++) {
			if (g_map_regs[i][j] != NULL) {
				(void)iounmap(g_map_regs[i][j]);
				g_map_regs[i][j] = NULL;
			}
		}
	}
	memory_drv_debug("Iounmap success. (max_chip_num=%d; max_chan_num=%d)\n", MAX_CHIP_NUM, DDR_MAX_CHAN_NUM);
}

STATIC int map_ddr_reg(void)
{
	unsigned int i;
	unsigned int j;
	ddr_hw_info_t *ddr_info_map = NULL;
#ifdef CFG_SOC_PLATFORM_MDC_V51
	int ret;
	int chip_type;
#endif

	for (i = 0; i < MAX_CHIP_NUM; i++) {
		mutex_init(&g_ddr_reg_lock[i]);
		mutex_init(&g_ddr_init_lock[i]);
	}

	ddr_info_map = &g_ddr_hw_info;

#ifdef CFG_SOC_PLATFORM_MDC_V51
	ret = devdrv_manager_get_chip_type(&chip_type);
	if (ret != 0) {
		memory_drv_err("get chip_type failed, ret = %d.\n", ret);
		return ret;
	}
	if (chip_type == CHIP_TYPE_ASCEND_V2) {
		ddr_info_map = &g_ddr_hw_info_bs9sx1a;
	}
#endif

	for (i = 0; i < MAX_CHIP_NUM; i++) {
		for (j = 0; j < DDR_MAX_CHAN_NUM; j++) {
			g_map_regs[i][j] = ioremap(ddr_info_map->reg_base[j] + i * DDRC_CHIP_REG_BASE_OFFSET, DDRC_MAP_SIZE);
			if (g_map_regs[i][j] == NULL) {
				memory_drv_err("Failed to ioremap ddr memory. (chip_num=%u; chan_num=%u)\n", i, j);
				unmap_ddr_reg();
				return -ENOMEM;
			}
		}
	}
	memory_drv_info("Ioremap success. (max_chip_num=%d; max_chan_num=%d)\n", MAX_CHIP_NUM, DDR_MAX_CHAN_NUM);
	return 0;
}

int memory_map_ddr_init(void)
{
	int ret;

	ret = map_ddr_reg();
	if (ret != 0) {
		memory_drv_err("Failed to init ddr map. (ret=%d)\n", ret);
		return ret;
	}

	memory_drv_debug("Memory map ddr init success.\n");
	return 0;
}

void memory_map_ddr_exit(void)
{
	unmap_ddr_reg();
	memory_drv_debug("memory map ddr exit success.\n");
}

int memory_ddr_reg_op(unsigned char op_type, const void __iomem *vir_addr, unsigned long reg_offset, unsigned int *val)
{
	if ((vir_addr == NULL) || (val == NULL) || ((op_type != DDRC_REG_WR) && (op_type != DDRC_REG_RD))) {
		memory_drv_err("Invalid para. (vir_addr=%pK; val=%pK)\n", vir_addr, val);
		*val = 0;
		return -EINVAL;
	}

	if (op_type == DDRC_REG_WR) {
		writel(*val, (void __iomem *)(uintptr_t)((long)(uintptr_t)vir_addr + reg_offset));
	} else {
		*val = readl((void __iomem *)(uintptr_t)((long)(uintptr_t)vir_addr + reg_offset));
	}

	return 0;
}

STATIC unsigned int get_flux_diff(unsigned int start, unsigned int end)
{
	unsigned int flux_diff;

	flux_diff = (unsigned int)((unsigned long long)end + ((unsigned long long)UINT_TYPE_MAX + 1) - start) %
		((unsigned long long)UINT_TYPE_MAX + 1);

	return flux_diff;
}

STATIC unsigned long get_ddr_time_interval(struct timespec64 start, struct timespec64 end)
{
	unsigned long time_use;

	time_use = (end.tv_sec - start.tv_sec) * US_PER_SECOND + (end.tv_nsec - start.tv_nsec) / NSEC_PER_USEC;
	return time_use;
}

STATIC int clear_flux_reg(const void __iomem *reg_base)
{
	int ret;
	int i;
	unsigned int op_val = 0;
	unsigned long reg_offset[DDRC_CLEAR_ITEM_NUM] = { DDRC_HISI_FLUX_RD_OFFSET, DDRC_HISI_FLUX_WR_OFFSET };

	/* Write flux register to 0 */
	for (i = 0; i < DDRC_CLEAR_ITEM_NUM; i++) {
		ret = memory_ddr_reg_op(DDRC_REG_WR, reg_base, reg_offset[i], &op_val);
		if (ret < 0) {
			memory_drv_err("Failed to clear ddr flux reg. (ret=%d; item_num=%d).\n", ret, i);
			return ret;
		}
	}

	return 0;
}

STATIC int set_master_id(unsigned int id, const void __iomem *reg_base)
{
	int ret;
	unsigned int op_val;

	if (id != DDRC_MASTER_ID_ALL) {
		/* Set master id */
		op_val = DDRC_CFG_STAID_SET | (id << DDRC_CFG_STAID_OFFSET);
		ret = memory_ddr_reg_op(DDRC_REG_WR, reg_base, DDRC_CFG_STAID, &op_val);
		if (ret < 0) {
			memory_drv_err("Failed to set master id for master id. (ret=%d; master_id=%u)\n", ret, id);
			return ret;
		}

		/* Set master id mask */
		op_val = DDRC_CFG_STAIDMSK_SET;
		ret = memory_ddr_reg_op(DDRC_REG_WR, reg_base, DDRC_CFG_STAIDMSK, &op_val);
		if (ret < 0) {
			memory_drv_err("Failed to set master id for master id mask. (ret=%d; master_id=%u)\n", ret, id);
			return ret;
		}
	}

	memory_drv_debug("Set master id success.\n");
	return 0;
}

STATIC int enable_flux_statistics(unsigned int device_id, const ddr_hw_info_t *info)
{
	int ret = 0;
	unsigned int i;
	unsigned int op_val;
	void __iomem *reg_base = NULL;

	mutex_lock(&g_ddr_reg_lock[device_id]);
	for (i = 0; i < DDR_MAX_CHAN_NUM; i++) {
		if ((info->channel_bitmap & (1 << i)) == 0) {
			continue;
		}
		reg_base = g_map_regs[device_id][i];
		/* disable flux statistic */
		op_val = 0;
		ret = memory_ddr_reg_op(DDRC_REG_WR, reg_base, DDRC_CTRL_PERF_OFFSET, &op_val);
		if (ret < 0) {
			mutex_unlock(&g_ddr_reg_lock[device_id]);
			memory_drv_err("Failed to write reg. (dev_id=%u; ret=%d)\n", device_id, ret);
			return ret;
		}

		ret = set_master_id(g_ddr_prof_cfg[device_id].master_id, reg_base);
		if (ret < 0) {
			memory_drv_err("Failed to set master id. (dev_id=%u; ret=%d)\n", device_id, ret);
			goto OUT;
		}

		ret = clear_flux_reg(reg_base);
		if (ret < 0) {
			memory_drv_err("Failed to clear flux reg. (dev_id=%u; ret=%d)\n", device_id, ret);
			goto OUT;
		}

		/* Enable flux statistic */
		op_val = DDRC_PERF_EN_SET;
		ret = memory_ddr_reg_op(DDRC_REG_WR, reg_base, DDRC_CTRL_PERF_OFFSET, &op_val);
		if (ret < 0) {
			memory_drv_err("Failed to enalbe flux statistic. (dev_id=%u; ret=%d)\n", device_id, ret);
			goto OUT;
		}
	}

OUT:
	mutex_unlock(&g_ddr_reg_lock[device_id]);
	return ret;
}

int memory_ddr_stat_init_from_reg(unsigned int device_id, unsigned int chan_num, unsigned int bitmap)
{
	int ret;
	STATIC unsigned int init_flag[MAX_CHIP_NUM] = {0};

	if ((chan_num == 0) || (chan_num > DDR_MAX_CHAN_NUM) || (device_id >= MAX_CHIP_NUM)) {
		devdrv_drv_err("Invalid parameter. (chan_num=%u; device_id=%u)\n", chan_num, device_id);
		return -EINVAL;
	}

	mutex_lock(&g_ddr_init_lock[device_id]);
	if (init_flag[device_id] == 0) {
		g_ddr_hw_info.channel_num = chan_num;
		g_ddr_hw_info.channel_bitmap = bitmap;
		g_ddr_prof_cfg[device_id].master_id = DDRC_MASTER_ID_ALL;

		ret = enable_flux_statistics(device_id, &g_ddr_hw_info);
		if (ret < 0) {
			mutex_unlock(&g_ddr_init_lock[device_id]);
			memory_drv_err("Failed to init ddr stat. (ret=%d; dev_id=%u)\n", ret, device_id);
			return ret;
		}

		init_flag[device_id] = 1;
	}
	mutex_unlock(&g_ddr_init_lock[device_id]);

	memory_drv_debug("Init ddr stat success. (dev_id=%u; init_flag[0]=%u)\n", device_id, init_flag[0]);
	return 0;
}

STATIC int get_ddr_statistic_info_from_reg(unsigned int device_id, ddr_flux_info_t *flux_info)
{
	int ret;
	int i = 0;
	int j;
	unsigned long reg_offset[DDRC_INFO_ITEM_NUM] = { DDRC_HISI_FLUX_RD_OFFSET, DDRC_HISI_FLUX_WR_OFFSET,
		DDRC_HISI_FLUXID_WR_OFFSET, DDRC_HISI_FLUXID_RD_OFFSET};
	unsigned int *val[DDRC_INFO_ITEM_NUM] = {NULL};
	struct timespec64 tmp_timestamp = { 0 };

	val[i++] = flux_info->flux_rd;
	val[i++] = flux_info->flux_wr;
	val[i++] = flux_info->id_flux_wr;
	val[i++] = flux_info->id_flux_rd;

	ktime_get_raw_ts64(&tmp_timestamp);
	for (i = 0; i < DDR_MAX_CHAN_NUM; i++) {
		/* channel bitmap is zero, set default value */
		if ((g_ddr_hw_info.channel_bitmap & (1 << i)) == 0) {
			flux_info->time_stamp[i].tv_sec = 0;
			flux_info->time_stamp[i].tv_nsec = 0;
			flux_info->flux_rd[i] = 0;
			flux_info->flux_wr[i] = 0;
			flux_info->id_flux_rd[i] = 0;
			flux_info->id_flux_wr[i] = 0;
			continue;
		}

		flux_info->time_stamp[i] = tmp_timestamp;
		for (j = 0; j < DDRC_INFO_ITEM_NUM; j++) {
			ret = memory_ddr_reg_op(DDRC_REG_RD, g_map_regs[device_id][i], reg_offset[j], &val[j][i]);
			if (ret < 0) {
				memory_drv_err("Failed to get stat info. (ret=%d; dev_id=%u; chan_num=%d; item_num=%d)\n", ret,
					device_id, i, j);
				return ret;
			}
		}
	}
	return 0;
}
#ifdef CFG_FEATURE_LP_ENABLE
#define ACK_MSG_LEN 4
int memory_ddr_stat_init_from_ipc(unsigned int device_id)
{
	int ret;
	STATIC unsigned int init_flag[MAX_CHIP_NUM] = {0};
	mutex_lock(&g_ddr_init_lock[device_id]);
	if (init_flag[device_id] == 0) {
		unsigned long long ack_msg[ACK_MSG_LEN] = {0};
		ret = devdrv_query_ddr_statistic_from_lp(device_id, DDR_INIT, (void*)ack_msg);
		if (ret != 0) {
			mutex_unlock(&g_ddr_init_lock[device_id]);
			memory_drv_err("Failed to init ddr statistic. (dev_id=%u; ret=%d)\n", device_id, ret);
			return ret;
		}
		init_flag[device_id] = 1;
	}
	mutex_unlock(&g_ddr_init_lock[device_id]);
	memory_drv_debug("Init ddr stat success. (dev_id=%u; init_flag[0]=%u)\n", device_id, init_flag[0]);
	return 0;
}

void save_ddr_statistic_info(unsigned long long *ack_msg, ddr_flux_info_t *flux_info)
{
	int idx = 1;
	flux_info->total_time_stamp = ack_msg[idx++];
	flux_info->total_flux_rd = ack_msg[idx++];
	flux_info->total_flux_wr = ack_msg[idx++];
}

int get_ddr_statistic_info_from_ipc(unsigned int device_id, ddr_flux_info_t *flux_info)
{
	int ret;
	unsigned long long ack_msg[ACK_MSG_LEN] = {0};
	ret = devdrv_query_ddr_statistic_from_lp(device_id, DDR_STATISTIC, (void *)ack_msg);
	if (ret != 0) {
		memory_drv_err("Failed to get ddr statistic. (dev_id=%u; ret=%d)\n", device_id, ret);
		return ret;
	}
	save_ddr_statistic_info(ack_msg, flux_info);
	memory_drv_debug("Get ddr statistic success. (dev_id=%u;  ret=%d)\n", device_id, ret);

	return 0;
}
#endif
STATIC ddr_prof_data_t *update_ddr_buff_index(ddr_prof_data_t *buf, unsigned int *data_len, unsigned int max_len)
{
	ddr_prof_data_t *prof_data_pt = NULL;
	*data_len += sizeof(ddr_prof_data_t);

	if (*data_len < max_len) {
		prof_data_pt = (ddr_prof_data_t *)(uintptr_t)((unsigned long)(uintptr_t)buf + *data_len);
	} else {
		memory_drv_err("Buffer is overflow.\n");
		return NULL;
	}

	return prof_data_pt;
}

STATIC int check_ddr_user_data(const ddr_prof_cfg_t *data, unsigned int data_size)
{
	ddr_prof_cfg_t *prof_cfg_pt = (ddr_prof_cfg_t *)data;

	/* check event num */
	if ((prof_cfg_pt->event_num == 0) || (prof_cfg_pt->event_num > MAX_EVENT_NUM)) {
		memory_drv_err("Cfg event_num is error. (event_num=%u)\n", prof_cfg_pt->event_num);
		return -EINVAL;
	}

	/* check event value */
	if (((prof_cfg_pt->event_num == 1) && ((data_size != (sizeof(ddr_prof_cfg_t) - sizeof(unsigned int))) ||
		(prof_cfg_pt->event[0] >= MAX_EVENT_NUM))) || ((prof_cfg_pt->event_num == MAX_EVENT_NUM) &&
		((data_size != sizeof(ddr_prof_cfg_t)) || ((prof_cfg_pt->event[0] >= MAX_EVENT_NUM) ||
		(prof_cfg_pt->event[1] >= MAX_EVENT_NUM))))) {
		memory_drv_err("Event value is error. (event_num=%u; data_size=%u)\n", prof_cfg_pt->event_num, data_size);
		return -EINVAL;
	}

	/* check master id */
	if ((prof_cfg_pt->master_id != DDRC_MASTER_ID_ALL) && (prof_cfg_pt->master_id >= DDR_MAX_CHAN_NUM)) {
		memory_drv_err("Master id is error. (master_id=%u)\n", prof_cfg_pt->master_id);
		return -EINVAL;
	}

	return 0;
}

/*
 * Set ddr profiling data buffer, for the user.
 */
STATIC unsigned int set_ddr_prof_buff(unsigned int device_id, void *buf, unsigned int len, ddr_flux_info_t *info)
{
	ddr_prof_data_t *prof_data_pt = NULL;
	unsigned int data_len = 0;
	unsigned int i, j;

	prof_data_pt = (ddr_prof_data_t *)buf;
	for (i = 0; i < g_ddr_prof_cfg[device_id].event_num; i++) {
		for (j = 0; j < DDR_MAX_CHAN_NUM; j++) {
			if ((g_ddr_hw_info.channel_bitmap & (1 << j)) == 0) {
				continue;
			}
			prof_data_pt->event = DDR_PROF_EVENT_RD;
			prof_data_pt->count = info->flux_rd[j];
			/* set write event data */
			if (g_ddr_prof_cfg[device_id].event[i] == DDR_PROF_EVENT_WR) {
				prof_data_pt->event = DDR_PROF_EVENT_WR;
				prof_data_pt->count = info->flux_wr[j];
			}

			prof_data_pt->timestamp = (unsigned int)get_ddr_time_interval(g_time_stamp_start[device_id],
				info->time_stamp[j]);
			prof_data_pt->ddr_id = j;
			prof_data_pt->master_id = DDRC_MASTER_ID_ALL;
			prof_data_pt = update_ddr_buff_index((ddr_prof_data_t *)buf, &data_len, len);
			if (prof_data_pt == NULL) {
				memory_drv_err("Buffer is overflow. (dev_id=%u)\n", device_id);
				return len;
			}

			/* if master id is set, set id event */
			if (g_ddr_prof_cfg[device_id].master_id == DDRC_MASTER_ID_ALL) {
				continue;
			}
			/* prof_data_pt already update, and should set again */
			if (g_ddr_prof_cfg[device_id].event[i] == DDR_PROF_EVENT_WR) {
				prof_data_pt->event = DDR_PROF_EVENT_WR;
				prof_data_pt->count = info->flux_wr[j];
			} else {
				prof_data_pt->event = DDR_PROF_EVENT_RD;
				prof_data_pt->count = info->flux_rd[j];
			}

			prof_data_pt->timestamp = (unsigned int)get_ddr_time_interval(g_time_stamp_start[device_id],
				info->time_stamp[j]);
			prof_data_pt->ddr_id = j;
			prof_data_pt->master_id = g_ddr_prof_cfg[device_id].master_id;

			prof_data_pt = update_ddr_buff_index((ddr_prof_data_t *)buf, &data_len, len);
			if (prof_data_pt == NULL) {
				memory_drv_err("Buffer is overflow. (dev_id=%u)\n", device_id);
				return len;
			}
		}
	}

	return data_len;
}

#ifdef CFG_INIT_DDR_STATISTICS
STATIC int memory_ddr_get_chan_mask(uint32_t *chan_mask)
{
#ifdef CFG_DDR_REG_ADDR_910
	*chan_mask = DDR_CHAN_BITMAP;    // 910没有PG，只有AG
#else
	void __iomem *ddr_chan_mask_vir_addr = NULL;
	ddr_chan_mask_vir_addr = ioremap(DDR_CHAN_MASK_IOREMAP_ADDR, DDR_CHAN_MASK_IOREMAP_SIZE);
	if (ddr_chan_mask_vir_addr == NULL) {
		memory_drv_err("ddr info ioremap fail\n");
		return -1;
	}
	*chan_mask = readl(ddr_chan_mask_vir_addr);    // 310B、710、610、610 lite获取通道自适应结果
	iounmap(ddr_chan_mask_vir_addr);
	ddr_chan_mask_vir_addr = NULL;
	memory_drv_info("ddr chan_mask: 0x%x\n", *chan_mask);
#endif
	return 0;
}
#endif

/*
 * DDR profiling sample func, callback for the profiling driver.
 */
int memory_ddr_prof_sample(struct prof_peri_para para)
{
	int ret;
	ddr_flux_info_t cur_flux_info = { 0 };
	ddr_flux_info_t tx_flux_info = { 0 };
	ddr_flux_info_t *bk_info_pt = NULL;
	unsigned int device_id = para.device_id;
	void *buf = para.buff;
	unsigned int len = para.buff_len;
	unsigned int i;
	int buf_data_len;

	if ((buf == NULL) || (dms_is_devid_valid(device_id) == false) || (len <
		g_ddr_prof_cfg[device_id].event_num * DDR_MAX_CHAN_NUM * COMMON_MASTER)) {
		memory_drv_err("Invalid parameter. (buf=%pK; device_id=%u; len=%u)\n", buf, device_id, len);
		return -EINVAL;
	}

	/* 1. get ddr flux statistic info */
	ret = get_ddr_statistic_info_from_reg(device_id, &cur_flux_info);
	if (ret < 0) {
		memory_drv_err("Failed to get ddr flux statistic info. (dev_id=%u; ret=%d)\n", device_id, ret);
		return ret;
	}

	/* 2. calculate flux difference and record it */
	for (i = 0; i < DDR_MAX_CHAN_NUM; i++) {
		tx_flux_info.flux_rd[i] =
			get_flux_diff(g_ddr_flux_info_bk[device_id].flux_rd[i], cur_flux_info.flux_rd[i]);
		tx_flux_info.flux_wr[i] =
			get_flux_diff(g_ddr_flux_info_bk[device_id].flux_wr[i], cur_flux_info.flux_wr[i]);
		tx_flux_info.id_flux_rd[i] =
			get_flux_diff(g_ddr_flux_info_bk[device_id].id_flux_rd[i], cur_flux_info.id_flux_rd[i]);
		tx_flux_info.id_flux_wr[i] =
			get_flux_diff(g_ddr_flux_info_bk[device_id].id_flux_wr[i], cur_flux_info.id_flux_wr[i]);
		tx_flux_info.time_stamp[i] = cur_flux_info.time_stamp[i];
		memory_drv_debug("tx_flux. (dev_id=%u; rd[%u]=%u; wr[%u]=%u)\n", device_id, i, tx_flux_info.flux_rd[i], i,
			tx_flux_info.flux_wr[i]);
	}

	/* 3. backup current flux info */
	bk_info_pt = &(g_ddr_flux_info_bk[device_id]);
	ret = memcpy_s(bk_info_pt, sizeof(ddr_flux_info_t), &cur_flux_info, sizeof(ddr_flux_info_t));
	if (ret != 0) {
		memory_drv_err("Failed to backup current flux info. (dev_id=%u; ret=%d)\n", device_id, ret);
		return -ENOMEM;
	}

	/* 4. set data buff */
	buf_data_len = set_ddr_prof_buff(device_id, buf, len, &tx_flux_info);

	memory_drv_debug("Sample ddr data success. (dev_id=%u)\n", device_id);
	return buf_data_len;
}
EXPORT_SYMBOL(memory_ddr_prof_sample);

/*
 * Get DDR profiling cfg data and init statistic, callback for the profiling driver.
 */
int memory_ddr_prof_init_user_cfg(struct prof_peri_para para)
{
	unsigned int device_id = para.device_id;
	void *data = para.user_data;
	unsigned int data_size = para.user_data_len;
	int ret;
#ifdef CFG_INIT_DDR_STATISTICS
	uint32_t chan_mask;
#endif

	if ((data == NULL) || (dms_is_devid_valid(device_id) == false) ||
		((data_size != sizeof(ddr_prof_cfg_t)) && (data_size != (sizeof(ddr_prof_cfg_t) - sizeof(unsigned int))))) {
		memory_drv_err("Invalid parameter. (data=%pK; device_id=%u; data_size=%u)\n", data, device_id, data_size);
		return -EINVAL;
	}

#ifdef CFG_INIT_DDR_STATISTICS
	ret = memory_ddr_get_chan_mask(&chan_mask);
	if (ret != 0) {
		memory_drv_err("ddr chan mask get error. (ret=%d; dev_id=%u)\n", ret, device_id);
		return ret;
	}

	ret = memory_ddr_stat_init_from_reg(device_id, DDR_MAX_CHAN_NUM, chan_mask);
	if (ret < 0) {
		memory_drv_err("Failed to init ddr stat. (ret=%d; ddev_id=%u)\n", ret, device_id);
		return ret;
	}
#endif

	ret = check_ddr_user_data((ddr_prof_cfg_t *)data, data_size);
	if (ret < 0) {
		memory_drv_err("Check cfg data error. (ret=%d; dev_id=%u)\n", ret, device_id);
		return ret;
	}

	ret = memcpy_s(&g_ddr_prof_cfg[device_id], sizeof(ddr_prof_cfg_t), data, data_size);
	if (ret != 0) {
		memory_drv_err("Failed to memcpy ddr prof cfg. (ret=%d; dev_id=%u)n", ret, device_id);
		return ret;
	}

	memory_drv_info("Device_id:%u.\n", device_id);
	memory_drv_info("Ddr_prof_cfg_period:%u.\n", g_ddr_prof_cfg[device_id].period);
	memory_drv_info("Ddr_prof_cfg_master_id:%u.\n", g_ddr_prof_cfg[device_id].master_id);
	memory_drv_info("Ddr_prof_cfg_event_num:%u.\n", g_ddr_prof_cfg[device_id].event_num);
	memory_drv_info("Ddr_prof_cfg_event[0]:%u.\n", g_ddr_prof_cfg[device_id].event[0]);
	memory_drv_info("Ddr_prof_cfg_event[1]:%u.\n", g_ddr_prof_cfg[device_id].event[1]);

	/* if master id is invalid, set master id and restart flux statistics */
	if (g_ddr_prof_cfg[device_id].master_id != DDRC_MASTER_ID_ALL) {
		ret = enable_flux_statistics(device_id, (const ddr_hw_info_t *)&g_ddr_hw_info);
		if (ret < 0) {
			memory_drv_err("Failed to enable ddr flux stat. (dev_id=%u; ret=%d)\n", device_id, ret);
			return ret;
		}
	}

	/* get statistic info and fill backup struct after statistic enable */
	ret = get_ddr_statistic_info_from_reg(device_id, &g_ddr_flux_info_bk[device_id]);
	if (ret < 0) {
		memory_drv_err("Failed to get ddr stat info . (dev_id=%u; ret=%d)\n", device_id, ret);
		return ret;
	}

	g_time_stamp_start[device_id] = g_ddr_flux_info_bk[device_id].time_stamp[0];

	memory_drv_debug("Set ddr user cfg success. (dev_id=%u)\n", device_id);
	return 0;
}
EXPORT_SYMBOL(memory_ddr_prof_init_user_cfg);

STATIC int update_ddr_max_bandwidth(unsigned int dev_id, unsigned long *bandwidth)
{
#if defined CFG_DDR_BANDWIDTH_BY_FREQ
	int ret;
	unsigned int freq = 0;

#ifdef CFG_MEMORY_DDR_INFO_FROM_SHAREMEM
	ret = memory_get_ddr_info_from_sharemem(FREQ_OFFSET, &freq, sizeof(unsigned int));
#else
	ret = devdrv_get_freq_from_lp_memory(dev_id, DDR_FREQ, &freq);
#endif
	if (ret != 0) {
		memory_drv_err("Failed to get freq and bandwidth. (dev_id=%u; ret=%d)\n", dev_id, ret);
		return ret;
	}

	*bandwidth = (unsigned long)freq * DDRC_FREQ_DOUBLE * DDRC_DQ_NUM / DDRC_BPS_PER_BYTE;
	memory_drv_debug("Get freq and bandwidth success. (dev_id=%u; freq=%u; bw=%lu)\n", dev_id, freq, *bandwidth);
#endif

#if defined CFG_DDR_BANDWIDTH_STUB
	*bandwidth = (unsigned long)(4266) * DDRC_FREQ_DOUBLE * DDRC_DQ_NUM / DDRC_BPS_PER_BYTE; /* 4266 is stub for freq */
#endif

	return 0;
}

STATIC void calculate_time_interval(ddr_flux_info_t *ddr_flux_info, unsigned long *time_interval)
{
	unsigned long long start_us = ddr_flux_info[SAMPLE_FIRST].total_time_stamp;
	unsigned long long end_us = ddr_flux_info[SAMPLE_SECOND].total_time_stamp;
	memory_drv_debug("calculate time interval success. (start_us=%llu), (end_us=%llu)\n", start_us, end_us);
	*time_interval = DDRC_FLUX_STAT_TIME * US_PER_MSECOND;  // default time interval
	if (end_us > start_us) {
		unsigned long time_diff = (unsigned long)(end_us - start_us);
		*time_interval = (time_diff > (*time_interval)) ? time_diff : (*time_interval);
	}
}

STATIC void calculate_total_flux_info(unsigned int chan_num, ddr_flux_info_t *ddr_flux_info, unsigned long *total_flux)
{
	unsigned long long flux_rd_first;
	unsigned long long flux_rd_second;
	unsigned long long flux_wr_first;
	unsigned long long flux_wr_second;

	flux_rd_first = ddr_flux_info[SAMPLE_FIRST].total_flux_rd;
	flux_rd_second = ddr_flux_info[SAMPLE_SECOND].total_flux_rd;

	if (flux_rd_second >= flux_rd_first) {
		*total_flux = (unsigned long)(flux_rd_second - flux_rd_first);
	} else { // deal with reverse
		*total_flux = (unsigned long)(flux_rd_second + (UINT_TYPE_MAX - (unsigned long)flux_rd_first));
	}

	flux_wr_first = ddr_flux_info[SAMPLE_FIRST].total_flux_wr;
	flux_wr_second = ddr_flux_info[SAMPLE_SECOND].total_flux_wr;

	if (flux_wr_second >= flux_wr_first) {
		*total_flux += (unsigned long)(flux_wr_second - flux_wr_first);
	} else { // deal with reverse
		*total_flux += (unsigned long)(flux_wr_second + (UINT_TYPE_MAX - (unsigned long)flux_wr_first));
	}
	*total_flux *= chan_num;
	memory_drv_debug("calculate total_flux success. (total_flux=%lu)\n", *total_flux);
}

unsigned int g_pre_rate = 0;
STATIC int calculate_ddr_bw_by_flux_info(unsigned long *total_flux, ddr_flux_info_t *ddr_flux_info,
	unsigned long *time_interval, unsigned long *ddr_bandwidth)
{
	int i;
	for (i = 0; i < DDR_MAX_CHAN_NUM; i++) {
		/* calculate total flux, including rollover case */
		*total_flux += get_flux_diff(ddr_flux_info[SAMPLE_FIRST].flux_rd[i],
			ddr_flux_info[SAMPLE_SECOND].flux_rd[i]);
		*total_flux += get_flux_diff(ddr_flux_info[SAMPLE_FIRST].flux_wr[i],
			ddr_flux_info[SAMPLE_SECOND].flux_wr[i]);
		*time_interval += get_ddr_time_interval(ddr_flux_info[SAMPLE_FIRST].time_stamp[i],
			ddr_flux_info[SAMPLE_SECOND].time_stamp[i]);
	}
	if (g_ddr_hw_info.channel_num == 0) {
		memory_drv_err("Channel_num is invalid. (channel_num=%u)\n", g_ddr_hw_info.channel_num);
		return -EINVAL;
	}
	*time_interval = *time_interval / (unsigned long)g_ddr_hw_info.channel_num;
	*ddr_bandwidth = (unsigned long)g_ddr_hw_info.channel_num * DDRC_BANDWIDTH_PER_CHAN;
	return 0;
}

STATIC int calculate_ddr_bw_util_rate(unsigned int device_id, int chip_type, unsigned int chan_num,
	ddr_flux_info_t * ddr_flux_info, unsigned int * rate)
{
	int ret;
	unsigned long total_flux = 0;
	unsigned long time_interval = 0;
	unsigned long ddr_bandwidth;
	unsigned long actual_ddr_bandwidth = 0;
	if ((chip_type == CHIP_TYPE_ASCEND_V2) || (chip_type == CHIP_TYPE_ASCEND_V51_LITE)) {
		if (chan_num == 0) {
			memory_drv_err("Channel num is invalid. (chan_num=%u)\n", chan_num);
			return -EINVAL;
		}
		/* bs9sx1a calculate total flux, and time interval */
		calculate_total_flux_info(chan_num, ddr_flux_info, &total_flux);
		/* bs9sx1a calculate time interval */
		calculate_time_interval(ddr_flux_info, &time_interval);
	} else {
		ret = calculate_ddr_bw_by_flux_info(&total_flux, ddr_flux_info, &time_interval, &ddr_bandwidth);
		if (ret != 0) {
			memory_drv_err("Failed to calculate_ddr_bw_by_flux_info. (ret=%d; dev_id=%u)\n", ret, device_id);
			return ret;
		}
	}

	ret = update_ddr_max_bandwidth(device_id, &ddr_bandwidth);
	if (ret != 0) {
		memory_drv_err("Failed to update max bandwidth. (ret=%d; dev_id=%u)\n", ret, device_id);
		return ret;
	}

	/* actual bandwidth formulate */
	if (time_interval > 0) {
		actual_ddr_bandwidth = total_flux * DDRC_TIME_STAMP_PER_SECOND / (time_interval * DDRC_FLUX_MB_DIV);
	} else {
		memory_drv_err("Timestamp error. (dev_id=%u), (time_interval=%lu)\n", device_id, time_interval);
		return -ETIME;
	}

	if (ddr_bandwidth != 0) {
		/* ddr bandwidth rate = actual bandwidth/theory bandwidth */
		*rate = (actual_ddr_bandwidth * DDRC_STAT_PERTAGE) / ddr_bandwidth;
	} else {
		memory_drv_err("Check ddr bandwidth error. (ddr_bandwidth=%lu; channel_num=%u)\n", ddr_bandwidth,
			g_ddr_hw_info.channel_num);
		return -EPERM;
	}

	memory_drv_debug("Calc ddr bw util rate. (dev_id=%u; flux=%lu; time=%lu; max_bw=%lu; actual_bw=%lu, rate=%u)\n",
		device_id, total_flux, time_interval, ddr_bandwidth, actual_ddr_bandwidth, *rate);

	if (*rate > DDRC_STAT_PERTAGE) {
		*rate = g_pre_rate;
	} else {
		g_pre_rate = *rate;
	}
	return 0;
}

int memory_ddr_stat_init(unsigned int device_id, int chip_type, unsigned int chan_num, unsigned int chan_bitmap)
{
#ifdef CFG_FEATURE_LP_ENABLE
	if ((chip_type == CHIP_TYPE_ASCEND_V2) || (chip_type == CHIP_TYPE_ASCEND_V51_LITE)) {
		return memory_ddr_stat_init_from_ipc(device_id);
	} else {
		return memory_ddr_stat_init_from_reg(device_id, chan_num, chan_bitmap);
	}
#else
	return memory_ddr_stat_init_from_reg(device_id, chan_num, chan_bitmap);
#endif
}

STATIC int get_ddr_statistic_info(unsigned int device_id, int chip_type, ddr_flux_info_t *flux_info)
{
#ifdef CFG_FEATURE_LP_ENABLE
	if ((chip_type == CHIP_TYPE_ASCEND_V2) || (chip_type == CHIP_TYPE_ASCEND_V51_LITE)) {
		return get_ddr_statistic_info_from_ipc(device_id, flux_info);
	} else {
		return get_ddr_statistic_info_from_reg(device_id, flux_info);
	}
#else
	return get_ddr_statistic_info_from_reg(device_id, flux_info);
#endif
}
/*
 * Get DDR bandwidth utilization rate, used by dsmi module
 */
STATIC int get_ddr_bw_util_rate(unsigned int device_id, int chip_type, unsigned int chan_num, unsigned int *rate)
{
	int ret;
	ddr_flux_info_t ddr_flux_info[DDRC_BW_ITEM_NUM] = {0};
	ddr_flux_info_t *flux_info_pt = NULL;

	if ((rate == NULL) || (device_id >= MAX_CHIP_NUM)) {
		memory_drv_err("Invalid para. (data=%pK; device_id=%u)\n", rate, device_id);
		return -EINVAL;
	}

	mutex_lock(&g_ddr_reg_lock[device_id]);

	flux_info_pt = &ddr_flux_info[SAMPLE_FIRST];
	ret = get_ddr_statistic_info(device_id, chip_type, flux_info_pt);
	if (ret < 0) {
		memory_drv_err("Failed to get ddr stat info for the first time . (ret=%d; dev_id=%u)\n", ret, device_id);
		goto OUT;
	}

	msleep(DDRC_FLUX_STAT_TIME);

	flux_info_pt = &ddr_flux_info[SAMPLE_SECOND];
	ret = get_ddr_statistic_info(device_id, chip_type, flux_info_pt);
	if (ret < 0) {
		memory_drv_err("Failed to get ddr stat info for the seconed time. (ret=%d; dev_id=%u)\n", ret, device_id);
		goto OUT;
	}

	msleep(10U);

	ret = calculate_ddr_bw_util_rate(device_id, chip_type, chan_num, ddr_flux_info, rate);
	if (ret != 0) {
		memory_drv_err("Failed to calculate bandwidth utilization rate. (ret=%d; dev_id=%u)\n", ret, device_id);
		goto OUT;
	}

	memory_drv_debug("Get ddr bw utilization rate success. (dev_id=%u)\n", device_id);

OUT:
	mutex_unlock(&g_ddr_reg_lock[device_id]);
	return ret;
}

int memory_get_ddr_bw_util_rate(u32 dev_id, u32 chan_num, u32 bitmap, u32 *rate)
{
	int ret;
	int chip_type = 0xFF;

	if ((dms_is_devid_valid(dev_id) == false) || (chan_num == 0) || (bitmap == 0) || (chan_num > DDR_MAX_CHAN_NUM)) {
		memory_drv_err("Invalid parameter. (dev_id=%u; chan_num=%u; bitmap=%u)\n", dev_id, chan_num, bitmap);
		return -EINVAL;
	}
	memory_drv_debug("Get user arg success. (dev_id=%u; chan_num=%u; bitmap=0x%x)\n", dev_id, chan_num, bitmap);

#if defined (CFG_SOC_PLATFORM_MDC_V51) || defined (CFG_SOC_PLATFORM_MDC_V51LITE)
	ret = devdrv_manager_get_chip_type(&chip_type);
	if (ret != 0) {
		devdrv_drv_err("get chip_type failed, ret = %d.\n", ret);
		return ret;
	}
#endif
	ret = memory_ddr_stat_init(dev_id, chip_type, chan_num, bitmap);
	if (ret != 0) {
		memory_drv_err("Failed to init ddr stat. (ret=%d)\n", ret);
		return ret;
	}

	msleep(10U);

	ret = get_ddr_bw_util_rate(dev_id, chip_type, chan_num, (unsigned int*)rate);
	if (ret != 0) {
		memory_drv_err("Failed to get bw util rate failed. (ret=%d; dev_id=%u)\n", ret, dev_id);
		return ret;
	}

	return 0;
}

int memory_get_ddr_bandwidth_utilization_rate(void *feature, char *in, unsigned int in_len, char *out,
	unsigned int out_len)
{
	int ret;
	unsigned int dev_id;
	struct memory_info user_arg = { 0 };
	unsigned int rate = 0;
	struct memory_scan_list *item = NULL;

	if ((in == NULL) || (in_len != sizeof(struct memory_info)) || (out == NULL) || (out_len != sizeof(unsigned int))) {
		memory_drv_err("Invalid parameter. (in=%pK; in_len=%u; out=%pK; out_len=%u)\n", in, in_len, out, out_len);
		return -EINVAL;
	}
	ret = memcpy_s(&user_arg, sizeof(struct memory_info), in, in_len);
	if (ret) {
		memory_drv_err("Memcpy_s failed. (ret=%d)\n", ret);
		return ret;
	}

	dev_id = user_arg.dev_id;
	item = find_operate_info_str(MEMORY_BW_UTIL, MEMORY_DDR);
	if (item == NULL) {
		memory_drv_err("Get ecc static handle struct failed. (dev_id=%u)\n", dev_id);
		return -EINVAL;
	}

	if (item->scan_flag == 0) {
		ret = memory_get_ddr_bw_util_rate(dev_id, user_arg.chan_num, user_arg.bitmap, &rate);
		if (ret != 0) {
			memory_drv_err("Get ddr bw util failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
			return ret;
		}
	} else {
		rate = item->value[dev_id].ddr_bw_util;
	}

	ret = memcpy_s(out, out_len, &rate, sizeof(unsigned int));
	if (ret) {
		memory_drv_err("Memcpy_s failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
		return ret;
	}

	memory_drv_debug("Get ddr bw util rate success. (dev_id=%u; rate=%u)\n", dev_id, rate);
	return 0;
}

#ifdef CFG_MEMORY_DDR_INFO_FROM_SHAREMEM
int memory_get_ddr_info_from_sharemem(unsigned long offset, void *buff, unsigned int size)
{
	int i;
	void __iomem *viraddr = NULL;
	unsigned long map_size = sizeof(MEMORY_INFO);

	if (offset > map_size || (offset > (map_size - size))) {
		memory_drv_err("Invalid offset. (offset=%lu; size=%u)\n", offset, size);
		return -EINVAL;
	}

	viraddr = ioremap(MEMORY_SHAREMEM_BASE_ADDR, map_size);
	if (viraddr == NULL) {
		memory_drv_err("Remap memory address space failed.\n");
		return -EFAULT;
	}

	if (offset < MANU_INFO_OFFSET) {
		*(unsigned int *)buff = readl((void __iomem *)(uintptr_t)((long)(uintptr_t)viraddr + offset));
	} else {
		for (i = 0; i < size; i++) {
			*(unsigned char *)buff = readb((void __iomem *)(uintptr_t)((long)(uintptr_t)viraddr + offset + i));
			buff++;
		}
	}

	(void)iounmap(viraddr);
	viraddr = NULL;
	return 0;
}

int memory_get_ddr_frequency(void *feature, char *in, unsigned int in_len, char *out, unsigned int out_len)
{
	int ret;
	unsigned int dev_id;
	unsigned int freq = 0;

	if ((in == NULL) || (in_len != sizeof(unsigned int)) || (out == NULL) || (out_len != sizeof(unsigned int))) {
		memory_drv_err("Invalid parameter. (in=%pK; in_len=%u; out=%pK; out_len=%u)\n", in, in_len, out, out_len);
		return -EINVAL;
	}

	dev_id = *(unsigned int *)in;

	ret = memory_get_ddr_info_from_sharemem(FREQ_OFFSET, &freq, sizeof(unsigned int));
	if (ret != 0) {
		memory_drv_err("Get ddr frequency failed. (dev_id=%u, ret=%d)\n", dev_id, ret);
		return ret;
	}

	*(unsigned int *)out = freq;

	return 0;
}

STATIC int parse_ddr_hardwarw_cmd(unsigned int cmd, unsigned long *offset, unsigned int *out_size)
{
	*out_size = sizeof(unsigned int);
	switch (cmd) {
		case DMS_SUBCMD_MEMORY_TYPE:
			*offset = DRAM_TPYE_OFFSET;
			break;
		case DMS_SUBCMD_MEMORY_CAPACITY:
			*offset = DRAM_SIZE_OFFSET;
			break;
		case DMS_SUBCMD_MEMORY_CHANNEL:
			*offset = CHAN_MASK_OFFSET;
			break;
		case DMS_SUBCMD_MEMORY_RANK_NUM:
			*offset = RANK_NUM_OFFSET;
			break;
		case DMS_SUBCMD_MEMORY_ECC_ENABLE:
			*offset = IECC_EN_OFFSET;
			break;
		case DMS_SUBCMD_MEMORY_MANUFACTURES:
			*offset = MANU_INFO_OFFSET;
			*out_size = sizeof(unsigned char) * DDR_MAX_CHAN_NUM;
			break;
		default:
			memory_drv_err("Not support command. (cmd=%d)", cmd);
			return -EOPNOTSUPP;
	}

	return 0;
}

STATIC int memory_get_ddr_hardware_info(unsigned int dev_id, unsigned int sub_cmd,
	char *in, unsigned int in_len, unsigned int *out_len)
{
	int ret;
	char *buff = NULL;
	unsigned long offset;

	ret = parse_ddr_hardwarw_cmd(sub_cmd, &offset, out_len);
	if (ret != 0) {
		memory_drv_err("Parse command error. (dev_id=%u; ret=%d)", dev_id, ret);
		return ret;
	}

	if (*out_len > in_len) {
		memory_drv_err("Buffer size is too samll. (dev_id=%u; in_size=%d, out_size=%d)",
			dev_id, in_len, *out_len);
		return -EINVAL;
	}

	buff = kzalloc(*out_len, GFP_KERNEL | __GFP_ACCOUNT);
	if (buff == NULL) {
		memory_drv_err("Buffer kzalloc failed.\n");
		return -ENOMEM;
	}

	ret = memory_get_ddr_info_from_sharemem(offset, (void *)buff, *out_len);
	if (ret != 0) {
		memory_drv_err("Get ddr info failed. (dev_id=%u; subcmd=%d; ret=%d)\n", dev_id, sub_cmd, ret);
		goto OUT;
	}

	ret = copy_to_user(in, buff, *out_len);
	if (ret != 0) {
		memory_drv_err("Failed to invoke copy_from_user. (size=%u)\n", *out_len);
		goto OUT;
	}

OUT:
	kfree(buff);
	buff = NULL;

	return ret;
}
#endif

#if defined(CFG_MEMORY_DDR_INFO_FROM_LP) || defined(CFG_MEMORY_DDR_INFO_FROM_SHAREMEM)
int memory_dmsi_get_device_info(void *feature, char *in, unsigned int in_len, char *out, unsigned int out_len)
{
	int ret;
	unsigned int i;
	unsigned int result_len = 0;
	struct dms_get_device_info_in *in_cfg = NULL;
	struct dms_get_device_info_out *cfg_out = NULL;
	struct memory_dsmi_get_device_info fn_get_table[] = {
#ifdef CFG_MEMORY_DDR_INFO_FROM_SHAREMEM
		{DMS_SUBCMD_MEMORY_TYPE,			memory_get_ddr_hardware_info},
		{DMS_SUBCMD_MEMORY_CAPACITY,		memory_get_ddr_hardware_info},
		{DMS_SUBCMD_MEMORY_CHANNEL,		 memory_get_ddr_hardware_info},
		{DMS_SUBCMD_MEMORY_RANK_NUM,		memory_get_ddr_hardware_info},
		{DMS_SUBCMD_MEMORY_ECC_ENABLE,	  memory_get_ddr_hardware_info},
		{DMS_SUBCMD_MEMORY_MANUFACTURES,	memory_get_ddr_hardware_info},
#elif defined(CFG_MEMORY_DDR_INFO_FROM_LP)
		{DMS_SUBCMD_MEMORY_TYPE,			memory_get_ddr_device_info},
		{DMS_SUBCMD_MEMORY_CAPACITY,		memory_get_ddr_device_info},
		{DMS_SUBCMD_MEMORY_CHANNEL,		 memory_get_ddr_device_info},
		{DMS_SUBCMD_MEMORY_RANK_NUM,		memory_get_ddr_device_info},
		{DMS_SUBCMD_MEMORY_ECC_ENABLE,	  memory_get_ddr_device_info},
		{DMS_SUBCMD_MEMORY_MANUFACTURES,	memory_get_ddr_device_info},
#endif
	};

	if ((in == NULL) || (in_len != sizeof(struct dms_get_device_info_in)) || (out == NULL) ||
		(out_len != sizeof(struct dms_get_device_info_out))) {
		memory_drv_err("Invalid parameter. (in=%pK; in_len=%u; out=%pK; out_len=%u)\n", in, in_len, out, out_len);
		return -EINVAL;
	}

	in_cfg = (struct dms_get_device_info_in *)in;
	cfg_out = (struct dms_get_device_info_out *)out;

	for (i = 0; i < (unsigned int)ARRAY_SIZE(fn_get_table); i++) {
		if (fn_get_table[i].sub_cmd == in_cfg->sub_cmd) {
			ret =
				fn_get_table[i].fn_get(in_cfg->dev_id, in_cfg->sub_cmd, in_cfg->buff, in_cfg->buff_size, &result_len);
			if (ret != 0) {
				memory_drv_err("Get device info failed. (ret=%d; dev_id=%u; sub_cmd=%u)\n",
					ret, in_cfg->dev_id, in_cfg->sub_cmd);
				return ret;
			}

			cfg_out->out_size = result_len;
			return 0;
		}
	}

	(void)feature;
	memory_drv_err("Command not support. (sub_cmd=%u)\n", in_cfg->sub_cmd);
	// Operation not supported
	return -EOPNOTSUPP;
}
#endif

#ifdef CFG_MEMORY_DDR_INFO_FROM_REG
STATIC int get_ddr_ecc_info_by_reg(unsigned int *sig_bit_count, unsigned int *dobule_bit_count, unsigned int dev_id,
	unsigned int bitmap)
{
	int ret;
	unsigned int i;
	unsigned int reg_val_1;
	unsigned int reg_val_2;
	for (i = 0; i < DDR_MAX_CHAN_NUM; i++) {
		if ((1 << i) & bitmap) {
			reg_val_1 = 0;
			ret = memory_ddr_reg_op(DDRC_REG_RD, g_map_regs[dev_id][i], DDR_ECC_OFFSET1, &reg_val_1);
			if (ret < 0) {
				memory_drv_err("Failed to read ddr reg. (dev_id=%u; ret=%d; i=%u)\n", dev_id, ret, i);
				return ret;
			}

			reg_val_2 = 0;
			ret = memory_ddr_reg_op(DDRC_REG_RD, g_map_regs[dev_id][i], DDR_ECC_OFFSET2, &reg_val_2);
			if (ret < 0) {
				memory_drv_err("Failed to read ddr reg. (dev_id=%u; ret=%d; i=%u)\n", dev_id, ret, i);
				return ret;
			}

			*sig_bit_count += reg_val_1;
			*dobule_bit_count += reg_val_2;
		}
	}
	return 0;
}

int get_ddr_ecc_statistics_from_reg(struct memory_info *user_arg)
{
	int ret;
	unsigned int bitmap;
	unsigned int chan_num;
	unsigned int ecc_err_single_bit_statis = 0;
	unsigned int ecc_err_dobule_bit_statis = 0;
	unsigned int dev_id;
	struct ddrc_info_t ddr_info = {0};

	if (user_arg == NULL) {
		memory_drv_err("Invalid parameter.(user_arg=%pK)\n", user_arg);
		return -EINVAL;
	}

	dev_id = user_arg->dev_id;
	ret = get_ddr_info_from_dts(&ddr_info);
	if (ret) {
		memory_drv_err("Failed to get ddr info from dts. (dev_id=%u; ret=%d)\n", dev_id, ret);
		return ret;
	}
	chan_num = ddr_info.chan_num;
	bitmap = ddr_info.bitmap;
	if ((dms_is_devid_valid(dev_id) == false) || (chan_num > DDR_MAX_CHAN_NUM) || (bitmap == 0)) {
		memory_drv_err("Invalid parameter.(dev_id=%u; chan_num=%u; bitmap=%u)\n", dev_id, chan_num, bitmap);
		return -EINVAL;
	}
	memory_drv_debug("User arg. (id=%u; chan_num=%u; bitmap=%u; type=%u)\n", dev_id, chan_num, bitmap, user_arg->type);

	ret = get_ddr_ecc_info_by_reg(&ecc_err_single_bit_statis, &ecc_err_dobule_bit_statis, dev_id, bitmap);
	if (ret < 0) {
		memory_drv_err("Failed to get_ddr_ecc_info_by_reg. (dev_id=%u; ret=%d)\n", dev_id, ret);
		return ret;
	}

	user_arg->ecc_sigle_bit = ecc_err_single_bit_statis;
	user_arg->ecc_double_bit = ecc_err_dobule_bit_statis;

	memory_drv_debug("Get ddr ecc statistic form reg success. (dev_id=%u; sigle_bit=%u; double_bit=%u)\n", dev_id,
		user_arg->ecc_sigle_bit, user_arg->ecc_double_bit);
	return 0;
}
#endif
#else
int memory_map_ddr_init(void)
{
	memory_drv_info("Memory map hbm stub init success.\n");
	return 0;
}

void memory_map_ddr_exit(void)
{
	memory_drv_info("Memory map hbm stub exit success.\n");
}

int memory_get_ddr_bandwidth_utilization_rate(void *feature, char *in, unsigned int in_len,
	char *out, unsigned int out_len)
{
	if ((in == NULL) || (in_len != sizeof(struct memory_info)) || (out == NULL) || (out_len != sizeof(unsigned int))) {
		memory_drv_err("Invalid parameter. (in=%pK; in_len=%u; out=%pK; out_len=%u)\n", in, in_len, out, out_len);
		return -EINVAL;
	}

	memory_drv_debug("Memory get ddr bandwidth stub success.\n");
	*(unsigned int *)out = DDR_BANDWIDTH_DEFAULT_RATE;
	return 0;
}

int get_ddr_ecc_statistics_from_reg(struct memory_info *user_arg)
{
	memory_drv_debug("Memory get ecc statistics from register stub success.\n");
	return 0;
}
#endif