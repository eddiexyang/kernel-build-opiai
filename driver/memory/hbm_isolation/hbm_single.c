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
#include "hbm_single.h"
#include <linux/errno.h>
#include <linux/vmalloc.h>
#include "linux/securec.h"
#include <linux/rtc.h>
#include <linux/io.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/crc16.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include "devdrv_manager_comm.h"
#include "hbm_isolation_common.h"
#include "memory_log.h"
#include "drvmem_base_info.h"
#include "memory_fault_init_cfg.h"
#include "flash_ops_wrap.h"

STATIC struct single_ecc_tool_s g_single_tools[MAX_OS_DEVICE_COUNT] = {0};
#ifdef CFG_HBM_FLASH
STATIC struct ecc_config_tool_s *g_ecc_config_tools = NULL;
#else
STATIC struct ecc_config_tool_s *g_ecc_config_tools[MAX_OS_DEVICE_COUNT] = {0};
STATIC const char *G_DEVICE_FLASH_PART_NAME[MAX_OS_DEVICE_COUNT] = { ECC_CONFIG_FLASH_PART0, ECC_CONFIG_FLASH_PART1,
	ECC_CONFIG_FLASH_PART2, ECC_CONFIG_FLASH_PART3 };
#endif

#ifdef CFG_HBM_FLASH
void wait_single_ecc_data_resource(uint32_t dev_id)
{
	mutex_lock(&g_ecc_config_tools->record_lock);	   /* gloabl value mutex lock */
}

void release_single_ecc_data_resource(uint32_t dev_id)
{
	mutex_unlock(&g_ecc_config_tools->record_lock);	 /* gloabl value mutex unlock */
}
#else
void wait_single_ecc_data_resource(uint32_t dev_id)
{
	mutex_lock(&g_ecc_config_tools[dev_id]->record_lock);
}

void release_single_ecc_data_resource(uint32_t dev_id)
{
	mutex_unlock(&g_ecc_config_tools[dev_id]->record_lock);
}
#endif

#ifndef CFG_FEATURE_NO_CECC_ADDR_REC
static inline void hbm_copy_single_ecc_info(struct single_ecc_record_s *original_info,
	const struct single_ecc_record_s *new_info)
{
	/* update original_info */
	original_info->hbmc_id = new_info->hbmc_id;
	original_info->single_bit_count = new_info->single_bit_count;
	original_info->single_bit_low_addr = new_info->single_bit_low_addr;
	original_info->single_bit_high_addr = new_info->single_bit_high_addr;	   /* update single_bit_high_addr */
	original_info->last_appear_time_stamp = new_info->last_appear_time_stamp;   /* update time stamp */
}

static inline void hbm_exchange_single_ecc_info(struct single_ecc_record_s *first_info,
	struct single_ecc_record_s *second_info)
{
	/* exchange first_info and second_info */
	struct single_ecc_record_s tmp = {0};			   /* use temporary variable */
	hbm_copy_single_ecc_info(&tmp, first_info);
	hbm_copy_single_ecc_info(first_info, second_info);
	hbm_copy_single_ecc_info(second_info, &tmp);
}

STATIC int hbm_resort_single_count(struct single_ecc_record_s *single_ecc_infos, int record_count,
	int sort_start_i)
{
	int record_j;

	/* update sort_start_i */
	sort_start_i = sort_start_i < record_count ? sort_start_i : record_count - 1;
	for (record_j = sort_start_i; record_j >= 1; record_j--) {
		if (single_ecc_infos[record_j].single_bit_count > single_ecc_infos[record_j - 1].single_bit_count) {
			hbm_exchange_single_ecc_info(&single_ecc_infos[record_j],
				&single_ecc_infos[record_j - 1]);   /* if latter part is larger than former part exchange them */
		} else {
			break;
		}
	}

	/* return final record index */
	return record_j;
}

static inline void hbm_add_new_single_ecc_count(uint32_t *original_count, const uint32_t change_count)
{
	/* update original_count */
	uint32_t new_count = *original_count + change_count;
	if (new_count < *original_count) {
		*original_count = MAX_UNINT32_VALUE;		/* if nnew_count reversed make it MAX_UNINT32_VALUE */
	} else {
		*original_count = new_count;
	}
}

STATIC void hbm_insert_new_single_ecc(uint32_t dev_id, struct single_ecc_record_s new_single_ecc)
{
	int record_i, record_j;
	struct single_ecc_record_s *single_ecc_infos = NULL;
	int new_insert_i = g_single_tools[dev_id].single_header.record_count;	   /* update to global record_count */
	single_ecc_infos = g_single_tools[dev_id].single_ecc_infos;				 /* update to global single_ecc_infos */
	for (record_i = g_single_tools[dev_id].single_header.record_count - 1; record_i >= 0; record_i--) {
		/* current single bit ECC error has been recorded */
		if (new_single_ecc.hbmc_id == single_ecc_infos[record_i].hbmc_id &&
			new_single_ecc.single_bit_low_addr == single_ecc_infos[record_i].single_bit_low_addr &&
			new_single_ecc.single_bit_high_addr == single_ecc_infos[record_i].single_bit_high_addr) {
			hbm_add_new_single_ecc_count(&single_ecc_infos[record_i].single_bit_count,
				new_single_ecc.single_bit_count);

			/* resort the single ecc count */
			record_j = hbm_resort_single_count(single_ecc_infos,
				g_single_tools[dev_id].single_header.record_count, record_i);
			/* record first new 64 single ecc info to flash */
			if (record_j < FLASH_MAX_SINGLE_ECC_COUNT) {
				g_single_tools[dev_id].is_single_ecc_to_flash = 1;
			}
			new_insert_i = -1;		  /* mark new_insert_i to -1 */
			break;
		} else if (new_single_ecc.single_bit_count > single_ecc_infos[record_i].single_bit_count) {
			new_insert_i = record_i;
		}
	}
	/* insert and sort single ecc info with ecc count */
	if (new_insert_i >= 0 && new_insert_i < OS_MAX_SINGLE_ECC_COUNT) {
		for (record_i = g_single_tools[dev_id].single_header.record_count - 1; record_i >= new_insert_i; record_i--) {
			if (record_i == OS_MAX_SINGLE_ECC_COUNT - 1) {
				continue;
			}
			/* if record count not equal to 128 then record it */
			hbm_copy_single_ecc_info(&single_ecc_infos[record_i + 1], &single_ecc_infos[record_i]);
		}

		hbm_copy_single_ecc_info(&single_ecc_infos[new_insert_i], &new_single_ecc);
		/* update global record_count */
		g_single_tools[dev_id].single_header.record_count = g_single_tools[dev_id].single_header.record_count + 1 >
			OS_MAX_SINGLE_ECC_COUNT ? OS_MAX_SINGLE_ECC_COUNT : g_single_tools[dev_id].single_header.record_count + 1;
		if (new_insert_i < FLASH_MAX_SINGLE_ECC_COUNT) {
			g_single_tools[dev_id].is_single_ecc_to_flash = 1;
		}
	}
}

STATIC int hbm_get_single_hbmc_info(uint32_t dev_id, uint32_t hbm_stack_i, uint32_t hbm_pc_i,
	uint32_t pre_corr_cnt[MAX_HBM_CONTROL_COUNT])
{
	int ret;
	uint64_t base;
	uint32_t hbmc_id, corr_change_cnt;
	uint32_t corr_cnt = 0;
	struct single_ecc_record_s new_single_ecc = {0};
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
	struct timespec64 stamp;
#else
	struct timeval stamp;
#endif
	/* get base address */
	ret = get_hbm_stack_base(dev_id, hbm_stack_i, &base);
	if (ret != 0) {
		memory_drv_err("get stack base addr fail.ret=%d\n", ret);
		return ret;
	}
	base += HBM_HBMC_BASE_ADDR + HBM_HBMC_PC_OFFSET * hbm_pc_i + HBM_HBMC_RASC_ADDR;	/* update base addresss */
	/* read single bit error count register to get current error count */
	ret = read_reg32(base + RASC_HIS_CORR_CNT, &corr_cnt);
	if (ret) {
		memory_drv_err("read dev-%u hbm_stack-%u hbm_pc-%u corrected ecc count failed, ret: %d.\n",
			dev_id, hbm_stack_i, hbm_pc_i, ret);
		return ret;
	}

	hbmc_id = (hbm_stack_i * MAX_HBM_PC_PER_STACK + hbm_pc_i) % MAX_HBM_CONTROL_COUNT;	  /* overflow prevention */
	corr_change_cnt = corr_cnt - pre_corr_cnt[hbmc_id];							 /* update current error count */
	new_single_ecc.hbmc_id = hbmc_id;
	new_single_ecc.single_bit_count = corr_change_cnt;
	/* occurs new single bit ECC error */
	if (corr_change_cnt > 0) {
		/* read single bit low address register to get current low address */
		ret = read_reg32(base + RASC_HIS_CORR_ADDR_L_ADDR, &new_single_ecc.single_bit_low_addr);
		if (ret) {
			memory_drv_err("read dev-%u hbm_stack-%u hbm_pc-%u corrected ecc low addr failed, ret: %d.\n",
				dev_id, hbm_stack_i, hbm_pc_i, ret);
			return ret;
		}
		/* read single bit high address register to get current high address */
		ret = read_reg32(base + RASC_HIS_CORR_ADDR_H_ADDR, &new_single_ecc.single_bit_high_addr);
		if (ret) {
			memory_drv_err("read dev-%u hbm_stack-%u hbm_pc-%u corrected ecc high addr failed, ret: %d.\n",
				dev_id, hbm_stack_i, hbm_pc_i, ret);
			return ret;
		}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
		ktime_get_real_ts64(&stamp);		/* get time stamp */
#else
		do_gettimeofday(&stamp);			/* get time stamp */
#endif
		new_single_ecc.last_appear_time_stamp = stamp.tv_sec;

		wait_single_ecc_data_resource(dev_id);			  /* gloabl value mutex lock */
		hbm_insert_new_single_ecc(dev_id, new_single_ecc);
		release_single_ecc_data_resource(dev_id);		   /* gloabl value mutex unlock */
		pre_corr_cnt[hbmc_id] = corr_cnt;
	}
	return ret;
}

STATIC int hbm_get_device_single_hbm_addr(uint32_t dev_id, uint32_t pre_corr_cnt[MAX_HBM_CONTROL_COUNT])
{
	uint32_t hbm_stack_i, hbm_pc_i;
	int ret;
	/* traverse all error */
	for (hbm_stack_i = 0; hbm_stack_i < MAX_HBM_STACK_PER_SOCKET; hbm_stack_i++) {
		for (hbm_pc_i = 0; hbm_pc_i < MAX_HBM_PC_PER_STACK; hbm_pc_i++) {
			ret = hbm_get_single_hbmc_info(dev_id, hbm_stack_i, hbm_pc_i, pre_corr_cnt);	/* update pre_corr_cnt */
			if (ret) {
				memory_drv_err("Read corrected ecc info failed. (dev=%u; hbm_stack=%u; hbm_pc=%u ret=%d)\n",
					dev_id, hbm_stack_i, hbm_pc_i, ret);		/* debug info */
				return ret;
			}
		}
	}
	return ret;
}

void hbm_process_single_count(void)
{
	int ret;
	uint32_t dev_id, dev_num;
	static uint32_t pre_corr_cnt[MAX_OS_DEVICE_COUNT][MAX_HBM_CONTROL_COUNT] = {0};
	/* get current device num */
	ret = devdrv_get_devnum(&dev_num);
	if (ret != 0) {
		memory_drv_err("Get dev_num failed. (ret=%d)\n", ret);
		return;
	}

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		/* do not get hbm corrected info again if failed fo 3 consecutive times */
		if (g_single_tools[dev_id].process_failed_count >= MAX_FAILED_COUNT) {
			continue;
		}
		/* update pre_corr_cnt */
		ret = hbm_get_device_single_hbm_addr(dev_id, pre_corr_cnt[dev_id]);
		if (ret) {
			g_single_tools[dev_id].process_failed_count++;	  /* update process_failed_count */
			memory_drv_err("dev-%u get hbm corrected ecc info failed for %u times, ret: %d.\n",
				dev_id, g_single_tools[dev_id].process_failed_count, ret);
		} else {
			g_single_tools[dev_id].process_failed_count = 0;	/* mark process_failed_count to 0 */
		}
	}
}
#endif

#ifdef CFG_HBM_FLASH
static int hbm_single_write_ecc_config_data(uint32_t dev_id)
{
	struct single_ecc_header_s *flash_single_header = NULL;

	/* update crc_check */
	flash_single_header = (struct single_ecc_header_s *)(g_ecc_config_tools->ecc_config_data +
		dev_id * ECC_FLASH_DIE_SIZE + SINGLE_ECC_RECORD_OFFSET);
	/* calculate devcie single die crc_check */
	flash_single_header->crc_check = crc16(ECC_CONFIG_CRC16,
		(uint8_t *)(g_ecc_config_tools->ecc_config_data + dev_id * ECC_FLASH_DIE_SIZE + SINGLE_ECC_RECORD_OFFSET +
		sizeof(struct single_ecc_header_s)),
		flash_single_header->record_count * sizeof(struct single_ecc_record_s));

	return hbm_flash_write(NULL, ECC_CONFIG_START_OFFSET, FLASH_ERASE_BLOCK_SIZE, g_ecc_config_tools->ecc_config_data);
}
#endif

void hbm_write_single_ecc_data_to_flash(void)
{
	int ret;
	uint32_t dev_id, dev_num;
	uint32_t write_count;
	struct single_ecc_header_s *flash_single_ecc_header = NULL;
	/* get current device num */
	ret = devdrv_get_devnum(&dev_num);
	if (ret != 0) {
		memory_drv_err("Get dev_num failed. (ret=%d)\n", ret);
		return;
	}

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		if (g_single_tools[dev_id].is_single_ecc_to_flash == 0 ||
			g_single_tools[dev_id].write_failed_count >= MAX_FAILED_COUNT) {
			continue;
		}

		wait_single_ecc_data_resource(dev_id);	  /* gloabl value mutex lock */
		/* update write_count */
		write_count = g_single_tools[dev_id].single_header.record_count > FLASH_MAX_SINGLE_ECC_COUNT ?
			FLASH_MAX_SINGLE_ECC_COUNT : g_single_tools[dev_id].single_header.record_count;
#ifdef CFG_HBM_FLASH
		flash_single_ecc_header = (struct single_ecc_header_s *)(g_ecc_config_tools->ecc_config_data +
			dev_id * ECC_FLASH_DIE_SIZE + SINGLE_ECC_RECORD_OFFSET);
		flash_single_ecc_header->record_count = write_count;
		/* copy current single_header to global value */
		ret = memcpy_s(g_ecc_config_tools->ecc_config_data + SINGLE_ECC_RECORD_OFFSET + dev_id * ECC_FLASH_DIE_SIZE,
			sizeof(struct single_ecc_header_s), (uint8_t *)&g_single_tools[dev_id].single_header,
			sizeof(struct single_ecc_header_s));
		if (ret != OK) {
			g_single_tools[dev_id].write_failed_count++;
			release_single_ecc_data_resource(dev_id);	   /* gloabl value mutex unlock */
			memory_drv_err("Memcpy single ecc header to config data failed. (device=%u; ret=%d)\n", dev_id, ret);
			continue;
		}
		/* copy current single_ecc_infos to global value */
		ret = memcpy_s(g_ecc_config_tools->ecc_config_data + SINGLE_ECC_RECORD_OFFSET + dev_id * ECC_FLASH_DIE_SIZE +
			sizeof(struct single_ecc_header_s), sizeof(struct single_ecc_record_s) * FLASH_MAX_SINGLE_ECC_COUNT,
			(uint8_t *)g_single_tools[dev_id].single_ecc_infos, sizeof(struct single_ecc_record_s) * write_count);
#else
		flash_single_ecc_header = (struct single_ecc_header_s *)(g_ecc_config_tools[dev_id]->ecc_config_data +
			SINGLE_ECC_RECORD_OFFSET);
		flash_single_ecc_header->record_count = write_count;

		ret = memcpy_s(g_ecc_config_tools[dev_id]->ecc_config_data + SINGLE_ECC_RECORD_OFFSET +
			sizeof(struct single_ecc_header_s), sizeof(struct single_ecc_record_s) * FLASH_MAX_SINGLE_ECC_COUNT,
			(uint8_t *)g_single_tools[dev_id].single_ecc_infos, sizeof(struct single_ecc_record_s) * write_count);
#endif
		if (ret) {
			g_single_tools[dev_id].write_failed_count++;
			release_single_ecc_data_resource(dev_id);	   /* gloabl value mutex unlock */
			memory_drv_err("memcpy device-%u single ecc infos to config data failed, ret: %d.\n", dev_id, ret);
			continue;
		}
		/* update global value to hbm flash */
		memory_drv_event("Write single ecc data start. (dev=%u)\n", dev_id);
#ifdef CFG_HBM_FLASH
		ret = hbm_single_write_ecc_config_data(dev_id);
#else
		if (dev_id == 0) {
			ret = hbm_write_ecc_config_data();
		} else {
			ret = hbm_flash_write(G_DEVICE_FLASH_PART_NAME[dev_id], ECC_CONFIG_START_OFFSET,
				FLASH_ERASE_BLOCK_SIZE, g_ecc_config_tools[dev_id]->ecc_config_data);
		}
#endif
		if (ret) {
			g_single_tools[dev_id].write_failed_count++;	/* update write_failed_count */
			release_single_ecc_data_resource(dev_id);	   /* gloabl value mutex unlock */
			memory_drv_err("Memcpy single ecc infos to config data failed. (ret=%d)\n", ret);
			continue;
		}
		g_single_tools[dev_id].is_single_ecc_to_flash = 0;	  /* mark is_single_ecc_to_flash */
		g_single_tools[dev_id].write_failed_count = 0;		  /* mark write_failed_count */
		release_single_ecc_data_resource(dev_id);	   /* gloabl value mutex unlock */
	}
}

void hbm_free_single_ecc_info(void)
{
	int ret;
	uint32_t dev_id, dev_num;
	/* get current device num */
	ret = devdrv_get_devnum(&dev_num);
	if (ret != 0) {
		memory_drv_err("Get dev_num failed. (ret=%d)\n", ret);
		return;
	}
	/* free all global value memory */
	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		if (g_single_tools[dev_id].single_ecc_infos != NULL) {	  /* judge pointer whether NULL */
			hbm_vfree((void **)&g_single_tools[dev_id].single_ecc_infos);
		}
#ifndef CFG_HBM_FLASH
		if (dev_id != 0 && g_ecc_config_tools[dev_id] != NULL) {
			if (g_ecc_config_tools[dev_id]->ecc_config_data != NULL) {
				hbm_vfree((void **)&g_ecc_config_tools[dev_id]->ecc_config_data);
			}
			mutex_destroy(&g_ecc_config_tools[dev_id]->record_lock);
			hbm_kfree((void **)&g_ecc_config_tools[dev_id]);
		}
#endif
	}
}

#ifdef CFG_HBM_FLASH
STATIC bool hbm_single_is_ecc_config_data_crc_right(uint32_t dev_id, struct single_ecc_header_s *flash_single_header)
{
	uint16_t calculate_crc;
	struct single_ecc_header_s *single_ecc_header = (struct single_ecc_header_s *)
		(g_ecc_config_tools->ecc_config_data + dev_id * ECC_FLASH_DIE_SIZE + SINGLE_ECC_RECORD_OFFSET +
		sizeof(struct single_ecc_header_s));		/* update single_ecc_header */
	/* multi die need add offset */
	calculate_crc = crc16(ECC_CONFIG_CRC16, (uint8_t *)(single_ecc_header),
		flash_single_header->record_count * sizeof(struct single_ecc_record_s));
	if (flash_single_header->crc_check == calculate_crc) {  /* judge calculate_crc whether equal recored crc_check */
		return true;
	}
	return false;
}
#endif

STATIC int hbm_init_ecc_mem_data(uint32_t dev_id)
{
	int ret = 0;
#ifdef CFG_HBM_FLASH
	struct single_ecc_header_s *flash_single_header = (struct single_ecc_header_s *)
		(g_ecc_config_tools->ecc_config_data + dev_id * ECC_FLASH_DIE_SIZE + SINGLE_ECC_RECORD_OFFSET);
	/* init single ecc flash data header for the first time */
	if (flash_single_header->signature != ECC_HEADER_SIGNATURE ||
		!hbm_single_is_ecc_config_data_crc_right(dev_id, flash_single_header)) {
		flash_single_header->signature = ECC_HEADER_SIGNATURE;	  /* init signature */
		flash_single_header->version = SINGLE_ECC_HEADER_VERSION;   /* init version */
		flash_single_header->record_count = 0;					  /* init record_count */
		flash_single_header->crc_check = ECC_CONFIG_CRC16;	/* init crc_check */
#else
	struct single_ecc_header_s *flash_single_header = (struct single_ecc_header_s *)
		(g_ecc_config_tools[dev_id]->ecc_config_data + SINGLE_ECC_RECORD_OFFSET);
	/* init single ecc flash data header for the first time */
	if (flash_single_header->signature != ECC_HEADER_SIGNATURE) {
		flash_single_header->signature = ECC_HEADER_SIGNATURE;
		flash_single_header->version = SINGLE_ECC_HEADER_VERSION;
		flash_single_header->record_count = 0;
#endif
	} else {
		flash_single_header->record_count = flash_single_header->record_count > FLASH_MAX_SINGLE_ECC_COUNT ?
			FLASH_MAX_SINGLE_ECC_COUNT : flash_single_header->record_count;			 /* update record_count */

		/* read single ecc info to memory */
#ifdef CFG_HBM_FLASH
		ret = memcpy_s((uint8_t *)g_single_tools[dev_id].single_ecc_infos, sizeof(struct single_ecc_record_s) *
			OS_MAX_SINGLE_ECC_COUNT, g_ecc_config_tools->ecc_config_data + SINGLE_ECC_RECORD_OFFSET +
			dev_id * ECC_FLASH_DIE_SIZE + sizeof(struct single_ecc_header_s), sizeof(struct single_ecc_record_s) *
			flash_single_header->record_count);
#else
		ret = memcpy_s((uint8_t *)g_single_tools[dev_id].single_ecc_infos, sizeof(struct single_ecc_record_s) *
			OS_MAX_SINGLE_ECC_COUNT, g_ecc_config_tools[dev_id]->ecc_config_data + SINGLE_ECC_RECORD_OFFSET +
			sizeof(struct single_ecc_header_s), sizeof(struct single_ecc_record_s) *
			flash_single_header->record_count);
#endif
		if (ret) {
			memory_drv_err("memcpy device-%u single ecc infos failed, ret: %d.\n", dev_id, ret);
			goto FREE_SINGLE_ECC_MEM_INFO;
		}
	}
	/* update global value memory */
	ret = memcpy_s((uint8_t *)&(g_single_tools[dev_id].single_header), sizeof(struct single_ecc_header_s),
		(uint8_t *)flash_single_header, sizeof(struct single_ecc_header_s));
	if (ret) {
		memory_drv_err("memcpy device-%u single ecc header failed, ret: %d.\n", dev_id, ret);
		goto FREE_SINGLE_ECC_MEM_INFO;
	}
	return OK;

FREE_SINGLE_ECC_MEM_INFO:
	hbm_vfree((void **)&g_single_tools[dev_id].single_ecc_infos);	   /* free memroy */
	return ret;
}

#ifdef CFG_HBM_FLASH
int hbm_clear_single_ecc_info(uint32_t dev_id)
{
	int ret;

	memory_drv_event("Ecc_config erase flash data. (dev_id=%u)\n", dev_id);
	wait_single_ecc_data_resource(dev_id);	  /* gloabl value mutex lock */
	/* device config_data is cleared  */
	ret = hbm_init_ecc_mem_data(dev_id);
	if (ret != OK) {
		release_single_ecc_data_resource(dev_id);	   /* gloabl value mutex unlock */
		memory_drv_err("init dev_id-%u single ecc memory data failed, ret: %d!\n", dev_id, ret);
		return ret;
	}
	release_single_ecc_data_resource(dev_id);	   /* gloabl value mutex unlock */
	return ret;
}
#else
STATIC int hbm_init_global_ecc_data(uint32_t dev_id)
{
	int ret;

	g_ecc_config_tools[dev_id] = (struct ecc_config_tool_s *)kzalloc(sizeof(struct ecc_config_tool_s),
		GFP_KERNEL | __GFP_ACCOUNT);
	if (g_ecc_config_tools[dev_id] == NULL) {
		memory_drv_err("device_id-%u global ecc config tools kzalloc failed!\n", dev_id);
		ret = -ENOMEM;
		return ret;
	}

	mutex_init(&g_ecc_config_tools[dev_id]->record_lock);
	g_ecc_config_tools[dev_id]->ecc_config_data = (uint8_t *)vmalloc(FLASH_ERASE_BLOCK_SIZE);
	if (g_ecc_config_tools[dev_id]->ecc_config_data == NULL) {
		memory_drv_err("alloc dev_id-%u ecc config data failed.\n", dev_id);
		ret = -ENOMEM;
		goto FREE_ECC_CONFIG_TOOLS;
	}
	ret = hbm_flash_read(G_DEVICE_FLASH_PART_NAME[dev_id], ECC_CONFIG_START_OFFSET,
		FLASH_ERASE_BLOCK_SIZE, g_ecc_config_tools[dev_id]->ecc_config_data);
	if (ret) {
		memory_drv_err("read dev_id-%u ecc flash data failed, ret: %d!\n", dev_id, ret);
		goto FREE_ECC_CONFIG_DATA;
	}

	ret = hbm_init_ecc_mem_data(dev_id);
	if (ret) {
		memory_drv_err("init dev_id-%u single ecc memory data failed, ret: %d!\n", dev_id, ret);
		goto FREE_ECC_CONFIG_DATA;
	}
	return ret;

FREE_ECC_CONFIG_DATA:
	hbm_vfree((void **)&g_ecc_config_tools[dev_id]->ecc_config_data);
FREE_ECC_CONFIG_TOOLS:
	mutex_destroy(&g_ecc_config_tools[dev_id]->record_lock);
	hbm_kfree((void **)&g_ecc_config_tools[dev_id]);
	return ret;
}

int hbm_clear_single_ecc_info(void)
{
	uint32_t dev_id, dev_num;
	int ret = 0;

	ret = devdrv_get_devnum(&dev_num);
	if (ret != 0) {
		memory_drv_err("Get dev_num failed. (ret=%d)\n", ret);
		return ret;
	}

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		wait_single_ecc_data_resource(dev_id);
		memory_drv_event("ecc_config erase dev_id-%u flash data.\n", dev_id);
		/* dev0 config_data is cleared by ecc_config.c */
		if (dev_id != 0) {
			ret = hbm_flash_erase(G_DEVICE_FLASH_PART_NAME[dev_id], ECC_CONFIG_START_OFFSET, FLASH_ERASE_BLOCK_SIZE);
			if (ret != OK) {
				release_single_ecc_data_resource(dev_id);
				memory_drv_err("dev_id-%u erase flash failed, ret: %d!\n", dev_id, ret);
				return ret;
			}
			/* set ecc_config data to 1 according to the initial value of flash */
			ret = memset_s((uint8_t *)g_ecc_config_tools[dev_id]->ecc_config_data, FLASH_ERASE_BLOCK_SIZE, 1,
						   FLASH_ERASE_BLOCK_SIZE);
			if (ret != OK) {
				release_single_ecc_data_resource(dev_id);
				memory_drv_err("memset_s ecc header info failed, ret: %d!\n", ret);
				return ret;
			}
		}

		ret = hbm_init_ecc_mem_data(dev_id);
		if (ret) {
			release_single_ecc_data_resource(dev_id);
			memory_drv_err("init dev_id-%u single ecc memory data failed, ret: %d!\n", dev_id, ret);
			return ret;
		}
		release_single_ecc_data_resource(dev_id);
	}
	return ret;
}
#endif

int hbm_init_single_ecc_info(void)
{
	uint32_t dev_id, dev_num;
	int ret;
	/* get current device num */
	ret = devdrv_get_devnum(&dev_num);
	if (ret != 0) {
		memory_drv_err("Get dev_num failed. (ret=%d)\n", ret);
		return ret;
	}
#ifdef CFG_HBM_FLASH
	g_ecc_config_tools = hbm_get_ecc_config_tool();	 /* update multi ecc info pointer to single ecc info pointer */
#endif
	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		/* record first 128 single ecc addr */
		g_single_tools[dev_id].single_ecc_infos = (struct single_ecc_record_s *)
			vmalloc(sizeof(struct single_ecc_record_s) * OS_MAX_SINGLE_ECC_COUNT);
		if (g_single_tools[dev_id].single_ecc_infos == NULL) {
			memory_drv_err("alloc dev_id-%u single ecc mem info data failed.\n", dev_id);
			ret = -ENOMEM;
			goto FREE_SINGLE_ECC_INFO;
		}
		/* init memory */
#ifdef CFG_HBM_FLASH
		ret = memset_s(g_single_tools[dev_id].single_ecc_infos,
			sizeof(struct single_ecc_record_s) * OS_MAX_SINGLE_ECC_COUNT, 0,
			sizeof(struct single_ecc_record_s) * OS_MAX_SINGLE_ECC_COUNT);
		if (ret != OK) {
			memory_drv_err("Memset_s failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
			goto FREE_SINGLE_ECC_INFO;
		}
		/* init memory data */
		ret = hbm_init_ecc_mem_data(dev_id);
		if (ret != OK) {
			memory_drv_err("Init global ecc data failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
			goto FREE_SINGLE_ECC_INFO;
		}
#else
		/*
		* single bit ECC info is in flash of each chip.
		* Unlike multi bit ECC info, multi bit ECC info is only saved in device0 in SMP mode.
		*/
		if (dev_id == 0) {
			g_ecc_config_tools[dev_id] = hbm_get_ecc_config_tool();
			ret = hbm_init_ecc_mem_data(dev_id);
			if (ret) {
				memory_drv_err("init dev_id-%u single ecc memory data failed, ret: %d!\n", dev_id, ret);
				goto FREE_SINGLE_ECC_INFO;
			}
		} else {
			ret = hbm_init_global_ecc_data(dev_id);
			if (ret) {
				memory_drv_err("dev_id-%u inti global ecc data failed, ret: %d.\n", dev_id, ret);
				goto FREE_SINGLE_ECC_INFO;
			}
		}
#endif
	}

	return OK;
FREE_SINGLE_ECC_INFO:
	hbm_free_single_ecc_info();	 /* free memory */
	return ret;
}

uint32_t hbm_get_single_ecc_addr_count(uint32_t dev_id)
{
	struct single_ecc_header_s *single_header = NULL;
	uint32_t ecc_addr_count;
#ifdef CFG_HBM_FLASH
	single_header = (struct single_ecc_header_s *)(g_ecc_config_tools->ecc_config_data +
		dev_id * ECC_FLASH_DIE_SIZE + SINGLE_ECC_RECORD_OFFSET);	/* update to global value */
#else
	single_header = (struct single_ecc_header_s *)(g_ecc_config_tools[dev_id]->ecc_config_data +
		SINGLE_ECC_RECORD_OFFSET);
#endif
	ecc_addr_count = single_header->record_count;	   /* update ecc_addr_count */

	return ecc_addr_count;
}

int hbm_get_one_single_ecc_info(uint32_t dev_id, uint32_t data_index,
	struct single_ecc_record_s *single_ecc_info)
{
	int ret;
	uint32_t ecc_addr_count;
	struct single_ecc_record_s *current_single_ecc_info = NULL;

	wait_single_ecc_data_resource(dev_id);	  /* gloabl value mutex lock */
	ecc_addr_count = hbm_get_single_ecc_addr_count(dev_id);	 /* update ecc_addr_count */
	if (data_index >= ecc_addr_count) {
		release_single_ecc_data_resource(dev_id);	   /* gloabl value mutex unlock */
		memory_drv_err("Get invalid single ecc info. (dev_id=%u; data_index=%u; record_count=%u)\n", dev_id,
			data_index, ecc_addr_count);
		return -EINVAL;
	}
	/* update current_single_ecc_info */
#ifdef CFG_HBM_FLASH
	current_single_ecc_info = (struct single_ecc_record_s *)(g_ecc_config_tools->ecc_config_data +
		dev_id * ECC_FLASH_DIE_SIZE + SINGLE_ECC_RECORD_OFFSET + sizeof(struct single_ecc_header_s));
#else
	current_single_ecc_info = (struct single_ecc_record_s *)(g_ecc_config_tools[dev_id]->ecc_config_data +
		SINGLE_ECC_RECORD_OFFSET + sizeof(struct single_ecc_header_s));
#endif
	current_single_ecc_info += data_index;
	/* read global memory */
	ret  = memcpy_s(single_ecc_info, sizeof(struct single_ecc_record_s),
		(uint8_t *)current_single_ecc_info, sizeof(struct single_ecc_record_s));
	release_single_ecc_data_resource(dev_id);	   /* gloabl value mutex unlock */
	if (ret) {
		memory_drv_err("Memcpy single ecc data failed. (dev_id=%u, index=%u, ret=%d)\n", dev_id, data_index, ret);
	}
	return ret;
}
