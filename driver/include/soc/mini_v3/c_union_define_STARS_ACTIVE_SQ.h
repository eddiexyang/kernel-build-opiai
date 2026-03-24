// ****************************************************************************** 
// Copyright     :  Copyright (C) 2021, Hisilicon Technologies Co. Ltd.
// File name     :  c_union_define_STARS_ACTIVE_SQ.h
// Project line  :  Platform And Key Technologies Development
// Department    :  CAD Development Department
// Author        :  xxx
// Version       :  1
// Date          :  2020/04/01
// Description   :  The description of xxx project
// Others        :  Generated automatically by nManager V4.2 
// History       :  xxx 2021/10/23 10:09:50 Create file
// ******************************************************************************

#ifndef __C_UNION_DEFINE_STARS_ACTIVE_SQ_H__
#define __C_UNION_DEFINE_STARS_ACTIVE_SQ_H__

typedef unsigned int U_STARS_SQ_BASE_ADDR0;

/* Define the union U_STARS_SQ_BASE_ADDR1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sq_base_addr_h         : 17  ; /* [16..0]  */
        unsigned int    reserved_0             : 14  ; /* [30..17]  */
        unsigned int    sq_base_addr_is_virtual : 1  ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_STARS_SQ_BASE_ADDR1;

/* Define the union U_STARS_SQ_CFG0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sq_smmu_stream_id     : 16  ; /* [15..0]  */
        unsigned int    sq_smmu_substream_id  : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_STARS_SQ_CFG0;

/* Define the union U_STARS_SQ_CFG1 */
typedef union
{
    /* Define the struct bits */
    struct
    {

        unsigned int    sq_smmu_two_stage_en  : 1   ; /* [0]  */
        unsigned int    reserved_3            : 3   ; /* [3..1]  */
        unsigned int    sq_ban_wr_cqe         : 1   ; /* [4]  */
        unsigned int    sq_profile_en         : 1   ; /* [5]  */
        unsigned int    sq_block_log_en       : 1   ; /* [6]  */
        unsigned int    sq_log_en             : 1   ; /* [7]  */
        unsigned int    sq_loc_host           : 1   ; /* [8]  */
        unsigned int    atomic_sat_mode       : 1   ; /* [9]  */
        unsigned int    atomic_rsp_mode       : 1   ; /* [10]  */
        unsigned int    sq_tsubtask_log_en    : 1   ; /* [11]  */
        unsigned int    sq_prof_block_en      : 1   ; /* [12]  */
        unsigned int    sq_prof_subtask_en    : 1   ; /* [13]  */
        unsigned int    reserved_2            : 2   ; /* [15..14]  */
        unsigned int    sq_cqid               : 9   ; /* [24..16]  */
        unsigned int    reserved_1            : 2   ; /* [26..25]  */
        unsigned int    sq_log_buffid         : 4   ; /* [30..27]  */
        unsigned int    reserved_0            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_STARS_SQ_CFG1;

/* Define the union U_STARS_SQ_CFG2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sq_vpc_poolid         : 1   ; /* [0]  */
        unsigned int    reserved_9            : 2   ; /* [2..1]  */
        unsigned int    sq_jpegd_poolid       : 1   ; /* [3]  */
        unsigned int    reserved_8            : 2   ; /* [5..4]  */
        unsigned int    sq_jpege_poolid       : 1   ; /* [6]  */
        unsigned int    reserved_7            : 2   ; /* [8..7]  */
        unsigned int    sq_sdma_poolid        : 1   ; /* [9]  */
        unsigned int    reserved_6            : 2   ; /* [11..10]  */
        unsigned int    sq_aicpu_poolid       : 1   ; /* [12]  */
        unsigned int    reserved_5            : 2   ; /* [14..13]  */
        unsigned int    sq_pciedma_poolid     : 1   ; /* [15]  */
        unsigned int    reserved_4            : 2   ; /* [17..16]  */
        unsigned int    sq_conds_poolid       : 1   ; /* [18]  */
        unsigned int    reserved_3            : 2   ; /* [20..19]  */
        unsigned int    sq_dsa_poolid         : 1   ; /* [21]  */
        unsigned int    reserved_2            : 2   ; /* [23..22]  */
        unsigned int    sq_aic_poolid         : 1   ; /* [24]  */
        unsigned int    reserved_1            : 2   ; /* [26..25]  */
        unsigned int    sq_vec_poolid         : 1   ; /* [27]  */
        unsigned int    reserved_0            : 4   ; /* [31..28]  */
   } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_STARS_SQ_CFG2;

typedef unsigned int U_STARS_SQ_CFG3;

/* Define the union U_STARS_SQ_CFG4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sq_length             : 16  ; /* [15..0]  */
        unsigned int    sq_partid             : 8   ; /* [23..16]  */
        unsigned int    sq_ffts_desc_addr_virtual : 1   ; /* [24]  */
        unsigned int    sq_ffts_poolid        : 1   ; /* [25]  */
        unsigned int    reserved_0            : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_STARS_SQ_CFG4;

/* Define the union U_STARS_SQ_CFG5 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sq_cq_int_num               : 7   ; /* [6..0]  */
        unsigned int    sw_swsq_event_record_pause  : 1   ; /* [7]  */
        unsigned int    sw_swsq_event_wait_fail_int : 1   ; /* [8]  */
        unsigned int    sq_sqe_dest_pid_vld         : 1   ; /* [9]  */
        unsigned int    reserved_0                  : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_STARS_SQ_CFG5;

typedef unsigned int U_STARS_SQ_RESV;

typedef unsigned int  U_STARS_CQ_BASE_ADDR0;

typedef unsigned int U_STARS_SQ_CFG6;

/* Define the union U_STARS_SQ_CFG7 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cq_full_int_mask         : 2   ; /* [1..0]  */
        unsigned int    task_debug_int_mask      : 2   ; /* [3..2]  */
        unsigned int    sw_status_error_int_mask : 2   ; /* [5..4]  */
        unsigned int    sqe_error_int_mask       : 2   ; /* [7..6]  */
        unsigned int    task_trap_int_mask       : 2   ; /* [9..8]  */
        unsigned int    task_error_int_mask      : 2   ; /* [11..10]  */
        unsigned int    res_conflict_int_mask    : 2   ; /* [13..12]  */
        unsigned int    cqe_written_int_mask     : 2   ; /* [15..14]  */
        unsigned int    reserved_0               : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} U_STARS_SQ_CFG7;

/* Define the union U_STARS_CQ_BASE_ADDR1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cq_base_addr_h        : 17  ; /* [16..0]  */
        unsigned int    reserved_0            : 14  ; /* [30..17]  */
        unsigned int    cq_base_addr_is_virtual : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_STARS_CQ_BASE_ADDR1;

/* Define the union U_STARS_CQ_CFG0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cq_length             : 16  ; /* [15..0]  */
        unsigned int    cq_loc_host           : 1   ; /* [16]  */
        unsigned int    reserved_1            : 2   ; /* [18..17]  */
        unsigned int    reserved_0            : 13  ; /* [31..19]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_STARS_CQ_CFG0;

/* Define the union U_SQCQ_FSM_IP_DONE_STATE_0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 26  ; /* [31..6]  */
        unsigned int    ip_done_bitmap_vld    : 1   ; /* [5]  */
        unsigned int    reserved_1            : 2   ; /* [4..3]  */
        unsigned int    ip_done_bitmap_id_0   : 3   ; /* [2..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_SQCQ_FSM_IP_DONE_STATE_0;

/* Define the union U_SQCQ_FSM_MISC_STATE0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    blk_id                : 16  ; /* [31..16]  */
        unsigned int    reserved_0            : 10  ; /* [15..6]  */
        unsigned int    sqe_type              : 6   ; /* [5..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_SQCQ_FSM_MISC_STATE0;

/* Define the union U_SQCQ_FSM_MISC_STATE1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    kernel_credit         : 8   ; /* [31..24]  */
        unsigned int    reserved_0            : 8   ; /* [23..16]  */
        unsigned int    blk_dim               : 16  ; /* [15..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_SQCQ_FSM_MISC_STATE1;

/* Define the union U_SQCQ_FSM_STATE0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 19  ; /* [31..13]  */
        unsigned int    dfx_sqcq_fsm_cq_write_ost_cnt : 1   ; /* [12]  */
        unsigned int    dfx_sqcq_fsm_write_value_ost_cnt : 1   ; /* [11]  */
        unsigned int    dfx_sqcq_fsm_profile_ost_cnt : 1   ; /* [10]  */
        unsigned int    dfx_sqcq_fsm_task_log_ost_cnt : 2   ; /* [9..8]  */
        unsigned int    reserved_1            : 1   ; /* [7]  */
        unsigned int    dfx_sqcq_fsm_sending_vld : 1   ; /* [6]  */
        unsigned int    dfx_sqcq_fsm_state    : 6   ; /* [5..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_SQCQ_FSM_STATE0;

/* Define the union U_SQCQ_FSM_STATE1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    sq_prefetch_busy      : 1   ; /* [1]  */
        unsigned int    reserved_1            : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_SQCQ_FSM_STATE1;

/* Define the union U_ACTIVE_TO_SQ_MAP0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 23  ; /* [31..9]  */
        unsigned int    active_to_sqid_map    : 9   ; /* [8..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_ACTIVE_TO_SQ_MAP0;

/* Define the union U_ACTIVE_TO_SQ_MAP1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    sqid_valid            : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_ACTIVE_TO_SQ_MAP1;

/* Define the union U_RERUN_USE_FRESH_SQE */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    rerun_use_fresh_sqe   : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_RERUN_USE_FRESH_SQE;

//==============================================================================
/* Define the global struct */
typedef struct
{
    volatile unsigned int           STARS_SQ_BASE_ADDR0[32];
    volatile U_STARS_SQ_BASE_ADDR1  STARS_SQ_BASE_ADDR1[32];
    volatile U_STARS_SQ_CFG0        STARS_SQ_CFG0[32];
    volatile U_STARS_SQ_CFG1        STARS_SQ_CFG1[32];
    volatile U_STARS_SQ_CFG2        STARS_SQ_CFG2[32];
    volatile unsigned int           STARS_SQ_CFG3[32];
    volatile U_STARS_SQ_CFG4        STARS_SQ_CFG4[32];
    volatile U_STARS_SQ_CFG5        STARS_SQ_CFG5[32];
    volatile unsigned int           STARS_SQ_CFG6[32];
    volatile U_STARS_SQ_CFG7        STARS_SQ_CFG7[32];
    volatile unsigned int           STARS_SQ_SW_STATUS[32];
    volatile unsigned int           STARS_CQ_BASE_ADDR0[32];
    volatile U_STARS_CQ_BASE_ADDR1  STARS_CQ_BASE_ADDR1[32];
    volatile U_STARS_CQ_CFG0        STARS_CQ_CFG0[32];
    volatile unsigned int           SQCQ_FSM_IP_OWN_STATE_0[32];
    volatile unsigned int           SQCQ_FSM_IP_DEBUG_STATE_0[32];
    volatile unsigned int           SQCQ_FSM_IP_EXCEPTION_STATE_0[32];
    volatile unsigned int           SQCQ_FSM_IP_TRAP_STATE_0[32];
    volatile U_SQCQ_FSM_IP_DONE_STATE_0 SQCQ_FSM_IP_DONE_STATE_0[32];
    volatile U_SQCQ_FSM_MISC_STATE0 SQCQ_FSM_MISC_STATE0[32];
    volatile U_SQCQ_FSM_MISC_STATE1 SQCQ_FSM_MISC_STATE1[32];
    volatile U_SQCQ_FSM_STATE0      SQCQ_FSM_STATE0[32];
    volatile U_SQCQ_FSM_STATE1      SQCQ_FSM_STATE1[32];
    volatile U_ACTIVE_TO_SQ_MAP0    ACTIVE_TO_SQ_MAP0[32];
    volatile U_ACTIVE_TO_SQ_MAP1    ACTIVE_TO_SQ_MAP1[32];
    volatile U_RERUN_USE_FRESH_SQE  RERUN_USE_FRESH_SQE[32];

} S_STARS_ACTIVE_SQ_REGS_TYPE;

/* Declare the struct pointor of the module STARS_ACTIVE_SQ */
extern volatile S_STARS_ACTIVE_SQ_REGS_TYPE *gopSTARS_ACTIVE_SQAllReg;

/* Declare the functions that set the member value */


#endif /* __C_UNION_DEFINE_STARS_ACTIVE_SQ_H__ */
