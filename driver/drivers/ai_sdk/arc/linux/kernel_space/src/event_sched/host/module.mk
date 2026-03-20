LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ascend_event_sched_host

LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)

LOCAL_INSTALLED_KO_FILES := ascend_event_sched_host.ko

LOCAL_DEPEND_KO := ascend_uda drv_davinci_intf_host drv_devmng_host
LOCAL_KBUILD_EXTRA_SYMBOLS := $(PWD)/$(DEVICE_OUT_INTERMEDIATES)/drv_pcie_ko/Module.symvers

include $(BUILD_HOST_KO)
