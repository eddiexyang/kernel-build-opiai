// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) Huawei Technologies Co., Ltd.
 * Description: ascend bootdot
 */

#include <linux/bootdot.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>

struct status_block {
	u32 magic;
	u8 valid;
	u8 block_id;
	u8 arg_1;
	u8 arg_2;
	u32 exception_id;
	u32 expect_status;
	u32 current_status;
	u32 reserve;
};

struct bootdot_device {
	void *mem_base;
	unsigned long mem_size;
	u32 blk_base;
	u32 blk_end;
	u32 blk_size;
	u32 blk_num;
};

static struct bootdot_device g_bootdot;
static DEFINE_SPINLOCK(g_bootdot_lock);
static bool g_bootdot_enable;
static u32 g_blk_id_show;
static struct proc_dir_entry *g_root_bootdot_dir;

static bool bootdot_blkid_is_valid(u32 block_id)
{
	if (block_id < g_bootdot.blk_base || block_id > g_bootdot.blk_end) {
		pr_err("bootdot blk_id[%u] is not in the [%u %u]\n",
		       block_id, g_bootdot.blk_base, g_bootdot.blk_end);
		return false;
	}

	return true;
}

static struct status_block *bootdot_get_blk(u32 block_id)
{
	if (!bootdot_blkid_is_valid(block_id))
		return NULL;

	return (struct status_block *)(uintptr_t)
		((uintptr_t)g_bootdot.mem_base +
		 (uintptr_t)(block_id - g_bootdot.blk_base) * g_bootdot.blk_size);
}

static void bootdot_erase_all_blk_info(void)
{
	if (g_bootdot.mem_base == NULL || g_bootdot.mem_size == 0)
		return;

	memset_io(g_bootdot.mem_base, 0, g_bootdot.mem_size);
}

int bootdot_init_blk(u32 block_id, u32 magic, u32 execption_id, u32 expect_status)
{
	struct status_block *blk;
	unsigned long flags;

	if (!g_bootdot_enable)
		return -ENODEV;

	blk = bootdot_get_blk(block_id);
	if (blk == NULL)
		return -EINVAL;

	spin_lock_irqsave(&g_bootdot_lock, flags);
	if (blk->current_status != blk->expect_status) {
		pr_err("bootdot initblk: blk-[%u] already failed, exception=0x%x cur=0x%x expect=0x%x\n",
		       block_id, blk->exception_id, blk->current_status, blk->expect_status);
		spin_unlock_irqrestore(&g_bootdot_lock, flags);
		return -EINVAL;
	}

	blk->magic = magic;
	blk->valid = BOOTDOT_BLKCK_BOOT_VALID;
	blk->block_id = (u8)block_id;
	blk->arg_1 = 0;
	blk->arg_2 = 0;
	blk->exception_id = execption_id;
	blk->expect_status = expect_status;
	blk->current_status = 0;
	blk->reserve = 0;
	spin_unlock_irqrestore(&g_bootdot_lock, flags);

	return 0;
}
EXPORT_SYMBOL_GPL(bootdot_init_blk);

int bootdot_set_blk(u32 block_id, u32 magic, u32 current_status)
{
	struct status_block *blk;
	unsigned long flags;

	if (!g_bootdot_enable)
		return -ENODEV;

	blk = bootdot_get_blk(block_id);
	if (blk == NULL)
		return -EINVAL;

	spin_lock_irqsave(&g_bootdot_lock, flags);
	if (blk->magic != magic) {
		pr_err("bootdot setblk fail: input magic 0x%x, saved magic 0x%x\n",
		       magic, blk->magic);
		spin_unlock_irqrestore(&g_bootdot_lock, flags);
		return -EINVAL;
	}

	blk->current_status = current_status;
	spin_unlock_irqrestore(&g_bootdot_lock, flags);

	return 0;
}
EXPORT_SYMBOL_GPL(bootdot_set_blk);

static int bootdot_blkget_show(struct seq_file *m, void *v)
{
	struct status_block *blk = bootdot_get_blk(g_blk_id_show);

	if (blk == NULL)
		return -EINVAL;

	seq_printf(m, "blk=%u magic=0x%x exce=0x%x cur=%u expect=%u\n",
		   g_blk_id_show, blk->magic, blk->exception_id,
		   blk->current_status, blk->expect_status);
	return 0;
}

static int bootdot_blkget_open(struct inode *inode, struct file *file)
{
	return single_open(file, bootdot_blkget_show, NULL);
}

static ssize_t bootdot_blkget_write(struct file *file, const char __user *buffer,
				    size_t count, loff_t *ppos)
{
	char raw[4] = { 0 };
	u32 block_id;
	size_t len = count;
	int ret;

	if (len == 0 || len > sizeof(raw) - 1)
		return -EINVAL;

	if (copy_from_user(raw, buffer, len))
		return -EFAULT;

	ret = kstrtouint(raw, 10, &block_id);
	if (ret)
		return ret;

	if (!bootdot_blkid_is_valid(block_id))
		return -EINVAL;

	g_blk_id_show = block_id;
	return count;
}

static const struct proc_ops g_bootdot_getblk_proc_ops = {
	.proc_open = bootdot_blkget_open,
	.proc_read = seq_read,
	.proc_release = single_release,
	.proc_write = bootdot_blkget_write,
};

static int bootdot_show_read(struct seq_file *m, void *v)
{
	u32 i;

	seq_printf(m, "bootdot_show, blk_num=%u\n", g_bootdot.blk_num);
	seq_printf(m, "magic        blockid        exception_id        expect        current\n");

	for (i = 0; i < g_bootdot.blk_num; ++i) {
		struct status_block *blk = (struct status_block *)(uintptr_t)
			((uintptr_t)g_bootdot.mem_base + (uintptr_t)i * g_bootdot.blk_size);

		seq_printf(m, "0x%08x     %07u        0x%08x            %u        %u\n",
			   blk->magic, blk->block_id, blk->exception_id,
			   blk->expect_status, blk->current_status);
	}

	return 0;
}

static int bootdot_init_proc_fs(void)
{
	g_root_bootdot_dir = proc_mkdir("bootdot", NULL);
	if (g_root_bootdot_dir == NULL) {
		pr_err("bootdot fail: create /proc/bootdot failed\n");
		return -EINVAL;
	}

	if (proc_create("getblk", 0644, g_root_bootdot_dir,
			&g_bootdot_getblk_proc_ops) == NULL) {
		remove_proc_entry("bootdot", NULL);
		g_root_bootdot_dir = NULL;
		pr_err("bootdot fail: create /proc/bootdot/getblk failed\n");
		return -EINVAL;
	}

	if (proc_create_single_data("bootdot-show", 0444, NULL,
				    bootdot_show_read, NULL) == NULL) {
		remove_proc_entry("getblk", g_root_bootdot_dir);
		remove_proc_entry("bootdot", NULL);
		g_root_bootdot_dir = NULL;
		pr_err("bootdot fail: create /proc/bootdot-show failed\n");
		return -EINVAL;
	}

	return 0;
}

static void bootdot_uninit_proc_fs(void)
{
	if (g_root_bootdot_dir == NULL)
		return;

	remove_proc_entry("getblk", g_root_bootdot_dir);
	remove_proc_entry("bootdot", NULL);
	remove_proc_entry("bootdot-show", NULL);
	g_root_bootdot_dir = NULL;
}

static int __init ascend_bootdot_init(void)
{
	struct device_node *np;
	struct resource res;
	unsigned long mem_size;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,bootdot");
	if (np == NULL) {
		pr_err("bootdot fail: cannot find hisilicon,bootdot node\n");
		return -ENODEV;
	}

	if (of_address_to_resource(np, 0, &res)) {
		of_node_put(np);
		pr_err("bootdot fail: address_to_resource failed\n");
		return -EINVAL;
	}
	of_node_put(np);

	mem_size = resource_size(&res);
	g_bootdot.mem_base = ioremap_wc(res.start, mem_size);
	if (g_bootdot.mem_base == NULL) {
		pr_err("bootdot fail: ioremap_wc failed\n");
		return -EINVAL;
	}

	g_bootdot.mem_size = mem_size;
	g_bootdot.blk_base = BOOTDOT_DEFAULT;
	g_bootdot.blk_size = sizeof(struct status_block);
	g_bootdot.blk_num = g_bootdot.mem_size / g_bootdot.blk_size;
	if (g_bootdot.blk_num == 0) {
		pr_err("bootdot fail: invalid bootdot layout\n");
		goto err_unmap;
	}

	g_bootdot.blk_end = g_bootdot.blk_base + g_bootdot.blk_num - 1;
	bootdot_erase_all_blk_info();

	g_blk_id_show = BOOTDOT_DEFAULT;
	if (bootdot_init_proc_fs())
		goto err_unmap;

	g_bootdot_enable = true;
	pr_info("bootdot probe succ, blk id:[%u, %u]\n",
		g_bootdot.blk_base, g_bootdot.blk_end);
	return 0;

err_unmap:
	iounmap(g_bootdot.mem_base);
	g_bootdot.mem_base = NULL;
	g_bootdot.mem_size = 0;
	return -EINVAL;
}

static void __exit ascend_bootdot_exit(void)
{
	if (!g_bootdot_enable)
		return;

	g_bootdot_enable = false;
	bootdot_uninit_proc_fs();
	if (g_bootdot.mem_base != NULL) {
		iounmap(g_bootdot.mem_base);
		g_bootdot.mem_base = NULL;
	}
	g_bootdot.mem_size = 0;
}

module_init(ascend_bootdot_init);
module_exit(ascend_bootdot_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Ascend bootdot driver");
MODULE_AUTHOR("Huawei");
