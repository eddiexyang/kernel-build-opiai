/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
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

#include "agentdrv_mdc_p2p.h"
#include "agentdrv_unit.h"
#include "agentdrv_msg.h"
#include "devdrv_util.h"

void agentdrv_mdc_sq_doorbell_interrupt(int dev_id, int db_id)
{
    int id = db_id - AGENTDRV_GENERAL_INTR_USED_DB_START;
    u32 offset;
    struct agentdrv_devctrl *agent_dev = agentdrv_get_dev(dev_id);
    if (agent_dev == NULL) {
        devdrv_err("Function agentdrv_get_dev failed, agent_dev is null.\n");
        return;
    }
    offset = id * AGENTDRV_QUEUE_CNT;
    if (offset >= AGENTDRV_GENERAL_INTERRUPT_NUM) {
        devdrv_err("Variable offset if invalid. (dev_id=%d)\n", dev_id);
        return;
    }
    agent_dev->interrupt_info[offset].status = AGENTDRV_ENABLE;
    wake_up_interruptible(&agent_dev->interrupt_info[offset].wait);
}

void agentdrv_mdc_cq_doorbell_interrupt(int dev_id, int db_id)
{
    int id = db_id - AGENTDRV_GENERAL_INTR_USED_DB_START;
    u32 offset;
    struct agentdrv_devctrl *agent_dev = agentdrv_get_dev(dev_id);
    if (agent_dev == NULL) {
        devdrv_err("Function agentdrv_get_dev failed, agent_dev is null.\n");
        return;
    }
    offset = id * AGENTDRV_QUEUE_CNT + 1;
    if (offset >= AGENTDRV_GENERAL_INTERRUPT_NUM) {
        devdrv_err("Variable offset if invalid. (dev_id=%d)\n", dev_id);
        return;
    }
    agent_dev->interrupt_info[offset].status = AGENTDRV_ENABLE;
    wake_up_interruptible(&agent_dev->interrupt_info[offset].wait);
}

int agentdrv_get_general_interrupt_db_info(struct agentdrv_msg_dev *msg_dev, void *data)
{
    struct devdrv_admin_msg_reply *reply = (struct devdrv_admin_msg_reply *)data;
    struct devdrv_general_interrupt_db_info *db_info = NULL;

    reply->len = sizeof(struct devdrv_admin_msg_reply) + sizeof(struct devdrv_general_interrupt_db_info);

    db_info = (struct devdrv_general_interrupt_db_info *)reply->data;
    db_info->db_num = AGENTDRV_GENERAL_INTR_USED_DB_NUM * AGENTDRV_QUEUE_CNT;
    db_info->db_start = AGENTDRV_GENERAL_INTR_USED_DB_START * AGENTDRV_QUEUE_CNT;

    return 0;
}
