/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description:define vpss init header file
 * Author: Hisilicon multimedia software group
 * Create: 2019/12/30
 */
#ifndef OT_CHNL_MOD_INIT_H
#define OT_CHNL_MOD_INIT_H

#include "hi_comm_audio.h"

#ifdef __cplusplus
extern "C" {
#endif /* end of #ifdef __cplusplus */

int chnl_module_init(void);
void chnl_module_exit(void);
void *chnl_reg_vpu(const void *info);
int chnl_unreg_vpu(void *vpu_hdl);

#ifdef __cplusplus
}
#endif /* end of #ifdef __cplusplus */

#endif /* end of #ifndef OT_CHNL_MOD_INIT_H */
