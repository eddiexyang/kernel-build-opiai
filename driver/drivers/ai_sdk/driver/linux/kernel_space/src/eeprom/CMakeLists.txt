set(product_list_310 "ascend310" "ascend310rc")
if(${PRODUCT} IN_LIST product_list_310)
	set(drv_e2prom_depends
            drv_nor_flash
	)
else()
	set(drv_e2prom_depends)
endif()

add_device_ko(LOCAL_MODULE drv_e2prom
            KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
            MAKE_ARGS ${MAKE_ARGS}
 			TARGETE_DPENDS "${drv_e2prom_depends}"
)

# add_device_ko_target(drv_e2prom ${CMAKE_CURRENT_SOURCE_DIR})

