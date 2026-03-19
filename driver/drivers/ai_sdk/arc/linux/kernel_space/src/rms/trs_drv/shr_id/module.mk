LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ascend_trs_shrid
LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)
LOCAL_INSTALLED_KO_FILES := ascend_trs_shrid.ko

LOCAL_DEPEND_KO:=drv_davinci_intf_host drv_devmng_host ascend_trs_core drv_seclib_host ascend_event_sched_host
include $(BUILD_HOST_KO)
