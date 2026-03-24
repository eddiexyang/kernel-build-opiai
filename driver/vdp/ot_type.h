/* Stub ot_type.h - HiSilicon OT SDK base types */
#ifndef __OT_TYPE_H__
#define __OT_TYPE_H__

#include <linux/types.h>

typedef unsigned char      td_u8;
typedef unsigned short     td_u16;
typedef unsigned int       td_u32;
typedef unsigned long long td_u64;
typedef signed char        td_s8;
typedef signed short       td_s16;
typedef signed int         td_s32;
typedef signed long long   td_s64;
typedef char               td_char;
typedef unsigned long      td_ulong;
typedef unsigned long      td_phys_addr_t;
typedef void               td_void;
typedef bool               td_bool;
typedef unsigned long      td_handle;
typedef float              td_float;
typedef double             td_double;

#define TD_TRUE   1
#define TD_FALSE  0
#define TD_NULL   NULL
#define TD_SUCCESS 0
#define TD_FAILURE (-1)

typedef td_s32 ot_vo_dev;
typedef td_s32 ot_vo_layer;
typedef td_s32 ot_vo_chn;
typedef td_s32 ot_gfx_layer;

#endif
