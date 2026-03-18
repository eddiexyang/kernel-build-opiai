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

#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#ifdef AOS_LLVM_BUILD
#include <linux/timecounter.h>

#include "devdrv_manager_comm.h"
#endif
#include "tsdrv_heartbeat.h"
#include "tsdrv_device.h"
#include "tsdrv_common.h"
#include "devdrv_user_common.h"
#include "devdrv_driver_pm.h"
#include "devdrv_pm.h"
#include "devdrv_manager_time.h"
#include "devdrv_manager_common.h"
#include "tsdrv_kernel_common.h"
#include "tsdrv_osal_bbox.h"
#include "tsdrv_pdata.h"
#include "tsdrv_fault_init.h"
#include "tsdrv_firmware_load.h"
#include "devdrv_platform.h"
#include "tsdrv_cpumask.h"
#ifndef TSDRV_UT
#include "dms_device_time_zone.h"
#endif

#define TSDRV_HEARTBEAT_MAX_LOST_COUNT 3
#define TSDRV_HEARTBEAT_CYCLE_TIME 6000

#ifdef CFG_FEATURE_TS_HB_DFX
/* =============================tsfw dfx reg info start=================================== */
#define ASCEND910_TS_HEART_BEAT_DOTTING_REG 0xaf000be8     /* SOC_TS_SYSCTRL_SC_TESTREG2_REG */
#define ASCEND910_TS_MAIN_THREAD_DOTTING_REG 0xaf000c04    /* SOC_TS_SYSCTRL_SC_TESTREG3_REG */
#define ASCEND910_TS_DOORBELL_DOTTING_REG 0xaf000c00       /* SOC_TS_SYSCTRL_SC_TESTREG8_REG */

#ifdef CFG_SOC_MDC_V51_LITE
#define ASCEND310P_TSC_HEART_BEAT_DOTTING_REG 0xb9080be8   /* SOC_TS_AIC_SYSCTRL_SC_TESTREG2_REG */
#define ASCEND310P_TSC_MAIN_THREAD_DOTTING_REG 0xb9080c04  /* SOC_TS_AIC_SYSCTRL_SC_TESTREG9_REG */
#define ASCEND310P_TSC_DOORBELL_DOTTING_REG 0xb9080c00     /* SOC_TS_AIC_SYSCTRL_SC_TESTREG8_REG */
#else
#define ASCEND310P_TSC_HEART_BEAT_DOTTING_REG 0xba080be8   /* SOC_TS_AIC_SYSCTRL_SC_TESTREG2_REG */
#define ASCEND310P_TSC_MAIN_THREAD_DOTTING_REG 0xba080c04  /* SOC_TS_AIC_SYSCTRL_SC_TESTREG9_REG */
#define ASCEND310P_TSC_DOORBELL_DOTTING_REG 0xba080c00     /* SOC_TS_AIC_SYSCTRL_SC_TESTREG8_REG */
#endif

#define ASCEND310P_TSV_HEART_BEAT_DOTTING_REG 0xb9080be8   /* SOC_TS_VEC_SYSCTRL_SC_TESTREG2_REG */
#define ASCEND310P_TSV_MAIN_THREAD_DOTTING_REG 0xb9080c04  /* SOC_TS_VEC_SYSCTRL_SC_TESTREG9_REG */
#define ASCEND310P_TSV_DOORBELL_DOTTING_REG 0xb9080c00     /* SOC_TS_VEC_SYSCTRL_SC_TESTREG8_REG */
#define ASCEND910B_TS_HEART_BEAT_DOTTING_REG 0x680040be8   /* SOC_TS_SYSCTRL_SC_TESTREG2_REG */
#define ASCEND910B_TS_MAIN_THREAD_DOTTING_REG 0x680040c04  /* SOC_TS_SYSCTRL_SC_TESTREG9_REG */
#define ASCEND910B_TS_DOORBELL_DOTTING_REG 0x680040c00     /* SOC_TS_SYSCTRL_SC_TESTREG8_REG */
#define TS_DFX_REG_SIZE 0x4
/* =============================tsfw dfx reg info end===================================== */
#endif

STATIC const u32 g_cycle_time = TSDRV_HEARTBEAT_CYCLE_TIME;

STATIC struct tsdrv_heart_beat_info heart_beat_info[TSDRV_MAX_DAVINCI_NUM][TSDRV_HEARTBEAT_MAX_TS_NUM];

#ifdef CFG_FEATURE_TS_HB_DFX
void tsdrv_hb_tsfw_dfx_reg_show(u32 ts_status, struct tsdrv_heart_beat_info *heart_beat, u32 flag)
{
    unsigned char *heart_beart = heart_beat->tsfw_reg.heart_beart_dotting;
    unsigned char *main_thread = heart_beat->tsfw_reg.main_thread_dotting;
    unsigned char *doorbell = heart_beat->tsfw_reg.doorbell_dotting;
    u32 devid = heart_beat->instance.devid;
    u32 tsid = heart_beat->instance.tsid;

    if (flag == TS_STATUS_ERR_SHOW) {
        TSDRV_PRINT_EVENT("[devid-%u][tsid-%u] ts_status(%u) is excepted! TS_HEART_BEAT_DOTTING(%u), "
            "TS_MAIN_THREAD_DOTTING(%u), TS_DOORBELL_DOTTING(%u)\n", devid, tsid, ts_status,
#ifndef AOS_LLVM_BUILD
            readl_relaxed(heart_beart), readl_relaxed(main_thread), readl_relaxed(doorbell));
#else
            readl(heart_beart), readl(main_thread), readl(doorbell));
#endif
        return;
    }

    if (flag == TS_HEART_BEAT_LOST_SHOW) {
        TSDRV_PRINT_EVENT("[devid-%u][tsid-%u] heart beat lost occur! lost_count(%u), TS_HEART_BEAT_DOTTING(%u), "
            "TS_MAIN_THREAD_DOTTING(%u), TS_DOORBELL_DOTTING(%u)\n", devid, tsid, (u32)heart_beat->lost_count,
#ifndef AOS_LLVM_BUILD
            readl_relaxed(heart_beart), readl_relaxed(main_thread), readl_relaxed(doorbell));
#else
            readl(heart_beart), readl(main_thread), readl(doorbell));
#endif
    }
}

int tsdrv_heart_beat_tsfw_dfx_reg_ioremap(u32 devid, u64 heart_beat_pa,
    u64 main_thread_pa, u64 doorbell_pa, struct tsdrv_heart_beat_tsfw_dfx_reg *tsfw_reg)
{
    u32 chip_id, die_id;

    tsdrv_get_chip_version(tsdrv_get_devinfo(devid), &chip_id, &die_id);
    TSDRV_PRINT_DEBUG("devid=%u, chip_id=%u, die_id=%u\n", devid, chip_id, die_id);

    tsfw_reg->heart_beart_dotting = (void *)ioremap(heart_beat_pa + chip_id * CHIP_BASEADDR_PA_OFFSET +
        die_id * DIE_BASEADDR_PA_OFFSET, TS_DFX_REG_SIZE);
    if (tsfw_reg->heart_beart_dotting == NULL) {
        TSDRV_PRINT_ERR("ioremap heart_beart_dotting failed\n");
        return -ENOMEM;
    }

    tsfw_reg->main_thread_dotting = (void *)ioremap(main_thread_pa + chip_id * CHIP_BASEADDR_PA_OFFSET +
        die_id * DIE_BASEADDR_PA_OFFSET, TS_DFX_REG_SIZE);
    if (tsfw_reg->main_thread_dotting == NULL) {
        TSDRV_PRINT_ERR("ioremap main_thread_dotting failed\n");
        goto err1;
    }

    tsfw_reg->doorbell_dotting = (void *)ioremap(doorbell_pa + chip_id * CHIP_BASEADDR_PA_OFFSET +
        die_id * DIE_BASEADDR_PA_OFFSET, TS_DFX_REG_SIZE);
    if (tsfw_reg->doorbell_dotting == NULL) {
        TSDRV_PRINT_ERR("ioremap doorbell_dotting failed\n");
        goto err2;
    }

    return 0;
err2:
    iounmap(tsfw_reg->main_thread_dotting);
    tsfw_reg->main_thread_dotting = NULL;
err1:
    iounmap(tsfw_reg->heart_beart_dotting);
    tsfw_reg->heart_beart_dotting = NULL;
    return -ENOMEM;
}

int tsdrv_heart_beat_tsfw_dfx_reg_init(u32 devid, u32 tsid,
    struct tsdrv_heart_beat_tsfw_dfx_reg *tsfw_reg)
{
    int ret;

    if (tsid == 0) {
        ret = tsdrv_heart_beat_tsfw_dfx_reg_ioremap(devid, ASCEND310P_TSC_HEART_BEAT_DOTTING_REG,
            ASCEND310P_TSC_MAIN_THREAD_DOTTING_REG,
            ASCEND310P_TSC_DOORBELL_DOTTING_REG, tsfw_reg);
    } else {
        ret = tsdrv_heart_beat_tsfw_dfx_reg_ioremap(devid, ASCEND310P_TSV_HEART_BEAT_DOTTING_REG,
            ASCEND310P_TSV_MAIN_THREAD_DOTTING_REG,
            ASCEND310P_TSV_DOORBELL_DOTTING_REG, tsfw_reg);
    }

    return ret;
}

void tsdrv_heart_beat_tsfw_dfx_reg_uninit(struct tsdrv_heart_beat_tsfw_dfx_reg *tsfw_reg)
{
    if (tsfw_reg->heart_beart_dotting != NULL) {
        iounmap(tsfw_reg->heart_beart_dotting);
        tsfw_reg->heart_beart_dotting = NULL;
    }

    if (tsfw_reg->main_thread_dotting != NULL) {
        iounmap(tsfw_reg->main_thread_dotting);
        tsfw_reg->main_thread_dotting = NULL;
    }

    if (tsfw_reg->doorbell_dotting != NULL) {
        iounmap(tsfw_reg->doorbell_dotting);
        tsfw_reg->doorbell_dotting = NULL;
    }
}
#endif

#ifdef AOS_LLVM_BUILD
STATIC void tsdrv_update_cpu_info(u32 aicpu_heart_beat_exception,
                                  struct devdrv_info *info, u32 *cpu_bitmap, u32 *cpu_count)
{
    u32 i;
    u32 bitmap_tmp = *cpu_bitmap;
    u32 count_tmp = *cpu_count;
    for (i = 0; i < info->ai_cpu_core_num; i++) {
        if (aicpu_heart_beat_exception & (0x01U << i)) {
            bitmap_tmp |= (0x01U << i);
            if (!(info->inuse.ai_cpu_error_bitmap & (0x01U << i))) {
                TSDRV_PRINT_ERR("receive TS message ai cpu: %u heart beat exception.\n", i);
            }
        } else {
            count_tmp++;
        }
    }
    *cpu_bitmap = bitmap_tmp;
    *cpu_count = count_tmp;
}
STATIC void tsdrv_update_core_info(u32 aicore_bitmap, struct devdrv_info *info, u32 *core_bitmap, u32 *core_count)
{
    u32 i, aicore_num_max;
    u32 bitmap_tmp = *core_bitmap;
    u32 count_tmp = *core_count;
    aicore_num_max = info->inuse.ai_core_num + hweight32(info->inuse.ai_core_error_bitmap);
    if (aicore_num_max > BITS_PER_BYTE * sizeof(u32)) {
        TSDRV_PRINT_ERR("Aicore num and error bitmap not valid. (dev_id=%u; num=%u; err_bitmap=0x%x)\n",
            info->dev_id, info->inuse.ai_core_num, info->inuse.ai_core_error_bitmap);
        aicore_num_max = BITS_PER_BYTE * sizeof(u32);
    }

    for (i = 0; i < aicore_num_max; i++) {
        if (aicore_bitmap & (0x01U << i)) {
            bitmap_tmp |= (0x01U << i);
            if (!(info->inuse.ai_core_error_bitmap & (0x01U << i))) {
                TSDRV_PRINT_ERR("receive TS message ai core: %u exception.\n", i);
            }
        } else {
            count_tmp++;
        }
    }
    *core_bitmap = bitmap_tmp;
    *core_count = count_tmp;
}


STATIC void devdrv_heart_beat_ai_down_local(struct devdrv_info *info, u32 tsid, const void *data)
{
    const struct devdrv_heart_beat_cq *cq = NULL;
    u32 core_bitmap;
    u32 core_count;
    u32 cpu_bitmap;
    u32 cpu_count;

    cq = (struct devdrv_heart_beat_cq *)data;
    cpu_bitmap = 0;
    cpu_count = 0;
    core_bitmap = 0;
    core_count = 0;
    if (tsid >= DEVDRV_MAX_TS_NUM) {
        TSDRV_PRINT_ERR("invalid tsid=%u.\ns", tsid);
        return;
    }

    if (cq->aicpu_heart_beat_exception) {
        tsdrv_update_cpu_info(cq->aicpu_heart_beat_exception, info, &cpu_bitmap, &cpu_count);
    }

    if (cq->aicore_bitmap) {
        tsdrv_update_core_info(cq->aicore_bitmap, info, &core_bitmap, &core_count);
    }

    if (cq->syspcie_sysdma_status & 0xFFFF) {
        TSDRV_PRINT_ERR("ts sysdma is broken.\n");
        info->ai_subsys_ip_broken_map |= (0x01U << DEVDRV_AI_SUBSYS_SDMA_WORKING_STATUS_OFFSET);
    }
    /* check the high 16bits of syspcie_sysdma_status */
    if ((cq->syspcie_sysdma_status >> 16) & 0xFFFF) {
        TSDRV_PRINT_ERR("ts syspcie is broken.\n");
        info->ai_subsys_ip_broken_map |= (0x01U << DEVDRV_AI_SUBSYS_SPCIE_WORKING_STATUS_OFFSET);
    }

    info->inuse.ai_cpu_num = cpu_count;
    info->inuse.ai_cpu_error_bitmap = cpu_bitmap;
    info->inuse.ai_core_num = core_count;
    info->inuse.ai_core_error_bitmap = core_bitmap;
}

void tsdrv_heart_beat_ai_down_local(u32 devid, u32 tsid, const void *data)
{
    struct devdrv_info *info = NULL;

    info = dms_get_devinfo(devid);
    if (info == NULL) {
        TSDRV_PRINT_ERR("dev_info[%u] is NULL.\n", devid);
        return;
    }
    devdrv_heart_beat_ai_down_local(info, tsid, data);
}
#endif

void tsdrv_hb_cq_callback(u32 devid, u32 tsid, const u8 *cq_slot, u8 *sq_slot)
{
#ifndef TSDRV_UT
    struct tsdrv_heart_beat_info *heart_beat = NULL;
    struct devdrv_heart_beat_cq *cq = NULL;

    TSDRV_PRINT_DEBUG("tsdrv_hb_cq_callback start.\n");
    if (cq_slot == NULL) {
        TSDRV_PRINT_ERR("[devid-%u][tsid-%u] cq slot is NULL.\n", devid, tsid);
        return;
    }
    cq = (struct devdrv_heart_beat_cq *)cq_slot;

    if (devid >= TSDRV_MAX_DAVINCI_NUM || tsid >= DEVDRV_MAX_TS_NUM) {
        TSDRV_PRINT_ERR("[devid-%u][tsid-%u] the id is invalid.\n", devid, tsid);
        return;
    }

    if (cq->report_type != 0) {
        TSDRV_PRINT_ERR("receive ts exception msg, call excep_code=0x%x, time=%lld.%lds, devid=%u tsid=%u \n",
            cq->exception_code, cq->exception_time.tv_sec, cq->exception_time.tv_nsec, devid, tsid);
        tsdrv_bbox_system_err_time(devid, cq->exception_code, &cq->exception_time, 0);
    }

    heart_beat = tsdrv_hb_get_heart_beat_info(devid, tsid);
    heart_beat->resp_msg.sn = cq->number;

#ifdef CFG_FEATURE_TS_HB_DFX
    if (cq->ts_status != 0) {
        tsdrv_hb_tsfw_dfx_reg_show(cq->ts_status, heart_beat, TS_STATUS_ERR_SHOW);
    }
#endif
    if (cq->ts_status || cq->syspcie_sysdma_status || cq->aicpu_heart_beat_exception || cq->aicore_bitmap) {
#ifndef AOS_LLVM_BUILD
        tsdrv_heart_beat_ai_down(devid, tsid, cq_slot);
#else
        tsdrv_heart_beat_ai_down_local(devid, tsid, cq_slot);
#endif
    }
#endif
}

void tsdrv_hb_cq_callback_mini(u32 device_id, u32 tsid, const u8 *cq_slot, u8 *sq_slot)
{
    tsdrv_hb_cq_callback(0, 0, cq_slot, sq_slot);
}

STATIC u64 tsdrv_hb_read_cntpct(void)
{
    u64 cntpct = 0;

#ifdef __aarch64__
    asm volatile("mrs %0, cntpct_el0" : "=r"(cntpct));
#endif
    return cntpct;
}

struct tsdrv_heart_beat_info *tsdrv_hb_get_heart_beat_info(u32 devid, u32 tsid)
{
    return &heart_beat_info[devid][tsid];
}

#if defined(CFG_SOC_PLATFORM_MINIV2)
#define MNTN_TS0_HEART_BEAT_LOST_CODE 0xA6060FFFU
#define MNTN_TS1_HEART_BEAT_LOST_CODE 0xA6061FFFU
#else
#define MNTN_TS0_HEART_BEAT_LOST_CODE 0xA607FFFFU
#endif

#if (defined CFG_SOC_PLATFORM_MINIV2) || ((defined CFG_SOC_PLATFORM_MINIV3) || (defined CFG_SOC_PLATFORM_CLOUD_V2))
static void tsdrv_hb_notice_bbox(struct ts_instance *instance)
{
    struct timespec64 os_time;
    u32 code;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
    ktime_get_coarse_real_ts64(&os_time);
#else
    os_time = current_kernel_time64();
#endif

#ifndef TSDRV_UT
#if defined(CFG_SOC_PLATFORM_MINIV2)
    code = (instance->tsid == 0) ? MNTN_TS0_HEART_BEAT_LOST_CODE : MNTN_TS1_HEART_BEAT_LOST_CODE;
#else
    code = MNTN_TS0_HEART_BEAT_LOST_CODE;
#endif
#endif
    tsdrv_bbox_system_err_time(instance->devid, code, &os_time, 0);
}
#endif

STATIC bool tsdrv_hb_is_ready(struct tsdrv_heart_beat_info *heart_beat)
{
    if ((tsdrv_is_ts_work(heart_beat->instance.devid, heart_beat->instance.tsid) == false) ||
        (heart_beat->init_state == TSDRV_HEARTBEAT_UNINIT) ||
        (atomic_read(&heart_beat->work_state) == TSDRV_HEARTBEAT_STOP)) {
        return false;
    }
    return true;
}

STATIC void tsdrv_hb_ts_down(struct tsdrv_heart_beat_info *heart_beat)
{
    struct devdrv_info *dev_info = NULL;

    TSDRV_PRINT_ERR("[devid-%u][tsid-%u] heart beat exception is detected, process ts down exception.\n",
        heart_beat->instance.devid, heart_beat->instance.tsid);

#if (defined CFG_SOC_PLATFORM_MINIV2) || ((defined CFG_SOC_PLATFORM_MINIV3) || (defined CFG_SOC_PLATFORM_CLOUD_V2))
    tsdrv_hb_notice_bbox(&heart_beat->instance);
#endif

    (void)tsdrv_heartbeat_fault_event_add(heart_beat->instance.devid, heart_beat->instance.tsid);
#ifndef AOS_LLVM_BUILD
    dev_info = devdrv_manager_get_devdrv_info(heart_beat->instance.devid);
#else
    dev_info = dms_get_devinfo(heart_beat->instance.devid);
#endif
    if (dev_info == NULL) {
        TSDRV_PRINT_ERR("[devid-%u] the device has not been registered.\n", heart_beat->instance.devid);
        return;
    }
    tsdrv_set_ts_status(heart_beat->instance.devid, heart_beat->instance.tsid, TS_DOWN);

#ifndef AOS_LLVM_BUILD
    devdrv_ts_exception_task((unsigned long)(uintptr_t)dev_info);
#endif
}

STATIC void tsdrv_hb_pack_msg(struct devdrv_heart_beat_sq *sq, struct ts_instance *instance, u32 count)
{
    struct timespec64 now, interval;
    struct timespec64 wall;

    ktime_get_coarse_real_ts64(&wall);
    ktime_get_raw_ts64(&now);

    sq->number = count;
    sq->devid = instance->devid;
    sq->cmd = DEVDRV_HEART_BEAT_SQ_CMD;
    sq->stamp = now;
    sq->wall = wall;
    sq->cntpct = tsdrv_hb_read_cntpct();
#ifndef TSDRV_UT
    dms_get_time_interval(&interval);
#endif
    sq->time_zone_interval = interval.tv_sec;
    return;
}

STATIC void tsdrv_hb_lost_proc(struct tsdrv_heart_beat_info *heart_beat)
{
    if (tsdrv_hb_is_ready(heart_beat) == false) {
        heart_beat->lost_count = 0;
        return;
    }

    heart_beat->lost_count++;
#ifdef CFG_FEATURE_TS_HB_DFX
    tsdrv_hb_tsfw_dfx_reg_show(0, heart_beat, TS_HEART_BEAT_LOST_SHOW);
#endif
    if (heart_beat->lost_count >= TSDRV_HEARTBEAT_MAX_LOST_COUNT) {
        tsdrv_hb_ts_down(heart_beat);
    }
}

STATIC void tsdrv_hb_send_msg(struct tsdrv_heart_beat_info *heart_beat, const u8 *msg)
{
    int ret;

    ret = devdrv_functional_send_sq(heart_beat->instance.devid, heart_beat->instance.tsid,
        heart_beat->msg_chan.sq_index, msg, sizeof(struct devdrv_heart_beat_sq));
    if (ret != 0) {
        TSDRV_PRINT_ERR("[devid-%u][tsid-%u] send heart beat msg failed.\n", heart_beat->instance.devid,
            heart_beat->instance.tsid);
    }
}

STATIC int tsdrv_hb_thread_fun(void *data)
{
    struct tsdrv_heart_beat_info *heart_beat = (struct tsdrv_heart_beat_info *)data;
    struct devdrv_heart_beat_sq sq;

    heart_beat->msg_sn = 0;
    while (!kthread_should_stop()) {
        if (tsdrv_hb_is_ready(heart_beat) == false) {
            msleep(g_cycle_time);
            continue;
        }
        heart_beat->msg_sn++;
        tsdrv_hb_pack_msg(&sq, &heart_beat->instance, heart_beat->msg_sn);

        tsdrv_hb_send_msg(heart_beat, (u8 *)&sq);

        msleep(g_cycle_time);

        if (heart_beat->resp_msg.sn == heart_beat->msg_sn) {
            heart_beat->lost_count = 0;
        } else {
            tsdrv_hb_lost_proc(heart_beat);
        }
        TSDRV_PRINT_DEBUG("[devid-%u][tsid-%u] send heart beat finished. lost count=%u\n",
                          heart_beat->instance.devid, heart_beat->instance.tsid, (u32)heart_beat->lost_count);
    }
    return 0;
}

STATIC int tsdrv_hb_create_and_run_thread(struct tsdrv_heart_beat_info *heart_beat)
{
    struct task_struct *tsk = NULL;

    tsk = kthread_create(tsdrv_hb_thread_fun, heart_beat, "ts_hb_dev%u_ts%u", heart_beat->instance.devid,
        heart_beat->instance.tsid);
    if (IS_ERR(tsk)) {
        TSDRV_PRINT_ERR("[devid-%u][tsid-%u] create heart beat thread failed.\n", heart_beat->instance.devid,
            heart_beat->instance.tsid);
        return -EINVAL;
    }

    tsdrv_bind_cpu(tsk);
    (void)wake_up_process(tsk);
    heart_beat->tsk = tsk;

    return 0;
}


STATIC void tsdrv_hb_destroy_functional_sqcq(struct tsdrv_heart_beat_info *heart_beat)
{
    atomic_set(&heart_beat->work_state, TSDRV_HEARTBEAT_STOP);
    heart_beat->init_state = TSDRV_HEARTBEAT_UNINIT;
    heart_beat->lost_count = 0;
    devdrv_destroy_functional_sq(heart_beat->instance.devid, heart_beat->instance.tsid, heart_beat->msg_chan.sq_index);
    devdrv_destroy_functional_cq(heart_beat->instance.devid, heart_beat->instance.tsid, heart_beat->msg_chan.cq_index);
    heart_beat->msg_chan.sq_index = DEVDRV_MAX_FUNCTIONAL_SQ_NUM;
    heart_beat->msg_chan.cq_index = DEVDRV_MAX_FUNCTIONAL_CQ_NUM;
}

STATIC int tsdrv_hb_send_chan_info_to_ts(struct tsdrv_heart_beat_info *heart_beat, u64 sq_addr,
    u64 cq_addr, u16 cmd_type)
{
    struct devdrv_mailbox_cqsq cqsq;
    int ret;
    u32 devid, tsid;
    struct tsdrv_mbox_data data;

    devid = heart_beat->instance.devid;
    tsid = heart_beat->instance.tsid;

    cqsq.cq_irq = (u16)tsdrv_get_dfx_cq_irq_vector(devid, tsid);
    cqsq.cmd_type = cmd_type;
    cqsq.valid = DEVDRV_MAILBOX_MESSAGE_VALID;
    cqsq.result = 0;
    cqsq.sq_index = heart_beat->msg_chan.sq_index;
    cqsq.cq0_index = heart_beat->msg_chan.cq_index;
    cqsq.sq_addr = sq_addr;
    cqsq.cq0_addr = cq_addr;
    cqsq.plat_type = DEVDRV_MANAGER_DEVICE_ENV;

    data.msg = &cqsq;
    data.msg_len = sizeof(struct devdrv_mailbox_cqsq);
    data.out_data = NULL;
    data.out_len = 0;

    ret = tsdrv_mailbox_send_sync(heart_beat->instance.devid, heart_beat->instance.tsid, &data);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Send mailbox sync failed. (devid=%u; tsid=%u; cmd_type=%u)\n",
            heart_beat->instance.devid, heart_beat->instance.tsid, (u32)cmd_type);
        return -EINVAL;
    }

    return 0;
}

STATIC int tsdrv_hb_create_functional_sqcq(struct tsdrv_heart_beat_info *heart_beat, u64 *sq_addr,
    u64 *cq_addr)
{
    u32 sq_index, cq_index;
    u32 devid, tsid;
    int ret;

    devid = heart_beat->instance.devid;
    tsid = heart_beat->instance.tsid;
    ret = devdrv_create_functional_sq(devid, tsid, DEVDRV_FUNCTIONAl_MAX_SQ_SLOT_LEN, &sq_index, sq_addr);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("[devid-%u][tsid-%u] create_functional_sq failed.\n", devid, tsid);
        return -ENOMEM;
#endif
    }
#if (defined CFG_SOC_PLATFORM_MINIV2) || (defined CFG_SOC_PLATFORM_CLOUD)
    ret = devdrv_create_functional_cq(devid, tsid, DEVDRV_FUNCTIONAL_DETAILED_CQ_LENGTH, DEVDRV_FUNCTIONAL_DETAILED_CQ,
        tsdrv_hb_cq_callback, &cq_index, cq_addr);
#else
    ret = devdrv_create_functional_cq(devid, tsid, DEVDRV_FUNCTIONAL_DETAILED_CQ_LENGTH, DEVDRV_FUNCTIONAL_DETAILED_CQ,
        tsdrv_hb_cq_callback_mini, &cq_index, cq_addr);
#endif
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("[devid-%u][tsid-%u] create_functional_cq failed.\n", devid, tsid);
        devdrv_destroy_functional_sq(devid, tsid, sq_index);
        return -ENOMEM;
#endif
    }

    heart_beat->msg_chan.sq_index = sq_index;
    heart_beat->msg_chan.cq_index = cq_index;

    ret = devdrv_functional_set_cq_func(devid, tsid, cq_index, DEVDRV_CQSQ_HEART_BEAT);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("[devid-%u][tsid-%u] functional_set_cq_func failed.\n", devid, tsid);
        goto init_fail;
#endif
    }

    ret = devdrv_functional_set_sq_func(devid, tsid, sq_index, DEVDRV_CQSQ_HEART_BEAT);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("[devid-%u][tsid-%u] functional_set_sq_func failed.\n", devid, tsid);
        goto init_fail;
#endif
    }

    return 0;
#ifndef TSDRV_UT
init_fail:
    tsdrv_hb_destroy_functional_sqcq(heart_beat);

    return -ENODEV;
#endif
}

STATIC int tsdrv_hb_init_per_ts(struct tsdrv_heart_beat_info *heart_beat)
{
    u64 sq_addr, cq_addr;
    int ret;

    TSDRV_PRINT_INFO("[devid-%u][tsid-%u] heart beat init start.\n", heart_beat->instance.devid,
        heart_beat->instance.tsid);

    ret = tsdrv_hb_create_functional_sqcq(heart_beat, &sq_addr, &cq_addr);
    if (ret != 0) {
        return -ENODEV;
    }

    ret = tsdrv_hb_send_chan_info_to_ts(heart_beat, sq_addr, cq_addr, DEVDRV_MAILBOX_CREATE_CQSQ_BEAT);
    if (ret != 0) {
        goto init_fail;
    }

    heart_beat->lost_count = 0;
    heart_beat->init_state = TSDRV_HEARTBEAT_INITED;
    atomic_set(&heart_beat->work_state, TSDRV_HEARTBEAT_WORK);

    ret = tsdrv_hb_create_and_run_thread(heart_beat);
    if (ret != 0) {
        goto thread_fail;
    }

    return 0;
thread_fail:
    (void)tsdrv_hb_send_chan_info_to_ts(heart_beat, 0, 0, DEVDRV_MAILBOX_RELEASE_CQSQ_BEAT);
init_fail:
    tsdrv_hb_destroy_functional_sqcq(heart_beat);

    return -ENODEV;
}

STATIC void tsdrv_hb_exit_per_ts(struct tsdrv_heart_beat_info *heart_beat)
{
    TSDRV_PRINT_INFO("[devid-%u][tsid-%u] heart beat exit start.\n", heart_beat->instance.devid,
        heart_beat->instance.tsid);

    if (!IS_ERR_OR_NULL(heart_beat->tsk)) {
        (void)kthread_stop(heart_beat->tsk);
    }
    heart_beat->tsk = NULL;

    if ((heart_beat->msg_chan.sq_index >= DEVDRV_MAX_FUNCTIONAL_SQ_NUM) ||
        (heart_beat->msg_chan.cq_index >= DEVDRV_MAX_FUNCTIONAL_CQ_NUM)) {
        return;
    }

    (void)tsdrv_hb_send_chan_info_to_ts(heart_beat, 0, 0, DEVDRV_MAILBOX_RELEASE_CQSQ_BEAT);
    tsdrv_hb_destroy_functional_sqcq(heart_beat);

    return;
}

int tsdrv_heart_beat_init(u32 devid)
{
    struct tsdrv_heart_beat_info *heart_beat = NULL;
    u32 i, j, tsnum;
    int ret;

    if (devid >= TSDRV_MAX_DAVINCI_NUM) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("[devid-%u] devid is invalid.\n", devid);
        return -EINVAL;
#endif
    }
    tsnum = tsdrv_get_ts_num(devid);
    if ((tsnum > DEVDRV_MAX_TS_NUM) || (tsnum == 0)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("[devid-%u] tsnum = %u is invalid.\n", devid, tsnum);
        return -EINVAL;
#endif
    }

    for (i = 0; i < tsnum; i++) {
        heart_beat = tsdrv_hb_get_heart_beat_info(devid, i);
        heart_beat->instance.devid = devid;
        heart_beat->instance.tsid = i;
#ifdef CFG_FEATURE_TS_HB_DFX
        ret = tsdrv_heart_beat_tsfw_dfx_reg_init(devid, i, &heart_beat->tsfw_reg);
        if (ret != 0) {
            TSDRV_PRINT_ERR("[devid-%u][tsid-%u] tsfw dfx reg init failed, all ts run without heart beat.\n",
                devid, i);
            goto heart_beat_exit;
        }
#endif
        mutex_init(&heart_beat->lock);
        mutex_lock(&heart_beat->lock);
        ret = tsdrv_hb_init_per_ts(heart_beat);
        mutex_unlock(&heart_beat->lock);
        if (ret != 0) {
#ifdef CFG_FEATURE_TS_HB_DFX
            tsdrv_heart_beat_tsfw_dfx_reg_uninit(&heart_beat->tsfw_reg);
#endif
            TSDRV_PRINT_ERR("[devid-%u][tsid-%u] heart beat init failed, all ts run without heart beat.\n", devid, i);
            goto heart_beat_exit;
        }
    }

    return 0;

heart_beat_exit:
    for (j = 0; j < i; j++) {
        heart_beat = tsdrv_hb_get_heart_beat_info(devid, j);
        tsdrv_hb_exit_per_ts(heart_beat);
#ifdef CFG_FEATURE_TS_HB_DFX
        tsdrv_heart_beat_tsfw_dfx_reg_uninit(&heart_beat->tsfw_reg);
#endif
    }

    return -ENODEV;
}
EXPORT_SYMBOL(tsdrv_heart_beat_init);

void tsdrv_heart_beat_exit(u32 devid)
{
    struct tsdrv_heart_beat_info *heart_beat = NULL;
    u32 tsnum, tsid;

    if (devid >= TSDRV_MAX_DAVINCI_NUM) {
        TSDRV_PRINT_ERR("[devid-%u] devid is invalid.\n", devid);
        return;
    }

    tsnum = tsdrv_get_ts_num(devid);
    if ((tsnum > DEVDRV_MAX_TS_NUM) || (tsnum == 0)) {
        TSDRV_PRINT_ERR("[devid-%u] tsnum = %u is invalid.\n", devid, tsnum);
        return;
    }

    for (tsid = 0; tsid < tsnum; tsid++) {
        heart_beat = tsdrv_hb_get_heart_beat_info(devid, tsid);
        mutex_lock(&heart_beat->lock);
        tsdrv_hb_exit_per_ts(heart_beat);
        mutex_unlock(&heart_beat->lock);
#ifdef CFG_FEATURE_TS_HB_DFX
        tsdrv_heart_beat_tsfw_dfx_reg_uninit(&heart_beat->tsfw_reg);
#endif
    }
}
EXPORT_SYMBOL(tsdrv_heart_beat_exit);

void tsdrv_heart_beat_set_work_state(u32 devid, u32 tsid, u8 state)
{
    struct tsdrv_heart_beat_info *heart_beat = NULL;

    if (devid >= TSDRV_MAX_DAVINCI_NUM || tsid >= DEVDRV_MAX_TS_NUM) {
        TSDRV_PRINT_ERR("[devid-%u][tsid-%u] input id is invalid.\n", devid, tsid);
        return;
    }

    heart_beat = tsdrv_hb_get_heart_beat_info(devid, tsid);
    state = (state == TSDRV_HEARTBEAT_STOP) ? TSDRV_HEARTBEAT_STOP : TSDRV_HEARTBEAT_WORK;
    atomic_set(&heart_beat->work_state, state);
}
EXPORT_SYMBOL(tsdrv_heart_beat_set_work_state);

