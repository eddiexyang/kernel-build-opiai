ifeq ($(TARGET_BUILD_TYPE),debug)
	EXTRA_CFLAGS += -DCFG_BUILD_DEBUG
endif

ifeq ($(TOP_DIR),)
KERNEL_DIR := $(srctree)

EXTRA_CFLAGS += -fstack-protector-all

EXTRA_CFLAGS += -I$(PWD)/drivers/dev_inc
EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/hdcdrv/hdc_host
EXTRA_CFLAGS += -DHDC_ENV_DEVICE

ifeq ($(TARGET_PRODUCT),cloud)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD
    EXTRA_CFLAGS += -DCFG_FEATURE_VFIO_DEVICE
else
    ifeq ($(TARGET_CHIP_ID),$(ASCEND_FLORENCE_DC_V10))
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
        EXTRA_CFLAGS += -DCFG_FEATURE_VFIO_DEVICE
    else
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV1
        EXTRA_CFLAGS += -DCFG_FEATURE_SIMPLIFY_MODE
        EXTRA_CFLAGS += -DCFG_FEATURE_PROCESS_NUM_LITE
    endif
endif

ccflags-y += -Wall -Werror -Wtrampolines $(WDATE_TIME) -Wfloat-equal -Wvla -Wundef -funsigned-char -Wformat=2 -Wstack-usage=2048 -Wcast-align
ccflags-y += -Wextra -Wno-unused-parameter -Wno-sign-compare -Wno-missing-field-initializers

obj-m += drv_pcie_hdc.o
drv_pcie_hdc-objs :=hdcdrv_core.o hdcdrv_core_com.o hdcdrv_device.o hdcdrv_mem.o hdcdrv_mem_com.o hdcdrv_epoll.o hdcdrv_sysfs.o
else # ifeq ($(TOP_DIR),)

EXTRA_CFLAGS += -fstack-protector-all

EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
EXTRA_CFLAGS += -I$(DRIVER_OPEN_KERNEL_DIR)/inc
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/hdcdrv/hdc_host
EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver
EXTRA_CFLAGS += -DHDC_ENV_DEVICE

ifeq ($(PRODUCT),ascend920esl)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_ESL_FPGA
else ifeq ($(PRODUCT),ascend910B)
    EXTRA_CFLAGS += -DCFG_FEATURE_SRIOV
    EXTRA_CFLAGS += -DCFG_FEATURE_MIRROR
    EXTRA_CFLAGS += -DCFG_FEATURE_SIMPLIFY_MODE
else ifeq ($(PRODUCT),ascend910)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD
    EXTRA_CFLAGS += -DCFG_FEATURE_VFIO_DEVICE
else ifeq ($(PRODUCT),ascend310p)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
    EXTRA_CFLAGS += -DCFG_FEATURE_VFIO_DEVICE
else ifeq ($(PRODUCT),ascend310B)

else
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV1
    EXTRA_CFLAGS += -DCFG_FEATURE_SIMPLIFY_MODE
    EXTRA_CFLAGS += -DCFG_FEATURE_PROCESS_NUM_LITE
endif

ifeq ($(PRODUCT),as31xm1)
    EXTRA_CFLAGS += -DCFG_FEATURE_HDC_REG_MEM
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V11
endif

ccflags-y += -Wall -Werror -Wtrampolines $(WDATE_TIME) -Wfloat-equal -Wvla -Wundef -funsigned-char -Wformat=2 -Wstack-usage=2048 -Wcast-align
ccflags-y += -Wextra -Wno-unused-parameter -Wno-sign-compare -Wno-missing-field-initializers

obj-m += drv_pcie_hdc.o
drv_pcie_hdc-objs :=hdcdrv_core.o hdcdrv_core_com.o hdcdrv_device.o hdcdrv_mem.o hdcdrv_mem_com.o hdcdrv_epoll.o hdcdrv_sysfs.o 

ifneq ($(filter $(PRODUCT), as31xm1),)
    EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver/
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/include/
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/config/
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/event/
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/dfm_template/
    EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/core/
    drv_pcie_hdc-objs += hdcdrv_status.o
endif
endif # ifeq ($(TOP_DIR),)
