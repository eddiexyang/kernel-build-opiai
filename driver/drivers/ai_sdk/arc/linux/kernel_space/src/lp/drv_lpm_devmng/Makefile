# for cmake
KERNEL_DIR :=$(srctree)

EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc/dbl
EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/include
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/core
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dbl/dev_urd
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/smf/sensor
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/event
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/smf/event
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/config

ifeq ($(use_hitest),kernel_true)
    CC:=hitestwrapper $(CC)
    LD:=hitestwrapper $(LD)
endif

ccflags-y += -I$(M)/ \
             -I$(M)/public/ \
             -I$(M)/dfx/ \
             -I$(M)/dfx/debugfs/ \
             -I$(M)/dfx/statistics/ \
             -I$(M)/init/ \
             -I$(M)/srv/ \
             -I$(M)/api/ \
             -I$(M)/drv/ \
             -I$(M)/drv/include/ \
             -I$(M)/drv/idle_drv/ \
             -I$(M)/drv/frequency_drv/ \
             -I$(M)/drv/temperature_drv/ \
             -I$(M)/drv/aic_freq_drv/ \
             -I$(M)/drv/sharemem_drv/

# -Wno-sign-compare needs to be kept, otherwise an error will be reported when compiling the kernel
EXTRA_CFLAGS += -fstack-protector-all
ccflags-y += -Wall -Werror -fno-common -Wfloat-equal -Wformat=2 $(WDATE_TIME) -Wcast-align  -Wundef -Wstrict-prototypes
ccflags-y += -Wunused -O2 -Wextra -Wno-unused-parameter -Wno-sign-compare

obj-m += drv_lpm_devmng.o
drv_lpm_devmng-objs := init/lpm_devmng_init.o public/lpm_devmng_common.o
drv_lpm_devmng-objs += api/lpm_devmng_dsmi.o
drv_lpm_devmng-objs += dfx/statistics/lpm_devmng_statistics.o
drv_lpm_devmng-objs += dfx/debugfs/lpm_devmng_debugfs.o dfx/debugfs/lpm_debugfs_base.o

ifeq ($(TARGET_BUILD_TYPE),debug)
	EXTRA_CFLAGS += -DLPM_BUILD_DEBUG
	EXTRA_CFLAGS += -DLPM_POWER_SAMPLING
endif

ifneq ($(filter $(PRODUCT), ascend910B),)
    drv_lpm_devmng-objs += public/lpm_devmng_sharemem.o public/lpm_devmng_ipc.o
    drv_lpm_devmng-objs += drv/sharemem_drv/lpm_devmng_sharemem_drv.o drv/sharemem_drv/lpm_devmng_sharemem_v2.o
    drv_lpm_devmng-objs += srv/lpm_devmng_temperature.o drv/temperature_drv/lpm_devmng_temperature_sharemem_v2.o
    drv_lpm_devmng-objs += srv/lpm_devmng_power.o srv/lpm_devmng_tool.o srv/lpm_devmng_turbo.o
    drv_lpm_devmng-objs += srv/lpm_devmng_idle.o drv/idle_drv/lpm_devmng_idle_ascend910b.o
    drv_lpm_devmng-objs += srv/lpm_devmng_frequency.o drv/frequency_drv/lpm_devmng_frequency_sharemem_v2.o
    drv_lpm_devmng-objs += srv/lpm_devmng_soc_stress.o
    drv_lpm_devmng-objs += drv/aic_freq_drv/lpm_devmng_aic_freq_sharemem_v2.o
    drv_lpm_devmng-objs += api/lpm_devmng_api.o
    drv_lpm_devmng-objs += srv/lpm_devmng_aic_freq.o
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD_V2
    EXTRA_CFLAGS += -DLPM_SHAREMEM_V2
    EXTRA_CFLAGS += -DLPM_IPC_V3
    EXTRA_CFLAGS += -DLPM_TEMPERATURE
    EXTRA_CFLAGS += -DLPM_LPTEST
    EXTRA_CFLAGS += -DLPM_SOC_STRESS
    EXTRA_CFLAGS += -DLPM_POWER
    EXTRA_CFLAGS += -DLPM_TURBO
    EXTRA_CFLAGS += -DLPM_IDLE
    EXTRA_CFLAGS += -DLPM_FREQUENCY
    EXTRA_CFLAGS += -DLPM_AIC_FREQ_SYSCNT
    EXTRA_CFLAGS += -DLPM_AIC_FREQ
    EXTRA_CFLAGS += -DLPM_DSMI_MAIN_CMD_LP
endif

ifneq ($(filter $(PRODUCT), ascend920esl ascend920emu),)
    drv_lpm_devmng-objs += public/lpm_devmng_sharemem.o public/lpm_devmng_ipc.o
    drv_lpm_devmng-objs += drv/sharemem_drv/lpm_devmng_sharemem_drv.o drv/sharemem_drv/lpm_devmng_sharemem_v2.o
    drv_lpm_devmng-objs += srv/lpm_devmng_temperature.o drv/temperature_drv/lpm_devmng_temperature_sharemem_v2.o
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD_V2
    EXTRA_CFLAGS += -DLPM_SHAREMEM_V2
    EXTRA_CFLAGS += -DLPM_IPC_V3
    EXTRA_CFLAGS += -DLPM_TEMPERATURE
endif

ifneq ($(filter $(PRODUCT), ascend610 BS9SX1A),)
    drv_lpm_devmng-objs += public/lpm_devmng_ipc.o
    drv_lpm_devmng-objs += srv/lpm_devmng_frequency.o drv/frequency_drv/lpm_devmng_frequency_ascend610.o
    drv_lpm_devmng-objs += srv/lpm_devmng_temperature.o drv/temperature_drv/lpm_devmng_temperature_ascend610.o
    drv_lpm_devmng-objs += public/lpm_log_base.o
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
    EXTRA_CFLAGS += -DLPM_IPC_V2
    EXTRA_CFLAGS += -DLPM_FREQUENCY
    EXTRA_CFLAGS += -DLPM_TEMPERATURE
    EXTRA_CFLAGS += -DLPM_BUILD_DEBUG
    EXTRA_CFLAGS += -DLPM_VPRINTK
endif

ifneq ($(filter $(PRODUCT), ascend610Lite),)
    drv_lpm_devmng-objs += public/lpm_devmng_ipc.o
    drv_lpm_devmng-objs += srv/lpm_devmng_tool.o
    drv_lpm_devmng-objs += drv/sharemem_drv/lpm_devmng_sharemem_drv.o
    drv_lpm_devmng-objs += drv/sharemem_drv/lpm_devmng_sharemem_v2.o
    drv_lpm_devmng-objs += public/lpm_devmng_sharemem.o
    drv_lpm_devmng-objs += public/lpm_log_base.o
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51_LITE
    EXTRA_CFLAGS += -DLPM_IPC_V3
    EXTRA_CFLAGS += -DLPM_BUILD_DEBUG
    EXTRA_CFLAGS += -DLPM_LPTEST
    EXTRA_CFLAGS += -DLPM_START_DIRECT
    EXTRA_CFLAGS += -DLPM_SHAREMEM_V2
    EXTRA_CFLAGS += -DLPM_VPRINTK
endif

ifneq ($(filter $(PRODUCT), ascend310p),)
    drv_lpm_devmng-objs += public/lpm_devmng_sharemem.o
    drv_lpm_devmng-objs += drv/sharemem_drv/lpm_devmng_sharemem_drv.o drv/sharemem_drv/lpm_devmng_sharemem_v1.o
    drv_lpm_devmng-objs += srv/lpm_devmng_aic_freq.o
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
    EXTRA_CFLAGS += -DLPM_SHAREMEM_V1
    EXTRA_CFLAGS += -DLPM_AIC_FREQ
    EXTRA_CFLAGS += -DLPM_DSMI_MAIN_CMD_LP
endif

ifneq ($(filter $(PRODUCT), helper310p),)
    drv_lpm_devmng-objs += public/lpm_devmng_sharemem.o
    drv_lpm_devmng-objs += drv/sharemem_drv/lpm_devmng_sharemem_drv.o drv/sharemem_drv/lpm_devmng_sharemem_v1.o
    drv_lpm_devmng-objs += srv/lpm_devmng_aic_freq.o
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_HELPER
    EXTRA_CFLAGS += -DLPM_START_DIRECT
    EXTRA_CFLAGS += -DLPM_SHAREMEM_V1
    EXTRA_CFLAGS += -DLPM_AIC_FREQ
    EXTRA_CFLAGS += -DLPM_DSMI_MAIN_CMD_LP
endif

ifneq ($(filter $(PRODUCT), ascend310B ascend310Brc),)
    drv_lpm_devmng-objs += public/lpm_devmng_sharemem.o public/lpm_devmng_ipc.o public/lpm_devmng_flash.o
    drv_lpm_devmng-objs += drv/sharemem_drv/lpm_devmng_sharemem_drv.o drv/sharemem_drv/lpm_devmng_sharemem_v2.o
    drv_lpm_devmng-objs += srv/lpm_devmng_profile.o srv/lpm_devmng_tool.o
    drv_lpm_devmng-objs += srv/lpm_devmng_idle.o drv/idle_drv/lpm_devmng_idle_ascend310b.o
    drv_lpm_devmng-objs += srv/lpm_devmng_frequency.o drv/frequency_drv/lpm_devmng_frequency_sharemem_v2.o
    drv_lpm_devmng-objs += srv/lpm_devmng_temperature.o drv/temperature_drv/lpm_devmng_temperature_sharemem_v2.o
    drv_lpm_devmng-objs += srv/lpm_devmng_aic_freq.o drv/aic_freq_drv/lpm_devmng_aic_freq_sharemem_v2.o
    drv_lpm_devmng-objs += srv/lpm_devmng_acg_cnt.o
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV3
    EXTRA_CFLAGS += -DLPM_IPC_V3
    EXTRA_CFLAGS += -DLPM_IDLE
    EXTRA_CFLAGS += -DLPM_PROFILE
    EXTRA_CFLAGS += -DLPM_FLASH
    EXTRA_CFLAGS += -DLPM_LPTEST
    EXTRA_CFLAGS += -DLPM_SHAREMEM_V2
    EXTRA_CFLAGS += -DLPM_FREQUENCY
    EXTRA_CFLAGS += -DLPM_TEMPERATURE
    EXTRA_CFLAGS += -DLPM_AIC_FREQ_SYSCNT
    EXTRA_CFLAGS += -DLPM_AIC_FREQ
    EXTRA_CFLAGS += -DLPM_ACG_CNT
    EXTRA_CFLAGS += -DLPM_DSMI_MAIN_CMD_LP
endif

ifneq ($(filter $(PRODUCT), ascend310Brc),)
    drv_lpm_devmng-objs += srv/lpm_devmng_suspend.o srv/lpm_devmng_vrd_upgrade.o
    EXTRA_CFLAGS += -DLPM_SUSPEND
    EXTRA_CFLAGS += -DLPM_VRD_UPGRADE
    EXTRA_CFLAGS += -DLPM_TRANSFORM_DEVID
endif

ifneq ($(filter $(PRODUCT), as31xm1),)
    drv_lpm_devmng-objs += public/lpm_devmng_sharemem.o public/lpm_devmng_ipc.o
    drv_lpm_devmng-objs += drv/sharemem_drv/lpm_devmng_sharemem_drv.o drv/sharemem_drv/lpm_devmng_sharemem_v2.o
    drv_lpm_devmng-objs += srv/lpm_devmng_tool.o srv/lpm_devmng_suspend.o
    drv_lpm_devmng-objs += srv/lpm_devmng_frequency.o drv/frequency_drv/lpm_devmng_frequency_sharemem_v2.o
    drv_lpm_devmng-objs += srv/lpm_devmng_temperature.o drv/temperature_drv/lpm_devmng_temperature_sharemem_v2.o
    drv_lpm_devmng-objs += srv/lpm_devmng_health_status.o srv/lpm_devmng_volt_current.o srv/lpm_devmng_acg_cnt.o
    drv_lpm_devmng-objs += srv/lpm_devmng_aic_freq.o
    drv_lpm_devmng-objs += srv/lpm_devmng_temp_threshold.o
    drv_lpm_devmng-objs += public/lpm_log_base.o
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV3
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V11
    EXTRA_CFLAGS += -DCFG_FEATURE_DMS_SUPPORT_ALL
    EXTRA_CFLAGS += -DLPM_IPC_V3
    EXTRA_CFLAGS += -DLPM_LPTEST
    EXTRA_CFLAGS += -DLPM_SHAREMEM_V2
    EXTRA_CFLAGS += -DLPM_FREQUENCY
    EXTRA_CFLAGS += -DLPM_TEMPERATURE
    EXTRA_CFLAGS += -DLPM_HEALTH_STATUS
    EXTRA_CFLAGS += -DLPM_VOLT_CURRENT
    EXTRA_CFLAGS += -DLPM_ACG_CNT
    EXTRA_CFLAGS += -DLPM_AIC_FREQ
    EXTRA_CFLAGS += -DLPM_SUSPEND
    EXTRA_CFLAGS += -DLPM_DSMI_MAIN_CMD_LP
    EXTRA_CFLAGS += -DLPM_TEMP_THRESHOLD
    EXTRA_CFLAGS += -DLPM_VPRINTK
endif
