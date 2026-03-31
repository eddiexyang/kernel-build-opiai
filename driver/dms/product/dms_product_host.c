/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description:
 * Author: Huawei
 * Create: 2023-10-10
 * File Name     : dms_product_host.c
 * Version       : Initial product
 * Author        :
 * Created       : 2023-10-10
 * Last Modified :
 * Description   :

 *  History       :
 * 1.Date        : 2023-10-10
 *    Author      :
 *    Modification: Created file
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>

#include "dms_template.h"
#include "dms_cmd_def.h"
#include "dms_acc_ctrl.h"
#include "devdrv_user_common.h"
#include "devdrv_manager_common.h"
#include "dms_product.h"
#include "dms_product_host.h"


int devdrv_get_work_mode(void *feature, char *in, u32 in_len, char *out, u32 out_len)
{
    unsigned int work_mode;
    int ret;
#ifndef CFG_SOC_PLATFORM_CLOUD
    struct devdrv_manager_info *manager_info = NULL;
#endif

    if (out == NULL || (out_len != sizeof(unsigned int))) {
        dms_err("Out char is NULL or in_len is wrong.");
        return -EINVAL;
    }

#ifdef CFG_SOC_PLATFORM_CLOUD
    ret = devdrv_manager_get_amp_smp_mode(&work_mode);
    if (ret) {
        dms_err("Get amp smp mode failed. (ret=%d)\n", ret);
        return -EFAULT;
    }
#else
    manager_info = devdrv_get_manager_info();
    if (manager_info == NULL) {
        dms_err("Manager_info is Null\n");
        return -EINVAL;
    }
    work_mode = manager_info->amp_or_smp;
#endif

    ret = memcpy_s((void *)out, out_len, (void *)&work_mode, sizeof(unsigned int));
    if (ret) {
        dms_err("Call memcpy_s failed. (ret=%d)\n", ret);
        return -EINVAL;
    }

    return ret;
}