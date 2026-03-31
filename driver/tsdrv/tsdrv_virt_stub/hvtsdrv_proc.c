/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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
#include "tsdrv_common.h"
#include "tsdrv_log.h"
#include "hvtsdrv_proc.h"

s32 hvtsdrv_vpc_msg_send(u32 devid, u32 fid, struct vtsdrv_msg *msg)
{
    return 0;
}
EXPORT_SYMBOL_UNRELEASE(hvtsdrv_vpc_msg_send);

int hvtsdrv_open(struct inode *inode, struct file *filep)
{
    return 0;
}

int hvtsdrv_release(struct inode *inode, struct file *filep)
{
    return 0;
}

int hvtsdrv_mmap(struct file *filep, struct vm_area_struct *vma)
{
    return 0;
}

long hvtsdrv_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
    return 0;
}

