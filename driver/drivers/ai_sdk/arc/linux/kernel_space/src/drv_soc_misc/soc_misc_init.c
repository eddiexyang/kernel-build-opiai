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

#include <linux/module.h>
#include <linux/types.h>

#include "soc_misc_init.h"
#include "soc_misc_info.h"
#include "soc_misc_feature.h"
#include "soc_misc_common.h"
#include "soc_misc_cpu_utilization.h"
#ifdef CFG_FEATURE_FAULT_MANAGER
#  include "soc_misc_dms_init.h"
#endif

#ifndef AOS_LLVM_BUILD
#include "devdrv_common.h"
#include "dms_notifier.h"
#else
#include "dms_define.h"
#endif

#ifndef AOS_LLVM_BUILD
STATIC int soc_misc_notifier(struct notifier_block *nb, unsigned long mode, void *data)
{
    int ret;

    if (data == NULL) {
        soc_misc_drv_err("data is null.\n");
        return -EINVAL;
    }

    switch (mode) {
        case DMS_DEVICE_UP0:
            ret = soc_misc_dev_info_init(data);
            if (ret != 0) {
                soc_misc_drv_err("Soc device info initialize failed. (ret=%d)\n", ret);
                return ret;
            }
            break;
        case DMS_DEVICE_SET_AICPU_NUM:
            ret = soc_misc_set_cpu_info(data);
            if (ret != 0) {
                soc_misc_drv_err("Set soc cpu info failed. (ret=%d)\n", ret);
                return ret;
            }
            break;
        case DMS_DEVICE_RESUME:
        case DMS_DEVICE_DOWN0:
        case DMS_DEVICE_SUSPEND:
        default:
            break;
    }
    return 0;
}

STATIC struct notifier_block g_soc_misc_dms_notifier = {
    .notifier_call = soc_misc_notifier,
};
#endif

STATIC int __init soc_misc_init(void)
{
    int ret;

#ifdef CFG_FEATURE_FAULT_MANAGER
    ret = soc_misc_fault_init();
    if (ret != 0) {
        soc_misc_drv_err("SOC info init failed. (ret=%d)\n", ret);
        return ret;
    }
#endif

#ifndef AOS_LLVM_BUILD
    ret = dms_register_notifier(&g_soc_misc_dms_notifier);
    if (ret != 0) {
        soc_misc_drv_err("register dms notifier failed. (ret=%d)\n", ret);
        goto notifier_fail;
    }

    ret = soc_misc_soc_info_init();
    if (ret != 0) {
        soc_misc_drv_err("SOC info init failed. (ret=%d)\n", ret);
        goto soc_info_init_fail;
    }

    ret = soc_misc_cpu_utilization_init();
    if (ret != 0) {
        soc_misc_drv_err("SOC cpu utilization init failed. (ret=%d)\n", ret);
        goto soc_cpu_uti_init_fail;
    }
    soc_misc_feature_init();
#endif
    soc_misc_drv_info("SOC driver init success.\n");
    return 0;

#ifndef AOS_LLVM_BUILD
soc_cpu_uti_init_fail:
    soc_misc_soc_info_uninit();
soc_info_init_fail:
    (void)dms_unregister_notifier(&g_soc_misc_dms_notifier);
notifier_fail:
#ifdef CFG_FEATURE_FAULT_MANAGER
    soc_misc_fault_exit();
#endif
    return ret;
#endif
}
module_init(soc_misc_init);

STATIC void __exit soc_misc_exit(void)
{
#ifdef CFG_FEATURE_FAULT_MANAGER
    soc_misc_fault_exit();
#endif
#ifndef AOS_LLVM_BUILD
    soc_misc_feature_uninit();
    soc_misc_cpu_utilization_exit();
    soc_misc_soc_info_uninit();
    (void)dms_unregister_notifier(&g_soc_misc_dms_notifier);
#endif
    soc_misc_drv_info("SOC driver exit success.\n");
}
module_exit(soc_misc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("DAVINCI driver");
