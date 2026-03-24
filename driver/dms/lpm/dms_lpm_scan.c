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
* Create: 2022-11-14
*/

#include <linux/kthread.h>
#include <linux/delay.h>

#include "dms_lpm.h"
#include "dms_lpm_common.h"
#include "dms_lpm_info.h"
#include "dms_cmd_def.h"
#include "dms_ipc_interface.h"
#include "dms_timer.h"

#define LPM_TIMER_INVALID 0xffff

#define SCAN_VALUE_INVALID 0xFFFFFFFF

#define LPM_SCAN_INFO_CYCLE 2000
#define LPM_SCAN_DELAY 10

#ifdef CFG_FEATURE_FORCE_NO_SCAN
#define SCAN_SWITCH 0
#else
#define SCAN_SWITCH 1
#endif

STATIC unsigned int g_timer_task_id = LPM_TIMER_INVALID;

STATIC int lpm_scan_period_check(struct lpm_retry_cycle *ctrl)
{
    if (ctrl->remain_cnt != 0) {
        ctrl->remain_cnt--;
        return -EINVAL;
    }

    return 0;
}

STATIC void lpm_scan_period_update(struct lpm_retry_cycle *ctrl, int result, u32 *value)
{
    struct timeval time_current = {0};
    long times_gap = 0;

    if (result == 0) {
        ctrl->remain_cnt = 0;
        ctrl->interval_cnt = 0;
        ctrl->prev_fail_time.tv_sec = 0;
        ctrl->prev_fail_time.tv_usec = 0;
        return;
    }

    *value = SCAN_VALUE_INVALID;

    (void)do_gettimeofday(&time_current);
    if (ctrl->prev_fail_time.tv_sec != 0 || ctrl->prev_fail_time.tv_usec != 0) {
        /* 1000 ms per second */
        times_gap = (time_current.tv_sec - ctrl->prev_fail_time.tv_sec) * 1000 +
                    (time_current.tv_usec - ctrl->prev_fail_time.tv_usec) / 1000; /* 1000 us per ms */
    }

    /* Stop when the interval exceeds 1 minute. The maximum interval is within 1 to 2 minutes. */
    if (times_gap < LPM_SCAN_FAILURE_INTERVAL_TIME_MAX) {
        ctrl->interval_cnt = ctrl->interval_cnt == 0 ? 1 : ctrl->interval_cnt * 2; /* 2: x=x*2 */
    }
    ctrl->prev_fail_time.tv_sec = time_current.tv_sec;
    ctrl->prev_fail_time.tv_usec = time_current.tv_usec;
    ctrl->remain_cnt = ctrl->interval_cnt;
}

STATIC struct lpm_scan_list g_lpm_scan_list[] = {
#ifdef CFG_FEATURE_GET_TV_FROM_IPC
    {lpm_get_temperature, DMS_SUBCMD_GET_TEMPERATURE, LPM_SOC_ID, {0}, {{0}}, SCAN_SWITCH},
    {lpm_get_voltage, DMS_SUBCMD_GET_VOLTAGE, LPM_CLUSTER_ID, {0}, {{0}}, SCAN_SWITCH},
#else
    {lpm_get_temperature, DMS_SUBCMD_GET_TEMPERATURE, LPM_SOC_ID, {0}, {{0}}, 0},
    {lpm_get_voltage, DMS_SUBCMD_GET_VOLTAGE, LPM_CLUSTER_ID, {0}, {{0}}, 0},
#endif
#ifdef CFG_FEATURE_GET_POWER_FROM_IPC
    {lpm_get_power, DMS_SUBCMD_GET_POWER, LPM_SOC_ID, {0}, {{0}}, SCAN_SWITCH},
#elif defined(CFG_FEATURE_GET_POWER_FROM_SHAREMEM)
    {lpm_get_power, DMS_SUBCMD_GET_POWER, LPM_SOC_ID, {0}, {{0}}, 0},
#endif
#ifdef CFG_REATURE_GET_AICPU_FREQ_FORM_IPC
    {lpm_get_frequency, DMS_SUBCMD_GET_FREQUENCY, LPM_CLUSTER_ID, {0}, {{0}}, SCAN_SWITCH},
#elif defined(CFG_REATURE_GET_AICPU_FREQ_FORM_SHAREMEM)
    {lpm_get_frequency, DMS_SUBCMD_GET_FREQUENCY, LPM_CLUSTER_ID, {0}, {{0}}, 0},
    {lpm_get_max_frequency, DMS_SUBCMD_GET_MAX_FREQUENCY, LPM_CLUSTER_ID, {0}, {{0}}, 0},
#endif
#ifdef CFG_REATURE_GET_AICORE_FREQ_FORM_IPC
    {lpm_get_frequency, DMS_SUBCMD_GET_FREQUENCY, LPM_AICORE0_ID, {0}, {{0}}, SCAN_SWITCH},
    {lpm_get_frequency, DMS_SUBCMD_GET_FREQUENCY, LPM_AICORE1_ID, {0}, {{0}}, SCAN_SWITCH},
#else
    {lpm_get_frequency, DMS_SUBCMD_GET_FREQUENCY, LPM_AICORE0_ID, {0}, {{0}}, 0},
    {lpm_get_frequency, DMS_SUBCMD_GET_FREQUENCY, LPM_AICORE1_ID, {0}, {{0}}, 0},
#endif
#ifdef CFG_REATURE_GET_HBM_FREQ_FORM_IPC
    {lpm_get_frequency, DMS_SUBCMD_GET_FREQUENCY, LPM_HBM_ID, {0}, {{0}}, SCAN_SWITCH},
#endif
#ifdef CFG_REATURE_GET_DDR_FREQ_FORM_IPC
    {lpm_get_frequency, DMS_SUBCMD_GET_FREQUENCY, LPM_DDR_ID, {0}, {{0}}, SCAN_SWITCH},
#elif defined(CFG_REATURE_GET_DDR_FREQ_FORM_SHAREMEM)
    {lpm_get_frequency, DMS_SUBCMD_GET_FREQUENCY, LPM_DDR_ID, {0}, {{0}}, 0},
#endif
#ifdef CFG_REATURE_GET_VECTOR_FREQ_FORM_IPC
    {lpm_get_frequency, DMS_SUBCMD_GET_FREQUENCY, LPM_VECTOR_ID, {0}, {{0}}, SCAN_SWITCH},
#endif
#ifdef CFG_FEATURE_GET_TEMP_FREQ_FROM_SHAREMEM
    {lpm_get_tsensor, DMS_SUBCMD_GET_TSENSOR, DMS_CLUSTER_TEMP_ID, {0}, {{0}}, 0},
    {lpm_get_tsensor, DMS_SUBCMD_GET_TSENSOR, DMS_PERI_TEMP_ID, {0}, {{0}}, 0},
    {lpm_get_tsensor, DMS_SUBCMD_GET_TSENSOR, DMS_AICORE0_TEMP_ID, {0}, {{0}}, 0},
    {lpm_get_tsensor, DMS_SUBCMD_GET_TSENSOR, DMS_AICORE_BASE_FREQ_ID, {0}, {{0}}, 0},
#endif
#ifdef CFG_REATURE_SOC_MAX_TEMP_FORM_IPC
    {lpm_get_tsensor, DMS_SUBCMD_GET_TSENSOR, DMS_SOC_TEMP_ID, {0}, {{0}}, SCAN_SWITCH},
#else
    {lpm_get_tsensor, DMS_SUBCMD_GET_TSENSOR, DMS_SOC_TEMP_ID, {0}, {{0}}, 0},
#endif
#ifdef CFG_REATURE_GET_HBM_TEMP_FORM_IPC
    {lpm_get_tsensor, DMS_SUBCMD_GET_TSENSOR, DMS_HBM_TEMP_ID, {0}, {{0}}, SCAN_SWITCH},
#elif defined(CFG_REATURE_GET_HBM_TEMP_FORM_SHAREMEM)
    {lpm_get_tsensor, DMS_SUBCMD_GET_TSENSOR, DMS_HBM_TEMP_ID, {0}, {{0}}, 0},
#endif
#ifdef CFG_REATURE_GET_N_DIE_TMEP_FORM_IPC
    {lpm_get_tsensor, DMS_SUBCMD_GET_TSENSOR, DMS_N_DIE_TEMP_ID, {0}, {{0}}, SCAN_SWITCH},
#elif defined(CFG_REATURE_GET_N_DIE_TMEP_FORM_SHAREMEM)
    {lpm_get_tsensor, DMS_SUBCMD_GET_TSENSOR, DMS_N_DIE_TEMP_ID, {0}, {{0}}, 0},
#endif
#ifdef CFG_GET_TSENSOR_INFO
    {lpm_get_tsensor, DMS_SUBCMD_GET_TSENSOR, DMS_THERMAL_THRESHOLD_ID, {0}, {{0}}, SCAN_SWITCH},
    {lpm_get_tsensor, DMS_SUBCMD_GET_TSENSOR, DMS_NPU_DDR_FREQ_ID, {0}, {{0}}, SCAN_SWITCH},
    {lpm_get_tsensor, DMS_SUBCMD_GET_TSENSOR, DMS_CLUSTER_TEMP_ID, {0}, {{0}}, 0},
    {lpm_get_tsensor, DMS_SUBCMD_GET_TSENSOR, DMS_PERI_TEMP_ID, {0}, {{0}}, 0},
    {lpm_get_tsensor, DMS_SUBCMD_GET_TSENSOR, DMS_AICORE0_TEMP_ID, {0}, {{0}}, 0},
    {lpm_get_tsensor, DMS_SUBCMD_GET_TSENSOR, DMS_AICORE1_TEMP_ID, {0}, {{0}}, 0},
    {lpm_get_tsensor, DMS_SUBCMD_GET_TSENSOR, DMS_AICORE_LIMIT_ID, {0}, {{0}}, 0},
    {lpm_get_tsensor, DMS_SUBCMD_GET_TSENSOR, DMS_AICORE_TOTAL_PER_ID, {0}, {{0}}, 0},
    {lpm_get_tsensor, DMS_SUBCMD_GET_TSENSOR, DMS_AICORE_ELIM_PER_ID, {0}, {{0}}, 0},
    {lpm_get_tsensor, DMS_SUBCMD_GET_TSENSOR, DMS_AICORE_BASE_FREQ_ID, {0}, {{0}}, 0},
    /* NTC_TMP not support scan, out value 4*sizeof(int) byte */
    {lpm_get_tsensor, DMS_SUBCMD_GET_TSENSOR, DMS_NTC_TEMP_ID, {0}, {{0}}, 0},
#endif

#ifdef CFG_REATURE_GET_INFO_FORM_IPC
    {lpm_get_status, DMS_SUBCMD_GET_LP_STATUS, LPM_SOC_ID, {0}, {{0}}, 0},
    {lpm_get_vol_cur, DMS_SUBCMD_GET_AIC_VOL_CUR, LPM_SOC_ID, {0}, {{0}}, 0},
    {lpm_get_vol_cur, DMS_SUBCMD_GET_HYBIRD_VOL_CUR, LPM_SOC_ID, {0}, {{0}}, 0},
    {lpm_get_vol_cur, DMS_SUBCMD_GET_TAISHAN_VOL_CUR, LPM_SOC_ID, {0}, {{0}}, 0},
    {lpm_get_vol_cur, DMS_SUBCMD_GET_DDR_VOL_CUR, LPM_SOC_ID, {0}, {{0}}, 0},
    {lpm_get_acg, DMS_SUBCMD_GET_ACG, LPM_SOC_ID, {0}, {{0}}, 0},
    {lpm_get_ddr_temp, DMS_SUBCMD_GET_TEMP_DDR, LPM_SOC_ID, {0}, {{0}}, 0},
    {lpm_get_temp_thold, DMS_SUBCMD_GET_DDR_THOLD, LPM_SOC_ID, {0}, {{0}}, 0},
    {lpm_get_temp_thold, DMS_SUBCMD_GET_SOC_THOLD, LPM_SOC_ID, {0}, {{0}}, 0},
    {lpm_get_temp_thold, DMS_SUBCMD_GET_SOC_MIN_THOLD, LPM_SOC_ID, {0}, {{0}}, 0},
    {lpm_set_temp_thold, DMS_SUBCMD_SET_TEMP_THOLD, LPM_SOC_ID, {0}, {{0}}, 0},
#endif
};

struct lpm_scan_list *find_operate_info_str(u32 type_id, u32 core_id)
{
    unsigned int i;
    unsigned int table_size = sizeof(g_lpm_scan_list) / sizeof(struct lpm_scan_list);

    for (i = 0; i < table_size; i++) {
        if ((type_id == g_lpm_scan_list[i].type_id) && (core_id == g_lpm_scan_list[i].core_id)) {
            return &g_lpm_scan_list[i];
        }
    }

    return NULL;
}

STATIC int lpm_scan_info(u64 arg)
{
    int ret;
    unsigned int i, dev_id;
    unsigned int num_dev = 0;
    struct lpm_retry_cycle *ctrl = NULL;
    unsigned int table_size = sizeof(g_lpm_scan_list) / sizeof(struct lpm_scan_list);

    ret = devdrv_get_devnum(&num_dev);
    if ((ret != 0) || (num_dev == 0) || (num_dev > LPM_DEVICE_NUM_MAX)) {
        dms_warn("Device num out of range. (num_dev=%u)\n", num_dev);
        return 0;
    }

    for (i = 0; i < table_size; i++) {
        for (dev_id = 0; dev_id < num_dev; dev_id++) {
            ctrl = &g_lpm_scan_list[i].period_ctrl[dev_id];
            if ((g_lpm_scan_list[i].scan_flag == 1) && (lpm_scan_period_check(ctrl) == 0)) {
                ret = g_lpm_scan_list[i].get_value_handle(dev_id, g_lpm_scan_list[i].core_id,
                    &g_lpm_scan_list[i].pf_value[dev_id]);
                lpm_scan_period_update(ctrl, ret, &g_lpm_scan_list[i].pf_value[dev_id]);
            }
            msleep(LPM_SCAN_DELAY);
        }
    }

    return 0;
}

int lpm_scan_init(void)
{
    int ret;
    struct dms_timer_task lpm_scan_task = {0};

    lpm_scan_task.expire_ms = LPM_SCAN_INFO_CYCLE;
    lpm_scan_task.handler_mode = COMMON_WORK;
    lpm_scan_task.exec_task = lpm_scan_info;

    ret = dms_timer_task_register(&lpm_scan_task, &g_timer_task_id);
    if (ret) {
        dms_err("Dms timer task register failed. (ret=%d) \n", ret);
        return ret;
    }

    return 0;
}

int lpm_scan_uninit(void)
{
    int ret;

    ret = dms_timer_task_unregister(g_timer_task_id);
    if (ret) {
        dms_err("Dms timer task unregister failed. (ret=%d) \n", ret);
        return ret;
    }

    return 0;
}

