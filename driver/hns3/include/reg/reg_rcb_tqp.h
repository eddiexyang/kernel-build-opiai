/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: reg_rcb_tqp
 * Author: huawei
 * Create: 2021-12-28
 */

#ifndef __REG_RCB_TQP_H__
#define __REG_RCB_TQP_H__

typedef union {
    struct {
        unsigned int rcb_rx_ring_base_addr_l : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rcb_cfg_rx_ring_baseaddr_l;

typedef union {
    struct {
        unsigned int rcb_rx_ring_base_addr_h : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rcb_cfg_rx_ring_baseaddr_l;

typedef union {
    struct {
        unsigned int ring_rx_bd_num : 12; /* [11:0] */
        unsigned int rsv : 20;            /* [31:12] */
    } bits;

    unsigned int value;
} u_rcb_cfg_rx_ring_bd_num;

typedef union {
    struct {
        unsigned int ring_bd_len : 2; /* [1:0] */
        unsigned int rsv : 30;        /* [31:2] */
    } bits;

    unsigned int value;
} u_rcb_cfg_rx_ring_bd_len;

typedef union {
    struct {
        unsigned int ring_rx_merge_en : 1; /* [0] */
        unsigned int rsv : 31;             /* [31:1] */
    } bits;

    unsigned int value;
} u_rcb_cfg_rx_ring_merge_en;

typedef union {
    struct {
        unsigned int rcb_rx_ring_tail_pointer : 16; /* [15:0] */
        unsigned int rsv : 16;                      /* [31:16] */
    } bits;

    unsigned int value;
} u_rcb_cfg_rx_ring_tail;

typedef union {
    struct {
        unsigned int rcb_rx_ring_head_pointer : 16; /* [15:0] */
        unsigned int rsv : 16;                      /* [31:16] */
    } bits;

    unsigned int value;
} u_rcb_cfg_rx_ring_head;

typedef union {
    struct {
        unsigned int rcb_rx_ring_fbdnum : 16; /* [15:0] */
        unsigned int rsv : 16;                /* [31:16] */
    } bits;

    unsigned int value;
} u_rcb_cfg_rx_ring_fbdnum;

typedef union {
    struct {
        unsigned int rcb_rx_ring_offset : 7; /* [6:0] */
        unsigned int rsv : 25;               /* [31:7] */
    } bits;

    unsigned int value;
} u_rcb_cfg_rx_ring_offset;

typedef union {
    struct {
        unsigned int rcb_rx_ring_fbd_offset : 7; /* [6:0] */
        unsigned int rsv : 25;                   /* [31:7] */
    } bits;

    unsigned int value;
} u_rcb_cfg_rx_ring_fbd_offset;

typedef union {
    struct {
        unsigned int rcb_rx_ring_pktnum_record : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rcb_cfg_rx_ring_pktnum_record;

typedef union {
    struct {
        unsigned int rcb_rx_lpid : 13;      /* [12:0] */
        unsigned int rcb_rx_lpid_valid : 1; /* [13] */
        unsigned int rcb_rx_nid_valid : 1;  /* [14] */
        unsigned int rsv : 17;              /* [31:15] */
    } bits;

    unsigned int value;
} u_rcb_cfg_rx_ring_lpid;

typedef union {
    struct {
        unsigned int rcb_rx_bd_err : 1; /* [0] */
        unsigned int rsv : 31;          /* [31:1] */
    } bits;

    unsigned int value;
} u_rcb_cfg_rx_ring_bd_err;

typedef union {
    struct {
        unsigned int rcb_tx_ring_base_addr_l : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rcb_cfg_tx_ring_baseaddr_l;

typedef union {
    struct {
        unsigned int rcb_tx_ring_base_addr_h : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rcb_cfg_tx_ring_baseaddr_h;

typedef union {
    struct {
        unsigned int ring_tx_bd_num : 12; /* [11:0] */
        unsigned int rsv : 20;            /* [31:12] */
    } bits;

    unsigned int value;
} u_rcb_cfg_tx_ring_bd_num;

typedef union {
    struct {
        unsigned int rcb_pri : 1; /* [0] */
        unsigned int rsv : 31;    /* [31:1] */
    } bits;

    unsigned int value;
} u_rcb_cfg_tx_ring_pri;

typedef union {
    struct {
        unsigned int ring_tc : 3; /* [2:0] */
        unsigned int rsv : 29;    /* [31:3] */
    } bits;

    unsigned int value;
} u_rcb_cfg_tx_ring_tc;

typedef union {
    struct {
        unsigned int ring_tx_merge_en : 1; /* [0] */
        unsigned int rsv : 31;             /* [31:1] */
    } bits;

    unsigned int value;
} u_rcb_cfg_tx_ring_merge_en;

typedef union {
    struct {
        unsigned int rcb_tx_ring_tail_pointer : 16; /* [15:0] */
        unsigned int rsv : 16;                      /* [31:16] */
    } bits;

    unsigned int value;
} u_rcb_cfg_tx_ring_tail;

typedef union {
    struct {
        unsigned int rcb_tx_ring_head_pointer : 16; /* [15:0] */
        unsigned int rsv : 16;                      /* [31:16] */
    } bits;

    unsigned int value;
} u_rcb_cfg_tx_ring_head;

typedef union {
    struct {
        unsigned int rcb_tx_ring_fbd_num : 16; /* [15:0] */
        unsigned int rsv : 16;                 /* [31:16] */
    } bits;

    unsigned int value;
} u_rcb_cfg_tx_ring_fbd_num;

typedef union {
    struct {
        unsigned int rcb_tx_ring_offset : 7; /* [6:0] */
        unsigned int rsv : 25;               /* [31:7] */
    } bits;

    unsigned int value;
} u_rcb_cfg_tx_ring_offset;

typedef union {
    struct {
        unsigned int rcb_tx_ring_ebd_num : 16; /* [15:0] */
        unsigned int rsv : 16;                 /* [31:16] */
    } bits;

    unsigned int value;
} u_rcb_cfg_tx_ring_ebd_num;

typedef union {
    struct {
        unsigned int rcb_tx_ring_pktnum_record : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rcb_cfg_tx_ring_pktnum_record;

typedef union {
    struct {
        unsigned int rcb_tx_ring_ebd_offset : 7; /* [6:0] */
        unsigned int rsv : 25;                   /* [31:7] */
    } bits;

    unsigned int value;
} u_rcb_cfg_tx_ring_ebd_offset;

typedef union {
    struct {
        unsigned int rcb_tx_bd_err : 1; /* [0] */
        unsigned int rsv : 31;          /* [31:1] */
    } bits;

    unsigned int value;
} u_rcb_cfg_tx_ring_bd_err;

typedef union {
    struct {
        unsigned int rcb_en : 1; /* [0] */
        unsigned int rsv : 31;   /* [31:1] */
    } bits;

    unsigned int value;
} u_rcb_cfg_en;

#endif  // __REG_RCB_TQP_H__