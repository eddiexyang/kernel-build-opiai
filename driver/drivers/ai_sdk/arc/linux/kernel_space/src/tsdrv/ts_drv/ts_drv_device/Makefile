MAKE_PRODUCT := $(PRODUCT)
ifeq ($(TARGET_BUILD_TYPE),debug)
	EXTRA_CFLAGS += -DCFG_BUILD_DEBUG
endif

# gmake
ifeq ($(TOP_DIR),)
	KERNEL_DIR :=$(srctree)
	TOP_DIR := $(KERNEL_DIR)/../..
    DRIVER_KERNEL_DIR := $(DRIVER_SRC_BASE_DIR)
    DRIVER_SRC_BASE_DIR := $(DRIVER_SRC_BASE_DIR)

	ifeq ($(TARGET_PRODUCT),cloud)
		MAKE_PRODUCT := ascend910
		ifeq ($(TARGET_CHIP_ID), $(A9XXGEN02M01))
			MAKE_PRODUCT := ascend910B
		endif
	else ifeq ($(TARGET_PRODUCT),mini)
		MAKE_PRODUCT := ascend310
		ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_DC_V10))
		    MAKE_PRODUCT := ascend310p
		endif
	else ifeq ($(TARGET_PRODUCT),mdc)
		ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_MDC_V10))
		    MAKE_PRODUCT := ascend610
		else ifeq ($(TARGET_CHIP_ID), $(ASCEND_MDC_BS9SX1A))
		    MAKE_PRODUCT := bs9sx1a
		endif
	else ifeq ($(TARGET_PRODUCT),helper)
	     MAKE_PRODUCT := helper310p
	endif
# cmake
else
    MAKE_PRODUCT := $(PRODUCT)
    DRIVER_SRC_BASE_DIR := $(DRIVER_KERNEL_DIR)/src
endif

EXTRA_CFLAGS += -DCFG_FEATURE_IPC_NOTIFY

ifneq ($(filter $(MAKE_PRODUCT),  ascend910 ascend910B ascend920esl ascend920emu),)
	EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD
	EXTRA_CFLAGS += -DCFG_FEATURE_PSS_SIGN
	EXTRA_CFLAGS += -DCFG_FEATURE_SURPORT_P2P
	EXTRA_CFLAGS += -DCFG_FEATURE_USE_MAILBOX_RECYCLE
    ifneq ($(filter $(MAKE_PRODUCT),  ascend910),)
        EXTRA_CFLAGS += -DCFG_FEATURE_RUNTIME_NO_THREAD
    endif
	ifneq ($(filter $(MAKE_PRODUCT),  ascend910B ascend920esl ascend920emu),)
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_ESL_FPGA
		EXTRA_CFLAGS += -DVER_CDQM_B607
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD_V2
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_DEVICE
		EXTRA_CFLAGS += -DCFG_FEATURE_CDQM
		EXTRA_CFLAGS += -DCFG_FEATURE_CALLBACK_EVENT
		EXTRA_CFLAGS += -DCFG_FEATURE_VFIO
		EXTRA_CFLAGS += -DCFG_FEATURE_SUPPORT_VSQ_MNG
		EXTRA_CFLAGS += -DCFG_FEATURE_RUNTIME_NO_THREAD
		EXTRA_CFLAGS += -DCFG_FEATURE_STARS
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_common/tsdrv_cdqm
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_device/tsdrv_cdqm
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_device/soc/cdqm_v1
		ifneq ($(filter ascend920esl, $(MAKE_PRODUCT)),)
			EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD_V2_ESL
		endif
	endif
else ifneq ($(filter $(MAKE_PRODUCT),  ascend310 ascend310rc ascend310B as31xm1 ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu ascend310p helper310p),)
	EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINI
	ifneq ($(filter $(MAKE_PRODUCT),  ascend310 ascend310rc),)
		EXTRA_CFLAGS += -DCFG_FEATURE_SURPORT_P2P
	endif
    ifneq ($(filter $(MAKE_PRODUCT),  ascend310B as31xm1 ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu),)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV3
        EXTRA_CFLAGS += -DCFG_FEATURE_CDQM
        EXTRA_CFLAGS += -DCFG_FEATURE_CMO
        EXTRA_CFLAGS += -DCFG_FEATURE_SURPORT_P2P
        EXTRA_CFLAGS += -DCFG_FEATURE_SUPPORT_VSQ_MNG
        EXTRA_CFLAGS += -DCFG_FEATURE_USE_MAILBOX_RECYCLE
		EXTRA_CFLAGS += -DCFG_FEATURE_RUNTIME_NO_THREAD
		EXTRA_CFLAGS += -DCFG_FEATURE_STARS
		EXTRA_CFLAGS += -DCFG_FEATURE_CALLBACK_EVENT
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_common/tsdrv_cdqm
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_device/tsdrv_cdqm
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_device/soc/cdqm_v1
        ifneq ($(filter $(MAKE_PRODUCT), ascend310B as31xm1 ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu),)
            EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_ESL_FPGA
        endif
    endif
    ifneq ($(filter $(PRODUCT), ascend310rc ascend310Brc ascend310Brcesl ascend310Brcemu), )
        EXTRA_CFLAGS += -DCFG_FEATURE_RC_MODE
    endif
	ifeq ($(MAKE_PRODUCT), ascend310p)
		EXTRA_CFLAGS += -DCFG_FEATURE_ID_MAPING
		EXTRA_CFLAGS += -DCFG_FEATURE_SURPORT_P2P
	endif
	ifneq ($(filter $(MAKE_PRODUCT), ascend310p helper310p),)
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
		EXTRA_CFLAGS += -DCFG_FEATURE_PSS_SIGN
		EXTRA_CFLAGS += -DCFG_FEATURE_RUNTIME_NO_THREAD
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_DEVICE
		EXTRA_CFLAGS += -DCFG_FEATURE_USE_MAILBOX_RECYCLE
		EXTRA_CFLAGS += -DCFG_FEATURE_IPC_NOTIFY_WITH_EVENTID
	endif
	ifeq ($(MAKE_PRODUCT), helper310p)
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_HELPER
    endif
else ifneq ($(filter $(MAKE_PRODUCT),  ascend610 bs9sx1a),)
	EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
	EXTRA_CFLAGS += -DCFG_FEATURE_RUNTIME_NO_THREAD
	EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC
	EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
	EXTRA_CFLAGS += -DSUPPORT_FLEXIBLE_BIND_CORE
endif

EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
EXTRA_CFLAGS += -I$(TOP_DIR)/libc_sec/include
EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver

ifneq ($(filter $(MAKE_PRODUCT),  ascend910 ascend910B ascend920esl ascend920emu),)
	EXTRA_CFLAGS += -I$(TOP_DIR)/inc/tee
        EXTRA_CFLAGS += -I$(TOP_DIR)/tee/inc/tee
	EXTRA_CFLAGS += -I$(TOP_DIR)/inc/soc/cloud_v1
	EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/config/user_config/cloud
	EXTRA_CFLAGS += -I$(TOP_DIR)/config/user_config/cloud
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/drv_devmng/drv_devmng_inc/
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_common
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_common/tsdrv_dev
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_common/tsdrv_id
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_common/tsdrv_nvme
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_common/tsdrv_nvme/shm
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_common/tsdrv_nvme/logic
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_platform/ts_platform_device/ascend910
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_platform/ts_platform_device
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/drv_devmng/drv_devmng_device/ascend910
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_device/ascend910
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_device/ascend910/tsdrv_conflict_check
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_common/tsdrv_conflict_check/ascend910
else ifneq ($(filter $(MAKE_PRODUCT), ascend310B as31xm1 ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu ascend310p helper310p),)
	EXTRA_CFLAGS += -I$(TOP_DIR)/inc/soc/hi1910p
	ifneq ($(filter $(MAKE_PRODUCT), ascend310p helper310p),)
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/config/user_config/miniv2
		EXTRA_CFLAGS += -I$(TOP_DIR)/config/user_config/miniv2
	else
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/config/user_config/miniv3
	endif
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/drv_devmng/drv_devmng_inc/
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_common
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_common/tsdrv_dev
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_common/tsdrv_id
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_common/tsdrv_nvme
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_common/tsdrv_nvme/shm
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_common/tsdrv_nvme/logic
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_platform/ts_platform_device/ascend610
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_platform/ts_platform_device
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/drv_devmng/drv_devmng_device/ascend910
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_device/ascend610
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_device/ascend610/tsdrv_conflict_check
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_common/tsdrv_conflict_check/ascend610
else ifneq ($(filter $(MAKE_PRODUCT),  ascend610 bs9sx1a),)
	EXTRA_CFLAGS += -I$(TOP_DIR)/inc/soc/hi1910p
	EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/config/user_config/miniv2
	EXTRA_CFLAGS += -I$(TOP_DIR)/config/user_config/miniv2
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/drv_devmng/drv_devmng_inc/
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_common
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_common/tsdrv_dev
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_common/tsdrv_id
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_common/tsdrv_nvme
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_common/tsdrv_nvme/shm
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_common/tsdrv_nvme/logic
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_platform/ts_platform_device/ascend610
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_platform/ts_platform_device
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/drv_devmng/drv_devmng_device/ascend910
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_device/ascend610
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_device/ascend610/tsdrv_conflict_check
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_common/tsdrv_conflict_check/ascend610
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/common
	EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_notify
else ifneq ($(filter $(MAKE_PRODUCT),  ascend310 ascend310rc),)
	EXTRA_CFLAGS += -I$(TOP_DIR)/inc/tee
        EXTRA_CFLAGS += -I$(TOP_DIR)/tee/inc/tee
	EXTRA_CFLAGS += -I$(TOP_DIR)/inc/soc/mini
	EXTRA_CFLAGS += -I$(TOP_DIR)/inc/soc/mini_v1
	EXTRA_CFLAGS += -I$(TOP_DIR)/abl/bbox/inc/bbox/device
	EXTRA_CFLAGS += -I$(TOP_DIR)/inc/toolchain/bbox/device
	EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/config/user_config/mini
	EXTRA_CFLAGS += -I$(TOP_DIR)/config/user_config/mini
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/drv_devmng/drv_devmng_inc/
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_device/ascend310
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_common
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_common/cbsqcq/
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_common/tsdrv_dev/
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_common/tsdrv_id
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_platform/ts_platform_device
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_platform/ts_platform_device/ascend310
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_device/ascend310/tsdrv_conflict_check
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_common/tsdrv_conflict_check/ascend310
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_common/tsdrv_nvme/sync_cb
	EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/drv_devmng/drv_devmng_device/ascend310
endif

EXTRA_CFLAGS += -I$(TOP_DIR)/abl/bbox/inc/bbox
EXTRA_CFLAGS += -I$(TOP_DIR)/inc/toolchain/bbox
EXTRA_CFLAGS += -I$(TOP_DIR)/inc/aicpu/common
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_device
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_common/devdrv_callback
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_device/tsdrv_virt_stub
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_common/tsdrv_nvme
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_common/tsdrv_nvme/shm
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_common/tsdrv_nvme/logic
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_common/tsdrv_osal
EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/tsdrv/ts_drv/ts_drv_vm

ccflags-y += -Wall -Werror
ccflags-y += $(WDATE_TIME) -Wfloat-equal -Wvla -Wundef -funsigned-char  -Wformat=2 -Wstack-usage=2048 -Wcast-align
ccflags-y += -Wextra -Wno-unused-parameter -Wno-sign-compare -Wno-missing-field-initializers -Wno-format-nonliteral -Wno-empty-body

ifneq ($(filter $(MAKE_PRODUCT), ascend310 ascend310rc ascend310B as31xm1 ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu ascend610 bs9sx1a helper310p ascend310p ascend910 ascend910B ascend920esl ascend920emu),)
	obj-m += drv_devdrv.o
else
	obj-$(CONFIG_NPU_DEVDRV_DEVDRIVER) += drv_devdrv.o
endif

ifeq ($(filter $(MAKE_PRODUCT),  ascend310 ascend310rc),)
	drv_devdrv-objs := ts_drv_common/devdrv_cqsq.o ts_drv_common/devdrv_devinit.o tsdrv_drvops.o  tsdrv_get_ssid.o
	drv_devdrv-objs += ts_drv_common/devdrv_functional_cqsq.o ts_drv_common/devdrv_mailbox.o ts_drv_common/tsdrv_delay_recycle.o
	drv_devdrv-objs += ts_drv_common/devdrv_id.o ts_drv_common/devdrv_recycle.o ts_drv_common/devdrv_shm.o
	drv_devdrv-objs += ts_drv_common/tsdrv_dev/tsdrv_device.o ts_drv_common/tsdrv_dev/tsdrv_ctx.o ts_drv_common/tsdrv_dev/tsdrv_capacity.o ts_drv_common/tsdrv_dev/tsdrv_hwinfo.o ts_drv_common/tsdrv_dev/tsdrv_hwinfo_platform.o
	drv_devdrv-objs += ts_drv_common/tsdrv_dev/tsdrv_dev_dfx.o ts_drv_common/tsdrv_dev/tsdrv_id_dfx.o ts_drv_common/tsdrv_dev/tsdrv_mailbox_dfx.o ts_drv_common/tsdrv_dev/tsdrv_comm_fops.o
	drv_devdrv-objs += ts_drv_common/tsdrv_dev/tsdrv_ctx_dfx.o ts_drv_common/tsdrv_dev/tsdrv_event_id_dfx.o ts_drv_common/tsdrv_dev/tsdrv_id_config_dfx.o
	drv_devdrv-objs += ts_drv_common/tsdrv_id/tsdrv_id.o ts_drv_common/tsdrv_id/tsdrv_sync.o
	drv_devdrv-objs += ts_drv_common/tsdrv_nvme/tsdrv_nvme.o ts_drv_common/tsdrv_nvme/shm/shm_sqcq.o ts_drv_common/tsdrv_nvme/logic/logic_cq.o ts_drv_common/tsdrv_nvme/logic/tsdrv_logic_cq.o
	drv_devdrv-objs += ts_drv_common/devdrv_callback/devdrv_cb.o ts_drv_common/devdrv_callback/devdrv_cbctrl.o ts_drv_common/devdrv_callback/tsdrv_cb_pm.o
	drv_devdrv-objs += ts_drv_common/devdrv_ipc_notify.o ts_drv_common/devdrv_nsm.o tsdrv_notify.o
	ifneq ($(filter $(MAKE_PRODUCT),  ascend910 ascend910B ascend920esl ascend920emu),)
		drv_devdrv-objs += ascend910/devdrv_config_p2p.o
		drv_devdrv-objs += ts_drv_common/tsdrv_conflict_check/ascend910/tsdrv_set_runtime_conflict_check.o
		drv_devdrv-objs += ascend910/tsdrv_conflict_check/tsdrv_bar_init.o
		drv_devdrv-objs += ts_drv_common/tsdrv_id/hvtsdrv_id.o ts_drv_common/tsdrv_id/hvtsdrv_id_event.o ts_drv_common/tsdrv_id/hvtsdrv_cqsq.o
		drv_devdrv-objs += tsdrv_virt_stub/hvtsdrv_proc.o tsdrv_virt_stub/hvtsdrv_cb.o tsdrv_virt_stub/hvtsdrv_logic_cq.o tsdrv_virt_stub/hvtsdrv_shm_sqcq.o
		ifneq ($(filter $(PRODUCT),  ascend910B ascend920esl ascend920emu),)
			drv_devdrv-objs += tsdrv_cdqm/tsdrv_cdqm_dev.o
			drv_devdrv-objs += ts_drv_common/tsdrv_cdqm/tsdrv_cdqm_module.o
			drv_devdrv-objs += soc/cdqm_v1/cdq_manager.o
			drv_devdrv-objs += ts_drv_common/tsdrv_vsq.o ts_drv_common/hvtsdrv_tsagent.o
			drv_devdrv-objs += tsdrv_virt/dvtsdrv_init.o tsdrv_virt/dvtsdrv_sriov_id.o tsdrv_virt/dvtsdrv_sriov_hwinfo.o
	    else
	        drv_devdrv-objs += tsdrv_virt_stub/hvtsdrv_tsagent.o
		endif
	else ifneq ($(filter $(MAKE_PRODUCT), ascend310B as31xm1 ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu ascend610 bs9sx1a ascend310p helper310p),)
		drv_devdrv-objs += ascend610/devdrv_config_p2p.o
		drv_devdrv-objs += ts_drv_common/tsdrv_conflict_check/ascend610/tsdrv_set_runtime_conflict_check.o
		drv_devdrv-objs += ascend610/tsdrv_conflict_check/tsdrv_bar_init.o
		drv_devdrv-objs += devdrv_devinfo.o
		ifneq ($(filter $(MAKE_PRODUCT),  ascend610 bs9sx1a),)
			drv_devdrv-objs += ascend610/bind_core.o
			drv_devdrv-objs += ts_drv_common/tsdrv_rts_streamid.o
			drv_devdrv-objs += ascend610/tsdrv_ts_node.o
		else
			drv_devdrv-objs += ts_drv_common/tsdrv_id/hvtsdrv_id.o ts_drv_common/tsdrv_id/hvtsdrv_id_event.o ts_drv_common/tsdrv_id/hvtsdrv_cqsq.o
			drv_devdrv-objs += tsdrv_virt_stub/hvtsdrv_proc.o tsdrv_virt_stub/hvtsdrv_cb.o tsdrv_virt_stub/hvtsdrv_logic_cq.o tsdrv_virt_stub/hvtsdrv_shm_sqcq.o
			ifneq ($(filter $(MAKE_PRODUCT),  ascend310B as31xm1 ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu),)
				drv_devdrv-objs += tsdrv_cdqm/tsdrv_cdqm_dev.o
				drv_devdrv-objs += ts_drv_common/tsdrv_cmo_id.o
				drv_devdrv-objs += ts_drv_common/tsdrv_cdqm/tsdrv_cdqm_module.o
				drv_devdrv-objs += soc/cdqm_v1/cdq_manager.o
				drv_devdrv-objs += ts_drv_common/tsdrv_vsq.o ts_drv_common/hvtsdrv_tsagent.o
		    else
		        drv_devdrv-objs += tsdrv_virt_stub/hvtsdrv_tsagent.o
			endif
		endif
	endif
else
	drv_devdrv-objs := ts_drv_common/devdrv_cqsq.o ts_drv_common/devdrv_devinit.o  tsdrv_get_ssid.o
	drv_devdrv-objs += ts_drv_common/devdrv_functional_cqsq.o ts_drv_common/devdrv_mailbox.o ts_drv_common/tsdrv_delay_recycle.o
	drv_devdrv-objs += ts_drv_common/devdrv_id.o ts_drv_common/devdrv_recycle.o ts_drv_common/devdrv_shm.o
	drv_devdrv-objs += ts_drv_common/tsdrv_dev/tsdrv_device.o ts_drv_common/tsdrv_dev/tsdrv_ctx.o
	drv_devdrv-objs += ts_drv_common/tsdrv_dev/tsdrv_capacity.o ts_drv_common/tsdrv_dev/tsdrv_hwinfo.o ts_drv_common/tsdrv_dev/tsdrv_hwinfo_platform.o
	drv_devdrv-objs += ts_drv_common/tsdrv_dev/tsdrv_dev_dfx.o ts_drv_common/tsdrv_dev/tsdrv_id_dfx.o ts_drv_common/tsdrv_dev/tsdrv_mailbox_dfx.o ts_drv_common/tsdrv_dev/tsdrv_comm_fops.o
	drv_devdrv-objs += ts_drv_common/tsdrv_dev/tsdrv_ctx_dfx.o ts_drv_common/tsdrv_dev/tsdrv_event_id_dfx.o ts_drv_common/tsdrv_dev/tsdrv_id_config_dfx.o
	drv_devdrv-objs += ts_drv_common/tsdrv_id/tsdrv_id.o ts_drv_common/tsdrv_id/tsdrv_sync.o
	drv_devdrv-objs += ts_drv_common/tsdrv_nvme/tsdrv_nvme.o ts_drv_common/tsdrv_nvme/shm/shm_sqcq.o ts_drv_common/tsdrv_nvme/logic/logic_cq.o ts_drv_common/tsdrv_nvme/logic/tsdrv_logic_cq.o
	drv_devdrv-objs += ts_drv_common/tsdrv_nvme/sync_cb/devdrv_sync_cbctrl.o ts_drv_common/tsdrv_nvme/sync_cb/devdrv_cbcq.o ts_drv_common/tsdrv_nvme/sync_cb/devdrv_cbsq.o
	drv_devdrv-objs += ts_drv_common/tsdrv_nvme/sync_cb/devdrv_cb_stub.o
	drv_devdrv-objs += ts_drv_common/tsdrv_id/hvtsdrv_id.o ts_drv_common/tsdrv_id/hvtsdrv_id_event.o ts_drv_common/tsdrv_id/hvtsdrv_cqsq.o
	drv_devdrv-objs += ts_drv_common/tsdrv_conflict_check/ascend310/tsdrv_set_runtime_conflict_check.o
	drv_devdrv-objs += ascend310/devdrv_config_p2p.o tsdrv_drvops.o
	drv_devdrv-objs += ascend310/tsdrv_conflict_check/tsdrv_bar_init.o
	drv_devdrv-objs += ts_drv_common/devdrv_ipc_notify.o ts_drv_common/devdrv_nsm.o tsdrv_notify.o
	drv_devdrv-objs += tsdrv_virt_stub/hvtsdrv_proc.o tsdrv_virt_stub/hvtsdrv_cb.o tsdrv_virt_stub/hvtsdrv_logic_cq.o tsdrv_virt_stub/hvtsdrv_shm_sqcq.o
	drv_devdrv-objs += tsdrv_virt_stub/hvtsdrv_tsagent.o
endif

drv_devdrv-objs += ts_drv_common/tsdrv_osal/tsdrv_osal_intr.o
drv_devdrv-objs += ts_drv_common/tsdrv_osal/tsdrv_osal_mm.o
drv_devdrv-objs += ts_drv_common/tsdrv_osal/tsdrv_osal_bbox.o
