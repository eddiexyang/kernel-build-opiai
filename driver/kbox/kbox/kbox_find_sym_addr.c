/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: kbox get sym addr
 * Author: wangshouping
 * Create: 2019-02-14
 */
#include "kbox_find_sym_addr.h"

#include "kbox_print.h"
#include "kbox_common_interface.h"

logbuf_lock_t printk_logbuf_lock;
log_buf_len_t g_log_buf_len_local;
struct printk_ringbuffer **kbox_prb;
KBOX_PRB_READ_VALID kbox_prb_read_valid;
KBOX_RPB_FIRST_VALID_SEQ  kbox_prb_first_valid_seq;
KBOX_DUMP_STACK_PRINT_INFO kbox_dump_stack_print_info;
SCHED_SHOW_TASK kbox_sched_show_task;
KBOX_WDOG_PRINT_STAT kbox_wdog_print_stat;
struct list_head *kbox_modules_ptr;
struct seq_operations *kbox_vmalloc_op_ptr;
PUT_MNT_NS_PTR kbox_put_mnt_ns;

#ifdef CONFIG_ARM64
#ifdef CONFIG_VMAP_STACK
unsigned long **kbox_overflow_stack = NULL;
#endif
#endif
unsigned long **kbox_irq_stack_ptr = NULL;
rwlock_t *kbox_tasklist_lock;
console_printk_t kbox_console_printk;
#ifdef CONFIG_ARM64
ON_SDEI_STACK kbox__on_sdei_stack;
#endif
KERN_ADDR_VALID_PTR kbox_kern_addr_valid;
KBOX_GET_STACK_INFO kbox_get_stack_info;

int *kbox_printk_context;
unsigned long *kbox_safe_print_seq;
#ifdef CONFIG_PRINTK_NMI
unsigned long *kbox_nmi_print_seq;
#endif
struct atomic_notifier_head *kbox_watchdog_notifier_list;
struct atomic_notifier_head *kbox_emerge_notifier_list;
KBOX_SET_CPU_AND_PID kbox_set_cpu_and_pid_ptr;
KBOX_CLEAR_CPU_AND_PID kbox_clear_cpu_and_pid_ptr;
KBOX_REGISTER_STORE_INTERFACE kbox_register_store_interface_ptr;
KBOX_UNREGISTER_STORE_INTERFACE kbox_unregister_store_interface_ptr;
KBOX_ARCH_TRIGGER_CPUMASK_BACKTRACE kbox_arch_trigger_cpumask_backtrace;
struct lsm_static_calls_table *kbox_static_calls_table;

#ifdef CONFIG_SECURITY
KBOX_SECURITY_ADD_HOOKS kbox_security_add_hooks;
KBOX_SECURITY_DELETE_HOOKS kbox_security_delete_hooks;
#endif

struct kbox_fun_sym {
	char *name;
	unsigned long *fun;
	bool optional;
};

struct kbox_fun_sym kbox_sym_table[] = {
	{ "logbuf_lock", (unsigned long *) &printk_logbuf_lock, false },
	{ "log_buf_len", (unsigned long *) &g_log_buf_len_local, false },
	{ "prb", (unsigned long *)&kbox_prb, false },
	{ "prb_first_valid_seq", (unsigned long *)&kbox_prb_first_valid_seq, false },
	{ "_prb_read_valid", (unsigned long *)&kbox_prb_read_valid, false },
	{ "dump_stack_print_info", (unsigned long *) &kbox_dump_stack_print_info, false },
	{ "sched_show_task", (unsigned long *) &kbox_sched_show_task, false },
	{ "cpustat_print_cpu", (unsigned long *) &kbox_wdog_print_stat, false },
	{ "modules", (unsigned long *) &kbox_modules_ptr, false },
	{ "vmalloc_op", (unsigned long *) &kbox_vmalloc_op_ptr, false },
	{ "put_mnt_ns", (unsigned long *) &kbox_put_mnt_ns, false },
#ifdef CONFIG_ARM64
#ifdef CONFIG_VMAP_STACK
	{ "overflow_stack", (unsigned long *) &kbox_overflow_stack, false },
#endif
#endif
#ifdef CONFIG_ARM64
	{ "irq_stack_ptr", (unsigned long *) &kbox_irq_stack_ptr, false },
#else
	{ "hardirq_stack_ptr", (unsigned long *) &kbox_irq_stack_ptr, false },
#endif
	{ "tasklist_lock", (unsigned long *) &kbox_tasklist_lock, false },
	{ "console_printk", (unsigned long *) &kbox_console_printk, false },
#ifdef CONFIG_ARM64
#ifdef CONFIG_ARM_SDE_INTERFACE
	{ "_on_sdei_stack", (unsigned long *) &kbox__on_sdei_stack, false },
#endif
#endif
	{ "kern_addr_valid", (unsigned long *) &kbox_kern_addr_valid, false },
	{ "printk_context", (unsigned long *) &kbox_printk_context, true },
	{ "safe_print_seq", (unsigned long *) &kbox_safe_print_seq, true },
#ifdef CONFIG_PRINTK_NMI
	{ "nmi_print_seq", (unsigned long *) &kbox_nmi_print_seq, true },
#endif
	{ "watchdog_notifier_list", (unsigned long *) &kbox_watchdog_notifier_list, true },
	{ "emerge_notifier_list", (unsigned long *) &kbox_emerge_notifier_list, true },
	{ "kbox_set_cpu_and_pid", (unsigned long *) &kbox_set_cpu_and_pid_ptr, true },
	{ "kbox_clear_cpu_and_pid", (unsigned long *) &kbox_clear_cpu_and_pid_ptr, true },
	{ "kbox_register_store_interface", (unsigned long *)&kbox_register_store_interface_ptr, true },
	{ "kbox_unregister_store_interface", (unsigned long *)&kbox_unregister_store_interface_ptr, true },
	{ "arch_trigger_cpumask_backtrace", (unsigned long *)&kbox_arch_trigger_cpumask_backtrace, true },
	{ "static_calls_table", (unsigned long *)&kbox_static_calls_table, true },
#ifdef CONFIG_X86_64
	{ "get_stack_info", (unsigned long*)&kbox_get_stack_info, false },
#endif
#ifdef CONFIG_SECURITY
	{ "security_add_hooks", (unsigned long*) &kbox_security_add_hooks, false },
	{ "security_delete_hooks", (unsigned long*) &kbox_security_delete_hooks, false },
#endif
	{ NULL, 0, false }
};

/*****************************************************************************
Function     : kbox_sym_init
Description  : all kernel symbols used by kbox
******************************************************************************/
int kbox_sym_init(void)
{
	int i = 0;
	unsigned long addr;

	while (kbox_sym_table[i].name) {
		addr = kallsyms_lookup_name_kprobe(kbox_sym_table[i].name);
		if (addr == 0) {
			if (kbox_sym_table[i].optional) {
				*(kbox_sym_table[i].fun) = 0;
				i++;
				continue;
			}
			printk(KERN_ERR "kbox sym init failed. can't find symbol %s\n",
			       kbox_sym_table[i].name);
			return -EFAULT;
		}

		*(kbox_sym_table[i].fun) = addr;
		i++;
	}
	return 0;
}
