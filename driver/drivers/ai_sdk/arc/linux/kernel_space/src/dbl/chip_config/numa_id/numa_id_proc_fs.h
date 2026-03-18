/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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
 * Create: 2022-10-15
 */

#ifndef NUMA_ID_PROC_FS_H
#define NUMA_ID_PROC_FS_H

#include <linux/proc_fs.h>

void numa_id_fs_add_device(u32 dev_id);
void numa_id_fs_del_device(u32 dev_id);
void numa_id_fs_init(struct proc_dir_entry *fs_entry);
void numa_id_fs_uninit(struct proc_dir_entry *fs_entry);

#endif /* NUMA_ID_PROC_FS_H */
