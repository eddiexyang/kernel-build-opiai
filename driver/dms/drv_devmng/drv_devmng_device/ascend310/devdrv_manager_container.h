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


#ifndef DEVDRV_MANAGER_CONTAINER_H
#define DEVDRV_MANAGER_CONTAINER_H

#include <linux/types.h>

#include "devdrv_user_common.h"
#include "devdrv_common.h"
#include "devdrv_mailbox.h"
#include "devdrv_manager_common.h"

struct devdrv_container_tflops_entry {
    u8 uuid[DEVDRV_MANAGER_UUID_NUM];
    u16 group_id;
    u16 tflops;
    u8 exited;
    u8 registered;
};

struct devdrv_container_tflops_table {
    struct devdrv_container_tflops_entry entry[DEVDRV_MINI_TOTAL_TFLOP];
    u32 entry_num;          /* container number, each container owns a unique uuid */
    u32 total_tflops;       /* allocated tflops */
    u32 max_entry;          /* maxium container num */
    u32 max_tflops;         /* maxium tflops */
    u16 group_id_generater; /* increment counter, inc when new docker */
};

struct devdrv_container_mailbox_msg {
    struct devdrv_mailbox_message_header header;

    u16 group_id;
    u16 tflops;

    u32 reserved1[12];

    u8 plat_type;
    u8 reserved2[3];
};

int devdrv_is_in_container(void);
int devdrv_manager_h2d_container(u32 devid, void *msg, u32 in_len, u32 *ack_len);
int devdrv_manager_container_register_session(u32 devid, u8 uuid[], int uuid_size, u32 tflops);
int devdrv_manager_container_register_process(u32 devid, u8 uuid[], int uuid_size, pid_t pid);
int devdrv_manager_check_running_mode(u32 devid, u32 *mode);
int devdrv_manager_container_process(struct file *filep, unsigned long arg);
int devdrv_virtual_id_to_physical_id(u32 virtual_id, u32 *physical_id);

int devdrv_manager_container_logical_id_to_physical_id(u32 logical_dev_id, u32 *physical_dev_id, u32 *vfid);
int devdrv_manager_container_is_in_container(void);
int devdrv_manager_container_is_in_admin_container(void);

#endif
