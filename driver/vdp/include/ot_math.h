/*
 * ot_math.h - OT math definitions (mediabase compatibility layer)
 *
 * Thin wrapper that includes the HiSilicon math header.
 * hi_math.h already defines ALIGN_UP, ALIGN_DOWN, MAX2, MIN2, DIV_UP, etc.
 */

#ifndef OT_MATH_H
#define OT_MATH_H

#include "hi_math.h"

/* hi_math.h provides: MAX2, MIN2, ALIGN_DOWN, DIV_UP, HI_ALIGN_UP, etc.
 * Add ALIGN_UP alias if not already defined. */
#ifndef ALIGN_UP
#define ALIGN_UP(x, a)  HI_ALIGN_UP(x, a)
#endif


/* 64-bit physical address split helpers (from hi_mpp_sample ot_math.h) */
static inline td_u32 get_low_addr(td_phys_addr_t phys_addr)
{
    return (td_u32)phys_addr;
}

static inline td_u32 get_high_addr(td_phys_addr_t phys_addr)
{
    return (td_u32)(phys_addr >> 32);
}

#endif /* OT_MATH_H */
