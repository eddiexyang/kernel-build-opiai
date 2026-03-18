if (${PRODUCT_SIDE} STREQUAL host)
	set(drv_soft_fault_depends drv_seclib_host ascend_urd ascend_uda ascend_dms_dtm ascend_dms_smf drv_devmng_host drv_pcie_host drv_davinci_intf_host)
	add_host_ko(LOCAL_MODULE drv_soft_fault
		KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
		MAKE_ARGS ${MAKE_ARGS}
		TARGETE_DPENDS "${drv_soft_fault_depends}")
else ()
	if( ${PRODUCT} STREQUAL ascend310p)
		set(drv_soft_fault_depends drv_pcie ascend_urd ascend_uda ascend_dms_dtm ascend_dms_smf drv_devmng hsm_firmware_update hsm_flash hsm_heartbeat)
	elseif( ${PRODUCT} STREQUAL ascend910B)
		set(drv_soft_fault_depends drv_pcie ascend_urd ascend_uda ascend_dms_dtm ascend_dms_smf drv_devmng)
	elseif( ${PRODUCT} STREQUAL ascend310Brc OR ${PRODUCT} STREQUAL helper310p)
		set(drv_soft_fault_depends ascend_urd ascend_uda ascend_dms_dtm ascend_dms_smf drv_devmng)
	else ()
		set(drv_soft_fault_depends drv_pcie ascend_urd ascend_uda ascend_dms_dtm ascend_dms_smf drv_devmng)
	endif ()
	add_device_ko(LOCAL_MODULE drv_soft_fault
		KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
		MAKE_ARGS ${MAKE_ARGS}
		TARGETE_DPENDS "${drv_soft_fault_depends}")
endif ()
