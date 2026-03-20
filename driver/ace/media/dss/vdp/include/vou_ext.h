#ifndef OT_VOU_EXT_H
#define OT_VOU_EXT_H

#include "ot_common.h"
#include "gfbg_vou_drv.h"

typedef td_s32 (*vo_fb_int_call_back)(const td_void *paraml, ot_vo_dev vo_dev, const td_void *paramr);

typedef struct {
    td_s32 (*pfn_vou_graphics_set_callback)(td_u32 layer, td_u32 int_type,
        vo_fb_int_call_back call_back, td_void *call_back_arg);
    td_s32 (*pfn_vou_graphics_open_layer)(td_u32 layer);
    td_void (*pfn_vou_graphics_vo_exit)(td_void);
} vou_export_func;

#endif
