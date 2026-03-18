/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 * Create: 2023-6-5
 */

#include "hnae3.h"

typedef enum {
    SERDES_LOOPBACK_ALL_DISABLE = 0, /* disable all loopback */
    SERDES_LOOPBACK_PARALLEL_TX_TO_RX,
    SERDES_LOOPBACK_PARALLEL_RX_TO_TX,
    SERDES_LOOPBACK_SERIAL_TX_TO_RX, /* Compared with SERIAL_PREDRV_TX_TO_RX, this is performed after the DRV. */
    SERDES_LOOPBACK_SERIAL_RX_TO_TX,        /* not supported */
    SERDES_LOOPBACK_SERIAL_PREDRV_TX_TO_RX, /* Compared with SERIAL_TX_TO_RX, this is performed before the DRV. */
    SERDES_LOOPBACK_END
} SERDES_LOOPBACK_TYPE_E;

typedef enum {
    SERDES_DISABLE = 0,
    SERDES_ENABLE,
    SERDES_ENABLE_END
} SERDES_ENABLE_E;

int network_custom_set_loopback(struct hnae3_handle *handle, bool en, u32 loop_mode);
