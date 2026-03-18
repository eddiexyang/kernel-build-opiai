ccflags-y += -Wall -Werror
MAKE_PRODUCT := $(PRODUCT)
ifeq ($(TARGET_BUILD_TYPE),debug)
    EXTRA_CFLAGS += -DCFG_BUILD_DEBUG
endif

DRIVER_SRC_BASE_DIR := $(DRIVER_KERNEL_DIR)/src

EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc/dbl
EXTRA_CFLAGS += -I$(TOP_DIR)/abl/libc_sec/include
EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/inc
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/mia/adapt/
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/sia/adapt/
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/sia/adapt/soc
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/sia/adapt/chan
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/sia/adapt/chan/stars_v1
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/sia/adapt/core
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/sia/adapt/core/stars_v1
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/sia/adapt/comm
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/sia/adapt/comm/stars_v1
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/sia/adapt/comm/tscpu
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/comm
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/mia/adapt/comm
ifneq ($(filter $(PRODUCT), ascend310B ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu),)
    EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/sia/adapt/soc/mini_v3
else ifneq ($(filter $(PRODUCT), ascend910B),)
    EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/sia/adapt/soc/cloud_v2
endif

obj-m += ascend_trs_mia_adapt.o
ascend_trs_mia_adapt-objs := trs_mia_device.o trs_mia_cfg.o trs_mia_device_id.o trs_mia_device_chan.o trs_mia_device_core.o trs_mia_info_ts.o trs_mia_device_db.o
ascend_trs_mia_adapt-objs += local_comm/trs_device_ts_db.o
