/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: Registre callback notifier for DIE with register_fs_exception_notifier
 * Create: 2019-06-28
 */

#include "kbox_fs_exception.h"

#include <linux/kdebug.h>
#include <linux/module.h>
#include <linux/sched/debug.h>
#include <linux/securec.h>
#include <linux/sched/clock.h>

#include <asm/ptrace.h>
#include <asm/stacktrace.h>

#include "../driver/drv_oper.h"
#include "kbox_common.h"
#include "kbox_common_interface.h"
#include "kbox_find_sym_addr.h"
#include "kbox_print.h"
#include "kbox_printk_buf.h"

static int kbox_fs_exception_notifier_callback(struct notifier_block *nfb,
	unsigned long action, void *data);

static struct notifier_block g_kbox_fs_exception_notifier = {
	.notifier_call = kbox_fs_exception_notifier_callback,
	.priority = 110
};

#define	DUMP_INIT	0
#define	DUMP_FINISH	1

static atomic_t g_dump_stat = ATOMIC_INIT(DUMP_INIT);

static void kbox_write_fs_exception_log(int fd, const unsigned int copy_log_len)
{
	int ret;

	kbox_store_pre_log(fd, copy_log_len);
	ret = kbox_regsiter_store_log();
	if (ret != 0) {
		kbox_err("kbox_regsiter_store_log fail, ret=%d", ret);
	}
	kbox_unregister_store_log();
}

static void kbox_fs_exception_record_log(unsigned int reason)
{
	int ret;
	unsigned long time;

	time = (unsigned long)local_clock();
	/* Write reason for log export, but we do not reset */
	ret = kbox_write_reset_reason(reason, time);
	if (ret != 0) {
		kbox_err("kbox_write_reset err ret = %d.", ret);
		return;
	}

	g_kbox_log_fd = kbox_log_open(time);
	if (g_kbox_log_fd < 0) {
		kbox_err("kbox_log_open fail.");
		return;
	}

	kbox_write_fs_exception_log(g_kbox_log_fd, KBOX_COPY_PRINTK_LOG_LEN);

	ret = kbox_log_close(g_kbox_log_fd);
	if (ret != 0) {
		kbox_err("kbox_log_close fail.");
	}

	g_kbox_log_fd = -1;
}

static int kbox_fs_exception_notifier_callback(struct notifier_block *self, unsigned long type, void *data)
{
	int ret;
	unsigned long flags;

	if (atomic_cmpxchg(&g_kbox_status, KBOX_STAT_INIT, KBOX_STAT_WORK) == KBOX_STAT_EXIT) {
		kbox_err_ratelimit("kbox is rmmoding, this fs_exception record will loss");
		return NOTIFY_OK;
	}

	if (&g_kbox_fs_exception_notifier != self) {
		kbox_err("wrong callback for fs exception notifier! self = 0x%pK", self);
		goto out;
	}

	if (atomic_cmpxchg(&g_dump_stat, DUMP_INIT, DUMP_FINISH) == DUMP_FINISH) {
		kbox_info("kbox has recorded fs exception before, exit.");
		goto out;
	}

	ret = kbox_event_pre_process(FS_EXCEPTION_TYPE);
	if (ret != EVENT_SUBSEQUENT_OK) {
		kbox_err("kbox is recording some other exceptions.");
		goto out;
	}

	spin_lock_irqsave(&g_kbox_event_lock, flags);
	kbox_fs_exception_record_log((unsigned int)FS_EXCEPTION_TYPE);
	spin_unlock_irqrestore(&g_kbox_event_lock, flags);
out:
	atomic_set(&g_kbox_status, KBOX_STAT_INIT);
	return NOTIFY_OK;
}

int kbox_fs_exception_init(void)
{
	int ret;

	ret = register_ext4_exception_notifier(&g_kbox_fs_exception_notifier);
	if (ret < 0) {
		kbox_err("kbox fs exception init register hook failed! ret = %d.", ret);
	}

	return ret;
}

void kbox_fs_exception_fini(void)
{
	(void)unregister_ext4_exception_notifier(&g_kbox_fs_exception_notifier);
}
