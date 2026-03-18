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

#ifndef _HCLGE_PROFILING_H_
#define _HCLGE_PROFILING_H_

#include "drv_profile.h"

struct hclge_sample_stats_info {
    u64 bandwidth;
    u64 rxPackets;
    u64 rxBytes;
    u64 rxErrors;
    u64 rxDropped;
    u64 txPackets;
    u64 txBytes;
    u64 txErrors;
    u64 txDropped;
    struct timespec64 sample_time;
};

int hns_roce_prof_sample(struct prof_peri_para para);
int higmac_sample_fun(struct prof_peri_para para);
#endif
