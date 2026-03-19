MAKE_PRODUCT := $(PRODUCT)
ifeq ($(TARGET_BUILD_TYPE),debug)
    EXTRA_CFLAGS += -DCFG_BUILD_DEBUG
endif

DRIVER_SRC_BASE_DIR := $(DRIVER_KERNEL_DIR)/src

EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc/dbl
EXTRA_CFLAGS += -I$(TOP_DIR)/libc_sec/include
EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver

EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/inc
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/trs_core
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/sia/adapt/
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/sia/adapt/soc
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/sia/adapt/chan
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/sia/adapt/core
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/sia/adapt/comm
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/sia/adapt/comm/tscpu
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/sia/adapt/local_comm
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/comm/adapt
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/dms/include/
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/dms/config/
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/dbl/dev_urd/
EXTRA_CFLAGS += -DCFG_FEATURE_IRQ_BIND

ccflags-y += -Wall -Werror

obj-m += ascend_trs_pm_adapt.o

ascend_trs_pm_adapt-objs := trs_device.o trs_device_comm.o trs_device_id.o trs_device_mbox.o local_comm/trs_device_ts_db.o local_comm/trs_device_notice.o trs_device_tsfw_shr_mem.o trs_device_ts_status.o

ascend_trs_pm_adapt-objs += chan/trs_device_chan.o chan/trs_chan_ops_mem.o chan/trs_chan_ops_irq.o chan/trs_chan_ops_mbox.o chan/trs_chan_ops_rsv_mem.o
ascend_trs_pm_adapt-objs += core/trs_device_core.o core/trs_core_ops.o
ascend_trs_pm_adapt-objs += comm/trs_chan_mem.o comm/trs_chan_mbox.o comm/trs_id_conv.o comm/trs_mbox.o comm/trs_rsv_mem.o comm/trs_ts_db.o comm/trs_chan_update.o comm/trs_chan_irq.o comm/trs_ts_status.o comm/trs_chan_maint_sqcq.o

ascend_trs_pm_adapt-objs += comm/tscpu/trs_tscpu_sq.o comm/tscpu/trs_tscpu_cq.o comm/tscpu/trs_tscpu_chan_sqcq.o

ifneq ($(filter $(PRODUCT), ascend310B as31xm1 ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu),)
    EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/sia/adapt/soc/mini_v3
    EXTRA_CFLAGS += -DCFG_FEATURE_TS_ABNORMAL_INFO
else ifneq ($(filter $(PRODUCT), ascend910B),)
    EXTRA_CFLAGS += -DCFG_FEATURE_TS_ABNORMAL_INFO
    EXTRA_CFLAGS += -DCFG_FEATURE_TS_MEM_NODE
    EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/sia/adapt/soc/cloud_v2
else ifneq ($(filter $(PRODUCT), helper310p),)
    EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/sia/adapt/soc/mini_v2
endif

ifneq ($(filter $(PRODUCT), ascend310B ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu),)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV3
endif

ifneq ($(filter $(PRODUCT), as31xm1), )
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V11
    EXTRA_CFLAGS += -DCFG_MEMORY_OPTIMIZE
    EXTRA_CFLAGS += -DCFG_SUPPORT_AOS_KERNEL
endif

ifneq ($(filter $(PRODUCT), helper310p),)
    EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/sia/adapt/chan/tscpu
    EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/sia/adapt/core/tscpu

    EXTRA_CFLAGS += -DCFG_FEATURE_TSCPU

    ascend_trs_pm_adapt-objs += chan/tscpu/trs_tscpu_chan_ops.o chan/tscpu/trs_tscpu_chan_ops_db.o
    ascend_trs_pm_adapt-objs += core/tscpu/trs_tscpu_core_ops.o
else
    EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/sia/adapt/chan/stars_v1
    EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/sia/adapt/core/stars_v1
    EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/sia/adapt/comm/stars_v1

    ascend_trs_pm_adapt-objs += chan/stars_v1/trs_chan_ops.o chan/stars_v1/trs_chan_ops_db.o chan/stars_v1/trs_chan_ops_stars.o
    ascend_trs_pm_adapt-objs += core/stars_v1/trs_stars_core_ops.o
    ascend_trs_pm_adapt-objs += comm/stars_v1/trs_stars.o comm/stars_v1/trs_stars_cq.o comm/stars_v1/trs_stars_sq.o comm/stars_v1/trs_chan_sqcq.o
endif
