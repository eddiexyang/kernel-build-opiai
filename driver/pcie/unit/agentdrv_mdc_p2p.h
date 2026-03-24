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
#ifndef _AGENTDRV_MDC_P2P_H_
#define _AGENTDRV_MDC_P2P_H_

#include <linux/wait.h>
#include "agentdrv_msg.h"
#include "nvme_comm_drv.h"

/* 2 for cq and sq */
#define AGENTDRV_GENERAL_INTERRUPT_NUM (AGENTDRV_GENERAL_INTR_USED_DB_NUM * 2)

struct agentdrv_general_interrupt_info {
    u32 id;
    u32 status;
    void *db_addr;
    wait_queue_head_t wait;
};

int agentdrv_get_general_interrupt_db_info(struct agentdrv_msg_dev *msg_dev, void *data);
void agentdrv_mdc_sq_doorbell_interrupt(int dev_id, int db_id);
void agentdrv_mdc_cq_doorbell_interrupt(int dev_id, int db_id);
#endif
