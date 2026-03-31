/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: drv log
 * Author:
 * Create: 2019-02-14
 */

#ifndef _KBOX_DRIVER_OPER_H_
#define _KBOX_DRIVER_OPER_H_
#include <linux/file.h>

#include <kbox/kbox_ioctl.h>

#define LOG_ALIGN 8
#define KBOX_LOG_NAME "kbox.txt"
#define DEFAULT_KBOX_LOG "/var/log/kbox/kbox.txt"
#define KBOX_MEMORY_IMAGE "/var/log/kbox/kbox.image"
#define KBOX_WRITE_FLAG (O_CREAT | O_RDWR | O_NOFOLLOW | O_TRUNC | O_LARGEFILE)

extern char kbox_log_path[KBOX_PATH_MAXLEN];
extern atomic_t g_drv_printk_flag;
extern spinlock_t g_drv_write_lock;
extern int kbox_export_dev_info(unsigned int type, unsigned int index);
extern int kbox_query_reset(kbox_query_ioc *query_cb);
extern int kbox_write_reset(unsigned int event, unsigned long time);
extern int kbox_log_open(unsigned long time);
extern int kbox_log_close(int fd);
extern int kbox_log_write(int fd, const char *text, unsigned int size);
extern int kbox_get_nospace_flag(void);
extern void kbox_export_memory_image(void);
#endif
