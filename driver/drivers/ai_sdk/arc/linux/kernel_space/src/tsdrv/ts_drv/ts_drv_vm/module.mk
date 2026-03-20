
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := drv_vtsdrv

LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)
LOCAL_DEPEND_KO := drv_vpcie drv_vpc_host drv_devmng_host ascend_uda drv_davinci_intf_host
LOCAL_INSTALLED_KO_FILES := drv_vtsdrv.ko

include $(BUILD_HOST_KO)
