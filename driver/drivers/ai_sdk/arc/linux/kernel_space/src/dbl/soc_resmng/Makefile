ifeq ($(TARGET_BUILD_TYPE),debug)
	EXTRA_CFLAGS += -DCFG_BUILD_DEBUG
endif

ifneq ($(NOT_SUPPORT_SP), y)
	EXTRA_CFLAGS += -fstack-protector-all
endif

ifeq ($(TOP_DIR),)
	#for dkms
	DRIVER_SRC_BASE_DIR := $(HIAI_DKMS_DIR)

	EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/libc_sec/include
	EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dev_inc/inc
	EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/inc/driver
else
	DRIVER_SRC_BASE_DIR := $(DRIVER_KERNEL_DIR)/src

	EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
	EXTRA_CFLAGS += -I$(TOP_DIR)/libc_sec/include
	EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver

endif

EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/inc

obj-m += ascend_soc_resmng.o
ascend_soc_resmng-objs := soc_resmng.o soc_subsys_ts.o



