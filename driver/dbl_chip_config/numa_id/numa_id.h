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
 * Create: 2022-07-27
 */

#ifndef NUMA_ID_H
#define NUMA_ID_H

#include <linux/types.h>
#include <linux/proc_fs.h>

void numa_id_init(struct proc_dir_entry *fs_entry);
void numa_id_uninit(struct proc_dir_entry *fs_entry);

int dbl_get_ddr_ai_nid(u32 devid, int nids[], int num);
int dbl_get_hbm_ai_nid(u32 devid, int nids[], int num);
int dbl_get_all_ai_nid(u32 devid, int nids[], int num);

#endif /* NUMA_ID_H */