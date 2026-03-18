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
#include "hbm_multi.h"
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/property.h>
#include <linux/notifier.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/vmalloc.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/kfifo.h>
#include <linux/jiffies.h>
#include "linux/securec.h"
#include <linux/rtc.h>
#include <acpi/ghes.h>
#include <linux/kallsyms.h>
#include <linux/pid.h>
#include <linux/cper.h>
#include <linux/crc16.h>
#include <linux/semaphore.h>
#include <linux/pagemap.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include "dms_define.h"
#include "dms_template.h"
#include "dms_cmd_def.h"
#include "urd_acc_ctrl.h"
#include "drv_ipc.h"
#include "devdrv_ipc.h"
#include "devdrv_manager_comm.h"
#include "devdrv_dfm.h"
#include "hbm_isolation_common.h"
#include "hbm_single.h"
#include "memory_fault_init_cfg.h"
#include "icm_interface.h"
#include "memory_log.h"
#include "memory_adapter.h"
#include "flash_ops_wrap.h"

struct ecc_config_tool_s *g_ecc_config_tool = NULL;

int hbm_get_isolated_page_statistics(void *feature, char *in, uint32_t in_len, char *out, uint32_t out_len);
int hbm_clear_isolated_info(void *feature, char *in, uint32_t in_len, char *out, uint32_t out_len);
int hbm_get_ecc_record_info(void *feature, char *in, uint32_t in_len, char *out, uint32_t out_len);

BEGIN_DMS_MODULE_DECLARATION(DRV_MEMORY_MODULE_BASIC_INFO)
BEGIN_FEATURE_COMMAND()
ADD_FEATURE_COMMAND(DRV_MEMORY_MODULE_BASIC_INFO, DMS_GET_ISOLATED_PAGES_INFO_CMD, ZERO_CMD, NULL, NULL,
	DMS_SUPPORT_ALL, hbm_get_isolated_page_statistics)
ADD_FEATURE_COMMAND(DRV_MEMORY_MODULE_BASIC_INFO, DMS_CLEAR_ISOLATED_INFO_CMD, ZERO_CMD, NULL, CHECK_PROCESS_DMP,
	DMS_SUPPORT_MANAGE_PHY, hbm_clear_isolated_info)
ADD_FEATURE_COMMAND(DRV_MEMORY_MODULE_BASIC_INFO, DMS_GET_ECC_RECORD_CMD, ZERO_CMD, NULL, NULL,
	DMS_SUPPORT_ALL, hbm_get_ecc_record_info)
END_FEATURE_COMMAND()
END_MODULE_DECLARATION()

#ifdef CFG_HBM_FLASH
STATIC int hbm_read_sec_ecc_config_data(void)
{
	int ret;
	ret = hbm_flash_read(NULL, ECC_CONFIG_START_OFFSET, FLASH_ERASE_BLOCK_SIZE, g_ecc_config_tool->ecc_config_data);
	if (ret != OK) {
		memory_drv_err("hbm_read_sec_ecc_config_data failed. (ret=%d)\n", ret);
		return -EINVAL;
	}

	return OK;
}

int hbm_write_ecc_config_data(uint32_t dev_id)
{
	int ret;
	uint32_t dev_num = 0;
	struct ecc_config_header_s *ecc_header_info = NULL;

	/* get device number */
	ret = devdrv_get_devnum(&dev_num);
	if (ret != OK) {
		memory_drv_err("Get dev_num failed. (ret=%d)\n", ret);
		return -EINVAL;
	}
	/* update ecc_header_info */
	ecc_header_info = (struct ecc_config_header_s *)(g_ecc_config_tool->ecc_config_data + dev_id * ECC_FLASH_DIE_SIZE);
	if (ecc_header_info->length > (MAX_RECORD_ECC_ADDR_COUNT * sizeof(struct multi_ecc_err_info_s))) {
		memory_drv_err("The length exceeds maximum. length=0x%x\n", ecc_header_info->length);
		return -EINVAL;
	}

	ecc_header_info->crc_check = crc16(ECC_CONFIG_CRC16,
		(uint8_t *)(g_ecc_config_tool->ecc_config_data + dev_id * ECC_FLASH_DIE_SIZE +
		sizeof(struct ecc_config_header_s)), ecc_header_info->length);  /* update crc_check */
	ret = hbm_flash_write(NULL, ECC_CONFIG_START_OFFSET, FLASH_ERASE_BLOCK_SIZE, g_ecc_config_tool->ecc_config_data);
	if (ret != OK) {
		memory_drv_err("hbm_write_sec_ecc_config_data failed. (ret=%d)\n", ret);
		return -EINVAL;
	}

	return ret;
}

#else
static inline int read_flash_ecc_record_data(void)
{
	return hbm_flash_read((const uint8_t *)ECC_CONFIG_FLASH_PART, ECC_CONFIG_START_OFFSET,
		FLASH_ERASE_BLOCK_SIZE, g_ecc_config_tool->ecc_config_data);
}

int hbm_write_ecc_config_data(void)
{
	struct ecc_config_header_s *ecc_header_info = (struct ecc_config_header_s *)g_ecc_config_tool->ecc_config_data;
	ecc_header_info->crc_check = crc16(ECC_CONFIG_CRC16, (uint8_t *)(g_ecc_config_tool->ecc_config_data +
		sizeof(struct ecc_config_header_s)), FLASH_ERASE_BLOCK_SIZE - sizeof(struct ecc_config_header_s));

	return hbm_flash_write((const uint8_t *)ECC_CONFIG_FLASH_PART, ECC_CONFIG_START_OFFSET,
		FLASH_ERASE_BLOCK_SIZE, g_ecc_config_tool->ecc_config_data);
}
#endif

struct ecc_config_tool_s *hbm_get_ecc_config_tool(void)
{
	return g_ecc_config_tool;	   /* get global ecc config data */
}

#ifdef CFG_HBM_FLASH
STATIC bool hbm_is_ecc_config_data_crc_right(uint8_t *ecc_config_data, uint8_t dev_id)
{
	uint16_t calculate_crc;
	struct ecc_config_header_s *ecc_header_info = NULL;

	/* multi die need add offset */
	ecc_header_info = (struct ecc_config_header_s *)(ecc_config_data + dev_id * ECC_FLASH_DIE_SIZE);
	calculate_crc = crc16(ECC_CONFIG_CRC16,
		(uint8_t *)(ecc_config_data + sizeof(struct ecc_config_header_s) + dev_id * ECC_FLASH_DIE_SIZE),
		ecc_header_info->length);   /* calculate length is data length */
	if (ecc_header_info->crc_check == calculate_crc) {
		return true;
	}
	return false;
}

STATIC int hbm_check_reset_ecc_header(uint8_t *ecc_config_data, uint32_t dev_id)
{
	int ret;
	struct ecc_config_header_s *ecc_header_info = (struct ecc_config_header_s *)
		(ecc_config_data + dev_id * ECC_FLASH_DIE_SIZE);
	uint32_t max_recorded_ecc_length = MAX_RECORD_ECC_ADDR_COUNT * sizeof(struct multi_ecc_err_info_s);

	/* judeg signature and crc */
	if (ecc_header_info->signature != ECC_HEADER_SIGNATURE ||
		!hbm_is_ecc_config_data_crc_right(ecc_config_data, dev_id)) {
		/* init ecc_config_header_s if inserted for the first time */
		ret = memset_s(ecc_config_data + dev_id * ECC_FLASH_DIE_SIZE, HBM_MULTI_INFO_SIZE, 0, HBM_MULTI_INFO_SIZE);
		if (ret != OK) {
			memory_drv_err("Memset_s ecc data info failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
			return ret;
		}
		ret = memset_s(ecc_config_data + HBM_MULTI_INFO_SIZE + HBM_SINGLE_INFO_SIZE + dev_id * ECC_FLASH_DIE_SIZE,
			HBM_STATISTICS_SIZE, 0, HBM_STATISTICS_SIZE);	   /* init statistics data */
		if (ret != OK) {
			memory_drv_err("Memset_s statistics data info failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
			return ret;
		}
		ecc_header_info->signature = ECC_HEADER_SIGNATURE;	  /* after memset need init signature */
		ecc_header_info->version = ECC_HEADER_VERSION_V2;	   /* after memset need init version */
		ecc_header_info->crc_check = ECC_CONFIG_CRC16;		  /* after memset need init crc_check */
	} else {
		ecc_header_info->length = ecc_header_info->length > max_recorded_ecc_length ? max_recorded_ecc_length :
			ecc_header_info->length;					/* update ecc_header_info length */
	}
	ecc_header_info->statistics_info_offset = STATISTICS_INFO_FLASH_OFFSET;
	return ret;
}
#else
STATIC bool hbm_is_ecc_config_data_crc_right(uint8_t *ecc_config_data, uint32_t ecc_data_length)
{
	uint16_t calculate_crc;
	struct ecc_config_header_s *ecc_header_info = (struct ecc_config_header_s *)ecc_config_data;

	calculate_crc = crc16(ECC_CONFIG_CRC16, (uint8_t *)(ecc_config_data + sizeof(struct ecc_config_header_s)),
		FLASH_ERASE_BLOCK_SIZE - sizeof(struct ecc_config_header_s));
	if (ecc_header_info->crc_check == calculate_crc) {
		return true;
	}
	return false;
}

STATIC int hbm_check_reset_ecc_header(uint8_t *ecc_config_data, uint32_t ecc_data_length)
{
	int ret = OK;
	struct ecc_config_header_s *ecc_header_info = (struct ecc_config_header_s *)ecc_config_data;

	if (ecc_header_info->signature != ECC_HEADER_SIGNATURE ||
		!hbm_is_ecc_config_data_crc_right(ecc_config_data, ecc_data_length)) {
		/* init ecc_config_header_s if inserted for the first time */
		ret = memset_s(ecc_config_data, ecc_data_length, 0, ecc_data_length);
		if (ret != OK) {
			memory_drv_err("memset_s ecc data info failed, ret: %d!\n", ret);
			return ret;
		}
		ecc_header_info->signature = ECC_HEADER_SIGNATURE;
		ecc_header_info->version = ECC_HEADER_VERSION;
	} else {
		uint32_t max_recorded_ecc_length = MAX_RECORD_ECC_ADDR_COUNT * sizeof(struct multi_ecc_err_info_s);
		ecc_header_info->length = ecc_header_info->length > max_recorded_ecc_length ? max_recorded_ecc_length :
			ecc_header_info->length;
	}
	ecc_header_info->statistics_info_offset = STATISTICS_INFO_FLASH_OFFSET;
	return ret;
}
#endif

#ifdef CFG_HBM_FLASH
STATIC int hbm_init_memory_data(uint32_t dev_id)
{
	int ret;
	struct ecc_config_header_s *ecc_header_info = NULL;

	/* check ecc header */
	ret = hbm_check_reset_ecc_header(g_ecc_config_tool->ecc_config_data, dev_id);
	if (ret != OK) {
		memory_drv_err("Reset ECC header info failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
		return ret;
	}
	ecc_header_info = (struct ecc_config_header_s *)(g_ecc_config_tool->ecc_config_data + dev_id * ECC_FLASH_DIE_SIZE);
	/* add debug print info */
	memory_drv_debug("Statistics record data. (signature=0x%x; version=0x%x; length=0x%x; crc_check=0x%x)\n",
					 ecc_header_info->signature, ecc_header_info->version, ecc_header_info->length,
					 ecc_header_info->crc_check);
	/* update g_ecc_config_tool data */
	ret = memcpy_s((uint8_t *)(g_ecc_config_tool->ecc_statistics_info + dev_id),
		sizeof(struct ecc_isolated_statistics_record), g_ecc_config_tool->ecc_config_data +
		STATISTICS_INFO_FLASH_OFFSET + dev_id * ECC_FLASH_DIE_SIZE, sizeof(struct ecc_isolated_statistics_record));
	if (ret != OK) {
		memory_drv_err("Copy ECC statistics info failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
	}
	return ret;
}
#else
STATIC int hbm_init_memory_data(void)
{
	int ret;
	struct ecc_config_header_s *ecc_header_info = NULL;

	ret = read_flash_ecc_record_data();
	if (ret) {
		memory_drv_err("Init read ECC recorded data failed. (ret=%d)\n", ret);
		return ret;
	}

	ret = hbm_check_reset_ecc_header(g_ecc_config_tool->ecc_config_data, FLASH_ERASE_BLOCK_SIZE);
	if (ret) {
		memory_drv_err("Reset ECC header info failed. (ret=%d)\n", ret);
		return ret;
	}

	ecc_header_info = (struct ecc_config_header_s *)g_ecc_config_tool->ecc_config_data;
	memory_drv_debug("signature=0x%x, version=0x%x, length=0x%x, crc_check=0x%x.\n",
					 ecc_header_info->signature, ecc_header_info->version, ecc_header_info->length,
					 ecc_header_info->crc_check);
	ret = memcpy_s((uint8_t *)g_ecc_config_tool->ecc_statistics_info, sizeof(struct ecc_isolated_statistics_record) *
				   MAX_OS_DEVICE_COUNT, g_ecc_config_tool->ecc_config_data + ecc_header_info->statistics_info_offset,
				   sizeof(struct ecc_isolated_statistics_record) * MAX_OS_DEVICE_COUNT);
	if (ret) {
		memory_drv_err("Copy ECC statistics info failed. (ret=%d)\n", ret);
	}
	return ret;
}
#endif

int hbm_get_isolated_page_statistics(void *feature, char *in, uint32_t in_len, char *out, uint32_t out_len)
{
	int ret;
	uint32_t dev_id = 0;

	if (in == NULL) {
		memory_drv_err("null pointer of input data.\n");
		return -EINVAL;
	}

	if (in_len != sizeof(uint32_t)) {
		memory_drv_err("invalid input data size, size = %u which should be %lu\n", in_len, sizeof(uint32_t));
		return -EINVAL;
	}

	ret = memcpy_s((void *)&dev_id, sizeof(uint32_t), (void *)in, in_len);
	if (ret) {
		memory_drv_err("Call memcpy_s failed. (ret=%d)\n", ret);
		return ret;
	}

	if (dev_id >= MAX_OS_DEVICE_COUNT) {
		memory_drv_err("physical id(%u) is invalid.\n", dev_id);
		return -EINVAL;
	}
	ret = memcpy_s((void *)out, out_len, (void *)&(g_ecc_config_tool->ecc_statistics_info[dev_id]),
		sizeof(struct ecc_isolated_statistics_record));
	if (ret) {
		memory_drv_err("Call memcpy_s failed. (ret=%d)\n", ret);
	}
	return ret;
}

#ifdef CFG_HBM_FLASH
STATIC int hbm_init_ecc_config_data(uint32_t dev_id)
{
	int ret;
	struct ecc_config_header_s *ecc_header_info = (struct ecc_config_header_s *)(g_ecc_config_tool->ecc_config_data +
		dev_id * ECC_FLASH_DIE_SIZE);

	/* init ecc_config_header_s if inserted for the first time */
	ret = memset_s(g_ecc_config_tool->ecc_config_data + dev_id * ECC_FLASH_DIE_SIZE,
		ECC_FLASH_DIE_SIZE, 0, ECC_FLASH_DIE_SIZE);
	if (ret != OK) {
		memory_drv_err("Memset_s ecc data info failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
		return ret;
	}
	ecc_header_info->signature = ECC_HEADER_SIGNATURE;	  /* after memset need init signature */
	ecc_header_info->version = ECC_HEADER_VERSION_V2;	   /* after memset need init version */
	ecc_header_info->crc_check = ECC_CONFIG_CRC16;		  /* after memset need init crc_check */
	ecc_header_info->statistics_info_offset = STATISTICS_INFO_FLASH_OFFSET;
	/* init gloabl statistics record */
	ret = memset_s((uint8_t *)(g_ecc_config_tool->ecc_statistics_info + dev_id),
		sizeof(struct ecc_isolated_statistics_record), 0, sizeof(struct ecc_isolated_statistics_record));
	if (ret != OK) {
		memory_drv_err("Memset_s statistics data failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
	}
	return ret;
}

int hbm_clear_isolated_info(void *feature, char *in, uint32_t in_len, char *out, uint32_t out_len)
{
	int ret;
	uint32_t dev_id;
	uint32_t dev_num = 0;

	memory_drv_event("Ecc_config clear flash data.\n");
	if ((in == NULL) || (in_len != sizeof(uint32_t))) {
		memory_drv_err("In or in_len error.\n");
		return -EINVAL;
	}
	/* get device number */
	ret = devdrv_get_devnum(&dev_num);
	if (ret != OK) {
		memory_drv_err("Get dev_num failed. (ret=%d)\n", ret);
		return -EINVAL;
	}
	/* translate device id */
	dev_id = *(unsigned int *)(uintptr_t)in;
	if (dev_id >= dev_num) {
		memory_drv_err("Device id is error. (dev_id=%u; dev_num%u)\n", dev_id, dev_num);
		return -EINVAL;
	}
	/* must use mutex for multithreaded competition */
	mutex_lock(&g_ecc_config_tool->record_lock);
	/* dmp process called no need kthread to call */
	ret = hbm_flash_erase(NULL, ECC_CONFIG_START_OFFSET, FLASH_ERASE_BLOCK_SIZE);
	if (ret != OK) {
		mutex_unlock(&g_ecc_config_tool->record_lock);
		memory_drv_err("Erase flash failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
		return ret;
	}
	/* init memory data */
	ret = hbm_init_ecc_config_data(dev_id);
	if (ret != OK) {
		mutex_unlock(&g_ecc_config_tool->record_lock);
		memory_drv_err("Clear ecc config data failed. (ret=%d; dev_id=%u)\n", ret, dev_id);
		return ret;
	}
	mutex_unlock(&g_ecc_config_tool->record_lock);
	/* after init need clear single ecc data */
	ret = hbm_clear_single_ecc_info(dev_id);
	if (ret != OK) {
		memory_drv_err("Clear single ecc config data failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
		return ret;
	}
	mutex_lock(&g_ecc_config_tool->record_lock);
	/* after erase flash need write again to make BIOS ECC check pass */
	ret = hbm_flash_write(NULL, ECC_CONFIG_START_OFFSET, FLASH_ERASE_BLOCK_SIZE, g_ecc_config_tool->ecc_config_data);
	mutex_unlock(&g_ecc_config_tool->record_lock);

	return ret;
}
#else
int hbm_clear_isolated_info(void *feature, char *in, uint32_t in_len, char *out, uint32_t out_len)
{
	int ret;

	memory_drv_event("ecc_config clear flash data.\n");
	mutex_lock(&g_ecc_config_tool->record_lock);

	ret = hbm_flash_erase(ECC_CONFIG_FLASH_PART, ECC_CONFIG_START_OFFSET, FLASH_ERASE_BLOCK_SIZE);
	if (ret != OK) {
		mutex_unlock(&g_ecc_config_tool->record_lock);
		memory_drv_err("erase flash failed, ret: %d!\n", ret);
		return ret;
	}

	ret = hbm_init_memory_data();
	if (ret != OK) {
		mutex_unlock(&g_ecc_config_tool->record_lock);
		memory_drv_err("clear ecc config data failed, ret: %d!\n", ret);
		return ret;
	}
	mutex_unlock(&g_ecc_config_tool->record_lock);

	ret = hbm_clear_single_ecc_info();
	if (ret != OK) {
		memory_drv_err("clear single ecc config data failed, ret: %d!\n", ret);
	}
	return ret;
}
#endif

#ifdef CFG_HBM_FLASH
static inline uint32_t hbm_get_current_multi_ecc_record_count(uint32_t dev_id)
{
	struct ecc_isolated_statistics_record *statistics_record = (struct ecc_isolated_statistics_record *)
		(g_ecc_config_tool->ecc_config_data + STATISTICS_INFO_FLASH_OFFSET + dev_id * ECC_FLASH_DIE_SIZE);

	return statistics_record->hbm_multi_isolated_page_count;	/* update multi isolated ecc count */
}
#else
static inline uint32_t hbm_get_current_multi_ecc_record_count(uint32_t dev_id)
{
	struct ecc_config_header_s *ecc_header_info = (struct ecc_config_header_s *)g_ecc_config_tool->ecc_config_data;
	struct ecc_isolated_statistics_record *statistics_record = (struct ecc_isolated_statistics_record *)
		(g_ecc_config_tool->ecc_config_data + ecc_header_info->statistics_info_offset);

	statistics_record += dev_id;

	return statistics_record->ddr_multi_isolated_page_count +
		statistics_record->hbm_multi_isolated_page_count;
}
#endif

STATIC void hbm_get_translate_multi_ecc_time(uint32_t dev_id, struct multi_ecc_time_data_s *multi_ecc_times)
{
	uint32_t recorded_count;
	uint32_t arr_i = 0;
	uint32_t recorded_i = 0;
	uint32_t chipid, curr_devid;
	struct external_info *time_info = NULL;
	struct multi_ecc_err_info_s *ecc_err_info = NULL;
	struct ecc_config_header_s *ecc_header_info = NULL;

	mutex_lock(&g_ecc_config_tool->record_lock);
#ifdef CFG_HBM_FLASH
	ecc_header_info = (struct ecc_config_header_s *)(g_ecc_config_tool->ecc_config_data + dev_id * ECC_FLASH_DIE_SIZE);
#else
	ecc_header_info = (struct ecc_config_header_s *)g_ecc_config_tool->ecc_config_data;
#endif
	multi_ecc_times->multi_record_count = hbm_get_current_multi_ecc_record_count(dev_id);

	recorded_count = ecc_header_info->length / sizeof(struct multi_ecc_err_info_s);
	for (recorded_i = 0; recorded_i < recorded_count; recorded_i++) {
#ifdef CFG_HBM_FLASH
		ecc_err_info = (struct multi_ecc_err_info_s *)(g_ecc_config_tool->ecc_config_data +
			dev_id * ECC_FLASH_DIE_SIZE + (sizeof(struct ecc_config_header_s) +
			recorded_i * sizeof(struct multi_ecc_err_info_s)));
#else
		ecc_err_info = (struct multi_ecc_err_info_s *)(g_ecc_config_tool->ecc_config_data +
			(sizeof(struct ecc_config_header_s) + recorded_i * sizeof(struct multi_ecc_err_info_s)) %
			FLASH_ERASE_BLOCK_SIZE);
#endif
		// filter addr belonging to the current devid
		if (memory_get_adapter()->get_chip_die(ecc_err_info->physical_addr, &chipid, &curr_devid) != OK ||
			curr_devid != dev_id) {
			continue;
		}
		/* multi ecc occurrence time is recorded in reverse order */
#ifdef CFG_HBM_FLASH
		time_info = (struct external_info *)&ecc_err_info->timestamp;
#else
		time_info = (struct external_info *)(g_ecc_config_tool->ecc_config_data +
			(ecc_header_info->statistics_info_offset - sizeof(struct external_info) * (recorded_i + 1)) %
			FLASH_ERASE_BLOCK_SIZE);
#endif
		if ((~time_info->timer_stamp) == 0) {
			time_info->timer_stamp = 0;
		}

		multi_ecc_times->multi_ecc_times[arr_i] = (uint32_t)time_info->timer_stamp;
		arr_i++;
		if (arr_i >= MAX_RECORD_ECC_ADDR_COUNT) {
			break;
		}
	}
	mutex_unlock(&g_ecc_config_tool->record_lock);
}

STATIC int hbm_read_multi_ecc_time_info(struct ecc_config_udata_s *ecc_config_udata)
{
	hbm_get_translate_multi_ecc_time(ecc_config_udata->dev_id, &ecc_config_udata->multi_ecc_time_data);
	return OK;
}

#ifdef CFG_HBM_FLASH
static inline void set_max_unint16_ecc_statistics(uint32_t *original_value, const uint32_t change_value)
{
	uint32_t bigger_value = *original_value + change_value;	 /* update bigger_value */
	if ((change_value > MAX_UNINT32_VALUE) || ((*original_value) > bigger_value)) {
		*original_value = MAX_UNINT32_VALUE;
	} else {
		*original_value = bigger_value;	 /* update original_value */
	}
}

STATIC int hbm_calculate_record_ecc_statistics(
	uint32_t change_single_bit_count[ECC_MODULE_COUNT][MAX_OS_DEVICE_COUNT],
	uint32_t change_multi_ecc_count[ECC_MODULE_COUNT][MAX_OS_DEVICE_COUNT],
	uint8_t is_write_to_flash, uint8_t is_change_print)
{
	uint32_t dev_id;
	int ret;
	uint32_t dev_num = 0;
	struct ecc_isolated_statistics_record *ecc_record_statistics_data = NULL;
	/* get die number */
	ret = devdrv_get_devnum(&dev_num);
	if (ret != OK) {
		memory_drv_err("Get dev_num failed. (ret=%d)\n", ret);
		return ret;
	}
	mutex_lock(&g_ecc_config_tool->record_lock);
	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		ecc_record_statistics_data = (struct ecc_isolated_statistics_record *)(g_ecc_config_tool->ecc_config_data +
			dev_id * ECC_FLASH_DIE_SIZE + STATISTICS_INFO_FLASH_OFFSET);	 /* update statistics data */
		set_max_unint16_ecc_statistics(&(g_ecc_config_tool->ecc_statistics_info[dev_id].hbm_single_bit_count),
			change_single_bit_count[1][dev_id]);
		set_max_unint16_ecc_statistics(&(g_ecc_config_tool->ecc_statistics_info[dev_id].hbm_mul_bit_count),
			change_multi_ecc_count[1][dev_id]);

		/* update ecc_record_statistics_data */
		ecc_record_statistics_data->hbm_single_bit_count =
			g_ecc_config_tool->ecc_statistics_info[dev_id].hbm_single_bit_count;	/* update single ecc count */
		ecc_record_statistics_data->hbm_mul_bit_count =
			g_ecc_config_tool->ecc_statistics_info[dev_id].hbm_mul_bit_count;   /* update multi ecc count */
		if (is_change_print) {
			memory_drv_event("Single bit ecc errors. (hbm=%u)\n",
				ecc_record_statistics_data->hbm_single_bit_count);
		}
		if (is_write_to_flash) {
			ret = hbm_write_ecc_config_data(dev_id);	  /* write ecc config data to flash */
			if (ret != OK) {
				memory_drv_err("ecc_config write data to flash failed, ret:%d.\n", ret);
			}
		}
	}
	mutex_unlock(&g_ecc_config_tool->record_lock);
	return ret;
}

#ifndef CFG_DDR_ISOLATION_710
STATIC int hbm_get_ecc_count_info(struct ecc_bit_count_info *ecc_bit_count)
{
	int ret;
	uint64_t base;
	uint32_t corr_single_cnt = 0;
	uint32_t corr_multi_cnt = 0;
	/* get base address */
	ret = get_hbm_stack_base(ecc_bit_count->dev_id, ecc_bit_count->hbm_stack_i, &base);
	if (ret != OK) {
		memory_drv_err("Get stack base addr fail. (ret=%d)\n", ret);
		return ret;
	}
	/* update base addresss */
	base += HBM_HBMC_BASE_ADDR + HBM_HBMC_PC_OFFSET * ecc_bit_count->hbm_pc_i + HBM_HBMC_RASC_ADDR;
	/* read multi bit error count register to get current error count */
	ret = read_reg32(base + RASC_HIS_UNCORR_CNT, &corr_multi_cnt);
	if (ret != OK) {
		memory_drv_err("Read corrected ecc count failed. (dev=%u; hbm_stack=%u; hbm_pc=%u; ret=%d)\n",
			ecc_bit_count->dev_id, ecc_bit_count->hbm_stack_i, ecc_bit_count->hbm_pc_i, ret);
		return ret;
	}
	ecc_bit_count->multi_bit_count = corr_multi_cnt;  /* update multi bit ecc count */

	/* get single bit ecc count */
	ret = read_reg32(base + RASC_HIS_CORR_CNT, &corr_single_cnt);
	if (ret != OK) {
		memory_drv_err("Read corrected ecc count failed. (dev=%u; hbm_stack=%u; hbm_pc=%u; ret=%d)\n",
			ecc_bit_count->dev_id, ecc_bit_count->hbm_stack_i, ecc_bit_count->hbm_pc_i, ret);
		return ret;
	}
	ecc_bit_count->single_bit_count = corr_single_cnt;   /* update single ecc count */

	return ret;
}

STATIC void hbm_ecc_count_record(uint32_t dev_id, uint32_t *single_bit_count, uint32_t *multi_bit_count)
{
	uint32_t hbm_stack_i, hbm_pc_i;
	int ret;
	struct ecc_bit_count_info ecc_bit_count = {0};
	uint32_t multi_ecc_record = 0;
	uint32_t single_ecc_record = 0;
	/* traverse all error */
	for (hbm_stack_i = 0; hbm_stack_i < MAX_HBM_STACK_PER_SOCKET; hbm_stack_i++) {
		for (hbm_pc_i = 0; hbm_pc_i < MAX_HBM_PC_PER_STACK; hbm_pc_i++) {
			ecc_bit_count.dev_id = dev_id;			  /* update dev_id */
			ecc_bit_count.hbm_stack_i = hbm_stack_i;	/* update hbm_stack */
			ecc_bit_count.hbm_pc_i = hbm_pc_i;		  /* update hbm_pc */
			/* update multi ecc current data and single ecc current data */
			ret = hbm_get_ecc_count_info(&ecc_bit_count);
			if (ret != OK) {
				memory_drv_err("Read corrected ecc count failed. (dev=%u; hbm_stack=%u; hbm_pc=%u; ret=%d)\n",
					dev_id, hbm_stack_i, hbm_pc_i, ret);
				break;
			}
			single_ecc_record += ecc_bit_count.single_bit_count;  /* update global single ecc record */
			multi_ecc_record += ecc_bit_count.multi_bit_count;	/* update global multi ecc record */
		}
	}
	*single_bit_count = single_ecc_record;		/* return current single ecc record */
	*multi_bit_count = multi_ecc_record;		  /* return current multi ecc record */
}
#else
STATIC void hbm_ecc_count_record(uint32_t dev_id, uint32_t *single_bit_count, uint32_t *multi_bit_count)
{
	get_ecc_count_record(dev_id, single_bit_count, multi_bit_count);
}
#endif

STATIC void hbm_update_ecc_aggregate_record(uint32_t dev_id)
{
	uint32_t single_ecc_record = 0;
	uint32_t multi_ecc_change = 0;
	static uint32_t multi_ecc_cur_record = 0;
	static uint32_t multi_ecc_pre_record = 0;

	struct ecc_isolated_statistics_record *ecc_record_statistics_data = NULL;
	ecc_record_statistics_data = (struct ecc_isolated_statistics_record *)(g_ecc_config_tool->ecc_config_data +
		dev_id * ECC_FLASH_DIE_SIZE + STATISTICS_INFO_FLASH_OFFSET);	 /* update statistics data */

	hbm_ecc_count_record(dev_id, &single_ecc_record, &multi_ecc_cur_record);
	if (multi_ecc_cur_record > multi_ecc_pre_record) {
		multi_ecc_change = multi_ecc_cur_record - multi_ecc_pre_record;	 /* update change multi ecc count */
		multi_ecc_pre_record = multi_ecc_cur_record;						/* update pre multi ecc count */
	}
	ecc_record_statistics_data->hbm_mul_bit_count = multi_ecc_change +
		g_ecc_config_tool->ecc_statistics_info[dev_id].hbm_mul_bit_count;   /* update multi ecc count */
}

#else
static inline void set_max_unint16_ecc_statistics(uint16_t *original_value, const uint32_t change_value)
{
	uint16_t bigger_value = *original_value + change_value;	 /* update bigger_value */
	if ((change_value > MAX_UNINT16_VALUE) || ((*original_value) > bigger_value)) {
		*original_value = MAX_UNINT16_VALUE;
	} else {
		*original_value = bigger_value;	 /* update original_value */
	}
}

STATIC int hbm_calculate_record_ecc_statistics(
	uint16_t change_single_bit_count[ECC_MODULE_COUNT][MAX_OS_DEVICE_COUNT],
	uint16_t change_multi_ecc_count[ECC_MODULE_COUNT][MAX_OS_DEVICE_COUNT],
	uint8_t is_write_to_flash, uint8_t is_change_print)
{
	int dev_id;
	int ret = 0;
	uint32_t dev_num = 0;
	struct ecc_config_header_s *ecc_header_info = NULL;
	struct ecc_isolated_statistics_record *ecc_record_statistics_data = NULL;

	/* get device number */
	ret = devdrv_get_devnum(&dev_num);
	if (ret != OK) {
		memory_drv_err("Get dev_num failed. (ret=%d)\n", ret);
		return -EINVAL;
	}
	mutex_lock(&g_ecc_config_tool->record_lock);
	ecc_header_info = (struct ecc_config_header_s *)g_ecc_config_tool->ecc_config_data;
	ecc_record_statistics_data = (struct ecc_isolated_statistics_record *)(g_ecc_config_tool->ecc_config_data +
			ecc_header_info->statistics_info_offset % FLASH_ERASE_BLOCK_SIZE);

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		set_max_unint16_ecc_statistics(&(g_ecc_config_tool->ecc_statistics_info[dev_id].ddr_single_bit_count),
			change_single_bit_count[0][dev_id]);
		set_max_unint16_ecc_statistics(&(g_ecc_config_tool->ecc_statistics_info[dev_id].ddr_mul_bit_count),
			change_multi_ecc_count[0][dev_id]);
		set_max_unint16_ecc_statistics(&(g_ecc_config_tool->ecc_statistics_info[dev_id].hbm_single_bit_count),
			change_single_bit_count[1][dev_id]);
		set_max_unint16_ecc_statistics(&(g_ecc_config_tool->ecc_statistics_info[dev_id].hbm_mul_bit_count),
			change_multi_ecc_count[1][dev_id]);

		ecc_record_statistics_data[dev_id].ddr_single_bit_count =
			g_ecc_config_tool->ecc_statistics_info[dev_id].ddr_single_bit_count;
		ecc_record_statistics_data[dev_id].ddr_mul_bit_count =
			g_ecc_config_tool->ecc_statistics_info[dev_id].ddr_mul_bit_count;
		ecc_record_statistics_data[dev_id].hbm_single_bit_count =
			g_ecc_config_tool->ecc_statistics_info[dev_id].hbm_single_bit_count;
		ecc_record_statistics_data[dev_id].hbm_mul_bit_count =
			g_ecc_config_tool->ecc_statistics_info[dev_id].hbm_mul_bit_count;

		if (is_change_print) {
			memory_drv_event("dev_id-%d current single bit ecc errors: ddr-%u, hbm-%u.\n",
				dev_id, ecc_record_statistics_data[dev_id].ddr_single_bit_count,
				ecc_record_statistics_data[dev_id].hbm_single_bit_count);
		}
	}

	if (is_write_to_flash) {
		ret = hbm_write_ecc_config_data();
		if (ret) {
			memory_drv_err("ecc_config write data to flash failed, ret:%d.\n", ret);
		}
	}
	mutex_unlock(&g_ecc_config_tool->record_lock);
	return ret;
}

STATIC int hbm_get_ecc_statistics(uint32_t dev_id, uint8_t module_type, uint16_t *single_bit_count,
	uint16_t *mul_bit_count)
{
#ifdef SUPPORT_IPC_GET_ECC
	struct devdrv_ipc_imu ipc = {0};
	struct devdrv_ipc_imu ack = {0};
	uint32_t new_single_bit_count, new_multi_bit_count;
	u8 ack_cmd_para;
	int ret;

	ipc.target_id = OBJ_IMU_DMP;
	ipc.source_id = OBJ_AP_DMP;
	ipc.cmd_type1 = IPC_IMU_ECC_1;
	ipc.cmd_type0 = module_type;

	ret = icm_msg_send_sync(ICM_FD_BUILD(dev_id, HISI_RPROC_TX_IMU_MBX25),
		(rproc_msg_t *)&ipc, 2, (rproc_msg_t *)&ack, 8); /* send 2 * 4bytes, receive 8 * 4bytes */
	if (ret) {
		memory_drv_warn("devdrv imu get ecc statistic failed.\n");
		return ret;
	}

	ack_cmd_para = ack.cmd_para0 | ack.cmd_para1 | ack.cmd_para2 | ack.cmd_para3;
	if ((ack.source_id != OBJ_IMU_DMP) || (ack.target_id != OBJ_AP_DMP) || (ack_cmd_para != 0) ||
		(ack.cmd_type0 != ipc.cmd_type0) || (ack.cmd_type1 != ipc.cmd_type1)) {
		memory_drv_warn("get invalid ipc message.\n"
						"ack.source_id = %d, ack.target_id = %d.\n"
						"ack.cmd_para0 = %d, ack.cmd_para1 = %d.\n"
						"ack.cmd_para2 = %d, ack.cmd_para3 = %d.\n"
						"ack.cmd_type0 = %d, ipc.cmd_type0 = %d.\n"
						"ack.cmd_type1 = %d, ipc.cmd_type1 = %d.\n",
						ack.source_id, ack.target_id, ack.cmd_para0,
						ack.cmd_para1, ack.cmd_para2, ack.cmd_para3,
						ack.cmd_type0, ipc.cmd_type0, ack.cmd_type1, ipc.cmd_type1);
		return -EINVAL;
	}
	new_single_bit_count = ((u32)ack.cmd_data0) | (((u32)ack.cmd_data1) << OFFSET_EIGHT) |
		(((u32)ack.cmd_data2) << OFFSET_SIXTEEN) | (((u32)ack.cmd_data3) << OFFSET_TWENTY_FOUR);
	new_multi_bit_count = ack.data1;

	*single_bit_count = new_single_bit_count > MAX_UNINT16_VALUE ? MAX_UNINT16_VALUE : new_single_bit_count;
	*mul_bit_count = new_multi_bit_count > MAX_UNINT16_VALUE ? MAX_UNINT16_VALUE : new_multi_bit_count;
#endif
	return OK;
}
#endif

#ifdef CFG_HBM_FLASH
STATIC void hbm_handle_single_ecc_change(uint32_t *single_bit_change_print, uint32_t *single_bit_change_record,
	uint8_t *is_change_print, uint8_t *is_write_to_flash)
#else
STATIC void hbm_handle_single_ecc_change(uint8_t *single_bit_change_print, uint8_t *single_bit_change_record,
	uint8_t *is_change_print, uint8_t *is_write_to_flash)
#endif
{
	static uint32_t print_interval_count = 0;
	static uint32_t recorded_interval_count = 0;

	print_interval_count++;
	recorded_interval_count++;
	if (*single_bit_change_print && (print_interval_count >= SINGLE_ECC_PRINT_INTERVAL)) {
		*single_bit_change_print = 0;
		*is_change_print = 1;
		print_interval_count = 0;
	}

	if (*single_bit_change_record && (recorded_interval_count >= SINGLE_ECC_WRITE_FLASH_INTERVAL)) {
		*single_bit_change_record = 0;
		*is_write_to_flash = 1;
		recorded_interval_count = 0;
	}
}

STATIC void hbm_handle_multi_ecc_change(uint8_t *multi_bit_change_record, uint32_t *multi_bit_record_interval_count,
	uint8_t *is_write_to_flash)
{
	if (*multi_bit_change_record && *multi_bit_record_interval_count >= SAVE_MULTI_ECC_COUNT_INTERVAL) {
		*is_write_to_flash = 1;			   /* update is_write_to_flash flag */
		*multi_bit_record_interval_count = 0;
		*multi_bit_change_record = 0;
	}
}

#ifdef CFG_HBM_FLASH
static inline void hbm_check_single_bit_change(uint32_t current_single_bit_count, uint32_t *pre_single_bit_count,
	uint32_t *change_single_bit_count, uint32_t *single_bit_change_record, uint32_t *single_bit_change_print)
#else
static inline void hbm_check_single_bit_change(uint16_t current_single_bit_count, uint16_t *pre_single_bit_count,
	uint16_t *change_single_bit_count, uint8_t *single_bit_change_record, uint8_t *single_bit_change_print)
#endif
{
	if (*pre_single_bit_count < current_single_bit_count) {
		*single_bit_change_record = 1;
		*single_bit_change_print = 1;	   /* update single_bit_change_print */
		*change_single_bit_count = current_single_bit_count - *pre_single_bit_count;
		*pre_single_bit_count = current_single_bit_count;	   /* update pre_single_bit_count */
	}
}
#ifdef CFG_HBM_FLASH
static inline void hbm_check_multi_bit_change(uint32_t current_multi_ecc_count, uint32_t *pre_multi_ecc_count,
	uint32_t *change_multi_ecc_count, uint8_t *multi_bit_change_record)
#else
static inline void hbm_check_multi_bit_change(uint16_t current_multi_ecc_count, uint16_t *pre_multi_ecc_count,
	uint16_t *change_multi_ecc_count, uint8_t *multi_bit_change_record)
#endif
{
	if (*pre_multi_ecc_count < current_multi_ecc_count) {
		*change_multi_ecc_count = current_multi_ecc_count - *pre_multi_ecc_count;
		*pre_multi_ecc_count = current_multi_ecc_count;	 /* update pre_multi_ecc_count */
		*multi_bit_change_record = 1;
	}
}

STATIC void hbm_period_statistic(void)
{
	int ret, arr_i;
	uint32_t dev_id, dev_num;
	uint8_t is_write_to_flash = 0;
	uint8_t is_change_print = 0;
	static uint32_t multi_bit_record_interval_count = 0;
	static uint8_t multi_bit_change_record = 0;
#ifdef CFG_HBM_FLASH
	uint32_t current_single_bit_count[ECC_MODULE_COUNT][MAX_OS_DEVICE_COUNT] = {0};
	uint32_t current_multi_ecc_count[ECC_MODULE_COUNT][MAX_OS_DEVICE_COUNT] = {0};
	uint32_t change_single_bit_count[ECC_MODULE_COUNT][MAX_OS_DEVICE_COUNT] = {0};
	uint32_t change_multi_ecc_count[ECC_MODULE_COUNT][MAX_OS_DEVICE_COUNT] = {0};
	static uint32_t single_bit_change_record = 0;
	static uint32_t single_bit_change_print = 0;
	static uint32_t pre_single_bit_count[ECC_MODULE_COUNT][MAX_OS_DEVICE_COUNT] = {0};
	static uint32_t pre_multi_ecc_count[ECC_MODULE_COUNT][MAX_OS_DEVICE_COUNT] = {0};
#else
	uint8_t all_ecc_module[ECC_MODULE_COUNT] = { IPC_IMU_DDR, IPC_IMU_HBM };
	uint16_t current_single_bit_count[ECC_MODULE_COUNT][MAX_OS_DEVICE_COUNT] = {0};
	uint16_t current_multi_ecc_count[ECC_MODULE_COUNT][MAX_OS_DEVICE_COUNT] = {0};
	uint16_t change_single_bit_count[ECC_MODULE_COUNT][MAX_OS_DEVICE_COUNT] = {0};
	uint16_t change_multi_ecc_count[ECC_MODULE_COUNT][MAX_OS_DEVICE_COUNT] = {0};
	static uint8_t single_bit_change_record = 0;
	static uint8_t single_bit_change_print = 0;
	static uint16_t pre_single_bit_count[ECC_MODULE_COUNT][MAX_OS_DEVICE_COUNT] = {0};
	static uint16_t pre_multi_ecc_count[ECC_MODULE_COUNT][MAX_OS_DEVICE_COUNT] = {0};
#endif
	ret = devdrv_get_devnum(&dev_num);
	if (ret != 0) {
		memory_drv_err("Get dev_num failed. (ret=%d)\n", ret);
		return;
	}

	for (arr_i = 0; arr_i < ECC_MODULE_COUNT; arr_i++) {
		for (dev_id = 0; dev_id < dev_num; dev_id++) {
#ifdef CFG_HBM_FLASH
			hbm_ecc_count_record(dev_id, &current_single_bit_count[arr_i][dev_id],
				&current_multi_ecc_count[arr_i][dev_id]);
#else
			ret = hbm_get_ecc_statistics(dev_id, all_ecc_module[arr_i],
				&current_single_bit_count[arr_i][dev_id], &current_multi_ecc_count[arr_i][dev_id]);
			if (ret) {
				memory_drv_warn("ecc config get ecc statistic failed, module:%d, dev_id:%u, \
					ret:%d.\n", arr_i, dev_id, ret);
				continue;
			}
#endif
			hbm_check_single_bit_change(current_single_bit_count[arr_i][dev_id], &pre_single_bit_count[arr_i][dev_id],
				&change_single_bit_count[arr_i][dev_id], &single_bit_change_record, &single_bit_change_print);
			hbm_check_multi_bit_change(current_multi_ecc_count[arr_i][dev_id], &pre_multi_ecc_count[arr_i][dev_id],
				&change_multi_ecc_count[arr_i][dev_id], &multi_bit_change_record);
		}
	}
	multi_bit_record_interval_count++;
	/* update write to flash flag */
	hbm_handle_single_ecc_change(&single_bit_change_print, &single_bit_change_record, &is_change_print,
		&is_write_to_flash);
	hbm_handle_multi_ecc_change(&multi_bit_change_record, &multi_bit_record_interval_count, &is_write_to_flash);
	ret = hbm_calculate_record_ecc_statistics(change_single_bit_count, change_multi_ecc_count, is_write_to_flash,
		is_change_print);
	if (ret) {
		memory_drv_err("calculate and record ecc statistics failed, ret:%d!\n", ret);
	}
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
/*
 * collect the processes who have the pfn.
 */

struct to_kill {
	struct list_head nd;
	struct task_struct *tsk;
	unsigned long addr;
	short size_shift;
};

pid_t *collect_process_by_pfn(unsigned long pfn, int *size)
{
	struct to_kill *tk = NULL;
	struct to_kill *next = NULL;
	struct page *page = NULL;
	struct page *page_online = NULL;
	int tmp = 0;
	pid_t *pids = NULL;
	LIST_HEAD(tokill);

	*size = 0;
	page_online = pfn_to_online_page(pfn);
	if (!page_online) {
		if (!pfn_valid(pfn)) {
			memory_drv_err("pfn is invalid\n");
			return NULL;
		}
	}
	page = compound_head(page_online);
	if (!page) {
		memory_drv_err("pfn to page is invalid\n");
		return NULL;
	}

	lock_page(page);
	if (!page_mapped(page)) {
		unlock_page(page);
		return NULL;
	}
	collect_procs(page, &tokill, true);
	unlock_page(page);

	if (list_empty(&tokill)) {
		return NULL;
	}

	list_for_each_entry (tk, &tokill, nd) {
		(*size)++;
	}

	pids = kmalloc(sizeof(pid_t) * (*size), GFP_KERNEL | __GFP_ACCOUNT);
	if (!pids) {
		memory_drv_err("Alloc memory error\n");
		return NULL;
	}

	list_for_each_entry_safe (tk, next, &tokill, nd) {
		pids[tmp] = tk->tsk->pid;
		put_task_struct(tk->tsk);
		kfree(tk);
		tmp++;
	}
	return pids;
}
#endif

#ifndef CFG_HBM_FAULT_EVENT
STATIC void period_check_ecc_pfns(void)
{
	struct pfn_list *pos = NULL;
	struct pfn_list *n = NULL;
	pid_t *ecc_pids = NULL;
	int ecc_pid_num = 0;
	uint32_t dev_id;

	for (dev_id = 0; dev_id < MAX_OS_DEVICE_COUNT; dev_id++) {
		mutex_lock(&g_ecc_config_tool->un_online_isolated_pfn_queue[dev_id].mutex);
		list_for_each_entry_safe(pos, n, &g_ecc_config_tool->un_online_isolated_pfn_queue[dev_id].pfns.list, list) {
			ecc_pids = collect_process_by_pfn(pos->pfn, &ecc_pid_num);
			if (ecc_pids == NULL) {
				list_del(&pos->list);
				hbm_kfree((void **)&pos);
				g_ecc_config_tool->un_online_isolated_pfn_queue[dev_id].pfn_num--;
			} else {
				hbm_kfree((void **)&ecc_pids);
			}
		}
		mutex_unlock(&g_ecc_config_tool->un_online_isolated_pfn_queue[dev_id].mutex);
	}
}
#endif

STATIC void statistics_update_task(struct work_struct *statistics_task)
{
	static uint32_t read_single_ecc_interval = 0;
	static uint32_t save_single_ecc_interval = 0;
	if (statistics_task == NULL) {
		memory_drv_err("param is NULL invalid.\n");
		return;
	}

	hbm_period_statistic();

	if (read_single_ecc_interval >= TEN_SECOND_INTERVAL) {
#ifndef CFG_HBM_FAULT_EVENT
		/* check and recovery multi-ecc events every 10s */
		period_check_ecc_pfns();
#endif
		/* read single bit ECC addr every 10s */
#ifndef CFG_FEATURE_NO_CECC_ADDR_REC
		hbm_process_single_count();
#endif
		read_single_ecc_interval = 0;
	} else {
		read_single_ecc_interval++;
	}
	if (save_single_ecc_interval >= SAVE_SINGLE_ECC_ADDR_INTERVAL) {
		hbm_write_single_ecc_data_to_flash();
		save_single_ecc_interval = 0;
	} else {
		save_single_ecc_interval++;
	}
}

STATIC void hbm_statistics_timer(struct timer_list *statistics_timer)
{
	struct ecc_config_tool_s *ecc_config_tool = NULL;
	if (statistics_timer == NULL) {
		memory_drv_err("statistics param invalid.\n");
		return;
	}
	ecc_config_tool = from_timer(ecc_config_tool, statistics_timer, statistics_timer);

	(void)mod_timer(&ecc_config_tool->statistics_timer, jiffies + PREIOD_STATISTIC_TIME * HZ);
	(void)schedule_work(&ecc_config_tool->statistics_task);
}

STATIC int hbm_check_ddr_ecc_is_recorded(uint64_t recorded_addr, uint64_t new_addr)
{
	uint64_t ddr_align_address = (recorded_addr & DDR_MEMERY_PAGE_MASK);
	uint64_t ddr_align_max_address = ddr_align_address + DDR_MEMERY_PAGE_SIZE * DDR_ALIGN_MAX_ADDR_COUNT;
	/* DDR ECC physical address isolates three memory pages */
	if ((ddr_align_address == 0) || (ddr_align_address == SECOND_DDR_START_BASE)) {
		if (((new_addr & DDR_MEMERY_PAGE_MASK) == ddr_align_address) && (new_addr < ddr_align_max_address)) {
			return ECC_ADDR_INFO_RECORDED;
		}
	} else {
		if ((new_addr >= (ddr_align_address - DDR_MEMERY_PAGE_SIZE)) && (new_addr < ddr_align_max_address)) {
			return ECC_ADDR_INFO_RECORDED;
		}
	}
	return ECC_ADDR_INFO_NOT_RECORDED;
}

STATIC int hbm_check_new_ecc_addr_recored(const struct multi_ecc_err_info_s new_ecc_addr_info,
	struct multi_ecc_err_info_s *ecc_addr_info_arr, uint16_t recorded_length)
{
	int is_recored = ECC_ADDR_INFO_NOT_RECORDED;
	int addr_i;
	int recorded_count = recorded_length / sizeof(struct multi_ecc_err_info_s);

	if (new_ecc_addr_info.module == ECC_MODULE_DDR) {
		for (addr_i = 0; addr_i < recorded_count; addr_i++) {
			/* DDR ECC physical address isolates three memory pages */
			if ((ecc_addr_info_arr[addr_i].module == new_ecc_addr_info.module) &&
				(hbm_check_ddr_ecc_is_recorded(ecc_addr_info_arr[addr_i].physical_addr,
				new_ecc_addr_info.physical_addr) == ECC_ADDR_INFO_RECORDED)) {
				is_recored = ECC_ADDR_INFO_RECORDED;
				break;
			}
		}
	} else {
		/* HBM ECC physical address is aligned with 1M for isolation, and the isolation size is 1M */
		for (addr_i = 0; addr_i < recorded_count; addr_i++) {
			if ((ecc_addr_info_arr[addr_i].module == new_ecc_addr_info.module) &&
				((ecc_addr_info_arr[addr_i].physical_addr & HBM_ROW_MEMERY_MASK) ==
				(new_ecc_addr_info.physical_addr & HBM_ROW_MEMERY_MASK))) {
				is_recored = ECC_ADDR_INFO_RECORDED;
				break;
			}
		}
	}
	return is_recored;
}

#ifdef CFG_HBM_FLASH
STATIC int copy_ecc_data_to_config_data(struct multi_ecc_err_info_s *ecc_addr_info,
	struct ecc_config_header_s *ecc_header_info,
	uint8_t *ecc_config_data, uint32_t data_len, uint32_t dev_id)
{
	int ret;
	uint32_t info_write_offset;
	/* adapt different kernel version */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
	struct timespec64 stamp;
#else
	struct timeval stamp;
#endif
	/* calculate memcpy length */
	info_write_offset = sizeof(struct ecc_config_header_s) + ecc_header_info->length + dev_id * ECC_FLASH_DIE_SIZE;
	/* adapt different kernel version */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
	ktime_get_real_ts64(&stamp);
#else
	do_gettimeofday(&stamp);
#endif
	/* write multi ecc occurrence time to flash */
	ecc_addr_info->timestamp = stamp.tv_sec;
	ret = memcpy_s(ecc_config_data + info_write_offset, sizeof(struct multi_ecc_err_info_s),
				   (uint8_t *)ecc_addr_info, sizeof(struct multi_ecc_err_info_s));
	if (ret != OK) {
		memory_drv_err("Memcpy ecc_addr_info failed. (total_length=%u; copy_len=%lu, ret=%d)\n",
			ecc_header_info->length, sizeof(struct multi_ecc_err_info_s), ret);
		return ret;
	}
	ecc_header_info->length = ecc_header_info->length + sizeof(struct multi_ecc_err_info_s);
	return ret;
}
#else
STATIC int copy_ecc_data_to_config_data(struct multi_ecc_err_info_s *ecc_addr_info,
	struct ecc_config_header_s *ecc_header_info,
	uint8_t *ecc_config_data, unsigned int data_len, uint32_t dev_id)
{
	int ret;
	unsigned int used_space;
	unsigned int info_write_offset;
	unsigned int extern_start_write_offset;
	unsigned int extern_used_length;
	struct external_info additional = {0};
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
	struct timespec64 stamp;
#else
	struct timeval stamp;
#endif
	(void)dev_id;

	info_write_offset = sizeof(struct ecc_config_header_s) + ecc_header_info->length;
	extern_used_length = ecc_header_info->length / sizeof(struct multi_ecc_err_info_s) * sizeof(struct external_info);
	used_space = info_write_offset + extern_used_length + STATISTICS_INFO_SIZE;
	if ((used_space >= data_len) ||
		(data_len - used_space < (sizeof(struct multi_ecc_err_info_s) + sizeof(struct external_info)))) {
		memory_drv_err("the allocated flash space for record ECC phy_addr is full, data_len:%u, offset:%u, \
			additional length:%u\n", data_len, info_write_offset, extern_used_length);
		return -ENOMEM;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
	ktime_get_real_ts64(&stamp);
#else
	do_gettimeofday(&stamp);
#endif
	additional.timer_stamp = stamp.tv_sec;

	ret = memcpy_s(ecc_config_data + info_write_offset, data_len - used_space,
				   (uint8_t *)ecc_addr_info, sizeof(struct multi_ecc_err_info_s));
	if (ret != OK) {
		memory_drv_err("memcpy ecc_addr_info failed, total length:%u, used length:%u, copy len:%lu, ret: %d\n",
			data_len, used_space, sizeof(struct multi_ecc_err_info_s), ret);
		return ret;
	}

	/* write multi ecc occurrence time to flash */
	extern_start_write_offset = data_len - STATISTICS_INFO_SIZE - extern_used_length -
		sizeof(struct external_info);
	ret = memcpy_s(ecc_config_data + extern_start_write_offset, data_len - used_space -
		sizeof(struct multi_ecc_err_info_s), (uint8_t *)&additional, sizeof(struct external_info));
	if (ret != OK) {
		memory_drv_err("memcpy external information failed, total length:%u, used length:%u, copy len:%lu, ret:%d\n",
			data_len, used_space, sizeof(struct external_info), ret);
	}
	ecc_header_info->length = ecc_header_info->length + sizeof(struct multi_ecc_err_info_s);
	return ret;
}
#endif

STATIC int hbm_add_multi_ecc_event(struct multi_ecc_err_info_s *ecc_addr_info,
	enum HBM_MEMORY_ERROR_RECORD_TYPE error_type, uint32_t host_pid)
{
	int ret;
	uint32_t chipid, ecc_dev_id;
	struct new_multi_ecc_list *new_ecc_event = NULL;

	ret = memory_get_adapter()->get_chip_die(ecc_addr_info->physical_addr, &chipid, &ecc_dev_id);
	if (ret != OK) {
		memory_drv_err("parse devid to chipid & devid fail.ret=%d\n", ret);
		return ret;
	}
	if (ecc_dev_id < MAX_OS_DEVICE_COUNT) {
		new_ecc_event = (struct new_multi_ecc_list *)kzalloc(sizeof(struct new_multi_ecc_list),
			GFP_KERNEL | __GFP_ACCOUNT);
		if (new_ecc_event == NULL) {
			memory_drv_err("New ecc event kzalloc failed.\n");
			return -ENOMEM;
		}
		new_ecc_event->dev_id = ecc_dev_id;
		new_ecc_event->physical_addr = ecc_addr_info->physical_addr;
		new_ecc_event->error_type = error_type;
		new_ecc_event->occupied_pid = (pid_t)host_pid;
		if (hbm_add_ecc_event(new_ecc_event)) {
			memory_drv_warn("Add new multi ecc event failed. (dev_id=%u; phy_addr=0x%llx)\n",
				ecc_dev_id, ecc_addr_info->physical_addr);
			hbm_kfree((void **)&new_ecc_event);
		}
	}

	return OK;
}

STATIC int hbm_record_multi_ecc_addr_info(struct multi_ecc_err_info_s *ecc_addr_info,
	struct ecc_config_header_s *ecc_header_info,
	struct ecc_isolated_statistics_record *statistics_record,
	uint8_t *ecc_config_data, unsigned int data_len)
{
	int ret = OK;
	int is_addr_recorded;
	uint32_t chipid, dev_id;
#ifdef CFG_HBM_FLASH
	uint32_t dev_num = 0;
#endif
	ret = memory_get_adapter()->get_chip_die(ecc_addr_info->physical_addr, &chipid, &dev_id);
	if (ret != OK) {
		memory_drv_err("parse devid to chipid & devid fail.ret=%d\n", ret);
		return ret;
	}
	if (memory_get_adapter()->filter_addr != NULL) {
		if (memory_get_adapter()->filter_addr(dev_id, ecc_addr_info->physical_addr) == 0) {
			memory_drv_event("phy addr 0x%llx has been filtered.\n", ecc_addr_info->physical_addr);
			return 0;
		}
	}
#ifdef CFG_HBM_FLASH
	is_addr_recorded = hbm_check_new_ecc_addr_recored(*ecc_addr_info, (struct multi_ecc_err_info_s *)
		(ecc_config_data + sizeof(struct ecc_config_header_s) + dev_id * ECC_FLASH_DIE_SIZE), ecc_header_info->length);
#else
	is_addr_recorded = hbm_check_new_ecc_addr_recored(*ecc_addr_info, (struct multi_ecc_err_info_s *)
		(ecc_config_data + sizeof(struct ecc_config_header_s)), ecc_header_info->length);
#endif
	if (is_addr_recorded != ECC_ADDR_INFO_RECORDED) {
		memory_drv_event("Record new multi ECC address.\n");
		/* only record multiple Bits ECC error address */
		ret = copy_ecc_data_to_config_data(ecc_addr_info, ecc_header_info, ecc_config_data, data_len, dev_id);
		if (ret != OK) {
			memory_drv_err("copy date to global memory failed, total len:%u, current length:%u, ret: %d\n",
				data_len, ecc_header_info->length, ret);
			return ret;
		}

		if (ecc_addr_info->module == ECC_MODULE_DDR) {
#ifndef CFG_HBM_FLASH
			statistics_record->ddr_multi_isolated_page_count = statistics_record->ddr_multi_isolated_page_count +
															   MULTI_ECC_INTERRUPT_COUNT;
#endif
		} else {
			statistics_record->hbm_multi_isolated_page_count = statistics_record->hbm_multi_isolated_page_count +
															   MULTI_ECC_INTERRUPT_COUNT;
		}
		g_ecc_config_tool->pending[dev_id] = 1;

#ifdef CFG_HBM_FLASH
		hbm_update_ecc_aggregate_record(dev_id);
		ret = hbm_write_ecc_config_data(dev_id);
		if (ret) {
			memory_drv_err("Handle multiple ecc info write flash failed. (ret=%d; dev_id=%u)\n", ret, dev_id);
			return ret;
		}
		/* get device number */
		ret = devdrv_get_devnum(&dev_num);
		if (ret != OK) {
			memory_drv_err("Get dev_num failed. (ret=%d)\n", ret);
			return -EINVAL;
		}
		hbm_check_statistic_to_bbox(ecc_config_data, dev_num, STATISTICS_INFO_FLASH_OFFSET);
#else
		ret = hbm_write_ecc_config_data();
		if (ret) {
			memory_drv_err("handle multiple ecc info write flash failed, ret:%d\n", ret);
			return ret;
		}
		hbm_check_statistic_to_bbox((struct ecc_isolated_statistics_record *)(ecc_config_data +
			ecc_header_info->statistics_info_offset), MAX_OS_DEVICE_COUNT);
#endif
	}
	return ret;
}

STATIC bool hbm_is_in_unisolated_pfns(unsigned int dev_id, unsigned long new_pfn)
{
	struct pfn_list *pos = NULL;
	struct pfn_list *n = NULL;

	list_for_each_entry_safe(pos, n, &g_ecc_config_tool->un_online_isolated_pfn_queue[dev_id].pfns.list, list) {
		if (new_pfn == pos->pfn) {
			return true;
		}
	}

	return false;
}

#ifdef CFG_HBM_FAULT_EVENT
STATIC int32_t hbm_add_unisolated_multi_ecc_event(unsigned long ecc_pfn, uint32_t ecc_dev_id)
{
	struct pfn_list *new_pfn = NULL;

	mutex_lock(&g_ecc_config_tool->un_online_isolated_pfn_queue[ecc_dev_id].mutex);
	if (!hbm_is_in_unisolated_pfns(ecc_dev_id, ecc_pfn)) {
		new_pfn = (struct pfn_list *)kzalloc(sizeof(struct pfn_list), GFP_KERNEL | __GFP_ACCOUNT);
		if (new_pfn == NULL) {
			mutex_unlock(&g_ecc_config_tool->un_online_isolated_pfn_queue[ecc_dev_id].mutex);
			memory_drv_err("New pfn kzalloc failed.\n");
			return -ENOMEM;
		};

		new_pfn->pfn = ecc_pfn;
		list_add(&new_pfn->list, &g_ecc_config_tool->un_online_isolated_pfn_queue[ecc_dev_id].pfns.list);
		g_ecc_config_tool->un_online_isolated_pfn_queue[ecc_dev_id].pfn_num++;
	}
	mutex_unlock(&g_ecc_config_tool->un_online_isolated_pfn_queue[ecc_dev_id].mutex);

	return 0;
}
#endif

STATIC int hbm_judge_ecc_isolated(struct multi_ecc_err_info_s *ecc_addr_info)
{
	int ret;
#ifndef CFG_HBM_FAULT_EVENT
	uint32_t occpied_dev_id, vf_id;
	enum devdrv_process_type cp_type;
	int ecc_pid_num = 0;
	pid_t *ecc_pids = NULL;
	struct pfn_list *new_pfn = NULL;
#endif
	uint32_t host_pid = 0;
	unsigned long ecc_pfn = __phys_to_pfn(ecc_addr_info->physical_addr);
	uint32_t chipid, ecc_dev_id;

	if (ecc_addr_info->type != ECC_TYPE_MULTI_BIT) {
		memory_drv_info("currect ecc is not multi bit ecc\n");
		return 0;
	}

	ret = memory_get_adapter()->get_chip_die(ecc_addr_info->physical_addr, &chipid, &ecc_dev_id);
	if (ret != OK) {
		memory_drv_err("parse devid to chipid & devid fail.ret=%d\n", ret);
		return ret;
	}

	ret = memory_get_adapter()->check_mirror_addr(ecc_addr_info->physical_addr, ecc_dev_id);
	if (ret != 0) {
		memory_drv_info("current multi bit addr is within mirror addr. (physical_addr=%llx)\n",
			ecc_addr_info->physical_addr);
		return 0;
	}

#ifndef CFG_HBM_FAULT_EVENT
	ecc_pids = collect_process_by_pfn(ecc_pfn, &ecc_pid_num);
	if (ecc_pids != NULL) {
		int ecc_pid_i;
		mutex_lock(&g_ecc_config_tool->un_online_isolated_pfn_queue[ecc_dev_id].mutex);
		if (!hbm_is_in_unisolated_pfns(ecc_dev_id, ecc_pfn)) {
			new_pfn = (struct pfn_list *)kzalloc(sizeof(struct pfn_list), GFP_KERNEL | __GFP_ACCOUNT);
			if (new_pfn == NULL) {
				mutex_unlock(&g_ecc_config_tool->un_online_isolated_pfn_queue[ecc_dev_id].mutex);
				memory_drv_err("New pfn kzalloc failed.\n");
				return -ENOMEM;
			};

			new_pfn->pfn = ecc_pfn;
			list_add(&new_pfn->list, &g_ecc_config_tool->un_online_isolated_pfn_queue[ecc_dev_id].pfns.list);
			g_ecc_config_tool->un_online_isolated_pfn_queue[ecc_dev_id].pfn_num++;
		}
		mutex_unlock(&g_ecc_config_tool->un_online_isolated_pfn_queue[ecc_dev_id].mutex);

		for (ecc_pid_i = 0; ecc_pid_i < ecc_pid_num; ecc_pid_i++) {
			ret = devdrv_query_process_host_pid(ecc_pids[ecc_pid_i], &occpied_dev_id, &vf_id, &host_pid, &cp_type);
			if (ret) {
				memory_drv_warn("Get host pid failed. (dev_pid=%d; ret=%d)\n", ecc_pids[ecc_pid_i], ret);
				continue;
			}
		}
		hbm_kfree((void **)&ecc_pids);
	}
#else
	ret = hbm_add_unisolated_multi_ecc_event(ecc_pfn, ecc_dev_id);
	if (ret) {
		memory_drv_err("Add unisolated multi bit ecc event failed. (ret=%d)\n", ret);
		return ret;
	}
#endif

	ret = hbm_add_multi_ecc_event(ecc_addr_info, HBM_NEW_MULTI_BITS_ECC, host_pid);
	if (ret) {
		memory_drv_err("Add multi ecc event failed. (ret=%d)\n", ret);
		return ret;
	}
	return 0;
}

STATIC int handle_ecc_type_info(struct multi_ecc_err_info_s *ecc_addr_info,
								struct ecc_config_header_s *ecc_header_info,
								uint8_t *ecc_config_data, unsigned int data_len)
{
	int ret = OK;
	uint32_t chipid, devid;
	struct ecc_isolated_statistics_record *statistics_record = NULL;

	ret = memory_get_adapter()->get_chip_die(ecc_addr_info->physical_addr, &chipid, &devid);
	if (ret != OK) {
		memory_drv_err("parse devid to chipid & dieid fail.ret=%d\n", ret);
		return ret;
	}
#ifdef CFG_HBM_FLASH
	statistics_record = (struct ecc_isolated_statistics_record *)(ecc_config_data + devid * ECC_FLASH_DIE_SIZE +
		STATISTICS_INFO_FLASH_OFFSET);
#else
	statistics_record = (struct ecc_isolated_statistics_record *)(ecc_config_data +
						ecc_header_info->statistics_info_offset % data_len);
	statistics_record = statistics_record + devid;
#endif
	ret = hbm_judge_ecc_isolated(ecc_addr_info);
	if (ret) {
		memory_drv_err("Judge multiple ecc isolated failed, ret:%d\n", ret);
	}

	if (ecc_header_info->length >= (MAX_RECORD_ECC_ADDR_COUNT * sizeof(struct multi_ecc_err_info_s))) {
		memory_drv_err("The recorded multi bit ECC address has reached the maximum value %d.\n",
			MAX_RECORD_ECC_ADDR_COUNT);
		memory_drv_err("The current length is %d.\n", ecc_header_info->length);
		return -ENOSPC;
	}

	ret = hbm_record_multi_ecc_addr_info(ecc_addr_info, ecc_header_info, statistics_record, ecc_config_data,
		data_len);
	if (ret) {
		memory_drv_err("Record multiple ecc_addr_info failed, ret:%d\n", ret);
	}

	return ret;
}

STATIC void hbm_read_err_addr(struct cper_sec_mem_err *mem_err,
	struct multi_ecc_err_info_s *err_info)
{
	if (mem_err->validation_bits & CPER_MEM_VALID_PA) {
		err_info->physical_addr = mem_err->physical_addr;
	}

	if (mem_err->validation_bits & CPER_MEM_VALID_RANK_NUMBER)
		err_info->rank = mem_err->rank;

	if (mem_err->validation_bits & CPER_MEM_VALID_MODULE)
		err_info->module_id = mem_err->module;

	if (mem_err->validation_bits & CPER_MEM_VALID_BANK) {
		err_info->bank = mem_err->bank;
	}

	if (mem_err->validation_bits & CPER_MEM_VALID_ROW) {
		err_info->row = mem_err->row;
	}

	if (mem_err->validation_bits & CPER_MEM_VALID_COLUMN) {
		err_info->column = mem_err->column;
	}

	return;
}

STATIC void hbm_read_err_module(struct cper_sec_mem_err *mem_err,
	struct multi_ecc_err_info_s *err_info)
{
	if (mem_err->validation_bits & CPER_MEM_VALID_MODULE) {
		if ((mem_err->module == ECC_MODULE_DDRC0) ||
			(mem_err->module == ECC_MODULE_DDRC1)) {
			err_info->module = ECC_MODULE_DDR;
		} else if ((mem_err->module >= ECC_MODULE_HBM_MIN) &&
			(mem_err->module <= ECC_MODULE_HBM_MAX)) {
			err_info->module = ECC_MODULE_HBM;
		} else {
			err_info->module = ECC_MODULE_UNKNOWN;
		}
	}

	return;
}

STATIC void hbm_read_err_type(struct cper_sec_mem_err *mem_err, struct multi_ecc_err_info_s *err_info)
{
	if (mem_err->validation_bits & CPER_MEM_VALID_ERROR_TYPE) {
		switch (mem_err->error_type) {
			case STANDARD_ERR_TYPE_SINGLE_BIT_ECC:
			case STANDARD_ERR_TYPE_SINGLE_BIT_ECC_THOLD:
			case STANDARD_ERR_TYPE_SCRUB_CORRECT:
			case HISI_ERR_TYPE_HA_CORRECT:
			case HISI_ERR_TYPE_VLS_CORRECT:
			case HISI_ERR_TYPE_RVLS_CORRECT:
				err_info->type = ECC_TYPE_SINGLE_BIT;
				break;

			case STANDARD_ERR_TYPE_MULTI_BIT_ECC:
			case STANDARD_ERR_TYPE_MEMORY_SPARING:
			case HISI_ERR_TYPE_HA_UNCORRECT:
			case HISI_ERR_TYPE_VLS_UNCORRECT:
			case HISI_ERR_TYPE_RVLS_UNCORRECT:
				err_info->type = ECC_TYPE_MULTI_BIT;
				break;

			case STANDARD_ERR_TYPE_SCRUB_UNCORRECT:
#ifdef CFG_HBM_FAULT_EVENT
			case STANDARD_ERR_TYPE_DSCRUB_UNCORRECT:
#endif
				err_info->type = ECC_TYPE_SCRUB_MULTI_BIT;
				break;

			default:
				err_info->type = ECC_TYPE_UNKNOWN;
				break;
		}
	}

	return;
}

STATIC int hbm_translate_mem_err(struct cper_sec_mem_err *mem_err, struct multi_ecc_err_info_s *ecc_err_info)
{
	/* get ecc error type, only record ecc error info */
	hbm_read_err_type(mem_err, ecc_err_info);
	if ((ecc_err_info->type != ECC_TYPE_MULTI_BIT) && (ecc_err_info->type != ECC_TYPE_SCRUB_MULTI_BIT)) {
		memory_drv_err("ECC type is not multiple bit error. (type=%u; valid_bits=0x%llx)\n",
			ecc_err_info->type, mem_err->validation_bits);
		return -ENOMSG;
	}

	/* get ecc error module, only record HBM module */
	hbm_read_err_module(mem_err, ecc_err_info);
	if (ecc_err_info->module != ECC_MODULE_HBM) {
		memory_drv_err("ECC module is not hbm. (module=%u)\n", ecc_err_info->module);
		return -ENOMSG;
	}

	hbm_read_err_addr(mem_err, ecc_err_info);
	if (ecc_err_info->physical_addr == 0x0) {
		memory_drv_err("ECC physical address is invalid.\n");
		return -EINVAL;
	}
	return OK;
}

int hbm_multi_ecc_record(struct cper_sec_mem_err *mem_err)
{
	int ret;
	uint32_t chipid, dev_id, fifo_size;
	struct multi_ecc_err_info_s ecc_err_info = {0};

	memory_drv_info("Enter memory ECC info record.\n");
	ret = hbm_translate_mem_err(mem_err, &ecc_err_info);
	if (ret) {
		memory_drv_err("Translate memory error info failed. (ret=%d)\n", ret);
		return ret;
	}

	ret = memory_get_adapter()->get_chip_die(ecc_err_info.physical_addr, &chipid, &dev_id);
	if (ret != OK) {
		memory_drv_err("parse devid to chipid & devid fail.ret=%d\n", ret);
		return ret;
	}
	if (dev_id >= MAX_OS_DEVICE_COUNT) {
		memory_drv_err("ECC physical address(0x%llx) is out of range, dev_id=%u\n",
					   ecc_err_info.physical_addr, dev_id);
		return -EINVAL;
	}

	fifo_size = kfifo_put(&(g_ecc_config_tool->reported_multi_ecc_queue), ecc_err_info);
	if (fifo_size == 0) {
		memory_drv_err("Reported multi ECC buff is full.\n");
		return -ENOMEM;
	}

	(void)schedule_work(&g_ecc_config_tool->ecc_handle_task);
	return OK;
}

STATIC void hbm_err_report_task(struct work_struct *statistics_task)
{
	uint8_t *ecc_config_data = NULL;
	struct multi_ecc_err_info_s ecc_err_info = {0};
	struct ecc_config_header_s *ecc_header_info = NULL;
#ifdef CFG_HBM_FLASH
	uint32_t dev_num;
	uint32_t chipid, dev_id;

	/* get dev_num */
	if (devdrv_get_devnum(&dev_num) != 0) {
		memory_drv_err("Get dev_num failed.\n");
		return;
	}
#endif
	while (!kfifo_is_empty(&g_ecc_config_tool->reported_multi_ecc_queue)) {
		int ret = kfifo_get(&(g_ecc_config_tool->reported_multi_ecc_queue), &ecc_err_info);
		if (ret == 0) {
			memory_drv_err("Get reported multi ECC info failed. (ret=%d)\n", ret);
			return;
		}

		memory_drv_event("physical_addr=0x%llx, rank=0x%x, type=0x%x, module=0x%x, module_id=0x%x.\n",
			ecc_err_info.physical_addr, ecc_err_info.rank, ecc_err_info.type,
			ecc_err_info.module, ecc_err_info.module_id);
		memory_drv_event("bank=0x%x, row=0x%x, column=0x%x.\n", ecc_err_info.bank, ecc_err_info.row,
			ecc_err_info.column);
#ifdef CFG_HBM_FLASH
		ret = memory_get_adapter()->get_chip_die(ecc_err_info.physical_addr, &chipid, &dev_id);
		if (ret != OK) {
			memory_drv_err("parse devid to chipid & devid fail.ret=%d\n", ret);
			continue;
		}
		if (dev_id > dev_num) {
			memory_drv_err("Physical addr is invalid. (dev_id=%u; dev_num=%u)\n", dev_id, dev_num);
			return;
		}
#endif
		mutex_lock(&g_ecc_config_tool->record_lock);
		ecc_config_data = g_ecc_config_tool->ecc_config_data;
#ifdef CFG_HBM_FLASH
		ecc_header_info = (struct ecc_config_header_s *)(ecc_config_data + dev_id *ECC_FLASH_DIE_SIZE);
#else
		ecc_header_info = (struct ecc_config_header_s *)ecc_config_data;
#endif
		ret = handle_ecc_type_info(&ecc_err_info, ecc_header_info, ecc_config_data, FLASH_ERASE_BLOCK_SIZE);
		mutex_unlock(&g_ecc_config_tool->record_lock);
		if (ret) {
			memory_drv_err("Handle ecc info failed. (ret=%d)\n", ret);
		}
	}
	memory_drv_info("exit ecc_config succeed.\n");
}

STATIC int hbm_read_multi_ecc_info(struct ecc_config_udata_s *udata)
{
	int ret;
	uint32_t chipid, dev_id;
	uint32_t recorded_count, dev_recorded_count;
	uint32_t current_index = 0;
	uint32_t recorded_i = 0;
#ifndef CFG_HBM_FLASH
	struct external_info *time_info = NULL;
#endif
	struct multi_ecc_err_info_s *ecc_err_info = NULL;
	struct ecc_config_header_s *ecc_header_info = NULL;
	struct hbm_hw_addr_s hbm_hw_addr;

	mutex_lock(&g_ecc_config_tool->record_lock);
#ifdef CFG_HBM_FLASH
	ecc_header_info = (struct ecc_config_header_s *)(g_ecc_config_tool->ecc_config_data +
		udata->dev_id * ECC_FLASH_DIE_SIZE);	 /* update ecc_header_info by dev_id */
#else
	ecc_header_info = (struct ecc_config_header_s *)g_ecc_config_tool->ecc_config_data;
#endif
	dev_recorded_count = hbm_get_current_multi_ecc_record_count(udata->dev_id);
	if (udata->data_index >= dev_recorded_count) {
		mutex_unlock(&g_ecc_config_tool->record_lock);
		memory_drv_err("Get multi invalid ecc info. (dev_id=%u; index=%u; dev_recorded_count=%u)\n",
			udata->dev_id, udata->data_index, dev_recorded_count);
		return -EINVAL;
	}

	recorded_count = ecc_header_info->length / sizeof(struct multi_ecc_err_info_s);
	for (recorded_i = 0; recorded_i < recorded_count; recorded_i++) {
#ifdef CFG_HBM_FLASH
		ecc_err_info = (struct multi_ecc_err_info_s *)(g_ecc_config_tool->ecc_config_data +
			udata->dev_id * ECC_FLASH_DIE_SIZE + sizeof(struct ecc_config_header_s) +
			recorded_i * sizeof(struct multi_ecc_err_info_s));
#else
		ecc_err_info = (struct multi_ecc_err_info_s *)(g_ecc_config_tool->ecc_config_data +
			(sizeof(struct ecc_config_header_s) + recorded_i * sizeof(struct multi_ecc_err_info_s)) %
			FLASH_ERASE_BLOCK_SIZE);
#endif
		ret = memory_get_adapter()->get_chip_die(ecc_err_info->physical_addr, &chipid, &dev_id);
		if (ret != OK) {
			memory_drv_err("parse devid to chipid & devid fail.ret=%d\n", ret);
			continue;
		}
		if (dev_id != udata->dev_id) {
			continue;
		}
		if (current_index != udata->data_index) {
			current_index++;
			continue;
		}
		if (memory_get_adapter()->get_hbm_hwaddr != NULL) {
			memory_get_adapter()->get_hbm_hwaddr(udata->dev_id, ecc_err_info->physical_addr, &hbm_hw_addr);
			udata->multi_ecc_data.rank = hbm_hw_addr.sid;
			udata->multi_ecc_data.module_id = hbm_hw_addr.module_id;
			udata->multi_ecc_data.bank = hbm_hw_addr.bank;
			udata->multi_ecc_data.row = hbm_hw_addr.row;
			udata->multi_ecc_data.column = hbm_hw_addr.col;
		} else {
#ifndef GTEST
			udata->multi_ecc_data.physical_addr = ecc_err_info->physical_addr;
			udata->multi_ecc_data.rank = ecc_err_info->rank;
			udata->multi_ecc_data.module_id = ecc_err_info->module_id;
			udata->multi_ecc_data.bank = ecc_err_info->bank;
			udata->multi_ecc_data.row = ecc_err_info->row;
			udata->multi_ecc_data.column = ecc_err_info->column;
#endif
		}
		udata->multi_ecc_data.type = ecc_err_info->type;
		udata->multi_ecc_data.module = ecc_err_info->module;

		/* multi ECC occurrence time is recorded in reverse order */
#ifdef CFG_HBM_FLASH
		udata->multi_ecc_data.timer_stamp = (int)ecc_err_info->timestamp;
#else
		time_info = (struct external_info *)(g_ecc_config_tool->ecc_config_data +
			ecc_header_info->statistics_info_offset - sizeof(struct external_info) * (recorded_i + 1));
		if ((~time_info->timer_stamp) == 0) {
			time_info->timer_stamp = 0;
		}
		udata->multi_ecc_data.timer_stamp = (int)time_info->timer_stamp;
#endif
		break;
	}
	mutex_unlock(&g_ecc_config_tool->record_lock);
	return OK;
}

STATIC int hbm_read_single_ecc_info(struct ecc_config_udata_s *udata)
{
	int ret;
	struct single_ecc_record_s single_ecc_record_data = {0};

	ret = hbm_get_one_single_ecc_info(udata->dev_id, udata->data_index,
		&single_ecc_record_data);
	if (ret) {
		memory_drv_err("Get one single ECC info failed. (dev_id=%u, index=%u, ret=%d)\n",
			udata->dev_id, udata->data_index, ret);
		return ret;
	}

	ret = memcpy_s((uint8_t *)&(udata->single_ecc_data), sizeof(struct single_ecc_data_s),
		(uint8_t *)&single_ecc_record_data, sizeof(struct single_ecc_data_s));
	if (ret) {
		memory_drv_err("Memcpy single ECC data record failed. (dev_id=%u, index=%u, ret=%d)\n",
			udata->dev_id, udata->data_index, ret);
	}
	return ret;
}

STATIC int hbm_read_ecc_addr_count(struct ecc_config_udata_s *udata)
{
	wait_single_ecc_data_resource(udata->dev_id);
	udata->ecc_address_count.single_ecc_addr_cnt = hbm_get_single_ecc_addr_count(udata->dev_id);
	release_single_ecc_data_resource(udata->dev_id);
	mutex_lock(&g_ecc_config_tool->record_lock);
	udata->ecc_address_count.multi_ecc_addr_cnt = hbm_get_current_multi_ecc_record_count(udata->dev_id);
	mutex_unlock(&g_ecc_config_tool->record_lock);
	return OK;
}


STATIC int hbm_read_pending_flag(struct ecc_config_udata_s *udata)
{
	mutex_lock(&g_ecc_config_tool->record_lock);
	udata->pending = g_ecc_config_tool->pending[udata->dev_id];
	mutex_unlock(&g_ecc_config_tool->record_lock);
	return OK;
}

int hbm_get_ecc_record_info(void *feature, char *in, uint32_t in_len, char *out, uint32_t out_len)
{
	int ret;
	uint32_t dev_num = 0;
	struct ecc_config_udata_s udata = {0};

	int (*read_handle[])(struct ecc_config_udata_s *) = {
		hbm_read_multi_ecc_time_info,
		hbm_read_single_ecc_info,
		hbm_read_multi_ecc_info,
		hbm_read_ecc_addr_count,
		hbm_read_pending_flag,
	};

	if (in == NULL) {
		memory_drv_err("null pointer of input data.\n");
		return -EINVAL;
	}

	if (in_len != sizeof(struct ecc_config_udata_s)) {
		memory_drv_err("invalid input data size, size = %u which should be %lu\n",
			in_len, sizeof(struct ecc_config_udata_s));
		return -EINVAL;
	}

	ret = memcpy_s((void *)&udata, sizeof(struct ecc_config_udata_s), (void *)in, sizeof(struct ecc_config_udata_s));
	if (ret) {
		memory_drv_err("Call memcpy_s failed. (ret=%d)\n", ret);
		return ret;
	}

	ret = devdrv_get_devnum(&dev_num);
	if (ret != 0) {
		memory_drv_err("Get dev_num failed. (ret=%d)\n", ret);
		return ret;
	}

	if (udata.dev_id >= dev_num) {
		memory_drv_err("Physical id is invalid. (dev_id=%u)\n", udata.dev_id);
		return -EINVAL;
	}

	if (udata.op_type >= ECC_MAX_READ_CMD) {
		memory_drv_err("Ecc config unknown op cmd. (op_type=%u)\n", udata.op_type);
		return -EPERM;
	}

	ret = read_handle[udata.op_type](&udata);
	if (ret) {
		memory_drv_err("Ecc config handle failed. (op_type=%u, ret=%d)\n", udata.op_type, ret);
		return ret;
	}

	ret = memcpy_s((void *)out, out_len, (void *)&udata, sizeof(struct ecc_config_udata_s));
	if (ret) {
		memory_drv_err("Call memcpy_s failed. (out_len=%u, ret=%d, copy_size=%lu)\n", out_len, ret,
			sizeof(struct ecc_config_udata_s));
	}
	return ret;
}

STATIC void hbm_isolated_pfn_queue_free(void)
{
	int dev_id;
	struct pfn_list *pfn_pos = NULL;
	struct pfn_list *pfn_n = NULL;

	for (dev_id = 0; dev_id < MAX_OS_DEVICE_COUNT; dev_id++) {
		mutex_lock(&g_ecc_config_tool->un_online_isolated_pfn_queue[dev_id].mutex);
		list_for_each_entry_safe(pfn_pos, pfn_n, &g_ecc_config_tool->un_online_isolated_pfn_queue[dev_id].pfns.list,
			list) {
			list_del(&pfn_pos->list);
			hbm_kfree((void **)&pfn_pos);
			g_ecc_config_tool->un_online_isolated_pfn_queue[dev_id].pfn_num--;
		}
		mutex_unlock(&g_ecc_config_tool->un_online_isolated_pfn_queue[dev_id].mutex);

		mutex_lock(&g_ecc_config_tool->online_isolated_pfn_queue[dev_id].mutex);
		list_for_each_entry_safe(pfn_pos, pfn_n, &g_ecc_config_tool->online_isolated_pfn_queue[dev_id].pfns.list,
			list) {
			list_del(&pfn_pos->list);
			hbm_kfree((void **)&pfn_pos);
			g_ecc_config_tool->online_isolated_pfn_queue[dev_id].pfn_num--;
		}
		mutex_unlock(&g_ecc_config_tool->online_isolated_pfn_queue[dev_id].mutex);

		mutex_destroy(&g_ecc_config_tool->online_isolated_pfn_queue[dev_id].mutex);
		mutex_destroy(&g_ecc_config_tool->un_online_isolated_pfn_queue[dev_id].mutex);
	}
}

STATIC void hbm_clear_tool(void)
{
	struct new_multi_ecc_list *pos = NULL;
	struct new_multi_ecc_list *n = NULL;

	if (g_ecc_config_tool != NULL) {
		if (g_ecc_config_tool->statistics_timer.function) {
			(void)del_timer_sync(&g_ecc_config_tool->statistics_timer);
		}
		if (g_ecc_config_tool->statistics_task.func) {
			(void)cancel_work_sync(&g_ecc_config_tool->statistics_task);
		}
		if (g_ecc_config_tool->ecc_handle_task.func) {
			(void)cancel_work_sync(&g_ecc_config_tool->ecc_handle_task);
		}
		mutex_lock(&g_ecc_config_tool->record_lock);
		if (g_ecc_config_tool->ecc_config_data != NULL) {
			hbm_vfree((void **)&g_ecc_config_tool->ecc_config_data);
		}
		mutex_unlock(&g_ecc_config_tool->record_lock);

		mutex_lock(&g_ecc_config_tool->multi_ecc_event_queue.mutex);
		list_for_each_entry_safe(pos, n, &g_ecc_config_tool->multi_ecc_event_queue.ecc_list.list, list) {
			list_del(&pos->list);
			hbm_kfree((void **)&pos);
			g_ecc_config_tool->multi_ecc_event_queue.error_num--;
		}
		mutex_unlock(&g_ecc_config_tool->multi_ecc_event_queue.mutex);

		hbm_isolated_pfn_queue_free();
		mutex_destroy(&g_ecc_config_tool->multi_ecc_event_queue.mutex);
		mutex_destroy(&g_ecc_config_tool->record_lock);
		hbm_kfree((void **)&g_ecc_config_tool);
	}
}

STATIC void hbm_isolated_pfn_queue_init(void)
{
	int dev_id;
	for (dev_id = 0; dev_id < MAX_OS_DEVICE_COUNT; dev_id++) {
		mutex_init(&g_ecc_config_tool->un_online_isolated_pfn_queue[dev_id].mutex);
		mutex_init(&g_ecc_config_tool->online_isolated_pfn_queue[dev_id].mutex);
		INIT_LIST_HEAD(&g_ecc_config_tool->un_online_isolated_pfn_queue[dev_id].pfns.list);
		INIT_LIST_HEAD(&g_ecc_config_tool->online_isolated_pfn_queue[dev_id].pfns.list);
	}
}

int hbm_ecc_isolation_init(void)
{
	int ret;
	uint32_t dev_num = 0;
	uint32_t dev_id;
#ifdef CFG_HBM_FLASH
	/* get device number */
	ret = devdrv_get_devnum(&dev_num);
	if (ret != OK) {
		memory_drv_err("Get dev_num failed. (ret=%d)\n", ret);
		return -EINVAL;
	}
#endif

	ret = memory_get_adapter()->get_mirror_cfg(dev_num);
	if (ret != 0) {
		memory_drv_err("get hbm mirror cfg failed. (ret=%d)\n", ret);
		return ret;
	}

	g_ecc_config_tool = (struct ecc_config_tool_s *)kzalloc(sizeof(struct ecc_config_tool_s),
		GFP_KERNEL | __GFP_ACCOUNT);
	if (g_ecc_config_tool == NULL) {
		memory_drv_err("Global ECC config data kzalloc failed.\n");
		ret = -ENOMEM;
		goto FAIL;
	}

	g_ecc_config_tool->ecc_config_data = (uint8_t *)vmalloc(FLASH_ERASE_BLOCK_SIZE);
	if (g_ecc_config_tool->ecc_config_data == NULL) {
		memory_drv_err("Alloc ecc_config data failed.\n");
		hbm_kfree((void **)&g_ecc_config_tool);
		ret = -ENOMEM;
		goto FAIL;
	}

	mutex_init(&g_ecc_config_tool->multi_ecc_event_queue.mutex);
	INIT_LIST_HEAD(&g_ecc_config_tool->multi_ecc_event_queue.ecc_list.list);

	hbm_isolated_pfn_queue_init();

	INIT_KFIFO(g_ecc_config_tool->reported_multi_ecc_queue);
	mutex_init(&g_ecc_config_tool->record_lock);
	for (dev_id = 0; dev_id < MAX_OS_DEVICE_COUNT; dev_id++)
		g_ecc_config_tool->pending[dev_id] = 0;

#ifdef CFG_HBM_FLASH
	ret = hbm_read_sec_ecc_config_data();
	if (ret) {
		memory_drv_err("read ECC config data failed. (ret=%d)\n", ret);
		goto FREE_TOOL;
	}

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		ret = hbm_init_memory_data(dev_id);
		if (ret != OK) {
			memory_drv_err("Init ECC config data failed. (ret=%d; dev_id=%u)\n", ret, dev_id);
			goto FREE_TOOL;
		}
	}
	hbm_check_statistic_to_bbox(g_ecc_config_tool->ecc_config_data, dev_num, STATISTICS_INFO_FLASH_OFFSET);
#else
	ret = hbm_init_memory_data();
	if (ret) {
		memory_drv_err("Init ECC config data failed. (ret=%d)\n", ret);
		goto FREE_TOOL;
	}
	hbm_check_statistic_to_bbox(g_ecc_config_tool->ecc_statistics_info, MAX_OS_DEVICE_COUNT);
#endif
	ret = hbm_init_single_ecc_info();
	if (ret) {
		memory_drv_err("Init single ECC config data failed. (ret=%d)\n", ret);
		goto FREE_TOOL;
	}
	if (memory_get_adapter()->hbm_init_trans_info != NULL)
		memory_get_adapter()->hbm_init_trans_info();

	timer_setup(&(g_ecc_config_tool->statistics_timer), hbm_statistics_timer, 0);
	INIT_WORK(&(g_ecc_config_tool->statistics_task), statistics_update_task);
	INIT_WORK(&(g_ecc_config_tool->ecc_handle_task), hbm_err_report_task);
	(void)mod_timer(&(g_ecc_config_tool->statistics_timer), jiffies + PREIOD_STATISTIC_TIME * HZ);
	CALL_INIT_MODULE(DRV_MEMORY_MODULE_BASIC_INFO);
	memory_drv_info("hbm ecc isolation driver init finish.\n");
	return OK;

FREE_TOOL:
	hbm_clear_tool();
FAIL:
	hbm_mirror_cfg_iounmap();

	return ret;
}

void hbm_ecc_isolation_exit(void)
{
	CALL_EXIT_MODULE(DRV_MEMORY_MODULE_BASIC_INFO);
	hbm_clear_tool();
	hbm_free_single_ecc_info();
	hbm_mirror_cfg_iounmap();
	memory_drv_info("hbm ecc isolation driver exit finish.\n");
}
