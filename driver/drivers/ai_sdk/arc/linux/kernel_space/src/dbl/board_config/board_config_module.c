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
 * Create: 2022-05-24
 */

#include <linux/module.h>
#include "board_serdes_info.h"
#include "board_config_module.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("DAVINCI driver");

int __init bdcfg_init(void)
{
    int rec = 0;
    rec = bdcfg_serdes_init();
    if (rec != 0) {
        bdcfg_err("Module init fail %d.\n", rec);
        return 0;
    }
    bdcfg_info("Module init success.\n");
    return 0;
}

void __exit bdcfg_exit(void)
{
    bdcfg_info("Module exit success.\n");
    return;
}
module_init(bdcfg_init);
module_exit(bdcfg_exit);
