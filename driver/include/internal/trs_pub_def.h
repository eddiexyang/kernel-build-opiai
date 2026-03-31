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
* Create: 2022-7-15
*/

#ifndef TRS_PUB_DEF_H
#define TRS_PUB_DEF_H

#include <linux/types.h>
#include <linux/sched.h>
#include <linux/smp.h>
#include <linux/version.h>
#include <linux/kref.h>
#include "trs_base_define.h"
#ifndef EMU_ST
#include "drv_log.h"
#else
#include "ut_log.h"
#endif

#define TRS_DEV_MAX_NUM 1124
#define TRS_TS_MAX_NUM 2
#define TRS_TS_INST_MAX_NUM (TRS_DEV_MAX_NUM * TRS_TS_MAX_NUM)

#define TRS_INST_STATUS_NORMAL 0
#define TRS_INST_STATUS_ABNORMAL 1

#define module_trs "trs_drv"

#define trs_err(fmt, ...) do { \
    drv_err(module_trs, "<%s:%d:%d:%d> " fmt, \
        current->comm, current->tgid, current->pid, smp_processor_id(), ##__VA_ARGS__); \
} while (0)
#define trs_warn(fmt, ...) do { \
    drv_warn(module_trs, "<%s:%d:%d:%d> " fmt, \
        current->comm, current->tgid, current->pid, smp_processor_id(), ##__VA_ARGS__); \
} while (0)
#define trs_info(fmt, ...) do { \
    drv_info(module_trs, "<%s:%d:%d:%d> " fmt, \
        current->comm, current->tgid, current->pid, smp_processor_id(), ##__VA_ARGS__); \
} while (0)
#define trs_debug(fmt, ...) do { \
    drv_pr_debug(module_trs, "<%s:%d:%d:%d> " fmt, \
        current->comm, current->tgid, current->pid, smp_processor_id(), ##__VA_ARGS__); \
} while (0)

#ifndef __GFP_ACCOUNT
#ifdef __GFP_KMEMCG
#define __GFP_ACCOUNT __GFP_KMEMCG /* for linux version 3.10 */
#endif

#ifdef __GFP_NOACCOUNT
#define __GFP_ACCOUNT 0 /* for linux version 4.1 */
#endif
#endif

#if LINUX_VERSION_CODE <= KERNEL_VERSION(4, 4, 0)
static inline unsigned int kref_read(const struct kref *kref)
{
    return atomic_read(&kref->refcount);
}

#define ALIGN_DOWN(addr, size)  ((addr)&(~((size)-1)))
#define PCI_VENDOR_ID_HUAWEI 0x19e5
#endif

enum {
    TRS_STREAM_ID = 0,
    TRS_EVENT_ID,
    TRS_NOTIFY_ID,
    TRS_MODEL_ID,
    TRS_CMO_ID,
    TRS_HW_SQ_ID,
    TRS_HW_CQ_ID,
    TRS_SW_SQ_ID,
    TRS_SW_CQ_ID,
    TRS_RSV_HW_SQ_ID,
    TRS_RSV_HW_CQ_ID,
    TRS_TASK_SCHED_CQ_ID,
    TRS_MAINT_SQ_ID,
    TRS_MAINT_CQ_ID,
    TRS_IPC_EVENT_ID,
    TRS_LOGIC_CQ_ID,
    TRS_CB_SQ_ID,
    TRS_CB_CQ_ID,
    TRS_CDQM_ID,
    TRS_ID_TYPE_MAX
};

static const char *trs_id_type_name[TRS_ID_TYPE_MAX] = {
    [TRS_STREAM_ID] = "StreamId",
    [TRS_EVENT_ID] = "EventId",
    [TRS_NOTIFY_ID] = "NotifyId",
    [TRS_MODEL_ID] = "ModelId",
    [TRS_CMO_ID] = "CmoId",
    [TRS_HW_SQ_ID] = "HwSqId",
    [TRS_HW_CQ_ID] = "HwCqId",
    [TRS_SW_SQ_ID] = "SwSqId",
    [TRS_SW_CQ_ID] = "SwCqId",
    [TRS_RSV_HW_SQ_ID] = "RsvHwSqId",
    [TRS_RSV_HW_CQ_ID] = "RsvHwCqId",
    [TRS_TASK_SCHED_CQ_ID] = "TaskSchedCqId",
    [TRS_MAINT_SQ_ID] = "MaintSqId",
    [TRS_MAINT_CQ_ID] = "MaintCqId",
    [TRS_IPC_EVENT_ID] = "IpcEventId",
    [TRS_LOGIC_CQ_ID] = "LogicCqId",
    [TRS_CB_SQ_ID] = "CbSqId",
    [TRS_CB_CQ_ID] = "CbCqId",
    [TRS_CDQM_ID] = "CdqmId"
};

static inline bool trs_id_is_local_type(int type)
{
    return ((type == TRS_LOGIC_CQ_ID) || (type == TRS_CB_SQ_ID) || (type == TRS_CB_CQ_ID));
}

static inline bool trs_id_need_divide_type(int type)
{
    return !((type == TRS_MAINT_SQ_ID) || (type == TRS_MAINT_CQ_ID));
}

static inline bool trs_id_is_hw_divide_type(int type)
{
    return ((type == TRS_EVENT_ID) || (type == TRS_NOTIFY_ID) || (type == TRS_CMO_ID) ||
        (type == TRS_HW_SQ_ID) || (type == TRS_HW_CQ_ID) || (type == TRS_RSV_HW_SQ_ID) ||
        (type == TRS_RSV_HW_CQ_ID) || (type == TRS_CDQM_ID));
}

static inline int trs_id_type_replace(int type)
{
    if (type == TRS_RSV_HW_SQ_ID) {
        return TRS_HW_SQ_ID;
    } else if (type == TRS_RSV_HW_CQ_ID) {
        return TRS_HW_CQ_ID;
    } else {
        /* do nothing */
    }

    return type;
}

#define TRS_ID_CACHE_BATCH_NUM 64

static inline const char *trs_id_type_to_name(int type)
{
    if ((type >= TRS_STREAM_ID) && (type < TRS_ID_TYPE_MAX)) {
        return trs_id_type_name[type];
    }
    return "UnknownId";
}

static inline void trs_id_inst_pack(struct trs_id_inst *inst, u32 devid, u32 tsid)
{
    inst->devid = devid;
    inst->tsid = tsid;
}

static inline u32 trs_id_inst_to_ts_inst(struct trs_id_inst *inst)
{
    return inst->devid * TRS_TS_MAX_NUM + inst->tsid;
}

static inline int trs_id_inst_check(struct trs_id_inst *inst)
{
    if (inst == NULL) {
        trs_err("Null ptr\n");
        return -EINVAL;
    }

    if ((inst->devid >= TRS_DEV_MAX_NUM) || (inst->tsid >= TRS_TS_MAX_NUM)) {
        trs_err("Invalid para. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return -EINVAL;
    }

    return 0;
}

static inline bool trs_bitmap_bit_is_vaild(u32 bitmap, u32 bit)
{
    return (bitmap & (1ul << bit));
}

#endif /* TRS_PUB_DEF_H */

