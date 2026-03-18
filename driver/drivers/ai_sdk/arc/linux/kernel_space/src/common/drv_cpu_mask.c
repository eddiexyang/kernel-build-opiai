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
* Create: 2022-12-25
*/
#include "drv_cpu_mask.h"

#include <linux/errno.h>
#include <linux/cpu.h>
#include <linux/fs.h>
#include <linux/bitmap.h>
#include <linux/version.h>
#include "drv_log.h"

#define MODULE_NAME "drv_comm"
#ifdef UT_VCAST
#define drv_comm_err(fmt, ...) drv_err(MODULE_NAME, fmt, ##__VA_ARGS__)
#else
#define drv_comm_err(fmt, ...) \
    drv_err(MODULE_NAME, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__)
#endif

#define CPU_INFO_SIZE (256)

int __attribute__((weak)) drv_get_ctrlcpu_mask_from_cpuset(cpumask_t *ctrl_cpumask)
{
    struct file *file = NULL;
    char buf[CPU_INFO_SIZE];
    loff_t pos = 0;
    int len, ret;

    file = filp_open("/sys/fs/cgroup/cpuset/CtrlCPU/cpuset.cpus", O_RDONLY, 0);
    if (IS_ERR(file)) {
        return -EEXIST;
    }

#ifdef AOS_LLVM_BUILD
    len = kernel_read(file, buf, CPU_INFO_SIZE - 1, &pos);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
    len = kernel_read(file, buf, CPU_INFO_SIZE - 1, &pos);
#else
    len = kernel_read(file, pos, buf, CPU_INFO_SIZE - 1);
#endif

    filp_close(file, NULL);
    file = NULL;

    if (len <= 0) {
        return -EFAULT;
    }

    ret = cpulist_parse((const char *)buf, ctrl_cpumask);
    if (ret) {
        drv_comm_err("fail to parse ctrl cpu list\n");
        return -EFAULT;
    }
    return 0;
}
