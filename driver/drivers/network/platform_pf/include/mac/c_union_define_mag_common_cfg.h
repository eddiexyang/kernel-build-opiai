/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: mag_common_cfg
 * Author: huawei
 * Create: 2021-12-28
 */

#ifndef __C_UNION_DEFINE_MAG_COMMON_CFG_H__
#define __C_UNION_DEFINE_MAG_COMMON_CFG_H__

/* define the union u_mac_hss_invert */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_mac_hss_tx_inv : 1; /* [0] */
        unsigned int cfg_mac_hss_rx_inv : 1; /* [1] */
        unsigned int rsv_0 : 30;             /* [31:2] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_mac_hss_invert;

/* define the union u_mac_serdes_los_mask */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_xxvge_serdes_los_msk : 1; /* [0] */
        unsigned int rsv_1 : 31;                   /* [31:1] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_mac_serdes_los_mask;

/* define the union u_mac_coefup_timer */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_coefup_timer : 16; /* [15:0] */
        unsigned int rsv_2 : 16;            /* [31:16] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_mac_coefup_timer;

/* define the union u_mac_led_mod */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_led_mod : 4; /* [3:0] */
        unsigned int rsv_3 : 28;      /* [31:4] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_mac_led_mod;

/* define the union u_mac_par_led_sel */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_mac_led0_port : 5; /* [4:0] */
        unsigned int cfg_mac_led1_port : 5; /* [9:5] */
        unsigned int rsv_4 : 22;            /* [31:10] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_mac_par_led_sel;

/* define the union u_mac_mux_fifo_af_threshold */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_data_mux_fifo_af_thd : 5;  /* [4:0] */
        unsigned int cfg_ptp_mux_fifo_af_thd : 5;   /* [9:5] */
        unsigned int cfg_data_mux_fifo_sch_thd : 5; /* [14:10] */
        unsigned int rsv_5 : 17;                    /* [31:15] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_mac_mux_fifo_af_threshold;

/* define the union u_mac_tp_mem_ctrl */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_tp_ram_tmod : 32; /* [31:0] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_mac_tp_mem_ctrl;

/* define the union u_mac_sp_mem_ctrl */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_sp_ram_tmod : 32; /* [31:0] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_mac_sp_mem_ctrl;

/* define the union u_mac_mem_power_mode */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_mem_power_mode : 4; /* [3:0] */
        unsigned int rsv_6 : 28;             /* [31:4] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_mac_mem_power_mode;

/* define the union u_rtc_pps_int_mux */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_rtc_pps0_mux : 2; /* [1:0] */
        unsigned int cfg_rtc_pps1_mux : 2; /* [3:2] */
        unsigned int rsv_7 : 28;           /* [31:4] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_rtc_pps_int_mux;

/* define the union u_rtc_ptp_mux */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_rtc_ptp_mux : 2; /* [1:0] */
        unsigned int rsv_8 : 30;          /* [31:2] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_rtc_ptp_mux;

/* define the union u_rgmii_intf_mode */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_rgmii_mode : 2; /* [1:0] */
        unsigned int rsv_9 : 30;         /* [31:2] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_rgmii_intf_mode;

/* define the union u_rgmii_intf_delay */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_rgmii_tx_delay : 4; /* [3:0] */
        unsigned int rsv_10 : 28;            /* [31:4] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_rgmii_intf_delay;

/* define the union u_rgmii_intf_reset */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_rgmii_rx_rst : 1; /* [0] */
        unsigned int cfg_rgmii_tx_rst : 1; /* [1] */
        unsigned int rsv_11 : 30;          /* [31:2] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_rgmii_intf_reset;

/* define the union u_rgmii_intf_mux */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_rgmii_lane_mux : 1; /* [0] */
        unsigned int rsv_12 : 31;            /* [31:1] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_rgmii_intf_mux;

/* define the union u_mac_link_sync_status */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int mac_link_sync_sts : 4; /* [3:0] */
        unsigned int rsv_13 : 28;           /* [31:4] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_mac_link_sync_status;

/* define the union u_mac_hilink_los_status */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int mac_hilink_los_sts : 4; /* [3:0] */
        unsigned int rsv_14 : 28;            /* [31:4] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_mac_hilink_los_status;

/* define the union u_mac_fifo_full_his */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int mac_mux_data_fifo_full : 1;     /* [0] */
        unsigned int mac_mux_ptp_data_fifo_full : 1; /* [1] */
        unsigned int rsv_15 : 30;                    /* [31:2] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_mac_fifo_full_his;

/* define the union u_mac_link_sync_his */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int mac_sync_ok_src : 1;   /* [0] */
        unsigned int mac_sync_fail_src : 1; /* [1] */
        unsigned int hilink_los_src : 1;    /* [2] */
        unsigned int rsv_16 : 29;           /* [31:3] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_mac_link_sync_his;

#endif  // __C_UNION_DEFINE_MAG_COMMON_CFG_H__
