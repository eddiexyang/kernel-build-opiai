LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := dbl_cfg_persistent

LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)

LOCAL_INSTALLED_KO_FILES := dbl_cfg_persistent.ko

include $(BUILD_DEVICE_KO)

