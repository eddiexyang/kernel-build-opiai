/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
 * Create: 2023-11-1
 */
#include <linux/uaccess.h>

#include "ascend_hal_define.h"
#include "dbl/feature_loader.h"

#include "event_sched_inner.h"
#include "dms_interface.h"
#include "devdrv_manager_comm.h"
#ifndef EMU_ST
#include "devdrv_common.h"
#endif

#include "trs_pub_def.h"
#include "trs_res_id_def.h"
#include "trs_shr_id.h"

/* for shrid pa addr manage */
#define SHRID_STARS_BASE_ADDR  0x06a0000000ULL
#define SHRID_ASCEND920_CHIP_ADDR_OFFSET   0x80000000000ULL
#define SHRID_ASCEND920_DIE_ADDR_OFFSET    0x10000000000ULL

#define SHRID_STARS_PCIE_BASE_ADDR 0x400004008000ULL /* PCIE BAR */
#define SHRID_PCIE_LOCAL_DEV_OFFSET    36ULL
#define SHRID_PCIE_REMOTE_DEV_OFFSET   32ULL

#define SHRID_STARS_NOTIFY_BASE_ADDR  0x100000ULL

#ifndef EMU_ST
static  int shr_id_get_notify_base_addr(u32 devid, u32 remotePhyId, u64 *baseAddr)
{
    int topology_type, ret;
    u32 localPhyId = devid;

    ret = dms_get_dev_topology(localPhyId, remotePhyId, &topology_type);
    if (ret != 0) {
        trs_err("Get topology_type failed. (devid=%u; remote_devid=%u; ret=%d)\n", devid, remotePhyId, ret);
        return ret;
    }

    if ((topology_type == TOPOLOGY_HCCS) || (topology_type == TOPOLOGY_SIO)) {
        struct devdrv_info *info;
        info = devdrv_manager_get_devdrv_info(remotePhyId);
        if (info == NULL) {
            trs_err("Get chipid failed. (remotePhyId=%u)\n", remotePhyId);
            return -EINVAL;
        }
        trs_debug("Notify info. (devid=%u; remote_devid=%u; chip=%u; die=%u; type=%d)\n",
            devid, remotePhyId, info->chip_id, info->die_id, topology_type);

        *baseAddr = SHRID_STARS_BASE_ADDR + SHRID_STARS_NOTIFY_BASE_ADDR +
            SHRID_ASCEND920_CHIP_ADDR_OFFSET * info->chip_id + SHRID_ASCEND920_DIE_ADDR_OFFSET * info->die_id;
    } else if ((topology_type == TOPOLOGY_PIX) || (topology_type == TOPOLOGY_PIB) || (topology_type == TOPOLOGY_PHB) ||
        (topology_type == TOPOLOGY_SYS)) {
        *baseAddr = SHRID_STARS_PCIE_BASE_ADDR + SHRID_STARS_NOTIFY_BASE_ADDR +
            ((u64)remotePhyId << SHRID_PCIE_REMOTE_DEV_OFFSET) + ((u64)localPhyId << SHRID_PCIE_LOCAL_DEV_OFFSET);
        trs_debug("Notify info. (devid=%u; remote_devid=%u; type=%d)\n", devid, remotePhyId, topology_type);
    } else {
        trs_warn("Not support. (devid=%u; rudevid=%u; topology_type=%u)\n", devid, remotePhyId, topology_type);
        return -EINVAL;
    }

    return 0;
}

#define SHRID_NOTIFY_SIZE   4
#define SHRID_NOTIFY_SLICE_SIZE (64 * 1024)
#define SHRID_NOTIFY_NUM_PER_SLICE  512
static inline u32 shr_id_get_notify_offset(u32 id)
{
    return (id % SHRID_NOTIFY_NUM_PER_SLICE) * SHRID_NOTIFY_SIZE +
        (id / SHRID_NOTIFY_NUM_PER_SLICE) * SHRID_NOTIFY_SLICE_SIZE;
}

static int shr_id_type_trans[SHR_ID_TYPE_MAX] = {
    [SHR_ID_NOTIFY_TYPE] = TRS_NOTIFY,
    [SHR_ID_EVENT_TYPE] = TRS_EVENT,
};

int shr_id_event_update(unsigned int devid, struct sched_published_event_info *event_info,
    struct sched_published_event_func *event_func)
{
    struct drvShrIdInfo *info = NULL;
    u64 baseAddr, notify_addr;
    struct trs_id_inst inst;
    u8 msg[40];
    int ret;

    if ((event_info->subevent_id != DRV_SUBEVENT_TRS_SHR_ID_CONFIG_MSG) &&
        (event_info->subevent_id != DRV_SUBEVENT_TRS_SHR_ID_DECONFIG_MSG)) {
        return 0;
    }

    if (event_info->msg == NULL) {
        trs_err("Msg is NULL.\n");
        return -EINVAL;
    }

    if (copy_from_user(&msg, (void *)(uintptr_t)event_info->msg, 40) != 0) { /* 40 is event msg */
        trs_err("Copy from user fail.\n");
        return -EFAULT;
    }

    info = (struct drvShrIdInfo *)(msg + sizeof(struct event_sync_msg));
    inst.devid = info->devid;
    inst.tsid = info->tsid;

    if (info->id_type >= SHR_ID_TYPE_MAX) {
        trs_err("Type is invalid. (type=%u)\n", info->id_type);
        return -EINVAL;
    }

    if (event_info->subevent_id == DRV_SUBEVENT_TRS_SHR_ID_CONFIG_MSG) { /* only shrIdOpen need check shrid. */
        if (!shr_id_is_belong_to_proc(&inst, current->tgid, shr_id_type_trans[info->id_type], info->shrid)) {
            trs_err("Id invalid. (devid=%u; rudevid=%u; tsid=%u; pid=%d; type=%u; shrid=%u)\n",
                devid, info->devid, info->tsid, current->tgid, info->id_type, info->shrid);
            return -EACCES;
        }
    }

    ret = shr_id_get_notify_base_addr(devid, info->devid, &baseAddr);
    if (ret != 0) {
        return ret;
    }

    notify_addr = baseAddr + shr_id_get_notify_offset(info->shrid);
    info->rsv[0] = (u32)(notify_addr & 0xffffffffULL); /* 0xffffffffULL is low 32 bit */
    info->rsv[1] = (u32)(notify_addr >> 32); /* 32 is high bit */

    if (copy_to_user((void *)(uintptr_t)event_info->msg, &msg, 40) != 0) { /* 40 is event msg */
        trs_err("Copy to user fail.\n");
        return -EFAULT;
    }

    trs_debug("Notify info. (devid=%u; remote_devid=%u; type=%u; shrid=%u; sub_id=%u)\n",
        devid, info->devid, info->id_type, info->shrid, event_info->subevent_id);

    return 0;
}
#else
int shr_id_event_update(unsigned int devid, struct sched_published_event_info *event_info,
    struct sched_published_event_func *event_func)
{
    return 0;
}
#endif

int shr_id_event_init(void)
{
    return sched_register_event_pre_proc_handle(EVENT_DRV_MSG, shr_id_event_update);
}
DECLAER_FEATURE_AUTO_INIT(shr_id_event_init, FEATURE_LOADER_STAGE_5);

void shr_id_event_uninit(void)
{
    sched_unregister_event_pre_proc_handle(EVENT_DRV_MSG);
}
DECLAER_FEATURE_AUTO_UNINIT(shr_id_event_uninit, FEATURE_LOADER_STAGE_5);

