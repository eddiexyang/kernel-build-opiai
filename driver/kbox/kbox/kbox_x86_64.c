/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: kbox arch related code for x86_64
 * Author: pengyeqing
 * Create: 2019-10-29
 */

#include "kbox_x86_64.h"

#include <asm/stacktrace.h>
#include <linux/securec.h>
#include <linux/nmi.h>

#include "kbox_find_sym_addr.h"
#include "kbox_print.h"
#include "kbox_common_interface.h"

#define PRINT_STACK_LINE_MAX 512
#define HEXBASEBUFSIZE (16 + 1)
#define MEM_INFO_LEN 80
#define X86_REG_NUM 20

void kbox_show_stack_x86_64(struct pt_regs *regs)
{
	int kstack_depth_to_print = PRINT_STACK_LINE_MAX * STACKSLOTS_PER_LINE;
	unsigned long *stack = NULL;
	char str_tmp[MEM_INFO_LEN + 1];
	int ret, i;
	struct stack_info info;

	preempt_disable();
	if (regs != NULL) {
		stack = (unsigned long *)kernel_stack_pointer(regs);
	} else {
		stack = (unsigned long *)&stack;
	}

	if (kbox_get_stack_info(stack, current, &info, NULL) != 0) {
		kbox_err("get_stack_info fail, invalid stack");
		return;
	}

	for (i = 0; i < kstack_depth_to_print; i++) {
		if (stack == info.end)
			break;

		if (i && ((i % STACKSLOTS_PER_LINE) == 0)) {
			kbox_info("%s", str_tmp);
			ret = memset_s(str_tmp, sizeof(str_tmp), ' ', sizeof(str_tmp));
			if (ret != 0) {
				kbox_err("memset_s fail, ret=%d", ret);
				goto out;
			}
			str_tmp[sizeof(str_tmp) - 1] = '\0';
		}
		ret = snprintf_s(str_tmp + (i % STACKSLOTS_PER_LINE) * HEXBASEBUFSIZE,
				sizeof(str_tmp) - (i % STACKSLOTS_PER_LINE) * HEXBASEBUFSIZE, HEXBASEBUFSIZE,
				" %016lx", *stack);
		if (ret == -1) {
			kbox_err("snprintf_s fail, ret=%d", ret);
			goto out;
		}

		stack++;
	}

out:
	preempt_enable();
	return;
}

/* for each register，kbox_do_show_regs() calls kbox_show_memblock(). */
void kbox_do_show_regs(const struct pt_regs *regs, int left_offset, int right_offset)
{
	int size = right_offset - left_offset;
	unsigned int i;
	unsigned long x86_regs[X86_REG_NUM];
	const char *x86_regs_name[] = {"CS", "IP", "SS", "SP", "BP", "SI", "DI", "AX",
					"BX", "CX", "DX", "ORIG_AX", "R8", "R9", "R10",
					"R11", "R12", "R13", "R14", "R15"};

	x86_regs[0] = regs->cs;
	x86_regs[1] = regs->ip;
	x86_regs[2] = regs->ss;
	x86_regs[3] = regs->sp;
	x86_regs[4] = regs->bp;
	x86_regs[5] = regs->si;
	x86_regs[6] = regs->di;
	x86_regs[7] = regs->ax;
	x86_regs[8] = regs->bx;
	x86_regs[9] = regs->cx;
	x86_regs[10] = regs->dx;
	x86_regs[11] = regs->orig_ax;
	x86_regs[12] = regs->r8;
	x86_regs[13] = regs->r9;
	x86_regs[14] = regs->r10;
	x86_regs[15] = regs->r11;
	x86_regs[16] = regs->r12;
	x86_regs[17] = regs->r13;
	x86_regs[18] = regs->r14;
	x86_regs[19] = regs->r15;

	kbox_printk("show registers info start:\n");
	for (i = 0; i < X86_REG_NUM; i++) {
		if (kbox_kern_addr_valid(x86_regs[i])) {
			kbox_show_memblock(x86_regs[i] + left_offset, size, x86_regs_name[i], x86_regs[i]);
		}
	}
	kbox_printk("show registers info end.\n");
}
