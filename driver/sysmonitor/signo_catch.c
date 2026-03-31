/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2020. All rights reserved.
 * Description: signal catch module
 * Author: tangyizhou
 * Create: 2020-5-20
 */
#include "signo_catch.h"

#include <linux/file.h>
#include <linux/kprobes.h>
#include <linux/proc_fs.h>
#include <trace/events/signal.h>
#include <asm/siginfo.h>
#include <linux/version.h>

#include "sysmonitor_main.h"

#define SIGNAL_COUNT 31

/* Here introduce euler_get_mm_exe_file and euler_get_task_exe_file
 * to solve the build and insmod error.
 */
static struct file *euler_get_mm_exe_file(const struct mm_struct *mm)
{
	struct file *exe_file = NULL;
	struct file __rcu **exe_file_pp = (struct file __rcu **)&mm->exe_file;

	rcu_read_lock();
	exe_file = rcu_dereference(mm->exe_file);
	if (exe_file && !get_file_rcu(exe_file_pp)) {
		exe_file = NULL;
	}
	rcu_read_unlock();
	return exe_file;
}

static struct file *euler_get_task_exe_file(struct task_struct *task)
{
	struct file *exe_file = NULL;
	struct mm_struct *mm = NULL;

	/* in oom_killer_process, task lock will be hold before send signal */
	if (spin_is_locked(&task->alloc_lock)) {
		return NULL;
	}

	task_lock(task);
	mm = task->mm;
	if (mm != NULL) {
		if (!(task->flags & PF_KTHREAD)) {
			exe_file = euler_get_mm_exe_file(mm);
		}
	}
	task_unlock(task);
	return exe_file;
}

static void save_exe_info(char *exe, int exe_size, struct task_struct *task)
{
	struct file *exe_file = NULL;

	exe_file = euler_get_task_exe_file(task);
	if (exe_file != NULL) {
		memcpy(exe, exe_file->f_path.dentry->d_name.name,
		       exe_file->f_path.dentry->d_name.len);
		fput(exe_file);
	}
}

static void save_msg_info(ce_signo_msg *msg, int sig, struct task_struct *p)
{
	int i;
	struct task_struct *ptask = NULL;

	memset(msg, 0, sizeof(ce_signo_msg));

	msg->send_pid = current->pid;
	memcpy(msg->send_comm, current->comm, TASK_COMM_LEN);

	msg->send_parent_pid = current->parent->pid;
	memcpy(msg->send_parent_comm, current->parent->comm, TASK_COMM_LEN);

	rcu_read_lock();
	ptask = rcu_dereference(current->parent);
	for (i = 0; i < CALL_CHAIN_NUM; i++) {
		if ((ptask->pid == 0) || (ptask->pid == 1))
			break;

		ptask = rcu_dereference(ptask->real_parent);
		msg->send_chain_pid[i] = task_pid_nr(ptask);
		memcpy(msg->send_chain_comm[i], ptask->comm, TASK_COMM_LEN);
	}
	rcu_read_unlock();

	msg->recv_pid = p->pid;
	memcpy(msg->recv_comm, p->comm, TASK_COMM_LEN);

	msg->signo = sig;
	save_exe_info(msg->send_exe, NAME_MAX, current);
	save_exe_info(msg->send_parent_exe, NAME_MAX, current->parent);
	save_exe_info(msg->recv_exe, NAME_MAX, p);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
static void do_store_sig_info(void *ignore, int sig, struct kernel_siginfo *info,
			struct task_struct *task, int group, int result)
#else
static void do_store_sig_info(void *ignore, int sig, struct siginfo *info,
			struct task_struct *task, int group, int result)
#endif
{
	ce_signo_msg msg;
	unsigned long sigcatchmask = get_sigcatchmask();

	if ((sig <= SIGNAL_COUNT) && (sigcatchmask & (1ul << (unsigned int)(sig - 1)))) {
		save_msg_info(&msg, sig, task);
		(void)save_msg(SIGNAL, &msg, sizeof(ce_signo_msg));
	}
}

/*
 * __tracepoint_signal_generate is NOT exported for out-of-tree modules
 * on mainline 6.x.  Resolve it at runtime via kprobe lookup.  If the
 * tracepoint symbol is absent, fall back to a no-op.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)

#include <linux/kprobes.h>

static struct tracepoint *tp_signal_generate;

static void match_signal_tracepoint(struct tracepoint *tp, void *priv)
{
	if (!strcmp(tp->name, "signal_generate"))
		*(struct tracepoint **)priv = tp;
}

int signo_catch_init(void)
{
	int ret;

	tp_signal_generate = NULL;
	for_each_kernel_tracepoint(match_signal_tracepoint, &tp_signal_generate);
	if (!tp_signal_generate) {
		printk(KERN_WARNING "signo_catch: signal_generate tracepoint not found\n");
		return 0;
	}
	ret = tracepoint_probe_register(tp_signal_generate,
					(void *)do_store_sig_info, NULL);
	if (ret) {
		printk(KERN_ERR "signo_catch: signal generate probe failed %d\n", ret);
		tp_signal_generate = NULL;
		return -1;
	}
	printk(KERN_INFO "signo_catch: signal generate probe registered\n");
	return 0;
}

void signo_catch_exit(void)
{
	if (tp_signal_generate) {
		tracepoint_probe_unregister(tp_signal_generate,
					    (void *)do_store_sig_info, NULL);
		tracepoint_synchronize_unregister();
	}
	printk(KERN_INFO "signo_catch: signal generate probe unregistered\n");
}

#else /* < 6.0 */

int signo_catch_init(void)
{
	int ret = register_trace_signal_generate(do_store_sig_info, NULL);
	if (ret) {
		printk(KERN_ERR "signo_catch: signal generate probe failed\n");
		return -1;
	}
	printk(KERN_INFO "signo_catch: signal generate probe registered\n");
	return 0;
}

void signo_catch_exit(void)
{
	unregister_trace_signal_generate(do_store_sig_info, NULL);
	tracepoint_synchronize_unregister();
	printk(KERN_INFO "signo_catch: signal generate probe unregistered\n");
}

#endif
