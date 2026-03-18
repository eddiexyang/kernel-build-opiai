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

#ifndef HWTS_UT_TEST
#ifndef AOS_LLVM_BUILD
#include <linux/irq.h>
#include <linux/irqchip/arm-gic-v3.h>
#endif
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/cpu.h>
#include <linux/msi.h>
#include <linux/kthread.h>
#include <linux/of_irq.h>
#include <linux/kernel.h>
#include <linux/notifier.h>

#ifndef COMPILE_WITH_UT
#include <linux/acpi.h>
#include <linux/property.h>
#endif

#include "dbl/chip_config.h"
#include "dbl/uda.h"
#include "devdrv_pm.h"
#include "devdrv_manager_common.h"
#include "devdrv_platform.h"
#include "devdrv_parse_pdata.h"
#include "devdrv_manager.h"
#include "tsdrv_common.h"
#include "tsdrv_log.h"
#include "tsdrv_pm.h"
#include "tsdrv_platform_interface.h"
#include "tsdrv_intr.h"
#include "tsdrv_pdata.h"
#include "tsdrv_ipc.h"
#include "tsdrv_ipc_dfx.h"
#include "tsdrv_ts_alloc_mem.h"
#include "tsdrv_alloc_mem_dfx.h"
#include "tsdrv_fault_init.h"
#include "tsdrv_device.h"
#include "tsdrv_firmware_load.h"
#include "tsdrv_parse.h"
#include "tsdrv_heartbeat.h"
#include "tsdrv_sys_panic.h"
#include "tsdrv_common_fault_platform.h"
#ifdef CFG_FEATURE_HARDWARE_SCHED
#include "event_sched_inner.h"
#endif

#ifdef CFG_FEATURE_PG
#include "hiss/hsm_info.h"
#include "ascend_platform.h"
#endif
#ifdef CFG_SOC_PLATFORM_MDC_V51
#include "drv_snapshot.h"
#include "tsdrv_ts_node.h"
#endif
#ifdef CFG_FEATURE_QOS
#include "ascend_kernel_hal.h"
#include "tsdrv_qos.h"
#endif

#ifdef CFG_TRS_REFACTOR_FEATURE
#include "trs_device.h"
#include "trs_stars_comm.h"
#endif

#ifndef CFG_TRS_REFACTOR_FEATURE
#if defined(CFG_SOC_PLATFORM_CLOUD_V2) || defined(CFG_SOC_PLATFORM_MINIV3)
#include "tsdrv_cdqm_dev.h"
#endif
#include "tsdrv_d2h_chan.h"
#include "devdrv_devinit.h"
#include "tsdrv_sync.h"
#endif

#ifdef AOS_LLVM_BUILD
#include "tsdrv_drvops.h"
#endif

#ifdef CFG_FEATURE_PG
#define HSM_THREAD_RUN 1
#define HSM_THREAD_STOP 0
#define TSDRV_HSM_TASK_WAIT_TIME_MS 10
STATIC atomic_t g_pg_info_task_state[DEVDRV_MAX_DAVINCI_NUM] = {0};

struct devdrv_pg_info_task {
    u32 dev_id;
    u32 multi_die;
    pg_cmd_data cmd;
    struct devdrv_pg_info *pg_info;
};
#endif

STATIC struct devdrv_client_info dev_platform_client_info[MAX_CHIP_NUM];
u32 tscpu_start_irq[MAX_CHIP_NUM] = {0, };
u32 tscpu_irq_num[MAX_CHIP_NUM] = {0, };

u32 soc_platform_get_chip_type(void);

/* when set tscpu/ipc irq affinity need adapt for diff os */
int devdrv_set_irq_affinity(unsigned int irq, const struct cpumask *cpumask)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)) && !defined(AOS_LLVM_BUILD)
    if (cpumask != NULL) {
        (void)irq_force_affinity(irq, cpumask);
    }
#else
    (void)irq_set_affinity_hint(irq, cpumask);
#endif
    return 0;
}

#ifdef CFG_SOC_PLATFORM_MDC_V11
// 1910B temporarily resolve dependencies
int vmngd_set_pg_info(unsigned int dev_id, vmng_common_pg *pg_info)
{
    return 0;
}
#endif

int devdrv_get_tscpu_irq_info(u32 dev_id, u32 *start_irq, u32 *irq_num)
{
    if (dev_id >= MAX_CHIP_NUM) {
        TSDRV_PRINT_ERR("dev_id %u error.\n", dev_id);
        return -EINVAL;
    }

    if ((start_irq == NULL) || (irq_num == NULL)) {
        TSDRV_PRINT_ERR("dev_id %u null ptr.\n", dev_id);
        return -EINVAL;
    }

    *start_irq = tscpu_start_irq[dev_id];
    *irq_num = tscpu_irq_num[dev_id];

    return 0;
}
EXPORT_SYMBOL(devdrv_get_tscpu_irq_info);

#ifndef TSDRV_UT
#if defined(CFG_SOC_PLATFORM_CLOUD_V2) || defined(CFG_SOC_PLATFORM_MINIV3)
int devdrv_get_stars_irq_base(u32 dev_id, int *irq)
{
    struct devdrv_info *dev_info = NULL;

    if (dev_id >= MAX_CHIP_NUM) {
        TSDRV_PRINT_ERR("dev_id %u error.\n", dev_id);
        return -EINVAL;
    }

    dev_info = (struct devdrv_info *)dev_platform_client_info[dev_id].priv;
    if (dev_info == NULL) {
        TSDRV_PRINT_ERR("dev_id %u not init.\n", dev_id);
        return -EINVAL;
    }

    *irq = dev_info->pdata->ts_pdata[0].irq_base;
    return 0;
}
EXPORT_SYMBOL(devdrv_get_stars_irq_base);

#ifndef CFG_TRS_REFACTOR_FEATURE
void devdrv_set_stars_irq_base(void)
{
    struct devdrv_info *dev_info;
    int i;

    for (i = 0; i < MAX_CHIP_NUM; ++i) {
        dev_info = NULL;
        dev_info = (struct devdrv_info *)dev_platform_client_info[i].priv;
        if (dev_info == NULL) {
            continue;
        }
        tsdrv_cdqm_set_stars_irq_base(i, dev_info->pdata->ts_pdata[0].irq_base);
    }
}
#endif

int devdrv_get_stars_reg_base(u32 dev_id, u64 *base_addr)
{
    u32 phy_devid;
#ifdef CFG_SOC_PLATFORM_CLOUD_V2
    u32 vfid;
    int ret;
#endif

    if (base_addr == NULL) {
        TSDRV_PRINT_ERR("dev_id %u null ptr.\n", dev_id);
        return -EINVAL;
    }

#ifdef CFG_SOC_PLATFORM_CLOUD_V2
    ret = devdrv_get_pfvf_id_by_devid(dev_id, &phy_devid, &vfid);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to get phy_devid and vfid. (devid=%u).\n", dev_id);
        return ret;
    }
#else
    phy_devid = dev_id;
#endif

    *base_addr = devdrv_get_addr_base(DEVDRV_DTS_STARS_INDEX, phy_devid);
    if (*base_addr == 0) {
        TSDRV_PRINT_ERR("Failed to get stars base address. (devid=%u; phy_devid=%u)\n", dev_id, phy_devid);
        return -EINVAL;
    }

    return 0;
}
EXPORT_SYMBOL(devdrv_get_stars_reg_base);
#endif
#endif /* TSDRV_UT */

STATIC int devdrv_device_suspend(struct platform_device *pdev, pm_message_t state)
{
    struct devdrv_info *dev_info = NULL;
    int ret;

    TSDRV_BOOTDOT_INIT(SNAPSHOT_STATUS_SUSPEND, TS_PLATFORM_SUSPEND_EXPECT);
    if (pdev == NULL) {
        TSDRV_PRINT_ERR("pdev is NULL\n");
        return -EINVAL;
    }

    dev_info = platform_get_drvdata(pdev);

#ifndef AOS_LLVM_BUILD
    TSDRV_BOOTDOT(TS_PLATFORM_SUSPEND_DEVMNG);
    ret = devdrv_manager_suspend(dev_info);
    if (ret != 0) {
        TSDRV_PRINT_ERR("devdrv platform suspend failed. ret=%d\n", ret);
        return ret;
    }
#endif
#ifdef CFG_SOC_PLATFORM_MDC_V51
    if (devdrv_get_ts_node_num() == 0) {
        TSDRV_PRINT_INFO("ts does not exist, return.\n");
        TSDRV_BOOTDOT(TS_PLATFORM_SUSPEND_EXPECT);
        return 0;
    }
#endif

#ifndef TSDRV_UT
    ret = uda_dev_ctrl(dev_info->dev_id, UDA_CTRL_SUSPEND);
    if (ret != 0) {
#ifndef AOS_LLVM_BUILD
        (void)devdrv_manager_resume(dev_info);
#endif
        (void)tsdrv_pm_resume(dev_info->dev_id);
        TSDRV_PRINT_ERR("Suspend dev failed. ret=%d\n", ret);
        return ret;
    }
#endif

    TSDRV_BOOTDOT(TS_PLATFORM_SUSPEND_PM);
    ret = tsdrv_pm_suspend(dev_info->dev_id);
    if (ret != 0) {
        TSDRV_PRINT_ERR("ts drv suspend failed. ret=%d\n", ret);
#ifndef TSDRV_UT
        (void)uda_dev_ctrl(dev_info->dev_id, UDA_CTRL_RESUME);
#endif
#ifndef AOS_LLVM_BUILD
        (void)devdrv_manager_resume(dev_info);
#endif
        /* When the sleep fails, the TS that has been successfully sleep
         * should be restored to the wakeup state */
        (void)tsdrv_pm_resume(dev_info->dev_id);
        return ret;
    }

    TSDRV_BOOTDOT(TS_PLATFORM_SUSPEND_EXPECT);
    return 0;
}

STATIC int devdrv_device_resume(struct platform_device *pdev)
{
    struct devdrv_info *dev_info = NULL;
    int ret;

    TSDRV_BOOTDOT_INIT(SNAPSHOT_STATUS_RESUME, TS_PLATFORM_RESUME_EXPECT);
    if (pdev == NULL) {
        TSDRV_PRINT_ERR("pdev is NULL\n");
        return -EINVAL;
    }

    dev_info = platform_get_drvdata(pdev);
#ifndef AOS_LLVM_BUILD
    TSDRV_BOOTDOT(TS_PLATFORM_RESUME_DEVMNG);
    ret = devdrv_manager_resume(dev_info);
    if (ret != 0) {
        TSDRV_PRINT_ERR("devdrv platform resume failed. ret=%d\n", ret);
        return ret;
    }
#endif
#ifdef CFG_SOC_PLATFORM_MDC_V51
    if (devdrv_get_ts_node_num() == 0) {
        TSDRV_PRINT_INFO("ts does not exist, return.\n");
        TSDRV_BOOTDOT(TS_PLATFORM_RESUME_EXPECT);
        return 0;
    }
#endif
    TSDRV_BOOTDOT(TS_PLATFORM_RESUME_PM);
    ret = tsdrv_pm_resume(dev_info->dev_id);
    if (ret != 0) {
        TSDRV_PRINT_ERR("ts drv resume failed. ret=%d\n", ret);
#ifndef AOS_LLVM_BUILD
        (void)devdrv_manager_suspend(dev_info);
#endif
        return ret;
    }

    TSDRV_BOOTDOT(TS_PLATFORM_RESUME_EXPECT);

#ifndef TSDRV_UT
    ret = uda_dev_ctrl(dev_info->dev_id, UDA_CTRL_RESUME);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Resume dev failed. ret=%d\n", ret);
#ifndef AOS_LLVM_BUILD
        (void)devdrv_manager_suspend(dev_info);
#endif
        return ret;
    }
#endif

    return 0;
}

u64 devdrv_get_addr_base(enum devdrv_dts_addr_index idx, u32 chip_id)
{
    struct devdrv_platform_data *pdata = NULL;
    struct devdrv_info *dev_info = NULL;

    if ((idx >= DEVDRV_DTS_MAX_RESOURCE_NODE) || (chip_id >= MAX_CHIP_NUM)) {
        return 0;
    } else {
        dev_info = dev_platform_client_info[chip_id].priv;
        pdata = dev_info->pdata;
        return pdata->platform_info.devdrv_addr_base[idx];
    }
}
EXPORT_SYMBOL(devdrv_get_addr_base);

STATIC void devdrv_devinfo_destroy(struct devdrv_info *dev_info)
{
    mutex_destroy(&dev_info->container.lock);
    mutex_destroy(&dev_info->lock);
    kfree(dev_info);
    dev_info = NULL;
}

#ifdef CFG_FEATURE_PG
STATIC void devdrv_set_hardware_version_by_soc_version(struct devdrv_info *dev_info, u32 *hardware_version)
{
#ifdef CFG_SOC_PLATFORM_MINIV3
#ifndef CFG_SOC_PLATFORM_MDC_V11
    if (strstr(dev_info->pg_info.spePgInfo.socVersion, "B1")) {
        *hardware_version = SOC_HARDWARE_VERSION_ASCEND310B1;
    } else if (strstr(dev_info->pg_info.spePgInfo.socVersion, "B2")) {
        *hardware_version = SOC_HARDWARE_VERSION_ASCEND310B2;
    } else if (strstr(dev_info->pg_info.spePgInfo.socVersion, "B3")) {
        *hardware_version = SOC_HARDWARE_VERSION_ASCEND310B3;
    } else if (strstr(dev_info->pg_info.spePgInfo.socVersion, "B4")) {
        *hardware_version = SOC_HARDWARE_VERSION_ASCEND310B4;
    } else {
        *hardware_version = SOC_HARDWARE_VERSION;
    }
#else
    *hardware_version = SOC_HARDWARE_VERSION_AS31XM1;
    TSDRV_PRINT_INFO("AS31XM1 hardware version show. (hardware_version_16=0x%08x)\n", *hardware_version);
#endif
#else
    if (strstr(dev_info->pg_info.spePgInfo.socVersion, "B1")) {
        *hardware_version = SOC_HARDWARE_VERSION_ASCEND910B1;
    } else if (strstr(dev_info->pg_info.spePgInfo.socVersion, "B2C")) {
        *hardware_version = SOC_HARDWARE_VERSION_ASCEND910B2C;
    } else if (strstr(dev_info->pg_info.spePgInfo.socVersion, "B2")) {
        *hardware_version = SOC_HARDWARE_VERSION_ASCEND910B2;
    } else if (strstr(dev_info->pg_info.spePgInfo.socVersion, "B3")) {
        *hardware_version = SOC_HARDWARE_VERSION_ASCEND910B3;
    } else if (strstr(dev_info->pg_info.spePgInfo.socVersion, "B4")) {
        *hardware_version = SOC_HARDWARE_VERSION_ASCEND910B4;
    } else if (strstr(dev_info->pg_info.spePgInfo.socVersion, "C1")) {
        *hardware_version = SOC_HARDWARE_VERSION_ASCEND910C1;
    } else if (strstr(dev_info->pg_info.spePgInfo.socVersion, "C2")) {
        *hardware_version = SOC_HARDWARE_VERSION_ASCEND910C2;
    } else if (strstr(dev_info->pg_info.spePgInfo.socVersion, "C3")) {
        *hardware_version = SOC_HARDWARE_VERSION_ASCEND910C3;
    } else {
        if (dev_info->multi_die == 1) {
            *hardware_version = SOC_HARDWARE_VERSION_ASCEND910C3;
        } else {
            *hardware_version = SOC_HARDWARE_VERSION_ASCEND910B4;
        }
    }
#endif
}

STATIC void devdrv_set_pg_info_min_spec(struct devdrv_pg_info *pg_info, u32 multi_die)
{
    pg_info->comPgInfo.aicPara.freq = SOC_DEFAULT_AICORE_FREQ;
    pg_info->comPgInfo.aicPara.totalNum = SOC_DEFAULT_AICORE_TOTAL_NUM;
    pg_info->comPgInfo.aicPara.minNum = SOC_DEFAULT_AICORE_MIN_NUM;
    pg_info->comPgInfo.aicPara.bitMap = SOC_DEFAULT_AICORE_BITMAP;
    pg_info->comPgInfo.aivPara.freq = SOC_DEFAULT_AIVECTOR_FREQ;
    pg_info->comPgInfo.aivPara.totalNum = SOC_DEFAULT_AIVECTOR_TOTAL_NUM;
    pg_info->comPgInfo.aivPara.minNum = SOC_DEFAULT_AIVECTOR_MIN_NUM;
    pg_info->comPgInfo.aivPara.bitMap = SOC_DEFAULT_AIVECTOR_BITMAP;
    pg_info->comPgInfo.vpcPara.totalNum = SOC_DEFAULT_VPC_TOTAL_NUM;
    pg_info->comPgInfo.vpcPara.minNum = SOC_DEFAULT_VPC_MIN_NUM;
    pg_info->comPgInfo.vpcPara.bitMap = SOC_DEFAULT_VPC_BITMAP;
    pg_info->comPgInfo.jpegdPara.totalNum = SOC_DEFAULT_JPEGD_TOTAL_NUM;
    pg_info->comPgInfo.jpegdPara.minNum = SOC_DEFAULT_JPEGD_MIN_NUM;
    pg_info->comPgInfo.jpegdPara.bitMap = SOC_DEFAULT_JPEGD_BITMAP;
    pg_info->comPgInfo.cpuPara.totalNum = SOC_DEFAULT_CPU_TOTAL_NUM;
    pg_info->comPgInfo.cpuPara.minNum = SOC_DEFAULT_CPU_MIN_NUM;
    pg_info->comPgInfo.cpuPara.bitMap = SOC_DEFAULT_CPU_BITMAP;
    pg_info->comPgInfo.hbmPara.freq = SOC_DEFAULT_HBM_FREQ;
    pg_info->comPgInfo.hbmPara.totalNum = SOC_DEFAULT_HBM_TOTAL_NUM;
    pg_info->comPgInfo.hbmPara.minNum = SOC_DEFAULT_HBM_MIN_NUM;
    pg_info->comPgInfo.hbmPara.bitMap = SOC_DEFAULT_HBM_BITMAP;
    pg_info->comPgInfo.l2Para.freq = SOC_DEFAULT_L2_FREQ;
    pg_info->comPgInfo.l2Para.totalNum = SOC_DEFAULT_L2_TOTAL_NUM;
    pg_info->comPgInfo.l2Para.minNum = SOC_DEFAULT_L2_MIN_NUM;
    pg_info->comPgInfo.l2Para.bitMap = SOC_DEFAULT_L2_BITMAP;
    pg_info->comPgInfo.mataPara.minNum = SOC_DEFAULT_MATA_MIN_NUM;

    if (multi_die == 1) {
        (void)strcpy_s(pg_info->spePgInfo.socVersion, MAX_CHIP_NAME, SOC_DEFAULT_SPEC_SINGLE_DIE);
    } else {
        (void)strcpy_s(pg_info->spePgInfo.socVersion, MAX_CHIP_NAME, SOC_DEFAULT_SPEC_MULTI_DIE);
    }

    TSDRV_PRINT_WARN("Set minimum specification for pg info.\n");
    return;
}

STATIC int devdrv_get_pg_info_thread(void *arg)
{
    int ret;
    struct devdrv_pg_info_task *task = (struct devdrv_pg_info_task *)arg;

    TSDRV_PRINT_INFO("Get pg info thread in.\n");
    ret = sec_read_pg_info_block(task->dev_id, task->cmd, (u8*)task->pg_info, sizeof(struct devdrv_pg_info));
    if (ret != 0) {
        TSDRV_PRINT_ERR("Call sec_read_pg_info_block failed. (ret=%d; dev_id=%u)\n", ret, task->dev_id);
        devdrv_set_pg_info_min_spec(task->pg_info, task->multi_die);
    } else {
        TSDRV_PRINT_INFO("Get pg info  success. (dev_id=%u)\n", task->dev_id);
    }
    atomic_set(&g_pg_info_task_state[task->dev_id], HSM_THREAD_STOP);

    return 0;
}

STATIC void devdrv_print_pg_info(struct devdrv_info *dev_info)
{
    TSDRV_PRINT_INFO("PG info get from hsm: (ai_core_num=%u; ai_core_freq=%u; ai_core_bitmap=0x%llx;\n"
        "ai_vector_num=%u; ai_vector_freq=%u; ai_vector_bitmap=0x%llx;\n"
        "vpc_bitmap=0x%llx; jpegd_bitmap=0x%llx; soc_version=%s;  dev_id=%u;\n"

        "ai_cpu_num=%u; ai_cpu_freq=%u; ai_cpu_bitmap=0x%llx;\n"
        "ai_hbm_num=%u; ai_hbm_freq=%u; ai_hbm_bitmap=0x%llx;\n"
        "ai_dvpp_num=%u; ai_dvpp_freq=%u; ai_dvpp_bitmap=0x%llx;\n"
        "ai_sio_num=%u; ai_sio_freq=%u; ai_sio_bitmap=0x%llx;\n"
        "ai_hccs_num=%u; ai_hccs_freq=%u; ai_hccs_bitmap=0x%llx;\n"
        "ai_mata_num=%u; ai_mata_freq=%u; ai_mata_bitmap=0x%llx;\n"
        "ai_l2_num=%u; ai_l2_freq=%u; ai_l2_bitmap=0x%llx;\n"
        "ai_gpu_num=%u; ai_gpu_freq=%u; ai_gpu_bitmap=0x%llx).\n",

        (u32)dev_info->pg_info.comPgInfo.aicPara.minNum, dev_info->pg_info.comPgInfo.aicPara.freq,
        dev_info->pg_info.comPgInfo.aicPara.bitMap,
        (u32)dev_info->pg_info.comPgInfo.aivPara.minNum, dev_info->pg_info.comPgInfo.aivPara.freq,
        dev_info->pg_info.comPgInfo.aivPara.bitMap,
        dev_info->pg_info.comPgInfo.vpcPara.bitMap, dev_info->pg_info.comPgInfo.jpegdPara.bitMap,
        dev_info->pg_info.spePgInfo.socVersion, dev_info->dev_id,

        (u32)dev_info->pg_info.comPgInfo.cpuPara.minNum, dev_info->pg_info.comPgInfo.cpuPara.freq,
        dev_info->pg_info.comPgInfo.cpuPara.bitMap,
        (u32)dev_info->pg_info.comPgInfo.hbmPara.minNum, dev_info->pg_info.comPgInfo.hbmPara.freq,
        dev_info->pg_info.comPgInfo.hbmPara.bitMap,
        (u32)dev_info->pg_info.comPgInfo.dvppPara.minNum, dev_info->pg_info.comPgInfo.dvppPara.freq,
        dev_info->pg_info.comPgInfo.dvppPara.bitMap,
        (u32)dev_info->pg_info.comPgInfo.sioPara.minNum, dev_info->pg_info.comPgInfo.sioPara.freq,
        dev_info->pg_info.comPgInfo.sioPara.bitMap,
        (u32)dev_info->pg_info.comPgInfo.hccsPara.minNum, dev_info->pg_info.comPgInfo.hccsPara.freq,
        dev_info->pg_info.comPgInfo.hccsPara.bitMap,
        (u32)dev_info->pg_info.comPgInfo.mataPara.minNum, dev_info->pg_info.comPgInfo.mataPara.freq,
        dev_info->pg_info.comPgInfo.mataPara.bitMap,
        (u32)dev_info->pg_info.comPgInfo.l2Para.minNum, dev_info->pg_info.comPgInfo.l2Para.freq,
        dev_info->pg_info.comPgInfo.l2Para.bitMap,
        (u32)dev_info->pg_info.comPgInfo.gpuPara.minNum, dev_info->pg_info.comPgInfo.gpuPara.freq,
        dev_info->pg_info.comPgInfo.gpuPara.bitMap);

    return;
}

STATIC void devdrv_get_pg_info(struct devdrv_info *dev_info)
{
    int ret;
    struct task_struct *get_pg_info_task = NULL;
    struct devdrv_pg_info_task arg = {dev_info->dev_id, 0, {PG_MODULE_TYPE_ALL, 0}, &dev_info->pg_info};

    ret = devdrv_manager_get_hw_info(dev_info);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Get hardware info failed. (dev_id=%u; ret=%d)\n", dev_info->dev_id, ret);
        goto out;
    }

    arg.multi_die = dev_info->multi_die;
    atomic_set(&g_pg_info_task_state[dev_info->dev_id], HSM_THREAD_RUN);
    get_pg_info_task =  kthread_run(devdrv_get_pg_info_thread, &arg, "get_pg_info_kthread");
    if (IS_ERR(get_pg_info_task) || (get_pg_info_task == NULL)) {
        TSDRV_PRINT_ERR("Create thread for get pg info failed. (dev_id=%u)\n", dev_info->dev_id);
        goto out;
    }

    while (atomic_read(&g_pg_info_task_state[dev_info->dev_id]) == HSM_THREAD_RUN)
        msleep(TSDRV_HSM_TASK_WAIT_TIME_MS);
    devdrv_print_pg_info(dev_info);
    return;

out:
    devdrv_set_pg_info_min_spec(&dev_info->pg_info, dev_info->multi_die);
    return;
}
#endif

STATIC int devdrv_get_hardware_version(struct devdrv_info *dev_info, unsigned int *hardware_version)
{
#if defined(CFG_SOC_PLATFORM_HELPER)
    *hardware_version = SOC_HARDWARE_VERSION_ASCEND310P;
    TSDRV_PRINT_INFO("Helper the same as ASCEND310P. (hardware_version_16=0x%x; hardware_version_10=%u)\n",
        *hardware_version, *hardware_version);
    return 0;
#else

#if defined(CFG_SOC_PLATFORM_MINIV2)
    unsigned int pad_info = 0;
    void __iomem *regs = NULL;

    regs = ioremap(PAD_INFO_REG_BASE_ADDR, PAD_INFO_MAP_SIZE);
    if (regs == NULL) {
        TSDRV_PRINT_ERR("ioremap error.\n");
        return -ENOMEM;
    }

    pad_info = readl((void __iomem *)(uintptr_t)((long)(uintptr_t)regs + PAD_INFO_REG_OFFSET_ADDR));

    iounmap(regs);
    regs = NULL;
    TSDRV_PRINT_INFO("pad_info =%u .\n", pad_info);

    if (tsdrv_get_chip_type() == CHIP_TYPE_MDC_BS9SX1A) {
        *hardware_version = SOC_HARDWARE_VERSION_BS9SX1A;
    } else {
        if (pad_info & (1 << MDC_DC_MD_OFFSET)) {
            *hardware_version = SOC_HARDWARE_VERSION_ASCEND610;
        } else {
            *hardware_version = SOC_HARDWARE_VERSION_ASCEND310P;
        }
    }

#else
#ifdef CFG_FEATURE_PG
    devdrv_set_hardware_version_by_soc_version(dev_info, hardware_version);
#else
    *hardware_version = SOC_HARDWARE_VERSION;
#endif
#endif
    return 0;
#endif
}

#ifdef CFG_FEATURE_AIV_INFO_FROM_AIC
STATIC void devdrv_get_aiv_info_from_aic(struct devdrv_info *dev_info)
{
    unsigned int i;

    dev_info->vector_core_num = dev_info->pg_info.comPgInfo.aicPara.minNum * 2; /* aiv num is 2 times of aic num */
    dev_info->vector_core_freq = dev_info->pg_info.comPgInfo.aicPara.freq;

    /* aiv bitmap is set according to aic bitmap */
    for (i = 0; i < BITS_PER_BYTE * sizeof(unsigned int); i++) {
        if ((dev_info->pg_info.comPgInfo.aicPara.bitMap) & (1 << i)) {
            dev_info->vector_core_bitmap |= ((u64)1 << (2 * i) | ((u64)1 << (2 * i + 1))); /* 2 times bit of aic */
        }
    }

    TSDRV_PRINT_INFO("Get ai vector info success. (device_id=%u; number=%u; frequency=%llu; bitmap=0x%llx)\n",
        dev_info->dev_id, dev_info->vector_core_num, dev_info->vector_core_freq, dev_info->vector_core_bitmap);
}
#endif

STATIC struct devdrv_info *devdrv_devinfo_create(struct platform_device *pdev, struct devdrv_platform_data *pdata)
{
    struct devdrv_info *dev_info = NULL;
    int ret = 0;
    u32 chip_id, die_id;
    unsigned long bitmap_tmp = 0;

    dev_info = kzalloc(sizeof(struct devdrv_info), GFP_KERNEL);
    if (dev_info == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("kmalloc dev_info failed.\n");
        return NULL;
#endif
    }
    mutex_init(&dev_info->lock);
    dev_info->cce_ops.cce_dev = NULL;
    dev_info->dev_id = pdata->dev_id;

#ifdef CFG_FEATURE_PG
    devdrv_get_pg_info(dev_info);
#endif
    ret = devdrv_get_hardware_version(dev_info, &dev_info->hardware_version);
    if (ret < 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("devdrv_get_hardware_version failed.\n");
        kfree(dev_info);
        dev_info = NULL;
        return NULL;
#endif
    }

    dev_info->pdata = pdata;
    dev_info->env_type = pdata->env_type;
    dev_info->dev = &pdev->dev;
    if (pdata->platform_info.occupy_bitmap == 0) {
#ifndef TSDRV_UT
        dev_info->ai_cpu_core_num = 0;
        dev_info->ai_cpu_core_id = 0;
#endif
    } else {
        dev_info->ai_cpu_core_num = __fls(pdata->platform_info.occupy_bitmap) -
                                    __ffs(pdata->platform_info.occupy_bitmap) + 1;
        dev_info->ai_cpu_core_id = __ffs(pdata->platform_info.occupy_bitmap);
    }

#if ((defined CFG_SOC_PLATFORM_MINI) && (!defined CFG_SOC_PLATFORM_MINIV2))
    pdata->ts_pdata[0].ts_cpu_core_num = 1;
    dev_info->ctrl_cpu_core_num = num_possible_cpus() - dev_info->ai_cpu_core_num;
    dev_info->run_mode = DEVDRV_NORMAL_MODE;
#else
    dev_info->ctrl_cpu_core_num = devdrv_get_cpu_number(pdata->dev_id, CPU_TYPE_OF_CCPU);
#endif
    bitmap_set(&bitmap_tmp, 0, dev_info->ctrl_cpu_core_num);
    dev_info->ctrl_cpu_occupy_bitmap = (u32)bitmap_tmp;

    dev_info->aicpu_occupy_bitmap = pdata->platform_info.occupy_bitmap;
    dev_info->driver_flag = 0;
    dev_info->ai_subsys_ip_broken_map = 0;

    tsdrv_get_chip_version(dev_info, &chip_id, &die_id);
    dev_info->chip_id = chip_id;
    dev_info->die_id = die_id;
#ifndef CFG_FEATURE_HW_INFO_FROM_BIOS
    dev_info->board_id = pdata->platform_info.board_id;
#endif
    dev_info->slot_id = pdata->platform_info.slot_id;
    dev_info->ts_mem_restrict_valid = pdata->ts_mem_restrict_valid;
#if defined(__LITTLE_ENDIAN)
    dev_info->ctrl_cpu_endian_little = 1;
#elif defined(__BIG_ENDIAN)
    dev_info->ctrl_cpu_endian_little = 0;
#endif

#ifdef CFG_FEATURE_PG
    dev_info->ai_core_num = dev_info->pg_info.comPgInfo.aicPara.minNum;
    dev_info->aicore_freq = dev_info->pg_info.comPgInfo.aicPara.freq;
    dev_info->aicore_bitmap = dev_info->pg_info.comPgInfo.aicPara.bitMap;
#else
    dev_info->ai_core_num = pdata->ai_core_num;
    dev_info->aicore_freq = pdata->ai_core_freq;
    dev_info->aicore_bitmap = pdata->ai_core_bitmap;
#endif

#ifdef CFG_FEATURE_AIV_INFO_FROM_AIC
    devdrv_get_aiv_info_from_aic(dev_info);
#elif (defined CFG_FEATURE_AIV_INFO_FROM_PG_INFO_V1)
    dev_info->vector_core_num = pdata->vector_core_num;
    dev_info->vector_core_freq = pdata->vector_core_freq;
    dev_info->vector_core_bitmap = pdata->vector_core_bitmap;
#elif (defined CFG_FEATURE_PG_V2)
    dev_info->vector_core_num = dev_info->pg_info.comPgInfo.aivPara.minNum;
    dev_info->vector_core_freq = dev_info->pg_info.comPgInfo.aivPara.freq;
    dev_info->vector_core_bitmap = dev_info->pg_info.comPgInfo.aivPara.bitMap;
#endif
    dev_info->ai_core_id = 0;
    dev_info->ctrl_cpu_ip = 0;
    dev_info->ctrl_cpu_id = ARMv8_Cortex_A55;
    mutex_init(&dev_info->container.lock);
    dev_info->ts_num = pdata->ts_num;
    dev_info->fw_verify = 0;

#ifdef CFG_SOC_PLATFORM_CLOUD
    sema_init(&dev_info->sem, 0);
#endif

    TSDRV_PRINT_INFO("(devid=%u; ts_num=%u; env_type=%u; aicore_num=%u;"
        "ctrl_cpu_num=%u; ctrl_cpu_bitmap=0x%x; "
        "aicpu_num=%u; aicpu_id=%u; aicpu_bitmap=0x%x; board_id=%u; slot_id=%u)\n",
        dev_info->dev_id, dev_info->ts_num, dev_info->env_type, dev_info->ai_core_num,
        dev_info->ctrl_cpu_core_num, dev_info->ctrl_cpu_occupy_bitmap,
        dev_info->ai_cpu_core_num, dev_info->ai_cpu_core_id, dev_info->aicpu_occupy_bitmap,
        dev_info->board_id, dev_info->slot_id);

    return dev_info;
}
#ifdef AOS_LLVM_BUILD
int tsdrv_manager_register(struct devdrv_info *dev_info)
{
    struct tsdrv_drv_ops *drvops = NULL;
    int ret;

    ret = dms_device_register(dev_info);
    if (ret) {
        TSDRV_PRINT_ERR("Dms device register failed. (dev_id=%u)\n", dev_info->dev_id);
        return -ENODEV;
    }

    drvops = tsdrv_get_drv_ops();
    if (drvops->tsdrv_firmware_load != NULL) {
        if (drvops->tsdrv_firmware_load(dev_info)) {
            TSDRV_PRINT_ERR("ts firmware load failed, devid(%u)\n", dev_info->dev_id);
        }
    }
    return 0;
}
#endif

struct devdrv_info *tsdrv_get_devinfo(u32 devid)
{
    return (struct devdrv_info *)dev_platform_client_info[devid].priv;
}
#ifdef CFG_SOC_PLATFORM_MDC_V51
STATIC int devdrv_device_register_no_tsnode(struct platform_device *pdev)
{
    struct devdrv_platform_data *pdata = NULL;
    struct devdrv_info *dev_info = NULL;
    int err = 0;
    u32 chip_id = 0;
    struct uda_dev_type type;
    struct uda_dev_para para;

    err = tsdrv_get_chip_id(pdev, &chip_id);
    if (err != 0) {
        return err;
    }

    tsdrv_set_platform_device(chip_id, pdev);

    pdata = devdrv_parse_pdata(pdev);
    if ((IS_ERR(pdata) != 0) || (pdata->ts_num > DEVDRV_MAX_TS_NUM)) {
        TSDRV_PRINT_ERR("parse pdata fail or ts num is invalid, err=%d\n", IS_ERR(pdata));
        return -ENOMEM;
    }
    dev_info = devdrv_devinfo_create(pdev, pdata);
    if (dev_info == NULL) {
        TSDRV_PRINT_ERR("dev_info create fail.\n");
        goto err_devinfo_create;
    }

#ifndef AOS_LLVM_BUILD
    err = devdrv_manager_register(dev_info);
#else
    err = tsdrv_manager_register(dev_info);
#endif
    if (err != 0) {
        goto err_devmng_register;
    }

    uda_davinci_local_real_entity_type_pack(&type);
    uda_dev_para_pack(&para, dev_info->dev_id, UDA_INVALID_UDEVID, soc_platform_get_chip_type(), dev_info->dev);
    err = uda_add_dev(&type, &para, &dev_info->dev_id);
    if (err != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Add fail. (dev_id=%u; err=%d)\n", dev_info->dev_id, err);
        goto uda_add_dev_failed;
#endif
    }

    platform_set_drvdata(pdev, dev_info);
    return 0;

#ifndef TSDRV_UT
uda_add_dev_failed:
#ifndef AOS_LLVM_BUILD
    devdrv_drv_unregister(dev_info);
#else
    trs_device_uninit(dev_info->dev_id);
#endif
#endif
err_devmng_register:
    devdrv_devinfo_destroy(dev_info);
err_devinfo_create:
    devdrv_destroy_pdata(pdev, pdata);
    return -ENOMEM;
}

STATIC int devdrv_device_remove_no_tsnode(struct platform_device *pdev, struct devdrv_info *dev_info,
                                          struct devdrv_platform_data *pdata)
{
    struct uda_dev_type type;

#ifndef AOS_LLVM_BUILD
    devdrv_manager_unregister(dev_info);
#else
    dms_device_unregister(dev_info);
#endif

    uda_davinci_local_real_entity_type_pack(&type);
    (void)uda_remove_dev(&type, dev_info->dev_id);
    tsdrv_set_platform_device(dev_info->dev_id, NULL);
    devdrv_destroy_pdata(pdev, pdata);
    devdrv_devinfo_destroy(dev_info);
    platform_set_drvdata(pdev, NULL);
    return 0;
}
#endif

u32 soc_platform_get_chip_type(void)
{
#ifdef CFG_SOC_PLATFORM_MINI
#ifdef CFG_SOC_PLATFORM_MINIV2
    return HISI_MINI_V2;
#endif
#ifdef CFG_SOC_PLATFORM_MINIV3
    return HISI_MINI_V3 ;
#endif
    return HISI_MINI_V1;
#endif

#ifdef CFG_SOC_PLATFORM_CLOUD
#ifdef CFG_SOC_PLATFORM_CLOUD_V2
    return HISI_CLOUD_V2;
#endif
    return HISI_CLOUD_V1;
#endif
}

STATIC int devdrv_device_probe(struct platform_device *pdev)
{
    struct devdrv_platform_data *pdata = NULL;
    struct devdrv_info *dev_info = NULL;
    int err = 0;
    u32 chip_id = 0;
    struct uda_dev_type type;
    struct uda_dev_para para;

#ifdef CFG_SOC_PLATFORM_MDC_V51
    if (devdrv_get_ts_node_num() == 0) {
        TSDRV_PRINT_INFO("ts node does not exist!\n");
        return devdrv_device_register_no_tsnode(pdev);
    }
#endif
    u32 chip_type;
    u32 dev_num;

    err = tsdrv_get_chip_id(pdev, &chip_id);
    if (err != 0) {
        return err;
    }

    tsdrv_set_platform_device(chip_id, pdev);

    pdata = devdrv_parse_pdata(pdev);
    if ((IS_ERR(pdata) != 0) || (pdata->ts_num > DEVDRV_MAX_TS_NUM)) {
        TSDRV_PRINT_ERR("parse pdata fail or ts num is invalid, err=%d\n", IS_ERR(pdata));
        return -ENOMEM;
    }
    dev_info = devdrv_devinfo_create(pdev, pdata);
    if (dev_info == NULL) {
        TSDRV_PRINT_ERR("dev_info create fail.\n");
        goto err_devinfo_create;
    }

    chip_type = soc_platform_get_chip_type();
    dev_num = (u32)tsdrv_get_cpu_node_num();
    err = dbl_nid_auto_set_nid(chip_type, dev_num, dev_info->dev_id);
    if (err != 0) {
        TSDRV_PRINT_WARN("dbl_nid_auto_set_nid unsuccessful. (devid=%u; chiptype=%u; devnum=%u; err=%d\n",
            dev_info->dev_id, chip_type, dev_num, err);
    }

    err = tsdrv_intr_init(dev_info);
    if (err != 0) {
        goto err_intr_init;
    }

    tsdrv_ipc_register(dev_info->dev_id);
#ifndef AOS_LLVM_BUILD
    err = devdrv_manager_register(dev_info);
#else
    err = tsdrv_manager_register(dev_info);
#endif
    if (err != 0) {
        goto err_devmng_register;
    }

    dev_platform_client_info[pdata->dev_id].priv = dev_info;
    /* register fault manager */
    err = tsdrv_fault_dev_init(dev_info->dev_id);
    if (err != 0) {
        TSDRV_PRINT_ERR("Failed to initialize the fault manager. (ret=%d; devid=%u)\n", err, dev_info->dev_id);
        goto err_tsdrv_register;
    }

    uda_davinci_local_real_entity_type_pack(&type);
    uda_dev_para_pack(&para, dev_info->dev_id, UDA_INVALID_UDEVID, soc_platform_get_chip_type(), dev_info->dev);
    err = uda_add_dev(&type, &para, &dev_info->dev_id);
    if (err != 0) {
        TSDRV_PRINT_ERR("Add fail. (dev_id=%u; err=%d)\n", dev_info->dev_id, err);
        goto err_fault_register;
    }

#if (defined CFG_FEATURE_VFIO_DEVICE) && (defined CFG_FEATURE_PG)
    vmngd_set_pg_info(dev_info->dev_id, (vmng_common_pg *)&dev_info->pg_info.comPgInfo);
#endif

    (void)tsdrv_heart_beat_init(dev_info->dev_id);

#ifndef CFG_TRS_REFACTOR_FEATURE
    (void)tsdrv_hb_dfx_file_create(dev_info->dev_id, TSDRV_PM_FID);
    (void)tsdrv_ipc_dfx_file_create(dev_info->dev_id, TSDRV_PM_FID);
    (void)tsdrv_alloc_mem_dfx_file_create(dev_info->dev_id, dev_info->ts_num, TSDRV_PM_FID);
    (void)dev_set_name(&pdev->dev, "platform");
    tsdrv_set_send_msg_proc(tsdrv_d2h_slow_msg_send);
#endif
    dev_platform_client_info[pdata->dev_id].init = 1;
    platform_set_drvdata(pdev, dev_info);

    TSDRV_PRINT_INFO("Tsdrv probe. (dev_num=%u)\n", dev_num);

    return 0;

err_fault_register:
    tsdrv_fault_dev_exit(dev_info->dev_id);
err_tsdrv_register:
#ifndef AOS_LLVM_BUILD
    devdrv_manager_unregister(dev_info);
#else
    dms_device_unregister(dev_info);
#endif
err_devmng_register:
    tsdrv_ipc_unregister(dev_info->dev_id);
err_intr_init:
    devdrv_devinfo_destroy(dev_info);
err_devinfo_create:
    devdrv_destroy_pdata(pdev, pdata);
    return -ENOMEM;
}

STATIC int devdrv_device_remove(struct platform_device *pdev)
{
    struct devdrv_platform_data *pdata = NULL;
    struct devdrv_info *dev_info = NULL;
    struct uda_dev_type type;

    if (pdev == NULL) {
        TSDRV_PRINT_ERR("pdev is NULL\n");
        return -EINVAL;
    }

    dev_info = platform_get_drvdata(pdev);
    if (dev_info == NULL) {
        TSDRV_PRINT_ERR("dev_info is NULL.\n");
        return -ENOMEM;
    }

    if (dev_info->dev_id >= MAX_CHIP_NUM) {
        TSDRV_PRINT_ERR("device id(%u) invalid, return.\n", dev_info->dev_id);
        return -ENOMEM;
    }

    pdata = (struct devdrv_platform_data *)dev_info->pdata;
    if (pdata == NULL) {
        TSDRV_PRINT_ERR("pdata is NULL.\n");
        devdrv_devinfo_destroy(dev_info);
        return -ENOMEM;
    }

#ifdef CFG_SOC_PLATFORM_MDC_V51
    if (devdrv_get_ts_node_num() == 0) {
        TSDRV_PRINT_INFO("ts node does not exist!\n");
        return devdrv_device_remove_no_tsnode(pdev, dev_info, pdata);
    }
#endif

    tsdrv_fault_dev_exit(dev_info->dev_id);
    tsdrv_heart_beat_exit(dev_info->dev_id);

    uda_davinci_local_real_entity_type_pack(&type);
    (void)uda_remove_dev(&type, dev_info->dev_id);

    dev_platform_client_info[dev_info->dev_id].init = 0;
    dev_platform_client_info[dev_info->dev_id].priv = NULL;

#ifndef AOS_LLVM_BUILD
    devdrv_manager_unregister(dev_info);
#else
    dms_device_unregister(dev_info);
#endif
    tsdrv_ipc_unregister(dev_info->dev_id);

#ifdef TSDRV_UT
    devdrv_destroy_ts_irq(dev_info);
#endif

    tsdrv_set_platform_device(dev_info->dev_id, NULL);
    devdrv_destroy_pdata(pdev, pdata);
    devdrv_devinfo_destroy(dev_info);
    platform_set_drvdata(pdev, NULL);
    return 0;
}

STATIC void devdrv_device_shutdown(struct platform_device *pdev)
{
#ifndef TSDRV_UT
    struct devdrv_info *dev_info = NULL;

    if (pdev == NULL) {
        TSDRV_PRINT_ERR("Platform device is NULL.\n");
        return;
    }

    dev_info = platform_get_drvdata(pdev);
    DRV_CHECK_PTR(dev_info, return, "Device info is invalid.\n");
    DRV_CHECK_EXP_ACT(dev_info->dev_id >= MAX_CHIP_NUM, return,
            "Device id is invalid. (devid=%u)\n", dev_info->dev_id);

    TSDRV_PRINT_INFO("Devdrv device shutdown. (devid=%u)\n", dev_info->dev_id);

    dev_info->status = DEVINFO_STATUS_SHUTDOWN;

#ifndef AOS_LLVM_BUILD
    devdrv_manager_unregister(dev_info);
#else
    dms_device_unregister(dev_info);
#endif
    (void)uda_dev_ctrl(dev_info->dev_id, UDA_CTRL_SHUTDOWN);
    tsdrv_pm_shutdown(dev_info);
#endif
}

static const struct of_device_id mini_devdrv_of_match[] = {
    { .compatible = "hisi,mini-devdrv-device" },
    {}
};
MODULE_DEVICE_TABLE(of, mini_devdrv_of_match);
#ifndef AOS_LLVM_BUILD
static const struct acpi_device_id devdrv_acpi_match[] = {
    {"HISI0000", 0}, {"HISI0301", 0}, {"HISI0303", 0}, {}
};
MODULE_DEVICE_TABLE(acpi, devdrv_acpi_match);
#endif
static struct platform_driver devdrv_platform_driver = {
    .probe = devdrv_device_probe,
    .remove = devdrv_device_remove,
    .suspend = devdrv_device_suspend,
    .resume = devdrv_device_resume,
    .shutdown = devdrv_device_shutdown,
    .driver = {
        .name = "devdrv_device_driver",
        .of_match_table = mini_devdrv_of_match,
#ifndef AOS_LLVM_BUILD
        .acpi_match_table = ACPI_PTR(devdrv_acpi_match),
#endif
    },
};

#ifdef CFG_SOC_PLATFORM_MINIV3
#define TSCPU_MAX_LPI_IRQ_NUM 14
#else
#define TSCPU_MAX_LPI_IRQ_NUM 16
#endif
#ifndef AOS_LLVM_BUILD
static void tscpu_write_lpi_msgs(struct msi_desc *desc, struct msi_msg *msg)
{
}

static void tscpu_free_msis(void *data)
{
    struct device *dev = data;
    platform_msi_domain_free_irqs(dev);
}
#endif
#ifndef CFG_SOC_PLATFORM_CLOUD_V2
static irqreturn_t irq_handle_not_used(int irq, void *data)
{
    return IRQ_HANDLED;
}
#endif

static u32 tscpu_irq[TSCPU_MAX_LPI_IRQ_NUM];
STATIC int tscpu_device_probe(struct platform_device *pdev)
{
    int irq_num = TSCPU_MAX_LPI_IRQ_NUM;
#ifndef AOS_LLVM_BUILD
    struct msi_desc *desc = NULL;
#endif
    static u32 devid = 0;
    u32 base_irq = 0;
    int ret = 0;
    int i, j;

    if (devid >= MAX_CHIP_NUM) {
        TSDRV_PRINT_ERR("Invalid devid=%u, MAX_CHIP_NUM=%d\n", devid, (int)MAX_CHIP_NUM);
        return -ENODEV;
    }

    if (!acpi_disabled) {
#ifndef AOS_LLVM_BUILD
        ret = platform_msi_domain_alloc_irqs(&pdev->dev, irq_num, tscpu_write_lpi_msgs);
        if (ret != 0) {
            TSDRV_PRINT_ERR("failed to allocate LPI(%d)\n", ret);
            return ret;
        }
        (void)devm_add_action(&pdev->dev, tscpu_free_msis, &pdev->dev);
        desc = first_msi_entry(&pdev->dev);
        if (desc != NULL) {
            base_irq = desc->irq;
        }
#else
        TSDRV_PRINT_ERR("acpi enabled\n");
#endif
    } else {
        for (i = 0; i < irq_num; i++) {
            tscpu_irq[i] = irq_of_parse_and_map(pdev->dev.of_node, i);
            if (tscpu_irq[i] == 0) {
                TSDRV_PRINT_ERR("irq parse fail, index=%d\n", i);
                goto err_request_irq;
            }
#ifndef CFG_SOC_PLATFORM_CLOUD_V2
            ret = request_irq(tscpu_irq[i], irq_handle_not_used, 0, "irq_route_to_ts", NULL);
            if (ret != 0) {
                tscpu_irq[i] = 0;
                TSDRV_PRINT_ERR("request irq fail, irq[%d]=%u\n", i, tscpu_irq[i]);
                goto err_request_irq;
            }
            (void)devdrv_set_irq_affinity(tscpu_irq[i], get_cpu_mask(4));
#endif
        }
#ifndef CFG_SOC_PLATFORM_CLOUD_V2
        if (tsdrv_get_hwirq_from_irq(tscpu_irq[0], &base_irq) != 0) {
            goto err_request_irq;
        }
#else
        base_irq = tscpu_irq[0];
#endif
    }

    tscpu_start_irq[devid] = base_irq;
    tscpu_irq_num[devid] = irq_num;

    TSDRV_PRINT_INFO("chip_num=%u; tscpu_start_irq=%u; num=%d \n", devid, base_irq, irq_num);
    devid++;

    return 0;

err_request_irq:
    for (j = 0; j < i; j++) {
        (void)free_irq(tscpu_irq[j], NULL);
        tscpu_irq[j] = 0;
    }
    return -ENODEV;
}

STATIC int tscpu_device_remove(struct platform_device *pdev)
{
    int i;

    for (i = 0; i < TSCPU_MAX_LPI_IRQ_NUM; i++) {
        if (tscpu_irq[i] != 0) {
            (void)free_irq(tscpu_irq[i], NULL);
            tscpu_irq[i] = 0;
        }
    }
    return 0;
}
#ifndef AOS_LLVM_BUILD
static const struct acpi_device_id tscpu_acpi_match[] = {
    {"HISI0001", 0}, {}
};
MODULE_DEVICE_TABLE(acpi, tscpu_acpi_match);
#endif
static const struct of_device_id tscpu_of_match[] = {
    { .compatible = "hisi,tscpu_irq" },
    {}
};
MODULE_DEVICE_TABLE(of, tscpu_of_match);

/* add this devcie alloc lpi irq for ts */
static struct platform_driver tscpu_platform_driver = {
    .probe = tscpu_device_probe,
    .remove = tscpu_device_remove,
    .driver = {
        .name = "tscpu_driver",
#ifndef AOS_LLVM_BUILD
        .acpi_match_table = ACPI_PTR(tscpu_acpi_match),
#endif
        .of_match_table = tscpu_of_match,
    },
};

static void tsdrv_drv_ops_init(void)
{
    struct tsdrv_drv_ops *drvops = NULL;
#ifndef AOS_LLVM_BUILD
    drvops = devdrv_manager_get_drv_ops();
#else
    drvops = tsdrv_get_drv_ops();
#endif
    if (drvops == NULL) {
        return;
    }

    drvops->tsdrv_firmware_load = tsdrv_firmware_load;
    drvops->tsdrv_heart_beat_set_work_state = tsdrv_heart_beat_set_work_state;
}

#define FFTS_CPU_MAX_LPI_IRQ_NUM 1
static u32 ffts_cpu_irq[TSDRV_MAX_DAVINCI_NUM][FFTS_CPU_MAX_LPI_IRQ_NUM];

static irqreturn_t ffts_cpu_irq_handle(int irq, void *data)
{
    return IRQ_HANDLED;
}

#ifndef AOS_LLVM_BUILD
static void ffts_cpu_write_lpi_msgs(struct msi_desc *desc, struct msi_msg *msg)
{
}

static void ffts_cpu_free_msis(void *data)
{
    struct device *dev = data;
    platform_msi_domain_free_irqs(dev);
}

int tsdrv_get_ffts_mcu_irq_id(u32 dev_id, u32 *hwirq)
{
    struct irq_data *irq_data = NULL;
    struct irq_data *parent = NULL;
    struct irq_desc *desc = NULL;
    u32 irq = ffts_cpu_irq[dev_id][0];

    desc = irq_to_desc(irq);
    if (desc == NULL) {
        TSDRV_PRINT_ERR("Irq is invalid. (irq=%u)\n", irq);
        return -EINVAL;
    }

    irq_data = irq_desc_get_irq_data(desc);
    parent = irq_data->parent_data;

    while (parent != NULL) {
        irq_data = parent;
        parent = irq_data->parent_data;
    }

    TSDRV_PRINT_EVENT("Shift hwirq id ok. (dev_id=%u; hwirq=%u)\n", dev_id, (u32)irq_data->hwirq);

    *hwirq = (u32)irq_data->hwirq;
    return 0;
}
#endif
int ffts_cpu_device_probe(struct platform_device *pdev)
{
#ifndef TSDRV_UT
    int irq_num = FFTS_CPU_MAX_LPI_IRQ_NUM;
#ifndef AOS_LLVM_BUILD
    struct msi_desc *desc = NULL;
#endif
    static u32 devid = 0;
    u32 base_irq = 0;
    int ret, i, j, fftscpu_id;

    if (devid >= TSDRV_MAX_DAVINCI_NUM) {
        TSDRV_PRINT_ERR("Invalid param. devid[%u], MAX_CHIP_NUM[%d].\n", devid, TSDRV_MAX_DAVINCI_NUM);
        return -ENODEV;
    }

    fftscpu_id = tsdrv_get_fftscpu_id(devid);
    if (fftscpu_id < 0) {
        TSDRV_PRINT_INFO("Unable to et fftscpu_id. devid[%u], fftscpu_id[%d].\n", devid, fftscpu_id);
        return -EINVAL;
    }

    if (!acpi_disabled) {
#ifndef AOS_LLVM_BUILD
        ret = platform_msi_domain_alloc_irqs(&pdev->dev, irq_num, ffts_cpu_write_lpi_msgs);
        if (ret != 0) {
            TSDRV_PRINT_ERR("Failed to allocate LPI, ret[%d]\n", ret);
            return ret;
        }
        (void)devm_add_action(&pdev->dev, ffts_cpu_free_msis, &pdev->dev);
        desc = first_msi_entry(&pdev->dev);
        base_irq = desc->irq;
#else
        TSDRV_PRINT_ERR("acpi enabled\n");
#endif
    } else {
        for (i = 0; i < irq_num; i++) {
            ffts_cpu_irq[devid][i] = irq_of_parse_and_map(pdev->dev.of_node, i);
            if (ffts_cpu_irq[devid][i] == 0) {
                TSDRV_PRINT_ERR("ffts irq parse fail, index=%d\n", i);
                return -EINVAL;
            }
            TSDRV_PRINT_INFO("ffts irq = %u.\n", ffts_cpu_irq[devid][i]);
        }
    }

    for (i = 0; i < irq_num; i++) {
#ifndef AOS_LLVM_BUILD
        if (!acpi_disabled) {
            ffts_cpu_irq[devid][i] = base_irq + (u32)i;
        }
#endif
        ret = request_irq(ffts_cpu_irq[devid][i], ffts_cpu_irq_handle, 0, "ffts_cpu_proc", NULL);
        if (ret != 0) {
            TSDRV_PRINT_ERR("Request irq failed. devid[%u], irq[%u], ret[%d]\n", devid, ffts_cpu_irq[devid][i], ret);
            goto err_request_irq;
        }

        (void)devdrv_set_irq_affinity(ffts_cpu_irq[devid][i], get_cpu_mask((u32)fftscpu_id));
    }

    TSDRV_PRINT_INFO("devid[%u], start_irq[%u], irq_num[%d], ffts_cpu_id[%d].\n",
        devid, base_irq, irq_num, fftscpu_id);

    devid++;

    return 0;
err_request_irq:
    for (j = 0; j < i; j++) {
        if (ffts_cpu_irq[devid][j] != 0) {
            (void)devdrv_set_irq_affinity(ffts_cpu_irq[devid][j], NULL);
            (void)free_irq(ffts_cpu_irq[devid][j], NULL);
        }
    }
    return -ENODEV;
#endif
}

int ffts_cpu_device_remove(struct platform_device *pdev)
{
#ifndef TSDRV_UT
    static u32 devid = 0;
    int i;

    for (i = 0; i < FFTS_CPU_MAX_LPI_IRQ_NUM; i++) {
        if (ffts_cpu_irq[devid][i] != 0) {
            devdrv_set_irq_affinity(ffts_cpu_irq[devid][i], NULL);
            (void)free_irq(ffts_cpu_irq[devid][i], NULL);
            ffts_cpu_irq[devid][i] = 0;
        }
    }
    devid++;
#endif
    return 0;
}
#ifndef AOS_LLVM_BUILD
static const struct acpi_device_id ffts_cpu_acpi_match[] = {
    {"HISI0002", 0}, {}
};
MODULE_DEVICE_TABLE(acpi, ffts_cpu_acpi_match);
#endif
static const struct of_device_id ffts_cpu_of_match[] = {
    {.compatible = "hisi,ffts_cpu_irq" },
    {}
};
MODULE_DEVICE_TABLE(of, ffts_cpu_of_match);

/* add this devcie alloc lpi irq for ts */
static struct platform_driver ffts_cpu_platform_driver = {
    .probe = ffts_cpu_device_probe,
    .remove = ffts_cpu_device_remove,
    .driver = {
        .name = "ffts_cpu_driver",
#ifndef AOS_LLVM_BUILD
        .acpi_match_table = ACPI_PTR(ffts_cpu_acpi_match),
#endif
        .of_match_table = ffts_cpu_of_match,
    },
};

#ifdef CFG_FEATURE_QOS
enum tsdrv_qos_node {
    TSDRV_QOS_NODE_AIC_DAT,
    TSDRV_QOS_NODE_AIC_INS,
    TSDRV_QOS_NODE_AIV_DAT,
    TSDRV_QOS_NODE_AIV_INS,
    TSDRV_QOS_NODE_SDNA,
    TSDRV_QOS_NODE_MAX
};

static struct qos_master_node node[TSDRV_QOS_NODE_MAX];
#define QOS_NODE_NAME_MAX_LEN 256
static int tsdrv_qos_node_register(void)
{
    int ret, idx, i;
    const char* name[5] = {"AIC_DAT_QOS", "AIC_INS_QOS", "AIV_DAT_QOS", "AIV_INS_QOS", "SDMA_QOS"};

    for (idx = 0; idx < TSDRV_QOS_NODE_MAX; idx++) {
        ret = strcpy_s(node[idx].name, QOS_NODE_NAME_MAX_LEN, name[idx]);
        if (ret != 0) {
            TSDRV_PRINT_ERR("Call strcpy_s failed. (ret=%d)\n", ret);
            goto err_qos_node_resigster;
        }
        node[idx].cfg.type = idx + MASTER_AIC_DAT;
        node[idx].set = tsdrv_set_qos_cfg;
        node[idx].get = tsdrv_get_qos_cfg;
        node[idx].set_allow = tsdrv_set_allow_cfg;
        node[idx].get_allow = tsdrv_get_allow_cfg;
        node[idx].set_otsd = tsdrv_set_otsd_cfg;
        node[idx].get_otsd = tsdrv_get_otsd_cfg;

        ret = hal_kernel_qos_node_register(&node[idx]);
        if (ret != 0) {
            TSDRV_PRINT_ERR("qos node resigster failed.\n");
            goto err_qos_node_resigster;
        }
    }

    return 0;

err_qos_node_resigster:
    for (i = 0; i < idx; i++) {
        hal_kernel_qos_node_unregister(&node[i]);
    }
    return -EINVAL;
}

static void tsdrv_qos_node_unregister(void)
{
    int idx;

    for (idx = 0; idx < TSDRV_QOS_NODE_MAX; idx++) {
        (void)hal_kernel_qos_node_unregister(&node[idx]);
        node[idx].cfg.type = MASTER_INVALID;
        node[idx].set = NULL;
        node[idx].get = NULL;
    }
}
#endif

static struct notifier_block tsdrv_panic_block = {
    .notifier_call = tsdrv_ts_panic_notify,
    .priority = INT_MIN + 1,
};

#ifndef CFG_TRS_REFACTOR_FEATURE
#include "dbl/uda.h"

#define TSDRV_DEVICE_NOTIFIER "tsdrv_device"
static int tsdrv_device_notifier_func(u32 udevid, enum uda_notified_action action)
{
    struct devdrv_info *dev_info = NULL;
    int ret = 0;

    if ((udevid >= MAX_CHIP_NUM) || (dev_platform_client_info[udevid].priv == NULL)) {
        TSDRV_PRINT_ERR("Invalid para. (udevid=%u)\n", udevid);
        return -EINVAL;
    }

    dev_info = (struct devdrv_info *)dev_platform_client_info[udevid].priv;

    if (action == UDA_INIT) {
        ret = devdrv_drv_register(dev_info);
    } else if (action == UDA_UNINIT) {
        devdrv_drv_unregister(dev_info);
    }

    TSDRV_PRINT_INFO("notifier action. (udevid=%u; action=%d; ret=%d)\n", udevid, action, ret);

    return ret;
}

static int tsdrv_notifier_register(void)
{
    struct uda_dev_type type;
    int ret;

    uda_davinci_local_real_entity_type_pack(&type);
    ret = uda_notifier_register(TSDRV_DEVICE_NOTIFIER, &type, UDA_PRI1, tsdrv_device_notifier_func);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Register notifier failed. (ret=%d)\n", ret);
        return ret;
    }

    return 0;
}

static void tsdrv_notifier_unregister(void)
{
    struct uda_dev_type type;

    uda_davinci_local_real_entity_type_pack(&type);
    (void)uda_notifier_unregister(TSDRV_DEVICE_NOTIFIER, &type);
}
#endif

STATIC int __init devdrv_platform_init(void)
{
    int err = 0;
#ifdef CFG_SOC_PLATFORM_MDC_V51
    if (devdrv_get_ts_node_num() == 0) {
        TSDRV_PRINT_INFO("ts node does not exist!\n");
        err = platform_driver_register(&devdrv_platform_driver);
        if (err != 0) {
            TSDRV_PRINT_ERR("platform drv register fail, err=%d.\n", err);
            return -ENODEV;
        }
        return 0;
    }
#endif
#ifdef AOS_LLVM_BUILD
    tsdrv_status_init();
#endif
    tsdrv_drv_ops_init();
    tsdrv_ts_mem_mgmt_init();
#ifndef AOS_LLVM_BUILD
    err = atomic_notifier_chain_register(&panic_notifier_list, &tsdrv_panic_block);
    if (err != 0) {
        TSDRV_PRINT_ERR("Failed to register panic notifier chain.\n");
        goto err_notifier_chain_register;
    }
#endif
    tsdrv_fault_init();
    tsdrv_pm_init();
    tsdrv_firmware_load_init();

#ifndef CFG_TRS_REFACTOR_FEATURE
    err = tsdrv_notifier_register();
    if (err != 0) {
        goto err_notifier_register;
    }

    err = tsdrv_d2h_chan_setup();

    if (err != 0) {
        goto err_d2h_chan_init;
    }
#endif

    (void)platform_driver_register(&ffts_cpu_platform_driver);
    (void)platform_driver_register(&tscpu_platform_driver);
    err = platform_driver_register(&devdrv_platform_driver);
    if (err != 0) {
        TSDRV_PRINT_ERR("platform drv register fail, err=%d.\n", err);
        goto err_platform_register;
    }

#ifdef CFG_FEATURE_QOS
    tsdrv_qos_node_register();
#endif

    TSDRV_PRINT_INFO("platform drv init succeed.\n");

    return 0;
err_platform_register:
#ifndef CFG_TRS_REFACTOR_FEATURE
    tsdrv_d2h_chan_cleanup();
err_d2h_chan_init:
    tsdrv_notifier_unregister();
err_notifier_register:
#endif
    tsdrv_firmware_load_exit();
    tsdrv_pm_exit();
    tsdrv_fault_exit();
#ifndef AOS_LLVM_BUILD
    (void)atomic_notifier_chain_unregister(&panic_notifier_list, &tsdrv_panic_block);
#endif
err_notifier_chain_register:
    tsdrv_ts_mem_mgmt_exit();
    return -ENODEV;
}
module_init(devdrv_platform_init);

STATIC void __exit devdrv_platform_exit(void)
{
#ifdef CFG_FEATURE_QOS
    tsdrv_qos_node_unregister();
#endif
#ifndef CFG_TRS_REFACTOR_FEATURE
    tsdrv_d2h_chan_cleanup();
    tsdrv_notifier_unregister();
#endif
    platform_driver_unregister(&devdrv_platform_driver);
    platform_driver_unregister(&tscpu_platform_driver);
    platform_driver_unregister(&ffts_cpu_platform_driver);
    tsdrv_firmware_load_exit();
    tsdrv_pm_exit();
#ifndef AOS_LLVM_BUILD
    (void)atomic_notifier_chain_unregister(&panic_notifier_list, &tsdrv_panic_block);
#endif
    tsdrv_fault_exit();
    tsdrv_ts_mem_mgmt_exit();
}
module_exit(devdrv_platform_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("DAVINCI driver");
#else
int devdrv_plat_utest(void)
{
    return 0;
}
#endif

