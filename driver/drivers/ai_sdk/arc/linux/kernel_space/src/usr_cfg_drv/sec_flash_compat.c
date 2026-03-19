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

#include <linux/errno.h>

#include "dev_user_cfg.h"

int sec_flash_read(unsigned int dev_id, unsigned int flash_offset,
    unsigned char *buf, unsigned int buf_len)
{
    DEV_USER_CFG_WARN("sec flash read is unavailable on 6.18 port. (device_id=%u; offset=0x%x; len=%u)\n",
        dev_id, flash_offset, buf_len);
    return -EOPNOTSUPP;
}
EXPORT_SYMBOL(sec_flash_read);

int sec_flash_write(unsigned int dev_id, unsigned int flash_offset,
    const unsigned char *buf, unsigned int buf_len)
{
    DEV_USER_CFG_WARN("sec flash write is unavailable on 6.18 port. (device_id=%u; offset=0x%x; len=%u)\n",
        dev_id, flash_offset, buf_len);
    return -EOPNOTSUPP;
}
EXPORT_SYMBOL(sec_flash_write);
