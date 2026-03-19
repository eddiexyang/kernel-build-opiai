LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ascend_trs_stars
LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)
LOCAL_INSTALLED_KO_FILES := ascend_trs_stars.ko

LOCAL_DEPEND_KO:=ascend_soc_resmng ascend_uda drv_devmng
include $(BUILD_DEVICE_KO)
