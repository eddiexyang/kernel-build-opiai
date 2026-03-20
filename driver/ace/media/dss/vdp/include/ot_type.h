/*
 * Minimal compatibility wrapper for legacy OT media headers.
 * The tree only carries the older HiDVPP base type headers.
 */
#ifndef OT_TYPE_H
#define OT_TYPE_H

#include "../../../../dvpp/dvpp/HiDvpp/cbb/include/hi_type.h"

typedef hi_char td_char;
typedef hi_s8 td_s8;
typedef hi_u8 td_u8;
typedef hi_s16 td_s16;
typedef hi_u16 td_u16;
typedef hi_s32 td_s32;
typedef hi_u32 td_u32;
typedef hi_s64 td_s64;
typedef hi_u64 td_u64;
typedef hi_ulong td_ulong;
typedef hi_float td_float;
typedef hi_double td_double;
typedef hi_bool td_bool;
typedef hi_void td_void;
typedef hi_phys_addr_t td_phys_addr_t;
typedef hi_uintptr_t td_uintptr_t;

#define TD_TRUE HI_TRUE
#define TD_FALSE HI_FALSE
#define TD_SUCCESS HI_SUCCESS
#define TD_FAILURE HI_FAILURE
#define TD_NULL HI_NULL

#ifndef ot_unused
#define ot_unused(x) ((void)(x))
#endif

#endif
