/*
* Copyright (c) Hisilicon Technologies Co., Ltd. 2011-2019. All rights reserved.
* Description: Definition of common and media modules
* Author: Hisilicon multimedia software group
* Create: 2011/06/28
*/

#ifndef __HI_DEFINES_H__
#define __HI_DEFINES_H__

#include "autoconf.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#ifndef HICHIP
#define HICHIP                          CONFIG_HI_CHIP_TYPE
#endif

#if defined(DVPP_MLV1R3)
#define DEVICE_NUM 1U
#elif defined(DVPP_ML) || defined(DVPP_MINI)
#define DEVICE_NUM 2U
#else // #ifdef DVPP_MINI
#define DEVICE_NUM 1U
#endif // #ifdef DVPP_MINI
#define MAX_DEVICE_NUMS 2U
#define CPU_NUM_PER_CHIP 8

#define CSC_MODE_NUM 6
#define CSC_COEFFICIENT_NUM 12

#define HIDVPP_V100ES                  0x3000AEF
#define HIDVPP_V100                    0x3000A100
#define HIDVPP_V300                    0x3000A300
#define HIDVPP_V500                    0x3000A500
#define HIDVPP_V200                    0x3000A200

#define HIDVPP_Vxxx                    0x35000000

#if HICHIP == HIDVPP_V100ES
#define CHIP_NAME                       "HiDvppV100ES"
#elif HICHIP == HIDVPP_V100
#define CHIP_NAME                       "HiDvppV100"
#elif HICHIP == HIDVPP_V300
#define CHIP_NAME                       "HiDvppV300"
#elif HICHIP == HIDVPP_V500
#define CHIP_NAME                       "HiDvppV500"
#elif HICHIP == HIDVPP_V200
#define CHIP_NAME                       "HiDvppV200"
#elif HICHIP == HIDVPP_Vxxx
#error HuHu, I am an dummy chip
#else
#error HICHIP define may be error
#endif

#define MPP_VER_PRIX                    "_MPP_V"

#define ALIGN_NUM                       8

#define LUMA_PHY_ALIGN                  32

#define DEFAULT_ALIGN                  32U
#define MAX_ALIGN                       1024
#define SEG_CMP_LENGTH                  256U

#define MAX_MMZ_NAME_LEN                32U

#define VO_MAX_NODE_NUM                 16

/* For VB */
// 目前VENC一个通道需要4个,VDEC一个通道需要2个,先预留到2048个
#define VB_MAX_POOLS_PER_DEVICE         2048U
#define VB_MAX_POOLS                    (VB_MAX_POOLS_PER_DEVICE * DEVICE_NUM)

/* For VENC */
#define VENC_MAX_NAME_LEN               16
// 目前mediabase中arch11软连接指向hidvpp,暂时用宏隔离,后面arch拆分后可以放到不同的arch中定义
#ifdef DVPP_MLV1R3
#define VENC_MAX_CHN_NUM_PER_DEVICE     128U
#else
#define VENC_MAX_CHN_NUM_PER_DEVICE     256U
#endif
#define VENC_MAX_CHN_ID_NUM             (VENC_MAX_CHN_NUM_PER_DEVICE * DEVICE_NUM)

#if defined(DVPP_MLV1R3)
#define VEDU_IP_NUM_PER_DEVICE          3U
#define VEDU_IP_NUM                     (VEDU_IP_NUM_PER_DEVICE * DEVICE_NUM)
#elif defined(DVPP_ML)
#define VEDU_IP_NUM                     0
#define VEDU_IP_NUM_PER_DEVICE          0
#else // #ifdef DVPP_ML
#define VEDU_IP_NUM_PER_DEVICE          3U
#define VEDU_IP_NUM                     (VEDU_IP_NUM_PER_DEVICE * DEVICE_NUM)
#endif // #ifdef DVPP_ML
#define H264E_MAX_RESOLUTION            9437184 // 9437184: 4096x2304
#define VENC_MAX_VGS_SIZE               8192U
#define JPEG_IP_NUM_PER_DEVICE          8U
#define JPEG_IP_NUM                     (DEVICE_NUM * JPEG_IP_NUM_PER_DEVICE)
#ifdef DVPP_ML
#define JPEGE_NUM_PER_JPEG              1U
#define JPEGE_IP_NUM_PER_DEVICE         8
#define JPEGE_IP_NUM                    8
#else // #ifdef DVPP_ML
#define JPEGE_NUM_PER_JPEG              0x1U
#define JPEGE_IP_NUM_PER_DEVICE         (JPEG_IP_NUM_PER_DEVICE * JPEGE_NUM_PER_JPEG)
#define JPEGE_IP_NUM                    (DEVICE_NUM * JPEGE_IP_NUM_PER_DEVICE)
#endif // #ifdef DVPP_ML
#define JPEGE_MAX_WIDTH                 8192
#define JPEGE_MAX_HEIGHT                8192
#define JPEGE_MIN_WIDTH                 32
#define JPEGE_MIN_HEIGHT                32
#define JPGE_MAX_NUM                    1
#define PRORES_MAX_WIDTH                7680
#define PRORES_MAX_HEIGHT               4320
#define PRORES_MIN_WIDTH                256
#define PRORES_MIN_HEIGHT               256
#define VENC_MAX_ROI_NUM                8U
#define H264E_MIN_HW_INDEX              0
#define H264E_MAX_HW_INDEX              11
#define H264E_MIN_VW_INDEX              0
#define H264E_MAX_VW_INDEX              3
#define VENC_QP_HISGRM_NUM              52
#define PRORES_MAX_TILE_NUM             3
#define MAX_TILE_NUM                    3U
#define H265E_ADAPTIVE_FRAME_TYPE       5U
#define H265E_ADAPTIVE_QP_TYPE          5U

#define VENC_MIN_INPUT_FRAME_RATE       1
#define VENC_MAX_INPUT_FRAME_RATE       240

#define VENC_MAX_RECEIVE_SOURCE         4

#define VENC_PIC_RECEIVE_SOURCE0        0
#define VENC_PIC_RECEIVE_SOURCE1        1
#define VENC_PIC_RECEIVE_SOURCE2        2
#define VENC_PIC_RECEIVE_SOURCE3        3

/* For RC */
#define RC_TEXTURE_THR_SIZE             16
#define MIN_BITRATE                     2
#define MAX_BITRATE                     (600 * 1024)
#define MAX_EXTRA_BITRATE               (1000 * 1024)

/* For VDEC */
#define VDH_MAX_CHN_NUM                 256

#define VEDU_CHN_START                  VDH_MAX_CHN_NUM
#define VEDU_H264D_ERRRATE              10
#define VEDU_H264D_FULLERR              100
#ifdef DVPP_ML
#ifndef DVPP_MLV1R3
#define VDEC_MAX_CHN_NUM_PER_DEVICE     32U
#define VDEC_USERSPACE_MAX_CHN_NUM_PER_DEVICE     256U // 910B上对外用户通道号范围为[0, 255], 内部限制为32
#else
#define VDEC_MAX_CHN_NUM_PER_DEVICE     128U
#define VDEC_USERSPACE_MAX_CHN_NUM_PER_DEVICE     VDEC_MAX_CHN_NUM_PER_DEVICE
#endif

#else // #ifdef DVPP_ML
#define VDEC_MAX_CHN_NUM_PER_DEVICE     256U
#define VDEC_USERSPACE_MAX_CHN_NUM_PER_DEVICE     VDEC_MAX_CHN_NUM_PER_DEVICE
#endif // #ifdef DVPP_ML
#define INTERNAL_VDEC_MAX_CHN_NUM       (VDEC_MAX_CHN_NUM_PER_DEVICE * DEVICE_NUM) // 256 * 2

#define INTERNAL_VDEC_USERSPACE_MAX_CHN_NUM       (VDEC_USERSPACE_MAX_CHN_NUM_PER_DEVICE * DEVICE_NUM)

#define VDEC_IP_NUM_PER_DEVICE 12U

#ifdef DVPP_MLV1R3
#define MAX_VDEC_FRAME_NUM     128 // 小核需要128个
#else
#define MAX_VDEC_FRAME_NUM     40 // 大于ref_num + display_num +1, VPC也要用来创建队列深度
#endif

// For VPC
#ifdef DVPP_ML

 // 必须要加1，否则在跑最大通道号时 VPC最后一个通道给test pattern使用，会有问题
#ifndef DVPP_MLV1R3
#define VPC_MAX_CHN_NUM_PER_DEVICE      (VDEC_MAX_CHN_NUM_PER_DEVICE + 1)
#else
#define VPC_MAX_CHN_NUM_PER_DEVICE      256U // ML版本VPC内核通道只用于HFBC，和VDEC通道数保持一致
#endif
// ML 版本要改成0，否则在算力切分场景下，vdec算力切分会有问题
#define EXTERNAL_VPC_MAX_CHN_NUM        0
#else // #ifdef DVPP_ML
#define VPC_MAX_CHN_NUM_PER_DEVICE      513U // 256 vpc通道+256 vdec通道+1 safety通道
#define EXTERNAL_VPC_MAX_CHN_NUM        (VPC_MAX_CHN_NUM_PER_DEVICE / 2U)
#endif // #ifdef DVPP_ML
#define INTERNAL_VPC_MAX_CHN_NUM        (VPC_MAX_CHN_NUM_PER_DEVICE * DEVICE_NUM)

#define VPC_TEST_PATTERN_CHNL_ID        (VPC_MAX_CHN_NUM_PER_DEVICE - 1)

// For PNGD
#define PNGD_MAX_CHN_NUM_PER_DEVICE     128U
#define INTERNAL_PNGD_MAX_CHN_NUM       (PNGD_MAX_CHN_NUM_PER_DEVICE * DEVICE_NUM)

#define H264D_ALIGN_W                   64
#define H264D_ALIGN_H                   16
#define H265D_ALIGN_W                   64
#define H265D_ALIGN_H                   64
#define JPEGD_ALIGN_W                   64U
#define JPEGD_ALIGN_H                   16U
#define JPEGD_RGB_ALIGN                 16
#define VPC_ALIGN_W                     16U
#define VPC_ALIGN_H                     2U

#define H264D_MAX_SPS                   32
#define H264D_MIN_SPS                   1
#define H264D_MAX_PPS                   256
#define H264D_MIN_PPS                   1
#define H264D_MAX_SLICE                 300
#define H264D_MIN_SLICE                 1

#define H265D_MAX_VPS                   16
#define H265D_MIN_VPS                   1
#define H265D_MAX_SPS                   16
#define H265D_MIN_SPS                   1
#define H265D_MAX_PPS                   64
#define H265D_MIN_PPS                   1
#define H265D_MAX_SLICE                 600
#define H265D_MIN_SLICE                 1

#define VDH_MIN_MSG_NUM                 1
#define VDH_MAX_MSG_NUM                 1
#define VDH_MIN_BIN_SIZE                72
#define VDH_MAX_BIN_SIZE                2048
#define VDH_MIN_EXT_MEM_LEVEL           1
#define VDH_MAX_EXT_MEM_LEVEL           6

#define JPEGD_LOCATION_DEEP 3 // 记录jpegd处在哪个subsys、sche 、port
#define JPEGD_IP_NUM_PER_DEVICE 56U

#define JPEGD_IP_NUM (DEVICE_NUM * JPEGD_IP_NUM_PER_DEVICE)
#define JPEGD_MAX_HANDLE_NUM_PER_DEVICE (JPEGD_IP_NUM_PER_DEVICE)

#ifdef DVPP_ML
#define JPEGD_MAX_WIDTH                 16384U
#define JPEGD_MAX_HEIGHT                16384U
#else // #ifdef DVPP_ML
#define JPEGD_MAX_WIDTH                 8192U
#define JPEGD_MAX_HEIGHT                8192U
#endif // #ifdef DVPP_ML
#define JPEGD_MIN_WIDTH                 32U
#define JPEGD_MIN_HEIGHT                32U
#define JPEGD_RGB_MAX_HEIGHT            8640U

#if defined(DVPP_MLV1R3)
#define VPC_IP_NUM_PER_DEVICE            2U
#elif defined(DVPP_ML) // #if defined(DVPP_MLV1R3)
#define VPC_IP_NUM_PER_DEVICE            10U
#else // #elif defined(DVPP_ML)
#define VPC_IP_NUM_PER_DEVICE            12U
#endif // #if defined(DVPP_MLV1R3) and #elif defined(DVPP_ML)
#define VPC_IP_NUM ((DEVICE_NUM) * (VPC_IP_NUM_PER_DEVICE))

#define VPC_MAX_WIDTH         8192U
#define VPC_MAX_HEIGHT        8192U
#define VPC_MIN_WIDTH         10U
#define VPC_MIN_HEIGHT        6U
#define VPC_MAX_WIDTH_4K_INPUT 4096U
#define VPC_MAX_HEIGHT_4K_INPUT 4096U
#define VPC_RGB_MAX_HEIGHT    4096
#define VPC_SUBSYS_NUM (DEVICE_NUM * 3U)

#ifdef DVPP_ML
#define VPC_MAX_WIDTH_STRIDE  ((hi_u32)(-1))
#define VPC_MAX_HEIGHT_STRIDE ((hi_u32)(-1))
#else // #ifndef DVPP_ML
#define VPC_MAX_WIDTH_STRIDE (16384U)
#define VPC_MAX_HEIGHT_STRIDE (16384U)
#endif // #ifdef DVPP_ML

#ifdef DVPP_MDC_V2
#define VPC_MIN_WIDTH_STRIDE (10U)
#else // #ifdef DVPP_MDC_V2
#define VPC_MIN_WIDTH_STRIDE (32U)
#endif // #ifdef DVPP_MDC_V2
#define VPC_MIN_HEIGHT_STRIDE (6U)

#ifdef DVPP_MDC
#define VPC_RX_IRQ_CPU_10 10
#define VPC_RX_IRQ_CPU_11 11
#endif

/* For Region */

#define RGN_MIN_WIDTH                   2
#define RGN_MIN_HEIGHT                  2

#define RGN_COVER_MIN_X                 -16384
#define RGN_COVER_MIN_Y                 -16384
#define RGN_COVER_MAX_X                 16382
#define RGN_COVER_MAX_Y                 16382
#define RGN_COVER_MAX_WIDTH             16384
#define RGN_COVER_MAX_HEIGHT            16384
#define RGN_COVER_MIN_THICK             2
#define RGN_COVER_MAX_THICK             8

#define RGN_COVEREX_MIN_X               -16384
#define RGN_COVEREX_MIN_Y               -16384
#define RGN_COVEREX_MAX_X               16382
#define RGN_COVEREX_MAX_Y               16382
#define RGN_COVEREX_MAX_WIDTH           16384
#define RGN_COVEREX_MAX_HEIGHT          16384
#define RGN_COVEREX_MIN_THICK           2
#define RGN_COVEREX_MAX_THICK           8

#define RGN_OVERLAY_MIN_X               0
#define RGN_OVERLAY_MIN_Y               0
#define RGN_OVERLAY_MAX_X               16382
#define RGN_OVERLAY_MAX_Y               16382
#define RGN_OVERLAY_MAX_WIDTH           16384
#define RGN_OVERLAY_MAX_HEIGHT          16384

#define RGN_OVERLAYEX_MIN_X             0
#define RGN_OVERLAYEX_MIN_Y             0
#define RGN_OVERLAYEX_MAX_X             16382
#define RGN_OVERLAYEX_MAX_Y             16382
#define RGN_OVERLAYEX_MAX_WIDTH         8192
#define RGN_OVERLAYEX_MAX_HEIGHT        8192

#define RGN_MOSAIC_X_ALIGN              1
#define RGN_MOSAIC_Y_ALIGN              1
#define RGN_MOSAIC_WIDTH_ALIGN          1
#define RGN_MOSAIC_HEIGHT_ALIGN         1

#define RGN_MOSAIC_MIN_X                (-16384)
#define RGN_MOSAIC_MIN_Y                (-16384)
#define RGN_MOSAIC_MAX_X                16384
#define RGN_MOSAIC_MAX_Y                16384
#define RGN_MOSAIC_MIN_WIDTH            8
#define RGN_MOSAIC_MIN_HEIGHT           8
#define RGN_MOSAIC_MAX_WIDTH            16384
#define RGN_MOSAIC_MAX_HEIGHT           16384

#define RGN_ALIGN                       2

#define RGN_HANDLE_MAX                  1024
#define RGN_MAX_BUF_NUM                 2

#define RGN_LINE_MAX_THICK              7
#define RGN_VPSS_MAX_COVER_NUM          4
#define RGN_VGS_MAX_TASK_WIDTH          16384

#define COVEREX_MAX_NUM_VI              16
#define OVERLAYEX_MAX_NUM_VI            16

#define OVERLAY_MAX_NUM_VENC            8
#define OVERLAYEX_MAX_NUM_VENC          8

#define COVER_MAX_NUM_VPSS              8
#define COVEREX_MAX_NUM_VPSS            8
#define OVERLAYEX_MAX_NUM_VPSS          8
#define MOSAIC_MAX_NUM_VPSS             4

#define OVERLAYEX_MAX_NUM_AVS           8

#define COVEREX_MAX_NUM_VO              1
#define OVERLAYEX_MAX_NUM_VO            1

#define OVERLAYEX_MAX_NUM_PCIV          1

#define RGN_VGS_TASK_WIDTH_MAX         16384

/*************************************/

#define VENC_MAX_SSE_NUM                8U
#define HISI_MAX_SENSOR_NUM             2

/* For VI */
/* number of channle and device on video input unit of chip
 * Note! VI_MAX_CHN_NUM is NOT equal to VI_MAX_DEV_NUM
 * multiplied by VI_MAX_CHN_NUM, because all VI devices
 * can't work at mode of 4 channles at the same time.
 */
#define VI_MAX_DEV_NUM                  4
#define VI_MAX_PHY_PIPE_NUM             4
#define VI_MAX_VIR_PIPE_NUM             8
#define VI_MAX_PIPE_NUM                 (VI_MAX_PHY_PIPE_NUM + VI_MAX_VIR_PIPE_NUM)
#define VI_MAX_STITCH_GRP_NUM           0
#define VI_MAX_WDR_NUM                  2
#define VI_MAX_PHY_CHN_NUM              1
#define VI_MAX_EXT_CHN_NUM              8
#define VI_EXT_CHN_START                VI_MAX_PHY_CHN_NUM
#define VI_MAX_CHN_NUM                  (VI_MAX_PHY_CHN_NUM + VI_MAX_EXT_CHN_NUM)
#define VI_MAX_EXTCHN_BIND_PER_CHN      8

#define VIPROC_IRQ_NUM                  1
#define VI_MAX_WDR_FRAME_NUM            2
#define VI_MAX_NODE_NUM                 5
#define VIPROC_IP_NUM                   1
#define VICAP_IP_NUM                    1

#define VI_MAX_SPLIT_NODE_NUM           5

#define VI_DEV_MIN_WIDTH                120
#define VI_DEV_MIN_HEIGHT               120
#define VI_DEV_MAX_WIDTH                16384
#define VI_DEV_MAX_HEIGHT               16384

#define VI_PIPE_OFFLINE_MIN_WIDTH       120
#define VI_PIPE_OFFLINE_MIN_HEIGHT      120
#define VI_PIPE_OFFLINE_MAX_WIDTH       16384
#define VI_PIPE_OFFLINE_MAX_HEIGHT      16384

#define VI_PIPE_ONLINE_MIN_WIDTH        120
#define VI_PIPE_ONLINE_MIN_HEIGHT       120
#define VI_PIPE_ONLINE_MAX_WIDTH        4096
#define VI_PIPE_ONLINE_MAX_HEIGHT       4096

#define VI_PIPE0_MAX_WIDTH             16384
#define VI_PIPE0_MAX_HEIGHT            16384
#define VI_PIPE1_MAX_WIDTH             16384
#define VI_PIPE1_MAX_HEIGHT            16384
#define VI_PIPE2_YUV_MAX_WIDTH         16384
#define VI_PIPE2_YUV_MAX_HEIGHT        16384
#define VI_PIPE2_MAX_WIDTH             16384
#define VI_PIPE2_MAX_HEIGHT            16384
#define VI_PIPE3_MAX_WIDTH             16384
#define VI_PIPE3_MAX_HEIGHT            16384
#define VI_PIPE_WDR_FIRST_MAX_WIDTH    VI_PIPE1_MAX_WIDTH
#define VI_PIPE_WDR_SECOND_MAX_WIDTH   VI_PIPE3_MAX_WIDTH
#define VI_VIR_PIPE_MAX_WIDTH          16384
#define VI_VIR_PIPE_MAX_HEIGHT         16384

#define VI_PHYCHN_OFFLINE_MIN_WIDTH     120
#define VI_PHYCHN_OFFLINE_MIN_HEIGHT    120
#define VI_PHYCHN_OFFLINE_MAX_WIDTH     16384
#define VI_PHYCHN_OFFLINE_MAX_HEIGHT    16384

#define VI_PHYCHN_ONLINE_MIN_WIDTH      120
#define VI_PHYCHN_ONLINE_MIN_HEIGHT     120
#define VI_PHYCHN_ONLINE_MAX_WIDTH      4096
#define VI_PHYCHN_ONLINE_MAX_HEIGHT     4096


#define VI_EXTCHN_MIN_WIDTH             32
#define VI_EXTCHN_MIN_HEIGHT            32
#define VI_EXTCHN_MAX_WIDTH             8192
#define VI_EXTCHN_MAX_HEIGHT            8192


#define VI_EXT_CHN_MAX_ZOOMIN           16
#define VI_EXT_CHN_MAX_ZOOMOUT          30

#define VI_CMP_PARAM_SIZE               152
#define VI_VPSS_DEFAULT_EARLINE         128


/* For DIS */
#define DIS_PYRAMID_LAYER_NUM           5
#define DIS_MAX_CHN_NUM                 16
#define DIS_MAX_IMAGE_WIDTH             8192
#define DIS_MIN_IMAGE_WIDTH             1280
#define DIS_MAX_IMAGE_HEIGHT            8192
#define DIS_MIN_IMAGE_HEIGHT            720

/* For VO */
#define VO_MAX_VIRT_DEV_NUM             32 /* max virtual dev num */
#define VO_VIRT_DEV_0                   2 /* virtual device 0 */
#define VO_VIRT_DEV_1                   3 /* virtual device 1 */
#define VO_VIRT_DEV_2                   4 /* virtual device 2 */
#define VO_VIRT_DEV_3                   5 /* virtual device 3 */

#define VO_VIRT_LAYER_0                 3 /* virtual layer 0 */
#define VO_VIRT_LAYER_1                 4 /* virtual layer 1 */
#define VO_VIRT_LAYER_2                 5 /* virtual layer 2 */
#define VO_VIRT_LAYER_3                 6 /* virtual layer 3 */

#define VO_MAX_PHY_DEV_NUM              2 /* max physical dev num */

#define VO_MIN_CHN_WIDTH                32 /* channel minimal width */
#define VO_MIN_CHN_HEIGHT               32 /* channel minimal height */
#define VO_MAX_ZOOM_RATIO               1000 /* max zoom ratio, 1000 means 100% scale */
#define VO_MAX_DEV_NUM                  (VO_MAX_PHY_DEV_NUM + VO_MAX_VIRT_DEV_NUM) /* max dev num */
#define VO_MAX_LAYER_NUM                (3 + VO_MAX_VIRT_DEV_NUM) /* max layer num */
#define VO_MAX_PRIORITY                 3 /* max layer priority */
#define VO_MAX_CHN_NUM                  65 /* max chn num */
#define VO_MAX_LAYER_IN_DEV             2 /* max layer num of each dev */
#define VO_MAX_GRAPHICS_LAYER_NUM       3
#define VO_MAX_WBC_NUM                  1
#define VO_MIN_TOLERATE                 1 /* min play toleration 1ms */
#define VO_MAX_TOLERATE                 100000 /* max play toleration 100s */

/* For AVS */
#define AVS_MAX_GRP_NUM                 32 /* max grp num */
#define AVS_PIPE_NUM                    8 /* max pipe num */
#define AVS_MAX_CHN_NUM                 2 /* max chn num */
#define AVS_SPLIT_NUM                   2
#define AVS_SPLIT_PIPE_NUM              6
#define AVS_CUBE_MAP_SURFACE_NUM        6

#define AVS_MAX_IN_WIDTH                8192
#define AVS_MAX_IN_HEIGHT               8192
#define AVS_MIN_IN_WIDTH                1280
#define AVS_MIN_IN_HEIGHT               720

#define AVS_MAX_OUT_WIDTH               16384
#define AVS_MAX_OUT_HEIGHT              8192
#define AVS_MIN_OUT_WIDTH               256
#define AVS_MIN_OUT_HEIGHT              256

/* For AUDIO */
#define AI_DEV_MAX_NUM                  3
#define AO_DEV_MIN_NUM                  0
#define AO_DEV_MAX_NUM                  5
#define AIO_MAX_NUM                     (AO_DEV_MAX_NUM)

#ifdef CONFIG_AUDIO_V200_SUPPORT
#define AENC_MAX_CHN_NUM                6
#define ADEC_MAX_CHN_NUM                6
#define AI_MAX_CHN_NUM                  2
#define AO_MAX_CHN_NUM                  2
#define AO_SYSCHN_CHNID                 2

#else
#define AENC_MAX_CHN_NUM                32
#define ADEC_MAX_CHN_NUM                32
#define AI_MAX_CHN_NUM                  16
#define AO_MAX_CHN_NUM                  3
#define AO_SYSCHN_CHNID                 (AO_MAX_CHN_NUM - 1)

#endif

#define AIO_MAX_CHN_NUM                 ((AO_MAX_CHN_NUM > AI_MAX_CHN_NUM) ? AO_MAX_CHN_NUM : AI_MAX_CHN_NUM)

#define VPSS_IP_NUM                     1
#define VPSS0                           0
#define VPSS_MAX_GRP_NUM                268
#define VPSS_VIPE_GRP_NUM               12
#define VPSS_MAX_GRP_PIPE_NUM           1
#define VPSS_PARALLEL_PIC_NUM           1
#define VPSS_MAX_PHY_CHN_NUM            3
#define VPSS_LOWDELAY_CHN_NUM           3
#define VPSS_MAX_EXT_CHN_NUM            8
#define VPSS_MAX_CHN_NUM                (VPSS_MAX_PHY_CHN_NUM + VPSS_MAX_EXT_CHN_NUM)
#define VPSS_MIN_IMAGE_WIDTH_SBS        512
#define VPSS_MIN_IMAGE_WIDTH            64
#define VPSS_MIN_IMAGE_HEIGHT           64
#define VPSS_VPC_MIN_IMAGE_WIDTH        64
#define VPSS_VPC_MIN_IMAGE_HEIGHT       64
#define VPSS_MAX_IMAGE_WIDTH_SLAVE      16384

#define VPSS_MAX_IMAGE_WIDTH_ONLINE     4096
#define VPSS_MAX_IMAGE_WIDTH            16384
#define VPSS_MAX_IMAGE_HEIGHT           16384
#define VPSS_VPC_MAX_IMAGE_WIDTH        4096
#define VPSS_VPC_MAX_IMAGE_HEIGHT       8192
#define VPSS_EXTCHN_MAX_IMAGE_WIDTH     16384
#define VPSS_EXTCHN_MAX_IMAGE_HEIGHT    16384
#define VPSS_MAX_ZOOMIN                 16
#define VPSS_MAX_ZOOMOUT                15
#define VPSS_EXT_CHN_MAX_ZOOMIN         16
#define VPSS_EXT_CHN_MAX_ZOOMOUT        30

#define VPSS_VGS_GRP_NO                 256

/* For PCIV */
#define PCIV_MAX_CHN_NUM                128 /* max pciv channel number in each pciv device */

/* For IVS_MD */
#define MD_MAX_CHN                      64
#define MD_MAX_WIDTH                    1920
#define MD_MAX_HEIGHT                   1080
#define MD_MIN_WIDTH                    64
#define MD_MIN_HEIGHT                   64

/* For RECT */
#define DPU_RECT_MAX_GRP_NUM            8
#define DPU_RECT_MAX_PIPE_NUM           2
#define DPU_RECT_MAX_CHN_NUM            2

#define DPU_RECT_IN_IMAGE_MAX_WIDTH     2048
#define DPU_RECT_IN_IMAGE_MAX_HEIGHT    2048
#define DPU_RECT_IN_IMAGE_MIN_WIDTH     128
#define DPU_RECT_IN_IMAGE_MIN_HEIGHT    64
#define DPU_RECT_OUT_IMAGE_MAX_WIDTH    1920
#define DPU_RECT_OUT_IMAGE_MAX_HEIGHT   1080
#define DPU_RECT_OUT_IMAGE_MIN_WIDTH    128
#define DPU_RECT_OUT_IMAGE_MIN_HEIGHT   64

/* For MATCH */
#define DPU_MATCH_MAX_GRP_NUM           8
#define DPU_MATCH_MAX_PIPE_NUM          2
#define DPU_MATCH_MAX_CHN_NUM           1

#define DPU_MATCH_IN_IMAGE_MAX_WIDTH    1920
#define DPU_MATCH_IN_IMAGE_MAX_HEIGHT   1080
#define DPU_MATCH_IN_IMAGE_MIN_WIDTH    128
#define DPU_MATCH_IN_IMAGE_MIN_HEIGHT   64
#define DPU_MATCH_OUT_IMAGE_MAX_WIDTH   1920
#define DPU_MATCH_OUT_IMAGE_MAX_HEIGHT  1080
#define DPU_MATCH_OUT_IMAGE_MIN_WIDTH   128
#define DPU_MATCH_OUT_IMAGE_MIN_HEIGHT  64

/* For Gdc */
#define HI_FISHEYE_MIN_IN_IMAGE_WIDTH      1920
#define HI_FISHEYE_MIN_IN_IMAGE_HEIGHT     1080
#define HI_FISHEYE_MIN_OUT_IMAGE_WIDTH     640
#define HI_FISHEYE_MIN_OUT_IMAGE_HEIGHT    360
// MAX width and height change s00605447
#define HI_FISHEYE_MAX_OUT_IMAGE_WIDTH    8192
#define HI_FISHEYE_MAX_OUT_IMAGE_HEIGHT   8192
#define LDC_MIN_IMAGE_WIDTH             640
#define LDC_MIN_IMAGE_HEIGHT            480

#define SPREAD_MIN_IMAGE_WIDTH          640
#define SPREAD_MIN_IMAGE_HEIGHT         480

#define PMF_MIN_IMAGE_WIDTH             480
#define PMF_MIN_IMAGE_HEIGHT            360

#define ROTATION_EX_MIN_IMAGE_WIDTH     480
#define ROTATION_EX_MIN_IMAGE_HEIGHT    360

#define HI_GDC_MAX_IMAGE_WIDTH             8192
#define HI_GDC_MAX_IMAGE_HEIGHT            8192
#define GDC_IP_NUM                      1
#define GDC_FUSION_NUM                  9

/* For VGS */
#define VGS_IP_NUM                      1
#define VGS0                            0
#define VGS1                            1
#define VGS_MAX_COVER_NUM               1
#define VGS_MAX_OSD_NUM                 1

/* For MCF */
#define MCF_MAX_GRP_NUM                 1 /* max grp num */
#define MCF_PIPE_NUM                    2 /* max pipe num */
#define MCF_MAX_CHN_NUM                 1 /* max chn num */

#if defined(DVPP_ML) && !defined(DVPP_MLV1R3)
#define MAX_VF_ID_NUM 13U // 0是物理机，1~12表示虚拟机, 910B上VF的上限是12个VF
#else
#define MAX_VF_ID_NUM 9U // 0是物理机，1~8表示虚拟机,VF的上限是8个VF
#endif

#ifdef DVPP_ML
#define MAX_CORE_NUM 32U // cloud是32个
#else
#define MAX_CORE_NUM 8U // DC/MDC PG1是8个
#endif // #ifdef DVPP_ML


#define QUAD_POINT_NUM                  4


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HI_DEFINES_H__ */

