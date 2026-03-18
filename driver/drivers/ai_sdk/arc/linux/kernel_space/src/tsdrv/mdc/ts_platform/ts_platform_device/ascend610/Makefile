KERNEL_DIR := $(srctree)
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/drivers/dev_platform/tsdrv/mdc/ts_drv/ts_drv_device
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/drivers/dev_platform/tsdrv/mdc/ts_drv/ts_drv_common
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/drivers/inc
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/drivers/dev_platform/tsdrv/mdc/ts_platform/ts_platform_device
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/drivers/dev_core/device_manager/device_manager_slave/devdrv_manager_slave
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/drivers/dev_core/device_manager/device_manager_inc
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/drivers/inc/comm
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/soc/hi1910p

devdrv_adapt-objs := devdrv_parse_pdata.o

ccflags-y += -Wall -Werror
ccflags-y += $(WDATE_TIME) -Wfloat-equal -Wvla -Wundef -funsigned-char -Wformat=2 -Wstack-usage=2048 -Wcast-align
ccflags-y += -Wextra -Wno-unused-parameter -Wno-sign-compare -Wno-missing-field-initializers -Wno-format-nonliteral -Wno-empty-body
