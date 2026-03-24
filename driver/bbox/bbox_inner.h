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

#ifndef BBOX_INNER_H
#define BBOX_INNER_H

#include "device/bbox_types.h"
#include "device/bbox_pub.h"

#define THREAD_NAME_LEN         16
#define BBOX_INVALID_DEVID      0xFFFF

// macro abort exception id
#define BBOX_EXCEPID_DETAILED_BIT    0 // detailed code: 0-11
#define BBOX_EXCEPID_FUNCTION_BIT   12 // module sub function: 12-16
#define BBOX_EXCEPID_COREID_BIT     17 // module id: 17-24
#define BBOX_EXCEPID_LEVEL_BIT      25 // warning level: 25-27
#define BBOX_EXCEPID_TYPE_BIT       28 // code type: 28-29
#define BBOX_EXCEPID_POSITION_BIT   30 // report position: 30-31

// exception id mask
#define BBOX_EXCEPID_COREID_MASK    0xFFU
#define BBOX_EXCEPID_POSITION_MASK  0x3U

// exception value
#define BBOX_EXCEPID_POSITION_DEVICE    0x2UL
#define BBOX_EXCEPID_CLASS_RECOVERY     0x1U
#define BBOX_EXCEPID_CLASS_EXCEPTION    0x2U


static inline u8 bbox_excepid_get_coreid(u32 excepid)
{
    u8 coreid = (u8)((excepid >> BBOX_EXCEPID_COREID_BIT) & BBOX_EXCEPID_COREID_MASK);
    return ((coreid >= (u8)BBOX_CORE_MAX) ? (u8)BBOX_UNDEF : coreid);
}

static inline u8 bbox_excepid_get_position(u32 excepid)
{
    return (u8)((excepid >> BBOX_EXCEPID_POSITION_BIT) & BBOX_EXCEPID_POSITION_MASK);
}

static inline bool bbox_excepid_check_coreid(u32 excepid)
{
    u8 coreid = bbox_excepid_get_coreid(excepid);
    return (bbox_check_coreid_valid(coreid) == BBOX_SUCCESS) ? true : false;
}

static inline bool bbox_excepid_check_position(u32 excepid)
{
    u8 pos = bbox_excepid_get_position(excepid);
    return (pos == BBOX_EXCEPID_POSITION_DEVICE) ? true : false;
}

static inline bool bbox_excepid_check(u32 excepid)
{
    return bbox_excepid_check_position(excepid) && bbox_excepid_check_coreid(excepid);
}

static inline bool bbox_excepid_compare_coreid(u32 excepid, u8 coreid)
{
    u8 e_coreid = bbox_excepid_get_coreid(excepid);
    return (e_coreid == coreid) ? true : false;
}

static inline bool bbox_excepid_check_reset_class(u32 excepid)
{
    return ((((excepid >> BBOX_EXCEPID_TYPE_BIT) & BBOX_EXCEPID_CLASS_RECOVERY) == BBOX_EXCEPID_CLASS_RECOVERY) ?
        true : false);
}

static inline bool bbox_excepid_check_excep_class(u32 excepid)
{
    return ((((excepid >> BBOX_EXCEPID_TYPE_BIT) & BBOX_EXCEPID_CLASS_EXCEPTION) == BBOX_EXCEPID_CLASS_EXCEPTION) ?
        true : false);
}

static inline u32 bbox_excepid_transf_class(u32 excepid)
{
    return (bbox_excepid_check_excep_class(excepid) == true) ? excepid : ((excepid & 0xCFFFFFFFU) | 0x20000000U);
}

static inline u32 bbox_excepid_generate(u8 type, u8 level, u8 coreid, u8 sub, u16 code)
{
    u32 excepid = BBOX_EXCEPID_POSITION_DEVICE << BBOX_EXCEPID_POSITION_BIT;
    excepid += (u32)type << BBOX_EXCEPID_TYPE_BIT;
    excepid += (u32)level << BBOX_EXCEPID_LEVEL_BIT;
    excepid += (u32)coreid << BBOX_EXCEPID_COREID_BIT;
    excepid += (u32)sub << BBOX_EXCEPID_FUNCTION_BIT;
    excepid += code;
    return excepid;
}

#endif
