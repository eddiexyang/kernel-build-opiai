LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ascend_trs_sec_eh_agent
LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)
LOCAL_INSTALLED_KO_FILES := ascend_trs_sec_eh_agent.ko

LOCAL_DEPEND_KO:=ascend_trs_pm_adapt
include $(BUILD_HOST_KO)
#LOCAL_DEPEND_KO:=ascend_trs_cdqm ascend_trs_pm_adapt
