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
*/
#ifndef LPM_DEVMNG_API_H
#define LPM_DEVMNG_API_H

#include <linux/of.h>
#include <linux/io.h>
#include "drv_profile.h"
#include "lpm_devmng_common.h"

#ifdef LPM_AIC_FREQ_SYSCNT
/*
 * return value:
 * = 0 ：no valid data or no new data
 * > 0 ： the length of filled buff data.
 * < 0 ： profiling data failed
 * Constraint information, please read carefully:
 * 1: device_id must be passed to udevid
 *    udevid: unified device id, include davinci physical device, mia device and kunpeng device,
              phy_devid is equal to udevid
 * 2: There is a sleep operation inside the api
 * 3: There is a record log operation inside the api
 * 4: The api uses a mutex lock to protect the same device_id data
 * 5: If the function is not supported, return -EOPNOTSUPP
 * 6: Only incremental data is read each time, and the call entry needs to be kept unique
 * 7: The data returned by the api is incremental data.
 *    In order to avoid data confusion, this api is only for profiling calls.
 *    Please do not call others without permission.
 * 8: If there are more than 55 pieces of frequency conversion information
 *    in one query cycle, there will be data loss
 * 9: One copy of syscnt and freqency information for each physical device,
 *    virtual machines of the same physical machine cannot collect LP channels at the same time
 */
int32_t lpm_devmng_prof_sample_syscnt_handle(struct prof_peri_para para);
#endif

#endif