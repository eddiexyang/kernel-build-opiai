LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := drv_ts

LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)

LOCAL_INSTALLED_KO_FILES := drv_ts.ko
LOCAL_DEPEND_KO := drv_dfm drv_devmng
include $(BUILD_DEVICE_KO)
