ifeq ($(TARGET_BUILD_TYPE),debug)
	EXTRA_CFLAGS += -DCFG_BUILD_DEBUG
endif

ifeq ($(TOP_DIR),)

ifneq ($(NOT_SUPPORT_SP), y)
    EXTRA_CFLAGS += -fstack-protector-all
endif

EXTRA_CFLAGS += -DCFG_FEATURE_SHARE_LOG

ifeq ($(DAVINCI_HIAI_DKMS),y)
    EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dev_inc_open/inc
    EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dev_inc/inc
    EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/libc_sec/include

    ifeq ($(TARGET_PRODUCT),cloud)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD_HOST
        ifeq ($(TARGET_CHIP_ID), hi1980b)
            EXTRA_CFLAGS += -DCFG_FEATURE_SRIOV
            EXTRA_CFLAGS += -DCFG_FEATURE_SIMPLIFY_MODE
            EXTRA_CFLAGS += -DCFG_DISABLE_TSD_NOTIFY
        else
            EXTRA_CFLAGS += -DCFG_FEATURE_VFIO
        endif
    else
        ifeq ($(TARGET_CHIP_ID),$(ASCEND_FLORENCE_DC_V10))
            EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
            EXTRA_CFLAGS += -DCFG_FEATURE_VFIO
        else ifeq ($(TARGET_CHIP_ID),hi1910b)

        else
            EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV1
            EXTRA_CFLAGS += -DCFG_FEATURE_SIMPLIFY_MODE
            EXTRA_CFLAGS += -DCFG_FEATURE_PROCESS_NUM_LITE
            ifneq ($(PRODUCT),as31xm1)
                EXTRA_CFLAGS += -DCFG_FEATURE_NOT_SUPPORT_UDA
            endif
        endif
    endif

else
    CUR_MAKEFILE_PATH := $(strip \
                $(eval LOCAL_MODULE_MAKEFILE := $$(lastword $$(MAKEFILE_LIST))) \
                $(patsubst %/,%, $(dir $(LOCAL_MODULE_MAKEFILE))) \
                )

    EXTRA_CFLAGS += -I$(CUR_MAKEFILE_PATH)/../../../../drivers/dev_inc
    EXTRA_CFLAGS += -I$(CUR_MAKEFILE_PATH)/../../../../abl/libc_sec/include
    ifeq ($(TARGET_PRODUCT),cloud)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD_HOST
        EXTRA_CFLAGS += -DCFG_FEATURE_VFIO
    else
        ifeq ($(TARGET_CHIP_ID),$(ASCEND_FLORENCE_DC_V10))
            EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
            EXTRA_CFLAGS += -DCFG_FEATURE_VFIO
        else
            EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV1
            EXTRA_CFLAGS += -DCFG_FEATURE_SIMPLIFY_MODE
            EXTRA_CFLAGS += -DCFG_FEATURE_PROCESS_NUM_LITE
            ifneq ($(PRODUCT),as31xm1)
                EXTRA_CFLAGS += -DCFG_FEATURE_NOT_SUPPORT_UDA
            endif
        endif
    endif

endif

ifeq ($(CUSTOM_OS_CCFLAGS), )
ccflags-y += -Wall -Werror -Wtrampolines $(WDATE_TIME) -Wfloat-equal -Wvla -Wundef -funsigned-char -Wformat=2 -Wstack-usage=2048 -Wcast-align
ccflags-y += -Wextra -Wno-unused-parameter -Wno-sign-compare -Wno-missing-field-initializers -Wno-format-nonliteral -Wno-empty-body
else
ccflags-y += $(CUSTOM_OS_CCFLAGS)
endif

obj-m += drv_pcie_hdc_host.o

ifeq ($(TARGET_PRODUCT),cloud)
    drv_pcie_hdc_host-objs := hdcdrv_core.o hdcdrv_core_com.o hdcdrv_host.o hdcdrv_mem.o hdcdrv_mem_com.o hdcdrv_epoll.o hdcdrv_sysfs.o
    ifneq ($(TARGET_CHIP_ID), hi1980b)
        drv_pcie_hdc_host-objs += hdcdrv_adapter.o
    endif
else
    ifeq ($(TARGET_CHIP_ID),$(ASCEND_FLORENCE_DC_V10))
        drv_pcie_hdc_host-objs := hdcdrv_core.o hdcdrv_core_com.o hdcdrv_host.o hdcdrv_mem.o hdcdrv_mem_com.o hdcdrv_epoll.o hdcdrv_sysfs.o hdcdrv_adapter.o
    else
        drv_pcie_hdc_host-objs := hdcdrv_core.o hdcdrv_core_com.o hdcdrv_host.o hdcdrv_mem.o hdcdrv_mem_com.o hdcdrv_epoll.o hdcdrv_sysfs.o
    endif
endif

else # ifeq ($(TOP_DIR),)

ifneq ($(NOT_SUPPORT_SP), y)
    EXTRA_CFLAGS += -fstack-protector-all
endif

EXTRA_CFLAGS += -DCFG_FEATURE_SHARE_LOG

ifeq ($(DAVINCI_HIAI_DKMS),y)
    EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dev_inc_open/inc
    EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dev_inc/inc
    EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/libc_sec/include

    ifneq ($(filter $(PRODUCT),  ascend910B ascend920esl ascend920emu),)
        EXTRA_CFLAGS += -DCFG_FEATURE_SRIOV
        EXTRA_CFLAGS += -DCFG_DISABLE_TSD_NOTIFY
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD_HOST
        EXTRA_CFLAGS += -DCFG_FEATURE_SIMPLIFY_MODE
    else ifeq ($(PRODUCT),ascend910)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD_HOST
        EXTRA_CFLAGS += -DCFG_FEATURE_VFIO
    else ifeq ($(PRODUCT),ascend310p)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
        EXTRA_CFLAGS += -DCFG_FEATURE_VFIO
    else ifeq ($(PRODUCT),ascend310B)

    else
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV1
        EXTRA_CFLAGS += -DCFG_FEATURE_PROCESS_NUM_LITE
        ifneq ($(PRODUCT),as31xm1)
            EXTRA_CFLAGS += -DCFG_FEATURE_NOT_SUPPORT_UDA
        endif
    endif

    ifeq ($(TARGET_PRODUCT),cloud)
        ifeq ($(TARGET_CHIP_ID), hi1980b)
            EXTRA_CFLAGS += -DCFG_FEATURE_SRIOV
            EXTRA_CFLAGS += -DCFG_DISABLE_TSD_NOTIFY
        endif
    endif
else

    EXTRA_CFLAGS += -I$(DRIVER_OPEN_KERNEL_DIR)/inc
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
    EXTRA_CFLAGS += -I$(TOP_DIR)/abl/libc_sec/include
    ifeq ($(PRODUCT),ascend920esl)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_ESL_FPGA
    else ifeq ($(PRODUCT),ascend910B)
        EXTRA_CFLAGS += -DCFG_FEATURE_SRIOV
        EXTRA_CFLAGS += -DCFG_DISABLE_TSD_NOTIFY
        EXTRA_CFLAGS += -DCFG_FEATURE_SIMPLIFY_MODE
    else ifeq ($(PRODUCT),ascend910)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD_HOST
        EXTRA_CFLAGS += -DCFG_FEATURE_VFIO
    else ifeq ($(PRODUCT),ascend310p)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
        EXTRA_CFLAGS += -DCFG_FEATURE_VFIO
    else ifneq ($(filter $(PRODUCT),  ascend310B),)

    else
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV1
        EXTRA_CFLAGS += -DCFG_FEATURE_SIMPLIFY_MODE
        EXTRA_CFLAGS += -DCFG_FEATURE_PROCESS_NUM_LITE
        ifneq ($(PRODUCT),as31xm1)
            EXTRA_CFLAGS += -DCFG_FEATURE_NOT_SUPPORT_UDA
        endif
    endif

    ifeq ($(PRODUCT),as31xm1)
        EXTRA_CFLAGS += -DCFG_FEATURE_HDC_REG_MEM
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V11
    endif
endif

ifeq ($(CUSTOM_OS_CCFLAGS), )
ccflags-y += -Wall -Werror -Wtrampolines $(WDATE_TIME) -Wfloat-equal -Wvla -Wundef -funsigned-char -Wformat=2 -Wstack-usage=2048 -Wcast-align
ccflags-y += -Wextra -Wno-unused-parameter -Wno-sign-compare -Wno-missing-field-initializers -Wno-format-nonliteral -Wno-empty-body
else
ccflags-y += $(CUSTOM_OS_CCFLAGS)
endif
obj-m += drv_pcie_hdc_host.o

ifneq ($(filter $(PRODUCT), ascend910 ascend310p),)
drv_pcie_hdc_host-objs := hdcdrv_core.o hdcdrv_core_com.o hdcdrv_host.o hdcdrv_mem.o hdcdrv_mem_com.o hdcdrv_epoll.o hdcdrv_sysfs.o hdcdrv_adapter.o
else
drv_pcie_hdc_host-objs := hdcdrv_core.o hdcdrv_core_com.o hdcdrv_host.o hdcdrv_mem.o hdcdrv_mem_com.o hdcdrv_epoll.o hdcdrv_sysfs.o
endif

ifneq ($(filter $(PRODUCT), as31xm1),)
    EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver/
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/include/
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/config/
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/event/
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/dfm_template/
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/core/
    drv_pcie_hdc_host-objs += hdcdrv_status.o
endif
endif # ifeq ($(TOP_DIR),)
