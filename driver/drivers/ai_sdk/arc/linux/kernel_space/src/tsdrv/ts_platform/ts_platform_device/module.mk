

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := drv_platform

LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)

ifeq ($(TARGET_PRODUCT), cloud)
    LOCAL_DEPEND_KO := tee_drv
else ifeq ($(TARGET_PRODUCT),mini)
    ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_DC_V10))
        LOCAL_DEPEND_KO:=drv_pkicms
    else
        LOCAL_DEPEND_KO := tee_drv
    endif
else ifeq ($(TARGET_PRODUCT),mdc)
    ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_MDC_V10))
        LOCAL_DEPEND_KO:=drv_pkicms
    else ifeq ($(TARGET_CHIP_ID), $(ASCEND_MDC_BS9SX1A))
        LOCAL_DEPEND_KO:=drv_pkicms
    endif
endif

LOCAL_DEPEND_KO+=dbl_chip_config

LOCAL_INSTALLED_KO_FILES := drv_platform.ko

include $(BUILD_DEVICE_KO)
