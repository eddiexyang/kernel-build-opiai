/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: mipi operate header file
 * Author: Hisilicon multimedia software group
 * Create: 2023-03-08
 */
#ifndef MIPI_RX_OPERATE_H
#define MIPI_RX_OPERATE_H

#include "hi_osal.h"
#include "mipi_rx_config.h"

extern osal_spinlock_t g_mipi_ctx_spinlock;
extern mipi_dev_ctx_t g_mipi_dev_ctx;
extern osal_mutex_t g_mipi_mutex;

unsigned char mipi_is_dev_cfged(combo_dev_t devno);
long mipi_rx_ioctl(unsigned int cmd, unsigned long arg, const void *private_data);

int init_mipi_rx_user_ref(void);
void destory_mipi_rx_user_ref(void);

int mipi_do_suspend(void);
int mipi_do_resume(void);

#endif