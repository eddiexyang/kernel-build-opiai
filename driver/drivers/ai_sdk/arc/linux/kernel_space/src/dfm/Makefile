
$(warning TOP_DIR = $(TOP_DIR) ......)

ifeq ($(TOP_DIR),)
KERNEL_DIR := $(srctree)

EXTRA_CFLAGS += -I$(PWD)/drivers/dev_inc
EXTRA_CFLAGS += -I$(PWD)/inc/driver
EXTRA_CFLAGS += -I$(PWD)/kernel/vendor/bbox/driver/mntn/blackbox
EXTRA_CFLAGS += -I$(PWD)/kernel/vendor/bbox/include/
ccflags-y += -Wall -Werror -funsigned-char -Wextra -Wformat=2 -Wfloat-equal -Wcast-align -Wvla -Wundef $(WDATE_TIME)
ccflags-y += -Wstack-usage=2048 -Wno-unused-parameter -Wno-sign-compare

ifeq ($(TARGET_PRODUCT), cloud)
EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD
EXTRA_CFLAGS += -I$(PWD)/drivers/dev_plat/dfm/ascd910
endif
ifeq ($(TARGET_PRODUCT), mdc)
	ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_MDC_V10))
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_plat/dfm/ascd610
	else ifeq ($(TARGET_CHIP_ID), $(ASCEND_MDC_BS9SX1A))
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_plat/dfm/ascd610
	else
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_plat/dfm/ascd310
	endif
endif
ifeq ($(TARGET_PRODUCT), mini)
	ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_DC_V10))
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_plat/dfm/ascd610
	else
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_plat/dfm/ascd310
	endif
endif
obj-m   += drv_dfm.o
ifeq ($(TARGET_PRODUCT), cloud)
	drv_dfm-objs := ascd910/devdrv_dfm_cloud.o
endif
ifeq ($(TARGET_PRODUCT), mdc)
	ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_MDC_V10))
		drv_dfm-objs := ascd610/devdrv_dfm_miniv2.o
	else ifeq ($(TARGET_CHIP_ID), $(ASCEND_MDC_BS9SX1A))
		drv_dfm-objs := ascd610/devdrv_dfm_miniv2.o
	else
		drv_dfm-objs := ascd310/devdrv_dfm_mini.o
	endif
endif
ifeq ($(TARGET_PRODUCT), mini)
	ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_DC_V10))
		drv_dfm-objs := ascd610/devdrv_dfm_miniv2.o
	else
		drv_dfm-objs := ascd310/devdrv_dfm_mini.o
	endif
endif

else
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc/ascend_platform
EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver
EXTRA_CFLAGS += -I$(TOP_DIR)/kernel/vendor/bbox/driver/mntn/blackbox
EXTRA_CFLAGS += -I$(TOP_DIR)/kernel/vendor/bbox/include/
EXTRA_CFLAGS += -I$(TOP_DIR)/abl/bbox/inc/bbox
EXTRA_CFLAGS += -I$(TOP_DIR)/inc/toolchain/bbox

ccflags-y += -Wall -Werror -funsigned-char -Wextra -Wformat=2 -Wfloat-equal -Wcast-align -Wvla -Wundef $(WDATE_TIME)
ccflags-y += -Wstack-usage=2048 -Wno-unused-parameter -Wno-sign-compare

ifneq ($(filter $(PRODUCT),  ascend910 ascend910B ascend920esl ascend920emu),)
include $(FEATURE_MK_PATH)
EXTRA_CFLAGS += $(CONFIG_DEFINES)
EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dfm/ascd910

ifneq ($(filter $(PRODUCT),  ascend910),)
    EXTRA_CFLAGS += -DCFG_FEATURE_DFM_WITH_DVPP
endif

obj-m   += drv_dfm.o
drv_dfm-objs := ascd910/devdrv_dfm_cloud.o

else ifneq ($(filter $(PRODUCT),  ascend310B ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu ascend610 bs9sx1a ascend310p helper310p ascend610Lite ascend610Liteesl as31xm1),)
   EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dfm/ascd610
   EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
   obj-m   += drv_dfm.o
   drv_dfm-objs := ascd610/devdrv_dfm_miniv2.o
else
   EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINI
   EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dfm/ascd310
   obj-m   += drv_dfm.o
   drv_dfm-objs := ascd310/devdrv_dfm_mini.o
endif
endif

