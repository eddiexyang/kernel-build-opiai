
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := drv_user_cfg

LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)

ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_DC_V10))
    LOCAL_DEPEND_KO:=
else
    ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_MDC_V10))
        LOCAL_DEPEND_KO:=
    else ifeq ($(TARGET_CHIP_ID), $(ASCEND_MDC_BS9SX1A))
        LOCAL_DEPEND_KO:=
    else
        LOCAL_DEPEND_KO:=drv_nor_flash tee_drv
    endif
endif

LOCAL_INSTALLED_KO_FILES := drv_user_cfg.ko

include $(BUILD_DEVICE_KO)
