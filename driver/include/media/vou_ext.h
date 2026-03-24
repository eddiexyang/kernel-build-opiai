/*
 * vou_ext.h - VO (Video Output) module external interface
 *
 * Derived from source analysis of gfbg_vou_drv.c which only uses
 * func_entry(vou_export_func, OT_ID_VO) to obtain the VO function table.
 * The actual VOU graphics functions are defined within the VDP module itself.
 */
#ifndef VOU_EXT_H
#define VOU_EXT_H

#include "ot_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*
 * VO export function table - placeholder structure.
 * gfbg_vou_drv.c uses func_entry(vou_export_func, OT_ID_VO) but
 * the actual usage is minimal - most VO graphics operations are
 * implemented directly in gfbg_vou_graphics.c within the same module.
 */
typedef struct {
    td_void *reserved;
} vou_export_func;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* VOU_EXT_H */
