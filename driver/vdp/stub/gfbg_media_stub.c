// SPDX-License-Identifier: GPL-2.0
/*
 * Stub implementations for mediabase symbols referenced by gfbg_main.c
 *
 * IDA Pro analysis of ascend_vdp_hifb.ko confirms that these functions
 * are referenced at link time but never called at runtime for ascend310Brc.
 * The real implementations live in drv/adp/ but depend on the full
 * mediabase SDK (VOU/TDE/SYS) which is not present in this tree.
 *
 * These stubs satisfy modpost and allow the module to load cleanly.
 */

#include "ot_type.h"
#include "gfbg_vou_drv.h"
#include "gfbg_graphics_drv.h"
#include "gfbg_vou_graphics.h"
#include "gfbg_blit.h"
#include "gfbg_rotate.h"

td_bool is_hd_layer(td_u32 layer_id)
{
	return TD_FALSE;
}

td_bool is_ad_layer(td_u32 layer_id)
{
	return TD_FALSE;
}

td_bool gfbg_tde_is_support_fmt(ot_fb_color_format format)
{
	return TD_FALSE;
}

td_s32 gfbg_drv_set_tde_callback(int_callback tde_callback)
{
	return TD_FAILURE;
}

td_s32 gfbg_drv_set_tde_rotate_callback(int_callback tde_rot_callback)
{
	return TD_FAILURE;
}

td_bool vou_graphics_dev_check(td_void)
{
	return TD_FALSE;
}

td_s32 vou_graphics_init(td_void)
{
	return TD_FAILURE;
}

td_u32 vou_get_gfbg_irq(td_void)
{
	return 0;
}

static vo_gfxlayer_context g_stub_gfx_layer_ctx;
vo_gfxlayer_context *vou_get_gfx_layer_ctx(td_void)
{
	return &g_stub_gfx_layer_ctx;
}

td_void graphic_drv_int_set_dhd0_ctrl(td_u32 int_ctrl)
{
}

/* --- round 2: symbols uncovered after first batch was stubbed --- */

td_bool is_4k_layer(td_u32 layer_id)
{
	return TD_FALSE;
}

td_bool is_sd_layer(td_u32 layer_id)
{
	return TD_FALSE;
}

td_bool is_cursor_layer(td_u32 layer_id)
{
	return TD_FALSE;
}

td_void gfbg_drv_get_ops(gfbg_drv_ops *ops)
{
}

td_s32 gfbg_drv_blit(const ot_fb_buf *src_img, const ot_fb_buf *dst_img,
		      const gfbg_blit_opt *opt, td_bool is_refresh_screen)
{
	return TD_FAILURE;
}

td_bool gfbg_get_rotation_support(td_void)
{
	return TD_FALSE;
}

gfbg_rotate gfbg_get_rotation(td_void)
{
	return TD_NULL;
}

td_void vou_set_gfbg_irq(td_u32 gfbg_irq)
{
}

td_s32 vou_int_msk_init(td_u32 layer_id)
{
	return TD_FAILURE;
}

td_s32 vou_graphics_deinit(td_void)
{
	return TD_SUCCESS;
}

/* --- round 3: final batch from comprehensive nm analysis --- */

#include "gfbg_comm.h"

td_void graphic_drv_int_set_dhd1_ctrl(td_u32 int_ctrl)
{
}

td_void gfbg_recalculate_stride(td_u32 *cmp_stride, td_u32 *uncmp_stride,
				const gfbg_stride_attr *attr)
{
}

td_void gfbg_rotation_register(td_void)
{
}

td_s32 gfbg_drv_set_vgs_rotate_callback(vgs_callback vgs_rot_callback)
{
	return TD_FAILURE;
}

td_s32 gfbg_vou_get_dev_id(td_u32 layer_id)
{
	return 0;
}

td_s32 graphic_drv_get_bind_dev(td_s32 layer_id)
{
	return 0;
}

td_void graphic_drv_get_int_state_vcnt(ot_vo_dev vo_dev, td_u32 *vcnt)
{
	if (vcnt)
		*vcnt = 0;
}
