/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Description:
 * Author: huawei
 * Create: 2023-2-17
 */
#include "tsdrv_kernel_common.h"
#include "tsdrv_log.h"

#include "tsdrv_notify.h"

int tsdrv_hwts_init(u32 devid)
{
    return 0;
}

void tsdrv_hwts_uninit(u32 devid)
{
}

#ifndef TSDRV_UT
int devdrv_write_notify_register(u32 devid, u32 notify_id)
{
    return 0;
}

#ifdef CFG_SOC_PLATFORM_MINIV2
#define CHIP_BASEADDR_PA_OFFSET             (0x8000000000ULL)
#elif defined (CFG_SOC_PLATFORM_CLOUD_V2)
#define CHIP_BASEADDR_PA_OFFSET             (0x80000000000ULL)
#else
#define CHIP_BASEADDR_PA_OFFSET             (0x200000000000ULL)
#endif

#if defined(CFG_SOC_PLATFORM_MINIV2)
#define MAX_NODE_NUM 2
#else
#define MAX_NODE_NUM 4
#endif

#define CHIP_NOTIFY_ADDR_OFFSET 0xE000UL
#define CHIP_HWTS_ADDR_OFFSET 0xAF100000ULL
int devdrv_get_notify_base_addr(u32 devid, u64 *phy_addr)
{
    if ((phy_addr == NULL) || (devid >= TSDRV_MAX_DAVINCI_NUM)) {
        TSDRV_PRINT_ERR("notify base: invalid null pointer.devid = %u\n", devid);
        return -EINVAL;
    }
    *phy_addr = (CHIP_BASEADDR_PA_OFFSET * (devid % MAX_NODE_NUM)) + CHIP_HWTS_ADDR_OFFSET +
        CHIP_NOTIFY_ADDR_OFFSET;

    return 0;
}
EXPORT_SYMBOL(devdrv_get_notify_base_addr);
#endif

