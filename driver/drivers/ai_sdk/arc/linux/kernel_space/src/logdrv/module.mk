LOCAL_PATH := $(call my-dir)

#compile for device

include $(CLEAR_VARS)

LOCAL_MODULE := drv_log

LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)
LOCAL_DEPEND_KO:=drv_devdrv drv_devmng drv_pcie_hdc
LOCAL_INSTALLED_KO_FILES := drv_log.ko

include $(BUILD_DEVICE_KO)
