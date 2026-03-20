LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := drv_hdmi

LOCAL_CFLAGS := -fstack-protector

LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)

LOCAL_INSTALLED_KO_FILES := drv_hdmi.ko

LOCAL_DEPEND_KO := drv_osal drv_media_compat

include $(BUILD_DEVICE_KO)
