LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ascend_trs_pm_adapt
LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)
LOCAL_INSTALLED_KO_FILES := ascend_trs_pm_adapt.ko

LOCAL_DEPEND_KO:=ascend_trs_sub_stars ascend_trs_core ascend_trs_nvme_chan ascend_trs_id_allocator ascend_soc_resmng drv_pcie_host drv_seclib_host drv_devmng_host ascend_uda ascend_urd
include $(BUILD_HOST_KO)
# LOCAL_DEPEND_KO:=ascend_trs_sub_stars ascend_trs_cdqm ascend_trs_core ascend_trs_nvme_chan ascend_trs_id_allocator ascend_soc_resmng drv_pcie_host drv_seclib_host drv_devmng_host
