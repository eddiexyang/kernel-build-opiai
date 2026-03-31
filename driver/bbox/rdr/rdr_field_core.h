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

#ifndef RDR_FIELD_CORE_H
#define RDR_FIELD_CORE_H

#include <linux/types.h>
#include "bbox_platform.h"
#include "common/bbox_common.h"
#include "device/bbox_pub.h"
#include "config/bbox_config.h"

#define RDR_VERSION           ((u32)(1UL << 16) | (u32)(2UL << 0))  // V1.0.2: 0x10002
#define RDR_BASEINFO_SIZE     0x2000U
#define FILE_MAGIC            0xdead8d8dU

#define RDR_PROC_EXEC_START 0xff115501U
#define RDR_PROC_EXEC_DONE  0xff123059U
#define RDR_REBOOT_DONE     0xff1230ffU

enum RDR_RECORD_TYPE_LIST {
    RDR_RECORD_NORMAL = 0x0,
    RDR_RECORD_DEFINE_EXCEPTION = 0x1,
    RDR_RECORD_UNDEFINE_EXCEPTION,
    RDR_RECORD_RESET_EXCEPTION
};

struct rdr_base_info_s {
    u32 excepid;
    u32 devid;
    u32 arg;
    u8 e_type;
    u8 e_core;
    u8 reserve[6];              // reserve 6 bytes
    char date[DATATIME_MAXLEN];
    struct bbox_time tm;
    u8 e_module[BBOX_MODULE_NAME_LEN];
    u8 e_desc[BBOX_EXCEPTIONDESC_MAXLEN];
    u32 start_flag;
    u32 reboot_flag;
    u8 comm_flag[DEVICE_MAX_NUM];
};

#define RDR_PRODUCT_RELATION_LEN 16
struct rdr_top_head_s {
    u32 magic;
    u32 version;
    u32 area_number;
    u32 dump_exception;
    u8 product_name[RDR_PRODUCT_RELATION_LEN];
};

struct rdr_area_s {
    u64 offset;     // area addr, unit is bytes(1 bytes)
    u32 length;     // area len, unit is bytes
    u8 coreid;      // module id
    u8 reserve[3];  // reserve 3 bytes
};

#define RDR_LOG_BUFFER_NUM 50U

struct log_record_data {
    u32 devid;
    u32 excepid;
    u8  e_exce_type;
    u8  e_from_core;
    u16 reserve;
    u32 arg;
    /* UTC time difference from 1970 to the time when an exception is recorded */
    struct bbox_time tm;
    char date[DATATIME_MAXLEN]; /* UTC time string */
};

struct rdr_log_record {
    enum RDR_RECORD_TYPE_LIST record_type;
    struct log_record_data record;
};

struct rdr_log_info_s {
    u16 event_flag;
    u8 log_num;
    u8 next_valid_index;
    struct rdr_log_record log_buffer[RDR_LOG_BUFFER_NUM];
};

struct rdr_struct_s {
    struct rdr_top_head_s top_head;
    struct rdr_base_info_s base_info;
    struct rdr_log_info_s log_info;
    struct rdr_area_s area_info[BBOX_AREA_MAXIMUM];
    struct rdr_base_info_s core_base_info;
    struct rdr_log_info_s core_log_info;
};

struct rdr_area_mem_info {
    u32 num;
    u8 area_mem_id[BBOX_AREA_MAXIMUM];
    u32 area_mem_size[BBOX_AREA_MAXIMUM];
    u64 area_mem_addr[BBOX_AREA_MAXIMUM];
    u64 end_addr;
};


struct rdr_struct_s *rdr_field_get_pbb(void);
struct rdr_struct_s *rdr_field_get_tmppbb(void);
void rdr_field_clear_tmppbb(void);
u64 rdr_field_get_pbb_size(void);

void rdr_field_save_args(u32 excepid, const struct bbox_time *tm, u32 devid, u32 arg);
void rdr_field_save_edata(const struct bbox_exception_info *e);
s32 rdr_field_get_areainfo(u8 coreid, struct bbox_module_result *retinfo);
void rdr_field_procexec_done(void);
void rdr_field_baseinfo_reinit(void);

s32 rdr_field_init(void);
void rdr_field_exit(void);

#endif
