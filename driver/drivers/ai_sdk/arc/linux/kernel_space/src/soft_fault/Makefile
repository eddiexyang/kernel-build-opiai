ifeq ($(TOP_DIR),) # for DKMS: TOP_DIR is NULL
KERNEL_DIR := $(srctree)
    ifeq ($(DAVINCI_HIAI_DKMS),y)
        ifneq ($(NOT_SUPPORT_SP), y)
            EXTRA_CFLAGS += -fstack-protector-all
        endif
        ccflags-y += -Wall -Werror
        EXTRA_CFLAGS += -DCFG_HOST_ENV
        EXTRA_CFLAGS += -DCFG_FEATURE_HEART_BEAT
        ifeq ($(TARGET_PRODUCT), cloud)
            EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD
            EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/ts_platform_host/ascend910
            EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/drv_devmng/drv_devmng_host/ascend910

            ifeq ($(TARGET_CHIP_ID), hi1980b)
                EXTRA_CFLAGS += -DCFG_FEATURE_SRIOV
            endif
        else ifeq ($(TARGET_PRODUCT), mini)
            ifeq ($(TARGET_CHIP_ID), hi1910)
                EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINI
                EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/ts_platform_host/ascend310
                EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/drv_devmng/drv_devmng_host/ascend310
            else ifeq ($(TARGET_CHIP_ID), hi1910b)
                EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINI
                EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/ts_platform_host/ascend610
                EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/drv_devmng/drv_devmng_host/ascend910
            else
                EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
                EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/ts_platform_host/ascend610
                EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/drv_devmng/drv_devmng_host/ascend910
            endif
        endif

        EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/inc
        EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/inc/driver
        EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/libc_sec/include
        EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/abl/bbox/inc/bbox
        EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/abl/bbox/inc/bbox/device
        EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/abl/ascend_hal/user_space/src/dms/
        EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/libc_sec/include
        EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dev_inc/inc
        EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dms/include
        EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dms/core
        EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dms/config
        EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dms/smf/sensor
        EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dms/smf/sensor/config
        EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dms/smf/event
        EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/ts_drv_common
        EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/drv_devmng/drv_devmng_inc
        EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dbl/dev_urd
    endif

obj-m += drv_soft_fault.o
drv_soft_fault-objs := soft_fault.o dms_sensor_interface.o heartbeat_host.o

else # for cmake and control cpu open
KERNEL_DIR :=$(srctree)
    ifneq ($(NOT_SUPPORT_SP), y)
        EXTRA_CFLAGS += -fstack-protector-all
    endif
    ccflags-y += -Wall -Werror -funsigned-char -Wextra -Wformat=2 -Wfloat-equal -Wcast-align -Wvla -Wundef -Wstack-usage=2048 $(WDATE_TIME)
    ccflags-y += -Wno-unused-parameter -Wno-sign-compare -Wno-missing-field-initializers

    EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver
    EXTRA_CFLAGS += -I${TOP_DIR}/libc_sec/include
    EXTRA_CFLAGS += -I$(TOP_DIR)/abl/bbox/inc/bbox
    EXTRA_CFLAGS += -I$(TOP_DIR)/abl/bbox/inc/bbox/device
    EXTRA_CFLAGS += -I$(TOP_DIR)/inc/toolchain/bbox/device # for ascend310 source release
    EXTRA_CFLAGS += -I$(TOP_DIR)/abl/ascend_hal/user_space/src/dms
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc/dbl
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/include
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/core
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/config
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/smf/sensor
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/smf/sensor/config
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/smf/event
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dbl/dev_urd
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_common
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_inc

    ifeq ($(PRODUCT), ascend910)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD
    else ifeq ($(PRODUCT), ascend310p)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
    endif
    ifneq ($(filter $(PRODUCT), ascend310 ascend310rc ascend310B ascend310Brc as31xm1),)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINI
    endif

    obj-m += drv_soft_fault.o
    drv_soft_fault-objs := soft_fault.o dms_sensor_interface.o
    ifneq ($(filter $(PRODUCT), ascend310 ascend310p ascend910 ascend910B ascend310B),)
        EXTRA_CFLAGS += -DCFG_FEATURE_EP_MODE
    endif

    ifeq ($(PRODUCT_SIDE), host)
        EXTRA_CFLAGS += -DCFG_HOST_ENV
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_host/ascend910
        ifneq ($(filter $(PRODUCT), ascend910 ascend910B),)
            EXTRA_CFLAGS += -I${DRIVER_KERNEL_DIR}/src/tsdrv/ts_platform/ts_platform_host/ascend910
        endif
        ifneq ($(filter $(PRODUCT), ascend310p ascend310B),)
            EXTRA_CFLAGS += -I${DRIVER_KERNEL_DIR}/src/tsdrv/ts_platform/ts_platform_host/ascend610
        endif
        ifneq ($(filter $(PRODUCT), ascend310 ascend310rc),)
            EXTRA_CFLAGS += -I${DRIVER_KERNEL_DIR}/src/tsdrv/ts_platform/ts_platform_host/ascend310
        endif
        drv_soft_fault-objs += heartbeat_host.o
    else
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device/ascend910
        ifneq ($(filter $(PRODUCT), ascend910 ascend910B),)
            EXTRA_CFLAGS += -I${DRIVER_KERNEL_DIR}/src/tsdrv/ts_platform/ts_platform_device/ascend910
        endif
        ifneq ($(filter $(PRODUCT), ascend310p helper310p ascend310B ascend310Brc as31xm1),)
            EXTRA_CFLAGS += -I${DRIVER_KERNEL_DIR}/src/tsdrv/ts_platform/ts_platform_device/ascend610
        endif
        ifneq ($(filter $(PRODUCT), ascend310 ascend310rc),)
            EXTRA_CFLAGS += -I${DRIVER_KERNEL_DIR}/src/tsdrv/ts_platform/ts_platform_device/ascend310
        endif
        ifneq ($(filter $(PRODUCT), ascend610 bs9sx1a ascend610Lite ascend610Liteesl),)
            EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
        endif

        ifneq ($(filter $(PRODUCT), ascend310 ascend310B ascend310p ascend910 ascend910B),)
            drv_soft_fault-objs += heartbeat_device.o
        endif

        ifneq ($(filter $(PRODUCT), ascend310B ascend310p ascend910 ascend910B),)
            drv_soft_fault-objs += os_reset.o
        endif

    endif

    ifneq ($(filter $(PRODUCT), ascend310 ascend310B ascend310p ascend910 ascend910B),)
        EXTRA_CFLAGS += -DCFG_FEATURE_HEART_BEAT
    endif

    ifneq ($(filter $(PRODUCT), ascend910B),)
        EXTRA_CFLAGS += -DCFG_FEATURE_SRIOV
    endif

    ifneq ($(filter $(PRODUCT), ascend310B ascend310p ascend910 ascend910B),)
        EXTRA_CFLAGS += -DCFG_FEATURE_OS_INIT_EVENT
    endif
endif
