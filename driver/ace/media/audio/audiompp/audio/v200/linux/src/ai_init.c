/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description:define ai init source file
 * Author: Hisilicon multimedia software group
 * Create: 2019/12/25
 */

#include "ot_ai_mod_init.h"

static int __init ai_mod_init(void)
{
    (td_void)ai_module_init();
    return 0;
}

static void __exit ai_mod_exit(void)
{
    ai_module_exit();
}

module_init(ai_mod_init);
module_exit(ai_mod_exit);

MODULE_LICENSE("Proprietary");

