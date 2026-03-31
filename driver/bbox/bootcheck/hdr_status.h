/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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
 * Create: 2022-08-13
 */

#ifndef HDR_STATUS_H
#define HDR_STATUS_H

/**
 *  the whole space is 512k, used for history data record
 *  status region use 31k
 *  the struct distribution is as follows:
 *  +-----------------------+
 *  | hdr log head(1k)      |
 *  +-----------------------+
 *  | boot log region(300k) |
 *  +-----------------------+
 *  | run log region(150k)  |
 *  +-----------------------+
 *  | hdr status head(1k)   |     status region:10           area:128                  block:24B
 *  +-----------------------+     +--------------------+     +-------------------+     +-----------------+
 *  | status region(30k)    |---->| first area(3k)     |---->| block(hboot)(24B) |---->| status_block    |
 *  +-----------------------+     +--------------------+     +-------------------+     +-----------------+
 *  | reserved(30k)         |     | ......             |     | ......            |
 *  +-----------------------+     +--------------------+     +-------------------+
 */

#define STATUS_BLOCK_INVALID    0x0
#define STATUS_BLOCK_VALID 0x76

/*
 * status block content
 * struct occupy 24B
 */
struct status_block {
    u32 magic;             // 组件自己定
    u8 valid;              // STATUS_BLOCK_VALID
    u8 block_id;           // 设定的id
    u8 arg_1;              // 辅助参数
    u8 arg_2;              // 辅助参数
    u32 exception_id;      // 填写异常码
    u32 expect_status;     // 期待的最终状态
    u32 current_status;    // 当前的状态
    u32 reserve;
};

#define STATUS_AREA_MAX_NUM     10U
#define STATUS_AREA_USED_NUM    5
#define STATUS_BLOCK_MAX_NUM    128U
#define STATUS_BLOCK_USED_NUM   100

/*
 * region config:
 * area size: region size / total_area_num
 * block size: area size / total_block_num
 * struct occupy 16B
 */
struct status_region_config {
    u32 total_area_num;    // total number: STATUS_AREA_MAX_NUM
    u32 used_area_num;     // used number: STATUS_AREA_USED_NUM
    u32 total_block_num;   // total number: STATUS_BLOCK_MAX_NUM
    u32 used_block_num;    // used number: STATUS_BLOCK_USED_NUM
};

#define STATUS_AREA_EMPTY       0x0
#define STATUS_AREA_CURRENT     0x4355
#define STATUS_AREA_HISTORY     0x4849
#define STATUS_AREA_RW_INIT     0x0
#define STATUS_AREA_WRITE_DONE  0x5752
#define STATUS_AREA_READ_DONE   0x5245
#define STATUS_AREA_EXCEPTION   0x1
#define STATUS_AREA_NORMAL      0x0

/*
 * area ctrl:
 * struct occupy 16B
 */
struct status_area_ctrl {
    u32 type;              // STATUS_AREA_EMPTY, STATUS_AREA_CURRENT, STATUS_AREA_HISTORY
    u32 rw_flag;           // STATUS_AREA_RW_INIT, STATUS_AREA_WRITE_DONE, STATUS_AREA_READ_DONE
    u32 reset_num;         // hot reset number
    u8 exception;
    u8 reserve[3];
};

/*
 * region ctrl:
 * struct occupy 168B
 */
struct status_region_ctrl {
    u32 area_index;        // 当前使用的区域序号，用于BIOS搬移当前区到对应序号的历史区，1-4, 0用于current
    u32 reserve;
    struct status_area_ctrl area_ctrl[STATUS_AREA_MAX_NUM];
};

/*
 * region info:
 * struct occupy 192B
 */
struct status_region_info {
    u32 offset;                // region offset
    u32 size;                  // region size
    struct status_region_config cfg;    // region config, BIOS filling in
    struct status_region_ctrl ctrl;     // region ctrl
};

#define HDR_STATUS_MAGIC        0x48445253
#define HDR_STATUS_VERSION      0x0100  // 0x0100表示V1.0

/*
 * status head
 * struct occupy 200B
 */
struct hdr_status_head {
    u32 magic;                 // HDR_STATUS_MAGIC
    u32 version;               // HDR_STATUS_VERSION
    struct status_region_info region;
};

#endif
