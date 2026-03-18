ifeq ($(TOP_DIR),) # for ctrlcpu open
	include $(FEATURE_MK_PATH)
	EXTRA_CFLAGS += $(CONFIG_DEFINES)
	EXTRA_CFLAGS += -DCFG_SURPPORT_PCIE_HOST_DEVICE_COMM

	ifeq ($(TARGET_PRODUCT),mini)
		EXTRA_CFLAGS += -DCFG_FEATURE_EP_MODE
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINI
		ifeq ($(TARGET_BUILD_TYPE),debug)
			EXTRA_CFLAGS += -DCFG_TEE_EFUSE_LDO_DBG
		endif
		ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_DC_V10))
			EXTRA_CFLAGS += -DCFG_FEATURE_VFIO
            EXTRA_CFLAGS += -DCFG_FEATURE_PSS_SIGN
		endif
	endif

	ifeq ($(TARGET_PRODUCT),cloud)
		EXTRA_CFLAGS += -DCFG_FEATURE_EP_MODE
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD
		EXTRA_CFLAGS += -DCFG_FEATURE_VFIO
		EXTRA_CFLAGS += -DCFG_FEATURE_NOTIFY_REBOOT
		ifeq ($(TARGET_BUILD_TYPE),debug)
			EXTRA_CFLAGS += -DCFG_TEE_EFUSE_LDO_DBG
		endif
		ifeq ($(TARGET_CHIP_ID), $(A9XXGEN02M01))
			EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD_V2
			EXTRA_CFLAGS += -DCFG_FEATURE_CHIP_DIE
		else
			EXTRA_CFLAGS += -DCFG_FEATURE_VFIO
            EXTRA_CFLAGS += -DCFG_FEATURE_PSS_SIGN
		endif
	endif

	ifeq ($(TARGET_PRODUCT),mdc)
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINI
		ifeq ($(TARGET_BUILD_TYPE),debug)
			EXTRA_CFLAGS += -DCFG_TEE_EFUSE_LDO_DBG
		endif
	endif

	ifeq ($(TARGET_BOARD_PLATFORM),kirin990)
		ifeq ($(chip_type),es)
			CHIP_VERSION=hi3690_es
			EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_KIRIN990_ES
		else
			CHIP_VERSION=hi3690
			EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_KIRIN990
		endif
	endif
	EXTRA_CFLAGS += -DCFG_FEATURE_IPC_NOTIFY

	ifeq ($(TARGET_BUILD_TYPE),debug)
		EXTRA_CFLAGS += -DCFG_BUILD_DEBUG
	endif

	ifeq ($(TARGET_PRODUCT),cloud)
		ccflags-y += -Wall
	endif

	KERNEL_DIR := $(srctree)
	ifneq ($(filter $(PRODUCT),  ascend610 ascend610Liteesl ascend610Lite bs9sx1a),)
		TS_SOC_DIR := $(PWD)/drivers/dev_platform/tsdrv/mdc/ts_drv/ts_drv_device/soc
	else
		TS_SOC_DIR := $(PWD)/drivers/dev_platform/tsdrv/ts_drv/ts_drv_device/soc
	endif
	EXTRA_CFLAGS += -I$(PWD)/drivers/soft_fault
	EXTRA_CFLAGS += -I$(PWD)/drivers/ai_sdk/arc/linux/kernel_space/src/dms/time/device
	ifeq ($(TARGET_PRODUCT),cloud)
		EXTRA_CFLAGS += -I$(PWD)/inc/driver
		EXTRA_CFLAGS += -I$(PWD)/inc/toolchain/bbox
		EXTRA_CFLAGS += -I$(PWD)/inc/aicpu/common
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_inc
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_plat/common
		EXTRA_CFLAGS += -I$(PWD)/drivers/npu_inc
		EXTRA_CFLAGS += -I$(TS_SOC_DIR)/ascend910/cloud
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/tsdrv/ts_drv/ts_drv_device
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/tsdrv/ts_drv/ts_drv_common
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_core/device_manager/device_manager_inc
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_plat/nor_flash
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_plat/faultmng/inc
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/drv_devmng/drv_devmng_device/ascend910
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/drv_devmng/drv_devmng_device/ascend910/cloud
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/tsdrv/ts_platform/ts_platform_device
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_plat/cpld
	else ifeq ($(TARGET_PRODUCT),mini)
		EXTRA_CFLAGS += -I$(PWD)/inc/driver
		EXTRA_CFLAGS += -I$(PWD)/inc/aicpu/common
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_inc
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_plat/common
		EXTRA_CFLAGS += -I$(PWD)/drivers/npu_inc
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_plat/nor_flash
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_core/device_manager/device_manager_inc
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/tsdrv/ts_platform/ts_platform_device
		ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_DC_V10))
			EXTRA_CFLAGS += -I$(TS_SOC_DIR)/ascend910/miniv2
			EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/tsdrv/ts_drv/ts_drv_device
			EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/tsdrv/ts_drv/ts_drv_common
			EXTRA_CFLAGS += -I$(PWD)/inc/toolchain/bbox
			EXTRA_CFLAGS += -I$(PWD)/drivers/dev_plat/faultmng/inc
			EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/drv_devmng/drv_devmng_device/ascend910
			EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/drv_devmng/drv_devmng_device/ascend910/miniv2
		else
			EXTRA_CFLAGS += -I$(PWD)/inc/soc/mini_v1
			EXTRA_CFLAGS += -I$(PWD)/inc/toolchain/bbox/device
			EXTRA_CFLAGS += -I$(PWD)/config/user_config/mini
			EXTRA_CFLAGS += -I$(TS_SOC_DIR)/ascend310/mini
			EXTRA_CFLAGS += -I$(TS_SOC_DIR)/ascend310
			EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/tsdrv/ts_drv/ts_drv_device/ascend310
			EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/tsdrv/ts_drv/ts_drv_common
			EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/drv_devmng/drv_devmng_device/ascend310
			EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/drv_devmng/drv_devmng_device/ascend310/mini
			EXTRA_CFLAGS += -I$(PWD)/drivers/dev_core/device_manager/device_manager_inc
			EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/tsdrv/ts_platform/ts_platform_device
		endif
	else ifeq ($(TARGET_PRODUCT),mdc)
		EXTRA_CFLAGS += -I$(PWD)/inc/driver
		EXTRA_CFLAGS += -I$(PWD)/inc/toolchain/bbox
		EXTRA_CFLAGS += -I$(PWD)/inc/aicpu/common
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_inc
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_plat/common
		EXTRA_CFLAGS += -I$(PWD)/drivers/npu_inc
		EXTRA_CFLAGS += -I$(TS_SOC_DIR)/ascend910/miniv2
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/tsdrv/mdc/ts_drv/ts_drv_device
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/tsdrv/mdc/ts_drv/ts_drv_common
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_plat/nor_flash
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_plat/faultmng/inc
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_plat/faultmng/msg
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/drv_devmng/drv_devmng_device/ascend910
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/drv_devmng/drv_devmng_device/ascend910/mdc
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_core/device_manager/device_manager_inc
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/tsdrv/mdc/ts_platform/ts_platform_device
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/drv_notify
	else
		EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/drivers/inc/comm
		EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/driver
		EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/drivers/inc
		EXTRA_CFLAGS += -I$(TS_SOC_DIR)/ascend910/mini
		EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/drivers/dev_platform/tsdrv/ts_drv/ts_drv_device
		EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/drivers/dev_platform/tsdrv/ts_drv/ts_drv_common
		EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/drivers/dev_core/device_manager/device_manager_inc
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/drv_devmng/drv_devmng_device
		EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/drivers/dev_platform/tsdrv/ts_platform/ts_platform_device
	endif

	ifeq ($(TARGET_BUILD_TYPE),debug)
		EXTRA_CFLAGS += -I$(PWD)/drivers/unreleased/debug
	endif

	ifeq ($(CONFIG_PLATFORM_HI6280),y)
		EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/drivers/dev_platform/tsdrv/ts_platform/ts_platform_device
	else ifeq ($(CONFIG_PLATFORM_HI3690),y)
		EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/drivers/dev_platform/tsdrv/ts_platform/ts_platform_device
	endif

	ifeq ($(TARGET_PRODUCT),cloud)
		EXTRA_CFLAGS += -I$(PWD)/inc/soc/cloud_v1
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/tsdrv/ts_platform/ts_platform_device/ascend910
		EXTRA_CFLAGS += -I$(PWD)/config/user_config/cloud
	else ifeq ($(TARGET_PRODUCT),mini)
		ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_DC_V10))
			EXTRA_CFLAGS += -DCFG_FEATURE_EP_MODE
			EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
			EXTRA_CFLAGS += -DCFG_FEATURE_NOTIFY_REBOOT
			EXTRA_CFLAGS += -I$(PWD)/inc/soc/hi1910p
			EXTRA_CFLAGS += -I$(PWD)/inc/hiss/
			EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/tsdrv/ts_platform/ts_platform_device/ascend610
			EXTRA_CFLAGS += -I$(PWD)/config/user_config/miniv2
		else
			EXTRA_CFLAGS += -I$(PWD)/inc/soc/mini_v1
			EXTRA_CFLAGS += -I$(PWD)/inc/soc/mini
			EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/tsdrv/ts_platform/ts_platform_device/ascend310
		endif
	else ifeq ($(TARGET_PRODUCT),mdc)
		ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_MDC_V10))
			EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
			EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
			EXTRA_CFLAGS += -DCFG_FEATURE_BOARD_ID_EXPAND
			EXTRA_CFLAGS += -I$(PWD)/inc/soc/hi1910p
			EXTRA_CFLAGS += -I$(PWD)/inc/hiss/
			EXTRA_CFLAGS += -I$(PWD)/inc/media/camera/internal/
			EXTRA_CFLAGS += -I$(PWD)/inc/dvpp
			EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/tsdrv/mdc/ts_platform/ts_platform_device/ascend610
			EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/drv_devmng/drv_devmng_device/ascend910/mdc
			EXTRA_CFLAGS += -I$(PWD)/drivers/dev_plat/mttcan
			EXTRA_CFLAGS += -I$(PWD)/drivers/dev_plat/ufs
			EXTRA_CFLAGS += -I$(PWD)/drivers/dev_plat/sensorhub
			EXTRA_CFLAGS += -I$(PWD)/config/user_config/miniv2
		else ifeq ($(TARGET_CHIP_ID), $(ASCEND_MDC_BS9SX1A))
			EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
			EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
			EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_BS9SX1A
			EXTRA_CFLAGS += -DCFG_FEATURE_BOARD_ID_EXPAND
			EXTRA_CFLAGS += -I$(PWD)/inc/soc/hi1910p
			EXTRA_CFLAGS += -I$(PWD)/inc/hiss/
			EXTRA_CFLAGS += -I$(PWD)/inc/media/camera/internal/
			EXTRA_CFLAGS += -I$(PWD)/inc/dvpp
			EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/tsdrv/mdc/ts_platform/ts_platform_device/ascend610
			EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/drv_devmng/drv_devmng_device/ascend910/mdc
			EXTRA_CFLAGS += -I$(PWD)/drivers/dev_plat/mttcan
			EXTRA_CFLAGS += -I$(PWD)/drivers/dev_plat/ufs
			EXTRA_CFLAGS += -I$(PWD)/drivers/dev_plat/sensorhub
			EXTRA_CFLAGS += -I$(PWD)/config/user_config/miniv2
		else
			EXTRA_CFLAGS += -I$(PWD)/inc/soc/hi1910
			EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/tsdrv/ts_platform/ts_platform_device/ascend310
			EXTRA_CFLAGS += -I$(PWD)/config/user_config/mini
		endif
	else
		EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/soc/hi1910
		EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/drivers/dev_platform/tsdrv/ts_platform/ts_platform_device/ascend310
		EXTRA_CFLAGS += -I$(PWD)/config/user_config/mini
	endif


	ifeq ($(TARGET_PRODUCT),cloud)
		BUILD_RPEFIX_DIR := drv_devmng/drv_devmng_device/ascend910
	else ifeq ($(TARGET_PRODUCT),mini)
		ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_DC_V10))
			BUILD_RPEFIX_DIR := drv_devmng/drv_devmng_device/ascend910
		else
			BUILD_RPEFIX_DIR := drv_devmng/drv_devmng_device/ascend310
		endif
	else ifeq ($(TARGET_PRODUCT),mdc)
		BUILD_RPEFIX_DIR := drv_devmng/drv_devmng_device/ascend910
	else
		obj-$(CONFIG_NPU_DEVDRV_DEVMANAGER) += drv_devmng.o
	endif

	drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_manager.o
	drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_cache_flush.o
	drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_manager_msg.o
	drv_devmng-y += $(BUILD_RPEFIX_DIR)/tsdrv_status.o

	ifeq ($(TARGET_BUILD_TYPE),debug)
		ifeq ($(TARGET_PRODUCT),mini)
			ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_DC_V10))
				drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_dsmi/devdrv_info_debug.o
			else
				drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_info_debug.o
			endif
		else
			drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_dsmi/devdrv_info_debug.o
		endif
	endif
	drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_pm.o
	drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_ipc_msg.o
	drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_ipc.o
	drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_driver_pm.o
	drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_gpioirq_register.o
	drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_black_box.o
	drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_manager_container.o
	drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_manager_time.o
	drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_manager_freq.o
	drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_i2c_ina2xx.o

	ifeq ($(TARGET_PRODUCT),mini)
		ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_DC_V10))
			drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_pcie.o
			drv_devmng-y += $(BUILD_RPEFIX_DIR)/appmon_black_box.o
			drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_dsmi/devdrv_info.o
		else
			drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_platform_drvops.o
			drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_info.o
			drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_manager_container_noplugin.o
			drv_devmng-y += $(BUILD_RPEFIX_DIR)/appmon_black_box.o
			drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_flash_config.o
			drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_manager_dev_share.o
		endif
	else
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_pcie.o
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/appmon_black_box.o
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_dsmi/devdrv_info.o
	endif

	ifeq ($(TARGET_PRODUCT),cloud)
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/cloud/devdrv_adapt_manager.o
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_dsmi/devdrv_manager_hccs.o
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_dsmi/devdrv_manager_llc.o
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_flow_control_mpam.o
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_manager_pid_map.o
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_chip_dev_map.o
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_manager_rand.o
	else ifeq ($(TARGET_PRODUCT),mini)
		ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_DC_V10))
			drv_devmng-y += $(BUILD_RPEFIX_DIR)/miniv2/devdrv_adapt_manager.o
			drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_dsmi/devdrv_manager_llc.o
			drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_manager_pid_map.o
		else
			drv_devmng-y += $(BUILD_RPEFIX_DIR)/mini/devdrv_adapt_manager.o
		endif
	else ifeq ($(TARGET_PRODUCT),mdc)
		ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_MDC_V10))
			drv_devmng-y += $(BUILD_RPEFIX_DIR)/miniv2/devdrv_adapt_manager.o
			drv_devmng-y += $(BUILD_RPEFIX_DIR)/mdc/devmng_dts.o
			drv_devmng-y += $(BUILD_RPEFIX_DIR)/mdc/devdrv_os_power.o
			drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_dsmi/devdrv_manager_llc.o
			drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_manager_pid_map.o
			drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_chip_dev_map.o
			drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_manager_rand.o
		else ifeq ($(TARGET_CHIP_ID), $(ASCEND_MDC_BS9SX1A))
			drv_devmng-y += $(BUILD_RPEFIX_DIR)/miniv2/devdrv_adapt_manager.o
			drv_devmng-y += $(BUILD_RPEFIX_DIR)/mdc/devmng_dts.o
			drv_devmng-y += $(BUILD_RPEFIX_DIR)/mdc/devdrv_os_power.o
			drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_dsmi/devdrv_manager_llc.o
			drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_manager_pid_map.o
			drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_chip_dev_map.o
			drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_manager_rand.o
		else
			drv_devmng-y += $(BUILD_RPEFIX_DIR)/mini/devdrv_adapt_manager.o
		endif
	else ifeq ($(CONFIG_PLATFORM_HI3690),y)
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/$(CHIP_VERSION)/devdrv_adapt_manager.o
	else ifeq ($(CONFIG_PLATFORM_HI6280),y)
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/hi6280/devdrv_adapt_manager.o
	else
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/mini/devdrv_adapt_manager.o
	endif

else # for cmake
	include $(FEATURE_MK_PATH)
	EXTRA_CFLAGS += $(CONFIG_DEFINES)
	ifeq ($(filter $(PRODUCT), ascend610 bs9sx1a ascend610Lite ascend610Liteesl as31xm1 ascend310Brc helper310p),)
		EXTRA_CFLAGS += -DCFG_SURPPORT_PCIE_HOST_DEVICE_COMM
	endif
	ifeq ($(PRODUCT),as31xm1)
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V11
	endif
	ifneq ($(filter $(PRODUCT),  ascend910 ascend910B ascend920esl ascend920emu),)
		EXTRA_CFLAGS += -DCFG_FEATURE_IMU_ENABLE
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD
		EXTRA_CFLAGS += -DCFG_FEATURE_VFIO
		ifneq ($(filter $(PRODUCT),  ascend910B ascend920esl ascend920emu),)
			EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD_V2
			EXTRA_CFLAGS += -DCFG_FEATURE_IPC_FORMAT_V2_CRC
			EXTRA_CFLAGS += -DCFG_FEATURE_CHIP_DIE
			EXTRA_CFLAGS += -DCFG_FEATURE_HISS
			EXTRA_CFLAGS += -DCFG_FEATURE_HW_INFO_FROM_BIOS
			EXTRA_CFLAGS += -DCFG_FEATURE_SRIOV
			EXTRA_CFLAGS += -DCFG_TRS_REFACTOR_FEATURE
			EXTRA_CFLAGS += -DCFG_FEATURE_CHIP_OFFSET_OUTSIDE_2G
			EXTRA_CFLAGS += -DCFG_FEATURE_VF_USE_DEVID
		else
			EXTRA_CFLAGS += -DCFG_FEATURE_PSS_SIGN
			EXTRA_CFLAGS += -DCFG_FEATURE_DDR
		endif
		ifeq ($(TARGET_BUILD_TYPE),debug)
			EXTRA_CFLAGS += -DCFG_TEE_EFUSE_LDO_DBG
		endif
		ccflags-y += -Wall
	endif

	ifneq ($(filter $(PRODUCT), ascend310 ascend310rc ascend310B ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu ascend610 ascend610Lite ascend610Liteesl bs9sx1a ascend310p helper310p as31xm1),)
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINI
		EXTRA_CFLAGS += -DCFG_FEATURE_DDR
		ifeq ($(TARGET_BUILD_TYPE),debug)
			EXTRA_CFLAGS += -DCFG_TEE_EFUSE_LDO_DBG
		endif
		ifeq ($(PRODUCT),ascend310p)
			EXTRA_CFLAGS += -DCFG_FEATURE_VFIO
			EXTRA_CFLAGS += -DCFG_FEATURE_HISS
			EXTRA_CFLAGS += -DCFG_FEATURE_PSS_SIGN
			EXTRA_CFLAGS += -DCFG_FEATURE_NOTIFY_REBOOT
		endif
		ifeq ($(PRODUCT),helper310p)
			EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_HELPER
			EXTRA_CFLAGS += -DCFG_FEATURE_CPU_NUMS_FIXED
			EXTRA_CFLAGS += -DCFG_FEATURE_HISS
			EXTRA_CFLAGS += -DCFG_FEATURE_BIND_TGID
		endif
		ifneq ($(filter $(PRODUCT), ascend310B ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu as31xm1),)
			EXTRA_CFLAGS += -DCFG_FEATURE_IMU_ENABLE
			EXTRA_CFLAGS += -DCFG_FEATURE_IPC_FORMAT_V2_CRC
			EXTRA_CFLAGS += -DCFG_FEATURE_CPU_NUMS_FIXED
			EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV3
			EXTRA_CFLAGS += -DCFG_FEATURE_HISS
			EXTRA_CFLAGS += -DCFG_TRS_REFACTOR_FEATURE
			EXTRA_CFLAGS += -DCFG_FEATURE_LLC_STUB
			ifneq ($(filter $(PRODUCT), ascend310B ascend310Besl ascend310Bemu),)
				EXTRA_CFLAGS += -DCFG_FEATURE_DEVMNG_BAR
			endif
			ifeq ($(PRODUCT),as31xm1)
				EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V11
			endif
		endif
	endif

	ifneq ($(filter $(PRODUCT), ascend610 bs9sx1a ascend310p helper310p ascend610Lite ascend610Liteesl),)
		EXTRA_CFLAGS += -DCFG_FEATURE_LP_ENABLE
	endif

	ifneq ($(filter $(PRODUCT), ascend610 bs9sx1a ascend610Lite ascend610Liteesl as31xm1),)
		EXTRA_CFLAGS += -DCFG_FEATURE_REBOOT_REASON
	endif

	ifneq ($(filter $(PRODUCT), ascend610 bs9sx1a ascend610Lite ascend610Liteesl ascend310Brc as31xm1),)
		EXTRA_CFLAGS += -DCFG_FEATURE_HOST_UNBIND
	endif

	ifeq ($(PRODUCT),ascend610Lite)
		EXTRA_CFLAGS += -DCFG_SOC_MDC_V51_LITE
	endif
	ifeq ($(PRODUCT),ascend610Liteesl)
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_LITE_ESL
		EXTRA_CFLAGS += -DCFG_SOC_MDC_V51_LITE
    endif

    ifeq ($(PRODUCT),bs9sx1a)
        EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_BS9SX1A
    endif

    ifneq ($(filter $(PRODUCT), ascend310 ascend310p ascend910 ascend910B ascend920esl ascend920emu ascend310B ascend310Besl ascend310Bemu),)
        EXTRA_CFLAGS += -DCFG_FEATURE_EP_MODE
    endif
    ifneq ($(filter $(PRODUCT), helper310p ascend310Brc ascend310Brcesl ascend310Brcemu as31xm1 ascend610 bs9sx1a ascend610Liteesl ascend610Lite),)
		EXTRA_CFLAGS += -DCFG_FEATURE_RC_MODE
		ifneq ($(filter $(PRODUCT), ascend310Brc ascend310Brcesl),)
			EXTRA_CFLAGS += -DCFG_FEATURE_VFIO
			EXTRA_CFLAGS += -DCFG_FEATURE_DEVICE_DEVID_CONVERT
			EXTRA_CFLAGS += -DCFG_FEATURE_VF_SINGLE_AICORE
			EXTRA_CFLAGS += -DCFG_FEATURE_VF_USE_DEVID
			EXTRA_CFLAGS += -DCFG_FEATURE_VFIO_SOC
			EXTRA_CFLAGS += -DCFG_FEATURE_BIND_TGID
			EXTRA_CFLAGS += -DCFG_FEATURE_DEVICE_QUERY_RESOURCE
		endif
    endif

	EXTRA_CFLAGS += -DCFG_FEATURE_IPC_NOTIFY

	ifeq ($(TARGET_BUILD_TYPE),debug)
		EXTRA_CFLAGS += -DCFG_BUILD_DEBUG
	endif

	ifeq ($(TARGET_BOARD_PLATFORM),kirin990)
		ifeq ($(chip_type),es)
			CHIP_VERSION=hi3690_es
			EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_KIRIN990_ES
		else
			CHIP_VERSION=hi3690
			EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_KIRIN990
		endif
	endif

	ifneq ($(filter $(PRODUCT),  ascend610 ascend610Liteesl ascend610Lite bs9sx1a),)
		TS_SOC_DIR := $(DRIVER_KERNEL_DIR)/src/tsdrv/mdc/ts_drv/ts_drv_device/soc
	else
		TS_SOC_DIR := $(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_device/soc
	endif

	EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/time
	EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/include
	EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/soft_fault
	ifneq ($(filter $(PRODUCT),  ascend910 ascend910B ascend920esl ascend920emu),)
		EXTRA_CFLAGS += -I$(TOP_DIR)/inc
		EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver
		EXTRA_CFLAGS += -I$(TOP_DIR)/abl/bbox/inc/bbox
		EXTRA_CFLAGS += -I$(TOP_DIR)/inc/toolchain/bbox
		EXTRA_CFLAGS += -I$(TOP_DIR)/inc/aicpu/common
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/common
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_inc
		EXTRA_CFLAGS += -I$(TS_SOC_DIR)/ascend910/cloud
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_device
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_common
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_inc/
		EXTRA_CFLAGS += -I$(DRIVER_CHIP_KERNEL_DIR)/src/nor_flash
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/faultmng/inc
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device/ascend910
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device/ascend910/cloud
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/common
		EXTRA_CFLAGS += -I$(DRIVER_CHIP_KERNEL_DIR)/src/cpld
	else ifneq ($(filter $(PRODUCT), ascend310 ascend310rc ascend310B ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu ascend310p helper310p as31xm1),)
		EXTRA_CFLAGS += -I$(TOP_DIR)/inc
		EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver
		EXTRA_CFLAGS += -I$(TOP_DIR)/abl/bbox/inc/bbox/device
		EXTRA_CFLAGS += -I$(TOP_DIR)/inc/toolchain/bbox/device
		EXTRA_CFLAGS += -I$(TOP_DIR)/inc/aicpu/common
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/common
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_inc
		EXTRA_CFLAGS += -I$(DRIVER_CHIP_KERNEL_DIR)/src/nor_flash
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/faultmng/inc
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_inc/
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device
		ifneq ($(filter $(PRODUCT), ascend310B ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu ascend310p helper310p as31xm1),)
			EXTRA_CFLAGS += -I$(TS_SOC_DIR)/ascend910/miniv2
			EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_device
			EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_common
			EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/usr_cfg_drv
			EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device/ascend910
			EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device/ascend910/mini
		else
			EXTRA_CFLAGS += -I$(TS_SOC_DIR)/ascend310/mini
			EXTRA_CFLAGS += -I$(TS_SOC_DIR)/ascend310
			EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_device/ascend310
			EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_drv/ts_drv_common
			EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device/ascend310
			EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device/ascend310/mini
			EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device
		endif
	else ifneq ($(filter $(PRODUCT), ascend610 bs9sx1a ascend610Lite ascend610Liteesl),)
		EXTRA_CFLAGS += -I$(TOP_DIR)/inc
		EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver
		EXTRA_CFLAGS += -I$(TOP_DIR)/inc/aicpu/common
		EXTRA_CFLAGS += -I$(TOP_DIR)/abl/bbox/inc/bbox
		EXTRA_CFLAGS += -I$(TOP_DIR)/inc/toolchain/bbox
		EXTRA_CFLAGS += -I$(TOP_DIR)/inc/aicpu/common
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/common
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_inc

		EXTRA_CFLAGS += -I$(TS_SOC_DIR)/ascend910/miniv2
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/usr_cfg_drv
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/mdc/ts_drv/ts_drv_device
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/mdc/ts_drv/ts_drv_common
		EXTRA_CFLAGS += -I$(DRIVER_CHIP_KERNEL_DIR)/src/nor_flash
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/faultmng/inc
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/faultmng/msg

		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_inc/
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device/ascend910
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device/ascend910/mdc
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/mdc/ts_platform/ts_platform_device
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_notify

	else
		EXTRA_CFLAGS += -I$(TOP_DIR)/inc
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/hisi/npu/drivers/inc/comm
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/hisi/npu/inc/driver
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/hisi/npu/drivers/inc
		EXTRA_CFLAGS += -I$(TS_SOC_DIR)/ascend910/mini
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/hisi/npu/drivers/dev_platform/tsdrv/ts_drv/ts_drv_device
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/hisi/npu/drivers/dev_platform/tsdrv/ts_drv/ts_drv_common
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/hisi/npu/drivers/dev_core/device_manager/device_manager_inc
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/hisi/npu/drivers/dev_core/device_manager/device_manager_slave/devdrv_manager_slave
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/hisi/npu/drivers/dev_platform/tsdrv/ts_platform/ts_platform_device
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device
	endif

	ifeq ($(TARGET_BUILD_TYPE),debug)
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/unreleased/debug
	endif

	ifneq ($(filter $(PRODUCT),  ascend910 ascend910B ascend920esl ascend920emu),)
		EXTRA_CFLAGS += -I$(TOP_DIR)/inc/soc/cloud_v1
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/ascend910
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/config/user_config/cloud
		EXTRA_CFLAGS += -I$(TOP_DIR)/config/user_config/cloud
	else ifneq ($(filter $(PRODUCT), ascend310 ascend310rc ascend310B ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu ascend310p helper310p as31xm1),)
		ifneq ($(filter $(PRODUCT),  ascend310B ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu ascend310p helper310p as31xm1),)
			EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
			EXTRA_CFLAGS += -DCFG_SOC_FEATURE_MULTI_TS
			EXTRA_CFLAGS += -I$(TOP_DIR)/inc
			EXTRA_CFLAGS += -I$(TOP_DIR)/inc/soc/hi1910p
			EXTRA_CFLAGS += -I$(TOP_DIR)/abl/bbox/inc/bbox
			EXTRA_CFLAGS += -I$(TOP_DIR)/inc/toolchain/bbox
			EXTRA_CFLAGS += -I$(TOP_DIR)/inc/hiss/
			EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/firmware/hiss/inc/hiss/
			EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/ascend610
			EXTRA_CFLAGS += -I$(TOP_DIR)/inc/dvpp
			EXTRA_CFLAGS += -I$(TOP_DIR)/ace/dvpp/inc/dvpp
			EXTRA_CFLAGS += -I$(TOP_DIR)/dvpp/include
			ifneq ($(filter $(PRODUCT), ascend310B ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu as31xm1),)
				EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/config/user_config/miniv3
				ifneq ($(filter $(PRODUCT), ascend310Brc ascend310Brcesl ascend310Brcemu as31xm1),)
					EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_common
				endif
			else
				EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/config/user_config/miniv2
				EXTRA_CFLAGS += -I$(TOP_DIR)/config/user_config/miniv2
			endif
			ifneq ($(filter $(PRODUCT), ascend310p),)
				EXTRA_CFLAGS += -DCFG_FEATURE_CAPABILITY_GROUP
			endif
		else
			EXTRA_CFLAGS += -I$(TOP_DIR)/inc
			EXTRA_CFLAGS += -I$(TOP_DIR)/inc/soc/mini_v1
			EXTRA_CFLAGS += -I$(TOP_DIR)/abl/bbox/inc/bbox
			EXTRA_CFLAGS += -I$(TOP_DIR)/inc/toolchain/bbox
			EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/config/user_config/mini
			EXTRA_CFLAGS += -I$(TOP_DIR)/config/user_config/mini
			EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/ts_platform/ts_platform_device/ascend310
		endif
	else ifneq ($(filter $(PRODUCT), ascend610 bs9sx1a ascend610Lite ascend610Liteesl),)
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
		EXTRA_CFLAGS += -DCFG_FEATURE_CAPABILITY_GROUP
		EXTRA_CFLAGS += -DCFG_FEATURE_BIN_DOWN
		EXTRA_CFLAGS += -DCFG_FEATURE_HISS
		EXTRA_CFLAGS += -DCFG_FEATURE_IPC_CRC
		EXTRA_CFLAGS += -DCFG_FEATURE_BOARD_ID_EXPAND
		EXTRA_CFLAGS += -I$(TOP_DIR)/inc
		EXTRA_CFLAGS += -I$(TOP_DIR)/inc/soc/hi1910p
		EXTRA_CFLAGS += -I$(TOP_DIR)/inc/hiss/
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/firmware/hiss/inc/hiss/
		EXTRA_CFLAGS += -I$(TOP_DIR)/inc/media/camera/internal/
		EXTRA_CFLAGS += -I$(TOP_DIR)/ace/media/inc/media/camera/internal/
		EXTRA_CFLAGS += -I$(TOP_DIR)/inc/dvpp
		EXTRA_CFLAGS += -I$(TOP_DIR)/ace/dvpp/inc/dvpp
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/tsdrv/mdc/ts_platform/ts_platform_device/ascend610
		EXTRA_CFLAGS += -I$(DRIVER_CHIP_KERNEL_DIR)/src/mttcan
		EXTRA_CFLAGS += -I$(DRIVER_CHIP_KERNEL_DIR)/src/ufs
		EXTRA_CFLAGS += -I$(DRIVER_CHIP_KERNEL_DIR)/src/sensorhub
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/config/user_config/miniv2
		EXTRA_CFLAGS += -I$(TOP_DIR)/config/user_config/miniv2
	else ifeq ($(CONFIG_PLATFORM_HI6280),y)
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/hisi/npu/drivers/dev_platform/tsdrv/ts_platform/ts_platform_device
	else ifeq ($(CONFIG_PLATFORM_HI3690),y)
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/hisi/npu/drivers/dev_platform/tsdrv/ts_platform/ts_platform_device
	else
		EXTRA_CFLAGS += -I$(TOP_DIR)/inc
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/hisi/npu/inc/soc/hi1910
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/hisi/npu/drivers/dev_platform/tsdrv/ts_platform/ts_platform_device/ascend310
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/config/user_config/mini
		EXTRA_CFLAGS += -I$(TOP_DIR)/config/user_config/mini
	endif

	ifneq ($(filter $(PRODUCT),  ascend310 ascend310rc),)
		BUILD_RPEFIX_DIR := drv_devmng/drv_devmng_device/ascend310
	else ifneq ($(filter $(PRODUCT),  ascend310B ascend310Besl ascend310Bemu ascend310Brc as31xm1 ascend310Brcesl ascend310Brcemu ascend610 bs9sx1a ascend610Lite ascend610Liteesl ascend310p helper310p ascend910 ascend910B ascend920esl ascend920emu),)
		BUILD_RPEFIX_DIR := drv_devmng/drv_devmng_device/ascend910
	else
		obj-$(CONFIG_NPU_DEVDRV_DEVMANAGER) += drv_devmng.o
	endif

	EXTRA_CFLAGS += -I$(TS_SOC_DIR)/$(BUILD_RPEFIX_DIR)

	drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_manager.o
	drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_cache_flush.o
	drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_manager_msg.o
	drv_devmng-y += $(BUILD_RPEFIX_DIR)/tsdrv_status.o

	ifeq ($(TARGET_BUILD_TYPE),debug)
		ifneq ($(filter $(PRODUCT),  ascend310 ascend310rc),)
			drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_info_debug.o
		else ifneq ($(filter $(PRODUCT), ascend610 ascend610Lite ascend610Liteesl bs9sx1a ascend310p helper310p ascend910 ascend910B ascend920esl ascend920emu),)
			drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_dsmi/devdrv_info_debug.o
		endif
	endif

	drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_pm.o $(BUILD_RPEFIX_DIR)/devdrv_ipc_msg.o $(BUILD_RPEFIX_DIR)/devdrv_ipc.o
	drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_driver_pm.o $(BUILD_RPEFIX_DIR)/devdrv_gpioirq_register.o
	drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_black_box.o
	drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_manager_container.o
	drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_manager_time.o
	drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_manager_freq.o
	drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_i2c_ina2xx.o


	ifneq ($(filter $(PRODUCT),  ascend310 ascend310rc),)
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_platform_drvops.o
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_info.o
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_manager_container_noplugin.o
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/appmon_black_box.o
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_flash_config.o
	else ifneq ($(filter $(PRODUCT),  ascend310B ascend310Besl ascend310Bemu ascend310Brc as31xm1 ascend310Brcesl ascend310Brcemu ascend610 ascend610Lite ascend610Liteesl bs9sx1a ascend310p helper310p ascend910 ascend910B ascend920esl ascend920emu),)
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_pcie.o
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/appmon_black_box.o
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_dsmi/devdrv_info.o
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_manager_rand.o
		ifneq ($(filter $(PRODUCT), ascend310Brc ascend310Brcesl ascend310Brcemu),)
			drv_devmng-y += drv_devmng/drv_devmng_common/dev_mnt_vdevice.o
			drv_devmng-y += drv_devmng/drv_devmng_common/devdrv_pcie.o
		endif
	endif

        ifneq ($(filter $(PRODUCT),  ascend310 ascend310rc helper310p ascend310B ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu),)
			EXTRA_CFLAGS += -DCFG_FEATURE_DEVICE_SHARE
			drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_manager_dev_share.o
        endif

	ifneq ($(filter $(PRODUCT),  ascend910 ascend910B ascend920esl ascend920emu),)
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/cloud/devdrv_adapt_manager.o
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_dsmi/devdrv_manager_hccs.o
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_dsmi/devdrv_manager_llc.o
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_flow_control_mpam.o
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_manager_pid_map.o
	else ifneq ($(filter $(PRODUCT),  ascend310 ascend310rc),)
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/mini/devdrv_adapt_manager.o
	else ifneq ($(filter $(PRODUCT), ascend310B ascend310Besl ascend310Bemu ascend310Brc as31xm1 ascend310Brcesl ascend310Brcemu),)
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/miniv2/devdrv_adapt_manager.o
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_dsmi/devdrv_manager_llc.o
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_manager_pid_map.o
	else ifneq ($(filter $(PRODUCT), ascend310p helper310p),)
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/miniv2/devdrv_adapt_manager.o
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_dsmi/devdrv_manager_llc.o
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_manager_pid_map.o
	else ifneq ($(filter $(PRODUCT),  ascend610 bs9sx1a ascend610Lite ascend610Liteesl),)
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/miniv2/devdrv_adapt_manager.o
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/mdc/devmng_dts.o
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/mdc/devdrv_os_power.o
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_dsmi/devdrv_manager_llc.o
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_manager_pid_map.o
	else ifeq ($(CONFIG_PLATFORM_HI3690),y)
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/$(CHIP_VERSION)/devdrv_adapt_manager.o
	else ifeq ($(CONFIG_PLATFORM_HI6280),y)
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/hi6280/devdrv_adapt_manager.o
	else
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/mini/devdrv_adapt_manager.o
	endif

	ifeq ($(PRODUCT),ascend910)
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_soc_misc/ascend910
		EXTRA_CFLAGS += -DCFG_FEATURE_NOTIFY_REBOOT
	else ifneq ($(filter $(PRODUCT), ascend310 ascend310rc),)
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_soc_misc/ascend310
	else ifneq ($(filter $(PRODUCT), ascend310p ascend610 bs9sx1a ascend610Lite ascend610Liteesl),)
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_soc_misc/ascend310p
	else ifneq ($(filter $(PRODUCT), helper310p),)
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_soc_misc/ascend310p
	else ifneq ($(filter $(PRODUCT), ascend310B ascend310Besl ascend310Bemu ascend310Brc as31xm1 ascend310Brcesl ascend310Brcemu),)
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_soc_misc/ascend310B
	else ifneq ($(filter $(PRODUCT), ascend910B ascend920esl ascend920emu),)
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_soc_misc/ascend910B
		EXTRA_CFLAGS += -I$(TOP_DIR)/inc/hiss/
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/firmware/hiss/inc/hiss/
		drv_devmng-y += $(BUILD_RPEFIX_DIR)/devdrv_chip_dev_map.o
	endif

EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_soc_misc/features
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/time/device

include $(FEATURE_MK_PATH)
EXTRA_CFLAGS += $(CONFIG_DEFINES)

endif
