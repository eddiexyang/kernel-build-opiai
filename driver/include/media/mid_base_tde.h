/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: tde MID base header file
 * Author: Hisilicon multimedia software group
 * Create: 2023/03/10
 */

#ifndef MID_BASE_TDE_H
#define MID_BASE_TDE_H
#include "hi_common_tde.h"
#ifdef __cplusplus
extern "C" {
#endif

#define MID_SUCCESS 0
#define MID_FAILURE (-1)

typedef enum {
    MID_FALSE = 0,
    MID_TRUE = 1,
} mid_bool;

/* Structure of the bitmap information set by customers */
typedef struct {
    unsigned long phys_addr; /* Header address of a bitmap or the Y component */
    unsigned int phys_len;
    hi_tde_color_format color_format; /* Color format */
    unsigned int height;                    /* Bitmap height */
    unsigned int width;                     /* Bitmap width */
    unsigned int stride;                    /* Stride of a bitmap or the Y component */
    mid_bool is_ycbcr_clut;            /* Whether the CLUT is in the YCbCr space. */
    mid_bool alpha_max_is_255;         /* The maximum alpha value of a bitmap is 255 or 128. */
    mid_bool support_alpha_ex_1555;    /* Whether to enable the alpha extension of an ARGB1555 bitmap. */
    unsigned char alpha0;                     /* Values of alpha0 and alpha1, used as the ARGB1555 format */
    unsigned char alpha1;                     /* Values of alpha0 and alpha1, used as the ARGB1555 format */
    unsigned long cbcr_phys_addr;            /* Address of the CbCr component, pilot */
    unsigned int cbcr_phys_len;
    unsigned int cbcr_stride; /* Stride of the CbCr component, pilot */
    /* <Address of the color look-up table (CLUT), for color extension or color correction */
    unsigned long clut_phys_addr;
    unsigned int clut_phys_len;
} mid_tde_surface;

/* Definition of the TDE rectangle */
typedef struct {
    int pos_x;  /* Horizontal coordinate */
    int pos_y;  /* Vertical coordinate */
    unsigned int width;  /* Width */
    unsigned int height; /* Height */
} mid_tde_rect;

/* dma module */
typedef struct {
    mid_tde_surface *dst_surface;
    mid_tde_rect *dst_rect;
} mid_tde_none_src;

/* single source */
typedef struct {
    mid_tde_surface *src_surface;
    mid_tde_surface *dst_surface;
    mid_tde_rect *src_rect;
    mid_tde_rect *dst_rect;
} mid_tde_single_src;

/* double source */
typedef struct {
    mid_tde_surface *bg_surface;
    mid_tde_surface *fg_surface;
    mid_tde_surface *dst_surface;
    mid_tde_rect *bg_rect;
    mid_tde_rect *fg_rect;
    mid_tde_rect *dst_rect;
} mid_tde_double_src;

/* Definition of fill colors */
typedef struct {
    hi_tde_color_format color_format; /* TDE pixel format */
    unsigned int color_value;               /* Fill colors that vary according to pixel formats */
} mid_tde_fill_color;

/* Definition of colorkey range */
typedef struct {
    unsigned char min_component;       /* Minimum value of a component */
    unsigned char max_component;       /* Maximum value of a component */
    unsigned char is_component_out;    /* The colorkey of a component is within or beyond the range. */
    unsigned char is_component_ignore; /* Whether to ignore a component. */
    unsigned char component_mask;      /* Component mask */
    unsigned char component_reserved;
    unsigned char component_reserved1;
    unsigned char component_reserved2;
} mid_tde_colorkey_component;

/* Definition of colorkey values */
typedef union {
    struct {
        mid_tde_colorkey_component alpha; /* Alpha component */
        mid_tde_colorkey_component red;   /* Red component */
        mid_tde_colorkey_component green; /* Green component */
        mid_tde_colorkey_component blue;  /* Blue component */
    } argb_colorkey;                     /* AUTO:mid_tde_colorkey_mode:MID_TDE_COLORKEY_MODE_NONE; */
    struct {
        mid_tde_colorkey_component alpha; /* Alpha component */
        mid_tde_colorkey_component y;     /* Y component */
        mid_tde_colorkey_component cb;    /* Cb component */
        mid_tde_colorkey_component cr;    /* Cr component */
    } ycbcr_colorkey;                    /* AUTO:mid_tde_colorkey_mode:MID_TDE_COLORKEY_MODE_FG; */
    struct {
        mid_tde_colorkey_component alpha; /* Alpha component */
        mid_tde_colorkey_component clut;  /* Palette component */
    } clut_colorkey;                     /* AUTO:mid_tde_colorkey_mode:MID_TDE_COLORKEY_MODE_BG; */
} mid_tde_colorkey;

/* Options for the alpha blending operation */
typedef struct {
    mid_bool global_alpha_en;     /* Global alpha enable */
    mid_bool pixel_alpha_en;      /* Pixel alpha enable */
    mid_bool src1_alpha_premulti; /* Src1 alpha premultiply enable */
    mid_bool src2_alpha_premulti; /* Src2 alpha premultiply enable */
    hi_tde_blend_cmd blend_cmd;  /* Alpha blending command */
    /* Src1 blending mode select. It is valid when blend_cmd is set to MID_TDE_BLEND_CMD_CONFIG. */
    hi_tde_blend_mode src1_blend_mode;
    /* Src2 blending mode select. It is valid when blend_cmd is set to MID_TDE_BLEND_CMD_CONFIG. */
    hi_tde_blend_mode src2_blend_mode;
} mid_tde_blend_opt;

/* CSC parameter option */
typedef struct {
    mid_bool src_csc_user_en;         /* User-defined ICSC parameter enable */
    mid_bool src_csc_param_reload_en; /* User-defined ICSC parameter reload enable */
    mid_bool dst_csc_user_en;         /* User-defined OCSC parameter enable */
    mid_bool dst_csc_param_reload_en; /* User-defined OCSC parameter reload enable */
    unsigned long src_csc_param_addr;       /* ICSC parameter address. The address must be 128-bit aligned. */
    int src_csc_param_len;
    unsigned long dst_csc_param_addr; /* OCSC parameter address. The address must be 128-bit aligned. */
    int dst_csc_param_len;
} mid_tde_csc_opt;

/* Definition of the pattern filling operation */
typedef struct {
    hi_tde_alpha_blending alpha_blending_cmd; /* Logical operation type */
    hi_tde_rop_mode rop_color; /* ROP type of the color space */
    hi_tde_rop_mode rop_alpha; /* ROP type of the alpha component */
    hi_tde_colorkey_mode colorkey_mode; /* Colorkey mode */
    mid_tde_colorkey colorkey_value; /* Colorkey value */
    hi_tde_clip_mode clip_mode; /* Clip mode */
    mid_tde_rect clip_rect; /* Clipping area */
    mid_bool clut_reload; /* Whether to reload the CLUT */
    unsigned char global_alpha; /* Global alpha */
    hi_tde_out_alpha_from out_alpha_from; /* Source of the output alpha */
    unsigned int color_resize; /* Colorize value */
    mid_tde_blend_opt blend_opt; /* Options of the blending operation */
    mid_tde_csc_opt csc_opt; /* CSC parameter option */
} mid_tde_pattern_fill_opt;

#ifdef __cplusplus
}
#endif
#endif /* MID_BASE_TDE_H */
