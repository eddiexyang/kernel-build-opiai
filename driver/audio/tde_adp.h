/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2018-2022. All rights reserved.
 * Description: tde adp
 * Author: Hisilicon multimedia software group
 * Create: 2018/10/30
 */

#ifndef TDE_ADP_H
#define TDE_ADP_H

#include "tde_define.h"
#include "media_mem_file.h"
#include "dvpp_mem_flag_def.h"
#include "tde_reg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/*
 * Handle responded with node
 * Add 4 byte pointer in physical buffer header,to save software node;
 * For need to consult current executing software node,but register can only
 * give the physical addr of it.
 */
#define TDE_NODE_HEAD_BYTE 16

/* Next node addr,update info,occupied bytes */
#define TDE_NODE_TAIL_BYTE 12

#define CMD_SIZE    72
#define JOB_SIZE    152
#define NODE_SIZE \
    ((((sizeof(tde_hw_node)) + (td_u32)(TDE_NODE_HEAD_BYTE) + (td_u32)(TDE_NODE_TAIL_BYTE)) + (0x0F)) & (~0x0FU))
#define FILTER_SIZE 1792

#define OT_TDE_FILTER_NUM 4U

#define TDE_INTNUM       85
#define TDE_REG_BASEADDR 0x4001e0000U
#define TDE_REG_CLOCK    0x11019d40

#define TDE_CTRL         0x0500
#define TDE_INT          0x0504
#define TDE_INTCLR       0x0508
#define TDE_AQ_NADDR_LOW 0x04fcU
#define TDE_AQ_NADDR_HI  0x04f8U

#define TDE_STA 0x4000

#define TDE_AQ_ADDR_LOW  0x4098
#define TDE_AQ_ADDR_HI  0x4094

#ifdef CONFIG_TDE_ZME_LINE_BUFFER2048
#define MAX_LINE_BUFFER  2048
#else
#define MAX_LINE_BUFFER  1920
#endif

#define TDE_MISCELLANEOUS 0x0514

#define TDE_IRQ_NAME "tde_osr_isr"

#ifndef __RTOS__
#define DESCRIPTION "TDE Device driver"
#define AUTHOR      "Digital Media Team."
#define TDE_VERSION "V1.0.0.0"
#endif

/* acsend tde soc cfg manager reg */
#define MEDIA_SUBCTL_BASEADDR  0x400130000
#define MEDIA_REG_SIZE         0x10000
#define TDE_WRAP_OFFSET        0x10000

#define SC_TDE_ICG_EN          0x0600
#define SC_TDE_ICG_DIS         0x0604
#define SC_TDE_ICG_ST          0x5600

#define SC_TDE_RESET_DREQ      0x0B84
#define SC_TDE_RESET_REQ       0x0B80
#define SC_TDE_RESET_ST        0x5B80

#define TDE_CKEN               0x0014
#define TDE_SRST_REQ           0x0010
#define TDE_SOFTRST_STATE      0x0610

#define SC_AWADDR_EXT_0        0x0000
#define SC_ARADDR_EXT_0        0x0004
#define SC_AXADDR_EXT_VAULE    0xF000

#define SC_AWUSER_0_0          0x0020
#define SC_ARUSER_0_0          0x0040
/* acsend tde soc cfg manager reg end */

typedef enum {
    TDE_DRV_INT_NODE = 0x1,
    TDE_DRV_INT_TIMEOUT = 0x2,
    TDE_DRV_INT_ERROR = 0x4,
    TDE_DRV_INT_NODE_COMP_AQ = 0x8
} tde_drv_int;

#define OT_GFX_TDE_ID       0  /* TDE ID */
#define OT_GFX_JPGDEC_ID    1  /* JPEG DECODE ID */
#define OT_GFX_JPGENC_ID    2  /* JPEG_ENCODE ID */
#define OT_GFX_FB_ID        3  /* FRAMEBUFFER ID */
#define OT_GFX_PNG_ID       4  /* PNG ID */
#define OT_GFX_HIGO_ID      5
#define OT_GFX_GFX2D_ID     6
#define OT_GFX_BUTT_ID      7

#define conver_id(module_id) ((module_id) + OT_ID_TDE - OT_GFX_TDE_ID)

#define CONFIG_TDE_TDE_EXPORT_FUNC

#define TDE_NO_SCALE_VSTEP     0x1000
#define TDE_NO_SCALE_HSTEP     0x100000
#define TDE_FLOAT_BITLEN       12
#define TDE_HAL_HSTEP_FLOATLEN 20
#define TDE_HAL_VSTEP_FLOATLEN 12
#define TDE_MAX_SLICE_WIDTH    256
#define TDE_MAX_SLICE_NUM      20
#define TDE_MAX_SURFACE_PITCH  0xffff
#define TDE_MAX_ZOOM_OUT_STEP  8
#define TDE_MAX_RECT_WIDTH_EX  0x2000
#define TDE_MAX_RECT_HEIGHT_EX 0x2000

#define TDE_MAX_SURFACE_WIDTH  0x1000
#define TDE_MAX_SURFACE_HEIGHT 0x1000

#define TDE_MAX_RECT_WIDTH  0x1000
#define TDE_MAX_RECT_HEIGHT 0x1000

#define TDE_MAX_SLICE_RECT_WIDTH  0xfff
#define TDE_MAX_SLICE_RECT_HEIGHT 0xfff

#define TDE_MAX_MINIFICATION_H 255
#define TDE_MAX_MINIFICATION_V 255

#define ROP 0x1                /* Rop */
#define ALPHABLEND (0x1 << 1U)  /* AlphaBlend */
#define COLORIZE   (0x1 << 2U)  /* Colorize */
#define CLUT       (0x1 << 3U)  /* Clut */
#define COLORKEY   (0x1 << 4U)  /* ColorKey */
#define CLIP       (0x1 << 5U)  /* Clip */
#define DEFLICKER  (0x1 << 6U)  /* Deflicker */
#define RESIZE     (0x1 << 7U)  /* Resize */
#define MIRROR     (0x1 << 8U)  /* Mirror */
#define CSCCOVERT  (0x1 << 9U)  /* CSC */
#define QUICKCOPY  (0x1 << 10U) /* copy */
#define QUICKFILL  (0x1 << 11U) /* fill */
#define PATTERFILL (0x1 << 12U) /* patterfill */
#define MASKROP    (0x1 << 13U) /* MaskRop */
#define MASKBLEND  (0x1 << 14U) /* MaskBlend */
#define ROTATE     (0x1 << 15U) /* Rotate */
#define COMPRESS   (0x1 << 16U) /* Compress */
#define SYNC       (0x1 << 17U) /* Sync */
#define SLICE      (0x1 << 18U) /* Slice */

#define ROP_MASK        0xffffffff
#define ALPHABLEND_MASK 0xffffffff
#define COLORIZE_MASK   0xffffffff
#define CLUT_MASK       0xffffffff
#define COLORKEY_MASK   0xffffffff
#define CLIP_MASK       0xffffffff
#define DEFLICKER_MASK  0xffffffff
#define RESIZE_MASK     0xffffffff
#define MIRROR_MASK     0xffffffff
#define CSCCOVERT_MASK  0xffffffff
#define QUICKCOPY_MASK  0xffffffff
#define QUICKFILL_MASK  0xffffffff
#define PATTERFILL_MASK 0xffffffff
#define MASKROP_MASK    0xffffffff
#define MASKBLEND_MASK  0xffffffff
#ifdef CONFIG_TDE_ROTATE_SUPPORT
#define ROTATE_MASK     0xffffffff
#else
#define ROTATE_MASK     0x0
#endif
#define COMPRESS_MASK   0xffffffff
#ifdef CONFIG_GFBG_LOW_DELAY_SUPPORT
#define SYNC_MASK       0xffffffff
#else
#define SYNC_MASK       0x0
#endif
#define SLICE_MASK      0x0

td_void tde_hal_get_capability(td_u32 *capability);
struct media_mem_file *tde_get_mem_file(void);
td_void tde_set_mem_file(struct media_mem_file* mem_file);

#define TDE_CAPABILITY (((ROP) & (ROP_MASK)) | \
                        ((ALPHABLEND) & (ALPHABLEND_MASK)) | \
                        ((COLORIZE) & (COLORIZE_MASK)) | \
                        ((CLUT) & (CLUT_MASK)) | \
                        ((COLORKEY) & (COLORKEY_MASK)) | \
                        ((CLIP) & (CLIP_MASK)) | \
                        ((DEFLICKER) & (DEFLICKER_MASK)) | \
                        ((RESIZE) & (RESIZE_MASK)) | \
                        ((MIRROR) & (MIRROR_MASK)) | \
                        ((CSCCOVERT) & (CSCCOVERT_MASK)) | \
                        ((QUICKCOPY) & (QUICKCOPY_MASK)) | \
                        ((QUICKFILL) & (QUICKFILL_MASK)) | \
                        ((PATTERFILL) & (PATTERFILL_MASK)) | \
                        ((MASKROP) & (MASKROP_MASK)) | \
                        ((MASKBLEND) & (MASKBLEND_MASK)) | \
                        ((ROTATE) & (ROTATE_MASK)) | \
                        ((COMPRESS) & (COMPRESS_MASK)) | \
                        ((SYNC) & (SYNC_MASK)) | \
                        ((SLICE) & (SLICE_MASK)))

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* TDE_ADP_H */
