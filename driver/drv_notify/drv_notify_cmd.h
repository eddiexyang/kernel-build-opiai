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

#ifndef __DRV_NOT_CMD_H__
#define __DRV_NOT_CMD_H__

#define NOTDRV_CMD_NOTIFY_SET 0x27
#define NOTDRV_CMD_NOTIFY_GET 0x28

struct drv_notify_cmd {
    int index;
    int value;
};

#endif
