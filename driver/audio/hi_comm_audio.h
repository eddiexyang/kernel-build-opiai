/*
 * Historical audio common aggregation header.
 * The original build consumed this public header via the media SDK tree.
 * We reconstruct that aggregation locally and keep it on top of the
 * source-equivalent inner headers already restored in this module tree.
 */

#ifndef HI_COMM_AUDIO_H
#define HI_COMM_AUDIO_H

#include "autoconf.h"
#include "hi_common.h"
#include "hi_comm_vb.h"
#include "hi_comm_sys_adapt.h"
#include "ot_common.h"
#include "ot_defines.h"
#include "ot_errno.h"

#define ot_mpp_bind_dst hi_mpp_bind_dest
#define ot_payload_type hi_payload_type
#define ot_ai_chn hi_ai_chn
#define ot_ao_chn hi_ao_chn
#define ot_aenc_chn hi_aenc_chn
#define ot_adec_chn hi_adec_chn
#define ot_audio_dev hi_audio_dev

#include "ot_inner_common_aio.h"
#include "ot_inner_common_aenc.h"
#include "ot_inner_common_adec.h"

#ifndef OT_VB_UID_USER
#define OT_VB_UID_USER VB_UID_USER
#endif
#ifndef OT_VB_UID_AI
#define OT_VB_UID_AI   VB_UID_AI
#endif
#ifndef OT_VB_MAX_USER
#define OT_VB_MAX_USER VB_MAX_USER
#endif
#ifndef OT_VB_INVALID_HANDLE
#define OT_VB_INVALID_HANDLE VB_INVALID_HANDLE
#endif
#ifndef OT_VB_INVALID_POOL_ID
#define OT_VB_INVALID_POOL_ID VB_INVALID_POOLID
#endif

#endif
