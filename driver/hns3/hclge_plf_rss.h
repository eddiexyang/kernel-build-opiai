/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: hclge_plf_rss
 * Author: huawei
 * Create: 2021-12-28
 */

#ifndef __HCLGE_PLF_RSS_H
#define __HCLGE_PLF_RSS_H

#include <linux/types.h>
#include "hnae3.h"

#define PPP_RSS_TC_MODE_DATA 0x3508
#define PPP_RSS_TC_MODE_CFG 0x350c
#define PPP_RSS_IDT_ALLOC_DATA 0x3518
#define PPP_PKT_TC_OVRD_CFG 0x351c
#define PPP_RSS_IDT_DATA 0x3540
#define PPP_RSS_IDT_CFG 0x3580
#define PPP_RSS_HASH_KEY_9 0x35c0
#define PPP_RSS_HASH_ALGO 0x35c4
#define PPP_RSS_HASH_CFG 0x35c8
#define PPP_RSS_TYPE_IPV4 0x3594
#define PPP_RSS_TYPE_IPV6 0x3598

#define HCLGE_PLF_RSS_CFG_TBL_SIZE  16

#define HCLGE_PLF_RSS_IND_TBL_SIZE_MAX	256
#define HCLGE_PLF_RSS_IND_TBL_SIZE		64
#define HCLGE_PLF_RSS_SET_BITMAP_MSK	GENMASK(15, 0)
#define HCLGE_PLF_RSS_KEY_SIZE_MAX		40
#define HCLGE_PLF_RSS_KEY_SIZE		40
#define HCLGE_PLF_RSS_HASH_ALGO_TOEPLITZ	0
#define HCLGE_PLF_RSS_HASH_ALGO_SIMPLE	1
#define HCLGE_PLF_RSS_HASH_ALGO_SYMMETRIC	2
#define HCLGE_PLF_RSS_HASH_ALGO_MASK	GENMASK(3, 0)

#define HCLGE_PLF_RSS_INPUT_TUPLE_OTHER	GENMASK(3, 0)
#define HCLGE_PLF_RSS_INPUT_TUPLE_SCTP	GENMASK(4, 0)
#define HCLGE_PLF_RSS_INPUT_TUPLE_SCTP_NO_PORT \
    (HCLGE_PLF_D_IP_BIT | HCLGE_PLF_S_IP_BIT | HCLGE_PLF_V_TAG_BIT)
#define HCLGE_PLF_D_PORT_BIT		BIT(0)
#define HCLGE_PLF_S_PORT_BIT		BIT(1)
#define HCLGE_PLF_D_IP_BIT			BIT(2)
#define HCLGE_PLF_S_IP_BIT			BIT(3)
#define HCLGE_PLF_V_TAG_BIT			BIT(4)

void hclge_plf_rss_init_cfg(struct hclge_plf_dev *hdev);
void hclge_plf_rss_cap_init(struct hclge_plf_dev *hdev);
int hclge_plf_rss_init_hw(struct hclge_plf_dev *hdev);
int hclge_plf_set_rss_tc_mode(struct hclge_plf_dev *hdev, u32 *tc_valid, u32 *tc_size, u32 *tc_offset);
int hclge_plf_set_rss(struct hnae3_handle *handle, const u32 *indir, const u8 *key, const u8 hfunc);
int hclge_plf_get_rss(struct hnae3_handle *handle, u32 *indir, u8 *key, u8 *hfunc);
int hclge_plf_set_rss_tuple(struct hnae3_handle *handle, struct ethtool_rxnfc *nfc);
int hclge_plf_get_rss_tuple(struct hnae3_handle *handle, struct ethtool_rxnfc *nfc);
#endif // __HCLGE_PLF_RSS_H
