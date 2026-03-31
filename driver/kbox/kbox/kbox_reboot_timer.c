/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Author      : wangshouping
 * Create      : 2019-02-14
 * Description : process the kernel reboot timeout
 */
#include "kbox_reboot_timer.h"

#include <linux/mm.h>
#include <linux/mutex.h>
#include <linux/nmi.h>
#include <linux/time64.h>
#include <linux/timer.h>
#include <linux/sched/clock.h>
#include <linux/sched/signal.h>

#include "../driver/drv_oper.h"
#include "kbox_common.h"
#include "kbox_common_interface.h"
#include "kbox_find_sym_addr.h"
#include "kbox_print.h"
#include "kbox_printk_buf.h"

#define MAX_SHOW_THREAD_NUM 15

/* unit: seconds time:5mins */
#define DEFAILT_INTERVAL (300 * HZ)
#define WARNNING_INTERVAL (3 * 300 * HZ)
static struct timer_list g_reboot_timer;
atomic_t g_reboot_timer_exist = ATOMIC_INIT(0);
atomic_t g_reboot_timer_up = ATOMIC_INIT(1);
static unsigned int g_log_num;

#define KBOX_REBOOT_TIMER_RECORD_LOG_NUM 2
unsigned int g_kbox_record_log_num = KBOX_REBOOT_TIMER_RECORD_LOG_NUM;
module_param(g_kbox_record_log_num, uint, 0400);

static void kbox_show_task(int num)
{
	struct task_struct *g = NULL;
	struct task_struct *p = NULL;
	int type = num;
	int i = 0;

	if (kbox_tasklist_lock == NULL) {
		kbox_err("kbox_tasklist_lock is null.");
		return;
	}
	if (!read_trylock(kbox_tasklist_lock)) {
		kbox_err("kbox_show_task try get tasklist_lock fail!");
		return;
	}
	for_each_process_thread(g, p) {
		/*
		 * reset the NMI-timeout, listing all files on a slow
		 * console might take a lot of time:
		 */
		touch_nmi_watchdog();
		switch (type) {
			case DSTAT_TASK:
				if (READ_ONCE(p->__state) & TASK_UNINTERRUPTIBLE) {
					kbox_sched_show_task(p);
					++i;
				}
				if (i >= MAX_SHOW_THREAD_NUM) {
					kbox_info("only show %d d staus threads to avoid a lot output.",
						MAX_SHOW_THREAD_NUM);
					goto out;
				}
				break;
			case ALL_TASK:
				kbox_sched_show_task(p);
				break;
			default:
				break;
		}
	}
out:
	read_unlock(kbox_tasklist_lock);
}
static void kbox_write_reboot_timer_log(int fd, const unsigned int copy_log_len)
{
	int ret;

	kbox_store_pre_log(fd, copy_log_len);

	ret = kbox_regsiter_store_log();
	if (ret != 0) {
		kbox_err("kbox_regsiter_store_log fail, ret = %d.", ret);
	}

	kbox_info("first reboot timer fire...");
	kbox_info("preint reason: the system is not reset after the reset command is executed.");
	kbox_info("DEFAILT_INTERVAL = %d s.", DEFAILT_INTERVAL / HZ);
	kbox_info("current process: %s (pid: %d, tgid: %d) on CPU: %d.",
		  current->comm, current->pid, current->tgid, smp_processor_id());
	kbox_info("first show d status task start.");
	kbox_show_task(DSTAT_TASK);
	kbox_info("first show d status task end.");
	atomic_set(&g_reboot_timer_up, 0);

	kbox_unregister_store_log();
}

static int kbox_reboot_timer_first_record_log(unsigned int reason, unsigned long time)
{
	int ret;

	ret = kbox_write_reset_reason(reason, time);
	if (ret != 0) {
		kbox_err("kbox_write_reset err, ret=%d.", ret);
		return ret;
	}
	g_kbox_log_fd = kbox_log_open(time);
	if (g_kbox_log_fd < 0) {
		kbox_err("kbox_log_open fail.");
		return ret;
	}
	kbox_write_reboot_timer_log(g_kbox_log_fd, KBOX_COPY_PRINTK_LOG_LEN);
	ret = kbox_log_close(g_kbox_log_fd);
	if (ret != 0) {
		kbox_err("kbox_log_close fail.");
	}
	g_kbox_log_fd = -1;
	return ret;
}

static void kbox_reboot_timer_record_log_info(void)
{
	kbox_info("current process: %s (pid: %d, tgid: %d) on CPU: %d.",
		current->comm, current->pid, current->tgid, smp_processor_id());
	kbox_info("show d status task start.");
	kbox_show_task(DSTAT_TASK);
	kbox_info("show d status task end.");
}

static int kbox_reboot_timer_record_log_max(void)
{
	int ret = 0;

	if (g_log_num >= g_kbox_record_log_num) {
		kbox_info("reboot timer record log num has been max : %u.", g_kbox_record_log_num);
		atomic_set(&g_reboot_timer_up, 1);
		atomic_set(&g_reboot_timer_exist, 0);
		g_log_num = 0;
		ret = -1;
	}
	return ret;
}

static void kbox_reboot_timer_record_log(unsigned int reason)
{
	int ret;
	unsigned long time;

	time = (unsigned long)local_clock();
	/* timer first shoot, recored message to kbox */
	if (atomic_read(&g_reboot_timer_up)) {
		ret = kbox_reboot_timer_first_record_log(reason, time);
		if (ret != 0) {
			return;
		}
	} else {
		kbox_set_console_level(KBOX_CONSOLE_VERBORS_LEVEL);
		kbox_reboot_timer_record_log_info();
		kbox_restore_console_level();
	}
	g_log_num++;
}

static void process_show_callback(void)
{
	unsigned long flags;

	spin_lock_irqsave(&g_kbox_event_lock, flags);
	kbox_reboot_timer_record_log((unsigned int)PREINT_TYPE);
	spin_unlock_irqrestore(&g_kbox_event_lock, flags);
}

static void reboot_timer_fn(struct timer_list *a)
{
	int ret;

	if (atomic_cmpxchg(&g_kbox_status, KBOX_STAT_INIT, KBOX_STAT_WORK) == KBOX_STAT_EXIT) {
		kbox_err("kbox is rmmoding, this record will loss.");
		return;
	}

	ret = kbox_reboot_timer_record_log_max();
	if (ret == -1) {
		kbox_info("reboot timer do not record log.");
		atomic_set(&g_kbox_status, KBOX_STAT_INIT);
		return;
	}

	process_show_callback();
	mod_timer(&g_reboot_timer, jiffies + WARNNING_INTERVAL);
	atomic_set(&g_kbox_status, KBOX_STAT_INIT);
}

void reboot_timer_init(void)
{
	kbox_info("kbox: the timer for monitor reboot event is starting...");
	timer_setup(&g_reboot_timer, reboot_timer_fn, 0);
	g_reboot_timer.expires = jiffies + DEFAILT_INTERVAL;
	g_reboot_timer.function = reboot_timer_fn;
	add_timer(&g_reboot_timer);
	atomic_set(&g_reboot_timer_exist, 1);
	g_log_num = 0;
	kbox_info("kbox: the timer for monitor reboot event is started...");
}

void reboot_timer_fini(void)
{
	/* dele timer */
	if (atomic_read(&g_reboot_timer_exist)) {
		del_timer_sync(&g_reboot_timer);
	}
	atomic_set(&g_reboot_timer_exist, 0);
}
