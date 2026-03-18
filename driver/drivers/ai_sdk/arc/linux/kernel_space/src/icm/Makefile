ifeq ($(TARGET_BUILD_TYPE),debug)
	EXTRA_CFLAGS += -DCFG_BUILD_DEBUG
endif

EXTRA_CFLAGS += -fstack-protector-all

EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
EXTRA_CFLAGS += -I$(DRIVER_OPEN_KERNEL_DIR)/inc

ifneq ($(filter $(PRODUCT),  ascend910),)
	EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD
else ifeq ($(PRODUCT),  ascend910B)
	EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD_V2
else ifneq ($(filter $(PRODUCT),  ascend310B ascend310Besl ascend310Brc ascend310Brcesl as31xm1),)
	EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_ESL_FPGA
	EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV3
else ifneq ($(filter $(PRODUCT),  ascend610 bs9sx1a ascend610Lite ascend610Liteesl),)
	EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
	EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2_MDC
	EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
else ifneq ($(filter $(PRODUCT),  ascend310p helper310p),)
	EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
else
	EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINI
endif

ifneq ($(filter $(PRODUCT),  ascend610Lite),)
	EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51_LITE
endif

ifneq ($(filter $(PRODUCT), as31xm1),)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V11
endif
ifneq ($(filter $(PRODUCT), ascend310p),)
	EXTRA_CFLAGS += -DCFG_EDP_MSG
endif
ifneq ($(filter $(PRODUCT), helper310p),)
	EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_HELPER
endif

ccflags-y += -Wall -Werror -Wtrampolines $(WDATE_TIME) -Wfloat-equal -Wvla -Wundef -funsigned-char -Wformat=2 -Wstack-usage=2048 -Wcast-align
ccflags-y += -Wextra -Wno-unused-parameter -Wno-sign-compare -Wno-missing-field-initializers

obj-m += drv_icm.o
drv_icm-objs :=icm_core.o icm_interface.o icm_ipc.o

