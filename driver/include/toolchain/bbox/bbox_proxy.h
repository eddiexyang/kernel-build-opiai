/**
 * @file bbox_proxy.h
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

#ifndef BBOX_PROXY_H
#define BBOX_PROXY_H

#include "bbox_proxy_config.h"

typedef struct proxy_excep_time_t {
    u64 tv_sec;
    u64 tv_usec;
} proxy_excep_time;

enum BBOX_PROXY_DUMP_STATUS {
    PROXY_STATUS_INIT  = 0,
    PROXY_STATUS_DOING = 1,
    PROXY_STATUS_DONE  = 2
};

struct bbox_proxy_exception_ctrl {
    proxy_excep_time e_clock;   // 模块触发异常时间
    u32 e_main_excepid;         // 模块触发的异常id
    u32 e_sub_excepid;          // 模块触发的异常id
    u32 e_info_offset;          // 模块全部异常信息偏移值，基于模块预留内存首地址，从magic开始
    u32 e_info_len;             // 模块全部异常信息长度
    u16 e_dump_status;          // 模块将异常信息存预留内存的控制状态
    u16 e_save_status;          // 代理将异常信息从预留内存导出的控制状态
    u32 e_reserved;             // 结构对齐预留
};

// 通过共享内存交互
#define BBOX_PROXY_MAGIC                0x56312e31
#define BBOX_PROXY_CTRL_RESERV          192

struct bbox_proxy_module_ctrl {
    u32 magic;                                                      // 使用宏BBOX_PROXY_MAGIC
    struct bbox_proxy_ctrl_info config;                             // ctrl块配置
    struct bbox_proxy_exception_ctrl block[BBOX_PROXY_CTRL_NUM];    // 模块dump信息控制状态
    u8 reserved[BBOX_PROXY_CTRL_RESERV];                            // 预留空间，用于后续扩展
};

#define BBOX_PROXY_CTRL_BLOCK_SIZE sizeof(struct bbox_proxy_module_ctrl) // total 512 byte

#endif // BBOX_PROXY_H
