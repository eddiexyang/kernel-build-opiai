LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ascend_virtmng_resmgr

LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)

LOCAL_DEPEND_KO := drv_davinci_intf ascend_virtmng_dev

LOCAL_INSTALLED_KO_FILES := ascend_virtmng_resmgr.ko

include $(BUILD_DEVICE_KO)
