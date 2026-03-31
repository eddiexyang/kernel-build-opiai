/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2011-2019. All rights reserved.
 * Description: voie_hal.h
 * Author: Hisilicon multimedia software group
 * Create: 2011/12/06
 * History:
 *   1.Date        : 2011/12/06
 *     Modification: Created file
 */

#ifndef VOIE_HAL_H
#define VOIE_HAL_H

#include "voie_reg.h"

td_s32 voie_hal_init(td_void);

td_void voie_hal_exit(td_void);

td_void voie_hal_set_clk(td_bool enable);

td_void voie_hal_get_int_status(td_u32 *int_status);

td_void voie_hal_clear_int_status(td_void);

td_void voie_hal_start(td_u32 phys_addr);

td_void voie_hal_set_g726_state(td_u32 *g726_state);

td_void voie_hal_set_adpcm_state(td_u32 *adpcm_state);

#endif /* end of VOIE_HAL_H */

