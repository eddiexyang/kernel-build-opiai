ifeq ($(TARGET_BUILD_TYPE),debug)
    EXTRA_CFLAGS += -DCFG_BUILD_DEBUG
endif

ifeq ($(ENABLE_ASAN),true)
    EXTRA_CFLAGS += -DCFG_ENABLE_ASAN
endif

ifneq ($(NOT_SUPPORT_SP), y)
    EXTRA_CFLAGS += -fstack-protector-all
endif

ccflags-y += -Wall -Werror -Wtrampolines -Wfloat-equal -Wvla -Wundef

ifneq ($(filter $(PRODUCT), ascend310rc ascend310Brc ascend310Brcesl ascend310Brcemu), )
    EXTRA_CFLAGS += -DCFG_FEATURE_RC_MODE
endif

EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dp_proc_mng/agent/comm/
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dp_proc_mng/agent/device/
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dp_proc_mng/common/
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dp_proc_mng/
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dp_proc_mng/agent/device/agent_comm/
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dp_proc_mng/agent/device/dppg_comm/

drv_dp_proc_mng-objs := dppg_comm/dp_proc_mng_module_ops.o agent_comm/dp_proc_mng_pid_map.o dp_proc_mng_agent_msg_client.o dp_proc_mng_agent_channel.o agent_comm/dp_proc_mng_agent_proc_info.o agent_comm/dp_proc_mng_agent_register_ops.o

obj-m := drv_dp_proc_mng.o
