LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ascend_vdp_hifb

LOCAL_CFLAGS := -fstack-protector

LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)

LOCAL_INSTALLED_KO_FILES := ascend_vdp_hifb.ko

include $(BUILD_DEVICE_KO)
