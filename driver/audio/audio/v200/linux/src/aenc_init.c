/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2022. All rights reserved.
 * Description:define aenc init source file
 * Author: Hisilicon multimedia software group
 * Create: 2019/12/25
 */

#include "ot_aenc_mod_init.h"

static int __init aenc_mod_init(void)
{
    (td_void)aenc_module_init();
    return 0;
}

static void __exit aenc_mod_exit(void)
{
    aenc_module_exit();
}

module_init(aenc_mod_init);
module_exit(aenc_mod_exit);

MODULE_LICENSE("Proprietary");

