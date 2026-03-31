/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: kbox record oom scene log
 * Author: wangshouping
 * Create: 2019-02-14
 */
#include "kbox_oom.h"

#include "fs/mount.h"
#include <linux/sched/clock.h>

#include <linux/binfmts.h>
#include <linux/fs_struct.h>
#include <linux/mount.h>
#include <linux/notifier.h>
#include <linux/nsproxy.h>
#include <linux/oom.h>
#include <linux/rbtree.h>
#include <linux/securec.h>
#include <linux/statfs.h>
#include <linux/vmalloc.h>

#include "../driver/drv_oper.h"
#include "kbox_common.h"
#include "kbox_common_interface.h"
#include "kbox_find_sym_addr.h"
#include "kbox_print.h"
#include "kbox_printk_buf.h"

#define BIT_SIZE 10
#define PST_TOTAL_LEN 100
static char g_oom_path_buff[PATH_MAX];

static char *get_path(const struct path *local_path)
{
	char *p = d_path(local_path, g_oom_path_buff, PATH_MAX);

	if (!IS_ERR(p)) {
		return p;
	}

	return NULL;
}

static int get_fs_path(struct task_struct *task, struct path *local_path, bool root)
{
	struct fs_struct *fs = NULL;
	int result = -ENOENT;

	if (!task || !local_path) {
		return -1;
	}
	task_lock(task);
	fs = task->fs;
	if (fs) {
		if (root) {
			get_fs_root(fs, local_path);
		} else {
			get_fs_pwd(fs, local_path);
		}
		result = 0;
	}
	task_unlock(task);
	return result;
}

static int can_statfs_in_oom(const char *fs_type)
{
	if (!fs_type) {
		return 0;
	}

	if (strncmp(fs_type, "tmpfs", sizeof("tmpfs")) == 0) {
		return 1;
	} else if (strncmp(fs_type, "squashfs", sizeof("squashfs")) == 0) {
		return 1;
	} else if (strncmp(fs_type, "hugetlbfs", sizeof("hugetlbfs")) == 0) {
		return 1;
	} else if (strncmp(fs_type, "rootfs", sizeof("rootfs")) == 0) {
		return 1;
	} else if (strncmp(fs_type, "ramfs", sizeof("ramfs")) == 0) {
		return 1;
	} else if (strncmp(fs_type, "devtmpfs", sizeof("devtmpfs")) == 0) {
		return 1;
	}

	return 0;
}
static void show_single_vfsmount(struct mount *mnt)
{
	struct kstatfs st;
	int err, pst;
	char *tmp_path = NULL;
	const char *fs_type = NULL;
	struct path mnt_path;
	struct vfsmount *vfsmnt = NULL;
	unsigned long total, used;

	if (mnt == NULL) {
		return;
	}
	vfsmnt = &mnt->mnt;
	mnt_path.dentry = vfsmnt->mnt_root;
	mnt_path.mnt = vfsmnt;
	fs_type = vfsmnt->mnt_sb->s_type->name;
	if (can_statfs_in_oom(fs_type) == 0) {
		return;
	}
	if (mnt_path.dentry == NULL || mnt_path.mnt == NULL) {
		return;
	}
	tmp_path = get_path(&mnt_path);
	if (tmp_path == NULL) {
		return;
	}
	err = vfs_statfs(&mnt_path, &st);
	if (err) {
		return;
	}
	if (st.f_blocks == 0) {
		return;
	}
	pst = 0;
	if (st.f_blocks - st.f_bfree > 0) {
		used = (unsigned long)((st.f_blocks - st.f_bfree) * st.f_bsize) >> BIT_SIZE;
		total = (unsigned long)(st.f_blocks * st.f_bsize) >> BIT_SIZE;
		if (total != 0) {
			if ((used >= ULONG_MAX / PST_TOTAL_LEN)) {
				pst = (int)(used / (total / PST_TOTAL_LEN));
			} else {
				pst = (int)(used * PST_TOTAL_LEN / total);
			}
		}
	}
	kbox_info("%-20s  %8lu %8lu %8lu %8d%%       %-10s", mnt->mnt_devname ? mnt->mnt_devname : "none",
		(unsigned long)(st.f_blocks * st.f_bsize) >> BIT_SIZE,
		(unsigned long)((st.f_blocks - st.f_bfree) * st.f_bsize) >> BIT_SIZE,
		(unsigned long)(st.f_bfree * st.f_bsize) >> BIT_SIZE, pst, tmp_path);
}

static int show_mount_memfs(const struct mnt_namespace *ns)
{
	struct rw_semaphore *namespace_sem = NULL;
	struct rb_node *node = NULL;

	if (ns == NULL) {
		kbox_err("ns is NULL.");
		return -1;
	}
	namespace_sem = (struct rw_semaphore *)kallsyms_lookup_name_kprobe("namespace_sem");
	if (namespace_sem == NULL) {
		kbox_err("sym namespace_sem is not found.");
		return -1;
	}

	kbox_info("memory file system info start:");
	kbox_info("Filesystem             1K-blocks    Used   Available Use(%%)   Mounted on");
	if (down_read_trylock(namespace_sem)) {
		for (node = rb_first(&ns->mounts); node != NULL; node = rb_next(node)) {
			struct mount *mnt = rb_entry(node, struct mount, mnt_node);

			show_single_vfsmount(mnt);
		}
		up_read(namespace_sem);
	} else {
		 kbox_err("down_read_trylock(namespace_sem) failed. ignore it.");
	}
	return 0;
}

static int show_mount_info(void)
{
	struct task_struct *task = NULL;
	struct nsproxy *nsp = NULL;
	struct mnt_namespace *ns = NULL;
	struct path root;

	task = current;
	rcu_read_lock();
	nsp = task->nsproxy;
	if (nsp) {
		ns = nsp->mnt_ns;
		if (!ns) {
			kbox_err("oom extend:get mnt_namespace failed.");
			goto RCU_UNLOCK;
		}
		get_mnt_ns(ns);
		if (get_fs_path(task, &root, 1)) {
			kbox_err("oom extend:get_mnt_ns() failed.");
			kbox_put_mnt_ns(ns);
			goto RCU_UNLOCK;
		}
	} else {
		kbox_err("oom extend:task_nsproxy() failed.");
		goto RCU_UNLOCK;
	}
	rcu_read_unlock();

	if (show_mount_memfs(ns) < 0) {
		path_put(&root);
		kbox_put_mnt_ns(ns);
		return -EINVAL;
	}

	path_put(&root);
	kbox_put_mnt_ns(ns);
	kbox_info("memory file system info end.");
	return 0;

RCU_UNLOCK:
	rcu_read_unlock();
	return -EINVAL;
}

static void kbox_show_vmallocinfo(void)
{
	kbox_info("Vmallocinfo Start >>>>>>>>>>>>>");
	kbox_show_seq_info(KBOX_VMALLOC_OP);
	kbox_info("Vmallocinfo End >>>>>>>>>>>>>");
}

static void kbox_show_tasks_info(void)
{
	struct task_struct *p = NULL;
	unsigned long total_rss_all = 0;
	unsigned long total = 0;
	unsigned long task_rss;
	unsigned long total_user = 0;

	for_each_process(p) {
		task_lock(p);
		total++;
		if (unlikely(p->mm == NULL)) {
			task_unlock(p);
			continue;
		}
		total_user++;

		task_rss = get_mm_rss(p->mm);

		total_rss_all += task_rss;
		task_unlock(p);
	}
	kbox_info("show tasks info:(total:total task number; total_user:total user task number.)");
	kbox_info("total=%lu total_user=%lu total_rss_all=%lu(pages)",
		  total, total_user, total_rss_all);
}

static void oom_extend_show(void)
{
	int ret;

	kbox_show_tasks_info();
	kbox_show_vmallocinfo();
	ret = show_mount_info();
	if (ret != 0) {
		kbox_info("show_mount_info error.");
	}
}

static int kbox_oom_callback(struct notifier_block *nfb, unsigned long action, void *hcpu);

static struct notifier_block g_kbox_oom_block = {
	.notifier_call = kbox_oom_callback,
	.priority = 110,
};

static void kbox_write_oom_log(int fd, const unsigned int copy_log_len)
{
	int ret;

	kbox_store_pre_log(fd, copy_log_len);

	ret = kbox_regsiter_store_log();
	if (ret != 0) {
		kbox_err("kbox_regsiter_store_log fail, ret=%d", ret);
	}

	oom_extend_show();

	kbox_unregister_store_log();
}

static void kbox_oom_record_log(unsigned int reason)
{
	int ret;
	unsigned long time;

	time = (unsigned long)local_clock();
	ret = kbox_write_reset_reason(reason, time);
	if (ret != 0) {
		kbox_err("kbox_write_reset err, ret=%d", ret);
		return;
	}

	g_kbox_log_fd = kbox_log_open(time);
	if (g_kbox_log_fd < 0) {
		kbox_err("kbox_log_open fail.");
		return;
	}

	kbox_write_oom_log(g_kbox_log_fd, KBOX_COPY_PRINTK_LOG_LEN_OOM);

	ret = kbox_log_close(g_kbox_log_fd);
	if (ret != 0) {
		kbox_err("kbox_log_close fail");
	}
	g_kbox_log_fd = -1;
}

static int kbox_oom_callback(struct notifier_block *nfb, unsigned long action, void *hcpu)
{
	int tmp_cpu;
	int ret;
	unsigned long flags;

	unused(action);

	if (atomic_cmpxchg(&g_kbox_status, KBOX_STAT_INIT, KBOX_STAT_WORK) == KBOX_STAT_EXIT) {
		kbox_err_ratelimit("kbox is rmmoding, this oom record will loss");
		return NOTIFY_DONE;
	}

	if (&g_kbox_oom_block != nfb) {
		kbox_err("wrong callback for oom chain! self = 0x%pK", nfb);
		goto out;
	}

	tmp_cpu = smp_processor_id();

	kbox_info("catch oom event on cpu %d.", tmp_cpu);

	ret = kbox_event_pre_process(OOM_TYPE);
	if (ret != EVENT_SUBSEQUENT_OK) {
		goto out;
	}
	spin_lock_irqsave(&g_kbox_event_lock, flags);
	kbox_oom_record_log((unsigned int)OOM_TYPE);
	spin_unlock_irqrestore(&g_kbox_event_lock, flags);
out:
	atomic_set(&g_kbox_status, KBOX_STAT_INIT);
	return NOTIFY_DONE;
}

int kbox_oom_init(void)
{
	int ret;

	ret = register_oom_notifier(&g_kbox_oom_block);
	if (ret < 0) {
		kbox_err("oom register hook failed! ret=%d.", ret);
		return ret;
	}

	return ret;
}
void kbox_oom_fini(void)
{
	(void)unregister_oom_notifier(&g_kbox_oom_block);
}
