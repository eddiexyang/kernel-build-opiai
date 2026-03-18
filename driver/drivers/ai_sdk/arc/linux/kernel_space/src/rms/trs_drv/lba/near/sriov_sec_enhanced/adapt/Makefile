MAKE_PRODUCT := $(PRODUCT)
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
    EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dev_inc_open/inc
    EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dev_inc/inc
    EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dev_inc/inc/dbl
    EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/inc/driver
else
    DRIVER_SRC_BASE_DIR := $(DRIVER_KERNEL_DIR)/src

    EXTRA_CFLAGS += -I$(DRIVER_OPEN_KERNEL_DIR)/inc/
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc/dbl
    EXTRA_CFLAGS += -I$(TOP_DIR)/libc_sec/include
    EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver
endif

ccflags-y += -Wall -Werror

EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/inc
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/inc
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/near/sriov_sec_enhanced/inc
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/near/sriov_sec_enhanced/adapt
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/near/sriov_sec_enhanced/adapt/comm
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/near/sriov_sec_enhanced/adapt/comm/stars_v1
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/near/sriov_sec_enhanced/adapt/comm/tscpu

EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/near/sriov_sec_enhanced/adapt/near_comm
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/near/sriov_sec_enhanced/adapt/near_comm/soc_adapt
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/near/sriov_sec_enhanced/adapt/near_comm/soc_adapt/cloud_v2
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/near/sriov_sec_enhanced/adapt/near_comm/soc_adapt/mini_v3
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/near/sriov_sec_enhanced/adapt/near_comm/soc_adapt/mini_v2
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/near/sriov_sec_enhanced/adapt/near_comm/trs_host_chan
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/near/sriov_sec_enhanced/adapt/near_comm/trs_host_chan/stars_v1
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/near/sriov_sec_enhanced/adapt/near_comm/trs_host_core
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/near/sriov_sec_enhanced/adapt/near_comm/trs_host_core/stars_v1
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/near/sriov_sec_enhanced/adapt/near_comm/trs_host_core/tscpu
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/near/sriov_sec_enhanced/adapt/trs_sec_eh_chan
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/near/sriov_sec_enhanced/adapt/trs_sec_eh_core
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/dms/include/
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/dms/config/
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/dbl/dev_urd/
obj-m += ascend_trs_sec_eh_adapt.o
ascend_trs_sec_eh_adapt-objs := trs_sec_eh.o trs_sec_eh_init.o trs_sec_eh_vpc.o
ascend_trs_sec_eh_adapt-objs += trs_sec_eh_chan/trs_sec_eh_chan.o trs_sec_eh_core/trs_sec_eh_core.o

ascend_trs_sec_eh_adapt-objs += near_comm/soc_adapt/soc_adapt.o near_comm/soc_adapt/cloud_v2/soc_adapt_res_cloud_v2.o near_comm/soc_adapt/mini_v3/soc_adapt_res_mini_v3.o
ascend_trs_sec_eh_adapt-objs += near_comm/soc_adapt/mini_v2/soc_adapt_res_mini_v2.o near_comm/trs_host_core/trs_core_near_ops_mbox.o
ascend_trs_sec_eh_adapt-objs += near_comm/trs_host_chan/stars_v1/trs_chan_stars_v1_ops.o near_comm/trs_host_chan/trs_chan_near_ops_mbox.o
ascend_trs_sec_eh_adapt-objs += near_comm/trs_host_chan/trs_chan_near_ops_rsv_mem.o near_comm/trs_host_chan/stars_v1/trs_chan_stars_v1_ops_stars.o
ascend_trs_sec_eh_adapt-objs += near_comm/trs_host_chan/trs_chan_near_ops_mem.o near_comm/trs_host_core/tscpu/trs_tscpu_core_near_ops.o
ascend_trs_sec_eh_adapt-objs += near_comm/trs_host_core/stars_v1/trs_core_stars_v1_ops.o near_comm/trs_host_db.o
ascend_trs_sec_eh_adapt-objs += near_comm/trs_host_id.o near_comm/trs_host_msg.o near_comm/trs_host_comm.o near_comm/trs_near_adapt_init.o near_comm/trs_host_core/trs_core_near_ops.o

ascend_trs_sec_eh_adapt-objs += comm/trs_chan_irq.o comm/trs_chan_mbox.o comm/trs_chan_mem.o comm/trs_chan_update.o comm/trs_rsv_mem.o comm/trs_ts_db.o comm/trs_ts_status.o comm/trs_chan_maint_sqcq.o
ascend_trs_sec_eh_adapt-objs += comm/stars_v1/trs_chan_sqcq.o comm/stars_v1/trs_stars.o comm/stars_v1/trs_stars_cq.o comm/stars_v1/trs_stars_sq.o

ascend_trs_sec_eh_adapt-objs += comm/tscpu/trs_tscpu_sq.o comm/tscpu/trs_tscpu_cq.o comm/tscpu/trs_tscpu_chan_sqcq.o
