/*
 * ot_type.h - OT type definitions (mediabase compatibility layer)
 *
 * This is a thin compatibility wrapper derived from IDA Pro reverse engineering
 * of ascend_vdp_hifb.ko. It maps td_xxx types to their hi_xxx equivalents
 * from the Huawei HiSilicon SDK.
 *
 * All type mappings have been precisely confirmed via binary analysis.
 */

#ifndef OT_TYPE_H
#define OT_TYPE_H

#include "hi_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/* Basic unsigned types */
typedef hi_u8           td_u8;
typedef hi_u16          td_u16;
typedef hi_u32          td_u32;
typedef hi_u64          td_u64;

/* Basic signed types */
typedef hi_s8           td_s8;
typedef hi_s16          td_s16;
typedef hi_s32          td_s32;
typedef hi_s64          td_s64;

/* Other basic types */
typedef hi_bool         td_bool;
typedef hi_void         td_void;
typedef hi_char         td_char;
typedef hi_ulong        td_ulong;
typedef hi_float        td_float;
typedef hi_double       td_double;

/* Address and pointer types */
typedef hi_phys_addr_t  td_phys_addr_t;
typedef hi_uintptr_t    td_uintptr_t;

/* Constants */
#define TD_TRUE         HI_TRUE
#define TD_FALSE        HI_FALSE
#define TD_SUCCESS      HI_SUCCESS
#define TD_FAILURE      HI_FAILURE
#define TD_NULL         HI_NULL

/* Utility macros */
#define ot_unused(x)    ((void)(x))

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* OT_TYPE_H */
