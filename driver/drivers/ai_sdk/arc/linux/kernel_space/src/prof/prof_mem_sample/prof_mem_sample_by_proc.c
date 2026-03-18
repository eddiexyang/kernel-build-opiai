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
 * Create: 2023-02-25
 */
#ifndef PROF_UNIT_TEST

#include <linux/securec.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/sched/mm.h>
#include <linux/sched/task.h>
#include <linux/timekeeping.h>

#include "kernel_version_adapt.h"
#include "chip_config.h"
#include "devdrv_interface.h"
#include "devdrv_manager_comm.h"

#include "drv_profile.h"
#include "prof_drv_dev.h"
#include "prof_mem_sample.h"

#define MEM_GET_FLAG_CGROUP_BIT       0
#define MEM_GET_FLAG_CDM_BIT          1

#define MEM_GET_FLAG_CGROUP_MASK      (0x1 << MEM_GET_FLAG_CGROUP_BIT)
#define MEM_GET_FLAG_CDM_MASK         (0x1 << MEM_GET_FLAG_CDM_BIT)

typedef int (*get_proc_meminfo)(u32, u32, int, u64 *);

static struct timespec64 g_start_timestamp[PROF_MEM_SAMPLE_MAX_DEV_NUM];

static get_proc_meminfo g_prof_get_proc_cdm_meminfo_func = NULL;
static get_proc_meminfo g_prof_get_proc_cgroup_meminfo_func = NULL;

static void prof_proc_mem_sample_func_init(void)
{
    if (g_prof_get_proc_cdm_meminfo_func == NULL) {
        g_prof_get_proc_cdm_meminfo_func = (get_proc_meminfo)(uintptr_t)
            __kallsyms_lookup_name("devmm_get_proc_cdm_meminfo");
    }
    if (g_prof_get_proc_cgroup_meminfo_func == NULL) {
        g_prof_get_proc_cgroup_meminfo_func = (get_proc_meminfo)(uintptr_t)
            __kallsyms_lookup_name("devmm_get_proc_cgroup_meminfo");
    }
}

static int prof_get_proc_cdm_meminfo(u32 devid, u32 vfid, int host_pid, u64 *used_size)
{
    *used_size = 0;
    if (g_prof_get_proc_cdm_meminfo_func != NULL) {
        return g_prof_get_proc_cdm_meminfo_func(devid, vfid, host_pid, used_size);
    }

    return 0;
}

static struct mm_struct *prof_mmget(int pid)
{
    struct task_struct *tsk = NULL;
    struct mm_struct *mm = NULL;
    struct pid *pro_id = NULL;

    pro_id = find_get_pid(pid);
    if (pro_id == NULL) {
        return NULL;
    }

    tsk = get_pid_task(pro_id, PIDTYPE_PID);
    if (tsk == NULL) {
        goto out_put_pid;
    }

    mm = get_task_mm(tsk);
    if (mm == NULL) {
        goto out_put_task;
    }

out_put_task:
    put_task_struct(tsk);
out_put_pid:
    put_pid(pro_id);

    return mm;
}

static int _prof_get_proc_rss_meminfo(int pid, u64 *used_size)
{
    struct mm_struct *mm = NULL;

    mm = prof_mmget(pid);
    if (mm == NULL) {
        prof_err("mmget failed. (pid=%d)\n", pid);
        return -1;
    }

    *used_size = get_mm_rss(mm) * PAGE_SIZE;
    mmput(mm);
    return 0;
}

static int prof_get_proc_rss_meminfo(u32 devid, u32 vfid, int host_pid, u64 *used_size)
{
    int ret, devpid;

    *used_size = 0;
    ret = devdrv_query_process_by_host_pid(host_pid, devid, DEVDRV_PROCESS_CP1, vfid, &devpid);
    if (ret != 0) {
        prof_err("Get device pid failed. (target_pid=%d; devid=%u; vfid=%u)\n",
            host_pid, devid, vfid);
        return -1;
    }

    return _prof_get_proc_rss_meminfo(devpid, used_size);
}

static int prof_get_proc_cgroup_meminfo(u32 devid, u32 vfid, int host_pid, u64 *used_size)
{
    *used_size = 0;
    if (g_prof_get_proc_cgroup_meminfo_func != NULL) {
        return g_prof_get_proc_cgroup_meminfo_func(devid, vfid, host_pid, used_size);
    } else {
        return prof_get_proc_rss_meminfo(devid, vfid, host_pid, used_size);
    }
}

static int g_mem_get_flag_of_proc[HISI_CHIP_NUM][DBL_MEMTYPE_NUM] = {
    [HISI_MINI_V1] = {
        [DBL_MEMTYPE_DDR] = MEM_GET_FLAG_CGROUP_MASK | MEM_GET_FLAG_CDM_MASK,
        [DBL_MEMTYPE_HBM] = 0
    },
    [HISI_MINI_V2] = {
        [DBL_MEMTYPE_DDR] = MEM_GET_FLAG_CGROUP_MASK | MEM_GET_FLAG_CDM_MASK,
        [DBL_MEMTYPE_HBM] = 0
    },
    [HISI_MINI_V3] = {
        [DBL_MEMTYPE_DDR] = MEM_GET_FLAG_CGROUP_MASK | MEM_GET_FLAG_CDM_MASK,
        [DBL_MEMTYPE_HBM] = 0
    },
    [HISI_CLOUD_V1] = {
        [DBL_MEMTYPE_DDR] = MEM_GET_FLAG_CGROUP_MASK,
        [DBL_MEMTYPE_HBM] = MEM_GET_FLAG_CDM_MASK
    },
    [HISI_CLOUD_V2] = {
        [DBL_MEMTYPE_DDR] = 0,
        [DBL_MEMTYPE_HBM] = MEM_GET_FLAG_CGROUP_MASK | MEM_GET_FLAG_CDM_MASK
    }
};

static int prof_mem_type_to_mem_get_flag(int mem_type, int *flag)
{
    u32 chip_type;
    chip_type = uda_get_chip_type(0);
    if (chip_type >= HISI_CHIP_NUM) {
        return -ENODEV;
    }

    *flag = g_mem_get_flag_of_proc[chip_type][mem_type];
    return 0;
}

static int _prof_get_proc_meminfo(u32 devid, u32 vfid, int host_pid, int flag, u64 *used_size)
{
    u64 cgroup_used_size = 0;
    u64 cdm_used_size = 0;
    int ret;

    if ((flag & MEM_GET_FLAG_CGROUP_MASK) != 0) {
        ret = prof_get_proc_cgroup_meminfo(devid, vfid, host_pid, &cgroup_used_size);
        if (ret != 0) {
            return ret;
        }
    }

    if ((flag & MEM_GET_FLAG_CDM_MASK) != 0) {
        ret = prof_get_proc_cdm_meminfo(devid, vfid, host_pid, &cdm_used_size);
        if (ret != 0) {
            return ret;
        }
    }

    *used_size = cgroup_used_size + cdm_used_size;
    return 0;
}

static int prof_get_proc_meminfo(u32 devid, u32 vfid, int host_pid, int mem_type, u64 *used_size)
{
    int mem_get_flag, ret;

    ret = prof_mem_type_to_mem_get_flag(mem_type, &mem_get_flag);
    if (ret != 0) {
        return ret;
    }

    return _prof_get_proc_meminfo(devid, vfid, host_pid, mem_get_flag, used_size);
}

static int prof_proc_mem_sample_para_check(struct prof_peri_para *para)
{
    if (para->buff == NULL) {
        prof_err("Buff is NULL.\n");
        return -EINVAL;
    }

    if (para->buff_len < sizeof(struct prof_mem_sample_data)) {
        prof_err("Buff_len is invalid. (buff_len=%u)\n", para->buff_len);
        return -EINVAL;
    }

    if (para->device_id >= PROF_MEM_SAMPLE_MAX_DEV_NUM) {
        prof_err("Devid is out of range. (devid=%u)\n", para->device_id);
        return -EINVAL;
    }

    if (para->vfid >= PROF_MEM_SAMPLE_MAX_VF_NUM) {
        prof_err("Vfid is out of range. (devid=%u)\n", para->vfid);
        return -EINVAL;
    }
    return 0;
}

static u64 prof_get_proc_mem_sample_timestamp(u32 devid)
{
    struct timespec64 timestamp;

    ktime_get_raw_ts64(&timestamp);
    return prof_get_time_interval(&g_start_timestamp[devid], &timestamp);
}

int prof_sample_proc_mem_used_info(struct prof_peri_para *para)
{
    struct prof_mem_sample_data *data = (struct prof_mem_sample_data *)para->buff;
    u64 ddr_used_size, hbm_used_size;
    int ret;

    ret = prof_proc_mem_sample_para_check(para);
    if (ret != 0) {
        return ret;
    }

    ret = prof_get_proc_meminfo(para->device_id, para->vfid, para->target_pid, DBL_MEMTYPE_DDR, &ddr_used_size);
    if (ret != 0) {
        prof_err("Get proc ddr meminfo failed. (devid=%u; vfid=%u; target_pid=%d; ret=%d)",
            para->device_id, para->vfid, para->target_pid, ret);
        return ret;
    }

    ret = prof_get_proc_meminfo(para->device_id, para->vfid, para->target_pid, DBL_MEMTYPE_HBM, &hbm_used_size);
    if (ret != 0) {
        prof_err("Get proc hbm meminfo failed. (devid=%u; vfid=%u; target_pid=%d; ret=%d)",
            para->device_id, para->vfid, para->target_pid, ret);
        return ret;
    }

    data->timestamp = (u32)prof_get_proc_mem_sample_timestamp(para->device_id);
    data->event = PROF_MEM_SAMPLE_PROC_MODE;
    data->ddr_used_size = ddr_used_size;
    data->hbm_used_size = hbm_used_size;

    return sizeof(struct prof_mem_sample_data);
}
EXPORT_SYMBOL(prof_sample_proc_mem_used_info);

int prof_proc_mem_sample_init(struct prof_peri_para *para)
{
    if (para->device_id >= PROF_MEM_SAMPLE_MAX_DEV_NUM) {
        prof_err("Devid is out of range. (devid=%u)\n", para->device_id);
        return -EINVAL;
    }

    prof_proc_mem_sample_func_init();
    ktime_get_raw_ts64(&g_start_timestamp[para->device_id]);
    prof_debug("Proc mem sample init. (dev_id=%u)\n", para->device_id);
    return 0;
}
EXPORT_SYMBOL(prof_proc_mem_sample_init);
#else
void prof_mem_sample_by_proc_ut_test(void)
{
}
#endif
