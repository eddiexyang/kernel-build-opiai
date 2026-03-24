/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: gfbg blit
 */

#ifndef GFBG_ROTATE_VGS_H
#define GFBG_ROTATE_VGS_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

td_s32 gfbg_drv_rotate_vgs_process(const ot_fb_buf *src_img, const ot_fb_buf *dst_img,
                                   const gfbg_rotate_opt *rotate_opt, td_bool is_refresh_screen);
#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* GFBG_ROTATE_VGS_H */