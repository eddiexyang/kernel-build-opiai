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

#ifndef _DEVDRV_IPC_COMMON_H_
#define _DEVDRV_IPC_COMMON_H_

#include <linux/notifier.h>

#define IPC_VRD_SHIFT	   8
#define IPC_VRD_MASK	   0xff
#define IPC_DATA1_OFFSET   8
#define IPC_DATA2_OFFSET   16
#define IPC_DATA3_OFFSET   24
#define PERCENT_UNIT       100
#define PERCENT_INTER_MEDIATE_DATE 50

struct ipc_msg {
    void *send_buf;
    void *recv_buf;
    unsigned int send_len;
    unsigned int recv_len;
};

struct ioctl_ipc {
    unsigned int dev_id;
    unsigned char target_id;
    unsigned char main_cmd;
    unsigned char sub_cmd;
    struct ipc_msg msg;
};

struct drv_ipc_msg {
    unsigned char cmd_type;     /* main cmd */
    unsigned char cmd_id;       /* sub cmd */
    unsigned char dest;         /* target id */
    unsigned char src;          /* source id */
    unsigned short seq;         /* ipc msg id */
    unsigned char msg_type : 1; /* 0:request 1:response */
    unsigned char reserved : 7; /* reserved */
    unsigned char len;          /* data len */
    unsigned char data[22];     /* data - 22 bytes */
    unsigned short crc;         /* crc check sum */
};

struct drv_ipc_user_msg {
    unsigned int dev_id;
    unsigned char target_id;
    unsigned char main_cmd;
    unsigned char sub_cmd;
    struct drv_ipc_msg msg;
    void *priv;
};

struct drv_ipc_module_info {
    unsigned int target_id;
    int (*drv_ipc_handler)(struct drv_ipc_user_msg *msg);
    int (*drv_ipc_notifier)(struct notifier_block *nb, unsigned long len, void *data);
};
int drv_ipc_target_lp_handler(struct drv_ipc_user_msg *user_msg);
int drv_ipc_target_imu_handler(struct drv_ipc_user_msg *user_msg);
int drv_ipc_target_tsc_handler(struct drv_ipc_user_msg *user_msg);
int drv_ipc_target_tsv_handler(struct drv_ipc_user_msg *user_msg);
int drv_ipc_target_safetyisland_handler(struct drv_ipc_user_msg *user_msg);
int drv_ipc_notifier_handle(struct notifier_block *nb, unsigned long len, void *data);

#endif
