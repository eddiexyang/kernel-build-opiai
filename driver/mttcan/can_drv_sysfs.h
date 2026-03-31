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

#ifndef _CAN_DRV_SYSFS_H
#define _CAN_DRV_SYSFS_H

#include <linux/can/dev.h>
#include <linux/of_device.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/securec.h>
#include "can_drv_log.h"

#define PARSED_AS_DECIMAL 10

struct run_status_cnt {
    u64 lec_type_stuff_err_cnt;
    u64 lec_type_form_err_cnt;
    u64 lec_type_ack_err_cnt;
    u64 lec_type_bit0_err_cnt;
    u64 lec_type_bit1_err_cnt;
    u64 lec_type_crc_err_cnt;
    u64 rx_fifo_lost_err_cnt;
    u64 rx_kfifo_full_cnt;
    u64 unknow_err_cnt;
    u64 tx_buffer_full_err_cnt;
    u64 tx_kfifo_full_cnt;
    u64 rx_tasklet_delay2ms_cnt;
    u64 rx_tasklet_delay5ms_cnt;
    u64 tx_fifo_lost_err_cnt;
};

#ifdef RUN_IN_AOS
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

void mttcan_proc_create_node(struct net_device *ndev);
void mttcan_proc_remove_node(struct net_device *ndev);

#else

#include <linux/sysfs.h>

/**
 * can_drv_sysfs_add_nodes - add sysfs node
 * @dev: can device object
 *
 * Create a CAN sysfs node for CAN history error query.
 * It should be called in the driver's probe function,
 * and the can_drv_sysfs_remove_nodes function should be called
 * in remove function to delete the created sysfs node
 */
void can_drv_sysfs_add_nodes(struct device *dev);

/**
 * can_drv_sysfs_remove_nodes - remove sysfs node
 * @dev: CAN device object
 *
 * Remove a CAN sysfs node.
 * It should be called in the driver's remove function,
 * More can related to can_drv_sysfs_add_nodes.
 */
void can_drv_sysfs_remove_nodes(struct device *dev);

#endif

#endif
