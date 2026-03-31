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
 * Create: 2022-08-13
 */

#ifndef __TSMNG_PM_H__
#define __TSMNG_PM_H__


#define DEVDRV_COMPUTE_POWER_LENGTH 56

struct computing_power_arg {
    unsigned int dev_id;
    struct tag_computing_power_msg compute_power_msg;
};

/*
 * ipc message
 */
struct ipcdrv_msg_payload {
    u8 result;
    u8 reserved[IPCDRV_MSG_LENGTH - 1];
};

/* call for inside module */
void tsmng_ipc_manager_init(u32 dev_id);
void tsmng_ipc_manager_exit(u32 dev_id);
int tsmng_inform_ts_idle(void *data, u32 dev_id);
#endif  /* __TSMNG_PM_H__ */
