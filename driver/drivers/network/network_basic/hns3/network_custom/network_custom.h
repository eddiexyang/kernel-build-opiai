/* SPDX-License-Identifier: GPL-2.0+ */
// Copyright (c) 2023 Hisilicon Limited.

#ifndef __NETWORK_CUSTOM_H
#define __NETWORK_CUSTOM_H
#include <linux/types.h>
#include <linux/spinlock_types.h>

enum hclge_custom_opcode_type {
    /* custom commands start 0xF000 */
    NETWORK_OPC_QUERY_OPTICAL_PRESENT         = 0xF000,
    NETWORK_OPC_QUERY_IMP_SHARE_MEM_INFO      = 0xF001,
    NETWORK_OPC_QUERY_MAC_SDS_RATE            = 0xF002,
    NETWORK_QUERY_ROCE_DFX_REG_NUM_CMD        = 0xF003,
    NETWORK_QUERY_ROCE_DFX_REG_INFO_CMD       = 0xF004,
    NETWORK_OPC_SET_OPTICAL_AUTO_ADAPT        = 0xF005,
};

enum connect_type {
    PCIE_CONNECT_TYPE       = 0x0,
    HCCS_CONNECT_TYPE       = 0x1,
    UNKNOWN_CONNECT_TYPE    = 0xff
};

enum {
    ROH_SDS_LINK_DO_ADAPT       = 0x1,
    ROH_SDS_LINK_ADAPT_DONE     = 0x2,
    ROH_SDS_LINK_ERROR          = 0xE,
    ROH_SDS_LINK_OFF            = 0xF
};

#define HCLGE_OPC_CONFIG_SERDES_ADAPT_STATUS    0x710A

#define SERDES_PRINT_TIMEOUT 60
#define LINK_STAT_MAX_IDX 10

struct hclge_link_his_stat {
    unsigned long long link_up_cnt;
    unsigned long long link_down_cnt;
    struct {
        unsigned int link_status;
        unsigned long long link_tv_sec;
    } stat[LINK_STAT_MAX_IDX];
    unsigned int stat_cnt;
    unsigned long long cur_tv_sec;
};

struct hclge_link_record {
    unsigned long long link_up_cnt;
    unsigned long long link_down_cnt;
    struct {
        unsigned int link_status;
        unsigned long long link_tv_sec;
    } his_stat[LINK_STAT_MAX_IDX];
    unsigned int link_stat_idx;
    spinlock_t lock;
};

struct custom_dev {
    struct hclge_dev *hdev;
    struct hclge_link_record link_record;
    u8 connect_type; /* 0: pcie connect, 1: hccs connect */
};

struct hclge_dev* hclge_dev_get(int dev_id);
extern int memset_s(void *dest, size_t destMax, int c, size_t count);
#endif
