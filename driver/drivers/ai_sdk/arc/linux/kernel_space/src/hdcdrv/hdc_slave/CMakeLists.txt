if( ${PRODUCT} STREQUAL ascend310p)
    set(drv_pcie_hdc_depends drv_pcie drv_devmng hsm_firmware_update hsm_flash hsm_heartbeat)
else ()
    set(drv_pcie_hdc_depends drv_pcie drv_devmng)
endif ()
add_device_ko(LOCAL_MODULE drv_pcie_hdc
			KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
			MAKE_ARGS ${MAKE_ARGS}
			TARGETE_DPENDS "${drv_pcie_hdc_depends}")

#add_device_ko_target(drv_pcie_hdc ${CMAKE_CURRENT_SOURCE_DIR})
#add_dependencies(drv_pcie_hdc drv_pcie drv_devmng)
