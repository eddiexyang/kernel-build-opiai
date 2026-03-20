
ifeq ($(TOP_DIR),)
	ifeq ($(DAVINCI_HIAI_DKMS),y) #for DKMS
		include $(FEATURE_MK_PATH)
		EXTRA_CFLAGS += $(CONFIG_DEFINES)

		ifneq ($(NOT_SUPPORT_SP), y)
			EXTRA_CFLAGS += -fstack-protector-all
		endif

		ccflags-y += -Wall
		ifeq ($(Driver_Install_Mode),vnpu_guest)
			ifeq ($(TARGET_CHIP_ID), hi1980b)
				obj-m += drv_devmng_host.o
				include $(HIAI_DKMS_DIR)/dms/time/dms_time.mk
				include $(HIAI_DKMS_DIR)/dms/drv_devmng/drv_devmng_host/drv_devmng_host.mk
				include $(HIAI_DKMS_DIR)/dms/adapt/dms_adapt.mk
				include $(HIAI_DKMS_DIR)/dms/core/dms_core.mk
				include $(HIAI_DKMS_DIR)/dms/custom/dms_custom.mk
				include $(HIAI_DKMS_DIR)/dms/devid/dms_devid.mk
				include $(HIAI_DKMS_DIR)/dms/event/dms_event_adapt.mk
				include $(HIAI_DKMS_DIR)/dms/power/dms_power.mk
				#include $(HIAI_DKMS_DIR)/dms/smf/sensor/dms_sensor.mk
				include $(HIAI_DKMS_DIR)/dms/status/dms_status.mk
				include $(HIAI_DKMS_DIR)/dms/config/dms_config.mk
				include $(HIAI_DKMS_DIR)/dms/product/dms_product.mk
				include $(HIAI_DKMS_DIR)/dms/xpc/dms_xpc.mk
			else
				obj-m += drv_vdevmng.o
				include $(HIAI_DKMS_DIR)/dms/drv_devmng/drv_devmng_vm/drv_vdevmng.mk
			endif
		else
			obj-m += drv_devmng_host.o
			include $(HIAI_DKMS_DIR)/dms/time/dms_time.mk
			include $(HIAI_DKMS_DIR)/dms/drv_devmng/drv_devmng_host/drv_devmng_host.mk
			include $(HIAI_DKMS_DIR)/dms/adapt/dms_adapt.mk
			include $(HIAI_DKMS_DIR)/dms/core/dms_core.mk
			include $(HIAI_DKMS_DIR)/dms/custom/dms_custom.mk
			include $(HIAI_DKMS_DIR)/dms/devid/dms_devid.mk
			include $(HIAI_DKMS_DIR)/dms/event/dms_event_adapt.mk
			include $(HIAI_DKMS_DIR)/dms/power/dms_power.mk
			#include $(HIAI_DKMS_DIR)/dms/smf/sensor/dms_sensor.mk
			include $(HIAI_DKMS_DIR)/dms/status/dms_status.mk
			include $(HIAI_DKMS_DIR)/dms/config/dms_config.mk
			include $(HIAI_DKMS_DIR)/dms/product/dms_product.mk
			include $(HIAI_DKMS_DIR)/dms/xpc/dms_xpc.mk
		endif
		EXTRA_CFLAGS += -DCFG_HOST_ENV
		EXTRA_CFLAGS += -DCFG_FEATURE_HEART_BEAT
		EXTRA_CFLAGS += -DCFG_FEATURE_HEALTH_ERR_CODE
		EXTRA_CFLAGS += -DCFG_FEATURE_DEV_TOPOLOGY
		EXTRA_CFLAGS += -DCFG_SURPPORT_PCIE_HOST_DEVICE_COMM
		EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dev_inc/inc
		EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/libc_sec/include
		EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dms
		EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/../inc
		EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dms/include
		EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dms/smf/sensor
		EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dms/dtm
		EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dms/smf
		EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dms/smf/event
		EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dms/event
		EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dms/power
		EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/src/dms/xpc
		EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dbl/dev_urd
		EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/common
		EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dms/status
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/firmware/hiss/inc
		ifeq ($(TARGET_CHIP_ID), hi1980b)
			EXTRA_CFLAGS += -DCFG_FEATURE_PG
			EXTRA_CFLAGS += -DCFG_FEATURE_SRIOV
			EXTRA_CFLAGS += -DCFG_FEATURE_VFG
			EXTRA_CFLAGS += -DCFG_FEATURE_HOST_AICPU
			EXTRA_CFLAGS += -DCFG_FEATURE_NEW_EVENT_CODE
			EXTRA_CFLAGS += -DCFG_CHIP_INFO_FROM_DEVINFO
			EXTRA_CFLAGS += -DCFG_FEATURE_ERRORCODE_ON_NEW_CHIPS
			EXTRA_CFLAGS += -DCFG_TRS_HB_REFACTOR_FEATURE
			EXTRA_CFLAGS += -DCFG_FEATURE_TOPOLOGY_BY_HCCS_LINK_STATUS
		else
			EXTRA_CFLAGS += -DCFG_FEATURE_TOPOLOGY_BY_SMP
		endif

		ifeq ($(TARGET_CHIP_ID), hi1910b)
			EXTRA_CFLAGS += -DCFG_FEATURE_PG
			EXTRA_CFLAGS += -DCFG_FEATURE_ERRORCODE_ON_NEW_CHIPS
			EXTRA_CFLAGS += -DCFG_TRS_HB_REFACTOR_FEATURE
			EXTRA_CFLAGS += -DCFG_FEATURE_NEW_EVENT_CODE
		endif

		ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_DC_V10))
			EXTRA_CFLAGS += -DCFG_CHIP_INFO_FROM_VMNG
			EXTRA_CFLAGS += -DCFG_FEATURE_OSC_FREQ
			EXTRA_CFLAGS += -DCFG_FEATURE_OLD_ALL_DEVICE_RESET_FLAG
		endif

		ifeq ($(TARGET_CHIP_ID), hi1980)
			EXTRA_CFLAGS += -DCFG_CHIP_INFO_FROM_VMNG
			EXTRA_CFLAGS += -DCFG_FEATURE_OLD_ALL_DEVICE_RESET_FLAG
		endif

		ifneq ($(filter $(TARGET_CHIP_ID), hi1910b hi1980 hi1980b),)
			EXTRA_CFLAGS += -DCFG_FEATURE_OSC_FREQ
		endif

		ifeq ($(TARGET_CHIP_ID), hi1910)
			EXTRA_CFLAGS += -DCFG_FEATURE_OLD_ALL_DEVICE_RESET_FLAG
		endif
	endif
else #for CMake & ctrl cpu open
	include $(FEATURE_MK_PATH)
	EXTRA_CFLAGS += $(CONFIG_DEFINES)

	ifneq ($(NOT_SUPPORT_SP), y)
		EXTRA_CFLAGS += -fstack-protector-all
	endif

	ifeq ($(filter $(PRODUCT), ascend610 bs9sx1a ascend610Lite ascend610Liteesl as31xm1 ascend310Brc helper310p),)
		EXTRA_CFLAGS += -DCFG_SURPPORT_PCIE_HOST_DEVICE_COMM
	endif

	ifneq ($(filter $(PRODUCT), ascend610 bs9sx1a),)
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
		EXTRA_CFLAGS += -DCFG_FEATURE_IPC_CRC
		EXTRA_CFLAGS += -DCFG_FEATURE_PARTIAL_GOOD
		EXTRA_CFLAGS += -DDRV_SUPPORT_SYSCOUNT
	endif
	ifneq ($(filter $(PRODUCT), ascend610Lite ascend610Liteesl),)
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
		EXTRA_CFLAGS += -DCFG_FEATURE_IPC_CRC
		EXTRA_CFLAGS += -DCFG_FEATURE_PARTIAL_GOOD
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_LITE_ESL
		EXTRA_CFLAGS += -DDRV_SUPPORT_SYSCOUNT
	endif
	ifneq ($(filter $(PRODUCT), ascend610Lite ascend610Liteesl ascend610 bs9sx1a as31xm1),)
		EXTRA_CFLAGS += -DCFG_FEATURE_REBOOT_REASON
	endif
	ifneq ($(filter $(PRODUCT), as31xm1),)
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V11
	endif

	ccflags-y += -Wall -funsigned-char -Wextra -Werror -Wformat=2 -Wfloat-equal -Wcast-align -Wundef $(WDATE_TIME)
	ccflags-y += -Wno-unused-parameter -Wno-sign-compare  -Wno-missing-field-initializers -Wno-format-nonliteral -Wno-empty-body

	ifeq ($(PRODUCT_SIDE), host)
		EXTRA_CFLAGS += -DCFG_HOST_ENV
		EXTRA_CFLAGS += -DCFG_FEATURE_DEV_TOPOLOGY
		obj-m += drv_devmng_host.o
		include ${DRIVER_KERNEL_DIR}/src/dms/drv_devmng/drv_devmng_host/drv_devmng_host.mk
		include ${DRIVER_KERNEL_DIR}/src/dms/power/dms_power.mk
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/power

		ifneq ($(filter $(PRODUCT), ascend910 ascend310p),)
			EXTRA_CFLAGS += -DCFG_CHIP_INFO_FROM_VMNG
			EXTRA_CFLAGS += -DCFG_FEATURE_OLD_ALL_DEVICE_RESET_FLAG
			obj-m += drv_vdevmng.o
			include ${DRIVER_KERNEL_DIR}/src/dms/drv_devmng/drv_devmng_vm/drv_vdevmng.mk
		endif
		ifneq ($(filter $(PRODUCT), ascend910B),)
			EXTRA_CFLAGS += -DCFG_FEATURE_HOST_AICPU
			EXTRA_CFLAGS += -DCFG_CHIP_INFO_FROM_DEVINFO
		endif
		ifneq ($(filter $(PRODUCT), ascend310p ascend310B ascend910 ascend910B),)
			EXTRA_CFLAGS += -DCFG_FEATURE_OSC_FREQ
		endif
		ifneq ($(filter $(PRODUCT), ascend310),)
			EXTRA_CFLAGS += -DCFG_FEATURE_OLD_ALL_DEVICE_RESET_FLAG
		endif
	else
		obj-m += drv_devmng.o
		include ${DRIVER_KERNEL_DIR}/src/dms/drv_devmng/drv_devmng_device/drv_devmng.mk
		include ${DRIVER_KERNEL_DIR}/src/dms/lpm/dms_lpm.mk
		include ${DRIVER_KERNEL_DIR}/src/dms/ipc/dms_ipc.mk
		include ${DRIVER_KERNEL_DIR}/src/dms/ts/dms_ts.mk
		include ${DRIVER_KERNEL_DIR}/src/dms/mem/dms_mem_info.mk
		ifneq ($(filter $(PRODUCT), ascend310p ascend910 ascend910B),)
			EXTRA_CFLAGS += -DSUPPORT_VDEV_MEM
		endif
		ifneq ($(filter $(PRODUCT), ascend310p helper310p ascend610 bs9sx1a ascend610Lite ascend610Liteesl),)
			EXTRA_CFLAGS += -DCFG_FEATURE_AIC_AIV_UTIL_FROM_TS
		endif
		ifneq ($(filter $(PRODUCT), ascend910B),)
			include ${DRIVER_KERNEL_DIR}/src/dms/hccs/dms_hccs.mk
			EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/hccs
			EXTRA_CFLAGS += -DCFG_FEATURE_AIC_AIV_UTIL_FROM_TS
			EXTRA_CFLAGS += -DCFG_FEATURE_IPC_FORMAT_V2
			EXTRA_CFLAGS += -DCFG_FEATURE_HCCS
		endif
		ifneq ($(filter $(PRODUCT), ascend310B ascend310Brc ascend310Besl ascend310Brcesl ascend310Bemu as31xm1),)
			EXTRA_CFLAGS += -DCFG_FEATURE_AIC_AIV_UTIL_FROM_TS
			EXTRA_CFLAGS += -DCFG_FEATURE_IPC_FORMAT_V2
		endif
		ifneq ($(filter $(PRODUCT), ascend310Brc),)
			EXTRA_CFLAGS += -DCFG_CHIP_INFO_FROM_VMNG
		endif
		ifneq ($(filter $(PRODUCT), ascend910),)
			EXTRA_CFLAGS += -DCFG_FEATURE_AI_UTIL_BY_CALCULATE
			EXTRA_CFLAGS += -DCFG_FEATURE_TS_FAULT_MASK
		endif
		ifneq ($(filter $(PRODUCT), ascend310 ascend310rc),)
			EXTRA_CFLAGS += -DCFG_FEATURE_AI_UTIL_FROM_IPC
		endif
		ifneq ($(filter $(PRODUCT), helper310p ascend310Brc ascend610 ascend610Lite ascend610Liteesl as31xm1),)
			EXTRA_CFLAGS += -DCFG_FEATURE_OSC_FREQ
		endif
	endif
	include ${DRIVER_KERNEL_DIR}/src/dms/adapt/dms_adapt.mk
	include ${DRIVER_KERNEL_DIR}/src/dms/core/dms_core.mk
	include ${DRIVER_KERNEL_DIR}/src/dms/custom/dms_custom.mk
	include ${DRIVER_KERNEL_DIR}/src/dms/devid/dms_devid.mk
	include ${DRIVER_KERNEL_DIR}/src/dms/event/dms_event_adapt.mk
	#include ${DRIVER_KERNEL_DIR}/src/dms/smf/sensor/dms_sensor.mk
	include ${DRIVER_KERNEL_DIR}/src/dms/status/dms_status.mk
	include ${DRIVER_KERNEL_DIR}/src/dms/config/dms_config.mk
	include $(DRIVER_KERNEL_DIR)/src/dms/product/dms_product.mk
	include $(DRIVER_KERNEL_DIR)/src/dms/xpc/dms_xpc.mk
	include ${DRIVER_KERNEL_DIR}/src/dms/time/dms_time.mk

	ifeq ($(PRODUCT_SIDE), device)
		EXTRA_CFLAGS += -DDMS_SHARED_NO_EXPORT
		drv_devmng-y += core/securec_compat.o
		drv_devmng-y += dtm/dms_dev_node.o
		drv_devmng-y += dtm/dms_node_type.o
		drv_devmng-y += smf/core/dms_sensor_init.o
		drv_devmng-y += smf/core/dms_notifier.o
		drv_devmng-y += smf/event/dms_event.o
		drv_devmng-y += smf/event/dms_event_converge.o
		drv_devmng-y += smf/event/dms_event_distribute.o
		drv_devmng-y += smf/event/dms_event_distribute_proc.o
		drv_devmng-y += smf/event/dms_event_dfx.o
		drv_devmng-y += smf/event/smf_event_adapt.o
		drv_devmng-y += smf/sensor/dms_sensor.o
		drv_devmng-y += smf/sensor/dms_sensor_notify.o
		drv_devmng-y += smf/sensor/dms_sensor_statis.o
		drv_devmng-y += smf/sensor/dms_sensor_discrete.o
		drv_devmng-y += smf/sensor/dms_sensor_general.o
		drv_devmng-y += smf/sensor/config/dms_sensor_type.o
		drv_devmng-y += smf/report/dfm_dev_register.o
		drv_devmng-y += smf/report/dfm_report.o
		drv_devmng-y += smf/report/dfm_safety_report.o
		drv_devmng-y += ../dbl/dev_urd/urd_feature.o
		drv_devmng-y += ../dbl/dev_urd/urd_kv.o
		drv_devmng-y += ../dbl/dev_urd/urd_acc_ctrl.o
		drv_devmng-y += ../dbl/dev_urd/urd_container.o
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/dtm
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/smf/core
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/smf/event
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/smf/sensor
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/smf/sensor/config
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dbl/dev_urd
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/fpdc
	endif

	EXTRA_CFLAGS += -I$(TOP_DIR)/libc_sec/include
	EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
	EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dbl/dev_urd
	EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms
	EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/event
	EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/smf/event
	EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/include
	EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/smf/sensor
	EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/smf/sensor/config
	EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/xpc
	EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/common
	EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_inc
	EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/soft_fault
	EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver
	EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/firmware/hiss/inc

	ifneq ($(filter $(PRODUCT),  ascend310 ascend310p ascend910 ascend910B ascend310B),)
		EXTRA_CFLAGS += -DCFG_FEATURE_HEART_BEAT
		EXTRA_CFLAGS += -DCFG_FEATURE_HEALTH_ERR_CODE
	endif
	ifneq ($(filter $(PRODUCT), ascend310B ascend310Brc ascend910B as31xm1),)
		EXTRA_CFLAGS += -DCFG_FEATURE_NEW_EVENT_CODE
	endif

	ifneq ($(filter $(PRODUCT),  ascend310B ascend310Brc ascend910B),)
		EXTRA_CFLAGS += -DCFG_FEATURE_ERRORCODE_ON_NEW_CHIPS
		EXTRA_CFLAGS += -DCFG_TRS_HB_REFACTOR_FEATURE
	endif

    ifneq ($(filter $(PRODUCT), ascend910 ascend310 ascend310p ascend310B ascend310Besl ascend310Bemu), )
        EXTRA_CFLAGS += -DCFG_FEATURE_TOPOLOGY_BY_SMP
    endif
    ifneq ($(filter $(PRODUCT), ascend910B ascend920esl), )
        EXTRA_CFLAGS += -DCFG_FEATURE_PG
        EXTRA_CFLAGS += -DCFG_FEATURE_VFG
        EXTRA_CFLAGS += -DCFG_FEATURE_SRIOV
        EXTRA_CFLAGS += -DCFG_FEATURE_TOPOLOGY_BY_HCCS_LINK_STATUS
    endif
	ifneq ($(filter $(PRODUCT), ascend310B ascend310Besl ascend310Brc ascend310Brcesl as31xm1), )
		EXTRA_CFLAGS += -DCFG_FEATURE_PG
		ifeq ($(PRODUCT),as31xm1)
			EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V11
		endif
	endif
    ifneq ($(filter $(PRODUCT), ascend310Besl),)
        EXTRA_CFLAGS += -DCFG_FEATURE_HEALTH_ERR_CODE
    endif
    # support device container
    ifneq ($(filter $(PRODUCT), ascend310rc helper310p ascend310Brc ascend310Brcesl ascend310Brcemu),)
        EXTRA_CFLAGS += -DCFG_FEATURE_DEVICE_CONTAINER
    endif
    ifneq ($(filter $(PRODUCT),  ascend920esl ascend310Besl ascend310Brcesl),)
        EXTRA_CFLAGS += -DCFG_ENV_ESL
    endif

    ifneq ($(filter $(PRODUCT), ascend310 ascend310rc),)
        EXTRA_CFLAGS += -DCFG_FEATURE_UNSUPPORT_FAULT_MANAGE
    endif

endif #end for CMake
