/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright(c) Huawei Technologies Co., Ltd.
 */

#ifndef __LINUX_BOOTDOT_H
#define __LINUX_BOOTDOT_H

#include <linux/types.h>

#define BOOTDOT_DEFAULT 10U
#define BOOTDOT_BLKCK_BOOT_VALID 0x76

int bootdot_init_blk(u32 block_id, u32 magic, u32 execption_id, u32 expect_status);
int bootdot_set_blk(u32 block_id, u32 magic, u32 current_status);

#endif /* __LINUX_BOOTDOT_H */
