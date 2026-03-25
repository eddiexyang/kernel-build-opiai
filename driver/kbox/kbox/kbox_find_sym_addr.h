/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: kbox get sym addr
 * Author: wangshouping
 * Create: 2019-02-14
 */
#ifndef _KBOX_FIND_SYM_ADDR_H_
#define _KBOX_FIND_SYM_ADDR_H_

#include <linux/fs.h>
#include <linux/kallsyms.h>
#include <linux/kprobes.h>
#include <linux/mnt_namespace.h>
#include <linux/notifier.h>
#include <linux/timekeeper_internal.h>
#include <linux/vmalloc.h>
#include <linux/lsm_hooks.h>

#include <asm/stacktrace.h>

#include "kernel/printk/printk_ringbuffer.h"

int kbox_sym_init(void);

/* define sym : printk logbuf_lock */
typedef raw_spinlock_t *logbuf_lock_t;
extern logbuf_lock_t printk_logbuf_lock;

/* define sym : printk log_buf */
typedef char **log_buf_t;
extern log_buf_t g_log_buf_local;

/* define sym : printk log_buf_len */
typedef u32 *log_buf_len_t;
extern log_buf_len_t g_log_buf_len_local;

extern struct printk_ringbuffer **kbox_prb;

typedef bool (*KBOX_PRB_READ_VALID)(struct printk_ringbuffer*, u64 *, struct printk_record *, unsigned int *);
extern KBOX_PRB_READ_VALID kbox_prb_read_valid;

typedef u64 (*KBOX_RPB_FIRST_VALID_SEQ)(struct printk_ringbuffer*);
extern KBOX_RPB_FIRST_VALID_SEQ  kbox_prb_first_valid_seq;

typedef void (*KBOX_DUMP_STACK_PRINT_INFO)(const char *log_lvl);
extern KBOX_DUMP_STACK_PRINT_INFO kbox_dump_stack_print_info;

/* define sym : sched_show_task */
typedef void (*SCHED_SHOW_TASK)(struct task_struct *p);
extern SCHED_SHOW_TASK kbox_sched_show_task;

typedef void (*KBOX_WDOG_PRINT_STAT)(void);
extern KBOX_WDOG_PRINT_STAT kbox_wdog_print_stat;

extern struct list_head *kbox_modules_ptr;


typedef void (*PUT_MNT_NS_PTR)(struct mnt_namespace *ns);
extern PUT_MNT_NS_PTR kbox_put_mnt_ns;

#ifdef CONFIG_ARM64
#ifdef CONFIG_VMAP_STACK
typedef unsigned long *(*OVERFLOW_STACK);
extern OVERFLOW_STACK kbox_overflow_stack;
#endif
#endif

typedef unsigned long *(*IRQ_STACK_PTR);
extern IRQ_STACK_PTR kbox_irq_stack_ptr;

/* define sym : vmalloc_op */
extern struct seq_operations *kbox_vmalloc_op_ptr;

extern rwlock_t *kbox_tasklist_lock;

/* define sym : print_console */
typedef int *console_printk_t;
extern console_printk_t kbox_console_printk;

#ifdef CONFIG_ARM64
typedef bool (*ON_SDEI_STACK)(unsigned long sp, struct stack_info *info);
extern ON_SDEI_STACK kbox__on_sdei_stack;
#endif

extern int *kbox_printk_context;
extern unsigned long *kbox_safe_print_seq;
#ifdef CONFIG_PRINTK_NMI
extern unsigned long *kbox_nmi_print_seq;
#endif
extern struct atomic_notifier_head *kbox_watchdog_notifier_list;
extern struct atomic_notifier_head *kbox_emerge_notifier_list;

typedef void (*KBOX_SET_CPU_AND_PID)(void);
extern KBOX_SET_CPU_AND_PID kbox_set_cpu_and_pid_ptr;

typedef void (*KBOX_CLEAR_CPU_AND_PID)(void);
extern KBOX_CLEAR_CPU_AND_PID kbox_clear_cpu_and_pid_ptr;

typedef int (*KBOX_REGISTER_STORE_INTERFACE)(void *kbox_store_log_fn);
extern KBOX_REGISTER_STORE_INTERFACE kbox_register_store_interface_ptr;

typedef void (*KBOX_UNREGISTER_STORE_INTERFACE)(void);
extern KBOX_UNREGISTER_STORE_INTERFACE kbox_unregister_store_interface_ptr;

typedef void (*KBOX_ARCH_TRIGGER_CPUMASK_BACKTRACE)(const struct cpumask *mask,
	int exclude_cpu);
extern KBOX_ARCH_TRIGGER_CPUMASK_BACKTRACE kbox_arch_trigger_cpumask_backtrace;

extern struct lsm_static_calls_table *kbox_static_calls_table;

typedef int (*KERN_ADDR_VALID_PTR)(unsigned long addr);
extern KERN_ADDR_VALID_PTR kbox_kern_addr_valid;
typedef int (*KBOX_GET_STACK_INFO)(unsigned long *stack, struct task_struct *task,
	struct stack_info *info, unsigned long *visit_mask);
extern KBOX_GET_STACK_INFO kbox_get_stack_info;

#ifdef CONFIG_SECURITY
typedef void (*KBOX_SECURITY_ADD_HOOKS)(struct security_hook_list *hooks,
	int count, const struct lsm_id *lsmid);
typedef void (*KBOX_SECURITY_DELETE_HOOKS)(struct security_hook_list *hooks,
	int count);
extern KBOX_SECURITY_ADD_HOOKS kbox_security_add_hooks;
extern KBOX_SECURITY_DELETE_HOOKS kbox_security_delete_hooks;
#endif

#endif
