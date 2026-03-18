/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
 * Create: 2023-01-03
 */
#ifdef CFG_FEATURE_HCCS_FAULT
#include "soc_misc_hccs.h"
#endif

#ifdef CFG_FEATURE_RBRG_FAULT
#include "soc_misc_rbrg.h"
#endif

#include "soc_misc_cpucore.h"
#include "devdrv_manager_comm.h"
#include "soc_misc_ras.h"

int soc_misc_init_ras_fault(void)
{
    int ret = 0;
    int ret_tmp;
    unsigned int dev_num = 0;
    int i;

#ifdef CFG_FEATURE_RBRG_FAULT
    if (soc_misc_rbrg_register(0) != 0)  {
        ret = -1;
    }
#endif

    ret_tmp = devdrv_get_devnum(&dev_num);
    if (ret_tmp != 0 || dev_num == 0) {
        dev_num = 0;
        ret = -1;
    }
    for (i = 0; i < dev_num; i++) {
        if (soc_misc_cpucore_register(i) != 0)  {
            ret = -1;
        }
    }

#ifdef CFG_FEATURE_HCCS_FAULT
    for (i = 0; i < dev_num; i++) {
        if (soc_misc_hccs_register(i) != 0) {
            ret = -1;
        }
    }
#endif

    return ret;
}

void soc_misc_uninit_ras_fault(void)
{
#ifdef CFG_FEATURE_RBRG_FAULT
    soc_misc_rbrg_unregister();
#endif

    soc_misc_cpucore_unregister();

#ifdef CFG_FEATURE_HCCS_FAULT
    soc_misc_hccs_unregister();
#endif
}
