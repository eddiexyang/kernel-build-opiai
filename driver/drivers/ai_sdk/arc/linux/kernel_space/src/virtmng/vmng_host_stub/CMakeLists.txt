set(drv_virtmng_host_stub_depends drv_pcie_host drv_seclib_host ascend_uda)
add_host_ko(LOCAL_MODULE drv_virtmng_host_stub
	KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
	MAKE_ARGS ${MAKE_ARGS}
	TARGETE_DPENDS "${drv_virtmng_host_stub_depends}")


