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

#define DEVDRV_MAX_DFM_LOG_SIZE 1024
#define DEVDRV_MAX_DFM_MAX_LEN 16
#define APPMON_MAX_EXCEPT_ID 24
STATIC char devdrv_dfm_log[DEVDRV_MAX_DFM_LOG_SIZE];
STATIC u32 g_appmon_except_id[APPMON_MAX_EXCEPT_ID] = {
    0xA4025021, 0xA6025022, 0x94025021, 0xA4025041, 0xA6025042, 0x94025041,
    0xA4025061, 0xA8025062, 0x94025061, 0xA4025081, 0xA8025082, 0x94025081,
    0xA60250A1, 0xA80250A2, 0x960250A1, 0xA60250C1, 0xA80250C2, 0x960250C1,
    0xA40250E1, 0xA60250E2, 0x940250E1, 0xA4025101, 0xA8025102, 0x94025101
};
int appmon_except_id_check(u32 excep_id)
{
    int i;

    for (i = 0; i < APPMON_MAX_EXCEPT_ID; i++) {
        if (excep_id == g_appmon_except_id[i]) {
            return 0;
        }
    }

    devdrv_drv_warn("invalid excep_id : 0x%x\n", excep_id);
    return -EINVAL;
}

#if defined(CFG_SOC_PLATFORM_CLOUD)
STATIC void appmon_dfm_dump(u32 dev_id, u32 excep_id, u32 etype, u32 module_id, char **black_box_info)
{
    int tlen;

    if (black_box_info == NULL) {
        devdrv_drv_err("dev_id = %u, invalid handler.\n", dev_id);
        return;
    }

    tlen = snprintf_s(devdrv_dfm_log, DEVDRV_MAX_DFM_LOG_SIZE, DEVDRV_MAX_DFM_LOG_SIZE - 1, "excep_id: 0x%x .\n",
        excep_id);
    if (tlen < 0) {
        devdrv_drv_err("dev_id = %u, excep_id: 0x%x ascend_monitor have errors: snprintf_s fail\n", dev_id, excep_id);
        *black_box_info = NULL;
        return;
    }

    *black_box_info = devdrv_dfm_log;
}

STATIC void appmon_dfm_dump_ex(u32 dev_id, u32 excep_id, u32 etype, u32 module_id, char **black_box_info)
{
    return;
}
#else
STATIC void appmon_dfm_dump(u32 excep_id, u32 etype, u32 module_id, char **black_box_info)
{
    int tlen;

    if (black_box_info == NULL) {
        devdrv_drv_err("invalid handler.\n");
        return;
    }

    tlen = snprintf_s(devdrv_dfm_log, DEVDRV_MAX_DFM_LOG_SIZE, DEVDRV_MAX_DFM_LOG_SIZE - 1, "excep_id: 0x%x .\n",
        excep_id);
    if (tlen < 0) {
        *black_box_info = NULL;
        devdrv_drv_err("excep_id: 0x%x ascend_monitor have errors: snprintf_s fail\n", excep_id);
        return;
    }

    *black_box_info = devdrv_dfm_log;
}

STATIC void appmon_dfm_dump_ex(u32 excep_id, u32 etype, u32 module_id, char **black_box_info)
{
    return;
}
#endif

int appmon_black_box_init(void)
{
    struct dfm_module_register dfm_module_info = {0};
    int ret;

    dfm_module_info.module_id = DFM_MODULE_ID_DRIVER;
    dfm_module_info.sub_module_id = DFM_SUBMODULE_ID_APPMOND;
    dfm_module_info.ops_dump = appmon_dfm_dump;
    dfm_module_info.ops_dump_ex = appmon_dfm_dump_ex;

    ret = dfm_register_module(&dfm_module_info);
    if (ret) {
        devdrv_drv_err("[ascend_monitor]dfm_register_module failed. ret = %d.\n", ret);
        return ret;
    }

    return 0;
}

void appmon_black_box_exit(void)
{
    (void)dfm_unregister_module(DFM_MODULE_ID_DRIVER, DFM_SUBMODULE_ID_APPMOND);
}
