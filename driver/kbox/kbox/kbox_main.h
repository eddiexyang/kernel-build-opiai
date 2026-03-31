/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: kbox module init
 * Author: wangshouping
 * Create: 2019-02-14
 */

#ifndef __KBOX_MAIN_H
#define __KBOX_MAIN_H
extern unsigned int config;
extern char *g_tmp_buf;
extern int show_regs_left_offset;
extern int show_regs_right_offset;
#define SHOW_REGS_DEFAULT_LEFT_OFFSET (-64)
#define SHOW_REGS_DEFAULT_RIGHT_OFFSET 64
#define CFG_PANIC_FLAG 0x01u
#define CFG_REBOOT_FLAG 0x02u
#define CFG_EMERGE_FLAG 0x04u
#define CFG_DIE_FLAG 0x08u
#define CFG_OOM_FLAG 0x10u
#define CFG_RLOCK_FLAG 0x40u /* kernel watchdog */
#define CFG_FS_EXCEPTION_FLAG 0x80u
#define CFG_FLAG_MASK                                                          \
		(CFG_PANIC_FLAG | CFG_REBOOT_FLAG | CFG_EMERGE_FLAG | CFG_DIE_FLAG |         \
		CFG_OOM_FLAG | CFG_RLOCK_FLAG | CFG_FS_EXCEPTION_FLAG)
#endif
