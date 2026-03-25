/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: kbox dev manage
 * Author:
 * Create: 2019-02-14
 */

#include "drv_cdev.h"

#include <linux/cdev.h>
#include <linux/err.h>
#include <linux/export.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/securec.h>
#include <linux/semaphore.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>

#include <kbox/kbox_ioctl.h>

#include "drv_main.h"
#include "drv_oper.h"
#include "drv_print.h"

struct device_to_type {
	char *type_name;
	unsigned int type;
};

struct device_to_type g_device_array[] = {
	{ "hmem", TYPE_PHY_MEM },
	{ "hmem", TYPE_PHY_SYM },
	{ "biosnvram", TYPE_PHY_SOTRAGE },
	{ "pcie", TYPE_PHY_PCIE },
	{ NULL, 0 }
};

static dev_t g_devno;
static struct cdev g_kbox_cdev;
static struct class *g_kbox_class;

static int device_name_to_type(const char * const name)
{
	int i = 0;
	int ret;
	int found = 0;

	while (g_device_array[i].type_name) {
		ret = strncmp(g_device_array[i].type_name, name, strlen(g_device_array[i].type_name));
		if (ret != 0) {
			i++;
			continue;
		}
		if (g_device_array[i].type == g_kbox_type) {
			found = (int)g_device_array[i].type;
			break;
		}
		i++;
	}
	return found;
}

static int kbox_acquire_default_log_path(void)
{
	int ret;
	ret = memcpy_s(kbox_log_path, KBOX_PATH_MAXLEN,
		       DEFAULT_KBOX_LOG, strlen(DEFAULT_KBOX_LOG) + 1);
	if (ret < 0) {
		return ret;
	}
	return 0;
}

static int kbox_acquire_log_path(kbox_export_ioc *export_cb)
{
	int ret;
	int offset;

	offset = strlen(export_cb->kbox_log_path);
	if (offset == 0) {
		ret = kbox_acquire_default_log_path();
		return ret;
	}
	ret = memset_s(kbox_log_path, KBOX_PATH_MAXLEN, 0, KBOX_PATH_MAXLEN);
	if (ret < 0) {
		return ret;
	}
	if (offset >= KBOX_PATH_MAXLEN - sizeof(KBOX_LOG_NAME)) {
		drv_info("kbox log path too long, input %d, max %ld",
			 offset, KBOX_PATH_MAXLEN - sizeof(KBOX_LOG_NAME));
		return -EINVAL;
	}
	ret = memcpy_s(kbox_log_path, KBOX_PATH_MAXLEN, export_cb->kbox_log_path, offset);
	if (ret < 0) {
		return ret;
	}
	if (kbox_log_path[offset - 1] != '/') {
		kbox_log_path[offset] = '/';
		offset++;
	}
	ret = memcpy_s(kbox_log_path + offset, KBOX_PATH_MAXLEN - offset,
		       KBOX_LOG_NAME, sizeof(KBOX_LOG_NAME) - 1);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

static int kbox_do_export_dev(const un_kbox_ioctl_cb *ioctl_cb)
{
	int ret;
	kbox_export_ioc *export_cb = (kbox_export_ioc *)ioctl_cb;

	if (export_cb == NULL) {
		drv_err("ioctl export failed. export_cb is NULL");
		return -EINVAL;
	}

	ret = device_name_to_type(export_cb->dev_name);
	if (ret == 0) {
		drv_err("device %s not found.", export_cb->dev_name);
		return -EINVAL;
	}

	ret = kbox_acquire_log_path(export_cb);
	if (ret < 0) {
		return ret;
	}

	drv_debug("export type=%u index=%u", export_cb->type, export_cb->index);
	ret = kbox_export_dev_info(export_cb->type, export_cb->index);
	if (ret < 0) {
		return ret;
	}
	return 0;
}

static int kbox_do_query(const un_kbox_ioctl_cb *ioctl_cb)
{
	int ret;

	kbox_query_ioc *query_cb = (kbox_query_ioc *)ioctl_cb;

	if (query_cb == NULL) {
		drv_err("ioctl query failed. query_cb is NULL");
		return -EINVAL;
	}
	if (query_cb->query_num == 0 || query_cb->query_num > MAX_RESET) {
		drv_err("ioctl query failed. query reset number should be [1, %d].", MAX_RESET);
		return -EINVAL;
	}

	ret = device_name_to_type(query_cb->dev_name);
	if (ret == 0) {
		drv_err("device %s not found.", query_cb->dev_name);
		return -EINVAL;
	}

	ret = kbox_query_reset(query_cb);
	return ret;
}

static kbox_ioc_tbl g_kbox_ioctl_tbl[] = {
	{ KBOX_IOC_EXPORT_DEV, sizeof(kbox_export_ioc), kbox_do_export_dev },
	{ KBOX_IOC_QUERY, sizeof(kbox_query_ioc), kbox_do_query }
};

static ssize_t kbox_dev_read(struct file *filp, char __user *buf, size_t count,
	loff_t *f_pos)
{
	return 0;
}

static ssize_t kbox_dev_write(struct file *filp, const char __user *buf,
	size_t count, loff_t *f_pos)
{
	return 0;
}

static void kbox_ioctl_strend(un_kbox_ioctl_cb *ioctl_cb, unsigned int cmd)
{
	if (ioctl_cb == NULL) {
		return;
	}
	if (cmd == (unsigned int)KBOX_IOC_EXPORT_DEV) {
		ioctl_cb->export.dev_name[KBOX_DEV_NAME_MAXLEN - 1] = '\0';
	} else if (cmd == (unsigned int)KBOX_IOC_QUERY) {
		ioctl_cb->query.dev_name[KBOX_DEV_NAME_MAXLEN - 1] = '\0';
	}
}
/*
 *  Common ioctl function called during 64bit and 32bit userspace and kernel 64bit
 *  The ioctl() implementation for kbox fs
 */
static long kbox_dev_ioctl_common(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret;
	int i;
	un_kbox_ioctl_cb *ioctl_cb = NULL;
	unsigned long ioc_cb_len;
	const int tbl_size = (sizeof(g_kbox_ioctl_tbl) / sizeof(kbox_ioc_tbl));

	if (_IOC_TYPE(cmd) != KBOX_IOC_MAGIC) {
		drv_err("kbox_dev_ioctl_common: cmd type fail.");
		return -EINVAL;
	}

	ret = down_interruptible(&g_kbox_sem);
	if (ret != 0) {
		return ret;
	}

	ioctl_cb = vmalloc(sizeof(un_kbox_ioctl_cb));
	if (ioctl_cb == NULL) {
		up(&g_kbox_sem);
		return -ENOMEM;
	}
	for (i = 0; i < tbl_size; i++) {
		if (cmd != g_kbox_ioctl_tbl[i].cmd) {
			continue;
		}

		ioc_cb_len = g_kbox_ioctl_tbl[i].cb_size;
		ret = memset_s(ioctl_cb, (size_t)ioc_cb_len, 0, (size_t)ioc_cb_len);
		if (ret != 0) {
			drv_err("kbox_dev_ioctl_common: memset_s fail, ret=%d!", ret);
			goto out;
		}

		if (copy_from_user(ioctl_cb, (void *)arg, ioc_cb_len)) {
			ret = -EFAULT;
			goto out;
		}

		kbox_ioctl_strend(ioctl_cb, cmd);

		ret = g_kbox_ioctl_tbl[i].func(ioctl_cb);
		if (ret != 0) {
			goto out;
		}

		if (copy_to_user((void *)arg, ioctl_cb, ioc_cb_len)) {
			ret = -EFAULT;
			goto out;
		}
		goto out;
	}
	/* if cmd mismatch go through here */
	ret = -EINVAL;
out:
	vfree(ioctl_cb);
	up(&g_kbox_sem);
	return ret;
}

static int kbox_dev_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int kbox_dev_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static struct file_operations g_kbox_dev_fops = {
	.owner = THIS_MODULE,
	.read = kbox_dev_read,
	.write = kbox_dev_write,
	.unlocked_ioctl = kbox_dev_ioctl_common,
	.open = kbox_dev_open,
	.release = kbox_dev_release,
};

int kbox_dev_init(void)
{
	int ret;
	struct device *kbox_device = NULL;

	/* Register your major, and accept a dynamic number. */
	ret = alloc_chrdev_region(&g_devno, 0, 1, KBOX_DEV_NAME);
	if (ret < 0) {
		drv_err("alloc chrdev failed. ret: %d\n", ret);
		return ret;
	}

	/* init kbox dev */
	cdev_init(&g_kbox_cdev, &g_kbox_dev_fops);
	g_kbox_cdev.owner = THIS_MODULE;
	ret = cdev_add(&g_kbox_cdev, g_devno, 1);
	if (ret < 0) {
		drv_err("add kbox dev fail, ret=%d\n", ret);
		unregister_chrdev_region(g_devno, 1);
		return ret;
	}

	g_kbox_class = class_create(THIS_MODULE, KBOX_DEV_NAME);
	if (IS_ERR(g_kbox_class)) {
		drv_err("create class fail!\n");
		cdev_del(&g_kbox_cdev);
		unregister_chrdev_region(g_devno, 1);
		return -ENODEV;
	}
	kbox_device = device_create(g_kbox_class, NULL, g_devno, NULL, KBOX_DEV_NAME);
	if (IS_ERR(kbox_device)) {
		drv_err("create dev kbox file fail\n");
		class_destroy(g_kbox_class);
		cdev_del(&g_kbox_cdev);
		unregister_chrdev_region(g_devno, 1);
		return PTR_ERR(kbox_device);
	}

	return 0;
}

void kbox_dev_fini(void)
{
	device_destroy(g_kbox_class, g_devno);
	class_destroy(g_kbox_class);

	/* remove kbox cdev */
	cdev_del(&g_kbox_cdev);
	unregister_chrdev_region(g_devno, 1);
}
