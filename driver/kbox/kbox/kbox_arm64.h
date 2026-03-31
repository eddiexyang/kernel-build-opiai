/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: kbox arch related code for arm64
 * Author: pengyeqing
 * Create: 2019-10-29
 */

#ifndef _KBOX_ARM64_H_
#define _KBOX_ARM64_H_

#include <asm/stacktrace.h>

void kbox_show_stack_log_lvl(const struct pt_regs *regs);
void kbox_do_show_regs(const struct pt_regs *regs, int left_offset, int right_offset);

#endif

