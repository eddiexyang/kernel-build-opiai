
ifeq ($(DAVINCI_HIAI_DKMS),y)
    EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/vmng_agent
    EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/vmng_vpc_drv
    EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/vmng_vpc_drv/vpc_agent
else
	EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/virtmng/vmng_agent
	EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/virtmng/vmng_vpc_drv
	EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/virtmng/vmng_vpc_drv/vpc_agent
endif

drv_vpc_agent-objs += vpc_agent/virtmngagent_msg.o vpc_agent/virtmngagent_msg_common.o vpc_agent/virtmngagent_msg_admin.o
drv_vpc_agent-objs += vpc_agent/virtmngagent_vpc.o vpc_agent/virtmngagent_vpc_unit.o
drv_vpc_agent-objs += vpc_agent/vmnga_vpc_interface.o