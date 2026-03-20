ifeq ($(TOP_DIR),)
ifeq ($(DAVINCI_HIAI_DKMS),y)
    EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dev_inc_open/inc
    EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dev_inc/inc
    EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/vmng_public
    EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/vmng_agent
    EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/vmng_vpc_drv
    EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/vmng_vpc_drv/vpc_agent
endif
else
    EXTRA_CFLAGS += -I$(DRIVER_OPEN_KERNEL_DIR)/inc
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/virtmng/vmng_public
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/virtmng/vmng_agent
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/virtmng/vmng_vpc_drv
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/virtmng/vmng_vpc_drv/vpc_agent
endif

ifneq ($(NOT_SUPPORT_SP), y)
    EXTRA_CFLAGS += -fstack-protector-all
endif
EXTRA_CFLAGS += -DVIRTMNG_AGENT
EXTRA_CFLAGS += -DVMNGA_VPC_MSG_NO_MODULE_INIT

ccflags-y += -Wall -Werror -Wtrampolines $(WDATE_TIME) -Wfloat-equal -Wvla -Wundef -funsigned-char -Wformat=2 -Wstack-usage=2048 -Wcast-align
ccflags-y += -Wextra -Wno-unused-parameter -Wno-sign-compare -Wno-missing-field-initializers
obj-m += drv_vpcie.o
drv_vpcie-objs := virtmngagent_pci.o virtmngagent_ctrl.o virtmngagent_unit.o
drv_vpcie-objs += virtmngagent_extension.o
drv_vpcie-objs += ../vmng_vpc_drv/virtmng_msg_pub.o
drv_vpcie-objs += ../vmng_vpc_drv/virtmng_msg_common.o
drv_vpcie-objs += ../vmng_vpc_drv/virtmng_msg_admin.o
drv_vpcie-objs += ../vmng_vpc_drv/virtmng_extension.o
drv_vpcie-objs += ../vmng_vpc_drv/virtmng_stack.o
drv_vpcie-objs += ../vmng_vpc_drv/virtmng_resource.o
drv_vpcie-objs += ../vmng_vpc_drv/vpc_agent/virtmngagent_msg.o
drv_vpcie-objs += ../vmng_vpc_drv/vpc_agent/virtmngagent_msg_common.o
drv_vpcie-objs += ../vmng_vpc_drv/vpc_agent/virtmngagent_msg_admin.o
drv_vpcie-objs += ../vmng_vpc_drv/vpc_agent/virtmngagent_vpc.o
