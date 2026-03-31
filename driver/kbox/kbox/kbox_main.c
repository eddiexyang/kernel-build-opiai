/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: kbox module init
 * Author: wangshouping
 * Create: 2019-02-14
 */
#include "kbox_main.h"

#include <linux/securec.h>
#include <linux/delay.h>

#include "kbox_common.h"
#include "kbox_common_interface.h"
#include "kbox_die.h"
#include "kbox_emerge.h"
#include "kbox_find_sym_addr.h"
#include "kbox_oom.h"
#include "kbox_panic.h"
#include "kbox_print.h"
#include "kbox_printk_buf.h"
#include "kbox_reboot.h"
#include "kbox_rlock.h"

#define KBOX_MODUEL_VERSION "EulerOS-KBOX-6.0"
#define KBOX_MODULE_DESCRIPTION "the crash message collector for EulerOS"
#define KBOX_MODULE_AUTHOR "HUAWEI TECHNOLOGIES CO., LTD."
#define KBOX_MODULE_LICENSE "GPL"

#define KBOX_MODULE_UNKNOWN_CONFIG 0xFFFFFFFF
#define KBOX_MODULE_UNKOWN_PRODUCT "Unknown"

MODULE_AUTHOR(KBOX_MODULE_AUTHOR);
MODULE_DESCRIPTION(KBOX_MODULE_DESCRIPTION);
MODULE_VERSION(KBOX_MODUEL_VERSION);
MODULE_LICENSE(KBOX_MODULE_LICENSE);

char *g_tmp_buf;
/* kbox module parameters */
static char *product = KBOX_MODULE_UNKOWN_PRODUCT;
unsigned int config = KBOX_MODULE_UNKNOWN_CONFIG;
int show_regs_left_offset = SHOW_REGS_DEFAULT_LEFT_OFFSET;
int show_regs_right_offset = SHOW_REGS_DEFAULT_RIGHT_OFFSET;

module_param(product, charp, 0400);
module_param(config, uint, 0400);
module_param(show_regs_left_offset, int, 0400);
module_param(show_regs_right_offset, int, 0400);

atomic_t g_kbox_status = ATOMIC_INIT(KBOX_STAT_INIT);

static void print_product_base_info(void)
{
	kbox_info("Product: %s.", product);
}

static int check_kbox_module_param(void)
{
	if ((!(config & CFG_FLAG_MASK)) || (config & ~CFG_FLAG_MASK)) {
		kbox_info("illegal input mask = 0x%x.", config);
		return -EINVAL;
	} else {
		kbox_show_regs_checkparam(show_regs_left_offset, show_regs_right_offset);
		kbox_info("the config 0x%x is ok.", config);
		return 0;
	}
}

static int kbox_allocate_tmp_buf(void)
{
	int ret;

	g_tmp_buf = vmalloc(PAGE_SIZE);
	if (g_tmp_buf == NULL) {
		return -ENOMEM;
	}
	ret = memset_s(g_tmp_buf, PAGE_SIZE, 0, PAGE_SIZE);
	if (ret != 0) {
		vfree(g_tmp_buf);
		g_tmp_buf = NULL;
		return -EIO;
	}
	ret = kbox_printk_tmp_log_buf_malloc();
	if (ret != 0) {
		kbox_err("kbox_printk_tmp_log_buf_malloc fail, ret = %d.", ret);
		vfree(g_tmp_buf);
		g_tmp_buf = NULL;
		return ret;
	}
	return 0;
}

static void kbox_free_tmp_buf(void)
{
	kbox_printk_tmp_log_buf_free();
	if (g_tmp_buf != NULL) {
		vfree(g_tmp_buf);
		g_tmp_buf = NULL;
	}
}

static void kbox_init_reboot(void)
{
	if (config & CFG_REBOOT_FLAG) {
		kbox_reboot_init();
	}
}

static int kbox_init_panic(void)
{
	int ret = 0;

	if (config & CFG_PANIC_FLAG) {
		ret = kbox_panic_init();
	}
	return ret;
}

static int kbox_init_oom(void)
{
	int ret = 0;

	if (config & CFG_OOM_FLAG) {
		ret = kbox_oom_init();
	}
	return ret;
}

static int kbox_init_rlock(void)
{
	int ret = 0;

	if (config & CFG_RLOCK_FLAG) {
		ret = kbox_rlock_init();
	}
	return ret;
}

static int kbox_init_die(void)
{
	int ret = 0;

	if (config & CFG_DIE_FLAG) {
		ret = kbox_die_init();
	}
	return ret;
}

static int kbox_init_emerge(void)
{
	int ret = 0;

	if (config & CFG_EMERGE_FLAG) {
		ret = kbox_emerge_init();
	}
	return ret;
}

static void kbox_fini_reboot(void)
{
	if (config & CFG_REBOOT_FLAG) {
		kbox_reboot_fini();
	}
}

static void kbox_fini_panic(void)
{
	if (config & CFG_PANIC_FLAG) {
		kbox_panic_fini();
	}
}

static void kbox_fini_oom(void)
{
	if (config & CFG_OOM_FLAG) {
		kbox_oom_fini();
	}
}

static void kbox_fini_rlock(void)
{
	if (config & CFG_RLOCK_FLAG) {
		kbox_rlock_fini();
	}
}

static void kbox_fini_die(void)
{
	if (config & CFG_DIE_FLAG) {
		kbox_die_fini();
	}
}

static void kbox_fini_emerge(void)
{
	if (config & CFG_EMERGE_FLAG) {
		kbox_emerge_fini();
	}
}

static int kbox_init_events(void)
{
	int ret;

	kbox_init_reboot();

	ret = kbox_init_panic();
	if (ret < 0) {
		kbox_err("init panic fail ret = %d.", ret);
		goto fail1;
	}
	ret = kbox_init_oom();
	if (ret < 0) {
		kbox_err("init oom fail ret = %d.", ret);
		goto fail2;
	}
	ret = kbox_init_rlock();
	if (ret < 0) {
		kbox_err("init rlock fail. ret = %d.", ret);
		goto fail3;
	}
	ret = kbox_init_die();
	if (ret < 0) {
		kbox_err("kbox: init die fail ret = %d.", ret);
		goto fail4;
	}
	ret = kbox_init_emerge();
	if (ret < 0) {
		kbox_err("init emerge fail ret = %d.", ret);
		goto fail5;
	}

	return 0;

fail5:
	kbox_fini_die();
fail4:
	kbox_fini_rlock();
fail3:
	kbox_fini_oom();
fail2:
	kbox_fini_panic();
fail1:
	kbox_fini_reboot();

	return ret;
}

static int __init kbox_init(void)
{
	int ret;

	ret = kallsyms_lookup_name_kprobe_init();
	if (ret != 0) {
		return ret;
	}

	ret = check_kbox_module_param();
	if (ret != 0) {
		kbox_err("load failed.");
		return -EINVAL;
	}
	print_product_base_info();
	ret = kbox_sym_init();
	if (ret != 0) {
		kbox_err("init symbol failed!");
		return -ENXIO;
	}

	ret = kbox_allocate_tmp_buf();
	if (ret != 0) {
		kbox_err("kbox_allocate_tmp_buf fail, ret = %d.", ret);
		return ret;
	}

	spin_lock_init(&g_kbox_event_lock);

	ret = kbox_init_events();
	if (ret < 0) {
		kbox_err("init fs exception fail ret = %d.", ret);
		kbox_free_tmp_buf();
		return ret;
	}

	kbox_info("-------- load OK --------");
	return 0;
}

static void __exit kbox_fini(void)
{
	while (atomic_cmpxchg(&g_kbox_status, KBOX_STAT_INIT, KBOX_STAT_EXIT) == KBOX_STAT_WORK) {
		msleep(100);
	}
	kbox_free_tmp_buf();
	kbox_fini_reboot();
	kbox_fini_panic();
	kbox_fini_oom();
	kbox_fini_rlock();
	kbox_fini_die();
	kbox_fini_emerge();
	atomic_set(&g_kbox_status, KBOX_STAT_INIT);
}

module_init(kbox_init);
module_exit(kbox_fini);
