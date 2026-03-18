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
    EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dev_inc/inc/dbl
    EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/inc/driver
    EXTRA_CFLAGS += -DDRV_HOST
    ifeq ($(TARGET_PRODUCT),cloud)
        ifneq ($(TARGET_CHIP_ID), hi1980)
            EXTRA_CFLAGS += -DCFG_FEATURE_SURPORT_UDEV_MNG
        endif
    else
        ifneq ($(TARGET_CHIP_ID),$(ASCEND_FLORENCE_DC_V10))
            EXTRA_CFLAGS += -DCFG_FEATURE_SURPORT_UDEV_MNG
        endif
    endif

    ifneq ($(TARGET_CHIP_ID),hi1910)
        EXTRA_CFLAGS += -DCFG_FEATURE_DEVID_TRANS
    endif
else
    DRIVER_SRC_BASE_DIR := $(DRIVER_KERNEL_DIR)/src

    EXTRA_CFLAGS += -I$(TOP_DIR)/libc_sec/include
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc/dbl
    EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver
    ifeq ($(PRODUCT_SIDE), host)
        EXTRA_CFLAGS += -DDRV_HOST
    endif

    ifeq ($(filter $(PRODUCT), ascend310p ascend910),)
        EXTRA_CFLAGS += -DCFG_FEATURE_SURPORT_UDEV_MNG
    endif

    ifeq ($(filter $(PRODUCT), ascend310 ascend310rc),)
        EXTRA_CFLAGS += -DCFG_FEATURE_DEVID_TRANS
    endif
    ifneq ($(filter $(PRODUCT), ascend310Brc ascend310Brcesl),)
        EXTRA_CFLAGS += -DCFG_FEATURE_VFIO_SOC
    endif

endif


obj-m += ascend_uda.o
ascend_uda-objs := uda_access.o uda_dev.o uda_fops.o uda_notifier.o uda_proc_fs.o

