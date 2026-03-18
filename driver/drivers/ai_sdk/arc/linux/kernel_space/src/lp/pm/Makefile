#
# Makefile for the HISILICON pm  drivers.
#
KERNEL_DIR :=$(srctree)

EXTRA_CFLAGS += -fstack-protector-all
ccflags-y += -Wall -Werror -fno-common -Wfloat-equal -Wformat=2 $(WDATE_TIME) -Wcast-align  -Wundef -Wstrict-prototypes
ccflags-y += -Wunused -O2 -Wextra -Wno-unused-parameter -Wno-sign-compare

ifeq ($(use_hitest),kernel_true)
    CC:=hitestwrapper $(CC)
    LD:=hitestwrapper $(LD)
endif

MODULE_NAME := drv_pm
obj-m := $(MODULE_NAME).o

ifneq ($(filter $(PRODUCT), ascend310rc ascend310),)
	$(MODULE_NAME)-objs := ascend310/pm.o ascend310/wakeup_reason.o
else
	$(MODULE_NAME)-objs := pm.o
endif

ifneq ($(filter $(PRODUCT), ascend310Brc as31xm1),)
	EXTRA_CFLAGS += -DHAL_KERNEL_SUSPEND_RESUME
endif

ifneq ($(filter $(PRODUCT), as31xm1 ascend610 BS9SX1A ascend610Lite),)
	EXTRA_CFLAGS += -DLPM_VPRINTK
endif