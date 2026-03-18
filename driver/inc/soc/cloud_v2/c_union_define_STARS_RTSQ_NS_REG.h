// ****************************************************************************** 
// Copyright     :  Copyright (C) 2020, Hisilicon Technologies Co. Ltd.
// File name     :  c_union_define_STARS_RTSQ_NS_REG.h
// Project line  :  Platform And Key Technologies Development
// Department    :  CAD Development Department
// Author        :  xxx
// Version       :  1
// Date          :  2020/04/01
// Description   :  The description of xxx project
// Others        :  Generated automatically by nManager V4.2 
// History       :  xxx 2020/10/21 09:06:20 Create file
// ******************************************************************************

#ifndef __C_UNION_DEFINE_STARS_RTSQ_NS_REG_H__
#define __C_UNION_DEFINE_STARS_RTSQ_NS_REG_H__

/* Define the union U_STARS_PRIORITY_CTRL0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
        unsigned int    reserved_1            : 5   ; /* [15..11]  */
        unsigned int    sp_level              : 3   ; /* [10..8]  */
        unsigned int    reserved_2            : 5   ; /* [7..3]  */
        unsigned int    lp_mode               : 3   ; /* [2..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_STARS_PRIORITY_CTRL0;

/* Define the union U_STARS_RTSQ_AXCACHE_SETTING */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 3   ; /* [31..29]  */
        unsigned int    reserved_1            : 1   ; /* [28]  */
        unsigned int    reserved_2            : 4   ; /* [27..24]  */
        unsigned int    reserved_3            : 3   ; /* [23..21]  */
        unsigned int    arsnoop_swapbuf       : 1   ; /* [20]  */
        unsigned int    arcache_swapbuf       : 4   ; /* [19..16]  */
        unsigned int    reserved_4            : 3   ; /* [15..13]  */
        unsigned int    reserved_5            : 1   ; /* [12]  */
        unsigned int    reserved_6            : 4   ; /* [11..8]  */
        unsigned int    reserved_7            : 3   ; /* [7..5]  */
        unsigned int    reserved_8            : 1   ; /* [4]  */
        unsigned int    reserved_9            : 4   ; /* [3..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_STARS_RTSQ_AXCACHE_SETTING;

/* Define the union U_STARS_NS_SQ_SWAP_BUF_BASE_ADDR_CFG1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ns_sq_swap_buf_is_virtual : 1   ; /* [31]  */
        unsigned int    reserved_0            : 8   ; /* [30..23]  */
        unsigned int    ns_sq_swap_buf_shift  : 6   ; /* [22..17]  */
        unsigned int    ns_sq_swap_buf_base_addr_h : 17  ; /* [16..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_STARS_NS_SQ_SWAP_BUF_BASE_ADDR_CFG1;

/* Define the union U_STARS_RTSQ_FSM_SEL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 21  ; /* [31..11]  */
        unsigned int    dfx_rtsq_fsm_sel      : 11  ; /* [10..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_STARS_RTSQ_FSM_SEL;

/* Define the union U_STARS_RTSQ_FSM_STATE */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
        unsigned int    dfx_rtsq_fwrsp_ost    : 1   ; /* [4]  */
        unsigned int    dfx_rtsq_fsm_state    : 4   ; /* [3..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_STARS_RTSQ_FSM_STATE;

/* Define the union U_STARS_SWAPIN_CTRL0_NS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 1   ; /* [31]  */
        unsigned int    rtsq_swapin_max_sdma_ns : 7   ; /* [30..24]  */
        unsigned int    reserved_1            : 1   ; /* [23]  */
        unsigned int    rtsq_swapin_max_aicpu_ns : 7   ; /* [22..16]  */
        unsigned int    reserved_2            : 1   ; /* [15]  */
        unsigned int    rtsq_swapin_max_aiv_ns : 7   ; /* [14..8]  */
        unsigned int    reserved_3            : 1   ; /* [7]  */
        unsigned int    rtsq_swapin_max_aic_ns : 7   ; /* [6..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_STARS_SWAPIN_CTRL0_NS;

/* Define the union U_STARS_SWAPIN_CTRL1_NS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 1   ; /* [31]  */
        unsigned int    rtsq_swapin_max_dsa_ns : 7   ; /* [30..24]  */
        unsigned int    reserved_1            : 1   ; /* [23]  */
        unsigned int    rtsq_swapin_max_jpege_ns : 7   ; /* [22..16]  */
        unsigned int    reserved_2            : 1   ; /* [15]  */
        unsigned int    rtsq_swapin_max_jpegd_ns : 7   ; /* [14..8]  */
        unsigned int    reserved_3            : 1   ; /* [7]  */
        unsigned int    rtsq_swapin_max_vpc_ns : 7   ; /* [6..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_STARS_SWAPIN_CTRL1_NS;

/* Define the union U_STARS_SWAPIN_CTRL2_NS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 1   ; /* [31]  */
        unsigned int    reserved_1            : 7   ; /* [30..24]  */
        unsigned int    reserved_2            : 1   ; /* [23]  */
        unsigned int    reserved_3            : 7   ; /* [22..16]  */
        unsigned int    reserved_4            : 1   ; /* [15]  */
        unsigned int    reserved_5            : 7   ; /* [14..8]  */
        unsigned int    reserved_6            : 1   ; /* [7]  */
        unsigned int    rtsq_swapin_max_hcpu_ns : 7   ; /* [6..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_STARS_SWAPIN_CTRL2_NS;

//==============================================================================
/* Define the global struct */
typedef struct
{
    volatile U_STARS_PRIORITY_CTRL0 STARS_PRIORITY_CTRL0;
    volatile unsigned int           STARS_WAIT_TIMEOUT_CTRL0;
    volatile unsigned int           STARS_WAIT_TIMEOUT_CTRL1;
    volatile U_STARS_RTSQ_AXCACHE_SETTING STARS_RTSQ_AXCACHE_SETTING;
    volatile unsigned int           STARS_NS_SQ_SWAP_BUF_BASE_ADDR_CFG0;
    volatile U_STARS_NS_SQ_SWAP_BUF_BASE_ADDR_CFG1 STARS_NS_SQ_SWAP_BUF_BASE_ADDR_CFG1;
    volatile U_STARS_RTSQ_FSM_SEL   STARS_RTSQ_FSM_SEL;
    volatile U_STARS_RTSQ_FSM_STATE STARS_RTSQ_FSM_STATE;
    volatile U_STARS_SWAPIN_CTRL0_NS STARS_SWAPIN_CTRL0_NS[8];
    volatile U_STARS_SWAPIN_CTRL1_NS STARS_SWAPIN_CTRL1_NS[8];
    volatile U_STARS_SWAPIN_CTRL2_NS STARS_SWAPIN_CTRL2_NS[8];

} S_STARS_RTSQ_NS_REG_REGS_TYPE;

/* Declare the struct pointor of the module STARS_RTSQ_NS_REG */
extern volatile S_STARS_RTSQ_NS_REG_REGS_TYPE *gopSTARS_RTSQ_NS_REGAllReg;

/* Declare the functions that set the member value */
int iSetSTARS_PRIORITY_CTRL0sp_level(unsigned int usp_level);
int iSetSTARS_WAIT_TIMEOUT_CTRL0wait_task_runtime_limit_l(unsigned int uwait_task_runtime_limit_l);
int iSetSTARS_WAIT_TIMEOUT_CTRL1wait_task_runtime_limit_h(unsigned int uwait_task_runtime_limit_h);
int iSetSTARS_RTSQ_AXCACHE_SETTINGarsnoop_swapbuf(unsigned int uarsnoop_swapbuf);
int iSetSTARS_RTSQ_AXCACHE_SETTINGarcache_swapbuf(unsigned int uarcache_swapbuf);
int iSetSTARS_NS_SQ_SWAP_BUF_BASE_ADDR_CFG0ns_sq_swap_buf_base_addr_l(unsigned int uns_sq_swap_buf_base_addr_l);
int iSetSTARS_NS_SQ_SWAP_BUF_BASE_ADDR_CFG1ns_sq_swap_buf_is_virtual(unsigned int uns_sq_swap_buf_is_virtual);
int iSetSTARS_NS_SQ_SWAP_BUF_BASE_ADDR_CFG1ns_sq_swap_buf_shift(unsigned int uns_sq_swap_buf_shift);
int iSetSTARS_NS_SQ_SWAP_BUF_BASE_ADDR_CFG1ns_sq_swap_buf_base_addr_h(unsigned int uns_sq_swap_buf_base_addr_h);
int iSetSTARS_RTSQ_FSM_SELdfx_rtsq_fsm_sel(unsigned int udfx_rtsq_fsm_sel);
int iSetSTARS_SWAPIN_CTRL0_NSrtsq_swapin_max_sdma_ns(unsigned int *uprtsq_swapin_max_sdma_ns);
int iSetSTARS_SWAPIN_CTRL0_NSrtsq_swapin_max_aicpu_ns(unsigned int *uprtsq_swapin_max_aicpu_ns);
int iSetSTARS_SWAPIN_CTRL0_NSrtsq_swapin_max_aiv_ns(unsigned int *uprtsq_swapin_max_aiv_ns);
int iSetSTARS_SWAPIN_CTRL0_NSrtsq_swapin_max_aic_ns(unsigned int *uprtsq_swapin_max_aic_ns);
int iSetSTARS_SWAPIN_CTRL1_NSrtsq_swapin_max_dsa_ns(unsigned int *uprtsq_swapin_max_dsa_ns);
int iSetSTARS_SWAPIN_CTRL1_NSrtsq_swapin_max_jpege_ns(unsigned int *uprtsq_swapin_max_jpege_ns);
int iSetSTARS_SWAPIN_CTRL1_NSrtsq_swapin_max_jpegd_ns(unsigned int *uprtsq_swapin_max_jpegd_ns);
int iSetSTARS_SWAPIN_CTRL1_NSrtsq_swapin_max_vpc_ns(unsigned int *uprtsq_swapin_max_vpc_ns);
int iSetSTARS_SWAPIN_CTRL2_NSrtsq_swapin_max_hcpu_ns(unsigned int *uprtsq_swapin_max_hcpu_ns);


#endif /* __C_UNION_DEFINE_STARS_RTSQ_NS_REG_H__ */
