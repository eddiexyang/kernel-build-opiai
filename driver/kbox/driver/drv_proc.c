/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: kbox proc
 * Author:
 * Create: 2019-02-14
 */
#include "drv_proc.h"

#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include "drv_main.h"
#include "drv_print.h"

static struct proc_dir_entry *g_kbox_proc_root;
struct proc_dir_entry *g_kbox_proc_devs;

static int mem_show(struct seq_file *seq, void *offset)
{
	seq_printf(seq, "mem_phy_start_addr=0x%lx\nmem_phy_size=0x%lx\n",
		g_kbox_phy_start, g_kbox_size);
	return 0;
}

static int mem_open(struct inode *inode, struct file *file)
{
	if (!inode || !file) {
		return -1;
	}
	return single_open(file, mem_show, NULL);
}

static struct proc_ops g_proc_dev_meminfo_ops = {
	.proc_open = mem_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

int kbox_proc_init(void)
{
	int ret = 0;
	struct proc_dir_entry *proc = NULL;

	drv_info("start proc init.");
	/* create /proc/kbox */
	g_kbox_proc_root = proc_mkdir(KBOX_PROC_NAME, NULL);
	if (g_kbox_proc_root == NULL) {
		drv_err("creat %s failed.\n", KBOX_PROC_NAME);
		return -EIO;
	}

	/* create /proc/kbox/devs */
	g_kbox_proc_devs = proc_mkdir(KBOX_PROC_NAME_DEV, g_kbox_proc_root);
	if (g_kbox_proc_devs == NULL) {
		remove_proc_entry(KBOX_PROC_NAME, NULL);
		drv_err("creat %s failed.\n", KBOX_PROC_NAME_DEV);
		return -1;
	}

	/* create /proc/kbox/devs/mem */
	proc = proc_create_data(KBOX_PROC_NAME_MEMINFO, 0400, g_kbox_proc_devs,
		&g_proc_dev_meminfo_ops, NULL);
	if (proc == NULL) {
		remove_proc_entry(KBOX_PROC_NAME_DEV, g_kbox_proc_root);
		remove_proc_entry(KBOX_PROC_NAME, NULL);
		drv_err("creat %s failed.\n", KBOX_PROC_NAME_MEMINFO);
		return -1;
	}

	return ret;
}

void kbox_proc_fini(void)
{
	/* delete /proc/kbox */
	remove_proc_entry(KBOX_PROC_NAME_MEMINFO, g_kbox_proc_devs);
	remove_proc_entry(KBOX_PROC_NAME_DEV, g_kbox_proc_root);
	remove_proc_entry(KBOX_PROC_NAME, NULL);

	drv_info("proc fini OK");
}
