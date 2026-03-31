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

#include <linux/xpc_kernel_inter.h>
#include <linux/delay.h>
#include <linux/securec.h>
#include "device/bbox_types.h"
#include "../common/bbox_print.h"
#include "../common/bbox_xpc_common.h"

s32 bbox_xpcshm_open_channel(s32 *chlid)
{
#ifndef BBOX_UT

    u32 repeat = BBOX_REPEAT_NUM;
    s32 id;
    struct chl_info ch = {0};
    char *name = "bbox_xpc_channel";
    s32 ret;

    ret = memcpy_s(ch.name, XPC_CHANNEL_NAME_MAX, (const void *)name, strlen(name) + 1);
    BB_CHECK_MEMCPY(ret, return BBOX_FAILURE);

    // create and open, can only open once
    id = xpcshm_open_channel(&ch, CHL_O_CREATE);
    if (id < 0) {
        BB_PRINT_ERR("Bbox open xpcshm channel faield. (errno=%d)\n", id);
        return BBOX_FAILURE;
    }

    // Write only when both ends are open
    while (xpcshm_get_chl_state(id) != XPC_CHL_STATE_OPEN) {
        usleep_range(BBOX_WAITTIME_MIN, BBOX_WAITTIME_MAX);

        (void)xpcshm_close_channel(id);

        id = xpcshm_open_channel(&ch, CHL_O_CREATE);
        if (id < 0) {
            BB_PRINT_ERR("Bbox open xpcshm channel faield. (errno=%d)\n", id);
            return BBOX_FAILURE;
        }

        if (repeat == 0) {
            (void)xpcshm_close_channel(id);
            BB_PRINT_INFO("Bbox open the xpc channel has reached the upper limit. (limitnum=%u)\n",
                          BBOX_REPEAT_NUM);
            return BBOX_FAILURE;
        } else {
            repeat--;
        }
    }

    *chlid = id;
    BB_PRINT_INFO("Bbox open xpcshm channel sucess. (chlid=%d)\n", id);
#endif
    return BBOX_SUCCESS;
}

void bbox_xpchsm_close_channel(s32 chlid)
{
#ifndef BBOX_UT
    if (chlid < 0) {
        BB_PRINT_ERR("Bbox close xpcshm channel faield. (chlid=%u)\n", chlid);
        return;
    }
    (void)xpcshm_close_channel(chlid);
#endif
    BB_PRINT_INFO("Bbox close xpcshm channel sucess. (chlid=%d)\n", chlid);
}

s32 bbox_xpc_poll_single(s32 chlid, struct chl_poll_ret *pollret)
{
    int ret = 0;
#ifndef BBOX_UT
    s32 chlidarray[1] = {chlid};
    int chlnum = 1;
    int resnum = 0;

    ret = xpcshm_poll(chlidarray, chlnum, pollret, &resnum, BBOX_POLL_TIMEOUT);
    if (ret == 0) {
        if (resnum == 1 && pollret[0].id == chlid) {
            return pollret[0].len;
        } else {
            BB_PRINT_ERR("Bbox xpcshm_poll err. (ret=%d, resnum=%d, res[0].id=%d)\n",
                ret, resnum, pollret[0].id);
            return BBOX_FAILURE;
        }
    }
#endif
    return ret;
}

void bbox_xpc_read(s32 chlid, u32 nexttype, func operfunc)
{
#ifndef BBOX_UT
    s32 ret;
    u32 len = BBOX_READ_SIZE;
    char array[BBOX_READ_SIZE];

    while (true) {
        if (nexttype == (PKT_TYPE_NORMAL | PKT_TYPE_INNER)) {
            memset_s(array, BBOX_READ_SIZE, 0, BBOX_READ_SIZE);
            ret = xpcshm_read(chlid, array, &len, &nexttype);
            if (ret >= 0) {
                operfunc(array, len);
            } else if (ret == XPC_ERR_CHL_EMPTY) {
                BB_PRINT_INFO("Bbox xpc read channel is empty.\n");
                break;
            } else {
                BB_PRINT_ERR("Bbox xpc read failed. (errno=%d)\n", ret);
                break;
            }
        } else {
            BB_PRINT_ERR("Bbox xpc pkt type is invalid. (type=%u)\n", nexttype);
            break;
        }
    }
#endif
}

s32 bbox_xpc_read_msg(s32 chlid, struct chl_poll_ret *pollret, func operfunc)
{
    s32 ret = bbox_xpc_poll_single(chlid, pollret);
#ifndef BBOX_UT
    if (ret > 0) {
        bbox_xpc_read(chlid, pollret[0].type, operfunc);
    }
#endif
    return ret;
}

