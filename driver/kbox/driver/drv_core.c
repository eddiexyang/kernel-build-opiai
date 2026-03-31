/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: kbox drv core init and check
 * Author:
 * Create: 2019-02-14
 */

#include "drv_core.h"

#include <linux/kallsyms.h>
#include <linux/module.h>
#include <linux/securec.h>
#include <linux/kprobes.h>

#include <asm/io.h>

#include "drv_main.h"
#include "drv_print.h"

int g_ioremap_flag;

static kallsyms_lookup_name_kprobe_t kallsyms_lookup_name_kprobe;

int check_mem_param(void)
{
	if (g_kbox_phy_start == 0 || g_kbox_size == 0) {
		drv_err("g_kbox_phy_start param error.");
		return -EINVAL;
	}
	if (g_kbox_size < MIN_KBOX_SIZE || g_kbox_size > MAX_KBOX_SIZE) {
		drv_err("g_kbox_size error, size should be 0x%x<g_kbox_size<0x%x",
			MIN_KBOX_SIZE, MAX_KBOX_SIZE);
		return -EINVAL;
	}
	return 0;
}

int check_sym_param(void)
{
	if (g_kbox_mem_start_sym == NULL || g_kbox_mem_size_sym == NULL) {
		drv_err("g_kbox_mem_start_sym and g_kbox_mem_size_sym param error.");
		return -EINVAL;
	}
	return 0;
}

static int mem_type_init(unsigned long phy_addr, unsigned long size)
{
#ifdef CONFIG_ARM64
	g_kbox_va_start = (unsigned long)ioremap_wc(phy_addr, size);
#else
	g_kbox_va_start = (unsigned long)ioremap(phy_addr, size);
#endif
	if (g_kbox_va_start != 0) {
		g_ioremap_flag = 1;
		drv_debug("ioremap success.");
		return 0;
	}
	g_ioremap_flag = 0;
	drv_err("call ioremap phy addr failed, phy=0x%pK size=0x%lx, try __va",
		(char *)phy_addr, size);
	g_kbox_va_start = (unsigned long)phys_to_virt(phy_addr);
	return 0;
}

static int sym_type_init(void)
{
	unsigned long tmp_ptr;

	tmp_ptr = kallsyms_lookup_name_kprobe(g_kbox_mem_start_sym);
	if (tmp_ptr == 0) {
		drv_err("can not find symbol %s.", g_kbox_mem_start_sym);
		return -EINVAL;
	}
	g_kbox_phy_start = (*(unsigned long *)tmp_ptr);
	if (g_kbox_phy_start == 0) {
		drv_err("symbol %s value error.", g_kbox_mem_start_sym);
		return -EINVAL;
	}

	tmp_ptr = kallsyms_lookup_name_kprobe(g_kbox_mem_size_sym);
	if (tmp_ptr == 0) {
		drv_err("can not find symbol %s.", g_kbox_mem_start_sym);
		return -EINVAL;
	}
	g_kbox_size = (*(unsigned long *)tmp_ptr);
	if (g_kbox_size < MIN_KBOX_SIZE || g_kbox_size > MAX_KBOX_SIZE) {
		drv_err("symbol %s value error.", g_kbox_mem_start_sym);
		return -EINVAL;
	}
	g_kbox_va_start = (unsigned long)phys_to_virt(g_kbox_phy_start);
	return 0;
}

static int storage_type_init(void)
{
	OS_GET_BIOS_NVRAM_ADDR os_get_bios_nvram_addr = NULL;

	os_get_bios_nvram_addr =
		(OS_GET_BIOS_NVRAM_ADDR)kallsyms_lookup_name_kprobe(OS_GET_BIOSNVRAM_ADDR_FUNC);
	if (os_get_bios_nvram_addr == 0) {
		drv_err("symbol %s not found.", OS_GET_BIOSNVRAM_ADDR_FUNC);
		return -EINVAL;
	}
	g_kbox_va_start = (unsigned long)os_get_bios_nvram_addr();
	if (g_kbox_va_start == 0) {
		drv_err("get_kbox_reserve_area interface return erorr addr.");
		return -EINVAL;
	}

	g_kbox_size = BIOSNVRAM_MEM_SIZE;
	return 0;
}

static int __kbox_addr_init(void)
{
	if (g_kbox_type == TYPE_PHY_MEM) {
		return mem_type_init(g_kbox_phy_start, g_kbox_size);
	} else if (g_kbox_type == TYPE_PHY_SYM) {
		return sym_type_init();
	} else if (g_kbox_type == TYPE_PHY_SOTRAGE) {
		return storage_type_init();
	} else {
		return -EINVAL;
	}
}

int kbox_addr_init(void)
{
	int ret;
	char *coldboot = NULL;
	char *os_cmdline = NULL;
	unsigned long os_cmdline_ptr = 0;

	ret = __kbox_addr_init();
	if (ret != 0) {
		drv_err("kbox init addr fail.");
		return ret;
	}
	/* init kbox buf after coldboot */
	os_cmdline_ptr = kallsyms_lookup_name_kprobe(OS_CMDLINE);
	if (os_cmdline_ptr == 0) {
		drv_err("can not find symbol %s.", OS_CMDLINE);
		return -EINVAL;
	}
	os_cmdline = (*(char **)(uintptr_t)os_cmdline_ptr);
	if (os_cmdline == NULL) {
		drv_err("symbol %s is null.", OS_CMDLINE);
		return -EINVAL;
	}
	coldboot = (strstr(os_cmdline, DEVICE_COLDBOOT));
	if (coldboot == NULL) {
		return 0;
	}
	drv_info("init kbox buf after coldboot.");
	ret = memset_s((void *)(uintptr_t)g_kbox_va_start,
		       (size_t)g_kbox_size, 0, (size_t)g_kbox_size);
	if (ret != 0) {
		drv_err("memset error, init kbox driver fail.");
		return -EAGAIN;
	}

	return 0;
}

void kbox_addr_fini(void)
{
	if (g_ioremap_flag == 1) {
		iounmap((void *)g_kbox_va_start);
	}
	g_kbox_type = 0;
}

static int reinit_kbox_manage_area(void)
{
	int ret;
	struct kbox_store_manage *kbox_manage = NULL;

	kbox_manage = (struct kbox_store_manage *)g_kbox_va_start;
	ret = memset_s((void *)kbox_manage, KBOX_LOG_OFFSET, 0, KBOX_LOG_OFFSET);
	if (ret != 0) {
		drv_err("memset error, init kbox driver fail.");
		return -EAGAIN;
	}
	/* some item already set to 0, so we do not init here */
	kbox_manage->magic = KBOX_MANAGE_MAGIC;
	kbox_manage->reset_manage.reset_offset = MANAGE_AREA_SIZE;
	kbox_manage->reset_manage.last_status = RESET_NOT_RECORD;
	kbox_manage->reset_manage.current_status = RESET_NOT_RECORD;
	kbox_manage->log_manage.magic = KBOX_MANAGE_MAGIC;
	kbox_manage->log_manage.log_offset = KBOX_LOG_OFFSET;
	kbox_manage->log_manage.log_content[0].remain_size =
		g_kbox_size - KBOX_LOG_OFFSET;
	return 0;
}

static void init_kbox_manage(void)
{
	struct kbox_store_manage *kbox_manage = NULL;
	struct log_reset_manage *reset_manage = NULL;

	kbox_manage = (struct kbox_store_manage *)g_kbox_va_start;
	reset_manage = &(kbox_manage->reset_manage);
	if (reset_manage->current_status == RESET_RECORDED) {
		reset_manage->last_status = reset_manage->current_status;
		reset_manage->current_status = RESET_RECORDED_SAVED;
	}
}

int kbox_manage_area_init(void)
{
	struct kbox_store_manage *kbox_manage = NULL;

	kbox_manage = (struct kbox_store_manage *)g_kbox_va_start;
	if (kbox_manage == NULL) {
		return -EFAULT;
	}
	if (kbox_manage->magic == KBOX_MANAGE_MAGIC) {
		init_kbox_manage();
		return 0;
	}
	if (kbox_manage->magic == KBOX_NOSPACE_MAGIC) {
		drv_err("there is no enough space to storage last exception event completely, "
				"we storage kbox memory image to /var/log/kbox, then reinit it.");
		kbox_export_memory_image();
	}
	drv_info("device need init! magic number=0x%lx, flag=0x%lx",
		kbox_manage->magic, KBOX_MANAGE_MAGIC);
	return reinit_kbox_manage_area();
}

int kallsyms_lookup_name_kprobe_init(void)
{
	struct kprobe kp = {
		.symbol_name = "kallsyms_lookup_name"
	};
	int ret = register_kprobe(&kp);
	if (ret < 0)
		return ret;
	kallsyms_lookup_name_kprobe = (kallsyms_lookup_name_kprobe_t) kp.addr;
	unregister_kprobe(&kp);
	return 0;
}
