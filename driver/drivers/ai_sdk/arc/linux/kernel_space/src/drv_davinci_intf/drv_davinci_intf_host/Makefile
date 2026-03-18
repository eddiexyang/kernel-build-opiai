ifeq ($(TOP_DIR),)
    ifneq ($(NOT_SUPPORT_SP), y)
        EXTRA_CFLAGS += -fstack-protector-all
    endif

    ccflags-y += -Wall -Werror
    obj-m   += drv_davinci_intf_host.o
    drv_davinci_intf_host-objs := davinci_intf_init.o davinci_intf_process.o
	EXTRA_CFLAGS += -DCFG_HOST_ENV
    ifeq ($(TARGET_CHIP_ID), hi1980b)
        EXTRA_CFLAGS += -DCFG_FEATURE_SRIOV
    endif
    ifeq ($(DAVINCI_HIAI_DKMS),y)
        EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dev_inc/inc
        EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/libc_sec/include
        EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/drv_davinci_intf_host
		EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/ts_drv_host/ts_drv_common
    else
        ccflags-y += -funsigned-char -Wextra -Wformat=2 -Wfloat-equal -Wcast-align -Wvla -Wundef -Wstack-usage=2048 $(WDATE_TIME)
        ccflags-y += -Wno-unused-parameter -Wno-sign-compare -Wno-missing-field-initializers
        TOPDIR_ABS := $(KERNEL_DIR)
        EXTRA_CFLAGS += -I$(TOPDIR_ABS)/../../../../../libc_sec/include
        EXTRA_CFLAGS += -I$(TOPDIR_ABS)/../../../../../drivers/ai_sdk/arc/linux/kernel_space/inc
        EXTRA_CFLAGS += -I$(TOPDIR_ABS)/../../../../../drivers/ai_sdk/arc/linux/kernel_space/src/drv_davinci_intf/drv_davinci_intf_host
		EXTRA_CFLAGS += -I$(TOPDIR_ABS)/../../../../../drivers/ai_sdk/arc/linux/kernel_space/src/tsdrv/ts_drv/ts_drv_host/ts_drv_common
    endif
else
    ifneq ($(NOT_SUPPORT_SP), y)
        EXTRA_CFLAGS += -fstack-protector-all
    endif

    ccflags-y += -Wall -Werror
    obj-m   += drv_davinci_intf_host.o
    drv_davinci_intf_host-objs := davinci_intf_init.o davinci_intf_process.o
	EXTRA_CFLAGS += -DCFG_HOST_ENV

    ifneq ($(filter $(PRODUCT),  ascend910B ascend920esl ascend920emu),)
        EXTRA_CFLAGS += -DCFG_FEATURE_SRIOV
    endif

    ifeq ($(DAVINCI_HIAI_DKMS),y)
        EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dev_inc/inc
        EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/libc_sec/include
        EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/drv_davinci_intf_host
		EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/ts_drv_host/ts_drv_common
    else
        ccflags-y += -funsigned-char -Wextra -Wformat=2 -Wfloat-equal -Wcast-align -Wvla -Wundef -Wstack-usage=2048 $(WDATE_TIME)
        ccflags-y += -Wno-unused-parameter -Wno-sign-compare -Wno-missing-field-initializers
        EXTRA_CFLAGS += -I$(TOP_DIR)/libc_sec/include
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_davinci_intf/drv_davinci_intf_host
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_host/ts_drv_common
    endif
endif
