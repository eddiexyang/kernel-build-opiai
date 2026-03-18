
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := drv_pcie

LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)

LOCAL_DEPEND_KO := dbl_runenv_config drv_dfm

LOCAL_INSTALLED_KO_FILES := drv_pcie.ko

include $(BUILD_DEVICE_KO)
