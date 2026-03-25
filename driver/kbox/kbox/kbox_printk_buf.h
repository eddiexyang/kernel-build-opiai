/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: kbox get sym addr
 * Author: wangshouping
 * Create: 2019-02-14
 */

#ifndef _KBOX_PRINTK_BUF_H_
#define _KBOX_PRINTK_BUF_H_

#define KBOX_COPY_PRINTK_LOG_LEN (128 * 1024)
#define KBOX_COPY_PRINTK_LOG_LEN_RLOCK (2 * 1024 * 1024)
#define KBOX_COPY_PRINTK_LOG_LEN_OOM (4 * 1024 * 1024)
int kbox_printk_tmp_log_buf_malloc(void);
void kbox_printk_tmp_log_buf_free(void);
int kbox_copy_latest_printk_buf_log(unsigned int fd);
void kbox_store_pre_log(int fd, const unsigned int copy_log_len);
int kbox_write_reset_reason(unsigned int reason, unsigned long time);

#endif
