#ifndef __HI_MIPI_RX_H__
#define __HI_MIPI_RX_H__

#include <linux/types.h>
#include <linux/platform_device.h>
#include <linux/ioctl.h>

typedef unsigned int combo_dev_t;
typedef unsigned int sns_clk_freq_t;
typedef unsigned int sns_rst_source_t;

typedef enum { INPUT_MODE_MIPI=0, INPUT_MODE_SUBLVDS, INPUT_MODE_LVDS, INPUT_MODE_HISPI, INPUT_MODE_SLVS, INPUT_MODE_BUTT } input_mode_t;
typedef enum { DATA_TYPE_RAW_8BIT=0, DATA_TYPE_RAW_10BIT, DATA_TYPE_RAW_12BIT, DATA_TYPE_RAW_14BIT, DATA_TYPE_RAW_16BIT, DATA_TYPE_YUV420_8BIT_NORMAL, DATA_TYPE_BUTT } data_type_t;
typedef enum { LANE_DIVIDE_MODE_0=0, LANE_DIVIDE_MODE_1, LANE_DIVIDE_MODE_BUTT } lane_divide_mode_t;
typedef enum { MIPI_WDR_MODE_NONE=0, MIPI_WDR_MODE_VC, MIPI_WDR_MODE_DT, MIPI_WDR_MODE_DOL, MIPI_WDR_MODE_BUTT } mipi_wdr_mode_t;
typedef enum { WDR_MODE_NONE=0, WDR_MODE_2To1_LINE, WDR_MODE_2To1_FRAME, WDR_MODE_BUTT } wdr_mode_t;
typedef enum { LVDS_WDR_MODE_NONE=0, LVDS_WDR_MODE_2F, LVDS_WDR_MODE_BUTT } lvds_wdr_mode_t;
typedef enum { LVDS_SYNC_MODE_SOF=0, LVDS_SYNC_MODE_SAV, LVDS_SYNC_MODE_BUTT } lvds_sync_mode_t;
typedef enum { LVDS_ENDIAN_LITTLE=0, LVDS_ENDIAN_BIG, LVDS_ENDIAN_BUTT } lvds_bit_endian_t;
typedef enum { MIPI_DATA_RATE_X1=0, MIPI_DATA_RATE_X2, MIPI_DATA_RATE_BUTT } mipi_data_rate_t;
typedef enum { SLVS_LANE_RATE_X1=0, SLVS_LANE_RATE_X2, SLVS_LANE_RATE_BUTT } slvs_lane_rate_t;

typedef struct { int x; int y; int width; int height; } img_rect_t;
typedef struct { int width; int height; } img_size_t;
/* lvds_sync_code_cfg_t defined in mipi_rx_reg.h - not here */
typedef struct { int attr; } lvds_vsync_attr_t;
typedef struct { int fid_type; int output_fil; } lvds_fid_attr_t;
typedef struct { unsigned int clk_src; unsigned int clk_freq; } sns_clk_cfg_t;

#define SNS_MAX_CLK_SOURCE_NUM 4
#define SNS_MAX_RST_SOURCE_NUM 4
#define MIPI_LANE_NUM 8
#define LVDS_LANE_NUM 16
#define SLVS_LANE_NUM 8
#define WDR_VC_NUM 4
#define SYNC_CODE_NUM 16
#define MEIDA_BLOCK_BOX_BLOCK_ID 0
#define CAMERA_MODID_MIPI_START_FAIL 0

typedef struct { combo_dev_t devno; input_mode_t input_mode; mipi_data_rate_t data_rate; img_rect_t img_rect; } combo_dev_attr_t;
typedef struct { data_type_t input_data_type; wdr_mode_t wdr_mode; int lane_id[LVDS_LANE_NUM]; lvds_bit_endian_t data_endian; lvds_bit_endian_t sync_code_endian; lvds_vsync_attr_t vsync_attr; lvds_fid_attr_t fid_attr; } lvds_dev_attr_t;
typedef struct { data_type_t input_data_type; mipi_wdr_mode_t wdr_mode; int lane_id[MIPI_LANE_NUM]; } mipi_dev_attr_t;

#define HI_MIPI_SET_DEV_ATTR      _IOW('m', 0x01, combo_dev_attr_t)
#define HI_MIPI_SET_PHY_CMVMODE   _IOW('m', 0x04, unsigned int)
#define HI_MIPI_SET_HS_MODE       _IOW('m', 0x05, lane_divide_mode_t)
#define HI_MIPI_ENABLE_MIPI_CLOCK _IOW('m', 0x08, unsigned int)
#define HI_MIPI_DISABLE_MIPI_CLOCK _IOW('m', 0x09, unsigned int)
#define HI_MIPI_ENABLE_SENSOR_CLOCK _IOW('m', 0x10, unsigned int)
#define HI_MIPI_DISABLE_SENSOR_CLOCK _IOW('m', 0x11, unsigned int)
#define HI_MIPI_SENSOR_RESET     _IOW('m', 0x12, unsigned int)
#define HI_MIPI_SENSOR_UNRESET   _IOW('m', 0x13, unsigned int)

#endif

/* OT SDK name aliases */
#define OT_MIPI_WDR_MODE_NONE MIPI_WDR_MODE_NONE
#define OT_MIPI_WDR_MODE_VC MIPI_WDR_MODE_VC
#define OT_MIPI_WDR_MODE_DT MIPI_WDR_MODE_DT
#define OT_MIPI_WDR_MODE_DOL MIPI_WDR_MODE_DOL
#define OT_MIPI_WDR_MODE_BUTT MIPI_WDR_MODE_BUTT
#define OT_INPUT_MODE_MIPI INPUT_MODE_MIPI
#define OT_INPUT_MODE_SUBLVDS INPUT_MODE_SUBLVDS
#define OT_INPUT_MODE_LVDS INPUT_MODE_LVDS
#define OT_INPUT_MODE_HISPI INPUT_MODE_HISPI
#define OT_INPUT_MODE_SLVS INPUT_MODE_SLVS
#define OT_INPUT_MODE_BUTT INPUT_MODE_BUTT
#define OT_DATA_TYPE_RAW_8BIT DATA_TYPE_RAW_8BIT
#define OT_DATA_TYPE_RAW_10BIT DATA_TYPE_RAW_10BIT
#define OT_DATA_TYPE_RAW_12BIT DATA_TYPE_RAW_12BIT
#define OT_DATA_TYPE_RAW_14BIT DATA_TYPE_RAW_14BIT
#define OT_DATA_TYPE_RAW_16BIT DATA_TYPE_RAW_16BIT
#define OT_DATA_TYPE_BUTT DATA_TYPE_BUTT
#define OT_LVDS_ENDIAN_LITTLE LVDS_ENDIAN_LITTLE
#define OT_LVDS_ENDIAN_BIG LVDS_ENDIAN_BIG
#define OT_MIPI_DATA_RATE_X1 MIPI_DATA_RATE_X1
#define OT_MIPI_DATA_RATE_X2 MIPI_DATA_RATE_X2
#define OT_LVDS_SYNC_MODE_SOF LVDS_SYNC_MODE_SOF
#define OT_LVDS_SYNC_MODE_SAV LVDS_SYNC_MODE_SAV
#define OT_WDR_MODE_NONE WDR_MODE_NONE
#define OT_WDR_MODE_2To1_LINE WDR_MODE_2To1_LINE
#define OT_WDR_MODE_2To1_FRAME WDR_MODE_2To1_FRAME
#define OT_LVDS_WDR_MODE_NONE LVDS_WDR_MODE_NONE
#define OT_LVDS_WDR_MODE_2F LVDS_WDR_MODE_2F
