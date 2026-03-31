/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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

#ifndef HVTSDRV_PROC_H
#define HVTSDRV_PROC_H

#include "virtmng_interface.h"
#include "vtsdrv_common.h"

#define HVTSDRV_DEV_OPEND 1
#define HVTSDRV_DEV_CLOSED 0

#define HVTSDRV_ALLOC_ID 1

s32 hvtsdrv_com_msg_recv(u32 dev_id, u32 fid, struct vmng_rx_msg_proc_info *proc_info);
s32 hvtsdrv_vpc_msg_recv(u32 dev_id, u32 fid, struct vmng_rx_msg_proc_info *proc_info);
s32 hvtsdrv_vpc_msg_send(u32 devid, u32 fid, struct vtsdrv_msg *msg);
struct file_operations *hvtsdrv_get_vdavinci_fop(void);
int hvtsdrv_open(struct inode *inode, struct file *filep);
int hvtsdrv_release(struct inode *inode, struct file *filep);
int hvtsdrv_mmap(struct file *filep, struct vm_area_struct *vma);
long hvtsdrv_ioctl(struct file *filep, unsigned int cmd, unsigned long arg);

#endif
