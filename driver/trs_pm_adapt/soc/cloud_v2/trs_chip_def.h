/*
* Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
* Create: 2022-10-08
*/
#ifndef TRS_CHIP_DEF_H
#define TRS_CHIP_DEF_H

#include "trs_pub_def.h"
#include "trs_core.h"
#include "trs_chan.h"
#include "trs_chan_mem.h"
#include "trs_chip_def_comm.h"
#include "trs_res_id_def.h"
#include "trs_rsv_mem.h"

/* Trs id config */
#define TRS_TS_DB_STRIDE            (4 * 1024)
#define TRS_STARS_SCHED_STRIDE      (4 * 1024)

#define TRS_LOGIC_CQ_ID_START       0U
#define TRS_LOGIC_CQ_ID_END         2048U

#define TRS_CB_SQ_ID_START          0U
#define TRS_CB_SQ_ID_END            1024U

#define TRS_CB_CQ_ID_START          0U
#define TRS_CB_CQ_ID_END            1024U

#define TRS_EVENT_ID_SPLIT          1024U

#define TRS_HOST_CQ_GROUP_NUM       16

/* Trs notify */
#define TRS_NOTIFY_SIZE             4
#define TRS_NOTIFY_SLICE_SIZE (64 * 1024)
#define TRS_NOTIFY_NUM_PER_SLICE 512   /* total silce num is 16 */

static inline u32 trs_get_notify_offset(u32 id)
{
    return (id % TRS_NOTIFY_NUM_PER_SLICE) * TRS_NOTIFY_SIZE + (id / TRS_NOTIFY_NUM_PER_SLICE) * TRS_NOTIFY_SLICE_SIZE;
}

static inline u32 trs_get_notify_size(void)
{
    return TRS_NOTIFY_SIZE;
}

/* Trs event */
#define TRS_EVENT_SIZE             4
#define TRS_EVENT_SLICE_SIZE (64 * 1024)
#define TRS_EVENT_NUM_PER_SLICE 4096   /* total silce num is 16 */

static inline u32 trs_get_event_offset(u32 id)
{
    return (id % TRS_EVENT_NUM_PER_SLICE) * TRS_EVENT_SIZE + (id / TRS_EVENT_NUM_PER_SLICE) * TRS_EVENT_SLICE_SIZE;
}

static inline u32 trs_get_event_size(void)
{
    return TRS_EVENT_SIZE;
}

static inline u32 trs_get_cq_group_num(void)
{
    return TRS_HOST_CQ_GROUP_NUM;
}

/* Trs ts doorbell config */
#define TRS_DB_OFFLINE_MBOX_START   1022u
#define TRS_DB_OFFLINE_MBOX_END     1023u

#define TRS_DB_MAINT_SQ_START       1008u
#define TRS_DB_MAINT_SQ_END         1012u

#define TRS_DB_MAINT_CQ_START       1012u
#define TRS_DB_MAINT_CQ_END         1022u

static inline void trs_get_mbox_db_cfg(u32 phy_devid, u32 tsid, u32 *start, u32 *end)
{
    *start = TRS_DB_OFFLINE_MBOX_START;
    *end = TRS_DB_OFFLINE_MBOX_END;
}

static inline void trs_get_maint_sq_db_cfg(u32 phy_devid, u32 tsid, u32 *start, u32 *end)
{
    *start = TRS_DB_MAINT_SQ_START;
    *end = TRS_DB_MAINT_SQ_END;
}

static inline void trs_get_maint_cq_db_cfg(u32 phy_devid, u32 tsid, u32 *start, u32 *end)
{
    *start = TRS_DB_MAINT_CQ_START;
    *end = TRS_DB_MAINT_CQ_END;
}

/* Trs chan mem type */
static inline int trs_get_hw_sq_mem_type(void)
{
    return (int)TRS_CHAN_MEM_DDR;
}

static inline int trs_get_sw_sq_mem_type(void)
{
    return (int)TRS_CHAN_MEM_DDR;
}

static inline int trs_get_maint_sq_mem_type(void)
{
    return (int)TRS_CHAN_MEM_DDR;
}

static inline int trs_get_hw_cq_mem_type(void)
{
    return (int)TRS_CHAN_MEM_DDR;
}

static inline int trs_get_sw_cq_mem_type(void)
{
    return (int)TRS_CHAN_MEM_DDR;
}

static inline int trs_get_maint_cq_mem_type(void)
{
    return (int)TRS_CHAN_MEM_DDR;
}

static inline int trs_get_hw_sqcq_rsv_mem_type(void)
{
    return (int)RSV_MEM_HW_SQCQ;
}

static inline int trs_get_maint_sqcq_rsv_mem_type(void)
{
    return (int)RSV_MEM_MAINT_SQCQ;
}

/* trs hw sched type */
static inline int trs_get_hw_type(void)
{
    return TRS_HW_TYPE_STARS;
}

static inline int trs_get_trigger_sq_db_cfg(struct trs_id_inst *inst, u32 *start, u32 *end)
{
    return -EOPNOTSUPP;
}

static inline bool trs_is_support_uio(void)
{
    return true;
}
#endif /* TRS_CHIP_DEF_H */
