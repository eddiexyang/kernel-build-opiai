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
 * Create: 2022-11-9
 */

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/gfp.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>
#include <asm/page.h>
#include <linux/time64.h>
#include <linux/version.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <asm/atomic.h>
#ifdef AOS_LLVM_BUILD
#include <linux/share_pool.h>
#endif

#include "tsdrv_bind_stl.h"
#include "tsdrv_ioctl.h"
#include "tsdrv_ctx.h"
#include "aicpu.h"
#include "tsdrv_device.h"
#include "tsdrv_drvops.h"
#include "tsdrv_get_ssid.h"
#include "tsdrv_common.h"

static atomic_t g_tsdrv_stl_flag = ATOMIC_INIT(0);
STATIC bool tsdrv_stl_flag_chk(void)
{
    int val = atomic_cmpxchg(&g_tsdrv_stl_flag, 0, 1);
    return val == 0;
}

static void tsdrv_set_current_time(void)
{
    struct aicpu_system_config *config = NULL;
    struct timespec64 wall64;

    config = (struct aicpu_system_config *)ioremap(SYSTEM_CONFIG_BASE, SYSTEM_CONFIG_SIZE);
    if (config == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("ioremap_cache failed.\n");
        return;
#endif
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
    ktime_get_coarse_real_ts64(&wall64);
#else
    wall64 = current_kernel_time64();
#endif
    config->boot_time.sec = (u64)wall64.tv_sec;
    config->boot_time.nsec = (u64)wall64.tv_nsec;

    iounmap(config);
}

#ifndef AOS_LLVM_BUILD
STATIC unsigned long tsdrv_map_stl_va(unsigned long pa, unsigned long len)
{
#ifndef TSDRV_UT
    unsigned long stl_va;
    struct vm_area_struct *vma = NULL;
    int ret;
    stl_va = vm_mmap(0, 0, len, PROT_READ | PROT_WRITE, MAP_SHARED, 0);
    if (IS_ERR_VALUE(stl_va)) {
        TSDRV_PRINT_ERR("vm_mmap fail, (err=%d).\n", (int)stl_va);
        return 0;
    };

    down_write(get_mmap_sem(current->mm));
    vma = find_vma(current->mm, stl_va);
    if (vma == NULL) {
        up_write(get_mmap_sem(current->mm));
        vm_munmap(stl_va, len);
        stl_va = 0;
        TSDRV_PRINT_ERR("vma is null.\n");
        return 0;
    }

    /* clean PTE_RDONLY flags or trigger SMMU event */
    vma->vm_page_prot = __pgprot(((~PTE_RDONLY) & vma->vm_page_prot.pgprot) | PTE_DIRTY);

    ret = remap_pfn_range(vma, stl_va, __phys_to_pfn(pa), len, pgprot_device(vma->vm_page_prot));
    if (ret != 0) {
        up_write(get_mmap_sem(current->mm));
        vm_munmap(stl_va, len);
        stl_va = 0;
        TSDRV_PRINT_ERR("Remap pfn fail. (ret=%d)\n", ret);
        return 0;
    }

    up_write(get_mmap_sem(current->mm));
    TSDRV_PRINT_INFO("remap stl va success.\n");
    return stl_va;
#endif
}
#else
STATIC unsigned long tsdrv_map_stl_va(unsigned long pa, unsigned long len)
{
    unsigned long stl_va;
    void *vaddr = NULL;
    void *ptr;

    vaddr = (void *)ioremap(pa, len);
    if (vaddr == NULL) {
        TSDRV_PRINT_ERR("iomem fail.\n");
        return 0;
    }

    ptr = mg_sp_make_share_k2u((unsigned long)(uintptr_t)vaddr, len, SP_PFN_MAP, 0, SPG_ID_DEFAULT);
    if (IS_ERR(ptr)) {
        TSDRV_PRINT_ERR("mg_sp_make_share_k2u fail ret = %d.\n", PTR_ERR(ptr));
        iounmap(vaddr);
        vaddr = 0;
        return 0;
    }
    stl_va = (unsigned long)(uintptr_t)ptr;
    TSDRV_PRINT_INFO("remap stl va success.\n");
    return stl_va;
}
#endif
STATIC void __iomem *tsdrv_remap_aicpu_system_addr(u32 devid, u32 tsid)
{
    void __iomem *vaddr = NULL;
    vaddr = ioremap(SYSTEM_CONFIG_BASE + ((phys_addr_t)tsid * (phys_addr_t)SYSTEM_CONFIG_SIZE) +
                                            (CHIP_BASEADDR_PA_OFFSET * devid),
                                            SYSTEM_CONFIG_SIZE);
    if (vaddr == NULL) {
        TSDRV_PRINT_ERR("Ioremap aicpu system config failed. (devid=%u; tsid=%u)\n",
            devid, tsid);
        return NULL;
    }

    return vaddr;
}

STATIC int tsdrv_cfg_ts_stl(struct tsdrv_ctx *ctx, u32 tsid, u32 devid)
{
    int ssid;
    unsigned long stl_va;
    struct aicpu_system_config *config = NULL;
    void __iomem *vaddr = NULL;

    ssid = tsdrv_msg_sync_ssid(devid, tsid, (u32)ctx->tgid, 0);
    if (ssid == TSDRV_INVALID_SSID) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Get ssid fail, (dev=%u, tsid=%u)\n", devid, tsid);
        return -EINVAL;
#endif
    }

    vaddr = tsdrv_remap_aicpu_system_addr(devid, tsid);
    if (vaddr == NULL) {
        return -EINVAL;
    }
    config = (struct aicpu_system_config *)((uintptr_t)vaddr);
    if (config->stl_va != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("The stl va has been configured. (devid=%u; tsid=%u)\n", devid, tsid);
        iounmap(config);
        config = NULL;
#endif
        return -EEXIST;
    }

    if (tsid == 0) {
        stl_va = tsdrv_map_stl_va(TS_MDC_AIC_STL_BASE, TS_MDC_AIC_STL_SIZE);
    } else {
        stl_va = tsdrv_map_stl_va(TS_MDC_AIV_STL_BASE, TS_MDC_AIV_STL_SIZE);
    }
    if (stl_va == 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("map stl va fail. (devid=%u; tsid=%u).\n", devid, tsid);
        iounmap(config);
        config = NULL;
#endif
        return -ENOMEM;
    }
    config->stl_ssid = ssid;
    config->stl_va = stl_va;

    if (tsdrv_get_drv_ops()->flush_cache != NULL) {
        tsdrv_get_drv_ops()->flush_cache((u64)((uintptr_t)config), (u32)SYSTEM_CONFIG_SIZE);
    }
    iounmap(config);
    config = NULL;
    tsdrv_set_current_time();

    TSDRV_PRINT_INFO("Bind stl operator is complete. (devid=%u, tsid=%u, ssid=%d, va=%pK).\n",
        devid, tsid, ssid, (void *)(uintptr_t)stl_va);
    return 0;
}

int tsdrv_ioctl_bind_stl_operator(struct tsdrv_ctx *ctx, struct devdrv_ioctl_arg *arg)
{
    u32 tsid;
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
    u32 tsnum;
    int ret;
    struct tsdrv_device *tsdrv_dev = tsdrv_get_dev(devid);
    u32 disable_ts_num = 0;

    if (tsdrv_get_chip_type() != CHIP_TYPE_MDC_BS9SX1A) {
        TSDRV_PRINT_INFO("The current chip type does not support binding to the STL operator.\n");
        arg->result = BIND_STL_NOT_SUPPORT;
        return -EPERM;
    }

    if (!tsdrv_stl_flag_chk()) {
        TSDRV_PRINT_WARN("Duplicate binding STL is not supported, (devid=%d).\n", devid);
        return -EBUSY;
    }

    tsnum = tsdrv_get_dev_tsnum(devid);
    for (tsid = 0; tsid < tsnum; tsid++) {
        if (tsdrv_dev->stl_enable_flag[tsid] == 0) {
            disable_ts_num++;
            TSDRV_PRINT_INFO("STL operator binding is not supported. (tsid=%u, disable_ts_num=%u)\n",
                tsid, disable_ts_num);
            continue;
        }

        ret = tsdrv_cfg_ts_stl(ctx, tsid, devid);
        if (ret != 0) {
            TSDRV_PRINT_ERR("Config ts stl fail. (tsid=%u)\n", tsid);
            return ret;
        }
    }
    if (disable_ts_num == tsnum) {
#ifndef TSDRV_UT
        TSDRV_PRINT_INFO("No ts support binding to the STL operator.\n");
        arg->result = BIND_STL_NOT_SUPPORT;
#endif
        return -EPERM;
    }

    return 0;
}
