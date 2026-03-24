/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2018-2023. All rights reserved.
 * Description: convert para from user to drv
 * Author: Hisilicon multimedia software group
 * Create: 2018/10/30
 */
#ifndef MPI_TDE_DRV_PARA_H
#define MPI_TDE_DRV_PARA_H

#include "mid_base_tde.h"
#include "mpi_defines_tde.h"
#include "ot_drv_tde.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/* user type to drv type */
td_void mpi_convert_surface_user_to_drv(const mid_tde_surface *user_surface, drv_tde_surface *drv_surface);
td_void mpi_convert_rect_user_to_drv(const mid_tde_rect *user_rect, drv_tde_rect *drv_rect);
td_void mpi_convert_pattern_fill_option_user_to_drv(const mid_tde_pattern_fill_opt *user_opt,
                                                    drv_tde_pattern_fill_opt *drv_opt);
#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* MPI_TDE_DRV_PARA_H */
