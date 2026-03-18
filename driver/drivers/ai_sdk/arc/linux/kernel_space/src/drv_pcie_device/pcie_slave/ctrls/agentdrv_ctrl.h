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
#ifndef _AGENTDRV_CTRL_H_
#define _AGENTDRV_CTRL_H_

#include <linux/pci.h>

#include "agentdrv_unit.h"

void agentdrv_atu_proc(struct agentdrv_devctrl *agent_dev);
int agentdrv_dev_register(struct agentdrv_devctrl *agent_dev, int func_id);
void agentdrv_dev_unregister(struct agentdrv_devctrl *agent_dev, int func_id);
void agentdrv_dev_flr_uninstance(struct agentdrv_devctrl *agent_dev, int func_id);
void devdrv_init_dev_num(const struct pci_device_id agentdrv_tbl[], int id_num);
void agentdrv_res_set_slot_num(void);
int agentdrv_ctrl_init(void);
int agentdrv_get_rx_atu(struct agentdrv_msg_dev *msg_dev, void *data);
int devdrv_get_dev_num(void);
void agentdrv_msg_release(struct agentdrv_devctrl *agent_dev, u32 func_id);

#endif
