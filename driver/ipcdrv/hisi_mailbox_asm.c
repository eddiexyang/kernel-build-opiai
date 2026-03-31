/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
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

#include "hisi_mailbox_asm.h"

void hisi_mdev_asm_sev(void)
{
#ifndef UT_TEST
    asm volatile ("sev");
#endif
}

void hisi_mdev_asm_wfe(void)
{
#ifndef UT_TEST
    asm volatile ("wfe");
#endif
}
