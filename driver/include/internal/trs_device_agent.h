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
* Create: 2022-10-15
*/

#ifndef TRS_DEVICE_AGENT_H
#define TRS_DEVICE_AGENT_H

#include "trs_msg.h"

struct trs_device_agent_ops {
    int (*trs_agent_get_id)(u32 devid, u32 tsid, struct trs_msg_id_sync *data);
    int (*trs_agent_put_id)(u32 devid, u32 tsid, struct trs_msg_id_sync *data);
    int (*trs_agent_get_cap)(u32 devid, u32 tsid, struct trs_msg_id_cap *id_cap);
    int (*trs_agent_get_phy_addr)(u32 devid, u32 tsid, struct trs_msg_get_phy_addr *info);
    int (*trs_agent_get_cq_group)(u32 devid, u32 tsid, struct trs_msg_cq_group *info);
    int (*trs_agent_get_proc_num)(u32 devid, u32 tsid, struct trs_msg_proc_num *data);
    int (*trs_agent_get_res_avail_num)(u32 devid, u32 tsid, struct trs_msg_res_num *data);
    int (*trs_agent_send_ssid_to_ts)(u32 devid, u32 tsid, int ssid, u32 hpid);
    int (*trs_agent_instance)(u32 devid, u32 tsid);
    void (*trs_agent_uninstance)(u32 devid, u32 tsid);
};

void trs_agent_register_ops(u32 devid, struct trs_device_agent_ops *ops);
void trs_agent_unregister_ops(u32 devid);

#endif

