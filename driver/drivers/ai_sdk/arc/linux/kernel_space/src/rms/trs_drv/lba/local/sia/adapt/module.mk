LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ascend_trs_pm_adapt
LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)
LOCAL_INSTALLED_KO_FILES := ascend_trs_pm_adapt.ko

LOCAL_DEPEND_KO:=ascend_trs_nvme_chan ascend_trs_id_allocator ascend_soc_resmng ascend_trs_id_pool ascend_trs_core drv_devmng ascend_urd ascend_uda dbl_chip_config dbl_runenv_config ascend_trs_tsmng ascend_trs_stars
include $(BUILD_DEVICE_KO)
#LOCAL_DEPEND_KO:=ascend_trs_cdqm ascend_trs_nvme_chan ascend_trs_id_allocator ascend_soc_resmng ascend_trs_id_pool ascend_trs_core drv_devmng
