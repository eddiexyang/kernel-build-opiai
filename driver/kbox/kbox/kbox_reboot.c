/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: kbox record reboot scene log
 * Author: wangshouping
 * Create: 2019-02-14
 */
#include "kbox_reboot.h"

#include <linux/binfmts.h>
#include <linux/kern_levels.h>
#include <linux/notifier.h>
#include <linux/pagemap.h>
#include <linux/sched/mm.h>
#include <linux/securec.h>
#include <linux/sched/clock.h>
#include <linux/os_kbox.h>
#include <linux/version.h>

#include "../driver/drv_oper.h"
#include "kbox_common.h"
#include "kbox_common_interface.h"
#include "kbox_emerge.h"
#include "kbox_find_sym_addr.h"
#include "kbox_print.h"
#include "kbox_printk_buf.h"
#include "kbox_reboot_timer.h"
#include "../driver/drv_main.h"

#define MAX_CALLS 5
#define CMDLINE_BUF_LEN 256
#define ARGC_NUM 2
#define REBOOTER_INFO_LEN 64

int g_kbox_log_fd = -1;
spinlock_t g_kbox_event_lock;

static char g_cmdline_buf[CMDLINE_BUF_LEN];
static volatile unsigned long g_reboot_method_flags;
static int g_rebooter_type;
static char g_rebooter_info[REBOOTER_INFO_LEN];

static int kbox_get_cmdline(struct task_struct *task, char *buffer, int buflen)
{
	int res = 0;
	int i;
	int len;
	struct mm_struct *mm = get_task_mm(task);

	if (mm == NULL) {
		goto out;
	}
	if (!mm->arg_end) {
		goto out_mm; /* Shh! No looking before we're done */
	}

	len = (int)(mm->arg_end - mm->arg_start);

	if (len > buflen) {
		len = buflen;
	}
	if (kbox_set_cpu_and_pid_ptr != NULL) {
		kbox_set_cpu_and_pid_ptr();
	}
	res = access_process_vm(task, mm->arg_start, buffer, len, 0);
	if (res <= 0) {
		if (kbox_clear_cpu_and_pid_ptr != NULL) {
			kbox_clear_cpu_and_pid_ptr();
		}
		goto out_mm;
	}
	if (kbox_clear_cpu_and_pid_ptr != NULL) {
		kbox_clear_cpu_and_pid_ptr();
	}
	for (i = 0; i < res; i++) {
		if (buffer[i] == '\0') {
			buffer[i] = ' ';
		}
	}
	buffer[i - 1] = '\0';
out_mm:
	mmput(mm);
out:
	return res;
}

static int get_login_type(const char *login_cmd, char **pos_ptr)
{
	int login_type;
	*pos_ptr = strstr(login_cmd, "pts");
	if (*pos_ptr == NULL) {
		*pos_ptr = strstr(login_cmd, "login");
		if (*pos_ptr == NULL)
			return -1;
		else
			login_type = TTY_LOGIN;
	} else {
		login_type = IP_LOGIN;
	}
	return login_type;
}

static void print_rebooter_info(const struct utmp *login_rcd)
{
	int check_ret;

	if (login_rcd == NULL)
		return;
	if (login_rcd->ut_host[0] != '\0') {
		check_ret = snprintf_s(g_rebooter_info, sizeof(g_rebooter_info), sizeof(g_rebooter_info) - 1,
			"%s", login_rcd->ut_host);
		g_rebooter_type = IP_LOGIN;
	} else {
		check_ret = snprintf_s(g_rebooter_info, sizeof(g_rebooter_info), sizeof(g_rebooter_info) - 1,
			"%s", login_rcd->ut_line);
		g_rebooter_type = TTY_LOGIN;
	}
	if (check_ret == -1)
		printk("%s: snprintf_s err %d\n", __FUNCTION__, check_ret);
}

static void show_rebooer_info(const char *buf_cmd, pid_t pid)
{
	char *ptr = NULL;
	char rebooter_tty[32] = {0};
	struct file *fp = NULL;
	loff_t pos = 0;
	struct utmp cur_rcd;
	int rcd_len = sizeof(struct utmp);
	int ret;
	int login_type;

	login_type = get_login_type(buf_cmd, &ptr);
	if (login_type == -1 && ptr == NULL)
		return;

	while (ptr[strlen(ptr) - 1] == ' ')
		ptr[strlen(ptr) - 1] = 0;
	if (login_type == IP_LOGIN) {
		ret = snprintf_s(rebooter_tty, sizeof(rebooter_tty), sizeof(rebooter_tty) - 1, "%s", ptr);
		if (ret == -1) {
			printk("%s: snprintf_s err %d\n", __FUNCTION__, ret);
			return;
		}
	}
	fp = filp_open(UTMP_FILE, O_RDONLY, 0);
	if (IS_ERR(fp)) {
		printk("open %s fail!\n", UTMP_FILE);
		return;
	}
	while (kernel_read(fp, &cur_rcd, rcd_len, &pos) == rcd_len) {
		if (cur_rcd.ut_type != USER_PROCESS)
			continue;
		if (login_type == IP_LOGIN && strncmp(rebooter_tty, cur_rcd.ut_line, sizeof(cur_rcd.ut_line))) {
			continue;
		} else if (login_type == TTY_LOGIN && cur_rcd.ut_pid != pid) {
			continue;
		}
		print_rebooter_info(&cur_rcd);
	}
	filp_close(fp, NULL);
}

static void kbox_show_reboot_trace(void)
{
	struct task_struct *p = NULL;
	unsigned int nr_calls = 0;
	int ret;

	g_rebooter_info[0] = '\0';
	kbox_info("call relation(max depth=%d.)", MAX_CALLS);
	printk("kbox");
	if (current->pid == 1) {
		printk("<- systemd\n");
		return;
	}
	p = current->real_parent;
	while (p != NULL && nr_calls < MAX_CALLS) {
		nr_calls++;

		ret = memset_s(g_cmdline_buf, sizeof(g_cmdline_buf), 0, sizeof(g_cmdline_buf));
		if (ret != 0) {
			kbox_err("memset_s fail, ret=%d.", ret);
			break;
		}
		(void)kbox_get_cmdline(p, g_cmdline_buf, CMDLINE_BUF_LEN);
		g_cmdline_buf[CMDLINE_BUF_LEN - 1] = '\0';
		if (g_cmdline_buf[0] == '\0') {
			printk(" <- [%s](pid: %d)", p->comm, p->pid);
		} else {
			printk(" <- [%s](pid: %d)", g_cmdline_buf, p->pid);
		}
		show_rebooer_info(g_cmdline_buf, p->pid);
		p = p->real_parent;
		if (p == NULL) {
			break;
		}
		if (p->pid == current->pid) {
			break;
		}
	}
	printk("\n");
	if (g_rebooter_info[0] != '\0') {
		if (g_rebooter_type == IP_LOGIN)
			printk("rebooter info: ip = %s", g_rebooter_info);
		else if (g_rebooter_type == TTY_LOGIN)
			printk("rebooter info: tty = %s", g_rebooter_info);
	}
}

static int kbox_get_page_content(const struct linux_binprm *bprm, unsigned long *pos,
	char *tmp_buf, unsigned long tmp_buf_size)
{
	char *kaddr = NULL;
	struct page *page = NULL;
	int ret;
	unsigned long len;
	unsigned long offset = 0;

	if (bprm == NULL || tmp_buf == NULL || pos == NULL || tmp_buf_size == 0) {
		return -EINVAL;
	}
	while (true) {
		ret = get_user_pages_remote(bprm->mm, *pos, 1, FOLL_FORCE, &page,
					    NULL);
		if (ret <= 0) {
			kbox_err("fail to get page for args, skip checking fake reboot.");
			return ret;
		}
		kaddr = kmap(page);
		kaddr += (*pos) % PAGE_SIZE;
		len = strnlen(kaddr, PAGE_SIZE - (*pos) % PAGE_SIZE);
		/*
		 * prevent infinite loop when arg is at the end of one page, '\0' is the
		 * first char of the next page, see below.
		 * -----------------------------
		 * PAGE 1 | xxxxx/usr/sbin/init |
		 * -----------------------------
		 * PAGE 2 | 0\xxxxxxxxxxxxxxxxx |
		 * -----------------------------
		 */
		if (len == 0 && (*pos) % PAGE_SIZE == 0) {
			kunmap(page);
			put_page(page);
			break;
		}
		/*
		 * not the whole arg is needed to check the fake reboot command,
		 * but offset & pos need to change to right position.
		 */
		if (len + offset < tmp_buf_size) {
			ret = memcpy_s(tmp_buf + offset, tmp_buf_size - offset, kaddr, len);
			if (ret != 0) {
				kbox_err("memcpy_s fail, ret=%d.", ret);
				kunmap(page);
				put_page(page);
				return ret;
			}
		} else if (tmp_buf_size - 1 > offset) {
			ret = memcpy_s(tmp_buf + offset, tmp_buf_size - offset, kaddr,
				tmp_buf_size - offset - 1);
			if (ret != 0) {
				kbox_err("memcpy_s fail, ret=%d.", ret);
				kunmap(page);
				put_page(page);
				return ret;
			}
		}
		offset += len;
		*pos += len;
		kunmap(page);
		put_page(page);
		if (*pos % PAGE_SIZE != 0) {
			break;
		}
	}
	return TRUE;
}

static bool kbox_is_reboot_cmd(const char *cmd)
{
	if (cmd == NULL) {
		return 0;
	}

	if (g_kbox_type == TYPE_PHY_SOTRAGE) {
		if (strncmp(cmd, "reboot", sizeof("reboot")) == 0) {
			return 0;
		}
	}
	if ((strncmp(cmd, "reboot", sizeof("reboot")) == 0) ||
		(strncmp(cmd, "poweroff", sizeof("poweroff")) == 0) ||
		(strncmp(cmd, "init", sizeof("init")) == 0) ||
		(strncmp(cmd, "telinit", sizeof("telinit")) == 0) ||
		(strncmp(cmd, "halt", sizeof("halt")) == 0) ||
		(strncmp(cmd, "shutdown", sizeof("shutdown")) == 0)) {
		return 1;
	}
	if (strncmp(cmd, "insmod_bsp_reboot_ko.sh", sizeof("insmod_bsp_reboot_ko.sh")) == 0) {
		if (test_and_set_bit(STATE_REBOOT_KO, &g_reboot_method_flags)) {
			kbox_info("the script:insmod_bsp_reboot_ko.sh is execv dup!");
			return 0;
		}
		return 1;
	}

	return 0;
}

static int kbox_check_arg_command(const struct linux_binprm *bprm, const char *tmp_buf, int argc,
	int *is_init_cmd, int *is_fake_reboot_cmd)
{
	const char *cmd = NULL;

	if (bprm == NULL || tmp_buf == NULL || is_init_cmd == NULL || is_fake_reboot_cmd == NULL) {
		return -EINVAL;
	}
	/* check first arg (command) */
	if (argc == bprm->argc) {
		cmd = strrchr(tmp_buf, '/');
		if (cmd != NULL) {
			cmd++;
		} else {
			cmd = tmp_buf;
		}
		if ((strncmp(cmd, "init", sizeof("init")) == 0) ||
		    (strncmp(cmd, "telinit", sizeof("telinit")) == 0)) {
			*is_init_cmd = 1;
		} else {
			if (!kbox_is_reboot_cmd(cmd))
				*is_fake_reboot_cmd = 1;
			return -EINVAL;
		}
	}

	if ((*is_init_cmd) != 0 && bprm->argc != ARGC_NUM) {
		*is_fake_reboot_cmd = 1;
		kbox_info("fake reboot cmd (argc is %d.)", bprm->argc);
		return -EINVAL;
	}

	if ((*is_init_cmd) != 0 && (argc == bprm->argc - 1) &&
		(strncmp(tmp_buf, "0", sizeof("0")) != 0) &&
		(strncmp(tmp_buf, "6", sizeof("6")) != 0)) {
		*is_fake_reboot_cmd = 1;
		kbox_info("fake reboot cmd (2nd arg is %s)", tmp_buf);
		return -EINVAL;
	}
	return 0;
}

static int kbox_is_fake_reboot_cmd(const struct linux_binprm *bprm)
{
	int argc;
	unsigned long pos;
	unsigned long tmp_buf_size = 20;
	char *tmp_buf = NULL;
	int ret;
	int init_cmd = 0;
	int is_fake_reboot_cmd = 0;

	if (bprm == NULL) {
		kbox_err("bprm is NULL");
		return -EINVAL;
	}
	tmp_buf = kmalloc(tmp_buf_size, GFP_KERNEL);
	if (tmp_buf == NULL) {
		kbox_err("no mem for checking fake reboot, skip it.");
		return is_fake_reboot_cmd;
	}

	argc = bprm->argc;
	pos = bprm->p;

	while (argc > 0) {
		ret = memset_s(tmp_buf, tmp_buf_size, 0, tmp_buf_size);
		if (ret != 0) {
			goto out;
		}

		/*
		 * an arg may be placed in two or more pages, '\0' is the last char of
		 * the arg. Check whether the arg reaches the '\0' by comparing strnlen
		 * with (PAGE_SIZE - pos % PAGE_SIZE)
		 */
		ret = kbox_get_page_content(bprm, &pos, tmp_buf, tmp_buf_size);
		if (ret != TRUE) {
			goto out;
		}
		pos++;
		ret = kbox_check_arg_command(bprm, tmp_buf, argc, &init_cmd, &is_fake_reboot_cmd);
		if (ret != 0) {
			goto out;
		}
		argc--;
	}

out:
	kfree(tmp_buf);

	return is_fake_reboot_cmd;
}

static void kbox_write_reboot_log(int fd, const unsigned int copy_log_len)
{
	kbox_dump_stack_print_info(KERN_DEFAULT);
	kbox_store_pre_log(fd, copy_log_len);
}

static void kbox_reboot_record_log(unsigned int reason)
{
	int ret;
	unsigned long time;

	time = (unsigned long)local_clock();
	ret = kbox_write_reset_reason(reason, time);
	if (ret != 0) {
		kbox_err("kbox_write_reset err ret=%d", ret);
		return;
	}

	g_kbox_log_fd = kbox_log_open(time);
	if (g_kbox_log_fd < 0) {
		kbox_err("kbox_log_open fail.\n");
		return;
	}

	kbox_write_reboot_log(g_kbox_log_fd, KBOX_COPY_PRINTK_LOG_LEN);

	ret = kbox_log_close(g_kbox_log_fd);
	if (ret != 0) {
		kbox_err("kbox_log_close fail.");
	}
	g_kbox_log_fd = -1;
}

static void kbox_reboot_notifier_callback(const char *cmd)
{
	int ret;
	unsigned long flags;

	if (atomic_cmpxchg(&g_kbox_status, KBOX_STAT_INIT, KBOX_STAT_WORK) == KBOX_STAT_EXIT) {
		kbox_err_ratelimit("kbox is rmmoding, this reboot record will loss");
		return;
	}

	if (cmd == NULL) {
		atomic_set(&g_kbox_status, KBOX_STAT_INIT);
		return;
	}
	ret = kbox_event_pre_process(REBOOT_TYPE);
	if (ret != EVENT_SUBSEQUENT_OK) {
		atomic_set(&g_kbox_status, KBOX_STAT_INIT);
		return;
	}
	spin_lock_irqsave(&g_kbox_event_lock, flags);
	set_bit(REBOOT_TYPE, &g_emerge_recorded_flags);
	kbox_info("last catch the trace cmd: %s", cmd);
	kbox_reboot_record_log((unsigned int)REBOOT_TYPE);
	kbox_notify_die(KBOX_DIE_GRACE);
	if (!atomic_read(&g_reboot_timer_exist)) {
		reboot_timer_init();
	}
	spin_unlock_irqrestore(&g_kbox_event_lock, flags);
	atomic_set(&g_kbox_status, KBOX_STAT_INIT);
}

static int kbox_bprm_check_security(struct linux_binprm *bprm)
{
	const char *filename = NULL;
	const char *cmd = NULL;

	smp_mb();
	if (bprm == NULL) {
		return -EINVAL;
	}
	filename = (char *)bprm->filename;
	if (likely(!IS_ERR(filename))) { /* judge filename is legal */
		cmd = strrchr(filename, '/');  /* find the first / location */
		if (cmd != NULL) {
			cmd++;
		} else {
			cmd = filename;
		}

		if (!kbox_is_reboot_cmd(cmd)) {
			goto out;
		}

		if (!uid_eq(bprm->cred->euid, KUIDT_INIT(0))) {
			kbox_info("not authorized user(effective UID: %d) is executing 'reboot' cmd",
				bprm->cred->euid.val);
			goto out;
		}
		/* check if the command is fake reboot, like init 3 */
		if ((strncmp(cmd, "init", sizeof("init")) == 0) || (strncmp(cmd, "telinit", sizeof("telinit")) == 0)) {
			if (kbox_is_fake_reboot_cmd(bprm)) {
				kbox_info("found a fake reboot cmd: %s", cmd);
				goto out;
			}
		}
		/* check if the command is executed in container */
		if (current->thread_pid->level != 0) {
			kbox_info("found a reboot cmd in container: %s", cmd);
			goto out;
		}
		/* if insmod_bsp_reboot_ko.sh, kbox start recording */
		if (test_bit(STATE_REBOOT_KO, &g_reboot_method_flags)) {
			kbox_show_reboot_trace();
			kbox_reboot_notifier_callback(cmd);
			goto out;
		}
		if (test_and_set_bit(STATE_REBOOT_CMD, &g_reboot_method_flags)) {
			/* reboot cmd re-entry£¬reboot event is processing,
			 * waiting for the first cmd be processed
			 */
			kbox_err("another reboot cmd is processing! this cmd is:%s", cmd);
			goto out;
		}
		kbox_info("first catch the trace cmd:%s", cmd);
		kbox_show_reboot_trace();
	} else {
		kbox_info("the file name in struct bprm is err ptr:%pK!", filename);
	}
out:
	return 0;
}

static int kbox_task_kill(struct task_struct *p, struct kernel_siginfo *info, int sig,
	const struct cred *cred)
{
	int ret = 0;
	const char *cmd = NULL;

	unused(info);
	smp_mb();
	if (p == NULL) {
		return ret;
	}
	if (p->pid == INIT_TASK_PID || current->pid == INIT_TASK_PID) {
		switch (sig) {
			case SIGTERM:
				cmd = "SIGTERM";
				break;
			case SIGUSR1:
				cmd = "SIGUSR1";
				break;
			case SIGUSR2:
				cmd = "SIGUSR2";
				break;
			default:
				break;
		}
	}
	if (cmd != NULL) {
		/*
 		 * in the scenario that when process stop by systemd,
		 * kbox will go into here, but only when reboot detected
		 * callback will be called.
		 */
		if (test_bit(STATE_REBOOT_CMD, &g_reboot_method_flags)) {
			if (!test_and_set_bit(STATE_REBOOT_SYSTEMD, &g_reboot_method_flags)) {
				kbox_reboot_notifier_callback(cmd);
			}
		}
	}
	return ret;
}

#ifdef CONFIG_SECURITY
static const struct lsm_id kbox_lsmid = {
	.name = "kbox_capability",
	.id = LSM_ID_UNDEF,
};

static struct security_hook_list kbox_capability_hooks[] = {
	{
		.hook = { .bprm_check_security = kbox_bprm_check_security },
	},
	{
		.hook = { .task_kill = kbox_task_kill },
	},
};

static void kbox_init_capability_hooks(void)
{
	if (kbox_static_calls_table == NULL)
		return;

	kbox_capability_hooks[0].scalls = kbox_static_calls_table->bprm_check_security;
	kbox_capability_hooks[1].scalls = kbox_static_calls_table->task_kill;
}
#endif /* CONFIG_SECURITY */

static void kbox_register_reboot_notifier(void)
{
#ifdef CONFIG_SECURITY
	kbox_init_capability_hooks();
	if (kbox_security_add_hooks == NULL || kbox_static_calls_table == NULL)
		return;
	kbox_security_add_hooks(kbox_capability_hooks, ARRAY_SIZE(kbox_capability_hooks),
			&kbox_lsmid);
#endif /* CONFIG_SECURITY */
}

static void kbox_unregister_reboot_notifier(void)
{
#ifdef CONFIG_SECURITY
	if (kbox_security_delete_hooks != NULL && kbox_static_calls_table != NULL)
		kbox_security_delete_hooks(kbox_capability_hooks,
				  ARRAY_SIZE(kbox_capability_hooks));
#endif /* CONFIG_SECURITY */
}

void kbox_reboot_init(void)
{
	kbox_register_reboot_notifier();
}

void kbox_reboot_fini(void)
{
	/* unregister reboot hook */
	kbox_unregister_reboot_notifier();
	reboot_timer_fini();
}
