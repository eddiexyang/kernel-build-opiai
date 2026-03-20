LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ascend_soc_platform
LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)
LOCAL_INSTALLED_KO_FILES := ascend_soc_platform.ko
LOCAL_KBUILD_EXTRA_SYMBOLS := \
	$(PWD)/$(DEVICE_OUT_INTERMEDIATES)/ascend_soc_resmng_ko/Module.symvers \
	$(PWD)/$(HOST_OUT_INTERMEDIATES)/ascend_uda_ko/Module.symvers \
	$(PWD)/$(HOST_OUT_INTERMEDIATES)/drv_devmng_host_ko/Module.symvers \
	$(PWD)/$(DEVICE_OUT_INTERMEDIATES)/drv_pcie_ko/Module.symvers \
	$(PWD)/$(DEVICE_OUT_INTERMEDIATES)/ascend_trs_pm_adapt_ko/Module.symvers

include $(BUILD_HOST_KO)
