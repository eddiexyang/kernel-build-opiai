LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ascend_trs_mia_agent
LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)
LOCAL_INSTALLED_KO_FILES := ascend_trs_mia_agent.ko

LOCAL_DEPEND_KO:=drv_pcie ascend_trs_id_pool ascend_trs_adapt_agent ascend_trs_mia_adapt ascend_trs_pm_adapt drv_devmng
include $(BUILD_DEVICE_KO)
