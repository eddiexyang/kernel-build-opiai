/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: kbox record panic scene log
 * Author: wangshouping
 * Create: 2019-02-14
 */
#include "kbox_panic.h"

#include <linux/notifier.h>
#include <linux/sched/clock.h>
#include <linux/delay.h>

#include "../driver/drv_oper.h"
#include "kbox_common.h"
#include "kbox_common_interface.h"
#include "kbox_emerge.h"
#include "kbox_main.h"
#include "kbox_print.h"
#include "kbox_printk_buf.h"
#include "kbox_rlock.h"

#define TRY_SPINLOCK_MSEC 3000

static int kbox_panic_notifier_callback(struct notifier_block *self,
	unsigned long val, void *cookie);

static struct notifier_block kbox_panic_notifier = {
	.notifier_call = kbox_panic_notifier_callback,
	.priority = 110,
};

static void kbox_write_panic_log(int fd, const unsigned int copy_log_len)
{
	kbox_info("stack:");
	kbox_show_stack_info(NULL);
	kbox_store_pre_log(fd, copy_log_len);
	kbox_store_cur_safe_buf(fd);
}

static void kbox_panic_record_log(unsigned int reason)
{
	int ret;
	unsigned long time;

	time = (unsigned long)local_clock();
	ret = kbox_write_reset_reason(reason, time);
	if (ret != 0) {
		kbox_err("kbox_write_reset err, ret=%d.", ret);
		return;
	}

	g_kbox_log_fd = kbox_log_open(time);
	if (g_kbox_log_fd < 0) {
		kbox_err("kbox_log_open fail.");
		return;
	}

	kbox_write_panic_log(g_kbox_log_fd, KBOX_COPY_PRINTK_LOG_LEN);

	ret = kbox_log_close(g_kbox_log_fd);
	if (ret != 0) {
		kbox_err("kbox_log_close fail.");
	}
	g_kbox_log_fd = -1;
}

static int check_painc_type_die(const char *cookie)
{
	if ((config & CFG_DIE_FLAG) &&
		strstr(cookie, PANIC_ON_DIE)) {
		kbox_info("panic notifier callback. die event has been record!");
		return PANIC_RECORDED;
	}
	return 0;
}

static int check_panic_type_oom(const char *cookie)
{
	if ((config & CFG_OOM_FLAG) &&
		!strncmp(cookie, (char *)PANIC_ON_OOM, PANIC_ON_OOM_LEN)) {
		kbox_info("panic notifier callback. oom event has been record!");
		return PANIC_RECORDED;
	}
	return 0;
}

static int check_panic_type_rlock(const char *cookie)
{
	unsigned int kbox_online_cpus_num = num_online_cpus();

	if (kbox_online_cpus_num > MAX_CPUS) {
		kbox_info("online cpu number > %d, panic record rlock event log.", MAX_CPUS);
		goto out;
	}

	if ((config & CFG_RLOCK_FLAG) &&
		((!strncmp(cookie, (char *)PANIC_ON_SOFTDOG, PANIC_ON_SOFTDOG_LEN)) ||
		(!strncmp(cookie, (char *)PANIC_ON_HARDDOG, PANIC_ON_HARDDOG_LEN)))) {
		kbox_info("rlock event has been record!");
		return PANIC_RECORDED;
	}
out:
	return 0;
}

static int check_panic_type(const char *cookie)
{
	int ret = 0;

	if (cookie == NULL) {
		kbox_err("cookie is NULL, do not record log.");
		goto out;
	}

	ret = check_painc_type_die(cookie);
	if (ret == PANIC_RECORDED) {
		goto out;
	}

	ret = check_panic_type_oom(cookie);
	if (ret == PANIC_RECORDED) {
		kbox_notify_die(KBOX_DIE_PANIC);
		goto out;
	}

	ret = check_panic_type_rlock(cookie);
	if (ret == PANIC_RECORDED) {
		kbox_notify_die(KBOX_DIE_PANIC);
		goto out;
	}
out:
	return ret;
}

static int try_get_spinlock_irq(spinlock_t *spinlock, unsigned long *irq_flags, int timeout)
{
	int count = 0;
	unsigned long flags;

	timeout = timeout / 10;
	while (count < timeout) {
		if (spin_trylock_irqsave(spinlock, flags))
			break;
		count++;
		mdelay(10);
	}
	if (count == timeout) {
		return 0;
	}
	*irq_flags = flags;
	return 1;
}

static int kbox_panic_notifier_callback(struct notifier_block *self,
	unsigned long val, void *cookie)
{
	char *panic_reason = (cookie ? cookie : PANIC_REASON_UNKOW);
	int ret;
	unsigned long flags;

	if (atomic_cmpxchg(&g_kbox_status, KBOX_STAT_INIT, KBOX_STAT_WORK) == KBOX_STAT_EXIT) {
		kbox_err_ratelimit("kbox is rmmoding, this panic record will loss");
		return NOTIFY_DONE;
	}

	if (&kbox_panic_notifier != self) {
		kbox_err("panic notifier callback. wrong callback for panic chain!");
		goto out;
	}

	kbox_info("catch panic event, panic reason:%s", panic_reason);
	ret = kbox_event_pre_process(PANIC_TYPE);
	if (ret != EVENT_SUBSEQUENT_OK) {
		goto out;
	}

	ret = check_panic_type((char *)cookie);
	if (ret != 0) {
		set_bit(PANIC_TYPE, &g_emerge_recorded_flags);
		goto out;
	}

	/*
	 * on x86 arch, panic can use nmi to stop other cpus,
	 * use spin_trylock_irqsave before panic event process.
	 */
	if (try_get_spinlock_irq(&g_kbox_event_lock, &flags, TRY_SPINLOCK_MSEC) == 0) {
		kbox_err("kbox_panic_notifier_callback get g_kbox_event_lock fail!");
		goto out;
	}
	set_bit(PANIC_TYPE, &g_emerge_recorded_flags);
	kbox_panic_record_log((unsigned int)PANIC_TYPE);
	kbox_notify_die(KBOX_DIE_PANIC);
	spin_unlock_irqrestore(&g_kbox_event_lock, flags);
out:
	kbox_info("end panic event.");
	atomic_set(&g_kbox_status, KBOX_STAT_INIT);
	return NOTIFY_DONE;
}

int kbox_panic_init(void)
{
	int ret;

	ret = atomic_notifier_chain_register(&panic_notifier_list, &kbox_panic_notifier);
	if (ret < 0) {
		kbox_info("panic init register hook failed! ret=%d.", ret);
		return ret;
	}

	return ret;
}

void kbox_panic_fini(void)
{
	(void)atomic_notifier_chain_unregister(&panic_notifier_list, &kbox_panic_notifier);
}
