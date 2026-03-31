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

#include "cdr_bootcheck.h"
#include <linux/securec.h>
#include "bootparam/bbox_boot_param.h"
#include "common/bbox_common.h"
#include "common/bbox_print.h"
#include "communication/bbox_message.h"
#include "config/bbox_config.h"
#include "rdr/rdr_common.h"

/*
 * @brief       : send message success, callback function
 * @param [in]  : void *arg         device id
 * @return      : NA
 */
STATIC void cdr_bootcheck_notify(void *arg)
{
    s32 i, ret;
    u32 devid;
    u64 addr = 0;
    u64 size = 0;
    char *data = NULL;
    struct cdr_head *head = NULL;

    BB_CHECK_PTR(arg, return, "invalid parameter. arg is NULL.\n");
    devid = *(u32 *)arg;

    ret = bbox_config_get_cdr(&addr, &size);
    if ((ret != BBOX_SUCCESS) || (addr == 0) || (size == 0)) {
        BB_PRINT_ERR("get chip data addr failed.\n");
        return;
    }

    addr = bbox_get_device_addr(devid, addr);
    data = (char *)bbox_ioremap(addr, size);
    if (data == NULL) {
        BB_PRINT_ERR("map chip data addr failed.\n");
        return;
    }

    head = (struct cdr_head *)data;
    if ((head->magic != CHIP_DATA_RECORD_MAGIC) ||
        (head->version != CHIP_DATA_RECORD_VERSION)) {
        BB_PRINT_INFO("chip data is unusable.\n");
        bbox_iounmap((void *)data);
        return;
    }

    for (i = 0; i < CDR_AREA_DATA_NUM; i++) {
        head->area[i].flag = (u8)CDR_FLAG_READ_CLEAN;
    }
    bbox_iounmap((void *)data);
}

STATIC bool cdr_bootcheck_check_area(const struct cdr_head *head)
{
    s32 i;

    BB_CHECK_PTR(head, return false, "invalid param, head is NULL.\n");

    for (i = 0; i < CDR_AREA_DATA_NUM; i++) {
        if (head->area[i].flag == (u8)CDR_FLAG_WRITE_DATA) {
            return true;
        }
    }

    BB_PRINT_INFO("Area flag is clean.\n");
    return false;
}

/*
 * @brief       : chip dfx data record process
 * @param [in]  : struct bbox_time *start       start time
 * @return      : !=0 failure; ==0 success
 */
s32 cdr_bootcheck_process(const struct bbox_time *start)
{
    s32 i, ret;
    s32 num = (s32)bbox_get_device_num();
    u64 addr = 0;
    u64 size = 0;
    const char *data = NULL;
    const struct cdr_head *head = NULL;
    struct bbox_time tm;

    BB_CHECK_PTR(start, return BBOX_FAILURE, "invalid param, start is NULL.\n");

    ret = bbox_config_get_cdr(&addr, &size);
    BB_CHECK_EXP_CTRL(BB_PRINT_ERR, ret != BBOX_SUCCESS, return BBOX_FAILURE, "get chip data addr failed.\n");
    if ((addr == 0) || (size == 0)) {
        BB_PRINT_INFO("bbox chip bootcheck is closed.\n");
        return BBOX_SUCCESS;
    }

    tm.tv_sec = start->tv_sec;
    tm.tv_nsec = start->tv_nsec;
    bbox_update_time_seq(&tm);
    for (i = 0; i < num; i++) {
        data = (const char *)bbox_ioremap(bbox_get_device_addr((u32)i, addr), size);
        if (data == NULL) {
            BB_PRINT_ERR("[device-%d] map chip data addr failed.\n", i);
            continue;
        }

        head = (const struct cdr_head *)data;
        if ((head->magic != CHIP_DATA_RECORD_MAGIC) ||
            (head->version != CHIP_DATA_RECORD_VERSION)) {
            BB_PRINT_INFO("[device-%d] chip data is unusable.\n", i);
            bbox_iounmap(data);
            data = NULL;
            continue;
        }

        if (cdr_bootcheck_check_area(head) == true) {
            ret = bbox_bootcheck_send_cdr_data((u32)i, cdr_bootcheck_notify, &tm, data, (u32)size);
            if (ret != BBOX_SUCCESS) {
                BB_PRINT_ERR("[device-%d] send chip data failed.\n", i);
                bbox_iounmap(data);
                data = NULL;
                continue;
            }
            BB_PRINT_INFO("[device-%d] process chip data success.\n", i);
        }
        bbox_iounmap(data);
        data = NULL;
    }

    return BBOX_SUCCESS;
}

