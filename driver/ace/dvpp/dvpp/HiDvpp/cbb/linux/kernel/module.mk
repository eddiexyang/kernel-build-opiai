LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := drv_osal

LOCAL_CFLAGS := -fstack-protector

LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)

LOCAL_INSTALLED_KO_FILES := drv_osal.ko

include $(BUILD_DEVICE_KO)
