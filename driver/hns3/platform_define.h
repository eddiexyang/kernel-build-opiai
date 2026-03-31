/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: platform_define
 * Author: huawei
 * Create: 2021-12-28
 */

#ifndef __PLATFORM_DEFINE_H__
#define __PLATFORM_DEFINE_H__

#include <linux/platform_device.h>
#include <linux/device.h>
#include "kcompat.h"
#include "hclge_plf_main.h"
#include "hclge_plf_tm.h"
#include "hns3_enet.h"

#define hclge_get_vport hclge_plf_get_vport

#define hclge_tm_prio_tc_info_update hclge_plf_tm_prio_tc_info_update
#define hclge_tm_schd_info_update hclge_plf_tm_schd_info_update

#define hclge_tm_schd_setup_hw hclge_plf_tm_schd_setup_hw
#define hclge_pause_setup_hw hclge_plf_pause_setup_hw
#define hclge_buffer_alloc hclge_plf_buffer_alloc
#define hclge_rss_indir_init_cfg hclge_plf_rss_indir_init_cfg
#define hclge_rss_init_hw hclge_plf_rss_init_hw
#define hclge_notify_client hclge_plf_notify_client
#define hclge_tm_dwrr_cfg hclge_plf_tm_dwrr_cfg

#define hclge_pfc_tx_stats_get hclge_plf_pfc_tx_stats_get
#define hclge_pfc_rx_stats_get hclge_plf_pfc_rx_stats_get

#define hclge_tm_pfc_info_update hclge_plf_tm_pfc_info_update

#endif // __PLATFORM_DEFINE_H