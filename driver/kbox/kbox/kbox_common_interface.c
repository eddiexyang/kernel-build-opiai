/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: kbox common interface
 * Author: wangshouping
 * Create: 2019-02-14
 */
#include "kbox_common_interface.h"

#include <linux/os_kbox.h>
#include <linux/securec.h>
#include <linux/smp.h>
#include <linux/spinlock.h>
#include <linux/nmi.h>
#include <linux/kprobes.h>

#include "../driver/drv_oper.h"
#include "kbox_common.h"
#include "kbox_find_sym_addr.h"
#include "kbox_main.h"
#include "kbox_print.h"

#ifdef CONFIG_X86_64
#include "kbox_x86_64.h"
#endif
#ifdef CONFIG_ARM64
#include "kbox_arm64.h"
#endif

#define KBOX_VM_AREA   0x04

#define KBOX_BUF_SIZE 256
char g_kbox_buf[KBOX_BUF_SIZE];

#define SHOW_MAX_LINE 3000

#define UINT_BYTE_NUM        (sizeof(unsigned int))
/* print 32 bytes per line */
#define LINE_BYTES 32
/* 8-byte alignment */
#define BYTE_ALIGN_MASK 3
#define LINE_ALIGN_MASK    (LINE_BYTES - 1)
#define ADDRESS_ALIGN_MASK ((-1UL) << BYTE_ALIGN_MASK)

static unsigned int g_kbox_counters[MAX_NUMNODES] = {0};
char *g_kbox_event[] = {
	UNKNOWN_STR, PANIC_STR, DIE_STR, OOM_STR,
	REBOOT_STR, EMERGE_STR, RLOCK_STR, PREINT_STR, FS_EXCEPTION_STR };
int g_record_cpu = -1;
static int g_console_printk_org;
char *g_die_notify_func = NULL;

atomic_t g_kbox_printk_flag = ATOMIC_INIT(0);

#define TMP_BUF_LEN 1200
char g_kbox_printk_buf[TMP_BUF_LEN] = {0};

kallsyms_lookup_name_kprobe_t kallsyms_lookup_name_kprobe;

module_param(g_die_notify_func, charp, S_IRUSR);

char *kbox_event_to_string(unsigned int event)
{
	if ((event > TYPE_PROTECT) && (event < TYPE_BUTTON)) {
		return g_kbox_event[event];
	}
	return g_kbox_event[0];
}

/*
 * Description : kbox die notify function
 * Param : reason--> event type
 */
void kbox_notify_die(enum kbox_die_reason reason)
{
	typedef int fn_notify_die_func_addr(int);
	fn_notify_die_func_addr *func_addr = NULL;

	kbox_info("notify die begin");
	if (g_die_notify_func == NULL) {
		kbox_info("no notify die func register. no need to notify");
		return;
	} else {
		func_addr = (fn_notify_die_func_addr*) kallsyms_lookup_name_kprobe(g_die_notify_func);
		if (func_addr == NULL) {
			kbox_info("notify die interface not found");
			return;
		}
		(void)func_addr((int)reason);
		kbox_info("notify the die by func %s,die reason is %d", g_die_notify_func, reason);
	}
}

static void kbox_store_log_fn(u64 tm, const char *text, const size_t text_len)
{
	char time_buf[TIME_BUF_LEN] = {0};
	u64 ts_usec;
	int time_len;
	int ret;

	if (kbox_get_nospace_flag() == 1) {
		return;
	}

	if (text == NULL || text_len <= 0) {
		return;
	}

	if (atomic_read(&g_kbox_printk_flag) == 1) {
		return;
	}

	/* collect timestamp */
	ts_usec = tm / TIME_USEC;
	time_len = snprintf_s(time_buf, sizeof(time_buf), sizeof(time_buf) - 1,
		"[%5llu.%6llu] ", ts_usec / TIME_USEC_TO_SEC, ts_usec % TIME_USEC_TO_SEC);
	if (time_len == -1) {
		return;
	}

	if (time_len > 0) {
		ret = memcpy_s(&g_kbox_printk_buf[0], sizeof(g_kbox_printk_buf), time_buf, time_len);
		if (ret == 0) {
			(void)memcpy_s(&g_kbox_printk_buf[time_len], (size_t)(sizeof(g_kbox_printk_buf) - time_len), text, text_len);
		}
	}
	(void)kbox_log_write(g_kbox_log_fd, g_kbox_printk_buf, time_len + text_len);
}

/*
 * Description : regsiter printk hook function
 * Param : return value-->OK(0)
 */
int kbox_regsiter_store_log(void)
{
	int ret;

	if (kbox_register_store_interface_ptr == NULL) {
		kbox_err("kbox_register_store_interface not found.");
		return -ENXIO;
	}

	ret = kbox_register_store_interface_ptr((void *)kbox_store_log_fn);
	if (ret != 0) {
		kbox_err("kbox_register_store_interface err.");
		return -EINVAL;
	}
	return 0;
}
/*
 * Description : unregister printk hook function
 */
void kbox_unregister_store_log(void)
{
	if (kbox_unregister_store_interface_ptr != NULL) {
		kbox_unregister_store_interface_ptr();
	}
}

/*
 * Description : process events concurrency
 * the same cpu occurred concurrency, keep the first record log
 * Param : return value--> EVENT_SUBSEQUENT_OK(record log);EVENT_SUBSEQUENT_FAIL
 *	   (not record current event)
 */
int kbox_event_pre_process(enum log_reset_type event)
{
	bool is_locked = TRUE;
	int cur_cpu;

	cur_cpu = smp_processor_id();
	is_locked = spin_is_locked(&g_kbox_event_lock);
	/* the same cpu occurred concurrency */
	if (is_locked && cur_cpu == g_record_cpu) {
		/* concurrency occurred, keep the first record log */
		kbox_err("the same cpu occurred concurrency, event:%d, cpu:%d", event,
			cur_cpu);
		return EVENT_SUBSEQUENT_FAIL;
	}
	g_record_cpu = smp_processor_id();
	return EVENT_SUBSEQUENT_OK;
}

static void *kbox_module_core_base(const struct module *mod)
{
	void *base = NULL;

	for_class_mod_mem_type(type, core) {
		if (mod->mem[type].base == NULL || mod->mem[type].size == 0) {
			continue;
		}

		if (base == NULL || mod->mem[type].base < base) {
			base = mod->mem[type].base;
		}
	}

	return base;
}

static unsigned long kbox_module_core_size(const struct module *mod)
{
	unsigned long size = 0;

	for_class_mod_mem_type(type, core) {
		size += mod->mem[type].size;
	}

	return size;
}

void kbox_show_module_info(void)
{
	unsigned int print_mod_count = 0;
	struct module *mod = NULL;
	struct list_head *kbox_modules = kbox_modules_ptr;

	if (kbox_modules_ptr == NULL) {
		kbox_err("get modules ptr is null.");
		return;
	}

	preempt_disable();

	kbox_printk("the modules info in current system:\n");
	kbox_printk("%-24s %-20s   %-12s\n", "mod_name", "mod_start", "core_size");

	list_for_each_entry_rcu(mod, kbox_modules, list) {
		kbox_printk("%-24s 0x%-20px 0x%-12lx\n", mod->name, kbox_module_core_base(mod),
			kbox_module_core_size(mod));
		print_mod_count = print_mod_count + 1;
		if (print_mod_count >= MAX_PRINT_MODULE) {
			kbox_info("the count of modules in system exceeds %u, exit.",
				MAX_PRINT_MODULE);
			break;
		}
	}

	preempt_enable();
}

static void kbox_vprintk(const char *fmt, va_list args)
{
	int len;
	int ret;

	if (kbox_get_nospace_flag() == 1) {
		return;
	}

	len = vscnprintf(g_kbox_buf, KBOX_BUF_SIZE, fmt, args);
	if (len == 0) {
		return;
	}
	ret = kbox_log_write(g_kbox_log_fd, g_kbox_buf, (unsigned int)len);
	if (ret < 0) {
		kbox_err("store kbox log fail. ret=%d.", ret);
	}
}

void kbox_printk(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	kbox_vprintk(fmt, args);
	va_end(args);
}

static int kbox_print_seq_info(struct seq_file *p)
{
	int ret;
	int check_ret;

	ret = kbox_log_write(g_kbox_log_fd, p->buf, p->count);
	if (ret != p->count) {
		kbox_err("save vmallocinfo fail.");
	}
	check_ret = memset_s(p->buf, p->size, 0, p->size);
	if (check_ret != 0) {
		kbox_err("kbox_print_seq_info memset_s fail, ret=%d.", check_ret);
	}
	p->count = 0;
	return ret;
}

void kbox_show_seq_info(int type)
{
	struct seq_file m;
	struct seq_operations *kbox_seq_op = NULL;
	void *p = NULL;
	loff_t pos = 0;
	unsigned int show_num = 0;
	int ret;
	struct vmap_area *va = NULL;
	struct vm_struct *v = NULL;
	unsigned long size = 0;

	if (type != KBOX_VMALLOC_OP) {
		kbox_err("type is err.");
		return;
	}

	m.buf = g_tmp_buf;
	m.index = 0;
	m.count = 0;
	m.read_pos = 0;
	m.size = PAGE_SIZE;
	m.private = g_kbox_counters;

	kbox_seq_op = kbox_vmalloc_op_ptr;

	p = kbox_seq_op->start(&m, &pos);
	while (p != NULL && !IS_ERR(p)) {
		if (show_num < SHOW_MAX_LINE) {
			(void)kbox_seq_op->show(&m, p);
			ret = kbox_print_seq_info(&m);
			if (ret < 0) {
				/* when write kbox memory error, skip save vmallocinfo to avoid continuous error print,
				 * but continue statistic vmalloc real occupy physical memory size.
				 */
				show_num = SHOW_MAX_LINE + 1;
			}
		}
		touch_nmi_watchdog();
		if (show_num == SHOW_MAX_LINE) {
			kbox_info("vmalloc info num over SHOW_MAX_LINE=%d!", SHOW_MAX_LINE);
		}
		show_num++;
		va = list_entry(p, struct vmap_area, list);
		if (va->vm) {
			v = va->vm;
			if (v->flags & VM_ALLOC) {
				size += v->size;
			}
		}
		p = kbox_seq_op->next(&m, p, &pos);
	}
	kbox_seq_op->stop(&m, p);
	kbox_info("vmalloc really occupy physical memory size %lu bytes.", size);
}

/*
 * description: set console log level to specifed level
 */
void kbox_set_console_level(int new_log_level)
{
	g_console_printk_org = (*kbox_console_printk);

	if (new_log_level != g_console_printk_org) {
		*kbox_console_printk = new_log_level;
	}
}

/*
 * description: restore console log level
 */
void kbox_restore_console_level(void)
{
	*kbox_console_printk = g_console_printk_org;
}

static void kbox_print_cpus_trace(unsigned long info)
{
	dump_stack();
	kbox_wdog_print_stat();
}

#ifdef CONFIG_ARM64
void kbox_show_stack_info(struct pt_regs *regs)
{
	kbox_show_stack_log_lvl(regs);
}

void kbox_notify_smp_cpus_backtrace(char *msg)
{
	unused(msg);
	if (kbox_arch_trigger_cpumask_backtrace != NULL) {
		kbox_arch_trigger_cpumask_backtrace(cpu_online_mask, smp_processor_id());
	}
}
#endif

#ifdef CONFIG_X86_64
void kbox_show_stack_info(struct pt_regs *regs)
{
	kbox_show_stack_x86_64(regs);
}

void kbox_notify_smp_cpus_backtrace(char *msg)
{
	unused(msg);
	if (kbox_arch_trigger_cpumask_backtrace != NULL) {
		kbox_arch_trigger_cpumask_backtrace(cpu_online_mask, smp_processor_id());
	}
}
#endif

/* print a block of kernel memory around the address given by kbox_do_show_regs(). */
void kbox_show_memblock(unsigned long address, int bytes, const char *reg_name, unsigned long reg_address)
{
	int dump_bytes;
	unsigned int temp_data;
	unsigned int  *align_addr = (unsigned int *) (address & ADDRESS_ALIGN_MASK);

	if ((bytes % LINE_BYTES) == 0) {
		dump_bytes = bytes;
	} else {
		dump_bytes = (int) (((unsigned int) bytes | LINE_ALIGN_MASK) + 1);
	}

	if (!kbox_kern_addr_valid(address)) {
		return;
	}
	/* feed dog to avoid dog barking during waiting. */
	touch_nmi_watchdog();

	if (address == 0 || reg_address == 0 || bytes <= 0 || reg_name == NULL) {
		return;
	}

	/*
	 * reg_addr: register address; start_addr: print start address;
	 * aligned_start_addr: aligned print start address.
	 */
	kbox_printk("%s\nreg_addr:%#lx start_addr:%#lx aligned_start_addr:%#lx",
			reg_name, (unsigned long *)reg_address, (unsigned long *)address, align_addr);
	while (dump_bytes > 0) {
		if (((dump_bytes % LINE_BYTES) == 0)) {
			kbox_printk("\n%04lx ", (unsigned long) align_addr & 0xffff);
		}
		/*
		 * if align_addr is not a valid kernel address, print "********"
		 * instead of the value of temp_data.
		 */
		if ((get_kernel_nofault(temp_data, align_addr)) ||
			(!kbox_kern_addr_valid((unsigned long) align_addr))) {
			kbox_printk(" ********");
		} else {
			kbox_printk(" %08x", temp_data);
		}

		dump_bytes -= UINT_BYTE_NUM;
		align_addr++;
	}
	kbox_printk("\n");
}

/*
 * check the module parameters are valid or not, called by check_kbox_module_param() and kbox_show_regs().
 * if show_regs_left_offset or show_regs_right_offset is invalid, set the parameters to the default value.
 */
void kbox_show_regs_checkparam(int left_offset, int right_offset)
{
	if ((left_offset < KBOX_SHOW_REGS_MIN_OFFSET) || (right_offset > KBOX_SHOW_REGS_MAX_OFFSET)) {
		kbox_info("show_regs_left_offset or show_regs_right_offset is invalid! the valid value is [%d ~ %d]!",
			KBOX_SHOW_REGS_MIN_OFFSET, KBOX_SHOW_REGS_MAX_OFFSET);
		show_regs_left_offset = SHOW_REGS_DEFAULT_LEFT_OFFSET;
		show_regs_right_offset = SHOW_REGS_DEFAULT_RIGHT_OFFSET;
		kbox_info("your changes with show_regs_left_offset or show_regs_right_offset will not take effect!");
		kbox_info("kbox will use the default value of them!");
	} else if (left_offset >= right_offset) {
		kbox_info("show_regs_left_offset should smaller than show_regs_right_offset!");
		show_regs_left_offset = SHOW_REGS_DEFAULT_LEFT_OFFSET;
		show_regs_right_offset = SHOW_REGS_DEFAULT_RIGHT_OFFSET;
		kbox_info("your changes with show_regs_left_offset or show_regs_right_offset will not take effect!");
		kbox_info("kbox will use the default value of them!");
	}
}

void kbox_show_regs(const struct pt_regs *regs)
{
	kbox_do_show_regs(regs, show_regs_left_offset, show_regs_right_offset);
}

int kallsyms_lookup_name_kprobe_init(void)
{
	struct kprobe kp = {
		.symbol_name = "kallsyms_lookup_name"
	};

	int ret = register_kprobe(&kp);
	if (ret < 0)
		return ret;
	kallsyms_lookup_name_kprobe = (kallsyms_lookup_name_kprobe_t) kp.addr;
	unregister_kprobe(&kp);
	return 0;
}
