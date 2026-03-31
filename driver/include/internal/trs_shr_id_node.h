/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
 * Create: 2022-07-28
 */
#ifndef TRS_SHR_ID_NODE_H
#define TRS_SHR_ID_NODE_H

#include <linux/types.h>

#include "trs_shr_id_ioctl.h"
#include "trs_id.h"

struct shr_id_node_op_attr {
    struct trs_id_inst inst;
    int res_type;   /* trs id type */
    int type;       /* shr id type */
    int id;
};

void shr_id_node_init(void);
int shr_id_node_create(const char *name, pid_t pid, struct shr_id_node_op_attr *attr, u32 flag);
int shr_id_node_destroy(const char *name, int type, pid_t pid);
int shr_id_node_open(const char *name, pid_t pid, unsigned long start_time,
    struct shr_id_node_op_attr *attr);
int shr_id_node_close(const char *name, int type, pid_t pid);
int shr_id_node_set_pids(const char *name, int type, pid_t create_pid, pid_t pid[], u32 pid_num);

#endif
