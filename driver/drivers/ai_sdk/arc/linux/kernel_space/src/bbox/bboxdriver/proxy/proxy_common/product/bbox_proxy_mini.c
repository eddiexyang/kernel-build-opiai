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

#include "../bbox_proxy_common.h"
#include "proxy/proxy_lpm/bbox_proxy_lpm.h"
#include "proxy/proxy_ts/bbox_proxy_ts.h"

s32 bbox_product_proxy_init(struct bbox_proxy_mgr *proxy)
{
    if (proxy == NULL) {
        return BBOX_FAILURE;
    }
    proxy->count = 0;
    (void)bbox_lpm_init();
    (void)bbox_ts_init();
    return BBOX_SUCCESS;
}


void bbox_product_proxy_exit(void)
{
    bbox_ts_exit();
    bbox_lpm_exit();
    return;
}
