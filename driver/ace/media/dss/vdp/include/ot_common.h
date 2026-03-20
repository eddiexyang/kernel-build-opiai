/*
 * Minimal compatibility wrapper for legacy OT media headers.
 */
#ifndef OT_COMMON_H
#define OT_COMMON_H

#include "ot_defines.h"
#include "../../../../dvpp/dvpp/HiDvpp/cbb/include/hi_common.h"
#include "../../../../dvpp/dvpp/HiDvpp/cbb/include/adapt/hi_common_adapt.h"
#include "ot_type.h"

typedef MOD_ID_E ot_mod_id;
typedef VO_DEV ot_vo_dev;
typedef GRAPHIC_LAYER ot_gfx_layer;
typedef hi_mpp_chn ot_mpp_chn;

#define OT_MPP_VERSION MPP_VERSION

#endif
