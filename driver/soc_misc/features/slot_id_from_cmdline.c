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

#include <linux/device.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <asm/uaccess.h>

#include "soc_misc_board_info.h"
#include "soc_misc_init.h"
#include "devdrv_common.h"
#include "davinci_api.h"
#include "dms_define.h"

int soc_misc_init_slot_id_from_cmdline(struct soc_misc_info_st *soc_info)
{
    struct file *fp = NULL;
    char *ptr = NULL;
    char *buffer = NULL;
    ssize_t read_num;
    loff_t pos = 0;
    int ret = -EINVAL;

    buffer = (char *)kzalloc(SLOT_ID_BUFFER_KERNEL_LEN * sizeof(char), GFP_KERNEL | __GFP_ACCOUNT);
    if (buffer == NULL) {
        soc_misc_drv_err("Kzalloc failed!\n");
        return -ENOMEM;
    }

    fp = filp_open(CMDLINE_LOCATION, O_RDONLY, 0);
    if (IS_ERR(fp)) {
        soc_misc_drv_err("Open file failed. (file=%s)\n", CMDLINE_LOCATION);
        goto FREE_BUFF;
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 16, 0)
    read_num = kernel_read(fp, buffer, SLOT_ID_BUFFER_KERNEL_LEN, &pos);
#else
    mm_segment_t old_fs;
    old_fs = get_fs();
    set_fs((mm_segment_t)KERNEL_DS);
    read_num = vfs_read(fp, buffer, SLOT_ID_BUFFER_KERNEL_LEN, &pos);
    set_fs(old_fs);
#endif
    if (read_num <= 0) {
        soc_misc_drv_err("read file failed. (file=%s)\n", CMDLINE_LOCATION);
        goto CLOSE_FP;
    }

    buffer[SLOT_ID_BUFFER_KERNEL_LEN - 1] = '\0';
    ptr = strstr(buffer, "slotid=");
    if (ptr == NULL) {
        soc_misc_drv_err("slotid is not found in cmdline\n");
        goto CLOSE_FP;
    }

    ret = 0;
    ptr += SLOT_ID_STRLEN;
    soc_info->board_info.slot_id = (u16)simple_strtoul(ptr, NULL, 0);

CLOSE_FP:
    filp_close(fp, NULL);
    fp = NULL;
FREE_BUFF:
    kfree(buffer);
    buffer = NULL;
    return ret;
}
