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
    EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/inc/driver
else
    DRIVER_SRC_BASE_DIR := $(DRIVER_KERNEL_DIR)/src

    EXTRA_CFLAGS += -I$(DRIVER_OPEN_KERNEL_DIR)/inc/
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
    EXTRA_CFLAGS += -I$(TOP_DIR)/libc_sec/include
    EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver
endif

ccflags-y += -Wall -Werror

EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/inc
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/near/sia/adapt
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/near/sia/adapt/trs_host_core
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/near/sriov_sec_enhanced/adapt/near_comm/trs_host_chan/stars_v1
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/near/sriov_sec_enhanced/adapt/near_comm/trs_host_chan
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/comm/adapt
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/near/comm/adapt
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/near/sriov_sec_enhanced/inc

obj-m += ascend_trs_sec_eh_agent.o
ascend_trs_sec_eh_agent-objs := trs_sec_eh_agent_init.o
ascend_trs_sec_eh_agent-objs += trs_sec_eh_cfg.o trs_sec_eh_id.o trs_sec_eh_sq.o trs_sec_eh_agent_msg.o trs_sec_eh_mbox.o

