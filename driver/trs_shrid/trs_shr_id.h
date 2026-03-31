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
#ifndef TRS_SHR_ID_H
#define TRS_SHR_ID_H
#include "trs_pub_def.h"
#include "trs_shr_id_fops.h"
#include "trs_shr_id_ioctl.h"

int shr_id_create(struct shr_id_proc_ctx *proc_ctx, struct shr_id_ioctl_info *ioctl_info);
int shr_id_destroy(struct shr_id_proc_ctx *proc_ctx, struct shr_id_ioctl_info *ioctl_info);

int shr_id_open(struct shr_id_proc_ctx *proc_ctx, struct shr_id_ioctl_info *ioctl_info);
int shr_id_close(struct shr_id_proc_ctx *proc_ctx, struct shr_id_ioctl_info *ioctl_info);

int shr_id_set_pid(struct shr_id_proc_ctx *proc_ctx, struct shr_id_ioctl_info *ioctl_info);

int shr_id_proc_add(struct shr_id_proc_ctx *proc_ctx);
void shr_id_proc_del(struct shr_id_proc_ctx *proc_ctx);
int shr_id_wait_for_proc_exit(pid_t pid);
bool shr_id_is_belong_to_proc(struct trs_id_inst *inst, int pid, int res_type, u32 res_id);

struct shr_id_proc_ctx *shr_id_proc_create(pid_t pid);
void shr_id_proc_destroy(struct shr_id_proc_ctx *proc_ctx);

#endif /* TRS_SHR_ID_H */
