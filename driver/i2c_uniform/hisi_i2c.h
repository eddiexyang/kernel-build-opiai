/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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
 * Description: i2c header file of driver logic layer
 * Author: huawei
 * Create: 2022-03-08
 */
#ifndef HISI_I2C_H
#define HISI_I2C_H
#include <linux/i2c.h>
#include <linux/version.h>
#ifdef CONFIG_FMEA
#include "kdrv_fmea_api.h"
#endif
#include "kdrv_i2c_api.h"
#define I2C_MAX_STD_MODE_FREQ	100000
#define I2C_MAX_FAST_MODE_FREQ		400000
#define I2C_MAX_HIGH_SPEED_MODE_FREQ	3400000

#define I2C_STD_MODE_DEF_SCL_NS	1000
#define I2C_FAST_MODE_DEF_SCL_NS		300
#define I2C_HIGH_SPEED_MODE_DEF_SCL_NS	120

#define HZ_PER_KHZ               1000

#define FAST_SPEED_MODE_DIVIDE 26
#define FAST_SPEED_MODE_DIVISOR 76
#define HIGH_SPEED_MODE_DIVIDE 6
#define HIGH_SPEED_MODE_DIVISOR 22
#define STD_SPEED_MODE_DIVIDE 40
#define STD_SPEED_MODE_DIVISOR 87

struct i2c_timings_cfg {
    u32 clk_rate_khz; // clk 200M   200000khz
    u32 spk_len; // Filtering burr length (number of cycles)
    u32 scl_hcnt; // Number of high-level cycles
    u32 scl_lcnt; // Number of low-level cycles
    u32 sda_hold_cnt;
};

struct hisi_i2c_timings {
    struct i2c_timings t;
    struct i2c_timings_cfg cfg;
};

struct hisi_i2c_trans_info {
    struct completion cmd_complete;
    struct i2c_msg *msgs;
    int msg_num;
    int msg_tx_idx;
    int buf_tx_idx;
    int msg_rx_idx;
    int buf_rx_idx;
    u32 xfer_err;
    u32 err_source;
    int msg_err;
};

struct i2c_int_status_info {
    u8 is_tx_aempty_irq;
    u8 is_rx_afull_irq;
    u8 is_trans_cplt_irq;
    u8 is_err_irq;
    u32 xfer_err;
    u32 err_source;
};
#ifndef CONFIG_FMEA
struct io_region {
    void __iomem *io_base;
    u32 io_size;
};
#endif
struct hisi_i2c_ops {
    void (*config_bus)(struct io_region *reg, u32 bus_freq_hz, struct i2c_timings_cfg *cfg);
    void (*enable_tx_aempty_int)(struct io_region *reg, u32 enable);
    void (*enable_rx_afull_int)(struct io_region *reg, u32 enable);
    void (*enable_trans_cplt_int)(struct io_region *reg, u32 enable);
    void (*enable_err_int)(struct io_region *reg, u32 enable);
    void (*disable_all_int)(struct io_region *reg);
    void (*get_int_status_info)(struct io_region *reg, struct i2c_int_status_info *status);
    void (*clear_all_int)(struct io_region *reg);
    void (*clear_tx_aempty_int)(struct io_region *reg);
    void (*clear_rx_afull_int)(struct io_region *reg);
    void (*clear_trans_cplt_int)(struct io_region *reg);
    void (*clear_err_int)(struct io_region *reg);
    void (*handle_errors)(struct device *dev, struct io_region *reg, u32 xfer_err, u32 err_source);
    void (*xfer_init)(struct io_region *reg, u16 addr, bool is_10bit_addr);
    void (*write_tx_data)(struct io_region *reg, u8 data, u16 wr, bool need_stop, bool need_restart);
    u8 (*read_rx_data)(struct io_region *reg);
    bool (*tx_fifo_full)(struct io_region *reg);
    bool (*rx_fifo_empty)(struct io_region *reg);
    u32 (*get_tx_fifo_limit)(struct io_region *reg);
    void (*get_err_status)(struct io_region *reg, struct kdrv_i2c_err_status *status);
    void (*get_timing_cfg)(struct io_region *reg, struct kdrv_i2c_timing_cfg *cfg);
};

struct hisi_i2c_controller {
    struct i2c_adapter adapter;
    struct io_region reg;
    struct device *dev;
    int irq;
    struct reset_control *rst;
    struct clk *clk;
    u32 bus_id;
    /* Intermediates for recording the transfer process */
    struct hisi_i2c_trans_info trans_info;
    struct kdrv_i2c_trans_statistic_info statis_info;
    /* I2C bus configuration timings */
    struct hisi_i2c_timings timings;
    /* i2c bus recovery */
    struct i2c_bus_recovery_info rinfo;
    struct pinctrl *pinctrl;
    struct pinctrl_state *pinctrl_pins_default;
    struct pinctrl_state *pinctrl_pins_gpio;
    struct pinctrl_state *pinctrl_pins_idle;
    const struct hisi_i2c_ops *soc_ops;
#ifdef CONFIG_FMEA
    struct fmea_iomm_unit obj;
#endif
};

struct hisi_i2c_controller *hisi_i2c_get_controller_by_id(u32 bus_id);
const struct hisi_i2c_ops *hisi_i2c_get_ops(void);
#ifdef CONFIG_FMEA
const struct fmea_item_ruler_data *i2c_get_fmea_ruler_data(u32 *nums);
const struct ndrv_fmea_item_info* i2c_get_fmea_table(void);
#endif
#endif