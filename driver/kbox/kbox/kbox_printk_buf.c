/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: kbox get sym addr
 * Author: wangshouping
 * Create: 2019-02-14
 */
#include <linux/securec.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>

#include "../driver/drv_oper.h"
#include "kbox_common_interface.h"
#include "kbox_find_sym_addr.h"
#include "kbox_print.h"
#include "kernel/printk/printk_ringbuffer.h"

#define LOG_SIZE 20

#define TRY_LOGBUF_LOCK_CNT 20
#define LOG_LINE_MAX 1024

u32 g_printk_tmp_log_buf_len;
struct log {
	u64 ts_nsec;
	u16 len;
	u16 text_len;
	u16 dict_len;
	u8 facility;
	u8 flags : 5;
	u8 level : 3;
};

char *g_printk_log_buf;

static char *g_printk_tmp_log_buf;

#define kbox_prb_for_each_record(from, rb, s, r) \
for ((s) = from; kbox_prb_read_valid(rb, &(s), r, NULL); (s) = (r)->info->seq + 1)

int kbox_printk_tmp_log_buf_malloc(void)
{
	char *buf = NULL;
	int ret;

	g_printk_tmp_log_buf_len = (1u << (*kbox_prb)->text_data_ring.size_bits) +
		(1u << (*kbox_prb)->desc_ring.count_bits) * TIME_BUF_LEN;
	if (g_printk_tmp_log_buf_len == 0) {
		kbox_err("get kernel log buf len=0.");
		return -EINVAL;
	}
	buf = (char *)vmalloc(g_printk_tmp_log_buf_len);
	if (buf == NULL) {
		kbox_err("printk tmp log buf malloc failed.");
		return -ENOMEM;
	}

	ret = memset_s((void *)buf, g_printk_tmp_log_buf_len, 0, g_printk_tmp_log_buf_len);
	if (ret != 0) {
		kbox_err("memset_s fail, ret=%d", ret);
		vfree(buf);
		return ret;
	}
	g_printk_tmp_log_buf = buf;
	return 0;
}

void kbox_printk_tmp_log_buf_free(void)
{
	if (g_printk_tmp_log_buf != NULL) {
		vfree(g_printk_tmp_log_buf);
		g_printk_tmp_log_buf = NULL;
	}
}

static int kbox_copy_log_time(const struct printk_record *klog_head, const u32 len_to_collect,
	const u32 collected_len)
{
	char time_buf[TIME_BUF_LEN];
	u64 ts_usec;
	int time_len;
	int ret;

	/* collect timestamp */
	ts_usec = klog_head->info->ts_nsec / TIME_USEC;
	time_len = snprintf_s(time_buf, sizeof(time_buf), sizeof(time_buf) - 1,
		"[%5llu.%6llu] ", ts_usec / TIME_USEC_TO_SEC, ts_usec % TIME_USEC_TO_SEC);
	if (time_len == -1) {
		kbox_err("snprintf_s failed.");
		return 0;
	}
	if ((collected_len + (u32)time_len) > len_to_collect) {
		return 0;
	} else if (time_len > 0) {
		ret = memcpy_s(&g_printk_tmp_log_buf[collected_len],
			       (size_t)(g_printk_tmp_log_buf_len - collected_len), time_buf,
			       (size_t)time_len);
		if (ret != 0) {
			return 0;
		}
	}
	return time_len;
}

static u32 kbox_copy_one_log(const struct printk_record *klog_head, const u32 len_to_collect,
	const u32 collected_len)
{
	u32 char_idx = 0;
	char char_log = klog_head->text_buf[0];

	while ((char_idx < klog_head->text_buf_size) && char_log != '\0') {
		if ((collected_len + char_idx + 1) > len_to_collect) {
			break;
		}
		g_printk_tmp_log_buf[collected_len + char_idx] = char_log;
		char_idx++;

		if (char_idx < klog_head->text_buf_size) {
			char_log = klog_head->text_buf[char_idx];
		}
	}
	return char_idx;
}

static int try_get_logbuf_lock(unsigned long *flags)
{
	int count = 0;

	while (count < TRY_LOGBUF_LOCK_CNT) {
		if (raw_spin_trylock_irqsave(printk_logbuf_lock, *flags)) {
			break;
		}
		count++;
		mdelay(1);
	}
	if (count == TRY_LOGBUF_LOCK_CNT) {
		return 0;
	}
	return 1;
}

static u32 kbox_log_collect(const u32 len_to_collect)
{
	u32 collected_len = 0;
	int time_len;
	u32 char_idx;
	unsigned long flags;
	struct printk_record record;
	struct printk_info info;
	u64 seq;
	char text_buf[LOG_LINE_MAX] = {0};

	if (g_printk_tmp_log_buf == NULL) {
		kbox_err("kbox_log_collect failed.");
		return 0;
	}

	if (try_get_logbuf_lock(&flags) == 0) {
		kbox_err("kbox_log_collect get logbuf_lock fail!");
		return 0;
	}

	record.info = &info;
	record.text_buf = text_buf;
	record.text_buf_size = LOG_LINE_MAX;

	seq = kbox_prb_first_valid_seq(*kbox_prb);
	kbox_prb_for_each_record(seq, *kbox_prb, seq, &record) {
		time_len = kbox_copy_log_time(&record, len_to_collect, collected_len);
		if (time_len == 0) {
			goto out;
		}

		collected_len += (u32)time_len;

		/* collect log text, when time_len == -1 continue to collect log text */
		char_idx = kbox_copy_one_log(&record, len_to_collect, collected_len);

		collected_len += char_idx;
		if ((collected_len + 1) > len_to_collect) {
			goto out;
		} else {
			g_printk_tmp_log_buf[collected_len] = '\n';
			collected_len++;
		}

		memset_s(record.text_buf, record.text_buf_size, 0, record.text_buf_size);
	}
out:
	raw_spin_unlock_irqrestore(printk_logbuf_lock, flags);
	return collected_len;
}

static int kbox_copy_printk_buf_log(int fd, const unsigned int copy_log_len)
{
	u32 len_to_collect = copy_log_len;
	u32 start;
	u32 collected_len;
	int ret;

	if (fd < 0) {
		return -EINVAL;
	}
	kbox_info("start to collect.");
	collected_len = kbox_log_collect(g_printk_tmp_log_buf_len);
	if (collected_len == 0) {
		kbox_err("kbox_log_collect none.");
		return -EINVAL;
	}

	kbox_info("collected_len = %u, g_printk_tmp_log_buf_len = %u", collected_len,
			g_printk_tmp_log_buf_len);

	if (collected_len <= len_to_collect) {
		/* write printk buf log to device */
		ret = kbox_log_write(fd, &g_printk_tmp_log_buf[0], collected_len);
		return ret;
	} else {
		start = collected_len - len_to_collect;
		ret = kbox_log_write(fd, &g_printk_tmp_log_buf[start], len_to_collect);
		return ret;
	}
}

void kbox_store_pre_log(int fd, const unsigned int copy_log_len)
{
	int ret;

	ret = kbox_copy_printk_buf_log(fd, copy_log_len);
	if (ret < 0) {
		kbox_err("kbox_copy_printk_buf fail, ret=%d", ret);
		return;
	}
	kbox_info("save kbox pre log success.");
}

int kbox_write_reset_reason(unsigned int reason, unsigned long time)
{
	char *event;

	event = kbox_event_to_string(reason);
	kbox_info("catch %s event, start logging.", event);
	return kbox_write_reset(reason, time);
}
