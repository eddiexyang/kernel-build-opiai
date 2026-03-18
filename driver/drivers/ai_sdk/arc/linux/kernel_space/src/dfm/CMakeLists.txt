set(drv_dfm_depends mntn_bbox)
add_device_ko(LOCAL_MODULE drv_dfm
            KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
            MAKE_ARGS ${MAKE_ARGS}
			TARGETE_DPENDS "${drv_dfm_depends}")
