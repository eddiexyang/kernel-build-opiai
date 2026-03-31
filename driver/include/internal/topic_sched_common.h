/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
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

#ifndef TOPIC_SCHED_COMMON_H
#define TOPIC_SCHED_COMMON_H

#include <stdbool.h>

void topic_sched_reset_pool_dev_cpu(const void __iomem *io_base, u32 pool_id);
void topic_sched_reset_pool_host_cpu(const void __iomem *io_base, u32 pool_id);
void topic_sched_pool_add_mb(const void __iomem *io_base, u32 pool_id, u32 mask);
void topic_sched_pool_del_mb(const void __iomem *io_base, u32 pool_id, u32 mask);
void topic_sched_add_host_pool(const void __iomem *io_base, u32 cpu_type, u32 pool_id, u64 mask);
void topic_sched_del_host_pool(const void __iomem *io_base, u32 cpu_type, u32 pool_id, u64 mask);

void topic_sched_get_wait_mb_free_status(const void __iomem *io_base, u32 *val);
void topic_sched_set_user_kernel_type(const void __iomem *io_base, u32 user_kernel_type);
int topic_sched_config_pid(const void __iomem *io_base, u32 identity, u32 host_pid, int pid_type, int pid);
void topic_sched_del_host_pid(const void __iomem *io_base, u32 identity, u32 host_pid, u32 side, u32 pid);
void topic_sched_set_cpu_port_head(const void __iomem *io_base, u32 port_id, u32 head);
void topic_sched_get_cpu_port_head(const void __iomem *io_base, u32 port_id, u32 *head);
void topic_sched_set_cpu_port_tail(const void __iomem *io_base, u32 port_id, u32 tail);
void topic_sched_init_cpu_port(const void __iomem *io_base, u32 port_id, u64 sq_base, u32 depth);
void topic_sched_uninit_cpu_port(const void __iomem *io_base, u32 port_id);
void topic_sched_enable_cpu_port(const void __iomem *io_base, u32 port_id);
void topic_sched_disable_cpu_port(const void __iomem *io_base, u32 port_id);
void topic_sched_clr_cpu_port_task(const void __iomem *io_base, u32 port_id, u32 mb_id, u32 tail);
void topic_sched_init_cpu_mailbox(const void __iomem *io_base, u32 mb_id, u64 mb_addr);
void topic_sched_init_aicpu_get_mailbox(const void __iomem *io_base, u32 mb_id, u64 mb_addr);

void topic_sched_cpu_status_report(const void __iomem *io_base, u32 mb_id, u32 status);
void topic_sched_cpu_errcode_report(const void __iomem *io_base, u32 mb_id, u32 error_code);
void topic_sched_cpu_get_status_report(const void __iomem *io_base, u32 mb_id, u32 status);
bool topic_sched_cpu_is_mb_valid(const void __iomem *io_base, u32 mb_id);
bool topic_sched_aicpu_is_get_mb_valid(const void __iomem *io_base, u32 mb_id);
void topic_sched_init_host_cpu_mailbox(const void __iomem *io_base, u64 mb_addr, u32 vf_id);

void topic_sched_mb_intr_clr(const void __iomem *io_base, u32 mb_id);

void topic_sched_conf_host_intr(const void __iomem *io_base, u32 int_value, u32 vf_id);

void topic_sched_cpu_intr_enable(const void __iomem *io_base, u32 mb_id);
void topic_sched_pool_add_aicpu_slot(const void __iomem *io_base, u32 pool_id, u32 mask);
void topic_sched_pool_del_aicpu_slot(const void __iomem *io_base, u32 pool_id, u32 mask);
void topic_sched_pool_add_ccpu_slot(const void __iomem *io_base, u32 pool_id, u32 mask);
void topic_sched_pool_del_ccpu_slot(const void __iomem *io_base, u32 pool_id, u32 mask);

#endif
