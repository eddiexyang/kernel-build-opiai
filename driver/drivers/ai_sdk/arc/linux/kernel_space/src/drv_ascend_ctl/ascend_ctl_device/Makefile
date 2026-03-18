ifeq ($(TOP_DIR),)
KERNEL_DIR := $(srctree)

EXTRA_CFLAGS += -fstack-protector-all

ccflags-y += -Wall -Werror

EXTRA_CFLAGS += -I$(PWD)/drivers/dev_inc

obj-m += drv_ascend_ctl.o
drv_ascend_ctl-objs :=ascend_ctl_dev.o ascend_ctl_sysfs.o

else

EXTRA_CFLAGS += -fstack-protector-all

ccflags-y += -Wall -Werror

EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc

obj-m += drv_ascend_ctl.o
drv_ascend_ctl-objs :=ascend_ctl_dev.o ascend_ctl_sysfs.o
endif

ifneq ($(filter $(PRODUCT), ascend610 bs9sx1a ascend610Lite ascend610Liteesl),)
	EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
endif

ifeq ($(filter $(PRODUCT), ascend310B ascend310Brc as31xm1),)
	EXTRA_CFLAGS += -DCFG_FEATURE_SMP_ENABLE
endif
