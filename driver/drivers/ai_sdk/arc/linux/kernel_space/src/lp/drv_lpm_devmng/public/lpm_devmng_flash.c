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

#include <linux/of.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/atomic.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include "ascend_kernel_hal.h"
#include "lpm_devmng_flash.h"

#define LPM_DEVMNG_FLASH_ITEM_NAME         "lpm_devmng" // should not large than 32
// thread status
#define LPM_DEVMNG_FLASH_THREAD_RUN        1
#define LPM_DEVMNG_FLASH_THREAD_STOP       0

// 4K flash data read and write can be completed within 1s
// wait up to 1.5s for fault tolerance
#define LPM_DEVMNG_FLASH_TASK_WAIT_TIME_MS 10
#define LPM_DEVMNG_FLASH_TASK_WAIT_CNT     150 // wait up to 100 times

#define LPM_DEVMNG_FLASH_RETRY_WAIT_TIME_MS 1
#define LPM_DEVMNG_FLASH_READ_RETRY_CNT     3
#define LPM_DEVMNG_FLASH_WRITE_RETRY_CNT    3


STATIC struct lpm_flash_priv *lpm_flash_priv_info(void)
{
	static struct lpm_flash_priv lpm_flash_priv = {0};
	return &lpm_flash_priv;
}

STATIC int32_t lpm_flash_init_item_config(void)
{
	uint32_t offset = 0;
	uint32_t item_id;
	struct lpm_flash_priv *flash_priv = lpm_flash_priv_info();
	uint32_t item_len[(uint32_t)LPM_FLASH_ITEM_MAX] = {
		sizeof(struct lpm_flash_profile_data)
	};

	for (item_id = 0; item_id < (uint32_t)LPM_FLASH_ITEM_MAX; item_id++) {
		flash_priv->item_info.item_cfg[item_id].offset = offset;
		flash_priv->item_info.item_cfg[item_id].len = item_len[item_id];
		offset += item_len[item_id];
	}

	// the total length of items should not large than the size of the reserved flash
	if (offset > LPM_DEVMNG_FLASH_MAX_LEN) {
		lpm_log_err("total len of flash item should not large than %u\n", offset);
		return -1;
	}
	return 0;
}

STATIC int32_t lpm_flash_get_item_data(uint32_t dev_id,
	enum lpm_flash_item_type item_type, uint8_t *data, uint32_t data_len)
{
	int32_t ret;
	struct lpm_flash_priv *flash_priv = lpm_flash_priv_info();
	struct lpm_flash_item_cfg *item_cfg = &flash_priv->item_info.item_cfg[(uint32_t)item_type];

	ret = memcpy_s(data, data_len,
		&flash_priv->dev_data[dev_id].flash_mem[item_cfg->offset], item_cfg->len);
	if (ret != 0) {
		lpm_log_err("dev_id=%u get item_type=%u data failed, ret=%d, offset=%u, item_len=%u\n",
			dev_id, (uint32_t)item_type, ret, item_cfg->offset, item_cfg->len);
		return -EIO;
	}
	return 0;
}

STATIC int32_t lpm_flash_set_item_data(uint32_t dev_id,
	enum lpm_flash_item_type item_type, uint8_t *data, uint32_t data_len)
{
	int32_t ret;
	struct lpm_flash_priv *flash_priv = lpm_flash_priv_info();
	struct lpm_flash_item_cfg *item_cfg = &flash_priv->item_info.item_cfg[(uint32_t)item_type];

	ret = memcpy_s(&flash_priv->dev_data[dev_id].flash_mem[item_cfg->offset], item_cfg->len,
		data, data_len);
	if (ret != 0) {
		lpm_log_err("dev_id=%u set item_type=%u data failed, ret=%d, offset=%u, item_len=%u\n",
			dev_id, (uint32_t)item_type, ret, item_cfg->offset, item_cfg->len);
		return ret;
	}
	return 0;
}

// return value:
// -ENOENT: user has not config flash
// -EINVAL: data read from flash illegal
// -EIO   : tried multiple times and still failed to read
// 0      : read success
STATIC int32_t lpm_flash_read_with_retry(uint32_t dev_id)
{
	int32_t ret;
	int32_t try_cnt;
	uint32_t out_len = 0;
	struct lpm_flash_priv *flash_priv = lpm_flash_priv_info();

	// inner function, not check param
	(void)memset_s(flash_priv->dev_data[dev_id].flash_mem, LPM_DEVMNG_FLASH_MAX_LEN,
		0, LPM_DEVMNG_FLASH_MAX_LEN);

	for (try_cnt = 0; try_cnt < LPM_DEVMNG_FLASH_READ_RETRY_CNT; try_cnt++) {
		ret = hal_kernel_get_user_config(dev_id, LPM_DEVMNG_FLASH_ITEM_NAME,
			flash_priv->dev_data[dev_id].flash_mem, LPM_DEVMNG_FLASH_MAX_LEN, &out_len);
		if (ret == -ENOENT) {
			// No such file or directory
			lpm_log_warn("user has not config dev_id=%u flash yet\n", dev_id);
			return -ENOENT;
		}
		if (ret != 0) {
			lpm_log_err("direct read dev_id=%u flash failed, ret=%d, try_cnt=%u\n", dev_id, ret, try_cnt);
			msleep(LPM_DEVMNG_FLASH_RETRY_WAIT_TIME_MS);
			continue;
		}
		if (out_len != LPM_DEVMNG_FLASH_MAX_LEN) {
			lpm_log_err("direct read dev_id=%u flash failed, out_len=%u should be %u\n",
				dev_id, out_len, LPM_DEVMNG_FLASH_MAX_LEN);
			return -EINVAL;
		}
		return 0;
	}

	// tried multiple times and still failed to read
	lpm_log_err("direct read dev_id=%u flash failed, ret=%d\n", dev_id, ret);
	return -EIO;
}

// return value:
// -EIO: tried multiple times and still failed to write
// 0   : write success
STATIC int32_t lpm_flash_write_with_retry(uint32_t dev_id)
{
	int32_t ret;
	int32_t try_cnt;
	struct lpm_flash_priv *flash_priv = lpm_flash_priv_info();

	for (try_cnt = 0; try_cnt < LPM_DEVMNG_FLASH_WRITE_RETRY_CNT; try_cnt++) {
		ret = hal_kernel_set_user_config(dev_id, LPM_DEVMNG_FLASH_ITEM_NAME,
			flash_priv->dev_data[dev_id].flash_mem, LPM_DEVMNG_FLASH_MAX_LEN);
		if (ret != 0) {
			lpm_log_err("direct write dev_id=%u flash failed, ret=%d, try_cnt=%u\n", dev_id, ret, try_cnt);
			continue;
		}
		return 0;
	}

	// tried multiple times and still failed to write
	lpm_log_err("direct write dev_id=%u flash failed, ret=%d\n", dev_id, ret);
	return -EIO;
}

// return value:
// -ENOENT: user has not config flash
// -EINVAL: data read from flash illegal
// -EIO   : flash read failed
// 0      : read success
STATIC int32_t lpm_flash_read_by_direct(struct lpm_flash_op_info *op_info)
{
	int32_t ret;

	// returnvalue:
	// 0      : read success
	// -ENOENT: user not set flash yet
	// other  : read failed
	// inner function, not check param
	// if there is a need to read flash frequently,
	// the reading part can be optimized and the flash data stored in memory can be directly used
	ret = lpm_flash_read_with_retry(op_info->dev_id);
	if (ret != 0) {
		// log record in read_all function, no need record repeat
		return ret;
	}

	ret = lpm_flash_get_item_data(op_info->dev_id, op_info->item_type, op_info->data, op_info->data_len);
	if (ret != 0) {
		lpm_log_err("dev_id=%u get item_type=%u data failed, ret=%d\n",
			op_info->dev_id, (uint32_t)op_info->item_type, ret);
		return -EIO;
	}

	return 0;
}

// return value:
// -EINVAL: data read from flash illegal
// -EIO   : flash read or write failed
// 0      : write success
STATIC int32_t lpm_flash_write_by_direct(struct lpm_flash_op_info *op_info)
{
	int32_t ret;

	// return value:
	// 0    : read success
	// other: read failed
	// inner function, not check param
	// if there is a need to write flash frequently,
	// the reading part can be optimized and the flash data stored in memory can be directly used
	ret = lpm_flash_read_with_retry(op_info->dev_id);
	if ((ret != -ENOENT) && (ret != 0)) {
		lpm_log_err("dev_id=%u read flash all failed when write, item_type=%u, ret=%d\n",
			op_info->dev_id, (uint32_t)op_info->item_type, ret);
		return ret;
	}

	// if the read flash api returns -ENOENT, it means user has not set flash yet
	// it is a normal scene, you can continue to write flash
	ret = lpm_flash_set_item_data(op_info->dev_id, op_info->item_type, op_info->data, op_info->data_len);
	if (ret != 0) {
		lpm_log_err("dev_id=%u set item_type=%u data failed, ret=%d\n",
			op_info->dev_id, (uint32_t)op_info->item_type, ret);
		return -EIO;
	}

	ret = lpm_flash_write_with_retry(op_info->dev_id);
	if (ret != 0) {
		lpm_log_err("direct write dev_id=%u flash failed, ret=%d\n", op_info->dev_id, ret);
		return ret;
	}
	return 0;
}

STATIC bool lpm_flash_check_op_info(const struct lpm_flash_op_info *op_info)
{
	struct lpm_flash_priv *flash_priv = lpm_flash_priv_info();

	if (op_info == NULL) {
		lpm_log_err("check op info failed, op_info is null\n");
		return false;
	}
	if (!lpm_common_check_dev_id(op_info->dev_id)) {
		lpm_log_err("check op info failed, dev_id=%u is out of range\n", op_info->dev_id);
		return false;
	}

	if (op_info->op_way >= LPM_FLASH_OP_WAY_MAX) {
		lpm_log_err("dev_id=%u check op info failed, op_way=%u is out of range\n",
			op_info->dev_id, (uint32_t)op_info->op_way);
		return false;
	}

	if (op_info->item_type >= LPM_FLASH_ITEM_MAX) {
		lpm_log_err("dev_id=%u op_way=%u check op info failed, item_type=%u is out of range\n",
			op_info->dev_id, (uint32_t)op_info->op_way, (uint32_t)op_info->item_type);
		return false;
	}
	if (op_info->data == NULL) {
		lpm_log_err("dev_id=%u op_way=%u item_type=%u check op info failed, data is null\n",
			op_info->dev_id, (uint32_t)op_info->op_way, (uint32_t)op_info->item_type);
		return false;
	}
	if (op_info->data_len != flash_priv->item_info.item_cfg[(uint32_t)op_info->item_type].len) {
		lpm_log_err("dev_id=%u op_way=%u item_type=%u check op info failed, data_len=%u should be %u\n",
			op_info->dev_id, (uint32_t)op_info->op_way, (uint32_t)op_info->item_type,
			op_info->data_len, flash_priv->item_info.item_cfg[(uint32_t)op_info->item_type].len);
		return false;
	}
	return true;
}

STATIC bool lpm_flash_check_thread_param(void *arg)
{
	struct lpm_flash_thread_param *in_param = NULL;
	if (arg == NULL) {
		lpm_log_err("flash kthread arg check failed, arg is null\n");
		return false;
	}

	in_param = (struct lpm_flash_thread_param *)arg;
	if (in_param->op_type >= LPM_FLASH_OP_TYPE_MAX) {
		lpm_log_err("flash kthread arg check failed, op_type=%u error\n", (uint32_t)in_param->op_type);
		return false;
	}

	if (!lpm_flash_check_op_info(&in_param->op_info)) {
		lpm_log_err("flash kthread arg check failed, op_info error\n");
		return false;
	}
	return true;
}

STATIC int32_t lpm_flash_kthread_handle(void *arg)
{
	int32_t ret;
	struct lpm_flash_priv *flash_priv = lpm_flash_priv_info();
	struct lpm_flash_thread_param *in_param = NULL;
	struct lpm_flash_thread_data *thread_data = NULL;

	if (!lpm_flash_check_thread_param(arg)) {
		// it may be that the dev_id is illegal and op_result cannot be set directly
		lpm_log_err("flash kthread arg check failed\n");
		return 0;
	}
	in_param = (struct lpm_flash_thread_param *)arg;
	thread_data = &flash_priv->dev_data[in_param->op_info.dev_id].thread_data;
	thread_data->op_result   = false;
	thread_data->flash_valid = true;

	if (in_param->op_type == LPM_FLASH_OP_TYPE_READ) {
		ret = lpm_flash_read_by_direct(&in_param->op_info);
		if (ret == -ENOENT) {
			// user has not set flash yet, only read care about this flag
			// normal scene, no need record error log
			thread_data->flash_valid = false;
			ret = 0;
		}
	} else {
		ret = lpm_flash_write_by_direct(&in_param->op_info);
	}

	if (ret != 0) {
		lpm_log_err("dev_id=%u flash kthread read/write(op_type=%u) failed, ret=%d\n",
			in_param->op_info.dev_id, (uint32_t)in_param->op_type, ret);
	} else {
		thread_data->op_result = true;
	}

	// change kthread status to notify kthread is done
	// There is no infinite loop in sub-thread processing, and it can exit normally
	atomic_set(&thread_data->thread_status, LPM_DEVMNG_FLASH_THREAD_STOP);
	return 0;
}

STATIC int32_t lpm_flash_handle_by_kthread(enum lpm_flash_op_type op_type, struct lpm_flash_op_info *op_info)
{
	int32_t wait_cnt = LPM_DEVMNG_FLASH_TASK_WAIT_CNT;
	struct lpm_flash_priv *flash_priv = lpm_flash_priv_info();
	struct task_struct *lpm_flash_task = NULL;
	struct lpm_flash_thread_data *thread_data = &flash_priv->dev_data[op_info->dev_id].thread_data;

	if (atomic_read(&thread_data->thread_status) == LPM_DEVMNG_FLASH_THREAD_RUN) {
		lpm_log_err("dev_id=%u last started thread did not exit normally, op_type=%u\n",
			op_info->dev_id, (uint32_t)op_type);
		return -EINPROGRESS;
	}

	// inner function, not check param
	thread_data->op_result   = false;
	thread_data->flash_valid = true;
	thread_data->thread_param.op_type = op_type;
	(void)memcpy_s(&thread_data->thread_param.op_info, sizeof(struct lpm_flash_op_info),
		op_info, sizeof(struct lpm_flash_op_info));

	atomic_set(&thread_data->thread_status, LPM_DEVMNG_FLASH_THREAD_RUN);
	lpm_flash_task = kthread_run(lpm_flash_kthread_handle, (void *)&thread_data->thread_param,
		"lpm_devmng_flash_kthread_%u", op_info->dev_id);
	if (IS_ERR(lpm_flash_task) || (lpm_flash_task == NULL)) {
		lpm_log_err("dev_id=%u create kthread for read/write(op_type=%u) flash failed\n",
			op_info->dev_id, (uint32_t)op_type);
		return -EINVAL;
	}

	while ((atomic_read(&thread_data->thread_status) == LPM_DEVMNG_FLASH_THREAD_RUN) && (wait_cnt >= 0)) {
		// add cnt limit to avoid death cycle
		// 4K flash data read and write can be completed within 1s
		// wait up to 1.5s for fault tolerance
		wait_cnt--;
		msleep(LPM_DEVMNG_FLASH_TASK_WAIT_TIME_MS);
	}

	if (wait_cnt < 0) {
		lpm_log_err("dev_id=%u read or write flash failed for wait timeout, op_type=%u\n",
			op_info->dev_id, (uint32_t)op_type);
		// Timer expired
		return -ETIME;
	}

	if (!thread_data->op_result) {
		// I/O error
		return -EIO;
	}

	if ((!thread_data->flash_valid) && (op_type == LPM_FLASH_OP_TYPE_READ)) {
		// user has not set flash yet, only read care about this flag
		return -ENOENT;
	}

	return 0;
}

STATIC void lpm_flash_destory_mem(uint32_t dev_num)
{
	uint32_t dev_id;
	struct lpm_flash_priv *flash_priv = lpm_flash_priv_info();

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		if (flash_priv->dev_data[dev_id].flash_mem != NULL) {
			kfree(flash_priv->dev_data[dev_id].flash_mem);
			flash_priv->dev_data[dev_id].flash_mem = NULL;
		}
		mutex_destroy(&flash_priv->dev_data[dev_id].flash_mutex);
	}
}

STATIC int32_t lpm_flash_init_mem(uint32_t dev_num)
{
	uint32_t dev_id;
	uint8_t *flash_mem = NULL;
	struct lpm_flash_priv *flash_priv = lpm_flash_priv_info();
	struct lpm_flash_dev_data *dev_data = NULL;

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		flash_mem = (uint8_t *)kzalloc(LPM_DEVMNG_FLASH_MAX_LEN, GFP_KERNEL);
		if (flash_mem == NULL) {
			lpm_log_err("dev_id=%u alloc flash_data mem failed\n", dev_id);
			goto alloc_failed;
		}

		dev_data = &flash_priv->dev_data[dev_id];
		dev_data->flash_mem = flash_mem;
		mutex_init(&dev_data->flash_mutex);
		atomic_set(&dev_data->thread_data.thread_status, LPM_DEVMNG_FLASH_THREAD_STOP);
	}
	return 0;

alloc_failed:
	lpm_flash_destory_mem(dev_id);
	return -1;
}

STATIC void lpm_flash_recover_dev_config(uint32_t dev_id)
{
	int32_t ret;
	bool flash_valid = true;
	uint32_t module_id;
	struct lpm_flash_recover_module *module_cfg = NULL;
	struct lpm_flash_item_cfg *item_cfg = NULL;
	struct lpm_flash_priv *flash_priv = lpm_flash_priv_info();

	// kernel thread, can directly read
	ret = lpm_flash_read_with_retry(dev_id);
	if (ret == -ENOENT) {
		// user has not set flash yet
		flash_valid = false;
	} else if (ret != 0) {
		// even if the flash reading fails, the modules must be notified to recover
		flash_valid = false;
		lpm_log_err("direct read dev_id=%u flash data failed when recover config, ret=%d\n",
			dev_id, ret);
	} else {
		flash_valid = true;
	}

	for (module_id = 0; module_id < flash_priv->recover_task.module_num; module_id++) {
		module_cfg = &flash_priv->recover_task.module_cfg[module_id];
		item_cfg = &flash_priv->item_info.item_cfg[(uint32_t)module_cfg->item_type];

		// the flash module does not pay attention to the recovery results of other modules
		ret = module_cfg->fn_recover(dev_id, flash_valid,
			&flash_priv->dev_data[dev_id].flash_mem[item_cfg->offset], item_cfg->len);
		if (ret != 0) {
			lpm_log_err("dev_id=%u recover item_type=%u config failed, ret=%d\n",
				dev_id, (uint32_t)module_cfg->item_type, ret);
		}
	}
}

STATIC void lpm_flash_recover_task_callback(struct work_struct *work)
{
	uint32_t dev_id;
	uint32_t dev_num = lpm_common_get_dev_num();
	struct lpm_flash_priv *flash_priv = lpm_flash_priv_info();

	lpm_log_info("lpm flash try to recover config from flash, dev_num=%u\n", dev_num);

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		mutex_lock(&flash_priv->dev_data[dev_id].flash_mutex);
		lpm_flash_recover_dev_config(dev_id);
		mutex_unlock(&flash_priv->dev_data[dev_id].flash_mutex);
	}

	flash_priv->recover_status = true;
	lpm_log_info("lpm flash recover config from flash success\n");

	(void)work;
}

STATIC void lpm_flash_init_recover_task(void)
{
	struct lpm_flash_priv *flash_priv = lpm_flash_priv_info();

	flash_priv->recover_status = false;
	INIT_WORK(&flash_priv->recover_task.init_work, lpm_flash_recover_task_callback);
	(void)schedule_work_on(LPM_DEVMNG_WORK_QUEUE_CPU, &flash_priv->recover_task.init_work);
}

STATIC void lpm_flash_sched_recover_task(void)
{
	struct lpm_flash_priv *flash_priv = lpm_flash_priv_info();

	// the config recovery has already been done, no need to repeat
	if (!flash_priv->recover_status) {
		(void)schedule_work_on(LPM_DEVMNG_WORK_QUEUE_CPU, &flash_priv->recover_task.init_work);
	}
}

STATIC void lpm_flash_uninit_recover_task(void)
{
	struct lpm_flash_priv *flash_priv = lpm_flash_priv_info();
	(void)cancel_work_sync(&flash_priv->recover_task.init_work);
}

STATIC bool lpm_flash_check_recover_module_cfg(const struct lpm_flash_recover_module *module_cfg)
{
	uint32_t module_id;
	struct lpm_flash_priv *flash_priv = lpm_flash_priv_info();

	if (module_cfg == NULL) {
		lpm_log_err("lpm flash register module failed, module cfg is null\n");
		return false;
	}
	if (module_cfg->item_type >= LPM_FLASH_ITEM_MAX) {
		lpm_log_err("lpm flash register module failed, item_type=%u is out of range\n",
			(uint32_t)module_cfg->item_type);
		return false;
	}
	if (module_cfg->fn_recover == NULL) {
		lpm_log_err("lpm flash register item_type=%u module failed, fn_recover is null\n",
			(uint32_t)module_cfg->item_type);
		return false;
	}

	// check repeat
	for (module_id = 0; module_id < flash_priv->recover_task.module_num; module_id++) {
		if (flash_priv->recover_task.module_cfg[module_id].item_type == module_cfg->item_type) {
			lpm_log_err("lpm flash register item_type=%u module repeat, module_id=%u\n",
				(uint32_t)module_cfg->item_type, module_id);
			return false;
		}
	}

	if (flash_priv->recover_task.module_num >= (uint32_t)LPM_FLASH_ITEM_MAX) {
		lpm_log_err("lpm flash register item_type=%u module failed, module_num=%u is full\n",
			(uint32_t)module_cfg->item_type, flash_priv->recover_task.module_num);
		return false;
	}
	return true;
}

// returnvalue:
// 0      : read success
// -ENOENT: user not set flash yet
// other  : read failed
int32_t lpm_flash_read(struct lpm_flash_op_info *op_info)
{
	int32_t ret;
	struct lpm_flash_priv *flash_priv = lpm_flash_priv_info();

	if (!lpm_flash_check_op_info(op_info)) {
		lpm_log_err("lpm read flash failed for op check failed\n");
		return -EINVAL;
	}

	// there is only one piece of memory under each dev to save the flash data read out at the time
	// and it is necessary to avoid re-entry
	// use mutex to protect critical resources, please pay attention when using
	mutex_lock(&flash_priv->dev_data[op_info->dev_id].flash_mutex);
	if (op_info->op_way == LPM_FLASH_OP_WAY_DIRECT) {
		ret = lpm_flash_read_by_direct(op_info);
	} else {
		ret = lpm_flash_handle_by_kthread(LPM_FLASH_OP_TYPE_READ, op_info);
	}

	if (ret == -ENOENT) {
		// user has not set flash yet, normal scene, no need record log
		mutex_unlock(&flash_priv->dev_data[op_info->dev_id].flash_mutex);
		return -ENOENT;
	}

	if (ret != 0) {
		mutex_unlock(&flash_priv->dev_data[op_info->dev_id].flash_mutex);
		lpm_log_err("dev_id=%u read flash data failed, ret=%d, op_way=%u\n",
			op_info->dev_id, ret, (uint32_t)op_info->op_way);
		return ret;
	}
	mutex_unlock(&flash_priv->dev_data[op_info->dev_id].flash_mutex);
	return 0;
}


// return value:
// 0: write success
// other: write failed
int32_t lpm_flash_write(struct lpm_flash_op_info *op_info)
{
	int32_t ret;
	struct lpm_flash_priv *flash_priv = lpm_flash_priv_info();

	if (!lpm_flash_check_op_info(op_info)) {
		lpm_log_err("lpm write flash failed for op check failed\n");
		return -EINVAL;
	}

	// use mutex to protect critical resources, please pay attention when using
	mutex_lock(&flash_priv->dev_data[op_info->dev_id].flash_mutex);
	if (op_info->op_way == LPM_FLASH_OP_WAY_DIRECT) {
		ret = lpm_flash_write_by_direct(op_info);
	} else {
		ret = lpm_flash_handle_by_kthread(LPM_FLASH_OP_TYPE_WRITE, op_info);
	}

	if (ret != 0) {
		mutex_unlock(&flash_priv->dev_data[op_info->dev_id].flash_mutex);
		lpm_log_err("dev_id=%u write flash data failed, ret=%d, op_way=%u\n",
			op_info->dev_id, ret, (uint32_t)op_info->op_way);
		return ret;
	}
	mutex_unlock(&flash_priv->dev_data[op_info->dev_id].flash_mutex);
	return 0;
}

int32_t lpm_flash_register_recover_module(const struct lpm_flash_recover_module *module_cfg)
{
	struct lpm_flash_priv *flash_priv = lpm_flash_priv_info();
	struct lpm_flash_recover_task *recover_task = &flash_priv->recover_task;

	// attention: should call before flash probe
	if (!lpm_flash_check_recover_module_cfg(module_cfg)) {
		lpm_log_err("lpm flash register module failed, cfg check failed\n");
		return -EINVAL;
	}

	recover_task->module_cfg[recover_task->module_num].item_type  = module_cfg->item_type;
	recover_task->module_cfg[recover_task->module_num].fn_recover = module_cfg->fn_recover;

	lpm_log_info("lpm flash register item_type=%u success, item_id=%u\n",
		(uint32_t)module_cfg->item_type, recover_task->module_num);

	recover_task->module_num++;
	return 0;
}

void lpm_flash_suspend_prepare(void)
{
	lpm_flash_uninit_recover_task();
	lpm_log_info("lpm flash suspend prepare success\n");
}

void lpm_flash_post_suspend(void)
{
	lpm_flash_sched_recover_task();
	lpm_log_info("lpm flash post suspend success\n");
}

int32_t lpm_flash_probe(uint64_t *param, uint32_t param_num)
{
	int32_t ret;
	uint32_t dev_num = lpm_common_get_dev_num();

	// alloc mem to save flash data
	ret = lpm_flash_init_mem(dev_num);
	if (ret != 0) {
		lpm_log_err("lpm flash probe failed when alloc mem, dev_num=%u\n", dev_num);
		return -1;
	}

	// config each item offset and len, avoid the sum of len large than 4k
	ret = lpm_flash_init_item_config();
	if (ret != 0) {
		lpm_log_err("lpm flash probe failed when config item, dev_num=%u\n", dev_num);
		goto config_item_failed;
	}

	// kmod thread can not read or write flash directly
	// create a thread to recover flash config
	lpm_flash_init_recover_task();

	lpm_log_info("lpm flash probe success, dev_num=%u\n", dev_num);

	(void)param;
	(void)param_num;
	return 0;

config_item_failed:
	lpm_flash_destory_mem(dev_num);
	return -1;
}

int32_t lpm_flash_remove(uint64_t *param, uint32_t param_num)
{
	uint32_t dev_num = lpm_common_get_dev_num();

	lpm_flash_uninit_recover_task();
	lpm_flash_destory_mem(dev_num);

	lpm_log_info("lpm flash remove success, dev_num=%u\n", dev_num);

	(void)param;
	(void)param_num;
	return 0;
}