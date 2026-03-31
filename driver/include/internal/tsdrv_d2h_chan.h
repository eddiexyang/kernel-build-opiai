/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
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
#ifndef HVTSDRV_D2H_CHAN_H
#define HVTSDRV_D2H_CHAN_H

#include "tsdrv_d2h_chan_ops.h"
#include "tsdrv_pci_chan.h"

int tsdrv_d2h_slow_msg_send(u32 devid, void *tx, size_t tx_size);

bool tsdrv_d2h_slow_chan_ready(u32 devid);

int tsdrv_d2h_chan_setup(void);
void tsdrv_d2h_chan_cleanup(void);

#endif /* _HVTSDRV_D2H_CHAN_H_ */
