
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := drv_upgrade

LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)

ifeq ($(TARGET_PRODUCT),cloud)
    LOCAL_DEPEND_KO:=drv_nor_flash tee_drv drv_dfm
endif

LOCAL_INSTALLED_KO_FILES := drv_upgrade.ko

include $(BUILD_DEVICE_KO)
