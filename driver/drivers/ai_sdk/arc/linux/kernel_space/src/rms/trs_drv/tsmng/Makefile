ifeq ($(TARGET_BUILD_TYPE),debug)
    EXTRA_CFLAGS += -DCFG_BUILD_DEBUG
endif

ifneq ($(NOT_SUPPORT_SP), y)
    EXTRA_CFLAGS += -fstack-protector-all
endif

#CFG
ifneq ($(filter $(PRODUCT), ascend310 ascend310rc),)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINI
endif

ifneq ($(filter $(PRODUCT), ascend910 ascend910B ascend920esl ascend920emu),)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD
    EXTRA_CFLAGS += -DCFG_FEATURE_IMU_ENABLE
    ifneq ($(filter $(PRODUCT), ascend910B ascend920esl ascend920emu),)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD_V2
        EXTRA_CFLAGS += -DCFG_FEATURE_TS_ALLOC_MEM
        EXTRA_CFLAGS += -DCFG_FEATURE_TS_ABNORMAL_INFO
    endif
    ccflags-y += -Wall
endif

ifneq ($(filter $(PRODUCT), ascend310p helper310p),)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINI
    EXTRA_CFLAGS += -DCFG_FEATURE_LP_ENABLE
endif

ifneq ($(filter $(PRODUCT), helper310p),)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_HELPER
endif

ifneq ($(filter $(PRODUCT), ascend610 bs9sx1a ascend610Lite ascend610Liteesl),)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINI
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
    EXTRA_CFLAGS += -DCFG_FEATURE_IPC_CRC
    EXTRA_CFLAGS += -DCFG_FEATURE_LP_ENABLE
endif

ifneq ($(filter $(PRODUCT), as31xm1), )
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V11
endif

ifneq ($(filter $(PRODUCT), ascend310B as31xm1 ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu),)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV3
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINI
    EXTRA_CFLAGS += -DCFG_FEATURE_IMU_ENABLE
    EXTRA_CFLAGS += -DCFG_FEATURE_TS_ABNORMAL_INFO
endif

ifneq ($(filter $(PRODUCT), ascend310 ascend310rc),)
    BUILD_RPEFIX_DIR := ascend310
else ifneq ($(filter $(PRODUCT), ascend310B as31xm1 ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu ascend610 ascend610Lite ascend610Liteesl bs9sx1a ascend310p helper310p ascend910 ascend910B ascend920esl ascend920emu),)
    BUILD_RPEFIX_DIR := ascend910
endif

ccflags-y += -Wall -Werror

#INCLUDE
EXTRA_CFLAGS += -I$(TOP_DIR)/inc
EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver
EXTRA_CFLAGS += -I$(TOP_DIR)/libc_sec/include
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc/dbl
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/rms/trs_drv/inc
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/rms/trs_drv/tsmng
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/rms/trs_drv/tsmng/$(BUILD_RPEFIX_DIR)
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/include
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dbl/dev_urd
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_inc
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/smf/sensor
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/smf/event
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/config
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_common
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/core
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device/ascend910
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/common

ifneq ($(filter $(PRODUCT), ascend310 ascend310rc),)
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/ascend310
else ifneq ($(filter $(PRODUCT), ascend310B as31xm1 ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu ascend310p helper310p ascend610 bs9sx1a ascend610Lite ascend610Liteesl),)
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/ascend610
else ifneq ($(filter $(PRODUCT), ascend910 ascend910B ascend920esl ascend920emu),)
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/ascend910
endif

obj-m += ascend_trs_tsmng.o
ascend_trs_tsmng-objs := tsmng_common.o
ascend_trs_tsmng-objs += $(BUILD_RPEFIX_DIR)/tsmng_pm.o
ascend_trs_tsmng-objs += $(BUILD_RPEFIX_DIR)/tsmng_notifier.o
ifneq ($(filter $(PRODUCT), ascend310B as31xm1 ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu ascend910B ascend920esl ascend920emu),)
    ascend_trs_tsmng-objs += $(BUILD_RPEFIX_DIR)/tsmng_abnormal_info.o
    ifneq ($(filter $(PRODUCT), ascend910B ascend920esl ascend920emu),)
        ascend_trs_tsmng-objs += $(BUILD_RPEFIX_DIR)/tsmng_mem.o
    endif
endif

ifneq ($(filter $(PRODUCT), ascend310B as31xm1 ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu ascend610 bs9sx1a ascend610Lite ascend610Liteesl ascend310p helper310p),)
    ascend_trs_tsmng-objs += $(BUILD_RPEFIX_DIR)/tsmng_group.o
endif
