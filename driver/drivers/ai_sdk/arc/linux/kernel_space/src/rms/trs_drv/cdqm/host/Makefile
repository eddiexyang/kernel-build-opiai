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
    EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dev_inc/inc
    EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/inc/driver
    ifeq ($(TARGET_CHIP_ID), hi1980b)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD_V2
    endif
    ifeq ($(TARGET_CHIP_ID), hi1910b)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV3
    endif
else
    DRIVER_SRC_BASE_DIR := $(DRIVER_KERNEL_DIR)/src

    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
    EXTRA_CFLAGS += -I$(TOP_DIR)/abl/libc_sec/include
    EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver
endif

ccflags-y += -Wall -Werror

EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/inc
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/cdqm/comm
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/cdqm/host

EXTRA_CFLAGS += -DCFG_MANAGER_HOST_ENV

ifneq ($(filter $(PRODUCT),  ascend310B ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu),)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV3
endif

ifneq ($(filter $(PRODUCT),  ascend910B ascend920esl ascend920emu),)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD_V2
endif

obj-m += ascend_trs_cdqm.o
ascend_trs_cdqm-objs := host_comm/cdqm_core.o host_comm/cdqm_proc.o host_comm/cdqm_fops.o
ascend_trs_cdqm-objs += cdq_manager.o cdqm_host.o

