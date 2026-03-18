LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ascend_virtmng_dev

LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)

LOCAL_DEPEND_KO := drv_pcie drv_icm

LOCAL_INSTALLED_KO_FILES := ascend_virtmng_dev.ko

include $(BUILD_DEVICE_KO)
