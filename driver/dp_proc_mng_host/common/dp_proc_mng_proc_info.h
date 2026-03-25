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
 * Create: 2022-08-13
 */
#ifndef __DP_PROC_MNG_PROC_INFO_H__
#define __DP_PROC_MNG_PROC_INFO_H__

#include <linux/types.h>
#include <linux/fs.h>
#include <linux/hashtable.h>

#include "dp_proc_mng_cmd.h"
#include "dp_proc_mng_log.h"
#include "ascend_hal_define.h"
#include "devdrv_manager_comm.h"

#ifndef __GFP_ACCOUNT
#ifdef __GFP_KMEMCG
#define GFP_ACCOUNT __GFP_KMEMCG /* for linux version 3.10 */
#endif

#ifdef __GFP_NOACCOUNT
#define GFP_ACCOUNT 0 /* for linux version 4.1 */
#endif
#else
#define GFP_ACCOUNT __GFP_ACCOUNT
#endif

#define DP_PROC_MNG_HASH_TABLE_BIT 10

#ifdef CFG_SOC_PLATFORM_MDC_V51
#define DP_PROC_MNG_MAX_NODE_NUM 1
#else
#define DP_PROC_MNG_MAX_NODE_NUM 4
#endif

#define RANDOM_SIZE          24

#define DP_PROC_MNG_MAX_SIGN_NUM  1000

#define DP_PROC_MNG_HASH_TABLE_BIT 10
#define DP_PROC_MNG_HASH_TABLE_SIZE (0x1 << DP_PROC_MNG_HASH_TABLE_BIT)
#define DP_PROC_MNG_HASH_TABLE_MASK (DP_PROC_MNG_HASH_TABLE_SIZE - 1)

struct dp_proc_mng_sign {
    pid_t hostpid;
    u32 docker_id;
    u32 host_process_status;
    pid_t devpid[DP_PROC_MNG_MAX_AGENT_NUM][DP_PROC_MNG_MAX_VF_NUM][PROCESS_CPTYPE_MAX];
    processStatus_t devpid_status[DP_PROC_MNG_MAX_AGENT_NUM][DP_PROC_MNG_MAX_VF_NUM][PROCESS_CPTYPE_MAX];
    char sign[PROCESS_SIGN_LENGTH];
    u32 cp_count; /* count cp1 & dev_only devpid num for releasing sign */
    u32 in_use_count;
    struct list_head list;
    struct hlist_node link; /* hash find link */
};

struct dp_proc_mng_info {
    struct list_head hostpid_list_header;   /* for hostpid check only in host */
    spinlock_t proc_hash_table_lock;
    DECLARE_HASHTABLE(proc_hash_table, DP_PROC_MNG_HASH_TABLE_BIT); /* a process can only run on one numa node */
    struct mutex dp_proc_mng_sign_list_lock; /* for hostpid check */
    u32 dp_proc_mng_sign_count[MAX_DOCKER_NUM + 1]; /* for hostpid check , 0~68 : container, 69 :non-container */
    struct list_head bind_cgroup_list;
    struct mutex bind_cgroup_list_lock;
    struct work_struct bind_cgroup_work;
    struct workqueue_struct *bind_cgroup_wq;
};

extern int (* const dp_proc_mng_ioctl_handlers[DP_PROC_MNG_CMD_MAX_CMD])(struct file *file,
    struct dp_proc_mng_ioctl_arg *arg);

int dp_proc_mng_davinci_module_init(const struct file_operations *ops);
void dp_proc_mng_davinci_module_uninit(void);

struct dp_proc_mng_info *dp_proc_get_manager_info(void);

int dp_proc_mng_info_init(void);
void dp_proc_mng_info_unint(void);

int dp_proc_mng_create_work(void);
void dp_proc_mng_destroy_work(void);

extern int devdrv_manager_container_logical_id_to_physical_id(u32 logical_dev_id, u32 *physical_dev_id, u32 *vfid);
extern int devdrv_manager_container_is_in_container(void);
extern int devdrv_manager_container_get_docker_id(u32 *docker_id);

#endif
