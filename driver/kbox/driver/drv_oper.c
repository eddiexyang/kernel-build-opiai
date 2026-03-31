/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: kbox drv log
 * Author:
 * Create: 2019-02-14
 */

#include "drv_oper.h"

#include <linux/fs.h>
#include <linux/securec.h>
#include <linux/types.h>
#include <linux/libnvdimm.h>

#include <asm/atomic.h>
#include <kbox/kbox_ioctl.h>

#include "drv_main.h"
#include "drv_print.h"

#define WRITE_SIZE 4096

atomic_t g_drv_printk_flag = ATOMIC_INIT(1);
spinlock_t g_drv_write_lock;
char kbox_log_path[KBOX_PATH_MAXLEN] = DEFAULT_KBOX_LOG;
static int g_kbox_nospace_flag;

static int get_current_log_idx(void)
{
	struct kbox_store_manage *kbox_manage = NULL;
	unsigned int idx;

	kbox_manage = (struct kbox_store_manage *)g_kbox_va_start;
	idx = kbox_manage->log_manage.next_idx;
	if (idx >= MAX_LOG_CONTENT) {
		return -EBADF;
	}
	idx = (idx + MAX_LOG_CONTENT - 1) % MAX_LOG_CONTENT;
	return (int)idx;
}

static int recycle_oldest_log_to_current(struct log_content *current_content,
	unsigned int size)
{
	unsigned int oldest_idx;
	struct log_content *oldest_content = NULL;
	unsigned int recycled_size;
	struct kbox_store_manage *kbox_manage = NULL;

	kbox_manage = (struct kbox_store_manage *)g_kbox_va_start;
	oldest_idx = kbox_manage->log_manage.oldest_idx;
	if (oldest_idx >= MAX_LOG_CONTENT) {
		return -EBADF;
	}

	drv_info("recycle oldest_idx=%u recycle size=%u", oldest_idx, size);
	recycled_size = 0;
	while (recycled_size < size) {
		oldest_content = kbox_manage->log_manage.log_content + oldest_idx;
		/* 1. one time write kbox area full
		     * 2. recycled all history log
		     */
		if (oldest_content == current_content) {
			g_kbox_nospace_flag = 1;
			kbox_manage->magic = KBOX_NOSPACE_MAGIC;
			return -ENOMEM;
		}
		recycled_size += (oldest_content->log_size + oldest_content->pad);
		drv_info("recycled size = %u", recycled_size);
		oldest_content->offset = 0;
		oldest_content->log_size = 0;
		oldest_content->pad = 0;
		oldest_content->valid = 0;
		oldest_content->remain_size = 0;
		kbox_manage->log_manage.oldest_idx =
			(oldest_idx + 1 + MAX_LOG_CONTENT) % MAX_LOG_CONTENT;
		oldest_idx = kbox_manage->log_manage.oldest_idx;
	}
	drv_info("recycled out = %u", recycled_size);
	if (recycled_size > (g_kbox_size - KBOX_LOG_OFFSET)) {
		return -EFAULT;
	}
	current_content->remain_size += recycled_size;
	return 0;
}

static unsigned int wrap_recycle_oldest_log(struct log_content *current_content)
{
	unsigned int oldest_idx;
	struct kbox_store_manage *kbox_manage = NULL;
	struct log_content *oldest_content = NULL;
	unsigned int recycle_oldest_size = 0;

	kbox_manage = (struct kbox_store_manage *)g_kbox_va_start;
	oldest_idx = kbox_manage->log_manage.oldest_idx;
	if (oldest_idx >= MAX_LOG_CONTENT) {
		drv_err("oldest_idx bigger or equal to MAX_LOG_CONTENT.");
		return 0;
	}
	oldest_content = kbox_manage->log_manage.log_content + oldest_idx;
	if (oldest_content->valid == 1) {
		if (current_content == oldest_content) {
			recycle_oldest_size = oldest_content->log_size + oldest_content->pad;
			oldest_content->offset = 0;
			oldest_content->log_size = 0;
			oldest_content->pad = 0;
			oldest_content->valid = 0;
			oldest_content->remain_size = 0;
			kbox_manage->log_manage.oldest_idx = (oldest_idx + 1 +
					MAX_LOG_CONTENT) % MAX_LOG_CONTENT;
		}
	}
	return recycle_oldest_size;
}

static int init_next_content(unsigned long time)
{
	unsigned int pad;
	unsigned int idx;
	unsigned int last_idx;
	unsigned int oldest_idx;
	unsigned int recycle_oldest_size = 0;
	unsigned int offset;
	unsigned long last_log_end_offset;
	struct log_content *current_content = NULL;
	struct log_content *last_content = NULL;
	struct kbox_store_manage *kbox_manage = NULL;

	kbox_manage = (struct kbox_store_manage *)g_kbox_va_start;
	idx = kbox_manage->log_manage.next_idx;
	oldest_idx = kbox_manage->log_manage.oldest_idx;
	current_content = kbox_manage->log_manage.log_content + idx;
	/* when next_idx is equal to oldest_idx, recycle oldest log */
	if (idx == oldest_idx) {
		recycle_oldest_size = wrap_recycle_oldest_log(current_content);
	}

	last_idx = (idx + MAX_LOG_CONTENT - 1) % MAX_LOG_CONTENT;
	last_content = kbox_manage->log_manage.log_content + last_idx;

	/* if last content invalid, all the content is new init, so no remain size */
	if (last_content->valid == 1) {
		current_content->remain_size = last_content->remain_size;
		last_log_end_offset = last_content->offset + last_content->log_size;
		offset = (unsigned int)last_log_end_offset;
		pad = 0;
		/* offset may overflow after roundup */
		offset = offset % (g_kbox_size - KBOX_LOG_OFFSET);
	} else {
		offset = 0;
		pad = 0;
	}

	current_content->remain_size += recycle_oldest_size - pad;
	current_content->pad = pad;
	current_content->offset = offset;
	current_content->log_size = 0;
	current_content->valid = 1;
	current_content->ts_nsec = time;
	kbox_manage->log_manage.next_idx = (idx + 1) % MAX_LOG_CONTENT;
	return (int)idx;
}

int kbox_log_open(unsigned long time)
{
	if (time == 0) {
		return -EINVAL;
	}

	if (g_kbox_va_start == 0) {
		return -ENOMEM;
	}
	return init_next_content(time);
}
EXPORT_SYMBOL_GPL(kbox_log_open);

static int kbox_copy_log(char *dest, unsigned int dest_size, const char *src,
	unsigned int src_size)
{
	int ret;
	char *end = (char *)g_kbox_va_start + g_kbox_size;
	char *write_start = NULL;
	char *read_start = NULL;
	unsigned int tmp_src_size;
	unsigned int tmp_dest_size;
	unsigned long tmp_size;

	if (dest + dest_size > end) {
		tmp_size = end - dest;
	} else {
		tmp_size = dest_size;
	}

	if (tmp_size >= src_size) {
		ret = memcpy_s(dest, dest_size, src, src_size);
		if (ret != 0) {
			drv_err("log not round up, memcpy_s fail. ret=%d", ret);
		}
		return ret;
	}

	/* round up */
	ret = memcpy_s(dest, tmp_size, src, tmp_size);
	if (ret != 0) {
		drv_err("log round up, memcpy_s head fail. ret=%d", ret);
		return ret;
	}
	write_start = (char *)g_kbox_va_start + KBOX_LOG_OFFSET;
	read_start = (char *)src + tmp_size;
	tmp_dest_size = dest_size - tmp_size;
	tmp_src_size = src_size - tmp_size;
	ret = memcpy_s(write_start, tmp_dest_size, read_start, tmp_src_size);
	if (ret != 0) {
		drv_err("log not round up, memcpy_s tail fail. ret=%d", ret);
	}
	return ret;
}

static bool check_write_param(const char *text, const unsigned int size)
{
	return (text == NULL || (size > (g_kbox_size - KBOX_LOG_OFFSET)));
}

static bool check_fd(const int input_fd, const int current_fd)
{
	return (current_fd < 0 || input_fd != current_fd);
}

int kbox_log_write(int fd, const char *text, unsigned int size)
{
	int write_idx, ret;
	char *write_addr = NULL;
	unsigned long flags, offset;
	struct kbox_store_manage *kbox_manage = NULL;
	struct log_content *write_content = NULL;

	/* event ->printk ->kbox ->error_info ->printk  ==> deadlock */
	if (atomic_read(&g_drv_printk_flag) == 1) {
		return 0;
	}
	if (g_kbox_nospace_flag == 1) {
		return -ENOMEM;
	}
	if (check_write_param(text, size)) {
		return -EINVAL;
	}
	kbox_manage = (struct kbox_store_manage *)g_kbox_va_start;
	if (kbox_manage == NULL) {
		return -ENOMEM;
	}
	write_idx = get_current_log_idx();
	if (check_fd(fd, write_idx)) {
		return -EINVAL;
	}

	spin_lock_irqsave(&g_drv_write_lock, flags);
	drv_debug("write_idx=%d\n", write_idx);
	write_content = &kbox_manage->log_manage.log_content[write_idx];
	drv_debug("remain_size=%u  write size=%u\n", write_content->remain_size, size);
	if (write_content->remain_size < size) {
		ret = recycle_oldest_log_to_current(write_content,
			size - write_content->remain_size);
		if (ret != 0) {
			drv_err("recycle buf fail, ret=%d", ret);
			spin_unlock_irqrestore(&g_drv_write_lock, flags);
			return -EFAULT;
		}
	}

	/* log may round up last call kbox_log_write */
	offset = (write_content->offset + write_content->log_size) % (g_kbox_size - KBOX_LOG_OFFSET);
	write_addr = (char *)g_kbox_va_start + KBOX_LOG_OFFSET + offset;
	/* check if write to the end of the log_buf */
	ret = kbox_copy_log(write_addr, write_content->remain_size, text, size);
	if (ret != 0) {
		drv_err("copy log fail. ret=%d", ret);
		spin_unlock_irqrestore(&g_drv_write_lock, flags);
		return -EFAULT;
	}

	write_content->log_size += size;
	write_content->remain_size -= size;
	spin_unlock_irqrestore(&g_drv_write_lock, flags);
	return (int)size;
}
EXPORT_SYMBOL_GPL(kbox_log_write);

int kbox_log_close(int fd)
{
	int write_idx;
	struct kbox_store_manage *kbox_manage = NULL;

	kbox_manage = (struct kbox_store_manage *)g_kbox_va_start;
	if (kbox_manage == NULL) {
		return -ENOMEM;
	}
	write_idx = get_current_log_idx();
	if (fd != write_idx) {
		return -EINVAL;
	}

	arch_wb_cache_pmem((char *)(g_kbox_va_start), g_kbox_size);

	return 0;
}
EXPORT_SYMBOL_GPL(kbox_log_close);

static bool reset_param_invalid(unsigned int event, unsigned long time)
{
	return ((event >= (unsigned int)TYPE_BUTTON) ||
			(event <= (unsigned int)TYPE_PROTECT) ||
			(time == 0));
}

int kbox_write_reset(unsigned int event, unsigned long time)
{
	unsigned int idx;
	unsigned int offset;
	struct reset_info *next_reset_info = NULL;
	struct kbox_store_manage *kbox_manage = NULL;

	if (reset_param_invalid(event, time)) {
		return -EINVAL;
	}

	kbox_manage = (struct kbox_store_manage *)g_kbox_va_start;
	if (kbox_manage == NULL) {
		return -EFAULT;
	}

	idx = kbox_manage->reset_manage.next_idx;
	if (idx > MAX_RESET) {
		return -ENOSPC;
	}
	offset = kbox_manage->reset_manage.reset_offset;
	next_reset_info = (struct reset_info *)(g_kbox_va_start + offset +
		idx * sizeof(struct reset_info));
	next_reset_info->ts_nsec = time;
	next_reset_info->valid = 1;
	next_reset_info->reason = event;
	kbox_manage->reset_manage.next_idx = (idx + 1) % MAX_RESET;
	kbox_manage->reset_manage.current_status = RESET_RECORDED;

	return 0;
}
EXPORT_SYMBOL_GPL(kbox_write_reset);

static int get_reset_info_in_order(struct reset_info *events, unsigned int buf_size)
{
	int ret;
	unsigned int idx;
	unsigned int i;
	void *dst = NULL;
	unsigned int dst_size = buf_size;
	struct reset_info *reset_info = NULL;
	struct kbox_store_manage *kbox_manage = NULL;

	if (buf_size < MAX_RESET * sizeof(struct reset_info)) {
		return -EINVAL;
	}

	dst = events;
	ret = 0;
	kbox_manage = (struct kbox_store_manage *)g_kbox_va_start;
	idx = (kbox_manage->reset_manage.next_idx + MAX_RESET) % MAX_RESET;
	for (i = 0; i < MAX_RESET; i++) {
		idx = ((idx + MAX_RESET - 1) % MAX_RESET);
		reset_info = (struct reset_info *)(g_kbox_va_start + MANAGE_AREA_SIZE +
			idx * sizeof(struct reset_info));
		if (reset_info->valid != 1) {
			continue;
		}
		ret = memcpy_s(dst, dst_size, (void *)reset_info, sizeof(struct reset_info));
		if (ret != 0) {
			break;
		}
		dst_size -= sizeof(struct reset_info);
		dst += sizeof(struct reset_info);
	}
	return ret;
}

int kbox_query_reset(kbox_query_ioc *query_cb)
{
	int ret;
	unsigned int count = query_cb->query_num;
	struct reset_info *events = query_cb->events;
	unsigned int buf_size = sizeof(query_cb->events);
	struct kbox_store_manage *kbox_manage = NULL;

	if (count > (buf_size / sizeof(struct reset_info)) || (count == 0)) {
		return -EINVAL;
	}

	kbox_manage = (struct kbox_store_manage *)g_kbox_va_start;
	if (kbox_manage == NULL) {
		return -EFAULT;
	}

	ret = get_reset_info_in_order(events, buf_size);
	if (ret != 0) {
		return ret;
	}
	query_cb->last_status = kbox_manage->reset_manage.last_status;
	query_cb->current_status = kbox_manage->reset_manage.current_status;
	return 0;
}

static int kbox_write_file_data(struct file *filp, const char *data, unsigned int data_len, loff_t *pos)
{
	int ret;
	unsigned int tmp_len;
	unsigned int written = 0;

	while (written < data_len) {
		tmp_len = WRITE_SIZE;
		if (data_len - written < WRITE_SIZE) {
			tmp_len = data_len - written;
		}

		ret = kernel_write(filp, data, tmp_len, pos);
		if ((unsigned int)ret != tmp_len) {
			drv_err("write file %s fail, ret=%d.", kbox_log_path, ret);
			return -EAGAIN;
		}
		data += ret;
		written += (unsigned int)ret;
	}
	return 0;
}

static int kbox_export_log_file(unsigned int log_idx)
{
	struct file *filp = NULL;
	char *data = NULL;
	char *end = (char *)g_kbox_va_start + g_kbox_size;
	unsigned int len;
	unsigned int tmp_len;
	int ret;
	loff_t pos = 0;
	struct kbox_store_manage *kbox_manage = NULL;
	struct log_content *log_content = NULL;

	kbox_manage = (struct kbox_store_manage *)g_kbox_va_start;
	log_content = &(kbox_manage->log_manage.log_content[log_idx]);
	data = (char *)g_kbox_va_start + KBOX_LOG_OFFSET + log_content->offset;
	len = log_content->log_size;

	filp = filp_open(kbox_log_path, KBOX_WRITE_FLAG, 0440);
	if (IS_ERR(filp)) {
		drv_err("open file %s fail.", kbox_log_path);
		return -EFAULT;
	}
	drv_debug("write src data=%pK len=%u", data, len);

	if (((uintptr_t)data + len) <= (g_kbox_va_start + g_kbox_size)) {
		ret = kbox_write_file_data(filp, data, len, &pos);
		if (ret != 0) {
			drv_err("write data fail.");
			(void)filp_close(filp, NULL);
			return -EFAULT;
		}
		drv_debug("write src data success.");
		(void)filp_close(filp, NULL);
		return 0;
	}

	tmp_len = (unsigned int)(end - data);
	ret = kbox_write_file_data(filp, data, tmp_len, &pos);
	if (ret != 0) {
		drv_err("write end data fail.");
		(void)filp_close(filp, NULL);
		return -EFAULT;
	}
	data = (char *)g_kbox_va_start + KBOX_LOG_OFFSET;
	tmp_len = len - tmp_len;
	ret = kbox_write_file_data(filp, data, tmp_len, &pos);
	if (ret != 0) {
		drv_err("write round data fail.");
		(void)filp_close(filp, NULL);
		return -EFAULT;
	}
	drv_debug("write src success, data=%pK", data);
	return filp_close(filp, NULL);
}

static unsigned long get_reset_time(unsigned int type, unsigned int index)
{
	unsigned int current_idx;
	unsigned int tmp_idx;
	unsigned int i;
	unsigned int reason;
	unsigned long time = 0;
	struct kbox_store_manage *kbox_manage = NULL;
	struct reset_info *reset_info = NULL;

	kbox_manage = (struct kbox_store_manage *)g_kbox_va_start;
	current_idx = kbox_manage->reset_manage.next_idx;
	if (current_idx > MAX_RESET) {
		drv_err("index error(%u).", current_idx);
		return 0;
	}

	tmp_idx = 0;
	for (i = 0; i < MAX_RESET; i++) {
		reset_info = (void *)kbox_manage + MANAGE_AREA_SIZE;
		reset_info += ((current_idx + MAX_RESET - 1 - i) % MAX_RESET);
		drv_debug("i=%u reset_info=%pK offset=%u", i, reset_info,
			((current_idx - 1 - i + MAX_RESET) % MAX_RESET));
		if (reset_info->valid != 1) {
			continue;
		}
		reason = reset_info->reason;
		drv_debug("tmp_idx=%u, query_index=%u reason=%u query_reason=%u", tmp_idx,
			index, reason, type);
		if (reason == type) {
			tmp_idx++;
		}
		if (tmp_idx == index) {
			time = reset_info->ts_nsec;
			break;
		}
	}
	return time;
}

static int get_idx_segment(const struct kbox_log_manage *log_manage, unsigned long time,
	unsigned int start, unsigned int end)
{
	unsigned int i;
	unsigned int flag = 0;
	const struct log_content *log_content = NULL;

	if (end > MAX_LOG_CONTENT) {
		drv_err("get id error. start=%u, end=%u", start, end);
		return -EINVAL;
	}

	for (i = start; i < end; i++) {
		log_content = &(log_manage->log_content[i]);
		if (log_content->valid != 1) {
			drv_debug("log event %d invalid", i);
			continue;
		}

		drv_debug("log event %u, time: %lu", i, log_content->ts_nsec);
		if (log_content->ts_nsec == time) {
			flag = 1;
			break;
		}
	}
	if (flag == 0) {
		return -EINVAL;
	}
	return (int)i;
}

static int get_log_time_idx(unsigned long time)
{
	struct kbox_store_manage *kbox_manage = NULL;
	struct kbox_log_manage *log_manage = NULL;

	kbox_manage = (struct kbox_store_manage *)g_kbox_va_start;
	log_manage = &(kbox_manage->log_manage);
	drv_debug("query log start [0 to MAX_LOG_CONTENT)");
	return get_idx_segment(log_manage, time, 0, MAX_LOG_CONTENT);
}

static int kbox_export_the_log(unsigned int type, unsigned int index)
{
	struct kbox_store_manage *kbox_manage = NULL;
	unsigned long time;
	int log_idx;
	int ret;

	kbox_manage = (struct kbox_store_manage *)g_kbox_va_start;
	if (kbox_manage == NULL) {
		drv_err("kbox start area error.");
		return -EFAULT;
	}

	time = get_reset_time(type, index);
	if (time == 0) {
		drv_err("can't find the log under input conditions.");
		return -EFAULT;
	}
	drv_debug("query event time: %lu", time);
	log_idx = get_log_time_idx(time);
	if (log_idx < 0) {
		drv_err("can't match the log under input conditions.");
		return -EFAULT;
	}

	ret = kbox_export_log_file((unsigned int)log_idx);
	return ret;
}

static int kbox_export_log_all(void)
{
	unsigned int i;
	int ret;
	struct file *filp = NULL;
	char *data = NULL;
	unsigned int len;
	unsigned int tmp_len;
	loff_t pos = 0;
	struct kbox_store_manage *kbox_manage = NULL;
	struct log_content *log_content = NULL;
	struct kbox_log_manage *log_manage = NULL;

	kbox_manage = (struct kbox_store_manage *)g_kbox_va_start;
	if (kbox_manage == NULL) {
		return -EFAULT;
	}
	filp = filp_open(kbox_log_path, KBOX_WRITE_FLAG, 0640);
	if (IS_ERR(filp)) {
		drv_err("open file %s fail.", kbox_log_path);
		return -EFAULT;
	}
	log_manage = &kbox_manage->log_manage;
	drv_debug("export [%u, %u)", log_manage->oldest_idx, log_manage->next_idx);

	for (i = 0; i < MAX_LOG_CONTENT; i++) {
		tmp_len = 0;
		log_content = &(kbox_manage->log_manage.log_content[i]);
		data = (char *)g_kbox_va_start + KBOX_LOG_OFFSET + log_content->offset;
		len = log_content->log_size;
		if (((uintptr_t)data + len) > (g_kbox_va_start + g_kbox_size)) {
			len = (unsigned int)((char *)(uintptr_t)g_kbox_va_start + g_kbox_size - data);
			tmp_len = log_content->log_size - len;
		}
		drv_debug("%u, to write size=%u", i, len);
		drv_debug("write src data=%pK", data);
		ret = kbox_write_file_data(filp, data, len, &pos);
		if (ret != 0) {
			(void)filp_close(filp, NULL);
			return -EAGAIN;
		}
		if (tmp_len > 0) {
			data = (char *)(uintptr_t)g_kbox_va_start + KBOX_LOG_OFFSET;
			ret = kbox_write_file_data(filp, data, tmp_len, &pos);
			if (ret != 0) {
				(void)filp_close(filp, NULL);
				return -EAGAIN;
			}
		}
		drv_debug("write src data=%pK", data);
	}
	return filp_close(filp, NULL);
}

static bool kbox_check_export_args(unsigned int type, unsigned int index)
{
	return ((type > (unsigned int)TYPE_PROTECT) && (type < (unsigned int)TYPE_BUTTON)) &&
	((index > 0) && (index < MAX_RESET));
}

static bool kbox_check_export_all(unsigned int type, unsigned int index)
{
	return ((type == 0) && (index == 0));
}

/* type=0,index=0: export all
 * type>x,index=y: export last x yth
 */
int kbox_export_dev_info(unsigned int type, unsigned int index)
{
	if (kbox_check_export_args(type, index)) {
		return kbox_export_the_log(type, index);
	} else if (kbox_check_export_all(type, index)) {
		return kbox_export_log_all();
	} else {
		drv_err("export log param error.");
		return -EINVAL;
	}
}

int kbox_get_nospace_flag(void)
{
	return g_kbox_nospace_flag;
}
EXPORT_SYMBOL_GPL(kbox_get_nospace_flag);

void kbox_export_memory_image(void)
{
	struct file *filp = NULL;
	loff_t pos = 0;
	int ret;

	filp = filp_open(KBOX_MEMORY_IMAGE, KBOX_WRITE_FLAG, 0640);
	if (IS_ERR(filp)) {
		drv_err("open file %s fail.", KBOX_MEMORY_IMAGE);
		return;
	}
	ret = kbox_write_file_data(filp, (char *)g_kbox_va_start, (unsigned int)g_kbox_size, &pos);
	if (ret != 0) {
		drv_err("write image data fail.");
		(void)filp_close(filp, NULL);
		return;
	}
	(void)filp_close(filp, NULL);
}
