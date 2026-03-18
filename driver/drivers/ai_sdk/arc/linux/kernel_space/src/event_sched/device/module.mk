LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ascend_event_sched

LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)

LOCAL_INSTALLED_KO_FILES := ascend_event_sched.ko
#LOCAL_DEPEND_KO := drv_dfm
include $(BUILD_DEVICE_KO)

