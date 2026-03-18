/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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
#include <securec.h>
#include <linux/slab.h>

#include "devdrv_common.h"
#include "devdrv_id.h"
#include "devdrv_devinit.h"
#include "tsdrv_common.h"
#include "tsdrv_log.h"
#include "tsdrv_drvops.h"
#include "tsdrv_device.h"
#include "tsdrv_hwinfo.h"
#include "tsdrv_id.h"
#include "tsdrv_sync.h"
#include "devdrv_cqsq.h"
#include "tsdrv_get_ssid.h"
#include "devdrv_cb.h"
#include "hvtsdrv_id_event.h"
STATIC int (*tsdrv_send_sync_msg)(u32 devid, void *tx, size_t tx_size) = NULL;

s32 tsdrv_sync_msg_send(u32 dev_id, struct tsdrv_msg_info *msg_info)
{
#ifdef CFG_FEATURE_RC_MODE
#ifndef TSDRV_UT
    msg_info->header.result = 0;
    msg_info->header.valid = TSDRV_MSG_SYNC_RSP_MAGIC;
    return 0;
#endif
#else
    if (tsdrv_send_sync_msg == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("tsdrv_send_sync_msg is null.\n");
        return -EFAULT;
#endif
    }

    return tsdrv_send_sync_msg(dev_id, msg_info, sizeof(struct tsdrv_msg_info));
#endif
}
EXPORT_SYMBOL_UNRELEASE(tsdrv_sync_msg_send);

STATIC void tsdrv_construct_sync_data(struct tsdrv_msg_info *msg_info,
    struct tsdrv_msg_resource_id *msg_resource, u32 tsid, u32 msg_id, enum tsdrv_id_type id_type, u32 req_id_num)
{
    msg_info->header.msg_id = msg_id;
    msg_info->header.valid = TSDRV_MSG_SYNC_MAGIC;
    msg_info->header.result = TSDRV_MSG_INVALID_RESULT;
    msg_info->header.tsid = tsid;
    msg_resource->id_type = id_type;
    msg_resource->sync_type = MSG_CMD_TYPE_SYNC;
    msg_resource->req_id_num = req_id_num;
    msg_resource->ret_id_num = 0;
}

#ifdef CFG_FEATURE_CMO
s32 tsdrv_msg_alloc_sync_cmo(u32 dev_id, u32 tsid)
{
    struct tsdrv_msg_resource_id *dev_msg_resource_id = NULL;
    struct tsdrv_msg_info msg_info = {{0}};
    u32 cur_cmo_num;

    int ret = tsdrv_get_ts_id_cur_num(dev_id, tsid, TSDRV_CMO_ID, &cur_cmo_num);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("get cur_cmo_num failed, ret(%d).\n", ret);
        return -EINVAL;
#endif
    }

    if (cur_cmo_num >= DEVDRV_MAX_CMO_ID - 1) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("cur_cmo_num(%u), don't try to get cmo from opposite side, dev_id(%u).\n",
            cur_cmo_num, dev_id);
        return -ENODEV;
#endif
    }

    dev_msg_resource_id = (struct tsdrv_msg_resource_id *)msg_info.payload;
    tsdrv_construct_sync_data(&msg_info, dev_msg_resource_id, tsid, TSDRV_CHAN_SYNC_CMO, TSDRV_CMO_ID,
        TSDRV_MSG_GET_ID_NUM);
    ret = tsdrv_sync_msg_send(dev_id, &msg_info);
    if ((ret != 0) || (msg_info.header.result != 0) || (msg_info.header.valid != TSDRV_MSG_SYNC_RSP_MAGIC)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("device(%u) msg chan alloc cmo d2h failed, ret = %d, result = %u, valid = 0x%x.\n",
            dev_id, ret, (u32)msg_info.header.result, (u32)msg_info.header.valid);
        return -EFAULT;
#endif
    }

    ret = tsdrv_add_cmo_msg_chan(dev_id, tsid, dev_msg_resource_id);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("add_cmo_msg_chan failed, ret(%d).\n", ret);
        return -EINVAL;
#endif
    }
    ret = tsdrv_inc_ts_id_cur_num(dev_id, tsid, TSDRV_CMO_ID, dev_msg_resource_id->ret_id_num);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("inc cmo_id_num failed, ret(%d).\n", ret);
        return -EINVAL;
#endif
    }
    return 0;
}

s32 tsdrv_sync_cmo_proc(u32 devid, struct tsdrv_msg_info *msg, u32 *ack_len)
{
    struct tsdrv_msg_resource_id *dev_msg_resource_id = NULL;
    u32 cur_cmo_num;
    u32 tsid;
    int ret;

    tsid = msg->header.tsid;
    if (msg->header.valid != TSDRV_MSG_SYNC_MAGIC) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("devid %u invalid message from opposite.\n", devid);
        return -EINVAL;
#endif
    }

    dev_msg_resource_id = (struct tsdrv_msg_resource_id *)msg->payload;
    dev_msg_resource_id->ret_id_num = 0;
    if ((dev_msg_resource_id->id_type != TSDRV_CMO_ID) || (dev_msg_resource_id->req_id_num == 0)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("devid %u invalid request from opposite, id_type = 0x%x, req_id_num = %u.\n",
            devid, (u32)dev_msg_resource_id->id_type, dev_msg_resource_id->req_id_num);
        return -EINVAL;
#endif
    }
    ret = tsdrv_get_ts_id_cur_num(devid, tsid, TSDRV_CMO_ID, &cur_cmo_num);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("get cur_cmo_num failed, ret(%d).\n", ret);
        return -EINVAL;
#endif
    }

    ret = tsdrv_alloc_cmo_msg_chan(devid, tsid, dev_msg_resource_id);
    msg->header.result = ret;
    msg->header.valid = TSDRV_MSG_SYNC_RSP_MAGIC;
    *ack_len = sizeof(struct tsdrv_msg_info);
    if (cur_cmo_num < dev_msg_resource_id->ret_id_num) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("devid %u cur_cmo_num = %u, ret_id_num = %u.\n", devid, cur_cmo_num,
            dev_msg_resource_id->ret_id_num);
        return -EINVAL;
#endif
    }

    ret = tsdrv_dec_ts_id_cur_num(devid, tsid, TSDRV_CMO_ID, dev_msg_resource_id->ret_id_num);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("dec cmo_id_num failed, ret(%d).\n", ret);
        return -EINVAL;
#endif
    }

    return 0;
}
#endif

s32 tsdrv_msg_alloc_sync_stream(u32 dev_id, u32 tsid)
{
    struct tsdrv_ts_resource *ts_res = tsdrv_get_ts_resoruce(dev_id, TSDRV_PM_FID, tsid);
    struct tsdrv_msg_resource_id *dev_msg_resource_id = NULL;
    struct tsdrv_msg_info msg_info = {{0}};
    u32 cur_stream_num;
    int ret;

    ret = tsdrv_get_ts_id_cur_num(dev_id, tsid, TSDRV_STREAM_ID, &cur_stream_num);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("get cur_stream_num failed, ret(%d).\n", ret);
        return -EINVAL;
#endif
    }

    if (cur_stream_num >= ts_res->id_res[TSDRV_STREAM_ID].id_capacity) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Don't try to get stream from opposite side. (devid=%u; cur_stream_num=%u; capacity=%u)\n",
            dev_id, cur_stream_num, ts_res->id_res[TSDRV_STREAM_ID].id_capacity);
        return -ENODEV;
#endif
    }

    dev_msg_resource_id = (struct tsdrv_msg_resource_id *)msg_info.payload;
    tsdrv_construct_sync_data(&msg_info, dev_msg_resource_id, tsid, TSDRV_CHAN_SYNC_STREAM, TSDRV_STREAM_ID,
        TSDRV_MSG_GET_ID_NUM);
    ret = tsdrv_sync_msg_send(dev_id, &msg_info);
    if ((ret != 0) || (msg_info.header.result != 0) || (msg_info.header.valid != TSDRV_MSG_SYNC_RSP_MAGIC)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("device(%u) msg chan alloc stream failed, ret = %d, result = %u, valid = 0x%x.\n",
            dev_id, ret, (u32)msg_info.header.result, (u32)msg_info.header.valid);
        return -EFAULT;
#endif
    }
    ret = devdrv_add_stream_msg_chan(dev_id, tsid, dev_msg_resource_id);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("add_stream_msg_chan failed, ret(%d).\n", ret);
        return -EINVAL;
#endif
    }
    ret = tsdrv_inc_ts_id_cur_num(dev_id, tsid, TSDRV_STREAM_ID, dev_msg_resource_id->ret_id_num);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("inc stream_id_num failed, ret(%d).\n", ret);
        return -EINVAL;
#endif
    }
    return 0;
}

s32 tsdrv_msg_alloc_sync_event(u32 dev_id, u32 tsid)
{
    struct tsdrv_ts_resource *ts_res = tsdrv_get_ts_resoruce(dev_id, TSDRV_PM_FID, tsid);
    struct tsdrv_msg_resource_id *dev_msg_resource_id = NULL;
    struct tsdrv_msg_info msg_info = {{0}};
    u32 cur_hw_event_num, cur_sw_event_num, capacity;
    u32 tmp_hw_event_num = 0;
    u32 tmp_sw_event_num = 0;
    int ret;
    u32 i;

    if (tsdrv_get_ts_id_cur_num(dev_id, tsid, TSDRV_EVENT_HW_ID, &cur_hw_event_num) != 0) {
#ifndef TSDRV_UT
        return -EINVAL;
#endif
    }

    if (tsdrv_get_ts_id_cur_num(dev_id, tsid, TSDRV_EVENT_SW_ID, &cur_sw_event_num) != 0) {
#ifndef TSDRV_UT
        return -EINVAL;
#endif
    }

    capacity = ts_res->id_res[TSDRV_EVENT_HW_ID].id_capacity + ts_res->id_res[TSDRV_EVENT_SW_ID].id_capacity;
    if ((cur_hw_event_num + cur_sw_event_num) >= capacity) {
#ifndef TSDRV_UT
        TSDRV_PRINT_DEBUG("(cur_hw_event_num=%u; cur_sw_event_num=%u; capacity=%u)\n", cur_hw_event_num,
            cur_sw_event_num, capacity);
        return -EBUSY;
#endif
    }

    dev_msg_resource_id = (struct tsdrv_msg_resource_id *)msg_info.payload;
    tsdrv_construct_sync_data(&msg_info, dev_msg_resource_id, tsid, TSDRV_CHAN_SYNC_EVENT, TSDRV_EVENT_SW_ID,
        TSDRV_MSG_GET_ID_NUM);
    ret = tsdrv_sync_msg_send(dev_id, &msg_info);
    if ((ret != 0) || (msg_info.header.result != 0) || (msg_info.header.valid != TSDRV_MSG_SYNC_RSP_MAGIC)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("device(%u) msg chan alloc event failed, ret = %d, result = %u, valid = 0x%x.\n",
            dev_id, ret, (u32)msg_info.header.result, (u32)msg_info.header.valid);
        return -EFAULT;
#endif
    }

    ret = devdrv_add_event_msg_chan(dev_id, tsid, dev_msg_resource_id);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("inc add_event_msg_chan failed, ret(%d).\n", ret);
        return -EINVAL;
#endif
    }

    for (i = 0; i < dev_msg_resource_id->ret_id_num; i++) {
        if (tsdrv_get_pfvf_type_by_devid(dev_id) == DEVDRV_SRIOV_TYPE_PF) {
            if (dev_msg_resource_id->id[i] >= DEVDRV_MAX_HW_EVENT_ID) {
                tmp_sw_event_num++;
            } else {
                tmp_hw_event_num++;
            }
        } else {
            tmp_sw_event_num++;
        }
    }

    if (tsdrv_inc_ts_id_cur_num(dev_id, tsid, TSDRV_EVENT_HW_ID, tmp_hw_event_num) != 0) {
        return -EINVAL;
    }
    if (tsdrv_inc_ts_id_cur_num(dev_id, tsid, TSDRV_EVENT_SW_ID, tmp_sw_event_num) != 0) {
        return -EINVAL;
    }

    return 0;
}

STATIC s32 tsdrv_msg_alloc_sync_id_l(u32 dev_id, u32 tsid, enum tsdrv_id_type id_type)
{
    struct tsdrv_msg_resource_id *dev_msg_resource_id = NULL;
    struct tsdrv_dev_resource *pm_dev_res = NULL;
    struct tsdrv_msg_info msg_info = {{0}};
    u32 cur_hw_id_num = 0;
    u32 cur_sw_id_num = 0;
    u32 tmp_hw_id_num = 0;
    u32 tmp_sw_id_num = 0;
    u32 max_cnt;
    int ret;
    u32 i;

    pm_dev_res = tsdrv_get_dev_resource(dev_id, TSDRV_PM_FID);
    if (pm_dev_res == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("get dev resource failed, devid(%u) fid(%u).\n", dev_id, TSDRV_PM_FID);
        return -EINVAL;
#endif
    }

    if (tsdrv_get_ts_id_cur_num(dev_id, tsid, TSDRV_EVENT_HW_ID, &cur_hw_id_num) != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_DEBUG("hw(%u)\n", cur_hw_id_num);
        return -EINVAL;
#endif
    }

    if (tsdrv_get_ts_id_cur_num(dev_id, tsid, TSDRV_EVENT_SW_ID, &cur_sw_id_num) != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_DEBUG("sw(%u)\n", cur_sw_id_num);
        return -EINVAL;
#endif
    }

    max_cnt = pm_dev_res->ts_resource[tsid].id_res[TSDRV_EVENT_HW_ID].id_capacity +
        pm_dev_res->ts_resource[tsid].id_res[TSDRV_EVENT_SW_ID].id_capacity;
    if ((cur_hw_id_num + cur_sw_id_num) >= max_cnt) {
        return -EAGAIN;
    }

    dev_msg_resource_id = (struct tsdrv_msg_resource_id *)msg_info.payload;
    tsdrv_construct_sync_data(&msg_info, dev_msg_resource_id, tsid, TSDRV_CHAN_SYNC_RES_ID, id_type,
        TSDRV_MSG_GET_ID_NUM);
    ret = tsdrv_sync_msg_send(dev_id, &msg_info);
    if ((ret != 0) || (msg_info.header.result != 0) || (msg_info.header.valid != TSDRV_MSG_SYNC_RSP_MAGIC) ||
        (dev_msg_resource_id->ret_id_num == 0)) {
        TSDRV_PRINT_ERR("device(%u) msg chan alloc failed, ret=%d, result=%u, valid=0x%x. id_type=%d ret_num=%u\n",
            dev_id, ret, (u32)msg_info.header.result, (u32)msg_info.header.valid, (int)id_type,
            dev_msg_resource_id->ret_id_num);
        return -EFAULT;
    }

    ret = tsdrv_get_drv_ops()->add_id_msg_chan(dev_id, tsid, dev_msg_resource_id);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("inc add_id_msg_chan failed, ret(%d),id_type(%d).\n", ret, (int)id_type);
        return -EINVAL;
#endif
    }

    for (i = 0; i < dev_msg_resource_id->ret_id_num; i++) {
        if (dev_msg_resource_id->id[i] >= DEVDRV_MAX_HW_EVENT_ID) {
            tmp_sw_id_num++;
        } else {
            tmp_hw_id_num++;
        }
    }

    if (tsdrv_inc_ts_id_cur_num(dev_id, tsid, TSDRV_EVENT_HW_ID, tmp_hw_id_num) != 0) {
        return -EINVAL;
    }
    if (tsdrv_inc_ts_id_cur_num(dev_id, tsid, TSDRV_EVENT_SW_ID, tmp_sw_id_num) != 0) {
        return -EINVAL;
    }

    return 0;
}
s32 tsdrv_msg_sync_id_map(u32 dev_id, struct tsdrv_id_map_sync_info id_map)
{
    struct tsdrv_msg_info msg_info = {{0}};
    s32 ret;

    msg_info.header.msg_id = TSDRV_CHAN_SYNC_ID_MAP;
    msg_info.header.valid = TSDRV_MSG_SYNC_MAGIC;
    msg_info.header.result = TSDRV_MSG_INVALID_RESULT;
    msg_info.header.tsid = id_map.event.tsid;
    ret = memcpy_s(msg_info.payload, TSDRV_MSG_INFO_PAYLOAD_LEN, &id_map, sizeof(struct tsdrv_id_map_sync_info));
#ifndef TSDRV_UT
    if (ret != 0) {
        TSDRV_PRINT_ERR("memcpy failed. (ret=%d)\n", ret);
        return -EFAULT;
    }
#endif
    ret = tsdrv_sync_msg_send(dev_id, &msg_info);
    if ((ret != 0) || (msg_info.header.result != 0) || (msg_info.header.valid != TSDRV_MSG_SYNC_RSP_MAGIC)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("device(%u) sync failed, ret=%d, result=%u, valid=0x%x. id_type=%d\n",
            dev_id, ret, (u32)msg_info.header.result, (u32)msg_info.header.valid, (int)id_map.id_type);
        return -EFAULT;
#endif
    }

    return 0;
}

s32 tsdrv_msg_alloc_sync_id(u32 dev_id, u32 tsid, enum tsdrv_id_type id_type)
{
    struct tsdrv_msg_resource_id *dev_msg_resource_id = NULL;
    struct tsdrv_dev_resource *pm_dev_res = NULL;
    struct tsdrv_msg_info msg_info = {{0}};
    u32 cur_id_num;
    int ret;

    pm_dev_res = tsdrv_get_dev_resource(dev_id, TSDRV_PM_FID);
    if (pm_dev_res == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("get dev resource failed, devid(%u) fid(%u).\n", dev_id, TSDRV_PM_FID);
        return -EINVAL;
#endif
    }

    if ((id_type == TSDRV_EVENT_HW_ID) || (id_type == TSDRV_EVENT_SW_ID)) {
        ret = tsdrv_msg_alloc_sync_id_l(dev_id, tsid, id_type);
        if (ret != 0) {
#ifndef TSDRV_UT
            TSDRV_PRINT_ERR("id sync failed, ret(%d),id_type(%d).\n", ret, (int)id_type);
            return -EINVAL;
#endif
        }
    } else {
        if (tsdrv_get_ts_id_cur_num(dev_id, tsid, id_type, &cur_id_num) != 0) {
            return -EINVAL;
        }
        if (cur_id_num >= pm_dev_res->ts_resource[tsid].id_res[id_type].id_capacity) {
            return -EAGAIN;
        }

        dev_msg_resource_id = (struct tsdrv_msg_resource_id *)msg_info.payload;
        tsdrv_construct_sync_data(&msg_info, dev_msg_resource_id, tsid, TSDRV_CHAN_SYNC_RES_ID, id_type,
            TSDRV_MSG_GET_ID_NUM);
        ret = tsdrv_sync_msg_send(dev_id, &msg_info);
        if ((ret != 0) || (msg_info.header.result != 0) || (msg_info.header.valid != TSDRV_MSG_SYNC_RSP_MAGIC) ||
            (dev_msg_resource_id->ret_id_num == 0)) {
#ifndef TSDRV_UT
            TSDRV_PRINT_ERR("device(%u) sync failed, ret=%d, result=%u, valid=0x%x. id_type=%d ret_num=%u\n",
                dev_id, ret, (u32)msg_info.header.result, (u32)msg_info.header.valid, (int)id_type,
                dev_msg_resource_id->ret_id_num);
            return -EFAULT;
#endif
        }

        ret = tsdrv_get_drv_ops()->add_id_msg_chan(dev_id, tsid, dev_msg_resource_id);
        if (ret != 0) {
            TSDRV_PRINT_ERR("inc add_id_msg_chan failed, ret(%d),id_type(%d).\n", ret, (int)id_type);
            return -EINVAL;
        }

        if (tsdrv_inc_ts_id_cur_num(dev_id, tsid, id_type, dev_msg_resource_id->ret_id_num) != 0) {
            return -EINVAL;
        }
    }

    return 0;
}

s32 tsdrv_msg_alloc_sync_ipc_event(u32 dev_id, u32 tsid)
{
    struct tsdrv_msg_resource_id *dev_msg_resource_id = NULL;
    struct tsdrv_msg_info msg_info = {{0}};
    u32 cur_id_num;
    int ret;

    if (tsdrv_get_ts_id_cur_num(dev_id, tsid, TSDRV_IPC_EVENT_ID, &cur_id_num) != 0) {
        return -EINVAL;
    }

    dev_msg_resource_id = (struct tsdrv_msg_resource_id *)msg_info.payload;
    tsdrv_construct_sync_data(&msg_info, dev_msg_resource_id, tsid, TSDRV_CHAN_SYNC_IPC_EVENT, TSDRV_IPC_EVENT_ID,
        TSDRV_MSG_GET_ID_NUM);
    ret = tsdrv_sync_msg_send(dev_id, &msg_info);
    if ((ret != 0) || (msg_info.header.result != 0) || (msg_info.header.valid != TSDRV_MSG_SYNC_RSP_MAGIC)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("device(%u) msg chan alloc ipc event failed,ret(%d),result(%u),valid(0x%x).\n",
            dev_id, ret, (u32)msg_info.header.result, (u32)msg_info.header.valid);
        return -EFAULT;
#endif
    }

    ret = tsdrv_get_drv_ops()->add_ipc_event_msg_chan(dev_id, tsid, dev_msg_resource_id);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("inc add_ipc_event_msg_chan failed, ret(%d).\n", ret);
        return -EINVAL;
#endif
    }

    if (tsdrv_inc_ts_id_cur_num(dev_id, tsid, TSDRV_IPC_EVENT_ID, dev_msg_resource_id->ret_id_num) != 0) {
        return -EINVAL;
    }
    return 0;
}

s32 tsdrv_msg_alloc_sync_sq(u32 dev_id, u32 tsid)
{
    struct tsdrv_msg_resource_id *dev_msg_resource_id = NULL;
    struct tsdrv_msg_info msg_info = {{0}};
    u32 cur_sq_num;
    int ret;

    ret = tsdrv_get_ts_id_cur_num(dev_id, tsid, TSDRV_SQ_ID, &cur_sq_num);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("get cur_sq_num failed, ret(%d).\n", ret);
        return -EINVAL;
#endif
    }

    dev_msg_resource_id = (struct tsdrv_msg_resource_id *)msg_info.payload;
    tsdrv_construct_sync_data(&msg_info, dev_msg_resource_id, tsid, TSDRV_CHAN_SYNC_SQ, TSDRV_SQ_ID,
        TSDRV_MSG_GET_ID_NUM);
    ret = tsdrv_sync_msg_send(dev_id, &msg_info);
    if ((ret != 0) || (msg_info.header.result != 0) || (msg_info.header.valid != TSDRV_MSG_SYNC_RSP_MAGIC)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("device(%u) msg chan alloc sq failed, ret = %d, result = %u, valid = 0x%x.\n",
            dev_id, ret, (u32)msg_info.header.result, (u32)msg_info.header.valid);
        return -EFAULT;
#endif
    }

    ret = devdrv_add_sq_msg_chan(dev_id, tsid, dev_msg_resource_id);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("add_sq_msg_chan failed, ret(%d).\n", ret);
        return -EINVAL;
#endif
    }
    ret = tsdrv_inc_ts_id_cur_num(dev_id, tsid, TSDRV_SQ_ID, dev_msg_resource_id->ret_id_num);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("inc sq_num failed, ret(%d).\n", ret);
        return -EINVAL;
#endif
    }

    return 0;
}

s32 tsdrv_msg_alloc_sync_cq(u32 dev_id, u32 tsid)
{
    struct tsdrv_ts_resource *ts_res = tsdrv_get_ts_resoruce(dev_id, TSDRV_PM_FID, tsid);
    struct tsdrv_msg_resource_id *dev_msg_resource_id = NULL;
    struct tsdrv_msg_info msg_info = {{0}};
    u32 cur_cq_num;

    int ret = tsdrv_get_ts_id_cur_num(dev_id, tsid, TSDRV_CQ_ID, &cur_cq_num);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("get cur_cq_num failed, ret(%d).\n", ret);
        return -EINVAL;
#endif
    }

    if (cur_cq_num >= ts_res->id_res[TSDRV_CQ_ID].id_capacity) {
#ifndef TSDRV_UT
        TSDRV_PRINT_INFO("Don't try to get CQ from opposite side. (devid=%u; cur_cq_num=%u; capaicty=%u)\n",
            dev_id, cur_cq_num, ts_res->id_res[TSDRV_CQ_ID].id_capacity);
        return -ENODEV;
#endif
    }

    dev_msg_resource_id = (struct tsdrv_msg_resource_id *)msg_info.payload;
    tsdrv_construct_sync_data(&msg_info, dev_msg_resource_id, tsid, TSDRV_CHAN_SYNC_CQ, TSDRV_CQ_ID,
        TSDRV_MSG_GET_ID_NUM);
    ret = tsdrv_sync_msg_send(dev_id, &msg_info);
    if ((ret != 0) || (msg_info.header.result != 0) || (msg_info.header.valid != TSDRV_MSG_SYNC_RSP_MAGIC)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("device(%u) msg chan alloc cq failed, ret = %d, result = %u, valid = 0x%x.\n",
            dev_id, ret, (u32)msg_info.header.result, (u32)msg_info.header.valid);
        return -EFAULT;
#endif
    }
    if (dev_msg_resource_id->ret_id_num == 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Sync cq fail, (ret_cq_num=%u; cur_cq_num=%u)\n", dev_msg_resource_id->ret_id_num, cur_cq_num);
        return -ENODEV;
#endif
    }
    ret = devdrv_add_cq_msg_chan(dev_id, tsid, dev_msg_resource_id);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("add_cq_msg_chan failed, ret(%d).\n", ret);
        return -EINVAL;
#endif
    }
    ret = tsdrv_inc_ts_id_cur_num(dev_id, tsid, TSDRV_CQ_ID, dev_msg_resource_id->ret_id_num);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("inc cq_num failed, ret(%d).\n", ret);
        return -EINVAL;
#endif
    }
    return 0;
}

s32 tsdrv_msg_alloc_sync_model(u32 dev_id, u32 tsid)
{
    struct tsdrv_ts_resource *ts_res = tsdrv_get_ts_resoruce(dev_id, TSDRV_PM_FID, tsid);
    struct tsdrv_msg_resource_id *dev_msg_resource_id = NULL;
    struct tsdrv_msg_info msg_info = {{0}};
    u32 cur_model_num;
    int ret;

    ret = tsdrv_get_ts_id_cur_num(dev_id, tsid, TSDRV_MODEL_ID, &cur_model_num);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("get cur_model_num failed, ret(%d).\n", ret);
        return -EINVAL;
#endif
    }

    if (cur_model_num >= ts_res->id_res[TSDRV_MODEL_ID].id_capacity) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Don't try to get model from opposite side. (devid=%u; cur_model_num=%u; capaicty=%u)\n",
            dev_id, cur_model_num, ts_res->id_res[TSDRV_MODEL_ID].id_capacity);
        return -ENODEV;
#endif
    }

    dev_msg_resource_id = (struct tsdrv_msg_resource_id *)msg_info.payload;
    tsdrv_construct_sync_data(&msg_info, dev_msg_resource_id, tsid, TSDRV_CHAN_SYNC_MODEL, TSDRV_MODEL_ID,
        TSDRV_MSG_GET_ID_NUM);
    ret = tsdrv_sync_msg_send(dev_id, &msg_info);
    if ((ret != 0) || (msg_info.header.result != 0) || (msg_info.header.valid != TSDRV_MSG_SYNC_RSP_MAGIC)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("device(%u) msg chan alloc model d2h failed, ret = %d, result = %u, valid = 0x%x.\n",
            dev_id, ret, (u32)msg_info.header.result, (u32)msg_info.header.valid);
        return -EFAULT;
#endif
    }

    ret = devdrv_add_model_msg_chan(dev_id, tsid, dev_msg_resource_id);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("add_model_msg_chan failed, ret(%d).\n", ret);
        return -EINVAL;
#endif
    }
    ret = tsdrv_inc_ts_id_cur_num(dev_id, tsid, TSDRV_MODEL_ID, dev_msg_resource_id->ret_id_num);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("inc model_id_num failed, ret(%d).\n", ret);
        return -EINVAL;
#endif
    }

    return 0;
}

s32 tsdrv_msg_alloc_sync_notify(u32 dev_id, u32 tsid)
{
    struct tsdrv_ts_resource *ts_res = tsdrv_get_ts_resoruce(dev_id, TSDRV_PM_FID, tsid);
    struct tsdrv_msg_resource_id *dev_msg_resource_id = NULL;
    struct tsdrv_msg_info msg_info = {{0}};
    u32 cur_notify_num;

    int ret = tsdrv_get_ts_id_cur_num(dev_id, tsid, TSDRV_NOTIFY_ID, &cur_notify_num);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("get cur_notify_num failed, ret(%d).\n", ret);
        return -EINVAL;
#endif
    }

    if (cur_notify_num >= ts_res->id_res[TSDRV_NOTIFY_ID].id_capacity) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Don't try to get notiy id from opposite side. (devid=%u; cur_notify_num=%u; capaicty=%u)\n",
            dev_id, cur_notify_num, ts_res->id_res[TSDRV_NOTIFY_ID].id_capacity);
        return -ENODEV;
#endif
    }

    dev_msg_resource_id = (struct tsdrv_msg_resource_id *)msg_info.payload;
    tsdrv_construct_sync_data(&msg_info, dev_msg_resource_id, tsid, TSDRV_CHAN_SYNC_NOTIFY, TSDRV_NOTIFY_ID,
        TSDRV_MSG_GET_ID_NUM);
    ret = tsdrv_sync_msg_send(dev_id, &msg_info);
    if ((ret != 0) || (msg_info.header.result != 0) || (msg_info.header.valid != TSDRV_MSG_SYNC_RSP_MAGIC)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("device(%u) msg chan alloc notify d2h failed, ret = %d, result = %u, valid = 0x%x.\n",
            dev_id, ret, (u32)msg_info.header.result, (u32)msg_info.header.valid);
        return -EFAULT;
#endif
    }

    ret = tsdrv_get_drv_ops()->add_notify_msg_chan(dev_id, tsid, dev_msg_resource_id);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("add_notify_msg_chan failed, ret(%d).\n", ret);
        return -EINVAL;
#endif
    }
    ret = tsdrv_inc_ts_id_cur_num(dev_id, tsid, TSDRV_NOTIFY_ID, dev_msg_resource_id->ret_id_num);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("inc notify_id_num failed, ret(%d).\n", ret);
        return -EINVAL;
#endif
    }

    return 0;
}

s32 tsdrv_msg_sync_stream_sqcq(u32 dev_id, u32 tsid, u32 stream_id, u32 valid, u32 sq_id, u32 cq_id)
{
    struct tsdrv_msg_info msg_head;
    struct tsdrv_msg_sync_stream_sqcq *msg = (struct tsdrv_msg_sync_stream_sqcq *)msg_head.payload;
    int ret;

    msg_head.header.msg_id = TSDRV_CHAN_SYNC_STREAM_SQ_CQ;
    msg_head.header.valid = TSDRV_MSG_SYNC_MAGIC;
    msg_head.header.result = TSDRV_MSG_INVALID_RESULT;
    msg_head.header.tsid = tsid;

    msg->stream_id = stream_id;
    msg->valid = valid;
    msg->sq_id = sq_id;
    msg->cq_id = cq_id;
    msg->remote_tgid = current->tgid;

    ret = tsdrv_sync_msg_send(dev_id, &msg_head);
    if ((ret != 0) || (msg_head.header.result != 0) || (msg_head.header.valid != TSDRV_MSG_SYNC_RSP_MAGIC)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("device(%u) msg chan send failed, ret = %d, result = %u, valid = 0x%x.\n",
            dev_id, ret, (u32)msg_head.header.result, (u32)msg_head.header.valid);
        return -EFAULT;
#endif
    }

    return 0;
}

s32 tsdrv_msg_phy_cbcqsq_exit(u32 dev_id, u32 tsid)
{
    struct tsdrv_msg_info msg_head;
    int ret;

    msg_head.header.msg_id = TSDRV_CHAN_SYNC_PHY_CBCQSQ_EXIT;
    msg_head.header.valid = TSDRV_MSG_SYNC_MAGIC;
    msg_head.header.result = TSDRV_MSG_INVALID_RESULT;
    msg_head.header.tsid = tsid;

    ret = tsdrv_sync_msg_send(dev_id, &msg_head);
    if ((ret != 0) || (msg_head.header.result != 0) || (msg_head.header.valid != TSDRV_MSG_SYNC_RSP_MAGIC)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("device(%u) msg chan send failed, ret = %d, result = %u, valid = 0x%x.\n",
            dev_id, ret, (u32)msg_head.header.result, (u32)msg_head.header.valid);
        return -EFAULT;
#endif
    }

    return 0;
}

s32 tsdrv_sync_stream_proc(u32 devid, struct tsdrv_msg_info *msg, u32 *ack_len)
{
    struct tsdrv_msg_resource_id *dev_msg_resource_id = NULL;
    u32 cur_stream_num;
    u32 tsid;
    int ret;

    tsid = msg->header.tsid;
    if (msg->header.valid != TSDRV_MSG_SYNC_MAGIC) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("devid %u invalid message from opposite.\n", devid);
        return -EINVAL;
#endif
    }

    dev_msg_resource_id = (struct tsdrv_msg_resource_id *)msg->payload;
    if ((dev_msg_resource_id->id_type != TSDRV_STREAM_ID) || (dev_msg_resource_id->req_id_num == 0) ||
        (dev_msg_resource_id->ret_id_num != 0)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("invalid request:id_type = 0x%x, req_id_num = %u, ret_id_num = %u.\n",
            (u32)dev_msg_resource_id->id_type, dev_msg_resource_id->req_id_num, dev_msg_resource_id->ret_id_num);
        return -EINVAL;
#endif
    }

    ret = tsdrv_get_ts_id_cur_num(devid, tsid, TSDRV_STREAM_ID, &cur_stream_num);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("get cur_stream_num failed, ret(%d).\n", ret);
        return -EINVAL;
#endif
    }
    ret = devdrv_alloc_stream_msg_chan(devid, tsid, dev_msg_resource_id);
    msg->header.result = ret;
    msg->header.valid = TSDRV_MSG_SYNC_RSP_MAGIC;
    *ack_len = sizeof(struct tsdrv_msg_info);

    if (cur_stream_num >= dev_msg_resource_id->ret_id_num) {
        ret = tsdrv_dec_ts_id_cur_num(devid, tsid, TSDRV_STREAM_ID, dev_msg_resource_id->ret_id_num);
        if (ret != 0) {
#ifndef TSDRV_UT
            TSDRV_PRINT_ERR("dec stream_id_num failed, ret(%d).\n", ret);
            return -EINVAL;
#endif
        }
    } else {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("devid %u invalid ret_id_num=%u, cur_stream_num=%u.\n", devid,
            dev_msg_resource_id->ret_id_num, cur_stream_num);
        return -EINVAL;
#endif
    }

    return 0;
}

s32 tsdrv_sync_sq_proc(u32 devid, struct tsdrv_msg_info *msg, u32 *ack_len)
{
    struct tsdrv_msg_resource_id *dev_msg_resource_id = NULL;
    u32 cur_sq_num;
    u32 tsid = 0;
    int ret;

    if (msg->header.valid != TSDRV_MSG_SYNC_MAGIC) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("devid %u invalid message from opposite.\n", devid);
        return -EINVAL;
#endif
    }

    dev_msg_resource_id = (struct tsdrv_msg_resource_id *)msg->payload;
    dev_msg_resource_id->ret_id_num = 0;
    if ((dev_msg_resource_id->id_type != TSDRV_SQ_ID) || (dev_msg_resource_id->req_id_num == 0)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("devid %u invalid request from opposite, id_type = 0x%x, req_id_num = %u\n",
            devid, (u32)dev_msg_resource_id->id_type, dev_msg_resource_id->req_id_num);
        return -EINVAL;
#endif
    }

    ret = tsdrv_get_ts_id_cur_num(devid, tsid, TSDRV_SQ_ID, &cur_sq_num);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("get cur_sq_num failed, ret(%d).\n", ret);
        return -EINVAL;
#endif
    }

    ret = devdrv_alloc_sq_msg_chan(devid, tsid, dev_msg_resource_id);
    msg->header.result = ret;
    msg->header.valid = TSDRV_MSG_SYNC_RSP_MAGIC;
    *ack_len = sizeof(struct tsdrv_msg_info);
    if (cur_sq_num < dev_msg_resource_id->ret_id_num) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("devid %u cur_sq = %u, ret_id = %u.\n", devid, cur_sq_num, dev_msg_resource_id->ret_id_num);
        return -EINVAL;
#endif
    }
    ret = tsdrv_dec_ts_id_cur_num(devid, tsid, TSDRV_SQ_ID, dev_msg_resource_id->ret_id_num);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("dec event_id_num failed, ret(%d).\n", ret);
        return -EINVAL;
#endif
    }

    return 0;
}

s32 tsdrv_sync_cq_proc(u32 devid, struct tsdrv_msg_info *msg, u32 *ack_len)
{
    struct tsdrv_msg_resource_id *dev_msg_resource_id = NULL;
    u32 cur_cq_num;
    u32 tsid = 0;
    int ret;

    if (msg->header.valid != TSDRV_MSG_SYNC_MAGIC) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("devid %u invalid message from opposite.\n", devid);
        return -EINVAL;
#endif
    }

    dev_msg_resource_id = (struct tsdrv_msg_resource_id *)msg->payload;
    dev_msg_resource_id->ret_id_num = 0;
    if ((dev_msg_resource_id->id_type != TSDRV_CQ_ID) || (dev_msg_resource_id->req_id_num == 0)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("devid %u invalid request from opposite, id_type = 0x%x, req_id_num = %u.\n",
            devid, (u32)dev_msg_resource_id->id_type, dev_msg_resource_id->req_id_num);
        return -EINVAL;
#endif
    }

    ret = tsdrv_get_ts_id_cur_num(devid, tsid, TSDRV_CQ_ID, &cur_cq_num);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("get cur_cq_num failed, ret(%d).\n", ret);
        return -EINVAL;
#endif
    }

    ret = devdrv_alloc_cq_msg_chan(devid, tsid, dev_msg_resource_id);
    msg->header.result = ret;
    msg->header.valid = TSDRV_MSG_SYNC_RSP_MAGIC;
    *ack_len = sizeof(struct tsdrv_msg_info);
    if (cur_cq_num < dev_msg_resource_id->ret_id_num) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("cur_cq_num = %u, ret_id_num = %u.\n", cur_cq_num, dev_msg_resource_id->ret_id_num);
        return -EINVAL;
#endif
    }

    ret = tsdrv_dec_ts_id_cur_num(devid, tsid, TSDRV_CQ_ID, dev_msg_resource_id->ret_id_num);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("dec cq_id_num failed, ret(%d).\n", ret);
        return -EINVAL;
#endif
    }
    return 0;
}

s32 tsdrv_sync_event_proc(u32 devid, struct tsdrv_msg_info *msg, u32 *ack_len)
{
    struct tsdrv_msg_resource_id *dev_msg_resource_id = NULL;
    u32 cur_event_hw_num, cur_event_sw_num;
    u32 tmp_event_hw_num = 0;
    u32 tmp_event_sw_num = 0;
    u32 tsid;
    int ret;
    u32 i;

    tsid = msg->header.tsid;
    if (msg->header.valid != TSDRV_MSG_SYNC_MAGIC) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("devid %u invalid message from opposite.\n", devid);
        return -EINVAL;
#endif
    }

    dev_msg_resource_id = (struct tsdrv_msg_resource_id *)msg->payload;
    dev_msg_resource_id->ret_id_num = 0;
    if ((dev_msg_resource_id->id_type != TSDRV_EVENT_SW_ID) || (dev_msg_resource_id->req_id_num == 0)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("devid %u invalid request from opposite, id_type = 0x%x, req_id_num = %u.\n",
            devid, (u32)dev_msg_resource_id->id_type, dev_msg_resource_id->req_id_num);
        return -EINVAL;
#endif
    }

    if ((tsdrv_get_ts_id_cur_num(devid, tsid, TSDRV_EVENT_SW_ID, &cur_event_sw_num) != 0) ||
        (tsdrv_get_ts_id_cur_num(devid, tsid, TSDRV_EVENT_HW_ID, &cur_event_hw_num) != 0)) {
        return -EINVAL;
    }

    ret = devdrv_alloc_event_msg_chan(devid, tsid, dev_msg_resource_id);
    msg->header.result = ret;
    msg->header.valid = TSDRV_MSG_SYNC_RSP_MAGIC;
    *ack_len = sizeof(struct tsdrv_msg_info);
    if ((cur_event_hw_num + cur_event_sw_num) < dev_msg_resource_id->ret_id_num) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("devid %u cur_event_num hw(%u) sw(%u), ret_id_num = %u\n",
            devid, cur_event_hw_num, cur_event_sw_num, dev_msg_resource_id->ret_id_num);
        return -EINVAL;
#endif
    }
    for (i = 0; i < dev_msg_resource_id->ret_id_num; i++) {
        if (dev_msg_resource_id->id[i] >= DEVDRV_MAX_HW_EVENT_ID) {
            tmp_event_sw_num++;
        } else {
            tmp_event_hw_num++;
        }
    }
    if (tsdrv_dec_ts_id_cur_num(devid, tsid, TSDRV_EVENT_HW_ID, tmp_event_hw_num) != 0) {
        return -EINVAL;
    }
    if (tsdrv_dec_ts_id_cur_num(devid, tsid, TSDRV_EVENT_SW_ID, tmp_event_sw_num) != 0) {
        return -EINVAL;
    }

    return 0;
}

s32 tsdrv_sync_res_id_proc_l(u32 devid, u32 tsid, struct tsdrv_msg_info *msg, u32 *ack_len)
{
    struct tsdrv_msg_resource_id *dev_msg_resource_id = (struct tsdrv_msg_resource_id *)msg->payload;
    u32 cur_id_hw_num, cur_id_sw_num;
    u32 tmp_id_hw_num = 0;
    u32 tmp_id_sw_num = 0;
    int ret;
    u32 i;

    if ((tsdrv_get_ts_id_cur_num(devid, tsid, TSDRV_EVENT_SW_ID, &cur_id_sw_num) != 0) ||
        (tsdrv_get_ts_id_cur_num(devid, tsid, TSDRV_EVENT_HW_ID, &cur_id_hw_num) != 0)) {
        return -EINVAL;
    }

    ret = devdrv_alloc_id_msg_chan(devid, tsid, dev_msg_resource_id);
    msg->header.result = ret;
    msg->header.valid = TSDRV_MSG_SYNC_RSP_MAGIC;
    *ack_len = sizeof(struct tsdrv_msg_info);
    if ((cur_id_hw_num + cur_id_sw_num) < dev_msg_resource_id->ret_id_num) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("devid(%u) cur_event_num hw(%u) sw(%u),ret_id_num(%u)\n",
            devid, cur_id_hw_num, cur_id_sw_num, dev_msg_resource_id->ret_id_num);
        return -EINVAL;
#endif
    }
    for (i = 0; i < dev_msg_resource_id->ret_id_num; i++) {
        if (dev_msg_resource_id->id[i] >= DEVDRV_MAX_HW_EVENT_ID) {
            tmp_id_sw_num++;
        } else {
            tmp_id_hw_num++;
        }
    }
    if (tsdrv_dec_ts_id_cur_num(devid, tsid, TSDRV_EVENT_HW_ID, tmp_id_hw_num) != 0) {
        return -EINVAL;
    }
    if (tsdrv_dec_ts_id_cur_num(devid, tsid, TSDRV_EVENT_SW_ID, tmp_id_sw_num) != 0) {
        return -EINVAL;
    }
    return 0;
}

s32 tsdrv_sync_res_id_proc(u32 devid, struct tsdrv_msg_info *msg, u32 *ack_len)
{
    struct tsdrv_msg_resource_id *dev_msg_resource_id = NULL;
    u32 cur_id_num;
    enum tsdrv_id_type id_type;
    u32 tsid;
    int ret;

    tsid = msg->header.tsid;
    if (msg->header.valid != TSDRV_MSG_SYNC_MAGIC) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("devid(%u) invalid message from opposite.\n", devid);
        return -EINVAL;
#endif
    }

    dev_msg_resource_id = (struct tsdrv_msg_resource_id *)msg->payload;
    dev_msg_resource_id->ret_id_num = 0;
    id_type = dev_msg_resource_id->id_type;
    if ((id_type >= TSDRV_MAX_ID) || (dev_msg_resource_id->req_id_num == 0)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("devid(%u) invalid request from opposite, id_type(0x%x), req_id_num(%u).\n",
            devid, (u32)dev_msg_resource_id->id_type, dev_msg_resource_id->req_id_num);
        return -EINVAL;
#endif
    }

    if ((id_type == TSDRV_EVENT_HW_ID) || (id_type == TSDRV_EVENT_SW_ID)) {
        ret = tsdrv_sync_res_id_proc_l(devid, tsid, msg, ack_len);
        if (ret != 0) {
#ifndef TSDRV_UT
            TSDRV_PRINT_ERR("id sync failed, ret(%d) id_type(%d).\n", ret, (int)id_type);
            return -EINVAL;
#endif
        }
    } else {
        ret = tsdrv_get_ts_id_cur_num(devid, tsid, id_type, &cur_id_num);
        if (ret != 0) {
#ifndef TSDRV_UT
            TSDRV_PRINT_ERR("get cur_num failed, ret(%d) id_type(%d).\n", ret, (int)id_type);
            return -EINVAL;
#endif
        }

        ret = devdrv_alloc_id_msg_chan(devid, tsid, dev_msg_resource_id);
        msg->header.result = ret;
        msg->header.valid = TSDRV_MSG_SYNC_RSP_MAGIC;
        *ack_len = sizeof(struct tsdrv_msg_info);
        if (cur_id_num < dev_msg_resource_id->ret_id_num) {
#ifndef TSDRV_UT
            TSDRV_PRINT_ERR("devid(%u) cur_num(%u) id_type(%d),ret_id_num(%u)\n",
                devid, cur_id_num, (int)id_type, dev_msg_resource_id->ret_id_num);
            return -EINVAL;
#endif
        }

        if (tsdrv_dec_ts_id_cur_num(devid, tsid, id_type, dev_msg_resource_id->ret_id_num) != 0) {
            return -EINVAL;
        }
    }
    return 0;
}


s32 tsdrv_sync_ipc_event_proc(u32 devid, struct tsdrv_msg_info *msg, u32 *ack_len)
{
    struct tsdrv_msg_resource_id *dev_msg_resource_id = NULL;
    u32 cur_id_num;
    u32 tsid;
    int ret;

    tsid = msg->header.tsid;
    if (msg->header.valid != TSDRV_MSG_SYNC_MAGIC) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("devid %u invalid message from opposite.\n", devid);
        return -EINVAL;
#endif
    }

    dev_msg_resource_id = (struct tsdrv_msg_resource_id *)msg->payload;
    dev_msg_resource_id->ret_id_num = 0;
    if ((dev_msg_resource_id->id_type != TSDRV_IPC_EVENT_ID) || (dev_msg_resource_id->req_id_num == 0)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("devid %u invalid request from opposite, id_type = 0x%x, req_id_num = %u.\n",
            devid, (u32)dev_msg_resource_id->id_type, dev_msg_resource_id->req_id_num);
        return -EINVAL;
#endif
    }

    ret = tsdrv_get_ts_id_cur_num(devid, tsid, TSDRV_IPC_EVENT_ID, &cur_id_num);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("get cur_ipc_event_num failed, ret(%d).\n", ret);
        return -EINVAL;
#endif
    }

    ret = devdrv_alloc_ipc_event_msg_chan(devid, tsid, dev_msg_resource_id);
    msg->header.result = ret;
    msg->header.valid = TSDRV_MSG_SYNC_RSP_MAGIC;
    *ack_len = sizeof(struct tsdrv_msg_info);
    if (cur_id_num < dev_msg_resource_id->ret_id_num) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("devid %u cur_ipc_event_num (%u), ret_id_num = %u\n",
            devid, cur_id_num, dev_msg_resource_id->ret_id_num);
        return -EINVAL;
#endif
    }

    ret = tsdrv_dec_ts_id_cur_num(devid, tsid, TSDRV_IPC_EVENT_ID, dev_msg_resource_id->ret_id_num);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("dec ipc_event_id_num failed, ret(%d).\n", ret);
        return -EINVAL;
#endif
    }
    return 0;
}

s32 tsdrv_sync_model_proc(u32 devid, struct tsdrv_msg_info *msg, u32 *ack_len)
{
    struct tsdrv_msg_resource_id *dev_msg_resource_id = NULL;
    u32 cur_model_num;
    u32 tsid;
    int ret;

    tsid = msg->header.tsid;
    if (msg->header.valid != TSDRV_MSG_SYNC_MAGIC) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("devid %u invalid message from opposite.\n", devid);
        return -EINVAL;
#endif
    }

    dev_msg_resource_id = (struct tsdrv_msg_resource_id *)msg->payload;
    dev_msg_resource_id->ret_id_num = 0;
    if ((dev_msg_resource_id->id_type != TSDRV_MODEL_ID) || (dev_msg_resource_id->req_id_num == 0)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("devid %u invalid request from opposite, id_type = 0x%x, req_id_num = %u.\n",
            devid, (u32)dev_msg_resource_id->id_type, dev_msg_resource_id->req_id_num);
        return -EINVAL;
#endif
    }
    ret = tsdrv_get_ts_id_cur_num(devid, tsid, TSDRV_MODEL_ID, &cur_model_num);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("get cur_model_num failed, ret(%d).\n", ret);
        return -EINVAL;
#endif
    }

    ret = devdrv_alloc_model_msg_chan(devid, tsid, dev_msg_resource_id);
    msg->header.result = ret;
    msg->header.valid = TSDRV_MSG_SYNC_RSP_MAGIC;
    *ack_len = sizeof(struct tsdrv_msg_info);
    if (cur_model_num < dev_msg_resource_id->ret_id_num) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("devid %u cur_model_num = %u, ret_id_num = %u.\n", devid, cur_model_num,
            dev_msg_resource_id->ret_id_num);
        return -EINVAL;
#endif
    }

    ret = tsdrv_dec_ts_id_cur_num(devid, tsid, TSDRV_MODEL_ID, dev_msg_resource_id->ret_id_num);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("dec model_id_num failed, ret(%d).\n", ret);
        return -EINVAL;
#endif
    }

    return 0;
}

s32 tsdrv_sync_notify_proc(u32 devid, struct tsdrv_msg_info *msg, u32 *ack_len)
{
    struct tsdrv_msg_resource_id *dev_msg_resource_id = NULL;
    u32 cur_notify_num;
    u32 tsid;
    int ret;

    tsid = msg->header.tsid;
    if (msg->header.valid != TSDRV_MSG_SYNC_MAGIC) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("devid %u invalid message from opposite.\n", devid);
        return -EINVAL;
#endif
    }

    dev_msg_resource_id = (struct tsdrv_msg_resource_id *)msg->payload;
    dev_msg_resource_id->ret_id_num = 0;
    if ((dev_msg_resource_id->id_type != TSDRV_NOTIFY_ID) || (dev_msg_resource_id->req_id_num == 0)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("devid %u invalid request from opposite, id_type = 0x%x, req_id_num = %u.\n",
            devid, (u32)dev_msg_resource_id->id_type, dev_msg_resource_id->req_id_num);
        return -EINVAL;
#endif
    }
    ret = tsdrv_get_ts_id_cur_num(devid, tsid, TSDRV_NOTIFY_ID, &cur_notify_num);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("get cur_notify_num failed, ret(%d).\n", ret);
        return -EINVAL;
#endif
    }

    ret = devdrv_alloc_notify_msg_chan(devid, tsid, dev_msg_resource_id);
    msg->header.result = ret;
    msg->header.valid = TSDRV_MSG_SYNC_RSP_MAGIC;
    *ack_len = sizeof(struct tsdrv_msg_info);
    if (cur_notify_num < dev_msg_resource_id->ret_id_num) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("devid %u cur_notify_num = %u, ret_id_num = %u.\n", devid, cur_notify_num,
            dev_msg_resource_id->ret_id_num);
        return -EINVAL;
#endif
    }
    ret = tsdrv_dec_ts_id_cur_num(devid, tsid, TSDRV_NOTIFY_ID, dev_msg_resource_id->ret_id_num);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("dec stream_id_num failed, ret(%d).\n", ret);
        return -EINVAL;
#endif
    }

    return 0;
}

s32 tsdrv_sync_stream_sqcq(u32 devid, struct tsdrv_msg_info *msg_head, u32 *ack_len)
{
    struct tsdrv_msg_sync_stream_sqcq *msg = (struct tsdrv_msg_sync_stream_sqcq *)msg_head->payload;
    struct tsdrv_ts_resource *ts_resource = NULL;
    u32 tsid;

    tsid = msg_head->header.tsid;
    if (msg_head->header.valid != TSDRV_MSG_SYNC_MAGIC) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("devid %u invalid message from opposite.\n", devid);
        return -EINVAL;
#endif
    }

    if ((msg->stream_id >= tsdrv_get_stream_id_max_num(devid, tsid)) || (tsid >= DEVDRV_MAX_TS_NUM) ||
        (msg->sq_id >= DEVDRV_MAX_SQ_NUM)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("devid %u stream_id %u sqid %u cqid %u invalid\n",
            devid, msg->stream_id, msg->sq_id, msg->cq_id);
        return -EINVAL;
#endif
    }

    msg_head->header.result = 0;
    msg_head->header.valid = TSDRV_MSG_SYNC_RSP_MAGIC;
    *ack_len = sizeof(struct tsdrv_msg_info);

    ts_resource = tsdrv_get_ts_resoruce(devid, TSDRV_PM_FID, tsid);

    ts_resource->stream_info[msg->stream_id].sq = msg->sq_id;
    ts_resource->stream_info[msg->stream_id].valid = msg->valid;
    ts_resource->stream_info[msg->stream_id].remote_tgid = msg->remote_tgid;
    return 0;
}

s32 tsdrv_sync_phy_cbcqsq_exit(u32 devid, struct tsdrv_msg_info *msg, u32 *ack_len)
{
    if (msg->header.valid != TSDRV_MSG_SYNC_MAGIC || msg->header.tsid >= DEVDRV_MAX_TS_NUM) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("devid %u invalid message from opposite, valid = 0x%x tsid = %u.\n",
            devid, (u32)msg->header.valid, msg->header.tsid);
        return -EFAULT;
#endif
    }

    if (tsdrv_get_env_type() != TSDRV_ENV_OFFLINE) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("d2h no support destroy phy cbcqsq, devid %u.\n", devid);
        return -EINVAL;
#endif
    }

    callback_physic_sqcq_exit(tsdrv_get_dev(devid), TSDRV_PM_FID, msg->header.tsid);

    msg->header.result = 0;
    msg->header.valid = TSDRV_MSG_SYNC_RSP_MAGIC;
    *ack_len = sizeof(struct tsdrv_msg_info);
    return 0;
}

#ifdef CFG_FEATURE_CDQM
s32 tsdrv_msg_sync_cdqm_init(u32 devid, u32 tsid, int irq)
{
    struct tsdrv_msg_info msg_head = { 0 };
    struct tsdrv_cdqm_init_msg *irq_msg = (struct tsdrv_cdqm_init_msg *)msg_head.payload;
    int ret;

    msg_head.header.msg_id = TSDRV_CHAN_SYNC_CDQM_INIT;
    msg_head.header.valid = TSDRV_MSG_SYNC_MAGIC;
    msg_head.header.result = TSDRV_MSG_INVALID_RESULT;
    msg_head.header.tsid = tsid;

    irq_msg->f2nf_irq = (u32)irq;

    ret = tsdrv_sync_msg_send(devid, &msg_head);
    if ((ret != 0) || (msg_head.header.result != 0) || (msg_head.header.valid != TSDRV_MSG_SYNC_RSP_MAGIC)) {
#ifndef TSDRV_UT
        devdrv_drv_err("device(%u) msg chan send failed, ret = %d, result = %u, valid = 0x%x.\n",
            devid, ret, (u32)msg_head.header.result, (u32)msg_head.header.valid);
        return -EFAULT;
#endif
    }
    return 0;
}


s32 tsdrv_msg_sync_cdqm_create(u32 devid, u32 tsid, struct cdq_cfg_info *para, u32 *cdq_id)
{
    struct tsdrv_msg_info msg_head;
    struct cdq_cfg_info *msg = (struct cdq_cfg_info *)msg_head.payload;
    int ret;

    msg_head.header.msg_id = TSDRV_CHAN_SYNC_CDQM_CREATE;
    msg_head.header.valid = TSDRV_MSG_SYNC_MAGIC;
    msg_head.header.result = TSDRV_MSG_INVALID_RESULT;
    msg_head.header.tsid = tsid;

    *msg = *para;

    ret = tsdrv_sync_msg_send(devid, &msg_head);
    if ((ret != 0) || (msg_head.header.result != 0) || (msg_head.header.valid != TSDRV_MSG_SYNC_RSP_MAGIC)) {
#ifndef TSDRV_UT
        devdrv_drv_err("device(%u) msg chan send failed, ret = %d, result = %u, valid = 0x%x.\n",
            devid, ret, (u32)msg_head.header.result, (u32)msg_head.header.valid);
        return -EFAULT;
#endif
    }

    *cdq_id = msg->cdq_id;

    return 0;
}

s32 tsdrv_msg_sync_cdqm_destroy(u32 devid, u32 tsid, struct cdqm_destroy_msg * para)
{
    int ret;
    struct tsdrv_msg_info msg_head;
    struct cdqm_destroy_msg *msg = (struct cdqm_destroy_msg *)msg_head.payload;

    msg_head.header.msg_id = TSDRV_CHAN_SYNC_CDQM_DESTROY;
    msg_head.header.valid = TSDRV_MSG_SYNC_MAGIC;
    msg_head.header.result = TSDRV_MSG_INVALID_RESULT;
    msg_head.header.tsid = tsid;

    msg->cdq_id = para->cdq_id;
    msg->pid = para->pid;

    ret = tsdrv_sync_msg_send(devid, &msg_head);
    if ((ret != 0) || (msg_head.header.result != 0) || (msg_head.header.valid != TSDRV_MSG_SYNC_RSP_MAGIC)) {
#ifndef TSDRV_UT
        devdrv_drv_err("device(%u) msg chan send failed, ret = %d, result = %u, valid = 0x%x.\n",
            devid, ret, (u32)msg_head.header.result, (u32)msg_head.header.valid);
        return -EFAULT;
#endif
    }

    return 0;
}

s32 tsdrv_msg_sync_cdqm_batch_abnormal(u32 devid, u32 tsid, u32 cdq_id)
{
    int ret;
    struct tsdrv_msg_info msg_head;
    struct cdqm_batch_abnormal_msg *msg = (struct cdqm_batch_abnormal_msg *)msg_head.payload;

    msg_head.header.msg_id = TSDRV_CHAN_SYNC_CDQM_BATCH_ABNORMAL;
    msg_head.header.valid = TSDRV_MSG_SYNC_MAGIC;
    msg_head.header.result = TSDRV_MSG_INVALID_RESULT;
    msg_head.header.tsid = tsid;

    msg->cdq_id = cdq_id;
    msg->devid = devid;

    ret = tsdrv_sync_msg_send(devid, &msg_head);
    if ((ret != 0) || (msg_head.header.result != 0) || (msg_head.header.valid != TSDRV_MSG_SYNC_RSP_MAGIC)) {
#ifndef TSDRV_UT
        devdrv_drv_err("device(%u) msg chan send failed, ret = %d, result = %u, valid = 0x%x.\n",
            devid, ret, (u32)msg_head.header.result, (u32)msg_head.header.valid);
        return -EFAULT;
#endif
    }

    return 0;
}

#endif

#ifdef CFG_FEATURE_ID_MAPING
s32 tsdrv_sync_id_map_proc(u32 devid, struct tsdrv_msg_info *msg, u32 *ack_len)
{
    struct tsdrv_id_map_sync_info *id_map = NULL;
    int ret;

    if (msg->header.valid != TSDRV_MSG_SYNC_MAGIC) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("devid %u invalid message from opposite.\n", devid);
        return -EINVAL;
#endif
    }

    id_map = (struct tsdrv_id_map_sync_info *)msg->payload;
    if ((id_map->id_type == TSDRV_EVENT_SW_ID)) {
        ret = tsdrv_event_id_maping(devid, &id_map->event);
    } else {
        TSDRV_PRINT_ERR("devid %u invalid request from opposite, id_type = 0x%x\n",
            devid, (u32)id_map->id_type);
        ret = -EINVAL;
    }
    msg->header.result = ret;
    msg->header.valid = TSDRV_MSG_SYNC_RSP_MAGIC;
    *ack_len = sizeof(struct tsdrv_msg_info);
    return 0;
}
#endif
s32 (*const tsdrv_chan_msg_handlers[TSDRV_CHAN_SYNC_MAX])(u32 devid, struct tsdrv_msg_info *msg, u32 *ack_len) = {
    [TSDRV_CHAN_SYNC_SSID] = tsdrv_sync_ssid_proc,
    [TSDRV_CHAN_SYNC_RES_ID] = tsdrv_sync_res_id_proc,
    [TSDRV_CHAN_SYNC_STREAM] = tsdrv_sync_stream_proc,
    [TSDRV_CHAN_SYNC_SQ] = tsdrv_sync_sq_proc,
    [TSDRV_CHAN_SYNC_CQ] = tsdrv_sync_cq_proc,
    [TSDRV_CHAN_SYNC_EVENT] = tsdrv_sync_event_proc,
    [TSDRV_CHAN_SYNC_MODEL] = tsdrv_sync_model_proc,
#ifdef CFG_FEATURE_CMO
    [TSDRV_CHAN_SYNC_CMO] = tsdrv_sync_cmo_proc,
#endif
    [TSDRV_CHAN_SYNC_NOTIFY] = tsdrv_sync_notify_proc,
    [TSDRV_CHAN_SYNC_IPC_EVENT] = tsdrv_sync_ipc_event_proc,
    [TSDRV_CHAN_SYNC_STREAM_SQ_CQ] = tsdrv_sync_stream_sqcq,
#ifdef CFG_FEATURE_CDQM
    [TSDRV_CHAN_SYNC_CDQM_INIT] = tsdrv_sync_cdqm_init_proc,
    [TSDRV_CHAN_SYNC_CDQM_CREATE] = tsdrv_sync_cdqm_create_proc,
    [TSDRV_CHAN_SYNC_CDQM_DESTROY] = tsdrv_sync_cdqm_destroy_proc,
    [TSDRV_CHAN_SYNC_CDQM_BATCH_ABNORMAL] = tsdrv_sync_cdqm_batch_abnormal_proc,
#endif
    [TSDRV_CHAN_SYNC_PHY_CBCQSQ_EXIT] = tsdrv_sync_phy_cbcqsq_exit,
#ifdef CFG_FEATURE_ID_MAPING
    [TSDRV_CHAN_SYNC_ID_MAP] = tsdrv_sync_id_map_proc,
#endif
};

s32 tsdrv_chan_msg_proc(u32 devid, void *msg, u32 msg_len, u32 *ack_len)
{
    struct tsdrv_msg_info *tsdrv_msg_info = NULL;
    u32 msg_id;

    if ((msg == NULL) || (ack_len == NULL) || (msg_len != sizeof(struct tsdrv_msg_info))) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("msg(%pK) or real_out_len(%pK) is NULL, msg_len(%u)\n", msg, ack_len, msg_len);
        return -EINVAL;
#endif
    }

    tsdrv_msg_info = (struct tsdrv_msg_info *)msg;
    msg_id = tsdrv_msg_info->header.msg_id;

    if (msg_id >= TSDRV_CHAN_SYNC_MAX) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("invalid msg_id=%u, devid=%u\n", msg_id, devid);
        return -EINVAL;
#endif
    }
    if (tsdrv_chan_msg_handlers[msg_id] == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("msg_id=%u handler is NULL, devid=%u\n", msg_id, devid);
        return -EFAULT;
#endif
    }
    return tsdrv_chan_msg_handlers[msg_id](devid, tsdrv_msg_info, ack_len);
}
EXPORT_SYMBOL(tsdrv_chan_msg_proc);

void tsdrv_set_send_msg_proc(int (*sync_msg_send)(u32 devid, void *tx, size_t tx_size))
{
    tsdrv_send_sync_msg = sync_msg_send;
}
EXPORT_SYMBOL(tsdrv_set_send_msg_proc);
