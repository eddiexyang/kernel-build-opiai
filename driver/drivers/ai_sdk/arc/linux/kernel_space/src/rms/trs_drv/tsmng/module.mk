LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ascend_trs_tsmng

LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)
LOCAL_DEPEND_KO := ascend_uda ascend_urd drv_icm

LOCAL_INSTALLED_KO_FILES := ascend_trs_tsmng.ko

include $(BUILD_DEVICE_KO)

