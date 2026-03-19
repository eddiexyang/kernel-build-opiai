
LOCAL_PATH := $(call my-dir)

#compile for device
include $(CLEAR_VARS)

LOCAL_MODULE := drv_prof

LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)
LOCAL_DEPEND_KO:=drv_devdrv drv_devmng drv_pcie drv_pcie_hdc ascend_uda dbl_chip_config ascend_soc_resmng ascend_trs_nvme_chan ascend_trs_core ascend_trs_pm_adapt ascend_virtmng_dev
LOCAL_INSTALLED_KO_FILES := drv_prof.ko

include $(BUILD_DEVICE_KO)
