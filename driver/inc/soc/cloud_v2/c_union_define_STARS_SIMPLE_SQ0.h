// ****************************************************************************** 
// Copyright     :  Copyright (C) 2020, Hisilicon Technologies Co. Ltd.
// File name     :  c_union_define_STARS_SIMPLE_SQ0.h
// Project line  :  Platform And Key Technologies Development
// Department    :  CAD Development Department
// Author        :  xxx
// Version       :  1
// Date          :  2020/04/01
// Description   :  The description of xxx project
// Others        :  Generated automatically by nManager V4.2 
// History       :  xxx 2020/10/21 09:06:20 Create file
// ******************************************************************************

#ifndef __C_UNION_DEFINE_STARS_SIMPLE_SQ0_H__
#define __C_UNION_DEFINE_STARS_SIMPLE_SQ0_H__

/* Define the union U_STARS_P0_SQ_DB */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
        unsigned int    p0_sq_tail            : 16  ; /* [15..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_STARS_P0_SQ_DB;

/* Define the union U_STARS_P0_SQ_CFG6 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    p0_sq_host_id         : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_STARS_P0_SQ_CFG6;

/* Define the union U_STARS_P0_SQ_CFG4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
        unsigned int    p0_sq_head            : 16  ; /* [15..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_STARS_P0_SQ_CFG4;

/* Define the union U_STARS_P0_SQ_CFG5 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 1   ; /* [31]  */
        unsigned int    p0_sq_priority        : 3   ; /* [30..28]  */
        unsigned int    reserved_1            : 12  ; /* [27..16]  */
        unsigned int    p0_sq_model_id        : 6   ; /* [15..10]  */
        unsigned int    reserved_2            : 2   ; /* [9..8]  */
        unsigned int    reserved_3            : 3   ; /* [7..5]  */
        unsigned int    p0_sq_vmid            : 4   ; /* [4..1]  */
        unsigned int    p0_sq_en              : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_STARS_P0_SQ_CFG5;

/* Define the union U_STARS_P0_TASK_CTRL0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 12  ; /* [31..20]  */
        unsigned int    p0_task_terminate     : 1   ; /* [19]  */
        unsigned int    p0_exception_handled  : 1   ; /* [18]  */
        unsigned int    p0_task_resume        : 1   ; /* [17]  */
        unsigned int    p0_debug_resume       : 1   ; /* [16]  */
        unsigned int    reserved_1            : 13  ; /* [15..3]  */
        unsigned int    p0_task_kill          : 1   ; /* [2]  */
        unsigned int    p0_task_pause         : 1   ; /* [1]  */
        unsigned int    p0_debug_pause        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_STARS_P0_TASK_CTRL0;

/* Define the union U_STARS_P0_TASK_CTRL1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 14  ; /* [31..18]  */
        unsigned int    reserved_1            : 1   ; /* [17]  */
        unsigned int    reserved_2            : 1   ; /* [16]  */
        unsigned int    reserved_3            : 14  ; /* [15..2]  */
        unsigned int    p0_prefetch_clear     : 1   ; /* [1]  */
        unsigned int    reserved_4            : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_STARS_P0_TASK_CTRL1;

/* Define the union U_P0_SQ_TO_ACTIVE_SQ_MAP0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 25  ; /* [31..7]  */
        unsigned int    p0_mapped_in_active_sqid : 7   ; /* [6..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_P0_SQ_TO_ACTIVE_SQ_MAP0;

/* Define the union U_P0_SQ_TO_ACTIVE_SQ_MAP1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    p0_mapped_in_active_set : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_P0_SQ_TO_ACTIVE_SQ_MAP1;

/* Define the union U_STARS_SQ_SEC_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    stars_sq_sec_en       : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_STARS_SQ_SEC_EN;

//==============================================================================
/* Define the global struct */
typedef struct
{
    volatile U_STARS_P0_SQ_DB       STARS_P0_SQ_DB[2048];
    volatile U_STARS_P0_SQ_CFG6     STARS_P0_SQ_CFG6[2048];
    volatile U_STARS_P0_SQ_CFG4     STARS_P0_SQ_CFG4[2048];
    volatile U_STARS_P0_SQ_CFG5     STARS_P0_SQ_CFG5[2048];
    volatile U_STARS_P0_TASK_CTRL0  STARS_P0_TASK_CTRL0[2048];
    volatile U_STARS_P0_TASK_CTRL1  STARS_P0_TASK_CTRL1[2048];
    volatile unsigned int           STARS_P0_SQ_COND0[2048];
    volatile unsigned int           STARS_P0_SQ_COND1[2048];
    volatile U_P0_SQ_TO_ACTIVE_SQ_MAP0 P0_SQ_TO_ACTIVE_SQ_MAP0[2048];
    volatile U_P0_SQ_TO_ACTIVE_SQ_MAP1 P0_SQ_TO_ACTIVE_SQ_MAP1[2048];
    volatile U_STARS_SQ_SEC_EN      STARS_SQ_SEC_EN[2048];

} S_STARS_SIMPLE_SQ0_REGS_TYPE;

/* Declare the struct pointor of the module STARS_SIMPLE_SQ0 */
extern volatile S_STARS_SIMPLE_SQ0_REGS_TYPE *gopSTARS_SIMPLE_SQ0AllReg;

/* Declare the functions that set the member value */
int iSetSTARS_P0_SQ_CFG6p0_sq_host_id(unsigned int *upp0_sq_host_id);
int iSetSTARS_P0_SQ_CFG5p0_sq_priority(unsigned int *upp0_sq_priority);
int iSetSTARS_P0_SQ_CFG5p0_sq_model_id(unsigned int *upp0_sq_model_id);
int iSetSTARS_P0_SQ_CFG5p0_sq_vmid(unsigned int *upp0_sq_vmid);
int iSetSTARS_P0_SQ_CFG5p0_sq_en(unsigned int *upp0_sq_en);
int iSetSTARS_SQ_SEC_ENstars_sq_sec_en(unsigned int *upstars_sq_sec_en);


#endif /* __C_UNION_DEFINE_STARS_SIMPLE_SQ0_H__ */
