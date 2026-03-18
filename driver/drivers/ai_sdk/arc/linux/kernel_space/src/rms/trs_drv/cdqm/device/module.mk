LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ascend_trs_cdqm
LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)
LOCAL_INSTALLED_KO_FILES := ascend_trs_cdqm.ko

include $(BUILD_DEVICE_KO)
LOCAL_DEPEND_KO:=drv_davinci_intf ascend_trs_id_allocator drv_devmng ascend_soc_resmng drv_pcie
