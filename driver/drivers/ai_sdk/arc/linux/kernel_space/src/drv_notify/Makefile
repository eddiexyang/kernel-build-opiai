ifeq ($(TOP_DIR),)
KERNEL_DIR := $(srctree)

EXTRA_CFLAGS += -fstack-protector-all

ccflags-y += -Wall -Werror

EXTRA_CFLAGS += -I$(PWD)/drivers/dev_inc

obj-m += drv_centre_notify.o
drv_centre_notify-objs :=drv_notify.o

else

EXTRA_CFLAGS += -fstack-protector-all

ccflags-y += -Wall -Werror

EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc/ascend_platform

obj-m += drv_centre_notify.o
drv_centre_notify-objs :=drv_notify.o
endif

ifneq ($(filter $(PRODUCT), ascend610 bs9sx1a ascend610Lite ascend610Liteesl),)
EXTRA_CFLAGS += -DCFG_FEATURE_BOARD_ID_EXPAND
endif

ifneq ($(filter $(PRODUCT), ascend610Lite ascend610Liteesl),)
EXTRA_CFLAGS += -DCFG_SOC_MDC_V51_LITE
endif

ifeq ($(PRODUCT), as31xm1)
EXTRA_CFLAGS += -DCFG_MASK_CHIP_TYPE8
EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V11
endif

include $(FEATURE_MK_PATH)
EXTRA_CFLAGS += $(CONFIG_DEFINES)
