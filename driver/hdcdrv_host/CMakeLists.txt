if (${PRODUCT} STREQUAL as31xm1)
	set(drv_pcie_hdc_host_depends drv_pcie_host)
	add_device_ko(LOCAL_MODULE drv_pcie_hdc_host
		KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
		MAKE_ARGS ${MAKE_ARGS}
		TARGETE_DPENDS "${drv_pcie_hdc_host_depends}")
else ()

set(product_list "ascend910" "ascend310p" "ascend910B")
if (${PRODUCT} IN_LIST product_list)
	set(drv_pcie_hdc_host_depends drv_pcie_host drv_seclib_host drv_virtmng_host drv_vpc_host ascend_uda ascend_soc_resmng dbl_runenv_config)
else ()
	if (${PRODUCT} STREQUAL ascend310)
		set(drv_pcie_hdc_host_depends drv_pcie_host drv_seclib_host ascend_uda ascend_soc_resmng drv_devmng_host dbl_runenv_config)
	else ()
		set(drv_pcie_hdc_host_depends drv_pcie_host drv_seclib_host ascend_uda ascend_soc_resmng dbl_runenv_config)
	endif()
endif ()

add_host_ko(LOCAL_MODULE drv_pcie_hdc_host
	KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
	MAKE_ARGS ${MAKE_ARGS}
	TARGETE_DPENDS "${drv_pcie_hdc_host_depends}")

endif ()