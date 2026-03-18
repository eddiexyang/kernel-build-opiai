MAKE_PRODUCT := $(PRODUCT)
ifeq ($(TARGET_BUILD_TYPE),debug)
    EXTRA_CFLAGS += -DCFG_BUILD_DEBUG
endif

ifneq ($(NOT_SUPPORT_SP), y)
    EXTRA_CFLAGS += -fstack-protector-all
endif

ccflags-y += -Wall -Werror

DRIVER_SRC_BASE_DIR := $(DRIVER_KERNEL_DIR)/src

EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
EXTRA_CFLAGS += -I$(TOP_DIR)/abl/libc_sec/include
EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver

EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/inc
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/near/sia/agent/comm
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/near/sia/agent/pcie
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/near/sia/agent/
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/sia/adapt
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/sia/adapt/core

obj-m += ascend_trs_adapt_agent.o
ascend_trs_adapt_agent-objs := pcie/trs_agent.o pcie/trs_d2h_msg_ops.o

ascend_trs_adapt_agent-objs += comm/trs_cache_id_recycle.o
ascend_trs_adapt_agent-objs += trs_pm_agent.o trs_agent_ops.o

