// ****************************************************************************** 
// Copyright     :  Copyright (C) 2020, Hisilicon Technologies Co. Ltd.
// File name     :  c_union_define_STARS_RTSQ_S_REG.h
// Project line  :  Platform And Key Technologies Development
// Department    :  CAD Development Department
// Author        :  xxx
// Version       :  1
// Date          :  2020/04/01
// Description   :  The description of xxx project
// Others        :  Generated automatically by nManager V4.2 
// History       :  xxx 2020/10/21 09:06:20 Create file
// ******************************************************************************

#ifndef __C_UNION_DEFINE_STARS_RTSQ_S_REG_H__
#define __C_UNION_DEFINE_STARS_RTSQ_S_REG_H__

/* Define the union U_STARS_POOL_SEC */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 23  ; /* [31..9]  */
        unsigned int    bypass_sq_sec_err     : 1   ; /* [8]  */
        unsigned int    pool_sec              : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_STARS_POOL_SEC;

/* Define the union U_STARS_VMG_SEC */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 23  ; /* [31..9]  */
        unsigned int    bypass_vmg_sec_err    : 1   ; /* [8]  */
        unsigned int    vmg_sec               : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_STARS_VMG_SEC;

/* Define the union U_STARS_SWAPBUF_NS_AXPROT_SETTING3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 5   ; /* [31..27]  */
        unsigned int    reserved_1            : 3   ; /* [26..24]  */
        unsigned int    reserved_2            : 5   ; /* [23..19]  */
        unsigned int    ns_sq_arprot_swapbuf  : 3   ; /* [18..16]  */
        unsigned int    reserved_3            : 7   ; /* [15..9]  */
        unsigned int    reserved_4            : 9   ; /* [8..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_STARS_SWAPBUF_NS_AXPROT_SETTING3;

/* Define the union U_STARS_SWAPBUF_S_AXPROT_SETTING3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 5   ; /* [31..27]  */
        unsigned int    reserved_1            : 3   ; /* [26..24]  */
        unsigned int    reserved_2            : 5   ; /* [23..19]  */
        unsigned int    s_sq_arprot_swapbuf   : 3   ; /* [18..16]  */
        unsigned int    reserved_3            : 7   ; /* [15..9]  */
        unsigned int    reserved_4            : 1   ; /* [8]  */
        unsigned int    reserved_5            : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_STARS_SWAPBUF_S_AXPROT_SETTING3;

/* Define the union U_STARS_SWAPBUF_NS_SETTING1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 7   ; /* [31..25]  */
        unsigned int    reserved_1            : 1   ; /* [24]  */
        unsigned int    reserved_2            : 7   ; /* [23..17]  */
        unsigned int    arns_swapbuf          : 1   ; /* [16]  */
        unsigned int    reserved_3            : 7   ; /* [15..9]  */
        unsigned int    reserved_4            : 1   ; /* [8]  */
        unsigned int    reserved_5            : 7   ; /* [7..1]  */
        unsigned int    reserved_6            : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_STARS_SWAPBUF_NS_SETTING1;

/* Define the union U_STARS_SWAPIN_CTRL0_S */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 1   ; /* [31]  */
        unsigned int    rtsq_swapin_max_sdma_s : 7   ; /* [30..24]  */
        unsigned int    reserved_1            : 1   ; /* [23]  */
        unsigned int    rtsq_swapin_max_aicpu_s : 7   ; /* [22..16]  */
        unsigned int    reserved_2            : 1   ; /* [15]  */
        unsigned int    rtsq_swapin_max_aiv_s : 7   ; /* [14..8]  */
        unsigned int    reserved_3            : 1   ; /* [7]  */
        unsigned int    rtsq_swapin_max_aic_s : 7   ; /* [6..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_STARS_SWAPIN_CTRL0_S;

/* Define the union U_STARS_SWAPIN_CTRL1_S */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 1   ; /* [31]  */
        unsigned int    rtsq_swapin_max_dsa_s : 7   ; /* [30..24]  */
        unsigned int    reserved_1            : 1   ; /* [23]  */
        unsigned int    rtsq_swapin_max_jpege_s : 7   ; /* [22..16]  */
        unsigned int    reserved_2            : 1   ; /* [15]  */
        unsigned int    rtsq_swapin_max_jpegd_s : 7   ; /* [14..8]  */
        unsigned int    reserved_3            : 1   ; /* [7]  */
        unsigned int    rtsq_swapin_max_vpc_s : 7   ; /* [6..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_STARS_SWAPIN_CTRL1_S;

/* Define the union U_STARS_SWAPIN_CTRL2_S */
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
        unsigned int    rtsq_swapin_max_hcpu_s : 7   ; /* [6..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_STARS_SWAPIN_CTRL2_S;

/* Define the union U_STARS_S_SQ_SWAP_BUF_BASE_ADDR_CFG1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    s_sq_swap_buf_is_virtual : 1   ; /* [31]  */
        unsigned int    reserved_0            : 8   ; /* [30..23]  */
        unsigned int    s_sq_swap_buf_shift   : 6   ; /* [22..17]  */
        unsigned int    s_sq_swap_buf_base_addr_h : 17  ; /* [16..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_STARS_S_SQ_SWAP_BUF_BASE_ADDR_CFG1;

//==============================================================================
/* Define the global struct */
typedef struct
{
    volatile U_STARS_POOL_SEC       STARS_POOL_SEC;
    volatile U_STARS_VMG_SEC        STARS_VMG_SEC;
    volatile U_STARS_SWAPBUF_NS_AXPROT_SETTING3 STARS_SWAPBUF_NS_AXPROT_SETTING3;
    volatile U_STARS_SWAPBUF_S_AXPROT_SETTING3 STARS_SWAPBUF_S_AXPROT_SETTING3;
    volatile U_STARS_SWAPBUF_NS_SETTING1 STARS_SWAPBUF_NS_SETTING1;
    volatile U_STARS_SWAPIN_CTRL0_S STARS_SWAPIN_CTRL0_S[8];
    volatile U_STARS_SWAPIN_CTRL1_S STARS_SWAPIN_CTRL1_S[8];
    volatile U_STARS_SWAPIN_CTRL2_S STARS_SWAPIN_CTRL2_S[8];
    volatile unsigned int           STARS_S_SQ_SWAP_BUF_BASE_ADDR_CFG0;
    volatile U_STARS_S_SQ_SWAP_BUF_BASE_ADDR_CFG1 STARS_S_SQ_SWAP_BUF_BASE_ADDR_CFG1;

} S_STARS_RTSQ_S_REG_REGS_TYPE;

/* Declare the struct pointor of the module STARS_RTSQ_S_REG */
extern volatile S_STARS_RTSQ_S_REG_REGS_TYPE *gopSTARS_RTSQ_S_REGAllReg;

/* Declare the functions that set the member value */
int iSetSTARS_POOL_SECbypass_sq_sec_err(unsigned int ubypass_sq_sec_err);
int iSetSTARS_POOL_SECpool_sec(unsigned int upool_sec);
int iSetSTARS_VMG_SECbypass_vmg_sec_err(unsigned int ubypass_vmg_sec_err);
int iSetSTARS_VMG_SECvmg_sec(unsigned int uvmg_sec);
int iSetSTARS_SWAPBUF_NS_AXPROT_SETTING3ns_sq_arprot_swapbuf(unsigned int uns_sq_arprot_swapbuf);
int iSetSTARS_SWAPBUF_S_AXPROT_SETTING3s_sq_arprot_swapbuf(unsigned int us_sq_arprot_swapbuf);
int iSetSTARS_SWAPBUF_NS_SETTING1arns_swapbuf(unsigned int uarns_swapbuf);
int iSetSTARS_SWAPIN_CTRL0_Srtsq_swapin_max_sdma_s(unsigned int *uprtsq_swapin_max_sdma_s);
int iSetSTARS_SWAPIN_CTRL0_Srtsq_swapin_max_aicpu_s(unsigned int *uprtsq_swapin_max_aicpu_s);
int iSetSTARS_SWAPIN_CTRL0_Srtsq_swapin_max_aiv_s(unsigned int *uprtsq_swapin_max_aiv_s);
int iSetSTARS_SWAPIN_CTRL0_Srtsq_swapin_max_aic_s(unsigned int *uprtsq_swapin_max_aic_s);
int iSetSTARS_SWAPIN_CTRL1_Srtsq_swapin_max_dsa_s(unsigned int *uprtsq_swapin_max_dsa_s);
int iSetSTARS_SWAPIN_CTRL1_Srtsq_swapin_max_jpege_s(unsigned int *uprtsq_swapin_max_jpege_s);
int iSetSTARS_SWAPIN_CTRL1_Srtsq_swapin_max_jpegd_s(unsigned int *uprtsq_swapin_max_jpegd_s);
int iSetSTARS_SWAPIN_CTRL1_Srtsq_swapin_max_vpc_s(unsigned int *uprtsq_swapin_max_vpc_s);
int iSetSTARS_SWAPIN_CTRL2_Srtsq_swapin_max_hcpu_s(unsigned int *uprtsq_swapin_max_hcpu_s);
int iSetSTARS_S_SQ_SWAP_BUF_BASE_ADDR_CFG0s_sq_swap_buf_base_addr_l(unsigned int us_sq_swap_buf_base_addr_l);
int iSetSTARS_S_SQ_SWAP_BUF_BASE_ADDR_CFG1s_sq_swap_buf_is_virtual(unsigned int us_sq_swap_buf_is_virtual);
int iSetSTARS_S_SQ_SWAP_BUF_BASE_ADDR_CFG1s_sq_swap_buf_shift(unsigned int us_sq_swap_buf_shift);
int iSetSTARS_S_SQ_SWAP_BUF_BASE_ADDR_CFG1s_sq_swap_buf_base_addr_h(unsigned int us_sq_swap_buf_base_addr_h);


#endif /* __C_UNION_DEFINE_STARS_RTSQ_S_REG_H__ */
