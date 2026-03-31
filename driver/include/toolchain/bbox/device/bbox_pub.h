/*
 * @file bbox_pub.h
 *
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
 */

#ifndef BBOX_PUB_H
#define BBOX_PUB_H

#include "bbox_types.h"

/**********************************************************
 *            basic type definitions                      *
 **********************************************************/
// 模块id列表
enum BBOX_COREID_LIST {
    BBOX_UNDEF                   = 0x0,
    BBOX_DRIVER                  = 0x1,
    BBOX_OS                      = 0x2,
    BBOX_TS                      = 0x3,
    BBOX_AICPU                   = 0x5,
    BBOX_DVPP                    = 0xa,
    BBOX_AIPP                    = 0xb,
    BBOX_LPM                     = 0xc,
    BBOX_MICROWATT               = 0xd,
    BBOX_TOOLCHAIN               = 0xf,
    BBOX_BSBC                    = 0x13,
    BBOX_BIOS                    = 0x14,
    BBOX_TEEOS                   = 0x15,
    BBOX_LPFW                    = 0x17,
    BBOX_NETWORK                 = 0x18,
    BBOX_TF                      = 0x1A,
    BBOX_HSM                     = 0x1B,
    BBOX_ISP                     = 0x1C,
    BBOX_SAFETYISLAND            = 0x1D,
    BBOX_DSS                     = 0x1E,
    BBOX_COMISOLATOR             = 0x1F,
    BBOX_AOS_SD                  = 0x20,
    BBOX_AOS_DP                  = 0x21,
    BBOX_AOS_LINUX               = 0x22,
    BBOX_AOS_CORE                = 0x23,
    BBOX_PEK                     = 0x24,
    BBOX_IMU                     = 0x25,
    BBOX_COMMON                  = 0x30,
    BBOX_CORE_MAX                = 0x31
};

#define BBOX_COREID_MAX (u32)BBOX_CORE_MAX

// 异常类型
enum BBOX_REBOOT_REASON {
    BBOX_REBOOT_REASON_LABEL0 = 0x0,            // label0:重启相关
    DEVICE_COLDBOOT = BBOX_REBOOT_REASON_LABEL0,            // 冷启动，如关机后第一次开机；掉电后第一次开机
    BIOS_EXCEPTION = 0x1,                                   // bios异常重启，前一次启动bios异常
    DEVICE_HOTBOOT = 0x2,                                   // 热复位，如按键复位，芯片硬复位等
    BBOX_REBOOT_REASON_LABEL1 = 0x10,           // label1:硬件原因复位
    ABNORMAL_EXCEPTION = BBOX_REBOOT_REASON_LABEL1,         // 未检测到的异常
    TSENSOR_EXCEPTION = 0x1F,                               // soc温保复位
    PMU_EXCEPTION = 0x20,                                   // 过流、欠压、PMU过温引起的硬件复位
    DDR_FATAL_EXCEPTION = 0X22,                             // ddr fatal异常复位，如:ddr颗粒超温复位
    CHIP_EXCEPTION = 0X23,                                  // 芯片异常复位
    BBOX_REBOOT_REASON_LABEL2 = 0x24,           // label2:os软件原因复位
    OS_PANIC = BBOX_REBOOT_REASON_LABEL2,                   // os panic，如访问非法地址
    OS_OOM = 0x2A,                                          // OOM 异常
    OS_COMM = 0x2B,                                         // 通信异常
    BBOX_REBOOT_REASON_LABEL3 = 0x2C,           // label3:模块复位
    STARTUP_EXCEPTION = 0x2C,                               // 模块启动异常
    HEARTBEAT_EXCEPTION = 0x2D,                             // 模块心跳异常
    RUN_EXCEPTION = 0x2E,                                   // 模块运行异常
    HDR_EXCEPTION = 0x2F,                                   // 模块快照异常
    LPM_EXCEPTION = 0x32,                                   // LPM子系统检测到的各种异常
    TS_EXCEPTION = 0x33,                                    // TS子系统检测到的各种异常
    MICROWATT_EXCEPTION = 0x34,                             // MICROWATT异常
    DVPP_EXCEPTION = 0x35,                                  // DVPP异常
    DRIVER_EXCEPTION = 0x36,                                // DRIVER异常
    TEE_EXCEPTION = 0x38,                                   // TEEOS异常
    LPFW_EXCEPTION = 0x39,                                  // LPFW异常
    NETWORK_EXCEPTION = 0x3A,                               // NETWORK异常
    HSM_EXCEPTION = 0x3B,                                   // HSM异常
    ATF_EXCEPTION = 0x3C,                                   // ATF异常
    ISP_EXCEPTION = 0x3D,                                   // ISP异常
    SAFETYISLAND_EXCEPTION = 0x3E,                          // SAFETYISLAND异常
    TOOLCHAIN_EXCEPTION = 0x3F,                             // TOOLCHAIN异常
    DSS_EXCEPTION = 0x40,                                   // DSS异常
    COMISOLATOR_EXCEPTION = 0x41,                           // COMISOLATOR异常
    SD_EXCEPTION = 0x42,                                    // SD异常
    DP_EXCEPTION = 0x43,                                    // DP异常
    BBOX_REBOOT_REASON_LABEL4 = 0x60,           // label4 reserve:
    SUSPEND_FAIL = BBOX_REBOOT_REASON_LABEL4,           /* 休眠失败LP发起的复位 */
    RESUME_FAIL  = 0x61,                                /* 唤醒失败LP发起的复位 */
    BBOX_REBOOT_REASON_LABEL8 = 0x89,           // label5: host侧异常
    DEVICE_LTO_EXCEPTION = 0x8A,                            // 设备启动超时: load timeout
    DEVICE_HBL_EXCEPTION = 0x8B,                            // 设备心跳丢失: heart beat lost
    BOOT_DOT_INFO = 0xFE,                       // 启动打点信息，用于MDC场景，启动后无条件导出打点日志.
    BBOX_EXCEPTION_REASON_INVALID = 0xFF
};

enum BBOX_PROCESS_PRI {
    BBOX_OTHER      = 0x0,          // 不确定
    BBOX_NOTICE     = 0x1,          // 提示
    BBOX_MINOR      = 0x2,          // 次要
    BBOX_MAJOR      = 0x3,          // 重要
    BBOX_CRITICAL   = 0x4,          // 紧急
    BBOX_PPRI_MAX
};

enum BBOX_REBOOT_PRI {
    BBOX_REBOOT_NOW  = 0x01,        // 立即重启
    BBOX_REBOOT_WAIT,               // 等待重启
    BBOX_REBOOT_NO,                 // 不重启
    BBOX_REBOOT_MAX
};

enum BBOX_REENTRANT {
    BBOX_REENTRANT_ALLOW = 0x01,    // 并发
    BBOX_REENTRANT_DISALLOW         // 并发去重
};

#define BBOX_COREID_MASK(coreid) \
    (u64)((((u8)(coreid) > (u8)BBOX_UNDEF) && ((u8)(coreid) < (u8)BBOX_CORE_MAX)) ? \
    (1ULL << (u8)((u8)(coreid) - 1U)) : 0ULL)

static inline s32 bbox_check_coreid_valid(u32 coreid)
{
    if ((coreid == (u32)BBOX_UNDEF) || (coreid >= (u32)BBOX_CORE_MAX)) {
        return BBOX_FAILURE;
    }
    return BBOX_SUCCESS;
}

static inline u64 bbox_coreid_mask(u32 coreid)
{
    if (bbox_check_coreid_valid(coreid) == BBOX_SUCCESS) {
        return (1ULL << (coreid - 1U));
    } else {
        return 0ULL;
    }
}

/**********************************************************
 *       module exception register definitions            *
 **********************************************************/

struct bbox_module_exception_ctrl {
    excep_time e_clock;                 // 模块触发异常时间
    u32 e_excepid;                      // 模块触发的异常id
    u32 e_block_offset;                 // 模块异常信息划分块起始偏移值，基于模块预留内存首地址，从magic开始
    u32 e_block_len;                    // 模块异常信息划分块长度
    u32 e_info_len;                     // 模块异常信息实际长度
};

#define BBOX_MODULE_MAGIC                0x56312e32
#define BBOX_MODULE_CTRL_PAD             3
#define BBOX_MODULE_CTRL_NUM             6
#define BBOX_MODULE_CTRL_RESERV          312

struct bbox_module_ctrl {
    u32 magic;                                                      // 使用宏BBOX_MAGIC
    u8 e_block_num;                                                 // 需要使用的控制块个数，最多BBOX_PROXY_CTRL_NUM
    u8 padding[BBOX_MODULE_CTRL_PAD];                               // padding
    struct bbox_module_exception_ctrl block[BBOX_MODULE_CTRL_NUM];  // 模块dump信息控制状态
    u8 reserved[BBOX_MODULE_CTRL_RESERV];                           // 预留空间，用于后续扩展
};

#define BBOX_MODULE_CTRL_BLOCK_SIZE sizeof(struct bbox_module_ctrl) // total 512 byte


/**********************************************************
 *              bbox interfaces definitions               *
 **********************************************************/
#ifndef BBOX_COMMON_STRUCT
#define BBOX_COMMON_STRUCT
typedef void (*bbox_e_callback)(u32, void*);

typedef struct bbox_exception_info {
    u32 e_excepid;                          // exception id
    u32 e_excepid_end;                      // can register exception id region. [excepid~excepid_end]
    u64 e_notify_core_mask;                 // need notify other core mask
    u64 e_reset_core_mask;                  // need reset other core mask
    u8 e_exce_type;                         // the type of exception
    u8 e_from_core;                         // the core of happen exception
    u8 e_process_priority;                  // exception process priority
    u8 e_reboot_priority;                   // exception reboot priority, just recommended host operation
    u8 e_reentrant;                         // whether to allow exception reentrant
    u8 e_from_module[BBOX_MODULE_NAME_LEN]; // the module of happen excption
    u8 e_desc[BBOX_EXCEPTIONDESC_MAXLEN];   // the desc of happen excption
    bbox_e_callback e_callback;             // will be called when excption has processed.
} bbox_exception_info_s;
#endif

struct bbox_report_info {
    u32 devid;          // device chip id, may NOT same with device slot id
    u32 excepid;        // exception id
    excep_time time;    // exception time
    u32 arg;            // arg
};

struct bbox_dump_done_ops_info {
    u32 devid;          // device chip id, may NOT same with device slot id
    u32 excepid;        // exception id
    u8 coreid;          // which core done
    u8 etype;           // exception type
    excep_time time;    // exception time
};

/*
 * @brief       : callback function, tell bbox dump done
 * @param [in]  : struct bbox_dump_done_ops_info *info      dump done info
 * @return      : NA
 */
typedef void (*bbox_dump_done_ops)(const struct bbox_dump_done_ops_info *info);

struct bbox_dump_ops_info {
    u32 devid;          // device chip id, may NOT same with device slot id
    u32 excepid;        // exception id
    u8 coreid;          // exception core id
    u8 etype;           // exception type
    excep_time time;    // exception time
    u32 arg;            // arg from exception report
};

/*
 * @brief       : module dump operate, the function over, need call fndone to mark dump over
 * @param [in]  : struct bbox_dump_ops_info *info           module dump info
 * @param [in]  : bbox_dump_done_ops done               dump done function pointer
 * @return      : NA
 */
typedef void (*bbox_dump_ops)(const struct bbox_dump_ops_info *info, bbox_dump_done_ops done);

struct bbox_reset_ops_info {
    u32 devid;          // device chip id, may NOT same with device slot id
    u32 excepid;        // exception id
    u8 coreid;          // exception core id
    u8 etype;           // exception type
};

/*
 * @brief       : module reset operate
 * @param [in]  : struct bbox_reset_ops_info *info           module reset info
 * @return      : NA
 */
typedef void (*bbox_reset_ops)(const struct bbox_reset_ops_info *info);

struct bbox_module_info {
    u8 coreid;                      // core id
    bbox_dump_ops    ops_dump;      // dump operate pointer
    bbox_reset_ops   ops_reset;     // reset operate pointer
};

struct bbox_module_result {
    u64   log_addr;         // reserved physical address
    u32   log_len;          // reserved physical length
};

/*
 * @brief       : register module
 * @param [in]  : struct bbox_module_info *info         module info
 * @param [in]  : struct bbox_module_result *result     register result
 * @return      : <0 failure; ==0 success
 */
s32 bbox_register_module(const struct bbox_module_info *info, struct bbox_module_result *result);

/*
 * @brief       : unregister module
 * @param [in]  : u8 core_id   core id
 * @return      : <0 failure; ==0 success
 */
s32 bbox_unregister_module(u8 coreid);

/*
 * @brief       : report exception
 * @param [in]  : struct bbox_report_ops_info *info     report info
 * @return      : =1: disallow reentrant
 *                =0: success
 *                <0: failure
 */
s32 bbox_exception_report(const struct bbox_report_info *info);

/*
 * @brief       : set lastword report flag
 * @param [in]  : u32 exceptionid exception id
 * @return      : <0 failure; ==0 success
 */
s32 bbox_set_lastword_report_flag(u32 exceptionid);

/*
 * @brief       : get Last Boot State info
 * @param [in]  : u32 devId :device id
 * @param [in]  : u32 key :the stage key. 0 for Secure Boot, 1 for rootfs cms
 * @param [out] : the boot state. 0 for normal, others for abnormal
 * @return      : <0 failure; ==0 success
 */
s32 bbox_getLastBootState(unsigned int devId, unsigned int key, unsigned int *state);

#ifndef BBOX_COMMON_INTERFACE
#define BBOX_COMMON_INTERFACE
/*
 * @brief       : register exception
 * @param [in]  : struct bbox_exception_info *e    exception info
 * @return      : e_excepid
 *                == 0 fail; >0 success
 */
u32 bbox_register_exception(const struct bbox_exception_info *e);

/*
 * func name: bbox_unregister_exception
 * func args: u32 excepid,     exception id;
 * return   : < 0 fail
 *            >=0 success
 */
s32 bbox_unregister_exception(u32 excepid);

/*
 * @brief       : get device error code
 * @param [in]  : dev_id                        device chip id, may NOT same with device slot id
 * @param [out] : u32 *e_code                   exception code array;
 * @param [in]  : u32 e_capacity                array num, max value is 128
 * @return      : >0: error of num;
 *                =0: none of error;
 *                <0: failure;
 */
s32 bbox_get_device_errorcode(u32 dev_id, u32 *e_code, u32 e_capacity);

/*
 * @brief       : get the exception description
 * @param [in]  : u32 ecode             exception id
 * @param [out] : u8 *desc              string array, exception description
 * @param [in]  : u32 length            string array length
 * @return      : <0 failure; ==0 success
 */
s32 bbox_get_device_ecode_info(u32 ecode, u8 *desc, u32 length);
#endif

#endif

