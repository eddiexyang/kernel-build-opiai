set(drv_ascend_ctl_depends )
add_device_ko(LOCAL_MODULE drv_ascend_ctl
              KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
			  MAKE_ARGS ${MAKE_ARGS}
              TARGETE_DPENDS "${drv_ascend_ctl_depends}")
