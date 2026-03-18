#
# Makefile for mttcan driver
#
ifeq ($(TOP_DIR),)
KERNEL_DIR := $(srctree)
EXTRA_CFLAGS += -I$(PWD)/drivers/ai_sdk/driver/linux/kernel_space/inc

ccflags-y += -Wall -Werror

KERNEL_DIR :=$(srctree)

MODULE_NAME = drv_mttcan

obj-m += $(MODULE_NAME).o

$(MODULE_NAME)-objs := can_drv_core.o can_drv_mttcan.o can_drv_interrupt.o can_drv_default_cfg.o \
                       can_drv_api.o can_drv_fault_mgr.o can_drv_sysfs.o bind_core.o drv_snapshot.o \
                       can_drv_dms_init.o can_drv_feature.o

EXTRA_CFLAGS += -DCONFIG_CAN_FAULT_REPORT -DSUPPORT_MTTCAN_FAULT_RECOVERY -DSUPPORT_MTTCAN_DFX

ifneq ($(filter $(PRODUCT),  ascend610 bs9sx1a),)
  EXTRA_CFLAGS += -DSNAPSHOT_ENABLE
  EXTRA_CFLAGS += -DSUPPORT_FLEXIBLE_BIND_CORE
endif

ifeq ($(TARGET_CHIP_ID),$(ASCEND_FLORENCE_MDC_V10))
  EXTRA_CFLAGS += -DRUN_IN_V1
else ifeq ($(TARGET_CHIP_ID), $(ASCEND_MDC_V2))
  EXTRA_CFLAGS += -DRUN_IN_V2
endif

else

EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/smf/sensor/config
EXTRA_CFLAGS += -I${DRIVER_KERNEL_DIR}/src/dms/config/
EXTRA_CFLAGS += -I${DRIVER_KERNEL_DIR}/src/dms/include/
EXTRA_CFLAGS += -I${DRIVER_KERNEL_DIR}/src/dms/smf/sensor/
EXTRA_CFLAGS += -I${DRIVER_KERNEL_DIR}/src/dms/event/
EXTRA_CFLAGS += -I${DRIVER_KERNEL_DIR}/src/dms/smf/event/
EXTRA_CFLAGS += -I${DRIVER_KERNEL_DIR}/src/dms/dfm_template
EXTRA_CFLAGS += -I${DRIVER_KERNEL_DIR}/src/dms/core
EXTRA_CFLAGS += -I${DRIVER_KERNEL_DIR}/src/fpdc
EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver

ccflags-y += -Wall -Werror

KERNEL_DIR :=$(srctree)

MODULE_NAME = drv_mttcan

obj-m += $(MODULE_NAME).o

$(MODULE_NAME)-objs := can_drv_core.o can_drv_mttcan.o can_drv_interrupt.o can_drv_default_cfg.o \
                       can_drv_api.o can_drv_fault_mgr.o can_drv_sysfs.o bind_core.o drv_snapshot.o \
                       can_drv_dms_init.o can_drv_feature.o

EXTRA_CFLAGS += -DCONFIG_CAN_FAULT_REPORT -DSUPPORT_MTTCAN_FAULT_RECOVERY -DSUPPORT_MTTCAN_DFX -DCOMMON_FUNC_PREFIX=mttcan

ifneq ($(NOT_SUPPORT_SP), y)
    EXTRA_CFLAGS += -fstack-protector-all
endif

ifneq ($(filter $(PRODUCT),  ascend610 bs9sx1a),)
  EXTRA_CFLAGS += -DSNAPSHOT_ENABLE
  EXTRA_CFLAGS += -DSUPPORT_FLEXIBLE_BIND_CORE
  EXTRA_CFLAGS += -DSUPPORT_CONFIG_KTIME_TYPE
# Release or Debug
ifeq ($(TARGET_BUILD_TYPE),debug)
  EXTRA_CFLAGS += -DCFG_BUILD_DEBUG
endif
endif

ifeq ($(PRODUCT),ascend610)
  EXTRA_CFLAGS += -DRUN_IN_V1 -DCFG_SOC_PLATFORM_MDC_V51
else ifeq ($(PRODUCT),bs9sx1a)
  EXTRA_CFLAGS += -DRUN_IN_V2 -DCFG_SOC_PLATFORM_MDC_V51
else ifneq ($(findstring $(PRODUCT), ascend310B ascend310Brc),)
  EXTRA_CFLAGS += -DDFM_DISABLE
  EXTRA_CFLAGS += -DSNAPSHOT_ENABLE
  EXTRA_CFLAGS += -DNOT_SUPPORT_OSNAME
  EXTRA_CFLAGS += -DSUPPORT_FLEXIBLE_BIND_CORE
  EXTRA_CFLAGS += -DCTRL_CPU_INFO_USER_CFG
  EXTRA_CFLAGS += -DLOG_PRINT_PID
endif

endif
