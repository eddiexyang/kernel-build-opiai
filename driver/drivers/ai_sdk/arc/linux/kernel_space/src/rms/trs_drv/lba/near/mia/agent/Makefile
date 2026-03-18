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
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc/dbl
EXTRA_CFLAGS += -I$(TOP_DIR)/libc_sec/include
EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver

EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/inc
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/near/mia/agent/
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/near/mia/agent/agent_comm
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/near/sia/agent/
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/lba/local/mia/adapt/

obj-m += ascend_trs_mia_agent.o
ascend_trs_mia_agent-objs := trs_near_mia_agent.o agent_comm/trs_cache_id_recycle.o

