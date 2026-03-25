/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright @ Huawei Technologies Co., Ltd. 2020-2020. ALL rights reversed.
 */
#ifndef __LINUX_KBOX__
#define __LINUX_KBOX__

#ifdef CONFIG_EULEROS_KBOX_ADD_PRINTK_NOTIFIER
#include <linux/types.h>
#include <asm/atomic.h>

extern int kern_addr_valid(unsigned long addr);
extern int g_kbox_record_pid;
extern int g_kbox_record_cpu;
extern void kbox_set_cpu_and_pid(void);
extern void kbox_clear_cpu_and_pid(void);

typedef void (*KBOX_STORE_LOG_FUN)(u64 time, const char *text, const size_t text_len);

extern KBOX_STORE_LOG_FUN kbox_store_printk;
extern int kbox_register_store_interface(KBOX_STORE_LOG_FUN kbox_store_log_fn);
extern void kbox_unregister_store_interface(void);
#endif

#endif
