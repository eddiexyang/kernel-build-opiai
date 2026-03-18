set(drv_davinci_intf_host_depends drv_seclib_host)
add_host_ko(LOCAL_MODULE drv_davinci_intf_host
			KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
			MAKE_ARGS ${MAKE_ARGS}
			TARGETE_DPENDS "${drv_davinci_intf_host_depends}")
