/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2020. All rights reserved.
 * Description: sysmonitor event msg handler, include signal, fd and network
 * Author: tangyizhou
 * Create: 2020-5-20
 */
#include "sysmonitor_main.h"

#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/proc_fs.h>
#include <linux/version.h>
#include <securec.h>

#include "signo_catch.h"

#define SYSMONITOR_MSG_MAX_LEN 1024
struct sysmonitor_msg {
	int type;
	char msg[SYSMONITOR_MSG_MAX_LEN];
};

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("sysmonitor module, monitor for signal, fd and net device");

static ulong sigcatchmask;
module_param(sigcatchmask, ulong, 0600);
MODULE_PARM_DESC(sigcatchmask,
	"mask for signal catch, set corresponding bit to 1 to enable signal catch");

struct proc_dir_entry *g_proc_sysmonitor;
static unsigned long g_msg_log_seq;
static unsigned long g_msg_buf_seq;
#define MSG_BUFSIZE 256
#define MSG_BUFMASK (MSG_BUFSIZE - 1)
static struct sysmonitor_msg g_msg_buf[MSG_BUFSIZE];
DECLARE_WAIT_QUEUE_HEAD(g_msg_wait);
DEFINE_SPINLOCK(g_msg_buf_lock);

ulong get_sigcatchmask(void)
{
	return sigcatchmask;
}

static ssize_t sysmonitor_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	int error, index;

	if (buf == NULL || count < sizeof(struct sysmonitor_msg)) {
		return -EINVAL;
	}

	/* ring buf size is MSG_BUFSIZE, so we can't read more than that */
	if ((g_msg_buf_seq - g_msg_log_seq) >= MSG_BUFSIZE) {
		g_msg_log_seq = g_msg_buf_seq - MSG_BUFSIZE + 1;
	}

	/* it will return immediately if secend arg is not 0 */
	error = wait_event_interruptible(g_msg_wait, g_msg_buf_seq != g_msg_log_seq);
	if (error != 0) {
		return error;
	}

	index = g_msg_log_seq & MSG_BUFMASK;
	g_msg_log_seq++;

	error = copy_to_user(buf, g_msg_buf + index, sizeof(struct sysmonitor_msg));
	if (error != 0) {
		return -EFAULT;
	}
	return sizeof(struct sysmonitor_msg);
}

static unsigned int sysmonitor_poll(struct file *file, poll_table *wait)
{
	poll_wait(file, &g_msg_wait, wait);
	if (g_msg_buf_seq != g_msg_log_seq) {
		return POLLIN | POLLRDNORM;
	}

	return 0;
}

static int sysmonitor_open(struct inode *inode, struct file *file)
{
	if (!try_module_get(THIS_MODULE)) {
		return -ENOENT;
	}

	return 0;
}

static int sysmonitor_release(struct inode *inode, struct file *file)
{
	module_put(THIS_MODULE);
	return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 5, 0)
static const struct file_operations g_proc_sysmonitor_operations = {
    .read = sysmonitor_read,
    .poll = sysmonitor_poll,
    .open = sysmonitor_open,
    .release = sysmonitor_release,
    .llseek = generic_file_llseek,
};
#else
static const struct proc_ops g_proc_sysmonitor_operations = {
    .proc_read = sysmonitor_read,
    .proc_poll = sysmonitor_poll,
    .proc_open = sysmonitor_open,
    .proc_release = sysmonitor_release,
    .proc_lseek = generic_file_llseek,
};
#endif

int save_msg(int type, const void *msg, int msg_size)
{
	struct sysmonitor_msg *tmp_msg = NULL;
	unsigned int index;
	unsigned long flags;

	if (msg_size <= 0) {
		pr_err("[sysmonitor]: save_msg, msg size is illegal\n");
		return -1;
	}

	if (msg_size > SYSMONITOR_MSG_MAX_LEN) {
		pr_err("[sysmonitor]: msg_size[%d] is larger than msg max size[%d]\n",
			msg_size, SYSMONITOR_MSG_MAX_LEN);
		return -1;
	}

	spin_lock_irqsave(&g_msg_buf_lock, flags);
	index = g_msg_buf_seq & MSG_BUFMASK;
	tmp_msg = g_msg_buf + index;
    if (memset_s(tmp_msg, sizeof(struct sysmonitor_msg), 0, sizeof(struct sysmonitor_msg))) {
        spin_unlock_irqrestore(&g_msg_buf_lock, flags);
        pr_err("[sysmonitor]: save_msg, memset_s failed.\n");
        return -1;
    }
	tmp_msg->type = type;
    if (memcpy_s(tmp_msg->msg, SYSMONITOR_MSG_MAX_LEN, msg, msg_size)) {
        spin_unlock_irqrestore(&g_msg_buf_lock, flags);
        pr_err("[sysmonitor]: save_msg, memcpy_s failed.\n");
        return -1;
    }
	g_msg_buf_seq++;

	spin_unlock_irqrestore(&g_msg_buf_lock, flags);

	if (waitqueue_active(&g_msg_wait))
		wake_up_interruptible(&g_msg_wait);

	return 0;
}

static int __init sysmonitor_module_init(void)
{
	g_proc_sysmonitor = proc_create("sysmonitor", 0400, NULL, &g_proc_sysmonitor_operations);
	if (g_proc_sysmonitor == NULL) {
		pr_err("[sysmonitor]: create /proc/sysmonitor failed.\n");
		return -1;
	}
	return signo_catch_init();
}

static void __exit sysmonitor_module_exit(void)
{
	proc_remove(g_proc_sysmonitor);
	signo_catch_exit();
}
module_init(sysmonitor_module_init);
module_exit(sysmonitor_module_exit);
