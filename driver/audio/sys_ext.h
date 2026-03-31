/*
 * Copyright (C) Hisilicon Technologies Co., Ltd. 2016-2019. All rights reserved.
 * Description: The internal interface for other modules
 * Author: Hisilicon multimedia software group
 * Create: 2016-11-15
 */

#ifndef __SYS_EXT_H__
#define __SYS_EXT_H__

#include "hi_type.h"
#include "mod_ext.h"
#include "hi_comm_video_adapt.h"
#include "hi_comm_sys_adapt.h"
#include "HiDvppQueryInfo.h"
#include "sys_ext_common.h"

#ifdef __LITEOS__
// 驱动头文件依赖了 linux 内核头文件
struct vmngd_client_instance;
enum devdrv_process_type;
#else
#include "virtmng_interface.h"
#include "devdrv_manager_comm.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#define SYS_INVALID_SUBSYS_ID         0xFF

#define SYS_DDR_MAXNUM                16

#define SYS_SEND_DATA_BLOCK_MASK      0x1U
#define SYS_SEND_DATA_BLOCK_OFFSET    0U
#define SYS_SEND_DATA_NOBLOCK         ((0U << SYS_SEND_DATA_BLOCK_OFFSET) & SYS_SEND_DATA_BLOCK_MASK)
#define SYS_SEND_DATA_BLOCK           ((1U << SYS_SEND_DATA_BLOCK_OFFSET) & SYS_SEND_DATA_BLOCK_MASK)

#define SYS_SEND_DATA_DELAY_MASK      0x6U
#define SYS_SEND_DATA_DELAY_OFFSET    1U
#define SYS_SEND_DATA_FINISH          ((0U << SYS_SEND_DATA_DELAY_OFFSET) & SYS_SEND_DATA_DELAY_MASK)
#define SYS_SEND_DATA_LOWDELAY        ((1U << SYS_SEND_DATA_DELAY_OFFSET) & SYS_SEND_DATA_DELAY_MASK)
#define SYS_SEND_DATA_LOWDELAY_FINISH ((2U << SYS_SEND_DATA_DELAY_OFFSET) & SYS_SEND_DATA_DELAY_MASK)

#define MAX_CPU_NUM_PER_DEVICE 16U

/* 256K */
#define SYS_DUMP_PROFILING_INFO_LEN 262144UL

#ifdef DVPP_ML
#define SMMU_ENABLE (0x4075U)
#else
#define SMMU_ENABLE (0x40001055U)
#endif // #ifdef DVPP_ML

#define SMMU_BYPASS (0x40080055U)

#define DVPP_MAX_IP_NUM_PER_DEVICE (56U) // 当前使用JPEGD的最大NUM为56
#define DVPP_MAX_THREAD_PER_MODULE (56U) // 单模块线程数最大暂定为56
#define PM_EVENT_HELPER_LEN 128

#define DVPP_BOOTDOT_INVALID_CORE_ID (0xFFU)

typedef enum {
    PROCESS_CORE_INIT = 1U,
    PROCESS_CORE_SUSPEND,
    PROCESS_CORE_RESUME,
    PROCESS_TYPE_BUTT,
} PROCESS_TYPE;

// core_id为0xFF时表示与core无关的状态
__inline static hi_u32 transform_block_status(PROCESS_TYPE process_type, hi_u8 core_id, hi_u8 status)
{
    return ((((hi_u32)process_type) << 16U) | (((hi_u32)core_id) << 8U) | ((hi_u32)status));
}

/* @Fake_sys_ext_head_01 */
/* @Fake_sys_ext_head_01_TAIL */

typedef enum {
    IRQ_NOR = 0,
    IRQ_ERR = 1,
    IRQ_TYPE_BUTT,
} IRQ_TYPE;

typedef enum {
    DVPP_IP_TYPE_VPC = 0,
    DVPP_IP_TYPE_JPEGD = 1,
    DVPP_IP_TYPE_VDEC = 2,
    DVPP_IP_TYPE_VENC = 3,
    DVPP_IP_TYPE_JPEGE = 4,
    DVPP_IP_TYPE_BUTT,
} DVPP_IP_TYPE;

typedef enum {
    CPU_TYPE_CTL = 0,
    CPU_TYPE_AI = 1,
    CPU_TYPE_BUTT,
} CPU_TYPE;

typedef struct {
    DVPP_IP_TYPE ip_type;
    IRQ_TYPE irq_type;
    hi_u32 irq;
    hi_u32 core_id;
} dvpp_core_info;

typedef struct {
    hi_u32 num;
    hi_void *mmz_name[SYS_DDR_MAXNUM];
} mpp_sys_ddr_name;

typedef struct {
    hi_s32 (*set_vpc_csc_param)(const hi_csc_param *csc_param);
    hi_s32 (*set_vdec_csc_param)(const hi_csc_param *csc_param);
} dvpp_set_csc_param;

typedef struct {
    hi_s32 (*get_vpc_csc_param)(hi_csc_param *csc_param);
    hi_s32 (*get_vdec_csc_param)(hi_csc_param *csc_param);
} dvpp_get_csc_param;

typedef struct {
    hi_mod_id mod_id;
    hi_pixel_format pixel_format;
    hi_u32 width;
    hi_u32 height;
    hi_u32 buf_line;
    hi_u32 align;  /* 0:auto */
    hi_dynamic_range dynamic_range;
    hi_compress_mode compress_mode;
    hi_video_format video_format;
    hi_bool is_3dnr_buffer;
} vb_base_info;

typedef enum {
    CORE_FULL_GOOD = 0,
    CORE_PARTIAL_GOOD,
    CORE_NOT_GOOD,
    CORE_USABLE_MODE_BUTT
} core_usable_mode;

typedef struct {
    core_usable_mode usable_mode;
    hi_u32 core_max_num;
    hi_u32 core_usable_num;
    hi_u64 core_is_usable;
} core_usable_info;

typedef struct {
    hi_u32 vf_aicpu_bitmap;
    hi_u32 vfg_aicpu_bitmap;
    hi_u32 vf_aicpu_bind_index[MAX_CPU_NUM_PER_DEVICE];
    hi_u32 vf_aicpu_num;
} dvpp_vf_bitmap_info;

typedef enum {
    SYS_VO_BT1120_CLK_SEL,
    SYS_VO_HDDAC_CLK_SEL,
    SYS_VO_HDDATE_CLK_SEL,
    SYS_VO_HDMI_CLK_SEL,
    SYS_VO_GET_HD_CLK_SEL,
    SYS_VO_BT1120_CLK_EN,
    SYS_VO_MIPI_TX_CLK_EN,
    SYS_VO_MIPI_CLK_EN,
    SYS_VO_HDMI_TX_CLK_EN,
    SYS_VO_MIPI_CLK_SEL,
    SYS_VO_MIPI_CHN_SEL,
    SYS_VO_PLL_FRAC_SET,
    SYS_VO_PLL_POSTDIV1_SET,
    SYS_VO_PLL_POSTDIV2_SET,
    SYS_VO_PLL_REFDIV_SET,
    SYS_VO_PLL_FBDIV_SET,
    SYS_VO_SPLL_FRAC_SET,
    SYS_VO_SPLL_POSTDIV1_SET,
    SYS_VO_SPLL_POSTDIV2_SET,
    SYS_VO_SPLL_REFDIV_SET,
    SYS_VO_SPLL_FBDIV_SET,

    SYS_AI_SAMPLECLK_DIVSEL,
    SYS_AI_BITCKL_DIVSEL,
    SYS_AI_SYSCKL_SEL,
    SYS_AI_MASTER,
    SYS_AI_RESET_SEL,
    SYS_AI_CLK_EN,

    SYS_AO_SAMPLECLK_DIVSEL,
    SYS_AO_BITCKL_DIVSEL,
    SYS_AO_SYSCKL_SEL,
    SYS_AO_MASTER,
    SYS_AO_RESET_SEL,
    SYS_AO_CLK_EN,

    SYS_AIO_RESET_SEL,
    SYS_AIO_CLK_EN,
    SYS_AIO_CLK_SEL,

    SYS_AIO_RX0SD_SEL,
    SYS_AIO_HDMISD_SEL,
    SYS_AIO_PADSD_SEL,
    SYS_AIO_CODECSD_SEL,
    SYS_AIO_HDMICLK_SEL,
    SYS_AIO_PADCLK_SEL,
    SYS_AIO_CODECCLK_SEL,

    SYS_ACODEC_RESET_SEL,

    SYS_VI_BUS_RESET_SEL,
    SYS_VI_BUS_CLK_EN,
    SYS_VI_PPC_RESET_SEL,
    SYS_VI_PPC_CLK_EN,
    SYS_VI_PORT_RESET_SEL,
    SYS_VI_PORT_CLK_EN,
    SYS_VI_PPC_PORT_CLK_EN,
    SYS_VI_PPC_CHN_CLK_EN,
    SYS_VI_DEV_RESET_SEL,
    SYS_VI_DEV_CLK_EN,
    SYS_VI_PROC_RESET_SEL,
    SYS_VI_PROC_CLK_EN,
    SYS_VI_PROC_PWR_EN,
    SYS_VI_PROC_PWR_ISO,

    SYS_VI_ISP_CFG_RESET_SEL,
    SYS_VI_ISP_CORE_RESET_SEL,
    SYS_VI_ISP_CLK_EN,
    SYS_VI_CAP_GET_ONLINE_FLAG,
    SYS_VI_CAP_PROC_ONLINE_SEL,
    SYS_VI_PROC_VPSS_ONLINE_SEL,
    SYS_VI_DEV_BIND_MIPI_SEL,
    SYS_VI_DEV_BIND_SLVS_SEL,
    SYS_VI_DEV_INPUT_SEL,
    SYS_VI_DEV_GET_PORT_CLK,
    SYS_VI_DEV_SET_PORT_CLK,
    SYS_VI_DEV_SET_PT_PPC_CLK,
    SYS_VI_DEV_GET_ISP_CLK,
    SYS_VI_PROC_GET_CLK,
    SYS_ISPFE_SET_CLK,
    SYS_ISPFE_GET_CLK,
    SYS_SENSOR_CLK_OUT_SEL,

    SYS_VO_PLL_POWER_CTRL,
    SYS_VO_BUS_RESET_SEL,
    SYS_VO_CFG_CLK_EN,
    SYS_VO_APB_CLK_EN,
    SYS_VO_BUS_CLK_EN,
    SYS_VO_HD_CLKOUT_PHASIC_REVERSE_EN,
    SYS_VO_OUT_DLY_TUNE,
    SYS_VO_OUT_HD_DIV,
    SYS_VO_DEV_DIV_MOD,
    SYS_VO_PPC_CFG_CLK_SEL,
    SYS_VO_PPC_CLK_SEL,
    SYS_VO_PPC_SC_CLK_SEL,
    SYS_VO_HD_CLK_SEL,
    SYS_VO_CORE_CLK_EN,
    SYS_VO_DEV_CLK_EN,
    SYS_VO_DEV_DATE_CLK_EN,
    SYS_VO_SD_DATE_CLK_EN,
    SYS_VO_SD_CLK_SEL,
    SYS_VO_LCD_CLK_EN,
    SYS_VO_LCD_MCLK_DIV,
    SYS_VO_DAC_CLK_PHASIC_REVERSE_EN,
    SYS_VO_HD_DAC_SEL,
    SYS_VO_DEV_DAC_EN,
    SYS_VO_HDMI_SSC_VDP_DIV,
    SYS_VO_HDMI_CLK_EN,
    SYS_VO_HDMI1_CLK_EN,
    SYS_VO_HDMI1_CLK_SEL,

    SYS_HDMI_RESET_SEL,
    SYS_HDMI_PIXEL_CLK_EN,
    SYS_HDMI_BUS_CLK_EN,

    SYS_VO_LOW_POWER_CTRL,

    SYS_VEDU_RESET_SEL,
    SYS_VEDU_CLK_EN,
    SYS_VEDU_CLK_SEL,
    SYS_VEDU_SED_RESET_SEL,
    SYS_VEDU_SED_CLK_EN,
    SYS_VEDU_PWR_EN,
    SYS_VEDU_PWR_ISO,
    SYS_VEDU_PWR_STATE,

    SYS_VPSS_CLK_SEL,
    SYS_VPSS_RESET_SEL,
    SYS_VPSS_CLK_EN,
    SYS_VPSS_GET_CLK_SEL,
    SYS_VPSS_SET_CLK_SEL,
    SYS_AVS_RESET_SEL,
    SYS_AVS_CLK_EN,
    SYS_AVS_PWR_EN,
    SYS_AVS_PWR_ISO,
    SYS_AVS_PWR_STATE,

    SYS_TDE_RESET_SEL,
    SYS_TDE_CLK_EN,

    SYS_JPGE_RESET_SEL,
    SYS_JPGE_CLK_EN,
    SYS_JPGE_SEL_CLK,

    SYS_JPGD_CLOCK_SEL,
    SYS_JPGD_RESET_SEL,
    SYS_JPGD_CLK_EN,

    SYS_MD_RESET_SEL,
    SYS_MD_CLK_EN,

    SYS_IVE_RESET_SEL,
    SYS_IVE_CLK_EN,

    SYS_FD_RESET_SEL,
    SYS_FD_CLK_EN,

    SYS_DSP_CLK_SET,
    SYS_DSP0_CORE_RESET_SEL,
    SYS_DSP0_CFG_RESET_SEL,
    SYS_DSP0_DBG_RESET_SEL,
    SYS_DSP0_CLK_EN,
    SYS_DSP0_POWER_EN,
    SYS_DSP0_POWER_STATE,
    SYS_DSP1_CORE_RESET_SEL,
    SYS_DSP1_CFG_RESET_SEL,
    SYS_DSP1_DBG_RESET_SEL,
    SYS_DSP1_CLK_EN,
    SYS_DSP1_POWER_EN,
    SYS_DSP1_POWER_STATE,
    SYS_DSP2_CORE_RESET_SEL,
    SYS_DSP2_CFG_RESET_SEL,
    SYS_DSP2_DBG_RESET_SEL,
    SYS_DSP2_CLK_EN,
    SYS_DSP2_POWER_EN,
    SYS_DSP2_POWER_STATE,
    SYS_DSP3_CORE_RESET_SEL,
    SYS_DSP3_CFG_RESET_SEL,
    SYS_DSP3_DBG_RESET_SEL,
    SYS_DSP3_CLK_EN,
    SYS_DSP3_POWER_EN,
    SYS_DSP3_POWER_STATE,

    SYS_EV_CNN_RESET_SEL,
    SYS_EV_CNN_CFG_RESET_SEL,
    SYS_EV_CNN_CLK_EN,

    SYS_WK_CNN_RESET_SEL,
    SYS_WK_CNN_CLK_EN,
    SYS_WK_CNN_GET_CLK_STATE,
    SYS_WK_CNN_CLK_SET,
    SYS_WK_CNN_CLK_GET,
    SYS_WK_CNN_PWR_EN,
    SYS_WK_CNN_PWR_ISO,
    SYS_WK_CNN_PWR_STATE,
    SYS_WK_CNN1_RESET_SEL,
    SYS_WK_CNN1_CLK_EN,
    SYS_WK_CNN1_CLK_SET,
    SYS_WK_CNN1_CLK_GET,
    SYS_WK_CNN1_PWR_EN,
    SYS_WK_CNN1_PWR_ISO,
    SYS_WK_CNN1_PWR_STATE,
    SYS_DPU_CLK_SET,
    SYS_DPU_RESET_SEL,
    SYS_DPU_CLK_EN,

    SYS_PCIE_RESET_SEL,
    SYS_PCIE_CLK_EN,

    SYS_CIPHER_RESET_SEL,
    SYS_CIPHER_CLK_EN,

    SYS_VGS_RESET_SEL,
    SYS_VGS_CLK_EN,
    SYS_VGS_GET_CLK_SEL,
    SYS_VGS_SET_CLK_SEL,
    SYS_VGS_BOOTROOM_RAM_USE,

    SYS_GDC_RESET_SEL,
    SYS_GDC_CLK_EN,
    SYS_GDC_GET_CLK_STATE,
    SYS_GDC_NNIE_RAM_USE,
    SYS_GDC_GET_CLKSEL,
    SYS_GDC_SET_CLKSEL,

    SYS_GDC_VGS_PWR_EN,
    SYS_GDC_VGS_PWR_ISO,
    SYS_GDC_VGS_PWR_STATE,

    SYS_VPSS_PWR_EN,
    SYS_VPSS_PWR_ISO,
    SYS_VPSS_PWR_STATE,

    SYS_DIS_RESET_SEL,
    SYS_DIS_CLK_EN,

    SYS_GDC_NNIE_MUTEX_SEL,
    SYS_VENC_NNIE_MUTEX_SEL,
    SYS_NNIE_GET_MUTEX_STATE,
    SYS_NNIE_GDC_GET_MUTEX_STATE,

    SYS_GET_SYS,
    SYS_GET_SCT,
    SYS_HDMI0_CTRL_RESET_SEL,
    SYS_HDMI0_PHY_RESET_SEL,
    SYS_HDMI1_CTRL_RESET_SEL,
    SYS_HDMI1_PHY_RESET_SEL,
    SYS_BUTT,
} sys_func;

typedef enum {
    SYS_PLAT_ASIC,
    SYS_PLAT_FPGA,
    SYS_PLAT_EMU,
    SYS_PLAT_ESL
} sys_platform_type;

#ifdef CONFIG_HI_TZASC_SUPPORT
typedef enum {
    NON_TEE_VERSION = 0,
    TEE_VERSION = 1,
} sys_security_version;
#endif

typedef hi_void fn_sys_proc_show(const osal_proc_entry_t *s);
typedef hi_s32 fn_sys_dump_info(hi_mod_id mod_id, void *buffer, hi_ulong buffer_len);

typedef struct {
    hi_u64 (*pfn_sys_get_time_stamp)(hi_void);
    hi_u64 (*pfn_sys_get_local_time_stamp)(hi_void);
    hi_void (*pfn_sys_sync_time_stamp)(hi_u64 base, hi_bool init);
    hi_u32 (*pfn_sys_get_chip_version)(hi_void);
    hi_s32 (*pfn_sys_get_stride)(hi_u32 width, hi_u32 *stride);
    hi_s32 (*pfn_sys_drv_ioctrl)(hi_mpp_chn *mpp_chn, sys_func func_id, hi_void *io_args);

    hi_s32 (*pfn_sys_register_sender)(bind_sender_info *info);
    hi_s32 (*pfn_sys_unregister_sender)(hi_mod_id mod_id);
    hi_s32 (*pfn_sys_register_receiver)(bind_receiver_info *info);
    hi_s32 (*pfn_sys_unregister_receiver)(hi_mod_id mod_id);
    hi_s32 (*pfn_sys_send_data)(hi_mod_id mod_id, hi_s32 dev_id, hi_s32 chn_id, hi_u32 flag,
        mpp_data_type data_type, hi_void *pv_data);
    hi_s32 (*pfn_sys_reset_data)(hi_mod_id mod_id, hi_s32 dev_id, hi_s32 chn_id, hi_void *v_data);

    hi_s32 (*pfn_get_bind_by_src)(hi_mpp_chn *src_chn, hi_mpp_bind_dest *bind_src);
    hi_s32 (*pfn_get_bind_num_by_src)(hi_mpp_chn *src_chn, hi_u32 *bind_num);
    hi_s32 (*pfn_get_bind_by_dest)(hi_mpp_chn *dest_chn, hi_mpp_chn *src_chn);
    hi_s32 (*pfn_get_mmz_name)(hi_mpp_chn *chn, hi_void **pp_mmz_name);
    hi_s32 (*pfn_get_mem_ddr)(const hi_mpp_chn *chn, mpp_sys_ddr_name *mem_ddr);
    hi_s32 (*pfn_get_spin_rec)(hi_u32 *spin_rec);

    hi_u32 (*pfn_sys_vreg_read)(hi_u32 addr, hi_u32 bytes);
    hi_s32 (*pfn_sys_vreg_write)(hi_u32 addr, hi_u32 value, hi_u32 bytes);
    hi_u32 (*pfn_sys_get_vreg_addr)(hi_void);
    hi_u64 (*pfn_sys_get_sched_clock)(hi_void);

    hi_s32 (*pfn_sys_get_time_zone)(hi_s32 *time_zone);
    hi_s32 (*pfn_sys_get_now_string)(hi_u8 *now_time);
    hi_gps_info *(*pfn_sys_get_gps_info)(hi_void);

    hi_void (*pfn_sys_get_cmp_cfg)(hi_void *v_frame, hi_void *y_cmp_cfg, hi_void *cmp_cfg, void *cmp_mode_ex_param);

    hi_void (*pfn_sys_get_vb_cfg)(hi_void *vb_base_info, hi_void *vb_cal_config);
    hi_void (*pfn_sys_get_vdec_buffer_cfg)(hi_payload_type type, hi_void *vb_base_info, hi_void *vb_cal_config);

    hi_void (*pfn_sys_get_cmp_bayer_cfg)(hi_void *sys_cmp_in, hi_void *bayer_cmp_cfg, hi_void *bayer_dcmp_cfg);
    hi_void (*pfn_sys_get_cmp_3dnr_cfg)(hi_void *sys_cmp_in, hi_void *y_cmp_cfg, hi_void *cmp_cfg);
    hi_void (*pfn_sys_get_dcmp_muv1_cfg)(hi_void *sys_cmp_in, hi_void *cmp_out);
    hi_void (*pfn_sys_get_cmp_tde_cfg)(hi_void *sys_cmp_in, hi_void *cmp_out);
    hi_void (*pfn_sys_get_cmp_seg_cfg)(const hi_void *sys_cmp_in, hi_void *y_cmp_cfg, hi_void *cmp_cfg);

    sys_platform_type (*pfn_sys_get_platform_type)(hi_void);
    sys_chip_type (*pfn_sys_get_chip_type)(hi_void);
    hi_u32 (*pfn_sys_get_ip_num_per_device)(DVPP_IP_TYPE ip_type);
    hi_u32 (*pfn_sys_get_ip_num)(DVPP_IP_TYPE ip_type);
    hi_u8 (*pfn_sys_get_subsys_id)(hi_u64 phys_base_reg);
    dvpp_set_csc_param* (*pfn_sys_set_csc_param_callback)(hi_void);
    dvpp_get_csc_param* (*pfn_sys_get_csc_param_callback)(hi_void);

    hi_s32 (*pfn_sys_get_cpu_info)(hi_u32 device_id, hi_u32 *cpu_core_num, hi_u32 *cpu_bitmap);
    hi_s32 (*pfn_sys_get_cpu_mask)(hi_u32 device_id, hi_void *mask, CPU_TYPE get_cpu_type);
    hi_s32 (*pfn_sys_get_vf_or_vfg_aicpu_bitmap)(hi_u32 device_id, hi_u32 vf_id, hi_u32* aicpu_bitmap, hi_u32 is_vfg);
    hi_s32 (*pfn_sys_get_vf_or_vfg_mask)(hi_u32 device_id, hi_u32 vf_id, hi_void *mask, hi_u32 is_vfg);
    hi_s32 (*pfn_sys_set_vf_and_vfg_aicpu_bitmap)(hi_u32 device_id, hi_u32 vf_id,
        hi_u32 vf_aicpu_bitmap, hi_u32 vfg_aicpu_bitmap);
    hi_u32 (*pfn_sys_get_irq_aicpu_id_by_index)(hi_u32 device_id, hi_u32 vf_id, hi_u32 idx);
    hi_s32 (*pfn_sys_set_aicpu_bind_index)(hi_u32 device_id, hi_u32 vf_id);
    hi_u32 (*pfn_sys_get_cpu_id_by_index)(hi_u32 device_id, hi_u32 idx, CPU_TYPE get_cpu_type);

    hi_s32 (*pfn_sys_bind_irq_to_cpu)(dvpp_core_info core_info, hi_u32 cpu_id);
    hi_s32 (*pfn_sys_bind_irq_to_cpu_mask)(hi_u32 irq, const hi_void *mask);
    hi_s32 (*pfn_sys_unbind_irq_to_cpu)(hi_u32 irq);
    hi_void* (*pfn_sys_get_module_irq_cpu_mask)(hi_u32 dvpp_ip_type, hi_u32 irq_type, hi_u32 core_id);
    hi_void* (*pfn_sys_get_module_thread_cpu_mask)(hi_u32 dvpp_ip_type, hi_u32 thread_index);
    hi_s32 (*pfn_sys_query_process_host_pid)(int pid, hi_u32 *chip_id, hi_u32 *vfid,
        hi_u32 *host_pid, enum devdrv_process_type *cp_type);
    hi_s32 (*pfn_sys_drv_logical_id_to_physical_id)(hi_u32 dev_id, hi_u32 *pf_id, hi_u32 *vf_id);
    hi_s32 (*pfn_sys_vmngd_get_pfvf_id_by_devid)(hi_u32 dev_id, hi_u32 *pf_id, hi_u32 *vf_id);
    hi_s32 (*pfn_sys_mem_check_attribute)(pid_t devpid, hi_u64 va, hi_u64 size, hi_u32 attribute);

    hi_void (*pfn_sys_bootdot_init_block)(hi_u64 execption_id, PROCESS_TYPE process_type, hi_u8 core_id, hi_u8 status);
    hi_void (*pfn_sys_bootdot_set_block_status)(PROCESS_TYPE process_type, hi_u8 core_id, hi_u8 status);

    hi_s32 (*pfn_sys_get_core_usable_info)(DVPP_IP_TYPE core_type, core_usable_info *usable_info);
    hi_void (*pfn_sys_export_func_spin_lock)(hi_void);
    hi_void (*pfn_sys_export_func_spin_unlock)(hi_void);

    hi_s32 (*pfn_sys_print_proc_title)(osal_proc_entry_t *s, const hi_char *format, ...) \
        __attribute__((format(printf, 2, 3)));
    hi_u32 (*pfn_sys_get_scale_coef)(hi_mod_id mod_id, hi_void *scale_coef_opt, hi_void *pv_rate, hi_void *pv_cfg);
    hi_void (*pfn_sys_get_scale_coef_kva)(hi_void **scale_coef_addr, hi_u64 *coef_len);

    hi_s32 (*pfn_sys_set_device_num)(hi_u32 device_num);
    hi_void (*pfn_module_cpu_mask_init)(hi_void);
    hi_u32 (*pfn_sys_get_die_num_per_device)(hi_void);
    hi_s32 (*pfn_sys_get_aicpu_bitmap)(hi_u32 ext_phy_dev_id, hi_u32 *cpu_bitmap);

#ifdef CONFIG_HI_TZASC_SUPPORT
    hi_s32 (*pfn_sys_get_security_version)(sys_security_version *version);
#endif
    hi_void (*pfn_sys_regist_dvpp_proc_show)(fn_sys_proc_show *func);
    hi_void (*pfn_sys_unregist_dvpp_proc_show)(hi_void);
    hi_void (*pfn_sys_regist_dvpp_dump_info)(fn_sys_dump_info *func);
    hi_void (*pfn_sys_unregist_dvpp_dump_info)(hi_void);
} sys_export_func;

extern hi_s32 g_sys_state;

extern dvpp_set_csc_param g_dvpp_set_csc_param;
extern dvpp_get_csc_param g_dvpp_get_csc_param;

typedef hi_u32 fn_sys_get_limited_core_num(hi_void);
extern fn_sys_get_limited_core_num *g_dvpp_jpege_get_limted_core_num;

extern int32_t g_is_depend_sys;
extern hi_u32 g_device_num;
extern hi_char g_pm_mpp_helper[PM_EVENT_HELPER_LEN];

extern hi_s32 sys_do_mod_init(void);
extern void sys_do_mod_exit(void);

hi_u32 vdec_get_ip_num_per_device(void);
hi_u32 vdec_get_ip_num(void);
hi_u32 jpegd_get_ip_num_per_device(void);
hi_u32 jpege_get_ip_num_per_device(void);
hi_u32 jpegd_get_ip_num(void);

#define ckfn_sys_entry() CHECK_FUNC_ENTRY(HI_ID_SYS)

#define ckfn_sys_get_sched_clock() \
    (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_sched_clock != NULL))
#define call_sys_get_sched_clock() \
    FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_sched_clock()

#define ckfn_sys_get_time_stamp() \
    (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_time_stamp != NULL))
#define call_sys_get_time_stamp() \
    FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_time_stamp()

#define ckfn_sys_get_local_time_stamp() \
    (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_local_time_stamp != NULL))
#define call_sys_get_local_time_stamp() \
    FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_local_time_stamp()

#define ckfn_sys_sync_time_stamp() \
    (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_sync_time_stamp != NULL))
#define call_sys_sync_time_stamp(base, init) \
    FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_sync_time_stamp((base), (init))

#define ckfn_sys_get_stride() \
    (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_stride != NULL))
#define call_sys_get_stride(width, stride) \
    FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_stride((width), (stride))

#define ckfn_sys_get_chip_version() \
    (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_chip_version != NULL))
#define call_sys_get_chip_version() \
    FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_chip_version()

#define ckfn_sys_drv_ioctrl() \
    (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_drv_ioctrl != NULL))
#define call_sys_drv_ioctrl(mpp_chn, func_id, io_args) \
    FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_drv_ioctrl((mpp_chn), (func_id), (io_args))

#define ckfn_sys_register_sender() \
    (func_entry(sys_export_func, HI_ID_SYS)->pfn_sys_register_sender != HI_NULL)
#define call_sys_register_sender(info) \
    (func_entry(sys_export_func, HI_ID_SYS)->pfn_sys_register_sender(info))

#define ckfn_sys_unregister_sender() \
    (func_entry(sys_export_func, HI_ID_SYS)->pfn_sys_unregister_sender != HI_NULL)
#define call_sys_unregister_sender(mod_id) \
    (func_entry(sys_export_func, HI_ID_SYS)->pfn_sys_unregister_sender(mod_id))

#define ckfn_sys_register_receiver() \
    (func_entry(sys_export_func, HI_ID_SYS)->pfn_sys_register_receiver != HI_NULL)
#define call_sys_register_receiver(info) \
    (func_entry(sys_export_func, HI_ID_SYS)->pfn_sys_register_receiver(info))

#define ckfn_sys_unregister_receiver() \
    (func_entry(sys_export_func, HI_ID_SYS)->pfn_sys_unregister_receiver != HI_NULL)
#define call_sys_unregister_receiver(mod_id) \
    (func_entry(sys_export_func, HI_ID_SYS)->pfn_sys_unregister_receiver(mod_id))

#define ckfn_sys_send_data() \
    (func_entry(sys_export_func, HI_ID_SYS)->pfn_sys_send_data != HI_NULL)
#define call_sys_send_data(mod_id, dev_id, chn_id, flag, data_type, pv_data) \
    (func_entry(sys_export_func, HI_ID_SYS)->pfn_sys_send_data((mod_id), (dev_id), (chn_id), \
    (flag), (data_type), (pv_data)))

#define ckfn_sys_reset_data() \
    (func_entry(sys_export_func, HI_ID_SYS)->pfn_sys_reset_data != HI_NULL)
#define call_sys_reset_data(mod_id, dev_id, chn_id, private) \
    (func_entry(sys_export_func, HI_ID_SYS)->pfn_sys_reset_data(mod_id, dev_id, chn_id, private))

#define ckfn_sys_get_bind_by_src() \
    (func_entry(sys_export_func, HI_ID_SYS)->pfn_get_bind_by_src != HI_NULL)
#define call_sys_get_bind_by_src(src_chn, bind_dest) \
    (func_entry(sys_export_func, HI_ID_SYS)->pfn_get_bind_by_src((src_chn), (bind_dest)))

#define ckfn_sys_get_bind_num_by_src() \
    (func_entry(sys_export_func, HI_ID_SYS)->pfn_get_bind_num_by_src != HI_NULL)
#define call_sys_get_bind_num_by_src(src_chn, bind_num) \
    (func_entry(sys_export_func, HI_ID_SYS)->pfn_get_bind_num_by_src((src_chn), (bind_num)))

#define ckfn_sys_get_bind_by_dest() \
    (func_entry(sys_export_func, HI_ID_SYS)->pfn_get_bind_by_dest != HI_NULL)
#define call_sys_get_bind_by_dest(dest_chn, src_chn) \
    (func_entry(sys_export_func, HI_ID_SYS)->pfn_get_bind_by_dest((dest_chn), (src_chn)))
#define ckfn_sys_get_mmz_name() \
    (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_get_mmz_name != NULL))
#define call_sys_get_mmz_name(src_chn, pp_mmz_name) \
    FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_get_mmz_name((src_chn), (pp_mmz_name))

#define ckfn_sys_get_mem_ddr() \
    (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_get_mem_ddr != NULL))
#define call_sys_get_mem_ddr(src_chn, mem_ddr) \
    FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_get_mem_ddr((src_chn), (mem_ddr))

#define ckfn_sys_get_time_zone() \
    (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_time_zone != NULL))
#define call_sys_get_time_zone(time_zone) \
    FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_time_zone((time_zone))

#define ckfn_sys_get_gps_info() \
    (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_gps_info != NULL))
#define call_sys_get_gps_info(gps_info) \
    FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_gps_info()

#define ckfn_sys_get_now_string() \
    (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_now_string != NULL))
#define call_sys_get_now_string(now_time) \
    FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_now_string((now_time))

#define ckfn_sys_get_cmp_cfg() \
    (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_cmp_cfg != NULL))
#define call_sys_get_cmp_cfg(video_frame, y_cmp_cfg, cmp_cfg, cmp_mode_ex_param) \
    FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_cmp_cfg((video_frame), (y_cmp_cfg), (cmp_cfg), \
    (cmp_mode_ex_param))

#define ckfn_sys_get_vb_cfg() \
    (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_vb_cfg != NULL))
#define call_sys_get_vb_cfg(vb_base_info, vb_cal_config) \
    FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_vb_cfg((vb_base_info), (vb_cal_config))

#define ckfn_sys_get_vdec_buffer_cfg() \
    (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_vdec_buffer_cfg != NULL))
#define call_sys_get_vdec_buffer_cfg(type, vb_base_info, vb_cal_config) \
    FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_vdec_buffer_cfg((type), (vb_base_info), (vb_cal_config))

#define ckfn_sys_get_cmp_bayer_cfg() \
    (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_cmp_bayer_cfg != NULL))
#define call_sys_get_cmp_bayer_cfg(sys_cmp_in, bayer_cmp_cfg, bayer_dcmp_cfg) \
    FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_cmp_bayer_cfg((sys_cmp_in), (bayer_cmp_cfg), (bayer_dcmp_cfg))

#define ckfn_sys_get_dcmp_muv1_cfg() \
    (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_dcmp_muv1_cfg != NULL))
#define call_sys_get_dcmp_muv1_cfg(sys_cmp_in, dcmp_out) \
    FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_dcmp_muv1_cfg((sys_cmp_in), (dcmp_out))

#define ckfn_sys_get_cmp_tde_cfg() \
    (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_cmp_tde_cfg != NULL))
#define call_sys_get_cmp_tde_cfg(sys_cmp_in, cmp_out) \
    FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_cmp_tde_cfg((sys_cmp_in), (cmp_out))

#define ckfn_sys_get_cmp_seg_cfg() \
    (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_cmp_seg_cfg != NULL))
#define call_sys_get_cmp_seg_cfg(sys_cmp_in, y_cmp_cfg, cmp_cfg) \
    FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_cmp_seg_cfg((sys_cmp_in), (y_cmp_cfg), (cmp_cfg))

#define ckfn_sys_get_platform_type() \
    (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_platform_type != NULL))
#define call_sys_get_platform_type() \
    FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_platform_type()

#define ckfn_sys_get_chip_type() \
    (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_chip_type != NULL))
#define call_sys_get_chip_type() \
    FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_chip_type()

#define ckfn_sys_get_ip_num_per_device() \
    (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_ip_num_per_device != NULL))
#define call_sys_get_ip_num_per_device(dvpp_ip_type) \
    FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_ip_num_per_device((dvpp_ip_type))

#define ckfn_sys_get_ip_num() \
    (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_ip_num != NULL))
#define call_sys_get_ip_num(dvpp_ip_type) \
    FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_ip_num((dvpp_ip_type))

#define ckfn_sys_set_csc_param_callback() \
    (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_set_csc_param_callback != NULL))
#define call_sys_set_csc_param_callback() \
    FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_set_csc_param_callback()

#define ckfn_sys_get_csc_param_callback() \
    (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_csc_param_callback != NULL))
#define call_sys_get_csc_param_callback() \
    FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_csc_param_callback()

#define ckfn_sys_get_cpu_info() \
    (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_cpu_info != NULL))
#define call_sys_get_cpu_info(device_id, cpu_core_num, cpu_bitmap) \
    FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_cpu_info((device_id), (cpu_core_num), (cpu_bitmap))

#define ckfn_sys_get_cpu_mask() \
    (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_cpu_mask != NULL))
#define call_sys_get_cpu_mask(device_id, mask, get_cpu_type) \
    FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_cpu_mask((device_id), (mask), (get_cpu_type))

#define ckfn_sys_get_cpu_id_by_index() \
    (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_cpu_id_by_index != NULL))
#define call_sys_get_cpu_id_by_index(device_id, idx, get_cpu_type) \
    FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_cpu_id_by_index((device_id), (idx), (get_cpu_type))

#define ckfn_sys_bind_irq_to_cpu() \
    (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_bind_irq_to_cpu != NULL))
#define call_sys_bind_irq_to_cpu(core_info, cpu_id) \
    FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_bind_irq_to_cpu((core_info), (cpu_id))

#define ckfn_sys_bind_irq_to_cpu_mask() \
        (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_bind_irq_to_cpu_mask != NULL))
#define call_sys_bind_irq_to_cpu_mask(irq, mask) \
        FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_bind_irq_to_cpu_mask((irq), (mask))

#define ckfn_sys_unbind_irq_to_cpu() \
    (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_unbind_irq_to_cpu != NULL))
#define call_sys_unbind_irq_to_cpu(irq) \
    FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_unbind_irq_to_cpu((irq))

#define ckfn_sys_print_proc_title() \
    (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_print_proc_title != NULL)
#define call_sys_print_proc_title(s, format, args...) \
    FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_print_proc_title(s, format, ##args)

#define ckfn_sys_get_die_num_per_device() \
    (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_die_num_per_device != NULL)
#define call_sys_get_die_num_per_device() \
    FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_die_num_per_device()

#define ckfn_sys_get_aicpu_bitmap() \
    (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_aicpu_bitmap != NULL)
#define call_sys_get_aicpu_bitmap(ex_dev_id, cpu_bitmap) \
    FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_aicpu_bitmap((ex_dev_id), (cpu_bitmap))


__inline static hi_bool ckfn_sys_get_vf_or_vfg_aicpu_bitmap(hi_void)
{
    if (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_vf_or_vfg_aicpu_bitmap != NULL)) {
        return HI_TRUE;
    }
    return HI_FALSE;
}

__inline static hi_s32 call_sys_get_vf_or_vfg_aicpu_bitmap(hi_u32 device_id, hi_u32 vf_id,
    hi_u32* aicpu_bitmap, hi_u32 is_vfg)
{
    return FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_vf_or_vfg_aicpu_bitmap(device_id, vf_id,
        aicpu_bitmap, is_vfg);
}


__inline static hi_bool ckfn_sys_get_vf_or_vfg_mask(hi_void)
{
    if (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_vf_or_vfg_mask != NULL)) {
        return HI_TRUE;
    }
    return HI_FALSE;
}

__inline static hi_s32 call_sys_get_vf_or_vfg_mask(hi_u32 device_id, hi_u32 vf_id, hi_void *mask, hi_u32 is_vfg)
{
    return FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_vf_or_vfg_mask(device_id, vf_id, mask, is_vfg);
}

__inline static hi_bool ckfn_sys_set_vf_and_vfg_aicpu_bitmap(hi_void)
{
    if (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_set_vf_and_vfg_aicpu_bitmap != NULL)) {
        return HI_TRUE;
    }
    return HI_FALSE;
}

__inline static hi_s32 call_sys_set_vf_and_vfg_aicpu_bitmap(hi_u32 device_id, hi_u32 vf_id,
    hi_u32 vf_aicpu_bitmap, hi_u32 vfg_aicpu_bitmap)
{
    return FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_set_vf_and_vfg_aicpu_bitmap(device_id, vf_id,
        vf_aicpu_bitmap, vfg_aicpu_bitmap);
}

__inline static hi_bool ckfn_sys_get_irq_aicpu_id_by_index(hi_void)
{
    if (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_irq_aicpu_id_by_index != NULL)) {
        return HI_TRUE;
    }
    return HI_FALSE;
}

__inline static hi_u32 call_sys_get_irq_aicpu_id_by_index(hi_u32 device_id, hi_u32 vf_id, hi_u32 idx)
{
    return FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_irq_aicpu_id_by_index(device_id, vf_id, idx);
}

__inline static hi_bool ckfn_sys_set_aicpu_bind_index(hi_void)
{
    if (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_set_aicpu_bind_index != NULL)) {
        return HI_TRUE;
    }
    return HI_FALSE;
}

__inline static hi_s32 call_sys_set_aicpu_bind_index(hi_u32 device_id, hi_u32 vf_id)
{
    return FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_set_aicpu_bind_index(device_id, vf_id);
}

#define ckfn_sys_bootdot_init_block() \
        (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_bootdot_init_block != NULL))
#define call_sys_bootdot_init_block(execption_id, process_type, core_id, status) \
        FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_bootdot_init_block((execption_id), \
            (process_type), (core_id), (status))

#define ckfn_sys_bootdot_set_block_status() \
        (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_bootdot_set_block_status != NULL))
#define call_sys_bootdot_set_block_status(process_type, core_id, status) \
        FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_bootdot_set_block_status((process_type), (core_id), (status))

__inline static hi_bool ckfn_sys_mem_check_attribute(hi_void)
{
    if (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_mem_check_attribute != NULL)) {
        return HI_TRUE;
    }
    return HI_FALSE;
}

__inline static hi_s32 call_sys_mem_check_attribute(pid_t pid, hi_u64 va, hi_u64 size, hi_u32 attribute)
{
    return FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_mem_check_attribute(pid, va, size, attribute);
}
#define ckfn_sys_get_core_usable_info() \
        (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_core_usable_info != NULL))
#define call_sys_get_core_usable_info(core_type, usable_info) \
        FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_core_usable_info((core_type), (usable_info))

#define ckfn_sys_export_func_spin_lock() \
        (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_export_func_spin_lock != NULL))
#define call_sys_export_func_spin_lock() \
        FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_export_func_spin_lock()

#define ckfn_sys_export_func_spin_unlock() \
        (ckfn_sys_entry() && (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_export_func_spin_unlock != NULL))
#define call_sys_export_func_spin_unlock() \
        FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_export_func_spin_unlock()

#define call_sys_set_mpp_chn(mpp_chn, mod, dev, chn) \
    do {                                             \
        (mpp_chn)->mod_id = (mod);                   \
        (mpp_chn)->dev_id = (dev);                   \
        (mpp_chn)->chn_id = (chn);                   \
    } while (0)

#define ckfn_sys_get_cmp_3dnr_cfg() \
    (func_entry(sys_export_func, HI_ID_SYS)->pfn_sys_get_cmp_3dnr_cfg != HI_NULL)
#define call_sys_get_cmp_3dnr_cfg(sys_cmp_in, y_cmp_cfg, cmp_cfg) \
    (func_entry(sys_export_func, HI_ID_SYS)->pfn_sys_get_cmp_3dnr_cfg(sys_cmp_in, y_cmp_cfg, cmp_cfg))

#define ckfn_sys_get_scale_coef() \
    (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_scale_coef != HI_NULL)
#define call_sys_get_scale_coef(mod_id, scale_coef_opt, pv_rate, pv_cfg) \
    (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_scale_coef((mod_id), (scale_coef_opt), (pv_rate), (pv_cfg)))
#define call_sys_get_scale_coef_kva(scale_coef_addr, coef_len) \
    (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_get_scale_coef_kva((scale_coef_addr), (coef_len)))


hi_void sys_set_chip_type(sys_chip_type chip_type);
hi_void sys_set_paltform_type(sys_platform_type plat_type);
hi_void sys_set_die_num(hi_u32 die_num);

#define ckfn_sys_set_device_num() \
    (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_set_device_num != HI_NULL)
#define call_sys_set_device_num(device_num) \
    (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_set_device_num(device_num))

#define ckfn_sys_regist_dvpp_proc_show() \
    (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_regist_dvpp_proc_show != HI_NULL)
#define call_sys_regist_dvpp_proc_show(func) \
    (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_regist_dvpp_proc_show(func))

#define ckfn_sys_unregist_dvpp_proc_show() \
    (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_unregist_dvpp_proc_show != HI_NULL)
#define call_sys_unregist_dvpp_proc_show() \
    (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_unregist_dvpp_proc_show())

#define ckfn_sys_regist_dvpp_dump_info() \
    (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_regist_dvpp_dump_info != HI_NULL)
#define call_sys_regist_dvpp_dump_info(func) \
    (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_regist_dvpp_dump_info(func))

#define ckfn_sys_unregist_dvpp_dump_info() \
    (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_unregist_dvpp_dump_info != HI_NULL)
#define call_sys_unregist_dvpp_dump_info() \
    (FUNC_ENTRY(sys_export_func, HI_ID_SYS)->pfn_sys_unregist_dvpp_dump_info())

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* __SYS_EXT_H__ */

