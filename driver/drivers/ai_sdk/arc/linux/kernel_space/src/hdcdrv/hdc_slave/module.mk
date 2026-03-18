
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := drv_pcie_hdc

LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)

LOCAL_DEPEND_KO := drv_pcie drv_devmng

LOCAL_INSTALLED_KO_FILES := drv_pcie_hdc.ko

include $(BUILD_DEVICE_KO)
