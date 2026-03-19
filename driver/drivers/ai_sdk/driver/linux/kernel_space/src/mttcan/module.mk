LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := drv_mttcan

LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)

LOCAL_DEPEND_KO := ascend_urd drv_user_cfg

LOCAL_INSTALLED_KO_FILES := $(LOCAL_MODULE).ko

include $(BUILD_DEVICE_KO)
