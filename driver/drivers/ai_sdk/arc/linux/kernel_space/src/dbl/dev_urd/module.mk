LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

PRODUCT_SIDE=host

LOCAL_MODULE := ascend_urd
LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)
LOCAL_DEPEND_KO := drv_davinci_intf_host
LOCAL_INSTALLED_KO_FILES := ascend_urd.ko

include $(BUILD_HOST_KO)

ifeq ($(build_device), true)
    include $(CLEAR_VARS)

    PRODUCT_SIDE=device

    LOCAL_MODULE := ascend_urd
    LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)
    LOCAL_DEPEND_KO := drv_davinci_intf drv_user_cfg
    LOCAL_INSTALLED_KO_FILES := ascend_urd.ko
    include $(BUILD_DEVICE_KO)
endif

