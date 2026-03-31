/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: reg_top
 * Author: huawei
 * Create: 2021-12-28
 */

#ifndef __REG_TOP_H__
#define __REG_TOP_H__

typedef union {
    struct {
        unsigned int device_id : 16; /* [15:0] */
        unsigned int rsv : 16;       /* [31:16] */
    } bits;

    unsigned int value;
} u_device_id;

typedef union {
    struct {
        unsigned int vendor_id : 16; /* [15:0] */
        unsigned int rsv : 16;       /* [31:16] */
    } bits;

    unsigned int value;
} u_vender_id;

typedef union {
    struct {
        unsigned int o_nic_idle : 1; /* [0] */
        unsigned int rsv : 31;       /* [31:1] */
    } bits;

    unsigned int value;
} u_nic_idle;

typedef union {
    struct {
        unsigned int cfg_ooo_wr_bp_wl : 16;     /* [15:0] */
        unsigned int cfg_ooo_wr_bp_clr_wl : 16; /* [31:16] */
    } bits;

    unsigned int value;
} u_mst_ooo_wr_bp;

typedef union {
    struct {
        unsigned int cfg_ooo_rd_bp_wl : 16;     /* [15:0] */
        unsigned int cfg_ooo_rd_bp_clr_wl : 16; /* [31:16] */
    } bits;

    unsigned int value;
} u_mst_ooo_rd_bp;

typedef union {
    struct {
        unsigned int mst_ooo_ava_rd_latency : 16; /* [15:0] */
        unsigned int mst_ooo_ava_wr_latency : 16; /* [31:16] */
    } bits;

    unsigned int value;
} u_mst0_ooo_latency;

typedef union {
    struct {
        unsigned int cfg_bp_bus_timer : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_bp_cpu_timeout;

typedef union {
    struct {
        unsigned int bp_cpu_history : 1; /* [0] */
        unsigned int bp_cpu_current : 1; /* [1] */
        unsigned int rsv : 30;           /* [31:2] */
    } bits;

    unsigned int value;
} u_bp_cpu_state;

typedef union {
    struct {
        unsigned int qos_allow_sel : 3;    /* [2:0] */
        unsigned int qos_allow_req_en : 1; /* [3] */
        unsigned int rsv : 28;             /* [31:4] */
    } bits;

    unsigned int value;
} u_cfg_eth_jam_qos;

typedef union {
    struct {
        unsigned int axi_slv_poison_mask : 1; /* [0] */
        unsigned int rsv : 31;                /* [31:1] */
    } bits;

    unsigned int value;
} u_axi_slv_poison_mask;

#endif  // __REG_TOP_H__