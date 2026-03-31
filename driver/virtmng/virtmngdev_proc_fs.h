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
 * Create: 2022-12-05
 */

#ifndef VIRTMNGDEV_PROC_FS_H
#define VIRTMNGDEV_PROC_FS_H

struct vmngd_user_input {
    u32 dev_id;
    u32 vfid;
    u32 vfg_id;
};

struct vmngd_procfs_entry {
    struct proc_dir_entry *dev_id;
    struct proc_dir_entry *vf_id;
    struct proc_dir_entry *vfg_id;
    struct proc_dir_entry *total_resource_info;
    struct proc_dir_entry *remain_resource_info;
    struct proc_dir_entry *each_resource_info;
    struct proc_dir_entry *vfg_resource_info;
    struct proc_dir_entry *vdev_ctrl_info;
    struct proc_dir_entry *all_resource_info;
};

int vmngd_proc_fs_init(void);
void vmngd_proc_fs_uninit(void);

#endif
