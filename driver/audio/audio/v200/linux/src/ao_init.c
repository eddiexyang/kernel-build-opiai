/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description:define ao init source file
 * Author: Hisilicon multimedia software group
 * Create: 2019/12/25
 */

#include "ot_ao_mod_init.h"
#include "ot_ao_export.h"

EXPORT_SYMBOL(ot_ao_get_export_symbol);

static int __init ao_mod_init(void)
{
    int ret;

    ret = ao_module_init();
    if (ret != 0) {
        return ret;
    }
    return 0;
}

static void __exit ao_mod_exit(void)
{
    ao_module_exit();
}

module_init(ao_mod_init);
module_exit(ao_mod_exit);

MODULE_LICENSE("Proprietary");

