ifeq ($(TARGET_BUILD_TYPE),debug)
    EXTRA_CFLAGS += -DCFG_BUILD_DEBUG
endif

ifneq ($(NOT_SUPPORT_SP), y)
    EXTRA_CFLAGS += -fstack-protector-all
endif

ccflags-y += -Wall -Werror

ifeq ($(TOP_DIR),)
    #for dkms
    DRIVER_SRC_BASE_DIR := $(HIAI_DKMS_DIR)

    EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/libc_sec/include
    EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dev_inc/inc
    EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dev_inc/inc/dbl
    EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/inc/driver
else
    DRIVER_SRC_BASE_DIR := $(DRIVER_KERNEL_DIR)/src

    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc/dbl
    EXTRA_CFLAGS += -I$(TOP_DIR)/libc_sec/include
    EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver
endif

EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/inc
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/trs_core

obj-m += ascend_trs_core.o
ascend_trs_core-objs := trs_fops.o trs_ts_inst.o trs_proc.o trs_res_mng.o trs_sqcq_map.o trs_hw_sqcq.o trs_sw_sqcq.o trs_logic_cq.o trs_cb_sqcq.o trs_shm_sqcq.o trs_proc_fs.o
ifeq ($(TARGET_BUILD_TYPE),debug)
    ascend_trs_core-objs += trs_core_trace.o
endif

ifneq ($(filter $(PRODUCT), as31xm1), )
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V11
endif
