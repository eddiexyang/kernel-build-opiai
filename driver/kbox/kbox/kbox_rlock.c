/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: kbox record panic scene log
 * Author: chenjialong
 * Create: 2019-03-13
 */
#include "kbox_rlock.h"

#include <linux/securec.h>
#include <linux/sched/clock.h>
#include <linux/irq_work.h>
#include <linux/delay.h>

#include "../driver/drv_oper.h"
#include "kbox_common.h"
#include "kbox_common_interface.h"
#include "kbox_find_sym_addr.h"
#include "kbox_print.h"
#include "kbox_printk_buf.h"
#include "kernel/printk/internal.h"

#define KBOX_NMIBUF_WAIT_MSEC     100
#define KBOX_NMIBUF_DELAY_MSEC    20

#ifndef CONFIG_PRINTK_SAFE_LOG_BUF_SHIFT
#define CONFIG_PRINTK_SAFE_LOG_BUF_SHIFT 13
#endif

#ifndef PRINTK_SAFE_CONTEXT_MASK
#define PRINTK_SAFE_CONTEXT_MASK 0
#endif

#ifndef PRINTK_NMI_CONTEXT_MASK
#define PRINTK_NMI_CONTEXT_MASK 0
#endif

#define SAFE_LOG_BUF_LEN ((1 << CONFIG_PRINTK_SAFE_LOG_BUF_SHIFT) - \
                sizeof(atomic_t) -          \
                sizeof(atomic_t) -          \
                sizeof(struct irq_work))
struct printk_safe_seq_buf {
	atomic_t        len;    /* length of written data */
	atomic_t        message_lost;
	struct irq_work     work;   /* IRQ work that flushes the buffer */
	unsigned char       buffer[SAFE_LOG_BUF_LEN];
};
static char g_printk_safe_tmp_buf[SAFE_LOG_BUF_LEN];
static int g_kbox_show_regs_rlock_once = 0;
static unsigned int g_kbox_online_cpus_num;
static char *g_interrupts_buf;
static int kbox_rlock_notifier_callback(struct notifier_block *self,
	unsigned long val, void *msg);

#ifdef CONFIG_PRINTK_NMI
static void kbox_wait_nmi_buf_flush(void)
{
	int query_cpu;
	int cur_cpu;
	int unfinished;
	size_t len;
	int timeout = KBOX_NMIBUF_WAIT_MSEC;
	struct printk_safe_seq_buf *s = NULL;

	cur_cpu = smp_processor_id();
	if (kbox_nmi_print_seq == NULL)
		return;
	while (timeout > 0) {
		mdelay(KBOX_NMIBUF_DELAY_MSEC);
		timeout -= KBOX_NMIBUF_DELAY_MSEC;
		unfinished = 0;
		for_each_online_cpu(query_cpu) {
			if (cur_cpu == query_cpu)
				continue;

			s = (struct printk_safe_seq_buf *)per_cpu_ptr(kbox_nmi_print_seq, query_cpu);
			if (s == NULL)
				continue;

			len = atomic_read(&s->len);
			if (len > 0) {
				unfinished++;
				break;
			}
		}
		if (unfinished == 0)
			break;
	}
}
#endif

static void kbox_print_rlock_log(char *msg, const struct pt_regs *regs)
{
	kbox_show_module_info();
	kbox_notify_smp_cpus_backtrace(msg);
#ifdef CONFIG_PRINTK_NMI
	/*
	 * when logbuf_lock is held, cpus backtrace will be stored in NMI buf and flushed to
	 * logbuf after NMI exits. So regs info and cpus backtrace may be outputed to kbox at
	 * the same time and leads to chaotic log. To avoid this, wait no longer than 100 ms
	 * for all NMI buffer to flush.
	 * Note: Do not use printk in this function.
	 */
	kbox_wait_nmi_buf_flush();
#endif
	if (g_kbox_show_regs_rlock_once == 0) {
		kbox_show_regs(regs);
		g_kbox_show_regs_rlock_once = 1;
	} else if (g_kbox_show_regs_rlock_once < INT_MAX) {
		/*
		 * g_kbox_show_regs_rlock_once will overflow if this function is called too many times.
		 * if rlock times more than INT_MAX since system startup, g_kbox_show_regs_rlock_once
		 * will not increase.
		 */
		g_kbox_show_regs_rlock_once++;
		kbox_printk("\nrlock %d times since system startup, ignore this rlock!\ncheck kbox_show_regs rlock "
				"info in previous kbox log.\n", g_kbox_show_regs_rlock_once);
	} else {
		kbox_printk("\nrlock times more than INT_MAX since system startup, ignore this rlock!\n");
	}
}

static inline int kbox_is_new_line(const char *buf)
{
	if (buf[0] != '\n' && buf[1] == KERN_SOH_ASCII && buf[2]) {
		switch (buf[2]) {
			case '0' ... '7':
			case 'd':   /* KERN_DEFAULT */
				return 1;
			default:
				return 0;
		}
	}
	return 0;
}

static void kbox_store_parse_safe_buf(int fd, const char *safe_buf, unsigned long len)
{
	const char *ptr = NULL;
	const char *end = NULL;
	int tmp_len = 0;
	int ret;

	if (safe_buf == NULL || len > SAFE_LOG_BUF_LEN) {
		kbox_info("kbox_store_parse_safe_buf err, len=%lu", len);
		return;
	}

	ptr = safe_buf;
	end = safe_buf + len;
	while (ptr < end) {
		if (ptr < end - 1) {
			ptr = printk_skip_level(ptr);
			if (ptr >= end) {
				break;
			}
		}
		g_printk_safe_tmp_buf[tmp_len++] = *ptr;
		if ((ptr < end - 2) && (kbox_is_new_line(ptr) == 1)) {
			g_printk_safe_tmp_buf[tmp_len++] = '\n';
		}
		ptr++;
	}
	if (tmp_len > 0 && tmp_len < SAFE_LOG_BUF_LEN &&
		g_printk_safe_tmp_buf[tmp_len - 1] != '\n') {
		g_printk_safe_tmp_buf[tmp_len] = '\n';
	}

	ret = kbox_log_write(fd, g_printk_safe_tmp_buf, tmp_len);
	if (ret != tmp_len) {
		kbox_err("save printk safe buf info fail!");
	}
}

#ifdef CONFIG_PRINTK_NMI
static void kbox_store_cpus_nmi_buf(int fd)
{
	int query_cpu;
	int cur_cpu;
	size_t len;
	struct printk_safe_seq_buf *s = NULL;

	cur_cpu = smp_processor_id();
	if (kbox_nmi_print_seq == NULL)
		return;

	for_each_online_cpu(query_cpu) {
		if (cur_cpu == query_cpu) {
			continue;
		}

		s = (struct printk_safe_seq_buf *)per_cpu_ptr(kbox_nmi_print_seq, query_cpu);
		if (s == NULL) {
			kbox_info("cpu%d nmi_print_seq is null, continue", query_cpu);
			continue;
		}

		len = atomic_read(&s->len);
		if (len == 0) {
			kbox_info("cpu%d nmi buf len=0, continue", query_cpu);
			continue;
		}
		if (len > sizeof(s->buffer)) {
			kbox_err("cpu%d nmi_print_seq internal error, buffer len=%lu", query_cpu, len);
			continue;
		}

		kbox_store_parse_safe_buf(fd, s->buffer, len);
	}
}
#else
static void kbox_store_cpus_nmi_buf(int fd)
{
	(void)fd;
}
#endif

void kbox_store_cur_safe_buf(int fd)
{
	struct printk_safe_seq_buf *s = NULL;
	unsigned long len;

	if (kbox_printk_context == NULL)
		return;

	if ((this_cpu_read(*kbox_printk_context) & PRINTK_SAFE_CONTEXT_MASK) &&
	    kbox_safe_print_seq != NULL) {
		s = (struct printk_safe_seq_buf *)this_cpu_ptr(kbox_safe_print_seq);
		len = atomic_read(&s->len);
		if (len > sizeof(s->buffer)) {
			kbox_err("current cpu safe_print_seq internal error, buffer len=%lu", len);
			return;
		}
		kbox_store_parse_safe_buf(fd, s->buffer, len);
	}
#ifdef CONFIG_PRINTK_NMI
	if ((this_cpu_read(*kbox_printk_context) & PRINTK_NMI_CONTEXT_MASK) &&
	    kbox_nmi_print_seq != NULL) {
		s = (struct printk_safe_seq_buf *)this_cpu_ptr(kbox_nmi_print_seq);
		len = atomic_read(&s->len);
		if (len > sizeof(s->buffer)) {
			kbox_err("current cpu nmi_print_seq internal error, buffer len=%lu", len);
			return;
		}
		kbox_store_parse_safe_buf(fd, s->buffer, len);
	}
#endif
}

static void kbox_write_rlock_log(int fd, const unsigned int copy_log_len, char *msg, const struct pt_regs *regs)
{
	int ret;

	kbox_store_pre_log(fd, copy_log_len);
	kbox_store_cur_safe_buf(fd);

	ret = kbox_regsiter_store_log();
	if (ret != 0) {
		kbox_err("kbox_regsiter_store_log fail, ret=%d", ret);
	}
	kbox_print_rlock_log(msg, regs);
	kbox_unregister_store_log();
	kbox_store_cpus_nmi_buf(fd);
}

static void kbox_rlock_record_log(unsigned int reason, char *msg, const struct pt_regs *regs)
{
	int ret;
	unsigned long time;

	time = (unsigned long)local_clock();
	ret = kbox_write_reset_reason(reason, time);
	if (ret != 0) {
		kbox_err("kbox_write_reset err ret=%d", ret);
		return;
	}

	g_kbox_log_fd = kbox_log_open(time);
	if (g_kbox_log_fd < 0) {
		kbox_err("kbox_log_open fail.");
		return;
	}
	kbox_write_rlock_log(g_kbox_log_fd, KBOX_COPY_PRINTK_LOG_LEN_RLOCK, msg, regs);
	ret = kbox_log_close(g_kbox_log_fd);
	if (ret != 0) {
		kbox_err("kbox_log_close fail.");
		return;
	}

	g_kbox_log_fd = -1;
}

static struct notifier_block g_kbox_rlock_notifier = {
	.notifier_call = kbox_rlock_notifier_callback,
	.priority = 110,
};

static int kbox_rlock_notifier_callback(struct notifier_block *self,
	unsigned long val, void *msg)
{
	int tmp_cpu;
	int ret;
	unsigned long flags;
	char *rlock_reason = (msg ? (char *)msg : RLOCK_REASON_UNKWN);
	struct pt_regs *regs = (struct pt_regs *)val;
	if (atomic_cmpxchg(&g_kbox_status, KBOX_STAT_INIT, KBOX_STAT_WORK) == KBOX_STAT_EXIT) {
		kbox_err_ratelimit("kbox is rmmoding, this rlock record will loss");
		return NOTIFY_DONE;
	}

	if (&g_kbox_rlock_notifier != self) {
		kbox_err("wrong callback for rlock chain! self = 0x%pK", self);
		goto out;
	}

	tmp_cpu = smp_processor_id();

	kbox_info("catch rlock event on cpu %d, rlock reason:%s", tmp_cpu, rlock_reason);

	ret = kbox_event_pre_process(RLOCK_TYPE);
	if (ret != EVENT_SUBSEQUENT_OK) {
		kbox_err("kbox is recording some other exceptions.");
		goto out;
	}

	spin_lock_irqsave(&g_kbox_event_lock, flags);
	kbox_rlock_record_log((unsigned int)RLOCK_TYPE, rlock_reason, regs);
	kbox_notify_die(KBOX_DIE_CPULOCK);
	spin_unlock_irqrestore(&g_kbox_event_lock, flags);
out:
	atomic_set(&g_kbox_status, KBOX_STAT_INIT);
	return NOTIFY_DONE;
}

int kbox_rlock_init(void)
{
	int ret;

	g_kbox_online_cpus_num = num_online_cpus();
	if (g_kbox_online_cpus_num > MAX_CPUS) {
		kbox_info("online cpu number > %d, kbox do not init rlock event.\n", MAX_CPUS);
		return 0;
	}

	g_interrupts_buf = vmalloc(INT_BUF_SIZE);
	if (g_interrupts_buf == NULL) {
		return -ENOMEM;
	}

	ret = memset_s(g_interrupts_buf, INT_BUF_SIZE, 0, INT_BUF_SIZE);
	if (ret != 0) {
		vfree(g_interrupts_buf);
		g_interrupts_buf = NULL;
		return -EIO;
	}

	if (kbox_watchdog_notifier_list == NULL) {
		vfree(g_interrupts_buf);
		g_interrupts_buf = NULL;
		kbox_err("watchdog_notifier_list not found.");
		return -ENXIO;
	}

	ret = atomic_notifier_chain_register(kbox_watchdog_notifier_list,
		&g_kbox_rlock_notifier);
	if (ret < 0) {
		vfree(g_interrupts_buf);
		g_interrupts_buf = NULL;
		kbox_err("rlock register watchdog notifier chain failed! ret=%d", ret);
	}

	return ret;
}

/****************************************************************************
description:    cleanup rlock module

****************************************************************************/
void kbox_rlock_fini(void)
{
	if (g_kbox_online_cpus_num > MAX_CPUS) {
		return;
	}

	if (kbox_watchdog_notifier_list != NULL) {
		(void)atomic_notifier_chain_unregister(kbox_watchdog_notifier_list,
			&g_kbox_rlock_notifier);
	}
	if (g_interrupts_buf != NULL) {
		vfree(g_interrupts_buf);
		g_interrupts_buf = NULL;
	}
}
