/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: inner header of RemixV100 audio codec
 * Author: Hisilicon multimedia software group
 * Create: 2022-09-10
 */

#ifndef ACODEC_H
#define ACODEC_H

#include "ot_inner_acodec.h"

td_u32 acodec_hal_read_reg(td_u32 offset);
td_void acodec_hal_write_reg(td_u32 offset, td_u32 value);

#endif /* End of #ifndef ACODEC_H */