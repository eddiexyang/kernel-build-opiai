/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2022. All rights reserved.
 * Description:define adec init source file
 * Author: Hisilicon multimedia software group
 * Create: 2019/12/25
 */

#include "ot_adec_mod_init.h"

static int __init adec_mod_init(void)
{
    (td_void)adec_module_init();
    return 0;
}

static void __exit adec_mod_exit(void)
{
    adec_module_exit();
}

module_init(adec_mod_init);
module_exit(adec_mod_exit);

MODULE_LICENSE("Proprietary");

