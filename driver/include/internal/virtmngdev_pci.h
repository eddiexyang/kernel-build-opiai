/*
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
 *
 * Description:
 * Author: huawei
 * Create: 2019-10-15
 */

#ifndef __VIRTMNGDEV_PCI_H__
#define __VIRTMNGDEV_PCI_H__

#include "event_sched_inner.h"
#include "virtmngdev_res_common.h"

#ifdef CFG_SOC_PLATFORM_CLOUD
#define VMNGD_SURPORT_MAX_DEV 4
#else
#ifdef CFG_SOC_PLATFORM_MDC_V51
#define VMNGD_SURPORT_MAX_DEV 2
#else
#define VMNGD_SURPORT_MAX_DEV 1
#endif
#endif

#define VMNGD_WAIT_TIMEOUT   (3 * (HZ))  // 3 second
#define VMNGD_VF_DEVID_START 32
#define VMNGD_MAX_VFID_NUM 16

struct chip_to_vf_max_num {
    u32 chip_type;
    u32 vf_max_num;
};

struct vmng_mdev_iova {
    dma_addr_t iova_base;
    dma_addr_t iova_end;
};

struct vmngd_dev {
    u32 valid;
    u32 dev_id;
    u32 peer_dev_id;
    struct device *dev;
    struct mutex mutex;
    u32 vf_num;
    u32 split_mode;
    struct vmng_bandwith_ctrl bw_ctrl;
    struct vmng_mdev_iova mdev_iova[VMNG_VDEV_MAX_PER_PDEV];
};

typedef enum {
    VMNGD_EVENT_CREATE_VF = 100,
    VMNGD_EVENT_DESTROY_VF,
    VMNGD_EVENT_MAX
}VMNGD_SUBEVENT_ID;

struct vmngd_event_msg {
    uint32_t dev_id;
    uint32_t vfid;
    uint32_t core_num;
    uint32_t total_core_num;
};

struct vmngd_ctrl {
    struct vmngd_dev devices[VMNGD_SURPORT_MAX_DEV];
    struct vmngd_resource_ops res_ops;
    u32 procfs_valid;
};

struct vmngd_clear_timer {
    struct hrtimer timer;
    ktime_t kt;
    u32 vaild_dev;
};

#define BANDWIDTH_CLEAR_INTERVAL 1 /* 1 second */

typedef int (*submit_event)(unsigned int chip_id, struct sched_published_event *event);
struct vmng_vdev_ctrl *vmngd_get_ctrl(u32 dev_id, u32 vfid);
struct mutex *vmngd_get_ctrl_mutex(u32 dev_id, u32 vfid);
int vmngd_dev_id_check(u32 dev_id, u32 fid);

#endif
