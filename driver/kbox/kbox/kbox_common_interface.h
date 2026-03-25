/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: kbox common interface
 * Author: wangshouping
 * Create: 2019-02-14
 */
#ifndef _KBOX_COMMON_INTERFACE_H_
#define _KBOX_COMMON_INTERFACE_H_
#include <kbox/kbox_event.h>
#define EVENT_SUBSEQUENT_FAIL (-1)
#define EVENT_SUBSEQUENT_OK 0
#define TRUE 1

#define MAX_PRINT_MODULE 1024
#define KBOX_CONSOLE_VERBORS_LEVEL 15 /* the lowest log level */
#define TIME_USEC 1000
#define TIME_USEC_TO_SEC 1000000
#define TIME_BUF_LEN 64
#define KBOX_SHOW_REGS_MIN_OFFSET (-512)
#define KBOX_SHOW_REGS_MAX_OFFSET 1024

int kbox_event_pre_process(enum log_reset_type event);
void kbox_notify_die(enum kbox_die_reason reason);
int kbox_regsiter_store_log(void);
void kbox_unregister_store_log(void);
void kbox_printk(const char *fmt, ...);
void kbox_show_module_info(void);
char *kbox_event_to_string(unsigned int event);
void kbox_show_stack_info(struct pt_regs *regs);
void kbox_notify_smp_cpus_backtrace(char *msg);
void kbox_show_regs(const struct pt_regs *regs);
void kbox_show_regs_checkparam(int left_offset, int right_offset);
void kbox_show_memblock(unsigned long address, int bytes, const char *reg_name, unsigned long reg_address);

extern int g_kbox_log_fd;
extern struct module *get_kernel_modules(void);
extern void kbox_show_seq_info(int type);
extern void kbox_set_console_level(int new_log_level);
extern void kbox_restore_console_level(void);
extern int kallsyms_lookup_name_kprobe_init(void);

typedef void (*smp_nmi_call_func_t)(unsigned long info);

typedef unsigned long (*kallsyms_lookup_name_kprobe_t)(const char *name);
extern kallsyms_lookup_name_kprobe_t kallsyms_lookup_name_kprobe;

#endif
