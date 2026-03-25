LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := sysmonitor

LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)

LOCAL_INSTALLED_KO_FILES := sysmonitor.ko

include $(BUILD_DEVICE_KO)
