LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ascend_event_sched

LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)

LOCAL_INSTALLED_KO_FILES := ascend_event_sched.ko
LOCAL_DEPEND_KO := \
	ascend_uda \
	drv_devmng \
	drv_pcie \
	drv_davinci_intf \
	ascend_soc_resmng \
	ascend_virtmng_dev \
	ascend_trs_nvme_chan \
	drv_user_cfg
include $(BUILD_DEVICE_KO)
