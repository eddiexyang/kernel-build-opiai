LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := drv_lpm_devmng
LOCAL_KBUILD_EXTRA_SYMBOLS := \
	$(PWD)/$(DEVICE_OUT_INTERMEDIATES)/ascend_urd_ko/Module.symvers \
	$(PWD)/$(DEVICE_OUT_INTERMEDIATES)/ascend_uda_ko/Module.symvers \
	$(PWD)/$(DEVICE_OUT_INTERMEDIATES)/drv_devmng_ko/Module.symvers \
	$(PWD)/$(DEVICE_OUT_INTERMEDIATES)/drv_icm_ko/Module.symvers \
	$(PWD)/$(DEVICE_OUT_INTERMEDIATES)/drv_user_cfg_ko/Module.symvers

LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)

LOCAL_INSTALLED_KO_FILES := drv_lpm_devmng.ko

include $(BUILD_DEVICE_KO)
