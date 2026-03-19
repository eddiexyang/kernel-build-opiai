LOCAL_PATH := $(call my-dir)

#compile for device

include $(CLEAR_VARS)

LOCAL_MODULE := drv_log

LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)
LOCAL_DEPEND_KO:=drv_devdrv drv_devmng drv_pcie_hdc ascend_trs_pm_adapt ascend_uda dbl_chip_config ascend_trs_nvme_chan ascend_soc_resmng drv_icm
LOCAL_INSTALLED_KO_FILES := drv_log.ko

include $(BUILD_DEVICE_KO)
