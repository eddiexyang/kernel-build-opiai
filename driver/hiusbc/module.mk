LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := drv_hiusbc

LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)

LOCAL_INSTALLED_KO_FILES := drv_hiusbc.ko

LOCAL_KBUILD_EXTRA_SYMBOLS := $(LOCAL_PATH)/hiusbc_udc_extra.symvers

include $(BUILD_DEVICE_KO)
