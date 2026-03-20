LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ascend_trs_sec_eh_agent
LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)
LOCAL_INSTALLED_KO_FILES := ascend_trs_sec_eh_agent.ko
LOCAL_DEPEND_KO := ascend_trs_cdqm ascend_trs_pm_adapt ascend_soc_resmng drv_virtmng_host

LOCAL_KBUILD_EXTRA_SYMBOLS := \
	$(PWD)/$(HOST_OUT_INTERMEDIATES)/ascend_uda_ko/Module.symvers \
	$(PWD)/$(HOST_OUT_INTERMEDIATES)/drv_vpc_host_ko/Module.symvers \
	$(PWD)/$(DEVICE_OUT_INTERMEDIATES)/drv_pcie_ko/Module.symvers
include $(BUILD_HOST_KO)
