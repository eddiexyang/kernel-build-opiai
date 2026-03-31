#ifndef __HI_MIPI_RX_H__
#define __HI_MIPI_RX_H__

#include <linux/types.h>
#include <linux/platform_device.h>
#include <linux/ioctl.h>

typedef void hi_void;
typedef signed char hi_s8;
typedef unsigned char hi_u8;
typedef int hi_s32;
typedef unsigned int hi_u32;
typedef unsigned long hi_ulong;

typedef unsigned int combo_dev_t;
typedef unsigned int sns_clk_source_t;
typedef unsigned int sns_clk_freq_t;
typedef unsigned int sns_rst_source_t;

typedef enum { INPUT_MODE_MIPI=0, INPUT_MODE_SUBLVDS, INPUT_MODE_LVDS, INPUT_MODE_HISPI, INPUT_MODE_SLVS, INPUT_MODE_BUTT } input_mode_t;
typedef enum {
    DATA_TYPE_RAW_8BIT = 0,
    DATA_TYPE_RAW_10BIT,
    DATA_TYPE_RAW_12BIT,
    DATA_TYPE_RAW_14BIT,
    DATA_TYPE_RAW_16BIT,
    DATA_TYPE_YUV420_8BIT_NORMAL,
    DATA_TYPE_YUV420_8BIT_LEGACY,
    DATA_TYPE_YUV422_8BIT,
    DATA_TYPE_YUV422_PACKED,
    DATA_TYPE_BUTT
} data_type_t;
typedef enum {
    LANE_DIVIDE_MODE_0 = 0,
    LANE_DIVIDE_MODE_1,
    LANE_DIVIDE_MODE_2,
    LANE_DIVIDE_MODE_3,
    LANE_DIVIDE_MODE_BUTT
} lane_divide_mode_t;
typedef enum { MIPI_WDR_MODE_NONE=0, MIPI_WDR_MODE_VC, MIPI_WDR_MODE_DT, MIPI_WDR_MODE_DOL, MIPI_WDR_MODE_BUTT } mipi_wdr_mode_t;
typedef enum { WDR_MODE_NONE=0, WDR_MODE_2To1_LINE, WDR_MODE_2To1_FRAME, WDR_MODE_BUTT } wdr_mode_t;
typedef enum {
    SENSOR_CLK_74P25MHz = 0,
    SENSOR_CLK_72MHz,
    SENSOR_CLK_54MHz,
    SENSOR_CLK_50MHz,
    SENSOR_CLK_24MHz,
    SENSOR_CLK_37P125MHz,
    SENSOR_CLK_36MHz,
    SENSOR_CLK_27MHz,
    SENSOR_CLK_25MHz,
    SENSOR_CLK_12MHz,
    SENSOR_CLK_FREQ_BUTT
} sns_clk_freq_enum_t;
typedef enum {
    LVDS_WDR_MODE_NONE = 0,
    LVDS_WDR_MODE_2F,
    LVDS_WDR_MODE_3F,
    LVDS_WDR_MODE_4F,
    LVDS_WDR_MODE_DOL_2F,
    LVDS_WDR_MODE_DOL_3F,
    LVDS_WDR_MODE_DOL_4F,
    LVDS_WDR_MODE_BUTT
} lvds_wdr_mode_t;
typedef enum { LVDS_SYNC_MODE_SOF=0, LVDS_SYNC_MODE_SAV, LVDS_SYNC_MODE_BUTT } lvds_sync_mode_t;
typedef enum { LVDS_ENDIAN_LITTLE=0, LVDS_ENDIAN_BIG, LVDS_ENDIAN_BUTT } lvds_bit_endian_t;
typedef enum { MIPI_DATA_RATE_X1=0, MIPI_DATA_RATE_X2, MIPI_DATA_RATE_BUTT } mipi_data_rate_t;
typedef enum { SLVS_LANE_RATE_LOW=0, SLVS_LANE_RATE_HIGH, SLVS_LANE_RATE_BUTT } slvs_lane_rate_t;
typedef enum { LVDS_VSYNC_NORMAL=0, LVDS_VSYNC_SHARE, LVDS_VSYNC_HCONNECT, LVDS_VSYNC_BUTT } lvds_vsync_type_t;
typedef enum { LVDS_FID_NONE=0, LVDS_FID_IN_SAV, LVDS_FID_IN_DATA, LVDS_FID_BUTT } lvds_fid_type_t;
typedef enum {
    SLVS_ERR_CHECK_MODE_NONE = 0,
    SLVS_ERR_CHECK_MODE_CRC,
    SLVS_ERR_CHECK_MODE_ECC_2BYTE,
    SLVS_ERR_CHECK_MODE_ECC_4BYTE,
    SLVS_ERR_CHECK_MODE_BUTT
} slvs_err_check_mode_t;

typedef struct { int x; int y; int width; int height; } img_rect_t;
typedef struct { int width; int height; } img_size_t;
/* lvds_sync_code_cfg_t defined in mipi_rx_reg.h - not here */
typedef struct {
    lvds_vsync_type_t sync_type;
    unsigned int hblank1;
    unsigned int hblank2;
} lvds_vsync_attr_t;
typedef struct {
    lvds_fid_type_t fid_type;
    int output_fil;
} lvds_fid_attr_t;
typedef struct { sns_clk_source_t clk_source; sns_clk_freq_t clk_freq; } sns_clk_cfg_t;

#define SNS_MAX_CLK_SOURCE_NUM 4
#define SNS_MAX_RST_SOURCE_NUM 4
#define MIPI_LANE_NUM 8
#define LVDS_LANE_NUM 16
#define SLVS_LANE_NUM 8
#define WDR_VC_NUM 4
#define SYNC_CODE_NUM 16
#define MIPI_RX_MAX_DEV_NUM 4U
#define MEIDA_BLOCK_BOX_BLOCK_ID 0
#define CAMERA_MODID_MIPI_START_FAIL 0

typedef struct {
    data_type_t input_data_type;
    mipi_wdr_mode_t wdr_mode;
    short lane_id[MIPI_LANE_NUM];
    short data_type[WDR_VC_NUM];
} mipi_dev_attr_t;
typedef struct {
    data_type_t input_data_type;
    lvds_wdr_mode_t wdr_mode;
    short lane_id[LVDS_LANE_NUM];
    lvds_sync_mode_t sync_mode;
    lvds_bit_endian_t data_endian;
    lvds_bit_endian_t sync_code_endian;
    lvds_vsync_attr_t vsync_attr;
    lvds_fid_attr_t fid_attr;
    unsigned short sync_code[LVDS_LANE_NUM][WDR_VC_NUM][SYNC_CODE_NUM];
} lvds_dev_attr_t;
typedef struct {
    data_type_t input_data_type;
    wdr_mode_t wdr_mode;
    short lane_id[SLVS_LANE_NUM];
    slvs_lane_rate_t lane_rate;
    slvs_err_check_mode_t err_check_mode;
    int sensor_valid_width;
} slvs_dev_attr_t;
typedef struct {
    combo_dev_t devno;
    input_mode_t input_mode;
    mipi_data_rate_t data_rate;
    img_rect_t img_rect;
    mipi_dev_attr_t mipi_attr;
    lvds_dev_attr_t lvds_attr;
    slvs_dev_attr_t slvs_attr;
} combo_dev_attr_t;

#define HI_MIPI_SET_DEV_ATTR      _IOW('m', 0x01, combo_dev_attr_t)
#define HI_MIPI_SET_PHY_CMVMODE     _IOW('m', 0x03, unsigned int)
#define HI_MIPI_SET_HS_MODE         _IOW('m', 0x04, lane_divide_mode_t)
#define HI_MIPI_RESET_SENSOR        _IOW('m', 0x05, sns_rst_source_t)
#define HI_MIPI_UNRESET_SENSOR      _IOW('m', 0x06, sns_rst_source_t)
#define HI_MIPI_RESET_MIPI          _IOW('m', 0x07, combo_dev_t)
#define HI_MIPI_UNRESET_MIPI        _IOW('m', 0x08, combo_dev_t)
#define HI_MIPI_RESET_SLVS          _IOW('m', 0x09, combo_dev_t)
#define HI_MIPI_UNRESET_SLVS        _IOW('m', 0x0A, combo_dev_t)
#define HI_MIPI_ENABLE_MIPI_CLOCK   _IOW('m', 0x0C, combo_dev_t)
#define HI_MIPI_DISABLE_MIPI_CLOCK  _IOW('m', 0x0D, combo_dev_t)
#define HI_MIPI_ENABLE_SLVS_CLOCK   _IOW('m', 0x0E, combo_dev_t)
#define HI_MIPI_DISABLE_SLVS_CLOCK  _IOW('m', 0x0F, combo_dev_t)
#define HI_MIPI_ENABLE_SENSOR_CLOCK _IOW('m', 0x10, sns_clk_source_t)
#define HI_MIPI_DISABLE_SENSOR_CLOCK _IOW('m', 0x11, sns_clk_source_t)
#define HI_MIPI_CONFIG_SENSOR_CLOCK _IOW('m', 0x17, sns_clk_cfg_t)

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
#define OT_LVDS_WDR_MODE_3F LVDS_WDR_MODE_3F
#define OT_LVDS_WDR_MODE_4F LVDS_WDR_MODE_4F
#define OT_LVDS_WDR_MODE_DOL_2F LVDS_WDR_MODE_DOL_2F
#define OT_LVDS_WDR_MODE_DOL_3F LVDS_WDR_MODE_DOL_3F
#define OT_LVDS_WDR_MODE_DOL_4F LVDS_WDR_MODE_DOL_4F
#define OT_LVDS_WDR_MODE_BUTT LVDS_WDR_MODE_BUTT
#define HI_WDR_MODE_NONE WDR_MODE_NONE
#define HI_WDR_MODE_DOL_2F WDR_MODE_2To1_FRAME
#define WORK_MODE_SLVS INPUT_MODE_SLVS
