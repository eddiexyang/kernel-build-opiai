/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: drv core init and check
 * Author:
 * Create: 2019-02-14
 */

#ifndef _KBOX_DRIVER_CORE_H_
#define _KBOX_DRIVER_CORE_H_

#define OS_CMDLINE "saved_command_line"
#define DEVICE_COLDBOOT "reboot_reason=DEVICE_COLDBOOT"
#define OS_GET_BIOSNVRAM_ADDR_FUNC "get_kbox_reserve_area"
#define BIOSNVRAM_MEM_SIZE 0x800000
typedef char *(*OS_GET_BIOS_NVRAM_ADDR)(void);
typedef unsigned long (*kallsyms_lookup_name_kprobe_t)(const char *name);

extern int check_sym_param(void);
extern int check_mem_param(void);
extern int kbox_addr_init(void);
extern void kbox_addr_fini(void);
extern int kbox_manage_area_init(void);
extern int kallsyms_lookup_name_kprobe_init(void);

#endif
