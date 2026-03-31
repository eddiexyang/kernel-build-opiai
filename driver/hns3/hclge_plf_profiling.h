/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: profiling
 * Author: huawei
 * Create: 2021-12-28
 */

#ifndef _HCLGE_PLF_PROFILING_H_
#define _HCLGE_PLF_PROFILING_H_

#include "drv_profile.h"

struct hclge_sample_stats_info {
    u64 bandwidth;
    u64 rx_packets;
    u64 rx_bytes;
    u64 rx_errors;
    u64 rx_dropped;
    u64 tx_packets;
    u64 tx_bytes;
    u64 tx_errors;
    u64 tx_dropped;
    struct timespec64 sample_time;
};

int higmac_sample_fun(struct prof_peri_para para);
#endif // __HCLGE_PLF_PROFILING_H
