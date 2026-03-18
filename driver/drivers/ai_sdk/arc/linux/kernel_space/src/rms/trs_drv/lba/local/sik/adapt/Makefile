MAKE_PRODUCT := $(PRODUCT)
ifeq ($(TARGET_BUILD_TYPE),debug)
    EXTRA_CFLAGS += -DCFG_BUILD_DEBUG
endif

DRIVER_SRC_BASE_DIR := $(DRIVER_KERNEL_DIR)/src

EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc/dbl
EXTRA_CFLAGS += -I$(HUAWEI_SECURE_C_DIR)/include
EXTRA_CFLAGS += -I$(HAL_API_HEADER_DIR)

EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/inc
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/trs_core
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/sik/adapt/
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/sik/adapt/chan
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/sik/adapt/core
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/sik/adapt/comm
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/sik/adapt/local_comm
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/sik/adapt/soc/kunpeng
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/sik/adapt/chan/stars_v1
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/sik/adapt/core/stars_v1
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/sik/adapt/comm/stars_v1

EXTRA_CFLAGS += -DCFG_SUPPORT_AOS_KERNEL

ccflags-y += -Wall -Werror

obj-m += kunpeng_trs_pm_adapt.o

kunpeng_trs_pm_adapt-objs := trs_local.o trs_device_comm.o trs_device_id.o trs_device_mbox.o
kunpeng_trs_pm_adapt-objs += chan/trs_device_chan.o chan/trs_chan_ops_mem.o chan/trs_chan_ops_mbox.o chan/stars_v1/trs_chan_ops.o chan/stars_v1/trs_chan_ops_stars.o
kunpeng_trs_pm_adapt-objs += core/trs_device_core.o core/trs_core_ops.o core/stars_v1/trs_stars_core_ops.o core/trs_core_smmu.o
kunpeng_trs_pm_adapt-objs += comm/trs_chan_irq.o comm/trs_chan_mbox.o comm/trs_chan_mem.o comm/trs_chan_update.o comm/trs_rsv_mem.o comm/trs_ts_db.o
kunpeng_trs_pm_adapt-objs += comm/stars_v1/trs_chan_sqcq.o comm/stars_v1/trs_stars.o comm/stars_v1/trs_stars_cq.o
kunpeng_trs_pm_adapt-objs += local_comm/trs_device_notice.o

