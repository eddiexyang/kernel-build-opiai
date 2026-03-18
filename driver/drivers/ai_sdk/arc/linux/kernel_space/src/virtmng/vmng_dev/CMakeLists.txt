if( ${PRODUCT} STREQUAL ascend310Brc)
	set(ascend_virtmng_dev_depends drv_icm drv_user_cfg dbl_chip_config ascend_uda ascend_soc_resmng)
else ()
	set(ascend_virtmng_dev_depends drv_pcie drv_icm drv_user_cfg dbl_chip_config ascend_uda ascend_soc_resmng)
endif ()
add_device_ko(LOCAL_MODULE ascend_virtmng_dev
	KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
	MAKE_ARGS ${MAKE_ARGS}
	TARGETE_DPENDS "${ascend_virtmng_dev_depends}")

