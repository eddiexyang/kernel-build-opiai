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


#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/radix-tree.h>
#include <linux/time.h>
#include <linux/uaccess.h>
#include <linux/spinlock.h>

#include "devdrv_manager_time.h"
#include "devdrv_manager_common.h"
#include "devdrv_user_common.h"

/* *
 * get cpu tick.
 * @return cpu tick
 */
u64 devdrv_manager_get_cpu_tick(void)
{
    u64 cnt = 0;
    isb();
    READ_SYSTEM_COUNTER_KER(cnt);
    return cnt;
}
