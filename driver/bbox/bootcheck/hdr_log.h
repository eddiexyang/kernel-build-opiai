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

#ifndef HDR_LOG_H
#define HDR_LOG_H

/**
 *  the whole space is 512k, used for history data record
 *  the struct distribution is as follows:
 *  +-----------------------+
 *  | hdr log head(1k)      |     region:                    area:                     module:
 *  +-----------------------+     +--------------------+     +-------------------+     +-----------------+
 *  | boot log region(300k) |---->| first area(30/15k) |---->| module(BIOS)      |---->| module head     |
 *  +-----------------------+     +--------------------+     +-------------------+     +-----------------+
 *  | run log region(150k)  |     | second area        |     | module(DDR)       |     | module data     |
 *  +-----------------------+     +--------------------+     +-------------------+     +-----------------+
 *  | hdr status head(1k)   |     | ......             |     | ......            |
 *  +-----------------------+     +--------------------+     +-------------------+
 *  | status region(30k)    |
 *  +-----------------------+
 *  | reserved(30k)         |
 *  +-----------------------+
 */

/* area num */
#define AREA_MAX_NUM                10U
#define HIS_AREA_CFG_NUM            5
#define ERR_AREA_CFG_NUM            2
#define RESERVED_AREA_CFG_NUM       3

/* module block: */
#define MODULE_IS_USED              1
#define MODULE_IS_NOT_USED          0
#define MODULE_ERRCODE_NO_ERR       0


struct module_head {
    u32 magic;
    u32 version;
    u32 module_id;         // 模块id，黑匣子分配
    u32 is_used;           // 0没有使用，1使用
    u32 err_code;          // 填写异常码，0本模块没有err
    u32 reason;            // 具体异常原因：如原始reg0和reg10的值
    u32 reset_cnt;         // 当前热复位次数
    char data[0];
};

/* area */
#define MODULE_MAX_NUM 10U
enum BOOT_MODULE_INDEX {
    BOOT_MODULE_BIOS,      // 12KB
    BOOT_MODULE_DDR,       // 4KB
    BOOT_MODULE_TEE,       // 4KB
    BOOT_MODULE_ATF,       // 4KB
    BOOT_MODULE_RESERVED1, // 2KB
    BOOT_MODULE_RESERVED2, // 2KB
    BOOT_MODULE_RESERVED3, // 2KB
    BOOT_MODULE_MAX
};

#define BOOT_MODULE_BIOS_OFFSET           0x0
#define BOOT_MODULE_BIOS_SIZE             0x3000
#define BOOT_MODULE_DDR_OFFSET            0x3000
#define BOOT_MODULE_DDR_SIZE              0x1000
#define BOOT_MODULE_TEE_OFFSET            0x4000
#define BOOT_MODULE_TEE_SIZE              0x1000
#define BOOT_MODULE_ATF_OFFSET            0x5000
#define BOOT_MODULE_ATF_SIZE              0x1000
#define BOOT_MODULE_RESERVED1_OFFSET      0x6000
#define BOOT_MODULE_RESERVED1_SIZE        0x800
#define BOOT_MODULE_RESERVED2_OFFSET      0x6800
#define BOOT_MODULE_RESERVED2_SIZE        0x800
#define BOOT_MODULE_RESERVED3_OFFSET      0x7800
#define BOOT_MODULE_RESERVED3_SIZE        0x800

enum RUN_MODULE_INDEX {
    RUN_MODULE_TEE,       // 2KB
    RUN_MODULE_ATF,       // 2KB
    RUN_MODULE_LPM,       // 4KB
    RUN_MODULE_OS,        // 4KB
    RUN_MODULE_RESERVED1, // 1KB
    RUN_MODULE_RESERVED2, // 1KB
    RUN_MODULE_RESERVED3, // 1KB
    RUN_MODULE_MAX
};

#define RUN_MODULE_TEE_OFFSET             0x0
#define RUN_MODULE_TEE_SIZE               0x800
#define RUN_MODULE_ATF_OFFSET             0x800
#define RUN_MODULE_ATF_SIZE               0x800
#define RUN_MODULE_LPM_OFFSET             0x1000
#define RUN_MODULE_LPM_SIZE               0x1000
#define RUN_MODULE_OS_OFFSET              0x2000
#define RUN_MODULE_RESERVED1_OFFSET       0x3000
#define RUN_MODULE_RESERVED1_SIZE         0x400
#define RUN_MODULE_RESERVED2_OFFSET       0x3400
#define RUN_MODULE_RESERVED2_SIZE         0x400
#define RUN_MODULE_RESERVED3_OFFSET       0x3800
#define RUN_MODULE_RESERVED3_SIZE         0x800

struct module_pos_info {
    u32 offset;  // offset from current area base
    u32 size;
};

struct log_area_config {
    u32 used_module_num;           // block used num
    u32 reserved;
    struct module_pos_info module_que[MODULE_MAX_NUM];  // BOOT_MODULE_INDEX RUN_MODULE_INDEX
};

// AREA IN HIS queue or ERR queue
enum AREA_FLAG {
    AREA_IN_NONE_Q      = 0,
    AREA_IN_L2BUF_HIS_Q = 1,
    AREA_IN_L2BUF_ERR_Q = 2,
    AREA_IN_DDR_HIS_Q   = 3,
    AREA_IN_DDR_ERR_Q   = 4
};

#define DFX_AREA_NOT_TAG     0
#define DFX_AREA_TAG_INIT    1
#define DFX_AREA_TAG_NO_ERR  2
#define DFX_AREA_TAG_ERR     3

struct log_area_ctrl {
    u32 flag;              // AREA_FLAG,0:reserved queue;1:L2BUF HIS;2:L2BUF ERR;3:DDR HIS;4:DDR ERR queue
    u32 tag;               // tag: N:0/Y:1/H:2/E:3
    u32 e_type;            // exception type, boot:STARTUP_EXCEPTION  run:last reset reason
    u32 module_id;         // boot:module id;            run:NA
    u32 err_code;          // boot:exception id;         run:NA
    u32 reset_num;         // reset number
};

/*
 * region:
 * area space: region space / total_area_num
 */
struct log_region_config {
    u32 total_area_num;            // total number
    u32 his_area_num;              // his record number
    u32 err_area_num;              // err record number
    struct log_area_config areas;  // area config
};

struct log_region_ctrl {
    u32 area_index;       // 当前要记录的区域序号，由BIOS计算，其他组件直接读出使用；
    u32 err_area_cnt;     // 当前保存的err record的个数
    struct log_area_ctrl area_ctrl[AREA_MAX_NUM];
};

struct log_region_info {
    u32 offset;                             // 本region相对于BBOX记录基地址的偏移
    u32 size;                               // 本region的大小
    struct log_region_config region_cfg;    // 配置，由BIOS设置
    struct log_region_ctrl region_ctrl;     // 控制信息
};

/* head info */
#define HDR_LOG_MAGIC   0xEAEA2020U
#define HDR_LOG_VERSION 0x0100      // 0x0100表示V1.0

struct hdr_log_head {               // hdr: history data record
    u32 magic;                      // HDR_LOG_MAGIC
    u32 version;                    // HDR_LOG_VERSION
    u32 reset_cnt;                  // 热复位计数
    struct log_region_info boot;    // 启动态历史信息
    struct log_region_info run;     // 运行态历史信息
};

static inline u32 hdr_log_get_module_offset(const struct log_region_info *region, s32 m_index)
{
    return region->region_cfg.areas.module_que[m_index].offset;
}

static inline u32 hdr_log_get_area_index(const struct log_region_info *region)
{
    return region->region_ctrl.area_index;
}

static inline u32 hdr_log_get_area_num(const struct log_region_info *region)
{
    return ((region->region_cfg.total_area_num == 0) ? AREA_MAX_NUM :
        ((region->region_cfg.total_area_num > AREA_MAX_NUM) ? AREA_MAX_NUM : region->region_cfg.total_area_num));
}

static inline u32 hdr_log_get_area_offset(const struct log_region_info *region, u32 area_index)
{
    return area_index * (region->size / hdr_log_get_area_num(region));
}

static inline struct module_head *hdr_log_get_run_module(const char *base,
    const struct log_region_info *region, u32 area_index, s32 m_index)
{
    if ((base == NULL) || (region == NULL) || (area_index >= AREA_MAX_NUM) || (m_index >= (s32)MODULE_MAX_NUM)) {
        return NULL;
    }
    return (struct module_head *)(&base[region->offset + hdr_log_get_area_offset(region, area_index) +
        hdr_log_get_module_offset(region, m_index)]);
}

static inline struct module_head *hdr_log_get_curr_run_module(const char *base,
    const struct log_region_info *region, s32 m_index)
{
    u32 area_index = hdr_log_get_area_index(region);
    return hdr_log_get_run_module(base, region, area_index, m_index);
}

#endif

