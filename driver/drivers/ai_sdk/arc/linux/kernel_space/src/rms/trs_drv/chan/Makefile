ccflags-y += -Wall -Werror
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
    EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dev_inc/inc/dbl
else
    DRIVER_SRC_BASE_DIR := $(DRIVER_KERNEL_DIR)/src

    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc/dbl
    EXTRA_CFLAGS += -I$(TOP_DIR)/libc_sec/include
    EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver
endif

EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/inc
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/chan

obj-m += ascend_trs_nvme_chan.o
ascend_trs_nvme_chan-objs := chan_init.o chan_rxtx.o chan_ts_inst.o chan_proc_fs.o chan_abnormal.o
ifeq ($(TARGET_BUILD_TYPE),debug)
    ascend_trs_nvme_chan-objs += chan_trace.o
endif

ifneq ($(filter $(PRODUCT), as31xm1), )
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V11
endif
