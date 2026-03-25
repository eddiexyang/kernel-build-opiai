/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
* Description: process the kernel restart event
* Author:
* Create: 2019-03-12
*/

#include "kbox_emerge.h"

#include <linux/kallsyms.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/sched/clock.h>

#include "../driver/drv_oper.h"
#include "kbox_common.h"
#include "kbox_common_interface.h"
#include "kbox_find_sym_addr.h"
#include "kbox_print.h"
#include "kbox_printk_buf.h"

static int kbox_emergency_restart_callback(struct notifier_block *self,
	unsigned long val, void *cookie);
static int kbox_kernel_reboot_notifier_callback(struct notifier_block *self,
	unsigned long val, void *msg);
unsigned long g_emerge_recorded_flags;

static struct notifier_block g_kbox_emergency_restart_notifier = {
	.notifier_call = kbox_emergency_restart_callback,
	.priority = 100
};

static struct notifier_block g_kbox_kernel_reboot_notifier = {
	.notifier_call = kbox_kernel_reboot_notifier_callback,
	.priority = 100
};

static bool kbox_event_reboot_is_record(void)
{
	bool ret = false;

	if (test_bit(REBOOT_TYPE, &g_emerge_recorded_flags)) {
		kbox_err("reboot log has been record, ignore it.");
		ret = true;
	}
	return ret;
}

static bool kbox_event_is_record(void)
{
	bool ret = false;

	ret = kbox_event_reboot_is_record();
	if (test_bit(PANIC_TYPE, &g_emerge_recorded_flags) || ret == true) {
		kbox_err(
			"kbox detect emerge event, but it seems call from panic or reboot. ignore it.");
		ret = true;
	}

	return ret;
}

static void kbox_write_emerge_log(int fd, const unsigned int copy_log_len)
{
	kbox_info("current process: %s (pid: %d, tgid: %d) on CPU: %d.",
		current->comm, current->pid, current->tgid, smp_processor_id());
	dump_stack();
	kbox_store_pre_log(fd, copy_log_len);
}

static void kbox_emergency_record_log(unsigned int reason)
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
	kbox_write_emerge_log(g_kbox_log_fd, KBOX_COPY_PRINTK_LOG_LEN);

	ret = kbox_log_close(g_kbox_log_fd);
	if (ret != 0) {
		kbox_err("kbox_log_close fail, ret = %d.", ret);
	}
	g_kbox_log_fd = -1;
}

static int kbox_emergency_restart_callback(struct notifier_block *self,
	unsigned long val, void *msg)
{
	int ret;
	int tmp_cpu;
	unsigned long flags;
	bool record_flag = false;

	if (atomic_cmpxchg(&g_kbox_status, KBOX_STAT_INIT, KBOX_STAT_WORK) == KBOX_STAT_EXIT) {
		kbox_err_ratelimit("kbox is rmmoding, this emerge record will loss");
		return NOTIFY_DONE;
	}

	if (&g_kbox_emergency_restart_notifier != self) {
		kbox_err("emergency restart callback failed. invalid self = 0x%pK",
			self);
		goto out;
	}

	tmp_cpu = smp_processor_id();
	kbox_info("catch kernel emergency_restart event on cpu %d.", tmp_cpu);
	ret = kbox_event_pre_process(EMERGE_TYPE);
	if (ret != EVENT_SUBSEQUENT_OK) {
		goto out;
	}

	spin_lock_irqsave(&g_kbox_event_lock, flags);

	record_flag = kbox_event_is_record();
	if (record_flag == true) {
		kbox_info("record_flag is %d, log has been record.", record_flag);
		spin_unlock_irqrestore(&g_kbox_event_lock, flags);
		goto out;
	}

	kbox_emergency_record_log((unsigned int)EMERGE_TYPE);
	kbox_notify_die(KBOX_DIE_PANIC);

	spin_unlock_irqrestore(&g_kbox_event_lock, flags);
out:
	atomic_set(&g_kbox_status, KBOX_STAT_INIT);
	return NOTIFY_DONE;
}
static int kbox_kernel_reboot_notifier_callback(struct notifier_block *self,
	unsigned long val, void *msg)
{
	int ret;
	int tmp_cpu;
	unsigned long flags;
	bool record_flag = false;

	char *restart_reason = (msg ? (char *)msg : "unknown");

	if (atomic_cmpxchg(&g_kbox_status, KBOX_STAT_INIT, KBOX_STAT_WORK) == KBOX_STAT_EXIT) {
		kbox_err_ratelimit("kbox is rmmoding, this emerge record will loss");
		return NOTIFY_DONE;
	}

	if (&g_kbox_kernel_reboot_notifier != self) {
		kbox_err("reboot notifier callback failed. invalid self = 0x%pK",
			self);
		goto out;
	}

	tmp_cpu = smp_processor_id();
	kbox_info("catch emerge event on cpu %d, kernel reboot info: %s.", tmp_cpu,
		restart_reason);
	ret = kbox_event_pre_process(EMERGE_TYPE);
	if (ret != EVENT_SUBSEQUENT_OK) {
		goto out;
	}

	spin_lock_irqsave(&g_kbox_event_lock, flags);
	record_flag = kbox_event_reboot_is_record();
	if (record_flag == true) {
		kbox_info("record_flag is %d, log has been record.", record_flag);
		spin_unlock_irqrestore(&g_kbox_event_lock, flags);
		goto out;
	}

	kbox_emergency_record_log((unsigned int)EMERGE_TYPE);
	kbox_notify_die(KBOX_DIE_PANIC);
	spin_unlock_irqrestore(&g_kbox_event_lock, flags);
out:
	atomic_set(&g_kbox_status, KBOX_STAT_INIT);
	return NOTIFY_DONE;
}

int kbox_emerge_init(void)
{
	int ret;

	ret = register_reboot_notifier(&g_kbox_kernel_reboot_notifier);
	if (ret < 0) {
		kbox_err("emerge init register hook failed! ret = %d.", ret);
		return ret;
	}

	if (kbox_emerge_notifier_list == NULL) {
		(void)unregister_reboot_notifier(&g_kbox_kernel_reboot_notifier);
		kbox_err("emerge_notifier_list not found.");
		return -ENXIO;
	}

	ret = atomic_notifier_chain_register(kbox_emerge_notifier_list,
		&g_kbox_emergency_restart_notifier);
	if (ret < 0) {
		kbox_err("register emerge hook failed! ret = %d.", ret);
		(void)unregister_reboot_notifier(&g_kbox_kernel_reboot_notifier);
		return ret;
	}

	return ret;
}

void kbox_emerge_fini(void)
{
	if (kbox_emerge_notifier_list != NULL) {
		(void)atomic_notifier_chain_unregister(kbox_emerge_notifier_list,
			&g_kbox_emergency_restart_notifier);
	}
	(void)unregister_reboot_notifier(&g_kbox_kernel_reboot_notifier);
}
