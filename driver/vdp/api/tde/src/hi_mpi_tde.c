/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: tde HI API file
 * Author: Hisilicon multimedia software group
 * Create: 2023/03/10
 */

#include "hi_mpi_tde.h"
#include "mid_mpi_tde.h"

hi_s32 hi_tde_open(hi_void)
{
    return mid_tde_open();
}

hi_void hi_tde_close(hi_void)
{
    mid_tde_close();
}

hi_s32 hi_tde_begin_job(hi_void)
{
    return mid_tde_begin_job();
}

hi_s32 hi_tde_end_job(hi_s32 handle, hi_bool is_sync, hi_bool is_block, hi_u32 time_out)
{
    return mid_tde_end_job(handle, (mid_bool)is_sync, (mid_bool)is_block, time_out);
}

hi_s32 hi_tde_cancel_job(hi_s32 handle)
{
    return mid_tde_cancel_job(handle);
}

hi_s32 hi_tde_wait_for_done(hi_s32 handle)
{
    return mid_tde_wait_for_done(handle);
}

hi_s32 hi_tde_wait_all_done(hi_void)
{
    return mid_tde_wait_all_done();
}

hi_s32 hi_tde_quick_fill(hi_s32 handle, const hi_tde_none_src *none_src, hi_u32 fill_data)
{
    return mid_tde_quick_fill(handle, (mid_tde_none_src *)none_src, fill_data);
}

hi_s32 hi_tde_quick_copy(hi_s32 handle, const hi_tde_single_src *single_src)
{
    return mid_tde_quick_copy(handle, (mid_tde_single_src *)single_src);
}

hi_s32 hi_tde_pattern_fill(hi_s32 handle, const hi_tde_double_src *double_src, const hi_tde_pattern_fill_opt *fill_opt)
{
    return mid_tde_pattern_fill(handle, (mid_tde_double_src *)double_src, (mid_tde_pattern_fill_opt *)fill_opt);
}