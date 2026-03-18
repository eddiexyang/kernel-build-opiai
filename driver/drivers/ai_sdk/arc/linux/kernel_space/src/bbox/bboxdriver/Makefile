ccflags-y += -Wall -Werror -Wtrampolines $(WDATE_TIME) -Wfloat-equal -Wvla -Wundef -funsigned-char -Wformat=2 -Wstack-usage=2048 -Wcast-align -Wextra
ccflags-y += -Wno-unused-parameter -Wno-missing-field-initializers -Wno-sign-compare

ifneq ($(NOT_SUPPORT_SP), y)
    EXTRA_CFLAGS += -fstack-protector-all
endif

ifneq ($(filter $(PRODUCT) $(TARGET_PRODUCT)_$(TARGET_CHIP_ID), ascend910B ascend920esl cloud_$(A9XXGEN02M01)),)
EXTRA_CFLAGS += -DBBOX_SOC_PLATFORM_MILAN
else ifneq ($(filter $(PRODUCT), ascend310B ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu),)
EXTRA_CFLAGS += -DBBOX_SOC_PLATFORM_MILANR3
else ifneq ($(filter $(PRODUCT) $(TARGET_PRODUCT), ascend610 ascend610Lite ascend610Liteesl bs9sx1a mdc),)
EXTRA_CFLAGS += -DBBOX_SOC_PLATFORM_MDC
endif

ifneq ($(filter $(PRODUCT), as31xm1),)
EXTRA_CFLAGS += -DBBOX_SOC_PLATFORM_MDC_V11
EXTRA_CFLAGS += -DCFG_FEATURE_MDC_CLOCK_CONFIG
endif

ifneq ($(filter $(PRODUCT), ascend610 bs9sx1a),)
EXTRA_CFLAGS += -DCFG_FEATURE_MDC_CLOCK_CONFIG
endif

EXTRA_CFLAGS += -I$(TOP_DIR)/abl/bbox/inc/bbox \
                -I$(TOP_DIR)/abl/bbox/inc/bbox/device \
                -I$(TOP_DIR)/inc/toolchain/bbox \
                -I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/inc \
                -I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/src/common  \
                -I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/src/bbox/bboxdriver \
                -I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/src/bbox/bboxdriver/config \
                -I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/src/bbox/bboxdriver/rdr \
                -I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/src/bbox/bboxdriver/common \
                -I${PWD}

GCOV_PROFILE := y

obj-m += mntn_bbox.o

ifneq ($(filter $(PRODUCT) $(TARGET_PRODUCT)_$(TARGET_CHIP_ID), ascend910B ascend920esl cloud_$(A9XXGEN02M01)),)
mntn_bbox-objs += config/bbox_config_dts.o
mntn_bbox-objs += bootcheck/product/bbox_bootcheck_milan.o
mntn_bbox-objs += register/bbox_sctrl_register.o
mntn_bbox-objs += proxy/proxy_common/product/bbox_proxy_milan.o
else ifneq ($(filter $(PRODUCT), ascend310B as31xm1 ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu),)
mntn_bbox-objs += config/bbox_config_dts.o
mntn_bbox-objs += bootcheck/product/bbox_bootcheck_milanr3.o
mntn_bbox-objs += register/bbox_sctrl_register.o
mntn_bbox-objs += proxy/proxy_common/product/bbox_proxy_milanr3.o
else ifneq ($(filter $(PRODUCT) $(TARGET_PRODUCT), ascend610 bs9sx1a mdc ascend610Lite ascend610Liteesl),)
mntn_bbox-objs += config/bbox_config_dts.o
mntn_bbox-objs += bootcheck/product/bbox_bootcheck_mdc.o
mntn_bbox-objs += register/bbox_sctrl_register.o
mntn_bbox-objs += proxy/proxy_common/product/bbox_proxy_mdc.o
mntn_bbox-objs += common/bbox_xpc_proc.o
mntn_bbox-objs += common/bbox_xpc_common.o
mntn_bbox-objs += common/drv_cpu_type.o
endif

mntn_bbox-objs += config/bbox_config.o
mntn_bbox-objs += bootcheck/bbox_bootcheck_pub.o bootcheck/bbox_bootcheck.o bootcheck/hdr_bootcheck.o bootcheck/cdr_bootcheck.o
mntn_bbox-objs += bootparam/bbox_boot_param.o
mntn_bbox-objs += rdr/rdr_core.o rdr/rdr_common.o rdr/rdr_exception_core.o rdr/rdr_module_core.o rdr/rdr_dump_core.o
mntn_bbox-objs += rdr/rdr_field_core.o rdr/rdr_dfx_core.o rdr/product/rdr_interface_mini.o rdr/product/rdr_interface_cloud.o
mntn_bbox-objs += proxy/proxy_common/bbox_proxy_common.o
mntn_bbox-objs += common/bbox_common.o common/bbox_sys_api.o
mntn_bbox-objs += communication/bbox_communication.o communication/bbox_message.o communication/bbox_packet.o communication/bbox_channel_status.o
mntn_bbox-objs += communication/bbox_ioctl.o communication/bbox_ioctl_drv.o
mntn_bbox-objs += os_adapter/bbox_adapter.o os_adapter/bbox_adapter_oom.o os_adapter/bbox_adapter_snapshot.o
mntn_bbox-objs += bbox_init.o
