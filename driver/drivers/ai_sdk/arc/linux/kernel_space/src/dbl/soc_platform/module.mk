LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ascend_soc_platform
LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)
LOCAL_DEPEND_KO:=ascend_soc_resmng ascend_trs_pm_adapt
LOCAL_INSTALLED_KO_FILES := ascend_soc_platform.ko

include $(BUILD_HOST_KO)

