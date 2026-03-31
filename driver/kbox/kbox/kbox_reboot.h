/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: kbox record reboot scene log
 * Author: wangshouping
 * Create: 2019-02-14
 */

#ifndef __KBOX_REBOOT_H
#define __KBOX_REBOOT_H

#include <linux/sched.h>

/* init id */
#define INIT_TASK_PID 1
/* reboot flags */
#define STATE_REBOOT_CMD 0
#define STATE_REBOOT_SYSTEMD 1
#define STATE_REBOOT_KO 2
void kbox_reboot_init(void);
void kbox_reboot_fini(void);

#define UT_LINESIZE     32
#define UT_NAMESIZE     32
#define UT_HOSTSIZE     256

#define USER_PROCESS 7

#define UTMP_FILE "/var/run/utmp"

struct exit_status {
	short int e_termination;    /* Process termination status.  */
	short int e_exit;           /* Process exit status.  */
};

/* refer to utmp.h for who */
struct utmp {
	short int ut_type;            /* Type of login.  */
	pid_t ut_pid;                 /* Process ID of login process.  */
	char ut_line[UT_LINESIZE];    /* Devicename.  */
	char ut_id[4];                /* Inittab ID.  */
	char ut_user[UT_NAMESIZE];    /* Username.  */
	char ut_host[UT_HOSTSIZE];    /* Hostname for remote login.  */
	struct exit_status ut_exit;   /* Exit status of a process */
#ifdef CONFIG_X86_64
	int32_t ut_session;           /* Session ID, used for windowing.  */
	struct {
		int32_t tv_sec;             /* Seconds.  */
		int32_t tv_usec;            /* Microseconds.  */
	} ut_tv;
#else
	/* The ut_session and ut_tv fields is 64bit in 64bit system */
	long int ut_session;          /* Session ID, used for windowing.  */
	struct timespec64 ut_tv;         /* Time entry was made.  */
#endif
	int32_t ut_addr_v6[4];        /* Internet address of remote host.  */
	char unused[20];
};

#define IP_LOGIN 1
#define TTY_LOGIN 2

#endif
