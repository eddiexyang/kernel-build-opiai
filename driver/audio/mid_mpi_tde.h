/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: tde API mid header file
 * Author: Hisilicon multimedia software group
 * Create: 2023/03/10
 */

#ifndef MID_MPI_TDE_H
#define MID_MPI_TDE_H

#include "mid_base_tde.h"
#ifdef __cplusplus
extern "C" {
#endif

/* MID Declaration */
__attribute__((weak)) int mid_tde_open(void);
__attribute__((weak)) void mid_tde_close(void);
__attribute__((weak)) int mid_tde_begin_job(void);
__attribute__((weak)) int mid_tde_end_job(int handle, mid_bool is_sync, mid_bool is_block, unsigned int time_out);
__attribute__((weak)) int mid_tde_cancel_job(int handle);
__attribute__((weak)) int mid_tde_wait_for_done(int handle);
__attribute__((weak)) int mid_tde_wait_all_done(void);
__attribute__((weak)) int mid_tde_quick_fill(int handle, const mid_tde_none_src *none_src, unsigned int fill_data);
__attribute__((weak)) int mid_tde_quick_copy(int handle, const mid_tde_single_src *single_src);
__attribute__((weak)) int mid_tde_pattern_fill(int handle,
                                               const mid_tde_double_src *double_src,
                                               const mid_tde_pattern_fill_opt *fill_opt);
#ifdef __cplusplus
}
#endif

#endif /* MID_MPI_TDE_H */