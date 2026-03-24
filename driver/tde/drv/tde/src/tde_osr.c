/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: tde osr
 * Author: Hisilicon multimedia software group
 * Create: 2018/10/30
 */

#include "tde_osilist.h"
#include "ot_type.h"
#include "tde_ioctl.h"
#include "tde_handle.h"
#include "tde_osilist.h"
#include "ot_type.h"
#include "tde_hal.h"
#include "ot_tde_mod_init.h"

#ifdef CONFIG_USE_SYS_CONFIG
#include "sys_ext.h"
#endif

td_s32 tde_drv_mod_init(td_void)
{
    td_s32 ret = TD_SUCCESS;
    ret = tde_init_module_k();

    return ret;
}

td_void tde_drv_mod_exit(td_void)
{
    tde_cleanup_module_k();
}

#ifndef OT_ADVCA_FUNCTION_RELEASE
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION(DESCRIPTION);
MODULE_VERSION(TDE_VERSION);
#else
MODULE_AUTHOR("");
MODULE_DESCRIPTION("");
MODULE_VERSION("");
#endif

