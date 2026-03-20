KERNEL_DIR :=$(srctree)

ifeq ($(TOP_DIR),)         # for DKMS cmake
  ifneq ($(NOT_SUPPORT_SP), y)
      EXTRA_CFLAGS += -fstack-protector-all
  endif

  EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/libc_sec/include
  EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dev_inc/inc
  EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/inc/driver
  EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/common

  EXTRA_CFLAGS += -DCFG_HOST_ENV
  EXTRA_CFLAGS += -DCFG_FEATURE_HOST_ENV

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
      EXTRA_CFLAGS += -DCFG_FEATURE_VFG
  endif

  ifeq ($(TARGET_CHIP_ID), hi1980b)
      EXTRA_CFLAGS += -DCFG_FEATURE_SRIOV
      EXTRA_CFLAGS += -DCFG_FEATURE_VFG
      EXTRA_CFLAGS += -DCFG_CHIP_INFO_FROM_DEVINFO
      EXTRA_CFLAGS += -DCFG_FEATURE_TOPOLOGY_BY_HCCS_STATUS
  else
      EXTRA_CFLAGS += -DCFG_FEATURE_TOPOLOGY_BY_SMP
  endif

else     # for cmake

  ifeq ($(OPIAI_KO_TYPE),HOST)
      EXTRA_CFLAGS += -DCFG_HOST_ENV
      EXTRA_CFLAGS += -DCFG_FEATURE_DEVICE_ENV
      EXTRA_CFLAGS += -DOPIAI_URD_STANDALONE_HOST_BUILD
      ifneq ($(filter $(PRODUCT), ascend910B), )
          EXTRA_CFLAGS += -DCFG_CHIP_INFO_FROM_DEVINFO
      endif
  else
      EXTRA_CFLAGS += -DCFG_FEATURE_DEVICE_ENV
  endif

  EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver
  EXTRA_CFLAGS += -I$(TOP_DIR)/libc_sec/include
  EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
  EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/common

  ifneq ($(filter $(PRODUCT),  ascend610 bs9sx1a ascend610Lite ascend610Liteesl),)
      EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
  endif
  ifneq ($(filter $(PRODUCT),  as31xm1),)
      EXTRA_CFLAGS += -DCFG_FEATURE_DMS_SUPPORT_ALL
      EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V11
  endif
  ifneq ($(filter $(PRODUCT), ascend910B ascend920esl ascend920emu), )
      EXTRA_CFLAGS += -DCFG_FEATURE_SRIOV
      EXTRA_CFLAGS += -DCFG_FEATURE_TOPOLOGY_BY_HCCS_STATUS
  endif
  ifneq ($(filter $(PRODUCT), ascend910B ascend920esl ascend310p), )
      EXTRA_CFLAGS += -DCFG_FEATURE_VFG
  endif
  ifneq ($(filter $(PRODUCT), ascend910B ascend920esl ascend310p), )
      EXTRA_CFLAGS += -DCFG_CHIP_INFO_FROM_DEVINFO
  endif

  ifneq ($(filter $(PRODUCT), ascend310 ascend310p ascend910 ascend910B ascend920esl ascend920emu ascend310B ascend310Besl ascend310Bemu),)
      EXTRA_CFLAGS += -DCFG_FEATURE_EP_MODE
  endif
  ifeq ($(PRODUCT_SIDE), device)
      ifneq ($(filter $(PRODUCT), helper310p ascend310Brc ascend310Brcesl ascend310Brcemu as31xm1),)
          EXTRA_CFLAGS += -DCFG_FEATURE_RC_MODE
      endif
  endif
endif

ifneq ($(NOT_SUPPORT_SP), y)
    EXTRA_CFLAGS += -fstack-protector-all
endif

ifeq ($(TARGET_BUILD_TYPE),debug)
    EXTRA_CFLAGS += -DCFG_BUILD_DEBUG
endif

ccflags-y += -Wall -Werror -Wtrampolines $(WDATE_TIME) -Wfloat-equal -Wvla -Wundef

obj-m += ascend_urd.o
ascend_urd-objs := urd_feature.o urd_init.o urd_notifier.o
ascend_urd-objs += urd_container.o urd_acc_ctrl.o
ascend_urd-objs += urd_kv.o
