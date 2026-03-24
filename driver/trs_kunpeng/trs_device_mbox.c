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
* Create: 2023-8-3
*/
#include <linux/mutex.h>
#include <asm/current.h>

#include "trs_chip_def_comm.h"
#include "trs_pub_def.h"
#include "trs_chan_update.h"
#include "trs_mailbox_def.h"
#include "trs_device_mbox.h"

typedef int (*notice_stars_func)(u32 devid, u32 tsid, void *msg, u32 msg_len);
static notice_stars_func notice_stars_handler[TRS_DEV_MAX_NUM] = {NULL};
static struct mutex notice_stars_mutex[TRS_DEV_MAX_NUM];

void trs_notice_stars_register(u32 devid, notice_stars_func handler)
{
    if (devid <= TRS_DEV_MAX_NUM) {
        notice_stars_handler[devid] = handler;
        trs_info("Register notice stars handle success. (devid=%u)\n", devid);
    }
}
EXPORT_SYMBOL(trs_notice_stars_register);

void trs_notice_stars_unregister(u32 devid)
{
    if (devid <= TRS_DEV_MAX_NUM) {
        notice_stars_handler[devid] = NULL;
    }
}
EXPORT_SYMBOL(trs_notice_stars_unregister);

int trs_mbox_send(struct trs_id_inst *inst, u32 chan_id, void *data, size_t size, int timeout)
{
    int ret;

    if ((trs_id_inst_check(inst) != 0) || (data == NULL)) {
        return -EINVAL;
    }

    ret = trs_mb_update(inst, (int)current->tgid, data, (u32)size);
    if (ret != 0) {
        trs_err("Fail to update mbox. (ret=%d; devid=%u; tgid=%d)\n", ret, inst->devid, (int)current->tgid);
        return ret;
    }

    ret = -ENXIO;
    if (notice_stars_handler[inst->devid] != NULL) {
        mutex_lock(&notice_stars_mutex[inst->devid]);
        ret = notice_stars_handler[inst->devid](inst->devid, inst->tsid, data, size);
        if (ret != 0) {
            trs_err("Stars ko handle failed. (ret=%u; devid=%u)\n", ret, inst->devid);
        }
        mutex_unlock(&notice_stars_mutex[inst->devid]);
    }
    return ret;
}
EXPORT_SYMBOL(trs_mbox_send);

int trs_mbox_config(struct trs_id_inst *inst)
{
    mutex_init(&notice_stars_mutex[inst->devid]);
    return 0;
}

void trs_mbox_deconfig(struct trs_id_inst *inst)
{
    mutex_destroy(&notice_stars_mutex[inst->devid]);
}

