LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := hclgeplf

LOCAL_KO_SRC_FOLDER := $(LOCAL_PATH)
LOCAL_DEPEND_KO := hnae3 hns3 drv_devmng ascend_dms_smf mntn_bbox

LOCAL_INSTALLED_KO_FILES := hclgeplf.ko

include $(BUILD_DEVICE_KO)
