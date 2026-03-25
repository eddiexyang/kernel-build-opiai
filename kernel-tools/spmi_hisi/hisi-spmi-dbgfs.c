/* Copyright (c) Huawei Technologies Co., Ltd. 2018-2021. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef DRV_UT
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/spinlock.h>
#include <linux/debugfs.h>
#include <linux/ctype.h>
#include <linux/securec.h>
#include <linux/hisi-spmi.h>
#include "hisi-spmi-dbgfs.h"

#define COUNT_MAX_SIZE  256
#define CHARS_PER_ITEM   3	/* Char format like 'XX ' */
#define ITEMS_PER_LINE	16	/* sixteen data items each line */
#define ADDR_LEN	 6	/* five bytes address + one space character */
#define MAX_BYTE_PER_TRANSACTION	16
#define MAX_LINE_LENGTH  (ADDR_LEN + (CHARS_PER_ITEM * ITEMS_PER_LINE) + 1)

static const mode_t dfs_mode = S_IRUSR | S_IWUSR;
static const char *dfs_root_name = "spmi";

/* Specific data of SPMI controller */
struct spmi_ctrl_data {
	u32 cnt;
	u32 addr;
	bool atf;
	struct dentry *dir;
	struct list_head node;
	struct spmi_controller *ctrl;
};

/* SPMI Log buffer */
struct spmi_log_buffer {
	size_t rpos;	/* Buffer current 'read' position */
	size_t wpos;	/* Buffer Current 'write' position */
	size_t len;	/* Buffer length */
	char data[0];	/* Log buffer data */
};

struct spmi_dbgfs {
	struct dentry *root;
	struct mutex  lock;
	struct list_head ctrl; /* List of the spmi_ctrl_data nodes */
	struct debugfs_blob_wrapper help_msg;
};

/* Parameters of SPMI transaction */
struct spmi_trans {
	u32 cnt;	/* The number of bytes to read */
	u32 addr;	/* 20-bits address: PID + SID + Register offset */
	u32 offset;	/* Last read data offset */
	bool raw_data;	/* For raw data dump set to true */
	bool atf;
	struct spmi_controller *ctrl;
	struct spmi_log_buffer *log; /* Log buffer data */
};

static struct spmi_dbgfs dbgfs_data = {
	.lock = __MUTEX_INITIALIZER(dbgfs_data.lock), /*lint -e651 -e708 -e570 -e64 -e785*/
	.ctrl = LIST_HEAD_INIT(dbgfs_data.ctrl),
	.help_msg = {
	.data =
"SPMI Debug-FS support\n"
"\n"
"Hierarchy schema:\n"
"/sys/kernel/debug/spmi\n"
"       /help            -- Static help text\n"
"       /spmi-0          -- Directory for the SPMI bus 0\n"
"       /spmi-0/address  -- Starting register address for read or write\n"
"       /spmi-0/atf -- test atf driver or kernel driver\n"
"       /spmi-0/count    -- Number of registers to read (only used for reads)\n"
"       /spmi-0/data     -- Initiates SPMI read (formatted output)\n"
"       /spmi-0/data_raw -- Initiates SPMI raw read or write\n"
"       /spmi-n          -- Directory for the SPMI bus n\n"
"\n"
"To perform the SPMI read or write transactions, you need to first write the\n"
"address of the slave device register to the 'address' file.  For reading\n"
"transactions, the number of bytes to be read needs to be written to the\n"
"'count' file.\n"
"\n"
"The 'address' file specifies the 20-bits address of a slave device register.\n"
"The upper 4 bits 'address[19..16]' specify slave identifier (SID) for\n"
"the slave device.  The lower 16 bits specify slave register address.\n"
"\n"
"Reading from the 'data' file will initiate one SPMI read transaction starting\n"
"from slave register 'address' for 'count' number of bytes.\n"
"\n"
"Writing to the 'data' file will initiate one SPMI write transaction starting\n"
"from slave register 'address'.  The number of registers written to will\n"
"match the number of bytes written to 'data' file.\n"
"\n"
"Example: Read 4 bytes starting at the register address 0x1234 for SID 2\n"
"\n"
"echo 0x21234 > address\n"
"echo 4 > count\n"
"cat data\n"
"\n"
"Example: Write 3 bytes starting at the register address 0x1008 for SID 1\n"
"\n"
"echo 0x11008 > address\n"
"echo 0x01 0x02 0x03 > data\n"
"\n"
"Note that the count file is not used for writing.  Since 3 bytes are\n"
"written to the 'data' file, then 3 bytes will be written across the\n"
"SPMI bus.\n\n",/*lint -e785*/
	},
};
#define HISI_SPMI_FN_MAIN_ID          0xc500eee0u
#define SPMI_READ                     0x00
#define SPMI_WRITE                    0x01

noinline int atfd_spmi_smc(u64 function_id, u64 arg0, u64 arg1, u64 arg2)
{
	register u64 x0 asm("x0") = function_id;
	register u64 x1 asm("x1") = arg0;
	register u64 x2 asm("x2") = arg1;
	register u64 x3 asm("x3") = arg2;

	asm volatile("smc    #0\n"
		: "+r" (x0)
		: "r" (x1), "r" (x2), "r" (x3));

	return (int)x0;/*lint -e715*/
}

static int spmi_dfs_open(struct spmi_ctrl_data *ctrl_data, struct file *file)
{
	size_t logbufsize = SZ_4K;

	struct spmi_log_buffer *log_buf = NULL;
	struct spmi_trans *spmi_tran = NULL;

	if (!ctrl_data) {
		pr_err("No find SPMI controller datas.\n");
		return -EINVAL;
	}

	/* Per file "transaction" data */
	spmi_tran = kzalloc(sizeof(*spmi_tran), GFP_KERNEL);
	if (!spmi_tran) {
		pr_err("Can't allocate memory for transaction data.\n");
		return -ENOMEM;
	}

	/* allocate memory of log_buf */
	log_buf = kzalloc(logbufsize, GFP_KERNEL);
	if (!log_buf) {
		kfree(spmi_tran);
		pr_err("Can't allocate memory for log_buf.\n");
		return -ENOMEM;
	}

	log_buf->rpos = 0;
	log_buf->wpos = 0;
	log_buf->len = logbufsize - sizeof(*log_buf);

	spmi_tran->log = log_buf;
	spmi_tran->cnt = ctrl_data->cnt;
	spmi_tran->addr = ctrl_data->addr;
	spmi_tran->atf = ctrl_data->atf;
	spmi_tran->ctrl = ctrl_data->ctrl;
	spmi_tran->offset = spmi_tran->addr;

	file->private_data = spmi_tran;
	return 0;
}

static int spmi_dfs_raw_data_open(struct inode *inode, struct file *file)
{
	int ret;
	struct spmi_trans *trans_data;
	struct spmi_ctrl_data *ctrl_data = inode->i_private;

	ret = spmi_dfs_open(ctrl_data, file);
	trans_data = file->private_data;
	trans_data->raw_data = true;
	return ret;
}

static int spmi_dfs_close(struct inode *inode, struct file *file)
{
	struct spmi_trans *trans_data = file->private_data;

	if (trans_data && trans_data->log) {
		file->private_data = NULL;
		kfree(trans_data->log);
		kfree(trans_data);
	}
	return 0;/*lint -e715*/
}

static int spmi_dfs_data_open(struct inode *inode, struct file *file)
{
	struct spmi_ctrl_data *ctrl = inode->i_private;
	return spmi_dfs_open(ctrl, file);
}

/**
 * spmi_read_data(): Read data on the SPMI bus.
 * @ctrl: The SPMI controller.
 * @buf: Buffer to save the data read.
 * @offset: SPMI address offset to start reading from.
 * @cnt: Number of read bytes.
 *
 * Returns 0 on success.
 * Otherwise returns error code from SPMI driver.
 */
static int spmi_read_data(struct spmi_controller *ctrl, uint8_t *buf, int offset, int cnt, bool atf)
{
	uint8_t sid;
	uint16_t addr;
	int rc = 0;
	int length;

	while (cnt > 0) {
		length = min(cnt, MAX_BYTE_PER_TRANSACTION);					/*lint !e666 */
		sid = (offset >> 16) & 0xF;/*lint -e702*/
		addr = offset & 0xFFFF;

		if (atf) {
			length = 1;
			rc = atfd_spmi_smc((u64)(HISI_SPMI_FN_MAIN_ID|SPMI_READ), (u64)sid, (u64)addr, (u64)NULL);
			if (rc < 0) {
				pr_err("spmi read failed. (err=%d)\n", rc);
				goto done;
			}
			*buf = (uint8_t)rc;
			rc = 0;
		} else {
			rc = spmi_ext_register_readl(ctrl, sid, addr, buf, length);
			if (rc < 0) {
				pr_err("spmi read failed. (err=%d)\n", rc);
				goto done;
			}
		}

		offset += length;
		cnt -= length;
		buf += length;
	}

done:
	return rc;
}

/**
 * spmi_write_data(): Write data on the SPMI bus.
 * @ctrl: The SPMI controller.
 * @buf: The data to be written.
 * @offset: SPMI address offset to start writing to.
 * @cnt: Number of write bytes.
 *
 * Returns 0 on success.
 * Otherwise returns error code from SPMI driver.
 */
static int spmi_write_data(struct spmi_controller *ctrl, uint8_t *buf, int offset, int cnt, bool atf)
{
	uint8_t sid;
	uint16_t addr;
	int rc = 0;
	int length;

	while (cnt > 0) {
		length = min(cnt, MAX_BYTE_PER_TRANSACTION);				/*lint !e666 */
		sid = (offset >> 16) & 0xF;/*lint -e702*/
		addr = offset & 0xFFFF;

		if (atf) {
			length = 1;
			rc = atfd_spmi_smc((u64)(HISI_SPMI_FN_MAIN_ID|SPMI_WRITE), (u64)sid, (u64)addr, (u64)*buf);
		} else {
			rc = spmi_ext_register_writel(ctrl, sid, addr, buf, length);
		}
		if (rc < 0) {
			pr_err("spmi write failed. (err=%d)\n", rc);
			goto done;
		}

		offset += length;
		cnt -= length;
		buf += length;
	}

done:
	return rc;
}

/**
 * print_to_log(): Format a string and Saved in the log buffer
 * @log: Target log buffer to place the result into.
 * @fmt: The format string to use.
 * @...: The other arguments for the format string.
 *
 * The return value is the number of characters written to @log buffer
 * not including the trailing '\0'.
 */
static unsigned int print_to_log(struct spmi_log_buffer *log, const char *fmt, ...)
{
	unsigned int count;
	va_list args;
	size_t size = log->len - log->wpos;
	char *buf = &log->data[log->wpos];

	va_start(args, fmt);
	count = vscnprintf(buf, size, fmt, args);
	va_end(args);

	log->wpos += (size_t)count;
	return count;
}

/**
 * write_next_line_to_log(): Writes a single line data into the log buffer.
 * @trans: The pointer to SPMI transaction data.
 * @offset: Offset of the SPMI address that starts to be read.
 * @pcnt: Pointer to 'cnt' variable. The number of bytes to read.
 *
 * The 'offset' is a 20-bits SPMI address which includes a 4-bit slave ID (SID),
 * an 8-bit peripheral ID (PID), and an 8-bit peripheral register address.
 *
 * On a successful read, the pcnt is decremented by the number of
 * data bytes read across the SPMI bus.
 * When the count reaches 0, all requested bytes have been read.
 */
static unsigned int write_next_line_to_log(struct spmi_trans *trans, int offset, size_t *pcnt)
{
	struct spmi_log_buffer *log = trans->log;
	size_t i, j;
	u8  data[ITEMS_PER_LINE];
	unsigned int count = 0;

	size_t padding = offset % ITEMS_PER_LINE;
	size_t temp = ARRAY_SIZE(data) - padding;
	size_t items_to_read = min(temp, *pcnt);			/*lint -e846 -e514 -e866 -e30 -e84 -e737  -e1058 -e712 -e666 */
	size_t temp_items_per_line = ITEMS_PER_LINE;
	size_t items_to_log = min(temp_items_per_line, padding + items_to_read);		/*lint !e666 */

	/* Buffer must have enough space for an entire line. */
	if ((log->len - log->wpos) < MAX_LINE_LENGTH)
		goto out;

	/* Read the number of "items". */
	if (spmi_read_data(trans->ctrl, data, (int)trans->addr, items_to_read, trans->atf))
		goto out;

	*pcnt -= items_to_read;/*lint -e737*/

	/* Set each line starts with the aligned offset (20-bit address). */
	count = print_to_log(log, "%5.5X ", offset & 0xffff0);
	if (count == 0)
		goto out;

	/* If the offset is unaligned, must add padding to right justify items. */
	for (i = 0; i < padding; ++i) {
		count = print_to_log(log, "-- ");
		if (count == 0)
			goto out;
	}

	/* Print the data items. */
	for (j = 0; i < items_to_log; ++i, ++j) {
		count = print_to_log(log, "%2.2X ", data[j]);
		if (count == 0)
			goto out;
	}

	/* If the last character was a space, need replace it with a newline. */
	if (log->wpos > 0 && log->data[log->wpos - 1] == ' ')
		log->data[log->wpos - 1] = '\n';

out:
	return count;
}

/**
 * write_raw_data_to_log(): Writes a single line data into the log buffer.
 * @trans: The pointer to SPMI transaction data.
 * @offset: Offset of the SPMI address that starts to be read.
 * @pcnt: Pointer to 'cnt' variable.  The number of bytes to read.
 *
 * The 'offset' is a 20-bits SPMI address which includes a 4-bit slave ID (SID),
 * an 8-bit peripheral ID (PID), and an 8-bit peripheral register address.
 *
 * On a successful read, the pcnt is decremented by the number of
 * data bytes read across the SPMI bus.
 * When the count reaches 0, all requested bytes have been read.
 */
static unsigned int write_raw_data_to_log(struct spmi_trans *trans, int offset, size_t *pcnt)
{
	struct spmi_log_buffer *log = trans->log;
	u8  data[16];

	int items_to_read = min(ARRAY_SIZE(data), *pcnt);/*lint -e846 -e514 -e866 -e30 -e84 -e737  -e1058 -e712 -e666 */
	int i;
	unsigned int count = 0;

	/* Buffer must have enough space for an entire line. */
	if ((log->len - log->wpos) < 80)
		goto out;

	/* Read the number of "items". */
	if (spmi_read_data(trans->ctrl, data, offset, items_to_read, trans->atf))
		goto out;

	*pcnt -= items_to_read;/*lint -e737*/

	/* Print the data items. */
	for (i = 0; i < items_to_read; ++i) {
		count = print_to_log(log, "0x%2.2X ", data[i]);
		if (count == 0)
			goto out;
	}

	/* If the last character was a space, need replace it with a newline. */
	if (log->wpos > 0 && log->data[log->wpos - 1] == ' ')
		log->data[log->wpos - 1] = '\n';

out:
	return count;
}

/**
 * get_log_data() : Reads data on the SPMI bus, and saves to the log buffer.
 * @trans: The pointer to SPMI transaction data.
 *
 * Returns SPMI error code for read failures or the number of "items" read.
 */
static int get_log_data(struct spmi_trans *trans)
{
	size_t item_count = trans->cnt;
	struct spmi_log_buffer *log = trans->log;
	unsigned int (*write_to_log)(struct spmi_trans *, int, size_t *);
	unsigned int count;
	int last_count;
	int items_read;
	int total_items_read = 0;
	u32 offset = trans->offset;

	if (item_count == 0)
		return 0;

	if (trans->raw_data)
		write_to_log = write_raw_data_to_log;
	else
		write_to_log = write_next_line_to_log;

	/* Resets the pointer of the log buffer. */
	log->wpos = log->rpos = 0;

	/* Keep reading data until the log is read completely. */
	do {
		last_count = (int)item_count;
		count = write_to_log(trans, 0, &item_count);/*lint -e737 -e712*/
		items_read = last_count - item_count;
		total_items_read += items_read;
		offset += (u32)items_read;
	} while (count && item_count > 0);

	/* Adjust the transaction count and offset. */
	trans->cnt = (u32)item_count;
	trans->offset += (u32)total_items_read;

	return total_items_read;
}/*lint -e550*/

/**
 * spmi_dfs_reg_write(): Write user's byte array (coded as string) over SPMI.
 * @file: The pointer of file.
 * @buf: The user data to be written.
 * @count: The maximum space in @buf.
 * @ppos: Starting position.
 * @return negative error value, or number of user byte written.
 */
static ssize_t spmi_dfs_reg_write(struct file *file, const char __user *buf,
			size_t count, loff_t *ppos)
{
	u8  *values = NULL;
	int ret = 0;
	int bytes_read;
	int data;
	int pos = 0;
	int value_count = 0;

	struct spmi_trans *trans = file->private_data;
	if (count > COUNT_MAX_SIZE) {
		return  -EFAULT;
	}
	/* Make a copy of the user datas. */
	char *k_buffer = kmalloc(count + 1, GFP_KERNEL);
	if (!k_buffer)
		return -ENOMEM;

	ret = copy_from_user(k_buffer, buf, count);/*lint -e838*/
	if (ret == count) {
		pr_err("Failed to copy data from user.\n");
		ret = -EFAULT;/*lint -e570*/
		goto free_buf;
	}

	count -= ret;
	*ppos += count;/*lint -e737 -e713*/
	k_buffer[count] = '\0';

	/* Override the text buffer with the raw data */
	values = k_buffer;/*lint -e64*/

	/* Parse the data in the buffer.  It should be a string of numbers */
	while (sscanf_s(k_buffer + pos, "%i%n", &data, &bytes_read) == 1) {
		values[value_count++] = data & 0xff;
		pos += bytes_read;
	}

	if (!value_count)
		goto free_buf;

	/* Perform the SPMI write function. */
	ret = spmi_write_data(trans->ctrl, values, (int)trans->addr, value_count, trans->atf);

	if (ret) {
		pr_err("SPMI write data failed. (err=%d)\n", ret);
	} else {
		trans->offset += (u32)value_count;
		ret = value_count;
	}

free_buf:
	kfree(k_buffer);
	return (ssize_t)ret;
}

/**
 * spmi_dfs_reg_read(): Reads value(s) over SPMI and fill
 * user's buffer a byte array (coded as string)
 * @file: The pointer of file.
 * @buf: Where to put the result.
 * @count: The maximum space in @buf.
 * @ppos: Starting position.
 * @return negative error value, or number of user bytes read.
 */
static ssize_t spmi_dfs_reg_read(struct file *file, char __user *buf,
	size_t count, loff_t *ppos)
{
	size_t ret_val;
	size_t len;
	struct spmi_trans *trans = file->private_data;
	struct spmi_log_buffer *log_buf = trans->log;

	/* Whether the log buffer is empty */
	if (log_buf->rpos >= log_buf->wpos) {
		if (get_log_data(trans) <= 0)
			return 0;
	}

	len = min(count, log_buf->wpos - log_buf->rpos);

	ret_val = copy_to_user(buf, &log_buf->data[log_buf->rpos], len);
	if (ret_val == len) {
		pr_err("error copy spmi register values to user\n");
		return -EFAULT;
	}

	/* 'ret_val' is the bytes of not copied data */
	len -= ret_val;

	*ppos += len;/*lint -e737 -e713*/
	log_buf->rpos += len;
	return (ssize_t)len;
}

static const struct file_operations spmi_dfs_raw_data_fops = {
	.read		= spmi_dfs_reg_read,
	.write		= spmi_dfs_reg_write,
	.open		= spmi_dfs_raw_data_open,
	.release	= spmi_dfs_close,
};/*lint -e785*/

static const struct file_operations spmi_dfs_reg_fops = {
	.read		= spmi_dfs_reg_read,
	.write		= spmi_dfs_reg_write,
	.open		= spmi_dfs_data_open,
	.release	= spmi_dfs_close,
};/*lint -e785*/

/**
 * spmi_dfs_create_fs(): Create debugfs file system.
 * @return NULL (if failed to create fs) or pointer to root directory.
 */
static struct dentry *spmi_dfs_create_fs(void)
{
	struct dentry *root = NULL, *file = NULL;

	pr_debug("Create debugfs file-system.\n");
	root = debugfs_create_dir(dfs_root_name, NULL);
	if (IS_ERR_OR_NULL(root)) {
		pr_err("Failed to create top level directory. err=%ld", (long)(uintptr_t)root);
		if (PTR_ERR(root) == -ENODEV)
			pr_err("The debugfs is not enabled in the kernel.\n");
		return NULL;
	}

	dbgfs_data.help_msg.size = strlen(dbgfs_data.help_msg.data);

	file = debugfs_create_blob("help", S_IRUGO, root, &dbgfs_data.help_msg);
	if (!file) {
		pr_err("Failed to create help entry.\n");
		goto err_out_remove_fs;
	}
	return root;

err_out_remove_fs:
	debugfs_remove_recursive(root);
	return NULL;
}

/**
 * spmi_dfs_get_root(): Return the pointer to SPMI debugfs root directory.
 * @brief return a pointer to the existing directory, or if no root directory
 * exists then create one. Directory is created with file that configures
 * SPMI transaction, namely: sid, address, and count.
 * @returns NULL or valid pointer on success.
 */
struct dentry *spmi_dfs_get_root(void)
{
	struct dentry *data_root = dbgfs_data.root;
	if (data_root)
		return data_root;

	if (mutex_lock_interruptible(&dbgfs_data.lock) < 0)
		return NULL;
	/* The critical section. */
	data_root = dbgfs_data.root;
	if (!data_root) { /* Double checking idiom. */
		dbgfs_data.root = spmi_dfs_create_fs();
		data_root = dbgfs_data.root;
	}
	mutex_unlock(&dbgfs_data.lock);
	return data_root;
}

/*
 * spmi_dfs_add_controller(): Adds a spmi controller entry.
 * @return zero if add controller success.
 */
int spmi_dfs_add_controller(struct spmi_controller *ctrl)
{
	struct dentry *dir = NULL;
	struct dentry *root = NULL;
	struct dentry *file = NULL;
	struct spmi_ctrl_data *spmictrl_data = NULL;

	pr_debug("Start to adding controller %s.\n", ctrl->dev.kobj.name);
	root = spmi_dfs_get_root();
	if (!root)
		return -ENOENT;

	/* Alloc data space for the controller. */
	spmictrl_data = kzalloc(sizeof(*spmictrl_data), GFP_KERNEL);
	if (!spmictrl_data)
		return -ENOMEM;

	dir = debugfs_create_dir(ctrl->dev.kobj.name, root);
	if (!dir) {
		pr_err("Failed to create entry for spmi controller %s.\n",
						ctrl->dev.kobj.name);
		goto err_create_dir_failed;
	}

	spmictrl_data->cnt  = 1;
	spmictrl_data->dir  = dir;
	spmictrl_data->ctrl = ctrl;

	(void)debugfs_create_u32("count", dfs_mode, dir, &spmictrl_data->cnt);

	(void)debugfs_create_x32("address", dfs_mode, dir, &spmictrl_data->addr);

	(void)debugfs_create_bool("atf", dfs_mode, dir, &spmictrl_data->atf);

	file = debugfs_create_file("data", dfs_mode, dir, spmictrl_data,
					&spmi_dfs_reg_fops);
	if (!file) {
		pr_err("Failed to create 'data' entry.\n");
		goto err_out_remove_fs;
	}

	file = debugfs_create_file("data_raw", dfs_mode, dir, spmictrl_data,
					&spmi_dfs_raw_data_fops);
	if (!file) {
		pr_err("Failed to create 'data' entry.\n");
		goto err_out_remove_fs;
	}

	list_add(&spmictrl_data->node, &dbgfs_data.ctrl);
	return 0;

err_out_remove_fs:
	debugfs_remove_recursive(dir);
err_create_dir_failed:
	kfree(spmictrl_data);
	return -ENOMEM;
}
EXPORT_SYMBOL_GPL(spmi_dfs_add_controller);

/*
 * spmi_dfs_del_controller(): Deletes SPMI controller entry.
 * @return zero if deletes controller success.
 */
int spmi_dfs_del_controller(struct spmi_controller *ctrl)
{
	struct spmi_ctrl_data *ctrl_data = NULL;
	struct list_head *pos = NULL, *tmp = NULL;
	int ret;

	pr_debug("Delete controller %s.\n", ctrl->dev.kobj.name);

	ret = mutex_lock_interruptible(&dbgfs_data.lock);
	if (ret)
		return ret;

	list_for_each_safe(pos, tmp, &dbgfs_data.ctrl) {
		ctrl_data = list_entry(pos, struct spmi_ctrl_data, node);/*lint -e826*/

		if (ctrl_data->ctrl == ctrl) {
			debugfs_remove_recursive(ctrl_data->dir);
			list_del(pos);
			kfree(ctrl_data);
			ret = 0;
			goto done;
		}
	}
	ret = -EINVAL;
	pr_debug("Unknown controller %s.\n", ctrl->dev.kobj.name);

done:
	mutex_unlock(&dbgfs_data.lock);
	return ret;
}
EXPORT_SYMBOL_GPL(spmi_dfs_del_controller);

/*
 * spmi_dfs_create_file(): Creates new file in the SPMI debugfs.
 * @returns NULL or valid dentry pointer on success.
 */
struct dentry *spmi_dfs_create_file(struct spmi_controller *ctrl,
				const char *name, void *data,
				const struct file_operations *fops)
{
	struct spmi_ctrl_data *spmictrl_data = NULL;

	list_for_each_entry(spmictrl_data, &dbgfs_data.ctrl, node) {/*lint -e826 -e64*/
		if (spmictrl_data->ctrl == ctrl)
			return debugfs_create_file(name, dfs_mode,
							spmictrl_data->dir, data, fops);
	}

	return NULL;
}

static void __exit spmi_dfs_delete_all_ctrl(struct list_head *head)
{
	struct list_head *tmp = NULL;
	struct list_head *pos = NULL;

	list_for_each_safe(pos, tmp, head) {
		struct spmi_ctrl_data *spmictrl_data;

		spmictrl_data = list_entry(pos, struct spmi_ctrl_data, node);/*lint -e826*/
		list_del(pos);
		kfree(spmictrl_data);
	}
}

static void __exit spmi_dfs_destroy(void)
{
	struct dentry *data_root = NULL;

	pr_debug("spmi destroy, de-initializing spmi debugfs.\n");
	if (mutex_lock_interruptible(&dbgfs_data.lock) < 0)
		return;
	data_root = dbgfs_data.root;
	if (data_root) {
		/* Delete debug fs data. */
		debugfs_remove_recursive(data_root);
		dbgfs_data.root = NULL;
		spmi_dfs_delete_all_ctrl(&dbgfs_data.ctrl);
	}
	mutex_unlock(&dbgfs_data.lock);
}

module_exit(spmi_dfs_destroy);/*lint -e528*/

MODULE_DESCRIPTION("HISI SPMI debugfs helpers");
MODULE_LICENSE("GPL v2");/*lint -e753*/
MODULE_ALIAS("platform:spmi_debug_fs");/*lint -e753*/
#else
void spmi_dfs_destroy(void)
{
}
#endif
