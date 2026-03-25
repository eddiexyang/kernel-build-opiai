/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: Registre callback notifier for DIE with register_die_notifier
 * Author:
 * Create: 2019-03-07
 */

#include "kbox_die.h"

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

static int kbox_die_notifier_callback(struct notifier_block *nfb,
	unsigned long action, void *data);

static struct notifier_block g_kbox_die_notifier = {
	.notifier_call = kbox_die_notifier_callback,
	.priority = 110
};

static void kbox_show_stack(struct pt_regs *regs)
{
	if (!user_mode(regs)) {
		kbox_info("stack:");
		kbox_show_stack_info(regs);
	}
}

static void kbox_write_die_log(int fd, const unsigned int copy_log_len,
	struct die_args *data)
{
	int ret;
	struct die_args *die_data = data;

	if (die_data == NULL || die_data->regs == NULL) {
		kbox_err("data or die_data->regs is NULL.");
		return;
	}

	if (die_data->str != NULL) {
		kbox_info("die info:%s:%04lx.", die_data->str,
			((unsigned long)die_data->err & 0xffff));
	}
	kbox_store_pre_log(fd, copy_log_len);
	ret = kbox_regsiter_store_log();
	if (ret != 0) {
		kbox_err("kbox_regsiter_store_log fail, ret = %d.", ret);
	}
	kbox_show_regs(die_data->regs);
	kbox_show_stack(die_data->regs);
	kbox_show_module_info();
	kbox_unregister_store_log();
}

static void kbox_die_record_log(unsigned int reason, struct die_args *data)
{
	int ret;
	unsigned long time;

	time = (unsigned long)local_clock();
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

	kbox_write_die_log(g_kbox_log_fd, KBOX_COPY_PRINTK_LOG_LEN, data);

	ret = kbox_log_close(g_kbox_log_fd);
	if (ret != 0) {
		kbox_err("kbox_log_close fail.");
	}

	g_kbox_log_fd = -1;
}

static int kbox_die_notifier_callback(struct notifier_block *self, unsigned long type, void *data)
{
	int ret;
	int tmp_cpu;
	unsigned long flags;

	if (atomic_cmpxchg(&g_kbox_status, KBOX_STAT_INIT, KBOX_STAT_WORK) == KBOX_STAT_EXIT) {
		kbox_err_ratelimit("kbox is rmmoding, this die record will loss");
		return NOTIFY_OK;
	}

	/* ignore other types because callback could be triggered by debug traps such as int_3 */
	if (type != (unsigned long)DIE_OOPS)
		goto out;

	if (&g_kbox_die_notifier != self) {
		kbox_err("wrong callback for die notifier! self = 0x%pK", self);
		goto out;
	}

	if (data == NULL) {
		kbox_err("wrong callback for die notifier! msg is null.");
		goto out;
	}

	tmp_cpu = smp_processor_id();
	kbox_info("catch die event on cpu %d.", tmp_cpu);
	ret = kbox_event_pre_process(DIE_TYPE);
	if (ret != EVENT_SUBSEQUENT_OK) {
		kbox_err("kbox is recording some other exceptions.");
		goto out;
	}

	spin_lock_irqsave(&g_kbox_event_lock, flags);
	kbox_die_record_log((unsigned int)DIE_TYPE, (struct die_args *)data);
	kbox_notify_die(KBOX_DIE_PANIC);
	spin_unlock_irqrestore(&g_kbox_event_lock, flags);
out:
	atomic_set(&g_kbox_status, KBOX_STAT_INIT);
	return NOTIFY_OK;
}

int kbox_die_init(void)
{
	int ret;

	ret = register_die_notifier(&g_kbox_die_notifier);
	if (ret < 0) {
		kbox_err("die init register hook failed! ret = %d.", ret);
	}

	return ret;
}

void kbox_die_fini(void)
{
	(void)unregister_die_notifier(&g_kbox_die_notifier);
}
