/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: hdmi api of hi
 * Author: Hisilicon multimedia software group
 * Create: 2023/03/19
 */

#include "hi_mpi_hdmi.h"
#include "mid_mpi_hdmi.h"

hi_s32 hi_mpi_hdmi_init(hi_void)
{
    return mid_mpi_hdmi_init();
}

hi_s32 hi_mpi_hdmi_deinit(hi_void)
{
    return mid_mpi_hdmi_deinit();
}

hi_s32 hi_mpi_hdmi_open(hi_hdmi_id hdmi)
{
    return mid_mpi_hdmi_open((mid_hdmi_id)hdmi);
}

hi_s32 hi_mpi_hdmi_close(hi_hdmi_id hdmi)
{
    return mid_mpi_hdmi_close((mid_hdmi_id)hdmi);
}

hi_s32 hi_mpi_hdmi_set_attr(hi_hdmi_id hdmi, const hi_hdmi_attr *attr)
{
    return mid_mpi_hdmi_set_attr((mid_hdmi_id)hdmi, (mid_hdmi_attr *)attr);
}

hi_s32 hi_mpi_hdmi_get_attr(hi_hdmi_id hdmi, hi_hdmi_attr *attr)
{
    return mid_mpi_hdmi_get_attr((mid_hdmi_id)hdmi, (mid_hdmi_attr *)attr);
}

hi_s32 hi_mpi_hdmi_start(hi_hdmi_id hdmi)
{
    return mid_mpi_hdmi_start((mid_hdmi_id)hdmi);
}

hi_s32 hi_mpi_hdmi_stop(hi_hdmi_id hdmi)
{
    return mid_mpi_hdmi_stop((mid_hdmi_id)hdmi);
}

hi_s32 hi_mpi_hdmi_set_infoframe(hi_hdmi_id hdmi, const hi_hdmi_infoframe *infoframe)
{
    return mid_mpi_hdmi_set_infoframe((mid_hdmi_id)hdmi, (mid_hdmi_infoframe *)infoframe);
}

hi_s32 hi_mpi_hdmi_get_sink_capability(hi_hdmi_id hdmi, hi_hdmi_sink_capability *capability)
{
    return mid_mpi_hdmi_get_sink_capability((mid_hdmi_id)hdmi, (mid_hdmi_sink_capability *)capability); // mid
}

hi_s32 hi_mpi_hdmi_register_callback(hi_hdmi_id hdmi, const hi_hdmi_callback_func *callback_func)
{
    return mid_mpi_hdmi_register_callback((mid_hdmi_id)hdmi, (mid_hdmi_callback_func *)callback_func); // mid
}