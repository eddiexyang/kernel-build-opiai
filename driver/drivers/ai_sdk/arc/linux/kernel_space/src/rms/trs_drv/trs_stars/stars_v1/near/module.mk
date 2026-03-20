LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ascend_trs_sub_stars
LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)
LOCAL_INSTALLED_KO_FILES := ascend_trs_sub_stars.ko

LOCAL_KBUILD_EXTRA_SYMBOLS := \
	$(PWD)/$(HOST_OUT_INTERMEDIATES)/ascend_soc_resmng_ko/Module.symvers \
	$(PWD)/$(HOST_OUT_INTERMEDIATES)/ascend_uda_ko/Module.symvers
include $(BUILD_HOST_KO)
