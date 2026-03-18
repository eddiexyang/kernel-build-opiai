KERNEL_DIR :=$(srctree)

ifeq ($(TOP_DIR),)         # for DKMS cmake
  ifneq ($(NOT_SUPPORT_SP), y)
    EXTRA_CFLAGS += -fstack-protector-all
  endif

  EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/libc_sec/include
  EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dev_inc/inc
  EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dms/dtm
  EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dms/config
  EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dms/smf/sensor
  EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dms/smf/event
  EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dms/smf/core
  EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/common
  EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dms/include
  EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dbl/dev_urd
  EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/tsdrv/ts_drv/ts_drv_common
  EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/inc/driver

  EXTRA_CFLAGS += -DCFG_HOST_ENV

  ifeq ($(TARGET_PRODUCT),mdc)
      EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
  endif

  ifeq ($(TARGET_PRODUCT),mini)
      EXTRA_CFLAGS += -DCFG_FEATURE_EP_MODE
  endif
  ifeq ($(TARGET_PRODUCT),cloud)
      EXTRA_CFLAGS += -DCFG_FEATURE_EP_MODE
  endif
  ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_DC_V10))
      EXTRA_CFLAGS += -DCFG_FEATURE_EP_MODE
  endif

  ifeq ($(TARGET_CHIP_ID), hi1980b)
		EXTRA_CFLAGS += -DCFG_FEATURE_PG
		EXTRA_CFLAGS += -DCFG_FEATURE_SRIOV
		EXTRA_CFLAGS += -DCFG_FEATURE_VFG
		EXTRA_CFLAGS += -DCFG_FEATURE_HOST_AICPU
		EXTRA_CFLAGS += -DCFG_FEATURE_NEW_EVENT_CODE
		EXTRA_CFLAGS += -DCFG_CHIP_INFO_FROM_DEVINFO
		EXTRA_CFLAGS += -DCFG_FEATURE_TOPOLOGY_BY_HCCS_LINK_STATUS
  else
      EXTRA_CFLAGS += -DCFG_FEATURE_TOPOLOGY_BY_SMP
  endif

  ifeq ($(TARGET_CHIP_ID), hi1910b)
      EXTRA_CFLAGS += -DCFG_FEATURE_NEW_EVENT_CODE
  endif
  ccflags-y += -Wall -Werror

  ifeq ($(TARGET_BUILD_TYPE),debug)
    EXTRA_CFLAGS += -DCFG_BUILD_DEBUG
  endif

obj-m += ascend_dms_smf.o
ascend_dms_smf-objs += dms_smf_init.o
include $(HIAI_DKMS_DIR)/dms/smf/core/dms_sensor_core.mk
include $(HIAI_DKMS_DIR)/dms/smf/event/dms_event.mk
include $(HIAI_DKMS_DIR)/dms/smf/sensor/dms_sensor.mk

else     # for cmake

  ifeq ($(PRODUCT_SIDE), host)
      EXTRA_CFLAGS += -DCFG_HOST_ENV
  endif

  ifneq ($(NOT_SUPPORT_SP), y)
      EXTRA_CFLAGS += -fstack-protector-all
  endif

  ifneq ($(filter $(PRODUCT), ascend610 bs9sx1a ascend610Lite ascend610Liteesl),)
      EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
      ifneq ($(filter $(PRODUCT), ascend610Lite),)
          EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_LITE_FPGA
      endif
  endif

  ifneq ($(filter $(PRODUCT), ascend610 bs9sx1a ascend610Lite ascend610Liteesl as31xm1),)
      EXTRA_CFLAGS += -DCFG_SUPPORT_DEFAULT_SEVERITY
  endif

  ifneq ($(filter $(PRODUCT), ascend310B ascend310Brc ascend910B as31xm1),)
      EXTRA_CFLAGS += -DCFG_FEATURE_NEW_EVENT_CODE
  endif
  ifneq ($(filter $(PRODUCT), ascend910B ascend920esl), )
      EXTRA_CFLAGS += -DCFG_FEATURE_PG
      EXTRA_CFLAGS += -DCFG_FEATURE_VFG
      EXTRA_CFLAGS += -DCFG_FEATURE_SRIOV
      EXTRA_CFLAGS += -DCFG_FEATURE_TOPOLOGY_BY_HCCS_LINK_STATUS
  endif
  ifneq ($(filter $(PRODUCT), as31xm1 ascend610 ascend610Lite bs9sx1a), )
      EXTRA_CFLAGS += -DCFG_FEATURE_MDC_CLOCKID_CONFIG
  endif
  EXTRA_CFLAGS += -I$(TOP_DIR)/abl/libc_sec/include
  EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
  EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/dtm
  EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/config
  EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/smf/sensor
  EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/smf/event
  EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/smf/core
  EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/include
  EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dbl/dev_urd
  EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/common
  EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/tsdrv/ts_drv/ts_drv_common
  EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver

  ccflags-y += -Wall -Werror

  ifeq ($(TARGET_BUILD_TYPE),debug)
    EXTRA_CFLAGS += -DCFG_BUILD_DEBUG
  endif

  obj-m += ascend_dms_smf.o
  ascend_dms_smf-objs += dms_smf_init.o
  include ${DRIVER_KERNEL_DIR}/src/dms/smf/core/dms_sensor_core.mk
  include ${DRIVER_KERNEL_DIR}/src/dms/smf/event/dms_event.mk
  include ${DRIVER_KERNEL_DIR}/src/dms/smf/sensor/dms_sensor.mk
  include ${DRIVER_KERNEL_DIR}/src/dms/smf/report/dfm_report.mk
endif
