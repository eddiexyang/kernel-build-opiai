LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := drv_lpm_fault
LOCAL_DEPEND_KO := ascend_dms_dtm ascend_dms_smf ascend_uda drv_icm drv_fpdc drv_devmng mntn_bbox
LOCAL_KBUILD_EXTRA_SYMBOLS := \
	$(PWD)/$(DEVICE_OUT_INTERMEDIATES)/ascend_dms_dtm_ko/Module.symvers \
	$(PWD)/$(DEVICE_OUT_INTERMEDIATES)/ascend_dms_smf_ko/Module.symvers \
	$(PWD)/$(DEVICE_OUT_INTERMEDIATES)/ascend_uda_ko/Module.symvers \
	$(PWD)/$(DEVICE_OUT_INTERMEDIATES)/drv_icm_ko/Module.symvers \
	$(PWD)/$(DEVICE_OUT_INTERMEDIATES)/drv_fpdc_ko/Module.symvers \
	$(PWD)/$(DEVICE_OUT_INTERMEDIATES)/drv_devmng_ko/Module.symvers \
	$(PWD)/$(DEVICE_OUT_INTERMEDIATES)/mntn_bbox_ko/Module.symvers

LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)

LOCAL_INSTALLED_KO_FILES := drv_lpm_fault.ko

include $(BUILD_DEVICE_KO)
