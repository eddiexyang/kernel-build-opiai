/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
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
#ifndef _AGENTDRV_DFX_H_
#define _AGENTDRV_DFX_H_

#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/version.h>

#include "agentdrv_platform.h"
#include "agentdrv_unit.h"

/* base addr */
#define AGENTDRV_IOB_TX_REG 0x00
#define AGENTDRV_EPF_CFG_REG (0x80000 * ((AGENTDRV_CORE_NUM) + 1) + 0x5000 + (0x4000 * (AGENTDRV_PORT_NUM)))
#define AGENTDRV_TL_REG (0x80000 * ((AGENTDRV_CORE_NUM) + 1) + 0x4000 + (0x4000 * (AGENTDRV_PORT_NUM)))

/* IOB TX REG */
#define AGENTDRV_OB_TX_PORT_ERR_CNT 0x2688
#define AGENTDRV_OB_TX_PORT_NP_CNT 0x2608
/* EPF CFG */
#define AGENTDRV_EPF_CFG_UNCR_ERR_STATUS 0x104
#define AGENTDRV_EPF_CFG_COR_ERR_STATUS 0x110
/* TL REG */
#define AGENTDRV_TL_RX_ERR_STATUS_DMA 0xb9c
/* DMA REG */
#define AGENTDRV_DMA_QUEUE_DESP0 0x50
#define AGENTDRV_DMA_QUEUE_DESP1 0x54
#define AGENTDRV_DMA_QUEUE_DESP2 0x58
#define AGENTDRV_DMA_QUEUE_DESP3 0x5c
#define AGENTDRV_DMA_QUEUE_DESP4 0x74
#define AGENTDRV_DMA_QUEUE_DESP5 0x78
#define AGENTDRV_DMA_QUEUE_DESP6 0x7c
#define AGENTDRV_DMA_QUEUE_DESP7 0x80
#define AGENTDRV_DMA_SQ_READ_ERR 0x68
#define AGENTDRV_DMA_SQ_STS 0x34
#define AGENTDRV_DMA_SQ_TAIL 0xc
#define AGENTDRV_DMA_CQ_HEAD 0x1c
#define AGENTDRV_DMA_CQ_TAIL 0x3c
#define AGENTDRV_DMA_QUEUE_INT_STS 0x40
/* channel index from 0~15, as 0x164, 0x264 */
#define AGENTDRV_DMA_QUEUE_ERR_SRC_CODE_H 0x64
#define AGENTDRV_DMA_QUEUE_ERR_SRC_CODE_L 0x60
/* MAC REG */
#define AGENTDRV_MAC_INT_STATUS 0x54
#define AGENTDRV_MAC_REG_LINK_INFO 0x60
#define AGENTDRV_MAC_SYMBOL_UNLOCL_COUNTER 0x8c
#define AGENTDRV_MAC_REG_DEBUG_PIPE9 0x250
#define AGENTDRV_MAC_REG_DEBUG_PIPE10 0x254
#define AGENTDRV_MAC_REG_DEBUG_PIPE11 0x258
#define AGENTDRV_MAC_PCS_RX_ERR_CNT 0x2bc

/* DL_REG */
#define AGENTDRV_DL_BP_EN 0x96c
#define AGENTDRV_DFX_MAC_BP_TIMER 0xB4

/* TL_REG */
#define AGENTDRV_TL_RX_TOTAL_CNT 0xb38
#define AGENTDRV_TL_RX_POST_CNT 0xb44
#define AGENTDRV_TL_RX_NONPOST_CNT 0xb48
#define AGENTDRV_TL_RX_CPL_CNT 0xb4c
#define AGENTDRV_TL_RX_ERRCPL_CNT 0xb50
#define AGENTDRV_TL_RX_ERR_CNT 0xb2c
#define AGENTDRV_RX_RX_BUFFER_STATUS 0xb18
#define AGENTDRV_TL_TX_VC0_P_FC_LEFT 0x9cc
#define AGENTDRV_TL_TX_FIFO_STS 0x9f0
#define AGENTDRV_TL_TX_MSG_CNT 0x924
#define AGENTDRV_TL_TX_DL_BP_CNT 0x990
#define AGENTDRV_TL_INT_STATUS0_DFX 0x56c
#define AGENTDRV_TL_INT_STATUS1_DFX 0x570

/* End of reg addr for linkdown dfx info */
/* bbox str size */
#define AGENTDRV_BLACK_BOX_BUF_LEN (16 * 1024)

/* get_ltssm_tracer_data_ok timeout 100ms, each delay 2ms, cycle 50  */
#define AGENTDRV_GET_TRACE_DATA_TIMEOUT 50

/* DFX linkdown ltssm trace START */
#define LTSSM_TRACER_HEAD \
    "ltssm tracer:\nltssm[ii]:  63:48 47:32 31 30 29 28 27 26 25 24 23 22 21:20 19:12 11:10 9:6 5:0\n"
#define LTSSM_TRACE_STR_FORMAT "%02u: 0x%04x %04x %x %x %x %x %x %x %x %x %x %x %x %02x %x %x %02x %-32s\n"
#define PCIE_LTSSM_TRACER_DEPTH 64
#define PCIE_MAC_REG_LTSSM_TRACER_CFG0_REG (AGENTDRV_PCIE_MAC + 0x2DC)      /* mac_ltssm_tracer_cfg0 */
#define PCIE_MAC_REG_LTSSM_TRACER_LAST_ADDR_REG (AGENTDRV_PCIE_MAC + 0x88)  /* LTSSM tracer last write address */
#define PCIE_MAC_REG_LTSSM_TRACER_ADDR_REG (AGENTDRV_PCIE_MAC + 0x2F8)      /* LTSSM tracer addr */
#define PCIE_MAC_REG_LTSSM_TRACER_OUTPUT_OK_REG (AGENTDRV_PCIE_MAC + 0x2A4) /* LTSSM tracer output */
#define PCIE_MAC_REG_LTSSM_TRACER_OUTPUT_2_REG (AGENTDRV_PCIE_MAC + 0x2A0)  /* LTSSM tracer output */
#define PCIE_MAC_REG_LTSSM_TRACER_OUTPUT_1_REG (AGENTDRV_PCIE_MAC + 0x84)   /* LTSSM tracer output */
#define PCIE_MAC_REG_LTSSM_TRACER_INPUT_REG (AGENTDRV_PCIE_MAC + 0x50)      /* LTSSM tracer input */

#define AGENTDRV_REG_BIT_19 19
#define AGENTDRV_TV_SEC 1000000
#define AGENTDRV_TV_NANOSEC 1000
/* ltssm_input */
#define AGENTDRV_LTSSM_INPUT_48 48
#define AGENTDRV_LTSSM_INPUT_BIT_48 0xFFFF
#define AGENTDRV_LTSSM_INPUT_32 32
#define AGENTDRV_LTSSM_INPUT_BIT_32 0xFFFF
#define AGENTDRV_LTSSM_INPUT_31 31
#define AGENTDRV_LTSSM_INPUT_BIT_31 0x1
#define AGENTDRV_LTSSM_INPUT_30 30
#define AGENTDRV_LTSSM_INPUT_29 29
#define AGENTDRV_LTSSM_INPUT_28 28
#define AGENTDRV_LTSSM_INPUT_27 27
#define AGENTDRV_LTSSM_INPUT_26 26
#define AGENTDRV_LTSSM_INPUT_25 25
#define AGENTDRV_LTSSM_INPUT_24 24
#define AGENTDRV_LTSSM_INPUT_23 23
#define AGENTDRV_LTSSM_INPUT_22 22
#define AGENTDRV_LTSSM_INPUT_20 20
#define AGENTDRV_LTSSM_INPUT_BIT_20 0x3
#define AGENTDRV_LTSSM_INPUT_12 12
#define AGENTDRV_LTSSM_INPUT_BIT_12 0x7F
#define AGENTDRV_LTSSM_INPUT_10 10
#define AGENTDRV_LTSSM_INPUT_BIT_10 0x3
#define AGENTDRV_LTSSM_INPUT_6 6
#define AGENTDRV_LTSSM_INPUT_BIT_6 0xF

#define AGENTDRV_VAL_NUM 32
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int ltssm_trace_signal_mask : 6;   /* [5..0]  */
        unsigned int reserved_0 : 3;                /* [8..6]  */
        unsigned int ltssm_trace_timer_clk_sel : 1; /* [9]  */
        unsigned int reserved_1 : 6;                /* [15..10]  */
        unsigned int ltssm_trace_lane_num : 4;      /* [19..16]  */
        unsigned int reserved_2 : 4;                /* [23..20]  */
        unsigned int sel_trace_rx_data_mode : 8;    /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} PCIE_MAC_REG_LTSSM_TRACER_CFG0;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int reg_ltssm_tracer_recap : 1;    /* [0]  */
        unsigned int reg_ltssm_tracer_cap_mode : 1; /* [1]  */
        unsigned int reserved_0 : 30;               /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} PCIE_MAC_REG_LTSSM_TRACER_INPUT;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int reg_ltssm_tracer_raddr : 6; /* [5..0]  */
        unsigned int reserved_0 : 26;            /* [31..6]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} PCIE_MAC_REG_LTSSM_TRACERADDR;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int ltssm_st : 6;                                     /* [5..0]  */
        unsigned int duration_counter : 4;                             /* [9..6]  */
        unsigned int mac_cur_link_speed : 2;                           /* [11..10]  */
        unsigned int train_bit_map : 8;                                /* [19..12]  */
        unsigned int rxl0s_st : 2;                                     /* [21..20]  */
        unsigned int any_change_pipe_req : 1;                          /* [22]  */
        unsigned int rcv_eios : 1;                                     /* [23]  */
        unsigned int dl_retrain : 1;                                   /* [24]  */
        unsigned int all_phy_rxeleidle_or_rx_skp_interval_timeout : 1; /* [25]  */
        unsigned int directed_speed_change : 1;                        /* [26]  */
        unsigned int any_det_eieos_ts : 1;                             /* [27]  */
        unsigned int rxl0s_to_recovery : 1;                            /* [28]  */
        unsigned int any_lane_rcv_speed_change : 1;                    /* [29]  */
        unsigned int changed_speed_recovery : 1;                       /* [30]  */
        unsigned int suceessful_speed_negotiation : 1;                 /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} PCIE_MAC_REG_LTSSM_TRACER_OUTPUT_1;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int train_bit_map : 16; /* [15..0]  */
        unsigned int txdetrx : 16;       /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} PCIE_MAC_REG_LTSSM_TRACER_OUTPUT_2;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int ltssm_tracer_data_ok : 1; /* [0]  */
        unsigned int reserved_0 : 31;          /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} PCIE_MAC_REG_LTSSM_TRACER_OUTPUT_OK;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int ltssm_tracer_last_waddr : 6;    /* [5..0]  */
        unsigned int ltssm_tracer_addr_rollback : 1; /* [6]  */
        unsigned int reserved_0 : 25;                /* [31..7]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} PCIE_MAC_REG_LTSSM_TRACER_LAST_ADDR;

struct AGENTDRV_LTSSM_STATE_T {
    int tag;
    char value[AGENTDRV_VAL_NUM];
};

/* DFX linkdown ltssm trace END */
struct agentdrv_dfx_linkdown_info {
    u32 pme_turn_off_times;
    u32 mac_bp_timer;              /* DL 0xb4 */
    u32 mac_link_info;             /* mac 0x60 */
    u32 bme_err;                   /* TL 0x56c */
    u32 mac_init_status;           /* MAC 0x54 */
    u32 mac_symbol_unlocl_counter; /* MAC 0x8c */
    u32 mac_debug_pipe9;           /* MAC 0x250 */
    u32 mac_debug_pipe10;          /* MAC 0x254 */
    u32 mac_debug_pipe11;          /* MAC 0x258 */
    u32 mac_pcs_err_cnt;           /* MAC 0x2bc */
    u32 cfg_bme_en;                /* CFG 0x4 */
    u32 tl_tx_int;                 /* TL 0x570 */
    u32 tl_fc_left;                /* TL 0x9cc */
    u32 tl_tx_fifo_sts;            /* TL 0x9f0 */
    u32 tl_tx_msg_cnt;             /* TL 0x924 */
    u32 tl_tx_total_cnt;           /* TL 0xb38 */
    u32 tl_tx_post_cnt;            /* TL 0xb44 */
    u32 tl_tx_nonpost_cnt;         /* TL 0xb48 */
    u32 tl_tx_cpl_cnt;             /* TL 0xb4c */
    u32 tl_tx_errcpl_cnt;          /* TL 0xb50 */
    u32 tl_tx_err_cnt;             /* TL 0xb2c */
    u32 buffer_status;             /* TL 0xb18 */
    u32 tl_tx_dl_bp_cnt;           /* TL 0x990 */
};

struct agentdrv_dfx_dma_info {
    u32 tx_port_err_cnt;   /* IOB TX 0x2688   */
    u32 tx_port_np_cnt;    /* IOB TX 0x2608 */
    u32 uncr_err_status;   /* EPF CFG 0x0104 */
    u32 cor_err_status;    /* EPF CFG 0x0110 */
    u32 tl_int_status0;    /* TL 0x056C */
    u32 tl_rx_err_status;  /* TL 0x0B9C */
    u32 queue_desp0_50;    /* DMA 0x50 */
    u32 queue_desp1_54;    /* DMA 0x54 */
    u32 queue_desp2_58;    /* DMA 0x58 */
    u32 queue_desp3_5c;    /* DMA 0x5c */
    u32 queue_desp4_74;    /* DMA 0x74 */
    u32 queue_desp5_78;    /* DMA 0x78 */
    u32 queue_desp6_7c;    /* DMA 0x7c */
    u32 queue_desp7_80;    /* DMA 0x80 */
    u32 queue_sq_read_err; /* DMA 0x68 */
    u32 queue_sq_sts;      /* DMA 0x34 */
    u64 sq_vir_base_src_code;
    u64 cq_vir_base_src_code;
    u32 soft_sq_tail;
    u32 soft_sq_head;
    u32 soft_cq_head;
    u32 hard_sq_tail; /* DMA 0xC */
    u32 hard_sq_head;
    u32 hard_cq_head;         /* DMA 0x1C */
    u32 hard_cq_tail;         /* DMA 0x3C */
    u32 queue_init_sts;       /* DMA 0x40 */
    u32 queue_err_src_code_h; /* DMA 0x64+n*100, n:0~15 */
    u32 queue_err_src_code_l; /* DMA 0x60+n*100, n:0~15 */
};

struct agentdrv_dfx_log_info {
    spinlock_t spinlock;
    u32 linkdown_in_hotreset;
    char *bbox_storage_str;
    int bbox_storage_str_offset;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
    struct timespec64 current_time;
#else
    struct timeval current_time;
#endif
    struct agentdrv_dfx_linkdown_info dfx_linkdown_info;
    struct agentdrv_dfx_dma_info dfx_dma_info;
};

void agentdrv_dfx_put_string(const char *srcStr);
void agentdrv_init_common_msg(void);
int agentdrv_dfx_init_struct(void);
void agentdrv_dfx_free_struct(void);
void agentdev_pcie_set_reg_ltssm_tracer_recap(struct agentdrv_platform_dev *p_dev, u32 val);
void agentdrv_set_dl_bp_en(struct agentdrv_devctrl *agent_dev);
void agentdrv_dfx_linkdown_collect_link_state(struct agentdrv_platform_dev *p_dev);
void agentdrv_dfx_linkdown_put_into_bbox(struct agentdrv_platform_dev *p_dev);
#endif
