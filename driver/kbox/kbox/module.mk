LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := kbox

LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)

LOCAL_INSTALLED_KO_FILES := kbox.ko

LOCAL_DEPEND_KO := kbox_init

include $(BUILD_DEVICE_KO)
