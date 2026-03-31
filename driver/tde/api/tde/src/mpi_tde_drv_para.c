/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: convert para from user to drv
 * Author: Hisilicon multimedia software group
 * Create: 2018/10/30
 */
#include <string.h>
#include <stdint.h>
#include "securec.h"
#include "mpi_tde_drv_para.h"

td_void mpi_convert_surface_user_to_drv(const mid_tde_surface *user_surface, drv_tde_surface *drv_surface)
{
    if ((user_surface != TD_NULL) && (drv_surface != TD_NULL)) {
        drv_surface->is_ycbcr_clut = (td_bool)user_surface->is_ycbcr_clut;
        drv_surface->color_format = (drv_tde_color_fmt)user_surface->color_format;
        drv_surface->clut_phys_addr = (td_phys_addr_t)user_surface->clut_phys_addr;
        drv_surface->cbcr_phys_addr = (td_phys_addr_t)user_surface->cbcr_phys_addr;
        drv_surface->cbcr_stride = user_surface->cbcr_stride;
        drv_surface->height = user_surface->height;
        drv_surface->phys_addr = (td_phys_addr_t)user_surface->phys_addr;
        drv_surface->stride = user_surface->stride;
        drv_surface->width = user_surface->width;
        drv_surface->alpha0 = user_surface->alpha0;
        drv_surface->alpha1 = user_surface->alpha1;
        drv_surface->alpha_max_is_255 = (td_bool)user_surface->alpha_max_is_255;
        drv_surface->support_alpha_ex_1555 = (td_bool)user_surface->support_alpha_ex_1555;
    } else {
        tde_mpi_warning("mpi_convert_surface_user_to_drv do nothing!\n");
    }
    return;
}

td_void mpi_convert_rect_user_to_drv(const mid_tde_rect *user_rect, drv_tde_rect *drv_rect)
{
    if ((user_rect != TD_NULL) && (drv_rect != TD_NULL)) {
        if (memcpy_s(drv_rect, sizeof(drv_tde_rect), user_rect, sizeof(mid_tde_rect)) != EOK) {
            tde_mpi_error("mpi_convert_rect_user_to_drv failed!\n");
        }
    } else {
        tde_mpi_warning("mpi_convert_rect_user_to_drv do nothing!\n");
    }
    return;
}

td_void mpi_convert_pattern_fill_option_user_to_drv(const mid_tde_pattern_fill_opt *user_opt,
                                                    drv_tde_pattern_fill_opt *drv_opt)
{
    if ((user_opt != TD_NULL) && (drv_opt != TD_NULL)) {
        if (memcpy_s(drv_opt, sizeof(drv_tde_pattern_fill_opt), user_opt, sizeof(mid_tde_pattern_fill_opt)) != EOK) {
            tde_mpi_error("mpi_convert_pattern_fill_option_user_to_drv failed!\n");
        }
    } else {
        tde_mpi_warning("mpi_convert_pattern_fill_option_user_to_drv do nothing!\n");
    }
    return;
}