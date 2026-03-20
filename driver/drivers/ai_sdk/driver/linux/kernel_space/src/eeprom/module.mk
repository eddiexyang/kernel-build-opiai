
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := drv_e2prom
LOCAL_DEPEND_KO := drv_user_cfg

LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)

LOCAL_INSTALLED_KO_FILES := drv_e2prom.ko

include $(BUILD_DEVICE_KO)
