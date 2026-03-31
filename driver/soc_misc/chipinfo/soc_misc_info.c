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

#include "soc_misc_info.h"
#include "soc_misc_init.h"
#include "soc_misc_adapt.h"
#include "drv_type.h"
#include "devdrv_common.h"
#include "cpu_info.h"
#include "board_id.h"
#include "slot_id.h"
#include "dms_template.h"
#include "urd_feature.h"
#include "dms_cmd_def.h"
#include "urd_acc_ctrl.h"
#include "dms_notifier.h"
#include "dms_define.h"
#include "devdrv_manager_comm.h"

#ifdef CFG_FEATURE_SRIOV
#include "devdrv_interface.h"
#endif

struct soc_misc_info_cb *g_soc_info_cb = NULL;

struct soc_misc_info_st *soc_misc_get_soc_info(unsigned int dev_id)
{
    if (dev_id >= DEVICE_NUM_MAX) {
        soc_misc_drv_err("Invalid dev_id. (dev_id=%u)", dev_id);
        return NULL;
    }

    if (g_soc_info_cb == NULL) {
        soc_misc_drv_err("Soc info is not ready. (dev_id=%u)", dev_id);
        return NULL;
    }

    return &(g_soc_info_cb->soc_misc_info[dev_id]);
}

STATIC int soc_misc_board_info_init(unsigned int dev_id)
{
    struct soc_misc_info_st *soc_info = NULL;
    int ret;

    soc_info = soc_misc_get_soc_info(dev_id);
    if (soc_info == NULL) {
        soc_misc_drv_err("Soc_info is NULL. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    ret = soc_misc_init_board_id(soc_info);
    if (ret != 0) {
        soc_misc_drv_err("Init board id from dts failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    ret = soc_misc_init_slot_id(soc_info);
    if (ret != 0) {
        soc_misc_drv_err("Init slot id from cmdline failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    return 0;
}

STATIC int soc_misc_cpu_info_init(unsigned int dev_id)
{
    struct soc_misc_info_st *soc_info = NULL;
    int ret;

    soc_info = soc_misc_get_soc_info(dev_id);
    if (soc_info == NULL) {
        soc_misc_drv_err("Soc_info is NULL. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    ret = soc_misc_init_cpu_info(soc_info);
    if (ret != 0) {
        soc_misc_drv_err("Init cpu info failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    return 0;
}

#ifdef CFG_FEATURE_SRIOV
STATIC int soc_misc_vdev_cpu_info_init(unsigned int dev_id)
{
    struct devdrv_info *dev_info = NULL;
    struct dms_dev_ctrl_block *dev_cb = NULL;
    struct soc_misc_info_st *soc_info = NULL;

    soc_info = soc_misc_get_soc_info(dev_id);
    if (soc_info == NULL) {
        soc_misc_drv_err("Soc_info is NULL. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    dev_cb = dms_get_dev_cb(dev_id);
    if (dev_cb == NULL) {
        soc_misc_drv_err("Get dev_ctrl block failed. (dev_id=%u)\n", dev_id);
        return -ENODEV ;
    }

    dev_info = (struct devdrv_info *)dev_cb->dev_info;
    if (dev_info == NULL) {
        soc_misc_drv_err("Invalid dev_info.\n");
        return -ENODEV ;
    }

    soc_info->cpu_info.tscpu_num = 0;
    soc_info->cpu_info.tscpu_os_sched = 0;
    soc_info->cpu_info.dcpu_num = 0;
    soc_info->cpu_info.dcpu_os_sched = 1;
    soc_info->cpu_info.aicpu_num = dev_info->ai_cpu_core_num;
    soc_info->cpu_info.aicpu_os_sched = 1;
    soc_info->cpu_info.ccpu_num = dev_info->ctrl_cpu_core_num;
    soc_info->cpu_info.ccpu_os_sched = 1;
    soc_misc_drv_info("Soc misc cpu info. (aicpu_num=%u; ctrl_cpu_num=%u)\n",
        soc_info->cpu_info.aicpu_num, soc_info->cpu_info.ccpu_num);

    return 0;
}

STATIC int soc_misc_vdev_board_info_init(unsigned int dev_id)
{
    int ret;
    struct soc_misc_info_st *soc_info = NULL;
    struct soc_misc_info_st *soc_info_pf = NULL;
    unsigned int pf_id = 0;
    unsigned int vf_id = 0;

    ret = devdrv_get_pfvf_id_by_devid(dev_id, &pf_id, &vf_id);
    if (ret != 0) {
        soc_misc_drv_err("Get pf and vf id failed. (dev_id=%u)\n", dev_id);
        return ret;
    }

    soc_info = soc_misc_get_soc_info(dev_id);
    if (soc_info == NULL) {
        soc_misc_drv_err("Soc_info is NULL. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    soc_info_pf = soc_misc_get_soc_info(pf_id);
    if (soc_info_pf == NULL) {
        soc_misc_drv_err("Soc_info is NULL. (dev_id=%u; pf_id=%u)\n", dev_id, pf_id);
        return -EINVAL;
    }

    soc_info->board_info.board_id = soc_info_pf->board_info.board_id;
    soc_info->board_info.slot_id = soc_info_pf->board_info.slot_id;
    return 0;
}
#endif

int soc_misc_dev_info_init(void *data)
{
#ifdef CFG_FEATURE_SRIOV
    int ret;
    unsigned int dev_id;
    struct devdrv_info *dev = NULL;

    dev = (struct devdrv_info *)data;
    if ((u32)(dev->dev_id) >= (u32)DEVICE_NUM_MAX) {
        soc_misc_drv_err("dev id invalid. (dev_id=%u).\n", dev->dev_id);
        return -EINVAL;
    }

    dev_id = dev->dev_id;
    if (devdrv_manager_is_pf_device(dev_id)) {
        return 0;
    }

    g_soc_info_cb->soc_misc_info[dev_id].dev_id = dev_id;
    ret = soc_misc_vdev_cpu_info_init(dev_id);
    if (ret != 0) {
        soc_misc_drv_err("Soc cpu info init failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    ret = soc_misc_vdev_board_info_init(dev_id);
    if (ret != 0) {
        soc_misc_drv_err("Soc board info init failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }
#endif

    return 0;
}

int soc_misc_soc_info_init(void)
{
    unsigned int dev_index;
    int ret;

    g_soc_info_cb = kzalloc(sizeof(struct soc_misc_info_cb), GFP_KERNEL | __GFP_ACCOUNT);
    if (g_soc_info_cb == NULL) {
        soc_misc_drv_err("Kzalloc for soc info cb failed.\n");
        return -ENOMEM;
    }

    ret = devdrv_get_devnum(&g_soc_info_cb->dev_num);
    if (ret != 0) {
        soc_misc_drv_err("Get dev_num failed. (ret=%d)\n", ret);
        goto INIT_FAILED;
    }

    for (dev_index = 0; dev_index < g_soc_info_cb->dev_num; dev_index++) {
        g_soc_info_cb->soc_misc_info[dev_index].dev_id = dev_index;
        ret = soc_misc_board_info_init(dev_index);
        if (ret != 0) {
            soc_misc_drv_err("Soc board info init failed. (dev_id=%u; ret=%d)\n", dev_index, ret);
            goto INIT_FAILED;
        }

        ret = soc_misc_cpu_info_init(dev_index);
        if (ret != 0) {
            soc_misc_drv_err("Soc cpu info init failed. (dev_id=%u; ret=%d)\n", dev_index, ret);
            goto INIT_FAILED;
        }
    }

    return 0;

INIT_FAILED:
    kfree(g_soc_info_cb);
    g_soc_info_cb = NULL;
    return ret;
}

void soc_misc_soc_info_uninit(void)
{
    kfree(g_soc_info_cb);
    g_soc_info_cb = NULL;
}
