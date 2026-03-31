/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: rtc_common
 * Author: huawei
 * Create: 2021-12-28
 */

#ifndef __C_UNION_DEFINE_RTC_COMMON_H__
#define __C_UNION_DEFINE_RTC_COMMON_H__

/* define the union u_rtc_pps_int_enable */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int rtc_pps_int_en : 1; /* [0] */
        unsigned int rsv_0 : 31;         /* [31:1] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_rtc_pps_int_enable;

/* define the union u_rtc_pps_int_status */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int rtc_pps_int_sts : 1; /* [0] */
        unsigned int rsv_1 : 31;          /* [31:1] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_rtc_pps_int_status;

/* define the union u_rtc_pps_int_set */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int rtc_pps_int_set : 1; /* [0] */
        unsigned int rsv_2 : 31;          /* [31:1] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_rtc_pps_int_set;

/* define the union u_rtc_pps_int_source */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int rtc_pps_int_src : 1; /* [0] */
        unsigned int rsv_3 : 31;          /* [31:1] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_rtc_pps_int_source;

/* define the union u_rtc_pps_int_rpt_type */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int rtc_pps_int_rpt_type : 2; /* [1:0] */
        unsigned int rsv_4 : 30;               /* [31:2] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_rtc_pps_int_rpt_type;

/* define the union u_rtc_pps_int_ce_status */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int rtc_pps_int_sts_ce : 1; /* [0] */
        unsigned int rsv_5 : 31;             /* [31:1] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_rtc_pps_int_ce_status;

/* define the union u_rtc_pps_int_nfe_status */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int rtc_pps_int_sts_nfe : 1; /* [0] */
        unsigned int rsv_6 : 31;              /* [31:1] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_rtc_pps_int_nfe_status;

/* define the union u_rtc_pps_int_fe_status */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int rtc_pps_int_sts_fe : 1; /* [0] */
        unsigned int rsv_7 : 31;             /* [31:1] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_rtc_pps_int_fe_status;

/* define the union u_cfg_time_sync_h */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int time_sync_h : 16; /* [15:0] */
        unsigned int rsv_8 : 16;       /* [31:16] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_cfg_time_sync_h;

/* define the union u_cfg_time_sync_m */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int time_sync_m : 32; /* [31:0] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_cfg_time_sync_m;

/* define the union u_cfg_time_sync_l */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int time_sync_l : 32; /* [31:0] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_cfg_time_sync_l;

/* define the union u_cfg_time_sync_rdy */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int time_sync_rdy : 1; /* [0] */
        unsigned int rsv_9 : 31;        /* [31:1] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_cfg_time_sync_rdy;

/* define the union u_cfg_ptp_offset_add_rdy */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int ptp_offset_add_rdy : 1; /* [0] */
        unsigned int rsv_10 : 31;            /* [31:1] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_cfg_ptp_offset_add_rdy;

/* define the union u_cfg_time_cyc_ns */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_quotient : 8; /* [7:0] */
        unsigned int rsv_11 : 24;      /* [31:8] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_cfg_time_cyc_ns;

/* define the union u_cfg_time_cyc_ns_dec0 */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_denominator : 32; /* [31:0] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_cfg_time_cyc_ns_dec0;

/* define the union u_cfg_time_cyc_ns_dec1 */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_numerator : 32; /* [31:0] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_cfg_time_cyc_ns_dec1;

/* define the union u_cfg_time_cyc_en */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_time_cyc_en : 1; /* [0] */
        unsigned int rsv_12 : 31;         /* [31:1] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_cfg_time_cyc_en;

/* define the union u_curr_time_out_h */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int time_out_h : 16; /* [15:0] */
        unsigned int rsv_13 : 16;     /* [31:16] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_curr_time_out_h;

/* define the union u_curr_time_out_l */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int time_out_l : 32; /* [31:0] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_curr_time_out_l;

/* define the union u_curr_time_out_ns */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int time_out_ns : 32; /* [31:0] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_curr_time_out_ns;

/* define the union u_cfg_pps_width */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_pps_width : 32; /* [31:0] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_cfg_pps_width;

/* define the union u_cfg_pps_period */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_pps_period : 5; /* [4:0] */
        unsigned int cfg_pps_en : 1;     /* [5] */
        unsigned int rsv_14 : 26;        /* [31:6] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_cfg_pps_period;

/* define the union u_cfg_pps_invert */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_pps_inv : 1; /* [0] */
        unsigned int rsv_15 : 31;     /* [31:1] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_cfg_pps_invert;

/* define the union u_cfg_rtc_ppe */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int rtc_ppe_cfg_quotient : 8;     /* [7:0] */
        unsigned int rtc_ppe_cfg_numerator : 11;   /* [18:8] */
        unsigned int rtc_ppe_cfg_denominator : 11; /* [29:19] */
        unsigned int rsv_16 : 2;                   /* [31:30] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_cfg_rtc_ppe;

#endif  // __C_UNION_DEFINE_RTC_COMMON_H__
