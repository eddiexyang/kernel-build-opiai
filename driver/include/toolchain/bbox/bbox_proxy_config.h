/**
 * @file bbox_proxy_config.h
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2022. All rights reserved.
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

#ifndef BBOX_PROXY_CONFIG_H
#define BBOX_PROXY_CONFIG_H

#include "device/bbox_types.h"

struct bbox_proxy_exception_info {
    u32 e_excepid;                          // 异常id
    u8 e_process_priority;                  // 异常处理级别
    u8 e_reboot_priority;                   // 异常重启级别
    u8 e_excep_type;                        // 异常类型
    u8 e_reentrant;                         // 异常是否可重入
    u64 e_notify_core_mask;                 // 异常联动掩码
    u64 e_reset_core_mask;                  // 异常联动掩码
    u8 e_desc[BBOX_EXCEPTIONDESC_MAXLEN];   // 异常描述
};

#define BBOX_PROXY_CAPACITY_REGISTER (1ULL << 0)
#define BBOX_PROXY_CAPACITY_DUMP_DDR (1ULL << 1)
#define BBOX_PROXY_CAPACITY_DUMP_LOG (1ULL << 2)
#define BBOX_PROXY_CAPACITY_TRANS_ID (1ULL << 3)

struct bbox_proxy_module_info {
    u8 coreid;          // 模块id
    u64 flag;           // dump能力标记位，BBOX_PROXY_CAPACITY_*
    const char *name;   // 模块名
    u64 log_addr;       // 模块dump起始地址
    u32 log_len;         // 模块dump长度
    u32 wait_timeout;    // dump等待超时时间，单位ms
    u32 e_count;         // 异常信息注册数量
};

#define BLOCK_TYPE_NORMAL  (1U << 0)  // 普通数据
#define BLOCK_TYPE_STARTUP (1U << 1)  // 启动异常数据

#define CHECK_NONE              0
#define CHECK_STARTUP_EXCEPID   (1U << 0)
#define CHECK_STARTUP_TMSTMP    (1U << 1)
#define CHECK_RUNTIME_EXCEPID   (1U << 2)
#define CHECK_RUNTIME_TMSTMP    (1U << 3)
#define CHECK_HEARTBEAT_EXCEPID (1U << 4)
#define CHECK_HEARTBEAT_TMSTMP  (1U << 5)

struct bbox_proxy_block_info {
    u32 ctrl_type : 16;
    u32 ctrl_flag : 16;
    u32 info_offset;
    u32 info_block_len;
};

#define BBOX_PROXY_CTRL_NUM             6U
#define BBOX_PROXY_CTRL_PAD             3U

struct bbox_proxy_ctrl_info {
    u8 e_block_num;                     // 需要使用的控制块个数，最多BBOX_PROXY_CTRL_NUM
    u8 padding[BBOX_PROXY_CTRL_PAD];    // padding
    struct bbox_proxy_block_info block_info[BBOX_PROXY_CTRL_NUM];   // 控制块配置
};

#define BBOX_PROXY_EXCEPTION_NUM    256U

struct bbox_proxy_info {
    struct bbox_proxy_module_info module;
    struct bbox_proxy_exception_info exception[BBOX_PROXY_EXCEPTION_NUM];
    struct bbox_proxy_ctrl_info ctrl;
};

#define BBOX_PROXY_INITIALIZER(name) { \
    .module = BBOX_PROXY_MODULE_##name, \
    .exception = BBOX_PROXY_EXCEPTION_##name, \
    .ctrl = BBOX_PROXY_CTRL_##name, \
}

#endif // BBOX_PROXY_CONFIG_H
