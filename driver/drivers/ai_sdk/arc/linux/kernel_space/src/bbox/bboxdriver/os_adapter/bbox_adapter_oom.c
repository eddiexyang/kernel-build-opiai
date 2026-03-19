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

#include "bbox_adapter_oom.h"
#include <linux/atomic.h>
#include <linux/oom.h>
#include <linux/jiffies.h>
#include "bbox_platform.h"
#include "bbox_product.h"
#include "bbox_adapter.h"
#include "device/bbox_types.h"
#include "rdr/rdr_dump_core.h"

#define OOM_NO_OPERATION 0
#define OOM_IN_OPERATION 1
#define OOM_TRIGGER_INTERVAL (10UL * (u32)HZ)

static atomic_t g_oom_in_opt = ATOMIC_INIT(0);
STATIC bbox_jiffies_t g_last_jiffies = 0;

/*
 * @brief       : process oom
 * @return      : != 0 failure; ==0 success
 */
STATIC s32 bbox_oom_process(void)
{
    struct bbox_time tm = {0, 0};
    static atomic64_t skip_cnt;
    s64 tmp_cnt;

    if (atomic_cmpxchg(&g_oom_in_opt, OOM_NO_OPERATION, OOM_IN_OPERATION) == OOM_IN_OPERATION) {
        (void)atomic64_add_unless(&skip_cnt, 1, S64_MAX);
        return BBOX_SUCCESS;
    }

    BB_PRINT_INFO("system oom happened, trigger exception!\n");
    tmp_cnt = atomic64_xchg(&skip_cnt, 0);
    if (tmp_cnt != 0) {
        BB_PRINT_INFO("skip oom trigger for %lld times while former one in operation.\n", tmp_cnt);
    }

    bbox_get_systime(&tm);
    bbox_update_time_seq(&tm);
    rdr_save_history_log_for_oom(DEFAULT_DEVICE_ID, BBOX_OS, OS_OOM, (u32)EXCEPID_AP_OOM, &tm);
    bbox_adapter_save_oom_log(DEFAULT_DEVICE_ID, (u32)EXCEPID_AP_OOM, OS_OOM, BBOX_OS, &tm);
    bbox_adapter_flush_log_cache();
    atomic_set(&g_oom_in_opt, OOM_NO_OPERATION);
    return BBOX_SUCCESS;
}

/*
 * @brief       : oom notify callback function
 * @param [in]  : struct notifier_block *self   register notifier
 * @param [in]  : bbox_event_t event            event
 * @param [in]  : bbox_arg_t *arg               arg
 * @return      : != 0 failure; ==0 success
 */
STATIC s32 bbox_oom_callback(struct notifier_block *self, bbox_event_t event, bbox_arg_t *arg)
{
    UNUSED(self);
    UNUSED(event);
    UNUSED(arg);

    // 10s interval trigger
    if (time_after(jiffies, g_last_jiffies + OOM_TRIGGER_INTERVAL)) {
        g_last_jiffies = jiffies;
        return bbox_oom_process();
    }

    return BBOX_SUCCESS;
}

static struct notifier_block g_ap_oom_notifier = {
    .notifier_call = bbox_oom_callback,
};

/*
 * @brief       : register oom notify
 * @return      : != 0 failure; ==0 success
 */
s32 bbox_oom_init(void)
{
    if (bbox_check_feature(FEATURE_OOM) == true) {
        return register_oom_notifier(&g_ap_oom_notifier);
    } else {
        return BBOX_SUCCESS;
    }
}

/*
 * @brief       : unregister oom notify
 * @return      : NA
 */
void bbox_oom_exit(void)
{
    if (bbox_check_feature(FEATURE_OOM) == true) {
        s32 ret = unregister_oom_notifier(&g_ap_oom_notifier);
        BB_CHECK_RET(ret != 0, "unregister oom notifier failed with %d.\n", ret);
    }
}
