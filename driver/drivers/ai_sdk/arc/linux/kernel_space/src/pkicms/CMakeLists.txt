if( ${PRODUCT} STREQUAL ascend310p)
	set(drv_pkicms_depends drv_user_cfg hsm_firmware_update)
elseif ( ${PRODUCT} STREQUAL ascend310B)
    set(drv_pkicms_depends drv_user_cfg)
elseif ( ${PRODUCT} STREQUAL ascend310Besl)
    set(drv_pkicms_depends drv_user_cfg hsm_firmware_update hsm_flash hsm_heartbeat)
elseif ( ${PRODUCT} STREQUAL ascend310Brc)
    set(drv_pkicms_depends drv_user_cfg hsm_firmware_update hsm_flash hsm_heartbeat)
elseif ( ${PRODUCT} STREQUAL as31xm1)
    set(drv_pkicms_depends drv_user_cfg hsm_firmware_update hsm_flash hsm_heartbeat)
elseif ( ${PRODUCT} STREQUAL ascend310Brcesl)
    set(drv_pkicms_depends drv_user_cfg hsm_firmware_update hsm_flash hsm_heartbeat)
else ()
	set(drv_pkicms_depends)
endif ()
list(APPEND MAKE_ARGS
    KBUILD_MODPOST_WARN=1
)
add_device_ko(LOCAL_MODULE drv_pkicms
            MAKE_ARGS ${MAKE_ARGS}
            KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
			TARGETE_DPENDS "${drv_pkicms_depends}")

