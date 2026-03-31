/*
 * ot_errno.h - OT error code definitions (mediabase compatibility layer)
 *
 * This is a thin compatibility wrapper derived from IDA Pro reverse engineering
 * of ascend_vdp_hifb.ko. The error code format is:
 *   0xA0000000 | (module_id << 16) | (level << 13) | error_id
 */

#ifndef OT_ERRNO_H
#define OT_ERRNO_H

#include "hi_errno.h"
#include "ot_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/*
 * Error code composition macro.
 * Format: 0xA0 | mod(8bit) | level(3bit) | err_id(13bit)
 */
#define OT_DEFINE_ERR(mod, level, err_id) \
    ((td_s32)(0xA0000000 | ((mod) << 16) | ((level) << 13) | (err_id)))

/* Error levels */
#define OT_ERR_LEVEL_DEBUG   0
#define OT_ERR_LEVEL_INFO    1
#define OT_ERR_LEVEL_NOTICE  2
#define OT_ERR_LEVEL_WARNING 3
#define OT_ERR_LEVEL_ERROR   4

/* Common error codes (err_id field) */
#define OT_ERR_INVALID_DEV_ID  1
#define OT_ERR_INVALID_CHN_ID  2
#define OT_ERR_ILLEGAL_PARAM   3
#define OT_ERR_EXIST           4
#define OT_ERR_UNEXIST         5
#define OT_ERR_NULL_PTR        6
#define OT_ERR_NOT_CFG         7
#define OT_ERR_NOT_SUPPORT     8
#define OT_ERR_NOT_PERM        9
#define OT_ERR_NOT_ENABLE      10
#define OT_ERR_NO_MEM          12
#define OT_ERR_NO_BUF          13
#define OT_ERR_BUF_EMPTY       14
#define OT_ERR_BUF_FULL        15
#define OT_ERR_NOT_READY       16
#define OT_ERR_BAD_ADDR        17
#define OT_ERR_BUSY            18
#define OT_ERR_REG             19

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* OT_ERRNO_H */
