LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ascend_trs_mia_adapt
LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)
LOCAL_INSTALLED_KO_FILES := ascend_trs_mia_adapt.ko

include $(BUILD_DEVICE_KO)
LOCAL_DEPEND_KO:=ascend_trs_pm_adapt ascend_soc_resmng ascend_trs_id_pool ascend_trs_nvme_chan ascend_trs_id_allocator ascend_trs_core drv_pcie
