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
 * Create: 2022-06-27
 */
#include <linux/module.h>

#include "cfg_persistent_module.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("DAVINCI driver");

int __init cfgpt_init(void)
{
    cfgpt_info("Module init success.\n");
    return 0;
}

void __exit cfgpt_exit(void)
{
    cfgpt_info("Module exit success.\n");
    return;
}

module_init(cfgpt_init);
module_exit(cfgpt_exit);

