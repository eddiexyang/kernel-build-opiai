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
#include "dp_proc_mng_cmd.h"
#include "dp_proc_mng_log.h"
#include "devdrv_interface.h"
#include "dp_proc_mng_channel.h"
#include "dp_proc_mng_agent_msg_client.h"

STATIC int dp_agent_common_msg_process(u32 devid, void *data, u32 in_data_len, u32 out_data_len, u32 *real_out_len)
{
    int ret = 0;

    if ((data == NULL) || (real_out_len == NULL)) {
        dp_proc_mng_drv_err("Data or out_len is NULL. (data=%pK; out_len=%pK)\n", data, real_out_len);
        return -EINVAL;
    }

    if (in_data_len < sizeof(struct dp_proc_mng_chan_msg_head)) {
        dp_proc_mng_drv_err("In_data_len is invalid. (in_data_len=%u)\n", in_data_len);
        return -EMSGSIZE;
    }

    if (devid >= DP_PROC_MNG_MAX_AGENT_DEVICE_NUM) {
        dp_proc_mng_drv_err("Device_id must less than DP_PROC_MNG_MAX_AGENT_DEVICE_NUM. "
            "(devid=%u; DP_PROC_MNG_MAX_AGENT_DEVICE_NUM=%d)\n",
            devid, DP_PROC_MNG_MAX_AGENT_DEVICE_NUM);
        return -ENODEV;
    }

    ret = dp_proc_mng_chan_msg_dispatch(data, in_data_len, out_data_len, real_out_len,
        &dp_proc_mng_agent_msg_processes[0]);

    return ret;
}

STATIC void dp_agent_common_msg_notify(u32 devid)
{
    dp_proc_mng_drv_info("Device common message chan is initing device. (dev=%u)\n", devid);
    return;
}

struct agentdrv_common_msg_client dp_agent_common_msg_client = {
    .type = AGENTDRV_COMMON_MSG_DP_PROC_MNG,
    .common_msg_recv = dp_agent_common_msg_process,
    .init_notify = dp_agent_common_msg_notify,
};


int dp_proc_mng_agent_msg_chan_init_instance(void)
{
#ifndef CFG_FEATURE_RC_MODE
    return agentdrv_register_common_msg_client(&dp_agent_common_msg_client);
#else
    return 0;
#endif
}

int dp_proc_mng_agent_msg_chan_uninit_instance(void)
{
#ifndef CFG_FEATURE_RC_MODE
    return agentdrv_unregister_common_msg_client(&dp_agent_common_msg_client);
#else
    return 0;
#endif
}

int dp_proc_mng_agent_common_msg_send(u32 devid, void *msg, unsigned int len, unsigned int out_len)
{
#ifndef CFG_FEATURE_RC_MODE
    return agentdrv_common_msg_send(devid, msg, len, out_len, &out_len, AGENTDRV_COMMON_MSG_DP_PROC_MNG);
#else
    return -ENOSPC;
#endif
}

