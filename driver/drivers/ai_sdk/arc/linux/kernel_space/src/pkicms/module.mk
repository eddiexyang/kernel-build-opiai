LOCAL_PATH := $(call my-dir)

#compile for device
include $(CLEAR_VARS)

LOCAL_MODULE := drv_pkicms

LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)
LOCAL_DEPEND_KO:=drv_user_cfg
LOCAL_INSTALLED_KO_FILES := drv_pkicms.ko

LOCAL_C_INCLUDES += $(LOCAL_PATH)

include $(BUILD_DEVICE_KO)
