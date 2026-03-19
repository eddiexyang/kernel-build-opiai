LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ascend_trs_cdqm
LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)
LOCAL_INSTALLED_KO_FILES := ascend_trs_cdqm.ko

LOCAL_DEPEND_KO:=drv_davinci_intf_host ascend_trs_id_allocator drv_devmng_host ascend_soc_resmng drv_seclib_host drv_pcie_host
include $(BUILD_HOST_KO)
