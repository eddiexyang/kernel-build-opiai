/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
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
 * Create: 2020-4-1
 */

#include <linux/module.h>
#include "pngd_ext.h"

int __init pngd_mod_init(void)
{
    pngd_module_init();
    g_is_depend_sys = 1;
    return 0;
}

void __exit pngd_mod_exit(void)
{
    pngd_module_exit();
    g_is_depend_sys = 0;
}

module_init(pngd_mod_init);
module_exit(pngd_mod_exit);

MODULE_LICENSE("Proprietary");
