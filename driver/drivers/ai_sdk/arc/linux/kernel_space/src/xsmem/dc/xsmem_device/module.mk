
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ascend_xsmem

LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)
LOCAL_DEPEND_KO := dbl_chip_config drv_davinci_intf drv_pcie drv_devmng

LOCAL_INSTALLED_KO_FILES := ascend_xsmem.ko

include $(BUILD_DEVICE_KO)
