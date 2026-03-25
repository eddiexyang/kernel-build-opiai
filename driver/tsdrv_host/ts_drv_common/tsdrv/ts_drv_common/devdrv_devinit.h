/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * Description:
 * Author: huawei
 * Create: 2019-10-15
 */
#ifndef DEVDRV_DEVINIT_H
#define DEVDRV_DEVINIT_H

#include <linux/cdev.h>
#include <linux/ioctl.h>

#include "devdrv_common.h"
#include "tsdrv_ioctl.h"
#include "tsdrv_kernel_common.h"
#ifdef AOS_LLVM_BUILD
#include <linux/mm.h>
#include <linux/wait.h>
#endif

#define TSDRV_UNREGISTER_MSLEEP     100
#define TSDRV_PRINT_FREQUENCY_CTRL  100
#define DEVDRV_CACHELINE_SIZE_64 64
#ifndef AOS_LLVM_BUILD
int devdrv_mmap(struct file *filep, struct vm_area_struct *vma);
#endif

int tsdrv_thread_bind_irq_alloc(u32 devid, u32 tsid);
void tsdrv_thread_bind_irq_free(u32 devid, u32 tsid, int irq);
void tsdrv_thread_bind_irq_set_wait_para(u32 devid, u32 tsid, int irq, wait_queue_head_t *wait_queue,
    atomic_t *wait_flag);
int tsdrv_cq_irq_init(u32 devid, u32 tsnum);
void tsdrv_cq_irq_exit(u32 devid, u32 tsnum);
int tsdrv_get_pfvf_type_by_devid(u32 devid);

void tsdrv_update_cq_tail_and_sq_head(struct tsdrv_ts_resource *ts_res,
    struct devdrv_ts_cq_info *cq_info, struct devdrv_cq_sub_info *cq_sub_info);
int devdrv_ioctl_report_wait(struct tsdrv_ctx *ctx, struct devdrv_ioctl_arg *arg);
int devdrv_drv_register(struct devdrv_info *dev_info);
void devdrv_drv_unregister(struct devdrv_info *dev_info);
int hvtsdrv_ioctl_get_ssid(struct tsdrv_ctx *ctx, struct devdrv_ioctl_arg *arg);
int devdrv_open(struct inode *inode, struct file *filep);
int devdrv_release(struct inode *inode, struct file *filep);
int devdrv_is_need_invalid_cache(void);

long devdrv_ioctl(struct file *filep, unsigned int cmd, unsigned long arg);

#endif
