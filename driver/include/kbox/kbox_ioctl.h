/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: kbox ioctl
 * Author:
 * Create: 2019-03-07
 */

#ifndef _KBOX_KBOX_IOCTL_H_
#define _KBOX_KBOX_IOCTL_H_

#include "kbox_event.h"

#define KBOX_TYPE_NAME_LEN (32)
#define KBOX_DEV_NAME_MAXLEN (KBOX_TYPE_NAME_LEN)
#define KBOX_PATH_MAXLEN (4096)

typedef struct {
	char dev_name[KBOX_DEV_NAME_MAXLEN];
	unsigned int type;
	unsigned int index;
	char kbox_log_path[KBOX_PATH_MAXLEN];
} kbox_export_ioc;

typedef struct {
	char dev_name[KBOX_DEV_NAME_MAXLEN];
	struct reset_info events[MAX_RESET];
	unsigned int last_status;
	unsigned int current_status;
	unsigned int query_num;
} kbox_query_ioc;

typedef union {
	kbox_export_ioc export;
	kbox_query_ioc query;
} un_kbox_ioctl_cb;

typedef int (*KBOX_IOCTL_FUNC)(const un_kbox_ioctl_cb *);
typedef struct {
	unsigned int cmd;
	unsigned long cb_size;
	KBOX_IOCTL_FUNC func;
} kbox_ioc_tbl;

/* Use 'K' as magic number */
#define KBOX_IOC_MAGIC 'k'
#define KBOX_IOC_EXPORT_DEV _IOWR(KBOX_IOC_MAGIC, 7, kbox_export_ioc)
#define KBOX_IOC_QUERY _IOWR(KBOX_IOC_MAGIC, 11, kbox_query_ioc)

#endif
