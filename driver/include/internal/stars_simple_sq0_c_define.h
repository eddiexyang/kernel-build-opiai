/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
 * Create: 2023-1-11
 */

#ifndef __STARS_SIMPLE_SQ0_C_DEFINE_H__
#define __STARS_SIMPLE_SQ0_C_DEFINE_H__

/* The base address of the module stars_simple_sq0 */
#define STARS_SIMPLE_SQ0_BASE_ADDR 0x8000000  /* module base addr */

/*
 * DEFINE REGISTER UNION
 */
/* Define the union STARS_P0_SQ_DB_UNION */
typedef union {
    /* Define the struct bits */
    struct {
        /*
         * The idx of the SQn entry that TS FW would write to(i.e. next).  In units of 64B
         * This is for HW H/S unit to write into it. SQ_head would be updated by both HW H/S unit and SQ state machine
         */
        unsigned int p0_sq_tail              : 16;      /* [15:0] */
        unsigned int reserved                : 16;      /* [31:16] */
    } bits;
    /* Define an unsigned member */
    unsigned int u32;
} STARS_P0_SQ_DB_UNION;

#define STARS_P0_SQ_DB                   (0x000008) // default val: 0x00000000

/* Define the union STARS_P0_SQ_CFG4_UNION */
typedef union {
    /* Define the struct bits */
    struct {
        /* The idx of the SQn entry that would be read from by the nth SQ.  In units of 64B. */
        unsigned int p0_sq_head              : 16;      /* [15:0] */
        unsigned int reserved                : 16;      /* [31:16] */
    } bits;
    /* Define an unsigned member */
    unsigned int u32;
} STARS_P0_SQ_CFG4_UNION;

#define STARS_P0_SQ_CFG4                   (0x000010) // default val: 0x00000000

/* Define the union STARS_P0_SQ_CFG5_UNION */
typedef union {
    /* Define the struct bits */
    struct {
        /* The enable switch of RTSQ. */
        unsigned int p0_sq_en                : 1;       /* [0] */
        unsigned int reserved                : 31;      /* [31:1] */
    } bits;
    /* Define an unsigned member */
    unsigned int u32;
} STARS_P0_SQ_CFG5_UNION;

#define STARS_P0_SQ_CFG5                   (0x000014) // default val: 0x00000000

/* Define the union STARS_P0_SQ_CFG0_UNION */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int reserved0               : 4;       /* [3:0] */
        /* total 64 model group; */
        unsigned int p0_sq_model_group       : 6;       /* [9:4] */
        /*
         * Priority setting of the current SQ
         * Compared against SP_LEVEL to determine the priority group
         */
        unsigned int p0_sq_priority          : 3;       /* [12:10] */
        unsigned int reserved1               : 19;      /* [31:13] */
    } bits;
    /* Define an unsigned member */
    unsigned int u32;
} STARS_P0_SQ_CFG0_UNION;

#define STARS_P0_SQ_CFG0                   (0x000200) // default val: 0x00000000

/* Define the union STARS_P0_TASK_CTRL0_UNION */
typedef union {
    /* Define the struct bits */
    struct {
        /* Writing 1 to this register would generate a pulse to the corresponding SQCQ state machine into debug state,
         * stop SQCQ state machine into launching new blocks（only stop DVPP task at block boundary）,
         * and report debug_paused interrupt
         */
        unsigned int p0_debug_pause          : 1;       /* [0] */
        /* Writing 1 to this register would generate a pulse internally,
         * which would park the main fsm to a special state,
         * stop SQCQ state machine into launching new blocks（only stop DVPP task at block boundary）,
         * and when all launched block has been excuted,an task_paused interrupt would be signaled
         * if it had not been masked and recorded in the corresponding interrupt status bit
         */
        unsigned int p0_task_pause           : 1;       /* [1] */
        /* Writing 1 to this register would generate a pulse internally,
         * which would then sends in ccu_stall commands to AI core(s),
         * and finally "park" to a task kill wait state,
         * that waits for a further task terminate pulse
         */
        unsigned int p0_task_kill            : 1;       /* [2] */
        unsigned int reserved0               : 13;      /* [15:3] */
        /* Writing 1 to this register would generate a pulse internally,
         * and would make the main fsm go back to the block issuing state
         */
        unsigned int p0_debug_resume         : 1;       /* [16] */
        /* Writing 1 to this register would generate a pulse internally,
         * and to make the main fsm to exit from any holding state
         * (e.g. if CQ is full, pre_p, post_p bit was hit, or task_pause),
         * to the next state of the process (e.g. pre_p then it jumps to kickstart state,
         * post_p then it jumps to the write CQ state, etc)
         */
        unsigned int p0_task_resume          : 1;       /* [17] */
        /* Writing 1 to this register would generate a pulse internally,
         * and to make the main fsm to go back into idle state,
         * if the FSM had been in an exception state before
         */
        unsigned int p0_exception_handled    : 1;       /* [18] */
        /* Writing 1 to this register would generate a pulse internally
         * which would make the main fsm to go back into idle state,
         * if previously a task_kill pulse had been issued
         */
        unsigned int p0_task_terminate       : 1;       /* [19] */
        unsigned int reserved1               : 12;      /* [31:20] */
    } bits;
    /* Define an unsigned member */
    unsigned int u32;
} STARS_P0_TASK_CTRL0_UNION;

#define STARS_P0_TASK_CTRL0                   (0x000210) // default val: 0x00000000

/* Define the union STARS_P0_TASK_CTRL1_UNION */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int reserved0               : 1;       /* [0] */
        /* write 1 to this bit, would clear all prefetched SQEs in the current SQ.
         * According to the corresponding SQ/CQ's head/tail setting, prefetch would be restarted.
         * Could only be used when STARS is under exception or paused
         */
        unsigned int p0_prefetch_clear       : 1;       /* [1] */
        unsigned int reserved1               : 30;      /* [31:2] */
    } bits;
    /* Define an unsigned member */
    unsigned int u32;
} STARS_P0_TASK_CTRL1_UNION;

#define STARS_P0_TASK_CTRL1                   (0x000220) // default val: 0x00000000

/* Define the union P0_SQ_TO_ACTIVE_SQ_MAP0_UNION */
typedef union {
    /* Define the struct bits */
    struct {
        /*
         * If mapped_in_active_set == 0, this value can take any value (i.e. do not check)
         * Otherwise this is the corresponding SQid that the current SQ has been mapped inside the active SQ set
         */
        unsigned int p0_mapped_in_active_sqid : 7;       /* [6:0] */
        unsigned int reserved                : 25;      /* [31:7] */
    } bits;
    /* Define an unsigned member */
    unsigned int u32;
} P0_SQ_TO_ACTIVE_SQ_MAP0_UNION;

#define P0_SQ_TO_ACTIVE_SQ_MAP0                   (0x000230) // default val: 0x00000000

/* Define the union P0_SQ_TO_ACTIVE_SQ_MAP1_UNION */
typedef union {
    /* Define the struct bits */
    struct {
        /*
         * The use case of this register is for example, when a certain active set SQ had encountered a breakpoint,
         * the debug interrupt was reported according to SQid,
         * while the DFX state is only avaiable in the active SQ set,
         * could know about this information by reading out this register
         * 0: Current SQ is not under any of the active SQ channels
         * 1: Current SQ is actually mapped to active SQ set (0 - 127)
         */
        unsigned int p0_mapped_in_active_set : 1;       /* [0] */
        unsigned int reserved                : 31;      /* [31:1] */
    } bits;
    /* Define an unsigned member */
    unsigned int u32;
} P0_SQ_TO_ACTIVE_SQ_MAP1_UNION;

#define P0_SQ_TO_ACTIVE_SQ_MAP1                   (0x000240) // default val: 0x00000000

typedef struct {
    volatile STARS_P0_SQ_DB_UNION        stars_p0_sq_db;
    unsigned int                         rsr0[1];
    volatile STARS_P0_SQ_CFG4_UNION      stars_p0_sq_cfg4;
    volatile STARS_P0_SQ_CFG5_UNION      stars_p0_sq_cfg5;
    unsigned int                         rsr1[122];
    volatile STARS_P0_SQ_CFG0_UNION      stars_p0_sq_cfg0;
    unsigned int                         rsr2[3];
    volatile STARS_P0_TASK_CTRL0_UNION   stars_p0_task_ctrl0;
    unsigned int                         rsr3[3];
    volatile STARS_P0_TASK_CTRL1_UNION   stars_p0_task_ctrl1;
    unsigned int                         rsr4[3];
    volatile P0_SQ_TO_ACTIVE_SQ_MAP0_UNION p0_sq_to_active_sq_map0;
    unsigned int                         rsr5[3];
    volatile P0_SQ_TO_ACTIVE_SQ_MAP1_UNION p0_sq_to_active_sq_map1;
    unsigned int                         rsr6[881];
} REGS_STARS_P0_SQ_DB_TO_P0_SQ_TO_ACTIVE_SQ_MAP1;

/*
 * DEFINE GLOBAL STRUCT
 */
typedef struct {
    unsigned int                         rsr0[2];                             /* 0 ~ 4 */
    volatile REGS_STARS_P0_SQ_DB_TO_P0_SQ_TO_ACTIVE_SQ_MAP1 stars_p0_sq_db[2048];                /* 8 ~ 8000004 */
} STARS_SIMPLE_SQ0_REGS_TYPE_STRU;
#endif /* __STARS_SIMPLE_SQ0_C_DEFINE_H__ */
