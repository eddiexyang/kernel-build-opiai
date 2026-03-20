
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := drv_pcie_hdc_host

LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)

LOCAL_DEPEND_KO := drv_vpcie ascend_uda drv_devmng_host dbl_runenv_config
LOCAL_KBUILD_EXTRA_SYMBOLS := $(PWD)/$(DEVICE_OUT_INTERMEDIATES)/drv_pcie_ko/Module.symvers

LOCAL_INSTALLED_KO_FILES := drv_pcie_hdc_host.ko

include $(BUILD_HOST_KO)
