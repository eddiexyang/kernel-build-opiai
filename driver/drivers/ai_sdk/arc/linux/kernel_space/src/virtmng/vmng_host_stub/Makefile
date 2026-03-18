
ifeq ($(TOP_DIR),)

	ifneq ($(NOT_SUPPORT_SP), y)
		EXTRA_CFLAGS += -fstack-protector-all
	endif
	ifeq ($(DAVINCI_HIAI_DKMS),y)
		EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dev_inc_open/inc
		EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dev_inc/inc
		EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/vmng_public
		EXTRA_CFLAGS += -DVIRTMNG_HOST
	else
	CUR_MAKEFILE_PATH := $(strip \
		$(eval LOCAL_MODULE_MAKEFILE := $$(lastword $$(MAKEFILE_LIST))) \
		$(patsubst %/,%, $(dir $(LOCAL_MODULE_MAKEFILE))) \
		)
	EXTRA_CFLAGS += -I$(CUR_MAKEFILE_PATH)/../../../../drivers/dev_inc
	EXTRA_CFLAGS += -I$(CUR_MAKEFILE_PATH)/../vmng_public
	EXTRA_CFLAGS += -DVIRTMNG_HOST
	endif
ccflags-y += -Wall -Werror -Wtrampolines $(WDATE_TIME) -Wfloat-equal -Wvla -Wundef -funsigned-char -Wformat=2 -Wstack-usage=2048 -Wcast-align
ccflags-y += -Wextra -Wno-unused-parameter -Wno-sign-compare -Wno-missing-field-initializers -Wno-empty-body
obj-m += drv_virtmng_host_stub.o
drv_virtmng_host_stub-objs := virtmnghost_ctrl_stub.o virtmnghost_msg_common_stub.o virtmnghost_vpc_stub.o
else
	ifneq ($(NOT_SUPPORT_SP), y)
		EXTRA_CFLAGS += -fstack-protector-all
	endif
	ifeq ($(DAVINCI_HIAI_DKMS),y)
		EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dev_inc_open/inc
		EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dev_inc/inc
		EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/vmng_public
		EXTRA_CFLAGS += -DVIRTMNG_HOST
	else
	EXTRA_CFLAGS += -I$(DRIVER_OPEN_KERNEL_DIR)/inc
	EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
	EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/virtmng/vmng_public
	EXTRA_CFLAGS += -DVIRTMNG_HOST
	endif
ccflags-y += -Wall -Werror -Wtrampolines $(WDATE_TIME) -Wfloat-equal -Wvla -Wundef -funsigned-char -Wformat=2 -Wstack-usage=2048 -Wcast-align
ccflags-y += -Wextra -Wno-unused-parameter -Wno-sign-compare -Wno-missing-field-initializers -Wno-empty-body
obj-m += drv_virtmng_host_stub.o
drv_virtmng_host_stub-objs := virtmnghost_ctrl_stub.o virtmnghost_msg_common_stub.o virtmnghost_vpc_stub.o
endif
