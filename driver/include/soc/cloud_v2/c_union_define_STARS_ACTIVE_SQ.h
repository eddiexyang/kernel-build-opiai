// ****************************************************************************** 
// Copyright     :  Copyright (C) 2021, Hisilicon Technologies Co. Ltd.
// File name     :  c_union_define.h
// Project line  :  Platform And Key Technologies Development
// Department    :  CAD Development Department
// Author        :  xxx
// Version       :  1
// Date          :  2020/04/01
// Description   :  The description of xxx project
// Others        :  Generated automatically by nManager V4.2 
// History       :  xxx 2021/06/04 16:12:21 Create file
// ******************************************************************************

#ifndef __C_UNION_DEFINE_H__
#define __C_UNION_DEFINE_H__

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
        unsigned int    reserved_2            : 3   ; /* [3..1]  */
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
        unsigned int    reserved_0            : 2   ; /* [15..14]  */
        unsigned int    sq_cqid               : 11  ; /* [26..16]  */
        unsigned int    sq_log_buffid         : 5   ; /* [31..27]  */
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
        unsigned int    sq_vpc_poolid         : 3   ; /* [2..0]  */
        unsigned int    sq_jpegd_poolid       : 3   ; /* [5..3]  */
        unsigned int    sq_jpege_poolid       : 3   ; /* [8..6]  */
        unsigned int    sq_sdma_poolid        : 3   ; /* [11..9]  */
        unsigned int    sq_aicpu_poolid       : 3   ; /* [14..12]  */
        unsigned int    sq_pciedma_poolid     : 3   ; /* [17..15]  */
        unsigned int    sq_conds_poolid       : 3   ; /* [20..18]  */
        unsigned int    sq_dsa_poolid         : 3   ; /* [23..21]  */
        unsigned int    sq_aic_poolid         : 3   ; /* [26..24]  */
        unsigned int    sq_vec_poolid         : 3   ; /* [29..27]  */
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
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
        unsigned int    sq_ffts_poolid        : 3   ; /* [27..25]  */
        unsigned int    reserved_0            : 4   ; /* [31..28]  */
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

/* Define the union U_SQCQ_FSM_IP_OWN_STATE_2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    ip_own_bitmap_2       : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_SQCQ_FSM_IP_OWN_STATE_2;

/* Define the union U_SQCQ_FSM_IP_DEBUG_STATE_2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    ip_debug_bitmap_2     : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_SQCQ_FSM_IP_DEBUG_STATE_2;

/* Define the union U_SQCQ_FSM_IP_EXCEPTION_STATE_2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    ip_exception_bitmap_2 : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_SQCQ_FSM_IP_EXCEPTION_STATE_2;

/* Define the union U_SQCQ_FSM_IP_TRAP_STATE_2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    ip_trap_bitmap_2      : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_SQCQ_FSM_IP_TRAP_STATE_2;

/* Define the union U_SQCQ_FSM_IP_DONE_STATE_2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    ip_done_bitmap_2      : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_SQCQ_FSM_IP_DONE_STATE_2;

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
        unsigned int    reserved_1            : 2   ; /* [7..6]  */
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
        unsigned int    reserved_0            : 21  ; /* [31..11]  */
        unsigned int    active_to_sqid_map    : 11  ; /* [10..0]  */
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
    volatile unsigned int           STARS_SQ_BASE_ADDR0[128];
    volatile U_STARS_SQ_BASE_ADDR1  STARS_SQ_BASE_ADDR1[128];
    volatile U_STARS_SQ_CFG0        STARS_SQ_CFG0[128];
    volatile U_STARS_SQ_CFG1        STARS_SQ_CFG1[128];
    volatile U_STARS_SQ_CFG2        STARS_SQ_CFG2[128];
    volatile unsigned int           STARS_SQ_CFG3[128];
    volatile U_STARS_SQ_CFG4        STARS_SQ_CFG4[128];
    volatile U_STARS_SQ_CFG5        STARS_SQ_CFG5[128];
    volatile unsigned int           STARS_SQ_CFG6[128];
    volatile U_STARS_SQ_CFG7        STARS_SQ_CFG7[128];
    volatile unsigned int           STARS_SQ_SW_STATUS[128];
    volatile unsigned int           STARS_CQ_BASE_ADDR0[128];
    volatile U_STARS_CQ_BASE_ADDR1  STARS_CQ_BASE_ADDR1[128];
    volatile U_STARS_CQ_CFG0        STARS_CQ_CFG0[128];
    volatile unsigned int           SQCQ_FSM_IP_OWN_STATE_0[128];
    volatile unsigned int           SQCQ_FSM_IP_OWN_STATE_1[128];
    volatile U_SQCQ_FSM_IP_OWN_STATE_2 SQCQ_FSM_IP_OWN_STATE_2[128];
    volatile unsigned int           SQCQ_FSM_IP_DEBUG_STATE_0[128];
    volatile unsigned int           SQCQ_FSM_IP_DEBUG_STATE_1[128];
    volatile U_SQCQ_FSM_IP_DEBUG_STATE_2 SQCQ_FSM_IP_DEBUG_STATE_2[128];
    volatile unsigned int           SQCQ_FSM_IP_EXCEPTION_STATE_0[128];
    volatile unsigned int           SQCQ_FSM_IP_EXCEPTION_STATE_1[128];
    volatile U_SQCQ_FSM_IP_EXCEPTION_STATE_2 SQCQ_FSM_IP_EXCEPTION_STATE_2[128];
    volatile unsigned int           SQCQ_FSM_IP_TRAP_STATE_0[128];
    volatile unsigned int           SQCQ_FSM_IP_TRAP_STATE_1[128];
    volatile U_SQCQ_FSM_IP_TRAP_STATE_2 SQCQ_FSM_IP_TRAP_STATE_2[128];
    volatile unsigned int           SQCQ_FSM_IP_DONE_STATE_0[128];
    volatile unsigned int           SQCQ_FSM_IP_DONE_STATE_1[128];
    volatile U_SQCQ_FSM_IP_DONE_STATE_2 SQCQ_FSM_IP_DONE_STATE_2[128];
    volatile U_SQCQ_FSM_MISC_STATE0 SQCQ_FSM_MISC_STATE0[128];
    volatile U_SQCQ_FSM_MISC_STATE1 SQCQ_FSM_MISC_STATE1[128];
    volatile U_SQCQ_FSM_STATE0      SQCQ_FSM_STATE0[128];
    volatile U_SQCQ_FSM_STATE1      SQCQ_FSM_STATE1[128];
    volatile U_ACTIVE_TO_SQ_MAP0    ACTIVE_TO_SQ_MAP0[128];
    volatile U_ACTIVE_TO_SQ_MAP1    ACTIVE_TO_SQ_MAP1[128];
    volatile U_RERUN_USE_FRESH_SQE  RERUN_USE_FRESH_SQE[128];

} S_STARS_ACTIVE_SQ_REGS_TYPE;

/* Declare the struct pointor of the module STARS_ACTIVE_SQ */
extern volatile S_STARS_ACTIVE_SQ_REGS_TYPE *gopSTARS_ACTIVE_SQAllReg;

/* Declare the functions that set the member value */


#endif /* __C_UNION_DEFINE_H__ */
