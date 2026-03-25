/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: kbox arch related code for arm64
 * Author: pengyeqing
 * Create: 2019-10-29
 */

#include "kbox_arm64.h"

#include <linux/securec.h>
#include <asm/stacktrace.h>
#include <linux/nmi.h>

#include "kbox_find_sym_addr.h"
#include "kbox_print.h"
#include "kbox_common_interface.h"

#define PRINT_STACK_LINE_MAX 512
#define HEXBASEBUFSIZE (16 + 1)
#define MEM_INFO_LEN 80
#define ROW_NUM 4
#define DATA_LEN 8
#define MEM_DATA_SIZE 32
#define ARM_EXTRA_REG_NUM 3
#define REG_NAME_LEN 8

static bool kbox_on_irq_stack(unsigned long sp, unsigned long *top)
{
	unsigned long low = (unsigned long)raw_cpu_read(*kbox_irq_stack_ptr);
	unsigned long high = low + IRQ_STACK_SIZE;

	if (!low) {
		return false;
	}

	if (sp < low || sp >= high) {
		return false;
	}

	if (top) {
		*top = high;
	}
	return true;
}

#ifdef CONFIG_VMAP_STACK
static bool kbox_on_overflow_stack(unsigned long sp, unsigned long *top)
{
	unsigned long low = (unsigned long)raw_cpu_ptr(kbox_overflow_stack);
	unsigned long high = low + OVERFLOW_STACK_SIZE;

	if (sp < low || sp >= high) {
		return false;
	}

	if (top) {
		*top = high;
	}

	return true;
}
#else
static bool kbox_on_overflow_stack(unsigned long sp, unsigned long *top)
{
	return false;
}
#endif

static bool kbox_on_sdei_stack(unsigned long sp, struct stack_info *info)
{
	if (!IS_ENABLED(CONFIG_VMAP_STACK)) {
		return false;
	}
	if (!IS_ENABLED(CONFIG_ARM_SDE_INTERFACE)) {
		return false;
	}
#if defined(CONFIG_VMAP_STACK) && defined(CONFIG_ARM_SDE_INTERFACE)
	if (in_nmi()) {
		return kbox__on_sdei_stack(sp, info);
	}
#endif

	return false;
}

static bool kbox_on_accessible_stack(unsigned long sp, unsigned long *top)
{
	bool ret = false;
	struct stack_info info = stackinfo_get_task(current);

	if (stackinfo_on_stack(&info, sp, 1)) {
		*top = info.high;
		ret = true;
		goto out;
	}

	if (preemptible()) {
		ret = false;
		goto out;
	}

	if (kbox_on_irq_stack(sp, top)) {
		ret = true;
		goto out;
	}

	if (kbox_on_overflow_stack(sp, top)) {
		ret = true;
		goto out;
	}

	if (kbox_on_sdei_stack(sp, &info)) {
		*top = info.high;
		ret = true;
		goto out;
	}

out:
	return ret;
}

static int kbox_show_dump_mem_info(const char *lvl, unsigned long first, unsigned long bottom, unsigned long top)
{
	int i;
	int ret;
	unsigned long p;
	char str_tmp[MEM_INFO_LEN + 1];

	ret = memset_s(str_tmp, sizeof(str_tmp), ' ', sizeof(str_tmp));
	if (ret != 0) {
		kbox_err("memset_s fail, ret=%d", ret);
		return -1;
	}
	str_tmp[sizeof(str_tmp) - 1] = '\0';

	for (p = first, i = 0; i < ROW_NUM && p < top; i++, p += DATA_LEN) {
		if (p >= bottom && p < top) {
			ret = snprintf_s(str_tmp + i * HEXBASEBUFSIZE,
				sizeof(str_tmp) - i * HEXBASEBUFSIZE, HEXBASEBUFSIZE,
				" %016lx", *(unsigned long *)p);
			if (ret == -1) {
				kbox_err("snprintf_s fail");
				return -1;
			}
		}
	}

	kbox_info("%s%04lx:%s", lvl, first & 0xffff, str_tmp);
	return 0;
}

static void kbox_show_dump_mem(const char *lvl, unsigned long bottom,
	unsigned long top)
{
	int line = 0;
	int ret;
	unsigned long first;

	for (first = bottom & ~(MEM_DATA_SIZE - 1); first < top; first += MEM_DATA_SIZE) {
		if (line >= PRINT_STACK_LINE_MAX) {
			kbox_info("show dump mem line has been max :%d", line);
			return;
		}
		ret = kbox_show_dump_mem_info(lvl, first, bottom, top);
		if (ret == -1) {
			kbox_err("show dump mem info fail");
			return;
		}
		line++;
	}
}

static void kbox_dump_mem(const char *lvl, const char *str,
	unsigned long bottom, unsigned long top)
{
	kbox_info("%s%s 0x%016lx to 0x%016lx.", lvl, str, bottom, top);
	kbox_show_dump_mem(lvl, bottom, top);
}

void kbox_show_stack_log_lvl(const struct pt_regs *regs)
{
	unsigned long top = 0;
	unsigned long bottom;
	bool ret = false;

	if (regs != NULL) {
		bottom = regs->sp;
	} else {
		bottom = (unsigned long)&bottom;
	}

	ret = kbox_on_accessible_stack(bottom, &top);
	if (ret == false) {
		kbox_err("no stack found.");
		return;
	}

	kbox_dump_mem("", "stack grow form:", bottom, top);
}

/* for each register，kbox_do_show_regs() calls kbox_show_memblock(). */
void kbox_do_show_regs(const struct pt_regs *regs, int left_offset, int right_offset)
{
	char name[REG_NAME_LEN];
	const char *arm_regs_name[] = {"PC", "LR", "SP"};
	int size = right_offset - left_offset;
	unsigned int i;
	u64 arm_regs[ARM_EXTRA_REG_NUM];

	arm_regs[0] = regs->pc;
	arm_regs[1] = regs->regs[30];
	arm_regs[2] = regs->sp;

	kbox_printk("show registers info start:\n");
	/* show PC,LR and SP. */
	for (i = 0; i < ARM_EXTRA_REG_NUM; i++) {
		if (kbox_kern_addr_valid(arm_regs[i])) {
			kbox_show_memblock(arm_regs[i] + left_offset, size, arm_regs_name[i], arm_regs[i]);
		}
	}
	/* show X0-X29. */
	for (i = 0; i < (ARRAY_SIZE(regs->regs) - 1); i++) {
		if (kbox_kern_addr_valid((regs->regs[i]))) {
			snprintf_s(name, sizeof(name), sizeof(name) - 1, "X%u", i);
			kbox_show_memblock(regs->regs[i] + left_offset, size, name, regs->regs[i]);
		}
	}
	kbox_printk("show registers info end.\n");
}
