LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := drv_dp_proc_mng

LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)
LOCAL_DEPEND_KO:=drv_pcie drv_devmng drv_davinci_intf
LOCAL_INSTALLED_KO_FILES := drv_dp_proc_mng.ko

include $(BUILD_DEVICE_KO)
