/*
* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
* Create: 2023-06-09
*/

#ifndef TRS_ABNORMAL_INFO_H
#define TRS_ABNORMAL_INFO_H

#include "icm_interface.h"

typedef enum abnormal_task_type {
    ABNORMAL_TASK_TYPE_AICPU,
    ABNORMAL_TASK_TYPE_DVPP,
    ABNORMAL_TASK_TYPE_AICORE,
    ABNORMAL_TASK_TYPE_UNKNOW,
    ABNORMAL_TASK_TYPE_MAX
}ABNORMAL_TASK_TYPE;

typedef enum abnormal_err_type {
    ABNORMAL_ERR_TYPE_TASK_TIMEOUT,
    ABNORMAL_ERR_TYPE_CQ_FULL,
    ABNORMAL_ERR_TYPE_MAX
}ABNORMAL_ERR_TYPE;


struct aicpu_task_info {
    u32 mb_bitmap;
    u32 dst_engine;
};

struct stars_abnormal_info {
    u32 vfid;
    u16 sqid;
    u16 sqe_id;  /* prepare for future */
    u16 task_id;
    u8 task_type;
    u8 err_type;
    union {
        struct aicpu_task_info aicpu_info;
        u8 abnormal_data[ICM_MSG_DATA_LENGTH - 12];
    };
};

typedef int (*tsmng_abnormal_proc_func)(u32 devid, u32 tsid, void *data);

int tsmng_register_abnormal_proc_func(tsmng_abnormal_proc_func func);
void tsmng_unregister_abnormal_proc_func(tsmng_abnormal_proc_func func);

#endif

