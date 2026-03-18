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

#ifndef DEVMNGI_MINI_UT

#include <linux/delay.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/securec.h>
#include <linux/timekeeping.h>

#include "devdrv_common.h"
#include "devdrv_manager_common.h"
#include "devdrv_manager_llc.h"
#include "ascend_platform.h"
#include "ascend_kernel_hal.h"
#include "devdrv_pm.h"
#include "devdrv_platform_resource.h"
#include "devdrv_manager_comm.h"

STATIC llc_info_t *g_llc_info[MAX_CHIP_NUM] = {NULL};

typedef enum hrtimer_restart (*hrtimer_timeout_handler)(struct hrtimer *timer);
STATIC void devdrv_llc_dfx_handler(unsigned long info);

/*
 * LLC timer for dfx statistic.
 */
STATIC enum hrtimer_restart devdrv_llc_hrtimer_handler(struct hrtimer *timer)
{
    llc_info_t *info = NULL;

    info = container_of(timer, llc_info_t, dfx_timer);
    tasklet_schedule(&info->tasklet);
    return HRTIMER_NORESTART;
}

/*
 * llc register operation, read or write
 */
STATIC int devdrv_llc_reg_op(unsigned char op_type, const void __iomem *regs, unsigned long reg_offset,
                             unsigned int *val)
{
#ifdef CFG_FEATURE_LLC_STUB
    return 0;
#else
    if (regs == NULL) {
        devdrv_drv_err("register is NULL.\n");
        return FAILED;
    }

    if (op_type == L3T_REG_WR) {
        writel(*val, (void __iomem *)(uintptr_t)((long)(uintptr_t)regs + reg_offset));
    } else {
        *val = readl((void __iomem *)(uintptr_t)((long)(uintptr_t)regs + reg_offset));
    }
#endif
    return 0;
}

#ifdef CFG_FEATURE_LLC_STUB
#define L3T_EVT_CNT0_STUB_OFFSET      0
#define L3T_EVT_CNT4_STUB_OFFSET      4
STATIC int devdrv_llc_get_stat_info_stub(unsigned int dev_id, llc_evt_info_t *evt_info)
{
    unsigned int i = 0;
    unsigned long j = 0;
    unsigned int op_val = 0;
    struct timespec64 tmp_timestamp = { 0 };
    static unsigned int hit_type_sel = 0;
    static unsigned int l3t_evt_cntx_value = 0;
    unsigned int hit_type[LLC_EVT_REG_NUM_PER_CHAN][LLC_EVT_REG_NUM_PER_CHAN] = {
        { LLC_EVT_TYPE0_RD_HIT, LLC_EVT_TYPE1_RD_HIT },
        { LLC_EVT_TYPE0_WR_HIT, LLC_EVT_TYPE1_WR_HIT },
    };

    ktime_get_raw_ts64(&tmp_timestamp);
    for (i = 0; i < L3T_TOTAL_NUM; i++) {
        evt_info->time_stamp[i] = tmp_timestamp;
        for (j = 0; j < L3T_EVT_NUM_MAX; j++) {
            /* 1. get event cnt low */
            /* 2. get event cnt high */
            l3t_evt_cntx_value++;
            if ((j == L3T_EVT_CNT0_STUB_OFFSET) || (j == L3T_EVT_CNT4_STUB_OFFSET)) {
                evt_info->event_cnt[i][j] = l3t_evt_cntx_value & LLC_CNT_MAX;
            } else {
                evt_info->event_cnt[i][j] = 0;
            }

            /* 3. get event type0 */
            hit_type_sel = hit_type_sel % 2; /* 2 for take 0 and 1. */
            op_val = hit_type[hit_type_sel][0];
            evt_info->event_type[i][j] = op_val;

            /* 4. get event type1 */
            op_val = hit_type[hit_type_sel][1];
            evt_info->event_type[i][j] |= ((u64)op_val << L3T_EVT_REG_BITS);
        }
    }
    hit_type_sel++;
    return 0;
}
#endif

/*
 * Get l3t event statistic information
 */
STATIC int devdrv_llc_get_stat_info(unsigned int dev_id, llc_evt_info_t *evt_info)
{
#ifdef CFG_FEATURE_LLC_STUB
    return devdrv_llc_get_stat_info_stub(dev_id, evt_info);
#else
    int ret;
    unsigned int i = 0;
    unsigned long j = 0;
    unsigned int op_val = 0;
    void __iomem *reg_base = NULL;
    struct timespec64 tmp_timestamp = {0};
    unsigned long reg_offset[L3T_EVT_REG_NUM] = {
        L3T_EVT_CNT0_L_OFFSET, L3T_EVT_CNT0_H_OFFSET, L3T_EVT_CNT1_L_OFFSET, L3T_EVT_CNT1_H_OFFSET,
        L3T_EVT_CNT2_L_OFFSET, L3T_EVT_CNT2_H_OFFSET, L3T_EVT_CNT3_L_OFFSET, L3T_EVT_CNT3_H_OFFSET,
        L3T_EVT_CNT4_L_OFFSET, L3T_EVT_CNT4_H_OFFSET, L3T_EVT_CNT5_L_OFFSET, L3T_EVT_CNT5_H_OFFSET,
        L3T_EVT_CNT6_L_OFFSET, L3T_EVT_CNT6_H_OFFSET, L3T_EVT_CNT7_L_OFFSET, L3T_EVT_CNT7_H_OFFSET
    };

    ktime_get_raw_ts64(&tmp_timestamp);
    for (i = 0; i < L3T_TOTAL_NUM; i++) {
        reg_base = g_llc_info[dev_id]->hw_info.regs[i];
        evt_info->time_stamp[i] = tmp_timestamp;
        for (j = 0; j < L3T_EVT_NUM_MAX; j++) {
            op_val = 0;
            /* 1. get event cnt low */
            ret = devdrv_llc_reg_op(L3T_REG_RD, reg_base, reg_offset[LLC_EVT_REG_NUM_PER_CHAN * j], &op_val);
            if (ret < 0) {
                devdrv_drv_err("devid %u devdrv_llc_reg_op failed ret = %d.\n", dev_id, ret);
                return ret;
            }

            evt_info->event_cnt[i][j] = op_val;

            /* 2. get event cnt high */
            op_val = 0;
            ret = devdrv_llc_reg_op(L3T_REG_RD, reg_base, reg_offset[LLC_EVT_REG_NUM_PER_CHAN * j + 1], &op_val);
            if (ret < 0) {
                devdrv_drv_err("devid %u devdrv_llc_reg_op failed ret = %d.\n", dev_id, ret);
                return ret;
            }

            op_val = op_val & L3T_EVT_CNT_H_MASK;
            evt_info->event_cnt[i][j] |= ((u64)op_val << L3T_EVT_REG_BITS);
            evt_info->event_cnt[i][j] &= LLC_CNT_MAX;

            /* 3. get event type0 */
            ret = devdrv_llc_reg_op(L3T_REG_RD, reg_base, L3T_EVT_TYPE0_OFFSET, &op_val);
            if (ret < 0) {
                devdrv_drv_err("devid %u devdrv_llc_reg_op failed ret = %d.\n", dev_id, ret);
                return ret;
            }

            evt_info->event_type[i][j] = op_val;

            /* 4. get event type1 */
            ret = devdrv_llc_reg_op(L3T_REG_RD, reg_base, L3T_EVT_TYPE1_OFFSET, &op_val);
            if (ret < 0) {
                devdrv_drv_err("devid %u devdrv_llc_reg_op failed ret = %d.\n", dev_id, ret);
                return ret;
            }
            evt_info->event_type[i][j] |= ((u64)op_val << L3T_EVT_REG_BITS);
        }
    }
    return 0;
#endif
}

/*
 * calculte event cnt difference between two event
 */
STATIC unsigned long long devdrv_llc_get_evt_cnt_diff(unsigned long long start, unsigned long long end)
{
    unsigned long long cnt_diff;

    cnt_diff = ((unsigned long long)end + ((unsigned long long)LLC_CNT_MAX + 1) - start) % \
        ((unsigned long long)LLC_CNT_MAX + 1);

    return cnt_diff;
}

STATIC void devdrv_llc_clr_dfx_state(unsigned int dev_id, unsigned int dfx_state, unsigned int host_type)
{
    int ret;
    llc_info_t *info = NULL;

    info = g_llc_info[dev_id];
    info->dfx_state &= ~dfx_state;

    if (host_type == LLC_DFX_HOST_DSMI) {
        info->dsmi_info.sample_type = LLC_PROF_TYPE_NONE;
    } else {
        info->prof_info.sample_type = LLC_PROF_TYPE_NONE;
    }

    /* when no sample event is running, cancel hrtimer */
    if (info->dfx_state == DFX_STAT_NONE) {
        ret = hrtimer_cancel(&info->dfx_timer);
        if (ret) {
            devdrv_drv_warn("dev_id[%u] hrtimer is active now, ret = %d.\n", dev_id, ret);
        }

        tasklet_kill(&info->tasklet);
    }
}

/*
 * llc register memory map
 */
STATIC int devdrv_llc_reg_map(unsigned int dev_id)
{
    int ret;
    unsigned int i = 0;
    unsigned long reg_base[L3T_TOTAL_NUM_MAX] = {
        L3_TAG0_REG_BASE, L3_TAG1_REG_BASE, L3_TAG2_REG_BASE, L3_TAG3_REG_BASE
    };
    llc_info_t *info = g_llc_info[dev_id];
    devdrv_hardware_info_t hardware_info = {0};

    ret = hal_kernel_get_hardware_info(dev_id, &hardware_info);
    if (ret != 0) {
        devdrv_drv_err("Failed to invoke hal_kernel_get_hardware_info. (devid=%u)\n", dev_id);
        return ret;
    }

    for (i = 0; i < L3T_TOTAL_NUM; i++) {
        if (reg_base[i] == 0) {
            devdrv_drv_warn("The register is configured to zero. (dev_id=%u)\n", dev_id);
            break;
        }

        info->hw_info.reg_base[i] = reg_base[i] + hardware_info.phy_addr_offset;
        info->hw_info.regs[i] = ioremap(info->hw_info.reg_base[i], L3T_MAP_SIZE);
        if (info->hw_info.regs[i] == NULL) {
            devdrv_drv_err("dev_id[%u] ioremap error.\n", dev_id);
            return -ENOMEM;
        }
    }

    return 0;
}

/*
 * llc register memory unmap
 */
STATIC void devdrv_llc_reg_unmap(unsigned int dev_id)
{
    unsigned int i = 0;
    llc_info_t *info = NULL;

    info = g_llc_info[dev_id];
    for (i = 0; i < L3T_TOTAL_NUM; i++) {
        if (info->hw_info.regs[i] != NULL) {
            iounmap(info->hw_info.regs[i]);
            info->hw_info.regs[i] = NULL;
        }
    }
}

/*
 * calculte time interval between start and end time.
 */
STATIC unsigned long devdrv_llc_get_time_interval(struct timespec64 start, struct timespec64 end)
{
    unsigned long time_use;

    time_use = (end.tv_sec - start.tv_sec) * LLC_TIME_STAMP_PER_SECOND +
               (end.tv_nsec - start.tv_nsec) / NSEC_PER_USEC;
    return time_use;
}

/*
 * Disable llc dfx statistic
 */
STATIC int devdrv_llc_set_dfx_stat_state(unsigned int dev_id, unsigned int state)
{
    int ret;
    unsigned int i = 0;
    unsigned int op_val = 0;
    void __iomem *reg_base = NULL;
    llc_info_t *info = NULL;

    info = g_llc_info[dev_id];

    /* Disable all l3t dfx stat */
    for (i = 0; i < L3T_TOTAL_NUM; i++) {
        reg_base = info->hw_info.regs[i];

        /* Write register */
        op_val = L3T_DFX_CTRL_DEFAULT;
        if (state == L3T_PERF_DFX_ENABLE) {
            op_val = op_val | L3T_PERF_EVT_EN_MASK;
        } else {
            op_val = op_val & (~L3T_PERF_EVT_EN_MASK);
        }
        ret = devdrv_llc_reg_op(L3T_REG_WR, reg_base, L3T_DFX_CTRL_OFFSET, &op_val);
        if (ret < 0) {
            devdrv_drv_err("devid %u devdrv_llc_reg_op failed, ret = %d.\n", dev_id, ret);
            return ret;
        }
    }

    return 0;
}

/*
 *  clear llc event counter
 */
STATIC int devdrv_llc_clr_evt_counter(unsigned int dev_id)
{
    int ret;
    unsigned int i = 0;
    unsigned long j = 0;
    unsigned int op_val = 0;
    void __iomem *reg_base = NULL;
    llc_info_t *info = NULL;

    unsigned long reg_offset[L3T_EVT_REG_NUM] = {
        L3T_EVT_CNT0_L_OFFSET, L3T_EVT_CNT0_H_OFFSET, L3T_EVT_CNT1_L_OFFSET, L3T_EVT_CNT1_H_OFFSET,
        L3T_EVT_CNT2_L_OFFSET, L3T_EVT_CNT2_H_OFFSET, L3T_EVT_CNT3_L_OFFSET, L3T_EVT_CNT3_H_OFFSET,
        L3T_EVT_CNT4_L_OFFSET, L3T_EVT_CNT4_H_OFFSET, L3T_EVT_CNT5_L_OFFSET, L3T_EVT_CNT5_H_OFFSET,
        L3T_EVT_CNT6_L_OFFSET, L3T_EVT_CNT6_H_OFFSET, L3T_EVT_CNT7_L_OFFSET, L3T_EVT_CNT7_H_OFFSET
    };

    info = g_llc_info[dev_id];
    for (i = 0; i < L3T_TOTAL_NUM; i++) {
        reg_base = info->hw_info.regs[i];
        for (j = 0; j < L3T_EVT_NUM_MAX; j++) {
            op_val = 0;
            /* 1. clear event counter low */
            ret = devdrv_llc_reg_op(L3T_REG_WR, reg_base, reg_offset[LLC_EVT_REG_NUM_PER_CHAN * j], &op_val);
            if (ret < 0) {
                devdrv_drv_err("devid %u devdrv_llc_reg_op failed ret = %d.\n", dev_id, ret);
                return ret;
            }

            /* 2. clear event counter high */
            ret = devdrv_llc_reg_op(L3T_REG_WR, reg_base, reg_offset[LLC_EVT_REG_NUM_PER_CHAN * j + 1], &op_val);
            if (ret < 0) {
                devdrv_drv_err("devid %u devdrv_llc_reg_op failed ret = %d.\n", dev_id, ret);
                return ret;
            }
        }
    }

    return 0;
}

/*
 * Enable LLC flux statistic
 */
STATIC int devdrv_llc_set_event_type(unsigned int dev_id, unsigned int sample_type)
{
    int ret;
    unsigned int i = 0;
    unsigned int op_val = 0;
    void __iomem *reg_base = NULL;
    llc_info_t *info = NULL;
    unsigned int *type_val_pt = NULL;
    unsigned int rd_hit_type[LLC_EVT_REG_NUM_PER_CHAN] = { LLC_EVT_TYPE0_RD_HIT, LLC_EVT_TYPE1_RD_HIT };
    unsigned int wr_hit_type[LLC_EVT_REG_NUM_PER_CHAN] = { LLC_EVT_TYPE0_WR_HIT, LLC_EVT_TYPE1_WR_HIT };

    if (sample_type >= LLC_PROF_TYPE_MAX) {
        devdrv_drv_err("devid %u sample type %u invalid.\n", dev_id, sample_type);
        return -EINVAL;
    }

    if (sample_type == LLC_PROF_TYPE_WR_HIT) {
        type_val_pt = wr_hit_type;
    } else {
        type_val_pt = rd_hit_type;
    }

    info = g_llc_info[dev_id];
    for (i = 0; i < L3T_TOTAL_NUM; i++) {
        reg_base = info->hw_info.regs[i];

        /* Write event type 0 register */
        op_val = type_val_pt[0];
        ret = devdrv_llc_reg_op(L3T_REG_WR, reg_base, L3T_EVT_TYPE0_OFFSET, &op_val);
        if (ret < 0) {
            devdrv_drv_err("devid %u devdrv_llc_reg_op failed, ret = %d.\n", dev_id, ret);
            return ret;
        }

        /* Write event type 1 register */
        op_val = type_val_pt[1];
        ret = devdrv_llc_reg_op(L3T_REG_WR, reg_base, L3T_EVT_TYPE1_OFFSET, &op_val);
        if (ret < 0) {
            devdrv_drv_err("devid %u devdrv_llc_reg_op failed, ret = %d.\n", dev_id, ret);
            return ret;
        }
    }

    return 0;
}

/*
 * Enable LLC Event type
 */
STATIC int devdrv_llc_enable_evt_type(unsigned int dev_id)
{
    int ret;
    unsigned int i = 0;
    llc_info_t *info = NULL;
    unsigned int op_val = L3T_EVT_CTRL_SET;

    info = g_llc_info[dev_id];
    for (i = 0; i < L3T_TOTAL_NUM; i++) {
        /* Write event type 0 register */
        ret = devdrv_llc_reg_op(L3T_REG_WR, info->hw_info.regs[i], L3T_EVT_CTRL_OFFSET, &op_val);
        if (ret < 0) {
            devdrv_drv_err("devid %u devdrv_llc_reg_op failed, ret = %d.\n", dev_id, ret);
            return ret;
        }
    }

    return 0;
}

/*
 * Update statistic information
 */
STATIC int devdrv_llc_update_stat_info(unsigned int dev_id)
{
    unsigned int i = 0;
    unsigned int j = 0;
    llc_info_t *info = NULL;
    llc_sample_info_t *sample_info = NULL;

    info = g_llc_info[dev_id];
    if (info->cur_sample_host == LLC_DFX_HOST_DSMI) {
        sample_info = &info->dsmi_info;
        sample_info->triggering_cnt++;
    } else {
        sample_info = &info->prof_info;
        sample_info->triggering_cnt++;
    }

    sample_info->total_time += devdrv_llc_get_time_interval(info->bk_evt_info.time_stamp[0],
                                                            info->cur_evt_info.time_stamp[0]);

    for (i = 0; i < L3T_TOTAL_NUM; i++) {
        for (j = 0; j < L3T_EVT_NUM_MAX; j++) {
            sample_info->evt_info.event_cnt[i][j] +=
                devdrv_llc_get_evt_cnt_diff(info->bk_evt_info.event_cnt[i][j],
                                            info->cur_evt_info.event_cnt[i][j]);
            sample_info->evt_info.event_type[i][j] = info->cur_evt_info.event_type[i][j];
        }
    }

    return 0;
}

/*
 * Set llc profiling data buffer, for the user.
 */
STATIC int devdrv_llc_prof_set_buf(unsigned int dev_id, llc_prof_data_t *buf, int len, llc_sample_info_t *info)
{
    int data_len = 0;
    unsigned int i = 0;
    unsigned int j = 0;
    llc_prof_data_t *prof_data_pt = NULL;
    unsigned long evt_mask_array[L3T_EVT_NUM_MAX] = {
        L3T_EVT_CNT0_TYPE_MASK, L3T_EVT_CNT1_TYPE_MASK, L3T_EVT_CNT2_TYPE_MASK, L3T_EVT_CNT3_TYPE_MASK,
        L3T_EVT_CNT4_TYPE_MASK, L3T_EVT_CNT5_TYPE_MASK, L3T_EVT_CNT6_TYPE_MASK, L3T_EVT_CNT7_TYPE_MASK
    };
    unsigned int evt_bit_array[L3T_EVT_NUM_MAX] = {
        L3T_EVT_CNT0_TYPE_BIT, L3T_EVT_CNT1_TYPE_BIT, L3T_EVT_CNT2_TYPE_BIT, L3T_EVT_CNT3_TYPE_BIT,
        L3T_EVT_CNT4_TYPE_BIT, L3T_EVT_CNT5_TYPE_BIT, L3T_EVT_CNT6_TYPE_BIT, L3T_EVT_CNT7_TYPE_BIT
    };

    prof_data_pt = buf;
    for (i = 0; i < L3T_TOTAL_NUM; i++) {
        for (j = 0; j < L3T_EVT_NUM_MAX; j++) {
            prof_data_pt->timestamp = info->total_time;
            prof_data_pt->event_id = (info->evt_info.event_type[i][j] & evt_mask_array[j]) >> evt_bit_array[j];
            prof_data_pt->count = info->evt_info.event_cnt[i][j];
            prof_data_pt->l3_tag_id = i;
            prof_data_pt++;
            data_len += sizeof(llc_prof_data_t);
            if (data_len > len) {
                data_len = data_len - sizeof(llc_prof_data_t);
                return data_len;
            }
        }
    }

    return data_len;
}

/*
 * Enable dfx event sample process.
 */
STATIC int devdrv_llc_enable_event_sample(unsigned int dev_id, unsigned int sample_type)
{
    int ret;

    /* 1. Disable dfx stat */
    ret = devdrv_llc_set_dfx_stat_state(dev_id, L3T_PERF_DFX_DISABLE);
    if (ret < 0) {
        devdrv_drv_err("devid %u devdrv_llc_set_dfx_stat_state err, ret = %d.\n", dev_id, ret);
        return FAILED;
    }

    /* 2. Set event type */
    ret = devdrv_llc_set_event_type(dev_id, sample_type);
    if (ret < 0) {
        devdrv_drv_err("devid %u devdrv_llc_set_event_type err, ret = %d.\n", dev_id, ret);
        return FAILED;
    }

    /* 3. Reset counters to 0 */
    ret = devdrv_llc_clr_evt_counter(dev_id);
    if (ret < 0) {
        devdrv_drv_err("devid %u devdrv_llc_clr_evt_counter err, ret = %d.\n", dev_id, ret);
        return FAILED;
    }

    /* 4. Enable event type */
    ret = devdrv_llc_enable_evt_type(dev_id);
    if (ret < 0) {
        devdrv_drv_err("devid %u devdrv_llc_set_event_type err, ret = %d.\n", dev_id, ret);
        return FAILED;
    }

    /* 5. Enable dfx stat */
    ret = devdrv_llc_set_dfx_stat_state(dev_id, L3T_PERF_DFX_ENABLE);
    if (ret < 0) {
        devdrv_drv_err("devid %u devdrv_llc_set_dfx_stat_state err, ret = %d.\n", dev_id, ret);
        return FAILED;
    }

    return 0;
}

#ifdef CFG_SOC_PLATFORM_MINIV2
/* unset task on cpu0*/
void devdrv_unset_cpu(void)
{
    cpumask_var_t cpumask;

    if (!zalloc_cpumask_var(&cpumask, GFP_KERNEL)) {
        return;
    }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
    cpumask_copy(cpumask, &current->cpus_mask);
#else
    cpumask_copy(cpumask, &current->cpus_allowed);
#endif
    if(cpumask_weight(cpumask) > 1) {
        cpumask_clear_cpu(0, cpumask);
        if (cpumask_available(cpumask)) {
            /* set cpumask */
            set_cpus_allowed_ptr(current, cpumask);
        }
    }
    free_cpumask_var(cpumask);
}
#endif
/*
 * update dfx stat
 */
STATIC int devdrv_llc_set_dfx_state(unsigned int dev_id, unsigned int dfx_state, unsigned int sample_type)
{
    int ret = 0;
    ktime_t kt;
    llc_info_t *info = NULL;

    info = g_llc_info[dev_id];
    if (dfx_state == DFX_STAT_DSMI_RUNNING) {
        info->dsmi_info.triggering_cnt = 0;
    }
    if (dfx_state == DFX_STAT_PROF_RUNNING) {
        info->prof_info.triggering_cnt = 0;
    }
    info->dfx_state |= dfx_state;
    if (info->dfx_state != DFX_STAT_ALL_RUNNING) {
        ret = devdrv_llc_enable_event_sample(dev_id, sample_type);
        if (ret < 0) {
            devdrv_drv_err("devid %u devdrv_llc_enable_dfx_event_sample err ret = %d.\n", dev_id, ret);
            return FAILED;
        }

        ret = devdrv_llc_get_stat_info(dev_id, &info->bk_evt_info);
        if (ret < 0) {
            devdrv_drv_err("devid %u devdrv_llc_get_stat_info error ret = %d.\n", dev_id, ret);
            return ret;
        }
#ifdef CFG_SOC_PLATFORM_MINIV2
        devdrv_unset_cpu();
#endif
        kt = ktime_set(0, DFX_STAT_HRTIMER_PERIOD);
        hrtimer_start(&(info->dfx_timer), kt, HRTIMER_MODE_REL);
    }

    return 0;
}

/*
 * LLC profiling sample func, callback for the profiling driver.
 */
EXPORT_SYMBOL(devdrv_llc_prof_sample);
int devdrv_llc_prof_sample(struct prof_peri_para para)
{
    int ret;
    llc_sample_info_t *info_pt = NULL;
    llc_sample_info_t *sample_info = NULL;
    llc_prof_backup_info_t *backup = NULL;
    int buf_data_len;
    llc_info_t *info = NULL;
    unsigned long time_interval;
    unsigned int dev_id = para.device_id;
    void *buf = para.buff;
    int len = para.buff_len;

    if ((buf == NULL) || (dev_id >= DEVDRV_MANGER_MAX_DEVICE_NUM)) {
        devdrv_drv_err("Parameter invalid. (buf=%pK; dev_id=%u)\n", buf, dev_id);
        return FAILED;
    }

    info = g_llc_info[dev_id];
    sample_info = &info->prof_info;
    backup = &info->prof_back;
    info_pt = kzalloc(sizeof(llc_sample_info_t), GFP_KERNEL | __GFP_ACCOUNT);
    if (info_pt == NULL) {
        devdrv_drv_err("Failed to invoke kzalloc. (dev_id=%u)\n", dev_id);
        return -ENOMEM;
    }

    /* 1. copy sample data out */
    mutex_lock(&info->dfx_lock);
    time_interval = sample_info->total_time;
    if (time_interval > 0) {
        ret = memcpy_s(info_pt, sizeof(llc_sample_info_t), sample_info, sizeof(llc_sample_info_t));
        if (ret != 0) {
            devdrv_drv_err("Failed to invoke memcpy_s. (dev_id=%u; ret=%d)\n", dev_id, ret);
            goto OUT;
        }

        backup->error_cnt = 0;
        ret = memcpy_s(&backup->last_result, sizeof(llc_sample_info_t), sample_info, sizeof(llc_sample_info_t));
        if (ret != 0) {
            /* The backup failed, but the result is correct. */
            devdrv_drv_warn("Failed to back up data, because invoke memcpy_s failed. (ret=%d)\n", ret);
        }
    } else {
        backup->error_cnt++;
        if (backup->error_cnt > LLC_GET_ERROR_NUM) {
            backup->error_cnt = LLC_GET_ERROR_NUM + 1;
            devdrv_drv_err("Timestamp error. (dev_id=%u; time_interval=%lu; triggering_cnt=%u)\n",
                dev_id, time_interval, sample_info->triggering_cnt);
            goto OUT;
        }

        devdrv_drv_warn("Timestamp error or system busy. (dev_id=%u; time_interval=%lu; triggering_cnt=%u; err_cnt=%u)\n",
            dev_id, time_interval, sample_info->triggering_cnt, backup->error_cnt);

        /* The result is incorrect, the last result is reported. */
        ret = memcpy_s(info_pt, sizeof(llc_sample_info_t), &backup->last_result, sizeof(llc_sample_info_t));
        if (ret != 0) {
            devdrv_drv_err("Failed to invoke memcpy_s. (dev_id=%u; ret=%d)\n", dev_id, ret);
            goto OUT;
        }
    }

    ret = memset_s(&info->prof_info.evt_info, sizeof(llc_evt_info_t), 0, sizeof(llc_evt_info_t));
    if (ret != OK) {
        devdrv_drv_err("Failed to invoke memset_s. (dev_id=%u; ret=%d)\n", dev_id, ret);
        goto OUT;
    }

    mutex_unlock(&info->dfx_lock);

    /* 2. fill data buff */
    buf_data_len = devdrv_llc_prof_set_buf(dev_id, (llc_prof_data_t *)buf, len, info_pt);

    kfree(info_pt);
    info_pt = NULL;
    return buf_data_len;

OUT:
    mutex_unlock(&info->dfx_lock);
    kfree(info_pt);
    info_pt = NULL;
    return FAILED;
}

/*
 * Get LLC profiling cfg data, callback for the profiling driver.
 */
EXPORT_SYMBOL(devdrv_llc_prof_sample_userdata);
int devdrv_llc_prof_sample_userdata(struct prof_peri_para para)
{
    int ret;
    llc_info_t *info = NULL;
    unsigned int dev_id = para.device_id;
    void *data = para.user_data;
    unsigned int data_size = para.user_data_len;

    if ((data == NULL) || (dev_id >= MAX_CHIP_NUM) || (data_size != sizeof(llc_prof_cfg_t))) {
        devdrv_drv_err("invalid para data is %pK, device_id=%u, data_size=%u.\n", data, dev_id, data_size);
        return FAILED;
    }

    info = g_llc_info[dev_id];
    mutex_lock(&info->dfx_lock);

    /* copy user data and fill configure info */
    ret = memcpy_s(&info->prof_cfg, sizeof(llc_prof_cfg_t), (void *)data, data_size);
    if (ret != OK) {
        mutex_unlock(&info->dfx_lock);
        devdrv_drv_err("devid %u memcpy_s return error: %d.\n", dev_id, ret);
        return FAILED;
    }

    if (info->prof_cfg.sample_type >= LLC_PROF_TYPE_MAX) {
        mutex_unlock(&info->dfx_lock);
        devdrv_drv_err("prof_cfg[%u].sample_type err.\n", dev_id);
        return FAILED;
    }

    info->prof_info.sample_type = info->prof_cfg.sample_type;

    ret = devdrv_llc_set_dfx_state(dev_id, DFX_STAT_PROF_RUNNING, info->prof_info.sample_type);
    if (ret < 0) {
        mutex_unlock(&info->dfx_lock);
        devdrv_drv_err("devid %u devdrv_llc_enable_dfx_stat return error: %d.\n", dev_id, ret);
        return FAILED;
    }

    mutex_unlock(&info->dfx_lock);
    /* wait timer to trigger at least once. */
    msleep(DFX_WAIT_HRTIMER_START_TIME);
    return 0;
}

/*
 * llc perf info check
 */
STATIC int devdrv_llc_check_perf_info(llc_perf_para_t *para)
{
    if (para->rd_hit_rate > LLC_STAT_PERTAGE) {
        para->rd_hit_rate = LLC_STAT_PERTAGE;
        devdrv_drv_warn("llc read hit rate[%u] is beyond normal value.\n", para->rd_hit_rate);
        return -EINVAL;
    }

    if (para->wr_hit_rate > LLC_STAT_PERTAGE) {
        para->wr_hit_rate = LLC_STAT_PERTAGE;
        devdrv_drv_warn("llc write hit rate[%u] is beyond normal value.\n", para->wr_hit_rate);
        return -EINVAL;
    }

    return 0;
}

/*
 * get llc read hit rate/write hit rate/throughput counter info
 */
STATIC void devdrv_llc_cnt_info(llc_sample_info_t *sample_info, llc_cnt_info_t *cnt_info)
{
    unsigned int i = 0;
    unsigned int j = 0;
    unsigned int k = 0;

    for (i = 0; i < L3T_TOTAL_NUM; i++) {
        if ((sample_info->evt_info.event_type[i][0] & LLC_EVT_TYPE0_MASK) == LLC_EVT_TYPE0_RD_HIT) {
            j = LLC_EVT_CNT_RDWR_OFFSET;
            for (k = 0; k < LLC_EVT_CNT_RDWR_OFFSET; k++) {
                cnt_info->total_rd_cnt += sample_info->evt_info.event_cnt[i][j++];
            }

            for (k = 0; k < LLC_EVT_CNT_RDWR_NUM; k++) {
                cnt_info->hit_rd_cnt += sample_info->evt_info.event_cnt[i][j++];
            }
        }

        if ((sample_info->evt_info.event_type[i][0] & LLC_EVT_TYPE0_MASK) == LLC_EVT_TYPE0_WR_HIT) {
            j = LLC_EVT_CNT_RDWR_OFFSET;
            for (k = 0; k < LLC_EVT_CNT_RDWR_OFFSET; k++) {
                cnt_info->total_wr_cnt += sample_info->evt_info.event_cnt[i][j++];
            }

            for (k = 0; k < LLC_EVT_CNT_RDWR_NUM; k++) {
                cnt_info->hit_wr_cnt += sample_info->evt_info.event_cnt[i][j++];
            }
        }

        cnt_info->throughput_cnt += sample_info->evt_info.event_cnt[i][0] + sample_info->evt_info.event_cnt[i][1];
    }
}

/*
 * Calculate performance para, including read hit rate/write hit rate/throughput
 */
STATIC int devdrv_llc_calculate_perf_para(llc_perf_para_t *para, llc_sample_info_t *sample_info, llc_dsmi_backup_info_t *backup)
{
    int ret;
    llc_cnt_info_t cnt_info = {0};
    unsigned long time_interval;

    devdrv_llc_cnt_info(sample_info, &cnt_info);

    if (cnt_info.total_wr_cnt > 0) {
        para->wr_hit_rate = (LLC_STAT_PERTAGE * cnt_info.hit_wr_cnt) / cnt_info.total_wr_cnt;
    } else {
        para->wr_hit_rate = 0;
    }

    if (cnt_info.total_rd_cnt > 0) {
        para->rd_hit_rate = (LLC_STAT_PERTAGE * cnt_info.hit_rd_cnt) / cnt_info.total_rd_cnt;
    } else {
        para->rd_hit_rate = 0;
    }

    time_interval = sample_info->total_time;
    if (time_interval > 0) {
        cnt_info.throughput_cnt = (cnt_info.throughput_cnt * LLC_BIT_WIDTH_BYTE) / LLC_THROUGHPUT_DIV; /* KB as unit */
        para->throughput = (cnt_info.throughput_cnt * LLC_TIME_STAMP_PER_SECOND) / time_interval; /* KB/s as unit */

        backup->error_cnt = 0;
        ret = memcpy_s(&backup->last_result, sizeof(llc_perf_para_t), para, sizeof(llc_perf_para_t));
        if (ret != 0) {
            /* The backup failed, but the result is correct. */
            devdrv_drv_warn("Failed to back up data, because invoke memcpy_s failed. (ret=%d)\n", ret);
        }
    } else {
        backup->error_cnt++;
        if (backup->error_cnt > LLC_GET_ERROR_NUM) {
            backup->error_cnt = LLC_GET_ERROR_NUM + 1;
            devdrv_drv_err("Timestamp error. (time_interval=%lu; triggering_cnt=%u)\n", time_interval, sample_info->triggering_cnt);
            return FAILED;
        }

        devdrv_drv_warn("Timestamp error or system busy. (time_interval=%lu; triggering_cnt=%u; err_cnt=%u)\n",
            time_interval, sample_info->triggering_cnt, backup->error_cnt);

        /* The result is incorrect, the last result is reported. */
        ret = memcpy_s(para, sizeof(llc_perf_para_t), &backup->last_result, sizeof(llc_perf_para_t));
        if (ret != 0) {
            devdrv_drv_warn("Failed to invoke memcpy_s failed. (ret=%d)\n", ret);
            return FAILED;
        }
    }

    ret = devdrv_llc_check_perf_info(para);
    if (ret) {
        devdrv_drv_warn("Para result is out of range.\n");
    }

    return 0;
}

/*
 * process sample data and calculte llc performance result
 */
STATIC int devdrv_llc_process_sample_data(unsigned int dev_id, llc_perf_para_t *para)
{
    int ret;
    llc_sample_info_t *sample_info = NULL;
    llc_info_t *info = NULL;

    if (dev_id >= DEVDRV_MANGER_MAX_DEVICE_NUM) {
        devdrv_drv_err("devid %u kmalloc failed.\n", dev_id);
        return -EINVAL;
    }
    info = g_llc_info[dev_id];

    /* copy sample data out */
    sample_info = kzalloc(sizeof(llc_sample_info_t), GFP_KERNEL | __GFP_ACCOUNT);
    if (sample_info == NULL) {
        devdrv_drv_err("devid %u kmalloc failed.\n", dev_id);
        return -ENOMEM;
    }

    ret = memcpy_s(sample_info, sizeof(llc_sample_info_t), &info->dsmi_info, sizeof(llc_sample_info_t));
    if (ret != 0) {
        devdrv_drv_err("devid %u memcpy_s failed.\n", dev_id);
        goto OUT;
    }

    ret = memset_s(&info->dsmi_info, sizeof(llc_sample_info_t), 0, sizeof(llc_sample_info_t));
    if (ret != OK) {
        devdrv_drv_err("devid %u memset_s error, %d. \n", dev_id, ret);
        goto OUT;
    }

    /* calculate sample result */
    para->dev_id = dev_id;
    ret = devdrv_llc_calculate_perf_para(para, sample_info, &info->dsmi_back);
    if (ret < 0) {
        devdrv_drv_err("devid %u devdrv_llc_calculate_perf_para error ret = %d.\n", dev_id, ret);
        goto OUT;
    }

OUT:
    kfree(sample_info);
    sample_info = NULL;
    return ret;
}

/*
 * Get LLC performance para, used by dsmi module
 */
int devdrv_llc_get_perf_para(unsigned int dev_id, unsigned int para_type, llc_perf_para_t *para)
{
    int ret;
    struct timespec64 time_start = {0};
    struct timespec64 time_stop = {0};
    unsigned long time_interval = 0;
    llc_info_t *info = NULL;

    if ((para == NULL) || (dev_id >= MAX_CHIP_NUM) || (para_type >= LLC_PROF_TYPE_MAX)) {
        devdrv_drv_err("invalid para para is %pK, device_id=%u, para_type=%u.\n", para, dev_id, para_type);
        return FAILED;
    }

    info = g_llc_info[dev_id];
    mutex_lock(&info->dsmi_lock);
    mutex_lock(&info->dfx_lock);

    /* Set sample type and start hrtimer. */
    info->dsmi_info.sample_type = para_type;
    ret = devdrv_llc_set_dfx_state(dev_id, DFX_STAT_DSMI_RUNNING, info->dsmi_info.sample_type);
    if (ret < 0) {
        devdrv_drv_err("devid %u devdrv_llc_enable_dfx_stat return error: %d.\n", dev_id, ret);
        goto OUT;
    }

    mutex_unlock(&info->dfx_lock);

    /* wait for sample period timeout */
    ktime_get_raw_ts64(&time_start);
    while (time_interval < LLC_DSMI_DFX_TIME) {
        msleep(LLC_SLEEP_UINT_MS);
        ktime_get_raw_ts64(&time_stop);
        time_interval = devdrv_llc_get_time_interval(time_start, time_stop);
    };

    mutex_lock(&info->dfx_lock);
    /* clear llc dfx state */
    devdrv_llc_clr_dfx_state(dev_id, DFX_STAT_DSMI_RUNNING, LLC_DFX_HOST_DSMI);

    /* process llc data and calculate perfomance result */
    ret = devdrv_llc_process_sample_data(dev_id, para);
    if (ret != 0) {
        devdrv_drv_err("devid %u devdrv_llc_process_sample_data return error: %d.\n", dev_id, ret);
    }

OUT:
    mutex_unlock(&info->dfx_lock);
    mutex_unlock(&info->dsmi_lock);
    return ret;
}

/*
 * LLC profiling sample stop func, callback for the profiling driver.
 */
EXPORT_SYMBOL(devdrv_llc_prof_sample_stop);
int devdrv_llc_prof_sample_stop(struct prof_peri_para para)
{
    int ret;
    llc_info_t *info = NULL;
    unsigned int dev_id = para.device_id;

    if (dev_id >= MAX_CHIP_NUM) {
        devdrv_drv_err("dev_id error, dev_id = %u.\n", dev_id);
        return FAILED;
    }

    info = g_llc_info[dev_id];
    mutex_lock(&info->dfx_lock);

    devdrv_llc_clr_dfx_state(dev_id, DFX_STAT_PROF_RUNNING, LLC_DFX_HOST_PROF);

    ret = memset_s(&info->prof_info, sizeof(llc_sample_info_t), 0, sizeof(llc_sample_info_t));
    if (ret != OK) {
        mutex_unlock(&info->dfx_lock);
        devdrv_drv_err("devid %u memset_s error, %d. \n", dev_id, ret);
        return FAILED;
    }

    mutex_unlock(&info->dfx_lock);

    return 0;
}

/*
 * LLC timer handler.
 */
STATIC int devdrv_llc_set_sample_para(llc_info_t *llc_info, unsigned int *cur_sample_type)
{
    *cur_sample_type = LLC_PROF_TYPE_NONE;
    if (llc_info->dfx_state == DFX_STAT_ALL_RUNNING) {
        if (llc_info->cur_sample_host == LLC_DFX_HOST_DSMI) {
            llc_info->cur_sample_host = LLC_DFX_HOST_PROF;
            *cur_sample_type = llc_info->prof_info.sample_type;
        } else {
            llc_info->cur_sample_host = LLC_DFX_HOST_DSMI;
            *cur_sample_type = llc_info->dsmi_info.sample_type;
        }
    } else if (llc_info->dfx_state == DFX_STAT_DSMI_RUNNING) {
        llc_info->cur_sample_host = LLC_DFX_HOST_DSMI;
        *cur_sample_type = llc_info->dsmi_info.sample_type;
    } else if (llc_info->dfx_state == DFX_STAT_PROF_RUNNING) {
        llc_info->cur_sample_host = LLC_DFX_HOST_PROF;
        *cur_sample_type = llc_info->prof_info.sample_type;
    } else if (llc_info->dfx_state == DFX_STAT_NONE) {
        devdrv_drv_warn("no sample event now.\n");
        return -EINVAL;
    }

    return OK;
}

/*
 * LLC timer handler.
 */
STATIC void devdrv_llc_dfx_handler(unsigned long info)
{
    int ret;
    ktime_t kt;
    unsigned int cur_sample_type = LLC_PROF_TYPE_NONE;
    llc_info_t *llc_info = (llc_info_t *)(uintptr_t)info;
    int dev_id = llc_info->dev_id;

    if (llc_info->dfx_state == DFX_STAT_NONE) {
        devdrv_drv_info("dev_id[%d] there has no sample event now.\n", dev_id);
        return;
    }

    /* get current statistic info and update sample info */
    ret = devdrv_llc_get_stat_info(dev_id, &llc_info->cur_evt_info);
    if (ret < 0) {
        devdrv_drv_err("Failed to invoke devdrv_llc_get_stat_info. (devid=%d; ret=%d).\n", dev_id, ret);
        goto fail;
    }

    ret = devdrv_llc_update_stat_info(dev_id);
    if (ret < 0) {
        devdrv_drv_err("devid %d devdrv_llc_update_stat_info error ret = %d.\n", dev_id, ret);
        goto fail;
    }

    /* decide sample type, dsmi or profiling */
    ret = devdrv_llc_set_sample_para(llc_info, &cur_sample_type);
    if (ret != 0) {
        devdrv_drv_warn("devid %d devdrv_llc_set_sample_para no sample event now, ret = %d.\n", dev_id, ret);
        goto fail;
    }

    ret = devdrv_llc_enable_event_sample(dev_id, cur_sample_type);
    if (ret < 0) {
        devdrv_drv_err("devid %d devdrv_llc_enable_dfx_event_sample err ret = %d.\n", dev_id, ret);
        goto fail;
    }

    ret = devdrv_llc_get_stat_info(dev_id, &llc_info->bk_evt_info);
    if (ret < 0) {
        devdrv_drv_err("devid %d devdrv_llc_get_stat_info error ret = %d.\n", dev_id, ret);
        goto fail;
    }

    /* next period */
    ret = hrtimer_cancel(&llc_info->dfx_timer);
    if (ret) {
        devdrv_drv_warn("dev_id[%d] hrtimer is active now, ret = %d.\n", dev_id, ret);
    }

    kt = ktime_set(0, DFX_STAT_HRTIMER_PERIOD);
    hrtimer_start(&llc_info->dfx_timer, kt, HRTIMER_MODE_REL);
    return;

fail:
    ret = hrtimer_cancel(&llc_info->dfx_timer);
    if (ret) {
        devdrv_drv_warn("dev_id[%d] hrtimer is active now, ret = %d.\n", dev_id, ret);
    }
}

/*
 * initialize LLC lock and timer
 */
int devdrv_llc_init(unsigned int dev_id)
{
    s32 ret;
    hrtimer_timeout_handler handler = devdrv_llc_hrtimer_handler;

    /* firstly alloc memory */
    g_llc_info[dev_id] = (llc_info_t *)vzalloc(sizeof(llc_info_t));
    if (g_llc_info[dev_id] == NULL) {
        devdrv_drv_err("Alloc memory failed. (dev_id=%u)\n", dev_id);
        devdrv_llc_exit(dev_id);
        return -ENOMEM;
    }

    /* initialize mutex/hrtimer/tasklet */
    mutex_init(&g_llc_info[dev_id]->dsmi_lock);
    mutex_init(&g_llc_info[dev_id]->dfx_lock);
    hrtimer_init(&(g_llc_info[dev_id]->dfx_timer), CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    g_llc_info[dev_id]->dfx_timer.function = handler;
    tasklet_init(&g_llc_info[dev_id]->tasklet, devdrv_llc_dfx_handler, (unsigned long)(uintptr_t)g_llc_info[dev_id]);
    /* register memory mapping */
    g_llc_info[dev_id]->dev_id = dev_id;
    ret = devdrv_llc_reg_map(dev_id);
    if (ret != 0) {
        devdrv_drv_err("devdrv_llc_reg_map fail. (dev_id=%u; ret=%d)\n", dev_id, ret);
        devdrv_llc_exit(dev_id);
        return ret;
    }

    return 0;
}

/*
 * destroy mutex and cancel timer
 */
void devdrv_llc_exit(unsigned int dev_id)
{
    int ret;

    if (g_llc_info[dev_id] != NULL) {
        devdrv_llc_reg_unmap(dev_id);
        tasklet_kill(&g_llc_info[dev_id]->tasklet);
        mutex_destroy(&g_llc_info[dev_id]->dsmi_lock);
        mutex_destroy(&g_llc_info[dev_id]->dfx_lock);

        ret = hrtimer_cancel(&(g_llc_info[dev_id]->dfx_timer));
        if (ret != 0) {
            devdrv_drv_warn("hrtimer is active now. (dev_id=%u; ret=%d)\n", dev_id, ret);
        }

        vfree(g_llc_info[dev_id]);
        g_llc_info[dev_id] = NULL;
    }
}

#else
int devdrv_llc_prof_sample(struct prof_peri_para para)
{
    devdrv_drv_warn("ut test.\n");

    return 0;
}

int devdrv_llc_process_sample_data(unsigned int dev_id, llc_perf_para_t *para)
{
    if (dev_id >= 4) {
        return -16;
    }

    return 0;
}
#endif
