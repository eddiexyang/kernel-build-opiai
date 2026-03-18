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
#ifndef PROF_UNIT_TEST

#include <linux/kthread.h>
#ifdef AOS_LLVM_BUILD
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/atomic.h>
#include <linux/hrtimer.h>
#include <asm-generic/getorder.h>
#endif

#ifdef CFG_SOC_PLATFORM_MDC_V51
#include <linux/cpumask.h>
#include "drv_cpu_type.h"
#endif

#include "tsdrv_common.h"
#include "prof.h"

#include "prof_drv_dev.h"

int prof_hash[CHANNEL_IDS_MAX] = {
#ifdef CFG_SOC_PLATFORM_MINIV2
    [CHANNEL_PCIE] = PROF_VALID_PERIPHERAL_FLAG,
#endif
    [CHANNEL_DDR] = PROF_VALID_PERIPHERAL_FLAG,
    [CHANNEL_LLC] = PROF_VALID_PERIPHERAL_FLAG,
    [CHANNEL_DVPP_VENC] = PROF_VALID_PERIPHERAL_VIRTUAL_FLAG,
    [CHANNEL_DVPP_JPEGE] = PROF_VALID_PERIPHERAL_VIRTUAL_FLAG,
    [CHANNEL_DVPP_VDEC] = PROF_VALID_PERIPHERAL_VIRTUAL_FLAG,
    [CHANNEL_DVPP_JPEGD] = PROF_VALID_PERIPHERAL_VIRTUAL_FLAG,
    [CHANNEL_DVPP_VPC] = PROF_VALID_PERIPHERAL_VIRTUAL_FLAG,

    [CHANNEL_TSCPU] = PROF_VALID_TSCPU_FLAG,
    [CHANNEL_AICORE] = PROF_VALID_TSCPU_HW_VIRTUAL_FLAG,
    [CHANNEL_TSFW] = PROF_VALID_TSCPU_SW_VIRTUAL_FLAG,
    [CHANNEL_HWTS_LOG] = PROF_VALID_TSCPU_HW_VIRTUAL_FLAG,
    [CHANNEL_KEY_POINT] = PROF_VALID_TSCPU_SW_VIRTUAL_FLAG,
    [CHANNEL_TSFW_L2] = PROF_VALID_TSCPU_FLAG,
#ifdef CFG_SOC_PLATFORM_MDC_V51
    [CHANNEL_AIV] = PROF_VALID_TSCPU_FLAG,
    [CHANNEL_TSFW1] = PROF_VALID_TSCPU_FLAG,
    [CHANNEL_HWTS_LOG1] = PROF_VALID_TSCPU_FLAG,
#else
    [CHANNEL_NPU_APP_MEM] = PROF_VALID_PERIPHERAL_VIRTUAL_FLAG,
    [CHANNEL_NPU_MEM] = PROF_VALID_PERIPHERAL_VIRTUAL_FLAG,
#endif
};

char *prof_peri_cb_regs[CHANNEL_IDS_MAX][PROF_PERI_SAMPLE_MODE_MAX] = {
    [CHANNEL_PCIE][PROF_PERI_SAMPLE] = PROF_PCIE_SAMPLE_FUNC_NAME,
    [CHANNEL_PCIE][PROF_PERI_SAMPLE_USERDATA] = PROF_PCIE_USERDATA_FUNC_NAME,
    [CHANNEL_PCIE][PROF_PERI_SAMPLE_STOP] = PROF_PCIE_STOP_FUNC_NAME,
    [CHANNEL_DDR][PROF_PERI_SAMPLE] = PROF_DDR_SAMPLE_FUNC_NAME,
    [CHANNEL_DDR][PROF_PERI_SAMPLE_USERDATA] = PROF_DDR_USERDATA_FUNC_NAME,
    [CHANNEL_LLC][PROF_PERI_SAMPLE] = PROF_LLC_SAMPLE_FUNC_NAME,
    [CHANNEL_LLC][PROF_PERI_SAMPLE_USERDATA] = PROF_LLC_USERDATA_FUNC_NAME,
    [CHANNEL_LLC][PROF_PERI_SAMPLE_STOP] = PROF_LLC_STOP_FUNC_NAME,
    [CHANNEL_DVPP_VENC][PROF_PERI_SAMPLE] = PROF_DVPP_VENC_SAMPLE_FUNC_NAME,
    [CHANNEL_DVPP_JPEGE][PROF_PERI_SAMPLE] = PROF_DVPP_JPEGE_SAMPLE_FUNC_NAME,
    [CHANNEL_DVPP_VDEC][PROF_PERI_SAMPLE] = PROF_DVPP_VDEC_SAMPLE_FUNC_NAME,
    [CHANNEL_DVPP_JPEGD][PROF_PERI_SAMPLE] = PROF_DVPP_JPEGD_SAMPLE_FUNC_NAME,
    [CHANNEL_DVPP_VPC][PROF_PERI_SAMPLE] = PROF_DVPP_VPC_SAMPLE_FUNC_NAME,
#ifndef CFG_SOC_PLATFORM_MDC_V51
    [CHANNEL_NPU_APP_MEM][PROF_PERI_SAMPLE] = PROF_NPU_APP_SAMPLE_FUNC_NAME,
    [CHANNEL_NPU_APP_MEM][PROF_PERI_SAMPLE_USERDATA] = PROF_NPU_APP_USERDATA_FUNC_NAME,
    [CHANNEL_NPU_MEM][PROF_PERI_SAMPLE] = PROF_NPU_SAMPLE_FUNC_NAME,
    [CHANNEL_NPU_MEM][PROF_PERI_SAMPLE_USERDATA] = PROF_NPU_USERDATA_FUNC_NAME,
#endif
};

int* prof_get_hash(void)
{
    return prof_hash;
}

char *prof_get_peri_cb_regs_handler(u32 channel_id, u32 mode)
{
    return prof_peri_cb_regs[channel_id][mode];
}

u32 prof_sq_init_get_buf_num(u32 channel_id, u32 vfid)
{
#if defined(CFG_SOC_PLATFORM_MDC_V51)
    switch (channel_id) {
        case CHANNEL_AICORE:
        case CHANNEL_AIV:
        case CHANNEL_HWTS_LOG:
        case CHANNEL_HWTS_LOG1:
            return PROF_GROUP_NUM_MAX;
        default:
            return 1;
    }
#else
    if (vfid != PROF_PHYSICAL_MACHINE_VFID && prof_is_valid_virtual_tscpu_hw_channel(channel_id)) {
        return PROF_CHANNEL_HW_NUM;
    }

    return 1;
#endif
}

STATIC u32 prof_tscpu_get_group_num(u32 dev_id, u32 ch_id)
{
    u32 ts_num = prof_device_info_get(dev_id)->ts_num;

    switch (ch_id) {
        case CHANNEL_AICORE:
        case CHANNEL_HWTS_LOG:
            if (ts_num == DC_TSNUM) {
                return 1;
            } else {
                return PROF_GROUP_NUM_MAX;
            }
        case CHANNEL_HWTS_LOG1:
        case CHANNEL_AIV:
            return PROF_GROUP_NUM_MAX;
        default:
            return 1;
    }
}

u32 prof_get_buff_data_len(u32 device_id, u32 channel_id, u32 vfid, u32 sub_channel_id)
{
    struct prof_sub_channel_info *sub_channel_info =
        prof_get_sub_channel_info_from_index(device_id, vfid, channel_id, sub_channel_id);
    u32 group_num = prof_tscpu_get_group_num(device_id, channel_id);
    u32 len;

    if (group_num > 1) {
        len = PROF_TS_BUF_ONE_GROUP_SIZE - BUFF_HEAD_SIZE;
    } else {
        len = sub_channel_info->buf_len - BUFF_HEAD_SIZE;
    }

    prof_debug("Profile received subordinate channel information."
        " (device_id=%u; vfid=%u; channel_id=%d; data_len=0x%x)\n",
        device_id, vfid, channel_id, len);
    return len;
}

u32 prof_sq_init_get_buf_len(struct prof_sub_channel_info *sub_channel_info)
{
    u32 memory_type = prof_get_mem_type(sub_channel_info->channel_id);
    u32 ts_num = prof_device_info_get(sub_channel_info->device_id)->ts_num;
    u32 buf_len;

    if (memory_type == MEM_RESERVE && ts_num == DC_TSNUM &&
        sub_channel_info->vfid != PROF_PHYSICAL_MACHINE_VFID) {
        buf_len = PROF_TS_BUF_VIR_TOTAL_SIZE;
    } else {
        buf_len = sub_channel_info->buf_len;
    }

    return buf_len;
}

int prof_data_flush_support_check(u32 channel_id)
{
    struct prof_device_info *prof_device = prof_device_info_get(0);
    u32 ts_num = prof_device->ts_num;

    if ((channel_id == CHANNEL_HWTS_LOG) || (channel_id == CHANNEL_TSFW)) {
        return PROF_OK;
    } else if (((channel_id == CHANNEL_HWTS_LOG1) || (channel_id == CHANNEL_TSFW1)) &&
        (ts_num == MDC_TSNUM)) {
        return PROF_OK;
    }

    return PROF_NOT_SUPPORT;
}

u32 prof_get_mem_type(int channel_id)
{
    u32 memory_type;

    switch (channel_id) {
        case CHANNEL_AICORE:
        case CHANNEL_HWTS_LOG:
        case CHANNEL_HWTS_LOG1:
        case CHANNEL_AIV:
        case CHANNEL_TSFW1:
            memory_type = MEM_RESERVE;
            break;
        case CHANNEL_TSCPU:
        case CHANNEL_KEY_POINT:
        case CHANNEL_TSFW_L2:
        case CHANNEL_TSFW:
#ifdef CFG_SOC_PLATFORM_MDC_V51
            memory_type = MEM_RESERVE;
#else
            memory_type = MEM_MALLOC;
#endif
            break;
        default:
            memory_type = MEM_MALLOC;
            break;
    }
    return memory_type;
}

void prof_init_tsid(struct prof_sub_channel_info *sub_channel_info)
{
#ifdef CFG_SOC_PLATFORM_MDC_V51
    if ((sub_channel_info->channel_id == CHANNEL_TSFW1) ||
        (sub_channel_info->channel_id == CHANNEL_HWTS_LOG1) ||
        (sub_channel_info->channel_id == CHANNEL_AIV)) {
        sub_channel_info->ts_channel.tsid = 1;
        return;
    }
#endif
    sub_channel_info->ts_channel.tsid = 0;
}

#ifdef CFG_SOC_PLATFORM_MDC_V51
STATIC u64 prof_get_mdc_reserved_mem(u32 device_id, u32 channel_id)
{
    u64 ret_addr = 0;
    switch (channel_id) {
        case CHANNEL_AICORE:
            ret_addr = PROF_MDC_AICORE_RESERVED_MEM_ADDR;
            break;
        case CHANNEL_HWTS_LOG:
            ret_addr = PROF_MDC_HWTS0_RESERVED_MEM_ADDR;
            break;
        case CHANNEL_HWTS_LOG1:
            ret_addr = PROF_MDC_HWTS1_RESERVED_MEM_ADDR;
            break;
        case CHANNEL_AIV:
            ret_addr = PROF_MDC_AIV_RESERVED_MEM_ADDR;
            break;
        case CHANNEL_TSCPU:
            ret_addr = PROF_MDC_TSCPU_RESERVED_MEM_ADDR;
            break;
        case CHANNEL_TSFW:
            ret_addr = PROF_MDC_TSFW_RESERVED_MEM_ADDR;
            break;
        case CHANNEL_TSFW1:
            ret_addr = PROF_MDC_TSFW1_RESERVED_MEM_ADDR;
            break;
        case CHANNEL_KEY_POINT:
            ret_addr = PROF_MDC_KEY_POINT_RESERVED_MEM_ADDR;
            break;
        case CHANNEL_TSFW_L2:
            ret_addr = PROF_MDC_TSFW_L2_RESERVED_MEM_ADDR;
            break;
        default:
            break;
    }
    return ret_addr;
}
#else
STATIC u64 prof_get_dc_reserved_mem(u32 device_id, u32 channel_id, u32 sub_ch_id)
{
    u64 ret_addr = 0;
    u32 offset = (sub_ch_id == 0) ? 0 : (sub_ch_id - 1);

    switch (channel_id) {
        case CHANNEL_AICORE:
            ret_addr = PROF_DC_AICORE_RESERVED_MEM_ADDR + offset * PROF_TS_BUF_VIR_ONE_ZONE_SIZE +
                (device_id * PROF_CHIP_ADDR_OFFSET);
            break;
        case CHANNEL_HWTS_LOG:
            ret_addr = PROF_DC_HWTS0_RESERVED_MEM_ADDR + offset * PROF_TS_BUF_VIR_ONE_ZONE_SIZE +
                (device_id * PROF_CHIP_ADDR_OFFSET);
            break;
        default:
            break;
    }
    return ret_addr;
}
#endif

u64 prof_sq_init_get_buf_pa(struct prof_sub_channel_info *sub_channel_info)
{
#ifdef CFG_SOC_PLATFORM_MDC_V51
    return sub_channel_info->phy_addr;
#else
    if (sub_channel_info->channel_id == CHANNEL_AICORE ||
        sub_channel_info->channel_id == CHANNEL_HWTS_LOG) {
        return prof_get_dc_reserved_mem(sub_channel_info->device_id,
            sub_channel_info->channel_id, 0);
    }
#endif

    return sub_channel_info->phy_addr;
}

u64 prof_get_reserved_mem(u32 device_id, u32 channel_id, u32 sub_ch_id)
{
    u64 ret_addr = 0;

#ifdef CFG_SOC_PLATFORM_MDC_V51
    ret_addr = prof_get_mdc_reserved_mem(device_id, channel_id);
#else
    ret_addr = prof_get_dc_reserved_mem(device_id, channel_id, sub_ch_id);
#endif

    prof_debug("Profile memory was reserved. (device_id=%u; channel_id=%u; phy_addr=%pK)\n",
        device_id, channel_id, (void *)(uintptr_t)ret_addr);
    return ret_addr;
}

int prof_channel_need_sync_rw_ptr(struct prof_sub_channel_info *sub_channel_info)
{
    int ret;
    switch (sub_channel_info->channel_id) {
        case CHANNEL_AICORE:
        case CHANNEL_HWTS_LOG:
        case CHANNEL_HWTS_LOG1:
        case CHANNEL_AIV:
            ret = PROF_OK;
            break;
        default:
            ret = PROF_ERROR;
            break;
    }
    return ret;
}
#ifdef CFG_SOC_PLATFORM_MDC_V51
cpumask_t g_prof_ctrlcpu;
void prof_init_ctrlcpu_mask(void)
{
    int ret;
    cpumask_clear(&g_prof_ctrlcpu);
    ret = drv_get_ctrlcpu_mask(&g_prof_ctrlcpu);
    if (ret != 0) {
        prof_err("Failed to get ctrl cpu mask. (ret=%d).\n", ret);
    }
}

int prof_init_affinity_cpuid_thread(u32 device_id, struct task_struct *thread)
{
    if (cpumask_empty(&g_prof_ctrlcpu)) {
        return PROF_OK;
    }
    if (cpumask_available(&g_prof_ctrlcpu)) {
        set_cpus_allowed_ptr(thread, &g_prof_ctrlcpu);
    }
    return PROF_OK;
}
#endif

STATIC u32 prof_tscpu_dc_mem_rev_buflen(u32 channel_id, u32 vfid)
{
    u32 buf_len;

    if (vfid == PROF_PHYSICAL_MACHINE_VFID) {
        if (channel_id == CHANNEL_AICORE) {
            buf_len = PROF_TS_BUF_PHY_AIC_BUF_SIZE;
        } else {
            buf_len = PROF_TS_BUFFER_LEN_MAX;
        }
    } else {
        buf_len = PROF_TS_BUF_VIR_ONE_ZONE_SIZE;
    }

    return buf_len;
}

u32 prof_tscpu_start_get_buf_len(u32 device_id, u32 channel_id, u32 vfid)
{
    u32 ts_num =  prof_device_info_get(device_id)->ts_num;
    u32 group_num = prof_tscpu_get_group_num(device_id, channel_id);
    u32 memory_type = prof_get_mem_type(channel_id);
    u32 buf_len;

    if ((ts_num == DC_TSNUM) && (memory_type == MEM_RESERVE)) {
        buf_len = prof_tscpu_dc_mem_rev_buflen(channel_id, vfid);
    } else if ((ts_num == MDC_TSNUM) && (group_num > 1)) {
        buf_len = PROF_TS_BUF_TOTAL_GROUP_SIZE;
    } else {
        buf_len = PROF_TS_BUFFER_LEN;
    }

    prof_debug("Profile received TS channel information. (channel_id=%u; buf_len=%u)",
        channel_id, buf_len);
    return buf_len;
}

STATIC void prof_tscpu_init_buff_head(struct prof_data_head *data_head, u32 buff_len)
{
    data_head->write_ptr = 0;
    data_head->read_ptr = 0;
    data_head->buf_len = buff_len;

#ifdef __aarch64__
    flush_cache((unsigned char *)data_head, BUFF_HEAD_SIZE);
#endif
}

int prof_tscpu_init_channel_info(struct prof_sub_channel_info *sub_channel_info)
{
    struct prof_data_head *data_head = NULL;
    u32 group_num = prof_tscpu_get_group_num(sub_channel_info->device_id,
        sub_channel_info->channel_id);
    u32 i;
    int ret;

    sub_channel_info->buf_len = prof_tscpu_start_get_buf_len(sub_channel_info->device_id,
        sub_channel_info->channel_id, sub_channel_info->vfid);
    if (sub_channel_info->vir_addr == NULL) {
        ret = prof_tscpu_alloc_channel_memory(sub_channel_info);
        if (ret != PROF_OK) {
            prof_err("Failed to allocate channel buffer for TS channel."
                " (devid=%u; vfid=%u; channel_id=%u; ret=%d)\n",
                sub_channel_info->device_id, sub_channel_info->vfid, sub_channel_info->channel_id, ret);
            return PROF_ERROR;
        }
    }

    PROF_CHANNEL_INFO_SHOW(sub_channel_info);

    for (i = 0; i < group_num; i++) {
        data_head = (struct prof_data_head *)(sub_channel_info->vir_addr + (u64)PROF_TS_BUF_ONE_GROUP_SIZE * i);
        prof_tscpu_init_buff_head(data_head, sub_channel_info->buf_len / group_num);
    }

    return PROF_OK;
}

int prof_alloc_all_tscpu_common_bufs(u32 device_id)
{
    return PROF_OK;
}

void prof_free_all_tscpu_common_bufs(u32 device_id)
{
}

#ifdef CFG_SOC_PLATFORM_MDC_V51
bool prof_hash_valid(u32 channel_id)
{
    if (channel_id >= CHANNEL_IDS_MAX) {
        return false;
    }
    if (prof_hash[channel_id] != 0) {
        return true;
    }
    return false;
}
#endif

bool prof_hash_channel_valid(u32 channel_id)
{
#ifdef CFG_SOC_PLATFORM_MDC_V51
    return prof_hash_valid(channel_id);
#else
    return prof_is_valid_channel(channel_id);
#endif
}

void prof_sub_resource_channel_state_init(struct prof_sub_channel_info *sub_resource_info)
{
    sub_resource_info->channel_state = (u32)PROF_CHANNEL_UNINIT;
    switch (sub_resource_info->channel_id) {
        case CHANNEL_AICORE:
        case CHANNEL_HWTS_LOG:
        case CHANNEL_TSFW:
        case CHANNEL_KEY_POINT:
        case CHANNEL_DVPP_VENC:
        case CHANNEL_DVPP_JPEGE:
        case CHANNEL_DVPP_VDEC:
        case CHANNEL_DVPP_JPEGD:
        case CHANNEL_DVPP_VPC:
        case CHANNEL_NPU_APP_MEM:
        case CHANNEL_NPU_MEM:
#ifdef CFG_SOC_PLATFORM_MINIV2
            sub_resource_info->channel_state = (u32)PROF_CHANNEL_IDLE;
#else
            if (sub_resource_info->sub_channel_id == 0) {
                sub_resource_info->channel_state = (u32)PROF_CHANNEL_IDLE;
            }
#endif
            break;
        default:
            if (prof_hash_channel_valid(sub_resource_info->channel_id) == true &&
                sub_resource_info->sub_channel_id == 0) {
                sub_resource_info->channel_state = (u32)PROF_CHANNEL_IDLE;
            }
            break;
    }
}

/* container_envir_check: phy_mach and vir_mach, only one type could be used each time. */
int prof_runtime_check(u32 device_id, u32 channel_id, u32 vfid)
{
    struct prof_sub_channel_info *sub_channel_info = NULL;
    u32 vf_enable_count = 0;
    u32 phy_enable_count = 0;
    u32 i;

    /* vfid: 0, phy_mach; 1~7, vir_mach */
    for (i = 0; i < PROF_SUB_CHANNEL_NUM_MAX; i++) {
        sub_channel_info = prof_get_sub_resource_info(device_id, channel_id, i);
        if (sub_channel_info->channel_state != (u32)PROF_CHANNEL_UNINIT &&
            sub_channel_info->channel_state != (u32)PROF_CHANNEL_IDLE) {
            if (i == PROF_PHYSICAL_MACHINE_VFID) {
                phy_enable_count++;
            } else {
                vf_enable_count++;
            }
        }
    }

    if (vfid == PROF_PHYSICAL_MACHINE_VFID && vf_enable_count != 0) {
        prof_err("It was ontainer scene. Physical machine was disabled. (vf_enable_count=%u)\n", vf_enable_count);
        return PROF_CONTAINER_SCENE_NOT_OPEN_PHY;
    }

    if (vfid != PROF_PHYSICAL_MACHINE_VFID && phy_enable_count != 0) {
        prof_err("It was hysical scene: Container machine was disabled. (phy_enable_count=%u)\n", phy_enable_count);
        return PROF_PHY_SCENE_NOT_OPEN_CONTAINER;
    }

    return PROF_OK;
}

int prof_get_available_sub_channel(struct prof_ioctl_para *para, struct prof_proc_ctx *proc_ctx,
    u32 phy_device_id, u32 *sub_channel_id)
{
    struct prof_channel_resource_info *channel_resource_info = NULL;
    struct prof_sub_channel_info *sub_channel_info = NULL;
    u32 device_id = para->device_id;
    u32 vfid = para->vfid;
    u32 channel_id = para->channel_id;
    int ret;

    channel_resource_info = prof_get_channel_resource_info(device_id, channel_id);
    mutex_lock(&(channel_resource_info->mutex));
    ret = prof_runtime_check(device_id, channel_id, vfid);
    if (ret != PROF_OK) {
        mutex_unlock(&(channel_resource_info->mutex));
        prof_err("Failed to invoke function [prof_runtime_check]."
            " (devid=%u; vfid=%u; channel_id=%u; ret=%d)\n",
            para->device_id, para->vfid, para->channel_id, ret);
        return ret;
    }

    sub_channel_info = prof_get_sub_resource_info(device_id, channel_id, vfid);
    if (sub_channel_info->channel_state == (u32)PROF_CHANNEL_IDLE) {
        *sub_channel_id = vfid;
        sub_channel_info->channel_state = (u32)PROF_CHANNEL_DISABLE;
        mutex_unlock(&(channel_resource_info->mutex));
        return PROF_OK;
    }

    mutex_unlock(&(channel_resource_info->mutex));
    return PROF_NOT_ENOUGH_SUB_CHANNEL_RESOURCE;
}

void prof_free_sub_channel_id(struct prof_sub_channel_res *sub_channel_res,
    u32 device_id, u32 phy_devid, u32 channel_id, u32 sub_channel_id)
{
    struct prof_channel_resource_info *channel_resource_info =
        prof_get_channel_resource_info(device_id, channel_id);
    mutex_lock(&(channel_resource_info->mutex));
    channel_resource_info->sub_resource_info[sub_channel_id].channel_state = (u32)PROF_CHANNEL_IDLE;
    mutex_unlock(&(channel_resource_info->mutex));
}

int prof_ctx_sub_channel_res_init(struct prof_sub_channel_res *sub_channel_res)
{
    return PROF_OK;
}

void prof_ctx_sub_channel_res_uninit(struct prof_sub_channel_res *sub_channel_res)
{
    return;
}

int prof_tscpu_platform_read_sub_ch(struct prof_ioctl_para *para,
    struct prof_sub_channel_info *sub_channel_info)
{
    u32 i, need_read_group_num;
    int ret;

    need_read_group_num = prof_tscpu_get_group_num(sub_channel_info->device_id,
        sub_channel_info->channel_id);

    for (i = 0; i < need_read_group_num; i++) {
        ret = prof_tscpu_ringbuffer(sub_channel_info->vir_addr +
            ((unsigned long long)i * PROF_TS_BUF_ONE_GROUP_SIZE), para,
            &sub_channel_info->prof_dfx, sub_channel_info->sub_channel_id);
        if (ret != PROF_OK) {
            prof_err("Failed to read the ringbuffer. (devid=%u; vfid=%u; channel_id=%u; ret=%d)\n",
                sub_channel_info->device_id, sub_channel_info->vfid, sub_channel_info->channel_id, ret);
            return ret;
        }
    }

    return ret;
}

u32 prof_tscpu_get_data_len(struct prof_sub_channel_info *sub_channel_info)
{
    u32 i, group_num;
    u32 data_len = 0;

    group_num = prof_tscpu_get_group_num(sub_channel_info->device_id,
        sub_channel_info->channel_id);

    for (i = 0; i < group_num; i++) {
        data_len += prof_tscpu_get_one_zone_data_len((unsigned char *)(sub_channel_info->vir_addr +
            (u64)i * PROF_TS_BUF_ONE_GROUP_SIZE),
            prof_get_buff_data_len(sub_channel_info->device_id, sub_channel_info->channel_id,
            sub_channel_info->vfid, sub_channel_info->sub_channel_id));
    }

    return data_len;
}

void prof_platform_ts_buff_free_flag(enum prof_ts_buff_free_flag *flag)
{
}

void prof_platform_ts_buff_not_free_op(struct prof_sub_channel_info *sub_channel_info)
{
}

void prof_init_hash(void)
{
#ifdef CFG_SOC_PLATFORM_MDC_V51
    if (tsdrv_get_chip_type() == CHIP_TYPE_MDC_BS9SX1A) {
        prof_hash[CHANNEL_DDR] = 0; // mdc BS9SX1A not support DDR prof channel
    }
    prof_init_ctrlcpu_mask();
#endif
}
#else
void prof_ut_test_ascd610(void)
{
}
module_init(prof_ut_test_ascd610);
#endif
