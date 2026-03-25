/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: kbox log manage
 * Author:
 * Create: 2019-02-14
 */

#ifndef _KBOX_DRIVER_MAIN_H_
#define _KBOX_DRIVER_MAIN_H_

#include <linux/rtc.h>
#include <linux/types.h>

#include <kbox/kbox_event.h>

#define MANAGE_AREA_SIZE (8 * 1024)
#define RESET_AREA_SIZE (4 * 1024)
#define MAX_KBOX_SIZE (128 * 1024 * 1024)
#define MIN_KBOX_SIZE (2 * 1024 * 1024)
#define KBOX_LOG_OFFSET (MANAGE_AREA_SIZE + RESET_AREA_SIZE)

#define TYPE_PHY_MEM 1
#define TYPE_PHY_SYM 2
#define TYPE_PHY_SOTRAGE 3
#define TYPE_PHY_PCIE 4

#define KBOX_MANAGE_MAGIC 0x20190326UL
#define KBOX_NOSPACE_MAGIC 0x12121212UL

#define RESET_MANAGE_RESERVE_LEN 4
#define MANAGE_RESERVE_LEN 8
/* this structs store in reset area */
struct log_reset_manage {
	unsigned long magic;
	/* log reset area start offset */
	unsigned int reset_offset;
	/* next write idx */
	unsigned int next_idx;
	/* save how many times kbox record durying last reboot */
	unsigned int last_status;
	/* save current recording status */
	unsigned int current_status;
	unsigned long reserve[RESET_MANAGE_RESERVE_LEN];
};

/*   <log area sketch>
 * --------------------- *|
 *      *********      *|
 * ------log1 end------- *|
 *      pad align      *| --|
 * ------log2 offset---- *|   | => one log area size
 *      log2 text      *| --|
 * ------log2 end------- *|
 *      log remain  <== *|==== next log write start
 *      pad align      *|
 * ------logx offset---- *|
 *      logx text      *|
 *      **********     *|
 *  --------------------- *|
 */

struct log_content {
	unsigned long magic;
	unsigned int log_size;
	unsigned int remain_size;
	unsigned int pad;
	unsigned int offset;
	unsigned long ts_nsec;
	struct rtc_time time;
	unsigned long valid;
};

struct kbox_log_manage {
	unsigned long magic;
	/* kbox log area start offset */
	unsigned int log_offset;
	/* next write idx */
	unsigned int next_idx;
	/* oldest log idx */
	unsigned int oldest_idx;
	struct log_content log_content[MAX_LOG_CONTENT];
	unsigned long reserve[MANAGE_RESERVE_LEN];
};

/* kbox global manage struct */
struct kbox_store_manage {
	unsigned long magic;
	unsigned long check_sum;
	struct log_reset_manage reset_manage;
	struct kbox_log_manage log_manage;
	unsigned long reserve[MANAGE_RESERVE_LEN];
};

extern unsigned long g_kbox_phy_start;
extern unsigned long g_kbox_size;
extern unsigned long g_kbox_va_start;
extern char *g_kbox_mem_start_sym;
extern char *g_kbox_mem_size_sym;
extern unsigned long g_kbox_type;
extern struct semaphore g_kbox_sem;

#endif
