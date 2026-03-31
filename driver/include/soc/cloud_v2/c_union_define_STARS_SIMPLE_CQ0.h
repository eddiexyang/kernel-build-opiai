// ****************************************************************************** 
// Copyright     :  Copyright (C) 2020, Hisilicon Technologies Co. Ltd.
// File name     :  c_union_define_STARS_SIMPLE_CQ0.h
// Project line  :  Platform And Key Technologies Development
// Department    :  CAD Development Department
// Author        :  xxx
// Version       :  1
// Date          :  2020/04/01
// Description   :  The description of xxx project
// Others        :  Generated automatically by nManager V4.2 
// History       :  xxx 2020/10/21 10:07:13 Create file
// ******************************************************************************

#ifndef __C_UNION_DEFINE_STARS_SIMPLE_CQ0_H__
#define __C_UNION_DEFINE_STARS_SIMPLE_CQ0_H__

/* Define the union U_STARS_P0_CQ_DB */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
        unsigned int    p0_cq_head            : 16  ; /* [15..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_STARS_P0_CQ_DB;

/* Define the union U_STARS_P0_CQ_CFG2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
        unsigned int    p0_cq_tail            : 16  ; /* [15..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_STARS_P0_CQ_CFG2;

/* Define the union U_STARS_P0_CQ_CFG3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    reserved_1            : 1   ; /* [1]  */
        unsigned int    p0_cq_phase_bit       : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_STARS_P0_CQ_CFG3;

//==============================================================================
/* Define the global struct */
typedef struct
{
    volatile U_STARS_P0_CQ_DB       STARS_P0_CQ_DB[128];
    volatile U_STARS_P0_CQ_CFG2     STARS_P0_CQ_CFG2[128];
    volatile U_STARS_P0_CQ_CFG3     STARS_P0_CQ_CFG3[128];

} S_STARS_SIMPLE_CQ0_REGS_TYPE;

/* Declare the struct pointor of the module STARS_SIMPLE_CQ0 */
extern volatile S_STARS_SIMPLE_CQ0_REGS_TYPE *gopSTARS_SIMPLE_CQ0AllReg;

/* Declare the functions that set the member value */
int iSetSTARS_P0_CQ_DBp0_cq_head(unsigned int *upp0_cq_head);


#endif /* __C_UNION_DEFINE_STARS_SIMPLE_CQ0_H__ */
