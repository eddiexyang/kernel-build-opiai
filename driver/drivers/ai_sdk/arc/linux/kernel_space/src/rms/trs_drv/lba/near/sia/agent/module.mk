LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ascend_trs_adapt_agent
LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)
LOCAL_INSTALLED_KO_FILES := ascend_trs_adapt_agent.ko

include $(BUILD_DEVICE_KO)
LOCAL_DEPEND_KO:=ascend_trs_pm_adapt ascend_trs_core ascend_trs_id_pool drv_pcie ascend_soc_resmng
