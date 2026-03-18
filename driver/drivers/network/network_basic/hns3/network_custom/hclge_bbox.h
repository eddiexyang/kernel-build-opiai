/* SPDX-License-Identifier: GPL-2.0+ */
// Copyright (c) 2016-2017 Hisilicon Limited.

#ifndef __HCLGE_BBOX_H
#define __HCLGE_BBOX_H
#include <linux/types.h>

#define HCLGE_BBOX_CONFIG_CMD_RSV_NUM 12

struct hclge_bbox_config_cmd {
    __le64 log_addr;
    __le32 log_len;
    u8 rsv[HCLGE_BBOX_CONFIG_CMD_RSV_NUM];
};
#endif
