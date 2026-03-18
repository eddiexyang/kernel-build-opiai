set(product_list "ascend910B" "ascend920esl" "ascend920emu")
if (${PRODUCT} IN_LIST product_list)
	set(drv_pcie_depends dbl_runenv_config drv_dfm drv_pmu)
else ()
	set(drv_pcie_depends dbl_runenv_config drv_dfm)
endif ()

add_device_ko(LOCAL_MODULE drv_pcie
			KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
			MAKE_ARGS ${MAKE_ARGS}
			TARGETE_DPENDS "${drv_pcie_depends}")
