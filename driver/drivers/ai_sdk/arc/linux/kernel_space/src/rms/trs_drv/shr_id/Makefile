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

    #for trs_shr_id_event_update.o
    EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/drv_devmng/drv_devmng_inc
    EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/ts_drv_common
    EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/ts_platform_host/ascend910
    #for trs_shr_id_event_update.o end
else
    DRIVER_SRC_BASE_DIR := $(DRIVER_KERNEL_DIR)/src

    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
    EXTRA_CFLAGS += -I$(TOP_DIR)/libc_sec/include
    EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver
endif

EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/inc
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/shr_id

EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_platform/ts_platform_host/ascend910
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_common

ccflags-y += -Wall -Werror

obj-m += ascend_trs_shrid.o
ascend_trs_shrid-objs := trs_shr_id.o trs_shr_id_fops.o trs_shr_id_node.o trs_shr_id_event_update.o

