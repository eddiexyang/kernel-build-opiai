LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ascend_dms_smf

LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)
LOCAL_DEPEND_KO := ascend_dms_dtm ascend_urd
LOCAL_KBUILD_EXTRA_SYMBOLS := \
	$(PWD)/$(DEVICE_OUT_INTERMEDIATES)/drv_fpdc_ko/Module.symvers

LOCAL_INSTALLED_KO_FILES := ascend_dms_smf.ko

include $(BUILD_HOST_KO)

ifeq ($(build_device), true)

include $(CLEAR_VARS)

LOCAL_MODULE := ascend_dms_smf

LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)
LOCAL_DEPEND_KO := ascend_dms_dtm ascend_urd drv_fpdc

LOCAL_INSTALLED_KO_FILES := ascend_dms_smf.ko

include $(BUILD_DEVICE_KO)

endif
