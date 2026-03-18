

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := drv_devdrv


LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)
LOCAL_DEPEND_KO:=drv_devmng drv_pcie
LOCAL_INSTALLED_KO_FILES := drv_devdrv.ko

include $(BUILD_DEVICE_KO)
