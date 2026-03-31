/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2011-2019. All rights reserved.
 * Description: voie_reg.h
 * Author: Hisilicon multimedia software group
 * Create: 2011/12/06
 * History:
 *   1.Date        : 2011/12/06
 *     Modification: Created file
 */

#ifndef VOIE_REG_H
#define VOIE_REG_H

#include "ot_type.h"

#define CHN_STATE_SIZE      12   /* there are 12 state reg */
#define CHN_STATE_BYTE_SIZE 48   /* there are 48 byte state reg */
#define CHN_LLI_SIZE        8    /* there are 8 l_li reg */
#define CHN_LLI_BYTE_SIZE   0x20 /* there are 32 byte l_li reg */
#define OT_VENDOR_HEAD_SIZE 4    /* there are 4 byte vendor head */

#define VOIE_INT_NUM 102

/* VOIE 模块寄存器基地址与地址空间大小 */
#define VOIE_REG_BASE_PA 0x20640000
#define VOIE_REG_SIZE    0x10000

/* VOIE 时钟寄存器地址 */
#define VOIE_CLK_REG_PA 0x20030070  // PERI_CRG28

/******************************************************************************/
/*                      VOIE int mask                                */
/******************************************************************************/
#define VOIE_CFG_ERR_MASK    (0x01 << 24)
#define VOIE_CHK_ERR_MASK    (0x01 << 16)
#define VOIE_SINGLE_END_MASK (0x01 << 8)
#define VOIE_PIC_END_MASK    0x01

/* 32位数据读写 */
#define voie_writew(addr, value) ((*(volatile td_u32 *)(addr)) = (value))
#define voie_readw(addr, v)      ((v) = (*(volatile td_u32 *)(addr)))

/******************************************************************************/
/*                      VOIE 寄存器定义                                */
/******************************************************************************/
#define VOIE_INTSTAT_REG     (VOIE_REG_BASE_PA + 0x0)  /* 中断状态寄存器(status reg) */
#define VOIE_INTMASK_REG     (VOIE_REG_BASE_PA + 0x4)  /* 中断屏蔽寄存器(interrupt mask reg) */
#define VOIE_RAWINT_REG      (VOIE_REG_BASE_PA + 0x8)  /* 屏蔽前状态寄存器(interrupt status before masking) */
#define VOIE_INTCLR_REG      (VOIE_REG_BASE_PA + 0xC)  /* 中断清除寄存器(interruption clear reg) */
#define VOIE_START_REG       (VOIE_REG_BASE_PA + 0x10) /* 编码开始信号寄存器(start command reg) */
#define VOIE_OUTSTANDING_REG (VOIE_REG_BASE_PA + 0x14) /* 总线outstading寄存器(outstanding reg) */
#define RESERVED0_REG        (VOIE_REG_BASE_PA + 0x18) /* 保留寄存器_0 */
#define VOIE_MODE_REG        (VOIE_REG_BASE_PA + 0x20) /* VOIE工作模式寄存器( working mode reg) */
#define VOIE_CFG_REG         (VOIE_REG_BASE_PA + 0x28) /* 首个链表地址寄存器(start address of first LLI) */
#define RESERVED1_REG        (VOIE_REG_BASE_PA + 0x2C) /* 保留寄存器_1 */
#define VOIE_LLICFG0_REG     (VOIE_REG_BASE_PA + 0x30) /* 链表成员寄存器：src_phy_addr */
#define VOIE_LLICFG1_REG     (VOIE_REG_BASE_PA + 0x34) /* 链表成员寄存器：dst_phy_addr */
#define VOIE_LLICFG2_REG     (VOIE_REG_BASE_PA + 0x38) /* 链表成员寄存器：state_phy_addr */
#define VOIE_LLICFG3_REG     (VOIE_REG_BASE_PA + 0x3C) /* 链表成员寄存器：next_l_li_addr */
#define VOIE_LLICFG4_REG     (VOIE_REG_BASE_PA + 0x40) /* 链表成员寄存器：ctrl（编码控制） */
#define RESERVED2_REG        (VOIE_REG_BASE_PA + 0x44) /* 链表成员寄存器：保留寄存器_2 */
#define VOIE_LLISTATE0_REG   (VOIE_REG_BASE_PA + 0x50) /* G726编码通道变量寄存器 */
#define VOIE_LLISTATE1_REG   (VOIE_REG_BASE_PA + 0x54) /* G726编码通道变量寄存器 */
#define VOIE_LLISTATE2_REG   (VOIE_REG_BASE_PA + 0x58) /* G726编码通道变量寄存器 */
#define VOIE_LLISTATE3_REG   (VOIE_REG_BASE_PA + 0x5C) /* G726编码通道变量寄存器 */
#define VOIE_LLISTATE4_REG   (VOIE_REG_BASE_PA + 0x60) /* G726编码通道变量寄存器 */
#define VOIE_LLISTATE5_REG   (VOIE_REG_BASE_PA + 0x64) /* G726编码通道变量寄存器 */
#define VOIE_LLISTATE6_REG   (VOIE_REG_BASE_PA + 0x68) /* G726编码通道变量寄存器 */
#define VOIE_LLISTATE7_REG   (VOIE_REG_BASE_PA + 0x6C) /* G726编码通道变量寄存器 */
#define VOIE_LLISTATE8_REG   (VOIE_REG_BASE_PA + 0x70) /* G726编码通道变量寄存器 */
#define VOIE_LLISTATE9_REG   (VOIE_REG_BASE_PA + 0x74) /* G726编码通道变量寄存器 */
#define VOIE_LLISTATE10_REG  (VOIE_REG_BASE_PA + 0x78) /* G726编码通道变量寄存器 */
#define VOIE_STATE_CHK0_REG  (VOIE_REG_BASE_PA + 0x7C) /* VOIE G726 state check （G726通道变量校验寄存器） */
#define VOIE_LLISTATE11_REG  (VOIE_REG_BASE_PA + 0x80) /* ADPCM编码通道变量寄存器 */
#define VOIE_STATE_CHK1_REG  (VOIE_REG_BASE_PA + 0x84) /* VOIE ADPCM state check（ADPCM通道变量校验寄存器） */

// define the union u_voie_intstat
typedef union {
    // define the struct bits
    struct {
        td_u32 voie_end_of_frame : 1;   // [0]
        td_u32 reserved_3 : 7;       // [7..1]
        td_u32 voie_endof_single : 1;  // [8]
        td_u32 reserved_2 : 7;       // [15..9]
        td_u32 chk_err : 1;           // [16]
        td_u32 reserved_1 : 7;       // [23..17]
        td_u32 cfg_err : 1;           // [24]
        td_u32 reserved_0 : 7;       // [31..25]
    } bits;

    // define an unsigned member
    td_u32 u32;
} u_voie_intstat;

// define the union u_voie_intmask
typedef union {
    // define the struct bits
    struct {
        td_u32 voie_end_of_pic_mask : 1;     // [0]
        td_u32 reserved_3 : 7;           // [7..1]
        td_u32 voie_endof_single_mask : 1;  // [8]
        td_u32 reserved_2 : 7;           // [15..9]
        td_u32 chk_err_mask : 1;           // [16]
        td_u32 reserved_1 : 7;           // [23..17]
        td_u32 cfg_err_mask : 1;           // [24]
        td_u32 reserved_0 : 7;           // [31..25]
    } bits;

    // define an unsigned member
    td_u32 u32;
} u_voie_intmask;

// define the union u_voie_rawint
typedef union {
    // define the struct bits
    struct {
        td_u32 voie_end_of_frame : 1;   // [0]
        td_u32 reserved_3 : 7;       // [7..1]
        td_u32 voie_endof_single : 1;  // [8]
        td_u32 reserved_2 : 7;       // [15..9]
        td_u32 chk_err : 1;           // [16]
        td_u32 reserved_1 : 7;       // [23..17]
        td_u32 cfg_err : 1;           // [24]
        td_u32 reserved_0 : 7;       // [31..25]
    } bits;

    // define an unsigned member
    td_u32 u32;
} u_voie_rawint;

// define the union u_voie_intclr
typedef union {
    // define the struct bits
    struct {
        td_u32 voie_end_of_frame_clr : 1;   // [0]
        td_u32 reserved_3 : 7;          // [7..1]
        td_u32 voie_endof_single_clr : 1;  // [8]
        td_u32 reserved_2 : 7;          // [15..9]
        td_u32 chk_err_clr : 1;           // [16]
        td_u32 reserved_1 : 7;          // [23..17]
        td_u32 cfg_err_clr : 1;           // [24]
        td_u32 reserved_0 : 7;          // [31..25]
    } bits;

    // define an unsigned member
    td_u32 u32;
} u_voie_intclr;

// define the union U_VIOE_START
typedef union {
    // define the struct bits
    struct {
        td_u32 start : 1;        // [0]
        td_u32 reserved_0 : 31;  // [31..1]
    } bits;

    // define an unsigned member
    td_u32 u32;
} u_voie_start;

// define the union u_voie_outstanding
typedef union {
    // define the struct bits
    struct {
        td_u32 voie_outstding_num : 4;  // [3:0]
        td_u32 reserved_0 : 28;         // [31..1]
    } bits;

    // define an unsigned member
    td_u32 u32;
} u_voie_outstanding;

// define the union u_voie_mode
typedef union {
    // define the struct bits
    struct {
        td_u32 time_en : 2;        // [1..0]
        td_u32 reserved_2 : 6;    // [7..2]
        td_u32 accesslock_en : 1;  // [8]
        td_u32 reserved_1 : 7;    // [15..9]
        td_u32 clk_gate_en : 2;     // [17..16]
        td_u32 memclk_gate_en : 1;  // [18]
        td_u32 reserved_0 : 13;   // [31..19]
    } bits;

    // define an unsigned member
    td_u32 u32;
} u_voie_mode;

// define the union u_voie_llicfg4
typedef union {
    // define the struct bits
    struct {
        td_u32 reserved_0 : 7;        // [6..0]
        td_u32 ot_vendor_head : 1;         // [7]
        td_u32 codec : 8;             // [15..8]
        td_u32 samples_per_frame : 16;  // [31..16]
    } bits;

    // define an unsigned member
    td_u32 u32;
} u_voie_llicfg4;

// define the union u_voie_llistate0
typedef union {
    // define the struct bits
    struct {
        td_u32 a2 : 16;  // [15..0]
        td_u32 a1 : 16;  // [31..16]
    } bits;

    // define an unsigned member
    td_u32 u32;
} u_voie_llistate0;

// define the union u_voie_llistate1
typedef union {
    // define the struct bits
    struct {
        td_u32 td : 1;           // [0]
        td_u32 reserved_1 : 13;  // [13..1]
        td_u32 pk2 : 1;          // [14]
        td_u32 pk1 : 1;          // [15]
        td_u32 reserved_0 : 6;   // [21..16]
        td_u32 ap : 10;          // [31..22]
    } bits;

    // define an unsigned member
    td_u32 u32;
} u_voie_llistate1;

// define the union u_voie_llistate2
typedef union {
    // define the struct bits
    struct {
        td_u32 b2 : 16;  // [15..0]
        td_u32 b1 : 16;  // [31..16]
    } bits;

    // define an unsigned member
    td_u32 u32;
} u_voie_llistate2;

// define the union u_voie_llistate3
typedef union {
    // define the struct bits
    struct {
        td_u32 b4 : 16;  // [15..0]
        td_u32 b3 : 16;  // [31..16]
    } bits;

    // define an unsigned member
    td_u32 u32;
} u_voie_llistate3;

// define the union u_voie_llistate4
typedef union {
    // define the struct bits
    struct {
        td_u32 b6 : 16;  // [15..0]
        td_u32 b5 : 16;  // [31..16]
    } bits;

    // define an unsigned member
    td_u32 u32;
} u_voie_llistate4;

// define the union u_voie_llistate5
typedef union {
    // define the struct bits
    struct {
        td_u32 reserved_1 : 4;  // [3..0]
        td_u32 dms : 12;        // [15..4]
        td_u32 reserved_0 : 2;  // [17..16]
        td_u32 dml : 14;        // [31..18]
    } bits;

    // define an unsigned member
    td_u32 u32;
} u_voie_llistate5;

// define the union u_voie_llistate6
typedef union {
    // define the struct bits
    struct {
        td_u32 reserved_1 : 5;  // [4..0]
        td_u32 dq2 : 11;        // [15..5]
        td_u32 reserved_0 : 5;  // [20..16]
        td_u32 dq1 : 11;        // [31..21]
    } bits;

    // define an unsigned member
    td_u32 u32;
} u_voie_llistate6;

// define the union u_voie_llistate7
typedef union {
    // define the struct bits
    struct {
        td_u32 reserved_1 : 5;  // [4..0]
        td_u32 dq4 : 11;        // [15..5]
        td_u32 reserved_0 : 5;  // [20..16]
        td_u32 dq3 : 11;        // [31..21]
    } bits;

    // define an unsigned member
    td_u32 u32;
} u_voie_llistate7;

// define the union u_voie_llistate8
typedef union {
    // define the struct bits
    struct {
        td_u32 reserved_1 : 5;  // [4..0]
        td_u32 dq6 : 11;        // [15..5]
        td_u32 reserved_0 : 5;  // [20..16]
        td_u32 dq5 : 11;        // [31..21]
    } bits;

    // define an unsigned member
    td_u32 u32;
} u_voie_llistate8;

// define the union u_voie_llistate9
typedef union {
    // define the struct bits
    struct {
        td_u32 reserved_1 : 5;  // [4..0]
        td_u32 sr2 : 11;        // [15..5]
        td_u32 reserved_0 : 5;  // [20..16]
        td_u32 sr1 : 11;        // [31..21]
    } bits;

    // define an unsigned member
    td_u32 u32;
} u_voie_llistate9;

// define the union u_voie_llistate10
typedef union {
    // define the struct bits
    struct {
        td_u32 yu : 13;  // [12..0]
        td_u32 yl : 19;  // [31..13]
    } bits;

    // define an unsigned member
    td_u32 u32;
} u_voie_llistate10;

// define the union u_voie_llistate11
typedef union {
    // define the struct bits
    struct {
        td_u32 index : 8;       // [7..0]
        td_u32 reserved_0 : 8;  // [15..8]
        td_u32 valprev : 16;    // [31..16]
    } bits;

    // define an unsigned member
    td_u32 u32;
} u_voie_llistate11;

// ==============================================================================
// define the global struct
typedef struct {
    volatile u_voie_intstat voie_intstat;
    volatile u_voie_intmask voie_intmask;
    volatile u_voie_rawint voie_rawint;
    volatile u_voie_intclr voie_intclr;
    volatile u_voie_start voie_start;
    volatile u_voie_outstanding voie_outstanding;
    volatile td_u32 reserved0[2]; /* 2: reserved len */
    volatile u_voie_mode voie_mode;
    volatile td_u32 reserved1;
    volatile td_u32 voie_cfg;
    volatile td_u32 reserved2;
    volatile td_u32 src_phy_addr;
    volatile td_u32 dst_phy_addr;
    volatile td_u32 state_phy_addr;
    volatile td_u32 next_l_li_addr;
    volatile u_voie_llicfg4 ctrl;
    volatile td_u32 reserved3[3]; /* 3: reserved len */
    volatile u_voie_llistate0 voie_llistate0;
    volatile u_voie_llistate1 voie_llistate1;
    volatile u_voie_llistate2 voie_llistate2;
    volatile u_voie_llistate3 voie_llistate3;
    volatile u_voie_llistate4 voie_llistate4;
    volatile u_voie_llistate5 voie_llistate5;
    volatile u_voie_llistate6 voie_llistate6;
    volatile u_voie_llistate7 voie_llistate7;
    volatile u_voie_llistate8 voie_llistate8;
    volatile u_voie_llistate9 voie_llistate9;
    volatile u_voie_llistate10 voie_llistate10;
    volatile td_u32 voie_state_chk0;
    volatile u_voie_llistate11 voie_llistate11;
    volatile td_u32 voie_state_chk1;
} voie_regs;

/* G726通道变量 */
typedef struct {
    volatile u_voie_llistate0 voie_llistate0;
    volatile u_voie_llistate1 voie_llistate1;
    volatile u_voie_llistate2 voie_llistate2;
    volatile u_voie_llistate3 voie_llistate3;
    volatile u_voie_llistate4 voie_llistate4;
    volatile u_voie_llistate5 voie_llistate5;
    volatile u_voie_llistate6 voie_llistate6;
    volatile u_voie_llistate7 voie_llistate7;
    volatile u_voie_llistate8 voie_llistate8;
    volatile u_voie_llistate9 voie_llistate9;
    volatile u_voie_llistate10 voie_llistate10;
    volatile td_u32 voie_state_chk0;
} voie_g726_state;

/* ADPCM通道变量 */
typedef struct {
    volatile u_voie_llistate11 voie_llistate11;
    volatile td_u32 voie_state_chk1;
    /* 预留2个WORD，使该结构体占用字节数为16bytes的整数倍 */
    td_u32 au32_reserved[2]; /* 2: reserved len */
} voie_adpcm_state;

/* 通道变量 */
typedef struct {
    voie_g726_state g726_state;
    voie_adpcm_state adpcm_state;
} voie_chn_state;

/* VOIE编码链表结构 */
typedef struct {
    volatile td_u32 src_phy_addr;
    volatile td_u32 dst_phy_addr;
    volatile td_u32 state_phy_addr; /* 通道变量存放的物理地址 */
    volatile td_u32 next_l_li_addr;
    volatile u_voie_llicfg4 ctrl; /* 编码控制 */
    volatile td_u32 reserved0;
    volatile td_u32 reserved1;
    volatile td_u32 reserved2;
} voie_lli_node;

#endif /* VOIE_REG_H */
