LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := drv_soc_misc

LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)
LOCAL_DEPEND_KO := drv_devmng ascend_dms_dtm ascend_dms_smf ascend_urd drv_fpdc ascend_uda drv_user_cfg

LOCAL_INSTALLED_KO_FILES := drv_soc_misc.ko

include $(BUILD_DEVICE_KO)

