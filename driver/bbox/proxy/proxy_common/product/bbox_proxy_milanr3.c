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

#ifdef BBOX_SOC_PLATFORM_MDC_V11
#include "bbox_proxy_config_mdc_v11.h"
#else
#include "bbox_proxy_config_milanr3.h"
#endif

#include "../bbox_proxy_common.h"
#include "common/bbox_common.h"

DEFINE_BBOX_PROXY(g_module_proxy);

s32 bbox_product_proxy_init(struct bbox_proxy_mgr *proxy)
{
    if (proxy != NULL) {
        s32 count = (s32)(sizeof(g_module_proxy) / sizeof(g_module_proxy[0]));
        count = BBOX_MIN(count, BBOX_CORE_MAX);
        for (proxy->count = 0; proxy->count < count; proxy->count++) {
            proxy->mgr[proxy->count].info = &g_module_proxy[proxy->count];
        }
        return BBOX_SUCCESS;
    }
    return BBOX_FAILURE;
}

void bbox_product_proxy_exit(void)
{
    return;
}

