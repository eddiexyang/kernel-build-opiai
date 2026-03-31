/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description:define acodec init source file
 * Author: Hisilicon multimedia software group
 * Create: 2022/9/10
 */

#include "ot_acodec_mod_init.h"

static __init int acodec_mod_init(void)
{
    return acodec_init();
}

static __exit void acodec_mod_exit(void)
{
    acodec_exit();
}

module_init(acodec_mod_init);
module_exit(acodec_mod_exit);

MODULE_LICENSE("Proprietary");

