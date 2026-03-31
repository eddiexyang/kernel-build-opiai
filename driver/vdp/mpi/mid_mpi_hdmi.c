/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: hdmi api of mid
 * Author: Hisilicon multimedia software group
 * Create: 2023/03/19
 */

#include "mid_mpi_hdmi.h"
#include "ot_mpi_hdmi.h"

int mid_mpi_hdmi_init(mid_void)
{
    return ot_mpi_hdmi_init();
}

int mid_mpi_hdmi_deinit(mid_void)
{
    return ot_mpi_hdmi_deinit();
}

int mid_mpi_hdmi_open(mid_hdmi_id hdmi)
{
    return ot_mpi_hdmi_open((ot_hdmi_id)hdmi);
}

int mid_mpi_hdmi_close(mid_hdmi_id hdmi)
{
    return ot_mpi_hdmi_close((ot_hdmi_id)hdmi);
}

int mid_mpi_hdmi_set_attr(mid_hdmi_id hdmi, const mid_hdmi_attr *attr)
{
    return ot_mpi_hdmi_set_attr((ot_hdmi_id)hdmi, (ot_hdmi_attr *)attr);
}

int mid_mpi_hdmi_get_attr(mid_hdmi_id hdmi, mid_hdmi_attr *attr)
{
    return ot_mpi_hdmi_get_attr((ot_hdmi_id)hdmi, (ot_hdmi_attr *)attr);
}

int mid_mpi_hdmi_start(mid_hdmi_id hdmi)
{
    return ot_mpi_hdmi_start((ot_hdmi_id)hdmi);
}

int mid_mpi_hdmi_stop(mid_hdmi_id hdmi)
{
    return ot_mpi_hdmi_stop((ot_hdmi_id)hdmi);
}

int mid_mpi_hdmi_set_infoframe(mid_hdmi_id hdmi, const mid_hdmi_infoframe *infoframe)
{
    return ot_mpi_hdmi_set_infoframe((ot_hdmi_id)hdmi, (ot_hdmi_infoframe *)infoframe);
}

int mid_mpi_hdmi_get_sink_capability(mid_hdmi_id hdmi, mid_hdmi_sink_capability *capability)
{
    int ret;
    ot_hdmi_sink_capability ot_capability = {0};

    ret = ot_mpi_hdmi_get_sink_capability((ot_hdmi_id)hdmi, &ot_capability);
    if (capability != NULL) { // get capability value
        capability->is_connected = (mid_bool)ot_capability.is_connected;
        capability->support_hdmi = (mid_bool)ot_capability.support_hdmi;
        capability->support_hdmi_2_0 = (mid_bool)ot_capability.support_hdmi_2_0;
        return ret; // return get_sink_capability
    } else {
        return OT_ERR_HDMI_NULL_PTR; // null ptr
    }
}

int mid_mpi_hdmi_register_callback(mid_hdmi_id hdmi, const mid_hdmi_callback_func *callback_func)
{
    return ot_mpi_hdmi_register_callback((ot_hdmi_id)hdmi, (ot_hdmi_callback_func *)callback_func); // ot
}