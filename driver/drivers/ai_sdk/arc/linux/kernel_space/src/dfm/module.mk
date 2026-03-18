

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := drv_dfm

LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)

LOCAL_INSTALLED_KO_FILES := drv_dfm.ko

LOCAL_DEPEND_KO := mntn_bbox

include $(BUILD_DEVICE_KO)
