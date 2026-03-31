/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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
* Create: 2019-10-15
*/

#ifndef FEATURES_SOC_MISC_BOARD_INFO_H
#define FEATURES_SOC_MISC_BOARD_INFO_H
#include "soc_misc_info.h"

int soc_misc_init_pcb_id(struct soc_misc_info_st *soc_info);
int soc_misc_init_pcb_id_from_pca6416(struct soc_misc_info_st *soc_info);
int soc_misc_init_pcb_id_not_support_default_5a(struct soc_misc_info_st *soc_info);
int soc_misc_init_pcb_id_not_support_default_ff(struct soc_misc_info_st *soc_info);
int soc_misc_init_pcb_id_not_support_default_01(struct soc_misc_info_st *soc_info);

int soc_misc_init_bom_id(struct soc_misc_info_st *soc_info);
int soc_misc_init_bom_id_from_pca6416(struct soc_misc_info_st *soc_info);
int soc_misc_init_bom_id_not_support_default_5a(struct soc_misc_info_st *soc_info);
int soc_misc_init_bom_id_not_support_default_ff(struct soc_misc_info_st *soc_info);
int soc_misc_init_bom_id_not_support_default_01(struct soc_misc_info_st *soc_info);

#ifdef CONFIG_BOARD_INFO_FROM_PCA6416
#define soc_misc_init_pcb_id soc_misc_init_pcb_id_from_pca6416
#define soc_misc_init_bom_id soc_misc_init_bom_id_from_pca6416
#endif
#ifdef CONFIG_BOARD_INFO_NOT_SUPPORT_DEFAULT_5A
#define soc_misc_init_pcb_id soc_misc_init_pcb_id_not_support_default_5a
#define soc_misc_init_bom_id soc_misc_init_bom_id_not_support_default_5a
#endif
#ifdef CONFIG_BOARD_INFO_NOT_SUPPORT_DEFAULT_FF
#define soc_misc_init_pcb_id soc_misc_init_pcb_id_not_support_default_ff
#define soc_misc_init_bom_id soc_misc_init_bom_id_not_support_default_ff
#endif
#ifdef CONFIG_BOARD_INFO_NOT_SUPPORT_DEFAULT_01
#define soc_misc_init_pcb_id soc_misc_init_pcb_id_not_support_default_01
#define soc_misc_init_bom_id soc_misc_init_bom_id_not_support_default_01
#endif
#if !defined(soc_misc_init_bom_id) && !defined(STATIC_SKIP)
#error "not defined feature macro"
#endif

#endif
