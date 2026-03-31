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

#include <linux/securec.h>
#include <linux/securectype.h>
#include "devdrv_common.h"
#include "devdrv_user_common.h"
#include "devdrv_dfm.h"
#include "devdrv_manager_common.h"
#include "devdrv_parse_pdata.h"
#include "tsdrv_status.h"

#define DEVDRV_MAX_DFM_LOG_SIZE 1024
#define DEVDRV_MAX_DFM_MAX_LEN 16
STATIC char devdrv_dfm_log[DEVDRV_MAX_DFM_LOG_SIZE];
STATIC const char devdrv_status_str[TS_MAX_STATUS][DEVDRV_MAX_DFM_MAX_LEN] = {
    [TS_WORK] = "working",
    [TS_SUSPEND] = "sleep",
    [TS_DOWN] = "exception",
    [TS_FAIL_TO_SUSPEND] = "fail to suspend",
    [TS_FW_VERIFY_FAIL]  = "fw verify error",
    [TS_INITING] = "initing",
    [TS_BOOTING] = "booting",
};

#if defined(CFG_SOC_PLATFORM_CLOUD)

STATIC void devdrv_dfm_dump(u32 dev_id, u32 excep_id, u32 etype, u32 module_id, char **black_box_info)
{
    struct devdrv_manager_info *manager_info = devdrv_get_manager_info();
    struct devdrv_info *dev_info = NULL;
    const char *status = NULL;
    enum devdrv_ts_status ts_status;
    int tlen;

    if (black_box_info == NULL) {
        devdrv_drv_err("dev_id = %u, invalid handler.\n", dev_id);
        return;
    }

    if (manager_info == NULL) {
        *black_box_info = NULL;
        return;
    }

    dev_info = manager_info->dev_info[0];
    if (dev_info == NULL) {
        *black_box_info = NULL;
        return;
    }

    ts_status = tsdrv_get_ts_status(dev_info->dev_id, 0);
    if (ts_status >= TS_MAX_STATUS) {
        *black_box_info = NULL;
        return;
    }

    status = devdrv_status_str[ts_status];

    tlen = snprintf_s(devdrv_dfm_log, DEVDRV_MAX_DFM_LOG_SIZE, DEVDRV_MAX_DFM_LOG_SIZE - 1,
                      "devid: %u device manager current status: %s.\n", dev_id, status);
    if (tlen < 0) {
        devdrv_drv_err("devid: %u device manager current status: %s. snprintf_s fail\n", dev_id, status);
        *black_box_info = NULL;
        return;
    }

    *black_box_info = devdrv_dfm_log;
}

STATIC void devdrv_dfm_dump_ex(u32 dev_id, u32 excep_id, u32 etype, u32 module_id, char **black_box_info)
{
    return;
}
#else

STATIC void devdrv_dfm_dump(u32 dev_id, u32 excep_id, u32 etype, u32 module_id, char **black_box_info)
{
    struct devdrv_manager_info *manager_info = devdrv_get_manager_info();
    struct devdrv_info *dev_info = NULL;
    enum devdrv_ts_status ts_status;
    const char *status = NULL;
    int tlen;

    if (black_box_info == NULL) {
        devdrv_drv_err("invalid handler.\n");
        return;
    }

    if (manager_info == NULL) {
        *black_box_info = NULL;
        return;
    }

    dev_info = manager_info->dev_info[0];
    if (dev_info == NULL) {
        *black_box_info = NULL;
        return;
    }

    ts_status = tsdrv_get_ts_status(dev_info->dev_id, 0);
    if (ts_status >= TS_MAX_STATUS) {
        *black_box_info = NULL;
        return;
    }

    status = devdrv_status_str[ts_status];

    tlen = snprintf_s(devdrv_dfm_log, DEVDRV_MAX_DFM_LOG_SIZE, DEVDRV_MAX_DFM_LOG_SIZE - 1,
                      "device manager current status: %s.\n", status);
    if (tlen < 0) {
        *black_box_info = NULL;
        devdrv_drv_err("device manager current status: %s. snprintf_s fail\n", status);
        return;
    }

    *black_box_info = devdrv_dfm_log;
}

STATIC void devdrv_dfm_dump_ex(u32 dev_id, u32 excep_id, u32 etype, u32 module_id, char **black_box_info)
{
    return;
}
#endif

int devdrv_black_box_init(void)
{
    struct dfm_module_register dfm_module_info = {0};
    int ret;

    dfm_module_info.module_id = DFM_MODULE_ID_DRIVER;
    dfm_module_info.sub_module_id = DFM_SUBMODULE_ID_DEVMNG;
    dfm_module_info.ops_dump = devdrv_dfm_dump;
    dfm_module_info.ops_dump_ex = devdrv_dfm_dump_ex;

    ret = dfm_register_module(&dfm_module_info);
    if (ret) {
        devdrv_drv_err("dfm_register_module failed, ret = %d.\n", ret);
        return ret;
    }

    return 0;
}

void devdrv_black_box_exit(void)
{
    (void)dfm_unregister_module(DFM_MODULE_ID_DRIVER, DFM_SUBMODULE_ID_DEVMNG);
}
