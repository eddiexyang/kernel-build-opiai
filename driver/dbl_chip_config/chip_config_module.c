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
 * Create: 2022-05-24
 */
#include <linux/module.h>
#include <linux/types.h>
#include <linux/proc_fs.h>

#include "kernel_version_adapt.h"

#include "dbl/chip_config.h"
#include "numa_id.h"
#include "chip_config_module.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("DAVINCI driver");

struct proc_dir_entry *ccfg_fs_entry = NULL;

void ccfg_fs_init(void)
{
#ifdef CONFIG_PROC_FS
    ccfg_fs_entry = proc_mkdir("ccfg", NULL);
    if (ccfg_fs_entry == NULL) {
        ccfg_err("Create ccfg top entry dir failed.\n");
        return;
    }
#endif
    return;
}

void ccfg_fs_uninit(void)
{
#ifdef CONFIG_PROC_FS
    remove_proc_subtree("ccfg", NULL);
#endif
}

int __init ccfg_init(void)
{
    ccfg_fs_init();
    numa_id_init(ccfg_fs_entry);
    ccfg_info("Module init success.\n");
    return 0;
}

void __exit ccfg_exit(void)
{
    numa_id_uninit(ccfg_fs_entry);
    ccfg_fs_uninit();
    ccfg_info("Module exit success.\n");
    return;
}

module_init(ccfg_init);
module_exit(ccfg_exit);

